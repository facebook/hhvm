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
#include <thrift/lib/cpp2/server/Overload.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/SetupFrameHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/SetupFrameInterceptor.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketErrorHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketRequestHandler.h>

THRIFT_FLAG_DECLARE_bool(rocket_server_reset_connctx_userdata_on_close);

namespace folly {
class AsyncTransport;
} // namespace folly

namespace apache::thrift {

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
class RequestChannelFrame;
class SetupFrame;

class RocketServerConnection;
class RocketServerFrameContext;
class RocketStreamClientCallback;
struct ChannelRequestCallbackFactory;

class RefactoredThriftRocketServerHandler : public RocketServerHandler {
 public:
  RefactoredThriftRocketServerHandler(
      std::shared_ptr<Cpp2Worker> worker,
      const folly::SocketAddress& clientAddress,
      folly::AsyncTransport* transport,
      const std::vector<std::unique_ptr<SetupFrameHandler>>& handlers,
      const std::vector<std::unique_ptr<SetupFrameInterceptor>>& interceptors);

  ~RefactoredThriftRocketServerHandler() override;

  // Non-copyable and non-movable
  RefactoredThriftRocketServerHandler(
      const RefactoredThriftRocketServerHandler&) = delete;
  RefactoredThriftRocketServerHandler& operator=(
      const RefactoredThriftRocketServerHandler&) = delete;
  RefactoredThriftRocketServerHandler(RefactoredThriftRocketServerHandler&&) =
      delete;
  RefactoredThriftRocketServerHandler& operator=(
      RefactoredThriftRocketServerHandler&&) = delete;

  void handleSetupFrame(
      SetupFrame&& frame, IRocketServerConnection& context) final;
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
      ChannelRequestCallbackFactory clientCallback) final;
  void connectionClosing() final;
  void requestComplete() final;
  void terminateInteraction(int64_t id) final;

  Cpp2ConnContext* getCpp2ConnContext() final { return &connContext_; }
  void onBeforeHandleFrame() override;
  int32_t getVersion() const final { return version_; }

 private:
  const std::shared_ptr<Cpp2Worker> worker_;
  const std::shared_ptr<void> connectionGuard_;
  folly::AsyncTransport* transport_;
  Cpp2ConnContext connContext_;
  const std::vector<std::unique_ptr<SetupFrameHandler>>& setupFrameHandlers_;
  const std::vector<std::unique_ptr<SetupFrameInterceptor>>&
      setupFrameInterceptors_;
  std::unique_ptr<RocketErrorHandler> errorHandler_;
  std::unique_ptr<RocketRequestHandler> requestHandler_;

  int32_t version_;
  std::chrono::milliseconds maxResponseWriteTime_;
  bool resetConnCtxUserDataOnClose_;

  folly::once_flag setupLoggingFlag_;

  bool didExecuteServiceInterceptorsOnConnection_ = false;
  void invokeServiceInterceptorsOnConnectionAttempted(
      IRocketServerConnection&) noexcept;
  void invokeServiceInterceptorsOnConnectionEstablished(
      IRocketServerConnection&) noexcept;
  void invokeServiceInterceptorsOnConnectionClosed() noexcept;
};

} // namespace rocket
} // namespace apache::thrift
