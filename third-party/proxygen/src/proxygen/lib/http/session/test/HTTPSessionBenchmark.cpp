/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Benchmark for per-transaction operations in HTTPSession.
//
// Measures the cost of checkIfEgressRateLimitedByUpstream() across N
// concurrent transactions in a real HTTPDownstreamSession, useful for
// detecting performance regressions in the egress rate-limiting path.

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/test/MockHTTPCodec.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/session/HTTPDownstreamSession.h>
#include <proxygen/lib/http/session/HTTPSession.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>

#include <proxygen/lib/http/session/test/TestUtils.h>

using namespace proxygen;
using namespace testing;
using namespace folly;

namespace {

// Minimal transport callback matching production behavior.
class BenchTransportCallback : public HTTPTransaction::TransportCallback {
 public:
  void firstHeaderByteFlushed() noexcept override {
  }
  void firstByteFlushed() noexcept override {
  }
  void trackedByteFlushed() noexcept override {
  }
  void lastByteFlushed() noexcept override {
  }
  void lastByteAcked(std::chrono::milliseconds) noexcept override {
  }
  void trackedByteEventTX(const ByteEvent&) noexcept override {
  }
  void trackedByteEventAck(const ByteEvent&) noexcept override {
  }
  void egressBufferEmpty() noexcept override {
    egressBufferEmptyCalls_++;
  }
  void headerBytesGenerated(HTTPHeaderSize&) noexcept override {
  }
  void headerBytesReceived(const HTTPHeaderSize&) noexcept override {
  }
  void bodyBytesGenerated(size_t) noexcept override {
  }
  void bodyBytesReceived(size_t) noexcept override {
  }
  void transportAppRateLimited() noexcept override {
  }
  void datagramBytesGenerated(size_t) noexcept override {
  }
  void datagramBytesReceived(size_t) noexcept override {
  }

  uint64_t egressBufferEmptyCalls_{0};
};

// Manages the lifecycle of a downstream HTTP/2 session with N concurrent
// transactions. Uses NiceMock<MockHTTPCodec> + NiceMock<MockAsyncTransport>
// to avoid real I/O. Transactions are configured in the state that triggers
// egressBufferEmpty(): transportCallback_ set, EOM not queued, egress body
// buffer empty.
class SessionBenchmarkHelper {
 public:
  explicit SessionBenchmarkHelper(uint32_t numTransactions)
      : numTransactions_(numTransactions) {
    setup();
  }

  ~SessionBenchmarkHelper() {
    teardown();
  }

  // Call checkIfEgressRateLimitedByUpstream() on every transaction.
  void invokeCheckOnAllTransactions() {
    for (auto& handler : handlers_) {
      if (handler->txn_) {
        handler->txn_->checkIfEgressRateLimitedByUpstream();
      }
    }
  }

 private:
  void setup() {
    codec_ = new NiceMock<MockHTTPCodec>();
    transport_ = new NiceMock<folly::test::MockAsyncTransport>();
    transactionTimeouts_ = makeTimeoutSet(&eventBase_);

    ON_CALL(*transport_, good()).WillByDefault(Return(true));
    ON_CALL(*transport_, getEventBase()).WillByDefault(Return(&eventBase_));
    ON_CALL(*transport_, setReadCB(_)).WillByDefault(SaveArg<0>(&transportCb_));
    ON_CALL(*transport_, writeChain(_, _, _))
        .WillByDefault([](folly::AsyncTransport::WriteCallback* cb,
                          std::shared_ptr<folly::IOBuf>,
                          folly::WriteFlags) { cb->writeSuccess(); });

    ON_CALL(mockController_, getGracefulShutdownTimeout())
        .WillByDefault(Return(std::chrono::milliseconds(0)));
    ON_CALL(mockController_, getHeaderIndexingStrategy())
        .WillByDefault(Return(HeaderIndexingStrategy::getDefaultInstance()));
    EXPECT_CALL(mockController_, attachSession(_)).Times(1);
    EXPECT_CALL(mockController_, onTransportReady(_)).Times(1);

    ON_CALL(*codec_, setCallback(_)).WillByDefault(SaveArg<0>(&codecCallback_));
    ON_CALL(*codec_, supportsParallelRequests()).WillByDefault(Return(true));
    ON_CALL(*codec_, supportsPushTransactions()).WillByDefault(Return(true));
    ON_CALL(*codec_, getTransportDirection())
        .WillByDefault(Return(TransportDirection::DOWNSTREAM));
    ON_CALL(*codec_, supportsStreamFlowControl()).WillByDefault(Return(true));
    ON_CALL(*codec_, getProtocol())
        .WillByDefault(Return(CodecProtocol::HTTP_2));
    ON_CALL(*codec_, supportsSessionFlowControl()).WillByDefault(Return(true));
    ON_CALL(*codec_, getIngressSettings())
        .WillByDefault(Return(&ingressSettings_));
    ON_CALL(*codec_, isReusable()).WillByDefault(Return(true));
    ON_CALL(*codec_, isWaitingToDrain()).WillByDefault(Return(false));
    ON_CALL(*codec_, getDefaultWindowSize())
        .WillByDefault(Return(http2::kInitialWindow));
    ON_CALL(*codec_, mapPriorityToDependency(_)).WillByDefault(Return(0));
    ON_CALL(*codec_, setParserPaused(_)).WillByDefault(Return());

    fakeMockCodec(*codec_);

    HTTPSession::setDefaultReadBufferLimit(65536);
    HTTPTransaction::setEgressBufferLimit(65536);
    httpSession_ =
        new HTTPDownstreamSession(transactionTimeouts_.get(),
                                  folly::AsyncTransport::UniquePtr(transport_),
                                  localAddr,
                                  peerAddr,
                                  &mockController_,
                                  std::unique_ptr<HTTPCodec>(codec_),
                                  mockTransportInfo,
                                  nullptr);
    httpSession_->setEgressSettings(
        {{SettingsId::MAX_CONCURRENT_STREAMS, numTransactions_ + 100}});
    httpSession_->startNow();
    eventBase_.loop();

    handlers_.reserve(numTransactions_);
    transportCallbacks_.reserve(numTransactions_);
    for (uint32_t i = 0; i < numTransactions_; i++) {
      handlers_.push_back(std::make_unique<NiceMock<MockHTTPHandler>>());
      transportCallbacks_.push_back(std::make_unique<BenchTransportCallback>());
    }

    for (uint32_t i = 0; i < numTransactions_; i++) {
      HTTPCodec::StreamID streamID = i * 2 + 1;
      auto req = makeGetRequest();

      EXPECT_CALL(mockController_, getRequestHandler(_, _))
          .WillOnce(Return(handlers_[i].get()))
          .RetiresOnSaturation();
      EXPECT_CALL(*handlers_[i], _setTransaction(_))
          .WillOnce([this, i](HTTPTransaction* txn) {
            handlers_[i]->txn_ = txn;
            txn->setTransportCallback(transportCallbacks_[i].get());
          });
      EXPECT_CALL(*handlers_[i], _onHeadersComplete(_));

      codecCallback_->onMessageBegin(streamID, req.get());
      codecCallback_->onHeadersComplete(streamID, std::move(req));
    }

    eventBase_.loop();
  }

  void teardown() {
    for (auto& handler : handlers_) {
      if (handler->txn_) {
        EXPECT_CALL(*handler, _onError(_)).WillOnce(Return());
        EXPECT_CALL(*handler, _detachTransaction()).WillOnce(Return());
      }
    }
    EXPECT_CALL(mockController_, detachSession(_)).Times(AtMost(1));
    httpSession_->dropConnection();
  }

  uint32_t numTransactions_;
  EventBase eventBase_;
  NiceMock<MockHTTPCodec>* codec_{nullptr};
  NiceMock<folly::test::MockAsyncTransport>* transport_{nullptr};
  folly::AsyncTransport::ReadCallback* transportCb_{nullptr};
  folly::HHWheelTimer::UniquePtr transactionTimeouts_;
  NiceMock<MockController> mockController_;
  HTTPCodec::Callback* codecCallback_{nullptr};
  HTTPDownstreamSession* httpSession_{nullptr};
  HTTPSettings ingressSettings_{
      {SettingsId::INITIAL_WINDOW_SIZE, http2::kInitialWindow}};

  std::vector<std::unique_ptr<NiceMock<MockHTTPHandler>>> handlers_;
  std::vector<std::unique_ptr<BenchTransportCallback>> transportCallbacks_;
};

// Measures the per-iteration cost of calling
// checkIfEgressRateLimitedByUpstream() on all N transactions in a real
// HTTPDownstreamSession, exercising the full code path including condition
// checks and the egressBufferEmpty() callback.
void benchCheckEgressRateLimited(uint32_t iters, uint32_t numTransactions) {
  SessionBenchmarkHelper* helper = nullptr;

  BENCHMARK_SUSPEND {
    helper = new SessionBenchmarkHelper(numTransactions);
  }

  for (uint32_t iter = 0; iter < iters; iter++) {
    helper->invokeCheckOnAllTransactions();
  }

  folly::doNotOptimizeAway(helper);

  BENCHMARK_SUSPEND {
    delete helper;
  }
}

} // namespace

BENCHMARK_NAMED_PARAM(benchCheckEgressRateLimited, 10_txns, 10)
BENCHMARK_NAMED_PARAM(benchCheckEgressRateLimited, 100_txns, 100)
BENCHMARK_NAMED_PARAM(benchCheckEgressRateLimited, 500_txns, 500)
BENCHMARK_NAMED_PARAM(benchCheckEgressRateLimited, 1000_txns, 1000)

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
