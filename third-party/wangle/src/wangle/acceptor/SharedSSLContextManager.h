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
  explicit SharedSSLContextManager(
      std::shared_ptr<const ServerSocketConfig> config)
      : config_(std::move(config)), seeds_(config_->initialTicketSeeds) {}
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
  std::shared_ptr<const ServerSocketConfig> config_;
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
  explicit SharedSSLContextManagerImpl(
      std::shared_ptr<const ServerSocketConfig> config)
      : SharedSSLContextManager(std::move(config)) {
    try {
      createContextManagers(config_->sslContextConfigs, config_->sniConfigs);
      LOG(INFO) << "Initialized SSL context configs";
    } catch (const std::runtime_error& ex) {
      if (config_->strictSSL) {
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
      createContextManagers(config_->sslContextConfigs, config_->sniConfigs);
      updateAcceptors();
      LOG(INFO) << "Updated TLS ticket keys";

    } catch (const std::runtime_error& ex) {
      LOG(ERROR) << "Failed to re-configure TLS: " << ex.what()
                 << "will keep old config";
    }
  }

  /*
   * Updates the config and reloads shared fizz, SSL context data with the given
   * SSL context config.
   * If there exists SNI configs, existing SNIs will use the given ssl config.
   */
  void updateSSLConfigAndReloadContexts(SSLContextConfig ssl) override {
    try {
      // This API overwrites all existing SNI configs with the given ssl config.
      // In case of a need to update SNI configs; the caller need to supply
      // customized SSL context config for each SNI config; hence an API update
      // is needed. This is not currently supported as SNI configs are not used
      // in the contexts this API is used.
      createContextManagers({std::move(ssl)}, {});
      updateAcceptors();
      LOG(INFO) << "Updated Fizz and SSL context configs";
    } catch (const std::runtime_error& ex) {
      LOG(ERROR) << "Failed to re-configure TLS: " << ex.what()
                 << "will keep old config";
    }
  }

  /*
   * Reloads shared fizz and SSL contexts data
   */
  void reloadSSLContextConfigs() override {
    try {
      createContextManagers(config_->sslContextConfigs, config_->sniConfigs);
      updateAcceptors();
      LOG(INFO) << "Reloaded Fizz and SSL context configs";
    } catch (const std::runtime_error& ex) {
      LOG(ERROR) << "Failed to re-configure TLS: " << ex.what()
                 << "will keep old config";
    }
  }

 protected:
  /*
   * Creates certManager_, fizzContext_, and ctxManager_ with the given SSL
   * context configs and sni configs.
   */
  void createContextManagers(
      const std::vector<SSLContextConfig>& sslContextConfigs,
      const std::vector<SNIConfig>& sniConfigs) {
    if (config_->fizzConfig.enableFizz) {
      certManager_ = FizzConfigUtilT::createCertManager(
          sslContextConfigs,
          /* pwFactory = */ nullptr,
          config_->strictSSL);
      fizzContext_ = FizzConfigUtilT::createFizzContext(
          sslContextConfigs, config_->fizzConfig, config_->strictSSL);
      if (fizzContext_) {
        fizzContext_->setCertManager(certManager_);
        auto fizzTicketCipher = FizzConfigUtilT::createFizzTicketCipher(
            seeds_,
            config_->sslCacheOptions.sslCacheTimeout,
            config_->sslCacheOptions.handshakeValidity,
            fizzContext_->getFactoryPtr(),
            certManager_,
            getPskContext(*config_));
        fizzContext_->setTicketCipher(fizzTicketCipher);
      }
    }
    auto ctxManager = std::make_shared<SSLContextManager>(
        "vip_" + config_->name,
        SSLContextManagerSettings().setStrict(config_->strictSSL),
        nullptr);
    for (const auto& sslContextConfig : sslContextConfigs) {
      ctxManager->addSSLContextConfig(
          sslContextConfig,
          config_->sslCacheOptions,
          &seeds_,
          config_->bindAddress,
          cacheProvider_);
    }
    for (const auto& sniConfig : sniConfigs) {
      ctxManager->addSSLContextConfig(
          sniConfig.snis,
          sniConfig.contextConfig,
          config_->sslCacheOptions,
          &seeds_,
          config_->bindAddress,
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
