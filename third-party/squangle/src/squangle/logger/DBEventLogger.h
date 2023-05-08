/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef COMMON_DB_EVENT_LOGGER_H
#define COMMON_DB_EVENT_LOGGER_H

#include <errmsg.h> // MySQL

#include <chrono>
#include <string>

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
  static const char* failureReasonToString(FailureReason reason) {
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
    }
    return "(should not happen)";
  }

  static folly::StringPiece operationTypeToString(
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
    }
    return "(should not happen)";
  }
};

typedef std::chrono::duration<uint64_t, std::micro> Duration;

struct SquangleLoggingData {
  SquangleLoggingData(
      const common::mysql_client::ConnectionKey* conn_key,
      const ConnectionContextBase* conn_context)
      : connKey(conn_key), connContext(conn_context) {}
  const common::mysql_client::ConnectionKey* connKey;
  const ConnectionContextBase* connContext;
  db::ClientPerfStats clientPerfStats;
};

struct CommonLoggingData {
  CommonLoggingData(
      OperationType op,
      Duration duration,
      std::optional<Duration> timeout,
      Duration max_thread_block_time = Duration(0),
      Duration total_thread_block_time = Duration(0))
      : operation_type(op),
        operation_duration(duration),
        operation_timeout(timeout),
        max_thread_block_time(max_thread_block_time),
        total_thread_block_time(total_thread_block_time) {}
  OperationType operation_type;
  // How long the single operation took
  Duration operation_duration;
  // Configured timeout for the operation
  std::optional<Duration> operation_timeout;
  // The most time spent executing code in a single iteration
  // of socketActionable
  Duration max_thread_block_time;
  Duration total_thread_block_time;
};

struct QueryLoggingData : CommonLoggingData {
  QueryLoggingData(
      OperationType op,
      Duration duration,
      std::optional<Duration> timeout,
      int queries,
      const std::string& queryString,
      int rows,
      uint64_t resultSize = 0,
      bool noIndexUsed = false,
      bool useChecksum = false,
      const std::unordered_map<std::string, std::string>& queryAttributes =
          std::unordered_map<std::string, std::string>(),
      std::unordered_map<std::string, std::string> responseAttributes =
          std::unordered_map<std::string, std::string>(),
      Duration maxThreadBlockTime = Duration(0),
      Duration totalThreadBlockTime = Duration(0),
      bool wasSlow = false)
      : CommonLoggingData(
            op,
            duration,
            timeout,
            maxThreadBlockTime,
            totalThreadBlockTime),
        queries_executed(queries),
        query(queryString),
        rows_received(rows),
        result_size(resultSize),
        no_index_used(noIndexUsed),
        use_checksum(useChecksum),
        query_attributes(queryAttributes),
        response_attributes(std::move(responseAttributes)),
        was_slow(wasSlow) {}
  int queries_executed;
  std::string query;
  int rows_received;
  uint64_t result_size;
  bool no_index_used;
  bool use_checksum;
  std::unordered_map<std::string, std::string> query_attributes;
  std::unordered_map<std::string, std::string> response_attributes;
  bool was_slow;
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
      const TConnectionInfo& connInfo) = 0;

  virtual void logQueryFailure(
      const QueryLoggingData& logging_data,
      FailureReason reason,
      unsigned int mysqlErrno,
      const std::string& error,
      const TConnectionInfo& connInfo) = 0;

  virtual void logConnectionSuccess(
      const CommonLoggingData& logging_data,
      const TConnectionInfo& connInfo) = 0;

  virtual void logConnectionFailure(
      const CommonLoggingData& logging_data,
      FailureReason reason,
      unsigned int mysqlErrno,
      const std::string& error,
      const TConnectionInfo& connInfo) = 0;

  virtual void setLoggingPrefix(std::string_view logging_prefix) = 0;

  const char* FailureString(FailureReason reason) {
    return EnumHelper::failureReasonToString(reason);
  }

  folly::StringPiece toString(OperationType operation_type) {
    return EnumHelper::operationTypeToString(operation_type);
  }

 protected:
  const std::string api_name_;
};

typedef DBLoggerBase<SquangleLoggingData> SquangleLoggerBase;
// This is a simple version of the base logger as an example for other versions.
class DBSimpleLogger : public SquangleLoggerBase {
 public:
  explicit DBSimpleLogger(std::string api_name)
      : DBLoggerBase(std::move(api_name)) {}

  ~DBSimpleLogger() override {}

  void logQuerySuccess(
      const QueryLoggingData& logging_data,
      const SquangleLoggingData& connInfo) override;

  void logQueryFailure(
      const QueryLoggingData& logging_data,
      FailureReason reason,
      unsigned int mysqlErrno,
      const std::string& error,
      const SquangleLoggingData& connInfo) override;

  void logConnectionSuccess(
      const CommonLoggingData& logging_data,
      const SquangleLoggingData& connInfo) override;

  void logConnectionFailure(
      const CommonLoggingData& logging_data,
      FailureReason reason,
      unsigned int mysqlErrno,
      const std::string& error,
      const SquangleLoggingData& connInfo) override;
};
} // namespace db
} // namespace facebook

#endif // COMMON_DB_EVENT_LOGGER_H
