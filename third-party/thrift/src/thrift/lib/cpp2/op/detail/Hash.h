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

#include <cmath>

#include <folly/Range.h>
#include <thrift/lib/cpp2/op/DeterministicAccumulator.h>
#include <thrift/lib/cpp2/op/StdHasher.h>
#include <thrift/lib/cpp2/op/detail/HashProtocol.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

// By default, pass the value directly to the accumulator.
template <typename Accumulator, typename T>
void accumulateHash(type::all_c, Accumulator& accumulator, const T& value) {
  accumulator.combine(value);
}

template <typename Accumulator, typename T>
void accumulateHash(type::string_c, Accumulator& accumulator, const T& value) {
  combineBuf(accumulator, value);
}

template <typename VTag, typename Accumulator, typename T>
void accumulateHash(
    type::list<VTag>, Accumulator& accumulator, const T& value) {
  auto listGuard = makeContainerHashGuard(accumulator, value.size());
  for (const auto& i : value) {
    accumulateHash(VTag{}, accumulator, i);
  }
}

template <typename KTag, typename Accumulator, typename T>
void accumulateHash(type::set<KTag>, Accumulator& accumulator, const T& value) {
  auto setGuard = makeContainerHashGuard(accumulator, value.size());
  auto valuesGuard = makeUnorderedHashGuard(accumulator);
  for (const auto& i : value) {
    accumulateHash(KTag{}, accumulator, i);
  }
}

template <typename KTag, typename VTag, typename Accumulator, typename T>
void accumulateHash(
    type::map<KTag, VTag>, Accumulator& accumulator, const T& value) {
  auto mapGuard = makeContainerHashGuard(accumulator, value.size());
  auto valuesGuard = makeUnorderedHashGuard(accumulator);
  for (const auto& i : value) {
    auto pairGuard = makeOrderedHashGuard(accumulator);
    accumulateHash(KTag{}, accumulator, i.first);
    accumulateHash(VTag{}, accumulator, i.second);
  }
}

template <typename Accumulator, typename T>
void accumulateHash(
    type::structured_c, Accumulator& accumulator, const T& value) {
  detail::HashProtocol<Accumulator> protocol(accumulator);
  value.write(&protocol);
}

template <typename Tag>
struct Hash {
  template <typename T, typename Accumulator>
  void operator()(const T& value, Accumulator& accumulator) const {
    accumulateHash(Tag{}, accumulator, value);
  }
  template <typename T = type::native_type<Tag>>
  auto operator()(const T& value) const {
    // TODO(afuller): Only use an accumulator for composite types.
    auto accumulator = makeDeterministicAccumulator<StdHasher>();
    accumulateHash(Tag{}, accumulator, value);
    return std::move(accumulator.result()).getResult();
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
