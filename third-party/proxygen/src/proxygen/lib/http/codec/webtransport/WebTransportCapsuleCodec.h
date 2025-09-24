/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/CapsuleCodec.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportFramer.h>

namespace proxygen {

enum class CodecVersion { H2, H3 };

class WebTransportCapsuleCodec : public CapsuleCodec {
 public:
  class Callback : public CapsuleCodec::Callback {
   public:
    ~Callback() override = default;

    virtual void onPaddingCapsule(PaddingCapsule capsule) = 0;
    virtual void onWTResetStreamCapsule(WTResetStreamCapsule capsule) = 0;
    virtual void onWTStopSendingCapsule(WTStopSendingCapsule capsule) = 0;
    virtual void onWTStreamCapsule(WTStreamCapsule capsule) = 0;
    virtual void onWTMaxDataCapsule(WTMaxDataCapsule capsule) = 0;
    virtual void onWTMaxStreamDataCapsule(WTMaxStreamDataCapsule capsule) = 0;
    virtual void onWTMaxStreamsBidiCapsule(WTMaxStreamsCapsule capsule) = 0;
    virtual void onWTMaxStreamsUniCapsule(WTMaxStreamsCapsule capsule) = 0;
    virtual void onWTDataBlockedCapsule(WTDataBlockedCapsule capsule) = 0;
    virtual void onWTStreamDataBlockedCapsule(
        WTStreamDataBlockedCapsule capsule) = 0;
    virtual void onWTStreamsBlockedBidiCapsule(
        WTStreamsBlockedCapsule capsule) = 0;
    virtual void onWTStreamsBlockedUniCapsule(
        WTStreamsBlockedCapsule capsule) = 0;
    virtual void onDatagramCapsule(DatagramCapsule capsule) = 0;
    virtual void onCloseWebTransportSessionCapsule(
        CloseWebTransportSessionCapsule capsule) = 0;
    virtual void onDrainWebTransportSessionCapsule(
        DrainWebTransportSessionCapsule capsule) = 0;
  };

  WebTransportCapsuleCodec(Callback* callback, CodecVersion version)
      : CapsuleCodec(callback), callback_(callback), version_(version) {
  }

  ~WebTransportCapsuleCodec() override = default;

  void setCallback(Callback* callback) {
    CapsuleCodec::setCallback(callback);
    callback_ = callback;
  }

 private:
  bool canParseCapsule(uint64_t capsuleType) override {
    switch (capsuleType) {
      // Common H2 and H3 capsule types
      case folly::to_underlying(CapsuleType::WT_MAX_DATA):
      case folly::to_underlying(CapsuleType::WT_MAX_STREAMS_BIDI):
      case folly::to_underlying(CapsuleType::WT_MAX_STREAMS_UNI):
      case folly::to_underlying(CapsuleType::WT_DATA_BLOCKED):
      case folly::to_underlying(CapsuleType::WT_STREAMS_BLOCKED_BIDI):
      case folly::to_underlying(CapsuleType::WT_STREAMS_BLOCKED_UNI):
      case folly::to_underlying(CapsuleType::CLOSE_WEBTRANSPORT_SESSION):
      case folly::to_underlying(CapsuleType::DRAIN_WEBTRANSPORT_SESSION):
        return true;
      // H2 only capsule types
      case folly::to_underlying(CapsuleType::PADDING):
      case folly::to_underlying(CapsuleType::WT_RESET_STREAM):
      case folly::to_underlying(CapsuleType::WT_STOP_SENDING):
      case folly::to_underlying(CapsuleType::WT_STREAM):
      case folly::to_underlying(CapsuleType::WT_STREAM_WITH_FIN):
      case folly::to_underlying(CapsuleType::WT_MAX_STREAM_DATA):
      case folly::to_underlying(CapsuleType::WT_STREAM_DATA_BLOCKED):
      case folly::to_underlying(CapsuleType::DATAGRAM):
        return version_ == CodecVersion::H2;
      default:
        return false;
    }
  }

  folly::Expected<folly::Unit, ErrorCode> parseCapsule(
      folly::io::Cursor& cursor) override;

  Callback* callback_{nullptr};
  CodecVersion version_;
};

} // namespace proxygen
