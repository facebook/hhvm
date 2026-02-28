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

#include <folly/Range.h>
#include <folly/SocketAddress.h>
#include <glog/logging.h>
#include <chrono>
#include <list>
#include <set>
#include <string>

#include <wangle/acceptor/NetworkAddress.h>

namespace wangle {

/**
 * Class that holds an LoadShed configuration for a service
 */
class LoadShedConfiguration {
 public:
  // Comparison function for SocketAddress that disregards the port
  struct AddressOnlyCompare {
    bool operator()(
        const folly::SocketAddress& addr1,
        const folly::SocketAddress& addr2) const {
      return addr1.getIPAddress() < addr2.getIPAddress();
    }
  };

  using AddressSet = std::set<folly::SocketAddress, AddressOnlyCompare>;
  using NetworkSet = std::set<NetworkAddress>;

  LoadShedConfiguration() = default;

  virtual ~LoadShedConfiguration() = default;

  void addAllowlistAddr(folly::StringPiece);

  /**
   * Set/get the set of IPs that should be allowlisted through even when we're
   * trying to shed load.
   */
  void setAllowlistAddrs(const AddressSet& addrs) {
    allowlistAddrs_ = addrs;
  }
  const AddressSet& getAllowlistAddrs() const {
    return allowlistAddrs_;
  }

  /**
   * Set/get the set of networks that should be allowlisted through even
   * when we're trying to shed load.
   */
  void setAllowlistNetworks(const NetworkSet& networks) {
    allowlistNetworks_ = networks;
  }
  const NetworkSet& getAllowlistNetworks() const {
    return allowlistNetworks_;
  }

  /**
   * Set/get the cpu usage soft limit.
   * Various shedding protections should engage when above this limit.
   */
  void setCpuSoftLimitRatio(double limit) {
    CHECK_GE(limit, 0.0);
    CHECK_LE(limit, 1.0);
    cpuSoftLimitRatio_ = limit;
  }
  double getCpuSoftLimitRatio() const {
    return cpuSoftLimitRatio_;
  }

  /**
   * Set/get the cpu usage hard limit.
   * More extreme shedding protections should engage when above this limit.
   */
  void setCpuHardLimitRatio(double limit) {
    CHECK_GE(limit, 0.0);
    CHECK_LE(limit, 1.0);
    cpuHardLimitRatio_ = limit;
  }
  double getCpuHardLimitRatio() const {
    return cpuHardLimitRatio_;
  }

  /**
   * Set/get the CPU usage exceed window size
   */
  void setCpuUsageExceedWindowSize(const uint64_t size) {
    cpuUsageExceedWindowSize_ = size;
  }
  uint64_t getCpuUsageExceedWindowSize() const {
    return cpuUsageExceedWindowSize_;
  }

  /**
   * Set/get the number of most soft utilized cpu cores to use when comparing
   * against soft cpu limits; a value of 0 or a value that equals the total
   * number of cores on the executing system implies that mean CPU should be
   * used.  This field exists to more meaningfully handle uneven load
   * distributions that can occur if for example network card interrupt
   * affinity is set such that only X of Y cores are utilized for network
   * packet processing.
   */
  void setSoftIrqLogicalCpuCoreQuorum(uint64_t quorum) {
    softIrqLogicalCpuCoreQuorum_ = quorum;
  }
  uint64_t getSoftIrqLogicalCpuCoreQuorum() const {
    return softIrqLogicalCpuCoreQuorum_;
  }

  /**
   * Set/get the soft cpu usage soft limit ratio.
   * Various shedding protections should engage when above this limit.
   */
  void setSoftIrqCpuSoftLimitRatio(double limit) {
    CHECK_GE(limit, 0.0);
    CHECK_LE(limit, 1.0);
    softIrqCpuSoftLimitRatio_ = limit;
  }
  double getSoftIrqCpuSoftLimitRatio() const {
    return softIrqCpuSoftLimitRatio_;
  }

  /**
   * Set/get the soft cpu usage hard limit ratio.
   * More extreme shedding protections should engage when above this limit.
   */
  void setSoftIrqCpuHardLimitRatio(double limit) {
    CHECK_GE(limit, 0.0);
    CHECK_LE(limit, 1.0);
    softIrqCpuHardLimitRatio_ = limit;
  }
  double getSoftIrqCpuHardLimitRatio() const {
    return softIrqCpuHardLimitRatio_;
  }

  /**
   * Set/get the memory usage soft limit ratio.
   * Various shedding protections should engage when above this limit.
   */
  void setMemSoftLimitRatio(double limit) {
    CHECK_GE(limit, 0.0);
    CHECK_LE(limit, 1.0);
    memSoftLimitRatio_ = limit;
  }
  double getMemSoftLimitRatio() const {
    return memSoftLimitRatio_;
  }

  /**
   * Set/get the memory usage hard limit ratio.
   * More extreme shedding protections should engage when above this limit.
   */
  void setMemHardLimitRatio(double limit) {
    CHECK_GE(limit, 0.0);
    CHECK_LE(limit, 1.0);
    memHardLimitRatio_ = limit;
  }
  double getMemHardLimitRatio() const {
    return memHardLimitRatio_;
  }

  /**
   * Set/get the memory usage kill limit ratio.
   * Threshold above which the process should abort (self-terimate) in order
   * to protect the underlying host.
   */
  void setMemKillLimitRatio(double limit) {
    CHECK_GE(limit, 0.0);
    CHECK_LE(limit, 1.0);
    memKillLimitRatio_ = limit;
  }
  double getMemKillLimitRatio() const {
    return memKillLimitRatio_;
  }

  /**
   * Set/get the tcp memory usage soft limit ratio.
   * Various shedding protections should engage when above this limit.
   */
  void setTcpMemSoftLimitRatio(double limit) {
    CHECK_GE(limit, 0.0);
    CHECK_LE(limit, 1.0);
    tcpMemSoftLimitRatio_ = limit;
  }
  double getTcpMemSoftLimitRatio() const {
    return tcpMemSoftLimitRatio_;
  }

  /**
   * Set/get the tcp memory usage hard limit ratio.
   * More extreme shedding protections should engage when above this limit.
   */
  void setTcpMemHardLimitRatio(double limit) {
    CHECK_GE(limit, 0.0);
    CHECK_LE(limit, 1.0);
    tcpMemHardLimitRatio_ = limit;
  }
  double getTcpMemHardLimitRatio() const {
    return tcpMemHardLimitRatio_;
  }

  /**
   * Set/get the udp memory usage soft limit ratio.
   * Various shedding protections should engage when above this limit.
   */
  void setUdpMemSoftLimitRatio(double limit) {
    CHECK_GE(limit, 0.0);
    CHECK_LE(limit, 1.0);
    udpMemSoftLimitRatio_ = limit;
  }
  double getUdpMemSoftLimitRatio() const {
    return udpMemSoftLimitRatio_;
  }

  /**
   * Set/get the udp memory usage hard limit ratio.
   * More extreme shedding protections should engage when above this limit.
   */
  void setUdpMemHardLimitRatio(double limit) {
    CHECK_GE(limit, 0.0);
    CHECK_LE(limit, 1.0);
    udpMemHardLimitRatio_ = limit;
  }
  double getUdpMemHardLimitRatio() const {
    return udpMemHardLimitRatio_;
  }

  void setLoadUpdatePeriod(std::chrono::milliseconds period) {
    period_ = period;
  }
  std::chrono::milliseconds getLoadUpdatePeriod() const {
    return period_;
  }

  void setLoadSheddingEnabled(bool enabled) {
    loadSheddingEnabled_ = enabled;
  }

  bool getLoadSheddingEnabled() const {
    return loadSheddingEnabled_;
  }

  bool isAllowlisted(const folly::SocketAddress& addr) const;

  /**
   * Performs a series of CHECKs to ensure the underlying configuration is
   * sane.
   * For example the following must be true: (1.0 - minCpuIdle_) >= maxCpuUsage_
   * Note: totalMemBytes represents the total system memory to be used when
   * normalizing minFreeMem_ and killMinFreeMemBytes_ in order for the
   * associated comparisons.
   */
  struct SysParams {
    uint64_t numLogicalCpuCores{0};
    uint64_t totalMemBytes{0};
  };
  virtual void checkIsSane(const SysParams& sysParams) const;

 private:
  AddressSet allowlistAddrs_;
  NetworkSet allowlistNetworks_;

  double cpuSoftLimitRatio_{1.0};
  double cpuHardLimitRatio_{1.0};
  uint64_t cpuUsageExceedWindowSize_{0};

  uint64_t softIrqLogicalCpuCoreQuorum_{0};
  double softIrqCpuSoftLimitRatio_{1.0};
  double softIrqCpuHardLimitRatio_{1.0};

  double memSoftLimitRatio_{1.0};
  double memHardLimitRatio_{1.0};
  double memKillLimitRatio_{1.0};

  double tcpMemSoftLimitRatio_{1.0};
  double tcpMemHardLimitRatio_{1.0};

  double udpMemSoftLimitRatio_{1.0};
  double udpMemHardLimitRatio_{1.0};

  std::chrono::milliseconds period_;

  bool loadSheddingEnabled_{true};
};

} // namespace wangle
