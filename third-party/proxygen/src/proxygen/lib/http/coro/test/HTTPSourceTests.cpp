/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "folly/coro/GmockHelpers.h"
#include "proxygen/lib/http/coro/HTTPBodyEventQueue.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/HTTPHybridSource.h"
#include "proxygen/lib/http/coro/HTTPSourceFilter.h"
#include "proxygen/lib/http/coro/HTTPSourceReader.h"
#include "proxygen/lib/http/coro/HTTPStreamSource.h"
#include "proxygen/lib/http/coro/HTTPStreamSourceHolder.h"
#include "proxygen/lib/http/coro/test/Mocks.h"
#include "proxygen/lib/http/coro/util/ExecutorSourceFilter.h"
#include "proxygen/lib/http/coro/util/test/TestHelpers.h"
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Sleep.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/logging/xlog.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <memory>
#include <proxygen/lib/http/HTTP3ErrorCode.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>

using namespace proxygen::coro;
using namespace testing;
using namespace proxygen;

namespace proxygen::coro::test {

class MockStreamSourceCallback : public HTTPStreamSource::Callback {
 public:
  MockStreamSourceCallback() {
  }

  MOCK_METHOD(void, bytesProcessed, (HTTPCodec::StreamID, size_t, size_t));
  MOCK_METHOD(void, windowOpen, (HTTPCodec::StreamID));
  MOCK_METHOD(void,
              sourceComplete,
              (HTTPCodec::StreamID, folly::Optional<HTTPError>));
};

class MockHTTPBodyEventQueueCallback : public HTTPBodyEventQueue::Callback {
 public:
  MockHTTPBodyEventQueueCallback() {
  }

  MOCK_METHOD(void, onEgressBytesBuffered, (int64_t), (noexcept));
};

class HTTPStreamSourceTest : public testing::Test {
 public:
  void SetUp() override {
    EXPECT_CALL(callback_, sourceComplete(_, _))
        .WillOnce(InvokeWithoutArgs([this] {
          if (terminateLoopOnComplete_) {
            evb_.terminateLoopSoon();
          }
        }));
  }

 protected:
  folly::coro::Task<void> drainSource(
      HTTPStreamSource* stream = nullptr,
      uint32_t maxRead = std::numeric_limits<uint32_t>::max(),
      bool* stopReading = nullptr) {
    if (!stream) {
      stream = &stream_;
    }
    bool final = false;
    bool eom = false;
    do {
      if (stopReading && *stopReading) {
        stream->stopReading(folly::none);
        co_return;
      }
      auto headerEvent = co_await co_awaitTry(stream->readHeaderEvent());
      if (headerEvent.hasException()) {
        error_ = getHTTPError(headerEvent);
        co_return;
      }
      final = headerEvent->isFinal();
      eom = headerEvent->eom;
      headerEvents_.emplace_back(std::move(*headerEvent));
      // eom must be final
      EXPECT_TRUE(final || !eom);
    } while (!final && !eom);

    while (!eom) {
      if (stopReading && *stopReading) {
        stream->stopReading(folly::none);
        co_return;
      }
      auto bodyEvent = co_await co_awaitTry(stream->readBodyEvent(maxRead));
      if (bodyEvent.hasException()) {
        error_ = getHTTPError(bodyEvent);
        co_return;
      }
      eom = bodyEvent->eom;
      if (bodyEvent->eventType != HTTPBodyEvent::EventType::SUSPEND) {
        bodyEvents_.emplace_back(std::move(*bodyEvent));
      }
    }
  }

  void run() {
    evb_.loop();
  }

  void reset() {
    headerEvents_.clear();
    bodyEvents_.clear();
    error_.reset();
  }

  folly::EventBase evb_;
  NiceMock<MockStreamSourceCallback> callback_;
  HTTPStreamSource stream_{
      &evb_, 0, callback_, 65535, std::chrono::milliseconds(250)};
  folly::Optional<HTTPError> error_;
  std::vector<HTTPHeaderEvent> headerEvents_;
  std::vector<HTTPBodyEvent> bodyEvents_;
  bool terminateLoopOnComplete_{true};
};

CO_TEST(FixedSourceTest, BasicCopyTest) {
  InSequence seq;

  auto request = std::make_unique<HTTPMessage>();
  request->setMethod("GET");
  request->setURL("https://test.facebook.com/");

  auto requestSource = std::make_unique<HTTPFixedSource>(std::move(request));
  auto requestCopy = std::make_unique<HTTPFixedSource>(*requestSource);

  auto headers = co_await requestSource->readHeaderEvent();
  auto headersCopy = co_await requestCopy->readHeaderEvent();

  EXPECT_NE(headers.headers, headersCopy.headers);
  EXPECT_EQ(headers.headers->getMethodString(),
            headersCopy.headers->getMethodString());
  EXPECT_EQ(headers.headers->getURL(), headersCopy.headers->getURL());
  EXPECT_EQ(headers.eom, headersCopy.eom);
}

CO_TEST(FixedSourceTest, BasicMoveTest) {
  InSequence seq;

  auto request = std::make_unique<HTTPMessage>();
  request->setMethod("GET");
  request->setURL("https://test.facebook.com/");

  auto requestSource = HTTPFixedSource(std::move(request));
  auto requestMove =
      std::make_unique<HTTPFixedSource>(std::move(requestSource));

  auto headers = co_await requestMove->readHeaderEvent();

  EXPECT_FALSE(requestSource.msg_);
  EXPECT_TRUE(requestSource.body_.empty());

  EXPECT_EQ("GET", headers.headers->getMethodString());
  EXPECT_EQ("https://test.facebook.com/", headers.headers->getURL());
}

TEST_F(HTTPStreamSourceTest, Basic) {
  auto req = std::make_unique<HTTPMessage>();
  req->setURL("/");
  stream_.headers(std::move(req), true);

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_FALSE(error_.hasValue());
  EXPECT_EQ(headerEvents_.size(), 1);
  EXPECT_EQ(bodyEvents_.size(), 0);
  EXPECT_EQ(headerEvents_[0].headers->getPathAsStringPiece(), "/");
  EXPECT_EQ(headerEvents_[0].isFinal(), true);
  EXPECT_EQ(headerEvents_[0].eom, true);
}

TEST_F(HTTPStreamSourceTest, Body) {
  auto resp = std::make_unique<HTTPMessage>();
  resp->setStatusCode(200);
  stream_.headers(std::move(resp));

  stream_.body(BufQueue(folly::IOBuf::copyBuffer("body")), 0);

  auto promiseSource = HTTPFixedSource::makeFixedResponse(
      200, folly::IOBuf::copyBuffer("push body"));

  auto promise = std::make_unique<HTTPMessage>();
  promise->setURL("/push");
  stream_.pushPromise(std::move(promise), promiseSource);
  stream_.eom();

  co_withExecutor(&evb_, drainSource(&stream_, 1)).start();
  evb_.loopOnce();

  EXPECT_FALSE(error_.hasValue());
  EXPECT_EQ(headerEvents_.size(), 1);
  EXPECT_EQ(bodyEvents_.size(), 5);
  EXPECT_EQ(headerEvents_[0].headers->getStatusCode(), 200);
  EXPECT_EQ(headerEvents_[0].isFinal(), true);
  EXPECT_EQ(headerEvents_[0].eom, false);
  EXPECT_EQ(bodyEvents_[0].eventType, HTTPBodyEvent::BODY);
  for (auto i = 1; i < 4; i++) {
    EXPECT_EQ(bodyEvents_[i].eventType, HTTPBodyEvent::BODY);
    EXPECT_EQ(bodyEvents_[i].eom, false);
    bodyEvents_[0].event.body.append(bodyEvents_[i].event.body.move());
  }
  auto buf = bodyEvents_[0].event.body.move();
  buf->coalesce();
  EXPECT_EQ(buf->moveToFbString(), std::string("body"));
  EXPECT_EQ(bodyEvents_[0].eom, false);
  EXPECT_EQ(bodyEvents_[4].eventType, HTTPBodyEvent::PUSH_PROMISE);
  EXPECT_EQ(bodyEvents_[4].event.push.promise->getPathAsStringPiece(), "/push");
  EXPECT_EQ(bodyEvents_[4].eom, true);
}

TEST_F(HTTPStreamSourceTest, ContentLength0) {
  auto req = makePostRequest(0);
  stream_.headers(std::move(req), false);
  stream_.eom();

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_FALSE(error_.hasValue());
  EXPECT_EQ(headerEvents_.size(), 1);
  EXPECT_EQ(bodyEvents_.size(), 0);
}

TEST_F(HTTPStreamSourceTest, ContentLengthShorterBody) {
  auto req = makePostRequest(2);
  stream_.headers(std::move(req), false);
  stream_.body(BufQueue(folly::IOBuf::copyBuffer("body")), 0);
  stream_.eom();

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_TRUE(error_.hasValue());
  EXPECT_EQ(error_->code, HTTPErrorCode::CONTENT_LENGTH_MISMATCH);
  EXPECT_EQ(headerEvents_.size(), 0);
  EXPECT_EQ(bodyEvents_.size(), 0);
}

TEST_F(HTTPStreamSourceTest, ContentLengthLongerBody) {
  auto req = makePostRequest(10);
  stream_.headers(std::move(req), false);
  stream_.body(BufQueue(folly::IOBuf::copyBuffer("body")), 0);
  stream_.eom();

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_TRUE(error_.hasValue());
  EXPECT_EQ(error_->code, HTTPErrorCode::CONTENT_LENGTH_MISMATCH);
  EXPECT_EQ(headerEvents_.size(), 0);
  EXPECT_EQ(bodyEvents_.size(), 0);
}

TEST_F(HTTPStreamSourceTest, ContentLengthMalformed) {
  auto req = std::make_unique<HTTPMessage>();
  req->getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "xxx");
  req->setURL("/");
  stream_.headers(std::move(req), false);
  stream_.body(BufQueue(folly::IOBuf::copyBuffer("body")), 0);
  stream_.eom();

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_FALSE(error_.hasValue());
  EXPECT_EQ(headerEvents_.size(), 1);
  EXPECT_EQ(bodyEvents_.size(), 1);
  EXPECT_EQ(headerEvents_[0].headers->getPathAsStringPiece(), "/");
  EXPECT_EQ(headerEvents_[0].isFinal(), true);
  EXPECT_EQ(headerEvents_[0].eom, false);
  EXPECT_EQ(bodyEvents_[0].eom, true);

  auto buf = bodyEvents_[0].event.body.move();
  buf->coalesce();
  EXPECT_EQ(buf->moveToFbString(), std::string("body"));
}

TEST_F(HTTPStreamSourceTest, ContentLengthNotOkForEmptyGetResponse) {
  auto resp = makeResponse(200, 10);
  stream_.headers(std::move(std::get<0>(resp)), true);
  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_TRUE(error_.hasValue());
  EXPECT_EQ(error_->code, HTTPErrorCode::CONTENT_LENGTH_MISMATCH);
  EXPECT_EQ(headerEvents_.size(), 0);
  EXPECT_EQ(bodyEvents_.size(), 0);
}

TEST_F(HTTPStreamSourceTest, ContentLengthOkForEmptyHeadResponse) {
  auto resp = makeResponse(200, 10);
  stream_.skipContentLengthValidation();
  stream_.headers(std::move(std::get<0>(resp)), true);
  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_FALSE(error_.hasValue());
  EXPECT_EQ(headerEvents_.size(), 1);
  EXPECT_EQ(bodyEvents_.size(), 0);
  EXPECT_EQ(headerEvents_[0].headers->getStatusCode(), 200);
  EXPECT_EQ(headerEvents_[0].isFinal(), true);
  EXPECT_EQ(headerEvents_[0].eom, true);
}

TEST_F(HTTPStreamSourceTest, ContentLengthIsNotIgnoredInChunkedBody) {
  // Confirm the intent of an RFC violation in this test:
  // According to https://datatracker.ietf.org/doc/html/rfc2616#section-4.4,
  //    Messages must not include both a Content-Length header field
  //      and a non-identity transfer-coding.
  //    If the message does include a non-identity transfer-coding,
  //      the Content-Length must be ignored.
  // We don't validate either of the two parts, opting for comparing declared
  //  Content-Length against what's actually received as in the non-chunked
  //  case.
  auto req = makePostRequest(40);
  req->setIsChunked(true);
  req->getHeaders().add(HTTP_HEADER_TRANSFER_ENCODING, "chunked");
  stream_.headers(std::move(req), false);
  stream_.body(BufQueue(folly::IOBuf::copyBuffer("body")), 0);
  stream_.eom();

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_TRUE(error_.hasValue());
  EXPECT_EQ(error_->code, HTTPErrorCode::CONTENT_LENGTH_MISMATCH);
  EXPECT_EQ(headerEvents_.size(), 0);
  EXPECT_EQ(bodyEvents_.size(), 0);
}

TEST_F(HTTPStreamSourceTest, ContentLengthOkFor304ResponsesWithEmptyBody) {
  auto resp = makeResponse(304, 10);
  stream_.headers(std::move(std::get<0>(resp)), true);

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_FALSE(error_.hasValue());
  EXPECT_EQ(headerEvents_.size(), 1);
  EXPECT_EQ(bodyEvents_.size(), 0);
  EXPECT_EQ(headerEvents_[0].headers->getStatusCode(), 304);
  EXPECT_EQ(headerEvents_[0].isFinal(), true);
  EXPECT_EQ(headerEvents_[0].eom, true);
}

TEST_F(HTTPStreamSourceTest, FinalHeaders) {
  EXPECT_TRUE(stream_.headersAllowed());
  stream_.headers(makeResponse(100), false);
  EXPECT_TRUE(stream_.headersAllowed());
  stream_.headers(makeResponse(200), false);
  EXPECT_FALSE(stream_.headersAllowed());
  stream_.headers(makeResponse(100), false);

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_TRUE(error_.hasValue());
  EXPECT_EQ(error_->code, HTTPErrorCode::INVALID_STATE_TRANSITION);
}

TEST_F(HTTPStreamSourceTest, BodyBeforeFinalHeaders) {
  EXPECT_TRUE(stream_.headersAllowed());
  stream_.headers(makeResponse(100), false);
  EXPECT_TRUE(stream_.headersAllowed());
  stream_.body(BufQueue(folly::IOBuf::copyBuffer("body")), 0);

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_TRUE(error_.hasValue());
  EXPECT_EQ(error_->code, HTTPErrorCode::INVALID_STATE_TRANSITION);
}

TEST_F(HTTPStreamSourceTest, DatagramPriorToHeader) {
  // rx/tx-ing datagrams before headers is a SM violation
  auto resp = makeResponse(200, 10);
  stream_.datagram(makeBuf(100));
  stream_.headers(std::move(std::get<0>(resp)), true);

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_TRUE(error_.hasValue());
  EXPECT_EQ(headerEvents_.size(), 0);
  EXPECT_EQ(bodyEvents_.size(), 0);
}

TEST_F(HTTPStreamSourceTest, BodyQueueDatagramEOM) {
  // if we rx/tx eom after datagram, we push an empty HTTPBodyEvent::BODY with
  // eom to the end of the bodyQueue
  auto resp = makeResponse(200, 0);
  stream_.headers(std::move(std::get<0>(resp)), false);
  stream_.datagram(makeBuf(10));
  stream_.datagram(makeBuf(10));
  stream_.eom();

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_FALSE(error_.hasValue());
  EXPECT_EQ(headerEvents_.size(), 1);
  // 2 datagrams plus empty HTTPBodyEvent::BODY w/ eom
  EXPECT_EQ(bodyEvents_.size(), 3);
  EXPECT_EQ(headerEvents_[0].headers->getStatusCode(), 200);
  EXPECT_EQ(bodyEvents_[2].eom, true);
}

TEST(HTTPBodyStreamSourceTest, BodyOnlyStreamSource) {
  folly::EventBase evb;
  struct HTTPBodyStreamSource : public HTTPStreamSource {
    using HTTPStreamSource::HTTPStreamSource;
    using HTTPStreamSource::validateHeadersAndSkip;
  } bodySource{&evb};

  constexpr uint8_t kContentLength = 100;
  auto req = getPostRequest(kContentLength);
  // ingress SM transition to body; consumer expected to invoke only
  // ::readBodyEvent
  bodySource.validateHeadersAndSkip(req, /*eom=*/false);
  bodySource.body(makeBuf(kContentLength), /*padding=*/0, /*eom=*/true);
  auto res = folly::coro::blockingWait(bodySource.readBodyEvent(), &evb);
  EXPECT_EQ(res.eventType, HTTPBodyEvent::BODY);
  EXPECT_EQ(res.event.body.chainLength(), 100);
  EXPECT_TRUE(res.eom);
}

class HTTPStreamSourceTestPerCode
    : public HTTPStreamSourceTest
    , public ::testing::WithParamInterface<uint16_t> {};

TEST_P(HTTPStreamSourceTestPerCode, ContentLengthIgnoredFor1xx) {
  uint16_t responseCode = GetParam();
  // Add a content length where it should be ignored
  auto resp = makeResponse(responseCode, 10);
  stream_.headers(std::move(std::get<0>(resp)), false);
  stream_.headers(std::move(std::get<0>(makeResponse(200, 0))), true);

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_FALSE(error_.hasValue());
  EXPECT_EQ(headerEvents_.size(), 2);
  EXPECT_EQ(bodyEvents_.size(), 0);
}

// Note: 1xx shouldn't have a Content-Length at all, and having one on a
// 101 Switching Protocols is especially bad.  TODO: validate this T138957911
INSTANTIATE_TEST_SUITE_P(HTTPStreamSourceTests,
                         HTTPStreamSourceTestPerCode,
                         ::testing::Values(100, 102, 103));

TEST_F(HTTPStreamSourceTest, BodyBackpressure) {
  auto stream = new HTTPStreamSource(&evb_, folly::none, &callback_, 4000);
  stream->setHeapAllocated();

  auto resp = std::make_unique<HTTPMessage>();
  resp->setStatusCode(200);
  stream->headers(std::move(resp));

  EXPECT_EQ(stream->body(BufQueue(makeBuf(100)), 0),
            HTTPStreamSource::FlowControlState::OPEN);
  EXPECT_EQ(stream->body(BufQueue(makeBuf(3899)), 0),
            HTTPStreamSource::FlowControlState::OPEN);
  EXPECT_EQ(stream->body(BufQueue(makeBuf(1)), 0),
            HTTPStreamSource::FlowControlState::CLOSED);
  EXPECT_EQ(stream->body(BufQueue(makeBuf(10)), 0),
            HTTPStreamSource::FlowControlState::CLOSED);
  EXPECT_CALL(callback_, windowOpen(HTTPCodec::MaxStreamID))
      .WillOnce(InvokeWithoutArgs([stream] { stream->eom(); }));

  co_withExecutor(&evb_, drainSource(stream, 100)).start();
  evb_.loopOnce();

  EXPECT_FALSE(error_.hasValue());
  EXPECT_EQ(headerEvents_.size(), 1);
  EXPECT_EQ(bodyEvents_.size(), 41);
  EXPECT_EQ(headerEvents_[0].headers->getStatusCode(), 200);
  EXPECT_EQ(headerEvents_[0].isFinal(), true);
  EXPECT_EQ(headerEvents_[0].eom, false);
}

TEST_F(HTTPStreamSourceTest, KMinThreshold) {
  /** Verifies that when kMinThreshold amount of bytes are read, we expect to
   * also process kMinThreshold bytes in bytesProcessed() because of the
   * absolute min threshold in WindowContainer.processed().
   */
  constexpr uint32_t kCapacity = 1024 * 1024;
  constexpr uint32_t kMinThreshold = 128 * 1024;
  auto stream = new HTTPStreamSource(&evb_, folly::none, &callback_, kCapacity);
  stream->setHeapAllocated();

  auto resp = std::make_unique<HTTPMessage>();
  resp->setStatusCode(200);
  stream->headers(std::move(resp));

  EXPECT_EQ(stream->body(BufQueue(makeBuf(kMinThreshold)), 0),
            HTTPStreamSource::FlowControlState::OPEN);
  EXPECT_CALL(
      callback_,
      bytesProcessed(HTTPCodec::MaxStreamID, kMinThreshold, kMinThreshold))
      .WillOnce(InvokeWithoutArgs([stream] { stream->eom(); }));

  co_withExecutor(&evb_, drainSource(stream, kMinThreshold)).start();
  evb_.loopOnce();
}

TEST_F(HTTPStreamSourceTest, Trailers) {
  auto req = std::make_unique<HTTPMessage>();
  req->setURL("/");
  stream_.headers(std::move(req));
  auto trailers = std::make_unique<HTTPHeaders>();
  trailers->add("foo", "bar");
  stream_.trailers(std::move(trailers));

  co_withExecutor(&evb_, drainSource()).start();
  run();

  EXPECT_FALSE(error_.hasValue());
  EXPECT_EQ(headerEvents_.size(), 1);
  EXPECT_EQ(bodyEvents_.size(), 1);
  EXPECT_EQ(headerEvents_[0].eom, false);
  EXPECT_EQ(bodyEvents_[0].eventType, HTTPBodyEvent::TRAILERS);
  EXPECT_EQ(bodyEvents_[0].event.trailers->size(), 1);
  EXPECT_EQ(bodyEvents_[0].eom, true);
}

TEST_F(HTTPStreamSourceTest, StateMachineError) {
  NiceMock<MockStreamSourceCallback> callback;
  auto validateError =
      [this](HTTPStreamSource* stream, size_t headerEvents, size_t bodyEvents) {
        co_withExecutor(&evb_, drainSource(stream)).start();
        evb_.loopOnce();

        EXPECT_TRUE(error_.hasValue());
        EXPECT_EQ(error_->code, HTTPErrorCode::INVALID_STATE_TRANSITION);
        EXPECT_EQ(headerEvents_.size(), headerEvents);
        EXPECT_EQ(bodyEvents_.size(), bodyEvents);
      };

  // body error
  auto stream = std::make_unique<HTTPStreamSource>(&evb_, 0, callback);
  stream->body(BufQueue(folly::IOBuf::copyBuffer("body")), 0);
  // Stick in an event to test drain after error
  stream->eom();
  validateError(stream.get(), 0, 0);

  // headers error
  reset();
  stream = std::make_unique<HTTPStreamSource>(&evb_, 0, callback);
  stream->headers(std::make_unique<HTTPMessage>());
  stream->body(BufQueue(folly::IOBuf::copyBuffer("body")), 0);
  stream->headers(std::make_unique<HTTPMessage>());
  validateError(stream.get(), 0, 0);

  // pushPromise error
  reset();
  stream = std::make_unique<HTTPStreamSource>(&evb_, 0, callback);
  stream->headers(std::make_unique<HTTPMessage>(), true);
  stream->pushPromise(std::make_unique<HTTPMessage>(),
                      HTTPFixedSource::makeFixedResponse(
                          200, folly::IOBuf::copyBuffer("push body")),
                      false);
  validateError(stream.get(), 0, 0);

  // trailers error
  reset();
  // Use real cb at the end
  stream = std::make_unique<HTTPStreamSource>(&evb_, 0, callback_);
  stream->trailers(nullptr);
  validateError(stream.get(), 0, 0);
}

TEST_F(HTTPStreamSourceTest, APIErrorConsumerLateHeaders) {
  // Calling readHeaderEvent after readBodyEvent
  auto resp = std::make_unique<HTTPMessage>();
  resp->setStatusCode(200);
  co_withExecutor(
      &evb_,
      [](HTTPStreamSource* stream) -> folly::coro::Task<void> {
        auto headerEvent = co_await co_awaitTry(stream->readHeaderEvent());
        EXPECT_FALSE(headerEvent.hasException());

        auto bodyEvent = co_await co_awaitTry(stream->readBodyEvent());
        EXPECT_FALSE(bodyEvent.hasException());

        headerEvent = co_await co_awaitTry(stream->readHeaderEvent());
        EXPECT_TRUE(headerEvent.hasException());
        EXPECT_EQ(getHTTPError(headerEvent).code,
                  HTTPErrorCode::INVALID_STATE_TRANSITION);
      }(&stream_))
      .start();
  evb_.loopOnce();

  stream_.headers(std::move(resp));
  stream_.body(makeBuf(100), 0);
  evb_.loop();
}

TEST_F(HTTPStreamSourceTest, APIErrorConsumerEarlyBody) {
  // Calling readBodyEvent before readHeaderEvent
  auto resp = std::make_unique<HTTPMessage>();
  resp->setStatusCode(200);
  co_withExecutor(&evb_,
                  [](HTTPStreamSource* stream) -> folly::coro::Task<void> {
                    auto bodyEvent =
                        co_await co_awaitTry(stream->readBodyEvent());
                    EXPECT_TRUE(bodyEvent.hasException());
                    EXPECT_EQ(getHTTPError(bodyEvent).code,
                              HTTPErrorCode::INVALID_STATE_TRANSITION);
                  }(&stream_))
      .start();
  evb_.loopOnce();

  stream_.headers(std::move(resp));
  evb_.loop();
}

TEST_F(HTTPStreamSourceTest, StateMachineErrorHeaderWait) {
  co_withExecutor(&evb_,
                  [](HTTPStreamSource* stream) -> folly::coro::Task<void> {
                    auto bodyEvent =
                        co_await co_awaitTry(stream->readHeaderEvent());
                    EXPECT_TRUE(bodyEvent.hasException());
                    EXPECT_EQ(getHTTPError(bodyEvent).code,
                              HTTPErrorCode::INVALID_STATE_TRANSITION);
                  }(&stream_))
      .start();
  evb_.loopOnce();

  stream_.eom();
  evb_.loop();
}

TEST_F(HTTPStreamSourceTest, StopReadingWhileWaiting) {
  // Heap allocated stream to verify stopReading doesn't delete while there's a
  // coro waiting
  auto stream = new HTTPStreamSource(&evb_, folly::none, &callback_, 4000);
  stream->setHeapAllocated();

  terminateLoopOnComplete_ = false;
  auto resp = std::make_unique<HTTPMessage>();
  resp->setStatusCode(200);
  co_withExecutor(
      &evb_,
      [](HTTPStreamSource* stream) -> folly::coro::Task<void> {
        auto bodyEvent = co_await co_awaitTry(stream->readHeaderEvent());
        EXPECT_TRUE(bodyEvent.hasException());
        EXPECT_EQ(getHTTPError(bodyEvent).code, HTTPErrorCode::NO_ERROR);
      }(stream))
      .start();
  evb_.loopOnce();

  stream->stopReading(folly::none);
  evb_.loop();
}

TEST_F(HTTPStreamSourceTest, BodyError) {
  auto resp = std::make_unique<HTTPMessage>();
  resp->setStatusCode(200);
  stream_.headers(std::move(resp));

  co_withExecutor(&evb_, drainSource()).start();
  evb_.loopOnce();

  EXPECT_EQ(headerEvents_.size(), 1);
  EXPECT_EQ(bodyEvents_.size(), 0);
  EXPECT_EQ(headerEvents_[0].headers->getStatusCode(), 200);
  EXPECT_EQ(headerEvents_[0].isFinal(), true);
  EXPECT_EQ(headerEvents_[0].eom, false);

  stream_.abort(HTTPErrorCode::TRANSPORT_READ_ERROR);
  run();

  EXPECT_TRUE(error_.hasValue());
  EXPECT_EQ(error_->code, HTTPErrorCode::TRANSPORT_READ_ERROR);
  EXPECT_EQ(bodyEvents_.size(), 0);
}

TEST_F(HTTPStreamSourceTest, stopReading) {
  bool stopReading = false;
  co_withExecutor(&evb_, drainSource(&stream_, 100, &stopReading)).start();
  evb_.loopOnce();
  stopReading = true;
  auto resp = std::make_unique<HTTPMessage>();
  resp->setStatusCode(200);
  stream_.headers(std::move(resp));
  run();

  EXPECT_EQ(headerEvents_.size(), 1);
  EXPECT_EQ(bodyEvents_.size(), 0);
}

TEST_F(HTTPStreamSourceTest, cancel) {
  folly::CancellationSource cancellationSource;
  co_withExecutor(&evb_,
                  folly::coro::co_withCancellation(
                      cancellationSource.getToken(), drainSource()))
      .start();
  evb_.loopOnce();
  auto resp = std::make_unique<HTTPMessage>();
  resp->setStatusCode(200);
  stream_.headers(std::move(resp));
  cancellationSource.requestCancellation();
  run();

  EXPECT_TRUE(error_.hasValue());
  EXPECT_EQ(error_->code, HTTPErrorCode::CORO_CANCELLED);
  EXPECT_EQ(headerEvents_.size(), 0);
  EXPECT_EQ(bodyEvents_.size(), 0);
}

class BodyEventQueueTest : public testing::Test {
 public:
  BodyEventQueueTest() : queue_(&evb_, 1, queueCb, 100) {
  }

  void SetUp() override {
    queue_.setSource(&source_);
  }

  folly::DrivableExecutor* getExecutor() {
    return &evb_;
  }

  void testCancelCoro(std::function<folly::coro::Task<void>()> coro) {
    folly::CancellationSource cancellationSource;
    co_withExecutor(
        &evb_,
        folly::coro::co_withCancellation(cancellationSource.getToken(), coro()))
        .start();
    evb_.loopOnce();
    cancellationSource.requestCancellation();
    evb_.loop();
  }

  folly::EventBase evb_;
  StrictMock<MockHTTPSource> source_;
  MockHTTPBodyEventQueueCallback queueCb;
  MockHTTPBodyEventQueue queue_;
};

TEST_F(BodyEventQueueTest, TestCancelReadHeaders) {
  auto coro = [this]() -> folly::coro::Task<void> {
    EXPECT_CALL(source_, readHeaderEvent())
        .WillOnce(folly::coro::gmock_helpers::CoInvoke(
            []() -> folly::coro::Task<HTTPHeaderEvent> {
              co_await folly::coro::sleepReturnEarlyOnCancel(
                  std::chrono::minutes(1));
              co_return HTTPHeaderEvent(
                  std::make_unique<HTTPMessage>(getResponse(200)), false);
            }));
    EXPECT_CALL(source_, stopReading(_));
    auto res = co_await co_awaitTry(queue_.readHeaderEvent());
    EXPECT_TRUE(res.hasException());
    EXPECT_EQ(getHTTPError(res).code, HTTPErrorCode::CORO_CANCELLED);
  };
  testCancelCoro(coro);
}

CO_TEST_F_X(BodyEventQueueTest, TestMatchingContentLength) {
  EXPECT_CALL(source_, readHeaderEvent())
      .WillOnce(Return(folly::coro::makeTask<HTTPHeaderEvent>(HTTPHeaderEvent(
          std::make_unique<HTTPMessage>(getResponse(200, 50)), false))));

  EXPECT_CALL(source_, readBodyEvent(_))
      .WillOnce(Return(folly::coro::makeTask<HTTPBodyEvent>(
          HTTPBodyEvent(makeBuf(25), false))))
      .WillOnce(Return(folly::coro::makeTask<HTTPBodyEvent>(
          HTTPBodyEvent(makeBuf(25), true))));

  EXPECT_CALL(queue_, contentLengthMismatch()).Times(0);

  co_await co_awaitTry(queue_.readHeaderEvent());
  auto res = co_await co_awaitTry(queue_.readBodyEvent());
  EXPECT_FALSE(res.hasException());
  res = co_await co_awaitTry(queue_.readBodyEvent());
  EXPECT_FALSE(res.hasException());
}

CO_TEST_F_X(BodyEventQueueTest, TestMismatchedContentLengthOverExpected) {
  EXPECT_CALL(source_, readHeaderEvent())
      .WillOnce(Return(folly::coro::makeTask<HTTPHeaderEvent>(HTTPHeaderEvent(
          std::make_unique<HTTPMessage>(getResponse(200, 50)), false))));

  EXPECT_CALL(source_, readBodyEvent(_))
      .WillOnce(Return(folly::coro::makeTask<HTTPBodyEvent>(
          HTTPBodyEvent(makeBuf(75), true))));

  EXPECT_CALL(queue_, contentLengthMismatch()).Times(1);

  co_await co_awaitTry(queue_.readHeaderEvent());
  auto res = co_await co_awaitTry(queue_.readBodyEvent());
  EXPECT_FALSE(res.hasException());
}

CO_TEST_F_X(BodyEventQueueTest, TestMismatchedContentLengthUnderExpected) {
  EXPECT_CALL(source_, readHeaderEvent())
      .WillOnce(Return(folly::coro::makeTask<HTTPHeaderEvent>(HTTPHeaderEvent(
          std::make_unique<HTTPMessage>(getResponse(200, 50)), false))));

  EXPECT_CALL(source_, readBodyEvent(_))
      .WillOnce(Return(folly::coro::makeTask<HTTPBodyEvent>(
          HTTPBodyEvent(makeBuf(25), true))));

  EXPECT_CALL(queue_, contentLengthMismatch()).Times(1);

  co_await co_awaitTry(queue_.readHeaderEvent());
  auto res = co_await co_awaitTry(queue_.readBodyEvent());
  EXPECT_FALSE(res.hasException());
}

CO_TEST_F_X(BodyEventQueueTest, TestSkipValidationOn304EmptyBody) {
  EXPECT_CALL(source_, readHeaderEvent())
      .WillOnce(Return(folly::coro::makeTask<HTTPHeaderEvent>(HTTPHeaderEvent(
          std::make_unique<HTTPMessage>(getResponse(304, 10)), true))));

  EXPECT_CALL(queue_, contentLengthMismatch()).Times(0);

  auto res = co_await co_awaitTry(queue_.readHeaderEvent());
  EXPECT_FALSE(res.hasException());
}

CO_TEST_F_X(BodyEventQueueTest, TestInvalidBodyLength) {
  EXPECT_CALL(source_, readHeaderEvent())
      .WillOnce(Return(folly::coro::makeTask<HTTPHeaderEvent>(HTTPHeaderEvent(
          std::make_unique<HTTPMessage>(getResponseWithInvalidBodyLength()),
          false))));

  EXPECT_CALL(source_, readBodyEvent(_))
      .WillOnce(Return(folly::coro::makeTask<HTTPBodyEvent>(
          HTTPBodyEvent(makeBuf(25), true))));

  EXPECT_CALL(queue_, contentLengthMismatch()).Times(0);

  co_await co_awaitTry(queue_.readHeaderEvent());
  auto res = co_await co_awaitTry(queue_.readBodyEvent());
  EXPECT_FALSE(res.hasException());
}

TEST_F(BodyEventQueueTest, TestCancelReadHeadersWithError) {
  auto coro = [this]() -> folly::coro::Task<void> {
    EXPECT_CALL(source_, readHeaderEvent())
        .WillOnce(Return(folly::coro::makeErrorTask<HTTPHeaderEvent>(
            folly::make_exception_wrapper<HTTPError>(
                HTTPErrorCode::CORO_CANCELLED, ""))));

    auto res = co_await co_awaitTry(queue_.readHeaderEvent());
    EXPECT_TRUE(res.hasException());
    EXPECT_EQ(getHTTPError(res).code, HTTPErrorCode::CORO_CANCELLED);
  };
  testCancelCoro(coro);
}

TEST_F(BodyEventQueueTest, TestCancelReadBody) {
  auto coro = [this]() -> folly::coro::Task<void> {
    EXPECT_CALL(source_, readBodyEvent(_))
        .WillOnce(folly::coro::gmock_helpers::CoInvoke(
            [](uint32_t) -> folly::coro::Task<HTTPBodyEvent> {
              co_await folly::coro::sleepReturnEarlyOnCancel(
                  std::chrono::minutes(1));
              co_return HTTPBodyEvent(std::unique_ptr<folly::IOBuf>(nullptr),
                                      false);
            }));
    EXPECT_CALL(source_, stopReading(_));
    auto res = co_await co_awaitTry(queue_.readBodyEvent());
    EXPECT_TRUE(res.hasException());
    EXPECT_EQ(getHTTPError(res).code, HTTPErrorCode::CORO_CANCELLED);
  };
  testCancelCoro(coro);
}

TEST_F(BodyEventQueueTest, TestCancelReadBodyWithError) {
  auto coro = [this]() -> folly::coro::Task<void> {
    EXPECT_CALL(source_, readBodyEvent(_))
        .WillOnce(Return(folly::coro::makeErrorTask<HTTPBodyEvent>(
            folly::make_exception_wrapper<HTTPError>(
                HTTPErrorCode::CORO_CANCELLED, ""))));
    auto res = co_await co_awaitTry(queue_.readBodyEvent());
    EXPECT_TRUE(res.hasException());
    EXPECT_EQ(getHTTPError(res).code, HTTPErrorCode::CORO_CANCELLED);
  };
  testCancelCoro(coro);
}

TEST_F(BodyEventQueueTest, TestCancelReadBodyAwaitingBuffer) {
  auto coro = [this]() -> folly::coro::Task<void> {
    EXPECT_CALL(source_, readBodyEvent(_))
        .WillOnce(Return(folly::coro::makeTask<HTTPBodyEvent>(
            HTTPBodyEvent(makeBuf(100), false))));
    EXPECT_CALL(source_, stopReading(_));
    auto res = co_await co_awaitTry(queue_.readBodyEvent(100));
    EXPECT_FALSE(res.hasException());
    EXPECT_FALSE(res->eom);
    res = co_await co_awaitTry(queue_.readBodyEvent());
    EXPECT_TRUE(res.hasException());
    EXPECT_EQ(getHTTPError(res).code, HTTPErrorCode::CORO_CANCELLED);
  };
  testCancelCoro(coro);
}

TEST(HTTPSourceFilterTest, test) {
  HTTPSourceHolder holder(HTTPFixedSource::makeFixedRequest("/"));
}

TEST(HTTPSourceReader, test) {
  folly::EventBase evb;
  size_t events = 0;
  auto source = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
  // Add a push
  source->pushes_.emplace_back(
      std::make_unique<HTTPMessage>(getGetRequest("/push")),
      HTTPFixedSource::makeFixedResponse(200, makeBuf(50)),
      /*eom=*/false);
  // Add trailers
  source->trailers_ = std::make_unique<HTTPHeaders>();
  source->trailers_->add("x-trailer1", "trailer1");

  HTTPSourceReader reader(source);
  reader
      .preRead([&events]() {
        events++;
        return HTTPSourceReader::Continue;
      })
      .onHeaders([&events](std::unique_ptr<HTTPMessage> headers,
                           bool final,
                           bool eom) {
        events++;
        EXPECT_EQ(headers->getStatusCode(), 200);
        EXPECT_TRUE(final);
        EXPECT_FALSE(eom);
        return HTTPSourceReader::Continue;
      })
      .onPushPromise([&events](std::unique_ptr<HTTPMessage> promise,
                               HTTPSourceHolder,
                               bool eom) {
        events++;
        EXPECT_EQ(promise->getPathAsStringPiece(), "/push");
        EXPECT_FALSE(eom);
        // response gets a stopReading on dtor
        return HTTPSourceReader::Continue;
      })
      .onBody([&events](BufQueue body, bool eom) {
        events++;
        EXPECT_EQ(body.chainLength(), 100);
        EXPECT_FALSE(eom);
        return HTTPSourceReader::Continue;
      })
      .onTrailers([&events](std::unique_ptr<HTTPHeaders> trailers) {
        events++;
        EXPECT_EQ(trailers->size(), 1);
        EXPECT_EQ(trailers->getSingleOrEmpty("x-trailer1"), "trailer1");
      });
  co_withExecutor(&evb,
                  [](HTTPSourceReader& reader) -> folly::coro::Task<void> {
                    co_await reader.read();
                  }(reader))
      .start();
  evb.loop();
  EXPECT_EQ(events, 4 * 2);
}

TEST(HTTPSourceReader, TestMissingTrailersFn) {
  folly::EventBase evb;
  size_t events = 0;
  size_t bodyEvents = 0;
  auto source = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));
  // add trailers
  source->trailers_ = std::make_unique<HTTPHeaders>();
  source->trailers_->add("x-trailer", "fb-header");

  HTTPSourceReader reader(source);
  reader
      .preRead([&events]() {
        events++;
        return HTTPSourceReader::Continue;
      })
      .onHeaders([&events](std::unique_ptr<HTTPMessage> headers,
                           bool final,
                           bool eom) {
        events++;
        EXPECT_EQ(headers->getStatusCode(), 200);
        EXPECT_TRUE(final);
        EXPECT_FALSE(eom);
        return HTTPSourceReader::Continue;
      })
      .onBody([&events, &bodyEvents](BufQueue body, bool eom) {
        events++;
        // first body event should contain body
        if (bodyEvents == 0) {
          EXPECT_EQ(body.chainLength(), 100);
          EXPECT_FALSE(eom);
        } else {
          // second body event should invoked when we rx trailer without having
          // set onTrailer(...)
          EXPECT_EQ(body.chainLength(), 0);
          EXPECT_TRUE(eom);
        }
        bodyEvents++;
        return HTTPSourceReader::Continue;
      });
  co_withExecutor(&evb,
                  [](HTTPSourceReader& reader) -> folly::coro::Task<void> {
                    co_await reader.read();
                  }(reader))
      .start();
  evb.loop();
  // 2 * 3 because onBody(...) gets invoked twice
  EXPECT_EQ(events, 2 * 3);
  EXPECT_EQ(bodyEvents, 2);
}

TEST(HTTPSourceReader, stop) {
  folly::EventBase evb;
  auto source = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));

  HTTPSourceReader reader(source);
  reader
      .onHeaders(
          [](std::unique_ptr<HTTPMessage> headers, bool final, bool eom) {
            EXPECT_EQ(headers->getStatusCode(), 200);
            EXPECT_TRUE(final);
            EXPECT_FALSE(eom);
            return HTTPSourceReader::Cancel; // Stop reading
          })
      .onBody([](BufQueue, bool) {
        XLOG(FATAL) << "onBody called after stop";
        return HTTPSourceReader::Continue;
      });
  co_withExecutor(&evb,
                  [](HTTPSourceReader& reader) -> folly::coro::Task<void> {
                    co_await reader.read();
                  }(reader))
      .start();
  evb.loop();
}

TEST(HTTPSourceReader, cancelHeaders) {
  folly::EventBase evb;
  auto source = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));

  folly::CancellationSource cancellationSource;
  HTTPSourceReader reader(source);
  reader
      .onHeadersAsync([](std::unique_ptr<HTTPMessage> headers,
                         bool final,
                         bool eom) -> folly::coro::Task<bool> {
        co_await folly::coro::sleepReturnEarlyOnCancel(
            std::chrono::seconds(10));
        co_return HTTPSourceReader::Continue;
      })
      .onBody([](BufQueue, bool) {
        XLOG(FATAL) << "onBody called after cancel";
        return HTTPSourceReader::Continue;
      })
      .onError([](const HTTPSourceReader::ErrorContext&, const HTTPError&) {
        XLOG(FATAL) << "onError called after cancel";
        return HTTPSourceReader::Continue;
      });
  co_withExecutor(&evb,
                  [](HTTPSourceReader& reader,
                     folly::CancellationSource& cancellationSource)
                      -> folly::coro::Task<void> {
                    co_await folly::coro::co_withCancellation(
                        cancellationSource.getToken(), reader.read());
                  }(reader, cancellationSource))
      .start();
  evb.loopOnce();
  cancellationSource.requestCancellation();
  evb.loop();
}

TEST(HTTPSourceReader, cancelPreRead) {
  folly::EventBase evb;
  auto source = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));

  folly::CancellationSource cancellationSource;
  HTTPSourceReader reader(source);
  reader
      .preReadAsync([]() -> folly::coro::Task<bool> {
        co_await folly::coro::sleepReturnEarlyOnCancel(
            std::chrono::seconds(10));
        co_return HTTPSourceReader::Continue;
      })
      .onHeaders(
          [](std::unique_ptr<HTTPMessage> headers, bool final, bool eom) {
            XLOG(FATAL) << "onHeaders called after cancel";
            return HTTPSourceReader::Continue;
          })
      .onError([](const HTTPSourceReader::ErrorContext&, const HTTPError&) {
        XLOG(FATAL) << "onError called after cancel";
        return HTTPSourceReader::Continue;
      });
  co_withExecutor(&evb,
                  [](HTTPSourceReader& reader,
                     folly::CancellationSource& cancellationSource)
                      -> folly::coro::Task<void> {
                    co_await folly::coro::co_withCancellation(
                        cancellationSource.getToken(), reader.read());
                  }(reader, cancellationSource))
      .start();
  evb.loopOnce();
  cancellationSource.requestCancellation();
  evb.loop();
}

TEST(HTTPSourceReader, cancelYieldException) {
  folly::EventBase evb;
  auto source = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));

  folly::CancellationSource cancellationSource;
  HTTPSourceReader reader(source);
  reader
      .onHeadersAsync([](std::unique_ptr<HTTPMessage> headers,
                         bool final,
                         bool eom) -> folly::coro::Task<bool> {
        co_await folly::coro::sleep(std::chrono::seconds(10));
        co_return HTTPSourceReader::Continue;
      })
      .onError([](const HTTPSourceReader::ErrorContext&, const HTTPError& err) {
        EXPECT_EQ(err.code, HTTPErrorCode::INTERNAL_ERROR);
        EXPECT_EQ(err.msg, "Operation cancelled");
      });
  co_withExecutor(&evb,
                  [](HTTPSourceReader& reader,
                     folly::CancellationSource& cancellationSource)
                      -> folly::coro::Task<void> {
                    co_await folly::coro::co_withCancellation(
                        cancellationSource.getToken(), reader.read());
                  }(reader, cancellationSource))
      .start();
  evb.loopOnce();
  cancellationSource.requestCancellation();
  evb.loop();
}

TEST(HTTPSourceReader, datagrams) {
  folly::EventBase evb;
  auto* source = new HTTPStreamSource(&evb);
  source->setHeapAllocated();
  // queue headers, 3x datagrams, then EOM
  source->headers(makeResponse(200));
  source->datagram(makeBuf(10));
  source->datagram(makeBuf(10));
  source->datagram(makeBuf(10));
  source->eom();

  size_t numDatagrams = 0, numBytesDatagrams = 0;

  HTTPSourceReader reader(source);
  reader
      .onHeaders(
          [](std::unique_ptr<HTTPMessage> headers, bool final, bool eom) {
            EXPECT_EQ(headers->getStatusCode(), 200);
            EXPECT_TRUE(final);
            EXPECT_FALSE(eom);
            return HTTPSourceReader::Continue; // Stop reading
          })
      .onDatagram([&](BufQueue buf) {
        numDatagrams++;
        numBytesDatagrams += buf.chainLength();
        return HTTPSourceReader::Continue;
      });
  co_withExecutor(&evb,
                  [](HTTPSourceReader& reader) -> folly::coro::Task<void> {
                    co_await reader.read();
                  }(reader))
      .start();
  evb.loop();

  EXPECT_EQ(numDatagrams, 3);
  EXPECT_EQ(numBytesDatagrams, 30);
}

TEST(HTTPSourceReader, error) {
  folly::EventBase evb;
  size_t events = 0;
  auto source =
      new HTTPErrorSource(HTTPError(HTTPErrorCode::PROTOCOL_ERROR, "oops"));

  HTTPSourceReader reader(source);
  reader.onError([&events](HTTPSourceReader::ErrorContext ec, HTTPError error) {
    events++;
    EXPECT_EQ(error.code, HTTPErrorCode::PROTOCOL_ERROR);
    EXPECT_EQ(ec, HTTPSourceReader::ErrorContext::HEADERS);
  });
  co_withExecutor(&evb,
                  [](HTTPSourceReader& reader) -> folly::coro::Task<void> {
                    co_await reader.read();
                  }(reader))
      .start();
  evb.loop();
  EXPECT_EQ(events, 1);
}

TEST(HTTPSourceReader, MaxReadSize) {
  folly::EventBase evb;
  MockHTTPSource mockSource;
  HTTPSourceReader reader{&mockSource};
  auto headerEv = HTTPHeaderEvent{makeResponse(200), /*inEOM=*/false};

  {
    InSequence s;
    EXPECT_CALL(mockSource, readHeaderEvent())
        .WillOnce(Return(folly::coro::makeTask(std::move(headerEv))));
    EXPECT_CALL(mockSource, readBodyEvent(100))
        .WillOnce(Return(folly::coro::makeTask(
            HTTPBodyEvent{makeBuf(100), /*inEOM=*/false})))
        .RetiresOnSaturation();
    EXPECT_CALL(mockSource, readBodyEvent(100))
        .WillOnce(Return(
            folly::coro::makeTask(HTTPBodyEvent{nullptr, /*inEOM=*/true})));
  }

  co_withExecutor(&evb, reader.read(/*maxBodySize=*/100)).start();
  evb.loop();
}

TEST(HTTPSourceReader, filter) {
  folly::EventBase evb;
  size_t events = 0;
  auto source = HTTPFixedSource::makeFixedResponse(200, makeBuf(100));

  HTTPSourceReader reader;
  class ByteCountFilter : public HTTPSourceFilter {
   public:
    folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override {
      XLOG(INFO) << __func__;
      auto bodyEvent = co_await readBodyEventImpl(max);
      bytes += (bodyEvent.eventType == HTTPBodyEvent::BODY)
                   ? bodyEvent.event.body.chainLength()
                   : 0;
      co_return bodyEvent;
    }
    size_t bytes = 0;
  };
  ByteCountFilter filter;
  reader.insertFilter(&filter);
  reader.setSource(source);
  reader
      .onHeaders([&events](std::unique_ptr<HTTPMessage> headers,
                           bool final,
                           bool eom) {
        events++;
        EXPECT_EQ(headers->getStatusCode(), 200);
        EXPECT_TRUE(final);
        EXPECT_FALSE(eom);
        return false;
      })
      .onBody([&events](BufQueue body, bool eom) {
        events++;
        EXPECT_EQ(body.chainLength(), 100);
        EXPECT_TRUE(eom);
        return false;
      });
  folly::coro::blockingWait(reader.read(), &evb);
  EXPECT_EQ(filter.bytes, 100);
  EXPECT_EQ(events, 2);
}

TEST(HTTPErrorTests, ErrorStrings) {
  EXPECT_EQ(getErrorString(HTTPErrorCode::PROTOCOL_ERROR), "PROTOCOL_ERROR");
  EXPECT_EQ(getErrorString(HTTPErrorCode::QPACK_DECOMPRESSION_FAILED),
            "QPACK_DECOMPRESSION_FAILED");
  EXPECT_EQ(getErrorString(HTTPErrorCode::INVALID_STATE_TRANSITION),
            "INVALID_STATE_TRANSITION");
  EXPECT_EQ(getErrorString(HTTPErrorCode(ErrorCode::NO_ERROR)), "NO_ERROR");
  EXPECT_EQ(getErrorString(HTTPErrorCode(HTTP3::ErrorCode::HTTP_NO_ERROR)),
            "_H3_NO_ERROR");
  EXPECT_EQ(getErrorString(HTTPErrorCode(100000)), "UNKNOWN_ERROR");
}

TEST(HTTPEventTest, DefaultTrailers) {
  HTTPBodyEvent trailerEvent1(std::make_unique<HTTPHeaders>());
  HTTPBodyEvent bodyEvent(nullptr, true);
  trailerEvent1 = std::move(bodyEvent);
}

CO_TEST(HybridSourceTest, BasicTest) {
  // Verify that a source holder holds a HybridSource instead of
  // taking over its source
  InSequence seq;

  auto request = std::make_unique<HTTPMessage>();
  request->setMethod("GET");
  request->setURL("https://test.facebook.com/");
  auto requestSource = std::make_unique<HTTPFixedSource>(std::move(request));
  HTTPSourceHolder requestSourceHolder;
  requestSourceHolder.setSource(requestSource.get());

  auto headers = co_await requestSourceHolder.readHeaderEvent();

  HTTPHybridSource hybridSource(std::move(headers.headers),
                                std::move(requestSourceHolder));
  HTTPSourceHolder combinedSourceHolder(&hybridSource);

  EXPECT_TRUE(bool(combinedSourceHolder));
}

CO_TEST(HybridSourceTest, StopReading) {
  auto request = std::make_unique<HTTPMessage>();
  request->setMethod("GET");
  request->setURL("https://test.facebook.com/");
  auto requestSource = std::make_unique<HTTPFixedSource>(std::move(request));
  HTTPSourceHolder requestSourceHolder;
  requestSourceHolder.setSource(requestSource.get());

  auto headers = co_await requestSourceHolder.readHeaderEvent();

  auto hybridSource = new HTTPHybridSource(std::move(headers.headers), nullptr);
  hybridSource->setHeapAllocated();
  HTTPSourceHolder combinedSourceHolder(hybridSource);

  EXPECT_TRUE(bool(combinedSourceHolder));

  combinedSourceHolder.stopReading();
}

class NameFilter : public HTTPSourceFilter {
 public:
  NameFilter(std::string inName, std::vector<std::string>& inNames)
      : name(std::move(inName)), names(inNames) {
    setHeapAllocated();
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    auto headerEvent = co_await readHeaderEventImpl();
    auto g = folly::makeGuard(lifetime(headerEvent));
    names.push_back(name);
    co_return headerEvent;
  }
  std::string name;
  std::vector<std::string>& names;
};

CO_TEST(FilterChainTest, SetBeforeInsert) {
  FilterChain chain;
  EXPECT_EQ(chain.head(), nullptr);
  auto reqSource = HTTPFixedSource::makeFixedRequest("/filterchain");
  chain.setSource(reqSource);
  EXPECT_NE(chain.head(), nullptr);
  std::vector<std::string> names;
  auto nameFilter = new NameFilter("filter1", names);
  chain.insertFront(nameFilter);
  nameFilter = new NameFilter("filter2", names);
  chain.insertEnd(nameFilter);
  auto headerEvent = co_await chain.head()->readHeaderEvent();
  // head_ -> filter1 -> filter2 -> source
  EXPECT_EQ(names.size(), 2);
  EXPECT_EQ(names[0], "filter2"); // vector is backwards
  EXPECT_EQ(names[1], "filter1");
  EXPECT_EQ(headerEvent.headers->getPathAsStringPiece(), "/filterchain");
}

CO_TEST(FilterChainTest, ReleaseFilterChain) {
  auto readChainTask = [](HTTPSourceHolder holder) -> folly::coro::Task<void> {
    auto headerEvent = co_await co_awaitTry(holder.readHeaderEvent());
    CHECK(!headerEvent.hasException());
    EXPECT_EQ(headerEvent->headers->getPathAsStringPiece(), "/filterchain");
    co_return;
  };
  FilterChain chain;
  EXPECT_EQ(chain.head(), nullptr);
  auto reqSource = HTTPFixedSource::makeFixedRequest("/filterchain");
  chain.setSource(reqSource);
  EXPECT_NE(chain.head(), nullptr);
  std::vector<std::string> names;
  auto nameFilter = new NameFilter("filter1", names);
  chain.insertFront(nameFilter);
  nameFilter = new NameFilter("filter2", names);
  chain.insertEnd(nameFilter);
  // release ownership
  co_await readChainTask(chain.release());
  // head_ -> filter1 -> filter2 -> source
  EXPECT_EQ(names.size(), 2);
  EXPECT_EQ(names[0], "filter2"); // vector is backwards
  EXPECT_EQ(names[1], "filter1");
}

CO_TEST(FilterChainTest, InsertBeforeSet) {
  FilterChain chain;
  EXPECT_EQ(chain.head(), nullptr);
  std::vector<std::string> names;
  auto nameFilter = new NameFilter("filter1", names);
  chain.insertEnd(nameFilter);
  EXPECT_EQ(chain.head(), nullptr);

  nameFilter = new NameFilter("filter2", names);
  chain.insertFront(nameFilter);
  EXPECT_EQ(chain.head(), nullptr);

  auto reqSource = HTTPFixedSource::makeFixedRequest("/filterchain");
  chain.setSource(reqSource);
  EXPECT_NE(chain.head(), nullptr);
  auto headerEvent = co_await chain.head()->readHeaderEvent();
  // head_ -> filter2 -> filter1 -> source
  EXPECT_EQ(names.size(), 2);
  EXPECT_EQ(names[0], "filter1"); // vector is backwards
  EXPECT_EQ(names[1], "filter2");
  EXPECT_EQ(headerEvent.headers->getPathAsStringPiece(), "/filterchain");
}

CO_TEST(FilterChainTest, StopReading) {
  FilterChain chain;
  EXPECT_EQ(chain.head(), nullptr);
  std::vector<std::string> names;
  auto nameFilter = new NameFilter("filter1", names);
  chain.insertEnd(nameFilter);
  EXPECT_EQ(chain.head(), nullptr);

  nameFilter = new NameFilter("filter2", names);
  chain.insertFront(nameFilter);
  EXPECT_EQ(chain.head(), nullptr);

  auto reqSource = HTTPFixedSource::makeFixedResponse(200, "hello world");
  chain.setSource(reqSource);
  EXPECT_NE(chain.head(), nullptr);
  auto headerEvent = co_await chain.head()->readHeaderEvent();
  // head_ -> filter2 -> filter1 -> source
  EXPECT_EQ(names.size(), 2);
  EXPECT_EQ(names[0], "filter1"); // vector is backwards
  EXPECT_EQ(names[1], "filter2");
  EXPECT_EQ(headerEvent.headers->getStatusCode(), 200);
  EXPECT_NE(chain.head(), nullptr); // not done yet
  // didn't read body, implicit stopReading from ~FilterChain
}

CO_TEST(ConsumerProducer, TestConcurrentAccess) {
  folly::ScopedEventBaseThread producerEvb{};
  folly::EventBase consumerEvb;

  size_t events = 0;
  auto source = new HTTPStreamSource(producerEvb.getEventBase());
  source->setHeapAllocated();

  // space out producer work to introduce randomness in execution
  producerEvb.add(
      [source]() { source->headers(makeResponse(200), /*eom=*/false); });
  producerEvb.add(
      [source]() { source->body(makeBuf(100), /*padding=*/0, /*eom=*/false); });
  producerEvb.add([source]() {
    // Add a push
    source->pushPromise(std::make_unique<HTTPMessage>(getGetRequest("/push")),
                        HTTPFixedSource::makeFixedResponse(200, makeBuf(50)),
                        /*eom=*/false);
  });
  producerEvb.add([source]() {
    // Add trailers
    auto trailers = std::make_unique<HTTPHeaders>();
    trailers->add("x-trailer1", "trailer1");
    source->trailers(std::move(trailers));
  });
  producerEvb.add([source]() { source->eom(); });

  auto execFilter = ExecutorSourceFilter::make(producerEvb.getEventBase());
  execFilter->setSource(source);
  HTTPSourceReader reader(execFilter.release());
  reader
      .preRead([&events]() {
        events++;
        return HTTPSourceReader::Continue;
      })
      .onHeaders([&events](std::unique_ptr<HTTPMessage> headers,
                           bool final,
                           bool eom) {
        events++;
        EXPECT_EQ(headers->getStatusCode(), 200);
        EXPECT_TRUE(final);
        EXPECT_FALSE(eom);
        return HTTPSourceReader::Continue;
      })
      .onPushPromise([&events](std::unique_ptr<HTTPMessage> promise,
                               HTTPSourceHolder,
                               bool eom) {
        events++;
        EXPECT_EQ(promise->getPathAsStringPiece(), "/push");
        EXPECT_FALSE(eom);
        // response gets a stopReading on dtor
        return HTTPSourceReader::Continue;
      })
      .onBody([&events](BufQueue body, bool eom) {
        events++;
        EXPECT_EQ(body.chainLength(), 100);
        EXPECT_FALSE(eom);
        return HTTPSourceReader::Continue;
      })
      .onTrailers([&events](std::unique_ptr<HTTPHeaders> trailers) {
        events++;
        EXPECT_EQ(trailers->size(), 1);
        EXPECT_EQ(trailers->getSingleOrEmpty("x-trailer1"), "trailer1");
      });
  co_withExecutor(&consumerEvb,
                  [](HTTPSourceReader& reader) -> folly::coro::Task<void> {
                    co_await reader.read();
                  }(reader))
      .start();
  consumerEvb.loop();
  co_return;
}

CO_TEST(ConsumerProducer, TestConcurrentAccessWithAbort) {
  folly::ScopedEventBaseThread producerEvb{};
  folly::EventBase consumerEvb;

  size_t events = 0;
  auto source = new HTTPStreamSource(producerEvb.getEventBase());
  source->setHeapAllocated();

  // space out producer work to introduce randomness in execution
  producerEvb.add(
      [source]() { source->headers(makeResponse(200), /*eom=*/false); });
  producerEvb.add(
      [source]() { source->body(makeBuf(100), /*padding=*/0, /*eom=*/false); });
  producerEvb.add([source]() { source->abort(HTTPErrorCode::CANCEL); });

  auto execFilter = ExecutorSourceFilter::make(producerEvb.getEventBase());
  execFilter->setSource(source);
  HTTPSourceReader reader(execFilter.release());
  reader
      .preRead([&events]() {
        events++;
        return HTTPSourceReader::Continue;
      })
      .onHeaders([&events](std::unique_ptr<HTTPMessage> headers,
                           bool final,
                           bool eom) {
        events++;
        EXPECT_EQ(headers->getStatusCode(), 200);
        EXPECT_TRUE(final);
        EXPECT_FALSE(eom);
        return HTTPSourceReader::Continue;
      })
      .onBody([](BufQueue body, bool eom) {
        EXPECT_EQ(body.chainLength(), 100);
        EXPECT_FALSE(eom);
        return HTTPSourceReader::Continue;
      });
  co_withExecutor(&consumerEvb,
                  [](HTTPSourceReader& reader) -> folly::coro::Task<void> {
                    co_await reader.read();
                  }(reader))
      .start();
  consumerEvb.loop();
  co_return;
}

CO_TEST(ConsumerProducer, TestHTTPStreamSourceHolderDestructor) {
  // HTTPStreamSourceHolder (wrapper around HTTPStreamSource) is not thread
  // safe, consumer and producer should be running within the same thread.
  // However, this tests that the destructor of HTTPStreamSourceHolder can be
  // invoked just fine outside of the consumerAndProducerEvb.
  folly::ScopedEventBaseThread consumerAndProducerEvb{};

  auto streamSourceHolder =
      HTTPStreamSourceHolder::make(consumerAndProducerEvb.getEventBase());
  CHECK(streamSourceHolder->get());

  // space out producer work to introduce randomness in execution
  consumerAndProducerEvb.add([streamSourceHolder]() {
    // source should still exist
    auto source = streamSourceHolder->get();
    CHECK(streamSourceHolder->get());
    source->headers(makeResponse(200), /*eom=*/false);
  });

  consumerAndProducerEvb.add([streamSourceHolder]() {
    // source should still exist
    auto source = streamSourceHolder->get();
    CHECK(streamSourceHolder->get());
    source->body(makeBuf(100), /*padding=*/0, /*eom=*/false);
  });

  consumerAndProducerEvb.add([streamSourceHolder]() {
    // source should still exist
    auto source = streamSourceHolder->get();
    CHECK(streamSourceHolder->get());
    source->datagram(makeBuf(100));
  });

  consumerAndProducerEvb.add([streamSourceHolder]() {
    // source should still exist
    auto source = streamSourceHolder->get();
    CHECK(streamSourceHolder->get());
    source->eom();
  });

  auto consumerTask = co_withExecutor(
      consumerAndProducerEvb.getEventBase(),
      folly::coro::co_invoke([streamSourceHolder]() -> folly::coro::Task<void> {
        auto maybeHeaders =
            co_await co_awaitTry(streamSourceHolder->get()->readHeaderEvent());
        CHECK(!maybeHeaders.hasException());

        // source should still exist
        CHECK(streamSourceHolder->get());
        auto maybeBody =
            co_await co_awaitTry(streamSourceHolder->get()->readBodyEvent());
        CHECK(!maybeBody.hasException());
        CHECK(maybeBody->eventType == HTTPBodyEvent::EventType::BODY);
        CHECK(maybeBody->event.body.chainLength() == 100 &&
              maybeBody->eom == false);

        // source should still exist
        CHECK(streamSourceHolder->get());
        auto maybeDatagram =
            co_await co_awaitTry(streamSourceHolder->get()->readBodyEvent());
        // this last event should be an abort so expect exception
        CHECK(!maybeDatagram.hasException() &&
              maybeDatagram->eventType == HTTPBodyEvent::EventType::DATAGRAM);

        // source should still exist
        CHECK(streamSourceHolder->get());
        auto maybeEom =
            co_await co_awaitTry(streamSourceHolder->get()->readBodyEvent());
        // this last event should be an abort so expect exception
        CHECK(!maybeEom.hasException() && maybeEom->eom);

        // source should no longer exist due to ::sourceComplete callback –
        // which should run destructor of the internal HTTPStreamSource within
        // the producer evb
        CHECK(streamSourceHolder->get() == nullptr);
      }));

  folly::coro::blockingWait(std::move(consumerTask));

  // destructor for HTTPStreamSourceHolder invoked here outside of
  // consumerAndProducerEvb, should be okay
  co_return;
}

TEST(HTTPStreamSourceHolder, Simple) {
  folly::EventBase evb;
  auto source = HTTPStreamSourceHolder::make(&evb);
  XCHECK(source->get());

  HTTPSourceReader reader{source->get()};
  co_withExecutor(&evb, reader.read()).start();

  source->get()->headers(makePostRequest(1'000));
  evb.loopOnce();

  for (uint8_t idx = 0; idx < 10; idx++) {
    source->get()->body(makeBuf(100), /*padding=*/0, /*eom=*/false);
    evb.loopOnce();
  }
  source->get()->eom();
  evb.loopOnce();

  evb.loop();
}

TEST(HTTPStreamSourceHolder, DestructorTest) {
  folly::EventBase evb;
  auto source = HTTPStreamSourceHolder::make(&evb);
  XCHECK(source->get());
  source->get()->headers(makePostRequest(1'000));
}

} // namespace proxygen::coro::test
// Test cases to write:
//
// body flow control error
// body in sink mode
// pushPromise sink mode
// onEOF
// readBodyEvent with no body events
// error from waitForEvent(body)
// enable sink mode with pending body
// event timeout
// release flow control
