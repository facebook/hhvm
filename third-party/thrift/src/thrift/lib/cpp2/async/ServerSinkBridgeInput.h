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

#include <thrift/lib/cpp2/async/ServerSinkBridge.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift::detail {

template <typename T>
folly::coro::AsyncGenerator<T&&> ServerSinkBridge::getInput(
    ServerSinkBridge::Ptr bridge, SinkElementDecoder<T>* decode) {
  bridge->notifySinkSubscribe();

  const auto effectiveThreshold = [&] {
    auto t = bridge->consumer_.bufferReplenishThreshold;
    if (t == 0) {
      return bridge->consumer_.bufferSize / 2;
    }
    CHECK_GT(t, 0) << "bufferReplenishThreshold must not be negative";
    CHECK_LT(t, bridge->consumer_.bufferSize)
        << "bufferReplenishThreshold (" << t
        << ") must be strictly less than bufferSize ("
        << bridge->consumer_.bufferSize << ")";
    return t;
  }();

  uint64_t counter = 0;
  while (true) {
    CoroConsumer consumer;
    if (bridge->serverWait(&consumer)) {
      folly::CancellationCallback cb{
          co_await folly::coro::co_current_cancellation_token, [&]() {
            bridge->notifySinkCancel();
            bridge->serverClose();
          }};
      co_await consumer.wait();
    }
    co_await folly::coro::co_safe_point;
    for (auto messages = bridge->serverGetMessages(); !messages.empty();
         messages.pop()) {
      auto& message = messages.front();

      if (std::holds_alternative<StreamMessage::Complete>(message)) {
        co_return;
      }

      auto& payload =
          std::get<StreamMessage::PayloadOrError>(message).streamPayloadTry;

      // INV-1: set clientException_ BEFORE co_yield, because the generator
      // may be destroyed without the body ever resuming.
      // INV-2: exception path must NOT reach notifySinkConsumed().
      if (payload.hasException()) {
        bridge->clientException_ = true;
        co_yield folly::coro::co_result((*decode)(std::move(payload)));
        co_return;
      }

      // INV-9: decode failures here must NOT set clientException_. If
      // decode() throws, the exception propagates as a server error.
      co_yield folly::coro::co_result((*decode)(std::move(payload)));
      // INV-3: credit accounting after yield-resume
      bridge->notifySinkConsumed();
      counter++;
      if (counter > effectiveThreshold) {
        bridge->serverPush(
            StreamMessage::RequestN{static_cast<int32_t>(counter)});
        counter = 0;
      }
    }
  }
}

} // namespace apache::thrift::detail

#endif
