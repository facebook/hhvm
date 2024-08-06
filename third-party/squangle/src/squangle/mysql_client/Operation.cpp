/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <errmsg.h> // mysql
#include <folly/Memory.h>
#include <folly/container/F14Map.h>
#include <folly/small_vector.h>
#include <folly/ssl/OpenSSLPtrTypes.h>
#include <gflags/gflags.h>
#include <mysql_async.h>
#include <squangle/mysql_client/InternalMysqlConnection.h>
#include <squangle/mysql_client/Row.h>
#include <atomic>

#include "squangle/base/ExceptionUtil.h"
#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/ConnectOperation.h"
#include "squangle/mysql_client/Flags.h"
#include "squangle/mysql_client/Operation.h"
#include "squangle/mysql_client/SSLOptionsProviderBase.h"

using namespace std::chrono_literals;

namespace facebook {
namespace common {
namespace mysql_client {

Operation::Operation(std::unique_ptr<ConnectionProxy> safe_conn)
    : EventHandler(safe_conn->get().mysql_client_.getEventBase()),
      AsyncTimeout(safe_conn->get().mysql_client_.getEventBase()),
      state_(OperationState::Unstarted),
      result_(OperationResult::Unknown),
      conn_proxy_(std::move(safe_conn)),
      mysql_errno_(0),
      observer_callback_(nullptr),
      mysql_client_(conn().mysql_client_) {
  timeout_ = Duration(FLAGS_async_mysql_timeout_micros);
  conn().resetActionable();
  request_context_.store(
      folly::RequestContext::saveContext(), std::memory_order_relaxed);
}

bool Operation::isInEventBaseThread() const {
  return connection().isInEventBaseThread();
}

bool Operation::isEventBaseSet() const {
  return connection().getEventBase() != nullptr;
}

Operation::~Operation() {}

void Operation::invokeActionable() {
  DCHECK(isInEventBaseThread());
  folly::RequestContextScopeGuard guard(
      request_context_.load(std::memory_order_relaxed));
  actionable();
}

void Operation::waitForActionable() {
  DCHECK(isInEventBaseThread());

  auto event_mask = conn().getReadWriteState();

  if (stopwatch_->elapsed(timeout_)) {
    timeoutTriggered();
    return;
  }

  auto leftUs = timeout_ - stopwatch_->elapsed();
  auto leftMs = std::chrono::duration_cast<std::chrono::milliseconds>(leftUs);
  scheduleTimeout(leftMs.count());
  registerHandler(event_mask);
}

void Operation::cancel() {
  folly::RequestContextScopeGuard guard(
      request_context_.exchange(nullptr, std::memory_order_relaxed));

  {
    // This code competes with `run()` to see who changes `state_` first,
    // since they both have the combination `check and change` this must
    // be locked
    auto locked = cancel_on_run_.wlock();
    if (state_ == OperationState::Cancelling ||
        state_ == OperationState::Completed) {
      // If the cancel was already called we dont do the cancelling
      // process again
      return;
    }

    if (state_ == OperationState::Unstarted) {
      *locked = true;
      // wait the user to call "run()" to run the completeOperation
      // otherwise we will throw exception
      return;
    }

    state_ = OperationState::Cancelling;
  }

  if (!connection().runInThread(
          this, &Operation::completeOperation, OperationResult::Cancelled)) {
    // if a strange error happen in EventBase , mark it cancelled now
    completeOperationInner(OperationResult::Cancelled);
  }
}

void Operation::handlerReady(uint16_t /*events*/) noexcept {
  DCHECK(conn().isInEventBaseThread());
  CHECK_THROW(
      state_ != OperationState::Completed &&
          state_ != OperationState::Unstarted,
      db::OperationStateException);

  if (state() == OperationState::Cancelling) {
    cancel();
  } else {
    invokeActionable();
  }
}

void Operation::timeoutTriggered() {
  specializedTimeoutTriggered();
}

Operation& Operation::run() {
  stopwatch_ = std::make_unique<StopWatch>();
  if (callbacks_.pre_operation_callback_) {
    CHECK_THROW(
        state() == OperationState::Unstarted, db::OperationStateException);
    callbacks_.pre_operation_callback_(*this);
  }
  {
    auto locked = cancel_on_run_.wlock();
    if (*locked) {
      state_ = OperationState::Cancelling;
      connection().runInThread(
          this, &Operation::completeOperation, OperationResult::Cancelled);
      return *this;
    }
    CHECK_THROW(
        state() == OperationState::Unstarted, db::OperationStateException);
    state_ = OperationState::Pending;
  }
  if (getOperationType() == db::OperationType::Connect) {
    timeout_ = std::min(
        Duration(FLAGS_async_mysql_max_connect_timeout_micros), timeout_);
  }
  return specializedRun();
}

void Operation::completeOperation(OperationResult result) {
  DCHECK(isInEventBaseThread());
  if (state_ == OperationState::Completed) {
    return;
  }

  CHECK_THROW(
      state_ == OperationState::Pending ||
          state_ == OperationState::Cancelling ||
          state_ == OperationState::Unstarted,
      db::OperationStateException);
  completeOperationInner(result);
}

void Operation::completeOperationInner(OperationResult result) {
  state_ = OperationState::Completed;
  result_ = result;
  opDuration_ = stopwatch_->elapsed();
  if ((result == OperationResult::Cancelled ||
       result == OperationResult::TimedOut) &&
      conn().hasInitialized()) {
    // Cancelled/timed out ops leave our connection in an undefined
    // state.  Close it to prevent trouble.
    conn().close();
  }

  unregisterHandler();
  cancelTimeout();

  if (callbacks_.post_operation_callback_) {
    callbacks_.post_operation_callback_(*this);
  }

  specializedCompleteOperation();

  // call observer callback
  if (observer_callback_) {
    observer_callback_(*this);
  }

  client().deferRemoveOperation(this);
}

std::unique_ptr<Connection> Operation::releaseConnection() {
  CHECK_THROW(
      state_ == OperationState::Completed ||
          state_ == OperationState::Unstarted,
      db::OperationStateException);
  return conn_proxy_->releaseConnection();
}

void Operation::snapshotMysqlErrors() {
  mysql_errno_ = conn().getErrno();
  if (mysql_errno_ != 0) {
    if (mysql_errno_ == CR_TLS_SERVER_NOT_FOUND) {
      mysql_error_ = "Server loadshedded the connection request.";
    } else {
      mysql_error_ = conn().getErrorMessage();
    }
  }
}

void Operation::setAsyncClientError(
    unsigned int mysql_errno,
    folly::StringPiece msg) {
  mysql_errno_ = mysql_errno;
  mysql_error_ = msg.toString();
}

void Operation::wait() const {
  conn().wait();
}

MysqlClientBase& Operation::client() const {
  return mysql_client_;
}

std::shared_ptr<Operation> Operation::getSharedPointer() {
  return shared_from_this();
}

const std::string& Operation::host() const {
  return conn().host();
}
int Operation::port() const {
  return conn().port();
}

void Operation::setObserverCallback(ObserverCallback obs_cb) {
  CHECK_THROW(state_ == OperationState::Unstarted, db::OperationStateException);
  // allow more callbacks to be set
  if (observer_callback_) {
    auto old_dbs_cb = std::move(observer_callback_);
    observer_callback_ = [obs = std::move(obs_cb),
                          old_obs = std::move(old_dbs_cb)](Operation& op) {
      obs(op);
      old_obs(op);
    };
  } else {
    observer_callback_ = std::move(obs_cb);
  }
}

ChainedCallback Operation::setCallback(
    ChainedCallback orgCallback,
    ChainedCallback newCallback) {
  if (!orgCallback) {
    return newCallback;
  }

  if (!newCallback) {
    return orgCallback;
  }

  return [orgCallback = std::move(orgCallback),
          newCallback = std::move(newCallback)](Operation& op) mutable {
    orgCallback(op);
    newCallback(op);
  };
}

void Operation::setPreOperationCallback(ChainedCallback chainedCallback) {
  callbacks_.pre_operation_callback_ = setCallback(
      std::move(callbacks_.pre_operation_callback_),
      std::move(chainedCallback));
}

void Operation::setPostOperationCallback(ChainedCallback chainedCallback) {
  callbacks_.post_operation_callback_ = setCallback(
      std::move(callbacks_.post_operation_callback_),
      std::move(chainedCallback));
}

AsyncPreQueryCallback Operation::appendCallback(
    AsyncPreQueryCallback&& callback1,
    AsyncPreQueryCallback&& callback2) {
  if (!callback1) {
    return std::move(callback2);
  }

  if (!callback2) {
    return std::move(callback1);
  }

  return [callback1 = std::move(callback1),
          callback2 = std::move(callback2)](FetchOperation& op) {
    return callback1(op).deferValue(
        [&op, callback2](auto&& /* unused */) { return callback2(op); });
  };
}

AsyncPostQueryCallback Operation::appendCallback(
    AsyncPostQueryCallback&& callback1,
    AsyncPostQueryCallback&& callback2) {
  if (!callback1) {
    return std::move(callback2);
  }

  if (!callback2) {
    return std::move(callback1);
  }

  return [callback1 = std::move(callback1),
          callback2 = std::move(callback2)](auto&& result) {
    return callback1(std::move(result)).deferValue([callback2](auto&& result) {
      return callback2(std::move(result));
    });
  };
}

void Operation::setPreQueryCallback(AsyncPreQueryCallback&& callback) {
  callbacks_.pre_query_callback_ = appendCallback(
      std::move(callbacks_.pre_query_callback_), std::move(callback));
}

void Operation::setPostQueryCallback(AsyncPostQueryCallback&& callback) {
  callbacks_.post_query_callback_ = appendCallback(
      std::move(callbacks_.post_query_callback_), std::move(callback));
}

folly::StringPiece Operation::resultString() const {
  return Operation::toString(result());
}

folly::StringPiece Operation::stateString() const {
  return Operation::toString(state());
}

folly::StringPiece Operation::toString(StreamState state) {
  switch (state) {
    case StreamState::InitQuery:
      return "InitQuery";
    case StreamState::RowsReady:
      return "RowsReady";
    case StreamState::QueryEnded:
      return "QueryEnded";
    case StreamState::Failure:
      return "Failure";
    case StreamState::Success:
      return "Success";
  }
  LOG(DFATAL) << "unable to convert state to string: "
              << static_cast<int>(state);
  return "Unknown state";
}

// overload of operator<< for StreamState
std::ostream& operator<<(std::ostream& os, StreamState state) {
  return os << Operation::toString(state);
}

folly::StringPiece Operation::toString(OperationState state) {
  switch (state) {
    case OperationState::Unstarted:
      return "Unstarted";
    case OperationState::Pending:
      return "Pending";
    case OperationState::Cancelling:
      return "Cancelling";
    case OperationState::Completed:
      return "Completed";
  }
  LOG(DFATAL) << "unable to convert state to string: "
              << static_cast<int>(state);
  return "Unknown state";
}

// overload of operator<< for OperationState
std::ostream& operator<<(std::ostream& os, OperationState state) {
  return os << Operation::toString(state);
}

folly::StringPiece Operation::toString(OperationResult result) {
  switch (result) {
    case OperationResult::Succeeded:
      return "Succeeded";
    case OperationResult::Unknown:
      return "Unknown";
    case OperationResult::Failed:
      return "Failed";
    case OperationResult::Cancelled:
      return "Cancelled";
    case OperationResult::TimedOut:
      return "TimedOut";
  }
  LOG(DFATAL) << "unable to convert result to string: "
              << static_cast<int>(result);
  return "Unknown result";
}

// overload of operator<< for OperationResult
std::ostream& operator<<(std::ostream& os, OperationResult result) {
  return os << Operation::toString(result);
}

folly::StringPiece FetchOperation::toString(FetchAction action) {
  switch (action) {
    case FetchAction::StartQuery:
      return "StartQuery";
    case FetchAction::InitFetch:
      return "InitFetch";
    case FetchAction::Fetch:
      return "Fetch";
    case FetchAction::WaitForConsumer:
      return "WaitForConsumer";
    case FetchAction::CompleteQuery:
      return "CompleteQuery";
    case FetchAction::CompleteOperation:
      return "CompleteOperation";
  }
  LOG(DFATAL) << "unable to convert result to string: "
              << static_cast<int>(action);
  return "Unknown result";
}

std::unique_ptr<Connection> blockingConnectHelper(
    std::shared_ptr<ConnectOperation> conn_op) {
  conn_op->run().wait();
  if (!conn_op->ok()) {
    throw MysqlException(
        conn_op->result(),
        conn_op->mysql_errno(),
        conn_op->mysql_error(),
        conn_op->getKey(),
        conn_op->elapsed());
  }

  return conn_op->releaseConnection();
}

Operation::OwnedConnection::OwnedConnection(std::unique_ptr<Connection>&& conn)
    : conn_(std::move(conn)) {
  CHECK_THROW(conn_, db::InvalidConnectionException);
}

Connection& Operation::OwnedConnection::get() {
  return *conn_.get();
}

const Connection& Operation::OwnedConnection::get() const {
  return *conn_.get();
}

std::unique_ptr<Connection> Operation::OwnedConnection::releaseConnection() {
  return std::move(conn_);
}

std::string Operation::threadOverloadMessage(double cbDelayUs) const {
  return fmt::format(
      "(CLIENT_OVERLOADED: cb delay {}ms, {} active conns)",
      std::lround(cbDelayUs / 1000.0),
      client().numStartedAndOpenConnections());
}

std::string Operation::timeoutMessage(std::chrono::milliseconds delta) const {
  auto toMs = std::chrono::duration_cast<std::chrono::milliseconds>(timeout_);
  return fmt::format(
      "(took {}ms, timeout was {}ms)", delta.count(), toMs.count());
}

/*static*/ std::string Operation::connectStageString(connect_stage stage) {
  return InternalMysqlConnection::findConnectStageName(stage).value_or("");
}

} // namespace mysql_client
} // namespace common
} // namespace facebook
