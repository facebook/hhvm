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

#pragma once

#include <optional>
#include <string>

#include <thrift/lib/cpp2/server/PreprocessResult.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>

namespace apache::thrift {

struct OverloadResult;

class ThriftRequestCore;
using ThriftRequestCoreUniquePtr =
    std::unique_ptr<ThriftRequestCore, RequestsRegistry::Deleter>;

namespace server {
class TServerObserver;
} // namespace server

namespace rocket {

/**
 * Handles all error scenarios for Rocket requests.
 */
class RocketErrorHandler {
 public:
  explicit RocketErrorHandler(
      server::TServerObserver* observer, RequestsRegistry* requestsRegistry);

  // Basic error handlers
  void handleRequestWithBadMetadata(ThriftRequestCoreUniquePtr request);
  void handleRequestWithBadChecksum(ThriftRequestCoreUniquePtr request);
  void handleDecompressionFailure(
      ThriftRequestCoreUniquePtr request, std::string&& reason);
  void handleServerNotReady(ThriftRequestCoreUniquePtr request);
  void handleServerShutdown(ThriftRequestCoreUniquePtr request);

  // Extended error handlers
  void handleRequestOverloadedServer(
      ThriftRequestCoreUniquePtr request, OverloadResult&& overloadResult);
  void handleQuotaExceededException(
      ThriftRequestCoreUniquePtr request,
      const std::string& errorCode,
      const std::string& errorMessage);
  void handleAppError(
      ThriftRequestCoreUniquePtr request,
      const PreprocessResult& appErrorResult);
  void handleRequestWithFdsExtractionFailure(
      ThriftRequestCoreUniquePtr request, std::string&& errorMessage);
  void handleInteractionLoadshedded(ThriftRequestCoreUniquePtr request);

  enum class InjectedFault { ERROR, DROP, DISCONNECT };
  void handleInjectedFault(
      ThriftRequestCoreUniquePtr request, InjectedFault fault);

  void handlePreprocessError(
      ThriftRequestCoreUniquePtr request,
      PreprocessResult&& preprocessResult,
      bool isInteractionCreatePresent,
      std::optional<int64_t>& interactionIdOpt);

 private:
  server::TServerObserver* observer_;
  RequestsRegistry* requestsRegistry_;

  void notifyTaskKilled();

  void sendErrorAndNotifyTaskKilled(
      ThriftRequestCoreUniquePtr request,
      TApplicationException::TApplicationExceptionType exceptionType,
      std::string message,
      const std::string& errorCode);

  void handleAppOverloadedException(
      ThriftRequestCoreUniquePtr request,
      AppOverloadedException& aoe,
      bool isInteractionCreatePresent,
      std::optional<int64_t>& interactionIdOpt);
};

} // namespace rocket
} // namespace apache::thrift
