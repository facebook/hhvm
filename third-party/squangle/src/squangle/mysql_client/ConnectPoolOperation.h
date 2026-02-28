/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <boost/polymorphic_cast.hpp>
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
class ConnectPoolOperationImpl : virtual public ConnectOperationImpl {
 public:
  virtual ~ConnectPoolOperationImpl() override = default;

  virtual void prepWait() = 0;
  virtual bool syncWait() = 0;
  virtual void cleanupWait() = 0;

  virtual void attemptFailed(OperationResult result) = 0;
  virtual void connectionCallback(
      std::unique_ptr<MysqlPooledHolder<Client>> mysql_conn) = 0;
};

template <typename Client>
class ConnectPoolOperation : public ConnectOperation {
 public:
  explicit ConnectPoolOperation(
      std::unique_ptr<ConnectPoolOperationImpl<Client>> impl)
      : ConnectOperation(std::move(impl)) {}

  ~ConnectPoolOperation() override {
    cancelPreOperation();
  }

  ConnectPoolOperation(const ConnectPoolOperation&) = delete;
  ConnectPoolOperation& operator=(const ConnectPoolOperation&) = delete;

  ConnectPoolOperation(ConnectPoolOperation&&) = delete;
  ConnectPoolOperation& operator=(ConnectPoolOperation&&) = delete;

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
    return boost::polymorphic_cast<ConnectPoolOperationImpl<Client>*>(
        ConnectOperation::impl());
  }
  const ConnectPoolOperationImpl<Client>* impl() const override {
    return boost::polymorphic_cast<const ConnectPoolOperationImpl<Client>*>(
        ConnectOperation::impl());
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
