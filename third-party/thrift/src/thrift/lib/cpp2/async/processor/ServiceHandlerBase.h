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

#include <folly/concurrency/memory/PrimaryPtr.h>
#include <folly/coro/AsyncScope.h>
#include <folly/coro/Task.h>
#include <folly/futures/Future.h>
#include <thrift/lib/cpp2/server/ServiceMethodDecoratorBase.h>

namespace apache::thrift {
class ThriftServer;
class ThriftServerStopController;

class ServiceMethodDecoratorBase;

/**
 * Base-class for user-implemented service handlers. This serves as a channel
 * user code to be notified by ThriftServer and respond to events (via
 * callbacks).
 */
class ServiceHandlerBase {
 private:
#if FOLLY_HAS_COROUTINES
  class MethodNotImplemented : public std::logic_error {
   public:
    MethodNotImplemented() : std::logic_error("Method not implemented") {}
  };
#endif

 public:
  struct BeforeStartServingParams {
    server::DecoratorDataHandleFactory* decoratorDataHandleFactory = nullptr;
  };
#if FOLLY_HAS_COROUTINES
  /**
   * co_onBeforeStartServing is a lifecycle method guaranteed to be called
   * before the server starts accepting connections. It happens before
   * co_onStartServing() is called.
   */
  virtual folly::coro::Task<void> co_onBeforeStartServing(
      BeforeStartServingParams) {
    co_return;
  }

  virtual folly::coro::Task<void> co_onStartServing() { co_return; }
  virtual folly::coro::Task<void> co_onStopRequested() {
    throw MethodNotImplemented();
  }
#endif

  virtual folly::SemiFuture<folly::Unit> semifuture_onBeforeStartServing(
      BeforeStartServingParams params) {
#if FOLLY_HAS_COROUTINES
    return co_onBeforeStartServing(std::move(params)).semi();
#else
    return folly::makeSemiFuture();
#endif
  }
  virtual folly::SemiFuture<folly::Unit> semifuture_onStartServing() {
#if FOLLY_HAS_COROUTINES
    return co_onStartServing().semi();
#else
    return folly::makeSemiFuture();
#endif
  }

  virtual folly::SemiFuture<folly::Unit> semifuture_onStopRequested() {
#if FOLLY_HAS_COROUTINES
    // TODO(srir): onStopRequested should be implemented similar to
    // onStartServing
    try {
      return co_onStopRequested().semi();
    } catch (MethodNotImplemented&) {
      // If co_onStopRequested() is not implemented we just return
    }
#endif
    return folly::makeSemiFuture();
  }

  virtual std::vector<std::reference_wrapper<ServiceMethodDecoratorBase>>
  fbthrift_getDecorators() {
    return {};
  }

  ThriftServer* getServer() { return server_; }
  const ThriftServer* getServer() const { return server_; }
  void attachServer(ThriftServer& server);
  void detachServer();

  /**
   * Asynchronously begins shutting down the Thrift server this handler is
   * attached to.
   *
   * This function is idempotent for the duration of a server lifecycle -- so
   * it's safe to call multiple times (e.g. from folly::AsyncSignalHandler).
   */
  void shutdownServer();

  virtual ~ServiceHandlerBase() = default;

 protected:
#if FOLLY_HAS_COROUTINES
  folly::coro::CancellableAsyncScope* getAsyncScope();
#endif

 private:
  ThriftServer* server_{nullptr};
  folly::Synchronized<
      std::optional<folly::PrimaryPtrRef<ThriftServerStopController>>,
      std::mutex>
      serverStopController_;
};

} // namespace apache::thrift
