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

#include <variant>

#include <folly/CancellationToken.h>
#include <folly/Overload.h>
#include <folly/Portability.h>
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/Baton.h>
#include <folly/coro/Task.h>

#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/StreamMessage.h>
#include <thrift/lib/cpp2/async/TwoWayBridge.h>
#include <thrift/lib/cpp2/async/TwoWayBridgeUtil.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>

namespace apache::thrift::detail {

class ClientSinkBridge;

// Server to Client: PayloadOrError (final response/error) or RequestN (credits)
using ClientSinkMessageServerToClient =
    std::variant<StreamMessage::PayloadOrError, StreamMessage::RequestN>;

// Client to Server: PayloadOrError (payload/error) or Complete (stream done)
using ClientSinkMessageClientToServer =
    std::variant<StreamMessage::PayloadOrError, StreamMessage::Complete>;

// Instantiated in ClientSinkBridge.cpp
extern template class TwoWayBridge<
    QueueConsumer,
    ClientSinkMessageServerToClient,
    ClientSinkBridge,
    ClientSinkMessageClientToServer,
    ClientSinkBridge>;

class ClientSinkBridge : public TwoWayBridge<
                             QueueConsumer,
                             ClientSinkMessageServerToClient,
                             ClientSinkBridge,
                             ClientSinkMessageClientToServer,
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

  bool wait(QueueConsumer* consumer);

  void push(folly::Try<StreamPayload>&& value);

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

  bool onSinkRequestN(int32_t n) override;

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

} // namespace apache::thrift::detail
