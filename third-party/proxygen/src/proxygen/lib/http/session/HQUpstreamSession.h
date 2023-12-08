/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <proxygen/lib/http/session/HQSession.h>

#include <folly/io/async/HHWheelTimer.h>

namespace proxygen {

class HQUpstreamSession : public HQSession {
  class ConnectTimeout;

  enum class ConnCallbackState { NONE, CONNECT_SUCCESS, REPLAY_SAFE, DONE };

 public:
  HQUpstreamSession(const std::chrono::milliseconds transactionsTimeout,
                    const std::chrono::milliseconds connectTimeoutMs,
                    HTTPSessionController* controller,
                    const wangle::TransportInfo& tinfo,
                    InfoCallback* sessionInfoCb)
      : HQSession(transactionsTimeout,
                  controller,
                  proxygen::TransportDirection::UPSTREAM,
                  tinfo,
                  sessionInfoCb),
        connectTimeoutMs_(connectTimeoutMs),
        connectTimeout_(*this) {
  }

  void setConnectCallback(ConnectCallback* connectCb) noexcept {
    connectCb_ = connectCb;
  }

  void connectSuccess() noexcept override;

  /**
   * Returns true if the underlying transport has completed full handshake.
   */
  bool isReplaySafe() const override {
    return sock_ ? sock_->replaySafe() : false;
  }

  void onConnectionEnd() noexcept override;

  void startNow() override;

  void onTransportReady() noexcept override;

  void onReplaySafe() noexcept override;

  void onFirstPeerPacketProcessed() noexcept override;

  void handleReplaySafe() noexcept;

  HTTPTransaction::Handler* getTransactionTimeoutHandler(
      HTTPTransaction* /* txn */) override {
    // No special handler for upstream requests that time out
    return nullptr;
  }

  void setupOnHeadersComplete(HTTPTransaction* /* txn */,
                              HTTPMessage* /* msg */) override {
  }

  void onConnectionSetupErrorHandler(quic::QuicError code) noexcept override;

  bool isDetachable(bool checkSocket) const override;

  void attachThreadLocals(folly::EventBase*,
                          std::shared_ptr<const folly::SSLContext>,
                          const WheelTimerInstance&,
                          HTTPSessionStats*,
                          FilterIteratorFn,
                          HeaderCodec::Stats*,
                          HTTPSessionController*) override;

  void detachThreadLocals(bool) override;

  /**
   * Returns true iff a new outgoing transaction can be made on this session
   */
  bool supportsMoreTransactions() const override {
    return sock_ && sock_->getNumOpenableBidirectionalStreams() &&
           HTTPSessionBase::supportsMoreTransactions();
  }

  uint32_t getNumOutgoingStreams() const override {
    // need transport API
    return static_cast<uint32_t>(streams_.size());
  }

  uint32_t getNumIncomingStreams() const override {
    // need transport API
    return static_cast<uint32_t>(numberOfIngressPushStreams());
  }

 protected:
  ~HQUpstreamSession() override;

 private:
  void connectTimeoutExpired() noexcept;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250) // inherits 'proxygen::detail::..' via dominance
#endif

  /**
   * Client-side representation of a push stream.
   * Does not support egress operations.
   */
  class HQIngressPushStream
      : public detail::singlestream::SSIngress
      , public HQSession::HQStreamTransportBase {
   public:
    HQIngressPushStream(HQSession& session,
                        hq::PushId pushId,
                        folly::Optional<HTTPCodec::StreamID> parentTxnId,
                        uint32_t seqNo,
                        const WheelTimerInstance& timeout,
                        HTTPSessionStats* stats = nullptr,
                        http2::PriorityUpdate priority = hqDefaultPriority)
        : detail::singlestream::SSIngress(folly::none),
          HQStreamTransportBase(session,
                                TransportDirection::UPSTREAM,
                                static_cast<HTTPCodec::StreamID>(pushId),
                                seqNo,
                                timeout,
                                stats,
                                priority,
                                parentTxnId,
                                hq::UnidirectionalStreamType::PUSH),
          pushId_(pushId) {
      // Ingress push streams are not initialized
      // until after the nascent push stream
      // has been received
      // notify the testing callbacks that a half-opened push transaction
      // has been created

      // NOTE: change the API to avoid accepting parent txn ids
      // as optional
      CHECK(parentTxnId.has_value());
      auto cb = ((HQUpstreamSession&)session_).serverPushLifecycleCb_;
      if (cb) {
        cb->onHalfOpenPushedTxn(&txn_, pushId, *parentTxnId, false);
      }
    }

    // Bind this stream to a transport stream
    void bindTo(quic::StreamId transportStreamId);

    void onPushMessageBegin(HTTPCodec::StreamID pushId,
                            HTTPCodec::StreamID parentTxnId,
                            HTTPMessage* /* msg */) override {
      LOG(ERROR) << "Push promise on push stream"
                 << " txn=" << txn_ << " pushID=" << pushId
                 << " parentTxnId=" << parentTxnId;
      session_.dropConnectionAsync(
          quic::QuicError(HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED,
                          "Push promise on push stream"),
          kErrorConnection);
    }

    // Abort procedure that is specific to ingress push streams.
    size_t sendAbort(HTTPTransaction* txn,
                     ErrorCode errorCode) noexcept override {
      // TBD: send "cancel push" here.

      return sendAbortImpl(
          toHTTP3ErrorCode(errorCode),
          folly::to<std::string>("Application aborts pushed txn,"
                                 " errorCode=",
                                 getErrorCodeString(errorCode),
                                 " pushID=",
                                 getPushId(),
                                 " txn=",
                                 txn->getID(),
                                 " hasIngressStream=",
                                 hasIngressStreamId()));
    }

    hq::PushId getPushId() const {
      return pushId_;
    }

   private:
    hq::PushId pushId_; // The push id in context of which this stream is
                        // received
  };                    // HQIngressPushStream

#ifdef _MSC_VER
#pragma warning(pop)
#endif

  // Find an ingress push stream
  HQIngressPushStream* findIngressPushStream(quic::StreamId);
  HQIngressPushStream* findIngressPushStreamByPushId(hq::PushId);

  uint32_t numberOfIngressPushStreams() const;

  /**
   * Attempt to bind an ingress push stream object (which has the txn)
   * to a nascent stream (which has the transport/codec).
   * returns true if binding was successful
   */
  bool tryBindIngressStreamToTxn(quic::StreamId streamID,
                                 hq::PushId pushId,
                                 HQIngressPushStream* pushStream = nullptr);

  // Create ingress push stream.
  HQStreamTransportBase* createIngressPushStream(quic::StreamId parentStreamId,
                                                 hq::PushId pushId) override;

  HQStreamTransportBase* findPushStream(quic::StreamId id) override;

  void findPushStreams(
      std::unordered_set<HQStreamTransportBase*>& streams) override {
    for (auto& pstream : ingressPushStreams_) {
      streams.insert(&pstream.second);
    }
  }

  bool erasePushStream(quic::StreamId streamId) override;

  void eraseUnboundStream(HQStreamTransportBase*) override;

  void cleanupUnboundPushStreams(std::vector<quic::StreamId>&) override;

  void dispatchPushStream(quic::StreamId /* pushStreamId */,
                          hq::PushId /* pushId */,
                          size_t /* toConsume */) override;

  // Incoming server push streams. Since the incoming push streams
  // can be created before transport stream
  std::unordered_map<hq::PushId, HQIngressPushStream> ingressPushStreams_;

  /**
   * The Session notifies when an upstream 'connection' has been established
   * and it is possible to start creating new streams / sending data
   * The connection callback only expects either success or error
   * so it gets automatically reset to nullptr after the first invocation
   */
  ConnectCallback* connectCb_{nullptr};

  class ConnectTimeout : public folly::HHWheelTimer::Callback {
   public:
    explicit ConnectTimeout(HQUpstreamSession& session) : session_(session) {
    }

    void timeoutExpired() noexcept override {
      session_.connectTimeoutExpired();
    }

   private:
    HQUpstreamSession& session_;
  };

  std::chrono::milliseconds connectTimeoutMs_;
  ConnectTimeout connectTimeout_;
  ConnCallbackState connCbState_{ConnCallbackState::NONE};
};

} // namespace proxygen
