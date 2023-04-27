/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HTTP2Framer.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/HTTPParallelCodec.h>
#include <proxygen/lib/http/codec/HTTPRequestVerifier.h>
#include <proxygen/lib/http/codec/HTTPSettings.h>
#include <proxygen/lib/http/codec/HeaderDecodeInfo.h>
#include <proxygen/lib/http/codec/compress/HPACKCodec.h>

#include <bitset>
#include <set>

namespace proxygen {

/**
 * An implementation of the framing layer for HTTP/2. Instances of this
 * class must not be used from multiple threads concurrently.
 */
class HTTP2Codec
    : public HTTPParallelCodec
    , HPACK::StreamingCallback {
 public:
  void onHeader(const HPACKHeaderName& name,
                const folly::fbstring& value) override;
  void onHeadersComplete(HTTPHeaderSize decodedSize, bool acknowledge) override;
  void onDecodeError(HPACK::DecodeError decodeError) override;

  explicit HTTP2Codec(TransportDirection direction);
  ~HTTP2Codec() override;

  // HTTPCodec API
  CodecProtocol getProtocol() const override {
    return CodecProtocol::HTTP_2;
  }

  const std::string& getUserAgent() const override {
    return userAgent_;
  }

  size_t onIngress(const folly::IOBuf& buf) override;
  bool onIngressUpgradeMessage(const HTTPMessage& msg) override;
  size_t generateConnectionPreface(folly::IOBufQueue& writeBuf) override;
  void generateHeader(
      folly::IOBufQueue& writeBuf,
      StreamID stream,
      const HTTPMessage& msg,
      bool eom = false,
      HTTPHeaderSize* size = nullptr,
      const folly::Optional<HTTPHeaders>& extraHeaders = folly::none) override;
  void generateContinuation(folly::IOBufQueue& writeBuf,
                            folly::IOBufQueue& queue,
                            StreamID stream,
                            size_t maxFrameSize);
  void generatePushPromise(folly::IOBufQueue& writeBuf,
                           StreamID stream,
                           const HTTPMessage& msg,
                           StreamID assocStream,
                           bool eom = false,
                           HTTPHeaderSize* size = nullptr) override;
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
  size_t generateChunkHeader(folly::IOBufQueue& writeBuf,
                             StreamID stream,
                             size_t length) override;
  size_t generateChunkTerminator(folly::IOBufQueue& writeBuf,
                                 StreamID stream) override;
  size_t generateTrailers(folly::IOBufQueue& writeBuf,
                          StreamID stream,
                          const HTTPHeaders& trailers) override;
  size_t generateEOM(folly::IOBufQueue& writeBuf, StreamID stream) override;
  size_t generateRstStream(folly::IOBufQueue& writeBuf,
                           StreamID stream,
                           ErrorCode statusCode) override;
  size_t generateGoaway(
      folly::IOBufQueue& writeBuf,
      StreamID lastStream = HTTPCodec::MaxStreamID,
      ErrorCode statusCode = ErrorCode::NO_ERROR,
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
                          StreamID stream,
                          const HTTPMessage::HTTP2Priority& pri) override;
  size_t generatePriority(folly::IOBufQueue& /* writeBuf */,
                          StreamID /* stream */,
                          HTTPPriority /* priority */) override;

  size_t generateCertificateRequest(
      folly::IOBufQueue& writeBuf,
      uint16_t requestId,
      std::unique_ptr<folly::IOBuf> certificateRequestData) override;
  size_t generateCertificate(folly::IOBufQueue& writeBuf,
                             uint16_t certId,
                             std::unique_ptr<folly::IOBuf> certData) override;
  const HTTPSettings* getIngressSettings() const override {
    return &ingressSettings_;
  }
  HTTPSettings* getEgressSettings() override {
    return &egressSettings_;
  }
  uint32_t getDefaultWindowSize() const override {
    return http2::kInitialWindow;
  }
  bool supportsPushTransactions() const override {
    return (transportDirection_ == TransportDirection::DOWNSTREAM &&
            ingressSettings_.getSetting(SettingsId::ENABLE_PUSH, 1)) ||
           (transportDirection_ == TransportDirection::UPSTREAM &&
            egressSettings_.getSetting(SettingsId::ENABLE_PUSH, 1));
  }
  bool peerHasWebsockets() const {
    return ingressSettings_.getSetting(SettingsId::ENABLE_CONNECT_PROTOCOL);
  }
  bool supportsExTransactions() const override {
    return ingressSettings_.getSetting(SettingsId::ENABLE_EX_HEADERS, 0) &&
           egressSettings_.getSetting(SettingsId::ENABLE_EX_HEADERS, 0);
  }
  void setHeaderCodecStats(HeaderCodec::Stats* hcStats) override {
    headerCodec_.setStats(hcStats);
  }

  bool isRequest(StreamID id) const {
    return ((transportDirection_ == TransportDirection::DOWNSTREAM &&
             (id & 0x1) == 1) ||
            (transportDirection_ == TransportDirection::UPSTREAM &&
             (id & 0x1) == 0));
  }

  size_t addPriorityNodes(PriorityQueue& queue,
                          folly::IOBufQueue& writeBuf,
                          uint8_t maxLevel) override;
  HTTPCodec::StreamID mapPriorityToDependency(uint8_t priority) const override;

  CompressionInfo getCompressionInfo() const override {
    return headerCodec_.getCompressionInfo();
  }

  // HTTP2Codec specific API

  static void requestUpgrade(HTTPMessage& request);

  static size_t generateDefaultSettings(folly::IOBufQueue& writeBuf);

#ifndef NDEBUG
  uint64_t getReceivedFrameCount() const {
    return receivedFrameCount_;
  }
#endif

  // Whether turn on the optimization to reuse IOBuf headroom when write DATA
  // frame. For other frames, it's always ON.
  void setReuseIOBufHeadroomForData(bool enabled) {
    reuseIOBufHeadroomForData_ = enabled;
  }

  void setHeaderIndexingStrategy(const HeaderIndexingStrategy* indexingStrat) {
    headerCodec_.setHeaderIndexingStrategy(indexingStrat);
  }
  const HeaderIndexingStrategy* getHeaderIndexingStrategy() const {
    return headerCodec_.getHeaderIndexingStrategy();
  }

  void setAddDateHeaderToResponse(bool addDateHeader) {
    addDateToResponse_ = addDateHeader;
  }

  void setValidateHeaders(bool validate) {
    validateHeaders_ = validate;
  }

  void setStrictValidation(bool strict) {
    strictValidation_ = strict;
  }

 private:
  size_t splitCompressed(size_t compressed,
                         uint32_t remainingFrameSize,
                         folly::IOBufQueue& writeBuf,
                         folly::IOBufQueue& queue);

  void generateHeaderImpl(folly::IOBufQueue& writeBuf,
                          StreamID stream,
                          const HTTPMessage& msg,
                          const folly::Optional<StreamID>& assocStream,
                          const folly::Optional<ExAttributes>& exAttributes,
                          bool eom,
                          HTTPHeaderSize* size,
                          const folly::Optional<HTTPHeaders>& extraHeaders);
  void encodeHeaders(folly::IOBufQueue& writeBuf,
                     const HTTPHeaders& headers,
                     std::vector<compress::Header>& allHeaders,
                     HTTPHeaderSize* size);

  size_t generateHeaderCallbackWrapper(StreamID stream,
                                       http2::FrameType type,
                                       size_t length);

  ErrorCode parseFrame(folly::io::Cursor& cursor);
  ErrorCode parseAllData(folly::io::Cursor& cursor);
  ErrorCode parseDataFrameData(folly::io::Cursor& cursor,
                               size_t bufLen,
                               size_t& parsed);
  ErrorCode parseHeaders(folly::io::Cursor& cursor);
  ErrorCode parseExHeaders(folly::io::Cursor& cursor);
  ErrorCode parsePriority(folly::io::Cursor& cursor);
  ErrorCode parseRstStream(folly::io::Cursor& cursor);
  ErrorCode parseSettings(folly::io::Cursor& cursor);
  ErrorCode parsePushPromise(folly::io::Cursor& cursor);
  ErrorCode parsePing(folly::io::Cursor& cursor);
  ErrorCode parseGoaway(folly::io::Cursor& cursor);
  ErrorCode parseContinuation(folly::io::Cursor& cursor);
  ErrorCode parseWindowUpdate(folly::io::Cursor& cursor);
  ErrorCode parseCertificateRequest(folly::io::Cursor& cursor);
  ErrorCode parseCertificate(folly::io::Cursor& cursor);
  ErrorCode parseHeadersImpl(
      folly::io::Cursor& cursor,
      std::unique_ptr<folly::IOBuf> headerBuf,
      const folly::Optional<http2::PriorityUpdate>& priority,
      const folly::Optional<uint32_t>& promisedStream,
      const folly::Optional<ExAttributes>& exAttributes);

  struct DeferredParseError {
    ErrorCode errorCode{ErrorCode::NO_ERROR};
    bool connectionError{false};
    std::string errorMessage;
    mutable std::unique_ptr<HTTPMessage> partialMessage;

    DeferredParseError(ErrorCode ec,
                       bool conn,
                       std::string msg,
                       std::unique_ptr<HTTPMessage> partialMsg = nullptr)
        : errorCode(ec),
          connectionError(conn),
          errorMessage(std::move(msg)),
          partialMessage(std::move(partialMsg)) {
    }

    DeferredParseError() = default;
    DeferredParseError(DeferredParseError&& goner) = default;
    DeferredParseError& operator=(DeferredParseError&& goner) = default;
    DeferredParseError(const DeferredParseError& other)
        : errorCode(other.errorCode),
          connectionError(other.connectionError),

          errorMessage(other.errorMessage),
          partialMessage(other.partialMessage ? std::make_unique<HTTPMessage>(
                                                    *other.partialMessage)
                                              : nullptr) {
    }
  };

  folly::Expected<std::unique_ptr<HTTPMessage>, DeferredParseError>
  parseHeadersDecodeFrames(
      const folly::Optional<http2::PriorityUpdate>& priority,
      const folly::Optional<ExAttributes>& exAttributes);
  void deliverDeferredParseError(const DeferredParseError& parseError);

  folly::Optional<ErrorCode> parseHeadersCheckConcurrentStreams(
      const folly::Optional<http2::PriorityUpdate>& priority);

  ErrorCode handleEndStream();
  ErrorCode checkNewStream(uint32_t stream, bool trailersAllowed);
  bool checkConnectionError(ErrorCode, const folly::IOBuf* buf);
  ErrorCode handleSettings(const std::deque<SettingPair>& settings);
  void handleSettingsAck();
  size_t maxSendFrameSize() const {
    return (uint32_t)ingressSettings_.getSetting(
        SettingsId::MAX_FRAME_SIZE, http2::kMaxFramePayloadLengthMin);
  }
  uint32_t maxRecvFrameSize() const {
    return (uint32_t)egressSettings_.getSetting(
        SettingsId::MAX_FRAME_SIZE, http2::kMaxFramePayloadLengthMin);
  }
  void streamError(const std::string& msg,
                   ErrorCode error,
                   bool newTxn = false,
                   folly::Optional<HTTPCodec::StreamID> streamId = folly::none,
                   std::unique_ptr<HTTPMessage> partialMsg = nullptr);
  bool parsingHeaders() const;
  bool parsingTrailers() const;

  HPACKCodec headerCodec_;

  // Current frame state
  http2::FrameHeader curHeader_;
  StreamID expectedContinuationStream_{0};
  // Used for parsing PUSH_PROMISE+CONTINUATION
  folly::Optional<StreamID> promisedStream_;
  bool parsingReq_{false};
  bool pendingEndStreamHandling_{false};
  bool ingressWebsocketUpgrade_{false};

  std::unordered_set<StreamID> upgradedStreams_;

  uint16_t curCertId_{0};
  folly::IOBufQueue curAuthenticatorBlock_{
      folly::IOBufQueue::cacheChainLength()};

  folly::IOBufQueue curHeaderBlock_{folly::IOBufQueue::cacheChainLength()};
  HTTPSettings ingressSettings_{
      {SettingsId::HEADER_TABLE_SIZE, 4096},
      {SettingsId::ENABLE_PUSH, 1},
      {SettingsId::MAX_FRAME_SIZE, 16384},
  };
  HTTPSettings egressSettings_{
      {SettingsId::HEADER_TABLE_SIZE, 4096},
      {SettingsId::ENABLE_PUSH, 0},
      {SettingsId::MAX_FRAME_SIZE, 16384},
      {SettingsId::MAX_HEADER_LIST_SIZE, 1 << 17},
  };
#ifndef NDEBUG
  uint64_t receivedFrameCount_{0};
#endif
  enum class FrameState : uint8_t {
    UPSTREAM_CONNECTION_PREFACE = 0,
    EXPECT_FIRST_SETTINGS = 1,
    FRAME_HEADER = 2,
    FRAME_DATA = 3,
    DATA_FRAME_DATA = 4,
  };
  FrameState frameState_ : 3;
  std::string userAgent_;

  size_t pendingDataFrameBytes_{0};
  size_t pendingDataFramePaddingBytes_{0};

  HeaderDecodeInfo decodeInfo_;
  std::vector<StreamID> virtualPriorityNodes_;
  folly::Optional<uint32_t> pendingTableMaxSize_;
  bool reuseIOBufHeadroomForData_{true};

  // True if last parsed HEADERS frame was trailers.
  // Reset only when HEADERS frame is parsed, thus
  // remains unchanged and used during CONTINUATION frame
  // parsing as well.
  // Applies only to DOWNSTREAM, for UPSTREAM we use
  // diffrent heuristic - lack of status code.
  bool parsingDownstreamTrailers_{false};
  bool addDateToResponse_{true};
  bool validateHeaders_{true};
  // Default false for now to match existing behavior
  bool strictValidation_{false};

  // CONTINUATION frame can follow either HEADERS or PUSH_PROMISE frames.
  // Keeps frame type of initiating frame of header block.
  http2::FrameType headerBlockFrameType_{http2::FrameType::DATA};
};

} // namespace proxygen
