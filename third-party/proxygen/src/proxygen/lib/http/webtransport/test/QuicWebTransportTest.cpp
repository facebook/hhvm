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
#include <proxygen/lib/http/webtransport/QuicWebTransport.h>
#include <proxygen/lib/http/webtransport/test/Mocks.h>

using namespace proxygen;
using namespace proxygen::test;
using namespace testing;
using quic::MockQuicSocketDriver;

class MockDeliveryCallback : public WebTransport::DeliveryCallback {
 public:
  MOCK_METHOD(void, onDelivery, (uint64_t, uint32_t), (noexcept));

  MOCK_METHOD(void, onDeliveryCancelled, (uint64_t, uint32_t), (noexcept));
};

namespace {
const uint32_t WT_ERROR_1 = 19;
const uint32_t WT_ERROR_2 = 77;
} // namespace

class QuicWebTransportTest : public Test {
 protected:
  void SetUp() override {
    handler_ = std::make_unique<StrictMock<MockWebTransportHandler>>();
    transport_ = std::make_unique<QuicWebTransport>(socketDriver_.getSocket());
    transport_->setHandler(handler_.get());
    socketDriver_.setMaxUniStreams(100);
  }

  void TearDown() override {
    webTransport()->closeSession();
  }

  WebTransport* webTransport() {
    return transport_.get();
  }

  folly::EventBase eventBase_;
  MockQuicSocketDriver socketDriver_{
      &eventBase_,
      nullptr,
      nullptr,
      MockQuicSocketDriver::TransportEnum::SERVER,
      "alpn1"};
  std::unique_ptr<StrictMock<MockWebTransportHandler>> handler_;
  std::unique_ptr<QuicWebTransport> transport_;
};

TEST_F(QuicWebTransportTest, PeerUniStream) {
  EXPECT_CALL(*handler_, onNewUniStream(_))
      .WillOnce([this](WebTransport::StreamReadHandle* readHandle) {
        readHandle->awaitNextRead(
            &eventBase_,
            [](WebTransport::StreamReadHandle*,
               folly::Try<WebTransport::StreamData> data) {
              EXPECT_FALSE(data.hasException());
              EXPECT_EQ(data->data->computeChainDataLength(), 5);
              EXPECT_TRUE(data->fin);
            });
      });
  socketDriver_.addReadEvent(2, folly::IOBuf::copyBuffer("hello"), true);
  eventBase_.loop();
}

TEST_F(QuicWebTransportTest, PeerBidiStream) {
  EXPECT_CALL(*handler_, onNewBidiStream(_))
      .WillOnce([this](WebTransport::BidiStreamHandle bidiHandle) {
        bidiHandle.writeHandle->writeStreamData(
            folly::IOBuf::copyBuffer("hello"), true, nullptr);
        bidiHandle.readHandle->awaitNextRead(
            &eventBase_,
            [](WebTransport::StreamReadHandle*,
               folly::Try<WebTransport::StreamData> data) {
              EXPECT_FALSE(data.hasException());
              EXPECT_TRUE(data->fin);
            });
      });
  socketDriver_.addReadEvent(0, nullptr, true);
  eventBase_.loop();
  EXPECT_EQ(socketDriver_.streams_[0].writeBuf.chainLength(), 5);
}

TEST_F(QuicWebTransportTest, NewUniStream) {
  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  auto id = handle.value()->getID();
  handle.value()->resetStream(WT_ERROR_1);
  eventBase_.loop();
  EXPECT_EQ(socketDriver_.streams_[id].error, WT_ERROR_1);
}

TEST_F(QuicWebTransportTest, NewBidiStream) {
  auto handle = webTransport()->createBidiStream();
  EXPECT_TRUE(handle.hasValue());
  auto id = handle->readHandle->getID();
  handle->readHandle->stopSending(WT_ERROR_1);
  EXPECT_EQ(socketDriver_.streams_[id].error, WT_ERROR_1);
  handle->writeHandle->resetStream(WT_ERROR_2);
  EXPECT_EQ(socketDriver_.streams_[id].error, WT_ERROR_2);
}

TEST_F(QuicWebTransportTest, Datagram) {
  webTransport()->sendDatagram(folly::IOBuf::copyBuffer("hello"));
  EXPECT_EQ(socketDriver_.outDatagrams_.front().chainLength(), 5);
  socketDriver_.addDatagram(folly::IOBuf::copyBuffer("world!"));
  EXPECT_CALL(*handler_, onDatagram(_))
      .WillOnce([](std::unique_ptr<folly::IOBuf> datagram) {
        EXPECT_EQ(datagram->computeChainDataLength(), 6);
      });
  socketDriver_.addDatagramsAvailableReadEvent();
  eventBase_.loop();
}

TEST_F(QuicWebTransportTest, OnStopSending) {
  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  auto id = handle.value()->getID();
  socketDriver_.addStopSending(id, WT_ERROR_1);
  eventBase_.loopOnce();
  auto res = handle.value()->writeStreamData(nullptr, true, nullptr);
  EXPECT_TRUE(res.hasError());
  EXPECT_EQ(res.error(), WebTransport::ErrorCode::STOP_SENDING);
  EXPECT_EQ(*handle.value()->stopSendingErrorCode(), WT_ERROR_1);
}

TEST_F(QuicWebTransportTest, ConnectionError) {
  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  EXPECT_CALL(*handler_, onSessionEnd(_)).WillOnce([](const auto& err) {
    EXPECT_EQ(*err, WT_ERROR_1);
  });
  socketDriver_.deliverConnectionError(
      quic::QuicError(quic::ApplicationErrorCode(WT_ERROR_1), "peer close"));
  eventBase_.loop();
}

TEST_F(QuicWebTransportTest, SetPriority) {
  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  EXPECT_CALL(
      *socketDriver_.getSocket(),
      setStreamPriority(handle.value()->getID(), quic::Priority(1, false, 1)));
  handle.value()->setPriority(1, 1, false);
  handle.value()->writeStreamData(nullptr, true, nullptr);
  eventBase_.loop();
}

TEST_F(QuicWebTransportTest, WriteWithDeliveryCallback) {
  auto handle = webTransport()->createUniStream();
  EXPECT_TRUE(handle.hasValue());
  auto mockCallback = std::make_unique<StrictMock<MockDeliveryCallback>>();

  folly::StringPiece data = "test data";

  uint64_t expectedStreamId = handle.value()->getID();
  uint32_t expectedOffset = data.size();
  EXPECT_CALL(*mockCallback, onDelivery(expectedStreamId, expectedOffset))
      .Times(1);

  handle.value()->writeStreamData(
      folly::IOBuf::copyBuffer(data), true, mockCallback.get());
  // The MockQuicSocketDriver automatically simulates the delivery of data
  // written to the QUIC socket.
  eventBase_.loop();
}

// TODO:
//
// new streams with no handler
// receive connection end
// create bidi fails
// create uni fails
// writeChain fails
// write blocks
// reset fails
// pause/resume
// setReadCallback fails
// sendDatagram fails
// close with error
// await uni/bidi stream credit
