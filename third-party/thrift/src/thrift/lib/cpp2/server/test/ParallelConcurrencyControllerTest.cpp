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

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/portability/GTest.h>
#include <folly/synchronization/Baton.h>
#include <folly/synchronization/Latch.h>

#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestServiceAsyncClient.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::transport;
using namespace apache::thrift::concurrency;

using Func = std::function<void(
    ServerRequest&&, const AsyncProcessorFactory::MethodMetadata&)>;

class MockAsyncProcessor : public AsyncProcessor {
 public:
  void processSerializedCompressedRequestWithMetadata(
      apache::thrift::ResponseChannelRequest::UniquePtr,
      apache::thrift::SerializedCompressedRequest&&,
      const apache::thrift::AsyncProcessorFactory::MethodMetadata&,
      apache::thrift::protocol::PROTOCOL_TYPES,
      apache::thrift::Cpp2RequestContext*,
      folly::EventBase*,
      apache::thrift::concurrency::ThreadManager*) override {}

  void executeRequest(
      ServerRequest&& request,
      const AsyncProcessorFactory::MethodMetadata& methodMetadata) override {
    if (executeRequestFunc_) {
      executeRequestFunc_(std::move(request), methodMetadata);
    } else {
      LOG(FATAL) << "Unimplemented executeRequest called";
    }
  }

  void setFunc(Func func) { executeRequestFunc_ = std::move(func); }

 private:
  Func executeRequestFunc_;
};

class FIFORequestPile : public RequestPileInterface {
 public:
  std::optional<ServerRequestRejection> enqueue(
      ServerRequest&& request) override {
    queue_.enqueue(std::move(request));
    return std::nullopt;
  }

  std::pair<std::optional<ServerRequest>, std::optional<intptr_t>> dequeue()
      override {
    if (auto res = queue_.try_dequeue()) {
      return std::make_pair(std::move(*res), std::nullopt);
    } else {
      return {std::nullopt, std::nullopt};
    }
  }

  void onRequestFinished(ServerRequestData&) override {}

  uint64_t requestCount() const override { return queue_.size(); }

 private:
  using Queue = folly::UMPMCQueue<
      ServerRequest,
      /* MayBlock */ false,
      /* LgSegmentSize  */ 5>;

  Queue queue_;
};

class ResourcePoolMock {
 public:
  ResourcePoolMock(
      FIFORequestPile* pile, ParallelConcurrencyController* controller)
      : pile_(pile), controller_(controller) {}

  void enqueue(ServerRequest&& request) {
    pile_->enqueue(std::move(request));
    controller_->onEnqueued();
  }

 private:
  FIFORequestPile* pile_;
  ParallelConcurrencyController* controller_;
};

class MockResponseChannelRequest : public ResponseChannelRequest {
 public:
  bool isActive() const override { return true; }

  bool isOneway() const override { return false; }

  bool includeEnvelope() const override { return false; }

  void sendReply(
      ResponsePayload&&,
      MessageChannel::SendCallback*,
      folly::Optional<uint32_t>) override {}

  void sendErrorWrapped(folly::exception_wrapper, std::string) override {}

  bool tryStartProcessing() override { return true; }
};

namespace {

ServerRequest getRequest(AsyncProcessor* ap, folly::EventBase*) {
  static Cpp2RequestContext ctx = Cpp2RequestContext(nullptr);
  ServerRequest req(
      ResponseChannelRequest::UniquePtr(new MockResponseChannelRequest),
      SerializedCompressedRequest(std::unique_ptr<folly::IOBuf>{}),
      &ctx, /* ctx  */
      static_cast<protocol::PROTOCOL_TYPES>(0),
      nullptr, /* requestContext  */
      ap,
      nullptr /* methodMetadata  */);
  return req;
}

Func blockingTaskGen(folly::Baton<>& baton, ParallelConcurrencyController&) {
  Func waitingTask = [&](ServerRequest&& /* request */,
                         const AsyncProcessorFactory::MethodMetadata&) {
    baton.wait();
  };

  return waitingTask;
}

Func endingTaskGen(folly::Baton<>& baton, ParallelConcurrencyController&) {
  Func waitingTask = [&](ServerRequest&& /* request */,
                         const AsyncProcessorFactory::MethodMetadata&) {
    baton.post();
  };

  return waitingTask;
}

std::unique_ptr<AsyncProcessor> makeAP(Func func) {
  auto mockAP = std::make_unique<MockAsyncProcessor>();
  mockAP->setFunc(func);
  std::unique_ptr<AsyncProcessor> endingAP(std::move(mockAP));
  return endingAP;
}

} // namespace

// This test just tests the normal case where we have 2 tasks that sit in
// the Executor, the count should return 2.
// When the tasks all finish, the count should return 0
TEST(ParallelConcurrencyControllerTest, NormalCases) {
  folly::EventBase eb;

  folly::CPUThreadPoolExecutor ex(1);
  FIFORequestPile pile;
  ParallelConcurrencyController controller(pile, ex);

  folly::Baton baton1;
  folly::Baton baton2;

  auto blockingAP = makeAP(blockingTaskGen(baton1, controller));

  auto endingAP = makeAP(endingTaskGen(baton2, controller));

  ResourcePoolMock pool(&pile, &controller);

  pool.enqueue(getRequest(blockingAP.get(), &eb));
  pool.enqueue(getRequest(endingAP.get(), &eb));

  EXPECT_EQ(controller.requestCount(), 2);
  baton1.post();

  baton2.wait();
  EXPECT_EQ(controller.requestCount(), 0);

  ex.join();
}

// This tests when the concurrency limit is set to 2
// In this case only 2 tasks can run concurrently
TEST(ParallelConcurrencyControllerTest, LimitedTasks) {
  folly::EventBase eb;

  folly::CPUThreadPoolExecutor ex(1);
  FIFORequestPile pile;
  ParallelConcurrencyController controller(pile, ex);

  controller.setExecutionLimitRequests(2);

  folly::Baton baton1;
  folly::Baton baton2;
  folly::Baton baton3;

  auto staringBlockingAP = makeAP(blockingTaskGen(baton1, controller));

  auto blockingAP = makeAP(blockingTaskGen(baton2, controller));

  auto endingAP = makeAP(endingTaskGen(baton3, controller));

  ResourcePoolMock pool(&pile, &controller);

  pool.enqueue(getRequest(blockingAP.get(), &eb));
  pool.enqueue(getRequest(blockingAP.get(), &eb));
  pool.enqueue(getRequest(endingAP.get(), &eb));

  EXPECT_EQ(controller.requestCount(), 2);

  baton1.post();
  baton2.post();

  baton3.wait();
  EXPECT_EQ(controller.requestCount(), 0);

  ex.join();
}

Func edgeTaskGen(
    folly::Latch& latch,
    folly::Baton<>& baton,
    ParallelConcurrencyController&) {
  Func waitingTask = [&](ServerRequest&& /* request */,
                         const AsyncProcessorFactory::MethodMetadata&) {
    baton.wait();
    latch.count_down();
  };

  return waitingTask;
}

std::unique_ptr<AsyncProcessor> getEdgeTaskAP(
    folly::Latch& latch,
    folly::Baton<>& baton,
    ParallelConcurrencyController& controller) {
  auto mockAP = new MockAsyncProcessor;
  mockAP->setFunc(edgeTaskGen(latch, baton, controller));
  std::unique_ptr<AsyncProcessor> endingAP(mockAP);
  return endingAP;
}

TEST(ParallelConcurrencyControllerTest, DifferentOrdering1) {
  folly::EventBase eb;

  folly::CPUThreadPoolExecutor ex(2);
  FIFORequestPile pile;
  ParallelConcurrencyController controller(pile, ex);
  controller.setExecutionLimitRequests(2);

  ResourcePoolMock pool(&pile, &controller);

  folly::Latch latch(3);

  folly::Baton baton1;
  folly::Baton baton2;
  folly::Baton baton3;

  auto mockAP1 = getEdgeTaskAP(latch, baton1, controller);
  auto mockAP2 = getEdgeTaskAP(latch, baton2, controller);
  auto mockAP3 = getEdgeTaskAP(latch, baton3, controller);

  // one scenario is right after one task finishes
  // we push another task into the queue
  pool.enqueue(getRequest(mockAP1.get(), &eb));
  pool.enqueue(getRequest(mockAP2.get(), &eb));

  EXPECT_EQ(controller.requestCount(), 2);

  // one task will finish immediately and another task is pushed
  // This shouldn't be causing any idle thread
  baton1.post();
  pool.enqueue(getRequest(mockAP3.get(), &eb));

  baton2.post();
  baton3.post();

  latch.wait();
  EXPECT_EQ(controller.requestCount(), 0);
  EXPECT_EQ(pile.requestCount(), 0);

  ex.join();
}

TEST(ParallelConcurrencyControllerTest, DifferentOrdering2) {
  folly::EventBase eb;

  folly::CPUThreadPoolExecutor ex(2);
  FIFORequestPile pile;
  ParallelConcurrencyController controller(pile, ex);
  controller.setExecutionLimitRequests(2);

  ResourcePoolMock pool(&pile, &controller);

  folly::Latch latch(3);

  folly::Baton baton1;
  folly::Baton baton2;
  folly::Baton baton3;

  auto mockAP1 = getEdgeTaskAP(latch, baton1, controller);
  auto mockAP2 = getEdgeTaskAP(latch, baton2, controller);
  auto mockAP3 = getEdgeTaskAP(latch, baton3, controller);

  // another scenario is right before one task finishes
  // we push another task into the queue
  pool.enqueue(getRequest(mockAP1.get(), &eb));
  pool.enqueue(getRequest(mockAP2.get(), &eb));

  EXPECT_EQ(controller.requestCount(), 2);

  // one task will finish immediately and another task is pushed
  pool.enqueue(getRequest(mockAP3.get(), &eb));
  baton1.post();
  baton2.post();
  baton3.post();

  latch.wait();
  EXPECT_EQ(controller.requestCount(), 0);
  EXPECT_EQ(pile.requestCount(), 0);

  ex.join();
}

TEST(ParallelConcurrencyControllerTest, InternalPrioritization) {
  THRIFT_FLAG_SET_MOCK(experimental_use_resource_pools, true);
  if (!apache::thrift::useResourcePoolsFlagsSet()) {
    GTEST_SKIP() << "Invalid resource pools mode";
  }

  std::atomic<int> counter{0};
  folly::Baton<> blockingBaton{};

  class BlockingCallTestService
      : public apache::thrift::ServiceHandler<TestService> {
   public:
    BlockingCallTestService(std::atomic<int>& counter) : counter_(counter) {}

    folly::SemiFuture<int32_t> semifuture_echoInt(int32_t) override {
      // this external task should be executed second
      // so counter now is 1
      EXPECT_EQ(counter_.load(), 1);
      return folly::makeSemiFuture(1);
    }

   private:
    std::atomic<int>& counter_;
  };

  auto handler = std::make_shared<BlockingCallTestService>(counter);

  RoundRobinRequestPile::Options options;
  auto requestPile = std::make_unique<apache::thrift::RoundRobinRequestPile>(
      std::move(options));

  // 1 thread, 2 priorities, so that we can serialize the calls
  auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(1, 2);

  auto concurrencyController =
      std::make_unique<apache::thrift::ParallelConcurrencyController>(
          *requestPile.get(), *executor.get());

  auto config = [&](apache::thrift::ThriftServer& server) mutable {
    server.resourcePoolSet().setResourcePool(
        apache::thrift::ResourcePoolHandle::defaultAsync(),
        std::move(requestPile),
        executor,
        std::move(concurrencyController));
  };

  ScopedServerInterfaceThread runner(handler, config);

  auto& thriftServer = dynamic_cast<ThriftServer&>(runner.getThriftServer());

  auto ka = thriftServer.getHandlerExecutorKeepAlive();

  ka->add([&]() { blockingBaton.wait(); });

  auto client = runner.newClient<TestServiceAsyncClient>();

  auto res = client->semifuture_echoInt(0);

  // wait for the request to reach the executor queue
  while (executor->getTaskQueueSize() != 1) {
    std::this_thread::yield();
  }

  // this is an internal task, which should
  // be prioritized over external requests
  // so counter should be 0 here
  ka->add([&]() {
    EXPECT_EQ(counter.load(), 0);
    ++counter;
  });

  blockingBaton.post();

  EXPECT_EQ(std::move(res).get(), 1);
}

TEST(ParallelConcurrencyControllerTest, FinishCallbackExecptionSafe) {
  THRIFT_FLAG_SET_MOCK(allow_resource_pools_for_wildcards, true);

  class DummyTestService : public apache::thrift::ServiceHandler<TestService> {
  };

  class BadAsyncProcessor : public AsyncProcessor {
   public:
    BadAsyncProcessor(folly::Baton<>& baton) : baton_(baton) {}

    void processSerializedCompressedRequestWithMetadata(
        apache::thrift::ResponseChannelRequest::UniquePtr,
        apache::thrift::SerializedCompressedRequest&&,
        const apache::thrift::AsyncProcessorFactory::MethodMetadata&,
        apache::thrift::protocol::PROTOCOL_TYPES,
        apache::thrift::Cpp2RequestContext*,
        folly::EventBase*,
        apache::thrift::concurrency::ThreadManager*) override {}

    void executeRequest(
        ServerRequest&& serverRequest,
        const AsyncProcessorFactory::MethodMetadata&) override {
      baton_.wait();

      using apache::thrift::detail::ServerRequestHelper;

      auto request = ServerRequestHelper::request(std::move(serverRequest));
      auto eb = ServerRequestHelper::eventBase(serverRequest);

      eb->runInEventBaseThread([request = std::move(request)]() {
        request->sendErrorWrapped(
            folly::make_exception_wrapper<TApplicationException>("bad news"),
            "1");
      });
    }

   private:
    folly::Baton<>& baton_;
  };

  class BadAsyncProcessorFactory : public AsyncProcessorFactory {
   public:
    BadAsyncProcessorFactory(folly::Baton<>& baton) : baton_(baton) {}

    std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
      return std::make_unique<BadAsyncProcessor>(baton_);
    }

    CreateMethodMetadataResult createMethodMetadata() override {
      WildcardMethodMetadataMap wildcardMap;
      wildcardMap.wildcardMetadata = std::make_shared<WildcardMethodMetadata>(
          MethodMetadata::ExecutorType::ANY);

      return wildcardMap;
    }

    std::vector<apache::thrift::ServiceHandlerBase*> getServiceHandlers()
        override {
      return {};
    }

   private:
    folly::Baton<>& baton_;
  };

  auto handler = std::make_shared<DummyTestService>();

  folly::Baton<> baton;

  auto fac = std::make_shared<BadAsyncProcessorFactory>(baton);

  auto config = [&](apache::thrift::ThriftServer& server) mutable {
    server.requireResourcePools();
    server.setInterface(fac);
  };

  ScopedServerInterfaceThread runner(handler, config);

  auto client = runner.newClient<TestServiceAsyncClient>();

  auto& thriftServer = dynamic_cast<ThriftServer&>(runner.getThriftServer());
  auto& rpSet = thriftServer.resourcePoolSet();
  auto& rp = rpSet.resourcePool(ResourcePoolHandle::defaultAsync());
  ConcurrencyControllerInterface& cc = *rp.concurrencyController();

  try {
    auto res = client->semifuture_echoInt(0);

    // make sure requestCount ever hit 1
    while (cc.requestCount() == 0) {
    }
    EXPECT_EQ(cc.requestCount(), 1);

    baton.post();

    std::move(res).get();
    ADD_FAILURE() << "Shouldn't have reached here!";
  } catch (...) {
    // in this case, an exception is thrown and HandlerCallback was not
    // constructed, we should still decrement the requestInExecution count
    // correctly
    EXPECT_EQ(cc.requestCount(), 0);
  }
}
