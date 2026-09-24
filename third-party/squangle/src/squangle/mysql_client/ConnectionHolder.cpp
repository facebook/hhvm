/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/ExceptionWrapper.h>
#include <folly/logging/xlog.h>

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
  resetCreationTime();
  client_.activeConnectionAdded(key_);
}

ConnectionHolder::ConnectionHolder(
    ConnectionHolder& other,
    std::shared_ptr<const ConnectionKey> key)
    : client_(other.client_),
      internalConn_(other.stealInternalConnection()),
      context_(other.context_),
      key_(std::move(key)),
      createWatch_(other.createWatch_),
      lastActivityWatch_(other.lastActivityWatch_),
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
  // Destructors are noexcept, and onClose() reaches
  // DBCounterBase::incrClosedConnections, which is pure virtual and therefore
  // supplied by whoever embeds this library.  This runs on every connection
  // teardown, including the cancel and timeout paths, so a throwing stats
  // implementation would end the process exactly when the network is already
  // misbehaving.
  if (auto ew = folly::try_and_catch([&] { closeInternalConnection(); })) {
    XLOG_EVERY_MS(ERR, 1000)
        << "Exception while closing a connection: " << ew.what();
  }

  // Guarded separately rather than sharing the block above: this is the pool's
  // active-connection accounting, so skipping it leaves the key inflated for
  // the life of the process and eats into the pool limit.  The close path is
  // the likeliest thing to throw, which is exactly when this still has to run.
  if (auto ew = folly::try_and_catch(
          [&] { client_.activeConnectionRemoved(key_); })) {
    XLOG_EVERY_MS(ERR, 1000)
        << "Exception removing an active connection: " << ew.what();
  }
}

void ConnectionHolder::closeInternalConnection() {
  if (!internalConn_) {
    return;
  }

  if (auto func = internalConn_->getCloseFunction()) {
    if (!client_.runInThread([func = std::move(func)]() { func(); })) {
      LOG(DFATAL) << "Connection couldn't be closed: error in folly::EventBase";
    }
  }

  onClose();
}

void ConnectionHolder::onClose() {
  if (opened_) {
    client_.stats()->incrClosedConnections(context_.get());
  }
}

void ConnectionHolder::connectionOpened() {
  opened_ = true;
  resetLastActivityTime();

  client_.stats()->incrOpenedConnections(context_.get());
}

} // namespace facebook::common::mysql_client
