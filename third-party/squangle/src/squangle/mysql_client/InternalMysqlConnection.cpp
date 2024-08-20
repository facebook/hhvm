/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <mysql_async.h>
#include <string>

#include "squangle/mysql_client/InternalMysqlConnection.h"
#include "squangle/mysql_client/MysqlClientBase.h"

namespace facebook::common::mysql_client {

size_t InternalMysqlResult::numFields() const {
  auto ret = mysql_num_fields(res_.get());
  VLOG(4) << fmt::format(
      "mysql_num_fields({}) returned {}", (void*)res_.get(), ret);
  return ret;
}

MYSQL_FIELD* InternalMysqlResult::fields() const {
  auto ret = mysql_fetch_fields(res_.get());
  VLOG(4) << fmt::format(
      "mysql_fetch_fields({}) returned {}", (void*)res_.get(), (void*)ret);
  return ret;
}

size_t InternalMysqlResult::numRows() const {
  auto ret = mysql_num_rows(res_.get());
  VLOG(4) << fmt::format(
      "mysql_num_rows({}) returned {}", (void*)res_.get(), ret);
  return ret;
}

std::unique_ptr<InternalRowMetadata> InternalMysqlResult::getRowMetadata()
    const {
  return std::make_unique<InternalMysqlRowMetadata>(*this);
}

InternalMysqlConnection::InternalMysqlConnection(
    MysqlClientBase& client,
    MYSQL* mysql)
    : client_(client), mysql_(mysql) {}

InternalMysqlConnection::~InternalMysqlConnection() {
  DCHECK(mysql_ == nullptr);
}

bool InternalMysqlConnection::close() {
  if (closeFdOnDestroy_ && mysql_) {
    auto mysql = mysql_;
    mysql_ = nullptr;
    // Close our connection in the thread from which it was created.
    if (!client_.runInThread([mysql = mysql]() {
          // Unregister server cert validation callback
          const void* callback{nullptr};
          auto ret =
              mysql_options(mysql, MYSQL_OPT_TLS_CERT_CALLBACK, &callback);
          VLOG(4) << logMysqlOptionsImpl(
              mysql, "MYSQL_OPT_TLS_CERT_CALLBACK", callback, ret);
          DCHECK_EQ(ret, 0); // should always succeed

          mysql_close(mysql);
          VLOG(4) << fmt::format("mysql_close({})", (void*)mysql);
        })) {
      LOG(DFATAL)
          << "Mysql connection couldn't be closed: error in folly::EventBase";
    }

    return true;
  }

  return false;
}

std::string InternalMysqlConnection::serverInfo() const {
  auto ret = mysql_get_server_info(mysql_);
  VLOG(4) << fmt::format(
      "mysql_get_server_info({}) returned {}", (void*)mysql_, ret);
  return ret;
}

long InternalMysqlConnection::threadId() const {
  auto ret = mysql_thread_id(mysql_);
  VLOG(4) << fmt::format("mysql_thread_id({}) returned {}", (void*)mysql_, ret);
  return ret;
}

void InternalMysqlConnection::disableSSL() {
  enum mysql_ssl_mode ssl_mode = SSL_MODE_DISABLED;
  auto ret = mysql_options(mysql_, MYSQL_OPT_SSL_MODE, &ssl_mode);
  VLOG(4) << logMysqlOptions("MYSQL_OPT_SSL_MODE", ssl_mode, ret);
  DCHECK_EQ(ret, 0); // should always succeed
};

bool InternalMysqlConnection::sslSessionReused() const {
  auto ret = mysql_get_ssl_session_reused(mysql_);
  VLOG(4) << fmt::format(
      "mysql_get_ssl_session_reused({}) returned {}", (void*)mysql_, ret);
  return ret;
}

std::string InternalMysqlConnection::getTlsVersion() const {
  auto version = mysql_get_ssl_version(mysql_);
  VLOG(4) << fmt::format(
      "mysql_get_ssl_version({}) returned {}",
      (void*)mysql_,
      version ? version : "(null)");
  if (version) {
    return std::string(version);
  }

  return "";
}

int InternalMysqlConnection::warningCount() const {
  return mysql_warning_count(mysql_);
}

std::string InternalMysqlConnection::escapeString(
    std::string_view unescaped) const {
  std::string escaped;
  escaped.resize((2 * unescaped.size()) + 1);
  auto size = escapeString(escaped.data(), unescaped.data(), unescaped.size());
  escaped.resize(size);
  return escaped;
}

size_t InternalMysqlConnection::escapeString(
    char* ptr,
    const char* src,
    size_t length) const {
  auto ret = mysql_real_escape_string(mysql_, ptr, src, length);
  VLOG(4) << fmt::format(
      "mysql_real_escape_string({}, {}, {:.60} ({}), {}) returned {}",
      (void*)mysql_,
      (void*)ptr,
      src ? src : "(null)",
      src,
      length,
      ret);
  return ret;
}

folly::EventHandler::EventFlags InternalMysqlConnection::getReadWriteState()
    const {
  NET_ASYNC* net_async = NET_ASYNC_DATA(&mysql_->net);
  // net_async can be null during some stages of connecting
  auto async_blocking_state =
      net_async ? net_async->async_blocking_state : NET_NONBLOCKING_CONNECT;
  switch (async_blocking_state) {
    case NET_NONBLOCKING_READ:
      return folly::EventHandler::READ;
    case NET_NONBLOCKING_WRITE:
    case NET_NONBLOCKING_CONNECT:
      return folly::EventHandler::WRITE;
    default:
      break;
  }

  LOG(FATAL) << "Unknown nonblocking state " << async_blocking_state;
}

unsigned int InternalMysqlConnection::getErrno() const {
  auto ret = mysql_errno(mysql_);
  VLOG(4) << fmt::format("mysql_errno({}) returned {}", (void*)mysql_, ret);
  return ret;
}

std::string InternalMysqlConnection::getErrorMessage() const {
  auto ret = mysql_error(mysql_);
  VLOG(4) << fmt::format("mysql_error({}) returned {}", (void*)mysql_, ret);
  return ret;
}

void InternalMysqlConnection::setConnectAttributes(
    const AttributeMap& attributes) {
  // reset all connection attributes on the MYSQL connection
  auto ret = mysql_options(mysql_, MYSQL_OPT_CONNECT_ATTR_RESET, nullptr);
  VLOG(4) << logMysqlOptions("MYSQL_OPT_CONNECT_ATTR_RESET", nullptr, ret);
  DCHECK_EQ(ret, 0); // should always succeed

  // then applly the new attributes
  for (const auto& [key, value] : attributes) {
    ret = mysql_options4(
        mysql_, MYSQL_OPT_CONNECT_ATTR_ADD, key.c_str(), value.c_str());
    VLOG(4) << logMysqlOptions4("MYSQL_OPT_CONNECT_ATTR_ADD", key, value, ret);
    DCHECK_EQ(ret, 0); // should always succeed
  }
}

int InternalMysqlConnection::setQueryAttributes(
    const AttributeMap& attributes) {
  // reset all connection attributes on the MYSQL connection
  auto ret = mysql_options(mysql_, MYSQL_OPT_QUERY_ATTR_RESET, nullptr);
  VLOG(4) << logMysqlOptions("MYSQL_OPT_QUERY_ATTR_RESET", nullptr, ret);
  if (ret != 0) {
    return ret;
  }

  // then applly the new attributes
  for (const auto& [key, value] : attributes) {
    if (ret = setQueryAttribute(key, value); ret != 0) {
      return ret;
    }
  }

  return 0;
}

int InternalMysqlConnection::setQueryAttribute(
    const std::string& key,
    const std::string& value) {
  auto ret = mysql_options4(
      mysql_, MYSQL_OPT_QUERY_ATTR_ADD, key.c_str(), value.c_str());
  VLOG(4) << logMysqlOptions4("MYSQL_OPT_QUERY_ATTR_ADD", key, value, ret);
  return ret;
}

namespace {

std::string formatRespAttrEntry(
    std::string_view func,
    MYSQL* mysql,
    const char*& data,
    const size_t& len,
    int ret) {
  return fmt::format(
      "{}({}, SESSION_TRACK_RESP_ATTR, {}, {}) returned {}, data returned: {}",
      func,
      (void*)mysql,
      (void*)data,
      (void*)&len,
      ret,
      ((ret != 0 || !data) ? "<null>" : std::string_view(data, len)));
}

static std::optional<std::string> readFirstRespAttrEntry(MYSQL* mysql) {
  size_t len;
  const char* data;

  auto ret = mysql_session_track_get_first(
      mysql, SESSION_TRACK_RESP_ATTR, &data, &len);
  VLOG(4) << formatRespAttrEntry(
      "mysql_session_track_get_first", mysql, data, len, ret);
  if (ret == 0) {
    return std::string(data, len);
  }

  return std::nullopt;
}

static std::optional<std::string> readNextRespAttrEntry(MYSQL* mysql) {
  size_t len;
  const char* data;

  auto ret =
      mysql_session_track_get_next(mysql, SESSION_TRACK_RESP_ATTR, &data, &len);
  VLOG(4) << formatRespAttrEntry(
      "mysql_session_track_get_next", mysql, data, len, ret);
  if (ret == 0) {
    return std::string(data, len);
  }

  return std::nullopt;
}

static folly::Optional<std::pair<std::string, std::string>>
readFirstResponseAtribute(MYSQL* mysql) {
  if (auto key = readFirstRespAttrEntry(mysql)) {
    if (auto value = readNextRespAttrEntry(mysql)) {
      return std::make_pair(std::move(*key), std::move(*value));
    }
  }

  return folly::none;
}

static folly::Optional<std::pair<std::string, std::string>>
readNextResponseAtribute(MYSQL* mysql) {
  if (auto key = readNextRespAttrEntry(mysql)) {
    if (auto value = readNextRespAttrEntry(mysql)) {
      return std::make_pair(std::move(*key), std::move(*value));
    }
  }

  return folly::none;
}

} // namespace

AttributeMap InternalMysqlConnection::getResponseAttributes() const {
  AttributeMap attrs;

  for (auto attr = readFirstResponseAtribute(mysql_); attr;
       attr = readNextResponseAtribute(mysql_)) {
    attrs[std::move(attr->first)] = std::move(attr->second);
  }

  return attrs;
}

void InternalMysqlConnection::setCompression(CompressionAlgorithm algo) {
  auto ret = mysql_options(mysql_, MYSQL_OPT_COMPRESS, nullptr);
  VLOG(4) << logMysqlOptions("MYSQL_OPT_COMPRESS", nullptr, ret);
  DCHECK_EQ(ret, 0); // should always succeed

  setCompressionOption(mysql_, algo);
}

bool InternalMysqlConnection::setSSLOptionsProvider(
    SSLOptionsProviderBase& provider) {
  return provider.setMysqlSSLOptions(mysql_);
}

std::optional<std::string> InternalMysqlConnection::getSniServerName() const {
  const char* opt_val = nullptr;
  auto ret = mysql_get_option(mysql_, MYSQL_OPT_TLS_SNI_SERVERNAME, &opt_val);
  VLOG(4) << fmt::format(
      "mysql_get_option({}, MYSQL_OPT_TLS_SNI_SERVERNAME, {}), returned {}, data = {}",
      (void*)mysql_,
      (void*)&opt_val,
      ret,
      ((ret != 0 || !opt_val) ? "<null>" : opt_val));
  if (ret == 0 && opt_val != nullptr) {
    return opt_val;
  }

  return std::nullopt;
}

void InternalMysqlConnection::setSniServerName(const std::string& name) {
  auto ret = mysql_options(mysql_, MYSQL_OPT_TLS_SNI_SERVERNAME, name.c_str());
  VLOG(4) << logMysqlOptions("MYSQL_OPT_COMPRESS", name, ret);
  DCHECK_EQ(ret, 0); // should always succeed
}

bool InternalMysqlConnection::setDscp(uint8_t dscp) {
  // DS field (QOS/TOS level) is 8 bits with DSCP packed into the most
  // significant 6 bits.
  uint dsf = dscp << 2;
  auto ret = mysql_options(mysql_, MYSQL_OPT_TOS, &dsf) == 0;
  VLOG(4) << logMysqlOptions("MYSQL_OPT_TOS", dsf, ret);
  return ret;
}

void InternalMysqlConnection::setCertValidatorCallback(
    const MysqlCertValidatorCallback& cb,
    void* context) {
  auto ret = mysql_options(mysql_, MYSQL_OPT_TLS_CERT_CALLBACK, &cb);
  VLOG(4) << logMysqlOptions("MYSQL_OPT_TLS_CERT_CALLBACK", &cb, ret);
  DCHECK_EQ(ret, 0); // should always succeed

  ret = mysql_options(mysql_, MYSQL_OPT_TLS_CERT_CALLBACK_CONTEXT, &context);
  VLOG(4) << logMysqlOptions(
      "MYSQL_OPT_TLS_CERT_CALLBACK_CONTEXT", &context, ret);
  DCHECK_EQ(ret, 0); // should always succeed
}

void InternalMysqlConnection::setConnectTimeout(Millis timeout) const {
  uint timeoutInMs = timeout.count();
  auto ret = mysql_options(mysql_, MYSQL_OPT_CONNECT_TIMEOUT_MS, &timeoutInMs);
  VLOG(4) << logMysqlOptions("MYSQL_OPT_CONNECT_TIMEOUT_MS", timeoutInMs, ret);
  DCHECK_EQ(ret, 0); // should always succeed
}

void InternalMysqlConnection::setReadTimeout(Millis timeout) const {
  uint timeoutInMs = timeout.count();
  auto ret = mysql_options(mysql_, MYSQL_OPT_READ_TIMEOUT_MS, &timeoutInMs);
  VLOG(4) << logMysqlOptions("MYSQL_OPT_READ_TIMEOUT_MS", timeoutInMs, ret);
  DCHECK_EQ(ret, 0); // should always succeed
}

void InternalMysqlConnection::setWriteTimeout(Millis timeout) const {
  uint timeoutInMs = timeout.count();
  auto ret = mysql_options(mysql_, MYSQL_OPT_WRITE_TIMEOUT_MS, &timeoutInMs);
  VLOG(4) << logMysqlOptions("MYSQL_OPT_WRITE_TIMEOUT_MS", timeoutInMs, ret);
  DCHECK_EQ(ret, 0); // should always succeed
}

int InternalMysqlConnection::getSocketDescriptor() const {
  auto ret = mysql_get_socket_descriptor(mysql_);
  VLOG(4) << fmt::format(
      "mysql_get_socket_descriptor({}) returned {}", (void*)mysql_, ret);
  return ret;
}

/*static*/ std::optional<std::string>
InternalMysqlConnection::findConnectStageName(connect_stage stage) {
  // enum connect_stage is defined in mysql at include/mysql_com.h
  // and this provides a way to log the string version of this enum
  static const folly::F14FastMap<connect_stage, std::string> stageToStringMap =
      {
          {connect_stage::CONNECT_STAGE_INVALID, "CONNECT_STAGE_INVALID"},
          {connect_stage::CONNECT_STAGE_NOT_STARTED,
           "CONNECT_STAGE_NOT_STARTED"},
          {connect_stage::CONNECT_STAGE_NET_BEGIN_CONNECT,
           "CONNECT_STAGE_NET_BEGIN_CONNECT"},
#if MYSQL_VERSION_ID >= 80020 // csm_wait_connect added in 8.0.20
          {connect_stage::CONNECT_STAGE_NET_WAIT_CONNECT,
           "CONNECT_STAGE_NET_WAIT_CONNECT"},
#endif
          {connect_stage::CONNECT_STAGE_NET_COMPLETE_CONNECT,
           "CONNECT_STAGE_NET_COMPLETE_CONNECT"},
          {connect_stage::CONNECT_STAGE_READ_GREETING,
           "CONNECT_STAGE_READ_GREETING"},
          {connect_stage::CONNECT_STAGE_PARSE_HANDSHAKE,
           "CONNECT_STAGE_PARSE_HANDSHAKE"},
          {connect_stage::CONNECT_STAGE_ESTABLISH_SSL,
           "CONNECT_STAGE_ESTABLISH_SSL"},
          {connect_stage::CONNECT_STAGE_AUTHENTICATE,
           "CONNECT_STAGE_AUTHENTICATE"},
          {connect_stage::CONNECT_STAGE_PREP_SELECT_DATABASE,
           "CONNECT_STAGE_PREP_SELECT_DATABASE"},
          {connect_stage::CONNECT_STAGE_PREP_INIT_COMMANDS,
           "CONNECT_STAGE_PREP_INIT_COMMANDS"},
          {connect_stage::CONNECT_STAGE_SEND_ONE_INIT_COMMAND,
           "CONNECT_STAGE_SEND_ONE_INIT_COMMAND"},
          {connect_stage::CONNECT_STAGE_COMPLETE, "CONNECT_STAGE_COMPLETE"},
      };

  if (auto it = stageToStringMap.find(stage); it != stageToStringMap.end()) {
    return it->second;
  }

  return std::nullopt;
}

uint64_t InternalMysqlConnection::getLastInsertId() const {
  auto ret = mysql_insert_id(mysql_);
  VLOG(4) << fmt::format("mysql_insert_id({}) returned {}", (void*)mysql_, ret);
  return ret;
}

uint64_t InternalMysqlConnection::getAffectedRows() const {
  auto ret = mysql_affected_rows(mysql_);
  VLOG(4) << fmt::format(
      "mysql_affected_rows({}) returned {}", (void*)mysql_, ret);
  return ret;
}

std::string InternalMysqlConnection::getConnectStageName() const {
  auto stage = getMySqlConnectStage();
  if (auto optStageName = findConnectStageName(stage)) {
    return std::move(*optStageName);
  }

  return fmt::format("Unexpected connect_stage: {}", (int)stage);
}

std::optional<std::string> InternalMysqlConnection::getRecvGtid() const {
  const char* data;
  size_t length;
  auto ret = mysql_session_track_get_first(
      mysql_, SESSION_TRACK_GTIDS, &data, &length);
  VLOG(4) << fmt::format(
      "mysql_session_track_get_first({}, SESSION_TRACK_GTIDS, {}, {}) returned {}, data = {}",
      (void*)mysql_,
      (void*)&data,
      (void*)&length,
      ret,
      ((ret || !data) ? "<null>" : std::string_view(data, length)));
  if (ret == 0) {
    return std::string(data, length);
  }

  return std::nullopt;
}

std::optional<std::string> InternalMysqlConnection::getSchemaChanged() const {
  const char* data;
  size_t length;
  auto ret = mysql_session_track_get_first(
      mysql_, SESSION_TRACK_SCHEMA, &data, &length);
  VLOG(4) << fmt::format(
      "mysql_session_track_get_first({}, SESSION_TRACK_SCHEMA, {}, {}) returned {}, data = {}",
      (void*)mysql_,
      (void*)&data,
      (void*)&length,
      ret,
      ((ret || !data) ? "<null>" : std::string_view(data, length)));
  if (ret == 0) {
    return std::string(data, length);
  }

  return std::nullopt;
}

bool InternalMysqlConnection::hasMoreResults() const {
  auto ret = mysql_more_results(mysql_);
  VLOG(4) << fmt::format(
      "mysql_more_results({}) returned {}", (void*)mysql_, ret);
  return ret;
}

namespace {

static inline InternalConnection::Status toHandlerStatus(
    net_async_status status) {
  if (status == NET_ASYNC_ERROR) {
    return InternalConnection::Status::ERROR;
  } else if (status == NET_ASYNC_COMPLETE) {
    return InternalConnection::Status::DONE;
  } else {
    return InternalConnection::Status::PENDING;
  }
}

} // namespace

InternalConnection::Status InternalMysqlConnection::tryConnectBlocking(
    const std::string& host,
    const std::string& user,
    const std::string& password,
    const std::string& db_name,
    uint16_t port,
    const std::string& unixSocket,
    int flags) const {
  auto ret = mysql_real_connect(
      mysql_,
      host.c_str(),
      user.c_str(),
      password.c_str(),
      db_name.c_str(),
      port,
      unixSocket.c_str(),
      flags);
  VLOG(4) << fmt::format(
      "mysql_real_connect({}, {}, {}, {}, {}, {}, {}, {}) returned {}",
      (void*)mysql_,
      host,
      user,
      password,
      db_name,
      port,
      unixSocket,
      flags,
      (void*)ret);
  return ret == nullptr ? ERROR : DONE;
}

InternalConnection::Status InternalMysqlConnection::tryConnectNonBlocking(
    const std::string& host,
    const std::string& user,
    const std::string& password,
    const std::string& db_name,
    uint16_t port,
    const std::string& unixSocket,
    int flags) const {
  auto ret = mysql_real_connect_nonblocking(
      mysql_,
      host.empty() ? nullptr : host.c_str(),
      user.c_str(),
      password.c_str(),
      db_name.c_str(),
      port,
      !unixSocket.empty() ? unixSocket.c_str() : nullptr,
      flags);
  VLOG(4) << fmt::format(
      "mysql_real_connect({}, {}, {}, {}, {}, {}, {}, {}) returned {}",
      (void*)mysql_,
      host,
      user,
      password,
      db_name,
      port,
      unixSocket,
      flags,
      ret);
  return toHandlerStatus(ret);
}

InternalConnection::Status InternalMysqlConnection::runQueryBlocking(
    std::string_view query) const {
  auto ret = mysql_real_query(mysql_, query.data(), query.size());
  VLOG(4) << fmt::format(
      "mysql_real_query({}, {:60}, {}) returned {}",
      (void*)mysql_,
      query,
      query.size(),
      ret);
  return ret ? ERROR : DONE;
}

InternalConnection::Status InternalMysqlConnection::runQueryNonBlocking(
    std::string_view query) const {
  auto ret = mysql_real_query_nonblocking(mysql_, query.data(), query.size());
  VLOG(4) << fmt::format(
      "mysql_real_query_nonblocking({}, {:60}, {}) returned {}",
      (void*)mysql_,
      query,
      query.size(),
      ret);
  return toHandlerStatus(ret);
}

InternalConnection::Status InternalMysqlConnection::resetConnBlocking() const {
  auto ret = mysql_reset_connection(mysql_);
  VLOG(4) << fmt::format(
      "mysql_reset_connection({}) returned {}", (void*)mysql_, ret);
  return ret ? ERROR : DONE;
}

InternalConnection::Status InternalMysqlConnection::resetConnNonBlocking()
    const {
  auto ret = mysql_reset_connection_nonblocking(mysql_);
  VLOG(4) << fmt::format(
      "mysql_reset_connection_nonblocking({}) returned {}", (void*)mysql_, ret);
  return toHandlerStatus(ret);
}

InternalConnection::Status InternalMysqlConnection::changeUserBlocking(
    const std::string& user,
    const std::string& password,
    const std::string& database) const {
  auto ret = mysql_change_user(
      mysql_, user.c_str(), password.c_str(), database.c_str());
  VLOG(4) << fmt::format(
      "mysql_change_user({}, {}, {}, {}) returned {}",
      (void*)mysql_,
      user,
      password,
      database,
      ret);
  return ret ? ERROR : DONE;
}

InternalConnection::Status InternalMysqlConnection::changeUserNonBlocking(
    const std::string& user,
    const std::string& password,
    const std::string& database) const {
  auto ret = mysql_change_user_nonblocking(
      mysql_, user.c_str(), password.c_str(), database.c_str());
  VLOG(4) << fmt::format(
      "mysql_change_user_nonblocking({}, {}, {}, {}) returned {}",
      (void*)mysql_,
      user,
      password,
      database,
      ret);
  return toHandlerStatus(ret);
}

InternalConnection::Status InternalMysqlConnection::nextResultBlocking() const {
  auto ret = mysql_next_result(mysql_);
  VLOG(4) << fmt::format(
      "mysql_next_result({}) returned {}", (void*)mysql_, ret);
  return ret ? ERROR : DONE;
}

InternalConnection::Status InternalMysqlConnection::nextResultNonBlocking()
    const {
  auto ret = mysql_next_result_nonblocking(mysql_);
  VLOG(4) << fmt::format(
      "mysql_next_result_nonblocking({}) returned {}", (void*)mysql_, ret);
  return toHandlerStatus(ret);
}

namespace {

InternalResult::FetchRowRet InternalMysqlRowFactory(
    MYSQL_RES* result,
    MYSQL_ROW mysqlRow) {
  std::unique_ptr<InternalRow> row;
  if (mysqlRow) {
    auto* lengths = mysql_fetch_lengths(result);
    VLOG(4) << fmt::format(
        "mysql_fetch_lengths({}) returned {}", (void*)result, (void*)lengths);

    auto numFields = mysql_num_fields(result);
    VLOG(4) << fmt::format(
        "mysql_num_fields({}) returned {}", (void*)result, numFields);

    row = std::make_unique<InternalMysqlRow>(mysqlRow, numFields, lengths);
  }

  return std::make_pair(DONE, std::move(row));
}

} // namespace

InternalResult::FetchRowRet InternalMysqlResult::fetchRowBlocking() const {
  auto mysqlRow = mysql_fetch_row(res_.get());
  VLOG(4) << fmt::format(
      "mysql_fetch_row({}) returned {}", (void*)res_.get(), (void*)mysqlRow);

  return InternalMysqlRowFactory(res_.get(), mysqlRow);
}

InternalResult::FetchRowRet InternalMysqlResult::fetchRowNonBlocking() const {
  std::unique_ptr<InternalRow> row;

  MYSQL_ROW mysqlRow;
  auto ret = mysql_fetch_row_nonblocking(res_.get(), &mysqlRow);
  VLOG(4) << fmt::format(
      "mysql_fetch_row_nonblocking({}) returned {}, MYSQL_ROW = {}",
      (void*)res_.get(),
      ret,
      (void*)mysqlRow);

  if (ret == NET_ASYNC_COMPLETE) {
    return InternalMysqlRowFactory(res_.get(), mysqlRow);
  }

  return std::make_pair(toHandlerStatus(ret), std::move(row));
}

std::unique_ptr<InternalResult> InternalMysqlConnection::getResult() const {
  MYSQL_RES* res = mysql_use_result(mysql_);
  VLOG(4) << fmt::format(
      "mysql_use_result({}) returned {}", (void*)mysql_, (void*)res);
  return std::make_unique<InternalMysqlResult>(res);
}

std::unique_ptr<InternalResult> InternalMysqlConnection::storeResult() const {
  MYSQL_RES* res = mysql_store_result(mysql_);
  VLOG(4) << fmt::format(
      "mysql_store_result({}) returned {}", (void*)mysql_, (void*)res);
  return std::make_unique<InternalMysqlResult>(res);
}

size_t InternalMysqlConnection::getFieldCount() const {
  auto ret = mysql_field_count(mysql_);
  VLOG(4) << fmt::format(
      "mysql_field_count({}) returned {}", (void*)mysql_, ret);
  return ret;
}

bool InternalMysqlConnection::dumpDebugInfo() const {
  auto ret = mysql_dump_debug_info(mysql_);
  VLOG(4) << fmt::format(
      "mysql_dump_debug_info({}) returned {}", (void*)mysql_, ret);
  return ret == 0;
}

bool InternalMysqlConnection::ping() const {
  auto ret = mysql_ping(mysql_);
  VLOG(4) << fmt::format("mysql_ping({}) returned {}", (void*)mysql_, ret);
  return ret == 0;
}

InternalMysqlConnection::MySqlConnectStage
InternalMysqlConnection::getMySqlConnectStage() const {
  auto ret = mysql_get_connect_stage(mysql_);
  VLOG(4) << fmt::format(
      "mysql_get_connect_stage({}) returned {}", (void*)mysql_, ret);
  return ret;
}

/*static*/ MYSQL* InternalMysqlConnection::createMysql() {
  auto ret = mysql_init(nullptr);
  VLOG(4) << fmt::format("mysql_init({}) returned {}", nullptr, (void*)ret);
  MYSQL* mysql = static_cast<MYSQL*>(ret);
  if (!mysql) {
    throw std::runtime_error("mysql_init failed");
  }
  return mysql;
}

std::string InternalMysqlConnection::logMysqlOptionsImpl(
    MYSQL* mysql,
    std::string_view opt,
    const void* param,
    int ret) {
  return fmt::format(
      "mysql_options({}, {}, {}) returned {}", (void*)mysql, opt, param, ret);
}

std::string InternalMysqlConnection::logMysqlOptionsImpl(
    MYSQL* mysql,
    std::string_view opt,
    int param,
    int ret) {
  return fmt::format(
      "mysql_options({}, {}, {}) returned {}", (void*)mysql, opt, param, ret);
}

std::string InternalMysqlConnection::logMysqlOptionsImpl(
    MYSQL* mysql,
    std::string_view opt,
    const std::string& param,
    int ret) {
  return fmt::format(
      "mysql_options({}, {}, {}) returned {}", (void*)mysql, opt, param, ret);
}

std::string InternalMysqlConnection::logMysqlOptions4Impl(
    MYSQL* mysql,
    std::string_view opt,
    const std::string& param1,
    const std::string& param2,
    int ret) {
  return fmt::format(
      "mysql_options4({}, {}, {}, {}) returned {}",
      (void*)mysql,
      opt,
      param1,
      param2,
      ret);
}

} // namespace facebook::common::mysql_client
