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

#include <thrift/lib/cpp2/async/ClientSinkBridge.h>
#include <thrift/lib/cpp2/async/ClientStreamBridge.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>

namespace apache::thrift::detail {

class ClientCallbackStapler : public BiDiClientCallback,
                              public SinkServerCallback,
                              public StreamServerCallback {
  struct DeletionGuard {
    explicit DeletionGuard(ClientCallbackStapler* self) : self_(self) {
      self_->depth_++;
    }
    DeletionGuard(const DeletionGuard&) = delete;
    DeletionGuard& operator=(const DeletionGuard&) = delete;
    DeletionGuard(DeletionGuard&&) = delete;
    DeletionGuard& operator=(DeletionGuard&&) = delete;
    ~DeletionGuard() {
      self_->depth_--;
      if (self_->depth_ == 0 && !self_->sink_ && !self_->stream_) {
        delete self_;
      }
    }
    ClientCallbackStapler* self_;
  };

 public:
  ClientCallbackStapler(SinkClientCallback* sink, StreamClientCallback* stream)
      : sink_(sink), stream_(stream) {}

  /* BiDi client methods: return whether whole contract is still alive. */

  // Calls sink onFirstResponse with dummy payload and stream onFirstResponse
  // with real payload.
  bool onFirstResponse(
      FirstResponsePayload&& payload,
      folly::EventBase* evb,
      BiDiServerCallback* cb) override {
    DeletionGuard guard(this);
    serverCb_ = cb;
    std::ignore =
        sink_->onFirstResponse(FirstResponsePayload{nullptr, {}}, evb, this);
    std::ignore = stream_->onFirstResponse(std::move(payload), evb, this);
    return sink_ || stream_;
  }
  void onFirstResponseError(folly::exception_wrapper ew) override {
    DeletionGuard guard(this);
    std::exchange(sink_, nullptr)->onFirstResponseError(ew);
    std::exchange(stream_, nullptr)->onFirstResponseError(ew);
  }

  bool onStreamNext(StreamPayload&& payload) override {
    DeletionGuard guard(this);
    std::ignore = stream_->onStreamNext(std::move(payload));
    return stream_ || sink_;
  }

  bool onStreamError(folly::exception_wrapper ew) override {
    DeletionGuard guard(this);
    std::exchange(stream_, nullptr)->onStreamError(std::move(ew));
    return stream_ || sink_;
  }

  bool onStreamComplete() override {
    DeletionGuard guard(this);
    std::exchange(stream_, nullptr)->onStreamComplete();
    return stream_ || sink_;
  }

  bool onSinkRequestN(uint64_t tokens) override {
    DeletionGuard guard(this);
    std::ignore = sink_->onSinkRequestN(tokens);
    return stream_ || sink_;
  }

  bool onSinkCancel() override {
    DeletionGuard guard(this);
    std::exchange(sink_, nullptr)->onFinalResponse(StreamPayload{nullptr, {}});
    return stream_ || sink_;
  }

  void resetServerCallback(BiDiServerCallback& serverCb) override {
    serverCb_ = &serverCb;
  }

  /* Stream/sink server methods: return whether that half-contract is alive. */

  bool onStreamRequestN(uint64_t n) override {
    DeletionGuard guard(this);
    std::ignore = serverCb_->onStreamRequestN(n);
    return stream_;
  }
  void onStreamCancel() override {
    DeletionGuard guard(this);
    stream_ = nullptr;
    std::ignore = serverCb_->onStreamCancel();
  }

  bool onSinkNext(StreamPayload&& payload) override {
    DeletionGuard guard(this);
    std::ignore = serverCb_->onSinkNext(std::move(payload));
    return sink_;
  }
  void onSinkError(folly::exception_wrapper ew) override {
    DeletionGuard guard(this);
    sink_ = nullptr;
    std::ignore = serverCb_->onSinkError(std::move(ew));
  }
  bool onSinkComplete() override {
    DeletionGuard guard(this);
    std::exchange(sink_, nullptr)->onFinalResponse(StreamPayload{nullptr, {}});
    std::ignore = serverCb_->onSinkComplete();
    return false;
  }

  void resetClientCallback(SinkClientCallback& sink) override { sink_ = &sink; }
  void resetClientCallback(StreamClientCallback& stream) override {
    stream_ = &stream;
  }

 private:
  BiDiServerCallback* serverCb_{};
  size_t depth_{0};
  SinkClientCallback* sink_{nullptr};
  StreamClientCallback* stream_{nullptr};

  friend struct DeletionGuard;
};

// There is no ClientBiDiBridge, instead we staple sink/stream bridges together
// and pretend there is.
// This stapled object is the client side to the transport and the server side
// to the two component bridges.
class ClientBiDiBridge
    : FirstResponseClientCallback<ClientSinkBridge::ClientPtr>,
      FirstResponseClientCallback<ClientStreamBridge::ClientPtr>,
      BiDiClientCallback {
 public:
  static BiDiClientCallback* create(
      FirstResponseClientCallback<ClientBridgePtrPair>* cb) {
    auto ret = new ClientBiDiBridge;
    ret->cb_ = cb;
    return ret;
  }

  // 1. Transport code calls this.
  bool onFirstResponse(
      FirstResponsePayload&& payload,
      folly::EventBase* evb,
      BiDiServerCallback* serverCallback) override {
    auto sinkClient = ClientSinkBridge::create(this);
    auto streamClient = ClientStreamBridge::create(this);
    auto stapled = new ClientCallbackStapler(sinkClient, streamClient);
    serverCallback->resetClientCallback(*stapled);
    return stapled->onFirstResponse(std::move(payload), evb, serverCallback);
  }
  void onFirstResponseError(folly::exception_wrapper ew) override {
    cb_->onFirstResponseError(std::move(ew));
    delete this;
  }

  // 2. stapled->onFirstResponse calls this above with a dummy payload.
  void onFirstResponse(
      FirstResponsePayload&&, ClientSinkBridge::ClientPtr bridge) override {
    pair_.sink = std::move(bridge);
    DCHECK(!pair_.stream);
  }

  // 3. stapled->onFirstResponse calls this above with the real payload and we
  // forward both bridges to the client.
  void onFirstResponse(
      FirstResponsePayload&& payload,
      ClientStreamBridge::ClientPtr bridge) override {
    pair_.stream = std::move(bridge);
    DCHECK(pair_.sink);
    cb_->onFirstResponse(std::move(payload), std::move(pair_));
    delete this;
  }

  bool onStreamNext(StreamPayload&&) override { std::terminate(); }
  bool onStreamError(folly::exception_wrapper) override { std::terminate(); }
  bool onStreamComplete() override { std::terminate(); }

  bool onSinkRequestN(uint64_t) override { std::terminate(); }
  bool onSinkCancel() override { std::terminate(); }

  void resetServerCallback(BiDiServerCallback&) override { std::terminate(); }

 private:
  FirstResponseClientCallback<ClientBridgePtrPair>* cb_{};
  ClientBridgePtrPair pair_;
};

} // namespace apache::thrift::detail
