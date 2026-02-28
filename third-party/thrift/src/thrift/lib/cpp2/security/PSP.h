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

namespace apache::thrift {

enum PSPNegotiationVersion : std::uint64_t {
  THRIFT_PSP_NONE = 0,

  // thriftv0 psp negotiation. 1RTT post handshake negotiation using fizz pspv0
  // AES-128-GCM
  THRIFT_PSP_V0 = (1 << 0),
};

}
