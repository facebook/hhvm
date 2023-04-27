/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/session/HQSession.h>
#include <proxygen/lib/http/session/HQUnidirectionalCallbacks.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>
#include <quic/dsr/test/Mocks.h>

namespace proxygen {

class MockDispatcher : public HQUnidirStreamDispatcher::Callback {
 public:
  using PeekCallback = quic::QuicSocket::PeekCallback;
  using ReadCallback = quic::QuicSocket::ReadCallback;
  using PeekCallbackAssignF = std::function<void(quic::StreamId,
                                                 hq::UnidirectionalStreamType,
                                                 size_t,
                                                 PeekCallback* const)>;
  using ReadCallbackAssignF = std::function<void(quic::StreamId,
                                                 hq::UnidirectionalStreamType,
                                                 size_t,
                                                 ReadCallback* const)>;
  using PrefaceParseF =
      std::function<folly::Optional<hq::UnidirectionalStreamType>(uint64_t)>;
  using StreamRejectF = std::function<void(quic::StreamId)>;
  using StreamReadF = std::function<void(quic::StreamId)>;
  using StreamInspectF = std::function<bool(quic::StreamId)>;
  using ErrorStreamF = std::function<void(quic::StreamId, const ReadError&)>;
  using StreamPeekF = std::function<void(quic::StreamId, const PeekData& x)>;
  using UsePeekApiF = std::function<bool()>;
  using NewPushStreamF =
      std::function<void(quic::StreamId, hq::PushId, size_t)>;
  using PRSeekF = std::function<void(quic::StreamId, uint64_t)>;

  void expectOnNewPushStream(NewPushStreamF impl = nullptr) {
    auto& exp = EXPECT_CALL(
        *this, onNewPushStream(::testing::_, ::testing::_, ::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectAssignPeekCallback(PeekCallbackAssignF impl = nullptr) {
    auto& exp = EXPECT_CALL(
        *this,
        assignPeekCallback(
            ::testing::_, ::testing::_, ::testing::_, ::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectAssignReadCallback(ReadCallbackAssignF impl = nullptr) {
    auto& exp = EXPECT_CALL(
        *this,
        assignReadCallback(
            ::testing::_, ::testing::_, ::testing::_, ::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectParsePreface(PrefaceParseF impl = nullptr) {
    auto& exp = EXPECT_CALL(*this, parseStreamPreface(::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectRejectStream(StreamRejectF impl = nullptr) {
    auto& exp = EXPECT_CALL(*this, rejectStream(::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectUnidirectionalReadAvailable(StreamReadF impl = nullptr) {
    auto& exp = EXPECT_CALL(*this, controlStreamReadAvailable(::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectUnidirectionalReadError(ErrorStreamF impl = nullptr) {
    auto& exp =
        EXPECT_CALL(*this, controlStreamReadError(::testing::_, ::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectPartialDataAvailable(StreamPeekF impl = nullptr) {
    auto& exp =
        EXPECT_CALL(*this, onPartialDataAvailable(::testing::_, ::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  MOCK_METHOD(void, onNewPushStream, (quic::StreamId, hq::PushId, size_t));
  MOCK_METHOD(void,
              assignReadCallback,
              (quic::StreamId,
               hq::UnidirectionalStreamType,
               size_t,
               quic::QuicSocket::ReadCallback* const));
  MOCK_METHOD(void,
              assignPeekCallback,
              (quic::StreamId,
               hq::UnidirectionalStreamType,
               size_t,
               quic::QuicSocket::PeekCallback* const));
  MOCK_METHOD(folly::Optional<hq::UnidirectionalStreamType>,
              parseStreamPreface,
              (uint64_t));
  MOCK_METHOD(void, rejectStream, (quic::StreamId));
  MOCK_METHOD(void, controlStreamReadAvailable, (quic::StreamId));
  MOCK_METHOD(void, controlStreamReadError, (quic::StreamId, const ReadError&));
  MOCK_METHOD(void, onPartialDataAvailable, (quic::StreamId, const PeekData&));
  MOCK_METHOD(void, processExpiredData, (quic::StreamId, uint64_t));
  MOCK_METHOD(void, processRejectedData, (quic::StreamId, uint64_t));
};

class MockServerPushLifecycleCallback : public ServerPushLifecycleCallback {
 public:
  virtual ~MockServerPushLifecycleCallback() = default;

  MOCK_METHOD(void,
              onPushPromiseBegin,
              (HTTPCodec::StreamID /* parent streamID */,
               hq::PushId /* pushID */));

  MOCK_METHOD(void,
              onPushPromise,
              (HTTPCodec::StreamID /* parent streamID */,
               hq::PushId /* pushID */,
               HTTPMessage* /* msg */));

  MOCK_METHOD(void,
              onNascentPushStreamBegin,
              (HTTPCodec::StreamID /* push stream ID */, bool /* eom */));

  MOCK_METHOD(void,
              onNascentPushStream,
              (HTTPCodec::StreamID /* push stream ID */,
               hq::PushId /* server push id */,
               bool /* eom */));

  MOCK_METHOD(void,
              onNascentEof,
              (HTTPCodec::StreamID /* push stream ID */,
               folly::Optional<hq::PushId> /* push id */));

  MOCK_METHOD(void,
              onOrphanedNascentStream,
              (HTTPCodec::StreamID /* push stream ID */,
               folly::Optional<hq::PushId> /* push id */));

  MOCK_METHOD(void,
              onHalfOpenPushedTxn,
              (const HTTPTransaction* /* txn */,
               hq::PushId /* push id */,
               HTTPCodec::StreamID /* assoc stream id */,
               bool /* eom */));

  MOCK_METHOD(void,
              onPushedTxn,
              (const HTTPTransaction* /* txn */,
               HTTPCodec::StreamID /* push stream id */,
               hq::PushId /* push id */,
               HTTPCodec::StreamID /* assoc stream id */,
               bool /* eom */));

  MOCK_METHOD(void, onPushedTxnTimeout, (const HTTPTransaction* /* txn */));

  MOCK_METHOD(void,
              onOrphanedHalfOpenPushedTxn,
              (const HTTPTransaction* /* txn */));

  MOCK_METHOD(void,
              onPushIdLimitExceeded,
              (hq::PushId /* incoming push id */,
               folly::Optional<hq::PushId> /* max allowed push id */,
               folly::Optional<HTTPCodec::StreamID> /* stream */));

  using PushPromiseBeginF =
      std::function<void(HTTPCodec::StreamID, hq::PushId)>;
  using PushPromiseF =
      std::function<void(HTTPCodec::StreamID, hq::PushId, HTTPMessage*)>;
  using NascentPushStreamBeginF =
      std::function<void(HTTPCodec::StreamID, bool)>;
  using NascentPushStreamF =
      std::function<void(HTTPCodec::StreamID, hq::PushId, bool)>;
  using NascentEofF =
      std::function<void(HTTPCodec::StreamID, folly::Optional<hq::PushId>)>;
  using OrphanedNascentStreamF =
      std::function<void(HTTPCodec::StreamID, folly::Optional<hq::PushId>)>;
  using HalfOpenPushedTxnF = std::function<void(
      const HTTPTransaction*, hq::PushId, HTTPCodec::StreamID, bool)>;
  using PushedTxnF = std::function<void(const HTTPTransaction*,
                                        HTTPCodec::StreamID,
                                        hq::PushId,
                                        HTTPCodec::StreamID,
                                        bool)>;
  using PushedTxnTimeoutF = std::function<void(const HTTPTransaction*)>;
  using OrphanedHalfOpenPushedTxnF =
      std::function<void(const HTTPTransaction*)>;

  using PushIdLimitExceededF =
      std::function<void(hq::PushId,
                         folly::Optional<hq::PushId>,
                         folly::Optional<HTTPCodec::StreamID>)>;

  void expectPushPromiseBegin(PushPromiseBeginF impl = nullptr) {
    auto& exp = EXPECT_CALL(*this, onPushPromiseBegin(testing::_, testing::_));
    if (impl) {
      exp.WillOnce(testing::Invoke(impl));
    }
  }

  void expectPushPromise(PushPromiseF impl = nullptr) {
    auto& exp =
        EXPECT_CALL(*this, onPushPromise(testing::_, testing::_, testing::_));
    if (impl) {
      exp.WillOnce(testing::Invoke(impl));
    }
  }

  void expectNascentPushStreamBegin(NascentPushStreamBeginF impl = nullptr) {
    auto& exp =
        EXPECT_CALL(*this, onNascentPushStreamBegin(testing::_, testing::_));
    if (impl) {
      exp.WillOnce(testing::Invoke(impl));
    }
  }

  void expectNascentPushStream(NascentPushStreamF impl = nullptr) {
    auto& exp = EXPECT_CALL(
        *this, onNascentPushStream(testing::_, testing::_, testing::_));
    if (impl) {
      exp.WillOnce(testing::Invoke(impl));
    }
  }

  void expectNascentEof(NascentEofF impl = nullptr) {
    auto& exp = EXPECT_CALL(*this, onNascentEof(testing::_, testing::_));
    if (impl) {
      exp.WillOnce(testing::Invoke(impl));
    }
  }

  void expectOrphanedNascentStream(OrphanedNascentStreamF impl = nullptr) {
    auto& exp =
        EXPECT_CALL(*this, onOrphanedNascentStream(testing::_, testing::_));
    if (impl) {
      exp.WillOnce(testing::Invoke(impl));
    }
  }

  void expectHalfOpenPushedTxn(HalfOpenPushedTxnF impl = nullptr) {
    auto& exp = EXPECT_CALL(
        *this,
        onHalfOpenPushedTxn(testing::_, testing::_, testing::_, testing::_));
    if (impl) {
      exp.WillOnce(testing::Invoke(impl));
    }
  }

  void expectPushedTxn(PushedTxnF impl = nullptr) {
    auto& exp = EXPECT_CALL(
        *this,
        onPushedTxn(
            testing::_, testing::_, testing::_, testing::_, testing::_));
    if (impl) {
      exp.WillOnce(testing::Invoke(impl));
    }
  }

  void expectPushedTxnTimeout(PushedTxnTimeoutF impl = nullptr) {
    auto& exp = EXPECT_CALL(*this, onPushedTxnTimeout(testing::_));
    if (impl) {
      exp.WillOnce(testing::Invoke(impl));
    }
  }

  void expectOrphanedHalfOpenPushedTxn(
      OrphanedHalfOpenPushedTxnF impl = nullptr) {
    auto& exp = EXPECT_CALL(*this, onOrphanedHalfOpenPushedTxn(testing::_));
    if (impl) {
      exp.WillOnce(testing::Invoke(impl));
    }
  }

  void expectPushIdLimitExceeded(PushIdLimitExceededF impl = nullptr) {
    auto& exp = EXPECT_CALL(
        *this, onPushIdLimitExceeded(testing::_, testing::_, testing::_));
    if (impl) {
      exp.WillOnce(testing::Invoke(impl));
    }
  }
};

class MockConnectCallback : public HQSession::ConnectCallback {
 public:
  MOCK_METHOD(void, connectSuccess, ());
  MOCK_METHOD(void, onReplaySafe, ());
  MOCK_METHOD(void, connectError, (quic::QuicError));
  MOCK_METHOD(void, onFirstPeerPacketProcessed, ());
};

class MockHQSession : public HQSession {
 public:
  MockHQSession(const folly::Optional<std::chrono::milliseconds>&
                    transactionsTimeout = folly::none,
                HTTPSessionController* controller = nullptr,
                const folly::Optional<proxygen::TransportDirection>& direction =
                    folly::none)
      : HQSession(transactionsTimeout.value_or(getDefaultTransactionTimeout()),
                  controller,
                  direction.value_or(getMockDefaultDirection()),
                  wangle::TransportInfo(),
                  nullptr),
        transactionTimeout_(
            transactionsTimeout.value_or(getDefaultTransactionTimeout())),
        direction_(direction.value_or(getMockDefaultDirection())),
        quicProtocolInfo_(std::make_shared<QuicProtocolInfo>()),
        quicStreamProtocolInfo_(std::make_shared<QuicStreamProtocolInfo>()) {
    LOG(INFO) << "Creating mock transaction on stream " << lastStreamId_;
    makeMockTransaction(lastStreamId_++);

    ON_CALL(*this, newTransaction(::testing::_))
        .WillByDefault(::testing::DoAll(
            ::testing::SaveArg<0>(&handler_),
            ::testing::WithArgs<0>(
                ::testing::Invoke([&](HTTPTransaction::Handler* handler) {
                  CHECK(txn_);
                  LOG(INFO) << "Setting transaction handler to " << handler;
                  txn_->HTTPTransaction::setHandler(handler);
                })),
            ::testing::Return(txn_.get())));
  }

  static std::chrono::milliseconds getDefaultTransactionTimeout() {
    return std::chrono::milliseconds(5000);
  }

  static proxygen::TransportDirection getMockDefaultDirection() {
    return proxygen::TransportDirection::UPSTREAM;
  }

  bool isDetachable(bool) const override {
    return false;
  }

  void attachThreadLocals(folly::EventBase*,
                          folly::SSLContextPtr,
                          const WheelTimerInstance&,
                          HTTPSessionStats*,
                          FilterIteratorFn,
                          HeaderCodec::Stats*,
                          HTTPSessionController*) override {
  }

  void detachThreadLocals(bool) override {
  }

  void onHeadersComplete(HTTPCodec::StreamID streamID,
                         std::unique_ptr<HTTPMessage> msg,
                         bool eom = false) {
    if (handler_) {
      handler_->onHeadersComplete(std::move(msg));
      if (eom) {
        handler_->onEOM();
      }
    }
  };

  void onHeadersComplete(HTTPCodec::StreamID streamID,
                         int statusCode,
                         const std::string& statusMessage,
                         bool eom = false) {
    auto resp = std::make_unique<HTTPMessage>();
    resp->setStatusCode(statusCode);
    resp->setStatusMessage(statusMessage);
    onHeadersComplete(streamID, std::move(resp), eom);
  }

  MOCK_METHOD(bool, isReplaySafe, (), (const));

  MOCK_METHOD(HTTPTransaction::Handler*,
              getTransactionTimeoutHandler,
              (HTTPTransaction*));

  MOCK_METHOD(void, setupOnHeadersComplete, (HTTPTransaction*, HTTPMessage*));

  MOCK_METHOD((void),
              onConnectionSetupErrorHandler,
              (quic::QuicError),
              (noexcept));

  MOCK_METHOD(HTTPTransaction*, newTransaction, (HTTPTransaction::Handler*));

  MOCK_METHOD(void, drain, ());

  MOCK_CONST_METHOD0(getQuicSocket, quic::QuicSocket*());

  MockHTTPTransaction* makeMockTransaction(HTTPCodec::StreamID id) {
    LOG(INFO) << "Creating mocked transaction on stream " << id;

    txn_ = std::make_unique<::testing::StrictMock<MockHTTPTransaction>>(
        direction_,
        id,
        0, /* seqNo */
        egressQueue_,
        nullptr, /* timer */
        transactionTimeout_);

    LOG(INFO) << "Setting default handlers on the new transaction "
              << txn_.get();

    EXPECT_CALL(*txn_, setHandler(::testing::_))
        .WillRepeatedly(
            ::testing::Invoke([txn = txn_.get()](HTTPTransactionHandler* hdlr) {
              LOG(INFO) << "Setting handler on " << txn << " to " << hdlr;
              txn->HTTPTransaction::setHandler(hdlr);
            }));

    EXPECT_CALL(*txn_, canSendHeaders())
        .WillRepeatedly(::testing::Invoke([txn = txn_.get()] {
          return txn->HTTPTransaction::canSendHeaders();
        }));

    EXPECT_CALL(txn_->mockTransport_, getCurrentTransportInfo(::testing::_))
        .WillRepeatedly(::testing::DoAll(
            ::testing::WithArgs<0>(
                ::testing::Invoke([&](wangle::TransportInfo* tinfo) {
                  if (tinfo) {
                    tinfo->protocolInfo = quicStreamProtocolInfo_;
                  }
                })),
            ::testing::Return(true)));

    LOG(INFO) << "Returning the new mocked transaction " << txn_.get();

    return txn_.get();
  }

  HQStreamTransportBase* findPushStream(quic::StreamId) override {
    return nullptr;
  }

  void findPushStreams(std::unordered_set<HQStreamTransportBase*>&) override {
  }
  bool erasePushStream(quic::StreamId) override {
    return false;
  }
  uint32_t getNumOutgoingStreams() const override {
    return static_cast<uint32_t>(streams_.size());
  }
  uint32_t getNumIncomingStreams() const override {
    return static_cast<uint32_t>(streams_.size());
  }

  void onNewPushStream(quic::StreamId /* streamId */,
                       hq::PushId /* pushId */,
                       size_t /* to consume */) override{};

  const std::chrono::milliseconds transactionTimeout_;
  const proxygen::TransportDirection direction_;

  HTTP2PriorityQueue egressQueue_;
  wangle::TransportInfo currentTransportInfo_;
  std::shared_ptr<QuicProtocolInfo> quicProtocolInfo_;
  std::shared_ptr<QuicStreamProtocolInfo> quicStreamProtocolInfo_;

  std::unique_ptr<::testing::StrictMock<MockHTTPTransactionTransport>>
      transport_;

  std::unique_ptr<::testing::StrictMock<MockHTTPTransaction>> txn_;

  HTTPCodec::StreamID lastStreamId_{1}; // streamID 0 is reserved
  HTTPTransaction::Handler* handler_;
};

class FakeHQHTTPCodecCallback : public FakeHTTPCodecCallback {
 public:
};

class MockQuicDSRRequestSender
    : public MockDSRRequestSender
    , public quic::test::MockDSRPacketizationRequestSender {
 public:
  MockQuicDSRRequestSender() = default;
};

} // namespace proxygen
