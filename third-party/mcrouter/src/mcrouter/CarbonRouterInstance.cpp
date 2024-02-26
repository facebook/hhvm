/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CarbonRouterInstance.h"

#include <folly/io/async/EventBase.h>
#include <folly/json/json.h>

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
    std::chrono::seconds timeInterval,
    std::shared_ptr<folly::FunctionScheduler> scheduler,
    const folly::IOThreadPoolExecutorBase& proxyThreads)
    : timeInterval_(timeInterval),
      scheduler_(scheduler),
      startMs_(std::chrono::steady_clock::time_point::min()),
      proxyThreads_(proxyThreads) {}

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
            kCpuStatsWorkerName_,
            std::chrono::milliseconds{
                kWorkerStartDelayMs_} /* start delay in ms */);
      }
    } else {
      if (scheduled_.exchange(false)) {
        scheduler->cancelFunctionAndWait(kCpuStatsWorkerName_);
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
