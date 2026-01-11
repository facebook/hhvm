/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/test/HTTPClientTestsCommon.h"
#include "proxygen/lib/http/coro/test/HTTPCoroSessionTests.h"
#include "proxygen/lib/http/coro/util/test/TestHelpers.h"
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportCapsuleCodec.h>
#include <proxygen/lib/http/webtransport/WtStreamManager.h>

using namespace proxygen;
using namespace testing;

using folly::coro::co_awaitTry;
using folly::coro::co_reschedule_on_current_executor;

namespace proxygen::coro::test {

constexpr auto kWtSettings = {SettingsId::ENABLE_CONNECT_PROTOCOL,
                              SettingsId::WT_MAX_SESSIONS};

struct WtCapsuleCodecCallback : public WebTransportCapsuleCodec::Callback {
  ~WtCapsuleCodecCallback() override = default;
  void onWTResetStreamCapsule(WTResetStreamCapsule capsule) noexcept override {
    rst.emplace(capsule);
    baton.post();
  }
  void onWTStopSendingCapsule(WTStopSendingCapsule capsule) noexcept override {
    ss.emplace(capsule);
    baton.post();
  }
  void onWTStreamCapsule(WTStreamCapsule capsule) noexcept override {
    XLOG(DBG4)
        << __func__ << "; id=" << capsule.streamId << "; len="
        << (capsule.streamData ? capsule.streamData->computeChainDataLength()
                               : 0);
    stream.emplace(std::move(capsule));
    baton.post();
  }
  void onWTMaxDataCapsule(WTMaxDataCapsule capsule) noexcept override {
    md.emplace(capsule);
    baton.post();
  }
  void onWTMaxStreamDataCapsule(
      WTMaxStreamDataCapsule capsule) noexcept override {
    msd.emplace(capsule);
    baton.post();
  }
  void onWTMaxStreamsBidiCapsule(
      WTMaxStreamsCapsule capsule) noexcept override {
    bidiMaxStreams.emplace(capsule);
    baton.post();
  }
  void onWTMaxStreamsUniCapsule(WTMaxStreamsCapsule capsule) noexcept override {
    uniMaxStreams.emplace(capsule);
    baton.post();
  }

  void onWTDataBlockedCapsule(WTDataBlockedCapsule) noexcept override {
  }
  void onWTStreamDataBlockedCapsule(
      WTStreamDataBlockedCapsule) noexcept override {
  }
  void onPaddingCapsule(PaddingCapsule) noexcept override {
  }
  void onWTStreamsBlockedBidiCapsule(
      WTStreamsBlockedCapsule) noexcept override {
  }
  void onWTStreamsBlockedUniCapsule(WTStreamsBlockedCapsule) noexcept override {
  }
  void onDatagramCapsule(DatagramCapsule) noexcept override {
  }
  void onCloseWTSessionCapsule(
      CloseWebTransportSessionCapsule) noexcept override {
  }
  void onDrainWTSessionCapsule(
      DrainWebTransportSessionCapsule) noexcept override {
  }
  void onConnectionError(
      WebTransportCapsuleCodec::ErrorCode) noexcept override {
    XLOG(FATAL) << "conn error";
  }
  void onCapsule(uint64_t capsuleType,
                 uint64_t capsuleLength) noexcept override {
    XLOG(DBG4) << __func__ << "; capsuleType=" << capsuleType
               << "; capsuleLength=" << capsuleLength;
  }

  folly::coro::Task<void> waitForEvent() {
    co_await baton;
    baton.reset();
  }

  std::optional<CloseWebTransportSessionCapsule> close;
  std::optional<DrainWebTransportSessionCapsule> drain;
  std::optional<WTResetStreamCapsule> rst;
  std::optional<WTStopSendingCapsule> ss;
  std::optional<WTStreamCapsule> stream;
  std::optional<WTMaxDataCapsule> md;
  std::optional<WTMaxStreamDataCapsule> msd;
  std::optional<WTMaxStreamsCapsule> bidiMaxStreams;
  std::optional<WTMaxStreamsCapsule> uniMaxStreams;

 private:
  folly::coro::Baton baton;
};

struct WtHandler : public WebTransportHandler {
  WtHandler() = default;
  void onNewUniStream(
      WebTransport::StreamReadHandle* readHandle) noexcept override {
    ctx->peerStreams.push_back({.readHandle = readHandle});
  }
  void onNewBidiStream(
      WebTransport::BidiStreamHandle bidiHandle) noexcept override {
    ctx->peerStreams.push_back(bidiHandle);
  }
  void onDatagram(std::unique_ptr<folly::IOBuf> datagram) noexcept override {
  }
  void onSessionEnd(folly::Optional<uint32_t> error) noexcept override {
    ctx->err = error.value_or(0);
  }
  void onSessionDrain() noexcept override {
  }
  void onWebTransportSession(
      std::shared_ptr<WebTransport> wtSession) noexcept override {
    this->wtSession = std::move(wtSession);
  }

  struct Ctx {
    std::vector<WebTransport::BidiStreamHandle> peerStreams;
    folly::Optional<uint32_t> err;
  };
  std::shared_ptr<Ctx> ctx = std::make_shared<Ctx>();
  std::shared_ptr<WebTransport> wtSession;
};

class HttpWtUpstreamSessionTest : public HTTPCoroSessionTest {
 public:
  HttpWtUpstreamSessionTest()
      : HTTPCoroSessionTest(TransportDirection::UPSTREAM),
        serverCodec_(peerCodec_.get()) {
  }

  void SetUp() override {
    // need to set wt settings before HTTPCoroSession is constructed
    initSelfCodec_ = [this](HTTPCodec&) { setWtSupport(true); };
    HTTPCoroSessionTest::setUp();
  }

 protected:
  // hacks to enable/disable wt
  std::array<HTTPSettings*, 2> getHttpSettings() {
    return {const_cast<HTTPSettings*>(codec_->getIngressSettings()),
            codec_->getEgressSettings()};
  }

  void setWtSupport(bool enabled) {
    for (auto httpSettings : getHttpSettings()) {
      for (const auto wtSetting : kWtSettings) {
        httpSettings->setSetting(wtSetting, int(enabled));
      }
    }
  }

  // TODO(@damlaj): derive from HTTPUpstreamSessionTest to reduce code
  // duplication
  HTTPMessage makeResponse(uint16_t statusCode) {
    HTTPMessage resp;
    resp.setHTTPVersion(1, 1);
    resp.setStatusCode(statusCode);
    return resp;
  }

  void deliverRespHeaders(HTTPCodec::StreamID id,
                          const HTTPMessage& resp,
                          bool eom = true) {
    serverCodec_->generateHeader(writeBuf_, id, resp, eom);
    // @lint-ignore CLANGTIDY
    transport_->addReadEvent(id, writeBuf_.move(), /*eof=*/false);
  }

  void deliverRstStream(HTTPCodec::StreamID id) {
    serverCodec_->generateRstStream(writeBuf_, id, ErrorCode::CANCEL);
    // @lint-ignore CLANGTIDY
    transport_->addReadEvent(id, writeBuf_.move(), /*eof=*/false);
  }

  HTTPCodec* serverCodec_{nullptr};
};

using H2WtUpstreamSessionTest = HttpWtUpstreamSessionTest;

CO_TEST_P_X(H2WtUpstreamSessionTest, Simple) {
  // valid wt req
  HTTPMessage msg;
  msg.setMethod(HTTPMethod::CONNECT);
  msg.setUpgradeProtocol("webtransport");

  auto reservation = session_->reserveRequest();
  auto fut = co_withExecutor(&evb_,
                             session_->sendWtReq(std::move(*reservation),
                                                 msg,
                                                 std::make_unique<WtHandler>()))
                 .start();

  MockLifecycleObserver obs;
  session_->addLifecycleObserver(&obs);
  folly::coro::Baton waitForHeaders;
  EXPECT_CALL(obs, onIngressMessage(_, _)).WillOnce([&]() {
    waitForHeaders.post();
    session_->removeLifecycleObserver(&obs);
  });

  // serialize non-final 1xx continue
  deliverRespHeaders(/*id=*/1, makeResponse(100), /*eom=*/false);
  co_await waitForHeaders;     // wait for session to parse 1xx resp headers
  co_await rescheduleN(2);     // for good measure
  EXPECT_FALSE(fut.isReady()); // fut only resolves once final headers are rx'd

  // serialize final 2xx
  deliverRespHeaders(/*id=*/1, makeResponse(200), /*eom=*/true);
  auto res = co_await co_awaitTry(std::move(fut));

  EXPECT_TRUE(res->resp->is2xxResponse());
  EXPECT_NE(res->wt, nullptr);
  res->wt->closeSession(/*error=*/folly::none);
}

CO_TEST_P_X(H2WtUpstreamSessionTest, Non2xxResp) {
  // valid wt req
  HTTPMessage msg;
  msg.setMethod(HTTPMethod::CONNECT);
  msg.setUpgradeProtocol("webtransport");

  auto reservation = session_->reserveRequest();
  auto fut = co_withExecutor(&evb_,
                             session_->sendWtReq(std::move(*reservation),
                                                 msg,
                                                 std::make_unique<WtHandler>()))
                 .start();

  // server rejecting CONNECT (i.e. sending 5xx) => res->wt == nullptr
  deliverRespHeaders(/*id=*/1, makeResponse(500), /*eom=*/false);
  auto res = co_await co_awaitTry(std::move(fut));
  EXPECT_TRUE(res->resp->is5xxResponse());
  EXPECT_EQ(res->wt, nullptr);
}

CO_TEST_P_X(H2WtUpstreamSessionTest, WtUpgradeReqRstErr) {
  // receiving a rst_stream when waiting for 2xx should propagate the
  // ::readHeaderEvent error
  HTTPMessage msg;
  msg.setMethod(HTTPMethod::CONNECT);
  msg.setUpgradeProtocol("webtransport");

  auto reservation = session_->reserveRequest();
  evb_.runAfterDelay([this]() { deliverRstStream(/*id=*/1); },
                     /*milliseconds=*/50);
  auto res = co_await co_awaitTry(session_->sendWtReq(
      std::move(*reservation), msg, std::make_unique<WtHandler>()));
  auto* ex = res.tryGetExceptionObject<HTTPError>();
  EXPECT_TRUE(ex && ex->code == HTTPErrorCode::CANCEL);
}

CO_TEST_P_X(H2WtUpstreamSessionTest, SendInvalidWtReq) {
  HTTPMessage msg;
  msg.setMethod(HTTPMethod::GET);

  {
    // invalid reservation
    HTTPCoroSession::RequestReservation reservation;
    auto res = co_await co_awaitTry(session_->sendWtReq(
        std::move(reservation), msg, std::make_unique<WtHandler>()));
    auto* ex = res.tryGetExceptionObject<HTTPError>();
    EXPECT_TRUE(ex && ex->code == HTTPErrorCode::INTERNAL_ERROR);
  }

  {
    // invalid msg
    auto reservation = session_->reserveRequest();
    auto res = co_await co_awaitTry(session_->sendWtReq(
        std::move(*reservation), msg, std::make_unique<WtHandler>()));
    auto* ex = res.tryGetExceptionObject<HTTPError>();
    EXPECT_TRUE(ex && ex->code == HTTPErrorCode::INTERNAL_ERROR);
  }

  {
    setWtSupport(false);
    // unsupported webtransport (settings not set)
    msg.setMethod(HTTPMethod::CONNECT);
    msg.setUpgradeProtocol("webtransport");
    auto reservation = session_->reserveRequest();
    auto res = co_await co_awaitTry(session_->sendWtReq(
        std::move(*reservation), msg, std::make_unique<WtHandler>()));
    auto* ex = res.tryGetExceptionObject<HTTPError>();
    EXPECT_TRUE(ex && ex->code == HTTPErrorCode::INTERNAL_ERROR);
  }
}

// only http/2 WebTransport tests for now
INSTANTIATE_TEST_SUITE_P(
    HttpWtUpstreamSessionTest,
    H2WtUpstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_2})),
    paramsToTestName);

// unit tests where a wt session is already successfully established
class WtTest : public H2WtUpstreamSessionTest {
 public:
  WtTest() = default;

  void SetUp() override {
    H2WtUpstreamSessionTest::SetUp();
    loopN(2);
    establishWtSession();

    /**
     * Every HTTPCoroSession upstream http/2 transport write is piped into the
     * peer http/2 codec, which subsequently pipes every http/2 body chunk into
     * WebTransportCapsuleCodec.
     */
    EXPECT_CALL(lifecycleObs_, onWrite(_, _)).WillRepeatedly([this]() {
      peerCodec_->onIngress(*coalesceWrites());
    });
    EXPECT_CALL(callbacks_, onBody(1, _, _))
        .WillRepeatedly([this](auto id, auto buf, auto padding) {
          XLOG(DBG5) << "::onBody len=" << buf->computeChainDataLength();
          wtCodec.onIngress(buf->clone(), false);
        });
    EXPECT_CALL(callbacks_, onMessageComplete(1, _)).WillRepeatedly([this]() {
      XLOG(DBG5) << "::onMessageComplete";
      wtCodec.onIngress(nullptr, true);
    });
    coroTp = CHECK_NOTNULL(dynamic_cast<TestCoroTransport*>(transport_));
  }

  void TearDown() override {
    wt->closeSession(folly::none);
  }

  void establishWtSession() {
    HTTPMessage msg;
    msg.setMethod(HTTPMethod::CONNECT);
    msg.setUpgradeProtocol("webtransport");

    auto handler = std::make_unique<WtHandler>();
    wtHandlerCtx = handler->ctx;

    // send/serialize CONNECT request
    auto reservation = session_->reserveRequest();
    auto fut =
        co_withExecutor(&evb_,
                        session_->sendWtReq(
                            std::move(*reservation), msg, std::move(handler)))
            .start();
    evb_.loopOnce(); // serialize request

    // release 1mb of http/2 conn- and stream-level fc
    constexpr uint32_t kInitWindowSize = 1ul << 20; // 1MiB
    session_->onWindowUpdate(0, kInitWindowSize);
    session_->onWindowUpdate(1, kInitWindowSize);

    // serialize final 2xx
    deliverRespHeaders(/*id=*/1, makeResponse(200), /*eom=*/false);
    auto res = folly::coro::blockingWait(std::move(fut), &evb_);
    wt = std::move(res.wt);
    // EXPECT_TRUE(wtHandler->wtSession);
  }

  std::unique_ptr<folly::IOBuf> coalesceWrites() {
    folly::IOBuf buf;
    auto writeEvents = std::move(coroTp->state_->writeEvents);
    for (auto& ev : writeEvents) {
      buf.appendToChain(ev.move());
    }
    return buf.pop();
  }

  void deliverWtData(std::unique_ptr<folly::IOBuf> wtData, bool eom = false) {
    peerCodec_->generateBody(writeBuf_,
                             1,
                             std::move(wtData),
                             /*padding=*/folly::none,
                             /*eom=*/eom);
    // @lint-ignore CLANGTIDY
    transport_->addReadEvent(1, writeBuf_.move(), /*eof=*/false);
  }

  void grantMaxData(uint64_t streamId, uint64_t offset) {
    if (streamId == detail::kInvalidVarint) {
      writeWTMaxData(wtBuf, {offset});
    } else {
      writeWTMaxStreamData(wtBuf,
                           {.streamId = streamId, .maximumStreamData = offset});
    }
    deliverWtData(wtBuf.move());
  }

  std::shared_ptr<WebTransport> wt;
  // wtCodec & wtCodecCb are used for parsing capsules; simulating a peer
  // receiving data from client
  WtCapsuleCodecCallback wtCodecCb;
  WebTransportCapsuleCodec wtCodec{&wtCodecCb, CodecVersion::H2};
  TestCoroTransport* coroTp{nullptr};
  std::shared_ptr<const WtHandler::Ctx> wtHandlerCtx;
  // wtBuf contains the serialize wt capsules; simulating a peer sending data
  // to client
  folly::IOBufQueue wtBuf{folly::IOBufQueue::cacheChainLength()};
};

/**
 * Tests the following egress related behaviours:
 *  - defaults to zero wt_initial_max_streams_uni/bidi; cannot create uni/bidi
 *    streams until we receive a max_streams capsule
 *
 *  - dequeues are blocked until connection- and stream-level flow
 *    control credit is received from peer
 *
 *  - writing only fin=true even if blocked on flow control should still go thru
 *
 *  - backpressure signal is irrelevant of peer fc credit and is resolved after
 *    the http/2 loop has dequeued from WtStreamManager
 */
CO_TEST_P_X(WtTest, SimpleUniEgress) {
  XCHECK(wt);

  // no available uni/bidi streams
  auto createStream = wt->createUniStream();
  EXPECT_TRUE(createStream.hasError());

  // asynchronously advertise max_streams
  evb_.runInLoop([&]() {
    writeWTMaxStreams(wtBuf, /*capsule=*/{1}, /*isBidi=*/false);
    deliverWtData(wtBuf.move());
  });

  co_await wt->awaitUniStreamCredit();

  // next awaitUniStreamCredit should be synchronously available
  wt->awaitUniStreamCredit().isReady();

  // peer advertised uni credit => ::createUniStream now yields handle
  createStream = wt->createUniStream();
  CHECK(createStream.hasValue());
  auto* wh = createStream.value();
  auto id = wh->getID();

  // fill up egress buffer => writes blocked
  constexpr uint16_t kBufLen = 65'535;
  auto writeRes =
      wt->writeStreamData(id, makeBuf(kBufLen), /*fin=*/false, nullptr);
  EXPECT_EQ(writeRes.value(), WebTransport::FCState::BLOCKED);

  // awaitWritable is only resolved after the data has been dequeued from
  // WtStreamManager
  auto awaitWritable = wt->awaitWritable(id);
  EXPECT_FALSE(awaitWritable->isReady());

  // asynchronously advertise max_data (default peer value when not present in
  // http settings is assumed to be 0)
  evb_.runInLoop([&]() {
    // grant conn- and stream-level fc
    grantMaxData(detail::kInvalidVarint, kBufLen);
    grantMaxData(id, kBufLen);
  });

  // wait for wt capsule codec callback to fire
  co_await wtCodecCb.waitForEvent();

  // validate we've rx'd a wt_stream capsule with expected values
  auto streamEvent = std::exchange(wtCodecCb.stream, {});
  XCHECK(streamEvent.has_value());
  EXPECT_EQ(streamEvent->streamId, id);
  EXPECT_EQ(streamEvent->streamData->computeChainDataLength(), kBufLen);

  // when WtStreamManager dequeued the buffered data, awaitWritable is
  // resolved
  EXPECT_TRUE(awaitWritable->isReady() && awaitWritable->value() == kBufLen);

  // blocked on both connection- and stream-level fc; buffered data will not be
  // dequeued from WtStreamManager
  wt->writeStreamData(
      id, makeBuf(1), /*fin=*/false, /*deliveryCallback=*/nullptr);

  co_await rescheduleN(4);
  EXPECT_FALSE(wtCodecCb.stream.has_value()); // no data has been written

  // release 1 byte of stream-level wt fc; buffered data will still not be
  // dequeued from WtStreamManager as we're still blocked on connection-level fc
  grantMaxData(id, kBufLen + 1);
  co_await rescheduleN(5);
  EXPECT_FALSE(wtCodecCb.stream.has_value()); // no data has been written

  // release 1 byte of connection-level wt fc; buffered data will be dequeued
  // from WtStreamManager
  grantMaxData(detail::kInvalidVarint, kBufLen + 1);
  co_await wtCodecCb.waitForEvent(); // ouch, this needs 5 loops otherwise
                                     // (i.e. co_await rescheduleN(5))
  streamEvent = std::exchange(wtCodecCb.stream, {});
  XCHECK(streamEvent.has_value());
  EXPECT_EQ(streamEvent->streamId, id);
  EXPECT_EQ(streamEvent->streamData->computeChainDataLength(), 1);

  // blocked on both connection- and stream-level fc; however if just fin=true,
  // this should be dequeued regardless of peer fc credit
  wt->writeStreamData(id, nullptr, /*fin=*/true, /*deliveryCallback=*/nullptr);

  co_await wtCodecCb.waitForEvent();
  streamEvent = std::exchange(wtCodecCb.stream, {});
  XCHECK(streamEvent.has_value());
  EXPECT_EQ(streamEvent->streamId, id);
  EXPECT_EQ(streamEvent->streamData->computeChainDataLength(), 0);
  EXPECT_EQ(streamEvent->fin, true);

  co_return;
}

/**
 * Tests the following ingress related behaviours:
 *  - Receiving data on a new stream id implicitly creates the data
 *
 *  - When bytes read have exceeded half the conn- and stream-level flow control
 *    advertised to the peer, the endpoint sends a max_data & max_stream_data
 *    frame respectively
 */
CO_TEST_P_X(WtTest, SimpleUniIngress) {
  XCHECK(wt);
  constexpr uint16_t kBufLen = 65'535;
  constexpr uint16_t kIngressId = 3;

  // send kBufLen / 2 bytes of data, ensure client is able to read
  writeWTStream(wtBuf,
                WTStreamCapsule{.streamId = kIngressId,
                                .streamData = makeBuf(kBufLen / 2 + 1),
                                .fin = false});
  deliverWtData(wtBuf.move());

  // wait until we get a peer stream
  while (wtHandlerCtx->peerStreams.empty()) {
    co_await folly::coro::co_reschedule_on_current_executor;
  }
  // ingress only => writeHandle == nullptr
  auto handle = wtHandlerCtx->peerStreams.at(0);
  EXPECT_FALSE(handle.writeHandle);
  EXPECT_TRUE(handle.readHandle);

  auto read = wt->readStreamData(kIngressId);
  EXPECT_TRUE(read->isReady());
  EXPECT_EQ(read->value().fin, false);
  EXPECT_EQ(read->value().data->computeChainDataLength(), kBufLen / 2 + 1);

  // consuming half of advertised rwnd issues MaxData & MaxStreamData to peer
  co_await wtCodecCb.waitForEvent();
  EXPECT_TRUE(wtCodecCb.md.has_value() && wtCodecCb.msd.has_value());

  // when receiving a rst_stream, read should resolve an exc
  read = wt->readStreamData(kIngressId);
  EXPECT_TRUE(read.hasValue());

  // asynchronously deliver reset stream
  evb_.runInLoop([&]() {
    writeWTResetStream(wtBuf,
                       {/*streamId=*/.streamId = kIngressId,
                        .appProtocolErrorCode = 0,
                        .reliableSize = 0});
    deliverWtData(wtBuf.move());
  });

  // expect read resolves with exception due to rst above
  auto readRes = co_await co_awaitTry(std::move(read).value());
  EXPECT_TRUE(readRes.hasException());

  co_return;
}

CO_TEST_P_X(WtTest, SimpleBidiEcho) {
  XCHECK(wt);
  constexpr uint16_t kBufLen = 65'535;

  // no available bidi streams
  auto createStream = wt->createBidiStream();
  EXPECT_TRUE(createStream.hasError());

  // asynchronously advertise max_streams
  evb_.runInLoop([&]() {
    writeWTMaxStreams(wtBuf, /*capsule=*/{1}, /*isBidi=*/true);
    deliverWtData(wtBuf.move());
  });

  co_await wt->awaitBidiStreamCredit();

  // next awaitBidiStreamCredit should be synchronously available
  EXPECT_TRUE(wt->awaitBidiStreamCredit().isReady());

  // peer advertised bidi credit => ::createBidiStream now yields handle
  createStream = wt->createBidiStream();
  CHECK(createStream.hasValue());
  auto handle = createStream.value();
  auto id = handle.readHandle->getID();

  // asynchronously advertise MaxData & MaxStreamData
  evb_.runInLoop([&]() {
    grantMaxData(detail::kInvalidVarint, kBufLen);
    grantMaxData(id, kBufLen);
  });

  // asynchronously deliver varius wt frames to exercise codepaths
  evb_.runInLoop([&]() {
    writeWTDataBlocked(wtBuf, {kBufLen});
    writeWTStreamDataBlocked(
        wtBuf, {/*streamId=*/.streamId = 0, .maximumStreamData = kBufLen});
    writeWTStreamsBlocked(wtBuf, {10}, /*isBidi=*/false);
    writeWTStreamsBlocked(wtBuf, {10}, /*isBidi=*/true);
    writePadding(wtBuf, {10});
    writeDrainWebTransportSession(wtBuf);
    deliverWtData(wtBuf.move());
  });

  /**
   * in a loop – write one byte, wait for peer codec to parse byte, send the
   * byte back, and finally read the byte.
   */
  for (uint8_t idx = 0; idx < std::numeric_limits<uint8_t>::max(); idx++) {
    // write idx to stream
    auto buf = folly::IOBuf::copyBuffer(&idx, sizeof(idx));
    wt->writeStreamData(
        id, buf->clone(), /*fin=*/false, /*deliveryCallback=*/nullptr);

    // wait for peer codec to receive event
    co_await wtCodecCb.waitForEvent();

    // validate we've rx'd a wt_stream capsule with val idx
    auto streamEvent = std::exchange(wtCodecCb.stream, {});
    XCHECK(streamEvent.has_value());
    EXPECT_EQ(streamEvent->streamId, id);
    EXPECT_EQ(streamEvent->streamData->length(), 1);
    EXPECT_EQ(*streamEvent->streamData->data(), idx);

    // send the same byte back to client
    writeWTStream(wtBuf,
                  WTStreamCapsule{.streamId = id,
                                  .streamData = buf->clone(),
                                  .fin = false});
    deliverWtData(wtBuf.move());

    // expect to client to rx same byte
    auto read = co_await wt->readStreamData(id).value();
    EXPECT_EQ(*read.data->data(), idx);
  }

  // deliver both stop_sending & rst_stream, which should bidirectionally reset
  // the stream (tbd – should we wait for app to specifically invoke
  // ::resetStream before reaping state)
  evb_.runInLoop([&]() {
    writeWTStopSending(wtBuf, {.streamId = id});
    writeWTResetStream(
        wtBuf, {.streamId = id, .appProtocolErrorCode = 0, .reliableSize = 0});
    writeCloseWebTransportSession(
        wtBuf,
        {.applicationErrorCode = 0, .applicationErrorMessage = "close wt"});
    deliverWtData(wtBuf.move());
  });

  // stream is reset, ::read will return an exception
  auto read = co_await co_awaitTry(wt->readStreamData(id).value());
  EXPECT_TRUE(read.hasException());

  wt->closeSession();
}

TEST_P(WtTest, TestErrConditions) {
  XCHECK(wt);

  // default no uni credit
  auto uniRes = wt->createUniStream();
  EXPECT_TRUE(uniRes.hasError());
  // default no bidi credit
  auto bidiRes = wt->createBidiStream();
  EXPECT_TRUE(bidiRes.hasError());

  // advertise one uni&bidi stream credit to client
  writeWTMaxStreams(wtBuf, /*capsule=*/{1}, /*isBidi=*/false);
  writeWTMaxStreams(wtBuf, /*capsule=*/{1}, /*isBidi=*/true);
  deliverWtData(wtBuf.move());
  loopN(2);

  /**
   * stream 0 doesn't exist; all ops expected to fail (e.g. write, read, reset,
   * stop_sending)
   */
  constexpr uint64_t streamId = 0;
  auto read = wt->readStreamData(streamId);
  auto write = wt->writeStreamData(
      streamId, /*data=*/nullptr, /*fin=*/false, /*deliveryCallback=*/nullptr);
  auto reset = wt->resetStream(streamId, /*error=*/0);
  auto ss = wt->stopSending(streamId, /*error=*/0);
  auto await = wt->awaitWritable(streamId);

  EXPECT_TRUE(read.hasError() && write.hasError() && reset.hasError() &&
              ss.hasError() && await.hasError());

  // quic transport info is defaulted
  std::ignore = wt->getTransportInfo();

  // local & peer addr sanity checks
  const auto& localAddr = wt->getLocalAddress();
  const auto& peerAddr = wt->getPeerAddress();
  EXPECT_EQ(localAddr.getIPAddress(), peerAddr.getIPAddress());
}

CO_TEST_P_X(WtTest, PeerBidiAndTransportEom) {
  // Deliver a peer-initiated bidi stream, followed by a transport eom (http/2
  // stream eom). This should trigger shutdown of WebTransport
  XCHECK(wt);

  constexpr uint16_t kBufLen = 65'535;
  constexpr uint16_t kIngressId = 1;

  // deliver a peer-initiated bidi stream of len=kBufLen & eom=false
  writeWTStream(wtBuf,
                WTStreamCapsule{.streamId = kIngressId,
                                .streamData = makeBuf(kBufLen),
                                .fin = false});
  deliverWtData(wtBuf.move());

  // wait until client receives stream
  while (wtHandlerCtx->peerStreams.empty()) {
    co_await rescheduleN(1);
  }

  // ensure it's recognized as bidi stream
  auto& handle = wtHandlerCtx->peerStreams.at(0);
  EXPECT_TRUE(handle.readHandle && handle.writeHandle);

  // data should be available synchronously
  auto read = wt->readStreamData(kIngressId);
  EXPECT_TRUE(read->isReady() && read->value().data &&
              read->value().fin == false);

  // next read will resolve with an error after http/2 stream eom is parsed
  read = wt->readStreamData(kIngressId);

  // deliver http/2 stream eom
  deliverWtData(/*wtData=*/nullptr, /*eom=*/true);

  // wait until WebTransportHandler::onSessionEnd is invoked
  while (!wtHandlerCtx->err.has_value()) {
    co_await rescheduleN(1);
  }

  // read should have an exception now
  EXPECT_TRUE(read->hasException());
}

CO_TEST_P_X(WtTest, MaxStreamsBidiUni) {
  // open 5 of each peer-initiated uni streams and peer-initiated bidi streams;
  // close them to verify that advertising MaxStreamsBidi/MaxStreamsUni is
  // working as expected (when 50% of the limit is reached)
  XCHECK(wt);
  uint64_t nextServerBidi = 0x01, nextServerUni = 0x03;

  // send 1 byte of data on the first five server bidi and server uni streams
  for (uint8_t idx = 0; idx < 5; idx++) {
    writeWTStream(wtBuf,
                  WTStreamCapsule{.streamId = nextServerBidi,
                                  .streamData = makeBuf(1),
                                  .fin = false});
    writeWTStream(wtBuf,
                  WTStreamCapsule{.streamId = nextServerUni,
                                  .streamData = makeBuf(1),
                                  .fin = false});
    nextServerBidi += 4;
    nextServerUni += 4;
  }
  deliverWtData(wtBuf.move());

  // wait until client receives stream
  while (wtHandlerCtx->peerStreams.empty()) {
    co_await rescheduleN(1);
  }

  // bidirectionally terminate each stream
  for (auto& handle : wtHandlerCtx->peerStreams) {
    // read handle is unconditional for peer-initiated streams
    CHECK_NOTNULL(handle.readHandle)->stopSending(0);
    if (handle.writeHandle) {
      handle.writeHandle->resetStream(0);
    }
  }

  // wait for peer codec to receive event
  co_await wtCodecCb.waitForEvent();

  EXPECT_EQ(wtCodecCb.bidiMaxStreams->maximumStreams, 15);
  EXPECT_EQ(wtCodecCb.uniMaxStreams->maximumStreams, 15);

  wt->closeSession();
}

INSTANTIATE_TEST_SUITE_P(
    WtTest,
    WtTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_2})),
    paramsToTestName);

} // namespace proxygen::coro::test
//
