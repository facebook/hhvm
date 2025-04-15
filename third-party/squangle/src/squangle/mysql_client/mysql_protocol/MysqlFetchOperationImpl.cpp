/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/mysql_protocol/MysqlFetchOperationImpl.h"
#include "squangle/mysql_client/ConnectOperation.h"
#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/MysqlClientBase.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnection.h"
#include "squangle/mysql_client/mysql_protocol/MysqlResult.h"

namespace {
const std::string kQueryChecksumKey = "checksum";
}

namespace facebook::common::mysql_client::mysql_protocol {

bool MysqlFetchOperationImpl::isStreamAccessAllowed() const {
  // XOR if isPaused or the caller is coming from IO Thread
  return isPaused() || isInEventBaseThread();
}

bool MysqlFetchOperationImpl::isPaused() const {
  return getActiveFetchAction() == FetchAction::WaitForConsumer;
}

void MysqlFetchOperationImpl::specializedRun() {
  if (!conn().runInThread([&]() { specializedRunImpl(); })) {
    completeOperationInner(OperationResult::Failed);
  }
}

void MysqlFetchOperationImpl::specializedRunImpl() {
  try {
    rendered_query_ = queries().renderQuery(&getInternalConnection());

    auto* mysql_conn = getMysqlConnection();
    if (auto ret = mysql_conn->setQueryAttributes(getAttributes())) {
      getOp().setAsyncClientError(ret, "Failed to set query attributes");
      completeOperation(OperationResult::Failed);
      return;
    }

    if ((use_checksum_ || conn().getConnectionOptions().getUseChecksum())) {
      if (auto ret = mysql_conn->setQueryAttribute(kQueryChecksumKey, "ON")) {
        getOp().setAsyncClientError(ret, "Failed to set checksum = ON");
        completeOperation(OperationResult::Failed);
        return;
      }
    }

    actionable();
  } catch (std::invalid_argument& e) {
    getOp().setAsyncClientError(
        static_cast<uint16_t>(SquangleErrno::SQ_INVALID_API_USAGE),
        std::string("Unable to parse Query: ") + e.what());
    completeOperation(OperationResult::Failed);
  }
}

void MysqlFetchOperationImpl::actionable() {
  DCHECK(isInEventBaseThread());
  DCHECK(getActiveFetchAction() != FetchAction::WaitForConsumer);

  folly::stop_watch<Duration> sw;
  auto logThreadBlockTimeGuard =
      folly::makeGuard([&]() { logThreadBlockTime(sw); });

  FetchOperation& op = getOp();

  const auto* mysql_conn = getMysqlConnection();

  // Add the socket to the FetchOperation so that we can wait on alerts
  changeHandlerFD(
      folly::NetworkSocket::fromFd(mysql_conn->getSocketDescriptor()));

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
    if (getActiveFetchAction() == FetchAction::StartQuery) {
      auto status = PENDING;

      if (query_executed_) {
        ++num_current_query_;
        status = mysql_conn->nextResult();
      } else {
        status = mysql_conn->runQuery(*rendered_query_);
      }

      if (status == PENDING) {
        waitForActionable();
        return;
      }

      current_last_insert_id_ = 0;
      current_affected_rows_ = 0;
      current_warnings_count_ = 0;
      current_recv_gtid_ = std::string();
      current_mysql_info_ = std::nullopt;
      current_rows_matched_ = std::nullopt;

      query_executed_ = true;
      if (status == ERROR) {
        setActiveFetchAction(FetchAction::CompleteQuery);
      } else {
        setActiveFetchAction(FetchAction::InitFetch);
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
    if (getActiveFetchAction() == FetchAction::InitFetch) {
      auto mysql_query_result = mysql_conn->getResult();
      auto num_fields = mysql_conn->getFieldCount();

      // Check to see if this an empty query or an error
      if (!mysql_query_result && num_fields > 0) {
        // Failure. CompleteQuery will read errors.
        setActiveFetchAction(FetchAction::CompleteQuery);
      } else {
        if (num_fields > 0) {
          auto row_metadata = mysql_query_result->getRowMetadata();
          current_row_stream_.assign(RowStream(
              std::move(mysql_query_result), std::move(row_metadata)));
          setActiveFetchAction(FetchAction::Fetch);
        } else {
          setActiveFetchAction(FetchAction::CompleteQuery);
        }
        op.notifyInitQuery();
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
    if (getActiveFetchAction() == FetchAction::Fetch) {
      if (!current_row_stream_) {
        setActiveFetchAction(FetchAction::CompleteQuery);
        continue;
      }

      // Try to catch when the user didn't pause or consumed the rows
      if (current_row_stream_->getCurrentRow().has_value()) {
        // This should help
        LOG(ERROR) << "Rows not consumed. Perhaps missing `pause`?";
        cancelOp();
        continue;
      }

      // When the query finished, `slurp` returns true, but there are no rows.
      if (!slurp()) {
        waitForActionable();
        break;
      }
      if (hasQueryFinished()) {
        setActiveFetchAction(FetchAction::CompleteQuery);
      } else {
        op.notifyRowsReady();
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
    if (getActiveFetchAction() == FetchAction::CompleteQuery) {
      getOp().snapshotMysqlErrors(
          mysql_conn->getErrno(), mysql_conn->getErrorMessage());

      bool more_results = false;
      if (mysql_errno() != 0 || cancel_) {
        setActiveFetchAction(FetchAction::CompleteOperation);
      } else {
        current_last_insert_id_ = mysql_conn->getLastInsertId();
        current_affected_rows_ = mysql_conn->getAffectedRows();
        current_mysql_info_ = mysql_conn->getMySQLInfo();
        if (current_mysql_info_ && !current_mysql_info_->empty()) {
          int matched_count = 0;
          // Do a regex match on current_mysql_info_ to get the matched count
          // for the query. This is used to calculate the number of rows
          // affected by the query
          static const RE2 rowsMatchedRegex("Rows matched: (\\d+)\\s+");
          if (re2::RE2::PartialMatch(
                  *current_mysql_info_, rowsMatchedRegex, &matched_count)) {
            current_rows_matched_ = matched_count;
          }
        }
        current_warnings_count_ = mysql_conn->warningCount();
        if (auto optGtid = mysql_conn->getRecvGtid()) {
          current_recv_gtid_ = *optGtid;
        }
        if (auto optSchemaChanged = mysql_conn->getSchemaChanged()) {
          conn().setCurrentSchema(std::move(*optSchemaChanged));
        }
        current_resp_attrs_ = readResponseAttributes();
        more_results = mysql_conn->hasMoreResults();
        setActiveFetchAction(
            more_results ? FetchAction::StartQuery
                         : FetchAction::CompleteOperation);

        // Call it after setting the active fetch action so the child class can
        // decide if it wants to change the state

        if (current_row_stream_) {
          rows_received_ += current_row_stream_->numRows();
          total_result_size_ += current_row_stream_->queryResultSize();
        }
        ++num_queries_executed_;
        no_index_used_ |= mysql_conn->getNoIndexUsed();
        was_slow_ |= mysql_conn->wasSlow();
        if (!op.notifyQuerySuccess(more_results)) {
          // This usually means a multi-query was passed to the single query API
          setActiveFetchAction(FetchAction::CompleteOperation);
          return;
        }
      }
      current_row_stream_.reset();
    }

    // Once this action is set, the operation is going to be completed no matter
    // the reason it was called. It exists the loop.
    if (getActiveFetchAction() == FetchAction::CompleteOperation) {
      logThreadBlockTimeGuard.dismiss();
      if (cancel_) {
        setState(OperationState::Cancelling);
        completeOperation(OperationResult::Cancelled);
      } else if (mysql_errno() != 0) {
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
    if (getActiveFetchAction() == FetchAction::WaitForConsumer) {
      cancelTimeout();
      break;
    }
  }
}

void MysqlFetchOperationImpl::pauseForConsumer() {
  DCHECK(isInEventBaseThread());
  DCHECK(state() == OperationState::Pending);

  paused_action_ = exchangeActiveFetchAction(FetchAction::WaitForConsumer);
}

void MysqlFetchOperationImpl::resumeImpl() {
  if (getActiveFetchAction() == FetchAction::WaitForConsumer) {
    // We should only allow pauses during fetch or between queries.
    // If we come back as RowsFetched and the stream has completed the query,
    // `actionable` will change the `active_fetch_action_` and we will
    // start the Query completion process.
    // When we pause between queries, the value of `paused_action_` is already
    // the value of the next states: StartQuery or CompleteOperation.
    setActiveFetchAction(paused_action_);
    // Leave timeout to be reset or checked when we hit
    // `waitForActionable`
    actionable();
  }
}

void MysqlFetchOperationImpl::resume() {
  conn().runInThread(this, &MysqlFetchOperationImpl::resumeImpl);
}

void MysqlFetchOperationImpl::specializedTimeoutTriggered() {
  DCHECK(getActiveFetchAction() != FetchAction::WaitForConsumer);
  auto delta = opElapsedMs();

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
  if (rowStream()) {
    rowStream()->close();
  }

  std::string rows;
  if (rowStream() && rowStream()->numRowsSeen()) {
    rows = fmt::format(
        "({} rows, {} bytes seen)",
        rowStream()->numRowsSeen(),
        rowStream()->queryResultSize());
  } else {
    rows = "(no rows seen)";
  }

  auto errorStr = generateTimeoutError(std::move(rows), delta);

  getOp().setAsyncClientError(CR_NET_READ_INTERRUPTED, errorStr);
  completeOperation(OperationResult::TimedOut);
}

void MysqlFetchOperationImpl::specializedCompleteOperation() {
  FetchOperation& op = getOp();

  auto& connection = conn();

  // Get all logging information
  db::QueryLoggingData logging_data(
      getOp().getOperationType(),
      opElapsed(),
      getTimeout(),
      num_queries_executed_,
      rendered_query_,
      rows_received_,
      total_result_size_,
      connection.serverInfo(),
      no_index_used_,
      use_checksum_ || connection.getConnectionOptions().getUseChecksum(),
      getAttributes(),
      readResponseAttributes(),
      getMaxThreadBlockTime(),
      getTotalThreadBlockTime(),
      was_slow_);

  // Stats for query
  if (result() == OperationResult::Succeeded) {
    // set last successful query time to MysqlConnectionHolder
    connection.setLastActivityTime(Clock::now());
    client_.logQuerySuccess(logging_data, connection);
  } else {
    auto reason = operationResultToFailureReason(result());
    client_.logQueryFailure(
        logging_data, reason, mysql_errno(), mysql_error(), connection);
  }

  if (result() != OperationResult::Succeeded) {
    op.notifyFailure(result());
  }
  // This frees the `Operation::wait()` call. We need to free it here because
  // callback can stealConnection and we can't notify anymore.
  connection.notify();
  op.notifyOperationCompleted(result());
}

void MysqlFetchOperationImpl::killRunningQuery() {
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
  auto conn_op = client_.beginConnection(conn().getKey());
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

} // namespace facebook::common::mysql_client::mysql_protocol
