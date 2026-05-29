/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/Operation.h"

namespace facebook::common::mysql_client {

class ConnectionProxy;
class SpecialOperation;

using SpecialOperationCallback =
    std::function<void(SpecialOperation&, OperationResult)>;

class SpecialOperationImpl : virtual public OperationBase {
 public:
  explicit SpecialOperationImpl(db::OperationType operation_type)
      : operation_type_(operation_type) {}
  virtual ~SpecialOperationImpl() override = default;

  // Returns the operation type (Reset, ChangeUser, etc.)
  db::OperationType getOperationType() const override {
    return operation_type_;
  }

  void setCallback(SpecialOperationCallback callback) {
    callback_ = std::move(callback);
  }

  InternalConnection::Status runSpecialOperation();

  // Expose the internal connection for derived Operation classes
  InternalConnection& internalConnection() {
    return getInternalConnection();
  }

 protected:
  [[nodiscard]] SpecialOperation& getOp() const;

  SpecialOperationCallback callback_{nullptr};

 private:
  db::OperationType operation_type_;
};

// SpecialOperation means operations like COM_RESET_CONNECTION,
// COM_CHANGE_USER, etc.
class SpecialOperation : public Operation {
 public:
  void setCallback(SpecialOperationCallback callback) {
    impl_->setCallback(std::move(callback));
  }

 protected:
  explicit SpecialOperation(std::unique_ptr<SpecialOperationImpl> impl)
      : impl_(std::move(impl)) {
    if (!impl_) {
      throw std::runtime_error("SpecialOperationImpl is null");
    }

    impl_->setOperation(*this);
  }

  virtual InternalConnection::Status runSpecialOperation() = 0;
  friend SpecialOperationImpl;

  // Protected accessor for derived classes to access the impl
  SpecialOperationImpl* specialOperationImpl() {
    return impl_.get();
  }

 private:
  virtual const char* getErrorMsg() const = 0;

  virtual OperationBase* impl() override {
    return static_cast<OperationBase*>(impl_.get());
  }
  virtual const OperationBase* impl() const override {
    return static_cast<const OperationBase*>(impl_.get());
  }

  std::unique_ptr<SpecialOperationImpl> impl_;
};

} // namespace facebook::common::mysql_client
