/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#include <folly/SharedMutex.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/SSLContext.h>

#include <glog/logging.h>
#include <wangle/acceptor/SSLContextSelectionMisc.h>
#include <wangle/ssl/PasswordInFileFactory.h>
#include <wangle/ssl/SSLContextConfig.h>
#include <wangle/ssl/SSLSessionCacheManager.h>
#include <wangle/ssl/TLSTicketKeySeeds.h>
#include <list>
#include <memory>
#include <vector>

namespace folly {

class SocketAddress;
class SSLContext;

} // namespace folly

namespace wangle {

class ClientHelloExtStats;
struct SSLCacheOptions;
class SSLStats;
class TLSTicketKeyManager;
struct TLSTicketKeySeeds;
class ServerSSLContext;

// SSLContextManager represents all of the different server side
// SSL configurations behind a VIP.
//
// One VIP can serve multiple, independent TLS configurations by
// selecting a configuration by the ServerNameIndication extension
// on the initial ClientHello.
//
// There is 1 SSLContextManager per acceptor.
class SSLContextManager {
 private:
  // SslContexts is an internal, private structure used for
  // storing all of the different TLS configurations behind
  // an acceptor.
  //
  //   1 Acceptor          : 1 SSLContextManager
  //   1 SSLContextManager : N SSLContexts.
  //
  // Declaration is in SSLContextManager.cpp
  class SslContexts;

 public:
  /**
   * Provide ability to perform explicit client certificate
   * verification
   * If this callback is used, it becomes the responsibility
   * of the callback to ensure that any verification is done (e.g. via
   * SSL_CTX_set_verify(...). We remove the verification option
   * on the SSLContext wrapper so that no other layer decides to set its own
   * verification callback overriding the callbacks behavior. E.g.
   * AsyncSSLSocket's verify callback wiping out any existing cb set.
   */
  struct ClientCertVerifyCallback {
    // no-op. Should be overridden if actual
    // verification is required. This should assign the callback functions
    // to the context, without altering the callback itself.
    virtual void attachSSLContext(
        const std::shared_ptr<folly::SSLContext>& sslCtx) const = 0;
    virtual ~ClientCertVerifyCallback() {}
  };

  explicit SSLContextManager(
      const std::string& vipName,
      bool strict,
      SSLStats* stats);
  virtual ~SSLContextManager();

  /**
   * Add a new X509 to SSLContextManager.  The details of a X509
   * is passed as a SSLContextConfig object.
   *
   * @param ctxConfig     Details of a X509, its private key, password, etc.
   * @param cacheOptions  Options for how to do session caching.
   * @param ticketSeeds   If non-null, the initial ticket key seeds to use.
   * @param vipAddress    Which VIP are the X509(s) used for? It is only for
   *                      for user friendly log message
   * @param externalCache Optional external provider for the session cache;
   *                      may be null
   */
  void addSSLContextConfig(
      const SSLContextConfig& ctxConfig,
      const SSLCacheOptions& cacheOptions,
      const TLSTicketKeySeeds* ticketSeeds,
      const folly::SocketAddress& vipAddress,
      const std::shared_ptr<SSLCacheProvider>& externalCache);

  /**
   * Resets SSLContextManager with new X509s
   *
   * @param ctxConfigs    Details of a X509s, private key, password, etc.
   * @param cacheOptions  Options for how to do session caching.
   * @param ticketSeeds   If non-null, and ticketHandler_ is a
   *                      TLSTicketKeyManager, the initial ticket key seeds to
   *                      use.
   * @param vipAddress    Which VIP are the X509(s) used for? It is only for
   *                      for user friendly log message
   * @param externalCache Optional external provider for the session cache;
   *                      may be null
   */
  void resetSSLContextConfigs(
      const std::vector<SSLContextConfig>& ctxConfig,
      const SSLCacheOptions& cacheOptions,
      const TLSTicketKeySeeds* ticketSeeds,
      const folly::SocketAddress& vipAddress,
      const std::shared_ptr<SSLCacheProvider>& externalCache);

  /**
   * Remove SSLContextConfig of the given key. Note that to remove the context
   * for wildcard domain, call either
   * removeSSLContextConfigByDomainName("*.example.com") or
   * removeSSLContextConfig(SSLContextKey(".example.com")).
   */
  void removeSSLContextConfigByDomainName(const std::string& domainName);
  void removeSSLContextConfig(const SSLContextKey& key);

  /**
   * Clears all ssl contexts
   */
  void clear();

  /**
   * Get the default SSL_CTX for a VIP
   */
  std::shared_ptr<folly::SSLContext> getDefaultSSLCtx() const;

  /**
   * Search first by exact domain, then by one level up
   */
  std::shared_ptr<folly::SSLContext> getSSLCtx(const SSLContextKey& key) const;

  /**
   * Search by the _one_ level up subdomain
   */
  std::shared_ptr<folly::SSLContext> getSSLCtxBySuffix(
      const SSLContextKey& key) const;

  /**
   * Search by the full-string domain name
   */
  std::shared_ptr<folly::SSLContext> getSSLCtxByExactDomain(
      const SSLContextKey& key) const;

  /**
   * Updates ticket keys in ticket handler if using TLSTicketKeyManager,
   * otherwise a no-op.
   */
  void reloadTLSTicketKeys(
      const std::vector<std::string>& oldSeeds,
      const std::vector<std::string>& currentSeeds,
      const std::vector<std::string>& newSeeds);

  void setSSLStats(SSLStats* stats) {
    stats_ = stats;
  }

  /**
   * SSLContextManager only collects SNI stats now
   */
  void setClientHelloExtStats(ClientHelloExtStats* stats);

  /*
   * Please read class header before setting this callback, it may have
   * unintended conseqeunces, as you are now fully responsible for any
   * verification.
   */
  void setClientVerifyCallback(std::unique_ptr<ClientCertVerifyCallback> cb) {
    clientCertVerifyCallback_ = std::move(cb);
  }

  void setPasswordFactory(std::shared_ptr<PasswordInFileFactory> factory) {
    passwordFactory_ = std::move(factory);
  }

 protected:
  // Return value indicates if any certificates were loaded. If not, the cert
  // manager skips this context config. Allows for contexts using certs only
  // supported by TLS 1.3/Fizz. Note: This is different than an error
  // occurring. If an error occurs, it ought to throw in here. Returning false
  // means the context is empty *due to a policy decision*.
  virtual bool loadCertKeyPairsInSSLContext(
      const std::shared_ptr<folly::SSLContext>&,
      const SSLContextConfig&,
      std::string& commonName) const;

  virtual bool loadCertKeyPairsInSSLContextExternal(
      const std::shared_ptr<folly::SSLContext>&,
      const SSLContextConfig&,
      std::string& /* commonName */) const {
    LOG(FATAL) << "Unsupported in base SSLContextManager";
    // unreachable
    return false;
  }

  virtual void overrideConfiguration(
      const std::shared_ptr<folly::SSLContext>&,
      const SSLContextConfig&) const {}

  std::string vipName_;
  SSLStats* stats_{nullptr};

  /**
   * Insert a SSLContext by domain name.
   */
  void insertSSLCtxByDomainName(
      const std::string& dn,
      std::shared_ptr<folly::SSLContext> sslCtx,
      CertCrypto certCrypto = CertCrypto::BEST_AVAILABLE,
      bool defaultFallback = false);

  void loadCertsFromFiles(
      const std::shared_ptr<folly::SSLContext>& sslCtx,
      const SSLContextConfig::CertificateInfo& cert) const;

  /**
   * Helper used to verify that all certificates installed for a single
   * `folly::SSLContext` convey the same identities (with the possibility of
   * different SubjectPublicKeyInfos)
   */
  void verifyCertNames(
      const std::shared_ptr<folly::SSLContext>& sslCtx,
      const std::string& description,
      std::string& groupIdentity,
      std::unique_ptr<std::list<std::string>>& subjectAltName,
      const std::string& lastCertPath,
      bool firstCert) const;

  void addServerContext(std::shared_ptr<ServerSSLContext> sslCtx);

 private:
  SSLContextManager(const SSLContextManager&) = delete;

  std::shared_ptr<SslContexts> contexts_;
  ClientHelloExtStats* clientHelloTLSExtStats_{nullptr};
  bool strict_{true};
  std::unique_ptr<ClientCertVerifyCallback> clientCertVerifyCallback_{nullptr};
  std::shared_ptr<ServerSSLContext> defaultCtx_;
  std::shared_ptr<PasswordInFileFactory> passwordFactory_{nullptr};
};

} // namespace wangle
