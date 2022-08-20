/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/test/HQSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>

using namespace proxygen;
using namespace testing;
/**
 * A test case to validate that mocks exported by HQsesion
 * work correctly
 */
class MocksTest : public Test {
 public:
  void SetUp() override {
    handler_ = std::make_unique<StrictMock<MockHTTPHandler>>();
    hqSession_ = std::make_unique<StrictMock<MockHQSession>>();

    hqSession_->quicStreamProtocolInfo_->ptoCount = 1;
    hqSession_->quicStreamProtocolInfo_->totalPTOCount = 2;
    hqSession_->quicStreamProtocolInfo_->totalTransportBytesSent = 3;
    hqSession_->quicStreamProtocolInfo_->streamTransportInfo.holbCount = 4;
  }

  void TearDown() override {
  }

 protected:
  std::unique_ptr<StrictMock<MockHTTPHandler>> handler_;
  folly::HHWheelTimer::UniquePtr timeoutSet_;
  wangle::TransportInfo tInfo_;
  std::unique_ptr<StrictMock<MockHQSession>> hqSession_;
};

// Test that the mocked `newTransaction` does the right thing
TEST_F(MocksTest, MockHQSessionNewTransaction) {
  // The default implementation of `newTransaction` is provided in Mocks.h
  // But it has to be enabled via `EXPECT_CALL` without an action.
  EXPECT_CALL(*hqSession_, newTransaction(_));

  // Capture the transaction object that HQSession will pass to the
  // handler
  HTTPTransaction* txn;
  EXPECT_CALL(*handler_, _setTransaction(_)).WillOnce(SaveArg<0>(&txn));

  hqSession_->newTransaction(handler_.get());

  // After the `setTransaction` has been invoked, the handler and transaction
  // must be tied.
  EXPECT_EQ(txn, hqSession_->txn_.get())
      << "`newTransaction` should call `handler::setTransaction`";
  EXPECT_EQ(handler_.get(), hqSession_->handler_)
      << "`newTransaction` should set the session handler";
}

// Test that the mocked `newTransaction` does the right thing
TEST_F(MocksTest, MockHQSessionPropagatesQuickProtocolInfo) {
  // The default implementation of `newTransaction` is provided in Mocks.h
  // But it has to be enabled via `EXPECT_CALL` without an action.
  EXPECT_CALL(*hqSession_, newTransaction(_));

  // Capture the transaction object that HQSession will pass to the
  // handler
  HTTPTransaction* txn;
  EXPECT_CALL(*handler_, _setTransaction(_)).WillOnce(SaveArg<0>(&txn));

  hqSession_->newTransaction(handler_.get());

  wangle::TransportInfo tinfo;

  hqSession_->txn_->getCurrentTransportInfo(&tinfo);

  auto receivedProtocolInfo =
      dynamic_cast<QuicStreamProtocolInfo*>(tinfo.protocolInfo.get());

  EXPECT_NE(receivedProtocolInfo, nullptr)
      << "`getCurrentTransportInfo` should return QuicStreamProtocolInfo";

  EXPECT_EQ(receivedProtocolInfo->totalPTOCount, 2)
      << "received protocol info should reflect the quic protocol info";
}
