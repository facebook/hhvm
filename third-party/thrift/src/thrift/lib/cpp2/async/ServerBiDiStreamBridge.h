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
#include <thrift/lib/cpp/ContextStack.h>
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
  ServerBiDiStreamBridge(
      StreamClientCallback* clientCb,
      folly::EventBase* evb,
      std::shared_ptr<ContextStack> contextStack = nullptr)
      : clientCb_(clientCb),
        evb_(evb),
        contextStack_(std::move(contextStack)) {}

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
    // Notify output stream subscription
    notifyStreamSubscribe(bridge->contextStack_.get());

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
          // Notify cancellation
          notifyStreamFinally(
              bridge->contextStack_.get(),
              details::STREAM_ENDING_TYPES::CANCEL);
          co_return;
        }
        // Notify credits received from client
        notifyStreamCredit(bridge->contextStack_.get(), message);
        credits += message;
      }

      if (credits == 0) {
        // Notify pause due to no credits
        notifyStreamPause(
            bridge->contextStack_.get(),
            details::STREAM_PAUSE_REASON::NO_CREDITS);
        continue;
      }

      auto next = co_await folly::coro::co_awaitTry(input.next());
      if (next.hasException()) {
        // Transformation threw an exception
        // Notify error
        handleStreamErrorWrapped(bridge->contextStack_.get(), next.exception());
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

      // Notify item sent
      notifyStreamNext(bridge->contextStack_.get());
      notifyStreamNextSent(bridge->contextStack_.get());

      credits--;
      if (credits == 0) {
        // Notify pause
        notifyStreamPause(
            bridge->contextStack_.get(),
            details::STREAM_PAUSE_REASON::NO_CREDITS);
      }
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

  //
  // Helper methods to encapsulate ContextStack usage
  //
  static void notifyStreamSubscribe(ContextStack* ctx) {
    if (ctx) {
      StreamEventHandler::StreamContext streamCtx;
      ctx->onStreamSubscribe(streamCtx);
    }
  }

  static void notifyStreamNext(ContextStack* ctx) {
    if (ctx) {
      ctx->onStreamNext();
    }
  }

  static void notifyStreamNextSent(ContextStack* ctx) {
    if (ctx) {
      ctx->onStreamNextSent();
    }
  }

  static void notifyStreamCredit(ContextStack* ctx, int64_t credits) {
    if (ctx && credits > 0) {
      ctx->onStreamCredit(static_cast<uint32_t>(credits));
    }
  }

  static void notifyStreamPause(
      ContextStack* ctx, details::STREAM_PAUSE_REASON reason) {
    if (ctx) {
      ctx->onStreamPause(reason);
    }
  }

  static void handleStreamErrorWrapped(
      ContextStack* ctx, const folly::exception_wrapper& ew) {
    if (ctx) {
      ctx->handleStreamErrorWrapped(ew);
    }
  }

  static void notifyStreamFinally(
      ContextStack* ctx, details::STREAM_ENDING_TYPES endType) {
    if (ctx) {
      ctx->onStreamFinally(endType);
    }
  }
  //
  // end of Helper methods to encapsulate ContextStack usage
  //

 private:
  StreamClientCallback* clientCb_{nullptr};
  folly::EventBase* evb_{nullptr};
  std::shared_ptr<ContextStack> contextStack_;

  enum StreamControl : int32_t {
    CANCEL = -1,
  };
};

} // namespace apache::thrift::detail
