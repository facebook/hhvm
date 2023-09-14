/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "McSSLUtil.h"

#include <folly/SharedMutex.h>
#include <folly/io/async/ssl/BasicTransportCertificate.h>
#include <folly/io/async/ssl/OpenSSLUtils.h>
#include <mcrouter/lib/network/TlsToPlainTransport.h>

#include "mcrouter/lib/fbi/cpp/LogFailure.h"

namespace facebook {
namespace memcache {

namespace {
static folly::SharedMutex& getMutex() {
  static folly::SharedMutex MUTEX;
  return MUTEX;
}

static McSSLUtil::SSLVerifyFunction& getAppFuncRef() {
  static McSSLUtil::SSLVerifyFunction VERIFIER;
  return VERIFIER;
}

static McSSLUtil::SSLToKtlsFunction& getKtlsFuncRef() {
  static McSSLUtil::SSLToKtlsFunction KTLSFUNC;
  return KTLSFUNC;
}

static McSSLUtil::DropCertificateX509PayloadFunction&
getDropCertificateX509PayloadFuncRef() {
  static McSSLUtil::DropCertificateX509PayloadFunction DROPCERTFUNC;
  return DROPCERTFUNC;
}

static McSSLUtil::KtlsStatsFunction& getKtlsStatsFuncRef() {
  static McSSLUtil::KtlsStatsFunction KTLSFUNC;
  return KTLSFUNC;
}
static apache::thrift::ClientIdentityHook& getClientIdentityHookFuncRef() {
  static apache::thrift::ClientIdentityHook CLIENTIDENTITYHOOKFUNC;
  return CLIENTIDENTITYHOOKFUNC;
}
} // namespace

const std::string McSSLUtil::kTlsToPlainProtocolName = "stopTLS";

bool McSSLUtil::verifySSLWithDefaultBehavior(
    folly::AsyncSSLSocket*,
    bool preverifyOk,
    X509_STORE_CTX* ctx) noexcept {
  if (!preverifyOk) {
    return false;
  }
  // XXX I'm assuming that this will be the case as a result of
  // preverifyOk being true
  DCHECK(X509_STORE_CTX_get_error(ctx) == X509_V_OK);

  // So the interesting thing is that this always returns the depth of
  // the cert it's asking you to verify, and the error_ assumes to be
  // just a poorly named function.
  auto certDepth = X509_STORE_CTX_get_error_depth(ctx);

  // Depth is numbered from the peer cert going up.  For anything in the
  // chain, let's just leave it to openssl to figure out it's validity.
  // We may want to limit the chain depth later though.
  if (certDepth != 0) {
    return preverifyOk;
  }

  auto cert = X509_STORE_CTX_get_current_cert(ctx);
  sockaddr_storage addrStorage;
  socklen_t addrLen = 0;
  if (!folly::ssl::OpenSSLUtils::getPeerAddressFromX509StoreCtx(
          ctx, &addrStorage, &addrLen)) {
    return false;
  }
  return folly::ssl::OpenSSLUtils::validatePeerCertNames(
      cert, reinterpret_cast<sockaddr*>(&addrStorage), addrLen);
}

void McSSLUtil::setApplicationKtlsFunctions(
    SSLToKtlsFunction func,
    KtlsStatsFunction statsFunc) {
  folly::SharedMutex::WriteHolder wh(getMutex());
  getKtlsFuncRef() = std::move(func);
  getKtlsStatsFuncRef() = std::move(statsFunc);
}

void McSSLUtil::setApplicationSSLVerifier(SSLVerifyFunction func) {
  folly::SharedMutex::WriteHolder wh(getMutex());
  getAppFuncRef() = std::move(func);
}

void McSSLUtil::setClientIdentityHook(apache::thrift::ClientIdentityHook func) {
  folly::SharedMutex::WriteHolder wh(getMutex());
  getClientIdentityHookFuncRef() = std::move(func);
}

void McSSLUtil::setDropCertificateX509PayloadFunction(
    DropCertificateX509PayloadFunction func) {
  folly::SharedMutex::WriteHolder wh(getMutex());
  getDropCertificateX509PayloadFuncRef() = std::move(func);
}

bool McSSLUtil::verifySSL(
    folly::AsyncSSLSocket* sock,
    bool preverifyOk,
    X509_STORE_CTX* ctx) noexcept {
  // It should be fine to hold onto the read holder since writes to this
  // will typically happen at app startup.
  folly::SharedMutex::ReadHolder rh(getMutex());
  auto& func = getAppFuncRef();
  if (!func) {
    return verifySSLWithDefaultBehavior(sock, preverifyOk, ctx);
  }
  return func(sock, preverifyOk, ctx);
}

apache::thrift::ClientIdentityHook McSSLUtil::getClientIdentityHook() noexcept {
  return getClientIdentityHookFuncRef();
}

bool McSSLUtil::negotiatedPlaintextFallback(
    const folly::AsyncSSLSocket& sock) noexcept {
  // get the negotiated protocol
  auto nextProto = sock.getApplicationProtocol();
  return nextProto == kMcSecurityTlsToPlaintextProto;
}

folly::AsyncTransportWrapper::UniquePtr McSSLUtil::moveToPlaintext(
    folly::AsyncSSLSocket& sock) noexcept {
  if (!negotiatedPlaintextFallback(sock)) {
    return nullptr;
  }

  /// get the addresses out of the sock
  folly::SocketAddress local;
  folly::SocketAddress peer;
  auto exceptionWrapper = folly::try_and_catch([&sock, &local, &peer] {
    sock.getLocalAddress(&local);
    sock.getPeerAddress(&peer);
  });
  if (exceptionWrapper) {
    LOG_FAILURE(
        "mcrouter",
        failure::Category::kSystemError,
        "Failed to get socket address: {}",
        exceptionWrapper.what());
    return nullptr;
  }

  // Get the stats for the socket
  SecurityTransportStats stats;
  stats.tfoSuccess = sock.getTFOSucceded();
  stats.tfoAttempted = sock.getTFOAttempted();
  stats.tfoFinished = sock.getTFOFinished();
  stats.sessionReuseSuccess = sock.getSSLSessionReused();
  stats.sessionReuseAttempted = sock.sessionResumptionAttempted();

  // We need to mark the SSL as shutdown here, but need to do
  // it quietly so no alerts are sent over the wire.
  // This prevents SSL thinking we are shutting down in a bad state
  // when AsyncSSLSocket is cleaned up, which could remove the session
  // from the session cache
  auto ssl = const_cast<SSL*>(sock.getSSL());
  SSL_set_quiet_shutdown(ssl, 1);
  SSL_shutdown(ssl);

  // fallback to plaintext
  auto selfCert =
      folly::ssl::BasicTransportCertificate::create(sock.getSelfCertificate());
  auto peerCert =
      folly::ssl::BasicTransportCertificate::create(sock.getPeerCertificate());
  auto evb = sock.getEventBase();
  auto zcId = sock.getZeroCopyBufId();
  auto fd = sock.detachNetworkSocket();

  TlsToPlainTransport::UniquePtr res(new TlsToPlainTransport(evb, fd, zcId));
  res->setSelfCertificate(std::move(selfCert));
  res->setPeerCertificate(std::move(peerCert));
  res->setStats(stats);
  res->setAddresses(std::move(local), std::move(peer));
  return res;
}

folly::AsyncTransportWrapper::UniquePtr McSSLUtil::moveToKtls(
    folly::AsyncTransportWrapper& sock) noexcept {
  folly::SharedMutex::ReadHolder rh(getMutex());
  auto& func = getKtlsFuncRef();
  if (func) {
    return func(sock);
  }
  return nullptr;
}

void McSSLUtil::dropCertificateX509Payload(
    folly::AsyncSSLSocket& sock) noexcept {
  folly::SharedMutex::ReadHolder rh(getMutex());
  auto& func = getDropCertificateX509PayloadFuncRef();
  if (func) {
    func(sock);
  }
}

folly::Optional<SecurityTransportStats> McSSLUtil::getKtlsStats(
    const folly::AsyncTransportWrapper& sock) noexcept {
  auto& func = getKtlsStatsFuncRef();
  if (func) {
    return func(sock);
  }
  return folly::none;
}
} // namespace memcache
} // namespace facebook
