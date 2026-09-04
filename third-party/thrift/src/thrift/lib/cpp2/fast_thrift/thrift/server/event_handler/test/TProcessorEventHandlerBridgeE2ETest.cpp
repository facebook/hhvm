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

// End-to-end coverage for TProcessorEventHandlerBridge installed in a real
// FastThriftServer, driven by a real client.
//
// The unit tests call the bridge's callbacks directly, which proves it reacts
// correctly but not that the pipeline ever reaches it. Two things can only
// break here: the event subscription (a wrong `kSubscribedEvents`, or an
// `onEvent` signature that drifts by a character, compiles and passes every
// unit test while silently never announcing a connection), and the server
// config that populates the request context and headers the bridge reads.
//
// Response headers are asserted in the unit test, where the bridge's own
// output is visible: getting from ResponseRpcMetadata.otherMetadata to the
// client is Thrift's serialization path, which the bridge does not influence.

#include <thrift/lib/cpp2/fast_thrift/thrift/server/event_handler/TProcessorEventHandlerBridge.h>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>
#include <folly/Synchronized.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/framework/FastServerModule.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServer.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::thrift::server {

namespace ftt = ::apache::thrift::fast_thrift::thrift;

namespace {

using test::integration::EchoResponse;

// Records what the service sees of its own request context. The bridge keeps
// the classic contexts entirely behind its own boundary.
struct ServiceObserved {
  bool sawRequestContext{false};
  bool sawConnectionContext{false};
  std::string headerValue;
};
using SharedServiceObserved = folly::Synchronized<ServiceObserved>;

class TestHandler
    : public FastServiceHandler<test::integration::FastThriftServer> {
 public:
  explicit TestHandler(SharedServiceObserved* observed) : observed_(observed) {}

  void async_tm_ping(ftt::FastHandlerCallbackPtr<void> cb) override {
    auto* requestContext = cb->requestContext();
    auto locked = observed_->wlock();
    locked->sawRequestContext = requestContext != nullptr;
    if (requestContext != nullptr) {
      locked->sawConnectionContext =
          requestContext->getConnectionContext() != nullptr;
      const auto& headers = requestContext->getHeaders();
      if (auto it = headers.find("probe"); it != headers.end()) {
        locked->headerValue = it->second;
      }
    }
    cb->done();
  }

  void async_tm_echo(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      std::unique_ptr<std::string> message) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = std::string("echoed:") + *message;
    cb->result(std::move(resp));
  }

 private:
  SharedServiceObserved* observed_;
};

// What the installed handlers observed, across every connection the test
// opens. Written from the server's IO thread and read from the test thread
// once the RPC has completed, so it is synchronized.
struct Observed {
  std::vector<std::string> calls;
  std::string methodName;
  bool sawRequestHeader{false};
  bool sawPeerAddress{false};
};
using SharedObserved = folly::Synchronized<Observed>;

class ObservingEventHandler : public apache::thrift::TProcessorEventHandler {
 public:
  explicit ObservingEventHandler(SharedObserved* observed)
      : observed_(observed) {}

  void* getServiceContext(
      std::string_view /*serviceName*/,
      std::string_view fnName,
      apache::thrift::TConnectionContext* connectionContext) override {
    auto locked = observed_->wlock();
    locked->calls.emplace_back("getServiceContext");
    locked->methodName = std::string(fnName);
    return connectionContext;
  }

  void preRead(void* ctx, std::string_view /*fnName*/) override {
    auto* reqCtx = static_cast<apache::thrift::Cpp2RequestContext*>(ctx);
    auto locked = observed_->wlock();
    locked->calls.emplace_back("preRead");
    // Populated only when the server wires request headers onto the context.
    locked->sawRequestHeader = reqCtx->getHeader() != nullptr &&
        reqCtx->getHeader()->getHeaders().contains("probe");
    locked->sawPeerAddress = reqCtx->getPeerAddress() != nullptr &&
        reqCtx->getPeerAddress()->isInitialized();
  }

  void postRead(
      void* /*ctx*/,
      std::string_view /*fnName*/,
      apache::thrift::transport::THeader* /*header*/,
      uint32_t /*bytes*/) override {
    observed_->wlock()->calls.emplace_back("postRead");
  }

  void preWrite(void* /*ctx*/, std::string_view /*fnName*/) override {
    observed_->wlock()->calls.emplace_back("preWrite");
  }

  void postWrite(
      void* ctx, std::string_view /*fnName*/, uint32_t /*bytes*/) override {
    observed_->wlock()->calls.emplace_back("postWrite");
    static_cast<apache::thrift::Cpp2RequestContext*>(ctx)
        ->getHeader()
        ->setHeader("stamped-by", "event-handler");
  }

 private:
  SharedObserved* observed_;
};

class ObservingServerEventHandler
    : public apache::thrift::server::TServerEventHandler {
 public:
  explicit ObservingServerEventHandler(SharedObserved* observed)
      : observed_(observed) {}

  void newConnection(
      apache::thrift::server::TConnectionContext* /*ctx*/) override {
    observed_->wlock()->calls.emplace_back("newConnection");
  }
  void connectionDestroyed(
      apache::thrift::server::TConnectionContext* /*ctx*/) override {
    observed_->wlock()->calls.emplace_back("connectionDestroyed");
  }

 private:
  SharedObserved* observed_;
};

} // namespace

class TProcessorEventHandlerBridgeE2ETest : public ::testing::Test {
 protected:
  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler_ = std::make_shared<TestHandler>(&serviceObserved_);

    FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;
    // Both required by the bridge: the first so a per-request context exists
    // at all, the second so the handlers see the request's headers.
    config.enableRequestContext = true;
    config.enableRequestHeaders = true;

    server_ = std::make_unique<FastThriftServer>(std::move(config));
    // Reserves the request slot the bridge publishes its classic context into.
    // Installing the bridge without this is a wiring error the adapter fatals
    // on, so every server that adds the module registers alongside it.
    server_->registerExtension<Cpp2BridgeExtension>();

    auto handlers = std::make_shared<TProcessorEventHandlers>();
    handlers->serviceName = "FastThriftServer";
    handlers->processor.push_back(
        std::make_shared<ObservingEventHandler>(&observed_));
    handlers->server.push_back(
        std::make_shared<ObservingServerEventHandler>(&observed_));

    server_->addModule(FastServerModule("event_handlers")
                           .addNativeThriftHandler<TProcessorEventHandlerBridge<
                               channel_pipeline::detail::ContextImpl>>(
                               TProcessorEventHandlerBridgeConfig{
                                   .handlers = std::move(handlers),
                                   .identityResolver = nullptr}));

    server_->setInterface(handler_);
    server_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    // The channel must be destroyed on the EventBase that owns it, and before
    // that thread goes away. Doing it here rather than at the end of each test
    // keeps it correct when an assertion returns early.
    if (client_ != nullptr) {
      clientThread_->getEventBase()->runInEventBaseThreadAndWait(
          [&] { client_.reset(); });
    }
    clientThread_.reset();
    server_->stop();
    server_.reset();
  }

  void connect() {
    auto* evb = clientThread_->getEventBase();
    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb, server_->getAddress());
      auto channel =
          apache::thrift::RocketClientChannel::newChannel(std::move(socket));
      client_ = std::make_unique<
          apache::thrift::Client<test::integration::FastThriftServer>>(
          std::move(channel));
    });
  }

  // The client channel lives on clientThread_'s EventBase, so the RPC has to
  // be issued from that thread — a sync_* call from here would drive an
  // EventBase that is already being driven.
  EchoResponse echo(
      apache::thrift::RpcOptions& options, const std::string& message) {
    auto* evb = clientThread_->getEventBase();
    folly::Baton<> done;
    EchoResponse result;
    folly::exception_wrapper error;
    evb->runInEventBaseThread([&] {
      client_->semifuture_echo(options, message)
          .via(evb)
          .thenValue([&](EchoResponse r) {
            result = std::move(r);
            done.post();
          })
          .thenError([&](const folly::exception_wrapper& ew) {
            error = ew;
            done.post();
          });
    });
    EXPECT_TRUE(done.try_wait_for(std::chrono::seconds{10}));
    EXPECT_FALSE(error) << error.what();
    return result;
  }

  void ping() {
    apache::thrift::RpcOptions options;
    pingWith(options);
  }

  void pingWith(apache::thrift::RpcOptions& options) {
    auto* evb = clientThread_->getEventBase();
    folly::Baton<> done;
    folly::exception_wrapper error;
    evb->runInEventBaseThread([&] {
      client_->semifuture_ping(options)
          .via(evb)
          .thenValue([&](auto&&) { done.post(); })
          .thenError([&](const folly::exception_wrapper& ew) {
            error = ew;
            done.post();
          });
    });
    EXPECT_TRUE(done.try_wait_for(std::chrono::seconds{10}));
    EXPECT_FALSE(error) << error.what();
  }

  std::shared_ptr<TestHandler> handler_;
  SharedObserved observed_;
  SharedServiceObserved serviceObserved_;
  std::unique_ptr<FastThriftServer> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  std::unique_ptr<apache::thrift::Client<test::integration::FastThriftServer>>
      client_;
};

// The whole chain over a real connection: the pipeline delivers the setup
// event, the bridge announces the connection, and one RPC drives the four
// request callbacks in classic order.
TEST_F(TProcessorEventHandlerBridgeE2ETest, RealRequestDrivesEveryCallback) {
  connect();

  apache::thrift::RpcOptions options;
  options.setWriteHeader("probe", "value");
  const auto response = echo(options, "hello");
  EXPECT_EQ(*response.message(), "echoed:hello");

  auto locked = observed_.rlock();
  EXPECT_EQ(
      locked->calls,
      (std::vector<std::string>{
          "newConnection",
          "getServiceContext",
          "preRead",
          "postRead",
          "preWrite",
          "postWrite"}));
  // Qualified with the service the bridge was configured for.
  EXPECT_EQ(locked->methodName, "FastThriftServer.echo");
  // Proves enableRequestHeaders reaches the THeader the handlers read.
  EXPECT_TRUE(locked->sawRequestHeader);
  // Proves the synthesized connection context carries the real peer.
  EXPECT_TRUE(locked->sawPeerAddress);
}

// The classic request context the bridge builds reaches the service, so a
// consumer with no fast_thrift equivalent to read — key-client binding, for
// one — has the context it needs rather than nothing.
TEST_F(TProcessorEventHandlerBridgeE2ETest, ServiceSeesItsOwnRequestContext) {
  connect();

  apache::thrift::RpcOptions options;
  options.setWriteHeader("probe", "value");
  pingWith(options);

  auto locked = serviceObserved_.rlock();
  EXPECT_TRUE(locked->sawRequestContext);
  EXPECT_TRUE(locked->sawConnectionContext);
  // The request's headers reach the service, which is what the bridge is
  // configured to populate them for.
  EXPECT_EQ(locked->headerValue, "value");
}

// Every request on one connection is announced once, and the connection
// itself only once — the state the bridge keys by stream id is not leaking
// between them.
TEST_F(TProcessorEventHandlerBridgeE2ETest, ManyRequestsOneConnection) {
  connect();

  for (int i = 0; i < 3; ++i) {
    ping();
  }

  auto locked = observed_.rlock();
  EXPECT_EQ(
      std::count(locked->calls.begin(), locked->calls.end(), "newConnection"),
      1);
  EXPECT_EQ(
      std::count(locked->calls.begin(), locked->calls.end(), "preRead"), 3);
  EXPECT_EQ(
      std::count(locked->calls.begin(), locked->calls.end(), "postWrite"), 3);
}

} // namespace apache::thrift::fast_thrift::thrift::server
