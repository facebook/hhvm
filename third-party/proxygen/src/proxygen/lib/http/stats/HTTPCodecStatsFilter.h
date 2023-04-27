/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/CodecProtocol.h>
#include <proxygen/lib/http/codec/HTTPCodecFilter.h>

namespace proxygen {

class HTTPCodecStats;

class HTTPCodecStatsFilter : public PassThroughHTTPCodecFilter {
 public:
  explicit HTTPCodecStatsFilter(HTTPCodecStats* counters,
                                CodecProtocol protocol);
  ~HTTPCodecStatsFilter() override;

  void setCounters(HTTPCodecStats* counters) {
    counters_ = counters;
  }
  // ingress

  bool isHTTP2() const {
    return isHTTP2CodecProtocol(protocol_);
  }

  bool isHQ() const {
    return isHQCodecProtocol(protocol_);
  }

  void onPushMessageBegin(StreamID stream,
                          StreamID assocStream,
                          HTTPMessage* msg) override;

  void onExMessageBegin(StreamID stream,
                        StreamID controlStream,
                        bool unidirectional,
                        HTTPMessage* msg) override;

  void onHeadersComplete(StreamID stream,
                         std::unique_ptr<HTTPMessage> msg) override;

  void onBody(StreamID stream,
              std::unique_ptr<folly::IOBuf> chain,
              uint16_t padding) override;

  void onAbort(StreamID stream, ErrorCode statusCode) override;

  void onGoaway(uint64_t lastGoodStreamID,
                ErrorCode statusCode,
                std::unique_ptr<folly::IOBuf> debugData = nullptr) override;

  void onPingRequest(uint64_t data) override;

  void onPingReply(uint64_t data) override;

  void onWindowUpdate(StreamID stream, uint32_t amount) override;

  void onSettings(const SettingsList& settings) override;

  void onSettingsAck() override;

  void onPriority(StreamID stream,
                  const HTTPMessage::HTTP2Priority& pri) override;
  void onPriority(StreamID stream, const HTTPPriority& pri) override;

  // egress

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

  void generateExHeader(folly::IOBufQueue& writeBuf,
                        StreamID stream,
                        const HTTPMessage& msg,
                        const HTTPCodec::ExAttributes& exAttributes,
                        bool eom = false,
                        HTTPHeaderSize* size = nullptr) override;

  size_t generateBody(folly::IOBufQueue& writeBuf,
                      StreamID stream,
                      std::unique_ptr<folly::IOBuf> chain,
                      folly::Optional<uint8_t> padding,
                      bool eom) override;

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

  size_t generateWindowUpdate(folly::IOBufQueue& writeBuf,
                              StreamID stream,
                              uint32_t delta) override;

  size_t generatePriority(folly::IOBufQueue& writeBuf,
                          StreamID stream,
                          const HTTPMessage::HTTP2Priority& pri) override;
  size_t generatePriority(folly::IOBufQueue& writeBuf,
                          StreamID streamId,
                          HTTPPriority pri) override;
  size_t generatePushPriority(folly::IOBufQueue& writeBuf,
                              StreamID pushId,
                              HTTPPriority pri) override;

 private:
  HTTPCodecStats* counters_;
  CodecProtocol protocol_;
};

} // namespace proxygen
