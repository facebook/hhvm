/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/synchronization/Baton.h>
#include <list>
#include <memory>

#include "squangle/base/Base.h"
#include "squangle/base/ConnectionKey.h"
#include "squangle/logger/DBEventCounter.h"
#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/MysqlConnectionHolder.h"
#include "squangle/mysql_client/Operation.h"
#include "squangle/mysql_client/PoolKey.h"
#include "squangle/mysql_client/PoolStorage.h"

namespace facebook::common::mysql_client {

template <typename Client>
class ConnectionPool;
template <typename Client>
class ConnectPoolOperation;

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

template <typename Client>
class MysqlPooledHolder : public MysqlConnectionHolder {
 public:
  // Constructed based on an already existing MysqlConnectionHolder, the
  // values are going to be copied and the old holder will be destroyed.
  MysqlPooledHolder(
      std::unique_ptr<MysqlConnectionHolder> holder_base,
      std::weak_ptr<ConnectionPool<Client>> weak_pool,
      const PoolKey& pool_key)
      : MysqlConnectionHolder(
            std::move(holder_base),
            pool_key.getConnectionKey()),
        good_for_(Duration::zero()),
        weak_pool_(weak_pool),
        pool_key_(pool_key) {
    auto lock_pool = weak_pool.lock();
    if (lock_pool) {
      lock_pool->stats()->incrCreatedPoolConnections();
      lock_pool->addOpenConnection(pool_key_);
    }
  }

  virtual ~MysqlPooledHolder() override {
    removeFromPool();
  }

  void setLifeDuration(Duration dur) {
    good_for_ = dur;
  }

  Duration getLifeDuration() {
    return good_for_;
  }

  void setOwnerPool(std::weak_ptr<ConnectionPool<Client>> pool) {
    // In case this connection belonged to a pool before
    removeFromPool();
    weak_pool_ = std::move(pool);
    // Extra care here, checking if we changing it to nullptr
    if (auto lock_pool = weak_pool_.lock(); lock_pool) {
      lock_pool->stats()->incrCreatedPoolConnections();
      lock_pool->addOpenConnection(pool_key_);
    }
  }

  const PoolKey& getPoolKey() const {
    return pool_key_;
  }

  std::shared_ptr<ConnectionPool<Client>> getPoolPtr() const {
    return weak_pool_.lock();
  }

 private:
  void removeFromPool() {
    if (auto lock_pool = weak_pool_.lock(); lock_pool) {
      lock_pool->stats()->incrDestroyedPoolConnections();
      lock_pool->removeOpenConnection(pool_key_);
    }
  }

  Duration good_for_;
  std::weak_ptr<ConnectionPool<Client>> weak_pool_;

  const PoolKey pool_key_;
};

class ConnectionPoolBase {
 public:
  explicit ConnectionPoolBase(PoolOptions pool_options)
      : pool_options_(pool_options) {}

  virtual ~ConnectionPoolBase() {}

  using ShouldThrottleCallback = std::function<
      bool(const PoolKey&, std::shared_ptr<db::ConnectionContextBase> context)>;

  db::PoolStats* stats() noexcept {
    return &pool_stats_;
  }

  const db::PoolStats* stats() const noexcept {
    return &pool_stats_;
  }

  PoolKeyStats getPoolKeyStats(const PoolKey& key) const;

  FOLLY_NODISCARD size_t perKeyLimit() const noexcept {
    return pool_options_.getPerKeyLimit();
  }

  FOLLY_NODISCARD size_t totalLimit() const noexcept {
    return pool_options_.getPoolLimit();
  }

  FOLLY_NODISCARD Duration ageTimeout() const noexcept {
    return pool_options_.getAgeTimeout();
  }

  FOLLY_NODISCARD Duration idleTimeout() const noexcept {
    return pool_options_.getIdleTimeout();
  }

  FOLLY_NODISCARD ExpirationPolicy expirationPolicy() const noexcept {
    return pool_options_.getExpPolicy();
  }

  FOLLY_NODISCARD bool poolPerMysqlInstance() const noexcept {
    return pool_options_.poolPerMysqlInstance();
  }

  // Note that unlike the AsyncMysqlClient this similar counter is for open
  // connections only, the intent of opening a connect is controlled
  // separately.
  void addOpenConnection(const PoolKey& conn_key);
  bool tryAddOpeningConn(
      const PoolKey& conn_key,
      std::shared_ptr<db::ConnectionContextBase> context,
      size_t enqueued_pool_ops,
      uint32_t client_total_conns,
      uint64_t client_conn_limit);

  void removeOpenConnection(const PoolKey& conn_key);
  void removeOpeningConn(const PoolKey& conn_key);

  // For debugging
  void displayOpenConnections();
  int getNumKeysInOpenConnections();
  int getNumKeysInPendingConnections();

  uint32_t getNumOpenConnections() const noexcept {
    return counters_.rlock()->num_open_connections;
  }

  uint32_t getNumPendingConnections() const noexcept {
    return counters_.rlock()->num_pending_connections;
  }

  static constexpr bool implementsPooling() {
    return true;
  }

  void setShouldThrottleCallback(ShouldThrottleCallback cb) {
    shouldThrottleCallback_ = std::move(cb);
  }

 protected:
  virtual bool isShuttingDown() const = 0;

  virtual void connectionSpotFreed(const PoolKey& conn_key) = 0;

  struct Counters {
    uint32_t num_open_connections = 0;
    // Counts the number of open connections for a given connectionKey
    folly::F14FastMap<PoolKey, uint64_t, PoolKeyHash> open_connections;
    // Same as above but for connections that we are still opening
    // This one doesn't need locking, only accessed by client thread
    uint32_t num_pending_connections = 0;
    folly::F14FastMap<PoolKey, uint64_t, PoolKeyHash> pending_connections;
  };

  bool canCreateMoreConnections(
      const PoolKey& pool_key,
      size_t enqueued_pool_ops,
      uint32_t client_total_conns,
      uint64_t client_conn_limit) const {
    return counters_.withRLock([&](const auto& counters) {
      return canCreateMoreConnections(
          pool_key,
          enqueued_pool_ops,
          client_total_conns,
          client_conn_limit,
          counters);
    });
  }

 private:
  template <typename Client>
  friend class MysqlPooledHolder;

  bool canCreateMoreConnections(
      const PoolKey& pool_key,
      size_t enqueued_pool_ops,
      uint32_t client_total_conns,
      uint64_t client_conn_limit,
      const Counters& counters) const;

  PoolOptions pool_options_;

  folly::Synchronized<Counters> counters_;

  ShouldThrottleCallback shouldThrottleCallback_;

  // Counters for connections created, cache hits and misses, etc.
  db::PoolStats pool_stats_;
};

//  This pool manages and creates mysql connections using the async or sync
//  client. When using the sync client the client's thread is used.  Multiple
//  pools can use the same client. The pool MUST always be acquired by using
//  `makePool`.
//
// How a connection is acquired:
//   First `beginConnection` is called as we always do with the client. Using
// the pool will only change the type of the returned Operation,
// `ConnectPoolOperation<Client>`. Different from ConnectOperation, instead of
// opening a new connection on `run()`, the pool operation will call
// `registerForConnection`. The pool will then check if there are any spare
// connections available to return or create a new one (based on the limits).
// In the event of not having any available connection later scenario, the
// operation will be queued until a connection is ready for it.
// With the connection in hands, the pool will assign it to the operation
// by calling `connectionCallback`. The last call will work like a
// `socketActionable` call triggering the complete operation procedures.
//   For the user the usage is the same as acquiring a connection using
// `AsyncMysqlClient` or `SyncMysqlClient`, but now using the pool.
template <typename Client>
class ConnectionPool
    : public ConnectionPoolBase,
      public std::enable_shared_from_this<ConnectionPool<Client>> {
 public:
  using ClientType = Client;

  ConnectionPool(std::shared_ptr<Client> mysql_client, PoolOptions pool_options)
      : ConnectionPoolBase(std::move(pool_options)),
        mysql_client_(std::move(mysql_client)),
        conn_storage_(totalLimit(), idleTimeout()) {}

  virtual ~ConnectionPool() override {}

  virtual void shutdown() = 0;

  // Returns the client that this pool is using
  std::shared_ptr<Client> getMysqlClient() {
    return mysql_client_;
  }

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
      const std::string& special_tag = "") {
    return beginConnection(ConnectionKey(
        host,
        port,
        database_name,
        user,
        password,
        special_tag,
        poolPerMysqlInstance()));
  }

  std::shared_ptr<ConnectOperation> beginConnection(
      const ConnectionKey& conn_key) {
    auto ret = std::make_shared<ConnectPoolOperation<Client>>(
        getSelfWeakPointer(), mysql_client_, conn_key);
    if (isShuttingDown()) {
      LOG(ERROR)
          << "Attempt to start pool operation while pool is shutting down";
      ret->cancel();
    }
    mysql_client_->addOperation(ret);
    return ret;
  }

  FOLLY_NODISCARD size_t getNumKey() const noexcept {
    return conn_storage_.getNumKey();
  }

 protected:
  // Checks if the limits (global, connections open or being open by pool, or
  // limit per key) can fit one more connection. As a final check, checks if
  // it's a waste to create a new connection to avoid start opening a new
  // connection when we already have enough being open for the demand in
  // queue.
  bool canCreateMoreConnections(const PoolKey& conn_key) const {
    validateCorrectThread();
    return ConnectionPoolBase::canCreateMoreConnections(
        conn_key,
        numQueuedOperations(conn_key),
        mysql_client_->numStartedAndOpenConnections(),
        mysql_client_->getPoolsConnectionLimit());
  }

  bool tryAddOpeningConn(
      const PoolKey& conn_key,
      std::shared_ptr<db::ConnectionContextBase> context) {
    return ConnectionPoolBase::tryAddOpeningConn(
        conn_key,
        context,
        numQueuedOperations(conn_key),
        mysql_client_->numStartedAndOpenConnections(),
        mysql_client_->getPoolsConnectionLimit());
  }

  void registerForConnection(ConnectPoolOperation<Client>* raw_pool_op) {
    validateCorrectThread();
    if (isShuttingDown()) {
      VLOG(4) << "Pool is shutting down, operation being canceled";
      raw_pool_op->cancel();
      return;
    }

    stats()->incrConnectionsRequested();
    // Pass that to pool
    auto pool_key = PoolKey(
        raw_pool_op->getConnectionKey(), raw_pool_op->getConnectionOptions());

    std::unique_ptr<MysqlPooledHolder<Client>> mysql_conn =
        conn_storage_.popConnection(pool_key);

    if (mysql_conn == nullptr) {
      // One more try before opening a new connection: finding a connection from
      // level2 cache and if found, run COM_CHANGE_USER
      if (raw_pool_op->getConnectionOptions().isEnableChangeUser()) {
        auto ret = conn_storage_.popInstanceConnection(pool_key);
        if (ret) { // found
          reuseConnWithChangeUser(raw_pool_op, pool_key, std::move(ret));
          return;
        }
      }
      openNewConnection(raw_pool_op, pool_key);
    } else if (mysql_conn->needResetBeforeReuse()) {
      // reset connection before reusing the connection
      resetConnection(raw_pool_op, pool_key, std::move(mysql_conn));
    } else {
      // Cache hit
      stats()->incrPoolHits();

      mysql_conn->setReusable(true);
      raw_pool_op->connectionCallback(std::move(mysql_conn));
    }
  }

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
      std::shared_ptr<db::ConnectionContextBase> context = nullptr) {
    validateCorrectThread();

    // Only called internally, this doesn't need to check if it's shutting
    // down
    if (isShuttingDown()) {
      return;
    }

    // Checking if limits allow creating more connections
    if (!tryAddOpeningConn(pool_key, context)) {
      return;
    }

    VLOG(11) << "Requesting new Connection";
    // get a shared pointer for operation

    auto connOp = mysql_client_->beginConnection(pool_key.connKey);
    connOp->setConnectionOptions(pool_key.connOptions);
    if (!context) {
      context = std::make_shared<db::ConnectionContextBase>();
    }
    connOp->setConnectionContext(std::move(context));

    // ADRIANA The attribute part we can do later :D time to do it
    connOp->setCallback(
        [pool_key, pool_ptr = getSelfWeakPointer()](ConnectOperation& connOp) {
          auto locked_pool = pool_ptr.lock();
          if (!locked_pool) {
            return;
          }
          if (!connOp.ok()) {
            VLOG(2) << "Failed to create new connection";
            locked_pool->removeOpeningConn(pool_key);
            locked_pool->failedToConnect(pool_key, connOp);
            return;
          }
          auto conn = connOp.releaseConnection();
          auto mysql_conn = conn->stealMysqlConnectionHolder();
          // Now we got a connection from the client, it will become a pooled
          // connection
          auto pooled_conn = std::make_unique<MysqlPooledHolder<Client>>(
              std::move(mysql_conn), pool_ptr, pool_key);
          locked_pool->removeOpeningConn(pool_key);
          locked_pool->addConnection(std::move(pooled_conn), true);
        });

    try {
      connOp->run();
    } catch (db::OperationStateException& e) {
      LOG(ERROR)
          << "Client is drain or dying, cannot ask for more connections: "
          << e.what();
    }
  }

  void reuseConnWithChangeUser(
      ConnectPoolOperation<Client>* rawPoolOp,
      const PoolKey& poolKey,
      std::unique_ptr<MysqlPooledHolder<Client>> mysqlConn) {
    auto conn =
        makeNewConnection(rawPoolOp->getConnectionKey(), std::move(mysqlConn));
    conn->needToCloneConnection_ = false;
    auto changeUserOp = Connection::changeUser(
        std::move(conn),
        poolKey.connKey.user,
        poolKey.connKey.password,
        poolKey.connKey.db_name);

    changeUserOp->setCallback(
        [this, rawPoolOp, poolKey, poolPtr = getSelfWeakPointer()](
            SpecialOperation& op, OperationResult result) {
          rawPoolOp->resetPreOperation();

          if (result == OperationResult::Failed) {
            openNewConnection(rawPoolOp, poolKey);
            return;
          }

          auto connection = op.releaseConnection();
          auto mysqlConn = connection->stealMysqlConnectionHolder(true);
          auto newMysqlConn = std::make_unique<MysqlConnectionHolder>(
              std::move(mysqlConn), poolKey.getConnectionKey());
          auto pooledConn = std::make_unique<MysqlPooledHolder<Client>>(
              std::move(newMysqlConn), poolPtr, poolKey);
          stats()->incrPoolHits();
          stats()->incrPoolHitsChangeUser();

          pooledConn->setReusable(true);
          rawPoolOp->connectionCallback(std::move(pooledConn));
        });

    // Failing to set pre-operation means rawPoolOp already fails (times out,
    // etc) and its pre-operation is already cancelled.
    if (!rawPoolOp->setPreOperation(changeUserOp)) {
      return;
    }

    // For async - run in mysql thread; for sync - run in current thread
    runInCorrectThread([changeUserOp = std::move(changeUserOp)] {
      changeUserOp->connection()->client()->addOperation(changeUserOp);
      changeUserOp->run();
    });
  }

  void connectionSpotFreed(const PoolKey& conn_key) override {
    runInCorrectThread([weak_pool = getSelfWeakPointer(), conn_key]() {
      auto pool = weak_pool.lock();
      if (pool) {
        pool->tryRequestNewConnection(conn_key);
      }
    });
  }

  void openNewConnection(
      ConnectPoolOperation<Client>* rawPoolOp,
      const PoolKey& poolKey) {
    stats()->incrPoolMisses();
    // TODO: Check if we are jammed and fail fast

    // The client holds shared pointers for all active operations
    // this method is called by the `run()` in the operation, so it
    // should always exist in the client
    auto pool_op = std::dynamic_pointer_cast<ConnectPoolOperation<Client>>(
        rawPoolOp->getSharedPointer());
    // Sanity check
    DCHECK(pool_op != nullptr);

    openNewConnectionPrep(*pool_op);

    conn_storage_.queueOperation(poolKey, pool_op);
    // Propagate the ConnectionContext from the incoming operation. These
    // contexts contain application specific logging that will be lost if not
    // passed to the new ConnectOperation that is spawned to fulfill the pool
    // miss. Propagating the context pointer ensures that both operations are
    // logged with the expected additional logging
    tryRequestNewConnection(poolKey, pool_op->connection_context_);

    openNewConnectionFinish(*pool_op, poolKey);
  }

  void resetConnection(
      ConnectPoolOperation<Client>* rawPoolOp,
      const PoolKey& poolKey,
      std::unique_ptr<MysqlPooledHolder<Client>> mysqlConn) {
    auto conn =
        makeNewConnection(rawPoolOp->getConnectionKey(), std::move(mysqlConn));
    conn->needToCloneConnection_ = false;
    auto resetOp = Connection::resetConn(std::move(conn));

    resetOp->setCallback([this, rawPoolOp, poolKey](
                             SpecialOperation& op, OperationResult result) {
      rawPoolOp->resetPreOperation();

      if (result == OperationResult::Failed) {
        openNewConnection(rawPoolOp, poolKey);
        return;
      }

      auto connection = op.releaseConnection();
      auto mysqlConnHolder = connection->stealMysqlConnectionHolder(true);
      auto pmysqlConn = mysqlConnHolder.release();
      std::unique_ptr<MysqlPooledHolder<Client>> mysqlConnection(
          static_cast<MysqlPooledHolder<Client>*>(pmysqlConn));

      stats()->incrPoolHits();
      mysqlConnection->setReusable(true);
      rawPoolOp->connectionCallback(std::move(mysqlConnection));
    });

    // Failing to set pre-operation means rawPoolOp already fails
    if (!rawPoolOp->setPreOperation(resetOp)) {
      return;
    }

    // For async - run in mysql thread; for sync - run in current thread
    runInCorrectThread([resetOp = std::move(resetOp)]() {
      resetOp->connection()->client()->addOperation(resetOp);
      resetOp->run();
    });
  }

 protected:
  std::weak_ptr<ConnectionPool<Client>> getSelfWeakPointer() {
    return this->weak_from_this();
  }

  virtual void validateCorrectThread() const = 0;

  virtual bool runInCorrectThread(
      std::function<void()>&& func,
      bool wait = false) = 0;

  size_t numQueuedOperations(const PoolKey& pool_key) const {
    return conn_storage_.numQueuedOperations(pool_key);
  }

  std::shared_ptr<Client> mysql_client_;

  PoolStorage<Client> conn_storage_;

 private:
  friend class ConnectPoolOperation<Client>;

  void recycleMysqlConnection(
      std::unique_ptr<MysqlConnectionHolder> mysql_conn) {
    // this method can run by any thread where the Connection is dying
    if (isShuttingDown()) {
      return;
    }

    VLOG(2) << "Trying to recycle connection";

    if (!mysql_conn->isReusable()) {
      return;
    }

    // Check server_status for in_transaction bit
    if (mysql_conn->inTransaction()) {
      // To avoid complication, we are just going to close the connection
      LOG_EVERY_N(INFO, 1000) << "Closing connection during a transaction."
                              << " Transaction will rollback.";
      return;
    }

    auto pmysql_conn = mysql_conn.release();
    bool scheduled =
        runInCorrectThread([pool = getSelfWeakPointer(), pmysql_conn]() {
          std::unique_ptr<MysqlPooledHolder<Client>> mysql_connection(
              static_cast<MysqlPooledHolder<Client>*>(pmysql_conn));
          auto shared_pool = pool.lock();
          if (!shared_pool) {
            return;
          }

          // in mysql 5.7 we can use mysql_reset_connection
          // We don't have a nonblocking version for reset connection, so we
          // are going to delete the old one and the open connection being
          // removed procedure is going to check if it needs to open new one
          shared_pool->addConnection(std::move(mysql_connection), false);
        });

    if (!scheduled) {
      delete pmysql_conn;
    }
  }

  // Anytime a connection is supposed to be added to the pool, being fresh or
  // recycled, we check if there is an operation in wait list for the
  // ConnectionKey inside MysqlPooledHolder, if there is, the match is made
  // here. Otherwise it is added to the pool.
  // At this point, the connection should already have been cleaned.
  // Should only be used internally
  void addConnection(
      std::unique_ptr<MysqlPooledHolder<Client>> mysql_conn,
      bool brand_new) {
    // Only called internally, this doesn't need to check if it's shutting
    // down
    validateCorrectThread();
    if (brand_new && expirationPolicy() == ExpirationPolicy::Age) {
      // TODO add noise to expiration age
      mysql_conn->setLifeDuration(ageTimeout());
    }

    VLOG(11) << "New connection ready to be used";
    auto pool_op = conn_storage_.popOperation(mysql_conn->getPoolKey());
    if (pool_op == nullptr) {
      VLOG(11) << "No operations waiting for Connection, enqueueing it";
      conn_storage_.queueConnection(std::move(mysql_conn));
    } else {
      mysql_conn->setReusable(true);
      pool_op->connectionCallback(std::move(mysql_conn));
    }
  }

  // Used for when we fail to open a requested connection. In case of mysql
  // failure (e.g. bad password) we propagate the error to all queued
  // AsyncConnectPoolOperation's.
  void failedToConnect(const PoolKey& pool_key, ConnectOperation& conn_op) {
    // Propagating ConnectOperation failure to queued operations in case
    // This will help us fail fast incorrect passwords or users.
    if (conn_op.result() == OperationResult::Failed) {
      conn_storage_.failOperations(
          pool_key,
          conn_op.result(),
          conn_op.mysql_errno(),
          conn_op.mysql_error());
    }
    connectionSpotFreed(pool_key);
  }

  virtual void openNewConnectionPrep(ConnectPoolOperation<Client>& pool_op) = 0;

  virtual void openNewConnectionFinish(
      ConnectPoolOperation<Client>& pool_op,
      const PoolKey& pool_key) = 0;

  virtual std::unique_ptr<Connection> makeNewConnection(
      const ConnectionKey& conn_key,
      std::unique_ptr<MysqlPooledHolder<Client>> mysqlConn) = 0;
};

template <typename Client>
class ConnectPoolOperation : public ConnectOperation {
 public:
  ~ConnectPoolOperation() override {
    cancelPreOperation();
  }

  // Don't call this; it's public strictly for ConnectionPool to be able to call
  // make_shared.
  ConnectPoolOperation(
      std::weak_ptr<ConnectionPool<Client>> pool,
      std::shared_ptr<Client> client,
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
  ConnectPoolOperation<Client>* specializedRun() override;

  void specializedTimeoutTriggered() override {
    if (auto locked_pool = pool_.lock(); locked_pool) {
      cancelPreOperation();

      // Check if the timeout happened because of the host is being slow or the
      // pool is lacking resources
      auto pool_key = PoolKey(getConnectionKey(), getConnectionOptions());
      auto key_stats = locked_pool->getPoolKeyStats(pool_key);
      auto num_open = key_stats.open_connections;
      auto num_opening = key_stats.pending_connections;

      // As a way to be realistic regarding the reason a connection was not
      // obtained, we start from the principle that this is pool's fault.
      // We can only blame the host (by forwarding 2013) if we have no
      // open connections and none trying to be open.
      // The second rule is applied where the resource restriction is so small
      // that the pool can't even try to open a connection.
      if (!(num_open == 0 &&
            (num_opening > 0 ||
             locked_pool->canCreateMoreConnections(pool_key)))) {
        setAsyncClientError(
            static_cast<uint16_t>(SquangleErrno::SQ_ERRNO_POOL_CONN_TIMEOUT),
            createTimeoutErrorMessage(key_stats, locked_pool->perKeyLimit()),
            "Connection timed out in pool");
        attemptFailed(OperationResult::TimedOut);
        return;
      }
    }

    ConnectOperation::timeoutHandler(false, true);
  }

  void attemptFailed(OperationResult result) override {
    ++attempts_made_;
    if (shouldCompleteOperation(result)) {
      completeOperation(result);
      return;
    }

    conn()->socketHandler()->unregisterHandler();
    conn()->socketHandler()->cancelTimeout();

    auto now = std::chrono::steady_clock::now();
    // Adjust timeout
    std::chrono::duration<uint64_t, std::micro> timeout_attempt_based =
        getConnectionOptions().getTimeout() +
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now - start_time_);

    timeout_ =
        min(timeout_attempt_based, getConnectionOptions().getTotalTimeout());

    specializedRun();
  }

 private:
  friend class ConnectionPool<Client>;
  friend class PoolStorageData<Client>;
  friend class AsyncConnectionPool;
  friend class SyncConnectionPool;

  void specializedRunImpl() {
    // Initialize all we need from our tevent handler
    if (attempts_made_ == 0) {
      conn()->initialize(false);
    }
    conn()->socketHandler()->setOperation(this);

    if (conn_options_.getSSLOptionsProviderPtr() && connection_context_) {
      connection_context_->isSslConnection = true;
    }

    // Set timeout for waiting for connection
    auto end = timeout_ + start_time_;
    auto now = std::chrono::steady_clock::now();
    if (now >= end) {
      timeoutTriggered();
      return;
    }

    if constexpr (uses_one_thread_v<Client>) {
      conn()->socketHandler()->scheduleTimeout(
          std::chrono::duration_cast<std::chrono::milliseconds>(end - now)
              .count());
    }

    // Remove before to not count against itself
    removeClientReference();

    if (auto shared_pool = pool_.lock(); shared_pool) {
      // Sync attributes in conn_options_ with the Operation::attributes_ value
      // as pool key uses the attributes from ConnectionOptions
      conn_options_.setAttributes(attributes_);
      shared_pool->registerForConnection(this);
    } else {
      VLOG(2) << "Pool is gone, operation must cancel";
      cancel();
    }
  }

  // Called when the connection is matched by the pool client
  void connectionCallback(
      std::unique_ptr<MysqlPooledHolder<Client>> mysql_conn) {
    // TODO: validate we are in the correct thread (for async)

    if (!mysql_conn) {
      LOG(DFATAL) << "Unexpected error";
      completeOperation(OperationResult::Failed);
      return;
    }
    if (mysql_errno_) {
      LOG_EVERY_N(ERROR, 1000)
          << "Connection pool callback was called with mysql err: "
          << mysql_errno_;
      completeOperation(OperationResult::Failed);
      return;
    }

    conn()->socketHandler()->changeHandlerFD(folly::NetworkSocket::fromFd(
        mysql_get_socket_descriptor(mysql_conn->mysql())));

    conn()->setMysqlConnectionHolder(std::move(mysql_conn));
    conn()->setConnectionOptions(getConnectionOptions());
    conn()->setConnectionDyingCallback(
        [pool = pool_](std::unique_ptr<MysqlConnectionHolder> mysql_conn) {
          auto shared_pool = pool.lock();
          if (shared_pool) {
            shared_pool->recycleMysqlConnection(std::move(mysql_conn));
          }
        });
    if (conn()->mysql()) {
      attemptSucceeded(OperationResult::Succeeded);
    } else {
      VLOG(2) << "Error: Failed to acquire connection";
      attemptFailed(OperationResult::Failed);
    }

    signalWaiter();
  }

  // Called when the connection that the pool is trying to acquire failed
  void failureCallback(
      OperationResult failure,
      unsigned int mysql_errno,
      const std::string& mysql_error) {
    mysql_errno_ = mysql_errno;
    mysql_error_ = mysql_error;
    attemptFailed(failure);
  }

  void socketActionable() override {
    DCHECK(client()->getEventBase()->isInEventBaseThread());
    LOG(DFATAL) << "Should not be called";
  }

  void prepWait() {
    baton_ = std::make_unique<folly::Baton<>>();
  }

  bool syncWait() {
    DCHECK(baton_);
    auto end = timeout_ + start_time_;
    return baton_->try_wait_until(end);
  }

  void cleanupWait() {
    baton_.reset();
  }

  void signalWaiter() {
    if (baton_) {
      baton_->post();
    }
  }

  std::string createTimeoutErrorMessage(
      const PoolKeyStats& pool_key_stats,
      size_t per_key_limit);

  std::weak_ptr<ConnectionPool<Client>> pool_;

  std::unique_ptr<folly::Baton<>> baton_;

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

} // namespace facebook::common::mysql_client
