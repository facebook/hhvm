/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <mysql.h>

#include "squangle/base/Base.h"
#include "squangle/base/ConnectionKey.h"
#include "squangle/logger/DBEventLogger.h"
#include "squangle/mysql_client/InternalConnection.h"

namespace facebook::common::mysql_client {

class AsyncConnectionPool;
class InternalMysqlResult;
class InternalMysqlRowMetadata;
class MysqlClientBase;

class InternalMysqlRow : public InternalRow {
 public:
  InternalMysqlRow(MYSQL_ROW row, size_t numCols, unsigned long* lengths)
      : row_(std::move(row)), numCols_(numCols), lengths_(lengths) {
    DCHECK(row_);
    DCHECK(numCols_);
    DCHECK(lengths_);
  }

  [[nodiscard]] bool isNull(size_t col) const override {
    DCHECK_LT(col, numCols_);
    return !row_[col];
  }

  [[nodiscard]] folly::StringPiece column(size_t col) const override {
    DCHECK_LT(col, numCols_);
    DCHECK(row_[col]);
    return folly::StringPiece(row_[col], lengths_[col]);
  }

  [[nodiscard]] size_t columnLength(size_t col) const override {
    DCHECK_LT(col, numCols_);
    return lengths_[col];
  }

 private:
  MYSQL_ROW row_;
  size_t numCols_;
  unsigned long* lengths_;
};

class InternalMysqlResult : public InternalResult {
 public:
  explicit InternalMysqlResult(MYSQL_RES* res) : res_(res) {}

  [[nodiscard]] FetchRowRet fetchRowBlocking() const override;

  [[nodiscard]] FetchRowRet fetchRowNonBlocking() const override;

  [[nodiscard]] size_t numFields() const;

  [[nodiscard]] MYSQL_FIELD* fields() const;

  [[nodiscard]] size_t numRows() const override;

  void close() override {
    res_->handle = nullptr;
  }

  [[nodiscard]] std::unique_ptr<InternalRowMetadata> getRowMetadata()
      const override;

 private:
  using MysqlResultDeleter =
      folly::static_function_deleter<MYSQL_RES, mysql_free_result>;
  using MysqlResultPtr = std::unique_ptr<MYSQL_RES, MysqlResultDeleter>;

  MysqlResultPtr res_;
};

class InternalMysqlRowMetadata : public InternalRowMetadata {
 public:
  explicit InternalMysqlRowMetadata(const InternalMysqlResult& result)
      : num_fields_(result.numFields()), fields_(result.fields()) {}

  [[nodiscard]] size_t numFields() const noexcept override {
    return num_fields_;
  }

  [[nodiscard]] folly::StringPiece getTableName(size_t index) const override {
    DCHECK_LT(index, num_fields_);
    return folly::StringPiece(
        fields_[index].table, fields_[index].table_length);
  }

  [[nodiscard]] folly::StringPiece getFieldName(size_t index) const override {
    DCHECK_LT(index, num_fields_);
    return folly::StringPiece(fields_[index].name, fields_[index].name_length);
  }

  [[nodiscard]] enum_field_types getFieldType(size_t index) const override {
    DCHECK_LT(index, num_fields_);
    return fields_[index].type;
  }

  [[nodiscard]] uint64_t getFieldFlags(size_t index) const override {
    DCHECK_LT(index, num_fields_);
    return fields_[index].flags;
  }

 private:
  size_t num_fields_;
  const MYSQL_FIELD* fields_;
};

// Holds the mysql connection for easier re use
class InternalMysqlConnection : public InternalConnection {
 public:
  explicit InternalMysqlConnection(
      MysqlClientBase& client,
      MYSQL* mysql = createMysql());

  // Closes the connection in hold
  virtual ~InternalMysqlConnection() override;

  // copy not allowed
  InternalMysqlConnection() = delete;
  InternalMysqlConnection(const InternalMysqlConnection&) = delete;
  InternalMysqlConnection& operator=(const InternalMysqlConnection&) = delete;

  void setReusable(bool reusable) override {
    canReuse_ = reusable;
  }

  [[nodiscard]] bool isReusable() const override {
    return canReuse_ && getErrno() == 0;
  }

  // Don't close the mysql fd in the destructor. Useful when connections
  // are managed outside this library.
  void disableCloseOnDestroy() override {
    closeFdOnDestroy_ = false;
  }

  [[nodiscard]] bool isSSL() const override {
    return mysql_->client_flag & CLIENT_SSL;
  }

  // Returns whether or not the connection is in a transaction based on server
  // status
  [[nodiscard]] bool inTransaction() const override {
    return mysql_->server_status & SERVER_STATUS_IN_TRANS;
  }

  void setNeedResetBeforeReuse() override {
    needResetBeforeReuse_ = true;
  }

  [[nodiscard]] bool needResetBeforeReuse() override {
    return needResetBeforeReuse_;
  }

  [[nodiscard]] std::string serverInfo() const override;

  [[nodiscard]] long threadId() const override;

  void disableLocalFiles() override {
    mysql_->options.client_flag &= ~CLIENT_LOCAL_FILES;
  }

  void disableSSL() override;

  [[nodiscard]] bool sslSessionReused() const override;

  [[nodiscard]] std::string getTlsVersion() const override;

  [[nodiscard]] int warningCount() const override;

  [[nodiscard]] std::string escapeString(
      std::string_view unescaped) const override;

  [[nodiscard]] size_t escapeString(char* out, const char* src, size_t length)
      const override;

  bool close() override;

  [[nodiscard]] folly::EventHandler::EventFlags getReadWriteState()
      const override;

  [[nodiscard]] unsigned int getErrno() const override;

  [[nodiscard]] std::string getErrorMessage() const override;

  void setConnectAttributes(const AttributeMap& attributes) override;

  int setQueryAttributes(const AttributeMap& attributes) override;

  int setQueryAttribute(const std::string& key, const std::string& value)
      override;

  [[nodiscard]] AttributeMap getResponseAttributes() const override;

  void setCompression(CompressionAlgorithm algo) override;

  [[nodiscard]] bool setSSLOptionsProvider(
      SSLOptionsProviderBase& provider) override;

  [[nodiscard]] std::optional<std::string> getSniServerName() const override;

  void setSniServerName(const std::string& name) override;

  [[nodiscard]] bool setDscp(uint8_t dscp) override;

  void setCertValidatorCallback(
      const MysqlCertValidatorCallback& cb,
      void* context) override;

  void setConnectTimeout(std::chrono::milliseconds timeout) const override;

  void setReadTimeout(std::chrono::milliseconds timeout) const override;

  void setWriteTimeout(std::chrono::milliseconds timeout) const override;

  [[nodiscard]] int getSocketDescriptor() const override;

  [[nodiscard]] bool isDoneWithTcpHandShake() const override {
    return getMySqlConnectStage() > tcpCompletionStage_;
  }

  [[nodiscard]] std::string getConnectStageName() const override;

  [[nodiscard]] static std::optional<std::string> findConnectStageName(
      connect_stage stage);

  bool storeSession(SSLOptionsProviderBase& provider) override {
    return provider.storeMysqlSSLSession(mysql_);
  }

  [[nodiscard]] uint64_t getLastInsertId() const override;

  [[nodiscard]] uint64_t getAffectedRows() const override;

  [[nodiscard]] std::optional<std::string> getRecvGtid() const override;

  [[nodiscard]] std::optional<std::string> getSchemaChanged() const override;

  [[nodiscard]] bool hasMoreResults() const override;

  [[nodiscard]] bool getNoIndexUsed() const override {
    return mysql_->server_status & SERVER_QUERY_NO_INDEX_USED;
  }

  [[nodiscard]] bool wasSlow() const override {
    return mysql_->server_status & SERVER_QUERY_WAS_SLOW;
  }

  [[nodiscard]] bool getAutocommit() const override {
    return mysql_->server_status & SERVER_STATUS_AUTOCOMMIT;
  }

  [[nodiscard]] Status tryConnectBlocking(
      const std::string& host,
      const std::string& user,
      const std::string& password,
      const std::string& db_name,
      uint16_t port,
      const std::string& unixSocket,
      int flags) const override;
  [[nodiscard]] Status tryConnectNonBlocking(
      const std::string& host,
      const std::string& user,
      const std::string& password,
      const std::string& db_name,
      uint16_t port,
      const std::string& unixSocket,
      int flags) const override;

  [[nodiscard]] Status runQueryBlocking(std::string_view query) const override;
  [[nodiscard]] Status runQueryNonBlocking(
      std::string_view query) const override;

  [[nodiscard]] Status resetConnBlocking() const override;
  [[nodiscard]] Status resetConnNonBlocking() const override;

  [[nodiscard]] Status changeUserBlocking(
      const std::string& user,
      const std::string& password,
      const std::string& database) const override;
  [[nodiscard]] Status changeUserNonBlocking(
      const std::string& user,
      const std::string& password,
      const std::string& database) const override;

  [[nodiscard]] Status nextResultBlocking() const override;
  [[nodiscard]] Status nextResultNonBlocking() const override;

  [[nodiscard]] std::unique_ptr<InternalResult> getResult() const override;

  [[nodiscard]] std::unique_ptr<InternalResult> storeResult() const override;

  [[nodiscard]] size_t getFieldCount() const override;

  [[nodiscard]] bool dumpDebugInfo() const override;

  [[nodiscard]] bool ping() const override;

 private:
  MysqlClientBase& client_;
  // Our MYSQL handle.
  MYSQL* mysql_;
  std::optional<std::string> current_schema_;
  bool closeFdOnDestroy_ = true;
  bool needResetBeforeReuse_ = false;
  bool canReuse_ = true;

  // Simple name for mysql internal connect stage enum
  using MySqlConnectStage = enum connect_stage;

  MySqlConnectStage getMySqlConnectStage() const;

  static MYSQL* createMysql();

  std::string logMysqlOptions(std::string_view opt, const void* param, int ret)
      const {
    return logMysqlOptionsImpl(mysql_, opt, param, ret);
  }

  std::string logMysqlOptions(std::string_view opt, int param, int ret) const {
    return logMysqlOptionsImpl(mysql_, opt, param, ret);
  }

  std::string logMysqlOptions(
      std::string_view opt,
      const std::string& param,
      int ret) const {
    return logMysqlOptionsImpl(mysql_, opt, param, ret);
  }

  std::string logMysqlOptions4(
      std::string_view opt,
      const std::string& param1,
      const std::string& param2,
      int ret) const {
    return logMysqlOptions4Impl(mysql_, opt, param1, param2, ret);
  }

  static std::string logMysqlOptionsImpl(
      MYSQL* mysql,
      std::string_view opt,
      const void* param,
      int ret);

  static std::string
  logMysqlOptionsImpl(MYSQL* mysql, std::string_view opt, int param, int ret);

  static std::string logMysqlOptionsImpl(
      MYSQL* mysql,
      std::string_view opt,
      const std::string& param,
      int ret);

  static std::string logMysqlOptions4Impl(
      MYSQL* mysql,
      std::string_view opt,
      const std::string& param1,
      const std::string& param2,
      int ret);

  // Mysql internal connect stage which handles the async tcp handshake
  // completion between client and server
  static constexpr MySqlConnectStage tcpCompletionStage_ =
      MySqlConnectStage::CONNECT_STAGE_NET_COMPLETE_CONNECT;
};

} // namespace facebook::common::mysql_client
