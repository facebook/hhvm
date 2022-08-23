/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/AsyncConnectionPool.h"
#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/FutureAdapter.h"

#include <memory>

#include <folly/MapUtil.h>
#include <folly/Memory.h>

#include <vector>

#include <mysql.h>
#include <chrono>

namespace facebook {
namespace common {
namespace mysql_client {

constexpr std::chrono::milliseconds PoolOptions::kCleanUpTimeout;
constexpr Duration PoolOptions::kDefaultMaxIdleTime;
constexpr Duration PoolOptions::kDefaultMaxAge;

MysqlPooledHolder::MysqlPooledHolder(
    std::unique_ptr<MysqlConnectionHolder> holder_base,
    std::weak_ptr<AsyncConnectionPool> weak_pool,
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

MysqlPooledHolder::~MysqlPooledHolder() {
  removeFromPool();
}

void MysqlPooledHolder::setOwnerPool(std::weak_ptr<AsyncConnectionPool> pool) {
  // In case this connection belonged to a pool before
  removeFromPool();
  weak_pool_ = pool;
  auto lock_pool = weak_pool_.lock();
  // Extra care here, checking if we changing it to nullptr
  if (lock_pool) {
    lock_pool->stats()->incrCreatedPoolConnections();
    lock_pool->addOpenConnection(pool_key_);
  }
}

void MysqlPooledHolder::removeFromPool() {
  auto lock_pool = weak_pool_.lock();
  if (lock_pool) {
    lock_pool->stats()->incrDestroyedPoolConnections();
    lock_pool->removeOpenConnection(pool_key_);
  }
}

std::shared_ptr<AsyncConnectionPool> AsyncConnectionPool::makePool(
    std::shared_ptr<AsyncMysqlClient> mysql_client,
    const PoolOptions& pool_options) {
  auto connectionPool =
      std::make_shared<AsyncConnectionPool>(mysql_client, pool_options);
  return connectionPool;
}

AsyncConnectionPool::AsyncConnectionPool(
    std::shared_ptr<AsyncMysqlClient> mysql_client,
    const PoolOptions& pool_options)
    : conn_storage_(
          mysql_client->threadId(),
          pool_options.getPoolLimit() * 2,
          pool_options.getIdleTimeout()),
      cleanup_timer_(mysql_client->getEventBase(), &conn_storage_),
      mysql_client_(mysql_client),
      conn_per_key_limit_(pool_options.getPerKeyLimit()),
      pool_conn_limit_(pool_options.getPoolLimit()),
      connection_age_timeout_(pool_options.getAgeTimeout()),
      expiration_policy_(pool_options.getExpPolicy()),
      pool_per_instance_(pool_options.poolPerMysqlInstance()),
      finished_shutdown_(false) {
  if (!mysql_client_->runInThread([this]() {
        cleanup_timer_.scheduleTimeout(PoolOptions::kCleanUpTimeout);
      })) {
    LOG(DFATAL) << "Unable to schedule timeout due Thrift event issue";
  }
}

AsyncConnectionPool::~AsyncConnectionPool() {
  VLOG(2) << "Connection pool dying";

  if (!finished_shutdown_.load(std::memory_order_acquire)) {
    shutdown();
  }

  VLOG(2) << "Connection pool shutdown completed";
}

void AsyncConnectionPool::shutdown() {
  VLOG(2) << "Shutting down";
  std::unique_lock<std::mutex> lock(shutdown_mutex_);
  // Will block adding anything to the pool
  shutting_down_ = true;

  // cancelTimeout can only be ran in the tevent thread
  if (std::this_thread::get_id() == mysql_client_->threadId()) {
    cleanup_timer_.cancelTimeout();
    conn_storage_.clearAll();
    finished_shutdown_.store(true, std::memory_order_relaxed);
    VLOG(1) << "Shutting down in tevent thread";
  } else {
    mysql_client_->runInThread([this]() {
      cleanup_timer_.cancelTimeout();
      conn_storage_.clearAll();
      // Reacquire lock
      std::unique_lock<std::mutex> shutdown_lock(shutdown_mutex_);
      finished_shutdown_.store(true, std::memory_order_relaxed);
      this->shutdown_condvar_.notify_one();
    });
    shutdown_condvar_.wait(lock, [this] {
      return finished_shutdown_.load(std::memory_order_acquire);
    });
  }
}

folly::SemiFuture<ConnectResult> AsyncConnectionPool::connectSemiFuture(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password,
    const ConnectionOptions& conn_opts) {
  return connectSemiFuture(
      host, port, database_name, user, password, "", conn_opts);
}

folly::SemiFuture<ConnectResult> AsyncConnectionPool::connectSemiFuture(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password,
    const std::string& special_tag,
    const ConnectionOptions& conn_opts) {
  auto op =
      beginConnection(host, port, database_name, user, password, special_tag);
  op->setConnectionOptions(conn_opts);
  return toSemiFuture(std::move(op));
}

folly::Future<ConnectResult> AsyncConnectionPool::connectFuture(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password,
    const ConnectionOptions& conn_opts) {
  return connectFuture(
      host, port, database_name, user, password, "", conn_opts);
}

folly::Future<ConnectResult> AsyncConnectionPool::connectFuture(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password,
    const std::string& special_tag,
    const ConnectionOptions& conn_opts) {
  return toFuture(connectSemiFuture(
      host, port, database_name, user, password, special_tag));
}

std::unique_ptr<Connection> AsyncConnectionPool::connect(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password,
    const ConnectionOptions& conn_opts) {
  auto op = beginConnection(host, port, database_name, user, password);
  op->setConnectionOptions(conn_opts);
  // This will throw (intended behaviour) in case the operation didn't succeed
  return blockingConnectHelper(std::move(op));
}

std::shared_ptr<ConnectOperation> AsyncConnectionPool::beginConnection(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password,
    const std::string& special_tag) {
  return beginConnection(ConnectionKey(
      host,
      port,
      database_name,
      user,
      password,
      special_tag,
      poolPerMysqlInstance()));
}

std::shared_ptr<ConnectOperation> AsyncConnectionPool::beginConnection(
    const ConnectionKey& conn_key) {
  std::shared_ptr<ConnectPoolOperation> ret;
  {
    std::unique_lock<std::mutex> lock(shutdown_mutex_);
    // Assigning here to read from pool safely
    ret = std::make_shared<ConnectPoolOperation>(
        getSelfWeakPointer(), mysql_client_, conn_key);
    if (shutting_down_) {
      LOG(ERROR)
          << "Attempt to start pool operation while pool is shutting down";
      ret->cancel();
    }
  }
  mysql_client_->addOperation(ret);
  return ret;
}

std::weak_ptr<AsyncConnectionPool> AsyncConnectionPool::getSelfWeakPointer() {
  if (self_pointer_.expired()) {
    self_pointer_ = shared_from_this();
  }
  return self_pointer_;
}

void AsyncConnectionPool::recycleMysqlConnection(
    std::unique_ptr<MysqlConnectionHolder> mysql_conn) {
  // this method can run by any thread where the Connection is dying
  {
    std::unique_lock<std::mutex> lock(shutdown_mutex_);
    if (shutting_down_) {
      return;
    }
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

  auto pool = getSelfWeakPointer();
  auto pmysql_conn = mysql_conn.release();
  bool scheduled = mysql_client_->runInThread([pool, pmysql_conn]() {
    std::unique_ptr<MysqlPooledHolder> mysql_connection(
        static_cast<MysqlPooledHolder*>(pmysql_conn));
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

void AsyncConnectionPool::openNewConnection(
    ConnectPoolOperation* rawPoolOp,
    const PoolKey& poolKey) {
  stats()->incrPoolMisses();
  // TODO: Check if we are jammed and fail fast

  // The client holds shared pointers for all active operations
  // this method is called by the `run()` in the operation, so it
  // should always exist in the client
  auto pool_op = std::dynamic_pointer_cast<ConnectPoolOperation>(
      rawPoolOp->getSharedPointer());
  // Sanity check
  DCHECK(pool_op != nullptr);
  conn_storage_.queueOperation(poolKey, pool_op);
  // Propagate the ConnectionContext from the incoming operation. These contexts
  // contain application specific logging that will be lost if not passed to
  // the new ConnectOperation that is spawned to fulfill the pool miss.
  // Propagating the context pointer ensures that both operations are logged
  // with the expected additional logging
  tryRequestNewConnection(poolKey, pool_op->connection_context_);
}

void AsyncConnectionPool::reuseConnWithChangeUser(
    ConnectPoolOperation* rawPoolOp,
    const PoolKey& poolKey,
    std::unique_ptr<MysqlPooledHolder> mysqlConn) {
  auto conn = std::make_unique<AsyncConnection>(
      getMysqlClient().get(),
      rawPoolOp->getConnectionKey(),
      std::move(mysqlConn));
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
        auto pooledConn = std::make_unique<MysqlPooledHolder>(
            std::move(newMysqlConn), poolPtr, poolKey);
        stats()->incrPoolHits();
        stats()->incrPoolHitsChangeUser();

        pooledConn->setReusable(true);
        rawPoolOp->connectionCallback(std::move(pooledConn));
      });

  // Failing to set pre-operation means rawPoolOp already fails (times out, etc)
  // and its pre-operation is already cancelled.
  if (!rawPoolOp->setPreOperation(changeUserOp)) {
    return;
  }
  getMysqlClient()->runInThread([changeUserOp = std::move(changeUserOp)]() {
    changeUserOp->connection()->client()->addOperation(changeUserOp);
    changeUserOp->run();
  });
}

void AsyncConnectionPool::resetConnection(
    ConnectPoolOperation* rawPoolOp,
    const PoolKey& poolKey,
    std::unique_ptr<MysqlPooledHolder> mysqlConn) {
  auto conn = std::make_unique<AsyncConnection>(
      getMysqlClient().get(),
      rawPoolOp->getConnectionKey(),
      std::move(mysqlConn));
  conn->needToCloneConnection_ = false;
  auto resetOp = Connection::resetConn(std::move(conn));

  resetOp->setCallback(
      [this, rawPoolOp, poolKey](SpecialOperation& op, OperationResult result) {
        rawPoolOp->resetPreOperation();

        if (result == OperationResult::Failed) {
          openNewConnection(rawPoolOp, poolKey);
          return;
        }

        auto connection = op.releaseConnection();
        auto mysqlConnHolder = connection->stealMysqlConnectionHolder(true);
        auto pmysqlConn = mysqlConnHolder.release();
        std::unique_ptr<MysqlPooledHolder> mysqlConnection(
            static_cast<MysqlPooledHolder*>(pmysqlConn));

        stats()->incrPoolHits();
        mysqlConnection->setReusable(true);
        rawPoolOp->connectionCallback(std::move(mysqlConnection));
      });

  // Failing to set pre-operation means rawPoolOp already fails
  if (!rawPoolOp->setPreOperation(resetOp)) {
    return;
  }
  getMysqlClient()->runInThread([resetOp = std::move(resetOp)]() {
    resetOp->connection()->client()->addOperation(resetOp);
    resetOp->run();
  });
}

void AsyncConnectionPool::registerForConnection(
    ConnectPoolOperation* raw_pool_op) {
  // Runs only in main thread by run() in the ConnectPoolOperation
  DCHECK_EQ(std::this_thread::get_id(), mysql_client_->threadId());
  {
    std::unique_lock<std::mutex> lock(shutdown_mutex_);
    if (shutting_down_) {
      VLOG(4) << "Pool is shutting down, operation being canceled";
      raw_pool_op->cancel();
      return;
    }
  }
  stats()->incrConnectionsRequested();
  // Pass that to pool
  auto pool_key = PoolKey(
      raw_pool_op->getConnectionKey(), raw_pool_op->getConnectionOptions());

  std::unique_ptr<MysqlPooledHolder> mysql_conn =
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

bool AsyncConnectionPool::canCreateMoreConnections(
    const PoolKey& pool_key) const {
  DCHECK_EQ(std::this_thread::get_id(), mysql_client_->threadId());
  auto enqueued_pool_ops = conn_storage_.numQueuedOperations(pool_key);

  auto client_total_conns = mysql_client_->numStartedAndOpenConnections();
  auto client_conn_limit = mysql_client_->getPoolsConnectionLimit();

  // We have the number of connections we are opening and the number of already
  // open, we shouldn't try to create over this sum
  auto [num_pool_allocated, open_conns, pending_conns] =
      counters_.withRLock([&](const auto& locked) {
        return std::make_tuple(
            locked.num_open_connections + locked.num_pending_connections,
            folly::get_default(locked.open_connections, pool_key, 0),
            folly::get_default(locked.pending_connections, pool_key, 0));
      });
  int num_per_key_allocated = open_conns + pending_conns;

  // First we check global limit, then limits of the pool. If we can create more
  // connections, we check if we need comparing the amount of already being
  // opened connections for that key with the number of enqueued operations (the
  // operation that is requesting a new connection should be enqueued at this
  // point.
  if (client_total_conns < client_conn_limit &&
      num_pool_allocated < pool_conn_limit_ &&
      num_per_key_allocated < conn_per_key_limit_ &&
      pending_conns < enqueued_pool_ops) {
    return true;
  }
  return false;
}

PoolKeyStats AsyncConnectionPool::getPoolKeyStats(
    const PoolKey& pool_key) const {
  PoolKeyStats stats;
  stats.connection_limit = conn_per_key_limit_;
  counters_.withRLock([&](const auto& locked) {
    stats.open_connections =
        folly::get_default(locked.open_connections, pool_key, 0);
    stats.pending_connections =
        folly::get_default(locked.pending_connections, pool_key, 0);
  });
  return stats;
}

void AsyncConnectionPool::addOpenConnection(const PoolKey& pool_key) {
  counters_.withWLock([&](auto& locked) {
    ++locked.open_connections[pool_key];
    ++locked.num_open_connections;
  });
}

void AsyncConnectionPool::removeOpenConnection(const PoolKey& pool_key) {
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

void AsyncConnectionPool::displayOpenConnections() {
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

int AsyncConnectionPool::getNumKeysInOpenConnections() {
  return counters_.rlock()->open_connections.size();
}

int AsyncConnectionPool::getNumKeysInPendingConnections() {
  return counters_.rlock()->pending_connections.size();
}

void AsyncConnectionPool::addOpeningConn(const PoolKey& pool_key) {
  counters_.withWLock([&](auto& locked) {
    ++locked.pending_connections[pool_key];
    ++locked.num_pending_connections;
  });
}

void AsyncConnectionPool::removeOpeningConn(const PoolKey& pool_key) {
  counters_.withWLock([&](auto& locked) {
    auto num = --locked.pending_connections[pool_key];
    DCHECK_GE(int64_t(num), 0);
    if (num == 0) {
      locked.pending_connections.erase(pool_key);
    }
    --locked.num_pending_connections;
  });
}

void AsyncConnectionPool::connectionSpotFreed(const PoolKey& pool_key) {
  // Now we check if we should create more connections in case there are queued
  // operations in need
  auto weak_pool = getSelfWeakPointer();
  mysql_client_->runInThread([weak_pool, pool_key]() {
    auto pool = weak_pool.lock();
    if (pool) {
      pool->tryRequestNewConnection(pool_key);
    }
  });
}

void AsyncConnectionPool::tryRequestNewConnection(
    const PoolKey& pool_key,
    std::shared_ptr<db::ConnectionContextBase> context) {
  // Only called internally, this doesn't need to check if it's shutting
  // down
  DCHECK_EQ(std::this_thread::get_id(), mysql_client_->threadId());
  {
    std::unique_lock<std::mutex> lock(shutdown_mutex_);
    if (shutting_down_) {
      return;
    }
  }

  // Checking if limits allow creating more connections
  if (canCreateMoreConnections(pool_key)) {
    VLOG(11) << "Requesting new Connection";
    // get a shared pointer for operation

    auto connOp = mysql_client_->beginConnection(pool_key.connKey);
    connOp->setConnectionOptions(pool_key.connOptions);
    if (!context) {
      context = std::make_shared<db::ConnectionContextBase>();
    }
    connOp->setConnectionContext(std::move(context));
    auto pool_ptr = getSelfWeakPointer();

    // ADRIANA The attribute part we can do later :D time to do it
    connOp->setCallback([pool_key, pool_ptr](ConnectOperation& connOp) {
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
      auto pooled_conn = std::make_unique<MysqlPooledHolder>(
          std::move(mysql_conn), pool_ptr, pool_key);
      locked_pool->removeOpeningConn(pool_key);
      locked_pool->addConnection(std::move(pooled_conn), true);
    });

    try {
      addOpeningConn(pool_key);
      connOp->run();
    } catch (db::OperationStateException& e) {
      LOG(ERROR) << "Client is drain or dying, cannot ask for more connections";
    }
  }
}

void AsyncConnectionPool::failedToConnect(
    const PoolKey& pool_key,
    ConnectOperation& conn_op) {
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

// Shall be called anytime a fresh connection is ready or a recycled
void AsyncConnectionPool::addConnection(
    std::unique_ptr<MysqlPooledHolder> mysql_conn,
    bool brand_new) {
  // Only called internally, this doesn't need to check if it's shutting
  // down
  DCHECK_EQ(std::this_thread::get_id(), mysql_client_->threadId());
  if (brand_new) {
    if (expiration_policy_ == ExpirationPolicy::Age) {
      // TODO add noise to expiration age
      mysql_conn->setLifeDuration(connection_age_timeout_);
    }
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

AsyncConnectionPool::CleanUpTimer::CleanUpTimer(
    folly::EventBase* base,
    ConnStorage* pool)
    : folly::AsyncTimeout(base), pool_(pool) {}

void AsyncConnectionPool::CleanUpTimer::timeoutExpired() noexcept {
  scheduleTimeout(PoolOptions::kCleanUpTimeout);
  pool_->cleanupOperations();
  pool_->cleanupConnections();
}

std::shared_ptr<ConnectPoolOperation>
AsyncConnectionPool::ConnStorage::popOperation(const PoolKey& pool_key) {
  DCHECK_EQ(std::this_thread::get_id(), allowed_thread_id_);

  PoolOpList& list = waitList_[pool_key];
  while (!list.empty()) {
    std::weak_ptr<ConnectPoolOperation> weak_op = list.front();
    list.pop_front();
    auto ret = weak_op.lock();
    if (ret && !ret->done()) {
      return ret;
    }
  }

  return nullptr;
}

void AsyncConnectionPool::ConnStorage::queueOperation(
    const PoolKey& pool_key,
    std::shared_ptr<ConnectPoolOperation>& pool_op) {
  DCHECK_EQ(std::this_thread::get_id(), allowed_thread_id_);

  PoolOpList& list = waitList_[pool_key];
  std::weak_ptr<ConnectPoolOperation> weak_op = pool_op;
  list.push_back(std::move(weak_op));
}

void AsyncConnectionPool::ConnStorage::failOperations(
    const PoolKey& pool_key,
    OperationResult op_result,
    unsigned int mysql_errno,
    const std::string& mysql_error) {
  DCHECK_EQ(std::this_thread::get_id(), allowed_thread_id_);

  PoolOpList& list = waitList_[pool_key];
  while (!list.empty()) {
    std::weak_ptr<ConnectPoolOperation> weak_op = list.front();
    list.pop_front();
    auto lock_op = weak_op.lock();
    if (lock_op && !lock_op->done()) {
      lock_op->failureCallback(op_result, mysql_errno, mysql_error);
    }
  }
}

std::unique_ptr<MysqlPooledHolder>
AsyncConnectionPool::ConnStorage::popConnection(const PoolKey& pool_key) {
  DCHECK_EQ(std::this_thread::get_id(), allowed_thread_id_);
  return stock_.popLevel1(pool_key);
}

std::unique_ptr<MysqlPooledHolder>
AsyncConnectionPool::ConnStorage::popInstanceConnection(const PoolKey& key) {
  DCHECK_EQ(std::this_thread::get_id(), allowed_thread_id_);
  size_t tries = 0;
  size_t bestSize = 0;
  // Find a pool key from level2_ cache, which has the largest number of
  // connections in level1_ cache. This function checks at most
  // "kMaxKeysToCheckInLevel2" number of keys in level2_ cache.
  return stock_.popLevel2(
      key,
      [&](const auto& key1, const auto& key2) mutable -> folly::Optional<bool> {
        tries++;
        if (tries == kMaxKeysToCheckInLevel2) {
          return folly::none;
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

void AsyncConnectionPool::ConnStorage::queueConnection(
    std::unique_ptr<MysqlPooledHolder> newConn) {
  DCHECK_EQ(std::this_thread::get_id(), allowed_thread_id_);

  const auto& key = newConn->getPoolKey();
  stock_.push(key, std::move(newConn), conn_limit_);
}

void AsyncConnectionPool::ConnStorage::cleanupConnections() {
  DCHECK_EQ(std::this_thread::get_id(), allowed_thread_id_);

  Timepoint now = std::chrono::steady_clock::now();
  stock_.cleanup([&](const auto& conn) {
    return (conn->getLifeDuration() != Duration::zero() &&
            (conn->getCreationTime() + conn->getLifeDuration() < now)) ||
        conn->getLastActivityTime() + max_idle_time_ < now;
  });
}

void AsyncConnectionPool::ConnStorage::cleanupOperations() {
  DCHECK_EQ(std::this_thread::get_id(), allowed_thread_id_);

  for (auto poolOpListIt = waitList_.begin();
       poolOpListIt != waitList_.end();) {
    auto& poolOpList = poolOpListIt->second;
    for (PoolOpList::iterator it = poolOpList.begin();
         it != poolOpList.end();) {
      // check if weak pointer expired
      auto op = (*it).lock();
      if (!op || op->done()) {
        it = poolOpList.erase(it);
        VLOG(11) << "Operation being erased during clean up";
      } else {
        ++it;
      }
    }
    if (poolOpList.empty()) {
      poolOpListIt = waitList_.erase(poolOpListIt);
    } else {
      ++poolOpListIt;
    }
  }
}

void AsyncConnectionPool::ConnStorage::clearAll() {
  DCHECK_EQ(std::this_thread::get_id(), allowed_thread_id_);

  // Clearing all operations in the queue
  for (auto& poolOpListIt : waitList_) {
    auto& poolOpList = poolOpListIt.second;
    for (PoolOpList::iterator it = poolOpList.begin(); it != poolOpList.end();
         ++it) {
      // check if weak pointer expired
      auto locked_op = (*it).lock();
      if (locked_op) {
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

void AsyncConnectionPool::ConnStorage::displayPoolStatus() {
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

void ConnectPoolOperation::attemptFailed(OperationResult result) {
  ++attempts_made_;
  if (shouldCompleteOperation(result)) {
    completeOperation(result);
    return;
  }

  conn()->socketHandler()->unregisterHandler();
  conn()->socketHandler()->cancelTimeout();

  auto now = std::chrono::steady_clock::now();
  // Adjust timeout
  auto timeout_attempt_based = getConnectionOptions().getTimeout() +
      std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_);
  timeout_ =
      min(timeout_attempt_based, getConnectionOptions().getTotalTimeout());

  specializedRun();
}

ConnectPoolOperation* ConnectPoolOperation::specializedRun() {
  std::weak_ptr<Operation> weakSelf = getSharedPointer();
  if (!client()->runInThread([weakSelf]() {
        // There is a race confition that allows a cancelled or completed
        // operation getting here. The self ptr check ensures that the client
        // has not freed the reference to the operation, and the state() check
        // verifies whether other relevant memebers have been cleaned up by
        // connect callbacks
        auto self =
            std::static_pointer_cast<ConnectPoolOperation>(weakSelf.lock());
        if (!self || (self->state() == OperationState::Completed)) {
          LOG(ERROR) << "ConnectPoolOperation freed before running";
          return;
        }

        self->specializedRunImpl();
      })) {
    completeOperationInner(OperationResult::Failed);
  }
  return this;
}

void ConnectPoolOperation::specializedRunImpl() {
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

  conn()->socketHandler()->scheduleTimeout(
      std::chrono::duration_cast<std::chrono::milliseconds>(end - now).count());

  auto shared_pool = pool_.lock();
  // Remove before to not count against itself
  removeClientReference();
  if (shared_pool) {
    // Sync attributes in conn_options_ with the Operation::attributes_ value
    // as pool key uses the attributes from ConnectionOptions
    conn_options_.setAttributes(attributes_);
    shared_pool->registerForConnection(this);
  } else {
    VLOG(2) << "Pool is gone, operation must cancel";
    cancel();
  }
}

void ConnectPoolOperation::specializedTimeoutTriggered() {
  auto locked_pool = pool_.lock();
  if (locked_pool) {
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
      auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start_time_);
      auto cbDelayUs = client()->callbackDelayMicrosAvg();
      bool stalled = (cbDelayUs >= kCallbackDelayStallThresholdUs);

      std::vector<std::string> parts;
      parts.push_back(fmt::format(
          "[{}]({})Connection to {}:{} timed out in pool",
          static_cast<uint16_t>(SquangleErrno::SQ_ERRNO_POOL_CONN_TIMEOUT),
          kErrorPrefix,
          host().c_str(),
          port()));
      parts.push_back(fmt::format(
          "(open {}, opening {}, key limit {})",
          num_open,
          num_opening,
          locked_pool->conn_per_key_limit_));
      parts.push_back(timeoutMessage(delta));
      if (stalled) {
        parts.push_back(threadOverloadMessage(cbDelayUs));
      }
      setAsyncClientError(
          static_cast<uint16_t>(SquangleErrno::SQ_ERRNO_POOL_CONN_TIMEOUT),
          folly::join(" ", parts),
          "Connection timed out in pool");
      attemptFailed(OperationResult::TimedOut);
      return;
    }
  }

  ConnectOperation::timeoutHandler(false, true);
}

void ConnectPoolOperation::connectionCallback(
    std::unique_ptr<MysqlPooledHolder> mysql_conn) {
  DCHECK(client()->getEventBase()->isInEventBaseThread());

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
  auto pool = pool_;
  conn()->setConnectionDyingCallback(
      [pool](std::unique_ptr<MysqlConnectionHolder> mysql_conn) {
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
}

void ConnectPoolOperation::failureCallback(
    OperationResult failure,
    unsigned int mysql_errno,
    const std::string& mysql_error) {
  mysql_errno_ = mysql_errno;
  mysql_error_ = mysql_error;
  attemptFailed(failure);
}

void ConnectPoolOperation::socketActionable() {
  DCHECK(client()->getEventBase()->isInEventBaseThread());
  LOG(DFATAL) << "Should not be called";
}

std::ostream& operator<<(std::ostream& os, ExpirationPolicy policy) {
  auto str = (policy == ExpirationPolicy::Age)
      ? "Age"
      : (policy == ExpirationPolicy::IdleTime ? "IdleTime" : "<invalid>");
  return os << str;
}

std::ostream& operator<<(std::ostream& os, const PoolOptions& options) {
  return os << "{per key limit:" << options.getPerKeyLimit()
            << ",pool limit:" << options.getPoolLimit()
            << ",idle timeout:" << options.getIdleTimeout().count()
            << "us,age timeout:" << options.getAgeTimeout().count()
            << "us,expiration policy:" << options.getExpPolicy()
            << ",pool per instance:" << options.poolPerMysqlInstance() << "}";
}

std::ostream& operator<<(std::ostream& os, PoolKey key) {
  return os << "{key:" << key.connKey.getDisplayString()
            << ",options:" << key.connOptions.getDisplayString() << "}";
}

} // namespace mysql_client
} // namespace common
} // namespace facebook
