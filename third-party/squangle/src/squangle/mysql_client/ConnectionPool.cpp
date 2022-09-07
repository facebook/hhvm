/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/ConnectionPool.h"

namespace facebook::common::mysql_client {

bool ConnectionPoolBase::canCreateMoreConnections(
    const PoolKey& pool_key,
    size_t enqueued_pool_ops,
    uint32_t client_total_conns,
    uint64_t client_conn_limit,
    const Counters& counters) const {
  // We have the number of connections we are opening and the number of already
  // open, we shouldn't try to create over this sum
  auto total_conns =
      counters.num_open_connections + counters.num_pending_connections;
  auto open_for_key =
      folly::get_default(counters.open_connections, pool_key, 0);
  auto pending_for_key =
      folly::get_default(counters.pending_connections, pool_key, 0);
  int conns_for_key = open_for_key + pending_for_key;

  // First we check global limit, then limits of the pool. If we can create more
  // connections, we check if we need comparing the amount of already being
  // opened connections for that key with the number of enqueued operations (the
  // operation that is requesting a new connection should be enqueued at this
  // point.
  return client_total_conns < client_conn_limit && total_conns < totalLimit() &&
      conns_for_key < perKeyLimit() && pending_for_key < enqueued_pool_ops;
}

void ConnectionPoolBase::addOpenConnection(const PoolKey& pool_key) {
  counters_.withWLock([&](auto& locked) {
    ++locked.open_connections[pool_key];
    ++locked.num_open_connections;
  });
}

void ConnectionPoolBase::removeOpenConnection(const PoolKey& pool_key) {
  counters_.withWLock([&](auto& locked) {
    auto iter = locked.open_connections.find(pool_key);
    DCHECK(iter != locked.open_connections.end());
    if (--iter->second == 0) {
      locked.open_connections.erase(iter);
    }

    --locked.num_open_connections;
  });

  connectionSpotFreed(pool_key);
}

// Checks if the limits (global, connections open or being open by pool, or
// limit per key) can fit one more connection. As a final check, checks if
// it's a waste to create a new connection to avoid start opening a new
// connection when we already have enough being open for the demand in
// queue.  If we have enough space we increment the counts.
bool ConnectionPoolBase::tryAddOpeningConn(
    const PoolKey& pool_key,
    size_t enqueued_pool_ops,
    uint32_t client_total_conns,
    uint64_t client_conn_limit) {
  return counters_.withWLock([&](auto& locked) {
    if (canCreateMoreConnections(
            pool_key,
            enqueued_pool_ops,
            client_total_conns,
            client_conn_limit,
            locked)) {
      ++locked.pending_connections[pool_key];
      ++locked.num_pending_connections;
      return true;
    }

    return false;
  });
}

void ConnectionPoolBase::removeOpeningConn(const PoolKey& pool_key) {
  counters_.withWLock([&](auto& locked) {
    auto num = --locked.pending_connections[pool_key];
    DCHECK_GE(int64_t(num), 0);
    if (num == 0) {
      locked.pending_connections.erase(pool_key);
    }
    --locked.num_pending_connections;
  });
}

void ConnectionPoolBase::displayOpenConnections() {
  counters_.withRLock([](const auto& locked) {
    LOG(INFO) << "*** Open connections";
    for (const auto& [key, value] : locked.open_connections) {
      LOG(INFO) << key.connKey.getDisplayString() << ": " << value;
    }

    LOG(INFO) << "*** Pending connections";
    for (const auto& [key, value] : locked.pending_connections) {
      LOG(INFO) << key.connKey.getDisplayString() << ": " << value;
    }
  });
}

int ConnectionPoolBase::getNumKeysInOpenConnections() {
  return counters_.rlock()->open_connections.size();
}

int ConnectionPoolBase::getNumKeysInPendingConnections() {
  return counters_.rlock()->pending_connections.size();
}

PoolKeyStats ConnectionPoolBase::getPoolKeyStats(
    const PoolKey& pool_key) const {
  return counters_.withRLock([&](const auto& locked) {
    PoolKeyStats stats;
    stats.open_connections =
        folly::get_default(locked.open_connections, pool_key, 0);
    stats.pending_connections =
        folly::get_default(locked.pending_connections, pool_key, 0);
    stats.connection_limit = perKeyLimit();
    return stats;
  });
}

} // namespace facebook::common::mysql_client
