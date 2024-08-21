/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/synchronization/Baton.h>

#include "squangle/mysql_client/ConnectOperation.h"
#include "squangle/mysql_client/PoolStorage.h"

namespace facebook::common::mysql_client {

template <typename Client>
class ConnectionPool;

template <typename Client>
class PoolStorageData;

template <typename Client>
class MysqlPooledHolder;

class AsyncConnectionPool;
class SyncConnectionPool;
class ConnectionHolder;
struct PoolKeyStats;

template <typename Client>
class ConnectPoolOperationImpl : public ConnectOperationImpl {
 public:
  // Don't call this; it's public strictly for ConnectionPool to be able to call
  // make_shared.
  ConnectPoolOperationImpl(
      std::weak_ptr<ConnectionPool<Client>> pool,
      std::shared_ptr<Client> client,
      std::shared_ptr<const ConnectionKey> conn_key)
      : ConnectOperationImpl(client.get(), std::move(conn_key)), pool_(pool) {}

  ConnectPoolOperation<Client>& getConnectPoolOp() const {
    return *dynamic_cast<ConnectPoolOperation<Client>*>(op_);
  }

 protected:
  void specializedRun() override;

  void specializedTimeoutTriggered() override {
    if (auto locked_pool = pool_.lock(); locked_pool) {
      auto& op = getConnectPoolOp();
      op.cancelPreOperation();

      // Check if the timeout happened because of the host is being slow or the
      // pool is lacking resources
      auto pool_key = PoolKey(op.getKey(), getConnectionOptions());
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
        op.setAsyncClientError(
            static_cast<uint16_t>(SquangleErrno::SQ_ERRNO_POOL_CONN_TIMEOUT),
            createTimeoutErrorMessage(key_stats, locked_pool->perKeyLimit()));
        attemptFailed(OperationResult::TimedOut);
        return;
      }
    }

    ConnectOperationImpl::timeoutHandler(false, true);
  }

  void attemptFailed(OperationResult result) override {
    ++attempts_made_;
    if (shouldCompleteOperation(result)) {
      completeOperation(result);
      return;
    }

    unregisterHandler();
    cancelTimeout();

    // Adjust timeout
    Duration timeout_attempt_based =
        getConnectionOptions().getTimeout() + opElapsedMs();

    OperationImpl::setTimeoutInternal(
        min(timeout_attempt_based, getConnectionOptions().getTotalTimeout()));

    specializedRun();
  }

 private:
  void specializedRunImpl() override {
    // Initialize all we need from our tevent handler
    if (attempts_made_ == 0) {
      conn().initialize(false);
    }

    withOptionalConnectionContext([&](auto& connection_context) {
      conn_options_.withPossibleSSLOptionsProvider(
          [&](const auto& /*provider*/) {
            connection_context.isSslConnection = true;
          });
    });

    // Set timeout for waiting for connection
    auto elapsed = OperationImpl::opElapsed();
    if (elapsed >= getTimeout()) {
      timeoutTriggered();
      return;
    }

    if constexpr (uses_one_thread_v<Client>) {
      scheduleTimeout(
          std::chrono::duration_cast<Millis>(getTimeout() - elapsed).count());
    }

    // Remove before to not count against itself
    removeClientReference();

    if (auto shared_pool = pool_.lock(); shared_pool) {
      // Sync attributes in conn_options_ with the Operation::attributes_ value
      // as pool key uses the attributes from ConnectionOptions
      conn_options_.setAttributes(getAttributes());
      shared_pool->registerForConnection(&getConnectPoolOp());
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

    if (mysql_errno()) {
      LOG_EVERY_N(ERROR, 1000)
          << "Connection pool callback was called with mysql err: "
          << mysql_errno();
      completeOperation(OperationResult::Failed);
      return;
    }

    changeHandlerFD(
        folly::NetworkSocket::fromFd(mysql_conn->getSocketDescriptor()));

    conn().setConnectionHolder(std::move(mysql_conn));
    conn().setConnectionOptions(getConnectionOptions());
    conn().setConnectionDyingCallback(
        [pool = pool_](std::unique_ptr<ConnectionHolder> mysql_conn) {
          auto shared_pool = pool.lock();
          if (shared_pool) {
            shared_pool->recycleMysqlConnection(std::move(mysql_conn));
          }
        });
    attemptSucceeded(OperationResult::Succeeded);

    signalWaiter();
  }

  void actionable() override {
    DCHECK(client().getEventBase()->isInEventBaseThread());
    LOG(DFATAL) << "Should not be called";
  }

  void prepWait() {
    baton_ = std::make_unique<folly::Baton<>>();
  }

  bool syncWait() {
    DCHECK(baton_);
    return baton_->try_wait_for(getTimeout() - opElapsed());
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

  friend class AsyncConnectionPool;
  friend class ConnectPoolOperation<Client>;
};

template <typename Client>
class ConnectPoolOperation : public ConnectOperation {
 public:
  explicit ConnectPoolOperation(std::unique_ptr<ConnectOperationImpl> impl)
      : ConnectOperation(std::move(impl)) {}

  ~ConnectPoolOperation() override {
    cancelPreOperation();
  }

  db::OperationType getOperationType() const override {
    return db::OperationType::PoolConnect;
  }

  void prepWait() {
    impl()->prepWait();
  }

  bool syncWait() {
    return impl()->syncWait();
  }

  void cleanupWait() {
    impl()->cleanupWait();
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
  ConnectPoolOperationImpl<Client>* impl() override {
    return (ConnectPoolOperationImpl<Client>*)ConnectOperation::impl();
  }
  const ConnectPoolOperationImpl<Client>* impl() const override {
    return (ConnectPoolOperationImpl<Client>*)ConnectOperation::impl();
  }

  void attemptFailed(OperationResult result) {
    return impl()->attemptFailed(result);
  }

 private:
  friend class ConnectionPool<Client>;
  friend class PoolStorageData<Client>;

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

  void connectionCallback(
      std::unique_ptr<MysqlPooledHolder<Client>> mysql_conn) {
    impl()->connectionCallback(std::move(mysql_conn));
  }

  // Called when the connection that the pool is trying to acquire failed
  void failureCallback(
      OperationResult failure,
      unsigned int mysql_errno,
      const std::string& mysql_error) {
    setAsyncClientError(mysql_errno, mysql_error);
    attemptFailed(failure);
  }

  void specializedRunImpl() {
    impl()->specializedRunImpl();
  }
};

} // namespace facebook::common::mysql_client
