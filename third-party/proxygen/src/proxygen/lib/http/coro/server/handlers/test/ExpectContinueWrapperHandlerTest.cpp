/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/server/handlers/ExpectContinueWrapperHandler.h"

#include <folly/coro/GmockHelpers.h>
#include <folly/coro/GtestHelpers.h>
#include <proxygen/lib/http/coro/HTTPFixedSource.h>
#include <proxygen/lib/http/coro/test/Mocks.h>

using namespace testing;
using namespace proxygen;
using namespace proxygen::coro;

namespace proxygen::coro::test {

class ExpectContinueWrapperHandlerTest : public Test {
 public:
  ExpectContinueWrapperHandlerTest()
      : mockHandler_(std::make_shared<MockHTTPHandler>()),
        expectContinueWrapper_(
            std::make_unique<ExpectContinueWrapperHandler>(mockHandler_)) {
  }

 protected:
  folly::EventBase evb_;
  MockHTTPSessionContext httpSessionContext_;
  std::shared_ptr<MockHTTPHandler> mockHandler_;
  std::unique_ptr<ExpectContinueWrapperHandler> expectContinueWrapper_;
};

CO_TEST_F(ExpectContinueWrapperHandlerTest, SendContinueResponse) {
  InSequence seq;

  // Create a request with Expect: 100-continue header
  auto request = std::make_unique<HTTPMessage>();
  request->setMethod("POST");
  request->setURL("https://test.facebook.com/continue");
  request->getHeaders().add(HTTP_HEADER_EXPECT, "100-continue");
  request->getHeaders().add(HTTP_HEADER_HOST, "test.facebook.com");
  auto requestSource = std::make_unique<HTTPFixedSource>(std::move(request));
  HTTPSourceHolder requestSourceHolder(requestSource.get());

  // The first header event should be handled by the wrapper
  // returning the 100 Continue response without triggering the
  // wrapped mockHandler.
  EXPECT_CALL(*mockHandler_, handleRequest(_, _, _)).Times(0);
  auto responseHolder = co_await expectContinueWrapper_->handleRequest(
      &evb_,
      httpSessionContext_.acquireKeepAlive(),
      std::move(requestSourceHolder));
  auto headerEvent = co_await responseHolder.readHeaderEvent();
  CO_ASSERT_EQ(headerEvent.headers->getStatusCode(), 100);
  CO_ASSERT_FALSE(headerEvent.eom);

  // The second header event should be returned from the mockhandler.
  EXPECT_CALL(*mockHandler_, handleRequest(_, _, _))
      .Times(1)
      .WillRepeatedly(folly::coro::gmock_helpers::CoInvoke(
          [](folly::EventBase *evb,
             HTTPSessionContextPtr ctx,
             HTTPSourceHolder requestSourceHolder)
              -> folly::coro::Task<HTTPSourceHolder> {
            // Make sure mock handler receives original request but with expect
            // header removed
            auto propagatedHeader =
                co_await requestSourceHolder.readHeaderEvent();
            EXPECT_EQ(propagatedHeader.headers->getHeaders().getSingleOrEmpty(
                          HTTP_HEADER_EXPECT),
                      "");
            EXPECT_EQ(propagatedHeader.headers->getHeaders().getSingleOrEmpty(
                          HTTP_HEADER_HOST),
                      "test.facebook.com");
            EXPECT_TRUE(propagatedHeader.eom);

            co_return HTTPFixedSource::makeFixedResponse(200, "MOCK BODY");
          }));
  auto headerEvent2 = co_await responseHolder.readHeaderEvent();
  CO_ASSERT_EQ(headerEvent2.headers->getStatusCode(), 200);
  CO_ASSERT_FALSE(headerEvent2.eom);

  // The response body event should be come from the mockHandler
  auto bodyEvent = co_await responseHolder.readBodyEvent();
  auto bodyStr = bodyEvent.event.body.move()->moveToFbString();
  CO_ASSERT_EQ(bodyStr, "MOCK BODY");
  CO_ASSERT_TRUE(bodyEvent.eom);
}

CO_TEST_F(ExpectContinueWrapperHandlerTest, NoExpectation) {
  InSequence seq;

  // Create a request without an expectation
  auto requestSource = HTTPFixedSource::makeFixedRequest(
      "https://test.facebook.com", HTTPMethod::POST);
  HTTPSourceHolder requestSourceHolder(requestSource);

  // The header event should be returned from the mockhandler directly.
  EXPECT_CALL(*mockHandler_, handleRequest(_, _, _))
      .Times(1)
      .WillRepeatedly(folly::coro::gmock_helpers::CoInvoke(
          [](folly::EventBase *evb,
             HTTPSessionContextPtr ctx,
             HTTPSourceHolder source) -> folly::coro::Task<HTTPSourceHolder> {
            co_return HTTPFixedSource::makeFixedResponse(200, "MOCK BODY");
          }));
  auto responseHolder = co_await expectContinueWrapper_->handleRequest(
      &evb_,
      httpSessionContext_.acquireKeepAlive(),
      std::move(requestSourceHolder));
  auto headerEvent = co_await responseHolder.readHeaderEvent();
  CO_ASSERT_EQ(headerEvent.headers->getStatusCode(), 200);
  CO_ASSERT_FALSE(headerEvent.eom);

  // The body event should be returned from the mockhandler directly.
  auto bodyEvent = co_await responseHolder.readBodyEvent();
  auto bodyStr = bodyEvent.event.body.move()->moveToFbString();
  CO_ASSERT_EQ(bodyStr, "MOCK BODY");
  CO_ASSERT_TRUE(bodyEvent.eom);
}

CO_TEST_F(ExpectContinueWrapperHandlerTest, InvalidExpectation) {
  InSequence seq;

  // Create a request with an invalid expectation
  auto request = std::make_unique<HTTPMessage>();
  request->setMethod("POST");
  request->setURL("https://test.facebook.com/continue");
  request->getHeaders().add(HTTP_HEADER_EXPECT, "blah");
  auto requestSource = std::make_unique<HTTPFixedSource>(std::move(request));
  HTTPSourceHolder requestSourceHolder(requestSource.get());

  // The mockHandler should never be called.
  EXPECT_CALL(*mockHandler_, handleRequest(_, _, _)).Times(0);

  // The wrapper should respond directly with 417.
  auto responseHolder = co_await expectContinueWrapper_->handleRequest(
      &evb_,
      httpSessionContext_.acquireKeepAlive(),
      std::move(requestSourceHolder));
  auto headerEvent = co_await responseHolder.readHeaderEvent();
  CO_ASSERT_EQ(headerEvent.headers->getStatusCode(), 417);
  CO_ASSERT_EQ(headerEvent.headers->getStatusMessage(), "Expectation Failed");
  CO_ASSERT_FALSE(headerEvent.eom);

  // The body event should be returned from the mockhandler directly.
  auto bodyEvent = co_await responseHolder.readBodyEvent();
  auto bodyStr = bodyEvent.event.body.move()->moveToFbString();
  CO_ASSERT_EQ(bodyStr, "Expectation Failed");
  CO_ASSERT_TRUE(bodyEvent.eom);
}

CO_TEST_F(ExpectContinueWrapperHandlerTest,
          StopReadingBeforeInvokeRequestWithoutBody) {
  InSequence seq;

  // Create a request with Expect: 100-continue header
  auto request = std::make_unique<HTTPMessage>();
  request->setMethod("POST");
  request->setURL("https://test.facebook.com/continue");
  request->getHeaders().add(HTTP_HEADER_EXPECT, "100-continue");
  auto requestSource = std::make_unique<HTTPFixedSource>(std::move(request));
  HTTPSourceHolder requestSourceHolder(requestSource.get());

  // The first header event should be handled by the wrapper
  // returning the 100 Continue response without triggering the
  // wrapped mockHandler.
  EXPECT_CALL(*mockHandler_, handleRequest(_, _, _)).Times(0);
  auto responseHolder = co_await expectContinueWrapper_->handleRequest(
      &evb_,
      httpSessionContext_.acquireKeepAlive(),
      std::move(requestSourceHolder));
  auto headerEvent = co_await responseHolder.readHeaderEvent();
  CO_ASSERT_EQ(headerEvent.headers->getStatusCode(), 100);
  CO_ASSERT_FALSE(headerEvent.eom);

  // The wrapped mock handler has not been invoked yet.
  // Stop reading and make sure this is handled gracefully.
  EXPECT_NO_THROW(responseHolder.stopReading());
}

CO_TEST_F(ExpectContinueWrapperHandlerTest,
          StopReadingBeforeInvokeRequestWithBody) {
  InSequence seq;

  // Create a request with Expect: 100-continue header and a body
  auto request = std::make_unique<HTTPMessage>();
  request->setMethod("POST");
  request->setURL("https://test.facebook.com/continue");
  request->getHeaders().add(HTTP_HEADER_EXPECT, "100-continue");
  auto body = folly::IOBuf::copyBuffer("REQUEST BODY");
  auto requestSource =
      std::make_unique<HTTPFixedSource>(std::move(request), std::move(body));
  HTTPSourceHolder requestSourceHolder(requestSource.get());

  // The first header event should be handled by the wrapper
  // returning the 100 Continue response without triggering the
  // wrapped mockHandler.
  EXPECT_CALL(*mockHandler_, handleRequest(_, _, _)).Times(0);
  auto responseHolder = co_await expectContinueWrapper_->handleRequest(
      &evb_,
      httpSessionContext_.acquireKeepAlive(),
      std::move(requestSourceHolder));
  auto headerEvent = co_await responseHolder.readHeaderEvent();
  CO_ASSERT_EQ(headerEvent.headers->getStatusCode(), 100);
  CO_ASSERT_FALSE(headerEvent.eom);

  // The wrapped mock handler has not been invoked yet.
  // Stop reading and make sure this is handled gracefully.
  EXPECT_NO_THROW(responseHolder.stopReading());
}

CO_TEST_F(ExpectContinueWrapperHandlerTest, StopReadingAfterInvoke) {
  InSequence seq;

  // Create a request with Expect: 100-continue header
  auto request = std::make_unique<HTTPMessage>();
  request->setMethod("POST");
  request->setURL("https://test.facebook.com/continue");
  request->getHeaders().add(HTTP_HEADER_EXPECT, "100-continue");
  auto requestSource = std::make_unique<HTTPFixedSource>(std::move(request));
  HTTPSourceHolder requestSourceHolder(requestSource.get());

  // The first header event should be handled by the wrapper
  // returning the 100 Continue response without triggering the
  // wrapped mockHandler.
  EXPECT_CALL(*mockHandler_, handleRequest(_, _, _)).Times(0);
  auto responseHolder = co_await expectContinueWrapper_->handleRequest(
      &evb_,
      httpSessionContext_.acquireKeepAlive(),
      std::move(requestSourceHolder));
  auto headerEvent = co_await responseHolder.readHeaderEvent();
  CO_ASSERT_EQ(headerEvent.headers->getStatusCode(), 100);
  CO_ASSERT_FALSE(headerEvent.eom);

  // The second header event should be returned from the mockhandler.
  EXPECT_CALL(*mockHandler_, handleRequest(_, _, _))
      .Times(1)
      .WillRepeatedly(folly::coro::gmock_helpers::CoInvoke(
          [](folly::EventBase *evb,
             HTTPSessionContextPtr ctx,
             HTTPSourceHolder source) -> folly::coro::Task<HTTPSourceHolder> {
            co_return HTTPFixedSource::makeFixedResponse(200, "MOCK BODY");
          }));
  auto headerEvent2 = co_await responseHolder.readHeaderEvent();
  CO_ASSERT_EQ(headerEvent2.headers->getStatusCode(), 200);
  CO_ASSERT_FALSE(headerEvent2.eom);

  // The wrapped mock handler has been invoked but message is not over yet.
  // Stop reading and make sure this is handled gracefully.
  EXPECT_NO_THROW(responseHolder.stopReading());
}
} // namespace proxygen::coro::test
