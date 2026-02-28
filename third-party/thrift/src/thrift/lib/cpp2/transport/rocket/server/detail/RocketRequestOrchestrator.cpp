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

#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketRequestOrchestrator.h>

#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketThriftRequests.h>

namespace apache::thrift::rocket {

RocketRequestOrchestrator::RocketRequestOrchestrator(
    folly::AsyncTransport* transport,
    Cpp2ConnContext& connContext,
    folly::once_flag& setupLoggingFlag,
    AsyncProcessorFactory* processorFactory,
    std::shared_ptr<AsyncProcessor> processor,
    Cpp2Worker::PerServiceMetadata* serviceMetadata,
    std::shared_ptr<concurrency::ThreadManager> threadManager,
    server::ServerConfigs* serverConfigs,
    RequestsRegistry* requestsRegistry,
    RocketErrorHandler* errorHandler,
    ThriftServer* server,
    Cpp2Worker* worker)
    : transport_(transport),
      connContext_(connContext),
      setupLoggingFlag_(setupLoggingFlag),
      processorFactory_(processorFactory),
      processor_(processor),
      serviceMetadata_(serviceMetadata),
      threadManager_(threadManager),
      serverConfigs_(serverConfigs),
      requestsRegistry_(requestsRegistry),
      errorHandler_(errorHandler),
      server_(server),
      worker_(worker) {}

} // namespace apache::thrift::rocket
