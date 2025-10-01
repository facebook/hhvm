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

#include <thrift/compiler/parse/parse_ast.h>

#include <optional>

#include <gtest/gtest.h>

using namespace apache::thrift::compiler;

TEST(ParserTest, type_resolution) {
  auto source_mgr = source_manager();
  source_mgr.add_virtual_file("test.thrift", R"(
    struct S {
      1: MissingType field;
    }
  )");
  auto diag = std::optional<diagnostic>();
  auto diags =
      diagnostics_engine(source_mgr, [&diag](diagnostic d) { diag = d; });

  // Types must be resolved in parse_ast.
  auto programs = parse_ast(source_mgr, diags, "test.thrift", {});
  EXPECT_TRUE(diags.has_errors());
  EXPECT_EQ(diag->message(), "Type `MissingType` not defined.");

  // Programs must be non-null even in case of an error.
  EXPECT_TRUE(programs != nullptr);
}

TEST(ParserTest, missing_includes) {
  auto source_mgr = source_manager();
  source_mgr.add_virtual_file("test.thrift", R"(
    package "facebook.com/thrift/test"
    include "nonexist.thrift"
  )");

  {
    int count = 0;
    auto diags = diagnostics_engine(source_mgr, [&count](diagnostic d) {
      EXPECT_EQ(d.level(), diagnostic_level::error);
      EXPECT_EQ(d.message(), "Could not find include file nonexist.thrift");
      ++count;
    });
    auto programs = parse_ast(source_mgr, diags, "test.thrift", {});
    EXPECT_EQ(programs, nullptr);
    EXPECT_TRUE(diags.has_errors());
    EXPECT_EQ(count, 1);
  }
  {
    int count = 0;
    auto diags = diagnostics_engine(source_mgr, [&count](diagnostic d) {
      EXPECT_EQ(d.level(), diagnostic_level::warning);
      // We should get both errors since we did not end parsing on first error.
      if (count == 0) {
        EXPECT_EQ(d.message(), "Could not find include file nonexist.thrift");
      } else {
        EXPECT_EQ(d.message(), "failed to open file: nonexist.thrift");
      }
      ++count;
    });
    parsing_params params;
    params.allow_missing_includes = true;
    auto programs = parse_ast(source_mgr, diags, "test.thrift", params);
    EXPECT_NE(programs, nullptr);
    EXPECT_FALSE(diags.has_errors());
    EXPECT_EQ(count, 2);
  }
}

TEST(ParserTest, include_paths) {
  auto source_mgr = source_manager();
  source_mgr.add_virtual_file("some/path/test.thrift", R"(
    struct S {
    }
  )");
  source_mgr.add_virtual_file("middleFile.thrift", R"(
    include "test.thrift"
  )");
  source_mgr.add_virtual_file("rootFile.thrift", R"(
    include "middleFile.thrift"
    include "some/path/test.thrift"
  )");
  auto diag = std::optional<diagnostic>();
  auto diags =
      diagnostics_engine(source_mgr, [&diag](diagnostic d) { diag = d; });

  parsing_params params;
  params.incl_searchpath.emplace_back("some/path/");

  // Types must be resolved in parse_ast.
  auto programs = parse_ast(source_mgr, diags, "rootFile.thrift", params);

  EXPECT_FALSE(diags.has_errors());
  EXPECT_TRUE(programs != nullptr);

  EXPECT_EQ(programs->programs().size(), 3);
}

TEST(ParserTest, struct_doc) {
  auto source_mgr = source_manager();
  source_mgr.add_virtual_file("test.thrift", R"(
    /// struct doc
    struct S {
      /// field doc
      1: i32 field1;
      /**
       * multiline doc
       */
      2: i32 field2;
      3: i32 field3; // ignored
    }
  )");
  auto diag = std::optional<diagnostic>();
  auto diags =
      diagnostics_engine(source_mgr, [&diag](diagnostic d) { diag = d; });

  // No parsing errors
  auto programs = parse_ast(source_mgr, diags, "test.thrift", {});
  EXPECT_FALSE(diags.has_errors());

  auto s = programs->root_program()->structs_and_unions()[0];
  EXPECT_EQ(s->doc(), "struct doc\n");
  EXPECT_EQ(s->fields()[0].doc(), "field doc\n");
  EXPECT_EQ(s->fields()[1].doc(), "multiline doc\n");
  EXPECT_FALSE(s->fields()[2].has_doc());
}

TEST(ParserTest, enum_doc) {
  auto source_mgr = source_manager();
  source_mgr.add_virtual_file("test.thrift", R"(
    /// enum doc
    enum E {
      /// value doc
      A = 1,
      /**
       * multiline doc
       */
      B = 2,
      C = 3, // ignored
    }
  )");
  auto diag = std::optional<diagnostic>();
  auto diags =
      diagnostics_engine(source_mgr, [&diag](diagnostic d) { diag = d; });

  // No parsing errors
  auto programs = parse_ast(source_mgr, diags, "test.thrift", {});
  EXPECT_FALSE(diags.has_errors());

  auto e = programs->root_program()->enums()[0];
  EXPECT_EQ(e->doc(), "enum doc\n");
  EXPECT_EQ(e->values()[0].doc(), "value doc\n");
  EXPECT_EQ(e->values()[1].doc(), "multiline doc\n");
  EXPECT_FALSE(e->values()[2].has_doc());
}

TEST(ParserTest, struct_annotation) {
  auto source_mgr = source_manager();
  source_mgr.add_virtual_file("test.thrift", R"(
    include "thrift/annotation/rust.thrift"

    struct S {
      @rust.Type{name = "Foo"}
      1: i32 field1;
      2: i32 field2;
      3: i32 field3;
    }
  )");
  auto diag = std::optional<diagnostic>();
  auto diags =
      diagnostics_engine(source_mgr, [&diag](diagnostic d) { diag = d; });

  parsing_params params;
  if (auto* includes = std::getenv("IMPLICIT_INCLUDES")) {
    params.incl_searchpath.emplace_back(includes);
  }
  auto programs = parse_ast(source_mgr, diags, "test.thrift", params);
  EXPECT_FALSE(diags.has_errors());

  auto& s = programs->root_program()->structs_and_unions()[0];
  EXPECT_EQ(s->fields()[0].structured_annotations().size(), 1);
  EXPECT_EQ(
      s->fields()[0].structured_annotations()[0].type()->uri(),
      "facebook.com/thrift/annotation/rust/Type");

  auto a = s->fields()[0].find_structured_annotation_or_null(
      "facebook.com/thrift/annotation/rust/Type");
  EXPECT_TRUE(a != nullptr);
  auto v = s->fields()[0].structured_annotations()[0].value();
  auto& m = v->get_map();
  ASSERT_EQ(m.size(), 1);
  ASSERT_EQ(m[0].first->get_string(), "name");
  ASSERT_EQ(m[0].second->get_string(), "Foo");
}

TEST(ParserTest, typedef_uri_requires_annotation) {
  source_manager source_mgr;

  // Virtual file without typedef explicit URIs - should have no parse errors
  source_mgr.add_virtual_file("test.thrift", R"(
    package "meta.com/thrift/test"

    include "thrift/annotation/thrift.thrift"

    struct MyStruct {}

    typedef MyStruct MyTypedef

    @thrift.AllowLegacyTypedefUri
    typedef MyStruct MyLegacyTypedef
  )");

  // Virtual file with @thrift.Uri annotations - will have parse errors due to
  // typedef explicit URIs being an error level validation
  source_mgr.add_virtual_file("test_explicit.thrift", R"(
    package "meta.com/thrift/test"

    include "thrift/annotation/thrift.thrift"
    include "test.thrift"

    @thrift.Uri{value="meta.com/thrift/test_explicit/ExplicitTypedef"}
    typedef test.MyStruct MyExplicitTypedef

    @thrift.AllowLegacyTypedefUri
    @thrift.Uri{value="meta.com/thrift/test_explicit/ExplicitLegacyTypedef"}
    typedef test.MyStruct MyExplicitLegacyTypedef
  )");

  parsing_params params;
  if (char* includes = std::getenv("IMPLICIT_INCLUDES")) {
    params.incl_searchpath.emplace_back(includes);
  }

  const auto expect_name_and_uri = [](const t_type* type,
                                      const std::string_view name,
                                      const std::string& uri) {
    EXPECT_EQ(name, type->get_scoped_name());
    EXPECT_EQ(uri, type->uri());
  };

  {
    diagnostics_engine diags(source_mgr, [](const diagnostic&) {});
    std::unique_ptr<t_program_bundle> bundle =
        parse_ast(source_mgr, diags, "test.thrift", params);
    EXPECT_FALSE(diags.has_errors());
    EXPECT_NE(nullptr, bundle);

    expect_name_and_uri(
        bundle->root_program()->structs_and_unions()[0],
        "test.MyStruct",
        "meta.com/thrift/test/MyStruct");

    expect_name_and_uri(
        bundle->root_program()->typedefs()[0], "test.MyTypedef", "");

    expect_name_and_uri(
        bundle->root_program()->typedefs()[1],
        "test.MyLegacyTypedef",
        "meta.com/thrift/test/MyLegacyTypedef");
  }

  // Parse with extra validation - expecting an error diagnostic from the
  // explicit typedef URI
  {
    diagnostics_engine diags(source_mgr, [](const diagnostic&) {});
    std::unique_ptr<t_program_bundle> bundle =
        parse_ast(source_mgr, diags, "test_explicit.thrift", params);
    EXPECT_TRUE(diags.has_errors());
    EXPECT_NE(nullptr, bundle);

    expect_name_and_uri(
        bundle->root_program()->typedefs()[0],
        "test_explicit.MyExplicitTypedef",
        "");

    expect_name_and_uri(
        bundle->root_program()->typedefs()[0]->get_true_type(),
        "test.MyStruct",
        "meta.com/thrift/test/MyStruct");

    expect_name_and_uri(
        bundle->root_program()->typedefs()[1],
        "test_explicit.MyExplicitLegacyTypedef",
        "meta.com/thrift/test_explicit/ExplicitLegacyTypedef");

    expect_name_and_uri(
        bundle->root_program()->typedefs()[1]->get_true_type(),
        "test.MyStruct",
        "meta.com/thrift/test/MyStruct");
  }
}

TEST(ParserTest, unresolved_include_circular_references) {
  source_manager source_mgr;
  source_mgr.add_virtual_file("test.thrift", R"(
    include "some/missing/test.thrift"

    typedef test.Foo Foo;

    struct MyStruct {
      1: Foo field = Foo.MY_DEFAULT;
    }

    service MyService {
      Foo func();
    }

    const list<Foo> MY_CONSTANT = test.MY_CONSTANT;
  )");
  std::optional<diagnostic> diag;
  diagnostics_engine diags(source_mgr, [&diag](diagnostic d) { diag = d; });

  parsing_params pparams;
  pparams.allow_missing_includes = true;
  std::unique_ptr<t_program_bundle> programs =
      parse_ast(source_mgr, diags, "test.thrift", pparams);
  EXPECT_TRUE(diags.has_errors());
  EXPECT_TRUE(diag.has_value());
  EXPECT_EQ("Circular typedef: Foo --> Foo", diag->message());
}
