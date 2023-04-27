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

#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/CertManager.h>
#include <fizz/server/TicketCipher.h>
#include <fizz/server/TicketCodec.h>
#include <fizz/server/TicketTypes.h>
#include <wangle/acceptor/Acceptor.h>
#include <wangle/acceptor/FizzConfigUtil.h>
#include <wangle/acceptor/ServerSocketConfig.h>
#include <wangle/ssl/SSLContextManager.h>
#include <wangle/ssl/TLSTicketKeySeeds.h>

namespace wangle {

/*
 * A class that stores and manages the TLS data (cert manager, SSL context and
 * Fizz context) shared among Acceptors.
 */

class SharedSSLContextManager {
 public:
  SharedSSLContextManager(ServerSocketConfig config)
      : config_(config), seeds_(config.initialTicketSeeds) {}
  virtual ~SharedSSLContextManager() = default;

  virtual std::shared_ptr<fizz::server::CertManager> getCertManager() {
    return certManager_;
  }

  virtual std::shared_ptr<SSLContextManager> getContextManager() {
    return ctxManager_;
  }

  virtual std::shared_ptr<const fizz::server::FizzServerContext>
  getFizzContext() {
    return fizzContext_;
  }

  virtual void addAcceptor(std::shared_ptr<wangle::Acceptor> acc) {
    acceptors_.emplace(acc.get(), acc);
  }

  virtual void setSSLCacheProvider(
      const std::shared_ptr<SSLCacheProvider>& cacheProvider) {
    cacheProvider_ = cacheProvider;
  }

  virtual void updateTLSTicketKeys(TLSTicketKeySeeds seeds) = 0;
  virtual void updateSSLConfigAndReloadContexts(SSLContextConfig ssl) = 0;
  virtual void reloadSSLContextConfigs() = 0;

 protected:
  wangle::ServerSocketConfig config_;
  std::shared_ptr<fizz::server::CertManager> certManager_;
  std::shared_ptr<wangle::SSLContextManager> ctxManager_;
  std::shared_ptr<fizz::server::FizzServerContext> fizzContext_;
  TLSTicketKeySeeds seeds_;
  std::unordered_map<void*, std::weak_ptr<wangle::Acceptor>> acceptors_;
  std::shared_ptr<SSLCacheProvider> cacheProvider_;
};

/*
 * An implementation of SharedSSLContextManager.
 */
template <typename FizzConfigUtilT>
class SharedSSLContextManagerImpl : public SharedSSLContextManager {
 public:
  explicit SharedSSLContextManagerImpl(ServerSocketConfig config)
      : SharedSSLContextManager(config) {
    try {
      reloadContexts();
      LOG(INFO) << "Initialized SSL context configs";
    } catch (const std::runtime_error& ex) {
      if (config_.strictSSL) {
        throw;
      } else {
        if (ctxManager_) {
          ctxManager_->clear();
        }
        LOG(ERROR) << "Failed to initialize SSL context configs: " << ex.what();
      }
    }
  }

  /*
   * Reloads tls secrets
   */
  void updateTLSTicketKeys(TLSTicketKeySeeds seeds) override {
    try {
      seeds_ = seeds;
      reloadContexts();
      updateAcceptors();
      LOG(INFO) << "Updated TLS ticket keys";

    } catch (const std::runtime_error& ex) {
      LOG(ERROR) << "Failed to re-configure TLS: " << ex.what()
                 << "will keep old config";
    }
  }

  /*
   * Updates the config and reloads shared fizz, SSL contexts data
   */
  void updateSSLConfigAndReloadContexts(SSLContextConfig ssl) override {
    for (auto& sslContext : config_.sslContextConfigs) {
      sslContext = ssl;
    }
    reloadSSLContextConfigs();
  }

  /*
   * Reloads shared fizz and SSL contexts data
   */
  void reloadSSLContextConfigs() override {
    try {
      reloadContexts();
      updateAcceptors();
      LOG(INFO) << "Reloaded Fizz and SSL context configs";

    } catch (const std::runtime_error& ex) {
      LOG(ERROR) << "Failed to re-configure TLS: " << ex.what()
                 << "will keep old config";
    }
  }

 protected:
  // recreates contexts using config_ and seeds_
  void reloadContexts() {
    if (config_.fizzConfig.enableFizz) {
      certManager_ = FizzConfigUtilT::createCertManager(config_, nullptr);
      fizzContext_ = FizzConfigUtilT::createFizzContext(config_);
      if (fizzContext_) {
        fizzContext_->setCertManager(certManager_);
        auto fizzTicketCipher = FizzConfigUtilT::createFizzTicketCipher(
            seeds_,
            config_.sslCacheOptions.sslCacheTimeout,
            config_.sslCacheOptions.handshakeValidity,
            fizzContext_->getFactoryPtr(),
            certManager_,
            getPskContext(config_));
        fizzContext_->setTicketCipher(fizzTicketCipher);
      }
    }
    auto ctxManager = std::make_shared<SSLContextManager>(
        "vip_" + config_.name, config_.strictSSL, nullptr);
    for (const auto& sslCtxConfig : config_.sslContextConfigs) {
      ctxManager->addSSLContextConfig(
          sslCtxConfig,
          config_.sslCacheOptions,
          &seeds_,
          config_.bindAddress,
          cacheProvider_);
    }
    ctxManager_ = std::move(ctxManager);
  }

  void updateAcceptors() {
    auto certManager = certManager_;
    auto ctxManager = ctxManager_;
    auto fizzContext = fizzContext_;

    for (auto weakAcceptor : acceptors_) {
      auto acceptor = weakAcceptor.second.lock();
      if (!acceptor) {
        continue;
      }
      auto evb = acceptor->getEventBase();
      if (!evb) {
        continue;
      }
      evb->runInEventBaseThread(
          [acceptor, certManager, ctxManager, fizzContext] {
            acceptor->resetSSLContextConfigs(
                certManager, ctxManager, fizzContext);
          });
    }
  }

  std::string getPskContext(const ServerSocketConfig& config) {
    std::string pskContext;
    if (!config.sslContextConfigs.empty()) {
      pskContext = config.sslContextConfigs.front().sessionContext.value_or("");
    }
    return pskContext;
  }
};

class AcceptorFactorySharedSSLContext : public AcceptorFactory {
 public:
  virtual std::shared_ptr<SharedSSLContextManager>
  initSharedSSLContextManager() = 0;

 protected:
  std::shared_ptr<wangle::SharedSSLContextManager> sharedSSLContextManager_;
};

} // namespace wangle
