/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>

using namespace testing;
using WTFCState = proxygen::WebTransport::FCState;
namespace {
constexpr uint32_t WT_APP_ERROR_1 = 19;
constexpr uint32_t WT_APP_ERROR_2 = 77;
folly::Optional<uint32_t> makeOpt(uint32_t n) {
  return n;
}
} // namespace

namespace proxygen::test {
class HTTPTransactionWebTransportTest : public testing::Test {
 public:
  void SetUp() override {
    setup(/*withHandler=*/true);
  }

  void setup(bool withHandler) {
    makeTxn();
    EXPECT_CALL(transport_, describe(_)).WillRepeatedly(Return());
    EXPECT_CALL(transport_, supportsWebTransport())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(transport_, usesEncodedApplicationErrorCodes())
        .WillRepeatedly(Return(true));
    if (withHandler) {
      handler_.expectTransaction();
      txn_->setHandler(&handler_);
      handler_.expectDetachTransaction();
    }
    EXPECT_CALL(transport_, sendHeaders(txn_.get(), _, _, false));
    EXPECT_CALL(transport_, notifyPendingEgress()).Times(AtLeast(0));
    EXPECT_CALL(transport_, detach(txn_.get())).WillOnce([this] {
      txn_.reset();
    });
    HTTPMessage req;
    req.setHTTPVersion(1, 1);
    req.setUpgradeProtocol("webtransport");
    req.setMethod(HTTPMethod::CONNECT);
    req.setURL("/webtransport");
    req.getHeaders().set(HTTP_HEADER_HOST, "www.facebook.com");
    txn_->sendHeaders(req);
    auto resp = std::make_unique<HTTPMessage>();
    resp->setHTTPVersion(1, 1);
    resp->setStatusCode(200);
    if (withHandler) {
      handler_.expectHeaders();
    }
    txn_->onIngressHeadersComplete(std::move(resp));

    wt_ = txn_->getWebTransport();
    EXPECT_NE(wt_, nullptr);
    auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
    ASSERT_NE(wtImpl, nullptr);
    wtImpl->setFlowControlLimits(0, kDefaultWTReceiveWindow);
  }

  void TearDown() override {
    if (txn_) {
      EXPECT_CALL(transport_, sendAbort(txn_.get(), _));
      txn_->sendAbort();
    }
  }

 protected:
  folly::EventBase eventBase_;
  testing::StrictMock<MockHTTPTransactionTransport> transport_;
  testing::StrictMock<MockHTTPHandler> handler_;
  HTTP2PriorityQueue txnEgressQueue_;
  std::unique_ptr<HTTPTransaction> txn_;

  static void readCallback(
      folly::Try<WebTransport::StreamData> streamData,
      bool expectException,
      size_t expectedLength,
      bool expectFin,
      folly::Optional<uint32_t> expectedErrorCode = folly::none) {
    VLOG(4) << __func__ << " expectException=" << uint64_t(expectException)
            << " expectedLength=" << expectedLength
            << " expectFin=" << expectFin;
    EXPECT_EQ(streamData.hasException(), expectException);
    if (expectException || streamData.hasException()) {
      if (expectedErrorCode) {
        auto wtEx = streamData.tryGetExceptionObject<WebTransport::Exception>();
        EXPECT_NE(wtEx, nullptr);
        if (wtEx) {
          EXPECT_EQ(wtEx->error, *expectedErrorCode);
        }
      }
      return;
    }
    if (streamData->data) {
      EXPECT_EQ(streamData->data->computeChainDataLength(), expectedLength);
    } else {
      EXPECT_EQ(expectedLength, 0);
    }
    EXPECT_EQ(streamData->fin, expectFin);
  }

  HTTPTransaction& makeTxn() {
    txn_ = std::make_unique<HTTPTransaction>(TransportDirection::DOWNSTREAM,
                                             HTTPCodec::StreamID(1),
                                             0,
                                             transport_,
                                             txnEgressQueue_,
                                             &evb_.timer());
    return *txn_;
  }
  WebTransport* wt_{nullptr};
  folly::EventBase evb_;
};

class MockDeliveryCallback : public WebTransport::ByteEventCallback {
 public:
  MOCK_METHOD(void, onByteEvent, (quic::StreamId, uint64_t), (noexcept));

  MOCK_METHOD(void,
              onByteEventCanceled,
              (quic::StreamId, uint64_t),
              (noexcept));
};

TEST_F(HTTPTransactionWebTransportTest, CreateStreams) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setBidiStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportBidiStream()).WillOnce(Return(0));
  EXPECT_CALL(transport_, initiateReadOnBidiStream(_, _))
      .WillOnce(Return(folly::unit));
  auto res = wt_->createBidiStream();
  EXPECT_TRUE(res.hasValue());
  EXPECT_CALL(transport_, resetWebTransportEgress(0, WT_APP_ERROR_1))
      .WillOnce(Return(folly::unit));
  EXPECT_EQ(res->writeHandle->getID(), 0);
  res->writeHandle->resetStream(WT_APP_ERROR_1);
  EXPECT_CALL(transport_, stopReadingWebTransportIngress(0, _))
      .WillRepeatedly(Return(folly::unit));
  res->readHandle->stopSending(WT_APP_ERROR_2);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto res2 = wt_->createUniStream();
  EXPECT_TRUE(res2.hasValue());
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, true, nullptr))
      .WillOnce(Return(WTFCState::UNBLOCKED));

  wtImpl->onMaxData(1000);
  res2.value()->writeStreamData(nullptr, true, nullptr);

  // Try creating streams but fail at transport
  EXPECT_CALL(transport_, newWebTransportBidiStream())
      .WillOnce(Return(folly::makeUnexpected(
          WebTransport::ErrorCode::STREAM_CREATION_ERROR)));
  EXPECT_EQ(wt_->createBidiStream().error(),
            WebTransport::ErrorCode::STREAM_CREATION_ERROR);
  EXPECT_CALL(transport_, newWebTransportUniStream())
      .WillOnce(Return(folly::makeUnexpected(
          WebTransport::ErrorCode::STREAM_CREATION_ERROR)));
  EXPECT_EQ(wt_->createUniStream().error(),
            WebTransport::ErrorCode::STREAM_CREATION_ERROR);

  EXPECT_CALL(transport_, sendEOM(txn_.get(), nullptr));
  wt_->closeSession();
}

TEST_F(HTTPTransactionWebTransportTest, ReadStream) {
  WebTransport::StreamReadHandle* readHandle{nullptr};
  EXPECT_CALL(handler_, onWebTransportUniStream(_, _))
      .WillOnce(SaveArg<1>(&readHandle));

  auto implHandle = txn_->onWebTransportUniStream(0);
  EXPECT_NE(readHandle, nullptr);

  // read with no data buffered
  auto fut = readHandle->readStreamData()
                 .via(&eventBase_)
                 .thenTry([](auto streamData) {
                   readCallback(std::move(streamData), false, 10, false);
                 });
  EXPECT_FALSE(fut.isReady());

  EXPECT_CALL(transport_, sendWTMaxData(kDefaultWTReceiveWindow + 10))
      .WillOnce(Return(folly::unit));
  implHandle->dataAvailable(makeBuf(10), false);
  EXPECT_FALSE(fut.isReady());
  eventBase_.loopOnce();
  EXPECT_TRUE(fut.isReady());

  // buffer data with no read
  implHandle->dataAvailable(makeBuf(32768), false);

  // full buffer, blocked
  EXPECT_CALL(transport_, pauseWebTransportIngress(0));
  EXPECT_CALL(transport_, readWebTransportData(_, _)).WillOnce(Invoke([] {
    return std::make_pair(makeBuf(32768), false);
  }));
  EXPECT_CALL(transport_, sendWTMaxData(kDefaultWTReceiveWindow + 10 + 65536))
      .WillOnce(Return(folly::unit));
  implHandle->readAvailable(0);
  EXPECT_CALL(transport_, resumeWebTransportIngress(0));
  fut = readHandle->readStreamData()
            .via(&eventBase_)
            .thenTry([](auto streamData) {
              readCallback(std::move(streamData), false, 65536, false);
            });
  EXPECT_FALSE(fut.isReady());
  eventBase_.loopOnce();
  EXPECT_TRUE(fut.isReady());

  // fin
  fut = readHandle->readStreamData()
            .via(&eventBase_)
            .thenTry([](auto streamData) {
              readCallback(std::move(streamData), false, 0, true);
            });
  EXPECT_FALSE(fut.isReady());

  // it gets stopReadingWebTransportIngress when the EOF is read out
  EXPECT_CALL(transport_, stopReadingWebTransportIngress(0, _))
      .WillRepeatedly(Return(folly::unit));

  implHandle->dataAvailable(nullptr, true);
  eventBase_.loopOnce();
  EXPECT_TRUE(fut.isReady());
}

TEST_F(HTTPTransactionWebTransportTest, ReadStreamBufferedError) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  WebTransport::StreamReadHandle* readHandle{nullptr};
  EXPECT_CALL(handler_, onWebTransportUniStream(_, _))
      .WillOnce(SaveArg<1>(&readHandle));

  auto implHandle = txn_->onWebTransportUniStream(2);
  EXPECT_NE(readHandle, nullptr);

  implHandle->readError(implHandle->getID(),
                        quic::QuicError(quic::ApplicationErrorCode(
                            WebTransport::toHTTPErrorCode(WT_APP_ERROR_2))));

  // read with buffered error - simulate coming directly from QUIC with
  // encoded error code
  auto fut =
      readHandle->readStreamData()
          .via(&eventBase_)
          .thenTry([](auto streamData) {
            readCallback(std::move(streamData), true, 0, false, WT_APP_ERROR_2);
          });
  eventBase_.loopOnce();
  EXPECT_TRUE(fut.isReady());
}

TEST_F(HTTPTransactionWebTransportTest, ReadStreamError) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  WebTransport::StreamReadHandle* readHandle{nullptr};
  EXPECT_CALL(handler_, onWebTransportUniStream(_, _))
      .WillOnce(SaveArg<1>(&readHandle));

  auto implHandle = txn_->onWebTransportUniStream(2);
  EXPECT_NE(readHandle, nullptr);

  // read with nothing queued
  auto fut =
      readHandle->readStreamData()
          .via(&eventBase_)
          .thenTry([](auto streamData) {
            readCallback(std::move(streamData), true, 0, false, WT_APP_ERROR_2);
          });
  EXPECT_FALSE(fut.isReady());

  // Don't encode the error, it will be passed directly
  implHandle->readError(
      implHandle->getID(),
      quic::QuicError(quic::ApplicationErrorCode(WT_APP_ERROR_2)));
  eventBase_.loopOnce();
  EXPECT_TRUE(fut.isReady());
}

TEST_F(HTTPTransactionWebTransportTest, ReadStreamCancel) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  WebTransport::StreamReadHandle* readHandle{nullptr};
  EXPECT_CALL(handler_, onWebTransportUniStream(_, _))
      .WillOnce(SaveArg<1>(&readHandle));

  txn_->onWebTransportUniStream(2);
  EXPECT_NE(readHandle, nullptr);

  // Get the read future
  auto fut = readHandle->readStreamData();

  // Cancel the future, the transport will get a STOP_SENDING
  EXPECT_CALL(
      transport_,
      stopReadingWebTransportIngress(2, makeOpt(WebTransport::kInternalError)))
      .WillOnce(Return(folly::unit));
  fut.cancel();
  EXPECT_TRUE(fut.isReady());
  EXPECT_NE(fut.result().tryGetExceptionObject<folly::FutureCancellation>(),
            nullptr);
}

TEST_F(HTTPTransactionWebTransportTest, WriteFails) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto res = wt_->createUniStream();
  EXPECT_TRUE(res.hasValue());

  wtImpl->onMaxData(1000);

  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, nullptr))
      .WillOnce(
          Return(folly::makeUnexpected(WebTransport::ErrorCode::SEND_ERROR)));
  EXPECT_EQ(res.value()->writeStreamData(makeBuf(10), false, nullptr).error(),
            WebTransport::ErrorCode::SEND_ERROR);
}

TEST_F(HTTPTransactionWebTransportTest, WriteStreamPauseStopSending) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto writeHandle = wt_->createUniStream();
  EXPECT_FALSE(writeHandle.hasError());

  // Grant flow control space before any writes
  wtImpl->onMaxData(1000);

  // Block write, then resume
  bool ready = false;
  quic::StreamWriteCallback* wcb{nullptr};
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, nullptr))
      .WillOnce(Return(WTFCState::BLOCKED));
  auto res = writeHandle.value()->writeStreamData(makeBuf(10), false, nullptr);
  EXPECT_TRUE(res.hasValue());
  EXPECT_CALL(transport_, notifyPendingWriteOnStream(2, testing::_))
      .WillOnce(DoAll(SaveArg<1>(&wcb), Return(folly::unit)));
  writeHandle.value()
      ->awaitWritable()
      .value()
      .via(&eventBase_)
      .thenTry([&ready](auto writeReady) {
        EXPECT_FALSE(writeReady.hasException());
        ready = true;
      });
  EXPECT_FALSE(ready);
  wcb->onStreamWriteReady(2, 65536);
  eventBase_.loopOnce();
  EXPECT_TRUE(ready);

  // Block write/stop sending
  ready = false;
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, nullptr))
      .WillOnce(Return(WTFCState::BLOCKED));
  auto res2 = writeHandle.value()->writeStreamData(makeBuf(10), false, nullptr);
  EXPECT_TRUE(res2.hasValue());
  EXPECT_CALL(transport_, notifyPendingWriteOnStream(2, testing::_))
      .WillOnce(DoAll(SaveArg<1>(&wcb), Return(folly::unit)));
  writeHandle.value()
      ->awaitWritable()
      .value()
      .via(&eventBase_)
      .thenTry([&ready, &writeHandle, this](auto writeReady) {
        EXPECT_TRUE(
            writeReady.withException([](const WebTransport::Exception& ex) {
              EXPECT_EQ(ex.error, WT_APP_ERROR_2);
            }));
        EXPECT_CALL(transport_, resetWebTransportEgress(2, WT_APP_ERROR_1));
        writeHandle.value()->resetStream(WT_APP_ERROR_1);
        ready = true;
      });
  EXPECT_FALSE(ready);
  txn_->onWebTransportStopSending(2, WT_APP_ERROR_2);
  eventBase_.loopOnce();
  EXPECT_TRUE(ready);
}

TEST_F(HTTPTransactionWebTransportTest, AwaitWritableCancel) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto writeHandle = wt_->createUniStream();
  EXPECT_FALSE(writeHandle.hasError());

  // Block write
  quic::StreamWriteCallback* wcb{nullptr};
  EXPECT_CALL(transport_, notifyPendingWriteOnStream(2, testing::_))
      .WillOnce(DoAll(SaveArg<1>(&wcb), Return(folly::unit)));
  // awaitWritable
  auto fut = writeHandle.value()->awaitWritable().value();

  // Cancel future
  fut.cancel();
  EXPECT_TRUE(fut.isReady());
  EXPECT_TRUE(fut.hasException());
  EXPECT_NE(fut.result().tryGetExceptionObject<folly::FutureCancellation>(),
            nullptr);

  // awaitWritable again
  bool ready = false;
  EXPECT_CALL(transport_, notifyPendingWriteOnStream(2, testing::_))
      .WillOnce(DoAll(SaveArg<1>(&wcb), Return(folly::unit)));
  writeHandle.value()
      ->awaitWritable()
      .value()
      .via(&eventBase_)
      .thenTry([&ready, &writeHandle, this](auto writeReady) {
        EXPECT_TRUE(writeReady.hasValue());
        EXPECT_CALL(transport_, resetWebTransportEgress(2, WT_APP_ERROR_1));
        writeHandle.value()->resetStream(WT_APP_ERROR_1);
        ready = true;
      });
  EXPECT_FALSE(ready);

  // Resume - only happens once because the reset, maybe?
  wtImpl->onMaxData(1000);
  wcb->onStreamWriteReady(2, 65536);
  eventBase_.loopOnce();
  EXPECT_TRUE(ready);
}

TEST_F(HTTPTransactionWebTransportTest, BidiStreamEdgeCases) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setBidiStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  WebTransport::BidiStreamHandle streamHandle{};
  EXPECT_CALL(handler_, onWebTransportBidiStream(_, _))
      .WillOnce(SaveArg<1>(&streamHandle));

  auto bidiHandle = txn_->onWebTransportBidiStream(0);
  EXPECT_NE(streamHandle.readHandle, nullptr);
  EXPECT_NE(streamHandle.writeHandle, nullptr);

  // deliver EOF before read
  bidiHandle.readHandle->dataAvailable(nullptr, true);

  // it gets stopReadingWebTransportIngress when the EOF is read out
  EXPECT_CALL(transport_,
              stopReadingWebTransportIngress(0, folly::Optional<uint32_t>()));

  EXPECT_CALL(transport_, sendWTMaxData(kDefaultWTReceiveWindow)).Times(0);
  auto fut = streamHandle.readHandle->readStreamData()
                 .via(&eventBase_)
                 .thenTry([](auto streamData) {
                   readCallback(std::move(streamData), false, 0, true);
                 });
  EXPECT_FALSE(fut.isReady());
  eventBase_.loopOnce();
  EXPECT_TRUE(fut.isReady());

  // Cancellation handling
  folly::CancellationCallback writeCancel(
      streamHandle.writeHandle->getCancelToken(), [&streamHandle] {
        // Write cancelled:
        // We can retrieve the stop sending code from the handle
        EXPECT_EQ(*streamHandle.writeHandle->stopSendingErrorCode(),
                  WT_APP_ERROR_2);
        // attempt to write, will error, but don't reset the stream
        auto res = streamHandle.writeHandle->writeStreamData(
            makeBuf(10), true, nullptr);
        EXPECT_TRUE(res.hasError());
        EXPECT_EQ(res.error(), WebTransport::ErrorCode::STOP_SENDING);
      });
  // Deliver SS
  txn_->onWebTransportStopSending(0, WT_APP_ERROR_2);
  EXPECT_CALL(transport_,
              resetWebTransportEgress(0, WebTransport::kInternalError));
  // Note the egress stream was not reset, will be reset when the txn detaches
}

TEST_F(HTTPTransactionWebTransportTest, StreamDetachWithOpenStreams) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setBidiStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportBidiStream()).WillOnce(Return(0));
  EXPECT_CALL(transport_, initiateReadOnBidiStream(_, _))
      .WillOnce(Return(folly::unit));
  auto res = wt_->createBidiStream();
  EXPECT_FALSE(res.hasError());
  bool readCancelled = false;
  bool writeCancelled = false;
  folly::CancellationCallback readCancel(
      res->readHandle->getCancelToken(), [&readCancelled, &res, this] {
        res->readHandle->readStreamData()
            .via(&eventBase_)
            .thenValue([](auto) {})
            .thenError(folly::tag_t<const WebTransport::Exception&>{},
                       [](auto const& ex) {
                         VLOG(4) << "readCancelled";
                         EXPECT_EQ(ex.error, WebTransport::kInternalError);
                       });
        readCancelled = true;
      });
  folly::CancellationCallback writeCancel(res->writeHandle->getCancelToken(),
                                          [&] {
                                            VLOG(4) << "writeCancelled";
                                            writeCancelled = true;
                                          });
  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS, "aborted");
  handler_.expectError();
  EXPECT_CALL(transport_, resetWebTransportEgress(0, _));
  EXPECT_CALL(
      transport_,
      stopReadingWebTransportIngress(0, makeOpt(WebTransport::kInternalError)));
  txn_->onError(ex);
  EXPECT_TRUE(readCancelled);
  EXPECT_TRUE(writeCancelled);
}

TEST_F(HTTPTransactionWebTransportTest, NoHandler) {
  TearDown();
  setup(/*withHandler=*/false);
  EXPECT_CALL(
      transport_,
      stopReadingWebTransportIngress(0, makeOpt(WebTransport::kInternalError)))
      .RetiresOnSaturation();
  EXPECT_CALL(transport_,
              resetWebTransportEgress(0, WebTransport::kInternalError));
  txn_->onWebTransportBidiStream(0);
  EXPECT_CALL(
      transport_,
      stopReadingWebTransportIngress(1, makeOpt(WebTransport::kInternalError)));
  txn_->onWebTransportUniStream(1);
}

TEST_F(HTTPTransactionWebTransportTest, StreamIDAPIs) {
  EXPECT_CALL(transport_, newWebTransportBidiStream()).WillOnce(Return(0));
  quic::StreamReadCallback* quicReadCallback{nullptr};
  EXPECT_CALL(transport_, initiateReadOnBidiStream(_, _))
      .WillOnce(DoAll(SaveArg<1>(&quicReadCallback), Return(folly::unit)));
  auto res = wt_->createBidiStream();
  auto id = res->readHandle->getID();

  // read by id
  auto fut = wt_->readStreamData(id)
                 .value()
                 .via(&eventBase_)
                 .thenTry([](auto streamData) {
                   readCallback(std::move(streamData), false, 10, 0);
                 });
  EXPECT_FALSE(fut.isReady());
  EXPECT_CALL(transport_, readWebTransportData(_, _)).WillOnce(Invoke([] {
    return std::make_pair(makeBuf(10), false);
  }));
  EXPECT_CALL(transport_, sendWTMaxData(kDefaultWTReceiveWindow + 10))
      .WillOnce(Return(folly::unit));
  quicReadCallback->readAvailable(id);
  eventBase_.loopOnce();
  EXPECT_TRUE(fut.isReady());

  // stopSending by ID
  EXPECT_CALL(transport_, stopReadingWebTransportIngress(0, _))
      .WillRepeatedly(Return(folly::unit));
  wt_->stopSending(id, WT_APP_ERROR_1);

  // write by ID
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->onMaxData(1000);
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(id, testing::_, false, nullptr))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  auto res2 = wt_->writeStreamData(id, makeBuf(10), false, nullptr);
  EXPECT_TRUE(res2.hasValue());

  // resetStream by ID
  EXPECT_CALL(transport_, resetWebTransportEgress(id, WT_APP_ERROR_2));
  wt_->resetStream(id, WT_APP_ERROR_2);
}

TEST_F(HTTPTransactionWebTransportTest, InvalidStreamIDAPIs) {
  uint64_t id = 7;

  EXPECT_EQ(wt_->stopSending(id, WT_APP_ERROR_1).error(),
            WebTransport::ErrorCode::INVALID_STREAM_ID);
  EXPECT_EQ(wt_->resetStream(id, WT_APP_ERROR_2).error(),
            WebTransport::ErrorCode::INVALID_STREAM_ID);
  EXPECT_EQ(wt_->readStreamData(id).error(),
            WebTransport::ErrorCode::INVALID_STREAM_ID);
  EXPECT_EQ(wt_->writeStreamData(id, makeBuf(10), false, nullptr).error(),
            WebTransport::ErrorCode::INVALID_STREAM_ID);
}

TEST_F(HTTPTransactionWebTransportTest, SendDatagram) {
  EXPECT_CALL(transport_, sendDatagram(_)).WillOnce(Return(folly::unit));
  EXPECT_TRUE(wt_->sendDatagram(makeBuf(100)));
}

TEST_F(HTTPTransactionWebTransportTest, RefreshTimeout) {
  txn_->setIdleTimeout(std::chrono::milliseconds(100));
  evb_.runAfterDelay(
      [this] {
        WebTransport::StreamReadHandle* readHandle{nullptr};
        EXPECT_CALL(handler_, onWebTransportUniStream(_, _))
            .WillOnce(SaveArg<1>(&readHandle));

        txn_->onWebTransportUniStream(0);
        EXPECT_NE(readHandle, nullptr);
      },
      50);
  evb_.runAfterDelay(
      [this] {
        EXPECT_CALL(transport_,
                    stopReadingWebTransportIngress(
                        0, makeOpt(WebTransport::kSessionGone)))
            .WillOnce(Return(folly::unit));
        handler_.expectEOM();
        txn_->onIngressEOM();
        EXPECT_CALL(transport_, sendEOM(txn_.get(), nullptr));
        wt_->closeSession();
      },
      150);
  evb_.loop();
}

TEST_F(HTTPTransactionWebTransportTest, StopSendingThenAbort) {
  WebTransport::StreamReadHandle* readHandle{nullptr};
  EXPECT_CALL(handler_, onWebTransportUniStream(_, _))
      .WillOnce(SaveArg<1>(&readHandle));

  txn_->onWebTransportUniStream(0);
  EXPECT_NE(readHandle, nullptr);
  EXPECT_CALL(transport_, stopReadingWebTransportIngress(0, _))
      .WillRepeatedly(Return(folly::unit));
  readHandle->stopSending(WT_APP_ERROR_2);
  // there's no way to abort this stream anymore.  stopSending removes the
  // read callback.
  // Questioning my life choices -- there's no way now to get the reset error
  // code.
  // 1. Should you be able to resetStream() while write is outstanding? (yes)
  // 2. Should you be able to stopSending() when read is outstanding?
  eventBase_.loopOnce();
}

TEST_F(HTTPTransactionWebTransportTest, WriteBufferingBasic) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto writeHandle = wt_->createUniStream();
  EXPECT_FALSE(writeHandle.hasError());

  // no flow control space initially, writes should be buffered
  auto res = writeHandle.value()->writeStreamData(makeBuf(100), false, nullptr);
  EXPECT_TRUE(res.hasValue());
  EXPECT_EQ(*res, WTFCState::BLOCKED);

  // grant flow control and verify buffered data is flushed
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, nullptr))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  wtImpl->onMaxData(1000);

  // write more data with available flow control, should send immediately
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, nullptr))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  res = writeHandle.value()->writeStreamData(makeBuf(50), false, nullptr);
  EXPECT_TRUE(res.hasValue());
  EXPECT_EQ(*res, WTFCState::UNBLOCKED);
  EXPECT_CALL(transport_, resetWebTransportEgress(_, _));
}

TEST_F(HTTPTransactionWebTransportTest, WriteBufferingEOF) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto writeHandle = wt_->createUniStream();
  EXPECT_FALSE(writeHandle.hasError());

  // write data with EOF but no flow control, should buffer
  auto res = writeHandle.value()->writeStreamData(makeBuf(100), true, nullptr);
  EXPECT_TRUE(res.hasValue());
  EXPECT_EQ(*res, WTFCState::BLOCKED);

  // grant flow control, should flush buffered data with EOF
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, true, nullptr))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  wtImpl->onMaxData(1000);
}

TEST_F(HTTPTransactionWebTransportTest, WriteBufferingPartialSend) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto writeHandle = wt_->createUniStream();
  EXPECT_FALSE(writeHandle.hasError());

  // write large data without flow control, should buffer
  auto res =
      writeHandle.value()->writeStreamData(makeBuf(1000), false, nullptr);
  EXPECT_TRUE(res.hasValue());
  EXPECT_EQ(*res, WTFCState::BLOCKED);

  // grant partial flow control, should flush partial data
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, nullptr))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  wtImpl->onMaxData(500);

  // grant more flow control, should flush remaining buffered data
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, nullptr))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  wtImpl->onMaxData(1000);
  EXPECT_CALL(transport_, resetWebTransportEgress(_, _));
}

TEST_F(HTTPTransactionWebTransportTest, StreamWriteReadyCallback) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto writeHandle = wt_->createUniStream();
  EXPECT_FALSE(writeHandle.hasError());

  // write data without flow control, should buffer
  auto res = writeHandle.value()->writeStreamData(makeBuf(100), false, nullptr);
  EXPECT_TRUE(res.hasValue());
  EXPECT_EQ(*res, WTFCState::BLOCKED);

  bool writeReady = false;
  quic::StreamWriteCallback* wcb{nullptr};
  EXPECT_CALL(transport_, notifyPendingWriteOnStream(2, testing::_))
      .WillOnce(DoAll(SaveArg<1>(&wcb), Return(folly::unit)));
  writeHandle.value()
      ->awaitWritable()
      .value()
      .via(&eventBase_)
      .thenTry([&writeReady](const auto& result) {
        EXPECT_FALSE(result.hasException());
        writeReady = true;
      });
  EXPECT_FALSE(writeReady);

  // trigger onStreamWriteReady, should flush buffered writes and resolve
  // promise
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, nullptr))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  wtImpl->onMaxData(1000);
  wcb->onStreamWriteReady(2, 65536);
  eventBase_.loopOnce();
  EXPECT_TRUE(writeReady);
  EXPECT_CALL(transport_, resetWebTransportEgress(_, _));
}

TEST_F(HTTPTransactionWebTransportTest, FlushBufferedWritesMultiple) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto writeHandle = wt_->createUniStream();
  EXPECT_FALSE(writeHandle.hasError());

  auto dcb1 = std::make_unique<StrictMock<MockDeliveryCallback>>();
  auto dcb2 = std::make_unique<StrictMock<MockDeliveryCallback>>();
  auto dcb3 = std::make_unique<StrictMock<MockDeliveryCallback>>();

  // write multiple chunks without flow control, should buffer into three
  // entries
  auto res1 =
      writeHandle.value()->writeStreamData(makeBuf(100), false, dcb1.get());
  EXPECT_TRUE(res1.hasValue());
  EXPECT_EQ(*res1, WTFCState::BLOCKED);

  auto res2 =
      writeHandle.value()->writeStreamData(makeBuf(200), false, dcb2.get());
  EXPECT_TRUE(res2.hasValue());
  EXPECT_EQ(*res2, WTFCState::BLOCKED);

  auto res3 =
      writeHandle.value()->writeStreamData(makeBuf(150), false, dcb3.get());
  EXPECT_TRUE(res3.hasValue());
  EXPECT_EQ(*res3, WTFCState::BLOCKED);

  // grant flow control, should flush first buffered entry
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, dcb1.get()))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  wtImpl->onMaxData(100);

  // grant more flow control, should flush second buffered entry
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, dcb2.get()))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  wtImpl->onMaxData(300);

  // grant more flow control, should flush third buffered entry
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, dcb3.get()))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  wtImpl->onMaxData(500);
  EXPECT_CALL(transport_, resetWebTransportEgress(_, _));
}

TEST_F(HTTPTransactionWebTransportTest, CoalescedWrites) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto writeHandle = wt_->createUniStream();
  EXPECT_FALSE(writeHandle.hasError());

  // write multiple chunks without flow control, should buffer all into one
  // coalesced entry because of no delivery callbacks
  auto res1 =
      writeHandle.value()->writeStreamData(makeBuf(100), false, nullptr);
  EXPECT_TRUE(res1.hasValue());
  EXPECT_EQ(*res1, WTFCState::BLOCKED);

  auto res2 =
      writeHandle.value()->writeStreamData(makeBuf(200), false, nullptr);
  EXPECT_TRUE(res2.hasValue());
  EXPECT_EQ(*res2, WTFCState::BLOCKED);

  auto res3 =
      writeHandle.value()->writeStreamData(makeBuf(150), false, nullptr);
  EXPECT_TRUE(res3.hasValue());
  EXPECT_EQ(*res3, WTFCState::BLOCKED);

  // grant flow control, should flush the entire buffered entry
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, nullptr))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  wtImpl->onMaxData(1000);
  EXPECT_CALL(transport_, resetWebTransportEgress(_, _));
}

TEST_F(HTTPTransactionWebTransportTest, CoalescedWritesPartialFlowControl) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/4,
      /*targetConcurrentStreams=*/4);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto writeHandle = wt_->createUniStream();
  EXPECT_FALSE(writeHandle.hasError());

  auto res1 =
      writeHandle.value()->writeStreamData(makeBuf(100), false, nullptr);
  EXPECT_TRUE(res1.hasValue());
  EXPECT_EQ(*res1, WTFCState::BLOCKED);

  auto res2 =
      writeHandle.value()->writeStreamData(makeBuf(200), false, nullptr);
  EXPECT_TRUE(res2.hasValue());
  EXPECT_EQ(*res2, WTFCState::BLOCKED);

  auto res3 =
      writeHandle.value()->writeStreamData(makeBuf(150), false, nullptr);
  EXPECT_TRUE(res3.hasValue());
  EXPECT_EQ(*res3, WTFCState::BLOCKED);

  // due to coalescing with nullptr callbacks, we expect only 2 transport calls
  // (not 3) when flow control is granted in non-aligned chunks
  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, nullptr))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  wtImpl->onMaxData(250);

  EXPECT_CALL(transport_,
              sendWebTransportStreamData(2, testing::_, false, nullptr))
      .WillOnce(Return(WTFCState::UNBLOCKED));
  wtImpl->onMaxData(450);

  EXPECT_CALL(transport_, resetWebTransportEgress(_, _));
}

TEST_F(HTTPTransactionWebTransportTest, RecvFlowControlCloseSession) {
  WebTransport::StreamReadHandle* readHandle{nullptr};
  EXPECT_CALL(handler_, onWebTransportUniStream(_, _))
      .WillOnce(SaveArg<1>(&readHandle));

  auto implHandle = txn_->onWebTransportUniStream(0);
  EXPECT_NE(readHandle, nullptr);

  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);

  EXPECT_FALSE(wtImpl->isSessionTerminated());
  EXPECT_FALSE(wtImpl->getSessionCloseError().has_value());

  EXPECT_CALL(transport_, readWebTransportData(0, 65535))
      .WillOnce(Invoke([](auto, auto) {
        return std::make_pair(makeBuf(2 * kDefaultWTReceiveWindow), false);
      }));
  EXPECT_CALL(
      transport_,
      stopReadingWebTransportIngress(0, makeOpt(WebTransport::kSessionGone)))
      .WillOnce(Return(folly::unit));

  // This tests both paths:
  // 1. dataAvailable returns SESSION_CLOSED when flow control is exceeded
  // 2. readAvailable calls terminateSession when dataAvailable returns
  // SESSION_CLOSED
  implHandle->readAvailable(0);

  EXPECT_TRUE(wtImpl->isSessionTerminated());
  EXPECT_TRUE(wtImpl->getSessionCloseError().has_value());
  EXPECT_EQ(wtImpl->getSessionCloseError().value(),
            WebTransport::kInternalError);

  EXPECT_EQ(wt_->createUniStream().error(),
            WebTransport::ErrorCode::SESSION_TERMINATED);
  EXPECT_EQ(wt_->createBidiStream().error(),
            WebTransport::ErrorCode::SESSION_TERMINATED);
}

TEST_F(HTTPTransactionWebTransportTest, ClientUniStreamRst) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);

  EXPECT_CALL(transport_, newWebTransportUniStream()).WillOnce(Return(2));
  auto writeHandle = wt_->createUniStream();
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/1,
      /*targetConcurrentStreams=*/4);
  EXPECT_FALSE(writeHandle.hasError());
  EXPECT_TRUE(wtImpl->shouldGrantStreamCredit(false));

  // For unidirectional egress streams (created by the client), we should NOT
  // send MaxStreams when they close, as that's controlled by the peer.
  // Only bidirectional streams and ingress unidirectional streams should
  // trigger MaxStreams updates.
  EXPECT_CALL(transport_, resetWebTransportEgress(2, _));
  writeHandle.value()->resetStream(WebTransport::kInternalError);
}

TEST_F(HTTPTransactionWebTransportTest, UniStreamCredit) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setUniStreamFlowControl(
      /*maxStreamId=*/1,
      /*targetConcurrentStreams=*/4);
  EXPECT_TRUE(wtImpl->shouldGrantStreamCredit(false));

  // Create a peer-initiated ingress unidirectional stream
  WebTransport::StreamReadHandle* readHandle{nullptr};
  EXPECT_CALL(handler_, onWebTransportUniStream(_, _))
      .WillOnce(SaveArg<1>(&readHandle));

  // This simulates the peer opening a unidirectional stream to us (ingress)
  txn_->onWebTransportUniStream(2);
  EXPECT_NE(readHandle, nullptr);

  // When we close this ingress stream, we should send MaxStreams
  // The new maxStreamID should be: 1 + (4 / 2) = 3
  EXPECT_CALL(transport_, sendWTMaxStreams(3, false))
      .WillOnce(Return(folly::unit));
  EXPECT_CALL(transport_, stopReadingWebTransportIngress(2, _))
      .WillRepeatedly(Return(folly::unit));

  auto implHandle =
      dynamic_cast<WebTransportImpl::StreamReadHandle*>(readHandle);
  ASSERT_NE(implHandle, nullptr);

  // Simulate the application reading and abandoning the stream due to an error
  // This triggers deliverReadError -> closeIngressStream ->
  // maybeGrantStreamCredit -> sendWTMaxStreams
  implHandle->deliverReadError(
      WebTransport::Exception(WebTransport::kInternalError, "test error"));
  auto fut = readHandle->readStreamData()
                 .via(&eventBase_)
                 .thenTry([](auto streamData) {
                   EXPECT_TRUE(streamData.hasException());
                 });

  eventBase_.loopOnce();
  EXPECT_TRUE(fut.isReady());
}

TEST_F(HTTPTransactionWebTransportTest, BidiStreamCredit) {
  auto wtImpl = dynamic_cast<WebTransportImpl*>(wt_);
  ASSERT_NE(wtImpl, nullptr);
  wtImpl->setBidiStreamFlowControl(
      /*maxStreamId=*/5,
      /*targetConcurrentStreams=*/12);
  EXPECT_TRUE(wtImpl->shouldGrantStreamCredit(true));

  // Create a peer-initiated bidirectional stream
  WebTransport::BidiStreamHandle bidiHandle{};
  EXPECT_CALL(handler_, onWebTransportBidiStream(_, _))
      .WillOnce(SaveArg<1>(&bidiHandle));

  // This simulates the peer opening a bidirectional stream to us (ingress)
  txn_->onWebTransportBidiStream(1);
  EXPECT_NE(bidiHandle.readHandle, nullptr);
  EXPECT_NE(bidiHandle.writeHandle, nullptr);

  // For bidi streams, we need to close BOTH sides.
  // Close the write side. This should NOT trigger MaxStreams yet
  // (we need both sides closed for peer-initiated bidi streams)
  EXPECT_CALL(transport_, resetWebTransportEgress(1, _));
  bidiHandle.writeHandle->resetStream(WebTransport::kInternalError);

  // Close the read side. This SHOULD trigger MaxStreams
  // because both sides of the peer-initiated bidi stream are now closed
  EXPECT_CALL(transport_, stopReadingWebTransportIngress(1, _))
      .WillRepeatedly(Return(folly::unit));
  // New maxStreamID should be: 5 + (12 / 2) = 11
  EXPECT_CALL(transport_, sendWTMaxStreams(11, true))
      .WillOnce(Return(folly::unit));

  auto implHandle =
      dynamic_cast<WebTransportImpl::StreamReadHandle*>(bidiHandle.readHandle);
  ASSERT_NE(implHandle, nullptr);

  // Simulate the application reading and abandoning the stream due to an error
  // This triggers deliverReadError -> closeIngressStream ->
  // maybeGrantStreamCredit -> sendWTMaxStreams (because the egress side was
  // already closed above)
  implHandle->deliverReadError(
      WebTransport::Exception(WebTransport::kInternalError, "test error"));
  auto fut = bidiHandle.readHandle->readStreamData()
                 .via(&eventBase_)
                 .thenTry([](auto streamData) {
                   EXPECT_TRUE(streamData.hasException());
                 });

  eventBase_.loopOnce();
  EXPECT_TRUE(fut.isReady());
}

} // namespace proxygen::test
