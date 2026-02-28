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

#include <folly/Overload.h>
#include <folly/Portability.h>
#include <folly/Try.h>
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/Task.h>
#include <thrift/lib/cpp/ContextStack.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/StreamMessage.h>
#include <thrift/lib/cpp2/async/TwoWayBridge.h>
#include <thrift/lib/cpp2/async/TwoWayBridgeUtil.h>

namespace apache::thrift::detail {

// Server to Client: PayloadOrError (payload/error) or Complete (stream done)
using ServerBiDiStreamMessageServerToClient =
    std::variant<StreamMessage::PayloadOrError, StreamMessage::Complete>;

// Client to Server: RequestN (credits) or Cancel
using ServerBiDiStreamMessageClientToServer =
    std::variant<StreamMessage::RequestN, StreamMessage::Cancel>;

class ServerBiDiStreamBridge : public TwoWayBridge<
                                   ServerBiDiStreamBridge,
                                   ServerBiDiStreamMessageServerToClient,
                                   CoroConsumer,
                                   ServerBiDiStreamMessageClientToServer,
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
  bool onStreamRequestN(int32_t credits) override {
    clientPush(StreamMessage::RequestN{credits});
    return true;
  }

  void onStreamCancel() override {
    clientPush(StreamMessage::Cancel{});
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

  template <typename Out>
  static folly::coro::Task<> getTask(
      ServerBiDiStreamBridge::Ptr bridge,
      folly::coro::AsyncGenerator<Out&&> input,
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

        auto& message = messages.front();
        bool cancelled = folly::variant_match(
            message,
            [&](StreamMessage::RequestN& requestN) {
              notifyBiDiStreamCredit(bridge->contextStack_.get(), requestN.n);
              credits += requestN.n;
              return false;
            },
            [](StreamMessage::Cancel) { return true; });
        if (cancelled) {
          co_return;
        }
      }

      if (credits == 0) {
        notifyBiDiStreamPause(
            bridge->contextStack_.get(),
            details::STREAM_PAUSE_REASON::NO_CREDITS);
        continue;
      }

      auto next = co_await folly::coro::co_awaitTry(input.next());
      if (next.hasException()) {
        handleBiDiStreamError(bridge->contextStack_.get(), next.exception());
        bridge->serverPush(
            StreamMessage::PayloadOrError{
                (*encode)(std::move(next.exception()))});
        co_return;
      }
      if (!next->has_value()) {
        // Empty means stream completion
        bridge->serverPush(StreamMessage::Complete{});
        co_return;
      }

      bridge->serverPush(StreamMessage::PayloadOrError{(*encode)(**next)});

      notifyBiDiStreamNext(bridge->contextStack_.get());

      credits--;
      if (credits == 0) {
        notifyBiDiStreamPause(
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

        auto& message = messages.front();
        bool terminated = folly::variant_match(
            message,
            [&](StreamMessage::PayloadOrError& payloadOrError) {
              auto& payload = payloadOrError.streamPayloadTry;
              if (payload.hasValue()) {
                // payload
                return !clientCb_->onStreamNext(std::move(payload).value());
              } else {
                // exception
                clientCb_->onStreamError(std::move(payload).exception());
                decref();
                return true;
              }
            },
            [&](StreamMessage::Complete) {
              clientCb_->onStreamComplete();
              decref();
              return true;
            });

        if (terminated) {
          return;
        }
      }
    }
  }

  static void notifyBiDiStreamNext(ContextStack* ctx) {
    if (ctx) {
      ctx->onBiDiStreamNext();
    }
  }

  static void notifyBiDiStreamCredit(ContextStack* ctx, int64_t credits) {
    if (ctx && credits > 0) {
      ctx->onBiDiStreamCredit(static_cast<uint32_t>(credits));
    }
  }

  static void notifyBiDiStreamPause(
      ContextStack* ctx, details::STREAM_PAUSE_REASON reason) {
    if (ctx) {
      ctx->onBiDiStreamPause(reason);
    }
  }

  static void handleBiDiStreamError(
      ContextStack* ctx, const folly::exception_wrapper& ew) {
    if (ctx) {
      ctx->handleBiDiStreamError(ew);
    }
  }

 private:
  StreamClientCallback* clientCb_{nullptr};
  folly::EventBase* evb_{nullptr};
  std::shared_ptr<ContextStack> contextStack_;
};

} // namespace apache::thrift::detail
