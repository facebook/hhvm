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

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/ClientChannel.h>

#if __has_include(<unistd.h>)
#include <unistd.h>
#define THRIFT_HAS_UNISTD_H
#endif

#include <limits.h> // IWYU pragma: keep

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

namespace apache::thrift {

ClientChannel::ClientChannel() {
  setChecksumSamplingRate(THRIFT_FLAG(thrift_client_checksum_sampling_rate));
}

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(ClientHostMetadata, getClientHostMetadata) {
  ClientHostMetadata hostMetadata;
#ifdef THRIFT_HAS_UNISTD_H
  char hostname[HOST_NAME_MAX + 1];
  if (gethostname(hostname, sizeof(hostname)) == 0) {
    hostMetadata.hostname = hostname;
  }
#endif
  return hostMetadata;
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    void, hookForClientTransport, folly::AsyncTransport*) {
  return;
}
} // namespace detail

/* static */ const std::optional<ClientHostMetadata>&
ClientChannel::getHostMetadata() {
  static const auto& hostMetadata =
      *new std::optional<ClientHostMetadata>{detail::getClientHostMetadata()};
  return hostMetadata;
}
} // namespace apache::thrift
