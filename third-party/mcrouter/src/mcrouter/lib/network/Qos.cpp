/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/network/Qos.h"

namespace facebook {
namespace memcache {

/**
 * Validates the qosClass and qosPath sets appropriate qos if valid.
 * Returns true on success and false otherwise.
 *
 */
bool getQoS(uint64_t qosClassLvl, uint64_t qosPathLvl, uint64_t& qos) {
  // class
  constexpr uint64_t kDefaultClass = 0x00;
  constexpr uint64_t kLowestClass = 0x20;
  constexpr uint64_t kMediumClass = 0x40;
  constexpr uint64_t kHighClass = 0x60;
  constexpr uint64_t kHighestClass = 0x80;
  constexpr uint64_t kMaxClassLvl = 4;
  constexpr uint64_t kQoSClasses[] = {
      kDefaultClass, kLowestClass, kMediumClass, kHighClass, kHighestClass};

  // path
  constexpr uint64_t kAnyPathNoProtection = 0x00;
  constexpr uint64_t kAnyPathProtection = 0x04;
  constexpr uint64_t kShortestPathNoProtection = 0x08;
  constexpr uint64_t kShortestPathProtection = 0x0c;
  constexpr uint64_t kMaxPathLvl = 3;
  constexpr uint64_t kQoSPaths[] = {
      kAnyPathNoProtection,
      kAnyPathProtection,
      kShortestPathNoProtection,
      kShortestPathProtection};

  if (qosClassLvl > kMaxClassLvl || qosPathLvl > kMaxPathLvl) {
    qos = kQoSClasses[0] | kQoSPaths[0];
    return false;
  }
  qos = kQoSClasses[qosClassLvl] | kQoSPaths[qosPathLvl];
  return true;
}

} // namespace memcache
} // namespace facebook
