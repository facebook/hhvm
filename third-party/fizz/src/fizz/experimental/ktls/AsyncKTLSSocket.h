/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <fizz/experimental/ktls/KTLS.h>
#include <folly/io/async/AsyncSocket.h>

namespace fizz {

/**
 * AsyncKTLSSocket acts like an AsyncSocket, but utilizes kTLS to ensure that
 * all data sent and received from it are encrypted and wrapped in TLS data
 * records.
 *
 * This does not provide full functionality to TLS. AsyncKTLSSocket handles
 * a subset of handshake messages to ensure interoperability with the majority
 * of clients.
 *
 * Currently, this implementation of AsyncKTLSSocket:
 *  * Only supports TLS 1.3
 *  * Requires both TX and RX enabled at the same time.
 *
 * kTLS is made a separate type (rather than being implemented transparently
 * in Fizz), because of the following reasons:
 *  * The upgrade should be explicit. As part of the upgrade, we return
 *    information on why the upgrade failed, so if the user is *expecting* to
 *    use kTLS in the common case, the user will know how to remediate
 *    situations like:
 *        * tls module not being loaded (add tls to /etc/modules ...)
 *        * cipher not supported (adjust server cipher preferences ...)
 *  * Some of the original settings that the user specified for the original
 *    Fizz socket will now be violated:
 *      * e.g. record sizes. kTLS uses a hardcoded 16k max record size. This may
 *        cause compatibility concerns
 *  * We can throw away much of the state from AsyncFizzSocket and keep just
 *    the bare minimum to handle TLS events. This allows us to save some
 *    memory per socket.
 *
 * KNOWN LIMITATIONS
 * =================
 * * kTLS does not have a way to handle KeyUpdate messages. (There is no way
 *   to update the TLS keys in the kernel). This makes kTLS non-compliant with a
 *   standard TLS 1.3 implementation. Do not use this class
 *   if you are expected to interop with clients that you do not control.
 *
 * * kTLS inherently is incompatible with SO_ZEROCOPY
 * (AsyncSocket::setZeroCopy).
 */
class AsyncKTLSSocket final : public folly::AsyncSocket {
 public:
  using UniquePtr =
      std::unique_ptr<AsyncKTLSSocket, folly::DelayedDestruction::Destructor>;
  struct TLSCallback {
    virtual ~TLSCallback() = default;
    /**
     * receivedNewSessionTicket() is invoked whenever the peer sends a
     * NewSessionTicket message.
     *
     * Implementations may choose to ignore this,
     * but it is advised to update any associated PSK cache if the peer is
     * issuing a new ticket to identify this session.
     *
     * NOTE: IOBufs point to borrowed memory valid for the lifetime of the
     * invocation.  Implementations *must* first unshare() any buffers if
     * they are needed past function return.
     */
    virtual void receivedNewSessionTicket(
        AsyncKTLSSocket* /*sock*/,
        fizz::NewSessionTicket /*ticket*/) {}
  };

  /**
   * Constructs an AsyncKTLSSocket from an AsyncSocket that already has
   * kTLS enabled.
   *
   * This could be the case if:
   *  * You are taking over a detached file descriptor from another
   *    AsyncKTLSSocket
   *  * You have established a ktls 1.3 connection out of band
   *
   * @param evb         The eventbase that the AsyncKTLSSocket will initially
   *                    be attached to.
   * @param fd          The kTLS enabled file descriptor.
   * @param tlsCallback An instance of `TLSCallback` which will be used to
   *                    handle non-application data events.
   * @param selfCert    The certificate used to authenticate *to* the peer.
   * @param peerCert    The certificate that the *peer* presented and
   *                    *we* authenticated.
   */
  AsyncKTLSSocket(
      folly::EventBase* evb,
      KTLSNetworkSocket fd,
      std::unique_ptr<TLSCallback> tlsCallback,
      std::shared_ptr<const Cert> selfCert,
      std::shared_ptr<const Cert> peerCert)
      : AsyncSocket(evb, fd),
        tlsCallback_(std::move(tlsCallback)),
        selfCert_(std::move(selfCert)),
        peerCert_(std::move(peerCert)) {}

  bool hasBufferedHandshakeData() const {
    return unparsedHandshakeData_ != nullptr;
  }

  std::string getSecurityProtocol() const override {
    return "Fizz/KTLS";
  }

  const folly::AsyncTransportCertificate* getPeerCertificate() const override {
    return peerCert_.get();
  }

  const folly::AsyncTransportCertificate* getSelfCertificate() const override {
    return selfCert_.get();
  }

  void dropPeerCertificate() noexcept override {
    peerCert_.reset();
  }

  void dropSelfCertificate() noexcept override {
    selfCert_.reset();
  }

 private:
  /**
   * kTLS does not support SO_ZEROCOPY
   */
  bool setZeroCopy(bool) override {
    return false;
  }
  void setZeroCopyEnableFunc(AsyncWriter::ZeroCopyEnableFunc) override {}

  AsyncSocket::ReadResult performReadMsg(
      struct ::msghdr& msg,
      AsyncReader::ReadCallback::ReadMode readMode) override;

  AsyncSocket::ReadResult handleNonApplicationData(
      uint8_t type,
      folly::IOBufQueue);

  AsyncSocket::ReadResult processAlert(folly::IOBufQueue);
  AsyncSocket::ReadResult processHandshakeData(folly::IOBufQueue);

  std::unique_ptr<TLSCallback> tlsCallback_;
  std::shared_ptr<const Cert> selfCert_;
  std::shared_ptr<const Cert> peerCert_;

  // This is unlikely to be used (see
  // [note.handshake_fits_in_record_assumption])
  // so we opt for a heap allocation (IOBufQueue is 64 bytes as of this
  // writing)
  std::unique_ptr<folly::IOBufQueue> unparsedHandshakeData_;
};
} // namespace fizz
