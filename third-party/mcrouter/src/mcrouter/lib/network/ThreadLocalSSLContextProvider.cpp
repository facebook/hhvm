/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ThreadLocalSSLContextProvider.h"

#include <folly/Singleton.h>
#include <folly/container/F14Map.h>
#include <folly/hash/Hash.h>
#include <folly/io/async/EventBaseLocal.h>
#include <folly/io/async/SSLContext.h>
#include <folly/io/async/SSLOptions.h>
#include <folly/portability/OpenSSL.h>
#include <folly/ssl/Init.h>
#include <wangle/client/ssl/SSLSessionCacheData.h>
#include <wangle/client/ssl/SSLSessionPersistentCache.h>
#include <wangle/ssl/SSLCacheOptions.h>
#include <wangle/ssl/SSLContextConfig.h>
#include <wangle/ssl/SSLSessionCacheManager.h>
#include <wangle/ssl/ServerSSLContext.h>
#include <wangle/ssl/TLSTicketKeyManager.h>
#include <wangle/ssl/TLSTicketKeySeeds.h>

#include "mcrouter/lib/fbi/cpp/LogFailure.h"
#include "mcrouter/lib/network/SecurityOptions.h"

using folly::SSLContext;

namespace facebook {
namespace memcache {

namespace {
/* Sessions are valid for upto 24 hours */
constexpr size_t kSessionLifeTime = 86400;
/* Handshakes are valid for 1 week */
constexpr size_t kHandshakeValidity = 604800;

struct ContextKey {
  folly::StringPiece pemCertPath;
  folly::StringPiece pemKeyPath;
  folly::StringPiece pemCaPath;
  bool requireClientVerification{false};
  SecurityMech mech{SecurityMech::TLS};

  bool operator==(const ContextKey& other) const {
    return pemCertPath == other.pemCertPath && pemKeyPath == other.pemKeyPath &&
        pemCaPath == other.pemCaPath &&
        requireClientVerification == other.requireClientVerification &&
        mech == other.mech;
  }
};

struct ClientContextInfo {
  std::string pemCertPath;
  std::string pemKeyPath;
  std::string pemCaPath;
  SecurityMech mech;

  std::shared_ptr<SSLContext> context;
  FizzContextAndVerifier fizzData;
  std::chrono::time_point<std::chrono::steady_clock> lastLoadTime;

  bool needsContext(
      std::chrono::time_point<std::chrono::steady_clock> now) const {
    constexpr auto kSslReloadInterval = std::chrono::minutes(30);
    if (!context) {
      return true;
    }
    return now - lastLoadTime > kSslReloadInterval;
  }

  bool needsFizzContext(
      std::chrono::time_point<std::chrono::steady_clock> now) const {
    constexpr auto kSslReloadInterval = std::chrono::minutes(30);
    if (!fizzData.first) {
      return true;
    }
    return now - lastLoadTime > kSslReloadInterval;
  }

  void setContext(
      std::shared_ptr<SSLContext> ctx,
      std::chrono::time_point<std::chrono::steady_clock> loadTime) {
    if (ctx) {
      context = std::move(ctx);
      lastLoadTime = loadTime;
    }
  }

  void setFizzData(
      FizzContextAndVerifier ctxAndVerifier,
      std::chrono::time_point<std::chrono::steady_clock> loadTime) {
    if (ctxAndVerifier.first) {
      fizzData = std::move(ctxAndVerifier);
      lastLoadTime = loadTime;
    }
  }
};

struct ServerContextInfo {
  std::string pemCertPath;
  std::string pemKeyPath;
  std::string pemCaPath;

  std::shared_ptr<SSLContext> context;
  std::shared_ptr<fizz::server::FizzServerContext> fizzContext;
  std::chrono::time_point<std::chrono::steady_clock> lastLoadTime;

  bool needsContexts(
      std::chrono::time_point<std::chrono::steady_clock> now) const {
    constexpr auto kSslReloadInterval = std::chrono::minutes(30);
    if (!context || !fizzContext) {
      return true;
    }
    return now - lastLoadTime > kSslReloadInterval;
  }

  void setContexts(
      std::shared_ptr<SSLContext> ctx,
      std::shared_ptr<fizz::server::FizzServerContext> fizzCtx,
      std::chrono::time_point<std::chrono::steady_clock> loadTime) {
    if (ctx && fizzCtx) {
      context = std::move(ctx);
      fizzContext = std::move(fizzCtx);
      lastLoadTime = loadTime;
    }
  }
};

struct ContextKeyHasher {
  size_t operator()(const ContextKey& key) const {
    return folly::Hash()(
        key.pemCertPath,
        key.pemKeyPath,
        key.pemCaPath,
        key.requireClientVerification,
        key.mech);
  }
};

void logCertFailure(
    folly::StringPiece name,
    folly::StringPiece path,
    const std::exception& ex) {
  LOG_FAILURE(
      "SSLCert",
      failure::Category::kBadEnvironment,
      "Failed to load {} from \"{}\", ex: {}",
      name,
      path,
      ex.what());
}

bool configureServerSSLContext(
    folly::SSLContext& sslContext,
    folly::StringPiece pemCertPath,
    folly::StringPiece certData,
    folly::StringPiece pemKeyPath,
    folly::StringPiece keyData,
    folly::StringPiece pemCaPath) {
  // Load certificate.
  try {
    sslContext.loadCertificateFromBufferPEM(certData);
  } catch (const std::exception& ex) {
    logCertFailure("certificate", pemCertPath, ex);
    return false;
  }

  // Load private key.
  try {
    sslContext.loadPrivateKeyFromBufferPEM(keyData);
  } catch (const std::exception& ex) {
    logCertFailure("private key", pemKeyPath, ex);
    return false;
  }

  // Load trusted certificates.
  try {
    sslContext.loadTrustedCertificates(pemCaPath.begin());
  } catch (const std::exception& ex) {
    logCertFailure("trusted certificates", pemCaPath, ex);
    return false;
  }

  // Load client CA list.
  try {
    sslContext.setSupportedClientCertificateAuthorityNamesFromFile(
        pemCaPath.begin());
  } catch (const std::exception& ex) {
    logCertFailure("client CA list", pemCaPath, ex);
    return false;
  }

  folly::ssl::setCipherSuites<folly::ssl::SSLServerOptions>(sslContext);

  return true;
}

using TicketCacheLayer = wangle::LRUPersistentCache<
    std::string,
    wangle::SSLSessionCacheData,
    folly::SharedMutex>;

// a SSL session cache that keys off string
class SSLTicketCache
    : public wangle::SSLSessionPersistentCacheBase<std::string> {
 public:
  explicit SSLTicketCache(std::shared_ptr<TicketCacheLayer> cache)
      : wangle::SSLSessionPersistentCacheBase<std::string>(std::move(cache)) {}

 protected:
  std::string getKey(const std::string& identity) const override {
    return identity;
  }
};

// global thread safe ticket cache
// TODO(jmswen) Try to come up with a cleaner approach here that doesn't require
// leaking.
folly::LeakySingleton<SSLTicketCache> ticketCache([] {
  // create cache layer of max size 100;
  auto cacheLayer = std::make_shared<TicketCacheLayer>(
      wangle::PersistentCacheConfig::Builder().setCapacity(100).build());
  cacheLayer->init();
  return new SSLTicketCache(std::move(cacheLayer));
});

std::shared_ptr<SSLContext> createServerSSLContext(
    folly::StringPiece pemCertPath,
    folly::StringPiece certData,
    folly::StringPiece pemKeyPath,
    folly::StringPiece keyData,
    folly::StringPiece pemCaPath,
    bool requireClientVerification,
    wangle::TLSTicketKeySeeds* ticketKeySeeds) {
  wangle::SSLContextConfig cfg;
  // don't need to set any certs on the cfg since the context is configured
  // in configureSSLContext;
  cfg.sessionCacheEnabled = true;
  cfg.sessionContext = "async-server";
  cfg.sessionTicketEnabled = ticketKeySeeds && ticketKeySeeds->isNotEmpty();

  // we'll use our own internal session cache instead of openssl's
  wangle::SSLCacheOptions cacheOpts;
  cacheOpts.sslCacheTimeout = std::chrono::seconds(kSessionLifeTime);
  cacheOpts.handshakeValidity = std::chrono::seconds(kHandshakeValidity);
  // defaults from wangle/acceptor/ServerSocketConfig.h
  cacheOpts.maxSSLCacheSize = 20480;
  cacheOpts.sslCacheFlushSize = 200;
  auto sslContext = std::make_shared<wangle::ServerSSLContext>();
  if (!configureServerSSLContext(
          *sslContext, pemCertPath, certData, pemKeyPath, keyData, pemCaPath)) {
    return nullptr;
  }
  sslContext->setServerECCurve("prime256v1");
#ifdef SSL_CTRL_SET_TLSEXT_TICKET_KEY_CB
  if (cfg.sessionTicketEnabled) {
    sslContext->setTicketHandler(
        wangle::TicketSeedHandler::fromSeeds(ticketKeySeeds));
  } else {
    sslContext->setOptions(SSL_OP_NO_TICKET);
  }
#endif
  sslContext->setupSessionCache(
      cfg,
      cacheOpts,
      nullptr, // external cache
      "async-server", // session context
      nullptr); // SSL Stats

#ifdef SSL_CTRL_SET_MAX_SEND_FRAGMENT
  // reduce send fragment size
  SSL_CTX_set_max_send_fragment(sslContext->getSSLCtx(), 8000);
#endif
  if (requireClientVerification) {
    sslContext->setVerificationOption(
        folly::SSLContext::VerifyClientCertificate::ALWAYS);
  } else {
    // request client certs and verify them if the client presents them.
    sslContext->setVerificationOption(
        folly::SSLContext::VerifyClientCertificate::IF_PRESENTED);
  }
#if FOLLY_OPENSSL_HAS_ALPN
  // servers can always negotiate this - it is up to the client to do so.
  sslContext->setAdvertisedNextProtocols(
      {kMcSecurityTlsToPlaintextProto.str()});
#endif
  return sslContext;
}

std::string readFile(folly::StringPiece path) {
  std::string data;
  if (path.empty()) {
    return data;
  }
  if (!folly::readFile(path.begin(), data)) {
    LOG_FAILURE(
        "SSLCert",
        failure::Category::kBadEnvironment,
        "Failed to load file from \"{}\"",
        path);
  }
  return data;
}

std::shared_ptr<SSLContext> createClientSSLContext(
    SecurityOptions opts,
    SecurityMech mech) {
  // The TLSv1_2 constructor argument sets the min version, while the call to
  // disableTLS13() sets the max version, also to TLS 1.2, essentially disabling
  // TLS 1.3 entirely. We need this particularly for StopTLS handling which
  // expects a TLS 1.2 connection.
  auto context = std::make_shared<ClientSSLContext>(
      ticketCache.get(), folly::SSLContext::SSLVersion::TLSv1_2);
  context->disableTLS13();
  // TODO: When enabling TLS 1.3, set TLS 1.3 ciphersuites from SSLCommonOptions
  auto ciphers = folly::ssl::SSLCommonOptions::ciphers();
  std::vector<std::string> cVec(ciphers.begin(), ciphers.end());
#if FOLLY_OPENSSL_HAS_ALPN
  if (mech == SecurityMech::TLS_TO_PLAINTEXT) {
    // Prepend ECDHE-RSA-NULL-SHA to make it obvious from the ClientHello
    // that we may not be using encryption. For this to work, we must set
    // the context's security level to 0. The default security level
    // used by OpenSSL prohibits weak ciphers from being sent in the
    // ClientHello.
    cVec.insert(cVec.begin(), "ECDHE-RSA-NULL-SHA");
    context->setAdvertisedNextProtocols(
        {kMcSecurityTlsToPlaintextProto.str(), "rs"});
#if FOLLY_OPENSSL_IS_110
    SSL_CTX_set_security_level(context->getSSLCtx(), 0);
#endif
  } else {
    // Thrift's Rocket transport requires an ALPN
    context->setAdvertisedNextProtocols({"rs"});
  }
#endif
  // note we use setCipherSuites instead of setClientOptions since client
  // options will enable false start by default.
  folly::ssl::setCipherSuites(*context, cVec);

  const auto& pemCertPath = opts.sslPemCertPath;
  const auto& pemKeyPath = opts.sslPemKeyPath;
  const auto& pemCaPath = opts.sslPemCaPath;
  if (!pemCertPath.empty() && !pemKeyPath.empty()) {
    try {
      context->loadCertificate(pemCertPath.c_str());
    } catch (const std::exception& ex) {
      logCertFailure("certificate", pemCertPath, ex);
      return nullptr;
    }
    // Load private key.
    try {
      context->loadPrivateKey(pemKeyPath.c_str());
    } catch (const std::exception& ex) {
      logCertFailure("private key", pemKeyPath, ex);
      return nullptr;
    }
  }
  if (!pemCaPath.empty()) {
    // we are going to verify server certificates
    context->loadTrustedCertificates(pemCaPath.c_str());
    // only verify that the server is trusted - no peer name verification yet
    context->authenticate(true, false);
    context->setVerificationOption(
        folly::SSLContext::SSLVerifyPeerEnum::VERIFY);
  }
  return context;
}

ClientContextInfo& getClientContextInfo(
    folly::EventBase& evb,
    const SecurityOptions& opts,
    SecurityMech mech) {
  static folly::EventBaseLocal<folly::F14FastMap<
      ContextKey,
      std::unique_ptr<ClientContextInfo>,
      ContextKeyHasher>>
      localContexts;

  ContextKey key;
  key.pemCertPath = opts.sslPemCertPath;
  key.pemKeyPath = opts.sslPemKeyPath;
  key.pemCaPath = opts.sslPemCaPath;
  key.mech = mech;

  auto& map = localContexts.try_emplace(evb);
  auto iter = map.find(key);
  if (iter == map.end()) {
    // Copy strings.
    auto info = std::make_unique<ClientContextInfo>();
    info->pemCertPath = opts.sslPemCertPath;
    info->pemKeyPath = opts.sslPemKeyPath;
    info->pemCaPath = opts.sslPemCaPath;
    info->mech = mech;

    // Point all StringPiece's to our own strings.
    key.pemCertPath = info->pemCertPath;
    key.pemKeyPath = info->pemKeyPath;
    key.pemCaPath = info->pemCaPath;
    iter = map.try_emplace(key, std::move(info)).first;
  }

  return *iter->second;
}

ServerContextInfo& getServerContextInfo(
    folly::EventBase& evb,
    folly::StringPiece pemCertPath,
    folly::StringPiece pemKeyPath,
    folly::StringPiece pemCaPath,
    bool requireClientVerification) {
  static folly::EventBaseLocal<folly::F14FastMap<
      ContextKey,
      std::unique_ptr<ServerContextInfo>,
      ContextKeyHasher>>
      localContexts;

  ContextKey key;
  key.pemCertPath = pemCertPath;
  key.pemKeyPath = pemKeyPath;
  key.pemCaPath = pemCaPath;
  key.requireClientVerification = requireClientVerification;

  auto& map = localContexts.try_emplace(evb);
  auto iter = map.find(key);
  if (iter == map.end()) {
    // Copy strings.
    auto info = std::make_unique<ServerContextInfo>();
    info->pemCertPath = pemCertPath.toString();
    info->pemKeyPath = pemKeyPath.toString();
    info->pemCaPath = pemCaPath.toString();

    // Point all StringPiece's to our own strings.
    key.pemCertPath = info->pemCertPath;
    key.pemKeyPath = info->pemKeyPath;
    key.pemCaPath = info->pemCaPath;
    iter = map.try_emplace(key, std::move(info)).first;
  }

  return *iter->second;
}

} // namespace

bool isAsyncSSLSocketMech(SecurityMech mech) {
  return mech == SecurityMech::TLS || mech == SecurityMech::TLS_TO_PLAINTEXT ||
      mech == SecurityMech::KTLS12;
}

FizzContextAndVerifier getFizzClientConfig(
    folly::EventBase& evb,
    const SecurityOptions& opts) {
  auto& info = getClientContextInfo(evb, opts, SecurityMech::TLS13_FIZZ);
  auto now = std::chrono::steady_clock::now();
  if (info.needsFizzContext(now)) {
    auto certData = readFile(opts.sslPemCertPath);
    auto keyData = readFile(opts.sslPemKeyPath);
    auto fizzData = createClientFizzContextAndVerifier(
        std::move(certData),
        std::move(keyData),
        opts.sslPemCaPath,
        opts.tlsPreferOcbCipher);
    info.setFizzData(std::move(fizzData), now);
  }
  return info.fizzData;
}

std::shared_ptr<folly::SSLContext> getClientContext(
    folly::EventBase& evb,
    const SecurityOptions& opts,
    SecurityMech mech) {
  if (!isAsyncSSLSocketMech(mech)) {
    LOG_FAILURE(
        "SSLConfig",
        failure::Category::kInvalidOption,
        "getClientContext specified invalid security mech: {}",
        static_cast<uint8_t>(mech));
    return nullptr;
  }
  auto& info = getClientContextInfo(evb, opts, mech);
  auto now = std::chrono::steady_clock::now();
  if (info.needsContext(now)) {
    auto ctx = createClientSSLContext(opts, mech);
    info.setContext(std::move(ctx), now);
  }
  return info.context;
}

ServerContextPair getServerContexts(
    folly::EventBase& evb,
    folly::StringPiece pemCertPath,
    folly::StringPiece pemKeyPath,
    folly::StringPiece pemCaPath,
    bool requireClientCerts,
    folly::Optional<wangle::TLSTicketKeySeeds> seeds,
    bool preferOcbCipher) {
  auto& info = getServerContextInfo(
      evb, pemCertPath, pemKeyPath, pemCaPath, requireClientCerts);
  auto now = std::chrono::steady_clock::now();
  if (info.needsContexts(now)) {
    auto certData = readFile(pemCertPath);
    auto keyData = readFile(pemKeyPath);
    auto ctx = createServerSSLContext(
        pemCertPath,
        certData,
        pemKeyPath,
        keyData,
        pemCaPath,
        requireClientCerts,
        seeds.get_pointer());
    auto fizzCtx = createFizzServerContext(
        pemCertPath,
        certData,
        pemKeyPath,
        keyData,
        pemCaPath,
        requireClientCerts,
        preferOcbCipher,
        seeds.get_pointer());
    info.setContexts(std::move(ctx), std::move(fizzCtx), now);
  }
  return ServerContextPair(info.context, info.fizzContext);
}

} // namespace memcache
} // namespace facebook
