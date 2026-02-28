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

#include <thrift/conformance/stresstest/server/StressTestServiceInterceptor.h>

namespace apache::thrift::stress {

std::string StressTestServiceInterceptor::getName() const {
  return "StressTestServiceInterceptor";
}

folly::coro::Task<std::optional<folly::Unit>>
StressTestServiceInterceptor::onRequest(ConnectionState*, RequestInfo) {
  co_return std::nullopt;
}

folly::coro::Task<void> StressTestServiceInterceptor::onResponse(
    RequestState*, ConnectionState*, ResponseInfo) {
  co_return;
}

std::optional<StressTestServiceInterceptor::ConnectionState>
StressTestServiceInterceptor::onConnectionEstablished(ConnectionInfo) {
  return std::nullopt;
}

void StressTestServiceInterceptor::onConnectionClosed(
    ConnectionState*, ConnectionInfo) noexcept {
  // Do nothing for now
}

} // namespace apache::thrift::stress
