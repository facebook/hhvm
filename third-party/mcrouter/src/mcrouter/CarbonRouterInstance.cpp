/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CarbonRouterInstance.h"

#include <fmt/format.h>
#include <folly/executors/thread_factory/InitThreadFactory.h>
#include <folly/experimental/io/MuxIOThreadPoolExecutor.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

bool isValidRouterName(folly::StringPiece name) {
  if (name.empty()) {
    return false;
  }

  for (auto c : name) {
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || (c == '_') || (c == '-'))) {
      return false;
    }
  }

  return true;
}

std::unique_ptr<folly::IOThreadPoolExecutorBase> createProxyThreadsExecutor(
    const McrouterOptions& opts) {
  const size_t numProxies = opts.num_proxies;
  auto threadPrefix = folly::to<std::string>("mcrpxy-", opts.router_name);
  std::shared_ptr<folly::ThreadFactory> threadFactory =
      std::make_shared<folly::NamedThreadFactory>(threadPrefix);
  threadFactory = std::make_shared<folly::InitThreadFactory>(
      std::move(threadFactory), [] { ProxyBase::registerProxyThread(); });

  if (opts.use_mux_io_thread_pool) {
    return std::make_unique<folly::MuxIOThreadPoolExecutor>(
        numProxies,
        folly::MuxIOThreadPoolExecutor::Options{}
            .setWakeUpInterval(
                std::chrono::microseconds{
                    opts.mux_io_thread_pool_wake_up_interval_us})
            .setIdleSpinMax(
                std::chrono::microseconds{
                    opts.mux_io_thread_pool_idle_spin_max_us}),
        std::move(threadFactory));
  } else {
    return std::make_unique<folly::IOThreadPoolExecutor>(
        numProxies /* max */, numProxies /* min */, std::move(threadFactory));
  }
}

} // namespace detail

void freeAllRouters() {
  if (auto manager = detail::McrouterManager::getSingletonInstance()) {
    manager->freeAllMcrouters();
  }
}

CpuStatsWorker::CpuStatsWorker(
    std::chrono::seconds timeInterval,
    std::shared_ptr<folly::FunctionScheduler> scheduler,
    const folly::IOThreadPoolExecutorBase& proxyThreads)
    : timeInterval_(timeInterval),
      scheduler_(scheduler),
      startMs_(std::chrono::steady_clock::time_point::min()),
      proxyThreads_(proxyThreads) {
  static std::atomic<int> uniqueId(0);
  name_ = fmt::format("{}{}", kCpuStatsWorkerName_, uniqueId++);
}

void CpuStatsWorker::schedule(bool enable) {
  std::lock_guard<std::mutex> lock(mx_);
  if (auto scheduler = scheduler_.lock()) {
    if (enable) {
      if (timeInterval_.count() > 0 && !scheduled_.exchange(true)) {
        scheduler->addFunction(
            [this]() { this->calculateCpuStats(); },
            timeInterval_.count() > kMinSchedulingInterval
                ? timeInterval_
                : std::chrono::seconds(
                      kMinSchedulingInterval), /* monitoring interval in ms */
            name_,
            std::chrono::milliseconds{
                kWorkerStartDelayMs_} /* start delay in ms */);
      }
    } else {
      if (scheduled_.exchange(false)) {
        scheduler->cancelFunctionAndWait(name_);
        avgCpu_ = 0;
        firstRun_ = true;
      }
    }
  }
}

CpuStatsWorker::~CpuStatsWorker() {
  schedule(false);
}

void CpuStatsWorker::calculateCpuStats() {
  auto end = std::chrono::steady_clock::now();
  auto currUsedCpuTime = proxyThreads_.getUsedCpuTime();
  if (!firstRun_.exchange(false)) {
    auto timeDeltaNs =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - startMs_)
            .count();
    auto cpuDeltaNs = (currUsedCpuTime - usedCpuTime_).count();
    avgCpu_ = 100 * cpuDeltaNs / (timeDeltaNs * proxyThreads_.numThreads());
  }
  // Store values for next iteration
  usedCpuTime_ = currUsedCpuTime;
  startMs_ = end;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
