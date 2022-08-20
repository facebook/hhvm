/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CpuController.h"

#include <cassert>

#include <folly/File.h>
#include <folly/FileUtil.h>

namespace facebook {
namespace memcache {

namespace {

bool readProcStat(std::vector<uint64_t>& curArray) {
  auto cpuStatFile = folly::File("/proc/stat");
  // Enough storage for the /proc/stat CPU data needed below
  std::array<char, 320> buf;
  if (folly::readNoInt(cpuStatFile.fd(), buf.data(), buf.size()) !=
      static_cast<ssize_t>(buf.size())) {
    return false;
  }

  if (sscanf(
          buf.data(),
          "cpu %lu %lu %lu %lu %lu %lu %lu %lu",
          &curArray[0],
          &curArray[1],
          &curArray[2],
          &curArray[3],
          &curArray[4],
          &curArray[5],
          &curArray[6],
          &curArray[7]) != static_cast<int>(curArray.size())) {
    return false;
  }

  return true;
}

} // namespace

CpuController::CpuController(
    const CpuControllerOptions& opts,
    folly::EventBase& evb)
    : evb_(evb), dataCollectionInterval_(opts.dataCollectionInterval) {
  assert(opts.shouldEnable());
}

void CpuController::start() {
  stopController_ = false;
  auto self = shared_from_this();
  evb_.runInEventBaseThread([this, self]() { cpuLoggingFn(); });
}

void CpuController::stop() {
  stopController_ = true;
}

// Compute the cpu utilization
void CpuController::cpuLoggingFn() {
  double cpuUtil = 0.0;

  // Corner case: When parsing /proc/stat fails, set the cpuUtil to 0.
  std::vector<uint64_t> cur(8);
  if (readProcStat(cur)) {
    if (firstLoop_) {
      prev_ = std::move(cur);
      firstLoop_ = false;
    } else {
      /**
       * The values in the /proc/stat is the CPU time since boot.
       * Columns [0, 1, ... 9] map to [user, nice, system, idle, iowait, irq,
       * softirq, steal, guest, guest_nice]. Guest related fields are not used
       * for the cpu util calculation. The total CPU time in the last
       * window is delta busy time over delta total time.
       */
      auto curUtil =
          cur[0] + cur[1] + cur[2] + cur[4] + cur[5] + cur[6] + cur[7];
      auto prevUtil = prev_[0] + prev_[1] + prev_[2] + prev_[4] + prev_[5] +
          prev_[6] + prev_[7];
      auto utilDiff = static_cast<double>(curUtil - prevUtil);
      auto totalDiff = utilDiff + cur[3] - prev_[3];

      /**
       * Corner case: If CPU didn't change or the proc/stat didn't get
       * updated or ticks didn't increase, set the cpuUtil to 0.
       */
      if (totalDiff < 0.001 || curUtil < prevUtil) {
        cpuUtil = 0.0;
      } else {
        // Corner case: The max of CPU utilization can be at most 100%.
        cpuUtil = std::min((utilDiff / totalDiff) * 100, 100.0);
      }
      prev_ = std::move(cur);
    }
  }
  if (stopController_) {
    update(0.0);
    return;
  }
  update(cpuUtil);
  auto self = shared_from_this();
  evb_.runAfterDelay(
      [this, self]() { cpuLoggingFn(); }, dataCollectionInterval_.count());
}

void CpuController::update(double cpuUtil) {
  percentLoad_.store(cpuUtil);
}

} // namespace memcache
} // namespace facebook
