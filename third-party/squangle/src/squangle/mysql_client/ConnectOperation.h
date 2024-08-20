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

#include "squangle/base/ConnectionKey.h"
#include "squangle/mysql_client/ConnectionOptions.h"
#include "squangle/mysql_client/Operation.h"

namespace facebook::common::mysql_client {

// An operation representing a pending connection.  Constructed via
// AsyncMysqlClient::beginConnection.
class ConnectOperationImpl : public OperationImpl {
 public:
  virtual ~ConnectOperationImpl() override;

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

  void setSSLOptionsProviderBase(
      std::unique_ptr<SSLOptionsProviderBase> ssl_options_provider);
  void setSSLOptionsProvider(
      std::shared_ptr<SSLOptionsProviderBase> ssl_options_provider);

  // Default timeout for queries created by the connection this
  // operation will create.
  void setDefaultQueryTimeout(Duration t);
  void setSniServerName(const std::string& sni_servername);
  void enableResetConnBeforeClose();
  void enableDelayedResetConn();
  void enableChangeUser();
  void setCertValidationCallback(
      CertValidatorCallback callback,
      const void* context = nullptr,
      bool opPtrAsContext = false);
  const CertValidatorCallback& getCertValidationCallback() const {
    return conn_options_.getCertValidationCallback();
  }

  // Overriding to narrow the return type
  // Each connect attempt will take at most this timeout to retry to acquire
  // the connection.
  void setTimeout(Duration timeout);

  // This timeout allows for clients to fail fast when tcp handshake
  // latency is high . This method allows to override the tcp timeout
  // connection options. These timeouts can either be set directly or by
  // passing in connection options.
  void setTcpTimeout(Duration timeout);

  const folly::Optional<Duration>& getTcpTimeout() const {
    return conn_options_.getConnectTcpTimeout();
  }

  // Sets the total timeout that the connect operation will use.
  // Each attempt will take at most `setTimeout`. Use this in case
  // you have strong timeout restrictions but still want the connection to
  // retry.
  void setTotalTimeout(Duration total_timeout);

  // Sets the number of attempts this operation will try to acquire a mysql
  // connection.
  void setConnectAttempts(uint32_t max_attempts);

  // Sets the DSCP (QoS) value associated with this connection
  //
  // See Also ConnectionOptions::setDscp
  void setDscp(uint8_t dscp);

  folly::Optional<uint8_t> getDscp() const {
    return conn_options_.getDscp();
  }

  uint32_t attemptsMade() const noexcept {
    return attempts_made_;
  }

  Duration getAttemptTimeout() const noexcept {
    return conn_options_.getTimeout();
  }

  // Set if we should open a new connection to kill a timed out query
  // Should not be used when connecting through a proxy
  void setKillOnQueryTimeout(bool killOnQueryTimeout);

  bool getKillOnQueryTimeout() const noexcept {
    return killOnQueryTimeout_;
  }

  void setCompression(folly::Optional<CompressionAlgorithm> compression_lib) {
    conn_options_.setCompression(std::move(compression_lib));
  }

  const folly::Optional<CompressionAlgorithm>& getCompression() const {
    return conn_options_.getCompression();
  }

  void setConnectionOptions(const ConnectionOptions& conn_options);

  static constexpr Duration kMinimumViableConnectTimeout =
      std::chrono::microseconds(50);

  bool isActive() const {
    return active_in_client_;
  }

 protected:
  friend class MysqlClientBase;

  ConnectOperationImpl(MysqlClientBase* mysql_client, ConnectionKey conn_key);

  static std::unique_ptr<ConnectOperationImpl> create(
      MysqlClientBase* mysql_client,
      ConnectionKey conn_key);

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

  uint32_t attempts_made_ = 0;
  bool killOnQueryTimeout_ = false;
  ConnectionOptions conn_options_;

 private:
  virtual void specializedRunImpl();

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
        ConnectOperationImpl* connect_operation)
        : folly::AsyncTimeout(base), op_(connect_operation) {}

    ConnectTcpTimeoutHandler() = delete;
    ConnectTcpTimeoutHandler(const ConnectTcpTimeoutHandler&) = delete;
    ConnectTcpTimeoutHandler& operator=(const ConnectTcpTimeoutHandler&) =
        delete;

    void timeoutExpired() noexcept override {
      op_->tcpConnectTimeoutTriggered();
    }

   private:
    ConnectOperationImpl* op_;
  };

  ConnectOperation& op() const;

  ConnectTcpTimeoutHandler tcp_timeout_handler_;

  friend class AsyncMysqlClient;
  friend class MysqlClientBase;
};

class ConnectOperation : public Operation {
 public:
  void setCallback(ConnectCallback cb) {
    impl()->setCallback(std::move(cb));
  }

  // Overriding to narrow the return type
  // Each connect attempt will take at most this timeout to retry to acquire
  // the connection.
  ConnectOperation& setTimeout(Duration timeout) {
    impl()->setTimeout(timeout);
    return *this;
  }

  // This timeout allows for clients to fail fast when tcp handshake
  // latency is high . This method allows to override the tcp timeout
  // connection options. These timeouts can either be set directly or by
  // passing in connection options.
  ConnectOperation& setTcpTimeout(Duration timeout) {
    impl()->setTcpTimeout(timeout);
    return *this;
  }
  const folly::Optional<Duration>& getTcpTimeout() const {
    return impl()->getTcpTimeout();
  }

  // Sets the total timeout that the connect operation will use.
  // Each attempt will take at most `setTimeout`. Use this in case
  // you have strong timeout restrictions but still want the connection to
  // retry.
  ConnectOperation& setTotalTimeout(Duration total_timeout) {
    impl()->setTotalTimeout(total_timeout);
    return *this;
  }

  // Sets the number of attempts this operation will try to acquire a mysql
  // connection.
  ConnectOperation& setConnectAttempts(uint32_t max_attempts) {
    impl()->setConnectAttempts(max_attempts);
    return *this;
  }

  // Sets the DSCP (QoS) value associated with this connection
  //
  // See Also ConnectionOptions::setDscp
  ConnectOperation& setDscp(uint8_t dscp) {
    impl()->setDscp(dscp);
    return *this;
  }
  folly::Optional<uint8_t> getDscp() const {
    return impl()->getDscp();
  }

  uint32_t attemptsMade() const noexcept {
    return impl()->attemptsMade();
  }

  Duration getAttemptTimeout() const noexcept {
    return impl()->getAttemptTimeout();
  }

  // Set if we should open a new connection to kill a timed out query
  // Should not be used when connecting through a proxy
  ConnectOperation& setKillOnQueryTimeout(bool killOnQueryTimeout) {
    impl()->setKillOnQueryTimeout(killOnQueryTimeout);
    return *this;
  }
  bool getKillOnQueryTimeout() const noexcept {
    return impl()->getKillOnQueryTimeout();
  }

  ConnectOperation& setCompression(
      folly::Optional<CompressionAlgorithm> compression_lib) {
    impl()->setCompression(std::move(compression_lib));
    return *this;
  }
  const folly::Optional<CompressionAlgorithm>& getCompression() const {
    return impl()->getCompression();
  }

  ConnectOperation& setSSLOptionsProviderBase(
      std::unique_ptr<SSLOptionsProviderBase> ssl_options_provider) {
    impl()->setSSLOptionsProviderBase(std::move(ssl_options_provider));
    return *this;
  }
  ConnectOperation& setSSLOptionsProvider(
      std::shared_ptr<SSLOptionsProviderBase> ssl_options_provider) {
    impl()->setSSLOptionsProvider(std::move(ssl_options_provider));
    return *this;
  }

  ConnectOperation& setConnectionOptions(
      const ConnectionOptions& conn_options) {
    impl()->setConnectionOptions(conn_options);
    return *this;
  }
  // Default timeout for queries created by the connection this
  // operation will create.
  ConnectOperation& setDefaultQueryTimeout(Duration t) {
    impl()->setDefaultQueryTimeout(t);
    return *this;
  }
  ConnectOperation& setSniServerName(const std::string& sni_servername) {
    impl()->setSniServerName(sni_servername);
    return *this;
  }
  ConnectOperation& enableResetConnBeforeClose() {
    impl()->enableResetConnBeforeClose();
    return *this;
  }
  ConnectOperation& enableDelayedResetConn() {
    impl()->enableDelayedResetConn();
    return *this;
  }
  ConnectOperation& enableChangeUser() {
    impl()->enableChangeUser();
    return *this;
  }
  ConnectOperation& setCertValidationCallback(
      CertValidatorCallback callback,
      const void* context = nullptr,
      bool opPtrAsContext = false) {
    impl()->setCertValidationCallback(
        std::move(callback), context, opPtrAsContext);
    return *this;
  }

  const ConnectionOptions& getConnectionOptions() {
    return impl()->getConnectionOptions();
  }
  const ConnectionKey& getKey() const {
    return impl()->getKey();
  }

  db::OperationType getOperationType() const override {
    return db::OperationType::Connect;
  }

  void reportServerCertContent(
      const std::string& sslCertCn,
      const std::vector<std::string>& sslCertSan,
      const std::vector<std::string>& sslCertIdentities,
      bool isValidated) {
    impl()->withOptionalConnectionContext([&](auto& ctx) {
      if (!sslCertCn.empty()) {
        ctx.sslCertCn = sslCertCn;
      }
      if (!sslCertSan.empty()) {
        ctx.sslCertSan = sslCertSan;
      }
      if (!sslCertIdentities.empty()) {
        ctx.sslCertIdentities = sslCertIdentities;
      }
      ctx.isServerCertValidated = isValidated;
    });
  }

  void mustSucceed() override;

  bool isActive() const {
    return impl()->isActive();
  }

  const CertValidatorCallback& getCertValidationCallback() const {
    return impl()->getCertValidationCallback();
  }

 protected:
  static std::shared_ptr<ConnectOperation> create(
      std::unique_ptr<ConnectOperationImpl> impl);
  friend MysqlClientBase;

  explicit ConnectOperation(std::unique_ptr<ConnectOperationImpl> impl)
      : impl_(std::move(impl)) {
    if (!impl_) {
      throw std::runtime_error("ConnectOperationImpl is null");
    }

    impl_->setOperation(*this);
  }

  virtual ConnectOperationImpl* impl() override {
    return (ConnectOperationImpl*)impl_.get();
  }
  virtual const ConnectOperationImpl* impl() const override {
    return (ConnectOperationImpl*)impl_.get();
  }

  std::unique_ptr<ConnectOperationImpl> impl_;
};

} // namespace facebook::common::mysql_client
