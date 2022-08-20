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
#include <unordered_map>

#include <folly/CPortability.h>
#include <folly/Overload.h>
#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/op/Hash.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

template <typename Tag>
struct EqualTo {
  static_assert(type::is_concrete_v<Tag>, "");
  template <typename T1 = type::native_type<Tag>, typename T2 = T1>
  constexpr bool operator()(const T1& lhs, const T2& rhs) const {
    return folly::overload(
        [](const auto& v1, const auto& v2, type::string_c) {
          return StringTraits<T1>::isEqual(v1, v2);
        },
        [](const auto& v1, const auto& v2, type::all_c) { return v1 == v2; })(
        lhs, rhs, Tag{});
  }

 private:
  template <typename T1 = type::native_type<Tag>, typename T2 = T1>
  constexpr bool equalTo(const T1& lhs, const T2& rhs, type::string_c) const {
    return StringTraits<T1>::isEqual(lhs, rhs);
  }

  template <typename T1 = type::native_type<Tag>, typename T2 = T1>
  constexpr bool equalTo(const T1& lhs, const T2& rhs, type::all_c) const {
    return lhs == rhs;
  }
};

template <typename Adapter, typename Tag>
struct EqualTo<type::adapted<Adapter, Tag>> {
  using adapted_tag = type::adapted<Adapter, Tag>;
  static_assert(type::is_concrete_v<adapted_tag>, "");
  template <typename T1, typename T2 = T1>
  constexpr bool operator()(const T1& lhs, const T2& rhs) const {
    return ::apache::thrift::adapt_detail::equal<Adapter>(lhs, rhs);
  }
};

template <typename Tag>
struct IdenticalTo : EqualTo<Tag> {};

// TODO(dokwon): Support field_ref types.
template <typename Tag, typename Context>
struct IdenticalTo<type::field<Tag, Context>> : IdenticalTo<Tag> {};

template <typename F, typename I>
struct FloatIdenticalTo {
  bool operator()(F lhs, F rhs) const {
    // NOTE: Thrift specifies that all NaN variations are considered
    // 'identical'; however, we do not implement that here for performance
    // reasons.
    return folly::bit_cast<I>(lhs) == folly::bit_cast<I>(rhs);
  }
};
template <>
struct IdenticalTo<type::float_t> : FloatIdenticalTo<float, int32_t> {};
template <>
struct IdenticalTo<type::double_t> : FloatIdenticalTo<double, int64_t> {};

template <typename ValTag>
struct IdenticalTo<type::list<ValTag>> {
  template <typename T = type::native_type<type::list<ValTag>>>
  bool operator()(const T& lhs, const T& rhs) const {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    return std::equal(
        lhs.begin(), lhs.end(), rhs.begin(), IdenticalTo<ValTag>());
  }
};

template <typename KeyTag>
struct IdenticalTo<type::set<KeyTag>> {
  // Create a multimap from hash(key)->&key
  template <typename C>
  static auto createHashMap(const C& set) {
    std::unordered_multimap<size_t, typename C::const_pointer> hashMap;
    for (const auto& key : set) {
      hashMap.emplace(op::hash<KeyTag>(key), &key);
    }
    return hashMap;
  }
  // Check if an identical key is in the given range.
  template <typename T, typename R>
  static bool inRange(const T& key, const R& range) {
    for (auto itr = range.first; itr != range.second; ++itr) {
      if (IdenticalTo<KeyTag>()(*itr->second, key)) {
        return true;
      }
    }
    return false;
  }

  template <typename T = type::native_type<type::set<KeyTag>>>
  bool operator()(const T& lhs, const T& rhs) const {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    // Build a map from hash to entry.
    auto hashMap = createHashMap(lhs);
    // Check that all entries match.
    for (const auto& key : rhs) {
      if (!inRange(key, hashMap.equal_range(op::hash<KeyTag>(key)))) {
        return false;
      }
    }
    return true;
  }
};

template <typename KeyTag, typename ValTag>
struct IdenticalTo<type::map<KeyTag, ValTag>> {
  // Create a multimap from hash(key)->&pair(key, value)
  template <typename C>
  static auto createHashMap(const C& map) {
    std::unordered_multimap<size_t, typename C::const_pointer> hashMap;
    for (const auto& entry : map) {
      hashMap.emplace(op::hash<KeyTag>(entry.first), &entry);
    }
    return hashMap;
  }
  // Check if an identical pair(key, value) exists in the range.
  template <typename T, typename R>
  static bool inRange(const T& entry, const R& range) {
    for (auto itr = range.first; itr != range.second; ++itr) {
      if (IdenticalTo<KeyTag>()(itr->second->first, entry.first)) {
        // Found the right key! The values must match.
        return IdenticalTo<ValTag>()(itr->second->second, entry.second);
      }
    }
    return false;
  }

  template <typename T = type::native_type<type::map<KeyTag, ValTag>>>
  bool operator()(const T& lhs, const T& rhs) const {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    // Build a map from hash to entry.
    auto hashMap = createHashMap(lhs);
    // Check that all entries match.
    for (const auto& entry : rhs) {
      if (!inRange(entry, hashMap.equal_range(op::hash<KeyTag>(entry.first)))) {
        return false;
      }
    }
    return true;
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
