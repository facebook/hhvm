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

#include <thrift/lib/cpp2/fast_thrift/bench/tcp/TcpClient.h>

#include <folly/container/F14Map.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

namespace apache::thrift::fast_thrift::bench {

/**
 * AppAdapter is a minimal client-side adapter for benchmarking.
 *
 * This adapter owns the transport handler and pipeline, similar to
 * ThriftClientChannel. It is optimized for tight benchmark loops:
 * - Promise/Future based response synchronization
 * - F14FastMap for tracking pending requests
 * - Minimal overhead per round-trip
 */
class TcpClient::AppAdapter {
 public:
  using BytesPtr = apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
  using TypeErasedBox =
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
  using Result = apache::thrift::fast_thrift::channel_pipeline::Result;
  using RequestId = uint64_t;

  explicit AppAdapter(folly::AsyncTransport::UniquePtr socket)
      : evb_(socket->getEventBase()),
        transportHandler_(
            apache::thrift::fast_thrift::transport::TransportHandler::create(
                std::move(socket))) {}

  ~AppAdapter() {
    if (transportHandler_) {
      transportHandler_->onClose(folly::exception_wrapper{});
    }
  }

  AppAdapter(const AppAdapter&) = delete;
  AppAdapter& operator=(const AppAdapter&) = delete;
  AppAdapter(AppAdapter&&) = delete;
  AppAdapter& operator=(AppAdapter&&) = delete;

  void setPipeline(
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl::Ptr
          pipeline) {
    pipeline_ = std::move(pipeline);
    transportHandler_->setPipeline(*pipeline_);
    transportHandler_->resumeRead();
  }

  apache::thrift::fast_thrift::transport::TransportHandler* transportHandler()
      const {
    return transportHandler_.get();
  }

  apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator*
  bufferAllocator() noexcept {
    return &allocator_;
  }
  /**
   * Called by the pipeline when a response message is received.
   * Looks up the pending request and fulfills its promise.
   */
  Result onRead(TypeErasedBox&& msg) noexcept {
    auto iobuf = std::move(msg).take<std::unique_ptr<folly::IOBuf>>();

    // Complete the oldest pending request
    if (!pendingRequests_.empty()) {
      auto it = pendingRequests_.begin();
      it->second.setValue(std::move(iobuf));
      pendingRequests_.erase(it);
    }

    return Result::Success;
  }

  /**
   * Send a payload to the server and return a future that completes when
   * the server echoes it back, containing the response IOBuf.
   * NOTE: Must be called from the EventBase thread.
   */
  folly::Future<std::unique_ptr<folly::IOBuf>> echo(
      std::unique_ptr<folly::IOBuf> data) {
    DCHECK(evb_->isInEventBaseThread());

    folly::Promise<std::unique_ptr<folly::IOBuf>> promise;
    auto future = promise.getFuture();

    RequestId requestId = nextRequestId_++;
    auto [it, inserted] =
        pendingRequests_.emplace(requestId, std::move(promise));
    DCHECK(inserted);

    auto result = pipeline_->fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(data)));
    switch (result) {
      case Result::Success:
        break;
      case Result::Backpressure:
        break;
      case Result::Error:
        it->second.setException(std::runtime_error("fireWrite failed"));
        pendingRequests_.erase(it);
        break;
    }

    return future;
  }

  void onException(folly::exception_wrapper&& /*e*/) noexcept {
    for (auto& [requestId, promise] : pendingRequests_) {
      promise.setException(std::runtime_error("Connection error"));
    }
    pendingRequests_.clear();
  }

  // Lifecycle methods
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

 private:
  folly::EventBase* evb_;
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      transportHandler_;
  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl::Ptr pipeline_;
  apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator
      allocator_;
  RequestId nextRequestId_{0};
  folly::F14FastMap<RequestId, folly::Promise<std::unique_ptr<folly::IOBuf>>>
      pendingRequests_;
};

TcpClient::TcpClient() = default;

TcpClient::~TcpClient() = default;

TcpClient::TcpClient(TcpClient&&) noexcept = default;
TcpClient& TcpClient::operator=(TcpClient&&) noexcept = default;

void TcpClient::connect(
    folly::EventBase* evb,
    const folly::SocketAddress& serverAddress,
    folly::AsyncSocket::ConnectCallback* connectCb,
    size_t zeroCopyThreshold) {
  auto socket = folly::AsyncSocket::newSocket(evb);

  // Configure MSG_ZEROCOPY before connect (AsyncSocket stores the value
  // and applies SO_ZEROCOPY sockopt after the socket is connected)
  if (zeroCopyThreshold > 0) {
    if (!socket->setZeroCopy(true)) {
      XLOG(WARN) << "MSG_ZEROCOPY not supported on this socket";
    }
    socket->setZeroCopyEnableThreshold(zeroCopyThreshold);
  }

  socket->connect(connectCb, serverAddress);

  appAdapter_ = std::make_unique<AppAdapter>(std::move(socket));

  auto pipeline =
      apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder<
          apache::thrift::fast_thrift::transport::TransportHandler,
          AppAdapter,
          apache::thrift::fast_thrift::channel_pipeline::
              SimpleBufferAllocator>()
          .setAllocator(appAdapter_->bufferAllocator())
          .setEventBase(evb)
          .setHead(appAdapter_->transportHandler())
          .setTail(appAdapter_.get())
          .build();

  appAdapter_->setPipeline(std::move(pipeline));
}

folly::Future<std::unique_ptr<folly::IOBuf>> TcpClient::echo(
    std::unique_ptr<folly::IOBuf> data) {
  return appAdapter_->echo(std::move(data));
}

void TcpClient::shutdown() {
  appAdapter_.reset();
}

} // namespace apache::thrift::fast_thrift::bench
