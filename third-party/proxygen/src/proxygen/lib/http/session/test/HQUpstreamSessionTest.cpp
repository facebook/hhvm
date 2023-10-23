/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/test/HQUpstreamSessionTest.h>

#include <proxygen/lib/http/codec/CodecUtil.h>
#include <proxygen/lib/http/session/HQUpstreamSession.h>

#include <folly/futures/Future.h>
#include <folly/portability/GTest.h>
#include <limits>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/codec/HQControlCodec.h>
#include <proxygen/lib/http/codec/HQStreamCodec.h>
#include <proxygen/lib/http/codec/HQUnidirectionalCodec.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/session/test/HQSessionMocks.h>
#include <proxygen/lib/http/session/test/HQSessionTestCommon.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <proxygen/lib/http/session/test/MockQuicSocketDriver.h>
#include <proxygen/lib/http/session/test/MockSessionObserver.h>
#include <proxygen/lib/http/session/test/TestUtils.h>
#include <quic/api/test/MockQuicSocket.h>
#include <wangle/acceptor/ConnectionManager.h>

using namespace proxygen;
using namespace proxygen::hq;
using namespace quic;
using namespace testing;
using namespace std::chrono;

namespace {
constexpr quic::StreamId kQPACKEncoderIngressStreamId = 7;
constexpr quic::StreamId kQPACKDecoderEgressStreamId = 10;
} // namespace

std::pair<HTTPCodec::StreamID, std::unique_ptr<HTTPCodec>>
HQUpstreamSessionTest::makeCodec(HTTPCodec::StreamID id) {
  return {id,
          std::make_unique<hq::HQStreamCodec>(
              id,
              TransportDirection::DOWNSTREAM,
              qpackCodec_,
              encoderWriteBuf_,
              decoderWriteBuf_,
              [] { return std::numeric_limits<uint64_t>::max(); },
              ingressSettings_)};
}

void HQUpstreamSessionTest::sendResponse(quic::StreamId id,
                                         const HTTPMessage& resp,
                                         std::unique_ptr<folly::IOBuf> body,
                                         bool eom) {
  auto c = makeCodec(id);
  auto res =
      streams_.emplace(std::piecewise_construct,
                       std::forward_as_tuple(id),
                       std::forward_as_tuple(c.first, std::move(c.second)));
  auto& stream = res.first->second;
  stream.readEOF = eom;
  stream.codec->generateHeader(
      stream.buf, stream.codecId, resp, body == nullptr ? eom : false);
  if (body && body->computeChainDataLength() > 0) {
    stream.codec->generateBody(
        stream.buf, stream.codecId, std::move(body), folly::none, eom);
  }
}

void HQUpstreamSessionTest::sendPartialBody(quic::StreamId id,
                                            std::unique_ptr<folly::IOBuf> body,
                                            bool eom) {
  auto it = streams_.find(id);
  CHECK(it != streams_.end());
  auto& stream = it->second;

  stream.readEOF = eom;
  if (body) {
    stream.codec->generateBody(
        stream.buf, stream.codecId, std::move(body), folly::none, eom);
  }
}

quic::StreamId HQUpstreamSessionTest::nextUnidirectionalStreamId() {
  auto id = nextUnidirectionalStreamId_;
  nextUnidirectionalStreamId_ += 4;
  return id;
}

void HQUpstreamSessionTest::SetUp() {
  HQSessionTest::SetUp();
  dynamic_cast<HQUpstreamSession*>(hqSession_)->setConnectCallback(&connectCb_);

  EXPECT_CALL(connectCb_, connectSuccess());

  hqSession_->onTransportReady();

  createControlStreams();

  flushAndLoop();
  EXPECT_EQ(httpCallbacks_.settings, 1);
}

void HQUpstreamSessionTest::TearDown() {
  // With control streams we may need an extra loop for proper shutdown
  if (!socketDriver_->isClosed()) {
    // Send the first GOAWAY with MAX_STREAM_ID immediately
    sendGoaway(HTTPCodec::MaxStreamID);
    // Schedule the second GOAWAY with the last seen stream ID, after some
    // delay
    sendGoaway(socketDriver_->getMaxStreamId(), milliseconds(50));
  }
  eventBase_.loopOnce();
}

void HQUpstreamSessionTest::sendGoaway(quic::StreamId lastStreamId,
                                       milliseconds delay) {
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  egressControlCodec_->generateGoaway(
      writeBuf, lastStreamId, ErrorCode::NO_ERROR);
  socketDriver_->addReadEvent(connControlStreamId_, writeBuf.move(), delay);
}

template <class HandlerType>
std::unique_ptr<StrictMock<HandlerType>>
HQUpstreamSessionTest::openTransactionBase(bool expectStartPaused) {
  // Returns a mock handler with txn_ field set in it
  auto handler = std::make_unique<StrictMock<HandlerType>>();
  handler->expectTransaction();
  if (expectStartPaused) {
    handler->expectEgressPaused();
  }
  HTTPTransaction* txn = hqSession_->newTransaction(handler.get());
  EXPECT_EQ(txn, handler->txn_);
  return handler;
}

std::unique_ptr<StrictMock<MockHTTPHandler>>
HQUpstreamSessionTest::openTransaction() {
  return openTransactionBase<MockHTTPHandler>();
}

void HQUpstreamSessionTest::flushAndLoop(bool eof,
                                         milliseconds eofDelay,
                                         milliseconds initialDelay,
                                         std::function<void()> extraEventsFn) {
  flush(eof, eofDelay, initialDelay, extraEventsFn);
  CHECK(eventBase_.loop());
}

void HQUpstreamSessionTest::flushAndLoopN(uint64_t n,
                                          bool eof,
                                          milliseconds eofDelay,
                                          milliseconds initialDelay,
                                          std::function<void()> extraEventsFn) {
  flush(eof, eofDelay, initialDelay, extraEventsFn);
  for (uint64_t i = 0; i < n; i++) {
    eventBase_.loopOnce();
  }
}

bool HQUpstreamSessionTest::flush(bool eof,
                                  milliseconds eofDelay,
                                  milliseconds initialDelay,
                                  std::function<void()> extraEventsFn) {
  bool done = true;
  if (!encoderWriteBuf_.empty()) {
    socketDriver_->addReadEvent(
        kQPACKEncoderIngressStreamId, encoderWriteBuf_.move(), milliseconds(0));
  }
  for (auto& stream : streams_) {
    if (socketDriver_->isStreamIdle(stream.first)) {
      continue;
    }
    if (stream.second.buf.chainLength() > 0) {
      socketDriver_->addReadEvent(
          stream.first, stream.second.buf.move(), initialDelay);
      done = false;
    }
    // EOM -> stream EOF
    if (stream.second.readEOF) {
      socketDriver_->addReadEOF(stream.first, eofDelay);
      done = false;
    }
  }
  if (!socketDriver_->inDatagrams_.empty()) {
    socketDriver_->addDatagramsAvailableReadEvent(initialDelay);
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

std::unique_ptr<MockSessionObserver>
HQUpstreamSessionTest::addMockSessionObserver(
    MockSessionObserver::EventSet eventSet) {
  auto observer = std::make_unique<NiceMock<MockSessionObserver>>(eventSet);
  EXPECT_CALL(*observer, attached(_));
  hqSession_->addObserver(observer.get());
  return observer;
}
StrictMock<MockController>& HQUpstreamSessionTest::getMockController() {
  return controllerContainer_.mockController;
}

// Use this test class for hq only tests
using HQUpstreamSessionTest = HQUpstreamSessionTest;
// Use this test class for hq only tests with qpack encoder streams on/off
using HQUpstreamSessionTestQPACK = HQUpstreamSessionTest;
// Use this test class for hq only tests with Datagram support
using HQUpstreamSessionTestDatagram = HQUpstreamSessionTest;

TEST_P(HQUpstreamSessionTest, SimpleGet) {
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM();
  handler->expectDetachTransaction();
  auto resp = makeResponse(200, 100);
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, GetWithTrailers) {
  auto handler = openTransaction();
  auto req = getGetRequest();
  handler->txn_->sendHeaders(req);
  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "trailer1");
  handler->txn_->sendTrailers(trailers);
  handler->txn_->sendEOM();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectTrailers();
  handler->expectEOM();
  handler->expectDetachTransaction();
  auto resp = makeResponse(200, 100);
  auto id = handler->txn_->getID();
  sendResponse(id, *std::get<0>(resp), std::move(std::get<1>(resp)), false);
  auto it = streams_.find(id);
  CHECK(it != streams_.end());
  auto& stream = it->second;
  trailers.remove("x-trailer-1");
  trailers.add("x-trailer-2", "trailer2");
  stream.codec->generateTrailers(stream.buf, stream.codecId, trailers);
  stream.codec->generateEOM(stream.buf, stream.codecId);
  stream.readEOF = true;
  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, AbortOnBodyWithBlockedQPACKTrailers) {
  auto handler = openTransaction();
  auto req = getGetRequest();
  handler->txn_->sendHeaders(req);
  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "trailer1");
  handler->txn_->sendTrailers(trailers);
  handler->txn_->sendEOM();
  handler->expectHeaders();
  handler->expectBody([&handler] { handler->txn_->sendAbort(); });
  handler->expectDetachTransaction();
  auto resp = makeResponse(200, 100);
  auto id = handler->txn_->getID();
  sendResponse(id, *std::get<0>(resp), std::move(std::get<1>(resp)), false);
  auto control1 = encoderWriteBuf_.move();
  flushAndLoopN(1);
  auto it = streams_.find(id);
  CHECK(it != streams_.end());
  auto& stream = it->second;
  trailers.remove("x-trailer-1");
  trailers.add("x-trailer-2", "trailer2");
  stream.codec->generateTrailers(stream.buf, stream.codecId, trailers);
  stream.codec->generateEOM(stream.buf, stream.codecId);
  stream.readEOF = true;
  auto control2 = encoderWriteBuf_.move();
  encoderWriteBuf_.append(std::move(control1));
  flushAndLoopN(1);
  encoderWriteBuf_.append(std::move(control2));

  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, PriorityUpdateIntoTransport) {
  auto handler = openTransaction();
  auto req = getGetRequest();
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3, i");
  EXPECT_CALL(*socketDriver_->getSocket(),
              setStreamPriority(_, quic::Priority(3, true)));
  handler->txn_->sendHeadersWithEOM(req);

  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM();
  handler->expectDetachTransaction();
  auto resp = makeResponse(200, 100);
  std::get<0>(resp)->getHeaders().add(HTTP_HEADER_PRIORITY, "u=5");
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  EXPECT_CALL(*socketDriver_->getSocket(),
              setStreamPriority(_, quic::Priority(5, false)));
  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, TestSupportsMoreTransactions) {
  auto infoCb = std::make_unique<
      testing::NiceMock<proxygen::MockHTTPSessionInfoCallback>>();
  hqSession_->setInfoCallback(infoCb.get());

  auto resp = makeResponse(200, 100);
  // set the max number of bidirectional streams = 1.
  socketDriver_->setMaxBidiStreams(1);

  // we should be able to open only one transaction
  EXPECT_TRUE(hqSession_->supportsMoreTransactions());
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM();
  handler->expectDetachTransaction();
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  flushAndLoop();

  // unable to create more transactions, should return nullptr
  EXPECT_FALSE(hqSession_->supportsMoreTransactions());
  auto* txn = hqSession_->newTransaction(handler.get());
  EXPECT_FALSE(txn);

  // receiving stream credits from peer should invoke
  // onSettingsOutgoingStreamsNotFull cb
  EXPECT_CALL(*infoCb, onSettingsOutgoingStreamsNotFull).Times(1);
  socketDriver_->setMaxBidiStreams(2);
  EXPECT_TRUE(hqSession_->supportsMoreTransactions());

  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, SendPriorityUpdate) {
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->expectHeaders();
  handler->expectBody([&]() {
    EXPECT_CALL(
        *socketDriver_->getSocket(),
        setStreamPriority(handler->txn_->getID(), quic::Priority(5, true)));
    handler->txn_->updateAndSendPriority(HTTPPriority(5, true));
  });
  handler->txn_->sendEOM();
  handler->expectEOM();
  handler->expectDetachTransaction();
  auto resp = makeResponse(200, 100);
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, SkipPriorityUpdateAfterSeenEOM) {
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM([&]() {
    EXPECT_CALL(
        *socketDriver_->getSocket(),
        setStreamPriority(handler->txn_->getID(), quic::Priority(5, true)))
        .Times(0);
    handler->txn_->updateAndSendPriority(HTTPPriority(5, true));
  });
  handler->txn_->sendEOM();

  handler->expectDetachTransaction();
  auto resp = makeResponse(200, 100);
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, NoNewTransactionIfSockIsNotGood) {
  socketDriver_->sockGood_ = false;
  EXPECT_EQ(hqSession_->newTransaction(nullptr), nullptr);
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, DropConnectionWithEarlyDataFailedError) {
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();

  EXPECT_CALL(*handler, _onError(_))
      .WillOnce(Invoke([](const HTTPException& error) {
        EXPECT_EQ(error.getProxygenError(), kErrorEarlyDataFailed);
        EXPECT_TRUE(std::string(error.what()).find("quic loses race") !=
                    std::string::npos);
      }));
  handler->expectDetachTransaction();
  socketDriver_->deliverConnectionError(
      {HTTP3::ErrorCode::GIVEUP_ZERO_RTT, "quic loses race"});
}

TEST_P(HQUpstreamSessionTest, TestGetHistoricalMaxOutgoingStreams) {
  EXPECT_EQ(hqSession_->getHistoricalMaxOutgoingStreams(), 0);
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM();
  handler->expectDetachTransaction();
  auto resp = makeResponse(200, 100);
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  auto handler1 = openTransaction();
  handler1->txn_->sendHeaders(getGetRequest());
  handler1->txn_->sendEOM();
  handler1->expectHeaders();
  handler1->expectBody();
  handler1->expectEOM();
  handler1->expectDetachTransaction();
  auto resp1 = makeResponse(200, 100);
  sendResponse(handler1->txn_->getID(),
               *std::get<0>(resp1),
               std::move(std::get<1>(resp1)),
               true);
  flushAndLoop();
  EXPECT_EQ(hqSession_->getHistoricalMaxOutgoingStreams(), 2);
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, ResponseTermedByFin) {
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM();
  handler->expectDetachTransaction();
  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setHTTPVersion(1, 0);
  // HTTP/1.0 response with no content-length, termed by tranport FIN
  sendResponse(handler->txn_->getID(), resp, makeBuf(100), true);
  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, WaitForReplaySafeCallback) {
  auto handler = openTransaction();
  StrictMock<folly::test::MockReplaySafetyCallback> cb1;
  StrictMock<folly::test::MockReplaySafetyCallback> cb2;
  StrictMock<folly::test::MockReplaySafetyCallback> cb3;

  auto sock = socketDriver_->getSocket();
  EXPECT_CALL(*sock, replaySafe()).WillRepeatedly(Return(false));
  handler->txn_->addWaitingForReplaySafety(&cb1);
  handler->txn_->addWaitingForReplaySafety(&cb2);
  handler->txn_->addWaitingForReplaySafety(&cb3);
  handler->txn_->removeWaitingForReplaySafety(&cb2);

  ON_CALL(*sock, replaySafe()).WillByDefault(Return(true));
  EXPECT_CALL(cb1, onReplaySafe_());
  EXPECT_CALL(cb3, onReplaySafe_());
  hqSession_->onReplaySafe();

  handler->expectDetachTransaction();
  handler->txn_->sendAbort();
  hqSession_->closeWhenIdle();
  eventBase_.loopOnce();
}

TEST_P(HQUpstreamSessionTest, AlreadyReplaySafe) {
  auto handler = openTransaction();

  StrictMock<folly::test::MockReplaySafetyCallback> cb;

  auto sock = socketDriver_->getSocket();
  EXPECT_CALL(*sock, replaySafe()).WillRepeatedly(Return(true));
  EXPECT_CALL(cb, onReplaySafe_());
  handler->txn_->addWaitingForReplaySafety(&cb);

  handler->expectDetachTransaction();
  handler->txn_->sendAbort();
  hqSession_->closeWhenIdle();
  eventBase_.loopOnce();
}

TEST_P(HQUpstreamSessionTest, Test100Continue) {
  InSequence enforceOrder;
  auto handler = openTransaction();
  auto req = getPostRequest(10);
  req.getHeaders().add(HTTP_HEADER_EXPECT, "100-continue");
  handler->txn_->sendHeaders(req);
  handler->txn_->sendEOM();
  handler->expectHeaders();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM();
  handler->expectDetachTransaction();
  sendResponse(handler->txn_->getID(), *makeResponse(100), nullptr, false);
  auto resp = makeResponse(200, 100);
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, TestSetIngressTimeoutAfterSendEom) {
  hqSession_->setIngressTimeoutAfterEom(true);

  // Send EOM separately.
  auto handler1 = openTransaction();
  handler1->expectHeaders();
  handler1->expectBody();
  handler1->expectEOM();
  handler1->expectDetachTransaction();

  auto transaction1 = handler1->txn_;
  EXPECT_TRUE(transaction1->hasIdleTimeout());
  transaction1->setIdleTimeout(std::chrono::milliseconds(100));
  EXPECT_FALSE(transaction1->isScheduled());

  transaction1->sendHeaders(getPostRequest(10));
  eventBase_.loopOnce();
  EXPECT_FALSE(transaction1->isScheduled());

  transaction1->sendBody(makeBuf(100) /* body */);
  eventBase_.loopOnce();
  EXPECT_FALSE(transaction1->isScheduled());

  transaction1->sendEOM();
  eventBase_.loopOnce();
  EXPECT_TRUE(transaction1->isScheduled());

  auto response1 = makeResponse(200, 100);
  sendResponse(transaction1->getID(),
               *std::get<0>(response1),
               std::move(std::get<1>(response1)),
               true);
  flushAndLoop();

  // Send EOM with header.
  auto handler2 = openTransaction();
  handler2->expectHeaders();
  handler2->expectBody();
  handler2->expectEOM();
  handler2->expectDetachTransaction();

  auto transaction2 = handler2->txn_;
  EXPECT_FALSE(transaction2->isScheduled());
  transaction2->sendHeadersWithOptionalEOM(getPostRequest(), true /* eom */);
  eventBase_.loopOnce();
  EXPECT_TRUE(transaction2->isScheduled());

  auto response2 = makeResponse(200, 100);
  sendResponse(transaction2->getID(),
               *std::get<0>(response2),
               std::move(std::get<1>(response2)),
               true);
  flushAndLoop();

  // Send EOM with body.
  auto handler3 = openTransaction();
  handler3->expectHeaders();
  handler3->expectBody();
  handler3->expectEOM();
  handler3->expectDetachTransaction();

  auto transaction3 = handler3->txn_;
  EXPECT_FALSE(transaction3->isScheduled());
  transaction3->sendHeaders(getPostRequest());
  eventBase_.loopOnce();
  EXPECT_FALSE(transaction3->isScheduled());
  transaction3->sendBody(makeBuf(100) /* body */);
  transaction3->sendEOM();
  eventBase_.loopOnce();
  EXPECT_TRUE(transaction3->isScheduled());

  auto response3 = makeResponse(200, 100);
  sendResponse(transaction3->getID(),
               *std::get<0>(response3),
               std::move(std::get<1>(response3)),
               true);
  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, GetAddresses) {
  EXPECT_EQ(socketDriver_->localAddress_, hqSession_->getLocalAddress());
  EXPECT_EQ(socketDriver_->peerAddress_, hqSession_->getPeerAddress());
  hqSession_->dropConnection();
}

TEST_P(HQUpstreamSessionTest, GetAddressesFromBase) {
  HTTPSessionBase* sessionBase = dynamic_cast<HTTPSessionBase*>(hqSession_);
  EXPECT_EQ(socketDriver_->localAddress_, sessionBase->getLocalAddress());
  EXPECT_EQ(socketDriver_->peerAddress_, sessionBase->getPeerAddress());
  hqSession_->dropConnection();
}

TEST_P(HQUpstreamSessionTest, GetAddressesAfterDropConnection) {
  HQSession::DestructorGuard dg(hqSession_);
  hqSession_->dropConnection();
  EXPECT_EQ(socketDriver_->localAddress_, hqSession_->getLocalAddress());
  EXPECT_EQ(socketDriver_->peerAddress_, hqSession_->getPeerAddress());
}

TEST_P(HQUpstreamSessionTest, DropConnectionTwice) {
  HQSession::DestructorGuard dg(hqSession_);
  hqSession_->closeWhenIdle();
  hqSession_->dropConnection();
}

TEST_P(HQUpstreamSessionTest, DropConnectionTwiceWithPendingStreams) {
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  socketDriver_->addReadEvent(15, writeBuf.move());
  flushAndLoopN(1);
  HQSession::DestructorGuard dg(hqSession_);
  hqSession_->dropConnection();
  eventBase_.loopOnce();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, DropConnectionAfterCloseWhenIdle) {
  HQSession::DestructorGuard dg(hqSession_);
  hqSession_->closeWhenIdle();
  flushAndLoopN(1);
  hqSession_->dropConnection();
}

TEST_P(HQUpstreamSessionTest, DropConnectionWithStreamAfterCloseWhenIdle) {
  HQSession::DestructorGuard dg(hqSession_);
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  hqSession_->closeWhenIdle();
  flushAndLoopN(1);
  handler->expectError([&](const HTTPException& err) {
    EXPECT_TRUE(err.hasProxygenError());
    EXPECT_EQ(err.getHttp3ErrorCode(), HTTP3::ErrorCode::HTTP_NO_ERROR);
  });
  handler->expectDetachTransaction();
  hqSession_->dropConnection();
}

TEST_P(HQUpstreamSessionTest, NotifyConnectCallbackBeforeDestruct) {
  MockConnectCallback connectCb;
  dynamic_cast<HQUpstreamSession*>(hqSession_)->setConnectCallback(&connectCb);
  EXPECT_CALL(connectCb, connectError(_)).Times(1);
  socketDriver_->deliverConnectionError(
      {quic::LocalErrorCode::CONNECT_FAILED, "Peer closed"});
}

TEST_P(HQUpstreamSessionTest, DropFromConnectError) {
  MockConnectCallback connectCb;
  HQUpstreamSession* upstreamSess =
      dynamic_cast<HQUpstreamSession*>(hqSession_);
  upstreamSess->setConnectCallback(&connectCb);
  EXPECT_CALL(connectCb, connectError(_)).WillOnce(InvokeWithoutArgs([&] {
    hqSession_->dropConnection();
  }));
  socketDriver_->addOnConnectionEndEvent(0);
  eventBase_.loop();
}

TEST_P(HQUpstreamSessionTest, FirstPeerPacketProcessed) {
  MockConnectCallback connectCb;
  HQUpstreamSession* upstreamSess =
      dynamic_cast<HQUpstreamSession*>(hqSession_);
  upstreamSess->setConnectCallback(&connectCb);
  EXPECT_CALL(connectCb, onFirstPeerPacketProcessed());
  upstreamSess->onFirstPeerPacketProcessed();

  upstreamSess->closeWhenIdle();
  eventBase_.loopOnce();
}

TEST_P(HQUpstreamSessionTest, NotifyReplaySafeAfterTransportReady) {
  MockConnectCallback connectCb;
  HQUpstreamSession* upstreamSess =
      dynamic_cast<HQUpstreamSession*>(hqSession_);
  upstreamSess->setConnectCallback(&connectCb);

  // onTransportReady gets called in SetUp() already

  EXPECT_CALL(connectCb, onReplaySafe());
  upstreamSess->onReplaySafe();

  upstreamSess->closeWhenIdle();
  eventBase_.loopOnce();
}

TEST_P(HQUpstreamSessionTest, TestConnectionToken) {
  HQSession::DestructorGuard dg(hqSession_);
  auto handler = openTransaction();
  handler->expectError();
  handler->expectDetachTransaction();

  // The transaction should not have a connection token
  // by default.
  EXPECT_EQ(handler->txn_->getConnectionToken(), folly::none);

  // Passing connection token to a session should
  // make it visible to the transaction.
  HTTPTransaction::ConnectionToken connToken{"TOKEN1234"};
  hqSession_->setConnectionToken(connToken);

  EXPECT_NE(handler->txn_->getConnectionToken(), folly::none);
  EXPECT_EQ(*handler->txn_->getConnectionToken(), connToken);

  // Clean up the session and the transaction.
  hqSession_->onConnectionError(
      quic::QuicError(quic::LocalErrorCode::CONNECT_FAILED,
                      "Connect Failure with Open streams"));
  eventBase_.loop();
  EXPECT_EQ(hqSession_->getConnectionCloseReason(),
            ConnectionCloseReason::SHUTDOWN);
}

TEST_P(HQUpstreamSessionTest, OnConnectionErrorWithOpenStreams) {
  HQSession::DestructorGuard dg(hqSession_);
  auto handler = openTransaction();
  handler->expectError();
  handler->expectDetachTransaction();
  hqSession_->onConnectionError(
      quic::QuicError(quic::LocalErrorCode::CONNECT_FAILED,
                      "Connect Failure with Open streams"));
  eventBase_.loop();
  EXPECT_EQ(hqSession_->getConnectionCloseReason(),
            ConnectionCloseReason::SHUTDOWN);
}

TEST_P(HQUpstreamSessionTest, OnConnectionErrorWithOpenStreamsPause) {
  HQSession::DestructorGuard dg(hqSession_);
  auto handler1 = openTransaction();
  auto handler2 = openTransaction();
  handler1->txn_->sendHeaders(getGetRequest());
  handler1->txn_->sendEOM();
  handler2->txn_->sendHeaders(getGetRequest());
  handler2->txn_->sendEOM();
  auto resp = makeResponse(200, 100);
  sendResponse(handler1->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  resp = makeResponse(200, 100);
  sendResponse(handler2->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  flush();
  eventBase_.runInLoop([&] {
    hqSession_->onConnectionError(
        quic::QuicError(quic::LocalErrorCode::CONNECT_FAILED,
                        "Connect Failure with Open streams"));
  });
  handler1->expectError(
      [&](const HTTPException&) { handler2->txn_->pauseIngress(); });
  handler1->expectDetachTransaction();
  handler2->expectError();
  handler2->expectDetachTransaction();
  eventBase_.loop();
  EXPECT_EQ(hqSession_->getConnectionCloseReason(),
            ConnectionCloseReason::SHUTDOWN);
}

TEST_P(HQUpstreamSessionTest, RejectDelegateSending) {
  auto handler = openTransaction();
  auto dsrRequestSender = std::make_unique<MockDSRRequestSender>();
  EXPECT_FALSE(handler->txn_->sendHeadersWithDelegate(
      getGetRequest(), std::move(dsrRequestSender)));
  handler->expectDetachTransaction();
  handler->terminate();
  eventBase_.loop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, GoawayStreamsUnacknowledged) {
  std::vector<std::unique_ptr<StrictMock<MockHTTPHandler>>> handlers;
  auto numStreams = 4;
  quic::StreamId goawayId = (numStreams * 4) / 2 + 4;
  for (auto n = 1; n <= numStreams; n++) {
    handlers.emplace_back(openTransaction());
    auto handler = handlers.back().get();
    handler->txn_->sendHeaders(getGetRequest());
    handler->txn_->sendEOM();
    EXPECT_CALL(*handler, _onGoaway(testing::_)).Times(2);
    if (handler->txn_->getID() >= goawayId) {
      handler->expectError([hdlr = handler](const HTTPException& err) {
        EXPECT_TRUE(err.hasProxygenError());
        EXPECT_EQ(err.getProxygenError(), kErrorStreamUnacknowledged);
        ASSERT_EQ(
            folly::to<std::string>("StreamUnacknowledged on transaction id: ",
                                   hdlr->txn_->getID()),
            std::string(err.what()));
      });
    } else {
      handler->expectHeaders();
      handler->expectBody();
      handler->expectEOM();
    }

    if (n < numStreams) {
      handler->expectDetachTransaction();
    } else {
      handler->expectDetachTransaction([&] {
        // Make sure the session can't create any more transactions.
        MockHTTPHandler handler2;
        EXPECT_EQ(hqSession_->newTransaction(&handler2), nullptr);
        // Send the responses for the acknowledged streams
        for (auto& hdlr : handlers) {
          auto id = hdlr->txn_->getID();
          if (id < goawayId) {
            auto resp = makeResponse(200, 100);
            sendResponse(
                id, *std::get<0>(resp), std::move(std::get<1>(resp)), true);
          }
        }
        flush();
      });
    }
  }

  sendGoaway(HTTPCodec::MaxStreamID, milliseconds(50));
  sendGoaway(goawayId, milliseconds(100));
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTest, GoawayIncreased) {
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  egressControlCodec_->generateGoaway(writeBuf, 12, ErrorCode::NO_ERROR);
  socketDriver_->addReadEvent(connControlStreamId_, writeBuf.move());
  flushAndLoopN(1);
  proxygen::hq::HQControlCodec egressControlCodec2(
      nextUnidirectionalStreamId_,
      proxygen::TransportDirection::DOWNSTREAM,
      proxygen::hq::StreamDirection::EGRESS,
      egressSettings_);
  egressControlCodec2.generateGoaway(writeBuf, 16, ErrorCode::NO_ERROR);
  socketDriver_->addReadEvent(connControlStreamId_, writeBuf.move());
  flushAndLoop();
  EXPECT_EQ(*socketDriver_->streams_[kConnectionStreamId].error,
            HTTP3::ErrorCode::HTTP_ID_ERROR);
}

TEST_P(HQUpstreamSessionTest, DelayedQPACK) {
  InSequence enforceOrder;
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  handler->expectHeaders();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM();
  handler->expectDetachTransaction();
  auto cont = makeResponse(100);
  auto resp = makeResponse(200, 100);
  cont->getHeaders().add("X-FB-Debug", "jvrbfihvuvvclgvfkbkikjlcbruleekj");
  std::get<0>(resp)->getHeaders().add("X-FB-Debug",
                                      "egedljtrbullljdjjvtjkekebffefclj");
  sendResponse(handler->txn_->getID(), *cont, nullptr, false);
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  auto control = encoderWriteBuf_.move();
  flushAndLoopN(1);
  encoderWriteBuf_.append(std::move(control));
  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, DelayedQPACKTimeout) {
  InSequence enforceOrder;
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  handler->expectError();
  auto resp = makeResponse(200, 100);
  std::get<0>(resp)->getHeaders().add("X-FB-Debug",
                                      "egedljtrbullljdjjvtjkekebffefclj");
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  auto control = encoderWriteBuf_.move();
  handler->expectDetachTransaction([this, &control]() mutable {
    // have the header block arrive after destruction
    encoderWriteBuf_.append(std::move(control));
    eventBase_.runInLoop([this] { flush(); });
    eventBase_.runAfterDelay([this] { hqSession_->closeWhenIdle(); }, 100);
  });
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTest, QPACKDecoderStreamFlushed) {
  InSequence enforceOrder;
  auto handler = openTransaction();
  handler->txn_->sendHeadersWithOptionalEOM(getGetRequest(), true);
  flushAndLoopN(1);
  handler->expectDetachTransaction();
  handler->txn_->sendAbort();
  flushAndLoop();
  auto& decoderStream = socketDriver_->streams_[kQPACKDecoderEgressStreamId];
  // type byte plus cancel
  EXPECT_EQ(decoderStream.writeBuf.chainLength(), 2);

  handler = openTransaction();
  handler->txn_->sendHeadersWithOptionalEOM(getGetRequest(), true);
  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM();
  auto resp = makeResponse(200, 100);
  std::get<0>(resp)->getHeaders().add("Response", "Dynamic");
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  auto qpackData = encoderWriteBuf_.move();
  flushAndLoopN(1);
  encoderWriteBuf_.append(std::move(qpackData));
  handler->expectDetachTransaction();
  hqSession_->closeWhenIdle();
  flushAndLoop();
  // type byte plus cancel plus ack
  EXPECT_EQ(decoderStream.writeBuf.chainLength(), 3);
}

TEST_P(HQUpstreamSessionTest, DelayedQPACKAfterReset) {
  // Stand on your head and spit wooden nickels
  // Ensure the session does not deliver input data to a transaction detached
  // earlier the same loop
  InSequence enforceOrder;
  // Send two requests
  auto handler1 = openTransaction();
  auto handler2 = openTransaction();
  handler1->txn_->sendHeadersWithOptionalEOM(getGetRequest(), true);
  handler2->txn_->sendHeadersWithOptionalEOM(getGetRequest(), true);
  // Send a response to txn1 that will block on QPACK data
  auto resp1 = makeResponse(302, 0);
  std::get<0>(resp1)->getHeaders().add("Response1", "Dynamic");
  sendResponse(handler1->txn_->getID(),
               *std::get<0>(resp1),
               std::move(std::get<1>(resp1)),
               true);
  // Save first QPACK data
  auto qpackData1 = encoderWriteBuf_.move();
  // Send response to txn2 that will block on *different* QPACK data
  auto resp2 = makeResponse(302, 0);
  std::get<0>(resp2)->getHeaders().add("Respnse2", "Dynamic");
  sendResponse(handler2->txn_->getID(),
               *std::get<0>(resp2),
               std::move(std::get<1>(resp2)),
               false);
  // Save second QPACK data
  auto qpackData2 = encoderWriteBuf_.move();

  // Abort *both* txns when txn1 gets headers.  This will leave txn2 detached
  // with pending input data in this loop.
  handler1->expectHeaders([&] {
    handler1->txn_->sendAbort();
    handler2->txn_->sendAbort();
  });

  auto streamIt1 = streams_.find(handler1->txn_->getID());
  CHECK(streamIt1 != streams_.end());
  auto streamIt2 = streams_.find(handler2->txn_->getID());
  CHECK(streamIt2 != streams_.end());
  // add all the events in the same callback, with the stream data coming
  // before the QPACK data
  std::vector<MockQuicSocketDriver::ReadEvent> events;
  events.emplace_back(handler2->txn_->getID(),
                      streamIt2->second.buf.move(),
                      streamIt2->second.readEOF,
                      folly::none,
                      false);
  events.emplace_back(handler1->txn_->getID(),
                      streamIt1->second.buf.move(),
                      streamIt1->second.readEOF,
                      folly::none,
                      false);
  events.emplace_back(kQPACKEncoderIngressStreamId,
                      std::move(qpackData1),
                      false,
                      folly::none,
                      false);
  socketDriver_->addReadEvents(std::move(events));
  handler2->expectDetachTransaction();
  handler1->expectDetachTransaction();
  eventBase_.loopOnce();
  // Add the QPACK data that would unblock txn2.  It's long gone and this
  // should be a no-op.
  socketDriver_->addReadEvent(kQPACKEncoderIngressStreamId,
                              std::move(qpackData2));
  eventBase_.loopOnce();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTestQPACK, QPACKQueuedOnClose) {
  InSequence enforceOrder;
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  handler->expectError();
  handler->expectDetachTransaction();
  auto resp = makeResponse(200, 100);
  std::get<0>(resp)->getHeaders().add("X-FB-Debug",
                                      "egedljtrbullljdjjvtjkekebffefclj");
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  auto control = encoderWriteBuf_.move();
  // Entire response is delivered from the transport to the session
  flushAndLoopN(1);
  // Connection end but the stream is still pending
  socketDriver_->addOnConnectionEndEvent(0);
  eventBase_.loop();
}

TEST_P(HQUpstreamSessionTest, TestDropConnectionSynchronously) {
  std::unique_ptr<testing::NiceMock<proxygen::MockHTTPSessionInfoCallback>>
      infoCb = std::make_unique<
          testing::NiceMock<proxygen::MockHTTPSessionInfoCallback>>();
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->expectError();
  handler->expectDetachTransaction();
  hqSession_->setInfoCallback(infoCb.get());
  // the session is destroyed synchronously, so the destroy callback gets
  // invoked
  EXPECT_CALL(*infoCb.get(), onDestroy(_)).Times(1);
  hqSession_->dropConnection();
  infoCb.reset();
  eventBase_.loopOnce();
}

TEST_P(HQUpstreamSessionTest, TestOnStopSendingHTTPRequestRejected) {
  auto handler = openTransaction();
  auto streamId = handler->txn_->getID();
  handler->txn_->sendHeaders(getGetRequest());
  eventBase_.loopOnce();
  EXPECT_CALL(*socketDriver_->getSocket(),
              resetStream(streamId, HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED))
      .Times(2) // once from on stopSending and once from sendAbort
      .WillRepeatedly(
          Invoke([&](quic::StreamId id, quic::ApplicationErrorCode) {
            // setWriteError will cancaleDeliveryCallbacks which will invoke
            // onCanceled to decrementPendingByteEvents on the txn.
            socketDriver_->setWriteError(id);
            return folly::unit;
          }));
  EXPECT_CALL(*handler, _onError(_))
      .Times(1)
      .WillOnce(Invoke([](HTTPException ex) {
        EXPECT_EQ(kErrorStreamUnacknowledged, ex.getProxygenError());
      }));
  handler->expectDetachTransaction();
  hqSession_->onStopSending(streamId, HTTP3::ErrorCode::HTTP_REQUEST_REJECTED);
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, TestGreaseFramePerSession) {
  // a grease frame is created when creating the first transaction
  auto handler1 = openTransaction();
  auto streamId1 = handler1->txn_->getID();
  handler1->txn_->sendHeaders(getGetRequest());
  handler1->txn_->sendEOM();
  handler1->expectHeaders();
  handler1->expectBody();
  handler1->expectEOM();
  handler1->expectDetachTransaction();
  auto resp1 = makeResponse(200, 100);
  sendResponse(handler1->txn_->getID(),
               *std::get<0>(resp1),
               std::move(std::get<1>(resp1)),
               true);
  flushAndLoop();
  FakeHTTPCodecCallback callback1;
  std::unique_ptr<HQStreamCodec> downstreamCodec =
      std::make_unique<hq::HQStreamCodec>(
          streamId1,
          TransportDirection::DOWNSTREAM,
          qpackCodec_,
          encoderWriteBuf_,
          decoderWriteBuf_,
          [] { return std::numeric_limits<uint64_t>::max(); },
          ingressSettings_);
  downstreamCodec->setCallback(&callback1);
  downstreamCodec->onIngress(
      *socketDriver_->streams_[streamId1].writeBuf.front());
  EXPECT_EQ(callback1.unknownFrames, 1);
  EXPECT_EQ(callback1.greaseFrames, 1);

  // no grease frame is created when creating the second transaction
  auto handler2 = openTransaction();
  auto streamId2 = handler2->txn_->getID();
  handler2->txn_->sendHeaders(getGetRequest());
  handler2->txn_->sendEOM();
  handler2->expectHeaders();
  handler2->expectBody();
  handler2->expectEOM();
  handler2->expectDetachTransaction();
  auto resp2 = makeResponse(200, 100);
  sendResponse(handler2->txn_->getID(),
               *std::get<0>(resp2),
               std::move(std::get<1>(resp2)),
               true);
  flushAndLoop();
  FakeHTTPCodecCallback callback2;
  downstreamCodec->setCallback(&callback2);
  downstreamCodec->onIngress(
      *socketDriver_->streams_[streamId2].writeBuf.front());
  EXPECT_EQ(callback2.unknownFrames, 0);
  EXPECT_EQ(callback2.greaseFrames, 0);
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTest, TestIngressLimitedSessionWithNewRequests) {
  auto infoCb = std::make_unique<
      testing::NiceMock<proxygen::MockHTTPSessionInfoCallback>>();
  hqSession_->setInfoCallback(infoCb.get());

  // set limit to 5,000 for this test
  hqSession_->setReadBufferLimit(5000);

  // create first stream and generate large response exceeding ingress limit
  EXPECT_CALL(*infoCb, onIngressLimitExceeded(_)).Times(1);
  auto handler1 = openTransaction();
  auto streamId1 = handler1->txn_->getID();
  handler1->txn_->sendHeaders(getGetRequest());
  handler1->txn_->sendEOM();
  // pause ingress so we don't invoke notifyIngressBodyProcessed and decrement
  // the session's pendingBodySize_
  handler1->txn_->pauseIngress();
  handler1->expectHeaders();
  handler1->expectBody();

  auto resp1 = makeResponse(200, 6000);
  sendResponse(
      streamId1, *std::get<0>(resp1), std::move(std::get<1>(resp1)), true);
  flushAndLoop();

  EXPECT_CALL(*socketDriver_->getSocket(), pauseRead(_));
  auto handler2 = openTransaction();
  handler2->expectDetachTransaction();
  handler2->txn_->sendAbort();

  // resume first txn and expect callbacks
  handler1->expectEOM();
  handler1->expectDetachTransaction();
  handler1->txn_->resumeIngress();

  flushAndLoop();
  hqSession_->setInfoCallback(nullptr);
  hqSession_->closeWhenIdle();
}

//   - in HQ we already have sent SETTINGS in SetUp, so tests that multiple
//     setting frames are not allowed
TEST_P(HQUpstreamSessionTest, ExtraSettings) {
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  handler->expectError();
  handler->expectDetachTransaction();

  // Need to use a new codec. Since generating settings twice is
  // forbidden
  HQControlCodec auxControlCodec_{nextUnidirectionalStreamId_,
                                  TransportDirection::DOWNSTREAM,
                                  StreamDirection::EGRESS,
                                  egressSettings_,
                                  UnidirectionalStreamType::CONTROL};
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  auxControlCodec_.generateSettings(writeBuf);
  socketDriver_->addReadEvent(
      connControlStreamId_, writeBuf.move(), milliseconds(0));

  flushAndLoop();

  EXPECT_EQ(*socketDriver_->streams_[kConnectionStreamId].error,
            HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED);
}

TEST_P(HQUpstreamSessionTest, Observer_Attach_Detach_Destroyed) {

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

TEST_P(HQUpstreamSessionTest, Observer_RequestStarted) {
  // Add an observer NOT subscribed to the RequestStarted event
  auto observerUnsubscribed =
      addMockSessionObserver(MockSessionObserver::EventSetBuilder().build());
  hqSession_->addObserver(observerUnsubscribed.get());

  // Add an observer subscribed to this event
  auto observerSubscribed = addMockSessionObserver(
      MockSessionObserver::EventSetBuilder()
          .enable(HTTPSessionObserverInterface::Events::requestStarted)
          .build());
  hqSession_->addObserver(observerSubscribed.get());

  // expect to see a request started with header 'x-meta-test-header' having
  // value 'abc123'
  EXPECT_CALL(*observerSubscribed, requestStarted(_, _))
      .WillOnce(Invoke(
          [](HTTPSessionObserverAccessor*,
             const proxygen::MockSessionObserver::RequestStartedEvent& event) {
            auto hdrs = event.requestHeaders;
            EXPECT_EQ(hdrs.getSingleOrEmpty("x-meta-test-header"), "abc123");
          }));

  auto handler = openTransaction();
  HTTPMessage req = getGetRequest();
  req.getHeaders().add("x-meta-test-header", "abc123");
  handler->txn_->sendHeaders(req);
  handler->txn_->sendEOM();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM();
  handler->expectDetachTransaction();
  auto resp = makeResponse(200, 100);
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  flushAndLoop();
  hqSession_->closeWhenIdle();
}

// Test Cases for which Settings are not sent in the test SetUp
using HQUpstreamSessionTestNoSettings = HQUpstreamSessionTest;

INSTANTIATE_TEST_SUITE_P(HQUpstreamSessionTest,
                         HQUpstreamSessionTestNoSettings,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           tp.shouldSendSettings_ = false;
                           return tp;
                         }()),
                         paramsToTestName);
TEST_P(HQUpstreamSessionTestNoSettings, SimpleGet) {
  EXPECT_CALL(connectCb_, connectError(_)).Times(1);
  socketDriver_->deliverConnectionError(
      {quic::LocalErrorCode::CONNECT_FAILED, "Peer closed"});
}

TEST_P(HQUpstreamSessionTestNoSettings, GoawayBeforeSettings) {
  auto handler = openTransaction();
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  handler->expectError();
  handler->expectDetachTransaction();

  sendGoaway(HTTPCodec::MaxStreamID);
  flushAndLoop();

  EXPECT_EQ(*socketDriver_->streams_[kConnectionStreamId].error,
            HTTP3::ErrorCode::HTTP_MISSING_SETTINGS);
}

/**
 * Push tests
 */

class HQUpstreamSessionTestPush : public HQUpstreamSessionTest {
 public:
  void SetUp() override {
    HQUpstreamSessionTest::SetUp();
    SetUpAssocHandler();
    nextPushId_ = kInitialPushId;
    lastPushPromiseHeadersSize_.compressed = 0;
    lastPushPromiseHeadersSize_.uncompressed = 0;
  }

  void SetUpAssocHandler() {
    // Create the primary request
    assocHandler_ = openTransaction();
    assocHandler_->txn_->sendHeaders(getGetRequest());
    assocHandler_->expectDetachTransaction();
  }

  void TearDown() override {
    HQUpstreamSessionTest::TearDown();
  }

  void SetUpServerPushLifecycleCallbacks() {
    if (!SLCcallback_) {
      SLCcallback_ = std::make_unique<MockServerPushLifecycleCallback>();
      hqSession_->setServerPushLifecycleCallback(SLCcallback_.get());
    }
  }

  hq::PushId nextPushId() {
    auto id = nextPushId_;
    nextPushId_ += kPushIdIncrement;
    return id;
  }

  // NOTE: Using odd numbers for push ids, to allow detecting
  // subtle bugs where streamID and pushID are quietly misplaced
  bool isPushIdValid(hq::PushId pushId) {
    return (pushId % 2) == 1;
  }

  using WriteFunctor =
      std::function<folly::Optional<size_t>(folly::IOBufQueue&)>;
  folly::Optional<size_t> writeUpTo(quic::StreamId id,
                                    size_t maxlen,
                                    WriteFunctor functor) {
    // Lookup the stream
    auto findRes = streams_.find(id);
    if (findRes == streams_.end()) {
      return folly::none;
    }

    folly::IOBufQueue tmpbuf{folly::IOBufQueue::cacheChainLength()};
    auto funcres = functor(tmpbuf);
    if (!funcres) {
      return folly::none;
    }

    auto eventbuf = tmpbuf.splitAtMost(maxlen);
    auto wlen = eventbuf->length();
    CHECK_LE(wlen, maxlen) << "The written len must not exceed the max len";
    socketDriver_->addReadEvent(id, std::move(eventbuf), milliseconds(0));
    return wlen;
  }

  // Use the common facilities to write the quic integer
  folly::Optional<size_t> writePushStreamPreface(quic::StreamId id,
                                                 size_t maxlen) {
    WriteFunctor f = [](folly::IOBufQueue& outbuf) {
      return generateStreamPreface(outbuf, hq::UnidirectionalStreamType::PUSH);
    };

    auto res = writeUpTo(id, maxlen, f);
    return res;
  }

  folly::Optional<size_t> writeUnframedPushId(quic::StreamId id,
                                              size_t maxlen,
                                              hq::PushId pushId) {
    WriteFunctor f = [=](folly::IOBufQueue& outbuf) -> folly::Optional<size_t> {
      folly::io::QueueAppender appender(&outbuf, 8);
      uint8_t size = 1 << (folly::Random::rand32() % 4);
      auto wlen = encodeQuicIntegerWithAtLeast(pushId, size, appender);
      CHECK_GE(wlen, size);
      return wlen;
    };

    auto res = writeUpTo(id, maxlen, f);
    return res;
  }

  void expectPushPromiseBegin(
      std::function<void(HTTPCodec::StreamID, hq::PushId)> callback =
          std::function<void(HTTPCodec::StreamID, hq::PushId)>()) {
    SetUpServerPushLifecycleCallbacks();
    SLCcallback_->expectPushPromiseBegin(callback);
  }

  void expectPushPromise(
      std::function<void(HTTPCodec::StreamID, hq::PushId, HTTPMessage*)>
          callback = std::function<
              void(HTTPCodec::StreamID, hq::PushId, HTTPMessage*)>()) {
    SetUpServerPushLifecycleCallbacks();
    SLCcallback_->expectPushPromise(callback);
  }

  void expectNascentPushStreamBegin(
      std::function<void(HTTPCodec::StreamID, bool)> callback =
          std::function<void(HTTPCodec::StreamID, bool)>()) {
    SetUpServerPushLifecycleCallbacks();
    SLCcallback_->expectNascentPushStreamBegin(callback);
  }

  void expectNascentPushStream(
      std::function<void(HTTPCodec::StreamID, hq::PushId, bool)> callback =
          std::function<void(HTTPCodec::StreamID, hq::PushId, bool)>()) {
    SetUpServerPushLifecycleCallbacks();
    SLCcallback_->expectNascentPushStream(callback);
  }

  void expectNascentEof(
      std::function<void(HTTPCodec::StreamID, folly::Optional<hq::PushId>)>
          callback = std::function<void(HTTPCodec::StreamID,
                                        folly::Optional<hq::PushId>)>()) {
    SetUpServerPushLifecycleCallbacks();
    SLCcallback_->expectNascentEof(callback);
  }

  void expectOrphanedNascentStream(
      std::function<void(HTTPCodec::StreamID, folly::Optional<hq::PushId>)>
          callback = std::function<void(HTTPCodec::StreamID,
                                        folly::Optional<hq::PushId>)>()) {

    SetUpServerPushLifecycleCallbacks();
    SLCcallback_->expectOrphanedNascentStream(callback);
  }

  void expectHalfOpenPushedTxn(
      std::function<
          void(const HTTPTransaction*, hq::PushId, HTTPCodec::StreamID, bool)>
          callback = std::function<void(const HTTPTransaction*,
                                        hq::PushId,
                                        HTTPCodec::StreamID,
                                        bool)>()) {
    SetUpServerPushLifecycleCallbacks();
    SLCcallback_->expectHalfOpenPushedTxn(callback);
  }

  void expectPushedTxn(std::function<void(const HTTPTransaction*,
                                          HTTPCodec::StreamID,
                                          hq::PushId,
                                          HTTPCodec::StreamID,
                                          bool)> callback =
                           std::function<void(const HTTPTransaction*,
                                              HTTPCodec::StreamID,
                                              hq::PushId,
                                              HTTPCodec::StreamID,
                                              bool)>()) {
    SetUpServerPushLifecycleCallbacks();
    SLCcallback_->expectPushedTxn(callback);
  }

  void expectPushedTxnTimeout(
      std::function<void(const HTTPTransaction*)> callback =
          std::function<void(const HTTPTransaction*)>()) {
    SetUpServerPushLifecycleCallbacks();
    SLCcallback_->expectPushedTxnTimeout(callback);
  }

  void expectOrphanedHalfOpenPushedTxn(
      std::function<void(const HTTPTransaction*)> callback =
          std::function<void(const HTTPTransaction*)>()) {
    SetUpServerPushLifecycleCallbacks();
    SLCcallback_->expectOrphanedHalfOpenPushedTxn(callback);
  }

  void sendPushPromise(quic::StreamId streamId,
                       hq::PushId pushId = kUnknownPushId,
                       const std::string& url = "/",
                       proxygen::HTTPHeaderSize* outHeaderSize = nullptr,
                       bool eom = false) {
    auto promise = getGetRequest(url);
    promise.setURL(url);

    return sendPushPromise(streamId, promise, pushId, outHeaderSize, eom);
  }

  void sendPushPromise(quic::StreamId streamId,
                       const HTTPMessage& promiseHeadersBlock,
                       hq::PushId pushId = kUnknownPushId,
                       proxygen::HTTPHeaderSize* outHeaderSize = nullptr,
                       bool eom = false) {

    // In case the user is not interested in knowing the size
    // of headers, but just in the fact that the headers were
    // written, use a temporary size for checks
    if (outHeaderSize == nullptr) {
      outHeaderSize = &lastPushPromiseHeadersSize_;
    }

    if (pushId == kUnknownPushId) {
      pushId = nextPushId();
    }

    auto c = makeCodec(streamId);
    auto res =
        streams_.emplace(std::piecewise_construct,
                         std::forward_as_tuple(streamId),
                         std::forward_as_tuple(c.first, std::move(c.second)));

    auto& pushPromiseRequest = res.first->second;
    pushPromiseRequest.id = streamId;

    // Push promises should not have EOF set.
    pushPromiseRequest.readEOF = eom;

    // Write the push promise to the request buffer.
    // The push promise includes the headers
    pushPromiseRequest.codec->generatePushPromise(pushPromiseRequest.buf,
                                                  streamId,
                                                  promiseHeadersBlock,
                                                  pushId,
                                                  eom,
                                                  outHeaderSize);
  }

  // Shared implementation for different push stream
  // methods
  ServerStream& createPushStreamImpl(quic::StreamId streamId,
                                     folly::Optional<hq::PushId> pushId,
                                     std::size_t len = kUnlimited,
                                     bool eom = true) {

    auto c = makeCodec(streamId);
    // Setting a push id allows us to send push preface
    auto res = streams_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(streamId),
        std::forward_as_tuple(c.first, std::move(c.second), pushId));

    auto& stream = res.first->second;
    stream.id = stream.codec->createStream();
    stream.readEOF = eom;

    // Generate the push stream preface, and if there's enough headroom
    // the unframed push id that follows it
    auto prefaceRes = writePushStreamPreface(stream.id, len);
    if (pushId.has_value()) {
      if (prefaceRes) {
        len -= *prefaceRes;
        writeUnframedPushId(stream.id, len, *pushId);
      }
    }

    return stream;
  }

  // Create a push stream with a header block and body
  void createPushStream(quic::StreamId streamId,
                        hq::PushId pushId,
                        const HTTPMessage& resp,
                        std::unique_ptr<folly::IOBuf> body = nullptr,
                        bool eom = true) {

    auto& stream = createPushStreamImpl(streamId, pushId, kUnlimited, eom);

    // Write the response
    stream.codec->generateHeader(
        stream.buf, stream.codecId, resp, body == nullptr ? eom : false);
    if (body) {
      stream.codec->generateBody(
          stream.buf, stream.codecId, std::move(body), folly::none, eom);
    }
  }

  // Convenience method for creating a push stream without the
  // need to allocate transport stream id
  void createPushStream(hq::PushId pushId,
                        const HTTPMessage& resp,
                        std::unique_ptr<folly::IOBuf> body = nullptr,
                        bool eom = true) {
    return createPushStream(
        nextUnidirectionalStreamId(), pushId, resp, std::move(body), eom);
  }

  // Create nascent stream (no body)
  void createNascentPushStream(quic::StreamId streamId,
                               folly::Optional<hq::PushId> pushId,
                               std::size_t len = kUnlimited,
                               bool eom = true) {
    createPushStreamImpl(streamId, pushId, len, eom);
  }

  bool lastPushPromiseHeadersSizeValid() {
    return ((lastPushPromiseHeadersSize_.uncompressed > 0) &&
            (lastPushPromiseHeadersSize_.compressed > 0));
  }

  void createNascentPushStream(hq::PushId pushId,
                               std::size_t prefaceBytes = kUnlimited,
                               bool eom = true) {
    return createNascentPushStream(
        nextUnidirectionalStreamId(), pushId, prefaceBytes, eom);
  }

  std::unique_ptr<MockHTTPHandler> expectPushResponse() {
    auto pushHandler = std::make_unique<MockHTTPHandler>();
    pushHandler->expectTransaction();
    assocHandler_->expectPushedTransaction(pushHandler.get());
    // Promise/Response - with no lambda it lacks RetiresOnSaturation
    pushHandler->expectHeaders([](std::shared_ptr<HTTPMessage>) {});
    pushHandler->expectHeaders([](std::shared_ptr<HTTPMessage>) {});
    pushHandler->expectBody();
    pushHandler->expectEOM();
    pushHandler->expectDetachTransaction();
    return pushHandler;
  }

  proxygen::HTTPHeaderSize lastPushPromiseHeadersSize_;
  hq::PushId nextPushId_;
  std::unique_ptr<StrictMock<MockHTTPHandler>> assocHandler_;

  std::unique_ptr<MockServerPushLifecycleCallback> SLCcallback_;
};

TEST_P(HQUpstreamSessionTestPush, DelayedQPACKPush) {
  assocHandler_->txn_->sendAbort();
  assocHandler_ = openTransaction();
  assocHandler_->txn_->sendHeaders(getGetRequest());
  assocHandler_->txn_->sendEOM();
  assocHandler_->expectHeaders();
  assocHandler_->expectBody();
  auto pushHandler = expectPushResponse();
  assocHandler_->expectEOM();
  assocHandler_->expectDetachTransaction();

  auto resp = makeResponse(200, 100);
  sendResponse(assocHandler_->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               false);
  flushAndLoopN(1);
  auto pushPromiseRequest = getGetRequest();
  pushPromiseRequest.getHeaders().set("Dynamic1", "a");
  hq::PushId pushId = nextPushId();
  sendPushPromise(assocHandler_->txn_->getID(), pushPromiseRequest, pushId);
  sendPartialBody(assocHandler_->txn_->getID(), nullptr, true);

  auto control = encoderWriteBuf_.move();
  flushAndLoopN(1);

  encoderWriteBuf_.append(std::move(control));
  flushAndLoopN(1);

  HTTPMessage pushResp;
  pushResp.setStatusCode(200);
  pushResp.getHeaders().set("Dynamic2", "b");
  createPushStream(pushId, pushResp, makeBuf(100), true);

  control = encoderWriteBuf_.move();
  flushAndLoopN(1);

  encoderWriteBuf_.append(std::move(control));
  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTestPush, TestPushPromiseCallbacksInvoked) {
  // the push promise is not followed by a push stream, and the eof is not
  // set.
  // The transaction is supposed to stay open and to time out eventually.
  assocHandler_->expectError([&](const HTTPException& ex) {
    ASSERT_EQ(ex.getProxygenError(), kErrorTimeout);
  });

  hq::PushId pushId = nextPushId();

  auto pushPromiseRequest = getGetRequest();

  expectPushPromiseBegin(
      [&](HTTPCodec::StreamID owningStreamId, hq::PushId promisedPushId) {
        EXPECT_EQ(promisedPushId, pushId);
        EXPECT_EQ(owningStreamId, assocHandler_->txn_->getID());
      });

  expectPushPromise([&](HTTPCodec::StreamID owningStreamId,
                        hq::PushId promisedPushId,
                        HTTPMessage* msg) {
    EXPECT_EQ(promisedPushId, pushId);
    EXPECT_EQ(owningStreamId, assocHandler_->txn_->getID());

    EXPECT_THAT(msg, NotNull());

    auto expectedHeaders = pushPromiseRequest.getHeaders();
    auto actualHeaders = msg->getHeaders();

    expectedHeaders.forEach(
        [&](const std::string& header, const std::string& /* val */) {
          EXPECT_TRUE(actualHeaders.exists(header));
          EXPECT_EQ(expectedHeaders.getNumberOfValues(header),
                    actualHeaders.getNumberOfValues(header));
        });
  });

  HTTPCodec::StreamID nascentStreamId;

  expectNascentPushStreamBegin([&](HTTPCodec::StreamID streamId, bool isEOF) {
    nascentStreamId = streamId;
    EXPECT_FALSE(isEOF);
  });

  expectNascentPushStream([&](HTTPCodec::StreamID pushStreamId,
                              hq::PushId pushStreamPushId,
                              bool /* isEOF */) {
    EXPECT_EQ(pushStreamPushId, pushId);
    EXPECT_EQ(pushStreamId, nascentStreamId);
  });

  sendPushPromise(assocHandler_->txn_->getID(), pushPromiseRequest, pushId);
  EXPECT_TRUE(lastPushPromiseHeadersSizeValid());

  HTTPMessage resp;
  resp.setStatusCode(200);
  createPushStream(pushId, resp, makeBuf(100), true);

  assocHandler_->txn_->sendEOM();

  auto pushHandler = expectPushResponse();

  hqSession_->closeWhenIdle();
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTestPush, TestIngressPushStream) {

  hq::PushId pushId = nextPushId();

  auto pushPromiseRequest = getGetRequest();

  HTTPCodec::StreamID nascentStreamId;

  expectNascentPushStreamBegin([&](HTTPCodec::StreamID streamId, bool isEOF) {
    nascentStreamId = streamId;
    EXPECT_FALSE(isEOF);
  });

  expectNascentPushStream([&](HTTPCodec::StreamID streamId,
                              hq::PushId pushStreamPushId,
                              bool isEOF) {
    EXPECT_EQ(streamId, nascentStreamId);
    EXPECT_EQ(pushId, pushStreamPushId);
    EXPECT_EQ(isEOF, false);
  });

  // Since push promise is not sent, full ingress push stream
  // not going to be created
  /*
    expectOrphanedNascentStream([&](HTTPCodec::StreamID streamId,
                                    folly::Optional<hq::PushId> maybePushId) {
      ASSERT_EQ(streamId, nascentStreamId);
      EXPECT_EQ(maybePushId.has_value(), true);
      EXPECT_EQ(maybePushId.value(), pushId);
    });
  */
  HTTPMessage resp;
  resp.setStatusCode(200);
  createPushStream(pushId, resp, makeBuf(100), true);

  // Currently, the new transaction is not created corectly,
  // and an error is expected. to be extended in the following
  // diffs which add creation of pushed transaction
  assocHandler_->expectError();

  assocHandler_->txn_->sendEOM();
  hqSession_->closeWhenIdle();
  flushAndLoop(); // One read for the letter, one read for quic integer. Is
                  // enough?
}

TEST_P(HQUpstreamSessionTestPush, TestPushPromiseFollowedByPushStream) {
  // the transaction is expected to timeout, since the PushPromise does not have
  // EOF set, and it is not followed by a PushStream.
  assocHandler_->expectError();

  hq::PushId pushId = nextPushId();

  auto pushPromiseRequest = getGetRequest();

  expectPushPromiseBegin(
      [&](HTTPCodec::StreamID owningStreamId, hq::PushId promisedPushId) {
        EXPECT_EQ(promisedPushId, pushId);
        EXPECT_EQ(owningStreamId, assocHandler_->txn_->getID());
      });

  expectPushPromise([&](HTTPCodec::StreamID owningStreamId,
                        hq::PushId promisedPushId,
                        HTTPMessage* msg) {
    EXPECT_EQ(promisedPushId, pushId);
    EXPECT_EQ(owningStreamId, assocHandler_->txn_->getID());

    EXPECT_THAT(msg, NotNull());

    auto expectedHeaders = pushPromiseRequest.getHeaders();
    auto actualHeaders = msg->getHeaders();

    expectedHeaders.forEach(
        [&](const std::string& header, const std::string& /* val */) {
          EXPECT_TRUE(actualHeaders.exists(header));
          EXPECT_EQ(expectedHeaders.getNumberOfValues(header),
                    actualHeaders.getNumberOfValues(header));
        });
  });

  HTTPCodec::StreamID nascentStreamId;

  expectNascentPushStreamBegin([&](HTTPCodec::StreamID streamId, bool isEOF) {
    nascentStreamId = streamId;
    folly::Optional<HTTPCodec::StreamID> expectedReadId(nascentStreamId);
    EXPECT_CALL(infoCb_, onRead(testing::_, testing::_, expectedReadId))
        .Times(testing::AtLeast(1));
    EXPECT_FALSE(isEOF);
  });

  // since push stream arrives after the promise,
  // full ingress push stream has to be created
  expectNascentPushStream([&](HTTPCodec::StreamID pushStreamId,
                              hq::PushId pushStreamPushId,
                              bool /* isEOF */) {
    EXPECT_EQ(pushStreamPushId, pushId);
    EXPECT_EQ(pushStreamId, nascentStreamId);
  });

  proxygen::HTTPHeaderSize pushPromiseSize;

  sendPushPromise(assocHandler_->txn_->getID(),
                  pushPromiseRequest,
                  pushId,
                  &pushPromiseSize);
  HTTPMessage resp;
  resp.setStatusCode(200);
  createPushStream(pushId, resp, makeBuf(100), true);

  assocHandler_->txn_->sendEOM();

  auto pushHandler = expectPushResponse();

  hqSession_->closeWhenIdle();
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTestPush, TestAbortedPushedTransactionAfterPromise) {
  assocHandler_->txn_->sendAbort();
  assocHandler_ = openTransaction();
  assocHandler_->txn_->sendHeaders(getGetRequest());
  assocHandler_->txn_->sendEOM();
  assocHandler_->expectHeaders();
  assocHandler_->expectBody();
  assocHandler_->expectEOM();
  assocHandler_->expectDetachTransaction();

  auto resp = makeResponse(200, 100);
  sendResponse(assocHandler_->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               false);
  flushAndLoopN(1);

  auto pushHandler = std::make_unique<MockHTTPHandler>();
  pushHandler->expectTransaction();
  assocHandler_->expectPushedTransaction(pushHandler.get());
  // Abort the pushed transaction upon reception of the push promise.
  pushHandler->expectHeaders(
      [&](std::shared_ptr<HTTPMessage>) { pushHandler->txn_->sendAbort(); });

  auto pushPromiseRequest = getGetRequest();
  hq::PushId pushId = nextPushId();
  sendPushPromise(assocHandler_->txn_->getID(), pushPromiseRequest, pushId);
  // Send body to close the main request stream
  sendPartialBody(assocHandler_->txn_->getID(), nullptr, true);
  pushHandler->expectDetachTransaction();

  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTestPush, TestAbortedPushedTransactionAfterResponse) {
  assocHandler_->txn_->sendAbort();
  assocHandler_ = openTransaction();
  assocHandler_->txn_->sendHeaders(getGetRequest());
  assocHandler_->txn_->sendEOM();
  assocHandler_->expectHeaders();
  assocHandler_->expectBody();
  assocHandler_->expectEOM();
  assocHandler_->expectDetachTransaction();

  auto resp = makeResponse(200, 100);
  sendResponse(assocHandler_->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               false);
  flushAndLoopN(1);

  auto pushHandler = std::make_unique<MockHTTPHandler>();
  pushHandler->expectTransaction();
  assocHandler_->expectPushedTransaction(pushHandler.get());
  // Expect normal promise.
  pushHandler->expectHeaders([](std::shared_ptr<HTTPMessage>) {});

  auto pushPromiseRequest = getGetRequest();
  pushPromiseRequest.getHeaders().set("Dynamic1", "a");
  hq::PushId pushId = nextPushId();
  sendPushPromise(assocHandler_->txn_->getID(), pushPromiseRequest, pushId);
  sendPartialBody(assocHandler_->txn_->getID(), nullptr, true);
  flushAndLoopN(1);

  // Abort the pushed transaction on response.
  pushHandler->expectHeaders(
      [&](std::shared_ptr<HTTPMessage>) { pushHandler->txn_->sendAbort(); });
  pushHandler->expectDetachTransaction();
  HTTPMessage pushResp;
  pushResp.setStatusCode(200);
  pushResp.getHeaders().set("Dynamic2", "b");
  createPushStream(pushId, pushResp, makeBuf(100), true);

  flushAndLoop();
  hqSession_->closeWhenIdle();
}

TEST_P(HQUpstreamSessionTestPush, TestOnPushedTransaction) {
  // the transaction is expected to timeout, since the PushPromise does not have
  // EOF set, and it is not followed by a PushStream.
  assocHandler_->expectError();
  // assocHandler_->expectHeaders();

  hq::PushId pushId = nextPushId();

  auto pushPromiseRequest = getGetRequest();

  proxygen::HTTPHeaderSize pushPromiseSize;

  sendPushPromise(assocHandler_->txn_->getID(),
                  pushPromiseRequest,
                  pushId,
                  &pushPromiseSize);

  HTTPMessage resp;
  resp.setStatusCode(200);
  createPushStream(pushId, resp, makeBuf(100), true);

  // Once both push promise and push stream have been received, a push
  // transaction should be created
  assocHandler_->txn_->sendEOM();

  auto pushHandler = expectPushResponse();

  hqSession_->closeWhenIdle();
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTestPush, TestOnPushedTransactionOutOfOrder) {
  // the transaction is expected to timeout, since the PushPromise does not have
  // EOF set, and it is not followed by a PushStream.
  assocHandler_->expectError();
  // assocHandler_->expectHeaders();

  hq::PushId pushId = nextPushId();

  HTTPMessage resp;
  resp.setStatusCode(200);
  createPushStream(pushId, resp, makeBuf(100), true);

  auto pushPromiseRequest = getGetRequest();
  proxygen::HTTPHeaderSize pushPromiseSize;
  sendPushPromise(assocHandler_->txn_->getID(),
                  pushPromiseRequest,
                  pushId,
                  &pushPromiseSize);

  // Once both push promise and push stream have been received, a push
  // transaction should be created
  auto pushHandler = expectPushResponse();

  assocHandler_->txn_->sendEOM();

  hqSession_->closeWhenIdle();
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTestPush, TestCloseDroppedConnection) {
  HQSession::DestructorGuard dg(hqSession_);
  // Two "onError" calls are expected:
  // the first when MockQuicSocketDriver closes the socket
  // the second when the error is propagated to the stream
  EXPECT_CALL(*assocHandler_, _onError(testing::_)).Times(2);

  // Create a nascent push stream with a preface only
  createNascentPushStream(1111 /* streamId */, folly::none /* pushId */);

  // Run the event loop to let the dispatcher register the nascent stream
  flushAndLoop();

  // Drop the connection
  hqSession_->dropConnection();
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTestPush, TestOrphanedPushStream) {
  // the transaction is expected to timeout, since the PushPromise does not have
  // EOF set, and it is not followed by a PushStream.
  assocHandler_->expectError();

  hq::PushId pushId = nextPushId();

  HTTPMessage resp;
  resp.setStatusCode(200);
  createPushStream(pushId, resp, makeBuf(100), true);

  assocHandler_->txn_->sendEOM();

  hqSession_->closeWhenIdle();
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTest, TestNoDatagram) {
  EXPECT_FALSE(httpCallbacks_.datagramEnabled);
  auto handler = openTransaction();
  EXPECT_EQ(handler->txn_->getDatagramSizeLimit(), 0);
  auto resp = makeResponse(200, 100);
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM();
  handler->expectDetachTransaction();
  hqSession_->closeWhenIdle();
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTestDatagram, TestDatagramSettings) {
  EXPECT_TRUE(httpCallbacks_.datagramEnabled);
  auto handler = openTransaction();
  EXPECT_GT(handler->txn_->getDatagramSizeLimit(), 0);
  auto resp = makeResponse(200, 100);
  sendResponse(handler->txn_->getID(),
               *std::get<0>(resp),
               std::move(std::get<1>(resp)),
               true);
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  handler->expectHeaders();
  handler->expectBody();
  handler->expectEOM();
  handler->expectDetachTransaction();
  hqSession_->closeWhenIdle();
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTestDatagram, TestReceiveDatagram) {
  EXPECT_TRUE(httpCallbacks_.datagramEnabled);
  auto handler = openTransaction();
  auto id = handler->txn_->getID();
  EXPECT_GT(handler->txn_->getDatagramSizeLimit(), 0);
  MockHTTPTransactionTransportCallback transportCallback_;
  handler->txn_->setTransportCallback(&transportCallback_);
  EXPECT_CALL(transportCallback_, datagramBytesReceived(::testing::_)).Times(1);
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  auto resp = makeResponse(200, 0);
  sendResponse(id, *std::get<0>(resp), std::move(std::get<1>(resp)), false);
  handler->expectHeaders();
  flushAndLoopN(1);
  auto h3Datagram = getH3Datagram(id, folly::IOBuf::wrapBuffer("testtest", 8));
  socketDriver_->addDatagram(std::move(h3Datagram));
  handler->expectDatagram();
  flushAndLoopN(1);
  auto it = streams_.find(id);
  CHECK(it != streams_.end());
  auto& stream = it->second;
  stream.readEOF = true;
  handler->expectEOM();
  handler->expectDetachTransaction();
  hqSession_->closeWhenIdle();
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTestDatagram, TestReceiveEarlyDatagramsSingleStream) {
  EXPECT_TRUE(httpCallbacks_.datagramEnabled);
  auto handler = openTransaction();
  auto id = handler->txn_->getID();
  EXPECT_GT(handler->txn_->getDatagramSizeLimit(), 0);
  handler->txn_->sendHeaders(getGetRequest());
  handler->txn_->sendEOM();
  for (auto i = 0; i < kDefaultMaxBufferedDatagrams * 2; ++i) {
    auto h3Datagram =
        getH3Datagram(id, folly::IOBuf::wrapBuffer("testtest", 8));
    socketDriver_->addDatagram(std::move(h3Datagram));
  }
  flushAndLoopN(1);
  auto resp = makeResponse(200, 0);
  sendResponse(id, *std::get<0>(resp), std::move(std::get<1>(resp)), false);
  handler->expectHeaders();
  EXPECT_CALL(*handler, _onDatagram(testing::_))
      .Times(kDefaultMaxBufferedDatagrams);
  flushAndLoopN(1);
  auto it = streams_.find(id);
  CHECK(it != streams_.end());
  auto& stream = it->second;
  stream.readEOF = true;
  handler->expectEOM();
  handler->expectDetachTransaction();
  hqSession_->closeWhenIdle();
  flushAndLoop();
}

TEST_P(HQUpstreamSessionTestDatagram, TestReceiveEarlyDatagramsMultiStream) {
  auto deliveredDatagrams = 0;
  EXPECT_TRUE(httpCallbacks_.datagramEnabled);
  std::vector<std::unique_ptr<StrictMock<MockHTTPHandler>>> handlers;

  for (auto i = 0; i < kMaxStreamsWithBufferedDatagrams * 2; ++i) {
    handlers.emplace_back(openTransaction());
    auto handler = handlers.back().get();
    auto id = handler->txn_->getID();
    EXPECT_GT(handler->txn_->getDatagramSizeLimit(), 0);
    handler->txn_->sendHeaders(getGetRequest());
    handler->txn_->sendEOM();
    auto h3Datagram =
        getH3Datagram(id, folly::IOBuf::wrapBuffer("testtest", 8));
    socketDriver_->addDatagram(std::move(h3Datagram));
    flushAndLoopN(1);
  }

  for (const auto& handler : handlers) {
    auto id = handler->txn_->getID();
    auto resp = makeResponse(200, 0);
    sendResponse(id, *std::get<0>(resp), std::move(std::get<1>(resp)), false);
    handler->expectHeaders();
    EXPECT_CALL(*handler, _onDatagram(testing::_))
        .WillRepeatedly(InvokeWithoutArgs([&]() { deliveredDatagrams++; }));
    flushAndLoopN(1);
    auto it = streams_.find(id);
    CHECK(it != streams_.end());
    auto& stream = it->second;
    stream.readEOF = true;
    handler->expectEOM();
    handler->expectDetachTransaction();
    flushAndLoopN(1);
  }
  EXPECT_EQ(deliveredDatagrams, kMaxStreamsWithBufferedDatagrams);
  hqSession_->closeWhenIdle();
  flushAndLoop();
}

class HQUpstreamSessionTestWebTransport : public HQUpstreamSessionTest {
 public:
  void SetUp() override {
    HQUpstreamSessionTest::SetUp();
    // Set up WT session
    handler_ = openTransaction();
    sessionId_ = handler_->txn_->getID();
    handler_->txn_->sendHeaders(getWTConnectRequest());
    handler_->expectHeaders();
    sendResponse(sessionId_, getWTResponse(), nullptr, false);
    flushAndLoopN(3);
    wt_ = handler_->txn_->getWebTransport();
    EXPECT_NE(wt_, nullptr);
  }

  void closeWTSession() {
    // should this close INGRESS?
    wt_->closeSession();
    handler_->expectEOM();
    handler_->expectDetachTransaction();
    socketDriver_->addReadEOF(sessionId_, std::chrono::milliseconds(0));
    hqSession_->closeWhenIdle();
    flushAndLoop();
  }

  static HTTPMessage getWTConnectRequest() {
    HTTPMessage req;
    req.setHTTPVersion(1, 1);
    req.setUpgradeProtocol("webtransport");
    req.setMethod(HTTPMethod::CONNECT);
    req.setURL("/webtransport");
    req.getHeaders().set(HTTP_HEADER_HOST, "www.facebook.com");
    return req;
  }

  static HTTPMessage getWTResponse() {
    HTTPMessage resp;
    resp.setHTTPVersion(1, 1);
    resp.setStatusCode(200);
    return resp;
  }

 protected:
  std::unique_ptr<StrictMock<MockHTTPHandler>> handler_;
  uint64_t sessionId_;
  WebTransport* wt_{nullptr};
};

TEST_P(HQUpstreamSessionTestWebTransport, BidirectionalStream) {
  InSequence enforceOrder;
  // Create a bidi WT stream
  auto stream = wt_->createBidiStream().value();
  auto id = stream.readHandle->getID();
  // small write
  stream.writeHandle->writeStreamData(makeBuf(10), false);
  eventBase_.loopOnce();

  // shrink the fcw to force it to block
  socketDriver_->setStreamFlowControlWindow(id, 100);
  bool writeComplete = false;
  stream.writeHandle->writeStreamData(makeBuf(65536), false)
      .value()
      .via(&eventBase_)
      .then([&](auto) {
        VLOG(4) << "big write complete";
        // after it completes, write FIN
        stream.writeHandle->writeStreamData(nullptr, true)
            .value()
            .via(&eventBase_)
            .then([&](auto) {
              VLOG(4) << "fin write complete";
              writeComplete = true;
            });
      });
  eventBase_.loopOnce();
  // grow the fcw which will complete the big write
  socketDriver_->setStreamFlowControlWindow(id, 100000);
  socketDriver_->setConnectionFlowControlWindow(100000);
  eventBase_.loopOnce();
  eventBase_.loopOnce();
  eventBase_.loopOnce();
  EXPECT_TRUE(writeComplete);

  // Wait for a read
  stream.readHandle->awaitNextRead(&eventBase_, [&](auto, auto) {
    // Now add a big buf, which will pause ingress
    socketDriver_->addReadEvent(
        id, makeBuf(70000), std::chrono::milliseconds(0));
  });
  // add a small read to trigger the above handler
  socketDriver_->addReadEvent(id, makeBuf(10), std::chrono::milliseconds(0));
  flushAndLoopN(3);
  EXPECT_TRUE(socketDriver_->isStreamPaused(id));

  // Read again
  stream.readHandle->awaitNextRead(&eventBase_, [&](auto, auto streamData) {
    EXPECT_EQ(streamData->data->computeChainDataLength(), 70000);
    // Add EOF and wait for it
    socketDriver_->addReadEOF(id, std::chrono::milliseconds(0));
    stream.readHandle->awaitNextRead(&eventBase_, [&](auto, auto streamData) {
      EXPECT_TRUE(streamData->fin);
    });
  });
  flushAndLoopN(2);
  closeWTSession();
}

TEST_P(HQUpstreamSessionTestWebTransport, RejectBidirectionalStream) {
  WebTransport::BidiStreamHandle stream;
  EXPECT_CALL(*handler_, onWebTransportBidiStream(_, _))
      .WillOnce(SaveArg<1>(&stream));
  folly::IOBufQueue buf(folly::IOBufQueue::cacheChainLength());
  hq::writeWTStreamPreface(buf, hq::WebTransportStreamType::BIDI, sessionId_);
  socketDriver_->addReadEvent(1, buf.move(), std::chrono::milliseconds(0));
  eventBase_.loopOnce();

  auto id = stream.writeHandle->getID();
  EXPECT_EQ(id, 1);

  // reset write handle
  stream.writeHandle->resetStream(19);
  EXPECT_EQ(
      WebTransport::toApplicationErrorCode(*socketDriver_->streams_[id].error)
          .value(),
      19);

  // stop sending read handle
  stream.readHandle->stopSending(77);
  EXPECT_EQ(
      WebTransport::toApplicationErrorCode(*socketDriver_->streams_[id].error)
          .value(),
      77);

  // add read error (peer reset)
  socketDriver_->addReadError(id, 78, std::chrono::milliseconds(0));
  eventBase_.loopOnce();

  closeWTSession();
}

TEST_P(HQUpstreamSessionTestWebTransport, PairOfUnisReset) {
  socketDriver_->setMaxUniStreams(10);
  auto writeHandle = wt_->createUniStream().value();
  auto writeId = writeHandle->getID();
  WebTransport::StreamReadHandle* readHandle{nullptr};
  EXPECT_CALL(*handler_, onWebTransportUniStream(_, _))
      .WillOnce(SaveArg<1>(&readHandle));
  folly::IOBufQueue buf(folly::IOBufQueue::cacheChainLength());
  hq::writeWTStreamPreface(buf, hq::WebTransportStreamType::UNI, sessionId_);
  socketDriver_->addReadEvent(15, buf.move(), std::chrono::milliseconds(0));
  eventBase_.loopOnce();
  eventBase_.loopOnce();

  auto readId = readHandle->getID();
  EXPECT_EQ(readId, 15);

  // Peer reset
  folly::CancellationCallback writeCancel(
      writeHandle->getCancelToken(), [&] { writeHandle->resetStream(77); });
  socketDriver_->addStopSending(writeId, WebTransport::toHTTPErrorCode(19));
  socketDriver_->addReadError(
      readId, WebTransport::toHTTPErrorCode(77), std::chrono::milliseconds(0));
  flushAndLoopN(2);
  // cancel handler ran, reset stream with err=77
  EXPECT_EQ(WebTransport::toApplicationErrorCode(
                *socketDriver_->streams_[writeId].error)
                .value(),
            77);
  // readHandle holds the reset error
  readHandle->readStreamData()
      .via(&eventBase_)
      .thenValue([](auto) {})
      .thenError(folly::tag_t<const WebTransport::Exception&>{},
                 [](auto const& ex) { EXPECT_EQ(ex.error, 77); });

  eventBase_.loopOnce();

  closeWTSession();
}

/**
 * Instantiate the Parametrized test cases
 */

// Make sure all the tests keep working with all the supported protocol versions
INSTANTIATE_TEST_SUITE_P(HQUpstreamSessionTest,
                         HQUpstreamSessionTest,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           return tp;
                         }()),
                         paramsToTestName);

// Instantiate tests for H3 Push functionality (requires HQ)
INSTANTIATE_TEST_SUITE_P(HQUpstreamSessionTest,
                         HQUpstreamSessionTestPush,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           tp.unidirectionalStreamsCredit = 4;
                           return tp;
                         }()),
                         paramsToTestName);

// Instantiate tests with QPACK on/off
INSTANTIATE_TEST_SUITE_P(HQUpstreamSessionTest,
                         HQUpstreamSessionTestQPACK,
                         Values(
                             [] {
                               TestParams tp;
                               tp.alpn_ = "h3";
                               return tp;
                             }(),
                             [] {
                               TestParams tp;
                               tp.alpn_ = "h3";
                               tp.createQPACKStreams_ = false;
                               return tp;
                             }()),
                         paramsToTestName);

// Instantiate h3 datagram tests
INSTANTIATE_TEST_SUITE_P(HQUpstreamSessionTest,
                         HQUpstreamSessionTestDatagram,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           tp.datagrams_ = true;
                           return tp;
                         }()),
                         paramsToTestName);

// Instantiate h3 webtransport tests
INSTANTIATE_TEST_SUITE_P(HQUpstreamSessionTest,
                         HQUpstreamSessionTestWebTransport,
                         Values([] {
                           TestParams tp;
                           tp.alpn_ = "h3";
                           tp.webTransport_ = true;
                           return tp;
                         }()),
                         paramsToTestName);
