/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/container/F14Map.h>

#include "squangle/mysql_client/PoolKey.h"
#include "squangle/mysql_client/TwoLevelCache.h"

namespace facebook::common::mysql_client {

template <typename Client>
class ConnectionPool;
template <typename Client>
class ConnectPoolOperation;
template <typename Client>
class MysqlPooledHolder;

template <typename Client>
// Auxiliary class to isolate the queue code. Clean ups also happen in this
// class, it mainly manages the ConnectPoolOperation and
// MysqlPooledHolder containers.
class PoolStorage {
 public:
  using PoolOpList = std::list<std::weak_ptr<ConnectPoolOperation<Client>>>;

  // At most this number of keys will be compared in the level2 cache to find
  // a key having the largest number of connections
  const int kMaxKeysToCheckInLevel2 = 3;
  explicit PoolStorage(size_t conn_limit, Duration max_idle_time)
      : conn_limit_(conn_limit), max_idle_time_(max_idle_time) {}

  ~PoolStorage() {}

  // Returns an shared pointer of the oldest valid operation in the queue
  // for the given PoolKey. The returned operation is removed from the
  // queue. We return shared to avoid the instance dying (for any reason)
  // before the connection is given to it.
  std::shared_ptr<ConnectPoolOperation<Client>> popOperation(
      const PoolKey& pool_key) {
    auto& list = waitList_[pool_key];
    while (!list.empty()) {
      std::weak_ptr<ConnectPoolOperation<Client>> weak_op = list.front();
      list.pop_front();
      auto ret = weak_op.lock();
      if (ret && !ret->done()) {
        return ret;
      }
    }

    return nullptr;
  }

  // Puts the new operation in the end of the list
  void queueOperation(
      const PoolKey& pool_key,
      std::shared_ptr<ConnectPoolOperation<Client>>& pool_op) {
    auto& list = waitList_[pool_key];
    std::weak_ptr<ConnectPoolOperation<Client>> weak_op = pool_op;
    list.push_back(std::move(weak_op));
  }

  // Calls failureCallback with the error description and removed all
  // the operations for conn_key from the queue.
  void failOperations(
      const PoolKey& pool_key,
      OperationResult op_result,
      unsigned int mysql_errno,
      const std::string& mysql_error) {
    auto& list = waitList_[pool_key];
    while (!list.empty()) {
      std::weak_ptr<ConnectPoolOperation<Client>> weak_op = list.front();
      list.pop_front();
      auto lock_op = weak_op.lock();
      if (lock_op && !lock_op->done()) {
        lock_op->failureCallback(op_result, mysql_errno, mysql_error);
      }
    }
  }

  // Returns a connection for the given ConnectionKey. The connection will
  // be removed from the queue. Depending on the policy, it will give the
  // oldest inserted connection (fifo) or the most recent inserted (lifo).
  std::unique_ptr<MysqlPooledHolder<Client>> popConnection(
      const PoolKey& pool_key) {
    return stock_.popLevel1(pool_key);
  }

  // Returns a connection for the given ConnectionKey, ignoring db name.
  // In order to reuse this connection, we need to run COM_CHANGE_USER.
  std::unique_ptr<MysqlPooledHolder<Client>> popInstanceConnection(
      const PoolKey& pool_key) {
    size_t tries = 0;
    size_t bestSize = 0;
    // Find a pool key from level2_ cache, which has the largest number of
    // connections in level1_ cache. This function checks at most
    // "kMaxKeysToCheckInLevel2" number of keys in level2_ cache.
    return stock_.popLevel2(
        pool_key,
        [&](const auto& key1, const auto& key2) mutable -> std::optional<bool> {
          if (++tries == kMaxKeysToCheckInLevel2) {
            return std::nullopt;
          }

          if (bestSize == 0) {
            bestSize = stock_.level1Size(key1);
          }
          auto testSize = stock_.level1Size(key2);
          if (testSize > bestSize) {
            bestSize = testSize;
            return true;
          }
          return false;
        });
  }

  // Puts the new connection in the back of the list.
  void queueConnection(std::unique_ptr<MysqlPooledHolder<Client>> newConn) {
    const auto& key = newConn->getPoolKey();
    stock_.push(key, std::move(newConn), conn_limit_);
  }

  // Checks and removes the connection that reached their idle time or age
  // limit.
  void cleanupConnections() {
    Timepoint now = std::chrono::steady_clock::now();
    // We keep shared_ptr here to prevent the pool from being destructed in the
    // middle of traversing the map
    std::shared_ptr<ConnectionPool<Client>> pool;
    stock_.cleanup([&](const auto& conn) {
      if (!pool) {
        pool = conn->getPoolPtr();
        if (!pool) {
          // the pool is already destroyed
          LOG(DFATAL)
              << "pool was already destroyed while cleanup was in progress";
        }
      }

      return (conn->getLifeDuration() != Duration::zero() &&
              (conn->getCreationTime() + conn->getLifeDuration() < now)) ||
          conn->getLastActivityTime() + max_idle_time_ < now;
    });
  }

  // Checks and removes the weak ptrs that already expired, so we have a
  // better approximation of the number of operations really waiting.
  void cleanupOperations() {
    for (auto mapIt = waitList_.begin(); mapIt != waitList_.end();) {
      auto& list = mapIt->second;
      for (auto it = list.begin(); it != list.end();) {
        // check if weak pointer expired
        if (auto op = it->lock(); !op || op->done()) {
          it = list.erase(it);
          VLOG(11) << "Operation being erased during clean up";
        } else {
          ++it;
        }
      }

      if (list.empty()) {
        mapIt = waitList_.erase(mapIt);
      } else {
        ++mapIt;
      }
    }
  }

  // Cancels all operations that are still in the queue and clears all the
  // storage
  void clearAll() {
    // Clearing all operations in the queue
    for (auto& [_, list] : waitList_) {
      for (auto& wptr : list) {
        // check if weak pointer expired
        if (auto locked_op = wptr.lock(); locked_op) {
          locked_op->cancel();
          VLOG(2) << "Cancelling operation in the pool during clean up";
        }
      }
    }

    waitList_.clear();
    // For the connections we don't need to close one by one, we can just
    // clear the list and leave the destructor to handle it.
    stock_.clear();
  }

  // for debugging
  void displayPoolStatus() {
    LOG(INFO) << "*** level1 cache";
    stock_.iterateLevel1([](const auto& key, const auto& value) {
      LOG(INFO) << key.connKey.getDisplayString()
                << ", # conn = " << value.size();
    });
    LOG(INFO) << "*** level2 cache";
    stock_.iterateLevel2([](const auto& key, const auto& value) {
      LOG(INFO) << key.connKey.getDisplayString(true)
                << ", # key = " << value.size();
    });
  }

  size_t numQueuedOperations(const PoolKey& pool_key) const {
    auto it = waitList_.find(pool_key);
    return it == waitList_.end() ? 0 : it->second.size();
  }

  FOLLY_NODISCARD Duration maxIdleTime() const noexcept {
    return max_idle_time_;
  }

  FOLLY_NODISCARD size_t getNumKey() const noexcept {
    return stock_.level1NumKey();
  }

 private:
  // This pool holds weak_ptr to the operation in wait list to avoid holding
  // async client in the draining process in case the operation has already
  // been discarded by the creator before got a connection. This also serves
  // to avoid giving them connections
  TwoLevelCache<
      Client,
      PoolKey,
      std::unique_ptr<MysqlPooledHolder<Client>>,
      PoolKeyHash,
      PoolKeyPartialHash>
      stock_;

  folly::F14NodeMap<PoolKey, PoolOpList, PoolKeyHash> waitList_;

  size_t conn_limit_;
  Duration max_idle_time_;
};

} // namespace facebook::common::mysql_client
