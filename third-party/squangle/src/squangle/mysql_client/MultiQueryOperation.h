/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/FetchOperation.h"

namespace facebook::common::mysql_client {

class MultiQueryOperation;

using MultiQueryCallback = std::function<
    void(MultiQueryOperation&, QueryResult*, QueryCallbackReason)>;

// An operation representing a query with multiple statements.
// If a callback is set, it invokes the callback as rows arrive.
// If there is no callback, it buffers all results into memory
// and makes them available as a RowBlock.
// This is inefficient for large results.
//
// Constructed via Connection::beginMultiQuery.
class MultiQueryOperation : public FetchOperation {
 public:
  ~MultiQueryOperation() override = default;

  // Set our callback.  This is invoked multiple times -- once for
  // every RowBatch and once, with nullptr for the RowBatch,
  // indicating the query is complete.
  void setCallback(MultiQueryCallback cb) {
    buffered_query_callback_ = std::move(cb);
  }
  void chainCallback(MultiQueryCallback cb) {
    auto origCb = std::move(buffered_query_callback_);
    if (origCb) {
      cb = [origCb = std::move(origCb), cb = std::move(cb)](
               MultiQueryOperation& op,
               QueryResult* result,
               QueryCallbackReason reason) {
        origCb(op, result, reason);
        cb(op, result, reason);
      };
    }
    setCallback(std::move(cb));
  }

  // Steal all rows. Only valid if there is no callback. Inefficient
  // for large result sets.
  // Only call after the query has finished, don't use it inside callbacks
  std::vector<QueryResult>&& stealQueryResults() {
    CHECK_THROW(done(), db::OperationStateException);
    return std::move(query_results_);
  }

  // Only call this after the query has finished and don't use it inside
  // callbacks
  const std::vector<QueryResult>& queryResults() const {
    CHECK_THROW(done(), db::OperationStateException);
    return query_results_;
  }

  // Returns the Query for a query index.
  const Query& getQuery(int index) const {
    return queries_.getQuery(index);
  }

  // Returns the list of Queries
  const std::vector<Query>& getQueries() const {
    return queries_.getQueries();
  }

  void setQueryResults(std::vector<QueryResult> query_results) {
    query_results_ = std::move(query_results);
  }

  // Overriding to narrow the return type
  MultiQueryOperation& setTimeout(Duration timeout) {
    Operation::setTimeout(timeout);
    return *this;
  }

  db::OperationType getOperationType() const override {
    return db::OperationType::MultiQuery;
  }

 protected:
  void notifyInitQuery() override;
  void notifyRowsReady() override;
  bool notifyQuerySuccess(bool more_results) override;
  void notifyFailure(OperationResult result) override;
  void notifyOperationCompleted(OperationResult result) override;

  MultiQueryOperation(
      std::unique_ptr<FetchOperationImpl> impl,
      std::vector<Query>&& queries);

  // Calls the FetchOperation specializedCompleteOperation and then does
  // callbacks if needed

 private:
  MultiQueryCallback buffered_query_callback_;

  // Storage fields for every statement in the query
  // Only to be used if there is no callback set.
  std::vector<QueryResult> query_results_;
  // Buffer to trans to `query_results_` and for buffered callback.
  std::unique_ptr<QueryResult> current_query_result_;

  int num_current_query_ = 0;

  friend class Connection;
};

} // namespace facebook::common::mysql_client
