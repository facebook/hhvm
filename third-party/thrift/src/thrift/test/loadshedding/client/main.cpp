/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <atomic>
#include <chrono>
#include <thread>

#include <folly/init/Init.h>
#include <thrift/test/loadshedding/client/Client.h>

using apache::thrift::test::Client;

DEFINE_string(host, "::1", "Server host [default: '::1']");
DEFINE_int32(port, 7777, "Server port [default: 7777]");
DEFINE_uint32(rps, 100, "Request per second [default: 100]");
DEFINE_uint32(
    duration_s,
    60,
    "Number of microseconds a server will spend per request [default: 100ms]");

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);

  Client client("::1", FLAGS_port);

  std::atomic<bool> loggerEnabled{true};
  std::thread logger([&client, &loggerEnabled] {
    auto last = std::chrono::system_clock::now();
    while (loggerEnabled) {
      auto sleepTime = std::chrono::seconds(1);
      std::this_thread::sleep_for(sleepTime);
      auto now = std::chrono::system_clock::now();
      auto elapsed =
          std::chrono::duration_cast<std::chrono::seconds>(now - last);
      last = now;

      uint64_t count = client.getResponseCount();
      double rate =
          static_cast<double>(client.getSuccess()) / std::max(1UL, count);

      LOG(INFO) << "RPS: " << (client.getResponseCount() / elapsed.count())
                << ", success rate: " << (round(100 * rate) / 100)
                << ", success: " << client.getSuccess()
                << ", errors: " << client.getError()
                << ", Latency p50: " << (client.getLatencyPercentile(0.5))
                << ", p90: " << (client.getLatencyPercentile(0.9))
                << ", p99: " << (client.getLatencyPercentile(0.99));
      client.clearStats();
    }
  });

  auto duration = std::chrono::seconds(FLAGS_duration_s);
  LOG(INFO) << "Running load test at " << FLAGS_rps << " RPS for "
            << FLAGS_duration_s << "s";
  client.runSynchronously(FLAGS_rps, duration);

  loggerEnabled = false;
  logger.join();
  return 0;
}
