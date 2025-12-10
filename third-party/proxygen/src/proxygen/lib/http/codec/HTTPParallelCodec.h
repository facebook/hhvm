/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <bitset>
#include <deque>
#include <folly/Optional.h>
#include <proxygen/lib/http/HTTPHeaders.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/HTTPSettings.h>
#include <proxygen/lib/http/codec/compress/HeaderCodec.h>

namespace folly::io {
class Cursor;
} // namespace folly::io

namespace proxygen {

/**
 * An implementation of common codec functionality used for multiple
 * parallel stream downloads. Currently shared by SPDY and HTTP/2
 */
class HTTPParallelCodec : public HTTPCodec {
 public:
  explicit HTTPParallelCodec(TransportDirection direction);

  [[nodiscard]] TransportDirection getTransportDirection() const override {
    return transportDirection_;
  }

  StreamID createStream() override;
  [[nodiscard]] bool isBusy() const override {
    return false;
  }
  [[nodiscard]] bool supportsStreamFlowControl() const override {
    return true;
  }
  [[nodiscard]] bool supportsSessionFlowControl() const override {
    return true;
  }
  [[nodiscard]] bool supportsParallelRequests() const override {
    return true;
  }
  [[nodiscard]] bool closeOnEgressComplete() const override {
    return false;
  }
  void setCallback(Callback* callback) override {
    callback_ = callback;
  }
  void setParserPaused(bool /* paused */) override {
  }
  [[nodiscard]] bool isParserPaused() const override {
    return false;
  }
  void onIngressEOF() override {
  }
  [[nodiscard]] bool isReusable() const override;
  [[nodiscard]] bool isWaitingToDrain() const override;
  [[nodiscard]] StreamID getLastIncomingStreamID() const override {
    return lastStreamID_;
  }
  void enableDoubleGoawayDrain() override;
  void disableDoubleGoawayDrain() override;

  bool onIngressUpgradeMessage(const HTTPMessage& msg) override;

  void setNextEgressStreamId(StreamID nextEgressStreamID) {
    if (nextEgressStreamID > nextEgressStreamID_ &&
        (nextEgressStreamID & 0x1) == (nextEgressStreamID_ & 0x1) &&
        nextEgressStreamID_ < std::numeric_limits<int32_t>::max()) {
      nextEgressStreamID_ = nextEgressStreamID;
    }
  }

  [[nodiscard]] bool isInitiatedStream(StreamID stream) const {
    return isInitiatedStream(transportDirection_, stream);
  }

  static bool isInitiatedStream(TransportDirection direction, StreamID stream) {
    bool odd = stream & 0x01;
    bool upstream = (direction == TransportDirection::UPSTREAM);
    return (odd && upstream) || (!odd && !upstream);
  }

  [[nodiscard]] bool isStreamIngressEgressAllowed(StreamID stream) const {
    bool isInitiated = isInitiatedStream(stream);
    return (isInitiated && stream <= ingressGoawayAck_) ||
           (!isInitiated && stream <= egressGoawayAck_);
  }

 protected:
  TransportDirection transportDirection_;
  StreamID nextEgressStreamID_;
  StreamID lastStreamID_{0};
  HTTPCodec::Callback* callback_{nullptr};
  StreamID ingressGoawayAck_{std::numeric_limits<uint32_t>::max()};
  StreamID egressGoawayAck_{std::numeric_limits<uint32_t>::max()};
  std::string goawayErrorMessage_;

  enum ClosingState {
    OPEN = 0,
    OPEN_WITH_GRACEFUL_DRAIN_ENABLED = 1,
    FIRST_GOAWAY_SENT = 2,
    CLOSED = 4 // HTTP2 only
  } sessionClosing_;

  template <typename T, typename... Args>
  bool deliverCallbackIfAllowed(T callbackFn,
                                char const* cbName,
                                StreamID stream,
                                Args&&... args) {
    if (isStreamIngressEgressAllowed(stream)) {
      if (callback_) {
        (*callback_.*callbackFn)(stream, std::forward<Args>(args)...);
      }
      return true;
    } else {
      VLOG(3) << "Suppressing " << cbName << " for stream=" << stream
              << " egressGoawayAck_=" << egressGoawayAck_;
    }
    return false;
  }
};
} // namespace proxygen
