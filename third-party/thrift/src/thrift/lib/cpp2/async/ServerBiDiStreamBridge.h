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

namespace apache::thrift::detail {

class ServerBiDiStreamBridge
    : public TwoWayBridge<
          ServerBiDiStreamBridge,
          folly::Try<StreamPayload>, // we send stream chunks to the client
          CoroConsumer,
          int64_t, // we receive credits from the client (or cancellation)
          ServerBiDiStreamBridge>,
      public StreamServerCallback {
 public:
  ServerBiDiStreamBridge(StreamClientCallback* clientCb, folly::EventBase* evb)
      : clientCb_(clientCb), evb_(evb) {}

  //
  // StreamServerCallback methods
  //
  bool onStreamRequestN(uint64_t credits) override {
    clientPush(credits);
    return true;
  }

  void onStreamCancel() override {
    clientPush(CANCEL);
    clientClose();
  }

  void resetClientCallback(StreamClientCallback& clientCb) override {
    clientCb_ = &clientCb;
  }
  //
  // end of StreamServerCallback methods
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

  template <typename In, typename Out>
  static folly::coro::Task<> getTask(
      ServerBiDiStreamBridge::Ptr bridge,
      folly::coro::AsyncGenerator<In> input,
      StreamElementEncoder<Out>* encode) {
    SCOPE_EXIT {
      bridge->serverClose();
    };

    int64_t credits{0};
    while (true) {
      if (credits == 0) {
        CoroConsumer c;
        if (bridge->serverWait(&c)) {
          co_await c.wait();
        }
      }

      for (auto messages = bridge->serverGetMessages(); !messages.empty();
           messages.pop()) {
        DCHECK(!bridge->isServerClosed());

        auto message = std::move(messages.front());
        if (message == CANCEL) {
          co_return;
        }
        credits += message;
      }

      if (credits == 0) {
        continue;
      }

      auto next = co_await folly::coro::co_awaitTry(input.next());
      if (next.hasException()) {
        // Transformation threw an exception
        auto encoded = (*encode)(std::move(next.exception()));
        bridge->serverPush(std::move(encoded));
        co_return;
      }
      if (!next->has_value()) {
        // Empty Try means stream completion
        bridge->serverPush({});
        co_return;
      }

      bridge->serverPush((*encode)(**next));
      credits--;
    }
  }

  // Process messages in the client queue
  void processClientMessages() {
    evb_->dcheckIsInEventBaseThread();
    while (!clientWait(this)) {
      for (auto messages = clientGetMessages(); !messages.empty();
           messages.pop()) {
        DCHECK(!isClientClosed());

        auto message = std::move(messages.front());

        if (message.hasValue()) {
          // payload
          if (!clientCb_->onStreamNext(std::move(message.value()))) {
            break;
          }
        } else if (message.hasException()) {
          // exception
          clientCb_->onStreamError(std::move(message.exception()));
          decref();
          return;
        } else {
          // empty Try means stream completion
          clientCb_->onStreamComplete();
          decref();
          return;
        }
      }
    }
  }

 private:
  StreamClientCallback* clientCb_{nullptr};
  folly::EventBase* evb_{nullptr};

  enum StreamControl : int32_t {
    CANCEL = -1,
  };
};

} // namespace apache::thrift::detail
