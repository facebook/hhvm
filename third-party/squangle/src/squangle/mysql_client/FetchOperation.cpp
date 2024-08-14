/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/FetchOperation.h"
#include "squangle/mysql_client/ConnectOperation.h"
#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/MysqlHandler.h"

namespace {

const std::string kQueryChecksumKey = "checksum";

}

namespace facebook::common::mysql_client {

FetchOperation::FetchOperation(
    std::unique_ptr<ConnectionProxy> conn,
    std::vector<Query>&& queries)
    : Operation(std::move(conn)), queries_(std::move(queries)) {}

FetchOperation::FetchOperation(
    std::unique_ptr<ConnectionProxy> conn,
    MultiQuery&& multi_query)
    : Operation(std::move(conn)), queries_(std::move(multi_query)) {}

FetchOperation& FetchOperation::setUseChecksum(bool useChecksum) noexcept {
  use_checksum_ = useChecksum;
  return *this;
}

bool FetchOperation::isStreamAccessAllowed() const {
  // XOR if isPaused or the caller is coming from IO Thread
  return isPaused() || isInEventBaseThread();
}

bool FetchOperation::isPaused() const {
  return active_fetch_action_ == FetchAction::WaitForConsumer;
}

FetchOperation& FetchOperation::specializedRun() {
  if (!conn().runInThread([&]() { specializedRunImpl(); })) {
    completeOperationInner(OperationResult::Failed);
  }

  return *this;
}

void FetchOperation::specializedRunImpl() {
  try {
    rendered_query_ = queries_.renderQuery(&conn().getInternalConnection());

    if (auto ret = conn().setQueryAttributes(attributes_)) {
      setAsyncClientError(ret, "Failed to set query attributes");
      completeOperation(OperationResult::Failed);
      return;
    }

    if ((use_checksum_ || conn().getConnectionOptions().getUseChecksum())) {
      if (auto ret = conn().setQueryAttribute(kQueryChecksumKey, "ON")) {
        setAsyncClientError(ret, "Failed to set checksum = ON");
        completeOperation(OperationResult::Failed);
        return;
      }
    }

    actionable();
  } catch (std::invalid_argument& e) {
    setAsyncClientError(
        static_cast<uint16_t>(SquangleErrno::SQ_INVALID_API_USAGE),
        std::string("Unable to parse Query: ") + e.what());
    completeOperation(OperationResult::Failed);
  }
}

FetchOperation::RowStream::RowStream(
    std::unique_ptr<InternalResult> mysql_query_result,
    std::unique_ptr<InternalRowMetadata> metadata,
    MysqlHandler* handler)
    : mysql_query_result_(std::move(mysql_query_result)),
      row_fields_(std::make_shared<EphemeralRowFields>(std::move(metadata))),
      handler_(handler) {}

EphemeralRow FetchOperation::RowStream::consumeRow() {
  if (!current_row_.has_value()) {
    LOG(DFATAL) << "Illegal operation";
  }
  EphemeralRow eph_row(std::move(*current_row_));
  current_row_.reset();
  return eph_row;
}

bool FetchOperation::RowStream::hasNext() {
  // Slurp needs to happen after `consumeRow` has been called.
  // Because it will move the buffer.
  slurp();
  // First iteration
  return current_row_.has_value();
}

bool FetchOperation::RowStream::slurp() {
  CHECK_THROW(mysql_query_result_ != nullptr, db::OperationStateException);
  if (current_row_.has_value() || query_finished_) {
    return true;
  }
  auto [result, row] = handler_->fetchRow(*mysql_query_result_);
  if (result == PENDING) {
    return false;
  }

  if (row == nullptr) {
    query_finished_ = true;
    return true;
  }
  current_row_.assign(EphemeralRow(std::move(row), row_fields_));
  query_result_size_ += current_row_->calculateRowLength();
  ++num_rows_seen_;
  return true;
}

void FetchOperation::setFetchAction(FetchAction action) {
  if (isPaused()) {
    paused_action_ = action;
  } else {
    active_fetch_action_ = action;
  }
}

uint64_t FetchOperation::currentLastInsertId() const {
  CHECK_THROW(isStreamAccessAllowed(), db::OperationStateException);
  return current_last_insert_id_;
}

uint64_t FetchOperation::currentAffectedRows() const {
  CHECK_THROW(isStreamAccessAllowed(), db::OperationStateException);
  return current_affected_rows_;
}

const std::string& FetchOperation::currentRecvGtid() const {
  CHECK_THROW(isStreamAccessAllowed(), db::OperationStateException);
  return current_recv_gtid_;
}

const AttributeMap& FetchOperation::currentRespAttrs() const {
  CHECK_THROW(isStreamAccessAllowed(), db::OperationStateException);
  return current_resp_attrs_;
}

FetchOperation::RowStream* FetchOperation::rowStream() {
  CHECK_THROW(isStreamAccessAllowed(), db::OperationStateException);
  return current_row_stream_.get_pointer();
}

AttributeMap FetchOperation::readResponseAttributes() {
  return conn().getResponseAttributes();
}

void FetchOperation::actionable() {
  DCHECK(isInEventBaseThread());
  DCHECK(active_fetch_action_ != FetchAction::WaitForConsumer);

  folly::stop_watch<Duration> sw;
  auto logThreadBlockTimeGuard =
      folly::makeGuard([&]() { logThreadBlockTime(sw); });

  auto& handler = conn().client().getMysqlHandler();
  auto& internalConn = conn().getInternalConnection();

  // Add the socket to the FetchOperation so that we can wait on alerts
  changeHandlerFD(folly::NetworkSocket::fromFd(conn().getSocketDescriptor()));

  // This loop runs the fetch actions required to successfully execute query,
  // request next results, fetch results, identify errors and complete operation
  // and queries.
  // All callbacks are done in the `notify` methods that children must
  // override. During callbacks for actions `Fetch` and `CompleteQuery`,
  // the consumer is allowed to pause the operation.
  // Some actions may request an action above it (like CompleteQuery may request
  // StartQuery) this is why we use this loop.
  while (true) {
    // When the fetch action is StartQuery it means either we need to execute
    // the query or ask for new results.
    // Next Actions:
    //  - StartQuery: may continue with StartQuery if socket not actionable, in
    //                this case socketActionable is exited;
    //  - CompleteOperation: if it fails to execute query or request next
    //                       results.
    //  - InitFetch: no errors during results request, so we initiate fetch.
    if (active_fetch_action_ == FetchAction::StartQuery) {
      auto status = PENDING;

      if (query_executed_) {
        ++num_current_query_;
        status = handler.nextResult(internalConn);
      } else {
        status = handler.runQuery(internalConn, *rendered_query_);
      }

      if (status == PENDING) {
        waitForActionable();
        return;
      }

      current_last_insert_id_ = 0;
      current_affected_rows_ = 0;
      current_recv_gtid_ = std::string();
      query_executed_ = true;
      if (status == ERROR) {
        active_fetch_action_ = FetchAction::CompleteQuery;
      } else {
        active_fetch_action_ = FetchAction::InitFetch;
      }
    }

    // Prior fetch start we read the values that may indicate errors, rows to
    // fetch or not. The initialize from children classes is called either way
    // to signal that any other calls from now are regarding a new query.
    // Next Actions:
    //  - CompleteOperation: in case an error occurred
    //  - Fetch: there are rows to fetch in this query
    //  - CompleteQuery: no rows to fetch (complete query will read rowsAffected
    //                   and lastInsertId to add to result
    if (active_fetch_action_ == FetchAction::InitFetch) {
      auto mysql_query_result = handler.getResult(internalConn);
      auto num_fields = handler.getFieldCount(internalConn);

      // Check to see if this an empty query or an error
      if (!mysql_query_result && num_fields > 0) {
        // Failure. CompleteQuery will read errors.
        active_fetch_action_ = FetchAction::CompleteQuery;
      } else {
        if (num_fields > 0) {
          auto row_metadata = mysql_query_result->getRowMetadata();
          current_row_stream_.assign(RowStream(
              std::move(mysql_query_result),
              std::move(row_metadata),
              &handler));
          active_fetch_action_ = FetchAction::Fetch;
        } else {
          active_fetch_action_ = FetchAction::CompleteQuery;
        }
        notifyInitQuery();
      }
    }

    // This action is going to stick around until all rows are fetched or an
    // error occurs. When the RowStream is ready, we notify the subclasses for
    // them to consume it.
    // If `pause` is called during the callback and the stream is consumed then,
    // `row_stream_` is checked and we skip to the next action `CompleteQuery`.
    // If row_stream_ isn't ready, we wait for actionable state.
    // Next Actions:
    //  - Fetch: in case it needs to fetch more rows, we break the loop and wait
    //           for actionable to be called again
    //  - CompleteQuery: an error occurred or rows finished to fetch
    //  - WaitForConsumer: in case `pause` is called during `notifyRowsReady`
    if (active_fetch_action_ == FetchAction::Fetch) {
      DCHECK(current_row_stream_.has_value());
      // Try to catch when the user didn't pause or consumed the rows
      if (current_row_stream_->current_row_.has_value()) {
        // This should help
        LOG(ERROR) << "Rows not consumed. Perhaps missing `pause`?";
        cancel_ = true;
        active_fetch_action_ = FetchAction::CompleteQuery;
        continue;
      }

      // When the query finished, `is_ready` is true, but there are no rows.
      bool is_ready = current_row_stream_->slurp();
      if (!is_ready) {
        waitForActionable();
        break;
      }
      if (current_row_stream_->hasQueryFinished()) {
        active_fetch_action_ = FetchAction::CompleteQuery;
      } else {
        notifyRowsReady();
      }
    }

    // In case the query has at least started and finished by error or not,
    // here the final checks and data are gathered for the current query.
    // It checks if any errors occurred during query, and call children classes
    // to deal with their specialized query completion.
    // If `pause` is called, then `paused_action_` will be already `StartQuery`
    // or `CompleteOperation`.
    // Next Actions:
    //  - StartQuery: There are more results and children is not opposed to it.
    //                QueryOperation child sets to CompleteOperation, since it
    //                is not supposed to receive more than one result.
    //  - CompleteOperation: In case an error occurred during query or there are
    //                       no more results to read.
    //  - WaitForConsumer: In case `pause` is called during notification.
    if (active_fetch_action_ == FetchAction::CompleteQuery) {
      snapshotMysqlErrors();

      bool more_results = false;
      if (mysql_errno_ != 0 || cancel_) {
        active_fetch_action_ = FetchAction::CompleteOperation;
      } else {
        current_last_insert_id_ = conn().getLastInsertId();
        current_affected_rows_ = conn().getAffectedRows();
        if (auto optGtid = conn().getRecvGtid()) {
          current_recv_gtid_ = *optGtid;
        }
        if (auto optSchemaChanged = conn().getSchemaChanged()) {
          conn().setCurrentSchema(std::move(*optSchemaChanged));
        }
        current_resp_attrs_ = readResponseAttributes();
        more_results = conn().hasMoreResults();
        active_fetch_action_ = more_results ? FetchAction::StartQuery
                                            : FetchAction::CompleteOperation;

        // Call it after setting the active_fetch_action_ so the child class can
        // decide if it wants to change the state

        if (current_row_stream_ && current_row_stream_->mysql_query_result_) {
          rows_received_ += current_row_stream_->mysql_query_result_->numRows();
          total_result_size_ += current_row_stream_->query_result_size_;
        }
        ++num_queries_executed_;
        no_index_used_ |= conn().getNoIndexUsed();
        was_slow_ |= conn().wasSlow();
        notifyQuerySuccess(more_results);
      }
      current_row_stream_.reset();
    }

    // Once this action is set, the operation is going to be completed no matter
    // the reason it was called. It exists the loop.
    if (active_fetch_action_ == FetchAction::CompleteOperation) {
      logThreadBlockTimeGuard.dismiss();
      if (cancel_) {
        state_ = OperationState::Cancelling;
        completeOperation(OperationResult::Cancelled);
      } else if (mysql_errno_ != 0) {
        completeOperation(OperationResult::Failed);
      } else {
        completeOperation(OperationResult::Succeeded);
      }
      break;
    }

    // If `pause` is called during the operation callbacks, this the Action it
    // should come to.
    // It's not necessary to unregister the socket event,  so just cancel the
    // timeout and wait for `resume` to be called.
    if (active_fetch_action_ == FetchAction::WaitForConsumer) {
      cancelTimeout();
      break;
    }
  }
}

void FetchOperation::pauseForConsumer() {
  DCHECK(isInEventBaseThread());
  DCHECK(state() == OperationState::Pending);

  paused_action_ = active_fetch_action_;
  active_fetch_action_ = FetchAction::WaitForConsumer;
}

void FetchOperation::resumeImpl() {
  CHECK_THROW(isPaused(), db::OperationStateException);

  // We should only allow pauses during fetch or between queries.
  // If we come back as RowsFetched and the stream has completed the query,
  // `actionable` will change the `active_fetch_action_` and we will
  // start the Query completion process.
  // When we pause between queries, the value of `paused_action_` is already
  // the value of the next states: StartQuery or CompleteOperation.
  active_fetch_action_ = paused_action_;
  // Leave timeout to be reset or checked when we hit
  // `waitForActionable`
  actionable();
}

void FetchOperation::resume() {
  DCHECK(active_fetch_action_ == FetchAction::WaitForConsumer);
  conn().runInThread(this, &FetchOperation::resumeImpl);
}

void FetchOperation::specializedTimeoutTriggered() {
  DCHECK(active_fetch_action_ != FetchAction::WaitForConsumer);
  auto deltaUs = stopwatch_->elapsed();
  auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(deltaUs);

  if (conn().getKillOnQueryTimeout()) {
    killRunningQuery();
  }

  /*
   * The MYSQL_RES struct contains a handle to the MYSQL struct that created
   * it. Currently, calling mysql_free_result attempts to flush the buffer in
   * accordance with the protocol. This makes it so that if a MYSQL_RES is
   * freed during a query and before the entire result is read, then the
   * subsequent queries sent over the same connection will still succeed.
   *
   * In Operation.h it can be seen that mysql_free_result is used to delete
   * the result set, instead of the nonblocking version. The logic to flush
   * the socket is impossible to correctly implement in a destructor, because
   * the function needs to be called repeatedly to ensure all data has been
   * read. Instead we use the code below to detach the result object from the
   * connection, so no network flushing is done.
   *
   * This does not cause a memory leak because the socket will still be cleaned
   * up when the connection is freed. AsyncMySQL also does not provide a way
   * for clients to read half a result, then send more queries. If we allowed
   * partial reads of results, then this strategy would not work. The most
   * common case where we would normally need to flush results is for client
   * query timeouts, where we might still be receiving rows when we interrupt
   * and return an error to the client.
   */
  if (rowStream() && rowStream()->mysql_query_result_) {
    rowStream()->mysql_query_result_->close();
  }

  std::string rows;
  if (rowStream() && rowStream()->numRowsSeen()) {
    rows = fmt::format(
        "({} rows, {} bytes seen)",
        rowStream()->numRowsSeen(),
        rowStream()->query_result_size_);
  } else {
    rows = "(no rows seen)";
  }

  auto cbDelayUs = client().callbackDelayMicrosAvg();
  bool stalled = cbDelayUs >= kCallbackDelayStallThresholdUs;

  std::vector<std::string> parts;
  parts.push_back(fmt::format(
      "[{}]({}) Query timed out",
      static_cast<uint16_t>(
          stalled ? SquangleErrno::SQ_ERRNO_QUERY_TIMEOUT_LOOP_STALLED
                  : SquangleErrno::SQ_ERRNO_QUERY_TIMEOUT),
      kErrorPrefix));

  parts.push_back(std::move(rows));
  parts.push_back(timeoutMessage(deltaMs));
  if (stalled) {
    parts.push_back(threadOverloadMessage(cbDelayUs));
  }

  setAsyncClientError(CR_NET_READ_INTERRUPTED, folly::join(" ", parts));
  completeOperation(OperationResult::TimedOut);
}

void FetchOperation::specializedCompleteOperation() {
  // Stats for query
  if (result_ == OperationResult::Succeeded) {
    // set last successful query time to MysqlConnectionHolder
    conn().setLastActivityTime(Clock::now());
    db::QueryLoggingData logging_data(
        getOperationType(),
        elapsed(),
        timeout_,
        num_queries_executed_,
        rendered_query_,
        rows_received_,
        total_result_size_,
        no_index_used_,
        use_checksum_ || conn().getConnectionOptions().getUseChecksum(),
        attributes_,
        readResponseAttributes(),
        getMaxThreadBlockTime(),
        getTotalThreadBlockTime(),
        was_slow_);
    client().logQuerySuccess(logging_data, conn());
  } else {
    db::FailureReason reason = db::FailureReason::DATABASE_ERROR;
    if (result_ == OperationResult::Cancelled) {
      reason = db::FailureReason::CANCELLED;
    } else if (result_ == OperationResult::TimedOut) {
      reason = db::FailureReason::TIMEOUT;
    }
    client().logQueryFailure(
        db::QueryLoggingData(
            getOperationType(),
            elapsed(),
            timeout_,
            num_queries_executed_,
            rendered_query_,
            rows_received_,
            total_result_size_,
            no_index_used_,
            use_checksum_ || conn().getConnectionOptions().getUseChecksum(),
            attributes_,
            readResponseAttributes(),
            getMaxThreadBlockTime(),
            getTotalThreadBlockTime(),
            was_slow_),
        reason,
        mysql_errno(),
        mysql_error(),
        conn());
  }

  if (result_ != OperationResult::Succeeded) {
    notifyFailure(result_);
  }
  // This frees the `Operation::wait()` call. We need to free it here because
  // callback can stealConnection and we can't notify anymore.
  conn().notify();
  notifyOperationCompleted(result_);
}

void FetchOperation::mustSucceed() {
  run().wait();
  if (!ok()) {
    throw db::RequiredOperationFailedException("Query failed: " + mysql_error_);
  }
}

void FetchOperation::killRunningQuery() {
  /*
   * Send kill command to terminate the current operation on the DB
   * Note that we use KILL <processlist_id> to kill the entire connection
   * In the event the DB is behind a proxy this will kill the persistent
   * connection the proxy is using, so ConnectionOptions::killQueryOnTimeout_
   * should always be false when accessing the DB through a proxy
   *
   * Note that there is a risk of a race condition in the event that a proxy
   * is used and a query from this client times out, then the query completes
   * almost immediately after the timeout and a proxy gives the persistent
   * connection to another client which begins a query on that connection
   * before this client is able to send the KILL query on a separate
   * proxy->db connection which then terminates the OTHER client's query
   */
  auto thread_id = conn().mysqlThreadId();
  auto host = conn().host();
  auto port = conn().port();
  auto conn_op = client().beginConnection(conn().getKey());
  conn_op->setConnectionOptions(conn().getConnectionOptions());
  conn_op->setCallback([thread_id, host, port](ConnectOperation& conn_op) {
    if (conn_op.ok()) {
      auto op = Connection::beginQuery(
          conn_op.releaseConnection(), "KILL %d", thread_id);
      op->setCallback([thread_id, host, port](
                          QueryOperation& /* unused */,
                          QueryResult* /* unused */,
                          QueryCallbackReason reason) {
        if (reason == QueryCallbackReason::Failure) {
          LOG(WARNING) << fmt::format(
              "Failed to kill query in thread {} on {}:{}",
              thread_id,
              host,
              port);
        }
      });
      op->run();
    }
  });
  conn_op->run();
}

} // namespace facebook::common::mysql_client
