/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/RequestContextFilterFactory.h"
#include "folly/coro/GtestHelpers.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/filters/VisitorFilter.h"

namespace proxygen::coro::test {

CO_TEST(RequestContextFilterFactoryTest, RequestContextInjection) {
  const folly::RequestToken kRequestToken{"proxygen::coro::test::token"};
  using RequestData = folly::ImmutableRequestData<int>;
  const auto setRequestContextData = [&](auto&&) {
    auto* context = folly::RequestContext::try_get();
    EXPECT_NE(context, nullptr);
    context->setContextData(kRequestToken, std::make_unique<RequestData>(42));
  };
  const auto ensureRequestContextData = [&](auto&&) {
    auto* context = folly::RequestContext::try_get();
    EXPECT_NE(context, nullptr);
    auto* data = dynamic_cast<const RequestData*>(
        context->getContextData(kRequestToken));
    EXPECT_TRUE(data && data->value() == 42);
  };

  auto* requestSource = HTTPFixedSource::makeFixedRequest(
      "https://www.facebook.com",
      HTTPMethod::GET,
      folly::IOBuf::wrapBuffer(folly::StringPiece{"foo"}));
  auto* responseSource = HTTPFixedSource::makeFixedResponse(200, "bar");

  auto requestVisitor = std::make_unique<VisitorFilter>(
      requestSource, setRequestContextData, ensureRequestContextData);
  auto responseVisitor = std::make_unique<VisitorFilter>(
      responseSource, ensureRequestContextData, ensureRequestContextData);

  auto [requestFilter, responseFilter] =
      RequestContextFilterFactory{}.makeFilters();
  CO_ASSERT_NE(requestFilter, nullptr);
  CO_ASSERT_NE(responseFilter, nullptr);
  requestFilter->setSource(requestVisitor.get());
  responseFilter->setSource(responseVisitor.get());

  const auto requestHeaderEvent =
      co_await folly::coro::co_awaitTry(requestFilter->readHeaderEvent());
  CO_ASSERT_FALSE(requestHeaderEvent.hasException());
  const auto requestBodyEvent =
      co_await folly::coro::co_awaitTry(requestFilter->readBodyEvent());
  CO_ASSERT_FALSE(requestBodyEvent.hasException());
  const auto responseHeaderEvent =
      co_await folly::coro::co_awaitTry(responseFilter->readHeaderEvent());
  CO_ASSERT_FALSE(responseHeaderEvent.hasException());
  const auto responseBodyEvent =
      co_await folly::coro::co_awaitTry(responseFilter->readBodyEvent());
  CO_ASSERT_FALSE(responseBodyEvent.hasException());
}

} // namespace proxygen::coro::test
