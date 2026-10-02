/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/ExceptionWrapper.h>
#include <folly/coro/Task.h>
#include <folly/synchronization/Baton.h>
#include <folly/synchronization/DelayedInit.h>

#include "squangle/mysql_client/AsyncPipeUsingGenerator.h"
#include "squangle/mysql_client/MysqlClientBase.h"
#include "squangle/mysql_client/Row.h"

namespace facebook::common::mysql_client {

class Connection;
class FetchOperation;
class MultiQueryStreamOperation;
class MultiQueryStreamHandler;
class Query;

enum class StreamState;

/// The StreamedQueryResult support move assignment
/// its unsafe to move assign in the middle of fetching results
class StreamedQueryResult {
 public:
  using RespAttrs = AttributeMap;

  explicit StreamedQueryResult(
      std::shared_ptr<RowFields> row_fields,
      bool hasDataInNativeFormat);
  ~StreamedQueryResult();

  StreamedQueryResult(const StreamedQueryResult&) = delete;
  StreamedQueryResult& operator=(StreamedQueryResult const&) = delete;

  StreamedQueryResult(StreamedQueryResult&& other) = delete;
  StreamedQueryResult& operator=(StreamedQueryResult&& other) = delete;

  uint64_t numAffectedRows();

  uint64_t lastInsertId();

  const std::string& recvGtid();

  const RespAttrs& responseAttributes();

  unsigned int warningsCount();

  class Iterator : public boost::iterator_facade<
                       Iterator,
                       const EphemeralRow,
                       boost::forward_traversal_tag> {
   public:
    Iterator(StreamedQueryResult* query_res, ssize_t row_num)
        : query_res_(query_res), row_num_(row_num) {}

    void increment();
    const EphemeralRow& dereference() const;

    bool equal(const Iterator& other) const {
      return row_num_ == other.row_num_;
    }

   private:
    friend class StreamedQueryResult;

    StreamedQueryResult* query_res_;
    std::optional<EphemeralRow> current_row_;
    ssize_t row_num_;
  };

  Iterator begin();

  Iterator end();

  const std::shared_ptr<RowFields>& getRowFields() const;

  std::optional<EphemeralRow> nextRow();
  folly::coro::Task<std::optional<EphemeralRow>> co_nextRow();

  bool hasDataInNativeFormat() const noexcept {
    return hasDataInNativeFormat_;
  }

 private:
  friend class Iterator;
  friend class MultiQueryStreamHandler;

  // Data about the query
  struct FinalData {
    int64_t num_affected_rows = 0;
    int64_t last_insert_id = 0;
    std::string recv_gtid;
    RespAttrs resp_attrs;
    unsigned int warnings_count = 0;

    FinalData(
        int64_t num_affected_rows,
        int64_t last_insert_id,
        std::string recv_gtid,
        RespAttrs resp_attrs,
        unsigned int warnings_count)
        : num_affected_rows{num_affected_rows},
          last_insert_id{last_insert_id},
          recv_gtid{std::move(recv_gtid)},
          resp_attrs{std::move(resp_attrs)},
          warnings_count{warnings_count} {}
  };

  const FinalData& checkFinalData() const;

  void setFinalData(FinalData final_data);

  void drain();
  folly::coro::Task<void> co_drain();

  AsyncPipeUsingGenerator<EphemeralRow> pipe_;

  folly::DelayedInit<FinalData> final_data_;
  std::shared_ptr<RowFields> row_fields_;
  folly::DelayedInit<folly::exception_wrapper> exception_;

  // Back-pointer to the owning handler.  StreamedQueryResult delegates
  // row-level operations (nextRow, drain) to virtual methods on the
  // handler, which each derived class implements using its own mechanism
  // (pipe or direct-from-operation).
  MultiQueryStreamHandler* handler_{nullptr};

  bool hasDataInNativeFormat_;
};

class MultiQueryStreamHandler {
 public:
  virtual ~MultiQueryStreamHandler();

  MultiQueryStreamHandler(const MultiQueryStreamHandler&) = delete;
  MultiQueryStreamHandler& operator=(const MultiQueryStreamHandler&) = delete;

  MultiQueryStreamHandler(MultiQueryStreamHandler&& other) noexcept = delete;
  MultiQueryStreamHandler& operator=(MultiQueryStreamHandler&& other) = delete;

  static std::unique_ptr<MultiQueryStreamHandler> create(
      MysqlClientBase& client,
      std::shared_ptr<MultiQueryStreamOperation> op);

  // Returns the next Query or nullptr if there are no more query results to
  // be read.
  virtual std::shared_ptr<StreamedQueryResult> nextQuery() = 0;
  virtual folly::coro::Task<std::shared_ptr<StreamedQueryResult>>
  co_nextQuery() = 0;

  virtual void drain() = 0;

  std::unique_ptr<Connection> releaseConnection();

  // This is a dangerous function.  Please use it with utmost care.  It allows
  // someone to do something with the raw connection outside of the bounds of
  // this class.  We added it to support a specific use-case: TAO calls
  // escapeString will the connection is running a query.  Please do not use
  // this for other purposes.
  template <typename Func>
  auto& accessConn(Func func) const {
    return func(connection());
  }

  unsigned int mysql_errno() const;
  const std::string& mysql_error() const;

 protected:
  friend class StreamedQueryResult;
  friend class MultiQueryStreamOperation;
  friend class Connection;

  MultiQueryStreamHandler(
      MysqlClientBase& client,
      std::shared_ptr<MultiQueryStreamOperation> op);

  Connection& connection() const;

  const QueryException& asQueryException() const;

  // Start the operation.
  void start();

  virtual void streamCallback(FetchOperation& op, StreamState state) = 0;

  // Row-level interface: StreamedQueryResult delegates here instead of
  // branching on handler type.  Each derived class implements these
  // using its own row-delivery mechanism.
  virtual std::optional<EphemeralRow> fetchOneRow(
      StreamedQueryResult& result) = 0;
  virtual folly::coro::Task<std::optional<EphemeralRow>> co_fetchOneRow(
      StreamedQueryResult& result) = 0;
  virtual void drainCurrentResult(StreamedQueryResult& result) = 0;

  bool hasDataInNativeFormat() const;

  // Helpers for derived-class streamCallback() implementations.
  // These delegate to private members of StreamedQueryResult that are
  // accessible via the base class's friendship.
  void initBackendResult(FetchOperation& op);
  static void drainResult(StreamedQueryResult& result);

  // Accessors for derived-class row-level implementations.
  // C++ friendship is not inherited, so derived classes use these to
  // access StreamedQueryResult's private members.
  void clearResultHandler(StreamedQueryResult& result);
  void setResultException(
      StreamedQueryResult& result,
      folly::exception_wrapper ex);
  void setResultFinalData(
      StreamedQueryResult& result,
      int64_t num_affected_rows,
      int64_t last_insert_id,
      std::string recv_gtid,
      AttributeMap resp_attrs,
      unsigned int warnings_count);
  AsyncPipeUsingGenerator<EphemeralRow>& resultRowPipe(
      StreamedQueryResult& result);
  const folly::DelayedInit<folly::exception_wrapper>& resultException(
      StreamedQueryResult& result);

  // Common members
  std::shared_ptr<MultiQueryStreamOperation> operation_;
  folly::DelayedInit<folly::exception_wrapper> exception_;

  std::shared_ptr<StreamedQueryResult> current_user_result_;
  std::shared_ptr<StreamedQueryResult> current_backend_result_;

  MysqlClientBase& client_;
  folly::Baton<> in_callback_baton_;
  folly::Baton<> op_done_baton_;
};

} // namespace facebook::common::mysql_client
