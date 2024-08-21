/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/InternalConnection.h"

namespace facebook::common::mysql_client {

class MysqlRow : public InternalRow {
 public:
  MysqlRow(MYSQL_ROW row, size_t numCols, unsigned long* lengths)
      : row_(std::move(row)), numCols_(numCols), lengths_(lengths) {
    DCHECK(row_);
    DCHECK(numCols_);
    DCHECK(lengths_);
  }

  [[nodiscard]] bool isNull(size_t col) const override {
    DCHECK_LT(col, numCols_);
    return !row_[col];
  }

  [[nodiscard]] folly::StringPiece column(size_t col) const override {
    DCHECK_LT(col, numCols_);
    DCHECK(row_[col]);
    return folly::StringPiece(row_[col], lengths_[col]);
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

} // namespace facebook::common::mysql_client
