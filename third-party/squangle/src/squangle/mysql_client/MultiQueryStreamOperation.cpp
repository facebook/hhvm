/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/MultiQueryStreamOperation.h"

namespace facebook::common::mysql_client {

MultiQueryStreamOperation::MultiQueryStreamOperation(
    std::unique_ptr<ConnectionProxy> conn,
    MultiQuery&& multi_query)
    : FetchOperation(std::move(conn), std::move(multi_query)) {}

MultiQueryStreamOperation::MultiQueryStreamOperation(
    std::unique_ptr<ConnectionProxy> conn,
    std::vector<Query>&& queries)
    : FetchOperation(std::move(conn), std::move(queries)) {}

void MultiQueryStreamOperation::invokeCallback(StreamState reason) {
  // Construct a CallbackVistor object and pass to apply_vistor. It will
  // call the appropriate overaload of 'operator()' depending on the type
  // of callback stored in stream_callback_ i.e. either MultiQueryStreamHandler
  // or MultiQueryStreamOperation::Callback.
  boost::apply_visitor(CallbackVisitor(*this, reason), stream_callback_);
}

void MultiQueryStreamOperation::notifyInitQuery() {
  invokeCallback(StreamState::InitQuery);
}

void MultiQueryStreamOperation::notifyRowsReady() {
  invokeCallback(StreamState::RowsReady);
}

void MultiQueryStreamOperation::notifyQuerySuccess(bool) {
  // Query Boundary, only for streaming to allow the user to read from the
  // connection.
  // This will allow pause in the end of the query. End of operations don't
  // allow.
  invokeCallback(StreamState::QueryEnded);
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
