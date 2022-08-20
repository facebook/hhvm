/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Conv.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>

namespace proxygen {

/**
 * Class for stream-parsing RFC 1867 style post data.  At present it does
 * not support nested multi-part content (multipart/mixed).
 * Can parse multiple POST bodies
 * unless one of them invokes the onError() callback.  After that, the codec is
 * no longer usable.
 */
class RFC1867Codec : HTTPCodec::Callback {
 public:
  class Callback {
   public:
    virtual ~Callback() {
    }
    // return < 0 to skip remainder of field callbacks?
    virtual int onFieldStart(const std::string& name,
                             folly::Optional<std::string> filename,
                             std::unique_ptr<HTTPMessage> msg,
                             uint64_t postBytesProcessed) = 0;
    virtual int onFieldData(std::unique_ptr<folly::IOBuf>,
                            uint64_t postBytesProcessed) = 0;
    /** On reading to end of a part indicated by boundary
     * @param endedOnBoundary indicate successful part end
     */
    virtual void onFieldEnd(bool endedOnBoundary,
                            uint64_t postBytesProcessed) = 0;
    virtual void onError() = 0;
  };

  // boundary is the parameter to Content-Type, eg:
  //
  //   Content-type: multipart/form-data, boundary=AaB03x
  explicit RFC1867Codec(const std::string& boundary) {
    CHECK(!boundary.empty());
    boundary_ = folly::to<std::string>("\n--", boundary);
    headerParser_.setCallback(this);
  }

  void setCallback(Callback* callback) {
    callback_ = callback;
  }

  // Pass the next piece of input data.  Returns unparsed data that requires
  // more input to continue
  std::unique_ptr<folly::IOBuf> onIngress(std::unique_ptr<folly::IOBuf> data);

  // The end of input has been seen.  Validate the parser state and reset
  // for more parsing.
  void onIngressEOM();

  uint64_t getBytesProcessed() const {
    return bytesProcessed_;
  }

 private:
  enum class ParserState {
    START,
    HEADERS_START,
    HEADERS,
    FIELD_DATA, // part, or field, not only file
    DONE,
    ERROR
  };

  // HTTPCodec::Callback
  void onMessageBegin(HTTPCodec::StreamID /*stream*/,
                      HTTPMessage* /*msg*/) override {
  }
  void onHeadersComplete(HTTPCodec::StreamID stream,
                         std::unique_ptr<HTTPMessage> msg) override;
  void onBody(HTTPCodec::StreamID /*stream*/,
              std::unique_ptr<folly::IOBuf> /*chain*/,
              uint16_t /*padding*/) override {
    parseError_ = true;
    headerParser_.setParserPaused(true);
  }
  void onTrailersComplete(HTTPCodec::StreamID /*stream*/,
                          std::unique_ptr<HTTPHeaders> /*trailers*/) override {
    parseError_ = true;
    headerParser_.setParserPaused(true);
  }
  void onMessageComplete(HTTPCodec::StreamID /*stream*/,
                         bool /*upgrade*/) override {
    headerParser_.setParserPaused(true);
  }

  void onError(HTTPCodec::StreamID /*stream*/,
               const HTTPException& /*error*/,
               bool /*newTxn*/) override {
    parseError_ = true;
    headerParser_.setParserPaused(true);
  }

  folly::IOBufQueue readToBoundary(bool& foundBoundary);

  std::string boundary_;
  Callback* callback_{nullptr};
  ParserState state_{ParserState::START};
  HTTP1xCodec headerParser_{TransportDirection::DOWNSTREAM};
  std::string field_;
  folly::IOBufQueue input_{folly::IOBufQueue::cacheChainLength()};
  folly::IOBufQueue value_;
  std::unique_ptr<folly::IOBuf> pendingCR_;
  uint64_t bytesProcessed_{0};
  bool parseError_{false};
};

} // namespace proxygen
