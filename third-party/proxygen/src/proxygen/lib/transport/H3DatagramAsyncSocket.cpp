/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/H3DatagramAsyncSocket.h>

#include <folly/FileUtil.h>
#include <quic/common/udpsocket/FollyQuicAsyncUDPSocket.h>
#include <quic/fizz/client/handshake/FizzClientQuicHandshakeContext.h>
#include <utility>
#include <wangle/acceptor/TransportInfo.h>

namespace proxygen {

using folly::AsyncSocketException;

H3DatagramAsyncSocket::H3DatagramAsyncSocket(folly::EventBase* evb,
                                             Options options)
    : folly::AsyncUDPSocket(evb),
      evb_(evb),
      options_(std::move(options)),
      transportConnected_(false),
      pendingEOM_(false),
      inResumeRead_(false) {
}

const folly::SocketAddress& H3DatagramAsyncSocket::address() const {
  static folly::SocketAddress localFallbackAddress;
  if (!upstreamSession_) {
    return localFallbackAddress;
  }
  return upstreamSession_->getLocalAddress();
}

void H3DatagramAsyncSocket::closeWithError(const AsyncSocketException& ex) {
  if (pendingError_.has_value()) {
    LOG(ERROR) << "Multiple errors. Previous error: '" << pendingError_->what()
               << "'";
    return;
  }
  if (!readCallback_) {
    LOG(ERROR)
        << "Error with readCallback not set. Will deliver when resuming reads.";
    pendingError_ = ex;
    return;
  }
  readCallback_->onReadError(ex);
  closeRead();
}

void H3DatagramAsyncSocket::connectSuccess() {
  if (!options_.httpRequest_) {
    closeWithError({AsyncSocketException::BAD_ARGS, "No HTTP Request"});
    return;
  }

  if (!upstreamSession_) {
    closeWithError({AsyncSocketException::INTERNAL_ERROR,
                    "ConnectSuccess with invalid session"});
    return;
  }

  // Send the HTTPMessage
  txn_ = upstreamSession_->newTransaction(this);
  if (!txn_ || !txn_->canSendHeaders()) {
    closeWithError({AsyncSocketException::INTERNAL_ERROR, "Transaction Error"});
    return;
  }
  txn_->sendHeaders(*options_.httpRequest_);
  upstreamSession_->closeWhenIdle();
  transportConnected_ = true;
}

void H3DatagramAsyncSocket::onReplaySafe() {
}

void H3DatagramAsyncSocket::connectError(quic::QuicError error) {
  closeWithError({AsyncSocketException::NETWORK_ERROR,
                  fmt::format("connectError: '{}'", error.message)});
}

void H3DatagramAsyncSocket::setTransaction(
    proxygen::HTTPTransaction* /*txn*/) noexcept {
  CHECK(!txn_);
}

void H3DatagramAsyncSocket::detachTransaction() noexcept {
  VLOG(4) << "Transaction Detached";
  txn_ = nullptr;
}

void H3DatagramAsyncSocket::onHeadersComplete(
    std::unique_ptr<proxygen::HTTPMessage> msg) noexcept {
  if (msg->getStatusCode() != 200) {
    closeWithError(
        {AsyncSocketException::INTERNAL_ERROR,
         fmt::format("HTTP Error: status code {}", msg->getStatusCode())});
    return;
  }

  // Upstream ready to receive buffers.
  for (auto& datagram : writeBuf_) {
    txn_->sendDatagram(std::move(datagram));
  }
  writeBuf_.clear();
}

void H3DatagramAsyncSocket::onDatagram(
    std::unique_ptr<folly::IOBuf> datagram) noexcept {

  if (!readCallback_) {
    if (readBuf_.size() < rcvBufPkts_) {
      // buffer until reads are resumed
      readBuf_.emplace_back(std::move(datagram));
    } else {
      VLOG_EVERY_N(2, 1000) << "Dropped incoming datagram.";
    }
    return;
  }

  deliverDatagram(std::move(datagram));
}

void H3DatagramAsyncSocket::deliverDatagram(
    std::unique_ptr<folly::IOBuf> datagram) noexcept {
  void* buf{nullptr};
  size_t len{0};
  ReadCallback::OnDataAvailableParams params;
  CHECK(readCallback_);
  CHECK(datagram);
  readCallback_->getReadBuffer(&buf, &len);
  if (buf == nullptr || len == 0 || len < datagram->computeChainDataLength()) {
    LOG(ERROR) << "Buffer too small to deliver "
               << datagram->computeChainDataLength() << " bytes datagram";
    return;
  }
  datagram->coalesce();
  memcpy(buf, datagram->data(), datagram->length());
  readCallback_->onDataAvailable(
      (upstreamSession_ ? upstreamSession_->getPeerAddress() : connectAddress_),
      size_t(datagram->length()),
      /*truncated*/ false,
      params);
}

void H3DatagramAsyncSocket::onBody(
    std::unique_ptr<folly::IOBuf> /*chain*/) noexcept {
}

void H3DatagramAsyncSocket::onTrailers(
    std::unique_ptr<proxygen::HTTPHeaders> /*trailers*/) noexcept {
}

void H3DatagramAsyncSocket::onEOM() noexcept {
  if (!readCallback_) {
    // close when resuming reads, after flushing buffered datagrams
    pendingEOM_ = true;
  } else {
    closeRead();
  }
}

void H3DatagramAsyncSocket::onUpgrade(
    proxygen::UpgradeProtocol /*protocol*/) noexcept {
  closeWithError(
      {AsyncSocketException::NOT_SUPPORTED, "onUpgrade not supported"});
}

void H3DatagramAsyncSocket::onError(
    const proxygen::HTTPException& error) noexcept {
  closeWithError({AsyncSocketException::INTERNAL_ERROR, error.describe()});
}

void H3DatagramAsyncSocket::onEgressPaused() noexcept {
}

void H3DatagramAsyncSocket::onEgressResumed() noexcept {
}

void H3DatagramAsyncSocket::startClient() {
  quic::TransportSettings transportSettings;
  transportSettings.datagramConfig.enabled = true;
  transportSettings.maxRecvPacketSize = options_.maxDatagramSize_;
  transportSettings.canIgnorePathMTU = true;
  if (sndBufPkts_ > 0) {
    transportSettings.datagramConfig.writeBufSize = sndBufPkts_;
  }
  if (rcvBufPkts_ > 0) {
    transportSettings.datagramConfig.readBufSize = rcvBufPkts_;
  }
  if (!upstreamSession_) {
    auto qEvb = std::make_shared<quic::FollyQuicEventBase>(evb_);
    auto sock = std::make_unique<quic::FollyQuicAsyncUDPSocket>(qEvb);
    auto fizzClientContext =
        quic::FizzClientQuicHandshakeContext::Builder()
            .setFizzClientContext(createFizzClientContext())
            .setCertificateVerifier(options_.certVerifier_)
            .build();
    auto client = std::make_shared<quic::QuicClientTransport>(
        qEvb, std::move(sock), fizzClientContext);
    CHECK(connectAddress_.isInitialized());
    client->addNewPeerAddress(connectAddress_);
    if (bindAddress_.isInitialized()) {
      client->setLocalAddress(bindAddress_);
    }
    client->setCongestionControllerFactory(
        std::make_shared<quic::DefaultCongestionControllerFactory>());
    transportSettings.datagramConfig.enabled = true;
    client->setTransportSettings(transportSettings);
    client->setSupportedVersions({quic::QuicVersion::MVFST});

    wangle::TransportInfo tinfo;
    upstreamSession_ =
        new proxygen::HQUpstreamSession(options_.txnTimeout_,
                                        options_.connectTimeout_,
                                        nullptr, // controller
                                        tinfo,
                                        nullptr); // codecfiltercallback
    upstreamSession_->setSocket(client);
    upstreamSession_->setConnectCallback(this);
    upstreamSession_->setInfoCallback(this);
    upstreamSession_->setEgressSettings(
        {{proxygen::SettingsId::_HQ_DATAGRAM, 1}});

    VLOG(4) << "connecting to " << connectAddress_.describe();
    upstreamSession_->startNow();
    client->start(upstreamSession_, upstreamSession_);
  }
}

std::shared_ptr<fizz::client::FizzClientContext>
H3DatagramAsyncSocket::createFizzClientContext() {
  auto ctx = std::make_shared<fizz::client::FizzClientContext>();

  if (options_.certAndKey_.has_value()) {
    std::string certData;
    folly::readFile(options_.certAndKey_->first.c_str(), certData);
    std::string keyData;
    folly::readFile(options_.certAndKey_->second.c_str(), keyData);
    auto cert = fizz::CertUtils::makeSelfCert(certData, keyData);
    ctx->setClientCertificate(std::move(cert));
  }

  std::vector<std::string> supportedAlpns = {proxygen::kH3FBCurrentDraft};
  ctx->setSupportedAlpns(supportedAlpns);
  ctx->setDefaultShares(
      {fizz::NamedGroup::x25519, fizz::NamedGroup::secp256r1});
  ctx->setSendEarlyData(false);
  return ctx;
}

ssize_t H3DatagramAsyncSocket::write(const folly::SocketAddress& address,
                                     const std::unique_ptr<folly::IOBuf>& buf) {
  if (!buf) {
    LOG(ERROR) << "Invalid write data";
    errno = EINVAL;
    return -1;
  }
  if (!connectAddress_.isInitialized()) {
    LOG(ERROR) << "Socket not connected. Must call connect()";
    errno = ENOTCONN;
    return -1;
  }
  // Can only write to the one address we are connected to
  if (address != connectAddress_) {
    LOG(ERROR) << "Socket can only write to address " << connectAddress_;
    errno = EINVAL;
    return -1;
  }
  auto size = buf->computeChainDataLength();
  if (!transportConnected_) {
    if (writeBuf_.size() < sndBufPkts_) {
      VLOG(10) << "Socket not connected yet. Buffering datagram";
      writeBuf_.emplace_back(buf->clone());
      return size;
    }
    LOG(ERROR) << "Socket write buffer is full. Discarding datagram";
    errno = ENOBUFS;
    return -1;
  }
  if (!txn_) {
    LOG(ERROR) << "Unable to create HTTP/3 transaction. Discarding datagram";
    errno = ECANCELED;
    return -1;
  }
  if (size > txn_->getDatagramSizeLimit()) {
    LOG(ERROR) << "Datagram too large len=" << size
               << " transport max datagram size len="
               << txn_->getDatagramSizeLimit() << ". Discarding datagram";
    errno = EMSGSIZE;
    return -1;
  }
  if (!txn_->sendDatagram(buf->clone())) {
    LOG(ERROR) << "Transport write buffer is full. Discarding datagram";
    // sendDatagram can only fail for exceeding the maximum size (checked
    // above) and if the write buffer is full
    errno = ENOBUFS;
    return -1;
  }
  return size;
}

void H3DatagramAsyncSocket::resumeRead(ReadCallback* cob) {
  // TODO: avoid re-entrancy
  if (inResumeRead_) {
    return;
  }
  SCOPE_EXIT {
    inResumeRead_ = false;
  };
  inResumeRead_ = true;
  readCallback_ = CHECK_NOTNULL(cob);
  folly::DelayedDestruction::DestructorGuard dg(this);
  // if there are buffered datagrams, deliver those first.
  auto it = readBuf_.begin();
  while (it != readBuf_.end()) {
    // the read callback could be reset from onDataAvailable
    if (readCallback_) {
      deliverDatagram(std::move(*it));
      it = readBuf_.erase(it);
    } else {
      return;
    }
  }
  // then, deliver errors
  if (pendingError_.has_value()) {
    auto err = *pendingError_;
    pendingError_ = folly::none;
    readCallback_->onReadError(err);
    closeRead();
  } else if (pendingEOM_) {
    // or close reads if EOM was seen already
    pendingEOM_ = false;
    closeRead();
  }
}
} // namespace proxygen
