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

// We already acquired a token, so now we should make as much progress as
// possible given one token. We will process and require requests from the pile
// until we find a request that haven't expired yet. We will then process that
// request and return. To make further progress we need to wait for another
// token.
void TokenBucketConcurrencyController::makeProgress() {
  while (auto requestOpt = pile_.dequeue()) {
    auto request = std::move(*requestOpt);
    if (expired(request)) {
      release(std::move(request));
      continue;
    }
    execute(std::move(request));
    return;
  }
  // If we got here then we couldn't find a request that hasn't expired yet, and
  // pile is empty now. We then return the token because we haven't used it to
  // process an (unexpired) request.
  returnToken();

  // If the got here then queue is empty. We now can disable slow mode, whether
  // it was enabled or not.
  clearSlowMode();
}

void TokenBucketConcurrencyController::fastPath() {
  executor_.add([this]() { makeProgress(); });
}

void TokenBucketConcurrencyController::onEnqueued() {
  if (consumeToken()) {
    fastPath();
    return;
  }

  if (enableSlowModeOnce()) {
    innerExecutor_->add([this]() {
      while (isSlowModeEnabled()) {
        blockingConsumeToken();
        fastPath();
      }
    });
  }
}

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
  request.setConcurrencyControllerNotification(this);
  request.requestData().setRequestExecutionBegin();
  AsyncProcessorHelper::executeRequest(std::move(request));
  request.requestData().setRequestExecutionEnd();
  notifyOnFinishExecution(request);
}

} // namespace apache::thrift
