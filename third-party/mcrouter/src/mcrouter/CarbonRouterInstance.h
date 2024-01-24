/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <folly/Range.h>
#include <folly/executors/IOThreadPoolExecutor.h>

#include "mcrouter/CallbackPool.h"
#include "mcrouter/CarbonRouterClient.h"
#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/ConfigApi.h"
#include "mcrouter/FileObserver.h"
#include "mcrouter/Proxy.h"
#include "mcrouter/ProxyConfigBuilder.h"

namespace facebook {
namespace memcache {
namespace mcrouter {
namespace detail {
class McrouterManager;
}

class ProxyThread;

/**
 * Class to to read CPU metrics of the mcrouter proxy threads
 * based on a FunctionScheduler
 */
class CpuStatsWorker {
 public:
  CpuStatsWorker(
      std::chrono::milliseconds timeIntervalMs,
      std::shared_ptr<folly::FunctionScheduler> scheduler,
      const folly::IOThreadPoolExecutorBase& proxyThreads);

  ~CpuStatsWorker();

  /*
   * Returns a size_t in range [0,100] measuring the average CPU
   * of the proxy threads over the specified timeIntervalMs window.
   */
  size_t getAvgCpu() {
    return avgCpu_;
  }

 private:
  size_t avgCpu_{0};
  std::shared_ptr<folly::FunctionScheduler> scheduler_;
  std::chrono::steady_clock::time_point startMs_;
  std::chrono::nanoseconds usedCpuTime_{0};
  const folly::IOThreadPoolExecutorBase& proxyThreads_;
  static constexpr int kWorkerStartDelayMs_ = 1000;
  static constexpr std::string_view kCpuStatsWorkerName_ = "cpu-stats_worker";

  void calculateCpuStats();
};

/**
 * A single mcrouter instance.  A mcrouter instance has a single config,
 * but might run across multiple threads.
 */
template <class RouterInfo>
class CarbonRouterInstance
    : public CarbonRouterInstanceBase,
      public std::enable_shared_from_this<CarbonRouterInstance<RouterInfo>> {
 public:
  /* Creation methods. All mcrouter instances are managed automatically,
     so the users don't need to worry about destruction. */

  /**
   * @return  If an instance with the given persistenceId already exists,
   *   returns a pointer to it. Options are ignored in this case.
   *   Otherwise, internally create an IOThreadPoolExecutor in no-resize mode,
   *   spin up a new instance and returns the pointer to it. May
   *   return nullptr if the McRouterManager singleton is unavailable, perhaps
   *   due to misconfiguration.
   * @param persistenceId String uniquely identifying this instance
   * @param options McrouterOptions to use when creating instance
   * @throw runtime_error  If no valid instance can be constructed from
   *   the provided options.
   */
  static CarbonRouterInstance<RouterInfo>* init(
      folly::StringPiece persistenceId,
      const McrouterOptions& options);

  /**
   * @return  If an instance with the given persistenceId already exists,
   *   returns a pointer to it. Options are ignored in this case.
   *   Otherwise spins up a new instance and returns the pointer to it. May
   *   return nullptr if the McRouterManager singleton is unavailable, perhaps
   *   due to misconfiguration.
   * @param persistenceId String uniquely identifying this instance
   * @param options McrouterOptions to use when creating instance
   * @param ioThreadPool  IOThreadPoolExecutor for the proxies. This must be
   *   created in no-resize mode.
   * @throw runtime_error  If no valid instance can be constructed from
   *   the provided options.
   */
  static CarbonRouterInstance<RouterInfo>* init(
      folly::StringPiece persistenceId,
      const McrouterOptions& options,
      std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool);

  /**
   * If an instance with the given persistenceId already exists,
   * returns a pointer to it. Otherwise returns nullptr.
   */
  static CarbonRouterInstance<RouterInfo>* get(
      folly::StringPiece persistenceId);

  /**
   * If an instance with the given persistenceId already exists,
   * returns true. Otherwise returns false.
   */
  static bool hasInstance(folly::StringPiece persistenceId);

  /**
   * Intended for short-lived instances with unusual configs
   * (i.e. for debugging).
   *
   * Create a new mcrouter instance.
   * @return  Pointer to the newly brought up instance or nullptr
   *   if there was a problem starting it up.
   * @throw runtime_error  If no valid instance can be constructed from
   *   the provided options.
   */
  static std::shared_ptr<CarbonRouterInstance<RouterInfo>> create(
      McrouterOptions input_options,
      std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool = nullptr);

  /**
   * Destroys ALL active instances for ALL RouterInfos.
   */
  static void freeAllMcrouters();

  /**
   * Create a handle to talk to mcrouter.
   *
   * @param maximum_outstanding_requests  If nonzero, at most this many requests
   *   will be allowed to be in flight at any single point in time.
   *   send() will block until the number of outstanding requests
   *   is less than this limit.
   * @throw std::runtime_error  If the client cannot be created
   *   (i.e. attempting to create multiple clients to transient mcrouter).
   */
  typename CarbonRouterClient<RouterInfo>::Pointer createClient(
      size_t maximum_outstanding_requests,
      bool maximum_outstanding_requests_error = false);

  /**
   * Same as createClient(), but you must use it from the same thread that's
   * running the assigned proxy's event base.  The sends call into proxy
   * callbacks directly, bypassing the queue.
   */
  typename CarbonRouterClient<RouterInfo>::Pointer createSameThreadClient(
      size_t maximum_outstanding_requests);

  /**
   * Shutdown all threads started by this instance. It's a blocking call and
   * should be called at most once. If it is not called, destructor will block
   * until all threads are stopped.
   */
  void shutdown() noexcept;

  ProxyBase* getProxyBase(size_t index) const override final;

  size_t getProxyCpu() const override final {
    return cpuStatsWorker_->getAvgCpu();
  }

  /**
   * @return  nullptr if index is >= opts.num_proxies,
   *   pointer to the proxy otherwise.
   */
  Proxy<RouterInfo>* getProxy(size_t index) const;

  const std::vector<Proxy<RouterInfo>*>& getProxies() const;

  folly::StringPiece routerInfoName() const override {
    return RouterInfo::name;
  }

  void setIOThreadPool(
      std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool) {
    proxyThreads_ = std::move(ioThreadPool);
  }

  const folly::IOThreadPoolExecutorBase& getIOThreadPool() const {
    return *proxyThreads_;
  }

  void resetCpuStatsWorker() {
    cpuStatsWorker_.reset();
  }

  CarbonRouterInstance(const CarbonRouterInstance&) = delete;
  CarbonRouterInstance& operator=(const CarbonRouterInstance&) = delete;
  CarbonRouterInstance(CarbonRouterInstance&&) noexcept = delete;
  CarbonRouterInstance& operator=(CarbonRouterInstance&&) = delete;

 private:
  CallbackPool<> onReconfigureSuccess_;

  // Lock to get before regenerating config structure
  std::mutex configReconfigLock_;

  // Corresponding handle
  ObservableRuntimeVars::CallbackHandle rxmitHandle_;

  /**
   * Logs mcrouter stats to disk every opts->stats_logging_interval
   * milliseconds
   */
  std::unique_ptr<McrouterLogger> mcrouterLogger_;

  std::atomic<bool> shutdownStarted_{false};

  FileObserverHandle runtimeVarsObserverHandle_;

  ConfigApi::CallbackHandle configUpdateHandle_;
  ConfigApi::CallbackHandle configAdditionalUpdateHandle_;

  /**
   * Both these vectors will contain opts.num_proxies elements.
   */
  std::vector<Proxy<RouterInfo>*> proxies_;
  std::vector<std::unique_ptr<folly::VirtualEventBase>> proxyEvbs_;
  std::shared_ptr<folly::IOThreadPoolExecutorBase> proxyThreads_;

  // Worker thread to calculate avg cpu across proxy threads
  std::unique_ptr<CpuStatsWorker> cpuStatsWorker_;

  /**
   * Indicates if evbs/IOThreadPoolExecutor has been created by McRouter or
   * passed as an argument in construction.
   */
  bool embeddedMode_{false};

  /**
   * The only reason this is a separate function is due to legacy accessor
   * needs.
   */
  static CarbonRouterInstance<RouterInfo>* createRaw(
      McrouterOptions input_options,
      std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool = nullptr);

  explicit CarbonRouterInstance(McrouterOptions input_options);

  ~CarbonRouterInstance() override;

  folly::Expected<folly::Unit, std::string> spinUp();

  folly::Expected<folly::Unit, std::string> setupProxy(
      const std::vector<folly::EventBase*>& evbs);

  void spawnAuxiliaryThreads();
  void joinAuxiliaryThreads() noexcept;
  void shutdownImpl() noexcept;

  void subscribeToConfigUpdate();

  void spawnStatLoggerThread();
  void startObservingRuntimeVarsFile();

  folly::Expected<folly::Unit, std::string> configure(
      const ProxyConfigBuilder& builder);
  /** (re)configure the router. true on success, false on error.
      NB file-based configuration is synchronous
      but server-based configuration is asynchronous */
  bool reconfigure(const ProxyConfigBuilder& builder);
  bool reconfigurePartially();
  /** Create the ProxyConfigBuilder used to reconfigure.
  Returns error reason if constructor fails. **/
  folly::Expected<ProxyConfigBuilder, std::string> createConfigBuilder();

  void registerOnUpdateCallbackForRxmits();

 public:
  /* Do not use for new code */
  class LegacyPrivateAccessor {
   public:
    static CarbonRouterInstance<RouterInfo>* createRaw(
        const McrouterOptions& opts) {
      return CarbonRouterInstance<RouterInfo>::createRaw(opts.clone());
    }

    static void destroy(CarbonRouterInstance<RouterInfo>* mcrouter) {
      delete mcrouter;
    }

    static CallbackPool<>& onReconfigureSuccess(
        CarbonRouterInstance<RouterInfo>& mcrouter) {
      mcrouter.configAdditionalUpdateHandle_ =
          mcrouter.configApi_->subscribeAdditionalCallback(
              [&mcrouter]() { mcrouter.onReconfigureSuccess_.notify(); });
      return mcrouter.onReconfigureSuccess_;
    }
  };

 private:
  friend class LegacyPrivateAccessor;
  friend class CarbonRouterClient<RouterInfo>;
  friend class detail::McrouterManager;
  friend class ProxyDestinationMap;
};

/**
 * Destroy all active instances.
 */
void freeAllRouters();

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "CarbonRouterInstance-inl.h"
