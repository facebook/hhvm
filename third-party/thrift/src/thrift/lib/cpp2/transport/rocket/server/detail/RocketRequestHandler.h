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

#include <folly/Function.h>
#include <folly/synchronization/CallOnce.h>

#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/transport/core/ThriftRequest.h>

namespace folly {
class AsyncTransport;
class EventBase;
} // namespace folly

namespace apache::thrift {

class AsyncProcessor;
class AsyncProcessorFactory;
class Cpp2ConnContext;
class RequestsRegistry;
class ThriftServer;
class ThriftRequestCore;

using ThriftRequestCoreUniquePtr =
    std::unique_ptr<ThriftRequestCore, RequestsRegistry::Deleter>;

namespace concurrency {
class ThreadManager;
}

namespace server {
class ServerConfigs;
}

namespace rocket {

class IRocketServerConnection;
class Payload;
class RequestFnfFrame;
class RequestResponseFrame;
class RequestStreamFrame;
class RequestChannelFrame;
class RocketErrorHandler;
class RocketServerFrameContext;
class RocketStreamClientCallback;
struct ChannelRequestCallbackFactory;

/**
 * RocketRequestHandler encapsulates all request processing logic for a
 * Rocket connection after successful setup. It owns all setup results and
 * provides a clean interface for processing different request types.
 *
 * This class eliminates the need for the handler to pass 'this' pointers
 * or capture handler members in lambdas, providing better separation of
 * concerns. The handler is created by RocketSetupProcessor after successful
 * setup frame validation.
 */
class RocketRequestHandler {
 public:
  RocketRequestHandler(
      AsyncProcessorFactory* processorFactory,
      std::shared_ptr<AsyncProcessorFactory> processorFactoryStorage,
      Cpp2Worker::PerServiceMetadata* serviceMetadata,
      std::shared_ptr<AsyncProcessor> processor,
      std::shared_ptr<concurrency::ThreadManager> threadManager,
      server::ServerConfigs* serverConfigs,
      RequestsRegistry* requestsRegistry,
      folly::EventBase* eventBase,
      Cpp2ConnContext* connContext,
      ThriftServer* server,
      Cpp2Worker* worker,
      folly::AsyncTransport* transport,
      uint32_t sampleRate,
      int32_t version,
      std::chrono::milliseconds maxResponseWriteTime,
      folly::once_flag* setupLoggingFlag,
      RocketErrorHandler* errorHandler);

  // Non-copyable and non-movable
  RocketRequestHandler(const RocketRequestHandler&) = delete;
  RocketRequestHandler& operator=(const RocketRequestHandler&) = delete;
  RocketRequestHandler(RocketRequestHandler&&) = delete;
  RocketRequestHandler& operator=(RocketRequestHandler&&) = delete;
  ~RocketRequestHandler() = default;

  /**
   * Handle request-response frame.
   */
  void handleRequestResponse(
      RequestResponseFrame&& frame, RocketServerFrameContext&& context);

  /**
   * Handle fire-and-forget frame.
   */
  void handleRequestFnf(
      RequestFnfFrame&& frame, RocketServerFrameContext&& context);

  /**
   * Handle request-stream frame.
   */
  void handleRequestStream(
      RequestStreamFrame&& frame,
      RocketServerFrameContext&& context,
      RocketStreamClientCallback* clientCallback);

  /**
   * Handle request-channel frame (sink/bidi).
   */
  void handleRequestChannel(
      RequestChannelFrame&& frame,
      RocketServerFrameContext&& context,
      ChannelRequestCallbackFactory clientCallback);

  /**
   * Terminate an interaction by ID.
   */
  void terminateInteraction(int64_t id);

  /**
   * Destroy all interactions (called on connection close).
   */
  void destroyAllInteractions();

  /**
   * Decrement active request count (called on request completion).
   */
  void requestComplete();

  /**
   * Get connection context.
   */
  Cpp2ConnContext* getConnContext() const { return connContext_; }

  /**
   * Coalesce processor with server-scoped legacy event handlers.
   * Called by RocketSetupProcessor after handler creation.
   */
  void coalesceProcessorWithLegacyEventHandlers();

 private:
  // Request factory methods
  ThriftRequestCoreUniquePtr makeRequestResponse(
      RequestRpcMetadata&& md,
      rocket::Payload&& debugPayload,
      std::shared_ptr<folly::RequestContext> ctx,
      RocketServerFrameContext&& context);

  ThriftRequestCoreUniquePtr makeRequestFnf(
      RequestRpcMetadata&& md,
      rocket::Payload&& debugPayload,
      std::shared_ptr<folly::RequestContext> ctx,
      RocketServerFrameContext&& context);

  ThriftRequestCoreUniquePtr makeRequestStream(
      RequestRpcMetadata&& md,
      rocket::Payload&& debugPayload,
      std::shared_ptr<folly::RequestContext> ctx,
      RocketServerFrameContext&& context,
      RocketStreamClientCallback* clientCallback);

  ThriftRequestCoreUniquePtr makeRequestSink(
      RequestRpcMetadata&& md,
      rocket::Payload&& debugPayload,
      std::shared_ptr<folly::RequestContext> ctx,
      RocketServerFrameContext&& context,
      ChannelRequestCallbackFactory& clientCallback);

  // Sampling method (no longer needs external 'this')
  apache::thrift::server::TServerObserver::SamplingStatus shouldSample(
      const transport::THeader& header);

  // Main request processing (template to handle different request factories)
  template <class F>
  void handleRequestCommon(
      Payload&& payload,
      F&& makeRequest,
      RpcKind expectedKind,
      IRocketServerConnection& connection);

  // Configuration members (from setup)
  AsyncProcessorFactory* processorFactory_;
  std::shared_ptr<AsyncProcessorFactory> processorFactoryStorage_;
  Cpp2Worker::PerServiceMetadata* serviceMetadata_;
  std::shared_ptr<AsyncProcessor> processor_;
  std::shared_ptr<concurrency::ThreadManager> threadManager_;
  server::ServerConfigs* serverConfigs_;
  RequestsRegistry* requestsRegistry_;
  folly::EventBase* eventBase_;

  // Connection context (reference, owned by facade)
  Cpp2ConnContext* connContext_;

  // Infrastructure
  ThriftServer* server_;
  Cpp2Worker* worker_;
  folly::AsyncTransport* transport_;

  // Sampling state
  uint32_t sampleRate_;
  static thread_local uint32_t sample_;

  // Protocol configuration
  int32_t version_;
  std::chrono::milliseconds maxResponseWriteTime_;

  // Shared state with facade
  folly::once_flag* setupLoggingFlag_;
  RocketErrorHandler* errorHandler_;
};

} // namespace rocket
} // namespace apache::thrift
