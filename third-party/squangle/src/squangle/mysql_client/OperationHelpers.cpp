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
    if (eph_row.isNull(i)) {
      block->appendNull();
    } else {
      block->appendValue(eph_row[i]);
    }
  }
  block->finishRow();
}

} // namespace

RowBlock makeRowBlockFromStream(
    std::shared_ptr<RowFields> row_fields,
    FetchOperation::RowStream* row_stream) {
  RowBlock row_block(std::move(row_fields));
  // Consume row_stream
  while (row_stream->hasNext()) {
    auto eph_row = row_stream->consumeRow();
    copyRowToRowBlock(&row_block, eph_row);
  }
  return row_block;
}

} // namespace facebook::common::mysql_client
