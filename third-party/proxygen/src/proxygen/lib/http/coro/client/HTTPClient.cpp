/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/HTTPClient.h"
#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/HTTPSourceReader.h"
#include "proxygen/lib/http/coro/client/CoroDNSResolver.h"
#include <folly/logging/xlog.h>

#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include <folly/SocketAddress.h>
#include <proxygen/lib/utils/URL.h>

using folly::coro::co_error;

namespace {
using namespace proxygen;
using namespace proxygen::coro;

const uint32_t kDefaultStreamWindow = 1 << 20;
const uint32_t kDefaultSessionWindow = kDefaultStreamWindow * 10;
const std::list<std::string> kDefaultNextProtocols = {"h2", "http/1.1"};
const std::list<std::string> kDefaultQUICNextProtocols = {"h3"};

HTTPCoroConnector::TLSParams makeTLSParams(
    folly::StringPiece clientCertPath,
    folly::StringPiece clientKeyPath,
    std::list<std::string> nextProtocols) {
  HTTPCoroConnector::TLSParams tlsParams(std::move(nextProtocols));
  tlsParams.caPaths = HTTPClient::getDefaultCAPaths();
  tlsParams.clientCertPath = clientCertPath;
  tlsParams.clientKeyPath = clientKeyPath;
  return tlsParams;
}

using SecureTransportImpl = HTTPClient::SecureTransportImpl;
void getConnParamsImpl(HTTPCoroConnector::ConnectionParams* connParams,
                       HTTPCoroConnector::QuicConnectionParams* qconnParams,
                       SecureTransportImpl secureTransportImpl,
                       folly::StringPiece sni,
                       const HTTPCoroConnector::TLSParams& tlsParams) {
  XCHECK(!qconnParams || (secureTransportImpl == SecureTransportImpl::FIZZ));
  HTTPCoroConnector::BaseConnectionParams* baseParams{nullptr};
  if (secureTransportImpl != SecureTransportImpl::NONE) {
    if (qconnParams) {
      baseParams = qconnParams;
      qconnParams->transportSettings
          .advertisedInitialConnectionFlowControlWindow = kDefaultSessionWindow;
      qconnParams->transportSettings
          .advertisedInitialBidiLocalStreamFlowControlWindow =
          kDefaultStreamWindow;
      // No such thing as remote bidi HTTP stream, but set it anyways
      qconnParams->transportSettings
          .advertisedInitialBidiRemoteStreamFlowControlWindow =
          kDefaultStreamWindow;
      qconnParams->transportSettings.shouldUseRecvmmsgForBatchRecv = true;
      qconnParams->transportSettings.maxRecvBatchSize = 64;
      qconnParams->transportSettings.batchingMode =
          quic::QuicBatchingMode::BATCHING_MODE_GSO;
      qconnParams->transportSettings.numGROBuffers_ = 64;
    } else {
      baseParams = connParams;
    }
    if (secureTransportImpl == SecureTransportImpl::TLS) {
      connParams->sslContext = HTTPCoroConnector::makeSSLContext(tlsParams);
    } else {
      // This can throw
      baseParams->fizzContextAndVerifier =
          HTTPCoroConnector::makeFizzClientContextAndVerifier(tlsParams);
      if (HTTPClient::getDefaultCertVerifier()) {
        baseParams->fizzContextAndVerifier.fizzCertVerifier =
            HTTPClient::getDefaultCertVerifier();
      }
    }
    baseParams->serverName = sni.str();
  }
}

std::unique_ptr<folly::IOBuf> stringToIOBuf(std::optional<std::string> str) {
  std::unique_ptr<folly::IOBuf> strBuf;
  if (str && !str->empty()) {
    auto strPtr = std::make_unique<std::string>(std::move(str).value());
    strBuf = folly::IOBuf::takeOwnership(
        strPtr->data(),
        strPtr->size(),
        [](void*, void* data) { delete static_cast<std::string*>(data); },
        strPtr.release());
  }
  return strBuf;
}

HTTPSource* makeHTTPRequestSource(
    const URL& url,
    HTTPMethod method,
    HTTPClient::RequestHeaderMap requestHeaders =
        HTTPClient::RequestHeaderMap(),
    std::optional<std::string> body = std::nullopt) {
  auto reqSource = HTTPFixedSource::makeFixedRequest(
      url.makeRelativeURL(), method, stringToIOBuf(body));
  for (auto& nameValue : requestHeaders) {
    reqSource->msg_->getHeaders().add(nameValue.first,
                                      std::move(nameValue.second));
  }
  if (!reqSource->msg_->getHeaders().exists(HTTP_HEADER_HOST)) {
    reqSource->msg_->getHeaders().add(HTTP_HEADER_HOST,
                                      url.getHostAndPortOmitDefault());
  }
  reqSource->msg_->setWantsKeepalive(true);
  reqSource->msg_->setSecure(url.isSecure());
  return reqSource;
}

HTTPSource* makeHTTPRequestSource(
    const URL& url,
    HTTPClient::RequestHeaderMap requestHeaders =
        HTTPClient::RequestHeaderMap(),
    std::optional<std::string> body = std::nullopt) {
  HTTPMethod method = (body) ? HTTPMethod::POST : HTTPMethod::GET;
  return makeHTTPRequestSource(
      url, method, std::move(requestHeaders), std::move(body));
}

folly::coro::Task<void> makeRequestReadResponse(
    HTTPCoroSession* session,
    HTTPCoroSession::RequestReservation reservation,
    HTTPSourceHolder reqSource,
    HTTPSourceReader reader,
    std::chrono::milliseconds timeout,
    Logger::SampledLoggerPtr logFn = HTTPClient::getDefaultLogImpl()) {
  Logger logger(session->acquireKeepAlive(), logFn);

  auto respSource = co_await co_awaitTry(session->sendRequest(
      logger.getRequestFilter(std::move(reqSource)), std::move(reservation)));

  if (respSource.hasException()) {
    co_yield co_error(std::move(respSource.exception()));
  }
  if (timeout.count() > 0) {
    respSource->setReadTimeout(timeout);
  }
  auto maybe = co_await co_awaitTry(
      reader.setSource(logger.getResponseFilter(std::move(*respSource)))
          .read());
  // Logger goes out of scope before reader, clear the reference
  reader.setSource(nullptr);
  // Wait for the log to happen even when cancelled
  co_await folly::coro::co_withCancellation(folly::CancellationToken(),
                                            Logger::logWhenDone(logger));
  if (maybe.hasException()) {
    co_yield co_error(std::move(maybe.exception()));
  }
  // session should be deleted automatically
  co_return;
}

} // namespace

namespace proxygen::coro {

std::vector<std::string>& HTTPClient::defaultCAPaths() {
  static std::vector<std::string> defaultCAPaths{"/etc/pki/tls/cert.pem"};
  return defaultCAPaths;
}

std::shared_ptr<fizz::CertificateVerifier>& HTTPClient::defaultCertVerifier() {
  static std::shared_ptr<fizz::CertificateVerifier> sDefaultCertVerifier;
  return sDefaultCertVerifier;
}

Logger::SampledLoggerPtr& HTTPClient::defaultLogImpl() {
  static Logger::SampledLoggerPtr logImpl =
      std::make_shared<Logger::SampledLogger>();
  return logImpl;
}

HTTPCoroConnector::ConnectionParams HTTPClient::getConnParams(
    SecureTransportImpl secureTransportImpl,
    folly::StringPiece sni,
    folly::StringPiece clientCertPath,
    folly::StringPiece clientKeyPath,
    std::list<std::string> nextProtocols) {
  HTTPCoroConnector::ConnectionParams connParams;
  if (secureTransportImpl != SecureTransportImpl::NONE &&
      nextProtocols.empty()) {
    nextProtocols = kDefaultNextProtocols;
  }
  auto tlsParams =
      makeTLSParams(clientCertPath, clientKeyPath, std::move(nextProtocols));
  getConnParamsImpl(&connParams, nullptr, secureTransportImpl, sni, tlsParams);
  return connParams;
}

HTTPCoroConnector::QuicConnectionParams HTTPClient::getQuicConnParams(
    folly::StringPiece sni,
    folly::StringPiece clientCertPath,
    folly::StringPiece clientKeyPath,
    std::list<std::string> nextProtocols) {
  if (nextProtocols.empty()) {
    nextProtocols = kDefaultQUICNextProtocols;
  }

  HTTPCoroConnector::QuicConnectionParams connParams;
  auto tlsParams = makeTLSParams(clientCertPath, clientKeyPath, nextProtocols);
  getConnParamsImpl(
      nullptr, &connParams, SecureTransportImpl::FIZZ, sni, tlsParams);
  return connParams;
}

HTTPCoroConnector::SessionParams HTTPClient::getSessionParams(
    std::optional<std::chrono::milliseconds> readTimeout) {
  HTTPCoroConnector::SessionParams sessParams =
      HTTPCoroConnector::defaultSessionParams();
  // NOLINTNEXTLINE(modernize-use-emplace)
  sessParams.settings.push_back(
      {SettingsId::INITIAL_WINDOW_SIZE, kDefaultStreamWindow});
  sessParams.connFlowControl = kDefaultSessionWindow;
  sessParams.connReadTimeout = readTimeout;
  sessParams.streamReadTimeout = readTimeout;
  return sessParams;
}

folly::coro::Task<HTTPCoroSession*> HTTPClient::getHTTPSession(
    folly::EventBase* evb,
    std::string host,
    uint16_t port,
    bool isSecure,
    bool useQuic,
    std::chrono::milliseconds connectTimeout,
    std::chrono::milliseconds readTimeout,
    std::string clientCertPath,
    std::string clientKeyPath,
    folly::Optional<std::string> serverAddress) {

  folly::SocketAddress serverAddr;
  folly::Optional<folly::SocketAddress> fallbackAddress;

  // Resolve hostname asynchronously, if necessary
  if (serverAddress.has_value()) {
    serverAddr.setFromIpPort(serverAddress.value(), port);
  } else {
    auto serverAddresses =
        co_await CoroDNSResolver::resolveHost(evb, host, connectTimeout);
    serverAddr = std::move(serverAddresses.primary);
    fallbackAddress = std::move(serverAddresses.fallback);
    serverAddr.setPort(port);
  }

  HTTPCoroConnector::ConnectionParams connParams;
  HTTPCoroConnector::QuicConnectionParams qconnParams;
  auto nextProtocols =
      useQuic ? kDefaultQUICNextProtocols : kDefaultNextProtocols;
  auto tlsParams =
      makeTLSParams(clientCertPath, clientKeyPath, std::move(nextProtocols));
  getConnParamsImpl(&connParams,
                    useQuic ? &qconnParams : nullptr,
                    // always use fizz for internal
                    ((isSecure || useQuic) ? SecureTransportImpl::FIZZ
                                           : SecureTransportImpl::NONE),
                    host,
                    tlsParams);
  auto sessParams = getSessionParams(readTimeout);
  if (useQuic) {
    co_return co_await HTTPCoroConnector::connect(
        evb, serverAddr, connectTimeout, qconnParams, sessParams);
  } else if (fallbackAddress.has_value()) {
    fallbackAddress.value().setPort(port);
    co_return co_await HTTPCoroConnector::happyEyeballsConnect(
        evb,
        std::move(serverAddr),
        std::move(fallbackAddress.value()),
        connectTimeout,
        connParams,
        sessParams);
  } else {
    co_return co_await HTTPCoroConnector::connect(
        evb, serverAddr, connectTimeout, connParams, sessParams);
  }
}

folly::coro::Task<HTTPCoroSession*> HTTPClient::getHTTPSessionViaProxy(
    HTTPCoroSession* proxySession,
    std::string host,
    uint16_t port,
    bool connectUnique,
    SecureTransportImpl secureTransportImpl,
    std::chrono::milliseconds connectTimeout,
    std::chrono::milliseconds readTimeout,
    std::string clientCertPath,
    std::string clientKeyPath) {
  HTTPCoroConnector::ConnectionParams connParams;
  auto tlsParams =
      makeTLSParams(clientCertPath, clientKeyPath, kDefaultNextProtocols);
  getConnParamsImpl(&connParams, nullptr, secureTransportImpl, host, tlsParams);
  auto reservation = proxySession->reserveRequest();
  if (reservation.hasException()) {
    co_yield co_error(std::move(reservation.exception()));
  }
  auto res = co_await HTTPCoroConnector::proxyConnect(
      proxySession,
      std::move(*reservation),
      folly::to<std::string>(host, ":", port),
      connectUnique,
      connectTimeout,
      connParams,
      getSessionParams(readTimeout));
  co_return res;
}

folly::coro::Task<HTTPClient::Response> HTTPClient::get(
    folly::EventBase* evb,
    std::string urlStr,
    std::chrono::milliseconds timeout,
    bool useQuic,
    RequestHeaderMap requestHeaders) {
  Response resp;
  co_await get(evb,
               std::move(urlStr),
               timeout,
               makeDefaultReader(resp),
               useQuic,
               std::move(requestHeaders));
  co_return resp;
}

folly::coro::Task<HTTPClient::Response> HTTPClient::get(
    HTTPCoroSession* session,
    URL url,
    std::chrono::milliseconds timeout,
    RequestHeaderMap requestHeaders) {
  Response resp;
  co_await get(session,
               std::move(url),
               makeDefaultReader(resp),
               timeout,
               std::move(requestHeaders));
  co_return resp;
}

folly::coro::Task<HTTPClient::Response> HTTPClient::get(
    HTTPCoroSession* session,
    HTTPCoroSession::RequestReservation reservation,
    URL url,
    std::chrono::milliseconds timeout,
    RequestHeaderMap requestHeaders) {
  Response resp;
  co_await get(session,
               std::move(reservation),
               std::move(url),
               makeDefaultReader(resp),
               timeout,
               std::move(requestHeaders));
  co_return resp;
}

folly::coro::Task<void> HTTPClient::get(HTTPCoroSession* session,
                                        URL url,
                                        HTTPSourceReader reader,
                                        std::chrono::milliseconds timeout,
                                        RequestHeaderMap requestHeaders) {
  if (!url.isValid() || !url.hasHost()) {
    co_yield co_error(std::runtime_error(
        folly::to<std::string>("Invalid url: ", url.getUrl())));
  }
  auto reservation = session->reserveRequest();
  if (reservation.hasException()) {
    co_yield co_error(std::move(reservation.exception()));
  }
  co_await makeRequestReadResponse(
      session,
      std::move(*reservation),
      makeHTTPRequestSource(url, std::move(requestHeaders)),
      std::move(reader),
      timeout);
}

folly::coro::Task<void> HTTPClient::get(
    HTTPCoroSession* session,
    HTTPCoroSession::RequestReservation reservation,
    URL url,
    HTTPSourceReader reader,
    std::chrono::milliseconds timeout,
    RequestHeaderMap requestHeaders) {
  if (!url.isValid() || !url.hasHost()) {
    co_yield co_error(std::runtime_error(
        folly::to<std::string>("Invalid url: ", url.getUrl())));
  }
  co_await makeRequestReadResponse(
      session,
      std::move(reservation),
      makeHTTPRequestSource(url, std::move(requestHeaders)),
      std::move(reader),
      timeout);
}

folly::coro::Task<void> HTTPClient::get(folly::EventBase* evb,
                                        std::string urlStr,
                                        std::chrono::milliseconds timeout,
                                        HTTPSourceReader reader,
                                        bool useQuic,
                                        RequestHeaderMap requestHeaders) {

  URL url(urlStr);
  if (!url.isValid() || !url.hasHost()) {
    co_yield co_error(std::runtime_error(
        folly::to<std::string>("Invalid url: ", url.getUrl())));
  }
  auto session = co_await getHTTPSession(evb,
                                         url.getHost(),
                                         url.getPort(),
                                         url.isSecure(),
                                         useQuic,
                                         timeout,
                                         timeout);

  auto sessionHolder = session->acquireKeepAlive();
  SCOPE_EXIT {
    auto weakSession = sessionHolder.get();
    if (weakSession) {
      weakSession->initiateDrain();
    }
  };
  co_await get(
      session, url, std::move(reader), timeout, std::move(requestHeaders));
}

folly::coro::Task<HTTPClient::Response> HTTPClient::post(
    folly::EventBase* evb,
    std::string urlStr,
    std::string body,
    std::chrono::milliseconds timeout,
    bool useQuic,
    RequestHeaderMap requestHeaders) {
  Response resp;
  co_await post(evb,
                std::move(urlStr),
                std::move(body),
                timeout,
                makeDefaultReader(resp),
                useQuic,
                std::move(requestHeaders));
  co_return resp;
}

folly::coro::Task<HTTPClient::Response> HTTPClient::post(
    HTTPCoroSession* session,
    URL url,
    std::string body,
    std::chrono::milliseconds timeout,
    RequestHeaderMap requestHeaders) {
  Response resp;
  co_await post(session,
                std::move(url),
                std::move(body),
                makeDefaultReader(resp),
                timeout,
                std::move(requestHeaders));
  co_return resp;
}

folly::coro::Task<void> HTTPClient::post(folly::EventBase* evb,
                                         std::string urlStr,
                                         std::string body,
                                         std::chrono::milliseconds timeout,
                                         HTTPSourceReader reader,
                                         bool useQuic,
                                         RequestHeaderMap requestHeaders) {
  URL url(urlStr);
  if (!url.isValid() || !url.hasHost()) {
    co_yield co_error(
        std::runtime_error(folly::to<std::string>("Invalid url: ", urlStr)));
  }

  auto session = co_await getHTTPSession(evb,
                                         url.getHost(),
                                         url.getPort(),
                                         url.isSecure(),
                                         useQuic,
                                         timeout,
                                         timeout);

  auto sessionHolder = session->acquireKeepAlive();
  SCOPE_EXIT {
    auto weakSession = sessionHolder.get();
    if (weakSession) {
      weakSession->initiateDrain();
    }
  };
  auto reservation = session->reserveRequest();
  if (reservation.hasException()) {
    co_yield co_error(std::move(reservation.exception()));
  }
  co_await makeRequestReadResponse(
      session,
      std::move(*reservation),
      makeHTTPRequestSource(url, std::move(requestHeaders), std::move(body)),
      std::move(reader),
      timeout);
}

folly::coro::Task<void> HTTPClient::post(HTTPCoroSession* session,
                                         URL url,
                                         std::string body,
                                         HTTPSourceReader reader,
                                         std::chrono::milliseconds timeout,
                                         RequestHeaderMap requestHeaders) {
  if (!url.isValid() || !url.hasHost()) {
    co_yield co_error(std::runtime_error(
        folly::to<std::string>("Invalid url: ", url.getUrl())));
  }
  auto reservation = session->reserveRequest();
  if (reservation.hasException()) {
    co_yield co_error(std::move(reservation.exception()));
  }
  co_await makeRequestReadResponse(
      session,
      std::move(*reservation),
      makeHTTPRequestSource(url, std::move(requestHeaders), std::move(body)),
      std::move(reader),
      timeout);
}

folly::coro::Task<HTTPClient::Response> HTTPClient::readResponse(
    HTTPSourceHolder responseSource) {
  Response resp;
  HTTPSourceReader reader = makeDefaultReader(resp);
  reader.setSource(std::move(responseSource));
  co_await reader.read();
  co_return resp;
}

folly::coro::Task<void> HTTPClient::request(
    HTTPCoroSession* session,
    HTTPCoroSession::RequestReservation reservation,
    HTTPSourceHolder reqSource,
    HTTPSourceReader reader,
    std::chrono::milliseconds timeout,
    Logger::SampledLoggerPtr logger) {
  return makeRequestReadResponse(session,
                                 std::move(reservation),
                                 std::move(reqSource),
                                 std::move(reader),
                                 timeout,
                                 std::move(logger));
}

folly::coro::Task<void> HTTPClient::request(
    HTTPCoroSession* session,
    HTTPCoroSession::RequestReservation reservation,
    HTTPMethod method,
    const URL& url,
    RequestHeaderMap requestHeaders,
    HTTPSourceReader reader,
    std::optional<std::string> body,
    std::chrono::milliseconds timeout,
    Logger::SampledLoggerPtr logger) {
  return makeRequestReadResponse(
      session,
      std::move(reservation),
      makeHTTPRequestSource(
          url, method, std::move(requestHeaders), std::move(body)),
      std::move(reader),
      timeout,
      std::move(logger));
}

HTTPSourceReader HTTPClient::makeDefaultReader(HTTPClient::Response& response) {
  HTTPSourceReader reader;
  reader
      .onHeaders([&response](std::unique_ptr<HTTPMessage> headers,
                             bool /*isFinal*/,
                             bool /*eom*/) {
        response.headers = std::move(headers);
        response.headers->dumpMessage(0);
        return HTTPSourceReader::Continue;
      })
      .onBody([&response](BufQueue body, bool) {
        response.body.append(body.move());
        return HTTPSourceReader::Continue;
      })
      .onTrailers([&response](std::unique_ptr<HTTPHeaders> trailers) {
        response.trailers = std::move(trailers);
      })
      .onError([](HTTPSourceReader::ErrorContext ec, const HTTPError& error) {
        HTTPError e(error);
        std::string_view ctx = (ec == HTTPSourceReader::ErrorContext::HEADERS)
                                   ? "headers"
                                   : "body";
        e.msg = folly::to<std::string>(
            "Error receiving response ", ctx, ", err=", error.msg);
        throw e;
      });
  // ignore all other events
  return reader;
}

} // namespace proxygen::coro
