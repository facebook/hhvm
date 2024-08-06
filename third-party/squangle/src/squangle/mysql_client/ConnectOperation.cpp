/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <chrono>

#include "squangle/mysql_client/ConnectOperation.h"
#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/Flags.h"
#include "squangle/mysql_client/MysqlHandler.h"

namespace facebook::common::mysql_client {

using namespace std::chrono_literals;

ConnectOperation::ConnectOperation(
    MysqlClientBase* mysql_client,
    ConnectionKey conn_key)
    : Operation(std::make_unique<Operation::OwnedConnection>(
          mysql_client->createConnection(conn_key, nullptr))),
      conn_key_(std::move(conn_key)),
      flags_(CLIENT_MULTI_STATEMENTS),
      active_in_client_(true),
      tcp_timeout_handler_(mysql_client->getEventBase(), this) {
  mysql_client->activeConnectionAdded(&conn_key_);
}

ConnectOperation& ConnectOperation::setConnectionOptions(
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
  return *this;
}

const ConnectionOptions& ConnectOperation::getConnectionOptions() const {
  return conn_options_;
}

ConnectOperation& ConnectOperation::setDefaultQueryTimeout(Duration t) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setQueryTimeout(t);
  return *this;
}

ConnectOperation& ConnectOperation::setSniServerName(
    const std::string& sni_servername) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setSniServerName(sni_servername);
  return *this;
}

ConnectOperation& ConnectOperation::enableResetConnBeforeClose() {
  conn_options_.enableResetConnBeforeClose();
  return *this;
}

ConnectOperation& ConnectOperation::enableDelayedResetConn() {
  conn_options_.enableDelayedResetConn();
  return *this;
}

ConnectOperation& ConnectOperation::enableChangeUser() {
  conn_options_.enableChangeUser();
  return *this;
}

ConnectOperation& ConnectOperation::setCertValidationCallback(
    CertValidatorCallback callback,
    const void* context,
    bool opPtrAsContext) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setCertValidationCallback(
      std::move(callback), context, opPtrAsContext);
  return *this;
}

ConnectOperation& ConnectOperation::setTimeout(Duration timeout) {
  conn_options_.setTimeout(timeout);
  Operation::setTimeout(timeout);
  return *this;
}

ConnectOperation& ConnectOperation::setTcpTimeout(Duration timeout) {
  conn_options_.setConnectTcpTimeout(timeout);
  return *this;
}

ConnectOperation& ConnectOperation::setTotalTimeout(Duration total_timeout) {
  conn_options_.setTotalTimeout(total_timeout);
  Operation::setTimeout(min(timeout_, total_timeout));
  return *this;
}
ConnectOperation& ConnectOperation::setConnectAttempts(uint32_t max_attempts) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setConnectAttempts(max_attempts);
  return *this;
}

ConnectOperation& ConnectOperation::setDscp(uint8_t dscp) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setDscp(dscp);
  return *this;
}

ConnectOperation& ConnectOperation::setKillOnQueryTimeout(
    bool killOnQueryTimeout) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  killOnQueryTimeout_ = killOnQueryTimeout;
  return *this;
}
ConnectOperation& ConnectOperation::setSSLOptionsProviderBase(
    std::unique_ptr<SSLOptionsProviderBase> /*ssl_options_provider*/) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  LOG(ERROR) << "Using deprecated function";
  return *this;
}
ConnectOperation& ConnectOperation::setSSLOptionsProvider(
    std::shared_ptr<SSLOptionsProviderBase> ssl_options_provider) {
  CHECK_THROW(
      state() == OperationState::Unstarted, db::OperationStateException);
  conn_options_.setSSLOptionsProvider(ssl_options_provider);
  return *this;
}

bool ConnectOperation::shouldCompleteOperation(OperationResult result) {
  // Cancelled doesn't really get to this point, the Operation is forced to
  // complete by Operation, adding this check here just-in-case.
  if (attempts_made_ >= conn_options_.getConnectAttempts() ||
      result == OperationResult::Cancelled) {
    return true;
  }

  return stopwatch_->elapsed(conn_options_.getTotalTimeout() + 1ms);
}

void ConnectOperation::attemptFailed(OperationResult result) {
  ++attempts_made_;
  if (shouldCompleteOperation(result)) {
    completeOperation(result);
    return;
  }

  // We need to update opDuration_ here because the logging function needs it.
  opDuration_ = stopwatch_->elapsed();
  logConnectCompleted(result);

  tcp_timeout_handler_.cancelTimeout();

  unregisterHandler();
  cancelTimeout();
  conn().close();

  // Adjust timeout
  Duration timeout_attempt_based =
      conn_options_.getTimeout() + stopwatch_->elapsed();
  timeout_ = min(timeout_attempt_based, conn_options_.getTotalTimeout());
  specializedRun();
}

void ConnectOperation::attemptSucceeded(OperationResult result) {
  ++attempts_made_;
  completeOperation(result);
}

void ConnectOperation::specializedRunImpl() {
  if (attempts_made_ == 0) {
    conn().initialize();
  } else {
    conn().initMysqlOnly();
  }
  removeClientReference();

  conn().setConnectAttributes(attributes_);

  if (const auto& optCompressionLib = getCompression()) {
    conn().setCompression(*optCompressionLib);
  }

  conn_options_.withPossibleSSLOptionsProvider([&](auto& provider) {
    if (conn().setSSLOptionsProvider(provider) && connection_context_) {
      connection_context_->isSslConnection = true;
    }
  });

  // Set sni field for ssl connection
  if (const auto& optSniServerName = conn_options_.getSniServerName()) {
    conn().setSniServerName(*optSniServerName);
  }

  if (const auto& optDscp = conn_options_.getDscp()) {
    if (!conn().setDscp(*optDscp)) {
      LOG(WARNING) << fmt::format(
          "Failed to set DSCP {} for MySQL Client", *optDscp);
    }
  }

  if (conn_options_.getCertValidationCallback()) {
    conn().setCertValidatorCallback(mysqlCertValidator, this);
  }

  // If the tcp timeout value is not set in conn options, use the default value
  auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(
      conn_options_.getConnectTcpTimeout().value_or(
          Duration(FLAGS_async_mysql_connect_tcp_timeout_micros)));
  // Set the connect timeout in mysql options and also on tcp_timeout_handler if
  // event base is set. Sync implmenation of MysqlClientBase may not have it
  // set. If the timeout is set to 0, skip setting any timeout
  if (timeout.count() != 0) {
    conn().setConnectTimeout(timeout);
    if (isEventBaseSet()) {
      tcp_timeout_handler_.scheduleTimeout(timeout.count());
    }
  }

  // connect is immediately "ready" to do one loop
  actionable();
}

ConnectOperation& ConnectOperation::specializedRun() {
  if (!connection().runInThread([&]() { specializedRunImpl(); })) {
    completeOperationInner(OperationResult::Failed);
  }
  return *this;
}

ConnectOperation::~ConnectOperation() {
  removeClientReference();
}

void ConnectOperation::actionable() {
  DCHECK(isInEventBaseThread());

  folly::stop_watch<Duration> sw;
  auto guard = folly::makeGuard([&]() { logThreadBlockTime(sw); });

  auto& handler = conn().client().getMysqlHandler();
  // MYSQL* mysql = conn()->mysql();
  const auto usingUnixSocket = !conn_key_.unixSocketPath().empty();

  auto status = handler.tryConnect(
      conn().getInternalConnection(), conn_options_, conn_key_, flags_);

  if (status == ERROR) {
    snapshotMysqlErrors();
    guard.dismiss();
    attemptFailed(OperationResult::Failed);
  } else {
    if ((isDoneWithTcpHandShake() || usingUnixSocket) &&
        tcp_timeout_handler_.isScheduled()) {
      // cancel tcp connect timeout
      tcp_timeout_handler_.cancelTimeout();
    }

    auto fd = conn().getSocketDescriptor();
    if (fd <= 0) {
      LOG(ERROR) << "Unexpected invalid socket descriptor on completed, "
                 << (status == DONE ? "errorless" : "pending")
                 << " connect.  fd=" << fd;
      setAsyncClientError(
          static_cast<uint16_t>(SquangleErrno::SQ_INITIALIZATION_FAILED),
          "mysql_get_socket_descriptor returned an invalid descriptor");
      guard.dismiss();
      attemptFailed(OperationResult::Failed);
    } else if (status == DONE) {
      auto socket = folly::NetworkSocket::fromFd(fd);
      changeHandlerFD(socket);
      conn().mysqlConnection()->setConnectionContext(connection_context_);
      conn().mysqlConnection()->connectionOpened();
      guard.dismiss();
      attemptSucceeded(OperationResult::Succeeded);
    } else {
      changeHandlerFD(folly::NetworkSocket::fromFd(fd));
      waitForActionable();
    }
  }
}

bool ConnectOperation::isDoneWithTcpHandShake() {
  return conn().isDoneWithTcpHandShake();
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
  auto deltaUs = stopwatch_->elapsed();
  auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(deltaUs);

  auto cbDelayUs = client().callbackDelayMicrosAvg();
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
    parts.push_back(fmt::format("at stage {}", conn().getConnectStageName()));
  }

  parts.push_back(timeoutMessage(deltaMs));
  if (stalled) {
    parts.push_back(threadOverloadMessage(cbDelayUs));
  }
  parts.push_back(fmt::format("(TcpTimeout:{})", (isTcpTimeout ? 1 : 0)));

  setAsyncClientError(CR_SERVER_LOST, folly::join(" ", parts));
  attemptFailed(OperationResult::TimedOut);
}

void ConnectOperation::logConnectCompleted(OperationResult result) {
  // If the connection wasn't initialized, it's because the operation
  // was cancelled before anything started, so we don't do the logs
  if (!conn().hasInitialized()) {
    return;
  }
  auto* context = connection_context_.get();
  if (result == OperationResult::Succeeded) {
    if (context) {
      context->sslVersion = conn().getTlsVersion();
    }
    client().logConnectionSuccess(
        db::CommonLoggingData(
            getOperationType(),
            opDuration_,
            timeout_,
            getMaxThreadBlockTime(),
            getTotalThreadBlockTime()),
        conn().getKey(),
        context);
  } else {
    db::FailureReason reason = db::FailureReason::DATABASE_ERROR;
    if (result == OperationResult::TimedOut) {
      reason = db::FailureReason::TIMEOUT;
    } else if (result == OperationResult::Cancelled) {
      reason = db::FailureReason::CANCELLED;
    }
    client().logConnectionFailure(
        db::CommonLoggingData(
            getOperationType(),
            opDuration_,
            timeout_,
            getMaxThreadBlockTime(),
            getTotalThreadBlockTime()),
        reason,
        conn().getKey(),
        mysql_errno(),
        mysql_error(),
        context);
  }
}

void ConnectOperation::maybeStoreSSLSession() {
  // If connection was successful
  if (result_ != OperationResult::Succeeded || !conn().hasInitialized()) {
    return;
  }

  // if there is an ssl provider set
  conn_options_.withPossibleSSLOptionsProvider([&](auto& provider) {
    if (conn().storeSession(provider)) {
      if (connection_context_) {
        connection_context_->sslSessionReused = true;
      }
      client().stats()->incrReusedSSLSessions();
    }
  });
}

void ConnectOperation::specializedCompleteOperation() {
  // Pass the callbacks to the Connection now that we are done with them
  conn().setCallbacks(std::move(callbacks_));

  // Operations that don't directly initiate a new TLS conneciton
  // shouldn't update the TLS session because it can propagate the
  // session object from a connection created usisn one client cert
  // to an SSL provider initialized with a different cert.
  if (getOperationType() == db::OperationType::Connect) {
    maybeStoreSSLSession();
  }

  // Can only log this on successful connections because unsuccessful
  // ones call mysql_close_free inside libmysql
  if (result_ == OperationResult::Succeeded && conn().ok() &&
      connection_context_) {
    connection_context_->endpointVersion = conn().serverInfo();
  }

  // Cancel tcp timeout
  tcp_timeout_handler_.cancelTimeout();

  logConnectCompleted(result_);

  // If connection_initialized_ is false the only way to complete the
  // operation is by cancellation
  DCHECK(conn().hasInitialized() || result_ == OperationResult::Cancelled);

  conn().setConnectionOptions(conn_options_);
  conn().setKillOnQueryTimeout(getKillOnQueryTimeout());
  conn().setConnectionContext(connection_context_);

  conn().notify();

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
  run().wait();
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
    client().activeConnectionRemoved(&conn_key_);
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
  auto guard = weak_self.lock();
  if (guard == nullptr) {
    LOG(ERROR) << "ConnectOperation object " << self
               << " is already deallocated";
    return 0;
  }

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

} // namespace facebook::common::mysql_client
