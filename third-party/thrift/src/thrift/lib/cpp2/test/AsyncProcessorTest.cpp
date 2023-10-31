/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <memory>
#include <stdexcept>

#include <folly/Portability.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <folly/test/TestUtils.h>

#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/AsyncProcessorHelper.h>
#include <thrift/lib/cpp2/async/HTTPClientChannel.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/BaseThriftServer.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/MonitoringServerInterface.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketRoutingHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/SetupFrameHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <thrift/lib/cpp2/test/gen-cpp2/Child.h>
#include <thrift/lib/cpp2/test/gen-cpp2/DummyControl.h>
#include <thrift/lib/cpp2/test/gen-cpp2/DummyMonitor.h>
#include <thrift/lib/cpp2/test/gen-cpp2/DummySecurity.h>
#include <thrift/lib/cpp2/test/gen-cpp2/DummyStatus.h>
#include <thrift/lib/cpp2/test/gen-cpp2/Parent.h>
#include <thrift/lib/cpp2/test/gen-cpp2/SchemaService.h>

namespace apache::thrift::test {

using MethodMetadata = AsyncProcessorFactory::MethodMetadata;
using MethodMetadataMap = AsyncProcessorFactory::MethodMetadataMap;
using CreateMethodMetadataResult =
    AsyncProcessorFactory::CreateMethodMetadataResult;

using TransportType = Cpp2ConnContext::TransportType;

namespace {
class ChildHandler : public apache::thrift::ServiceHandler<Child> {
  MOCK_METHOD(std::unique_ptr<InteractionIf>, createInteraction, ());

  int parentMethod1() override { return 42; }
  int parentMethod3() override { return 24; }
  void childMethod2(std::string& result) override { result = "hello"; }
};

MethodMetadataMap& expectMethodMetadataMap(
    CreateMethodMetadataResult& createMethodMetadataResult) {
  if (auto map = std::get_if<MethodMetadataMap>(&createMethodMetadataResult)) {
    return *map;
  }
  throw std::logic_error{"Expected createMethodMetadata to return a map"};
}

std::unique_ptr<HTTP2RoutingHandler> createHTTP2RoutingHandler(
    ThriftServer& server) {
  auto h2_options = std::make_unique<proxygen::HTTPServerOptions>();
  h2_options->threads = static_cast<size_t>(server.getNumIOWorkerThreads());
  h2_options->idleTimeout = server.getIdleTimeout();
  h2_options->shutdownOn = {SIGINT, SIGTERM};
  return std::make_unique<HTTP2RoutingHandler>(
      std::move(h2_options), server.getThriftProcessor(), server);
}

} // namespace

TEST(AsyncProcessorMetadataTest, ParentMetadata) {
  apache::thrift::ServiceHandler<Parent> service;
  auto createMethodMetadataResult = service.createMethodMetadata();
  auto& metadataMap = expectMethodMetadataMap(createMethodMetadataResult);

  EXPECT_EQ(metadataMap.size(), 3);
  EXPECT_NE(metadataMap.find("parentMethod1"), metadataMap.end());
  EXPECT_NE(metadataMap.find("parentMethod2"), metadataMap.end());
  EXPECT_NE(metadataMap.find("parentMethod3"), metadataMap.end());
}

#if defined(THRIFT_SCHEMA_AVAILABLE)
TEST(AsyncProcessorMetadataTest, Schema) {
  apache::thrift::ServiceHandler<SchemaService> service;
  auto schemas = service.getServiceMetadataV1();
  EXPECT_TRUE(schemas);
  EXPECT_EQ(schemas->size(), 1);
  auto schema = schemas->at(0);
  EXPECT_EQ(schema.definitions()->size(), 1);
  auto svc_schema_0 = schema.definitions()->at(0).get_serviceDef();
  EXPECT_EQ(svc_schema_0.functions()->size(), 1);
}
#endif

TEST(AsyncProcessorMetadataTest, ChildMetadata) {
  ChildHandler service;
  auto createMethodMetadataResult = service.createMethodMetadata();
  auto& metadataMap = expectMethodMetadataMap(createMethodMetadataResult);

  EXPECT_EQ(metadataMap.size(), 6);
  EXPECT_NE(metadataMap.find("parentMethod1"), metadataMap.end());
  EXPECT_NE(metadataMap.find("parentMethod2"), metadataMap.end());
  EXPECT_NE(metadataMap.find("parentMethod3"), metadataMap.end());
  EXPECT_NE(metadataMap.find("childMethod1"), metadataMap.end());
  EXPECT_NE(metadataMap.find("childMethod2"), metadataMap.end());
  EXPECT_NE(
      metadataMap.find("Interaction.interactionMethod"), metadataMap.end());
}

namespace {
class ChildHandlerWithMetadata : public ChildHandler {
 public:
  using MetadataFactory = std::function<CreateMethodMetadataResult(
      CreateMethodMetadataResult defaultResult)>;

  explicit ChildHandlerWithMetadata(MetadataFactory metadataFactory)
      : metadataFactory_(std::move(metadataFactory)) {}

 private:
  CreateMethodMetadataResult createMethodMetadata() override {
    return metadataFactory_(ChildHandler::createMethodMetadata());
  }

  MetadataFactory metadataFactory_;
};

class AsyncProcessorMethodResolutionTestP
    : public ::testing::TestWithParam<TransportType> {
 public:
  TransportType transportType() const { return GetParam(); }

  std::unique_ptr<ScopedServerInterfaceThread> makeServer(
      std::shared_ptr<AsyncProcessorFactory> service,
      ScopedServerInterfaceThread::ServerConfigCb configureServer = {}) {
    auto runner = std::make_unique<ScopedServerInterfaceThread>(
        std::move(service), std::move(configureServer));
    if (transportType() == TransportType::HTTP2) {
      auto& thriftServer =
          dynamic_cast<ThriftServer&>(runner->getThriftServer());
      thriftServer.addRoutingHandler(createHTTP2RoutingHandler(thriftServer));
    }
    return runner;
  }

  template <typename ClientT>
  std::unique_ptr<ClientT> makeClientFor(ScopedServerInterfaceThread& runner) {
    return runner.newClient<ClientT>(
        nullptr, [&](auto socket) -> ClientChannel::Ptr {
          switch (transportType()) {
            case TransportType::HEADER:
              return HeaderClientChannel::newChannel(std::move(socket));
            case TransportType::ROCKET:
              return RocketClientChannel::newChannel(std::move(socket));
            case TransportType::HTTP2: {
              auto channel =
                  HTTPClientChannel::newHTTP2Channel(std::move(socket));
              channel->setProtocolId(protocol::T_COMPACT_PROTOCOL);
              return channel;
            }
            default:
              throw std::logic_error{"Unreachable!"};
          }
        });
  }
};
} // namespace

TEST_P(AsyncProcessorMethodResolutionTestP, EmptyMap) {
  auto service = std::make_shared<ChildHandlerWithMetadata>(
      [](auto&&) -> MethodMetadataMap { return {}; });
  auto runner = makeServer(service);
  auto client = makeClientFor<ChildAsyncClient>(*runner);

  EXPECT_THROW(client->semifuture_parentMethod1().get(), TApplicationException);
  EXPECT_THROW(client->semifuture_childMethod2().get(), TApplicationException);
}

TEST_P(AsyncProcessorMethodResolutionTestP, MistypedMetadataDeathTest) {
  if constexpr (!folly::kIsDebug) {
    // Opt-mode causes UB instead of dying
    return;
  }
  folly::SingletonVault::singleton()->destroyInstances();
  folly::SingletonVault::singleton()->reenableInstances();
  auto runTest = [&](auto&& callback) {
    auto service =
        std::make_shared<ChildHandlerWithMetadata>([](auto defaultResult) {
          MethodMetadataMap result;
          const auto& defaultMap = expectMethodMetadataMap(defaultResult);
          for (auto& [name, _] : defaultMap) {
            class DummyMethodMetadata : public MethodMetadata {};
            result.emplace(name, std::make_shared<DummyMethodMetadata>());
          }
          return result;
        });
    auto runner = makeServer(service);
    callback(makeClientFor<ChildAsyncClient>(*runner));
  };

  EXPECT_DEATH(
      runTest([](auto client) { client->semifuture_parentMethod1().get(); }),
      "Received MethodMetadata of an unknown type");
  EXPECT_DEATH(
      runTest([](auto client) { client->semifuture_childMethod2().get(); }),
      "Received MethodMetadata of an unknown type");
}

TEST_P(AsyncProcessorMethodResolutionTestP, ParentMapDeathTest) {
  if constexpr (!folly::kIsDebug) {
    // Opt-mode causes UB instead of dying
    return;
  }
  // We strictly require the correct metadata type, even if the parent's map
  // might contain all the function pointers we need.
  EXPECT_DEATH(
      [&] {
        auto service = std::make_shared<ChildHandlerWithMetadata>([](auto&&) {
          return apache::thrift::ServiceHandler<Parent>{}
              .createMethodMetadata();
        });
        auto runner = makeServer(service);
        auto client = makeClientFor<ChildAsyncClient>(*runner);
        client->semifuture_parentMethod1().get();
      }(),
      "Received MethodMetadata of an unknown type");
}

TEST_P(AsyncProcessorMethodResolutionTestP, Wildcard) {
  auto runTest = [&](auto&& callback) {
    class ChildImpl : public ChildHandler {
      CreateMethodMetadataResult createMethodMetadata() override {
        auto defaultResult = ChildHandler::createMethodMetadata();
        auto& defaultMap = expectMethodMetadataMap(defaultResult);
        MethodMetadataMap knownMethods;
        // swap out for another method to make sure we are using this map
        knownMethods.emplace(
            "parentMethod1", std::move(defaultMap["parentMethod3"]));
        return WildcardMethodMetadataMap{
            std::make_shared<const WildcardMethodMetadata>(),
            std::move(knownMethods)};
      }

      std::unique_ptr<AsyncProcessor> getProcessor() override {
        class ProcessorImpl : public ChildAsyncProcessor {
          using ChildAsyncProcessor::ChildAsyncProcessor;

          void processSerializedCompressedRequestWithMetadata(
              ResponseChannelRequest::UniquePtr req,
              SerializedCompressedRequest&& serializedRequest,
              const MethodMetadata& methodMetadata,
              protocol::PROTOCOL_TYPES protType,
              Cpp2RequestContext* context,
              folly::EventBase* eb,
              concurrency::ThreadManager* tm) override {
            if (dynamic_cast<const WildcardMethodMetadata*>(&methodMetadata)) {
              // Instead of crashing when receiving a wildcard method metadata,
              // return an error so we can check that the correct metadata was
              // propagated.
              req->sendErrorWrapped(
                  folly::make_exception_wrapper<TApplicationException>(
                      TApplicationException::UNKNOWN_METHOD, ""),
                  "");
              return;
            }
            ChildAsyncProcessor::processSerializedCompressedRequestWithMetadata(
                std::move(req),
                std::move(serializedRequest),
                methodMetadata,
                protType,
                context,
                eb,
                tm);
          }

          void executeRequest(
              ServerRequest&& request,
              const AsyncProcessorFactory::MethodMetadata& methodMetadata)
              override {
            if (dynamic_cast<const WildcardMethodMetadata*>(&methodMetadata)) {
              // Instead of crashing when receiving a wildcard method metadata,
              // return an error so we can check that the correct metadata was
              // propagated.
              auto eb = detail::ServerRequestHelper::eventBase(request);
              eb->runInEventBaseThread(
                  [request = std::move(request)]() mutable {
                    request.request()->sendErrorWrapped(
                        folly::make_exception_wrapper<TApplicationException>(
                            TApplicationException::UNKNOWN_METHOD, ""),
                        "");
                  });
              return;
            }
            ChildAsyncProcessor::executeRequest(
                std::move(request), methodMetadata);
          }
        };
        return std::make_unique<ProcessorImpl>(this);
      }
    };
    auto runner = makeServer(std::make_shared<ChildImpl>());
    return callback(makeClientFor<ChildAsyncClient>(*runner));
  };

  EXPECT_EQ(
      runTest(
          [](auto client) { return client->semifuture_parentMethod1().get(); }),
      24);
  // This method is not in the map so the processor should receive
  // WildcardMethodMetadata
  EXPECT_THROW(
      runTest([](auto client) { client->semifuture_childMethod2().get(); }),
      TApplicationException);
}

TEST_P(AsyncProcessorMethodResolutionTestP, Interaction) {
  if (transportType() != TransportType::ROCKET) {
    // Interactions are only supported on rocket
    return;
  }
  class ChildWithInteraction : public apache::thrift::ServiceHandler<Child> {
    std::unique_ptr<InteractionIf> createInteraction() override {
      class InteractionImpl : public InteractionIf {
        int interactionMethod() override { return ++counter_; }
        std::atomic<int> counter_{0};
      };
      return std::make_unique<InteractionImpl>();
    }
  };
  auto service = std::make_shared<ChildWithInteraction>();
  auto runner = makeServer(service);
  auto client = makeClientFor<ChildAsyncClient>(*runner);

  auto interaction2 = client->createInteraction();
  EXPECT_EQ(interaction2.semifuture_interactionMethod().get(), 1);
  EXPECT_EQ(interaction2.semifuture_interactionMethod().get(), 2);
}

TEST_P(
    AsyncProcessorMethodResolutionTestP,
    MonitoringAndStatusMethodMultiplexing) {
  class Monitor : public apache::thrift::ServiceHandler<DummyMonitor>,
                  public MonitoringServerInterface {
    std::int64_t getCounter() override { return 420; }
  };
  class Status : public apache::thrift::ServiceHandler<DummyStatus>,
                 public StatusServerInterface {
    void async_eb_getStatus(
        std::unique_ptr<HandlerCallback<std::int64_t>> callback) override {
      callback->result(360);
    }
  };
  class Control : public apache::thrift::ServiceHandler<DummyControl>,
                  public ControlServerInterface {
    std::int64_t getOption() override { return 42; }
  };
  class Security : public apache::thrift::ServiceHandler<DummySecurity>,
                   public SecurityServerInterface {
    std::int64_t getState() override { return 4200; }
  };
  auto service =
      std::make_shared<ChildHandlerWithMetadata>([](auto defaultResult) {
        MethodMetadataMap result;
        auto& defaultMap = expectMethodMetadataMap(defaultResult);
        result.emplace("parentMethod1", std::move(defaultMap["parentMethod1"]));
        return result;
      });
  auto runner = makeServer(service, [&](ThriftServer& server) {
    server.setStatusInterface(std::make_shared<Status>());
    server.setMonitoringInterface(std::make_shared<Monitor>());
    server.setControlInterface(std::make_shared<Control>());
    server.setSecurityInterface(std::make_shared<Security>());
  });

  auto client = makeClientFor<ChildAsyncClient>(*runner);
  auto monitoringClient = makeClientFor<DummyMonitorAsyncClient>(*runner);
  auto statusClient = makeClientFor<DummyStatusAsyncClient>(*runner);
  auto controlClient = makeClientFor<DummyControlAsyncClient>(*runner);
  auto securityClient = makeClientFor<DummySecurityAsyncClient>(*runner);

  EXPECT_EQ(client->semifuture_parentMethod1().get(), 42);
  // The monitoring interface should be invoked if the user interface doesn't
  // have the method.
  EXPECT_EQ(monitoringClient->semifuture_getCounter().get(), 420);
  // The status interface should be invoked if the user interface doesn't
  // have the method.
  EXPECT_EQ(statusClient->semifuture_getStatus().get(), 360);
  // The control interface should be invoked if no handler with higher
  // precedence has the method.
  EXPECT_EQ(controlClient->semifuture_getOption().get(), 42);
  // The security interface should be invoked if no handler with higher
  // precedence has the method.
  EXPECT_EQ(securityClient->semifuture_getState().get(), 4200);
  // If the method is in neither user, status, or monitoring interfaces, we
  // expect an error.
  EXPECT_THROW(client->semifuture_parentMethod3().get(), TApplicationException);
}

TEST_P(
    AsyncProcessorMethodResolutionTestP,
    MonitoringMethodMultiplexingCollision) {
  class ChildMonitor : public ChildHandler, public MonitoringServerInterface {
    void childMethod2(std::string& result) override {
      result = "hello from Monitor";
    }
  };
  auto runner =
      makeServer(std::make_shared<ChildHandler>(), [&](ThriftServer& server) {
        server.setMonitoringInterface(std::make_shared<ChildMonitor>());
      });

  auto client = makeClientFor<ChildAsyncClient>(*runner);

  // The user method should take precedence
  EXPECT_EQ(client->semifuture_childMethod2().get(), "hello");
}

TEST_P(
    AsyncProcessorMethodResolutionTestP,
    WildcardMethodMetadataWithMonitoringAndStatus) {
  class Monitor : public apache::thrift::ServiceHandler<DummyMonitor>,
                  public MonitoringServerInterface {
    std::int64_t getCounter() override { return 420; }
  };
  class Status : public apache::thrift::ServiceHandler<DummyStatus>,
                 public StatusServerInterface {
    void async_eb_getStatus(
        std::unique_ptr<HandlerCallback<std::int64_t>> callback) override {
      callback->result(360);
    }
  };
  class Control : public apache::thrift::ServiceHandler<DummyControl>,
                  public ControlServerInterface {
    std::int64_t getOption() override { return 42; }
  };
  class Security : public apache::thrift::ServiceHandler<DummySecurity>,
                   public SecurityServerInterface {
    std::int64_t getState() override { return 4200; }
  };
  class ChildHandlerWithWildcard : public ChildHandlerWithMetadata {
   public:
    ChildHandlerWithWildcard()
        : ChildHandlerWithMetadata([](auto defaultResult) {
            auto& defaultMap = expectMethodMetadataMap(defaultResult);
            return WildcardMethodMetadataMap{
                std::make_shared<const WildcardMethodMetadata>(),
                std::move(defaultMap)};
          }) {}
    std::unique_ptr<AsyncProcessor> getProcessor() override {
      // Instead of crashing on WildcardMethodMetadata, we send back an unknown
      // method error.
      class Processor : public AsyncProcessor {
       public:
        void processSerializedCompressedRequestWithMetadata(
            ResponseChannelRequest::UniquePtr req,
            SerializedCompressedRequest&& serializedRequest,
            const MethodMetadata& untypedMethodMetadata,
            protocol::PROTOCOL_TYPES protocol,
            Cpp2RequestContext* context,
            folly::EventBase* eb,
            concurrency::ThreadManager* tm) override {
          if (untypedMethodMetadata.isWildcard()) {
            AsyncProcessorHelper::sendUnknownMethodError(
                std::move(req), "someUnknownMethod");
            return;
          }
          processor_->processSerializedCompressedRequestWithMetadata(
              std::move(req),
              std::move(serializedRequest),
              untypedMethodMetadata,
              protocol,
              context,
              eb,
              tm);
        }

        void executeRequest(
            ServerRequest&& request,
            const AsyncProcessorFactory::MethodMetadata& methodMetadata)
            override {
          if (methodMetadata.isWildcard()) {
            auto eb = detail::ServerRequestHelper::eventBase(request);
            eb->runInEventBaseThread([request = std::move(request)]() mutable {
              AsyncProcessorHelper::sendUnknownMethodError(
                  detail::ServerRequestHelper::request(std::move(request)),
                  "someUnknownMethod");
            });
            return;
          }
          processor_->executeRequest(std::move(request), methodMetadata);
        }

        explicit Processor(std::unique_ptr<AsyncProcessor> processor)
            : processor_(std::move(processor)) {}

       private:
        std::unique_ptr<AsyncProcessor> processor_;
      };

      return std::make_unique<Processor>(
          ChildHandlerWithMetadata::getProcessor());
    }
  };
  auto service = std::make_shared<ChildHandlerWithWildcard>();
  auto runner = makeServer(service, [&](ThriftServer& server) {
    server.setStatusInterface(std::make_shared<Status>());
    server.setMonitoringInterface(std::make_shared<Monitor>());
    server.setControlInterface(std::make_shared<Control>());
    server.setSecurityInterface(std::make_shared<Security>());
  });

  auto client = makeClientFor<ChildAsyncClient>(*runner);
  auto monitoringClient = makeClientFor<DummyMonitorAsyncClient>(*runner);
  auto statusClient = makeClientFor<DummyStatusAsyncClient>(*runner);
  auto controlClient = makeClientFor<DummyControlAsyncClient>(*runner);
  auto securityClient = makeClientFor<DummySecurityAsyncClient>(*runner);

  EXPECT_EQ(client->semifuture_parentMethod1().get(), 42);
  // The monitoring interface should not be available
  EXPECT_THROW(
      monitoringClient->semifuture_getCounter().get(), TApplicationException);
  // The status interface should not be available
  EXPECT_THROW(
      statusClient->semifuture_getStatus().get(), TApplicationException);
  // The control interface should not be available
  EXPECT_THROW(
      controlClient->semifuture_getOption().get(), TApplicationException);
  // The security interface should not be available
  EXPECT_THROW(
      securityClient->semifuture_getState().get(), TApplicationException);
}

INSTANTIATE_TEST_SUITE_P(
    AsyncProcessorMethodResolutionTestP,
    AsyncProcessorMethodResolutionTestP,
    ::testing::Values(
        TransportType::HEADER, TransportType::ROCKET, TransportType::HTTP2));

} // namespace apache::thrift::test

namespace apache::thrift::detail {
// Setup monitoring interface handler for test below
THRIFT_PLUGGABLE_FUNC_SET(
    std::unique_ptr<rocket::SetupFrameHandler>,
    createRocketMonitoringSetupFrameHandler,
    ThriftServer& server) {
  class MonitoringConnectionHandler : public rocket::SetupFrameHandler {
   public:
    explicit MonitoringConnectionHandler(ThriftServer& server)
        : server_(server) {}

    std::optional<rocket::ProcessorInfo> tryHandle(
        const RequestSetupMetadata& meta) override {
      if (meta.interfaceKind_ref() == InterfaceKind::MONITORING) {
        auto processorFactory = server_.getMonitoringInterface();
        DCHECK(processorFactory);
        return rocket::ProcessorInfo{
            *processorFactory,
            std::make_shared<concurrency::ThreadManagerExecutorAdapter>(
                folly::getGlobalCPUExecutor()),
            server_,
            nullptr /* requestsRegistry */
        };
      }
      return std::nullopt;
    }

   private:
    ThriftServer& server_;
  };
  return std::make_unique<MonitoringConnectionHandler>(server);
}
} // namespace apache::thrift::detail

namespace apache::thrift::test {

TEST(AsyncProcessorMethodResolutionTest, MultipleService) {
  class Monitor : public apache::thrift::ServiceHandler<Child>,
                  public MonitoringServerInterface {
    MOCK_METHOD(std::unique_ptr<InteractionIf>, createInteraction, ());

    int parentMethod1() override { return 84; }
    void childMethod2(std::string& result) override {
      result = "hello from Monitor";
    }
  };
  ScopedServerInterfaceThread runner{
      std::make_shared<ChildHandler>(), [&](ThriftServer& server) {
        server.setMonitoringInterface(std::make_shared<Monitor>());
      }};

  auto client = runner.newClient<ChildAsyncClient>(
      nullptr, RocketClientChannel::newChannel);
  auto monitoringClient =
      runner.newClient<ChildAsyncClient>(nullptr, [](auto socket) {
        RequestSetupMetadata setupMetadata;
        setupMetadata.interfaceKind_ref() = InterfaceKind::MONITORING;
        return RocketClientChannel::newChannelWithMetadata(
            std::move(socket), std::move(setupMetadata));
      });

  EXPECT_EQ(client->semifuture_parentMethod1().get(), 42);
  EXPECT_EQ(client->semifuture_childMethod2().get(), "hello");
  EXPECT_EQ(monitoringClient->semifuture_parentMethod1().get(), 84);
  EXPECT_EQ(
      monitoringClient->semifuture_childMethod2().get(), "hello from Monitor");
}

} // namespace apache::thrift::test
