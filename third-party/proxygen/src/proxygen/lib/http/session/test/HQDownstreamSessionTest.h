/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBaseManager.h>
#include <proxygen/lib/http/codec/HQControlCodec.h>
#include <proxygen/lib/http/codec/HQStreamCodec.h>
#include <proxygen/lib/http/codec/HQUnidirectionalCodec.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/session/HQDownstreamSession.h>
#include <proxygen/lib/http/session/test/HQSessionMocks.h>
#include <proxygen/lib/http/session/test/HQSessionTestCommon.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <proxygen/lib/http/session/test/MockQuicSocketDriver.h>
#include <proxygen/lib/http/session/test/MockSessionObserver.h>
#include <proxygen/lib/http/session/test/TestUtils.h>
#include <quic/api/test/MockQuicSocket.h>
#include <wangle/acceptor/ConnectionManager.h>

#include <folly/futures/Future.h>
#include <folly/portability/GTest.h>

constexpr quic::StreamId kQPACKEncoderIngressStreamId = 6;
constexpr quic::StreamId kQPACKEncoderEgressStreamId = 7;
constexpr quic::StreamId kQPACKDecoderEgressStreamId = 11;

class TestTransportCallback
    : public proxygen::HTTPTransactionTransportCallback {
 public:
  void firstHeaderByteFlushed() noexcept override {
  }

  void firstByteFlushed() noexcept override {
  }

  void lastByteFlushed() noexcept override {
    lastByteFlushed_ = true;
  }

  void trackedByteFlushed() noexcept override {
  }

  void lastByteAcked(
      std::chrono::milliseconds /* latency */) noexcept override {
    lastByteAcked_ = true;
  }

  void headerBytesGenerated(proxygen::HTTPHeaderSize& size) noexcept override {
    headerBytesGenerated_ += size.compressedBlock;
  }

  void headerBytesReceived(
      const proxygen::HTTPHeaderSize& /* size */) noexcept override {
  }

  void bodyBytesGenerated(size_t nbytes) noexcept override {
    bodyBytesGenerated_ += nbytes;
  }

  void bodyBytesReceived(size_t /* size */) noexcept override {
  }

  void lastEgressHeaderByteAcked() noexcept override {
    lastEgressHeadersByteDelivered_ = true;
  }

  void bodyBytesTx(uint64_t bodyOffset) noexcept override {
    numBodyBytesTxCalls_++;
    bodyBytesTxOffset_ = bodyOffset;
  }

  void bodyBytesDelivered(uint64_t bodyOffset) noexcept override {
    numBodyBytesDeliveredCalls_++;
    bodyBytesDeliveredOffset_ = bodyOffset;
  }

  void bodyBytesDeliveryCancelled(uint64_t bodyOffset) noexcept override {
    numBodyBytesCanceledCalls_++;
    bodyBytesCanceledOffset_ = bodyOffset;
  }

  uint64_t headerBytesGenerated_{0};
  bool lastEgressHeadersByteDelivered_{false};
  uint64_t numBodyBytesDeliveredCalls_{0};
  uint64_t bodyBytesDeliveredOffset_{0};
  uint64_t numBodyBytesTxCalls_{0};
  uint64_t bodyBytesTxOffset_{0};
  uint64_t numBodyBytesCanceledCalls_{0};
  uint64_t bodyBytesCanceledOffset_{0};
  uint64_t bodyBytesGenerated_{0};
  bool lastByteFlushed_{false};
  bool lastByteAcked_{false};
};

class HQDownstreamSessionTest : public HQSessionTest {
 public:
  HQDownstreamSessionTest(
      folly::Optional<TestParams> overrideParams = folly::none)
      : HQSessionTest(proxygen::TransportDirection::DOWNSTREAM,
                      overrideParams) {
  }

 protected:
  proxygen::HTTPCodec::StreamID sendRequest(const std::string& url = "/",
                                            int8_t priority = 0,
                                            bool eom = true);

  quic::StreamId nextStreamId();

  quic::StreamId sendRequest(const proxygen::HTTPMessage& req,
                             bool eom = true,
                             quic::StreamId id = quic::kEightByteLimit);

  quic::StreamId sendHeader();

  folly::Promise<folly::Unit> sendRequestLater(proxygen::HTTPMessage req,
                                               bool eof = false);

 public:
  void SetUp() override;
  void TearDown() override;

 protected:
  void SetUpBase();
  void SetUpOnTransportReady();

  template <class HandlerType>
  std::unique_ptr<testing::StrictMock<HandlerType>>
  addSimpleStrictHandlerBase();

  std::unique_ptr<testing::StrictMock<proxygen::MockHTTPHandler>>
  addSimpleStrictHandler();

  std::pair<quic::StreamId,
            std::unique_ptr<testing::StrictMock<proxygen::MockHTTPHandler>>>
  checkRequest(proxygen::HTTPMessage req = proxygen::getGetRequest());

  void flushRequestsAndWaitForReads(
      bool eof = false,
      std::chrono::milliseconds eofDelay = std::chrono::milliseconds(0),
      std::chrono::milliseconds initialDelay = std::chrono::milliseconds(0),
      std::function<void()> extraEventsFn = std::function<void()>());

  void flushRequestsAndLoop(
      bool eof = false,
      std::chrono::milliseconds eofDelay = std::chrono::milliseconds(0),
      std::chrono::milliseconds initialDelay = std::chrono::milliseconds(0),
      std::function<void()> extraEventsFn = std::function<void()>());

  void flushRequestsAndLoopN(
      uint64_t n,
      bool eof = false,
      std::chrono::milliseconds eofDelay = std::chrono::milliseconds(0),
      std::chrono::milliseconds initialDelay = std::chrono::milliseconds(0),
      std::function<void()> extraEventsFn = std::function<void()>());

  bool flushRequests(
      bool eof = false,
      std::chrono::milliseconds eofDelay = std::chrono::milliseconds(0),
      std::chrono::milliseconds initialDelay = std::chrono::milliseconds(0),
      std::function<void()> extraEventsFn = std::function<void()>());

  testing::StrictMock<proxygen::MockController>& getMockController();

  std::unique_ptr<proxygen::HTTPCodec> makeCodec(
      proxygen::HTTPCodec::StreamID id);

  struct ClientStream {
    explicit ClientStream(std::unique_ptr<proxygen::HTTPCodec> c)
        : codec(std::move(c)) {
    }

    proxygen::HTTPCodec::StreamID id;
    folly::IOBufQueue buf{folly::IOBufQueue::cacheChainLength()};
    bool readEOF{false};
    std::unique_ptr<proxygen::HTTPCodec> codec;
  };

  ClientStream& getStream(proxygen::HTTPCodec::StreamID id);

  void expectTransactionTimeout(
      testing::StrictMock<proxygen::MockHTTPHandler>& handler,
      folly::Function<void()> fn = folly::Function<void()>());

  std::unique_ptr<proxygen::MockSessionObserver> setMockSessionObserver();

  std::unordered_map<quic::StreamId, ClientStream> requests_;
  quic::StreamId nextStreamId_{0};
  quic::QuicSocket::StreamTransportInfo streamTransInfo_;
  TestTransportCallback transportCallback_;
};

class HQDownstreamSessionBeforeTransportReadyTest
    : public HQDownstreamSessionTest {
  void SetUp() override {
    // Just do a basic setup, but don't call onTransportReady nor create the
    // control streams just yet, so to give the test a chance to manipulate
    // the session before onTransportReady
    SetUpBase();
  }
};
