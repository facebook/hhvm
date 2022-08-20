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

#include <list>

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/MeteredExecutor.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Task.h>
#include <folly/portability/GTest.h>

using namespace folly;

class MeteredExecutorTest : public testing::Test {
 protected:
  void createAdapter(
      int numLevels,
      std::unique_ptr<Executor> exc =
          std::make_unique<CPUThreadPoolExecutor>(1)) {
    executors_.resize(numLevels + 1);
    executors_[0] = exc.get();
    for (int i = 0; i < numLevels; i++) {
      auto mlsa = std::make_unique<MeteredExecutor>(std::move(exc));
      exc = std::move(mlsa);
      executors_[i + 1] = exc.get();
    }
    owning_ = std::move(exc);
  }

  void add(Func f, uint8_t level = 0) { executors_[level]->add(std::move(f)); }

  void join() {
    folly::Baton baton;
    executors_.back()->add([&] { baton.post(); });
    baton.wait();
  }

  Executor::KeepAlive<> getKeepAlive(int level) { return executors_[level]; }

 protected:
  std::shared_ptr<void> owning_;
  std::vector<Executor*> executors_;
};

TEST_F(MeteredExecutorTest, SingleLevel) {
  createAdapter(1);

  folly::Baton baton;
  add([&] { baton.wait(); });

  int32_t v = 0;
  add([&] { EXPECT_EQ(0, v++); });
  // first lopri task executes in the scheduling order with hipri
  // tasks, but all subsequent lopri tasks yield.
  add([&] { EXPECT_EQ(1, v++); }, 1);
  add([&] { EXPECT_EQ(4, v++); }, 1);
  add([&] { EXPECT_EQ(5, v++); }, 1);
  add([&] { EXPECT_EQ(6, v++); }, 1);
  add([&] { EXPECT_EQ(2, v++); });
  add([&] { EXPECT_EQ(3, v++); });
  baton.post();

  join();
  EXPECT_EQ(v, 7);
}

TEST_F(MeteredExecutorTest, TwoLevels) {
  int32_t v = 0;

  createAdapter(2);

  folly::Baton baton;
  add([&] { baton.wait(); });
  add([&] { EXPECT_EQ(0, v++); });
  add([&] { EXPECT_EQ(1, v++); }, 1);
  add([&] { EXPECT_EQ(4, v++); }, 1);
  add([&] { EXPECT_EQ(5, v++); }, 1);
  add([&] { EXPECT_EQ(6, v++); }, 1);
  add([&] { EXPECT_EQ(7, v++); }, 1);
  add([&] { EXPECT_EQ(8, v++); }, 2);
  add([&] { EXPECT_EQ(13, v++); }, 2);
  add([&] { EXPECT_EQ(14, v++); }, 2);
  add([&] { EXPECT_EQ(15, v++); }, 2);
  add([&] { EXPECT_EQ(16, v++); }, 2);
  add([&] { EXPECT_EQ(9, v++); }, 1);
  add([&] { EXPECT_EQ(10, v++); }, 1);
  add([&] { EXPECT_EQ(11, v++); }, 1);
  add([&] { EXPECT_EQ(12, v++); }, 1);
  add([&] { EXPECT_EQ(2, v++); });
  add([&] { EXPECT_EQ(3, v++); });
  baton.post();

  join();
  EXPECT_EQ(v, 17);
}

TEST_F(MeteredExecutorTest, PreemptTest) {
  int32_t v = 0;

  createAdapter(1);

  folly::Baton baton1, baton2, baton3;
  add([&] { baton1.wait(); });

  add([&] { EXPECT_EQ(0, v++); });
  // first lopri task executes in the scheduling order with hipri
  // tasks, but all subsequent lopri tasks yield.
  add([&] { EXPECT_EQ(1, v++); }, 1);
  add([&] { EXPECT_EQ(4, v++); }, 1);
  add([&] { EXPECT_EQ(5, v++); }, 1);
  add([&] { EXPECT_EQ(2, v++); });
  add([&] { EXPECT_EQ(3, v++); });
  add(
      [&] {
        EXPECT_EQ(6, v++);
        baton2.post();
        baton3.wait();
      },
      1);

  // These P1 tasks should be run AFTER the 3 P0 tasks are added below. We
  // should expect the P0 tasks to preempt these.
  add([&] { EXPECT_EQ(7, v++); }, 1);
  add([&] { EXPECT_EQ(11, v++); }, 1);
  add([&] { EXPECT_EQ(12, v++); }, 1);
  add([&] { EXPECT_EQ(13, v++); }, 1);

  // Kick off the tasks, but we'll halt part-way through the P1 tasks to add
  // some P0 tasks into the mix.
  baton1.post();
  baton2.wait();

  // Throw in the high pris.
  add([&] { EXPECT_EQ(8, v++); });
  add([&] { EXPECT_EQ(9, v++); });
  add([&] { EXPECT_EQ(10, v++); });

  baton3.post();

  join();
  EXPECT_EQ(v, 14);
}

class MeteredExecutorTestP
    : public MeteredExecutorTest,
      public ::testing::WithParamInterface<std::tuple<int>> {
 protected:
  int maxReadAtOnce;
  void SetUp() override { std::tie(maxReadAtOnce) = GetParam(); }
};

TEST_P(MeteredExecutorTestP, TwoLevelsWithKeepAlives) {
  auto hipri_exec = std::make_unique<CPUThreadPoolExecutor>(1);
  auto hipri_ka = getKeepAliveToken(hipri_exec.get());
  auto mipri_exec = std::make_unique<MeteredExecutor>(hipri_ka);
  auto mipri_ka = getKeepAliveToken(mipri_exec.get());
  auto lopri_exec = std::make_unique<MeteredExecutor>(mipri_ka);
  executors_ = {hipri_exec.get(), mipri_exec.get(), lopri_exec.get()};

  int32_t v = 0;
  folly::Baton baton;
  add([&] { baton.wait(); });
  add([&] { EXPECT_EQ(0, v++); });
  add([&] { EXPECT_EQ(1, v++); }, 1);
  add([&] { EXPECT_EQ(4, v++); }, 1);
  add([&] { EXPECT_EQ(5, v++); }, 1);
  add([&] { EXPECT_EQ(6, v++); }, 1);
  add([&] { EXPECT_EQ(7, v++); }, 1);
  add([&] { EXPECT_EQ(8, v++); }, 2);
  add([&] { EXPECT_EQ(13, v++); }, 2);
  add([&] { EXPECT_EQ(14, v++); }, 2);
  add([&] { EXPECT_EQ(15, v++); }, 2);
  add([&] { EXPECT_EQ(16, v++); }, 2);
  add([&] { EXPECT_EQ(9, v++); }, 1);
  add([&] { EXPECT_EQ(10, v++); }, 1);
  add([&] { EXPECT_EQ(11, v++); }, 1);
  add([&] { EXPECT_EQ(12, v++); }, 1);
  add([&] { EXPECT_EQ(2, v++); });
  add([&] { EXPECT_EQ(3, v++); });
  baton.post();

  lopri_exec.reset();
  EXPECT_EQ(v, 17);
}

TEST_P(MeteredExecutorTestP, RequestContext) {
  createAdapter(3);

  folly::Baton baton;
  add([&] { baton.wait(); });

  auto addAndCheckRCTX = [this](int8_t pri = 0) {
    RequestContextScopeGuard g;
    auto f = [rctx = RequestContext::saveContext()]() mutable {
      EXPECT_EQ(rctx.get(), RequestContext::get());

      // Verify we do not have dangling reference to finished requests.
      static std::shared_ptr<folly::RequestContext> lastFinished;
      EXPECT_TRUE(!lastFinished || lastFinished.unique());
      lastFinished = std::move(rctx);
    };
    add(std::move(f), pri);
  };

  addAndCheckRCTX();
  addAndCheckRCTX(3);
  addAndCheckRCTX(3);
  addAndCheckRCTX(3);
  addAndCheckRCTX(1);
  addAndCheckRCTX(1);
  addAndCheckRCTX(1);
  addAndCheckRCTX(2);
  addAndCheckRCTX(2);
  addAndCheckRCTX(2);
  baton.post();
  join();
}

INSTANTIATE_TEST_SUITE_P(
    MeteredExecutorSuite, MeteredExecutorTestP, testing::Values(1, 3));

TEST_F(MeteredExecutorTest, ResetJoins) {
  createAdapter(2);

  folly::Baton baton;
  add([&] { baton.wait(); });

  int v = 0;
  add([&] { ++v; });
  add([&] { ++v; }, 2);
  add([&] { ++v; }, 2);
  add([&] { ++v; }, 1);
  add([&] { ++v; }, 1);

  baton.post();
  owning_.reset();
  EXPECT_EQ(v, 5);
}

TEST_F(MeteredExecutorTest, ConcurrentShutdown) {
  // ensure no data races on shutdown when executor has 2 threads
  createAdapter(2, std::make_unique<CPUThreadPoolExecutor>(2));
}

TEST_F(MeteredExecutorTest, CostOfMeteredExecutors) {
  // This test is to demonstrate how many tasks are scheduled
  // on the primarty executor when it is wrapped by MeteredExecutors
  class MyExecutor : public folly::Executor {
   public:
    int count{0};
    bool driveWhenAdded{false};
    std::list<folly::Func> queue;
    void add(folly::Func f) override {
      ++count;
      queue.push_back(std::move(f));
      if (driveWhenAdded) {
        drive();
      }
    }
    void drive() {
      while (!queue.empty()) {
        auto ff = std::move(queue.front());
        queue.pop_front();
        ff();
      }
    }
  };

  auto exc = std::make_unique<MyExecutor>();
  auto drive = [exc = exc.get()] { exc->drive(); };
  auto getCount = [exc = exc.get()] { return std::exchange(exc->count, 0); };
  auto driveOnAdd = [exc = exc.get()] { exc->driveWhenAdded = true; };
  createAdapter(3, std::move(exc));

  // When queues are empty, we will schedule as many tasks on the main
  // executor as there are executors in the chain.
  add([&] {}, 0);
  drive();
  EXPECT_EQ(1, getCount());

  add([&] {}, 1);
  drive();
  EXPECT_EQ(2, getCount());

  add([&] {}, 2);
  drive();
  EXPECT_EQ(3, getCount());

  add([&] {}, 3);
  drive();
  EXPECT_EQ(4, getCount());

  add([&] {}, 3);
  drive();
  EXPECT_EQ(4, getCount());

  // However, when queues are not empty, each additional task
  // scheduled on any MeteredExecutor, results in a only one more
  // task scheduled onto the main executor.
  add([&] {}, 3);
  add([&] {}, 3);
  add([&] {}, 3);
  drive();
  EXPECT_EQ(6, getCount());

  // To allow shutting down properly
  driveOnAdd();
}

TEST_F(MeteredExecutorTest, ExceptionHandling) {
  createAdapter(2);

  folly::Baton baton;
  add([&] {
    baton.wait();
    throw std::runtime_error("dummy");
  });

  bool thrown = false;
  add(
      [&] {
        thrown = true;
        throw std::runtime_error("dummy");
      },
      1);
  baton.post();
  join();
  EXPECT_EQ(true, thrown);
}

namespace {

coro::Task<bool> co_isOnCPUExc() {
  auto executor = co_await coro::co_current_executor;
  auto cpuexec = dynamic_cast<CPUThreadPoolExecutor*>(executor);
  co_return cpuexec != nullptr;
}

template <typename T>
coro::Task<bool> co_run(Executor::KeepAlive<> ka, coro::Task<T> f) {
  auto cpuexec = dynamic_cast<CPUThreadPoolExecutor*>(ka.get());
  EXPECT_TRUE(cpuexec != nullptr);
  co_return co_await std::move(f).scheduleOn(cpuexec);
}

} // namespace

TEST_F(MeteredExecutorTest, UnderlyingExecutor) {
  createAdapter(1);
  EXPECT_FALSE(coro::blockingWait(co_isOnCPUExc().scheduleOn(getKeepAlive(1))));
  EXPECT_TRUE(coro::blockingWait(
      co_run(getKeepAlive(0), co_isOnCPUExc()).scheduleOn(getKeepAlive(0))));
}
