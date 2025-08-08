/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/mysql_protocol/MysqlConnectOperationImpl.h"
#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/Flags.h"
#include "squangle/mysql_client/MysqlClientBase.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnection.h"

using namespace std::chrono_literals;

namespace facebook::common::mysql_client::mysql_protocol {

MysqlConnectOperationImpl::MysqlConnectOperationImpl(
    MysqlClientBase* mysql_client,
    std::shared_ptr<const ConnectionKey> conn_key)
    : OperationBase(std::make_unique<MysqlOperationImpl::OwnedConnection>(
          mysql_client->createConnection(conn_key))),
      ConnectOperationImpl(mysql_client, std::move(conn_key)),
      flags_(CLIENT_MULTI_STATEMENTS),
      active_in_client_(true),
      tcp_timeout_handler_(mysql_client->getEventBase(), this) {
  DCHECK(conn_key_); // The connection key is a MySQL connection key
}

MysqlConnectOperationImpl::~MysqlConnectOperationImpl() {
  removeClientReference();
}

bool MysqlConnectOperationImpl::shouldCompleteOperation(
    OperationResult result) {
  // Cancelled doesn't really get to this point, the Operation is forced to
  // complete by Operation, adding this check here just-in-case.
  if (attempts_made_ >= conn_options_.getConnectAttempts() ||
      result == OperationResult::Cancelled) {
    return true;
  }

  return hasOpElapsed(conn_options_.getTotalTimeout() + 1ms);
}

void MysqlConnectOperationImpl::attemptFailed(OperationResult result) {
  ++attempts_made_;
  if (shouldCompleteOperation(result)) {
    completeOperation(result);
    return;
  }

  // We need to update duration_ here because the logging function needs it.
  setDuration();
  logConnectCompleted(result);

  tcp_timeout_handler_.cancelTimeout();

  unregisterHandler();
  cancelTimeout();
  conn().close();

  // Adjust timeout
  Duration timeout_attempt_based = conn_options_.getTimeout() + opElapsed();
  setTimeoutInternal(
      min(timeout_attempt_based, conn_options_.getTotalTimeout()));
  specializedRun();
}

void MysqlConnectOperationImpl::attemptSucceeded(OperationResult result) {
  ++attempts_made_;
  completeOperation(result);
}

void MysqlConnectOperationImpl::specializedRunImpl() {
  if (attempts_made_ == 0) {
    conn().initialize();
  } else {
    conn().initMysqlOnly();
  }
  removeClientReference();

  auto* mysql_conn = getMysqlConnection();
  auto attrs = getAttributes();
  // The MySQL protocol needs any CATs to be sent via connection attributes.
  // Note that the standard MySQL server doesn't know what to do with.  It is
  // intended for use inside Meta's architecture so will be useless elsewhere.
  if (auto cats = getCryptoAuthTokenList()) {
    attrs.emplace("crypto_auth_tokens", *cats);
  }

  mysql_conn->setConnectAttributes(attrs);

  if (const auto& optCompressionLib = getCompression()) {
    mysql_conn->setCompression(*optCompressionLib);
  }

  conn_options_.withPossibleSSLOptionsProvider([&](auto& provider) {
    if (mysql_conn->setSSLOptionsProvider(provider) && connection_context_) {
      connection_context_->isSslConnection = true;
    }
  });

  // Set sni field for ssl connection
  if (const auto& optSniServerName = conn_options_.getSniServerName()) {
    mysql_conn->setSniServerName(*optSniServerName);
  }

  if (const auto& optDscp = conn_options_.getDscp()) {
    if (!mysql_conn->setDscp(*optDscp)) {
      LOG(WARNING) << fmt::format(
          "Failed to set DSCP {} for MySQL Client", *optDscp);
    }
  }

  if (conn_options_.getCertValidationCallback()) {
    mysql_conn->setCertValidatorCallback(mysqlCertValidator, &getOp());
  }

  // If the tcp timeout value is not set in conn options, use the default value
  auto timeout = std::chrono::duration_cast<Millis>(
      conn_options_.getConnectTcpTimeout().value_or(
          Duration(FLAGS_async_mysql_connect_tcp_timeout_micros)));
  // Set the connect timeout in mysql options and also on tcp_timeout_handler if
  // event base is set. Sync implmenation of MysqlClientBase may not have it
  // set. If the timeout is set to 0, skip setting any timeout
  if (timeout.count() != 0) {
    mysql_conn->setConnectTimeout(timeout);
    if (isEventBaseSet()) {
      tcp_timeout_handler_.scheduleTimeout(timeout.count());
    }
  }

  // connect is immediately "ready" to do one loop
  actionable();
}

void MysqlConnectOperationImpl::specializedRun() {
  if (!conn().runInThread([&]() { specializedRunImpl(); })) {
    completeOperationInner(OperationResult::Failed);
  }
}

void MysqlConnectOperationImpl::actionable() {
  DCHECK(isInEventBaseThread());

  folly::stop_watch<Duration> sw;
  auto guard = folly::makeGuard([&]() { logThreadBlockTime(sw); });

  auto* mysql_conn = getMysqlConnection();
  // MYSQL* mysql = conn()->mysql();
  const auto usingUnixSocket = !getMysqlKeyRef().unixSocketPath().empty();

  auto status = mysql_conn->tryConnect(conn_options_, conn_key_, flags_);

  if (status == ERROR) {
    getOp().snapshotMysqlErrors(
        mysql_conn->getErrno(), mysql_conn->getErrorMessage());
    guard.dismiss();
    attemptFailed(OperationResult::Failed);
  } else {
    if ((isDoneWithTcpHandShake() || usingUnixSocket) &&
        tcp_timeout_handler_.isScheduled()) {
      // cancel tcp connect timeout
      tcp_timeout_handler_.cancelTimeout();
    }

    auto fd = mysql_conn->getSocketDescriptor();
    if (fd <= 0) {
      LOG(ERROR) << "Unexpected invalid socket descriptor on completed, "
                 << (status == DONE ? "errorless" : "pending")
                 << " connect.  fd=" << fd;
      getOp().setAsyncClientError(
          static_cast<uint16_t>(SquangleErrno::SQ_INITIALIZATION_FAILED),
          "mysql_get_socket_descriptor returned an invalid descriptor");
      guard.dismiss();
      attemptFailed(OperationResult::Failed);
    } else if (status == DONE) {
      auto socket = folly::NetworkSocket::fromFd(fd);
      changeHandlerFD(socket);
      setConnConnectionContext(connection_context_);
      mysqlConnection()->connectionOpened();
      guard.dismiss();
      attemptSucceeded(OperationResult::Succeeded);
    } else {
      changeHandlerFD(folly::NetworkSocket::fromFd(fd));
      waitForActionable();
    }
  }
}

bool MysqlConnectOperationImpl::isDoneWithTcpHandShake() {
  auto mysql_conn = getMysqlConnection();
  return mysql_conn->isDoneWithTcpHandShake();
}

void MysqlConnectOperationImpl::specializedTimeoutTriggered() {
  timeoutHandler(false);
}

void MysqlConnectOperationImpl::tcpConnectTimeoutTriggered() {
  if (!isDoneWithTcpHandShake()) {
    timeoutHandler(true);
  }
  // else  do nothing since we have made progress
}

void MysqlConnectOperationImpl::timeoutHandler(
    bool isTcpTimeout,
    bool isPoolConnection) {
  std::optional<std::string> location;
  if (!isPoolConnection) {
    location =
        fmt::format("at stage {}", getMysqlConnection()->getConnectStageName());
  }
  auto errorStr = generateTimeoutError(
      opElapsedMs(),
      [](bool stalled) {
        return static_cast<uint16_t>(
            stalled ? SquangleErrno::SQ_ERRNO_CONN_TIMEOUT_LOOP_STALLED
                    : SquangleErrno::SQ_ERRNO_CONN_TIMEOUT);
      },
      isPoolConnection ? "ConnectPool" : "Connect",
      std::move(location),
      fmt::format("(TcpTimeout:{})", (isTcpTimeout ? 1 : 0)));

  getOp().setAsyncClientError(CR_SERVER_LOST, errorStr);
  attemptFailed(OperationResult::TimedOut);
}

void MysqlConnectOperationImpl::logConnectCompleted(OperationResult result) {
  // If the connection wasn't initialized, it's because the operation
  // was cancelled before anything started, so we don't do the logs
  if (!conn().hasInitialized()) {
    return;
  }
  auto* context = connection_context_.get();
  if (result == OperationResult::Succeeded) {
    withOptionalConnectionContext([&](auto& context) {
      context.sslVersion = getMysqlConnection()->getTlsVersion();
    });
    client_.logConnectionSuccess(
        db::CommonLoggingData(
            getOp().getOperationType(),
            elapsed(),
            getTimeout(),
            conn().serverInfo(),
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
    client_.logConnectionFailure(
        db::CommonLoggingData(
            getOp().getOperationType(),
            elapsed(),
            getTimeout(),
            std::nullopt,
            getMaxThreadBlockTime(),
            getTotalThreadBlockTime()),
        reason,
        conn().getKey(),
        mysql_errno(),
        mysql_error(),
        context);
  }
}

void MysqlConnectOperationImpl::maybeStoreSSLSession() {
  // If connection was successful
  if (result() != OperationResult::Succeeded || !conn().hasInitialized()) {
    return;
  }

  // if there is an ssl provider set
  conn_options_.withPossibleSSLOptionsProvider([&](auto& provider) {
    if (getMysqlConnection()->storeSession(provider)) {
      if (connection_context_) {
        connection_context_->sslSessionReused = true;
      }
      client_.stats()->incrReusedSSLSessions();
    }
  });
}

void MysqlConnectOperationImpl::specializedCompleteOperation() {
  // Pass the callbacks to the Connection now that we are done with them
  conn().setCallbacks(std::move(callbacks_));

  // Operations that don't directly initiate a new TLS conneciton
  // shouldn't update the TLS session because it can propagate the
  // session object from a connection created usisn one client cert
  // to an SSL provider initialized with a different cert.
  if (getOp().getOperationType() == db::OperationType::Connect) {
    maybeStoreSSLSession();
  }

  // Can only log this on successful connections because unsuccessful
  // ones call mysql_close_free inside libmysql
  if (result() == OperationResult::Succeeded && conn().ok() &&
      connection_context_) {
    connection_context_->endpointVersion = conn().serverInfo();
  }

  // Cancel tcp timeout
  tcp_timeout_handler_.cancelTimeout();

  logConnectCompleted(result());

  // If connection_initialized_ is false the only way to complete the
  // operation is by cancellation
  DCHECK(conn().hasInitialized() || result() == OperationResult::Cancelled);

  conn().setConnectionOptions(conn_options_);
  conn().setKillOnQueryTimeout(getKillOnQueryTimeout());
  setConnConnectionContext(connection_context_);

  conn().notify();

  op().callConnectCallback();

  // In case this operation didn't even get the chance to run, we still need
  // to remove the reference it added to the async client
  removeClientReference();
}

void MysqlConnectOperationImpl::removeClientReference() {
  if (active_in_client_) {
    // It's safe to call the client since we still have a ref counting
    // it won't die before it goes to 0
    active_in_client_ = false;
    client_.activeConnectionRemoved(conn_key_);
  }
}

int MysqlConnectOperationImpl::mysqlCertValidator(
    X509* server_cert,
    const void* context,
    const char** errptr) {
  ConnectOperation* self =
      reinterpret_cast<ConnectOperation*>(const_cast<void*>(context));
  CHECK_NOTNULL(self);

  // Hold a shared pointer to the Operation object while running the callback
  auto weak_self = self->weak_from_this();
  auto guard = weak_self.lock();
  if (!guard) {
    LOG(ERROR) << "ConnectOperation object " << self
               << " is already deallocated";
    return 0;
  }

  const CertValidatorCallback callback =
      self->getConnectionOptions().getCertValidationCallback();
  CHECK(callback);
  folly::StringPiece errorMessage;

  // "libmysql" expects this callback to return "0" if the cert validation was
  // successful, and return "1" if validation failed.
  int result =
      callback(server_cert, std::move(weak_self), errorMessage) ? 0 : 1;
  if (!errorMessage.empty()) {
    *errptr = errorMessage.data();
  }
  return result;
}

} // namespace facebook::common::mysql_client::mysql_protocol
