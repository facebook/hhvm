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

#include <folly/Executor.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/async/processor/AsyncProcessorFunc.h>
#include <thrift/lib/cpp2/async/processor/EventTask.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallbackBase.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache::thrift {

template <typename ChildType>
class RequestTask final : public EventTask {
 public:
  RequestTask(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      folly::Executor::KeepAlive<> executor,
      Cpp2RequestContext* ctx,
      bool oneway,
      ChildType* childClass,
      AsyncProcessorFunc::ExecuteFunc<ChildType> executeFunc)
      : EventTask(
            std::move(req),
            std::move(serializedRequest),
            std::move(executor),
            ctx,
            oneway),
        childClass_(childClass),
        executeFunc_(executeFunc) {}

  void run() override;

 private:
  ChildType* childClass_;
  AsyncProcessorFunc::ExecuteFunc<ChildType> executeFunc_;
};

template <typename ChildType>
void RequestTask<ChildType>::run() {
  // Since this request was queued, reset the processBegin
  // time to the actual start time, and not the queue time.
  req_.requestContext()->getTimestamps().processBegin =
      std::chrono::steady_clock::now();
  if (!oneway_ && !req_.request()->getShouldStartProcessing()) {
    apache::thrift::HandlerCallbackBase::releaseRequest(
        apache::thrift::detail::ServerRequestHelper::request(std::move(req_)),
        apache::thrift::detail::ServerRequestHelper::eventBase(req_));
    return;
  }
  (childClass_->*executeFunc_)(std::move(req_));
}

} // namespace apache::thrift
