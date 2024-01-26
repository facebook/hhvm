/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CarbonRouterInstance.h"

#include <folly/io/async/EventBase.h>
#include <folly/json.h>

#include "mcrouter/lib/AuxiliaryCPUThreadPool.h"
#include "mcrouter/lib/fbi/cpp/LogFailure.h"

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

} // namespace detail

void freeAllRouters() {
  if (auto manager = detail::McrouterManager::getSingletonInstance()) {
    manager->freeAllMcrouters();
  }
}

CpuStatsWorker::CpuStatsWorker(
    std::chrono::milliseconds timeIntervalMs,
    std::shared_ptr<folly::FunctionScheduler> scheduler,
    const folly::IOThreadPoolExecutorBase& proxyThreads)
    : scheduler_(scheduler),
      startMs_(std::chrono::steady_clock::time_point::min()),
      proxyThreads_(proxyThreads) {
  if (timeIntervalMs.count() > 0 && scheduler) {
    scheduler->addFunction(
        [this]() { this->calculateCpuStats(); },
        timeIntervalMs, /* monitoring interval in ms */
        kCpuStatsWorkerName_,
        std::chrono::milliseconds{
            kWorkerStartDelayMs_} /* start delay in ms */);
  }
}

CpuStatsWorker::~CpuStatsWorker() {
  if (auto scheduler = scheduler_.lock()) {
    scheduler->cancelFunctionAndWait(kCpuStatsWorkerName_);
  }
}

void CpuStatsWorker::calculateCpuStats() {
  auto end = std::chrono::steady_clock::now();
  auto currUsedCpuTime = proxyThreads_.getUsedCpuTime();
  if (usedCpuTime_.count() > 0 &&
      startMs_ > std::chrono::steady_clock::time_point::min()) {
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
