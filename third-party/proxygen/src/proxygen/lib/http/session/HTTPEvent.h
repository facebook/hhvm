/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <glog/logging.h>
#include <proxygen/lib/http/HTTPConstants.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>

namespace proxygen {

/**
 * Helper class that holds some event in the lifecycle
 * of an HTTP request or response.
 *
 * The main use for this class is to queue up events in
 * situations where the code handling events isn't able
 * to process them and the thing generating events isn't
 * able to stop.
 */
class HTTPEvent {
 public:
  enum class Type : uint8_t {
    // Ingress events
    MESSAGE_BEGIN,
    HEADERS_COMPLETE,
    BODY,
    CHUNK_HEADER,
    CHUNK_COMPLETE,
    TRAILERS_COMPLETE,
    MESSAGE_COMPLETE,
    UPGRADE,
    ERROR,
  };

  HTTPEvent(HTTPCodec::StreamID streamID, Type event, bool upgrade = false)
      : streamID_(streamID), length_(0), event_(event), upgrade_(upgrade) {
  }

  HTTPEvent(HTTPCodec::StreamID streamID, Type event, size_t length)
      : streamID_(streamID), length_(length), event_(event), upgrade_(false) {
    // This constructor should only be used for CHUNK_HEADER.
    // (Ideally we would take the event type as a template parameter
    // so we could enforce this check at compile time.  Unfortunately,
    // that would prevent us from using this constructor with
    // deferredCallbacks_.emplace().)
    CHECK(event == Type::CHUNK_HEADER);
  }

  HTTPEvent(HTTPCodec::StreamID streamID,
            Type event,
            std::unique_ptr<HTTPMessage> headers)
      : headers_(std::move(headers)),
        streamID_(streamID),
        length_(0),
        event_(event),
        upgrade_(false) {
  }

  HTTPEvent(HTTPCodec::StreamID streamID,
            Type event,
            std::unique_ptr<folly::IOBuf> body)
      : body_(std::move(body)),
        streamID_(streamID),
        length_(0),
        event_(event),
        upgrade_(false) {
  }

  HTTPEvent(HTTPCodec::StreamID streamID,
            Type event,
            std::unique_ptr<HTTPHeaders> trailers)
      : trailers_(std::move(trailers)),
        streamID_(streamID),
        length_(0),
        event_(event),
        upgrade_(false) {
  }

  HTTPEvent(HTTPCodec::StreamID streamID, std::unique_ptr<HTTPException> error)
      : error_(std::move(error)),
        streamID_(streamID),
        length_(0),
        event_(Type::ERROR),
        upgrade_(false) {
    CHECK(error_);
  }

  HTTPEvent(HTTPCodec::StreamID streamID, Type event, UpgradeProtocol protocol)
      : streamID_(streamID),
        length_(0),
        event_(event),
        upgrade_(false),
        protocol_(protocol) {
  }

  Type getEvent() const {
    return event_;
  }

  HTTPCodec::StreamID getStreamID() const {
    return streamID_;
  }

  std::unique_ptr<HTTPMessage> getHeaders() {
    return std::move(headers_);
  }

  std::unique_ptr<folly::IOBuf> getBody() {
    return std::move(body_);
  }

  std::unique_ptr<HTTPException> getError() {
    return std::move(error_);
  }

  bool isUpgrade() const {
    CHECK(event_ == Type::MESSAGE_COMPLETE);
    return upgrade_;
  }

  size_t getChunkLength() const {
    CHECK(event_ == Type::CHUNK_HEADER);
    return length_;
  }

  std::unique_ptr<HTTPHeaders> getTrailers() {
    return std::move(trailers_);
  }

  UpgradeProtocol getUpgradeProtocol() {
    return protocol_;
  }

 private:
  std::unique_ptr<HTTPMessage> headers_;
  std::unique_ptr<folly::IOBuf> body_;
  std::unique_ptr<HTTPHeaders> trailers_;
  std::unique_ptr<HTTPException> error_;
  HTTPCodec::StreamID streamID_;
  size_t length_; // Only valid when event_ == CHUNK_HEADER
  Type event_;
  bool upgrade_; // Only valid when event_ == MESSAGE_COMPLETE
  UpgradeProtocol protocol_;
};

std::ostream& operator<<(std::ostream& os, HTTPEvent::Type e);

} // namespace proxygen
