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

#include <thrift/lib/cpp/ContextStack.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>

namespace apache::thrift::detail {

class ServerCallbackStapler : public BiDiServerCallback,
                              public SinkClientCallback,
                              public StreamClientCallback {
  struct DeletionGuard {
    explicit DeletionGuard(ServerCallbackStapler* self) : self_(self) {
      self_->depth_++;
    }
    ~DeletionGuard() {
      self_->depth_--;
      if (self_->depth_ == 0 && !self_->sink_ && !self_->stream_) {
        delete self_;
      }
    }
    ServerCallbackStapler* self_;

    DeletionGuard(const DeletionGuard&) = delete;
    DeletionGuard& operator=(const DeletionGuard&) = delete;
    DeletionGuard(DeletionGuard&&) noexcept = delete;
    DeletionGuard& operator=(DeletionGuard&&) noexcept = delete;
  };

 public:
  ServerCallbackStapler() : sink_(nullptr), stream_(nullptr) {}

  ~ServerCallbackStapler() override {
    if (contextStack_) {
      if (hasError_) {
        contextStack_->onBiDiFinally(details::BIDI_FINISH_REASON::ERROR);
      } else if (hasCancellation_) {
        contextStack_->onBiDiFinally(details::BIDI_FINISH_REASON::CANCEL);
      } else {
        contextStack_->onBiDiFinally(details::BIDI_FINISH_REASON::COMPLETE);
      }
    }
  }

  void setSinkServerCallback(SinkServerCallback* sink) { sink_ = sink; }
  void setStreamServerCallback(StreamServerCallback* stream) {
    stream_ = stream;
  }
  void setContextStack(std::shared_ptr<ContextStack> contextStack) {
    contextStack_ = std::move(contextStack);
  }

  //
  // BiDiServerCallback methods
  //

  bool onStreamRequestN(uint64_t n) override {
    DeletionGuard guard(this);
    std::ignore = stream_->onStreamRequestN(n);
    return sink_ || stream_;
  }

  bool onStreamCancel() override {
    DeletionGuard guard(this);
    hasCancellation_ = true;
    std::exchange(stream_, nullptr)->onStreamCancel();
    return sink_ || stream_;
  }

  bool onSinkNext(StreamPayload&& payload) override {
    DeletionGuard guard(this);
    std::ignore = sink_->onSinkNext(std::move(payload));
    return sink_ || stream_;
  }

  bool onSinkError(folly::exception_wrapper ew) override {
    DeletionGuard guard(this);
    hasError_ = true;
    std::exchange(sink_, nullptr)->onSinkError(std::move(ew));
    return sink_ || stream_;
  }

  bool onSinkComplete() override {
    DeletionGuard guard(this);
    std::ignore = std::exchange(sink_, nullptr)->onSinkComplete();
    return sink_ || stream_;
  }

  void resetClientCallback(BiDiClientCallback& clientCb) override {
    clientCb_ = &clientCb;
  }

  //
  // end of BiDiServerCallback methods
  //

  //
  // SinkClientCallback methods
  //
  bool onFirstResponse(
      FirstResponsePayload&& payload,
      folly::EventBase* eb,
      SinkServerCallback*) override {
    DeletionGuard guard(this);
    std::ignore = clientCb_->onFirstResponse(std::move(payload), eb, this);
    return sink_;
  }

  void onFirstResponseError(folly::exception_wrapper ew) override {
    DeletionGuard guard(this);
    sink_ = nullptr;
    clientCb_->onFirstResponseError(std::move(ew));
  }

  void onFinalResponse(StreamPayload&&) override {
    DeletionGuard guard(this);
    std::ignore = clientCb_->onSinkCancel();
    sink_ = nullptr;
  }

  void onFinalResponseError(folly::exception_wrapper) override {
    DeletionGuard guard(this);
    hasError_ = true;
    sink_ = nullptr;
  }

  bool onSinkRequestN(uint64_t n) override {
    DeletionGuard guard(this);
    std::ignore = clientCb_->onSinkRequestN(n);
    return sink_;
  }

  void resetServerCallback(SinkServerCallback& sink) override { sink_ = &sink; }

  //
  // end of SinkClientCallback methods
  //

  //
  // StreamClientCallback methods
  //

  bool onFirstResponse(
      FirstResponsePayload&& payload,
      folly::EventBase* eb,
      StreamServerCallback*) override {
    DeletionGuard guard(this);
    std::ignore = clientCb_->onFirstResponse(std::move(payload), eb, this);
    return stream_;
  }

  bool onStreamNext(StreamPayload&& payload) override {
    DeletionGuard guard(this);
    std::ignore = clientCb_->onStreamNext(std::move(payload));
    return stream_;
  }

  void onStreamError(folly::exception_wrapper ew) override {
    DeletionGuard guard(this);
    hasError_ = true;
    stream_ = nullptr;
    std::ignore = clientCb_->onStreamError(std::move(ew));
  }

  void onStreamComplete() override {
    DeletionGuard guard(this);
    stream_ = nullptr;
    std::ignore = clientCb_->onStreamComplete();
  }

  void resetServerCallback(StreamServerCallback& stream) override {
    stream_ = &stream;
  }

  //
  // end of StreamClientCallback methods
  //

 private:
  BiDiClientCallback* clientCb_{};
  size_t depth_{0};

  SinkServerCallback* sink_{nullptr};
  StreamServerCallback* stream_{nullptr};
  std::shared_ptr<ContextStack> contextStack_;

  bool hasError_{false};
  bool hasCancellation_{false};
};

} // namespace apache::thrift::detail
