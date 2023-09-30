/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/session/HQSession.h>

namespace proxygen {

class HQDownstreamSession : public HQSession {
 public:
  HQDownstreamSession(const std::chrono::milliseconds transactionsTimeout,
                      HTTPSessionController* controller,
                      const wangle::TransportInfo& tinfo,
                      InfoCallback* sessionInfoCb)
      : HQSession(transactionsTimeout,
                  controller,
                  proxygen::TransportDirection::DOWNSTREAM,
                  tinfo,
                  sessionInfoCb) {
  }

  void onTransportReady() noexcept override;

  void onFullHandshakeDone() noexcept override;

  void onAppRateLimited() noexcept override;

  HTTPTransaction::Handler* getTransactionTimeoutHandler(
      HTTPTransaction* txn) override {
    return getController()->getTransactionTimeoutHandler(txn,
                                                         getLocalAddress());
  }

  void setupOnHeadersComplete(HTTPTransaction* txn, HTTPMessage* msg) override;

  void onConnectionSetupErrorHandler(quic::QuicError) noexcept override;

  bool isDetachable(bool) const override;

  void attachThreadLocals(folly::EventBase*,
                          folly::SSLContextPtr,
                          const WheelTimerInstance&,
                          HTTPSessionStats*,
                          FilterIteratorFn,
                          HeaderCodec::Stats*,
                          HTTPSessionController*) override;

  void detachThreadLocals(bool) override;

  bool isReplaySafe() const override {
    LOG(FATAL) << __func__ << " is an upstream interface";
    return false;
  }
  // Create a new pushed transaction.
  HTTPTransaction* newPushedTransaction(
      HTTPCodec::StreamID,           /* parentRequestStreamId */
      HTTPTransaction::PushHandler*, /* handler */
      ProxygenError* error = nullptr) override;

  /**
   * Returns true iff a new outgoing transaction can be made on this session
   */
  bool supportsMoreTransactions() const override {
    return sock_ && sock_->getNumOpenableUnidirectionalStreams() &&
           HTTPSessionBase::supportsMoreTransactions();
  }

  uint32_t getNumOutgoingStreams() const override {
    // need transport API
    return static_cast<uint32_t>(numberOfEgressPushStreams());
  }

  uint32_t getNumIncomingStreams() const override {
    // need transport API
    return static_cast<uint32_t>(streams_.size());
  }

  folly::Optional<HTTPHeaders> getExtraHeaders(
      const HTTPMessage& haeders, quic::StreamId streamId) override;

 private:
  ~HQDownstreamSession() override {
    CHECK_EQ(getNumStreams(), 0);
  }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250) // inherits 'proxygen::detail::..' via dominance
#endif

  /**
   * Server side representation of a push stream
   * Does not support ingress
   */
  class HQEgressPushStream
      : public detail::singlestream::SSEgress
      , public HQStreamTransportBase {
   public:
    HQEgressPushStream(HQSession& session,
                       quic::StreamId streamId,
                       hq::PushId pushId,
                       folly::Optional<HTTPCodec::StreamID> parentTxnId,
                       uint32_t seqNo,
                       std::unique_ptr<HTTPCodec> codec,
                       const WheelTimerInstance& timeout,
                       HTTPSessionStats* stats = nullptr,
                       http2::PriorityUpdate priority = hqDefaultPriority)
        : detail::singlestream::SSEgress(streamId),
          HQStreamTransportBase(session,
                                TransportDirection::DOWNSTREAM,
                                streamId,
                                seqNo,
                                timeout,
                                stats,
                                priority,
                                parentTxnId,
                                hq::UnidirectionalStreamType::PUSH),
          pushId_(pushId) {
      // Request streams are eagerly initialized
      initCodec(std::move(codec), __func__);
      // DONT init ingress on egress-only stream
    }

    hq::PushId getPushId() const {
      return pushId_;
    }

    // Unlike request streams and ingres push streams,
    // the egress push stream does not have to flush
    // ingress queues
    void transactionTimeout(HTTPTransaction* txn) noexcept override {
      VLOG(4) << __func__ << " txn=" << txn_;
      DCHECK(txn == &txn_);
    }

    void sendPushPromise(HTTPTransaction* /* txn */,
                         folly::Optional<hq::PushId> /* pushId */,
                         const HTTPMessage& /* headers */,
                         HTTPHeaderSize* /* outSize */,
                         bool /* includeEOM */) override;

    /**
     * Write the encoded push id to the egress stream.
     */
    size_t generateStreamPushId();

    // Egress only stream should not pause ingress
    void pauseIngress(HTTPTransaction* /* txn */) noexcept override {
      VLOG(4) << __func__
              << " Ingress function called on egress-only stream, ignoring";
    }

    // Egress only stream should not pause ingress
    void resumeIngress(HTTPTransaction* /* txn */) noexcept override {
      VLOG(4) << __func__
              << " Ingress function called on egress-only stream, ignoring";
    }

   private:
    hq::PushId pushId_; // The push id in context of which this stream is sent
  };                    // HQEgressPushStream
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  std::unordered_map<quic::StreamId, HQEgressPushStream> egressPushStreams_;

  // Find an egress push stream
  HQEgressPushStream* findEgressPushStream(quic::StreamId);

  uint32_t numberOfEgressPushStreams() const;

  HQEgressPushStream* FOLLY_NULLABLE
  createEgressPushStream(hq::PushId pushId,
                         quic::StreamId streamId,
                         quic::StreamId parentStreamId);

  HQStreamTransportBase* findPushStream(quic::StreamId id) override;

  // Only need to search ingress push streams, so this is a no-op
  void findPushStreams(
      std::unordered_set<HQStreamTransportBase*>& streams) override {
    for (auto& pstream : egressPushStreams_) {
      streams.insert(&pstream.second);
    }
  }

  bool erasePushStream(quic::StreamId streamId) override;

  void dispatchPushStream(quic::StreamId /* pushStreamId */,
                          hq::PushId /* pushId */,
                          size_t /* toConsume */) override {
    LOG(DFATAL) << "nope";
  }

  // This is the current method of creating new push IDs.
  hq::PushId createNewPushId();

  bool pushAllowedByGoaway(hq::PushId pushId);

  // Value of the next pushId, used for outgoing push transactions
  hq::PushId nextAvailablePushId_{0};

  // Whether or not we have already received an onTransportReady callback.
  bool transportReadyNotified_{false};
};

} // namespace proxygen
