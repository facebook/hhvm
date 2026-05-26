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
#include <memory>

#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/logging/xlog.h>
#include <folly/net/NetworkSocket.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/SocketOptions.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/Messages.h>

namespace apache::thrift::fast_thrift::connection {

// Head endpoint of the acceptance pipeline. Owns the AsyncServerSocket and
// is itself the AcceptCallback the socket dispatches to — no trampoline.
//
// Pipeline lifecycle drives accept lifecycle:
//   onPipelineActive   → bind + listen + addAcceptCallback + startAccepting
//   onPipelineInactive → removeAcceptCallback (no more accepts after return)
class ConnectionListener : public folly::DelayedDestruction,
                           public folly::AsyncServerSocket::AcceptCallback {
 public:
  using Ptr = std::
      unique_ptr<ConnectionListener, folly::DelayedDestruction::Destructor>;

  ConnectionListener(
      folly::EventBase* evb,
      folly::SocketAddress address,
      SocketOptions socketOptions,
      bool enableReusePortBpfSpread) noexcept
      : evb_(evb),
        address_(std::move(address)),
        socketOptions_(socketOptions),
        enableReusePortBpfSpread_(enableReusePortBpfSpread),
        socket_(new folly::AsyncServerSocket(evb_)) {}

  ConnectionListener(const ConnectionListener&) = delete;
  ConnectionListener& operator=(const ConnectionListener&) = delete;
  ConnectionListener(ConnectionListener&&) = delete;
  ConnectionListener& operator=(ConnectionListener&&) = delete;

  // Bind to the pipeline. Takes a DestructorGuard so the pipeline stays
  // alive across any async path that may call back into this endpoint.
  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept {
    DCHECK(pipeline);
    DCHECK(!pipeline_) << "setPipeline called twice without resetPipeline";
    pipeline_ = pipeline;
    pipelineGuard_ =
        std::make_unique<folly::DelayedDestruction::DestructorGuard>(pipeline);
  }

  void resetPipeline() noexcept {
    pipeline_ = nullptr;
    pipelineGuard_.reset();
  }

  folly::SocketAddress getAddress() const {
    folly::SocketAddress out;
    socket_->getAddress(&out);
    return out;
  }

  // Activate the pipeline and start accepting. Pipeline must be wired
  // first via setPipeline(). Runs on evb_.
  void start() {
    DCHECK(pipeline_) << "ConnectionListener::start called before setPipeline";
    pipeline_->activate();
    socket_->setReusePortEnabled(true);
    if (socketOptions_.tfoEnabled) {
      socket_->setTFOEnabled(true, socketOptions_.tfoQueueSize);
    }
    socket_->bind(address_);
    socket_->listen(static_cast<int>(socketOptions_.listenBacklog));
    if (enableReusePortBpfSpread_) {
      attachReusePortBpfSpread();
    }
    socket_->addAcceptCallback(this, evb_);
    socket_->startAccepting();
  }

  // Stop accepting and deactivate the pipeline. removeAcceptCallback is
  // async — it queues a loop callback that fires acceptStopped() later.
  // We can't block on that callback here because stop() is often called
  // from inside an EVB callback (handler/manager teardown), and blocking
  // the loop would prevent the queued callback from ever firing. Instead
  // we take a self-DestructorGuard; acceptStopped() releases it. Safe to
  // drop the owning unique_ptr right after stop() returns — destroy() on
  // a DD with outstanding guards defers real destruction.
  void stop() {
    stopGuard_ =
        std::make_unique<folly::DelayedDestruction::DestructorGuard>(this);
    socket_->removeAcceptCallback(this, evb_);
    if (pipeline_) {
      pipeline_->deactivate();
    }
  }

  // === AsyncServerSocket::AcceptCallback ===

  void connectionAccepted(
      folly::NetworkSocket fd,
      const folly::SocketAddress& clientAddr,
      AcceptInfo) noexcept override {
    if (FOLLY_UNLIKELY(!pipeline_)) {
      folly::netops::close(fd);
      return;
    }
    XLOG(DBG3) << "Connection accepted from " << clientAddr.describe();
    auto socket = folly::AsyncSocket::newSocket(evb_, fd);
    socket->setMaxReadsPerEvent(socketOptions_.maxReadsPerEvent);
    folly::AsyncTransport::UniquePtr transport(socket.release());
    ConnectionMessage msg{
        .transport = std::move(transport),
        .clientAddr = clientAddr,
    };
    auto result =
        pipeline_->fireRead(channel_pipeline::erase_and_box(std::move(msg)));
    switch (result) {
      case channel_pipeline::Result::Success:
        return;
      case channel_pipeline::Result::Backpressure:
        XLOG_EVERY_MS(WARN, 1000)
            << "Acceptance pipeline reported backpressure from "
            << clientAddr.describe();
        return;
      case channel_pipeline::Result::Error:
        XLOG(WARN) << "Acceptance pipeline rejected connection from "
                   << clientAddr.describe();
        return;
    }
  }

  void acceptError(folly::exception_wrapper ew) noexcept override {
    XLOG(ERR) << "Accept error: " << ew.what();
  }

  void acceptStopped() noexcept override {
    XLOG(DBG3) << "Accept stopped";
    stopGuard_.reset();
  }

  // === HeadEndpointHandler — no-arg lifecycle / one-arg onWrite per
  // EndpointAdapter's concept ===

  channel_pipeline::Result onWrite(channel_pipeline::TypeErasedBox&&) noexcept {
    // Acceptance pipeline is one-way; outbound writes are an error.
    return channel_pipeline::Result::Error;
  }

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onReadReady() noexcept {}

 protected:
  ~ConnectionListener() override = default;

 private:
  void attachReusePortBpfSpread() noexcept;

  folly::EventBase* evb_;
  folly::SocketAddress address_;
  SocketOptions socketOptions_;
  bool enableReusePortBpfSpread_;
  folly::AsyncServerSocket::UniquePtr socket_;
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> pipelineGuard_;
  // Self-guard taken in stop(); released in acceptStopped(). Keeps `this`
  // alive across the queued loop callback that removeAcceptCallback fires.
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> stopGuard_;
};

} // namespace apache::thrift::fast_thrift::connection
