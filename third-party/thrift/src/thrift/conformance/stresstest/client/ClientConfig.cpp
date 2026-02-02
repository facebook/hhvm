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

#include <thrift/conformance/stresstest/util/IoUringUtil.h>

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
DEFINE_bool(stopTLSv1, false, "Enable stopTLS v1");
DEFINE_bool(stopTLSv2, false, "Enable stopTLS v2");
DEFINE_bool(continuous, false, "Runs a single test continuously");
DEFINE_int64(runs_per_client, 0, "Runs a fixed number of workloads");
DEFINE_int64(
    stats_interval,
    10,
    "How often to poll in secs stats when running continuously");
DEFINE_bool(io_uring, false, "Flag to enable io_uring on the client");
DEFINE_bool(
    src_port_bind,
    false,
    "Enable src port calculation and binding prior to connection. Default is false. (true, false)");
DEFINE_bool(quic, false, "Flag to enable quic on the client");
DEFINE_int64(runtime_s, 10, "Runtime of test in seconds");
DEFINE_int64(warmup_s, 2, "Warmup time of test in seconds");
DEFINE_uint32(target_qps, 1000, "Target QPS for generated load");
DEFINE_bool(
    gen_load, false, "Generate constant QPS load instead of using concurrency");
DEFINE_uint64(
    gen_load_interval,
    5,
    "interval in milliseconds used by the generated load");
DEFINE_bool(enable_checksum, false, "Enable checksum validation using XXH3");
DEFINE_string(
    compression_config, "", "Compression config to use (ZSTD, ZLIB, LZ4)");
DEFINE_string(
    compression_level,
    "",
    "Compression level value to use. (DEFAULT, LESS, MORE)");
DEFINE_string(
    thrift_protocol,
    "COMPACT",
    "Thrift serialization protocol. Default is COMPACT. (COMPACT, BINARY)");
DEFINE_bool(
    enable_rocket_frame_relative_alignment,
    false,
    "Enable frame relative alignment. Default is false. (true, false)");

namespace apache::thrift::stress {

namespace detail {

template <typename Config>
Config createConfigFromFlags() {
  Config config;
  if (FLAGS_compression_level.empty()) {
    return config;
  }

  std::remove_reference_t<decltype(Config().levelPreset().value())> levelPreset;
  if (TEnumTraits<
          std::remove_reference_t<decltype(Config().levelPreset().value())>>()
          .findValue(FLAGS_compression_level, &levelPreset)) {
    config.levelPreset_ref() = levelPreset;
    return config;
  } else {
    LOG(FATAL) << fmt::format(
        "Unrecognized option for compression_level '{}'",
        FLAGS_compression_level);
  }
}

std::optional<CompressionConfig> createCompressionConfigFromFlags() {
  if (FLAGS_compression_config.empty()) {
    return std::nullopt;
  }

  CodecConfig codecConfig;
  if (FLAGS_compression_config == "ZSTD") {
    codecConfig.set_zstdConfig(
        detail::createConfigFromFlags<ZstdCompressionCodecConfig>());
  } else if (FLAGS_compression_config == "ZLIB") {
    codecConfig.set_zlibConfig(
        detail::createConfigFromFlags<ZlibCompressionCodecConfig>());
  } else if (FLAGS_compression_config == "LZ4") {
    codecConfig.set_lz4Config(
        detail::createConfigFromFlags<Lz4CompressionCodecConfig>());
  } else {
    LOG(FATAL) << fmt::format(
        "Unrecognized option for compression_config '{}'",
        FLAGS_compression_config);
  }

  CompressionConfig compressionConfig;
  compressionConfig.codecConfig() = codecConfig;

  return compressionConfig;
}

protocol::PROTOCOL_TYPES createThriftProtocolFromFlags() {
  if (FLAGS_thrift_protocol == "COMPACT") {
    return protocol::T_COMPACT_PROTOCOL;
  } else if (FLAGS_thrift_protocol == "BINARY") {
    return protocol::T_BINARY_PROTOCOL;
  } else {
    LOG(FATAL) << fmt::format(
        "Unrecognized option for thrift_protocol '{}'", FLAGS_thrift_protocol);
  }
}

} // namespace detail

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
  connCfg.compressionConfigOpt = detail::createCompressionConfigFromFlags();
  connCfg.certPath = FLAGS_client_cert_path;
  connCfg.keyPath = FLAGS_client_key_path;
  connCfg.trustedCertsPath = FLAGS_client_ca_path;
  connCfg.ioUring = FLAGS_io_uring;
  connCfg.ioUringZcrx = FLAGS_io_zcrx;
  connCfg.srcPortBind = FLAGS_src_port_bind;
  connCfg.useQuic = FLAGS_quic;
  connCfg.stopTLSv1 = FLAGS_stopTLSv1 && (security == ClientSecurity::FIZZ);
  connCfg.stopTLSv2 = FLAGS_stopTLSv2 && (security == ClientSecurity::FIZZ);
  connCfg.thriftProtocol = detail::createThriftProtocolFromFlags();

  ClientConfig config{};

  config.continuous = FLAGS_continuous;
  config.numRunsPerClient = FLAGS_runs_per_client;
  if (config.continuous && config.numRunsPerClient > 0) {
    LOG(FATAL) << "Cannot specify both --continuous and --runs_per_client";
  }
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

  config.enableChecksum = FLAGS_enable_checksum;

  config.enableRocketFrameRelativeAlignment =
      FLAGS_enable_rocket_frame_relative_alignment;

  return config;
}

} // namespace apache::thrift::stress
