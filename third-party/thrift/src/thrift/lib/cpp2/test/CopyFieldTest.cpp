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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/TypeClass.h>

using namespace apache::thrift;

// Fields without any unique-ness.
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
}

//
TEST(CopyUniqueTest, Unique) {
  auto v = std::vector<std::unique_ptr<int>>{};
  v.push_back(std::make_unique<int>(101));
  v.push_back(std::make_unique<int>(202));
  auto v_copy =
      detail::st::copy_field<type_class::list<type_class::integral>>(v);
  static_assert(std::is_same_v<decltype(v), decltype(v_copy)>);
  auto v_iter = v.begin();
  auto v_copy_iter = v_copy.begin();
  for (; v_iter != v.end() && v_copy_iter != v_copy.end();
       ++v_iter, ++v_copy_iter) {
    EXPECT_EQ(**v_iter, **v_copy_iter);
    EXPECT_NE(v_iter, v_copy_iter);
  }
  EXPECT_NE(&v, &v_copy);

  auto s = std::set<std::unique_ptr<int>>{};
  s.insert(std::make_unique<int>(101));
  s.insert(std::make_unique<int>(202));
  auto s_copy =
      detail::st::copy_field<type_class::set<type_class::integral>>(s);
  static_assert(std::is_same_v<decltype(s), decltype(s_copy)>);
  auto s_iter = s.begin();
  auto s_copy_iter = s_copy.begin();
  for (; s_iter != s.end() && s_copy_iter != s_copy.end();
       ++s_iter, ++s_copy_iter) {
    EXPECT_EQ(**s_iter, **s_copy_iter);
    EXPECT_NE(s_iter, s_copy_iter);
  }
  EXPECT_NE(&s, &s_copy);

  auto m = std::map<int, std::unique_ptr<int>>{};
  m.emplace(0, std::make_unique<int>(101));
  m.emplace(1, std::make_unique<int>(202));
  auto m_copy = detail::st::copy_field<
      type_class::map<type_class::integral, type_class::integral>>(m);
  static_assert(std::is_same_v<decltype(m), decltype(m_copy)>);
  auto m_iter = m.begin();
  auto m_copy_iter = m_copy.begin();
  for (; m_iter != m.end() && m_copy_iter != m_copy.end();
       ++m_iter, ++m_copy_iter) {
    EXPECT_EQ(m_iter->first, m_copy_iter->first);
    EXPECT_NE(&m_iter->first, &m_copy_iter->first);
    EXPECT_EQ(*m_iter->second, *m_copy_iter->second);
    EXPECT_NE(m_iter->second, m_copy_iter->second);
  }
  EXPECT_NE(&m, &m_copy);
}

// Field-level annotations such as `cpp.ref`, `cpp.ref_type = "unique"`.
TEST(CopyUniqueTest, UniqueRef) {
  auto pi = std::make_unique<int>(42);
  auto pi_copy = detail::st::copy_field<type_class::integral>(pi);
  static_assert(std::is_same_v<decltype(pi), decltype(pi_copy)>);
  EXPECT_EQ(*pi, *pi_copy);
  EXPECT_NE(pi, pi_copy);

  auto ps = std::make_unique<std::string>("foo");
  auto ps_copy = detail::st::copy_field<type_class::string>(ps);
  static_assert(std::is_same_v<decltype(ps), decltype(ps_copy)>);
  EXPECT_EQ(*ps, *ps_copy);
  EXPECT_NE(ps, ps_copy);

  auto pv = std::make_unique<std::vector<int>>(std::vector<int>{1, 2, 3});
  auto pv_copy =
      detail::st::copy_field<type_class::list<type_class::integral>>(pv);
  static_assert(std::is_same_v<decltype(pv), decltype(pv_copy)>);
  EXPECT_EQ(*pv, *pv_copy);
  EXPECT_NE(pv, pv_copy);

  auto pvp = std::make_unique<std::vector<std::unique_ptr<int>>>();
  pvp->push_back(std::make_unique<int>(101));
  pvp->push_back(std::make_unique<int>(202));
  auto pvp_copy =
      detail::st::copy_field<type_class::list<type_class::integral>>(pvp);
  static_assert(std::is_same_v<decltype(pvp), decltype(pvp_copy)>);
  auto pvp_iter = pvp->begin();
  auto pvp_copy_iter = pvp_copy->begin();
  for (; pvp_iter != pvp->end() && pvp_copy_iter != pvp_copy->end();
       ++pvp_iter, ++pvp_copy_iter) {
    EXPECT_EQ(**pvp_iter, **pvp_copy_iter);
    EXPECT_NE(pvp_iter, pvp_copy_iter);
  }
  EXPECT_NE(&pvp, &pvp_copy);

  auto psp = std::make_unique<std::set<std::unique_ptr<int>>>();
  psp->insert(std::make_unique<int>(101));
  psp->insert(std::make_unique<int>(202));
  auto psp_copy =
      detail::st::copy_field<type_class::set<type_class::integral>>(psp);
  static_assert(std::is_same_v<decltype(psp), decltype(psp_copy)>);
  auto psp_iter = psp->begin();
  auto psp_copy_iter = psp_copy->begin();
  for (; psp_iter != psp->end() && psp_copy_iter != psp_copy->end();
       ++psp_iter, ++psp_copy_iter) {
    EXPECT_EQ(**psp_iter, **psp_copy_iter);
    EXPECT_NE(psp_iter, psp_copy_iter);
  }
  EXPECT_NE(&psp, &psp_copy);

  auto pmp = std::make_unique<std::map<int, std::unique_ptr<int>>>();
  pmp->emplace(0, std::make_unique<int>(101));
  pmp->emplace(1, std::make_unique<int>(202));
  auto pmp_copy = detail::st::copy_field<
      type_class::map<type_class::integral, type_class::integral>>(pmp);
  static_assert(std::is_same_v<decltype(pmp), decltype(pmp_copy)>);
  auto pmp_iter = pmp->begin();
  auto pmp_copy_iter = pmp_copy->begin();
  for (; pmp_iter != pmp->end() && pmp_copy_iter != pmp_copy->end();
       ++pmp_iter, ++pmp_copy_iter) {
    EXPECT_EQ(pmp_iter->first, pmp_copy_iter->first);
    EXPECT_NE(&pmp_iter->first, &pmp_copy_iter->first);
    EXPECT_EQ(*pmp_iter->second, *pmp_copy_iter->second);
    EXPECT_NE(pmp_iter->second, pmp_copy_iter->second);
  }
  EXPECT_NE(&pmp, &pmp_copy);
}
