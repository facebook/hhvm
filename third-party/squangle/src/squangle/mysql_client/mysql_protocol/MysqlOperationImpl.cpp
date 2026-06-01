/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <boost/polymorphic_cast.hpp>
#include <fmt/chrono.h>

#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnection.h"
#include "squangle/mysql_client/mysql_protocol/MysqlOperationImpl.h"

namespace facebook::common::mysql_client::mysql_protocol {

void MysqlOperationImpl::protocolCompleteOperation(OperationResult result) {
  conn().runInThread(this, &MysqlOperationImpl::completeOperation, result);
}

MysqlOperationImpl::MysqlOperationImpl()
    : OperationBase(),
      EventHandler(nullptr, folly::NetworkSocket()),
      AsyncTimeout() {}

void MysqlOperationImpl::initializeFromConnection() {
  // Re-initialize EventHandler and AsyncTimeout with the correct event base
  // now that the connection is set up.
  // Note: For sync clients, getEventBase() returns nullptr, and that's OK -
  // we only attach when there's actually an EventBase available.
  auto* eventBase = conn().getEventBase();
  if (eventBase) {
    EventHandler::changeHandlerFD(folly::NetworkSocket());
    EventHandler::attachEventBase(eventBase);
    AsyncTimeout::attachEventBase(eventBase);
  }
}

bool MysqlOperationImpl::isInEventBaseThread() const {
  return conn().isInEventBaseThread();
}

bool MysqlOperationImpl::isEventBaseSet() const {
  return conn().getEventBase() != nullptr;
}

void MysqlOperationImpl::invokeActionable() {
  DCHECK(isInEventBaseThread());
  auto guard = makeRequestGuard();
  actionable();
}

/*static*/
MysqlConnection* MysqlOperationImpl::getMysqlConnection(
    InternalConnection* conn) {
  return boost::polymorphic_cast<MysqlConnection*>(conn);
}

/*static*/
const MysqlConnection* MysqlOperationImpl::getMysqlConnection(
    const InternalConnection* conn) {
  return boost::polymorphic_cast<const MysqlConnection*>(conn);
}

const MysqlConnection* MysqlOperationImpl::getMysqlConnection() const {
  return getMysqlConnection(&getInternalConnection());
}

MysqlConnection* MysqlOperationImpl::getMysqlConnection() {
  return getMysqlConnection(&getInternalConnection());
}

void MysqlOperationImpl::waitForActionable() {
  DCHECK(isInEventBaseThread());

  // Check if timeout manager is still valid (EventBase not shutting down)
  // This can happen if an operation is scheduled during client shutdown
  if (!getTimeoutManager()) {
    completeOperation(OperationResult::Failed);
    return;
  }

  auto event_mask = getMysqlConnection()->getReadWriteState();

  if (hasOpElapsed(getTimeout())) {
    timeoutTriggered();
    return;
  }

  auto leftUs = getTimeout() - opElapsed();
  auto leftMs = std::chrono::duration_cast<Millis>(leftUs);
  scheduleTimeout(leftMs.count());
  registerHandler(event_mask);
}

void MysqlOperationImpl::handlerReady(uint16_t /*events*/) noexcept {
  // handlerReady is `noexcept` so we can't throw from it.
  try {
    DCHECK(conn().isInEventBaseThread());

    auto st = state();
    if (st == OperationState::Cancelling) {
      cancel();
    } else if (
        st != OperationState::Completed && st != OperationState::Unstarted) {
      invokeActionable();
    } else {
      LOG(WARNING) << "handlerReady() called in unexpected state: " << st;
    }
  } catch (const std::exception& ex) {
    LOG(ERROR) << "Exception in handlerReady: " << ex.what();
  } catch (...) {
    LOG(ERROR) << "Unknown exception in handlerReady";
  }
}

void MysqlOperationImpl::timeoutExpired() noexcept {
  // timeoutExpired is `noexcept` so we can't throw from it.
  try {
    timeoutTriggered();
  } catch (const std::exception& ex) {
    LOG(ERROR) << "Exception in timeoutExpired: " << ex.what();
  } catch (...) {
    LOG(ERROR) << "Unknown exception in timeoutExpired";
  }
}

void MysqlOperationImpl::timeoutTriggered() {
  specializedTimeoutTriggered();
}

void MysqlOperationImpl::completeOperation(OperationResult result) {
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

void MysqlOperationImpl::completeOperationInner(OperationResult result) {
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

  deferRemoveOperation(op_);
}

/*static*/ std::string MysqlOperationImpl::connectStageString(
    connect_stage stage) {
  return MysqlConnection::findConnectStageName(stage).value_or("");
}

} // namespace facebook::common::mysql_client::mysql_protocol
