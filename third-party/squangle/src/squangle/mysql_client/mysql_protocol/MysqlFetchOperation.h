/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/MultiQueryOperation.h"
#include "squangle/mysql_client/MultiQueryStreamOperation.h"
#include "squangle/mysql_client/QueryOperation.h"

namespace facebook::common::mysql_client::mysql_protocol {

// MysqlQueryOperation is a UNIFIED class that combines QueryOperation logic
// with MySQL protocol implementation using COMPOSITION.
//
// Key difference from legacy pattern:
// - Legacy: Caller creates MysqlFetchOperationImpl, passes to QueryOperation
// - Unified: MysqlQueryOperation creates its own impl internally
//
class MysqlQueryOperation : public QueryOperation {
 public:
  // Factory method that takes a Connection and handles wrapping internally
  static std::shared_ptr<MysqlQueryOperation> create(
      std::unique_ptr<Connection> conn,
      Query&& query,
      LoggingFuncsPtr logging_funcs = nullptr);

  // Factory method that takes a ConnectionProxy directly (for sync operations)
  static std::shared_ptr<MysqlQueryOperation> createWithProxy(
      std::unique_ptr<ConnectionProxy> conn_proxy,
      Query&& query,
      LoggingFuncsPtr logging_funcs = nullptr);

 private:
  MysqlQueryOperation(std::unique_ptr<FetchOperationImpl> impl, Query&& query);
};

// MysqlMultiQueryOperation is a UNIFIED class that combines MultiQueryOperation
// logic with MySQL protocol implementation using COMPOSITION.
//
class MysqlMultiQueryOperation : public MultiQueryOperation {
 public:
  // Factory method that takes a Connection and handles wrapping internally
  static std::shared_ptr<MysqlMultiQueryOperation> create(
      std::unique_ptr<Connection> conn,
      std::vector<Query>&& queries,
      LoggingFuncsPtr logging_funcs = nullptr);

  // Factory method that takes a ConnectionProxy directly (for sync operations)
  static std::shared_ptr<MysqlMultiQueryOperation> createWithProxy(
      std::unique_ptr<ConnectionProxy> conn_proxy,
      std::vector<Query>&& queries,
      LoggingFuncsPtr logging_funcs = nullptr);

 private:
  MysqlMultiQueryOperation(
      std::unique_ptr<FetchOperationImpl> impl,
      std::vector<Query>&& queries);
};

// MysqlMultiQueryStreamOperation is a UNIFIED class that combines
// MultiQueryStreamOperation logic with MySQL protocol implementation.
//
class MysqlMultiQueryStreamOperation : public MultiQueryStreamOperation {
 public:
  // Factory method that takes a ConnectionProxy directly (no wrapping needed)
  static std::shared_ptr<MysqlMultiQueryStreamOperation> create(
      std::unique_ptr<ConnectionProxy> conn,
      MultiQuery&& multi_query,
      LoggingFuncsPtr logging_funcs = nullptr);

 private:
  MysqlMultiQueryStreamOperation(
      std::unique_ptr<FetchOperationImpl> impl,
      MultiQuery&& multi_query);
};

} // namespace facebook::common::mysql_client::mysql_protocol
