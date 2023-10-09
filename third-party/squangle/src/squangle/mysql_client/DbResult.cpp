/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/DbResult.h"

#include <ostream>

#include <folly/ScopeGuard.h>

#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/Operation.h"

namespace facebook {
namespace common {
namespace mysql_client {

std::ostream& operator<<(
    std::ostream& stream,
    MultiQueryStreamHandler::State state) {
  stream << MultiQueryStreamHandler::toString(state);
  return stream;
}

MysqlException::MysqlException(
    OperationResult failure_type,
    unsigned int mysql_errno,
    const std::string& mysql_error,
    ConnectionKey conn_key,
    Duration elapsed_time)
    : Exception(
          (failure_type == OperationResult::Failed ||
           failure_type == OperationResult::TimedOut ||
           failure_type == OperationResult::Cancelled)
              ? fmt::format(
                    "Mysql error {}. {} to db {} at {}:{}",
                    mysql_errno,
                    mysql_error,
                    conn_key.db_name(),
                    conn_key.host(),
                    conn_key.port())
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
    ConnectionKey conn_key,
    Duration elapsed)
    : OperationResultBase(std::move(conn_key), elapsed),
      conn_(std::move(conn)),
      result_(result) {}

std::unique_ptr<Connection> DbResult::releaseConnection() {
  return std::move(conn_);
}

ConnectResult::ConnectResult(
    std::unique_ptr<Connection>&& conn,
    OperationResult result,
    ConnectionKey conn_key,
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

QueryResult::QueryResult(QueryResult&& other) noexcept
    : row_fields_info_(other.row_fields_info_),
      query_num_(other.query_num_),
      partial_(other.partial_),
      was_slow_(other.was_slow_),
      num_rows_(other.num_rows_),
      num_rows_affected_(other.num_rows_affected_),
      last_insert_id_(other.last_insert_id_),
      recv_gtid_(std::move(other.recv_gtid_)),
      resp_attrs_(std::move(other.resp_attrs_)),
      operation_result_(other.operation_result_),
      row_blocks_(std::move(other.row_blocks_)) {
  other.row_blocks_.clear();
  other.num_rows_ = 0;
}

QueryResult& QueryResult::operator=(QueryResult&& other) {
  if (this != &other) {
    row_fields_info_ = other.row_fields_info_;
    query_num_ = other.query_num_;
    partial_ = other.partial_;
    num_rows_ = other.num_rows_;
    num_rows_affected_ = other.num_rows_affected_;
    last_insert_id_ = other.last_insert_id_;
    recv_gtid_ = std::move(other.recv_gtid_);
    resp_attrs_ = std::move(other.resp_attrs_);
    operation_result_ = other.operation_result_;

    row_blocks_ = std::move(other.row_blocks_);
    other.row_blocks_.clear();
    other.num_rows_ = 0;
    was_slow_ = other.was_slow_;
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

StreamedQueryResult::StreamedQueryResult(
    MultiQueryStreamHandler* stream_handler,
    size_t query_idx)
    : stream_handler_(stream_handler), query_idx_(query_idx) {}

StreamedQueryResult::~StreamedQueryResult() {
  // In case of premature deletion.
  checkAccessToResult();
}

StreamedQueryResult::Iterator StreamedQueryResult::begin() {
  StreamedQueryResult::Iterator begin_it(this, 0);
  begin_it.increment();
  return begin_it;
}

StreamedQueryResult::Iterator StreamedQueryResult::end() {
  return StreamedQueryResult::Iterator(this, -1);
}

void StreamedQueryResult::Iterator::increment() {
  auto row = query_res_->nextRow();
  if (!row) {
    row_num_ = -1;
    return;
  }
  ++row_num_;
  current_row_ = std::move(row);
}

const EphemeralRow& StreamedQueryResult::Iterator::dereference() const {
  return *current_row_;
}

folly::Optional<EphemeralRow> StreamedQueryResult::nextRow() {
  checkStoredException();
  // Blocks when the stream is over and we need to handle IO
  auto current_row = stream_handler_->fetchOneRow(this);
  if (!current_row) {
    checkStoredException();
  } else {
    ++num_rows_;
  }
  return current_row;
}

void StreamedQueryResult::checkStoredException() {
  if (exception_wrapper_) {
    SCOPE_EXIT {
      exception_wrapper_ = {};
    };
    exception_wrapper_.throw_exception();
  }
}

void StreamedQueryResult::checkAccessToResult() {
  checkStoredException();
  if (stream_handler_) {
    stream_handler_->fetchQueryEnd(this);
  }
}

void StreamedQueryResult::setResult(
    int64_t affected_rows,
    int64_t last_insert_id,
    const std::string& recv_gtid,
    const RespAttrs& resp_attrs) {
  num_affected_rows_ = affected_rows;
  last_insert_id_ = last_insert_id;
  recv_gtid_ = recv_gtid;
  resp_attrs_ = resp_attrs;
}

void StreamedQueryResult::setException(folly::exception_wrapper ex) {
  exception_wrapper_ = std::move(ex);
}

void StreamedQueryResult::freeHandler() {
  stream_handler_.reset();
}

MultiQueryStreamHandler::MultiQueryStreamHandler(
    std::shared_ptr<MultiQueryStreamOperation> op)
    : operation_(std::move(op)) {}

folly::Optional<StreamedQueryResult> MultiQueryStreamHandler::nextQuery() {
  if (state_ == State::RunQuery) {
    start();
  }

  // Runs in User thread
  connection()->wait();
  DCHECK(operation_->isPaused() || operation_->done());

  folly::Optional<StreamedQueryResult> res;
  // Accepted states: InitResult, OperationSucceeded or OperationFailed
  if (state_ == State::InitResult) {
    res.assign(StreamedQueryResult(this, ++curr_query_));
    resumeOperation();
  } else if (state_ == State::OperationFailed) {
    handleQueryFailed(nullptr);
  } else if (state_ != State::OperationSucceeded) {
    LOG(DFATAL) << "Bad state transition. Perhaps reading next result without"
                << " deleting or consuming current stream? Current state is "
                << toString(state_) << ".";
    handleBadState();
  }
  return res;
}

std::unique_ptr<Connection> MultiQueryStreamHandler::releaseConnection() {
  // Runs in User thread
  connection()->wait();
  if (state_ == State::OperationSucceeded || state_ == State::OperationFailed) {
    return operation_->releaseConnection();
  }

  exception_wrapper_ =
      folly::make_exception_wrapper<db::OperationStateException>(
          "Trying to release connection without consuming stream");
  LOG(DFATAL) << "Releasing the Connection without reading result. Read stream"
              << " content or delete stream result. Current state "
              << toString(state_) << ".";
  handleBadState();

  // Should throw above.
  return nullptr;
}

// Information about why this operation failed.
unsigned int MultiQueryStreamHandler::mysql_errno() const {
  return operation_->mysql_errno();
}

const std::string& MultiQueryStreamHandler::mysql_error() const {
  return operation_->mysql_error();
}

void MultiQueryStreamHandler::streamCallback(
    FetchOperation* op,
    StreamState op_state) {
  // Runs in IO Thread
  if (op_state == StreamState::InitQuery) {
    op->pauseForConsumer();
    state_ = State::InitResult;
  } else if (op_state == StreamState::RowsReady) {
    op->pauseForConsumer();
    state_ = State::ReadRows;
  } else if (op_state == StreamState::QueryEnded) {
    op->pauseForConsumer();
    state_ = State::ReadResult;
  } else if (op_state == StreamState::Success) {
    state_ = State::OperationSucceeded;
  } else {
    exception_wrapper_ = folly::make_exception_wrapper<QueryException>(
        op->numCurrentQuery(),
        op->result(),
        op->mysql_errno(),
        op->mysql_error(),
        *op->connection()->getKey(),
        op->elapsed());
    state_ = State::OperationFailed;
  }
  op->connection()->notify();
}

folly::Optional<EphemeralRow> MultiQueryStreamHandler::fetchOneRow(
    StreamedQueryResult* result) {
  checkStreamedQueryResult(result);
  connection()->wait();
  // Accepted states: ReadRows, ReadResult, OperationFailed
  if (state_ == State::ReadRows) {
    if (!operation_->rowStream()->hasNext()) {
      resumeOperation();
      // Recursion to get `wait` and double check the stream.
      return fetchOneRow(result);
    }
    return folly::Optional<EphemeralRow>(operation_->rowStream()->consumeRow());
  }

  if (state_ == State::ReadResult) {
    handleQueryEnded(result);
  } else if (state_ == State::OperationFailed) {
    handleQueryFailed(result);
  } else {
    LOG(DFATAL) << "Bad state transition. Only ReadRows, ReadResult and "
                << "OperationFailed are allowed. Received " << toString(state_)
                << ".";
    handleBadState();
  }
  return folly::Optional<EphemeralRow>();
}

void MultiQueryStreamHandler::fetchQueryEnd(StreamedQueryResult* result) {
  checkStreamedQueryResult(result);
  connection()->wait();
  // Accepted states: ReadResult, OperationFailed
  if (state_ == State::ReadResult) {
    handleQueryEnded(result);
  } else if (state_ == State::OperationFailed) {
    handleQueryFailed(result);
  } else if (state_ != State::ReadRows || fetchOneRow(result)) {
    LOG(DFATAL) << "Expected end of query, but received " << toString(state_)
                << ".";
    handleBadState();
  }
}

void MultiQueryStreamHandler::resumeOperation() {
  connection()->resetActionable();
  operation_->resume();
}

void MultiQueryStreamHandler::handleQueryEnded(StreamedQueryResult* result) {
  result->setResult(
      operation_->currentAffectedRows(),
      operation_->currentLastInsertId(),
      operation_->currentRecvGtid(),
      operation_->currentRespAttrs());
  result->freeHandler();
  resumeOperation();
}

void MultiQueryStreamHandler::handleQueryFailed(StreamedQueryResult* result) {
  DCHECK(exception_wrapper_);
  if (result) {
    result->setException(exception_wrapper_);
    result->freeHandler();
  } else {
    exception_wrapper_.throw_exception();
  }
}

void MultiQueryStreamHandler::handleBadState() {
  operation_->cancel();
  resumeOperation();
}

void MultiQueryStreamHandler::start() {
  CHECK_EQ(state_, State::RunQuery);
  CHECK(operation_);
  operation_->setCallback(this);
  state_ = State::WaitForInitResult;
  operation_->run();
}

void MultiQueryStreamHandler::checkStreamedQueryResult(
    StreamedQueryResult* result) {
  CHECK_EQ(result->stream_handler_.get(), this);
  CHECK_EQ(result->query_idx_, curr_query_);
}

std::string MultiQueryStreamHandler::toString(State state) {
  switch (state) {
    case State::RunQuery:
      return "RunQuery";
    case State::WaitForInitResult:
      return "WaitForInitResult";
    case State::InitResult:
      return "InitResult";
    case State::ReadRows:
      return "ReadRows";
    case State::ReadResult:
      return "ReadResult";
    case State::OperationSucceeded:
      return "OperationSucceeded";
    case State::OperationFailed:
      return "OperationFailed";
    default:
      LOG(DFATAL) << "Illegal state" << (int64_t)state;
  }
  return "Unknown";
}

MultiQueryStreamHandler::MultiQueryStreamHandler(
    MultiQueryStreamHandler&& other) noexcept {
  // Its OK to move another object only if we haven't
  // yet invoked nextQuery on it or if the operation
  // is done()
  CHECK(other.state_ == State::RunQuery || other.operation_->done());
  operation_ = std::move(other.operation_);
  other.operation_ = nullptr;
}

MultiQueryStreamHandler::~MultiQueryStreamHandler() {
  if (operation_) {
    CHECK(
        state_ == State::OperationSucceeded ||
        state_ == State::OperationFailed);
    CHECK(operation_->done());
  }
}

Connection* MultiQueryStreamHandler::connection() const {
  return operation_->connection();
}

EphemeralRowFields* StreamedQueryResult::getRowFields() const {
  CHECK(stream_handler_ != nullptr) << "Trying to get the row fileds after "
                                    << "query end";
  return stream_handler_->operation_->rowStream()->getEphemeralRowFields();
}
} // namespace mysql_client
} // namespace common
} // namespace facebook
