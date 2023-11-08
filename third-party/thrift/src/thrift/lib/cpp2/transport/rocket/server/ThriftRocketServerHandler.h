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

#include <chrono>
#include <memory>
#include <vector>

#include <folly/Function.h>
#include <folly/SocketAddress.h>

#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/SetupFrameHandler.h>

namespace folly {
class AsyncTransport;
} // namespace folly

namespace apache {
namespace thrift {

class AsyncProcessor;
class Cpp2ConnContext;
class RequestRpcMetadata;
class ThriftRequestCore;
using ThriftRequestCoreUniquePtr =
    std::unique_ptr<ThriftRequestCore, RequestsRegistry::Deleter>;

namespace concurrency {
class ThreadManager;
} // namespace concurrency

namespace server {
class ServerConfigs;
} // namespace server

namespace rocket {

class Payload;
class RequestFnfFrame;
class RequestResponseFrame;
class RequestStreamFrame;
class SetupFrame;

class RocketServerConnection;
class RocketServerFrameContext;

class ThriftRocketServerHandler : public RocketServerHandler {
 public:
  ThriftRocketServerHandler(
      std::shared_ptr<Cpp2Worker> worker,
      const folly::SocketAddress& clientAddress,
      folly::AsyncTransport* transport,
      const std::vector<std::unique_ptr<SetupFrameHandler>>& handlers);

  ~ThriftRocketServerHandler() override;

  void handleSetupFrame(
      SetupFrame&& frame, RocketServerConnection& context) final;
  void handleRequestResponseFrame(
      RequestResponseFrame&& frame, RocketServerFrameContext&& context) final;
  void handleRequestFnfFrame(
      RequestFnfFrame&& frame, RocketServerFrameContext&& context) final;
  void handleRequestStreamFrame(
      RequestStreamFrame&& frame,
      RocketServerFrameContext&& context,
      RocketStreamClientCallback* clientCallback) final;
  void handleRequestChannelFrame(
      RequestChannelFrame&& frame,
      RocketServerFrameContext&& context,
      RocketSinkClientCallback* clientCallback) final;
  void connectionClosing() final;

  apache::thrift::server::TServerObserver::SamplingStatus shouldSample(
      const transport::THeader& header);

  void requestComplete() final;

  void terminateInteraction(int64_t id) final;

  Cpp2ConnContext* getCpp2ConnContext() final { return &connContext_; }

  void onBeforeHandleFrame() override;

  int32_t getVersion() const final { return version_; }

 private:
  const std::shared_ptr<Cpp2Worker> worker_;
  const std::shared_ptr<void> connectionGuard_;
  folly::AsyncTransport*
      transport_; // connContext_ exposes this only as `const`
  Cpp2ConnContext connContext_;
  const std::vector<std::unique_ptr<SetupFrameHandler>>& setupFrameHandlers_;
  AsyncProcessorFactory* processorFactory_ = nullptr;
  std::shared_ptr<AsyncProcessorFactory> processorFactoryStorage_;
  Cpp2Worker::PerServiceMetadata* serviceMetadata_ = nullptr;
  std::shared_ptr<AsyncProcessor> processor_;
  std::shared_ptr<concurrency::ThreadManager> threadManager_;
  server::ServerConfigs* serverConfigs_ = nullptr;
  RequestsRegistry* requestsRegistry_ = nullptr;
  folly::EventBase* eventBase_;

  uint32_t sampleRate_{0};
  static thread_local uint32_t sample_;

  int32_t version_;
  std::chrono::milliseconds maxResponseWriteTime_;

  folly::once_flag setupLoggingFlag_;

  template <class F>
  void handleRequestCommon(
      Payload&& payload, F&& makeRequest, RpcKind expectedKind);

  FOLLY_NOINLINE void handleRequestWithBadMetadata(
      ThriftRequestCoreUniquePtr request);
  FOLLY_NOINLINE void handleRequestWithBadChecksum(
      ThriftRequestCoreUniquePtr request);
  FOLLY_NOINLINE void handleRequestOverloadedServer(
      ThriftRequestCoreUniquePtr request,
      const std::string& errorCode,
      const std::string& errorMessage);
  FOLLY_NOINLINE void handleAppError(
      ThriftRequestCoreUniquePtr request,
      const PreprocessResult& appErrorResult);
  FOLLY_NOINLINE void handleDecompressionFailure(
      ThriftRequestCoreUniquePtr request, std::string&& reason);
  FOLLY_NOINLINE void handleServerNotReady(ThriftRequestCoreUniquePtr request);
  FOLLY_NOINLINE void handleServerShutdown(ThriftRequestCoreUniquePtr request);

  enum class InjectedFault { ERROR, DROP, DISCONNECT };
  FOLLY_NOINLINE void handleInjectedFault(
      ThriftRequestCoreUniquePtr request, InjectedFault fault);
};

} // namespace rocket
} // namespace thrift
} // namespace apache
