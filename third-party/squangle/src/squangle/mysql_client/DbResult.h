/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef COMMON_ASYNC_MYSQL_RESULT_H
#define COMMON_ASYNC_MYSQL_RESULT_H

#include "squangle/base/Base.h"
#include "squangle/base/ConnectionKey.h"
#include "squangle/base/ExceptionUtil.h"
#include "squangle/logger/DBEventLogger.h"
#include "squangle/mysql_client/Row.h"

#include <folly/Exception.h>
#include <folly/ExceptionWrapper.h>
#include <folly/fibers/Baton.h>
#include <chrono>

namespace facebook {
namespace common {
namespace mysql_client {

enum class OperationResult;

// Basic info about the Operation and connection info, everything that is
// common between failure and success should come here.
class OperationResultBase {
 public:
  OperationResultBase(ConnectionKey conn_key, Duration elapsed_time)
      : conn_key_(std::move(conn_key)), elapsed_time_(elapsed_time) {}

  const ConnectionKey* getConnectionKey() const {
    return &conn_key_;
  }

  Duration elapsed() const {
    return elapsed_time_;
  }

 private:
  const ConnectionKey conn_key_;
  const Duration elapsed_time_;
};

// This exception represents a basic mysql error, either during a connection
// opening or querying, when it times out or a mysql error happened
// (invalid host or query, disconnected during query, etc)
class MysqlException : public db::Exception, public OperationResultBase {
 public:
  MysqlException(
      OperationResult failure_type,
      unsigned int mysql_errno,
      const std::string& mysql_error,
      ConnectionKey conn_key,
      Duration elapsed_time);

  unsigned int mysql_errno() const {
    return mysql_errno_;
  }
  const std::string& mysql_error() const {
    return mysql_error_;
  }

  // Returns the type of error occurred:
  // Cancelled: query it was cancelled by the client or user
  // Failed: generally a mysql error happened during the query,
  // more details will be in `mysql_errno`
  // Timeout: query timed out and the client timer was triggered
  OperationResult failureType() const {
    return failure_type_;
  }

 private:
  OperationResult failure_type_;

  unsigned int mysql_errno_;
  std::string mysql_error_;
};

// This exception represents a Query error, when it times out or a
// mysql error happened (invalid query, disconnected during query, etc)
class QueryException : public MysqlException {
 public:
  QueryException(
      int num_executed_queries,
      OperationResult failure_type,
      unsigned int mysql_errno,
      const std::string& mysql_error,
      ConnectionKey conn_key,
      Duration elapsed_time)
      : MysqlException(
            failure_type,
            mysql_errno,
            mysql_error,
            std::move(conn_key),
            elapsed_time),
        num_executed_queries_(num_executed_queries) {}

  // In case of MultiQuery was ran, some queries might have succeeded
  int numExecutedQueries() const {
    return num_executed_queries_;
  }

 private:
  int num_executed_queries_;
};

class Connection;

class DbResult : public OperationResultBase {
 public:
  DbResult(
      std::unique_ptr<Connection>&& conn,
      OperationResult result,
      ConnectionKey conn_key,
      Duration elapsed_time);

  bool ok() const;

  // releases the connection that was used for the async operation.
  // Call this only in the future interface.
  std::unique_ptr<Connection> releaseConnection();

  OperationResult operationResult() const {
    return result_;
  }

 private:
  std::unique_ptr<Connection> conn_;

  OperationResult result_;
};

class ConnectResult : public DbResult {
 public:
  ConnectResult(
      std::unique_ptr<Connection>&& conn,
      OperationResult result,
      ConnectionKey conn_key,
      Duration elapsed_time,
      uint32_t num_attempts);

  uint32_t numAttempts() const {
    return num_attempts_;
  }

 private:
  uint32_t num_attempts_;
};

template <typename SingleMultiResult>
class FetchResult : public DbResult {
 public:
  FetchResult(
      SingleMultiResult query_result,
      int num_queries_executed,
      uint64_t result_size,
      std::unique_ptr<Connection>&& conn,
      OperationResult result,
      ConnectionKey conn_key,
      Duration elapsed)
      : DbResult(std::move(conn), result, std::move(conn_key), elapsed),
        fetch_result_(std::move(query_result)),
        num_queries_executed_(num_queries_executed),
        result_size_(result_size) {}

  int numQueriesExecuted() const {
    return num_queries_executed_;
  }

  uint64_t resultSize() const {
    return result_size_;
  }

  const SingleMultiResult& queryResult() const {
    return fetch_result_;
  }

  SingleMultiResult&& stealQueryResult() {
    return std::move(fetch_result_);
  }

 private:
  SingleMultiResult fetch_result_;

  int num_queries_executed_;
  uint64_t result_size_;
};

// A QueryResult encapsulates the data regarding a query, as rows fetched,
// last insert id, etc.
// It is intended to create a layer over a collection of RowBlock of the same
// query.
// It is not not given index access for sake of efficiency that in this
// collection.
//
// Iterator access is provided to loop between Row's. We give the rows
// sequentially in the same order they where added in RowBlocks.
//
// for (const auto& row : query_result) {
//  ...
// }
class QueryResult {
 public:
  using RespAttrs = AttributeMap;
  class Iterator;

  explicit QueryResult(int queryNum);

  ~QueryResult() {}

  // Move Constructor
  QueryResult(QueryResult&& other) noexcept;

  // Move assignment
  QueryResult& operator=(QueryResult&& other);

  int queryNum() const {
    return query_num_;
  }

  bool hasRows() const {
    return num_rows_ > 0;
  }

  // Partial is set true when this has part of the results, as in a callback.
  // When the QueryResult has all the data for a query this is false, typically
  // when the query is completely ran before user starts capturing results.
  bool partial() const {
    return partial_;
  }

  // A QueryResult is ok when it is a partial result during the operation
  // given in callbacks or the query has succeeded.
  bool ok() const;

  // TODO#4890524: Remove this call
  bool succeeded() const;

  RowFields* getRowFields() const {
    return row_fields_info_.get();
  }
  std::shared_ptr<RowFields> getSharedRowFields() const {
    return row_fields_info_;
  }

  // Only call this if you are in a callback and really want the Rows.
  // If you want to iterate through rows just use the iterator class here.
  RowBlock& currentRowBlock() {
    CHECK_THROW(
        partial() && row_blocks_.size() == 1, db::OperationStateException);
    return row_blocks_[0];
  }

  // Only call this if you are in a callback and really want just the ownership
  // over the Rows. Another way to obtain the ownership is moving the
  // QueryResult to a location of your preference and the Rows are going to be
  // moved to the new location.
  // If you want to iterate through rows just use the iterator class here.
  RowBlock stealCurrentRowBlock() {
    CHECK_THROW(
        partial() && row_blocks_.size() == 1, db::OperationStateException);
    RowBlock ret(std::move(row_blocks_[0]));
    row_blocks_.clear();
    return ret;
  }

  // Only call this if the fetch operation has ended. There are two ways to own
  // the RowBlocks, you can either use this method or move QueryResult to a
  // location of your preference and the RowBlocks are going to be moved to
  // the new location as well. If you want to iterate through rows just use the
  // iterator class here.
  std::vector<RowBlock>&& stealRows() {
    return std::move(row_blocks_);
  }

  // Only call this if the fetch operation has ended.
  // If you want to iterate through rows just use the iterator class here.
  const std::vector<RowBlock>& rows() const {
    return row_blocks_;
  }

  void setRowBlocks(std::vector<RowBlock>&& row_blocks) {
    num_rows_ = 0;
    row_blocks_ = std::move(row_blocks);
    for (const auto& block : row_blocks_) {
      num_rows_ += block.numRows();
    }
  }

  void setOperationResult(OperationResult op_result);

  // Last insert id (aka mysql_insert_id).
  uint64_t lastInsertId() const {
    return last_insert_id_;
  }

  void setLastInsertId(uint64_t last_insert_id) {
    last_insert_id_ = last_insert_id;
  }

  // Number of rows affected (aka mysql_affected_rows).
  uint64_t numRowsAffected() const {
    return num_rows_affected_;
  }

  void setNumRowsAffected(uint64_t num_rows_affected) {
    num_rows_affected_ = num_rows_affected;
  }

  // Current GTID
  const std::string& recvGtid() const {
    return recv_gtid_;
  }

  void setRecvGtid(const std::string& recv_gtid) {
    recv_gtid_ = recv_gtid;
  }

  // Query was slow
  bool wasSlow() const {
    return was_slow_;
  }

  void setWasSlow(bool was_slow) {
    was_slow_ = was_slow;
  }

  // Current response attributes
  const RespAttrs& responseAttributes() const {
    return resp_attrs_;
  }

  void setResponseAttributes(RespAttrs resp_attrs) {
    resp_attrs_ = std::move(resp_attrs);
  }

  // This can be called for complete or partial results. It's going to return
  // the total of rows stored in the QueryResult.
  size_t numRows() const {
    return num_rows_;
  }

  size_t numBlocks() const {
    return row_blocks_.size();
  }

  // Function for easier lookup of single row result, in case the result has
  // more rows, it will throw exception
  Row getOnlyRow() const {
    assertOnlyRow();
    return row_blocks_.at(0).getRow(0);
  }

  class Iterator : public boost::iterator_facade<
                       Iterator,
                       const Row,
                       boost::single_pass_traversal_tag,
                       const Row> {
   public:
    Iterator(
        const std::vector<RowBlock>* row_block_vector,
        size_t block_number,
        size_t row_number_in_block)
        : row_block_vector_(row_block_vector),
          current_block_number_(block_number),
          current_row_in_block_(row_number_in_block) {}

    void increment() {
      ++current_row_in_block_;
      if (current_row_in_block_ >=
          row_block_vector_->at(current_block_number_).numRows()) {
        ++current_block_number_;
        current_row_in_block_ = 0;
      }
    }
    Iterator operator+(int n) {
      Iterator it = *this;
      for (auto ii = 0; ii < n; ++ii) {
        ++it;
      }

      return it;
    }
    const Row dereference() const {
      return row_block_vector_->at(current_block_number_)
          .getRow(current_row_in_block_);
    }
    bool equal(const Iterator& other) const {
      return (
          row_block_vector_ == other.row_block_vector_ &&
          current_block_number_ == other.current_block_number_ &&
          current_row_in_block_ == other.current_row_in_block_);
    }

   private:
    const std::vector<RowBlock>* row_block_vector_;
    size_t current_block_number_;
    size_t current_row_in_block_;
  };

  Iterator begin() const {
    return Iterator(&row_blocks_, 0, 0);
  }

  Iterator end() const {
    return Iterator(&row_blocks_, row_blocks_.size(), 0);
  }

  void setRowFields(std::shared_ptr<RowFields> row_fields_info) {
    row_fields_info_ = std::move(row_fields_info);
  }

  void appendRowBlock(RowBlock&& block) {
    num_rows_ += block.numRows();
    row_blocks_.emplace_back(std::move(block));
  }

  void setPartialRows(RowBlock&& partial_row_blocks_) {
    row_blocks_.clear();
    num_rows_ = partial_row_blocks_.numRows();
    row_blocks_.emplace_back(std::move(partial_row_blocks_));
  }

  void setPartial(bool partial) {
    partial_ = partial;
  }

 private:
  void assertOnlyRow() const {
    CHECK_THROW(numRows() == 1, std::out_of_range);
  }
  std::shared_ptr<RowFields> row_fields_info_;
  int query_num_;
  bool partial_;
  bool was_slow_ = false;

  uint64_t num_rows_;
  uint64_t num_rows_affected_;
  uint64_t last_insert_id_;
  std::string recv_gtid_;
  RespAttrs resp_attrs_;

  OperationResult operation_result_;

  std::vector<RowBlock> row_blocks_;
};

class FetchOperation;
class MultiQueryStreamOperation;
class StreamedQueryResult;
class MultiQueryStreamHandler;
enum class StreamState;

/// The StreamedQueryResult support move assignment and construction, but
/// its unsafe to move assign in the middle of fetching results
class StreamedQueryResult {
 public:
  uint64_t numAffectedRows() {
    checkAccessToResult();
    return num_affected_rows_;
  }
  uint64_t lastInsertId() {
    // Will throw exception if there was an error
    checkAccessToResult();
    return last_insert_id_;
  }

  const std::string& recvGtid() {
    // Will throw exception if there was an error
    checkAccessToResult();
    return recv_gtid_;
  }

  using RespAttrs = AttributeMap;
  const RespAttrs& responseAttributes() {
    // Will throw exception if there was an error
    checkAccessToResult();
    return resp_attrs_;
  }

  class Iterator;

  class Iterator : public boost::iterator_facade<
                       Iterator,
                       const EphemeralRow,
                       boost::forward_traversal_tag> {
   public:
    Iterator(StreamedQueryResult* query_res, int row_num)
        : query_res_(query_res), row_num_(row_num) {}

    void increment();
    const EphemeralRow& dereference() const;

    bool equal(const Iterator& other) const {
      return row_num_ == other.row_num_;
    }

   private:
    friend class StreamedQueryResult;

    StreamedQueryResult* query_res_;
    folly::Optional<EphemeralRow> current_row_;
    int row_num_;
  };

  Iterator begin();

  Iterator end();

  StreamedQueryResult(MultiQueryStreamHandler* handler, size_t query_id_);
  ~StreamedQueryResult();

  StreamedQueryResult(const StreamedQueryResult&) = delete;
  StreamedQueryResult& operator=(StreamedQueryResult const&) = delete;

  // only move construction and assignment allowed
  StreamedQueryResult(StreamedQueryResult&& other) = default;

  StreamedQueryResult& operator=(StreamedQueryResult&& other) /* may throw */ {
    if (this != &other) {
      this->~StreamedQueryResult();
      new (this) StreamedQueryResult(std::move(other));
    }
    return *this;
  }

  EphemeralRowFields* getRowFields() const;

  folly::Optional<EphemeralRow> nextRow();

 private:
  friend class Iterator;
  friend class MultiQueryStreamHandler;

  void setResult(
      int64_t affected_rows,
      int64_t last_insert_id,
      const std::string& recv_gtid,
      const RespAttrs& resp_attrs);
  void setException(folly::exception_wrapper ex);
  void freeHandler();

  void checkStoredException();
  void checkAccessToResult();

  struct MultiQueryStreamHandlerDeleter {
    void operator()(void*) const {}
  };
  using MultiQueryStreamHandlerPtr =
      std::unique_ptr<MultiQueryStreamHandler, MultiQueryStreamHandlerDeleter>;

  MultiQueryStreamHandlerPtr stream_handler_ = nullptr;

  const size_t query_idx_;
  size_t num_rows_ = 0;
  int64_t num_affected_rows_ = -1;
  int64_t last_insert_id_ = 0;
  std::string recv_gtid_;
  RespAttrs resp_attrs_;

  folly::exception_wrapper exception_wrapper_;
};

class MultiQueryStreamHandler {
 public:
  // This allows the User Thread to know what to read from `FetchOperation`.
  // The states `InitResult`, `ReadRows`, `ReadResult` are synchronization
  // points. So the FetchOperation is going to be paused and will require
  // `resume` to proceed.
  enum class State {
    RunQuery,
    WaitForInitResult,
    InitResult,
    ReadRows,
    ReadResult,
    OperationSucceeded,
    OperationFailed,
  };

  static std::string toString(State state);

  explicit MultiQueryStreamHandler(
      std::shared_ptr<MultiQueryStreamOperation> op);

  ~MultiQueryStreamHandler();

  MultiQueryStreamHandler(const MultiQueryStreamHandler&) = delete;
  MultiQueryStreamHandler& operator=(const MultiQueryStreamHandler&) = delete;

  // NOTE: Its unsafe to move MultiQueryStreamHandler after invoking nextQuery
  // to retrieve the first query's result. Its safe to do this only before the
  // first invocation of nextQuery or after all queries are done.
  MultiQueryStreamHandler(MultiQueryStreamHandler&& other) noexcept;
  MultiQueryStreamHandler& operator=(MultiQueryStreamHandler&& other) {
    if (this != &other) {
      this->~MultiQueryStreamHandler();
      new (this) MultiQueryStreamHandler(std::move(other));
    }
    return *this;
  }

  // Returns the next Query or nullptr if there are no more query results to be
  // read.
  folly::Optional<StreamedQueryResult> nextQuery();

  std::unique_ptr<Connection> releaseConnection();

  // This is a dangerous function.  Please use it with utmost care.  It allows
  // someone to do something with the raw connection outside of the bounds of
  // this class.  We added it to support a specific use-case: TAO calls
  // escapeString will the connection is running a query.  Please do not use
  // this for other purposes.
  template <typename Func>
  auto accessConn(Func func) const {
    return func(connection());
  }

  unsigned int mysql_errno() const;
  const std::string& mysql_error() const;

 private:
  friend class Connection;
  friend class StreamedQueryResult;
  friend class FetchOperation;
  friend class MultiQueryStreamOperation;

  // This schedules the operation to be run and starts the result retreival
  // process
  void start();

  void streamCallback(FetchOperation* op, StreamState state);

  std::atomic<State> state_{State::RunQuery};

  // Provider to StreamedQueryResult call
  folly::Optional<EphemeralRow> fetchOneRow(StreamedQueryResult* result);

  void fetchQueryEnd(StreamedQueryResult* result);

  void resumeOperation();
  void handleQueryEnded(StreamedQueryResult* result);
  void handleQueryFailed(StreamedQueryResult* result);
  void handleBadState();

  // sanity checks on StreamedQueryResult
  void checkStreamedQueryResult(StreamedQueryResult* result);

  Connection* connection() const;

  folly::exception_wrapper exception_wrapper_;

  // Created in `NextQuery` and unique_ptr is returned to the user. The one
  // given to the user will only get destroyed when the query ends and we
  // clear this pointer.
  // This is only used when `StreamQueryResult` itself calls the Handler for
  // events.
  StreamedQueryResult* current_result_ = nullptr;

  size_t curr_query_ = 0; // the current query whose results we are fetching

  std::shared_ptr<MultiQueryStreamOperation> operation_;
};

typedef FetchResult<std::vector<QueryResult>> DbMultiQueryResult;
typedef FetchResult<QueryResult> DbQueryResult;
} // namespace mysql_client
} // namespace common
} // namespace facebook

#endif // COMMON_ASYNC_MYSQL_ROW_H
