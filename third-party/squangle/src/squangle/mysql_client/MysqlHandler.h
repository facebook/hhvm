/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/base/ConnectionKey.h"
#include "squangle/mysql_client/InternalConnection.h"

namespace facebook::common::mysql_client {

class ConnectionOptions;

// MysqlHandler interface that is impletemented by the sync and async
// clients appropriately.
class MysqlHandler {
 public:
  using Status = InternalConnection::Status;
  virtual ~MysqlHandler() = default;
  virtual Status tryConnect(
      const InternalConnection& conn,
      const ConnectionOptions& opts,
      const ConnectionKey& key,
      int flags) = 0;
  virtual Status runQuery(
      const InternalConnection& conn,
      std::string_view query) = 0;
  virtual std::unique_ptr<InternalResult> getResult(
      const InternalConnection& conn) = 0;
  virtual size_t getFieldCount(const InternalConnection& conn) = 0;
  virtual Status nextResult(const InternalConnection& conn) = 0;
  virtual InternalResult::FetchRowRet fetchRow(InternalResult& res) = 0;
  virtual Status resetConn(const InternalConnection& conn) = 0;
  virtual Status changeUser(
      const InternalConnection& conn,
      const std::string& user,
      const std::string& password,
      const std::string& database) = 0;
};

} // namespace facebook::common::mysql_client
