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

#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <folly/CancellationToken.h>
#include <folly/Optional.h>
#include <folly/Synchronized.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/fibers/FiberManagerMap.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/util/TestInterface.h>
#include <thrift/lib/cpp2/test/util/TestThriftServerFactory.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::util;
using namespace apache::thrift::transport;
using apache::thrift::protocol::PROTOCOL_TYPES;

DECLARE_int32(thrift_cpp2_protocol_reader_string_limit);

namespace {

enum ThriftServerTypes {
  THRIFT_SERVER,
};

enum ClientChannelTypes {
  HEADER,
  HTTP2,
};

struct TestHeaderClientChannelFactory {
 public:
  apache::thrift::ClientChannel::Ptr create(
      folly::AsyncTransport::UniquePtr socket) {
    auto channel = apache::thrift::HeaderClientChannel::newChannel(
        std::move(socket),
        apache::thrift::HeaderClientChannel::Options().setProtocolId(
            protocol_));

    channel->setTimeout(timeout_);

    return channel;
  }

  void setProtocolId(apache::thrift::protocol::PROTOCOL_TYPES protocol) {
    protocol_ = protocol;
  }

  void setTimeout(uint32_t timeout) { timeout_ = timeout; }

 private:
  apache::thrift::protocol::PROTOCOL_TYPES protocol_{
      apache::thrift::protocol::T_COMPACT_PROTOCOL};
  uint32_t timeout_{5000};
};

class SharedServerTests
    : public testing::TestWithParam<
          std::tuple<ThriftServerTypes, ClientChannelTypes, PROTOCOL_TYPES>> {
 protected:
  void SetUp() override {
    base.reset(new folly::EventBase);

    auto protocolId = std::get<2>(GetParam());

    switch (std::get<0>(GetParam())) {
      case THRIFT_SERVER: {
        auto f = std::make_unique<TestThriftServerFactory<TestInterface>>();
        serverFactory = std::move(f);
        break;
      }
      default:
        FAIL();
    }

    switch (std::get<1>(GetParam())) {
      case HEADER: {
        auto c = std::make_unique<TestHeaderClientChannelFactory>();
        c->setProtocolId(protocolId);
        channelFactory = std::move(c);
        break;
      }
      default:
        FAIL();
    }
  }

  void createServer() { server = serverFactory->create(); }

  void startServer() {
    if (!server) {
      createServer();
    }
    sst = std::make_unique<ScopedServerThread>(server);
  }

  void createSocket() {
    if (!sst) {
      startServer();
    }
    socket = folly::AsyncTransport::UniquePtr(
        new folly::AsyncSocket(base.get(), *sst->getAddress()));
  }

  void createChannel() {
    if (!socket) {
      createSocket();
    }
    channel = channelFactory->create(std::move(socket));
  }

  void createClient() {
    if (!channel) {
      createChannel();
    }
    client = std::make_unique<TestServiceAsyncClient>(std::move(channel));
  }

  void init() {
    createServer();
    startServer();
    createSocket();
    createChannel();
    createClient();
  }

  void TearDown() override {
    client.reset();
    channel.reset();
    socket.reset();
    sst.reset();
    server.reset();
    channelFactory.reset();
    serverFactory.reset();
    base.reset();
  }

 protected:
  std::unique_ptr<folly::EventBase> base;
  std::unique_ptr<TestServerFactory> serverFactory{nullptr};
  std::shared_ptr<TestHeaderClientChannelFactory> channelFactory{nullptr};

  std::shared_ptr<BaseThriftServer> server{nullptr};
  std::unique_ptr<ScopedServerThread> sst{nullptr};

  folly::AsyncTransport::UniquePtr socket{nullptr};
  apache::thrift::ClientChannel::Ptr channel{nullptr};
  std::unique_ptr<TestServiceAsyncClient> client{nullptr};
};
} // namespace

TEST_P(SharedServerTests, AsyncThrift2Test) {
  init();

  client->sendResponse(
      [&](ClientReceiveState&& state) {
        std::string response;
        try {
          TestServiceAsyncClient::recv_sendResponse(response, state);
        } catch (const std::exception&) {
        }
        EXPECT_EQ(response, "test64");
        base->terminateLoopSoon();
      },
      64);
  base->loop();
}

TEST_P(SharedServerTests, GetLoadTest) {
  init();

  RpcOptions rpcOptions;
  rpcOptions.setWriteHeader("load", "thrift.active_requests");
  auto callback = std::unique_ptr<RequestCallback>(
      new FunctionReplyCallback([&](ClientReceiveState&& state) {
        std::string response;
        auto headers = state.header()->getHeaders();
        auto load = headers.find("load");
        EXPECT_NE(load, headers.end());
        EXPECT_NE(load->second, "");
        TestServiceAsyncClient::recv_wrapped_sendResponse(response, state);
        EXPECT_EQ(response, "test64");
        base->terminateLoopSoon();
      }));
  client->sendResponse(rpcOptions, std::move(callback), 64);
  base->loop();

  server->setGetLoad([&](std::string counter) {
    EXPECT_EQ(counter, "thrift.active_requests");
    return 1;
  });

  rpcOptions.setWriteHeader("load", "thrift.active_requests");
  callback = std::unique_ptr<RequestCallback>(
      new FunctionReplyCallback([&](ClientReceiveState&& state) {
        std::string response;
        auto headers = state.header()->getHeaders();
        auto load = headers.find("load");
        EXPECT_NE(load, headers.end());
        EXPECT_NE(load->second, "");
        TestServiceAsyncClient::recv_wrapped_sendResponse(response, state);
        EXPECT_EQ(response, "test64");
        base->terminateLoopSoon();
      }));
  client->sendResponse(rpcOptions, std::move(callback), 64);
  base->loop();
}

TEST_P(SharedServerTests, SerializationInEventBaseTest) {
  init();

  std::string response;
  client->sync_serializationTest(response, true);
  EXPECT_EQ("hello world", response);
}

TEST_P(SharedServerTests, HandlerInEventBaseTest) {
  init();

  std::string response;
  client->sync_eventBaseAsync(response);
  EXPECT_EQ("hello world", response);
}

bool compareIOBufChain(const folly::IOBuf* buf1, const folly::IOBuf* buf2) {
  folly::io::Cursor c1(buf1);
  folly::io::Cursor c2(buf2);
  folly::ByteRange b1;
  folly::ByteRange b2;
  while (1) {
    if (b1.empty()) {
      b1 = c1.peekBytes();
      c1.skip(b1.size());
    }
    if (b2.empty()) {
      b2 = c2.peekBytes();
      c2.skip(b2.size());
    }
    if (b1.empty() || b2.empty()) {
      // one is finished, the other must be finished too
      return b1.empty() && b2.empty();
    }

    size_t m = std::min(b1.size(), b2.size());
    if (memcmp(b1.data(), b2.data(), m) != 0) {
      return false;
    }
    b1.advance(m);
    b2.advance(m);
  }
}

TEST_P(SharedServerTests, LargeSendTest) {
  channelFactory->setTimeout(45000);
  init();

  std::unique_ptr<folly::IOBuf> response;
  std::unique_ptr<folly::IOBuf> request;

  constexpr size_t oneMB = 1 << 20;
  std::string oneMBBuf;
  oneMBBuf.reserve(oneMB);
  for (uint32_t i = 0; i < (1 << 20) / 32; i++) {
    oneMBBuf.append(32, char(i & 0xff));
  }
  ASSERT_EQ(oneMBBuf.size(), oneMB);

  // A bit more than 1GiB, which used to be the max frame size
  constexpr size_t hugeSize = (size_t(1) << 30) + (1 << 10);
  request = folly::IOBuf::wrapBuffer(oneMBBuf.data(), oneMB);
  for (uint32_t i = 0; i < hugeSize / oneMB - 1; i++) {
    request->prependChain(folly::IOBuf::wrapBuffer(oneMBBuf.data(), oneMB));
  }
  request->prependChain(
      folly::IOBuf::wrapBuffer(oneMBBuf.data(), hugeSize % oneMB));
  ASSERT_EQ(request->computeChainDataLength(), hugeSize);

  client->sync_echoIOBuf(response, *request);
  ASSERT_EQ(
      request->computeChainDataLength() + kEchoSuffix.size(),
      response->computeChainDataLength());

  // response = request + kEchoSuffix. Make sure it's so
  request->prependChain(
      folly::IOBuf::wrapBuffer(kEchoSuffix.data(), kEchoSuffix.size()));
  // Not EXPECT_EQ; do you want to print two >1GiB strings on error?
  EXPECT_TRUE(compareIOBufChain(request.get(), response.get()));
}

TEST_P(SharedServerTests, OnewaySyncClientTest) {
  init();

  client->sync_noResponse(0);
}

TEST_P(SharedServerTests, ThriftServerSizeLimits) {
  init();

  gflags::FlagSaver flagSaver;
  FLAGS_thrift_cpp2_protocol_reader_string_limit = 1024 * 1024;

  std::string response;

  // make a largest possible input which should not throw an exception
  std::string smallInput(1 << 19, '1');
  client->sync_echoRequest(response, smallInput);

  // make an input that is too large by 1 byte
  std::string largeInput(1 << 21, '1');
  EXPECT_THROW(client->sync_echoRequest(response, largeInput), std::exception);
}

namespace {
class FiberExecutor : public folly::Executor {
 public:
  void add(folly::Func f) override {
    folly::fibers::getFiberManager(*folly::getEventBase()).add(std::move(f));
  }
};
} // namespace

TEST_P(SharedServerTests, FiberExecutorTest) {
  serverFactory->setServerSetupFunction([](BaseThriftServer& server) {
    server.setThreadManagerType(
        apache::thrift::BaseThriftServer::ThreadManagerType::EXECUTOR_ADAPTER);
    server.setThreadManagerExecutor(std::make_shared<FiberExecutor>());
  });

  init();

  std::string response;

  client->sync_sendResponse(response, 1);
  EXPECT_EQ("test1", response);
}

TEST_P(SharedServerTests, FreeCallbackTest) {
  init();

  RpcOptions options;
  options.setTimeout(std::chrono::milliseconds(1));

  try {
    client->sync_notCalledBack(options);
  } catch (...) {
    // Expect timeout
    return;
  }
  ADD_FAILURE();
}

namespace {
class TestServerEventHandler : public server::TServerEventHandler,
                               public TProcessorEventHandler {
 public:
  void check() { EXPECT_EQ(8, count); }
  void preServe(const folly::SocketAddress*) override { EXPECT_EQ(0, count++); }
  void newConnection(TConnectionContext*) override { EXPECT_EQ(1, count++); }
  void connectionDestroyed(TConnectionContext*) override {
    EXPECT_EQ(7, count++);
  }

  void* getContext(const char*, TConnectionContext*) override {
    EXPECT_EQ(2, count++);
    return nullptr;
  }
  void freeContext(void*, const char*) override { EXPECT_EQ(6, count++); }
  void preRead(void*, const char*) override { EXPECT_EQ(3, count++); }
  void onReadData(void*, const char*, const SerializedMessage&) override {
    EXPECT_EQ(4, count++);
  }

  void postRead(void*, const char*, THeader*, uint32_t) override {
    EXPECT_EQ(5, count++);
  }

 private:
  std::atomic<int> count{0};
};
} // namespace

TEST_P(SharedServerTests, CallbackOrderingTest) {
  auto serverHandler = std::make_shared<TestServerEventHandler>();
  TProcessorBase::addProcessorEventHandler(serverHandler);
  serverFactory->setServerEventHandler(serverHandler);

  init();

  auto channel = static_cast<ClientChannel*>(client->getChannel());
  auto socket = channel->getTransport();
  client->noResponse([](ClientReceiveState&&) {}, 1000);
  base->tryRunAfterDelay([&]() { socket->closeNow(); }, 100);
  base->tryRunAfterDelay([&]() { base->terminateLoopSoon(); }, 500);
  base->loopForever();
  serverHandler->check();
  TProcessorBase::removeProcessorEventHandler(serverHandler);
}

using testing::Combine;
using testing::Values;

INSTANTIATE_TEST_CASE_P(
    ThriftServerTests,
    SharedServerTests,
    Combine(
        Values(ThriftServerTypes::THRIFT_SERVER),
        Values(ClientChannelTypes::HEADER),
        Values(protocol::T_BINARY_PROTOCOL, protocol::T_COMPACT_PROTOCOL)));
