/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/coro/HTTPFixedSource.h>
#include <proxygen/lib/http/coro/HTTPSourceReader.h>
#include <proxygen/lib/http/coro/HTTPStreamSource.h>
#include <proxygen/lib/http/coro/filters/MutateFilter.h>
#include <proxygen/lib/http/coro/util/test/TestHelpers.h>

using namespace testing;

namespace proxygen::coro::test {

class MutateFilterTest : public testing::Test {
 public:
  folly::DrivableExecutor* getExecutor() {
    return &evb_;
  }

 protected:
  folly::EventBase evb_;
};

CO_TEST_F_X(MutateFilterTest, SimpleTest) {
  auto msg = makeResponse(200);
  // create response source
  HTTPStreamSource respSource(&evb_);
  respSource.headers(std::move(msg), /*eom=*/false);
  respSource.body(
      folly::IOBuf::copyBuffer("hello"), /*padding=*/0, /*eom=*/false);

  // create visitor
  MutateFilter::HeaderMutateFn headerHook = [](HTTPHeaderEvent& headerEvent) {
    // add two random header fields
    auto& headers = headerEvent.headers->getHeaders();
    headers.add("x-header-a", "x-value-a");
    headers.add("x-header-b", "x-value-b");
  };
  MutateFilter::BodyMutateFn bodyHook = [](HTTPBodyEvent& bodyEvent) {
    // replace "hello" in body with "world"
    EXPECT_EQ(bodyEvent.eventType, HTTPBodyEvent::BODY);
    CHECK(!bodyEvent.event.body.empty());
    auto bodyStr = bodyEvent.event.body.move()->to<std::string>();
    EXPECT_EQ(bodyStr, "hello");
    bodyEvent.event.body.append(folly::IOBuf::copyBuffer("world"));
  };

  auto* mutateSource =
      new MutateFilter(&respSource, std::move(headerHook), std::move(bodyHook));
  mutateSource->setHeapAllocated();

  HTTPSourceReader reader(mutateSource);
  reader.onHeaders(
      [](std::unique_ptr<HTTPMessage> msg, bool /*final*/, bool eom) {
        const auto& headers = msg->getHeaders();
        EXPECT_TRUE(headers.exists("x-header-a") &&
                    headers.exists("x-header-b"));
        EXPECT_EQ(headers.getSingleOrEmpty("x-header-a"), "x-value-a");
        EXPECT_EQ(headers.getSingleOrEmpty("x-header-b"), "x-value-b");
        EXPECT_FALSE(eom);
        return true;
      });

  reader.onBody([](BufQueue body, bool /*eom*/) {
    CHECK(!body.empty());
    auto bodyStr = body.move()->to<std::string>();
    EXPECT_EQ(bodyStr, "world");
    return true;
  });

  // read response
  auto res = co_await co_awaitTry(reader.read());
  EXPECT_FALSE(res.hasException());
}

CO_TEST_F_X(MutateFilterTest, PassThruOnError) {
  auto* respSource =
      new HTTPErrorSource(HTTPError(HTTPErrorCode::CANCEL, "cancelled"));

  // create visitor
  MutateFilter::HeaderMutateFn headerHook =
      [](HTTPHeaderEvent& /*headerEvent*/) {
        XLOG(FATAL) << "header hook called";
      };
  MutateFilter::BodyMutateFn bodyHook = [](HTTPBodyEvent& /*bodyEvent*/) {
    XLOG(FATAL) << "body hook called";
  };

  auto* mutateSource =
      new MutateFilter(respSource, std::move(headerHook), std::move(bodyHook));
  mutateSource->setHeapAllocated();

  HTTPSourceReader reader(mutateSource);
  // read response
  auto res = co_await co_awaitTry(reader.read());
  EXPECT_TRUE(res.hasException());
}

} // namespace proxygen::coro::test
