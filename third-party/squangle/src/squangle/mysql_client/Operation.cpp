/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fmt/chrono.h>
#include <folly/Memory.h>
#include <folly/container/F14Map.h>
#include <folly/small_vector.h>
#include <folly/ssl/OpenSSLPtrTypes.h>
#include <gflags/gflags.h>

#include "squangle/base/ExceptionUtil.h"
#include "squangle/mysql_client/ConnectOperation.h"
#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/Flags.h"
#include "squangle/mysql_client/Operation.h"

using namespace std::chrono_literals;

namespace facebook::common::mysql_client {

void OperationBase::run() {
  client_.addOperation(op_->shared_from_this());
  stopwatch_ = std::make_unique<StopWatch>();
  if (callbacks_.pre_operation_callback_) {
    CHECK_THROW(
        state() == OperationState::Unstarted, db::OperationStateException);
    callbacks_.pre_operation_callback_(*op_);
  }

  {
    auto locked = cancel_on_run_.wlock();
    if (*locked) {
      setState(OperationState::Cancelling);
      protocolCompleteOperation(OperationResult::Cancelled);
      return;
    }

    CHECK_THROW(
        state() == OperationState::Unstarted, db::OperationStateException);
    setState(OperationState::Pending);
  }

  if (getOp().getOperationType() == db::OperationType::Connect) {
    setTimeoutInternal(
        std::min(
            Duration(FLAGS_async_mysql_max_connect_timeout_micros),
            getTimeout()));
  }

  specializedRun();
}

void OperationBase::setAttributes(const AttributeMap& attributes) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  for (const auto& [key, value] : attributes) {
    attributes_[key] = value;
  }
}

void OperationBase::setAttributes(AttributeMap&& attributes) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  for (auto& [key, value] : attributes) {
    attributes_[key] = std::move(value);
  }
}

void OperationBase::setAttribute(
    const std::string& key,
    const std::string& value) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  attributes_[key] = value;
}

unsigned int OperationBase::mysql_errno() const {
  return getOp().mysql_errno();
}
const std::string& OperationBase::mysql_error() const {
  return getOp().mysql_error();
}

void OperationBase::setObserverCallback(ObserverCallback obs_cb) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
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

void OperationBase::setPreQueryCallback(AsyncPreQueryCallback&& callback) {
  callbacks_.pre_query_callback_ = appendCallback(
      std::move(callbacks_.pre_query_callback_), std::move(callback));
}

void OperationBase::setPostQueryCallback(AsyncPostQueryCallback&& callback) {
  callbacks_.post_query_callback_ = appendCallback(
      std::move(callbacks_.post_query_callback_), std::move(callback));
}

void OperationBase::setTimeout(Duration timeout) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  setTimeoutInternal(timeout);
}

void OperationBase::setConnConnectionContext(
    std::shared_ptr<db::ConnectionContextBase> context) {
  conn().setConnectionContext(std::move(context));
}

void OperationBase::cancel() {
  folly::RequestContextScopeGuard guard(
      request_context_.exchange(nullptr, std::memory_order_relaxed));

  {
    // This code competes with `run()` to see who changes `state_` first,
    // since they both have the combination `check and change` this must
    // be locked
    auto locked = cancel_on_run_.wlock();
    if (state() == OperationState::Cancelling ||
        state() == OperationState::Completed) {
      // If the cancel was already called we dont do the cancelling
      // process again
      return;
    }

    if (state() == OperationState::Unstarted) {
      *locked = true;
      // wait the user to call "run()" to run the completeOperation
      // otherwise we will throw exception
      return;
    }

    setState(OperationState::Cancelling);
  }

  protocolCompleteOperation(OperationResult::Cancelled);
}

std::unique_ptr<Connection> OperationBase::releaseConnection() {
  CHECK_THROW(
      state() == OperationState::Completed ||
          state() == OperationState::Unstarted,
      db::OperationStateException);
  return conn_proxy_->releaseConnection();
}

void Operation::snapshotMysqlErrors(unsigned int errnum, std::string error) {
  mysql_errno_ = errnum;
  if (mysql_errno_ != 0) {
    if (mysql_errno_ == CR_TLS_SERVER_NOT_FOUND) {
      mysql_error_ = "Server loadshedded the connection request.";
    } else {
      mysql_error_ = std::move(error);
    }
  }
}

void Operation::setAsyncClientError(
    unsigned int mysql_errno,
    folly::StringPiece msg) {
  mysql_errno_ = mysql_errno;
  mysql_error_ = msg.toString();
}

std::shared_ptr<Operation> Operation::getSharedPointer() {
  return shared_from_this();
}

ChainedCallback OperationBase::setCallback(
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

std::optional<folly::SemiFuture<folly::Unit>>
OperationBase::callPreQueryCallback(FetchOperation& op) const {
  if (callbacks_.pre_query_callback_) {
    return callbacks_.pre_query_callback_(op);
  }

  return std::nullopt;
}

DbQueryResult OperationBase::callPostQueryCallback(DbQueryResult result) const {
  // If we have a callback set, wrap (and then unwrap) the result to/from the
  // callback's std::variant wrapper
  if (callbacks_.post_query_callback_) {
    return callbacks_.post_query_callback_(std::move(result))
        .deferValue([](auto&& result) {
          return std::get<DbQueryResult>(std::move(result));
        })
        .get();
  }

  return result;
}

DbMultiQueryResult OperationBase::callPostQueryCallback(
    DbMultiQueryResult result) const {
  // If we have a callback set, wrap (and then unwrap) the result to/from the
  // callback's std::variant wrapper
  if (callbacks_.post_query_callback_) {
    return callbacks_.post_query_callback_(std::move(result))
        .deferValue([](auto&& result) {
          return std::get<DbMultiQueryResult>(std::move(result));
        })
        .get();
  }

  return result;
}

AsyncPostQueryCallback OperationBase::stealPostQueryCallback() {
  return std::move(callbacks_.post_query_callback_);
}

AsyncPreQueryCallback OperationBase::appendCallback(
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

AsyncPostQueryCallback OperationBase::appendCallback(
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

const InternalConnection& OperationBase::getInternalConnection() const {
  return conn().getInternalConnection();
}

InternalConnection& OperationBase::getInternalConnection() {
  return conn().getInternalConnection();
}

void OperationBase::deferRemoveOperation(Operation* op) const {
  client_.deferRemoveOperation(op);
}

std::string OperationBase::timeoutMessage(Millis delta) const {
  return fmt::format("(took {}, timeout was {})", delta, getTimeoutMs());
}

std::string OperationBase::threadOverloadMessage(Duration cbDelay) const {
  // This message is used here https://fburl.com/code/d6t3td0r to perform
  // matching for SLA calculation. Please update the string if you change the
  // message.
  return fmt::format(
      "(CLIENT_OVERLOADED: cb delay {}, {} active conns)",
      std::chrono::duration_cast<std::chrono::milliseconds>(cbDelay),
      client_.numStartedAndOpenConnections());
}

void OperationBase::setPreOperationCallback(ChainedCallback chainedCallback) {
  callbacks_.pre_operation_callback_ = setCallback(
      std::move(callbacks_.pre_operation_callback_),
      std::move(chainedCallback));
}

void OperationBase::setPostOperationCallback(ChainedCallback chainedCallback) {
  callbacks_.post_operation_callback_ = setCallback(
      std::move(callbacks_.post_operation_callback_),
      std::move(chainedCallback));
}

OperationBase::OperationBase(std::unique_ptr<ConnectionProxy> safe_conn)
    : client_(safe_conn->get()->client()),
      conn_proxy_(std::move(safe_conn)),
      observer_callback_(nullptr) {
  conn().resetActionable();
  timeout_ = Duration(FLAGS_async_mysql_timeout_micros);
  request_context_.store(
      folly::RequestContext::saveContext(), std::memory_order_relaxed);
}

OperationBase::OwnedConnection::OwnedConnection(
    std::unique_ptr<Connection>&& conn)
    : conn_(std::move(conn)) {
  CHECK_THROW(conn_, db::InvalidConnectionException);
}

Connection* OperationBase::OwnedConnection::get() {
  return conn_.get();
}

const Connection* OperationBase::OwnedConnection::get() const {
  return conn_.get();
}

std::unique_ptr<Connection>
OperationBase::OwnedConnection::releaseConnection() {
  return std::move(conn_);
}

folly::StringPiece Operation::resultString() const {
  return toString(result());
}

folly::StringPiece Operation::stateString() const {
  return toString(state());
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

db::FailureReason operationResultToFailureReason(OperationResult result) {
  switch (result) {
    case OperationResult::Cancelled:
      return db::FailureReason::CANCELLED;
    case OperationResult::TimedOut:
      return db::FailureReason::TIMEOUT;
    default:
      return db::FailureReason::DATABASE_ERROR;
  }
}

std::unique_ptr<Connection> blockingConnectHelper(ConnectOperation& conn_op) {
  conn_op.run().wait();
  if (!conn_op.ok()) {
    conn_op.connection()
        ->client()
        .exceptionBuilder()
        .buildMysqlException(
            conn_op.result(),
            conn_op.mysql_errno(),
            conn_op.mysql_error(),
            conn_op.getKey(),
            conn_op.opElapsed())
        .throw_exception();
  }

  return conn_op.releaseConnection();
}

bool Operation::ok() const {
  return impl()->ok();
}

// Is the operation complete (success or failure)?
bool Operation::done() const {
  return impl()->done();
}

OperationResult Operation::result() const {
  return impl()->result();
}

OperationState Operation::state() const {
  return impl()->state();
}

Operation& Operation::run() {
  impl()->run();
  return *this;
}

// Wait for the Operation to complete.
void Operation::wait() const {
  conn().wait();
}

Operation& Operation::setTimeout(Duration timeout) {
  impl()->setTimeout(timeout);
  return *this;
}

Operation& Operation::setAttributes(const AttributeMap& attributes) {
  impl()->setAttributes(attributes);
  return *this;
}
Operation& Operation::setAttributes(AttributeMap&& attributes) {
  impl()->setAttributes(std::move(attributes));
  return *this;
}
Operation& Operation::setAttribute(
    const std::string& key,
    const std::string& value) {
  impl()->setAttribute(key, value);
  return *this;
}

std::unique_ptr<Connection> Operation::releaseConnection() {
  return impl()->releaseConnection();
}

} // namespace facebook::common::mysql_client
