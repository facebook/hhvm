/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/session/test/MockQuicSocketDriver.h>
#include <proxygen/lib/http/webtransport/QuicWtSession.h>
#include <proxygen/lib/http/webtransport/test/Mocks.h>
#include <quic/state/StateData.h>

using namespace proxygen;
using namespace proxygen::test;
using namespace testing;
using quic::MockQuicSocketDriver;

namespace {
constexpr uint32_t WT_ERROR_1 = 19;
constexpr uint32_t WT_ERROR_2 = 77;
constexpr uint64_t kMaxWtIngressBuf = 65'535;

class MockConnectionState : public quic::QuicConnectionStateBase {
 public:
  explicit MockConnectionState(quic::QuicNodeType type)
      : quic::QuicConnectionStateBase(type) {
  }
};

class MockDeliveryCallback : public WebTransport::ByteEventCallback {
 public:
  MOCK_METHOD(void, onByteEvent, (quic::StreamId, uint64_t), (noexcept));

  MOCK_METHOD(void,
              onByteEventCanceled,
              (quic::StreamId, uint64_t),
              (noexcept));
};
} // namespace

class QuicWtSessionTest : public Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(*socketDriver_.getSocket(), getState())
        .WillRepeatedly(Return(&mockState_));

    auto handler = std::make_unique<StrictMock<MockWebTransportHandler>>();
    handler_ = handler.get();
    session_ = std::make_unique<QuicWtSession>(socketDriver_.getSocket(),
                                               std::move(handler));
    socketDriver_.setMaxUniStreams(100);
  }

  void TearDown() override {
    if (session_) {
      webTransport()->closeSession();
    }
  }

  WebTransport* webTransport() {
    return session_.get();
  }

  folly::EventBase eventBase_;
  MockConnectionState mockState_{quic::QuicNodeType::Server};
  MockQuicSocketDriver socketDriver_{
      &eventBase_,
      nullptr,
      nullptr,
      MockQuicSocketDriver::TransportEnum::SERVER,
      "alpn1"};
  StrictMock<MockWebTransportHandler>* handler_{nullptr};
  std::unique_ptr<QuicWtSession> session_;
};

TEST_F(QuicWtSessionTest, PeerUniStream) {
  EXPECT_CALL(*handler_, onNewUniStream(_))
      .WillOnce([this](WebTransport::StreamReadHandle* readHandle) {
        readHandle->awaitNextRead(
            &eventBase_,
            [](WebTransport::StreamReadHandle*,
               uint64_t,
               folly::Try<WebTransport::StreamData> data) {
              EXPECT_FALSE(data.hasException());
              EXPECT_EQ(data->data->computeChainDataLength(), 5);
              EXPECT_TRUE(data->fin);
            });
      });
  socketDriver_.addReadEvent(2, folly::IOBuf::copyBuffer("hello"), true);
  eventBase_.loop();
}

TEST_F(QuicWtSessionTest, PeerBidiStream) {
  EXPECT_CALL(*handler_, onNewBidiStream(_))
      .WillOnce([this](WebTransport::BidiStreamHandle bidiHandle) {
        bidiHandle.writeHandle->writeStreamData(
            folly::IOBuf::copyBuffer("hello"), true, nullptr);
        bidiHandle.readHandle->awaitNextRead(
            &eventBase_,
            [](WebTransport::StreamReadHandle*,
               uint64_t,
               folly::Try<WebTransport::StreamData> data) {
              EXPECT_FALSE(data.hasException());
              EXPECT_TRUE(data->fin);
            });
      });
  socketDriver_.addReadEvent(0, nullptr, true);
  eventBase_.loop();
  EXPECT_EQ(socketDriver_.streams_[0].writeBuf.chainLength(), 5);
}

TEST_F(QuicWtSessionTest, NewUniStream) {
  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  auto id = handle.value()->getID();
  handle.value()->resetStream(WT_ERROR_1);
  eventBase_.loop();
  EXPECT_EQ(socketDriver_.streams_[id].error, WT_ERROR_1);
}

TEST_F(QuicWtSessionTest, NewBidiStream) {
  auto handle = webTransport()->createBidiStream();
  EXPECT_TRUE(handle.hasValue());
  auto id = handle->readHandle->getID();
  handle->readHandle->stopSending(WT_ERROR_1);
  EXPECT_EQ(socketDriver_.streams_[id].error, WT_ERROR_1);
  handle->writeHandle->resetStream(WT_ERROR_2);
  EXPECT_EQ(socketDriver_.streams_[id].error, WT_ERROR_2);
}

TEST_F(QuicWtSessionTest, AwaitUniStreamCredit) {
  // credit already available -> immediate future
  auto fastFut = webTransport()->awaitUniStreamCredit();
  EXPECT_TRUE(fastFut.isReady());

  // set max uni streams to 0 to exhaust stream credit
  socketDriver_.setMaxUniStreams(0);

  // try creating a uni stream when no credit is available, should fail
  auto res = webTransport()->createUniStream();
  EXPECT_FALSE(res.hasValue());
  EXPECT_EQ(res.error(), WebTransport::ErrorCode::STREAM_CREATION_ERROR);
  auto fut = webTransport()->awaitUniStreamCredit();
  EXPECT_FALSE(fut.isReady());

  // calling with 0 streams available should not fulfill the promise
  socketDriver_.getSocket()->connCb_->onUnidirectionalStreamsAvailable(0);
  EXPECT_FALSE(fut.isReady());

  // set max uni streams to make credit available
  socketDriver_.setMaxUniStreams(1);
  EXPECT_TRUE(fut.isReady());
  std::move(fut).getTry();

  // now that we have credit, creating a uni stream should succeed
  auto res2 = webTransport()->createUniStream();
  EXPECT_TRUE(res2.hasValue());
  EXPECT_NE(res2.value(), nullptr);
}

TEST_F(QuicWtSessionTest, AwaitBidiStreamCredit) {
  // credit already available -> immediate future
  auto fastFut = webTransport()->awaitBidiStreamCredit();
  EXPECT_TRUE(fastFut.isReady());

  // set max bidi streams to 0 to exhaust stream credit
  socketDriver_.setMaxBidiStreams(0);

  // try creating a bidi stream when no credit is available, should fail
  auto res = webTransport()->createBidiStream();
  EXPECT_FALSE(res.hasValue());
  EXPECT_EQ(res.error(), WebTransport::ErrorCode::STREAM_CREATION_ERROR);
  auto fut = webTransport()->awaitBidiStreamCredit();
  EXPECT_FALSE(fut.isReady());

  // calling with 0 streams available should not fulfill the promise
  socketDriver_.getSocket()->connCb_->onBidirectionalStreamsAvailable(0);
  EXPECT_FALSE(fut.isReady());

  // set max bidi streams to make credit available
  socketDriver_.setMaxBidiStreams(1);
  EXPECT_TRUE(fut.isReady());
  std::move(fut).getTry();

  // now that we have credit, creating a bidi stream should succeed
  auto res2 = webTransport()->createBidiStream();
  EXPECT_TRUE(res2.hasValue());
  EXPECT_NE(res2->readHandle, nullptr);
  EXPECT_NE(res2->writeHandle, nullptr);
}

TEST_F(QuicWtSessionTest, WriteStreamData) {
  // id=2 is client-initiated uni, so it's not egress for the server
  constexpr uint64_t kClientInitiatedUniId = 2;

  auto result = webTransport()->writeStreamData(
      kClientInitiatedUniId, folly::IOBuf::copyBuffer("test"), true, nullptr);
  EXPECT_FALSE(result.hasValue());
  EXPECT_EQ(result.error(), WebTransport::ErrorCode::INVALID_STREAM_ID);

  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  auto id = handle.value()->getID();

  auto data = folly::IOBuf::copyBuffer("hello world");
  auto res =
      webTransport()->writeStreamData(id, std::move(data), true, nullptr);
  EXPECT_TRUE(res.hasValue());

  eventBase_.loop();
  EXPECT_EQ(socketDriver_.streams_[id].writeBuf.chainLength(), 11);
}

TEST_F(QuicWtSessionTest, ReadStreamData) {
  // id=3 is server-initiated uni, so it's not ingress for the server
  constexpr uint64_t kServerInitiatedUniId = 3;

  auto res = webTransport()->readStreamData(kServerInitiatedUniId);
  EXPECT_FALSE(res.hasValue());
  EXPECT_EQ(res.error(), WebTransport::ErrorCode::INVALID_STREAM_ID);

  // simulate peer opening a bidi stream
  WebTransport::StreamReadHandle* readHandle = nullptr;
  EXPECT_CALL(*handler_, onNewBidiStream(_))
      .WillOnce([&](WebTransport::BidiStreamHandle handle) {
        readHandle = handle.readHandle;
      });
  socketDriver_.addReadEvent(0, nullptr, false);
  eventBase_.loopOnce();
  ASSERT_NE(readHandle, nullptr);

  auto id = readHandle->getID();
  auto readRes = webTransport()->readStreamData(id);
  EXPECT_TRUE(readRes.hasValue());

  auto data = folly::IOBuf::copyBuffer("test data");
  socketDriver_.addReadEvent(id, std::move(data), true);
  eventBase_.loopOnce();

  EXPECT_TRUE(readRes.value().isReady());
  auto streamData = std::move(readRes.value()).getTry();
  EXPECT_TRUE(streamData.hasValue());
  EXPECT_EQ(streamData.value().data->computeChainDataLength(), 9);
  EXPECT_TRUE(streamData.value().fin);
}

TEST_F(QuicWtSessionTest, Datagram) {
  // send multiple datagrams
  webTransport()->sendDatagram(folly::IOBuf::copyBuffer("out1"));
  webTransport()->sendDatagram(folly::IOBuf::copyBuffer("out2"));
  EXPECT_EQ(socketDriver_.outDatagrams_.size(), 2);

  // receive multiple datagrams
  socketDriver_.addDatagram(folly::IOBuf::copyBuffer("in1"));
  socketDriver_.addDatagram(folly::IOBuf::copyBuffer("in2"));

  std::vector<std::string> received;
  EXPECT_CALL(*handler_, onDatagram(_))
      .Times(2)
      .WillRepeatedly([&](std::unique_ptr<folly::IOBuf> datagram) {
        received.push_back(datagram->toString());
      });
  socketDriver_.addDatagramsAvailableReadEvent();
  eventBase_.loop();

  ASSERT_EQ(received.size(), 2);
  EXPECT_EQ(received[0], "in1");
  EXPECT_EQ(received[1], "in2");

  // sending a datagram that exceeds size limit should fail
  auto maxSize = socketDriver_.getSocket()->getDatagramSizeLimit();
  auto largeDatagram = folly::IOBuf::create(maxSize + 100);
  largeDatagram->append(maxSize + 100);
  auto res = webTransport()->sendDatagram(std::move(largeDatagram));
  EXPECT_FALSE(res.hasValue());
  EXPECT_EQ(res.error(), WebTransport::ErrorCode::GENERIC_ERROR);

  // simulate datagram read failure: close only the connection's write state
  // so readDatagramBufs() returns CONNECTION_CLOSED, while read state stays
  // OPEN to allow the datagram available event to fire.
  // closeSession(kInternalError) will be called, which closes the socket.
  socketDriver_.streams_[quic::kConnectionStreamId].writeState =
      quic::MockQuicSocketDriver::StateEnum::CLOSED;
  socketDriver_.addDatagramsAvailableReadEvent();
  eventBase_.loop();
  session_.reset();
}

TEST_F(QuicWtSessionTest, StopSending) {
  // id=3 is server-initiated uni, so it's not ingress for the server
  constexpr uint64_t kServerInitiatedUniId = 3;

  auto res = webTransport()->stopSending(kServerInitiatedUniId, WT_ERROR_1);
  EXPECT_FALSE(res.hasValue());
  EXPECT_EQ(res.error(), WebTransport::ErrorCode::INVALID_STREAM_ID);

  // simulate peer opening a bidi stream
  WebTransport::StreamReadHandle* readHandle = nullptr;

  EXPECT_CALL(*handler_, onNewBidiStream(_))
      .WillOnce([&](WebTransport::BidiStreamHandle handle) {
        readHandle = handle.readHandle;
      });
  socketDriver_.addReadEvent(0, nullptr, false);
  eventBase_.loopOnce();
  ASSERT_NE(readHandle, nullptr);

  // we call stop sending
  auto id = readHandle->getID();
  auto res2 = webTransport()->stopSending(id, WT_ERROR_1);
  EXPECT_TRUE(res2.hasValue());
  EXPECT_EQ(socketDriver_.streams_[id].error, WT_ERROR_1);

  // receive stop sending
  auto handle = webTransport()->createBidiStream();
  EXPECT_TRUE(handle.hasValue());
  auto id2 = handle->writeHandle->getID();
  auto* writeHandle = handle->writeHandle;
  EXPECT_EQ(writeHandle->exception(), nullptr);

  // trigger stop sending to cancel the write handle and set exception
  socketDriver_.addStopSending(id2, WT_ERROR_1);
  eventBase_.loopOnce();

  EXPECT_NE(writeHandle->exception(), nullptr);
  EXPECT_EQ(writeHandle->exception()->error, WT_ERROR_1);
}

TEST_F(QuicWtSessionTest, ResetStream) {
  // id=2 is client-initiated uni, so it's not egress for the server
  constexpr uint64_t kClientInitiatedUniId = 2;

  auto res = webTransport()->resetStream(kClientInitiatedUniId, WT_ERROR_1);
  EXPECT_FALSE(res.hasValue());
  EXPECT_EQ(res.error(), WebTransport::ErrorCode::INVALID_STREAM_ID);

  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  auto id = handle.value()->getID();

  auto res2 = webTransport()->resetStream(id, WT_ERROR_1);
  EXPECT_TRUE(res2.hasValue());
  eventBase_.loop();
  EXPECT_EQ(socketDriver_.streams_[id].error, WT_ERROR_1);
}

TEST_F(QuicWtSessionTest, SetPriority) {
  auto handle = webTransport()->createUniStream();
  ASSERT_TRUE(handle.hasValue());
  auto id = handle.value()->getID();

  quic::HTTPPriorityQueue::Priority priority(/*u=*/3,
                                             /*i=*/true,
                                             /*o=*/100);
  auto result = webTransport()->setPriority(id, priority);
  EXPECT_TRUE(result.hasValue());

  auto pri = socketDriver_.getSocket()->getStreamPriority(id);
  ASSERT_TRUE(pri.has_value());
  quic::HTTPPriorityQueue::Priority httpPriority(*pri);
  EXPECT_EQ(httpPriority->urgency, 3);
  EXPECT_EQ(httpPriority->incremental, true);
}

TEST_F(QuicWtSessionTest, AwaitWritable) {
  // Note: This test verifies basic functionality but doesn't test the blocking
  // scenario where awaitWritable returns a "not ready" future.
  // MockQuicSocketDriver auto-flushes writes immediately, making it difficult
  // to test the blocked state. The blocking behavior is tested at the
  // WtStreamManager level (see WtStreamManagerTest's AwaitWritableTest).

  // id=2 is client-initiated uni, so it's not egress for the server
  constexpr uint64_t kClientInitiatedUniId = 2;

  auto res = webTransport()->awaitWritable(kClientInitiatedUniId);
  EXPECT_FALSE(res.hasValue());
  EXPECT_EQ(res.error(), WebTransport::ErrorCode::INVALID_STREAM_ID);

  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  auto streamId = handle.value()->getID();

  auto res2 = webTransport()->awaitWritable(streamId);
  EXPECT_TRUE(res2.hasValue());

  // verify the future completes successfully with available bytes
  EXPECT_TRUE(res2.value().isReady());
  auto bytes = std::move(res2).value().getTry();
  EXPECT_TRUE(bytes.hasValue());
  EXPECT_GT(bytes.value(), 0);
}

TEST_F(QuicWtSessionTest, ConnectionError) {
  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  EXPECT_CALL(*handler_, onSessionEnd(_)).WillOnce([](const auto& err) {
    EXPECT_EQ(*err, WT_ERROR_1);
  });
  socketDriver_.deliverConnectionError(
      quic::QuicError(quic::ApplicationErrorCode(WT_ERROR_1), "peer close"));
  eventBase_.loop();
}

TEST_F(QuicWtSessionTest, ConnectionEnd) {
  EXPECT_CALL(*handler_, onSessionEnd(_)).WillOnce([](const auto& err) {
    EXPECT_FALSE(err.has_value());
  });
  socketDriver_.deliverConnectionError(
      quic::QuicError(quic::LocalErrorCode::NO_ERROR, "clean close"));
  eventBase_.loop();
}

TEST_F(QuicWtSessionTest, DeliveryCallback) {
  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  auto mockCallback = std::make_unique<StrictMock<MockDeliveryCallback>>();
  folly::StringPiece data = "test data";

  uint64_t expectedId = handle.value()->getID();
  uint32_t expectedOffset = data.size();
  EXPECT_CALL(*mockCallback, onByteEvent(expectedId, expectedOffset)).Times(1);

  handle.value()->writeStreamData(
      folly::IOBuf::copyBuffer(data), true, mockCallback.get());
  // The MockQuicSocketDriver automatically simulates the delivery of data
  // written to the QUIC socket.
  eventBase_.loop();
}

TEST_F(QuicWtSessionTest, CloseTransportCancelsReadTokens) {
  WebTransport::StreamReadHandle* readHandle1 = nullptr;
  WebTransport::StreamReadHandle* readHandle2 = nullptr;
  folly::CancellationToken token1;
  folly::CancellationToken token2;

  // create two read handles from incoming uni streams
  EXPECT_CALL(*handler_, onNewUniStream(_))
      .WillOnce([&](WebTransport::StreamReadHandle* handle) {
        readHandle1 = handle;
        token1 = handle->getCancelToken();
      })
      .WillOnce([&](WebTransport::StreamReadHandle* handle) {
        readHandle2 = handle;
        token2 = handle->getCancelToken();
      });

  // trigger two incoming uni streams
  socketDriver_.addReadEvent(2, nullptr, false);
  socketDriver_.addReadEvent(6, nullptr, false);
  eventBase_.loopOnce();

  ASSERT_NE(readHandle1, nullptr);
  ASSERT_NE(readHandle2, nullptr);
  EXPECT_FALSE(token1.isCancellationRequested());
  EXPECT_FALSE(token2.isCancellationRequested());

  // invoke read on the first handle (will be pending since no data)
  auto fut = readHandle1->readStreamData();

  // close the transport by delivering a connection error
  EXPECT_CALL(*handler_, onSessionEnd(_));
  socketDriver_.deliverConnectionError(quic::QuicError(
      quic::ApplicationErrorCode(WT_ERROR_1), "connection close"));
  eventBase_.loop();

  // cancellation tokens should now all be cancelled
  EXPECT_TRUE(token1.isCancellationRequested());
  EXPECT_TRUE(token2.isCancellationRequested());
  EXPECT_TRUE(fut.isReady());
  auto readResult = std::move(fut).getTry();
  EXPECT_TRUE(readResult.hasException());
  auto* ex = readResult.tryGetExceptionObject<WebTransport::Exception>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(ex->error, WT_ERROR_1);
}

TEST_F(QuicWtSessionTest, DestructorTerminatesOpenStreams) {
  WebTransport::StreamReadHandle* uniReadHandle = nullptr;
  WebTransport::StreamReadHandle* bidiReadHandle = nullptr;
  WebTransport::StreamWriteHandle* bidiWriteHandle = nullptr;
  WebTransport::StreamWriteHandle* uniWriteHandle = nullptr;

  // create an incoming uni stream
  EXPECT_CALL(*handler_, onNewUniStream(_))
      .WillOnce([&](WebTransport::StreamReadHandle* handle) {
        uniReadHandle = handle;
      });
  socketDriver_.addReadEvent(2, nullptr, false);
  eventBase_.loopOnce();
  ASSERT_NE(uniReadHandle, nullptr);

  // create an incoming bidi stream
  EXPECT_CALL(*handler_, onNewBidiStream(_))
      .WillOnce([&](WebTransport::BidiStreamHandle handle) {
        bidiReadHandle = handle.readHandle;
        bidiWriteHandle = handle.writeHandle;
      });
  socketDriver_.addReadEvent(0, nullptr, false);
  eventBase_.loopOnce();
  ASSERT_NE(bidiReadHandle, nullptr);
  ASSERT_NE(bidiWriteHandle, nullptr);

  // create an outgoing uni stream
  auto uniStreamRes = webTransport()->createUniStream();
  ASSERT_TRUE(uniStreamRes.hasValue());
  uniWriteHandle = uniStreamRes.value();

  // start pending read operations
  auto uniReadFuture = uniReadHandle->readStreamData();
  auto bidiReadFuture = bidiReadHandle->readStreamData();

  // store cancellation tokens before destroying transport
  auto uniReadToken = uniReadHandle->getCancelToken();
  auto bidiReadToken = bidiReadHandle->getCancelToken();
  auto bidiWriteToken = bidiWriteHandle->getCancelToken();
  auto uniWriteToken = uniWriteHandle->getCancelToken();

  // verify cancellation tokens are not yet cancelled
  EXPECT_FALSE(uniReadToken.isCancellationRequested());
  EXPECT_FALSE(bidiReadToken.isCancellationRequested());
  EXPECT_FALSE(bidiWriteToken.isCancellationRequested());
  EXPECT_FALSE(uniWriteToken.isCancellationRequested());

  session_.reset();
  eventBase_.loop();

  // cancellation tokens should now all be cancelled
  EXPECT_TRUE(uniReadToken.isCancellationRequested());
  EXPECT_TRUE(bidiReadToken.isCancellationRequested());
  EXPECT_TRUE(bidiWriteToken.isCancellationRequested());
  EXPECT_TRUE(uniWriteToken.isCancellationRequested());

  EXPECT_TRUE(uniReadFuture.isReady());
  auto uniReadResult = std::move(uniReadFuture).getTry();
  EXPECT_TRUE(uniReadResult.hasException());
  auto* uniEx = uniReadResult.tryGetExceptionObject<WebTransport::Exception>();
  ASSERT_NE(uniEx, nullptr);
  EXPECT_EQ(uniEx->error, 0);

  EXPECT_TRUE(bidiReadFuture.isReady());
  auto bidiReadResult = std::move(bidiReadFuture).getTry();
  EXPECT_TRUE(bidiReadResult.hasException());
  auto* bidiEx =
      bidiReadResult.tryGetExceptionObject<WebTransport::Exception>();
  ASSERT_NE(bidiEx, nullptr);
  EXPECT_EQ(bidiEx->error, 0);
}

TEST_F(QuicWtSessionTest, ReadError) {
  WebTransport::StreamReadHandle* readHandle = nullptr;
  EXPECT_CALL(*handler_, onNewUniStream(_))
      .WillOnce(
          [&](WebTransport::StreamReadHandle* handle) { readHandle = handle; });
  socketDriver_.addReadEvent(2, nullptr, false);
  eventBase_.loopOnce();
  ASSERT_NE(readHandle, nullptr);

  // set up a pending read on the stream
  auto readFut = readHandle->readStreamData();

  // deliver a stream-level read error
  socketDriver_.addReadError(
      2, quic::QuicErrorCode(quic::ApplicationErrorCode(WT_ERROR_2)));
  eventBase_.loopOnce();

  // the read future should resolve with the error
  EXPECT_TRUE(readFut.isReady());
  auto result = std::move(readFut).getTry();
  EXPECT_TRUE(result.hasException());
  auto* ex = result.tryGetExceptionObject<WebTransport::Exception>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(ex->error, WT_ERROR_2);
}

TEST_F(QuicWtSessionTest, ConnectionEndWithError) {
  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  EXPECT_CALL(*handler_, onSessionEnd(_)).WillOnce([](const auto& err) {
    EXPECT_TRUE(err.has_value());
    EXPECT_EQ(*err, WT_ERROR_1);
  });
  // directly invoke the onConnectionEnd(QuicError) overload via the
  // connection callback, rather than going through deliverConnectionError
  socketDriver_.getSocket()->connCb_->onConnectionEnd(
      quic::QuicError(quic::ApplicationErrorCode(WT_ERROR_1), "peer close"));
  eventBase_.loop();
  session_.reset();
}

TEST_F(QuicWtSessionTest, ReadAvailableReadFails) {
  WebTransport::StreamReadHandle* readHandle = nullptr;
  EXPECT_CALL(*handler_, onNewBidiStream(_))
      .WillOnce([&](WebTransport::BidiStreamHandle handle) {
        readHandle = handle.readHandle;
      });
  socketDriver_.addReadEvent(0, nullptr, false);
  eventBase_.loopOnce();
  ASSERT_NE(readHandle, nullptr);

  auto id = readHandle->getID();
  socketDriver_.setReadError(id);
  // trigger readAvailable: read() will fail, session should continue
  socketDriver_.addReadEvent(id, folly::IOBuf::copyBuffer("data"), false);
  eventBase_.loopOnce();
  auto uniHandle = webTransport()->createUniStream();
  EXPECT_TRUE(uniHandle.hasValue());
}

TEST_F(QuicWtSessionTest, WriteFlowControlBlocked) {
  auto handle = webTransport()->createUniStream();
  ASSERT_TRUE(handle.hasValue());
  auto id = handle.value()->getID();

  // block writes by setting stream flow control window to 0
  socketDriver_.setStreamFlowControlWindow(id, 0);

  // write data: eventsAvailable() will see maxData==0 and call
  // notifyPendingWriteOnStream instead of writing
  handle.value()->writeStreamData(
      folly::IOBuf::copyBuffer("blocked data"), false, nullptr);
  eventBase_.loopOnce();

  // data should NOT have been written to the socket
  EXPECT_EQ(socketDriver_.streams_[id].writeBuf.chainLength(), 0);

  // unblock by restoring flow control window: this triggers
  // onStreamWriteReady() which re-inserts into priority queue and calls
  // eventsAvailable()
  socketDriver_.setStreamFlowControlWindow(id, 65536);
  eventBase_.loop();

  // data should now be written
  EXPECT_EQ(socketDriver_.streams_[id].writeBuf.chainLength(), 12);
}

TEST_F(QuicWtSessionTest, WriteChainFails) {
  // use a bidi stream so the read handle keeps the stream entry alive
  // after resetStream (uni stream would be freed immediately)
  WebTransport::BidiStreamHandle bidiHandle{nullptr, nullptr};
  EXPECT_CALL(*handler_, onNewBidiStream(_))
      .WillOnce(
          [&](WebTransport::BidiStreamHandle handle) { bidiHandle = handle; });
  socketDriver_.addReadEvent(0, nullptr, false);
  eventBase_.loopOnce();
  ASSERT_NE(bidiHandle.writeHandle, nullptr);
  auto id = bidiHandle.writeHandle->getID();

  // block writes via flow control so data queues in WtStreamManager
  socketDriver_.setStreamFlowControlWindow(id, 0);

  bidiHandle.writeHandle->writeStreamData(
      folly::IOBuf::copyBuffer("will fail"), true, nullptr);
  eventBase_.loopOnce();

  // data is queued but not written (flow-control blocked)
  EXPECT_EQ(socketDriver_.streams_[id].writeBuf.chainLength(), 0);

  // poison the stream write state, then resume flow control and fire callback
  auto& stream = socketDriver_.streams_[id];
  stream.flowControlWindow = 65536;
  stream.writeState = quic::MockQuicSocketDriver::StateEnum::ERROR;

  // directly fire onStreamWriteReady to trigger
  // eventsAvailable() -> writeChain fails -> resetStream(kInternalError)
  auto pendingCb = stream.pendingWriteCb;
  stream.pendingWriteCb.stream = nullptr;
  if (pendingCb.stream) {
    pendingCb.stream->onStreamWriteReady(id, 65536);
  }
  eventBase_.loop();

  // the stream should have been reset with an error
  EXPECT_EQ(socketDriver_.streams_[id].writeState,
            quic::MockQuicSocketDriver::StateEnum::ERROR);
}

TEST_F(QuicWtSessionTest, StreamWriteError) {
  auto handle = webTransport()->createUniStream();
  ASSERT_TRUE(handle.hasValue());
  auto id = handle.value()->getID();

  // block writes by setting stream flow control window to 0
  socketDriver_.setStreamFlowControlWindow(id, 0);

  // write data to trigger the flow-control-blocked path
  // eventsAvailable() sees maxData==0, calls notifyPendingWriteOnStream
  handle.value()->writeStreamData(
      folly::IOBuf::copyBuffer("blocked"), false, nullptr);
  eventBase_.loopOnce();

  // the stream now has a pending write callback registered
  // deliver a write error to trigger onStreamWriteError()
  socketDriver_.deliverWriteError(
      id,
      socketDriver_.streams_[id],
      quic::QuicErrorCode(quic::ApplicationErrorCode(WT_ERROR_1)));
  eventBase_.loop();

  // the stream should have been reset with kInternalError
  EXPECT_EQ(socketDriver_.streams_[id].writeState,
            quic::MockQuicSocketDriver::StateEnum::ERROR);
}

TEST_F(QuicWtSessionTest, IngressBackpressure) {
  // simulate peer opening two bidi streams
  WebTransport::StreamReadHandle* readHandle1 = nullptr;
  WebTransport::StreamReadHandle* readHandle2 = nullptr;

  EXPECT_CALL(*handler_, onNewBidiStream(_))
      .WillOnce([&](WebTransport::BidiStreamHandle handle) {
        readHandle1 = handle.readHandle;
      })
      .WillOnce([&](WebTransport::BidiStreamHandle handle) {
        readHandle2 = handle.readHandle;
      });

  socketDriver_.addReadEvent(0, nullptr, false);
  socketDriver_.addReadEvent(4, nullptr, false);
  eventBase_.loopOnce();

  ASSERT_NE(readHandle1, nullptr);
  ASSERT_NE(readHandle2, nullptr);

  auto streamId1 = readHandle1->getID();
  auto streamId2 = readHandle2->getID();

  // fill id=1 buffer to trigger pause when bufferedBytes() >=
  // kMaxWTIngressBuf
  auto buf1 = folly::IOBuf::create(kMaxWtIngressBuf);
  buf1->append(kMaxWtIngressBuf);
  socketDriver_.addReadEvent(streamId1, std::move(buf1), false);
  eventBase_.loop();
  EXPECT_EQ(socketDriver_.streams_[streamId1].readState,
            MockQuicSocketDriver::PAUSED);
  EXPECT_EQ(socketDriver_.streams_[streamId2].readState,
            MockQuicSocketDriver::OPEN); // id=2 should be independent of
                                         // id=1

  // fill id=2 buffer to trigger pause
  auto buf2 = folly::IOBuf::create(kMaxWtIngressBuf);
  buf2->append(kMaxWtIngressBuf);
  socketDriver_.addReadEvent(streamId2, std::move(buf2), false);
  eventBase_.loop();
  EXPECT_EQ(socketDriver_.streams_[streamId1].readState,
            MockQuicSocketDriver::PAUSED);
  EXPECT_EQ(socketDriver_.streams_[streamId2].readState,
            MockQuicSocketDriver::PAUSED);

  // resume id=1 by reading
  auto fut1 = readHandle1->readStreamData();
  EXPECT_TRUE(fut1.isReady());
  std::move(fut1).via(&eventBase_).thenValue([](WebTransport::StreamData) {});
  eventBase_.loop();

  // id=1 resumed, id=2 still paused
  EXPECT_EQ(socketDriver_.streams_[streamId1].readState,
            MockQuicSocketDriver::OPEN);
  EXPECT_EQ(socketDriver_.streams_[streamId2].readState,
            MockQuicSocketDriver::PAUSED);
}
