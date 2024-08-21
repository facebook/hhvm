/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <glog/logging.h>
#include <chrono>

#include "squangle/base/Base.h"
#include "squangle/base/ConnectionKey.h"
#include "squangle/logger/DBEventCounter.h"
#include "squangle/mysql_client/InternalConnection.h"

namespace facebook::common::mysql_client {

class MysqlClientBase;

class ConnectionHolder : public InternalConnection {
 public:
  using Clock = std::chrono::steady_clock;

  ConnectionHolder(
      MysqlClientBase& client,
      std::unique_ptr<InternalConnection> internalConn,
      std::shared_ptr<const ConnectionKey> key);

  ConnectionHolder(
      ConnectionHolder& other,
      std::shared_ptr<const ConnectionKey> key);

  virtual ~ConnectionHolder() override;

  // copy not allowed
  ConnectionHolder() = delete;
  ConnectionHolder(const ConnectionHolder&) = delete;
  ConnectionHolder& operator=(const ConnectionHolder&) = delete;

  [[nodiscard]] std::unique_ptr<InternalConnection> stealInternalConnection() {
    return std::move(internalConn_);
  }

  [[nodiscard]] MysqlClientBase& getMysqlClient() const {
    return client_;
  }

  // [[nodiscard]] const std::string& host() const {
  //   return key_.host();
  // }

  // [[nodiscard]] int port() const {
  //   return key_.port();
  // }

  // [[nodiscard]] const std::string& user() const {
  //   return key_.user();
  // }

  // [[nodiscard]] const std::string& database() const {
  //   return key_.db_name();
  // }

  // [[nodiscard]] const std::string& password() const {
  //   return key_.password();
  // }

  void setConnectionContext(
      std::shared_ptr<db::ConnectionContextBase> context) {
    context_ = std::move(context);
  }

  void setCreationTime(Timepoint creation_time) {
    createTime_ = creation_time;
  }

  [[nodiscard]] Timepoint getCreationTime() const {
    return createTime_;
  }

  void setLastActivityTime(Timepoint last_activity_time) {
    lastActiveTime_ = last_activity_time;
  }

  [[nodiscard]] Timepoint getLastActivityTime() const {
    return lastActiveTime_;
  }

  [[nodiscard]] std::shared_ptr<const ConnectionKey> getKey() const {
    return key_;
  }

  [[nodiscard]] MysqlClientBase& getClient() const {
    return client_;
  }

  [[nodiscard]] bool isConnectionOpened() const {
    return opened_;
  }

  void connectionOpened();

  void disableCloseOnDestroy() override {
    internalConn_->disableCloseOnDestroy();
  }

  void setReusable(bool reusable) override {
    internalConn_->setReusable(reusable);
  }

  [[nodiscard]] bool isReusable() const override {
    return internalConn_->isReusable();
  }

  [[nodiscard]] bool isSSL() const override {
    return internalConn_->isSSL();
  }

  [[nodiscard]] bool inTransaction() const override {
    return internalConn_->inTransaction();
  }

  void setNeedResetBeforeReuse() override {
    internalConn_->setNeedResetBeforeReuse();
  }

  bool needResetBeforeReuse() override {
    return internalConn_->needResetBeforeReuse();
  }

  [[nodiscard]] std::string serverInfo() const override {
    return internalConn_->serverInfo();
  }

  [[nodiscard]] long threadId() const override {
    return internalConn_->threadId();
  }

  void disableLocalFiles() override {
    internalConn_->disableLocalFiles();
  }

  void disableSSL() override {
    internalConn_->disableSSL();
  }

  [[nodiscard]] bool sslSessionReused() const override {
    return internalConn_->sslSessionReused();
  }

  [[nodiscard]] std::string getTlsVersion() const override {
    return internalConn_->getTlsVersion();
  }

  [[nodiscard]] int warningCount() const override {
    return internalConn_->warningCount();
  }

  [[nodiscard]] std::string escapeString(
      std::string_view unescaped) const override {
    return internalConn_->escapeString(unescaped);
  }

  [[nodiscard]] size_t escapeString(char* out, const char* src, size_t length)
      const override {
    return internalConn_->escapeString(out, src, length);
  }

  std::function<void()> getCloseFunction() override {
    // We shouldn't be calling this version
    DCHECK(false);
    return nullptr;
  }

  [[nodiscard]] folly::EventHandler::EventFlags getReadWriteState()
      const override {
    return internalConn_->getReadWriteState();
  }

  [[nodiscard]] unsigned int getErrno() const override {
    return internalConn_->getErrno();
  }

  [[nodiscard]] std::string getErrorMessage() const override {
    return internalConn_->getErrorMessage();
  }

  void setConnectAttributes(const AttributeMap& attributes) override {
    internalConn_->setConnectAttributes(attributes);
  }

  int setQueryAttributes(const AttributeMap& attributes) override {
    return internalConn_->setQueryAttributes(attributes);
  }

  int setQueryAttribute(const std::string& attribute, const std::string& value)
      override {
    return internalConn_->setQueryAttribute(attribute, value);
  }

  [[nodiscard]] AttributeMap getResponseAttributes() const override {
    return internalConn_->getResponseAttributes();
  }

  void setCompression(CompressionAlgorithm algo) override {
    internalConn_->setCompression(algo);
  }

  [[nodiscard]] bool setSSLOptionsProvider(
      SSLOptionsProviderBase& provider) override {
    return internalConn_->setSSLOptionsProvider(provider);
  }

  [[nodiscard]] std::optional<std::string> getSniServerName() const override {
    return internalConn_->getSniServerName();
  }

  void setSniServerName(const std::string& name) override {
    internalConn_->setSniServerName(name);
  }

  [[nodiscard]] bool setDscp(uint8_t dscp) override {
    return internalConn_->setDscp(dscp);
  }

  void setCertValidatorCallback(
      const MysqlCertValidatorCallback& cb,
      void* context) override {
    internalConn_->setCertValidatorCallback(cb, context);
  }

  void setConnectTimeout(Millis timeout) const override {
    internalConn_->setConnectTimeout(timeout);
  }

  void setReadTimeout(Millis timeout) const override {
    internalConn_->setReadTimeout(timeout);
  }

  void setWriteTimeout(Millis timeout) const override {
    internalConn_->setWriteTimeout(timeout);
  }

  [[nodiscard]] const InternalConnection& getInternalConnection() const {
    return *internalConn_;
  }

  [[nodiscard]] int getSocketDescriptor() const override {
    return internalConn_->getSocketDescriptor();
  }

  [[nodiscard]] bool isDoneWithTcpHandShake() const override {
    return internalConn_->isDoneWithTcpHandShake();
  }

  [[nodiscard]] std::string getConnectStageName() const override {
    return internalConn_->getConnectStageName();
  }

  [[nodiscard]] bool storeSession(SSLOptionsProviderBase& provider) override {
    return internalConn_->storeSession(provider);
  }

  [[nodiscard]] uint64_t getLastInsertId() const override {
    return internalConn_->getLastInsertId();
  }

  [[nodiscard]] uint64_t getAffectedRows() const override {
    return internalConn_->getAffectedRows();
  }

  [[nodiscard]] std::optional<std::string> getRecvGtid() const override {
    return internalConn_->getRecvGtid();
  }

  [[nodiscard]] std::optional<std::string> getSchemaChanged() const override {
    return internalConn_->getSchemaChanged();
  }

  [[nodiscard]] bool hasMoreResults() const override {
    return internalConn_->hasMoreResults();
  }

  [[nodiscard]] bool getNoIndexUsed() const override {
    return internalConn_->getNoIndexUsed();
  }

  [[nodiscard]] bool wasSlow() const override {
    return internalConn_->wasSlow();
  }

  [[nodiscard]] bool getAutocommit() const override {
    return internalConn_->getAutocommit();
  }

  [[nodiscard]] Status tryConnect(
      const ConnectionOptions& opts,
      std::shared_ptr<const ConnectionKey> conn_key,
      int flags) const override {
    return internalConn_->tryConnect(opts, std::move(conn_key), flags);
  }

  [[nodiscard]] Status runQuery(std::string_view query) const override {
    return internalConn_->runQuery(query);
  }

  [[nodiscard]] Status resetConn() const override {
    return internalConn_->resetConn();
  }

  [[nodiscard]] Status changeUser(
      std::shared_ptr<const ConnectionKey> conn_key) const override {
    return internalConn_->changeUser(std::move(conn_key));
  }

  [[nodiscard]] Status nextResult() const override {
    return internalConn_->nextResult();
  }

  [[nodiscard]] std::unique_ptr<InternalResult> getResult() const override {
    return internalConn_->getResult();
  }

  [[nodiscard]] size_t getFieldCount() const override {
    return internalConn_->getFieldCount();
  }

  [[nodiscard]] bool dumpDebugInfo() const override {
    return internalConn_->dumpDebugInfo();
  }

  [[nodiscard]] bool ping() const override {
    return internalConn_->ping();
  }

 protected:
  // Used to update the connection key when updating to a pooled connection
  void updateConnectionKey(std::shared_ptr<const ConnectionKey> new_key);

  void onClose();

  template <typename Client>
  friend class ConnectionPool;

 private:
  MysqlClientBase& client_;
  std::unique_ptr<InternalConnection> internalConn_;
  std::shared_ptr<db::ConnectionContextBase> context_;
  std::shared_ptr<const ConnectionKey> key_;
  bool opened_{false};
  Timepoint createTime_;
  Timepoint lastActiveTime_;
};

} // namespace facebook::common::mysql_client
