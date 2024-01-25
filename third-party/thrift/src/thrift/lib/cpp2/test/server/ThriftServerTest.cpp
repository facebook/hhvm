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

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <fmt/core.h>

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/protocol/CertUtils.h>
#include <folly/CPortability.h>
#include <folly/Conv.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Memory.h>
#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/SocketAddress.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/executors/MeteredExecutor.h>
#include <folly/experimental/TestUtil.h>
#include <folly/experimental/coro/Baton.h>
#include <folly/experimental/coro/Sleep.h>
#include <folly/experimental/observer/SimpleObservable.h>
#include <folly/io/GlobalShutdownSocketSet.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/test/TestSSLServer.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <folly/synchronization/test/Barrier.h>
#include <folly/system/ThreadName.h>
#include <folly/test/TestUtils.h>
#include <proxygen/httpserver/HTTPServerOptions.h>
#include <quic/client/QuicClientAsyncTransport.h>
#include <quic/common/events/FollyQuicEventBase.h>
#include <quic/common/udpsocket/FollyQuicAsyncUDPSocket.h>
#include <quic/fizz/client/handshake/FizzClientQuicHandshakeContext.h>
#include <thrift/lib/cpp/server/TServerEventHandler.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/HTTPClientChannel.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersClientExtension.h>
#include <thrift/lib/cpp2/server/Cpp2Connection.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/StatusServerInterface.h>
#include <thrift/lib/cpp2/server/ThriftQuicServer.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/DummyStatus.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestServiceAsyncClient.h>
#include <thrift/lib/cpp2/test/util/TestInterface.h>
#include <thrift/lib/cpp2/test/util/TestThriftServerFactory.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>
#include <wangle/acceptor/ServerSocketConfig.h>

using namespace fizz::client;
using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::util;
using namespace apache::thrift::async;
using namespace apache::thrift::transport;
using namespace apache::thrift::concurrency;
using namespace std::literals;
using folly::test::find_resource;
using std::string;

THRIFT_FLAG_DECLARE_bool(server_rocket_upgrade_enabled);
THRIFT_FLAG_DECLARE_bool(rocket_allocating_strategy_parser);
DECLARE_int32(thrift_cpp2_protocol_reader_string_limit);
namespace {
constexpr auto kForcedQueueTimeout = 10ms /*ms*/;
} // namespace

std::unique_ptr<HTTP2RoutingHandler> createHTTP2RoutingHandler(
    ThriftServer& server) {
  auto h2_options = std::make_unique<proxygen::HTTPServerOptions>();
  h2_options->threads = static_cast<size_t>(server.getNumIOWorkerThreads());
  h2_options->idleTimeout = server.getIdleTimeout();
  h2_options->shutdownOn = {SIGINT, SIGTERM};
  return std::make_unique<HTTP2RoutingHandler>(
      std::move(h2_options), server.getThriftProcessor(), server);
}

TEST(ThriftServer, H2ClientAddressTest) {
  class EchoClientAddrTestInterface
      : public apache::thrift::ServiceHandler<TestService> {
    void sync_sendResponse(std::string& _return, int64_t /* size */) override {
      _return = getConnectionContext()->getPeerAddress()->describe();
    }
  };

  ScopedServerInterfaceThread runner(
      std::make_shared<EchoClientAddrTestInterface>());
  auto& thriftServer = dynamic_cast<ThriftServer&>(runner.getThriftServer());
  thriftServer.addRoutingHandler(createHTTP2RoutingHandler(thriftServer));

  folly::EventBase base;
  folly::AsyncSocket::UniquePtr socket(
      new folly::AsyncSocket(&base, runner.getAddress()));
  TestServiceAsyncClient client(
      HTTPClientChannel::newHTTP2Channel(std::move(socket)));
  auto channel =
      boost::polymorphic_downcast<HTTPClientChannel*>(client.getChannel());

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, channel->getTransport()->getLocalAddress().describe());
}

TEST(ThriftServer, OnewayDeferredHandlerTest) {
  class OnewayTestInterface
      : public apache::thrift::ServiceHandler<TestService> {
   public:
    folly::Baton<> done;

    folly::Future<folly::Unit> future_noResponse(int64_t size) override {
      auto executor = getHandlerExecutor();
      auto ctx = getConnectionContext();
      return folly::futures::sleep(std::chrono::milliseconds(size))
          .via(executor)
          .thenValue(
              [ctx](auto&&) { EXPECT_EQ("noResponse", ctx->getMethodName()); })
          .thenValue([this](auto&&) { done.post(); });
    }
  };

  auto handler = std::make_shared<OnewayTestInterface>();
  ScopedServerInterfaceThread runner(handler);

  handler->done.reset();
  auto client = runner.newClient<TestServiceAsyncClient>();
  client->sync_noResponse(100);
  ASSERT_TRUE(handler->done.try_wait_for(std::chrono::seconds(1)));
}

TEST(ThriftServer, ResponseTooBigTest) {
  ScopedServerInterfaceThread runner(std::make_shared<TestInterface>());
  runner.getThriftServer().setMaxResponseSize(4096);
  auto client = runner.newClient<TestServiceAsyncClient>();

  std::string request(4096, 'a');
  std::string response;
  try {
    client->sync_echoRequest(response, request);
    ADD_FAILURE() << "should throw";
  } catch (const TApplicationException& tae) {
    EXPECT_EQ(
        tae.getType(),
        TApplicationException::TApplicationExceptionType::INTERNAL_ERROR);
  } catch (...) {
    ADD_FAILURE() << "unexpected exception thrown";
  }
}

class TestConnCallback : public folly::AsyncSocket::ConnectCallback {
 public:
  void connectSuccess() noexcept override {}

  void connectErr(const folly::AsyncSocketException& ex) noexcept override {
    exception.reset(new folly::AsyncSocketException(ex));
  }

  std::unique_ptr<folly::AsyncSocketException> exception;
};

TEST(ThriftServer, SSLClientOnPlaintextServerTest) {
  TestThriftServerFactory<TestInterface> factory;
  ScopedServerThread sst(factory.create());
  folly::EventBase base;
  auto sslCtx = std::make_shared<folly::SSLContext>();
  auto socket = TAsyncSSLSocket::newSocket(sslCtx, &base);
  TestConnCallback cb;
  socket->connect(&cb, *sst.getAddress());
  base.loop();
  ASSERT_TRUE(cb.exception);
  auto msg = cb.exception->what();
  EXPECT_NE(nullptr, strstr(msg, "unexpected message"));
}

TEST(ThriftServer, DefaultCompressionTest) {
  /* Tests the functionality of default transforms, ensuring the server properly
     applies them even if the client does not apply any transforms. */
  class Callback : public RequestCallback {
   public:
    explicit Callback(bool compressionExpected, uint16_t expectedTransform)
        : compressionExpected_(compressionExpected),
          expectedTransform_(expectedTransform) {}

   private:
    void requestSent() override {}

    void replyReceived(ClientReceiveState&& state) override {
      auto trans = state.header()->getTransforms();
      if (compressionExpected_) {
        EXPECT_EQ(trans.size(), 1);
        for (auto& tran : trans) {
          EXPECT_EQ(tran, expectedTransform_);
        }
      } else {
        EXPECT_EQ(trans.size(), 0);
      }
    }
    void requestError(ClientReceiveState&& state) override {
      state.exception().throw_exception();
    }
    bool compressionExpected_;
    uint16_t expectedTransform_;
  };

  TestThriftServerFactory<TestInterface> factory;
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());
  ScopedServerThread sst(server);
  folly::EventBase base;

  // no compression if client does not compress/send preference
  auto socket = folly::AsyncSocket::newSocket(&base, *sst.getAddress());
  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));
  client.sendResponse(std::make_unique<Callback>(false, 0), 64);
  base.loop();

  // Ensure that client transforms take precedence
  auto channel =
      boost::polymorphic_downcast<ClientChannel*>(client.getChannel());
  apache::thrift::CompressionConfig compressionConfig;
  compressionConfig.codecConfig_ref().ensure().set_zstdConfig();
  channel->setDesiredCompressionConfig(compressionConfig);
  client.sendResponse(
      std::make_unique<Callback>(
          true, apache::thrift::transport::THeader::ZSTD_TRANSFORM),
      64);
  base.loop();
}

TEST(ThriftServer, HeaderTest) {
  TestThriftServerFactory<TestInterface> factory;
  auto serv = factory.create();
  ScopedServerThread sst(serv);
  folly::EventBase base;
  auto socket = folly::AsyncSocket::newSocket(&base, *sst.getAddress());

  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));

  RpcOptions options;
  // Set it as a header directly so the client channel won't set a
  // timeout and the test won't throw TTransportException
  options.setWriteHeader(
      apache::thrift::transport::THeader::CLIENT_TIMEOUT_HEADER,
      folly::to<std::string>(10));
  try {
    client.sync_processHeader(options);
    ADD_FAILURE() << "should timeout";
  } catch (const TApplicationException& e) {
    EXPECT_EQ(
        e.getType(), TApplicationException::TApplicationExceptionType::TIMEOUT);
  }
}

TEST(ThriftServer, ServerEventHandlerTest) {
  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
   public:
    void voidResponse() override {}
  };

  class TestEventHandler : public server::TServerEventHandler {
   public:
    void preServe(const folly::SocketAddress*) { ++preServeCalls; }
    void newConnection(TConnectionContext*) { ++newConnectionCalls; }
    void connectionDestroyed(TConnectionContext*) {
      ++connectionDestroyedCalls;
    }

    ssize_t preServeCalls{0};
    ssize_t newConnectionCalls{0};
    ssize_t connectionDestroyedCalls{0};
  };
  auto eh1 = std::make_shared<TestEventHandler>();
  auto eh2 = std::make_shared<TestEventHandler>();
  auto eh3 = std::make_shared<TestEventHandler>();
  auto eh4 = std::make_shared<TestEventHandler>();

  auto testIf = std::make_shared<TestInterface>();

  auto runner = std::make_unique<ScopedServerInterfaceThread>(
      testIf, "::1", 0, [&](auto& ts) {
        ts.setServerEventHandler(eh1);
        ts.addServerEventHandler(eh2);
        ts.addServerEventHandler(eh3);
        ts.setServerEventHandler(eh4);
      });

  EXPECT_EQ(0, eh1->preServeCalls);
  EXPECT_EQ(1, eh2->preServeCalls);
  EXPECT_EQ(1, eh3->preServeCalls);
  EXPECT_EQ(1, eh4->preServeCalls);

  auto client = runner->newClient<TestServiceAsyncClient>(
      nullptr, [](auto socket) mutable {
        return RocketClientChannel::newChannel(std::move(socket));
      });

  client->semifuture_voidResponse().get();

  EXPECT_EQ(0, eh1->newConnectionCalls);
  EXPECT_EQ(1, eh2->newConnectionCalls);
  EXPECT_EQ(1, eh3->newConnectionCalls);
  EXPECT_EQ(1, eh4->newConnectionCalls);

  runner.reset();

  EXPECT_EQ(0, eh1->connectionDestroyedCalls);
  EXPECT_EQ(1, eh2->connectionDestroyedCalls);
  EXPECT_EQ(1, eh3->connectionDestroyedCalls);
  EXPECT_EQ(1, eh4->connectionDestroyedCalls);
}

TEST(ThriftServer, GetPortTest) {
  ThriftServer server;
  EXPECT_EQ(server.getPort(), 0);

  folly::SocketAddress addr;
  addr.setFromLocalPort(8080);
  server.setAddress(addr);
  EXPECT_EQ(server.getPort(), 8080);

  server.setPort(8090);
  EXPECT_EQ(server.getPort(), 8090);

  server.setAddress(addr);
  EXPECT_EQ(server.getPort(), 8080);
}

namespace {
class ServerErrorCallback : public RequestCallback {
 private:
  struct PrivateCtor {};

 public:
  ServerErrorCallback(
      PrivateCtor,
      folly::fibers::Baton& baton,
      THeader& header,
      folly::exception_wrapper& ew)
      : baton_(baton), header_(header), ew_(ew) {}

  static std::unique_ptr<RequestCallback> create(
      folly::fibers::Baton& baton,
      THeader& header,
      folly::exception_wrapper& ew) {
    return std::make_unique<ServerErrorCallback>(
        PrivateCtor{}, baton, header, ew);
  }
  void requestSent() override {}
  void replyReceived(ClientReceiveState&& state) override {
    header_ = std::move(*state.extractHeader());
    ew_ = TestServiceAsyncClient::recv_wrapped_voidResponse(state);
    baton_.post();
  }
  void requestError(ClientReceiveState&&) override { ADD_FAILURE(); }

 private:
  folly::fibers::Baton& baton_;
  THeader& header_;
  folly::exception_wrapper& ew_;
};

void doLoadHeaderTest(bool isRocket) {
  static constexpr int kEmptyMetricLoad = 12345;

  auto makeClient = [=](auto& runner) {
    if (!isRocket) {
      return runner.template newStickyClient<TestServiceAsyncClient>(
          folly::getGlobalCPUExecutor().get(), [](auto socket) mutable {
            return HeaderClientChannel::newChannel(std::move(socket));
          });
    } else {
      return runner.template newStickyClient<TestServiceAsyncClient>(
          folly::getGlobalCPUExecutor().get(), RocketClientChannel::newChannel);
    }
  };

  auto checkLoadHeader = [](const auto& header,
                            folly::Optional<std::string> loadMetric) {
    auto& headers = header.getHeaders();
    auto load = [&]() -> folly::Optional<int64_t> {
      if (auto value = header.getServerLoad()) {
        return value;
      }
      if (auto* loadPtr = folly::get_ptr(headers, THeader::QUERY_LOAD_HEADER)) {
        return folly::to<int64_t>(*loadPtr);
      }
      return {};
    }();
    ASSERT_EQ(loadMetric.hasValue(), load.has_value());

    if (!loadMetric) {
      return;
    }

    folly::StringPiece loadSp(*loadMetric);
    if (loadSp.removePrefix("custom_load_metric_")) {
      EXPECT_EQ(loadSp, std::to_string(*load));
    } else if (loadSp.empty()) {
      EXPECT_EQ(kEmptyMetricLoad, *load);
    } else {
      FAIL() << "Unexpected load metric";
    }
  };

  class BlockInterface : public apache::thrift::ServiceHandler<TestService> {
   public:
    folly::Optional<folly::Baton<>> block;
    void voidResponse() override {
      if (block) {
        block.value().wait();
      }
    }
  };

  uint32_t nCalls = 0;
  ScopedServerInterfaceThread runner(
      std::make_shared<BlockInterface>(), "::1", 0, [&nCalls](auto& server) {
        server.setGetLoad([](const std::string& metric) {
          folly::StringPiece metricPiece(metric);
          if (metricPiece.removePrefix("custom_load_metric_")) {
            return folly::to<int32_t>(metricPiece.toString());
          } else if (metricPiece.empty()) {
            return kEmptyMetricLoad;
          }
          ADD_FAILURE() << "Unexpected load metric on request";
          return -42;
        });

        server.setIsOverloaded(
            [&nCalls](const auto*, const std::string* method) {
              EXPECT_EQ("voidResponse", *method);
              return ++nCalls == 4;
            });
      });

  auto client = makeClient(runner);

  {
    // No load header
    RpcOptions options;
    auto [_, header] = client->header_semifuture_voidResponse(options).get();
    checkLoadHeader(*header, folly::none);
  }

  {
    // Empty load header
    RpcOptions options;
    const std::string kLoadMetric;
    options.setWriteHeader(THeader::QUERY_LOAD_HEADER, kLoadMetric);
    auto [_, header] = client->header_semifuture_voidResponse(options).get();
    checkLoadHeader(*header, kLoadMetric);
  }

  {
    // Custom load header
    RpcOptions options;
    const std::string kLoadMetric{"custom_load_metric_789"};
    options.setWriteHeader(THeader::QUERY_LOAD_HEADER, kLoadMetric);
    auto [_, header] = client->header_semifuture_voidResponse(options).get();
    checkLoadHeader(*header, kLoadMetric);
  }

  {
    // Force server overload. Load should still be returned on server overload.
    RpcOptions options;
    const std::string kLoadMetric;
    options.setWriteHeader(THeader::QUERY_LOAD_HEADER, kLoadMetric);

    folly::fibers::Baton baton;
    THeader header;
    folly::exception_wrapper ew;
    auto callback = ServerErrorCallback::create(baton, header, ew);
    client->voidResponse(options, std::move(callback));
    baton.wait();

    // Check that request was actually rejected due to server overload
    const bool matched =
        ew.with_exception([](const TApplicationException& tae) {
          ASSERT_EQ(
              TApplicationException::TApplicationExceptionType::LOADSHEDDING,
              tae.getType());
        });

    if (!useResourcePoolsFlagsSet(/* no isOverloaded with resource pools*/)) {
      ASSERT_TRUE(matched);
    }
    checkLoadHeader(header, kLoadMetric);
  }

  {
    // Force queue timeout.
    // for Rocket: load should still be returned
    // for Header: load is not returned because of thread safety concerns.
    auto handler = dynamic_cast<BlockInterface*>(
        runner.getThriftServer().getProcessorFactory().get());
    handler->block.emplace();
    auto fut = client->semifuture_voidResponse();
    auto guard = folly::makeGuard([&] {
      handler->block.value().post();
      std::move(fut).get();
    });
    RpcOptions options;
    const std::string kLoadMetric;
    options.setWriteHeader(THeader::QUERY_LOAD_HEADER, kLoadMetric);
    options.setQueueTimeout(std::chrono::milliseconds(10));

    folly::fibers::Baton baton;
    THeader header;
    folly::exception_wrapper ew;
    auto callback = ServerErrorCallback::create(baton, header, ew);
    client->voidResponse(options, std::move(callback));
    baton.wait();

    // Check that request was actually rejected due to queue timeout
    const bool matched =
        ew.with_exception([](const TApplicationException& tae) {
          ASSERT_EQ(TApplicationException::TIMEOUT, tae.getType());
        });
    ASSERT_TRUE(matched);
    if (isRocket) {
      checkLoadHeader(header, kLoadMetric);
    } else {
      checkLoadHeader(header, folly::none);
    }

    EXPECT_EQ(
        *folly::get_ptr(header.getHeaders(), "ex"),
        kServerQueueTimeoutErrorCode);
  }

  {
    // Force task timeout.
    // for Rocket: load should still be returned
    // for Header: load is not returned because of thread safety concerns.
    auto handler = dynamic_cast<BlockInterface*>(
        runner.getThriftServer().getProcessorFactory().get());
    handler->block.emplace();

    RpcOptions options;
    const std::string kLoadMetric;
    options.setWriteHeader(THeader::QUERY_LOAD_HEADER, kLoadMetric);
    options.setTimeout(std::chrono::seconds(1));

    auto prevTaskExpireTime = runner.getThriftServer().getTaskExpireTime();
    auto prevUseClientTimeout = runner.getThriftServer().getUseClientTimeout();
    runner.getThriftServer().setTaskExpireTime(std::chrono::milliseconds(100));
    runner.getThriftServer().setUseClientTimeout(false);
    auto guard = folly::makeGuard([&] {
      handler->block.value().post();
      runner.getThriftServer().setTaskExpireTime(prevTaskExpireTime);
      runner.getThriftServer().setUseClientTimeout(prevUseClientTimeout);
    });

    folly::fibers::Baton baton;
    THeader header;
    folly::exception_wrapper ew;
    auto callback = ServerErrorCallback::create(baton, header, ew);
    client->voidResponse(options, std::move(callback));
    baton.wait();

    // Check that request was actually rejected due to task timeout
    const bool matched =
        ew.with_exception([](const TApplicationException& tae) {
          ASSERT_EQ(TApplicationException::TIMEOUT, tae.getType());
        });
    ASSERT_TRUE(matched);
    if (isRocket) {
      checkLoadHeader(header, kLoadMetric);
    } else {
      checkLoadHeader(header, folly::none);
    }

    EXPECT_EQ(
        *folly::get_ptr(header.getHeaders(), "ex"), kTaskExpiredErrorCode);
  }
}
} // namespace

TEST(ThriftServer, LoadHeaderTest_HeaderClientChannel) {
  doLoadHeaderTest(false);
}
TEST(ThriftServer, LoadHeaderTest_RocketClientChannel) {
  doLoadHeaderTest(true);
}

//
// Test enforcement of egress memory limit -- setEgressMemoryLimit()
//
// Client issues a series of large requests but does not read their responses.
// The server should queue outgoing responses until the egress limit is reached,
// then drop the client connection along with all undelivered responses.
//
TEST(ThriftServer, EnforceEgressMemoryLimit) {
  class TestServiceHandler
      : public apache::thrift::ServiceHandler<TestService> {
   public:
    // only used to configure the server-side connection socket
    int echoInt(int) override {
      shrinkSocketWriteBuffer();
      return 0;
    }

    void echoRequest(
        std::string& _return, std::unique_ptr<std::string> req) override {
      _return = *std::move(req);
      barrier.wait();
    }

    folly::test::Barrier barrier{2};

   private:
    void shrinkSocketWriteBuffer() {
      auto const_transport =
          getRequestContext()->getConnectionContext()->getTransport();
      auto transport = const_cast<folly::AsyncTransport*>(const_transport);
      auto sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
      sock->setSendBufSize(0); // (smallest possible size)
    }
  };

  // Allocate server
  auto handler = std::make_shared<TestServiceHandler>();
  auto runner = std::make_shared<ScopedServerInterfaceThread>(handler);
  auto& server = runner->getThriftServer();
  auto& thriftServer = dynamic_cast<ThriftServer&>(server);
  const auto kChunkSize = 1ul << 20; // (1 MiB)
  server.setEgressMemoryLimit(kChunkSize * 4); // (4 MiB)
  server.setWriteBatchingInterval(std::chrono::milliseconds::zero());

  // Allocate client
  folly::EventBase evb;
  auto clientSocket = folly::AsyncSocket::newSocket(&evb, runner->getAddress());
  auto clientSocketPtr = clientSocket.get();
  clientSocketPtr->setRecvBufSize(0); // set recv buffer as small as possible
  auto clientChannel = RocketClientChannel::newChannel(std::move(clientSocket));
  auto clientChannelPtr = clientChannel.get();
  TestServiceAsyncClient client(std::move(clientChannel));

  // Dummy request which triggers some server-side socket configuration
  client.sync_echoInt(42);

  // This is used to flush the client write queue
  apache::thrift::rocket::RocketClient::FlushList flushList;
  clientChannelPtr->setFlushList(&flushList);
  auto flushClientWrites = [&]() {
    auto cbs = std::move(flushList);
    while (!cbs.empty()) {
      auto* callback = &cbs.front();
      cbs.pop_front();
      callback->runLoopCallback();
    }
  };

  // Tests whether the client socket is still writable. We avoid reading from
  // the socket because we don't want to flush the server queue.
  auto isClientChannelGood = [&](std::chrono::seconds timeout) -> bool {
    const auto expires = std::chrono::steady_clock::now() + timeout;
    while (clientChannelPtr->good() &&
           std::chrono::steady_clock::now() < expires) {
      client.semifuture_noResponse(0); // (write-only)
      flushClientWrites();
      /* sleep override */ std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return clientChannelPtr->good();
  };

  std::vector<folly::SemiFuture<std::string>> fv;

  // Reach the egress buffer limit. Notice that the server will report a bit
  // less memory than expected because a small portion of the response data will
  // be buffered in the kernel.
  for (size_t b = 0; b + kChunkSize < server.getEgressMemoryLimit();
       b += kChunkSize) {
    std::string data(kChunkSize, 'a');
    fv.emplace_back(client.semifuture_echoRequest(std::move(data)));
    flushClientWrites();
    handler->barrier.wait();
  }

  // The client socket should still be open
  ASSERT_TRUE(isClientChannelGood(std::chrono::seconds(5)));

  ASSERT_GT(thriftServer.getUsedIOMemory().get().egress, 0);

  // The next response should put us over the egress limit
  std::string data(kChunkSize, 'a');
  fv.emplace_back(client.semifuture_echoRequest(std::move(data)));
  flushClientWrites();
  handler->barrier.wait();

  // Wait for the connection to drop
  ASSERT_FALSE(isClientChannelGood(std::chrono::seconds(20)));

  // Start reading again
  clientChannelPtr->setFlushList(nullptr);
  clientSocketPtr->setRecvBufSize(65535);
  auto t = std::thread([&] { evb.loopForever(); });
  SCOPE_EXIT {
    evb.terminateLoopSoon();
    t.join();
  };

  // All responses should have exceptions.
  auto all = folly::collectAll(std::move(fv));
  all.wait();
  ASSERT_TRUE(all.hasValue());
  for (const auto& rsp : std::move(all).get()) {
    EXPECT_TRUE(rsp.hasException());
  }
}

TEST(ThriftServer, SocketWriteTimeout) {
  if (folly::kIsSanitizeThread) {
    GTEST_SUCCEED() << "Disabled in TSAN mode";
    return;
  }

  class TestServiceHandler
      : public apache::thrift::ServiceHandler<TestService> {
   public:
    void echoRequest(
        std::string& _return, std::unique_ptr<std::string>) override {
      _return = std::string(kResponseSize, 'x'); // big response
      barrier.wait();
    }

    folly::test::Barrier barrier{2};

   private:
    const size_t kResponseSize = 10ul << 20;
  };

  const std::chrono::seconds kSocketWriteTimeout{5};

  // Server
  auto handler = std::make_shared<TestServiceHandler>();
  auto runner = std::make_shared<ScopedServerInterfaceThread>(handler);
  auto& server = runner->getThriftServer();
  server.setSocketWriteTimeout(kSocketWriteTimeout);
  server.setWriteBatchingInterval(std::chrono::milliseconds::zero());

  // Client
  folly::EventBase evb;
  auto clientSocket = folly::AsyncSocket::newSocket(&evb, runner->getAddress());
  auto clientChannel = RocketClientChannel::newChannel(std::move(clientSocket));
  auto clientChannelPtr = clientChannel.get();
  TestServiceAsyncClient client(std::move(clientChannel));

  // Establish connection
  client.sync_noResponse(42);

  // This is used to flush the client write queue
  apache::thrift::rocket::RocketClient::FlushList flushList;
  clientChannelPtr->setFlushList(&flushList);
  auto flushClientWrites = [&]() {
    auto cbs = std::move(flushList);
    while (!cbs.empty()) {
      auto* callback = &cbs.front();
      cbs.pop_front();
      callback->runLoopCallback();
    }
  };

  // Issue a series of requests without reading responses. We need enough
  // data to fill both the write buffer on the server and the read buffer on the
  // client, so that the server will be forced to queue the responses.
  std::vector<folly::SemiFuture<std::string>> fv;
  for (auto i = 0; i < 10; i++) {
    fv.emplace_back(client.semifuture_echoRequest("ignored"));
    flushClientWrites();
    handler->barrier.wait();
  }

  // Trigger write timeout on the server
  /* sleep override */ std::this_thread::sleep_for(kSocketWriteTimeout * 2);

  // Start reading responses
  auto t = std::thread([&] { evb.loopForever(); });
  SCOPE_EXIT {
    evb.terminateLoopSoon();
    t.join();
  };

  // All responses should have failed
  auto all = folly::collectAll(std::move(fv));
  all.wait();
  ASSERT_TRUE(all.hasValue());
  for (const auto& rsp : std::move(all).get()) {
    EXPECT_TRUE(rsp.hasException());
  }
}

namespace long_shutdown {
namespace {

class BlockingTestInterface
    : public apache::thrift::ServiceHandler<TestService> {
 public:
  explicit BlockingTestInterface(folly::Baton<>& baton) : baton_(baton) {}

 private:
  void noResponse(int64_t) override {
    baton_.post();
    // Locks up the server and prevents clean shutdown
    /* sleep override */ std::this_thread::sleep_for(
        std::chrono::seconds::max());
  }

  folly::Baton<>& baton_;
};

// Delay on snapshot dumping thread to simulate work being done
std::chrono::milliseconds actualDumpSnapshotDelay = 0ms;
// The advertised max delay for the returned task
std::chrono::milliseconds requestedDumpSnapshotDelay = 0ms;

// Dummy exit code to signal that we did manage to finish the (fake) snapshot
constexpr int kExitCode = 156;

} // namespace
} // namespace long_shutdown

namespace apache::thrift::detail {

THRIFT_PLUGGABLE_FUNC_SET(
    apache::thrift::ThriftServer::DumpSnapshotOnLongShutdownResult,
    dumpSnapshotOnLongShutdown) {
  using namespace long_shutdown;
  return {
      folly::futures::sleep(actualDumpSnapshotDelay).defer([](auto&&) {
        std::quick_exit(kExitCode);
      }),
      requestedDumpSnapshotDelay};
}

} // namespace apache::thrift::detail

TEST(ThriftServerDeathTest, LongShutdown_DumpSnapshot) {
  EXPECT_EXIT(
      ({
        THRIFT_FLAG_SET_MOCK(dump_snapshot_on_long_shutdown, true);
        folly::Baton<> ready;
        ScopedServerInterfaceThread runner(
            std::make_shared<long_shutdown::BlockingTestInterface>(ready),
            [](ThriftServer& server) {
              server.setWorkersJoinTimeout(1s);
              // We need at least 2 cpu threads for the test
              server.setNumCPUWorkerThreads(2);
              server.setThreadManagerType(
                  apache::thrift::BaseThriftServer::ThreadManagerType::SIMPLE);
              server.setThreadFactory(std::make_shared<PosixThreadFactory>(
                  PosixThreadFactory::ATTACHED));
            });

        long_shutdown::requestedDumpSnapshotDelay = 1s;
        long_shutdown::actualDumpSnapshotDelay = 0ms;

        auto client = runner.newClient<TestServiceAsyncClient>();
        client->semifuture_noResponse(0).get();
        ready.wait();
      }),
      testing::ExitedWithCode(long_shutdown::kExitCode),
      "");
}

TEST(ThriftServerDeathTest, LongShutdown_DumpSnapshotTimeout) {
  EXPECT_DEATH(
      ({
        THRIFT_FLAG_SET_MOCK(dump_snapshot_on_long_shutdown, true);
        folly::Baton<> ready;
        ScopedServerInterfaceThread runner(
            std::make_shared<long_shutdown::BlockingTestInterface>(ready),
            [](ThriftServer& server) {
              server.setWorkersJoinTimeout(1s);
              // We need at least 2 cpu threads for the test
              server.setNumCPUWorkerThreads(2);
              server.setThreadManagerType(
                  apache::thrift::BaseThriftServer::ThreadManagerType::SIMPLE);
              server.setThreadFactory(std::make_shared<PosixThreadFactory>(
                  PosixThreadFactory::ATTACHED));
            });

        long_shutdown::requestedDumpSnapshotDelay = 500ms;
        long_shutdown::actualDumpSnapshotDelay = 60s;

        auto client = runner.newClient<TestServiceAsyncClient>();
        client->semifuture_noResponse(0).get();
        ready.wait();
      }),
      "Could not drain active requests within allotted deadline");
}

TEST(ThriftServerDeathTest, OnStopRequestedException) {
  class ThrowExceptionInterface
      : public apache::thrift::ServiceHandler<TestService> {
   public:
    folly::coro::Task<void> co_onStopRequested() override {
      throw std::runtime_error("onStopRequested");
    }
  };

  EXPECT_DEATH(
      ({
        ScopedServerInterfaceThread runner(
            std::make_shared<ThrowExceptionInterface>());
      }),
      "onStopRequested");
}

namespace {
enum class TransportType { Header, Rocket };
enum class Compression { Enabled, Disabled };
enum class ErrorType {
  Overload,
  AppOverload,
  MethodOverload,
  Client,
  Server,
  PreprocessorOverload,
};
} // namespace

class HeaderOrRocketTest : public testing::Test {
 public:
  TransportType transport = TransportType::Rocket;
  Compression compression = Compression::Enabled;
  auto makeStickyClient(
      ScopedServerInterfaceThread& runner, folly::EventBase* evb) {
    return runner.newStickyClient<TestServiceAsyncClient>(
        evb,
        [&](auto socket) mutable { return makeChannel(std::move(socket)); });
  }
  auto makeClient(ScopedServerInterfaceThread& runner, folly::EventBase* evb) {
    return runner.newClient<TestServiceAsyncClient>(
        evb,
        [&](auto socket) mutable { return makeChannel(std::move(socket)); });
  }

  ClientChannel::Ptr makeChannel(folly::AsyncTransport::UniquePtr socket) {
    auto channel = [&]() -> ClientChannel::Ptr {
      if (transport == TransportType::Header) {
        return HeaderClientChannel::newChannel(std::move(socket));
      } else {
        return RocketClientChannel::newChannel(std::move(socket));
      }
    }();
    if (compression == Compression::Enabled) {
      apache::thrift::CompressionConfig compressionConfig;
      compressionConfig.codecConfig_ref().ensure().set_zstdConfig();
      channel->setDesiredCompressionConfig(compressionConfig);
    }
    return channel;
  }
};

class OverloadTest
    : public HeaderOrRocketTest,
      public ::testing::WithParamInterface<
          std::tuple<TransportType, Compression, ErrorType, bool>> {
 public:
  ErrorType errorType;

  bool isCustomError() {
    return errorType == ErrorType::Client || errorType == ErrorType::Server;
  }

  bool useQueueConcurrency() {
    return useResourcePoolsFlagsSet() &&
        THRIFT_FLAG(enforce_queue_concurrency_resource_pools);
  }

  TApplicationException::TApplicationExceptionType getShedError() {
    if (isCustomError()) {
      return TApplicationException::UNKNOWN;
    } else if (
        errorType == ErrorType::PreprocessorOverload ||
        errorType == ErrorType::AppOverload ||
        errorType == ErrorType::MethodOverload) {
      return TApplicationException::LOADSHEDDING;
    } else if (useQueueConcurrency()) {
      return TApplicationException::TIMEOUT;
    } else {
      return TApplicationException::LOADSHEDDING;
    }
  }

  std::string getShedMessage() {
    if (isCustomError()) {
      return "message";
    } else if (errorType == ErrorType::PreprocessorOverload) {
      return "preprocess load shedding";
    } else if (errorType == ErrorType::AppOverload) {
      return "load shedding due to custom isOverloaded() callback";
    } else if (errorType == ErrorType::MethodOverload) {
      return "method loadshedding request";
    } else if (useQueueConcurrency()) {
      return "Queue Timeout";
    } else {
      return "load shedding due to max request limit";
    }
  }

  void validateErrorHeaders(const RpcOptions& rpc) {
    auto& headers = rpc.getReadHeaders();
    if (errorType == ErrorType::Client) {
      EXPECT_EQ(*folly::get_ptr(headers, "ex"), kAppClientErrorCode);
      EXPECT_EQ(*folly::get_ptr(headers, "uex"), "name");
      EXPECT_EQ(*folly::get_ptr(headers, "uexw"), "message");
    } else if (errorType == ErrorType::Server) {
      EXPECT_EQ(*folly::get_ptr(headers, "uex"), "name");
      EXPECT_EQ(*folly::get_ptr(headers, "uexw"), "message");
    } else if (
        errorType == ErrorType::AppOverload ||
        errorType == ErrorType::PreprocessorOverload) {
      EXPECT_EQ(*folly::get_ptr(headers, "ex"), kAppOverloadedErrorCode);
      EXPECT_EQ(folly::get_ptr(headers, "uex"), nullptr);
      EXPECT_EQ(folly::get_ptr(headers, "uexw"), nullptr);
    } else if (errorType == ErrorType::MethodOverload) {
      EXPECT_EQ(*folly::get_ptr(headers, "ex"), kAppOverloadedErrorCode);
      EXPECT_EQ(folly::get_ptr(headers, "uex"), nullptr);
      EXPECT_EQ(folly::get_ptr(headers, "uexw"), nullptr);
    } else if (errorType == ErrorType::Overload && !useQueueConcurrency()) {
      EXPECT_EQ(*folly::get_ptr(headers, "ex"), kOverloadedErrorCode);
      EXPECT_EQ(folly::get_ptr(headers, "uex"), nullptr);
      EXPECT_EQ(folly::get_ptr(headers, "uexw"), nullptr);
    } else if (errorType == ErrorType::Overload && useQueueConcurrency()) {
      EXPECT_EQ(*folly::get_ptr(headers, "ex"), kServerQueueTimeoutErrorCode);
      EXPECT_EQ(folly::get_ptr(headers, "uex"), nullptr);
      EXPECT_EQ(folly::get_ptr(headers, "uexw"), nullptr);
    } else {
      FAIL() << "Unknown error type: " << (int)errorType;
    }
  }

  void SetUp() override {
    bool concurrencyFlag;
    std::tie(transport, compression, errorType, concurrencyFlag) = GetParam();

    if (concurrencyFlag) {
      THRIFT_FLAG_SET_MOCK(enforce_queue_concurrency_resource_pools, true);
    } else {
      THRIFT_FLAG_SET_MOCK(enforce_queue_concurrency_resource_pools, false);
    }
  }
};

class HeaderOrRocket : public HeaderOrRocketTest,
                       public ::testing::WithParamInterface<TransportType> {
 public:
  void SetUp() override { transport = GetParam(); }
};

TEST_P(HeaderOrRocket, OnewayClientConnectionCloseTest) {
  static folly::Baton baton;

  class OnewayTestInterface
      : public apache::thrift::ServiceHandler<TestService> {
    void noResponse(int64_t) override { baton.post(); }
  };

  ScopedServerInterfaceThread runner(std::make_shared<OnewayTestInterface>());
  {
    auto client = makeClient(runner, nullptr);
    client->sync_noResponse(0);
  }
  bool posted = baton.try_wait_for(1s);
  EXPECT_TRUE(posted);
}

TEST_P(HeaderOrRocket, ResourcePoolSetWorkers) {
  static folly::Baton enterBaton;
  static folly::Baton exitBaton;

  class OnewayTestInterface
      : public apache::thrift::ServiceHandler<TestService> {
    void noResponse(int64_t) override {
      enterBaton.post();
      exitBaton.wait();
    }
  };

  ScopedServerInterfaceThread runner(std::make_shared<OnewayTestInterface>());
  // Only test in the presence of resource pools
  if (!runner.getThriftServer().resourcePoolSet().empty()) {
    auto client = makeClient(runner, nullptr);
    EXPECT_EQ(runner.getThriftServer().resourcePoolSet().idleWorkerCount(), 1);
    EXPECT_EQ(runner.getThriftServer().resourcePoolSet().workerCount(), 1);
    client->sync_noResponse(0);
    enterBaton.wait();
    // The above call blocks so we expect our idle worker count to reflect that.
    EXPECT_EQ(runner.getThriftServer().resourcePoolSet().idleWorkerCount(), 0);
    exitBaton.post();
  }
}

TEST_P(HeaderOrRocket, RequestParamsNullCheck) {
  class TestInterfaceSF : public apache::thrift::ServiceHandler<TestService> {
    folly::SemiFuture<folly::Unit> semifuture_voidResponse() override {
      EXPECT_NE(getRequestContext(), nullptr);
      EXPECT_NE(getHandlerExecutor(), nullptr);
      EXPECT_NE(getEventBase(), nullptr);
      return folly::makeSemiFuture().deferValue([this](folly::Unit) {
        EXPECT_EQ(getRequestContext(), nullptr);
        EXPECT_EQ(getHandlerExecutor(), nullptr);
        EXPECT_EQ(getEventBase(), nullptr);
      });
    }
  };

  class TestInterfaceCoro : public apache::thrift::ServiceHandler<TestService> {
    folly::coro::Task<void> co_voidResponse() override {
      EXPECT_NE(getRequestContext(), nullptr);
      EXPECT_NE(getHandlerExecutor(), nullptr);
      EXPECT_NE(getEventBase(), nullptr);
      co_await folly::coro::co_reschedule_on_current_executor;
      EXPECT_EQ(getRequestContext(), nullptr);
      EXPECT_EQ(getHandlerExecutor(), nullptr);
      EXPECT_EQ(getEventBase(), nullptr);
    }
  };
  {
    ScopedServerInterfaceThread runner(std::make_shared<TestInterfaceSF>());
    auto client = makeClient(runner, nullptr);
    client->semifuture_voidResponse().get();
  }
  {
    ScopedServerInterfaceThread runner(std::make_shared<TestInterfaceCoro>());
    auto client = makeClient(runner, nullptr);
    client->semifuture_voidResponse().get();
  }
}

TEST_P(HeaderOrRocket, OnewayQueueTimeTest) {
  static folly::Baton running, finished;
  static folly::Baton running2;

  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
    void voidResponse() override {
      static int once;
      EXPECT_EQ(once++, 0);
      running.post();
      finished.wait();
    }
    void noResponse(int64_t) override { running2.post(); }
  };

  ScopedServerInterfaceThread runner(std::make_shared<TestInterface>());
  runner.getThriftServer().setQueueTimeout(100ms);

  auto client = makeClient(runner, nullptr);

  auto first = client->semifuture_voidResponse();
  EXPECT_TRUE(running.try_wait_for(1s));
  auto second = client->semifuture_noResponse(0);
  EXPECT_THROW(
      client->sync_voidResponse(RpcOptions{}.setTimeout(1s)),
      TApplicationException);
  finished.post();
  // even though 3rd request was loaded shedded, the second is oneway
  // and should have went through
  EXPECT_TRUE(running2.try_wait_for(1s));
}

TEST_P(HeaderOrRocket, Priority) {
  int callCount{0};
  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
    int& callCount_;

   public:
    explicit TestInterface(int& callCount) : callCount_(callCount) {}
    void priorityHigh() override {
      callCount_++;
      EXPECT_EQ(
          getConnectionContext()->getRequestExecutionScope().getPriority(),
          PRIORITY::HIGH);
    }
    void priorityBestEffort() override {
      callCount_++;
      EXPECT_EQ(
          getConnectionContext()->getRequestExecutionScope().getPriority(),
          PRIORITY::BEST_EFFORT);
    }
  };

  ScopedServerInterfaceThread runner(
      std::make_shared<TestInterface>(callCount));
  folly::EventBase base;
  auto client = makeClient(runner, &base);
  client->sync_priorityHigh();
  client->sync_priorityBestEffort();
  EXPECT_EQ(callCount, 2);
}

namespace {
template <typename F, typename Duration = decltype(1s)>
bool blockWhile(F&& f, Duration duration = 1s) {
  auto now = std::chrono::steady_clock::now();
  while (f()) {
    if (std::chrono::steady_clock::now() > now + duration) {
      return false;
    }
    std::this_thread::yield();
  }
  return true;
}
} // namespace

TEST_P(HeaderOrRocket, ThreadManagerAdapterOverSimpleTMUpstreamPriorities) {
  THRIFT_FLAG_SET_MOCK(allow_set_thread_manager_resource_pools, true);
  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
   public:
    TestInterface() {}
    void voidResponse() override { callback_(getHandlerExecutor()); }
    folly::Function<void(folly::Executor*)> callback_;
  };

  auto handler = std::make_shared<TestInterface>();
  ScopedServerInterfaceThread runner(handler, "::1", 0, [&](auto& ts) {
    // empty executor will default to SimpleThreadManager
    auto executor = ThreadManager::newSimpleThreadManager("tm", 1);
    auto tm = std::make_shared<ThreadManagerExecutorAdapter>(executor);
    auto tf =
        std::make_shared<PosixThreadFactory>(PosixThreadFactory::ATTACHED);
    tm->threadFactory(tf);
    tm->start();
    ts.setThreadManager(tm);
  });
  std::make_shared<TestInterface>();
  auto client = makeClient(runner, nullptr);

  folly::Baton baton1;
  folly::Baton baton2;
  // First request blocks the CPU thread, then waits for more requests to be
  // scheduled from upstream, then schedules internal task on the same thread.
  // This internal task should execute before upstream tasks.
  int testCounter = 0;
  handler->callback_ = [&](auto executor) {
    baton1.post();
    baton2.wait();
    EXPECT_EQ(0, testCounter++);
    executor->add([&] { EXPECT_EQ(1, testCounter++); });
  };
  client->semifuture_voidResponse();
  baton1.wait();

  handler->callback_ = [&](auto) { EXPECT_EQ(2, testCounter++); };
  auto tm = runner.getThriftServer().getThreadManager_deprecated();
  auto normalPriExecutor = dynamic_cast<ThreadManager*>(
      tm->getKeepAlive(NORMAL, ThreadManager::Source::INTERNAL).get());
  auto req2 = client->semifuture_voidResponse();
  EXPECT_TRUE(blockWhile(
      [&] { return normalPriExecutor->pendingUpstreamTaskCount() != 1; }));
  baton2.post();

  req2.wait();
  EXPECT_EQ(3, testCounter);
}

TEST_P(
    HeaderOrRocket, ThreadManagerAdapterOverMeteredExecutorUpstreamPriorities) {
  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
   public:
    explicit TestInterface(int& testCounter) : testCounter_(testCounter) {}
    void voidResponse() override { callback_(getHandlerExecutor()); }
    int echoInt(int value) override {
      EXPECT_EQ(value, testCounter_++);
      return value;
    }
    folly::Function<void(folly::Executor*)> callback_;
    int& testCounter_;
  };

  int testCounter = 0;
  auto handler = std::make_shared<TestInterface>(testCounter);
  ScopedServerInterfaceThread runner(handler, "::1", 0, [&](auto& ts) {
    auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(
        1, std::make_shared<folly::NamedThreadFactory>("cpu"));
    auto tm = std::make_shared<ThreadManagerExecutorAdapter>(executor);
    tm->setNamePrefix("cpu");
    tm->threadFactory(
        std::make_shared<PosixThreadFactory>(PosixThreadFactory::ATTACHED));
    tm->start();
    ts.setThreadManager(tm);
  });
  auto client = makeClient(runner, nullptr);

  folly::Baton baton1;
  folly::Baton baton2;
  // First request blocks the CPU thread, then waits for more requests to be
  // scheduled from upstream, then schedules internal tasks on the same thread.
  // These internal tasks should execute before upstream tasks, except for the
  // first upstream task (because this is how metered scheduling works).
  handler->callback_ = [&](auto executor) {
    baton1.post();
    baton2.wait();
    EXPECT_EQ(0, testCounter++);
    executor->add([&] { EXPECT_EQ(2, testCounter++); });
    executor->add([&] { EXPECT_EQ(3, testCounter++); });
    executor->add([&] { EXPECT_EQ(4, testCounter++); });
  };
  client->semifuture_voidResponse();
  baton1.wait();

  // While first request is blocking the CPU thread, send more requests to
  // the server, and make sure they are added to the upstream queue.
  std::vector<folly::SemiFuture<int>> requests;
  auto sendRequestAndWait = [&](int expectedValue) {
    auto tm = runner.getThriftServer().getThreadManager_deprecated();
    auto upstreamQueue = dynamic_cast<folly::MeteredExecutor*>(
        tm->getKeepAlive(NORMAL, ThreadManager::Source::UPSTREAM).get());
    auto numPendingReqs = upstreamQueue->pendingTasks();
    requests.emplace_back(client->semifuture_echoInt(expectedValue));
    numPendingReqs++;
    EXPECT_TRUE(blockWhile(
        [&] { return upstreamQueue->pendingTasks() != numPendingReqs; }));
  };
  sendRequestAndWait(1);
  sendRequestAndWait(5);
  sendRequestAndWait(6);
  sendRequestAndWait(7);

  baton2.post();
  for (auto& req : requests) {
    req.wait();
  }
  EXPECT_EQ(8, testCounter);
}

TEST_P(HeaderOrRocket, ThreadManagerAdapterManyPools) {
  int callCount{0};
  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
    int& callCount_;
    std::array<std::array<std::string, 3>, 2> prefixes = {
        {{"tm-1-", "tm-4-", "cpu0"}, {"tm.H", "tm.BE", "tm.N"}}};

   public:
    explicit TestInterface(int& callCount) : callCount_(callCount) {}
    void priorityHigh() override {
      callCount_++;
      EXPECT_THAT(
          *folly::getCurrentThreadName(),
          testing::StartsWith(prefixes[useResourcePoolsFlagsSet() ? 1 : 0][0]));
    }
    void priorityBestEffort() override {
      callCount_++;
      EXPECT_THAT(
          *folly::getCurrentThreadName(),
          testing::StartsWith(prefixes[useResourcePoolsFlagsSet() ? 1 : 0][1]));
    }
    void voidResponse() override {
      callCount_++;
      EXPECT_THAT(
          *folly::getCurrentThreadName(),
          testing::StartsWith(prefixes[useResourcePoolsFlagsSet() ? 1 : 0][2]));
    }
  };

  ScopedServerInterfaceThread runner(
      std::make_shared<TestInterface>(callCount), "::1", 0, [](auto& ts) {
        if (!useResourcePoolsFlagsSet()) {
          auto tm = std::shared_ptr<ThreadManagerExecutorAdapter>(
              new ThreadManagerExecutorAdapter(
                  {nullptr,
                   nullptr,
                   nullptr,
                   std::make_shared<folly::CPUThreadPoolExecutor>(
                       1, std::make_shared<folly::NamedThreadFactory>("cpu")),
                   nullptr}));
          tm->setNamePrefix("tm");
          tm->threadFactory(std::make_shared<PosixThreadFactory>(
              PosixThreadFactory::ATTACHED));
          tm->start();
          ts.setThreadManager(tm);
        } else {
          ts.setCPUWorkerThreadName("tm");
          ts.setThreadManagerType(
              apache::thrift::BaseThriftServer::ThreadManagerType::PRIORITY);
          // Just allow the defaults to happen
        }
      });
  folly::EventBase base;
  auto client = makeClient(runner, &base);
  client->sync_priorityHigh();
  client->sync_priorityBestEffort();
  client->sync_voidResponse();
  EXPECT_EQ(callCount, 3);
}

TEST_P(HeaderOrRocket, ThreadManagerAdapterSinglePool) {
  int callCount{0};
  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
    int& callCount_;
    std::array<std::string, 2> threadNames{{"cpu0", "cpu.N0"}};

   public:
    explicit TestInterface(int& callCount) : callCount_(callCount) {}
    void priorityHigh() override {
      callCount_++;
      EXPECT_EQ(
          threadNames[useResourcePoolsFlagsSet() ? 1 : 0],
          *folly::getCurrentThreadName());
    }
    void priorityBestEffort() override {
      callCount_++;
      EXPECT_EQ(
          threadNames[useResourcePoolsFlagsSet() ? 1 : 0],
          *folly::getCurrentThreadName());
    }
    void voidResponse() override {
      callCount_++;
      EXPECT_EQ(
          threadNames[useResourcePoolsFlagsSet() ? 1 : 0],
          *folly::getCurrentThreadName());
    }
  };

  ScopedServerInterfaceThread runner(
      std::make_shared<TestInterface>(callCount), "::1", 0, [](auto& ts) {
        if (!useResourcePoolsFlagsSet()) {
          auto tm = std::shared_ptr<ThreadManagerExecutorAdapter>(
              new ThreadManagerExecutorAdapter(
                  std::make_shared<folly::CPUThreadPoolExecutor>(
                      1, std::make_shared<folly::NamedThreadFactory>("cpu"))));
          tm->setNamePrefix("tm");
          tm->threadFactory(std::make_shared<PosixThreadFactory>(
              PosixThreadFactory::ATTACHED));
          tm->start();
          ts.setThreadManager(tm);
        } else {
          ts.setThreadManagerType(
              apache::thrift::BaseThriftServer::ThreadManagerType::SIMPLE);
          ts.setCPUWorkerThreadName("cpu");
          ts.setNumCPUWorkerThreads(1);
        }
      });
  folly::EventBase base;
  auto client = makeClient(runner, &base);
  client->sync_priorityHigh();
  client->sync_priorityBestEffort();
  client->sync_voidResponse();
  EXPECT_EQ(callCount, 3);
}

TEST_P(HeaderOrRocket, StickyToThreadPool) {
  THRIFT_FLAG_SET_MOCK(allow_set_thread_manager_resource_pools, true);
  int callCount{0};
  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
    int& callCount_;

   public:
    explicit TestInterface(int& callCount) : callCount_(callCount) {}
    folly::SemiFuture<folly::Unit> semifuture_priorityHigh() override {
      EXPECT_THAT(
          *folly::getCurrentThreadName(), testing::StartsWith("foo-pri1"));
      return folly::makeSemiFuture().defer([=](auto&&) {
        callCount_++;
        EXPECT_THAT(
            *folly::getCurrentThreadName(), testing::StartsWith("foo-pri1"));
      });
    }
    folly::coro::Task<void> co_priorityBestEffort() override {
      callCount_++;
      EXPECT_THAT(
          *folly::getCurrentThreadName(), testing::StartsWith("foo-pri4"));
      co_await folly::coro::co_reschedule_on_current_executor;
      callCount_++;
      EXPECT_THAT(
          *folly::getCurrentThreadName(), testing::StartsWith("foo-pri4"));
      co_return;
    }
  };

  ScopedServerInterfaceThread runner(
      std::make_shared<TestInterface>(callCount), "::1", 0, [](auto& ts) {
        auto tm = PriorityThreadManager::newPriorityThreadManager(1);
        tm->setNamePrefix("foo");
        tm->start();
        ts.setThreadManager(tm);
      });
  folly::EventBase base;
  auto client = makeClient(runner, &base);
  client->sync_priorityHigh();
  client->sync_priorityBestEffort();
  EXPECT_EQ(callCount, 3);
}

TEST_P(HeaderOrRocket, CancellationTest) {
  class NotCalledBackHandler {
   public:
    explicit NotCalledBackHandler(
        std::unique_ptr<HandlerCallback<void>> callback)
        : thriftCallback_{std::move(callback)},
          cancelCallback_(
              thriftCallback_->getConnectionContext()
                  ->getConnectionContext()
                  ->getCancellationToken(),
              [this]() { requestCancelled(); }) {}

    folly::Baton<> cancelBaton;

   private:
    void requestCancelled() {
      // Invoke the thrift callback once the request has canceled.
      // Even after the request has been canceled handlers still should
      // eventually invoke the request callback.
      std::exchange(thriftCallback_, nullptr)
          ->exception(std::runtime_error("request cancelled"));
      cancelBaton.post();
    }

    std::unique_ptr<HandlerCallback<void>> thriftCallback_;
    folly::CancellationCallback cancelCallback_;
  };

  class NotCalledBackInterface
      : public apache::thrift::ServiceHandler<TestService> {
   public:
    using NotCalledBackHandlers =
        std::vector<std::shared_ptr<NotCalledBackHandler>>;

    void async_tm_notCalledBack(
        std::unique_ptr<HandlerCallback<void>> cb) override {
      auto handler = std::make_shared<NotCalledBackHandler>(std::move(cb));
      notCalledBackHandlers_.lock()->push_back(std::move(handler));
      handlersCV_.notify_one();
    }

    /**
     * Get all handlers for currently pending notCalledBack() thrift calls.
     *
     * If there is no call currently pending this function will wait for up to
     * the specified timeout for one to arrive.  If the timeout expires before a
     * notCalledBack() call is received an empty result set will be returned.
     */
    NotCalledBackHandlers getNotCalledBackHandlers(
        std::chrono::milliseconds timeout) {
      auto end_time = std::chrono::steady_clock::now() + timeout;

      NotCalledBackHandlers results;
      auto handlers = notCalledBackHandlers_.lock();
      if (!handlersCV_.wait_until(handlers.as_lock(), end_time, [&] {
            return !handlers->empty();
          })) {
        // If we get here we timed out.
        // Just return an empty result set in this case.
        return results;
      }
      results.swap(*handlers);
      return results;
    }

   private:
    folly::Synchronized<NotCalledBackHandlers, std::mutex>
        notCalledBackHandlers_;
    std::condition_variable handlersCV_;
  };
  ScopedServerInterfaceThread runner(
      std::make_shared<NotCalledBackInterface>());
  folly::EventBase base;
  auto client = makeClient(runner, &base);

  auto interface = std::dynamic_pointer_cast<NotCalledBackInterface>(
      runner.getThriftServer().getProcessorFactory());
  runner.getThriftServer().setTaskExpireTime(0s);
  ASSERT_TRUE(interface);
  EXPECT_EQ(0, interface->getNotCalledBackHandlers(0s).size());

  // Make a call to notCalledBack(), which will time out since the server never
  // reponds to this API.
  try {
    RpcOptions rpcOptions;
    rpcOptions.setTimeout(std::chrono::milliseconds(10));
    client->sync_notCalledBack(rpcOptions);
    EXPECT_FALSE(true) << "request should have never returned";
  } catch (const TTransportException& tte) {
    if (tte.getType() != TTransportException::TIMED_OUT) {
      throw;
    }
  }

  // Wait for the server to register the call
  auto handlers = interface->getNotCalledBackHandlers(10s);
  ASSERT_EQ(1, handlers.size()) << "expected a single notCalledBack() call";
  auto wasCancelled = handlers[0]->cancelBaton.ready();
  // Currently we do not trigger per-request cancellations, but only
  // when client closes the connection.
  EXPECT_FALSE(wasCancelled);

  // Close the client.  This should trigger request cancellation on the server.
  client.reset();

  // The handler's cancellation token should be triggered when we close the
  // connection.
  wasCancelled = handlers[0]->cancelBaton.try_wait_for(10s);
  EXPECT_TRUE(wasCancelled);
}

TEST_P(HeaderOrRocket, QueueTimeoutOnServerShutdown) {
  class BlockInterface : public apache::thrift::ServiceHandler<TestService> {
   public:
    folly::Baton<> stopEnter, stopExit;

    int count = 0;
    void voidResponse() override {
      if (count == 1) {
        // Wait for the server to start shutdown
        auto worker = getRequestContext()->getConnectionContext()->getWorker();
        auto deadline = std::chrono::steady_clock::now() + 2s;
        while (!worker->isStopping() &&
               std::chrono::steady_clock::now() < deadline) {
          std::this_thread::yield();
        }
        EXPECT_TRUE(worker->isStopping());
      }
      count++;
    }

    folly::coro::Task<void> co_onStopRequested() override {
      stopEnter.post();
      stopExit.wait();
      co_return;
    }
  };

  auto blockIf = std::make_shared<BlockInterface>();
  auto runner = std::make_unique<ScopedServerInterfaceThread>(blockIf);
  if (useResourcePoolsFlagsSet()) {
    // Limit active requests to 1 for this test and ensure we have two threads
    // in the worker pool.
    auto& resourcePool =
        runner->getThriftServer().resourcePoolSet().resourcePool(
            ResourcePoolHandle::defaultAsync());
    resourcePool.concurrencyController()
        .value()
        .get()
        .setExecutionLimitRequests(1);
    dynamic_cast<folly::ThreadPoolExecutor&>(
        resourcePool.executor().value().get())
        .setNumThreads(2);
  }

  auto client = runner->newStickyClient<TestServiceAsyncClient>(
      folly::getGlobalIOExecutor()->getEventBase(),
      [&](auto socket) mutable { return makeChannel(std::move(socket)); });

  // Send a request to establish a connection before stopping server
  client->semifuture_voidResponse().get();

  auto& server = runner->getThriftServer();

  server.stop();

  // We need to send requests after onStopRequested() has started executing as
  // our request will block the threadmanager
  EXPECT_TRUE(blockIf->stopEnter.try_wait_for(2s));

  // In this test, we send 2 requests, so that one request starts executing when
  // onStopRequested completes and the other request gets cancelled.
  auto first = client->semifuture_voidResponse();

  // Wait for the first request to reach the server
  auto deadline = std::chrono::steady_clock::now() + 2s;
  while (server.getActiveRequests() < 1 &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  EXPECT_EQ(server.getActiveRequests(), 1);

  // Send second request
  folly::fibers::Baton baton;
  THeader th;
  folly::exception_wrapper ew;
  client->voidResponse(ServerErrorCallback::create(baton, th, ew));

  // Wait for the second request to reach the server
  deadline = std::chrono::steady_clock::now() + 2s;
  while (server.getActiveRequests() < 2 &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  EXPECT_EQ(server.getActiveRequests(), 2);

  // Resume onStopRequested()
  blockIf->stopExit.post();

  // First request should complete normally
  std::move(first).get();

  // Second request should have been cancelled.
  EXPECT_TRUE(baton.try_wait_for(2s));
  ASSERT_TRUE(ew.with_exception([](const TApplicationException& tae) {
    EXPECT_EQ(TApplicationException::TIMEOUT, tae.getType());
  }));
  EXPECT_EQ(
      *folly::get_ptr(th.getHeaders(), "ex"), kServerQueueTimeoutErrorCode);
}

TEST_P(HeaderOrRocket, ConnectionIdleTimeoutTestSSL) {
  using namespace std::chrono_literals;
  folly::ScopedEventBaseThread clientEvbThread;
  ScopedServerInterfaceThread runner(
      std::make_shared<TestInterface>(), "::1", 0, [](auto& server) {
        server.setIdleTimeout(std::chrono::milliseconds(20));
        auto sslConfig = std::make_shared<wangle::SSLContextConfig>();
        sslConfig->setCertificate(
            find_resource(folly::test::kTestCert).string(),
            find_resource(folly::test::kTestKey).string(),
            "");
        sslConfig->clientCAFiles = std::vector<std::string>{
            find_resource(folly::test::kTestCA).string()};
        sslConfig->sessionContext = "ThriftServerTest";
        sslConfig->setNextProtocols({"rs"});
        server.setSSLConfig(std::move(sslConfig));
      });

  auto client = makeStickyClient(runner, clientEvbThread.getEventBase());
  std::string response;
  std::this_thread::sleep_for(10ms);
  // succeeds because connection is still a live
  EXPECT_NO_THROW(client->sync_sendResponse(response, 200));
  std::this_thread::sleep_for(800ms);
  // throws an exception because connection is dropped
  EXPECT_THROW(client->sync_sendResponse(response, 200), TTransportException);
}

TEST_P(HeaderOrRocket, ConnectionAgeTimeout) {
  using namespace std::chrono_literals;
  folly::ScopedEventBaseThread clientEvbThread;
  ScopedServerInterfaceThread runner(
      std::make_shared<TestInterface>(), "::1", 0, [](auto& server) {
        server.setConnectionAgeTimeout(100ms);
      });

  auto client = makeStickyClient(runner, clientEvbThread.getEventBase());
  std::string response;
  std::this_thread::sleep_for(10ms);
  // succeeds because connection is still a live
  EXPECT_NO_THROW(client->sync_sendResponse(response, 200));
  std::this_thread::sleep_for(200ms);
  // throws an exception because connection is dropped
  EXPECT_THROW(client->sync_sendResponse(response, 200), TTransportException);
}

TEST_P(HeaderOrRocket, ResponseWriteTimeout) {
  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
   public:
    TestInterface() = delete;
    TestInterface(
        folly::Baton<>& requestReceivedBaton,
        folly::Baton<>& callbackInstalledBaton)
        : requestReceivedBaton_{requestReceivedBaton},
          callbackInstalledBaton_{callbackInstalledBaton} {}

    // only used to configure the server-side connection socket
    int sync_echoInt(int) override {
      shrinkSocketSendBuffer();
      return 0;
    }

    void sync_sendResponse(std::string& _return, int64_t block) override {
      if (block > 0) {
        requestReceivedBaton_.post();
        callbackInstalledBaton_.wait();
      }

      // Write a response that is
      // a) much larger than the send buffer, and
      // b) random to mitigate undesired effects of (possible) compression at
      //    lower points in the networking stack
      _return = getRandomString(bufsize_ * 100);
    }

   private:
    void shrinkSocketSendBuffer() {
      auto const_transport =
          getRequestContext()->getConnectionContext()->getTransport();
      auto transport = const_cast<folly::AsyncTransport*>(const_transport);
      auto sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
      sock->setSendBufSize(0); // smallest possible size
      socklen_t bufsizelen = sizeof(bufsize_);
      if (sock->getSockOpt<int>(SOL_SOCKET, SO_SNDBUF, &bufsize_, &bufsizelen) <
          0) {
        FAIL() << "Unable to get server send socket buffer size";
      }
    }

    std::string getRandomString(size_t len) {
      auto randomChar = []() -> char { return 32 + (rand() % 95); };
      std::string str(len, 0);
      std::generate_n(str.begin(), len, randomChar);
      return str;
    }

    int bufsize_{0};
    folly::Baton<>& requestReceivedBaton_;
    folly::Baton<>& callbackInstalledBaton_;
  };

  class SlowReadCallback : public folly::AsyncReader::ReadCallback {
   public:
    SlowReadCallback() = delete;

    explicit SlowReadCallback(
        folly::AsyncSocket* socket,
        size_t maxBytesPerLoop,
        std::chrono::milliseconds sleepMsPerLoop)
        : socket_{socket},
          maxBytesPerLoop_{maxBytesPerLoop},
          sleepMsPerLoop_{sleepMsPerLoop} {}

    void install() {
      socket_->getEventBase()->runImmediatelyOrRunInEventBaseThreadAndWait(
          [this]() {
            wrapped_ = socket_->getReadCallback();
            socket_->setMaxReadsPerEvent(1);
            socket_->setRecvBufSize(0); // smallest possible size
            socket_->setReadCB(this);
          });
    }

    void uninstall() {
      socket_->getEventBase()->runImmediatelyOrRunInEventBaseThreadAndWait(
          [this]() { socket_->setReadCB(wrapped_); });
    }

    void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
      wrapped_->getReadBuffer(bufReturn, lenReturn);
      if (*lenReturn > 0 && maxBytesPerLoop_ > 0) {
        *lenReturn = std::min<size_t>(maxBytesPerLoop_, *lenReturn);
      }
    }
    void getReadBuffers(folly::IOBufIovecBuilder::IoVecVec& iovs) override {
      wrapped_->getReadBuffers(iovs);
    }
    void readDataAvailable(size_t len) noexcept override {
      if (sleepMsPerLoop_ > std::chrono::milliseconds::zero()) {
        std::this_thread::sleep_for(sleepMsPerLoop_);
      }
      wrapped_->readDataAvailable(len);
    }
    ZeroCopyMemStore* readZeroCopyEnabled() noexcept override {
      return wrapped_->readZeroCopyEnabled();
    }
    void getZeroCopyFallbackBuffer(
        void** bufReturn, size_t* lenReturn) noexcept override {
      wrapped_->getZeroCopyFallbackBuffer(bufReturn, lenReturn);
    }
    void readZeroCopyDataAvailable(
        std::unique_ptr<IOBuf>&& zeroCopyData,
        size_t additionalBytes) noexcept override {
      wrapped_->readZeroCopyDataAvailable(
          std::move(zeroCopyData), additionalBytes);
    }
    bool isBufferMovable() noexcept override {
      return wrapped_->isBufferMovable();
    }
    size_t maxBufferSize() const override { return wrapped_->maxBufferSize(); }
    void readBufferAvailable(std::unique_ptr<IOBuf> readBuf) noexcept override {
      wrapped_->readBufferAvailable(std::move(readBuf));
    }
    void readEOF() noexcept override { wrapped_->readEOF(); }
    void readErr(const folly::AsyncSocketException& ex) noexcept override {
      wrapped_->readErr(ex);
    }

   private:
    folly::AsyncSocket* socket_;
    folly::AsyncReader::ReadCallback* wrapped_{nullptr};
    size_t maxBytesPerLoop_;
    std::chrono::milliseconds sleepMsPerLoop_;
  };

  folly::Baton<> requestReceivedBaton;
  folly::Baton<> callbackInstalledBaton;
  ScopedServerInterfaceThread ssit(std::make_shared<TestInterface>(
      requestReceivedBaton, callbackInstalledBaton));

  // set maxResponseWriteTime to 1 second
  auto& config =
      apache::thrift::detail::getThriftServerConfig(ssit.getThriftServer());
  config.setMaxResponseWriteTime(folly::observer::makeStaticObserver(
      std::make_optional(std::chrono::milliseconds{1000})));

  std::unique_ptr<SlowReadCallback> readCallback;

  auto client = ssit.newStickyClient<TestServiceAsyncClient>(
      nullptr, [&readCallback, this](auto socket) {
        readCallback =
            std::make_unique<SlowReadCallback>(socket.get(), 1000, 100ms);
        return makeChannel(std::move(socket));
      });

  // Call to shrink server socket buffer
  client->semifuture_echoInt(0).get();

  // Test normal client reads (timeout should not fire)
  auto t1 = client->semifuture_sendResponse(0).getTry();
  EXPECT_FALSE(t1.hasException());

  // Test slow client reads (timeout should fire)
  auto fut = client->semifuture_sendResponse(1);

  /* Header sets the read callback (to a new TAsyncTransportHandler) on the
   * EventBase thread for every request. This code ensures that our
   * SlowReadCallback wraps the active ReadCallback.
   *
   * This process isn't necessary for Rocket, because it uses the same
   * ReadCallback across requests.
   */
  requestReceivedBaton.wait();
  readCallback->install();
  callbackInstalledBaton.post();

  auto t2 = std::move(fut).getTry();
  EXPECT_TRUE(t2.hasException());
}

INSTANTIATE_TEST_CASE_P(
    HeaderOrRocket,
    HeaderOrRocket,
    testing::Values(TransportType::Header, TransportType::Rocket));

TEST_P(OverloadTest, Test) {
  class BlockInterface : public apache::thrift::ServiceHandler<TestService> {
   public:
    folly::Baton<> block;
    void voidResponse() override { block.wait(); }

    void async_eb_eventBaseAsync(
        std::unique_ptr<HandlerCallback<std::unique_ptr<::std::string>>>
            callback) override {
      callback->appOverloadedException("method loadshedding request");
    }
  };

  ScopedServerInterfaceThread runner(std::make_shared<BlockInterface>());
  folly::EventBase base;
  auto client = makeClient(runner, &base);

  runner.getThriftServer().setIsOverloaded(
      [&](const auto*, const string* method) {
        if (errorType == ErrorType::AppOverload) {
          EXPECT_EQ("voidResponse", *method);
          return true;
        }
        return false;
      });

  runner.getThriftServer().setPreprocess([&](auto&) -> PreprocessResult {
    if (errorType == ErrorType::Client) {
      return {AppClientException("name", "message")};
    } else if (errorType == ErrorType::Server) {
      return {AppServerException("name", "message")};
    } else if (errorType == ErrorType::PreprocessorOverload) {
      return {AppOverloadedException("name", "preprocess load shedding")};
    }
    return {};
  });

  // force overloaded
  folly::Function<void()> onExit = [] {};
  auto guard = folly::makeGuard([&] { onExit(); });
  if (errorType == ErrorType::Overload) {
    // Thrift is overloaded on max requests
    runner.getThriftServer().setMaxRequests(1);
    runner.getThriftServer().setQueueTimeout(kForcedQueueTimeout);
    auto handler = dynamic_cast<BlockInterface*>(
        runner.getThriftServer().getProcessorFactory().get());
    client->semifuture_voidResponse();
    while (runner.getThriftServer().getActiveRequests() < 1) {
      std::this_thread::yield();
    }
    onExit = [handler] { handler->block.post(); };
  }

  RpcOptions rpcOptions;
  rpcOptions.setWriteHeader(kClientLoggingHeader.str(), "");
  try {
    if (errorType == ErrorType::MethodOverload) {
      std::string dummy;
      client->sync_eventBaseAsync(rpcOptions, dummy);
    } else {
      client->sync_voidResponse(rpcOptions);
    }
    FAIL() << "Expected that the service call throws TApplicationException";
  } catch (const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(getShedError(), ex.getType());
    EXPECT_TRUE(ex.getMessage().find(getShedMessage()) != std::string::npos);

    validateErrorHeaders(rpcOptions);

  } catch (...) {
    FAIL()
        << "Expected that the service call throws TApplicationException, got "
        << folly::exceptionStr(std::current_exception());
  }
}

INSTANTIATE_TEST_CASE_P(
    OverloadTestsFixture,
    OverloadTest,
    ::testing::Combine(
        testing::Values(TransportType::Header, TransportType::Rocket),
        testing::Values(Compression::Enabled, Compression::Disabled),
        testing::Values(
            ErrorType::Overload,
            ErrorType::MethodOverload,
            ErrorType::AppOverload,
            ErrorType::Client,
            ErrorType::Server,
            ErrorType::PreprocessorOverload),
        testing::Bool()));

TEST(ThriftServer, QueueTimeoutPctSetTest) {
  ScopedServerInterfaceThread runner(std::make_shared<TestInterface>());
  auto client =
      runner.newStickyClient<TestServiceAsyncClient>(nullptr, [](auto socket) {
        return HeaderClientChannel::newChannel(std::move(socket));
      });

  // setup a small queue timeout which will be overrided by QueueTimeoutPct.
  runner.getThriftServer().setQueueTimeout(std::chrono::milliseconds(5));
  runner.getThriftServer().setQueueTimeoutPct(50);

  // Run a long request.
  auto slowRequestFuture = client->semifuture_sendResponse(20000);

  RpcOptions rpcOptions;
  // QueueTimeout will be set to 100ms.
  rpcOptions.setTimeout(std::chrono::milliseconds(200));

  std::string response;
  try {
    client->sync_sendResponse(rpcOptions, response, 1000);
  } catch (const TApplicationException&) {
    FAIL() << "Should not trigger timeout.";
  }
  // Run another long request.
  auto slowRequestFuture1 = client->semifuture_sendResponse(20000);
  // Client Timeout not set, setQueueTimeoutPct will not be effective.
  RpcOptions rpcOptions1;
  std::string response1;
  EXPECT_ANY_THROW(client->sync_sendResponse(rpcOptions1, response1, 1000));

  folly::EventBase base;
  std::move(slowRequestFuture).via(&base).getVia(&base);
  std::move(slowRequestFuture1).via(&base).getVia(&base);
}

TEST(ThriftServer, QueueTimeoutDisabledTest) {
  ScopedServerInterfaceThread runner(std::make_shared<TestInterface>());
  auto client =
      runner.newStickyClient<TestServiceAsyncClient>(nullptr, [](auto socket) {
        return HeaderClientChannel::newChannel(std::move(socket));
      });

  // setup queuetimeout to 0 disables queuetimeout.
  runner.getThriftServer().setQueueTimeout(std::chrono::milliseconds(0));
  runner.getThriftServer().setQueueTimeoutPct(10);

  // Run a long request.
  auto slowRequestFuture = client->semifuture_sendResponse(20000);

  RpcOptions rpcOptions;

  // QueueTimeout will be set to 1ms if not disabled. However, since it was
  // disabled by setQueueTimeout above, queue timeout won't be triggerd.
  rpcOptions.setTimeout(std::chrono::milliseconds(10));

  std::string response;
  EXPECT_ANY_THROW(client->sync_sendResponse(rpcOptions, response, 1000));

  folly::EventBase base;
  std::move(slowRequestFuture).via(&base).getVia(&base);
}

TEST(ThriftServer, ClientTimeoutTest) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = factory.create();
  ScopedServerThread sst(server);
  folly::EventBase base;

  auto getClient = [&base, &sst]() {
    auto socket = folly::AsyncSocket::newSocket(&base, *sst.getAddress());

    return std::make_shared<TestServiceAsyncClient>(
        HeaderClientChannel::newChannel(std::move(socket)));
  };

  int cbCtor = 0;
  int cbCall = 0;

  auto callback = [&cbCall, &cbCtor](
                      std::shared_ptr<TestServiceAsyncClient> client,
                      bool& timeout) {
    cbCtor++;
    return std::unique_ptr<RequestCallback>(new FunctionReplyCallback(
        [&cbCall, client, &timeout](ClientReceiveState&& state) {
          cbCall++;
          if (state.exception()) {
            timeout = true;
            auto ex = state.exception().get_exception();
            auto& e = dynamic_cast<TTransportException const&>(*ex);
            EXPECT_EQ(TTransportException::TIMED_OUT, e.getType());
            return;
          }
          try {
            std::string resp;
            client->recv_sendResponse(resp, state);
          } catch (const TApplicationException& e) {
            timeout = true;
            EXPECT_EQ(TApplicationException::TIMEOUT, e.getType());
            EXPECT_TRUE(
                state.header()->getFlags() & HEADER_FLAG_SUPPORT_OUT_OF_ORDER);
            return;
          }
          timeout = false;
        }));
  };

  // Set the timeout to be 5 milliseconds, but the call will take 10 ms.
  // The server should send a timeout after 5 milliseconds
  RpcOptions options;
  options.setTimeout(std::chrono::milliseconds(5));
  auto client1 = getClient();
  bool timeout1;
  client1->sendResponse(options, callback(client1, timeout1), 10000);
  base.loop();
  EXPECT_TRUE(timeout1);
  usleep(10000);

  // This time we set the timeout to be 100 millseconds.  The server
  // should not time out
  options.setTimeout(std::chrono::milliseconds(100));
  client1->sendResponse(options, callback(client1, timeout1), 10000);
  base.loop();
  EXPECT_FALSE(timeout1);
  usleep(10000);

  // This time we set server timeout to be 5 millseconds.  However, the
  // task should start processing within that millisecond, so we should
  // not see an exception because the client timeout should be used after
  // processing is started
  server->setTaskExpireTime(std::chrono::milliseconds(5));
  client1->sendResponse(options, callback(client1, timeout1), 10000);
  base.loop();
  usleep(10000);

  // The server timeout stays at 5 ms, but we put the client timeout at
  // 5 ms.  We should timeout even though the server starts processing within
  // 5ms.
  options.setTimeout(std::chrono::milliseconds(5));
  client1->sendResponse(options, callback(client1, timeout1), 10000);
  base.loop();
  EXPECT_TRUE(timeout1);
  usleep(50000);

  // And finally, with the server timeout at 50 ms, we send 2 requests at
  // once.  Because the first request will take more than 50 ms to finish
  // processing (the server only has 1 worker thread), the second request
  // won't start processing until after 50ms, and will timeout, despite the
  // very high client timeout.
  // We don't know which one will timeout (race conditions) so we just check
  // the xor
  auto client2 = getClient();
  bool timeout2;
  server->setTaskExpireTime(std::chrono::milliseconds(50));
  options.setTimeout(std::chrono::milliseconds(110));
  client1->sendResponse(options, callback(client1, timeout1), 100000);
  client2->sendResponse(options, callback(client2, timeout2), 100000);
  base.loop();
  EXPECT_TRUE(timeout1 || timeout2);
  EXPECT_FALSE(timeout1 && timeout2);

  EXPECT_EQ(cbCall, cbCtor);
}

TEST(ThriftServer, ConnectionIdleTimeoutTest) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = factory.create();
  server->setIdleTimeout(std::chrono::milliseconds(20));
  apache::thrift::util::ScopedServerThread st(server);

  folly::EventBase base;
  auto socket = folly::AsyncSocket::newSocket(&base, *st.getAddress());

  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));

  std::string response;
  client.sync_sendResponse(response, 200);
  EXPECT_EQ(response, "test200");
  base.loop();
}

TEST(ThriftServer, BadSendTest) {
  class Callback : public RequestCallback {
    void requestSent() override {}
    void replyReceived(ClientReceiveState&&) override { ADD_FAILURE(); }
    void requestError(ClientReceiveState&& state) override {
      EXPECT_TRUE(state.exception());
      auto ex =
          state.exception()
              .get_exception<apache::thrift::transport::TTransportException>();
      ASSERT_TRUE(ex);
      EXPECT_THAT(
          ex->what(), testing::StartsWith("transport is closed in write()"));
    }
  };

  TestThriftServerFactory<TestInterface> factory;
  ScopedServerThread sst(factory.create());
  folly::EventBase base;
  auto socket = folly::AsyncSocket::newSocket(&base, *sst.getAddress());
  auto socketPtr = socket.get();
  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));

  client.sendResponse(std::unique_ptr<RequestCallback>(new Callback), 64);

  socketPtr->shutdownWriteNow();
  base.loop();

  std::string response;
  EXPECT_THROW(client.sync_sendResponse(response, 64), TTransportException);
}

TEST(ThriftServer, ResetStateTest) {
  folly::EventBase base;

  // Create a server socket and bind, don't listen.  This gets us a
  // port to test with which is guaranteed to fail.
  auto ssock = std::unique_ptr<
      folly::AsyncServerSocket,
      folly::DelayedDestruction::Destructor>(new folly::AsyncServerSocket);
  ssock->bind(0);
  EXPECT_FALSE(ssock->getAddresses().empty());

  // We do this loop a bunch of times, because the bug which caused
  // the assertion failure was a lost race, which doesn't happen
  // reliably.
  for (int i = 0; i < 1000; ++i) {
    auto socket =
        folly::AsyncSocket::newSocket(&base, ssock->getAddresses()[0]);

    // Create a client.
    TestServiceAsyncClient client(
        HeaderClientChannel::newChannel(std::move(socket)));

    std::string response;
    // This will fail, because there's no server.
    EXPECT_THROW(client.sync_sendResponse(response, 64), TTransportException);
    // On a failed client object, this should also throw an exception.
    // In the past, this would generate an assertion failure and
    // crash.
    EXPECT_THROW(client.sync_sendResponse(response, 64), TTransportException);
  }
}

TEST_P(HeaderOrRocket, FailureInjection) {
  enum ExpectedFailure { NONE = 0, ERROR, TIMEOUT, DISCONNECT, END };

  std::atomic<ExpectedFailure> expected(NONE);

  using apache::thrift::transport::TTransportException;

  class Callback : public RequestCallback {
   public:
    explicit Callback(const std::atomic<ExpectedFailure>* expected)
        : expected_(expected) {}

   private:
    void requestSent() override {}

    void replyReceived(ClientReceiveState&& state) override {
      std::string response;
      try {
        TestServiceAsyncClient::recv_sendResponse(response, state);
        EXPECT_EQ(NONE, *expected_);
      } catch (const apache::thrift::TApplicationException&) {
        const auto& headers = state.header()->getHeaders();
        EXPECT_TRUE(
            headers.find("ex") != headers.end() &&
            headers.find("ex")->second == kInjectedFailureErrorCode);
        EXPECT_EQ(ERROR, *expected_);
      } catch (...) {
        ADD_FAILURE() << "Unexpected exception thrown";
      }

      // Now do it again with exception_wrappers.
      auto ew =
          TestServiceAsyncClient::recv_wrapped_sendResponse(response, state);
      if (ew) {
        EXPECT_TRUE(
            ew.is_compatible_with<apache::thrift::TApplicationException>());
        EXPECT_EQ(ERROR, *expected_);
      } else {
        EXPECT_EQ(NONE, *expected_);
      }
    }

    void requestError(ClientReceiveState&& state) override {
      ASSERT_TRUE(state.exception());
      auto ex_ = state.exception().get_exception();
      auto& ex = dynamic_cast<const TTransportException&>(*ex_);
      if (ex.getType() == TTransportException::TIMED_OUT) {
        EXPECT_EQ(TIMEOUT, *expected_);
      } else {
        EXPECT_EQ(DISCONNECT, *expected_);
      }
    }

    const std::atomic<ExpectedFailure>* expected_;
  };

  ScopedServerInterfaceThread sst(std::make_shared<TestInterface>());
  folly::EventBase base;
  auto client = makeClient(sst, &base);

  auto& server = sst.getThriftServer();
  SCOPE_EXIT { server.setFailureInjection(ThriftServer::FailureInjection()); };

  RpcOptions rpcOptions;
  rpcOptions.setTimeout(std::chrono::milliseconds(100));
  for (int i = 0; i < END; ++i) {
    auto exp = static_cast<ExpectedFailure>(i);
    ThriftServer::FailureInjection fi;
    LOG(INFO) << i;
    switch (exp) {
      case NONE:
        break;
      case ERROR:
        fi.errorFraction = 1;
        break;
      case TIMEOUT:
        fi.dropFraction = 1;
        break;
      case DISCONNECT:
        fi.disconnectFraction = 1;
        break;
      case END:
        LOG(FATAL) << "unreached";
    }

    server.setFailureInjection(std::move(fi));

    expected = exp;

    auto callback = std::make_unique<Callback>(&expected);
    client->sendResponse(rpcOptions, std::move(callback), 1);
    base.loop();
  }
}

TEST(ThriftServer, useExistingSocketAndExit) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());
  folly::AsyncServerSocket::UniquePtr serverSocket(
      new folly::AsyncServerSocket);
  serverSocket->bind(0);
  server->useExistingSocket(std::move(serverSocket));
  // In the past, this would cause a SEGV
}

TEST(ThriftServer, useExistingSocketAndConnectionIdleTimeout) {
  // This is ConnectionIdleTimeoutTest, but with an existing socket
  TestThriftServerFactory<TestInterface> factory;
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());
  folly::AsyncServerSocket::UniquePtr serverSocket(
      new folly::AsyncServerSocket);
  serverSocket->bind(0);
  server->useExistingSocket(std::move(serverSocket));

  server->setIdleTimeout(std::chrono::milliseconds(20));
  apache::thrift::util::ScopedServerThread st(server);

  folly::EventBase base;
  auto socket = folly::AsyncSocket::newSocket(&base, *st.getAddress());

  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));

  std::string response;
  client.sync_sendResponse(response, 200);
  EXPECT_EQ(response, "test200");
  base.loop();
}

namespace {
class ReadCallbackTest : public folly::AsyncTransport::ReadCallback {
 public:
  void getReadBuffer(void**, size_t*) override {}
  void readDataAvailable(size_t) noexcept override {}
  void readEOF() noexcept override { eof = true; }

  void readErr(const folly::AsyncSocketException&) noexcept override {
    eof = true;
  }

  bool eof = false;
};
} // namespace

TEST(ThriftServer, ShutdownSocketSetTest) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());
  ScopedServerThread sst(server);
  folly::EventBase base;
  ReadCallbackTest cb;

  auto socket2 = folly::AsyncSocket::newSocket(&base, *sst.getAddress());
  socket2->setReadCB(&cb);

  base.tryRunAfterDelay(
      [&]() { folly::tryGetShutdownSocketSet()->shutdownAll(); }, 10);
  base.tryRunAfterDelay([&]() { base.terminateLoopSoon(); }, 30);
  base.loopForever();
  EXPECT_EQ(cb.eof, true);
}

TEST(ThriftServer, ShutdownDegenarateServer) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = factory.create();
  server->setMaxRequests(1);
  server->setNumIOWorkerThreads(1);
  ScopedServerThread sst(server);
}

TEST(ThriftServer, ModifyingIOThreadCountLive) {
  TestThriftServerFactory<TestInterface> factory;
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());
  auto iothreadpool = std::make_shared<folly::IOThreadPoolExecutor>(0);
  server->setIOThreadPool(iothreadpool);

  ScopedServerThread sst(server);
  // If there are no worker threads, generally the server event base
  // will stop loop()ing.  Create a timeout event to make sure
  // it continues to loop for the duration of the test.
  server->getServeEventBase()->runInEventBaseThread(
      [&]() { server->getServeEventBase()->tryRunAfterDelay([]() {}, 5000); });

  server->getServeEventBase()->runInEventBaseThreadAndWait(
      [=]() { iothreadpool->setNumThreads(0); });

  folly::EventBase base;

  auto socket = folly::AsyncSocket::newSocket(&base, *sst.getAddress());

  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));

  std::string response;

  boost::polymorphic_downcast<ClientChannel*>(client.getChannel())
      ->setTimeout(100);

  // This should fail as soon as it connects:
  // since AsyncServerSocket has no accept callbacks installed,
  // it should close the connection right away.
  ASSERT_ANY_THROW(client.sync_sendResponse(response, 64));

  server->getServeEventBase()->runInEventBaseThreadAndWait(
      [=]() { iothreadpool->setNumThreads(30); });

  auto socket2 = folly::AsyncSocket::newSocket(&base, *sst.getAddress());

  // Can't reuse client since the channel has gone bad
  TestServiceAsyncClient client2(
      HeaderClientChannel::newChannel(std::move(socket2)));

  client2.sync_sendResponse(response, 64);
}

TEST(ThriftServer, setIOThreadPool) {
  auto exe = std::make_shared<folly::IOThreadPoolExecutor>(1);
  TestThriftServerFactory<TestInterface> factory;
  factory.useSimpleThreadManager(false);
  auto server = std::static_pointer_cast<ThriftServer>(factory.create());

  // Set the exe, this used to trip various calls like
  // CHECK(ioThreadPool->numThreads() == 0).
  server->setIOThreadPool(exe);
  EXPECT_EQ(1, server->getNumIOWorkerThreads());
}

TEST(ThriftServer, IdleServerTimeout) {
  TestThriftServerFactory<TestInterface> factory;

  auto server = factory.create();
  auto thriftServer = dynamic_cast<ThriftServer*>(server.get());
  thriftServer->setIdleServerTimeout(std::chrono::milliseconds(50));

  ScopedServerThread scopedServer(server);
  scopedServer.join();
}

TEST(ThriftServer, ServerConfigTest) {
  ThriftServer server;

  wangle::ServerSocketConfig defaultConfig;
  // If nothing is set, expect defaults
  auto serverConfig = server.getServerSocketConfig();
  EXPECT_EQ(
      serverConfig.sslHandshakeTimeout, std::chrono::milliseconds::zero());

  // Idle timeout of 0 with no SSL handshake set, expect it to be 0.
  server.setIdleTimeout(std::chrono::milliseconds::zero());
  serverConfig = server.getServerSocketConfig();
  EXPECT_EQ(
      serverConfig.sslHandshakeTimeout, std::chrono::milliseconds::zero());

  // Expect the explicit to always win
  server.setSSLHandshakeTimeout(std::chrono::milliseconds(100));
  serverConfig = server.getServerSocketConfig();
  EXPECT_EQ(serverConfig.sslHandshakeTimeout, std::chrono::milliseconds(100));

  // Clear it and expect it to be zero again (due to idle timeout = 0)
  server.setSSLHandshakeTimeout(std::nullopt);
  serverConfig = server.getServerSocketConfig();
  EXPECT_EQ(
      serverConfig.sslHandshakeTimeout, std::chrono::milliseconds::zero());
}

TEST(ThriftServer, MultiPort) {
  class MultiPortThriftServer : public ThriftServer {
   public:
    using ServerBootstrap::getSockets;
  };

  auto server = std::make_shared<MultiPortThriftServer>();
  server->setInterface(std::make_shared<TestInterface>());
  server->setNumIOWorkerThreads(1);
  server->setNumCPUWorkerThreads(1);

  // Add two ports 0 to trigger listening on two random ports.
  folly::SocketAddress addr;
  addr.setFromLocalPort(static_cast<uint16_t>(0));
  server->setAddresses({addr, addr});

  ScopedServerThread t(server);

  auto sockets = server->getSockets();
  ASSERT_EQ(sockets.size(), 2);

  folly::SocketAddress addr1, addr2;
  sockets[0]->getAddress(&addr1);
  sockets[1]->getAddress(&addr2);

  EXPECT_NE(addr1.getPort(), addr2.getPort());

  // Test that we can talk via first port.
  folly::EventBase base;

  auto testFn = [&](folly::SocketAddress& address) {
    auto socket = folly::AsyncSocket::newSocket(&base, address);
    TestServiceAsyncClient client(
        HeaderClientChannel::newChannel(std::move(socket)));
    std::string response;
    client.sync_sendResponse(response, 42);
    EXPECT_EQ(response, "test42");
  };

  testFn(addr1);
  testFn(addr2);
}

TEST(ThriftServer, ClientIdentityHook) {
  /* Tests that the server calls the client identity hook when creating a new
     connection context */

  std::atomic<bool> flag{false};
  auto hook = [&flag](
                  const folly::AsyncTransport* /* unused */,
                  const X509* /* unused */,
                  const folly::SocketAddress& /* unused */) {
    flag = true;
    return std::unique_ptr<void, void (*)(void*)>(nullptr, [](void*) {});
  };

  TestThriftServerFactory<TestInterface> factory;
  auto server = factory.create();
  server->setClientIdentityHook(hook);
  apache::thrift::util::ScopedServerThread st(server);

  folly::EventBase base;
  auto socket = folly::AsyncSocket::newSocket(&base, *st.getAddress());
  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));
  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_TRUE(flag);
}

namespace {
void setupServerSSL(ThriftServer& server) {
  auto sslConfig = std::make_shared<wangle::SSLContextConfig>();
  sslConfig->setCertificate(
      find_resource(folly::test::kTestCert).string(),
      find_resource(folly::test::kTestKey).string(),
      "");
  sslConfig->clientCAFiles =
      std::vector<std::string>{find_resource(folly::test::kTestCA).string()};
  sslConfig->sessionContext = "ThriftServerTest";
  server.setSSLConfig(std::move(sslConfig));
}

std::shared_ptr<folly::SSLContext> makeClientSslContext() {
  auto ctx = std::make_shared<folly::SSLContext>();
  ctx->loadCertificate(find_resource(folly::test::kTestCert).c_str());
  ctx->loadPrivateKey(find_resource(folly::test::kTestKey).c_str());
  ctx->loadTrustedCertificates(find_resource(folly::test::kTestCA).c_str());
  ctx->authenticate(
      true /* verify server cert */, false /* don't verify server name */);
  ctx->setVerificationOption(folly::SSLContext::SSLVerifyPeerEnum::VERIFY);
  return ctx;
}

void doBadRequestHeaderTest(bool secure) {
  auto server = std::static_pointer_cast<ThriftServer>(
      TestThriftServerFactory<TestInterface>().create());
  if (secure) {
    setupServerSSL(*server);
  }
  ScopedServerThread sst(std::move(server));

  folly::EventBase evb;
  folly::AsyncSocket::UniquePtr socket(
      secure ? new folly::AsyncSSLSocket(makeClientSslContext(), &evb)
             : new folly::AsyncSocket(&evb));
  socket->connect(nullptr /* connect callback */, *sst.getAddress());

  class RecordWriteSuccessCallback
      : public folly::AsyncTransport::WriteCallback {
   public:
    void writeSuccess() noexcept override {
      EXPECT_FALSE(success_);
      success_.emplace(true);
    }

    void writeErr(
        size_t /* bytesWritten */,
        const folly::AsyncSocketException& /* exception */) noexcept override {
      EXPECT_FALSE(success_);
      success_.emplace(false);
    }

    bool success() const { return success_ && *success_; }

   private:
    folly::Optional<bool> success_;
  };
  RecordWriteSuccessCallback recordSuccessWriteCallback;

  class CheckClosedReadCallback : public folly::AsyncTransport::ReadCallback {
   public:
    explicit CheckClosedReadCallback(folly::AsyncSocket& socket)
        : socket_(socket) {
      socket_.setReadCB(this);
    }

    ~CheckClosedReadCallback() override {
      // We expect that the server closed the connection
      EXPECT_TRUE(remoteClosed_);
      socket_.close();
    }

    void getReadBuffer(void** bufout, size_t* lenout) override {
      // For this test, we never do anything with the buffered data, but we
      // still need to implement the full ReadCallback interface.
      *bufout = buf_;
      *lenout = sizeof(buf_);
    }

    void readDataAvailable(size_t /* len */) noexcept override {}

    void readEOF() noexcept override { remoteClosed_ = true; }

    void readErr(const folly::AsyncSocketException& ex) noexcept override {
      ASSERT_EQ(ECONNRESET, ex.getErrno());
      remoteClosed_ = true;
    }

   private:
    folly::AsyncSocket& socket_;
    char buf_[1024];
    bool remoteClosed_{false};
  };

  EXPECT_TRUE(socket->good());
  {
    CheckClosedReadCallback checkClosedReadCallback_(*socket);
    constexpr folly::StringPiece kBadRequest("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    socket->write(
        &recordSuccessWriteCallback, kBadRequest.data(), kBadRequest.size());
    evb.loop();
  }

  EXPECT_TRUE(recordSuccessWriteCallback.success());
  EXPECT_FALSE(socket->good());
}
} // namespace

TEST(ThriftServer, BadRequestHeaderNoSsl) {
  doBadRequestHeaderTest(false /* secure */);
}

TEST(ThriftServer, BadRequestHeaderSsl) {
  doBadRequestHeaderTest(true /* secure */);
}

TEST(ThriftServer, SSLRequiredRejectsPlaintext) {
  auto server = std::static_pointer_cast<ThriftServer>(
      TestThriftServerFactory<TestInterface>().create());
  server->setSSLPolicy(SSLPolicy::REQUIRED);
  setupServerSSL(*server);
  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  auto socket = folly::AsyncSocket::newSocket(&base, *sst.getAddress());
  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));

  std::string response;
  EXPECT_THROW(client.sync_sendResponse(response, 64);, TTransportException);
}

namespace {
class FizzStopTLSConnector
    : public fizz::client::AsyncFizzClient::HandshakeCallback,
      public fizz::AsyncFizzBase::EndOfTLSCallback {
 public:
  folly::AsyncSocket::UniquePtr connect(
      const folly::SocketAddress& address, folly::EventBase* eb) {
    eb_ = eb;

    auto sock = folly::AsyncSocket::newSocket(eb_, address);
    auto ctx = std::make_shared<fizz::client::FizzClientContext>();
    ctx->setSupportedAlpns({"rs"});
    auto thriftParametersContext =
        std::make_shared<apache::thrift::ThriftParametersContext>();
    thriftParametersContext->setUseStopTLS(true);
    auto extension =
        std::make_shared<apache::thrift::ThriftParametersClientExtension>(
            thriftParametersContext);

    client_.reset(new fizz::client::AsyncFizzClient(
        std::move(sock), std::move(ctx), std::move(extension)));
    client_->connect(
        this,
        nullptr,
        folly::none,
        folly::none,
        folly::Optional<std::vector<fizz::ech::ECHConfig>>(folly::none),
        std::chrono::milliseconds(100));
    return promise_.getFuture().getVia(eb_);
  }

  void fizzHandshakeSuccess(
      fizz::client::AsyncFizzClient* client) noexcept override {
    client->setEndOfTLSCallback(this);
  }

  void fizzHandshakeError(
      fizz::client::AsyncFizzClient* /* unused */,
      folly::exception_wrapper ex) noexcept override {
    promise_.setException(ex);
    FAIL();
  }

  void endOfTLS(
      fizz::AsyncFizzBase* transport, std::unique_ptr<folly::IOBuf>) override {
    auto sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
    DCHECK(sock);

    auto fd = sock->detachNetworkSocket();
    auto zcId = sock->getZeroCopyBufId();

    // create new socket
    auto plaintextTransport =
        folly::AsyncSocket::UniquePtr(new folly::AsyncSocket(eb_, fd, zcId));
    promise_.setValue(std::move(plaintextTransport));
  }

  fizz::client::AsyncFizzClient::UniquePtr client_;
  folly::Promise<folly::AsyncSocket::UniquePtr> promise_;
  folly::EventBase* eb_;
};
} // namespace

TEST(ThriftServer, StopTLSDowngrade) {
  auto server = std::static_pointer_cast<ThriftServer>(
      TestThriftServerFactory<TestInterface>().create());
  server->setSSLPolicy(SSLPolicy::REQUIRED);
  auto sslConfig = std::make_shared<wangle::SSLContextConfig>();
  sslConfig->setNextProtocols({"rs"});
  sslConfig->setCertificate(
      find_resource(folly::test::kTestCert).string(),
      find_resource(folly::test::kTestKey).string(),
      "");
  sslConfig->clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  server->setSSLConfig(std::move(sslConfig));
  ThriftTlsConfig thriftConfig;
  thriftConfig.enableThriftParamsNegotiation = true;
  thriftConfig.enableStopTLS = true;
  server->setThriftConfig(thriftConfig);
  server->setAcceptorFactory(
      std::make_shared<DefaultThriftAcceptorFactory>(server.get()));
  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  FizzStopTLSConnector connector;
  auto transport = connector.connect(*sst.getAddress(), &base);
  // Note we only use stop tls with rocket
  TestServiceAsyncClient client(
      RocketClientChannel::newChannel(std::move(transport)));

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
  base.loopOnce();
}

TEST(ThriftServer, SSLRequiredAllowsLocalPlaintext) {
  auto server = std::static_pointer_cast<ThriftServer>(
      TestThriftServerFactory<TestInterface>().create());
  server->setAllowPlaintextOnLoopback(true);
  server->setSSLPolicy(SSLPolicy::REQUIRED);
  setupServerSSL(*server);
  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  // ensure that the address is loopback
  auto port = sst.getAddress()->getPort();
  folly::SocketAddress loopback("::1", port);
  auto socket = folly::AsyncSocket::newSocket(&base, loopback);
  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
  base.loop();
}

TEST(ThriftServer, SSLRequiredLoopbackUsesSSL) {
  auto server = std::static_pointer_cast<ThriftServer>(
      TestThriftServerFactory<TestInterface>().create());
  server->setAllowPlaintextOnLoopback(true);
  server->setSSLPolicy(SSLPolicy::REQUIRED);
  setupServerSSL(*server);
  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  // ensure that the address is loopback
  auto port = sst.getAddress()->getPort();
  folly::SocketAddress loopback("::1", port);

  auto ctx = makeClientSslContext();
  auto sslSock = TAsyncSSLSocket::newSocket(ctx, &base);
  sslSock->connect(nullptr /* connect callback */, loopback);

  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(sslSock)));

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
  base.loop();
}

TEST(ThriftServer, SSLPermittedAcceptsPlaintextAndSSL) {
  auto server = std::static_pointer_cast<ThriftServer>(
      TestThriftServerFactory<TestInterface>().create());
  server->setSSLPolicy(SSLPolicy::PERMITTED);
  setupServerSSL(*server);
  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  {
    SCOPED_TRACE("Plaintext");
    auto socket = folly::AsyncSocket::newSocket(&base, *sst.getAddress());
    TestServiceAsyncClient client(
        HeaderClientChannel::newChannel(std::move(socket)));

    std::string response;
    client.sync_sendResponse(response, 64);
    EXPECT_EQ(response, "test64");
    base.loop();
  }

  {
    SCOPED_TRACE("SSL");
    auto ctx = makeClientSslContext();
    auto sslSock = TAsyncSSLSocket::newSocket(ctx, &base);
    sslSock->connect(nullptr /* connect callback */, *sst.getAddress());

    TestServiceAsyncClient client(
        HeaderClientChannel::newChannel(std::move(sslSock)));

    std::string response;
    client.sync_sendResponse(response, 64);
    EXPECT_EQ(response, "test64");
    base.loop();
  }
}

TEST(ThriftServer, ClientOnlyTimeouts) {
  class SendResponseInterface
      : public apache::thrift::ServiceHandler<TestService> {
    void sync_sendResponse(
        std::string& _return, int64_t shouldSleepMs) override {
      auto header = getConnectionContext()->getHeader();
      if (shouldSleepMs) {
        usleep(shouldSleepMs * 1000);
      }
      _return = fmt::format(
          "{}:{}",
          header->getClientTimeout().count(),
          header->getClientQueueTimeout().count());
    }
  };
  TestThriftServerFactory<SendResponseInterface> factory;
  ScopedServerThread st(factory.create());

  folly::EventBase base;
  auto socket = folly::AsyncSocket::newSocket(&base, *st.getAddress());
  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(socket)));

  for (bool clientOnly : {false, true}) {
    for (bool shouldTimeOut : {true, false}) {
      std::string response;
      RpcOptions rpcOpts;
      rpcOpts.setTimeout(std::chrono::milliseconds(30));
      rpcOpts.setQueueTimeout(std::chrono::milliseconds(30));
      rpcOpts.setClientOnlyTimeouts(clientOnly);
      try {
        client.sync_sendResponse(rpcOpts, response, shouldTimeOut ? 50 : 0);
        EXPECT_FALSE(shouldTimeOut);
        if (clientOnly) {
          EXPECT_EQ(response, "0:0");
        } else {
          EXPECT_EQ(response, "30:30");
        }
      } catch (...) {
        EXPECT_TRUE(shouldTimeOut);
      }
    }
  }
  base.loop();
}

TEST(ThriftServerTest, QueueTimeHeaderTest) {
  THRIFT_OMIT_TEST_WITH_RESOURCE_POOLS(/* ThreadManager features */);
  using namespace ::testing;
  // Tests that queue time metadata is returned in the THeader when
  // queueing delay on server side is greater than pre-defined threshold.
  static constexpr std::chrono::milliseconds kDefaultQueueTimeout{100};
  class QueueTimeTestHandler
      : public apache::thrift::ServiceHandler<TestService> {
   public:
    void sync_sendResponse(std::string& _return, int64_t size) override {
      _return = folly::to<std::string>(size);
    }
  };

  auto handler = std::make_shared<QueueTimeTestHandler>();
  ScopedServerInterfaceThread runner(handler);
  folly::EventBase eb;
  auto client = runner.newClient<TestServiceAsyncClient>(
      &eb, RocketClientChannel::newChannel);
  // Queue a task on the runner's ThreadManager to block it from
  // executing the Thrift request.
  auto tServer = dynamic_cast<ThriftServer*>(&runner.getThriftServer());
  tServer->setQueueTimeout(kDefaultQueueTimeout);
  auto threadManager = tServer->getThreadManager();

  folly::Baton<> startupBaton;
  threadManager->add([&startupBaton]() {
    startupBaton.post();
    /* sleep override */
    std::this_thread::sleep_for(50ms);
  });

  startupBaton.wait();
  // Send the request with a high queue timeout to make sure it succeeds.
  RpcOptions options;
  options.setTimeout(std::chrono::milliseconds(1000));
  auto [resp, header] =
      client->header_semifuture_sendResponse(options, 42).get();
  EXPECT_EQ(resp, "42");
  // Check that queue time headers are set
  auto queueTimeout = header->getServerQueueTimeout();
  auto queueingTime = header->getProcessDelay();
  EXPECT_TRUE(queueTimeout.hasValue() && queueingTime.hasValue());
  EXPECT_EQ(queueTimeout.value(), kDefaultQueueTimeout);
  EXPECT_THAT(
      queueingTime.value(),
      AllOf(
          Gt(std::chrono::milliseconds(5)), Lt(std::chrono::milliseconds(50))));
}

TEST(ThriftServer, QueueTimeoutStressTest) {
  static std::atomic<int> server_reply = 0;
  static std::atomic<int> received_reply = 0;

  class SendResponseInterface
      : public apache::thrift::ServiceHandler<TestService> {
    void sync_sendResponse(std::string& _return, int64_t id) override {
      DCHECK(lastSeenId_ < id);

      if (lastSeenId_ + 1 == id) {
        std::this_thread::sleep_for(sleepTime_);
        sleepTime_ *= 2;
      } else {
        sleepTime_ = std::chrono::microseconds{1};
      }
      server_reply++;
      lastSeenId_ = id;
      _return = "wow";
    }

    std::chrono::microseconds sleepTime_{1};
    int64_t lastSeenId_{-1};
  };

  constexpr size_t kNumReqs = 50000;

  {
    ScopedServerInterfaceThread runner(
        std::make_shared<SendResponseInterface>());
    auto tServer = dynamic_cast<ThriftServer*>(&runner.getThriftServer());
    tServer->setQueueTimeout(10ms);
    auto client = runner.newStickyClient<TestServiceAsyncClient>(
        nullptr /* executor */, [](auto socket) mutable {
          return RocketClientChannel::newChannel(std::move(socket));
        });

    std::vector<folly::SemiFuture<std::string>> futures;
    for (size_t req = 0; req < kNumReqs; ++req) {
      futures.emplace_back(client->semifuture_sendResponse(req));
    }
    size_t exceptions = 0;
    for (auto& future : futures) {
      auto t = std::move(future).getTry();
      if (t.hasValue()) {
        ++received_reply;
      } else {
        ++exceptions;
      }
    }

    EXPECT_LT(exceptions, kNumReqs);
    EXPECT_GT(exceptions, 0);
  }

  EXPECT_EQ(received_reply, server_reply);
}

class ServerResponseEnqueuedInterface : public TestInterface {
 public:
  explicit ServerResponseEnqueuedInterface(
      folly::Baton<>& responseEnqueuedBaton)
      : responseEnqueuedBaton_(responseEnqueuedBaton) {}

  void async_eb_eventBaseAsync(
      std::unique_ptr<
          apache::thrift::HandlerCallback<std::unique_ptr<::std::string>>>
          callback) override {
    // Since `eventBaseAsync` is a `thread = 'eb'` method, this runs on
    // the IO thread, and we can guarantee that the baton is posted
    // no earlier than the write was enqueued to the WriteBatcher.
    //
    // In contrast, if we posted the baton from a regular request handler
    // thread pool, there would be a chance that it would fire BEFORE the IO
    // thread could enqueue the write.
    callback->getEventBase()->dcheckIsInEventBaseThread();
    callback->getEventBase()->runInEventBaseThread(
        [&]() mutable { responseEnqueuedBaton_.post(); });

    callback->result(folly::make_unique<std::string>("done"));
  }

  folly::Baton<>& responseEnqueuedBaton_;
};

class WriteBatchingTest : public testing::Test {
 protected:
  std::unique_ptr<ScopedServerInterfaceThread> runner_;
  std::unique_ptr<TestServiceAsyncClient> client_;
  folly::Baton<> baton_;
  size_t tearDownDummyRequestCount_ = 0;

  void init(
      std::chrono::milliseconds batchingInterval,
      size_t batchingSize,
      size_t batchingByteSize,
      size_t tearDownDummyRequestCount) {
    tearDownDummyRequestCount_ = tearDownDummyRequestCount;

    runner_ = folly::make_unique<ScopedServerInterfaceThread>(
        std::make_shared<ServerResponseEnqueuedInterface>(baton_));
    runner_->getThriftServer().setWriteBatchingInterval(batchingInterval);
    runner_->getThriftServer().setWriteBatchingSize(batchingSize);
    runner_->getThriftServer().setWriteBatchingByteSize(batchingByteSize);

    client_ = runner_->newStickyClient<TestServiceAsyncClient>(
        nullptr, [](auto socket) mutable {
          return RocketClientChannel::newChannel(std::move(socket));
        });
  }

  void waitUntilServerWriteScheduled() {
    baton_.wait();
    baton_.reset();
  }

  void TearDown() override {
    // Since we have configured the Thrift Server to use write batching, the
    // last ErrorFrame that is sent by the server during destruction will be
    // buffered and will cause the test to wait until the buffer is flushed. We
    // can avoid this by sending dummy requests before the ErrorFrame is queued
    // so that the buffer is flushed immediately after the ErrorFrame is queued.
    for (size_t i = 0; i < tearDownDummyRequestCount_; ++i) {
      client_->semifuture_eventBaseAsync();
      waitUntilServerWriteScheduled();
    }
  }
};

TEST_F(WriteBatchingTest, SizeEarlyFlushTest) {
  init(std::chrono::seconds{1}, 3, 0, 2);

  // Send first request. This will cause 2 writes to be buffered on the server
  // (1 SetupFrame and 1 response). Ensure we don't get a response.
  auto f = client_->semifuture_eventBaseAsync();
  waitUntilServerWriteScheduled();
  bool isFulfilled = std::move(f).wait(std::chrono::milliseconds{50});
  EXPECT_FALSE(isFulfilled);

  // Send second request. This will cause batching size limit to be reached and
  // buffered writes will be flushed. Ensure we get a response.
  f = client_->semifuture_eventBaseAsync();
  waitUntilServerWriteScheduled();
  isFulfilled = std::move(f).wait(std::chrono::milliseconds{50});
  EXPECT_TRUE(isFulfilled);
}

TEST_F(WriteBatchingTest, ByteSizeEarlyFlushTest) {
  init(std::chrono::seconds{1}, 4, 60, 2);

  // Send first request. This will cause 2 writes to be buffered on the server
  // (1 SetupFrame - 14 bytes and 1 response - 25 bytes). Ensure we don't get
  // a response.
  auto f = client_->semifuture_eventBaseAsync();
  waitUntilServerWriteScheduled();
  bool isFulfilled = std::move(f).wait(std::chrono::milliseconds{50});
  EXPECT_FALSE(isFulfilled);

  // Send second request. This will cause batching byte size limit to be
  // reached and buffered writes will be flushed. Ensure we get a response.
  f = client_->semifuture_eventBaseAsync();
  waitUntilServerWriteScheduled();
  isFulfilled = std::move(f).wait(std::chrono::milliseconds{50});
  EXPECT_TRUE(isFulfilled);
}

TEST_F(WriteBatchingTest, IntervalTest) {
  init(std::chrono::milliseconds{100}, 3, 0, 2);

  // Send first request. This will cause 2 writes to be buffered on the server
  // (1 SetupFrame and 1 response). Ensure we get a response after batching
  // interval has elapsed even if batching size has not been reached.
  auto f = client_->semifuture_sendResponse(0).wait();
  EXPECT_TRUE(f.hasValue());
}

TEST_P(HeaderOrRocket, PreprocessHeaders) {
  ScopedServerInterfaceThread runner(std::make_shared<TestInterface>());
  folly::EventBase base;
  auto client = makeClient(runner, &base);

  runner.getThriftServer().setPreprocess([&](auto& params) -> PreprocessResult {
    auto p = folly::get_ptr(params.headers, "foo");
    if (!p) {
      ADD_FAILURE() << "expected to see header 'foo'";
    } else if (*p != "bar") {
      ADD_FAILURE() << "expected to see header with value 'bar'";
    }
    return {};
  });

  RpcOptions rpcOptions;
  rpcOptions.setWriteHeader("foo", "bar");
  client->sync_voidResponse(rpcOptions);
}

TEST_P(HeaderOrRocket, TaskTimeoutBeforeProcessing) {
  folly::Baton haltBaton;
  std::atomic<int> processedCount{0};

  class VoidResponseInterface
      : public apache::thrift::ServiceHandler<TestService> {
   public:
    VoidResponseInterface() = delete;
    VoidResponseInterface(
        folly::Baton<>& haltBaton, std::atomic<int>& processedCount)
        : haltBaton_{haltBaton}, processedCount_{processedCount} {}

    void voidResponse() override {
      processedCount_++;
      haltBaton_.wait();
    }

   private:
    folly::Baton<>& haltBaton_;
    std::atomic<int>& processedCount_;
  };

  {
    ScopedServerInterfaceThread runner(
        std::make_shared<VoidResponseInterface>(haltBaton, processedCount),
        "::1",
        0,
        [](auto& server) {
          // Set task timeout and disable queue timeout
          server.setQueueTimeout(0ms);
          server.setTaskExpireTime(10ms);
          server.setUseClientTimeout(false);
        });
    auto client = makeClient(runner, folly::getEventBase());

    // Make two requests and test that they time out
    for (int i = 0; i < 2; ++i) {
      folly::fibers::Baton baton;
      THeader th;
      folly::exception_wrapper ew;
      client->voidResponse(ServerErrorCallback::create(baton, th, ew));
      baton.wait();
      bool matched = ew.with_exception([](const TApplicationException& tae) {
        EXPECT_EQ(TApplicationException::TIMEOUT, tae.getType());
      });
      ASSERT_TRUE(matched);
      EXPECT_EQ(*folly::get_ptr(th.getHeaders(), "ex"), kTaskExpiredErrorCode);
    }
    haltBaton.post();
  }

  // Assert only one request (the first) was processed
  EXPECT_EQ(processedCount, 1);
}

TEST(ThriftServer, RocketOverSSLNoALPN) {
  auto server = std::static_pointer_cast<ThriftServer>(
      TestThriftServerFactory<TestInterface>().create());
  server->setSSLPolicy(SSLPolicy::REQUIRED);
  setupServerSSL(*server);
  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  auto port = sst.getAddress()->getPort();
  folly::SocketAddress loopback("::1", port);

  auto ctx = makeClientSslContext();
  ctx->disableTLS13();
  folly::AsyncSSLSocket::UniquePtr sslSock(
      new folly::AsyncSSLSocket(ctx, &base));
  sslSock->connect(nullptr /* connect callback */, loopback);

  TestServiceAsyncClient client(
      RocketClientChannel::newChannel(std::move(sslSock)));

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
}

// Tests that the TransportPeekingManager's logic succeeds when using TLS 1.3
// and not providing an ALPN. The RocketOverSSLNoALPN test above currently uses
// TLS 1.2.
TEST(ThriftServer, RocketOverSSLNoALPNWithTLS13) {
  auto server = std::static_pointer_cast<ThriftServer>(
      TestThriftServerFactory<TestInterface>().create());
  server->setSSLPolicy(SSLPolicy::REQUIRED);
  setupServerSSL(*server);
  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  auto port = sst.getAddress()->getPort();
  folly::SocketAddress loopback("::1", port);

  // Should use TLS 1.3 by default
  auto ctx = makeClientSslContext();
  folly::AsyncSSLSocket::UniquePtr sslSock(
      new folly::AsyncSSLSocket(ctx, &base));
  sslSock->connect(nullptr /* connect callback */, loopback);

  TestServiceAsyncClient client(
      RocketClientChannel::newChannel(std::move(sslSock)));

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
}

TEST(ThriftServer, HeaderToRocketUpgradeOverTLS13) {
  THRIFT_FLAG_SET_MOCK(server_rocket_upgrade_enabled, true);

  auto server = std::static_pointer_cast<ThriftServer>(
      TestThriftServerFactory<TestInterface>().create());
  server->setSSLPolicy(SSLPolicy::REQUIRED);

  auto sslConfig = std::make_shared<wangle::SSLContextConfig>();
  sslConfig->setCertificate(
      find_resource(folly::test::kTestCert).string(),
      find_resource(folly::test::kTestKey).string(),
      "");
  sslConfig->clientCAFiles =
      std::vector<std::string>{find_resource(folly::test::kTestCA).string()};
  sslConfig->sessionContext = "ThriftServerTest";
  sslConfig->setNextProtocols(**ThriftServer::defaultNextProtocols());

  server->setSSLConfig(std::move(sslConfig));

  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  auto port = sst.getAddress()->getPort();
  folly::SocketAddress loopback("::1", port);

  // TLS 1.3 enabled by default
  auto ctx = makeClientSslContext();
  folly::AsyncSSLSocket::UniquePtr sslSock(
      new folly::AsyncSSLSocket(ctx, &base));
  sslSock->connect(nullptr /* connect callback */, loopback);

  TestServiceAsyncClient client(
      HeaderClientChannel::newChannel(std::move(sslSock)));

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
}

TEST(ThriftServer, PooledRocketSyncChannel) {
  auto server = std::static_pointer_cast<ThriftServer>(
      TestThriftServerFactory<TestInterface>().create());
  setupServerSSL(*server);
  ScopedServerThread sst(std::move(server));

  auto port = sst.getAddress()->getPort();
  auto clientEvbThread = std::make_shared<folly::ScopedEventBaseThread>();

  auto channel = apache::thrift::PooledRequestChannel::newSyncChannel(
      [clientEvbThread = std::move(
           clientEvbThread)]() -> folly::Executor::KeepAlive<folly::EventBase> {
        return {clientEvbThread->getEventBase()};
      },
      [port](folly::EventBase& evb) mutable {
        folly::SocketAddress loopback("::1", port);
        auto ctx = makeClientSslContext();
        folly::AsyncSSLSocket::UniquePtr sslSock(
            new folly::AsyncSSLSocket(ctx, &evb));
        sslSock->connect(nullptr /* connect callback */, loopback);
        return RocketClientChannel::newChannel(std::move(sslSock));
      });
  TestServiceAsyncClient client(std::move(channel));

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
}

static std::shared_ptr<quic::QuicClientTransport> makeQuicClient(
    folly::EventBase& evb, folly::SocketAddress&& peerAddr) {
  auto qEvb = std::make_shared<quic::FollyQuicEventBase>(&evb);
  auto sock = std::make_unique<quic::FollyQuicAsyncUDPSocket>(qEvb);
  auto ctx = std::make_shared<fizz::client::FizzClientContext>();
  ctx->setSupportedAlpns({"rs"});
  auto verifier = fizz::DefaultCertificateVerifier::createFromCAFiles(
      fizz::VerificationContext::Client,
      {find_resource(folly::test::kTestCA).string()});

  {
    // set up fizz client cert
    std::string certData;
    folly::readFile(find_resource(folly::test::kTestCert).c_str(), certData);

    std::string keyData;
    folly::readFile(find_resource(folly::test::kTestKey).c_str(), keyData);

    if (!certData.empty() && !keyData.empty()) {
      auto cert = fizz::CertUtils::makeSelfCert(
          std::move(certData), std::move(keyData));
      ctx->setClientCertificate(std::move(cert));
    }
  }

  auto quicClient = std::make_shared<quic::QuicClientTransport>(
      qEvb,
      std::move(sock),
      quic::FizzClientQuicHandshakeContext::Builder()
          .setFizzClientContext(std::move(ctx))
          .setCertificateVerifier(std::move(verifier))
          .build());
  quicClient->addNewPeerAddress(std::move(peerAddr));
  return quicClient;
}

TEST(ThriftServer, RocketOverQuic) {
  // copied over from TestThriftServerFactory
  std::shared_ptr<apache::thrift::ThriftServer> server =
      std::make_shared<apache::thrift::ThriftQuicServer>();
  server->setNumIOWorkerThreads(1);
  server->setThreadManagerType(
      apache::thrift::BaseThriftServer::ThreadManagerType::SIMPLE);
  server->setNumCPUWorkerThreads(1);
  server->setThreadFactory(
      std::make_shared<apache::thrift::concurrency::PosixThreadFactory>());
  server->setPort(0);
  server->setInterface(std::make_unique<TestInterface>());

  server->setSSLPolicy(SSLPolicy::REQUIRED);

  auto sslConfig = std::make_shared<wangle::SSLContextConfig>();
  sslConfig->setCertificate(
      find_resource(folly::test::kTestCert).string(),
      find_resource(folly::test::kTestKey).string(),
      "");
  sslConfig->clientCAFiles =
      std::vector<std::string>{find_resource(folly::test::kTestCA).string()};
  sslConfig->sessionContext = "ThriftServerTest";
  sslConfig->setNextProtocols({"rs"});
  server->setSSLConfig(std::move(sslConfig));
  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  auto port = sst.getAddress()->getPort();
  folly::SocketAddress loopback("::1", port);

  folly::AsyncTransport::UniquePtr asyncTransport(
      new quic::QuicClientAsyncTransport(
          makeQuicClient(base, std::move(loopback))));

  TestServiceAsyncClient client(
      RocketClientChannel::newChannel(std::move(asyncTransport)));

  std::string response;
  client.sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
}

#if FOLLY_HAS_MEMORY_RESOURCE
class AccountingMemoryPool : public folly::detail::std_pmr::memory_resource {
 public:
  size_t allocated = 0;
  size_t deallocated = 0;

 protected:
  void* do_allocate(std::size_t bytes, std::size_t alignment) override {
    allocated += bytes;
    return folly::detail::std_pmr::new_delete_resource()->allocate(
        bytes, alignment);
  }

  [[nodiscard]] bool do_is_equal(const folly::detail::std_pmr::memory_resource&
                                     other) const noexcept override {
    return this == &other;
  }

  void do_deallocate(
      void* p, std::size_t bytes, std::size_t alignment) override {
    deallocated += bytes;
    return folly::detail::std_pmr::new_delete_resource()->deallocate(
        p, bytes, alignment);
  }
};

TEST(ThriftServer, CustomParserAllocatorTest) {
  THRIFT_FLAG_SET_MOCK(rocket_allocating_strategy_parser, true);
  AccountingMemoryPool pool;
  auto alloc = std::make_shared<rocket::ParserAllocatorType>(&pool);

  ScopedServerInterfaceThread runner(
      std::make_shared<TestInterface>(), [alloc](ThriftServer& server) {
        server.setCustomAllocatorForParser(alloc);
      });

  auto client = runner.newClient<apache::thrift::Client<TestService>>();

  std::string request(4096, 'a');
  std::string response;
  client->sync_echoRequest(response, request);

  EXPECT_TRUE(pool.allocated > 0);
  EXPECT_TRUE(pool.deallocated > 0);
}
#endif

TEST(ThriftServer, AlpnNotAllowMismatch) {
  auto server = std::static_pointer_cast<ThriftServer>(
      TestThriftServerFactory<TestInterface>().create());
  server->setSSLPolicy(SSLPolicy::REQUIRED);

  auto sslConfig = std::make_shared<wangle::SSLContextConfig>();
  sslConfig->setCertificate(
      find_resource(folly::test::kTestCert).string(),
      find_resource(folly::test::kTestKey).string(),
      "");
  sslConfig->clientCAFiles =
      std::vector<std::string>{find_resource(folly::test::kTestCA).string()};
  sslConfig->sessionContext = "ThriftServerTest";
  sslConfig->setNextProtocols({"rs"});
  server->setSSLConfig(std::move(sslConfig));
  ScopedServerThread sst(std::move(server));

  folly::EventBase base;
  auto port = sst.getAddress()->getPort();
  folly::SocketAddress loopback("::1", port);

  auto ctx = makeClientSslContext();
  ctx->setAdvertisedNextProtocols({"h2"});
  folly::AsyncSSLSocket::UniquePtr sslSock(
      new folly::AsyncSSLSocket(ctx, &base));
  sslSock->connect(nullptr /* connect callback */, loopback);

  TestServiceAsyncClient client(
      RocketClientChannel::newChannel(std::move(sslSock)));

  std::string response;
  EXPECT_THROW(client.sync_sendResponse(response, 64), TTransportException);
}

TEST(ThriftServer, SocketQueueTimeout) {
  TestThriftServerFactory<apache::thrift::ServiceHandler<TestService>> factory;
  auto baseServer = factory.create();

  auto checkSocketQueueTimeout = [&](std::chrono::nanoseconds expectedTimeout) {
    auto server = std::dynamic_pointer_cast<ThriftServer>(baseServer);
    ASSERT_NE(server, nullptr);
    const auto sockets = server->getSockets();
    EXPECT_GT(sockets.size(), 0);
    for (auto& socket : sockets) {
      // Reliably inducing the socket queue timeout is non-trivial through
      // the public API of ThriftServer. Instead, we just need to ensure
      // that each socket has the correct queue timeout after the
      // ThriftServer is set up. The underlying timeout behavior is covered
      // by AsyncServerSocket tests.
      EXPECT_EQ(*socket->getQueueTimeout(), expectedTimeout);
      EXPECT_NE(nullptr, socket->getConnectionEventCallback());
    }
  };

  constexpr auto kDefaultTimeout = 10ms;
  constexpr auto kBaselineTimeout = 20ms;
  constexpr auto kBaselineTimeoutUpdated = 30ms;
  constexpr auto kOverrideTimeout = 40ms;
  THRIFT_FLAG_SET_MOCK(
      server_default_socket_queue_timeout_ms, kDefaultTimeout.count());

  ScopedServerThread st(baseServer);

  auto& serverConfig =
      apache::thrift::detail::getThriftServerConfig(*baseServer);
  // Debug mode does not follow the default Thrift flag and socket queue timeout
  // should always be disabled here
  checkSocketQueueTimeout(folly::kIsDebug ? 0ms : kDefaultTimeout);

  folly::observer::SimpleObservable<std::optional<std::chrono::milliseconds>>
      baseline{kBaselineTimeout};
  // Should trigger value to change
  serverConfig.setSocketQueueTimeout(
      baseline.getObserver(), AttributeSource::BASELINE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  checkSocketQueueTimeout(kBaselineTimeout);

  // Should use updated config
  baseline.setValue(kBaselineTimeoutUpdated);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  checkSocketQueueTimeout(kBaselineTimeoutUpdated);

  folly::observer::SimpleObservable<std::optional<std::chrono::milliseconds>>
      overrideTimeoutObs{kOverrideTimeout};
  // Should use override instead of config
  serverConfig.setSocketQueueTimeout(overrideTimeoutObs.getObserver());
  checkSocketQueueTimeout(kOverrideTimeout);

  // Should go back to baseline instead of override
  serverConfig.setSocketQueueTimeout(
      folly::observer::makeStaticObserver(
          std::optional<std::chrono::milliseconds>()),
      AttributeSource::OVERRIDE);
  checkSocketQueueTimeout(kBaselineTimeoutUpdated);

  // Should go back to using disabled
  serverConfig.setSocketQueueTimeout(
      folly::observer::makeStaticObserver(
          std::optional<std::chrono::milliseconds>()),
      AttributeSource::BASELINE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  checkSocketQueueTimeout(folly::kIsDebug ? 0ms : kDefaultTimeout);
}

TEST(ThriftServer, PerConnectionSocketOptions) {
  class TestServiceHandler
      : public apache::thrift::ServiceHandler<TestService> {
    void voidResponse() override {
      auto socket = const_cast<folly::AsyncSocket*>(
          this->getRequestContext()
              ->getConnectionContext()
              ->getTransport()
              ->getUnderlyingTransport<folly::AsyncSocket>());
      ASSERT_NE(socket, nullptr);
      auto readSockOpt = [&](int level, int optname) -> int {
        int value = -1;
        socklen_t len = sizeof(value);
        socket->getSockOpt(level, optname, &value, &len);
        return value;
      };
      soKeepAlive = readSockOpt(SOL_SOCKET, SO_KEEPALIVE);
      tcpKeepIdle = readSockOpt(IPPROTO_TCP, TCP_KEEPIDLE);
      tcpKeepIntvl = readSockOpt(IPPROTO_TCP, TCP_KEEPINTVL);
      tcpKeepCnt = readSockOpt(IPPROTO_TCP, TCP_KEEPCNT);
    }

   public:
    int soKeepAlive = 0;
    int tcpKeepIdle = 0;
    int tcpKeepIntvl = 0;
    int tcpKeepCnt = 0;
  };

  auto handler = std::make_shared<TestServiceHandler>();
  ScopedServerInterfaceThread runner(handler, [](ThriftServer& server) {
    folly::SocketOptionMap socketOptions{
        {{SOL_SOCKET, SO_KEEPALIVE}, 1},
        {{IPPROTO_TCP, TCP_KEEPIDLE}, 2},
        {{IPPROTO_TCP, TCP_KEEPINTVL}, 3},
        {{IPPROTO_TCP, TCP_KEEPCNT}, 4},
    };
    server.setPerConnectionSocketOptions(std::move(socketOptions));
  });

  auto client = runner.newClient<TestServiceAsyncClient>();
  client->sync_voidResponse();

  EXPECT_EQ(handler->soKeepAlive, 1);
  EXPECT_EQ(handler->tcpKeepIdle, 2);
  EXPECT_EQ(handler->tcpKeepIntvl, 3);
  EXPECT_EQ(handler->tcpKeepCnt, 4);
}

TEST(ThriftServer, RocketOnly) {
  THRIFT_FLAG_SET_MOCK(server_rocket_upgrade_enabled, true);
  TestThriftServerFactory<TestInterface> factory;
  auto serv = factory.create();
  ScopedServerThread sst(serv);
  serv->setLegacyTransport(
      apache::thrift::BaseThriftServer::LegacyTransport::DISABLED);
  folly::EventBase base;

  // Header rejected
  try {
    TestServiceAsyncClient client(HeaderClientChannel::newChannel(
        HeaderClientChannel::WithoutRocketUpgrade{},
        folly::AsyncSocket::newSocket(&base, *sst.getAddress())));
    client.sync_voidResponse();
    ADD_FAILURE() << "should reject";
  } catch (const TTransportException&) {
  }

  // Rocket via upgrade accepted
  try {
    TestServiceAsyncClient client(HeaderClientChannel::newChannel(
        HeaderClientChannel::WithRocketUpgrade{},
        folly::AsyncSocket::newSocket(&base, *sst.getAddress())));
    client.sync_voidResponse();
  } catch (const TTransportException&) {
    ADD_FAILURE() << "should accept";
  }

  // Rocket accepted
  try {
    TestServiceAsyncClient client(RocketClientChannel::newChannel(
        folly::AsyncSocket::newSocket(&base, *sst.getAddress())));
    client.sync_voidResponse();
  } catch (const TTransportException&) {
    ADD_FAILURE() << "should accept";
  }
}

TEST(ThriftServer, DisableHeaderReject) {
  THRIFT_FLAG_SET_MOCK(server_rocket_upgrade_enabled, true);
  TestThriftServerFactory<TestInterface> factory;
  auto serv = factory.create();
  ScopedServerThread sst(serv);
  serv->setLegacyTransport(
      apache::thrift::BaseThriftServer::LegacyTransport::DISABLED);
  folly::EventBase base;

  // Header traffic rejected
  try {
    Client<TestService> client(HeaderClientChannel::newChannel(
        HeaderClientChannel::WithoutRocketUpgrade{},
        folly::AsyncSocket::newSocket(&base, *sst.getAddress())));
    client.sync_voidResponse();
    ADD_FAILURE() << "should reject";
  } catch (const TTransportException&) {
  }

  // Disabled header reject and connection accepted
  try {
    serv->setLegacyTransport(
        apache::thrift::BaseThriftServer::LegacyTransport::ALLOWED);
    Client<TestService> client(HeaderClientChannel::newChannel(
        HeaderClientChannel::WithRocketUpgrade{},
        folly::AsyncSocket::newSocket(&base, *sst.getAddress())));
    client.sync_voidResponse();
  } catch (const TTransportException&) {
    ADD_FAILURE() << "should accept";
  }
}

// verifying that getThreadManager calls are valid
// in both ResourcePool and non-resorucePool scenario
TEST(ThriftServer, getThreadManager) {
  ScopedServerInterfaceThread runner(std::make_shared<TestInterface>());
  auto tm = runner.getThriftServer().getThreadManager();

  folly::Baton b;

  int a = 0;

  auto fun = [&a, &b]() {
    ++a;
    b.post();
  };

  tm->add(fun);
  b.wait();

  EXPECT_EQ(a, 1);
}

TEST(ThriftServer, acceptConnection) {
  struct Interface1 : public apache::thrift::ServiceHandler<TestService> {
    void echoRequest(
        std::string& _return, std::unique_ptr<std::string>) override {
      _return = "echo";
    }
  };
  struct Interface2 : public apache::thrift::ServiceHandler<TestService> {
    void echoRequest(
        std::string& _return, std::unique_ptr<std::string>) override {
      _return = "echoOverride";
    }
  };
  ScopedServerInterfaceThread runner(std::make_shared<Interface1>());

  auto client1 = runner.newClient<TestServiceAsyncClient>();
  EXPECT_EQ("echo", client1->semifuture_echoRequest("echo").get());

  folly::NetworkSocket fds_rocket[2];
  CHECK(!folly::netops::socketpair(PF_UNIX, SOCK_STREAM, 0, fds_rocket));
  folly::NetworkSocket fds_header[2];
  CHECK(!folly::netops::socketpair(PF_UNIX, SOCK_STREAM, 0, fds_header));

  dynamic_cast<ThriftServer&>(runner.getThriftServer())
      .acceptConnection(fds_rocket[0], {}, {}, std::make_shared<Interface2>());
  dynamic_cast<ThriftServer&>(runner.getThriftServer())
      .acceptConnection(fds_header[0], {}, {}, std::make_shared<Interface2>());

  auto client2_rocket =
      std::make_unique<TestServiceAsyncClient>(PooledRequestChannel::newChannel(
          [fd = fds_rocket[1]](folly::EventBase& evb) {
            return RocketClientChannel::newChannel(
                folly::AsyncSocket::newSocket(&evb, fd));
          },
          1));

  auto client2_header =
      std::make_unique<TestServiceAsyncClient>(PooledRequestChannel::newChannel(
          [fd = fds_header[1]](folly::EventBase& evb) {
            return HeaderClientChannel::newChannel(
                folly::AsyncSocket::newSocket(&evb, fd));
          },
          1));

  EXPECT_EQ(
      "echoOverride", client2_rocket->semifuture_echoRequest("echo").get());
  EXPECT_EQ(
      "echoOverride", client2_header->semifuture_echoRequest("echo").get());
  EXPECT_EQ("echo", client1->semifuture_echoRequest("echo").get());
}

TEST(ThriftServer, SetupThreadManager) {
  ScopedServerInterfaceThread runner(
      std::make_shared<apache::thrift::ServiceHandler<TestService>>(),
      "::1",
      0,
      [](auto& ts) { ts.setupThreadManager(); });
}

TEST(ThriftServer, GetSetMaxRequests) {
  for (auto target : std::array<uint32_t, 2>{1000, 0}) {
    {
      // Test set before setupThreadManager
      ThriftServer server;
      server.setInterface(std::make_shared<TestInterface>());
      server.setMaxRequests(target);
      EXPECT_EQ(server.getMaxRequests(), target);
      // Make the thrift server simple to create
      server.setThreadManagerType(
          apache::thrift::BaseThriftServer::ThreadManagerType::SIMPLE);
      server.setNumCPUWorkerThreads(1);
      server.setupThreadManager();
      EXPECT_EQ(server.getMaxRequests(), target);
      if (server.useResourcePools()) {
        auto concurrencyLimit =
            target == 0 ? std::numeric_limits<decltype(target)>::max() : target;
        EXPECT_EQ(
            concurrencyLimit,
            server.resourcePoolSet()
                .resourcePool(ResourcePoolHandle::defaultAsync())
                .concurrencyController()
                .value()
                .get()
                .getExecutionLimitRequests());
      }
    }
    {
      // Test set after setupThreadManager
      ThriftServer server;
      server.setInterface(std::make_shared<TestInterface>());
      // Make the thrift server simple to create
      server.setThreadManagerType(
          apache::thrift::BaseThriftServer::ThreadManagerType::SIMPLE);
      server.setNumCPUWorkerThreads(1);
      server.setupThreadManager();
      server.setMaxRequests(target);
      EXPECT_EQ(server.getMaxRequests(), target);
      if (server.useResourcePools()) {
        auto concurrencyLimit =
            target == 0 ? std::numeric_limits<decltype(target)>::max() : target;
        EXPECT_EQ(
            concurrencyLimit,
            server.resourcePoolSet()
                .resourcePool(ResourcePoolHandle::defaultAsync())
                .concurrencyController()
                .value()
                .get()
                .getExecutionLimitRequests());
      }
    }
  }
}

TEST(ThriftServer, AddRemoveWorker) {
  ThriftServer server;
  server.setInterface(std::make_shared<TestInterface>());
  server.setupThreadManager();
  auto tm = server.getThreadManager_deprecated();
  auto tc = tm->workerCount();
  tm->addWorker(10);
  EXPECT_EQ(tc + 10, tm->workerCount());
  tm->removeWorker(5);
  EXPECT_EQ(tc + 10 - 5, tm->workerCount());
  EXPECT_THROW(tm->removeWorker(tc + 1), InvalidArgumentException);
}

TEST_P(HeaderOrRocket, setMaxReuqestsToOne) {
  ScopedServerInterfaceThread runner(
      std::make_shared<TestInterface>(), "::1", 0, [](auto&& server) {
        server.setMaxRequests(1);
      });
  auto client = makeClient(runner, nullptr);
  EXPECT_EQ(client->semifuture_sendResponse(42).get(), "test42");
}

namespace {

folly::observer::SimpleObservable<AdaptiveConcurrencyController::Config>
    oConfig{AdaptiveConcurrencyController::Config{}};

}

namespace apache::thrift::detail {

THRIFT_PLUGGABLE_FUNC_SET(
    folly::observer::Observer<
        apache::thrift::AdaptiveConcurrencyController::Config>,
    makeAdaptiveConcurrencyConfig) {
  return oConfig.getObserver();
}

} // namespace apache::thrift::detail

namespace {

static auto makeConfig(size_t concurrency, double jitter = 0.0) {
  AdaptiveConcurrencyController::Config config;
  config.minConcurrency = concurrency;
  config.recalcPeriodJitter = jitter;
  return config;
}

void setConfig(size_t concurrency, double jitter = 0.0) {
  oConfig.setValue(makeConfig(concurrency, jitter));
  folly::observer_detail::ObserverManager::waitForAllUpdates();
}
} // namespace

TEST_P(HeaderOrRocket, AdaptiveConcurrencyConfig) {
  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
   public:
    void voidResponse() override {}
  };

  folly::EventBase base;
  ScopedServerInterfaceThread runner(std::make_shared<TestInterface>());
  runner.getThriftServer().setMaxRequests(5000);
  EXPECT_EQ(runner.getThriftServer().getMaxRequests(), 5000);
  auto& controller = runner.getThriftServer().adaptiveConcurrencyController();
  EXPECT_FALSE(controller.enabled());
  auto client = makeClient(runner, &base);

  setConfig(20, 0);

  // controller will enforce minconcurrency after first request
  EXPECT_FALSE(controller.enabled());
  client->sync_voidResponse();
  EXPECT_TRUE(controller.enabled());
  EXPECT_EQ(runner.getThriftServer().getMaxRequests(), 20);
  // verify controller's limit overrides explicit limit on the server
  runner.getThriftServer().setMaxRequests(2000);
  EXPECT_EQ(runner.getThriftServer().getMaxRequests(), 20);

  setConfig(0, 0);
  EXPECT_FALSE(controller.enabled());
  // disabling controller will not cause the explicit limit to take effect
  EXPECT_EQ(runner.getThriftServer().getMaxRequests(), 20);
}

TEST_P(HeaderOrRocket, OnStartStopServingTest) {
  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
   public:
    folly::Baton<> startEnter;
    folly::Baton<> stopEnter;
    folly::Baton<> backgroundEnter;
    folly::coro::Baton startExit, stopExit, backgroundExit;

    void voidResponse() override {}

    void echoRequest(std::string& result, std::unique_ptr<std::string> req) {
      result = std::move(*req);
    }

    folly::coro::Task<void> co_onStartServing() override {
      startEnter.post();
      co_await getAsyncScope()->co_schedule(co_backgroundTask());
      co_await startExit;
      co_return;
    }

    folly::coro::Task<void> co_backgroundTask() {
      backgroundEnter.post();
      co_await backgroundExit;
      // Wait for cancellation
      const folly::CancellationToken& ct =
          co_await folly::coro::co_current_cancellation_token;
      for (size_t retry = 0; retry < 20 && !ct.isCancellationRequested();
           retry++) {
        co_await folly::coro::sleepReturnEarlyOnCancel(100ms);
      }
      EXPECT_TRUE(ct.isCancellationRequested());
      co_return;
    }

    folly::coro::Task<void> co_onStopRequested() override {
      stopEnter.post();
      co_await stopExit;
      co_return;
    }
  };

  auto testIf = std::make_shared<TestInterface>();

  class TestEventHandler : public server::TServerEventHandler {
   public:
    void preStart(const folly::SocketAddress* address) override {
      this->address = *address;
      preStartEnter.post();
      preStartExit.wait();
    }
    folly::Baton<> preStartEnter, preStartExit;
    folly::SocketAddress address;
  };
  auto preStartHandler = std::make_shared<TestEventHandler>();

  std::unique_ptr<ScopedServerInterfaceThread> runner;

  folly::Baton<> serverSetupBaton;
  std::thread startRunnerThread([&] {
    runner = std::make_unique<ScopedServerInterfaceThread>(
        testIf, "::1", 0, [&](auto& ts) {
          auto tf = std::make_shared<PosixThreadFactory>(
              PosixThreadFactory::ATTACHED);
          // We need at least 2 threads for the test
          ts.setThreadManagerType(
              apache::thrift::BaseThriftServer::ThreadManagerType::SIMPLE);
          ts.setNumCPUWorkerThreads(2);
          ts.setThreadFactory(std::move(tf));
          ts.addServerEventHandler(preStartHandler);
          ts.setInternalMethods({"voidResponse"});
          ts.setRejectRequestsUntilStarted(true);
          serverSetupBaton.post();
        });
  });

  serverSetupBaton.wait();

  // Wait for preStart callback
  EXPECT_TRUE(preStartHandler->preStartEnter.try_wait_for(2s));
  // Provide a single IO thread for the client so that only one connection is
  // created. While the server is stopping, we may not be able to create a new
  // connection (which PooledRequestChannel will do given many IO threads)
  auto clientEvbThread = std::make_shared<folly::ScopedEventBaseThread>();
  TestServiceAsyncClient client(
      apache::thrift::PooledRequestChannel::newSyncChannel(
          clientEvbThread,
          [address = preStartHandler->address,
           this](folly::EventBase& eb) mutable {
            return makeChannel(folly::AsyncSocket::newSocket(&eb, address));
          }));

  client.semifuture_voidResponse().get();
  EXPECT_THROW(
      client.semifuture_echoRequest("echo").get(), TApplicationException);
  EXPECT_FALSE(testIf->startEnter.ready());
  preStartHandler->preStartExit.post();

  // Wait for onStartServing()
  EXPECT_TRUE(testIf->startEnter.try_wait_for(2s));
  client.semifuture_voidResponse().get();
  EXPECT_THROW(
      client.semifuture_echoRequest("echo").get(), TApplicationException);
  testIf->startExit.post();

  startRunnerThread.join();
  // Wait until server is marked as started.
  for (size_t retry = 0; retry < 10; ++retry) {
    try {
      auto echo = client.semifuture_echoRequest("echo").get();
      EXPECT_EQ("echo", echo);
      break;
    } catch (const TApplicationException&) {
      std::this_thread::sleep_for(std::chrono::milliseconds{100});
    }
  }

  client.semifuture_voidResponse().get();
  EXPECT_EQ("echo", client.semifuture_echoRequest("echo").get());

  runner->getThriftServer().setEnabled(false);
  client.semifuture_voidResponse().get();
  EXPECT_THROW(
      client.semifuture_echoRequest("echo").get(), TApplicationException);

  runner->getThriftServer().setEnabled(true);
  client.semifuture_voidResponse().get();
  EXPECT_EQ("echo", client.semifuture_echoRequest("echo").get());

  // Wait for backgroundTask to start
  EXPECT_TRUE(testIf->backgroundEnter.try_wait_for(2s));
  client.semifuture_voidResponse().get();
  testIf->backgroundExit.post();

  // Stop the server on a different thread
  folly::getGlobalIOExecutor()->getEventBase()->runInEventBaseThread(
      [&runner]() { runner.reset(); });

  // Wait for onStopRequested()
  EXPECT_TRUE(testIf->stopEnter.try_wait_for(2s));
  client.semifuture_voidResponse().get();
  // New client for creating a new connection
  TestServiceAsyncClient client2(
      apache::thrift::PooledRequestChannel::newSyncChannel(
          clientEvbThread,
          [address = preStartHandler->address,
           this](folly::EventBase& eb) mutable {
            return makeChannel(folly::AsyncSocket::newSocket(&eb, address));
          }));
  client2.semifuture_voidResponse().get();
  testIf->stopExit.post();
}

TEST_P(HeaderOrRocket, StatusOnStartingAndStopping) {
  // Grab the address after the server starts listening but has not "started"
  // processing user requests.
  class TestEventHandler : public server::TServerEventHandler {
   public:
    void preStart(const folly::SocketAddress* address) override {
      this->address = *address;
      preStartDone.post();
    }
    folly::Baton<> preStartDone;
    folly::SocketAddress address;
  };
  auto preStartHandler = std::make_shared<TestEventHandler>();

  // Block on onStartServing and onStopRequested to allow testing
  class Handler : public apache::thrift::ServiceHandler<TestService> {
   public:
    folly::SemiFuture<folly::Unit> semifuture_onStartServing() override {
      return folly::makeSemiFuture().deferValue([&](auto&&) {
        EXPECT_TRUE(starting.try_wait_for(2s));
        return folly::unit;
      });
    }

    folly::SemiFuture<folly::Unit> semifuture_onStopRequested() override {
      onStopRequestedCalled.post();
      return folly::makeSemiFuture().deferValue([&](auto&&) {
        EXPECT_TRUE(stopping.try_wait_for(2s));
        return folly::unit;
      });
    }

    folly::Baton<> starting;
    folly::Baton<> stopping;
    folly::Baton<> onStopRequestedCalled;
  };
  auto handler = std::make_shared<Handler>();

  class DummyStatusHandler : public apache::thrift::ServiceHandler<DummyStatus>,
                             public StatusServerInterface {
    void async_eb_getStatus(
        std::unique_ptr<HandlerCallback<std::int64_t>> callback) override {
      ThriftServer* server = callback->getRequestContext()
                                 ->getConnectionContext()
                                 ->getWorker()
                                 ->getServer();
      callback->result(static_cast<std::int64_t>(server->getServerStatus()));
    }
  };

  std::unique_ptr<ScopedServerInterfaceThread> runner;
  // Start server in another thread because ScopedServerInterfaceThread blocks
  // until the server has started.
  std::thread startRunnerThread([&] {
    runner = std::make_unique<ScopedServerInterfaceThread>(
        handler, [&](ThriftServer& server) {
          server.setStatusInterface(std::make_shared<DummyStatusHandler>());
          server.addServerEventHandler(preStartHandler);
        });
  });
  // Wait for address to be available
  preStartHandler->preStartDone.wait();

  auto clientEvbThread = std::make_shared<folly::ScopedEventBaseThread>();
  DummyStatusAsyncClient client(PooledRequestChannel::newSyncChannel(
      clientEvbThread,
      [address = preStartHandler->address, this](folly::EventBase& eb) mutable {
        return makeChannel(folly::AsyncSocket::newSocket(&eb, address));
      }));

  EXPECT_EQ(
      client.semifuture_getStatus().get(),
      static_cast<std::int64_t>(ThriftServer::ServerStatus::STARTING));

  handler->starting.post();
  startRunnerThread.join();
  EXPECT_EQ(
      client.semifuture_getStatus().get(),
      static_cast<std::int64_t>(ThriftServer::ServerStatus::RUNNING));

  runner->getThriftServer().stop();
  EXPECT_TRUE(handler->onStopRequestedCalled.try_wait_for(2s));
  EXPECT_EQ(
      client.semifuture_getStatus().get(),
      static_cast<std::int64_t>(ThriftServer::ServerStatus::PRE_STOPPING));
  handler->stopping.post();
}

namespace {
enum class ProcessorImplementation {
  Containment,
  ContainmentLegacy,
  Inheritance
};
} // namespace

class HeaderOrRocketCompression
    : public HeaderOrRocketTest,
      public ::testing::WithParamInterface<
          std::tuple<TransportType, Compression, ProcessorImplementation>> {
 public:
  ProcessorImplementation processorImplementation =
      ProcessorImplementation::Containment;

  void SetUp() override {
    std::tie(transport, compression, processorImplementation) = GetParam();
  }

  std::unique_ptr<AsyncProcessorFactory> makeFactory() {
    // processor can only check for compressed request payload on rocket
    if (compression == Compression::Enabled &&
        transport == TransportType::Rocket) {
      return std::make_unique<CompressionCheckTestInterface>(
          processorImplementation, CompressionAlgorithm::ZSTD);
    } else {
      return std::make_unique<CompressionCheckTestInterface>(
          processorImplementation, CompressionAlgorithm::NONE);
    }
  }

 private:
  // Custom processor derived from AsyncProcessor with compressed request API
  struct ContainmentCompressionCheckProcessor : public AsyncProcessor {
    ContainmentCompressionCheckProcessor(
        std::unique_ptr<AsyncProcessor> underlyingProcessor,
        CompressionAlgorithm compression)
        : underlyingProcessor_(std::move(underlyingProcessor)),
          compression_(compression) {}
    void executeRequest(
        ServerRequest&&,
        const AsyncProcessorFactory::MethodMetadata&) override {
      LOG(FATAL) << "executeRequest shouldn't be called in this test";
    }

    void processSerializedCompressedRequestWithMetadata(
        ResponseChannelRequest::UniquePtr req,
        SerializedCompressedRequest&& serializedRequest,
        const AsyncProcessorFactory::MethodMetadata& methodMetadata,
        protocol::PROTOCOL_TYPES prot,
        Cpp2RequestContext* ctx,
        folly::EventBase* eb,
        concurrency::ThreadManager* tm) override {
      // check that SerializedCompressedRequest has expected compression
      EXPECT_EQ(compression_, serializedRequest.getCompressionAlgorithm());
      underlyingProcessor_->processSerializedCompressedRequestWithMetadata(
          std::move(req),
          std::move(serializedRequest),
          methodMetadata,
          prot,
          ctx,
          eb,
          tm);
    }

    std::unique_ptr<AsyncProcessor> underlyingProcessor_;
    CompressionAlgorithm compression_;
  };

  // Custom processor derived from AsyncProcessor without compressed request API
  struct LegacyContainmentProcessor : public AsyncProcessor {
    explicit LegacyContainmentProcessor(
        std::unique_ptr<AsyncProcessor> underlyingProcessor)
        : underlyingProcessor_(std::move(underlyingProcessor)) {}

    void executeRequest(
        ServerRequest&&,
        const AsyncProcessorFactory::MethodMetadata&) override {
      LOG(FATAL) << "executeRequest shouldn't be called in this test";
    }

    void processSerializedCompressedRequestWithMetadata(
        ResponseChannelRequest::UniquePtr req,
        SerializedCompressedRequest&& serializedRequest,
        const AsyncProcessorFactory::MethodMetadata& methodMetadata,
        protocol::PROTOCOL_TYPES prot,
        Cpp2RequestContext* ctx,
        folly::EventBase* eb,
        concurrency::ThreadManager* tm) override {
      underlyingProcessor_->processSerializedCompressedRequestWithMetadata(
          std::move(req),
          std::move(serializedRequest),
          methodMetadata,
          prot,
          ctx,
          eb,
          tm);
    }

    std::unique_ptr<AsyncProcessor> underlyingProcessor_;
  };

  // Custom processor derived from generated processor
  struct InheritanceCompressionCheckProcessor
      : public TestInterface::ProcessorType {
    InheritanceCompressionCheckProcessor(
        apache::thrift::ServiceHandler<TestService>* iface,
        CompressionAlgorithm compression)
        : TestInterface::ProcessorType(iface), compression_(compression) {}

    void executeRequest(
        ServerRequest&&,
        const AsyncProcessorFactory::MethodMetadata&) override {
      LOG(FATAL) << "executeRequest shouldn't be called in this test";
    }

    void processSerializedCompressedRequestWithMetadata(
        apache::thrift::ResponseChannelRequest::UniquePtr req,
        apache::thrift::SerializedCompressedRequest&& serializedRequest,
        const apache::thrift::AsyncProcessorFactory::MethodMetadata& mm,
        apache::thrift::protocol::PROTOCOL_TYPES prot,
        apache::thrift::Cpp2RequestContext* ctx,
        folly::EventBase* eb,
        apache::thrift::concurrency::ThreadManager* tm) override {
      // check that SerializedCompressedRequest has expected compression
      EXPECT_EQ(compression_, serializedRequest.getCompressionAlgorithm());
      TestInterface::ProcessorType::
          processSerializedCompressedRequestWithMetadata(
              std::move(req),
              std::move(serializedRequest),
              mm,
              prot,
              ctx,
              eb,
              tm);
    }

    CompressionAlgorithm compression_;
  };

  struct CompressionCheckTestInterface : public TestInterface {
    CompressionCheckTestInterface(
        ProcessorImplementation processorImplementation,
        CompressionAlgorithm compression)
        : processorImplementation_(processorImplementation),
          compression_(compression) {}
    std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
      switch (processorImplementation_) {
        case ProcessorImplementation::Containment:
          return std::make_unique<ContainmentCompressionCheckProcessor>(
              std::make_unique<TestInterface::ProcessorType>(this),
              compression_);
        case ProcessorImplementation::ContainmentLegacy:
          return std::make_unique<LegacyContainmentProcessor>(
              std::make_unique<TestInterface::ProcessorType>(this));
        case ProcessorImplementation::Inheritance:
          return std::make_unique<InheritanceCompressionCheckProcessor>(
              this, compression_);
      }
    }

    CreateMethodMetadataResult createMethodMetadata() override {
      // We want to return WildcardMethodMetadataMap here because default
      // implementation will return MethodMetadataMap and requests will be
      // routed to executeRequest rather than to
      // processSerializedCompressedRequestWithMetadata. This test relies on
      // routing requests to processSerializedCompressedRequestWithMetadata.
      WildcardMethodMetadataMap wildcardMap;
      wildcardMap.wildcardMetadata = std::make_shared<WildcardMethodMetadata>();
      wildcardMap.knownMethods = {};
      return wildcardMap;
    }

    ProcessorImplementation processorImplementation_;
    CompressionAlgorithm compression_;
  };
};

TEST_P(HeaderOrRocketCompression, ClientCompressionTest) {
  THRIFT_OMIT_TEST_WITH_RESOURCE_POOLS(
      /* Test mock does not implement executeRequest */);
  folly::EventBase base;
  ScopedServerInterfaceThread runner(makeFactory());
  auto client = makeClient(runner, &base);

  std::string response;
  client->sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
}

INSTANTIATE_TEST_CASE_P(
    HeaderOrRocketCompression,
    HeaderOrRocketCompression,
    ::testing::Combine(
        testing::Values(TransportType::Header, TransportType::Rocket),
        testing::Values(Compression::Enabled, Compression::Disabled),
        testing::Values(
            ProcessorImplementation::Containment,
            ProcessorImplementation::ContainmentLegacy,
            ProcessorImplementation::Inheritance)));
