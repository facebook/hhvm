/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/io/async/EventBase.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>
#include <proxygen/lib/http/codec/test/MockHTTPCodec.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <proxygen/lib/test/TestAsyncTransport.h>

using namespace proxygen;
using namespace testing;

using std::unique_ptr;

class DownstreamTransactionTest : public testing::Test {
 public:
  DownstreamTransactionTest() {
  }

  void SetUp() override {
    EXPECT_CALL(transport_, describe(_)).WillRepeatedly(Return());
  }

  void setupRequestResponseFlow(HTTPTransaction* txn,
                                uint32_t size,
                                bool delayResponse = false) {
    EXPECT_CALL(handler_, _setTransaction(txn));
    EXPECT_CALL(handler_, _detachTransaction());
    EXPECT_CALL(transport_, detach(txn));
    if (delayResponse) {
      EXPECT_CALL(handler_, _onHeadersComplete(_));
    } else {
      EXPECT_CALL(handler_, _onHeadersComplete(_))
          .WillOnce(Invoke([=](std::shared_ptr<HTTPMessage> /*msg*/) {
            auto response = makeResponse(200);
            txn->sendHeaders(*response.get());
            txn->sendBody(makeBuf(size));
            txn->sendEOM();
          }));
    }
    EXPECT_CALL(transport_, sendHeaders(txn, _, _, _))
        .WillOnce(
            Invoke([=](Unused, const HTTPMessage& headers, Unused, Unused) {
              EXPECT_EQ(headers.getStatusCode(), 200);
            }));
    EXPECT_CALL(transport_, sendBody(txn, _, false, false))
        .WillRepeatedly(Invoke(
            [=](Unused, std::shared_ptr<folly::IOBuf> body, Unused, Unused) {
              auto cur = body->computeChainDataLength();
              sent_ += cur;
              return cur;
            }));
    if (delayResponse) {
      EXPECT_CALL(transport_, sendEOM(txn, _));
    } else {
      EXPECT_CALL(transport_, sendEOM(txn, _))
          .WillOnce(InvokeWithoutArgs([=]() {
            CHECK_EQ(sent_, size);
            txn->onIngressBody(makeBuf(size), 0);
            txn->onIngressEOM();
            return 5;
          }));
    }
    EXPECT_CALL(handler_, _onBodyWithOffset(_, _))
        .WillRepeatedly(
            Invoke([=](uint64_t, std::shared_ptr<folly::IOBuf> body) {
              received_ += body->computeChainDataLength();
            }));
    EXPECT_CALL(handler_, _onEOM()).WillOnce(InvokeWithoutArgs([=] {
      CHECK_EQ(received_, size);
    }));
    EXPECT_CALL(transport_, notifyPendingEgress())
        .WillOnce(InvokeWithoutArgs([=] { txn->onWriteReady(size, 1); }))
        .WillOnce(DoDefault()); // The second call is for sending the eom

    txn->setHandler(&handler_);
  }

 protected:
  folly::EventBase eventBase_;
  MockHTTPTransactionTransport transport_;
  StrictMock<MockHTTPHandler> handler_;
  HTTP2PriorityQueue txnEgressQueue_;
  uint32_t received_{0};
  uint32_t sent_{0};
  std::unique_ptr<HTTPTransaction> txn_;

  HTTPTransaction& makeTxn(bool useFlowControl = false,
                           uint32_t receiveInitialWindowSize = 0,
                           uint32_t sendInitialWindowSize = 0) {
    txn_ = std::make_unique<HTTPTransaction>(TransportDirection::DOWNSTREAM,
                                             HTTPCodec::StreamID(1),
                                             1,
                                             transport_,
                                             txnEgressQueue_,
                                             &eventBase_.timer(),
                                             std::chrono::milliseconds(500),
                                             nullptr,
                                             useFlowControl,
                                             receiveInitialWindowSize,
                                             sendInitialWindowSize);
    return *txn_;
  }

  HTTPTransaction& makeExTxn() {
    txn_ = std::make_unique<HTTPTransaction>(TransportDirection::DOWNSTREAM,
                                             HTTPCodec::StreamID(2),
                                             1,
                                             transport_,
                                             txnEgressQueue_,
                                             &eventBase_.timer(),
                                             std::chrono::milliseconds(500),
                                             nullptr,
                                             false,
                                             0,
                                             0,
                                             http2::DefaultPriority,
                                             HTTPCodec::NoStream,
                                             HTTPCodec::ExAttributes(1, true));
    return *txn_;
  }
};

/**
 * Test that the the transaction properly forwards callbacks to the
 * handler and that it interacts with its transport as expected.
 */
TEST_F(DownstreamTransactionTest, SimpleCallbackForwarding) {
  // flow control is disabled
  auto& txn = makeTxn();
  setupRequestResponseFlow(&txn, 100);

  txn.onIngressHeadersComplete(makeGetRequest());
  eventBase_.loop();
}

TEST_F(DownstreamTransactionTest, InvariantViolationHandler) {
  auto& txn = makeTxn();

  EXPECT_CALL(handler_, _setTransaction(&txn));
  EXPECT_CALL(handler_, _onInvariantViolation(_))
      .WillOnce(Invoke([](const HTTPException& ex) {
        EXPECT_EQ(ex.getDirection(),
                  HTTPException::Direction::INGRESS_AND_EGRESS);
        EXPECT_EQ(ex.getErrno(), (int)HTTPException::Direction::EGRESS);
        EXPECT_EQ(std::string(ex.what()),
                  "Invalid egress state transition, state=Start, "
                  "event=sendBody, streamID=1");
      }));
  EXPECT_CALL(transport_, sendAbort(_, _));
  EXPECT_CALL(handler_, _detachTransaction());
  EXPECT_CALL(transport_, detach(&txn));

  txn.setHandler(&handler_);
  // Send body before headers -- oops;
  txn.sendBody(makeBuf(10));
  eventBase_.loop();
}

/**
 * Testing that we're sending a window update for simple requests
 */
TEST_F(DownstreamTransactionTest, RegularWindowUpdate) {
  auto& txn = makeTxn(true, // flow control enabled
                      400,
                      http2::kInitialWindow);
  uint32_t reqBodySize = 220;
  setupRequestResponseFlow(&txn, reqBodySize);

  // test that the window update is generated
  EXPECT_CALL(transport_, sendWindowUpdate(_, reqBodySize));

  // run the test
  txn.onIngressHeadersComplete(makeGetRequest());
  eventBase_.loop();
}

TEST_F(DownstreamTransactionTest, NoWindowUpdate) {
  auto& txn = makeTxn(true, // flow control enabled
                      450,  // more than 2x req size
                      http2::kInitialWindow);
  uint32_t reqBodySize = 220;
  setupRequestResponseFlow(&txn, reqBodySize, true);

  EXPECT_CALL(transport_, sendWindowUpdate(_, reqBodySize)).Times(0);

  // run the test
  txn.onIngressHeadersComplete(makeGetRequest());
  txn.onIngressBody(makeBuf(reqBodySize), 0);
  txn.onIngressEOM();
  auto response = makeResponse(200);
  txn.sendHeaders(*response.get());
  txn.sendBody(makeBuf(reqBodySize));
  txn.sendEOM();
  eventBase_.loop();
}

TEST_F(DownstreamTransactionTest, FlowControlInfoCorrect) {
  auto& txn = makeTxn(true, 450, 100);

  EXPECT_CALL(transport_, getFlowControlInfo(_))
      .WillOnce(Invoke([=](HTTPTransaction::FlowControlInfo* info) {
        info->flowControlEnabled_ = true;
        info->sessionSendWindow_ = 1;
        info->sessionRecvWindow_ = 2;
      }));
  HTTPTransaction::FlowControlInfo info;
  txn.getCurrentFlowControlInfo(&info);

  EXPECT_EQ(info.flowControlEnabled_, true);
  EXPECT_EQ(info.sessionSendWindow_, 1);
  EXPECT_EQ(info.sessionRecvWindow_, 2);
  EXPECT_EQ(info.streamRecvWindow_, 450);
  EXPECT_EQ(info.streamSendWindow_, 100);
}

TEST_F(DownstreamTransactionTest, ExpectingWindowUpdate) {
  auto& txn = makeTxn(true, 450, 100);
  uint32_t reqBodySize = 220;

  // Get a request, pause ingress, fill up the sendWindow, then expect for a
  // timeout to be scheduled.
  txn.onIngressHeadersComplete(makeGetRequest());
  txn.pauseIngress();
  txn.onIngressBody(makeBuf(reqBodySize), 0);
  txn.onIngressEOM();
  auto response = makeResponse(200);
  txn.sendHeaders(*response.get());
  txn.sendBody(makeBuf(reqBodySize));
  txn.sendEOM();
  txn.onWriteReady(1000, 1);
  EXPECT_EQ(eventBase_.timer().count(), 1);
}

TEST_F(DownstreamTransactionTest, NoWindowUpdateAfterDoneSending) {
  auto& txn = makeTxn(true, 450, 220);
  uint32_t reqBodySize = 220;

  // Ensure that after flushing an EOM we are not expecting window update.
  txn.onIngressHeadersComplete(makeGetRequest());
  txn.onIngressBody(makeBuf(reqBodySize), 0);
  auto response = makeResponse(200);
  txn.sendHeaders(*response.get());
  txn.sendBody(makeBuf(reqBodySize));
  txn.sendEOM();
  txn.onWriteReady(1000, 1);
  txn.pauseIngress();
  txn.onIngressEOM();
  EXPECT_EQ(eventBase_.timer().count(), 0);
}

/**
 * Testing window increase using window update; we're actually using this in
 * production to avoid bumping the window using the SETTINGS frame
 */
TEST_F(DownstreamTransactionTest, WindowIncrease) {
  // set initial window size higher than per-stream window
  auto& txn = makeTxn(true, // flow control enabled
                      http2::kInitialWindow,
                      http2::kInitialWindow);
  uint32_t reqSize = 500;
  setupRequestResponseFlow(&txn, reqSize);

  // we expect the difference from the per stream window and the initial window,
  // together with the bytes sent in the request
  uint32_t perStreamWindow = http2::kInitialWindow + 1024 * 1024;
  uint32_t expectedWindowUpdate = perStreamWindow - http2::kInitialWindow;
  EXPECT_CALL(transport_, sendWindowUpdate(_, expectedWindowUpdate));

  // use a higher window
  txn.setReceiveWindow(perStreamWindow);

  txn.onIngressHeadersComplete(makeGetRequest());
  eventBase_.loop();
}

/**
 * Testing that we're not sending window update when per-stream window size is
 * smaller than the initial window size
 */
TEST_F(DownstreamTransactionTest, WindowDecrease) {
  // set initial window size higher than per-stream window
  auto& txn = makeTxn(true, // flow control enabled
                      http2::kInitialWindow,
                      http2::kInitialWindow);
  setupRequestResponseFlow(&txn, 500);

  // in this case, there should be no window update, as we decrease the window
  // below the number of bytes we're sending
  EXPECT_CALL(transport_, sendWindowUpdate(_, _)).Times(0);

  // use a smaller window
  uint32_t perStreamWindow = http2::kInitialWindow - 1000;
  txn.setReceiveWindow(perStreamWindow);

  txn.onIngressHeadersComplete(makeGetRequest());
  eventBase_.loop();
}

TEST_F(DownstreamTransactionTest, ParseErrorCbs) {
  // Test where the transaction gets on parse error and then a body
  // callback. This is possible because codecs are stateless between
  // frames.

  auto& txn = makeTxn();

  HTTPException err(HTTPException::Direction::INGRESS, "test");
  err.setHttpStatusCode(400);

  InSequence enforceOrder;

  EXPECT_CALL(handler_, _setTransaction(&txn));
  EXPECT_CALL(handler_, _onError(_))
      .WillOnce(Invoke([](const HTTPException& ex) {
        ASSERT_EQ(ex.getDirection(), HTTPException::Direction::INGRESS);
        ASSERT_EQ(std::string(ex.what()), "test");
      }));
  // onBody() is suppressed since ingress is complete after ingress onError()
  // onEOM() is suppressed since ingress is complete after ingress onError()
  EXPECT_CALL(transport_, sendAbort(_, _));

  // New-ish: onError can be delivered more than once if the first error was
  // unidirectional and the other direction isn't closed before the second error
  // occurs.  Is this OK?
  EXPECT_CALL(handler_, _onError(_))
      .WillOnce(Invoke([](const HTTPException& ex) {
        EXPECT_EQ(ex.getDirection(),
                  HTTPException::Direction::INGRESS_AND_EGRESS);
        EXPECT_NE(
            std::string(ex.what()).find("onIngressBody after ingress closed"),
            std::string::npos);
      }));
  EXPECT_CALL(handler_, _detachTransaction());
  EXPECT_CALL(transport_, detach(&txn));

  txn.setHandler(&handler_);
  txn.onError(err);
  // Since the transaction is already closed for ingress, giving it
  // ingress body causes the transaction to be aborted and closed
  // immediately.
  txn.onIngressBody(makeBuf(10), 0);

  eventBase_.loop();
}

TEST_F(DownstreamTransactionTest, DetachFromNotify) {
  unique_ptr<StrictMock<MockHTTPHandler>> handler(
      new StrictMock<MockHTTPHandler>);

  auto& txn = makeTxn();

  InSequence enforceOrder;

  EXPECT_CALL(*handler, _setTransaction(&txn));
  EXPECT_CALL(*handler, _onHeadersComplete(_))
      .WillOnce(Invoke([&](std::shared_ptr<HTTPMessage> /*msg*/) {
        auto response = makeResponse(200);
        txn.sendHeaders(*response.get());
        txn.sendBody(makeBuf(10));
      }));
  EXPECT_CALL(transport_, sendHeaders(&txn, _, _, _))
      .WillOnce(Invoke([&](Unused, const HTTPMessage& headers, Unused, Unused) {
        EXPECT_EQ(headers.getStatusCode(), 200);
      }));
  EXPECT_CALL(transport_, notifyEgressBodyBuffered(10));
  EXPECT_CALL(transport_, notifyEgressBodyBuffered(-10))
      .WillOnce(InvokeWithoutArgs([&]() {
        txn.setHandler(nullptr);
        handler.reset();
      }));
  EXPECT_CALL(transport_, detach(&txn));

  HTTPException err(HTTPException::Direction::INGRESS_AND_EGRESS, "test");

  txn.setHandler(handler.get());
  txn.onIngressHeadersComplete(makeGetRequest());
  txn.onError(err);
}

TEST_F(DownstreamTransactionTest, DeferredEgress) {
  EXPECT_CALL(transport_, describe(_)).WillRepeatedly(Return());
  EXPECT_CALL(transport_, notifyPendingEgress()).WillRepeatedly(Return());

  auto& txn = makeTxn(true, 10, 10);

  InSequence enforceOrder;

  EXPECT_CALL(handler_, _setTransaction(&txn));
  EXPECT_CALL(handler_, _onHeadersComplete(_))
      .WillOnce(Invoke([&](std::shared_ptr<HTTPMessage> /*msg*/) {
        auto response = makeResponse(200);
        txn.sendHeaders(*response.get());
        txn.sendBody(makeBuf(10));
        txn.sendBody(makeBuf(20));
        txn.sendBody(makeBuf(30));
      }));
  EXPECT_CALL(transport_, sendHeaders(&txn, _, _, _))
      .WillOnce(Invoke([&](Unused, const HTTPMessage& headers, Unused, Unused) {
        EXPECT_EQ(headers.getStatusCode(), 200);
      }));

  // sendBody
  EXPECT_CALL(transport_, notifyEgressBodyBuffered(10));
  EXPECT_CALL(handler_, _onEgressPaused());
  EXPECT_CALL(transport_, notifyEgressBodyBuffered(20));
  EXPECT_CALL(transport_, notifyEgressBodyBuffered(30));

  txn.setHandler(&handler_);
  txn.onIngressHeadersComplete(makeGetRequest());

  // onWriteReady, send, then dequeue (window now full)
  EXPECT_CALL(transport_, notifyEgressBodyBuffered(-30));

  txn.onIngressWindowUpdate(20);
  EXPECT_EQ(txn.onWriteReady(30, 1), false);

  // Buffer released on error
  EXPECT_CALL(transport_, notifyEgressBodyBuffered(-30));
  EXPECT_CALL(handler_, _onError(_));
  EXPECT_CALL(handler_, _detachTransaction());
  EXPECT_CALL(transport_, detach(&txn));

  HTTPException err(HTTPException::Direction::INGRESS_AND_EGRESS, "test");
  txn.onError(err);
}

TEST_F(DownstreamTransactionTest, InternalError) {
  unique_ptr<StrictMock<MockHTTPHandler>> handler(
      new StrictMock<MockHTTPHandler>);

  auto& txn = makeTxn();

  InSequence enforceOrder;

  EXPECT_CALL(*handler, _setTransaction(&txn));
  EXPECT_CALL(*handler, _onHeadersComplete(_))
      .WillOnce(Invoke([&](std::shared_ptr<HTTPMessage> /*msg*/) {
        auto response = makeResponse(200);
        txn.sendHeaders(*response.get());
      }));
  EXPECT_CALL(transport_, sendHeaders(&txn, _, _, _))
      .WillOnce(Invoke([&](Unused, const HTTPMessage& headers, Unused, Unused) {
        EXPECT_EQ(headers.getStatusCode(), 200);
      }));
  EXPECT_CALL(transport_, sendAbort(&txn, ErrorCode::INTERNAL_ERROR));
  EXPECT_CALL(*handler, _detachTransaction());
  EXPECT_CALL(transport_, detach(&txn));

  HTTPException err(HTTPException::Direction::INGRESS_AND_EGRESS, "test");

  txn.setHandler(handler.get());
  txn.onIngressHeadersComplete(makeGetRequest());
  txn.sendAbort();
}

TEST_F(DownstreamTransactionTest, UnpausedFlowControlViolation) {
  InSequence enforceOrder;
  auto& txn = makeTxn(true, // flow control enabled
                      400,
                      http2::kInitialWindow);

  EXPECT_CALL(handler_, _setTransaction(&txn));
  EXPECT_CALL(handler_, _onHeadersComplete(_));
  EXPECT_CALL(transport_, sendAbort(&txn, ErrorCode::FLOW_CONTROL_ERROR));
  EXPECT_CALL(handler_, _onError(_))
      .WillOnce(Invoke([](const HTTPException& ex) {
        EXPECT_EQ(ex.getDirection(),
                  HTTPException::Direction::INGRESS_AND_EGRESS);
        EXPECT_NE(std::string(ex.what()).find("reserve failed"),
                  std::string::npos);
      }));
  EXPECT_CALL(handler_, _detachTransaction());
  EXPECT_CALL(transport_, detach(&txn));

  txn.setHandler(&handler_);
  txn.onIngressHeadersComplete(makePostRequest(401));
  txn.onIngressBody(makeBuf(401), 0);
}

TEST_F(DownstreamTransactionTest, ParseIngressErrorExTxnUnidirectional) {
  // Test where the ex transaction using QoS0 gets Ingress error
  auto& exTxn = makeExTxn();
  HTTPException err(HTTPException::Direction::INGRESS, "test");
  err.setHttpStatusCode(400);

  InSequence enforceOrder;

  EXPECT_CALL(handler_, _setTransaction(&exTxn));
  EXPECT_CALL(handler_, _onError(_))
      .WillOnce(Invoke([](const HTTPException& ex) {
        ASSERT_EQ(ex.getDirection(), HTTPException::Direction::INGRESS);
        ASSERT_EQ(std::string(ex.what()), "test");
      }));
  // onBody() is suppressed since ingress is complete after ingress onError()
  // onEOM() is suppressed since ingress is complete after ingress onError()
  EXPECT_CALL(transport_, sendAbort(_, _));
  EXPECT_CALL(handler_, _onError(_))
      .WillOnce(Invoke([](const HTTPException& ex) {
        EXPECT_EQ(ex.getDirection(),
                  HTTPException::Direction::INGRESS_AND_EGRESS);
        EXPECT_NE(
            std::string(ex.what()).find("onIngressBody after ingress closed"),
            std::string::npos);
      }));
  EXPECT_CALL(handler_, _detachTransaction());
  EXPECT_CALL(transport_, detach(&exTxn));

  exTxn.setHandler(&handler_);
  exTxn.onError(err);
  // Since the transaction is already closed for ingress, giving it
  // ingress body causes the transaction to be aborted and closed
  // immediately.
  exTxn.onIngressBody(makeBuf(10), 0);

  eventBase_.loop();
}

TEST_F(DownstreamTransactionTest, ParseIngressErrorExTxnNonUnidirectional) {
  // Test where the ex transaction using QoS0 gets Ingress error
  auto& exTxn = makeExTxn();
  HTTPException err(HTTPException::Direction::INGRESS, "test");
  err.setHttpStatusCode(400);

  InSequence enforceOrder;

  EXPECT_CALL(handler_, _setTransaction(&exTxn));
  // Ingress error will propagate
  // even if INGRESS state is completed for unidrectional ex_txn
  EXPECT_CALL(handler_, _onError(_))
      .WillOnce(Invoke([](const HTTPException& ex) {
        ASSERT_EQ(ex.getDirection(), HTTPException::Direction::INGRESS);
        ASSERT_EQ(std::string(ex.what()), "test");
      }));

  EXPECT_CALL(transport_, sendAbort(_, _));
  EXPECT_CALL(handler_, _onError(_))
      .WillOnce(Invoke([](const HTTPException& ex) {
        EXPECT_EQ(ex.getDirection(),
                  HTTPException::Direction::INGRESS_AND_EGRESS);
        EXPECT_NE(
            std::string(ex.what()).find("onIngressBody after ingress closed"),
            std::string::npos);
      }));
  EXPECT_CALL(handler_, _detachTransaction());
  EXPECT_CALL(transport_, detach(&exTxn));

  exTxn.setHandler(&handler_);
  exTxn.onError(err);
  // Since the transaction is already closed for ingress, giving it
  // ingress body causes the transaction to be aborted and closed
  // immediately.
  exTxn.onIngressBody(makeBuf(10), 0);

  eventBase_.loop();
}

TEST_F(DownstreamTransactionTest, IngressStateViolationWithByteEvents) {
  auto& txn = makeTxn();

  EXPECT_CALL(handler_, _setTransaction(&txn));
  txn.setHandler(&handler_);
  EXPECT_CALL(handler_, _onHeadersComplete(_));
  txn.onIngressHeadersComplete(makeGetRequest());
  EXPECT_CALL(handler_, _onEOM());
  txn.onIngressEOM();

  // When headers are sent, hold a byte event
  EXPECT_CALL(transport_, sendHeaders(&txn, _, _, _))
      .WillOnce(
          InvokeWithoutArgs([&txn] { txn.incrementPendingByteEvents(); }));
  txn.sendHeaders(getResponse(200));

  EXPECT_CALL(transport_, sendAbort(_, _));
  // The error is delivered immediately after abort is sent
  EXPECT_CALL(handler_, _onError(_))
      .WillOnce(Invoke([](const HTTPException& ex) {
        EXPECT_EQ(ex.getDirection(),
                  HTTPException::Direction::INGRESS_AND_EGRESS);
        EXPECT_NE(
            std::string(ex.what()).find("onIngressEOM after ingress closed"),
            std::string::npos);
      }));

  // Double EOM
  txn.onIngressEOM();

  // The transaction detaches after the byte event is released
  EXPECT_CALL(handler_, _detachTransaction());
  EXPECT_CALL(transport_, detach(&txn));
  txn.decrementPendingByteEvents();
  eventBase_.loop();
}
