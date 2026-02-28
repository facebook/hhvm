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

#include <thrift/lib/cpp/protocol/TType.h>

// folly::Optional is obsolete so avoid making its definition visible via
// Thrift-generated headers.
namespace folly {
template <class Value>
class Optional;
}

namespace apache::thrift::detail {

union ThriftValue;
using OptionalThriftValue = folly::Optional<ThriftValue>;

/**
 * See `TypeInfo.set`
 */
using VoidPtrFuncPtr = void* (*)(...);

struct TypeInfo {
  protocol::TType type;

  /**
   * Returns the value of a Thrift object (read from the given `valuePtr`), as a
   * native C++ Thrift type.
   *
   * Dereferences smart pointers and converts user-defined (via cpp.type) values
   * as needed.
   */
  OptionalThriftValue (*get)(const void* valuePtr, const TypeInfo& typeInfo);

  /**
   * Writes a Thrift value to the given memory.
   *
   * This functions takes a pointer to the output Thrift object and optionally
   * the value to set (see details below).
   *
   * For container types, the function is the initialization function to clear
   * the container before deserializing into the container.
   *
   * Parameters:
   *
   * 1. `outValuePtr` points to the "target" memory that this function should
   * populate with the appropriate contents corresponding to the value passed as
   * the second argument.
   * 2. The second parameter is the actual value that must be adapted and
   * written to `outValuePtr`. This typically corresponds to a Thrift value that
   * was read from a serialized object, and is in one of the native default C++
   * types (eg. bool, int8_t, int16_t, etc.). Callers must reinterpret_cast the
   * functions to specify the actual type of the second argument.
   *
   * For primitive types, the relationship between `.type` and the expected type
   * of the second argument is fixed, as seen in `readThriftValue()`, i.e.:
   *
   *   `TType.T_I64` -> `std::int64_t`
   *   `TType::T_I32` -> `std::int32_t`
   *   `TType::T_I16` -> `std::int16_t`
   *   `TType::T_BYTE` -> `std::int8_t`
   *   `TType::T_BOOL` -> `bool`
   *   `TType::T_DOUBLE` -> `double`
   *   `TType::T_FLOAT` -> `float`
   *   `TType::T_STRING` -> `const std::string&` or `const folly::IOBuf&`
   *   For the TTypes above, `set` always returns `nullptr`.
   *
   *   `TType::T_STRUCT` -> `const TypeInfo&`. Returns `void*`
   *
   *   For the following TTypes, the `set` function expects a `void*` argument,
   *   and returns a `void*`:
   *     `TType::T_MAP`
   *     `TType::T_SET`
   *     `TType::T_LIST`
   */
  VoidPtrFuncPtr set;

  // A pointer to additional type information, e.g. `MapFieldExt` for a map.
  const void* typeExt;
};

template <typename TypeClass, typename T, typename Enable = void>
struct TypeToInfo;

} // namespace apache::thrift::detail
