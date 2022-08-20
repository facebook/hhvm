/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <folly/Function.h>
#include <folly/Optional.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <mcrouter/lib/network/SecurityOptions.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace facebook {
namespace memcache {
/**
 * A utility class for SSL related items in McRouter clients and servers.
 * Manages app-specific SSL routines that are used by clients and servers during
 * and immediately after SSL handshakes.
 */
class McSSLUtil {
 public:
  using SSLVerifyFunction =
      folly::Function<bool(folly::AsyncSSLSocket*, bool, X509_STORE_CTX*)
                          const noexcept>;

  using DropCertificateX509PayloadFunction =
      folly::Function<bool(folly::AsyncSSLSocket&) const noexcept>;

  static const std::string kTlsToPlainProtocolName;

  static bool verifySSLWithDefaultBehavior(
      folly::AsyncSSLSocket*,
      bool,
      X509_STORE_CTX*) noexcept;

  using SSLToKtlsFunction =
      folly::Function<folly::AsyncTransportWrapper::UniquePtr(
          folly::AsyncTransportWrapper& sock) noexcept>;

  using KtlsStatsFunction =
      folly::Function<folly::Optional<SecurityTransportStats>(
          const folly::AsyncTransportWrapper& sock) noexcept>;

  /**
   * Install a function that can potentially convert a transport wrapper to
   * a kTLS socket and then retrieve stats from it, respectively.
   * The installed functions will be called from multiple threads so it must be
   * threadsafe.  The install function should be called once, typicaly around
   * application init and before any requests have been served or made.
   */
  static void setApplicationKtlsFunctions(
      SSLToKtlsFunction toKtlsFunc,
      KtlsStatsFunction statsFunc);

  /**
   * Install an app specific SSL verifier.  This function will be called
   * from multiple threads, so it must be threadsafe.
   * This function should be called once, typically around application init and
   * before the server has received any requests.
   */
  static void setApplicationSSLVerifier(SSLVerifyFunction func);

  /**
   * Install an identity hook to extract identities from a Cpp2ConnContext
   */
  static void setClientIdentityHook(apache::thrift::ClientIdentityHook func);

  /*
   * Certificate x509 payload drop function
   */
  static void setDropCertificateX509PayloadFunction(
      DropCertificateX509PayloadFunction func);

  /**
   * Verify an SSL connection.  If no application verifier is set, the default
   * verifier is used.
   */
  static bool verifySSL(folly::AsyncSSLSocket*, bool, X509_STORE_CTX*) noexcept;

  /* Extracts identities from a Transport into a Cpp2ConnectionContext */
  static apache::thrift::ClientIdentityHook getClientIdentityHook() noexcept;

  /**
   * Check if the ssl connection successfully negotiated falling back to
   * plaintext
   */
  static bool negotiatedPlaintextFallback(
      const folly::AsyncSSLSocket& sock) noexcept;

  /**
   * Move the existing ssl socket to plaintext if "mc_tls_to_pt" was
   * successfully negotiated.  Return value of nullptr means unable to
   * move.  If non null, the returned transport should be used and sock
   * has been detached.  The underlying FD will not change.
   * The returned wrapper will wrap a transport inheriting AsyncSocket.
   */
  static folly::AsyncTransportWrapper::UniquePtr moveToPlaintext(
      folly::AsyncSSLSocket& sock) noexcept;

  /**
   * Drops certificate x509 payload after connection is established
   */
  static void dropCertificateX509Payload(folly::AsyncSSLSocket& sock) noexcept;

  /**
   * Move the existing transport to kTLS if possible.  Return value of
   * nullptr means kTLS could not be used (either the negotiated cipher isn't
   * supported or the kernel doesn't, etc.).  The underlying FD will not
   * change.  The TLS handshake should have already occurred on sock.
   * The returned wrapper will wrap a transport inheriting AsyncSocket.
   */
  static folly::AsyncTransportWrapper::UniquePtr moveToKtls(
      folly::AsyncTransportWrapper& sock) noexcept;

  /**
   * If the underlying transport is a kTLS transport, get stats for it.
   * A result of none implies fallback to ktls failed in a previous step
   * or no stats function was installed.
   */
  static folly::Optional<SecurityTransportStats> getKtlsStats(
      const folly::AsyncTransportWrapper& sock) noexcept;
};
} // namespace memcache
} // namespace facebook
