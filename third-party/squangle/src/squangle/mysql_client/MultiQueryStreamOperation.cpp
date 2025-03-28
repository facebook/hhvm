/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/MultiQueryStreamOperation.h"

namespace facebook::common::mysql_client {

/*static*/
std::shared_ptr<MultiQueryStreamOperation> MultiQueryStreamOperation::create(
    std::unique_ptr<FetchOperationImpl> opImpl,
    MultiQuery&& multi_query) {
  return std::shared_ptr<MultiQueryStreamOperation>(
      new MultiQueryStreamOperation(std::move(opImpl), std::move(multi_query)));
}

MultiQueryStreamOperation::MultiQueryStreamOperation(
    std::unique_ptr<FetchOperationImpl> opImpl,
    MultiQuery&& multi_query)
    : FetchOperation(std::move(opImpl), std::move(multi_query)) {}

MultiQueryStreamOperation::MultiQueryStreamOperation(
    std::unique_ptr<FetchOperationImpl> opImpl,
    std::vector<Query>&& queries)
    : FetchOperation(std::move(opImpl), std::move(queries)) {}

void MultiQueryStreamOperation::invokeCallback(StreamState reason) {
  // Construct a CallbackVistor object and pass to apply_vistor. It will
  // call the appropriate overaload of 'operator()' depending on the type
  // of callback stored in stream_callback_ i.e. either MultiQueryStreamHandler
  // or MultiQueryStreamOperation::Callback.
  std::visit(CallbackVisitor(*this, reason), stream_callback_);
}

void MultiQueryStreamOperation::notifyInitQuery() {
  invokeCallback(StreamState::InitQuery);
}

void MultiQueryStreamOperation::notifyRowsReady() {
  invokeCallback(StreamState::RowsReady);
}

bool MultiQueryStreamOperation::notifyQuerySuccess(bool) {
  // Query Boundary, only for streaming to allow the user to read from the
  // connection.
  // This will allow pause in the end of the query. End of operations don't
  // allow.
  invokeCallback(StreamState::QueryEnded);
  return true;
}

void MultiQueryStreamOperation::notifyFailure(OperationResult) {
  // Nop
}

void MultiQueryStreamOperation::notifyOperationCompleted(
    OperationResult result) {
  auto reason =
      (result == OperationResult::Succeeded ? StreamState::Success
                                            : StreamState::Failure);

  invokeCallback(reason);
  stream_callback_ = nullptr;
}

} // namespace facebook::common::mysql_client
