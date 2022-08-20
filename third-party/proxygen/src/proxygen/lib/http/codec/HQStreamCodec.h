/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Function.h>

#include <proxygen/lib/http/codec/HQFramedCodec.h>
#include <proxygen/lib/http/codec/HQFramer.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/HeaderDecodeInfo.h>
#include <proxygen/lib/http/codec/compress/HPACKStreamingCallback.h>

namespace proxygen {

class QPACKCodec;

namespace hq {

class HQStreamCodec
    : public HQFramedCodec
    , public HPACK::StreamingCallback {
 public:
  HQStreamCodec() = delete;
  HQStreamCodec(StreamID streamId,
                TransportDirection direction,
                QPACKCodec& headerCodec,
                folly::IOBufQueue& encoderWriteBuf,
                folly::IOBufQueue& decoderWriteBuf,
                folly::Function<uint64_t()> qpackEncoderMaxData,
                HTTPSettings& ingressSettings);
  ~HQStreamCodec() override;

  void setStrictValidation(bool strict) {
    strictValidation_ = strict;
  }

  void setActivationHook(folly::Function<folly::Function<void()>()> hook) {
    activationHook_ = std::move(hook);
  }

  StreamID getStreamID() const {
    return streamId_;
  }

  // HTTPCodec API
  HTTPCodec::StreamID createStream() override {
    // prevent calling more than once?
    return streamId_;
  }

  CodecProtocol getProtocol() const override {
    return CodecProtocol::HQ;
  }

  const std::string& getUserAgent() const override {
    return userAgent_;
  }

  bool isWaitingToDrain() const override {
    // This should never get called on a data stream codec
    // But it does from HQStreamTransport::generateGoaway
    return false;
  }

  size_t onIngress(const folly::IOBuf& buf) override {
    return onFramedIngress(buf);
  }

  void onIngressEOF() override {
    if (onFramedIngressEOF() && callback_) {
      auto g = folly::makeGuard(activationHook_());
      callback_->onMessageComplete(streamId_, false);
    } // else the conn was in error or paused
  }
  void generateHeader(
      folly::IOBufQueue& writeBuf,
      StreamID stream,
      const HTTPMessage& msg,
      bool eom = false,
      HTTPHeaderSize* size = nullptr,
      const folly::Optional<HTTPHeaders>& extraHeaders = folly::none) override;

  void generatePushPromise(folly::IOBufQueue& writeBuf,
                           StreamID stream,
                           const HTTPMessage& msg,
                           StreamID assocStream,
                           bool eom = false,
                           HTTPHeaderSize* size = nullptr) override;

  size_t generateBody(folly::IOBufQueue& writeBuf,
                      StreamID stream,
                      std::unique_ptr<folly::IOBuf> chain,
                      folly::Optional<uint8_t> padding,
                      bool eom) override;

  size_t generateChunkHeader(folly::IOBufQueue& /*writeBuf*/,
                             StreamID /*stream*/,
                             size_t /*length*/) override {
    // no op
    return 0;
  }

  // HQ has no chunk terminators
  size_t generateChunkTerminator(folly::IOBufQueue& /*writeBuf*/,
                                 StreamID /*stream*/) override {
    // no op
    return 0;
  }

  size_t generateTrailers(folly::IOBufQueue& /*writeBuf*/,
                          StreamID /*stream*/,
                          const HTTPHeaders& /*trailers*/) override;

  size_t generateEOM(folly::IOBufQueue& writeBuf, StreamID stream) override;

  uint32_t getDefaultWindowSize() const override {
    CHECK(false) << __func__ << " not supported";
    return 0;
  }

  bool peerHasWebsockets() const {
    return false;
  }

  bool isRequest(StreamID /*id*/) const {
    CHECK(false) << __func__ << " not implemented yet";
    return false;
  }

  CompressionInfo getCompressionInfo() const override;

  void onHeader(const HPACKHeaderName& name,
                const folly::fbstring& value) override;
  void onHeadersComplete(HTTPHeaderSize decodedSize, bool acknowledge) override;
  void onDecodeError(HPACK::DecodeError decodeError) override;

 protected:
  ParseResult checkFrameAllowed(FrameType type) override;
  ParseResult parseData(folly::io::Cursor& cursor,
                        const FrameHeader& header) override;
  ParseResult parseHeaders(folly::io::Cursor& cursor,
                           const FrameHeader& header) override;
  ParseResult parsePushPromise(folly::io::Cursor& cursor,
                               const FrameHeader& header) override;

 private:
  void generateHeaderImpl(folly::IOBufQueue& writeBuf,
                          const HTTPMessage& msg,
                          folly::Optional<StreamID> pushId,
                          HTTPHeaderSize* size,
                          const folly::Optional<HTTPHeaders>& extraHeaders);

  uint64_t maxEncoderStreamData() {
    auto maxData = qpackEncoderMaxDataFn_ ? qpackEncoderMaxDataFn_() : 0;
    if (qpackEncoderWriteBuf_.chainLength() >= maxData) {
      return 0;
    }
    return maxData - qpackEncoderWriteBuf_.chainLength();
  }

  size_t generateBodyImpl(folly::IOBufQueue& writeBuf,
                          std::unique_ptr<folly::IOBuf> chain);

  std::string userAgent_;
  HeaderDecodeInfo decodeInfo_;
  QPACKCodec& headerCodec_;
  folly::IOBufQueue& qpackEncoderWriteBuf_;
  folly::IOBufQueue& qpackDecoderWriteBuf_;
  folly::Function<uint64_t()> qpackEncoderMaxDataFn_;
  // Default false for now to match existing behavior
  bool strictValidation_{false};
  bool finalIngressHeadersSeen_{false};
  bool parsingTrailers_{false};
  bool finalEgressHeadersSeen_{false};
  bool isConnect_{false};
  folly::Function<folly::Function<void()>()> activationHook_{
      [] { return [] {}; }};
  HTTPSettings& ingressSettings_;
};
} // namespace hq
} // namespace proxygen
