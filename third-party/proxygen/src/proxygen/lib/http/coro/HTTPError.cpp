/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPError.h"
#include <folly/logging/xlog.h>

#include <folly/container/F14Map.h>
#include <proxygen/lib/http/HTTPException.h>

namespace {
using namespace proxygen;
using namespace proxygen::coro;

bool checkInvalidAppErrorCode(HTTPErrorCode ec) {
  switch (ec) {
    // Internal H2 errors
    case HTTPErrorCode::PROTOCOL_ERROR:
    case HTTPErrorCode::SETTINGS_TIMEOUT:
    case HTTPErrorCode::STREAM_CLOSED:
    case HTTPErrorCode::FRAME_SIZE_ERROR:
    case HTTPErrorCode::COMPRESSION_ERROR:

    // Internal lib errors
    case HTTPErrorCode::INVALID_STATE_TRANSITION:
    case HTTPErrorCode::WRITE_TIMEOUT:
    case HTTPErrorCode::TRANSPORT_EOF:
    case HTTPErrorCode::TRANSPORT_READ_ERROR:
    case HTTPErrorCode::TRANSPORT_WRITE_ERROR:
    case HTTPErrorCode::HEADER_PARSE_ERROR:
    case HTTPErrorCode::BODY_PARSE_ERROR:
    case HTTPErrorCode::DROPPED:
    case HTTPErrorCode::CORO_CANCELLED:
      XLOG(WARNING) << "Application supplied internal error code ec="
                    << uint16_t(ec);
      return true;

    // Internal H3 errors
    case HTTPErrorCode::GENERAL_PROTOCOL_ERROR:
    case HTTPErrorCode::STREAM_CREATION_ERROR:
    case HTTPErrorCode::CLOSED_CRITICAL_STREAM:
    case HTTPErrorCode::FRAME_UNEXPECTED:
    case HTTPErrorCode::FRAME_ERROR:
    case HTTPErrorCode::ID_ERROR:
    case HTTPErrorCode::SETTINGS_ERROR:
    case HTTPErrorCode::MISSING_SETTINGS:
    case HTTPErrorCode::INCOMPLETE_REQUEST:
    case HTTPErrorCode::QPACK_DECOMPRESSION_FAILED:
    case HTTPErrorCode::QPACK_ENCODER_STREAM_ERROR:
    case HTTPErrorCode::QPACK_DECODER_STREAM_ERROR:
      XLOG(WARNING) << "Application supplied internal H3 error code ec="
                    << uint16_t(ec);
      return true;
    default:
      return false;
  }
}

// Maybe tolower or camel case?
#define HTTP_ERROR_SET_NAME(error) name = #error;
#define HTTP_ERROR_SET_VALUE_TO_NAME(value) \
  errorStringMap[HTTPErrorCode(value)] = std::move(name)

folly::F14FastMap<HTTPErrorCode, std::string> getErrorStringMap() {
  folly::F14FastMap<HTTPErrorCode, std::string> errorStringMap;
  std::string name;
  HTTP_ERROR_GEN_VALUES(HTTP_ERROR_SET_NAME, HTTP_ERROR_SET_VALUE_TO_NAME);
  return errorStringMap;
}

} // namespace

namespace proxygen::coro {

const std::string& getErrorString(HTTPErrorCode ec) {
  static const auto sErrorStringMap = getErrorStringMap();
  auto it = sErrorStringMap.find(ec);
  if (it == sErrorStringMap.end()) {
    static const std::string unknownError("UNKNOWN_ERROR");
    return unknownError;
  }
  return it->second;
}

// Convert an error from an HTTPSource to one that can be sent using H2.
ErrorCode HTTPErrorCode2ErrorCode(HTTPErrorCode ec, bool fromSource) {
  if (fromSource && checkInvalidAppErrorCode(ec)) {
    return ErrorCode::INTERNAL_ERROR;
  }
  switch (ec) {
    case HTTPErrorCode::READ_TIMEOUT:
    case HTTPErrorCode::_H3_INTERNAL_ERROR:
    case HTTPErrorCode::INTERNAL_ERROR:
      return ErrorCode::INTERNAL_ERROR;

    case HTTPErrorCode::REQUEST_REJECTED:
    case HTTPErrorCode::REFUSED_STREAM:
      return ErrorCode::REFUSED_STREAM;

    case HTTPErrorCode::REQUEST_CANCELLED:
    case HTTPErrorCode::CANCEL:
      return ErrorCode::CANCEL;

    case HTTPErrorCode::_H3_CONNECT_ERROR:
    case HTTPErrorCode::CONNECT_ERROR:
      return ErrorCode::CONNECT_ERROR;

    case HTTPErrorCode::EXCESSIVE_LOAD:
    case HTTPErrorCode::ENHANCE_YOUR_CALM:
      return ErrorCode::ENHANCE_YOUR_CALM;

    case HTTPErrorCode::VERSION_FALLBACK:
    case HTTPErrorCode::HTTP_1_1_REQUIRED:
      return ErrorCode::HTTP_1_1_REQUIRED;

    case HTTPErrorCode::_H3_NO_ERROR:
    case HTTPErrorCode::NO_ERROR:
      return ErrorCode::NO_ERROR;

    // There is no H3 version of this
    case HTTPErrorCode::INADEQUATE_SECURITY:
      return ErrorCode(ec);

    // This is really an internal code we need to map sometimes
    case HTTPErrorCode::MESSAGE_ERROR:
    case HTTPErrorCode::PROTOCOL_ERROR:
    case HTTPErrorCode::GENERAL_PROTOCOL_ERROR:
      XCHECK(!fromSource);
      return ErrorCode::PROTOCOL_ERROR;

    case HTTPErrorCode::FLOW_CONTROL_ERROR:
      XCHECK(!fromSource);
      return ErrorCode::FLOW_CONTROL_ERROR;

    default:
      // Any unknown custom error codes are mapped to INTERNAL_ERROR here.
      return ErrorCode::INTERNAL_ERROR;
  }
}

// Convert an error from an HTTPSource to one that can be sent using H3.
HTTP3::ErrorCode HTTPErrorCode2HTTP3ErrorCode(HTTPErrorCode ec,
                                              bool fromSource) {
  if (fromSource && checkInvalidAppErrorCode(ec)) {
    return HTTP3::ErrorCode::HTTP_INTERNAL_ERROR;
  }
  switch (ec) {
    // These are all OK to come from the application, return the ErrorCode flav
    case HTTPErrorCode::READ_TIMEOUT:
    case HTTPErrorCode::_H3_INTERNAL_ERROR:
    case HTTPErrorCode::INTERNAL_ERROR:
      return HTTP3::ErrorCode::HTTP_INTERNAL_ERROR;

    case HTTPErrorCode::REQUEST_REJECTED:
    case HTTPErrorCode::REFUSED_STREAM:
      return HTTP3::ErrorCode::HTTP_REQUEST_REJECTED;

    case HTTPErrorCode::REQUEST_CANCELLED:
    case HTTPErrorCode::CANCEL:
      return HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED;

    case HTTPErrorCode::_H3_CONNECT_ERROR:
    case HTTPErrorCode::CONNECT_ERROR:
      return HTTP3::ErrorCode::HTTP_CONNECT_ERROR;

    case HTTPErrorCode::EXCESSIVE_LOAD:
    case HTTPErrorCode::ENHANCE_YOUR_CALM:
      return HTTP3::ErrorCode::HTTP_EXCESSIVE_LOAD;

    case HTTPErrorCode::VERSION_FALLBACK:
    case HTTPErrorCode::HTTP_1_1_REQUIRED:
      return HTTP3::ErrorCode::HTTP_VERSION_FALLBACK;

    case HTTPErrorCode::_H3_NO_ERROR:
    case HTTPErrorCode::NO_ERROR:
      return HTTP3::ErrorCode::HTTP_NO_ERROR;

    // There is no H3 version of this
    case HTTPErrorCode::INADEQUATE_SECURITY:
      XLOG(WARNING) << "Got INADEQUATE_SECURITY for H3 conn";
      return HTTP3::ErrorCode::HTTP_INTERNAL_ERROR;

    // This is really an internal code we need to map sometimes
    case HTTPErrorCode::MESSAGE_ERROR:
    case HTTPErrorCode::PROTOCOL_ERROR:
    case HTTPErrorCode::GENERAL_PROTOCOL_ERROR:
      XCHECK(!fromSource);
      return HTTP3::ErrorCode::HTTP_GENERAL_PROTOCOL_ERROR;

    default:
      if (!fromSource && ((ec >= HTTPErrorCode::_H3_NO_ERROR &&
                           ec <= HTTPErrorCode::VERSION_FALLBACK) ||
                          (ec >= HTTPErrorCode::QPACK_DECOMPRESSION_FAILED &&
                           ec <= HTTPErrorCode::QPACK_DECODER_STREAM_ERROR))) {
        // Some other H3 error from the library
        return HTTP3::ErrorCode(ec);
      }
      // Any unknown custom error codes are mapped to INTERNAL_ERROR here.
      return HTTP3::ErrorCode::HTTP_INTERNAL_ERROR;
  }
}

// TODO: for the below functions -- we should normalize the codec error codes
// with the same meaning and return only one code
// Convert an H2 error code read off the wire to an HTTPErrorCode
HTTPErrorCode ErrorCode2HTTPErrorCode(ErrorCode ec) {
  XCHECK(ec != ErrorCode::NO_ERROR);
  if (ec >= ErrorCode::PROTOCOL_ERROR && ec <= ErrorCode::HTTP_1_1_REQUIRED) {
    return HTTPErrorCode(ec);
  }
  // Any unknown custom error codes are mapped to PROTOCOL_ERROR here.
  return HTTPErrorCode::PROTOCOL_ERROR;
}

// Convert an H3 error code read off the wire to an HTTPErrorCode
HTTPErrorCode HTTP3ErrorCode2HTTPErrorCode(HTTP3::ErrorCode ec) {
  XCHECK(ec != HTTP3::ErrorCode::HTTP_NO_ERROR);
  if ((ec >= HTTP3::ErrorCode::HTTP_GENERAL_PROTOCOL_ERROR &&
       ec <= HTTP3::ErrorCode::HTTP_VERSION_FALLBACK) ||
      (ec >= HTTP3::ErrorCode::HTTP_QPACK_DECOMPRESSION_FAILED &&
       ec <= HTTP3::ErrorCode::HTTP_QPACK_DECODER_STREAM_ERROR)) {
    return HTTPErrorCode(ec);
  }
  // Any unknown custom error codes are mapped to PROTOCOL_ERROR here.
  return HTTPErrorCode::PROTOCOL_ERROR;
}

// Convert HTTPCodec::onError HTTPException to an HTTPErrorCode
HTTPErrorCode HTTPException2HTTPErrorCode(const proxygen::HTTPException& ex) {
  if (ex.hasCodecStatusCode()) {
    // These come from HTTP2Codec
    return ErrorCode2HTTPErrorCode(ex.getCodecStatusCode());
  } else if (ex.hasHttp3ErrorCode()) {
    return HTTP3ErrorCode2HTTPErrorCode(ex.getHttp3ErrorCode());
  } else if (ex.hasProxygenError()) {
    // These come from HTTP1xCodec
    auto err = ex.getProxygenError();
    XCHECK_NE(err, proxygen::kErrorNone);
    switch (err) {
      case proxygen::kErrorParseHeader:
        return HTTPErrorCode::HEADER_PARSE_ERROR;
      case proxygen::kErrorParseBody:
        return HTTPErrorCode::BODY_PARSE_ERROR;
      case proxygen::kErrorEOF:
        return HTTPErrorCode::TRANSPORT_EOF;
      case proxygen::kErrorUnknown:
      default:
        return HTTPErrorCode::PROTOCOL_ERROR;
    }
  }
  return HTTPErrorCode::PROTOCOL_ERROR;
}

ProxygenError HTTPErrorCode2ProxygenError(HTTPErrorCode code) {
  switch (code) {
    case HTTPErrorCode::NO_ERROR:
    case HTTPErrorCode::_H3_NO_ERROR:
      return kErrorNone;
    case HTTPErrorCode::GENERAL_PROTOCOL_ERROR:
    case HTTPErrorCode::MESSAGE_ERROR:
      return kErrorMessage;
    case HTTPErrorCode::STREAM_CLOSED:
    case HTTPErrorCode::PROTOCOL_ERROR:
      return kErrorStreamAbort;
    case HTTPErrorCode::CANCEL:
    case HTTPErrorCode::REQUEST_CANCELLED:
    case HTTPErrorCode::CORO_CANCELLED:
      return kErrorCanceled;
    case HTTPErrorCode::REFUSED_STREAM:
    case HTTPErrorCode::REQUEST_REJECTED:
      return kErrorStreamUnacknowledged;
    case HTTPErrorCode::INVALID_STATE_TRANSITION:
      return kErrorIngressStateTransition;
    case HTTPErrorCode::HEADER_PARSE_ERROR:
      return kErrorParseHeader;
    case HTTPErrorCode::BODY_PARSE_ERROR:
    case HTTPErrorCode::CONTENT_LENGTH_MISMATCH:
      return kErrorParseBody;
    case HTTPErrorCode::TRANSPORT_READ_ERROR:
      return kErrorConnectionReset;
    case HTTPErrorCode::TRANSPORT_WRITE_ERROR:
      return kErrorWrite;
    case HTTPErrorCode::TRANSPORT_EOF:
      return kErrorEOF;
    case HTTPErrorCode::READ_TIMEOUT:
      return kErrorTimeout;
    case HTTPErrorCode::DROPPED:
      return kErrorDropped;
    default:
      return kErrorStreamAbort;
  }
}

HTTPException HTTPErrorToHTTPException(const HTTPError& err) {
  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS, err.what());
  ex.setProxygenError(HTTPErrorCode2ProxygenError(err.code));
  ErrorCode ec = static_cast<ErrorCode>(err.code);
  if (ec >= ErrorCode::PROTOCOL_ERROR && ec <= ErrorCode::HTTP_1_1_REQUIRED) {
    ex.setCodecStatusCode(ec);
  }
  if (err.httpMessage) {
    ex.setPartialMsg(
        std::make_unique<HTTPMessage>(std::move(*err.httpMessage)));
  }
  return ex;
}
} // namespace proxygen::coro
