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

#include <folly/CancellationToken.h>
#include <folly/Portability.h>
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/Baton.h>
#include <folly/coro/Task.h>

#include <thrift/lib/cpp2/async/SinkBridgeUtil.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/TwoWayBridge.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>

namespace apache {
namespace thrift {
namespace detail {

class ClientSinkBridge;

// Instantiated in ClientSinkBridge.cpp
extern template class TwoWayBridge<
    ClientSinkConsumer,
    ClientMessage,
    ClientSinkBridge,
    ServerMessage,
    ClientSinkBridge>;

class ClientSinkBridge : public TwoWayBridge<
                             ClientSinkConsumer,
                             ClientMessage,
                             ClientSinkBridge,
                             ServerMessage,
                             ClientSinkBridge>,
                         public SinkClientCallback {
 public:
  struct ClientDeleter : Deleter {
    void operator()(ClientSinkBridge* ptr);
  };
  using ClientPtr = std::unique_ptr<ClientSinkBridge, ClientDeleter>;

  ~ClientSinkBridge() override;

  using FirstResponseCallback = FirstResponseClientCallback<ClientPtr>;

  static SinkClientCallback* create(FirstResponseCallback* callback);

  void close();

  bool wait(ClientSinkConsumer* consumer);

  void push(ServerMessage&& value);

  ClientQueue getMessages();

  // User (client) thread must call exactly one of sink and cancel

#if FOLLY_HAS_COROUTINES
  folly::coro::Task<folly::Try<StreamPayload>> sink(
      folly::coro::AsyncGenerator<folly::Try<StreamPayload>&&> generator);
#endif

  void cancel(folly::Try<StreamPayload> payload);

  // SinkClientCallback method
  bool onFirstResponse(
      FirstResponsePayload&& firstPayload,
      folly::EventBase* evb,
      SinkServerCallback* serverCallback) override;

  void onFirstResponseError(folly::exception_wrapper ew) override;

  void onFinalResponse(StreamPayload&& payload) override;

  void onFinalResponseError(folly::exception_wrapper ew) override;

  bool onSinkRequestN(uint64_t n) override;

  void resetServerCallback(SinkServerCallback& serverCallback) override;

  void consume();

  bool hasServerCancelled();

  void canceled() {}

 private:
  explicit ClientSinkBridge(FirstResponseCallback* callback);

  void processServerMessages();

  union {
    FirstResponseCallback* firstResponseCallback_;
    SinkServerCallback* serverCallback_;
  };
  // Only accessed on client thread after onFirstResponse
  folly::Executor::KeepAlive<folly::EventBase> evb_;
  folly::CancellationSource serverCancelSource_;
};

} // namespace detail
} // namespace thrift
} // namespace apache
