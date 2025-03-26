/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

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
  // We don't want to pay the cost of doing a dynamic_cast in prod when this
  // should _always_ be a MysqlConnection.  Thus just validate on debug builds.
  DCHECK(dynamic_cast<MysqlConnection*>(conn));
  return static_cast<MysqlConnection*>(conn);
}

/*static*/
const MysqlConnection* MysqlOperationImpl::getMysqlConnection(
    const InternalConnection* conn) {
  // We don't want to pay the cost of doing a dynamic_cast in prod when this
  // should _always_ be a MysqlConnection.  Thus just validate on debug builds.
  DCHECK(dynamic_cast<const MysqlConnection*>(conn));
  return static_cast<const MysqlConnection*>(conn);
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

std::string MysqlOperationImpl::threadOverloadMessage(double cbDelayUs) const {
  // This message is used here https://fburl.com/code/d6t3td0r to perform
  // matching for SLA calculation. Please update the string if you change the
  // message.
  return fmt::format(
      "(CLIENT_OVERLOADED: cb delay {}ms, {} active conns)",
      std::lround(cbDelayUs / 1000.0),
      client_.numStartedAndOpenConnections());
}

std::string MysqlOperationImpl::timeoutMessage(Millis delta) const {
  return fmt::format("(took {}, timeout was {})", delta, getTimeoutMs());
}

/*static*/ std::string MysqlOperationImpl::connectStageString(
    connect_stage stage) {
  return MysqlConnection::findConnectStageName(stage).value_or("");
}

} // namespace facebook::common::mysql_client::mysql_protocol
