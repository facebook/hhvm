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

#include <wangle/acceptor/SocketOptions.h>

#include <folly/io/SocketOptionMap.h>
#include <folly/portability/Sockets.h>

namespace wangle {

folly::SocketOptionMap filterIPSocketOptions(
    const folly::SocketOptionMap& allOptions,
    const int addrFamily) {
  folly::SocketOptionMap opts;

  for (const auto& opt : allOptions) {
    if (opt.first.level == SOL_SOCKET) {
      opts[opt.first] = opt.second;
      continue;
    }

    if (addrFamily == AF_INET) {
      if (opt.first.level == IPPROTO_IPV6) {
        continue;
      }
    } else if (addrFamily == AF_INET6) {
      if (opt.first.level == IPPROTO_IP) {
        continue;
      }
    } else {
      LOG(FATAL) << "Address family " << addrFamily << " was not IPv4 or IPv6";
    }

    // Include all other options
    opts[opt.first] = opt.second;
  }

  return opts;
}

} // namespace wangle
