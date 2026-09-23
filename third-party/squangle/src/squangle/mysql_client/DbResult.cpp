/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/DbResult.h"

#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/Operation.h"

namespace facebook::common::mysql_client {

MysqlException::MysqlException(
    OperationResult failure_type,
    unsigned int mysql_errno,
    const std::string& mysql_error,
    std::shared_ptr<const ConnectionKey> conn_key,
    Duration elapsed_time)
    : Exception(
          (failure_type == OperationResult::Failed ||
           failure_type == OperationResult::TimedOut ||
           failure_type == OperationResult::Cancelled)
              ? fmt::format(
                    "Mysql error {}. {} to db {} at {}:{}",
                    mysql_errno,
                    mysql_error,
                    conn_key->db_name(),
                    conn_key->host(),
                    conn_key->port())
              : std::string(Operation::toString(failure_type))),
      OperationResultBase(std::move(conn_key), elapsed_time),
      failure_type_(failure_type),
      mysql_errno_(mysql_errno),
      mysql_error_(mysql_error) {}

bool DbResult::ok() const {
  return result_ == OperationResult::Succeeded;
}

DbResult::DbResult(
    std::unique_ptr<Connection>&& conn,
    OperationResult result,
    std::shared_ptr<const ConnectionKey> conn_key,
    Duration elapsed)
    : OperationResultBase(std::move(conn_key), elapsed),
      conn_(std::move(conn)),
      result_(result) {}

DbResult::~DbResult() = default;

DbResult::DbResult(DbResult&& other) noexcept = default;
DbResult& DbResult::operator=(DbResult&& other) noexcept = default;

std::unique_ptr<Connection> DbResult::releaseConnection() {
  return std::move(conn_);
}

ConnectResult::ConnectResult(
    std::unique_ptr<Connection>&& conn,
    OperationResult result,
    std::shared_ptr<const ConnectionKey> conn_key,
    Duration elapsed_time,
    uint32_t num_attempts)
    : DbResult(std::move(conn), result, std::move(conn_key), elapsed_time),
      num_attempts_(num_attempts) {}

QueryResult::QueryResult(int query_num)
    : query_num_(query_num),
      partial_(true),
      num_rows_(0),
      num_rows_affected_(0),
      last_insert_id_(0),
      operation_result_(OperationResult::Unknown) {}

// In the constructor below, the code does not move `row_fields_info_` over to
// the new QueryResult.  It breaks if you change it.  When I debugged it, I see
// that inside the callback the customer is allowed to move the result into
// their own storage (i.e. https://fburl.com/code/1l7wvull).  Apparently we are
// reusing the QueryResult internally and if we row fields info over the reused
// one will no longer have a copy of the shared_ptr.  This is not an ideal
// situation, but for now, just leave it.
QueryResult::QueryResult(QueryResult&& other) noexcept
    : row_fields_info_(other.row_fields_info_), // don't use std::move()
      query_num_(other.query_num_),
      partial_(other.partial_),
      was_slow_(other.was_slow_),
      num_rows_(other.num_rows_),
      num_rows_affected_(other.num_rows_affected_),
      last_insert_id_(other.last_insert_id_),
      recv_gtid_(std::move(other.recv_gtid_)),
      resp_attrs_(std::move(other.resp_attrs_)),
      deferred_resp_attrs_(std::move(other.deferred_resp_attrs_)),
      warnings_count_(other.warnings_count_),
      mysql_info_(other.mysql_info_),
      rows_matched_(other.rows_matched_),
      operation_result_(other.operation_result_),
      row_blocks_(std::move(other.row_blocks_)) {
  other.row_blocks_.clear();
  other.num_rows_ = 0;
  other.warnings_count_ = 0;
  other.mysql_info_ = std::nullopt;
  other.rows_matched_ = std::nullopt;
}

QueryResult& QueryResult::operator=(QueryResult&& other) noexcept {
  if (this != &other) {
    row_fields_info_ = other.row_fields_info_;
    query_num_ = other.query_num_;
    partial_ = other.partial_;
    was_slow_ = other.was_slow_;
    num_rows_ = other.num_rows_;
    num_rows_affected_ = other.num_rows_affected_;
    last_insert_id_ = other.last_insert_id_;
    recv_gtid_ = std::move(other.recv_gtid_);
    resp_attrs_ = std::move(other.resp_attrs_);
    deferred_resp_attrs_ = std::move(other.deferred_resp_attrs_);
    warnings_count_ = other.warnings_count_;
    mysql_info_ = std::move(other.mysql_info_),
    rows_matched_ = other.rows_matched_,
    operation_result_ = other.operation_result_;

    row_blocks_ = std::move(other.row_blocks_);
    other.row_blocks_.clear();
    other.num_rows_ = 0;
    other.warnings_count_ = 0;
    other.mysql_info_ = std::nullopt;
    other.rows_matched_ = std::nullopt;
  }
  return *this;
}

bool QueryResult::ok() const {
  return (partial_ && operation_result_ == OperationResult::Unknown) ||
      operation_result_ == OperationResult::Succeeded;
}

bool QueryResult::succeeded() const {
  return operation_result_ == OperationResult::Succeeded;
}

void QueryResult::setOperationResult(OperationResult op_result) {
  operation_result_ = op_result;
}

} // namespace facebook::common::mysql_client
