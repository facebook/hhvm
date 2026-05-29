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

#include <thrift/lib/cpp2/fast_thrift/bench/runner/BenchmarkRegistry.h>
#include <thrift/lib/cpp2/fast_thrift/bench/runner/ClientFactory.h>
#include <thrift/lib/cpp2/fast_thrift/bench/runner/ClientRunner.h>

#include <iostream>
#include <unordered_set>

#include <gflags/gflags.h>
#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <folly/system/HardwareConcurrency.h>

DEFINE_string(host, "::1", "Server host (IPv4 or IPv6 address)");
DEFINE_int32(port, 0, "Server port");
DEFINE_int32(runtime_s, 30, "Runtime per test (seconds)");
DEFINE_int32(
    client_threads,
    1,
    "Number of concurrent client threads (0 = hardware concurrency)");
DEFINE_string(
    test_name,
    "",
    "Comma-separated names of tests to run (empty = run all tests)");
DEFINE_string(test_type, "tcp", "Type of client to test (tcp)");
DEFINE_uint64(
    zero_copy_threshold,
    0,
    "Minimum payload size in bytes for MSG_ZEROCOPY (0 = disabled)");

namespace apache::thrift::fast_thrift::bench {

std::unordered_set<std::string> parseTestNames(const std::string& testNames) {
  std::unordered_set<std::string> selectedTests;
  std::string remaining = testNames;
  size_t pos = 0;

  while ((pos = remaining.find(',')) != std::string::npos) {
    std::string token = remaining.substr(0, pos);
    token.erase(0, token.find_first_not_of(" \t"));
    token.erase(token.find_last_not_of(" \t") + 1);
    if (!token.empty()) {
      selectedTests.insert(token);
    }
    remaining.erase(0, pos + 1);
  }

  remaining.erase(0, remaining.find_first_not_of(" \t"));
  remaining.erase(remaining.find_last_not_of(" \t") + 1);
  if (!remaining.empty()) {
    selectedTests.insert(remaining);
  }

  return selectedTests;
}

size_t resolveClientThreads(int32_t flagValue) {
  if (flagValue < 0) {
    std::cerr << "Error: --client_threads must be >= 0" << std::endl;
    return 0;
  }
  if (flagValue == 0) {
    auto hwConcurrency = folly::available_concurrency();
    return hwConcurrency > 0 ? hwConcurrency : 1;
  }
  return static_cast<size_t>(flagValue);
}

int runClient() {
  if (FLAGS_port == 0) {
    std::cerr << "Error: --port is required" << std::endl;
    return 1;
  }

  size_t numThreads = resolveClientThreads(FLAGS_client_threads);
  if (numThreads == 0) {
    return 1;
  }

  auto selectedTests = parseTestNames(FLAGS_test_name);

  folly::SocketAddress serverAddress(FLAGS_host, FLAGS_port);
  std::cout << "Server: " << serverAddress.describe() << std::endl;
  std::cout << "Threads: " << numThreads << std::endl;
  std::cout << "Test Type: " << FLAGS_test_type << std::endl;
  std::cout << "Runtime: " << FLAGS_runtime_s << "s per test\n" << std::endl;

  ClientFactory factory(
      FLAGS_test_type, static_cast<size_t>(FLAGS_zero_copy_threshold));

  for (const auto& bench : BenchmarkRegistry::instance().benchmarks()) {
    if (!selectedTests.empty() &&
        selectedTests.find(bench.name) == selectedTests.end()) {
      continue;
    }

    ClientRunner runner(
        serverAddress, numThreads, bench, FLAGS_runtime_s, factory);

    runner.run();
    std::cout << std::endl;
  }

  return 0;
}

} // namespace apache::thrift::fast_thrift::bench

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  return apache::thrift::fast_thrift::bench::runClient();
}
