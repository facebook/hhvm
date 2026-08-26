/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/OperationHelpers.h"

#include "squangle/util/StorageRow.h"

namespace facebook::common::mysql_client {

namespace {

void copyRowToRowBlock(RowBlock* block, const EphemeralRow& eph_row) {
  // Build the row whole, then hand it over: a row that does not match the
  // block's column count is rejected at a single point with the block left
  // untouched, rather than partway through the loop.
  StorageRow row(eph_row.numFields());
  for (int i = 0; i < eph_row.numFields(); ++i) {
    switch (eph_row.getType(i)) {
      case InternalRow::Type::Null:
        row.appendNull();
        break;
      case InternalRow::Type::Bool:
        row.appendValue(eph_row.getBool(i));
        break;

      case InternalRow::Type::Int64:
        row.appendValue(eph_row.getInt64(i));
        break;

      case InternalRow::Type::UInt64:
        row.appendValue(eph_row.getUInt64(i));
        break;

      case InternalRow::Type::Double:
        row.appendValue(eph_row.getDouble(i));
        break;

      case InternalRow::Type::String:
        row.appendValue(eph_row.getString(i));
        break;
    }
  }
  block->addRow(std::move(row));
}

} // namespace

RowBlock makeRowBlockFromStream(
    std::shared_ptr<RowFields> row_fields,
    RowStream* row_stream) {
  RowBlock row_block(std::move(row_fields));
  try {
    // Consume row_stream
    while (row_stream->hasNext()) {
      auto eph_row = row_stream->consumeRow();
      copyRowToRowBlock(&row_block, eph_row);
    }
  } catch (const std::exception& ex) {
    // Retag as MalformedResultError so the fetch loop can tell a bad result
    // set apart from an exception thrown by a consumer callback. The
    // underlying throws are std::out_of_range (index/capacity) and
    // std::logic_error (row lifecycle) from Row.h.
    throw MalformedResultError(ex.what());
  }
  return row_block;
}

} // namespace facebook::common::mysql_client
