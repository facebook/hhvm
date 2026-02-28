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

#include <thrift/test/loadshedding/client/Client.h>

#include <chrono>
#include <cmath>
#include <thread>

#include <gflags/gflags.h>
#include <folly/Random.h>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/test/loadshedding/if/gen-cpp2/BackendService.h>

DEFINE_uint32(
    time_per_request_us,
    100 * 1000,
    "Number of microseconds a server will spend per request [default: 100ms]");
DEFINE_bool(
    poisson,
    true,
    "Generated traffic follow a Poisson distribution, a fixed interval is used otherwise [default: true]");

namespace apache::thrift::test {

using namespace facebook::thrift::test;

using apache::thrift::HeaderClientChannel;

Client::Client(const std::string& addr, int port) {
  scopedEventBaseThread_.getEventBase()->runInEventBaseThreadAndWait(
      [this, &addr, port]() {
        auto eventBase = this->scopedEventBaseThread_.getEventBase();
        auto socket = folly::AsyncSocket::newSocket(eventBase, addr, port);
        auto channel = HeaderClientChannel::newChannel(std::move(socket));
        this->client_ =
            std::make_unique<BackendServiceAsyncClient>(std::move(channel));
      });
}

Client::~Client() {
  scopedEventBaseThread_.getEventBase()->runInEventBaseThreadAndWait(
      [this]() { this->client_.reset(); });
}

void Client::runSynchronously(
    double rps, std::chrono::duration<double> duration) {
  auto eventBase = scopedEventBaseThread_.getEventBase();
  auto deadline = std::chrono::steady_clock::now() + duration;

  while (std::chrono::steady_clock::now() < deadline) {
    eventBase->runInEventBaseThread([this]() {
      BackendRequest request;
      *request.time_per_request() = FLAGS_time_per_request_us;
      *request.consumeCPU() = false;

      auto sendTime = std::chrono::steady_clock::now();
      this->client_->future_doWork(request)
          .thenValue([sendTime, this](BackendResponse /* response */) {
            auto receiveTime = std::chrono::steady_clock::now();
            auto latency =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    receiveTime - sendTime);
            auto& stats = this->stats_;
            stats.addValue(latency);
            stats.responseCount++;
            stats.success++;
          })
          .thenError([this](folly::exception_wrapper&& /* ew */) {
            auto& stats = this->stats_;
            stats.responseCount++;
            stats.errors++;
          });
    });
    long targetDeltaTime = 1000 / rps;
    if (FLAGS_poisson) {
      // correct `targetDeltaTime` to follow a Poisson distribution
      targetDeltaTime *= -log(folly::Random::randDouble01());
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(targetDeltaTime));
  }
}

} // namespace apache::thrift::test
