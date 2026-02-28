/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <proxygen/lib/http/session/test/ByteEventTrackerMocks.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>

#include <chrono>

using namespace testing;
using namespace proxygen;

class ByteEventTrackerTest : public Test {
 public:
  void SetUp() override {
    txn_.setTransportCallback(&transportCallback_);
  }

 protected:
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
  MockByteEventTrackerCallback callback_;
  std::shared_ptr<ByteEventTracker> byteEventTracker_{
      new ByteEventTracker(&callback_)};
};

TEST_F(ByteEventTrackerTest, Ping) {
  auto pingByteEventCb = [](ByteEvent& event) {
    EXPECT_EQ(event.getType(), ByteEvent::PING_REPLY_SENT);
  };
  byteEventTracker_->addPingByteEvent(
      10, proxygen::getCurrentTime(), 0, pingByteEventCb);
  EXPECT_CALL(callback_, onPingReplyLatency(_));
  byteEventTracker_->processByteEvents(byteEventTracker_, 10);
}

TEST_F(ByteEventTrackerTest, Ttlb) {
  auto lastBodyByteEventCb = [](ByteEvent& event) {
    EXPECT_EQ(event.getType(), ByteEvent::LAST_BYTE);
  };
  byteEventTracker_->addLastByteEvent(&txn_, 10, lastBodyByteEventCb);
  EXPECT_CALL(transportCallback_, headerBytesGenerated(_)); // sendAbort calls?
  txn_.sendAbort(); // put it in a state for detach
  EXPECT_CALL(transportCallback_, lastByteFlushed());
  EXPECT_CALL(
      callback_,
      onTxnByteEventWrittenToBuf(AllOf(
          Property(&ByteEvent::getByteOffset, 10),
          Property(&ByteEvent::getType, ByteEvent::EventType::LAST_BYTE))));
  EXPECT_CALL(transport_, detach(_));
  byteEventTracker_->processByteEvents(byteEventTracker_, 10);
}

class MockTrackedByteEventCb {
 public:
  MockTrackedByteEventCb() = default;
  MOCK_METHOD(void, onEventByteCallback, (const ByteEvent&));
};

TEST_F(ByteEventTrackerTest, TrackBytes) {
  MockTrackedByteEventCb mockCB;
  EXPECT_CALL(mockCB, onEventByteCallback(_)).Times(2);
  auto trackedByteEventCb = [&](ByteEvent& event) {
    EXPECT_EQ(event.getType(), ByteEvent::TRACKED_BYTE);
    mockCB.onEventByteCallback(event);
  };
  byteEventTracker_->addTrackedByteEvent(&txn_, 10, trackedByteEventCb);
  byteEventTracker_->addTrackedByteEvent(
      &txn_, 10, trackedByteEventCb); // same offset
  byteEventTracker_->processByteEvents(byteEventTracker_, 10);
}
