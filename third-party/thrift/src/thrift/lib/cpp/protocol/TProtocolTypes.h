/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef THRIFT_PROTOCOL_TPROTOCOLTYPES_H_
#define THRIFT_PROTOCOL_TPROTOCOLTYPES_H_ 1

#include <cstdint>

namespace apache {
namespace thrift {
namespace protocol {

enum PROTOCOL_TYPES : uint16_t {
  T_BINARY_PROTOCOL = 0,
  T_JSON_PROTOCOL = 1,
  T_COMPACT_PROTOCOL = 2,
  T_DEBUG_PROTOCOL = 3,
  T_VIRTUAL_PROTOCOL = 4,
  T_SIMPLE_JSON_PROTOCOL = 5,
  // The frozen2 protocol is deprecated, but we don't want reuse its ID.
  // T_FROZEN2_PROTOCOL = 6,
};
}
} // namespace thrift
} // namespace apache

#endif // #define _THRIFT_PROTOCOL_TPROTOCOLTYPES_H_ 1
