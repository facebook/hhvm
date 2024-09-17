/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/InternalConnection.h"

namespace facebook::common::mysql_client::mysql_protocol {

class MysqlResult : public InternalResult {
 public:
  explicit MysqlResult(MYSQL_RES* res) : res_(res) {}

  [[nodiscard]] size_t numFields() const;

  [[nodiscard]] MYSQL_FIELD* fields() const;

  [[nodiscard]] size_t numRows() const override;

  void close() override {
    res_->handle = nullptr;
  }

 protected:
  friend class MysqlFetchOperationImpl;

  using MysqlResultDeleter =
      folly::static_function_deleter<MYSQL_RES, mysql_free_result>;
  using MysqlResultPtr = std::unique_ptr<MYSQL_RES, MysqlResultDeleter>;

  [[nodiscard]] std::unique_ptr<InternalRowMetadata> getRowMetadata() const;

  MysqlResultPtr res_;
};

class AsyncMysqlResult : public MysqlResult {
 public:
  explicit AsyncMysqlResult(MYSQL_RES* res) : MysqlResult(res) {}

  [[nodiscard]] FetchRowRet fetchRow() override;
};

class SyncMysqlResult : public MysqlResult {
 public:
  explicit SyncMysqlResult(MYSQL_RES* res) : MysqlResult(res) {}

  [[nodiscard]] FetchRowRet fetchRow() override;
};

} // namespace facebook::common::mysql_client::mysql_protocol
