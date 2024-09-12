/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <boost/optional/optional_io.hpp>
#include <fizz/record/Extensions.h>
#include <fizz/record/Types.h>
#include <folly/io/Cursor.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/TimeoutManager.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/test/MockHTTPCodec.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/session/HTTPDirectResponseHandler.h>
#include <proxygen/lib/http/session/HTTPDownstreamSession.h>
#include <proxygen/lib/http/session/HTTPSession.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPSessionTest.h>
#include <proxygen/lib/http/session/test/MockSecondaryAuthManager.h>
#include <proxygen/lib/http/session/test/TestUtils.h>
#include <proxygen/lib/test/TestAsyncTransport.h>
#include <sstream>
#include <string>
#include <vector>
#include <wangle/acceptor/ConnectionManager.h>

using folly::test::MockAsyncTransport;

using namespace wangle;
using namespace fizz;
using namespace folly;
using namespace proxygen;
using namespace std;
using namespace testing;

const HTTPSettings kDefaultIngressSettings{
    {SettingsId::INITIAL_WINDOW_SIZE, http2::kInitialWindow}};
const HTTPSettings kIngressCertAuthSettings{
    {SettingsId::SETTINGS_HTTP_CERT_AUTH, 128}};
HTTPSettings kEgressCertAuthSettings{
    {SettingsId::SETTINGS_HTTP_CERT_AUTH, 128}};

class MockCodecDownstreamTest : public testing::Test {
 public:
  MockCodecDownstreamTest()
      : eventBase_(),
        codec_(new StrictMock<MockHTTPCodec>()),
        transport_(new NiceMock<MockAsyncTransport>()),
        transactionTimeouts_(makeTimeoutSet(&eventBase_)) {

    HTTP2PriorityQueue::setNodeLifetime(std::chrono::milliseconds(2));
    EXPECT_CALL(*transport_, writeChain(_, _, _))
        .WillRepeatedly(Invoke(this, &MockCodecDownstreamTest::onWriteChain));
    EXPECT_CALL(*transport_, good())
        .WillRepeatedly(ReturnPointee(&transportGood_));
    EXPECT_CALL(*transport_, closeNow())
        .WillRepeatedly(Assign(&transportGood_, false));
    EXPECT_CALL(*transport_, getEventBase())
        .WillRepeatedly(Return(&eventBase_));
    EXPECT_CALL(*transport_, setReadCB(_))
        .WillRepeatedly(SaveArg<0>(&transportCb_));
    EXPECT_CALL(mockController_, getGracefulShutdownTimeout())
        .WillRepeatedly(Return(std::chrono::milliseconds(0)));
    EXPECT_CALL(mockController_, getHeaderIndexingStrategy())
        .WillRepeatedly(Return(HeaderIndexingStrategy::getDefaultInstance()));
    EXPECT_CALL(mockController_, attachSession(_));
    EXPECT_CALL(mockController_, onTransportReady(_));
    EXPECT_CALL(*codec_, setCallback(_))
        .WillRepeatedly(SaveArg<0>(&codecCallback_));
    EXPECT_CALL(*codec_, supportsParallelRequests())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*codec_, supportsPushTransactions())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*codec_, getTransportDirection())
        .WillRepeatedly(Return(TransportDirection::DOWNSTREAM));
    EXPECT_CALL(*codec_, getEgressSettings());
    EXPECT_CALL(*codec_, supportsStreamFlowControl())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*codec_, getProtocol())
        .WillRepeatedly(Return(CodecProtocol::HTTP_2));
    EXPECT_CALL(*codec_, getUserAgent()).WillRepeatedly(ReturnRef(userAgent_));
    EXPECT_CALL(*codec_, setParserPaused(_)).WillRepeatedly(Return());
    EXPECT_CALL(*codec_, supportsSessionFlowControl())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*codec_, getIngressSettings())
        .WillRepeatedly(Return(&kDefaultIngressSettings));
    EXPECT_CALL(*codec_, isReusable())
        .WillRepeatedly(ReturnPointee(&reusable_));
    EXPECT_CALL(*codec_, isWaitingToDrain())
        .WillRepeatedly(ReturnPointee(&drainPending_));
    EXPECT_CALL(*codec_, generateSettings(_));
    EXPECT_CALL(*codec_, getDefaultWindowSize())
        .WillRepeatedly(Return(http2::kInitialWindow));
    EXPECT_CALL(*codec_, createStream()).WillRepeatedly(InvokeWithoutArgs([&] {
      return pushStreamID_ += 2;
    }));
    EXPECT_CALL(*codec_, enableDoubleGoawayDrain()).WillRepeatedly(Invoke([&] {
      doubleGoaway_ = true;
    }));
    EXPECT_CALL(*codec_, generateGoaway(_, _, _, _))
        .WillRepeatedly(Invoke([this](IOBufQueue& writeBuf,
                                      HTTPCodec::StreamID /*lastStream*/,
                                      ErrorCode,
                                      std::shared_ptr<folly::IOBuf>) {
          LOG(INFO) << "MOCK GENERATE GOAWAY";
          if (reusable_) {
            reusable_ = false;
            drainPending_ = doubleGoaway_;
          } else if (!drainPending_) {
            return 0;
          } else {
            drainPending_ = false;
          }
          if (liveGoaways_) {
            writeBuf.append(string("x"));
          }
          return 1;
        }));
    EXPECT_CALL(*codec_, generateRstStream(_, _, _)).WillRepeatedly(Return(1));
    EXPECT_CALL(*codec_, addPriorityNodes(_, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*codec_, mapPriorityToDependency(_)).WillRepeatedly(Return(0));

    HTTPSession::setDefaultReadBufferLimit(65536);
    httpSession_ =
        new HTTPDownstreamSession(transactionTimeouts_.get(),
                                  AsyncTransport::UniquePtr(transport_),
                                  localAddr,
                                  peerAddr,
                                  &mockController_,
                                  std::unique_ptr<HTTPCodec>(codec_),
                                  mockTransportInfo,
                                  nullptr);
    httpSession_->startNow();
    eventBase_.loop();
  }

  void onWriteChain(folly::AsyncTransport::WriteCallback* callback,
                    std::shared_ptr<IOBuf> /*iob*/,
                    WriteFlags) {
    writeCount_++;
    if (invokeWriteSuccess_) {
      callback->writeSuccess();
    } else {
      cbs_.push_back(callback);
    }
  }

  ~MockCodecDownstreamTest() override {
    AsyncSocketException ex(AsyncSocketException::UNKNOWN, "");
    for (auto& cb : cbs_) {
      cb->writeErr(0, ex);
    }
  }

  void SetUp() override {
    HTTPSession::setDefaultWriteBufferLimit(65536);
  }

  // Pass a function to execute inside HTTPCodec::onIngress(). This
  // function also takes care of passing an empty ingress buffer to the codec.
  template <class T>
  void onIngressImpl(T f) {
    EXPECT_CALL(*codec_, onIngress(_)).WillOnce(Invoke([&f](const IOBuf& buf) {
      CHECK_GT(buf.computeChainDataLength(), 0);
      // The test should be independent of the dummy buffer,
      // so don't pass it in.
      f();
      return buf.computeChainDataLength();
    }));

    void* buf;
    size_t bufSize;
    transportCb_->getReadBuffer(&buf, &bufSize);
    transportCb_->readDataAvailable(bufSize);
  }

  void testGoaway(bool doubleGoaway, bool dropConnection);

  void testConnFlowControlBlocked(bool timeout);

 protected:
  EventBase eventBase_;
  // invalid once httpSession_ is destroyed
  StrictMock<MockHTTPCodec>* codec_;
  std::string userAgent_{"MockCodec"};
  HTTPCodec::Callback* codecCallback_{nullptr};
  NiceMock<MockAsyncTransport>* transport_;
  folly::AsyncTransport::ReadCallback* transportCb_;
  folly::HHWheelTimer::UniquePtr transactionTimeouts_;
  StrictMock<MockController> mockController_;
  HTTPDownstreamSession* httpSession_;
  HTTPCodec::StreamID pushStreamID_{0};
  bool reusable_{true};
  bool transportGood_{true};
  bool drainPending_{false};
  bool doubleGoaway_{false};
  bool liveGoaways_{false};
  bool invokeWriteSuccess_{false};
  uint32_t writeCount_{0};
  std::vector<folly::AsyncTransport::WriteCallback*> cbs_;
};

TEST_F(MockCodecDownstreamTest, OnAbortThenTimeouts) {
  // Test what happens when txn1 (out of many transactions) gets an abort
  // followed by a transaction timeout followed by a write timeout
  MockHTTPHandler handler1;
  MockHTTPHandler handler2;
  auto req1 = makeGetRequest();
  auto req2 = makeGetRequest();

  fakeMockCodec(*codec_);

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler1))
      .WillOnce(Return(&handler2));

  EXPECT_CALL(handler1, _setTransaction(_))
      .WillOnce(
          Invoke([&handler1](HTTPTransaction* txn) { handler1.txn_ = txn; }));
  EXPECT_CALL(handler1, _onHeadersComplete(_))
      .WillOnce(Invoke([&handler1](std::shared_ptr<HTTPMessage>) {
        handler1.sendHeaders(200, 100);
        handler1.sendBody(100);
      }));
  EXPECT_CALL(handler1, _onError(_));
  EXPECT_CALL(handler1, _detachTransaction());
  EXPECT_CALL(handler2, _setTransaction(_))
      .WillOnce(
          Invoke([&handler2](HTTPTransaction* txn) { handler2.txn_ = txn; }));
  EXPECT_CALL(handler2, _onHeadersComplete(_))
      .WillOnce(Invoke([&handler2](std::shared_ptr<HTTPMessage>) {
        handler2.sendHeaders(200, 100);
        handler2.sendBody(100);
      }));
  EXPECT_CALL(handler2, _onBodyWithOffset(_, _));
  EXPECT_CALL(handler2, _onError(_))
      .WillOnce(Invoke([&](const HTTPException& ex) {
        ASSERT_EQ(ex.getProxygenError(), kErrorWriteTimeout);
        ASSERT_EQ(folly::to<std::string>("WriteTimeout on transaction id: ",
                                         handler2.txn_->getID()),
                  std::string(ex.what()));
      }));
  EXPECT_CALL(handler2, _detachTransaction());

  EXPECT_CALL(mockController_, detachSession(_));

  codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req1.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req1));
  codecCallback_->onMessageBegin(HTTPCodec::StreamID(3), req2.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(3), std::move(req2));
  // do the write, enqeue byte event
  eventBase_.loopOnce();

  // recv an abort, detach the handler from txn1 (txn1 stays around due to the
  // enqueued byte event)
  codecCallback_->onAbort(HTTPCodec::StreamID(1), ErrorCode::PROTOCOL_ERROR);
  // recv a transaction timeout on txn1 (used to erroneously create a direct
  // response handler)
  handler1.txn_->timeoutExpired();

  // have a write timeout expire (used to cause the direct response handler to
  // write out data, messing up the state machine)
  eventBase_.runAfterDelay(
      [this] {
        // refresh ingress timeout
        codecCallback_->onBody(HTTPCodec::StreamID(3), makeBuf(10), 0);
      },
      transactionTimeouts_->getDefaultTimeout().count() / 2);
  // hold evb open long enough to fire write timeout (since transactionTimeouts_
  // is internal
  eventBase_.runAfterDelay(
      [] {}, transactionTimeouts_->getDefaultTimeout().count() + 100);
  eventBase_.loop();
}

TEST_F(MockCodecDownstreamTest, ServerPush) {
  MockHTTPHandler handler;
  MockHTTPPushHandler pushHandler;
  auto req = makeGetRequest();
  HTTPTransaction* pushTxn = nullptr;

  InSequence enforceOrder;

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler));
  EXPECT_CALL(handler, _setTransaction(_)).WillOnce(SaveArg<0>(&handler.txn_));

  EXPECT_CALL(handler, _onHeadersComplete(_))
      .WillOnce(Invoke([&](std::shared_ptr<HTTPMessage>) {
        pushTxn = handler.txn_->newPushedTransaction(&pushHandler);
        pushHandler.sendPushHeaders(
            "/foo", "www.foo.com", 100, handler.txn_->getPriority());
        pushHandler.sendBody(100);
        pushTxn->sendEOM();
        eventBase_.loopOnce(); // flush the push txn's body
      }));
  EXPECT_CALL(pushHandler, _setTransaction(_))
      .WillOnce(Invoke(
          [&pushHandler](HTTPTransaction* txn) { pushHandler.txn_ = txn; }));

  EXPECT_CALL(*codec_, generatePushPromise(_, 2, _, _, _, _));
  EXPECT_CALL(*codec_,
              generateBody(_, 2, PtrBufHasLen(uint64_t(100)), _, true));
  EXPECT_CALL(pushHandler, _detachTransaction());

  EXPECT_CALL(handler, _onEOM()).WillOnce(Invoke([&] {
    handler.sendReplyWithBody(200, 100);
    eventBase_.loopOnce(); // flush the response to the normal request
  }));

  EXPECT_CALL(*codec_, generateHeader(_, 1, _, _, _, _));
  EXPECT_CALL(*codec_,
              generateBody(_, 1, PtrBufHasLen(uint64_t(100)), _, true));
  EXPECT_CALL(handler, _detachTransaction());

  codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req));
  codecCallback_->onMessageComplete(HTTPCodec::StreamID(1), false);

  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, ServerPushAfterGoaway) {
  // Tests if goaway
  //   - drains acknowledged server push transactions
  //   - aborts server pushed transactions not created at the client
  //   - prevents new transactions from being created.
  MockHTTPHandler handler;
  MockHTTPPushHandler pushHandler1;
  MockHTTPPushHandler pushHandler2;
  HTTPTransaction* pushTxn = nullptr;

  fakeMockCodec(*codec_);

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler));

  EXPECT_CALL(handler, _setTransaction(_))
      .WillOnce(
          Invoke([&handler](HTTPTransaction* txn) { handler.txn_ = txn; }));
  EXPECT_CALL(handler, _onHeadersComplete(_))
      .WillOnce(Invoke([&](std::shared_ptr<HTTPMessage>) {
        // Initiate server push transactions.
        pushTxn = handler.txn_->newPushedTransaction(&pushHandler1);
        CHECK_EQ(pushTxn->getID(), HTTPCodec::StreamID(2));
        pushHandler1.sendPushHeaders(
            "/foo", "www.foo.com", 100, handler.txn_->getPriority());
        pushHandler1.sendBody(100);
        pushTxn->sendEOM();
        // Initiate the second push transaction which will be aborted
        pushTxn = handler.txn_->newPushedTransaction(&pushHandler2);
        CHECK_EQ(pushTxn->getID(), HTTPCodec::StreamID(4));
        pushHandler2.sendPushHeaders(
            "/foo", "www.foo.com", 100, handler.txn_->getPriority());
        pushHandler2.sendBody(100);
        pushTxn->sendEOM();
      }));
  // Push transaction 1 - drained
  EXPECT_CALL(pushHandler1, _setTransaction(_))
      .WillOnce(Invoke(
          [&pushHandler1](HTTPTransaction* txn) { pushHandler1.txn_ = txn; }));
  EXPECT_CALL(pushHandler1, _detachTransaction());
  // Push transaction 2 - aborted by onError after goaway
  EXPECT_CALL(pushHandler2, _setTransaction(_))
      .WillOnce(Invoke(
          [&pushHandler2](HTTPTransaction* txn) { pushHandler2.txn_ = txn; }));
  EXPECT_CALL(pushHandler2, _onError(_))
      .WillOnce(Invoke([&](const HTTPException& err) {
        EXPECT_TRUE(err.hasProxygenError());
        EXPECT_EQ(err.getProxygenError(), kErrorStreamUnacknowledged);
        ASSERT_EQ(
            folly::to<std::string>("StreamUnacknowledged on transaction id: ",
                                   pushHandler2.txn_->getID()),
            std::string(err.what()));
      }));
  EXPECT_CALL(pushHandler2, _detachTransaction());

  EXPECT_CALL(handler, _onEOM());
  EXPECT_CALL(handler, _detachTransaction());

  // Receive client request
  auto req = makeGetRequest();
  codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req));
  codecCallback_->onMessageComplete(HTTPCodec::StreamID(1), false);

  // Receive goaway acknowledging only the first pushed transactions with id 2.
  codecCallback_->onGoaway(2, ErrorCode::NO_ERROR);

  // New server pushed transaction cannot be created after goaway
  MockHTTPPushHandler pushHandler3;
  EXPECT_EQ(handler.txn_->newPushedTransaction(&pushHandler3), nullptr);

  // Send response to the initial client request and this destroys the session
  handler.sendReplyWithBody(200, 100);

  eventBase_.loop();

  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, ServerPushAbort) {
  // Test that assoc txn and other push txns are not affected when client aborts
  // a push txn
  MockHTTPHandler handler;
  MockHTTPPushHandler pushHandler1;
  MockHTTPPushHandler pushHandler2;
  HTTPTransaction* pushTxn1 = nullptr;
  HTTPTransaction* pushTxn2 = nullptr;

  fakeMockCodec(*codec_);

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler));

  EXPECT_CALL(handler, _setTransaction(_))
      .WillOnce(
          Invoke([&handler](HTTPTransaction* txn) { handler.txn_ = txn; }));
  EXPECT_CALL(handler, _onHeadersComplete(_))
      .WillOnce(Invoke([&](std::shared_ptr<HTTPMessage>) {
        // Initiate server push transactions
        pushTxn1 = handler.txn_->newPushedTransaction(&pushHandler1);
        CHECK_EQ(pushTxn1->getID(), HTTPCodec::StreamID(2));
        pushHandler1.sendPushHeaders(
            "/foo", "www.foo.com", 100, handler.txn_->getPriority());
        pushHandler1.sendBody(100);

        pushTxn2 = handler.txn_->newPushedTransaction(&pushHandler2);
        CHECK_EQ(pushTxn2->getID(), HTTPCodec::StreamID(4));
        pushHandler2.sendPushHeaders(
            "/bar", "www.bar.com", 200, handler.txn_->getPriority());
        pushHandler2.sendBody(200);
        pushTxn2->sendEOM();
      }));

  // pushTxn1 should be aborted
  EXPECT_CALL(pushHandler1, _setTransaction(_))
      .WillOnce(Invoke(
          [&pushHandler1](HTTPTransaction* txn) { pushHandler1.txn_ = txn; }));
  EXPECT_CALL(pushHandler1, _onError(_))
      .WillOnce(Invoke([&](const HTTPException& err) {
        EXPECT_TRUE(err.hasProxygenError());
        EXPECT_EQ(err.getProxygenError(), kErrorStreamAbort);
        ASSERT_EQ("Stream aborted, streamID=2, code=CANCEL",
                  std::string(err.what()));
      }));
  EXPECT_CALL(pushHandler1, _detachTransaction());

  EXPECT_CALL(pushHandler2, _setTransaction(_))
      .WillOnce(Invoke(
          [&pushHandler2](HTTPTransaction* txn) { pushHandler2.txn_ = txn; }));
  EXPECT_CALL(pushHandler2, _detachTransaction());

  EXPECT_CALL(handler, _onEOM());
  EXPECT_CALL(handler, _detachTransaction());

  // Receive client request
  auto req = makeGetRequest();
  codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req));
  codecCallback_->onMessageComplete(HTTPCodec::StreamID(1), false);

  // Send client abort on one push txn
  codecCallback_->onAbort(HTTPCodec::StreamID(2), ErrorCode::CANCEL);

  handler.sendReplyWithBody(200, 100);

  eventBase_.loop();

  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, ServerPushAbortAssoc) {
  // Test that associated push transactions remain alive when client aborts
  // the assoc stream
  MockHTTPHandler handler;
  MockHTTPPushHandler pushHandler1;
  MockHTTPPushHandler pushHandler2;

  fakeMockCodec(*codec_);
  invokeWriteSuccess_ = true;

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler));

  EXPECT_CALL(handler, _setTransaction(_))
      .WillOnce(
          Invoke([&handler](HTTPTransaction* txn) { handler.txn_ = txn; }));
  EXPECT_CALL(handler, _onHeadersComplete(_))
      .WillOnce(Invoke([&](std::shared_ptr<HTTPMessage>) {
        // Initiate server push transactions
        auto pushTxn = handler.txn_->newPushedTransaction(&pushHandler1);
        CHECK_EQ(pushTxn->getID(), HTTPCodec::StreamID(2));
        pushHandler1.sendPushHeaders(
            "/foo", "www.foo.com", 100, handler.txn_->getPriority());
        pushHandler1.sendBody(100);

        pushTxn = handler.txn_->newPushedTransaction(&pushHandler2);
        CHECK_EQ(pushTxn->getID(), HTTPCodec::StreamID(4));
        pushHandler2.sendPushHeaders(
            "/foo", "www.foo.com", 100, handler.txn_->getPriority());
        pushHandler2.sendBody(100);
      }));

  // Both push txns and the assoc txn should remain alive, and in this case
  // time out.
  EXPECT_CALL(pushHandler1, _setTransaction(_))
      .WillOnce(Invoke(
          [&pushHandler1](HTTPTransaction* txn) { pushHandler1.txn_ = txn; }));
  EXPECT_CALL(pushHandler2, _setTransaction(_))
      .WillOnce(Invoke(
          [&pushHandler2](HTTPTransaction* txn) { pushHandler2.txn_ = txn; }));
  EXPECT_CALL(pushHandler1, _detachTransaction());
  EXPECT_CALL(pushHandler2, _detachTransaction());

  EXPECT_CALL(handler, _onError(_))
      .WillOnce(Invoke([&](const HTTPException& err) {
        EXPECT_TRUE(err.hasProxygenError());
        EXPECT_EQ(err.getProxygenError(), kErrorStreamAbort);
        ASSERT_EQ("Stream aborted, streamID=1, code=CANCEL",
                  std::string(err.what()));
      }));
  EXPECT_CALL(handler, _detachTransaction());

  // Receive client request
  auto req = makeGetRequest();
  codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req));
  eventBase_.loopOnce();

  // Send client abort on assoc stream
  codecCallback_->onAbort(HTTPCodec::StreamID(1), ErrorCode::CANCEL);

  // Push txns can still send body and EOM
  pushHandler1.sendBody(200);
  pushHandler2.sendBody(200);
  pushHandler1.sendEOM();
  pushHandler2.sendEOM();

  eventBase_.loopOnce();

  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, ServerPushClientMessage) {
  // Test that error is generated when client sends data on a pushed stream
  MockHTTPHandler handler;
  MockHTTPPushHandler pushHandler;
  auto req = makeGetRequest();
  HTTPTransaction* pushTxn = nullptr;

  InSequence enforceOrder;

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler));
  EXPECT_CALL(handler, _setTransaction(_)).WillOnce(SaveArg<0>(&handler.txn_));

  EXPECT_CALL(handler, _onHeadersComplete(_))
      .WillOnce(Invoke([&](std::shared_ptr<HTTPMessage> msg) {
        pushTxn = handler.txn_->newPushedTransaction(&pushHandler);
        auto pri = handler.txn_->getPriority();
        msg->setHTTP2Priority(
            std::make_tuple(pri.streamDependency, pri.exclusive, pri.weight));
      }));
  EXPECT_CALL(pushHandler, _setTransaction(_))
      .WillOnce(Invoke(
          [&pushHandler](HTTPTransaction* txn) { pushHandler.txn_ = txn; }));

  codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req));

  EXPECT_CALL(*codec_, generateRstStream(_, 2, ErrorCode::STREAM_CLOSED))
      .WillRepeatedly(Return(1));
  EXPECT_CALL(pushHandler, _onError(_))
      .WillOnce(Invoke([&](const HTTPException& ex) {
        EXPECT_TRUE(ex.hasCodecStatusCode());
        EXPECT_EQ(ex.getCodecStatusCode(), ErrorCode::STREAM_CLOSED);
        ASSERT_EQ("Downstream attempts to send ingress, abort.",
                  std::string(ex.what()));
      }));
  EXPECT_CALL(pushHandler, _detachTransaction());

  // While the assoc stream is open and pushHandler has been initialized, send
  // an upstream message on the push stream causing a RST_STREAM.
  req = makeGetRequest();
  codecCallback_->onMessageBegin(HTTPCodec::StreamID(2), req.get());

  EXPECT_CALL(handler, _onEOM()).WillOnce(InvokeWithoutArgs([&] {
    handler.sendReplyWithBody(200, 100);
    eventBase_.loop(); // flush the response to the assoc request
  }));
  EXPECT_CALL(*codec_, generateHeader(_, 1, _, _, _, _));
  EXPECT_CALL(*codec_,
              generateBody(_, 1, PtrBufHasLen(uint64_t(100)), _, true));
  EXPECT_CALL(handler, _detachTransaction());

  // Complete the assoc request/response
  codecCallback_->onMessageComplete(HTTPCodec::StreamID(1), false);

  eventBase_.loop();

  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, ReadTimeout) {
  // Test read timeout path
  codec_->enableDoubleGoawayDrain();
  MockHTTPHandler handler1;
  auto req1 = makeGetRequest();

  EXPECT_CALL(*codec_, onIngressEOF()).WillRepeatedly(Return());

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler1));

  EXPECT_CALL(handler1, _setTransaction(_))
      .WillOnce(
          Invoke([&handler1](HTTPTransaction* txn) { handler1.txn_ = txn; }));
  EXPECT_CALL(handler1, _onHeadersComplete(_));

  codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req1.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req1));
  httpSession_->timeoutExpired();
  EXPECT_EQ(httpSession_->getConnectionCloseReason(),
            ConnectionCloseReason::TIMEOUT);

  EXPECT_CALL(handler1, _onEOM()).WillOnce(Invoke([&handler1]() {
    handler1.txn_->pauseIngress();
  }));

  // send the EOM, then another timeout.  Still no-op since it's waiting
  // upstream
  codecCallback_->onMessageComplete(HTTPCodec::StreamID(1), false);
  httpSession_->timeoutExpired();

  EXPECT_CALL(*transport_, writeChain(_, _, _))
      .WillRepeatedly(
          Invoke([](folly::AsyncTransport::WriteCallback* callback,
                    std::shared_ptr<folly::IOBuf>,
                    folly::WriteFlags) { callback->writeSuccess(); }));

  EXPECT_CALL(handler1, _detachTransaction());

  // Send the response, timeout.  Now it's idle and should close.
  handler1.txn_->resumeIngress();
  EXPECT_CALL(*codec_, generateHeader(_, 1, _, _, _, _));
  handler1.sendReplyWithBody(200, 100);
  EXPECT_CALL(*codec_,
              generateBody(_, 1, PtrBufHasLen(uint64_t(100)), _, true));
  EXPECT_CALL(mockController_, detachSession(_));
  eventBase_.loop();
}

TEST_F(MockCodecDownstreamTest, Ping) {
  // Test ping mechanism and that we prioritize the ping reply
  MockHTTPHandler handler1;
  auto req1 = makeGetRequest();

  InSequence enforceOrder;

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler1));

  EXPECT_CALL(handler1, _setTransaction(_))
      .WillOnce(
          Invoke([&handler1](HTTPTransaction* txn) { handler1.txn_ = txn; }));
  EXPECT_CALL(handler1, _onHeadersComplete(_));
  EXPECT_CALL(handler1, _onEOM()).WillOnce(InvokeWithoutArgs([&handler1]() {
    handler1.sendReplyWithBody(200, 100);
  }));

  // Header egresses immediately
  EXPECT_CALL(*codec_, generateHeader(_, _, _, _, _, _));
  // Ping jumps ahead of queued body in the loop callback
  EXPECT_CALL(*codec_, generatePingReply(_, _));
  EXPECT_CALL(*codec_, generateBody(_, _, _, _, true));
  EXPECT_CALL(handler1, _detachTransaction());

  codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req1.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req1));
  codecCallback_->onMessageComplete(HTTPCodec::StreamID(1), false);
  codecCallback_->onPingRequest(1);

  eventBase_.loop();

  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, FlowControlAbort) {
  MockHTTPHandler handler1;
  auto req1 = makePostRequest();

  InSequence enforceOrder;

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler1));

  EXPECT_CALL(handler1, _setTransaction(_))
      .WillOnce(
          Invoke([&handler1](HTTPTransaction* txn) { handler1.txn_ = txn; }));
  EXPECT_CALL(handler1, _onHeadersComplete(_))
      .WillOnce(
          InvokeWithoutArgs([&handler1]() { handler1.txn_->sendAbort(); }));

  // Header egresses immediately
  EXPECT_CALL(handler1, _detachTransaction());

  codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req1.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req1));
  EXPECT_CALL(*codec_, generateWindowUpdate(_, 0, http2::kInitialWindow));
  codecCallback_->onBody(
      HTTPCodec::StreamID(1), makeBuf(http2::kInitialWindow), 0);
  EXPECT_CALL(*codec_, generateWindowUpdate(_, 0, http2::kInitialWindow));
  codecCallback_->onBody(
      HTTPCodec::StreamID(1), makeBuf(http2::kInitialWindow), 0);

  eventBase_.loop();

  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, Buffering) {
  StrictMock<MockHTTPHandler> handler;
  auto req1 = makePostRequest(20);
  auto chunk = makeBuf(10);
  auto chunkStr = chunk->clone()->moveToFbString();

  fakeMockCodec(*codec_);

  httpSession_->setDefaultReadBufferLimit(10);

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler));

  EXPECT_CALL(handler, _setTransaction(_))
      .WillOnce(
          Invoke([&handler](HTTPTransaction* txn) { handler.txn_ = txn; }));
  EXPECT_CALL(handler, _onHeadersComplete(_))
      .WillOnce(
          InvokeWithoutArgs([&handler]() { handler.txn_->pauseIngress(); }));

  EXPECT_CALL(*transport_, writeChain(_, _, _))
      .WillRepeatedly(Invoke([&](folly::AsyncTransport::WriteCallback* callback,
                                 const shared_ptr<IOBuf>&,
                                 WriteFlags) { callback->writeSuccess(); }));

  codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req1.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req1));
  for (int i = 0; i < 2; i++) {
    codecCallback_->onBody(HTTPCodec::StreamID(1), chunk->clone(), 0);
  }
  codecCallback_->onMessageComplete(HTTPCodec::StreamID(1), false);

  EXPECT_CALL(handler, _onBodyWithOffset(_, _))
      .WillOnce(ExpectString(chunkStr))
      .WillOnce(ExpectString(chunkStr));

  EXPECT_CALL(handler, _onEOM());

  EXPECT_CALL(handler, _detachTransaction());

  eventBase_.tryRunAfterDelay(
      [&handler, this] {
        handler.txn_->resumeIngress();
        handler.sendReplyWithBody(200, 100);
        eventBase_.runInLoop([this] { httpSession_->dropConnection(); });
      },
      30);

  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));
  eventBase_.loop();
}

TEST_F(MockCodecDownstreamTest, FlowControlWindow) {
  // Test window updates
  MockHTTPHandler handler1;
  auto req1 = makeGetRequest();

  fakeMockCodec(*codec_);

  {
    InSequence enforceOrder;
    EXPECT_CALL(mockController_, getRequestHandler(_, _))
        .WillOnce(Return(&handler1));

    EXPECT_CALL(handler1, _setTransaction(_))
        .WillOnce(
            Invoke([&handler1](HTTPTransaction* txn) { handler1.txn_ = txn; }));
    EXPECT_CALL(handler1, _onHeadersComplete(_))
        .WillOnce(InvokeWithoutArgs([this]() {
          codecCallback_->onSettings({{SettingsId::INITIAL_WINDOW_SIZE, 4000}});
        }));
    EXPECT_CALL(*codec_, generateSettingsAck(_));
    EXPECT_CALL(handler1, _onEOM()).WillOnce(InvokeWithoutArgs([&handler1]() {
      handler1.sendHeaders(200, 16000);
      handler1.sendBody(12000);
      // 12kb buffered -> pause upstream
    }));
    EXPECT_CALL(handler1, _onEgressPaused())
        .WillOnce(InvokeWithoutArgs([this]() {
          eventBase_.runInLoop([this] {
            // triggers 4k send, 8kb buffered, handler still paused
            codecCallback_->onWindowUpdate(1, 4000);
          });
          eventBase_.runAfterDelay(
              [this] {
                // triggers 6k send, 2kb buffered, handler still paused
                codecCallback_->onWindowUpdate(1, 6000);
              },
              10);
          eventBase_.runAfterDelay(
              [this] {
                // triggers 2kb send, 0 buffered, 2k window => resume
                codecCallback_->onWindowUpdate(1, 4000);
              },
              20);
        }));
    EXPECT_CALL(handler1, _onEgressResumed())
        .WillOnce(InvokeWithoutArgs([&handler1]() {
          handler1.sendBody(4000);
          // 2kb send, 2kb buffered => pause upstream
        }));
    EXPECT_CALL(handler1, _onEgressPaused())
        .WillOnce(InvokeWithoutArgs([this]() {
          eventBase_.runInLoop([this] {
            // triggers 2kb send, resume
            codecCallback_->onWindowUpdate(1, 4000);
          });
        }));
    EXPECT_CALL(handler1, _onEgressResumed())
        .WillOnce(
            InvokeWithoutArgs([&handler1]() { handler1.txn_->sendEOM(); }));

    EXPECT_CALL(handler1, _detachTransaction());

    codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req1.get());
    codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req1));
    codecCallback_->onMessageComplete(HTTPCodec::StreamID(1), false);
    // Pad coverage numbers
    std::ostringstream stream;
    stream << *handler1.txn_ << *httpSession_ << httpSession_->getLocalAddress()
           << httpSession_->getPeerAddress();
    EXPECT_TRUE(httpSession_->isBusy());

    EXPECT_CALL(*codec_, onIngressEOF());
    EXPECT_CALL(mockController_, detachSession(_));
  }

  EXPECT_CALL(*transport_, writeChain(_, _, _))
      .WillRepeatedly(
          Invoke([](folly::AsyncTransport::WriteCallback* callback,
                    std::shared_ptr<folly::IOBuf>,
                    folly::WriteFlags) { callback->writeSuccess(); }));
  eventBase_.loop();
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, DoubleResume) {
  // Test ping mechanism and egress re-ordering
  MockHTTPHandler handler1;
  auto req1 = makePostRequest(5);
  auto buf = makeBuf(5);
  auto bufStr = buf->clone()->moveToFbString();

  fakeMockCodec(*codec_);

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler1));

  EXPECT_CALL(handler1, _setTransaction(_))
      .WillOnce(
          Invoke([&handler1](HTTPTransaction* txn) { handler1.txn_ = txn; }));
  EXPECT_CALL(handler1, _onHeadersComplete(_))
      .WillOnce(InvokeWithoutArgs([&handler1, this] {
        handler1.txn_->pauseIngress();
        eventBase_.tryRunAfterDelay(
            [&handler1] { handler1.txn_->resumeIngress(); }, 50);
      }));
  EXPECT_CALL(handler1, _onBodyWithOffset(_, _))
      .WillOnce(Invoke(
          [&handler1, &bufStr](uint64_t, std::shared_ptr<folly::IOBuf> chain) {
            EXPECT_EQ(bufStr, chain->moveToFbString());
            handler1.txn_->pauseIngress();
            handler1.txn_->resumeIngress();
          }));

  EXPECT_CALL(handler1, _onEOM()).WillOnce(InvokeWithoutArgs([&handler1]() {
    handler1.sendReplyWithBody(200, 100, false);
  }));
  EXPECT_CALL(handler1, _detachTransaction());

  codecCallback_->onMessageBegin(HTTPCodec::StreamID(1), req1.get());
  codecCallback_->onHeadersComplete(HTTPCodec::StreamID(1), std::move(req1));
  codecCallback_->onBody(HTTPCodec::StreamID(1), std::move(buf), 0);
  codecCallback_->onMessageComplete(HTTPCodec::StreamID(1), false);

  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));

  EXPECT_CALL(*transport_, writeChain(_, _, _))
      .WillRepeatedly(
          Invoke([](folly::AsyncTransport::WriteCallback* callback,
                    std::shared_ptr<folly::IOBuf>,
                    folly::WriteFlags) { callback->writeSuccess(); }));

  eventBase_.loop();
  httpSession_->dropConnection();
}

void MockCodecDownstreamTest::testConnFlowControlBlocked(bool timeout) {
  // Let the connection level flow control window fill and then make sure
  // control frames still can be processed
  NiceMock<MockHTTPHandler> handler1;
  NiceMock<MockHTTPHandler> handler2;
  auto wantToWrite = http2::kInitialWindow + 50000;
  auto wantToWriteStr = folly::to<string>(wantToWrite);
  auto req1 = makeGetRequest();
  auto req2 = makeGetRequest();
  auto resp1 = makeResponse(200);
  resp1->getHeaders().set(HTTP_HEADER_CONTENT_LENGTH, wantToWriteStr);
  auto resp2 = makeResponse(200);
  resp2->getHeaders().set(HTTP_HEADER_CONTENT_LENGTH, wantToWriteStr);
  invokeWriteSuccess_ = true;

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler1));
  EXPECT_CALL(handler1, _setTransaction(_))
      .WillOnce(SaveArg<0>(&handler1.txn_));
  EXPECT_CALL(handler1, _onHeadersComplete(_));
  EXPECT_CALL(*codec_, generateHeader(_, 1, _, _, _, _))
      .WillOnce(
          Invoke([](folly::IOBufQueue& writeBuf,
                    HTTPCodec::StreamID,
                    const HTTPMessage&,
                    bool,
                    HTTPHeaderSize*,
                    folly::Optional<HTTPHeaders>) { writeBuf.append("", 1); }));
  unsigned bodyLen = 0;
  EXPECT_CALL(*codec_, generateBody(_, 1, _, _, false))
      .WillRepeatedly(Invoke([&](folly::IOBufQueue& /*writeBuf*/,
                                 HTTPCodec::StreamID,
                                 std::shared_ptr<folly::IOBuf> chain,
                                 folly::Optional<uint8_t>,
                                 bool /*eom*/) {
        bodyLen += chain->computeChainDataLength();
        return 0; // don't want byte events
      }));

  codecCallback_->onMessageBegin(1, req1.get());
  codecCallback_->onHeadersComplete(1, std::move(req1));
  codecCallback_->onWindowUpdate(1, wantToWrite); // ensure the per-stream
                                                  // window doesn't block
  handler1.txn_->sendHeaders(*resp1);
  handler1.txn_->sendBody(makeBuf(wantToWrite)); // conn blocked, stream open
  handler1.txn_->sendEOM();
  eventBase_.loopOnce();                    // actually send (most of) the body
  CHECK_EQ(bodyLen, http2::kInitialWindow); // should have written a full window

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler2));
  EXPECT_CALL(handler2, _setTransaction(_))
      .WillOnce(SaveArg<0>(&handler2.txn_));
  EXPECT_CALL(handler2, _onHeadersComplete(_));
  EXPECT_CALL(*codec_, generateHeader(_, 3, _, _, _, _))
      .WillOnce(
          Invoke([](folly::IOBufQueue& writeBuf,
                    HTTPCodec::StreamID,
                    const HTTPMessage&,
                    bool,
                    HTTPHeaderSize*,
                    folly::Optional<HTTPHeaders>) { writeBuf.append("", 1); }));

  auto writeCount = writeCount_;

  // Make sure we can send headers of response to a second request
  codecCallback_->onMessageBegin(3, req2.get());
  codecCallback_->onHeadersComplete(3, std::move(req2));
  handler2.txn_->sendHeaders(*resp2);

  eventBase_.loopOnce();

  EXPECT_EQ(writeCount + 1, writeCount_);

  if (timeout) {
    // don't send a window update, the handlers will get timeouts
    EXPECT_CALL(handler1, _onError(_))
        .WillOnce(Invoke([](const HTTPException& ex) {
          EXPECT_EQ(ex.getProxygenError(), kErrorTimeout);
        }));
    EXPECT_CALL(handler2, _onError(_))
        .WillOnce(Invoke([](const HTTPException& ex) {
          EXPECT_EQ(ex.getProxygenError(), kErrorTimeout);
        }));
    EXPECT_CALL(mockController_, detachSession(_));
    // send a window update to refresh the stream level timeout
    codecCallback_->onWindowUpdate(1, 1);
    // silly, the timeout set is internal and there's no fd, so hold the
    // eventBase open until the timeout can fire
    eventBase_.runAfterDelay([] {}, 500);
  } else {
    // Give a connection level window update of 10 bytes -- this
    // should allow 10 bytes of the txn1 response to be written
    codecCallback_->onWindowUpdate(0, 10);
    EXPECT_CALL(*codec_,
                generateBody(_, 1, PtrBufHasLen(uint64_t(10)), _, false));
    eventBase_.loopOnce();

    // Just tear everything down now.
    EXPECT_CALL(handler1, _detachTransaction());
    codecCallback_->onAbort(handler1.txn_->getID(), ErrorCode::INTERNAL_ERROR);
    eventBase_.loopOnce();

    EXPECT_CALL(handler2, _detachTransaction());
    EXPECT_CALL(mockController_, detachSession(_));
    httpSession_->dropConnection();
  }

  eventBase_.loop();
}

TEST_F(MockCodecDownstreamTest, ConnFlowControlBlocked) {
  testConnFlowControlBlocked(false);
}

TEST_F(MockCodecDownstreamTest, ConnFlowControlTimeout) {
  testConnFlowControlBlocked(true);
}

TEST_F(MockCodecDownstreamTest, UnpausedLargePost) {
  // Make sure that a large POST that streams into the handler generates
  // connection level flow control so that the entire POST can be received.
  InSequence enforceOrder;
  NiceMock<MockHTTPHandler> handler1;
  unsigned kNumChunks = 10;
  auto wantToWrite = http2::kInitialWindow * kNumChunks;
  auto wantToWriteStr = folly::to<string>(wantToWrite);
  auto req1 = makePostRequest(wantToWrite);
  auto req1Body = makeBuf(wantToWrite);

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler1));
  EXPECT_CALL(handler1, _setTransaction(_))
      .WillOnce(SaveArg<0>(&handler1.txn_));

  EXPECT_CALL(handler1, _onHeadersComplete(_));
  for (unsigned i = 0; i < kNumChunks; ++i) {
    EXPECT_CALL(*codec_, generateWindowUpdate(_, 0, http2::kInitialWindow));
    EXPECT_CALL(handler1,
                _onBodyWithOffset(_, PtrBufHasLen(http2::kInitialWindow)));
    EXPECT_CALL(*codec_, generateWindowUpdate(_, 1, http2::kInitialWindow));
  }
  EXPECT_CALL(handler1, _onEOM());

  codecCallback_->onMessageBegin(1, req1.get());
  codecCallback_->onHeadersComplete(1, std::move(req1));
  // Give kNumChunks chunks, each of the maximum window size. We should generate
  // window update for each chunk
  for (unsigned i = 0; i < kNumChunks; ++i) {
    codecCallback_->onBody(1, makeBuf(http2::kInitialWindow), 0);
  }
  codecCallback_->onMessageComplete(1, false);

  // Just tear everything down now.
  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, IngressPausedWindowUpdate) {
  // Test sending a large response body while the handler has ingress paused. We
  // should process the ingress window_updates and deliver the full body
  InSequence enforceOrder;
  NiceMock<MockHTTPHandler> handler1;
  auto req = makeGetRequest();
  size_t respSize = http2::kInitialWindow * 10;
  unique_ptr<HTTPMessage> resp;
  unique_ptr<folly::IOBuf> respBody;
  tie(resp, respBody) = makeResponse(200, respSize);
  size_t written = 0;

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler1));
  EXPECT_CALL(handler1, _setTransaction(_))
      .WillOnce(SaveArg<0>(&handler1.txn_));

  EXPECT_CALL(handler1, _onHeadersComplete(_))
      .WillOnce(InvokeWithoutArgs([&]() {
        // Pause ingress. Make sure we process the window updates anyway
        handler1.txn_->pauseIngress();
      }));
  EXPECT_CALL(*codec_, generateHeader(_, _, _, _, _, _));
  EXPECT_CALL(*codec_, generateBody(_, _, _, _, _))
      .WillRepeatedly(Invoke([&](folly::IOBufQueue&,
                                 HTTPCodec::StreamID,
                                 std::shared_ptr<folly::IOBuf> chain,
                                 folly::Optional<uint8_t>,
                                 bool /*eom*/) {
        auto len = chain->computeChainDataLength();
        written += len;
        return len;
      }));

  codecCallback_->onWindowUpdate(0, respSize); // open conn-level window
  codecCallback_->onMessageBegin(1, req.get());
  codecCallback_->onHeadersComplete(1, std::move(req));
  EXPECT_TRUE(handler1.txn_->isIngressPaused());

  // Unblock txn-level flow control and try to egress the body
  codecCallback_->onWindowUpdate(1, respSize);
  handler1.txn_->sendHeaders(*resp);
  handler1.txn_->sendBody(std::move(respBody));

  eventBase_.loop();
  EXPECT_EQ(written, respSize);

  // Just tear everything down now.
  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, ShutdownThenSendPushHeaders) {
  // Test that notifying session of shutdown before sendHeaders() called on a
  // pushed txn lets that push txn finish.
  EXPECT_CALL(*codec_, supportsPushTransactions()).WillRepeatedly(Return(true));

  InSequence enforceOrder;
  NiceMock<MockHTTPHandler> handler;
  MockHTTPPushHandler pushHandler;
  auto req = makeGetRequest();

  EXPECT_CALL(mockController_, getRequestHandler(_, _))
      .WillOnce(Return(&handler));
  EXPECT_CALL(handler, _setTransaction(_)).WillOnce(SaveArg<0>(&handler.txn_));

  EXPECT_CALL(handler, _onHeadersComplete(_))
      .WillOnce(Invoke([&](std::shared_ptr<HTTPMessage>) {
        auto pushTxn = handler.txn_->newPushedTransaction(&pushHandler);
        // start shutdown process
        httpSession_->notifyPendingShutdown();
        // we should be able to process new requests
        EXPECT_TRUE(codec_->isReusable());
        pushHandler.sendPushHeaders(
            "/foo", "www.foo.com", 0, handler.txn_->getPriority());
        // we should* still* be able to process new requests
        EXPECT_TRUE(codec_->isReusable());
        pushTxn->sendEOM();
      }));
  EXPECT_CALL(pushHandler, _setTransaction(_))
      .WillOnce(SaveArg<0>(&pushHandler.txn_));
  EXPECT_CALL(*codec_, generatePushPromise(_, 2, _, _, _, _));
  EXPECT_CALL(*codec_, generateEOM(_, 2));
  EXPECT_CALL(pushHandler, _detachTransaction());
  EXPECT_CALL(handler, _onEOM()).WillOnce(Invoke([&] { handler.sendReply(); }));
  EXPECT_CALL(*codec_, generateHeader(_, 1, _, _, _, _));
  EXPECT_CALL(*codec_, generateEOM(_, 1));
  EXPECT_CALL(handler, _detachTransaction());

  codecCallback_->onMessageBegin(1, req.get());
  codecCallback_->onHeadersComplete(1, std::move(req));
  codecCallback_->onMessageComplete(1, false);

  // finish shutdown
  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();

  eventBase_.loop();
}

TEST_F(MockCodecDownstreamTest, ReadIobufChainShutdown) {
  // Given an ingress IOBuf chain of 2 parts, if we shutdown after reading the
  // first part of the chain, we shouldn't read the second part.  One way to
  // simulate a 2 part chain is to put more ingress in readBuf while we are
  // inside HTTPCodec::onIngress()

  InSequence enforceOrder;

  auto f = [&]() {
    void* buf;
    size_t bufSize;
    transportCb_->getReadBuffer(&buf, &bufSize);
    transportCb_->readDataAvailable(bufSize);
  };

  EXPECT_CALL(*codec_, onIngress(_))
      .WillOnce(Invoke([&](const IOBuf& buf) {
        // This first time, don't process any data. This will cause the
        // ingress chain to grow in size later.
        EXPECT_FALSE(buf.isChained());
        return 0;
      }))
      .WillOnce(Invoke([&](const IOBuf& buf) {
        // Now there should be a second buffer in the chain.
        EXPECT_TRUE(buf.isChained());
        // Shutdown writes. This enough to destroy the session.
        httpSession_->closeWhenIdle();
        return buf.length();
      }));
  // We shouldn't get a third onIngress() callback. This will be enforced by the
  // test framework since the codec is a strict mock.
  EXPECT_CALL(*codec_, isBusy());
  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));

  f();
  f(); // The first time wasn't processed, so this should make a len=2 chain.
  eventBase_.loop();
}

void MockCodecDownstreamTest::testGoaway(bool doubleGoaway,
                                         bool dropConnection) {
  NiceMock<MockHTTPHandler> handler;
  MockHTTPHandler pushHandler;

  liveGoaways_ = true;
  if (doubleGoaway) {
    EXPECT_CALL(mockController_, getRequestHandler(_, _))
        .WillOnce(Return(&handler));
    EXPECT_CALL(handler, _setTransaction(_))
        .WillOnce(SaveArg<0>(&handler.txn_));

    EXPECT_CALL(handler, _onHeadersComplete(_));
    EXPECT_CALL(handler, _onEOM()).WillOnce(Invoke([&] {
      handler.sendReply();
    }));
    EXPECT_CALL(*codec_, generateHeader(_, 1, _, _, _, _));
    EXPECT_CALL(*codec_, generateEOM(_, 1));
    EXPECT_CALL(handler, _detachTransaction());

    // Turn on double GOAWAY drain
    codec_->enableDoubleGoawayDrain();
  }

  // Send a GOAWAY acking uninitiated transactions
  EXPECT_FALSE(drainPending_);
  httpSession_->notifyPendingShutdown();
  EXPECT_EQ(drainPending_, doubleGoaway);
  EXPECT_FALSE(reusable_);

  if (doubleGoaway) {
    // Should be able to process new requests
    auto req1 = makeGetRequest();
    codecCallback_->onMessageBegin(1, req1.get());
    codecCallback_->onHeadersComplete(1, std::move(req1));
    codecCallback_->onMessageComplete(1, false);
  }

  folly::AsyncTransport::WriteCallback* cb = nullptr;
  {
    InSequence enforceOrder;
    EXPECT_CALL(*transport_, writeChain(_, _, _))
        .WillOnce(Invoke([&](folly::AsyncTransport::WriteCallback* callback,
                             const shared_ptr<IOBuf>,
                             WriteFlags) {
          // don't immediately flush the goaway
          cb = callback;
        }));
    if (doubleGoaway && !dropConnection) {
      EXPECT_CALL(*transport_, writeChain(_, _, _))
          .WillOnce(Invoke(this, &MockCodecDownstreamTest::onWriteChain));
    }
  }
  if (doubleGoaway || !dropConnection) {
    // single goaway, drop connection doesn't get onIngressEOF
    EXPECT_CALL(*codec_, onIngressEOF());
  }
  eventBase_.loopOnce();

  EXPECT_CALL(mockController_, detachSession(_));
  if (dropConnection) {
    EXPECT_CALL(*transport_, closeWithReset())
        .Times(AtLeast(1))
        .WillOnce(DoAll(Assign(&transportGood_, false), Invoke([cb] {
                          AsyncSocketException ex(AsyncSocketException::UNKNOWN,
                                                  "");
                          cb->writeErr(0, ex);
                        })));

    httpSession_->dropConnection();
  } else {
    EXPECT_CALL(*codec_, isBusy());
    httpSession_->closeWhenIdle();
    cb->writeSuccess();
  }
  EXPECT_FALSE(drainPending_);
  EXPECT_FALSE(reusable_);
}

TEST_F(MockCodecDownstreamTest, SendDoubleGoawayTimeout) {
  testGoaway(true, true);
}
TEST_F(MockCodecDownstreamTest, SendDoubleGoawayIdle) {
  testGoaway(true, false);
}
TEST_F(MockCodecDownstreamTest, SendGoawayTimeout) {
  testGoaway(false, true);
}
TEST_F(MockCodecDownstreamTest, SendGoawayIdle) {
  testGoaway(false, false);
}

TEST_F(MockCodecDownstreamTest, DropConnection) {
  NiceMock<MockHTTPHandler> handler;
  MockHTTPHandler pushHandler;

  liveGoaways_ = true;

  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));
  EXPECT_CALL(*transport_, closeWithReset())
      .Times(AtLeast(1))
      .WillOnce(Assign(&transportGood_, false));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, DropConnectionNogoaway) {
  NiceMock<MockHTTPHandler> handler;
  MockHTTPHandler pushHandler;

  liveGoaways_ = false;

  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));
  EXPECT_CALL(*transport_, closeNow())
      .Times(AtLeast(1))
      .WillOnce(Assign(&transportGood_, false));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, ShutdownThenError) {
  // Test that we ignore any errors after we shutdown the socket in HTTPSession.
  onIngressImpl([&] {
    // This executes as the implementation of HTTPCodec::onIngress()
    InSequence dummy;

    HTTPException err(HTTPException::Direction::INGRESS, "foo");
    err.setHttpStatusCode(400);
    HTTPMessage req = getGetRequest();
    EXPECT_CALL(mockController_, getParseErrorHandler(_, _, _))
        .WillOnce(Return(nullptr));

    // Creates and adds a txn to the session
    codecCallback_->onMessageBegin(1, &req);

    httpSession_->closeWhenIdle();

    codecCallback_->onError(1, err, false);
  });
  // flush the shutdown callback
  EXPECT_CALL(mockController_, detachSession(_));
  eventBase_.loopOnce();
}

TEST_F(MockCodecDownstreamTest, PingDuringShutdown) {
  onIngressImpl([&] {
    InSequence dummy;

    // Shutdown writes only. Since the session is empty, this normally
    // causes the session to close, but it is held open since we are in
    // the middle of parsing ingress.
    EXPECT_CALL(*codec_, isBusy());
    EXPECT_CALL(*codec_, onIngressEOF());
    httpSession_->closeWhenIdle();

    // We read a ping off the wire, which makes us enqueue a ping reply
    EXPECT_CALL(*codec_, generatePingReply(_, _)).WillOnce(Return(10));
    codecCallback_->onPingRequest(1);

    // When this function returns, the controller gets detachSession()
    EXPECT_CALL(mockController_, detachSession(_));
  });
}

TEST_F(MockCodecDownstreamTest, SettingsAck) {
  EXPECT_CALL(*codec_, generateSettingsAck(_));
  codecCallback_->onSettings({{SettingsId::INITIAL_WINDOW_SIZE, 4000}});
  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();
}

TEST_F(MockCodecDownstreamTest, TestSendCertificateRequest) {
  auto certRequestContext = folly::IOBuf::copyBuffer("0123456789abcdef");
  fizz::SignatureAlgorithms sigAlgs;
  sigAlgs.supported_signature_algorithms.push_back(
      SignatureScheme::ecdsa_secp256r1_sha256);
  std::vector<fizz::Extension> extensions;
  extensions.push_back(encodeExtension(std::move(sigAlgs)));

  std::unique_ptr<StrictMock<MockSecondaryAuthManager>> secondAuthManager_(
      new StrictMock<MockSecondaryAuthManager>());
  httpSession_->setSecondAuthManager(std::move(secondAuthManager_));
  auto authManager = dynamic_cast<MockSecondaryAuthManager*>(
      httpSession_->getSecondAuthManager());
  EXPECT_CALL(*codec_, getIngressSettings())
      .WillOnce(Return(&kIngressCertAuthSettings));
  EXPECT_CALL(*codec_, getEgressSettings())
      .WillOnce(Return(&kEgressCertAuthSettings));
  EXPECT_CALL(*authManager, createAuthRequest(_, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return std::make_pair(120, IOBuf::copyBuffer("authenticatorrequest"));
      }));
  EXPECT_CALL(*codec_, generateCertificateRequest(_, _, _))
      .WillOnce(Return(20));
  auto encodedSize = httpSession_->sendCertificateRequest(
      std::move(certRequestContext), std::move(extensions));
  EXPECT_EQ(encodedSize, 20);

  EXPECT_CALL(*codec_, onIngressEOF());
  EXPECT_CALL(mockController_, detachSession(_));
  httpSession_->dropConnection();
}
