/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <proxygen/lib/http/session/HTTPSessionActivityTracker.h>
#include <proxygen/lib/http/session/test/ByteEventTrackerMocks.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <proxygen/lib/http/session/test/MockByteEventTracker.h>

using namespace testing;
using namespace proxygen;

class MockHTTPSessionActivityTracker : public HTTPSessionActivityTracker {
 public:
  MockHTTPSessionActivityTracker(size_t ingressTH, size_t egressTH)
      : HTTPSessionActivityTracker(nullptr, ingressTH, egressTH) {
  }
  MOCK_METHOD(void, reportActivity, ());
};

class HTTPSessionActivityTrackerTest : public Test {
 public:
  void SetUp() override {
    timeouts_ = folly::HHWheelTimer::newTimer(
        &evb_,
        std::chrono::milliseconds(folly::HHWheelTimer::DEFAULT_TICK_INTERVAL),
        folly::TimeoutManager::InternalEnum::INTERNAL,
        std::chrono::milliseconds(60000));
    transaction_ = std::make_unique<proxygen::MockHTTPTransaction>(
        proxygen::TransportDirection::DOWNSTREAM,
        1,
        1,
        downstreamEgressQueue_,
        timeouts_.get(),
        folly::none);
    httpSessionActivityTracker_ =
        std::make_unique<MockHTTPSessionActivityTracker>(1000, 1000);
  }

 protected:
  std::unique_ptr<MockHTTPSessionActivityTracker> httpSessionActivityTracker_;
  proxygen::HTTP2PriorityQueue downstreamEgressQueue_;
  folly::EventBase evb_;
  folly::HHWheelTimer::UniquePtr timeouts_;
  std::unique_ptr<proxygen::MockHTTPTransaction> transaction_;
};

TEST_F(HTTPSessionActivityTrackerTest, onIngress) {
  EXPECT_CALL(*httpSessionActivityTracker_, reportActivity()).Times(3);
  EXPECT_FALSE(httpSessionActivityTracker_->onIngressBody(500));
  EXPECT_FALSE(httpSessionActivityTracker_->onIngressBody(300));
  EXPECT_TRUE(httpSessionActivityTracker_->onIngressBody(700));
  EXPECT_TRUE(httpSessionActivityTracker_->onIngressBody(2700));
  EXPECT_TRUE(httpSessionActivityTracker_->onIngressBody(900));
}

TEST_F(HTTPSessionActivityTrackerTest, addTrackedEgressByteEvent) {

  {
    MockByteEventTracker byteEventTracker(nullptr);
    EXPECT_CALL(byteEventTracker, addTrackedByteEvent(_, _, _)).Times(0);
    httpSessionActivityTracker_->addTrackedEgressByteEvent(
        500, 400, &byteEventTracker, transaction_.get());
  }
  {
    MockByteEventTracker byteEventTracker(nullptr);
    httpSessionActivityTracker_ =
        std::make_unique<MockHTTPSessionActivityTracker>(1000, 1000);
    EXPECT_CALL(byteEventTracker,
                addTrackedByteEvent(transaction_.get(), 1000, _));
    EXPECT_CALL(byteEventTracker,
                addTrackedByteEvent(transaction_.get(), 2000, _));
    httpSessionActivityTracker_->addTrackedEgressByteEvent(
        0, 500, &byteEventTracker, transaction_.get());
    httpSessionActivityTracker_->addTrackedEgressByteEvent(
        500, 1600, &byteEventTracker, transaction_.get());
  }
  {
    MockByteEventTracker byteEventTracker(nullptr);
    httpSessionActivityTracker_ =
        std::make_unique<MockHTTPSessionActivityTracker>(1000, 1000);
    EXPECT_CALL(byteEventTracker,
                addTrackedByteEvent(transaction_.get(), 1000, _));
    EXPECT_CALL(byteEventTracker,
                addTrackedByteEvent(transaction_.get(), 2000, _));
    EXPECT_CALL(byteEventTracker,
                addTrackedByteEvent(transaction_.get(), 3000, _));
    httpSessionActivityTracker_->addTrackedEgressByteEvent(
        0, 1100, &byteEventTracker, transaction_.get());
    httpSessionActivityTracker_->addTrackedEgressByteEvent(
        1100, 2600, &byteEventTracker, transaction_.get());
  }
}
