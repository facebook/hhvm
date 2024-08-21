/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventHandler.h>

#include "squangle/base/Base.h"
#include "squangle/mysql_client/Compression.h"
#include "squangle/mysql_client/SSLOptionsProviderBase.h"

namespace facebook::common::mysql_client {

class ConnectionKey;
class ConnectionOptions;

using MysqlCertValidatorCallback =
    int (*)(X509* server_cert, const void* context, const char** errptr);

enum InternalStatus {
  PENDING,
  DONE,
  ERROR,
};

class InternalRow {
 public:
  virtual ~InternalRow() = default;

  virtual bool isNull(size_t col) const = 0;

  virtual folly::StringPiece column(size_t col) const = 0;

  virtual size_t columnLength(size_t col) const = 0;
};

class InternalRowMetadata {
 public:
  virtual ~InternalRowMetadata() = default;

  virtual size_t numFields() const = 0;

  virtual folly::StringPiece getTableName(size_t field) const = 0;

  virtual folly::StringPiece getFieldName(size_t field) const = 0;

  virtual enum_field_types getFieldType(size_t field) const = 0;

  virtual uint64_t getFieldFlags(size_t field) const = 0;
};

class InternalResult {
 public:
  virtual ~InternalResult() = default;

  using FetchRowRet = std::pair<InternalStatus, std::unique_ptr<InternalRow>>;

  virtual FetchRowRet fetchRow() const = 0;

  virtual size_t numRows() const = 0;

  virtual void close() = 0;

  virtual std::unique_ptr<InternalRowMetadata> getRowMetadata() const = 0;
};

class InternalConnection {
 public:
  using Status = InternalStatus;

  virtual ~InternalConnection() = default;

  virtual void setReusable(bool reusable) = 0;

  virtual bool isReusable() const = 0;

  virtual void disableCloseOnDestroy() = 0;

  virtual bool isSSL() const = 0;

  virtual bool inTransaction() const = 0;

  virtual void setNeedResetBeforeReuse() = 0;

  virtual bool needResetBeforeReuse() = 0;

  virtual std::string serverInfo() const = 0;

  virtual long threadId() const = 0;

  virtual void disableLocalFiles() = 0;

  virtual void disableSSL() = 0;

  virtual bool sslSessionReused() const = 0;

  virtual std::string getTlsVersion() const = 0;

  virtual int warningCount() const = 0;

  virtual std::string escapeString(std::string_view unescaped) const = 0;

  virtual size_t escapeString(char* out, const char* src, size_t length)
      const = 0;

  virtual std::function<void()> getCloseFunction() = 0;

  virtual folly::EventHandler::EventFlags getReadWriteState() const = 0;

  virtual unsigned int getErrno() const = 0;

  virtual std::string getErrorMessage() const = 0;

  virtual void setConnectAttributes(const AttributeMap& attributes) = 0;

  virtual int setQueryAttributes(const AttributeMap& attributes) = 0;

  virtual int setQueryAttribute(
      const std::string& key,
      const std::string& value) = 0;

  virtual AttributeMap getResponseAttributes() const = 0;

  virtual void setCompression(CompressionAlgorithm algo) = 0;

  virtual bool setSSLOptionsProvider(SSLOptionsProviderBase& provider) = 0;

  virtual std::optional<std::string> getSniServerName() const = 0;

  virtual void setSniServerName(const std::string& name) = 0;

  virtual bool setDscp(uint8_t dscp) = 0;

  virtual void setCertValidatorCallback(
      const MysqlCertValidatorCallback& cb,
      void* context) = 0;

  virtual void setConnectTimeout(Millis timeout) const = 0;

  virtual void setReadTimeout(Millis timeout) const = 0;

  virtual void setWriteTimeout(Millis timeout) const = 0;

  virtual int getSocketDescriptor() const = 0;

  virtual bool isDoneWithTcpHandShake() const = 0;

  virtual std::string getConnectStageName() const = 0;

  virtual bool storeSession(SSLOptionsProviderBase& provider) = 0;

  virtual uint64_t getLastInsertId() const = 0;

  virtual uint64_t getAffectedRows() const = 0;

  virtual std::optional<std::string> getRecvGtid() const = 0;

  virtual std::optional<std::string> getSchemaChanged() const = 0;

  virtual bool hasMoreResults() const = 0;

  virtual bool getNoIndexUsed() const = 0;

  virtual bool wasSlow() const = 0;

  virtual bool getAutocommit() const = 0;

  virtual bool ping() const = 0;

  virtual Status tryConnect(
      const ConnectionOptions& opts,
      std::shared_ptr<const ConnectionKey> conn_key,
      int flags) const = 0;

  virtual Status runQuery(std::string_view query) const = 0;

  virtual Status resetConn() const = 0;

  virtual Status changeUser(
      std::shared_ptr<const ConnectionKey> conn_key) const = 0;

  virtual Status nextResult() const = 0;

  virtual std::unique_ptr<InternalResult> getResult() const = 0;

  virtual size_t getFieldCount() const = 0;

  virtual bool dumpDebugInfo() const = 0;
};

} // namespace facebook::common::mysql_client
