/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/SocketAddress.h>
#include <folly/io/async/HHWheelTimer.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/test/MockHTTPCodec.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>
#include <proxygen/lib/test/TestAsyncTransport.h>

#include <proxygen/lib/http/connpool/SessionHolder.h>

namespace proxygen {

folly::SocketAddress local("127.0.0.1", 80);
folly::SocketAddress peer("127.0.0.1", 12345);

class MockSessionHolderCallback : public SessionHolder::Callback {
 public:
  MOCK_METHOD(void, detachIdle, (SessionHolder*), ());
  MOCK_METHOD(void, detachPartiallyFilled, (SessionHolder*), ());
  MOCK_METHOD(void, detachFilled, (SessionHolder*), ());
  MOCK_METHOD(void, attachIdle, (SessionHolder*), ());
  MOCK_METHOD(void, attachPartiallyFilled, (SessionHolder*), ());
  MOCK_METHOD(void, attachFilled, (SessionHolder*), ());
  MOCK_METHOD(void, addDrainingSession, (HTTPSessionBase*), ());
};

std::unique_ptr<testing::NiceMock<MockHTTPCodec>> makeCodecCommon() {
  static int txnIdx = 1;
  auto codec = std::make_unique<testing::NiceMock<MockHTTPCodec>>();
  EXPECT_CALL(*codec, getTransportDirection())
      .WillRepeatedly(testing::Return(TransportDirection::UPSTREAM));
  EXPECT_CALL(*codec, createStream())
      .WillRepeatedly(testing::InvokeWithoutArgs([&]() { return txnIdx++; }));
  EXPECT_CALL(*codec, isReusable()).WillRepeatedly(testing::Return(true));
  EXPECT_CALL(*codec, getProtocol())
      .WillRepeatedly(testing::Return(CodecProtocol::HTTP_2));
  return codec;
}

std::unique_ptr<testing::NiceMock<MockHTTPCodec>> makeSerialCodec() {
  auto codec = makeCodecCommon();
  EXPECT_CALL(*codec, supportsParallelRequests())
      .WillRepeatedly(testing::Return(false));
  EXPECT_CALL(*codec, getProtocol())
      .WillRepeatedly(testing::Return(CodecProtocol::HTTP_1_1));
  return codec;
}

std::unique_ptr<testing::NiceMock<MockHTTPCodec>> makeParallelCodec() {
  auto codec = makeCodecCommon();
  EXPECT_CALL(*codec, supportsParallelRequests())
      .WillRepeatedly(testing::Return(true));
  EXPECT_CALL(*codec, generateRstStream(testing::_, testing::_, testing::_))
      .WillRepeatedly(testing::Return(1));
  EXPECT_CALL(*codec, getProtocol())
      .WillRepeatedly(testing::Return(CodecProtocol::HTTP_2));
  return codec;
}

class SessionPoolFixture
    : public testing::Test
    , public SessionHolder::Stats
    , public HTTPTransaction::Handler
    , public HTTPSessionBase::InfoCallback {
 public:
  void SetUp() override {
  }

  void TearDown() override {
    evb_.loop();
  }

  HTTPUpstreamSession* makeSerialSession() {
    return makeSession(makeSerialCodec());
  }

  HTTPUpstreamSession* makeParallelSession() {
    return makeSession(makeParallelCodec());
  }

  HTTPUpstreamSession* makeSession(std::unique_ptr<HTTPCodec> codec) {
    auto sock = folly::AsyncTransport::UniquePtr(new TestAsyncTransport(&evb_));
    wangle::TransportInfo tinfo;
    tinfo.acceptTime = getCurrentTime();
    return new HTTPUpstreamSession(timeouts_.get(),
                                   std::move(sock),
                                   local,
                                   peer,
                                   std::move(codec),
                                   tinfo,
                                   this);
  }

  void onCreate(const HTTPSessionBase&) override {
    numSessions_++;
  }
  // Note: you must not start any asynchronous work from onDestroy()
  void onDestroy(const HTTPSessionBase&) override {
    numSessions_--;
  }
  using HTTPSessionBase::InfoCallback::onRead;
  using HTTPSessionBase::InfoCallback::onWrite;

  // SessionHolder::Stats
  void onConnectionCreated() override {
  }

  void onConnectionClosed() override {
    ++closed_;
  }
  void onConnectionActivated() override {
    ++activated_;
  }
  void onConnectionDeactivated() override {
    ++deactivated_;
  }
  void onRead(size_t /*bytesRead*/) override {
  }
  void onWrite(size_t /*bytesWritten*/) override {
  }

  // HTTPTransaction::Handler
  void setTransaction(HTTPTransaction* /*txn*/) noexcept override {
    attached_ = true;
  }
  void detachTransaction() noexcept override {
    attached_ = false;
  }
  void onHeadersComplete(
      std::unique_ptr<HTTPMessage> /*msg*/) noexcept override {
  }
  void onBody(std::unique_ptr<folly::IOBuf> /*chain*/) noexcept override {
  }
  void onChunkHeader(size_t /*length*/) noexcept override{};
  void onChunkComplete() noexcept override{};
  void onTrailers(std::unique_ptr<HTTPHeaders> /*trailers*/) noexcept override {
  }
  void onEOM() noexcept override {
  }
  void onUpgrade(UpgradeProtocol /*protocol*/) noexcept override {
  }
  void onError(const HTTPException& /*error*/) noexcept override {
  }
  void onEgressPaused() noexcept override {
  }
  void onEgressResumed() noexcept override {
  }

 protected:
  folly::EventBase evb_;
  folly::HHWheelTimer::UniquePtr timeouts_{folly::HHWheelTimer::newTimer(
      &evb_,
      std::chrono::milliseconds(folly::HHWheelTimer::DEFAULT_TICK_INTERVAL),
      folly::TimeoutManager::InternalEnum::INTERNAL,
      std::chrono::milliseconds(100))};
  uint32_t closed_{0};
  uint32_t activated_{0};
  uint32_t deactivated_{0};
  uint32_t numSessions_{0};
  bool attached_{false};
  bool timeout_{false};
};

} // namespace proxygen
