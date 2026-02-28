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

#include <wangle/acceptor/LoadShedConfiguration.h>

#include <folly/Conv.h>
#include <folly/portability/OpenSSL.h>

using folly::SocketAddress;
using std::string;

namespace wangle {

void LoadShedConfiguration::addAllowlistAddr(folly::StringPiece input) {
  auto addr = input.str();
  size_t separator = addr.find_first_of('/');
  if (separator == string::npos) {
    allowlistAddrs_.insert(SocketAddress(addr, 0));
  } else {
    unsigned prefixLen = folly::to<unsigned>(addr.substr(separator + 1));
    addr.erase(separator);
    allowlistNetworks_.insert(
        NetworkAddress(SocketAddress(addr, 0), prefixLen));
  }
}

bool LoadShedConfiguration::isAllowlisted(const SocketAddress& address) const {
  if (allowlistAddrs_.find(address) != allowlistAddrs_.end()) {
    return true;
  }
  for (auto& network : allowlistNetworks_) {
    if (network.contains(address)) {
      return true;
    }
  }
  return false;
}

void LoadShedConfiguration::checkIsSane(const SysParams& sysParams) const {
  if (loadSheddingEnabled_) {
    // Cpu soft/hard limit ratios must have values in the range of [0-1],
    // inclusive, where the hard limit must be greater than or equal to the
    // soft limit.
    CHECK_GE(cpuHardLimitRatio_, 0.0);
    CHECK_LE(cpuHardLimitRatio_, 1.0);
    CHECK_GE(cpuSoftLimitRatio_, 0.0);
    CHECK_LE(cpuSoftLimitRatio_, cpuHardLimitRatio_);

    // CPU exceed window must be of size at least equal to 1.
    CHECK_GE(cpuUsageExceedWindowSize_, 1);

    // Cpu irq soft/hard limit ratios must have values in the range of [0-1],
    // inclusive, where the hard limit must be greater than or equal to the
    // soft limit.
    CHECK_GE(softIrqLogicalCpuCoreQuorum_, 0);
    CHECK_LE(softIrqLogicalCpuCoreQuorum_, sysParams.numLogicalCpuCores);
    CHECK_GE(softIrqCpuHardLimitRatio_, 0.0);
    CHECK_LE(softIrqCpuHardLimitRatio_, 1.0);
    CHECK_GE(softIrqCpuSoftLimitRatio_, 0.0);
    CHECK_LE(softIrqCpuSoftLimitRatio_, softIrqCpuHardLimitRatio_);

    // Mem soft/hard limit ratios must have values in the range of [0-1],
    // inclusive, where the hard limit must be greater than or equal to the
    // soft limit.  We allow the mem kill limit ratio to be any valid value
    // in the same inclusive range of [0-1].
    CHECK_GE(memHardLimitRatio_, 0.0);
    CHECK_LE(memHardLimitRatio_, 1.0);
    CHECK_GE(memSoftLimitRatio_, 0.0);
    CHECK_LE(memSoftLimitRatio_, memHardLimitRatio_);
    CHECK_GE(memKillLimitRatio_, memHardLimitRatio_);
    CHECK_LE(memKillLimitRatio_, 1.0);

    // TCP/UDP mem soft/hard limit ratios must have values in the range of
    // [0-1], inclusive, where the hard limit must be greater than or equal
    // to the soft limit.
    CHECK_GE(tcpMemHardLimitRatio_, 0.0);
    CHECK_LE(tcpMemHardLimitRatio_, 1.0);
    CHECK_GE(tcpMemSoftLimitRatio_, 0.0);
    CHECK_LE(tcpMemSoftLimitRatio_, tcpMemHardLimitRatio_);
    CHECK_GE(udpMemHardLimitRatio_, 0.0);
    CHECK_LE(udpMemHardLimitRatio_, 1.0);
    CHECK_GE(udpMemSoftLimitRatio_, 0.0);
    CHECK_LE(udpMemSoftLimitRatio_, udpMemHardLimitRatio_);

    // Period must be greater than or equal to 0.
    CHECK_GE(period_.count(), std::chrono::milliseconds(0).count());
  }
}

} // namespace wangle
