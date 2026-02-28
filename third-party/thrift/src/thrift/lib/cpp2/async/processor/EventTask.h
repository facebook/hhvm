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

#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/processor/ServerRequest.h>
#include <thrift/lib/cpp2/async/processor/ServerRequestHelper.h>

namespace apache::thrift {

class EventTask : public concurrency::Runnable, public InteractionTask {
 public:
  EventTask(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      folly::Executor::KeepAlive<> executor,
      Cpp2RequestContext* ctx,
      bool oneway)
      : req_(std::move(req), std::move(serializedRequest), ctx, {}, {}, {}, {}),
        oneway_(oneway) {
    detail::ServerRequestHelper::setExecutor(req_, std::move(executor));
  }

  ~EventTask() override;

  void expired();
  void failWith(folly::exception_wrapper ex, std::string exCode) override;

  void setTile(TilePtr&& tile) override;

  friend class TilePromise;

 protected:
  ServerRequest req_;
  bool oneway_;
};

} // namespace apache::thrift
