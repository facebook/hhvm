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
#include <proxygen/lib/http/codec/TransportDirection.h>
#include <string>

#include <proxygen/external/http_parser/http_parser.h>

namespace proxygen {

class HTTP1xCodec : public HTTPCodec {
 public:
  // Default strictValidation to false for now to match existing behavior
  explicit HTTP1xCodec(TransportDirection direction,
                       bool force1_1 = false,
                       bool strictValidation = false);
  ~HTTP1xCodec() override;

  HTTP1xCodec(HTTP1xCodec&&) = default;

  // Returns codec for response generation, allowing to set flags that are
  // normally set during request processing.
  // Normally codec processes request/response pair, but is also used for
  // serialization and processes single message.
  static HTTP1xCodec makeResponseCodec(bool mayChunkEgress);

  // HTTPCodec API
  CodecProtocol getProtocol() const override {
    return CodecProtocol::HTTP_1_1;
  }

  const std::string& getUserAgent() const override {
    return userAgent_;
  }

  TransportDirection getTransportDirection() const override {
    return transportDirection_;
  }
  StreamID createStream() override;
  void setCallback(Callback* callback) override {
    callback_ = callback;
  }
  bool isBusy() const override;
  void setParserPaused(bool paused) override;
  bool isParserPaused() const override {
    return parserPaused_;
  }
  size_t onIngress(const folly::IOBuf& buf) override;
  size_t onIngressImpl(const folly::IOBuf& buf);
  void onIngressEOF() override;
  bool isReusable() const override;
  bool isWaitingToDrain() const override {
    return disableKeepalivePending_ && keepalive_;
  }
  bool isEgressBusy() const {
    return ((transportDirection_ == TransportDirection::DOWNSTREAM &&
             responsePending_) ||
            // count egress busy for non-upgraded upstream codecs with a
            // pending response.  HTTP/1.x servers are inconsistent in how they
            // interpret an EOF with a pending response, so don't trigger one
            // unless the connection was upgraded.
            (transportDirection_ == TransportDirection::UPSTREAM &&
             (requestPending_ || (!egressUpgrade_ && responsePending_))));
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
                      folly::Optional<uint8_t> padding,
                      bool eom) override;
  size_t generateChunkHeader(folly::IOBufQueue& writeBuf,
                             StreamID txn,
                             size_t length) override;
  size_t generateChunkTerminator(folly::IOBufQueue& writeBuf,
                                 StreamID txn) override;
  size_t generateTrailers(folly::IOBufQueue& writeBuf,
                          StreamID txn,
                          const HTTPHeaders& trailers) override;
  size_t generateEOM(folly::IOBufQueue& writeBuf, StreamID txn) override;
  size_t generateRstStream(folly::IOBufQueue& writeBuf,
                           StreamID txn,
                           ErrorCode statusCode) override;
  size_t generateGoaway(
      folly::IOBufQueue& writeBuf,
      StreamID lastStream,
      ErrorCode statusCode,
      std::unique_ptr<folly::IOBuf> debugData = nullptr) override;

  size_t generateImmediateGoaway(folly::IOBufQueue&,
                                 ErrorCode,
                                 std::unique_ptr<folly::IOBuf>) override {
    keepalive_ = false;
    return 0;
  }

  void setAllowedUpgradeProtocols(std::list<std::string> protocols);
  const std::string& getAllowedUpgradeProtocols();

  void setStrictValidation(bool strict) {
    strictValidation_ = strict;
  }

  /**
   * @returns true if the codec supports the given NPN protocol.
   */
  static bool supportsNextProtocol(const std::string& npn);

 private:
  /** Simple state model used to track the parsing of HTTP headers */
  enum class HeaderParseState : uint8_t {
    kParsingHeaderIdle,
    kParsingHeaderStart,
    kParsingHeaderName,
    kParsingHeaderValue,
    kParsingHeadersComplete,
    kParsingTrailerName,
    kParsingTrailerValue
  };

  std::string generateWebsocketKey() const;
  std::string generateWebsocketAccept(const std::string& acceptKey) const;
  mutable std::string websockAcceptKey_;

  /** Used to keep track of whether a client requested keep-alive. This is
   * only useful to support HTTP 1.0 keep-alive for a downstream connection
   * where keep-alive is disabled unless the client requested it. */
  enum class KeepaliveRequested : uint8_t {
    UNSET,
    ENABLED,  // incoming message requested keepalive
    DISABLED, // incoming message disabled keepalive
  };

  void addDateHeader(folly::IOBufQueue& writeBuf, size_t& len);

  /** Check whether we're currently parsing ingress message headers */
  bool isParsingHeaders() const {
    return (headerParseState_ > HeaderParseState::kParsingHeaderIdle) &&
           (headerParseState_ < HeaderParseState::kParsingHeadersComplete);
  }

  /** Check whether we're currently parsing ingress header-or-trailer name */
  bool isParsingHeaderOrTrailerName() const {
    return (headerParseState_ == HeaderParseState::kParsingHeaderName) ||
           (headerParseState_ == HeaderParseState::kParsingTrailerName);
  }

  /** Invoked when a parsing error occurs. It will send an exception to
      the callback object to report the error and do any other cleanup
      needed. It optionally takes a message to pass to the generated
      HTTPException passed to callback_. */
  void onParserError(const char* what = nullptr);

  /** Push out header name-value pair to hdrs and clear currentHeader*_ */
  bool pushHeaderNameAndValue(HTTPHeaders& hdrs);

  /** Serialize websocket headers into a buffer **/
  void serializeWebsocketHeader(folly::IOBufQueue& writeBuf,
                                size_t& len,
                                bool upstream);

  // Parser callbacks
  int onMessageBegin();
  int onURL(const char* buf, size_t len);
  int onReason(const char* buf, size_t len);
  int onHeaderField(const char* buf, size_t len);
  int onHeaderValue(const char* buf, size_t len);
  int onHeadersComplete(size_t len);
  int onBody(const char* buf, size_t len);
  int onChunkHeader(size_t len);
  int onChunkComplete();
  int onMessageComplete();

  HTTPCodec::Callback* callback_;
  StreamID ingressTxnID_;
  StreamID egressTxnID_;
  http_parser parser_;
  const folly::IOBuf* currentIngressBuf_;
  std::unique_ptr<HTTPMessage> msg_;
  std::unique_ptr<HTTPMessage> upgradeRequest_;
  std::unique_ptr<HTTPHeaders> trailers_;
  std::string currentHeaderName_;
  folly::StringPiece currentHeaderNameStringPiece_;
  std::string currentHeaderValue_;
  std::string url_;
  std::string userAgent_;
  std::string reason_;
  std::string upgradeHeader_; // last sent/received client upgrade header
  std::string allowedNativeUpgrades_; // DOWNSTREAM only
  HTTPHeaderSize headerSize_;
  HeaderParseState headerParseState_;
  TransportDirection transportDirection_;
  KeepaliveRequested keepaliveRequested_; // only used in DOWNSTREAM mode
  std::pair<CodecProtocol, std::string> upgradeResult_; // DOWNSTREAM only
  bool force1_1_ : 1; // Use HTTP/1.1 even if msg is 1.0
  bool strictValidation_ : 1;
  bool parserActive_ : 1;
  bool pendingEOF_ : 1;
  bool parserPaused_ : 1;
  bool parserError_ : 1;
  bool requestPending_ : 1;
  bool responsePending_ : 1;
  bool egressChunked_ : 1;
  bool inChunk_ : 1;
  bool lastChunkWritten_ : 1;
  bool keepalive_ : 1;
  bool disableKeepalivePending_ : 1;
  // TODO: replace the 2 booleans below with an enum "request method"
  bool connectRequest_ : 1;
  bool headRequest_ : 1;
  bool expectNoResponseBody_ : 1;
  bool mayChunkEgress_ : 1;
  bool is1xxResponse_ : 1;
  bool inRecvLastChunk_ : 1;
  bool ingressUpgrade_ : 1;
  bool ingressUpgradeComplete_ : 1;
  bool egressUpgrade_ : 1;
  bool nativeUpgrade_ : 1;
  bool headersComplete_ : 1;

  // C-callable wrappers for the http_parser callbacks
  static int onMessageBeginCB(http_parser* parser);
  static int onPathCB(http_parser* parser, const char* buf, size_t len);
  static int onQueryStringCB(http_parser* parser, const char* buf, size_t len);
  static int onUrlCB(http_parser* parser, const char* buf, size_t len);
  static int onReasonCB(http_parser* parser, const char* buf, size_t len);
  static int onHeaderFieldCB(http_parser* parser, const char* buf, size_t len);
  static int onHeaderValueCB(http_parser* parser, const char* buf, size_t len);
  static int onHeadersCompleteCB(http_parser* parser,
                                 const char* buf,
                                 size_t len);
  static int onBodyCB(http_parser* parser, const char* buf, size_t len);
  static int onChunkHeaderCB(http_parser* parser);
  static int onChunkCompleteCB(http_parser* parser);
  static int onMessageCompleteCB(http_parser* parser);

  static const http_parser_settings* getParserSettings();
};

} // namespace proxygen
