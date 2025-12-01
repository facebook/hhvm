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

#include <thrift/lib/cpp2/dynamic/Binary.h>
#include <thrift/lib/cpp2/dynamic/List.h>
#include <thrift/lib/cpp2/dynamic/String.h>
#include <thrift/lib/cpp2/dynamic/Struct.h>
#include <thrift/lib/cpp2/dynamic/Union.h>

#include <folly/Hash.h>

#include <cstddef>
#include <functional>

namespace apache::thrift::dynamic {

// Forward declarations
class Set;
class Map;

namespace detail {

/**
 * Hash functor for Datum values used in F14 containers.
 * Provides transparent hashing for all Thrift types.
 */
struct DatumHash {
  using is_avalanching = void;

  template <typename T>
    requires std::is_arithmetic_v<T>
  std::size_t operator()(const T& value) const {
    return std::hash<T>{}(value);
  }

  std::size_t operator()(const String& value) const {
    return std::hash<std::string_view>{}(value.view());
  }

  std::size_t operator()(const Binary& value) const {
    return folly::IOBufHash{}(value.data_);
  }

  std::size_t operator()(const Any&) const {
    throw std::runtime_error("unimplemented");
  }

  std::size_t operator()(const Struct&) const {
    throw std::runtime_error("unimplemented");
  }

  std::size_t operator()(const Union&) const {
    throw std::runtime_error("unimplemented");
  }

  std::size_t operator()(const List&) const {
    throw std::runtime_error("unimplemented");
  }

  std::size_t operator()(const Set&) const {
    throw std::runtime_error("unimplemented");
  }

  std::size_t operator()(const Map&) const {
    throw std::runtime_error("unimplemented");
  }

  std::size_t operator()(const Null&) const { return 0; }

  std::size_t operator()(const Datum&) const {
    throw std::runtime_error("unimplemented");
  }

  std::size_t operator()(const DynamicConstRef&) const {
    throw std::runtime_error("unimplemented");
  }
};

/**
 * Equality functor for Datum values used in F14 containers.
 * Provides transparent equality comparison for all Thrift types.
 */
struct DatumEqual : std::equal_to<> {
  using is_transparent = void;
};

} // namespace detail
} // namespace apache::thrift::dynamic
