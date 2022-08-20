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

#ifndef _THRIFT_CONCURRENCY_FUNCTION_RUNNER_H
#define _THRIFT_CONCURRENCY_FUNCTION_RUNNER_H 1

#include <chrono>
#include <condition_variable>
#include <mutex>

#include <folly/Function.h>
#include <folly/ScopeGuard.h>
#include <folly/portability/Unistd.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp/concurrency/Exception.h>
#include <thrift/lib/cpp/concurrency/Thread.h>

namespace apache {
namespace thrift {
namespace concurrency {

/**
 * Convenient implementation of Runnable that will execute arbitrary callbacks.
 * Interfaces are provided to accept both a generic 'void(void)' callback, and
 * a 'void* (void*)' pthread_create-style callback.
 *
 * Example use:
 *  void* my_thread_main(void* arg);
 *  shared_ptr<ThreadFactory> factory = ...;
 *  // To create a thread that executes my_thread_main once:
 *  shared_ptr<Thread> thread = factory->newThread(
 *    FunctionRunner::create(my_thread_main, some_argument));
 *  thread->start();
 *
 *  bool A::foo();
 *  A* a = new A();
 *  // To create a thread that executes a.foo() every 100 milliseconds:
 *  factory->newThread(FunctionRunner::create(
 *    std::bind(&A::foo, a), 100))->start();
 *
 */

class FunctionRunner : public virtual Runnable {
 public:
  // This is the type of callback 'pthread_create()' expects.
  typedef void* (*PthreadFuncPtr)(void* arg);
  // This a fully-generic void(void) callback for custom bindings.
  typedef folly::Function<void()> VoidFunc;

  typedef folly::Function<bool()> BoolFunc;

  /**
   * Syntactic sugar to make it easier to create new FunctionRunner
   * objects wrapped in shared_ptr.
   */
  template <class F>
  static std::shared_ptr<FunctionRunner> create(F&& cob) {
    return std::shared_ptr<FunctionRunner>(
        new FunctionRunner(std::forward<F>(cob)));
  }

  static std::shared_ptr<FunctionRunner> create(
      const PthreadFuncPtr& func, void* arg) {
    return std::shared_ptr<FunctionRunner>(new FunctionRunner(func, arg));
  }

  template <class F>
  static std::shared_ptr<FunctionRunner> create(F&& cob, int intervalMs) {
    return std::shared_ptr<FunctionRunner>(
        new FunctionRunner(std::forward<F>(cob), intervalMs));
  }

  /**
   * Given a 'pthread_create' style callback, this FunctionRunner will
   * execute the given callback.  Note that the 'void*' return value is ignored.
   */
  FunctionRunner(PthreadFuncPtr func, void* arg)
      : func_([=] { func(arg); }), repFunc_(), initFunc_() {}

  /**
   * Given a generic callback, this FunctionRunner will execute it.
   */
  template <class F>
  explicit FunctionRunner(F&& cob)
      : func_(std::forward<F>(cob)), repFunc_(), initFunc_() {}

  /**
   * Given a bool foo(...) type callback, FunctionRunner will execute
   * the callback repeatedly with 'intervalMs' milliseconds between the calls,
   * until it returns false. Note that the actual interval between calls will
   * be intervalMs plus execution time of the callback.
   */
  template <class F>
  FunctionRunner(F&& cob, int intervalMs)
      : func_(),
        repFunc_(std::forward<F>(cob)),
        intervalMs_(intervalMs),
        initFunc_() {
    if (intervalMs_ < 0) {
      throw InvalidArgumentException();
    }
  }

  /**
   * Set a callback to be called when the thread is started.
   */
  void setInitFunc(VoidFunc&& initFunc) { initFunc_ = std::move(initFunc); }

  void run() override {
    if (initFunc_) {
      std::unique_lock<std::mutex> l(mutex_);
      if (initFunc_) {
        initFunc_();
      }
    }
    if (intervalMs_ != -1) {
      auto g = folly::makeGuard([&] { repFinished_.post(); });
      std::unique_lock<std::mutex> l(mutex_);
      while (repFunc_ && repFunc_()) {
        // this wait could time out (normal interval-"sleep" case),
        // or the monitor_ could have been notify()'ed by stop method.
        cond_.wait_for(l, std::chrono::milliseconds(intervalMs_));
      }
    } else {
      func_();
    }
  }

  void stop() { stopRep(); }

  ~FunctionRunner() override {
    if (stopRep()) {
      repFinished_.wait();
    }
  }

 private:
  bool stopRep() {
    std::lock_guard<std::mutex> g(mutex_);
    if (!repFunc_) {
      return false;
    }
    repFunc_ = nullptr;
    cond_.notify_one();
    return true;
  }

  VoidFunc func_;
  BoolFunc repFunc_;
  const int intervalMs_{-1}; // -1 iff invalid (no periodic function)
  VoidFunc initFunc_;
  std::mutex mutex_;
  std::condition_variable cond_;
  folly::Baton<> repFinished_;
};

} // namespace concurrency
} // namespace thrift
} // namespace apache

#endif // #ifndef _THRIFT_CONCURRENCY_FUNCTION_RUNNER_H
