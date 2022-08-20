/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <stdint.h>
#include <vector>

#include <proxygen/lib/stats/PeriodicStatsDataBase.h>

namespace proxygen {

/**
 * Container struct to store various resource utilization data.
 */
struct ResourceData : public PeriodicStatsDataBase {
 public:
  ResourceData() = default;
  virtual ~ResourceData() = default;

  /**
   * Gets the cpu ratio utilization (0, 1.0 over the last update interval)
   * Note for the purposes of this implementation, a core is considered
   * utilized should it not be idle (i.e. excludes both idle and iowait
   * proc stats).
   */
  double getCpuRatioUtil() const {
    return cpuRatioUtil_;
  }

  /**
   * Gets the cpu percentage utilization (0-100 over last update interval)
   * Note for the purposes of this implementation, a core is considered
   * utilized should it not be idle (i.e. excludes both idle and iowait
   * proc stats)
   */
  double getCpuPctUtil() const {
    return getPctFromRatio(cpuRatioUtil_);
  }

  /**
   * Gets the average soft cpu ratio utilization (0, 1.0 over the last update
   * interval).
   */
  double getSoftIrqCpuRatioUtil() const {
    return cpuSoftIrqRatioUtil_;
  }

  /**
   * Gets the soft cpu ratio utilization (0, 1.0 over the last update interval)
   * for each cpu core in the system.  Note for the purposes of this
   * implementation, a core is considered utilized should it not be idle
   * (i.e. excludes both idle and iowait proc stats).
   */
  const std::vector<double>& getSoftIrqCpuCoreRatioUtils() const {
    return softIrqCpuCoreRatioUtils_;
  }

  /**
   * Gets the number of employed cpu cores as inferred by the length of the
   * per core soft utilization data already queried.
   */
  uint64_t getNumLogicalCpuCores() const {
    return softIrqCpuCoreRatioUtils_.size();
  }

  /**
   * Returns the percentage that maps to the specified ratio in the range
   * [0.0, 1.0].
   */
  static double getPctFromRatio(double ratio) {
    return ratio * 100;
  }

  // Gets the unused memory (free/available) of the system in bytes
  uint64_t getUnusedMemBytes() const {
    return totalMemBytes_ - usedMemBytes_;
  }

  // Gets the used memory of the system in bytes
  uint64_t getUsedMemBytes() const {
    return usedMemBytes_;
  }

  // Gets the total memory of the system in bytes
  uint64_t getTotalMemBytes() const {
    return totalMemBytes_;
  }

  /**
   * Gets the unused memory (0-100 Free/available) of the system as a
   * percent
   */
  double getFreeMemPct() const {
    return 100 - getUsedMemPct();
  }

  // Gets the used memory (0-100) of the system as a percent
  double getUsedMemPct() const {
    return ((double)usedMemBytes_) / totalMemBytes_ * 100;
  }

  // Gets the used memory (0-1.0) of the system as a ratio
  double getUsedMemRatio() const {
    return ((double)usedMemBytes_) / totalMemBytes_;
  }

  // Returns current level of TCP memory consumption
  // measured in memory pages
  uint64_t getTcpMemPages() const {
    return tcpMemoryPages_;
  }

  // Returns Low TCP Memory threshold measured in memory pages
  uint64_t getLowTcpMemLimitPages() const {
    return minTcpMemLimit_;
  }

  // Returns Pressure TCP Memory threshold measured in memory pages
  uint64_t getPressureTcpMemLimitPages() const {
    return pressureTcpMemLimit_;
  }

  // Returns Low TCP Memory threshold measured in memory pages
  uint64_t getMaxTcpMemLimitPages() const {
    return maxTcpMemLimit_;
  }

  // Gets the used TCP memory (0-1.0) as a ratio to max TCP mem limit
  double getTcpMemRatio() const {
    return calculateRatio(tcpMemoryPages_, maxTcpMemLimit_);
  }

  // Gets the low TCP memory threshold (0-1.0) as a ratio to
  // max TCP mem limit
  double getLowTcpMemLimitRatio() const {
    return calculateRatio(minTcpMemLimit_, maxTcpMemLimit_);
  }

  // Gets the TCP memory pressure threshold (0-1.0) as a ratio to
  // max TCP mem limit
  double getPressureTcpMemLimitRatio() const {
    return calculateRatio(pressureTcpMemLimit_, maxTcpMemLimit_);
  }

  // Gets the used TCP memory (0-100) of the system as a percent
  double getUsedTcpMemPct() const {
    return getTcpMemRatio() * 100;
  }

  // Gets the low TCP memory threshold (0-100) of the system as a percent
  double getLowTcpMemThresholdPct() const {
    return getLowTcpMemLimitRatio() * 100;
  }

  // Gets the Pressure TCP memory threshold (0-100) of the system as a percent
  double getPressureTcpMemThresholdPct() const {
    return getPressureTcpMemLimitRatio() * 100;
  }

  // Returns True if TCP memory fields were initialized successfully,
  // returns False otherwise
  bool tcpMemoryStatsCollected() const {
    return tcpMemoryPages_ != 0 && maxTcpMemLimit_ != 0 &&
           pressureTcpMemLimit_ != 0 && minTcpMemLimit_ != 0;
  }

  // Returns current level of UDP memory consumption
  // measured in memory pages
  uint64_t getUdpMemPages() const {
    return udpMemoryPages_;
  }

  // Returns Low UDP Memory threshold measured in memory pages
  uint64_t getLowUdpMemLimitPages() const {
    return minUdpMemLimit_;
  }

  // Returns Pressure UDP Memory threshold measured in memory pages
  uint64_t getPressureUdpMemLimitPages() const {
    return pressureUdpMemLimit_;
  }

  // Returns Low UDP Memory threshold measured in memory pages
  uint64_t getMaxUdpMemLimitPages() const {
    return maxUdpMemLimit_;
  }

  // Gets the used UDP memory (0-1.0) as a ratio to max UDP mem limit
  double getUdpMemRatio() const {
    return calculateRatio(udpMemoryPages_, maxUdpMemLimit_);
  }

  // Gets the low UDP memory threshold (0-1.0) as a ratio to
  // max UDP mem limit
  double getLowUdpMemLimitRatio() const {
    return calculateRatio(minUdpMemLimit_, maxUdpMemLimit_);
  }

  // Gets the UDP memory pressure threshold (0-1.0) as a ratio to
  // max UDP mem limit
  double getPressureUdpMemLimitRatio() const {
    return calculateRatio(pressureUdpMemLimit_, maxUdpMemLimit_);
  }

  // Gets the used UDP memory (0-100) of the system as a percent
  double getUsedUdpMemPct() const {
    return getUdpMemRatio() * 100;
  }

  // Gets the low UDP memory threshold (0-100) of the system as a percent
  double getLowUdpMemThresholdPct() const {
    return getLowUdpMemLimitRatio() * 100;
  }

  // Gets the Pressure UDP memory threshold (0-100) of the system as a percent
  double getPressureUdpMemThresholdPct() const {
    return getPressureUdpMemLimitRatio() * 100;
  }

  // Returns True if UDP memory fields were initialized successfully,
  // returns False otherwise
  bool udpMemoryStatsCollected() const {
    return udpMemoryPages_ != 0 && maxUdpMemLimit_ != 0 &&
           pressureUdpMemLimit_ != 0 && minUdpMemLimit_ != 0;
  }

  void setCpuStats(double cpuRatioUtil,
                   double cpuSoftIrqRatioUtil,
                   std::vector<double>&& softIrqCpuCoreRatioUtils) {
    cpuRatioUtil_ = cpuRatioUtil;
    cpuSoftIrqRatioUtil_ = cpuSoftIrqRatioUtil;
    softIrqCpuCoreRatioUtils_ = softIrqCpuCoreRatioUtils;
  }

  void setMemStats(uint64_t usedMemBytes, uint64_t totalMemBytes) {
    usedMemBytes_ = usedMemBytes;
    totalMemBytes_ = totalMemBytes;
  }

  /**
   * Sets the structure fields describing TCP memory state.
   */
  void setTcpMemStats(uint64_t current,
                      uint64_t minThreshold,
                      uint64_t pressureThreshold,
                      uint64_t maxThreshold) {
    tcpMemoryPages_ = current;
    minTcpMemLimit_ = minThreshold;
    pressureTcpMemLimit_ = pressureThreshold;
    maxTcpMemLimit_ = maxThreshold;
  }

  /**
   * Sets the structure fields describing UDP memory state.
   */
  void setUdpMemStats(uint64_t current,
                      uint64_t minThreshold,
                      uint64_t pressureThreshold,
                      uint64_t maxThreshold) {
    udpMemoryPages_ = current;
    minUdpMemLimit_ = minThreshold;
    pressureUdpMemLimit_ = pressureThreshold;
    maxUdpMemLimit_ = maxThreshold;
  }

 protected:
  // Convert an absolute integer value and max limit to a float point ratio.
  double calculateRatio(uint64_t value, uint64_t maxLimit) const {
    if (maxLimit != 0) {
      return double(value) / maxLimit;
    }
    return 0.0;
  }

  // Resource utilization metrics
  double cpuRatioUtil_{0};
  double cpuSoftIrqRatioUtil_{0};
  std::vector<double> softIrqCpuCoreRatioUtils_;
  uint64_t usedMemBytes_{0};
  uint64_t totalMemBytes_{0};
  uint64_t tcpMemoryPages_{0};
  uint64_t maxTcpMemLimit_{0};
  uint64_t pressureTcpMemLimit_{0};
  uint64_t minTcpMemLimit_{0};
  uint64_t udpMemoryPages_{0};
  uint64_t maxUdpMemLimit_{0};
  uint64_t pressureUdpMemLimit_{0};
  uint64_t minUdpMemLimit_{0};
};

/**
 * A class that abstracts away actually fetching the underlying data.
 * Handles abstraction of the fetching (from particular OSes, container
 * systems, etc)
 */
class Resources {
 public:
  virtual ~Resources() = default;

  /**
   * getCurrentData performs the querying resource utilization metrics.
   */
  virtual ResourceData getCurrentData() = 0;
};

} // namespace proxygen
