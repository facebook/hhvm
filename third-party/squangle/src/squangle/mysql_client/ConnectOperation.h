/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/AsyncTimeout.h>
#include <chrono>

#include "squangle/mysql_client/ConnectionOptions.h"
#include "squangle/mysql_client/Operation.h"

namespace facebook::common::mysql_client {

// An operation representing a pending connection.  Constructed via
// AsyncMysqlClient::beginConnection.
class ConnectOperation : public Operation {
 public:
  ~ConnectOperation() override;

  void setCallback(ConnectCallback cb) {
    connect_callback_ = std::move(cb);
  }

  const std::string& database() const {
    return conn_key_.db_name();
  }
  const std::string& user() const {
    return conn_key_.user();
  }

  const ConnectionKey& getConnectionKey() const {
    return conn_key_;
  }
  const ConnectionOptions& getConnectionOptions() const;
  const ConnectionKey& getKey() const {
    return conn_key_;
  }

  ConnectOperation& setSSLOptionsProviderBase(
      std::unique_ptr<SSLOptionsProviderBase> ssl_options_provider);
  ConnectOperation& setSSLOptionsProvider(
      std::shared_ptr<SSLOptionsProviderBase> ssl_options_provider);

  // Default timeout for queries created by the connection this
  // operation will create.
  ConnectOperation& setDefaultQueryTimeout(Duration t);
  ConnectOperation& setConnectionContext(
      std::shared_ptr<db::ConnectionContextBase> e) {
    CHECK_THROW(
        state_ == OperationState::Unstarted, db::OperationStateException);
    connection_context_ = std::move(e);
    return *this;
  }
  ConnectOperation& setSniServerName(const std::string& sni_servername);
  ConnectOperation& enableResetConnBeforeClose();
  ConnectOperation& enableDelayedResetConn();
  ConnectOperation& enableChangeUser();
  ConnectOperation& setCertValidationCallback(
      CertValidatorCallback callback,
      const void* context = nullptr,
      bool opPtrAsContext = false);
  const CertValidatorCallback& getCertValidationCallback() const {
    return conn_options_.getCertValidationCallback();
  }

  db::ConnectionContextBase* getConnectionContext() {
    CHECK_THROW(
        state_ == OperationState::Unstarted, db::OperationStateException);
    return connection_context_.get();
  }

  const db::ConnectionContextBase* getConnectionContext() const {
    CHECK_THROW(
        state_ == OperationState::Unstarted ||
            state_ == OperationState::Completed,
        db::OperationStateException);
    return connection_context_.get();
  }

  void reportServerCertContent(
      const std::string& sslCertCn,
      const std::vector<std::string>& sslCertSan,
      const std::vector<std::string>& sslCertIdentities,
      bool isValidated) {
    if (connection_context_) {
      if (!sslCertCn.empty()) {
        connection_context_->sslCertCn = sslCertCn;
      }
      if (!sslCertSan.empty()) {
        connection_context_->sslCertSan = sslCertSan;
      }
      if (!sslCertIdentities.empty()) {
        connection_context_->sslCertIdentities = sslCertIdentities;
      }
      connection_context_->isServerCertValidated = isValidated;
    }
  }

  // Don't call this; it's public strictly for AsyncMysqlClient to be
  // able to call make_shared.
  ConnectOperation(MysqlClientBase* mysql_client, ConnectionKey conn_key);

  void mustSucceed() override;

  // Overriding to narrow the return type
  // Each connect attempt will take at most this timeout to retry to acquire
  // the connection.
  ConnectOperation& setTimeout(Duration timeout);

  // This timeout allows for clients to fail fast when tcp handshake
  // latency is high . This method allows to override the tcp timeout
  // connection options. These timeouts can either be set directly or by
  // passing in connection options.
  ConnectOperation& setTcpTimeout(Duration timeout);

  const folly::Optional<Duration>& getTcpTimeout() const {
    return conn_options_.getConnectTcpTimeout();
  }

  // Sets the total timeout that the connect operation will use.
  // Each attempt will take at most `setTimeout`. Use this in case
  // you have strong timeout restrictions but still want the connection to
  // retry.
  ConnectOperation& setTotalTimeout(Duration total_timeout);

  // Sets the number of attempts this operation will try to acquire a mysql
  // connection.
  ConnectOperation& setConnectAttempts(uint32_t max_attempts);

  // Sets the DSCP (QoS) value associated with this connection
  //
  // See Also ConnectionOptions::setDscp
  ConnectOperation& setDscp(uint8_t dscp);

  folly::Optional<uint8_t> getDscp() const {
    return conn_options_.getDscp();
  }

  uint32_t attemptsMade() const {
    return attempts_made_;
  }

  Duration getAttemptTimeout() const {
    return conn_options_.getTimeout();
  }

  // Set if we should open a new connection to kill a timed out query
  // Should not be used when connecting through a proxy
  ConnectOperation& setKillOnQueryTimeout(bool killOnQueryTimeout);

  bool getKillOnQueryTimeout() const {
    return killOnQueryTimeout_;
  }

  ConnectOperation& setCompression(
      folly::Optional<CompressionAlgorithm> compression_lib) {
    conn_options_.setCompression(std::move(compression_lib));
    return *this;
  }

  const folly::Optional<CompressionAlgorithm>& getCompression() const {
    return conn_options_.getCompression();
  }

  ConnectOperation& setConnectionOptions(const ConnectionOptions& conn_options);

  static constexpr Duration kMinimumViableConnectTimeout =
      std::chrono::microseconds(50);

  db::OperationType getOperationType() const override {
    return db::OperationType::Connect;
  }

  bool isActive() const {
    return active_in_client_;
  }

 protected:
  virtual void attemptFailed(OperationResult result);
  virtual void attemptSucceeded(OperationResult result);

  ConnectOperation& specializedRun() override;
  void socketActionable() override;
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

  uint32_t attempts_made_ = 0;
  bool killOnQueryTimeout_ = false;
  ConnectionOptions conn_options_;

  // Context information for logging purposes.
  std::shared_ptr<db::ConnectionContextBase> connection_context_;

 private:
  void specializedRunImpl();

  void logConnectCompleted(OperationResult result);

  void maybeStoreSSLSession();

  bool isDoneWithTcpHandShake();

  static int mysqlCertValidator(
      X509* server_cert,
      const void* context,
      const char** errptr);

  const ConnectionKey conn_key_;

  int flags_;

  ConnectCallback connect_callback_;
  bool active_in_client_;

  // Timeout used for controlling early timeout of just the tcp handshake phase
  // before doing heavy lifting like ssl and other mysql protocol for connection
  // establishment
  class ConnectTcpTimeoutHandler : public folly::AsyncTimeout {
   public:
    ConnectTcpTimeoutHandler(
        folly::EventBase* base,
        ConnectOperation* connect_operation)
        : folly::AsyncTimeout(base), op_(connect_operation) {}

    ConnectTcpTimeoutHandler() = delete;
    ConnectTcpTimeoutHandler(const ConnectTcpTimeoutHandler&) = delete;
    ConnectTcpTimeoutHandler& operator=(const ConnectTcpTimeoutHandler&) =
        delete;

    void timeoutExpired() noexcept override {
      op_->tcpConnectTimeoutTriggered();
    }

   private:
    ConnectOperation* op_;
  };

  ConnectTcpTimeoutHandler tcp_timeout_handler_;

  friend class AsyncMysqlClient;
  friend class MysqlClientBase;
};

} // namespace facebook::common::mysql_client
