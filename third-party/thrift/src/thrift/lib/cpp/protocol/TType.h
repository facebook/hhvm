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

#pragma once

#include <stdint.h>

namespace apache {
namespace thrift {
namespace protocol {

/**
 * Enumerated definition of the types that the Thrift protocol supports.
 * Take special note of the T_STOP type which is used specifically to mark
 * the end of a sequence of fields.
 */
enum TType : uint8_t {
  T_STOP = 0,
  T_VOID = 1,
  T_BOOL = 2,
  T_BYTE = 3,
  T_I08 = 3,
  T_I16 = 6,
  T_I32 = 8,
  T_U64 = 9,
  T_I64 = 10,
  T_DOUBLE = 4,
  T_STRING = 11,
  T_UTF7 = 11,
  T_STRUCT = 12,
  T_MAP = 13,
  T_SET = 14,
  T_LIST = 15,
  T_UTF8 = 16,
  T_UTF16 = 17,
  T_STREAM = 18,
  T_FLOAT = 19,
};

} // namespace protocol
} // namespace thrift
} // namespace apache
