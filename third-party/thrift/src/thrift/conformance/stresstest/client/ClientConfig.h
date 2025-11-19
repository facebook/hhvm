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

#pragma once

#include <chrono>
#include <gflags/gflags.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

DECLARE_string(server_host);
DECLARE_int32(server_port);
DECLARE_string(test_name);
DECLARE_int64(client_threads);
DECLARE_int64(connections_per_thread);
DECLARE_int64(clients_per_connection);
DECLARE_string(client_security);
DECLARE_string(client_cert_path);
DECLARE_string(client_key_path);
DECLARE_string(client_ca_path);
DECLARE_bool(continuous);
DECLARE_int64(runs_per_client);
DECLARE_int64(stats_interval);
DECLARE_bool(io_uring);
DECLARE_int64(runtime_s);
DECLARE_int64(warmup_s);
DECLARE_uint32(target_qps);
DECLARE_bool(gen_load);
DECLARE_uint64(gen_load_interval);
DECLARE_string(compression_config);
DECLARE_string(compression_level);
DECLARE_string(thrift_protocol);
DECLARE_bool(enable_rocket_frame_relative_alignment);
DECLARE_bool(use_grpc_client);
DECLARE_string(grpc_async_client_poll_mode);
DECLARE_string(grpc_client_mode);

namespace apache::thrift::stress {

enum class ClientSecurity {
  None = 0,
  TLS,
  FIZZ,
};

struct ClientConnectionConfig {
  folly::SocketAddress serverHost;
  ClientSecurity security;
  std::optional<CompressionConfig> compressionConfigOpt;
  std::string certPath;
  std::string keyPath;
  std::string trustedCertsPath;
  bool ioUring{false};
  bool ioUringZcrx{false};
  bool useQuic{false};
  bool stopTLSv1{false};
  bool stopTLSv2{false};
  protocol::PROTOCOL_TYPES thriftProtocol{protocol::T_COMPACT_PROTOCOL};
  folly::AsyncSocket::ConnectCallback* connectCb;
};

struct ClientConfig {
  uint64_t numClientThreads;
  uint64_t numConnectionsPerThread;
  uint64_t numClientsPerConnection;
  bool continuous{false};
  uint64_t numRunsPerClient;
  bool useLoadGenerator;
  uint32_t targetQps;
  std::chrono::duration<int64_t, std::milli> gen_load_interval;
  ClientConnectionConfig connConfig;
  bool enableChecksum{false};
  bool enableRocketFrameRelativeAlignment{false};

  static ClientConfig createFromFlags();
};

} // namespace apache::thrift::stress
