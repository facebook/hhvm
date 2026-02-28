/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef COMMON_DB_EVENT_LOGGER_H
#define COMMON_DB_EVENT_LOGGER_H

#include <chrono>
#include <string>

#include "squangle/base/Base.h"
#include "squangle/base/ConnectionKey.h"
#include "squangle/logger/DBEventCounter.h"

namespace facebook {
namespace db {

enum class FailureReason {
  BAD_USAGE,
  TIMEOUT,
  CANCELLED,
  DATABASE_ERROR,
  DISCOVERY_ERROR,
};

enum class OperationType {
  None,
  Query,
  MultiQuery,
  MultiQueryStream,
  Connect,
  PoolConnect,
  Locator,
  TestDatabase,
  Reset,
  ChangeUser,
  ThriftQuery,
};

class EnumHelper {
 public:
  static constexpr const char* failureReasonToString(FailureReason reason) {
    switch (reason) {
      case FailureReason::BAD_USAGE:
        return "BadUsage";
      case FailureReason::TIMEOUT:
        return "Timeout";
      case FailureReason::CANCELLED:
        return "Cancelled";
      case FailureReason::DATABASE_ERROR:
        return "DatabaseError";
      case FailureReason::DISCOVERY_ERROR:
        return "DiscoveryError";
      default:
        return "(should not happen)";
    }
  }

  static constexpr folly::StringPiece operationTypeToString(
      OperationType operation_type) {
    switch (operation_type) {
      case OperationType::None:
        return "None";
      case OperationType::Query:
        return "Query";
      case OperationType::MultiQuery:
        return "MultiQuery";
      case OperationType::MultiQueryStream:
        return "MultiQueryStream";
      case OperationType::Connect:
        return "Connect";
      case OperationType::PoolConnect:
        return "PoolConnect";
      case OperationType::Locator:
        return "Locator";
      case OperationType::TestDatabase:
        return "TestDatabase";
      case OperationType::Reset:
        return "Reset";
      case OperationType::ChangeUser:
        return "ChangeUser";
      case OperationType::ThriftQuery:
        return "ThriftQuery";
      default:
        return "(should not happen)";
    }
  }
};

using Duration = std::chrono::duration<uint64_t, std::micro>;

class SquangleLoggingData {
 public:
  SquangleLoggingData(
      std::shared_ptr<const common::mysql_client::ConnectionKey> conn_key,
      const ConnectionContextBase* conn_context,
      db::ClientPerfStats clientPerfStats = db::ClientPerfStats())
      : connKey_(std::move(conn_key)),
        connContext_(conn_context),
        clientPerfStats_(clientPerfStats) {
    if (!connKey_) {
      connKey_ = std::make_shared<common::mysql_client::MysqlConnectionKey>();
    }
  }

  [[nodiscard]] std::shared_ptr<const common::mysql_client::ConnectionKey>
  getConnKey() const {
    return connKey_;
  }

  [[nodiscard]] const common::mysql_client::ConnectionKey& getConnKeyRef()
      const {
    return *connKey_;
  }

  [[nodiscard]] const ConnectionContextBase* getConnContext() const {
    return connContext_;
  }

  [[nodiscard]] const db::ClientPerfStats& getClientPerfStats() const {
    return clientPerfStats_;
  }

 private:
  std::shared_ptr<const common::mysql_client::ConnectionKey> connKey_;
  const ConnectionContextBase* connContext_;
  db::ClientPerfStats clientPerfStats_;
};

struct CommonLoggingData {
  CommonLoggingData(
      OperationType op,
      Duration duration,
      std::optional<Duration> timeout = std::nullopt,
      std::optional<std::string> db_version = std::nullopt,
      Duration max_thread_block_time = Duration(0),
      Duration total_thread_block_time = Duration(0))
      : operation_type(op),
        operation_duration(duration),
        operation_timeout(timeout),
        db_version(std::move(db_version)),
        max_thread_block_time(max_thread_block_time),
        total_thread_block_time(total_thread_block_time) {}
  OperationType operation_type;
  // How long the single operation took
  Duration operation_duration;
  // Configured timeout for the operation
  std::optional<Duration> operation_timeout;
  std::optional<std::string> db_version;
  // The most time spent executing code in a single iteration
  // of socketActionable
  Duration max_thread_block_time;
  Duration total_thread_block_time;
};

struct QueryLoggingData : CommonLoggingData {
  using AttributeMap = common::mysql_client::AttributeMap;
  QueryLoggingData(
      OperationType op,
      Duration duration,
      std::optional<Duration> timeout,
      int queries,
      std::shared_ptr<folly::fbstring> queryString,
      common::mysql_client::LoggingFuncsPtr loggingFuncs,
      int rows,
      uint64_t resultSize = 0,
      std::optional<std::string> db_version = std::nullopt,
      bool noIndexUsed = false,
      bool useChecksum = false,
      const AttributeMap& queryAttributes = AttributeMap(),
      AttributeMap responseAttributes = AttributeMap(),
      Duration maxThreadBlockTime = Duration(0),
      Duration totalThreadBlockTime = Duration(0),
      bool wasSlow = false,
      unsigned int warningsCount = 0,
      std::optional<uint64_t> rowsMatched = std::nullopt,
      uint64_t rowsAffected = 0)
      : CommonLoggingData(
            op,
            duration,
            timeout,
            std::move(db_version),
            maxThreadBlockTime,
            totalThreadBlockTime),
        queries_executed(queries),
        query(std::move(queryString)),
        loggingFuncs(std::move(loggingFuncs)),
        rows_received(rows),
        result_size(resultSize),
        no_index_used(noIndexUsed),
        use_checksum(useChecksum),
        query_attributes(queryAttributes),
        response_attributes(std::move(responseAttributes)),
        was_slow(wasSlow),
        warnings_count(warningsCount),
        rows_matched(rowsMatched),
        rows_affected(rowsAffected) {}
  int queries_executed;
  std::shared_ptr<folly::fbstring> query;
  common::mysql_client::LoggingFuncsPtr loggingFuncs;
  int rows_received;
  uint64_t result_size;
  bool no_index_used;
  bool use_checksum;
  AttributeMap query_attributes;
  AttributeMap response_attributes;
  bool was_slow;
  unsigned int warnings_count;
  std::optional<int> thrift_rpc_priority;
  std::optional<uint64_t> rows_matched;
  uint64_t rows_affected;
};

// Base class for logging events of db client apis. This should be used as an
// abstract and the children have specific ways to log.
template <typename TConnectionInfo>
class DBLoggerBase {
 public:
  // The api name should be given to differentiate the kind of client being used
  // to contact DB.
  explicit DBLoggerBase(std::string api_name)
      : api_name_(std::move(api_name)) {}

  virtual ~DBLoggerBase() {}

  // Basic logging functions to be overloaded in children
  virtual void logQuerySuccess(
      const QueryLoggingData& loggingData,
      const TConnectionInfo& connInfo) const = 0;

  virtual void logQueryFailure(
      const QueryLoggingData& logging_data,
      FailureReason reason,
      unsigned int mysqlErrno,
      const std::string& error,
      const TConnectionInfo& connInfo) const = 0;

  virtual void logConnectionSuccess(
      const CommonLoggingData& logging_data,
      const TConnectionInfo& connInfo) const = 0;

  virtual void logConnectionFailure(
      const CommonLoggingData& logging_data,
      FailureReason reason,
      unsigned int mysqlErrno,
      const std::string& error,
      const TConnectionInfo& connInfo) const = 0;

  virtual void setLoggingPrefix(std::string_view logging_prefix) = 0;

  static constexpr const char* FailureString(FailureReason reason) {
    return EnumHelper::failureReasonToString(reason);
  }

  static constexpr folly::StringPiece toString(OperationType operation_type) {
    return EnumHelper::operationTypeToString(operation_type);
  }

 protected:
  const std::string api_name_;
};

using SquangleLoggerBase = DBLoggerBase<SquangleLoggingData>;
// This is a simple version of the base logger as an example for other versions.
class DBSimpleLogger : public SquangleLoggerBase {
 public:
  explicit DBSimpleLogger(std::string api_name)
      : DBLoggerBase(std::move(api_name)) {}

  ~DBSimpleLogger() override {}

  void logQuerySuccess(
      const QueryLoggingData& logging_data,
      const SquangleLoggingData& connInfo) const override;

  void logQueryFailure(
      const QueryLoggingData& logging_data,
      FailureReason reason,
      unsigned int mysqlErrno,
      const std::string& error,
      const SquangleLoggingData& connInfo) const override;

  void logConnectionSuccess(
      const CommonLoggingData& logging_data,
      const SquangleLoggingData& connInfo) const override;

  void logConnectionFailure(
      const CommonLoggingData& logging_data,
      FailureReason reason,
      unsigned int mysqlErrno,
      const std::string& error,
      const SquangleLoggingData& connInfo) const override;
};
} // namespace db
} // namespace facebook

#endif // COMMON_DB_EVENT_LOGGER_H
