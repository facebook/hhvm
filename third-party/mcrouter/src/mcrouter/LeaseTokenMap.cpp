/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LeaseTokenMap.h"

#include <folly/Conv.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

constexpr uint64_t kClearIdMask = 0xFFFFFFFF00000000;
constexpr uint64_t kAddMagicMask = 0x7aceb00c00000000;

inline bool hasMagic(uint64_t token) {
  return (token & kClearIdMask) == kAddMagicMask;
}
inline uint64_t applyMagic(uint32_t id) {
  return kAddMagicMask | id;
}

std::string leaseTokenTimeoutFunctionName() {
  static std::atomic<uint64_t> uniqueId(0);
  return folly::to<std::string>(
      "carbon-lease-token-timeout-", uniqueId.fetch_add(1));
}

} // namespace

LeaseTokenMap::LeaseTokenMap(
    const std::shared_ptr<folly::FunctionScheduler>& functionScheduler,
    std::chrono::milliseconds leaseTokenTtl,
    std::chrono::milliseconds cleanupInterval)
    : functionScheduler_(functionScheduler),
      timeoutFunctionName_(leaseTokenTimeoutFunctionName()),
      leaseTokenTtl_(leaseTokenTtl) {
  assert(leaseTokenTtl_.count() > 0);
  if (!functionScheduler) {
    throw std::runtime_error("null function scheduler");
  }
  functionScheduler->addFunction(
      [this]() { tokenCleanupTimeout(); },
      cleanupInterval,
      timeoutFunctionName_,
      cleanupInterval);
}

LeaseTokenMap::~LeaseTokenMap() {
  if (auto functionScheduler = functionScheduler_.lock()) {
    functionScheduler->cancelFunctionAndWait(timeoutFunctionName_);
  }
}

uint64_t LeaseTokenMap::insert(std::string routeName, Item item) {
  std::lock_guard<std::mutex> lock(mutex_);

  uint64_t specialToken = applyMagic(nextId_++);

  auto it = data_.emplace(
      specialToken,
      LeaseTokenMap::ListItem(
          specialToken, std::move(routeName), std::move(item), leaseTokenTtl_));
  invalidationQueue_.push_back(it.first->second);

  return specialToken;
}

folly::Optional<LeaseTokenMap::Item> LeaseTokenMap::query(
    folly::StringPiece routeName,
    uint64_t token) {
  folly::Optional<LeaseTokenMap::Item> item;

  if (!hasMagic(token)) {
    return item;
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = data_.find(token);
    if (it != data_.end() && it->second.routeName == routeName) {
      item.emplace(std::move(it->second.item));
      data_.erase(it);
    }
  }

  return item;
}

uint64_t LeaseTokenMap::getOriginalLeaseToken(
    folly::StringPiece routeName,
    uint64_t token) const {
  if (!hasMagic(token)) {
    return token;
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = data_.find(token);
    if (it != data_.end() && it->second.routeName == routeName) {
      return it->second.item.originalToken;
    }
  }
  return token;
}

void LeaseTokenMap::tokenCleanupTimeout() {
  const auto now = ListItem::Clock::now();
  std::lock_guard<std::mutex> lock(mutex_);
  auto cur = invalidationQueue_.begin();
  while (cur != invalidationQueue_.end() && cur->tokenTimeout <= now) {
    uint64_t specialToken = cur->specialToken;
    cur = invalidationQueue_.erase(cur);
    data_.erase(specialToken);
  }
}

size_t LeaseTokenMap::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return data_.size();
}

bool LeaseTokenMap::conflicts(uint64_t originalToken) {
  return hasMagic(originalToken);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
