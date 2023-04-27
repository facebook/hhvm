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

#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>

namespace apache {
namespace thrift {

class FakeThreadManager : public apache::thrift::concurrency::ThreadManager {
 public:
  ~FakeThreadManager() override {}

  void start() override {}

  void join() override {}

  void add(folly::Func f) noexcept override {
    auto thread =
        factory_.newThread(concurrency::FunctionRunner::create(std::move(f)));
    thread->start();
  }

  [[nodiscard]] KeepAlive<> getKeepAlive(
      ExecutionScope, Source) const override {
    return getKeepAliveToken(const_cast<FakeThreadManager*>(this));
  }

  // Following methods are not required for this fake object.
  void add(
      std::shared_ptr<apache::thrift::concurrency::Runnable> /*task*/,
      int64_t /*timeout*/,
      int64_t /*expiration*/,
      apache::thrift::concurrency::ThreadManager::Source
      /*source*/) noexcept override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  void stop() override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  STATE state() const override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return STARTED;
  }

  std::shared_ptr<apache::thrift::concurrency::ThreadFactory> threadFactory()
      const override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return std::shared_ptr<apache::thrift::concurrency::ThreadFactory>();
  }

  void threadFactory(
      std::shared_ptr<apache::thrift::concurrency::ThreadFactory> /*value*/)
      override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  std::string getNamePrefix() const override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return "";
  }

  void setNamePrefix(const std::string& /*name*/) override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  void addWorker(size_t /*value*/) override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  void removeWorker(size_t /*value*/) override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  size_t idleWorkerCount() const override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return 1;
  }

  size_t workerCount() const override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return 1;
  }

  size_t pendingTaskCount() const override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return 1;
  }

  size_t pendingUpstreamTaskCount() const override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return 1;
  }

  size_t totalTaskCount() const override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return 1;
  }

  size_t expiredTaskCount() override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return 1;
  }

  void remove(std::shared_ptr<apache::thrift::concurrency::Runnable> /*task*/)
      override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  std::shared_ptr<apache::thrift::concurrency::Runnable> removeNextPending()
      override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return std::shared_ptr<apache::thrift::concurrency::Runnable>();
  }

  void clearPending() override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  void setExpireCallback(ExpireCallback /*expireCallback*/) override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  void setCodelCallback(ExpireCallback /*expireCallback*/) override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  void setThreadInitCallback(InitCallback /*initCallback*/) override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  void enableCodel(bool) override {
    LOG(FATAL) << "Method not implemented in this fake object";
  }

  bool codelEnabled() const override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return false;
  }

  folly::Codel* getCodel() override {
    LOG(FATAL) << "Method not implemented in this fake object";
    return nullptr;
  }

 private:
  apache::thrift::concurrency::PosixThreadFactory factory_;
};

} // namespace thrift
} // namespace apache
