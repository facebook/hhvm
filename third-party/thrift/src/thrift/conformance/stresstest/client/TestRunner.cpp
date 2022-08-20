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
#include <folly/portability/GFlags.h>

#include <thrift/conformance/stresstest/client/StressTestRegistry.h>
#include <thrift/conformance/stresstest/util/Util.h>

DEFINE_string(server_host, "::1", "Host running the stress test server");
DEFINE_int32(server_port, 5000, "Port of the stress test server");
DEFINE_string(test_name, "", "Stress test to run");
DEFINE_int64(runtime_s, 10, "Runtime of test in seconds");
DEFINE_int64(client_threads, 1, "Nnumber of client threads");
DEFINE_int64(connections_per_thread, 1, "Number of clients per client thread");
DEFINE_int64(clients_per_connection, 1, "Number of clients per connection");
DEFINE_string(
    client_security,
    "",
    "Set to 'FIZZ' or 'TLS' to enable security (requires certPath, keyPath and caPath to be specified)");
DEFINE_string(
    client_cert_path,
    "folly/io/async/test/certs/tests-cert.pem",
    "Path to client certificate file");
DEFINE_string(
    client_key_path,
    "folly/io/async/test/certs/tests-key.pem",
    "Path to client key file");
DEFINE_string(
    client_ca_path,
    "folly/io/async/test/certs/ca-cert.pem",
    "Path to client trusted CA file");

namespace apache {
namespace thrift {
namespace stress {

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

  // run the test and sleep for the duration
  runner.run(test.get());
  std::this_thread::sleep_for(std::chrono::seconds(FLAGS_runtime_s));
  runner.stop();

  // collect and print statistics
  return StressTestStats{
      .memoryStats = runner.getMemoryStats(),
      .rpcStats = runner.getRpcStats(),
  };
}

/* static */ ClientConfig TestRunner::createClientConfigFromFlags() {
  ClientSecurity security;
  if (FLAGS_client_security.empty()) {
    security = ClientSecurity::None;
  } else {
    if (FLAGS_client_security == "TLS") {
      security = ClientSecurity::TLS;
    } else if (FLAGS_client_security == "FIZZ") {
      security = ClientSecurity::FIZZ;
    } else {
      LOG(FATAL) << fmt::format(
          "Unrecognized option for security '{}'", FLAGS_client_security);
    }
  }

  ClientConnectionConfig connCfg{};
  connCfg.serverHost =
      folly::SocketAddress(FLAGS_server_host, FLAGS_server_port);
  connCfg.security = security;
  connCfg.certPath = FLAGS_client_cert_path;
  connCfg.keyPath = FLAGS_client_key_path;
  connCfg.trustedCertsPath = FLAGS_client_ca_path;

  ClientConfig config{};
  config.numClientThreads = FLAGS_client_threads <= 0
      ? std::thread::hardware_concurrency()
      : static_cast<uint64_t>(FLAGS_client_threads);
  config.numConnectionsPerThread =
      static_cast<uint64_t>(FLAGS_connections_per_thread);
  config.numClientsPerConnection =
      static_cast<uint64_t>(FLAGS_clients_per_connection);
  config.connConfig = std::move(connCfg);

  return config;
}

} // namespace stress
} // namespace thrift
} // namespace apache
