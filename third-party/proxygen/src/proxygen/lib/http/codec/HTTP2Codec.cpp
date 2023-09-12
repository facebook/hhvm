/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HTTP2Codec.h>

#include <folly/base64.h>
#include <proxygen/lib/http/codec/CodecUtil.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>
#include <proxygen/lib/utils/Logging.h>

#include <folly/Conv.h>
#include <folly/Random.h>
#include <folly/Try.h>
#include <folly/io/Cursor.h>
#include <folly/tracing/ScopedTraceSection.h>
#include <type_traits>

using namespace folly::io;
using namespace folly;

using std::string;

namespace {
const size_t kDefaultGrowth = 4000;
} // namespace

namespace proxygen {

HTTP2Codec::HTTP2Codec(TransportDirection direction)
    : HTTPParallelCodec(direction),
      headerCodec_(direction),
      frameState_(direction == TransportDirection::DOWNSTREAM
                      ? FrameState::UPSTREAM_CONNECTION_PREFACE
                      : FrameState::EXPECT_FIRST_SETTINGS) {

  const auto maxHeaderListSize =
      egressSettings_.getSetting(SettingsId::MAX_HEADER_LIST_SIZE);
  if (maxHeaderListSize) {
    headerCodec_.setMaxUncompressed(maxHeaderListSize->value);
  }

  VLOG(4) << "creating " << getTransportDirectionString(direction)
          << " HTTP/2 codec";
}

HTTP2Codec::~HTTP2Codec() {
}

// HTTPCodec API

size_t HTTP2Codec::onIngress(const folly::IOBuf& buf) {
  // TODO: ensure only 1 parse at a time on stack.
  FOLLY_SCOPED_TRACE_SECTION("HTTP2Codec - onIngress");

  Cursor cursor(&buf);
  size_t parsed = 0;
  ErrorCode connError = ErrorCode::NO_ERROR;
  auto bufLen = cursor.totalLength();
  while (connError == ErrorCode::NO_ERROR) {
    size_t remaining = bufLen - parsed;
    if (frameState_ == FrameState::UPSTREAM_CONNECTION_PREFACE) {
      if (remaining >= http2::kConnectionPreface.length()) {
        auto test = cursor.readFixedString(http2::kConnectionPreface.length());
        parsed += http2::kConnectionPreface.length();
        if (test != http2::kConnectionPreface) {
          goawayErrorMessage_ = "missing connection preface";
          VLOG(4) << goawayErrorMessage_;
          connError = ErrorCode::PROTOCOL_ERROR;
        }
        frameState_ = FrameState::EXPECT_FIRST_SETTINGS;
      } else {
        break;
      }
    } else if (frameState_ == FrameState::FRAME_HEADER ||
               frameState_ == FrameState::EXPECT_FIRST_SETTINGS) {
      // Waiting to parse the common frame header
      if (remaining >= http2::kFrameHeaderSize) {
        connError = parseFrameHeader(cursor, curHeader_);
        parsed += http2::kFrameHeaderSize;
        if (frameState_ == FrameState::EXPECT_FIRST_SETTINGS &&
            curHeader_.type != http2::FrameType::SETTINGS) {
          goawayErrorMessage_ = folly::to<string>(
              "GOAWAY error: got invalid connection preface frame type=",
              getFrameTypeString(curHeader_.type),
              "(",
              curHeader_.type,
              ")",
              " for streamID=",
              curHeader_.stream);
          VLOG(4) << goawayErrorMessage_;
          connError = ErrorCode::PROTOCOL_ERROR;
        }
        if (curHeader_.length > maxRecvFrameSize()) {
          VLOG(4) << "Excessively large frame len=" << curHeader_.length;
          connError = ErrorCode::FRAME_SIZE_ERROR;
        }

        if (callback_) {
          callback_->onFrameHeader(curHeader_.stream,
                                   curHeader_.flags,
                                   curHeader_.length,
                                   static_cast<uint8_t>(curHeader_.type));
        }

        if (curHeader_.type == http2::FrameType::DATA) {
          frameState_ = FrameState::DATA_FRAME_DATA;
          pendingDataFrameBytes_ = curHeader_.length;
          pendingDataFramePaddingBytes_ = 0;
        } else {
          frameState_ = FrameState::FRAME_DATA;
        }
#ifndef NDEBUG
        receivedFrameCount_++;
#endif
      } else {
        break;
      }
    } else if (frameState_ == FrameState::DATA_FRAME_DATA && remaining > 0 &&
               (remaining < curHeader_.length ||
                pendingDataFrameBytes_ < curHeader_.length)) {
      // FrameState::DATA_FRAME_DATA with partial data only
      size_t dataParsed = 0;
      connError = parseDataFrameData(cursor, remaining, dataParsed);
      if (dataParsed == 0 && pendingDataFrameBytes_ > 0) {
        // We received only the padding byte, we will wait for more
        break;
      } else {
        parsed += dataParsed;
        if (pendingDataFrameBytes_ == 0) {
          frameState_ = FrameState::FRAME_HEADER;
        }
      }
    } else { // FrameState::FRAME_DATA
             // or FrameState::DATA_FRAME_DATA with all data available
      // Already parsed the common frame header
      const auto frameLen = curHeader_.length;
      if (remaining >= frameLen) {
        connError = parseFrame(cursor);
        parsed += curHeader_.length;
        frameState_ = FrameState::FRAME_HEADER;
      } else {
        break;
      }
    }
  }
  checkConnectionError(connError, &buf);
  return parsed;
}

ErrorCode HTTP2Codec::parseFrame(folly::io::Cursor& cursor) {
  FOLLY_SCOPED_TRACE_SECTION("HTTP2Codec - parseFrame");
  if (expectedContinuationStream_ != 0 &&
      (curHeader_.type != http2::FrameType::CONTINUATION ||
       expectedContinuationStream_ != curHeader_.stream)) {
    goawayErrorMessage_ = folly::to<string>(
        "GOAWAY error: while expected CONTINUATION with stream=",
        expectedContinuationStream_,
        ", received streamID=",
        curHeader_.stream,
        " of type=",
        getFrameTypeString(curHeader_.type));
    VLOG(4) << goawayErrorMessage_;
    return ErrorCode::PROTOCOL_ERROR;
  }
  if (expectedContinuationStream_ == 0 &&
      curHeader_.type == http2::FrameType::CONTINUATION) {
    goawayErrorMessage_ = folly::to<string>(
        "GOAWAY error: unexpected CONTINUATION received with streamID=",
        curHeader_.stream);
    VLOG(4) << goawayErrorMessage_;
    return ErrorCode::PROTOCOL_ERROR;
  }
  if (frameAffectsCompression(curHeader_.type) &&
      curHeaderBlock_.chainLength() + curHeader_.length >
          egressSettings_.getSetting(SettingsId::MAX_HEADER_LIST_SIZE, 0)) {
    // this may be off by up to the padding length (max 255), but
    // these numbers are already so generous, and we're comparing the
    // max-uncompressed to the actual compressed size.  Let's fail
    // before buffering.

    // TODO(t6513634): it would be nicer to stream-process this header
    // block to keep the connection state consistent without consuming
    // memory, and fail just the request per the HTTP/2 spec (section
    // 10.3)
    goawayErrorMessage_ = folly::to<string>(
        "Failing connection due to excessively large headers");
    LOG(ERROR) << goawayErrorMessage_;
    return ErrorCode::PROTOCOL_ERROR;
  }

  expectedContinuationStream_ = (frameAffectsCompression(curHeader_.type) &&
                                 !(curHeader_.flags & http2::END_HEADERS))
                                    ? curHeader_.stream
                                    : 0;

  switch (curHeader_.type) {
    case http2::FrameType::DATA:
      return parseAllData(cursor);
    case http2::FrameType::HEADERS:
      return parseHeaders(cursor);
    case http2::FrameType::PRIORITY:
      return parsePriority(cursor);
    case http2::FrameType::RST_STREAM:
      return parseRstStream(cursor);
    case http2::FrameType::SETTINGS:
      return parseSettings(cursor);
    case http2::FrameType::PUSH_PROMISE:
      return parsePushPromise(cursor);
    case http2::FrameType::EX_HEADERS:
      if (ingressSettings_.getSetting(SettingsId::ENABLE_EX_HEADERS, 0)) {
        return parseExHeaders(cursor);
      } else {
        VLOG(2) << "EX_HEADERS not enabled, ignoring the frame";
        break;
      }
    case http2::FrameType::PING:
      return parsePing(cursor);
    case http2::FrameType::GOAWAY:
      return parseGoaway(cursor);
    case http2::FrameType::WINDOW_UPDATE:
      return parseWindowUpdate(cursor);
    case http2::FrameType::CONTINUATION:
      return parseContinuation(cursor);
    case http2::FrameType::ALTSVC:
      // fall through, unimplemented
      break;
    case http2::FrameType::CERTIFICATE_REQUEST:
      return parseCertificateRequest(cursor);
    case http2::FrameType::CERTIFICATE:
      return parseCertificate(cursor);
    default:
      // Implementations MUST ignore and discard any frame that has a
      // type that is unknown
      break;
  }

  // Landing here means unknown, unimplemented or ignored frame.
  VLOG(2) << "Skipping frame (type=" << (uint8_t)curHeader_.type << ")";
  cursor.skip(curHeader_.length);
  return ErrorCode::NO_ERROR;
}

ErrorCode HTTP2Codec::handleEndStream() {
  if (curHeader_.type != http2::FrameType::HEADERS &&
      curHeader_.type != http2::FrameType::EX_HEADERS &&
      curHeader_.type != http2::FrameType::CONTINUATION &&
      curHeader_.type != http2::FrameType::DATA) {
    return ErrorCode::NO_ERROR;
  }

  // do we need to handle case where this stream has already aborted via
  // another callback (onHeadersComplete/onBody)?
  pendingEndStreamHandling_ |= (curHeader_.flags & http2::END_STREAM);

  // with a websocket upgrade, we need to send message complete cb to
  // mirror h1x codec's behavior. when the stream closes, we need to
  // send another callback to clean up the stream's resources.
  if (ingressWebsocketUpgrade_) {
    ingressWebsocketUpgrade_ = false;
    deliverCallbackIfAllowed(&HTTPCodec::Callback::onMessageComplete,
                             "onMessageComplete",
                             curHeader_.stream,
                             true);
  }

  if (pendingEndStreamHandling_ && expectedContinuationStream_ == 0) {
    pendingEndStreamHandling_ = false;
    deliverCallbackIfAllowed(&HTTPCodec::Callback::onMessageComplete,
                             "onMessageComplete",
                             curHeader_.stream,
                             false);
  }
  return ErrorCode::NO_ERROR;
}

ErrorCode HTTP2Codec::parseAllData(Cursor& cursor) {
  std::unique_ptr<IOBuf> outData;
  uint16_t padding = 0;
  VLOG(10) << "parsing all frame DATA bytes for stream=" << curHeader_.stream
           << " length=" << curHeader_.length;
  auto ret = http2::parseData(cursor, curHeader_, outData, padding);
  RETURN_IF_ERROR(ret);

  if (callback_ && (padding > 0 || (outData && !outData->empty()))) {
    if (!outData) {
      outData = std::make_unique<IOBuf>();
    }
    deliverCallbackIfAllowed(&HTTPCodec::Callback::onBody,
                             "onBody",
                             curHeader_.stream,
                             std::move(outData),
                             padding);
  }
  return handleEndStream();
}

ErrorCode HTTP2Codec::parseDataFrameData(Cursor& cursor,
                                         size_t bufLen,
                                         size_t& parsed) {
  FOLLY_SCOPED_TRACE_SECTION("HTTP2Codec - parseDataFrameData");
  if (bufLen == 0) {
    VLOG(10) << "No data to parse";
    return ErrorCode::NO_ERROR;
  }

  std::unique_ptr<IOBuf> outData;
  uint16_t padding = 0;
  VLOG(10) << "parsing DATA frame data for stream=" << curHeader_.stream
           << " frame data length=" << curHeader_.length
           << " pendingDataFrameBytes_=" << pendingDataFrameBytes_
           << " pendingDataFramePaddingBytes_=" << pendingDataFramePaddingBytes_
           << " bufLen=" << bufLen << " parsed=" << parsed;
  // Parse the padding information only the first time
  if (pendingDataFrameBytes_ == curHeader_.length &&
      pendingDataFramePaddingBytes_ == 0) {
    if (frameHasPadding(curHeader_) && bufLen == 1) {
      // We need to wait for more bytes otherwise we won't be able to pass
      // the correct padding to the first onBody call
      return ErrorCode::NO_ERROR;
    }
    const auto ret = http2::parseDataBegin(cursor, curHeader_, parsed, padding);
    RETURN_IF_ERROR(ret);
    if (padding > 0) {
      pendingDataFramePaddingBytes_ = padding - 1;
      pendingDataFrameBytes_--;
      bufLen--;
      parsed++;
    }
    VLOG(10)
        << "out padding=" << padding
        << " pendingDataFrameBytes_=" << pendingDataFrameBytes_
        << " pendingDataFramePaddingBytes_=" << pendingDataFramePaddingBytes_
        << " bufLen=" << bufLen << " parsed=" << parsed;
  }
  if (bufLen > 0) {
    // Check if we have application data to parse
    if (pendingDataFrameBytes_ > pendingDataFramePaddingBytes_) {
      const size_t pendingAppData =
          pendingDataFrameBytes_ - pendingDataFramePaddingBytes_;
      const size_t toClone = std::min(pendingAppData, bufLen);
      cursor.clone(outData, toClone);
      bufLen -= toClone;
      pendingDataFrameBytes_ -= toClone;
      parsed += toClone;
      VLOG(10) << "parsed some app data, pendingDataFrameBytes_="
               << pendingDataFrameBytes_ << " pendingDataFramePaddingBytes_="
               << pendingDataFramePaddingBytes_ << " bufLen=" << bufLen
               << " parsed=" << parsed;
    }
    // Check if we have padding bytes to parse
    if (bufLen > 0 && pendingDataFramePaddingBytes_ > 0) {
      size_t toSkip = 0;
      auto ret = http2::parseDataEnd(
          cursor, bufLen, pendingDataFramePaddingBytes_, toSkip);
      RETURN_IF_ERROR(ret);
      pendingDataFrameBytes_ -= toSkip;
      pendingDataFramePaddingBytes_ -= toSkip;
      parsed += toSkip;
      VLOG(10) << "parsed some padding, pendingDataFrameBytes_="
               << pendingDataFrameBytes_ << " pendingDataFramePaddingBytes_="
               << pendingDataFramePaddingBytes_ << " bufLen=" << bufLen
               << " parsed=" << parsed;
    }
  }

  if (callback_ && (padding > 0 || (outData && !outData->empty()))) {
    if (!outData) {
      outData = std::make_unique<IOBuf>();
    }
    deliverCallbackIfAllowed(&HTTPCodec::Callback::onBody,
                             "onBody",
                             curHeader_.stream,
                             std::move(outData),
                             padding);
  }
  return (pendingDataFrameBytes_ > 0) ? ErrorCode::NO_ERROR : handleEndStream();
}

ErrorCode HTTP2Codec::parseHeaders(Cursor& cursor) {
  FOLLY_SCOPED_TRACE_SECTION("HTTP2Codec - parseHeaders");
  folly::Optional<http2::PriorityUpdate> priority;
  std::unique_ptr<IOBuf> headerBuf;
  VLOG(4) << "parsing HEADERS frame for stream=" << curHeader_.stream
          << " length=" << curHeader_.length;
  auto err = http2::parseHeaders(cursor, curHeader_, priority, headerBuf);
  RETURN_IF_ERROR(err);
  if (transportDirection_ == TransportDirection::DOWNSTREAM) {
    RETURN_IF_ERROR(
        checkNewStream(curHeader_.stream, true /* trailersAllowed */));
  }
  err = parseHeadersImpl(
      cursor, std::move(headerBuf), priority, folly::none, folly::none);
  return err;
}

ErrorCode HTTP2Codec::parseExHeaders(Cursor& cursor) {
  FOLLY_SCOPED_TRACE_SECTION("HTTP2Codec - parseExHeaders");
  HTTPCodec::ExAttributes exAttributes;
  folly::Optional<http2::PriorityUpdate> priority;
  std::unique_ptr<IOBuf> headerBuf;
  VLOG(4) << "parsing ExHEADERS frame for stream=" << curHeader_.stream
          << " length=" << curHeader_.length;
  auto err = http2::parseExHeaders(
      cursor, curHeader_, exAttributes, priority, headerBuf);
  RETURN_IF_ERROR(err);
  if (isRequest(curHeader_.stream)) {
    RETURN_IF_ERROR(
        checkNewStream(curHeader_.stream, false /* trailersAllowed */));
  }
  return parseHeadersImpl(
      cursor, std::move(headerBuf), priority, folly::none, exAttributes);
}

ErrorCode HTTP2Codec::parseContinuation(Cursor& cursor) {
  std::unique_ptr<IOBuf> headerBuf;
  VLOG(4) << "parsing CONTINUATION frame for stream=" << curHeader_.stream
          << " length=" << curHeader_.length;
  auto err = http2::parseContinuation(cursor, curHeader_, headerBuf);
  RETURN_IF_ERROR(err);
  err = parseHeadersImpl(
      cursor, std::move(headerBuf), folly::none, folly::none, folly::none);
  return err;
}

ErrorCode HTTP2Codec::parseHeadersImpl(
    Cursor& /*cursor*/,
    std::unique_ptr<IOBuf> headerBuf,
    const folly::Optional<http2::PriorityUpdate>& priority,
    const folly::Optional<uint32_t>& promisedStream,
    const folly::Optional<ExAttributes>& exAttributes) {
  curHeaderBlock_.append(std::move(headerBuf));
  std::unique_ptr<HTTPMessage> msg;
  uint32_t headersCompleteStream = curHeader_.stream;

  // if we're not parsing CONTINUATION, then it's start of new header block
  if (curHeader_.type != http2::FrameType::CONTINUATION) {
    headerBlockFrameType_ = curHeader_.type;
    if (promisedStream) {
      parsingReq_ = true;
    } else if (exAttributes) {
      parsingReq_ = isRequest(curHeader_.stream);
    } else {
      parsingReq_ = transportDirection_ == TransportDirection::DOWNSTREAM;
    }
  } else if (headerBlockFrameType_ == http2::FrameType::PUSH_PROMISE) {
    CHECK(promisedStream_.hasValue());
    headersCompleteStream = *promisedStream_;
  }

  DeferredParseError parseError;
  if (curHeader_.flags & http2::END_HEADERS) {
    auto parseRes = parseHeadersDecodeFrames(priority, exAttributes);
    if (parseRes.hasError()) {
      parseError = std::move(parseRes.error());
      if (parseError.connectionError) {
        return parseError.errorCode;
      }
    } else {
      msg = std::move(*parseRes);
    }
  }

  // Report back what we've parsed
  auto concurError = parseHeadersCheckConcurrentStreams(priority);
  if (concurError.has_value()) {
    return concurError.value();
  }

  bool trailers = parsingTrailers();
  bool allHeaderFramesReceived =
      (curHeader_.flags & http2::END_HEADERS) &&
      (headerBlockFrameType_ == http2::FrameType::HEADERS);
  if (allHeaderFramesReceived && !trailers) {
    // Only deliver onMessageBegin once per stream.
    // For responses with CONTINUATION, this will be delayed until
    // the frame with the END_HEADERS flag set.
    deliverCallbackIfAllowed(&HTTPCodec::Callback::onMessageBegin,
                             "onMessageBegin",
                             curHeader_.stream,
                             msg.get());
  } else if (curHeader_.type == http2::FrameType::EX_HEADERS) {
    deliverCallbackIfAllowed(&HTTPCodec::Callback::onExMessageBegin,
                             "onExMessageBegin",
                             curHeader_.stream,
                             exAttributes->controlStream,
                             exAttributes->unidirectional,
                             msg.get());
  } else if (curHeader_.type == http2::FrameType::PUSH_PROMISE) {
    DCHECK(promisedStream);
    deliverCallbackIfAllowed(&HTTPCodec::Callback::onPushMessageBegin,
                             "onPushMessageBegin",
                             *promisedStream,
                             curHeader_.stream,
                             msg.get());
    promisedStream_ = *promisedStream;
    headersCompleteStream = *promisedStream;
  }

  if (curHeader_.flags & http2::END_HEADERS) {
    if (!msg) {
      deliverDeferredParseError(parseError);
      return ErrorCode::NO_ERROR;
    }
    if (!(curHeader_.flags & http2::END_STREAM)) {
      // If it there are DATA frames coming, consider it chunked
      msg->setIsChunked(true);
    }
    if (trailers) {
      VLOG(4) << "Trailers complete for streamId=" << headersCompleteStream
              << " direction=" << transportDirection_;
      auto trailerHeaders =
          std::make_unique<HTTPHeaders>(msg->extractHeaders());
      msg.reset();
      deliverCallbackIfAllowed(&HTTPCodec::Callback::onTrailersComplete,
                               "onTrailersComplete",
                               headersCompleteStream,
                               std::move(trailerHeaders));
    } else {
      if (transportDirection_ == TransportDirection::UPSTREAM &&
          curHeader_.stream & 0x01 &&
          curHeader_.stream >= nextEgressStreamID_) {
        goawayErrorMessage_ = folly::to<std::string>(
            "HEADERS on idle upstream stream=", curHeader_.stream);
        LOG(ERROR) << goawayErrorMessage_;
        return ErrorCode::PROTOCOL_ERROR;
      }
      deliverCallbackIfAllowed(&HTTPCodec::Callback::onHeadersComplete,
                               "onHeadersComplete",
                               headersCompleteStream,
                               std::move(msg));
      promisedStream_ = folly::none;
    }
  }
  return handleEndStream();
}

folly::Expected<std::unique_ptr<HTTPMessage>, HTTP2Codec::DeferredParseError>
HTTP2Codec::parseHeadersDecodeFrames(
    const folly::Optional<http2::PriorityUpdate>& priority,
    const folly::Optional<ExAttributes>& exAttributes) {
  // decompress headers
  Cursor headerCursor(curHeaderBlock_.front());

  // DO NOT return from this method until after the call to decodeStreaming
  // unless you return a connection error.  Otherwise the HPACK state will
  // get messed up.
  decodeInfo_.init(parsingReq_,
                   parsingDownstreamTrailers_,
                   validateHeaders_,
                   strictValidation_,
                   exAttributes && exAttributes->controlStream != 0);
  if (priority) {
    decodeInfo_.msg->setHTTP2Priority(std::make_tuple(
        priority->streamDependency, priority->exclusive, priority->weight));
  }

  headerCodec_.decodeStreaming(
      headerCursor, curHeaderBlock_.chainLength(), this);
  auto msg = std::move(decodeInfo_.msg);
  // Saving this in case we need to log it on error
  auto g = folly::makeGuard([this] { curHeaderBlock_.move(); });
  // Check decoding error
  if (decodeInfo_.decodeError != HPACK::DecodeError::NONE) {
    static const std::string decodeErrorMessage =
        "Failed decoding header block for stream=";
    // Avoid logging header blocks that have failed decoding due to being
    // excessively large.
    if (decodeInfo_.decodeError != HPACK::DecodeError::HEADERS_TOO_LARGE) {
      goawayErrorMessage_ =
          folly::to<std::string>(decodeErrorMessage,
                                 curHeader_.stream,
                                 ": decompression error=",
                                 uint32_t(decodeInfo_.decodeError));
      LOG(ERROR) << goawayErrorMessage_
                 << (VLOG_IS_ON(3) ? ", header block=" : "");
      VLOG(3) << IOBufPrinter::printHexFolly(curHeaderBlock_.front(), true);
    } else {
      goawayErrorMessage_ = folly::to<std::string>(
          decodeErrorMessage, curHeader_.stream, ": headers too large");
    }

    if (msg) {
      // print the partial message
      msg->dumpMessage(3);
    }
    return folly::makeUnexpected(DeferredParseError(
        ErrorCode::COMPRESSION_ERROR, true, empty_string, std::move(msg)));
  }

  // Validate circular dependencies.
  if (priority && (curHeader_.stream == priority->streamDependency)) {
    return folly::makeUnexpected(DeferredParseError(
        ErrorCode::PROTOCOL_ERROR,
        false,
        folly::to<string>("Circular dependency for txn=", curHeader_.stream)));
  }

  // Check parsing error
  if (!decodeInfo_.parsingError.empty()) {
    // This is "malformed" per the RFC
    LOG(ERROR) << "Failed parsing header list for stream=" << curHeader_.stream
               << ", error=" << decodeInfo_.parsingError;
    if (!decodeInfo_.headerErrorValue.empty()) {
      std::cerr << " value=" << decodeInfo_.headerErrorValue << std::endl;
    }
    VLOG(3) << "Header block="
            << IOBufPrinter::printHexFolly(curHeaderBlock_.front(), true);
    if (transportDirection_ == TransportDirection::DOWNSTREAM &&
        parsingHeaders() && !parsingTrailers()) {
      return folly::makeUnexpected(
          DeferredParseError(ErrorCode::NO_ERROR,
                             false,
                             folly::to<std::string>("HTTP2Codec stream error: ",
                                                    "stream=",
                                                    curHeader_.stream,
                                                    " status=",
                                                    400,
                                                    " error: ",
                                                    decodeInfo_.parsingError),
                             std::move(msg)));
    } else {
      // Upstream, PUSH_PROMISE, EX_HEADERS or trailers parsing failed:
      // PROTOCOL_ERROR
      return folly::makeUnexpected(DeferredParseError(
          ErrorCode::PROTOCOL_ERROR,
          false,
          folly::to<string>("Field section parsing failed txn=",
                            curHeader_.stream),
          std::move(msg)));
    }
  }

  return msg;
}

void HTTP2Codec::deliverDeferredParseError(
    const DeferredParseError& parseError) {
  CHECK(!parseError.connectionError);
  if (parseError.errorCode != ErrorCode::NO_ERROR) {
    streamError(parseError.errorMessage,
                parseError.errorCode,
                parsingHeaders(),
                folly::none,
                std::move(parseError.partialMessage));
    if (promisedStream_) {
      streamError("Malformed PUSH_PROMISE",
                  ErrorCode::REFUSED_STREAM,
                  false,
                  *promisedStream_);
      promisedStream_ = folly::none;
    }
  } else {
    HTTPException err(HTTPException::Direction::INGRESS,
                      parseError.errorMessage);
    err.setHttpStatusCode(400);
    err.setProxygenError(kErrorParseHeader);
    err.setPartialMsg(std::move(parseError.partialMessage));
    deliverCallbackIfAllowed(&HTTPCodec::Callback::onError,
                             "onError",
                             curHeader_.stream,
                             err,
                             parsingHeaders());
  }
}

folly::Optional<ErrorCode> HTTP2Codec::parseHeadersCheckConcurrentStreams(
    const folly::Optional<http2::PriorityUpdate>& priority) {
  if (!isInitiatedStream(curHeader_.stream) &&
      (curHeader_.type == http2::FrameType::HEADERS ||
       curHeader_.type == http2::FrameType::EX_HEADERS)) {
    if (curHeader_.flags & http2::PRIORITY) {
      DCHECK(priority);
      // callback_->onPriority(priority.get());
    }

    // callback checks total number of streams is smaller than settings max
    if (callback_ &&
        callback_->numIncomingStreams() >=
            egressSettings_.getSetting(SettingsId::MAX_CONCURRENT_STREAMS,
                                       std::numeric_limits<int32_t>::max())) {
      streamError(folly::to<string>("Exceeded max_concurrent_streams"),
                  ErrorCode::REFUSED_STREAM,
                  true);
      return ErrorCode::NO_ERROR;
    }
  }
  return folly::none;
}

void HTTP2Codec::onHeader(const HPACKHeaderName& name,
                          const folly::fbstring& value) {
  if (decodeInfo_.onHeader(name, value)) {
    if (userAgent_.empty() && name.getHeaderCode() == HTTP_HEADER_USER_AGENT) {
      userAgent_ = value.toStdString();
    }
  } else {
    VLOG(4) << "dir=" << uint32_t(transportDirection_)
            << decodeInfo_.parsingError << " codec=" << headerCodec_;
  }
}

void HTTP2Codec::onHeadersComplete(HTTPHeaderSize decodedSize,
                                   bool /*acknowledge*/) {
  decodeInfo_.onHeadersComplete(decodedSize);
  decodeInfo_.msg->setAdvancedProtocolString(http2::kProtocolString);

  HTTPMessage* msg = decodeInfo_.msg.get();
  HTTPRequestVerifier& verifier = decodeInfo_.verifier;
  if ((transportDirection_ == TransportDirection::DOWNSTREAM) &&
      verifier.hasUpgradeProtocol() &&
      (*msg->getUpgradeProtocol() == headers::kWebsocketString) &&
      msg->getMethod() == HTTPMethod::CONNECT) {
    msg->setIngressWebsocketUpgrade();
    ingressWebsocketUpgrade_ = true;
  } else if (!upgradedStreams_.empty()) {
    auto it = upgradedStreams_.find(curHeader_.stream);
    if (it != upgradedStreams_.end()) {
      upgradedStreams_.erase(curHeader_.stream);
      // a websocket upgrade was sent on this stream.
      if (msg->getStatusCode() != 200) {
        return;
      }
      msg->setIngressWebsocketUpgrade();
    }
  }
}

void HTTP2Codec::onDecodeError(HPACK::DecodeError decodeError) {
  decodeInfo_.decodeError = decodeError;
}

ErrorCode HTTP2Codec::parsePriority(Cursor& cursor) {
  VLOG(4) << "parsing PRIORITY frame for stream=" << curHeader_.stream
          << " length=" << curHeader_.length;
  http2::PriorityUpdate pri;
  auto err = http2::parsePriority(cursor, curHeader_, pri);
  RETURN_IF_ERROR(err);
  if (curHeader_.stream == pri.streamDependency) {
    streamError(
        folly::to<string>("Circular dependency for txn=", curHeader_.stream),
        ErrorCode::PROTOCOL_ERROR,
        false);
    return ErrorCode::NO_ERROR;
  }
  // Now we have two onPriority overloads, this function pointer has to be
  // explicitly specified via a cast:
  auto onPriFunc = static_cast<void (HTTPCodec::Callback::*)(
      StreamID, const HTTPMessage::HTTP2Priority&)>(
      &HTTPCodec::Callback::onPriority);
  deliverCallbackIfAllowed(
      onPriFunc,
      "onPriority",
      curHeader_.stream,
      std::make_tuple(pri.streamDependency, pri.exclusive, pri.weight));
  return ErrorCode::NO_ERROR;
}

size_t HTTP2Codec::addPriorityNodes(PriorityQueue& queue,
                                    folly::IOBufQueue& writeBuf,
                                    uint8_t maxLevel) {
  HTTPCodec::StreamID parent = 0;
  size_t bytes = 0;
  while (maxLevel--) {
    auto id = createStream();
    virtualPriorityNodes_.push_back(id);
    queue.addPriorityNode(id, parent);
    bytes += generatePriority(writeBuf, id, std::make_tuple(parent, false, 0));
    parent = id;
  }
  return bytes;
}

ErrorCode HTTP2Codec::parseRstStream(Cursor& cursor) {
  // rst for stream in idle state - protocol error
  VLOG(4) << "parsing RST_STREAM frame for stream=" << curHeader_.stream
          << " length=" << curHeader_.length;
  upgradedStreams_.erase(curHeader_.stream);
  ErrorCode statusCode = ErrorCode::NO_ERROR;
  auto err = http2::parseRstStream(cursor, curHeader_, statusCode);
  RETURN_IF_ERROR(err);
  if (statusCode == ErrorCode::PROTOCOL_ERROR) {
    VLOG(3) << "RST_STREAM with code=" << getErrorCodeString(statusCode)
            << " for streamID=" << curHeader_.stream
            << " user-agent=" << userAgent_;
  }
  deliverCallbackIfAllowed(
      &HTTPCodec::Callback::onAbort, "onAbort", curHeader_.stream, statusCode);
  return ErrorCode::NO_ERROR;
}

ErrorCode HTTP2Codec::parseSettings(Cursor& cursor) {
  VLOG(4) << "parsing SETTINGS frame for stream=" << curHeader_.stream
          << " length=" << curHeader_.length;
  std::deque<SettingPair> settings;
  auto err = http2::parseSettings(cursor, curHeader_, settings);
  RETURN_IF_ERROR(err);
  if (curHeader_.flags & http2::ACK) {
    handleSettingsAck();
    return ErrorCode::NO_ERROR;
  }
  return handleSettings(settings);
}

void HTTP2Codec::handleSettingsAck() {
  if (pendingTableMaxSize_) {
    headerCodec_.setDecoderHeaderTableMaxSize(*pendingTableMaxSize_);
    pendingTableMaxSize_ = folly::none;
  }
  if (callback_) {
    callback_->onSettingsAck();
  }
}

ErrorCode HTTP2Codec::handleSettings(const std::deque<SettingPair>& settings) {
  SettingsList settingsList;
  for (auto& setting : settings) {
    switch (setting.first) {
      case SettingsId::HEADER_TABLE_SIZE: {
        uint32_t tableSize = setting.second;
        if (setting.second > http2::kMaxHeaderTableSize) {
          VLOG(2) << "Limiting table size from " << tableSize << " to "
                  << http2::kMaxHeaderTableSize;
          tableSize = http2::kMaxHeaderTableSize;
        }
        headerCodec_.setEncoderHeaderTableSize(tableSize);
      } break;
      case SettingsId::ENABLE_PUSH:
        if ((setting.second != 0 && setting.second != 1) ||
            (setting.second == 1 &&
             transportDirection_ == TransportDirection::UPSTREAM)) {
          goawayErrorMessage_ =
              folly::to<string>("GOAWAY error: ENABLE_PUSH invalid setting=",
                                setting.second,
                                " for streamID=",
                                curHeader_.stream);
          VLOG(4) << goawayErrorMessage_;
          return ErrorCode::PROTOCOL_ERROR;
        }
        break;
      case SettingsId::MAX_CONCURRENT_STREAMS:
        break;
      case SettingsId::INITIAL_WINDOW_SIZE:
        if (setting.second > http2::kMaxWindowUpdateSize) {
          goawayErrorMessage_ = folly::to<string>(
              "GOAWAY error: INITIAL_WINDOW_SIZE invalid size=",
              setting.second,
              " for streamID=",
              curHeader_.stream);
          VLOG(4) << goawayErrorMessage_;
          return ErrorCode::PROTOCOL_ERROR;
        }
        break;
      case SettingsId::MAX_FRAME_SIZE:
        if (setting.second < http2::kMaxFramePayloadLengthMin ||
            setting.second > http2::kMaxFramePayloadLength) {
          goawayErrorMessage_ =
              folly::to<string>("GOAWAY error: MAX_FRAME_SIZE invalid size=",
                                setting.second,
                                " for streamID=",
                                curHeader_.stream);
          VLOG(4) << goawayErrorMessage_;
          return ErrorCode::PROTOCOL_ERROR;
        }
        ingressSettings_.setSetting(SettingsId::MAX_FRAME_SIZE, setting.second);
        break;
      case SettingsId::MAX_HEADER_LIST_SIZE:
        break;
      case SettingsId::ENABLE_EX_HEADERS: {
        auto ptr = egressSettings_.getSetting(SettingsId::ENABLE_EX_HEADERS);
        if (ptr && ptr->value > 0) {
          VLOG(4) << getTransportDirectionString(getTransportDirection())
                  << " got ENABLE_EX_HEADERS=" << setting.second;
          if (setting.second != 0 && setting.second != 1) {
            goawayErrorMessage_ =
                folly::to<string>("GOAWAY error: invalid ENABLE_EX_HEADERS=",
                                  setting.second,
                                  " for streamID=",
                                  curHeader_.stream);
            VLOG(4) << goawayErrorMessage_;
            return ErrorCode::PROTOCOL_ERROR;
          }
          break;
        } else {
          // egress ENABLE_EX_HEADERS is disabled, consider the ingress
          // ENABLE_EX_HEADERS as unknown setting, and ignore it.
          continue;
        }
      }
      case SettingsId::ENABLE_CONNECT_PROTOCOL:
        if (setting.second > 1) {
          goawayErrorMessage_ = folly::to<string>(
              "GOAWAY error: ENABLE_CONNECT_PROTOCOL invalid number=",
              setting.second,
              " for streamID=",
              curHeader_.stream);
          VLOG(4) << goawayErrorMessage_;
          return ErrorCode::PROTOCOL_ERROR;
        }
        break;
      case SettingsId::THRIFT_CHANNEL_ID:
      case SettingsId::THRIFT_CHANNEL_ID_DEPRECATED:
        break;
      case SettingsId::SETTINGS_HTTP_CERT_AUTH:
        break;
      default:
        continue; // ignore unknown setting
    }
    ingressSettings_.setSetting(setting.first, setting.second);
    settingsList.push_back(*ingressSettings_.getSetting(setting.first));
  }
  if (callback_) {
    callback_->onSettings(settingsList);
  }
  return ErrorCode::NO_ERROR;
}

ErrorCode HTTP2Codec::parsePushPromise(Cursor& cursor) {
  // stream id must be idle - protocol error
  // assoc-stream-id=closed/unknown - protocol error, unless rst_stream sent

  /*
   * What does "must handle" mean in the following context?  I have to
   * accept this as a valid pushed resource?

    However, an endpoint that has sent RST_STREAM on the associated
    stream MUST handle PUSH_PROMISE frames that might have been
    created before the RST_STREAM frame is received and processed.
  */
  if (transportDirection_ != TransportDirection::UPSTREAM) {
    goawayErrorMessage_ =
        folly::to<string>("Received PUSH_PROMISE on DOWNSTREAM codec");
    VLOG(2) << goawayErrorMessage_;
    return ErrorCode::PROTOCOL_ERROR;
  }
  if (egressSettings_.getSetting(SettingsId::ENABLE_PUSH, -1) != 1) {
    goawayErrorMessage_ =
        folly::to<string>("Received PUSH_PROMISE on codec with push disabled");
    VLOG(2) << goawayErrorMessage_;
    return ErrorCode::PROTOCOL_ERROR;
  }
  VLOG(4) << "parsing PUSH_PROMISE frame for stream=" << curHeader_.stream
          << " length=" << curHeader_.length;
  uint32_t promisedStream;
  std::unique_ptr<IOBuf> headerBlockFragment;
  auto err = http2::parsePushPromise(
      cursor, curHeader_, promisedStream, headerBlockFragment);
  RETURN_IF_ERROR(err);
  RETURN_IF_ERROR(checkNewStream(promisedStream, false /* trailersAllowed */));
  err = parseHeadersImpl(cursor,
                         std::move(headerBlockFragment),
                         folly::none,
                         promisedStream,
                         folly::none);
  return err;
}

ErrorCode HTTP2Codec::parsePing(Cursor& cursor) {
  VLOG(4) << "parsing PING frame length=" << curHeader_.length;
  uint64_t opaqueData = 0;
  auto err = http2::parsePing(cursor, curHeader_, opaqueData);
  RETURN_IF_ERROR(err);
  if (callback_) {
    if (curHeader_.flags & http2::ACK) {
      callback_->onPingReply(opaqueData);
    } else {
      callback_->onPingRequest(opaqueData);
    }
  }
  return ErrorCode::NO_ERROR;
}

ErrorCode HTTP2Codec::parseGoaway(Cursor& cursor) {
  VLOG(4) << "parsing GOAWAY frame length=" << curHeader_.length;
  uint32_t lastGoodStream = 0;
  ErrorCode statusCode = ErrorCode::NO_ERROR;
  std::unique_ptr<IOBuf> debugData;

  auto err = http2::parseGoaway(
      cursor, curHeader_, lastGoodStream, statusCode, debugData);
  if (statusCode != ErrorCode::NO_ERROR) {
    VLOG(3)
        << "Goaway error statusCode=" << getErrorCodeString(statusCode)
        << " lastStream=" << lastGoodStream << " user-agent=" << userAgent_
        << " debugData="
        << ((debugData) ? string((char*)debugData->data(), debugData->length())
                        : empty_string);
  }
  RETURN_IF_ERROR(err);
  if (lastGoodStream < ingressGoawayAck_) {
    ingressGoawayAck_ = lastGoodStream;
    // Drain all streams <= lastGoodStream
    // and abort streams > lastGoodStream
    if (callback_) {
      callback_->onGoaway(lastGoodStream, statusCode, std::move(debugData));
    }
  } else {
    LOG(WARNING) << "Received multiple GOAWAY with increasing ack";
  }
  return ErrorCode::NO_ERROR;
}

ErrorCode HTTP2Codec::parseWindowUpdate(Cursor& cursor) {
  VLOG(4) << "parsing WINDOW_UPDATE frame for stream=" << curHeader_.stream
          << " length=" << curHeader_.length;
  uint32_t delta = 0;
  auto err = http2::parseWindowUpdate(cursor, curHeader_, delta);
  RETURN_IF_ERROR(err);
  if (delta == 0) {
    VLOG(4) << "Invalid 0 length delta for stream=" << curHeader_.stream;
    if (curHeader_.stream == 0) {
      goawayErrorMessage_ = folly::to<string>(
          "GOAWAY error: invalid/0 length delta for streamID=",
          curHeader_.stream);
      return ErrorCode::PROTOCOL_ERROR;
    } else {
      // Parsing a zero delta window update should cause a protocol error
      // and send a rst stream
      goawayErrorMessage_ = folly::to<std::string>(
          "streamID=", curHeader_.stream, " with window update delta=", delta);
      VLOG(4) << goawayErrorMessage_;
      streamError(goawayErrorMessage_, ErrorCode::PROTOCOL_ERROR);
      // Stream error and protocol error
      return ErrorCode::PROTOCOL_ERROR;
    }
  }
  // if window exceeds 2^31-1, connection/stream error flow control error
  // must be checked in session/txn
  deliverCallbackIfAllowed(&HTTPCodec::Callback::onWindowUpdate,
                           "onWindowUpdate",
                           curHeader_.stream,
                           delta);
  return ErrorCode::NO_ERROR;
}

ErrorCode HTTP2Codec::parseCertificateRequest(Cursor& cursor) {
  VLOG(4) << "parsing CERTIFICATE_REQUEST frame length=" << curHeader_.length;
  uint16_t requestId = 0;
  std::unique_ptr<IOBuf> authRequest;

  auto err = http2::parseCertificateRequest(
      cursor, curHeader_, requestId, authRequest);
  RETURN_IF_ERROR(err);
  if (callback_) {
    callback_->onCertificateRequest(requestId, std::move(authRequest));
  }
  return ErrorCode::NO_ERROR;
}

ErrorCode HTTP2Codec::parseCertificate(Cursor& cursor) {
  VLOG(4) << "parsing CERTIFICATE frame length=" << curHeader_.length;
  uint16_t certId = 0;
  std::unique_ptr<IOBuf> authData;
  auto err = http2::parseCertificate(cursor, curHeader_, certId, authData);
  RETURN_IF_ERROR(err);
  if (curAuthenticatorBlock_.empty()) {
    curCertId_ = certId;
  } else if (certId != curCertId_) {
    // Received CERTIFICATE frame with different Cert-ID.
    return ErrorCode::PROTOCOL_ERROR;
  }
  curAuthenticatorBlock_.append(std::move(authData));
  if (curAuthenticatorBlock_.chainLength() > http2::kMaxAuthenticatorBufSize) {
    // Received excessively long authenticator.
    return ErrorCode::PROTOCOL_ERROR;
  }
  if (!(curHeader_.flags & http2::TO_BE_CONTINUED)) {
    auto authenticator = curAuthenticatorBlock_.move();
    if (callback_) {
      callback_->onCertificate(certId, std::move(authenticator));
    } else {
      curAuthenticatorBlock_.reset();
    }
  }
  return ErrorCode::NO_ERROR;
}

ErrorCode HTTP2Codec::checkNewStream(uint32_t streamId, bool trailersAllowed) {
  bool existingStream = (streamId <= lastStreamID_);
  if (streamId == 0 || (!trailersAllowed && existingStream)) {
    goawayErrorMessage_ =
        folly::to<string>("GOAWAY error: received streamID=",
                          streamId,
                          " as invalid new stream for lastStreamID_=",
                          lastStreamID_);
    VLOG(4) << goawayErrorMessage_;
    return ErrorCode::PROTOCOL_ERROR;
  }
  parsingDownstreamTrailers_ = trailersAllowed && existingStream;
  if (parsingDownstreamTrailers_) {
    VLOG(4) << "Parsing downstream trailers streamId=" << streamId;
  }

  if (sessionClosing_ != ClosingState::CLOSED && !existingStream) {
    lastStreamID_ = streamId;
  }

  if (isInitiatedStream(streamId)) {
    // this stream should be initiated by us, not by peer
    goawayErrorMessage_ = folly::to<string>(
        "GOAWAY error: invalid new stream received with streamID=", streamId);
    VLOG(4) << goawayErrorMessage_;
    return ErrorCode::PROTOCOL_ERROR;
  } else {
    return ErrorCode::NO_ERROR;
  }
}

size_t HTTP2Codec::generateConnectionPreface(folly::IOBufQueue& writeBuf) {
  if (transportDirection_ == TransportDirection::UPSTREAM) {
    VLOG(4) << "generating connection preface";
    writeBuf.append(http2::kConnectionPreface);
    return http2::kConnectionPreface.length();
  }
  return 0;
}

bool HTTP2Codec::onIngressUpgradeMessage(const HTTPMessage& msg) {
  if (!HTTPParallelCodec::onIngressUpgradeMessage(msg)) {
    return false;
  }
  if (msg.getHeaders().getNumberOfValues(http2::kProtocolSettingsHeader) != 1) {
    VLOG(4) << __func__ << " with no HTTP2-Settings";
    return false;
  }

  const auto& settingsHeader =
      msg.getHeaders().getSingleOrEmpty(http2::kProtocolSettingsHeader);
  if (settingsHeader.empty()) {
    return true;
  }

  auto decoded = folly::makeTryWith([&settingsHeader] {
                   return folly::base64URLDecode(settingsHeader);
                 }).value_or(std::string());

  // Must be well formed Base64Url and not too large
  if (decoded.empty() || decoded.length() > http2::kMaxFramePayloadLength) {
    VLOG(4) << __func__ << " failed to decode HTTP2-Settings";
    return false;
  }
  std::unique_ptr<IOBuf> decodedBuf =
      IOBuf::wrapBuffer(decoded.data(), decoded.length());
  IOBufQueue settingsQueue{IOBufQueue::cacheChainLength()};
  settingsQueue.append(std::move(decodedBuf));
  Cursor c(settingsQueue.front());
  std::deque<SettingPair> settings;
  // downcast is ok because of above length check
  http2::FrameHeader frameHeader{(uint32_t)settingsQueue.chainLength(),
                                 0,
                                 http2::FrameType::SETTINGS,
                                 0,
                                 0};
  auto err = http2::parseSettings(c, frameHeader, settings);
  if (err != ErrorCode::NO_ERROR) {
    VLOG(4) << __func__ << " bad settings frame";
    return false;
  }

  if (handleSettings(settings) != ErrorCode::NO_ERROR) {
    VLOG(4) << __func__ << " handleSettings failed";
    return false;
  }

  return true;
}

void HTTP2Codec::generateHeader(
    folly::IOBufQueue& writeBuf,
    StreamID stream,
    const HTTPMessage& msg,
    bool eom,
    HTTPHeaderSize* size,
    const folly::Optional<HTTPHeaders>& extraHeaders) {
  generateHeaderImpl(writeBuf,
                     stream,
                     msg,
                     folly::none, /* assocStream */
                     folly::none, /* controlStream */
                     eom,
                     size,
                     extraHeaders);
}

void HTTP2Codec::generatePushPromise(folly::IOBufQueue& writeBuf,
                                     StreamID stream,
                                     const HTTPMessage& msg,
                                     StreamID assocStream,
                                     bool eom,
                                     HTTPHeaderSize* size) {
  generateHeaderImpl(writeBuf,
                     stream,
                     msg,
                     assocStream,
                     folly::none, /* controlStream */
                     eom,
                     size,
                     folly::none /* extraHeaders */);
}

void HTTP2Codec::generateExHeader(folly::IOBufQueue& writeBuf,
                                  StreamID stream,
                                  const HTTPMessage& msg,
                                  const HTTPCodec::ExAttributes& exAttributes,
                                  bool eom,
                                  HTTPHeaderSize* size) {
  generateHeaderImpl(writeBuf,
                     stream,
                     msg,
                     folly::none, /* assocStream */
                     exAttributes,
                     eom,
                     size,
                     folly::none /* extraHeaders */);
}

size_t HTTP2Codec::splitCompressed(size_t compressed,
                                   uint32_t remainingFrameSize,
                                   folly::IOBufQueue& writeBuf,
                                   folly::IOBufQueue& queue) {
  CHECK_GT(compressed, 0) << "compressed block must be at least 1 byte";
  auto chunkLen = compressed;
  if (chunkLen > remainingFrameSize) {
    // There's more here than fits in one frame.  Put the remainder in queue
    chunkLen = remainingFrameSize;
    auto tailSize = compressed - remainingFrameSize;
    auto head = writeBuf.split(writeBuf.chainLength() - tailSize);
    queue.append(writeBuf.move());
    writeBuf.append(std::move(head));
  }
  return chunkLen;
}

void HTTP2Codec::generateHeaderImpl(
    folly::IOBufQueue& writeBuf,
    StreamID stream,
    const HTTPMessage& msg,
    const folly::Optional<StreamID>& assocStream,
    const folly::Optional<HTTPCodec::ExAttributes>& exAttributes,
    bool eom,
    HTTPHeaderSize* size,
    const folly::Optional<HTTPHeaders>& extraHeaders) {
  HTTPHeaderSize localSize;
  if (!size) {
    size = &localSize;
  }
  if (assocStream) {
    CHECK(!exAttributes);
    VLOG(4) << "generating PUSH_PROMISE for stream=" << stream;
  } else if (exAttributes) {
    CHECK(!assocStream);
    VLOG(4) << "generating ExHEADERS for stream=" << stream
            << " with control stream=" << exAttributes->controlStream
            << " unidirectional=" << exAttributes->unidirectional;
  } else {
    VLOG(4) << "generating HEADERS for stream=" << stream;
  }

  if (!isStreamIngressEgressAllowed(stream)) {
    VLOG(2) << "Suppressing HEADERS/PROMISE for stream=" << stream
            << " ingressGoawayAck_=" << ingressGoawayAck_;
    if (size) {
      size->uncompressed = 0;
      size->compressed = 0;
    }
    return;
  }

  if (msg.isRequest()) {
    DCHECK(transportDirection_ == TransportDirection::UPSTREAM || assocStream ||
           exAttributes);
    if (msg.isEgressWebsocketUpgrade()) {
      upgradedStreams_.insert(stream);
    }
  } else {
    DCHECK(transportDirection_ == TransportDirection::DOWNSTREAM ||
           exAttributes);
  }

  auto httpPri = msg.getHTTP2Priority();
  folly::Optional<http2::PriorityUpdate> pri;
  if (httpPri) {
    pri = http2::PriorityUpdate{
        std::get<0>(*httpPri), std::get<1>(*httpPri), std::get<2>(*httpPri)};
    if (pri->streamDependency == stream) {
      LOG(ERROR) << "Overwriting circular dependency for stream=" << stream;
      pri = http2::DefaultPriority;
    }
  }
  auto headerSize = http2::calculatePreHeaderBlockSize(assocStream.has_value(),
                                                       exAttributes.has_value(),
                                                       pri.has_value(),
                                                       false);
  auto maxFrameSize = maxSendFrameSize();
  uint32_t remainingFrameSize =
      maxFrameSize - headerSize + http2::kFrameHeaderSize;
  auto frameHeader = writeBuf.preallocate(headerSize, kDefaultGrowth);
  writeBuf.postallocate(headerSize);
  headerCodec_.encodeHTTP(msg, writeBuf, addDateToResponse_, extraHeaders);
  *size = headerCodec_.getEncodedSize();

  IOBufQueue queue(IOBufQueue::cacheChainLength());
  auto chunkLen =
      splitCompressed(size->compressed, remainingFrameSize, writeBuf, queue);
  bool endHeaders = queue.chainLength() == 0;
  if (assocStream) {
    DCHECK_EQ(transportDirection_, TransportDirection::DOWNSTREAM);
    DCHECK(!eom);
    generateHeaderCallbackWrapper(
        stream,
        http2::FrameType::PUSH_PROMISE,
        http2::writePushPromise((uint8_t*)frameHeader.first,
                                frameHeader.second,
                                writeBuf,
                                *assocStream,
                                stream,
                                chunkLen,
                                http2::kNoPadding,
                                endHeaders));
  } else if (exAttributes) {
    generateHeaderCallbackWrapper(
        stream,
        http2::FrameType::EX_HEADERS,
        http2::writeExHeaders((uint8_t*)frameHeader.first,
                              frameHeader.second,
                              writeBuf,
                              chunkLen,
                              stream,
                              *exAttributes,
                              pri,
                              http2::kNoPadding,
                              eom,
                              endHeaders));
  } else {
    generateHeaderCallbackWrapper(
        stream,
        http2::FrameType::HEADERS,
        http2::writeHeaders((uint8_t*)frameHeader.first,
                            frameHeader.second,
                            writeBuf,
                            chunkLen,
                            stream,
                            pri,
                            http2::kNoPadding,
                            eom,
                            endHeaders));
  }

  if (!endHeaders) {
    generateContinuation(
        writeBuf, queue, assocStream ? *assocStream : stream, maxFrameSize);
  }
}

void HTTP2Codec::generateContinuation(folly::IOBufQueue& writeBuf,
                                      folly::IOBufQueue& queue,
                                      StreamID stream,
                                      size_t maxFrameSize) {
  bool endHeaders = false;
  while (!endHeaders) {
    auto chunk = queue.split(std::min(maxFrameSize, queue.chainLength()));
    endHeaders = (queue.chainLength() == 0);
    VLOG(4) << "generating CONTINUATION for stream=" << stream;
    generateHeaderCallbackWrapper(
        stream,
        http2::FrameType::CONTINUATION,
        http2::writeContinuation(
            writeBuf, stream, endHeaders, std::move(chunk)));
  }
}

void HTTP2Codec::encodeHeaders(folly::IOBufQueue& writeBuf,
                               const HTTPHeaders& headers,
                               std::vector<compress::Header>& allHeaders,
                               HTTPHeaderSize* size) {
  headerCodec_.encode(allHeaders, writeBuf);
  if (size) {
    *size = headerCodec_.getEncodedSize();
  }

  if (headerCodec_.getEncodedSize().uncompressed >
      ingressSettings_.getSetting(SettingsId::MAX_HEADER_LIST_SIZE,
                                  std::numeric_limits<uint32_t>::max())) {
    // The remote side told us they don't want headers this large...
    // but this function has no mechanism to fail
    string serializedHeaders;
    headers.forEach(
        [&serializedHeaders](const string& name, const string& value) {
          serializedHeaders =
              folly::to<string>(serializedHeaders, "\\n", name, ":", value);
        });
    LOG(ERROR) << "generating HEADERS frame larger than peer maximum nHeaders="
               << headers.size() << " all headers=" << serializedHeaders;
  }
}

size_t HTTP2Codec::generateHeaderCallbackWrapper(StreamID stream,
                                                 http2::FrameType type,
                                                 size_t length) {
  if (callback_) {
    callback_->onGenerateFrameHeader(
        stream, static_cast<uint8_t>(type), length);
  }
  return length;
}

size_t HTTP2Codec::generateBody(folly::IOBufQueue& writeBuf,
                                StreamID stream,
                                std::unique_ptr<folly::IOBuf> chain,
                                folly::Optional<uint8_t> padding,
                                bool eom) {
  // todo: generate random padding for everything?
  size_t written = 0;
  if (!isStreamIngressEgressAllowed(stream)) {
    VLOG(2) << "Suppressing DATA for stream=" << stream
            << " ingressGoawayAck_=" << ingressGoawayAck_;
    return 0;
  }
  VLOG(4) << "generating DATA for stream=" << stream
          << " size=" << (chain ? chain->computeChainDataLength() : 0);
  IOBufQueue queue(IOBufQueue::cacheChainLength());
  queue.append(std::move(chain));
  size_t maxFrameSize = maxSendFrameSize();
  while (queue.chainLength() > maxFrameSize) {
    auto chunk = queue.split(maxFrameSize);
    written += generateHeaderCallbackWrapper(
        stream,
        http2::FrameType::DATA,
        http2::writeData(writeBuf,
                         std::move(chunk),
                         stream,
                         padding,
                         false,
                         reuseIOBufHeadroomForData_));
  }

  return written + generateHeaderCallbackWrapper(
                       stream,
                       http2::FrameType::DATA,
                       http2::writeData(writeBuf,
                                        queue.move(),
                                        stream,
                                        padding,
                                        eom,
                                        reuseIOBufHeadroomForData_));
}

size_t HTTP2Codec::generateChunkHeader(folly::IOBufQueue& /*writeBuf*/,
                                       StreamID /*stream*/,
                                       size_t /*length*/) {
  // HTTP/2 has no chunk headers
  return 0;
}

size_t HTTP2Codec::generateChunkTerminator(folly::IOBufQueue& /*writeBuf*/,
                                           StreamID /*stream*/) {
  // HTTP/2 has no chunk terminators
  return 0;
}

size_t HTTP2Codec::generateTrailers(folly::IOBufQueue& writeBuf,
                                    StreamID stream,
                                    const HTTPHeaders& trailers) {
  if (trailers.size() == 0) {
    // No point in sending an empty trailer block, convert to EOM.
    return generateEOM(writeBuf, stream);
  }
  VLOG(4) << "generating TRAILERS for stream=" << stream;
  std::vector<compress::Header> allHeaders;
  CodecUtil::appendHeaders(trailers, allHeaders, HTTP_HEADER_NONE);

  HTTPHeaderSize size{0, 0, 0};
  uint8_t headerSize = http2::kFrameHeaderSize;
  auto remainingFrameSize = maxSendFrameSize();
  auto frameHeader = writeBuf.preallocate(headerSize, kDefaultGrowth);
  writeBuf.postallocate(headerSize);
  encodeHeaders(writeBuf, trailers, allHeaders, &size);
  IOBufQueue queue(IOBufQueue::cacheChainLength());
  auto chunkLen =
      splitCompressed(size.compressed, remainingFrameSize, writeBuf, queue);
  bool endHeaders = queue.chainLength() == 0;
  generateHeaderCallbackWrapper(stream,
                                http2::FrameType::HEADERS,
                                http2::writeHeaders((uint8_t*)frameHeader.first,
                                                    frameHeader.second,
                                                    writeBuf,
                                                    chunkLen,
                                                    stream,
                                                    folly::none,
                                                    http2::kNoPadding,
                                                    true /*eom*/,
                                                    endHeaders));

  if (!endHeaders) {
    generateContinuation(writeBuf, queue, stream, remainingFrameSize);
  }

  return size.compressed;
}

size_t HTTP2Codec::generateEOM(folly::IOBufQueue& writeBuf, StreamID stream) {
  VLOG(4) << "sending EOM for stream=" << stream;
  upgradedStreams_.erase(stream);
  if (!isStreamIngressEgressAllowed(stream)) {
    VLOG(2) << "suppressed EOM for stream=" << stream
            << " ingressGoawayAck_=" << ingressGoawayAck_;
    return 0;
  }
  return generateHeaderCallbackWrapper(
      stream,
      http2::FrameType::DATA,
      http2::writeData(writeBuf,
                       nullptr,
                       stream,
                       http2::kNoPadding,
                       true,
                       reuseIOBufHeadroomForData_));
}

size_t HTTP2Codec::generateRstStream(folly::IOBufQueue& writeBuf,
                                     StreamID stream,
                                     ErrorCode statusCode) {
  VLOG(4) << "sending RST_STREAM for stream=" << stream
          << " with code=" << getErrorCodeString(statusCode);
  if (!isStreamIngressEgressAllowed(stream)) {
    VLOG(2) << "suppressed RST_STREAM for stream=" << stream
            << " ingressGoawayAck_=" << ingressGoawayAck_;
    return 0;
  }
  // Suppress any EOM callback for the current frame.
  if (stream == curHeader_.stream) {
    curHeader_.flags &= ~http2::END_STREAM;
    pendingEndStreamHandling_ = false;
    ingressWebsocketUpgrade_ = false;
  }
  upgradedStreams_.erase(stream);

  if (statusCode == ErrorCode::PROTOCOL_ERROR) {
    VLOG(2) << "sending RST_STREAM with code=" << getErrorCodeString(statusCode)
            << " for stream=" << stream << " user-agent=" << userAgent_;
  }
  return generateHeaderCallbackWrapper(
      stream,
      http2::FrameType::RST_STREAM,
      http2::writeRstStream(writeBuf, stream, statusCode));
}

size_t HTTP2Codec::generateGoaway(folly::IOBufQueue& writeBuf,
                                  StreamID lastStream,
                                  ErrorCode statusCode,
                                  std::unique_ptr<folly::IOBuf> debugData) {
  if (sessionClosing_ == ClosingState::CLOSED) {
    VLOG(4) << "Not sending GOAWAY for closed session";
    return 0;
  }

  // If the caller didn't specify a last stream, choose the correct one
  // If there's an error or this is the final GOAWAY, use last received stream
  if (lastStream == HTTPCodec::MaxStreamID) {
    if (statusCode != ErrorCode::NO_ERROR || !isReusable() ||
        isWaitingToDrain()) {
      lastStream = getLastIncomingStreamID();
    } else {
      lastStream = http2::kMaxStreamID;
    }
  }
  DCHECK_LE(lastStream, egressGoawayAck_) << "Cannot increase last good stream";
  egressGoawayAck_ = lastStream;
  switch (sessionClosing_) {
    case ClosingState::OPEN:
    case ClosingState::OPEN_WITH_GRACEFUL_DRAIN_ENABLED:
      if (lastStream == http2::kMaxStreamID &&
          statusCode == ErrorCode::NO_ERROR) {
        sessionClosing_ = ClosingState::FIRST_GOAWAY_SENT;
      } else {
        // The user of this codec decided not to do the double goaway
        // drain, or this is not a graceful shutdown
        sessionClosing_ = ClosingState::CLOSED;
      }
      break;
    case ClosingState::FIRST_GOAWAY_SENT:
      sessionClosing_ = ClosingState::CLOSED;
      break;
    case ClosingState::CLOSING:
    case ClosingState::CLOSED:
      LOG(FATAL) << "unreachable";
  }

  VLOG(4) << "Sending GOAWAY with last acknowledged stream=" << lastStream
          << " with code=" << getErrorCodeString(statusCode);
  if (statusCode == ErrorCode::PROTOCOL_ERROR) {
    VLOG(2) << "sending GOAWAY with last acknowledged stream=" << lastStream
            << " with code=" << getErrorCodeString(statusCode)
            << " user-agent=" << userAgent_;
  }

  return generateHeaderCallbackWrapper(
      0,
      http2::FrameType::GOAWAY,
      http2::writeGoaway(
          writeBuf, lastStream, statusCode, std::move(debugData)));
}

size_t HTTP2Codec::generatePingRequest(folly::IOBufQueue& writeBuf,
                                       folly::Optional<uint64_t> data) {
  // should probably let the caller specify when integrating with session
  // we know HTTPSession sets up events to track ping latency
  if (!data.has_value()) {
    data = folly::Random::rand64();
  }
  VLOG(4) << "Generating ping request with data=" << *data;
  return generateHeaderCallbackWrapper(
      0,
      http2::FrameType::PING,
      http2::writePing(writeBuf, *data, false /* no ack */));
}

size_t HTTP2Codec::generatePingReply(folly::IOBufQueue& writeBuf,
                                     uint64_t data) {
  VLOG(4) << "Generating ping reply with data=" << data;
  return generateHeaderCallbackWrapper(
      0,
      http2::FrameType::PING,
      http2::writePing(writeBuf, data, true /* ack */));
}

size_t HTTP2Codec::generateSettings(folly::IOBufQueue& writeBuf) {
  std::deque<SettingPair> settings;
  for (auto& setting : egressSettings_.getAllSettings()) {
    switch (setting.id) {
      case SettingsId::HEADER_TABLE_SIZE:
        if (pendingTableMaxSize_) {
          LOG(ERROR) << "Can't have more than one settings in flight, skipping";
          continue;
        } else {
          pendingTableMaxSize_ = setting.value;
        }
        break;
      case SettingsId::ENABLE_PUSH:
        if (transportDirection_ == TransportDirection::DOWNSTREAM) {
          // HTTP/2 spec says downstream must not send this flag
          // HTTP2Codec uses it to determine if push features are enabled
          continue;
        } else {
          CHECK(setting.value == 0 || setting.value == 1);
        }
        break;
      case SettingsId::MAX_CONCURRENT_STREAMS:
      case SettingsId::INITIAL_WINDOW_SIZE:
      case SettingsId::MAX_FRAME_SIZE:
        break;
      case SettingsId::MAX_HEADER_LIST_SIZE:
        headerCodec_.setMaxUncompressed(setting.value);
        break;
      case SettingsId::ENABLE_EX_HEADERS:
        CHECK(setting.value == 0 || setting.value == 1);
        if (setting.value == 0) {
          continue; // just skip the experimental setting if disabled
        } else {
          VLOG(4) << "generating ENABLE_EX_HEADERS=" << setting.value;
        }
        break;
      case SettingsId::ENABLE_CONNECT_PROTOCOL:
        if (setting.value == 0) {
          continue;
        }
        break;
      case SettingsId::THRIFT_CHANNEL_ID:
      case SettingsId::THRIFT_CHANNEL_ID_DEPRECATED:
        break;
      default:
        LOG(ERROR) << "ignore unknown settingsId="
                   << std::underlying_type<SettingsId>::type(setting.id)
                   << " value=" << setting.value;
        continue;
    }

    settings.push_back(SettingPair(setting.id, setting.value));
  }
  VLOG(4) << getTransportDirectionString(getTransportDirection())
          << " generating " << (unsigned)settings.size() << " settings";
  return generateHeaderCallbackWrapper(
      0, http2::FrameType::SETTINGS, http2::writeSettings(writeBuf, settings));
}

void HTTP2Codec::requestUpgrade(HTTPMessage& request) {
  auto& headers = request.getHeaders();
  headers.set(HTTP_HEADER_UPGRADE, http2::kProtocolCleartextString);
  bool addUpgrade =
      !request.checkForHeaderToken(HTTP_HEADER_CONNECTION, "Upgrade", false);
  IOBufQueue writeBuf{IOBufQueue::cacheChainLength()};
  generateDefaultSettings(writeBuf);
  writeBuf.trimStart(http2::kFrameHeaderSize);
  auto binarySettings = writeBuf.move()->to<std::string>();
  headers.set(http2::kProtocolSettingsHeader,
              folly::base64URLEncode(binarySettings));
  bool addSettings = !request.checkForHeaderToken(
      HTTP_HEADER_CONNECTION, http2::kProtocolSettingsHeader.c_str(), false);
  if (addUpgrade && addSettings) {
    headers.add(HTTP_HEADER_CONNECTION,
                folly::to<string>("Upgrade, ", http2::kProtocolSettingsHeader));
  } else if (addUpgrade) {
    headers.add(HTTP_HEADER_CONNECTION, "Upgrade");
  } else if (addSettings) {
    headers.add(HTTP_HEADER_CONNECTION, http2::kProtocolSettingsHeader);
  }
}

size_t HTTP2Codec::generateDefaultSettings(folly::IOBufQueue& writeBuf) {
  static HTTP2Codec defaultCodec{TransportDirection::UPSTREAM};
  return defaultCodec.generateSettings(writeBuf);
}

size_t HTTP2Codec::generateSettingsAck(folly::IOBufQueue& writeBuf) {
  VLOG(4) << getTransportDirectionString(getTransportDirection())
          << " generating settings ack";
  return generateHeaderCallbackWrapper(
      0, http2::FrameType::SETTINGS, http2::writeSettingsAck(writeBuf));
}

size_t HTTP2Codec::generateWindowUpdate(folly::IOBufQueue& writeBuf,
                                        StreamID stream,
                                        uint32_t delta) {
  VLOG(4) << "generating window update for stream=" << stream << ": Processed "
          << delta << " bytes";
  if (!isStreamIngressEgressAllowed(stream)) {
    VLOG(2) << "suppressed WINDOW_UPDATE for stream=" << stream
            << " ingressGoawayAck_=" << ingressGoawayAck_;
    return 0;
  }
  return generateHeaderCallbackWrapper(
      stream,
      http2::FrameType::WINDOW_UPDATE,
      http2::writeWindowUpdate(writeBuf, stream, delta));
}

size_t HTTP2Codec::generatePriority(folly::IOBufQueue& writeBuf,
                                    StreamID stream,
                                    const HTTPMessage::HTTP2Priority& pri) {
  VLOG(4) << "generating priority for stream=" << stream;
  if (!isStreamIngressEgressAllowed(stream)) {
    VLOG(2) << "suppressed PRIORITY for stream=" << stream
            << " ingressGoawayAck_=" << ingressGoawayAck_;
    return 0;
  }
  return generateHeaderCallbackWrapper(
      stream,
      http2::FrameType::PRIORITY,
      http2::writePriority(
          writeBuf,
          stream,
          {std::get<0>(pri), std::get<1>(pri), std::get<2>(pri)}));
}

size_t HTTP2Codec::generatePriority(folly::IOBufQueue& /* writeBuf */,
                                    StreamID /* stream */,
                                    HTTPPriority /* priority */) {
  return 0;
}

size_t HTTP2Codec::generateCertificateRequest(
    folly::IOBufQueue& writeBuf,
    uint16_t requestId,
    std::unique_ptr<folly::IOBuf> certificateRequestData) {
  VLOG(4) << "generating CERTIFICATE_REQUEST with Request-ID=" << requestId;
  return http2::writeCertificateRequest(
      writeBuf, requestId, std::move(certificateRequestData));
}

size_t HTTP2Codec::generateCertificate(folly::IOBufQueue& writeBuf,
                                       uint16_t certId,
                                       std::unique_ptr<folly::IOBuf> certData) {
  size_t written = 0;
  VLOG(4) << "sending CERTIFICATE with Cert-ID=" << certId << "for stream=0";
  IOBufQueue queue(IOBufQueue::cacheChainLength());
  queue.append(std::move(certData));
  // The maximum size of an autenticator fragment, combined with the Cert-ID can
  // not exceed the maximal allowable size of a sent frame.
  size_t maxChunkSize = maxSendFrameSize() - sizeof(certId);
  while (queue.chainLength() > maxChunkSize) {
    auto chunk = queue.splitAtMost(maxChunkSize);
    written +=
        http2::writeCertificate(writeBuf, certId, std::move(chunk), true);
  }
  return written +
         http2::writeCertificate(writeBuf, certId, queue.move(), false);
}

bool HTTP2Codec::checkConnectionError(ErrorCode err, const folly::IOBuf* buf) {
  if (err != ErrorCode::NO_ERROR) {
    LOG(ERROR) << "Connection error " << getErrorCodeString(err)
               << " with ingress=";
    VLOG(3) << IOBufPrinter::printHexFolly(buf, true);
    if (callback_) {
      std::string errorDescription = goawayErrorMessage_.empty()
                                         ? "Connection error"
                                         : goawayErrorMessage_;
      HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                       errorDescription);
      ex.setCodecStatusCode(err);
      callback_->onError(0, ex, false);
    }
    return true;
  }
  return false;
}

void HTTP2Codec::streamError(const std::string& msg,
                             ErrorCode code,
                             bool newTxn,
                             folly::Optional<HTTPCodec::StreamID> streamId,
                             std::unique_ptr<HTTPMessage> partialMessage) {
  HTTPException error(HTTPException::Direction::INGRESS_AND_EGRESS, msg);
  error.setCodecStatusCode(code);
  if (partialMessage) {
    error.setPartialMsg(std::move(partialMessage));
  }
  deliverCallbackIfAllowed(&HTTPCodec::Callback::onError,
                           "onError",
                           streamId ? *streamId : curHeader_.stream,
                           error,
                           newTxn);
}

HTTPCodec::StreamID HTTP2Codec::mapPriorityToDependency(
    uint8_t priority) const {
  // If the priority is out of the maximum index of virtual nodes array, we
  // return the lowest level virtual node as a punishment of not setting
  // priority correctly.
  return virtualPriorityNodes_.empty()
             ? 0
             : virtualPriorityNodes_[std::min(
                   priority, uint8_t(virtualPriorityNodes_.size() - 1))];
}

bool HTTP2Codec::parsingHeaders() const {
  return (curHeader_.type == http2::FrameType::HEADERS ||
          (curHeader_.type == http2::FrameType::CONTINUATION &&
           headerBlockFrameType_ == http2::FrameType::HEADERS));
}

bool HTTP2Codec::parsingTrailers() const {
  // HEADERS frame is used for request/response headers and trailers.
  // Per spec, specific role of HEADERS frame is determined by it's postion
  // within the stream. We don't keep full stream state in this codec,
  // thus using heuristics to distinguish between headers/trailers.
  // For DOWNSTREAM case, request headers HEADERS frame would be creating
  // new stream, thus HEADERS on existing stream ID are considered trailers
  // (see checkNewStream).
  // For UPSTREAM case, response headers are required to have status code,
  // thus if no status code we consider that trailers.
  if (parsingHeaders()) {
    if (transportDirection_ == TransportDirection::DOWNSTREAM) {
      return parsingDownstreamTrailers_;
    } else {
      // We *might* be parsing trailers even if we couldn't decode the block.
      // We probably aren't, because trailers are rare, but if we do,
      // we might miscategorize as headers and give a second onMessageBegin.
      return decodeInfo_.parsingError.empty() && !decodeInfo_.hasStatus();
    }
  }
  return false;
}
} // namespace proxygen
