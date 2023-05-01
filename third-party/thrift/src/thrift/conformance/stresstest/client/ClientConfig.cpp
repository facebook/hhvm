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

#include <thrift/conformance/stresstest/client/ClientConfig.h>

#include <thread>
#include <glog/logging.h>

DEFINE_string(server_host, "::1", "Host running the stress test server");
DEFINE_int32(server_port, 5000, "Port of the stress test server");
DEFINE_string(test_name, "", "Stress test to run");
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
DEFINE_bool(continuous, false, "Runs a single test continuously");
DEFINE_int64(
    stats_interval,
    10,
    "How often to poll in secs stats when running continuously");
DEFINE_bool(io_uring, false, "Flag to enable io_uring on the client");
DEFINE_int64(runtime_s, 10, "Runtime of test in seconds");
DEFINE_int64(warmup_s, 2, "Warmup time of test in seconds");
DEFINE_uint32(target_qps, 1000, "Target QPS for generated load");
DEFINE_bool(
    gen_load, false, "Generate constant QPS load instead of using concurrency");
DEFINE_uint64(
    gen_load_interval,
    5,
    "interval in milliseconds used by the generated load");

namespace apache::thrift::stress {

/* static */ ClientConfig ClientConfig::createFromFlags() {
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
  connCfg.serverHost = folly::SocketAddress(
      FLAGS_server_host, FLAGS_server_port, /* allowNameLookup */ true);
  connCfg.security = security;
  connCfg.certPath = FLAGS_client_cert_path;
  connCfg.keyPath = FLAGS_client_key_path;
  connCfg.trustedCertsPath = FLAGS_client_ca_path;
  connCfg.ioUring = FLAGS_io_uring;

  ClientConfig config{};

  // Configure qps load generator
  config.useLoadGenerator = FLAGS_gen_load;
  config.gen_load_interval = std::chrono::milliseconds(FLAGS_gen_load_interval);
  config.targetQps = FLAGS_target_qps;

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

} // namespace apache::thrift::stress
