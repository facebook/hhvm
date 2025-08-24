/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/HTTPSourceFilter.h"

#include "proxygen/lib/http/coro/transport/test/TestCoroTransport.h"
#include "proxygen/lib/http/coro/util/TimedBaton.h"

#include "proxygen/lib/http/session/test/MockHTTPSessionStats.h"
#include <folly/logging/xlog.h>
#include <proxygen/lib/http/codec/CodecProtocol.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/session/HTTPSessionStats.h>
#include <proxygen/lib/http/session/test/MockQuicSocketDriver.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace proxygen::coro::test {

struct TestParams {
  CodecProtocol codecProtocol;
  bool useDynamicTable{true};
  bool enableDatagrams{false};
};

std::string paramsToTestName(const testing::TestParamInfo<TestParams> &info);

class TestHTTPTransport {
 public:
  virtual ~TestHTTPTransport() {
  }

  // Add a read event for the given stream
  virtual void addReadEvent(HTTPCodec::StreamID id,
                            std::unique_ptr<folly::IOBuf> data,
                            bool eom = false) = 0;

  // Add a read event for the control stream
  virtual void addReadEvent(std::unique_ptr<folly::IOBuf> data,
                            bool eom = false) = 0;

  virtual void pauseWrites(HTTPCodec::StreamID id) = 0;
};

class TestUniplexTransport
    : public TestHTTPTransport
    , public TestCoroTransport {
 public:
  TestUniplexTransport(folly::EventBase *evb, TestCoroTransport::State *state)
      : TestCoroTransport(evb, state) {
  }

  void addReadEvent(std::unique_ptr<folly::IOBuf> data,
                    bool eom = false) override {
    TestCoroTransport::addReadEvent(std::move(data), eom);
  }

  void addReadEvent(HTTPCodec::StreamID /*id*/,
                    std::unique_ptr<folly::IOBuf> ev,
                    bool /*endStream*/) override {
    // This transport is uniplexed, ignore stream ID
    addReadEvent(std::move(ev), false);
  }

  void pauseWrites(HTTPCodec::StreamID /*id*/) override {
    TestCoroTransport::pauseWrites();
  }
};

class TestCoroMultiplexTransport : public TestHTTPTransport {
 public:
  class DummyConnectionCallback
      : public quic::QuicSocket::ConnectionSetupCallback
      , public quic::QuicSocket::ConnectionCallback {
   public:
    void onNewBidirectionalStream(quic::StreamId /*id*/) noexcept override {
      XLOG(FATAL) << __func__ << " on dummy conn cb";
    }
    void onNewUnidirectionalStream(quic::StreamId /*id*/) noexcept override {
      XLOG(FATAL) << __func__ << " on dummy conn cb";
    }
    void onStopSending(quic::StreamId /*id*/,
                       quic::ApplicationErrorCode /*error*/) noexcept override {
      XLOG(FATAL) << __func__ << " on dummy conn cb";
    }
    void onConnectionEnd() noexcept override {
      XLOG(FATAL) << __func__ << " on dummy conn cb";
    }
    void onConnectionSetupError(quic::QuicError code) noexcept override {
      onConnectionError(std::move(code));
    }
    void onConnectionError(quic::QuicError /*code*/) noexcept override {
      XLOG(FATAL) << __func__ << " on dummy conn cb";
    }
  };

  TestCoroMultiplexTransport(folly::EventBase *evb, TransportDirection dir)
      : socketDriver_(evb,
                      &dummyConnCb_,
                      &dummyConnCb_,
                      dir == TransportDirection::DOWNSTREAM
                          ? quic::MockQuicSocketDriver::TransportEnum::SERVER
                          : quic::MockQuicSocketDriver::TransportEnum::CLIENT) {
    if (dir == TransportDirection::DOWNSTREAM) {
      nextBidirectionalStreamId_ = 0;
      nextUnidirectionalStreamId_ = 2;
      socketDriver_.setMaxBidiStreams(0);
      socketDriver_.setMaxUniStreams(103);
    } else {
      nextBidirectionalStreamId_ = 1;
      nextUnidirectionalStreamId_ = 3;
      socketDriver_.setMaxBidiStreams(10);
      socketDriver_.setMaxUniStreams(3);
    }
    createControlStream(hq::UnidirectionalStreamType::CONTROL,
                        connControlStreamId_);
    createControlStream(hq::UnidirectionalStreamType::QPACK_ENCODER,
                        qpackEncoderStreamId_);
    createControlStream(hq::UnidirectionalStreamType::QPACK_DECODER,
                        qpackDecoderStreamId_);
  }

  void createControlStream(hq::UnidirectionalStreamType streamType,
                           quic::StreamId &id) {
    id = nextUnidirectionalStreamId_;
    nextUnidirectionalStreamId_ += 4;
    folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
    hq::writeStreamPreface(writeBuf, uint64_t(streamType));
    addReadEvent(id, writeBuf.move(), false);
  }

  std::shared_ptr<quic::MockQuicSocket> getSocket() {
    return socketDriver_.getSocket();
  }

  void addReadEvent(HTTPCodec::StreamID id,
                    std::unique_ptr<folly::IOBuf> ev,
                    bool endStream) override {
    socketDriver_.addReadEvent(id, std::move(ev), endStream);
  }

  void addReadEvent(std::unique_ptr<folly::IOBuf> ev, bool eof) override {
    if (ev) {
      addReadEvent(connControlStreamId_, std::move(ev), false);
    }
    if (eof) {
      socketDriver_.addOnConnectionEndEvent(0);
    }
  }

  void pauseWrites(HTTPCodec::StreamID id) override {
    socketDriver_.pauseWrites(id);
  }

  DummyConnectionCallback dummyConnCb_;
  quic::MockQuicSocketDriver socketDriver_;
  quic::StreamId connControlStreamId_;
  quic::StreamId qpackEncoderStreamId_;
  quic::StreamId qpackDecoderStreamId_;
  quic::StreamId nextUnidirectionalStreamId_;
  quic::StreamId nextBidirectionalStreamId_;
};

class MockLifecycleObserver : public LifecycleObserver {
 public:
  MOCK_METHOD(void, onAttached, (HTTPCoroSession &));
  // MOCK_METHOD(void, onTransportReady, (const HTTPCoroSession&));
  // MOCK_METHOD(void, onConnectionError, (const HTTPCoroSession&));
  // MOCK_METHOD(void, onFullHandshakeCompletion, (const HTTPCoroSession&));
  MOCK_METHOD(void, onIngressError, (const HTTPCoroSession &, ProxygenError));
  MOCK_METHOD(void, onIngressEOF, ());
  MOCK_METHOD(void,
              onRead,
              (const HTTPCoroSession &,
               size_t,
               folly::Optional<HTTPCodec::StreamID>));
  MOCK_METHOD(void, onWrite, (const HTTPCoroSession &, size_t));
  MOCK_METHOD(void, onRequestBegin, (const HTTPCoroSession &));
  MOCK_METHOD(void, onRequestEnd, (const HTTPCoroSession &, uint32_t));
  MOCK_METHOD(void, onActivateConnection, (const HTTPCoroSession &));
  MOCK_METHOD(void, onDeactivateConnection, (const HTTPCoroSession &));
  MOCK_METHOD(void, onDrainStarted, (const HTTPCoroSession &));
  MOCK_METHOD(void, onDestroy, (const HTTPCoroSession &));
  MOCK_METHOD(void,
              onIngressMessage,
              (const HTTPCoroSession &, const HTTPMessage &));
  // MOCK_METHOD(void, onIngressLimitExceeded, (const HTTPCoroSession&));
  // MOCK_METHOD(void, onIngressPaused, (const HTTPCoroSession&));
  MOCK_METHOD(void, onTransactionAttached, (const HTTPCoroSession &));
  MOCK_METHOD(void, onTransactionDetached, (const HTTPCoroSession &));
  MOCK_METHOD(void, onPingReplySent, (int64_t));
  MOCK_METHOD(void, onPingReplyReceived, ());
  MOCK_METHOD(void, onSettingsOutgoingStreamsFull, (const HTTPCoroSession &));
  MOCK_METHOD(void,
              onSettingsOutgoingStreamsNotFull,
              (const HTTPCoroSession &));
  MOCK_METHOD(void, onFlowControlWindowClosed, (const HTTPCoroSession &));
  // MOCK_METHOD(void, onEgressBuffered, (const HTTPCoroSession&));
  // MOCK_METHOD(void, onEgressBufferCleared, (const HTTPCoroSession&));
  MOCK_METHOD(void,
              onSettings,
              (const HTTPCoroSession &, const SettingsList &));
  MOCK_METHOD(void, onSettingsAck, (const HTTPCoroSession &));
  MOCK_METHOD(void,
              onGoaway,
              (const HTTPCoroSession &, const uint64_t, const ErrorCode));
};

class HTTPCoroSessionTest : public testing::TestWithParam<TestParams> {
 protected:
  explicit HTTPCoroSessionTest(TransportDirection direction)
      : direction_(direction) {
    initCodec();
  }

  folly::DrivableExecutor *getExecutor() {
    return &evb_;
  }

  void initCodec();

  void run() {
    evb_.loop();
  }

  void loopN(size_t n) {
    for (size_t i = 0; i < n; i++) {
      evb_.loopOnce();
    }
  }

  folly::coro::Task<void> rescheduleN(size_t n) {
    for (size_t i = 0; i < n; i++) {
      co_await folly::coro::co_reschedule_on_current_executor;
    }
  }

  void setUp(std::shared_ptr<HTTPHandler> handler = nullptr);

  void TearDown() override {
    evb_.loop();
    for (auto &fn : tearDownFns_) {
      fn();
    }
    if (isHQ()) {
      static const auto httpNoError =
          quic::ApplicationErrorCode(HTTP3::ErrorCode::HTTP_NO_ERROR);
      EXPECT_EQ(muxTransport_->socketDriver_.getConnErrorCode().value_or(
                    httpNoError) != httpNoError,
                bool(expectedError_) || bool(notExpectedError_));
      if (!expectedError_ && !notExpectedError_) {
        for (auto &stream : muxTransport_->socketDriver_.streams_) {
          XLOG(DBG2) << "Verifying stream=" << stream.first;
          EXPECT_TRUE(stream.second.unsentBuf.empty());
          EXPECT_TRUE(stream.second.pendingWriteBuf.empty());
        }
      }
    } else {
      if (notExpectedError_) {
        EXPECT_NE(transportState_.readError, notExpectedError_);
      } else {
        EXPECT_EQ(transportState_.readError, expectedError_);
      }
    }
  }

  void onTearDown(std::function<void()> fn) {
    tearDownFns_.emplace_back(std::move(fn));
  }

  void setTestCodecSetting(HTTPSettings *settings,
                           SettingsId id,
                           uint32_t value) {
    if (settings) {
      settings->setSetting(id, value);
    }
  }

  bool IS_H1() const {
    return GetParam().codecProtocol == CodecProtocol::HTTP_1_1;
  }

  bool isHQ() const {
    return GetParam().codecProtocol == CodecProtocol::HQ;
  }

  TransportDirection oppositeDirection() {
    return (direction_ == TransportDirection::UPSTREAM)
               ? TransportDirection::DOWNSTREAM
               : TransportDirection::UPSTREAM;
  }

  void flushQPACKEncoder() {
    if (isHQ()) {
      transport_->addReadEvent(muxTransport_->qpackEncoderStreamId_,
                               multiCodec_->getQPACKEncoderWriteBuf().move(),
                               false);
    }
  }

  void parseOutputUniplex() {
    XCHECK(!isHQ());
    folly::IOBufQueue parseOutputStream{folly::IOBufQueue::cacheChainLength()};
    for (auto &event : transportState_.writeEvents) {
      parseOutputStream.append(event.move());
      size_t consumed = 0;
      do {
        consumed = peerCodec_->onIngress(*parseOutputStream.front());
        parseOutputStream.trimStart(consumed);
      } while (consumed > 0 && !parseOutputStream.empty());
    }
    transportState_.writeEvents.clear();
    if (parseOutputStream.chainLength() > 0) {
      // replace unparsed output at the head of writeEvents
      transportState_.writeEvents.emplace_back(std::move(parseOutputStream));
    }
    if (transportState_.writesClosed) {
      peerCodec_->onIngressEOF();
    }
  }

  // stopSending controls whether H3 bidirectional streams get RST+SS or
  // just RST
  void resetStream(HTTPCodec::StreamID id,
                   ErrorCode code,
                   bool stopSending = true);
  void generateGoaway(HTTPCodec::StreamID id, ErrorCode code);
  void generateGoaway();
  void windowUpdate(HTTPCodec::StreamID id, uint32_t delta);
  void windowUpdate(uint32_t delta);
  void expectStreamAbort(HTTPCodec::StreamID id, ErrorCode code);
  void expectGoaway(HTTPCodec::StreamID id, ErrorCode code);

  TransportDirection direction_;
  TestHTTPTransport *transport_{nullptr};
  TestCoroTransport::State transportState_;
  std::unique_ptr<TestCoroMultiplexTransport> muxTransport_;
  FakeSessionStats fakeSessionStats_;
  HTTPCoroSession *session_{nullptr};
  testing::NiceMock<MockLifecycleObserver> lifecycleObs_;
  std::unique_ptr<HTTPCodec> peerCodec_;
  hq::HQMultiCodec *multiCodec_{nullptr};
  testing::NiceMock<MockHTTPCodecCallback> callbacks_;
  folly::IOBufQueue writeBuf_{folly::IOBufQueue::cacheChainLength()};
  folly::CancellationSource cancellationSource_;
  folly::Optional<folly::coro::TransportIf::ErrorCode> expectedError_;
  folly::Optional<folly::coro::TransportIf::ErrorCode> notExpectedError_;
  std::list<std::function<void()>> tearDownFns_;
  folly::EventBase evb_;
};

inline folly::coro::Task<HTTPBodyEvent> readBodyEventNoSuspend(
    HTTPSource &source, uint32_t max = std::numeric_limits<uint32_t>::max()) {
  while (true) {
    auto event = co_await co_awaitTry(source.readBodyEvent(max));
    if (event.hasException()) {
      co_yield folly::coro::co_error(std::move(event.exception()));
    }
    if (event->eventType == HTTPBodyEvent::EventType::SUSPEND) {
      co_await std::move(event->event.resume);
      continue;
    }
    co_return event;
  }
}

} // namespace proxygen::coro::test
