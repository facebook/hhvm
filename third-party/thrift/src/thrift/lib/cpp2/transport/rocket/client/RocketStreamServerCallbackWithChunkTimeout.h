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
#include <cstdint>
#include <memory>

#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketStreamServerCallback.h>

namespace apache::thrift::rocket {
class RocketClient;

class RocketStreamServerCallbackWithChunkTimeout
    : public RocketStreamServerCallback {
 public:
  RocketStreamServerCallbackWithChunkTimeout(
      StreamId streamId,
      RocketClient& client,
      StreamClientCallback& clientCallback,
      std::chrono::milliseconds chunkTimeout,
      uint64_t initialCredits)
      : RocketStreamServerCallback(streamId, client, clientCallback),
        chunkTimeout_(chunkTimeout),
        credits_(initialCredits) {}

  bool onStreamRequestN(uint64_t tokens) override;

  bool onInitialPayload(FirstResponsePayload&&, folly::EventBase*);

  FOLLY_NODISCARD bool onStreamPayload(StreamPayload&&);

  void timeoutExpired() noexcept;

 private:
  void scheduleTimeout();
  void cancelTimeout();

  const std::chrono::milliseconds chunkTimeout_;
  uint64_t credits_{0};
  std::unique_ptr<folly::HHWheelTimer::Callback> timeout_;
};

} // namespace apache::thrift::rocket
