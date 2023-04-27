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

#include <folly/synchronization/CallOnce.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp2/server/ResourcePool.h>

namespace apache::thrift {

class ThriftServer;

// This is a thin wrapper over folly::Executor to convert
// the executor to a ThreadManager
//
// It only expose add() interface to act as an Executor
//
// This is only for the purpose of ResourcePool migration,
// This should not be used for any custom purpose
class ExecutorToThreadManagerAdaptor : public concurrency::ThreadManager {
 public:
  explicit ExecutorToThreadManagerAdaptor(ResourcePoolSet& resourcePoolSet)
      : resourcePoolSet_(resourcePoolSet),
        defaultAsyncExecutor_(
            resourcePoolSet.resourcePool(ResourcePoolHandle::defaultAsync())
                .executor()
                .value()
                .get()) {}

  // These are the only two interfaces that are implemented
  void add(
      std::shared_ptr<concurrency::Runnable> task,
      [[maybe_unused]] int64_t timeout = 0,
      [[maybe_unused]] int64_t expiration = 0,
      [[maybe_unused]] Source source = Source::UPSTREAM) noexcept override {
    defaultAsyncExecutor_.add([task = std::move(task)]() { task->run(); });
  }

  void add(folly::Func f) override { defaultAsyncExecutor_.add(std::move(f)); }

  void start() override {}

  void stop() override {}

  void join() override {}

  STATE state() const override { return concurrency::ThreadManager::STARTED; }

  std::shared_ptr<concurrency::ThreadFactory> threadFactory() const override {
    return std::shared_ptr<concurrency::ThreadFactory>(
        new concurrency::PosixThreadFactory());
  }

  void threadFactory(std::shared_ptr<concurrency::ThreadFactory>) override {}

  std::string getNamePrefix() const override {
    return "rp.executor_to_thread_manager_adaptor";
  }

  void setNamePrefix(const std::string&) override {}

  void addWorker(size_t count) override { adjustWorkerCount(count); }

  void removeWorker(size_t count) override { adjustWorkerCount(-count); }

  size_t idleWorkerCount() const override {
    return resourcePoolSet_.idleWorkerCount();
  }

  size_t workerCount() const override { return resourcePoolSet_.workerCount(); }

  size_t pendingTaskCount() const override {
    return resourcePoolSet_.numQueued();
  }

  size_t pendingUpstreamTaskCount() const override { return 0; }

  size_t totalTaskCount() const override {
    return resourcePoolSet_.numQueued() + resourcePoolSet_.numInExecution();
  }

  size_t expiredTaskCount() override { return 0; }

  void remove(std::shared_ptr<concurrency::Runnable>) override {}

  std::shared_ptr<concurrency::Runnable> removeNextPending() override {
    return nullptr;
  }

  void clearPending() override {}

  void enableCodel(bool) override {}

  bool codelEnabled() const override { return false; }

  folly::Codel* getCodel() override { return nullptr; }

  void setExpireCallback(ExpireCallback) override {}

  void setCodelCallback(ExpireCallback) override {}

  void setThreadInitCallback(InitCallback) override {}

  void addTaskObserver(std::shared_ptr<Observer>) override {}

  std::chrono::nanoseconds getUsedCpuTime() const override {
    return std::chrono::nanoseconds();
  }

  [[nodiscard]] KeepAlive<> getKeepAlive(
      ExecutionScope, Source) const override {
    return folly::getKeepAliveToken(defaultAsyncExecutor_);
  }

 private:
  ResourcePoolSet& resourcePoolSet_;
  folly::Executor& defaultAsyncExecutor_;

  void adjustWorkerCount(int delta) {
    if (auto* asThreadPoolExecutor =
            dynamic_cast<folly::ThreadPoolExecutor*>(&defaultAsyncExecutor_)) {
      int count = int(asThreadPoolExecutor->numThreads());
      if (delta < 0 && count < std::abs(delta)) {
        // This is what ThreadManger does
        throw concurrency::InvalidArgumentException();
      }
      asThreadPoolExecutor->setNumThreads(count + delta);
    }
  }
};

} // namespace apache::thrift
