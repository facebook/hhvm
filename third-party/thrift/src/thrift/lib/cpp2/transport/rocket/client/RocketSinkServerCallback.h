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

#include <memory>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/HHWheelTimer.h>
#include <folly/io/async/Request.h>

#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {
class RocketClient;

class RocketSinkServerCallback : public SinkServerCallback {
 public:
  RocketSinkServerCallback(
      StreamId streamId,
      RocketClient& client,
      SinkClientCallback& clientCallback,
      std::unique_ptr<CompressionConfig> compressionConfig)
      : client_(client),
        clientCallback_(&clientCallback),
        streamId_(streamId),
        compressionConfig_(std::move(compressionConfig)) {}

  bool onSinkNext(StreamPayload&&) override;
  void onSinkError(folly::exception_wrapper) override;
  bool onSinkComplete() override;

  void resetClientCallback(SinkClientCallback& clientCallback) override {
    clientCallback_ = &clientCallback;
  }

  FOLLY_NODISCARD bool onInitialPayload(
      FirstResponsePayload&&, folly::EventBase*);
  void onInitialError(folly::exception_wrapper);

  void onFinalResponse(StreamPayload&&);
  void onFinalResponseError(folly::exception_wrapper);

  void onSinkRequestN(uint64_t tokens);

  StreamId streamId() const noexcept { return streamId_; }

 private:
  RocketClient& client_;
  SinkClientCallback* clientCallback_;
  StreamId streamId_;
  enum class State { BothOpen, StreamOpen };
  State state_{State::BothOpen};
  std::unique_ptr<CompressionConfig> compressionConfig_;
};

} // namespace apache::thrift::rocket
