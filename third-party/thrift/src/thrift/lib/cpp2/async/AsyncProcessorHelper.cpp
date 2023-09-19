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
#include <folly/GLog.h>
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
  ctx->getTimestamps().processBegin = std::chrono::steady_clock::now();

  auto ap = detail::ServerRequestHelper::asyncProcessor(serverRequest);
  const AsyncProcessorFactory::MethodMetadata& metadata =
      *serverRequest.methodMetadata();
  folly::RequestContextScopeGuard rctx(serverRequest.follyRequestContext());
  try {
    ap->executeRequest(std::move(serverRequest), metadata);
  } catch (...) {
    LOG(WARNING) << "exception in executeRequest: "
                 << folly::exceptionStr(std::current_exception());

    auto eb = detail::ServerRequestHelper::eventBase(serverRequest);
    auto req = detail::ServerRequestHelper::request(std::move(serverRequest));
    // We can return an error if the request has not been consumed already
    if (eb && req) {
      eb->runInEventBaseThread([request = std::move(req)]() {
        request->sendErrorWrapped(
            folly::make_exception_wrapper<TApplicationException>(
                TApplicationException::INTERNAL_ERROR,
                "AsyncProcessorHelper::executeRequest exception"),
            kUnknownErrorCode);
      });
    }
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
