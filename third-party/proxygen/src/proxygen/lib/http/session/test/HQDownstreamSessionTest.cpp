/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Expected.h>
#include <proxygen/lib/http/session/HQDownstreamSession.h>

#include <folly/io/async/EventBaseManager.h>
#include <proxygen/lib/http/codec/HQControlCodec.h>
#include <proxygen/lib/http/codec/HQStreamCodec.h>
#include <proxygen/lib/http/codec/HQUnidirectionalCodec.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/session/test/HQDownstreamSessionTest.h>
#include <proxygen/lib/http/session/test/HQSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <proxygen/lib/http/session/test/MockQuicSocketDriver.h>
#include <proxygen/lib/http/session/test/MockSessionObserver.h>
#include <proxygen/lib/http/session/test/TestUtils.h>
#include <quic/api/test/MockQuicSocket.h>
#include <quic/dsr/Types.h>
#include <wangle/acceptor/ConnectionManager.h>

#include <folly/futures/Future.h>
#include <folly/portability/GTest.h>

using namespace proxygen;
using namespace proxygen::hq;
using namespace quic;
using namespace testing;
using namespace std::chrono;

using HQDownstreamSessionTestDeliveryAck = HQDownstreamSessionTest;

// Use this test class for h3 server push tests
using HQDownstreamSessionTestPush = HQDownstreamSessionTest;

namespace {
HTTPMessage getProgressiveGetRequest() {
  auto req = proxygen::getGetRequest();
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=1, i");
  return req;
}
} // namespace

HTTPCodec::StreamID HQDownstreamSessionTest::sendRequest(const std::string& url,
                                                         int8_t priority,
                                                         bool eom) {
  auto req = proxygen::getGetRequest();
  req.setURL(url);
  req.setPriority(priority);
  return sendRequest(req, eom);
}

quic::StreamId HQDownstreamSessionTest::nextStreamId() {
  auto id = nextStreamId_;
  nextStreamId_ += 4;
  return id;
}

quic::StreamId HQDownstreamSessionTest::sendRequest(
    const proxygen::HTTPMessage& req, bool eom, quic::StreamId id) {
  if (id == quic::kEightByteLimit) {
    id = nextStreamId();
  }
  auto res = requests_.emplace(std::piecewise_construct,
                               std::forward_as_tuple(id),
                               std::forward_as_tuple(makeCodec(id)));
  auto& request = res.first->second;
  request.id = request.codec->createStream();
  request.readEOF = eom;
  request.codec->generateHeader(request.buf, request.id, req, eom);
  return id;
}

quic::StreamId HQDownstreamSessionTest::sendHeader() {
  return sendRequest("/", 0, false);
}

folly::Promise<folly::Unit> HQDownstreamSessionTest::sendRequestLater(
    proxygen::HTTPMessage req, bool eof) {
  folly::Promise<folly::Unit> reqp;
  reqp.getSemiFuture().via(&eventBase_).thenValue([=](auto&&) {
    auto id = sendRequest(req, eof);
    socketDriver_->addReadEvent(
        id, getStream(id).buf.move(), std::chrono::milliseconds(0));
    socketDriver_->addReadEOF(id, std::chrono::milliseconds(0));
    // note that eof=true used to terminate the connection and now it
    // no longer does
  });
  return reqp;
}

void HQDownstreamSessionTest::SetUp() {
  SetUpBase();
  SetUpOnTransportReady();
}

void HQDownstreamSessionTest::TearDown() {
  // with these versions we need to wait for GOAWAY delivery on the control
  // stream
  eventBase_.loop();
}

void HQDownstreamSessionTest::SetUpBase() {
  HQSessionTest::SetUp();
  streamTransInfo_ = {.totalHeadOfLineBlockedTime =
                          std::chrono::milliseconds(100),
                      .holbCount = 2,
                      .isHolb = true};

  EXPECT_CALL(*socketDriver_->getSocket(), getStreamTransportInfo(testing::_))
      .WillRepeatedly(testing::Return(streamTransInfo_));
}

void HQDownstreamSessionTest::SetUpOnTransportReady() {
  hqSession_->onTransportReady();

  if (createControlStreams()) {
    eventBase_.loopOnce();
    EXPECT_EQ(httpCallbacks_.settings, 1);
  }
}

template <class HandlerType>
std::unique_ptr<testing::StrictMock<HandlerType>>
HQDownstreamSessionTest::addSimpleStrictHandlerBase() {
  auto handler = std::make_unique<testing::StrictMock<HandlerType>>();

  // The ownership model here is suspect, but assume the callers won't destroy
  // handler before it's requested
  auto rawHandler = handler.get();
  EXPECT_CALL(getMockController(), getRequestHandler(testing::_, testing::_))
      .WillOnce(testing::Return(rawHandler))
      .RetiresOnSaturation();

  EXPECT_CALL(*handler, _setTransaction(testing::_))
      .WillOnce(testing::SaveArg<0>(&handler->txn_));

  return handler;
}

std::unique_ptr<testing::StrictMock<proxygen::MockHTTPHandler>>
HQDownstreamSessionTest::addSimpleStrictHandler() {
  return addSimpleStrictHandlerBase<proxygen::MockHTTPHandler>();
}

std::pair<quic::StreamId,
          std::unique_ptr<testing::StrictMock<proxygen::MockHTTPHandler>>>
HQDownstreamSessionTest::checkRequest(proxygen::HTTPMessage req) {
  auto id = sendRequest(req);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM(
      [hdlr = handler.get()] { hdlr->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();
  return {id, std::move(handler)};
}

void HQDownstreamSessionTest::flushRequestsAndWaitForReads(
    bool eof,
    std::chrono::milliseconds eofDelay,
    std::chrono::milliseconds initialDelay,
    std::function<void()> extraEventsFn) {
  while (!flushRequests(eof, eofDelay, initialDelay, extraEventsFn)) {
    CHECK(eventBase_.loop());
  }
  CHECK(eventBase_.loop());
}

void HQDownstreamSessionTest::flushRequestsAndLoop(
    bool eof,
    std::chrono::milliseconds eofDelay,
    std::chrono::milliseconds initialDelay,
    std::function<void()> extraEventsFn) {
  flushRequests(eof, eofDelay, initialDelay, extraEventsFn);
  CHECK(eventBase_.loop());
}

void HQDownstreamSessionTest::flushRequestsAndLoopN(
    uint64_t n,
    bool eof,
    std::chrono::milliseconds eofDelay,
    std::chrono::milliseconds initialDelay,
    std::function<void()> extraEventsFn) {
  flushRequests(eof, eofDelay, initialDelay, extraEventsFn);
  for (uint64_t i = 0; i < n; i++) {
    eventBase_.loopOnce();
  }
}

bool HQDownstreamSessionTest::flushRequests(
    bool eof,
    std::chrono::milliseconds eofDelay,
    std::chrono::milliseconds initialDelay,
    std::function<void()> extraEventsFn) {
  bool done = true;

  if (!encoderWriteBuf_.empty()) {
    socketDriver_->addReadEvent(
        kQPACKEncoderIngressStreamId, encoderWriteBuf_.move(), initialDelay);
    initialDelay = std::chrono::milliseconds(0);
  }
  for (auto& req : requests_) {
    if (socketDriver_->isStreamIdle(req.first)) {
      continue;
    }
    if (req.second.buf.chainLength() > 0) {
      socketDriver_->addReadEvent(
          req.first, req.second.buf.move(), initialDelay);
      done = false;
    }
    // EOM -> stream EOF
    if (req.second.readEOF) {
      socketDriver_->addReadEOF(req.first, eofDelay);
      done = false;
    }
  }
  if (extraEventsFn) {
    extraEventsFn();
  }
  if (eof || eofDelay.count() > 0) {
    /*  wonkiness.  Should somehow close the connection?
     * socketDriver_->addReadEOF(1, eofDelay);
     */
  }
  return done;
}

testing::StrictMock<proxygen::MockController>&
HQDownstreamSessionTest::getMockController() {
  return controllerContainer_.mockController;
}

std::unique_ptr<proxygen::HTTPCodec> HQDownstreamSessionTest::makeCodec(
    proxygen::HTTPCodec::StreamID id) {
  return std::make_unique<proxygen::hq::HQStreamCodec>(
      id,
      proxygen::TransportDirection::UPSTREAM,
      qpackCodec_,
      encoderWriteBuf_,
      decoderWriteBuf_,
      [] { return std::numeric_limits<uint64_t>::max(); },
      ingressSettings_);
}

HQDownstreamSessionTest::ClientStream& HQDownstreamSessionTest::getStream(
    proxygen::HTTPCodec::StreamID id) {
  auto it = requests_.find(id);
  CHECK(it != requests_.end());
  return it->second;
}

void HQDownstreamSessionTest::expectTransactionTimeout(
    testing::StrictMock<proxygen::MockHTTPHandler>& handler,
    folly::Function<void()> fn) {
  EXPECT_CALL(getMockController(),
              getTransactionTimeoutHandler(testing::_, testing::_))
      .WillOnce(testing::Return(&handler));
  EXPECT_CALL(handler, _setTransaction(testing::_))
      .WillOnce(testing::SaveArg<0>(&handler.txn_));
  handler.expectError(
      [&handler, &fn](const proxygen::HTTPException& ex) mutable {
        if (fn) {
          fn();
        }
        EXPECT_FALSE(ex.hasHttpStatusCode());
        handler.sendHeaders(408, 100);
        handler.sendBody(100);
        handler.sendEOM();
      });
  handler.expectDetachTransaction();
}

std::unique_ptr<MockSessionObserver>
HQDownstreamSessionTest::addMockSessionObserver(
    MockSessionObserver::EventSet eventSet) {
  auto observer = std::make_unique<NiceMock<MockSessionObserver>>(eventSet);
  EXPECT_CALL(*observer, attached(_));
  hqSession_->addObserver(observer.get());
  return observer;
}

std::shared_ptr<MockSessionObserver>
HQDownstreamSessionTest::addMockSessionObserverShared(
    MockSessionObserver::EventSet eventSet) {
  auto observer = std::make_shared<NiceMock<MockSessionObserver>>(eventSet);
  EXPECT_CALL(*observer, attached(_));
  hqSession_->addObserver(observer);
  return observer;
}

TEST_P(HQDownstreamSessionTest, GetMaxPushIdOK) {
  folly::Optional<hq::PushId> expectedId = hqSession_->getMaxAllowedPushId();
  EXPECT_EQ(expectedId, folly::none);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, SimpleGet) {
  auto idh = checkRequest();
  flushRequestsAndLoop();
  EXPECT_GT(socketDriver_->streams_[idh.first].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[idh.first].writeEOF);
  // Checks that the server response is sent using the QPACK dynamic table
  CHECK_GE(qpackCodec_.getCompressionInfo().ingress.headerTableSize_, 0);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, PriorityUpdateIntoTransport) {
  auto request = getProgressiveGetRequest();
  sendRequest(request);
  auto handler = addSimpleStrictHandler();
  EXPECT_CALL(*socketDriver_->getSocket(),
              setStreamPriority(_, Priority(1, true)));
  handler->expectHeaders();
  handler->expectEOM([&]() {
    auto resp = makeResponse(200, 0);
    std::get<0>(resp)->getHeaders().add(HTTP_HEADER_PRIORITY, "u=2");
    EXPECT_CALL(*socketDriver_->getSocket(),
                setStreamPriority(_, Priority(2, false)))
        .Times(1);
    handler->sendRequest(*std::get<0>(resp));
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, ReplyResponsePriority) {
  auto request = getProgressiveGetRequest();
  sendRequest(request);
  auto handler = addSimpleStrictHandler();
  EXPECT_CALL(*socketDriver_->getSocket(),
              setStreamPriority(_, Priority(1, true)))
      .Times(1);
  handler->expectHeaders();
  handler->expectEOM([&]() {
    auto resp = makeResponse(200, 0);
    EXPECT_CALL(*socketDriver_->getSocket(), getStreamPriority(_))
        .Times(1)
        .WillOnce(Return(quic::Priority(1, true)));
    handler->sendRequest(*std::get<0>(resp));
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, SetLocalPriorityOnHeadersComplete) {
  uint8_t urgency = 5;
  bool incremental = false;
  auto request = getProgressiveGetRequest();
  sendRequest(request);
  auto handler = addSimpleStrictHandler();
  // Check that the priority is set from the request headers
  EXPECT_CALL(*socketDriver_->getSocket(),
              setStreamPriority(_, Priority(1, true)))
      .Times(1);
  // Check that the priority is set from the local update
  EXPECT_CALL(*socketDriver_->getSocket(),
              setStreamPriority(_, Priority(urgency, incremental)))
      .Times(1);
  handler->expectHeaders([&]() {
    handler->txn_->updateAndSendPriority(HTTPPriority(urgency, incremental));
  });
  handler->expectEOM([&]() {
    auto resp = makeResponse(200, 0);
    EXPECT_CALL(*socketDriver_->getSocket(), getStreamPriority(_))
        .Times(1)
        .WillOnce(Return(quic::Priority(urgency, incremental)));
    handler->sendRequest(*std::get<0>(resp));
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, SetLocalPriorityAfterClientEOM) {
  uint8_t urgency = 5;
  bool incremental = false;
  auto request = getProgressiveGetRequest();
  sendRequest(request);
  auto handler = addSimpleStrictHandler();
  // Check that the priority is set from the request headers
  EXPECT_CALL(*socketDriver_->getSocket(),
              setStreamPriority(_, Priority(1, true)))
      .Times(1);
  // Check that the priority is set from the local update
  EXPECT_CALL(*socketDriver_->getSocket(),
              setStreamPriority(_, Priority(urgency, incremental)))
      .Times(1);
  handler->expectHeaders();
  handler->expectEOM([&]() {
    handler->txn_->updateAndSendPriority(HTTPPriority(urgency, incremental));
    auto resp = makeResponse(200, 0);
    EXPECT_CALL(*socketDriver_->getSocket(), getStreamPriority(_))
        .Times(1)
        .WillOnce(Return(quic::Priority(urgency, incremental)));
    handler->sendRequest(*std::get<0>(resp));
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTestPush, PushPriority) {
  sendRequest("/", 1);
  HTTPMessage promiseReq, parentResp;
  promiseReq.getHeaders().set(HTTP_HEADER_HOST, "www.foo.com");
  promiseReq.setURL("/");
  promiseReq.setHTTPPriority(0, false);

  parentResp.setStatusCode(200);
  parentResp.setStatusMessage("Ohai");

  HTTPMessage pushResp(parentResp);
  pushResp.setHTTPPriority(1, false);

  auto handler = addSimpleStrictHandler();
  StrictMock<MockHTTPPushHandler> pushHandler;
  handler->expectHeaders();
  HTTPCodec::StreamID pushStreamId = 0;
  handler->expectEOM([&] {
    EXPECT_CALL(*socketDriver_->getSocket(),
                setStreamPriority(handler->txn_->getID(), _))
        .Times(0);
    handler->txn_->sendHeaders(parentResp);
    handler->txn_->sendBody(makeBuf(100));

    auto outgoingStreams = hqSession_->getNumOutgoingStreams();
    auto* pushTxn = handler->txn_->newPushedTransaction(&pushHandler);
    ASSERT_NE(pushTxn, nullptr);
    EXPECT_EQ(hqSession_->getNumOutgoingStreams(), outgoingStreams + 1);
    // PushPromise doesn't update parent streram's priority. It does update push
    // stream priority
    EXPECT_CALL(*socketDriver_->getSocket(),
                setStreamPriority(handler->txn_->getID(), _))
        .Times(0);
    EXPECT_CALL(*socketDriver_->getSocket(),
                setStreamPriority(pushTxn->getID(), Priority(0, false)))
        .Times(1);
    pushTxn->sendHeaders(promiseReq);
    pushStreamId = pushTxn->getID();
    EXPECT_CALL(*socketDriver_->getSocket(),
                setStreamPriority(pushStreamId, Priority(1, false)))
        .Times(1);
    pushTxn->sendHeaders(pushResp);
    pushTxn->sendBody(makeBuf(200));
    pushTxn->sendEOM();
  });
  EXPECT_CALL(pushHandler, _setTransaction(_))
      .WillOnce(Invoke([&](HTTPTransaction* txn) { pushHandler.txn_ = txn; }));
  EXPECT_CALL(pushHandler, _detachTransaction());

  flushRequestsAndLoopN(1);
  handler->txn_->sendEOM();
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, OnPriorityCallback) {
  // Simulate priority arriving too early, connection still valid
  // this is going to be stored and applied when receiving headers
  hqSession_->onPriority(0, HTTPPriority(3, false));
  EXPECT_CALL(*socketDriver_->getSocket(),
              setStreamPriority(0, Priority(3, false)));
  auto id = sendRequest(getProgressiveGetRequest());
  CHECK_EQ(id, 0);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&]() {
    handler->sendHeaders(200, 1000);
    // Priority update on the stream
    EXPECT_CALL(*socketDriver_->getSocket(),
                setStreamPriority(0, Priority(2, true)));
    hqSession_->onPriority(id, HTTPPriority(2, true));
    handler->sendBody(1000);
    handler->sendEOM();
  });
  handler->expectEOM();
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  // Priority on a stream we can't find - no-op
  hqSession_->onPriority(id, HTTPPriority(4, true));
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, GetStopSending) {
  auto id = sendRequest(getGetRequest());
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([hdlr = handler.get()] { hdlr->sendHeaders(200, 100); });
  handler->expectError([](const proxygen::HTTPException& ex) {
    EXPECT_EQ(ex.getCodecStatusCode(), ErrorCode::CANCEL);
    EXPECT_EQ(ex.getProxygenError(), kErrorStreamAbort);
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoopN(1);
  socketDriver_->addStopSending(id, HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED);
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, HttpRateLimitNormal) {
  // The rate-limiting code grabs the event base from the EventBaseManager,
  // so we need to set it.
  folly::EventBaseManager::get()->setEventBase(&eventBase_, false);
  uint32_t rspLengthBytes = 100000;

  // make sure we are not limited by connection flow control
  socketDriver_->getSocket()->setConnectionFlowControlWindow(rspLengthBytes *
                                                             2);
  // Create a request
  auto id = sendRequest();

  // Set a low rate-limit on the transaction
  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders([&] {
    uint32_t rateLimit_kbps = 640;
    handler1->txn_->setEgressRateLimit(rateLimit_kbps * 1024);
  });
  // Send a somewhat big response that we know will get rate-limited
  handler1->expectEOM([&] {
    // At 640kbps, this should take slightly over 800ms
    handler1->sendHeaders(200, rspLengthBytes);
    handler1->sendBody(rspLengthBytes);
  });
  EXPECT_CALL(*handler1, _onEgressPaused()).Times(AtLeast(1));
  handler1->expectEgressResumed([&handler1] { handler1->txn_->sendEOM(); });
  handler1->expectDetachTransaction();
  flushRequestsAndLoop();

  // Check that the write side got blocked
  socketDriver_->expectStreamWritesPaused(id);
  // Open flow control again
  socketDriver_->getSocket()->setStreamFlowControlWindow(id,
                                                         rspLengthBytes * 2);
  flushRequestsAndLoop();

  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, SimplePost) {
  auto id = sendRequest(getPostRequest(10), false);
  auto& request = getStream(id);
  request.codec->generateBody(
      request.buf, request.id, makeBuf(10), HTTPCodec::NoPadding, true);
  request.readEOF = true;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectBody(); // should check length too but meh
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, SimpleGetEofDelay) {
  auto idh = checkRequest();
  flushRequestsAndLoop(false, std::chrono::milliseconds(10));
  EXPECT_GT(socketDriver_->streams_[idh.first].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[idh.first].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, UnfinishedPost) {
  auto id = sendRequest(getPostRequest(10), false);
  auto& request = getStream(id);
  request.codec->generateBody(
      request.buf, request.id, makeBuf(9), HTTPCodec::NoPadding, true);
  request.readEOF = true;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectError([&handler](const proxygen::HTTPException& ex) {
    // The HTTP/1.1 parser tracks content-length and 400's if it is short
    // The HQStreamCodec does no such thing, and it's caught by
    // HTTPTransaction, with a different error.
    EXPECT_EQ(ex.getProxygenError(), kErrorParseBody);
    handler->sendReplyWithBody(400, 100);
    // afrind: this logic is in HTTPSession so should move to base or
    // duplicate in HQSession (see also custom error handlers)
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->dropConnection();
}

TEST_P(HQDownstreamSessionTest, Multiplexing) {
  std::vector<std::unique_ptr<StrictMock<MockHTTPHandler>>> handlers;
  for (auto n = 0; n < 10; n++) {
    auto idh = checkRequest();
    handlers.emplace_back(std::move(idh.second));
  }
  flushRequestsAndWaitForReads();
  for (auto& req : requests_) {
    EXPECT_GT(socketDriver_->streams_[req.first].writeBuf.chainLength(), 110);
    EXPECT_TRUE(socketDriver_->streams_[req.first].writeEOF);
  }
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, Maxreadsperloop) {
  std::vector<std::unique_ptr<StrictMock<MockHTTPHandler>>> handlers;
  for (auto n = 0; n < 20; n++) {
    auto idh = checkRequest();
    handlers.emplace_back(std::move(idh.second));
  }

  flushRequestsAndLoopN(1);
  // After one loop, reads on some streams will be idle
  // while on some other they will not
  int idleCount = 0;
  int nonIdleCount = 0;
  for (auto& req : requests_) {
    if (socketDriver_->isStreamIdle(req.first)) {
      idleCount++;
    } else {
      nonIdleCount++;
    }
  }
  EXPECT_GT(idleCount, 0);
  EXPECT_GT(nonIdleCount, 0);

  // Now finish all the reads
  eventBase_.loop();
  for (auto& req : requests_) {
    EXPECT_GT(socketDriver_->streams_[req.first].writeBuf.chainLength(), 110);
    EXPECT_TRUE(socketDriver_->streams_[req.first].writeEOF);
  }
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, OnFlowControlUpdate) {
  auto id = sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler] {
    handler->sendHeaders(200, 100);
    handler->txn_->sendBody(makeBuf(100));
  });
  handler->expectEgressPaused();
  handler->expectEgressResumed([&handler] { handler->txn_->sendEOM(); });
  handler->expectDetachTransaction();

  // Initialize the flow control window to less than the response body
  socketDriver_->setStreamFlowControlWindow(id, 10);
  flushRequestsAndLoop();
  // Check that the write side got blocked
  socketDriver_->expectStreamWritesPaused(id);
  // Open the flow control window
  socketDriver_->getSocket()->setStreamFlowControlWindow(id, 200);
  CHECK(eventBase_.loop());
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, OnFlowControlUpdateOnUnknownStream) {
  auto id = sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();

  // Call flowControlUpdate on a stream the Application doesn't know
  socketDriver_->sock_->connCb_->onFlowControlUpdate(id + 4);
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

// This test does not work with header compression
TEST_P(HQDownstreamSessionTest, OnConnectionWindowPartialHeaders) {
  // Only enough conn window to send headers initially.
  auto id = sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();

  // Initialize the flow control window to less than the response body
  socketDriver_->setConnectionFlowControlWindow(10 + numCtrlStreams_);
  flushRequestsAndLoop();
  // Check that the write side got blocked
  socketDriver_->expectConnWritesPaused();
  // We should have some bytes pending to be written out in the QPACK Encoder
  // stream
  EXPECT_GT(socketDriver_->streams_[kQPACKEncoderEgressStreamId]
                .writeBuf.chainLength(),
            0);
  EXPECT_FALSE(socketDriver_->streams_[id].writeEOF);
  // Open the flow control window
  socketDriver_->getSocket()->setConnectionFlowControlWindow(200);
  CHECK(eventBase_.loop());
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, OnConnectionWindowPartialBody) {
  flushRequestsAndLoop(); // loop once for SETTINGS, etc
  // Only enough conn window to send headers initially.
  auto id = sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  // TODO: we should probably pause egress on conn limited.
  // handler->expectEgressPaused();
  // handler->expectEgressResumed();
  handler->expectDetachTransaction();

  // Initialize the flow control window to less than the response body
  socketDriver_->setConnectionFlowControlWindow(110 + numCtrlStreams_);
  flushRequestsAndLoop();
  // Check that the write side got blocked
  socketDriver_->expectConnWritesPaused();
  // We should have some bytes pending to be written out in the QPACK Encoder
  // stream
  EXPECT_GT(socketDriver_->streams_[kQPACKEncoderEgressStreamId]
                .writeBuf.chainLength(),
            0);
  EXPECT_GT(qpackCodec_.getCompressionInfo().egress.headerTableSize_, 0);
  EXPECT_FALSE(socketDriver_->streams_[id].writeEOF);
  // Open the flow control window
  socketDriver_->getSocket()->setConnectionFlowControlWindow(200 +
                                                             numCtrlStreams_);
  CHECK(eventBase_.loop());
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, SeparateEom) {
  // Only enough conn window to send headers initially.
  auto id = sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler] {
    handler->sendHeaders(200, 100);
    handler->sendBody(100);
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_FALSE(socketDriver_->streams_[id].writeEOF);

  handler->sendEOM();
  // Open the flow control window
  CHECK(eventBase_.loop());
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->closeWhenIdle();
}

std::unique_ptr<folly::IOBuf> getSimpleRequestData() {
  std::string req("GET / HTTP/1.1\nHost: www.facebook.com\n\n");
  return folly::IOBuf::copyBuffer(req);
}

std::tuple<size_t, size_t, size_t> estimateResponseSize(bool isHq,
                                                        HTTPMessage msg,
                                                        size_t contentLength,
                                                        size_t chunkSize) {
  folly::IOBufQueue estimateSizeBuf{folly::IOBufQueue::cacheChainLength()};
  std::unique_ptr<HTTPCodec> codec;
  QPACKCodec qpackCodec;
  folly::IOBufQueue encoderWriteBuf{folly::IOBufQueue::cacheChainLength()};
  folly::IOBufQueue decoderWriteBuf{folly::IOBufQueue::cacheChainLength()};
  HTTPSettings dummySettings;
  qpackCodec.setEncoderHeaderTableSize(kQPACKTestDecoderMaxTableSize);
  if (isHq) {
    codec = std::make_unique<hq::HQStreamCodec>(
        0,
        TransportDirection::DOWNSTREAM,
        qpackCodec,
        encoderWriteBuf,
        decoderWriteBuf,
        [] { return std::numeric_limits<uint64_t>::max(); },
        dummySettings);
  } else {
    codec = std::make_unique<HTTP1xCodec>(TransportDirection::DOWNSTREAM, true);
  }

  MockHTTPCodecCallback callback;
  codec->setCallback(&callback);
  auto txn = codec->createStream();

  if (!isHq) {
    EXPECT_CALL(callback, onHeadersComplete(_, _));
    EXPECT_CALL(callback, onMessageBegin(_, _));
    codec->onIngress(*getSimpleRequestData());
  }

  codec->generateHeader(estimateSizeBuf, txn, msg);
  size_t currentLength = contentLength;

  bool chunking = (chunkSize != 0);
  if (!chunking) {
    chunkSize = std::numeric_limits<size_t>::max();
  }
  auto currentSize = estimateSizeBuf.chainLength();
  while (currentLength > 0) {
    uint32_t toSend = std::min(currentLength, chunkSize);
    std::vector<uint8_t> buf;
    buf.resize(toSend, 'a');
    if (chunking) {
      codec->generateChunkHeader(estimateSizeBuf, txn, toSend);
    }
    codec->generateBody(estimateSizeBuf,
                        txn,
                        folly::IOBuf::copyBuffer(buf),
                        HTTPCodec::NoPadding,
                        false);
    if (chunking) {
      codec->generateChunkTerminator(estimateSizeBuf, txn);
    }
    currentLength -= toSend;
  }
  size_t framingOverhead =
      estimateSizeBuf.chainLength() - currentSize - contentLength;
  currentSize = estimateSizeBuf.chainLength();
  codec->generateEOM(estimateSizeBuf, txn);

  size_t eomSize = estimateSizeBuf.chainLength() - currentSize;
  size_t estimatedSize = estimateSizeBuf.chainLength();
  return std::tuple<size_t, size_t, size_t>(
      estimatedSize, framingOverhead, eomSize);
}

TEST_P(HQDownstreamSessionTest, PendingEomBuffered) {
  size_t contentLength = 100;
  size_t chunkSize = 5;

  auto reply = makeResponse(200);
  reply->setIsChunked(true);
  size_t estimatedSize = 0;
  size_t framingOverhead = 0;
  size_t eomSize = 0;
  std::tie(estimatedSize, framingOverhead, eomSize) =
      estimateResponseSize(true, *reply, contentLength, chunkSize);
  // EOMs are 0 bytes in H3, but there is framing overhead of at least two
  // bytes.
  auto bytesWithheld = 2;

  auto id = sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler, contentLength, chunkSize] {
    handler->sendChunkedReplyWithBody(
        200, contentLength, chunkSize, false, true);
  });

  // Set the flow control window to be less than the EOM overhead added by
  // the codec
  socketDriver_->setStreamFlowControlWindow(id, estimatedSize - bytesWithheld);
  flushRequestsAndLoop();
  CHECK(eventBase_.loop());
  EXPECT_GE(socketDriver_->streams_[id].writeBuf.chainLength(),
            estimatedSize - bytesWithheld);
  EXPECT_FALSE(socketDriver_->streams_[id].writeEOF);

  handler->expectDetachTransaction();
  socketDriver_->getSocket()->setStreamFlowControlWindow(id, estimatedSize);

  CHECK(eventBase_.loop());
  EXPECT_GE(socketDriver_->streams_[id].writeBuf.chainLength(), estimatedSize);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, PendingEomQueuedNotFlushed) {
  auto reply = makeResponse(200);
  reply->setWantsKeepalive(true);
  reply->getHeaders().add(HTTP_HEADER_CONTENT_LENGTH,
                          folly::to<std::string>(1));
  size_t estimatedSize = 0;
  size_t framingOverhead = 0;
  size_t eomSize = 0;
  std::tie(estimatedSize, framingOverhead, eomSize) =
      estimateResponseSize(true, *reply, 1, 0);
  CHECK_EQ(eomSize, 0);
  auto bytesWithheld = framingOverhead;

  auto id = sendRequest(getGetRequest());
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler, this, id, estimatedSize, bytesWithheld] {
    // Initialize the flow control window to just less than the
    // estimated size of the eom codec which the codec generates..
    socketDriver_->setStreamFlowControlWindow(id,
                                              estimatedSize - bytesWithheld);
    handler->sendReplyWithBody(200, 1);
  });

  flushRequestsAndLoop();
  CHECK(eventBase_.loop());
  EXPECT_GE(socketDriver_->streams_[id].writeBuf.chainLength(),
            estimatedSize - bytesWithheld);
  EXPECT_FALSE(socketDriver_->streams_[id].writeEOF);

  handler->expectDetachTransaction();
  socketDriver_->getSocket()->setStreamFlowControlWindow(id, estimatedSize);

  CHECK(eventBase_.loop());
  EXPECT_GE(socketDriver_->streams_[id].writeBuf.chainLength(), estimatedSize);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, PendingEomQueuedNotFlushedConn) {
  // flush control streams first
  flushRequestsAndLoop();
  CHECK(eventBase_.loop());

  auto reply = makeResponse(200);
  reply->setWantsKeepalive(true);
  size_t estimatedSize = 0;
  size_t framingOverhead = 0;
  size_t eomSize = 0;
  std::tie(estimatedSize, framingOverhead, eomSize) =
      estimateResponseSize(true, *reply, 1, 0);

  // No EOM yet
  auto id = sendRequest(getGetRequest(), false);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler] { handler->sendHeaders(200, 1); });
  flushRequestsAndLoopN(1);

  socketDriver_->addReadEOF(id, std::chrono::milliseconds(0));
  handler->expectEOM([&handler, this] {
    handler->txn_->sendBody(makeBuf(1));
    handler->txn_->sendEOM();
    socketDriver_->setConnectionFlowControlWindow(1);
  });

  // Set the conn flow control to be enough for the body byte but not enough
  // for the framing overhead
  auto remaining = framingOverhead + eomSize;
  flushRequestsAndLoop();
  CHECK(eventBase_.loop());
  EXPECT_GE(socketDriver_->streams_[id].writeBuf.chainLength(),
            estimatedSize - remaining);
  EXPECT_FALSE(socketDriver_->streams_[id].writeEOF);

  handler->expectDetachTransaction();
  for (size_t i = 0; i < remaining; i++) {
    socketDriver_->getSocket()->setConnectionFlowControlWindow(1);

    CHECK(eventBase_.loop());
    EXPECT_GE(socketDriver_->streams_[id].writeBuf.chainLength(),
              estimatedSize - remaining + i);

    EXPECT_TRUE(socketDriver_->streams_[id].writeEOF != (i < remaining - 1));
  }

  // Need flow control for goaway
  socketDriver_->getSocket()->setConnectionFlowControlWindow(100);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, SendEomLaterChunked) {
  size_t contentLength = 100;
  size_t chunkSize = 10;

  auto id = sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler, contentLength, chunkSize] {
    handler->sendChunkedReplyWithBody(
        200, contentLength, chunkSize, false, false);
  });
  handler->expectEOM([&handler] { handler->sendEOM(); });
  handler->expectDetachTransaction();

  flushRequestsAndLoop();
  CHECK(eventBase_.loop());
  EXPECT_GE(socketDriver_->streams_[id].writeBuf.chainLength(), contentLength);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, SendEomLater) {
  size_t contentLength = 100;
  auto id = sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler, contentLength] {
    handler->sendHeaders(200, contentLength);
    handler->sendBody(contentLength);
  });
  handler->expectEOM([&handler] { handler->sendEOM(); });
  handler->expectDetachTransaction();

  flushRequestsAndLoop();
  CHECK(eventBase_.loop());
  EXPECT_GE(socketDriver_->streams_[id].writeBuf.chainLength(), contentLength);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->closeWhenIdle();
}

// Invoke notifyPendingShutdown, which will include an outgoing
// Connection: close header on the next outbound headers.  The next incoming
// request containing a Connection: close header will complete the drain state
// machine

// closeWhenIdle on an idle conn - immediate delete
TEST_P(HQDownstreamSessionTest, ShutdownCloseIdle) {
  EXPECT_TRUE(hqSession_->isReusable());
  hqSession_->closeWhenIdle();
}

// closeWhenIdle invoked when a request is open, delete happens when it finishes
TEST_P(HQDownstreamSessionTest, ShutdownCloseIdleReq) {
  sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([this] {
    hqSession_->closeWhenIdle();
    EXPECT_TRUE(hqSession_->isClosing());
  });
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
}

// dropConnection invoked while a request being processed, it receives an
// error
TEST_P(HQDownstreamSessionTest, ShutdownDropWithReq) {
  sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM();
  handler->expectError();
  handler->expectDetachTransaction();
  flushRequestsAndLoopN(1);
  hqSession_->dropConnection();
}

// dropConnection invoked while a request is partial, it receives an
// error from the transport
TEST_P(HQDownstreamSessionTest, ShutdownDropWithPartialReq) {
  sendRequest(getPostRequest(10), false);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectError();
  handler->expectDetachTransaction();
  flushRequestsAndLoopN(1);
  hqSession_->dropConnection();
}

// Call drop connection while there are bytes pending to egress
TEST_P(HQDownstreamSessionTest, DropConnectionPendingEgress) {
  // NOTE: this test assumes that dropConnection() gets called by the handler
  // before the session has the chance to write data.
  // This is not true anymore when there are control streams
  // So let's just loop a bit to give time to the Downstream Session to send the
  // control stream preface
  flushRequestsAndLoop();

  sendRequest(getGetRequest());
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler, this] {
    handler->sendReplyWithBody(200, 1);
    eventBase_.runInLoop([this] { hqSession_->dropConnection(); }, true);
  });
  handler->expectEOM();
  handler->expectError();
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
}

TEST_P(HQDownstreamSessionTest, TestInfoCallbacks) {
  folly::Optional<HTTPCodec::StreamID> id = sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();
  EXPECT_CALL(infoCb_, onRequestBegin(_)).Times(1);
  EXPECT_CALL(infoCb_, onActivateConnection(_)).Times(1);
  EXPECT_CALL(infoCb_, onIngressMessage(_, _)).Times(1);
  EXPECT_CALL(infoCb_, onRead(_, _, id)).Times(AtLeast(2));
  EXPECT_CALL(infoCb_, onWrite(_, _)).Times(AtLeast(1));
  EXPECT_CALL(infoCb_, onDestroy(_)).Times(1);
  EXPECT_CALL(infoCb_, onRequestEnd(_, _)).Times(1);
  EXPECT_CALL(infoCb_, onDeactivateConnection(_)).Times(1);
  flushRequestsAndLoop();
  hqSession_->dropConnection();
}

TEST_P(HQDownstreamSessionTest, NotifyDropNoStreams) {
  hqSession_->notifyPendingShutdown();
  eventBase_.loop();
}

TEST_P(HQDownstreamSessionTest, ShutdownDropWithUnflushedResp) {
  auto id = sendRequest();
  // should be enough to trick HQSession into serializing the EOM into
  // HQStreamTransport but without enough to send it.
  // HTTP/3 has an extra grease frame on the first transaction
  socketDriver_->setStreamFlowControlWindow(id, 209);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler] {
    handler->sendChunkedReplyWithBody(200, 100, 100, false, true);
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoopN(1);
  hqSession_->dropConnection();
}

// rst_stream while a request is partial, terminate cleanly
TEST_P(HQDownstreamSessionTest, Cancel) {
  auto id = sendRequest(getPostRequest(10), false);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([this, id] {
    socketDriver_->addReadError(id,
                                HTTP3::ErrorCode::HTTP_INTERNAL_ERROR,
                                std::chrono::milliseconds(0));
    hqSession_->closeWhenIdle();
  });
  handler->expectError();
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  EXPECT_EQ(*socketDriver_->streams_[id].error,
            HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED);
}

TEST_P(HQDownstreamSessionTest, EndOfStreamWithPartialFrame) {
  auto id = sendRequest(getPostRequest(10), false);
  auto& request = getStream(id);
  // generate body + EOM, but trim the last byte
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  request.codec->generateBody(
      writeBuf, request.id, makeBuf(10), HTTPCodec::NoPadding, true);
  request.buf.append(writeBuf.split(writeBuf.chainLength() - 1));
  request.readEOF = true;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectError();
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  EXPECT_EQ(*socketDriver_->streams_[kConnectionStreamId].error,
            HTTP3::ErrorCode::HTTP_FRAME_ERROR);
}

// read() returns a LocalErrorCode
TEST_P(HQDownstreamSessionTest, ReadErrorSync) {
  auto id = sendRequest(getPostRequest(10), false);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([this, id] {
    // mark the stream in read error and trigger a readAvailable call
    socketDriver_->setReadError(id);
    // This is just to trigger readAvailable
    socketDriver_->addReadEvent(id, makeBuf(10), milliseconds(0));
    hqSession_->closeWhenIdle();
  });
  handler->expectError();
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
}

// Connection dies in error with an open stream
TEST_P(HQDownstreamSessionTest, TransportErrorWithOpenStream) {
  sendRequest(getPostRequest(10), false);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([this] {
    eventBase_.runInLoop([this] {
      // This should error out the stream first, then destroy the session
      socketDriver_->deliverConnectionError(
          quic::QuicError(quic::TransportErrorCode::PROTOCOL_VIOLATION, ""));
    });
  });
  handler->expectError([](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorConnectionReset);
  });
  handler->expectDetachTransaction();
  EXPECT_CALL(infoCb_, onConnectionError(_)).Times(0);
  flushRequestsAndLoop();
}

// writeChain() returns a LocalErrorCode with a half-closed stream
TEST_P(HQDownstreamSessionTest, WriteError) {
  auto id = sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler, this, id] {
    handler->sendHeaders(200, 100);
    socketDriver_->setWriteError(id);
    hqSession_->closeWhenIdle();
  });
  handler->expectError([](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorWrite);
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
}

// writeChain() returns a LocalErrorCode with stream open both ways
TEST_P(HQDownstreamSessionTest, WriteErrorPartialReq) {
  auto id = sendRequest(getPostRequest(10), false);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler, this, id] {
    handler->sendReplyWithBody(200, 100);
    socketDriver_->setWriteError(id);
    hqSession_->closeWhenIdle();
  });
  handler->expectError();
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
}

// Test write on non writable stream
TEST_P(HQDownstreamSessionTest, WriteNonWritableStream) {
  auto idh = checkRequest();
  // delay the eof event so that we won't have to loop
  flushRequestsAndLoop(false, milliseconds(0), milliseconds(50), [&] {
    // Force the read in the loop, so that this will trigger a write.
    eventBase_.loop();
    socketDriver_->flowControlAccess_.clear();
  });
  // Once the eof is written and no more bytes remain, we should never
  // call flow control methods.
  EXPECT_EQ(socketDriver_->flowControlAccess_.count(idh.first), 0);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, WriteErrorFlowControl) {
  auto id = sendRequest(getPostRequest(10), false);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler, this, id] {
    handler->sendReplyWithBody(200, 100);
    socketDriver_->forceStreamClose(id);
    hqSession_->closeWhenIdle();
  });
  handler->expectError();
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
}

// Connection error on idle connection
TEST_P(HQDownstreamSessionTest, ConnectionErrorIdle) {
  socketDriver_->deliverConnectionError(
      quic::QuicError(quic::TransportErrorCode::PROTOCOL_VIOLATION, ""));
  eventBase_.loopOnce();
}

// Connection End on an idle connection
TEST_P(HQDownstreamSessionTest, ConnectionEnd) {
  nextStreamId();
  socketDriver_->addOnConnectionEndEvent(10);
  CHECK(eventBase_.loop());
}

TEST_P(HQDownstreamSessionTest, BadHttpHeaders) {
  auto id = nextStreamId();
  std::array<uint8_t, 4> badHeaders{0x01, 0x02, 0x00, 0x81};
  auto buf = folly::IOBuf::copyBuffer(badHeaders.data(), badHeaders.size());
  socketDriver_->addReadEvent(id, std::move(buf), milliseconds(0));
  socketDriver_->addReadEOF(id);
  /* T35641532 -- Should QPACK errors be a session errors ?
  testing::StrictMock<MockHTTPHandler> handler;
  EXPECT_CALL(getMockController(), getParseErrorHandler(_, _, _))
      .WillOnce(Return(&handler));
  EXPECT_CALL(handler, setTransaction(testing::_))
      .WillOnce(testing::SaveArg<0>(&handler.txn_));
  handler.expectError();
  handler.expectDetachTransaction();
  */
  flushRequestsAndLoop();
  // The QPACK error will cause the connection to get dropped
}

TEST_P(HQDownstreamSessionTest, BadHttpStrict) {
  hqSession_->setStrictValidation(true);
  sendRequest(getGetRequest("/foo\xff"));
  testing::StrictMock<MockHTTPHandler> handler;
  EXPECT_CALL(getMockController(), getParseErrorHandler(_, _, _))
      .WillOnce(Return(&handler));
  EXPECT_CALL(handler, _setTransaction(testing::_))
      .WillOnce(testing::SaveArg<0>(&handler.txn_));
  handler.expectError([&handler](const HTTPException& ex) {
    EXPECT_TRUE(ex.hasHttpStatusCode());
    handler.sendReplyWithBody(ex.getHttpStatusCode(), 100);
  });
  handler.expectDetachTransaction();

  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, BadHttpHeadersStrict) {
  hqSession_->setStrictValidation(true);
  auto req = getGetRequest("/foo");
  req.getHeaders().set("Foo", "bad value\xff");
  sendRequest(req);
  testing::StrictMock<MockHTTPHandler> handler;
  EXPECT_CALL(getMockController(), getParseErrorHandler(_, _, _))
      .WillOnce(Return(&handler));
  EXPECT_CALL(handler, _setTransaction(testing::_))
      .WillOnce(testing::SaveArg<0>(&handler.txn_));
  handler.expectError([&handler](const HTTPException& ex) {
    EXPECT_TRUE(ex.hasHttpStatusCode());
    handler.sendReplyWithBody(ex.getHttpStatusCode(), 100);
  });
  handler.expectDetachTransaction();

  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, SendFinOnly) {
  HTTPMessage req;
  req.setMethod(HTTPMethod::GET);
  req.setHTTPVersion(0, 9);
  req.setURL("/");
  sendRequest(req);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler] {
    HTTPMessage resp;
    resp.setStatusCode(200);
    resp.setHTTPVersion(0, 9);
    handler->txn_->sendHeaders(resp);
    handler->txn_->sendEOM();
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, PauseResume) {
  auto id = sendRequest(getPostRequest(65547), false);
  auto& request = getStream(id);
  auto handler = addSimpleStrictHandler();
  // Handler pauses as soon as it receives headers.  Nothing buffered so
  // transport continues reading
  handler->expectHeaders([&handler] { handler->txn_->pauseIngress(); });
  flushRequestsAndLoop();
  EXPECT_FALSE(socketDriver_->isStreamPaused(id));

  // Generate some body, but over the limit.  The session (currently) reads
  // everything from the transport, so it will exceed the limit and pause
  request.codec->generateBody(
      request.buf, request.id, makeBuf(65537), HTTPCodec::NoPadding, true);
  flushRequestsAndLoop();
  EXPECT_TRUE(socketDriver_->isStreamPaused(id));
  EXPECT_TRUE(socketDriver_->streams_[id].readBuf.empty());

  // Now send some more data, all buffered
  request.codec->generateBody(
      request.buf, request.id, makeBuf(10), HTTPCodec::NoPadding, true);
  request.readEOF = true;
  flushRequestsAndLoop();
  EXPECT_FALSE(socketDriver_->streams_[id].readBuf.empty());

  auto id2 = sendRequest();
  auto handler2 = addSimpleStrictHandler();
  // stream 2 will start paused at the transport, so even headers are parsed.
  flushRequestsAndLoopN(1);
  EXPECT_FALSE(socketDriver_->streams_[id2].readBuf.empty());
  EXPECT_TRUE(socketDriver_->isStreamPaused(id2));
  hqSession_->closeWhenIdle();

  // After resume, body (2 calls) and EOM delivered
  EXPECT_CALL(*handler, _onBodyWithOffset(_, _)).Times(2);
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();
  handler2->expectHeaders(
      [&handler2] { handler2->sendReplyWithBody(200, 100); });
  handler2->expectEOM();
  handler2->expectDetachTransaction();
  handler->txn_->resumeIngress();
  eventBase_.loop();
}

TEST_P(HQDownstreamSessionTest, EnqueuedAbort) {
  sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler] {
    handler->sendHeaders(200, 100);
    handler->txn_->sendBody(makeBuf(100));
    handler->txn_->sendAbort();
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, TransactionTimeout) {
  sendRequest(getPostRequest(10), false);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler] {
    // fire the timeout as soon as receiving the headers
    handler->txn_->setIdleTimeout(std::chrono::milliseconds(0));
  });
  handler->expectError([&handler](const HTTPException& ex) {
    EXPECT_FALSE(ex.hasHttpStatusCode());
    handler->terminate();
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, ManagedTimeoutUnidirectionalReadReset) {
  std::chrono::milliseconds connIdleTimeout{200};
  auto connManager = wangle::ConnectionManager::makeUnique(
      &eventBase_, connIdleTimeout, nullptr);
  connManager->addConnection(hqSession_, true);
  HQSession::DestructorGuard dg(hqSession_);

  // Just keep sending instructions to set the dynamic table capacity
  std::array<uint8_t, 1> data1{0b00100111};
  auto buf1 = folly::IOBuf::copyBuffer(data1.data(), data1.size());
  socketDriver_->addReadEvent(6, std::move(buf1), milliseconds(0));
  std::array<uint8_t, 1> data2{0b00100110};
  auto buf2 = folly::IOBuf::copyBuffer(data2.data(), data2.size());
  socketDriver_->addReadEvent(6, std::move(buf2), milliseconds(100));
  // Check that the session did not timeout, yet
  eventBase_.runAfterDelay(
      [&] {
        EXPECT_NE(hqSession_->getConnectionCloseReason(),
                  ConnectionCloseReason::TIMEOUT);
      },
      250);

  flushRequestsAndLoop();
}

TEST_P(HQDownstreamSessionTest, ManagedTimeoutActiveStreams) {
  std::chrono::milliseconds connIdleTimeout{300};
  auto connManager = wangle::ConnectionManager::makeUnique(
      &eventBase_, connIdleTimeout, nullptr);
  HQSession::DestructorGuard dg(hqSession_);
  sendRequest(getPostRequest(10), false);
  auto handler = addSimpleStrictHandler();
  connManager->addConnection(hqSession_, true);
  // Txn idle timer is > connIdleTimeout
  auto lastErrorTime = std::chrono::steady_clock::now();
  handler->expectHeaders([&handler] {
    handler->txn_->setIdleTimeout(std::chrono::milliseconds(500));
  });
  handler->expectError(
      [&handler, this, &lastErrorTime](const HTTPException& ex) {
        // we should txn timeout
        EXPECT_FALSE(ex.hasHttpStatusCode());
        EXPECT_EQ(ex.getProxygenError(), kErrorTimeout);
        EXPECT_TRUE(hqSession_->isScheduled());
        hqSession_->cancelTimeout();
        handler->terminate();
        lastErrorTime = std::chrono::steady_clock::now();
      });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  auto now = std::chrono::steady_clock::now();
  EXPECT_GE(
      std::chrono::duration_cast<std::chrono::milliseconds>(now - lastErrorTime)
          .count(),
      connIdleTimeout.count());
  // Connection timeouts in the loop and closes.
  EXPECT_EQ(hqSession_->getConnectionCloseReason(),
            ConnectionCloseReason::TIMEOUT);
}

TEST_P(HQDownstreamSessionTest, ManagedTimeoutNoStreams) {
  std::chrono::milliseconds connIdleTimeout{300};
  auto connManager = wangle::ConnectionManager::makeUnique(
      &eventBase_, connIdleTimeout, nullptr);
  HQSession::DestructorGuard dg(hqSession_);
  connManager->addConnection(hqSession_, true);
  eventBase_.loop();
  EXPECT_EQ(hqSession_->getConnectionCloseReason(),
            ConnectionCloseReason::TIMEOUT);
}

TEST_P(HQDownstreamSessionTest, TransactionTimeoutNoCodecId) {
  auto id = nextStreamId();
  auto res = requests_.emplace(std::piecewise_construct,
                               std::forward_as_tuple(id),
                               std::forward_as_tuple(makeCodec(id)));
  auto req = getGetRequest();
  auto& request = res.first->second;
  request.id = request.codec->createStream();
  request.codec->generateHeader(request.buf, request.id, req, false);
  // Send only a new line, so that onMessageBegin does not get called
  request.buf.split(request.buf.chainLength() - 1);
  testing::StrictMock<MockHTTPHandler> handler;
  expectTransactionTimeout(handler);
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, SendOnFlowControlPaused) {
  // 106 bytes of resp headers, 1 byte of body but 5 bytes of chunk overhead
  auto id = sendRequest();
  // HTTP/3 has an extra grease frame on the first transaction
  socketDriver_->setStreamFlowControlWindow(id, 103);

  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler] {
    handler->sendHeaders(200, 100);
    handler->txn_->sendBody(makeBuf(100));
  });
  handler->expectEgressPaused([&handler] { handler->txn_->sendEOM(); });
  flushRequestsAndLoop();
  socketDriver_->setStreamFlowControlWindow(id, 100);
  handler->expectDetachTransaction();
  eventBase_.loop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, Http_100Continue) {
  auto req = getPostRequest(100);
  req.getHeaders().add(HTTP_HEADER_EXPECT, "100-continue");
  auto id = sendRequest(req, false);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler] {
    HTTPMessage continueResp;
    continueResp.setStatusCode(100);
    handler->txn_->sendHeaders(continueResp);
  });
  flushRequestsAndLoopN(1);
  auto& request = getStream(id);
  request.codec->generateBody(
      request.buf, request.id, makeBuf(100), HTTPCodec::NoPadding, true);
  request.readEOF = true;

  handler->expectBody();
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, ByteEvents) {
  sendRequest();
  auto handler = addSimpleStrictHandler();
  MockHTTPTransactionTransportCallback callback;
  handler->expectHeaders([&handler, &callback] {
    handler->txn_->setTransportCallback(&callback);
  });
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();
  EXPECT_CALL(callback, headerBytesGenerated(_));
  EXPECT_CALL(callback, bodyBytesGenerated(_));
  EXPECT_CALL(callback, firstHeaderByteFlushed());
  EXPECT_CALL(callback, firstByteFlushed());
  EXPECT_CALL(callback, lastByteFlushed());
  EXPECT_CALL(callback, lastByteAcked(_));
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, Observer_Attach_Detach_Destroyed) {
  MockSessionObserver::EventSet eventSet;

  // Test attached/detached callbacks when adding/removing observers
  {
    auto observer = addMockSessionObserver(eventSet);
    EXPECT_CALL(*observer, detached(_));
    hqSession_->removeObserver(observer.get());
  }

  {
    auto observer = addMockSessionObserver(eventSet);
    EXPECT_CALL(*observer, destroyed(_, _));
    hqSession_->dropConnection();
  }
}

TEST_P(HQDownstreamSessionTest, Observer_Attach_Detach_Destroyed_Shared) {
  MockSessionObserver::EventSet eventSet;

  // Test attached/detached callbacks when adding/removing observers
  {
    auto observer = addMockSessionObserverShared(eventSet);
    EXPECT_CALL(*observer, detached(_));
    hqSession_->removeObserver(observer.get());
  }

  {
    auto observer = addMockSessionObserverShared(eventSet);
    EXPECT_CALL(*observer, destroyed(_, _));
    hqSession_->dropConnection();
  }
}

TEST_P(HQDownstreamSessionTest, Observer_RequestStarted) {

  // Add an observer NOT subscribed to the RequestStarted event
  auto observerUnsubscribed =
      addMockSessionObserver(MockSessionObserver::EventSetBuilder().build());
  hqSession_->addObserver(observerUnsubscribed.get());

  // Add an observer subscribed to this event
  auto observerSubscribed = addMockSessionObserver(
      MockSessionObserver::EventSetBuilder()
          .enable(HTTPSessionObserverInterface::Events::RequestStarted)
          .build());
  hqSession_->addObserver(observerSubscribed.get());

  EXPECT_CALL(*observerUnsubscribed, requestStarted(_, _)).Times(0);

  // Add a request started event to the observer
  EXPECT_CALL(*observerSubscribed, requestStarted(_, _))
      .WillOnce(Invoke(
          [](HTTPSessionObserverAccessor*,
             const proxygen::HTTPSessionObserverInterface::RequestStartedEvent&
                 event) {
            auto hdrs = event.requestHeaders;
            EXPECT_EQ(hdrs.getSingleOrEmpty("x-meta-test-header"), "abc123");
          }));
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler]() {
    handler->sendReplyWithBody(200 /* status code */,
                               100 /* content size */,
                               true /* keepalive */,
                               true /* sendEOM */,
                               false /*trailers*/);
  });
  handler->expectDetachTransaction();
  HTTPSession::DestructorGuard g(hqSession_);
  HTTPMessage req = getGetRequest();
  req.getHeaders().add("x-meta-test-header", "abc123");
  sendRequest(req);

  flushRequestsAndLoop(true, milliseconds(0));
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, AppRateLimited) {
  sendRequest();
  auto handler = addSimpleStrictHandler();
  MockHTTPTransactionTransportCallback callback;
  handler->expectHeaders([&handler, &callback] {
    handler->txn_->setTransportCallback(&callback);
  });
  handler->expectEOM([this, &handler] {
    handler->sendHeaders(200, 150);
    handler->txn_->sendBody(makeBuf(100));
    // force trigger onAppRateLimited
    hqSession_->onAppRateLimited();
  });
  EXPECT_CALL(callback, headerBytesGenerated(_));
  EXPECT_CALL(callback, bodyBytesGenerated(Ge(100))); // For HQ it's 100
  EXPECT_CALL(callback, firstHeaderByteFlushed());
  EXPECT_CALL(callback, firstByteFlushed());
  EXPECT_CALL(callback, transportAppRateLimited());
  flushRequestsAndLoop();

  // send some more bytes and force trigger onAppRateLimited
  EXPECT_CALL(callback, bodyBytesGenerated(Ge(50))); // For HQ it's 52
  EXPECT_CALL(callback, transportAppRateLimited());
  handler->txn_->sendBody(makeBuf(50));
  hqSession_->onAppRateLimited();
  flushRequestsAndLoop();

  // Send the EOM, txn should not detach yet
  EXPECT_CALL(callback, bodyBytesGenerated(0));
  EXPECT_CALL(callback, lastByteFlushed());
  handler->txn_->sendEOM(); // 0 length EOM
  flushRequestsAndLoopN(1);

  // Let the delivery callback fire, now it can cleanup
  EXPECT_CALL(callback, lastByteAcked(_));
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, LastByteEventZeroSize) {
  sendRequest();
  auto handler = addSimpleStrictHandler();
  MockHTTPTransactionTransportCallback callback;
  handler->expectHeaders([&handler, &callback] {
    handler->txn_->setTransportCallback(&callback);
  });
  handler->expectEOM([&handler] {
    handler->sendHeaders(200, 100);
    handler->txn_->sendBody(makeBuf(100));
  });
  EXPECT_CALL(callback, headerBytesGenerated(_));
  EXPECT_CALL(callback, bodyBytesGenerated(Ge(100))); // For HQ it's 103
  EXPECT_CALL(callback, firstHeaderByteFlushed());
  EXPECT_CALL(callback, firstByteFlushed());
  flushRequestsAndLoop();

  // Send the EOM, txn should not detach yet
  EXPECT_CALL(callback, bodyBytesGenerated(0));
  EXPECT_CALL(callback, lastByteFlushed());
  handler->txn_->sendEOM(); // 0 length EOM
  flushRequestsAndLoopN(1);

  // Let the delivery callback fire, now it can cleanup
  EXPECT_CALL(callback, lastByteAcked(_));
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, DropWithByteEvents) {
  sendRequest();
  auto handler = addSimpleStrictHandler();
  MockHTTPTransactionTransportCallback callback;
  handler->expectHeaders([&handler, &callback] {
    handler->txn_->setTransportCallback(&callback);
  });
  handler->expectEOM([&handler] { handler->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();
  EXPECT_CALL(callback, headerBytesGenerated(_));
  EXPECT_CALL(callback, bodyBytesGenerated(_));
  EXPECT_CALL(callback, firstHeaderByteFlushed());
  EXPECT_CALL(callback, firstByteFlushed());
  EXPECT_CALL(callback, lastByteFlushed());
  flushRequestsAndLoopN(1);
  hqSession_->dropConnection();
}

TEST_P(HQDownstreamSessionTest, TransportInfo) {
  wangle::TransportInfo transInfo;
  quic::QuicSocket::TransportInfo quicInfo;
  quicInfo.srtt = std::chrono::microseconds(135);
  quicInfo.rttvar = std::chrono::microseconds(246);
  quicInfo.writableBytes = 212;
  quicInfo.congestionWindow = 5 * quic::kDefaultUDPSendPacketLen;
  quicInfo.packetsRetransmitted = 513;
  quicInfo.timeoutBasedLoss = 90;
  quicInfo.pto = std::chrono::microseconds(34);
  quicInfo.bytesSent = 23;
  quicInfo.bytesRecvd = 123;
  quicInfo.ptoCount = 1;
  quicInfo.totalPTOCount = 2;
  EXPECT_CALL(*socketDriver_->getSocket(), getTransportInfo())
      .Times(3)
      .WillRepeatedly(Return(quicInfo));
  hqSession_->getCurrentTransportInfoWithoutUpdate(&transInfo);
  EXPECT_EQ(135, transInfo.rtt.count());
  EXPECT_EQ(246, transInfo.rtt_var);
  EXPECT_EQ(5, transInfo.cwnd);
  EXPECT_EQ(5 * quic::kDefaultUDPSendPacketLen, transInfo.cwndBytes);
  EXPECT_EQ(513, transInfo.rtx);
  EXPECT_EQ(90, transInfo.rtx_tm);
  EXPECT_EQ(34, transInfo.rto);
  EXPECT_EQ(23, transInfo.totalBytes);
  auto quicProtocolInfo =
      dynamic_cast<QuicProtocolInfo*>(transInfo.protocolInfo.get());
  EXPECT_EQ(0, quicProtocolInfo->ptoCount);
  EXPECT_EQ(0, quicProtocolInfo->totalPTOCount);
  EXPECT_EQ(0, quicProtocolInfo->totalTransportBytesSent);
  EXPECT_EQ(0, quicProtocolInfo->totalTransportBytesRecvd);
  hqSession_->getCurrentTransportInfo(&transInfo);
  EXPECT_EQ(1, quicProtocolInfo->ptoCount);
  EXPECT_EQ(2, quicProtocolInfo->totalPTOCount);
  EXPECT_EQ(23, quicProtocolInfo->totalTransportBytesSent);
  EXPECT_EQ(123, quicProtocolInfo->totalTransportBytesRecvd);
  hqSession_->dropConnection();
}

// Current Transport Info tests
TEST_P(HQDownstreamSessionTest, CurrentTransportInfo) {
  sendRequest();
  auto handler = addSimpleStrictHandler();
  MockHTTPTransactionTransportCallback callback;
  handler->expectHeaders([&handler, &callback] {
    handler->txn_->setTransportCallback(&callback);
  });

  QuicStreamProtocolInfo resultProtocolInfo;
  handler->expectEOM([&handler, &resultProtocolInfo] {
    wangle::TransportInfo transInfo;
    handler->txn_->getCurrentTransportInfo(&transInfo);
    auto quicStreamProtocolInfo =
        dynamic_cast<QuicStreamProtocolInfo*>(transInfo.protocolInfo.get());

    if (quicStreamProtocolInfo) {
      resultProtocolInfo.streamTransportInfo =
          quicStreamProtocolInfo->streamTransportInfo;
    }
  });

  handler->expectDetachTransaction();
  handler->expectError([&](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorDropped);
  });

  flushRequestsAndLoop();
  hqSession_->dropConnection();

  // The stream transport info field should be equal to
  // the mock object
  EXPECT_EQ(resultProtocolInfo.streamTransportInfo.totalHeadOfLineBlockedTime,
            streamTransInfo_.totalHeadOfLineBlockedTime);
  EXPECT_EQ(resultProtocolInfo.streamTransportInfo.holbCount,
            streamTransInfo_.holbCount);
  EXPECT_EQ(resultProtocolInfo.streamTransportInfo.isHolb,
            streamTransInfo_.isHolb);
}

TEST_P(HQDownstreamSessionTest, GetAddresses) {
  EXPECT_EQ(socketDriver_->localAddress_, hqSession_->getLocalAddress());
  EXPECT_EQ(socketDriver_->peerAddress_, hqSession_->getPeerAddress());
  hqSession_->dropConnection();
}

TEST_P(HQDownstreamSessionTest, GetAddressesFromBase) {
  HTTPSessionBase* sessionBase = dynamic_cast<HTTPSessionBase*>(hqSession_);
  EXPECT_EQ(socketDriver_->localAddress_, sessionBase->getLocalAddress());
  EXPECT_EQ(socketDriver_->peerAddress_, sessionBase->getPeerAddress());
  hqSession_->dropConnection();
}

TEST_P(HQDownstreamSessionTest, GetAddressesAfterDropConnection) {
  HQSession::DestructorGuard dg(hqSession_);
  hqSession_->dropConnection();
  EXPECT_EQ(socketDriver_->localAddress_, hqSession_->getLocalAddress());
  EXPECT_EQ(socketDriver_->peerAddress_, hqSession_->getPeerAddress());
}

TEST_P(HQDownstreamSessionTest, RstCancelled) {
  auto id = nextStreamId();
  auto buf = folly::IOBuf::create(3);
  memcpy(buf->writableData(), "GET", 3);
  buf->append(3);
  socketDriver_->addReadEvent(id, std::move(buf), milliseconds(0));
  flushRequestsAndLoopN(1);
  socketDriver_->addReadError(id,
                              HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED,
                              std::chrono::milliseconds(0));
  hqSession_->closeWhenIdle();
  flushRequestsAndLoop();
  EXPECT_EQ(*socketDriver_->streams_[id].error,
            HTTP3::ErrorCode::HTTP_REQUEST_REJECTED);
}

TEST_P(HQDownstreamSessionTest, LocalErrQueuedEgress) {
  sendRequest(getPostRequest(10), false);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&handler, this] {
    socketDriver_->setStreamFlowControlWindow(0, 0);
    socketDriver_->setConnectionFlowControlWindow(0);
    handler->sendHeaders(200, 65536 * 2);
    handler->sendBody(65536 * 2);
  });
  handler->expectEgressPaused();
  flushRequestsAndLoopN(2);
  handler->expectError([](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorShutdown);
  });
  handler->expectDetachTransaction();
  socketDriver_->deliverConnectionError(
      quic::QuicError(quic::LocalErrorCode::CONNECTION_RESET, ""));
  flushRequestsAndLoop();
}

TEST_P(HQDownstreamSessionTest, NoErrorNoStreams) {
  EXPECT_CALL(infoCb_, onIngressError(_, kErrorNone));
  socketDriver_->deliverConnectionError(
      quic::QuicError(HTTP3::ErrorCode::HTTP_NO_ERROR, ""));
  flushRequestsAndLoop();
}

TEST_P(HQDownstreamSessionTest, NoErrorOneStreams) {
  sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM();
  flushRequestsAndLoopN(1);
  // stream gets an error
  handler->expectError([](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorEOF);
  });
  handler->expectDetachTransaction();
  // This is for connection level errors, maybe should be reported a
  EXPECT_CALL(infoCb_, onIngressError(_, kErrorEOF));
  socketDriver_->deliverConnectionError(
      quic::QuicError(HTTP3::ErrorCode::HTTP_NO_ERROR, ""));
}

TEST_P(HQDownstreamSessionTest, Connect) {
  auto handler = addSimpleStrictHandler();
  // Send HTTP 200 OK to accept the CONNECT request
  handler->expectHeaders([&handler] { handler->sendHeaders(200, 100); });
  handler->expectEOM([&] { handler->terminate(); });

  // Data should be received using onBody
  EXPECT_CALL(*handler, _onBodyWithOffset(_, _))
      .WillOnce(ExpectString("12345"))
      .WillOnce(ExpectString("abcdefg"));
  handler->expectDetachTransaction();

  proxygen::HTTPMessage req;
  req.setURL("test.net/path");
  req.setMethod("CONNECT");
  req.getHeaders().add(proxygen::HTTP_HEADER_HOST, "https://test.net/path");
  auto id = sendRequest(req, /* eom */ false);
  auto& request = getStream(id);

  auto buf1 = folly::IOBuf::copyBuffer("12345");
  request.codec->generateBody(
      request.buf, request.id, std::move(buf1), HTTPCodec::NoPadding, true);
  flushRequestsAndLoopN(1);

  auto buf2 = folly::IOBuf::copyBuffer("abcdefg");
  request.codec->generateBody(
      request.buf, request.id, std::move(buf2), HTTPCodec::NoPadding, true);
  flushRequestsAndLoopN(1);

  request.readEOF = true;
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, ConnectUDP) {
  auto handler = addSimpleStrictHandler();
  // Send HTTP 200 OK to accept the CONNECT request
  handler->expectHeaders([&handler] { handler->sendHeaders(200, 100); });
  handler->expectEOM([&] { handler->terminate(); });

  // Data should be received using onBody
  EXPECT_CALL(*handler, _onBodyWithOffset(_, _))
      .WillOnce(ExpectString("12345"))
      .WillOnce(ExpectString("abcdefg"));
  handler->expectDetachTransaction();

  proxygen::HTTPMessage req;
  req.setURL("test.net/path");
  req.setMethod("CONNECT-UDP");
  req.getHeaders().add(proxygen::HTTP_HEADER_HOST, "https://test.net/path");
  auto id = sendRequest(req, /* eom */ false);
  auto& request = getStream(id);

  auto buf1 = folly::IOBuf::copyBuffer("12345");
  request.codec->generateBody(
      request.buf, request.id, std::move(buf1), HTTPCodec::NoPadding, true);
  flushRequestsAndLoopN(1);

  auto buf2 = folly::IOBuf::copyBuffer("abcdefg");
  request.codec->generateBody(
      request.buf, request.id, std::move(buf2), HTTPCodec::NoPadding, true);
  flushRequestsAndLoopN(1);

  request.readEOF = true;
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

// Just open a stream and send nothing
TEST_P(HQDownstreamSessionTest, zeroBytes) {
  auto id = nextStreamId();
  socketDriver_->addReadEvent(
      id, folly::IOBuf::copyBuffer("", 0), milliseconds(0));
  testing::StrictMock<MockHTTPHandler> handler;
  expectTransactionTimeout(handler);
  eventBase_.loop();
  hqSession_->closeWhenIdle();
}

// For HQ, send an incomplete frame header
TEST_P(HQDownstreamSessionTest, oneByte) {
  auto id = nextStreamId();
  socketDriver_->addReadEvent(
      id, folly::IOBuf::copyBuffer("", 1), milliseconds(0));
  testing::StrictMock<MockHTTPHandler> handler;
  expectTransactionTimeout(handler);
  eventBase_.loop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, TestGoawayID) {
  // This test check that unidirectional stream IDs are not accounted for
  // in the Goaway Max Stream ID
  auto req = getGetRequest();
  // Explicitly skip some stream IDs to simulate out of order delivery
  sendRequest(req, true, 4);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([hdlr = handler.get()] {
    // Delay sending EOM so the streams are active when draining
    hdlr->sendReplyWithBody(200, 100, true, false);
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoopN(1);
  hqSession_->closeWhenIdle();
  // Give it some time to send the two goaways and receive the delivery callback
  flushRequestsAndLoopN(3);
  EXPECT_EQ(httpCallbacks_.goaways, 2);
  EXPECT_THAT(httpCallbacks_.goawayStreamIds,
              ElementsAre(kMaxClientBidiStreamId, 8));
  handler->sendEOM();
  flushRequestsAndLoop();
}

TEST_P(HQDownstreamSessionTest, TestReceiveGoaway) {
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  egressControlCodec_->generateGoaway(
      writeBuf, HTTPCodec::MaxStreamID, ErrorCode::NO_ERROR, nullptr);
  socketDriver_->addReadEvent(
      connControlStreamId_, writeBuf.move(), std::chrono::milliseconds(0));
  flushRequestsAndLoopN(1);
  EXPECT_FALSE(hqSession_->isClosing());
  hqSession_->closeWhenIdle();
  eventBase_.loop();
}

TEST_P(HQDownstreamSessionTest, TestGetGoaway) {
  std::vector<std::unique_ptr<StrictMock<MockHTTPHandler>>> handlers;
  auto numStreams = 3;
  for (auto n = 1; n <= numStreams; n++) {
    auto req = getGetRequest();
    // Explicitly skip some stream IDs to simulate out of order delivery
    sendRequest(req, true, n * 8);
    handlers.emplace_back(addSimpleStrictHandler());
    auto handler = handlers.back().get();
    handler->expectHeaders();
    handler->expectEOM([hdlr = handler] {
      // Delay sending EOM so the streams are active when draining
      hdlr->sendReplyWithBody(200, 100, true, false);
    });
    handler->expectDetachTransaction();
  }
  flushRequestsAndLoopN(1);
  hqSession_->closeWhenIdle();
  // Give it some time to send the two goaways and receive the delivery callback
  flushRequestsAndLoopN(3);
  EXPECT_EQ(httpCallbacks_.goaways, 2);
  EXPECT_THAT(httpCallbacks_.goawayStreamIds,
              ElementsAre(kMaxClientBidiStreamId, numStreams * 8 + 4));

  // Check that a new stream with id >= lastStreamId gets rejected
  auto errReq = getGetRequest();
  quic::StreamId errStreamId = numStreams * 8 + 4;
  sendRequest(errReq, true, errStreamId);
  flushRequestsAndLoopN(1);
  auto& errStream = socketDriver_->streams_[errStreamId];
  EXPECT_EQ(errStream.writeState, MockQuicSocketDriver::StateEnum::ERROR);
  EXPECT_TRUE(errStream.error == HTTP3::ErrorCode::HTTP_REQUEST_REJECTED);

  // Check that a new stream with id <= lastStreamId is instead just fine
  auto okReq = getGetRequest();
  sendRequest(okReq, true, numStreams * 8 - 4);
  auto okHandler = addSimpleStrictHandler();
  okHandler->expectHeaders();
  okHandler->expectEOM([&] { okHandler->sendReplyWithBody(200, 100); });
  okHandler->expectDetachTransaction();
  flushRequestsAndLoopN(1);

  // now send response EOM on the pending transactions, to finish shutdown
  for (auto& handler : handlers) {
    handler->sendEOM();
  }
  flushRequestsAndLoop();
}

TEST_P(HQDownstreamSessionTest, DelayedQPACK) {
  auto req = getGetRequest();
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequest(req);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM(
      [hdlr = handler.get()] { hdlr->sendReplyWithBody(200, 100); });
  handler->expectDetachTransaction();

  auto controlStream = encoderWriteBuf_.move();
  flushRequestsAndLoopN(1);
  encoderWriteBuf_.append(std::move(controlStream));
  flushRequestsAndLoop();
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, cancelQPACK) {
  auto req = getGetRequest();
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequest(req);
  auto& request = getStream(id);
  // discard part of request, header won't get qpack-ack'd
  request.buf.trimEnd(request.buf.chainLength() - 3);
  request.readEOF = false;
  flushRequestsAndLoopN(1);
  socketDriver_->addReadError(id,
                              HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED,
                              std::chrono::milliseconds(0));
  hqSession_->closeWhenIdle();
  flushRequestsAndLoop();
  // this will evict all headers, which is only legal if the cancellation is
  // emitted and processed.
  qpackCodec_.setEncoderHeaderTableSize(0);
  EXPECT_EQ(*socketDriver_->streams_[id].error,
            HTTP3::ErrorCode::HTTP_REQUEST_REJECTED);
  eventBase_.loopOnce();
}

TEST_P(HQDownstreamSessionTest, DelayedQPACKCanceled) {
  auto req = getGetRequest();
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequest(req);
  // This request never gets a handler

  auto controlStream = encoderWriteBuf_.move();
  // receive header block with unsatisfied dep
  flushRequestsAndLoopN(1);

  // cancel this request
  socketDriver_->addReadError(id,
                              HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED,
                              std::chrono::milliseconds(0));
  flushRequestsAndLoopN(1);

  // Now send the dependency
  encoderWriteBuf_.append(std::move(controlStream));
  flushRequestsAndLoop();

  // This used to crash
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, DelayedQPACKTimeout) {
  auto req = getPostRequest(10);
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequest(req, false);
  auto& request = getStream(id);
  folly::IOBufQueue reqTail(folly::IOBufQueue::cacheChainLength());
  reqTail.append(request.buf.move());
  request.buf.append(reqTail.split(reqTail.chainLength() / 2));
  // reqTail now has the second half of request

  flushRequests();
  testing::StrictMock<MockHTTPHandler> handler;
  expectTransactionTimeout(handler, [&] {
    request.buf.append(reqTail.move());
    auto body = folly::IOBuf::wrapBuffer("\3\3\3\3\3\3\3\3\3\3", 10);
    request.codec->generateBody(
        request.buf, request.id, std::move(body), HTTPCodec::NoPadding, true);
    flushRequests();
  });
  eventBase_.loop();
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, QPACKEncoderLimited) {
  auto req = getGetRequest();
  socketDriver_->getSocket()->setStreamFlowControlWindow(
      kQPACKEncoderEgressStreamId, 10);
  auto id = sendRequest(req);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([hdlr = handler.get()] {
    HTTPMessage resp;
    resp.setStatusCode(200);
    resp.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
    hdlr->txn_->sendHeaders(resp);
    hdlr->txn_->sendEOM();
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();

  // QPACK will attempt to index the header, but cannot reference it because
  // it runs out of stream flow control
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 30);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, DelayedQPACKStopSendingReset) {
  auto req = getGetRequest();
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequest(req);
  // This request never gets a handler

  auto controlStream = encoderWriteBuf_.move();
  // receive header block with unsatisfied dep
  flushRequestsAndLoopN(1);

  // cancel this request
  socketDriver_->addStopSending(id, HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED);
  socketDriver_->addReadError(id,
                              HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED,
                              std::chrono::milliseconds(0));
  flushRequestsAndLoopN(1);

  // Now send the dependency
  encoderWriteBuf_.append(std::move(controlStream));
  flushRequestsAndLoop();

  // This used to crash
  hqSession_->closeWhenIdle();
}

using ControlStreamsStallTest = HQDownstreamSessionBeforeTransportReadyTest;

TEST_P(ControlStreamsStallTest, StalledQpackStream) {
  // This test does not pre-setup any control streams for a reason.
  // We want to be able to control unidirectional (QPACK) stream lifetime to
  // perform the test.

  QuicConnectionStateBase state(quic::QuicNodeType::Server);
  EXPECT_CALL(*(socketDriver_->sock_), getState())
      .WillRepeatedly(testing::Invoke(
          [&state]() -> QuicConnectionStateBase* { return &state; }));

  // Get a request going.
  auto req = getGetRequest();
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequest(req);

  // Add data to the control stream 6 with fake stream offset 42.
  // This is to ensure unidirectional stream dispatcher in session does not
  // instantiate an actual full blown control stream object, but only create a
  // pending tmp stream placeholder and attach a peek callback to it to wait for
  // the preface.
  std::array<uint8_t, 1> data1{0b00100111};
  auto buf1 = folly::IOBuf::copyBuffer(data1.data(), data1.size());
  socketDriver_->addReadEvent(6, std::move(buf1), milliseconds(0), 42);

  // Let the sesion roll.
  hqSession_->onTransportReady();

  // Now cancel the request we queded up in the beginning.
  socketDriver_->addStopSending(id, HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED);
  socketDriver_->addReadError(id,
                              HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED,
                              std::chrono::milliseconds(0));

  // Roll the event base. Once session loops, the expectation is that the
  // pending control stream is alive and well and has its peek callback
  // attached.
  flushRequestsAndLoopN(1);

  EXPECT_NE(socketDriver_->streams_[6].peekCB, nullptr);

  // Close the session; all good.
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, QPACKHeadersTooLarge) {
  hqSession_->setEgressSettings({{SettingsId::MAX_HEADER_LIST_SIZE, 60}});
  auto req = getGetRequest();
  req.getHeaders().add("X-FB-Debug", "rfccffgvtvnenjkbtitkfdufddnvbecu");
  auto id = sendRequest(req);
  testing::StrictMock<MockHTTPHandler> errHandler;
  EXPECT_CALL(getMockController(), getParseErrorHandler(_, _, _))
      .WillOnce(Return(&errHandler));
  EXPECT_CALL(errHandler, _setTransaction(testing::_))
      .WillOnce(testing::SaveArg<0>(&errHandler.txn_));
  errHandler.expectError([&errHandler](const HTTPException& ex) {
    EXPECT_EQ(ex.getHttp3ErrorCode(),
              (uint32_t)HTTP3::ErrorCode::HTTP_QPACK_DECOMPRESSION_FAILED);
    errHandler.sendReplyWithBody(400, 100);
  });
  errHandler.expectDetachTransaction();
  flushRequestsAndLoopN(2);
  // Gets a response
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 30);
  EXPECT_EQ(*socketDriver_->streams_[id].error,
            (uint32_t)HTTP3::ErrorCode::HTTP_QPACK_DECOMPRESSION_FAILED);
  auto decoderStream = socketDriver_->streams_[kQPACKDecoderEgressStreamId]
                           .writeBuf.front()
                           ->clone();
  decoderStream->coalesce();
  // preface, cancel, ici
  EXPECT_EQ(decoderStream->computeChainDataLength(), 3);
  EXPECT_EQ(decoderStream->data()[1], 0x40); // stream 0 cancelled

  // But the conn is still usable
  sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([hdlr = handler.get(), this] {
    HTTPMessage resp;
    resp.setStatusCode(200);
    hdlr->txn_->sendHeaders(resp);
    hdlr->txn_->sendEOM();
    hqSession_->closeWhenIdle();
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
}

TEST_P(HQDownstreamSessionBeforeTransportReadyTest, NotifyPendingShutdown) {
  hqSession_->notifyPendingShutdown();
  SetUpOnTransportReady();
  // Give it some time to send the two goaways and receive the delivery callback
  flushRequestsAndLoopN(3);
  // There is a check for this already for all the tests, but adding this to
  // make it explicit that SETTINGS should be sent before GOAWAY even in this
  // corner case, otherwise the peer will error out the session
  EXPECT_EQ(httpCallbacks_.settings, 1);
  EXPECT_EQ(httpCallbacks_.goaways, 2);
  EXPECT_THAT(httpCallbacks_.goawayStreamIds,
              ElementsAre(kMaxClientBidiStreamId, 0));
}

// NOTE: a failure for this test may cause an infinite loop in processReadData
TEST_P(HQDownstreamSessionTest, ProcessReadDataOnDetachedStream) {
  auto id = sendRequest("/", 0, false);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders([&] {
    eventBase_.runAfterDelay(
        [&] {
          // schedule a few events to run in the eventbase back-to-back
          // call readAvailable with just the EOF
          auto& stream = socketDriver_->streams_[id];
          CHECK(!stream.readEOF);
          stream.readEOF = true;
          CHECK(stream.readCB);
          stream.readCB->readAvailable(id);
          // now send an error so that the stream gets marked for detach
          stream.readCB->readError(
              id, quic::QuicError(HTTP3::ErrorCode::HTTP_NO_ERROR));
          // then closeWhenIdle (like during shutdown), this calls
          // checkForShutdown that calls checkForDetach and may detach a
          // transaction that was added to the pendingProcessReadSet in the same
          // loop
          hqSession_->closeWhenIdle();
        },
        10);
  });
  flushRequestsAndLoopN(1);

  handler->expectError();
  handler->expectDetachTransaction();

  flushRequestsAndLoop();
}

// Test Cases for which Settings are not sent in the test SetUp
using HQDownstreamSessionTestNoSettings = HQDownstreamSessionTest;
INSTANTIATE_TEST_SUITE_P(HQDownstreamSessionTest,
                         HQDownstreamSessionTestNoSettings,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           tp.shouldSendSettings_ = false;
                           return tp;
                         }()),
                         paramsToTestName);
TEST_P(HQDownstreamSessionTestNoSettings, SimpleGet) {
  auto idh = checkRequest();
  flushRequestsAndLoop();
  EXPECT_GT(socketDriver_->streams_[idh.first].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[idh.first].writeEOF);
  // Checks that the server response is sent without the QPACK dynamic table
  CHECK_EQ(qpackCodec_.getCompressionInfo().ingress.headerTableSize_, 0);

  // TODO: Check that QPACK does not use the dynamic table for the response
  hqSession_->closeWhenIdle();
}

// This test is checking two different scenarios for different protocol
//   - in HQ we already have sent SETTINGS in SetUp, so tests that multiple
//     setting frames are not allowed
TEST_P(HQDownstreamSessionTest, ExtraSettings) {
  sendRequest();
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM();
  handler->expectError([&](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorConnection);
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoopN(1);

  // Need to use a new codec. Since generating settings twice is
  // forbidden
  HQControlCodec auxControlCodec_{0x0003,
                                  TransportDirection::UPSTREAM,
                                  StreamDirection::EGRESS,
                                  egressSettings_};
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  auxControlCodec_.generateSettings(writeBuf);
  socketDriver_->addReadEvent(
      connControlStreamId_, writeBuf.move(), milliseconds(0));

  flushRequestsAndLoop();

  EXPECT_EQ(*socketDriver_->streams_[kConnectionStreamId].error,
            HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED);
}

using HQDownstreamSessionFilterTestHQ = HQDownstreamSessionTest;
TEST_P(HQDownstreamSessionFilterTestHQ, ControlStreamFilters) {
  uint64_t settingsReceived = 0;

  class TestFilter : public PassThroughHTTPCodecFilter {
   public:
    explicit TestFilter(uint64_t& settingsReceived)
        : settingsReceived_(settingsReceived) {
    }

    void onSettings(const SettingsList& settings) override {
      settingsReceived_++;
    }
    uint64_t& settingsReceived_;
  };

  hqSession_->addCodecFilter<TestFilter>(settingsReceived);
  sendSettings();
  flushRequestsAndLoop();
  EXPECT_EQ(settingsReceived, 1);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, onErrorEmptyEnqueued) {
  folly::IOBufQueue rst{folly::IOBufQueue::cacheChainLength()};
  auto id1 = sendRequest();

  InSequence handlerSequence;
  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1, this, id1] {
    handler1->sendHeaders(200, 100);
    socketDriver_->setStreamFlowControlWindow(id1, 100);
    // After one loop, it will become stream flow control blocked, and txn
    // will think it is enqueued, but session will not.
    handler1->expectEgressPaused();
    handler1->sendBody(101);
    eventBase_.runInLoop([&handler1, this, id1] {
      handler1->expectError();
      handler1->expectDetachTransaction();
      socketDriver_->addReadError(id1,
                                  HTTP3::ErrorCode::HTTP_INTERNAL_ERROR,
                                  std::chrono::milliseconds(0));
    });
  });
  flushRequestsAndLoop();

  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, dropWhilePaused) {
  folly::IOBufQueue rst{folly::IOBufQueue::cacheChainLength()};
  sendRequest();

  InSequence handlerSequence;
  auto handler1 = addSimpleStrictHandler();
  handler1->expectHeaders();
  handler1->expectEOM([&handler1, this] {
    // pause writes
    socketDriver_->setConnectionFlowControlWindow(0);
    // fill session buffer
    handler1->sendReplyWithBody(200, hqSession_->getWriteBufferLimit());
  });
  flushRequestsAndLoop();

  handler1->expectError([&](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorDropped);
  });
  handler1->expectDetachTransaction();
  hqSession_->dropConnection();
}

TEST_P(HQDownstreamSessionTest, StopSendingOnUnknownUnidirectionalStreams) {
  auto greaseStreamId = nextUnidirectionalStreamId();
  createControlStream(socketDriver_.get(),
                      greaseStreamId,
                      proxygen::hq::UnidirectionalStreamType(
                          *getGreaseId(folly::Random::rand32(16))));
  auto idh = checkRequest();
  flushRequestsAndLoop();

  EXPECT_EQ(*socketDriver_->streams_[greaseStreamId].error,
            HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR);
  // Also check that the request completes correctly
  EXPECT_GT(socketDriver_->streams_[idh.first].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[idh.first].writeEOF);
  // Checks that the server response is sent using the QPACK dynamic table
  CHECK_GE(qpackCodec_.getCompressionInfo().ingress.headerTableSize_, 0);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, DataOnUnknownControlStream) {
  auto randPreface =
      hq::UnidirectionalStreamType(*getGreaseId(folly::Random::rand32(16)));
  // Create  unidirectional stream with an unknown stream preface
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  generateStreamPreface(writeBuf, randPreface);
  socketDriver_->addReadEvent(14, writeBuf.move());
  flushRequestsAndLoop();

  // Send an extra varint on the same stream, ignoring STOP_SENDING
  folly::IOBufQueue writeBuf2{folly::IOBufQueue::cacheChainLength()};
  generateStreamPreface(writeBuf2, randPreface);
  socketDriver_->addReadEvent(14, writeBuf.move());
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, eofControlStream) {
  sendRequest();

  InSequence handlerSequence;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM();
  handler->expectError([&](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorConnection);
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoopN(1);
  socketDriver_->addReadEOF(connControlStreamId_);
  flushRequestsAndLoop();
}

TEST_P(HQDownstreamSessionTest, resetControlStream) {
  sendRequest();

  InSequence handlerSequence;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM();
  handler->expectError([&](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorConnection);
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoopN(1);
  socketDriver_->addReadError(connControlStreamId_,
                              HTTP3::ErrorCode::HTTP_INTERNAL_ERROR);
  flushRequestsAndLoop();
  EXPECT_EQ(*socketDriver_->streams_[kConnectionStreamId].error,
            HTTP3::ErrorCode::HTTP_CLOSED_CRITICAL_STREAM);
}

TEST_P(HQDownstreamSessionTest, controlStreamWriteError) {
  sendRequest();

  InSequence handlerSequence;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&handler] { handler->sendHeaders(200, 100); });
  handler->expectError([&](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorWrite);
  });
  handler->expectDetachTransaction();
  socketDriver_->setWriteError(kQPACKEncoderEgressStreamId);
  flushRequestsAndLoop();
  EXPECT_EQ(*socketDriver_->streams_[kConnectionStreamId].error,
            HTTP3::ErrorCode::HTTP_CLOSED_CRITICAL_STREAM);
}

TEST_P(HQDownstreamSessionTest, TooManyControlStreams) {
  // This creates a request stream, so that we can check the HTTP3::ErrorCode
  // at the end of the test. With no active streams we would drop the
  // connection with no error instead.
  sendRequest();
  InSequence handlerSequence;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM();
  handler->expectError([&](const HTTPException& ex) {
    EXPECT_EQ(ex.getProxygenError(), kErrorConnection);
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoopN(1);

  // Create an extra control stream, that causes the connection to get dropped
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  generateStreamPreface(writeBuf, UnidirectionalStreamType::CONTROL);
  socketDriver_->addReadEvent(14, writeBuf.move());

  flushRequestsAndLoop();
  EXPECT_EQ(*socketDriver_->streams_[kConnectionStreamId].error,
            HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR);
}

TEST_P(HQDownstreamSessionTest, TestGreaseFramePerSession) {
  // a grease frame will be created in the first bidir stream
  auto idh1 = checkRequest();
  flushRequestsAndLoop();
  EXPECT_GT(socketDriver_->streams_[idh1.first].writeBuf.chainLength(), 110);
  FakeHTTPCodecCallback callback1;
  std::unique_ptr<HQStreamCodec> upstreamCodec =
      std::make_unique<hq::HQStreamCodec>(
          idh1.first,
          TransportDirection::UPSTREAM,
          qpackCodec_,
          encoderWriteBuf_,
          decoderWriteBuf_,
          [] { return std::numeric_limits<uint64_t>::max(); },
          ingressSettings_);
  upstreamCodec->setCallback(&callback1);
  upstreamCodec->onIngress(
      *socketDriver_->streams_[idh1.first].writeBuf.front());
  EXPECT_EQ(callback1.unknownFrames, 1);
  EXPECT_EQ(callback1.greaseFrames, 1);

  // no grease frame will be created in the second bidir stream
  auto idh2 = checkRequest();
  flushRequestsAndLoop();
  FakeHTTPCodecCallback callback2;
  upstreamCodec->setCallback(&callback2);
  upstreamCodec->onIngress(
      *socketDriver_->streams_[idh2.first].writeBuf.front());
  EXPECT_EQ(callback2.unknownFrames, 0);
  EXPECT_EQ(callback2.greaseFrames, 0);
  hqSession_->closeWhenIdle();
}

namespace {
void verifyDSROffset(size_t dsrOffset,
                     size_t headerBytes,
                     size_t dataFrameHeaderSize) {
  // See HQFramer.cpp writeGreaseFrame, grease 'index' rand(16)
  // Length=0 -> 1 byte
  auto minGreaseFrameSize = *quic::getQuicIntegerSize(*hq::getGreaseId(0)) + 1;
  auto maxGreaseFrameSize = *quic::getQuicIntegerSize(*hq::getGreaseId(15)) + 1;

  // HEADERS frame id encoded length = 1
  auto headerFrameHeaderSize = *quic::getQuicIntegerSize(headerBytes) + 1;
  auto noGreaseSize = headerFrameHeaderSize + headerBytes + dataFrameHeaderSize;

  EXPECT_GE(dsrOffset, minGreaseFrameSize + noGreaseSize);
  EXPECT_LE(dsrOffset, maxGreaseFrameSize + noGreaseSize);
}
} // namespace

TEST_P(HQDownstreamSessionTest, DelegateResponse) {
  // Send a request but use DSR to delegate the response.
  // This test exits normally.
  sendRequest("/cdn.thing", 0, true);
  InSequence handlerSequence;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  auto dsrRequestSender = std::make_unique<MockQuicDSRRequestSender>();
  auto rawDsrSender = dsrRequestSender.get();
  std::unique_ptr<quic::DSRPacketizationRequestSender> senderStorage;
  handler->expectEOM([&]() {
    handler->txn_->setTransportCallback(&transportCallback_);
    auto mockDsrRequestSender =
        dynamic_cast<MockDSRRequestSender*>(rawDsrSender);
    CHECK(mockDsrRequestSender);
    EXPECT_CALL(*socketDriver_->getSocket(),
                setDSRPacketizationRequestSender(_, _))
        .WillOnce(Invoke(
            [&](StreamId,
                std::unique_ptr<quic::DSRPacketizationRequestSender> sender) {
              EXPECT_EQ(rawDsrSender, sender.get());
              senderStorage = std::move(sender);
              return folly::unit;
            }));
    folly::Optional<size_t> dsrOffset;
    EXPECT_CALL(*mockDsrRequestSender, onHeaderBytesGenerated(_))
        .WillOnce(Invoke([&](auto bytes) {
          handler->txn_->addBufferMeta();
          handler->txn_->sendEOM();
          dsrOffset = bytes;
        }));
    EXPECT_TRUE(handler->sendHeadersWithDelegate(
        200, 1000 * 10, std::move(dsrRequestSender)));
    EXPECT_GT(transportCallback_.bodyBytesGenerated_, 0);
    auto dataFrameHeaderSize = transportCallback_.bodyBytesGenerated_;
    ASSERT_TRUE(dsrOffset.hasValue());
    verifyDSROffset(*dsrOffset,
                    transportCallback_.headerBytesGenerated_,
                    dataFrameHeaderSize);
    EXPECT_TRUE(handler->txn_->isEgressStarted());
    handler->txn_->onWriteReady(10 * 1000, 1.0);
    EXPECT_EQ(transportCallback_.bodyBytesGenerated_,
              10 * 1000 + dataFrameHeaderSize);
    // from sendHeadersWithDelegate, to actually doing writeChain and
    // writeBufMeta, there is an extra loop. So the verification + abort also
    // needs to be placed in a later loop.
    eventBase_.runInLoop([&] {
      ASSERT_TRUE(transportCallback_.lastByteFlushed_);
      handler->expectDetachTransaction();
      EXPECT_CALL(*socketDriver_->getSocket(),
                  setDSRPacketizationRequestSender(Eq(0), Eq(nullptr)));
    });
  });
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, DelegateResponseError) {
  // Send a request but use DSR to delegate the response.
  // This test exits via the error path on the DSR request.
  auto streamId = sendRequest("/cdn.thing", 0, true);
  InSequence handlerSequence;
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  auto dsrRequestSender = std::make_unique<MockQuicDSRRequestSender>();
  auto rawDsrSender = dsrRequestSender.get();
  std::unique_ptr<quic::DSRPacketizationRequestSender> senderStorage;
  handler->expectEOM([&]() {
    handler->txn_->setTransportCallback(&transportCallback_);
    auto mockDsrRequestSender =
        dynamic_cast<MockDSRRequestSender*>(rawDsrSender);
    CHECK(mockDsrRequestSender);
    EXPECT_CALL(*socketDriver_->getSocket(),
                setDSRPacketizationRequestSender(_, _))
        .WillOnce(Invoke(
            [&](StreamId,
                std::unique_ptr<quic::DSRPacketizationRequestSender> sender) {
              EXPECT_EQ(rawDsrSender, sender.get());
              senderStorage = std::move(sender);
              return folly::unit;
            }));
    folly::Optional<size_t> dsrOffset;
    EXPECT_CALL(*mockDsrRequestSender, onHeaderBytesGenerated(_))
        .WillOnce(Invoke([&](auto bytes) {
          handler->txn_->addBufferMeta();
          handler->txn_->sendEOM();
          dsrOffset = bytes;
        }));
    EXPECT_TRUE(handler->sendHeadersWithDelegate(
        200, 1000 * 10, std::move(dsrRequestSender)));
    EXPECT_GT(transportCallback_.bodyBytesGenerated_, 0);
    auto dataFrameHeaderSize = transportCallback_.bodyBytesGenerated_;
    ASSERT_TRUE(dsrOffset.hasValue());
    verifyDSROffset(*dsrOffset,
                    transportCallback_.headerBytesGenerated_,
                    dataFrameHeaderSize);
    EXPECT_TRUE(handler->txn_->isEgressStarted());
    handler->txn_->onWriteReady(10 * 1000, 1.0);
    EXPECT_EQ(transportCallback_.bodyBytesGenerated_,
              10 * 1000 + dataFrameHeaderSize);
    // from sendHeadersWithDelegate, to actually doing writeChain and
    // writeBufMeta, there is an extra loop. So the verification + abort also
    // needs to be placed in a later loop.
    eventBase_.runInLoop([&] {
      ASSERT_TRUE(transportCallback_.lastByteFlushed_);
      // Both onStopSending and terminate the handler can clear the buffered
      // BufMetas and make the transaction detachable.
      handler->expectDetachTransaction();
      EXPECT_CALL(*socketDriver_->getSocket(),
                  setDSRPacketizationRequestSender(Eq(0), Eq(nullptr)));
      hqSession_->onStopSending(streamId,
                                HTTP3::ErrorCode::HTTP_REQUEST_REJECTED);
    });
  });
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, getHTTPPriority) {
  folly::Optional<HTTPPriority> expectedResults = HTTPPriority{1, true};

  auto request = getProgressiveGetRequest();
  sendRequest(request);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();
  handler->expectEOM([&]() {
    auto resp = makeResponse(200, 0);

    EXPECT_CALL(*socketDriver_->getSocket(),
                getStreamPriority(handler->txn_->getID()))
        .WillOnce(Return(quic::Priority(expectedResults.value().urgency,
                                        expectedResults.value().incremental)))
        .WillOnce(
            Return(folly::makeUnexpected(LocalErrorCode::STREAM_NOT_EXISTS)))
        .WillOnce(Return(quic::Priority(expectedResults.value().urgency,
                                        expectedResults.value().incremental)));

    auto urgencyIncremental = handler->txn_->getHTTPPriority().value();
    EXPECT_EQ(urgencyIncremental.urgency, expectedResults.value().urgency);
    EXPECT_EQ(urgencyIncremental.incremental,
              expectedResults.value().incremental);
    EXPECT_EQ(handler->txn_->getHTTPPriority(), folly::none);
    handler->sendRequest(*std::get<0>(resp));
  });
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTest, IdleTimeoutNoStreams) {
  std::chrono::milliseconds connIdleTimeout{200};
  auto connManager = wangle::ConnectionManager::makeUnique(
      &eventBase_, connIdleTimeout, nullptr);
  connManager->addConnection(hqSession_, true);
  // Just run the loop, the session will timeout, drain and close
  auto start = std::chrono::steady_clock::now();
  eventBase_.loop();
  auto end = std::chrono::steady_clock::now();
  EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count(),
            connIdleTimeout.count());
}

TEST_P(HQDownstreamSessionTest, IdleTimeoutResetWithPing) {
  std::chrono::milliseconds connIdleTimeout{200};
  auto connManager = wangle::ConnectionManager::makeUnique(
      &eventBase_, connIdleTimeout, nullptr);
  connManager->addConnection(hqSession_, true);
  for (int i = 1; i <= 4; i++) {
    socketDriver_->addPingReceivedReadEvent(std::chrono::milliseconds(100));
  }
  // Just run the loop, the session will timeout, drain and close
  auto start = std::chrono::steady_clock::now();
  flushRequestsAndLoop();
  auto end = std::chrono::steady_clock::now();
  EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count(),
            connIdleTimeout.count() * 2);
}

TEST_P(HQDownstreamSessionTest, IdleTimeoutResetWithPingAcknowledged) {
  std::chrono::milliseconds connIdleTimeout{200};
  auto connManager = wangle::ConnectionManager::makeUnique(
      &eventBase_, connIdleTimeout, nullptr);
  connManager->addConnection(hqSession_, true);
  for (int i = 1; i <= 4; i++) {
    socketDriver_->addPingAcknowledgedReadEvent(std::chrono::milliseconds(100));
  }
  // Just run the loop, the session will timeout, drain and close
  auto start = std::chrono::steady_clock::now();
  flushRequestsAndLoop();
  auto end = std::chrono::steady_clock::now();
  EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count(),
            connIdleTimeout.count() * 2);
}

/**
 * Instantiate the Parametrized test cases
 */

// Make sure all the tests keep working with all the supported protocol versions
INSTANTIATE_TEST_SUITE_P(HQDownstreamSessionTest,
                         HQDownstreamSessionTest,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           return tp;
                         }()),
                         paramsToTestName);

INSTANTIATE_TEST_SUITE_P(HQDownstreamSessionTest,
                         HQDownstreamSessionFilterTestHQ,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           tp.createQPACKStreams_ = true;
                           tp.shouldSendSettings_ = false;
                           return tp;
                         }()),
                         paramsToTestName);

INSTANTIATE_TEST_SUITE_P(HQDownstreamSessionBeforeTransportReadyTest,
                         HQDownstreamSessionBeforeTransportReadyTest,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           return tp;
                         }()),
                         paramsToTestName);

TEST_P(HQDownstreamSessionTestPush, SimplePush) {
  auto id = sendRequest("/", 1);
  HTTPMessage promiseReq, res;
  promiseReq.getHeaders().set(HTTP_HEADER_HOST, "www.foo.com");
  promiseReq.setURL("/");
  res.setStatusCode(200);
  res.setStatusMessage("Ohai");

  auto handler = addSimpleStrictHandler();
  StrictMock<MockHTTPPushHandler> pushHandler;
  handler->expectHeaders();
  HTTPCodec::StreamID pushStreamId = 0;
  handler->expectEOM([&] {
    // Generate response for the associated stream
    handler->txn_->sendHeaders(res);
    handler->txn_->sendBody(makeBuf(100));

    // Different from H2, this counts as an outgoing stream as soon as the
    // txn is created.
    // TODO: maybe create the stream lazily when trying to send the real
    // headers instead?
    auto outgoingStreams = hqSession_->getNumOutgoingStreams();
    auto* pushTxn = handler->txn_->newPushedTransaction(&pushHandler);
    ASSERT_NE(pushTxn, nullptr);
    EXPECT_EQ(hqSession_->getNumOutgoingStreams(), outgoingStreams + 1);
    // Generate a push request (PUSH_PROMISE)
    pushTxn->sendHeaders(promiseReq);
    pushStreamId = pushTxn->getID();
    LOG(INFO) << "pushStreamId=" << pushStreamId;
    pushTxn->sendHeaders(res);
    pushTxn->sendBody(makeBuf(200));
    pushTxn->sendEOM();
  });
  EXPECT_CALL(pushHandler, _setTransaction(_))
      .WillOnce(Invoke([&](HTTPTransaction* txn) { pushHandler.txn_ = txn; }));
  EXPECT_CALL(pushHandler, _detachTransaction());

  flushRequestsAndLoopN(1);
  handler->txn_->sendEOM();
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  auto pushIt = pushes_.find(pushStreamId);
  ASSERT_TRUE(pushIt != pushes_.end());
  EXPECT_GT(socketDriver_->streams_[pushIt->first].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[pushIt->first].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTestPush, PushPriorityCallback) {
  auto id = sendRequest("/", 1);
  HTTPMessage promiseReq, res;
  promiseReq.getHeaders().set(HTTP_HEADER_HOST, "www.foo.com");
  promiseReq.setURL("/");
  res.setStatusCode(200);
  res.setStatusMessage("Ohai");

  auto handler = addSimpleStrictHandler();
  StrictMock<MockHTTPPushHandler> pushHandler;
  handler->expectHeaders();
  HTTPCodec::StreamID pushStreamId = 0;
  handler->expectEOM([&] {
    handler->txn_->sendHeaders(res);
    handler->txn_->sendBody(makeBuf(100));

    auto outgoingStreams = hqSession_->getNumOutgoingStreams();
    auto* pushTxn = handler->txn_->newPushedTransaction(&pushHandler);
    ASSERT_NE(pushTxn, nullptr);
    EXPECT_EQ(hqSession_->getNumOutgoingStreams(), outgoingStreams + 1);
    // Generate a push request (PUSH_PROMISE)
    pushTxn->sendHeaders(promiseReq);
    pushStreamId = pushTxn->getID();
    pushTxn->sendHeaders(res);
    pushTxn->sendBody(makeBuf(200));
    pushTxn->sendEOM();
  });
  EXPECT_CALL(pushHandler, _setTransaction(_))
      .WillOnce(Invoke([&](HTTPTransaction* txn) { pushHandler.txn_ = txn; }));
  EXPECT_CALL(pushHandler, _detachTransaction());

  flushRequestsAndLoopN(1);

  // Push stream's priority can be updated either with stream id or push id:
  auto pushId = pushes_.find(pushStreamId)->second;
  EXPECT_CALL(*socketDriver_->getSocket(),
              setStreamPriority(pushStreamId, Priority(6, true)));
  hqSession_->onPushPriority(pushId, HTTPPriority(6, true));
  EXPECT_CALL(*socketDriver_->getSocket(),
              setStreamPriority(pushStreamId, Priority(5, true)));
  hqSession_->onPriority(pushStreamId, HTTPPriority(5, true));

  handler->txn_->sendEOM();
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  auto pushIt = pushes_.find(pushStreamId);
  ASSERT_TRUE(pushIt != pushes_.end());
  EXPECT_GT(socketDriver_->streams_[pushIt->first].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[pushIt->first].writeEOF);
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTestPush, StopSending) {
  auto id = sendRequest("/", 1);
  HTTPMessage req, res;
  req.getHeaders().set("HOST", "www.foo.com");
  req.setURL("https://www.foo.com/");
  res.setStatusCode(200);
  res.setStatusMessage("Ohai");

  auto handler = addSimpleStrictHandler();
  StrictMock<MockHTTPPushHandler> pushHandler;
  handler->expectHeaders();
  HTTPCodec::StreamID pushStreamId = 0;
  handler->expectEOM([&] {
    // Generate response for the associated stream
    handler->txn_->sendHeaders(res);
    handler->txn_->sendBody(makeBuf(100));

    // Different from H2, this counts as an outgoing stream as soon as the
    // txn is created.
    // TODO: maybe create the stream lazily when trying to send the real
    // headers instead?
    auto outgoingStreams = hqSession_->getNumOutgoingStreams();
    auto* pushTxn = handler->txn_->newPushedTransaction(&pushHandler);
    ASSERT_NE(pushTxn, nullptr);
    EXPECT_EQ(hqSession_->getNumOutgoingStreams(), outgoingStreams + 1);
    // Generate a push request (PUSH_PROMISE)
    pushTxn->sendHeaders(req);
    pushStreamId = pushTxn->getID();
    LOG(INFO) << "pushStreamId=" << pushStreamId;
    pushTxn->sendHeaders(res);
    pushTxn->sendBody(makeBuf(200));
    // NO EOM
  });
  EXPECT_CALL(pushHandler, _setTransaction(_))
      .WillOnce(Invoke([&](HTTPTransaction* txn) { pushHandler.txn_ = txn; }));
  EXPECT_CALL(pushHandler, _onError(_));
  EXPECT_CALL(pushHandler, _detachTransaction());

  flushRequestsAndLoopN(1);
  handler->txn_->sendEOM();
  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  EXPECT_GT(socketDriver_->streams_[id].writeBuf.chainLength(), 110);
  EXPECT_TRUE(socketDriver_->streams_[id].writeEOF);
  auto pushIt = pushes_.find(pushStreamId);
  ASSERT_TRUE(pushIt != pushes_.end());
  EXPECT_GT(socketDriver_->streams_[pushIt->first].writeBuf.chainLength(), 110);
  EXPECT_FALSE(socketDriver_->streams_[pushIt->first].writeEOF);
  // Cancel the push with stop sending
  socketDriver_->addStopSending(pushIt->first,
                                HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED);
  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

using DropConnectionInTransportReadyTest =
    HQDownstreamSessionBeforeTransportReadyTest;

INSTANTIATE_TEST_SUITE_P(ControlStreamsStallTest,
                         ControlStreamsStallTest,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           tp.checkUniridStreamCallbacks = false;
                           return tp;
                         }()),
                         paramsToTestName);

INSTANTIATE_TEST_SUITE_P(DropConnectionInTransportReadyTest,
                         DropConnectionInTransportReadyTest,
                         Values(
                             [] {
                               TestParams tp;
                               tp.alpn_ = "unsupported";
                               tp.expectOnTransportReady = false;
                               return tp;
                             }(),
                             [] {
                               TestParams tp;
                               tp.alpn_ = "h3";
                               tp.unidirectionalStreamsCredit = 1;
                               tp.expectOnTransportReady = false;
                               return tp;
                             }()),
                         paramsToTestName);
// Instantiate hq server push tests
INSTANTIATE_TEST_SUITE_P(HQDownstreamSessionTest,
                         HQDownstreamSessionTestPush,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           tp.unidirectionalStreamsCredit = 8;
                           return tp;
                         }()),
                         paramsToTestName);

// Use this test class for mismatched alpn tests
class HQDownstreamSessionTestUnsupportedAlpn : public HQDownstreamSessionTest {
 public:
  void SetUp() override {
    SetUpBase();
  }
};

TEST_P(DropConnectionInTransportReadyTest, TransportReadyFailure) {
  HQDownstreamSession::DestructorGuard dg(hqSession_);
  EXPECT_CALL(infoCb_, onTransportReady(_)).Times(0);
  EXPECT_CALL(infoCb_, onConnectionError(_))
      .WillOnce(Invoke([](const HTTPSessionBase& session) {
        const auto hqSession = dynamic_cast<const HQSession*>(&session);
        ASSERT_NE(hqSession, nullptr);
        ASSERT_NE(hqSession->getQuicSocket(), nullptr);
      }));
  SetUpOnTransportReady();
  EXPECT_EQ(hqSession_->getQuicSocket(), nullptr);
}

TEST_P(HQDownstreamSessionTestDeliveryAck,
       DropConnectionWithDeliveryAckCbSetError) {
  auto req = getGetRequest();
  auto streamId = sendRequest(req);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();

  // Start the response.
  handler->expectEOM([&]() {
    handler->txn_->setTransportCallback(&transportCallback_);
    handler->sendHeaders(200, 1723);
  });

  auto sock = socketDriver_->getSocket();

  // This is a copy of the one in MockQuicSocketDriver, only hijacks data stream
  // and forces an error.
  EXPECT_CALL(*sock,
              registerByteEventCallback(quic::QuicSocket::ByteEvent::Type::ACK,
                                        testing::_,
                                        testing::_,
                                        testing::_))
      .WillRepeatedly(
          testing::Invoke([streamId, &socketDriver = socketDriver_](
                              quic::QuicSocket::ByteEvent::Type,
                              quic::StreamId id,
                              uint64_t offset,
                              MockQuicSocket::ByteEventCallback* cb)
                              -> folly::Expected<folly::Unit, LocalErrorCode> {
            if (id == streamId) {
              return folly::makeUnexpected(LocalErrorCode::INVALID_OPERATION);
            }

            socketDriver->checkNotReadOnlyStream(id);
            auto it = socketDriver->streams_.find(id);
            if (it == socketDriver->streams_.end() ||
                it->second.nextWriteOffset >= offset) {
              return folly::makeUnexpected(LocalErrorCode::STREAM_NOT_EXISTS);
            }
            CHECK_NE(it->second.writeState,
                     MockQuicSocketDriver::StateEnum::CLOSED);
            it->second.deliveryCallbacks.push_back({offset, cb});
            return folly::unit;
          }));

  EXPECT_CALL(*handler, _onError(_))
      .WillOnce(Invoke([](const HTTPException& error) {
        EXPECT_TRUE(std::string(error.what())
                        .find("failed to register byte event callback") !=
                    std::string::npos);
      }));
  handler->expectDetachTransaction();

  flushRequestsAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTestDeliveryAck, TestBodyDeliveryAck) {
  auto req = getGetRequest();
  sendRequest(req);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();

  auto length = 42;
  auto offset = length - 1;
  // Start the response.
  handler->expectEOM([&]() {
    handler->txn_->setTransportCallback(&transportCallback_);
    handler->sendHeaders(200, length);
    handler->sendBody(length);
    EXPECT_TRUE(handler->txn_->trackEgressBodyOffset(
        handler->txn_->bodyBytesSent() - 1, ByteEvent::EventFlags::ACK));
    handler->sendEOM();
  });

  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  EXPECT_EQ(transportCallback_.numBodyBytesDeliveredCalls_, 1);
  EXPECT_EQ(transportCallback_.bodyBytesDeliveredOffset_, offset);

  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTestDeliveryAck, TestBodyTxCallback) {
  auto req = getGetRequest();
  sendRequest(req);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();

  auto length = 42;
  auto offset = length - 1;
  // Start the response.
  handler->expectEOM([&]() {
    handler->txn_->setTransportCallback(&transportCallback_);
    handler->sendHeaders(200, length);
    EXPECT_TRUE(handler->txn_->trackEgressBodyOffset(
        offset, ByteEvent::EventFlags::TX));
    handler->sendBody(length);
    handler->sendEOM();
  });

  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  EXPECT_EQ(transportCallback_.numBodyBytesTxCalls_, 1);
  EXPECT_EQ(transportCallback_.bodyBytesTxOffset_, offset);

  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTestDeliveryAck, TestBodyDeliveryAckMultiple) {
  auto req = getGetRequest();
  sendRequest(req);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();

  auto length = 42 + 17;
  auto offset1 = 41;
  auto offset2 = length - 1;
  // Start the response.
  handler->expectEOM([&]() {
    handler->txn_->setTransportCallback(&transportCallback_);
    handler->sendHeaders(200, length);
    // Test registering callbacks before sendBody
    EXPECT_TRUE(handler->txn_->trackEgressBodyOffset(
        offset1, ByteEvent::EventFlags::TX | ByteEvent::EventFlags::ACK));
    EXPECT_TRUE(handler->txn_->trackEgressBodyOffset(
        offset2, ByteEvent::EventFlags::ACK));
    handler->sendBody(42);
    handler->sendBody(17);
    handler->sendEOM();
  });

  handler->expectDetachTransaction();
  flushRequestsAndLoop();
  EXPECT_EQ(transportCallback_.numBodyBytesTxCalls_, 1);
  EXPECT_EQ(transportCallback_.bodyBytesTxOffset_, offset1);
  EXPECT_EQ(transportCallback_.numBodyBytesDeliveredCalls_, 2);
  EXPECT_EQ(transportCallback_.bodyBytesDeliveredOffset_, offset2);

  hqSession_->closeWhenIdle();
}

TEST_P(HQDownstreamSessionTestDeliveryAck, TestBodyDeliveryErr) {
  auto req = getGetRequest();
  auto streamId = sendRequest(req);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();

  auto length = 42;
  auto offset = length - 1;
  // Start the response.
  handler->expectEOM([&]() {
    handler->txn_->setTransportCallback(&transportCallback_);
    handler->sendHeaders(200, length);
    EXPECT_TRUE(handler->txn_->trackEgressBodyOffset(
        offset, ByteEvent::EventFlags::ACK));
  });
  flushRequestsAndLoop();
  EXPECT_TRUE(transportCallback_.lastEgressHeadersByteDelivered_);

  // One day, txn_->sendHeaders() will return number of bytes written, and we
  // won't need this. For now, H3 frame headers size is 2 bytes.
  const uint64_t frameHeaderSize = 2;
  const uint64_t streamOffsetAfterHeaders =
      (2 * frameHeaderSize) + transportCallback_.headerBytesGenerated_;

  auto sock = socketDriver_->getSocket();

  // This is a copy of the one in MockQuicSocketDriver, only hijacks data stream
  // and forces an error.
  EXPECT_CALL(*sock,
              registerByteEventCallback(quic::QuicSocket::ByteEvent::Type::ACK,
                                        testing::_,
                                        testing::_,
                                        testing::_))
      .WillRepeatedly(testing::Invoke(
          [streamId,
           &streamOffsetAfterHeaders = streamOffsetAfterHeaders,
           &socketDriver = socketDriver_](quic::QuicSocket::ByteEvent::Type,
                                          quic::StreamId id,
                                          uint64_t offset,
                                          MockQuicSocket::ByteEventCallback* cb)
              -> folly::Expected<folly::Unit, LocalErrorCode> {
            if (id == streamId && offset > streamOffsetAfterHeaders) {
              for (auto& it : socketDriver->streams_) {
                auto& stream = it.second;
                stream.readState = quic::MockQuicSocketDriver::ERROR;
                stream.writeState = quic::MockQuicSocketDriver::ERROR;
              }
              return folly::makeUnexpected(LocalErrorCode::INVALID_OPERATION);
            }

            socketDriver->checkNotReadOnlyStream(id);
            auto it = socketDriver->streams_.find(id);
            if (it == socketDriver->streams_.end() ||
                it->second.nextWriteOffset >= offset) {
              return folly::makeUnexpected(LocalErrorCode::STREAM_NOT_EXISTS);
            }
            CHECK_NE(it->second.writeState,
                     MockQuicSocketDriver::StateEnum::CLOSED);
            it->second.deliveryCallbacks.push_back({offset, cb});
            return folly::unit;
          }));

  EXPECT_CALL(*handler, _onError(_))
      .WillOnce(Invoke([&handler = handler](const HTTPException& error) {
        EXPECT_TRUE(std::string(error.what())
                        .find("failed to register byte event callback") !=
                    std::string::npos);
        handler->txn_->sendAbort();
      }));

  handler->expectDetachTransaction();

  handler->sendBody(42);
  flushRequestsAndLoop();
}

TEST_P(HQDownstreamSessionTestDeliveryAck, TestBodyDeliveryCancel) {
  auto req = getGetRequest();
  sendRequest(req);
  auto handler = addSimpleStrictHandler();
  handler->expectHeaders();

  auto length = 42;
  auto offset = length - 1;
  // Start the response.
  handler->expectEOM([&]() {
    handler->txn_->setTransportCallback(&transportCallback_);
    handler->sendHeaders(200, length);
    EXPECT_TRUE(handler->txn_->trackEgressBodyOffset(
        offset, ByteEvent::EventFlags::TX | ByteEvent::EventFlags::ACK));
    handler->sendBody(length);
    // handler->sendEOM();
  });

  flushRequestsAndLoopN(1);

  EXPECT_CALL(*handler, _onError(_)).Times(1);
  handler->expectDetachTransaction();
  socketDriver_->deliverErrorOnAllStreams(
      quic::QuicError(LocalErrorCode::INVALID_OPERATION, "fake error"));
  flushRequestsAndLoop();

  EXPECT_EQ(transportCallback_.numBodyBytesCanceledCalls_, 2);
  EXPECT_EQ(transportCallback_.bodyBytesCanceledOffset_, offset);
}

INSTANTIATE_TEST_SUITE_P(HQDownstreamSessionTest,
                         HQDownstreamSessionTestDeliveryAck,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           return tp;
                         }()),
                         paramsToTestName);
