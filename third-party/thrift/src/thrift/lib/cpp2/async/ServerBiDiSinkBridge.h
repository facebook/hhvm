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
  ServerBiDiSinkBridge(
      SinkClientCallback* clientCb,
      folly::EventBase* evb,
      std::shared_ptr<ContextStack> contextStack = nullptr)
      : clientCb_(clientCb),
        evb_(evb),
        contextStack_(std::move(contextStack)) {}

  //
  // SinkServerCallback
  //
  bool onSinkNext(StreamPayload&& payload) override {
    notifyBiDiSinkNext();
    clientPush(folly::Try<StreamPayload>(std::move(payload)));
    return true;
  }

  void onSinkError(folly::exception_wrapper ew) override {
    handleBiDiSinkError(ew);

    using apache::thrift::detail::EncodedError;
    auto rex = ew.get_mutable_exception<rocket::RocketException>();
    auto payload = rex
        ? folly::Try<StreamPayload>(EncodedError(rex->moveErrorData()))
        : folly::Try<StreamPayload>(std::move(ew));
    // We set this to null because the transport side must be aware of the
    // cancellation already
    clientCb_ = nullptr;
    clientPush(std::move(payload));
    clientClose();
  }

  bool onSinkComplete() override {
    // We set this to null because the transport side must be aware of the
    // cancellation already
    clientCb_ = nullptr;
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

  void canceled() {
    // Note that canceled() is actually called on the thread that destroys
    // the input AsyncGenerator, which generally shouldn't be IOThread.
    // The sending of cancellation and the decref MUST happen on the IOThread
    // (i.e EventBase) or you risk a data race. You'll get a TSAN failure if you
    // get rid of evb_->add here.
    evb_->add([this]() {
      // This cancels the sink if its open
      if (clientCb_ != nullptr) {
        clientCb_->onFinalResponse(
            StreamPayload{nullptr, StreamPayloadMetadata{}});
      }
      decref();
    });
  }
  //
  // end of TwoWayBridge methods
  //

  template <typename In>
  static folly::coro::AsyncGenerator<In&&> getInput(
      ServerBiDiSinkBridge::Ptr bridge, SinkElementDecoder<In>* decode) {
    /**
     * Cleanup is the destructor guard for the input async generator.
     * SCOPE_EXIT in the body of the async generator is insufficient since it
     * would never run if the generator is never co_awaited.
     */
    struct Cleanup {
      explicit Cleanup(ServerBiDiSinkBridge::Ptr bridge)
          : bridge{std::move(bridge)} {}
      ~Cleanup() {
        if (bridge != nullptr) {
          bridge->serverClose();
          // If the client is already closed, then the ClientCallback must
          // already be aware that the server sink side is complete so we
          // don't need to do anything. Otherwise, we need to close the client
          if (!bridge->isClientClosed()) {
            bridge->clientClose(); // Also cancels
          }
        }
      }
      Cleanup(const Cleanup&) = delete;
      Cleanup(Cleanup&& other) noexcept { bridge = std::move(other.bridge); }
      Cleanup& operator=(const Cleanup&) = delete;
      Cleanup& operator=(Cleanup&& other) noexcept {
        bridge = std::move(other.bridge);
        return *this;
      }

      ServerBiDiSinkBridge::Ptr bridge;
    };

    auto bridgePtr = bridge->copy();
    return folly::coro::co_invoke(
        [cleanup = Cleanup(std::move(bridgePtr))](
            ServerBiDiSinkBridge::Ptr bridge,
            SinkElementDecoder<In>* decode) mutable
            -> folly::coro::AsyncGenerator<In&&> {
          uint64_t creditsUsed{0};
          while (true) {
            CoroConsumer c;
            if (bridge->serverWait(&c)) {
              co_await c.wait();
            }

            for (auto messages = bridge->serverGetMessages(); !messages.empty();
                 messages.pop()) {
              // Empty try denotes end of sink
              if (!messages.front().hasException() &&
                  !messages.front().hasValue()) {
                co_return;
              }
              // This handled both both error and result cases. If the try
              // contains an exception, it will eventually result in yielding
              // a co_error
              co_yield folly::coro::co_result(
                  (*decode)(std::move(std::move(messages.front()))));

              if (++creditsUsed > bridge->bufferSize_ / 2) {
                notifyBiDiSinkCredit(bridge->contextStack_.get(), creditsUsed);
                bridge->serverPush(uint64_t(creditsUsed));
                creditsUsed = 0;
              }
            }
          }
        },
        std::move(bridge),
        decode);
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

  static void notifyBiDiSinkNext(ContextStack* ctx) {
    if (ctx) {
      ctx->onBiDiSinkNext();
    }
  }

  static void notifyBiDiSinkCredit(ContextStack* ctx, uint64_t credits) {
    if (ctx && credits > 0) {
      ctx->onBiDiSinkCredit(static_cast<uint32_t>(credits));
    }
  }

  static void handleBiDiSinkError(
      ContextStack* ctx, const folly::exception_wrapper& ew) {
    if (ctx) {
      ctx->handleBiDiSinkError(ew);
    }
  }

  // Instance wrapper methods
  void notifyBiDiSinkNext() { notifyBiDiSinkNext(contextStack_.get()); }

  void handleBiDiSinkError(const folly::exception_wrapper& ew) {
    handleBiDiSinkError(contextStack_.get(), ew);
  }

 private:
  // Note that we use the property of clientCb_ being null to indicate that
  // the transport side is already aware of the closure.
  SinkClientCallback* clientCb_{nullptr};
  folly::EventBase* evb_{nullptr};
  std::shared_ptr<ContextStack> contextStack_;

  uint64_t bufferSize_{0};
};

} // namespace apache::thrift::detail
