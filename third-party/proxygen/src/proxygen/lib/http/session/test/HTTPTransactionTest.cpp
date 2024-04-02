/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <chrono>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <memory>
#include <proxygen/lib/http/session/ByteEvents.h>
#include <proxygen/lib/http/session/HTTP2PriorityQueue.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <proxygen/lib/http/session/test/MockHTTPTransactionObserver.h>

using namespace proxygen;
using namespace testing;
using namespace std::chrono_literals;

class HTTPTransactionObservabilityTest : public ::testing::Test {
 protected:
  void SetUp() override {
    txn_ = std::make_unique<HTTPTransaction>(TransportDirection::DOWNSTREAM,
                                             HTTPCodec::StreamID(1),
                                             0,
                                             transport_,
                                             txnEgressQueue_,
                                             &evb_.timer());
  }

  void TearDown() override {
    if (txn_) {
      EXPECT_CALL(transport_, detach(txn_.get())).WillOnce([this] {
        txn_.reset();
      });
      EXPECT_CALL(transport_, sendAbort(txn_.get(), _));
      txn_->sendAbort();
    }
  }

  std::unique_ptr<MockHTTPTransactionObserver> addMockTxnObserver(
      MockHTTPTransactionObserver::EventSet eventSet) {
    auto observer =
        std::make_unique<NiceMock<MockHTTPTransactionObserver>>(eventSet);
    EXPECT_CALL(*observer, attached(Eq(txn_->getObserverAccessor()))).Times(1);
    txn_->addObserver(observer.get());
    return std::move(observer);
  }

  std::shared_ptr<MockHTTPTransactionObserver> addMockTxnObserverShared(
      MockHTTPTransactionObserver::EventSet eventSet) {
    auto observer =
        std::make_shared<NiceMock<MockHTTPTransactionObserver>>(eventSet);
    EXPECT_CALL(*observer, attached(Eq(txn_->getObserverAccessor()))).Times(1);
    txn_->addObserver(observer);
    return observer;
  }

  folly::EventBase evb_;
  HTTP2PriorityQueue txnEgressQueue_;
  StrictMock<MockHTTPTransactionTransport> transport_;
  std::unique_ptr<HTTPTransaction> txn_;
};

TEST_F(HTTPTransactionObservabilityTest, ObserverAttachedDetached) {
  MockHTTPTransactionObserver::EventSet eventSet;
  auto observer = addMockTxnObserver(eventSet);
  EXPECT_CALL(*observer, detached(Eq(txn_->getObserverAccessor()))).Times(1);
  txn_->removeObserver(observer.get());
}

TEST_F(HTTPTransactionObservabilityTest, ObserverAttachedDestroyed) {
  MockHTTPTransactionObserver::EventSet eventSet;
  auto observer = addMockTxnObserver(eventSet);
  EXPECT_CALL(*observer, destroyed(txn_->getObserverAccessor(), _));
  txn_ = nullptr;
}

TEST_F(HTTPTransactionObservabilityTest, ObserverAttachedDetachedSharedPtr) {
  MockHTTPTransactionObserver::EventSet eventSet;
  auto observer = addMockTxnObserverShared(eventSet);
  EXPECT_CALL(*observer, detached(Eq(txn_->getObserverAccessor()))).Times(1);
  txn_->removeObserver(observer.get());
}

TEST_F(HTTPTransactionObservabilityTest, ObserverAttachedDestroyedSharedPtr) {
  MockHTTPTransactionObserver::EventSet eventSet;
  auto observer = addMockTxnObserverShared(eventSet);
  EXPECT_CALL(*observer, destroyed(txn_->getObserverAccessor(), _));
  txn_ = nullptr;
}

TEST_F(HTTPTransactionObservabilityTest,
       OnBytesEvent_ObserverNotSubscribedToTxnBytesEvent) {
  auto observer =
      addMockTxnObserver(MockHTTPTransactionObserver::EventSetBuilder()
                             .build() /* no events set */);
  EXPECT_CALL(*observer, onBytesEvent(_, _)).Times(0);
  txn_->onEgressHeaderFirstByte();
}

TEST_F(HTTPTransactionObservabilityTest,
       OnBytesEvent_ObserverSubscribedToTxnBytesEvent) {
  auto observer = addMockTxnObserver(
      MockHTTPTransactionObserver::EventSetBuilder()
          .enable(MockHTTPTransactionObserver::Events::TxnBytes)
          .build());
  InSequence s;
  EXPECT_CALL(*observer, onBytesEvent(_, _))
      .WillOnce(Invoke(
          [&](HTTPTransactionObserverAccessor* observerAccessor,
              const proxygen::HTTPTransactionObserverInterface::TxnBytesEvent&
                  event) {
            EXPECT_THAT(observerAccessor, Eq(txn_->getObserverAccessor()));
            EXPECT_THAT(event.type,
                        Eq(proxygen::HTTPTransactionObserverInterface::
                               TxnBytesEvent::Type::FIRST_HEADER_BYTE_WRITE));
          }));
  EXPECT_CALL(*observer, onBytesEvent(_, _))
      .WillOnce(Invoke(
          [&](HTTPTransactionObserverAccessor* observerAccessor,
              const proxygen::HTTPTransactionObserverInterface::TxnBytesEvent&
                  event) {
            EXPECT_THAT(observerAccessor, Eq(txn_->getObserverAccessor()));
            EXPECT_THAT(event.type,
                        Eq(proxygen::HTTPTransactionObserverInterface::
                               TxnBytesEvent::Type::FIRST_BODY_BYTE_WRITE));
          }));
  EXPECT_CALL(*observer, onBytesEvent(_, _))
      .WillOnce(Invoke(
          [&](HTTPTransactionObserverAccessor* observerAccessor,
              const proxygen::HTTPTransactionObserverInterface::TxnBytesEvent&
                  event) {
            EXPECT_THAT(observerAccessor, Eq(txn_->getObserverAccessor()));
            EXPECT_THAT(event.type,
                        Eq(proxygen::HTTPTransactionObserverInterface::
                               TxnBytesEvent::Type::LAST_BODY_BYTE_WRITE));
          }));
  EXPECT_CALL(*observer, onBytesEvent(_, _))
      .WillOnce(Invoke(
          [&](HTTPTransactionObserverAccessor* observerAccessor,
              const proxygen::HTTPTransactionObserverInterface::TxnBytesEvent&
                  event) {
            EXPECT_THAT(observerAccessor, Eq(txn_->getObserverAccessor()));
            EXPECT_THAT(event.type,
                        Eq(proxygen::HTTPTransactionObserverInterface::
                               TxnBytesEvent::Type::FIRST_BODY_BYTE_ACK));
          }));
  EXPECT_CALL(*observer, onBytesEvent(_, _))
      .WillOnce(Invoke(
          [&](HTTPTransactionObserverAccessor* observerAccessor,
              const proxygen::HTTPTransactionObserverInterface::TxnBytesEvent&
                  event) {
            EXPECT_THAT(observerAccessor, Eq(txn_->getObserverAccessor()));
            EXPECT_THAT(event.type,
                        Eq(proxygen::HTTPTransactionObserverInterface::
                               TxnBytesEvent::Type::LAST_BODY_BYTE_ACK));
          }));
  txn_->onEgressHeaderFirstByte();
  txn_->onEgressBodyFirstByte();
  txn_->onEgressBodyLastByte();
  txn_->onEgressTrackedByteEventAck(
      {0 /* byteOffset */, ByteEvent::FIRST_BYTE});
  txn_->onEgressLastByteAck(42ms /* latency */);

  // Currently, observers are not notified of egress bytes acked for non-first
  // byte
  EXPECT_CALL(*observer, onBytesEvent(_, _)).Times(0);
  txn_->onEgressTrackedByteEventAck(
      {2 /* byteOffset */, ByteEvent::TRACKED_BYTE});
  txn_->onEgressTrackedByteEventAck({3 /* byteOffset */, ByteEvent::LAST_BYTE});
}
