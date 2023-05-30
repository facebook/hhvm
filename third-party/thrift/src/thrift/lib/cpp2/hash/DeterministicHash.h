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

// TODO(afuller): Migrate all usage and delete this folder, when the 'op'
// library is public.
#pragma once

#include <type_traits>
#include <utility>

#include <thrift/lib/cpp2/op/Hash.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache {
namespace thrift {
namespace hash {

/**
 * Utility to compute deterministic hash using custom Hasher implementation.
 * Hashes are guaranteed to be stable and consistent across different languages
 * and implementations. Structs have to be Thrift generated since we use custom
 * protocol API.
 *
 * This function requires Hasher to be default constructible.
 *
 * Example:
 *   MyStruct data = getMyStruct();
 *   auto hash = deterministic_hash<MyHasher>(data);
 */
template <typename Hasher, typename Struct>
auto deterministic_hash(const Struct& data) {
  return deterministic_hash(data, [] { return Hasher{}; });
}

/**
 * Utility to compute deterministic hash using custom Hasher implementation.
 * Hashes are guaranteed to be stable and consistent across different languages
 * and implementations. Structs have to be Thrift generated since we use custom
 * protocol API.
 *
 * This function requires Generator to be a functor producing custom Hashers.
 *
 * Example:
 *   MyStruct data = getMyStruct();
 *   auto generator = [args] { return MyHasher{args}; };
 *   auto hash = deterministic_hash(data, generator);
 */
template <typename Struct, typename HasherGenerator>
auto deterministic_hash(const Struct& data, HasherGenerator generator) {
  op::DeterministicAccumulator acc{std::move(generator)};
  op::hash<type::struct_t<Struct>>(data, acc);
  return std::move(acc.result()).getResult();
}

} // namespace hash
} // namespace thrift
} // namespace apache
