/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/Operation.h"
#include "squangle/mysql_client/Query.h"

namespace facebook::common::mysql_client {

class MysqlHandler;

// A fetching operation (query or multiple queries) use the same primary
// actions. This is an abstract base for this kind of operation.
// FetchOperation controls the flow of fetching a result:
//  - When there are rows to be read, it will identify it and call the
//  subclasses
// for them to consume the state;
//  - When there are no rows to be read or an error happened, proper
//  notifications
// will be made as well.
// This is the only Operation that can be paused, and the pause should only be
// called from within `notify` calls. That will allow another thread to read
// the state.
class FetchOperation : public Operation {
 public:
  using RespAttrs = AttributeMap;
  ~FetchOperation() override = default;
  void mustSucceed() override;

  // Number of queries that succeed to execute
  int numQueriesExecuted() {
    CHECK_THROW(state_ != OperationState::Pending, db::OperationStateException);
    return num_queries_executed_;
  }

  uint64_t resultSize() const {
    CHECK_THROW(
        state_ != OperationState::Unstarted, db::OperationStateException);
    return total_result_size_;
  }

  FetchOperation& setUseChecksum(bool useChecksum) noexcept;

  // This class encapsulates the operations and access to the MySQL ResultSet.
  // When the consumer receives a notification for RowsFetched, it should
  // consume `rowStream`:
  //   while (rowStream->hasNext()) {
  //     EphemeralRow row = consumeRow();
  //   }
  // The state within RowStream is also used for FetchOperation to know
  // whether or not to go to next query.
  class RowStream {
   public:
    RowStream(
        std::unique_ptr<InternalResult> mysql_query_result,
        std::unique_ptr<InternalRowMetadata> metadata,
        MysqlHandler* handler);

    EphemeralRow consumeRow();

    bool hasNext();

    EphemeralRowFields* getEphemeralRowFields() {
      return &*row_fields_;
    }

    ~RowStream() = default;
    RowStream(RowStream&&) = default;
    RowStream& operator=(RowStream&&) = default;

   private:
    friend class FetchOperation;
    bool slurp();
    // user shouldn't take information from this
    bool hasQueryFinished() {
      return query_finished_;
    }
    uint64_t numRowsSeen() const {
      return num_rows_seen_;
    }

    bool query_finished_ = false;
    uint64_t num_rows_seen_ = 0;
    uint64_t query_result_size_ = 0;

    // All memory lifetime is guaranteed by FetchOperation.
    std::unique_ptr<InternalResult> mysql_query_result_;
    folly::Optional<EphemeralRow> current_row_;
    std::shared_ptr<EphemeralRowFields> row_fields_;
    MysqlHandler* handler_ = nullptr;
  };

  // Streaming calls. Should only be called when using the StreamCallback.
  // TODO#10716355: We shouldn't let these functions visible for non-stream
  // mode. Leaking for tests.
  uint64_t currentLastInsertId() const;
  uint64_t currentAffectedRows() const;
  const std::string& currentRecvGtid() const;
  const RespAttrs& currentRespAttrs() const;

  bool noIndexUsed() const {
    return no_index_used_;
  }

  bool wasSlow() const {
    return was_slow_;
  }

  int numCurrentQuery() const {
    return num_current_query_;
  }

  RowStream* rowStream();

  // Stalls the FetchOperation until `resume` is called.
  // This is used to allow another thread to access the socket functions.
  void pauseForConsumer();

  // Resumes the operation to the action it was before `pause` was called.
  // Should only be called after pause.
  void resume();

  int rows_received_ = 0;

 protected:
  MultiQuery queries_;

  FetchOperation& specializedRun() override;

  FetchOperation(
      std::unique_ptr<ConnectionProxy> conn,
      std::vector<Query>&& queries);
  FetchOperation(
      std::unique_ptr<ConnectionProxy> conn,
      MultiQuery&& multi_query);

  enum class FetchAction {
    StartQuery,
    InitFetch,
    Fetch,
    WaitForConsumer,
    CompleteQuery,
    CompleteOperation
  };

  void setFetchAction(FetchAction action);
  static folly::StringPiece toString(FetchAction action);

  // In socket actionable it is analyzed the action that is required to
  // continue the operation. For example, if the fetch action is StartQuery,
  // it runs query or requests more results depending if it had already ran or
  // not the query. The same process happens for the other FetchActions. The
  // action member can be changed in other member functions called in
  // socketActionable to keep the fetching flow running.
  void socketActionable() override;
  void specializedTimeoutTriggered() override;
  void specializedCompleteOperation() override;

  // Overridden in child classes and invoked when the Query fetching
  // has done specific actions that might be needed for report (callbacks,
  // store fetched data, initialize data).
  virtual void notifyInitQuery() = 0;
  virtual void notifyRowsReady() = 0;
  virtual void notifyQuerySuccess(bool more_results) = 0;
  virtual void notifyFailure(OperationResult result) = 0;
  virtual void notifyOperationCompleted(OperationResult result) = 0;

  bool cancel_ = false;

 private:
  friend class MultiQueryStreamHandler;
  void specializedRunImpl();

  void resumeImpl();
  // Checks if the current thread has access to stream, or result data.
  bool isStreamAccessAllowed() const;
  bool isPaused() const;

  // Read the response attributes
  RespAttrs readResponseAttributes();

  // Asynchronously kill a currently running query, returns
  // before the query is killed
  void killRunningQuery();

  // Current query data
  folly::Optional<RowStream> current_row_stream_;
  bool query_executed_ = false;
  bool no_index_used_ = false;
  bool use_checksum_ = false;
  bool was_slow_ = false;
  // TODO: Rename `executed` to `succeeded`
  int num_queries_executed_ = 0;
  // During a `notify` call, the consumer might want to know the index of the
  // current query, that's what `num_current_query_` is counting.
  int num_current_query_ = 0;
  // Best effort attempt to calculate the size of the result set in bytes.
  // Only counts the actual data in rows, not bytes sent over the wire, and
  // doesn't include column/table metadata or mysql packet overhead
  uint64_t total_result_size_ = 0;

  uint64_t current_affected_rows_ = 0;
  uint64_t current_last_insert_id_ = 0;
  std::string current_recv_gtid_;
  RespAttrs current_resp_attrs_;

  // When the Fetch gets paused, active fetch action moves to
  // `WaitForConsumer` and the action that got paused gets saved so tat
  // `resume` can set it properly afterwards.
  FetchAction active_fetch_action_ = FetchAction::StartQuery;
  FetchAction paused_action_ = FetchAction::StartQuery;

  std::shared_ptr<folly::fbstring> rendered_query_;
};

} // namespace facebook::common::mysql_client
