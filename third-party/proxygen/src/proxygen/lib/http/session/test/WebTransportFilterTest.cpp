/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportFramer.h>
#include <proxygen/lib/http/session/WebTransportFilter.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <proxygen/lib/http/webtransport/test/Mocks.h>

using namespace proxygen;
using namespace proxygen::test;
using namespace testing;

namespace {
const uint32_t WT_ERROR_CODE = 42;
const std::string WT_REASON = "test close reason";
} // namespace

class MockWebTransportFilter : public WebTransportFilter {
 public:
  MockWebTransportFilter(HTTPTransaction* txn, CodecVersion version)
      : WebTransportFilter(txn, version) {
  }

  MOCK_METHOD(void, onPaddingCapsule, (PaddingCapsule), (override, noexcept));
  MOCK_METHOD(void,
              onWTResetStreamCapsule,
              (WTResetStreamCapsule),
              (override, noexcept));
  MOCK_METHOD(void,
              onWTStopSendingCapsule,
              (WTStopSendingCapsule),
              (override, noexcept));
  MOCK_METHOD(void, onWTStreamCapsule, (WTStreamCapsule), (override, noexcept));
  MOCK_METHOD(void,
              onWTMaxDataCapsule,
              (WTMaxDataCapsule),
              (override, noexcept));
  MOCK_METHOD(void,
              onWTMaxStreamDataCapsule,
              (WTMaxStreamDataCapsule),
              (override, noexcept));
  MOCK_METHOD(void,
              onWTMaxStreamsBidiCapsule,
              (WTMaxStreamsCapsule),
              (override, noexcept));
  MOCK_METHOD(void,
              onWTMaxStreamsUniCapsule,
              (WTMaxStreamsCapsule),
              (override, noexcept));
  MOCK_METHOD(void,
              onWTDataBlockedCapsule,
              (WTDataBlockedCapsule),
              (override, noexcept));
  MOCK_METHOD(void,
              onWTStreamDataBlockedCapsule,
              (WTStreamDataBlockedCapsule),
              (override, noexcept));
  MOCK_METHOD(void,
              onWTStreamsBlockedBidiCapsule,
              (WTStreamsBlockedCapsule),
              (override, noexcept));
  MOCK_METHOD(void,
              onWTStreamsBlockedUniCapsule,
              (WTStreamsBlockedCapsule),
              (override, noexcept));
  MOCK_METHOD(void, onDatagramCapsule, (DatagramCapsule), (override, noexcept));
  MOCK_METHOD(void,
              onCloseWTSessionCapsule,
              (CloseWebTransportSessionCapsule),
              (override, noexcept));
  MOCK_METHOD(void,
              onDrainWTSessionCapsule,
              (DrainWebTransportSessionCapsule),
              (override, noexcept));
  MOCK_METHOD(void,
              onConnectionError,
              (CapsuleCodec::ErrorCode),
              (override, noexcept));

  MOCK_METHOD((folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>),
              newWebTransportBidiStream,
              (),
              (override));
  MOCK_METHOD((folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>),
              newWebTransportUniStream,
              (),
              (override));
  MOCK_METHOD((folly::Expected<WebTransport::FCState, WebTransport::ErrorCode>),
              sendWebTransportStreamData,
              (HTTPCodec::StreamID,
               std::unique_ptr<folly::IOBuf>,
               bool,
               WebTransport::ByteEventCallback*),
              (override));
  MOCK_METHOD((folly::Expected<folly::Unit, WebTransport::ErrorCode>),
              resetWebTransportEgress,
              (HTTPCodec::StreamID, uint32_t),
              (override));
  MOCK_METHOD((folly::Expected<folly::Unit, WebTransport::ErrorCode>),
              sendDatagram,
              (std::unique_ptr<folly::IOBuf>),
              (override));
  MOCK_METHOD((const folly::SocketAddress&),
              getLocalAddress,
              (),
              (override, const));
  MOCK_METHOD((const folly::SocketAddress&),
              getPeerAddress,
              (),
              (override, const));
};

class WebTransportFilterTest : public Test {
 protected:
  void SetUp() override {
    handler_ = std::make_unique<StrictMock<MockWebTransportHandler>>();
    egressQueue_ = std::make_unique<NiceMock<HTTP2PriorityQueue>>();

    txn_ = std::make_unique<NiceMock<MockHTTPTransaction>>(
        TransportDirection::DOWNSTREAM,
        1, // stream id
        0, // seq no
        *egressQueue_,
        nullptr,                // timer
        folly::none,            // idle timeout
        nullptr,                // stats
        false,                  // use flow control
        1024,                   // receive window
        1024,                   // send window
        http2::DefaultPriority, // priority
        folly::none,            // assoc stream id
        folly::none             // ex attributes
    );

    filter_ =
        std::make_unique<MockWebTransportFilter>(txn_.get(), CodecVersion::H3);
    filter_->setHandler(handler_.get());
  }

  void TearDown() override {
    filter_->clearTransaction();
  }

  std::unique_ptr<folly::IOBuf> createCloseSessionCapsule(
      uint32_t errorCode = WT_ERROR_CODE,
      const std::string& reason = WT_REASON) {
    folly::IOBufQueue queue;
    CloseWebTransportSessionCapsule capsule{.applicationErrorCode = errorCode,
                                            .applicationErrorMessage = reason};
    writeCloseWebTransportSession(queue, capsule);

    return queue.move();
  }

  std::unique_ptr<StrictMock<MockWebTransportHandler>> handler_;
  std::unique_ptr<NiceMock<HTTP2PriorityQueue>> egressQueue_;
  std::unique_ptr<NiceMock<MockHTTPTransaction>> txn_;
  std::unique_ptr<MockWebTransportFilter> filter_;
};

TEST_F(WebTransportFilterTest, OnCloseWebTransportSessionCapsule) {
  // expect the capsule callback to be called and call through to original
  // implementation to test for onSessionEnd()
  EXPECT_CALL(*filter_, onCloseWTSessionCapsule(_))
      .WillOnce(Invoke([&](const CloseWebTransportSessionCapsule& capsule) {
        filter_->WebTransportFilter::onCloseWTSessionCapsule(capsule);
      }));
  EXPECT_CALL(*handler_, onSessionEnd(Optional<uint32_t>(WT_ERROR_CODE)));

  auto capsuleData = createCloseSessionCapsule();
  filter_->onBody(std::move(capsuleData));
}

TEST_F(WebTransportFilterTest, OnCloseWebTransportSessionCapsuleNoHandler) {
  // clear the handler to test the case where handler is null
  filter_->setHandler(nullptr);

  auto capsuleData = createCloseSessionCapsule();
  // this should not crash even without a handler
  EXPECT_NO_THROW(filter_->onBody(std::move(capsuleData)));
}

TEST_F(WebTransportFilterTest, MultipleCapsulesInOneBody) {
  folly::IOBufQueue queue;

  // first capsule - close session
  auto firstCapsule = createCloseSessionCapsule(WT_ERROR_CODE);
  queue.append(std::move(firstCapsule));

  // second capsule - another close session
  auto secondCapsule = createCloseSessionCapsule(WT_ERROR_CODE + 1);
  queue.append(std::move(secondCapsule));

  EXPECT_CALL(*filter_, onCloseWTSessionCapsule(_))
      .Times(2)
      .WillRepeatedly(
          Invoke([&](const CloseWebTransportSessionCapsule& capsule) {
            filter_->WebTransportFilter::onCloseWTSessionCapsule(capsule);
          }));

  EXPECT_CALL(*handler_, onSessionEnd(Optional<uint32_t>(WT_ERROR_CODE)));
  EXPECT_CALL(*handler_, onSessionEnd(Optional<uint32_t>(WT_ERROR_CODE + 1)));

  filter_->onBody(queue.move());
}

TEST_F(WebTransportFilterTest, StreamCreationLimits) {
  // Use H2 codec version so the filter manages stream IDs directly without
  // delegating to an underlying transport provider
  auto filter =
      std::make_unique<WebTransportFilter>(txn_.get(), CodecVersion::H2);

  // Initially should NOT be able to create streams since maxStreams is 0 for H2
  EXPECT_FALSE(filter->canCreateUniStream());
  EXPECT_FALSE(filter->canCreateBidiStream());

  WTMaxStreamsCapsule uniCapsule{2};
  WTMaxStreamsCapsule bidiCapsule{3};
  filter->onWTMaxStreamsUniCapsule(uniCapsule);
  filter->onWTMaxStreamsBidiCapsule(bidiCapsule);
  EXPECT_TRUE(filter->canCreateUniStream());
  EXPECT_TRUE(filter->canCreateBidiStream());

  // Create the first uni stream (stream ID will be 3)
  auto uniResult1 = filter->newWebTransportUniStream();
  EXPECT_TRUE(uniResult1.hasValue());
  EXPECT_EQ(uniResult1.value(), 3);
  EXPECT_TRUE(filter->canCreateUniStream());

  // Create the second uni stream (stream ID will be 7)
  auto uniResult2 = filter->newWebTransportUniStream();
  EXPECT_TRUE(uniResult2.hasValue());
  EXPECT_EQ(uniResult2.value(), 7);

  // Now we've created 2 uni streams, limit reached, should not be able to
  // create more
  EXPECT_FALSE(filter->canCreateUniStream());

  // Create the first bidi stream (stream ID will be 1)
  auto bidiResult1 = filter->newWebTransportBidiStream();
  EXPECT_TRUE(bidiResult1.hasValue());
  EXPECT_EQ(bidiResult1.value(), 1);
  EXPECT_TRUE(filter->canCreateBidiStream());

  // Create the second and third bidi streams
  auto bidiResult2 = filter->newWebTransportBidiStream();
  auto bidiResult3 = filter->newWebTransportBidiStream();
  EXPECT_TRUE(bidiResult2.hasValue());
  EXPECT_TRUE(bidiResult3.hasValue());

  // Now we've created 3 bidi streams, limit reached, should not be able to
  // create more
  EXPECT_FALSE(filter->canCreateBidiStream());
}

TEST_F(WebTransportFilterTest,
       OnCloseWebTransportSessionCapsuleWhenEgressComplete) {
  // Test that sendEOM() is NOT called when transaction egress is already
  // complete
  auto egressQueueForPushed = std::make_unique<NiceMock<HTTP2PriorityQueue>>();

  // Create an upstream transaction with assocStreamId set, which starts it in
  // SendingDone state
  auto txnWithEgressComplete = std::make_unique<NiceMock<MockHTTPTransaction>>(
      TransportDirection::UPSTREAM, // Must be upstream for SendingDone
      2,                            // stream id
      0,                            // seq no
      *egressQueueForPushed,
      nullptr,                // timer
      folly::none,            // idle timeout
      nullptr,                // stats
      false,                  // use flow control
      1024,                   // receive window
      1024,                   // send window
      http2::DefaultPriority, // priority
      1                       // assocStreamId - this triggers SendingDone
  );

  // Verify the transaction is in SendingDone state
  EXPECT_TRUE(txnWithEgressComplete->isEgressComplete());

  // Create a filter with this transaction
  auto filterWithCompleteTxn = std::make_unique<MockWebTransportFilter>(
      txnWithEgressComplete.get(), CodecVersion::H3);
  filterWithCompleteTxn->setHandler(handler_.get());

  // Verify that sendEOM() is NOT called when egress is already complete
  EXPECT_CALL(*txnWithEgressComplete, sendEOM()).Times(0);

  // Expect the capsule callback to be called and call through to the real
  // implementation to test the fix
  EXPECT_CALL(*filterWithCompleteTxn, onCloseWTSessionCapsule(_))
      .WillOnce(Invoke([&](const CloseWebTransportSessionCapsule& capsule) {
        filterWithCompleteTxn->WebTransportFilter::onCloseWTSessionCapsule(
            capsule);
      }));

  EXPECT_CALL(*handler_, onSessionEnd(Optional<uint32_t>(WT_ERROR_CODE)));

  auto capsuleData = createCloseSessionCapsule();
  filterWithCompleteTxn->onBody(std::move(capsuleData));
}

TEST_F(WebTransportFilterTest,
       OnCloseWebTransportSessionCapsuleWhenEgressNotComplete) {
  // Verify the normal case where egress is not complete, sendEOM() should be
  // called. The transaction is in Start state by default (not SendingDone).

  // Verify that sendEOM() IS called when egress is not complete
  EXPECT_CALL(*txn_, sendEOM()).Times(1);

  // Expect the capsule callback to be called and call through to the real
  // implementation to test the normal case
  EXPECT_CALL(*filter_, onCloseWTSessionCapsule(_))
      .WillOnce(Invoke([&](const CloseWebTransportSessionCapsule& capsule) {
        filter_->WebTransportFilter::onCloseWTSessionCapsule(capsule);
      }));

  EXPECT_CALL(*handler_, onSessionEnd(Optional<uint32_t>(WT_ERROR_CODE)));

  auto capsuleData = createCloseSessionCapsule();
  filter_->onBody(std::move(capsuleData));
}
