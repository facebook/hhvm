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

#pragma once

#include <sys/types.h>

#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <folly/DefaultKeepAliveExecutor.h>
#include <folly/Executor.h>
#include <folly/Portability.h>
#include <folly/executors/Codel.h>
#include <folly/executors/QueueObserver.h>
#include <folly/io/async/Request.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/Unistd.h>
#include <folly/synchronization/LifoSem.h>
#include <folly/system/HardwareConcurrency.h>

#include <thrift/lib/cpp/concurrency/FunctionRunner.h>
#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>
#include <thrift/lib/cpp/concurrency/Thread.h>

FOLLY_GFLAGS_DECLARE_bool(codel_enabled);

namespace apache::thrift::concurrency {

class Runnable;
class ThreadFactory;
class ThreadManagerObserver;

/**
 * ThreadManager class
 *
 * This class manages a pool of threads. It uses a ThreadFactory to create
 * threads. It never actually creates or destroys worker threads, rather
 * It maintains statistics on number of idle threads, number of active threads,
 * task backlog, and average wait and service times and informs the PoolPolicy
 * object bound to instances of this manager of interesting transitions. It is
 * then up the PoolPolicy object to decide if the thread pool size needs to be
 * adjusted and call this object addWorker and removeWorker methods to make
 * changes.
 *
 * This design allows different policy implementations to used this code to
 * handle basic worker thread management and worker task execution and focus on
 * policy issues. The simplest policy, StaticPolicy, does nothing other than
 * create a fixed number of threads.
 */
class ThreadManager : public virtual folly::Executor {
 protected:
  ThreadManager() {}

 public:
  using PRIORITY = apache::thrift::concurrency::PRIORITY;

  static const size_t DEFAULT_MAX_QUEUE_SIZE = 1 << 16; // should be power of 2

  class Task;
  using ExpireCallback = std::function<void(std::shared_ptr<Runnable>)>;
  using InitCallback = std::function<void()>;

  ~ThreadManager() override {}

  /**
   * Starts the thread manager. Verifies all attributes have been properly
   * initialized, then allocates necessary resources to begin operation
   */
  virtual void start() = 0;

  /**
   * Stops the thread manager. Aborts all remaining unprocessed task, shuts
   * down all created worker threads, and releases all allocated resources.
   * This method blocks for all worker threads to complete, thus it can
   * potentially block forever if a worker thread is running a task that
   * won't terminate.
   */
  virtual void stop() = 0;

  /**
   * Joins the thread manager. This is the same as stop, except that it will
   * wait until all the tasks have finished, rather than aborting the tasks.
   */
  virtual void join() = 0;

  enum STATE {
    UNINITIALIZED,
    STARTING,
    STARTED,
    JOINING,
    STOPPING,
    STOPPED,
  };

  virtual STATE state() const = 0;

  virtual std::shared_ptr<ThreadFactory> threadFactory() const = 0;

  virtual void threadFactory(std::shared_ptr<ThreadFactory> value) = 0;

  virtual std::string getNamePrefix() const = 0;

  virtual void setNamePrefix(const std::string& name) = 0;

  virtual void addWorker(size_t value = 1) = 0;

  virtual void removeWorker(size_t value = 1) = 0;

  /**
   * Gets the current number of idle worker threads
   */
  virtual size_t idleWorkerCount() const = 0;

  /**
   * Gets the current number of total worker threads
   */
  virtual size_t workerCount() const = 0;

  /**
   * Gets the current number of pending tasks
   */
  virtual size_t pendingTaskCount() const = 0;

  /**
   * Gets the current number of pending tasks
   */
  virtual size_t pendingUpstreamTaskCount() const = 0;

  /**
   * Gets the current number of pending and executing tasks
   */
  virtual size_t totalTaskCount() const = 0;

  /**
   * Gets the number of tasks which have been expired without being run.
   */
  virtual size_t expiredTaskCount() = 0;

  enum class Source {
    INTERNAL = 0,
    EXISTING_INTERACTION = 1,
    UPSTREAM = 2,
    LAST = UPSTREAM,
  };
  static constexpr int N_SOURCES = static_cast<int>(Source::LAST) + 1;

  /**
   * Adds a task to be executed at some time in the future by a worker thread.
   *
   * @param task  The task to queue for execution
   *
   * @param timeout, this argument is deprecated, add() will always be
   * non-blocking
   *
   * @param expiration when nonzero, the number of milliseconds the task is
   * valid to be run; if exceeded, the task will be dropped off the queue and
   * not run.
   * @param upstream hint whether the task is come from an upstream.
   * Implementations may prioritize those tasks different than other tasks of
   * the same priority.
   */
  virtual void add(
      std::shared_ptr<Runnable> task,
      int64_t timeout,
      int64_t expiration,
      Source source) noexcept = 0;
  void add(
      std::shared_ptr<Runnable> task,
      int64_t timeout = 0,
      int64_t expiration = 0,
      bool upstream = false) noexcept {
    add(std::move(task),
        timeout,
        expiration,
        upstream ? Source::UPSTREAM : Source::INTERNAL);
  }

  /**
   * Implements folly::Executor::add()
   */
  void add(folly::Func f) override = 0;

  /**
   * Removes a pending task
   */
  virtual void remove(std::shared_ptr<Runnable> task) = 0;

  /**
   * Remove the next pending task which would be run.
   *
   * @return the task removed.
   */
  virtual std::shared_ptr<Runnable> removeNextPending() = 0;

  /**
   * Removes all pending tasks.
   */
  virtual void clearPending() = 0;

  /**
   * Set a callback to be called when a task is expired and not run.
   *
   * @param expireCallback a function called with the shared_ptr<Runnable> for
   * the expired task.
   */
  virtual void setExpireCallback(ExpireCallback expireCallback) = 0;
  virtual void setCodelCallback(ExpireCallback expireCallback) = 0;

  /**
   * Set a callback to be called when a worker thread is created.
   */
  virtual void setThreadInitCallback(InitCallback initCallback) = 0;

  /**
   * Creates a simple thread manager that uses count number of worker threads
   */
  static std::shared_ptr<ThreadManager> newSimpleThreadManager(
      size_t count = 4);

  /**
   * Creates a simple thread manager that uses count number of worker threads
   * and sets the name prefix
   */
  static std::shared_ptr<ThreadManager> newSimpleThreadManager(
      const std::string& name,
      size_t count = 4,
      PosixThreadFactory::THREAD_PRIORITY priority =
          PosixThreadFactory::NORMAL_PRI);

  /**
   * Creates a thread manager with support for priorities. Unlike
   * PriorityThreadManager, requests are still served from a single
   * thread pool.
   */
  static std::shared_ptr<ThreadManager> newPriorityQueueThreadManager(
      size_t numThreads);

  struct RunStats {
    const std::string& threadPoolName;
    std::chrono::steady_clock::time_point queueBegin;
    std::chrono::steady_clock::time_point workBegin;
    std::chrono::steady_clock::time_point workEnd;
  };

  class Observer {
   public:
    virtual ~Observer() {}

    virtual void preRun(folly::RequestContext*) = 0;
    virtual void postRun(folly::RequestContext*, const RunStats&) = 0;
  };

  static void setGlobalObserver(std::shared_ptr<Observer> observer);

  virtual void addTaskObserver(std::shared_ptr<Observer>) {
    LOG(FATAL) << "Method not implemented";
  }

  /**
   * Returns the cpu time used by this thread pool. Requires support for
   * thread-specific clocks.
   */
  virtual std::chrono::nanoseconds getUsedCpuTime() const {
    return std::chrono::nanoseconds(0);
  }

  virtual void enableCodel(bool) = 0;

  virtual bool codelEnabled() const = 0;

  virtual folly::Codel* getCodel() = 0;

  /**
   * This class may be used by getKeepAlive() to decide which executor to
   * return.
   */
  class ExecutionScope {
   public:
    ExecutionScope(PRIORITY priority) : pri_(priority) {}

    PRIORITY getPriority() const { return pri_; }

    void setTenantId(std::optional<uint32_t> tenant) { tenant_id_ = tenant; }

    std::optional<uint32_t> getTenantId() const { return tenant_id_; }

   private:
    PRIORITY pri_;
    std::optional<uint32_t> tenant_id_;
  };

  [[nodiscard]] virtual KeepAlive<> getKeepAlive(
      ExecutionScope es, Source level) const = 0;

  class Impl;
};

/**
 * PriorityThreadManager class
 *
 * This class extends ThreadManager by adding priorities to tasks.
 * It is up to the specific implementation to define how worker threads
 * are assigned to run tasks of various priorities. The current
 * implementation, PriorityImpl, runs a task on a thread with exactly
 * the same priority.
 *
 * The interface of this class was kept as close to ThreadManager's
 * as possible, diverging only where the original interface doesn't
 * make sense in the priority world.
 */
class PriorityThreadManager : public ThreadManager {
 public:
  using ThreadManager::addWorker;
  virtual void addWorker(PRIORITY priority, size_t value) = 0;

  using ThreadManager::removeWorker;
  virtual void removeWorker(PRIORITY priority, size_t value) = 0;

  using ThreadManager::workerCount;
  virtual size_t workerCount(PRIORITY priority) = 0;

  using ThreadManager::pendingTaskCount;
  virtual size_t pendingTaskCount(PRIORITY priority) const = 0;

  using ThreadManager::idleWorkerCount;
  virtual size_t idleWorkerCount(PRIORITY priority) const = 0;

  using ThreadManager::add;
  virtual void add(
      PRIORITY priority,
      std::shared_ptr<Runnable> task,
      int64_t timeout,
      int64_t expiration,
      ThreadManager::Source source) noexcept = 0;
  void add(
      PRIORITY priority,
      std::shared_ptr<Runnable> task,
      int64_t timeout = 0,
      int64_t expiration = 0,
      bool upstream = false) noexcept {
    add(priority,
        std::move(task),
        timeout,
        expiration,
        upstream ? Source::UPSTREAM : Source::INTERNAL);
  }

  uint8_t getNumPriorities() const override { return N_PRIORITIES; }

  using ThreadManager::codelEnabled;
  using ThreadManager::getCodel;
  virtual folly::Codel* getCodel(PRIORITY priority) = 0;

  /**
   * Creates a priority-aware thread manager given thread factory and size for
   * each priority.
   *
   * At least NORMAL_PRIORITY_MINIMUM_THREADS threads are created for
   * priority NORMAL.
   */
  static std::shared_ptr<PriorityThreadManager> newPriorityThreadManager(
      const std::array<
          std::pair<std::shared_ptr<ThreadFactory>, size_t>,
          N_PRIORITIES>& counts);

  /**
   * Creates a priority-aware thread manager that uses counts[X]
   * worker threads for priority X.
   */
  static std::shared_ptr<PriorityThreadManager> newPriorityThreadManager(
      const std::array<size_t, N_PRIORITIES>& counts);

  /**
   * Creates a priority-aware thread manager that uses normalThreadsCount
   * threads of NORMAL priority, and an implementation-defined number of threads
   * for other priorities. Useful when the vast majority of tasks are NORMAL,
   * but occasionally a non-NORMAL task may arrive.
   *
   * @param normalThreadsCount - number of threads of NORMAL priority, defaults
   *          to the number of CPUs on the system
   */
  static std::shared_ptr<PriorityThreadManager> newPriorityThreadManager(
      size_t normalThreadsCount = folly::available_concurrency());

  class PriorityImpl;
};

FOLLY_PUSH_WARNING
FOLLY_MSVC_DISABLE_WARNING(4250) // inherits keepAliveAcq/Rel via dominance
// Adapter class that converts a folly::Executor to a ThreadManager interface
class ThreadManagerExecutorAdapter : public ThreadManager,
                                     public folly::DefaultKeepAliveExecutor {
 public:
  struct Options {
    Options();
    explicit Options(std::string name) : wrappedExecutorName(std::move(name)) {}
    std::string wrappedExecutorName;
    // When wrapping a generic executor, propagate this parameter to the
    // MeteredExecutors.
    uint32_t meteredExecutorMaxInQueue = 1;
  };

  /* implicit */
  explicit ThreadManagerExecutorAdapter(
      std::shared_ptr<folly::Executor> exe, Options opts = Options());
  explicit ThreadManagerExecutorAdapter(
      folly::Executor::KeepAlive<> ka, Options opts = Options());
  explicit ThreadManagerExecutorAdapter(
      std::array<std::shared_ptr<Executor>, N_PRIORITIES>,
      Options opts = Options());

  ~ThreadManagerExecutorAdapter() override;

  void join() override;
  void start() override;
  void stop() override;
  STATE state() const override { return STARTED; }

  std::shared_ptr<ThreadFactory> threadFactory() const override;
  void threadFactory(std::shared_ptr<ThreadFactory> value) override;
  std::string getNamePrefix() const override;
  void setNamePrefix(const std::string& name) override;
  void addWorker(size_t value = 1) override;
  void removeWorker(size_t value = 1) override;

  size_t idleWorkerCount() const override;
  size_t workerCount() const override;
  size_t pendingUpstreamTaskCount() const override;
  size_t pendingTaskCount() const override;
  size_t totalTaskCount() const override;
  size_t expiredTaskCount() override;

  void add(
      std::shared_ptr<Runnable> task,
      int64_t /*timeout*/,
      int64_t /*expiration*/,
      ThreadManager::Source source) noexcept override;
  void add(folly::Func f) override;

  void remove(std::shared_ptr<Runnable> /*task*/) override {}
  std::shared_ptr<Runnable> removeNextPending() override { return nullptr; }
  void clearPending() override {}

  void setExpireCallback(ExpireCallback /*expireCallback*/) override {}
  void setCodelCallback(ExpireCallback /*expireCallback*/) override {}
  void setThreadInitCallback(InitCallback /*initCallback*/) override {}
  void enableCodel(bool) override {}
  bool codelEnabled() const override { return false; }
  folly::Codel* getCodel() override { return nullptr; }

  [[nodiscard]] KeepAlive<> getKeepAlive(
      ExecutionScope es, Source source) const override;

 protected:
  bool fromGenericExecutor() const { return fromGenericExecutor_; }

 private:
  explicit ThreadManagerExecutorAdapter(
      std::array<folly::Executor*, N_PRIORITIES * N_SOURCES>);

  void joinKeepAliveOnce() {
    if (!std::exchange(keepAliveJoined_, true)) {
      joinKeepAlive();
    }
  }

  std::vector<std::shared_ptr<void>> owning_;
  std::array<folly::Executor*, N_PRIORITIES * N_SOURCES> executors_;
  bool fromGenericExecutor_ = false;
  bool keepAliveJoined_{false};
  Options opts_;
};
FOLLY_POP_WARNING

FOLLY_PUSH_WARNING
FOLLY_MSVC_DISABLE_WARNING(4250) // inherits keepAliveAcq/Rel via dominance
class SimpleThreadManager : public ThreadManager,
                            public folly::DefaultKeepAliveExecutor {
 public:
  explicit SimpleThreadManager(size_t workerCount = 4);
  ~SimpleThreadManager() override;

  void start() override;
  void stop() override;
  void join() override;
  STATE state() const override;
  std::shared_ptr<ThreadFactory> threadFactory() const override;
  void threadFactory(std::shared_ptr<ThreadFactory> value) override;
  std::string getNamePrefix() const override;
  void setNamePrefix(const std::string& name) override;
  void addWorker(size_t value = 1) override;
  void removeWorker(size_t value = 1) override;
  size_t idleWorkerCount() const override;
  size_t workerCount() const override;
  size_t pendingTaskCount() const override;
  size_t pendingUpstreamTaskCount() const override;
  size_t totalTaskCount() const override;
  size_t expiredTaskCount() override;
  void add(
      std::shared_ptr<Runnable> task,
      int64_t timeout,
      int64_t expiration,
      Source source) noexcept override;
  void add(folly::Func f) override;
  void remove(std::shared_ptr<Runnable> task) override;
  std::shared_ptr<Runnable> removeNextPending() override;
  void clearPending() override;
  void enableCodel(bool) override;
  bool codelEnabled() const override;
  folly::Codel* getCodel() override;
  void setExpireCallback(ExpireCallback expireCallback) override;
  void setCodelCallback(ExpireCallback expireCallback) override;
  void setThreadInitCallback(InitCallback initCallback) override;
  void addTaskObserver(std::shared_ptr<Observer> observer) override;
  std::chrono::nanoseconds getUsedCpuTime() const override;

  [[nodiscard]] KeepAlive<> getKeepAlive(
      ExecutionScope es, Source source) const override;

 private:
  void joinKeepAliveOnce() {
    if (!std::exchange(keepAliveJoined_, true)) {
      joinKeepAlive();
    }
  }

  std::unique_ptr<Impl> impl_;
  bool keepAliveJoined_{false};
};
FOLLY_POP_WARNING

inline std::shared_ptr<ThreadFactory> Factory(
    PosixThreadFactory::THREAD_PRIORITY prio) {
  return std::make_shared<PosixThreadFactory>(PosixThreadFactory::OTHER, prio);
}

} // namespace apache::thrift::concurrency
