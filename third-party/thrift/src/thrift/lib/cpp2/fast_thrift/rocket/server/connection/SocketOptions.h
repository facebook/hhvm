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

namespace apache::thrift::fast_thrift::rocket::server::connection {

/**
 * Listening-socket tuning knobs applied by ConnectionHandler at
 * startAccepting time. Pure data — set on the embedder via the server's
 * setSocketOptions and forwarded through ConnectionManager.
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
};

} // namespace apache::thrift::fast_thrift::rocket::server::connection
