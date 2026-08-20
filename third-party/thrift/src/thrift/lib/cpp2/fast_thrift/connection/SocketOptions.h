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

#include <cstdint>

namespace apache::thrift::fast_thrift::connection {

/**
 * Listening-socket and accept-path tuning knobs. Pure data — set on the
 * embedder via the server's setSocketOptions and forwarded through
 * ConnectionManager to every IO thread's ConnectionHandler.
 */
struct SocketOptions {
  // Accept queue depth (kernel listen backlog).
  uint32_t listenBacklog{1024};

  // TCP Fast Open. When enabled, the kernel may carry payload bytes in
  // the SYN, saving 1 RTT on connection setup. tfoQueueSize bounds the
  // kernel-side TFO pending-cookie queue.
  bool tfoEnabled{false};
  uint32_t tfoQueueSize{0};

  // Per-connection AsyncSocket::setMaxReadsPerEvent — caps how many
  // read() syscalls one socket does per EVB callback before yielding.
  // Matches legacy ThriftServer's socketMaxReadsPerEvent_ default.
  uint32_t maxReadsPerEvent{16};

  // Cap on the connections an IO thread will hold parked in a TLS stage —
  // accepted, but not yet resolved to an established connection: awaiting
  // peek classification, the fizz handshake, or a StopTLS V1 downgrade. Past
  // the cap a connection is closed straight away rather than parked, bounding
  // what a peer that stalls one of those steps can pin. Zero disables the cap.
  //
  // Each parking stage carries the cap separately, so a thread's ceiling is
  // this many per stage the policy installs: two under SSLPolicy::REQUIRED,
  // three under PERMITTED. Has no effect under DISABLED, where a connection
  // is established on the accept itself and is never parked.
  uint32_t maxPendingConnections{0};
};

} // namespace apache::thrift::fast_thrift::connection
