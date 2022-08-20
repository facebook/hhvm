/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef THRIFT_CONCURRENCY_INITTHREADFACTORY_H_
#define THRIFT_CONCURRENCY_INITTHREADFACTORY_H_ 1

#include <functional>
#include <memory>

#include <thrift/lib/cpp/concurrency/Thread.h>

namespace apache {
namespace thrift {
namespace concurrency {

/**
 * Thread factory with initializer functions.
 */
class InitThreadFactory : public ThreadFactory {
 public:
  explicit InitThreadFactory(
      const std::shared_ptr<ThreadFactory>& threadFactory,
      std::function<void()>&& threadInitializer,
      std::function<void()>&& threadFinalizer = [] {})
      : threadFactory_(threadFactory),
        threadInitializer_(std::move(threadInitializer)),
        threadFinalizer_(std::move(threadFinalizer)) {}

  std::shared_ptr<Thread> newThread(
      const std::shared_ptr<Runnable>& runnable) const override;

  std::shared_ptr<Thread> newThread(
      const std::shared_ptr<Runnable>& runnable,
      DetachState detachState) const override;

  virtual Thread::id_t getCurrentThreadId() const override {
    return threadFactory_->getCurrentThreadId();
  }

 private:
  std::shared_ptr<ThreadFactory> threadFactory_;
  std::function<void()> threadInitializer_{nullptr};
  std::function<void()> threadFinalizer_{nullptr};
};

} // namespace concurrency
} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT_CONCURRENCY_INITTHREADFACTORY_H_
