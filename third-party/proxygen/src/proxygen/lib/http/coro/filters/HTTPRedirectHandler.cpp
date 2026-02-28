/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/HTTPRedirectHandler.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include <folly/logging/xlog.h>

namespace {
bool isRedirectable(uint16_t code,
                    folly::Optional<proxygen::HTTPMethod> maybeMethod) {
  if (!maybeMethod) {
    return false;
  }
  auto method = *maybeMethod;
  // According to spec, 303 response code is a redirect from a POST that
  // should include a Location header but redirect as a GET
  // reference: http://tools.ietf.org/html/rfc2616#section-10.3.4
  if (code == 303 && method == proxygen::HTTPMethod::POST) {
    return true;
  }

  if (method != proxygen::HTTPMethod::GET &&
      method != proxygen::HTTPMethod::HEAD) {
    return false;
  }

  // RFC suggests 303 is to be used to redirect POST to GET
  // http://tools.ietf.org/html/rfc2616#section-10.3.4
  // but youtube is sending it from GET requests so lets support it
  return code == 301 || code == 302 || code == 303 || code == 307 ||
         code == 308;
}
} // namespace

namespace proxygen::coro {

using folly::coro::co_error;

folly::coro::Task<HTTPHeaderEvent>
HTTPRedirectHandler::ResponseFilter::readHeaderEvent() {
  size_t numRedirects = 0;

  while (true) {
    auto headerEvent = co_await readHeaderEventImpl();
    auto statusCode = headerEvent.headers->getStatusCode();
    if (!isRedirectable(statusCode, handler_.request_->getMethod())) {
      co_return headerEvent;
    }

    headerEvent.headers->dumpMessage(4);
    folly::Optional<URL> newUrl;
    {
      auto abortRequest =
          folly::makeGuard([this] { HTTPSourceFilter::stopReading(); });
      if (headerEvent.eom) {
        abortRequest.dismiss();
      }
      auto const& location = headerEvent.headers->getHeaders().getSingleOrEmpty(
          HTTP_HEADER_LOCATION);

      if (location.empty()) {
        co_yield co_error(Exception(Exception::Type::InvalidRedirect,
                                    "Missing location header for redirect"));
      } else if (++numRedirects > handler_.maxRedirects_) {
        co_yield co_error(Exception(Exception::Type::MaxRedirects,
                                    "Exceeded maximum redirect depth"));
      } else if (!ParseURL::isSupportedScheme(location)) {
        co_yield co_error(Exception(Exception::Type::UnsupportedScheme,
                                    "Unsupported Scheme"));
      }
      newUrl = handler_.getRedirectDestination(location);
      if (!newUrl) {
        co_yield co_error(Exception(Exception::Type::InvalidRedirect,
                                    "Unparsable redirect location"));
      }

      handler_.prepareRequest(newUrl->getUrl(), *newUrl, statusCode);
    }

    co_await folly::coro::co_withCancellation(
        cancellationSource_.getToken(), handler_.redirect(std::move(*newUrl)));
  }
}

void HTTPRedirectHandler::ResponseFilter::stopReading(
    folly::Optional<const HTTPErrorCode> error) {
  cancellationSource_.requestCancellation();
  if (readable()) {
    HTTPSourceFilter::stopReading(error);
  }
}

folly::Optional<URL> HTTPRedirectHandler::getRedirectDestination(
    const std::string& location) const {
  auto url = ParseURL::getRedirectDestination(
      request_->getURL(),
      request_->getScheme(),
      location,
      request_->getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST));
  if (!url) {
    return folly::none;
  }
  return URL(*url);
}

/**
 * Strips various headers (Host which is set in another filter, Cookie
 * and Authorization headers) and sets a new url from the location which
 * may be absolute or relative.  The spec is unclear about what to do with
 * request headers during a redirect, so I followed some advice from this
 * discussion:
 *
 * https://code.google.com/p/go/issues/detail?id=4800&q=request%20header
 *
 * Since we are building a more restricted HTTP client, I started with a
 * basic allowlist, but even that grew large pretty quickly.  Seems
 * reasonable to just not forward on known private headers, even for same
 * origin requests.
 */
void HTTPRedirectHandler::prepareRequest(const std::string& newUrlStr,
                                         const URL& newUrl,
                                         uint16_t statusCode) {
  if (request_->getMethod() == HTTPMethod::POST && statusCode == 303) {
    request_->setMethod(HTTPMethod::GET);
    request_->getHeaders().remove(HTTP_HEADER_CONTENT_LENGTH);
    requestBody_.move();
  }

  if (sessionFactory_->requiresAbsoluteURLs()) {
    request_->setURL(newUrlStr);
  } else {
    request_->setURL(newUrl.makeRelativeURL());
    request_->getHeaders().set(HTTP_HEADER_HOST,
                               newUrl.getHostAndPortOmitDefault());
  }

  // Strip sensitive headers as described in function comment
  // TODO(t4397642) determine if there are more headers we should strip
  request_->getHeaders().remove(HTTP_HEADER_AUTHORIZATION);
  request_->getHeaders().remove(HTTP_HEADER_COOKIE);
}

folly::coro::Task<void> HTTPRedirectHandler::redirect(URL url) {
  if (redirectCallback_) {
    redirectCallback_(url.getUrl());
  }

  auto res = co_await sessionFactory_->getSessionWithReservation(
      url.getHost(), url.getPort(), url.isSecure(), connectTimeout_);
  auto reqSource = HTTPFixedSource::makeFixedSource(
      std::make_unique<HTTPMessage>(*request_), requestBody_.clone());
  // TODO: there isn't currently a way to install any filters on the redirected
  // requests/responses.
  auto respSource =
      co_await res.session->sendRequest(reqSource, std::move(res.reservation));
  if (readTimeout_) {
    respSource.setReadTimeout(*readTimeout_);
  }
  respFilter_.setSource(respSource.release());
}

} // namespace proxygen::coro
