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
#include <compare>
#include <concepts>
#include <functional>
#include <memory>
#include <unordered_map>

#include <folly/CPortability.h>
#include <folly/Optional.h>
#include <folly/functional/Invoke.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/Hash.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache::thrift::op::detail {

// The 'equal to' operator.
//
// Delegates to operator==, by default.
template <typename Tag = void>
struct EqualTo {
  static_assert(type::is_concrete_v<Tag>);

  using T = type::native_type<Tag>;

  bool operator()(const T& lhs, const T& rhs) const { return lhs == rhs; }
};
template <>
struct EqualTo<type::void_t> {
  template <typename L, typename R>
  constexpr bool operator()(const L& lhs, const R& rhs) const {
    return EqualTo<type::infer_tag<L>>{}(lhs, rhs);
  }
};

// The 'identical to' operator.
template <typename Tag, typename = void>
struct IdenticalTo : EqualTo<Tag> {}; // Delegates to EqualTo, by default.
template <>
struct IdenticalTo<type::void_t> {
  template <typename T>
  constexpr bool operator()(const T& lhs, const T& rhs) const {
    return IdenticalTo<type::infer_tag<T>>{}(lhs, rhs);
  }
};

template <typename Tag>
struct LessThan;

template <typename Tag>
struct DefaultComparePolicy;

template <
    typename Tag,
    template <class...> class ComparePolicy = DefaultComparePolicy,
    typename = void>
struct CompareThreeWay;

// The 'less than' operator.
//
// For use with ordered containers.
template <typename Tag>
struct LessThan {
  static_assert(type::is_concrete_v<Tag>);

  using T = type::native_type<Tag>;

  constexpr bool operator()(const T& lhs, const T& rhs) const {
    return CompareThreeWay<Tag>{}(lhs, rhs) == std::partial_ordering::less;
  }
};
template <>
struct LessThan<type::void_t> {
  template <typename L, typename R>
  constexpr bool operator()(const L& lhs, const R& rhs) const {
    return LessThan<type::infer_tag<L>>{}(lhs, rhs);
  }
};

// The type returned by a call to `LessThan::operator()`, if well defined.
template <typename Tag>
using less_than_t = decltype(LessThan<Tag>{}(
    std::declval<const type::native_type<Tag>&>(),
    std::declval<const type::native_type<Tag>&>()));

// If the given tag is comparable using LessThan.
template <typename Tag>
inline constexpr bool less_than_comparable_v =
    folly::is_detected_v<less_than_t, Tag>;

// Resolves to R, if the tag can be used with LessThan.
template <typename Tag, typename R = void>
using if_less_than_comparable = folly::type_t<R, less_than_t<Tag>>;

// A CompareThreeWay implementation that delegates to native comparisons.
// Thrift-aware behavior is supplied by concrete CompareThreeWay
// specializations.
template <typename Tag, typename T = type::native_type<Tag>, typename = void>
struct DefaultCompareThreeWay {
  static_assert(type::is_concrete_v<Tag>);

  constexpr std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    if constexpr (requires {
                    {
                      lhs <=> rhs
                    } -> std::convertible_to<std::partial_ordering>;
                  }) {
      return lhs <=> rhs;
    } else {
      if (std::equal_to<>{}(lhs, rhs)) {
        return std::partial_ordering::equivalent;
      }
      if (std::less<>{}(lhs, rhs)) {
        return std::partial_ordering::less;
      }
      return std::partial_ordering::greater;
    }
  }
};

// The 'compare three way' operator.
template <
    typename Tag,
    template <class...> class ComparePolicy,
    typename Enable>
struct CompareThreeWay : DefaultCompareThreeWay<Tag> {
}; // Delegates by default.

template <typename Tag>
struct DefaultComparePolicy {
  template <typename T>
  std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    return CompareThreeWay<Tag, DefaultComparePolicy>{}(lhs, rhs);
  }
};

template <>
struct DefaultComparePolicy<type::void_t> {
  template <typename T>
  std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    return DefaultComparePolicy<type::infer_tag<T>>{}(lhs, rhs);
  }
};

template <template <class...> class ComparePolicy>
struct CompareThreeWay<type::void_t, ComparePolicy> {
  template <typename T>
  constexpr std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    return CompareThreeWay<type::infer_tag<T>, ComparePolicy>{}(lhs, rhs);
  }
};

// The type returned by a call to `CompareThreeWay::operator()`, if well
// defined.
template <typename Tag>
using compare_three_way_t = decltype(CompareThreeWay<Tag>{}(
    std::declval<const type::native_type<Tag>&>(),
    std::declval<const type::native_type<Tag>&>()));

// If the given tag is comparable.
template <typename Tag>
inline constexpr bool comparable_v =
    folly::is_detected_v<compare_three_way_t, Tag>;

// Resolves to R, if the tag *can* be used with CompareThreeWay.
template <typename Tag, typename R = std::partial_ordering>
using if_comparable = folly::type_t<R, compare_three_way_t<Tag>>;

// Resolves to R, if the tag *cannot* be used with CompareThreeWay.
template <typename Tag, typename R = std::partial_ordering>
using if_not_comparable = std::enable_if_t<!comparable_v<Tag>, R>;

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

template <typename Tag, template <class...> class ComparePolicy>
  requires type::is_a_v<Tag, type::number_c>
struct CompareThreeWay<Tag, ComparePolicy> {
  using T = type::native_type<Tag>;

  std::partial_ordering operator()(T lhs, T rhs) const {
    return std::partial_order(lhs, rhs);
  }
};

// Delegate all IOBuf comparisons directly to folly.
template <typename UTag>
struct CheckIOBufOp {
  static_assert(
      type::is_a_v<UTag, type::string_c>, "expected string or binary");
};
template <typename UTag>
struct EqualTo<type::cpp_type<folly::IOBuf, UTag>> : CheckIOBufOp<UTag>,
                                                     folly::IOBufEqualTo {};
template <typename UTag>
struct EqualTo<type::cpp_type<std::unique_ptr<folly::IOBuf>, UTag>>
    : CheckIOBufOp<UTag>, folly::IOBufEqualTo {};

struct IOBufCompareToStd {
  template <typename T>
  std::partial_ordering operator()(const T& a, const T& b) const {
    return folly::to_underlying(folly::IOBufCompare{}(a, b)) <=> 0;
  }
};

template <typename UTag, template <class...> class ComparePolicy>
struct CompareThreeWay<type::cpp_type<folly::IOBuf, UTag>, ComparePolicy>
    : CheckIOBufOp<UTag>, IOBufCompareToStd {};

template <typename UTag, template <class...> class ComparePolicy>
struct CompareThreeWay<
    type::cpp_type<std::unique_ptr<folly::IOBuf>, UTag>,
    ComparePolicy> : CheckIOBufOp<UTag>,
                     IOBufCompareToStd {};

template <class I1, class I2, class Cmp>
auto lexicographicalCompareThreeWay(I1 f1, I1 l1, I2 f2, I2 l2, Cmp comp)
    -> decltype(comp(*f1, *f2)) {
  for (; f1 != l1 && f2 != l2; ++f1, ++f2) {
    if (auto c = comp(*f1, *f2); c != std::partial_ordering::equivalent) {
      return c;
    }
  }

  return (f1 != l1) ? std::partial_ordering::greater
      : (f2 != l2)  ? std::partial_ordering::less
                    : std::partial_ordering::equivalent;
}

template <class T, class Comp>
[[maybe_unused]] std::partial_ordering sortAndLexicographicalCompareThreeWay(
    const T& lhs, const T& rhs, Comp&& comp) {
  std::vector<decltype(lhs.begin())> l, r;
  for (auto i = lhs.begin(); i != lhs.end(); ++i) {
    l.push_back(i);
  }
  for (auto i = rhs.begin(); i != rhs.end(); ++i) {
    r.push_back(i);
  }
  auto less = [&](auto lhsIter, auto rhsIter) {
    return comp(*lhsIter, *rhsIter) == std::partial_ordering::less;
  };
  auto compare_three_way = [&](auto lhsIter, auto rhsIter) {
    return comp(*lhsIter, *rhsIter);
  };
  std::sort(l.begin(), l.end(), less);
  std::sort(r.begin(), r.end(), less);
  return lexicographicalCompareThreeWay(
      l.begin(), l.end(), r.begin(), r.end(), compare_three_way);
}

template <
    class T,
    class E,
    template <class...> class ComparePolicy = DefaultComparePolicy>
std::partial_ordering compareLists(const T& lhs, const T& rhs) {
  return lexicographicalCompareThreeWay(
      lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), ComparePolicy<E>{});
}

template <
    class T,
    class E,
    template <class...> class ComparePolicy = DefaultComparePolicy>
std::partial_ordering compareSets(const T& lhs, const T& rhs) {
  return sortAndLexicographicalCompareThreeWay(lhs, rhs, ComparePolicy<E>{});
}

template <
    class T,
    class K,
    class V,
    template <class...> class ComparePolicy = DefaultComparePolicy>
std::partial_ordering compareMaps(const T& lhs, const T& rhs) {
  auto compare_three_way = [](const auto& l, const auto& r) {
    auto ret = ComparePolicy<K>{}(l.first, r.first);
    if (ret != std::partial_ordering::equivalent) {
      return ret;
    }
    return ComparePolicy<V>{}(l.second, r.second);
  };

  return sortAndLexicographicalCompareThreeWay(lhs, rhs, compare_three_way);
}

template <
    class T,
    class E,
    template <class...> class ComparePolicy = DefaultComparePolicy>
struct ListLessThan {
  bool operator()(const T& lhs, const T& rhs) const {
    return compareLists<T, E, ComparePolicy>(lhs, rhs) ==
        std::partial_ordering::less;
  }
};

template <
    class T,
    class E,
    template <class...> class ComparePolicy = DefaultComparePolicy>
struct SetLessThan {
  bool operator()(const T& lhs, const T& rhs) const {
    return compareSets<T, E, ComparePolicy>(lhs, rhs) ==
        std::partial_ordering::less;
  }
};

template <
    class T,
    class K,
    class V,
    template <class...> class ComparePolicy = DefaultComparePolicy>
struct MapLessThan {
  bool operator()(const T& lhs, const T& rhs) const {
    return compareMaps<T, K, V, ComparePolicy>(lhs, rhs) ==
        std::partial_ordering::less;
  }
};

template <typename T, typename E>
struct LessThan<type::cpp_type<T, type::list<E>>>
    : std::conditional_t<
          folly::is_invocable_v<std::less<>, const T&, const T&>,
          std::less<>,
          ListLessThan<T, E>> {};

template <typename VTag>
struct LessThan<type::list<VTag>> {
  // Ideally this should be a non-template function. But there are cases like
  //
  //   @cpp.Type={name="fbvector<fbvector<double>>"}
  //   typedef list<list<double>> doubleListList
  //
  // In which case we use `fbvector<double>` with tag `list<double_t>` type
  // Ideally it should be migrated to the following code
  //
  //   @cpp.Type={template="fbvector"}
  //   typedef list<double> doubleList
  //   @cpp.Type={template="fbvector"}
  //   typedef list<doubleList> doubleListList
  //
  // TODO: Migrate all the cases above and make this function non-template.
  template <typename T = type::native_type<type::list<VTag>>>
  bool operator()(const T& lhs, const T& rhs) const {
    // `std::vector::operator<` has the same implementation as this function.
    // https://github.com/gcc-mirror/gcc/blob/6cb2f2c7f36c999590a949f663d6057cbc67271f/libstdc%2B%2B-v3/include/bits/stl_vector.h#L2077-L2081
    return std::lexicographical_compare(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), LessThan<VTag>{});
  }
};

template <typename T, typename E>
struct LessThan<type::cpp_type<T, type::set<E>>>
    : std::conditional_t<
          folly::is_invocable_v<std::less<>, const T&, const T&>,
          std::less<>,
          SetLessThan<T, E>> {};

template <typename T, typename K, typename V>
struct LessThan<type::cpp_type<T, type::map<K, V>>>
    : std::conditional_t<
          folly::is_invocable_v<std::less<>, const T&, const T&>,
          std::less<>,
          MapLessThan<T, K, V>> {};

template <typename K, typename V>
struct LessThan<type::map<K, V>> {
  using map_type = type::native_type<type::map<K, V>>;

  bool operator()(const map_type& x, const map_type& y) const {
    if constexpr (folly::is_invocable_v<
                      std::less<>,
                      const map_type&,
                      const map_type&>) {
      return std::less<>{}(x, y);
    } else {
      return MapLessThan<map_type, K, V>{}(x, y);
    }
  }
};

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

template <typename VTag>
struct EqualTo<type::list<VTag>> {
  // TODO: Similar to LessThan version, this should be a non-template function.
  template <typename T = type::native_type<type::list<VTag>>>
  bool operator()(const T& lhs, const T& rhs) const {
    // `std::vector::operator==` has the same implementation as this function.
    // https://github.com/gcc-mirror/gcc/blob/6cb2f2c7f36c999590a949f663d6057cbc67271f/libstdc%2B%2B-v3/include/bits/stl_vector.h#L2037-L2042
    return lhs.size() == rhs.size() &&
        std::equal(lhs.begin(), lhs.end(), rhs.begin(), EqualTo<VTag>{});
  }
};

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
    template <class...> typename Equality,
    typename Tag = type::map<KTag, VTag>,
    typename T = type::native_type<Tag>>
struct MapEquality {
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
      if (Equality<KTag>()(itr->second->first, entry.first)) {
        // Found the right key! The values must match.
        return Equality<VTag>()(itr->second->second, entry.second);
      }
    }
    return false;
  }
};
template <typename KTag, typename VTag>
struct IdenticalTo<type::map<KTag, VTag>>
    : MapEquality<KTag, VTag, IdenticalTo> {};
template <typename T, typename KTag, typename VTag>
struct IdenticalTo<type::cpp_type<T, type::map<KTag, VTag>>>
    : MapEquality<
          KTag,
          VTag,
          IdenticalTo,
          type::cpp_type<T, type::map<KTag, VTag>>> {};

template <typename T, typename KTag, typename VTag>
struct EqualTo<type::cpp_type<T, type::map<KTag, VTag>>>
    : std::conditional_t<
          folly::is_invocable_v<
              std::equal_to<>,
              const type::native_type<KTag>&,
              const type::native_type<KTag>&> &&
              folly::is_invocable_v<
                  std::equal_to<>,
                  const type::native_type<VTag>&,
                  const type::native_type<VTag>&>,
          std::equal_to<>,
          MapEquality<
              KTag,
              VTag,
              EqualTo,
              type::cpp_type<T, type::map<KTag, VTag>>>> {};

// TODO(dokwon): Support field_ref types.
template <typename Tag, typename Context>
struct IdenticalTo<type::field<Tag, Context>> : IdenticalTo<Tag> {};

template <typename VTag, template <class...> class ComparePolicy>
struct CompareThreeWay<type::list<VTag>, ComparePolicy> {
  template <typename T = type::native_type<type::list<VTag>>>
  std::partial_ordering operator()(const T& l, const T& r) const {
    return compareLists<T, VTag, ComparePolicy>(l, r);
  }
};

template <typename T, typename E, template <class...> class ComparePolicy>
struct CompareThreeWay<type::cpp_type<T, type::list<E>>, ComparePolicy> {
  std::partial_ordering operator()(const T& l, const T& r) const {
    return compareLists<T, E, ComparePolicy>(l, r);
  }
};

template <typename T, typename E>
struct CompareThreeWay<type::cpp_type<T, type::list<E>>, DefaultComparePolicy> {
  std::partial_ordering operator()(const T& l, const T& r) const {
    // Preserve existing `LessThan` behavior for custom C++ containers: prefer
    // their native ordering when present under the default policy.
    if constexpr (folly::is_invocable_v<std::less<>, const T&, const T&>) {
      return DefaultCompareThreeWay<type::cpp_type<T, type::list<E>>>{}(l, r);
    } else {
      return compareLists<T, E>(l, r);
    }
  }
};

template <typename VTag, template <class...> class ComparePolicy>
struct CompareThreeWay<type::set<VTag>, ComparePolicy> {
  template <typename T = type::native_type<type::set<VTag>>>
  std::partial_ordering operator()(const T& l, const T& r) const {
    return compareSets<T, VTag, ComparePolicy>(l, r);
  }
};

template <typename VTag>
struct CompareThreeWay<type::set<VTag>, DefaultComparePolicy> {
  template <typename T = type::native_type<type::set<VTag>>>
  std::partial_ordering operator()(const T& l, const T& r) const {
    return compareSets<T, VTag>(l, r);
  }
};

template <typename T, typename E, template <class...> class ComparePolicy>
struct CompareThreeWay<type::cpp_type<T, type::set<E>>, ComparePolicy> {
  std::partial_ordering operator()(const T& l, const T& r) const {
    return compareSets<T, E, ComparePolicy>(l, r);
  }
};

template <typename T, typename E>
struct CompareThreeWay<type::cpp_type<T, type::set<E>>, DefaultComparePolicy> {
  std::partial_ordering operator()(const T& l, const T& r) const {
    // Preserve existing `LessThan` behavior for custom C++ containers: prefer
    // their native ordering when present under the default policy.
    if constexpr (folly::is_invocable_v<std::less<>, const T&, const T&>) {
      return DefaultCompareThreeWay<type::cpp_type<T, type::set<E>>>{}(l, r);
    } else {
      return compareSets<T, E>(l, r);
    }
  }
};

template <typename K, typename V, template <class...> class ComparePolicy>
struct CompareThreeWay<type::map<K, V>, ComparePolicy> {
  using map_type = type::native_type<type::map<K, V>>;

  std::partial_ordering operator()(const map_type& l, const map_type& r) const {
    return compareMaps<map_type, K, V, ComparePolicy>(l, r);
  }
};

template <typename K, typename V>
struct CompareThreeWay<type::map<K, V>, DefaultComparePolicy> {
  using map_type = type::native_type<type::map<K, V>>;

  std::partial_ordering operator()(const map_type& l, const map_type& r) const {
    // Preserve existing `LessThan` behavior for maps: prefer native ordering
    // when present under the default policy.
    if constexpr (folly::is_invocable_v<
                      std::less<>,
                      const map_type&,
                      const map_type&>) {
      return DefaultCompareThreeWay<type::map<K, V>>{}(l, r);
    } else {
      return compareMaps<map_type, K, V>(l, r);
    }
  }
};

template <
    typename T,
    typename K,
    typename V,
    template <class...> class ComparePolicy>
struct CompareThreeWay<type::cpp_type<T, type::map<K, V>>, ComparePolicy> {
  std::partial_ordering operator()(const T& l, const T& r) const {
    return compareMaps<T, K, V, ComparePolicy>(l, r);
  }
};

template <typename T, typename K, typename V>
struct CompareThreeWay<
    type::cpp_type<T, type::map<K, V>>,
    DefaultComparePolicy> {
  std::partial_ordering operator()(const T& l, const T& r) const {
    // Preserve existing `LessThan` behavior for custom C++ containers: prefer
    // their native ordering when present under the default policy.
    if constexpr (folly::is_invocable_v<std::less<>, const T&, const T&>) {
      return DefaultCompareThreeWay<type::cpp_type<T, type::map<K, V>>>{}(l, r);
    } else {
      return compareMaps<T, K, V>(l, r);
    }
  }
};

// Hooks for adapted types.
template <typename Adapter, typename Tag>
struct EqualTo<type::adapted<Adapter, Tag>> {
  using adapted_tag = type::adapted<Adapter, Tag>;
  static_assert(type::is_concrete_v<adapted_tag>);
  template <typename T>
  constexpr bool operator()(const T& lhs, const T& rhs) const {
    if constexpr (adapt_detail::is_equal_adapter_v<Adapter, T>) {
      return Adapter::equal(lhs, rhs);
    } else if constexpr (folly::is_invocable_v<
                             std::equal_to<>,
                             const T&,
                             const T&>) {
      return lhs == rhs;
    } else {
      const auto& thriftLhs = Adapter::toThrift(lhs);
      const auto& thriftRhs = Adapter::toThrift(rhs);
      return EqualTo<Tag>{}(thriftLhs, thriftRhs);
    }
  }
};
template <
    typename Adapter,
    typename Tag,
    template <class...> class ComparePolicy>
struct CompareThreeWay<type::adapted<Adapter, Tag>, ComparePolicy> {
  using adapted_tag = type::adapted<Adapter, Tag>;
  static_assert(type::is_concrete_v<adapted_tag>);
  template <typename T>
  constexpr std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    if constexpr (adapt_detail::is_compare_three_way_adapter_v<Adapter, T>) {
      return Adapter::compareThreeWay(lhs, rhs);
    } else {
      return compareWithAdapterHooks(lhs, rhs);
    }
  }

 private:
  template <typename T>
  static constexpr std::partial_ordering compareWithAdapterHooks(
      const T& lhs, const T& rhs) {
    if constexpr (adapt_detail::is_less_adapter_v<Adapter, T>) {
      return compareWithAdapterLess(lhs, rhs);
    } else if constexpr (folly::
                             is_invocable_v<std::less<>, const T&, const T&>) {
      return compareWithNativeLess(lhs, rhs);
    } else {
      return compareThrift(lhs, rhs);
    }
  }

  template <typename T>
  static constexpr std::partial_ordering compareWithAdapterLess(
      const T& lhs, const T& rhs) {
    if (EqualTo<adapted_tag>{}(lhs, rhs)) {
      return std::partial_ordering::equivalent;
    }
    if (Adapter::less(lhs, rhs)) {
      return std::partial_ordering::less;
    }
    return std::partial_ordering::greater;
  }

  template <typename T>
  static constexpr std::partial_ordering compareWithNativeLess(
      const T& lhs, const T& rhs) {
    if constexpr (folly::is_invocable_v<std::equal_to<>, const T&, const T&>) {
      return DefaultCompareThreeWay<adapted_tag, T>{}(lhs, rhs);
    } else {
      const auto& thriftLhs = Adapter::toThrift(lhs);
      const auto& thriftRhs = Adapter::toThrift(rhs);
      if (EqualTo<Tag>{}(thriftLhs, thriftRhs)) {
        return std::partial_ordering::equivalent;
      }
      if (std::less<>{}(lhs, rhs)) {
        return std::partial_ordering::less;
      }
      return std::partial_ordering::greater;
    }
  }

  template <typename T>
  static constexpr std::partial_ordering compareThrift(
      const T& lhs, const T& rhs) {
    const auto& thriftLhs = Adapter::toThrift(lhs);
    const auto& thriftRhs = Adapter::toThrift(rhs);
    return ComparePolicy<Tag>{}(thriftLhs, thriftRhs);
  }
};

enum class FieldIterOrder { Declaration, FieldIdAscending };

template <
    FieldIterOrder Order,
    typename T,
    template <class...> class ComparePolicy = DefaultComparePolicy>
std::partial_ordering compareStructFields(const T& lhs, const T& rhs) {
  std::partial_ordering result = std::partial_ordering::equivalent;
  auto compareField = [&](auto id) {
    if (result != std::partial_ordering::equivalent) {
      return;
    }
    using Id = decltype(id);
    using Tag = get_type_tag<T, Id>;
    const auto* lhsValue = get_value_or_null(get<Id>(lhs));
    const auto* rhsValue = get_value_or_null(get<Id>(rhs));

    if (lhsValue == nullptr && rhsValue == nullptr) {
      return;
    }
    if (lhsValue == nullptr) {
      result = std::partial_ordering::less;
      return;
    }
    if (rhsValue == nullptr) {
      result = std::partial_ordering::greater;
      return;
    }
    result = ComparePolicy<Tag>{}(*lhsValue, *rhsValue);
  };

  if constexpr (Order == FieldIterOrder::FieldIdAscending) {
    for_each_field_id_ascending<T>(compareField);
  } else {
    for_each_ordinal<T>(compareField);
  }
  return result;
}

template <
    typename T,
    template <class...> class ComparePolicy = DefaultComparePolicy>
std::partial_ordering compareStructFieldsByFieldId(const T& lhs, const T& rhs) {
  return compareStructFields<
      FieldIterOrder::FieldIdAscending,
      T,
      ComparePolicy>(lhs, rhs);
}

template <typename T, template <class...> class ComparePolicy>
struct CompareThreeWay<type::struct_t<T>, ComparePolicy> {
  std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    return compareStructFields<FieldIterOrder::Declaration, T, ComparePolicy>(
        lhs, rhs);
  }
};

template <typename T, template <class...> class ComparePolicy>
struct CompareThreeWay<type::exception_t<T>, ComparePolicy> {
  std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    return compareStructFields<FieldIterOrder::Declaration, T, ComparePolicy>(
        lhs, rhs);
  }
};

template <
    typename T,
    template <class...> class ComparePolicy = DefaultComparePolicy>
std::partial_ordering compareUnions(const T& lhs, const T& rhs) {
  if (lhs.getType() != rhs.getType()) {
    // Preserve the existing generated/detail less-than order for unions:
    // different active fields are ordered by active type id.
    return lhs.getType() <=> rhs.getType();
  }

  return invoke_by_field_id<T>(
      static_cast<FieldId>(lhs.getType()),
      [&](auto id) {
        using Id = decltype(id);
        using Tag = get_type_tag<T, Id>;
        ::apache::thrift::detail::union_value_unsafe_fn f;
        return ComparePolicy<Tag>{}(f(get<Id>(lhs)), f(get<Id>(rhs)));
      },
      [] {
        return std::partial_ordering::equivalent; // union is __EMPTY__
      });
}

template <
    typename T,
    template <class...> class ComparePolicy = DefaultComparePolicy>
std::partial_ordering compareUnionsByFieldId(const T& lhs, const T& rhs) {
  if (lhs.getType() == T::Type::__EMPTY__ &&
      rhs.getType() == T::Type::__EMPTY__) {
    return std::partial_ordering::equivalent;
  }

  if (lhs.getType() == T::Type::__EMPTY__) {
    return std::partial_ordering::less;
  }

  if (rhs.getType() == T::Type::__EMPTY__) {
    return std::partial_ordering::greater;
  }

  if (lhs.getType() != rhs.getType()) {
    // Under the Thrift object model, fields are compared in field-id order.
    // `getType()` returns field id of active field. If `lhs` and `rhs` have
    // different active field, we can use it to check which one is smaller.
    //
    // A union whose active field has a larger id compares as less-than: its
    // earlier fields are all unset (null < non-null), so it loses before its
    // own active field is ever reached.
    //
    // Example: union A { 1: i32 a1; 2: i32 a2; }
    //   lhs.a2() = 1;  // active field id = 2
    //   rhs.a1() = 2;  // active field id = 1
    //   // Compare a1 first: null (lhs) < 2 (rhs) -> lhs < rhs.
    return lhs.getType() > rhs.getType() ? std::partial_ordering::less
                                         : std::partial_ordering::greater;
  }

  return compareUnions<T, ComparePolicy>(lhs, rhs);
}

template <typename T, template <class...> class ComparePolicy>
struct CompareThreeWay<type::union_t<T>, ComparePolicy> {
  std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    return compareUnions<T, ComparePolicy>(lhs, rhs);
  }
};

template <template <class...> class ComparePolicy = DefaultComparePolicy>
struct StructLessThan {
  template <class T>
  bool operator()(const T& lhs, const T& rhs) const {
    return compareStructFields<FieldIterOrder::Declaration, T, ComparePolicy>(
               lhs, rhs) == std::partial_ordering::less;
  }
};

// StructLessThan but compare fields by sorted field id order (instead of
// field declaration order). This is used to match the behavior of the
// Thrift Object Model, which compares struct fields by sorted field id
// order.
template <template <class...> class ComparePolicy = DefaultComparePolicy>
struct StructLessThanByFieldId {
  template <class T>
  bool operator()(const T& lhs, const T& rhs) const {
    return compareStructFieldsByFieldId<T, ComparePolicy>(lhs, rhs) ==
        std::partial_ordering::less;
  }
};

template <template <class...> class Equality = EqualTo>
struct StructEquality {
  template <class T>
  bool operator()(const T& lhs, const T& rhs) const {
    bool result = true;
    for_each_ordinal<T>([&](auto ord) {
      if (result == false) {
        return;
      }

      using Ord = decltype(ord);
      using Tag = get_type_tag<T, Ord>;
      const auto* lhsValue = get_value_or_null(get<Ord>(lhs));
      const auto* rhsValue = get_value_or_null(get<Ord>(rhs));

      if (lhsValue == nullptr && rhsValue == nullptr) {
        return;
      }

      if (lhsValue == nullptr) {
        result = false;
        return;
      }

      if (rhsValue == nullptr) {
        result = false;
        return;
      }

      if (!Equality<Tag>{}(*lhsValue, *rhsValue)) {
        result = false;
      }
    });

    return result;
  }
};

template <template <class...> class ComparePolicy = DefaultComparePolicy>
struct UnionLessThan {
  template <class T>
  bool operator()(const T& lhs, const T& rhs) const {
    return compareUnions<T, ComparePolicy>(lhs, rhs) ==
        std::partial_ordering::less;
  }
};

// Similar to StructLessThanByFieldId, but for unions
template <template <class...> class ComparePolicy = DefaultComparePolicy>
struct UnionLessThanByFieldId {
  template <class T>
  bool operator()(const T& lhs, const T& rhs) const {
    return compareUnionsByFieldId<T, ComparePolicy>(lhs, rhs) ==
        std::partial_ordering::less;
  }
};

template <template <class...> class Equality = EqualTo>
struct UnionEquality {
  template <class T>
  bool operator()(const T& lhs, const T& rhs) const {
    if (lhs.getType() != rhs.getType()) {
      return false;
    }

    return invoke_by_field_id<T>(
        static_cast<FieldId>(lhs.getType()),
        [&](auto id) {
          using Id = decltype(id);
          using Tag = get_type_tag<T, Id>;
          return Equality<Tag>{}(*get<Id>(lhs), *get<Id>(rhs));
        },
        [] {
          return true; // union is __EMPTY__
        });
  }
};

template <class T>
struct StableComparePolicy {
  template <typename U>
  std::partial_ordering operator()(const U& lhs, const U& rhs) const {
    return CompareThreeWay<T, StableComparePolicy>{}(lhs, rhs);
  }
};

template <>
struct StableComparePolicy<type::void_t> {
  template <typename T>
  std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    return StableComparePolicy<type::infer_tag<T>>{}(lhs, rhs);
  }
};

template <class T>
struct StableComparePolicy<type::struct_t<T>> {
  std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    return compareStructFieldsByFieldId<T, StableComparePolicy>(lhs, rhs);
  }
};

template <class T>
struct StableComparePolicy<type::union_t<T>> {
  std::partial_ordering operator()(const T& lhs, const T& rhs) const {
    return compareUnionsByFieldId<T, StableComparePolicy>(lhs, rhs);
  }
};

// StableLessThan compares Thrift values recursively, comparing
// struct fields by sorted field id order (instead of field declaration order).
// This matches the behavior of the Thrift Object Model for comparison.
template <class T>
struct StableLessThan {
  template <typename U>
  bool operator()(const U& lhs, const U& rhs) const {
    return StableComparePolicy<T>{}(lhs, rhs) == std::partial_ordering::less;
  }
};

template <>
struct StableLessThan<type::void_t> {
  template <typename T>
  bool operator()(const T& lhs, const T& rhs) const {
    return StableLessThan<type::infer_tag<T>>{}(lhs, rhs);
  }
};

} // namespace apache::thrift::op::detail
