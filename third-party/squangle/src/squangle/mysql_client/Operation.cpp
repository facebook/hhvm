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

#include "squangle/base/ExceptionUtil.h"
#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/ConnectOperation.h"
#include "squangle/mysql_client/Flags.h"
#include "squangle/mysql_client/Operation.h"
#include "squangle/mysql_client/detail/MysqlConnection.h"

using namespace std::chrono_literals;

namespace facebook::common::mysql_client {

void OperationBase::run() {
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
    setTimeoutInternal(std::min(
        Duration(FLAGS_async_mysql_max_connect_timeout_micros), getTimeout()));
  }

  specializedRun();
}

void OperationImpl::protocolCompleteOperation(OperationResult result) {
  conn().runInThread(this, &OperationImpl::completeOperation, result);
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

void OperationBase::setTimeout(Duration timeout) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  setTimeoutInternal(timeout);
}

OperationImpl::OperationImpl(std::unique_ptr<ConnectionProxy> safe_conn)
    : OperationBase(std::move(safe_conn)),
      EventHandler(conn().mysql_client_.getEventBase()),
      AsyncTimeout(conn().mysql_client_.getEventBase()),
      observer_callback_(nullptr),
      mysql_client_(conn().mysql_client_) {
  conn().resetActionable();
}

bool OperationImpl::isInEventBaseThread() const {
  return conn().isInEventBaseThread();
}

bool OperationImpl::isEventBaseSet() const {
  return conn().getEventBase() != nullptr;
}

void OperationImpl::invokeActionable() {
  DCHECK(isInEventBaseThread());
  auto guard = makeRequestGuard();
  actionable();
}

std::optional<folly::SemiFuture<folly::Unit>>
OperationImpl::callPreQueryCallback(FetchOperation& op) const {
  if (callbacks_.pre_query_callback_) {
    return callbacks_.pre_query_callback_(op);
  }

  return std::nullopt;
}

DbQueryResult OperationImpl::callPostQueryCallback(DbQueryResult result) const {
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

DbMultiQueryResult OperationImpl::callPostQueryCallback(
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

AsyncPostQueryCallback OperationImpl::stealPostQueryCallback() {
  return std::move(callbacks_.post_query_callback_);
}

void OperationImpl::waitForActionable() {
  DCHECK(isInEventBaseThread());

  auto event_mask = conn().getReadWriteState();

  if (hasOpElapsed(getTimeout())) {
    timeoutTriggered();
    return;
  }

  auto leftUs = getTimeout() - opElapsed();
  auto leftMs = std::chrono::duration_cast<Millis>(leftUs);
  scheduleTimeout(leftMs.count());
  registerHandler(event_mask);
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

void OperationImpl::handlerReady(uint16_t /*events*/) noexcept {
  DCHECK(conn().isInEventBaseThread());
  CHECK_THROW(
      state() != OperationState::Completed &&
          state() != OperationState::Unstarted,
      db::OperationStateException);

  if (state() == OperationState::Cancelling) {
    cancel();
  } else {
    invokeActionable();
  }
}

void OperationImpl::timeoutTriggered() {
  specializedTimeoutTriggered();
}

void OperationImpl::completeOperation(OperationResult result) {
  DCHECK(isInEventBaseThread());
  if (state() == OperationState::Completed) {
    return;
  }

  CHECK_THROW(
      state() == OperationState::Pending ||
          state() == OperationState::Cancelling ||
          state() == OperationState::Unstarted,
      db::OperationStateException);
  completeOperationInner(result);
}

void OperationImpl::completeOperationInner(OperationResult result) {
  setState(OperationState::Completed);
  setResult(result);
  setDuration();
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
    callbacks_.post_operation_callback_(*op_);
  }

  specializedCompleteOperation();

  // call observer callback
  if (observer_callback_) {
    observer_callback_(*op_);
  }

  client().deferRemoveOperation(op_);
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

MysqlClientBase& OperationImpl::client() const {
  return mysql_client_;
}

std::shared_ptr<Operation> Operation::getSharedPointer() {
  return shared_from_this();
}

// const std::string& OperationImpl::host() const {
//   return conn().host();
// }
// int OperationImpl::port() const {
//   return conn().port();
// }

void OperationImpl::setObserverCallback(ObserverCallback obs_cb) {
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

AsyncPreQueryCallback OperationImpl::appendCallback(
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

AsyncPostQueryCallback OperationImpl::appendCallback(
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

void OperationImpl::setPreQueryCallback(AsyncPreQueryCallback&& callback) {
  callbacks_.pre_query_callback_ = appendCallback(
      std::move(callbacks_.pre_query_callback_), std::move(callback));
}

void OperationImpl::setPostQueryCallback(AsyncPostQueryCallback&& callback) {
  callbacks_.post_query_callback_ = appendCallback(
      std::move(callbacks_.post_query_callback_), std::move(callback));
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

std::unique_ptr<Connection> blockingConnectHelper(
    std::shared_ptr<ConnectOperation> conn_op) {
  conn_op->run().wait();
  if (!conn_op->ok()) {
    throw MysqlException(
        conn_op->result(),
        conn_op->mysql_errno(),
        conn_op->mysql_error(),
        conn_op->getKey(),
        conn_op->opElapsed());
  }

  return conn_op->releaseConnection();
}

OperationImpl::OwnedConnection::OwnedConnection(
    std::unique_ptr<Connection>&& conn)
    : conn_(std::move(conn)) {
  CHECK_THROW(conn_, db::InvalidConnectionException);
}

Connection* OperationImpl::OwnedConnection::get() {
  return conn_.get();
}

const Connection* OperationImpl::OwnedConnection::get() const {
  return conn_.get();
}

std::unique_ptr<Connection>
OperationImpl::OwnedConnection::releaseConnection() {
  return std::move(conn_);
}

std::string OperationImpl::threadOverloadMessage(double cbDelayUs) const {
  return fmt::format(
      "(CLIENT_OVERLOADED: cb delay {}ms, {} active conns)",
      std::lround(cbDelayUs / 1000.0),
      client().numStartedAndOpenConnections());
}

std::string OperationImpl::timeoutMessage(Millis delta) const {
  return fmt::format(
      "(took {}ms, timeout was {}ms)", delta.count(), getTimeoutMs().count());
}

unsigned int OperationImpl::mysql_errno() const {
  return getOp().mysql_errno();
}
const std::string& OperationImpl::mysql_error() const {
  return getOp().mysql_error();
}

/*static*/ std::string OperationImpl::connectStageString(connect_stage stage) {
  return detail::MysqlConnection::findConnectStageName(stage).value_or("");
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
