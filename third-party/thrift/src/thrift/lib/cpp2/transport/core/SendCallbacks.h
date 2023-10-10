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

#include <folly/ExceptionWrapper.h>
#include <folly/io/async/HHWheelTimer.h>
#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <wangle/acceptor/ManagedConnection.h>

/*
namespace wangle {

class ManagedConnection;

} // namespace wangle
*/

namespace apache::thrift {

class ResponseWriteTimeoutSendCallback : public MessageChannel::SendCallback {
 public:
  ResponseWriteTimeoutSendCallback() = delete;
  ResponseWriteTimeoutSendCallback(
      wangle::ManagedConnection& connection,
      std::chrono::milliseconds maxResponseWriteTime,
      MessageChannel::SendCallback* wrapped)
      : connection_{connection},
        timeout_{connection},
        maxResponseWriteTime_{maxResponseWriteTime},
        wrapped_{wrapped} {}

  void sendQueued() override;
  void messageSent() override;
  void messageSendError(folly::exception_wrapper&& e) override;

 private:
  class ResponseWriteTimeout : public folly::HHWheelTimer::Callback {
   public:
    ResponseWriteTimeout() = delete;
    explicit ResponseWriteTimeout(wangle::ManagedConnection& connection)
        : connection_{connection} {}

    void timeoutExpired() noexcept override;

   private:
    wangle::ManagedConnection& connection_;
  };

  wangle::ManagedConnection& connection_;
  ResponseWriteTimeout timeout_;
  const std::chrono::milliseconds maxResponseWriteTime_;
  MessageChannel::SendCallback* wrapped_;
};

} // namespace apache::thrift
