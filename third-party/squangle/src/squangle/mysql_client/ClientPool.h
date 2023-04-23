/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/AsyncMysqlClient.h"

#include <folly/Random.h>
#include <folly/Singleton.h>
#include <folly/hash/Hash.h>

namespace facebook {
namespace common {
namespace mysql_client {

class AsyncMysqlClientFactory {
 public:
  std::shared_ptr<AsyncMysqlClient> makeClient() {
    return std::shared_ptr<AsyncMysqlClient>(
        new AsyncMysqlClient(), AsyncMysqlClient::deleter);
  }
};

/*
 * For MySQL heavy applications, we require more AsyncMysqlClient's.
 * Given that each spins a thread.
 * This is a simple round robin of pools.
 */
// TClientFactory is here just to allow the PoolOptions to be passed as well.
template <class TClient, class TClientFactory>
class ClientPool {
 public:
  explicit ClientPool(
      std::unique_ptr<TClientFactory> client_factory,
      size_t num_clients = 10) {
    if (num_clients == 0) {
      throw std::logic_error(
          "Invalid number of clients, it needs to be more than 0");
    }
    client_pool_.reserve(num_clients);
    for (int i = 0; i < num_clients; ++i) {
      client_pool_.emplace_back(client_factory->makeClient());
    }
  }

  std::shared_ptr<TClient> getClient() const {
    auto idx = folly::Random::rand32() % client_pool_.size();
    return client_pool_[idx];
  }

  template <typename F>
  void forEachClient(F func) {
    for (auto& client : client_pool_) {
      func(client);
    }
  }

  // Passing in a key will allow the use of a consistent AsyncConnectionPool
  // object per key. This will greatly increase pool hits as currently
  // the multiple pools do not share any resources. This also allows the
  // MultiPool to respect limits
  std::shared_ptr<TClient> getClient(const std::string& key) const {
    return getClient(folly::Hash()(key));
  }

  // Using size_t key to index the client pool
  std::shared_ptr<TClient> getClient(size_t key) const {
    return client_pool_[key % client_pool_.size()];
  }

  static std::shared_ptr<TClient> getClientFromDefault() {
    auto client_pool =
        folly::Singleton<ClientPool<TClient, TClientFactory>>::try_get();
    if (!client_pool) {
      throw std::logic_error(
          "MultiMysqlClientPool singleton has already been destroyed.");
    }
    return client_pool->getClient();
  }

  static std::shared_ptr<TClient> getClientFromDefault(const std::string& key) {
    auto client_pool =
        folly::Singleton<ClientPool<TClient, TClientFactory>>::try_get();
    if (!client_pool) {
      throw std::logic_error(
          "MultiMysqlClientPool singleton has already been destroyed.");
    }
    return client_pool->getClient(key);
  }

 private:
  std::vector<std::shared_ptr<TClient>> client_pool_;
};
} // namespace mysql_client
} // namespace common
} // namespace facebook
