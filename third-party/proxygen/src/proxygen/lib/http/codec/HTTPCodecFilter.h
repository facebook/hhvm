/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/utils/FilterChain.h>

namespace proxygen {

using HTTPCodecFilter = GenericFilter<HTTPCodec,
                                      HTTPCodec::Callback,
                                      &HTTPCodec::setCallback,
                                      true>;

/**
 * An implementation of HTTPCodecFilter that passes through all calls. This is
 * useful to subclass if you aren't interested in intercepting every function.
 * See HTTPCodec.h for documentation on these methods.
 */
class PassThroughHTTPCodecFilter : public HTTPCodecFilter {
 public:
  /**
   * By default, the filter gets both calls and callbacks
   */
  explicit PassThroughHTTPCodecFilter(bool calls = true, bool callbacks = true)
      : HTTPCodecFilter(calls, callbacks) {
  }

  // HTTPCodec::Callback methods
  void onMessageBegin(StreamID stream, HTTPMessage* msg) override;

  void onPushMessageBegin(StreamID stream,
                          StreamID assocStream,
                          HTTPMessage* msg) override;

  void onHeadersComplete(StreamID stream,
                         std::unique_ptr<HTTPMessage> msg) override;

  void onBody(StreamID stream,
              std::unique_ptr<folly::IOBuf> chain,
              uint16_t padding) override;

  void onChunkHeader(StreamID stream, size_t length) override;

  void onChunkComplete(StreamID stream) override;

  void onTrailersComplete(StreamID stream,
                          std::unique_ptr<HTTPHeaders> trailers) override;

  void onMessageComplete(StreamID stream, bool upgrade) override;

  void onFrameHeader(StreamID stream_id,
                     uint8_t flags,
                     uint64_t length,
                     uint64_t type,
                     uint16_t version = 0) override;

  void onError(StreamID stream,
               const HTTPException& error,
               bool newStream = false) override;

  void onAbort(StreamID stream, ErrorCode code) override;

  void onGoaway(uint64_t lastGoodStreamID,
                ErrorCode code,
                std::unique_ptr<folly::IOBuf> debugData = nullptr) override;

  void onPingRequest(uint64_t data) override;

  void onPingReply(uint64_t data) override;

  void onWindowUpdate(StreamID stream, uint32_t amount) override;

  void onSettings(const SettingsList& settings) override;

  void onSettingsAck() override;

  void onPriority(StreamID stream, const HTTPPriority& pri) override;

  void onPushPriority(StreamID stream, const HTTPPriority& pri) override;

  void onGenerateFrameHeader(StreamID streamID,
                             uint8_t type,
                             uint64_t length,
                             uint16_t version) override;

  void onCertificateRequest(uint16_t requestId,
                            std::unique_ptr<folly::IOBuf> authRequest) override;

  void onCertificate(uint16_t certId,
                     std::unique_ptr<folly::IOBuf> authenticator) override;

  [[nodiscard]] uint32_t numOutgoingStreams() const override;

  [[nodiscard]] uint32_t numIncomingStreams() const override;

  // HTTPCodec methods
  [[nodiscard]] CompressionInfo getCompressionInfo() const override;

  [[nodiscard]] CodecProtocol getProtocol() const override;

  [[nodiscard]] const std::string& getUserAgent() const override;

  [[nodiscard]] TransportDirection getTransportDirection() const override;

  [[nodiscard]] bool supportsStreamFlowControl() const override;

  [[nodiscard]] bool supportsSessionFlowControl() const override;

  StreamID createStream() override;

  void setCallback(HTTPCodec::Callback* callback) override;

  [[nodiscard]] bool isBusy() const override;

  void setParserPaused(bool paused) override;

  [[nodiscard]] bool isParserPaused() const override;

  size_t onIngress(const folly::IOBuf& buf) override;

  void onIngressEOF() override;

  bool onIngressUpgradeMessage(const HTTPMessage& msg) override;

  [[nodiscard]] bool isReusable() const override;

  [[nodiscard]] bool isWaitingToDrain() const override;

  [[nodiscard]] bool closeOnEgressComplete() const override;

  [[nodiscard]] bool supportsParallelRequests() const override;

  [[nodiscard]] bool supportsPushTransactions() const override;

  size_t generateConnectionPreface(folly::IOBufQueue& writeBuf) override;

  void generateHeader(
      folly::IOBufQueue& writeBuf,
      StreamID stream,
      const HTTPMessage& msg,
      bool eom,
      HTTPHeaderSize* size,
      const folly::Optional<HTTPHeaders>& extraHeaders) override;

  void generatePushPromise(folly::IOBufQueue& writeBuf,
                           StreamID stream,
                           const HTTPMessage& msg,
                           StreamID assocStream,
                           bool eom,
                           HTTPHeaderSize* size) override;

  size_t generateBody(folly::IOBufQueue& writeBuf,
                      StreamID stream,
                      std::unique_ptr<folly::IOBuf> chain,
                      folly::Optional<uint8_t> padding,
                      bool eom) override;

  size_t generateBodyDSR(StreamID stream,
                         size_t length,
                         folly::Optional<uint8_t> padding,
                         bool eom) override;

  size_t generateChunkHeader(folly::IOBufQueue& writeBuf,
                             StreamID stream,
                             size_t length) override;

  size_t generateChunkTerminator(folly::IOBufQueue& writeBuf,
                                 StreamID stream) override;

  size_t generateTrailers(folly::IOBufQueue& writeBuf,
                          StreamID stream,
                          const HTTPHeaders& trailers) override;

  size_t generatePadding(folly::IOBufQueue& writeBuf,
                         StreamID stream,
                         uint16_t bytes) override;

  size_t generateEOM(folly::IOBufQueue& writeBuf, StreamID stream) override;

  size_t generateRstStream(folly::IOBufQueue& writeBuf,
                           StreamID stream,
                           ErrorCode statusCode) override;

  size_t generateGoaway(
      folly::IOBufQueue& writeBuf,
      StreamID lastStream,
      ErrorCode statusCode,
      std::unique_ptr<folly::IOBuf> debugData = nullptr) override;

  size_t generatePingRequest(
      folly::IOBufQueue& writeBuf,
      folly::Optional<uint64_t> data = folly::none) override;

  size_t generatePingReply(folly::IOBufQueue& writeBuf, uint64_t data) override;

  size_t generateSettings(folly::IOBufQueue& writeBuf) override;

  size_t generateSettingsAck(folly::IOBufQueue& writeBuf) override;

  size_t generateWindowUpdate(folly::IOBufQueue& writeBuf,
                              StreamID stream,
                              uint32_t delta) override;

  size_t generatePriority(folly::IOBufQueue& writeBuf,
                          StreamID streamId,
                          HTTPPriority priority) override;

  size_t generatePushPriority(folly::IOBufQueue& writeBuf,
                              StreamID pushId,
                              HTTPPriority priority) override;

  size_t generateCertificateRequest(
      folly::IOBufQueue& writeBuf,
      uint16_t requestId,
      std::unique_ptr<folly::IOBuf> chain) override;

  size_t generateCertificate(folly::IOBufQueue& writeBuf,
                             uint16_t certId,
                             std::unique_ptr<folly::IOBuf> certData) override;

  HTTPSettings* getEgressSettings() override;

  [[nodiscard]] const HTTPSettings* getIngressSettings() const override;

  void setHeaderCodecStats(HeaderCodec::Stats* stats) override;

  void enableDoubleGoawayDrain() override;

  void disableDoubleGoawayDrain() override;

  [[nodiscard]] HTTPCodec::StreamID getLastIncomingStreamID() const override;

  [[nodiscard]] uint32_t getDefaultWindowSize() const override;

  [[nodiscard]] int8_t mapDependencyToPriority(StreamID parent) const override;
};

using HTTPCodecFilterChain = FilterChain<HTTPCodec,
                                         HTTPCodec::Callback,
                                         PassThroughHTTPCodecFilter,
                                         &HTTPCodec::setCallback,
                                         true>;

} // namespace proxygen
