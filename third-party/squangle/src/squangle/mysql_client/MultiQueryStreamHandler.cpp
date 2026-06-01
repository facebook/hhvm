/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/coro/BlockingWait.h>

#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/MultiQueryStreamHandler.h"
#include "squangle/mysql_client/MultiQueryStreamOperation.h"

using namespace std::chrono_literals;

namespace facebook::common::mysql_client {

// ===========================================================================
// StreamedQueryResult
// ===========================================================================

StreamedQueryResult::StreamedQueryResult(
    std::shared_ptr<RowFields> row_fields,
    bool hasDataInNativeFormat)
    : row_fields_(std::move(row_fields)),
      hasDataInNativeFormat_(hasDataInNativeFormat) {}

StreamedQueryResult::~StreamedQueryResult() {
  if (!final_data_) {
    // Consume any left over data
    drain();
  }
}

uint64_t StreamedQueryResult::numAffectedRows() {
  drain();
  checkFinalData();
  return final_data_->num_affected_rows;
}

uint64_t StreamedQueryResult::lastInsertId() {
  drain();
  checkFinalData();
  return final_data_->last_insert_id;
}

const std::string& StreamedQueryResult::recvGtid() {
  drain();
  checkFinalData();
  return final_data_->recv_gtid;
}

using RespAttrs = AttributeMap;
const RespAttrs& StreamedQueryResult::responseAttributes() {
  drain();
  checkFinalData();
  return final_data_->resp_attrs;
}

unsigned int StreamedQueryResult::warningsCount() {
  drain();
  checkFinalData();
  return final_data_->warnings_count;
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
  if (row_num_ == -1) {
    throw std::runtime_error("Accessing invalid StreamedQueryResult iterator");
  }

  auto row = query_res_->nextRow();
  if (row) {
    ++row_num_;
    current_row_ = std::move(row);
  } else {
    row_num_ = -1;
  }
}

const EphemeralRow& StreamedQueryResult::Iterator::dereference() const {
  if (row_num_ == -1) {
    throw std::runtime_error("Accessing invalid StreamedQueryResult iterator");
  }

  return *current_row_;
}

const std::shared_ptr<RowFields>& StreamedQueryResult::getRowFields() const {
  return row_fields_;
}

std::optional<EphemeralRow> StreamedQueryResult::nextRow() {
  if (!handler_) {
    return std::nullopt;
  }
  return handler_->fetchOneRow(*this);
}

folly::coro::Task<std::optional<EphemeralRow>>
StreamedQueryResult::co_nextRow() {
  if (!handler_) {
    co_return std::nullopt;
  }
  co_return co_await handler_->co_fetchOneRow(*this);
}

void StreamedQueryResult::checkFinalData() const {
  if (exception_) {
    throw *exception_;
  }

  if (!final_data_) {
    throw std::runtime_error(
        "Accessing status variables before query has completed successfully");
  }
}

void StreamedQueryResult::setFinalData(FinalData final_data) {
  final_data_.emplace(std::move(final_data));
}

folly::coro::Task<void> StreamedQueryResult::co_drain() {
  while (!exception_) {
    auto retTry = co_await folly::coro::co_awaitTry(co_nextRow());
    if (retTry.hasException() || !*retTry) {
      break;
    }
  }
}

void StreamedQueryResult::drain() {
  if (final_data_) {
    return;
  }

  if (handler_) {
    handler_->drainCurrentResult(*this);
  }
}

// ===========================================================================
// MultiQueryStreamHandler
// ===========================================================================

MultiQueryStreamHandler::MultiQueryStreamHandler(
    MysqlClientBase& client,
    std::shared_ptr<MultiQueryStreamOperation> op)
    : operation_(std::move(op)), client_(client) {
  CHECK(operation_);
}

MultiQueryStreamHandler::~MultiQueryStreamHandler() {
  op_done_baton_.wait();
}

void MultiQueryStreamHandler::start() {
  operation_->setCallback([&](FetchOperation& op, StreamState state) {
    this->streamCallback(op, state);
  });
  operation_->run();
  op_done_baton_.post();
}

std::unique_ptr<Connection> MultiQueryStreamHandler::releaseConnection() {
  drain();
  return operation_->releaseConnection();
}

unsigned int MultiQueryStreamHandler::mysql_errno() const {
  return operation_->mysql_errno();
}

const std::string& MultiQueryStreamHandler::mysql_error() const {
  return operation_->mysql_error();
}

Connection& MultiQueryStreamHandler::connection() const {
  return *operation_->connection();
}

bool MultiQueryStreamHandler::hasDataInNativeFormat() const {
  return operation_->hasDataInNativeFormat();
}

/* static */
void MultiQueryStreamHandler::drainResult(StreamedQueryResult& result) {
  result.drain();
}

void MultiQueryStreamHandler::initBackendResult(FetchOperation& op) {
  DCHECK(!current_backend_result_);
  auto* stream = op.rowStream();
  std::shared_ptr<RowFields> row_fields;
  if (stream) {
    row_fields = stream->getEphemeralRowFieldsShared()->makeBufferedFields();
  }
  current_backend_result_ = std::make_shared<StreamedQueryResult>(
      std::move(row_fields), hasDataInNativeFormat());
  current_backend_result_->handler_ = this;
}

void MultiQueryStreamHandler::clearResultHandler(StreamedQueryResult& result) {
  result.handler_ = nullptr;
}

void MultiQueryStreamHandler::setResultException(
    StreamedQueryResult& result,
    std::unique_ptr<QueryException> ex) {
  result.exception_ = std::move(ex);
}

void MultiQueryStreamHandler::setResultFinalData(
    StreamedQueryResult& result,
    int64_t num_affected_rows,
    int64_t last_insert_id,
    std::string recv_gtid,
    AttributeMap resp_attrs,
    unsigned int warnings_count) {
  result.setFinalData(
      StreamedQueryResult::FinalData(
          num_affected_rows,
          last_insert_id,
          std::move(recv_gtid),
          std::move(resp_attrs),
          warnings_count));
}

AsyncPipeUsingGenerator<EphemeralRow>& MultiQueryStreamHandler::resultRowPipe(
    StreamedQueryResult& result) {
  return result.pipe_;
}

std::unique_ptr<QueryException>& MultiQueryStreamHandler::resultException(
    StreamedQueryResult& result) {
  return result.exception_;
}

// ===========================================================================
// Derived class definitions (anonymous namespace)
// ===========================================================================

namespace {

class SyncMultiQueryStreamHandler : public MultiQueryStreamHandler {
  using MultiQueryStreamHandler::MultiQueryStreamHandler;
  ~SyncMultiQueryStreamHandler() override {
    drain();
    // Wait for the callback thread to finish before destroying members.
    in_callback_baton_.wait();
  }
  SyncMultiQueryStreamHandler(SyncMultiQueryStreamHandler&&) = delete;
  SyncMultiQueryStreamHandler& operator=(SyncMultiQueryStreamHandler&&) =
      delete;

  void streamCallback(FetchOperation& op, StreamState state) override;
  std::shared_ptr<StreamedQueryResult> nextQuery() override;
  folly::coro::Task<std::shared_ptr<StreamedQueryResult>> co_nextQuery()
      override;
  void drain() override;
  std::optional<EphemeralRow> fetchOneRow(StreamedQueryResult& result) override;
  folly::coro::Task<std::optional<EphemeralRow>> co_fetchOneRow(
      StreamedQueryResult& result) override;
  void drainCurrentResult(StreamedQueryResult& result) override;
  std::shared_ptr<StreamedQueryResult> syncNextQuery();
  void syncResumeCallback();

  enum class SyncState {
    WaitingForData,
    InitQuery,
    RowsReady,
    QueryEnded,
    Success,
    Failure,
  };

  bool sync_done_{false};
  std::atomic<SyncState> sync_state_{SyncState::WaitingForData};
  folly::Baton<> data_ready_baton_;
};

class AsyncMultiQueryStreamHandler : public MultiQueryStreamHandler {
  using MultiQueryStreamHandler::MultiQueryStreamHandler;
  ~AsyncMultiQueryStreamHandler() override {
    drain();
    // Wait for the callback thread to finish before destroying pipe_.
    // This prevents a data race where the callback thread is still in
    // closeWriter() when we start destroying our members.
    in_callback_baton_.wait();
  }
  AsyncMultiQueryStreamHandler(AsyncMultiQueryStreamHandler&&) = delete;
  AsyncMultiQueryStreamHandler& operator=(AsyncMultiQueryStreamHandler&&) =
      delete;

  void streamCallback(FetchOperation& op, StreamState state) override;
  std::shared_ptr<StreamedQueryResult> nextQuery() override;
  folly::coro::Task<std::shared_ptr<StreamedQueryResult>> co_nextQuery()
      override;
  void drain() override;
  std::optional<EphemeralRow> fetchOneRow(StreamedQueryResult& result) override;
  folly::coro::Task<std::optional<EphemeralRow>> co_fetchOneRow(
      StreamedQueryResult& result) override;
  void drainCurrentResult(StreamedQueryResult& result) override;

  folly::coro::Task<void> co_drainHandler();
  void initAndPublishResult(FetchOperation& op);
  void writeRowToResult(EphemeralRow row);
  void finalizeQuery(FetchOperation& op);
  void finalizeSuccess();
  void finalizeFailure(FetchOperation& op);

  // Query-level pipe for delivering StreamedQueryResult objects.
  AsyncPipeUsingGenerator<std::shared_ptr<StreamedQueryResult>> pipe_;

  // Resume callback for paused operation.  Starts as nullopt (first read
  // doesn't need a resume) and is populated after the first row is
  // successfully read.
  std::optional<std::function<void()>> resume_cb_;
};

} // namespace

// ===========================================================================
// Factory
// ===========================================================================

/* static */
std::unique_ptr<MultiQueryStreamHandler> MultiQueryStreamHandler::create(
    MysqlClientBase& client,
    std::shared_ptr<MultiQueryStreamOperation> op) {
  std::unique_ptr<MultiQueryStreamHandler> ret;
  if (client.useDirectStreamMode()) {
    ret = std::unique_ptr<MultiQueryStreamHandler>(
        new SyncMultiQueryStreamHandler(client, std::move(op)));
  } else {
    ret = std::unique_ptr<MultiQueryStreamHandler>(
        new AsyncMultiQueryStreamHandler(client, std::move(op)));
  }

  ret->start();
  return ret;
}

// ===========================================================================
// Async mode implementation
// ===========================================================================

std::shared_ptr<StreamedQueryResult> AsyncMultiQueryStreamHandler::nextQuery() {
  return folly::coro::blockingWait(co_nextQuery());
}

folly::coro::Task<std::shared_ptr<StreamedQueryResult>>
AsyncMultiQueryStreamHandler::co_nextQuery() {
  if (current_user_result_) {
    // Drain anything that might be left in the existing result
    drainResult(*current_user_result_);
    current_user_result_.reset();
  }

  if (!pipe_.isReaderClosed()) {
    auto itemTry = co_await pipe_.next();
    if (itemTry.has_value()) {
      current_user_result_ = std::move(*itemTry);
    }
  }

  // Check exception_ only after the pipe read above has returned.
  // finalizeFailure() writes exception_ before closing the pipe, so the
  // pipe close → pipe read provides the happens-before ordering that
  // makes reading exception_ safe here.
  if (exception_) {
    co_yield folly::coro::co_error(*exception_);
  }

  co_return current_user_result_;
}

void AsyncMultiQueryStreamHandler::drain() {
  folly::coro::blockingWait(co_drainHandler());
}

folly::coro::Task<void> AsyncMultiQueryStreamHandler::co_drainHandler() {
  while (true) {
    auto retTry = co_await folly::coro::co_awaitTry(co_nextQuery());
    if (retTry.hasException() || !*retTry) {
      break;
    }
  }
}

std::optional<EphemeralRow> AsyncMultiQueryStreamHandler::fetchOneRow(
    StreamedQueryResult& result) {
  return folly::coro::blockingWait(co_fetchOneRow(result));
}

folly::coro::Task<std::optional<EphemeralRow>>
AsyncMultiQueryStreamHandler::co_fetchOneRow(StreamedQueryResult& result) {
  auto& rowPipe = resultRowPipe(result);

  if (!rowPipe.isReaderClosed()) {
    while (true) {
      try {
        // Resume the operation from the previous iteration so the next
        // callback can fire.  We defer this until the consumer asks for
        // a new row so the previous EphemeralRow's memory stays valid.
        if (resume_cb_) {
          (*resume_cb_)();
        }

        auto itemTry = co_await rowPipe.next();
        if (!itemTry.has_value()) {
          resume_cb_.reset();
          break;
        }

        if (!resume_cb_) {
          resume_cb_.emplace([op = operation_]() { op->resume(); });
        }
        co_return std::move(*itemTry);
      } catch (const QueryException& ex) {
        resume_cb_.reset();
        setResultException(result, std::make_unique<QueryException>(ex));
      }

      if (resultException(result)) {
        co_yield folly::coro::co_error(*resultException(result));
      }
    }
  }

  clearResultHandler(result);
  co_return std::nullopt;
}

void AsyncMultiQueryStreamHandler::drainCurrentResult(
    StreamedQueryResult& result) {
  while (fetchOneRow(result)) {
  }
}

void AsyncMultiQueryStreamHandler::initAndPublishResult(FetchOperation& op) {
  initBackendResult(op);
  pipe_.write(current_backend_result_);
}

void AsyncMultiQueryStreamHandler::writeRowToResult(EphemeralRow row) {
  DCHECK(current_backend_result_);
  resultRowPipe(*current_backend_result_).write(std::move(row));
}

void AsyncMultiQueryStreamHandler::finalizeQuery(FetchOperation& op) {
  DCHECK(current_backend_result_);
  setResultFinalData(
      *current_backend_result_,
      op.currentAffectedRows(),
      op.currentLastInsertId(),
      op.currentRecvGtid(),
      op.currentRespAttrs(),
      op.currentWarningsCount());
  resultRowPipe(*current_backend_result_).closeWriter();
  current_backend_result_.reset();
}

void AsyncMultiQueryStreamHandler::finalizeSuccess() {
  DCHECK(!current_backend_result_);
  pipe_.closeWriter();
}

void AsyncMultiQueryStreamHandler::finalizeFailure(FetchOperation& op) {
  exception_ = std::make_unique<QueryException>(
      op.numCurrentQuery(),
      op.result(),
      op.mysql_errno(),
      op.mysql_error(),
      op.connection()->getKey(),
      op.opElapsed());

  if (current_backend_result_) {
    setResultFinalData(
        *current_backend_result_,
        op.currentAffectedRows(),
        op.currentLastInsertId(),
        op.currentRecvGtid(),
        op.currentRespAttrs(),
        op.currentWarningsCount());
    resultRowPipe(*current_backend_result_).closeWriter(*exception_);
    current_backend_result_.reset();
    pipe_.closeWriter();
  } else {
    pipe_.closeWriter(*exception_);
  }
}

void AsyncMultiQueryStreamHandler::streamCallback(
    FetchOperation& op,
    StreamState op_state) {
  in_callback_baton_.reset();
  switch (op_state) {
    case StreamState::InitQuery:
      initAndPublishResult(op);
      break;

    case StreamState::RowsReady: {
      DCHECK(current_backend_result_);
      auto& stream = *op.rowStream();
      if (stream.hasNext()) {
        auto row = stream.consumeRow();
        op.pauseForConsumer();
        writeRowToResult(std::move(row));
      }
      break;
    }

    case StreamState::QueryEnded:
      finalizeQuery(op);
      break;

    case StreamState::Success:
      finalizeSuccess();
      break;

    case StreamState::Failure:
      finalizeFailure(op);
      break;

    default:
      CHECK(false)
          << "Somehow got invalid StreamState value - check for memory corruption";
  }
  in_callback_baton_.post();
}

// ===========================================================================
// Sync (direct) mode implementation
// ===========================================================================

std::shared_ptr<StreamedQueryResult> SyncMultiQueryStreamHandler::nextQuery() {
  return syncNextQuery();
}

folly::coro::Task<std::shared_ptr<StreamedQueryResult>>
SyncMultiQueryStreamHandler::co_nextQuery() {
  co_return syncNextQuery();
}

void SyncMultiQueryStreamHandler::drain() {
  while (!exception_) {
    try {
      auto result = syncNextQuery();
      if (!result) {
        break;
      }
      drainResult(*result);
    } catch (...) {
      break;
    }
  }
}

std::optional<EphemeralRow> SyncMultiQueryStreamHandler::fetchOneRow(
    StreamedQueryResult& result) {
  while (true) {
    auto state = sync_state_.load(std::memory_order_acquire);

    // If we're in RowsReady state, try to consume from the row stream.
    // The stream may have multiple rows buffered from a single RowsReady
    // callback, so we consume them all before unblocking the callback.
    if (state == SyncState::RowsReady) {
      auto* stream = operation_->rowStream();
      if (stream && stream->hasNext()) {
        return stream->consumeRow();
      }
      // Row stream exhausted for this batch - unblock callback to get more
      syncResumeCallback();
      // Fall through to wait for next callback
    }

    // If the query has ended, set final data on the result
    if (state == SyncState::QueryEnded) {
      if (current_backend_result_) {
        setResultFinalData(
            *current_backend_result_,
            operation_->currentAffectedRows(),
            operation_->currentLastInsertId(),
            operation_->currentRecvGtid(),
            operation_->currentRespAttrs(),
            operation_->currentWarningsCount());
        clearResultHandler(result);
      }
      syncResumeCallback();
      return std::nullopt;
    }

    if (state == SyncState::Failure) {
      if (current_backend_result_) {
        clearResultHandler(result);
      }
      throw *exception_;
    }

    // Wait for the next callback to signal us
    data_ready_baton_.wait();
  }
}

folly::coro::Task<std::optional<EphemeralRow>>
SyncMultiQueryStreamHandler::co_fetchOneRow(StreamedQueryResult& result) {
  co_return fetchOneRow(result);
}

void SyncMultiQueryStreamHandler::drainCurrentResult(
    StreamedQueryResult& result) {
  while (fetchOneRow(result)) {
  }
}

void SyncMultiQueryStreamHandler::streamCallback(
    FetchOperation& op,
    StreamState op_state) {
  in_callback_baton_.reset();
  switch (op_state) {
    case StreamState::InitQuery:
      // Pause the operation so the event base won't fire more callbacks until
      // the user thread calls resume(). This lets us return from the callback
      // without blocking the event base thread.
      op.pauseForConsumer();
      sync_state_.store(SyncState::InitQuery, std::memory_order_release);
      data_ready_baton_.post();
      break;

    case StreamState::RowsReady:
      op.pauseForConsumer();
      sync_state_.store(SyncState::RowsReady, std::memory_order_release);
      data_ready_baton_.post();
      break;

    case StreamState::QueryEnded:
      op.pauseForConsumer();
      sync_state_.store(SyncState::QueryEnded, std::memory_order_release);
      data_ready_baton_.post();
      break;

    case StreamState::Success:
      sync_state_.store(SyncState::Success, std::memory_order_release);
      data_ready_baton_.post();
      break;

    case StreamState::Failure:
      exception_ = std::make_unique<QueryException>(
          op.numCurrentQuery(),
          op.result(),
          op.mysql_errno(),
          op.mysql_error(),
          op.connection()->getKey(),
          op.opElapsed());
      sync_state_.store(SyncState::Failure, std::memory_order_release);
      data_ready_baton_.post();
      break;

    default:
      CHECK(false)
          << "Somehow got invalid StreamState value - check for memory corruption";
  }
  in_callback_baton_.post();
}

std::shared_ptr<StreamedQueryResult>
SyncMultiQueryStreamHandler::syncNextQuery() {
  if (current_user_result_) {
    // Drain the current result before fetching next query
    drainResult(*current_user_result_);
    current_user_result_.reset();
    current_backend_result_.reset();
  }

  // If we've already seen a terminal state, don't wait for more callbacks
  if (sync_done_) {
    if (exception_) {
      throw *exception_;
    }
    return nullptr;
  }

  if (exception_) {
    sync_done_ = true;
    throw *exception_;
  }

  while (true) {
    // Wait for next callback
    data_ready_baton_.wait();

    auto state = sync_state_.load(std::memory_order_acquire);
    if (state == SyncState::InitQuery) {
      initBackendResult(*operation_);
      current_user_result_ = current_backend_result_;
      syncResumeCallback();
      return current_user_result_;
    }

    if (state == SyncState::Success) {
      sync_done_ = true;
      return nullptr;
    }

    if (state == SyncState::Failure) {
      sync_done_ = true;
      throw *exception_;
    }

    // RowsReady or QueryEnded can appear here if the previous result's
    // drain short-circuited (e.g. final_data_ was already set) while a
    // callback with intermediate state was still pending on the baton.
    // Consume the stale state and loop to wait for the next callback.
    if (state == SyncState::RowsReady) {
      auto* stream = operation_->rowStream();
      if (stream) {
        while (stream->hasNext()) {
          stream->consumeRow();
        }
      }
      syncResumeCallback();
      continue;
    }

    if (state == SyncState::QueryEnded) {
      if (current_backend_result_) {
        setResultFinalData(
            *current_backend_result_,
            operation_->currentAffectedRows(),
            operation_->currentLastInsertId(),
            operation_->currentRecvGtid(),
            operation_->currentRespAttrs(),
            operation_->currentWarningsCount());
        current_backend_result_.reset();
      }
      syncResumeCallback();
      continue;
    }

    LOG(DFATAL) << "Unexpected state in syncNextQuery: "
                << static_cast<int>(state);
    return nullptr;
  }
}

void SyncMultiQueryStreamHandler::syncResumeCallback() {
  // Reset state and data_ready baton BEFORE resuming the operation, so the
  // next callback's post will correctly wake us up.
  sync_state_.store(SyncState::WaitingForData, std::memory_order_release);
  data_ready_baton_.reset();
  // Resume the operation — this tells the event base to continue processing
  // MySQL data and will eventually fire the next callback.
  operation_->resume();
}

} // namespace facebook::common::mysql_client
