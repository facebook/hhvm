/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HTTPBinaryCodec.h>

#include <proxygen/lib/http/codec/CodecUtil.h>
#include <quic/codec/QuicInteger.h>

namespace proxygen {

namespace {
folly::Expected<size_t, quic::TransportErrorCode> encodeInteger(
    uint64_t i, folly::io::QueueAppender& appender) {
  return quic::encodeQuicInteger(i, [&](auto val) { appender.writeBE(val); });
}

void encodeString(folly::StringPiece str, folly::io::QueueAppender& appender) {
  encodeInteger(str.size(), appender);
  appender.pushAtMost((const uint8_t*)str.data(), str.size());
}
} // namespace

HTTPBinaryCodec::HTTPBinaryCodec(TransportDirection direction) {
  transportDirection_ = direction;
  state_ = ParseState::FRAMING_INDICATOR;
  parseError_ = folly::none;
  parserPaused_ = false;
}

HTTPBinaryCodec::~HTTPBinaryCodec() {
}

ParseResult HTTPBinaryCodec::parseFramingIndicator(folly::io::Cursor& cursor,
                                                   bool& request,
                                                   bool& knownLength) {
  size_t parsed = 0;

  // Parse the framingIndicator and advance the cursor
  auto framingIndicator = quic::decodeQuicInteger(cursor);
  if (!framingIndicator) {
    return folly::makeUnexpected(
        std::string("Failure to parse Framing Indicator"));
  }
  // Increase parsed by the number of bytes read
  parsed += framingIndicator->second;
  // Sanity check the value of the framingIndicator
  if (framingIndicator->first >
      static_cast<uint64_t>(
          HTTPBinaryCodec::FramingIndicator::RESPONSE_INDETERMINATE_LENGTH)) {
    return folly::makeUnexpected(
        fmt::format("Invalid Framing Indicator: {}", framingIndicator->first));
  }

  // Set request to true if framingIndicator is even (0 and 2 correspond to
  // requests)
  request = ((framingIndicator->first & 0x01) == 0);
  // Set knownLength to true if framingIndicator is 0 or 1
  knownLength = ((framingIndicator->first & 0x02) == 0);
  if (!knownLength) {
    return folly::makeUnexpected(
        std::string("Unsupported indeterminate length Binary HTTP Request"));
  }
  return parsed;
}

ParseResult HTTPBinaryCodec::parseKnownLengthString(
    folly::io::Cursor& cursor,
    size_t remaining,
    folly::StringPiece stringName,
    std::string& stringValue) {
  size_t parsed = 0;

  // Parse the encodedStringLength and advance cursor
  auto encodedStringLength = quic::decodeQuicInteger(cursor);
  if (!encodedStringLength) {
    return folly::makeUnexpected(
        fmt::format("Failure to parse: {} length", stringName));
  }
  // Increase parsed by the number of bytes read
  parsed += encodedStringLength->second;
  // Check that we have not gone beyond "remaining"
  if (encodedStringLength->first > remaining - parsed) {
    return folly::makeUnexpected(
        fmt::format("Failure to parse: {}", stringName));
  }

  // Handle edge case where field is not present/has length 0
  if (encodedStringLength->first == 0) {
    stringValue.clear();
    return parsed;
  }

  // Read the value of the encodedString
  stringValue = cursor.readFixedString(encodedStringLength->first);

  // Increase parsed by the number of bytes read
  parsed += encodedStringLength->first;
  return parsed;
}

ParseResult HTTPBinaryCodec::parseRequestControlData(folly::io::Cursor& cursor,
                                                     size_t remaining,
                                                     HTTPMessage& msg) {
  size_t parsed = 0;

  // Parse method
  std::string method;
  auto methodRes = parseKnownLengthString(cursor, remaining, "method", method);
  if (methodRes.hasError()) {
    return methodRes;
  }
  parsed += *methodRes;
  remaining -= *methodRes;
  msg.setMethod(method);

  // Parse scheme
  std::string scheme;
  auto schemeRes = parseKnownLengthString(cursor, remaining, "scheme", scheme);
  if (schemeRes.hasError()) {
    return schemeRes;
  }
  parsed += *schemeRes;
  remaining -= *schemeRes;
  if (scheme == proxygen::headers::kHttp) {
    msg.setSecure(false);
  } else if (scheme == proxygen::headers::kHttps) {
    msg.setSecure(true);
  } else {
    return folly::makeUnexpected(
        std::string("Failure to parse: scheme. Should be 'http' or 'https'"));
  }

  // Parse authority
  std::string authority;
  auto authorityRes =
      parseKnownLengthString(cursor, remaining, "authority", authority);
  if (authorityRes.hasError()) {
    return authorityRes;
  }
  parsed += *authorityRes;
  remaining -= *authorityRes;

  // Parse path
  std::string path;
  auto pathRes = parseKnownLengthString(cursor, remaining, "path", path);
  if (pathRes.hasError()) {
    return pathRes;
  }
  // Set relative path to msg URL
  auto parseUrl = msg.setURL(path);
  if (!parseUrl.valid()) {
    return folly::makeUnexpected(
        fmt::format("Failure to parse: invalid URL path '{}'", path));
  }
  parsed += *pathRes;
  remaining -= *pathRes;

  return parsed;
}

ParseResult HTTPBinaryCodec::parseResponseControlData(folly::io::Cursor& cursor,
                                                      size_t remaining,
                                                      HTTPMessage& msg) {
  // Parse statusCode and advance cursor
  auto statusCode = quic::decodeQuicInteger(cursor);
  if (!statusCode) {
    return folly::makeUnexpected(
        std::string("Failure to parse response status code"));
  }
  // Sanity check status code
  if (statusCode->first < 200 || statusCode->first > 599) {
    return folly::makeUnexpected(
        fmt::format("Invalid response status code: {}", statusCode->first));
  }
  msg.setStatusCode(statusCode->first);
  return statusCode->second;
}

ParseResult HTTPBinaryCodec::parseHeadersHelper(folly::io::Cursor& cursor,
                                                size_t remaining,
                                                HeaderDecodeInfo& decodeInfo,
                                                bool isTrailers) {
  size_t parsed = 0;

  // Parse length of headers and advance cursor
  auto lengthOfHeaders = quic::decodeQuicInteger(cursor);
  if (!lengthOfHeaders) {
    return folly::makeUnexpected(
        std::string("Failure to parse number of headers"));
  }
  // Check that we had enough bytes to parse lengthOfHeaders
  if (remaining < lengthOfHeaders->second) {
    return folly::makeUnexpected(
        fmt::format("Header parsing underflowed! Not enough space ({} bytes "
                    "remaining) to parse the length of headers ({} bytes)",
                    remaining,
                    lengthOfHeaders->second));
  }
  // Increase parsed and decrease remaining by the number of bytes read
  parsed += lengthOfHeaders->second;
  remaining -= lengthOfHeaders->second;
  if (remaining < lengthOfHeaders->first) {
    return folly::makeUnexpected(fmt::format(
        "Header parsing underflowed! Headers length in bytes ({}) is "
        "inconsistent with remaining buffer length ({})",
        lengthOfHeaders->first,
        remaining));
  }

  auto numHeaders = 0;
  while (parsed < lengthOfHeaders->first) {
    std::string headerName;
    auto headerNameRes =
        parseKnownLengthString(cursor, remaining, "headerName", headerName);
    if (headerNameRes.hasError()) {
      return headerNameRes;
    }
    parsed += *headerNameRes;
    remaining -= *headerNameRes;

    std::string headerValue;
    auto headerValueRes =
        parseKnownLengthString(cursor, remaining, "headerValue", headerValue);
    if (headerValueRes.hasError()) {
      return headerValueRes;
    }
    parsed += *headerValueRes;
    remaining -= *headerValueRes;

    if (!decodeInfo.onHeader(proxygen::HPACKHeaderName(headerName),
                             headerValue) ||
        !decodeInfo.parsingError.empty()) {
      return folly::makeUnexpected(fmt::format(
          "Error parsing field section (Error: {})", decodeInfo.parsingError));
    }
    numHeaders++;
  }
  if (numHeaders < 1 && !isTrailers) {
    return folly::makeUnexpected(
        fmt::format("Number of headers (key value pairs) should be >= 1. "
                    "Header count is {}",
                    numHeaders));
  }

  return parsed;
}

ParseResult HTTPBinaryCodec::parseHeaders(folly::io::Cursor& cursor,
                                          size_t remaining,
                                          HeaderDecodeInfo& decodeInfo) {
  return parseHeadersHelper(cursor, remaining, decodeInfo, false);
}

ParseResult HTTPBinaryCodec::parseContent(folly::io::Cursor& cursor,
                                          size_t remaining,
                                          HTTPMessage& msg) {
  size_t parsed = 0;

  // Parse the contentLength and advance cursor
  auto contentLength = quic::decodeQuicInteger(cursor);
  if (!contentLength) {
    return folly::makeUnexpected(
        std::string("Failure to parse content length"));
  }
  // Increase parsed by the number of bytes read
  parsed += contentLength->second;
  if (contentLength->first == 0) {
    return parsed;
  }
  // Check that we have not gone beyond "remaining"
  if (contentLength->first > remaining - parsed) {
    return folly::makeUnexpected(std::string("Failure to parse content"));
  }

  // Write the data to msgBody_ and then advance the cursor
  msgBody_ = std::make_unique<folly::IOBuf>();
  cursor.clone(*msgBody_.get(), contentLength->first);

  // Increase parsed by the number of bytes read
  parsed += contentLength->first;
  return parsed;
}

ParseResult HTTPBinaryCodec::parseTrailers(folly::io::Cursor& cursor,
                                           size_t remaining,
                                           HeaderDecodeInfo& decodeInfo) {
  return parseHeadersHelper(cursor, remaining, decodeInfo, true);
}

size_t HTTPBinaryCodec::onIngress(const folly::IOBuf& buf) {
  auto len = bufferedIngress_.chainLength();
  bufferedIngress_.append(buf.clone());
  return bufferedIngress_.chainLength() - len;
}

void HTTPBinaryCodec::onIngressEOF() {
  size_t parsedTot = 0;
  folly::io::Cursor cursor(bufferedIngress_.front());
  auto bufLen = bufferedIngress_.chainLength();
  if (!bufLen) {
    parseError_ = "Empty buffer provided!";
  }

  while (!parseError_ && parsedTot < bufLen && !parserPaused_) {
    size_t parsed = 0;
    ParseResult parseResult;
    switch (state_) {
      case ParseState::FRAMING_INDICATOR:
        // FRAMING_INDICATOR should be the first item that is parsed
        parseResult = parseFramingIndicator(cursor, request_, knownLength_);
        if (parseResult.hasError()) {
          parseError_ = parseResult.error();
          break;
        }
        parsed += *parseResult;
        // If the framing indicator is for a request, then the
        // TransportDirection should be DOWNSTREAM and vice versa.
        if ((transportDirection_ == TransportDirection::DOWNSTREAM) !=
            request_) {
          parseError_ =
              fmt::format("Invalid Framing Indicator '{}' for {} codec",
                          request_ ? "request" : "response",
                          folly::to_underlying(transportDirection_));
          break;
        }
        if (!request_) {
          // If it's a response, then the next item to parse is the
          // INFORMATIONAL_RESPONSE
          state_ = ParseState::INFORMATIONAL_RESPONSE;
        } else {
          // else we parse the control data
          state_ = ParseState::CONTROL_DATA;
        }
        break;

      case ParseState::INFORMATIONAL_RESPONSE:
        // TODO(T118289674) - Currently, the OHAI protocol doesn't support
        // informational responses
        // (https://ietf-wg-ohai.github.io/oblivious-http/draft-ietf-ohai-ohttp.html#name-informational-responses).
        // Since we are primarily building this codec for an MVP 3rd Party OHAI
        // proxy, we will skip parsing the INFORMATIONAL_RESPONSE for now and we
        // can implement this later for complete functionality.
        state_ = ParseState::CONTROL_DATA;
        break;

      case ParseState::CONTROL_DATA:
        decodeInfo_.init(request_,
                         false /* isRequestTrailers */,
                         true /* validate */,
                         true /* strictValidation */,
                         false /* allowEmptyPath */);
        // The control data has a different format based on request/response
        if (request_) {
          parseResult = parseRequestControlData(
              cursor, bufLen - parsedTot, *decodeInfo_.msg);
        } else {
          parseResult = parseResponseControlData(
              cursor, bufLen - parsedTot, *decodeInfo_.msg);
        }
        if (parseResult.hasError()) {
          parseError_ = parseResult.error();
          break;
        }
        parsed += *parseResult;
        state_ = ParseState::HEADERS_SECTION;
        break;

      case ParseState::HEADERS_SECTION:
        CHECK(decodeInfo_.msg);
        parseResult = parseHeaders(cursor, bufLen - parsedTot, decodeInfo_);
        if (parseResult.hasError()) {
          parseError_ = parseResult.error();
          break;
        }
        parsed += *parseResult;
        state_ = ParseState::CONTENT;
        msg_ = std::move(decodeInfo_.msg);
        break;

      case ParseState::CONTENT:
        CHECK(msg_);
        parseResult = parseContent(cursor, bufLen - parsedTot, *msg_);
        if (parseResult.hasError()) {
          parseError_ = parseResult.error();
          break;
        }
        parsed += *parseResult;
        state_ = ParseState::TRAILERS_SECTION;
        break;

      case ParseState::TRAILERS_SECTION:
        decodeInfo_.init(request_,
                         true /* isRequestTrailers */,
                         true /* validate */,
                         true /* strictValidation */,
                         false /* allowEmptyPath */);
        parseResult = parseTrailers(cursor, bufLen - parsedTot, decodeInfo_);
        if (parseResult.hasError()) {
          parseError_ = parseResult.error();
          break;
        }
        trailers_ =
            std::make_unique<HTTPHeaders>(decodeInfo_.msg->getHeaders());
        parsed += *parseResult;
        state_ = ParseState::PADDING;
        break;

      case ParseState::PADDING:
        // This needs to be the last section
        parsed = bufLen - parsedTot;
        cursor.advanceToEnd();
        break;

      default:
        CHECK(false);
        break;
    }
    parsedTot += parsed;
  }

  if (parseError_) {
    callback_->onError(
        ingressTxnID_,
        HTTPException(HTTPException::Direction::INGRESS,
                      fmt::format("Invalid Message: {}", *parseError_)));
  } else {

    if (!msg_) {
      if (state_ == ParseState::HEADERS_SECTION) {
        // Case where the sent message only contains control data
        msg_ = std::move(decodeInfo_.msg);
      } else {
        callback_->onError(
            ingressTxnID_,
            HTTPException(
                HTTPException::Direction::INGRESS,
                fmt::format("Message not formed (incomplete binary data)")));
        return;
      }
    }
    callback_->onHeadersComplete(ingressTxnID_, std::move(msg_));
    if (msgBody_) {
      callback_->onBody(ingressTxnID_, std::move(msgBody_), 0);
    }
    if (trailers_) {
      callback_->onTrailersComplete(ingressTxnID_, std::move(trailers_));
    }
    callback_->onMessageComplete(ingressTxnID_, false);
  }
}

size_t HTTPBinaryCodec::generateHeaderHelper(folly::io::QueueAppender& appender,
                                             const HTTPHeaders& headers) {
  // Calculate the number of bytes it will take to encode all the headers
  size_t headersLength = 0;
  headers.forEach([&](folly::StringPiece name, folly::StringPiece value) {
    auto nameSize = name.size();
    auto valueSize = value.size();
    headersLength += quic::getQuicIntegerSize(nameSize).value() + nameSize +
                     quic::getQuicIntegerSize(valueSize).value() + valueSize;
  });

  // Encode all the headers
  auto lengthOfAllHeaders = encodeInteger(headersLength, appender);
  headersLength += lengthOfAllHeaders.value();
  headers.forEach([&](folly::StringPiece name, folly::StringPiece value) {
    encodeString(name, appender);
    encodeString(value, appender);
  });

  return headersLength;
}

void HTTPBinaryCodec::generateHeader(
    folly::IOBufQueue& writeBuf,
    StreamID txn,
    const HTTPMessage& msg,
    bool eom,
    HTTPHeaderSize* size,
    const folly::Optional<HTTPHeaders>& extraHeaders) {
  folly::io::QueueAppender appender(&writeBuf, queueAppenderMaxGrowth);
  if (transportDirection_ == TransportDirection::DOWNSTREAM) {
    // Encode Framing Indicator for Request
    encodeInteger(folly::to<uint64_t>(
                      HTTPBinaryCodec::FramingIndicator::REQUEST_KNOWN_LENGTH),
                  appender);
    // Encode Request Control Data
    encodeString(msg.getMethodString(), appender);
    encodeString(msg.isSecure() ? "https" : "http", appender);
    encodeString(msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST), appender);

    std::string pathWithQueryString = msg.getPath();
    if (!msg.getQueryString().empty()) {
      pathWithQueryString.append("?");
      pathWithQueryString.append(msg.getQueryString());
    }
    encodeString(pathWithQueryString, appender);
  } else {
    encodeInteger(folly::to<uint64_t>(
                      HTTPBinaryCodec::FramingIndicator::RESPONSE_KNOWN_LENGTH),
                  appender);
    // Response Control Data
    encodeInteger(msg.getStatusCode(), appender);
  }
  generateHeaderHelper(appender, msg.getHeaders());
}

size_t HTTPBinaryCodec::generateBody(folly::IOBufQueue& writeBuf,
                                     StreamID txn,
                                     std::unique_ptr<folly::IOBuf> chain,
                                     folly::Optional<uint8_t> padding,
                                     bool eom) {
  folly::io::QueueAppender appender(&writeBuf, queueAppenderMaxGrowth);
  size_t lengthWritten = 0;
  if (chain) {
    lengthWritten = chain->computeChainDataLength();
    encodeInteger(lengthWritten, appender);
    appender.insert(std::move(chain));
  }
  if (eom) {
    lengthWritten += generateEOM(writeBuf, txn);
  }

  return lengthWritten;
}

size_t HTTPBinaryCodec::generateTrailers(folly::IOBufQueue& writeBuf,
                                         StreamID txn,
                                         const HTTPHeaders& trailers) {
  folly::io::QueueAppender appender(&writeBuf, queueAppenderMaxGrowth);
  auto trailersLengthWritten = generateHeaderHelper(appender, trailers);
  encodeInteger(0, appender);
  trailersLengthWritten++;

  return trailersLengthWritten;
}

size_t HTTPBinaryCodec::generateEOM(folly::IOBufQueue& writeBuf, StreamID txn) {
  return 0;
}

size_t HTTPBinaryCodec::generateChunkHeader(folly::IOBufQueue& writeBuf,
                                            HTTPBinaryCodec::StreamID stream,
                                            size_t length) {
  // TODO(T118289674) - Implement HTTPBinaryCodec
  return 0;
}

size_t HTTPBinaryCodec::generateChunkTerminator(
    folly::IOBufQueue& writeBuf, HTTPBinaryCodec::StreamID stream) {
  // TODO(T118289674) - Implement HTTPBinaryCodec
  return 0;
}

size_t HTTPBinaryCodec::generateRstStream(folly::IOBufQueue& writeBuf,
                                          HTTPBinaryCodec::StreamID stream,
                                          ErrorCode statusCode) {
  // TODO(T118289674) - Implement HTTPBinaryCodec
  return 0;
}

size_t HTTPBinaryCodec::generateGoaway(
    folly::IOBufQueue& writeBuf,
    HTTPBinaryCodec::StreamID lastStream,
    ErrorCode statusCode,
    std::unique_ptr<folly::IOBuf> debugData) {
  // TODO(T118289674) - Implement HTTPBinaryCodec
  return 0;
}

} // namespace proxygen
