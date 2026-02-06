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

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include <folly/io/Cursor.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>

namespace folly {
class AsyncTransport;
class EventBase;
} // namespace folly

namespace apache::thrift {

class AsyncProcessor;
class AsyncProcessorFactory;
class Cpp2ConnContext;
class RequestSetupMetadata;
class ThriftServer;

namespace concurrency {
class ThreadManager;
}

namespace server {
class ServerConfigs;
}

class RequestsRegistry;

namespace rocket {

class RocketErrorHandler;
class RocketRequestHandler;
class RocketServerConnection;
class SetupFrame;
class SetupFrameHandler;
class SetupFrameInterceptor;

struct ProcessorInfo;

/**
 * RocketSetupProcessor handles all setup frame processing logic for Rocket
 * server connections. It validates setup frames, negotiates protocol versions,
 * configures processors, and creates the RocketRequestHandler for processing
 * subsequent requests.
 */
class RocketSetupProcessor {
 public:
  RocketSetupProcessor(
      Cpp2Worker& worker,
      Cpp2ConnContext& connContext,
      const std::vector<std::unique_ptr<SetupFrameHandler>>& handlers,
      const std::vector<std::unique_ptr<SetupFrameInterceptor>>& interceptors,
      int32_t& version,
      folly::AsyncTransport* transport,
      std::chrono::milliseconds maxResponseWriteTime,
      folly::once_flag* setupLoggingFlag,
      RocketErrorHandler* errorHandler);

  /**
   * Process a setup frame and create a RocketRequestHandler.
   * Returns nullptr if setup fails (connection will be closed).
   * If isCustomHandler is provided, it will be set to true if a custom
   * handler was used, false otherwise.
   */
  std::unique_ptr<RocketRequestHandler> handleSetupFrame(
      SetupFrame&& frame,
      RocketServerConnection& connection,
      const std::function<void()>& onConnectionAttempted = nullptr,
      bool* isCustomHandler = nullptr);

 private:
  bool validateSetupFrameBasics(
      const SetupFrame& frame, RocketServerConnection& connection);

  bool validateProtocolKey(
      folly::io::Cursor& cursor,
      const SetupFrame& frame,
      RocketServerConnection& connection);

  bool deserializeSetupMetadata(
      RequestSetupMetadata& meta,
      folly::io::Cursor& cursor,
      const SetupFrame& frame,
      RocketServerConnection& connection);

  bool negotiateProtocolVersion(
      const RequestSetupMetadata& meta, RocketServerConnection& connection);

  bool validateMimeTypes(
      const SetupFrame& frame, RocketServerConnection& connection);

  bool runSetupInterceptors(
      const RequestSetupMetadata& meta, RocketServerConnection& connection);

  std::unique_ptr<RocketRequestHandler> setupProcessor(
      const RequestSetupMetadata& meta,
      RocketServerConnection& connection,
      bool& isCustomHandler);

  std::unique_ptr<RocketRequestHandler> setupCustomProcessor(
      const std::shared_ptr<ProcessorInfo>& processorInfo,
      RocketServerConnection& connection);

  std::unique_ptr<RocketRequestHandler> setupDefaultProcessor();

  void configureConnectionSettings(
      const RequestSetupMetadata& meta, RocketServerConnection& connection);

  folly::Expected<std::optional<CustomCompressionSetupResponse>, std::string>
  handleSetupFrameCustomCompression(
      const CompressionSetupRequest& setupRequest,
      RocketServerConnection& connection);

  Cpp2Worker& worker_;
  Cpp2ConnContext& connContext_;
  const std::vector<std::unique_ptr<SetupFrameHandler>>& setupFrameHandlers_;
  const std::vector<std::unique_ptr<SetupFrameInterceptor>>&
      setupFrameInterceptors_;
  int32_t& version_;
  folly::AsyncTransport* transport_;
  std::chrono::milliseconds maxResponseWriteTime_;
  folly::once_flag* setupLoggingFlag_;
  RocketErrorHandler* errorHandler_;
};

} // namespace rocket
} // namespace apache::thrift
