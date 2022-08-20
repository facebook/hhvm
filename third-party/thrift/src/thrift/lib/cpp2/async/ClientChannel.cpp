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

#include <thrift/lib/cpp2/async/ClientChannel.h>

#ifdef __linux__
#include <sys/utsname.h>
#endif

#include <thrift/lib/cpp2/Flags.h>

THRIFT_FLAG_DEFINE_int64(thrift_client_checksum_sampling_rate, 0);

namespace apache {
namespace thrift {

ClientChannel::ClientChannel() {
  setChecksumSamplingRate(THRIFT_FLAG(thrift_client_checksum_sampling_rate));
}

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(ClientHostMetadata, getClientHostMetadata) {
  ClientHostMetadata hostMetadata;
#ifdef __linux__
  struct utsname bufs;
  ::uname(&bufs);
  hostMetadata.hostname = bufs.nodename;
#endif
  return hostMetadata;
}
} // namespace detail

/* static */ const std::optional<ClientHostMetadata>&
ClientChannel::getHostMetadata() {
  static const auto& hostMetadata =
      *new std::optional<ClientHostMetadata>{detail::getClientHostMetadata()};
  return hostMetadata;
}
} // namespace thrift
} // namespace apache
