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
#include <utility>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventHandler.h>
#include <folly/io/async/HHWheelTimer.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

namespace apache::thrift::rocket {

class KeepAliveWatcher : public folly::HHWheelTimer::Callback,
                         public virtual folly::DelayedDestruction,
                         private folly::AsyncTransport::WriteCallback {
 public:
  KeepAliveWatcher(
      folly::EventBase* evb,
      folly::AsyncTransport* socket,
      const std::chrono::milliseconds interval,
      const std::chrono::milliseconds timeout);

  KeepAliveWatcher(const KeepAliveWatcher&) = delete;
  KeepAliveWatcher(KeepAliveWatcher&&) = delete;
  KeepAliveWatcher& operator=(const KeepAliveWatcher&) = delete;
  KeepAliveWatcher& operator=(KeepAliveWatcher&&) = delete;

  void start(SetupFrame* setupFrame = nullptr);

  void stop();

  void detachEventBase();

  void attachEventBase(folly::EventBase* evb);

  void handleKeepaliveFrame(std::unique_ptr<folly::IOBuf> frame);

  void timeoutExpired() noexcept override;

  void writeSuccess() noexcept override {}

  void writeErr(size_t, const folly::AsyncSocketException&) noexcept override {
    stop();
  }

 private:
  ~KeepAliveWatcher() override { stop(); }

  void sendKeepAliveFrame(SetupFrame* setupFrame);

  void checkTimeoutToCloseOrSchedule();

  std::unique_ptr<folly::IOBuf> makeKeepAliveFrame(
      SetupFrame* setupFrame = nullptr);

  bool started_{false};
  folly::EventBase* evb_;
  folly::AsyncTransport* socket_;
  std::chrono::milliseconds interval_;
  std::chrono::milliseconds timeout_;
  std::chrono::time_point<std::chrono::steady_clock> lastKeepAliveTs_;
};
} // namespace apache::thrift::rocket
