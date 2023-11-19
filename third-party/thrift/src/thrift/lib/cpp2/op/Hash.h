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

#include <folly/Traits.h>
#include <thrift/lib/cpp2/op/detail/Hash.h>

namespace apache {
namespace thrift {
namespace op {

template <typename Tag>
using Hash = detail::Hash<Tag>;

// Hash the given value. Same hash result will be produced for thrift values
// that are identical to, or equal to each other. Default hash algorithm,
// StdHasher, uses folly::hash_combine.
//
// For example:
//   hash<i32_t>(myInt) // returns hash of myInt.
//   hash<set<i32_t>>(myIntSet) // returns hash of myIntSet
template <typename Tag>
inline constexpr Hash<Tag> hash{};

} // namespace op
} // namespace thrift
} // namespace apache
