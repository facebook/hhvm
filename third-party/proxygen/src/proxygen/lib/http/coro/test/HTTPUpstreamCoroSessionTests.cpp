/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/test/HTTPClientTestsCommon.h"
#include "proxygen/lib/http/coro/test/HTTPCoroSessionTests.h"
#include "proxygen/lib/http/coro/test/HTTPTestSources.h"
#include "proxygen/lib/http/coro/test/Mocks.h"
#include <folly/logging/xlog.h>

#include <folly/ExceptionWrapper.h>
#include <folly/coro/Timeout.h>

#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/util/test/TestHelpers.h"
#include <proxygen/lib/http/codec/HTTP2Codec.h>

using namespace proxygen;
using namespace testing;
using TransportErrorCode = folly::coro::TransportIf::ErrorCode;

namespace proxygen::coro::test {

class HTTPUpstreamSessionTest : public HTTPCoroSessionTest {
 public:
  HTTPUpstreamSessionTest()
      : HTTPCoroSessionTest(TransportDirection::UPSTREAM),
        serverCodec_(peerCodec_.get()) {
  }

  void SetUp() override {
    HTTPCoroSessionTest::setUp();
  }

  // TearDown defined in parent

  folly::coro::Task<std::unique_ptr<HTTPFixedSource>> expectResponse(
      HTTPSourceHolder responseSource,
      uint16_t statusCode,
      uint32_t contentLength = 0,
      bool eom = true) {
    auto res = co_await co_awaitTry(responseSource.readHeaderEvent());
    EXPECT_FALSE(res.hasException());
    if (res.hasException()) {
      co_return nullptr;
    }
    auto streamID = *responseSource.getStreamID();
    EXPECT_EQ(res->headers->getSeqNo(),
              HTTPCodec::streamIDToSeqNo(GetParam().codecProtocol, streamID));
    EXPECT_EQ(res->headers->getStatusCode(), statusCode);
    EXPECT_TRUE(res->isFinal());
    auto response = std::make_unique<HTTPFixedSource>(std::move(res->headers));
    if (res->eom) {
      EXPECT_EQ(contentLength, 0);
      co_return response;
    }
    bool readEom = false;
    do {
      auto bodyEvent =
          co_await co_awaitTry(readBodyEventNoSuspend(responseSource));
      EXPECT_FALSE(bodyEvent.hasException());
      if (bodyEvent.hasException()) {
        co_return nullptr;
      }
      switch (bodyEvent->eventType) {
        case HTTPBodyEvent::PUSH_PROMISE:
          co_withExecutor(&evb_,
                          onPushPromise(std::move(bodyEvent->event.push)))
              .start();
          break;
        case HTTPBodyEvent::BODY: {
          if (!bodyEvent->event.body.empty()) {
            EXPECT_GT(contentLength, 0);
            auto length = bodyEvent->event.body.chainLength();
            XCHECK_GE(contentLength, length);
            contentLength -= length;
            response->body_.append(bodyEvent->event.body.move());
          }
          break;
        }
        default:
          // unhandled for now
          break;
      }
      readEom = bodyEvent->eom;
    } while (!readEom);
    EXPECT_EQ(contentLength, 0);
    co_return response;
  }

  void serializeResponse(HTTPCodec::StreamID id,
                         uint16_t statusCode,
                         std::unique_ptr<folly::IOBuf> body = nullptr,
                         bool eom = true,
                         bool eof = false) {
    serializeResponse(
        id, makeResponse(statusCode, eom), std::move(body), eom, eof);
  }

  void serializeResponse(HTTPCodec::StreamID id,
                         HTTPMessage response,
                         std::unique_ptr<folly::IOBuf> body = nullptr,
                         bool eom = true,
                         bool eof = false) {
    serializeResponseHeader(id, std::move(response), eom && !body);
    if (body) {
      serverCodec_->generateBody(
          writeBuf_, id, std::move(body), HTTPCodec::NoPadding, eom);
    }
    transport_->addReadEvent(id, writeBuf_.move(), (eom && isHQ()) || eof);
  }

  HTTPMessage makeResponse(uint16_t statusCode, bool eom = false) {
    HTTPMessage resp;
    resp.setHTTPVersion(1, 1);
    resp.setStatusCode(statusCode);
    if (GetParam().codecProtocol == CodecProtocol::HTTP_1_1) {
      // Gross, but H1 can't serialize a response without receiving a request
      auto fake =
          folly::IOBuf::copyBuffer(std::string("GET / HTTP/1.1\r\n\r\n"));
      NiceMock<MockHTTPCodecCallback> callbacks;
      serverCodec_->setCallback(&callbacks);
      serverCodec_->onIngress(*fake);
      if (!eom) {
        resp.setIsChunked(true);
        resp.getHeaders().set(HTTP_HEADER_TRANSFER_ENCODING, "chunked");
      }
    }
    return resp;
  }

  void serializeResponseHeader(HTTPCodec::StreamID id,
                               uint16_t statusCode,
                               bool eom = true) {
    serializeResponseHeader(id, makeResponse(statusCode, eom), eom);
  }

  void serializeResponseHeader(HTTPCodec::StreamID id,
                               HTTPMessage resp,
                               bool eom = true,
                               bool flushQPACK = true) {
    if (isHQ()) {
      multiCodec_->addCodec(id);
    }
    serverCodec_->generateHeader(writeBuf_, id, resp, eom);
    if (flushQPACK) {
      flushQPACKEncoder();
    }
  }

  folly::coro::Task<void> onPushPromise(HTTPPushEvent pushEvent) {
    if (pushPromiseHandler_) {
      co_await pushPromiseHandler_(std::move(pushEvent));
    } else {
      auto &it = pushes_.emplace_back(std::move(pushEvent), nullptr);
      it.second =
          co_await expectResponse(it.first.movePushSource(), 200, 100, true);
    }
  }

  /* sends a new push promise for "GET /push" on given streamID, optionally
   * adding streamID's EOM. Returns a tuple of [PushStreamId, PushId].
   */
  std::pair<HTTPCodec::StreamID, HTTPCodec::StreamID> serializePushPromise(
      HTTPCodec::StreamID streamID,
      bool eom,
      bool flush = true,
      folly::IOBufQueue *pushStreamBuf = nullptr) {
    HTTPCodec::StreamID pushID;
    HTTPCodec::StreamID pushStreamID;
    if (isHQ()) {
      pushStreamID = muxTransport_->nextUnidirectionalStreamId_;
      muxTransport_->nextUnidirectionalStreamId_ += 4;
      pushID = multiCodec_->nextPushID();
      uint64_t streamType = uint64_t(hq::UnidirectionalStreamType::PUSH);
      folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
      bool flushPushStream = false;
      if (!pushStreamBuf) {
        pushStreamBuf = &writeBuf;
        flushPushStream = true;
      }
      hq::writeStreamPreface(*pushStreamBuf, streamType);
      hq::writeStreamPreface(*pushStreamBuf, pushID);
      if (flushPushStream) {
        transport_->addReadEvent(pushStreamID, pushStreamBuf->move(), false);
      }
    } else {
      pushID = pushStreamID = serverCodec_->createStream();
    }
    HTTPHeaderSize size;
    HTTPMessage promise = getGetRequest("/push");
    serverCodec_->generatePushPromise(
        writeBuf_, pushID, promise, streamID, false, &size);
    flushQPACKEncoder();
    if (eom) {
      serverCodec_->generateEOM(writeBuf_, streamID);
    }
    if (flush) {
      transport_->addReadEvent(streamID, writeBuf_.move(), isHQ() && eom);
    }
    return {pushStreamID, pushID};
  }

  /* Simple push test */
  folly::coro::Task<HTTPCodec::StreamID> testPush(
      bool expectIncomingStream = true, bool pushEOM = true, bool eof = true) {
    // send a request for GET /
    auto responseSource = co_await co_awaitTry(
        session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
    XCHECK(!responseSource.hasException());
    auto streamID = *responseSource->getStreamID();

    // serialize a response with no EOM
    serializeResponse(streamID, 200, makeBuf(100), false);

    // serialize a PUSH_PROMISE and stream EOM
    auto pushID = serializePushPromise(streamID, true).first;

    // Expect the request's response
    co_await expectResponse(std::move(*responseSource), 200, 100, true);
    if (isHQ()) {
      // TODO: HQ (properly) doesn't count the stream until the actual stream is
      // opened.
      co_await rescheduleN(1);
    }
    EXPECT_EQ(session_->numIncomingStreams(), expectIncomingStream ? 1 : 0);

    // Now serialize the push response, with optional EOM
    serializeResponse(pushID, 200, makeBuf(100), pushEOM);

    // Optionally add EOF
    if (eof) {
      serverCodec_->generateGoaway(writeBuf_, streamID, ErrorCode::NO_ERROR);
      transport_->addReadEvent(writeBuf_.move(), false);
    }
    co_return pushID;
  }

  void expectH1ConnectionReset() {
    if (IS_H1()) {
      EXPECT_TRUE(transportState_.closedWithReset);
      expectedError_ = TransportErrorCode::NETWORK_ERROR;
    }
  }

  HTTPCodec *serverCodec_{nullptr};
  std::list<std::pair<HTTPPushEvent, std::unique_ptr<HTTPFixedSource>>> pushes_;
  std::function<folly::coro::Task<void>(HTTPPushEvent)> pushPromiseHandler_;
};

// Use this test class for h1 only tests
using H1UpstreamSessionTest = HTTPUpstreamSessionTest;
// Use this test class for h2 only tests
using H2UpstreamSessionTest = HTTPUpstreamSessionTest;
// Use this test class for hq only tests
using HQUpstreamSessionTest = HTTPUpstreamSessionTest;
// Use this test class for h2/hq only tests
using H2QUpstreamSessionTest = HTTPUpstreamSessionTest;
// Use this test class for h1/h2 only tests
using H12UpstreamSessionTest = HTTPUpstreamSessionTest;

CO_TEST_P_X(HTTPUpstreamSessionTest, Simple) {
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  EXPECT_EQ(session_->numIncomingStreams(), 0);
  EXPECT_EQ(session_->numOutgoingStreams(), 1);
  if (!IS_H1()) {
    EXPECT_EQ(session_->numTransactionsAvailable(), 9);
  }
  XCHECK(!responseSource.hasException());
  serializeResponse(*responseSource->getStreamID(), 200, makeBuf(100), true);
  co_await expectResponse(std::move(*responseSource), 200, 100, true);
  transport_->addReadEvent(nullptr, true);
}

CO_TEST_P_X(HTTPUpstreamSessionTest, SimplePost) {
  auto responseSource = co_await co_awaitTry(session_->sendRequest(
      HTTPFixedSource::makeFixedRequest("/", HTTPMethod::POST, makeBuf(100))));
  XCHECK(!responseSource.hasException());
  serializeResponse(*responseSource->getStreamID(), 200, makeBuf(100), true);
  co_await expectResponse(std::move(*responseSource), 200, 100, true);
  transport_->addReadEvent(nullptr, true);
}

CO_TEST_P_X(HTTPUpstreamSessionTest, LargePost) {
  co_await rescheduleN(1);
  HTTPCodec::StreamID id;
  auto responseSource = co_await co_awaitTry(session_->sendRequest(
      new OnEOMSource(HTTPFixedSource::makeFixedRequest(
                          "/", HTTPMethod::POST, makeBuf(70000)),
                      [this, &id]() -> OnEOMSource::CallbackReturn {
                        // Serialize the response when the EOM goes out
                        // The response is large enough to trigger a
                        // WINDOW_UPDATE
                        serializeResponse(id, 200, makeBuf(40000), true);
                        co_return folly::none;
                      })));
  XCHECK(!responseSource.hasException());
  id = *responseSource->getStreamID();
  evb_.runInLoop([this, id] {
    // The POST will run out of flow control, give it some in the loop
    windowUpdate(10000);
    windowUpdate(id, 10000);
  });

  co_await expectResponse(std::move(*responseSource), 200, 40000, true);
  co_await rescheduleN(1);

  transport_->addReadEvent(nullptr, true);
  if (isHQ()) {
    EXPECT_GT(muxTransport_->socketDriver_.streams_[id].writeBuf.chainLength(),
              70000);
  }
}

CO_TEST_P_X(HTTPUpstreamSessionTest, Padding) {
  auto responseSource = co_await co_awaitTry(session_->sendRequest(
      HTTPFixedSource::makeFixedRequest("/", HTTPMethod::POST, makeBuf(100))));
  XCHECK(!responseSource.hasException());
  // @lint-ignore CLANGTIDY bugprone-unchecked-optional-access
  const auto id = *responseSource->getStreamID();
  HTTPMessage msg = makeResponse(200, /*eom=*/false);
  serializeResponseHeader(id, std::move(msg), false);
  serverCodec_->generatePadding(writeBuf_, id, 10);
  serverCodec_->generateBody(
      writeBuf_, id, makeBuf(90), HTTPCodec::NoPadding, false);
  serverCodec_->generatePadding(writeBuf_, id, 5);
  serverCodec_->generateBody(
      writeBuf_, id, makeBuf(10), HTTPCodec::NoPadding, true);
  transport_->addReadEvent(id, writeBuf_.move(), true);
  co_await expectResponse(std::move(*responseSource), 200, 100, true);
  transport_->addReadEvent(nullptr, true);
}

CO_TEST_P_X(H2QUpstreamSessionTest, ReceiveFullResponsePriorToRequestEOM) {
  /**
   * This test basically creates a large post request and checks that we can
   * receive a response before the egress request is completed.
   *
   * From RFC7540:
   * A server can send a complete response prior to the client sending an entire
   * request if the response does not depend on any portion of the request that
   * has not been sent and received.
   */

  /**
   * Create HTTPSource that will only produce a header and *no* body to simulate
   * that req egress isn't "complete" before we receive resp. When the final
   * header is read from the req source, sendRequest() returns and we serialize
   * the 200 response to the client.
   */
  auto reqHeaders = std::make_unique<HTTPMessage>(getPostRequest(1000));
  auto *reqStreamSource = new HTTPStreamSource(&evb_);
  reqStreamSource->setHeapAllocated();
  reqStreamSource->headers(std::move(reqHeaders));

  // send request & get id
  auto responseSource =
      co_await co_awaitTry(session_->sendRequest(reqStreamSource));
  XCHECK(!responseSource.hasException());
  auto id = *responseSource->getStreamID();
  serializeResponse(id, 200, makeBuf(1500));

  // egress stop_sending or rst_stream w/ no_error to peer after
  // message complete
  if (isHQ()) {
    muxTransport_->socketDriver_.addStopSending(id, HTTP3::HTTP_NO_ERROR);
  } else {
    serverCodec_->generateRstStream(writeBuf_, id, ErrorCode::NO_ERROR);
    transport_->addReadEvent(writeBuf_.move());
  }

  // recv-ing full response & stop_sending or rst_stream w/ no_error should
  // detach transaction
  EXPECT_CALL(lifecycleObs_, onTransactionDetached(_));
  co_await expectResponse(std::move(*responseSource), 200, 1500, true);
}

CO_TEST_P_X(H2QUpstreamSessionTest, StopReadingOnAbort) {
  /** Verifies that when receiving a RST_STREAM / STOP_SENDING frame from the
   * peer while body egress is still in progress, we invoke ::stopReading() w/
   * a HTTPErrorCode */
  HTTPCodec::StreamID id;

  auto reqHeaders = std::make_unique<HTTPMessage>(getPostRequest(1000));
  MockHTTPSource reqSource;
  EXPECT_CALL(reqSource, readHeaderEvent).WillOnce([]() {
    return folly::coro::makeTask(HTTPHeaderEvent(
        std::make_unique<HTTPMessage>(getPostRequest(1000)), false));
  });

  EXPECT_CALL(reqSource, readBodyEvent(_))
      .WillOnce([&]() -> folly::coro::Task<HTTPBodyEvent> {
        // StreamID available after one evb loop
        co_await folly::coro::co_reschedule_on_current_executor;
        if (isHQ()) {
          muxTransport_->socketDriver_.addStopSending(id, HTTP3::HTTP_NO_ERROR);
        } else {
          serverCodec_->generateRstStream(writeBuf_, id, ErrorCode::NO_ERROR);
          transport_->addReadEvent(writeBuf_.move());
        }
        co_await folly::coro::co_reschedule_on_current_executor;
        co_return HTTPBodyEvent(makeBuf(1000), false);
      });

  EXPECT_CALL(reqSource,
              stopReading(A<folly::Optional<const HTTPErrorCode>>()));

  auto responseSource = co_await co_awaitTry(session_->sendRequest(&reqSource));
  XCHECK(!responseSource.hasException());
  id = *responseSource->getStreamID();
  auto resSourceHeaderEvent =
      co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(resSourceHeaderEvent.hasException());
}

CO_TEST_P_X(H2UpstreamSessionTest, RstStreamAfterEOM) {
  HTTPCodec::StreamID id;
  auto reqHeaders = std::make_unique<HTTPMessage>(getGetRequest());
  auto *reqStreamSource = new HTTPStreamSource(&evb_);
  reqStreamSource->setHeapAllocated();
  reqStreamSource->headers(std::move(reqHeaders), false);

  // send request & get id
  auto responseSource =
      co_await co_awaitTry(session_->sendRequest(reqStreamSource));
  XCHECK(!responseSource.hasException());
  id = *responseSource->getStreamID();

  // send response with eom and follow with RST_STREAM
  serializeResponse(id, 200, makeBuf(1500));
  serverCodec_->generateRstStream(writeBuf_, id, ErrorCode::NO_ERROR);
  transport_->addReadEvent(writeBuf_.move());
  // add connection end
  transport_->addReadEvent(nullptr, true);

  co_await expectResponse(std::move(*responseSource), 200, 1500, true);
  co_await rescheduleN(2);
}

CO_TEST_P_X(HQUpstreamSessionTest, ReceiveStopSendingStreamPriorToResponseEOM) {
  // receiving a STOP_SENDING prior to response eom (due to possible packet
  // re-ordering) should generate just a RST_STREAM and not STOP_SENDING
  HTTPCodec::StreamID id;

  /**
   * Create HTTPSource that will only produce a header and *no* body to simulate
   * that req egress isn't "complete" before we receive resp. When the final
   * header is read from the req source, sendRequest() returns and we serialize
   * the 200 response to the client without EOM then send a RST_STREAM.
   */
  auto reqHeaders = std::make_unique<HTTPMessage>(getPostRequest(1000));
  auto *reqStreamSource = new HTTPStreamSource(&evb_);
  reqStreamSource->setHeapAllocated();
  reqStreamSource->headers(std::move(reqHeaders));

  // send request & get id
  auto responseSource =
      co_await co_awaitTry(session_->sendRequest(reqStreamSource));
  XCHECK(!responseSource.hasException());
  id = *responseSource->getStreamID();

  // serialize partial response
  serializeResponse(id, 200, makeBuf(1500), /*eom=*/false);

  // Deliver stopSending callback to session. This should result in the session
  // calling resetStream but not stopSending on the socket.
  EXPECT_CALL(*muxTransport_->getSocket(),
              resetStream(id,
                          quic::ApplicationErrorCode(
                              HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED)))
      .Times(1);
  EXPECT_CALL(*muxTransport_->getSocket(), stopSending(id, _)).Times(0);
  muxTransport_->socketDriver_.addStopSending(id,
                                              HTTP3::ErrorCode::HTTP_NO_ERROR);
  co_await rescheduleN(1);

  // deliver rest of body
  serverCodec_->generateBody(
      writeBuf_, id, makeBuf(500), HTTPCodec::NoPadding, /*eom=*/true);
  transport_->addReadEvent(id, writeBuf_.move(), /*eom=*/true);

  // recv-ing stop_sending w/ NO_ERROR prior to full response  should
  // detach transaction successfully without error
  EXPECT_CALL(lifecycleObs_, onTransactionDetached(_));
  co_await expectResponse(std::move(*responseSource), 200, 2000, true);
  co_await rescheduleN(2);
}

CO_TEST_P_X(HTTPUpstreamSessionTest, ReserveRequest) {
  session_->setMaxConcurrentOutgoingStreams(1);
  EXPECT_EQ(session_->getNextStreamSeqNum(), 0);

  // Reserve a request
  EXPECT_CALL(lifecycleObs_, onTransactionAttached(_));
  auto res = session_->reserveRequest();
  EXPECT_FALSE(res.hasException());
  EXPECT_EQ(session_->getNextStreamSeqNum(), 0); // still not bumped

  // Try to reserve another - fails
  auto res2 = session_->reserveRequest();
  EXPECT_TRUE(res2.hasException());
  EXPECT_EQ(getHTTPError(res2).code, HTTPErrorCode::REFUSED_STREAM);

  // cancel first reservation without sending, can get another
  EXPECT_CALL(lifecycleObs_, onTransactionDetached(_));
  res->cancel();
  EXPECT_TRUE(session_->supportsMoreTransactions());
  EXPECT_CALL(lifecycleObs_, onTransactionAttached(_));
  res2 = session_->reserveRequest();
  EXPECT_FALSE(res.hasException());

  // move assign
  res = std::move(res2);
  // res2 is empty.  cancel is now a no-op
  // @lint - ignore
  res2->cancel();
  EXPECT_FALSE(session_->supportsMoreTransactions());

  if (!IS_H1()) {
    session_->setMaxConcurrentOutgoingStreams(2);
    {
      EXPECT_CALL(lifecycleObs_, onTransactionAttached(_));
      auto res3 = session_->reserveRequest();
      EXPECT_FALSE(res3.hasException());
      EXPECT_FALSE(session_->supportsMoreTransactions());
      // let res3 destruct without sending
      EXPECT_CALL(lifecycleObs_, onTransactionDetached(_));
    }
    EXPECT_TRUE(session_->supportsMoreTransactions());
  }

#if NDEBUG
  // res2 is no longer a valid reservation, will get an error.  This is a
  // DFATAL so only try in opt builds
  auto respSource = co_await co_awaitTry(session_->sendRequest(
      (HTTPFixedSource::makeFixedRequest("/")), std::move(*res2)));
  EXPECT_EQ(getHTTPError(respSource).code, HTTPErrorCode::INTERNAL_ERROR);
#endif

  EXPECT_CALL(lifecycleObs_, onTransactionDetached(_));
  // Valid request
  auto responseSource = co_await co_awaitTry(session_->sendRequest(
      HTTPFixedSource::makeFixedRequest("/"), std::move(*res)));
  XCHECK(!responseSource.hasException());
  serializeResponse(*responseSource->getStreamID(), 200, makeBuf(100), true);
  co_await expectResponse(std::move(*responseSource), 200, 100, true);
  EXPECT_EQ(session_->getNextStreamSeqNum(), 1);
  transport_->addReadEvent(nullptr, true);
}

CO_TEST_P_X(H2QUpstreamSessionTest, IngressResetStream) {
  // Send request, response with a reset stream
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  auto id = *responseSource->getStreamID();
  resetStream(id, ErrorCode::CANCEL);
  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(headerEvent.hasException());
  session_->initiateDrain();
}

// Could be H1 if we check upstream pipeline limit
CO_TEST_P_X(H2QUpstreamSessionTest, EgressStreamLimitExceeded) {
  if (isHQ()) {
    muxTransport_->socketDriver_.setMaxBidiStreams(0);
  } else {
    setTestCodecSetting(serverCodec_->getEgressSettings(),
                        SettingsId::MAX_CONCURRENT_STREAMS,
                        0);
    serverCodec_->generateSettings(writeBuf_);
    transport_->addReadEvent(writeBuf_.move(), false);
  }
  co_await folly::coro::co_reschedule_on_current_executor;

  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  EXPECT_TRUE(responseSource.hasException());
  EXPECT_EQ(getHTTPError(responseSource).code, HTTPErrorCode::REFUSED_STREAM);
  transport_->addReadEvent(nullptr, true);
}

CO_TEST_P_X(HQUpstreamSessionTest, StreamsFull) {
  muxTransport_->socketDriver_.setMaxBidiStreams(1);

  EXPECT_TRUE(session_->supportsMoreTransactions());
  EXPECT_EQ(session_->numTransactionsAvailable(), 1);
  auto res = session_->reserveRequest();
  EXPECT_FALSE(res.hasException());
  EXPECT_FALSE(session_->supportsMoreTransactions());
  EXPECT_EQ(session_->numTransactionsAvailable(), 0);
  res->cancel();
  EXPECT_TRUE(session_->supportsMoreTransactions());
  EXPECT_EQ(session_->numTransactionsAvailable(), 1);

  EXPECT_CALL(lifecycleObs_, onSettingsOutgoingStreamsNotFull(_));
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  EXPECT_FALSE(session_->supportsMoreTransactions());
  serializeResponse(*responseSource->getStreamID(), 200, makeBuf(100), true);
  co_await expectResponse(std::move(*responseSource), 200, 100, true);
  muxTransport_->socketDriver_.setMaxBidiStreams(2);
}

// Push tests -- H2 only
// TODO: HQ push id limits
CO_TEST_P_X(H2UpstreamSessionTest, IngressStreamLimitExceeded) {
  session_->setSetting(SettingsId::MAX_CONCURRENT_STREAMS, 0);
  // Set the setting to 0, but don't send to the peer

  pushPromiseHandler_ = [](HTTPPushEvent pushEvent) -> folly::coro::Task<void> {
    auto pushSource = pushEvent.movePushSource();
    auto event = co_await co_awaitTry(pushSource->readHeaderEvent());
    EXPECT_TRUE(event.hasException());
  };
  // send a request and receive a response that includes a push promise and
  // complete push response.
  co_await testPush();
  onTearDown([this] { EXPECT_EQ(pushes_.size(), 0); });
}

CO_TEST_P_X(H2QUpstreamSessionTest, Push) {
  // send a request and receive a response that includes a push promise and
  // complete push response.
  co_await testPush();
  onTearDown([this] {
    EXPECT_EQ(pushes_.size(), 1);
    auto &push = pushes_.front();
    XCHECK(push.second);
    EXPECT_EQ(push.first.promise->getURL(), "/push");
    EXPECT_EQ(push.second->msg_->getStatusCode(), 200);
    EXPECT_EQ(push.second->body_.chainLength(), 100);
  });
}

CO_TEST_P_X(H2QUpstreamSessionTest, PushEgressRstStream) {
  // When we receive a push promise, stop reading from the source.
  pushPromiseHandler_ =
      [this](HTTPPushEvent pushEvent) -> folly::coro::Task<void> {
    // The pushEvent will automatically call stopReading on destruction,
    // triggering STOP_SENDING/RST_STREAM (w/ ErrorCode::CANCEL) since we
    // haven't seen EOM yet.
    co_await rescheduleN(2);
    XLOG(DBG4) << "Abandoning push";
    co_return;
  };

  auto pushID = co_await testPush(
      /*expectIncomingStream=*/true, /*pushEOM=*/false, /*eof=*/false);

  onTearDown([this, pushID] {
    EXPECT_EQ(pushes_.size(), 0);
    // upstream .stopReading() = cancel error code
    if (isHQ()) {
      EXPECT_EQ(muxTransport_->socketDriver_.streams_[pushID].error,
                HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED);
    } else {
      expectStreamAbort(pushID, ErrorCode::CANCEL);
      parseOutputUniplex();
    }
  });
}

CO_TEST_P_X(H2QUpstreamSessionTest, PushIngressRstStream) {
  // Instead of serializing the push response EOM, serialize a RST_STREAM
  // for the push stream.  This will result in the push handler receiving an
  // error.
  pushPromiseHandler_ =
      [this](HTTPPushEvent pushEvent) -> folly::coro::Task<void> {
    auto &it = pushes_.emplace_back(std::move(pushEvent), nullptr);
    auto req =
        co_await co_awaitTry(it.first.movePushSource()->readHeaderEvent());
    EXPECT_TRUE(req.hasException());
    co_return;
  };
  auto pushID = co_await testPush(
      /*expectIncomingStream=*/true, /*pushEOM=*/false, /*eof=*/false);
  resetStream(pushID, ErrorCode::CANCEL);
  serverCodec_->generateGoaway(writeBuf_, pushID + 4, ErrorCode::NO_ERROR);
  transport_->addReadEvent(writeBuf_.move(), false);

  onTearDown([this] { EXPECT_EQ(pushes_.size(), 1); });
}

// Test when the parent stream goes away between onPushMessageBegin and
// onHeadersComplete.
// TODO: H3 requires a QPACK dynamic table stall for this case
CO_TEST_P_X(H2UpstreamSessionTest, PushParentReset) {
  // send a request, then reset it.  The peer sends a response that
  // includes a push promise and complete push response.
  //
  // Note, we need the PUSH_PROMISE to have a continuation, so give the server
  // codec a ridiculously small MAX_FRAME_SIZE.
  HTTPSettings *settings = (HTTPSettings *)serverCodec_->getIngressSettings();
  settings->setSetting(SettingsId::MAX_FRAME_SIZE, 5);

  // send a request for GET /
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  auto streamID = *responseSource->getStreamID();

  // serialize a response with no body/EOM
  serializeResponse(streamID, 200, nullptr, false);

  // serialize a PUSH_PROMISE and stream EOM
  serializePushPromise(streamID, false, false);
  auto toSend = writeBuf_.split(writeBuf_.chainLength() - 1);
  auto toHold = writeBuf_.move();
  writeBuf_.append(std::move(toSend));
  transport_->addReadEvent(streamID, writeBuf_.move(), false);

  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_FALSE(headerEvent.hasException());
  // Reset the parent stream, and flush the CONTINUATION byte
  responseSource->stopReading();
  transport_->addReadEvent(streamID, std::move(toHold), false);
  transport_->addReadEvent(nullptr, true);
  onTearDown([this, settings] {
    EXPECT_EQ(pushes_.size(), 0);
    // Reset the MAX_FRAME_SIZE
    settings->setSetting(SettingsId::MAX_FRAME_SIZE,
                         http2::kMaxFramePayloadLengthMin);
  });
}

CO_TEST_P_X(HQUpstreamSessionTest, PromiseBeforePush) {
  // send a request for GET /
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  auto streamID = *responseSource->getStreamID();

  // serialize a response with no EOM
  serializeResponse(streamID, 200, makeBuf(100), false);

  // serialize a PUSH_PROMISE, eom, flush, capture pushStream preface
  multiCodec_->addCodec(streamID);
  folly::IOBufQueue pushStreamBuf{folly::IOBufQueue::cacheChainLength()};
  auto pushID =
      serializePushPromise(streamID, true, true, &pushStreamBuf).first;

  // Wait for PUSH_PROMISE to get parsed
  co_await folly::coro::co_reschedule_on_current_executor;

  // Expect the request's response
  co_await expectResponse(std::move(*responseSource), 200, 100, true);

  // Now serialize the push response
  transport_->addReadEvent(pushID, pushStreamBuf.move(), false);
  serializeResponse(pushID, 200, makeBuf(100), true);

  serverCodec_->generateGoaway(writeBuf_, streamID, ErrorCode::NO_ERROR);
  transport_->addReadEvent(writeBuf_.move(), false);
  onTearDown([this] {
    EXPECT_EQ(pushes_.size(), 1);
    auto &push = pushes_.front();
    ASSERT_NE(push.second, nullptr);
    EXPECT_EQ(push.first.promise->getURL(), "/push");
    EXPECT_EQ(push.second->msg_->getStatusCode(), 200);
    EXPECT_EQ(push.second->body_.chainLength(), 100);
  });
}

CO_TEST_P_X(HQUpstreamSessionTest, SendPushPriority) {
  // send a request for GET /
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  auto streamID = *responseSource->getStreamID();

  // serialize a response with no EOM
  serializeResponse(streamID, 200, makeBuf(100), false);

  // Send stream priority update frame. This should also set local priority
  // (even tho the request has been written).
  auto *session = static_cast<HTTPQuicCoroSession *>(session_);
  muxTransport_->socketDriver_.expectSetPriority(
      streamID, quic::HTTPPriorityQueue::Priority(3, false));
  EXPECT_GT(session->sendPriority(streamID, HTTPPriority(3, false)), 0);

  // serialize a PUSH_PROMISE, eom, flush, capture pushStream preface
  multiCodec_->addCodec(streamID);
  folly::IOBufQueue pushStreamBuf{folly::IOBufQueue::cacheChainLength()};
  auto pushRes = serializePushPromise(streamID, true, true, &pushStreamBuf);

  // Wait for PUSH_PROMISE to get parsed
  co_await folly::coro::co_reschedule_on_current_executor;

  // Expect the request's response
  co_await expectResponse(std::move(*responseSource), 200, 100, true);

  // Send push priority
  EXPECT_GT(session->sendPushPriority(pushRes.second, HTTPPriority(4, true)),
            0);

  // Now serialize the push response
  transport_->addReadEvent(pushRes.first, pushStreamBuf.move(), false);
  serializeResponse(pushRes.first, 200, makeBuf(100), true);

  serverCodec_->generateGoaway(writeBuf_, streamID, ErrorCode::NO_ERROR);
  transport_->addReadEvent(writeBuf_.move(), false);
  onTearDown([this] {
    EXPECT_EQ(pushes_.size(), 1);
    auto &push = pushes_.front();
    ASSERT_NE(push.second, nullptr);
    EXPECT_EQ(push.first.promise->getURL(), "/push");
    EXPECT_EQ(push.second->msg_->getStatusCode(), 200);
    EXPECT_EQ(push.second->body_.chainLength(), 100);
  });
}

CO_TEST_P_X(H1UpstreamSessionTest, DrainAfterCodecNotReusable) {
  // send first request, verify no exception
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());

  // rx response with Connection: Close header (i.e. no keep-alive)
  auto id = *responseSource->getStreamID();
  auto respMsg = makeResponse(200, true);
  respMsg.getHeaders().set(HTTPHeaderCode::HTTP_HEADER_CONNECTION, "close");
  respMsg.getHeaders().set(HTTPHeaderCode::HTTP_HEADER_CONTENT_LENGTH, "0");
  serializeResponseHeader(id, std::move(respMsg), true);
  transport_->addReadEvent(id, writeBuf_.move(), false);

  // read full response, sanity check status code
  HTTPSourceReader reader(std::move(*responseSource));
  reader.onHeaders(
      [](std::unique_ptr<HTTPMessage> headers, bool /*final*/, bool eom) {
        EXPECT_EQ(headers->getStatusCode(), 200);
        EXPECT_TRUE(eom);
        return HTTPSourceReader::Continue;
      });
  auto resp = co_await co_awaitTry(reader.read());
  EXPECT_FALSE(resp.hasException());

  // verify session is draining and attempting to send another request will
  // yield exception
  auto drainedException = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  CHECK(drainedException.hasException());
  auto ex = drainedException.exception().get_exception<HTTPError>();

  // reservation fails with "Exceeded stream limit" since
  // supportsMoreTransactions() == false
  EXPECT_EQ(ex->code, HTTPErrorCode::REFUSED_STREAM);
  EXPECT_EQ(ex->msg, "Exceeded stream limit");
}

CO_TEST_P_X(H1UpstreamSessionTest, ReceiveGoaway) {
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  auto id = *responseSource->getStreamID();
  generateGoaway(id, ErrorCode::NO_ERROR);
  serializeResponseHeader(id, 200, false);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  auto resp = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
  serverCodec_->generateBody(
      writeBuf_, id, makeBuf(100), HTTPCodec::NoPadding, true);
  // no EOF, sess will close when stream detaches
  transport_->addReadEvent(id, writeBuf_.move(), false);
  auto event = co_await co_awaitTry(readBodyEventNoSuspend(*responseSource));
  EXPECT_FALSE(event.hasException());
  EXPECT_EQ(event->eventType, HTTPBodyEvent::BODY);
  EXPECT_EQ(event->event.body.chainLength(), 100);
  EXPECT_TRUE(event->eom);
}

CO_TEST_P_X(H1UpstreamSessionTest, SendGoaway) {
  // session_->initiateDrain() can't add Connection: close to the
  // next request, because sending a request after drain will fail.
  // Have to add Connection: close manually
  auto req = HTTPFixedSource::makeFixedRequest("/");
  req->msg_->getHeaders().set(HTTP_HEADER_CONNECTION, "close");
  auto responseSource = co_await co_awaitTry(session_->sendRequest(req));
  XCHECK(!responseSource.hasException());
  auto id = *responseSource->getStreamID();
  serializeResponseHeader(id, 200, false);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  auto resp = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
  serverCodec_->generateBody(
      writeBuf_, id, makeBuf(100), HTTPCodec::NoPadding, true);
  // no EOF, sess will close when stream detaches
  transport_->addReadEvent(id, writeBuf_.move(), false);
  auto event = co_await co_awaitTry(readBodyEventNoSuspend(*responseSource));
  EXPECT_FALSE(event.hasException());
  EXPECT_EQ(event->eventType, HTTPBodyEvent::BODY);
  EXPECT_EQ(event->event.body.chainLength(), 100);
  EXPECT_TRUE(event->eom);
}

CO_TEST_P_X(H1UpstreamSessionTest, DrainAfterFailedUpgrade) {
  auto req = HTTPFixedSource::makeFixedRequest("/websocket");
  req->msg_->setEgressWebsocketUpgrade();
  auto responseSource = co_await co_awaitTry(session_->sendRequest(req));
  XCHECK(!responseSource.hasException());
  auto id = *responseSource->getStreamID();
  serializeResponseHeader(id, 500, /*eom*/ true);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  {
    EXPECT_CALL(lifecycleObs_, onDrainStarted(_)).Times(1);
    auto resp = co_await co_awaitTry(responseSource->readHeaderEvent());
    EXPECT_FALSE(resp.hasException());
    EXPECT_EQ(resp->headers->getStatusCode(), 500);
    EXPECT_CALL(lifecycleObs_, onDrainStarted(_)).Times(0);
  }
}

TEST_P(H2QUpstreamSessionTest, ReceiveGoaway) {
  // Client receives GOAWAY, no EOF.  Even though the peer is allowing for
  // more streams, this session doesn't have any in-flight, so it egresses
  // it's own GOAWAY and closed the connection.
  generateGoaway(1, ErrorCode::NO_ERROR);
}

TEST_P(HTTPUpstreamSessionTest, SendGoaway) {
  // Client sends a GOAWAY, codec becomes unusable and session exits
  session_->initiateDrain();
}

TEST_P(HTTPUpstreamSessionTest, LifecycleObserverTestNullptr) {
  EXPECT_DEATH(session_->addLifecycleObserver(nullptr), "Check failed:");
}

TEST_P(HTTPUpstreamSessionTest, LifecycleObserverTests) {
  auto obs1 = std::make_unique<StrictMock<MockLifecycleObserver>>();
  auto obs2 = std::make_unique<StrictMock<MockLifecycleObserver>>();

  // helper function to clear both observers
  auto clearObservers = [&]() {
    session_->removeLifecycleObserver(obs1.get());
    session_->removeLifecycleObserver(obs2.get());
  };

  /**
   * test #1: adding obs2 during a ::deliverLifecycleCallback invocation
   * will not call back into obs2 for that event / during that iteration
   */
  {
    EXPECT_CALL(*obs1, onAttached(_));
    session_->addLifecycleObserver(obs1.get());

    EXPECT_CALL(*obs1, onActivateConnection(_)).Times(1);
    EXPECT_CALL(*obs1, onTransactionAttached(_)).WillOnce([&]() {
      EXPECT_CALL(*obs2, onAttached(_));
      session_->addLifecycleObserver(obs2.get());
    });
    EXPECT_CALL(*obs2, onTransactionAttached(_)).Times(0);

    // both should expect onTransactionDetached & onDeactivateConnection
    // callbacks
    EXPECT_CALL(*obs1, onTransactionDetached(_));
    EXPECT_CALL(*obs2, onTransactionDetached(_));
    EXPECT_CALL(*obs1, onDeactivateConnection(_));
    EXPECT_CALL(*obs2, onDeactivateConnection(_));

    session_->reserveRequest();
    clearObservers();
  }

  {
    /**
     * test #2: deleting obs2 prior to its subsequent
     * ::deliverLifecycleCallback invocation should work as expected
     */
    EXPECT_CALL(*obs1, onAttached(_));
    EXPECT_CALL(*obs2, onAttached(_));
    session_->addLifecycleObserver(obs1.get());
    session_->addLifecycleObserver(obs2.get());

    // both should receive the first ::onActivateConnection &
    // ::onTransactionAttached callback
    EXPECT_CALL(*obs1, onActivateConnection(_)).Times(1);
    EXPECT_CALL(*obs2, onActivateConnection(_)).Times(1);
    EXPECT_CALL(*obs1, onTransactionAttached(_))
        .WillOnce([&]() { session_->removeLifecycleObserver(obs2.get()); })
        .RetiresOnSaturation();
    EXPECT_CALL(*obs2, onTransactionAttached(_)).Times(1);

    // only obs1 should receive the detach/deactivate callback
    EXPECT_CALL(*obs1, onTransactionDetached(_));
    EXPECT_CALL(*obs1, onDeactivateConnection(_));
    EXPECT_CALL(*obs2, onTransactionDetached(_)).Times(0);

    session_->reserveRequest();
    clearObservers();
  }

  {
    /**
     * test #3: single obs1 removes itself from within ::onAttached callback;
     * this should never deliver any more
     */
    EXPECT_CALL(*obs1, onAttached(_)).WillOnce([&]() {
      session_->removeLifecycleObserver(obs1.get());
    });
    session_->addLifecycleObserver(obs1.get());
  }
}

CO_TEST_P_X(HTTPUpstreamSessionTest, BodyError) {
  auto reservation = *(session_->reserveRequest());
  auto respSource = co_await co_withExecutor(
                        &evb_,
                        session_->sendRequest(
                            new ErrorSource("super super long text", true, 11),
                            std::move(reservation)))
                        .start();

  folly::exception_wrapper ew;
  while (true) {
    auto event = co_await co_awaitTry(readBodyEventNoSuspend(respSource));
    if (event.hasException()) {
      ew = std::move(event).exception();
      break;
    } else if (event.value().eom) {
      break;
    }
  }

  CO_ASSERT_TRUE(bool(ew));
  CO_ASSERT_TRUE(ew.get_exception<HTTPError>() != nullptr);
  EXPECT_NE(ew.get_exception<HTTPError>()->code, HTTPErrorCode::READ_TIMEOUT);
  if (IS_H1()) {
    expectedError_ = TransportErrorCode::NETWORK_ERROR;
  }
}

CO_TEST_P_X(H2QUpstreamSessionTest, ReceiveGoawayWithOpenStream) {
  auto responseSource1 = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource1.hasException());
  auto responseSource2 = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  auto id = *responseSource1->getStreamID();
  generateGoaway(id, ErrorCode::NO_ERROR);
  serializeResponseHeader(id, 200, false);
  transport_->addReadEvent(id, writeBuf_.move(), false);
  auto resp = co_await co_awaitTry(responseSource1->readHeaderEvent());
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
  serverCodec_->generateBody(
      writeBuf_, id, makeBuf(100), HTTPCodec::NoPadding, true);
  // no EOF, sess will close when stream detaches
  transport_->addReadEvent(id, writeBuf_.move(), true);
  auto event = co_await co_awaitTry(readBodyEventNoSuspend(*responseSource1));
  EXPECT_FALSE(event.hasException());
  EXPECT_EQ(event->eventType, HTTPBodyEvent::BODY);
  EXPECT_EQ(event->event.body.chainLength(), 100);
  EXPECT_TRUE(event->eom);
  resp = co_await co_awaitTry(responseSource2->readHeaderEvent());
  EXPECT_TRUE(resp.hasException());
  EXPECT_EQ(getHTTPError(resp).code, HTTPErrorCode::REFUSED_STREAM);
}

// H3 GOAWAY does not have an error!
CO_TEST_P_X(H2UpstreamSessionTest, ReceiveGoawayWithError) {
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  auto id = *responseSource->getStreamID();
  generateGoaway(id, ErrorCode::PROTOCOL_ERROR);
  auto resp = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(resp.hasException());
  EXPECT_EQ(getHTTPError(resp).code, HTTPErrorCode::PROTOCOL_ERROR);
}

CO_TEST_P_X(H2UpstreamSessionTest, ReceiveGoawayBeforeRequestEgress) {
  // Send a GOAWAY with last stream ID 1 and an error code
  serverCodec_->generateGoaway(writeBuf_, 1, ErrorCode::PROTOCOL_ERROR);
  transport_->addReadEvent(writeBuf_.move(), false);
  co_await folly::coro::co_reschedule_on_current_executor;

  // Attempt to send 3 POST requests with 25000 bytes each
  auto responseSource1 = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest(
          "/", HTTPMethod::POST, makeBuf(25000))));
  auto responseSource2 = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest(
          "/", HTTPMethod::POST, makeBuf(25000))));
  auto responseSource3 = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest(
          "/", HTTPMethod::POST, makeBuf(25000))));

  co_await folly::coro::co_reschedule_on_current_executor;
  co_await folly::coro::co_reschedule_on_current_executor;
  co_await folly::coro::co_reschedule_on_current_executor;
}

TEST_P(HTTPUpstreamSessionTest, IdleTimeoutNoStreams) {
  // Just run the loop.  The session should idle timeout, drain and close.
  session_->setConnectionReadTimeout(std::chrono::milliseconds(250));
}

// Need to change the test to get let the mock socket create the stream first
CO_TEST_P_X(H12UpstreamSessionTest, WriteTimeout) {
  // Pause writes, so the session sees a write timeout while sending a request.
  // The response header event is an error.
  session_->setWriteTimeout(std::chrono::milliseconds(250));
  // TODO: not quite right but I don't know the id before sendRequest
  transport_->pauseWrites(0);
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(headerEvent.hasException());
  // Socket is closed with RST for H1 and H2
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
}

CO_TEST_P_X(H2UpstreamSessionTest, WriteErrorWithCompleteStreams) {
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  session_->initiateDrain();
  // Flush request
  co_await folly::coro::co_reschedule_on_current_executor;

  // Ack the first SETTINGS sent from the test codec so it can generate more
  HTTP2Codec fakeUpstream(TransportDirection::UPSTREAM);
  fakeUpstream.generateConnectionPreface(writeBuf_);
  fakeUpstream.generateSettingsAck(writeBuf_);
  serverCodec_->onIngress(*writeBuf_.front());
  writeBuf_.move();

  // Send the real response
  serializeResponse(*responseSource->getStreamID(), 200, makeBuf(100));

  // Send another SETTINGS frame, this will trigger the write of a SETTINGS
  // ACK
  serverCodec_->generateSettings(writeBuf_);
  transport_->addReadEvent(writeBuf_.move(), false);

  // Force the write end of the transport closed, triggers a write error
  static_cast<TestUniplexTransport *>(transport_)->shutdownWrite();
  // Wait two loops, one to parse the SETTINGS, one to flush the ACK and hit
  // the write error
  co_await rescheduleN(2);

  // Now read out the buffered response, it's still fine
  co_await expectResponse(std::move(*responseSource), 200, 100, true);
  // Socket is closed with RST for H1 and H2
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
}

CO_TEST_P_X(HTTPUpstreamSessionTest, PostBodyReadTimeout) {
  // Send a request that hangs in readBodyEvent
  session_->setConnectionReadTimeout(std::chrono::milliseconds(250));
  session_->setStreamReadTimeout(std::chrono::milliseconds(250));
  auto req = std::make_unique<HTTPMessage>(getPostRequest(100));
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(new TimeoutSource(std::move(req))));
  XCHECK(!responseSource.hasException());
  auto resp = co_await co_awaitTry(folly::coro::timeoutNoDiscard(
      responseSource->readHeaderEvent(), std::chrono::milliseconds(500)));
  // Wait for a header event for 2x the read timeout -- the read timer isn't
  // running.  timeoutNoDiscard will cancel the read
  EXPECT_TRUE(resp.hasException());
  EXPECT_EQ(getHTTPError(resp).code, HTTPErrorCode::CORO_CANCELLED);
  session_->closeWhenIdle();
  onTearDown([this] { expectH1ConnectionReset(); });
}

CO_TEST_P_X(HTTPUpstreamSessionTest, PostReadTimeout) {
  session_->setConnectionReadTimeout(std::chrono::milliseconds(250));
  session_->setStreamReadTimeout(std::chrono::milliseconds(250));
  auto responseSource = co_await co_awaitTry(session_->sendRequest(
      HTTPFixedSource::makeFixedRequest("/", HTTPMethod::POST, makeBuf(100))));
  XCHECK(!responseSource.hasException());
  auto resp = co_await co_awaitTry(folly::coro::timeoutNoDiscard(
      responseSource->readHeaderEvent(), std::chrono::milliseconds(1500)));
  // Timer is running, read timeout before timeoutNoDiscard
  EXPECT_TRUE(resp.hasException());
  EXPECT_EQ(getHTTPError(resp).code, HTTPErrorCode::READ_TIMEOUT);
  session_->closeWhenIdle();
  onTearDown([this] { expectH1ConnectionReset(); });
}

CO_TEST_P_X(HTTPUpstreamSessionTest, PostReadCustomTimeout) {
  session_->setConnectionReadTimeout(std::chrono::milliseconds(250));
  session_->setStreamReadTimeout(std::chrono::seconds(20));
  auto responseSource = co_await co_awaitTry(session_->sendRequest(
      HTTPFixedSource::makeFixedRequest("/", HTTPMethod::POST, makeBuf(100))));
  XCHECK(!responseSource.hasException());
  responseSource->setReadTimeout(std::chrono::milliseconds(250));
  auto resp = co_await co_awaitTry(folly::coro::timeoutNoDiscard(
      responseSource->readHeaderEvent(), std::chrono::milliseconds(1500)));
  // Timer is running, read timeout before timeoutNoDiscard
  EXPECT_TRUE(resp.hasException());
  EXPECT_EQ(getHTTPError(resp).code, HTTPErrorCode::READ_TIMEOUT);
  session_->closeWhenIdle();
  onTearDown([this] { expectH1ConnectionReset(); });
}

CO_TEST_P_X(HTTPUpstreamSessionTest, GetReadTimeout) {
  session_->setConnectionReadTimeout(std::chrono::milliseconds(250));
  session_->setStreamReadTimeout(std::chrono::milliseconds(250));
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  auto resp = co_await co_awaitTry(folly::coro::timeoutNoDiscard(
      responseSource->readHeaderEvent(), std::chrono::milliseconds(1500)));
  // Timer is running, read timeout before timeoutNoDiscard
  EXPECT_TRUE(resp.hasException());
  EXPECT_EQ(getHTTPError(resp).code, HTTPErrorCode::READ_TIMEOUT);
  session_->closeWhenIdle();
  onTearDown([this] { expectH1ConnectionReset(); });
}

// TODO: hq flow control tests
CO_TEST_P_X(H2UpstreamSessionTest, ConnFlowControlOnBodyError) {
  // Send a request, response is larger than both the conn and stream flow
  // control window, but conn will trip first.
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  auto streamID = *responseSource->getStreamID();
  serializeResponse(streamID, 200, makeBuf(70000), false);
  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_FALSE(headerEvent.hasException());
  // Get more data to trigger before reading body events to trigger flow
  // control error.
  co_await folly::coro::co_reschedule_on_current_executor;
  auto bodyEvent =
      co_await co_awaitTry(readBodyEventNoSuspend(*responseSource));
  EXPECT_TRUE(bodyEvent.hasException());
  EXPECT_EQ(getHTTPError(bodyEvent).code, HTTPErrorCode::FLOW_CONTROL_ERROR);
}

// TODO: hq flow control tests
CO_TEST_P_X(H2UpstreamSessionTest, StreamFlowControlOnBodyError) {
  // Send a request, response is larger than stream flow control window.
  // Stream is reset, but connection is open.
  session_->setConnectionFlowControl(70000);
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  auto streamID = *responseSource->getStreamID();
  serializeResponse(streamID, 200, makeBuf(70000), false);
  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_FALSE(headerEvent.hasException());
  // Get more data before reading body events
  co_await folly::coro::co_reschedule_on_current_executor;
  auto bodyEvent =
      co_await co_awaitTry(readBodyEventNoSuspend(*responseSource));
  EXPECT_TRUE(bodyEvent.hasException());
  EXPECT_EQ(getHTTPError(bodyEvent).code, HTTPErrorCode::FLOW_CONTROL_ERROR);
  transport_->addReadEvent(nullptr, true);
  // TODO: verify graceful vs error shutdowns
}

// H2 only
TEST_P(H2UpstreamSessionTest, ConnFlowControlOnWindowUpdateError) {
  // Add a window update that overflows the conn window
  windowUpdate(std::numeric_limits<int32_t>::max());
  // expect an error?
}

// H2 only
CO_TEST_P_X(H2UpstreamSessionTest, StreamFlowControlOnWindowUpdateError) {
  // Add a window update that overflows the stream window.  The connection is
  // closed with error
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  auto streamID = *responseSource->getStreamID();
  windowUpdate(streamID, std::numeric_limits<int32_t>::max());
  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(headerEvent.hasException());
  EXPECT_EQ(getHTTPError(headerEvent).code, HTTPErrorCode::FLOW_CONTROL_ERROR);
}

TEST_P(H1UpstreamSessionTest, CodecHTTPError) {
  std::string badRequest("BLARF");
  HTTPCodec::StreamID id = 1;
  transport_->addReadEvent(id, folly::IOBuf::copyBuffer(badRequest), false);

  evb_.loop();
  expectedError_ = TransportErrorCode::NETWORK_ERROR;
}

CO_TEST_P_X(H2QUpstreamSessionTest, CodecHTTPError) {
  auto responseSource =
      co_await session_->sendRequest(HTTPFixedSource::makeFixedRequest("/"));
  auto streamID = *responseSource.getStreamID();
  HTTPMessage req = getResponse(200);
  req.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "100");
  req.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "200");
  if (isHQ()) {
    multiCodec_->addCodec(streamID);
  }
  serverCodec_->generateHeader(writeBuf_, streamID, req, true);
  transport_->addReadEvent(streamID, writeBuf_.move(), false);
  generateGoaway();

  auto headerEvent = co_await co_awaitTry(responseSource.readHeaderEvent());
  EXPECT_TRUE(headerEvent.hasException());
}

CO_TEST_P_X(H2QUpstreamSessionTest, OnDeactivateConnectionLifecycle) {
  EXPECT_CALL(lifecycleObs_, onActivateConnection(_)).Times(1);
  EXPECT_CALL(lifecycleObs_, onTransactionAttached(_)).Times(1);
  // reserving a request should invoke both activated & attached cbs
  auto res = session_->reserveRequest().value();

  EXPECT_CALL(lifecycleObs_, onTransactionAttached(_)).Times(1);
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  serializeResponse(*responseSource->getStreamID(), 200, makeBuf(100), true);

  // detaching a stream with a pending reservation should not
  // ::onDeactivateConnection

  EXPECT_CALL(lifecycleObs_, onTransactionDetached(_)).Times(1);
  EXPECT_CALL(lifecycleObs_, onDeactivateConnection(_)).Times(0);
  co_await expectResponse(std::move(*responseSource), 200, 100, true);

  // cancelling last pending stream (i.e. reservation) should invoke
  // ::onDeactivateConnection
  EXPECT_CALL(lifecycleObs_, onTransactionDetached(_)).Times(1);
  EXPECT_CALL(lifecycleObs_, onDeactivateConnection(_)).Times(1);
  res.cancel();
  transport_->addReadEvent(nullptr, true);
}

// TODO: hq push
CO_TEST_P_X(H2UpstreamSessionTest, PushPromiseParseError) {
  // send a request.  The peer sends a response that
  // includes a push promise with a parse error and complete push response.
  // send a request for GET /
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  auto streamID = *responseSource->getStreamID();

  // serialize a response with no body/EOM
  serializeResponse(streamID, 200, makeBuf(100), false);

  // Serialize a push promise, with a parsing error
  auto pushID = serverCodec_->createStream();
  HTTPHeaderSize size;
  HTTPMessage promise = getGetRequest("/push");
  promise.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "100");
  promise.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "200");
  serverCodec_->generatePushPromise(
      writeBuf_, pushID, promise, streamID, false, &size);

  // Now serialize the push response, with EOM
  serializeResponse(pushID, 200, makeBuf(100), true);

  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(headerEvent.hasException());
  EXPECT_EQ(getHTTPError(headerEvent).httpMessage->getStatusCode(), 200);
  session_->initiateDrain();
  // TODO: verify we egress a reset stream for push stream too
  onTearDown([this] { EXPECT_EQ(pushes_.size(), 0); });
}

CO_TEST_P_X(H2UpstreamSessionTest, ResetReleaseConnFlowControl) {
  // When a stream is reset it releases connection flow control.  Send two
  // POST requests.  The second one is blocked on connection flow control,
  // which is released when the first stream is reset.
  auto responseSource1 = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  auto streamID1 = *responseSource1->getStreamID();
  auto responseSource2 = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  auto streamID2 = *responseSource2->getStreamID();
  // Serialize first response, then reset it.  The second response would
  // generate a flow control error if the reset didn't release flow control
  serializeResponse(streamID1, 200, makeBuf(http2::kInitialWindow), false);
  resetStream(streamID1, ErrorCode::CANCEL);
  serializeResponse(streamID2, 200, makeBuf(100), true);
  // Because we only read 64k per loop, we see the headers and almost all the
  // body before the RST_STREAM is read.
  auto headerEvent = co_await co_awaitTry(responseSource1->readHeaderEvent());
  EXPECT_FALSE(headerEvent.hasException());
  auto bodyEvent =
      co_await co_awaitTry(readBodyEventNoSuspend(*responseSource1));
  EXPECT_FALSE(bodyEvent.hasException());
  bodyEvent = co_await co_awaitTry(readBodyEventNoSuspend(*responseSource1));
  EXPECT_TRUE(bodyEvent.hasException());
  session_->initiateDrain();
  co_await expectResponse(std::move(*responseSource2), 200, 100, true);
}

CO_TEST_P_X(H2UpstreamSessionTest, FreeConnFlowControlAfterRst) {
  // After a stream is reset and state is gone, connection flow control is
  // still released.

  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  // Send response headers
  auto streamID = *responseSource->getStreamID();
  serializeResponse(streamID, 200, nullptr, false);
  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_FALSE(headerEvent.hasException());

  // Abandon and wait for stream to abort and detach state
  responseSource->stopReading();
  co_await rescheduleN(1);

  // Send some body
  serverCodec_->generateBody(writeBuf_,
                             streamID,
                             makeBuf(http2::kInitialWindow / 2),
                             HTTPCodec::NoPadding,
                             true);
  transport_->addReadEvent(streamID, writeBuf_.move(), true);
  co_await rescheduleN(2);

  // The session should release this flow control immediately
  EXPECT_CALL(callbacks_, onWindowUpdate(_, http2::kInitialWindow / 2));
  parseOutputUniplex();
}

CO_TEST_P_X(H2UpstreamSessionTest, EgressWhileWritesBlocked) {
  co_await folly::coro::co_reschedule_on_current_executor;
  auto uniplexTransport = static_cast<TestUniplexTransport *>(transport_);
  // Preface/SETTINGS
  EXPECT_EQ(transportState_.writeEvents.size(), 1);
  auto responseSource1 = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource1.hasException());
  // Pause writes and let the egress go to the writeBuf
  transport_->pauseWrites(0);
  co_await folly::coro::co_reschedule_on_current_executor;
  // Headers are present for first req
  EXPECT_EQ(transportState_.writeEvents.size(), 2);
  auto responseSource2 = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  co_await folly::coro::co_reschedule_on_current_executor;
  // Headers for second req are in writeBuf_ but not in transport
  EXPECT_EQ(transportState_.writeEvents.size(), 2);
  uniplexTransport->resumeWrites();
  co_await folly::coro::co_reschedule_on_current_executor;
  // Headers for second req are in transport
  EXPECT_EQ(transportState_.writeEvents.size(), 3);
  session_->initiateDrain();
}

TEST_P(HTTPUpstreamSessionTest, SessionCancelledNoStreams) {
  cancellationSource_.requestCancellation();
  evb_.loop();
  if (isHQ()) {
    // Questionnable that this results in a connection error
    expectedError_ = TransportErrorCode::NETWORK_ERROR;
  }
  expectH1ConnectionReset();
}

CO_TEST_P_X(HTTPUpstreamSessionTest, SessionCancelledWithStream) {
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  cancellationSource_.requestCancellation();
  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(headerEvent.hasException());
  if (isHQ()) {
    expectedError_ = TransportErrorCode::NETWORK_ERROR;
  }
  onTearDown([this] { expectH1ConnectionReset(); });
}

TEST_P(HTTPUpstreamSessionTest, SessionDropNoStreams) {
  session_->dropConnection();
  evb_.loopOnce();
  if (isHQ()) {
    loopN(3);
    // Questionnable that this results in a connection error
    expectedError_ = TransportErrorCode::NETWORK_ERROR;
  }
  expectH1ConnectionReset();
}

CO_TEST_P_X(HTTPUpstreamSessionTest, SessionDropWithStream) {
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  session_->dropConnection();
  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(headerEvent.hasException());
  co_await folly::coro::co_reschedule_on_current_executor;
  if (isHQ()) {
    co_await rescheduleN(3);
    expectedError_ = TransportErrorCode::NETWORK_ERROR;
  }
  expectH1ConnectionReset();
}

CO_TEST_P_X(HTTPUpstreamSessionTest, StreamIngressCompleteNoRST) {
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  auto streamID = *responseSource->getStreamID();
  serializeResponse(streamID, 200, makeBuf(100), true);
  session_->initiateDrain();
  // Suspend this coro so the session reads the response
  co_await folly::coro::co_reschedule_on_current_executor;
  responseSource->stopReading();
  // TODO: verify no RST_STREAM is sent
}

CO_TEST_P_X(HTTPUpstreamSessionTest, CancelBeforeSendingHeaders) {
  folly::CancellationSource cancelSource;
  auto coro = [this]() -> folly::coro::Task<void> {
    auto req = std::make_unique<HTTPMessage>(getPostRequest(10));
    auto responseSource = co_await co_awaitTry(
        session_->sendRequest(new TimeoutSource(std::move(req),
                                                /*timeoutHeaders=*/true,
                                                /*errorOnCancel=*/false)));
    EXPECT_TRUE(responseSource.hasException());
    EXPECT_EQ(getHTTPError(responseSource).code, HTTPErrorCode::CORO_CANCELLED);
  };
  co_withExecutor(
      &evb_, folly::coro::co_withCancellation(cancelSource.getToken(), coro()))
      .start();
  co_await folly::coro::co_reschedule_on_current_executor;
  cancelSource.requestCancellation();
  session_->initiateDrain();
}

CO_TEST_P_X(HTTPUpstreamSessionTest, CancelDuringPostBody) {
  folly::CancellationSource cancelSource;
  auto coro = [this]() -> folly::coro::Task<void> {
    auto req = std::make_unique<HTTPMessage>(getPostRequest(10));
    auto responseSource = co_await co_awaitTry(session_->sendRequest(
        new TimeoutSource(std::move(req), /*timeoutHeaders=*/false)));
    EXPECT_FALSE(responseSource.hasException());
    auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
    EXPECT_TRUE(headerEvent.hasException());
    auto err = getHTTPError(headerEvent);
    // the read coro was cancelled
    EXPECT_EQ(err.code, HTTPErrorCode::CORO_CANCELLED);
    EXPECT_TRUE(err.msg.ends_with("Read cancelled"));
  };
  co_withExecutor(
      &evb_, folly::coro::co_withCancellation(cancelSource.getToken(), coro()))
      .start();
  co_await folly::coro::co_reschedule_on_current_executor;
  cancelSource.requestCancellation();
  session_->initiateDrain();
  onTearDown([this] { expectH1ConnectionReset(); });
}

CO_TEST_P_X(HTTPUpstreamSessionTest, SupportsMoreTransactions) {
  EXPECT_TRUE(session_->supportsMoreTransactions());
  session_->setMaxConcurrentOutgoingStreams(1);
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  EXPECT_FALSE(responseSource.hasException());
  EXPECT_FALSE(session_->supportsMoreTransactions());
  serializeResponse(*responseSource->getStreamID(), 200, makeBuf(100), true);
  co_await expectResponse(std::move(*responseSource), 200, 100, true);
  EXPECT_TRUE(session_->supportsMoreTransactions());
  session_->initiateDrain();
  EXPECT_FALSE(session_->supportsMoreTransactions());
}

CO_TEST_P_X(HTTPUpstreamSessionTest, SendRequestHeadersAvailable) {
  NiceMock<MockHTTPSource> reqSource;
  EXPECT_CALL(reqSource, readHeaderEvent).Times(0);
  EXPECT_CALL(reqSource, readBodyEvent(_)).WillOnce([&]() {
    return folly::coro::makeTask(HTTPBodyEvent{makeBuf(1500), /*inEOM=*/true});
  });
  EXPECT_CALL(reqSource, stopReading(_)).Times(0);

  HTTPHeaderEvent headerEv{
      std::make_unique<HTTPMessage>(getPostRequest(/*contentLength=*/1500)),
      /*inEOM=*/false};
  auto reservation = session_->reserveRequest().value();
  auto res = session_->sendRequest(
      std::move(reservation), *headerEv.headers, &reqSource);
  XCHECK(!res.hasError());

  EXPECT_CALL(lifecycleObs_, onTransactionDetached(_));
  serializeResponse(res->getStreamID().value(), 200, makeBuf(100), true);
  co_await HTTPSourceReader{std::move(*res)}.read();

  EXPECT_CALL(lifecycleObs_, onTransactionDetached(_));
  reservation = session_->reserveRequest().value();
  // expect error if ::sendRequest during drain
  session_->initiateDrain();
  res =
      session_->sendRequest(std::move(reservation), *headerEv.headers, nullptr);
  EXPECT_TRUE(res.hasError());
}

CO_TEST_P_X(H1UpstreamSessionTest, DrainStartOnReset) {
  // When h1 resets a stream, make sure it invokes the drain started callback
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  EXPECT_CALL(lifecycleObs_, onDrainStarted);
  responseSource->stopReading();
  onTearDown([this] { expectH1ConnectionReset(); });
}

CO_TEST_P_X(H12UpstreamSessionTest, CloseOnResponseWithoutRequest) {
  serializeResponse(1, 200, makeBuf(100), true);
  {
    EXPECT_CALL(lifecycleObs_, onDrainStarted);
    co_await folly::coro::co_reschedule_on_current_executor;
  }
  if (IS_H1()) {
    onTearDown([this] { expectH1ConnectionReset(); });
  } else {
    co_await folly::coro::co_reschedule_on_current_executor;
    EXPECT_CALL(callbacks_, onGoaway(_, ErrorCode::PROTOCOL_ERROR, _));
    parseOutputUniplex();
  }
}

CO_TEST_P_X(H1UpstreamSessionTest, TrailingGarbage) {
  // Send an incomplete request by holding the EOM until the response
  // headers come back.  There's a second response on the wire, which triggers
  // a connection error, but the body and EOM have been received so they are
  // delivered.
  folly::coro::Baton baton;
  auto onEomRequestSource = new OnEOMSource(
      HTTPFixedSource::makeFixedRequest("/", HTTPMethod::POST, makeBuf(10)),
      [&baton]() -> OnEOMSource::CallbackReturn {
        co_await baton;
        co_return folly::none;
      });
  auto responseSource =
      co_await co_awaitTry(session_->sendRequest(onEomRequestSource));
  XCHECK(!responseSource.hasException());
  auto resp = makeResponse(200, true); // eom=true prevents te: chunked
  resp.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "10");
  serializeResponseHeader(
      *responseSource->getStreamID(), std::move(resp), false);
  writeBuf_.append(makeBuf(10));

  serializeResponseHeader(
      *responseSource->getStreamID() + 1, makeResponse(200), false);
  transport_->addReadEvent(
      *responseSource->getStreamID(), writeBuf_.move(), false);

  HTTPSourceReader reader(std::move(*responseSource));
  reader
      .onHeaders([&baton](std::unique_ptr<HTTPMessage>, bool, bool) {
        baton.post();
        return HTTPSourceReader::Continue;
      })
      .onBody([](BufQueue body, bool eom) {
        EXPECT_EQ(body.chainLength(), 10);
        EXPECT_TRUE(eom);
        return HTTPSourceReader::Continue;
      });
  co_await reader.read();
  onTearDown([this] { expectH1ConnectionReset(); });
}

// H3 specific tests
CO_TEST_P_X(HQUpstreamSessionTest, CreateBidiStreamFailure) {
  // reserve request successfully
  auto reservation = session_->reserveRequest();
  XCHECK(reservation.hasValue());

  // make next call to QuicSocket::createBidirectionalStream fail
  EXPECT_CALL(*muxTransport_->socketDriver_.sock_, createBidirectionalStream(_))
      .WillOnce(Return(
          quic::make_unexpected(quic::LocalErrorCode::STREAM_LIMIT_EXCEEDED)));

  // ::sendRequest should fail accordingly
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));

  // verify exception yielded
  EXPECT_TRUE(responseSource.hasException());
  auto ex = CHECK_NOTNULL(responseSource.tryGetExceptionObject<HTTPError>());
  EXPECT_EQ(ex->code, HTTPErrorCode::REFUSED_STREAM);
}

CO_TEST_P_X(HQUpstreamSessionTest, CreateControlStreamFail) {
  // Kill the default session
  session_->closeWhenIdle();

  // Make a new muxTransport with no uni credit
  TestCoroMultiplexTransport muxTransport(&evb_, direction_);
  transport_ = &muxTransport;
  muxTransport.socketDriver_.setMaxUniStreams(0);
  auto codec = std::make_unique<hq::HQMultiCodec>(direction_);
  wangle::TransportInfo tinfo;
  auto session = HTTPCoroSession::makeUpstreamCoroSession(
      muxTransport.getSocket(), std::move(codec), std::move(tinfo));
  NiceMock<MockLifecycleObserver> infoCb;
  session->addLifecycleObserver(&infoCb);
  folly::coro::co_withCancellation(cancellationSource_.getToken(),
                                   session->run())
      .start();
  EXPECT_CALL(infoCb, onDestroy(_));

  auto responseSource = co_await co_awaitTry(
      session->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  // This request never goes through
  EXPECT_TRUE(responseSource.hasException());
  co_await rescheduleN(4);
  EXPECT_EQ(HTTP3::ErrorCode(*muxTransport.socketDriver_.getConnErrorCode()),
            HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR);
}

CO_TEST_P_X(HQUpstreamSessionTest, StopSending) {
  auto req = std::make_unique<HTTPMessage>(getPostRequest(100));
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(new TimeoutSource(std::move(req))));
  XCHECK(!responseSource.hasException());
  // Server asks client to stop sending.  Cancels the egress coro
  auto id = *responseSource->getStreamID();
  muxTransport_->socketDriver_.addStopSending(
      id, HTTP3::ErrorCode::HTTP_EXCESSIVE_LOAD);
  // Now close the ingress
  resetStream(id, ErrorCode::PROTOCOL_ERROR);
  auto resp = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(resp.hasException());
  EXPECT_EQ(getHTTPError(resp).code, HTTPErrorCode::GENERAL_PROTOCOL_ERROR);
  session_->initiateDrain();
}

CO_TEST_P_X(HQUpstreamSessionTest, StopSendingEgressComplete) {
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest(
          "/", HTTPMethod::POST, makeBuf(70000))));
  XCHECK(!responseSource.hasException());
  // Server asks client to stop sending.  Egress coro is already complete
  auto id = *responseSource->getStreamID();
  muxTransport_->socketDriver_.addStopSending(
      id, HTTP3::ErrorCode::HTTP_EXCESSIVE_LOAD);
  // Now close the ingress
  resetStream(id, ErrorCode::REFUSED_STREAM);
  auto resp = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(resp.hasException());
  EXPECT_EQ(getHTTPError(resp).code, HTTPErrorCode::REQUEST_REJECTED);
  session_->initiateDrain();
  // Grant enough conn FCW for the GOAWAY and QPACK stream
  muxTransport_->socketDriver_.setConnectionFlowControlWindow(5);
}

CO_TEST_P_X(HQUpstreamSessionTest, CancelWithStopSending) {
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  auto id = *responseSource->getStreamID();
  co_await folly::coro::co_reschedule_on_current_executor;
  responseSource->stopReading();
  serializeResponse(id, 200, makeBuf(100), true);
  co_await folly::coro::co_reschedule_on_current_executor;
  session_->initiateDrain();
}

CO_TEST_P_X(HQUpstreamSessionTest, QPACKQueuedOnClose) {
  // It takes 2 loops for the encoder stream to get established
  co_await rescheduleN(2);
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  auto id = *responseSource->getStreamID();
  auto resp = makeResponse(200);
  resp.getHeaders().add("Dynamic", "Header");
  // flush the header but not the control data
  serializeResponseHeader(id, std::move(resp), false, false);
  serverCodec_->generateBody(
      writeBuf_, id, makeBuf(100), HTTPCodec::NoPadding, true);
  transport_->addReadEvent(id, writeBuf_.move(), true);
  co_await folly::coro::co_reschedule_on_current_executor;
  // Add connection end
  transport_->addReadEvent(nullptr, true);
  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(headerEvent.hasException());
  EXPECT_EQ(getHTTPError(headerEvent).code,
            HTTPErrorCode::QPACK_DECOMPRESSION_FAILED);
}

CO_TEST_P_X(HQUpstreamSessionTest, DrainSessionOnConnectionError) {
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  // ::onConnectionError callback should invoke ::onDrainStarted
  EXPECT_CALL(lifecycleObs_, onDrainStarted(_));
  muxTransport_->socketDriver_.closeImpl(std::nullopt);
}

CO_TEST_P_X(HQUpstreamSessionTest, QPACKQueuedOnCloseNoEncoderStream) {
  // Don't wait for the encoder stream to be established
  auto responseSource = co_await co_awaitTry(
      session_->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
  XCHECK(!responseSource.hasException());
  auto id = *responseSource->getStreamID();
  auto resp = makeResponse(200);
  resp.getHeaders().add("Dynamic", "Header");
  // flush the header but not the control data
  serializeResponseHeader(id, std::move(resp), false, false);
  serverCodec_->generateBody(
      writeBuf_, id, makeBuf(100), HTTPCodec::NoPadding, true);
  transport_->addReadEvent(id, writeBuf_.move(), true);
  // Add connection end
  transport_->addReadEvent(nullptr, true);
  auto headerEvent = co_await co_awaitTry(responseSource->readHeaderEvent());
  EXPECT_TRUE(headerEvent.hasException());
  EXPECT_EQ(getHTTPError(headerEvent).code,
            HTTPErrorCode::QPACK_DECOMPRESSION_FAILED);
}

INSTANTIATE_TEST_SUITE_P(
    HTTPUpstreamSessionTest,
    HTTPUpstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_1_1}),
           TestParams({.codecProtocol = CodecProtocol::HTTP_2}),
           TestParams({.codecProtocol = CodecProtocol::HQ})),
    paramsToTestName);

INSTANTIATE_TEST_SUITE_P(
    HTTPUpstreamSessionTest,
    H1UpstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_1_1})),
    paramsToTestName);

INSTANTIATE_TEST_SUITE_P(
    HTTPUpstreamSessionTest,
    H2UpstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_2})),
    paramsToTestName);

INSTANTIATE_TEST_SUITE_P(
    HTTPUpstreamSessionTest,
    HQUpstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HQ})),
    paramsToTestName);

INSTANTIATE_TEST_SUITE_P(
    HTTPUpstreamSessionTest,
    H2QUpstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_2}),
           TestParams({.codecProtocol = CodecProtocol::HQ})),
    paramsToTestName);

INSTANTIATE_TEST_SUITE_P(
    HTTPUpstreamSessionTest,
    H12UpstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_1_1}),
           TestParams({.codecProtocol = CodecProtocol::HTTP_2})),
    paramsToTestName);

} // namespace proxygen::coro::test
