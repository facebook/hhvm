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

  MOCK_METHOD(void, onPaddingCapsule, (PaddingCapsule), (override));
  MOCK_METHOD(void, onWTResetStreamCapsule, (WTResetStreamCapsule), (override));
  MOCK_METHOD(void, onWTStopSendingCapsule, (WTStopSendingCapsule), (override));
  MOCK_METHOD(void, onWTStreamCapsule, (WTStreamCapsule), (override));
  MOCK_METHOD(void, onWTMaxDataCapsule, (WTMaxDataCapsule), (override));
  MOCK_METHOD(void,
              onWTMaxStreamDataCapsule,
              (WTMaxStreamDataCapsule),
              (override));
  MOCK_METHOD(void, onWTMaxStreamsCapsule, (WTMaxStreamsCapsule), (override));
  MOCK_METHOD(void, onWTDataBlockedCapsule, (WTDataBlockedCapsule), (override));
  MOCK_METHOD(void,
              onWTStreamDataBlockedCapsule,
              (WTStreamDataBlockedCapsule),
              (override));
  MOCK_METHOD(void,
              onWTStreamsBlockedCapsule,
              (WTStreamsBlockedCapsule),
              (override));
  MOCK_METHOD(void, onDatagramCapsule, (DatagramCapsule), (override));
  MOCK_METHOD(void,
              onCloseWebTransportSessionCapsule,
              (CloseWebTransportSessionCapsule),
              (override));
  MOCK_METHOD(void,
              onDrainWebTransportSessionCapsule,
              (DrainWebTransportSessionCapsule),
              (override));
  MOCK_METHOD(void, onConnectionError, (CapsuleCodec::ErrorCode), (override));

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
  EXPECT_CALL(*filter_, onCloseWebTransportSessionCapsule(_))
      .WillOnce(Invoke([&](const CloseWebTransportSessionCapsule& capsule) {
        filter_->WebTransportFilter::onCloseWebTransportSessionCapsule(capsule);
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

  EXPECT_CALL(*filter_, onCloseWebTransportSessionCapsule(_))
      .Times(2)
      .WillRepeatedly(
          Invoke([&](const CloseWebTransportSessionCapsule& capsule) {
            filter_->WebTransportFilter::onCloseWebTransportSessionCapsule(
                capsule);
          }));

  EXPECT_CALL(*handler_, onSessionEnd(Optional<uint32_t>(WT_ERROR_CODE)));
  EXPECT_CALL(*handler_, onSessionEnd(Optional<uint32_t>(WT_ERROR_CODE + 1)));

  filter_->onBody(queue.move());
}
