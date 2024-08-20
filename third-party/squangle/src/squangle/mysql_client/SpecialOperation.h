/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/MysqlHandler.h"
#include "squangle/mysql_client/Operation.h"

namespace facebook::common::mysql_client {

class ConnectionProxy;
class SpecialOperation;

using SpecialOperationCallback =
    std::function<void(SpecialOperation&, OperationResult)>;

// SpecialOperation means operations like COM_RESET_CONNECTION,
// COM_CHANGE_USER, etc.
class SpecialOperationImpl : public OperationImpl {
 public:
  explicit SpecialOperationImpl(std::unique_ptr<ConnectionProxy> conn)
      : OperationImpl(std::move(conn)) {}

  void setCallback(SpecialOperationCallback callback) {
    callback_ = std::move(callback);
  }

 protected:
  void actionable() override;
  void specializedCompleteOperation() override;
  void specializedTimeoutTriggered() override;
  void specializedRun() override;

 private:
  SpecialOperation& getOp() const;

  SpecialOperationCallback callback_{nullptr};
};

class SpecialOperation : public Operation {
 public:
  void setCallback(SpecialOperationCallback callback) {
    impl_->setCallback(std::move(callback));
  }

 protected:
  explicit SpecialOperation(std::unique_ptr<SpecialOperationImpl> impl)
      : impl_(std::move(impl)) {
    if (!impl_) {
      throw std::runtime_error("ConnectOperationImpl is null");
    }

    impl_->setOperation(*this);
  }

  void mustSucceed() override;

  virtual MysqlHandler::Status callMysqlHandler() = 0;
  friend SpecialOperationImpl;

 private:
  virtual const char* getErrorMsg() const = 0;

  virtual OperationBase* impl() override {
    return (OperationBase*)impl_.get();
  }
  virtual const OperationBase* impl() const override {
    return (OperationBase*)impl_.get();
  }

  std::unique_ptr<SpecialOperationImpl> impl_;
};

} // namespace facebook::common::mysql_client
