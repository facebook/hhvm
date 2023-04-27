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

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <unordered_map>

#include <folly/CPortability.h>
#include <folly/Optional.h>
#include <folly/functional/Invoke.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Ordering.h>
#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/Hash.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

// named comparison functions, similar to c++20
//
// TODO(afuller): Dedupe/Merge with folly.
inline constexpr bool is_eq(folly::ordering cmp) noexcept {
  return cmp == folly::ordering::eq;
}
inline bool is_eq(folly::partial_ordering cmp) noexcept {
  return cmp == folly::partial_ordering::equivalent;
}
inline constexpr bool is_neq(folly::ordering cmp) noexcept {
  return cmp != folly::ordering::eq;
}
inline bool is_neq(folly::partial_ordering cmp) noexcept {
  return cmp != folly::partial_ordering::equivalent;
}
inline constexpr bool is_lt(folly::ordering cmp) noexcept {
  return cmp == folly::ordering::lt;
}
inline constexpr bool is_lteq(folly::ordering cmp) noexcept {
  return cmp != folly::ordering::gt;
}
inline constexpr bool is_gt(folly::ordering cmp) noexcept {
  return cmp == folly::ordering::gt;
}
inline constexpr bool is_gteq(folly::ordering cmp) noexcept {
  return cmp != folly::ordering::lt;
}

// The 'equal to' operator.
//
// Delegates to std::equal_to, by default.
template <typename LTag = void, typename RTag = LTag, typename = void>
struct EqualTo : std::equal_to<type::native_type<LTag>> {
  static_assert(type::is_concrete_v<LTag>, "");
  static_assert(type::is_concrete_v<RTag>, "");
};
template <>
struct EqualTo<type::void_t> {
  template <typename L, typename R>
  constexpr bool operator()(const L& lhs, const R& rhs) const {
    return EqualTo<type::infer_tag<L>, type::infer_tag<R>>{}(lhs, rhs);
  }
};
template <typename LTag>
struct EqualTo<LTag, type::void_t> {
  template <typename L, typename R>
  constexpr bool operator()(const L& lhs, const R& rhs) const {
    return EqualTo<LTag, type::infer_tag<R>>{}(lhs, rhs);
  }
};
template <typename RTag>
struct EqualTo<type::void_t, RTag> {
  template <typename L, typename R>
  constexpr bool operator()(const L& lhs, const R& rhs) const {
    return EqualTo<type::infer_tag<L>, RTag>{}(lhs, rhs);
  }
};

// The 'identical to' operator.
//
// Unlike other binary operators, only accepts a single tag.
template <typename Tag, typename = void>
struct IdenticalTo : EqualTo<Tag> {}; // Delegates to EqualTo, by default.
template <>
struct IdenticalTo<type::void_t> {
  template <typename T>
  constexpr bool operator()(const T& lhs, const T& rhs) const {
    return IdenticalTo<type::infer_tag<T>>{}(lhs, rhs);
  }
};

// The 'less than' operator.
//
// For use with ordered containers.
template <typename LTag, typename RTag = LTag, typename = void>
struct LessThan : std::less<> { // Deletegates to std::less<>, by default.
  static_assert(type::is_concrete_v<LTag>, "");
  static_assert(type::is_concrete_v<RTag>, "");
};
template <>
struct LessThan<type::void_t> {
  template <typename L, typename R>
  constexpr bool operator()(const L& lhs, const R& rhs) const {
    return LessThan<type::infer_tag<L>, type::infer_tag<R>>{}(lhs, rhs);
  }
};
template <typename LTag>
struct LessThan<LTag, type::void_t> {
  template <typename L, typename R>
  constexpr bool operator()(const L& lhs, const R& rhs) const {
    return LessThan<LTag, type::infer_tag<R>>{}(lhs, rhs);
  }
};
template <typename RTag>
struct LessThan<type::void_t, RTag> {
  template <typename L, typename R>
  constexpr bool operator()(const L& lhs, const R& rhs) const {
    return LessThan<type::infer_tag<L>, RTag>{}(lhs, rhs);
  }
};

// The type returned by a call to `LessThan::operator()`, if well defined.
template <typename LTag, typename RTag = LTag>
using less_than_t = decltype(LessThan<LTag, RTag>{}(
    std::declval<const type::native_type<LTag>&>(),
    std::declval<const type::native_type<RTag>&>()));

// If the give tags are comparable using LessThan.
template <typename LTag, typename RTag = LTag>
FOLLY_INLINE_VARIABLE constexpr bool less_than_comparable_v =
    folly::is_detected_v<less_than_t, LTag, RTag>;

// Resolves to R, if the two tags can be used together in LessThan.
template <typename LTag, typename RTag = LTag, typename R = void>
using if_less_than_comparable = folly::type_t<R, less_than_t<LTag, RTag>>;

// A CompareWith implementation that delegates to EqualTo and LessThan.
template <
    typename LTag,
    typename RTag = LTag,
    typename EqualTo = EqualTo<LTag, RTag>,
    typename LessThan = LessThan<LTag, RTag>,
    typename L = type::native_type<LTag>,
    typename R = type::native_type<RTag>,
    typename = if_less_than_comparable<LTag, RTag>>
struct DefaultCompareWith {
  constexpr folly::ordering operator()(const L& lhs, const R& rhs) const {
    if (equalTo(lhs, rhs)) {
      return folly::ordering::eq;
    } else if (lessThan(lhs, rhs)) {
      return folly::ordering::lt;
    }
    return folly::ordering::gt;
  }

 protected:
  EqualTo equalTo;
  LessThan lessThan;
};

// The 'compare with' operator.
//
// TODO(afuller): Add more efficient specializations.
template <typename LTag, typename RTag = LTag, typename = void>
struct CompareWith : DefaultCompareWith<LTag, RTag> {}; // Delegates by default.

template <>
struct CompareWith<type::void_t> {
  template <typename L, typename R>
  constexpr folly::ordering operator()(const L& lhs, const R& rhs) const {
    return CompareWith<type::infer_tag<L>, type::infer_tag<R>>{}(lhs, rhs);
  }
};
template <typename LTag>
struct CompareWith<LTag, type::void_t> {
  template <typename L, typename R>
  constexpr folly::ordering operator()(const L& lhs, const R& rhs) const {
    return CompareWith<LTag, type::infer_tag<R>>{}(lhs, rhs);
  }
};
template <typename RTag>
struct CompareWith<type::void_t, RTag> {
  template <typename L, typename R>
  constexpr folly::ordering operator()(const L& lhs, const R& rhs) const {
    return CompareWith<type::infer_tag<L>, RTag>{}(lhs, rhs);
  }
};

// The type returned by a call to `CompareWith::operator()`, if well defined.
template <typename LTag, typename RTag = LTag>
using compare_with_t = decltype(CompareWith<LTag, RTag>{}(
    std::declval<const type::native_type<LTag>&>(),
    std::declval<const type::native_type<RTag>&>()));

// If the give tags are comparable.
template <typename LTag, typename RTag = LTag>
FOLLY_INLINE_VARIABLE constexpr bool comparable_v =
    folly::is_detected_v<compare_with_t, LTag, RTag>;

// Resolves to R, if the two tags *can* be used together in CompareWith.
template <
    typename LTag,
    typename RTag = LTag,
    typename R = folly::partial_ordering>
using if_comparable = folly::type_t<R, compare_with_t<LTag, RTag>>;

// Resolves to R, if the two tags *cannot* be used together in CompareWith.
template <
    typename LTag,
    typename RTag = LTag,
    typename R = folly::partial_ordering>
using if_not_comparable = std::enable_if_t<!comparable_v<LTag, RTag>, R>;

// An EqualTo that delegates to CompareWith.
template <
    typename LTag,
    typename RTag = LTag,
    typename CompareWith = CompareWith<LTag, RTag>,
    typename L = type::native_type<LTag>,
    typename R = type::native_type<RTag>>
struct DefaultEqualTo {
  constexpr bool operator()(const L& lhs, const R& rhs) const {
    return is_eq(compareWith(lhs, rhs));
  }

 protected:
  CompareWith compareWith;
};

// A LessThan that delegates to CompareWith.
template <
    typename LTag,
    typename RTag = LTag,
    typename CompareWith = CompareWith<LTag, RTag>,
    typename L = type::native_type<LTag>,
    typename R = type::native_type<RTag>>
struct DefaultLessThan {
  constexpr bool operator()(const L& lhs, const R& rhs) const {
    return is_lt(compareWith(lhs, rhs));
  }

 protected:
  CompareWith compareWith;
};

// Use bit_cast for floating point identical.
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

// Delegate all IOBuf comparisons directly to folly.
template <typename LUTag, typename RUTag>
struct CheckIOBufOp {
  static_assert(
      type::is_a_v<LUTag, type::string_c> &&
          type::is_a_v<RUTag, type::string_c>,
      "expected string or binary");
};
template <typename LUTag, typename RUTag>
struct EqualTo<
    type::cpp_type<folly::IOBuf, LUTag>,
    type::cpp_type<folly::IOBuf, RUTag>> : CheckIOBufOp<LUTag, RUTag>,
                                           folly::IOBufEqualTo {};
template <typename LUTag, typename RUTag>
struct EqualTo<
    type::cpp_type<std::unique_ptr<folly::IOBuf>, LUTag>,
    type::cpp_type<std::unique_ptr<folly::IOBuf>, RUTag>>
    : CheckIOBufOp<LUTag, RUTag>, folly::IOBufEqualTo {};
template <typename LUTag, typename RUTag>
struct LessThan<
    type::cpp_type<folly::IOBuf, LUTag>,
    type::cpp_type<folly::IOBuf, RUTag>> : CheckIOBufOp<LUTag, RUTag>,
                                           folly::IOBufLess {};
template <typename LUTag, typename RUTag>
struct LessThan<
    type::cpp_type<std::unique_ptr<folly::IOBuf>, LUTag>,
    type::cpp_type<std::unique_ptr<folly::IOBuf>, RUTag>>
    : CheckIOBufOp<LUTag, RUTag>, folly::IOBufLess {};

template <typename LUTag, typename RUTag>
struct CompareWith<
    type::cpp_type<folly::IOBuf, LUTag>,
    type::cpp_type<folly::IOBuf, RUTag>> : CheckIOBufOp<LUTag, RUTag>,
                                           folly::IOBufCompare {};
template <typename LUTag, typename RUTag>
struct CompareWith<
    type::cpp_type<std::unique_ptr<folly::IOBuf>, LUTag>,
    type::cpp_type<std::unique_ptr<folly::IOBuf>, RUTag>>
    : CheckIOBufOp<LUTag, RUTag>, folly::IOBufCompare {};

template <class T, class Comp>
FOLLY_MAYBE_UNUSED bool sortAndLexicographicalCompare(
    const T& lhs, const T& rhs, Comp&& comp) {
  std::vector<const typename T::value_type*> l, r;
  for (const auto& i : lhs) {
    l.push_back(&i);
  }
  for (const auto& i : rhs) {
    r.push_back(&i);
  }
  auto less = [&](const auto* lhsPtr, const auto* rhsPtr) {
    return comp(*lhsPtr, *rhsPtr);
  };
  std::sort(l.begin(), l.end(), less);
  std::sort(r.begin(), r.end(), less);
  return std::lexicographical_compare(
      l.begin(), l.end(), r.begin(), r.end(), less);
}

template <class T, class E, template <class...> class LessThanImpl = LessThan>
struct ListLessThan {
  bool operator()(const T& l, const T& r) const {
    return std::lexicographical_compare(
        l.begin(), l.end(), r.begin(), r.end(), LessThanImpl<E>{});
  }
};

template <class T, class E, template <class...> class LessThanImpl = LessThan>
struct SetLessThan {
  bool operator()(const T& lhs, const T& rhs) const {
    return sortAndLexicographicalCompare(lhs, rhs, LessThanImpl<E>{});
  }
};

template <
    class T,
    class K,
    class V,
    template <class...> class LessThanImpl = LessThan>
struct MapLessThan {
  bool operator()(const T& lhs, const T& rhs) const {
    auto less = [](const auto& l, const auto& r) {
      if (LessThanImpl<K>{}(l.first, r.first)) {
        return true;
      }
      if (LessThanImpl<K>{}(r.first, l.first)) {
        return false;
      }
      return LessThanImpl<V>{}(l.second, r.second);
    };
    return sortAndLexicographicalCompare(lhs, rhs, less);
  }
};

template <typename T, typename E>
struct LessThan<
    type::cpp_type<T, type::list<E>>,
    type::cpp_type<T, type::list<E>>>
    : std::conditional_t<
          folly::is_invocable_v<std::less<>, const T&, const T&>,
          std::less<>,
          ListLessThan<T, E>> {};

template <typename T, typename E>
struct LessThan<
    type::cpp_type<T, type::set<E>>,
    type::cpp_type<T, type::set<E>>>
    : std::conditional_t<
          folly::is_invocable_v<std::less<>, const T&, const T&>,
          std::less<>,
          SetLessThan<T, E>> {};

template <typename T, typename K, typename V>
struct LessThan<
    type::cpp_type<T, type::map<K, V>>,
    type::cpp_type<T, type::map<K, V>>>
    : std::conditional_t<
          folly::is_invocable_v<std::less<>, const T&, const T&>,
          std::less<>,
          MapLessThan<T, K, V>> {};

// Identical for lists.
template <
    typename VTag,
    typename Tag = type::list<VTag>,
    typename T = type::native_type<Tag>>
struct ListIdenticalTo {
  bool operator()(const T& lhs, const T& rhs) const {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), cmp);
  }

 protected:
  IdenticalTo<VTag> cmp;
};
template <typename VTag>
struct IdenticalTo<type::list<VTag>> : ListIdenticalTo<VTag> {};
template <typename T, typename VTag>
struct IdenticalTo<type::cpp_type<T, type::list<VTag>>>
    : ListIdenticalTo<VTag, type::cpp_type<T, type::list<VTag>>> {};

// Identical for sets.
template <
    typename KTag,
    typename Tag = type::set<KTag>,
    typename T = type::native_type<Tag>>
struct SetIdenticalTo {
  bool operator()(const T& lhs, const T& rhs) const {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    // Build a map from hash to entry.
    auto hashMap = createHashMap(lhs);
    // Check that all entries match.
    for (const auto& key : rhs) {
      if (!inRange(key, hashMap.equal_range(op::hash<KTag>(key)))) {
        return false;
      }
    }
    return true;
  }

 private:
  // Create a multimap from hash(key)->&key
  static auto createHashMap(const T& set) {
    std::unordered_multimap<size_t, typename T::const_pointer> hashMap;
    for (const auto& key : set) {
      hashMap.emplace(op::hash<KTag>(key), &key);
    }
    return hashMap;
  }

  // Check if an identical key is in the given range.
  template <typename K, typename R>
  static bool inRange(const K& key, const R& range) {
    for (auto itr = range.first; itr != range.second; ++itr) {
      if (IdenticalTo<KTag>()(*itr->second, key)) {
        return true;
      }
    }
    return false;
  }
};
template <typename KTag>
struct IdenticalTo<type::set<KTag>> : SetIdenticalTo<KTag> {};
template <typename T, typename KTag>
struct IdenticalTo<type::cpp_type<T, type::set<KTag>>>
    : SetIdenticalTo<KTag, type::cpp_type<T, type::set<KTag>>> {};

// Identical for maps.
template <
    typename KTag,
    typename VTag,
    typename Tag = type::map<KTag, VTag>,
    typename T = type::native_type<Tag>>
struct MapIdenticalTo {
  bool operator()(const T& lhs, const T& rhs) const {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    // Build a map from hash to entry.
    auto hashMap = createHashMap(lhs);
    // Check that all entries match.
    for (const auto& entry : rhs) {
      if (!inRange(entry, hashMap.equal_range(op::hash<KTag>(entry.first)))) {
        return false;
      }
    }
    return true;
  }

 private:
  // Create a multimap from hash(key)->&pair(key, value)
  static auto createHashMap(const T& map) {
    std::unordered_multimap<size_t, typename T::const_pointer> hashMap;
    for (const auto& entry : map) {
      hashMap.emplace(op::hash<KTag>(entry.first), &entry);
    }
    return hashMap;
  }

  // Check if an identical pair(key, value) exists in the range.
  template <typename E, typename R>
  static bool inRange(const E& entry, const R& range) {
    for (auto itr = range.first; itr != range.second; ++itr) {
      if (IdenticalTo<KTag>()(itr->second->first, entry.first)) {
        // Found the right key! The values must match.
        return IdenticalTo<VTag>()(itr->second->second, entry.second);
      }
    }
    return false;
  }
};
template <typename KTag, typename VTag>
struct IdenticalTo<type::map<KTag, VTag>> : MapIdenticalTo<KTag, VTag> {};
template <typename T, typename KTag, typename VTag>
struct IdenticalTo<type::cpp_type<T, type::map<KTag, VTag>>>
    : MapIdenticalTo<KTag, VTag, type::cpp_type<T, type::map<KTag, VTag>>> {};

// TODO(dokwon): Support field_ref types.
template <typename Tag, typename Context>
struct IdenticalTo<type::field<Tag, Context>> : IdenticalTo<Tag> {};

// Hooks for adapted types.
template <typename Adapter, typename Tag>
struct EqualTo<type::adapted<Adapter, Tag>> {
  using adapted_tag = type::adapted<Adapter, Tag>;
  static_assert(type::is_concrete_v<adapted_tag>, "");
  template <typename T1, typename T2 = T1>
  constexpr bool operator()(const T1& lhs, const T2& rhs) const {
    return ::apache::thrift::adapt_detail::equal<Adapter>(lhs, rhs);
  }
};
template <typename Adapter, typename Tag>
struct LessThan<type::adapted<Adapter, Tag>> {
  using adapted_tag = type::adapted<Adapter, Tag>;
  static_assert(type::is_concrete_v<adapted_tag>, "");
  template <typename T1, typename T2 = T1>
  constexpr bool operator()(const T1& lhs, const T2& rhs) const {
    return ::apache::thrift::adapt_detail::less<Adapter>(lhs, rhs);
  }
};

template <template <class...> class LessThanImpl = LessThan>
struct StructLessThan {
  template <class T>
  bool operator()(const T& lhs, const T& rhs) const {
    folly::Optional<bool> result;
    for_each_ordinal<T>([&](auto ord) {
      if (result.has_value()) {
        return;
      }

      using Ord = decltype(ord);
      using Tag = get_type_tag<T, Ord>;
      const auto* lhsValue = getValueOrNull(get<Ord>(lhs));
      const auto* rhsValue = getValueOrNull(get<Ord>(rhs));

      if (lhsValue == rhsValue) {
        return;
      }

      if (lhsValue == nullptr) {
        result = true;
        return;
      }

      if (rhsValue == nullptr) {
        result = false;
        return;
      }

      LessThanImpl<Tag> less;
      if (less(*lhsValue, *rhsValue)) {
        result = true;
        return;
      }

      if (less(*rhsValue, *lhsValue)) {
        result = false;
        return;
      }
    });

    if (result.has_value()) {
      return *result;
    }

    return false;
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
