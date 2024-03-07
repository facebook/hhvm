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

#include "wangle/bootstrap/ClientBootstrap.h"
#include "wangle/bootstrap/ServerBootstrap.h"
#include "wangle/channel/Handler.h"

#include <boost/thread.hpp>
#include <folly/String.h>
#include <folly/experimental/TestUtil.h>
#include <folly/portability/GTest.h>
#include <glog/logging.h>

using namespace wangle;
using namespace folly;

using BytesPipeline = Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>>;

using TestServer = ServerBootstrap<BytesPipeline>;
using TestClient = ClientBootstrap<BytesPipeline>;

class TestClientPipelineFactory : public PipelineFactory<BytesPipeline> {
 public:
  BytesPipeline::Ptr newPipeline(
      std::shared_ptr<AsyncTransport> sock) override {
    // Socket should be connected already
    EXPECT_TRUE(sock->good());

    // Check after a small delay that socket is readable
    EventBaseManager::get()->getEventBase()->tryRunAfterDelay(
        [sock]() { EXPECT_TRUE(sock->readable()); }, 100);

    auto pipeline = BytesPipeline::create();
    pipeline->addBack(new BytesToBytesHandler());
    pipeline->finalize();
    return pipeline;
  }
};

class TestPipelineFactory : public PipelineFactory<BytesPipeline> {
 public:
  BytesPipeline::Ptr newPipeline(std::shared_ptr<AsyncTransport>) override {
    pipelines++;
    auto pipeline = BytesPipeline::create();
    pipeline->addBack(new BytesToBytesHandler());
    pipeline->finalize();
    return pipeline;
  }
  std::atomic<int> pipelines{0};
};

class TestAcceptor : public Acceptor {
  EventBase base_;

 public:
  TestAcceptor() : Acceptor(ServerSocketConfig()) {
    Acceptor::init(nullptr, &base_);
  }
  void onNewConnection(
      AsyncTransport::UniquePtr,
      const folly::SocketAddress*,
      const std::string& /* nextProtocolName */,
      SecureTransportType,
      const TransportInfo&) override {}
};

class TestAcceptorFactory : public AcceptorFactory {
 public:
  std::shared_ptr<Acceptor> newAcceptor(EventBase*) override {
    return std::make_shared<TestAcceptor>();
  }
};

TEST(Bootstrap, Basic) {
  TestServer server;
  TestClient client;
}

TEST(Bootstrap, ServerWithPipeline) {
  TestServer server;
  server.childPipeline(std::make_shared<TestPipelineFactory>());
  server.bind(0);
  server.stop();
}

TEST(Bootstrap, ServerWithChildHandler) {
  TestServer server;
  server.childHandler(std::make_shared<TestAcceptorFactory>());
  server.bind(0);
  server.stop();
}

TEST(Bootstrap, ClientServerTest) {
  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();
  server.childPipeline(factory);
  server.bind(0);
  auto base = EventBaseManager::get()->getEventBase();

  SocketAddress address;
  server.getSockets()[0]->getAddress(&address);

  TestClient client;
  client.pipelineFactory(std::make_shared<TestClientPipelineFactory>());
  client.connect(address);
  base->loop();
  server.stop();
  server.join();

  EXPECT_EQ(factory->pipelines, 1);
}

TEST(Bootstrap, ClientConnectionManagerTest) {
  // Create a single IO thread, and verify that
  // client connections are pooled properly

  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();
  server.childPipeline(factory);
  server.setUseSharedSSLContextManager(true);
  server.group(std::make_shared<IOThreadPoolExecutor>(1));
  server.bind(0);
  auto base = EventBaseManager::get()->getEventBase();

  SocketAddress address;
  server.getSockets()[0]->getAddress(&address);

  TestClient client;
  client.pipelineFactory(std::make_shared<TestClientPipelineFactory>());

  client.connect(address);

  TestClient client2;
  client2.pipelineFactory(std::make_shared<TestClientPipelineFactory>());
  client2.connect(address);

  base->loop();
  server.stop();
  server.join();

  EXPECT_EQ(factory->pipelines, 2);
}

TEST(Bootstrap, ServerAcceptGroupTest) {
  // Verify that server is using the accept IO group

  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();
  server.childPipeline(factory);
  server.setUseSharedSSLContextManager(true);
  server.group(std::make_shared<IOThreadPoolExecutor>(1), nullptr);
  server.bind(0);

  SocketAddress address;
  server.getSockets()[0]->getAddress(&address);

  boost::barrier barrier(2);
  auto thread = std::thread([&]() {
    TestClient client;
    client.pipelineFactory(std::make_shared<TestClientPipelineFactory>());
    client.connect(address);
    EventBaseManager::get()->getEventBase()->loop();
    barrier.wait();
  });
  barrier.wait();
  server.stop();
  thread.join();
  server.join();

  EXPECT_EQ(factory->pipelines, 1);
}

TEST(Bootstrap, ServerAcceptGroup2Test) {
  // Verify that server is using the accept IO group

  // Check if reuse port is supported, if not, don't run this test
  try {
    EventBase base;
    auto serverSocket = AsyncServerSocket::newSocket(&base);
    serverSocket->bind(0);
    serverSocket->listen(0);
    serverSocket->startAccepting();
    serverSocket->setReusePortEnabled(true);
    serverSocket->stopAccepting();
  } catch (...) {
    LOG(INFO) << "Reuse port probably not supported";
    return;
  }

  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();
  server.childPipeline(factory);
  server.setUseSharedSSLContextManager(true);
  server.group(std::make_shared<IOThreadPoolExecutor>(4), nullptr);
  server.bind(0);

  SocketAddress address;
  server.getSockets()[0]->getAddress(&address);

  TestClient client;
  client.pipelineFactory(std::make_shared<TestClientPipelineFactory>());

  client.connect(address);
  EventBaseManager::get()->getEventBase()->loop();

  server.stop();
  server.join();

  EXPECT_EQ(factory->pipelines, 1);
}

TEST(Bootstrap, SharedThreadPool) {
  // Check if reuse port is supported, if not, don't run this test
  try {
    EventBase base;
    auto serverSocket = AsyncServerSocket::newSocket(&base);
    serverSocket->bind(0);
    serverSocket->listen(0);
    serverSocket->startAccepting();
    serverSocket->setReusePortEnabled(true);
    serverSocket->stopAccepting();
  } catch (...) {
    LOG(INFO) << "Reuse port probably not supported";
    return;
  }

  auto pool = std::make_shared<IOThreadPoolExecutor>(2);

  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();
  server.childPipeline(factory);
  server.setUseSharedSSLContextManager(true);
  server.group(pool, pool);

  server.bind(0);

  SocketAddress address;
  server.getSockets()[0]->getAddress(&address);

  TestClient client;
  client.pipelineFactory(std::make_shared<TestClientPipelineFactory>());
  client.connect(address);

  TestClient client2;
  client2.pipelineFactory(std::make_shared<TestClientPipelineFactory>());
  client2.connect(address);

  TestClient client3;
  client3.pipelineFactory(std::make_shared<TestClientPipelineFactory>());
  client3.connect(address);

  TestClient client4;
  client4.pipelineFactory(std::make_shared<TestClientPipelineFactory>());
  client4.connect(address);

  TestClient client5;
  client5.pipelineFactory(std::make_shared<TestClientPipelineFactory>());
  client5.connect(address);

  EventBaseManager::get()->getEventBase()->loop();

  server.stop();
  server.join();

  EXPECT_EQ(factory->pipelines, 5);
}

TEST(Bootstrap, ExistingSocket) {
  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();
  server.childPipeline(factory);
  folly::AsyncServerSocket::UniquePtr socket(new AsyncServerSocket);
  server.bind(std::move(socket));
}

std::atomic<int> connections{0};

class TestHandlerPipeline : public InboundHandler<AcceptPipelineType> {
 public:
  void read(Context* ctx, AcceptPipelineType conn) override {
    if (conn.type() == typeid(ConnEvent)) {
      auto connEvent = boost::get<ConnEvent>(conn);
      if (connEvent == ConnEvent::CONN_ADDED) {
        connections++;
      }
    }
    return ctx->fireRead(conn);
  }
};

template <typename HandlerPipeline>
class TestHandlerPipelineFactory : public AcceptPipelineFactory {
 public:
  AcceptPipeline::Ptr newPipeline(Acceptor*) override {
    auto pipeline = AcceptPipeline::create();
    pipeline->addBack(HandlerPipeline());
    return pipeline;
  }
};

TEST(Bootstrap, LoadBalanceHandler) {
  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();
  server.childPipeline(factory);

  auto pipelinefactory =
      std::make_shared<TestHandlerPipelineFactory<TestHandlerPipeline>>();
  server.pipeline(pipelinefactory);
  server.bind(0);
  auto base = EventBaseManager::get()->getEventBase();

  SocketAddress address;
  server.getSockets()[0]->getAddress(&address);

  TestClient client;
  client.pipelineFactory(std::make_shared<TestClientPipelineFactory>());
  client.connect(address);
  base->loop();
  server.stop();
  server.join();

  EXPECT_EQ(factory->pipelines, 1);
  EXPECT_EQ(connections, 1);
}

class TestUDPPipeline : public InboundHandler<AcceptPipelineType, Unit> {
 public:
  void read(Context*, AcceptPipelineType) override {
    connections++;
  }
};

TEST(Bootstrap, UDP) {
  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();
  auto pipelinefactory =
      std::make_shared<TestHandlerPipelineFactory<TestUDPPipeline>>();
  server.pipeline(pipelinefactory);
  server.channelFactory(std::make_shared<AsyncUDPServerSocketFactory>());
  server.bind(0);
}

TEST(Bootstrap, UDPClientServerTest) {
  connections = 0;

  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();
  auto pipelinefactory =
      std::make_shared<TestHandlerPipelineFactory<TestUDPPipeline>>();
  server.pipeline(pipelinefactory);
  server.channelFactory(std::make_shared<AsyncUDPServerSocketFactory>());
  server.bind(0);

  auto base = EventBaseManager::get()->getEventBase();

  SocketAddress address;
  server.getSockets()[0]->getAddress(&address);

  SocketAddress localhost("::1", 0);
  AsyncUDPSocket client(base);
  client.bind(localhost);
  auto data = IOBuf::create(1);
  data->append(1);
  *(data->writableData()) = 'a';
  client.write(address, std::move(data));
  base->loop();
  server.stop();
  server.join();

  EXPECT_EQ(connections, 1);
}

TEST(Bootstrap, UnixServer) {
  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();

  folly::test::TemporaryDirectory tmpdir("wangle-bootstrap-test");
  auto socketPath = (tmpdir.path() / "sock").string();

  server.childPipeline(factory);
  SocketAddress address;
  address.setFromPath(socketPath);
  server.bind(address);
  auto base = EventBaseManager::get()->getEventBase();

  TestClient client;
  client.pipelineFactory(std::make_shared<TestClientPipelineFactory>());
  auto pipelineFuture = client.connect(address);
  base->loop();
  server.stop();
  server.join();

  EXPECT_TRUE(std::move(pipelineFuture).get() != nullptr);
  EXPECT_EQ(factory->pipelines, 1);
}

TEST(Bootstrap, ServerBindFailure) {
  // Bind to a TCP socket
  EventBase base;
  auto serverSocket = AsyncServerSocket::newSocket(&base);
  serverSocket->bind(0);
  serverSocket->listen(0);

  SocketAddress address;
  serverSocket->getAddress(&address);

  // Now try starting a server using the address we are already listening on
  // This should fail.

  TestServer server;
  auto factory = std::make_shared<TestPipelineFactory>();
  server.childPipeline(factory);
  try {
    server.bind(address);
    FAIL() << "shouldn't be allowed to bind to an in-use address";
  } catch (const std::system_error& ex) {
    EXPECT_EQ(EADDRINUSE, ex.code().value())
        << "unexpected error code " << ex.code().value() << ": "
        << ex.code().message();
  }
}
