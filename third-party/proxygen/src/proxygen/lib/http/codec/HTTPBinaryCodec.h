/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/HeaderDecodeInfo.h>
#include <proxygen/lib/http/codec/TransportDirection.h>
#include <string>

namespace proxygen {

using ParseResult = folly::Expected<size_t, std::string>;

/* The HTTPBinaryCodec class is an implementation of the "Binary Representation
 * of HTTP Messages" RFC -
 * (https://datatracker.ietf.org/doc/html/draft-ietf-httpbis-binary-message-01).
 * We currently only support the "Known Length Messages" mode.
 */
class HTTPBinaryCodec : public HTTPCodec {
 public:
  // Default strictValidation to false for now to match existing behavior
  explicit HTTPBinaryCodec(TransportDirection direction);
  ~HTTPBinaryCodec() override;

  HTTPBinaryCodec(HTTPBinaryCodec&&) = default;

  // HTTPCodec API
  CodecProtocol getProtocol() const override {
    return CodecProtocol::HTTP_BINARY;
  }

  const std::string& getUserAgent() const override {
    return userAgent_;
  }

  TransportDirection getTransportDirection() const override {
    return transportDirection_;
  }
  StreamID createStream() override {
    return 0;
  }
  void setCallback(Callback* callback) override {
    callback_ = callback;
  }
  bool isBusy() const override {
    return false;
  }
  void setParserPaused(bool paused) override {
    parserPaused_ = paused;
  }
  bool isParserPaused() const override {
    return parserPaused_;
  }
  size_t onIngress(const folly::IOBuf& buf) override;
  void onIngressEOF() override;
  bool isReusable() const override {
    return true;
  }
  bool isWaitingToDrain() const override {
    return false;
  }
  bool isEgressBusy() const {
    return false;
  }
  // True if the session requires an EOF (or RST) to terminate the message
  bool closeOnEgressComplete() const override {
    return !isEgressBusy() && !isReusable();
  }
  bool supportsParallelRequests() const override {
    return false;
  }
  bool supportsPushTransactions() const override {
    return false;
  }
  size_t generateChunkHeader(folly::IOBufQueue& writeBuf,
                             StreamID stream,
                             size_t length) override;
  size_t generateChunkTerminator(folly::IOBufQueue& writeBuf,
                                 StreamID stream) override;
  size_t generateRstStream(folly::IOBufQueue& writeBuf,
                           StreamID stream,
                           ErrorCode statusCode) override;
  size_t generateGoaway(
      folly::IOBufQueue& writeBuf,
      StreamID lastStream = HTTPCodec::MaxStreamID,
      ErrorCode statusCode = ErrorCode::NO_ERROR,
      std::unique_ptr<folly::IOBuf> debugData = nullptr) override;
  void generateHeader(
      folly::IOBufQueue& writeBuf,
      StreamID txn,
      const HTTPMessage& msg,
      bool eom = false,
      HTTPHeaderSize* size = nullptr,
      const folly::Optional<HTTPHeaders>& extraHeaders = folly::none) override;
  size_t generateBody(folly::IOBufQueue& writeBuf,
                      StreamID txn,
                      std::unique_ptr<folly::IOBuf> chain,
                      folly::Optional<uint8_t> padding = folly::none,
                      bool eom = false) override;
  size_t generateTrailers(folly::IOBufQueue& writeBuf,
                          StreamID txn,
                          const HTTPHeaders& trailers) override;
  size_t generateEOM(folly::IOBufQueue& writeBuf, StreamID txn) override;

 protected:
  /* The format of Binary HTTP Messages is the following:
   *
   * Message with Known-Length {
   *    Framing Indicator (i) = 0..1,
   *    Known-Length Informational Response (..),
   *    Control Data (..),
   *    Known-Length Field Section (..),
   *    Known-Length Content (..),
   *    Known-Length Field Section (..),
   *    Padding (..),
   *  }
   */
  ParseResult parseFramingIndicator(folly::io::Cursor& cursor,
                                    bool& request,
                                    bool& knownLength);
  ParseResult parseKnownLengthString(folly::io::Cursor& cursor,
                                     size_t remaining,
                                     folly::StringPiece stringName,
                                     std::string& stringValue);
  ParseResult parseRequestControlData(folly::io::Cursor& cursor,
                                      size_t remaining,
                                      HTTPMessage& msg);
  ParseResult parseResponseControlData(folly::io::Cursor& cursor,
                                       size_t remaining,
                                       HTTPMessage& msg);
  ParseResult parseHeaders(folly::io::Cursor& cursor,
                           size_t remaining,
                           HeaderDecodeInfo& decodeInfo);
  ParseResult parseContent(folly::io::Cursor& cursor,
                           size_t remaining,
                           HTTPMessage& msg);
  ParseResult parseTrailers(folly::io::Cursor& cursor,
                            size_t remaining,
                            HeaderDecodeInfo& decodeInfo);
  ParseResult parseHeadersHelper(folly::io::Cursor& cursor,
                                 size_t remaining,
                                 HeaderDecodeInfo& decodeInfo,
                                 bool isTrailers);
  size_t generateHeaderHelper(folly::io::QueueAppender& appender,
                              const HTTPHeaders& headers);

  bool request_{true};
  bool knownLength_{true};
  enum class ParseState : uint8_t {
    FRAMING_INDICATOR = 0,
    INFORMATIONAL_RESPONSE = 1,
    CONTROL_DATA = 2,
    HEADERS_SECTION = 3,
    CONTENT = 4,
    TRAILERS_SECTION = 5,
    PADDING = 6,
  };
  ParseState state_;
  bool parserPaused_;
  folly::Optional<std::string> parseError_{folly::none};

  enum class FramingIndicator : uint8_t {
    REQUEST_KNOWN_LENGTH = 0,
    RESPONSE_KNOWN_LENGTH = 1,
    REQUEST_INDETERMINATE_LENGTH = 2,
    RESPONSE_INDETERMINATE_LENGTH = 3,
  };

  const size_t queueAppenderMaxGrowth = 256;

  // This callback_ will be how we return decoded responses to the caller
  HTTPCodec::Callback* callback_;

  StreamID ingressTxnID_;

  folly::IOBufQueue bufferedIngress_{folly::IOBufQueue::cacheChainLength()};
  // We don't need to use an IOBufQueue for msgBody_ since we are writing a
  // complete message to it and don't need to rely on the efficient size
  // computation provided by IOBufQueue
  std::unique_ptr<folly::IOBuf> msgBody_;

  HeaderDecodeInfo decodeInfo_;
  std::unique_ptr<HTTPMessage> msg_;
  std::unique_ptr<HTTPHeaders> trailers_;
  std::string userAgent_;
  TransportDirection transportDirection_;
};

} // namespace proxygen
