/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/HTTPSourceReader.h"
#include "proxygen/lib/http/coro/client/HTTPCoroConnector.h"
#include "proxygen/lib/http/coro/filters/Logger.h"
#include <folly/coro/Task.h>
#include <folly/io/IOBufQueue.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/utils/URL.h>

#include <string>
namespace fizz {
class CertificateVerifier;
}

namespace proxygen::coro {

/**
 * Extremely simple coroutine enabled HTTP client APIs.  These APIs take an
 * absolute URL, and return a struct containing the final headers and the body
 * of the response, or yield an exception.
 *
 * This is a work in progress.  Prefer extending or modifying these APIs as
 * needed, rather than copying and forking this code.
 *
 * Steps:
 *   1.  Parse the URL for host/port
 *   2.  Perform a DNS resolution, if needed
 *   3.  Establish TCP or TLS connection, as required
 *   4.  Send HTTP request
 *   5.  Read HTTP response, coalescing the body into a string
 *
 * If the response is non-200, an exception is thrown.  If any errors occur
 * before receiving the end of the response, and exception is thrown and the
 * partial response is not available.
 */
class HTTPClient {
 public:
  struct Response {
    std::unique_ptr<HTTPMessage> headers;
    folly::IOBufQueue body{folly::IOBufQueue::cacheChainLength()};
    std::unique_ptr<HTTPHeaders> trailers;
  };

  using RequestHeaderMap = std::map<std::string, std::string>;

  // Reads the response into a Response object
  static folly::coro::Task<Response> get(
      folly::EventBase* evb,
      std::string url,
      std::chrono::milliseconds timeout,
      bool useQuic = false,
      RequestHeaderMap requestHeaders = RequestHeaderMap());

  // Same as above, but takes an existing session.  timeout=0 defaults to the
  // session timeout
  static folly::coro::Task<Response> get(
      HTTPCoroSession* session,
      URL url,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      RequestHeaderMap requestHeaders = RequestHeaderMap());

  // Same as above, but takes an existing session and reservation.
  // timeout=0 defaults to the session timeout
  static folly::coro::Task<Response> get(
      HTTPCoroSession* session,
      HTTPCoroSession::RequestReservation reservation,
      URL url,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      RequestHeaderMap requestHeaders = RequestHeaderMap());

  // Reads the response using a custom HTTPSourceReader
  static folly::coro::Task<void> get(
      folly::EventBase* evb,
      std::string url,
      std::chrono::milliseconds timeout,
      HTTPSourceReader reader,
      bool useQuic = false,
      RequestHeaderMap requestHeaders = RequestHeaderMap());

  // Same as above, but takes an existing session.  timeout=0 defaults to the
  // session timeout
  static folly::coro::Task<void> get(
      HTTPCoroSession* session,
      URL url,
      HTTPSourceReader reader,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      RequestHeaderMap requestHeaders = RequestHeaderMap());

  // Same as above, but takes an existing session and reservation.
  // timeout=0 defaults to the session timeout
  static folly::coro::Task<void> get(
      HTTPCoroSession* session,
      HTTPCoroSession::RequestReservation reservation,
      URL url,
      HTTPSourceReader reader,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      RequestHeaderMap requestHeaders = RequestHeaderMap());

  // Sends a post and reads response into a Response object
  static folly::coro::Task<Response> post(
      folly::EventBase* evb,
      std::string url,
      std::string body,
      std::chrono::milliseconds timeout,
      bool useQuic = false,
      RequestHeaderMap requestHeaders = RequestHeaderMap());

  // Same as above, but takes an existing session.  timeout=0 defaults to the
  // session timeout
  static folly::coro::Task<Response> post(
      HTTPCoroSession* session,
      URL url,
      std::string body,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      RequestHeaderMap requestHeaders = RequestHeaderMap());

  // Sends a post, and reads response using a custom HTTPSourceReader
  static folly::coro::Task<void> post(
      folly::EventBase* evb,
      std::string url,
      std::string body,
      std::chrono::milliseconds timeout,
      HTTPSourceReader reader,
      bool useQuic = false,
      RequestHeaderMap requestHeaders = RequestHeaderMap());

  // Same as above, but takes an existing session.  timeout=0 defaults to the
  // session timeout
  static folly::coro::Task<void> post(
      HTTPCoroSession* session,
      URL url,
      std::string body,
      HTTPSourceReader reader,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      RequestHeaderMap requestHeaders = RequestHeaderMap());

  static folly::coro::Task<void> request(
      HTTPCoroSession* session,
      HTTPCoroSession::RequestReservation reservation,
      HTTPSourceHolder reqSource,
      HTTPSourceReader reader,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      Logger::SampledLoggerPtr logger = nullptr);

  static folly::coro::Task<void> request(
      HTTPCoroSession* session,
      HTTPCoroSession::RequestReservation reservation,
      HTTPMethod method,
      const URL& url,
      RequestHeaderMap requestHeaders,
      HTTPSourceReader reader,
      std::optional<std::string> body = std::nullopt,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      Logger::SampledLoggerPtr logger = nullptr);

  // Reads the response from HTTPSourceHolder
  static folly::coro::Task<Response> readResponse(
      HTTPSourceHolder responseSource);

  static HTTPSourceReader makeDefaultReader(HTTPClient::Response& response);

  static void setDefaultCAPaths(std::vector<std::string> caPaths) {
    defaultCAPaths() = std::move(caPaths);
  }

  static const std::vector<std::string>& getDefaultCAPaths() {
    return defaultCAPaths();
  }

  static void setDefaultFizzCertVerifier(
      std::shared_ptr<fizz::CertificateVerifier> v) {
    defaultCertVerifier() = v;
  }

  static std::shared_ptr<fizz::CertificateVerifier> getDefaultCertVerifier() {
    return defaultCertVerifier();
  }

  static void setDefaultLogImpl(Logger::SampledLoggerPtr logImpl) {
    defaultLogImpl() = std::move(logImpl);
  }

  static Logger::SampledLoggerPtr& getDefaultLogImpl() {
    return defaultLogImpl();
  }

  /**
   * Get client connection parameters.  Can be used to help set up an
   * HTTPCoroSessionPool.
   */
  enum class SecureTransportImpl { NONE, TLS, FIZZ };
  static HTTPCoroConnector::ConnectionParams getConnParams(
      SecureTransportImpl secureTransportImpl,
      folly::StringPiece sni = folly::StringPiece(),
      folly::StringPiece clientCertPath = folly::StringPiece(),
      folly::StringPiece clientKeyPath = folly::StringPiece(),
      std::list<std::string> nextProtocols = {});

  static HTTPCoroConnector::QuicConnectionParams getQuicConnParams(
      folly::StringPiece sni = folly::StringPiece(),
      folly::StringPiece clientCertPath = folly::StringPiece(),
      folly::StringPiece clientKeyPath = folly::StringPiece(),
      std::list<std::string> nextProtocols = {});

  /**
   * Get a SessionParams with sensible defaults. This creates params with the
   * same defaults as `getHTTPSession`, so it can be used with
   * `HTTPCoroSession` pool to create sessions that have the same behavior as
   * those acquired from `getHTTPSession`.
   *
   * `readTimeout` is used for both the connection and stream read timeout
   * (`connReadTimeout` and `streamReadTimeout`).
   */
  static HTTPCoroConnector::SessionParams getSessionParams(
      std::optional<std::chrono::milliseconds> readTimeout = std::nullopt);

  /*
   * Get an HTTPCoroSession.  This can be useful if you want to send multiple
   * sequential requests from a single session.  For full pooling support see
   * HTTPCoroSessionPool.
   *
   * `readTimeout` is used for both the connection and stream read timeout
   * (`connReadTimeout` and `streamReadTimeout`).
   */
  static folly::coro::Task<HTTPCoroSession*> getHTTPSession(
      folly::EventBase* evb,
      std::string host,
      uint16_t port,
      bool isSecure, // todo, combine with useQuic
      bool useQuic,
      std::chrono::milliseconds connectTimeout,
      std::chrono::milliseconds readTimeout,
      std::string clientCertPath = "",
      std::string clientKeyPath = "",
      folly::Optional<std::string> serverAddress = folly::none);

  /**
   * Returns an HTTPCoroSession that is using an HTTP CONNECT stream on
   * proxySession as its transport.
   * The parameters refer to the destination (host, port,
   *  secureTransportImpl, etc).
   * connectUnique is true if this session should take ownership of the
   * proxySession.  If false, proxySession can be used for other requests
   * to to the proxy.
   *
   * `readTimeout` is used for both the connection and stream read timeout
   * (`connReadTimeout` and `streamReadTimeout`).
   */
  static folly::coro::Task<HTTPCoroSession*> getHTTPSessionViaProxy(
      HTTPCoroSession* proxySession,
      std::string host,
      uint16_t port,
      bool connectUnique,
      SecureTransportImpl secureTransportImpl,
      std::chrono::milliseconds connectTimeout,
      std::chrono::milliseconds readTimeout,
      std::string clientCertPath = "",
      std::string clientKeyPath = "");

 private:
  static std::vector<std::string>& defaultCAPaths();

  static std::shared_ptr<fizz::CertificateVerifier>& defaultCertVerifier();

  static Logger::SampledLoggerPtr& defaultLogImpl();
};

} // namespace proxygen::coro
