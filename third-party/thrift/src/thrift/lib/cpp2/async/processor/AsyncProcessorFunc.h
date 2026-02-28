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

#include <folly/container/F14Map.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/async/processor/ServerRequest.h>

namespace apache::thrift {

class AsyncProcessorFunc {
 public:
  template <typename Derived>
  using ProcessFunc = void (Derived::*)(
      ResponseChannelRequest::UniquePtr,
      SerializedCompressedRequest&&,
      Cpp2RequestContext* context,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm);

  template <typename Derived>
  using ExecuteFunc = void (Derived::*)(ServerRequest&& request);

  template <typename Derived>
  struct ProcessFuncs {
    ProcessFunc<Derived> compact;
    ProcessFunc<Derived> binary;
    ExecuteFunc<Derived> compactExecute;
    ExecuteFunc<Derived> binaryExecute;
  };

  template <typename ProcessFuncs>
  using ProcessMap = folly::F14ValueMap<std::string, ProcessFuncs>;

  template <typename Derived>
  using InteractionConstructor = std::unique_ptr<Tile> (Derived::*)();

  template <typename InteractionConstructor>
  using InteractionConstructorMap =
      folly::F14ValueMap<std::string, InteractionConstructor>;
};
} // namespace apache::thrift
