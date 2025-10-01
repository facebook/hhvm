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

#include <folly/Portability.h>
#include <folly/Try.h>
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/Task.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/TwoWayBridge.h>
#include <thrift/lib/cpp2/async/TwoWayBridgeUtil.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>

namespace apache::thrift::detail {

class ServerBiDiSinkBridge
    : public TwoWayBridge<
          ServerBiDiSinkBridge,
          uint64_t, // we send credits to the client
          CoroConsumer,
          folly::Try<StreamPayload>, // we receive payloads from the client
          ServerBiDiSinkBridge>,
      public SinkServerCallback {
 public:
  ServerBiDiSinkBridge(SinkClientCallback* clientCb, folly::EventBase* evb)
      : clientCb_(clientCb), evb_(evb) {}

  //
  // SinkServerCallback
  //
  bool onSinkNext(StreamPayload&& payload) override {
    clientPush(folly::Try<StreamPayload>(std::move(payload)));
    return true;
  }

  void onSinkError(folly::exception_wrapper ew) override {
    using apache::thrift::detail::EncodedError;
    auto rex = ew.get_exception<rocket::RocketException>();
    auto payload = rex
        ? folly::Try<StreamPayload>(EncodedError(rex->moveErrorData()))
        : folly::Try<StreamPayload>(std::move(ew));
    clientPush(std::move(payload));
    clientClose();
  }

  bool onSinkComplete() override {
    clientPush(/* empty Try */ {});
    clientClose();
    return true;
  }

  void resetClientCallback(SinkClientCallback& clientCb) override {
    clientCb_ = &clientCb;
  }

  //
  // end of SinkServerCallback
  //
  //
  // TwoWayBridge methods
  //
  void consume() {
    evb_->runInEventBaseThread([this] { processClientMessages(); });
  }

  void canceled() { decref(); }
  //
  // end of TwoWayBridge methods
  //

  template <typename In>
  static folly::coro::AsyncGenerator<In&&> getInput(
      ServerBiDiSinkBridge::Ptr bridge, SinkElementDecoder<In>* decode) {
    SCOPE_EXIT {
      bridge->serverClose();
    };

    uint64_t creditsUsed{0};
    while (true) {
      CoroConsumer c;
      if (bridge->serverWait(&c)) {
        co_await c.wait();
      }

      for (auto messages = bridge->serverGetMessages(); !messages.empty();
           messages.pop()) {
        auto payloadTry = std::move(messages.front());
        if (!payloadTry.hasValue() && !payloadTry.hasException()) {
          // Empty Try means sink completion
          co_return;
        }

        if (payloadTry.hasException()) {
          // TODO(sazonovk): Is this the right way to handle errors? NO
          payloadTry.exception().throw_exception();
        }

        auto decodedTry = (*decode)(std::move(payloadTry));
        if (decodedTry.hasException()) {
          // TODO(sazonovk): Is this the right way to handle errors? NO
          decodedTry.exception().throw_exception();
        }

        co_yield std::move(decodedTry.value());

        if (++creditsUsed > bridge->bufferSize_ / 2) {
          bridge->serverPush(uint64_t(creditsUsed));
          creditsUsed = 0;
        }
      }
    }
  }

  void processClientMessages() {
    evb_->dcheckIsInEventBaseThread();

    do {
      for (auto messages = clientGetMessages(); !messages.empty();
           messages.pop()) {
        DCHECK(!isClientClosed());

        auto message = std::move(messages.front());
        if (!clientCb_->onSinkRequestN(message)) {
          return;
        }
      }
    } while (!clientWait(this));
  }

  void setBufferSize(uint64_t bufferSize) { bufferSize_ = bufferSize; }

  using TwoWayBridge::serverPush;

 private:
  SinkClientCallback* clientCb_{nullptr};
  folly::EventBase* evb_{nullptr};

  uint64_t bufferSize_{0};
};

} // namespace apache::thrift::detail
