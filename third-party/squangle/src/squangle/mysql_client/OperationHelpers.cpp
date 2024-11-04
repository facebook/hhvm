/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/OperationHelpers.h"

namespace facebook::common::mysql_client {

namespace {

void copyRowToRowBlock(RowBlock* block, const EphemeralRow& eph_row) {
  block->startRow();
  for (int i = 0; i < eph_row.numFields(); ++i) {
    switch (eph_row.getType(i)) {
      case InternalRow::Type::Null:
        block->appendNull();
        break;
      case InternalRow::Type::Bool:
        block->appendValue(eph_row.getBool(i));
        break;

      case InternalRow::Type::Int64:
        block->appendValue(eph_row.getInt64(i));
        break;

      case InternalRow::Type::UInt64:
        block->appendValue(eph_row.getUInt64(i));
        break;

      case InternalRow::Type::Double:
        block->appendValue(eph_row.getDouble(i));
        break;

      case InternalRow::Type::String:
        block->appendValue(eph_row.getString(i));
        break;
    }
  }
  block->finishRow();
}

} // namespace

RowBlock makeRowBlockFromStream(
    std::shared_ptr<RowFields> row_fields,
    RowStream* row_stream) {
  RowBlock row_block(std::move(row_fields));
  // Consume row_stream
  while (row_stream->hasNext()) {
    auto eph_row = row_stream->consumeRow();
    copyRowToRowBlock(&row_block, eph_row);
  }
  return row_block;
}

} // namespace facebook::common::mysql_client
