/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <vector>

#include <boost/filesystem/operations.hpp>

#include <folly/DynamicConverter.h>
#include <folly/MapUtil.h>
#include <folly/Singleton.h>
#include <folly/Synchronized.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/executors/ThreadPoolExecutor.h>
#include <folly/fibers/FiberManager.h>
#include <folly/io/async/EventBase.h>

#include "mcrouter/AsyncWriter.h"
#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/ExecutorObserver.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/McrouterLogger.h"
#include "mcrouter/McrouterManager.h"
#include "mcrouter/Proxy.h"
#include "mcrouter/ProxyConfig.h"
#include "mcrouter/ProxyConfigBuilder.h"
#include "mcrouter/RuntimeVarsData.h"
#include "mcrouter/ServiceInfo.h"
#include "mcrouter/TargetHooks.h"
#include "mcrouter/ThreadUtil.h"
#include "mcrouter/lib/AuxiliaryCPUThreadPool.h"
#include "mcrouter/routes/McRouteHandleProvider.h"
#include "mcrouter/stats.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

bool isValidRouterName(folly::StringPiece name);

} // namespace detail

template <class RouterInfo>
/* static  */ CarbonRouterInstance<RouterInfo>*
CarbonRouterInstance<RouterInfo>::init(
    folly::StringPiece persistenceId,
    const McrouterOptions& options) {
  if (auto manager = detail::McrouterManager::getSingletonInstance()) {
    return manager->mcrouterGetCreate<RouterInfo>(persistenceId, options);
  }

  return nullptr;
}

template <class RouterInfo>
/* static  */ CarbonRouterInstance<RouterInfo>*
CarbonRouterInstance<RouterInfo>::init(
    folly::StringPiece persistenceId,
    const McrouterOptions& options,
    std::shared_ptr<folly::IOThreadPoolExecutor> ioThreadPool) {
  if (auto manager = detail::McrouterManager::getSingletonInstance()) {
    return manager->mcrouterGetCreate<RouterInfo>(
        persistenceId, options, ioThreadPool);
  }

  return nullptr;
}

template <class RouterInfo>
CarbonRouterInstance<RouterInfo>* CarbonRouterInstance<RouterInfo>::get(
    folly::StringPiece persistenceId) {
  if (auto manager = detail::McrouterManager::getSingletonInstance()) {
    return manager->mcrouterGet<RouterInfo>(persistenceId);
  }

  return nullptr;
}

template <class RouterInfo>
bool CarbonRouterInstance<RouterInfo>::hasInstance(
    folly::StringPiece persistenceId) {
  return get(persistenceId) != nullptr;
}

template <class RouterInfo>
CarbonRouterInstance<RouterInfo>* CarbonRouterInstance<RouterInfo>::createRaw(
    McrouterOptions input_options,
    std::shared_ptr<folly::IOThreadPoolExecutor> ioThreadPool) {
  extraValidateOptions(input_options);
  folly::Executor::KeepAlive<> auxThreadPool;
  if (auto threadPool = AuxiliaryCPUThreadPoolSingleton::try_get()) {
    auxThreadPool = &threadPool->getThreadPool();
  } else {
    throw std::runtime_error(
        "Not creating CarbonRouterInstance. Process is being shutdown");
  }

  if (!detail::isValidRouterName(input_options.service_name) ||
      !detail::isValidRouterName(input_options.router_name)) {
    throw std::runtime_error(
        "Invalid service_name or router_name provided; must be"
        " strings matching [a-zA-Z0-9_-]+");
  }

  if (input_options.test_mode) {
    // test-mode disables all logging.
    LOG(WARNING) << "Running mcrouter in test mode. This mode should not be "
                    "used in production.";
    applyTestMode(input_options);
  }

  if (!input_options.async_spool.empty()) {
    auto rc = ::access(input_options.async_spool.c_str(), W_OK);
    PLOG_IF(WARNING, rc) << "Error while checking spooldir ("
                         << input_options.async_spool << ")";
  }

  if (input_options.enable_failure_logging) {
    initFailureLogger();
  }

  auto router = new CarbonRouterInstance<RouterInfo>(std::move(input_options));

  folly::Expected<folly::Unit, std::string> result;
  try {
    folly::json::serialization_opts jsonOpts;
    jsonOpts.sort_keys = true;
    auto dict = folly::toDynamic(router->getStartupOpts());
    auto jsonStr = folly::json::serialize(dict, jsonOpts);
    failure::setServiceContext(routerName(router->opts()), std::move(jsonStr));

    router->setIOThreadPool(std::move(ioThreadPool));

    result = router->spinUp();
    if (result.hasValue()) {
      return router;
    }
  } catch (...) {
    result = folly::makeUnexpected(
        folly::exceptionStr(std::current_exception()).toStdString());
  }

  result.error() = folly::sformat(
      "mcrouter error (router name '{}', flavor '{}',"
      " service '{}'): {}",
      router->opts().router_name,
      router->opts().flavor_name,
      router->opts().service_name,
      result.error());

  // If router cannot be spun up, reset the references to
  // the SR client factories so that SR-related singletons
  // can be released.
  // We schedule the deletion on auxiliary thread pool
  // to avoid potential deadlock with the current thread.
  auxThreadPool->add([router, auxThreadPool]() mutable {
    router->resetMetadata();
    router->resetAxonProxyClientFactory();
  });

  throw std::runtime_error(std::move(result.error()));
}

template <class RouterInfo>
std::shared_ptr<CarbonRouterInstance<RouterInfo>>
CarbonRouterInstance<RouterInfo>::create(
    McrouterOptions input_options,
    std::shared_ptr<folly::IOThreadPoolExecutor> ioThreadPool) {
  return folly::fibers::runInMainContext([&]() mutable {
    return std::shared_ptr<CarbonRouterInstance<RouterInfo>>(
        createRaw(std::move(input_options), std::move(ioThreadPool)),
        /* Custom deleter since ~CarbonRouterInstance() is private */
        [](CarbonRouterInstance<RouterInfo>* inst) { delete inst; });
  });
}

template <class RouterInfo>
typename CarbonRouterClient<RouterInfo>::Pointer
CarbonRouterInstance<RouterInfo>::createClient(
    size_t max_outstanding,
    bool max_outstanding_error) {
  return CarbonRouterClient<RouterInfo>::create(
      this->shared_from_this(),
      max_outstanding,
      max_outstanding_error,
      opts().thread_affinity
          ? CarbonRouterClient<RouterInfo>::ThreadMode::AffinitizedRemoteThread
          : CarbonRouterClient<RouterInfo>::ThreadMode::FixedRemoteThread);
}

template <class RouterInfo>
typename CarbonRouterClient<RouterInfo>::Pointer
CarbonRouterInstance<RouterInfo>::createSameThreadClient(
    size_t max_outstanding) {
  return CarbonRouterClient<RouterInfo>::create(
      this->shared_from_this(),
      max_outstanding,
      /* maxOutstandingError= */ true,
      CarbonRouterClient<RouterInfo>::ThreadMode::SameThread);
}

template <class RouterInfo>
folly::Expected<folly::Unit, std::string>
CarbonRouterInstance<RouterInfo>::setupProxy(
    const std::vector<folly::EventBase*>& evbs) {
  VLOG(2) << "Proxy setup";
  for (size_t i = 0; i < opts_.num_proxies; i++) {
    CHECK(evbs[i] != nullptr);
    proxyEvbs_.push_back(std::make_unique<folly::VirtualEventBase>(*evbs[i]));

    try {
      proxies_.emplace_back(
          Proxy<RouterInfo>::createProxy(*this, *proxyEvbs_[i], i));
    } catch (...) {
      return folly::makeUnexpected(folly::sformat(
          "Failed to create proxy: {}",
          folly::exceptionStr(std::current_exception())));
    }
  }
  return folly::Unit();
}

template <class RouterInfo>
folly::Expected<folly::Unit, std::string>
CarbonRouterInstance<RouterInfo>::spinUp() {
  if (opts_.force_same_thread && opts_.thread_affinity) {
    return folly::makeUnexpected(std::string(
        "force_same_thread and thread_affinity may not both be true"));
  }
  // Must init compression before creating proxies.
  if (opts_.enable_compression) {
    initCompression(*this);
  }

  bool configuringFromDisk = false;
  {
    std::lock_guard<std::mutex> lg(configReconfigLock_);
    auto builder = createConfigBuilder();
    if (builder.hasError()) {
      std::string initialError = std::move(builder.error());
      // If we cannot create ConfigBuilder from normal config,
      // try creating it from backup files.
      configApi_->enableReadingFromBackupFiles();
      configuringFromDisk = true;
      builder = createConfigBuilder();
      if (builder.hasError()) {
        return folly::makeUnexpected(folly::sformat(
            "Failed to configure, initial error '{}', from backup '{}'",
            initialError,
            builder.error()));
      }
    }

    std::string threadPrefix;
    // Create an IOThreadPooLExecutor if there are no evbs configured
    if (!proxyThreads_) {
      // Create IOThreadPoolExecutor
      try {
        threadPrefix = folly::to<std::string>("mcrpxy-", opts_.router_name);
        proxyThreads_ = std::make_shared<folly::IOThreadPoolExecutor>(
            opts_.num_proxies /* max */,
            opts_.num_proxies /* min */,
            std::make_shared<folly::NamedThreadFactory>(threadPrefix));
        embeddedMode_ = true;

      } catch (...) {
        return folly::makeUnexpected(folly::sformat(
            "Failed to create IOThreadPoolExecutor {}",
            folly::exceptionStr(std::current_exception())));
      }
    }

    auto executorObserver = std::make_shared<ExecutorObserver>();
    proxyThreads_->addObserver(executorObserver);
    std::vector<folly::EventBase*> threadPoolEvbs =
        executorObserver->extractEvbs();
    if (threadPoolEvbs.size() != opts_.num_proxies) {
      return folly::makeUnexpected(folly::sformat(
          "IOThreadPoolExecutor size does not match num_proxies sz={} proxies={} {}",
          threadPoolEvbs.size(),
          opts_.num_proxies,
          folly::exceptionStr(std::current_exception())));
    }
    proxyThreads_->removeObserver(executorObserver);

    if (opts_.enable_service_router && mcrouter::gSRInitHook) {
      try {
        setMetadata(mcrouter::gSRInitHook(proxyThreads_, threadPrefix, opts_));
      } catch (...) {
        return folly::makeUnexpected(folly::sformat(
            "Failed to create SR {}",
            folly::exceptionStr(std::current_exception())));
      }
    }

    auto proxyResult = setupProxy(threadPoolEvbs);
    if (proxyResult.hasError()) {
      return folly::makeUnexpected(std::string("Failed to create proxy"));
    }

    if (opts_.enable_axonlog && mcrouter::gAxonInitHook) {
      try {
        mcrouter::gAxonInitHook(*this, proxyThreads_, threadPrefix);
      } catch (...) {
        return folly::makeUnexpected(folly::sformat(
            "Failed to create SR for Axon proxy {}",
            folly::exceptionStr(std::current_exception())));
      }
    }

    auto configResult = configure(builder.value());
    if (configResult.hasValue()) {
      configApi_->subscribeToTrackedSources();
    } else {
      configFailures_.store(
          configFailures_.load(std::memory_order_relaxed) + 1,
          std::memory_order_relaxed);
      configApi_->abandonTrackedSources();

      // If we successfully created ConfigBuilder from normal config, but
      // failed to configure, we have to create ConfigBuilder again,
      // this time reading from backup files.
      configApi_->enableReadingFromBackupFiles();
      configuringFromDisk = true;
      builder = createConfigBuilder();
      auto reconfigResult = configure(builder.value());
      if (reconfigResult.hasValue()) {
        configApi_->subscribeToTrackedSources();
      } else {
        configApi_->abandonTrackedSources();
        LOG(ERROR) << "Failed to configure proxies";
        return folly::makeUnexpected(folly::sformat(
            "Failed to configure, initial error '{}', from backup '{}'",
            configResult.error(),
            reconfigResult.error()));
      }
    }
  }

  configuredFromDisk_.store(configuringFromDisk, std::memory_order_relaxed);

  startTime_.store(time(nullptr), std::memory_order_relaxed);

  spawnAuxiliaryThreads();

  return folly::Unit();
}

template <class RouterInfo>
Proxy<RouterInfo>* CarbonRouterInstance<RouterInfo>::getProxy(
    size_t index) const {
  return index < proxies_.size() ? proxies_[index] : nullptr;
}

template <class RouterInfo>
const std::vector<Proxy<RouterInfo>*>&
CarbonRouterInstance<RouterInfo>::getProxies() const {
  return proxies_;
}

template <class RouterInfo>
ProxyBase* CarbonRouterInstance<RouterInfo>::getProxyBase(size_t index) const {
  return getProxy(index);
}

template <class RouterInfo>
CarbonRouterInstance<RouterInfo>::CarbonRouterInstance(
    McrouterOptions inputOptions)
    : CarbonRouterInstanceBase(std::move(inputOptions)) {}

template <class RouterInfo>
void CarbonRouterInstance<RouterInfo>::shutdownImpl() noexcept {
  joinAuxiliaryThreads();
  proxyEvbs_.clear();
  resetMetadata();
  resetAxonProxyClientFactory();
  if (proxyThreads_ && embeddedMode_) {
    proxyThreads_->join();
  }
  proxyThreads_.reset();
}

template <class RouterInfo>
void CarbonRouterInstance<RouterInfo>::shutdown() noexcept {
  CHECK(!shutdownStarted_.exchange(true));
  shutdownImpl();
}

template <class RouterInfo>
CarbonRouterInstance<RouterInfo>::~CarbonRouterInstance() {
  if (!shutdownStarted_.exchange(true)) {
    shutdownImpl();
  }
}

template <class RouterInfo>
void CarbonRouterInstance<RouterInfo>::subscribeToConfigUpdate() {
  configUpdateHandle_ = configApi_->subscribe([this]() {
    bool success = false;
    {
      std::lock_guard<std::mutex> lg(configReconfigLock_);

      if (opts_.enable_partial_reconfigure) {
        try {
          if (reconfigurePartially()) {
            configuredFromDisk_.store(false, std::memory_order_relaxed);
            return;
          }
        } catch (const std::exception& e) {
          MC_LOG_FAILURE(
              opts(),
              failure::Category::kInvalidConfig,
              "Error on partial reconfiguring: {}",
              e.what());
        }
      }

      auto builder = createConfigBuilder();
      if (builder) {
        success = reconfigure(builder.value());
      }
    }
    if (success) {
      configuredFromDisk_.store(false, std::memory_order_relaxed);
      onReconfigureSuccess_.notify();
    } else {
      LOG(ERROR) << "Error while reconfiguring mcrouter after config change";
    }
  });
}

template <class RouterInfo>
void CarbonRouterInstance<RouterInfo>::spawnAuxiliaryThreads() {
  configApi_->startObserving();
  subscribeToConfigUpdate();

  startObservingRuntimeVarsFile();
  registerOnUpdateCallbackForRxmits();
  registerForStatsUpdates();
  spawnStatLoggerThread();
}

template <class RouterInfo>
void CarbonRouterInstance<RouterInfo>::startObservingRuntimeVarsFile() {
  if (opts_.runtime_vars_file.empty()) {
    return;
  }

  auto onUpdate = [rtVarsDataWeak = rtVarsDataWeak()](std::string data) {
    if (auto rtVarsDataPtr = rtVarsDataWeak.lock()) {
      rtVarsDataPtr->set(
          std::make_shared<const RuntimeVarsData>(std::move(data)));
    }
  };

  rtVarsDataObserver_ =
      startObservingRuntimeVarsFileCustom(opts_.runtime_vars_file, onUpdate);

  if (rtVarsDataObserver_) {
    return;
  }

  boost::system::error_code ec;
  if (!boost::filesystem::exists(opts_.runtime_vars_file, ec)) {
    return;
  }

  if (auto scheduler = functionScheduler()) {
    runtimeVarsObserverHandle_ = startObservingFile(
        opts_.runtime_vars_file,
        scheduler,
        std::chrono::milliseconds(opts_.file_observer_poll_period_ms),
        std::chrono::milliseconds(opts_.file_observer_sleep_before_update_ms),
        std::move(onUpdate));
  } else {
    MC_LOG_FAILURE(
        opts(),
        failure::Category::kSystemError,
        "Global function scheduler not available");
  }
}

template <class RouterInfo>
void CarbonRouterInstance<RouterInfo>::spawnStatLoggerThread() {
  mcrouterLogger_ = createMcrouterLogger(*this);
  mcrouterLogger_->start();
}

template <class RouterInfo>
void CarbonRouterInstance<RouterInfo>::joinAuxiliaryThreads() noexcept {
  // unsubscribe from config update
  configUpdateHandle_.reset();
  if (configApi_) {
    configApi_->stopObserving(pid_);
  }

  deregisterForStatsUpdates();

  if (mcrouterLogger_) {
    mcrouterLogger_->stop();
  }

  runtimeVarsObserverHandle_.reset();
}

template <class RouterInfo>
bool CarbonRouterInstance<RouterInfo>::reconfigure(
    const ProxyConfigBuilder& builder) {
  auto result = configure(builder);

  if (result.hasError()) {
    configFailures_.store(
        configFailures_.load(std::memory_order_relaxed) + 1,
        std::memory_order_relaxed);
    configApi_->abandonTrackedSources();
  } else {
    configApi_->subscribeToTrackedSources();
  }

  return result.hasValue();
}

template <class RouterInfo>
bool CarbonRouterInstance<RouterInfo>::reconfigurePartially() {
  auto partialUpdates = configApi_->releasePartialUpdatesLocked();
  VLOG(2) << "receive partial updates:" << partialUpdates.size();
  if (partialUpdates.empty()) {
    return false;
  }
  partialReconfigAttempt_.store(
      partialReconfigAttempt_.load(std::memory_order_relaxed) +
          partialUpdates.size(),
      std::memory_order_relaxed);

  // Use the first proxy's config, as it's same in all proxies.
  // Also there is no contention for holding the read lock as write lock is
  // only obtained by proxy_config_swap() from config thread (same thread
  // invoking this function)
  {
    auto proxyConfig = getProxy(0)->getConfigLocked();
    // Make sure partial config is allow for all updates
    for (const auto& update : partialUpdates) {
      if (!proxyConfig.second.allowPartialConfig(update.tierName)) {
        VLOG(1) << folly::sformat(
            "tier {} not allow partial reconfigure", update.tierName);
        return false;
      }
    }
  }

  auto& partialConfigs = getProxy(0)->getConfigUnsafe()->getPartialConfigs();
  for (size_t i = 0; i < opts_.num_proxies; i++) {
    for (const auto& update : partialUpdates) {
      auto& tierPartialConfigs = partialConfigs.at(update.tierName).second;
      if (i == 0) {
        VLOG(1) << folly::sformat(
            "partial update: tier={}, oldApString={}, newApString={}, oldFailureDomain={}, newFailureDomain={}",
            update.tierName,
            update.oldApString,
            update.newApString,
            update.oldFailureDomain,
            update.newFailureDomain);
      }
      for (const auto& [apAttr, poolList] : tierPartialConfigs) {
        auto oldAp = createAccessPoint(
            update.oldApString, update.oldFailureDomain, *this, *apAttr);
        auto newAp =
            std::const_pointer_cast<const AccessPoint>(createAccessPoint(
                update.newApString, update.newFailureDomain, *this, *apAttr));
        if (FOLLY_UNLIKELY(oldAp->getProtocol() != newAp->getProtocol())) {
          return false;
        }
        auto replacedAp = getProxy(i)->replaceAP(*oldAp, newAp);
        if (!replacedAp) {
          VLOG(2) << folly::sformat(
              "could not replace AP for tier={}, proxy={}, protocol={}",
              update.tierName,
              i,
              mc_protocol_to_string(oldAp->getProtocol()));
          return false;
        }
        auto proxyConfig = getProxy(i)->getConfigLocked();
        for (const auto& pool : poolList) {
          if (!proxyConfig.second.updateAccessPoints(pool, replacedAp, newAp)) {
            VLOG(2) << folly::sformat(
                "could not update AccessPoints for tier={}, pool={}, proxy={}, protocol={}",
                update.tierName,
                pool,
                i,
                mc_protocol_to_string(oldAp->getProtocol()));
            return false;
          }
        }
      }
    }
  }
  int numUpdates = partialUpdates.size();
  if (!configApi_->updatePartialConfigSource(std::move(partialUpdates))) {
    return false;
  }

  VLOG_IF(1, !opts_.constantly_reload_configs)
      << "Partially reconfigured " << opts_.num_proxies << " proxies";
  partialReconfigSuccess_.store(
      partialReconfigSuccess_.load(std::memory_order_relaxed) + numUpdates,
      std::memory_order_relaxed);
  return true;
}

template <class RouterInfo>
folly::Expected<folly::Unit, std::string>
CarbonRouterInstance<RouterInfo>::configure(const ProxyConfigBuilder& builder) {
  VLOG_IF(1, !opts_.constantly_reload_configs) << "started reconfiguring";
  std::vector<std::shared_ptr<ProxyConfig<RouterInfo>>> newConfigs;
  newConfigs.reserve(opts_.num_proxies);
  try {
    for (size_t i = 0; i < opts_.num_proxies; i++) {
      newConfigs.push_back(builder.buildConfig<RouterInfo>(*getProxy(i), i));
    }
  } catch (const std::exception& e) {
    std::string error = folly::sformat("Failed to reconfigure: {}", e.what());
    MC_LOG_FAILURE(opts(), failure::Category::kInvalidConfig, error);

    return folly::makeUnexpected(std::move(error));
  }

  for (size_t i = 0; i < opts_.num_proxies; i++) {
    proxy_config_swap(getProxy(i), newConfigs[i]);
  }

  VLOG_IF(1, !opts_.constantly_reload_configs)
      << "reconfigured " << opts_.num_proxies << " proxies with "
      << newConfigs[0]->getPools().size() << " pools, "
      << newConfigs[0]->calcNumClients() << " clients "
      << newConfigs[0]->getConfigMd5Digest() << ")";

  return folly::Unit();
}

template <class RouterInfo>
folly::Expected<ProxyConfigBuilder, std::string>
CarbonRouterInstance<RouterInfo>::createConfigBuilder() {
  VLOG_IF(1, !opts_.constantly_reload_configs) << "creating config builder";
  /* mark config attempt before, so that
     successful config is always >= last config attempt. */
  lastConfigAttempt_.store(time(nullptr), std::memory_order_relaxed);
  configFullAttempt_.store(
      configFullAttempt_.load(std::memory_order_relaxed) + 1,
      std::memory_order_relaxed);
  configApi_->trackConfigSources();
  std::string config;
  std::string path;
  std::string error;
  if (configApi_->getConfigFile(config, path)) {
    try {
      // assume default_route, default_region and default_cluster are same for
      // each proxy
      return ProxyConfigBuilder(opts_, configApi(), config, RouterInfo::name);
    } catch (const std::exception& e) {
      MC_LOG_FAILURE(
          opts(),
          failure::Category::kInvalidConfig,
          "Failed to reconfigure: {}",
          e.what());
      error = e.what();
    }
  }
  MC_LOG_FAILURE(
      opts(),
      failure::Category::kBadEnvironment,
      "Can not read config from {}",
      path);
  configFailures_.store(
      configFailures_.load(std::memory_order_relaxed) + 1,
      std::memory_order_relaxed);
  configApi_->abandonTrackedSources();
  return folly::makeUnexpected(std::move(error));
}

template <class RouterInfo>
void CarbonRouterInstance<RouterInfo>::registerOnUpdateCallbackForRxmits() {
  rxmitHandle_ = rtVarsData().subscribeAndCall(
      [this](
          std::shared_ptr<const RuntimeVarsData> /* oldVars */,
          std::shared_ptr<const RuntimeVarsData> newVars) {
        if (!newVars) {
          return;
        }
        const auto val =
            newVars->getVariableByName("disable_rxmit_reconnection");
        if (val != nullptr) {
          checkLogic(
              val.isBool(),
              "runtime vars 'disable_rxmit_reconnection' is not a boolean");
          disableRxmitReconnection_ = val.asBool();
        }
      });
}

template <class RouterInfo>
/* static */ void CarbonRouterInstance<RouterInfo>::freeAllMcrouters() {
  freeAllRouters();
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
