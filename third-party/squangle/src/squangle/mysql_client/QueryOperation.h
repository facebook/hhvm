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

using QueryCallback =
    std::function<void(QueryOperation&, QueryResult*, QueryCallbackReason)>;

// An operation representing a query.  If a callback is set, it
// invokes the callback as rows arrive.  If there is no callback, it
// buffers all results into memory and makes them available as a
// RowBlock.  This is inefficient for large results.
//
// Constructed via Connection::beginQuery.
class QueryOperation : public FetchOperation {
 public:
  ~QueryOperation() override = default;

  void setCallback(QueryCallback cb) {
    buffered_query_callback_ = std::move(cb);
  }
  void chainCallback(QueryCallback cb) {
    auto origCb = std::move(buffered_query_callback_);
    if (origCb) {
      cb = [origCb = std::move(origCb), cb = std::move(cb)](
               QueryOperation& op,
               QueryResult* result,
               QueryCallbackReason reason) {
        origCb(op, result, reason);
        cb(op, result, reason);
      };
    }
    setCallback(cb);
  }

  // Steal all rows.  Only valid if there is no callback.  Inefficient
  // for large result sets.
  QueryResult&& stealQueryResult() {
    CHECK_THROW(ok(), db::OperationStateException);
    return std::move(*query_result_);
  }

  const QueryResult& queryResult() const {
    CHECK_THROW(ok(), db::OperationStateException);
    return *query_result_;
  }

  // Returns the Query of this operation
  const Query& getQuery() const {
    return queries_.getQuery(0);
  }

  // Steal all rows.  Only valid if there is no callback.  Inefficient
  // for large result sets.
  std::vector<RowBlock>&& stealRows() {
    return query_result_->stealRows();
  }

  const std::vector<RowBlock>& rows() const {
    return query_result_->rows();
  }

  // Last insert id (aka mysql_insert_id).
  uint64_t lastInsertId() const {
    return query_result_->lastInsertId();
  }

  // Number of rows affected (aka mysql_affected_rows).
  uint64_t numRowsAffected() const {
    return query_result_->numRowsAffected();
  }

  // Received gtid.
  const std::string& recvGtid() const {
    return query_result_->recvGtid();
  }

  void setQueryResult(QueryResult query_result) {
    query_result_ = std::make_unique<QueryResult>(std::move(query_result));
  }

  // Overriding to narrow the return type
  QueryOperation& setTimeout(Duration timeout) {
    Operation::setTimeout(timeout);
    return *this;
  }

  db::OperationType getOperationType() const override {
    return db::OperationType::Query;
  }

 protected:
  void notifyInitQuery() override;
  void notifyRowsReady() override;
  void notifyQuerySuccess(bool more_results) override;
  void notifyFailure(OperationResult result) override;
  void notifyOperationCompleted(OperationResult result) override;

  QueryOperation(std::unique_ptr<FetchOperationImpl> impl, Query&& query);

 private:
  QueryCallback buffered_query_callback_;
  std::unique_ptr<QueryResult> query_result_;
  friend class Connection;
};

} // namespace facebook::common::mysql_client
