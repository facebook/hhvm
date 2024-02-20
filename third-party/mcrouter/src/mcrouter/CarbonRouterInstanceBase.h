/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <memory>
#include <unordered_map>

#include <folly/Synchronized.h>
#include <folly/container/EvictingCacheMap.h>
#include <folly/experimental/FunctionScheduler.h>
#include <folly/experimental/ReadMostlySharedPtr.h>
#include <folly/fibers/TimedMutex.h>
#include <folly/io/async/EventBaseThread.h>
#include <folly/synchronization/CallOnce.h>

#include "mcrouter/ConfigApi.h"
#include "mcrouter/ExternalStatsHandler.h"
#include "mcrouter/LeaseTokenMap.h"
#include "mcrouter/Observable.h"
#include "mcrouter/PoolStats.h"
#include "mcrouter/TkoTracker.h"
#include "mcrouter/lib/network/Transport.h"
#include "mcrouter/options.h"

namespace facebook {
namespace memcache {

// Forward declarations
struct CodecConfig;
using CodecConfigPtr = std::unique_ptr<CodecConfig>;
class CompressionCodecManager;

namespace mcrouter {

// Forward declarations
class AsyncWriter;
template <class RouterInfo>
class Proxy;
class RuntimeVarsData;
using ObservableRuntimeVars =
    Observable<std::shared_ptr<const RuntimeVarsData>>;

using ShadowLeaseTokenMap = folly::Synchronized<
    folly::EvictingCacheMap<int64_t, int64_t>,
    folly::fibers::TimedMutex>;

using LogPostprocessCallbackFunc = std::function<void(
    const folly::dynamic&, // Serialized request
    const folly::dynamic&, // Serialized reply
    const char* const, // Name of operation (e.g. 'get')
    const folly::StringPiece)>; // User ip

class CarbonRouterInstanceBase {
 public:
  using SvcIdentAuthCallbackFunc = Transport::SvcIdentAuthCallbackFunc;

  explicit CarbonRouterInstanceBase(McrouterOptions inputOptions);
  virtual ~CarbonRouterInstanceBase() = default;

  pid_t pid() const {
    return pid_;
  }

  const McrouterOptions& opts() const {
    return opts_;
  }

  /**
   * Returns compression codec manager.
   * If compression is disabled, this method will return nullptr.
   */
  const CompressionCodecManager* getCodecManager() const {
    return compressionCodecManager_.get();
  }

  void setUpCompressionDictionaries(
      std::unordered_map<uint32_t, CodecConfigPtr>&& codecConfigs) noexcept;

  TkoTrackerMap& tkoTrackerMap() {
    return tkoTrackerMap_;
  }

  ExternalStatsHandler& externalStatsHandler() {
    return externalStatsHandler_;
  }

  ConfigApi& configApi() {
    assert(configApi_.get() != nullptr);
    return *configApi_;
  }

  ObservableRuntimeVars& rtVarsData() {
    return *rtVarsData_;
  }

  std::weak_ptr<ObservableRuntimeVars> rtVarsDataWeak() {
    return rtVarsData_;
  }

  /**
   * Returns an AsyncWriter for stats related purposes.
   */
  folly::ReadMostlySharedPtr<AsyncWriter> statsLogWriter();

  LeaseTokenMap& leaseTokenMap() {
    return leaseTokenMap_;
  }

  ShadowLeaseTokenMap& shadowLeaseTokenMap() {
    using UnsynchronizedMap = typename ShadowLeaseTokenMap::DataType;

    folly::call_once(shadowLeaseTokenMapInitFlag_, [this]() {
      shadowLeaseTokenMap_ = std::make_unique<ShadowLeaseTokenMap>(
          UnsynchronizedMap{opts().max_shadow_token_map_size});
    });
    return *shadowLeaseTokenMap_;
  }

  const LogPostprocessCallbackFunc& postprocessCallback() const {
    return postprocessCallback_;
  }

  void setPostprocessCallback(LogPostprocessCallbackFunc&& newCallback) {
    postprocessCallback_ = std::move(newCallback);
  }

  const SvcIdentAuthCallbackFunc& svcIdentAuthCallbackFunc() const {
    return svcIdentAuthCallback_;
  }

  void setSvcIdentAuthCallbackFunc(SvcIdentAuthCallbackFunc&& newCallback) {
    svcIdentAuthCallback_ = std::move(newCallback);
  }

  /**
   * Returns an AsyncWriter for mission critical work (use statsLogWriter() for
   * auxiliary / low priority work).
   */
  folly::ReadMostlySharedPtr<AsyncWriter> asyncWriter();

  std::unordered_map<std::string, std::string> getStartupOpts() const;
  void setStartupOpts(
      std::unordered_map<std::string, std::string> additionalOpts);

  uint64_t startTime() const {
    return startTime_.load(std::memory_order_relaxed);
  }

  time_t lastConfigAttempt() const {
    return lastConfigAttempt_.load(std::memory_order_relaxed);
  }

  size_t configFailures() const {
    return configFailures_.load(std::memory_order_relaxed);
  }

  bool configuredFromDisk() const {
    return configuredFromDisk_.load(std::memory_order_relaxed);
  }

  size_t partialReconfigAttempt() const {
    return partialReconfigAttempt_.load(std::memory_order_relaxed);
  }

  size_t partialReconfigSuccess() const {
    return partialReconfigSuccess_.load(std::memory_order_relaxed);
  }

  size_t configFullAttempt() const {
    return configFullAttempt_.load(std::memory_order_relaxed);
  }

  bool isRxmitReconnectionDisabled() const {
    return disableRxmitReconnection_;
  }

  /**
   * This function finds the index of poolName in the statsEnabledPools_
   * sorted array by doing binary search. If exact match is not found,
   * index with maximum prefix match is returned.
   *
   * @return index of the pool in the statsEnabledPools_ vector
   *         -1 if not found
   */
  int32_t getStatsEnabledPoolIndex(folly::StringPiece poolName) const;

  /**
   * @return  reference to the statsEnabledPools_ vector
   */
  const std::vector<std::string>& getStatsEnabledPools() const {
    return statsEnabledPools_;
  }

  /**
   * @return  nullptr if index is >= opts.num_proxies,
   *          pointer to the proxy otherwise.
   */
  virtual ProxyBase* getProxyBase(size_t index) const = 0;

  /**
   * Returns a size_t in range [0,100] measuring the average CPU
   * of the proxy threads over the specified timeIntervalMs window.
   */
  virtual size_t getProxyCpu() const = 0;

  /**
   * Bump and return the index of the next proxy to be used by clients.
   */
  size_t nextProxyIndex();

  /**
   * Returns a FunctionScheduler suitable for running periodic background tasks
   * on. Null may be returned if the global instance has been destroyed.
   */
  std::shared_ptr<folly::FunctionScheduler> functionScheduler();

  /**
   * Returns name of router e.g. Memcache"
   */
  virtual folly::StringPiece routerInfoName() const = 0;

  template <class T>
  auto getMetadata() {
    return std::static_pointer_cast<T>(metadata_);
  }

  void setMetadata(std::shared_ptr<void> metadata) {
    metadata_ = std::move(metadata);
  }

  template <class T>
  auto getAxonProxyClientFactory() {
    return std::static_pointer_cast<T>(axonProxyClientFactory_);
  }

  void setAxonProxyClientFactory(std::shared_ptr<void> clientFactory) {
    axonProxyClientFactory_ = std::move(clientFactory);
  }

  /**
   * Runtime features that can be enabled from runtime_features block
   * in routing config
   */
  struct RuntimeFeatures {
    std::atomic<bool> enableOdslScuba = false;
    std::atomic<bool> enableOdslODS = false;
  };
  RuntimeFeatures runtimeFeatures_;

 protected:
  void resetMetadata() {
    metadata_.reset();
  }

  void resetAxonProxyClientFactory() {
    axonProxyClientFactory_.reset();
  }

  /**
   * Register this instance for periodic stats updates.
   */
  void registerForStatsUpdates();

  /**
   * Deregister this instance for periodic stats updates.
   */
  void deregisterForStatsUpdates();

  const McrouterOptions opts_;
  const pid_t pid_;
  const std::unique_ptr<ConfigApi> configApi_;

  LogPostprocessCallbackFunc postprocessCallback_;
  SvcIdentAuthCallbackFunc svcIdentAuthCallback_;

  // These next four fields are used for stats
  std::atomic<uint64_t> startTime_{0};
  std::atomic<time_t> lastConfigAttempt_{0};
  std::atomic<size_t> configFailures_{0};
  std::atomic<bool> configuredFromDisk_{false};
  std::atomic<size_t> partialReconfigAttempt_{0};
  std::atomic<size_t> partialReconfigSuccess_{0};
  std::atomic<size_t> configFullAttempt_{0};

  // Stores whether we should reconnect after hitting rxmit threshold
  std::atomic<bool> disableRxmitReconnection_{false};

  folly::Optional<folly::observer::Observer<std::string>> rtVarsDataObserver_;

 private:
  size_t statsIndex() const {
    return statsIndex_;
  }

  void statsIndex(size_t newIndex) {
    statsIndex_ = newIndex;
  }

  TkoTrackerMap tkoTrackerMap_;
  ExternalStatsHandler externalStatsHandler_;
  std::unique_ptr<const CompressionCodecManager> compressionCodecManager_;

  // Stores data for runtime variables.
  const std::shared_ptr<ObservableRuntimeVars> rtVarsData_;

  // Keep track of lease tokens of failed over requests.
  LeaseTokenMap leaseTokenMap_;

  // In order to shadow lease-sets properly, we need to pass the correct token
  // to the shadow destination.
  std::unique_ptr<ShadowLeaseTokenMap> shadowLeaseTokenMap_;
  folly::once_flag shadowLeaseTokenMapInitFlag_;

  std::unordered_map<std::string, std::string> additionalStartupOpts_;
  std::atomic<bool> startupOptsInitialized_{false};

  std::mutex nextProxyMutex_;
  size_t nextProxy_{0};

  // Current stats index. Only accessed / updated  by stats background thread.
  size_t statsIndex_{0};

  // Name of the stats update function registered with the function scheduler.
  const std::string statsUpdateFunctionHandle_;

  std::vector<std::string> statsEnabledPools_;

  // Aggregates stats for all associated proxies. Should be called periodically.
  void updateStats();

  /**
   * Opaque metadata used by SRRoute, to avoid circular dependency
   */
  std::shared_ptr<void> metadata_;

  std::shared_ptr<void> axonProxyClientFactory_;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
