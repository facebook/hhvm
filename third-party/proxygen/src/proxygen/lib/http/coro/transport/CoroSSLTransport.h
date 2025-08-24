/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/io/coro/Transport.h>

#include "proxygen/lib/http/coro/util/TimedBaton.h"
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/CertificateIdentityVerifier.h>
#include <folly/io/async/SSLContext.h>
#include <folly/ssl/SSLSession.h>
#include <folly/ssl/SSLSessionManager.h>

namespace proxygen::coro {

class CoroSSLTransport : public folly::coro::TransportIf {
 public:
  struct TransportOptions {
    // If this verifier is set, it's used during the TLS handshake. It will be
    // invoked to verify the peer's end-entity leaf certificate after OpenSSL's
    // chain validation and after calling the HandshakeCB's handshakeVer() and
    // only if these are successful.
    TransportOptions() {
    }
    std::shared_ptr<folly::CertificateIdentityVerifier> verifier;
  };

  CoroSSLTransport(
      std::unique_ptr<folly::coro::TransportIf> transport,
      std::shared_ptr<const folly::SSLContext> sslContext,
      // const std::shared_ptr<ClientExtensions>& extensions = nullptr,
      TransportOptions transportOptions = CoroSSLTransport::TransportOptions());

  ~CoroSSLTransport() override;

  folly::coro::Task<void> connect(
      // std::shared_ptr<const CertificateVerifier> verifier,
      folly::Optional<std::string> sni,
      std::chrono::milliseconds timeout);

  void setVerificationOption(folly::SSLContext::SSLVerifyPeerEnum verifyPeer) {
    verifyPeer_ = verifyPeer;
  }

  std::shared_ptr<folly::ssl::SSLSession> getSSLSession() {
    return sslSessionManager_.getSession();
  }

  /**
   * Sets the SSL session that will be attempted for TLS resumption.
   */
  void setSSLSession(std::shared_ptr<folly::ssl::SSLSession> session) {
    sslSessionManager_.setSession(std::move(session));
  }

  /**
   * Determine if the session specified during setSSLSession was reused
   * or if the server rejected it and issued a new session.
   */
  bool getSSLSessionReused() const;

  std::string getApplicationProtocol() const;

  const char* getNegotiatedCipherName() const;

  /**
   * Get the SSL version for this connection.
   */
  int getSSLVersion() const;

  /* TransportIf overrides */
  folly::EventBase* getEventBase() noexcept override {
    return transport_->getEventBase();
  }

  folly::coro::Task<size_t> read(folly::MutableByteRange buf,
                                 std::chrono::milliseconds timeout =
                                     std::chrono::milliseconds(0)) override;

  folly::coro::Task<size_t> read(folly::IOBufQueue& buf,
                                 size_t minReadSize,
                                 size_t newAllocationSize,
                                 std::chrono::milliseconds timeout =
                                     std::chrono::milliseconds(0)) override;

  folly::coro::Task<folly::Unit> write(
      folly::ByteRange buf,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      folly::WriteFlags writeFlags = folly::WriteFlags::NONE,
      WriteInfo* writeInfo = nullptr) override;
  folly::coro::Task<folly::Unit> write(
      folly::IOBufQueue& ioBufQueue,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
      folly::WriteFlags writeFlags = folly::WriteFlags::NONE,
      WriteInfo* writeInfo = nullptr) override;

  folly::SocketAddress getLocalAddress() const noexcept override {
    return localAddr_;
  }

  folly::SocketAddress getPeerAddress() const noexcept override {
    return peerAddr_;
  }

  void close() override;
  void shutdownWrite() override;
  void closeWithReset() override;
  folly::AsyncTransport* getTransport() const override {
    return transport_->getTransport();
  }
  const folly::AsyncTransportCertificate* getPeerCertificate() const override;

  int bioWrite(const char* buf, size_t sz);
  int bioRead(char* buf, size_t sz);
  long bioCtrl(int /*i*/, long /*l*/, void* /*p*/);
  int bioCallbackCtrl(int /*idx*/, BIO_info_cb* /*cb*/);

  enum class IOResult { Success, EndOfFile };

 private:
  // Callback for SSL_CTX_set_verify()
  static int sslVerifyCallback(int preverifyOk, X509_STORE_CTX* ctx);
  static CoroSSLTransport* getFromSSL(const SSL* ssl);

  bool setupSSLBio();
  bool applyVerificationOptions(const folly::ssl::SSLUniquePtr& ssl);
  folly::coro::Task<void> doConnect(std::chrono::milliseconds timeout);
  folly::coro::Task<IOResult> handleReturnMaybeIO(
      int ret, std::optional<std::chrono::steady_clock::time_point> deadline);
  folly::coro::Task<IOResult> transportRead(
      std::optional<std::chrono::steady_clock::time_point> deadline);
  folly::coro::Task<folly::Unit> writeImpl(folly::ByteRange buf,
                                           std::chrono::milliseconds timeout,
                                           folly::WriteFlags writeFlags,
                                           WriteInfo* writeInfo,
                                           bool writev);
  bool willBlock(int ret,
                 int* sslErrorOut,
                 unsigned long* errErrorOut) noexcept;
  void failWrites();
  void closeNow();
  bool shutdownRead();

  std::unique_ptr<folly::coro::TransportIf> transport_;
  folly::SocketAddress localAddr_;
  folly::SocketAddress peerAddr_;
  TransportOptions transportOptions_;

  std::shared_ptr<const folly::SSLContext> ctx_;
  folly::ssl::SSLUniquePtr ssl_;

  folly::SSLContext::SSLVerifyPeerEnum verifyPeer_{
      folly::SSLContext::SSLVerifyPeerEnum::USE_CTX};

  // Manages the session for the socket
  folly::ssl::SSLSessionManager sslSessionManager_;
  mutable std::unique_ptr<const folly::AsyncTransportCertificate> peerCertData_;
  folly::CancellationSource cancellationSource_;
  std::string sni_;
  std::shared_ptr<bool> deleted_{std::make_shared<bool>(false)};
  folly::IOBufQueue transportReadBuf_{folly::IOBufQueue::cacheChainLength()};
  TimedBaton readsBlocked_;
  std::optional<std::chrono::steady_clock::time_point> readDeadline_;
  size_t transportBytesOutstanding_{0};
  TimedBaton writesBlocked_;
  std::optional<std::chrono::steady_clock::time_point> writeDeadline_;
  size_t writers_{0};
  bool pendingShutdown_{false};
  bool pendingClose_{false};
};

} // namespace proxygen::coro
