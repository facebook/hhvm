/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef COMMON_ASYNC_MYSQL_HANDLER_H
#define COMMON_ASYNC_MYSQL_HANDLER_H

namespace facebook {
namespace common {
namespace mysql_client {

class ConnectionOptions;

// MysqlHandler interface that is impletemented by the sync and async
// clients appropriately.
class MysqlHandler {
 public:
  enum Status {
    PENDING,
    DONE,
    ERROR,
  };
  virtual ~MysqlHandler() = default;
  virtual Status tryConnect(
      MYSQL* mysql,
      const ConnectionOptions& opts,
      const ConnectionKey& key,
      int flags) = 0;
  virtual Status runQuery(MYSQL* mysql, folly::StringPiece queryStmt) = 0;
  virtual MYSQL_RES* getResult(MYSQL* mysql) = 0;
  virtual Status nextResult(MYSQL* mysql) = 0;
  virtual Status fetchRow(MYSQL_RES* res, MYSQL_ROW& row) = 0;
  virtual Status resetConn(MYSQL* mysql) = 0;
  virtual Status changeUser(
      MYSQL* mysql,
      const std::string& user,
      const std::string& password,
      const std::string& database) = 0;
};

} // namespace mysql_client
} // namespace common
} // namespace facebook

#endif // COMMON_ASYNC_MYSQL_HANDLER_H
