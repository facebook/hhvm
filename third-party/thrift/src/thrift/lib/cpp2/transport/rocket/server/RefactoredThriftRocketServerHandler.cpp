/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/transport/rocket/server/RefactoredThriftRocketServerHandler.h>

#include <memory>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/stop_watch.h>

#include <fmt/core.h>
#include <folly/ExceptionString.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/LoggingEventHelper.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketSinkClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketSetupProcessor.h>

namespace {
const int64_t kRocketServerMaxVersion = 10;

#if FOLLY_HAS_COROUTINES
template <typename ServiceInterceptorsType>
std::string formatServiceInterceptorExceptions(
    std::string_view methodName,
    const std::vector<std::pair<std::size_t, std::exception_ptr>>& exceptions,
    const ServiceInterceptorsType& serviceInterceptors) {
  std::string message = fmt::format(
      "ServiceInterceptor::{} threw exceptions:\n[{}] {}\n",
      methodName,
      serviceInterceptors[exceptions[0].first]->getQualifiedName().get(),
      folly::exceptionStr(exceptions[0].second));
  for (std::size_t i = 1; i < exceptions.size(); ++i) {
    message += fmt::format(
        "[{}] {}\n",
        serviceInterceptors[exceptions[i].first]->getQualifiedName().get(),
        folly::exceptionStr(exceptions[i].second));
  }
  return message;
}
#endif // FOLLY_HAS_COROUTINES
} // namespace

THRIFT_FLAG_DECLARE_int64(rocket_server_max_version);

namespace apache::thrift::rocket {

RefactoredThriftRocketServerHandler::RefactoredThriftRocketServerHandler(
    std::shared_ptr<Cpp2Worker> worker,
    const folly::SocketAddress& clientAddress,
    folly::AsyncTransport* transport,
    const std::vector<std::unique_ptr<SetupFrameHandler>>& handlers,
    const std::vector<std::unique_ptr<SetupFrameInterceptor>>& interceptors)
    : worker_(std::move(worker)),
      connectionGuard_(worker_->getActiveRequestsGuard()),
      transport_(transport),
      connContext_(
          &clientAddress,
          transport,
          nullptr, /* eventBaseManager */
          nullptr, /* x509PeerCert */
          worker_->getServer()->getClientIdentityHook(),
          worker_.get(),
          worker_->getServer()->getServiceInterceptors().size()),
      setupFrameHandlers_(handlers),
      setupFrameInterceptors_(interceptors),
      version_(
          static_cast<int32_t>(std::min(
              kRocketServerMaxVersion,
              THRIFT_FLAG(rocket_server_max_version)))),
      maxResponseWriteTime_(worker_->getServer()
                                ->getThriftServerConfig()
                                .getMaxResponseWriteTime()
                                .get()),
      resetConnCtxUserDataOnClose_(worker_->getServer()
                                       ->getThriftServerConfig()
                                       .getResetConnCtxUserDataOnClose()) {
  connContext_.setTransportType(Cpp2ConnContext::TransportType::ROCKET);

  errorHandler_ = std::make_unique<RocketErrorHandler>(
      worker_->getServer()->getObserver(), worker_->getRequestsRegistry());

  for (const auto& handler : worker_->getServer()->getEventHandlersUnsafe()) {
    handler->newConnection(&connContext_);
  }
}

RefactoredThriftRocketServerHandler::~RefactoredThriftRocketServerHandler() {
  invokeServiceInterceptorsOnConnectionClosed();

  for (const auto& handler : worker_->getServer()->getEventHandlersUnsafe()) {
    handler->connectionDestroyed(&connContext_);
  }
  // Ensure each connAccepted() call has a matching connClosed()
  if (auto* observer = worker_->getServer()->getObserver()) {
    observer->connClosed(
        server::TServerObserver::ConnectionInfo(
            reinterpret_cast<uint64_t>(transport_),
            connContext_.getSecurityProtocol()));
  }
}

void RefactoredThriftRocketServerHandler::handleSetupFrame(
    SetupFrame&& frame, IRocketServerConnection& icontext) {
  auto& connection = static_cast<RocketServerConnection&>(icontext);

  // Delegate setup processing - RocketSetupProcessor creates and returns
  // RocketRequestHandler directly
  RocketSetupProcessor setupProcessor(
      *worker_,
      connContext_,
      setupFrameHandlers_,
      setupFrameInterceptors_,
      version_,
      transport_,
      maxResponseWriteTime_,
      &setupLoggingFlag_,
      errorHandler_.get());

  bool isCustomHandler = false;
  requestHandler_ = setupProcessor.handleSetupFrame(
      std::move(frame),
      connection,
      [this, &connection]() {
        invokeServiceInterceptorsOnConnectionAttempted(connection);
      },
      &isCustomHandler);

  if (!requestHandler_) {
    // Setup failed, connection will be closed by setupProcessor
    return;
  }

  invokeServiceInterceptorsOnConnectionEstablished(connection);

  // For custom handlers, coalesce happens AFTER ConnectionEstablished
  if (isCustomHandler) {
    requestHandler_->coalesceProcessorWithLegacyEventHandlers();
  }
}

void RefactoredThriftRocketServerHandler::handleRequestResponseFrame(
    RequestResponseFrame&& frame, RocketServerFrameContext&& context) {
  if (!requestHandler_) {
    return; // Setup not completed
  }
  requestHandler_->handleRequestResponse(std::move(frame), std::move(context));
}

void RefactoredThriftRocketServerHandler::handleRequestFnfFrame(
    RequestFnfFrame&& frame, RocketServerFrameContext&& context) {
  if (!requestHandler_) {
    return;
  }
  requestHandler_->handleRequestFnf(std::move(frame), std::move(context));
}

void RefactoredThriftRocketServerHandler::handleRequestStreamFrame(
    RequestStreamFrame&& frame,
    RocketServerFrameContext&& context,
    RocketStreamClientCallback* clientCallback) {
  if (!requestHandler_) {
    return;
  }
  requestHandler_->handleRequestStream(
      std::move(frame), std::move(context), clientCallback);
}

void RefactoredThriftRocketServerHandler::handleRequestChannelFrame(
    RequestChannelFrame&& frame,
    RocketServerFrameContext&& context,
    ChannelRequestCallbackFactory clientCallback) {
  if (!requestHandler_) {
    return;
  }
  requestHandler_->handleRequestChannel(
      std::move(frame), std::move(context), clientCallback);
}

void RefactoredThriftRocketServerHandler::connectionClosing() {
  connContext_.connectionClosed();
  if (resetConnCtxUserDataOnClose_) {
    // Despite the method name, this only resets TransportInfo and userdata, not
    // the entire Cpp2ConnContext!
    connContext_.reset();
  }
  if (requestHandler_) {
    requestHandler_->destroyAllInteractions();
  }
}

void RefactoredThriftRocketServerHandler::requestComplete() {
  if (requestHandler_) {
    requestHandler_->requestComplete();
  }
}

void RefactoredThriftRocketServerHandler::terminateInteraction(int64_t id) {
  if (requestHandler_) {
    requestHandler_->terminateInteraction(id);
  }
}

void RefactoredThriftRocketServerHandler::onBeforeHandleFrame() {
  worker_->getServer()->touchRequestTimestamp();
}

void RefactoredThriftRocketServerHandler::
    invokeServiceInterceptorsOnConnectionAttempted(
        IRocketServerConnection& connection) noexcept {
#if FOLLY_HAS_COROUTINES
  auto* server = worker_->getServer();
  const auto& serviceInterceptors = server->getServiceInterceptors();
  std::vector<std::pair<std::size_t, std::exception_ptr>> exceptions;

  folly::stop_watch<std::chrono::microseconds> totalTimer;
  for (std::size_t i = 0; i < serviceInterceptors.size(); ++i) {
    ServiceInterceptorBase::ConnectionInfo connectionInfo{
        &connContext_,
        connContext_.getStorageForServiceInterceptorOnConnectionByIndex(i)};
    try {
      serviceInterceptors[i]->internal_onConnectionAttempted(
          std::move(connectionInfo), server->getInterceptorMetricCallback());
    } catch (...) {
      exceptions.emplace_back(i, folly::current_exception());
    }
  }
  server->getInterceptorMetricCallback().onConnectionAttemptedTotalComplete(
      totalTimer.elapsed());
  if (!exceptions.empty()) {
    auto message = formatServiceInterceptorExceptions(
        "onConnectionAttempted", exceptions, serviceInterceptors);
    return connection.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::REJECTED_SETUP, std::move(message)));
  }
#endif // FOLLY_HAS_COROUTINES
}

void RefactoredThriftRocketServerHandler::
    invokeServiceInterceptorsOnConnectionEstablished(
        IRocketServerConnection& connection) noexcept {
#if FOLLY_HAS_COROUTINES
  auto* server = worker_->getServer();
  const auto& serviceInterceptors = server->getServiceInterceptors();
  std::vector<std::pair<std::size_t, std::exception_ptr>> exceptions;
  didExecuteServiceInterceptorsOnConnection_ = true;

  folly::stop_watch<std::chrono::microseconds> totalTimer;
  for (std::size_t i = 0; i < serviceInterceptors.size(); ++i) {
    ServiceInterceptorBase::ConnectionInfo connectionInfo{
        &connContext_,
        connContext_.getStorageForServiceInterceptorOnConnectionByIndex(i)};
    try {
      serviceInterceptors[i]->internal_onConnectionEstablished(
          std::move(connectionInfo), server->getInterceptorMetricCallback());
    } catch (...) {
      exceptions.emplace_back(i, folly::current_exception());
    }
  }
  server->getInterceptorMetricCallback().onConnectionTotalComplete(
      totalTimer.elapsed());
  if (!exceptions.empty()) {
    auto message = formatServiceInterceptorExceptions(
        "onConnectionEstablished", exceptions, serviceInterceptors);
    return connection.close(
        folly::make_exception_wrapper<RocketException>(
            ErrorCode::REJECTED_SETUP, std::move(message)));
  }
#endif // FOLLY_HAS_COROUTINES
}

void RefactoredThriftRocketServerHandler::
    invokeServiceInterceptorsOnConnectionClosed() noexcept {
#if FOLLY_HAS_COROUTINES
  if (!didExecuteServiceInterceptorsOnConnection_) {
    return;
  }

  auto* server = worker_->getServer();
  const auto& serviceInterceptors = server->getServiceInterceptors();
  folly::stop_watch<std::chrono::microseconds> totalTimer;
  for (std::size_t i = 0; i < serviceInterceptors.size(); ++i) {
    ServiceInterceptorBase::ConnectionInfo connectionInfo{
        &connContext_,
        connContext_.getStorageForServiceInterceptorOnConnectionByIndex(i)};

    serviceInterceptors[i]->internal_onConnectionClosed(
        connectionInfo, server->getInterceptorMetricCallback());
  }
  server->getInterceptorMetricCallback().onConnectionClosedTotalComplete(
      totalTimer.elapsed());
#endif // FOLLY_HAS_COROUTINES
}

} // namespace apache::thrift::rocket
