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

#include <thrift/lib/cpp2/gen/module_types_cpp.h>

#include <algorithm>
#include <concepts>

#include <gtest/gtest.h>
#include <folly/Memory.h>
#include <folly/Traits.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <thrift/lib/cpp2/TypeClass.h>

using namespace apache::thrift;

namespace {

template <typename Container>
struct NonCopyable : Container {
  using Container::Container;

  NonCopyable() = default;
  NonCopyable(NonCopyable&&) = default;
  NonCopyable& operator=(NonCopyable&&) = default;
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};

template <typename T>
using NonCopyableVector = NonCopyable<std::vector<T>>;
template <typename T>
using NonCopyableSet = NonCopyable<std::set<T>>;
template <typename K, typename V>
using NonCopyableMap = NonCopyable<std::map<K, V>>;
template <typename T>
using NonCopyableF14FastSet = NonCopyable<folly::F14FastSet<T>>;
template <typename K, typename V>
using NonCopyableF14FastMap = NonCopyable<folly::F14FastMap<K, V>>;

template <typename T, typename... Values>
T makeValue(Values&&... values);

struct MakeValueImpl {
  template <typename T, typename... Values>
    requires folly::is_instantiation_of_v<std::vector, T> ||
      folly::is_instantiation_of_v<std::set, T> ||
      folly::is_instantiation_of_v<folly::F14FastSet, T>
  void operator()(T& out, Values&&... values) const {
    (out.insert(
         out.end(),
         makeValue<typename T::value_type>(std::forward<Values>(values))),
     ...);
  }

  template <typename T, typename... Values>
    requires folly::is_instantiation_of_v<std::map, T> ||
      folly::is_instantiation_of_v<folly::F14FastMap, T>
  void operator()(T& out, Values&&... values) const {
    (out.insert(
         out.end(),
         {makeValue<typename T::key_type>(values.first),
          makeValue<typename T::mapped_type>(values.second)}),
     ...);
  }

  template <typename T, typename D, typename... Values>
  void operator()(std::unique_ptr<T, D>& out, Values&&... values) const {
    out = std::make_unique<T>(makeValue<T>(std::forward<Values>(values)...));
  }

  template <typename C, typename... Values>
  void operator()(NonCopyable<C>& out, Values&&... values) const {
    (*this)(static_cast<C&>(out), std::forward<Values>(values)...);
  }

  template <typename T, typename... Values>
  void operator()(T& out, Values&&... values) const {
    out = T(std::forward<Values>(values)...);
  }
};

template <typename T, typename... Values>
T makeValue(Values&&... values) {
  T out{};
  MakeValueImpl{}(out, std::forward<Values>(values)...);
  return out;
}

struct DeepEqualFn {
  template <typename T, typename D>
  bool operator()(
      const std::unique_ptr<T, D>& a, const std::unique_ptr<T, D>& b) const {
    if (a == nullptr || b == nullptr) {
      return a == nullptr && b == nullptr;
    }
    return (*this)(*a, *b);
  }

  template <typename T>
    requires folly::is_instantiation_of_v<std::vector, T> ||
      folly::is_instantiation_of_v<std::set, T>
  bool operator()(const T& a, const T& b) const {
    return std::ranges::equal(a, b, *this);
  }

  template <typename T>
    requires folly::is_instantiation_of_v<std::map, T> ||
      folly::is_instantiation_of_v<folly::F14FastMap, T>
  bool operator()(const T& a, const T& b) const {
    return a.size() == b.size() &&
        std::ranges::all_of(a, [&](const auto& entry) {
             const auto found = b.find(entry.first);
             return found != b.end() && (*this)(entry.second, found->second);
           });
  }

  template <typename T>
    requires folly::is_instantiation_of_v<folly::F14FastSet, T>
  bool operator()(const T& a, const T& b) const {
    return a.size() == b.size() &&
        std::ranges::all_of(a, [&](const auto& e) { return b.contains(e); });
  }

  template <typename C>
  bool operator()(const NonCopyable<C>& a, const NonCopyable<C>& b) const {
    return (*this)(static_cast<const C&>(a), static_cast<const C&>(b));
  }

  template <typename T>
  bool operator()(const T& a, const T& b) const {
    return a == b;
  }
};

template <typename T, typename U>
  requires std::same_as<T, U>
bool deepEqual(const T& a, const U& b) {
  return DeepEqualFn{}(a, b);
}

} // namespace

TEST(CopyUniqueTest, CopyConstructibleCheck) {
  using namespace type_class;
  using apache::thrift::detail::st::copy_constructible_check;

  EXPECT_TRUE((copy_constructible_check<integral, int>()));
  EXPECT_TRUE((copy_constructible_check<list<integral>, std::vector<int>>()));
  EXPECT_TRUE((copy_constructible_check<set<integral>, std::set<int>>()));
  EXPECT_TRUE((
      copy_constructible_check<map<integral, integral>, std::map<int, int>>()));
  EXPECT_TRUE((copy_constructible_check<
               list<list<integral>>,
               std::vector<std::vector<int>>>()));
  EXPECT_TRUE((copy_constructible_check<
               map<integral, list<integral>>,
               std::map<int, std::vector<int>>>()));

  EXPECT_FALSE((copy_constructible_check<integral, std::unique_ptr<int>>()));
  EXPECT_FALSE((copy_constructible_check<
                list<integral>,
                std::vector<std::unique_ptr<int>>>()));
  EXPECT_FALSE((copy_constructible_check<
                set<integral>,
                std::set<std::unique_ptr<int>>>()));
  EXPECT_FALSE((copy_constructible_check<
                map<integral, integral>,
                std::map<int, std::unique_ptr<int>>>()));
  EXPECT_FALSE((copy_constructible_check<
                map<integral, integral>,
                std::map<std::unique_ptr<int>, int>>()));
  EXPECT_FALSE((copy_constructible_check<
                list<list<integral>>,
                std::vector<std::vector<std::unique_ptr<int>>>>()));
  EXPECT_FALSE((copy_constructible_check<
                map<integral, list<integral>>,
                std::map<int, std::vector<std::unique_ptr<int>>>>()));

  EXPECT_FALSE(
      (copy_constructible_check<list<integral>, NonCopyableVector<int>>()));
  EXPECT_FALSE(
      (copy_constructible_check<set<integral>, NonCopyableSet<int>>()));
  EXPECT_FALSE((copy_constructible_check<
                map<integral, integral>,
                NonCopyableMap<int, int>>()));
  EXPECT_FALSE((copy_constructible_check<
                list<list<integral>>,
                std::vector<NonCopyableVector<int>>>()));
  EXPECT_FALSE((copy_constructible_check<
                map<integral, list<integral>>,
                std::map<int, NonCopyableVector<int>>>()));
}

TEST(CopyUniqueTest, NoUnique) {
  auto i = 42;
  auto i_copy = detail::st::copy_field<type_class::integral>(i);
  static_assert(std::is_same_v<decltype(i), decltype(i_copy)>);
  EXPECT_EQ(i, i_copy);
  EXPECT_NE(&i, &i_copy);

  auto s = std::string("foo");
  auto s_copy = detail::st::copy_field<type_class::string>(s);
  static_assert(std::is_same_v<decltype(s), decltype(s_copy)>);
  EXPECT_EQ(s, s_copy);
  EXPECT_NE(&s, &s_copy);

  auto v = std::vector<int>{1, 2, 3};
  auto v_copy =
      detail::st::copy_field<type_class::list<type_class::integral>>(v);
  static_assert(std::is_same_v<decltype(v), std::vector<int>>);
  EXPECT_EQ(v, v_copy);
  EXPECT_NE(&v, &v_copy);

  auto se = std::set<int>{1, 2, 3};
  auto se_copy =
      detail::st::copy_field<type_class::set<type_class::integral>>(se);
  static_assert(std::is_same_v<decltype(se), decltype(se_copy)>);
  EXPECT_EQ(se, se_copy);
  EXPECT_NE(&se, &se_copy);

  auto m = std::map<int, int>{{0, 101}, {1, 202}};
  auto m_copy = detail::st::copy_field<
      type_class::map<type_class::integral, type_class::integral>>(m);
  static_assert(std::is_same_v<decltype(m), decltype(m_copy)>);
  EXPECT_EQ(m, m_copy);
  EXPECT_NE(&m, &m_copy);
}

TEST(CopyUniqueTest, Unique) {
  using namespace type_class;
  using apache::thrift::detail::st::copy_field;

  auto v = makeValue<std::vector<std::unique_ptr<int>>>(101, 202);
  auto v_copy = copy_field<list<integral>>(v);
  EXPECT_TRUE(deepEqual(v, v_copy));

  auto s = makeValue<std::set<std::unique_ptr<int>>>(101, 202);
  auto s_copy = copy_field<set<integral>>(s);
  EXPECT_TRUE(deepEqual(s, s_copy));

  auto m = makeValue<std::map<int, std::unique_ptr<int>>>(
      std::pair{0, 101}, std::pair{1, 202});
  auto m_copy = copy_field<map<integral, integral>>(m);
  EXPECT_TRUE(deepEqual(m, m_copy));
}

// Field-level annotations such as `cpp.ref`, `cpp.ref_type = "unique"`.
TEST(CopyUniqueTest, UniqueRef) {
  using namespace type_class;
  using apache::thrift::detail::st::copy_field;

  auto pi = makeValue<std::unique_ptr<int>>(42);
  auto pi_copy = copy_field<integral>(pi);
  EXPECT_TRUE(deepEqual(pi, pi_copy));

  auto ps = makeValue<std::unique_ptr<std::string>>("foo");
  auto ps_copy = copy_field<string>(ps);
  EXPECT_TRUE(deepEqual(ps, ps_copy));

  auto pv = makeValue<std::unique_ptr<std::vector<int>>>(1, 2, 3);
  auto pv_copy = copy_field<list<integral>>(pv);
  EXPECT_TRUE(deepEqual(pv, pv_copy));

  auto pvp =
      makeValue<std::unique_ptr<std::vector<std::unique_ptr<int>>>>(101, 202);
  auto pvp_copy = copy_field<list<integral>>(pvp);
  EXPECT_TRUE(deepEqual(pvp, pvp_copy));

  auto psp =
      makeValue<std::unique_ptr<std::set<std::unique_ptr<int>>>>(101, 202);
  auto psp_copy = copy_field<set<integral>>(psp);
  EXPECT_TRUE(deepEqual(psp, psp_copy));

  auto pmp = makeValue<std::unique_ptr<std::map<int, std::unique_ptr<int>>>>(
      std::pair{0, 101}, std::pair{1, 202});
  auto pmp_copy = copy_field<map<integral, integral>>(pmp);
  EXPECT_TRUE(deepEqual(pmp, pmp_copy));
}

TEST(CopyUniqueTest, AllocatorUniqueRef) {
  using namespace type_class;
  using apache::thrift::detail::st::copy_field;

  auto pi = folly::allocate_unique<int>(std::allocator<int>{}, 42);
  auto pi_copy = copy_field<integral>(pi);
  EXPECT_TRUE(deepEqual(pi, pi_copy));

  using Vec = std::vector<std::unique_ptr<int>>;
  auto pvp = folly::allocate_unique<Vec>(
      std::allocator<Vec>{}, makeValue<Vec>(101, 202));
  auto pvp_copy = copy_field<list<integral>>(pvp);
  EXPECT_TRUE(deepEqual(pvp, pvp_copy));
}

TEST(CopyUniqueTest, NullUniqueRef) {
  std::unique_ptr<int> pi;
  EXPECT_EQ(detail::st::copy_field<type_class::integral>(pi), nullptr);

  std::unique_ptr<int, folly::allocator_delete<std::allocator<int>>> pa;
  EXPECT_EQ(detail::st::copy_field<type_class::integral>(pa), nullptr);
}

TEST(CopyUniqueTest, NonCopyableContainer) {
  using namespace type_class;
  using apache::thrift::detail::st::copy_field;

  auto v = makeValue<NonCopyableVector<int>>(1, 2, 3);
  auto v_copy = copy_field<list<integral>>(v);
  EXPECT_TRUE(deepEqual(v, v_copy));

  auto s = makeValue<NonCopyableSet<int>>(1, 2, 3);
  auto s_copy = copy_field<set<integral>>(s);
  EXPECT_TRUE(deepEqual(s, s_copy));

  auto m =
      makeValue<NonCopyableMap<int, int>>(std::pair{0, 101}, std::pair{1, 202});
  auto m_copy = copy_field<map<integral, integral>>(m);
  EXPECT_TRUE(deepEqual(m, m_copy));
}

TEST(CopyUniqueTest, NonCopyableContainerOfUnique) {
  using namespace type_class;
  using apache::thrift::detail::st::copy_field;

  auto v = makeValue<NonCopyableVector<std::unique_ptr<int>>>(101, 202);
  auto v_copy = copy_field<list<integral>>(v);
  EXPECT_TRUE(deepEqual(v, v_copy));

  auto m = makeValue<NonCopyableMap<int, std::unique_ptr<int>>>(
      std::pair{0, 101}, std::pair{1, 202});
  auto m_copy = copy_field<map<integral, integral>>(m);
  EXPECT_TRUE(deepEqual(m, m_copy));
}

TEST(CopyUniqueTest, NestedNonCopyableContainer) {
  using namespace type_class;
  using apache::thrift::detail::st::copy_field;

  std::vector<NonCopyableVector<int>> v;
  v.push_back(makeValue<NonCopyableVector<int>>(1, 2, 3));
  v.push_back(makeValue<NonCopyableVector<int>>(4, 5));
  auto v_copy = copy_field<list<list<integral>>>(v);
  EXPECT_TRUE(deepEqual(v, v_copy));
}

TEST(CopyUniqueTest, F14StringKeys) {
  using namespace type_class;
  using apache::thrift::detail::st::copy_field;

  auto m = makeValue<folly::F14FastMap<std::string, int>>(
      std::pair{"a", 101}, std::pair{"b", 202});
  auto m_copy = copy_field<map<string, integral>>(m);
  EXPECT_TRUE(deepEqual(m, m_copy));

  auto mp = makeValue<folly::F14FastMap<std::string, std::unique_ptr<int>>>(
      std::pair{"a", 101}, std::pair{"b", 202});
  auto mp_copy = copy_field<map<string, integral>>(mp);
  EXPECT_TRUE(deepEqual(mp, mp_copy));

  auto mn = makeValue<NonCopyableF14FastMap<std::string, int>>(
      std::pair{"a", 101}, std::pair{"b", 202});
  auto mn_copy = copy_field<map<string, integral>>(mn);
  EXPECT_TRUE(deepEqual(mn, mn_copy));

  auto sn = makeValue<NonCopyableF14FastSet<std::string>>("a", "b");
  auto sn_copy = copy_field<set<string>>(sn);
  EXPECT_TRUE(deepEqual(sn, sn_copy));
}
