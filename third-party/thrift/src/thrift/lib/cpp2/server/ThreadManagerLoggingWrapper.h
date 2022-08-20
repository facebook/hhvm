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

#include <folly/synchronization/CallOnce.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>

namespace apache::thrift {

class BaseThriftServer;

// This is a logging wrapper that wraps a ThreadManager
// It logs the methods that are of interest to ResourcePool rollout
// This will only exist temporarily and shall only be used by
// thrift server team soley
class ThreadManagerLoggingWrapper : public concurrency::ThreadManager {
 public:
  ThreadManagerLoggingWrapper(
      std::shared_ptr<concurrency::ThreadManager> tm,
      const BaseThriftServer* server,
      bool shouldLog = true)
      : tm_(std::move(tm)), server_(server), shouldLog_(shouldLog) {}

  void add(
      std::shared_ptr<concurrency::Runnable> task,
      int64_t timeout = 0,
      int64_t expiration = 0,
      Source source = Source::UPSTREAM) noexcept override {
    tm_->add(std::move(task), timeout, expiration, source);
  }

  void add(folly::Func f) override { tm_->add(std::move(f)); }

  void start() override { tm_->start(); }

  void stop() override { tm_->stop(); }

  void join() override {
    recordStackTrace("join");
    tm_->join();
  }

  STATE state() const override { return tm_->state(); }

  std::shared_ptr<concurrency::ThreadFactory> threadFactory() const override {
    recordStackTrace("threadFactory");
    return tm_->threadFactory();
  }

  void threadFactory(std::shared_ptr<concurrency::ThreadFactory> fac) override {
    recordStackTrace("threadFactory");
    tm_->threadFactory(std::move(fac));
  }

  std::string getNamePrefix() const override { return tm_->getNamePrefix(); }

  void setNamePrefix(const std::string& name) override {
    tm_->setNamePrefix(name);
  }

  void addWorker(size_t num) override {
    recordStackTrace("addWorker");
    tm_->addWorker(num);
  }

  void removeWorker(size_t num) override {
    recordStackTrace("removeWorker");
    tm_->removeWorker(num);
  }

  size_t idleWorkerCount() const override { return tm_->idleWorkerCount(); }

  size_t workerCount() const override { return tm_->workerCount(); }

  size_t pendingTaskCount() const override { return tm_->pendingTaskCount(); }

  size_t pendingUpstreamTaskCount() const override {
    return tm_->pendingUpstreamTaskCount();
  }

  size_t totalTaskCount() const override { return tm_->totalTaskCount(); }

  size_t expiredTaskCount() override { return tm_->expiredTaskCount(); }

  void remove(std::shared_ptr<concurrency::Runnable> r) override {
    recordStackTrace("remove");
    tm_->remove(std::move(r));
  }

  std::shared_ptr<concurrency::Runnable> removeNextPending() override {
    recordStackTrace("removeNextPending");
    return tm_->removeNextPending();
  }

  void clearPending() override {
    recordStackTrace("clearPending");
    tm_->clearPending();
  }

  void enableCodel(bool e) override {
    recordStackTrace("enableCodel");
    tm_->enableCodel(e);
  }

  bool codelEnabled() const override { return tm_->codelEnabled(); }

  folly::Codel* getCodel() override {
    recordStackTrace("getCodel");
    return tm_->getCodel();
  }

  void setExpireCallback(ExpireCallback e) override {
    recordStackTrace("setExpireCallback");
    tm_->setExpireCallback(std::move(e));
  }

  void setCodelCallback(ExpireCallback e) override {
    recordStackTrace("setCodelCallback");
    tm_->setCodelCallback(std::move(e));
  }

  void setThreadInitCallback(InitCallback e) override {
    recordStackTrace("setThreadInitCallback");
    tm_->setThreadInitCallback(std::move(e));
  }

  void addTaskObserver(std::shared_ptr<Observer> o) override {
    recordStackTrace("addTaskObserver");
    tm_->addTaskObserver(std::move(o));
  }

  std::chrono::nanoseconds getUsedCpuTime() const override {
    return tm_->getUsedCpuTime();
  }

  [[nodiscard]] KeepAlive<> getKeepAlive(
      ExecutionScope scope, Source source) const override {
    return tm_->getKeepAlive(scope, source);
  }

 private:
  std::shared_ptr<concurrency::ThreadManager> tm_;
  const BaseThriftServer* server_;
  bool shouldLog_;
  // logging should only be done once if any as
  // it's quite expensive
  static folly::once_flag recordFlag_;

  void recordStackTrace(std::string) const;
};

} // namespace apache::thrift
