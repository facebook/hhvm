/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/ConnectionHolder.h"
#include "squangle/mysql_client/MysqlClientBase.h"

namespace facebook::common::mysql_client {

ConnectionHolder::ConnectionHolder(
    MysqlClientBase& client,
    std::unique_ptr<InternalConnection> internalConn,
    std::shared_ptr<const ConnectionKey> key)
    : client_(client),
      internalConn_(std::move(internalConn)),
      key_(std::move(key)) {
  createTime_ = std::chrono::steady_clock::now();
  client_.activeConnectionAdded(key_);
}

ConnectionHolder::ConnectionHolder(
    ConnectionHolder& other,
    std::shared_ptr<const ConnectionKey> key)
    : client_(other.client_),
      internalConn_(other.stealInternalConnection()),
      context_(other.context_),
      key_(std::move(key)),
      createTime_(other.createTime_),
      lastActiveTime_(other.lastActiveTime_),
      opened_(other.opened_),
      poolFlagsHit_(other.poolFlagsHit_),
      poolFlagsChangeUser_(other.poolFlagsChangeUser_) {
  client_.activeConnectionAdded(key_);
}

void ConnectionHolder::updateConnectionKey(
    std::shared_ptr<const ConnectionKey> key) {
  client_.activeConnectionRemoved(key_);
  key_ = std::move(key);
  client_.activeConnectionAdded(key_);
}

ConnectionHolder::~ConnectionHolder() {
  if (internalConn_) {
    if (auto func = internalConn_->getCloseFunction()) {
      if (!client_.runInThread([func = std::move(func)]() { func(); })) {
        LOG(DFATAL)
            << "Connection couldn't be closed: error in folly::EventBase";
      }
    }

    onClose();
  }
  client_.activeConnectionRemoved(key_);
}

void ConnectionHolder::onClose() {
  if (opened_) {
    client_.stats()->incrClosedConnections(context_.get());
  }
}

void ConnectionHolder::connectionOpened() {
  opened_ = true;
  lastActiveTime_ = Clock::now();

  client_.stats()->incrOpenedConnections(context_.get());
}

} // namespace facebook::common::mysql_client
