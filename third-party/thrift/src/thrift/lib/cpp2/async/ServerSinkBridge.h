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

#include <folly/Function.h>
#include <folly/Portability.h>

#if FOLLY_HAS_COROUTINES
#include <folly/experimental/coro/AsyncGenerator.h>
#include <folly/experimental/coro/Baton.h>
#include <folly/experimental/coro/Task.h>

#include <thrift/lib/cpp2/async/SinkBridgeUtil.h>
#endif
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/TwoWayBridge.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>

namespace apache {
namespace thrift {
namespace detail {

struct SinkConsumerImpl {
#if FOLLY_HAS_COROUTINES
  using Consumer = folly::Function<folly::coro::Task<folly::Try<StreamPayload>>(
      folly::coro::AsyncGenerator<folly::Try<StreamPayload>&&>)>;
  Consumer consumer;
  uint64_t bufferSize;
  std::chrono::milliseconds chunkTimeout;
  folly::Executor::KeepAlive<> executor;
  TilePtr interaction{};

  explicit operator bool() const { return (bool)consumer; }
#endif
};

#if FOLLY_HAS_COROUTINES
class ServerSinkBridge;

// This template explicitly instantiated in ServerSinkBridge.cpp
extern template class TwoWayBridge<
    ServerSinkBridge,
    ClientMessage,
    CoroConsumer,
    ServerMessage,
    ServerSinkBridge>;

class ServerSinkBridge : public TwoWayBridge<
                             ServerSinkBridge,
                             ClientMessage,
                             CoroConsumer,
                             ServerMessage,
                             ServerSinkBridge>,
                         public SinkServerCallback {
 public:
  ~ServerSinkBridge() override;

  static Ptr create(
      SinkConsumerImpl&& sinkConsumer,
      folly::EventBase& evb,
      SinkClientCallback* callback);

  // SinkServerCallback method
  bool onSinkNext(StreamPayload&& payload) override;

  void onSinkError(folly::exception_wrapper ew) override;

  bool onSinkComplete() override;

  void resetClientCallback(SinkClientCallback& clientCallback) override;

  // start should be called on threadmanager's thread
  folly::coro::Task<void> start();

  // TwoWayBridge consumer
  void consume();

  void canceled() {}

 private:
  ServerSinkBridge(
      SinkConsumerImpl&& sinkConsumer,
      folly::EventBase& evb,
      SinkClientCallback* callback);

  folly::coro::AsyncGenerator<folly::Try<StreamPayload>&&> makeGenerator();

  void processClientMessages();

  void close();

  SinkConsumerImpl consumer_;
  folly::Executor::KeepAlive<folly::EventBase> evb_;
  SinkClientCallback* clientCallback_;

  // only access in threadManager thread
  bool clientException_{false};

  // only access in evb_ thread
  bool sinkComplete_{false};
  TileStreamGuard interaction_;
};
#endif

} // namespace detail
} // namespace thrift
} // namespace apache
