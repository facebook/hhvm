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

#include <thrift/lib/cpp2/fast_thrift/connection/endpoint/ConnectionListener.h>

#include <array>
#include <cerrno>

#if defined(__linux__)
#include <linux/filter.h>
#include <sys/socket.h>
#endif

#include <folly/String.h>
#include <folly/logging/xlog.h>
#include <folly/net/NetworkSocket.h>

namespace apache::thrift::fast_thrift::connection {

void ConnectionListener::attachReusePortBpfSpread() noexcept {
  // Replace the kernel's default 4-tuple-hash REUSEPORT selection with a
  // 2-instruction cBPF program returning a random u32 — kernel mods by
  // group size internally. Every worker installs the same program on its
  // own fd; last-write-wins is harmless. Failed attach leaves the kernel's
  // default selector in place.
#if defined(__linux__)
  auto code = std::to_array<sock_filter>({
      BPF_STMT(
          BPF_LD | BPF_W | BPF_ABS,
          static_cast<uint32_t>(SKF_AD_OFF + SKF_AD_RANDOM)),
      BPF_STMT(BPF_RET | BPF_A, 0),
  });
  struct sock_fprog prog = {
      .len = static_cast<unsigned short>(code.size()),
      .filter = code.data(),
  };
  for (auto fd : socket_->getNetworkSockets()) {
    if (::setsockopt(
            fd.toFd(),
            SOL_SOCKET,
            SO_ATTACH_REUSEPORT_CBPF,
            &prog,
            sizeof(prog)) != 0) {
      const int savedErrno = errno;
      XLOGF_EVERY_MS(
          ERR,
          1000,
          "SO_ATTACH_REUSEPORT_CBPF failed on fd {}: errno={} ({})",
          fd.toFd(),
          savedErrno,
          folly::errnoStr(savedErrno));
    }
  }
#endif
}

} // namespace apache::thrift::fast_thrift::connection
