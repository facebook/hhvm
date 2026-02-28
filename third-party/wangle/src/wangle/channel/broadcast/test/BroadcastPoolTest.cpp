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

#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/broadcast/BroadcastPool.h>
#include <wangle/channel/broadcast/test/Mocks.h>

#include "wangle/channel/Handler.h"

using namespace folly;
using namespace testing;
using namespace wangle;

class BroadcastPoolTest : public Test {
 public:
  void SetUp() override {
    addr = std::make_shared<SocketAddress>();
    serverPool = std::make_shared<StrictMock<MockServerPool>>(addr);

    pipelineFactory =
        std::make_shared<StrictMock<MockBroadcastPipelineFactory>>();

    pool = std::make_unique<BroadcastPool<int, std::string>>(
        serverPool,
        pipelineFactory,
        std::make_shared<ClientBootstrapFactory>());

    startServer();
  }

  void TearDown() override {
    Mock::VerifyAndClear(serverPool.get());
    Mock::VerifyAndClear(pipelineFactory.get());

    serverPool.reset();
    addr.reset();
    pipelineFactory.reset();
    pool.reset();

    stopServer();
  }

 protected:
  class ServerPipelineFactory : public PipelineFactory<DefaultPipeline> {
   public:
    DefaultPipeline::Ptr newPipeline(std::shared_ptr<AsyncTransport>) override {
      auto pipeline = DefaultPipeline::create();
      pipeline->addBack(new BytesToBytesHandler());
      pipeline->finalize();
      return pipeline;
    }
  };

  void startServer() {
    server = std::make_unique<ServerBootstrap<DefaultPipeline>>();
    server->childPipeline(std::make_shared<ServerPipelineFactory>());
    server->bind(0);
    server->getSockets()[0]->getAddress(addr.get());
  }

  void stopServer() {
    server.reset();
  }

  std::unique_ptr<BroadcastPool<int, std::string>> pool;
  std::shared_ptr<StrictMock<MockServerPool>> serverPool;
  std::shared_ptr<StrictMock<MockBroadcastPipelineFactory>> pipelineFactory;
  NiceMock<MockSubscriber<int, std::string>> subscriber;
  std::unique_ptr<ServerBootstrap<DefaultPipeline>> server;
  std::shared_ptr<SocketAddress> addr;
};

TEST_F(BroadcastPoolTest, BasicConnect) {
  // Test simple calls to getHandler()
  std::string routingData1 = "url1";
  std::string routingData2 = "url2";
  BroadcastHandler<int, std::string>* handler1 = nullptr;
  BroadcastHandler<int, std::string>* handler2 = nullptr;
  uint64_t handler1Id = 0;
  uint64_t handler2Id = 0;
  auto base = EventBaseManager::get()->getEventBase();

  InSequence dummy;

  // No broadcast available for routingData1. Test that a new connection
  // is established and handler created.
  EXPECT_FALSE(pool->isBroadcasting(routingData1));
  pool->getHandler(routingData1)
      .thenValue([&](BroadcastHandler<int, std::string>* h) {
        handler1 = h;
        handler1Id = handler1->getArbitraryIdentifier();
        handler1->subscribe(&subscriber);
      });
  EXPECT_TRUE(handler1 == nullptr);
  EXPECT_CALL(*pipelineFactory, setRoutingData(_, "url1")).Times(1);
  base->loopOnce(); // Do async connect
  ASSERT_TRUE(handler1 != nullptr);
  EXPECT_NE(0, handler1Id);
  EXPECT_TRUE(pool->isBroadcasting(routingData1));

  // Broadcast available for routingData1. Test that the same handler
  // is returned.
  pool->getHandler(routingData1)
      .thenValue([&](BroadcastHandler<int, std::string>* h) {
        EXPECT_EQ(handler1Id, h->getArbitraryIdentifier());
        EXPECT_TRUE(h == handler1);
      })
      .wait();
  EXPECT_TRUE(pool->isBroadcasting(routingData1));

  // Close the handler. This will delete the pipeline and the broadcast.
  handler1->readEOF(handler1->getContext());
  EXPECT_FALSE(pool->isBroadcasting(routingData1));

  // routingData1 doesn't have an available broadcast now. Test that a
  // new connection is established again and handler created.
  handler1 = nullptr;
  pool->getHandler(routingData1)
      .thenValue([&](BroadcastHandler<int, std::string>* h) {
        handler1 = h;
        handler1Id = handler1->getArbitraryIdentifier();
        handler1->subscribe(&subscriber);
      });
  EXPECT_TRUE(handler1 == nullptr);
  EXPECT_CALL(*pipelineFactory, setRoutingData(_, "url1")).Times(1);
  base->loopOnce(); // Do async connect
  EXPECT_TRUE(handler1 != nullptr);
  EXPECT_TRUE(pool->isBroadcasting(routingData1));

  // Cleanup
  handler1->readEOF(handler1->getContext());

  // Test that a new connection is established for routingData2 with
  // a new handler created
  EXPECT_FALSE(pool->isBroadcasting(routingData2));
  pool->getHandler(routingData2)
      .thenValue([&](BroadcastHandler<int, std::string>* h) {
        handler2 = h;
        handler2Id = handler2->getArbitraryIdentifier();
        handler2->subscribe(&subscriber);
      });
  EXPECT_TRUE(handler2 == nullptr);
  EXPECT_CALL(*pipelineFactory, setRoutingData(_, "url2")).Times(1);
  base->loopOnce(); // Do async connect
  EXPECT_TRUE(handler2 != nullptr);
  EXPECT_TRUE(handler2Id != handler1Id);
  EXPECT_TRUE(pool->isBroadcasting(routingData2));

  // Cleanup
  handler2->readEOF(handler2->getContext());
}

TEST_F(BroadcastPoolTest, OutstandingConnect) {
  // Test with multiple getHandler() calls for the same routing data
  // when a connect request is in flight
  std::string routingData = "url1";
  BroadcastHandler<int, std::string>* handler1 = nullptr;
  BroadcastHandler<int, std::string>* handler2 = nullptr;
  auto base = EventBaseManager::get()->getEventBase();

  InSequence dummy;

  // No broadcast available for routingData. Kick off a connect request.
  EXPECT_FALSE(pool->isBroadcasting(routingData));
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>* h) {
        handler1 = h;
        handler1->subscribe(&subscriber);
      });
  EXPECT_TRUE(handler1 == nullptr);
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  // Invoke getHandler() for the same routing data when a connect request
  // is outstanding
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>* h) {
        handler2 = h;
        handler2->subscribe(&subscriber);
      });
  EXPECT_TRUE(handler1 == nullptr);
  EXPECT_TRUE(handler2 == nullptr);
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  EXPECT_CALL(*pipelineFactory, setRoutingData(_, "url1")).Times(1);

  base->loopOnce(); // Do async connect

  // Verify that both promises are fulfilled
  EXPECT_TRUE(handler1 != nullptr);
  EXPECT_TRUE(handler2 != nullptr);
  EXPECT_TRUE(handler1 == handler2);
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  // Invoke getHandler() again to test if the same handler is returned
  // from the existing connection
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>* h) {
        EXPECT_TRUE(h == handler1);
      })
      .wait();
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  // Cleanup
  handler1->readEOF(handler1->getContext());
}

TEST_F(BroadcastPoolTest, ConnectError) {
  // Test when an exception occurs during connect request
  std::string routingData = "url1";
  BroadcastHandler<int, std::string>* handler1 = nullptr;
  BroadcastHandler<int, std::string>* handler2 = nullptr;
  bool handler1Error = false;
  bool handler2Error = false;
  auto base = EventBaseManager::get()->getEventBase();

  InSequence dummy;

  // Stop the server to inject connect failure
  stopServer();

  // No broadcast available for routingData. Kick off a connect request.
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>* h) { handler1 = h; })
      .thenError(folly::tag_t<std::exception>{}, [&](const std::exception&) {
        handler1Error = true;
        EXPECT_FALSE(pool->isBroadcasting(routingData));
      });
  EXPECT_TRUE(handler1 == nullptr);
  EXPECT_FALSE(handler1Error);
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  // Invoke getHandler() again while the connect request is in flight
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>* h) { handler2 = h; })
      .thenError(folly::tag_t<std::exception>{}, [&](const std::exception&) {
        handler2Error = true;
        EXPECT_FALSE(pool->isBroadcasting(routingData));
      });
  EXPECT_TRUE(handler2 == nullptr);
  EXPECT_FALSE(handler2Error);
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  base->loopOnce(); // Do async connect

  // Verify that the exception is set on both promises
  EXPECT_TRUE(handler1 == nullptr);
  EXPECT_TRUE(handler2 == nullptr);
  EXPECT_TRUE(handler1Error);
  EXPECT_TRUE(handler2Error);

  // The broadcast should have been deleted now
  EXPECT_FALSE(pool->isBroadcasting(routingData));

  // Start the server now. Connect requests should succeed.
  startServer();
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>* h) {
        handler1 = h;
        handler1->subscribe(&subscriber);
      });
  EXPECT_TRUE(handler1 == nullptr);
  EXPECT_CALL(*pipelineFactory, setRoutingData(_, "url1")).Times(1);
  base->loopOnce(); // Do async connect
  EXPECT_TRUE(handler1 != nullptr);
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  // Cleanup
  handler1->readEOF(handler1->getContext());
}

TEST_F(BroadcastPoolTest, ConnectErrorServerPool) {
  // Test when an error occurs in ServerPool when trying to kick off
  // a connect request
  std::string routingData = "url1";
  BroadcastHandler<int, std::string>* handler1 = nullptr;
  bool handler1Error = false;

  InSequence dummy;

  // Inject a ServerPool error
  serverPool->failConnect();
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>* h) { handler1 = h; })
      .thenError(folly::tag_t<std::exception>{}, [&](const std::exception&) {
        handler1Error = true;
        EXPECT_FALSE(pool->isBroadcasting(routingData));
      });
  EXPECT_TRUE(handler1 == nullptr);
  EXPECT_TRUE(handler1Error);
  EXPECT_FALSE(pool->isBroadcasting(routingData));
}

TEST_F(BroadcastPoolTest, RoutingDataException) {
  // Test when an exception occurs while setting routing data on
  // the pipeline after the socket connect succeeds.
  std::string routingData = "url";
  BroadcastHandler<int, std::string>* handler = nullptr;
  bool handlerError = false;
  auto base = EventBaseManager::get()->getEventBase();

  InSequence dummy;

  EXPECT_FALSE(pool->isBroadcasting(routingData));
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>* h) { handler = h; })
      .thenError(folly::tag_t<std::exception>{}, [&](const std::exception&) {
        handlerError = true;
        EXPECT_FALSE(pool->isBroadcasting(routingData));
      });
  EXPECT_TRUE(handler == nullptr);
  EXPECT_CALL(*pipelineFactory, setRoutingData(_, "url"))
      .WillOnce(Throw(std::exception()));
  base->loopOnce(); // Do async connect
  EXPECT_TRUE(handler == nullptr);
  EXPECT_TRUE(handlerError);
  EXPECT_FALSE(pool->isBroadcasting(routingData));
}

TEST_F(BroadcastPoolTest, RoutingDataPipelineDeletion) {
  // Test when the broadcast pipeline gets deleted inline while setting
  // routing data after the socket connection succeeds.
  std::string routingData = "url";
  BroadcastHandler<int, std::string>* handler = nullptr;
  bool handlerError = false;
  auto base = EventBaseManager::get()->getEventBase();

  InSequence dummy;

  EXPECT_FALSE(pool->isBroadcasting(routingData));
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>* h) { handler = h; })
      .thenError(folly::tag_t<std::exception>{}, [&](const std::exception&) {
        handlerError = true;
        EXPECT_FALSE(pool->isBroadcasting(routingData));
      });
  EXPECT_TRUE(handler == nullptr);
  EXPECT_CALL(*pipelineFactory, setRoutingData(_, "url"))
      .WillOnce(Invoke([&](DefaultPipeline* pipeline, const std::string&) {
        pipeline->readException(std::runtime_error("upstream error"));
      }));
  base->loopOnce(); // Do async connect
  EXPECT_TRUE(handler == nullptr);
  EXPECT_TRUE(handlerError);
  EXPECT_FALSE(pool->isBroadcasting(routingData));
}

TEST_F(BroadcastPoolTest, HandlerEOFPoolDeletion) {
  // Test against use-after-free on BroadcastManager when the pool
  // is deleted before the handler
  std::string routingData = "url1";
  BroadcastHandler<int, std::string>* handler = nullptr;
  DefaultPipeline* pipeline = nullptr;
  auto base = EventBaseManager::get()->getEventBase();

  InSequence dummy;

  // Dispatch a connect request and create a handler
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>* h) {
        handler = h;
        handler->subscribe(&subscriber);
        pipeline = dynamic_cast<DefaultPipeline*>(
            handler->getContext()->getPipeline());
      });
  EXPECT_CALL(*pipelineFactory, setRoutingData(_, "url1")).Times(1);
  base->loopOnce(); // Do async connect
  EXPECT_TRUE(pool->isBroadcasting(routingData));
  EXPECT_TRUE(handler != nullptr);
  EXPECT_TRUE(pipeline != nullptr);

  EXPECT_CALL(subscriber, onCompleted()).Times(1);

  // This will also delete the pipeline and the handler
  pipeline->readEOF();
  EXPECT_FALSE(pool->isBroadcasting(routingData));
}

TEST_F(BroadcastPoolTest, SubscriberDeletionBeforeConnect) {
  // Test when the caller goes away before connect request returns
  // resulting in a new BroadcastHandler without any subscribers
  std::string routingData = "url1";
  BroadcastHandler<int, std::string>* handler = nullptr;
  bool handler1Connected = false;
  bool handler2Connected = false;
  auto base = EventBaseManager::get()->getEventBase();

  InSequence dummy;

  // No broadcast available for routingData. Kick off a connect request.
  EXPECT_FALSE(pool->isBroadcasting(routingData));
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>*) {
        handler1Connected = true;
        // Do not subscribe to the handler. This will simulate
        // the caller going away before we get here.
      });
  EXPECT_FALSE(handler1Connected);
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  // Invoke getHandler() for the same routing data when a connect request
  // is outstanding
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>*) {
        handler2Connected = true;
        // Do not subscribe to the handler.
      });
  EXPECT_FALSE(handler2Connected);
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  EXPECT_CALL(*pipelineFactory, setRoutingData(_, "url1")).Times(1);

  base->loopOnce(); // Do async connect

  // Verify that both promises are fulfilled, but the broadcast is
  // deleted from the pool because no subscriber was added.
  EXPECT_TRUE(handler1Connected);
  EXPECT_TRUE(handler2Connected);
  EXPECT_FALSE(pool->isBroadcasting(routingData));

  // Test test same scenario but with one subscriber going away
  // sooner, but another subscriber being added to the handler.
  handler1Connected = false;
  handler2Connected = false;
  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>*) {
        handler1Connected = true;
        // Do not subscribe to the handler. This will simulate
        // the caller going away before we get here.
      });
  EXPECT_FALSE(handler1Connected);
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  pool->getHandler(routingData)
      .thenValue([&](BroadcastHandler<int, std::string>* h) {
        handler2Connected = true;
        // Subscriber to the handler. The handler should stick around now.
        handler = h;
        handler->subscribe(&subscriber);
      });
  EXPECT_FALSE(handler2Connected);
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  EXPECT_CALL(*pipelineFactory, setRoutingData(_, "url1")).Times(1);

  base->loopOnce(); // Do async connect

  // Verify that both promises are fulfilled, but the broadcast is
  // deleted from the pool because no subscriber was added.
  EXPECT_TRUE(handler1Connected);
  EXPECT_TRUE(handler2Connected);
  EXPECT_TRUE(pool->isBroadcasting(routingData));

  // Cleanup
  handler->readEOF(handler->getContext());
}

TEST_F(BroadcastPoolTest, ThreadLocalPool) {
  // Test that thread-local broadcast pool works correctly
  MockObservingPipelineFactory factory1(serverPool, pipelineFactory);
  MockObservingPipelineFactory factory2(serverPool, pipelineFactory);
  BroadcastHandler<int, std::string>* broadcastHandler = nullptr;
  uint64_t broadcastHandlerId = 0;
  const std::string kUrl = "url";

  InSequence dummy;

  // There should be no broadcast available for this routing data
  EXPECT_FALSE(factory1.broadcastPool()->isBroadcasting(kUrl));
  EXPECT_FALSE(factory2.broadcastPool()->isBroadcasting(kUrl));

  // Test creating a new broadcast
  EXPECT_CALL(*pipelineFactory, setRoutingData(_, kUrl))
      .WillOnce(Invoke([&](DefaultPipeline* pipeline, const std::string&) {
        broadcastHandler = pipelineFactory->getBroadcastHandler(pipeline);
        broadcastHandlerId = broadcastHandler->getArbitraryIdentifier();
      }));
  auto pipeline1 = factory1.newPipeline(nullptr, kUrl, nullptr, nullptr);
  pipeline1->transportActive();
  EventBaseManager::get()->getEventBase()->loopOnce();
  EXPECT_TRUE(factory1.broadcastPool()->isBroadcasting(kUrl));
  EXPECT_FALSE(factory2.broadcastPool()->isBroadcasting(kUrl));

  // Test broadcast with the same routing data in the same thread. No
  // new broadcast handler should be created.
  EXPECT_CALL(*pipelineFactory, setRoutingData(_, _)).Times(0);
  auto pipeline2 = factory1.newPipeline(nullptr, kUrl, nullptr, nullptr);
  pipeline2->transportActive();
  EXPECT_TRUE(factory1.broadcastPool()->isBroadcasting(kUrl));
  EXPECT_FALSE(factory2.broadcastPool()->isBroadcasting(kUrl));

  // Test creating a broadcast with the same routing data but in a
  // different thread. Should return a different broadcast handler.
  std::thread([&] {
    // There should be no broadcast available for this routing data since we
    // are on a different thread.
    EXPECT_FALSE(factory1.broadcastPool()->isBroadcasting(kUrl));
    EXPECT_FALSE(factory2.broadcastPool()->isBroadcasting(kUrl));

    EXPECT_CALL(*pipelineFactory, setRoutingData(_, kUrl))
        .WillOnce(Invoke([&](DefaultPipeline* pipeline, const std::string&) {
          EXPECT_NE(
              pipelineFactory->getBroadcastHandler(pipeline)
                  ->getArbitraryIdentifier(),
              broadcastHandlerId);
        }));
    auto pipeline3 = factory1.newPipeline(nullptr, kUrl, nullptr, nullptr);
    pipeline3->transportActive();
    EventBaseManager::get()->getEventBase()->loopOnce();
    EXPECT_TRUE(factory1.broadcastPool()->isBroadcasting(kUrl));
    EXPECT_FALSE(factory2.broadcastPool()->isBroadcasting(kUrl));

    // Cleanup
    pipeline3->readEOF();
  }).join();

  // Test creating a broadcast with the same routing data but using a
  // different ObservingPipelineFactory. Should return a different broadcast
  // handler since a different thread-local BroadcastPool is used.
  EXPECT_CALL(*pipelineFactory, setRoutingData(_, kUrl))
      .WillOnce(Invoke([&](DefaultPipeline* pipeline, const std::string&) {
        EXPECT_NE(
            pipelineFactory->getBroadcastHandler(pipeline)
                ->getArbitraryIdentifier(),
            broadcastHandlerId);
      }));
  auto pipeline4 = factory2.newPipeline(nullptr, kUrl, nullptr, nullptr);
  pipeline4->transportActive();
  EventBaseManager::get()->getEventBase()->loopOnce();
  EXPECT_TRUE(factory2.broadcastPool()->isBroadcasting(kUrl));

  // Cleanup
  pipeline1->readEOF();
  pipeline2->readEOF();
  pipeline4->readEOF();
}
