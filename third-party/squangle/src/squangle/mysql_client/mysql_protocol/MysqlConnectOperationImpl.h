/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/ConnectOperation.h"
#include "squangle/mysql_client/mysql_protocol/MysqlOperationImpl.h"

namespace facebook::common::mysql_client::mysql_protocol {

class MysqlConnectOperationImpl : public MysqlOperationImpl,
                                  virtual public ConnectOperationImpl {
 public:
  // Don't call this; it's public strictly for AsyncMysqlClient to be
  // able to call make_shared.
  MysqlConnectOperationImpl(
      MysqlClientBase* mysql_client,
      std::shared_ptr<const ConnectionKey> conn_key);
  virtual ~MysqlConnectOperationImpl() override;

  static constexpr Duration kMinimumViableConnectTimeout =
      std::chrono::microseconds(50);

  bool isActive() const override {
    return active_in_client_;
  }

 protected:
  virtual void attemptFailed(OperationResult result);
  virtual void attemptSucceeded(OperationResult result);

  virtual void specializedRun() override;
  void actionable() override;
  void specializedTimeoutTriggered() override;
  void specializedCompleteOperation() override;

  // Called when tcp timeout is triggered
  void tcpConnectTimeoutTriggered();

  // Removes the Client ref, it can be called by child classes without needing
  // to add them as friend classes of AsyncMysqlClient
  virtual void removeClientReference();

  bool shouldCompleteOperation(OperationResult result);

  folly::ssl::SSLSessionUniquePtr getSSLSession();

  // Implementation of timeout handling for tcpTimeout and overall connect
  // timeout
  void timeoutHandler(bool isTcpTimeout, bool isPool = false);

 private:
  virtual void specializedRunImpl();

  void logConnectCompleted(OperationResult result);

  void maybeStoreSSLSession();

  bool isDoneWithTcpHandShake();

  const MysqlConnectionKey& getMysqlKeyRef() const {
    DCHECK(dynamic_cast<const MysqlConnectionKey*>(conn_key_.get()));
    return *((const MysqlConnectionKey*)conn_key_.get());
  }

  static int mysqlCertValidator(
      X509* server_cert,
      const void* context,
      const char** errptr);

  int flags_;

  bool active_in_client_;

  // Timeout used for controlling early timeout of just the tcp handshake phase
  // before doing heavy lifting like ssl and other mysql protocol for connection
  // establishment
  class ConnectTcpTimeoutHandler : public folly::AsyncTimeout {
   public:
    ConnectTcpTimeoutHandler(
        folly::EventBase* base,
        MysqlConnectOperationImpl* connect_operation)
        : folly::AsyncTimeout(base), op_(connect_operation) {}

    ConnectTcpTimeoutHandler() = delete;
    ConnectTcpTimeoutHandler(const ConnectTcpTimeoutHandler&) = delete;
    ConnectTcpTimeoutHandler& operator=(const ConnectTcpTimeoutHandler&) =
        delete;

    void timeoutExpired() noexcept override {
      op_->tcpConnectTimeoutTriggered();
    }

   private:
    MysqlConnectOperationImpl* op_;
  };

  ConnectTcpTimeoutHandler tcp_timeout_handler_;

  friend class AsyncMysqlClient;
  friend class MysqlClientBase;
};

} // namespace facebook::common::mysql_client::mysql_protocol
