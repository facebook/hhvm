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

#include <memory>
#include <optional>

#include <thrift/lib/cpp2/async/AsyncProcessor.h>

namespace folly {
class EventBase;
}

namespace apache {
namespace thrift {

class AsyncProcessor;
class Cpp2Worker;
class RequestSetupMetadata;
class ThriftServer;
class RequestsRegistry;

namespace concurrency {
class ThreadManager;
} // namespace concurrency

namespace server {
class ServerConfigs;
} // namespace server

namespace rocket {

struct ProcessorInfo {
  ProcessorInfo(
      apache::thrift::AsyncProcessorFactory& processorFactory,
      std::shared_ptr<apache::thrift::concurrency::ThreadManager> threadManager,
      server::ServerConfigs& serverConfigs,
      RequestsRegistry* requestsRegistry,
      bool useResourcePool = false)
      : processorFactory_(processorFactory),
        threadManager_(std::move(threadManager)),
        serverConfigs_(serverConfigs),
        requestsRegistry_(std::move(requestsRegistry)),
        useResourcePool_(useResourcePool) {}

  apache::thrift::AsyncProcessorFactory& processorFactory_;
  std::shared_ptr<apache::thrift::concurrency::ThreadManager> threadManager_;
  server::ServerConfigs& serverConfigs_;
  RequestsRegistry* FOLLY_NULLABLE requestsRegistry_;
  bool useResourcePool_;
};

/*
 * An interface used by ThriftServer to allow overriding
 * default processor and thread pool based on connection setup frame.
 */
class SetupFrameHandler {
 public:
  SetupFrameHandler() = default;
  virtual ~SetupFrameHandler() = default;
  SetupFrameHandler(const SetupFrameHandler&) = delete;

  virtual std::optional<ProcessorInfo> tryHandle(
      const RequestSetupMetadata& meta) = 0;
};

} // namespace rocket
} // namespace thrift
} // namespace apache
