/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

#include <folly/IntrusiveList.h>
#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/experimental/FunctionScheduler.h>

namespace folly {
class AsyncTimeout;
} // namespace folly

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Class responsible for mapping lease-tokens to destinations.
 * All operations are thread-safe.
 */
class LeaseTokenMap {
 public:
  /**
   * Item stored in this data structure.
   */
  struct Item {
    uint64_t originalToken;
    size_t routeHandleChildIndex;
  };

  /**
   * Creates a LeaseTokenMap.
   *
   * @param functionScheduler FunctionScheduler responsible for timeouts.
   * @param leaseTokenTtl     How many milliseconds the lease token will live.
   *                          Must be greater than 0.
   * @param cleanupInterval   Interval at which lease tokens are cleaned up.
   */
  explicit LeaseTokenMap(
      const std::shared_ptr<folly::FunctionScheduler>& functionScheduler,
      std::chrono::milliseconds leaseTokenTtl =
          std::chrono::milliseconds(10000),
      std::chrono::milliseconds cleanupInterval =
          std::chrono::milliseconds(500));
  ~LeaseTokenMap();

  /**
   * Inserts a lease token into the map and returns a special token.
   *
   * @param routeName   The name of the route handle that processed the
   *                    lease-get.
   * @param item        Item to insert into the map, containing the original
   *                    token (i.e. lease token returned by memcached) and
   *                    the destination that requests with this token should
   *                    be redirected to.
   * @return            Special token, that should be returned to
   *                    the client.
   */
  uint64_t insert(std::string routeName, Item item);

  /**
   * Queries the map for a special token. If found, the entry is
   * deleted from the map.
   *
   * @param routeName   Name of the route handle.
   * @param token       Lease token provided by the client.
   * @return            If found, return an Item, containing the original
   *                    lease token and the destination. If not found, return
   *                    and empty folly::Optional.
   */
  folly::Optional<Item> query(folly::StringPiece routeName, uint64_t token);

  /**
   * Return the original lease token (i.e. the lease token returned by
   * memcached).
   *
   * @param routeName   Name of the route handle.
   * @param token       Lease token. Can be either a special token or an
   *                    ordinary lease token.
   * @return            The original lease token (i.e. the one returned by
   *                    memcached).
   */
  uint64_t getOriginalLeaseToken(folly::StringPiece routeName, uint64_t token)
      const;

  /**
   * Return the size of the data structure (i.e. how many tokens are stored).
   */
  size_t size() const;

  /**
   * Tell whether an originalToken (i.e. token returned by memcached)
   * conflicts with specialToken space (tokens returned by this data
   * structure).
   */
  static bool conflicts(uint64_t originalToken);

 private:
  struct ListItem {
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    ListItem(
        uint64_t sToken,
        std::string route,
        Item it,
        std::chrono::milliseconds timeout)
        : specialToken(sToken),
          routeName(std::move(route)),
          item(std::move(it)),
          tokenTimeout(Clock::now() + timeout) {}

    uint64_t specialToken;
    std::string routeName;
    Item item;
    TimePoint tokenTimeout;

    folly::IntrusiveListHook listHook;
  };

  // Hold the id of the next element to be inserted in the data structure.
  uint32_t nextId_{0};

  // Underlying data structure.
  std::unordered_map<uint64_t, ListItem> data_;
  // Keeps an in-order list of what should be invalidated.
  folly::IntrusiveList<ListItem, &ListItem::listHook> invalidationQueue_;
  // Mutex to synchronize access to underlying data structure
  mutable std::mutex mutex_;

  std::weak_ptr<folly::FunctionScheduler> functionScheduler_;
  const std::string timeoutFunctionName_;
  std::chrono::milliseconds leaseTokenTtl_;

  void tokenCleanupTimeout();
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
