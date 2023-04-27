/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HQFramedCodec.h>

#include <proxygen/lib/http/codec/HQFramer.h>
#include <proxygen/lib/http/codec/HQUtils.h>
#include <proxygen/lib/utils/Logging.h>

#include <folly/io/Cursor.h>
#include <folly/tracing/ScopedTraceSection.h>

#include <iomanip>

namespace proxygen { namespace hq {

using namespace folly::io;
using namespace folly;

ParseResult HQFramedCodec::parseFrame(Cursor& cursor) {
  switch (curHeader_.type) {
    case hq::FrameType::DATA:
      return parseData(cursor, curHeader_);
    case hq::FrameType::HEADERS:
      return parseHeaders(cursor, curHeader_);
    case hq::FrameType::CANCEL_PUSH:
      return parseCancelPush(cursor, curHeader_);
    case hq::FrameType::SETTINGS:
      return parseSettings(cursor, curHeader_);
    case hq::FrameType::PUSH_PROMISE:
      return parsePushPromise(cursor, curHeader_);
    case hq::FrameType::GOAWAY:
      return parseGoaway(cursor, curHeader_);
    case hq::FrameType::MAX_PUSH_ID:
      return parseMaxPushId(cursor, curHeader_);
    case hq::FrameType::PRIORITY_UPDATE:
    case hq::FrameType::PUSH_PRIORITY_UPDATE:
    case hq::FrameType::FB_PRIORITY_UPDATE:
    case hq::FrameType::FB_PUSH_PRIORITY_UPDATE:
      return parsePriorityUpdate(cursor, curHeader_);
    default:
      // Implementations MUST ignore and discard any frame that has a
      // type that is unknown
      if (callback_) {
        callback_->onUnknownFrame(streamId_,
                                  static_cast<uint64_t>(curHeader_.type));
      }
      break;
  }

  VLOG(3) << "Skipping frame (type=" << (uint64_t)curHeader_.type << ")";
  cursor.skip(curHeader_.length);
  return folly::none;
}

size_t HQFramedCodec::onFramedIngress(const IOBuf& buf) {
  FOLLY_SCOPED_TRACE_SECTION("HQFramedCodec - onFramedIngress");
  // if for some reason onFramedIngress gets called again after erroring out,
  // skip parsing
  if (connError_ != folly::none) {
    return 0;
  }
  Cursor cursor(&buf);
  size_t parsedTot = 0;
  auto bufLen = cursor.totalLength();
  while (connError_ == folly::none && bufLen > 0 && !parserPaused_) {
    size_t parsed = 0;
    if (frameState_ == FrameState::FRAME_HEADER_TYPE) {
      auto type = quic::decodeQuicInteger(cursor);
      if (!type) {
        break;
      }
      curHeader_.type = FrameType(type->first);
      parsed += type->second;
      auto res = checkFrameAllowed(curHeader_.type);
      if (res) {
        VLOG(4) << "Frame not allowed: 0x" << std::setfill('0')
                << std::setw(sizeof(uint64_t) * 2) << std::hex
                << (uint64_t)curHeader_.type << " on streamID=" << streamId_;
        connError_ = res;
        break;
      }
      frameState_ = FrameState::FRAME_HEADER_LENGTH;
    } else if (frameState_ == FrameState::FRAME_HEADER_LENGTH) {
      auto length = quic::decodeQuicInteger(cursor);
      if (!length) {
        break;
      }
      curHeader_.length = length->first;
      parsed += length->second;
      if (callback_) {
        callback_->onFrameHeader(streamId_,
                                 0, // no flags!
                                 curHeader_.length,
                                 static_cast<uint64_t>(curHeader_.type));
      }
#ifndef NDEBUG
      receivedFrameCount_++;
#endif
      pendingDataFrameBytes_ = curHeader_.length;
      if (curHeader_.length == 0) {
        // If the frame length is zero, call parseFrame immediately.  It is up
        // to each frame to determine whether length 0 is valid.
        connError_ = parseFrame(cursor);
        frameState_ = FrameState::FRAME_HEADER_TYPE;
      } else {
        // For DATA frames, move to the streaming state
        if (curHeader_.type == FrameType::DATA) {
          frameState_ = FrameState::FRAME_PAYLOAD_STREAMING;
        } else {
          frameState_ = FrameState::FRAME_PAYLOAD;
        }
      }
    } else if (frameState_ == FrameState::FRAME_PAYLOAD) {
      // Already parsed the common frame header
      const auto frameLen = curHeader_.length;
      if (bufLen >= frameLen) {
        connError_ = parseFrame(cursor);
        // if connError_ is set, it means there was a frame error,
        // so it doesn't really matter whether we have actually parsed all the
        // data or not
        parsed += curHeader_.length;
        frameState_ = FrameState::FRAME_HEADER_TYPE;
      } else {
        break;
      }
    } else if (bufLen > 0 &&
               frameState_ == FrameState::FRAME_PAYLOAD_STREAMING) {
      FrameHeader auxDataFrameHeader;
      auxDataFrameHeader.type = FrameType::DATA;
      auxDataFrameHeader.length = std::min(pendingDataFrameBytes_, bufLen);
      connError_ = parseData(cursor, auxDataFrameHeader);
      parsed += auxDataFrameHeader.length;
      pendingDataFrameBytes_ -= auxDataFrameHeader.length;
      if (pendingDataFrameBytes_ == 0) {
        frameState_ = FrameState::FRAME_HEADER_TYPE;
      }
    }
    CHECK_GE(bufLen, parsed);
    bufLen -= parsed;
    parsedTot += parsed;
    totalBytesParsed_ += parsed;
  }
  checkConnectionError(connError_, &buf);
  return parsedTot;
}

bool HQFramedCodec::onFramedIngressEOF() {
  if (connError_ != folly::none) {
    return false;
  } else if (parserPaused_) {
    deferredEOF_ = true;
    return false;
  } else if (frameState_ != FrameState::FRAME_HEADER_TYPE) {
    VLOG(3) << "Stream ended in the middle of a frame type=" << curHeader_.type;
    connError_ = HTTP3::ErrorCode::HTTP_FRAME_ERROR;
    checkConnectionError(connError_, nullptr);
    return false;
  }
  // Caller will fire onMessageComplete
  return true;
}

bool HQFramedCodec::checkConnectionError(ParseResult err,
                                         const folly::IOBuf* buf) {
  if (err != folly::none) {
    LOG(ERROR) << "Connection error with ingress=";
    if (buf) {
      VLOG(3) << IOBufPrinter::printHexFolly(buf, true);
    }
    setParserPaused(true);
    if (callback_) {
      HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                       "Connection error");
      ex.setHttp3ErrorCode(err.value());
      callback_->onError(kSessionStreamId, ex, false);
    }
    return true;
  }
  return false;
}

}} // namespace proxygen::hq
