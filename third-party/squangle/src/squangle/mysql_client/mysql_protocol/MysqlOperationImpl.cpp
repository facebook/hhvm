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
    : OperationBase(nullptr),
      EventHandler(client_.getEventBase()),
      AsyncTimeout(client_.getEventBase()) {}

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
