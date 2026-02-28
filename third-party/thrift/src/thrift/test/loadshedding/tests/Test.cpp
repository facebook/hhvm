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

#include <thread>

#include <folly/init/Init.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>
#include <thrift/test/loadshedding/client/Client.h>
#include <thrift/test/loadshedding/server/BackendServiceHandler.h>

using namespace apache::thrift;
using namespace facebook::thrift::test;
using namespace std::chrono;

using apache::thrift::util::ScopedServerThread;

DEFINE_uint32(rps, 100, "Request per second [default: 100]");

int main(int argc, char* argv[]) {
  const folly::Init init(&argc, &argv);

  auto handler = std::make_shared<BackendServiceHandler>();
  auto server = std::make_shared<ThriftServer>();
  server->setPort(0);
  server->setInterface(handler);

  ScopedServerThread thread;
  thread.start(server);

  const auto addr = server->getAddress();
  apache::thrift::test::Client client("::1", addr.getPort());

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
      double rate = static_cast<double>(client.getSuccess()) /
          std::max<uint64_t>(1, count);

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

  // Start the load test and increase the RPS regularly
  LOG(INFO) << "Starting the integration test by increment of " << FLAGS_rps
            << " RPS";
  for (auto i = 1; i < 5; i++) {
    auto rps = FLAGS_rps * i;
    LOG(INFO) << "Running integration test at " << rps << " RPS";
    client.runSynchronously(rps, std::chrono::seconds(30));
  }

  loggerEnabled = false;
  logger.join();

  return 0;
}
