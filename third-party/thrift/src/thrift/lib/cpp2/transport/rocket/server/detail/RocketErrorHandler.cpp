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

#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketErrorHandler.h>

#include <fmt/format.h>
#include <folly/ExceptionWrapper.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>
#include <thrift/lib/cpp2/transport/rocket/server/InteractionOverload.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketThriftRequests.h>

namespace apache::thrift::rocket {

RocketErrorHandler::RocketErrorHandler(
    server::TServerObserver* observer, RequestsRegistry* requestsRegistry)
    : observer_(observer), requestsRegistry_(requestsRegistry) {}

void RocketErrorHandler::handleRequestWithBadMetadata(
    ThriftRequestCoreUniquePtr request) {
  sendErrorAndNotifyTaskKilled(
      std::move(request),
      TApplicationException::UNSUPPORTED_CLIENT_TYPE,
      "Invalid metadata object",
      kRequestParsingErrorCode);
}

void RocketErrorHandler::handleRequestWithBadChecksum(
    ThriftRequestCoreUniquePtr request) {
  sendErrorAndNotifyTaskKilled(
      std::move(request),
      TApplicationException::CHECKSUM_MISMATCH,
      "Checksum mismatch",
      kChecksumMismatchErrorCode);
}

void RocketErrorHandler::handleDecompressionFailure(
    ThriftRequestCoreUniquePtr request, std::string&& reason) {
  sendErrorAndNotifyTaskKilled(
      std::move(request),
      TApplicationException::INVALID_TRANSFORM,
      fmt::format("decompression failure: {}", std::move(reason)),
      kRequestParsingErrorCode);
}

void RocketErrorHandler::handleServerNotReady(
    ThriftRequestCoreUniquePtr request) {
  sendErrorAndNotifyTaskKilled(
      std::move(request),
      TApplicationException::LOADSHEDDING,
      "server not ready",
      kQueueOverloadedErrorCode);
}

void RocketErrorHandler::handleServerShutdown(
    ThriftRequestCoreUniquePtr request) {
  sendErrorAndNotifyTaskKilled(
      std::move(request),
      TApplicationException::LOADSHEDDING,
      "server shutting down",
      kQueueOverloadedErrorCode);
}

void RocketErrorHandler::handleRequestOverloadedServer(
    ThriftRequestCoreUniquePtr request, OverloadResult&& overloadResult) {
  if (observer_) {
    observer_->serverOverloaded(overloadResult.loadShedder);
  }

  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::LOADSHEDDING,
          std::move(overloadResult.errorMessage)),
      std::move(overloadResult.errorCode));
  if (request->includeInRecentRequests()) {
    requestsRegistry_->getRequestCounter().incrementOverloadCount();
  }
}

void RocketErrorHandler::handleQuotaExceededException(
    ThriftRequestCoreUniquePtr request,
    const std::string& errorCode,
    const std::string& errorMessage) {
  if (observer_) {
    observer_->taskKilled();
    observer_->quotaExceeded();
  }

  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::TENANT_QUOTA_EXCEEDED, errorMessage),
      errorCode);
}

void RocketErrorHandler::handleAppError(
    ThriftRequestCoreUniquePtr request,
    const PreprocessResult& appErrorResult) {
  notifyTaskKilled();

  folly::variant_match(
      appErrorResult,
      [&](const AppClientException& ace) {
        request->sendErrorWrapped(ace, kAppClientErrorCode);
      },
      [&](const AppServerException& ase) {
        request->sendErrorWrapped(ase, kAppServerErrorCode);
      },
      [&](const auto&) { folly::assume_unreachable(); });
}

void RocketErrorHandler::handleRequestWithFdsExtractionFailure(
    ThriftRequestCoreUniquePtr request, std::string&& errorMessage) {
  sendErrorAndNotifyTaskKilled(
      std::move(request),
      TApplicationException::UNKNOWN,
      std::move(errorMessage),
      kRequestParsingErrorCode);
}

void RocketErrorHandler::handleInteractionLoadshedded(
    ThriftRequestCoreUniquePtr request) {
  sendErrorAndNotifyTaskKilled(
      std::move(request),
      TApplicationException::LOADSHEDDING,
      "Interaction already loadshedded",
      kInteractionLoadsheddedErrorCode);
}

void RocketErrorHandler::handleInjectedFault(
    ThriftRequestCoreUniquePtr request, InjectedFault fault) {
  switch (fault) {
    case InjectedFault::ERROR:
      sendErrorAndNotifyTaskKilled(
          std::move(request),
          TApplicationException::INJECTED_FAILURE,
          "injected failure",
          kInjectedFailureErrorCode);
      return;
    case InjectedFault::DROP:
      VLOG(1)
          << "ERROR: injected drop: "
          << request->getRequestContext()->getPeerAddress()->getAddressStr();
      return;
    case InjectedFault::DISCONNECT:
      return request->closeConnection(
          folly::make_exception_wrapper<TApplicationException>(
              TApplicationException::INJECTED_FAILURE, "injected failure"));
    default:
      folly::assume_unreachable();
  }
}

void RocketErrorHandler::handlePreprocessError(
    ThriftRequestCoreUniquePtr request,
    PreprocessResult&& preprocessResult,
    bool isInteractionCreatePresent,
    std::optional<int64_t>& interactionIdOpt) {
  folly::variant_match(
      preprocessResult,
      [&](AppClientException& ace) { handleAppError(std::move(request), ace); },
      [&](const AppServerException& ase) {
        handleAppError(std::move(request), ase);
      },
      [&](AppOverloadedException& aoe) {
        handleAppOverloadedException(
            std::move(request),
            aoe,
            isInteractionCreatePresent,
            interactionIdOpt);
      },
      [&](AppQuotaExceededException& aqe) {
        handleQuotaExceededException(
            std::move(request),
            kTenantQuotaExceededErrorCode,
            aqe.getMessage());
      },
      [](std::monostate&) { folly::assume_unreachable(); });
}

void RocketErrorHandler::handleAppOverloadedException(
    ThriftRequestCoreUniquePtr request,
    AppOverloadedException& aoe,
    bool isInteractionCreatePresent,
    std::optional<int64_t>& interactionIdOpt) {
  std::string_view exCode = kAppOverloadedErrorCode;
  if ((isInteractionCreatePresent || interactionIdOpt) &&
      shouldTerminateInteraction(
          isInteractionCreatePresent,
          interactionIdOpt,
          request->getRequestContext()->getConnectionContext())) {
    exCode = kInteractionLoadsheddedAppOverloadErrorCode;
  }

  handleRequestOverloadedServer(
      std::move(request),
      OverloadResult{
          std::string(exCode), aoe.getMessage(), LoadShedder::CUSTOM});
}

void RocketErrorHandler::notifyTaskKilled() {
  if (observer_) {
    observer_->taskKilled();
  }
}

void RocketErrorHandler::sendErrorAndNotifyTaskKilled(
    ThriftRequestCoreUniquePtr request,
    TApplicationException::TApplicationExceptionType exceptionType,
    std::string message,
    const std::string& errorCode) {
  notifyTaskKilled();
  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          exceptionType, std::move(message)),
      errorCode);
}

} // namespace apache::thrift::rocket
