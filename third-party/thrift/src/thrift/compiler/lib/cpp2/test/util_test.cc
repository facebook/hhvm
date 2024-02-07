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

#include <thrift/compiler/lib/cpp2/util.h>

#include <algorithm>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <folly/Utility.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_paramlist.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/source_location.h>
#include <thrift/compiler/test/gen_testing.h>
#include <thrift/compiler/test/parser_test_helpers.h>

namespace apache::thrift::compiler {
namespace {

class UtilTest : public ::testing::Test {};

TEST_F(UtilTest, is_orderable_set_template) {
  t_set t(&t_base_type::t_double());
  t.set_annotation("cpp2.template", "blah");
  t_program p("path/to/program.thrift");
  t_struct s(&p, "struct_name");
  s.append(std::make_unique<t_field>(&t, "set_field", 1));
  EXPECT_FALSE(cpp2::is_orderable(t));
  EXPECT_FALSE(cpp2::is_orderable(s));
}

TEST_F(UtilTest, is_orderable_struct) {
  t_program p("path/to/program.thrift");
  t_struct s(&p, "struct_name");
  s.append(
      std::make_unique<t_field>(&t_base_type::t_string(), "field_name", 1));
  EXPECT_TRUE(cpp2::is_orderable(s));
}

TEST_F(UtilTest, is_orderable_struct_self_reference) {
  t_program p("path/to/program.thrift");

  t_set t(&t_base_type::t_double());
  t.set_annotation("cpp2.template", "blah");

  t_struct c(&p, "C");
  c.append(std::make_unique<t_field>(&t, "set_field", 1));
  EXPECT_FALSE(cpp2::is_orderable(c));

  t_struct b(&p, "B");
  t_struct a(&p, "A");

  b.append(std::make_unique<t_field>(&a, "a", 1));
  a.append(std::make_unique<t_field>(&b, "b", 1));
  a.append(std::make_unique<t_field>(&c, "c", 2));

  EXPECT_FALSE(cpp2::is_orderable(a));
  EXPECT_FALSE(cpp2::is_orderable(b));

  std::unordered_map<t_type const*, bool> memo;
  EXPECT_FALSE(cpp2::is_orderable(memo, a));
  EXPECT_FALSE(cpp2::is_orderable(memo, b));
  EXPECT_FALSE(cpp2::is_orderable(memo, c));
}

TEST_F(UtilTest, is_eligible_for_constexpr) {
  auto is_eligible_for_constexpr = [](const t_type* t) {
    return cpp2::is_eligible_for_constexpr()(t);
  };
  auto i32 = t_base_type::t_i32();
  EXPECT_TRUE(is_eligible_for_constexpr(&i32));
  EXPECT_TRUE(is_eligible_for_constexpr(&t_base_type::t_double()));
  EXPECT_TRUE(is_eligible_for_constexpr(&t_base_type::t_bool()));
  EXPECT_FALSE(is_eligible_for_constexpr(&t_base_type::t_string()));
  EXPECT_FALSE(is_eligible_for_constexpr(&t_base_type::t_binary()));

  auto list = t_list(&i32);
  EXPECT_FALSE(is_eligible_for_constexpr(&list));

  auto set = t_set(&i32);
  EXPECT_FALSE(is_eligible_for_constexpr(&set));

  auto map = t_map(&i32, &t_base_type::t_double());
  EXPECT_FALSE(is_eligible_for_constexpr(&map));

  for (auto a : {"cpp.indirection"}) {
    auto ref = t_base_type::t_i32();
    ref.set_annotation(a, "true");
    EXPECT_FALSE(is_eligible_for_constexpr(&ref));
  }

  for (auto a : {"cpp.template", "cpp2.template", "cpp.type", "cpp2.type"}) {
    auto type = i32;
    type.set_annotation(a, "custom_int");
    EXPECT_TRUE(is_eligible_for_constexpr(&type)) << a;
  }

  auto program = t_program("path/to/program.thrift");
  {
    auto s = t_struct(&program, "struct_name");
    EXPECT_TRUE(is_eligible_for_constexpr(&s));
  }
  {
    auto s = t_struct(&program, "struct_name");
    s.append(std::make_unique<t_field>(&i32, "field1", 1));
    EXPECT_TRUE(is_eligible_for_constexpr(&s));
  }
  for (auto a : {"cpp.virtual", "cpp2.virtual", "cpp.allocator"}) {
    auto s = t_struct(&program, "struct_name");
    s.set_annotation(a, "true");
    EXPECT_FALSE(is_eligible_for_constexpr(&s)) << a;
  }
  {
    auto s = t_struct(&program, "struct_name");
    s.append(std::make_unique<t_field>(&i32, "field1", 1));
    s.append(std::make_unique<t_field>(&set, "field2", 2));
    EXPECT_FALSE(is_eligible_for_constexpr(&s));
  }
  for (auto a : {"cpp.ref", "cpp2.ref"}) {
    auto s = t_struct(&program, "struct_name");
    auto field = std::make_unique<t_field>(&i32, "field1", 1);
    field->set_annotation(a, "true");
    s.append(std::move(field));
    EXPECT_FALSE(is_eligible_for_constexpr(&s)) << a;
  }
  for (auto a : {"cpp.ref_type", "cpp2.ref_type"}) {
    auto s = t_struct(&program, "struct_name");
    auto field = std::make_unique<t_field>(&i32, "field1", 1);
    field->set_annotation(a, "unique");
    s.append(std::move(field));
    EXPECT_FALSE(is_eligible_for_constexpr(&s)) << a;
  }

  auto u = t_union(&program, "union_name");
  EXPECT_FALSE(is_eligible_for_constexpr(&u));
  auto e = t_exception(&program, "exception_name");
  EXPECT_FALSE(is_eligible_for_constexpr(&e));
}

TEST_F(UtilTest, for_each_transitive_field) {
  auto program = t_program("path/to/program.thrift");
  auto empty = t_struct(&program, "struct_name");
  cpp2::for_each_transitive_field(&empty, [](const t_field*) {
    ADD_FAILURE();
    return true;
  });
  //            a
  //           / \
  //          b   c
  //         / \
  //        d   e
  //             \
  //              f
  auto i32 = t_base_type::t_i32();
  auto a = t_struct(&program, "a");
  auto b = t_struct(&program, "b");
  auto e = t_struct(&program, "e");
  a.append(std::make_unique<t_field>(&b, "b", 1));
  a.append(std::make_unique<t_field>(&i32, "c", 2));
  b.append(std::make_unique<t_field>(&i32, "d", 1));
  b.append(std::make_unique<t_field>(&e, "e", 2));
  e.append(std::make_unique<t_field>(&i32, "f", 1));

  auto fields = std::vector<std::string>();
  cpp2::for_each_transitive_field(&a, [&](const t_field* f) {
    fields.push_back(f->get_name());
    return true;
  });
  EXPECT_THAT(fields, testing::ElementsAreArray({"b", "d", "e", "f", "c"}));

  fields = std::vector<std::string>();
  cpp2::for_each_transitive_field(&a, [&](const t_field* f) {
    auto name = f->get_name();
    fields.push_back(name);
    return name != "e"; // Stop at e.
  });
  EXPECT_THAT(fields, testing::ElementsAreArray({"b", "d", "e"}));

  auto depth = 1'000'000;
  auto structs = std::vector<std::unique_ptr<t_struct>>();
  structs.reserve(depth);
  structs.push_back(std::make_unique<t_paramlist>(&program));
  for (int i = 1; i < depth; ++i) {
    structs.push_back(std::make_unique<t_paramlist>(&program));
    structs[i - 1]->append(
        std::make_unique<t_field>(structs[i].get(), "field", 1));
  }
  auto count = 0;
  cpp2::for_each_transitive_field(structs.front().get(), [&](const t_field*) {
    ++count;
    return true;
  });
  EXPECT_EQ(count, depth - 1);
}

TEST_F(UtilTest, field_transitively_refers_to_unique) {
  auto i = t_base_type::t_i32();
  auto li = t_list(&i);
  auto lli = t_list(t_list(&i));
  auto si = t_set(&i);
  auto mii = t_map(&i, &i);

  const t_type* no_uniques[] = {&i, &li, &lli, &si, &mii};
  for (const auto* no_unique : no_uniques) {
    // no_unique f;
    auto f = t_field(no_unique, "f", 1);
    EXPECT_FALSE(cpp2::field_transitively_refers_to_unique(&f));
  }

  // typedef binary (cpp.type = "std::unique_ptr<folly::IOBuf>") IOBufPtr;
  auto p = t_base_type::t_binary();
  p.set_annotation("cpp.type", "std::unique_ptr<folly::IOBuf>");

  auto lp = t_list(&p);
  auto llp = t_list(t_list(&p));
  auto sp = t_set(&p);
  auto mip = t_map(&i, &p);

  const t_type* uniques[] = {&p, &lp, &llp, &sp, &mip};
  for (const auto* unique : uniques) {
    // unique f;
    auto f = t_field(unique, "f", 1);
    EXPECT_TRUE(cpp2::field_transitively_refers_to_unique(&f));
  }

  const t_type* types[] = {&i, &li, &si, &mii, &p, &lp, &sp, &mip};
  for (const auto* type : types) {
    // type r (cpp.ref = "true");
    auto r = t_field(type, "r", 1);
    r.set_annotation("cpp.ref", "true");
    EXPECT_TRUE(cpp2::field_transitively_refers_to_unique(&r));

    // type u (cpp.ref_type = "unique");
    auto u = t_field(type, "u", 1);
    u.set_annotation("cpp.ref_type", "unique");
    EXPECT_TRUE(cpp2::field_transitively_refers_to_unique(&u));

    // type s (cpp.ref_type = "shared");
    auto s = t_field(type, "s", 1);
    s.set_annotation("cpp.ref_type", "shared");
    EXPECT_FALSE(cpp2::field_transitively_refers_to_unique(&s));
  }
}

TEST_F(UtilTest, get_gen_type_class) {
  // a single example as demo
  EXPECT_EQ(
      "::apache::thrift::type_class::string",
      cpp2::get_gen_type_class(t_base_type::t_string()));
}

TEST_F(UtilTest, is_custom_type) {
  t_program p("path/to/program.thrift");

  {
    auto cppType = t_base_type::t_string();
    auto typeDef = t_typedef(&p, "Type", cppType);
    EXPECT_FALSE(cpp2::is_custom_type(cppType));
    EXPECT_FALSE(cpp2::is_custom_type(typeDef));
    cppType.set_annotation("cpp.type", "folly::fbstring");
    EXPECT_TRUE(cpp2::is_custom_type(cppType));
    EXPECT_TRUE(cpp2::is_custom_type(typeDef));
  }

  {
    auto cpp2Type = t_base_type::t_string();
    auto typeDef = t_typedef(&p, "Type", cpp2Type);
    EXPECT_FALSE(cpp2::is_custom_type(cpp2Type));
    EXPECT_FALSE(cpp2::is_custom_type(typeDef));
    cpp2Type.set_annotation("cpp2.type", "folly::fbstring");
    EXPECT_TRUE(cpp2::is_custom_type(cpp2Type));
    EXPECT_TRUE(cpp2::is_custom_type(typeDef));
  }

  const auto i32 = t_base_type::t_i32();
  {
    auto cppTemplate = t_list(&i32);
    auto typeDef = t_typedef(&p, "Type", cppTemplate);
    EXPECT_FALSE(cpp2::is_custom_type(cppTemplate));
    EXPECT_FALSE(cpp2::is_custom_type(typeDef));
    cppTemplate.set_annotation("cpp.template", "std::deque");
    EXPECT_TRUE(cpp2::is_custom_type(cppTemplate));
    EXPECT_TRUE(cpp2::is_custom_type(typeDef));
  }

  {
    auto cpp2Template = t_list(&i32);
    auto typeDef = t_typedef(&p, "Type", cpp2Template);
    EXPECT_FALSE(cpp2::is_custom_type(cpp2Template));
    EXPECT_FALSE(cpp2::is_custom_type(typeDef));
    cpp2Template.set_annotation("cpp2.template", "std::deque");
    EXPECT_TRUE(cpp2::is_custom_type(cpp2Template));
    EXPECT_TRUE(cpp2::is_custom_type(typeDef));
  }

  {
    auto cppAdapter = i32;
    auto typeDef = t_typedef(&p, "Type", cppAdapter);
    auto typeDef2 = t_typedef(&p, "TypeDef", typeDef);
    EXPECT_FALSE(cpp2::is_custom_type(cppAdapter));
    EXPECT_FALSE(cpp2::is_custom_type(typeDef));
    typeDef.set_annotation("cpp.adapter", "Adapter");
    auto adapter = gen::adapter_builder(p, "cpp");
    typeDef.add_structured_annotation(adapter.make("MyAdapter"));
    EXPECT_TRUE(cpp2::is_custom_type(typeDef));
    EXPECT_TRUE(cpp2::is_custom_type(typeDef2));
  }
}

TEST_F(UtilTest, topological_sort) {
  std::map<std::string, std::vector<std::string>> graph{
      {"e", {"c", "a"}},
      {"d", {"b", "c"}},
      {"c", {"d", "b", "a"}},
      {"b", {}},
      {"a", {"b"}},
  };
  std::vector<std::string> vertices;
  vertices.reserve(graph.size());
  for (const auto& kvp : graph) {
    vertices.push_back(kvp.first);
  }
  auto result = cpp2::topological_sort<std::string>(
      vertices.begin(), vertices.end(), graph);
  EXPECT_EQ(std::vector<std::string>({"b", "a", "d", "c", "e"}), result);
}

TEST_F(UtilTest, lpt_slit) {
  const std::vector<size_t> in{1, 4, 8, 7, 2, 2, 9, 5, 6, 10, 20, 1, 8};

  // split 1
  auto res1 = cpp2::lpt_split(in, 1, folly::identity);
  std::vector<std::vector<size_t>> res1_expected{
      {20, 10, 9, 8, 8, 7, 6, 5, 4, 2, 2, 1, 1}};
  EXPECT_EQ(res1, res1_expected);

  // split 2
  auto res2 = cpp2::lpt_split(in, 2, folly::identity);
  std::vector<std::vector<size_t>> res2_expected = {
      {10, 9, 8, 7, 5, 2, 1}, {20, 8, 6, 4, 2, 1}};
  EXPECT_EQ(res2, res2_expected);

  // split 3
  auto res3 = cpp2::lpt_split(in, 3, folly::identity);
  std::vector<std::vector<size_t>> res3_expected = {
      {9, 8, 7, 4}, {10, 8, 6, 2, 1}, {20, 5, 2, 1}};
  EXPECT_EQ(res3, res3_expected);

  // split 4
  auto res4 = cpp2::lpt_split(in, 4, folly::identity);
  std::vector<std::vector<size_t>> res4_expected = {
      {8, 8, 5}, {9, 7, 4, 1}, {10, 6, 2, 2}, {20, 1}};
  EXPECT_EQ(res4, res4_expected);
}

TEST_F(UtilTest, get_internal_injected_field_id) {
  EXPECT_THROW(cpp2::get_internal_injected_field_id(-1), std::runtime_error);
  EXPECT_EQ(cpp2::get_internal_injected_field_id(0), -1000);
  EXPECT_EQ(cpp2::get_internal_injected_field_id(999), -1999);
  EXPECT_THROW(cpp2::get_internal_injected_field_id(1000), std::runtime_error);
}

TEST_F(UtilTest, gen_adapter_dependency_graph) {
  t_program p("path/to/program.thrift");
  std::mt19937 gen;
  auto test = [&](std::string name, std::vector<const t_type*> expected) {
    constexpr int kIters = 42;
    for (int iter = 0; iter < kIters; iter++) {
      std::vector<const t_type*> objects = expected;
      std::vector<t_typedef*> typedefs;
      for (auto type : objects) {
        if (auto typedf = dynamic_cast<t_typedef const*>(type)) {
          typedefs.push_back(const_cast<t_typedef*>(typedf));
        }
      }
      std::shuffle(objects.begin(), objects.end(), gen);
      auto deps = cpp2::gen_dependency_graph(&p, objects);
      auto sorted_objects = cpp2::topological_sort<const t_type*>(
          objects.begin(), objects.end(), deps);
      ASSERT_EQ(sorted_objects.size(), expected.size()) << name;
      for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(sorted_objects[i], expected[i])
            << name << ": got " << sorted_objects[i]->name() << " instead of "
            << expected[i]->name();
      }
    }
  };

  auto adapter = gen::adapter_builder(p, "cpp");
  t_struct s1(&p, "struct");
  t_typedef t1(&p, "typedef_with_adapter", s1);
  t1.add_structured_annotation(adapter.make("MyAdapter"));
  test("adapted typedef after struct", {&s1, &t1});

  t_struct f2(nullptr, "foreign_struct");
  t_typedef t2(&p, "typedef_with_adapter", f2);
  t2.add_structured_annotation(adapter.make("MyAdapter"));
  t_struct s2(&p, "dependent_struct");
  s2.append_field(std::make_unique<t_field>(t2, "field"));
  test("dependent struct after typedef", {&t2, &s2});

  t_union u3(&p, "union");
  t_typedef t3(&p, "typedef_with_adapter", u3);
  t3.add_structured_annotation(adapter.make("MyAdapter"));
  test("adapted typedef after union", {&u3, &t3});

  t_typedef t4(&p, "typedef_with_adapter", f2);
  t4.add_structured_annotation(adapter.make("MyAdapter"));
  t_exception e4(&p, "exception");
  e4.append_field(std::make_unique<t_field>(t4, "field"));
  test("dependent exception after typedef", {&t4, &e4});

  t_typedef t5(&p, "typedef_of_typedef", t2);
  test("dependent typedef after typedef", {&t2, &t5});
}

TEST_F(UtilTest, simple_struct_dependency_graph) {
  auto source_mgr = source_manager();
  auto program = dedent_and_parse_to_program(source_mgr, R"(
    struct FirstStruct {
      1: string one;
      2: MissingType two;
    }
    struct SecondStruct {
      1: string another;
      2: FirstStruct referent;
    }
  )");

  std::vector<const t_type*> objects(
      program->structured_definitions().begin(),
      program->structured_definitions().end());
  auto edges = cpp2::gen_dependency_graph(program.get(), objects);

  // We should really define some sort of "EXPECT_GRAPH_ISOMORPHIC" primitive,
  // but since that's hard we'll just use a simplistic/brute-force approach.
  EXPECT_EQ(edges.size(), 2);

  // Technically redundant, but making explicit
  const t_type* first_node = nullptr;
  const t_type* second_node = nullptr;

  for (const auto& vertex_with_destinations : edges) {
    const std::string& name = vertex_with_destinations.first->name();
    if (name == "FirstStruct") {
      first_node = vertex_with_destinations.first;
    } else if (name == "SecondStruct") {
      second_node = vertex_with_destinations.first;
    } else {
      throw std::runtime_error("Wrong graph node: " + name);
    }
  }

  EXPECT_TRUE(first_node != nullptr);
  EXPECT_TRUE(second_node != nullptr);

  EXPECT_TRUE(edges.at(first_node).empty());
  ASSERT_THAT(edges.at(second_node), testing::ElementsAre(first_node));
}

// This is the same test as the above, but where one of the structures
// has a type that doesn't actually resolve.  The same dep graph should
// be generated without crashing.
TEST_F(UtilTest, struct_dependency_graph_with_bad_type) {
  auto source_mgr = source_manager();
  auto program = dedent_and_parse_to_program(source_mgr, R"(
    struct FirstStruct {
      1: string one;
      2: MissingType two;
    }
    struct SecondStruct {
      1: string another;
      2: FirstStruct referent;
    }
  )");

  std::vector<const t_type*> objects(
      program->structured_definitions().begin(),
      program->structured_definitions().end());
  auto edges = cpp2::gen_dependency_graph(program.get(), objects);

  // We should really define some sort of "EXPECT_GRAPH_ISOMORPHIC" primitive,
  // but since that's hard we'll just use a simplistic/brute-force approach.
  EXPECT_EQ(edges.size(), 2);

  // Technically redundant, but making explicit
  const t_type* first_node = nullptr;
  const t_type* second_node = nullptr;

  for (const auto& vertex_with_destinations : edges) {
    const std::string& name = vertex_with_destinations.first->name();
    if (name == "FirstStruct") {
      first_node = vertex_with_destinations.first;
    } else if (name == "SecondStruct") {
      second_node = vertex_with_destinations.first;
    } else {
      throw std::runtime_error("Wrong graph node: " + name);
    }
  }

  EXPECT_TRUE(first_node != nullptr);
  EXPECT_TRUE(second_node != nullptr);

  EXPECT_TRUE(edges.at(first_node).empty());
  ASSERT_THAT(edges.at(second_node), testing::ElementsAre(first_node));
}

TEST_F(UtilTest, structs_and_typedefs_dependency_graph) {
  auto source_mgr = source_manager();
  auto program = dedent_and_parse_to_program(source_mgr, R"(
    struct ContainsList {
      1: list<S> (cpp.template = "dependent") l;
    }
    struct TransitiveContainsDependentList {
      1: Dependent l;
    }
    struct TransitiveContainsList {
      1: Independent l;
    }
    struct TransitiveContainsStruct {
      1: AlsoS s;
    }
    struct TransitiveContainsStructRef {
      1: AlsoS s (cpp.ref);
    }
    typedef list<S> (cpp.template = "dependent") Dependent
    typedef list<S> Independent
    typedef S AlsoS

    struct S {}
  )");

  std::vector<const t_type*> objects(
      program->structured_definitions().begin(),
      program->structured_definitions().end());
  objects.insert(
      objects.end(), program->typedefs().begin(), program->typedefs().end());
  auto edges = cpp2::gen_dependency_graph(program.get(), objects);

  EXPECT_EQ(edges.size(), 9);

  for (const auto& [node, deps] : edges) {
    const std::string& name = node->name();
    if (name == "S") {
      EXPECT_EQ(deps.size(), 0);
    } else if (name == "ContainsList") {
      EXPECT_EQ(deps.size(), 1);
      EXPECT_EQ(deps.at(0)->name(), "S");
    } else if (name == "TransitiveContainsList") {
      EXPECT_EQ(deps.size(), 1);
      EXPECT_EQ(deps.at(0)->name(), "Independent");
    } else if (name == "TransitiveContainsDependentList") {
      EXPECT_EQ(deps.size(), 2);
      EXPECT_EQ(deps.at(0)->name(), "Dependent");
      EXPECT_EQ(deps.at(1)->name(), "S");
    } else if (name == "TransitiveContainsStruct") {
      EXPECT_EQ(deps.size(), 2);
      EXPECT_EQ(deps.at(0)->name(), "AlsoS");
      EXPECT_EQ(deps.at(1)->name(), "S");
    } else if (name == "TransitiveContainsStructRef") {
      EXPECT_EQ(deps.size(), 1);
      EXPECT_EQ(deps.at(0)->name(), "AlsoS");
    } else if (name == "Dependent") {
      EXPECT_EQ(deps.size(), 0);
    } else if (name == "Independent") {
      EXPECT_EQ(deps.size(), 0);
    } else if (name == "AlsoS") {
      EXPECT_EQ(deps.size(), 0);
    } else {
      FAIL() << "Wrong graph node: " << name;
    }
  }
}

} // namespace
} // namespace apache::thrift::compiler
