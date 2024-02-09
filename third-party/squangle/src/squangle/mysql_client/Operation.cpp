/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <errmsg.h> // mysql
#include <folly/Memory.h>
#include <folly/container/F14Map.h>
#include <folly/small_vector.h>
#include <folly/ssl/OpenSSLPtrTypes.h>
#include <gflags/gflags.h>
#include <mysql_async.h>
#include <cmath>

#include "squangle/base/ExceptionUtil.h"
#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/Operation.h"
#include "squangle/mysql_client/SSLOptionsProviderBase.h"

// Default timeout of 0 is no-op for connect tcp timeout
DEFINE_int64(
    async_mysql_connect_tcp_timeout_micros,
    0,
    "default timeout, in micros, for mysql connect");

DEFINE_int64(
    async_mysql_connect_timeout_micros,
    1030 * 1000,
    "default timeout, in micros, for mysql connect");

DEFINE_int64(
    async_mysql_timeout_micros,
    10 * 1000 * 1000,
    "default timeout, in micros, for mysql operations");

DEFINE_int64(
    async_mysql_max_connect_timeout_micros,
#if defined(FOLLY_SANITIZE_ADDRESS) || (FOLLY_SANITIZE_THREAD)
    30 * 1000 * 1000,
#else
    3 * 1000 * 1000,
#endif
    "The maximum connect timeout, to protect customers from themselves");

namespace facebook {
namespace common {
namespace mysql_client {

namespace {
const std::string kQueryChecksumKey = "checksum";

using MysqlCertValidatorCallback =
    int (*)(X509* server_cert, const void* context, const char** errptr);

class OperationGuard {
 public:
  using Func = std::function<void()>;

  explicit OperationGuard(Func func) : func_(std::move(func)) {}
  ~OperationGuard() {
    func_();
  }

 private:
  Func func_;
};
} // namespace

namespace chrono = std::chrono;

ConnectionOptions::ConnectionOptions()
    : connection_timeout_(FLAGS_async_mysql_connect_timeout_micros),
      total_timeout_(FLAGS_async_mysql_timeout_micros * 2),
      query_timeout_(FLAGS_async_mysql_timeout_micros) {}

std::string ConnectionOptions::getDisplayString() const {
  // Reserve 4 + 2 extra elements
  folly::small_vector<std::string, 6> parts;

  parts.push_back(
      fmt::format("conn timeout={}us", connection_timeout_.count()));
  parts.push_back(fmt::format("query timeout={}us", query_timeout_.count()));
  parts.push_back(fmt::format("total timeout={}us", total_timeout_.count()));
  parts.push_back(fmt::format("conn attempts={}", max_attempts_));
  if (dscp_.has_value()) {
    parts.push_back(fmt::format("outbound dscp={}", *dscp_));
  }
  if (ssl_options_provider_ != nullptr) {
    parts.push_back(fmt::format(
        "SSL options provider={}", (void*)ssl_options_provider_.get()));
  }
  if (compression_lib_.has_value()) {
    parts.push_back(fmt::format(
        "compression library={}", (void*)compression_lib_.get_pointer()));
  }

  if (!attributes_.empty()) {
    std::vector<std::string> substrings;
    for (const auto& [key, value] : attributes_) {
      substrings.push_back(fmt::format("{}={}", key, value));
    }

    parts.push_back(fmt::format(
        "connection attributes=[{}]", folly::join(",", substrings)));
  }
  return fmt::format("({})", folly::join(", ", parts));
}

Operation::Operation(ConnectionProxy&& safe_conn)
    : state_(OperationState::Unstarted),
      result_(OperationResult::Unknown),
      conn_proxy_(std::move(safe_conn)),
      mysql_errno_(0),
      request_context_(folly::RequestContext::saveContext()),
      observer_callback_(nullptr),
      mysql_client_(conn()->mysql_client_) {
  timeout_ = Duration(FLAGS_async_mysql_timeout_micros);
  conn()->resetActionable();
}

bool Operation::isInEventBaseThread() const {
  return connection()->isInEventBaseThread();
}

bool Operation::isEventBaseSet() const {
  return connection()->getEventBase() != nullptr;
}

Operation::~Operation() {}

void Operation::invokeSocketActionable() {
  DCHECK(isInEventBaseThread());
  folly::RequestContextScopeGuard guard(request_context_);
  socketActionable();
}

void Operation::waitForSocketActionable() {
  DCHECK(isInEventBaseThread());

  MYSQL* mysql = conn()->mysql();
  uint16_t event_mask = 0;
  NET_ASYNC* net_async = NET_ASYNC_DATA(&mysql->net);
  // net_async can be null during some stages of connecting
  auto async_blocking_state =
      net_async ? net_async->async_blocking_state : NET_NONBLOCKING_CONNECT;
  switch (async_blocking_state) {
    case NET_NONBLOCKING_READ:
      event_mask |= folly::EventHandler::READ;
      break;
    case NET_NONBLOCKING_WRITE:
    case NET_NONBLOCKING_CONNECT:
      event_mask |= folly::EventHandler::WRITE;
      break;
    default:
      LOG(FATAL) << "Unknown nonblocking state " << async_blocking_state;
  }

  auto end = timeout_ + start_time_;
  auto now = chrono::steady_clock::now();
  if (now >= end) {
    timeoutTriggered();
    return;
  }

  conn()->socketHandler()->scheduleTimeout(
      chrono::duration_cast<chrono::milliseconds>(end - now).count());
  conn()->socketHandler()->registerHandler(event_mask);
}

void Operation::cancel() {
  folly::RequestContextScopeGuard guard(std::move(request_context_));

  {
    // This code competes with `run()` to see who changes `state_` first,
    // since they both have the combination `check and change` this must
    // be locked
    std::unique_lock<std::mutex> l(run_state_mutex_);

    if (state_ == OperationState::Cancelling ||
        state_ == OperationState::Completed) {
      // If the cancel was already called we dont do the cancelling
      // process again
      return;
    }

    if (state_ == OperationState::Unstarted) {
      cancel_on_run_ = true;
      // wait the user to call "run()" to run the completeOperation
      // otherwise we will throw exception
      return;
    }

    state_ = OperationState::Cancelling;
  }

  if (!connection()->runInThread(
          this, &Operation::completeOperation, OperationResult::Cancelled)) {
    // if a strange error happen in EventBase , mark it cancelled now
    completeOperationInner(OperationResult::Cancelled);
  }
}

void Operation::timeoutTriggered() {
  specializedTimeoutTriggered();
}

Operation* Operation::run() {
  start_time_ = chrono::steady_clock::now();
  if (callbacks_.pre_operation_callback_) {
    CHECK_THROW(
        state() == OperationState::Unstarted, db::OperationStateException);
    callbacks_.pre_operation_callback_(*this);
  }
  {
    std::unique_lock<std::mutex> l(run_state_mutex_);
    if (cancel_on_run_) {
      state_ = OperationState::Cancelling;
      cancel_on_run_ = false;
      connection()->runInThread(
          this, &Operation::completeOperation, OperationResult::Cancelled);
      return this;
    }
    CHECK_THROW(
        state() == OperationState::Unstarted, db::OperationStateException);
    state_ = OperationState::Pending;
  }
  if (getOperationType() == db::OperationType::Connect) {
    timeout_ = std::min(
        Duration(FLAGS_async_mysql_max_connect_timeout_micros), timeout_);
  }
  return specializedRun();
}

void Operation::completeOperation(OperationResult result) {
  DCHECK(isInEventBaseThread());
  if (state_ == OperationState::Completed) {
    return;
  }

  CHECK_THROW(
      state_ == OperationState::Pending ||
          state_ == OperationState::Cancelling ||
          state_ == OperationState::Unstarted,
      db::OperationStateException);
  completeOperationInner(result);
}

void Operation::completeOperationInner(OperationResult result) {
  state_ = OperationState::Completed;
  result_ = result;
  end_time_ = chrono::steady_clock::now();
  if ((result == OperationResult::Cancelled ||
       result == OperationResult::TimedOut) &&
      conn()->hasInitialized()) {
    // Cancelled/timed out ops leave our connection in an undefined
    // state.  Close it to prevent trouble.
    conn()->close();
  }

  conn()->socketHandler()->unregisterHandler();
  conn()->socketHandler()->cancelTimeout();

  if (callbacks_.post_operation_callback_) {
    callbacks_.post_operation_callback_(*this);
  }

  specializedCompleteOperation();

  // call observer callback
  if (observer_callback_) {
    observer_callback_(*this);
  }

  client()->deferRemoveOperation(this);
}

std::unique_ptr<Connection>&& Operation::releaseConnection() {
  CHECK_THROW(
      state_ == OperationState::Completed ||
          state_ == OperationState::Unstarted,
      db::OperationStateException);
  return std::move(conn_proxy_.releaseConnection());
}

void Operation::snapshotMysqlErrors() {
  MYSQL* mysql = conn()->mysql();
  mysql_errno_ = ::mysql_errno(mysql);
  if (mysql_errno_ != 0) {
    if (mysql_errno_ == CR_TLS_SERVER_NOT_FOUND) {
      mysql_error_ = "Server loadshedded the connection request.";
    } else {
      mysql_error_ = ::mysql_error(mysql);
    }

    mysql_normalize_error_ = mysql_error_;
  }
}

void Operation::setAsyncClientError(
    unsigned int mysql_errno,
    folly::StringPiece msg,
    folly::StringPiece normalizeMsg) {
  if (normalizeMsg.empty()) {
    normalizeMsg = msg;
  }
  mysql_errno_ = mysql_errno;
  mysql_error_ = msg.toString();
  mysql_normalize_error_ = normalizeMsg.toString();
}

void Operation::wait() {
  return conn()->wait();
}

MysqlClientBase* Operation::client() const {
  return mysql_client_;
}

std::shared_ptr<Operation> Operation::getSharedPointer() {
  return shared_from_this();
}

const std::string& Operation::host() const {
  return conn()->host();
}
int Operation::port() const {
  return conn()->port();
}

void Operation::setObserverCallback(ObserverCallback obs_cb) {
  CHECK_THROW(state_ == OperationState::Unstarted, db::OperationStateException);
  // allow more callbacks to be set
  if (observer_callback_) {
    auto old_dbs_cb = std::move(observer_callback_);
    observer_callback_ = [obs = std::move(obs_cb),
                          old_obs = std::move(old_dbs_cb)](Operation& op) {
      obs(op);
      old_obs(op);
    };
  } else {
    observer_callback_ = std::move(obs_cb);
  }
}

ChainedCallback Operation::setCallback(
    ChainedCallback orgCallback,
    ChainedCallback newCallback) {
  if (!orgCallback) {
    return newCallback;
  }

  if (!newCallback) {
    return orgCallback;
  }

  return [orgCallback = std::move(orgCallback),
          newCallback = std::move(newCallback)](Operation& op) mutable {
    orgCallback(op);
    newCallback(op);
  };
}

void Operation::setPreOperationCallback(ChainedCallback chainedCallback) {
  callbacks_.pre_operation_callback_ = setCallback(
      std::move(callbacks_.pre_operation_callback_),
      std::move(chainedCallback));
}

void Operation::setPostOperationCallback(ChainedCallback chainedCallback) {
  callbacks_.post_operation_callback_ = setCallback(
      std::move(callbacks_.post_operation_callback_),
      std::move(chainedCallback));
}

AsyncPreQueryCallback Operation::appendCallback(
    AsyncPreQueryCallback&& callback1,
    AsyncPreQueryCallback&& callback2) {
  if (!callback1) {
    return std::move(callback2);
  }

  if (!callback2) {
    return std::move(callback1);
  }

  return [callback1 = std::move(callback1),
          callback2 = std::move(callback2)](FetchOperation& op) {
    return callback1(op).deferValue(
        [&op, callback2](auto&& /* unused */) { return callback2(op); });
  };
}

AsyncPostQueryCallback Operation::appendCallback(
    AsyncPostQueryCallback&& callback1,
    AsyncPostQueryCallback&& callback2) {
  if (!callback1) {
    return std::move(callback2);
  }

  if (!callback2) {
    return std::move(callback1);
  }

  return [callback1 = std::move(callback1),
          callback2 = std::move(callback2)](auto&& result) {
    return callback1(std::move(result)).deferValue([callback2](auto&& result) {
      return callback2(std::move(result));
    });
  };
}

void Operation::setPreQueryCallback(AsyncPreQueryCallback&& callback) {
  callbacks_.pre_query_callback_ = appendCallback(
      std::move(callbacks_.pre_query_callback_), std::move(callback));
}

void Operation::setPostQueryCallback(AsyncPostQueryCallback&& callback) {
  callbacks_.post_query_callback_ = appendCallback(
      std::move(callbacks_.post_query_callback_), std::move(callback));
}

void ConnectTcpTimeoutHandler::timeoutExpired() noexcept {
  op_->tcpConnectTimeoutTriggered();
}

ConnectOperation::ConnectOperation(
    MysqlClientBase* mysql_client,
    ConnectionKey conn_key)
    : Operation(Operation::ConnectionProxy(Operation::OwnedConnection(
          mysql_client->createConnection(conn_key, nullptr)))),
      conn_key_(std::move(conn_key)),
      flags_(CLIENT_MULTI_STATEMENTS),
      active_in_client_(true),
      tcp_timeout_handler_(mysql_client->getEventBase(), this) {
  mysql_client->activeConnectionAdded(&conn_key_);
}

ConnectOperation* ConnectOperation::setConnectionOptions(
    const ConnectionOptions& conn_opts) {
  setTimeout(conn_opts.getTimeout());
  setDefaultQueryTimeout(conn_opts.getQueryTimeout());
  setAttributes(conn_opts.getAttributes());
  setConnectAttempts(conn_opts.getConnectAttempts());
  if (conn_opts.getDscp().has_value()) {
    setDscp(*conn_opts.getDscp());
  }
  setTotalTimeout(conn_opts.getTotalTimeout());
  setCompression(conn_opts.getCompression());
  auto provider = conn_opts.getSSLOptionsProvider();
  if (conn_opts.getConnectTcpTimeout()) {
    setTcpTimeout(*conn_opts.getConnectTcpTimeout());
  }
  if (conn_opts.getSniServerName()) {
    setSniServerName(*conn_opts.getSniServerName());
  }
  if (provider) {
    setSSLOptionsProvider(std::move(provider));
  }
  if (conn_opts.getCertValidationCallback()) {
    setCertValidationCallback(
        conn_opts.getCertValidationCallback(),
        conn_opts.getCertValidationContext(),
        conn_opts.isOpPtrAsValidationContext());
  }
  return this;
}

const ConnectionOptions& ConnectOperation::getConnectionOptions() const {
  return conn_options_;
}

ConnectOperation* ConnectOperation::setDefaultQueryTimeout(Duration t) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setQueryTimeout(t);
  return this;
}

ConnectOperation* ConnectOperation::setSniServerName(
    const std::string& sni_servername) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setSniServerName(sni_servername);
  return this;
}

ConnectOperation* ConnectOperation::enableResetConnBeforeClose() {
  conn_options_.enableResetConnBeforeClose();
  return this;
}

ConnectOperation* ConnectOperation::enableDelayedResetConn() {
  conn_options_.enableDelayedResetConn();
  return this;
}

ConnectOperation* ConnectOperation::enableChangeUser() {
  conn_options_.enableChangeUser();
  return this;
}

ConnectOperation* ConnectOperation::setCertValidationCallback(
    CertValidatorCallback callback,
    const void* context,
    bool opPtrAsContext) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setCertValidationCallback(
      std::move(callback), context, opPtrAsContext);
  return this;
}

ConnectOperation* ConnectOperation::setTimeout(Duration timeout) {
  conn_options_.setTimeout(timeout);
  Operation::setTimeout(timeout);
  return this;
}

ConnectOperation* ConnectOperation::setTcpTimeout(Duration timeout) {
  conn_options_.setConnectTcpTimeout(timeout);
  return this;
}

ConnectOperation* ConnectOperation::setTotalTimeout(Duration total_timeout) {
  conn_options_.setTotalTimeout(total_timeout);
  Operation::setTimeout(min(timeout_, total_timeout));
  return this;
}
ConnectOperation* ConnectOperation::setConnectAttempts(uint32_t max_attempts) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setConnectAttempts(max_attempts);
  return this;
}

ConnectOperation* ConnectOperation::setDscp(uint8_t dscp) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setDscp(dscp);
  return this;
}

ConnectOperation* ConnectOperation::setKillOnQueryTimeout(
    bool killOnQueryTimeout) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  killOnQueryTimeout_ = killOnQueryTimeout;
  return this;
}
ConnectOperation* ConnectOperation::setSSLOptionsProviderBase(
    std::unique_ptr<SSLOptionsProviderBase> /*ssl_options_provider*/) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  LOG(ERROR) << "Using deprecated function";
  return this;
}
ConnectOperation* ConnectOperation::setSSLOptionsProvider(
    std::shared_ptr<SSLOptionsProviderBase> ssl_options_provider) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setSSLOptionsProvider(ssl_options_provider);
  return this;
}

bool ConnectOperation::shouldCompleteOperation(OperationResult result) {
  // Cancelled doesn't really get to this point, the Operation is forced to
  // complete by Operation, adding this check here just-in-case.
  if (attempts_made_ >= conn_options_.getConnectAttempts() ||
      result == OperationResult::Cancelled) {
    return true;
  }

  auto now = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
  if (now > start_time_ + conn_options_.getTotalTimeout()) {
    return true;
  }

  return false;
}

void ConnectOperation::attemptFailed(OperationResult result) {
  ++attempts_made_;
  if (shouldCompleteOperation(result)) {
    completeOperation(result);
    return;
  }

  // We need to update end_time_ here because the logging function uses this
  // in calculating duration time.
  end_time_ = std::chrono::steady_clock::now();
  logConnectCompleted(result);

  tcp_timeout_handler_.cancelTimeout();

  conn()->socketHandler()->unregisterHandler();
  conn()->socketHandler()->cancelTimeout();
  conn()->close();

  auto now = std::chrono::steady_clock::now();
  // Adjust timeout
  std::chrono::duration<uint64_t, std::micro> timeout_attempt_based =
      conn_options_.getTimeout() +
      std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_);
  timeout_ = min(timeout_attempt_based, conn_options_.getTotalTimeout());
  specializedRun();
}

void ConnectOperation::attemptSucceeded(OperationResult result) {
  ++attempts_made_;
  completeOperation(result);
}

void ConnectOperation::specializedRunImpl() {
  if (attempts_made_ == 0) {
    conn()->initialize();
  } else {
    conn()->initMysqlOnly();
  }
  removeClientReference();
  MYSQL* mysql = conn()->mysql();
  if (!mysql) {
    setAsyncClientError(
        static_cast<uint16_t>(SquangleErrno::SQ_INITIALIZATION_FAILED),
        "Connection initialization failed. Ran out of Memory?");
    attemptFailed(OperationResult::Failed);
    return;
  }

  mysql_options(conn()->mysql(), MYSQL_OPT_CONNECT_ATTR_RESET, 0);
  for (const auto& [key, value] : attributes_) {
    mysql_options4(
        conn()->mysql(),
        MYSQL_OPT_CONNECT_ATTR_ADD,
        key.c_str(),
        value.c_str());
  }

  const auto& compression_lib = getCompression();
  if (compression_lib) {
    mysql_options(mysql, MYSQL_OPT_COMPRESS, nullptr);
    setCompressionOption(mysql, *compression_lib);
  }

  auto provider = conn_options_.getSSLOptionsProviderPtr();
  if (provider && provider->setMysqlSSLOptions(mysql)) {
    if (connection_context_) {
      connection_context_->isSslConnection = true;
    }
  }

  // Set sni field for ssl connection
  if (conn_options_.getSniServerName()) {
    mysql_options(
        mysql,
        MYSQL_OPT_TLS_SNI_SERVERNAME,
        (*conn_options_.getSniServerName()).c_str());
  }

  if (conn_options_.getDscp().has_value()) {
    // DS field (QOS/TOS level) is 8 bits with DSCP packed into the most
    // significant 6 bits.
    uint dsf = *conn_options_.getDscp() << 2;
    int ret = mysql_options(mysql, MYSQL_OPT_TOS, &dsf);
    if (ret != 0) {
      LOG(WARNING) << fmt::format(
          "Failed to set DSCP {} for MySQL Client socket",
          *conn_options_.getDscp());
    }
  }

  if (conn_options_.getCertValidationCallback()) {
    MysqlCertValidatorCallback callback = mysqlCertValidator;
    const void* self = this;
    mysql_options(mysql, MYSQL_OPT_TLS_CERT_CALLBACK, &callback);
    mysql_options(mysql, MYSQL_OPT_TLS_CERT_CALLBACK_CONTEXT, &self);
  }

  conn()->socketHandler()->setOperation(this);

  // If the tcp timeout value is not set in conn options, use the default value
  uint timeoutInMs = chrono::duration_cast<chrono::milliseconds>(
                         conn_options_.getConnectTcpTimeout().value_or(Duration(
                             FLAGS_async_mysql_connect_tcp_timeout_micros)))
                         .count();
  // Set the connect timeout in mysql options and also on tcp_timeout_handler if
  // event base is set. Sync implmenation of MysqlClientBase may not have it
  // set. If the timeout is set to 0, skip setting any timeout
  if (timeoutInMs != 0) {
    mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT_MS, &timeoutInMs);
    if (isEventBaseSet()) {
      tcp_timeout_handler_.scheduleTimeout(timeoutInMs);
    }
  }

  // connect is immediately "ready" to do one loop
  socketActionable();
}

ConnectOperation* ConnectOperation::specializedRun() {
  if (!connection()->runInThread(this, &ConnectOperation::specializedRunImpl)) {
    completeOperationInner(OperationResult::Failed);
  }
  return this;
}

ConnectOperation::~ConnectOperation() {
  removeClientReference();
}

void ConnectOperation::socketActionable() {
  DCHECK(isInEventBaseThread());

  folly::stop_watch<Duration> sw;
  auto logThreadBlockTimeGuard =
      std::make_unique<OperationGuard>([&]() { logThreadBlockTime(sw); });

  auto& handler = conn()->client()->getMysqlHandler();
  MYSQL* mysql = conn()->mysql();
  const auto usingUnixSocket = !conn_key_.unixSocketPath().empty();

  auto status = handler.tryConnect(mysql, conn_options_, conn_key_, flags_);

  if (status == MysqlHandler::ERROR) {
    snapshotMysqlErrors();
    logThreadBlockTimeGuard.reset();
    attemptFailed(OperationResult::Failed);
  } else {
    if ((isDoneWithTcpHandShake() || usingUnixSocket) &&
        tcp_timeout_handler_.isScheduled()) {
      // cancel tcp connect timeout
      tcp_timeout_handler_.cancelTimeout();
    }

    auto fd = mysql_get_socket_descriptor(mysql);
    if (fd <= 0) {
      LOG(ERROR) << "Unexpected invalid socket descriptor on completed, "
                 << (status == MysqlHandler::DONE ? "errorless" : "pending")
                 << " connect.  fd=" << fd;
      setAsyncClientError(
          static_cast<uint16_t>(SquangleErrno::SQ_INITIALIZATION_FAILED),
          "mysql_get_socket_descriptor returned an invalid "
          "descriptor");
      logThreadBlockTimeGuard.reset();
      attemptFailed(OperationResult::Failed);
    } else if (status == MysqlHandler::DONE) {
      auto socket = folly::NetworkSocket::fromFd(fd);
      conn()->socketHandler()->changeHandlerFD(socket);
      conn()->mysqlConnection()->setConnectionContext(connection_context_);
      conn()->mysqlConnection()->connectionOpened();
      logThreadBlockTimeGuard.reset();
      attemptSucceeded(OperationResult::Succeeded);
    } else {
      conn()->socketHandler()->changeHandlerFD(
          folly::NetworkSocket::fromFd(fd));
      waitForSocketActionable();
    }
  }
}

bool ConnectOperation::isDoneWithTcpHandShake() {
  enum connect_stage stage = mysql_get_connect_stage(conn()->mysql());
  return stage > tcpCompletionStage_;
}

void ConnectOperation::specializedTimeoutTriggered() {
  timeoutHandler(false);
}

void ConnectOperation::tcpConnectTimeoutTriggered() {
  if (!isDoneWithTcpHandShake()) {
    timeoutHandler(true);
  }
  // else  do nothing since we have made progress
}

void ConnectOperation::timeoutHandler(
    bool isTcpTimeout,
    bool isPoolConnection) {
  auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start_time_);

  auto cbDelayUs = client()->callbackDelayMicrosAvg();
  bool stalled = (cbDelayUs >= kCallbackDelayStallThresholdUs);

  // Overall the message looks like this:
  //   [<errno>](Mysql Client) Connect[Pool] to <host>:<port> timed out
  //   [at stage <connect_stage>] (took Nms, timeout was Nms)
  //   [(CLIENT_OVERLOADED: cb delay Nms, N active conns)] [TcpTimeout:N]
  std::vector<std::string> parts;
  parts.push_back(fmt::format(
      "[{}]({})Connect{} to {}:{} timed out",
      static_cast<uint16_t>(
          stalled ? SquangleErrno::SQ_ERRNO_CONN_TIMEOUT_LOOP_STALLED
                  : SquangleErrno::SQ_ERRNO_CONN_TIMEOUT),
      kErrorPrefix,
      isPoolConnection ? "Pool" : "",
      host(),
      port()));
  if (!isPoolConnection) {
    parts.push_back(fmt::format(
        "at stage {}",
        connectStageString(mysql_get_connect_stage(conn()->mysql()))));
  }

  parts.push_back(timeoutMessage(delta));
  if (stalled) {
    parts.push_back(threadOverloadMessage(cbDelayUs));
  }
  parts.push_back(fmt::format("(TcpTimeout:{})", (isTcpTimeout ? 1 : 0)));

  setAsyncClientError(
      CR_SERVER_LOST,
      folly::join(" ", parts),
      fmt::format("Connect timed out{}", stalled ? " (loop stalled)" : ""));
  attemptFailed(OperationResult::TimedOut);
}

void ConnectOperation::logConnectCompleted(OperationResult result) {
  // If the connection wasn't initialized, it's because the operation
  // was cancelled before anything started, so we don't do the logs
  if (!conn()->hasInitialized()) {
    return;
  }
  auto* context = connection_context_.get();
  auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time_ - start_time_);
  if (result == OperationResult::Succeeded) {
    if (context) {
      context->sslVersion = conn()->getTlsVersion();
    }
    client()->logConnectionSuccess(
        db::CommonLoggingData(
            getOperationType(),
            elapsed,
            timeout_,
            getMaxThreadBlockTime(),
            getTotalThreadBlockTime()),
        *conn()->getKey(),
        context);
  } else {
    db::FailureReason reason = db::FailureReason::DATABASE_ERROR;
    if (result == OperationResult::TimedOut) {
      reason = db::FailureReason::TIMEOUT;
    } else if (result == OperationResult::Cancelled) {
      reason = db::FailureReason::CANCELLED;
    }
    client()->logConnectionFailure(
        db::CommonLoggingData(
            getOperationType(),
            elapsed,
            timeout_,
            getMaxThreadBlockTime(),
            getTotalThreadBlockTime()),
        reason,
        *conn()->getKey(),
        mysql_errno(),
        mysql_error(),
        context);
  }
}

void ConnectOperation::maybeStoreSSLSession() {
  // if there is an ssl provider set
  auto provider = conn_options_.getSSLOptionsProviderPtr();
  if (!provider) {
    return;
  }

  // If connection was successful
  if (result_ != OperationResult::Succeeded || !conn()->hasInitialized()) {
    return;
  }

  if (provider->storeMysqlSSLSession(conn()->mysql())) {
    if (connection_context_) {
      connection_context_->sslSessionReused = true;
    }
    client()->stats()->incrReusedSSLSessions();
  }
}

void ConnectOperation::specializedCompleteOperation() {
  // Pass the callbacks to the Connection now that we are done with them
  conn()->setCallbacks(std::move(callbacks_));

  // Operations that don't directly initiate a new TLS conneciton
  // shouldn't update the TLS session because it can propagate the
  // session object from a connection created usisn one client cert
  // to an SSL provider initialized with a different cert.
  if (getOperationType() == db::OperationType::Connect) {
    maybeStoreSSLSession();
  }

  // Can only log this on successful connections because unsuccessful
  // ones call mysql_close_free inside libmysql
  if (result_ == OperationResult::Succeeded && conn()->ok() &&
      connection_context_) {
    connection_context_->endpointVersion = conn()->serverInfo();
  }

  // Cancel tcp timeout
  tcp_timeout_handler_.cancelTimeout();

  logConnectCompleted(result_);

  // If connection_initialized_ is false the only way to complete the
  // operation is by cancellation
  DCHECK(conn()->hasInitialized() || result_ == OperationResult::Cancelled);

  conn()->setConnectionOptions(conn_options_);
  conn()->setKillOnQueryTimeout(getKillOnQueryTimeout());
  conn()->setConnectionContext(connection_context_);

  conn()->notify();

  if (connect_callback_) {
    connect_callback_(*this);
    // Release callback since no other callbacks will be made
    connect_callback_ = nullptr;
  }
  // In case this operation didn't even get the chance to run, we still need
  // to remove the reference it added to the async client
  removeClientReference();
}

void ConnectOperation::mustSucceed() {
  run();
  wait();
  if (!ok()) {
    throw db::RequiredOperationFailedException(
        "Connect failed: " + mysql_error_);
  }
}

void ConnectOperation::removeClientReference() {
  if (active_in_client_) {
    // It's safe to call the client since we still have a ref counting
    // it won't die before it goes to 0
    active_in_client_ = false;
    client()->activeConnectionRemoved(&conn_key_);
  }
}

int ConnectOperation::mysqlCertValidator(
    X509* server_cert,
    const void* context,
    const char** errptr) {
  ConnectOperation* self =
      reinterpret_cast<ConnectOperation*>(const_cast<void*>(context));
  CHECK_NOTNULL(self);

  // Hold a shared pointer to the Operation object while running the callback
  auto weak_self = self->weak_from_this();
  if (weak_self.expired()) {
    LOG(ERROR) << "ConnectOperation object " << self
               << " is already deallocated";
    return 0;
  }
  auto guard = weak_self.lock();

  const CertValidatorCallback callback =
      self->conn_options_.getCertValidationCallback();
  CHECK(callback);
  const void* callbackContext = self->conn_options_.isOpPtrAsValidationContext()
      ? self
      : self->conn_options_.getCertValidationContext();
  folly::StringPiece errorMessage;

  // "libmysql" expects this callback to return "0" if the cert validation was
  // successful, and return "1" if validation failed.
  int result = callback(server_cert, callbackContext, errorMessage) ? 0 : 1;
  if (!errorMessage.empty()) {
    *errptr = errorMessage.data();
  }
  return result;
}

FetchOperation::FetchOperation(
    ConnectionProxy&& conn,
    std::vector<Query>&& queries)
    : Operation(std::move(conn)), queries_(std::move(queries)) {}

FetchOperation::FetchOperation(ConnectionProxy&& conn, MultiQuery&& multi_query)
    : Operation(std::move(conn)), queries_(std::move(multi_query)) {}

FetchOperation* FetchOperation::setUseChecksum(bool useChecksum) noexcept {
  use_checksum_ = useChecksum;
  return this;
}

bool FetchOperation::isStreamAccessAllowed() const {
  // XOR if isPaused or the caller is coming from IO Thread
  return isPaused() || isInEventBaseThread();
}

bool FetchOperation::isPaused() const {
  return active_fetch_action_ == FetchAction::WaitForConsumer;
}

FetchOperation* FetchOperation::specializedRun() {
  if (!connection()->runInThread(this, &FetchOperation::specializedRunImpl)) {
    completeOperationInner(OperationResult::Failed);
  }

  return this;
}

void FetchOperation::specializedRunImpl() {
  try {
    MYSQL* mysql = conn()->mysql();
    rendered_query_ = queries_.renderQuery(mysql);

    mysql_options(mysql, MYSQL_OPT_QUERY_ATTR_RESET, 0);
    for (const auto& [key, value] : attributes_) {
      if (int retErrCode = setQueryAttribute(key, value)) {
        setAsyncClientError(
            retErrCode,
            fmt::format("Failed to set query attribute: {} = {}", key, value));
        completeOperation(OperationResult::Failed);
        return;
      }
    }

    if (use_checksum_ || conn()->getConnectionOptions().getUseChecksum()) {
      if (int retErrCode = setQueryAttribute(kQueryChecksumKey, "ON")) {
        setAsyncClientError(retErrCode, "Failed to set checksum = ON");
        completeOperation(OperationResult::Failed);
        return;
      }
    }
    socketActionable();
  } catch (std::invalid_argument& e) {
    setAsyncClientError(
        static_cast<uint16_t>(SquangleErrno::SQ_INVALID_API_USAGE),
        std::string("Unable to parse Query: ") + e.what(),
        "Unable to parse Query");
    completeOperation(OperationResult::Failed);
  }
}

int FetchOperation::setQueryAttribute(
    const std::string& key,
    const std::string& value) {
  return mysql_options4(
      conn()->mysql(), MYSQL_OPT_QUERY_ATTR_ADD, key.c_str(), value.c_str());
}

FetchOperation::RowStream::RowStream(
    MYSQL_RES* mysql_query_result,
    MysqlHandler* handler)
    : mysql_query_result_(mysql_query_result),
      row_fields_(
          mysql_fetch_fields(mysql_query_result),
          mysql_num_fields(mysql_query_result)),
      handler_(handler) {}

EphemeralRow FetchOperation::RowStream::consumeRow() {
  if (!current_row_.has_value()) {
    LOG(DFATAL) << "Illegal operation";
  }
  EphemeralRow eph_row(std::move(*current_row_));
  current_row_.reset();
  return eph_row;
}

bool FetchOperation::RowStream::hasNext() {
  // Slurp needs to happen after `consumeRow` has been called.
  // Because it will move the buffer.
  slurp();
  // First iteration
  return current_row_.has_value();
}

bool FetchOperation::RowStream::slurp() {
  CHECK_THROW(mysql_query_result_ != nullptr, db::OperationStateException);
  if (current_row_.has_value() || query_finished_) {
    return true;
  }
  MYSQL_ROW row;
  auto status = handler_->fetchRow(mysql_query_result_.get(), row);
  if (status == MysqlHandler::PENDING) {
    return false;
  }
  if (row == nullptr) {
    query_finished_ = true;
    return true;
  }
  unsigned long* field_lengths = mysql_fetch_lengths(mysql_query_result_.get());
  current_row_.assign(EphemeralRow(row, field_lengths, &row_fields_));
  query_result_size_ += current_row_->calculateRowLength();
  ++num_rows_seen_;
  return true;
}

void FetchOperation::setFetchAction(FetchAction action) {
  if (isPaused()) {
    paused_action_ = action;
  } else {
    active_fetch_action_ = action;
  }
}

uint64_t FetchOperation::currentLastInsertId() const {
  CHECK_THROW(isStreamAccessAllowed(), db::OperationStateException);
  return current_last_insert_id_;
}

uint64_t FetchOperation::currentAffectedRows() const {
  CHECK_THROW(isStreamAccessAllowed(), db::OperationStateException);
  return current_affected_rows_;
}

const std::string& FetchOperation::currentRecvGtid() const {
  CHECK_THROW(isStreamAccessAllowed(), db::OperationStateException);
  return current_recv_gtid_;
}

const AttributeMap& FetchOperation::currentRespAttrs() const {
  CHECK_THROW(isStreamAccessAllowed(), db::OperationStateException);
  return current_resp_attrs_;
}

FetchOperation::RowStream* FetchOperation::rowStream() {
  CHECK_THROW(isStreamAccessAllowed(), db::OperationStateException);
  return current_row_stream_.get_pointer();
}

static folly::Optional<std::pair<std::string, std::string>>
readFirstResponseAtribute(MYSQL* mysql) {
  size_t len;
  const char* data;

  if (!mysql_session_track_get_first(
          mysql, SESSION_TRACK_RESP_ATTR, &data, &len)) {
    auto key = std::string(data, len);
    if (!mysql_session_track_get_next(
            mysql, SESSION_TRACK_RESP_ATTR, &data, &len)) {
      return std::make_pair(std::move(key), std::string(data, len));
    }
  }

  return folly::none;
}

static folly::Optional<std::pair<std::string, std::string>>
readNextResponseAtribute(MYSQL* mysql) {
  size_t len;
  const char* data;

  if (!mysql_session_track_get_next(
          mysql, SESSION_TRACK_RESP_ATTR, &data, &len)) {
    auto key = std::string(data, len);
    if (!mysql_session_track_get_next(
            mysql, SESSION_TRACK_RESP_ATTR, &data, &len)) {
      return std::make_pair(std::move(key), std::string(data, len));
    }
  }

  return folly::none;
}

AttributeMap FetchOperation::readResponseAttributes() {
  AttributeMap attrs;
  MYSQL* mysql = conn()->mysql();
  for (auto attr = readFirstResponseAtribute(mysql); attr;
       attr = readNextResponseAtribute(mysql)) {
    attrs[std::move(attr->first)] = std::move(attr->second);
  }

  return attrs;
}

void FetchOperation::socketActionable() {
  DCHECK(isInEventBaseThread());
  DCHECK(active_fetch_action_ != FetchAction::WaitForConsumer);

  folly::stop_watch<Duration> sw;
  auto logThreadBlockTimeGuard =
      std::make_unique<OperationGuard>([&]() { logThreadBlockTime(sw); });

  auto& handler = conn()->client()->getMysqlHandler();
  MYSQL* mysql = conn()->mysql();

  // This loop runs the fetch actions required to successfully execute query,
  // request next results, fetch results, identify errors and complete operation
  // and queries.
  // All callbacks are done in the `notify` methods that children must
  // override. During callbacks for actions `Fetch` and `CompleteQuery`,
  // the consumer is allowed to pause the operation.
  // Some actions may request an action above it (like CompleteQuery may request
  // StartQuery) this is why we use this loop.
  while (1) {
    // When the fetch action is StartQuery it means either we need to execute
    // the query or ask for new results.
    // Next Actions:
    //  - StartQuery: may continue with StartQuery if socket not actionable, in
    //                this case socketActionable is exited;
    //  - CompleteOperation: if it fails to execute query or request next
    //                       results.
    //  - InitFetch: no errors during results request, so we initiate fetch.
    if (active_fetch_action_ == FetchAction::StartQuery) {
      auto status = MysqlHandler::PENDING;

      if (query_executed_) {
        ++num_current_query_;
        status = handler.nextResult(mysql);
      } else {
        status = handler.runQuery(mysql, rendered_query_);
      }

      if (status == MysqlHandler::PENDING) {
        waitForSocketActionable();
        return;
      }

      current_last_insert_id_ = 0;
      current_affected_rows_ = 0;
      current_recv_gtid_ = std::string();
      query_executed_ = true;
      if (status == MysqlHandler::ERROR) {
        active_fetch_action_ = FetchAction::CompleteQuery;
      } else {
        active_fetch_action_ = FetchAction::InitFetch;
      }
    }

    // Prior fetch start we read the values that may indicate errors, rows to
    // fetch or not. The initialize from children classes is called either way
    // to signal that any other calls from now are regarding a new query.
    // Next Actions:
    //  - CompleteOperation: in case an error occurred
    //  - Fetch: there are rows to fetch in this query
    //  - CompleteQuery: no rows to fetch (complete query will read rowsAffected
    //                   and lastInsertId to add to result
    if (active_fetch_action_ == FetchAction::InitFetch) {
      auto* mysql_query_result = handler.getResult(mysql);
      auto num_fields = mysql_field_count(mysql);

      // Check to see if this an empty query or an error
      if (!mysql_query_result && num_fields > 0) {
        // Failure. CompleteQuery will read errors.
        active_fetch_action_ = FetchAction::CompleteQuery;
      } else {
        if (num_fields > 0) {
          current_row_stream_.assign(RowStream(mysql_query_result, &handler));
          active_fetch_action_ = FetchAction::Fetch;
        } else {
          active_fetch_action_ = FetchAction::CompleteQuery;
        }
        notifyInitQuery();
      }
    }

    // This action is going to stick around until all rows are fetched or an
    // error occurs. When the RowStream is ready, we notify the subclasses for
    // them to consume it.
    // If `pause` is called during the callback and the stream is consumed then,
    // `row_stream_` is checked and we skip to the next action `CompleteQuery`.
    // If row_stream_ isn't ready, we wait for socket actionable.
    // Next Actions:
    //  - Fetch: in case it needs to fetch more rows, we break the loop and wait
    //           for socketActionable to be called again
    //  - CompleteQuery: an error occurred or rows finished to fetch
    //  - WaitForConsumer: in case `pause` is called during `notifyRowsReady`
    if (active_fetch_action_ == FetchAction::Fetch) {
      DCHECK(current_row_stream_.has_value());
      // Try to catch when the user didn't pause or consumed the rows
      if (current_row_stream_->current_row_.has_value()) {
        // This should help
        LOG(ERROR) << "Rows not consumed. Perhaps missing `pause`?";
        cancel_ = true;
        active_fetch_action_ = FetchAction::CompleteQuery;
        continue;
      }

      // When the query finished, `is_ready` is true, but there are no rows.
      bool is_ready = current_row_stream_->slurp();
      if (!is_ready) {
        waitForSocketActionable();
        break;
      }
      if (current_row_stream_->hasQueryFinished()) {
        active_fetch_action_ = FetchAction::CompleteQuery;
      } else {
        notifyRowsReady();
      }
    }

    // In case the query has at least started and finished by error or not,
    // here the final checks and data are gathered for the current query.
    // It checks if any errors occurred during query, and call children classes
    // to deal with their specialized query completion.
    // If `pause` is called, then `paused_action_` will be already `StartQuery`
    // or `CompleteOperation`.
    // Next Actions:
    //  - StartQuery: There are more results and children is not opposed to it.
    //                QueryOperation child sets to CompleteOperation, since it
    //                is not supposed to receive more than one result.
    //  - CompleteOperation: In case an error occurred during query or there are
    //                       no more results to read.
    //  - WaitForConsumer: In case `pause` is called during notification.
    if (active_fetch_action_ == FetchAction::CompleteQuery) {
      snapshotMysqlErrors();

      bool more_results = false;
      if (mysql_errno_ != 0 || cancel_) {
        active_fetch_action_ = FetchAction::CompleteOperation;
      } else {
        current_last_insert_id_ = mysql_insert_id(mysql);
        current_affected_rows_ = mysql_affected_rows(mysql);
        const char* data;
        size_t length;
        if (!mysql_session_track_get_first(
                mysql, SESSION_TRACK_GTIDS, &data, &length)) {
          current_recv_gtid_ = std::string(data, length);
        }
        if (!mysql_session_track_get_first(
                mysql, SESSION_TRACK_SCHEMA, &data, &length)) {
          conn()->setCurrentSchema(std::string_view(data, length));
        }
        current_resp_attrs_ = readResponseAttributes();
        more_results = mysql_more_results(mysql);
        active_fetch_action_ = more_results ? FetchAction::StartQuery
                                            : FetchAction::CompleteOperation;

        // Call it after setting the active_fetch_action_ so the child class can
        // decide if it wants to change the state

        if (current_row_stream_ && current_row_stream_->mysql_query_result_) {
          rows_received_ +=
              mysql_num_rows(current_row_stream_->mysql_query_result_.get());
          total_result_size_ += current_row_stream_->query_result_size_;
        }
        ++num_queries_executed_;
        no_index_used_ |= mysql->server_status & SERVER_QUERY_NO_INDEX_USED;
        was_slow_ |= mysql->server_status & SERVER_QUERY_WAS_SLOW;
        notifyQuerySuccess(more_results);
      }
      current_row_stream_.reset();
    }

    // Once this action is set, the operation is going to be completed no matter
    // the reason it was called. It exists the loop.
    if (active_fetch_action_ == FetchAction::CompleteOperation) {
      logThreadBlockTimeGuard.reset();
      if (cancel_) {
        state_ = OperationState::Cancelling;
        completeOperation(OperationResult::Cancelled);
      } else if (mysql_errno_ != 0) {
        completeOperation(OperationResult::Failed);
      } else {
        completeOperation(OperationResult::Succeeded);
      }
      break;
    }

    // If `pause` is called during the operation callbacks, this the Action it
    // should come to.
    // It's not necessary to unregister the socket event,  so just cancel the
    // timeout and wait for `resume` to be called.
    if (active_fetch_action_ == FetchAction::WaitForConsumer) {
      conn()->socketHandler()->cancelTimeout();
      break;
    }
  }
}

void FetchOperation::pauseForConsumer() {
  DCHECK(isInEventBaseThread());
  DCHECK(state() == OperationState::Pending);

  paused_action_ = active_fetch_action_;
  active_fetch_action_ = FetchAction::WaitForConsumer;
}

void FetchOperation::resumeImpl() {
  CHECK_THROW(isPaused(), db::OperationStateException);

  // We should only allow pauses during fetch or between queries.
  // If we come back as RowsFetched and the stream has completed the query,
  // `socketActionable` will change the `active_fetch_action_` and we will
  // start the Query completion process.
  // When we pause between queries, the value of `paused_action_` is already
  // the value of the next states: StartQuery or CompleteOperation.
  active_fetch_action_ = paused_action_;
  // Leave timeout to be reset or checked when we hit
  // `waitForSocketActionable`
  socketActionable();
}

void FetchOperation::resume() {
  DCHECK(active_fetch_action_ == FetchAction::WaitForConsumer);
  connection()->runInThread(this, &FetchOperation::resumeImpl);
}

namespace {
void copyRowToRowBlock(RowBlock* block, const EphemeralRow& eph_row) {
  block->startRow();
  for (int i = 0; i < eph_row.numFields(); ++i) {
    if (eph_row.isNull(i)) {
      block->appendNull();
    } else {
      block->appendValue(eph_row[i]);
    }
  }
  block->finishRow();
}

RowBlock makeRowBlockFromStream(
    std::shared_ptr<RowFields> row_fields,
    FetchOperation::RowStream* row_stream) {
  RowBlock row_block(std::move(row_fields));
  // Consume row_stream
  while (row_stream->hasNext()) {
    auto eph_row = row_stream->consumeRow();
    copyRowToRowBlock(&row_block, eph_row);
  }
  return row_block;
}
} // namespace

void FetchOperation::specializedTimeoutTriggered() {
  DCHECK(active_fetch_action_ != FetchAction::WaitForConsumer);
  auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
      chrono::steady_clock::now() - start_time_);

  if (conn()->getKillOnQueryTimeout()) {
    killRunningQuery();
  }

  /*
   * The MYSQL_RES struct contains a handle to the MYSQL struct that created
   * it. Currently, calling mysql_free_result attempts to flush the buffer in
   * accordance with the protocol. This makes it so that if a MYSQL_RES is
   * freed during a query and before the entire result is read, then the
   * subsequent queries sent over the same connection will still succeed.
   *
   * In Operation.h it can be seen that mysql_free_result is used to delete
   * the result set, instead of the nonblocking version. The logic to flush
   * the socket is impossible to correctly implement in a destructor, because
   * the function needs to be called repeatedly to ensure all data has been
   * read. Instead we use the code below to detach the result object from the
   * connection, so no network flushing is done.
   *
   * This does not cause a memory leak because the socket will still be cleaned
   * up when the connection is freed. AsyncMySQL also does not provide a way
   * for clients to read half a result, then send more queries. If we allowed
   * partial reads of results, then this strategy would not work. The most
   * common case where we would normally need to flush results is for client
   * query timeouts, where we might still be receiving rows when we interrupt
   * and return an error to the client.
   */
  if (rowStream() && rowStream()->mysql_query_result_) {
    rowStream()->mysql_query_result_->handle = nullptr;
  }

  std::string rows;
  if (rowStream() && rowStream()->numRowsSeen()) {
    rows = fmt::format(
        "({} rows, {} bytes seen)",
        rowStream()->numRowsSeen(),
        rowStream()->query_result_size_);
  } else {
    rows = "(no rows seen)";
  }

  auto cbDelayUs = client()->callbackDelayMicrosAvg();
  bool stalled = cbDelayUs >= kCallbackDelayStallThresholdUs;

  std::vector<std::string> parts;
  parts.push_back(fmt::format(
      "[{}]({}) Query timed out",
      static_cast<uint16_t>(
          stalled ? SquangleErrno::SQ_ERRNO_QUERY_TIMEOUT_LOOP_STALLED
                  : SquangleErrno::SQ_ERRNO_QUERY_TIMEOUT),
      kErrorPrefix));

  parts.push_back(std::move(rows));
  parts.push_back(timeoutMessage(delta));
  if (stalled) {
    parts.push_back(threadOverloadMessage(cbDelayUs));
  }

  setAsyncClientError(
      CR_NET_READ_INTERRUPTED,
      folly::join(" ", parts),
      fmt::format("Query timed out{}", stalled ? " (loop stalled)" : ""));
  completeOperation(OperationResult::TimedOut);
}

void FetchOperation::specializedCompleteOperation() {
  // Stats for query
  if (result_ == OperationResult::Succeeded) {
    // set last successful query time to MysqlConnectionHolder
    conn()->setLastActivityTime(chrono::steady_clock::now());
    db::QueryLoggingData logging_data(
        getOperationType(),
        elapsed(),
        timeout_,
        num_queries_executed_,
        rendered_query_.toString(),
        rows_received_,
        total_result_size_,
        no_index_used_,
        use_checksum_ || conn()->getConnectionOptions().getUseChecksum(),
        attributes_,
        readResponseAttributes(),
        getMaxThreadBlockTime(),
        getTotalThreadBlockTime(),
        was_slow_);
    client()->logQuerySuccess(logging_data, *conn().get());
  } else {
    db::FailureReason reason = db::FailureReason::DATABASE_ERROR;
    if (result_ == OperationResult::Cancelled) {
      reason = db::FailureReason::CANCELLED;
    } else if (result_ == OperationResult::TimedOut) {
      reason = db::FailureReason::TIMEOUT;
    }
    client()->logQueryFailure(
        db::QueryLoggingData(
            getOperationType(),
            elapsed(),
            timeout_,
            num_queries_executed_,
            rendered_query_.toString(),
            rows_received_,
            total_result_size_,
            no_index_used_,
            use_checksum_ || conn()->getConnectionOptions().getUseChecksum(),
            attributes_,
            readResponseAttributes(),
            getMaxThreadBlockTime(),
            getTotalThreadBlockTime(),
            was_slow_),
        reason,
        mysql_errno(),
        mysql_error(),
        *conn().get());
  }

  if (result_ != OperationResult::Succeeded) {
    notifyFailure(result_);
  }
  // This frees the `Operation::wait()` call. We need to free it here because
  // callback can stealConnection and we can't notify anymore.
  conn()->notify();
  notifyOperationCompleted(result_);
}

void FetchOperation::mustSucceed() {
  run();
  wait();
  if (!ok()) {
    throw db::RequiredOperationFailedException("Query failed: " + mysql_error_);
  }
}

void FetchOperation::killRunningQuery() {
  /*
   * Send kill command to terminate the current operation on the DB
   * Note that we use KILL <processlist_id> to kill the entire connection
   * In the event the DB is behind a proxy this will kill the persistent
   * connection the proxy is using, so ConnectionOptions::killQueryOnTimeout_
   * should always be false when accessing the DB through a proxy
   *
   * Note that there is a risk of a race condition in the event that a proxy
   * is used and a query from this client times out, then the query completes
   * almost immediately after the timeout and a proxy gives the persistent
   * connection to another client which begins a query on that connection
   * before this client is able to send the KILL query on a separate
   * proxy->db connection which then terminates the OTHER client's query
   */
  auto thread_id = conn()->mysqlThreadId();
  auto host = conn()->host();
  auto port = conn()->port();
  auto conn_op = client()
                     ->beginConnection(*conn()->getKey())
                     ->setConnectionOptions(conn()->getConnectionOptions());
  conn_op->setCallback([thread_id, host, port](ConnectOperation& conn_op) {
    if (conn_op.ok()) {
      auto op = Connection::beginQuery(
          conn_op.releaseConnection(), "KILL %d", thread_id);
      op->setCallback([thread_id, host, port](
                          QueryOperation& /* unused */,
                          QueryResult* /* unused */,
                          QueryCallbackReason reason) {
        if (reason == QueryCallbackReason::Failure) {
          LOG(WARNING) << fmt::format(
              "Failed to kill query in thread {} on {}:{}",
              thread_id,
              host,
              port);
        }
      });
      op->run();
    }
  });
  conn_op->run();
}

MultiQueryStreamOperation::MultiQueryStreamOperation(
    ConnectionProxy&& conn,
    MultiQuery&& multi_query)
    : FetchOperation(std::move(conn), std::move(multi_query)) {}

MultiQueryStreamOperation::MultiQueryStreamOperation(
    ConnectionProxy&& conn,
    std::vector<Query>&& queries)
    : FetchOperation(std::move(conn), std::move(queries)) {}

void MultiQueryStreamOperation::invokeCallback(StreamState reason) {
  // Construct a CallbackVistor object and pass to apply_vistor. It will
  // call the appropriate overaload of 'operator()' depending on the type
  // of callback stored in stream_callback_ i.e. either MultiQueryStreamHandler
  // or MultiQueryStreamOperation::Callback.
  boost::apply_visitor(CallbackVisitor(this, reason), stream_callback_);
}

void MultiQueryStreamOperation::notifyInitQuery() {
  invokeCallback(StreamState::InitQuery);
}

void MultiQueryStreamOperation::notifyRowsReady() {
  invokeCallback(StreamState::RowsReady);
}

void MultiQueryStreamOperation::notifyQuerySuccess(bool) {
  // Query Boundary, only for streaming to allow the user to read from the
  // connection.
  // This will allow pause in the end of the query. End of operations don't
  // allow.
  invokeCallback(StreamState::QueryEnded);
}

void MultiQueryStreamOperation::notifyFailure(OperationResult) {
  // Nop
}

void MultiQueryStreamOperation::notifyOperationCompleted(
    OperationResult result) {
  auto reason =
      (result == OperationResult::Succeeded ? StreamState::Success
                                            : StreamState::Failure);

  invokeCallback(reason);
  stream_callback_ = nullptr;
}

QueryOperation::QueryOperation(ConnectionProxy&& conn, Query&& query)
    : FetchOperation(std::move(conn), std::vector<Query>{std::move(query)}),
      query_result_(std::make_unique<QueryResult>(0)) {}

void QueryOperation::notifyInitQuery() {
  auto* row_stream = rowStream();
  if (row_stream) {
    // Populate RowFields, this is the metadata of rows.
    query_result_->setRowFields(
        row_stream->getEphemeralRowFields()->makeBufferedFields());
  }
}

void QueryOperation::notifyRowsReady() {
  // QueryOperation acts as consumer of FetchOperation, and will buffer the
  // result.
  auto row_block =
      makeRowBlockFromStream(query_result_->getSharedRowFields(), rowStream());

  // Empty result set
  if (row_block.numRows() == 0) {
    return;
  }

  query_result_->appendRowBlock(std::move(row_block));
  if (buffered_query_callback_) {
    buffered_query_callback_(
        *this, query_result_.get(), QueryCallbackReason::RowsFetched);
  }
}

void QueryOperation::notifyQuerySuccess(bool more_results) {
  if (more_results) {
    // Bad usage of QueryOperation, we are going to cancel the query
    cancel_ = true;
    setFetchAction(FetchAction::CompleteOperation);
  }

  query_result_->setOperationResult(OperationResult::Succeeded);
  query_result_->setNumRowsAffected(FetchOperation::currentAffectedRows());
  query_result_->setLastInsertId(FetchOperation::currentLastInsertId());
  query_result_->setRecvGtid(FetchOperation::currentRecvGtid());
  query_result_->setWasSlow(FetchOperation::wasSlow());
  query_result_->setResponseAttributes(FetchOperation::currentRespAttrs());

  query_result_->setPartial(false);

  // We are not going to make callback to user now since this only one query,
  // we make when we finish the operation
}

void QueryOperation::notifyFailure(OperationResult result) {
  // Next call will be to notify user
  query_result_->setOperationResult(result);
}

void QueryOperation::notifyOperationCompleted(OperationResult result) {
  if (!buffered_query_callback_) {
    return;
  }

  // Nothing that changes the non-callback state is safe to be done here.
  auto reason =
      (result == OperationResult::Succeeded ? QueryCallbackReason::Success
                                            : QueryCallbackReason::Failure);
  buffered_query_callback_(*this, query_result_.get(), reason);
  // Release callback since no other callbacks will be made
  buffered_query_callback_ = nullptr;
}

MultiQueryOperation::MultiQueryOperation(
    ConnectionProxy&& conn,
    std::vector<Query>&& queries)
    : FetchOperation(std::move(conn), std::move(queries)),
      current_query_result_(std::make_unique<QueryResult>(0)) {}

void MultiQueryOperation::notifyInitQuery() {
  auto* row_stream = rowStream();
  if (row_stream) {
    // Populate RowFields, this is the metadata of rows.
    current_query_result_->setRowFields(
        row_stream->getEphemeralRowFields()->makeBufferedFields());
  }
}

void MultiQueryOperation::notifyRowsReady() {
  // Create buffered RowBlock
  auto row_block = makeRowBlockFromStream(
      current_query_result_->getSharedRowFields(), rowStream());
  if (row_block.numRows() == 0) {
    return;
  }

  current_query_result_->appendRowBlock(std::move(row_block));
  if (buffered_query_callback_) {
    buffered_query_callback_(
        *this, current_query_result_.get(), QueryCallbackReason::RowsFetched);
  }
}

void MultiQueryOperation::notifyFailure(OperationResult result) {
  // This needs to be called before notifyOperationCompleted, because
  // in non-callback mode we "notify" the conditional variable in `Connection`.
  current_query_result_->setOperationResult(result);
}

void MultiQueryOperation::notifyQuerySuccess(bool) {
  current_query_result_->setPartial(false);

  current_query_result_->setOperationResult(OperationResult::Succeeded);
  current_query_result_->setNumRowsAffected(
      FetchOperation::currentAffectedRows());
  current_query_result_->setLastInsertId(FetchOperation::currentLastInsertId());
  current_query_result_->setRecvGtid(FetchOperation::currentRecvGtid());
  current_query_result_->setResponseAttributes(
      FetchOperation::currentRespAttrs());

  // Notify the callback before moving the result into the operation. This is
  // because the callback can't access the result out of the operation.
  if (buffered_query_callback_) {
    buffered_query_callback_(
        *this, current_query_result_.get(), QueryCallbackReason::QueryBoundary);
  }

  query_results_.emplace_back(std::move(*current_query_result_.get()));
  current_query_result_ =
      std::make_unique<QueryResult>(current_query_result_->queryNum() + 1);
}

void MultiQueryOperation::notifyOperationCompleted(OperationResult result) {
  if (!buffered_query_callback_) { // No callback to be done
    return;
  }
  // Nothing that changes the non-callback state is safe to be done here.
  auto reason =
      (result == OperationResult::Succeeded ? QueryCallbackReason::Success
                                            : QueryCallbackReason::Failure);
  buffered_query_callback_(*this, current_query_result_.get(), reason);
  // Release callback since no other callbacks will be made
  buffered_query_callback_ = nullptr;
}

MultiQueryOperation::~MultiQueryOperation() {}

void SpecialOperation::socketActionable() {
  auto status = callMysqlHandler();
  if (status == MysqlHandler::PENDING) {
    waitForSocketActionable();
  } else {
    auto result = (status == MysqlHandler::DONE)
        ? OperationResult::Succeeded
        : OperationResult::Failed; // MysqlHandler::ERROR
    completeOperation(result);
    if (callback_) {
      callback_(*this, result);
    }
  }
}

void SpecialOperation::mustSucceed() {
  run();
  wait();
  if (!ok()) {
    throw db::RequiredOperationFailedException(getErrorMsg() + mysql_error_);
  }
}

void SpecialOperation::specializedCompleteOperation() {
  conn()->notify();
}

void SpecialOperation::specializedTimeoutTriggered() {
  completeOperation(OperationResult::TimedOut);
}

SpecialOperation* SpecialOperation::specializedRun() {
  MYSQL* mysql = conn()->mysql();
  conn()->socketHandler()->changeHandlerFD(
      folly::NetworkSocket::fromFd(mysql_get_socket_descriptor(mysql)));
  socketActionable();
  return this;
}

MysqlHandler::Status ResetOperation::callMysqlHandler() {
  auto& handler = conn()->client()->getMysqlHandler();
  MYSQL* mysql = conn()->mysql();
  return handler.resetConn(mysql);
}

MysqlHandler::Status ChangeUserOperation::callMysqlHandler() {
  auto& handler = conn()->client()->getMysqlHandler();
  MYSQL* mysql = conn()->mysql();
  return handler.changeUser(mysql, user_, password_, database_);
}

folly::StringPiece Operation::resultString() const {
  return Operation::toString(result());
}

folly::StringPiece Operation::stateString() const {
  return Operation::toString(state());
}

folly::StringPiece Operation::toString(StreamState state) {
  switch (state) {
    case StreamState::InitQuery:
      return "InitQuery";
    case StreamState::RowsReady:
      return "RowsReady";
    case StreamState::QueryEnded:
      return "QueryEnded";
    case StreamState::Failure:
      return "Failure";
    case StreamState::Success:
      return "Success";
  }
  LOG(DFATAL) << "unable to convert state to string: "
              << static_cast<int>(state);
  return "Unknown state";
}

// overload of operator<< for StreamState
std::ostream& operator<<(std::ostream& os, StreamState state) {
  return os << Operation::toString(state);
}

folly::StringPiece Operation::toString(QueryCallbackReason reason) {
  switch (reason) {
    case QueryCallbackReason::RowsFetched:
      return "RowsFetched";
    case QueryCallbackReason::QueryBoundary:
      return "QueryBoundary";
    case QueryCallbackReason::Failure:
      return "Failure";
    case QueryCallbackReason::Success:
      return "Success";
  }
  LOG(DFATAL) << "unable to convert reason to string: "
              << static_cast<int>(reason);
  return "Unknown reason";
}

// overload of operator<< for QueryCallbackReason
std::ostream& operator<<(std::ostream& os, QueryCallbackReason reason) {
  return os << Operation::toString(reason);
}

folly::StringPiece Operation::toString(OperationState state) {
  switch (state) {
    case OperationState::Unstarted:
      return "Unstarted";
    case OperationState::Pending:
      return "Pending";
    case OperationState::Cancelling:
      return "Cancelling";
    case OperationState::Completed:
      return "Completed";
  }
  LOG(DFATAL) << "unable to convert state to string: "
              << static_cast<int>(state);
  return "Unknown state";
}

// overload of operator<< for OperationState
std::ostream& operator<<(std::ostream& os, OperationState state) {
  return os << Operation::toString(state);
}

folly::StringPiece Operation::toString(OperationResult result) {
  switch (result) {
    case OperationResult::Succeeded:
      return "Succeeded";
    case OperationResult::Unknown:
      return "Unknown";
    case OperationResult::Failed:
      return "Failed";
    case OperationResult::Cancelled:
      return "Cancelled";
    case OperationResult::TimedOut:
      return "TimedOut";
  }
  LOG(DFATAL) << "unable to convert result to string: "
              << static_cast<int>(result);
  return "Unknown result";
}

// overload of operator<< for OperationResult
std::ostream& operator<<(std::ostream& os, OperationResult result) {
  return os << Operation::toString(result);
}

folly::StringPiece FetchOperation::toString(FetchAction action) {
  switch (action) {
    case FetchAction::StartQuery:
      return "StartQuery";
    case FetchAction::InitFetch:
      return "InitFetch";
    case FetchAction::Fetch:
      return "Fetch";
    case FetchAction::WaitForConsumer:
      return "WaitForConsumer";
    case FetchAction::CompleteQuery:
      return "CompleteQuery";
    case FetchAction::CompleteOperation:
      return "CompleteOperation";
  }
  LOG(DFATAL) << "unable to convert result to string: "
              << static_cast<int>(action);
  return "Unknown result";
}

// enum connect_stage is defined in mysql at include/mysql_com.h
// and this provides a way to log the string version of this enum
folly::fbstring Operation::connectStageString(connect_stage stage) {
  static const folly::F14FastMap<connect_stage, folly::fbstring>
      stageToStringMap =
  { {connect_stage::CONNECT_STAGE_INVALID, "CONNECT_STAGE_INVALID"},
    {connect_stage::CONNECT_STAGE_NOT_STARTED, "CONNECT_STAGE_NOT_STARTED"},
    {connect_stage::CONNECT_STAGE_NET_BEGIN_CONNECT,
     "CONNECT_STAGE_NET_BEGIN_CONNECT"},
#if MYSQL_VERSION_ID >= 80020 // csm_wait_connect added in 8.0.20
    {connect_stage::CONNECT_STAGE_NET_WAIT_CONNECT,
     "CONNECT_STAGE_NET_WAIT_CONNECT"},
#endif
    {connect_stage::CONNECT_STAGE_NET_COMPLETE_CONNECT,
     "CONNECT_STAGE_NET_COMPLETE_CONNECT"},
    {connect_stage::CONNECT_STAGE_READ_GREETING, "CONNECT_STAGE_READ_GREETING"},
    {connect_stage::CONNECT_STAGE_PARSE_HANDSHAKE,
     "CONNECT_STAGE_PARSE_HANDSHAKE"},
    {connect_stage::CONNECT_STAGE_ESTABLISH_SSL, "CONNECT_STAGE_ESTABLISH_SSL"},
    {connect_stage::CONNECT_STAGE_AUTHENTICATE, "CONNECT_STAGE_AUTHENTICATE"},
    {connect_stage::CONNECT_STAGE_PREP_SELECT_DATABASE,
     "CONNECT_STAGE_PREP_SELECT_DATABASE"},
    {connect_stage::CONNECT_STAGE_PREP_INIT_COMMANDS,
     "CONNECT_STAGE_PREP_INIT_COMMANDS"},
    {connect_stage::CONNECT_STAGE_SEND_ONE_INIT_COMMAND,
     "CONNECT_STAGE_SEND_ONE_INIT_COMMAND"},
    {connect_stage::CONNECT_STAGE_COMPLETE, "CONNECT_STAGE_COMPLETE"},
  };

  try {
    return stageToStringMap.at(stage);
  } catch (const std::out_of_range& /* ex */) {
    return fmt::format("Unexpected connect_stage: {}", (int)stage);
  }
}

std::unique_ptr<Connection> blockingConnectHelper(
    std::shared_ptr<ConnectOperation> conn_op) {
  conn_op->run()->wait();
  if (!conn_op->ok()) {
    throw MysqlException(
        conn_op->result(),
        conn_op->mysql_errno(),
        conn_op->mysql_error(),
        *conn_op->getKey(),
        conn_op->elapsed());
  }

  return std::move(conn_op->releaseConnection());
}

Operation::OwnedConnection::OwnedConnection() {}

Operation::OwnedConnection::OwnedConnection(std::unique_ptr<Connection>&& conn)
    : conn_(std::move(conn)) {}

Connection* Operation::OwnedConnection::get() {
  return conn_.get();
}

std::unique_ptr<Connection>&& Operation::OwnedConnection::releaseConnection() {
  return std::move(conn_);
}

Operation::ConnectionProxy::ConnectionProxy(Operation::OwnedConnection&& conn)
    : ownedConn_(std::move(conn)) {}

Operation::ConnectionProxy::ConnectionProxy(
    Operation::ReferencedConnection&& conn)
    : referencedConn_(std::move(conn)) {}

Connection* Operation::ConnectionProxy::get() {
  return ownedConn_.get() ? ownedConn_.get() : referencedConn_.get();
}

std::unique_ptr<Connection>&& Operation::ConnectionProxy::releaseConnection() {
  if (ownedConn_.get() != nullptr) {
    return ownedConn_.releaseConnection();
  }
  throw std::runtime_error("Releasing connection from referenced conn");
}

std::string Operation::threadOverloadMessage(double cbDelayUs) const {
  return fmt::format(
      "(CLIENT_OVERLOADED: cb delay {}ms, {} active conns)",
      std::lround(cbDelayUs / 1000.0),
      client()->numStartedAndOpenConnections());
}

std::string Operation::timeoutMessage(std::chrono::milliseconds delta) const {
  return fmt::format(
      "(took {}ms, timeout was {}ms)",
      delta.count(),
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout_).count());
}

} // namespace mysql_client
} // namespace common
} // namespace facebook
