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
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/Baton.h>
#include <folly/coro/Task.h>

#include <thrift/lib/cpp2/async/TwoWayBridgeUtil.h>
#endif
#include <thrift/lib/cpp/ContextStack.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/StreamMessage.h>
#include <thrift/lib/cpp2/async/TwoWayBridge.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>

namespace apache::thrift::detail {

struct SinkConsumerImpl {
#if FOLLY_HAS_COROUTINES
  using Consumer = folly::Function<folly::coro::Task<folly::Try<StreamPayload>>(
      folly::coro::AsyncGenerator<folly::Try<StreamPayload>&&>)>;
  Consumer consumer;
  uint64_t bufferSize{};
  std::chrono::milliseconds chunkTimeout{};
  folly::Executor::KeepAlive<> executor;
  TilePtr interaction{};
  ContextStack::UniquePtr contextStack = nullptr;
  explicit operator bool() const { return (bool)consumer; }
#endif
};

class ServerSinkFactory;

#if FOLLY_HAS_COROUTINES

using ServerSinkMessageServerToClient =
    std::variant<StreamMessage::PayloadOrError, StreamMessage::RequestN>;

using ServerSinkMessageClientToServer =
    std::variant<StreamMessage::PayloadOrError, StreamMessage::Complete>;

class ServerSinkBridge;

// This template explicitly instantiated in ServerSinkBridge.cpp
extern template class TwoWayBridge<
    ServerSinkBridge,
    ServerSinkMessageServerToClient,
    QueueConsumer,
    ServerSinkMessageClientToServer,
    ServerSinkBridge>;

class ServerSinkBridge : public TwoWayBridge<
                             ServerSinkBridge,
                             ServerSinkMessageServerToClient,
                             QueueConsumer,
                             ServerSinkMessageClientToServer,
                             ServerSinkBridge>,
                         public SinkServerCallback {
 public:
  ~ServerSinkBridge() override;

  static Ptr create(
      SinkConsumerImpl&& sinkConsumer,
      folly::EventBase& evb,
      SinkClientCallback* callback);

  // start() should be called on the CPU thread
  folly::coro::Task<void> start();

  uint64_t getBufferSize() const { return consumer_.bufferSize; }

  //
  // SinkServerCallback method
  //
  bool onSinkNext(StreamPayload&& payload) override;

  void onSinkError(folly::exception_wrapper ew) override;

  bool onSinkComplete() override;

  void resetClientCallback(SinkClientCallback& clientCallback) override;
  //
  // end of SinkServerCallback method
  //

  //
  // TwoWayBridge methods
  //
  void consume();

  void canceled() {}
  //
  // end of TwoWayBridge methods
  //

  using TwoWayBridge::serverClose;
  using TwoWayBridge::serverGetMessages;
  using TwoWayBridge::serverPush;
  using TwoWayBridge::serverWait;

 private:
  ServerSinkBridge(
      SinkConsumerImpl&& sinkConsumer,
      folly::EventBase& evb,
      SinkClientCallback* callback);

  folly::coro::AsyncGenerator<folly::Try<StreamPayload>&&> makeGenerator();
  void processClientMessages();
  void close();

  // Helper methods to encapsulate ContextStack usage
  void notifySinkSubscribe();
  void notifySinkNext();
  void notifySinkFinally(details::SINK_ENDING_TYPES endingType);
  void notifySinkError(
      const folly::exception_wrapper& exception,
      details::SINK_ENDING_TYPES endingType =
          details::SINK_ENDING_TYPES::ERROR);
  void notifySinkCredit(uint64_t credits);
  void notifySinkConsumed();
  void notifySinkCancel();

  SinkConsumerImpl consumer_;
  folly::Executor::KeepAlive<folly::EventBase> evb_;
  SinkClientCallback* clientCallback_;

  // only access on the CPU thread
  bool clientException_{false};

  // only access on the IO thread
  bool sinkComplete_{false};

  TileStreamGuard interaction_;

  friend ServerSinkFactory;
};

#endif

} // namespace apache::thrift::detail
