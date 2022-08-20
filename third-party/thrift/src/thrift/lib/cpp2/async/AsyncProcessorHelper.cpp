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

#include <thrift/lib/cpp2/async/AsyncProcessorHelper.h>

#include <fmt/core.h>

#include <thrift/lib/cpp/TApplicationException.h>

namespace apache::thrift {

/* static */ void AsyncProcessorHelper::sendUnknownMethodError(
    ResponseChannelRequest::UniquePtr request, std::string_view methodName) {
  auto message = fmt::format("Method name {} not found", methodName);
  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::UNKNOWN_METHOD, std::move(message)),
      kMethodUnknownErrorCode);
}

/* static */ void AsyncProcessorHelper::executeRequest(
    ServerRequest&& serverRequest) {
  // Since this request was queued, reset the processBegin
  // time to the actual start time, and not the queue time.
  auto ctx = serverRequest.requestContext();
  if (ctx->getTimestamps().getSamplingStatus().isEnabled()) {
    ctx->getTimestamps().processBegin = std::chrono::steady_clock::now();
  }

  auto ap = detail::ServerRequestHelper::asyncProcessor(serverRequest);
  const AsyncProcessorFactory::MethodMetadata& metadata =
      *serverRequest.methodMetadata();
  folly::RequestContextScopeGuard rctx(serverRequest.follyRequestContext());
  try {
    ap->executeRequest(std::move(serverRequest), metadata);
  } catch (std::exception& ex) {
    // Temporary code - just ensure that a failure produces an error.
    // TODO: T113039894
    folly::exception_wrapper ew(std::current_exception(), ex);
    auto eb = detail::ServerRequestHelper::eventBase(serverRequest);
    auto req = detail::ServerRequestHelper::request(std::move(serverRequest));
    eb->runInEventBaseThread([request = std::move(req)]() {
      request->sendErrorWrapped(
          folly::make_exception_wrapper<TApplicationException>(
              TApplicationException::INTERNAL_ERROR,
              "AsyncProcessorHelper::executeRequest - resource pools mode"),
          kUnknownErrorCode);
    });
    return;
  }
}

/* static */ SelectPoolResult AsyncProcessorHelper::selectResourcePool(
    const ServerRequest&,
    const AsyncProcessorFactory::MethodMetadata& methodMetadata) {
  switch (methodMetadata.executorType) {
    case AsyncProcessorFactory::MethodMetadata::ExecutorType::EVB:
      return std::ref(ResourcePoolHandle::defaultSync());
    default:
      return std::ref(ResourcePoolHandle::defaultAsync());
  }
}

} // namespace apache::thrift
