/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Conv.h>
#include <proxygen/lib/http/HTTP3ErrorCode.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/ProxygenErrorEnum.h>
#include <proxygen/lib/http/codec/ErrorCode.h>
#include <string>

namespace proxygen {
class HTTPException;
}

namespace proxygen::coro {

/**
 * HTTP Error Codes
 *
 * This error code can be returned when reading from an HTTPSource.  Users of
 * the library may read this error when reading from a source (either a server
 * reading a request or a client reading a response or push).
 *
 * The library itself may read it from the HTTPSource chain provided by a user
 * of the library (a request via sendRequest or a response via handleRequest).
 *
 * Applications using the library that wish to abort sending can return an error
 * code from readHeaderEvent or readBodyEvent.  Only a subset of codes make
 * sense for the application to send.  For H2, this set is:
 *   INTERNAL_ERROR, REFUSED_STREAM, CANCEL, CONNECT_ERROR, ENHANCE_YOUR_CALM,
 * INADEQUATE_SECURITY, HTTP_1_1_REQUIRED.
 *
 * Any other errors returned by applications will be mapped to one of these.
 *
 * Philosophy:
 *
 *  We want to have a single error code enum that captures the following:
 *
 *   1) Any error code that can be received on the wire according to any
 *      official HTTP protocol or supported extension.  This way, the library
 *      can pass the code directly to the caller.
 *   2) Errors detected by the library that are not passed on the wire, such as
 *      timeouts or transport errors.
 *
 * == DO NOT MODIFY this list of errors unless you know what you are doing ==
 */
// clang-format off
#define HTTP_ERROR_GEN_VALUES(n, v) \
  /* HTTP/2 error codes, from the RFC */ \
  n(NO_ERROR)               v(0),\
  n(PROTOCOL_ERROR)         v(1),\
  n(INTERNAL_ERROR)         v(2),\
  n(FLOW_CONTROL_ERROR)     v(3),\
  n(SETTINGS_TIMEOUT)       v(4),\
  n(STREAM_CLOSED)          v(5),\
  n(FRAME_SIZE_ERROR)       v(6),\
  n(REFUSED_STREAM)         v(7),\
  n(CANCEL)                 v(8),\
  n(COMPRESSION_ERROR)      v(9),\
  n(CONNECT_ERROR)         v(10),\
  n(ENHANCE_YOUR_CALM)     v(11),\
  n(INADEQUATE_SECURITY)   v(12),\
  n(HTTP_1_1_REQUIRED)     v(13),\
  \
  \
  /*  HTTP/3 error codes */                \
  n(_H3_NO_ERROR)                v(0x100),\
  n(GENERAL_PROTOCOL_ERROR)      v(0x101),\
  n(_H3_INTERNAL_ERROR)          v(0x102),\
  n(STREAM_CREATION_ERROR)       v(0x103),\
  n(CLOSED_CRITICAL_STREAM)      v(0x104),\
  n(FRAME_UNEXPECTED)            v(0x105),\
  n(FRAME_ERROR)                 v(0x106),\
  n(EXCESSIVE_LOAD)              v(0x107), /* ENHANCE_YOUR_CALM */ \
  n(ID_ERROR)                    v(0x108),\
  n(SETTINGS_ERROR)              v(0x109),\
  n(MISSING_SETTINGS)            v(0x10A),\
  n(REQUEST_REJECTED)            v(0x10B),  /* REFUSED_STREAM */ \
  n(REQUEST_CANCELLED)           v(0x10C), /* CANCEL */ \
  n(INCOMPLETE_REQUEST)          v(0x10D),\
  n(MESSAGE_ERROR)               v(0x10E),\
  n(_H3_CONNECT_ERROR)           v(0x10F),\
  n(VERSION_FALLBACK)            v(0x110),\
  n(QPACK_DECOMPRESSION_FAILED)  v(0x200),\
  n(QPACK_ENCODER_STREAM_ERROR)  v(0x201),\
  n(QPACK_DECODER_STREAM_ERROR)  v(0x202),\
  \
  \
  /* Higher level/more specific error codes returned by the library but */\
  /* never transmitted on the wire                                      */\
  n(INVALID_STATE_TRANSITION)     /* HTTP state machine violation */    \
                        v(0x1000),\
  n(HEADER_PARSE_ERROR) v(0x1001),/* Error parsing HTTP headers */ \
  n(BODY_PARSE_ERROR)   v(0x1002),/* Error parsing HTTP body */\
  n(READ_TIMEOUT)       v(0x1003),/* Timed out waiting for header/body event */\
  n(WRITE_TIMEOUT)      v(0x1004),/* Socket write timeout */\
  n(CORO_CANCELLED)     v(0x1005),/* Caller cancelled the coroutine */\
  n(TRANSPORT_EOF)      v(0x1006),/* EOF read before the stream was complete */\
  n(TRANSPORT_READ_ERROR)         /* Error reading from the transport */\
                        v(0x1007),\
  n(TRANSPORT_WRITE_ERROR)        /* Error writing to the transport */ \
                        v(0x1008),\
  n(DROPPED)            v(0x1009),/* Caller dropped session with open streams */\
  n(CONTENT_LENGTH_MISMATCH)      /* Content-Length did not match what was observed */\
                        v(0x100A)
  // ==DO NOT MODIFY this list of errors unless you know what you are doing ==
// clang-format on

#define HTTP_ERROR_ENUM(error) error

#define HTTP_ERROR_VALUE(value) = value

enum class HTTPErrorCode : uint16_t {
  HTTP_ERROR_GEN_VALUES(HTTP_ERROR_ENUM, HTTP_ERROR_VALUE)
};

#undef HTTP_ERROR_ENUM
#undef HTTP_ERROR_VALUE

#define HTTP_ERROR_NO_VALUE(value)
#define HTTP_ERROR_GEN(x) HTTP_ERROR_GEN_VALUES(x, HTTP_ERROR_NO_VALUE)

const std::string& getErrorString(HTTPErrorCode ec);

// Helpers to normalize H2/H3 wire error codes to a single semantic
inline bool isInternalError(HTTPErrorCode ec) {
  return ec == HTTPErrorCode::_H3_INTERNAL_ERROR ||
         ec == HTTPErrorCode::INTERNAL_ERROR;
}
inline bool isRequestRejected(HTTPErrorCode ec) {
  return ec == HTTPErrorCode::REQUEST_REJECTED ||
         ec == HTTPErrorCode::REFUSED_STREAM;
}
inline bool isCancelled(HTTPErrorCode ec) {
  return ec == HTTPErrorCode::REQUEST_CANCELLED || ec == HTTPErrorCode::CANCEL;
}

inline bool isConnectError(HTTPErrorCode ec) {
  return ec == HTTPErrorCode::_H3_CONNECT_ERROR ||
         ec == HTTPErrorCode::CONNECT_ERROR;
}

inline bool isExcessiveLoad(HTTPErrorCode ec) {
  return ec == HTTPErrorCode::EXCESSIVE_LOAD ||
         ec == HTTPErrorCode::ENHANCE_YOUR_CALM;
}

inline bool isVersionFallback(HTTPErrorCode ec) {
  return ec == HTTPErrorCode::VERSION_FALLBACK ||
         ec == HTTPErrorCode::HTTP_1_1_REQUIRED;
}

inline bool isProtocolError(HTTPErrorCode ec) {
  return ec == HTTPErrorCode::PROTOCOL_ERROR ||
         ec == HTTPErrorCode::GENERAL_PROTOCOL_ERROR;
}

inline bool isNoError(HTTPErrorCode ec) {
  return ec == HTTPErrorCode::NO_ERROR || ec == HTTPErrorCode::_H3_NO_ERROR;
}

/**
 * Simple error structure
 */
struct HTTPError : public std::exception {
  HTTPErrorCode code;
  std::string msg;
  std::unique_ptr<HTTPMessage> httpMessage;

  explicit HTTPError(HTTPErrorCode inCode)
      : code(inCode), msg(getErrorString(inCode)) {
  }

  HTTPError(HTTPErrorCode inCode, std::string inMsg)
      : code(inCode), msg(std::move(inMsg)) {
  }

  HTTPError(HTTPError&& goner) = default;
  HTTPError& operator=(HTTPError&& goner) = default;
  HTTPError(const HTTPError& other) : code(other.code), msg(other.msg) {
    if (other.httpMessage) {
      httpMessage = std::make_unique<HTTPMessage>(*other.httpMessage);
    }
  }
  HTTPError& operator=(const HTTPError& other) {
    code = other.code;
    msg = other.msg;
    if (other.httpMessage) {
      httpMessage = std::make_unique<HTTPMessage>(*other.httpMessage);
    }
    return *this;
  }

  std::string describe() const {
    return folly::to<std::string>(
        "error=", getErrorString(code), "(", code, ") msg=", msg);
  }

  const char* what() const noexcept override {
    return msg.c_str();
  }
};

// Convert an error from an HTTPSource to one that can be sent using H2/H3.
// Can be used for lib internal errors also, by passing fromSource=false
ErrorCode HTTPErrorCode2ErrorCode(HTTPErrorCode ec, bool fromSource = true);
HTTP3::ErrorCode HTTPErrorCode2HTTP3ErrorCode(HTTPErrorCode ec,
                                              bool fromSource = true);

// Convert an H2/H3 error code read off the wire to an HTTPErrorCode
HTTPErrorCode ErrorCode2HTTPErrorCode(ErrorCode ec);
HTTPErrorCode HTTP3ErrorCode2HTTPErrorCode(HTTP3::ErrorCode ec);

// Convert HTTP1xCodec::onError HTTPException to an HTTPErrorCode
HTTPErrorCode HTTPException2HTTPErrorCode(const proxygen::HTTPException& ex);

// convert HTTPError to ProxygenError (usage of ProxygenError is discouraged)
ProxygenError HTTPErrorCode2ProxygenError(HTTPErrorCode code);

// convert HTTPError to HTTPException (usage of HTTPException is discouraged)
HTTPException HTTPErrorToHTTPException(const HTTPError& err);

} // namespace proxygen::coro
