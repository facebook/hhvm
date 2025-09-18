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

#include <thrift/lib/cpp2/type/Name.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache::thrift::conformance::data {

namespace detail {
template <typename Tag>
struct GetTestTypeName : type::detail::GetName<Tag> {};

template <typename Tag>
struct GetTestTypeName<type::list<Tag>> {
  const std::string& operator()() const {
    FOLLY_EXPORT static const auto* kName =
        new std::string(fmt::format("list<{}>", GetTestTypeName<Tag>()()));
    return *kName;
  }
};

template <typename Tag>
struct GetTestTypeName<type::set<Tag>> {
  const std::string& operator()() const {
    FOLLY_EXPORT static const auto* kName =
        new std::string(fmt::format("set<{}>", GetTestTypeName<Tag>()()));
    return *kName;
  }
};

template <typename KeyTag, typename ValueTag>
struct GetTestTypeName<type::map<KeyTag, ValueTag>> {
  const std::string& operator()() const {
    FOLLY_EXPORT static const auto* kName = new std::string(fmt::format(
        "map<{},{}>",
        GetTestTypeName<KeyTag>()(),
        GetTestTypeName<ValueTag>()()));
    return *kName;
  }
};

} // namespace detail

// When called, returns a std::string representing the given
// type tag's name for conformance tests
template <typename T>
inline static constexpr detail::GetTestTypeName<T> getTestTypeName;

} // namespace apache::thrift::conformance::data
