/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>
#include <proxygen/lib/http/webtransport/WtStreamManager.h>
#include <proxygen/lib/http/webtransport/WtUtils.h>
#include <quic/api/QuicSocket.h>
#include <quic/priority/HTTPPriorityQueue.h>

namespace proxygen {

using QuicSocket = quic::QuicSocket;
using StreamId = quic::StreamId;
using QuicError = quic::QuicError;

// fwd-decl; defined below
struct H3ConnectStreamCallback;

/**
 * This class is effectively an adaptor, it exposes a WebTransport interace over
 * a quic::QuicSocket. It's a pure virtual interface because we have two main
 * expected use cases / derived classes:
 *
 *   - exposing a direct mapping of WebTransport to quic, this implementation
 *     assumes a unique ownership of the QuicSocket and all of QuicSocket's
 *     stream data is directly piped to/from WtStreamManager.
 *
 *   - exposing a mapping of WebTransport to http/3 over quic, this
 *     implementation assumes a shared ownership of the QuicSocket and
 *     H3WtSession will manage a subset (only those belonging to the original
 *     CONNECT stream) of QuicSocket's streams via QuicWtSessionBase.
 */
class QuicWtSessionBase
    : public detail::WtSessionBase
    , private quic::StreamWriteCallback {
 public:
  [[nodiscard]] quic::TransportInfo getTransportInfo() const noexcept override {
    XCHECK(quicSocket_);
    return quicSocket_->getTransportInfo();
  }

  folly::Expected<StreamWriteHandle*, ErrorCode> createUniStream() noexcept
      override;
  folly::Expected<BidiStreamHandle, ErrorCode> createBidiStream() noexcept
      override;
  folly::SemiFuture<folly::Unit> awaitUniStreamCredit() noexcept override;
  folly::SemiFuture<folly::Unit> awaitBidiStreamCredit() noexcept override;

  folly::Expected<folly::Unit, ErrorCode> sendDatagram(
      IoBufPtr datagram) noexcept override;

  [[nodiscard]] const folly::SocketAddress& getLocalAddress() const override {
    return quicSocket_->getLocalAddress();
  }

  [[nodiscard]] const folly::SocketAddress& getPeerAddress() const override {
    return quicSocket_->getPeerAddress();
  }

  folly::Expected<folly::Unit, ErrorCode> closeSession(
      folly::Optional<uint32_t> error) noexcept override;

 protected:
  // TODO(@joannajo): Remove WtHandlerDeleter + setHandler() once MoQSession is
  // fully migrated to QuicWtSession.
  struct WtHandlerDeleter {
    bool owning{true};
    void operator()(WebTransportHandler* p) const noexcept {
      if (owning) {
        std::default_delete<WebTransportHandler>{}(p);
      }
    }
  };
  using WtHandlerPtr = std::unique_ptr<WebTransportHandler, WtHandlerDeleter>;

  QuicWtSessionBase(std::shared_ptr<QuicSocket> quicSocket,
                    WtHandlerPtr wtHandler,
                    detail::WtStreamManager::WtConfig wtConfig,
                    H3ConnectStreamCallback* observer = nullptr);
  ~QuicWtSessionBase() override = 0;

  void onDatagram(IoBufPtr dgram) noexcept;

  struct QuicReadCallback : public quic::QuicSocket::ReadCallback {
    QuicWtSessionBase& sess;
    explicit QuicReadCallback(QuicWtSessionBase& session) : sess(session) {
    }
    void readAvailable(StreamId streamId) noexcept override;
    void readError(StreamId streamId, QuicError error) noexcept override;
  } readCb_{*this};

  struct QuicStopSendingCallback : public quic::StopSendingCallback {
    QuicWtSessionBase& sess;
    explicit QuicStopSendingCallback(QuicWtSessionBase& session)
        : sess(session) {
    }
    void onStopSending(StreamId id,
                       quic::ApplicationErrorCode ec) noexcept override;
  } stopSendingCb_{*this};

  struct StreamManagerCallback
      : public detail::WtStreamManager::ReadCallback
      , public detail::WtStreamManager::EgressCallback
      , public detail::WtStreamManager::IngressCallback {
    QuicWtSessionBase& sess;
    explicit StreamManagerCallback(QuicWtSessionBase& session) : sess(session) {
    }
    void readReady(detail::WtStreamManager::WtReadHandle& rh) noexcept override;
    void eventsAvailable() noexcept override;
    void onNewPeerStream(uint64_t streamId) noexcept override;
  } smCb_{*this};

  /**
   * attempts to acquire an ingress stream id - returns false if it fails or
   * true if successful.
   */
  bool acquireIngressStream(uint64_t id) noexcept;

  std::shared_ptr<quic::QuicSocket> quicSocket_{nullptr};
  WtHandlerPtr wtHandler_;
  std::unique_ptr<quic::HTTPPriorityQueue> priorityQueue_;
  detail::WtStreamManager sm_;
  H3ConnectStreamCallback* observer_{nullptr};

 private:
  /**
   * Returns true iff there is bidi/uni credit w.r.t both QuicSocket &
   * WtStreamManager. If this returns true, we can safely proceed to create
   * egress streams via ::create(Uni|Bidi)Stream
   */
  bool hasEgressUniCredit() const noexcept;
  bool hasEgressBidiCredit() const noexcept;

  BidiStreamHandle createWtEgressHandle(StreamId id) noexcept;

  // -- StreamWriteCallback overrides --
  void onStreamWriteReady(StreamId id, uint64_t maxToSend) noexcept override;
  void onStreamWriteError(StreamId id, QuicError error) noexcept override;

  void maybePauseIngress(
      detail::WtStreamManager::WtReadHandle& handle) noexcept;
  void maybeResumeIngress(
      detail::WtStreamManager::WtReadHandle& handle) noexcept;
};

class QuicWtSession final : public QuicWtSessionBase {
 public:
  QuicWtSession(std::shared_ptr<QuicSocket> quicSocket,
                std::unique_ptr<WebTransportHandler> wtHandler);
  QuicWtSession(std::shared_ptr<QuicSocket> quicSocket,
                WebTransportHandler* wtHandler);
  ~QuicWtSession() override;

  folly::Expected<folly::Unit, ErrorCode> closeSession(
      folly::Optional<uint32_t> error) noexcept override;

  void setHandler(WebTransportHandler* handler) noexcept {
    if (!handler) {
      closeSession(folly::none);
    }
    wtHandler_ = WtHandlerPtr(handler, WtHandlerDeleter{.owning = false});
  }

 private:
  QuicWtSession(std::shared_ptr<QuicSocket> quicSocket, WtHandlerPtr wtHandler);

  struct QuicConnectionCallback : public QuicSocket::ConnectionCallback {
    QuicWtSession& sess;
    explicit QuicConnectionCallback(QuicWtSession& session) : sess(session) {
    }
    void onNewBidirectionalStream(StreamId id) noexcept override;
    void onNewUnidirectionalStream(StreamId id) noexcept override;
    void onConnectionEnd() noexcept override;
    void onConnectionEnd(QuicError error) noexcept override;
    void onConnectionError(QuicError code) noexcept override;
    void onBidirectionalStreamsAvailable(
        uint64_t numStreamsAvailable) noexcept override;
    void onUnidirectionalStreamsAvailable(
        uint64_t numStreamsAvailable) noexcept override;
    void onStopSending(StreamId, quic::ApplicationErrorCode) noexcept override {
      // handled by per-stream QuicSocket::addStopSending
    }
  } connCb_{*this};

  struct QuicDgramCallback : public QuicSocket::DatagramCallback {
    QuicWtSession& sess;
    explicit QuicDgramCallback(QuicWtSession& session) : sess(session) {
    }
    void onDatagramsAvailable() noexcept override;
  } dgramCb_{*this};
};

/**
 * H3ConnectStreamCallback is a mechanism to signal egress connect stream events
 * (e.g. CloseSession, MaxData, etc.) to the backing http/3 transport (as it
 * owns the connect stream state).
 *
 * NOTE: Only a subset of the std::variant<...> will be invoked (namely
 * MaxConnData, MaxStreams(Uni|Bidi), CloseSession, DrainSession), maybe
 * should we instead create a virtual fn for each of the subset?
 */
struct H3ConnectStreamCallback {
  explicit H3ConnectStreamCallback(folly::IOBufQueue& writeBuf) noexcept
      : visitor(writeBuf) {
  }
  virtual ~H3ConnectStreamCallback() noexcept = default;
  virtual void onEvent(detail::WtStreamManager::Event&& ev) noexcept;
  detail::WtEventVisitor visitor;
};

class H3WtSession final : public QuicWtSessionBase {
 public:
  H3WtSession(std::shared_ptr<QuicSocket> quicSocket,
              std::unique_ptr<WebTransportHandler> wtHandler,
              detail::WtStreamManager::WtConfig wtConfig,
              uint64_t connectStreamId,
              H3ConnectStreamCallback& observer) noexcept;

  ~H3WtSession() noexcept override;

  folly::Expected<folly::Unit, ErrorCode> closeSession(
      folly::Optional<uint32_t> error) noexcept override;

  /**
   * H3WtSession::acquireIngressStream should be invoked by the backing http/3
   * when an ingress wt stream (for the given connect stream) has been received.
   * After this function is invoked, the peer's stream will be fully managed by
   * H3WtSession.
   *
   * NOTE: the wt stream prefix is expected to already have been consumed by the
   * backing http/3 session.
   */
  using QuicWtSessionBase::acquireIngressStream;

  /**
   * Invoked when the backing http/3 session receives a datagram destined for
   * this CONNECT stream.
   *
   * NOTE: the quarter stream id is expected to have already been consumed by
   * the backing http/3 session.
   */
  using QuicWtSessionBase::onDatagram;

  /**
   * An egress stream is created by either of the two below functions. Currently
   * the way this designed, the backing http/3 session is completely oblivious
   * of any outgoing streams (as they're fully managed by H3WtSession).
   * Consequently, H3WtSession serializes the wt frame prefix on behalf of the
   * http/3 session on stream creation.
   */
  folly::Expected<StreamWriteHandle*, ErrorCode> createUniStream() noexcept
      override;
  folly::Expected<BidiStreamHandle, ErrorCode> createBidiStream() noexcept
      override;

  /**
   * Serializes the http/3 datagram, i.e. prefixes the datagram w/ quarter
   * stream id (connectStreamId_ / 4) per RFC9297. Similarly to
   * ::create(Uni|Bidi)Stream functions above, ::sendDatagram writes bypass
   * the http session and directly write to the QuicSocket.
   */
  folly::Expected<folly::Unit, ErrorCode> sendDatagram(
      IoBufPtr datagram) noexcept override;

  /**
   * The backing http/3 session should invoke these functions when the
   * corresponding capsules have been received & parsed on the CONNECT stream.
   */
  using WtSmResult = detail::WtStreamManager::Result;
  WtSmResult onConnMaxData(detail::WtStreamManager::MaxConnData mcd) noexcept;
  WtSmResult onMaxStreams(detail::WtStreamManager::MaxStreamsUni ms) noexcept;
  WtSmResult onMaxStreams(detail::WtStreamManager::MaxStreamsBidi ms) noexcept;
  void onDrainSession(detail::WtStreamManager::DrainSession ds) noexcept;
  void onCloseSession(detail::WtStreamManager::CloseSession&& cs) noexcept;

 private:
  // sends the http/3 wt frame prefix via QuicSocket::writeChain -- invoked
  // by both ::create(Uni|Bidi)Stream on success
  void writeWtFramePrefix(uint64_t id) noexcept;

  uint64_t connectStreamId_;
};

} // namespace proxygen
