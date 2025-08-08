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

class MysqlRow : public InternalRow {
 public:
  MysqlRow(MYSQL_ROW row, size_t numCols, unsigned long* lengths)
      : row_(std::move(row)), numCols_(numCols), lengths_(lengths) {
    DCHECK(row_);
    DCHECK(numCols_);
    DCHECK(lengths_);
  }

  [[nodiscard]] folly::StringPiece columnString(size_t col) const override {
    DCHECK_LT(col, numCols_);
    DCHECK(row_[col]);
    return folly::StringPiece(row_[col], lengths_[col]);
  }

  [[nodiscard]] bool columnBool(size_t /*col*/) const override {
    throw std::logic_error("columnBool not implemented for MysqlRow");
  }

  [[nodiscard]] int64_t columnInt64(size_t /*col*/) const override {
    throw std::logic_error("columnInt64 not implemented for MysqlRow");
  }

  [[nodiscard]] uint64_t columnUInt64(size_t /*col*/) const override {
    throw std::logic_error("columnUInt64 not implemented for MysqlRow");
  }

  [[nodiscard]] double columnDouble(size_t /*col*/) const override {
    throw std::logic_error("columnDouble not implemented for MysqlRow");
  }

  [[nodiscard]] InternalRow::Type columnType(size_t col) const override {
    DCHECK_LT(col, numCols_);
    if (!row_[col]) {
      return InternalRow::Type::Null;
    }

    // MySQL returns all data as strings - note this is NOT the column type -
    // just the type of the data returned via this protocol
    return InternalRow::Type::String;
  }

  [[nodiscard]] size_t columnLength(size_t col) const override {
    DCHECK_LT(col, numCols_);
    return lengths_[col];
  }

 private:
  MYSQL_ROW row_;
  size_t numCols_;
  unsigned long* lengths_;
};

} // namespace facebook::common::mysql_client::mysql_protocol
