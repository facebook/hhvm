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

#include <glog/logging.h>
#include <thrift/lib/cpp2/transport/rocket/client/KeepAliveWatcher.h>

namespace apache::thrift::rocket {
KeepAliveWatcher::KeepAliveWatcher(
    folly::EventBase* evb,
    folly::AsyncTransport* socket,
    const std::chrono::milliseconds interval,
    const std::chrono::milliseconds timeout)
    : evb_(evb),
      socket_(socket),
      interval_(interval),
      timeout_(timeout),
      lastKeepAliveTs_(std::chrono::steady_clock::now()) {}

void KeepAliveWatcher::start(SetupFrame* setupFrame) {
  if (started_) {
    DCHECK(setupFrame == nullptr);
    return;
  }
  if (!evb_) {
    return;
  }
  started_ = true;
  lastKeepAliveTs_ = std::chrono::steady_clock::now();
  sendKeepAliveFrame(setupFrame);
  checkTimeoutToCloseOrSchedule();
}

void KeepAliveWatcher::stop() {
  if (std::exchange(started_, false)) {
    cancelTimeout();
  }
}

void KeepAliveWatcher::detachEventBase() {
  stop();
  evb_ = nullptr;
}

void KeepAliveWatcher::attachEventBase(folly::EventBase* evb) {
  if (evb_) {
    LOG(ERROR) << "Attempted attach without detaching first!";
  }
  evb_ = evb;
  start(nullptr);
}

void KeepAliveWatcher::handleKeepaliveFrame(
    std::unique_ptr<folly::IOBuf> frame) {
  KeepAliveFrame keepAliveFrame(std::move(frame));
  if (keepAliveFrame.hasRespondFlag()) {
    LOG(WARNING) << "Received Respond Flagged Keep Alive Frame, ignoring!";
    return;
  }
  lastKeepAliveTs_ = std::chrono::steady_clock::now();
}

void KeepAliveWatcher::sendKeepAliveFrame(SetupFrame* setupFrame) {
  DestructorGuard dg(this);
  evb_->dcheckIsInEventBaseThread();
  socket_->writeChain(this, makeKeepAliveFrame(setupFrame));
}

void KeepAliveWatcher::checkTimeoutToCloseOrSchedule() {
  DestructorGuard dg(this);
  evb_->dcheckIsInEventBaseThread();
  if ((std::chrono::steady_clock::now() - lastKeepAliveTs_) > timeout_) {
    FB_LOG_EVERY_MS(ERROR, 1000) << "Slow Connection Detected, closing socket.";
    socket_->closeNow();
  } else {
    // Reschedule timeout for next round.
    evb_->timer().scheduleTimeout(this, interval_);
  }
}

void KeepAliveWatcher::timeoutExpired() noexcept {
  sendKeepAliveFrame(nullptr);
  checkTimeoutToCloseOrSchedule();
}

std::unique_ptr<folly::IOBuf> KeepAliveWatcher::makeKeepAliveFrame(
    SetupFrame* setupFrame) {
  auto frame =
      KeepAliveFrame(Flags().respond(true), folly::IOBuf::copyBuffer(""))
          .serialize();
  if (UNLIKELY(setupFrame != nullptr)) {
    Serializer writer;
    std::move(*setupFrame).serialize(writer);
    auto setupBuffer = std::move(writer).move();
    setupBuffer->prependChain(std::move(frame));
    frame = std::move(setupBuffer);
  }
  return frame;
}

} // namespace apache::thrift::rocket
