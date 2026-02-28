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
#include <condition_variable>
#include <ctime>
#include <deque>
#include <memory>
#include <mutex>
#include <random>
#include <thread>

#include <fmt/chrono.h>
#include <gtest/gtest.h>
#include <folly/CPortability.h>
#include <folly/Synchronized.h>
#include <folly/executors/Codel.h>
#include <folly/lang/Keep.h>
#include <folly/portability/PThread.h>
#include <folly/portability/SysResource.h>
#include <folly/portability/SysTime.h>
#include <folly/synchronization/Baton.h>
#include <folly/synchronization/Latch.h>
#include <thrift/lib/cpp/concurrency/FunctionRunner.h>
#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>

using namespace apache::thrift::concurrency;

class ThreadManagerTest : public testing::Test {
 public:
  ~ThreadManagerTest() override { ThreadManager::setGlobalObserver(nullptr); }

 private:
  gflags::FlagSaver flagsaver_;
};

#if FOLLY_HAVE_WEAK_SYMBOLS
static folly::WorkerProvider* kWorkerProviderGlobal = nullptr;

namespace folly {
FOLLY_KEEP std::unique_ptr<folly::QueueObserverFactory>
make_queue_observer_factory(
    const std::string&, size_t, folly::WorkerProvider* workerProvider) {
  kWorkerProviderGlobal = workerProvider;
  return {};
}
} // namespace folly
#endif

// Loops until x==y for up to timeout ms.
// The end result is the same as of {EXPECT,ASSERT}_EQ(x,y)
// (depending on OP) if x!=y after the timeout passes
#define X_EQUAL_SPECIFIC_TIMEOUT(OP, timeout, x, y)         \
  do {                                                      \
    using std::chrono::steady_clock;                        \
    using std::chrono::milliseconds;                        \
    auto end = steady_clock::now() + milliseconds(timeout); \
    while ((x) != (y) && steady_clock::now() < end) {       \
    }                                                       \
    OP##_EQ(x, y);                                          \
  } while (0)

#define CHECK_EQUAL_SPECIFIC_TIMEOUT(timeout, x, y) \
  X_EQUAL_SPECIFIC_TIMEOUT(EXPECT, timeout, x, y)
#define REQUIRE_EQUAL_SPECIFIC_TIMEOUT(timeout, x, y) \
  X_EQUAL_SPECIFIC_TIMEOUT(ASSERT, timeout, x, y)

// A default timeout of 1 sec should be long enough for other threads to
// stabilize the values of x and y, and short enough to catch real errors
// when x is not going to be equal to y anytime soon
#define CHECK_EQUAL_TIMEOUT(x, y) CHECK_EQUAL_SPECIFIC_TIMEOUT(1000, x, y)
#define REQUIRE_EQUAL_TIMEOUT(x, y) REQUIRE_EQUAL_SPECIFIC_TIMEOUT(1000, x, y)

using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

std::chrono::milliseconds toMilliseconds(TimePoint::duration d) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(d);
}

class LoadTask : public Runnable {
 public:
  LoadTask(
      std::mutex* mutex,
      std::condition_variable* cond,
      size_t* count,
      std::chrono::milliseconds timeout)
      : mutex_(mutex), cond_(cond), count_(count), timeout_(timeout) {}

  void run() override {
    startTime_ = std::chrono::steady_clock::now();
    usleep(std::chrono::microseconds(timeout_).count());
    endTime_ = std::chrono::steady_clock::now();

    {
      std::unique_lock<std::mutex> l(*mutex_);

      (*count_)--;
      if (*count_ == 0) {
        cond_->notify_one();
      }
    }
  }

  std::mutex* mutex_;
  std::condition_variable* cond_;
  size_t* count_;
  std::chrono::milliseconds timeout_;
  TimePoint startTime_;
  TimePoint endTime_;
};

/**
 * Dispatch count tasks, each of which blocks for timeout milliseconds then
 * completes. Verify that all tasks completed and that thread manager cleans
 * up properly on delete.
 */
static void loadTest(
    size_t numTasks, std::chrono::milliseconds timeout, size_t numWorkers) {
  std::mutex mutex;
  std::condition_variable cond;
  size_t tasksLeft = numTasks;

  auto threadManager = ThreadManager::newSimpleThreadManager(numWorkers);
  auto threadFactory = std::make_shared<PosixThreadFactory>();
  threadManager->threadFactory(threadFactory);
  threadManager->start();

  std::set<std::shared_ptr<LoadTask>> tasks;
  for (size_t n = 0; n < numTasks; n++) {
    tasks.insert(
        std::make_shared<LoadTask>(&mutex, &cond, &tasksLeft, timeout));
  }

  TimePoint startTime = std::chrono::steady_clock::now();
  for (const auto& task : tasks) {
    threadManager->add(task);
  }

  TimePoint tasksStartedTime = std::chrono::steady_clock::now();

  {
    std::unique_lock<std::mutex> l(mutex);
    while (tasksLeft > 0) {
      cond.wait(l);
    }
  }
  TimePoint endTime = std::chrono::steady_clock::now();

  TimePoint firstTime = TimePoint::max();
  TimePoint lastTime = TimePoint::min();
  double averageTimeMs = 0;
  using Duration = TimePoint::duration;
  Duration minTime = Duration::max();
  Duration maxTime = Duration::min();

  for (const auto& task : tasks) {
    EXPECT_GT(task->startTime_, TimePoint());
    EXPECT_GT(task->endTime_, TimePoint());

    Duration delta = task->endTime_ - task->startTime_;
    assert(delta > Duration());

    firstTime = std::min(firstTime, task->startTime_);
    lastTime = std::max(lastTime, task->endTime_);
    minTime = std::min(minTime, delta);
    maxTime = std::max(maxTime, delta);

    averageTimeMs += toMilliseconds(delta).count();
  }
  averageTimeMs /= numTasks;

  GTEST_LOG_(INFO) << fmt::format(
      "first start: {} last end: {} min: {} max {} average: {}ms",
      toMilliseconds(firstTime.time_since_epoch()),
      toMilliseconds(lastTime.time_since_epoch()),
      toMilliseconds(minTime),
      toMilliseconds(maxTime),
      averageTimeMs);

  double idealTimeMs =
      ((numTasks + (numWorkers - 1)) / numWorkers) * averageTimeMs;
  double actualTimeMs = toMilliseconds(endTime - startTime).count();
  Duration taskStartTime = tasksStartedTime - startTime;

  double overheadPct = (actualTimeMs - idealTimeMs) / idealTimeMs;
  if (overheadPct < 0) {
    overheadPct *= -1.0;
  }

  GTEST_LOG_(INFO) << fmt::format(
      "ideal time: {:.2f}ms actual time: {:.2f}ms "
      "task startup time: {} overhead: {:.2f}ms",
      idealTimeMs,
      actualTimeMs,
      toMilliseconds(taskStartTime),
      overheadPct * 100);

  // Fail if the test took 10% more time than the ideal time.
  EXPECT_LT(overheadPct, 0.10);
}

TEST_F(ThreadManagerTest, LoadTest) {
  size_t numTasks = 10000;
  std::chrono::milliseconds timeout(50);
  size_t numWorkers = 100;
  loadTest(numTasks, timeout, numWorkers);
}

class BlockTask : public Runnable {
 public:
  BlockTask(
      std::mutex* mutex,
      std::condition_variable* cond,
      std::mutex* bmutex,
      std::condition_variable* bcond,
      bool* blocked,
      size_t* count)
      : mutex_(mutex),
        cond_(cond),
        bmutex_(bmutex),
        bcond_(bcond),
        blocked_(blocked),
        count_(count),
        started_(false) {}

  void run() override {
    started_ = true;
    {
      std::unique_lock<std::mutex> l(*bmutex_);
      while (*blocked_) {
        bcond_->wait(l);
      }
    }

    {
      std::unique_lock<std::mutex> l(*mutex_);
      (*count_)--;
      if (*count_ == 0) {
        cond_->notify_one();
      }
    }
  }

  std::mutex* mutex_;
  std::condition_variable* cond_;
  std::mutex* bmutex_;
  std::condition_variable* bcond_;
  bool* blocked_;
  size_t* count_;
  bool started_;
};

static void expireTestCallback(
    std::shared_ptr<Runnable>,
    std::mutex* mutex,
    std::condition_variable* cond,
    size_t* count) {
  std::unique_lock<std::mutex> l(*mutex);
  --(*count);
  if (*count == 0) {
    cond->notify_one();
  }
}

static void expireTest(
    size_t numWorkers, std::chrono::milliseconds expirationTime) {
  size_t maxPendingTasks = numWorkers;
  size_t activeTasks = numWorkers + maxPendingTasks;
  std::mutex mutex;
  std::condition_variable cond;

  auto threadManager = ThreadManager::newSimpleThreadManager(numWorkers);
  auto threadFactory = std::make_shared<PosixThreadFactory>();
  threadManager->threadFactory(threadFactory);
  threadManager->setExpireCallback(
      std::bind(
          expireTestCallback,
          std::placeholders::_1,
          &mutex,
          &cond,
          &activeTasks));
  threadManager->start();

  // Add numWorkers + maxPendingTasks to fill up the ThreadManager's task queue
  std::vector<std::shared_ptr<BlockTask>> tasks;
  tasks.reserve(activeTasks);

  std::mutex bmutex;
  std::condition_variable bcond;
  bool blocked = true;
  for (size_t n = 0; n < numWorkers + maxPendingTasks; ++n) {
    auto task = std::make_shared<BlockTask>(
        &mutex, &cond, &bmutex, &bcond, &blocked, &activeTasks);
    tasks.push_back(task);
    threadManager->add(task, 0, expirationTime.count());
  }

  // Sleep for more than the expiration time
  usleep(std::chrono::microseconds(expirationTime).count() * 1.10);

  // Unblock the tasks
  {
    std::unique_lock<std::mutex> l(bmutex);
    blocked = false;
    bcond.notify_all();
  }
  // Wait for all tasks to complete or expire
  {
    std::unique_lock<std::mutex> l(mutex);
    while (activeTasks != 0) {
      cond.wait(l);
    }
  }

  // The first numWorkers tasks should have completed,
  // the remaining ones should have expired without running
  for (size_t index = 0; index < tasks.size(); ++index) {
    if (index < numWorkers) {
      EXPECT_TRUE(tasks[index]->started_);
    } else {
      EXPECT_TRUE(!tasks[index]->started_);
    }
  }
}

// DO_BEFORE(aristidis,20250715): Test is flaky. Find owner or remove.
TEST_F(ThreadManagerTest, DISABLED_ExpireTest) {
  size_t numWorkers = 100;
  std::chrono::milliseconds expireTime(50);
  expireTest(numWorkers, expireTime);
}

class AddRemoveTask : public Runnable,
                      public std::enable_shared_from_this<AddRemoveTask> {
 public:
  AddRemoveTask(
      uint32_t timeoutUs,
      const std::shared_ptr<ThreadManager>& manager,
      std::mutex* mutex,
      std::condition_variable* cond,
      int64_t* count,
      int64_t* objectCount)
      : timeoutUs_(timeoutUs),
        manager_(manager),
        mutex_(mutex),
        cond_(cond),
        count_(count),
        objectCount_(objectCount) {
    std::unique_lock<std::mutex> l(*mutex_);
    ++*objectCount_;
  }

  ~AddRemoveTask() override {
    std::unique_lock<std::mutex> l(*mutex_);
    --*objectCount_;
  }

  void run() override {
    usleep(timeoutUs_);

    {
      std::unique_lock<std::mutex> l(*mutex_);

      if (*count_ <= 0) {
        // The task count already dropped to 0.
        // We add more tasks than count_, so some of them may still be running
        // when count_ drops to 0.
        return;
      }

      --*count_;
      if (*count_ == 0) {
        cond_->notify_all();
        return;
      }
    }

    // Add ourself to the task queue again
    manager_->add(shared_from_this());
  }

 private:
  int32_t timeoutUs_;
  std::shared_ptr<ThreadManager> manager_;
  std::mutex* mutex_;
  std::condition_variable* cond_;
  int64_t* count_;
  int64_t* objectCount_;
};

class WorkerCountChanger : public Runnable {
 public:
  WorkerCountChanger(
      const std::shared_ptr<ThreadManager>& manager,
      std::mutex* mutex,
      int64_t* count,
      int64_t* addAndRemoveCount)
      : manager_(manager),
        mutex_(mutex),
        count_(count),
        addAndRemoveCount_(addAndRemoveCount) {}

  void run() override {
    // Continue adding and removing threads until the tasks are all done
    while (true) {
      {
        std::unique_lock<std::mutex> l(*mutex_);
        if (*count_ == 0) {
          return;
        }
        ++*addAndRemoveCount_;
      }
      addAndRemove();
    }
  }

  void addAndRemove() {
    // Add a random number of workers
    std::uniform_int_distribution<> workerDist(1, 10);
    uint32_t workersToAdd = workerDist(rng_);
    manager_->addWorker(workersToAdd);

    std::uniform_int_distribution<> taskDist(1, 50);
    uint32_t tasksToAdd = taskDist(rng_);
    (void)tasksToAdd;

    // Sleep for a random amount of time
    std::uniform_int_distribution<> sleepDist(1000, 5000);
    uint32_t sleepUs = sleepDist(rng_);
    usleep(sleepUs);

    // Remove the same number of workers we added
    manager_->removeWorker(workersToAdd);
  }

 private:
  std::mt19937 rng_;
  std::shared_ptr<ThreadManager> manager_;
  std::mutex* mutex_;
  int64_t* count_;
  int64_t* addAndRemoveCount_;
};

// Run lots of tasks, while several threads are all changing
// the number of worker threads.
TEST_F(ThreadManagerTest, AddRemoveWorker) {
  // Number of tasks to run
  int64_t numTasks = 100000;
  // Minimum number of workers to keep at any point in time
  size_t minNumWorkers = 10;
  // Number of threads that will be adding and removing workers
  int64_t numAddRemoveWorkers = 30;
  // Number of tasks to run in parallel
  int64_t numParallelTasks = 200;

  auto threadManager = ThreadManager::newSimpleThreadManager(minNumWorkers);
  auto threadFactory = std::make_shared<PosixThreadFactory>();
  threadManager->threadFactory(threadFactory);
  threadManager->start();

  std::mutex mutex;
  std::condition_variable cond;
  int64_t currentTaskObjects = 0;
  int64_t count = numTasks;
  int64_t addRemoveCount = 0;

  std::mt19937 rng;
  std::uniform_int_distribution<> taskTimeoutDist(1, 3000);
  for (int64_t n = 0; n < numParallelTasks; ++n) {
    int64_t taskTimeoutUs = taskTimeoutDist(rng);
    auto task = std::make_shared<AddRemoveTask>(
        taskTimeoutUs,
        threadManager,
        &mutex,
        &cond,
        &count,
        &currentTaskObjects);
    threadManager->add(task);
  }

  auto addRemoveFactory = std::make_shared<PosixThreadFactory>();
  addRemoveFactory->setDetached(false);
  std::deque<std::shared_ptr<Thread>> addRemoveThreads;
  for (int64_t n = 0; n < numAddRemoveWorkers; ++n) {
    auto worker = std::make_shared<WorkerCountChanger>(
        threadManager, &mutex, &count, &addRemoveCount);
    auto thread = addRemoveFactory->newThread(worker);
    addRemoveThreads.push_back(thread);
    thread->start();
  }

  while (!addRemoveThreads.empty()) {
    addRemoveThreads.front()->join();
    addRemoveThreads.pop_front();
  }

  LOG(INFO) << "add remove count: " << addRemoveCount;
  EXPECT_GT(addRemoveCount, 0);

  // Stop the ThreadManager, and ensure that all Task objects have been
  // destroyed.
  threadManager->stop();
  EXPECT_EQ(0, currentTaskObjects);
}

TEST_F(ThreadManagerTest, NeverStartedTest) {
  // Test destroying a ThreadManager that was never started.
  // This ensures that calling stop() on an unstarted ThreadManager works
  // properly.
  {
    auto threadManager = ThreadManager::newSimpleThreadManager(10); //
  }

  // Destroy a ThreadManager that has a ThreadFactory but was never started.
  {
    auto threadManager = ThreadManager::newSimpleThreadManager(10);
    auto threadFactory = std::make_shared<PosixThreadFactory>();
    threadManager->threadFactory(threadFactory);
  }
}

TEST_F(ThreadManagerTest, OnlyStartedTest) {
  // Destroy a ThreadManager that has a ThreadFactory and was started.
  for (int i = 0; i < 1000; ++i) {
    auto threadManager = ThreadManager::newSimpleThreadManager(10);
    auto threadFactory = std::make_shared<PosixThreadFactory>();
    threadManager->threadFactory(threadFactory);
    threadManager->start();
  }
}

TEST_F(ThreadManagerTest, RequestContext) {
  class TestData : public folly::RequestData {
   public:
    explicit TestData(int data) : data(data) {}

    bool hasCallback() override { return false; }

    int data;
  };

  // Create new request context for this scope.
  folly::RequestContextScopeGuard rctx;
  EXPECT_EQ(nullptr, folly::RequestContext::get()->getContextData("test"));
  folly::RequestContext::get()->setContextData(
      "test", std::make_unique<TestData>(42));
  auto data = folly::RequestContext::get()->getContextData("test");
  EXPECT_EQ(42, dynamic_cast<TestData*>(data)->data);

  struct VerifyRequestContext {
    ~VerifyRequestContext() {
      auto data2 = folly::RequestContext::get()->getContextData("test");
      EXPECT_TRUE(data2 != nullptr);
      if (data2 != nullptr) {
        EXPECT_EQ(42, dynamic_cast<TestData*>(data2)->data);
      }
    }
  };

  {
    auto threadManager = ThreadManager::newSimpleThreadManager(10);
    auto threadFactory = std::make_shared<PosixThreadFactory>();
    threadManager->threadFactory(threadFactory);
    threadManager->start();
    threadManager->add([] { VerifyRequestContext(); });
    threadManager->add([x = VerifyRequestContext()] {});
    threadManager->join();
  }
}

TEST_F(ThreadManagerTest, Exceptions) {
  class ThrowTask : public Runnable {
   public:
    void run() override {
      throw std::runtime_error("This should not crash the program");
    }
  };
  {
    auto threadManager = ThreadManager::newSimpleThreadManager(10);
    auto threadFactory = std::make_shared<PosixThreadFactory>();
    threadManager->threadFactory(threadFactory);
    threadManager->start();
    threadManager->add(std::make_shared<ThrowTask>());
    threadManager->join();
  }
}

class TestObserver : public ThreadManager::Observer {
 public:
  TestObserver(int64_t timeout, const std::string& expectedName)
      : timesCalled(0), timeout(timeout), expectedName(expectedName) {}

  void preRun(folly::RequestContext*) override {}
  void postRun(
      folly::RequestContext*, const ThreadManager::RunStats& stats) override {
    EXPECT_EQ(expectedName, stats.threadPoolName);

    // Note: Technically could fail if system clock changes.
    EXPECT_GT((stats.workBegin - stats.queueBegin).count(), 0);
    EXPECT_GT((stats.workEnd - stats.workBegin).count(), 0);
    EXPECT_GT((stats.workEnd - stats.workBegin).count(), timeout - 1);
    ++timesCalled;
  }

  uint64_t timesCalled;
  int64_t timeout;
  std::string expectedName;
};

class FailThread : public PthreadThread {
 public:
  FailThread(
      int policy,
      int priority,
      std::optional<int> stackSize,
      bool detached,
      std::shared_ptr<Runnable> runnable)
      : PthreadThread(policy, priority, stackSize, detached, runnable) {}

  void start() override { throw 2; }
};

class FailThreadFactory : public PosixThreadFactory {
 public:
  class FakeImpl : public Impl {
   public:
    FakeImpl(
        POLICY policy,
        PosixThreadFactory::THREAD_PRIORITY priority,
        std::optional<int> stackSize,
        DetachState detached)
        : Impl(policy, priority, stackSize, detached) {}

    std::shared_ptr<Thread> newThread(
        const std::shared_ptr<Runnable>& runnable,
        DetachState detachState) const override {
      auto result = std::make_shared<FailThread>(
          toPthreadPolicy(policy_),
          toPthreadPriority(policy_, priority_),
          stackSize_,
          detachState == DETACHED,
          runnable);
      result->weakRef(result);
      runnable->thread(result);
      return result;
    }
  };

  explicit FailThreadFactory(
      POLICY /*policy*/ = kDefaultPolicy,
      THREAD_PRIORITY /*priority*/ = kDefaultPriority,
      std::optional<int> /*stackSize*/ = std::nullopt,
      bool detached = true) {
    impl_ = std::make_shared<FailThreadFactory::FakeImpl>(
        kDefaultPolicy,
        kDefaultPriority,
        std::nullopt,
        detached ? DETACHED : ATTACHED);
  }
};

class DummyFailureClass {
 public:
  DummyFailureClass() {
    threadManager_ = ThreadManager::newSimpleThreadManager(20);
    threadManager_->setNamePrefix("foo");
    auto threadFactory = std::make_shared<FailThreadFactory>();
    threadManager_->threadFactory(threadFactory);
    threadManager_->start();
  }

 private:
  std::shared_ptr<ThreadManager> threadManager_;
};

TEST_F(ThreadManagerTest, ThreadStartFailureTest) {
  for (int i = 0; i < 10; i++) {
    EXPECT_THROW(DummyFailureClass(), int);
  }
}

TEST_F(ThreadManagerTest, ObserverTest) {
  auto observer = std::make_shared<TestObserver>(1000, "foo");
  ThreadManager::setGlobalObserver(observer);

  std::mutex mutex;
  std::condition_variable cond;
  size_t tasks = 1;

  auto threadManager = ThreadManager::newSimpleThreadManager(10);
  threadManager->setNamePrefix("foo");
  threadManager->threadFactory(std::make_shared<PosixThreadFactory>());
  threadManager->start();

  auto task = std::make_shared<LoadTask>(
      &mutex, &cond, &tasks, std::chrono::milliseconds(1000));
  threadManager->add(task);
  threadManager->join();
  EXPECT_EQ(1, observer->timesCalled);
}

TEST_F(ThreadManagerTest, ObserverAssignedAfterStart) {
  class MyTask : public Runnable {
   public:
    void run() override {}
  };
  class MyObserver : public ThreadManager::Observer {
   public:
    MyObserver(std::string name, std::shared_ptr<std::string> tgt)
        : name_(std::move(name)), tgt_(std::move(tgt)) {}
    void preRun(folly::RequestContext*) override {}
    void postRun(
        folly::RequestContext*, const ThreadManager::RunStats&) override {
      *tgt_ = name_;
    }

   private:
    std::string name_;
    std::shared_ptr<std::string> tgt_;
  };

  // start a tm
  auto tm = ThreadManager::newSimpleThreadManager(1);
  tm->setNamePrefix("foo");
  tm->threadFactory(std::make_shared<PosixThreadFactory>());
  tm->start();
  // set the observer w/ observable side-effect
  auto tgt = std::make_shared<std::string>();
  ThreadManager::setGlobalObserver(std::make_shared<MyObserver>("bar", tgt));
  // add a task - observable side-effect should trigger
  tm->add(std::make_shared<MyTask>());
  tm->join();
  // confirm the side-effect
  EXPECT_EQ("bar", *tgt);
}

TEST_F(ThreadManagerTest, PosixThreadFactoryPriority) {
  auto getNiceValue = [](PosixThreadFactory::THREAD_PRIORITY prio) -> int {
    PosixThreadFactory factory(PosixThreadFactory::OTHER, prio);
    factory.setDetached(false);
    int result = 0;
    auto t = factory.newThread(
        FunctionRunner::create([&] { result = getpriority(PRIO_PROCESS, 0); }));
    t->start();
    t->join();
    return result;
  };

  // NOTE: Test may not have permission to raise priority,
  // so use prio <= NORMAL.
  EXPECT_EQ(0, getNiceValue(PosixThreadFactory::NORMAL_PRI));
  EXPECT_LT(0, getNiceValue(PosixThreadFactory::LOW_PRI));
  auto th = std::thread([&] {
    for (int i = 0; i < 20; ++i) {
      if (setpriority(PRIO_PROCESS, 0, i) != 0) {
        PLOG(WARNING) << "failed setpriority(" << i << ")";
        continue;
      }
      EXPECT_EQ(i, getNiceValue(PosixThreadFactory::INHERITED_PRI));
    }
  });
  th.join();
}

TEST_F(ThreadManagerTest, PriorityThreadManagerWorkerCount) {
  auto threadManager = PriorityThreadManager::newPriorityThreadManager({{
      1 /*HIGH_IMPORTANT*/,
      2 /*HIGH*/,
      3 /*IMPORTANT*/,
      4 /*NORMAL*/,
      5 /*BEST_EFFORT*/
  }});
  threadManager->start();

  EXPECT_EQ(1, threadManager->workerCount(PRIORITY::HIGH_IMPORTANT));
  EXPECT_EQ(2, threadManager->workerCount(PRIORITY::HIGH));
  EXPECT_EQ(3, threadManager->workerCount(PRIORITY::IMPORTANT));
  EXPECT_EQ(4, threadManager->workerCount(PRIORITY::NORMAL));
  EXPECT_EQ(5, threadManager->workerCount(PRIORITY::BEST_EFFORT));

  threadManager->addWorker(PRIORITY::HIGH_IMPORTANT, 1);
  threadManager->addWorker(PRIORITY::HIGH, 1);
  threadManager->addWorker(PRIORITY::IMPORTANT, 1);
  threadManager->addWorker(PRIORITY::NORMAL, 1);
  threadManager->addWorker(PRIORITY::BEST_EFFORT, 1);

  EXPECT_EQ(2, threadManager->workerCount(PRIORITY::HIGH_IMPORTANT));
  EXPECT_EQ(3, threadManager->workerCount(PRIORITY::HIGH));
  EXPECT_EQ(4, threadManager->workerCount(PRIORITY::IMPORTANT));
  EXPECT_EQ(5, threadManager->workerCount(PRIORITY::NORMAL));
  EXPECT_EQ(6, threadManager->workerCount(PRIORITY::BEST_EFFORT));

  threadManager->removeWorker(PRIORITY::HIGH_IMPORTANT, 1);
  threadManager->removeWorker(PRIORITY::HIGH, 1);
  threadManager->removeWorker(PRIORITY::IMPORTANT, 1);
  threadManager->removeWorker(PRIORITY::NORMAL, 1);
  threadManager->removeWorker(PRIORITY::BEST_EFFORT, 1);

  EXPECT_EQ(1, threadManager->workerCount(PRIORITY::HIGH_IMPORTANT));
  EXPECT_EQ(2, threadManager->workerCount(PRIORITY::HIGH));
  EXPECT_EQ(3, threadManager->workerCount(PRIORITY::IMPORTANT));
  EXPECT_EQ(4, threadManager->workerCount(PRIORITY::NORMAL));
  EXPECT_EQ(5, threadManager->workerCount(PRIORITY::BEST_EFFORT));
}

// DO_BEFORE(aristidis,20250715): Test is flaky. Find owner or remove.
TEST_F(ThreadManagerTest, DISABLED_PriorityQueueThreadManagerExecutor) {
  auto threadManager = ThreadManager::newPriorityQueueThreadManager(1);
  threadManager->start();
  folly::Baton<> reqSyncBaton;
  folly::Baton<> reqDoneBaton;
  // block the TM
  threadManager->add([&] { reqSyncBaton.wait(); });

  std::string foo;
  threadManager->addWithPriority(
      [&] {
        foo += "a";
        reqDoneBaton.post();
      },
      0);
  // Should be added by default at highest priority
  threadManager->add([&] { foo += "b"; });
  threadManager->addWithPriority([&] { foo += "c"; }, 1);

  // unblock the TM
  reqSyncBaton.post();

  // wait until the request that's supposed to finish last is done
  reqDoneBaton.wait();

  EXPECT_EQ("bca", foo);
}

std::array<std::function<std::shared_ptr<ThreadManager>()>, 3> factories = {
    std::bind(
        (std::shared_ptr<ThreadManager> (*)(
            size_t))ThreadManager::newSimpleThreadManager,
        1),
    std::bind(ThreadManager::newPriorityQueueThreadManager, 1),
    []() -> std::shared_ptr<apache::thrift::concurrency::ThreadManager> {
      return PriorityThreadManager::newPriorityThreadManager({{
          1 /*HIGH_IMPORTANT*/,
          2 /*HIGH*/,
          3 /*IMPORTANT*/,
          4 /*NORMAL*/,
          5 /*BEST_EFFORT*/
      }});
    }};
class JoinTest : public testing::TestWithParam<
                     std::function<std::shared_ptr<ThreadManager>()>> {};

// DO_BEFORE(aristidis,20250715): Test is flaky. Find owner or remove.
TEST_P(JoinTest, DISABLED_Join) {
  auto threadManager = GetParam()();
  auto threadFactory = std::make_shared<PosixThreadFactory>();
  threadManager->threadFactory(threadFactory);
  threadManager->start();
  folly::Baton<> wait1, wait2, joinStarted, joined;
  // block the TM
  threadManager->add(
      FunctionRunner::create([&] { wait1.wait(); }), 0, 0, false);
  threadManager->add(FunctionRunner::create([&] { wait2.wait(); }), 0, 0, true);
  std::thread t([&] {
    joinStarted.post();
    threadManager->join();
    joined.post();
  });

  joinStarted.wait();
  EXPECT_FALSE(joined.try_wait_for(std::chrono::milliseconds(100)));
  joined.reset();
  wait1.post();
  EXPECT_FALSE(joined.try_wait_for(std::chrono::milliseconds(100)));
  joined.reset();
  wait2.post();
  EXPECT_TRUE(joined.try_wait_for(std::chrono::milliseconds(100)));
  t.join();
}

INSTANTIATE_TEST_CASE_P(
    ThreadManagerTest, JoinTest, ::testing::ValuesIn(factories));

#if FOLLY_HAVE_WEAK_SYMBOLS
class TMThreadIDCollectorTest : public ::testing::Test {
 protected:
  void SetUp() override { kWorkerProviderGlobal = nullptr; }
  void TearDown() override { kWorkerProviderGlobal = nullptr; }
  static constexpr size_t kNumThreads = 4;
};

TEST_F(TMThreadIDCollectorTest, BasicTest) {
  // This is a sanity check test. We start a ThreadManager, queue a task,
  // and then invoke the collectThreadIds() API to capture the TID of the
  // active thread.
  auto tm = ThreadManager::newSimpleThreadManager(1);
  tm->setNamePrefix("baz");
  tm->threadFactory(std::make_shared<PosixThreadFactory>());
  tm->start();

  std::atomic<pid_t> threadId = {};
  folly::Baton<> bat;
  tm->add([&]() {
    threadId.exchange(folly::getOSThreadID());
    bat.post();
  });
  {
    bat.wait();
    auto idsWithKA = kWorkerProviderGlobal->collectThreadIds();
    auto& ids = idsWithKA.threadIds;
    EXPECT_EQ(ids.size(), 1);
    EXPECT_EQ(ids[0], threadId.load());
  }
}

TEST_F(TMThreadIDCollectorTest, CollectIDMultipleInvocationTest) {
  // This test verifies that multiple invocations of collectthreadId()
  // do not deadlock.
  auto tm = ThreadManager::newSimpleThreadManager(kNumThreads);
  tm->setNamePrefix("bar");
  tm->threadFactory(std::make_shared<PosixThreadFactory>());
  tm->start();

  folly::Synchronized<std::vector<pid_t>> threadIds;
  std::array<folly::Baton<>, kNumThreads> bats;
  folly::Baton<> tasksAddedBat;
  for (size_t i = 0; i < kNumThreads; ++i) {
    tm->add([i, &threadIds, &bats, &tasksAddedBat]() {
      threadIds.wlock()->push_back(folly::getOSThreadID());
      if (i == kNumThreads - 1) {
        tasksAddedBat.post();
      }
      bats[i].wait();
    });
  }
  {
    tasksAddedBat.wait();
    auto idsWithKA1 = kWorkerProviderGlobal->collectThreadIds();
    auto idsWithKA2 = kWorkerProviderGlobal->collectThreadIds();
    auto& ids1 = idsWithKA1.threadIds;
    auto& ids2 = idsWithKA2.threadIds;
    EXPECT_EQ(ids1.size(), 4);
    EXPECT_EQ(ids1.size(), ids2.size());
    EXPECT_EQ(ids1, ids2);
  }
  for (auto& bat : bats) {
    bat.post();
  }
  tm->join();
}

TEST_F(TMThreadIDCollectorTest, CollectIDBlocksThreadExitTest) {
  // This test verifies that collectThreadId() call prevents the
  // active ThreadManager threads from exiting.
  auto tm = ThreadManager::newSimpleThreadManager(kNumThreads);
  tm->setNamePrefix("bar");
  tm->threadFactory(std::make_shared<PosixThreadFactory>());
  tm->start();

  std::array<folly::Baton<>, kNumThreads> bats;
  folly::Baton<> tasksAddedBat;
  for (size_t i = 0; i < kNumThreads; ++i) {
    tm->add([i, &bats, &tasksAddedBat]() {
      if (i == kNumThreads - 1) {
        tasksAddedBat.post();
      }
      bats[i].wait();
    });
  }
  folly::Baton<> waitForCollectBat;
  folly::Baton<> threadCountBat;
  auto bgCollector = std::thread([&]() {
    tasksAddedBat.wait();
    auto idsWithKA = kWorkerProviderGlobal->collectThreadIds();
    waitForCollectBat.post();
    auto posted = threadCountBat.try_wait_for(std::chrono::milliseconds(100));
    // The thread count reduction should not have returned (thread exit
    // is currently blocked).
    EXPECT_FALSE(posted);
    auto& ids = idsWithKA.threadIds;
    EXPECT_EQ(ids.size(), kNumThreads);
  });
  waitForCollectBat.wait();
  for (auto& bat : bats) {
    bat.post();
  }
  tm->removeWorker(2);
  threadCountBat.post();
  bgCollector.join();
  EXPECT_EQ(tm->workerCount(), 2);
  tm->join();
}
#endif

//
// =============================================================================
// This section validates the cpu time accounting logic in ThreadManager. It
// requires thread-specific clocks, so only Linux is supported at this time.
// =============================================================================
//
#ifdef __linux__

// Like ASSERT_NEAR, but for chrono duration types
#define ASSERT_NEAR_NS(a, b, c)  \
  do {                           \
    ASSERT_NEAR(                 \
        nanoseconds(a).count(),  \
        nanoseconds(b).count(),  \
        nanoseconds(c).count()); \
  } while (0)

static std::chrono::nanoseconds thread_clock_now() {
  timespec tp;
  clockid_t clockid;
  CHECK(!pthread_getcpuclockid(pthread_self(), &clockid));
  CHECK(!clock_gettime(clockid, &tp));
  return std::chrono::nanoseconds(tp.tv_nsec) + std::chrono::seconds(tp.tv_sec);
}

// Burn thread cpu cycles
static void burn(std::chrono::milliseconds ms) {
  auto expires = thread_clock_now() + ms;
  while (thread_clock_now() < expires) {
  }
}

// Loop without using much cpu time
static void idle(std::chrono::milliseconds ms) {
  using clock = std::chrono::high_resolution_clock;
  auto expires = clock::now() + ms;
  while (clock::now() < expires) {
    /* sleep override */
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

TEST_F(ThreadManagerTest, UsedCpuTime_Simple) {
  using namespace std::chrono;
  auto tm = ThreadManager::newSimpleThreadManager(3);
  tm->start();

  const auto t0 = tm->getUsedCpuTime();

  // Schedule 3 threads: two doing busy work, 1 idling
  {
    folly::Latch latch(3);
    tm->add([&] { // + 500ms cpu time
      burn(500ms);
      latch.count_down();
    });
    tm->add([&] { // + 300ms cpu time
      burn(300ms);
      latch.count_down();
    });
    tm->add([&] { // + ~0ms cpu time
      idle(500ms);
      latch.count_down();
    });
    latch.wait();
    ASSERT_EQ(tm->workerCount(), 3);
    ASSERT_NEAR_NS(tm->getUsedCpuTime() - t0, 800ms, 20ms); // = 800ms ± 20ms
  }

  // Remove one thread, cpu time should not change
  {
    tm->removeWorker(1);
    ASSERT_EQ(tm->workerCount(), 2);
    ASSERT_NEAR_NS(tm->getUsedCpuTime() - t0, 800ms, 20ms); // = 800ms ± 20ms
  }

  // Do a bit more work, cpu time should add to previous value
  {
    folly::Latch latch(1);
    tm->add([&] { // + 200ms cpu time
      burn(200ms);
      latch.count_down();
    });
    latch.wait();
    ASSERT_NEAR_NS(tm->getUsedCpuTime() - t0, 1s, 20ms); // = 1s ± 20ms
  }

  // Remove all threads, cpu time should be preserved
  {
    tm->removeWorker(2);
    ASSERT_EQ(tm->workerCount(), 0);
    ASSERT_NEAR_NS(tm->getUsedCpuTime() - t0, 1s, 20ms); // = 1s ± 20ms
  }
}

TEST_F(ThreadManagerTest, UsedCpuTime_Priority) {
  using namespace std::chrono;
  auto tm = PriorityThreadManager::newPriorityThreadManager({{
      1 /*HIGH_IMPORTANT*/,
      1 /*HIGH*/,
      1 /*IMPORTANT*/,
      1 /*NORMAL*/,
      1 /*BEST_EFFORT*/
  }});
  tm->start();

  const auto t0 = tm->getUsedCpuTime();

  auto runner = [](std::function<void()>&& fn) {
    return FunctionRunner::create(std::move(fn));
  };

  // Schedule 3 threads: 2 doing busy work, 1 idling
  folly::Latch latch(3);
  tm->add(HIGH, runner([&] {
            burn(500ms);
            latch.count_down();
          })); // + 500ms cpu time
  tm->add(NORMAL, runner([&] {
            burn(300ms);
            latch.count_down();
          })); // + 300ms cpu time
  tm->add(BEST_EFFORT, runner([&] {
            idle(500ms);
            latch.count_down();
          })); // + 0ms cpu time
  latch.wait();
  ASSERT_NEAR_NS(tm->getUsedCpuTime() - t0, 800ms, 20ms); // = 800ms ± 20ms

  // Removing a thread should preserve cpu time
  tm->removeWorker(NORMAL, 1);
  ASSERT_EQ(tm->workerCount(NORMAL), 0);
  ASSERT_NEAR_NS(tm->getUsedCpuTime() - t0, 800ms, 20ms); // = 800ms ± 20ms
}

#else //  __linux__

//
// On other platforms, just make sure getUsedCpuTime() does not crash and
// returns 0.
//

TEST_F(ThreadManagerTest, UsedCpuTime) {
  using namespace std::chrono;

  auto tm = ThreadManager::newSimpleThreadManager(3);
  tm->start();

  ASSERT_EQ(tm->getUsedCpuTime().count(), 0);

  auto burn = [](milliseconds ms) {
    auto expires = steady_clock::now() + ms;
    while (steady_clock::now() < expires) {
    }
  };

  folly::Latch latch(1);
  tm->add([&] {
    burn(500ms);
    latch.count_down();
  });
  latch.wait();

  ASSERT_EQ(tm->getUsedCpuTime().count(), 0);
}

#endif // __linux__
