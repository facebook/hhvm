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

#include <thrift/conformance/stresstest/server/StressTestHandler.h>

#include <folly/experimental/coro/AsyncGenerator.h>
#include <folly/experimental/coro/Sleep.h>
#include <folly/experimental/coro/Task.h>

namespace apache {
namespace thrift {
namespace stress {

StressTestHandler::StressTestHandler() {}

void StressTestHandler::async_eb_ping(
    std::unique_ptr<HandlerCallback<void>> callback) {
  callback->done();
}

void StressTestHandler::async_tm_requestResponseTm(
    std::unique_ptr<HandlerCallback<std::unique_ptr<BasicResponse>>> callback,
    std::unique_ptr<BasicRequest> request) {
  if (request->processInfo()->processingMode() == ProcessingMode::Async) {
    auto* tm = callback->getThreadManager();
    tm->add([this,
             callback = std::move(callback),
             request = std::move(request)]() mutable {
      requestResponseImpl(std::move(callback), std::move(request));
    });
    return;
  }
  requestResponseImpl(std::move(callback), std::move(request));
}

void StressTestHandler::async_eb_requestResponseEb(
    std::unique_ptr<HandlerCallback<std::unique_ptr<BasicResponse>>> callback,
    std::unique_ptr<BasicRequest> request) {
  if (request->processInfo()->processingMode() == ProcessingMode::Async) {
    auto* evb = callback->getEventBase();
    evb->add([this,
              callback = std::move(callback),
              request = std::move(request)]() mutable {
      requestResponseImpl(std::move(callback), std::move(request));
    });
    return;
  }
  requestResponseImpl(std::move(callback), std::move(request));
}

ResponseAndServerStream<BasicResponse, BasicResponse>
StressTestHandler::streamTm(std::unique_ptr<StreamRequest> request) {
  simulateWork(
      *request->processInfo()->initialResponseProcessingTimeMs(),
      *request->processInfo()->serverWorkSimulationMode());
  auto response =
      makeBasicResponse(*request->processInfo()->initialResponseSize());
  auto stream = folly::coro::co_invoke(
      [this, request = std::move(request)]() mutable
      -> folly::coro::AsyncGenerator<BasicResponse&&> {
        auto numChunks = request->processInfo()->numChunks();
        for (int64_t i = 0; i < numChunks; i++) {
          co_await co_simulateWork(
              *request->processInfo()->serverChunkProcessingTimeMs(),
              *request->processInfo()->serverWorkSimulationMode());
          co_yield makeBasicResponse(*request->processInfo()->chunkSize());
        }
      });
  return {std::move(response), std::move(stream)};
}

ResponseAndSinkConsumer<BasicResponse, BasicResponse, BasicResponse>
StressTestHandler::sinkTm(std::unique_ptr<StreamRequest> request) {
  simulateWork(
      *request->processInfo()->initialResponseProcessingTimeMs(),
      *request->processInfo()->serverWorkSimulationMode());
  auto response =
      makeBasicResponse(*request->processInfo()->initialResponseSize());
  auto consumer = SinkConsumer<BasicResponse, BasicResponse>{
      [this, request = std::move(request)](
          folly::coro::AsyncGenerator<BasicResponse&&> gen)
          -> folly::coro::Task<BasicResponse> {
        while (co_await gen.next()) {
          co_await co_simulateWork(
              *request->processInfo()->serverChunkProcessingTimeMs(),
              *request->processInfo()->serverWorkSimulationMode());
        }
        co_await co_simulateWork(
            *request->processInfo()->finalResponseProcessingTimeMs(),
            *request->processInfo()->serverWorkSimulationMode());
        co_return makeBasicResponse(
            *request->processInfo()->finalResponseSize());
      },
      10 /* TODO: make buffer size a parameter */};
  return {std::move(response), std::move(consumer)};
}

void StressTestHandler::requestResponseImpl(
    std::unique_ptr<HandlerCallback<std::unique_ptr<BasicResponse>>> callback,
    std::unique_ptr<BasicRequest> request) const {
  simulateWork(
      *request->processInfo()->processingTimeMs(),
      *request->processInfo()->workSimulationMode());
  callback->result(makeBasicResponse(*request->processInfo()->responseSize()));
}

void StressTestHandler::simulateWork(
    int64_t timeMs, WorkSimulationMode mode) const {
  if (timeMs > 0) {
    auto duration = std::chrono::milliseconds(timeMs);
    switch (mode) {
      case WorkSimulationMode::Default: {
        busyWait(duration);
        break;
      }
      case WorkSimulationMode::Sleep: {
        std::this_thread::sleep_for(duration);
        break;
      }
    }
  }
}

folly::coro::Task<void> StressTestHandler::co_simulateWork(
    int64_t timeMs, WorkSimulationMode mode) const {
  if (timeMs > 0) {
    auto duration = std::chrono::milliseconds(timeMs);
    switch (mode) {
      case WorkSimulationMode::Default: {
        busyWait(duration);
        break;
      }
      case WorkSimulationMode::Sleep: {
        co_await folly::coro::sleep(duration);
        break;
      }
    }
  }
}

void StressTestHandler::busyWait(std::chrono::milliseconds duration) const {
  auto deadline = std::chrono::steady_clock::now() + duration;
  while (std::chrono::steady_clock::now() < deadline) {
    // wait for deadline
  }
}

BasicResponse StressTestHandler::makeBasicResponse(int64_t payloadSize) const {
  BasicResponse chunk;
  if (payloadSize > 0) {
    chunk.payload() = std::string('x', payloadSize);
  }
  return chunk;
}

} // namespace stress
} // namespace thrift
} // namespace apache
