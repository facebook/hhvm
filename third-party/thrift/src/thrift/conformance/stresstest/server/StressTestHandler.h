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

#include <thrift/conformance/stresstest/if/gen-cpp2/StressTest.h>

namespace apache {
namespace thrift {
namespace stress {

class StressTestHandler : public apache::thrift::ServiceHandler<StressTest> {
 public:
  StressTestHandler();

  void async_eb_ping(std::unique_ptr<HandlerCallback<void>> callback) override;

  void async_tm_echo(
      std::unique_ptr<HandlerCallback<std::unique_ptr<std::string>>> callback,
      std::unique_ptr<::std::string> payload) override {
    callback->result(std::move(payload));
  }

  void async_eb_echoEb(
      std::unique_ptr<HandlerCallback<std::unique_ptr<std::string>>> callback,
      std::unique_ptr<::std::string> payload) override {
    callback->result(std::move(payload));
  }

  void async_tm_requestResponseTm(
      std::unique_ptr<HandlerCallback<std::unique_ptr<BasicResponse>>> callback,
      std::unique_ptr<BasicRequest> request) override;

  void async_eb_requestResponseEb(
      std::unique_ptr<HandlerCallback<std::unique_ptr<BasicResponse>>> callback,
      std::unique_ptr<BasicRequest> request) override;

  ResponseAndServerStream<BasicResponse, BasicResponse> streamTm(
      std::unique_ptr<StreamRequest> request) override;

  ResponseAndSinkConsumer<BasicResponse, BasicResponse, BasicResponse> sinkTm(
      std::unique_ptr<StreamRequest> request) override;

 private:
  void requestResponseImpl(
      std::unique_ptr<HandlerCallback<std::unique_ptr<BasicResponse>>> callback,
      std::unique_ptr<BasicRequest> request) const;

  void simulateWork(int64_t timeMs, WorkSimulationMode mode) const;

  folly::coro::Task<void> co_simulateWork(
      int64_t timeMs, WorkSimulationMode mode) const;

  void busyWait(std::chrono::milliseconds duration) const;

  BasicResponse makeBasicResponse(int64_t payloadSize) const;
};

} // namespace stress
} // namespace thrift
} // namespace apache
