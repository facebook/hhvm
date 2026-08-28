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

#include <thrift/conformance/stresstest/client/ClientRunner.h>
#include <thrift/conformance/stresstest/client/StressTestRegistry.h>
#include <thrift/conformance/stresstest/common/StressTestResultFile.h>
#include <thrift/conformance/stresstest/util/Util.h>

DEFINE_string(
    stress_test_result_path,
    "",
    "Write the final stress test result as Thrift SimpleJSON");
DEFINE_string(
    stress_test_result_run_id,
    "",
    "Run identifier stored in the stress test result");

namespace apache::thrift::stress {

namespace {

void writeClientResult(
    const std::string& testName, const StressTestStats& stats) {
  if (FLAGS_stress_test_result_path.empty()) {
    return;
  }

  ClientResult result;
  result.metadata()->runId() = FLAGS_stress_test_result_run_id;
  result.metadata()->workload() = testName;
  result.requests()->succeeded() = stats.rpcStats.numSuccess;
  result.requests()->failed() = stats.rpcStats.numFailure;
  result.requests()->total() =
      stats.rpcStats.numSuccess + stats.rpcStats.numFailure;
  writeStressTestResult(FLAGS_stress_test_result_path, result);
}

} // namespace

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
  if (!FLAGS_stress_test_result_path.empty()) {
    CHECK(!FLAGS_stress_test_result_run_id.empty())
        << "--stress_test_result_run_id is required with "
           "--stress_test_result_path";
    CHECK_EQ(selectedTests_.size(), 1)
        << "--stress_test_result_path requires exactly one selected test";
    CHECK(!cfg_.continuous)
        << "--stress_test_result_path is not supported with --continuous";
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
  return runTest(testName, instantiate(testName));
}

StressTestStats TestRunner::runTest(
    const std::string& testName, std::unique_ptr<StressTestBase> test) {
  resetMemoryStats();

  // initialize the client runner
  ClientRunner runner(cfg_);
  // run the test
  runner.run(test.get());
  // collect and print statistics
  auto result = StressTestStats{
      .runtimeSeconds = FLAGS_runtime_s,
      .memoryStats = runner.getMemoryStats(),
      .rpcStats = runner.getRpcStats(),
  };
  writeClientResult(testName, result);
  return result;
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
            .runtimeSeconds = FLAGS_runtime_s,
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
