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
#include <proxygen/lib/http/coro/filters/VisitorFilter.h>
#include <proxygen/lib/http/coro/util/test/TestHelpers.h>

using namespace testing;

namespace proxygen::coro::test {

class VisitorFilterTest : public testing::Test {
 public:
  folly::DrivableExecutor* getExecutor() {
    return &evb_;
  }

 protected:
  folly::EventBase evb_;
};

CO_TEST_F_X(VisitorFilterTest, SimpleTest) {
  auto msg = makeResponse(200);
  msg->getHeaders().set("x-header-a", "x-value-a");
  msg->getHeaders().set("x-header-b", "x-value-b");

  // create response source
  auto* respSource = new HTTPStreamSource(&evb_);
  respSource->setHeapAllocated();
  respSource->headers(std::move(msg), /*eom=*/false);
  respSource->body(
      folly::IOBuf::copyBuffer("hello"), /*padding=*/0, /*eom=*/false);

  // create visitor
  size_t numHeaderCallbacks{0}, numBodyCallbacks{0};
  VisitorFilter::HeaderHookFn headerHook =
      [&numHeaderCallbacks](const folly::Try<HTTPHeaderEvent>& headerEvent) {
        CHECK(!headerEvent.hasException());
        // validate message is the same
        numHeaderCallbacks++;
        const auto& headers = headerEvent->headers->getHeaders();
        EXPECT_EQ(headers.getSingleOrEmpty("x-header-a"), "x-value-a");
        EXPECT_EQ(headers.getSingleOrEmpty("x-header-b"), "x-value-b");
      };
  VisitorFilter::BodyHookFn bodyHook =
      [&numBodyCallbacks](const folly::Try<HTTPBodyEvent>& bodyEvent) {
        CHECK(!bodyEvent.hasException());
        if (bodyEvent->eventType == HTTPBodyEvent::SUSPEND) {
          return;
        }
        // validate we receive two sepearate body events
        EXPECT_EQ(bodyEvent->eventType, HTTPBodyEvent::BODY);
        auto bodyStr = bodyEvent->event.body.clone()->to<std::string>();
        EXPECT_EQ(bodyStr, numBodyCallbacks == 0 ? "hello" : "world!");
        numBodyCallbacks++;
      };

  auto* visitorSource =
      new VisitorFilter(respSource, std::move(headerHook), std::move(bodyHook));
  visitorSource->setHeapAllocated();

  // queue the second body event asynchronously because why not.
  evb_.runAfterDelay(
      [=]() {
        respSource->body(
            folly::IOBuf::copyBuffer("world!"), /*padding=*/0, /*eom=*/true);
      },
      50);

  HTTPSourceReader reader(visitorSource);
  co_await reader.read();
  EXPECT_EQ(numHeaderCallbacks, 1);
  EXPECT_EQ(numBodyCallbacks, 2);
}

CO_TEST_F_X(VisitorFilterTest, VisitorInvokedOnError) {
  auto msg = makeResponse(200);

  // create error source
  auto* respSource = new HTTPErrorSource(
      HTTPError(HTTPErrorCode::CANCEL, "cancelled"), /*heapAllocated=*/true);

  // create visitor that verifies headerEvent yields error and body hook is
  // never invoked
  bool expectHeaderHook{false};
  VisitorFilter::HeaderHookFn headerHook =
      [&expectHeaderHook](const folly::Try<HTTPHeaderEvent>& headerEvent) {
        expectHeaderHook = true;
        CHECK(headerEvent.hasException());
      };
  VisitorFilter::BodyHookFn bodyHook =
      [](const folly::Try<HTTPBodyEvent>& /*bodyEvent*/) {
        XLOG(FATAL) << "body hook called";
      };

  // create visitor source
  auto* visitorSource =
      new VisitorFilter(respSource, std::move(headerHook), std::move(bodyHook));
  visitorSource->setHeapAllocated();

  HTTPSourceReader reader(visitorSource);
  auto res = co_await co_awaitTry(reader.read());
  EXPECT_TRUE(res.hasException());
  EXPECT_TRUE(expectHeaderHook);
}

CO_TEST_F_X(VisitorFilterTest, PassThruObserver) {
  auto msg = makeResponse(200);

  // create response source
  auto* respSource = new HTTPStreamSource(&evb_);
  respSource->setHeapAllocated();
  respSource->headers(std::move(msg), /*eom=*/false);
  respSource->body(
      folly::IOBuf::copyBuffer("hello"), /*padding=*/0, /*eom=*/true);

  // create dummy passthru observer filter, verify everything works as expected.
  auto* visitorSource = new VisitorFilter(respSource, nullptr, nullptr);
  visitorSource->setHeapAllocated();

  // sanity check headers and body
  HTTPSourceReader reader(visitorSource);
  reader.onHeaders([](std::unique_ptr<HTTPMessage> msg, bool final, bool eom) {
    CHECK(final && !eom);
    EXPECT_EQ(msg->getStatusCode(), 200);
    return true;
  });
  reader.onBody([](BufQueue body, bool eom) {
    CHECK(!body.empty() && eom);
    EXPECT_EQ(body.move()->to<std::string>(), "hello");
    return true;
  });
  auto res = co_await co_awaitTry(reader.read());
  EXPECT_FALSE(res.hasException());
}

} // namespace proxygen::coro::test
