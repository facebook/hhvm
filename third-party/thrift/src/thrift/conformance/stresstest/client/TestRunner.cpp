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

#include <thrift/conformance/stresstest/client/TestRunner.h>

#include <fmt/core.h>
#include <gflags/gflags.h>

#include <thrift/conformance/stresstest/client/StressTestRegistry.h>
#include <thrift/conformance/stresstest/util/Util.h>

namespace apache::thrift::stress {

void StressTestStats::log() const {
  LOG(INFO) << fmt::format(
      "Total requests:        {} ({} succeeded, {} failed)",
      (rpcStats.numSuccess + rpcStats.numFailure),
      rpcStats.numSuccess,
      rpcStats.numFailure);
  LOG(INFO) << fmt::format(
      "Average QPS:           {:.2f}",
      (static_cast<double>(rpcStats.numSuccess) / FLAGS_runtime_s));
  LOG(INFO) << fmt::format(
      "Request latency:       P50={:.2f}us, P99={:.2f}us, P100={:.2f}us",
      rpcStats.latencyHistogram.getPercentileEstimate(.5),
      rpcStats.latencyHistogram.getPercentileEstimate(.99),
      rpcStats.latencyHistogram.getPercentileEstimate(1.0));
  LOG(INFO) << "Allocated memory stats:";
  LOG(INFO) << fmt::format(
      "  Before test:         {} bytes", memoryStats.threadStart);
  LOG(INFO) << fmt::format(
      "  Clients connected:   {} bytes", memoryStats.connectionsEstablished);
  LOG(INFO) << fmt::format(
      "  During test:         P50={} bytes, P99={} bytes, P100={} bytes",
      memoryStats.p50,
      memoryStats.p99,
      memoryStats.p100);
  LOG(INFO) << fmt::format(
      "  Clients idle:        {} bytes", memoryStats.connectionsIdle);
}

TestRunner::TestRunner(ClientConfig cfg)
    : cfg_(cfg), availableTests_(StressTestRegistry::getInstance().listAll()) {
  if (!FLAGS_test_name.empty()) {
    if (!isRegistered(FLAGS_test_name)) {
      LOG(FATAL) << fmt::format(
          "Selected test '{}' does not exist", FLAGS_test_name);
    }
    selectedTests_.push_back(FLAGS_test_name);
  } else {
    selectedTests_ = availableTests_;
  }
}

const std::vector<std::string>& TestRunner::getSelectedTests() const {
  return selectedTests_;
}

const std::vector<std::string>& TestRunner::getAvailableTests() const {
  return availableTests_;
}

bool TestRunner::isRegistered(std::string testName) const {
  return std::find(availableTests_.begin(), availableTests_.end(), testName) !=
      availableTests_.end();
}

std::unique_ptr<StressTestBase> TestRunner::instantiate(
    std::string testName) const {
  auto ret = StressTestRegistry::getInstance().create(testName);
  if (!ret) {
    LOG(FATAL) << fmt::format("Failed to instantiate test '{}'", testName);
  }
  return ret;
}

StressTestStats TestRunner::run(std::string testName) {
  return run(instantiate(testName));
}

StressTestStats TestRunner::run(std::unique_ptr<StressTestBase> test) {
  resetMemoryStats();

  // initialize the client runner
  ClientRunner runner(cfg_);
  // run the test
  runner.run(test.get());
  // collect and print statistics
  return StressTestStats{
      .memoryStats = runner.getMemoryStats(),
      .rpcStats = runner.getRpcStats(),
  };
}

void TestRunner::runTests() {
  LOG(INFO) << "Using io_uring: " << (FLAGS_io_uring ? "true" : "false");
  if (cfg_.continuous) {
    runContinuously();
  } else if (cfg_.numRunsPerClient > 0) {
    runFixedCount();
  } else {
    runFixedTime();
  }
}

void TestRunner::runContinuously() {
  LOG(INFO) << "Starting Continuous Benchmark";
  auto& testName = getSelectedTests().front();
  LOG(INFO) << fmt::format("Running stress test '{}'", testName);
  runContinuously(instantiate(testName));
}

void TestRunner::runContinuously(std::unique_ptr<StressTestBase> test) {
  resetMemoryStats();

  // initialize the client runner
  ClientRunner runner(cfg_);

  scheduleContinuousStats(runner);

  // run the test
  runner.run(test.get());
}

void TestRunner::scheduleContinuousStats(ClientRunner& runner) {
  LOG(INFO) << "scheduling stats poller every " << FLAGS_runtime_s
            << " seconds";
  functionScheduler_.addFunction(
      [&] {
        LOG(INFO) << "\nStress Test Stats:";
        auto stats = StressTestStats{
            .memoryStats = runner.getMemoryStats(),
            .rpcStats = runner.getRpcStats(),
        };
        stats.log();

        runner.resetStats();
      },
      std::chrono::seconds(FLAGS_runtime_s),
      "stats",
      std::chrono::seconds(FLAGS_runtime_s));
  functionScheduler_.start();
}

void TestRunner::runFixedTime() {
  LOG(INFO) << fmt::format("Starting Fixed Duration Benchmark");
  for (const auto& test : getSelectedTests()) {
    LOG(INFO) << fmt::format("Running stress test '{}'", test);
    auto result = run(test);
    result.log();
  }
}

void TestRunner::runFixedCount() {
  LOG(INFO) << "Starting Fixed Count Benchmark";
  for (const auto& test : getSelectedTests()) {
    LOG(INFO) << fmt::format("Running stress test '{}'", test);
    auto result = run(test);
    result.log();
  }
}

} // namespace apache::thrift::stress
