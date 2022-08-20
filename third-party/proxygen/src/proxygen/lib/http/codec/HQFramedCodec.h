/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HQFramer.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>

#include <folly/lang/Assume.h>

namespace proxygen { namespace hq {

/*
 * HQFramedCodec encapsulates the core logic for interfacing with the HQ Framer,
 * so that it can be used by both Stream and Control codecs.
 * It also provides common implementations of HTTPCodec APIs where it makes
 * sense, mostly for disabling calls that do not make sense for HQ
 */
class HQFramedCodec : public HTTPCodec {
 public:
  explicit HQFramedCodec(HTTPCodec::StreamID streamId,
                         TransportDirection direction)
      : streamId_(streamId),
        transportDirection_(direction),
        frameState_(FrameState::FRAME_HEADER_TYPE) {
  }

  ~HQFramedCodec() override = default;

  size_t onFramedIngress(const folly::IOBuf& buf);

  // HTTPCodec API

  // Only implemented in the Stream Codec
  CodecProtocol getProtocol() const override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // Only implemented in the Stream Codec
  const std::string& getUserAgent() const override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  TransportDirection getTransportDirection() const override {
    return transportDirection_;
  }

  // Stream multiplexing handled at the transport
  HTTPCodec::StreamID createStream() override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  void setCallback(proxygen::HTTPCodec::Callback* callback) override {
    callback_ = callback;
  }

  bool isBusy() const override {
    return false;
  }

  void setParserPaused(bool paused) override {
    bool resumed = parserPaused_ && !paused;
    parserPaused_ = paused;
    if (!paused && deferredEOF_) {
      deferredEOF_ = false;
      onIngressEOF();
    } else if (resumed && resumeHook_) {
      resumeHook_();
    }
  }

  void setResumeHook(folly::Function<void()> resumeHook) {
    resumeHook_ = std::move(resumeHook);
  }

  bool isParserPaused() const override {
    return parserPaused_;
  }

  bool isReusable() const override {
    return false;
  }

  bool closeOnEgressComplete() const override {
    return false;
  }

  bool supportsParallelRequests() const override {
    return false;
  }

  // No h2c upgrade in HQ
  bool onIngressUpgradeMessage(const HTTPMessage& /*msg*/) override {
    return false;
  }

  // no connection preface for HQ
  size_t generateConnectionPreface(folly::IOBufQueue& /*writeBuf*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // only valid for the Stream Codec
  void generateHeader(folly::IOBufQueue& /*writeBuf*/,
                      StreamID /*stream*/,
                      const HTTPMessage& /*msg*/,
                      bool /*eom = false*/,
                      HTTPHeaderSize* /*size = nullptr*/,
                      const folly::Optional<HTTPHeaders>&) override {
    LOG(FATAL) << __func__ << " must be implemented in child class";
    folly::assume_unreachable();
  }

  // only valid for the Stream Codec
  void generatePushPromise(folly::IOBufQueue& /*writeBuf*/,
                           StreamID /*stream*/,
                           const HTTPMessage& /*msg*/,
                           StreamID /*assocStream*/,
                           bool /*eom = false*/,
                           HTTPHeaderSize* /*size = nullptr*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // Not Supported for the time being
  void generateExHeader(folly::IOBufQueue& /*writeBuf*/,
                        StreamID /*stream*/,
                        const HTTPMessage& /*msg*/,
                        const HTTPCodec::ExAttributes& /*exAttributes*/,
                        bool /*eom = false*/,
                        HTTPHeaderSize* /*size = nullptr*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
  }

  // only valid for the Stream Codec
  size_t generateBody(folly::IOBufQueue& /* writeBuf*/,
                      StreamID /*stream*/,
                      std::unique_ptr<folly::IOBuf> /*chain*/,
                      folly::Optional<uint8_t> /*padding*/,
                      bool /*eom*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // HQ has no chunk headers
  size_t generateChunkHeader(folly::IOBufQueue& /*writeBuf*/,
                             StreamID /*stream*/,
                             size_t /*length*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // HQ has no chunk terminators
  size_t generateChunkTerminator(folly::IOBufQueue& /*writeBuf*/,
                                 StreamID /*stream*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  size_t generateTrailers(folly::IOBufQueue& /*writeBuf*/,
                          StreamID /*stream*/,
                          const HTTPHeaders& /*trailers*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // Only valid for the Stream Codec
  size_t generateEOM(folly::IOBufQueue& /*writeBuf*/,
                     StreamID /*stream*/) override {
    LOG(FATAL) << __func__ << " must be implemented in child class";
    folly::assume_unreachable();
  }

  // Handled at the transport layer
  size_t generateRstStream(folly::IOBufQueue& /*writeBuf*/,
                           StreamID /*stream*/,
                           ErrorCode /*statusCode*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // only valid for the Control Codec
  size_t generateGoaway(
      folly::IOBufQueue& /*writeBuf*/,
      StreamID /*lastStream*/,
      ErrorCode /*statusCode*/,
      std::unique_ptr<folly::IOBuf> /*debugData = nullptr*/) override {
    LOG(FATAL) << __func__ << " must be implemented in child class";
    folly::assume_unreachable();
  }

  // Handled at the transport layer
  size_t generatePingRequest(
      folly::IOBufQueue& /*writeBuf*/,
      folly::Optional<uint64_t> /* data */ = folly::none) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // Handled at the transport layer
  size_t generatePingReply(folly::IOBufQueue& /*writeBuf*/,
                           uint64_t /* data */) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // only valid for the Control Codec
  size_t generateSettings(folly::IOBufQueue& /*writeBuf*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // No Settings ACK in HQ
  size_t generateSettingsAck(folly::IOBufQueue& /*writeBuf*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // Flow Control Handled at the transport layer
  size_t generateWindowUpdate(folly::IOBufQueue& /*writeBuf*/,
                              StreamID /*stream*/,
                              uint32_t /*delta*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // only valid for the Control Codec
  size_t generatePriority(folly::IOBufQueue& /*writeBuf*/,
                          StreamID /*stream*/,
                          const HTTPMessage::HTTP2Priority& /*pri*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // only valid for the Control Codec
  const HTTPSettings* getIngressSettings() const override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // only valid for the Control Codec
  HTTPSettings* getEgressSettings() override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // Flow Control Handled at the transport layer
  uint32_t getDefaultWindowSize() const override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  bool supportsPushTransactions() const override {
    return true;
  }

  bool peerHasWebsockets() const {
    return false;
  }

  bool supportsExTransactions() const override {
    return false;
  }

  void setHeaderCodecStats(HeaderCodec::Stats* /*hcStats*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
  }

  // only valid on the Stream Codec
  bool isRequest(StreamID /*id*/) const {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // only valid for the Control Codec
  size_t addPriorityNodes(PriorityQueue& /*queue*/,
                          folly::IOBufQueue& /*writeBuf*/,
                          uint8_t /*maxLevel*/) override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // only valid for the Control Codec
  HTTPCodec::StreamID mapPriorityToDependency(
      uint8_t /*priority*/) const override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  // HTTPCodec has a default implementation, override that here to fail.
  // StreamCodec returns the QPACK table info
  CompressionInfo getCompressionInfo() const override {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

 protected:
  virtual ParseResult checkFrameAllowed(FrameType type) = 0;

  virtual ParseResult parseData(folly::io::Cursor& /*cursor*/,
                                const FrameHeader& /*header*/) {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  virtual ParseResult parseHeaders(folly::io::Cursor& /*cursor*/,
                                   const FrameHeader& /*header*/) {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  virtual ParseResult parseCancelPush(folly::io::Cursor& /*cursor*/,
                                      const FrameHeader& /*header*/) {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  virtual ParseResult parseSettings(folly::io::Cursor& /*cursor*/,
                                    const FrameHeader& /*header*/) {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  virtual ParseResult parsePushPromise(folly::io::Cursor& /*cursor*/,
                                       const FrameHeader& /*header*/) {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  virtual ParseResult parseGoaway(folly::io::Cursor& /*cursor*/,
                                  const FrameHeader& /*header*/) {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  virtual ParseResult parseMaxPushId(folly::io::Cursor& /*cursor*/,
                                     const FrameHeader& /*header*/) {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  virtual ParseResult parsePushPriorityUpdate(folly::io::Cursor&,
                                              const FrameHeader&) {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  virtual ParseResult parsePriorityUpdate(folly::io::Cursor&,
                                          const FrameHeader&) {
    LOG(FATAL) << __func__ << " not supported on this codec";
    folly::assume_unreachable();
  }

  uint64_t getCodecTotalBytesParsed() const {
    return totalBytesParsed_;
  }

  bool onFramedIngressEOF();

  HTTPCodec::StreamID streamId_;
  TransportDirection transportDirection_;
  HTTPCodec::Callback* callback_{nullptr};
  hq::FrameHeader curHeader_;
  bool parserPaused_{false};
  bool deferredEOF_{false};

 private:
  ParseResult parseFrame(folly::io::Cursor& cursor);
  bool checkConnectionError(ParseResult err, const folly::IOBuf* buf);

  // Current frame state
  size_t pendingDataFrameBytes_{0};

#ifndef NDEBUG
  uint64_t receivedFrameCount_{0};
#endif

  enum class FrameState : uint8_t {
    FRAME_HEADER_TYPE = 0,
    FRAME_HEADER_LENGTH = 1,
    FRAME_PAYLOAD = 2,
    FRAME_PAYLOAD_STREAMING = 3,
  };
  FrameState frameState_ : 3;
  ParseResult connError_{folly::none};
  uint64_t totalBytesParsed_{0};
  folly::Function<void()> resumeHook_;
};

}} // namespace proxygen::hq
