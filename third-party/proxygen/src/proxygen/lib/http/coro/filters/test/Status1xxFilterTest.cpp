/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/Status1xxFilter.h"

#include <folly/coro/GmockHelpers.h>
#include <folly/coro/GtestHelpers.h>
#include <proxygen/lib/http/coro/test/Mocks.h>

using namespace testing;
using namespace proxygen;
using namespace proxygen::coro;

namespace proxygen::coro::test {

CO_TEST(Status1xxFilterTest, Ignore1xxResponses) {
  InSequence seq;

  MockHTTPSource mockSource;

  EXPECT_CALL(mockSource, readHeaderEvent())
      .Times(3)
      .WillRepeatedly(folly::coro::gmock_helpers::CoInvoke(
          []() -> folly::coro::Task<HTTPHeaderEvent> {
            auto headers1xx = std::make_unique<HTTPMessage>();
            headers1xx->setStatusCode(100);
            headers1xx->getHeaders().add("testName", "testValue");
            co_return HTTPHeaderEvent(std::move(headers1xx), false);
          }));
  EXPECT_CALL(mockSource, readHeaderEvent())
      .Times(1)
      .WillOnce([]() -> folly::coro::Task<HTTPHeaderEvent> {
        auto headers200 = std::make_unique<HTTPMessage>();
        headers200->setStatusCode(200);
        headers200->getHeaders().add("co", "ro");
        co_return HTTPHeaderEvent(std::move(headers200), true);
      });

  // Consume first response directly from source
  auto headerEvent = co_await mockSource.readHeaderEvent();
  EXPECT_TRUE(headerEvent.headers->is1xxResponse());

  // Consume remaining responses through filter
  Status1xxFilter filter1xx;
  filter1xx.setSource(&mockSource);

  // 1xx responses should be skipped.
  headerEvent = co_await filter1xx.readHeaderEvent();
  EXPECT_FALSE(headerEvent.headers->is1xxResponse());
  EXPECT_EQ(headerEvent.headers->getStatusCode(), 200);
  EXPECT_TRUE(headerEvent.headers->getHeaders().exists("co"));
}
} // namespace proxygen::coro::test
