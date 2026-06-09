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
#include <cstdint>

namespace apache::thrift {

struct RpcTransportStats {
  RpcTransportStats() = default;

  uint32_t requestSerializedSizeBytes{
      0}; // size of serialized payload without meta data (uncompressed)
  uint32_t requestWireSizeBytes{0}; // size of data (possibly compressed)
  uint32_t requestMetadataAndPayloadSizeBytes{
      0}; // size of meta data (uncompressed) and data (possibly compressed)
  uint32_t responseSerializedSizeBytes{0};
  uint32_t responseWireSizeBytes{0};
  uint32_t responseMetadataAndPayloadSizeBytes{0};

  // I/O latencies
  std::chrono::nanoseconds requestWriteLatency{0};
  std::chrono::nanoseconds responseRoundTripLatency{0};

  // Sub-measurement of responseRoundTripLatency: the interval from request
  // bytes leaving the kernel (the same epoch responseRoundTripLatency starts
  // from) until the first response PAYLOAD frame is received by the client.
  // Decomposes responseRoundTripLatency into a first-frame interval and an
  // implicit post-first-frame tail. Captured in user space on the client, so
  // it includes server-side post-send delay, network return path, client
  // kernel recv, transport decrypt, frame parse/reassembly, and client
  // scheduling delay before the response callback runs -- it is NOT a true
  // wire-byte TTFB. Non-negative when populated; zero when unset (e.g. on
  // error/timeout paths or before the first response payload frame is
  // received). Populated only by `RocketClientChannelBase` today; other
  // Thrift transports (notably `fast_thrift::ThriftClientChannel`) leave
  // this field at the default zero value.
  std::chrono::nanoseconds firstResponsePayloadFrameLatency{0};
};

} // namespace apache::thrift
