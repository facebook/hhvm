/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HQFramedCodec.h>
#include <proxygen/lib/http/codec/HQFramer.h>
#include <proxygen/lib/http/codec/HQUnidirectionalCodec.h>
#include <proxygen/lib/http/codec/HQUtils.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/compress/QPACKCodec.h>

#include <folly/io/IOBuf.h>
#include <folly/lang/Assume.h>

namespace proxygen { namespace hq {

class HQControlCodec
    : public HQUnidirectionalCodec
    , public HQFramedCodec {

 public:
  HQControlCodec(
      HTTPCodec::StreamID streamId,
      TransportDirection direction,
      StreamDirection streamDir,
      HTTPSettings& settings,
      UnidirectionalStreamType streamType = UnidirectionalStreamType::CONTROL)
      : HQUnidirectionalCodec(streamType, streamDir),
        HQFramedCodec(streamId, direction),
        settings_(settings) {
    VLOG(4) << "creating " << getTransportDirectionString(direction)
            << " HQ Control codec for stream " << streamId_;
    egressGoawayAck_ = direction == TransportDirection::UPSTREAM
                           ? kMaxPushId + 1
                           : kMaxClientBidiStreamId;
  }

  ~HQControlCodec() override {
  }

  // HQ Unidirectional Codec API
  std::unique_ptr<folly::IOBuf> onUnidirectionalIngress(
      std::unique_ptr<folly::IOBuf> buf) override {
    CHECK(buf);
    auto consumed = onFramedIngress(*buf);
    folly::IOBufQueue q;
    q.append(std::move(buf));
    q.trimStart(consumed);
    return q.move();
  }

  void onUnidirectionalIngressEOF() override {
    LOG(ERROR) << "Unexpected control stream EOF";
    if (callback_) {
      HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                       "Control stream EOF");
      ex.setHttp3ErrorCode(HTTP3::ErrorCode::HTTP_CLOSED_CRITICAL_STREAM);
      callback_->onError(streamId_, ex, false);
    }
  }

  // HTTPCodec API
  bool isWaitingToDrain() const override;

  CodecProtocol getProtocol() const override {
    return CodecProtocol::HQ;
  }

  size_t onIngress(const folly::IOBuf& /*buf*/) override {
    LOG(FATAL) << __func__ << " not supported";
    folly::assume_unreachable();
  }

  void onIngressEOF() override {
    // error
  }

  size_t generateGoaway(
      folly::IOBufQueue& writeBuf,
      StreamID lastStream,
      ErrorCode statusCode,
      std::unique_ptr<folly::IOBuf> debugData = nullptr) override;

  size_t generateSettings(folly::IOBufQueue& writeBuf) override;

  size_t generatePriority(folly::IOBufQueue& writeBuf,
                          StreamID stream,
                          const HTTPMessage::HTTP2Priority& pri) override;

  size_t generatePriority(folly::IOBufQueue& writeBuf,
                          StreamID stream,
                          HTTPPriority priority) override;

  size_t generatePushPriority(folly::IOBufQueue& writeBuf,
                              StreamID pushId,
                              HTTPPriority priority) override;

  const HTTPSettings* getIngressSettings() const override {
    CHECK(isIngress());
    return &settings_;
  }

  HTTPSettings* getEgressSettings() override {
    CHECK(isEgress());
    return &settings_;
  }

  void enableDoubleGoawayDrain() override {
    doubleGoaway_ = true;
  }

  uint32_t getDefaultWindowSize() const override {
    CHECK(false) << __func__ << " not supported";
    folly::assume_unreachable();
  }

  void setHeaderCodecStats(HeaderCodec::Stats* /*hcStats*/) override {
    CHECK(false) << __func__ << " not supported";
  }

  CompressionInfo getCompressionInfo() const override {
    CHECK(false) << __func__ << " not supported";
    folly::assume_unreachable();
  }

  size_t addPriorityNodes(PriorityQueue& queue,
                          folly::IOBufQueue& writeBuf,
                          uint8_t maxLevel) override;

  HTTPCodec::StreamID mapPriorityToDependency(uint8_t priority) const override;

 protected:
  ParseResult checkFrameAllowed(FrameType type) override;
  ParseResult parseCancelPush(folly::io::Cursor& cursor,
                              const FrameHeader& header) override;
  ParseResult parseSettings(folly::io::Cursor& cursor,
                            const FrameHeader& header) override;
  ParseResult parseGoaway(folly::io::Cursor& cursor,
                          const FrameHeader& header) override;
  ParseResult parseMaxPushId(folly::io::Cursor& cursor,
                             const FrameHeader& header) override;
  ParseResult parsePriorityUpdate(folly::io::Cursor& cursor,
                                  const FrameHeader& header) override;
  ParseResult parsePushPriorityUpdate(folly::io::Cursor& cursor,
                                      const FrameHeader& header) override;

  uint64_t finalGoawayId();

 protected:
  bool doubleGoaway_{true};
  bool sentGoaway_{false};
  bool sentFinalGoaway_{false};
  bool receivedSettings_{false};
  bool sentSettings_{false};
  quic::StreamId egressGoawayAck_;
  quic::StreamId minUnseenStreamID_{kMaxClientBidiStreamId};
  uint64_t minUnseenPushID_{kMaxPushId + 1};
  HTTPSettings& settings_;
};

}} // namespace proxygen::hq
