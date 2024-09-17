/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/mysql_protocol/MysqlResult.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnection.h"
#include "squangle/mysql_client/mysql_protocol/MysqlRow.h"
#include "squangle/mysql_client/mysql_protocol/MysqlRowMetadata.h"

namespace facebook::common::mysql_client::mysql_protocol {

class MysqlRowMetadata;

size_t MysqlResult::numFields() const {
  auto ret = mysql_num_fields(res_.get());
  VLOG(4) << fmt::format(
      "mysql_num_fields({}) returned {}", (void*)res_.get(), ret);
  return ret;
}

MYSQL_FIELD* MysqlResult::fields() const {
  auto ret = mysql_fetch_fields(res_.get());
  VLOG(4) << fmt::format(
      "mysql_fetch_fields({}) returned {}", (void*)res_.get(), (void*)ret);
  return ret;
}

size_t MysqlResult::numRows() const {
  auto ret = mysql_num_rows(res_.get());
  VLOG(4) << fmt::format(
      "mysql_num_rows({}) returned {}", (void*)res_.get(), ret);
  return ret;
}

std::unique_ptr<InternalRowMetadata> MysqlResult::getRowMetadata() const {
  return std::make_unique<MysqlRowMetadata>(*this);
}

namespace {

InternalResult::FetchRowRet MysqlRowFactory(
    MYSQL_RES* result,
    MYSQL_ROW mysqlRow) {
  std::unique_ptr<InternalRow> row;
  if (mysqlRow) {
    auto* lengths = mysql_fetch_lengths(result);
    VLOG(4) << fmt::format(
        "mysql_fetch_lengths({}) returned {}", (void*)result, (void*)lengths);

    auto numFields = mysql_num_fields(result);
    VLOG(4) << fmt::format(
        "mysql_num_fields({}) returned {}", (void*)result, numFields);

    row = std::make_unique<MysqlRow>(mysqlRow, numFields, lengths);
  }

  return std::make_pair(DONE, std::move(row));
}

} // namespace

InternalResult::FetchRowRet SyncMysqlResult::fetchRow() const {
  auto mysqlRow = mysql_fetch_row(res_.get());
  VLOG(4) << fmt::format(
      "mysql_fetch_row({}) returned {}", (void*)res_.get(), (void*)mysqlRow);

  return MysqlRowFactory(res_.get(), mysqlRow);
}

InternalResult::FetchRowRet AsyncMysqlResult::fetchRow() const {
  std::unique_ptr<InternalRow> row;

  MYSQL_ROW mysqlRow;
  auto ret = mysql_fetch_row_nonblocking(res_.get(), &mysqlRow);
  VLOG(4) << fmt::format(
      "mysql_fetch_row_nonblocking({}) returned {}, MYSQL_ROW = {}",
      (void*)res_.get(),
      ret,
      (void*)mysqlRow);

  if (ret == NET_ASYNC_COMPLETE) {
    return MysqlRowFactory(res_.get(), mysqlRow);
  }

  return std::make_pair(MysqlConnection::toHandlerStatus(ret), std::move(row));
}

} // namespace facebook::common::mysql_client::mysql_protocol
