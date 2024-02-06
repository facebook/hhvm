/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <fizz/client/FizzClientContext.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/AsyncUDPSocket.h>
#include <proxygen/lib/http/session/HQUpstreamSession.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <quic/api/QuicSocket.h>
#include <quic/client/QuicClientTransport.h>

namespace proxygen {

class H3DatagramAsyncSocketTest;

class H3DatagramAsyncSocket
    : public folly::AsyncUDPSocket
    , HQSession::ConnectCallback
    , HTTPSessionBase::InfoCallback
    , proxygen::HTTPTransactionHandler
    , public folly::DelayedDestruction {

  friend class H3DatagramAsyncSocketTest;

 public:
  enum class Mode {
    CLIENT = 0,
    SERVER = 1,
  };

  struct Options {
    Mode mode_;
    std::chrono::milliseconds txnTimeout_{10000};
    std::chrono::milliseconds connectTimeout_{3000};
    std::shared_ptr<proxygen::HTTPMessage> httpRequest_;
    folly::Optional<std::pair<std::string, std::string>> certAndKey_;
    std::shared_ptr<const fizz::CertificateVerifier> certVerifier_;
    uint16_t maxDatagramSize_{1400};
  };

 public:
  H3DatagramAsyncSocket(folly::EventBase* evb,
                        H3DatagramAsyncSocket::Options options);

  ~H3DatagramAsyncSocket() override {
    if (txn_) {
      txn_->setHandler(nullptr);
    }
    if (upstreamSession_) {
      upstreamSession_->setConnectCallback(nullptr);
      upstreamSession_->setInfoCallback(nullptr);
    }
  }

  /*
   * AsyncUDPSocket
   */
  const folly::SocketAddress& address() const override;

  void bind(const folly::SocketAddress& address,
            BindOptions /*options*/ = BindOptions()) override {
    bindAddress_ = address;
  }

  void connect(const folly::SocketAddress& address) override {
    CHECK(options_.mode_ == Mode::CLIENT);
    connectAddress_ = address;
    startClient();
  }

  void setFD(folly::NetworkSocket /*fd*/, FDOwnership /*ownership*/) override {
    LOG(FATAL) << __func__ << " not supported";
  }

  void setCmsgs(const folly::SocketCmsgMap& /*cmsgs*/) override {
    LOG(FATAL) << __func__ << " not supported";
  }

  void appendCmsgs(const folly::SocketCmsgMap& /*cmsgs*/) override {
    LOG(FATAL) << __func__ << " not supported";
  }

  ssize_t write(const folly::SocketAddress& address,
                const std::unique_ptr<folly::IOBuf>& buf) override;

  int writem(folly::Range<folly::SocketAddress const*> /*addrs*/,
             const std::unique_ptr<folly::IOBuf>* /*bufs*/,
             size_t /*count*/) override {
    return -1;
  }

  ssize_t writeGSO(const folly::SocketAddress& /*address*/,
                   const std::unique_ptr<folly::IOBuf>& /*buf*/,
                   folly::AsyncUDPSocket::WriteOptions /*options*/) override {
    return -1;
  }

  ssize_t writeChain(const folly::SocketAddress& /*address*/,
                     std::unique_ptr<folly::IOBuf>&& /*buf*/,
                     WriteOptions /*options*/) override {
    return -1;
  }

  int writemGSO(folly::Range<folly::SocketAddress const*> /*addrs*/,
                const std::unique_ptr<folly::IOBuf>* /*bufs*/,
                size_t /*count*/,
                const WriteOptions* /*options*/) override {
    return -1;
  }

  ssize_t writev(const folly::SocketAddress& /*address*/,
                 const struct iovec* /*vec*/,
                 size_t /*iovec_len*/,
                 folly::AsyncUDPSocket::WriteOptions /*options*/) override {
    return -1;
  }

  ssize_t writev(const folly::SocketAddress& /*address*/,
                 const struct iovec* /*vec*/,
                 size_t /*iovec_len*/) override {
    return -1;
  }

  ssize_t recvmsg(struct msghdr* /*msg*/, int /*flags*/) override;

  int recvmmsg(struct mmsghdr* msgvec,
               unsigned int vlen,
               unsigned int flags,
               struct timespec* /*timeout*/) override;

  void resumeRead(ReadCallback* cob) override;

  void pauseRead() override {
    readCallback_ = nullptr;
  }

  void close() override {
    if (txn_ && !txn_->isEgressEOMSeen()) {
      txn_->sendEOM();
    }
    if (upstreamSession_) {
      upstreamSession_->closeWhenIdle();
    }
  }

  folly::NetworkSocket getNetworkSocket() const override {
    // Not great but better than crashing.
    VLOG(4) << "getNetworkSocket returning fake socket";
    return folly::NetworkSocket();
  }

  void setReusePort(bool /*reusePort*/) override {
    // Meaningless
  }

  void setReuseAddr(bool /*reuseAddr*/) override {
    // Meaningless
  }

  void setRcvBuf(int rcvBuf) override {
    if (rcvBuf > 0) {
      // This is going to be the maximum number of packets buffered here or at
      // the the quic transport
      rcvBufPkts_ = rcvBuf / options_.maxDatagramSize_;
    }
  }

  void setSndBuf(int sndBuf) override {
    if (sndBuf > 0) {
      // This is going to be the maximum number of packets buffered here or at
      // the the quic transport
      sndBufPkts_ = sndBuf / options_.maxDatagramSize_;
    }
  }

  void setBusyPoll(int /*busyPollUs*/) override {
    VLOG(4) << "busy poll not supported";
  }

  void dontFragment(bool /*df*/) override {
    // Meaningless.
  }

  void setDFAndTurnOffPMTU() override {
    // Meaningless.
  }

  void setErrMessageCallback(ErrMessageCallback*) override {
    // TODO do we want to support this and convey errors this way?
    VLOG(4) << "err message callback not supported";
  }

  bool isBound() const override {
    return false;
  }

  bool isReading() const override {
    return readCallback_ != nullptr;
  }

  void detachEventBase() override {
    LOG(FATAL) << __func__ << " unsupported";
    folly::assume_unreachable();
  }

  void attachEventBase(folly::EventBase* /*evb*/) override {
    LOG(FATAL) << __func__ << " unsupported";
    folly::assume_unreachable();
  }

  int getGSO() override {
    return -1;
  }

  void setOverrideNetOpsDispatcher(
      std::shared_ptr<folly::netops::Dispatcher> /*dispatcher*/) override {
    LOG(FATAL) << __func__ << " unsupported";
    folly::assume_unreachable();
  }

  std::shared_ptr<folly::netops::Dispatcher> getOverrideNetOpsDispatcher()
      const override {
    LOG(FATAL) << __func__ << " unsupported";
    folly::assume_unreachable();
  }

 private:
  /*
   * proxygen::HQSession::ConnectCallback
   */
  void connectSuccess() override;
  void onReplaySafe() override;
  void connectError(quic::QuicError error) override;

  // HTTPTransactionHandler methods
  void setTransaction(proxygen::HTTPTransaction* txn) noexcept override;
  void detachTransaction() noexcept override;
  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override;
  void onDatagram(std::unique_ptr<folly::IOBuf> datagram) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override;
  void onTrailers(
      std::unique_ptr<proxygen::HTTPHeaders> trailers) noexcept override;
  void onEOM() noexcept override;
  void onUpgrade(proxygen::UpgradeProtocol protocol) noexcept override;
  void onError(const proxygen::HTTPException& error) noexcept override;
  void onEgressPaused() noexcept override;
  void onEgressResumed() noexcept override;

  // Set the HTTP/3 Session. for testing only
  void setUpstreamSession(proxygen::HQUpstreamSession* session) {
    CHECK(!upstreamSession_);
    upstreamSession_ = session;
    upstreamSession_->setConnectCallback(this);
    upstreamSession_->setInfoCallback(this);
  }

  /*
   * HTTPSessionBase::InfoCallback
   */
  void onDestroy(const HTTPSessionBase&) override {
    upstreamSession_ = nullptr;
  }

 private:
  void startClient();
  std::shared_ptr<fizz::client::FizzClientContext> createFizzClientContext();
  void closeWithError(const folly::AsyncSocketException& ex);
  void deliverDatagram(std::unique_ptr<folly::IOBuf> datagram) noexcept;

  void closeRead() {
    if (readCallback_) {
      auto cb = readCallback_;
      readCallback_ = nullptr;
      cb->onReadClosed();
    }
  }

  folly::EventBase* evb_;
  Options options_;
  folly::SocketAddress bindAddress_;
  folly::SocketAddress connectAddress_;
  proxygen::HQUpstreamSession* upstreamSession_{nullptr};

  HTTPTransaction* txn_{nullptr};
  folly::Optional<folly::AsyncSocketException> pendingError_;

  unsigned int rcvBufPkts_{100};
  unsigned int sndBufPkts_{100};
  // Buffers Incoming Datagrams when reads are paused
  std::deque<std::unique_ptr<folly::IOBuf>> readBuf_;
  // Buffers Outgoing Datagrams before the transport is ready
  std::deque<std::unique_ptr<folly::IOBuf>> writeBuf_;
  std::unique_ptr<folly::IOBuf> pendingDelivery_;

  bool transportConnected_ : 1;
  bool pendingEOM_ : 1;
  bool inResumeRead_ : 1;
};

} // namespace proxygen
