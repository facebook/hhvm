/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/MultiQueryOperation.h"
#include "squangle/mysql_client/OperationHelpers.h"

namespace facebook::common::mysql_client {

MultiQueryOperation::MultiQueryOperation(
    std::unique_ptr<FetchOperationImpl> impl,
    std::vector<Query>&& queries)
    : FetchOperation(std::move(impl), std::move(queries)),
      current_query_result_(std::make_unique<QueryResult>(0)) {}

void MultiQueryOperation::notifyInitQuery() {
  auto* row_stream = rowStream();
  if (row_stream) {
    // Populate RowFields, this is the metadata of rows.
    current_query_result_->setRowFields(
        row_stream->getEphemeralRowFields()->makeBufferedFields());
  }
}

void MultiQueryOperation::notifyRowsReady() {
  // Create buffered RowBlock
  auto row_block = makeRowBlockFromStream(
      current_query_result_->getSharedRowFields(), rowStream());
  if (row_block.numRows() == 0) {
    return;
  }

  current_query_result_->appendRowBlock(std::move(row_block));
  if (buffered_query_callback_) {
    buffered_query_callback_(
        *this, current_query_result_.get(), QueryCallbackReason::RowsFetched);
  }
}

void MultiQueryOperation::notifyFailure(OperationResult result) {
  // This needs to be called before notifyOperationCompleted, because
  // in non-callback mode we "notify" the conditional variable in `Connection`.
  current_query_result_->setOperationResult(result);
}

void MultiQueryOperation::notifyQuerySuccess(bool) {
  current_query_result_->setPartial(false);

  current_query_result_->setOperationResult(OperationResult::Succeeded);
  current_query_result_->setNumRowsAffected(
      FetchOperation::currentAffectedRows());
  current_query_result_->setLastInsertId(FetchOperation::currentLastInsertId());
  current_query_result_->setRecvGtid(FetchOperation::currentRecvGtid());
  current_query_result_->setResponseAttributes(
      FetchOperation::currentRespAttrs());

  // Notify the callback before moving the result into the operation. This is
  // because the callback can't access the result out of the operation.
  if (buffered_query_callback_) {
    buffered_query_callback_(
        *this, current_query_result_.get(), QueryCallbackReason::QueryBoundary);
  }

  query_results_.emplace_back(std::move(*current_query_result_.get()));
  current_query_result_ =
      std::make_unique<QueryResult>(current_query_result_->queryNum() + 1);
}

void MultiQueryOperation::notifyOperationCompleted(OperationResult result) {
  if (!buffered_query_callback_) { // No callback to be done
    return;
  }
  // Nothing that changes the non-callback state is safe to be done here.
  auto reason =
      (result == OperationResult::Succeeded ? QueryCallbackReason::Success
                                            : QueryCallbackReason::Failure);
  buffered_query_callback_(*this, current_query_result_.get(), reason);
  // Release callback since no other callbacks will be made
  buffered_query_callback_ = nullptr;
}

} // namespace facebook::common::mysql_client
