/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <proxygen/lib/http/session/HQByteEventTracker.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <quic/api/test/MockQuicSocket.h>
#include <quic/api/test/Mocks.h>

#include <chrono>

using namespace testing;
using namespace proxygen;

using QuicByteEvent = quic::QuicSocket::ByteEvent;
using QuicByteEventType = quic::QuicSocket::ByteEvent::Type;

class HQByteEventTrackerTest : public Test {
 public:
  void SetUp() override {
    socket_ = std::make_shared<quic::MockQuicSocket>(
        nullptr, &connectionSetupCallback_, &connectionCallback_);
    byteEventTracker_ =
        std::make_shared<HQByteEventTracker>(nullptr, socket_.get(), streamId_);
    txn_.setTransportCallback(&transportCallback_);
  }

  /**
   * Setup an EXPECT_CALL for QuicSocket::registerTxCallback.
   */
  std::unique_ptr<quic::QuicSocket::ByteEventCallback*>
  expectRegisterTxCallback(const uint64_t offset,
                           folly::Expected<folly::Unit, quic::LocalErrorCode>
                               returnVal = folly::Unit()) const {
    auto capturedCallbackPtr =
        std::make_unique<quic::QuicSocket::ByteEventCallback*>(nullptr);
    EXPECT_CALL(*socket_, registerTxCallback(streamId_, offset, _))
        .WillOnce(
            DoAll(SaveArg<2>(&*capturedCallbackPtr.get()), Return(returnVal)));
    return capturedCallbackPtr;
  }

  /**
   * Setup an EXPECT_CALL for QuicSocket::registerDeliveryCallback.
   *
   * Returns a unique_ptr<ptr*> where the inner pointer will be populated to
   * point to the callback handler passed to registerDeliveryCallback.
   */
  std::unique_ptr<quic::QuicSocket::ByteEventCallback*>
  expectRegisterDeliveryCallback(
      const uint64_t offset,
      folly::Expected<folly::Unit, quic::LocalErrorCode> returnVal =
          folly::Unit()) const {
    auto capturedCallbackPtr =
        std::make_unique<quic::QuicSocket::ByteEventCallback*>(nullptr);
    EXPECT_CALL(*socket_, registerDeliveryCallback(streamId_, offset, _))
        .WillOnce(
            DoAll(SaveArg<2>(&*capturedCallbackPtr.get()), Return(returnVal)));
    return capturedCallbackPtr;
  }

  /**
   * Get a matcher for a proxygen ByteEvent.
   */
  static auto getByteEventMatcher(ByteEvent::EventType eventType,
                                  uint64_t offset) {
    return AllOf(testing::Property(&ByteEvent::getType, eventType),
                 testing::Property(&ByteEvent::getByteOffset, offset));
  }

 protected:
  const HTTPCodec::StreamID streamId_{1};
  quic::MockConnectionSetupCallback connectionSetupCallback_;
  quic::MockConnectionCallback connectionCallback_;
  std::shared_ptr<quic::MockQuicSocket> socket_;
  folly::EventBase eventBase_;
  WheelTimerInstance transactionTimeouts_{std::chrono::milliseconds(500),
                                          &eventBase_};
  NiceMock<MockHTTPTransactionTransport> transport_;
  StrictMock<MockHTTPHandler> handler_;
  HTTP2PriorityQueue txnEgressQueue_;
  HTTPTransaction txn_{TransportDirection::DOWNSTREAM,
                       HTTPCodec::StreamID(1),
                       1,
                       transport_,
                       txnEgressQueue_,
                       transactionTimeouts_.getWheelTimer(),
                       transactionTimeouts_.getDefaultTimeout()};
  MockHTTPTransactionTransportCallback transportCallback_;
  std::shared_ptr<ByteEventTracker> byteEventTracker_;
};

/**
 * Test when first and last body byte are written in the same buffer.
 */
TEST_F(HQByteEventTrackerTest, FirstLastBodyByteSingleWrite) {
  const uint64_t firstBodyByteOffset = 1;
  const uint64_t lastBodyByteOffset = 10;

  InSequence s;

  // first and last byte events created
  byteEventTracker_->addFirstBodyByteEvent(firstBodyByteOffset, &txn_);
  byteEventTracker_->addLastByteEvent(&txn_, lastBodyByteOffset);
  EXPECT_EQ(2, txn_.getNumPendingByteEvents());

  // first and last byte written
  EXPECT_CALL(transportCallback_, firstByteFlushed());
  auto fbbTxCbHandler = expectRegisterTxCallback(firstBodyByteOffset);
  auto fbbAckCbHandler = expectRegisterDeliveryCallback(firstBodyByteOffset);
  EXPECT_CALL(transportCallback_, lastByteFlushed());
  auto lbbTxCbHandler = expectRegisterTxCallback(lastBodyByteOffset);
  auto lbbAckCbHandler = expectRegisterDeliveryCallback(lastBodyByteOffset);

  byteEventTracker_->processByteEvents(byteEventTracker_, lastBodyByteOffset);
  ASSERT_THAT(fbbTxCbHandler, NotNull());
  ASSERT_THAT(fbbAckCbHandler, NotNull());
  ASSERT_THAT(lbbTxCbHandler, NotNull());
  ASSERT_THAT(lbbAckCbHandler, NotNull());
  Mock::VerifyAndClearExpectations(&socket_);
  Mock::VerifyAndClearExpectations(&transportCallback_);
  EXPECT_EQ(4, txn_.getNumPendingByteEvents());

  EXPECT_CALL(transportCallback_,
              trackedByteEventTX(getByteEventMatcher(
                  ByteEvent::EventType::FIRST_BYTE, firstBodyByteOffset)));
  (*fbbTxCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = firstBodyByteOffset,
                                  .type = QuicByteEventType::TX});

  EXPECT_CALL(transportCallback_,
              trackedByteEventAck(getByteEventMatcher(
                  ByteEvent::EventType::FIRST_BYTE, firstBodyByteOffset)));
  (*fbbAckCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = firstBodyByteOffset,
                                  .type = QuicByteEventType::ACK});

  EXPECT_CALL(transportCallback_,
              trackedByteEventTX(getByteEventMatcher(
                  ByteEvent::EventType::LAST_BYTE, lastBodyByteOffset)));
  (*lbbTxCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = lastBodyByteOffset,
                                  .type = QuicByteEventType::TX});

  EXPECT_CALL(transportCallback_,
              trackedByteEventAck(getByteEventMatcher(
                  ByteEvent::EventType::LAST_BYTE, lastBodyByteOffset)));
  (*lbbAckCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = lastBodyByteOffset,
                                  .type = QuicByteEventType::ACK});

  EXPECT_EQ(0, txn_.getNumPendingByteEvents());
}

/**
 * Test when first and last body byte are written in separate buffers.
 */
TEST_F(HQByteEventTrackerTest, FirstLastBodyByteSeparateWrites) {
  const uint64_t firstBodyByteOffset = 1;
  const uint64_t lastBodyByteOffset = 10;
  const uint64_t firstWriteBytes = 5; // bytes written on first write

  InSequence s;

  // first and last byte events created
  byteEventTracker_->addFirstBodyByteEvent(firstBodyByteOffset, &txn_);
  byteEventTracker_->addLastByteEvent(&txn_, lastBodyByteOffset);
  EXPECT_EQ(2, txn_.getNumPendingByteEvents());

  // first byte written
  EXPECT_CALL(transportCallback_, firstByteFlushed());
  auto fbbTxCbHandler = expectRegisterTxCallback(firstBodyByteOffset);
  auto fbbAckCbHandler = expectRegisterDeliveryCallback(firstBodyByteOffset);
  byteEventTracker_->processByteEvents(byteEventTracker_, firstWriteBytes);
  Mock::VerifyAndClearExpectations(&socket_);
  Mock::VerifyAndClearExpectations(&transportCallback_);
  EXPECT_EQ(3, txn_.getNumPendingByteEvents());
  ASSERT_THAT(fbbTxCbHandler, NotNull());
  ASSERT_THAT(fbbAckCbHandler, NotNull());

  // last byte written
  EXPECT_CALL(transportCallback_, lastByteFlushed());
  auto lbbTxCbHandler = expectRegisterTxCallback(lastBodyByteOffset);
  auto lbbAckCbHandler = expectRegisterDeliveryCallback(lastBodyByteOffset);
  byteEventTracker_->processByteEvents(byteEventTracker_, lastBodyByteOffset);
  Mock::VerifyAndClearExpectations(&socket_);
  Mock::VerifyAndClearExpectations(&transportCallback_);
  EXPECT_EQ(4, txn_.getNumPendingByteEvents());
  ASSERT_THAT(lbbTxCbHandler, NotNull());
  ASSERT_THAT(lbbAckCbHandler, NotNull());

  // TX of first and last byte
  EXPECT_CALL(transportCallback_,
              trackedByteEventTX(getByteEventMatcher(
                  ByteEvent::EventType::FIRST_BYTE, firstBodyByteOffset)));
  (*fbbTxCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = firstBodyByteOffset,
                                  .type = QuicByteEventType::TX});
  EXPECT_CALL(transportCallback_,
              trackedByteEventTX(getByteEventMatcher(
                  ByteEvent::EventType::LAST_BYTE, lastBodyByteOffset)));
  (*lbbTxCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = lastBodyByteOffset,
                                  .type = QuicByteEventType::TX});

  // ACK of first and last byte
  EXPECT_CALL(transportCallback_,
              trackedByteEventAck(getByteEventMatcher(
                  ByteEvent::EventType::FIRST_BYTE, firstBodyByteOffset)));
  (*fbbAckCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = firstBodyByteOffset,
                                  .type = QuicByteEventType::ACK});
  EXPECT_CALL(transportCallback_,
              trackedByteEventAck(getByteEventMatcher(
                  ByteEvent::EventType::LAST_BYTE, lastBodyByteOffset)));
  (*lbbAckCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = lastBodyByteOffset,
                                  .type = QuicByteEventType::ACK});

  EXPECT_EQ(0, txn_.getNumPendingByteEvents());
}

/**
 * Test when first and last body byte are written in separate buffers.
 *
 * TX and ACK events are interleaved and extra bytes beyond the last byte
 * are reported written.
 */
TEST_F(HQByteEventTrackerTest,
       FirstLastBodyByteSeparateWritesInterleavedExtraBytes) {
  const uint64_t firstBodyByteOffset = 1;
  const uint64_t lastBodyByteOffset = 10;
  const uint64_t firstWriteBytes = 5; // bytes written on first write

  // first byte event created and byte written
  byteEventTracker_->addFirstBodyByteEvent(firstBodyByteOffset, &txn_);
  EXPECT_CALL(transportCallback_, firstByteFlushed());
  auto fbbTxCbHandler = expectRegisterTxCallback(firstBodyByteOffset);
  auto fbbAckCbHandler = expectRegisterDeliveryCallback(firstBodyByteOffset);
  byteEventTracker_->processByteEvents(byteEventTracker_, firstWriteBytes);
  Mock::VerifyAndClearExpectations(&socket_);
  Mock::VerifyAndClearExpectations(&transportCallback_);
  EXPECT_EQ(2, txn_.getNumPendingByteEvents());
  ASSERT_THAT(fbbTxCbHandler, NotNull());
  ASSERT_THAT(fbbAckCbHandler, NotNull());

  // TX and ACK of first byte
  EXPECT_CALL(transportCallback_,
              trackedByteEventTX(getByteEventMatcher(
                  ByteEvent::EventType::FIRST_BYTE, firstBodyByteOffset)));
  (*fbbTxCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = firstBodyByteOffset,
                                  .type = QuicByteEventType::TX});
  EXPECT_CALL(transportCallback_,
              trackedByteEventAck(getByteEventMatcher(
                  ByteEvent::EventType::FIRST_BYTE, firstBodyByteOffset)));
  (*fbbAckCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = firstBodyByteOffset,
                                  .type = QuicByteEventType::ACK});

  // last byte event created and byte written
  byteEventTracker_->addLastByteEvent(&txn_, lastBodyByteOffset);
  EXPECT_CALL(transportCallback_, lastByteFlushed());
  auto lbbTxCbHandler = expectRegisterTxCallback(lastBodyByteOffset);
  auto lbbAckCbHandler = expectRegisterDeliveryCallback(lastBodyByteOffset);
  byteEventTracker_->processByteEvents(byteEventTracker_,
                                       lastBodyByteOffset + 40);
  Mock::VerifyAndClearExpectations(&socket_);
  Mock::VerifyAndClearExpectations(&transportCallback_);
  EXPECT_EQ(2, txn_.getNumPendingByteEvents());
  ASSERT_THAT(lbbTxCbHandler, NotNull());
  ASSERT_THAT(lbbAckCbHandler, NotNull());

  // TX and ACK of last byte
  EXPECT_CALL(transportCallback_,
              trackedByteEventTX(getByteEventMatcher(
                  ByteEvent::EventType::LAST_BYTE, lastBodyByteOffset)));
  (*lbbTxCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = lastBodyByteOffset,
                                  .type = QuicByteEventType::TX});
  EXPECT_CALL(transportCallback_,
              trackedByteEventAck(getByteEventMatcher(
                  ByteEvent::EventType::LAST_BYTE, lastBodyByteOffset)));
  (*lbbAckCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = lastBodyByteOffset,
                                  .type = QuicByteEventType::ACK});

  EXPECT_EQ(0, txn_.getNumPendingByteEvents());
}

/**
 * Test when the first and last body byte have the same offset (single byte txn)
 */
TEST_F(HQByteEventTrackerTest, FirstLastBodyByteSingleByte) {
  const uint64_t firstBodyByteOffset = 1;
  const uint64_t lastBodyByteOffset = firstBodyByteOffset;

  InSequence s; // required due to same EXPECT_CALL triggered twice

  // first byte event created and byte written
  byteEventTracker_->addFirstBodyByteEvent(firstBodyByteOffset, &txn_);
  byteEventTracker_->addLastByteEvent(&txn_, lastBodyByteOffset);
  EXPECT_EQ(2, txn_.getNumPendingByteEvents());

  EXPECT_CALL(transportCallback_, firstByteFlushed());
  auto fbbTxCbHandler = expectRegisterTxCallback(firstBodyByteOffset);
  auto fbbAckCbHandler = expectRegisterDeliveryCallback(firstBodyByteOffset);
  EXPECT_CALL(transportCallback_, lastByteFlushed());
  auto lbbTxCbHandler = expectRegisterTxCallback(lastBodyByteOffset);
  auto lbbAckCbHandler = expectRegisterDeliveryCallback(lastBodyByteOffset);

  byteEventTracker_->processByteEvents(byteEventTracker_, lastBodyByteOffset);
  EXPECT_EQ(4, txn_.getNumPendingByteEvents());
  Mock::VerifyAndClearExpectations(&socket_);
  Mock::VerifyAndClearExpectations(&transportCallback_);
  ASSERT_THAT(fbbTxCbHandler, NotNull());
  ASSERT_THAT(fbbAckCbHandler, NotNull());
  ASSERT_THAT(lbbTxCbHandler, NotNull());
  ASSERT_THAT(lbbAckCbHandler, NotNull());

  // TX of first and last byte
  EXPECT_CALL(transportCallback_,
              trackedByteEventTX(getByteEventMatcher(
                  ByteEvent::EventType::FIRST_BYTE, firstBodyByteOffset)));
  (*fbbTxCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = firstBodyByteOffset,
                                  .type = QuicByteEventType::TX});
  EXPECT_CALL(transportCallback_,
              trackedByteEventTX(getByteEventMatcher(
                  ByteEvent::EventType::LAST_BYTE, lastBodyByteOffset)));
  (*lbbTxCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = lastBodyByteOffset,
                                  .type = QuicByteEventType::TX});

  // ACK of first and last byte
  EXPECT_CALL(transportCallback_,
              trackedByteEventAck(getByteEventMatcher(
                  ByteEvent::EventType::FIRST_BYTE, firstBodyByteOffset)));
  (*fbbAckCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = firstBodyByteOffset,
                                  .type = QuicByteEventType::ACK});
  EXPECT_CALL(transportCallback_,
              trackedByteEventAck(getByteEventMatcher(
                  ByteEvent::EventType::LAST_BYTE, lastBodyByteOffset)));
  (*lbbAckCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = lastBodyByteOffset,
                                  .type = QuicByteEventType::ACK});

  EXPECT_EQ(0, txn_.getNumPendingByteEvents());
}

/**
 * Test when the first and last body byte have the same offset (single byte txn)
 *
 * Offset is zero.
 */
TEST_F(HQByteEventTrackerTest, FirstLastBodyByteSingleByteZeroOffset) {
  const uint64_t firstBodyByteOffset = 0;
  const uint64_t lastBodyByteOffset = firstBodyByteOffset;

  InSequence s; // required due to same EXPECT_CALL triggered twice

  // first byte event created and byte written
  byteEventTracker_->addFirstBodyByteEvent(firstBodyByteOffset, &txn_);
  byteEventTracker_->addLastByteEvent(&txn_, lastBodyByteOffset);
  EXPECT_EQ(2, txn_.getNumPendingByteEvents());

  EXPECT_CALL(transportCallback_, firstByteFlushed());
  auto fbbTxCbHandler = expectRegisterTxCallback(firstBodyByteOffset);
  auto fbbAckCbHandler = expectRegisterDeliveryCallback(firstBodyByteOffset);
  EXPECT_CALL(transportCallback_, lastByteFlushed());
  auto lbbTxCbHandler = expectRegisterTxCallback(lastBodyByteOffset);
  auto lbbAckCbHandler = expectRegisterDeliveryCallback(lastBodyByteOffset);

  byteEventTracker_->processByteEvents(byteEventTracker_, lastBodyByteOffset);
  EXPECT_EQ(4, txn_.getNumPendingByteEvents());
  Mock::VerifyAndClearExpectations(&socket_);
  Mock::VerifyAndClearExpectations(&transportCallback_);
  ASSERT_THAT(fbbTxCbHandler, NotNull());
  ASSERT_THAT(fbbAckCbHandler, NotNull());
  ASSERT_THAT(lbbTxCbHandler, NotNull());
  ASSERT_THAT(lbbAckCbHandler, NotNull());

  // TX of first and last byte
  EXPECT_CALL(transportCallback_,
              trackedByteEventTX(getByteEventMatcher(
                  ByteEvent::EventType::FIRST_BYTE, firstBodyByteOffset)));
  (*fbbTxCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = firstBodyByteOffset,
                                  .type = QuicByteEventType::TX});
  EXPECT_CALL(transportCallback_,
              trackedByteEventTX(getByteEventMatcher(
                  ByteEvent::EventType::LAST_BYTE, lastBodyByteOffset)));
  (*lbbTxCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = lastBodyByteOffset,
                                  .type = QuicByteEventType::TX});

  // ACK of first and last byte
  EXPECT_CALL(transportCallback_,
              trackedByteEventAck(getByteEventMatcher(
                  ByteEvent::EventType::FIRST_BYTE, firstBodyByteOffset)));
  (*fbbAckCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = firstBodyByteOffset,
                                  .type = QuicByteEventType::ACK});
  EXPECT_CALL(transportCallback_,
              trackedByteEventAck(getByteEventMatcher(
                  ByteEvent::EventType::LAST_BYTE, lastBodyByteOffset)));
  (*lbbAckCbHandler)
      ->onByteEvent(QuicByteEvent{.id = streamId_,
                                  .offset = lastBodyByteOffset,
                                  .type = QuicByteEventType::ACK});

  EXPECT_EQ(0, txn_.getNumPendingByteEvents());
}

/**
 * Test when the QUIC byte events are canceled after registration
 */
TEST_F(HQByteEventTrackerTest, FirstLastBodyByteCancellation) {
  const uint64_t firstBodyByteOffset = 1;
  const uint64_t lastBodyByteOffset = 10;

  InSequence s;

  // first byte event created and byte written
  byteEventTracker_->addFirstBodyByteEvent(firstBodyByteOffset, &txn_);
  EXPECT_EQ(1, txn_.getNumPendingByteEvents());

  EXPECT_CALL(transportCallback_, firstByteFlushed());
  auto fbbTxCbHandler = expectRegisterTxCallback(firstBodyByteOffset);
  auto fbbAckCbHandler = expectRegisterDeliveryCallback(firstBodyByteOffset);
  byteEventTracker_->processByteEvents(byteEventTracker_, firstBodyByteOffset);
  Mock::VerifyAndClearExpectations(&socket_);
  Mock::VerifyAndClearExpectations(&transportCallback_);
  EXPECT_EQ(2, txn_.getNumPendingByteEvents());
  ASSERT_THAT(fbbTxCbHandler, NotNull());
  ASSERT_THAT(fbbAckCbHandler, NotNull());

  // last byte event created and byte written
  byteEventTracker_->addLastByteEvent(&txn_, lastBodyByteOffset);
  EXPECT_EQ(3, txn_.getNumPendingByteEvents());

  EXPECT_CALL(transportCallback_, lastByteFlushed());
  auto lbbTxCbHandler = expectRegisterTxCallback(lastBodyByteOffset);
  auto lbbAckCbHandler = expectRegisterDeliveryCallback(lastBodyByteOffset);
  byteEventTracker_->processByteEvents(byteEventTracker_, lastBodyByteOffset);
  Mock::VerifyAndClearExpectations(&socket_);
  Mock::VerifyAndClearExpectations(&transportCallback_);
  EXPECT_EQ(4, txn_.getNumPendingByteEvents());
  ASSERT_THAT(lbbTxCbHandler, NotNull());
  ASSERT_THAT(lbbAckCbHandler, NotNull());

  EXPECT_CALL(transportCallback_, trackedByteEventTX(_)).Times(0);
  (*fbbTxCbHandler)
      ->onByteEventCanceled(QuicByteEvent{.id = streamId_,
                                          .offset = firstBodyByteOffset,
                                          .type = QuicByteEventType::TX});
  Mock::VerifyAndClearExpectations(&transportCallback_);

  EXPECT_CALL(transportCallback_, trackedByteEventTX(_)).Times(0);
  (*lbbTxCbHandler)
      ->onByteEventCanceled(QuicByteEvent{.id = streamId_,
                                          .offset = lastBodyByteOffset,
                                          .type = QuicByteEventType::TX});
  Mock::VerifyAndClearExpectations(&transportCallback_);

  EXPECT_CALL(transportCallback_, trackedByteEventAck(_)).Times(0);
  (*fbbAckCbHandler)
      ->onByteEventCanceled(QuicByteEvent{.id = streamId_,
                                          .offset = firstBodyByteOffset,
                                          .type = QuicByteEventType::ACK});
  Mock::VerifyAndClearExpectations(&transportCallback_);

  EXPECT_CALL(transportCallback_, trackedByteEventAck(_)).Times(0);
  (*lbbAckCbHandler)
      ->onByteEventCanceled(QuicByteEvent{.id = streamId_,
                                          .offset = lastBodyByteOffset,
                                          .type = QuicByteEventType::ACK});
  Mock::VerifyAndClearExpectations(&transportCallback_);

  EXPECT_EQ(0, txn_.getNumPendingByteEvents());
}

/**
 * Test when registration of QUIC byte events fails.
 *
 * Callbacks should be deleted, and thus there should be no error about leaks.
 */
TEST_F(HQByteEventTrackerTest, FirstLastBodyByteErrorOnRegistration) {
  const uint64_t firstBodyByteOffset = 1;
  const uint64_t lastBodyByteOffset = 10;

  // first and last byte events created
  byteEventTracker_->addFirstBodyByteEvent(firstBodyByteOffset, &txn_);
  byteEventTracker_->addLastByteEvent(&txn_, lastBodyByteOffset);
  EXPECT_EQ(2, txn_.getNumPendingByteEvents());

  // first and last byte written
  EXPECT_CALL(transportCallback_, firstByteFlushed());
  auto fbbTxCbHandler = expectRegisterTxCallback(
      firstBodyByteOffset, folly::makeUnexpected(quic::LocalErrorCode()));
  auto fbbAckCbHandler = expectRegisterDeliveryCallback(
      firstBodyByteOffset, folly::makeUnexpected(quic::LocalErrorCode()));
  EXPECT_CALL(transportCallback_, lastByteFlushed());
  auto lbbTxCbHandler = expectRegisterTxCallback(
      lastBodyByteOffset, folly::makeUnexpected(quic::LocalErrorCode()));
  auto lbbAckCbHandler = expectRegisterDeliveryCallback(
      lastBodyByteOffset, folly::makeUnexpected(quic::LocalErrorCode()));

  byteEventTracker_->processByteEvents(byteEventTracker_, lastBodyByteOffset);
  Mock::VerifyAndClearExpectations(&socket_);
  Mock::VerifyAndClearExpectations(&transportCallback_);
  EXPECT_EQ(0, txn_.getNumPendingByteEvents());
}
