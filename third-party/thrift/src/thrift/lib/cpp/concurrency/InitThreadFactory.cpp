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

#include <thrift/lib/cpp/concurrency/InitThreadFactory.h>

#include <functional>
#include <memory>

#include <folly/ScopeGuard.h>

namespace apache::thrift::concurrency {

namespace {

// Used with InitThreadFactory
class InitRunnable : public Runnable {
 public:
  explicit InitRunnable(
      std::function<void()> threadInitializer,
      std::function<void()> threadFinalizer,
      std::shared_ptr<Runnable> runnable)
      : threadInitializer_(std::move(threadInitializer)),
        threadFinalizer_(std::move(threadFinalizer)),
        runnable_(runnable) {}

  void run() override {
    threadInitializer_();
    SCOPE_EXIT {
      threadFinalizer_();
    };
    runnable_->run();
  }

  std::shared_ptr<Thread> thread() const override {
    return runnable_->thread();
  }

  void thread(std::shared_ptr<Thread> value) override {
    runnable_->thread(value);
  }

 private:
  std::function<void()> threadInitializer_;
  std::function<void()> threadFinalizer_;
  std::shared_ptr<Runnable> runnable_;
};

} // anonymous namespace

std::shared_ptr<Thread> InitThreadFactory::newThread(
    const std::shared_ptr<Runnable>& runnable) const {
  return threadFactory_->newThread(
      std::make_shared<InitRunnable>(
          threadInitializer_, threadFinalizer_, runnable));
}

std::shared_ptr<Thread> InitThreadFactory::newThread(
    const std::shared_ptr<Runnable>& runnable, DetachState detachState) const {
  return threadFactory_->newThread(
      std::make_shared<InitRunnable>(
          threadInitializer_, threadFinalizer_, runnable),
      detachState);
}

} // namespace apache::thrift::concurrency
