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

#include <thread>
#include <glog/logging.h>

#include <thrift/lib/cpp2/async/AsyncProcessorHelper.h>
#include <thrift/lib/cpp2/server/TokenBucketConcurrencyController.h>

using namespace apache::thrift::detail;

namespace apache::thrift {
/*static*/ bool TokenBucketConcurrencyController::expired(
    const ServerRequest& request) {
  return !request.request()->isOneway() &&
      !request.request()->getShouldStartProcessing();
}

/*static*/ void TokenBucketConcurrencyController::release(
    ServerRequest&& request) {
  auto eb = ServerRequestHelper::eventBase(request);
  auto req = ServerRequestHelper::request(std::move(request));
  HandlerCallbackBase::releaseRequest(std::move(req), eb);
}

void TokenBucketConcurrencyController::execute(ServerRequest&& request) {
  request.requestData().setRequestExecutionBegin();
  AsyncProcessorHelper::executeRequest(std::move(request));
  request.requestData().setRequestExecutionEnd();
  notifyOnFinishExecution(request);
}

} // namespace apache::thrift
