/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Asynchronous Connection Pool based on our async MySQL client.
//
// This pool offers na async way to acquire a connection by creating new ones
// or recycling an existing one. Also provides a way to limit the number of
// open connections per database/user and for the client.
//
// AsyncConnectionPool - This pool holds multiple MySQL connections and
//   manages them to make sure only healthy connections are given back.
//   The interface to request a connection works just like the
//   AsyncMysqlClient, an ConnectPoolOperation is started by `beginConnection`.
//
// ConnectPoolOperation - An abstraction of ConnectOperation that instead of
//   opening a new connection, requests a connection to the pool it was created
//   by. The usage and error treat are the same.

#ifndef COMMON_ASYNC_CONNECTION_POOL_H
#define COMMON_ASYNC_CONNECTION_POOL_H

#include <folly/String.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/futures/Future.h>
#include <chrono>
#include <list>
#include <memory>

#include "squangle/logger/DBEventCounter.h"
#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/Operation.h"

namespace facebook {
namespace common {
namespace mysql_client {

class ConnectPoolOperation;
class AsyncConnectionPool;
class PoolKey;

// level1_ map: Key = PoolKey, Value = list of MysqlPooledHolder
// level2_ map: Key = PoolKey (dbname is ignored), Value = set of PoolKey
//
// Pool keys will be present in level2_ set as long as its level1_ list include
// non-zero connections.
template <
    typename Key,
    typename Value,
    typename FullKeyHash,
    typename PartialKeyHash>
class TwoLevelCache {
 public:
  TwoLevelCache() {}

  void push(const Key& key, Value value, size_t max) {
    auto& list = level1_[key];
    if (list.empty()) {
      // The key is new in level1_, let's add it to level2_.
      level2_[key].insert(key);
    }

    list.push_back(std::move(value));
    if (list.size() > max) {
      list.pop_front();
    }
  }

  Value popLevel1(const Key& key) {
    if (auto it = level1_.find(key);
        it != level1_.end() && it->second.size() > 0) {
      auto ret = std::move(it->second.front());
      it->second.pop_front();

      if (it->second.empty()) {
        // The key does not exist in level1_, let's remove it from level2_.
        eraseFromLevel2(key);
        level1_.erase(it);
      }
      return ret;
    }
    return Value();
  }

  // Finds a key from level2_ cache, with a predicate comp, and pops its
  // corresponding connection from level1_ cache.
  // Predicate comp returns true, if the best key needs to be updated, false, if
  // the best key remains unchanged, or folly::none, if we need to stop
  // traversing keys in level2_.
  template <typename Pred>
  Value popLevel2(const Key& key, Pred comp) {
    if (auto it = level2_.find(key); it != level2_.end()) {
      auto test_it = it->second.begin();
      DCHECK(test_it != it->second.end());
      auto best = test_it++;
      while (test_it != it->second.end()) {
        auto res = comp(*best, *test_it);
        if (!res.hasValue()) {
          break;
        }
        if (res.value()) {
          best = test_it;
        }
        ++test_it;
      }
      return popLevel1(*best);
    }
    return Value();
  }

  // Cleans up connections in level2_ (and level1_) cache, which meet the pred.
  template <typename Pred>
  void cleanup(Pred pred) {
    // We keep shared_ptr here to prevent the pool from being destructed in the
    // middle of traversing the map
    std::shared_ptr<AsyncConnectionPool> pool;
    for (auto it1 = level1_.begin(); it1 != level1_.end();) {
      auto& list = it1->second;
      DCHECK(!list.empty());
      for (auto it2 = list.begin(); it2 != list.end();) {
        if (pred(*it2)) {
          if (!pool) {
            pool = (*it2)->getPoolPtr();
            if (!pool) {
              // the pool is already destroyed
              LOG(DFATAL)
                  << "pool was already destroyed while cleanup was in progress";
              return;
            }
          }
          it2 = list.erase(it2);
        } else {
          ++it2;
        }
      }

      if (list.empty()) {
        // The key does not exist in level1_, let's remove it from level2_
        eraseFromLevel2(it1->first);
        it1 = level1_.erase(it1);
      } else {
        ++it1;
      }
    }
  }

  void clear() {
    level1_.clear();
    level2_.clear();
  }

  template <typename Func>
  void iterateLevel1(Func func) {
    for (const auto& [key, value] : level1_) {
      func(key, value);
    }
  }

  template <typename Func>
  void iterateLevel2(Func func) {
    for (const auto& [key, value] : level2_) {
      func(key, value);
    }
  }

  size_t level1Size(const Key& key) const {
    if (auto it = level1_.find(key); it != level1_.end()) {
      return it->second.size();
    }
    return 0;
  }

  size_t level2Size(const Key& key) const {
    if (auto it = level2_.find(key); it != level2_.end()) {
      return it->second.size();
    }
    return 0;
  }

  // for debugging, return number of pool keys in level1 map
  size_t level1NumKey() const {
    return level1_.size();
  }

 private:
  using Level2Value = folly::F14FastSet<Key, FullKeyHash>;
  using Level1Map = folly::F14FastMap<Key, std::list<Value>, FullKeyHash>;
  using Level2Map =
      folly::F14FastMap<Key, Level2Value, PartialKeyHash, PartialKeyHash>;

  // This should be called before erasing the empty list from level1
  void eraseFromLevel2(const Key& key) {
    DCHECK(level1_.contains(key));
    DCHECK(level1_.at(key).empty());
    auto it = level2_.find(key);
    DCHECK(it != level2_.end());
    it->second.erase(key);
    if (it->second.empty()) {
      level2_.erase(it);
    }
  }

  Level1Map level1_;
  Level2Map level2_;
};

// In order to keep always healthy connections avoid and avoid holding one
// connection for way too long, we have the options:
//   Age: Connection will be closed when reaches a limit from the time it
// was opened. With this option the connections also get killed by idle time.
//   IdleTime: Doesn't close a connection due the total time it has been opened,
// only closes a connection due being idle for a given amount of time.
enum class ExpirationPolicy {
  Age,
  IdleTime,
  // TODO: Add KeepAlive option, this one will keep it alive and only close by
  // age
};

std::ostream& operator<<(std::ostream& os, ExpirationPolicy policy);

class PoolOptions {
 public:
  // Doing these the long way as we want to avoid depending on C++14 or FB's
  // TimeLiterals.h for Open Source.
  static constexpr Duration kDefaultMaxAge = std::chrono::seconds(60);
  static constexpr Duration kDefaultMaxIdleTime = std::chrono::seconds(4);
  static constexpr std::chrono::milliseconds kCleanUpTimeout =
      std::chrono::milliseconds(300);
  static const int kDefaultMaxOpenConn = 100;

  PoolOptions()
      : per_key_limit_(kDefaultMaxOpenConn),
        pool_limit_(kDefaultMaxOpenConn * 100),
        idle_timeout_(kDefaultMaxIdleTime),
        age_timeout_(kDefaultMaxAge),
        exp_policy_(ExpirationPolicy::Age),
        pool_per_instance_{false} {}

  PoolOptions& setPerKeyLimit(int conn_limit) {
    per_key_limit_ = conn_limit;
    return *this;
  }
  PoolOptions& setPoolLimit(int total_limit) {
    pool_limit_ = total_limit;
    return *this;
  }
  PoolOptions& setIdleTimeout(Duration idle_timeout) {
    idle_timeout_ = idle_timeout;
    return *this;
  }
  PoolOptions& setAgeTimeout(Duration age_timeout) {
    age_timeout_ = age_timeout;
    return *this;
  }
  PoolOptions& setExpPolicy(ExpirationPolicy exp_policy) {
    exp_policy_ = exp_policy;
    return *this;
  }
  // If pooling per instance is chosen, then the db name will be ignored
  // for the purposes of connection pooling. The user will be responsible
  // for ensuring they are connected to the correct database. This is useful
  // for instances with many databases
  PoolOptions& setPoolPerMysqlInstance(bool poolPerInstance) {
    pool_per_instance_ = poolPerInstance;
    return *this;
  }

  uint64_t getPerKeyLimit() const {
    return per_key_limit_;
  }
  uint64_t getPoolLimit() const {
    return pool_limit_;
  }
  Duration getIdleTimeout() const {
    return idle_timeout_;
  }
  Duration getAgeTimeout() const {
    return age_timeout_;
  }
  ExpirationPolicy getExpPolicy() const {
    return exp_policy_;
  }
  bool poolPerMysqlInstance() const {
    return pool_per_instance_;
  }

  bool operator==(const PoolOptions& other) const {
    return per_key_limit_ == other.per_key_limit_ &&
        pool_limit_ == other.pool_limit_ &&
        idle_timeout_ == other.idle_timeout_ &&
        // Note: we ignore age_timeout if using the IdleTime expiration policy
        (exp_policy_ == ExpirationPolicy::IdleTime ||
         age_timeout_ == other.age_timeout_) &&
        exp_policy_ == other.exp_policy_ &&
        pool_per_instance_ == other.pool_per_instance_;
  }
  bool operator!=(const PoolOptions& other) const {
    return !(operator==(other));
  }

 private:
  uint64_t per_key_limit_;
  uint64_t pool_limit_;
  Duration idle_timeout_;
  Duration age_timeout_;
  ExpirationPolicy exp_policy_;
  bool pool_per_instance_;
};

std::ostream& operator<<(std::ostream& os, const PoolOptions& options);

class PoolKey {
 public:
  // Hashes Connections and Operations waiting for connections based on basic
  // Connection info (ConnectionKey) and Connection Attributes.
  PoolKey(ConnectionKey conn_key, ConnectionOptions conn_opts)
      : connKey(std::move(conn_key)), connOptions(std::move(conn_opts)) {
    options_hash_ = folly::hash::hash_range(
        connOptions.getAttributes().begin(), connOptions.getAttributes().end());
    partial_hash_ =
        folly::hash::hash_combine(connKey.partial_hash, options_hash_);
    full_hash_ = folly::hash::hash_combine(connKey.hash, options_hash_);
  }

  bool operator==(const PoolKey& rhs) const {
    return full_hash_ == rhs.full_hash_ && options_hash_ == rhs.options_hash_ &&
        connKey == rhs.connKey;
  }

  bool operator!=(const PoolKey& rhs) const {
    return !(*this == rhs);
  }

  bool partialCompare(const PoolKey& rhs) const {
    return partial_hash_ == rhs.partial_hash_ &&
        options_hash_ == rhs.options_hash_ && connKey.partialEqual(rhs.connKey);
  }

  const ConnectionKey* getConnectionKey() const {
    return &connKey;
  }

  const ConnectionKey connKey;
  const ConnectionOptions connOptions;

  size_t getHash() const {
    return full_hash_;
  }

  size_t getPartialHash() const {
    return partial_hash_;
  }

  size_t getOptionsHash() const {
    return options_hash_;
  }

 private:
  size_t options_hash_;
  size_t full_hash_;
  size_t partial_hash_;
};

struct PoolKeyStats {
  size_t open_connections;
  size_t pending_connections;
  size_t connection_limit;
};

std::ostream& operator<<(std::ostream& os, PoolKey key);

class PoolKeyHash {
 public:
  size_t operator()(const PoolKey& k) const {
    return k.getHash();
  }
};

class PoolKeyPartialHash {
 public:
  size_t operator()(const PoolKey& k) const {
    return k.getPartialHash();
  }

  bool operator()(const PoolKey& lhs, const PoolKey& rhs) const {
    return lhs.partialCompare(rhs);
  }
};

class MysqlPooledHolder : public MysqlConnectionHolder {
 public:
  // Constructed based on an already existing MysqlConnectionHolder, the
  // values are going to be copied and the old holder will be destroyed.
  MysqlPooledHolder(
      std::unique_ptr<MysqlConnectionHolder> holder_base,
      std::weak_ptr<AsyncConnectionPool> weak_pool,
      const PoolKey& pool_key);

  ~MysqlPooledHolder() override;

  void setLifeDuration(Duration dur) {
    good_for_ = dur;
  }

  Duration getLifeDuration() {
    return good_for_;
  }

  void setOwnerPool(std::weak_ptr<AsyncConnectionPool> pool);

  const PoolKey& getPoolKey() const {
    return pool_key_;
  }

  std::shared_ptr<AsyncConnectionPool> getPoolPtr() const {
    return weak_pool_.lock();
  }

 private:
  void removeFromPool();

  Duration good_for_;
  std::weak_ptr<AsyncConnectionPool> weak_pool_;

  const PoolKey pool_key_;
};

typedef std::list<std::unique_ptr<MysqlPooledHolder>> MysqlConnectionList;
typedef std::list<std::weak_ptr<ConnectPoolOperation>> PoolOpList;

//  This pool manages and creates mysql connections asynchronous using async
//  client and its event thread. Multiple pools can use the same client.
//  The pool MUST always be acquired by using `makePool`.
//
// How a connection is acquired:
//   First `beginConnection` is called as we always do with the client. Using
// the pool will only change the type of the returned Operation,
// `ConnectPoolOperation`. Different from ConnectOperation, instead of opening
// a new connection on `run()`, the pool operation will call
// `registerForConnection`. The pool will then check if there are any spare
// connections available to return or create a new one (based on the limits).
// In the event of not having any available connection later scenario, the
// operation will be queued until a connection is ready for it.
// With the connection in hands, the pool will assign it to the operation
// by calling `connectionCallback`. The last call will work like a
// `socketActionable` call triggering the complete operation procedures.
//   For the user the usage is the same as acquiring a connection using
// `AsyncMysqlClient`, but now using the pool.
class AsyncConnectionPool
    : public std::enable_shared_from_this<AsyncConnectionPool> {
 public:
  // Don't use std::chrono::duration::MAX to avoid overflows
  static std::shared_ptr<AsyncConnectionPool> makePool(
      std::shared_ptr<AsyncMysqlClient> mysql_client,
      const PoolOptions& pool_options = PoolOptions());

  // The destructor will start the shutdown phase
  ~AsyncConnectionPool();

  FOLLY_NODISCARD folly::SemiFuture<ConnectResult> connectSemiFuture(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const ConnectionOptions& conn_opts = ConnectionOptions());

  FOLLY_NODISCARD folly::SemiFuture<ConnectResult> connectSemiFuture(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const std::string& special_tag,
      const ConnectionOptions& conn_opts = ConnectionOptions());

  folly::Future<ConnectResult> connectFuture(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const ConnectionOptions& conn_opts = ConnectionOptions());

  folly::Future<ConnectResult> connectFuture(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const std::string& special_tag,
      const ConnectionOptions& conn_opts = ConnectionOptions());

  std::unique_ptr<Connection> connect(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const ConnectionOptions& conn_opts = ConnectionOptions());

  // Returns a ConnectPoolOperation that will abstract the wait for the client
  // to find or create a connection for the operation.
  // In shutting down mode, this will return a cancelled operation
  // (same as the client).
  std::shared_ptr<ConnectOperation> beginConnection(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const std::string& special_tag = "");

  // Returns a ConnectPoolOperation from a ConnKey
  std::shared_ptr<ConnectOperation> beginConnection(
      const ConnectionKey& conn_key);

  // Returns the client that this pool is using
  std::shared_ptr<AsyncMysqlClient> getMysqlClient() {
    return mysql_client_;
  }

  // It will clean the pool and block any new connections or operations
  // Shutting down phase:
  // The remaining connections or operations that are linked to this pool
  // will know (using their weak_pointer to this pool) that the pool is dead
  // and proceed without the pool.
  void shutdown();

  db::PoolStats* stats() noexcept {
    return &pool_stats_;
  }

  const db::PoolStats* stats() const noexcept {
    return &pool_stats_;
  }

  PoolKeyStats getPoolKeyStats(const PoolKey& key) const;

  uint32_t getNumOpenConnections() const noexcept {
    return counters_.rlock()->num_open_connections;
  }

  uint32_t getNumPendingConnections() const noexcept {
    return counters_.rlock()->num_pending_connections;
  }

  // Don't use the constructor directly, only public to use make_shared
  AsyncConnectionPool(
      std::shared_ptr<AsyncMysqlClient> mysql_client,
      const PoolOptions& pool_options);

  // For debugging
  void displayOpenConnections();
  int getNumKeysInOpenConnections();
  int getNumKeysInPendingConnections();

  FOLLY_NODISCARD size_t perKeyLimit() const noexcept {
    return conn_per_key_limit_;
  }

  FOLLY_NODISCARD size_t totalLimit() const noexcept {
    return pool_conn_limit_;
  }

  FOLLY_NODISCARD Duration ageTimeout() const noexcept {
    return connection_age_timeout_;
  }

  FOLLY_NODISCARD Duration idleTimeout() const noexcept {
    return conn_storage_.maxIdleTime();
  }

  FOLLY_NODISCARD ExpirationPolicy expirationPolicy() const noexcept {
    return expiration_policy_;
  }

  FOLLY_NODISCARD bool poolPerMysqlInstance() const noexcept {
    return pool_per_instance_;
  }

  // for debugging, return number of pool keys in the pool
  FOLLY_NODISCARD size_t getNumKey() const noexcept {
    return conn_storage_.getNumKey();
  }

 private:
  friend class Connection;
  friend class MysqlPooledHolder;
  friend class Operation;
  friend class ConnectPoolOperation;
  friend class FetchOperation;

  std::weak_ptr<AsyncConnectionPool> getSelfWeakPointer();

  // Caches the connection in case it's marked as reusable (default). If the
  // connection is in a transaction or the user marked as not reusable, then
  // we close it.
  void recycleMysqlConnection(std::unique_ptr<MysqlConnectionHolder> mysqlConn);

  // Used by ConnectPoolOperation to register that this operation needs a
  // connection.
  // If there is a connection available, it will schedule a callback call in
  // the client thread. If the pool has no connections available, it will try
  // to create more depending on the amount of already open connection. In
  // case there is a chance for poolOp receive a connection, it will be queued
  // in the pool.
  // TODO#4527126: no chance it will be able to get a connection soon, so fail
  // fast.
  void registerForConnection(ConnectPoolOperation* poolOp);

  // Used internally when want a new connection. It checks if we should open
  // more connections, if so it creates ConnectOperation and once the
  // operation is completed the callback will call `addConnection`.
  // If the expiration policy is age, here we set the life limit of the
  // connection.
  // If we fail in creating the connection, `failedToConnect` will be called.
  // TODO#4527126: maybe have a cache for the unreachable hosts to fail faster
  // these connections.
  void tryRequestNewConnection(
      const PoolKey& pool_key,
      std::shared_ptr<db::ConnectionContextBase> context = nullptr);

  // Used for when we fail to open a requested connection. In case of mysql
  // failure (e.g. bad password) we propagate the error to all queued
  // ConnectPoolOperation's.
  void failedToConnect(const PoolKey& pool_key, ConnectOperation& conn_op);

  // Anytime a connection is supposed to be added to the pool, being fresh or
  // recycled, we check if there is an operation in wait list for the
  // ConnectionKey inside MysqlPooledHolder, if there is, the match is made
  // here. Otherwise it is added to the pool.
  // At this point, the connection should already have been cleaned.
  // Should only be used internally
  void addConnection(
      std::unique_ptr<MysqlPooledHolder> mysqlConn,
      bool brand_new);

  // Checks if the limits (global, connections open or being open by pool, or
  // limit per key) can fit one more connection. As a final check, checks if
  // it's a waste to create a new connection to avoid start opening a new
  // connection when we already have enough being open for the demand in
  // queue.
  bool canCreateMoreConnections(const PoolKey& conn_key) const;

  ///////////// Counter control functions

  // Note that unlike the AsyncMysqlClient this similar counter is for open
  // connections only, the intent of opening a connect is controlled
  // separately.
  void addOpenConnection(const PoolKey& conn_key);
  void addOpeningConn(const PoolKey& conn_key);

  void removeOpenConnection(const PoolKey& conn_key);
  void removeOpeningConn(const PoolKey& conn_key);

  void connectionSpotFreed(const PoolKey& conn_key);

  void openNewConnection(
      ConnectPoolOperation* rawPoolOp,
      const PoolKey& poolKey);

  void resetConnection(
      ConnectPoolOperation* rawPoolOp,
      const PoolKey& poolKey,
      std::unique_ptr<MysqlPooledHolder> mysqlConn);

  void reuseConnWithChangeUser(
      ConnectPoolOperation* rawPoolOp,
      const PoolKey& poolKey,
      std::unique_ptr<MysqlPooledHolder> mysqlConn);

  // Auxiliary class to isolate the queue code. Clean ups also happen in this
  // class, it mainly manages the ConnectPoolOperation and
  // MysqlPooledHolder containers.
  class ConnStorage {
   public:
    // At most this number of keys will be compared in the level2 cache to find
    // a key having the largest number of connections
    const int kMaxKeysToCheckInLevel2 = 3;
    explicit ConnStorage(
        std::thread::id allowed_threadid,
        size_t conn_limit,
        Duration max_idle_time)
        : allowed_thread_id_(allowed_threadid),
          conn_limit_(conn_limit),
          max_idle_time_(max_idle_time) {}

    ~ConnStorage() {}

    // Returns an shared pointer of the oldest valid operation in the queue
    // for the given PoolKey. The returned operation is removed from the
    // queue. We return shared to avoid the instance dying (for any reason)
    // before the connection is given to it.
    std::shared_ptr<ConnectPoolOperation> popOperation(const PoolKey& pool_key);

    // Puts the new operation in the end of the list
    void queueOperation(
        const PoolKey& pool_key,
        std::shared_ptr<ConnectPoolOperation>& poolOp);

    // Calls failureCallback with the error description and removed all
    // the operations for conn_key from the queue.
    void failOperations(
        const PoolKey& pool_key,
        OperationResult op_result,
        unsigned int mysql_errno,
        const std::string& mysql_error);

    // Returns a connection for the given ConnectionKey. The connection will
    // be removed from the queue. Depending on the policy, it will give the
    // oldest inserted connection (fifo) or the most recent inserted (lifo).
    std::unique_ptr<MysqlPooledHolder> popConnection(const PoolKey& pool_key);

    // Returns a connection for the given ConnectionKey, ignoring db name.
    // In order to reuse this connection, we need to run COM_CHANGE_USER.
    std::unique_ptr<MysqlPooledHolder> popInstanceConnection(
        const PoolKey& pool_key);

    // Puts the new connection in the back of the list.
    void queueConnection(std::unique_ptr<MysqlPooledHolder> newConn);

    // Checks and removes the connection that reached their idle time or age
    // limit.
    void cleanupConnections();

    // Checks and removes the weak ptrs that already expired, so we have a
    // better approximation of the number of operations really waiting.
    void cleanupOperations();

    // Cancels all operations that are still in the queue and clears all the
    // storage
    void clearAll();

    // for debugging
    void displayPoolStatus();

    size_t numQueuedOperations(const PoolKey& pool_key) const {
      DCHECK_EQ(std::this_thread::get_id(), allowed_thread_id_);
      auto it = waitList_.find(pool_key);
      return it == waitList_.end() ? 0 : it->second.size();
    }

    FOLLY_NODISCARD Duration maxIdleTime() const noexcept {
      return max_idle_time_;
    }

    // for debugging, return number of pool keys in the pool
    size_t getNumKey() const {
      return stock_.level1NumKey();
    }

   private:
    // We keep a copy to check that all manipulation is coming from the
    // right thread.
    std::thread::id allowed_thread_id_;

    // This pool holds weak_ptr to the operation in wait list to avoid holding
    // async client in the draining process in case the operation has already
    // been discarded by the creator before got a connection. This also serves
    // to avoid giving them connections
    TwoLevelCache<
        PoolKey,
        std::unique_ptr<MysqlPooledHolder>,
        PoolKeyHash,
        PoolKeyPartialHash>
        stock_;

    folly::F14NodeMap<PoolKey, PoolOpList, PoolKeyHash> waitList_;

    size_t conn_limit_;
    Duration max_idle_time_;
  } conn_storage_;

  class CleanUpTimer : public folly::AsyncTimeout {
   public:
    explicit CleanUpTimer(folly::EventBase* base, ConnStorage* pool);
    void timeoutExpired() noexcept override;

   private:
    ConnStorage* pool_;
  } cleanup_timer_;

  std::shared_ptr<AsyncMysqlClient> mysql_client_;

  // per ConnectionKey
  const size_t conn_per_key_limit_;
  // Limit the total of open connections (or being opened)
  const size_t pool_conn_limit_;
  Duration connection_age_timeout_;
  ExpirationPolicy expiration_policy_;
  const bool pool_per_instance_;

  struct Counters {
    uint32_t num_open_connections = 0;
    // Counts the number of open connections for a given connectionKey
    folly::F14FastMap<PoolKey, uint64_t, PoolKeyHash> open_connections;
    // Same as above but for connections that we are still opening
    // This one doesn't need locking, only accessed by client thread
    uint32_t num_pending_connections = 0;
    folly::F14FastMap<PoolKey, uint64_t, PoolKeyHash> pending_connections;
  };

  folly::Synchronized<Counters> counters_;

  // To allow us to pass weak_ptr for the PoolOperation`s
  std::weak_ptr<AsyncConnectionPool> self_pointer_;

  // Counters for connections created, cache hits and misses, etc.
  db::PoolStats pool_stats_;

  struct ShutdownData {
    bool shutting_down = false;
    bool finished_shutdown = false;
  };

  folly::Synchronized<ShutdownData> shutdown_data_;

  bool isShuttingDown() const {
    return shutdown_data_.rlock()->shutting_down;
  }

  AsyncConnectionPool(const AsyncConnectionPool&) = delete;
  AsyncConnectionPool& operator=(const AsyncConnectionPool&) = delete;
};

class ConnectPoolOperation : public ConnectOperation {
 public:
  ~ConnectPoolOperation() override {
    cancelPreOperation();
  }

  // Don't call this; it's public strictly for AsyncConnectionPool to be
  // able to call make_shared.
  ConnectPoolOperation(
      std::weak_ptr<AsyncConnectionPool> pool,
      std::shared_ptr<AsyncMysqlClient> client,
      ConnectionKey conn_key)
      : ConnectOperation(client.get(), std::move(conn_key)), pool_(pool) {}

  db::OperationType getOperationType() const override {
    return db::OperationType::PoolConnect;
  }

  bool setPreOperation(std::shared_ptr<Operation> op) {
    return preOperation_.wlock()->set(std::move(op));
  }

  void cancelPreOperation() {
    preOperation_.wlock()->cancel();
  }

  void resetPreOperation() {
    preOperation_.wlock()->reset();
  }

 protected:
  void attemptFailed(OperationResult result) override;

  ConnectPoolOperation* specializedRun() override;
  void specializedTimeoutTriggered() override;
  void socketActionable() override;

 private:
  void specializedRunImpl();
  // Called when the connection is matched by the pool client
  void connectionCallback(std::unique_ptr<MysqlPooledHolder> mysql_conn);
  // Called when the connection that the pool is trying to acquire failed
  void failureCallback(
      OperationResult failure,
      unsigned int mysql_errno,
      const std::string& mysql_error);

  std::weak_ptr<AsyncConnectionPool> pool_;

  // PreOperation keeps any other operation that needs to be canceled when
  // ConnectPoolOperation is cancelled.
  // PreOperation is not reused and its lifetime is with ConnectPoolOperation.
  // PreOperation is not thread-safe, and ConnectPoolOperation is responsible
  // for adding a lock.
  class PreOperation {
   public:
    void cancel() {
      if (preOperation_) {
        preOperation_->cancel();
        preOperation_.reset();
      }
      canceled_ = true;
    }

    void reset() {
      preOperation_.reset();
    }

    // returns true if set pre-operation succeeds, false otherwise
    bool set(std::shared_ptr<Operation> op) {
      if (canceled_) {
        return false;
      }
      preOperation_ = std::move(op);
      return true;
    }

   private:
    std::shared_ptr<Operation> preOperation_;
    bool canceled_{false};
  };

  // Operation that is required before completing this operation, which could
  // be reset_connection or change_user operation. There's at most 1
  // pre-operation.
  folly::Synchronized<PreOperation> preOperation_;

  friend class AsyncConnectionPool;
};
} // namespace mysql_client
} // namespace common
} // namespace facebook

#endif // COMMON_ASYNC_CONNECTION_POOL_H
