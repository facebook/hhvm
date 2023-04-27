/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include "m_ctype.h"
#include "sql/json_dom.h"
#include "sql/json_path.h"
#include "sql_string.h"
#include "unittest/gunit/test_utils.h"

/**
 Test json path abstraction.
 */
namespace json_path_unittest {

class JsonPathTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }
  THD *thd() const { return initializer.thd(); }
  my_testing::Server_initializer initializer;
  void vet_wrapper_seek(Json_wrapper *wrapper, const Json_path &path,
                        const std::string &expected, bool expected_null) const;
  void vet_wrapper_seek(const char *json_text, const char *path_text,
                        const std::string &expected, bool expected_null) const;
};

/**
  Struct that defines input and expected result for negative testing
  of path parsing.
*/
struct Bad_path {
  const char *m_path_expression;  ///< the path to parse
  const size_t m_expected_index;  ///< the offset of the syntax error
};

/**
  Class that contains parameterized test cases for bad paths.
*/
class JsonBadPathTestP : public ::testing::TestWithParam<Bad_path> {};

/**
  Struct that defines input and expected result for positive testing
  of path parsing.
*/
struct Good_path {
  const char *m_path_expression;  ///< the path to parse
  const char *m_expected_path;    ///< expected canonical path
};

/**
  Struct that defines input and expected result for testing
  Json_dom.get_location().
*/
struct Location_tuple {
  const char *m_json_text;        // the document text
  const char *m_path_expression;  // the path to parse
};

/**
  Struct that defines input and expected result for testing
  the only_needs_one argument of Json_wrapper.seek().
*/
struct Ono_tuple {
  const char *m_json_text;        // the document text
  const char *m_path_expression;  // the path to parse
  const uint m_expected_hits;     // total number of matches
};

/**
  Struct that defines input for cloning test cases.
*/
struct Clone_tuple {
  const char *m_path_expression_1;  // the first path to parse
  const char *m_path_expression_2;  // the second path to parse
};

/**
  Class that contains parameterized test cases for good paths.
*/
class JsonGoodPathTestP : public ::testing::TestWithParam<Good_path> {};

/**
  Class that contains parameterized test cases for dom locations.
*/
class JsonGoodLocationTestP : public ::testing::TestWithParam<Location_tuple> {
};

/**
  Class that contains parameterized test cases for the only_needs_one
  arg of Json_wrapper.seek().
*/
class JsonGoodOnoTestP : public ::testing::TestWithParam<Ono_tuple> {
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }
  my_testing::Server_initializer initializer;

 protected:
  THD *thd() const { return initializer.thd(); }
};

/**
  Class that contains parameterized test cases for cloning tests.
*/
class JsonGoodCloneTestP : public ::testing::TestWithParam<Clone_tuple> {
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }
  my_testing::Server_initializer initializer;
};

/*
  Helper functions.
*/

/* Concatenate the left and right strings and write the result into dest */
char *concat(char *dest, const char *left, const char *right) {
  dest[0] = 0;
  std::strcat(dest, left);
  std::strcat(dest, right);

  return dest;
}

/** Code common to good_path() and good_leg_types() */
void good_path_common(const char *path_expression, Json_path *json_path) {
  size_t bad_idx = 0;
  EXPECT_FALSE(parse_path(strlen(path_expression), path_expression, json_path,
                          &bad_idx));

  EXPECT_EQ(0U, bad_idx) << "bad_idx != 0 for " << path_expression;
}

/** Verify that a good path parses correctly */
void good_path(bool check_path, const char *path_expression,
               std::string expected_path) {
  Json_path json_path;
  good_path_common(path_expression, &json_path);
  if (check_path) {
    String str;
    EXPECT_FALSE(json_path.to_string(&str));
    EXPECT_EQ(expected_path, std::string(str.ptr(), str.length()));

    // Move-construct a new path and verify that it's the same.
    Json_path path2{std::move(json_path)};
    str.length(0);
    EXPECT_FALSE(path2.to_string(&str));
    EXPECT_EQ(expected_path, std::string(str.ptr(), str.length()));

    // Move-assign to a new path and verify that it's the same.
    Json_path path3;
    path3 = std::move(path2);
    str.length(0);
    EXPECT_FALSE(path3.to_string(&str));
    EXPECT_EQ(expected_path, std::string(str.ptr(), str.length()));
  }
}

void good_path(const char *path_expression, std::string expected_path) {
  good_path(true, path_expression, expected_path);
}

/** Shorter form of good_path() */
void good_path(const char *path_expression) {
  good_path(false, path_expression, "");
}

/** Verify whether the path contains a wildcard, ellipsis or range token. */
void contains_wildcard(const char *path_expression, bool expected_answer) {
  Json_path json_path;
  good_path_common(path_expression, &json_path);
  EXPECT_EQ(expected_answer, json_path.can_match_many());
}

/** Verify that the leg at the given offset looks good */
void good_leg_at(const char *path_expression, int leg_index,
                 const std::string &expected_leg,
                 enum_json_path_leg_type expected_leg_type) {
  Json_path json_path;
  good_path_common(path_expression, &json_path);

  const Json_path_leg *actual_leg = *(json_path.begin() + leg_index);
  String str;
  EXPECT_FALSE(actual_leg->to_string(&str));
  EXPECT_EQ(expected_leg, std::string(str.ptr(), str.length()));
  EXPECT_EQ(expected_leg_type, actual_leg->get_type());
}

/* compare two path legs */
void compare_legs(const Json_path_leg *left, const Json_path_leg *right) {
  String left_str;
  String right_str;
  EXPECT_EQ(0, left->to_string(&left_str));
  EXPECT_EQ(0, right->to_string(&right_str));
  EXPECT_EQ(std::string(left_str.ptr(), left_str.length()),
            std::string(right_str.ptr(), right_str.length()));
}

/** Compare two paths */
void compare_paths(Json_path &left, Json_path_clone &right) {
  EXPECT_EQ(left.leg_count(), right.leg_count());

  Json_path_iterator right_it = right.begin();
  for (const Json_path_leg *left_leg : left) {
    compare_legs(left_leg, *right_it++);
  }
}

/** Verify that clones look alike */
void verify_clone(const char *path_expression_1,
                  const char *path_expression_2) {
  Json_path real_path1;
  good_path_common(path_expression_1, &real_path1);

  Json_path_clone cloned_path;
  for (const Json_path_leg *leg : real_path1) cloned_path.append(leg);
  compare_paths(real_path1, cloned_path);

  Json_path real_path2;
  good_path_common(path_expression_2, &real_path2);
  cloned_path.clear();
  for (const Json_path_leg *leg : real_path2) cloned_path.append(leg);
  compare_paths(real_path2, cloned_path);
}

/**
   Verify that a good path has the expected sequence of leg types.
*/
void good_leg_types(const char *path_expression,
                    enum_json_path_leg_type *expected_leg_types,
                    size_t length) {
  Json_path json_path;
  good_path_common(path_expression, &json_path);

  EXPECT_EQ(length, json_path.leg_count());
  Json_path_iterator it = json_path.begin();
  for (size_t idx = 0; idx < length; idx++) {
    EXPECT_EQ(expected_leg_types[idx], (*it++)->get_type());
  }
}

/** Verify that a bad path fails as expected */
void bad_path(const char *path_expression, size_t expected_index) {
  size_t actual_index = 0;
  Json_path json_path;
  EXPECT_TRUE(parse_path(strlen(path_expression), path_expression, &json_path,
                         &actual_index))
      << "Unexpectedly parsed " << path_expression;
  EXPECT_EQ(expected_index, actual_index)
      << "Unexpected index for " << path_expression;
}

/** Bad identifiers are ok as member names if they are double-quoted */
void bad_identifier(const char *identifier, size_t expected_index) {
  char dummy1[30];
  char dummy2[30];
  char *path_expression;

  path_expression = concat(dummy1, "$.", identifier);
  bad_path(path_expression, expected_index);

  path_expression = concat(dummy1, "$.\"", identifier);
  path_expression = concat(dummy2, path_expression, "\"");
  good_path(path_expression);
}

/*
  Helper functions for Json_wrapper tests.
*/

void JsonPathTest::vet_wrapper_seek(Json_wrapper *wrapper,
                                    const Json_path &path,
                                    const std::string &expected,
                                    bool expected_null) const {
  Json_wrapper_vector hits(PSI_NOT_INSTRUMENTED);
  wrapper->seek(path, path.leg_count(), &hits, true, false);
  String result_buffer;

  if (hits.size() == 1) {
    EXPECT_FALSE(hits[0].to_string(&result_buffer, true, "test"));
  } else {
    Json_array *a = new (std::nothrow) Json_array();
    for (uint i = 0; i < hits.size(); ++i) {
      a->append_clone(hits[i].to_dom(thd()));
    }
    Json_wrapper w(a);
    EXPECT_FALSE(w.to_string(&result_buffer, true, "test"));
  }

  std::string actual = std::string(result_buffer.ptr(), result_buffer.length());

  if (expected_null) {
    const char *source_output = "";
    const char *result_output = "";

    if (hits.size() > 0) {
      String source_buffer;
      EXPECT_FALSE(wrapper->to_string(&source_buffer, true, "test"));
      source_output = source_buffer.ptr();
      result_output = actual.c_str();
    }
    EXPECT_TRUE(hits.size() == 0)
        << "Unexpected result wrapper for " << source_output
        << ". The output is " << result_output << "\n";
  } else {
    EXPECT_EQ(expected, actual);
  }
}

void JsonPathTest::vet_wrapper_seek(const char *json_text,
                                    const char *path_text,
                                    const std::string &expected,
                                    bool expected_null) const {
  Json_dom_ptr dom = Json_dom::parse(json_text, std::strlen(json_text), false,
                                     nullptr, nullptr);

  String serialized_form;
  EXPECT_FALSE(json_binary::serialize(thd(), dom.get(), &serialized_form));
  json_binary::Value binary = json_binary::parse_binary(
      serialized_form.ptr(), serialized_form.length());

  Json_wrapper dom_wrapper(std::move(dom));
  Json_wrapper binary_wrapper(binary);

  Json_path path;
  good_path_common(path_text, &path);
  vet_wrapper_seek(&dom_wrapper, path, expected, expected_null);
  vet_wrapper_seek(&binary_wrapper, path, expected, expected_null);
}

void vet_dom_location(const char *json_text, const char *path_text) {
  Json_dom_ptr dom = Json_dom::parse(json_text, std::strlen(json_text), false,
                                     nullptr, nullptr);
  Json_path path;
  good_path_common(path_text, &path);
  Json_dom_vector hits(PSI_NOT_INSTRUMENTED);

  dom->seek(path, path.leg_count(), &hits, true, false);
  EXPECT_EQ(1U, hits.size());
  if (hits.size() > 0) {
    Json_dom *child = hits[0];
    Json_path location = child->get_location();
    String str;
    EXPECT_EQ(0, location.to_string(&str));
    EXPECT_EQ(path_text, std::string(str.ptr(), str.length()));
  }
}

/**
  Vet the short-circuiting effects of the only_needs_one argument
  of Json_wrapper.seek().

  @param[in] wrapper        A wrapped JSON document.
  @param[in] path           A path to search for.
  @param[in] expected_hits  Total number of expected matches.
*/
void vet_only_needs_one(Json_wrapper &wrapper, const Json_path &path,
                        uint expected_hits) {
  Json_wrapper_vector all_hits(PSI_NOT_INSTRUMENTED);
  wrapper.seek(path, path.leg_count(), &all_hits, true, false);

  EXPECT_EQ(expected_hits, all_hits.size());

  Json_wrapper_vector only_needs_one_hits(PSI_NOT_INSTRUMENTED);
  wrapper.seek(path, path.leg_count(), &only_needs_one_hits, true, true);
  uint expected_onoh_hits = (expected_hits == 0) ? 0 : 1;
  EXPECT_EQ(expected_onoh_hits, only_needs_one_hits.size());
}

/**
  Vet the short-circuiting effects of the only_needs_one argument
  of Json_wrapper.seek().

  @param[in] json_text              Text of the json document to search.
  @param[in] path_text              Text of the path expression to use.
  @param[in] expected_hits          Total number of expected matches.
  @param[in] thd                    THD handle
*/
void vet_only_needs_one(const char *json_text, const char *path_text,
                        uint expected_hits, const THD *thd) {
  Json_dom_ptr dom = Json_dom::parse(json_text, std::strlen(json_text), false,
                                     nullptr, nullptr);

  String serialized_form;
  EXPECT_FALSE(json_binary::serialize(thd, dom.get(), &serialized_form));
  json_binary::Value binary = json_binary::parse_binary(
      serialized_form.ptr(), serialized_form.length());

  Json_wrapper dom_wrapper(std::move(dom));
  Json_wrapper binary_wrapper(binary);

  Json_path path;
  good_path_common(path_text, &path);
  vet_only_needs_one(dom_wrapper, path, expected_hits);
  vet_only_needs_one(binary_wrapper, path, expected_hits);
}

/*

  Helper functions for testing Json_object.remove()
  and Json_array.remove().
*/

/**
   Format a Json_dom object to JSON text using  Json_wrapper's
   to_string functionality.

   @param dom The DOM object to be formatted
*/
std::string format(Json_dom *dom) {
  String buffer;
  Json_wrapper wrapper(dom->clone());
  EXPECT_FALSE(wrapper.to_string(&buffer, true, "format"));

  return std::string(buffer.ptr(), buffer.length());
}

/*
  Tests
*/

// Good paths with no column scope.
static const Good_path good_paths_no_column_scope[] = {
    {"$", "$"},
    {" $", "$"},
    {"$ ", "$"},
    {"  $   ", "$"},

    {"$[5]", "$[5]"},
    {"$[ 5 ]", "$[5]"},
    {" $[ 5 ] ", "$[5]"},
    {" $ [ 5  ] ", "$[5]"},

    {"$[456]", "$[456]"},
    {"$[ 456 ]", "$[456]"},
    {" $[ 456 ] ", "$[456]"},
    {" $ [  456   ] ", "$[456]"},

    {"$[last]", "$[last]"},
    {"$[ last]", "$[last]"},
    {"$[last ]", "$[last]"},
    {"$[last-1]", "$[last-1]"},
    {"$[last -1]", "$[last-1]"},
    {"$[last- 1]", "$[last-1]"},

    {"$[4294967295]", "$[4294967295]"},
    {"$[last-4294967295]", "$[last-4294967295]"},

    {"$.a", "$.a"},
    {"$ .a", "$.a"},
    {"$. a", "$.a"},
    {" $ .  a ", "$.a"},

    {" $. abc", "$.abc"},
    {" $ . abc", "$.abc"},
    {" $ . abc ", "$.abc"},
    {" $  . abc ", "$.abc"},

    {"$.a[7]", "$.a[7]"},
    {" $ . a [ 7 ] ", "$.a[7]"},

    {"$[7].a", "$[7].a"},
    {" $ [ 7 ] . a ", "$[7].a"},

    {"$.*", "$.*"},
    {" $ . * ", "$.*"},

    {"$.*.b", "$.*.b"},
    {" $ . * . b ", "$.*.b"},

    {"$.*[4]", "$.*[4]"},
    {"  $ . * [ 4 ]  ", "$.*[4]"},

    {"$[*]", "$[*]"},
    {" $ [ * ] ", "$[*]"},

    {"$[*].a", "$[*].a"},
    {"  $ [ * ] . a ", "$[*].a"},

    {"$[*][31]", "$[*][31]"},
    {" $ [ * ] [ 31 ] ", "$[*][31]"},

    {"$**.abc", "$**.abc"},
    {" $  ** . abc ", "$**.abc"},

    {"$**[0]", "$**[0]"},
    {" $ ** [ 0 ] ", "$**[0]"},

    {"$**.a", "$**.a"},
    {" $ ** . a ", "$**.a"},

    // backslash in front of a quote
    {"$.\"\\\\\"", "$.\"\\\\\""},

    // 0-length member names must be quoted
    {"$.\"\"", "$.\"\""},
    {"$.\"\".\"\"", "$.\"\".\"\""},
    {"$.\"\".a.\"\"", "$.\"\".a.\"\""},
    {"$.abc.\"\"", "$.abc.\"\""},
    {"$.abc.\"\".def", "$.abc.\"\".def"},
    {"$.\"abc\".\"\".def", "$.abc.\"\".def"},

    {"$[0 to 0]", "$[0 to 0]"},
    {"$[1 to 1]", "$[1 to 1]"},
    {"$[1 to 3]", "$[1 to 3]"},
    {"$[  1  to  3  ]", "$[1 to 3]"},
    {"$[0 to 4294967295]", "$[0 to 4294967295]"},
    {"$[last to last]", "$[last to last]"},
    {"$[last-0 to last - 0]", "$[last to last]"},
    {"$[last-1 to last-1]", "$[last-1 to last-1]"},
    {"$[last to 1]", "$[last to 1]"},
    {"$[1 to last]", "$[1 to last]"},
};

/** Test good paths without column scope */
TEST_P(JsonGoodPathTestP, GoodPaths) {
  Good_path param = GetParam();
  good_path(param.m_path_expression, param.m_expected_path);
}

INSTANTIATE_TEST_CASE_P(PositiveNoColumnScope, JsonGoodPathTestP,
                        ::testing::ValuesIn(good_paths_no_column_scope));

/** Test that path leg types look correct. */
TEST_F(JsonPathTest, LegTypes) {
  {
    SCOPED_TRACE("");
    enum_json_path_leg_type leg_types1[] = {jpl_member};
    good_leg_types("$.a", leg_types1, 1);
  }

  {
    SCOPED_TRACE("");
    enum_json_path_leg_type leg_types2[] = {jpl_array_cell};
    good_leg_types("$[3456]", leg_types2, 1);
  }

  {
    SCOPED_TRACE("");
    enum_json_path_leg_type leg_types3[] = {jpl_member_wildcard};
    good_leg_types("$.*", leg_types3, 1);
  }

  {
    SCOPED_TRACE("");
    enum_json_path_leg_type leg_types4[] = {jpl_array_cell_wildcard};
    good_leg_types("$[*]", leg_types4, 1);
  }

  {
    SCOPED_TRACE("");
    enum_json_path_leg_type leg_types5[] = {jpl_member, jpl_member};
    good_leg_types("$.foo.bar", leg_types5, 2);
  }

  {
    SCOPED_TRACE("");
    enum_json_path_leg_type leg_types6[] = {jpl_member, jpl_array_cell};
    good_leg_types("$.foo[987654321]", leg_types6, 2);
  }

  {
    SCOPED_TRACE("");
    enum_json_path_leg_type leg_types7[] = {jpl_member, jpl_member_wildcard};
    good_leg_types("$.foo.*", leg_types7, 2);
  }

  {
    SCOPED_TRACE("");
    enum_json_path_leg_type leg_types8[] = {jpl_member,
                                            jpl_array_cell_wildcard};
    good_leg_types("$.foo[*]", leg_types8, 2);
  }

  {
    SCOPED_TRACE("");
    enum_json_path_leg_type leg_types9[] = {jpl_ellipsis, jpl_member};
    good_leg_types("$**.foo", leg_types9, 2);
  }

  {
    SCOPED_TRACE("");
    good_leg_types(" $ ", nullptr, 0);
  }
}

/** Test accessors. */
TEST_F(JsonPathTest, Accessors) {
  {
    SCOPED_TRACE("");
    good_leg_at("$[*][31]", 0, "[*]", jpl_array_cell_wildcard);
  }
  {
    SCOPED_TRACE("");
    good_leg_at("$.abc[ 3 ].def", 2, ".def", jpl_member);
  }
  {
    SCOPED_TRACE("");
    good_leg_at("$.abc**.def", 1, "**", jpl_ellipsis);
  }
}

/** Test detection of wildcard/ellipsis tokens. */
TEST_F(JsonPathTest, WildcardDetection) {
  {
    SCOPED_TRACE("");
    contains_wildcard("$", false);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$.foo", false);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$[3]", false);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$.foo.bar", false);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$[3].foo", false);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$[3][5]", false);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$.*", true);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$[*]", true);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$.*.bar", true);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$**.bar", true);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$[*].foo", true);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$**.foo", true);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$[3].*", true);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$[*][5]", true);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$**[5]", true);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$[1 to 2]", true);
  }
  {
    SCOPED_TRACE("");
    contains_wildcard("$.a[1 to 2].b", true);
  }
}

TEST_P(JsonBadPathTestP, BadPaths) {
  Bad_path param = GetParam();
  bad_path(param.m_path_expression, param.m_expected_index);
}

// Bad paths with no column scope.
static const Bad_path bad_paths_no_column_scope[] = {
    // no leading $
    {"foo", 1},
    {"[5]", 1},

    // no period before key name
    {"$foo", 1},
    {"$[5]foo", 4},

    // array index not a number or a valid range
    {"$[a]", 2},
    {"$[5].foo[b]", 9},
    {"$[]", 2},
    {"$[1.2]", 4},
    {"$[1,2]", 4},
    {"$[1,]", 4},
    {"$[1 TO 3]", 5},
    {"$[1 tO 3]", 5},
    {"$[1 To 3]", 5},
    {"$[1 to]", 5},
    {"$[1to]", 4},
    {"$[1to 2]", 4},
    {"$[1 to2]", 5},
    {"$[1 ti 2]", 5},
    {"$[1 t", 5},
    {"$[1 to ", 5},
    {"$[1 to 2,]", 9},
    {"$[4 to 3]", 8},
    {"$[0 tolast]", 5},
    {"$[lastto 2]", 7},
    {"$[lastto to 2]", 7},
    {"$[last+0]", 7},
    {"$[last+1]", 7},
    {"$[LAST]", 2},

    // absurdly large array index, largest supported array index is 2^32-1
    {"$[9999999999999999999999999999999999999999"
     "999999999999999999999999999]",
     2},
    {"$[4294967296]", 2},
    {"$[18446744073709551616]", 2},
    {"$[9223372036854775808]", 2},
    {"$[4294967296 to 2]", 2},
    {"$[0 to 4294967296]", 7},
    {"$[0 to 4294967297]", 7},
    {"$[last-4294967296]", 7},

    // period not followed by member name
    {"$.", 2},
    {"$.foo.", 6},
    {"$[3].", 5},
    {"$.[3]", 2},
    {"$.foo[4].", 9},

    // array index not terminated by ]
    {"$[4", 3},
    {"$[4a]", 4},
    {"$[4abc]", 4},

    // ends in ellipsis
    {"$**", 3},
    {"$.foo**", 7},

    // paths shouldn't have column scopes if the caller says
    // they don't
    {"a.b.c$", 1},
    {"b.c$", 1},
    {"c$", 1},
    {"a.b.c$.e", 1},
    {"b.c$.e", 1},
    {"c$.e", 1},

    // unterminated double-quoted name
    {"$.\"bar", 6},

    // 0-length member names must be quoted
    {"$..ab", 2},
    {"$.", 2},
    {"$. ", 3},
    {"$.abc.", 6},
    {"$.abc..def", 6},
    {"$.\"abc\"..def", 8},

    // backslash in front of a quote, and no end quote
    {"$.\"\\\"", 5},

    // reject plus in front of array index
    {"$[+1]", 2},

    // negative array indexes are rejected
    {"$[-0]", 2},
    {"$[-1]", 2},
    {"$[0 to -1]", 7},
    {"$[-1 to 0]", 2},
    {"$[- 1]", 2},
    {"$[-]", 2},
};

INSTANTIATE_TEST_CASE_P(NegativeNoColumnScope, JsonBadPathTestP,
                        ::testing::ValuesIn(bad_paths_no_column_scope));

/** Good paths with column scope not supported yet */
TEST_F(JsonPathTest, PositiveColumnScope) {
  //
  // Test good path syntax
  //
  bad_path("a.b.c$", 1);
}

/** Test good quoted key names */
static const Good_path good_quoted_key_names[] = {
    {"$.\"a\"", "$.a"},
    {"$ .\"a\"", "$.a"},
    {"$. \"a\"", "$.a"},
    {" $ .  \"a\" ", "$.a"},

    {" $. \"abc\"", "$.abc"},
    {" $ . \"abc\"", "$.abc"},
    {" $ . \"abc\" ", "$.abc"},
    {" $  . \"abc\" ", "$.abc"},

    {"$.\"a\"[7]", "$.a[7]"},
    {" $ . \"a\" [ 7 ] ", "$.a[7]"},

    {"$[7].\"a\"", "$[7].a"},
    {" $ [ 7 ] . \"a\" ", "$[7].a"},

    {"$.*.\"b\"", "$.*.b"},
    {" $ . * . \"b\" ", "$.*.b"},

    {"$[*].\"a\"", "$[*].a"},
    {"  $ [ * ] . \"a\" ", "$[*].a"},

    {"$**.\"abc\"", "$**.abc"},
    {" $ ** . \"abc\" ", "$**.abc"},

    {"$**.\"a\"", "$**.a"},
    {" $ ** . \"a\" ", "$**.a"},

    // embedded spaces
    {"$.\" c d \"", "$.\" c d \""},
    {"$.\" c d \".\"a b\"", "$.\" c d \".\"a b\""},
    {"$.\"a b\".\" c d \"", "$.\"a b\".\" c d \""},
};

INSTANTIATE_TEST_CASE_P(QuotedKeyNamesPositive, JsonGoodPathTestP,
                        ::testing::ValuesIn(good_quoted_key_names));

/** Test bad quoted key names */
static const Bad_path bad_quoted_key_names[] = {
    // no closing quote
    {"$.a.\"bcd", 8},
    {"$.a.\"", 5},
    {"$.\"a\".\"bcd", 10},

    // not followed by a member or array cell
    {"$.abc.\"def\"ghi", 11},
    {"$.abc.\"def\"5", 11},

    // unrecognized escape character
    {"$.abc.\"def\\aghi\"", 16},

    // unrecognized unicode escape
    {"$.abcd.\"ef\\u01kfmno\"", 20},

    // not preceded by a period
    {"$\"abcd\"", 1},
    //{ false, "$.ghi\"abcd\"", 5 },
};

INSTANTIATE_TEST_CASE_P(QuotedKeyNamesNegative, JsonBadPathTestP,
                        ::testing::ValuesIn(bad_quoted_key_names));

/* Test that unquoted key names may not be ECMAScript identifiers */

static const Good_path good_ecmascript_identifiers[] = {
    // keywords, however, are allowed
    {"$.if.break.return", "$.if.break.return"},

    // member name can start with $ and _
    {"$.$abc", "$.$abc"},
    {"$.$abc", "$.$abc"},

    // internal digits are ok
    {"$.a1_$bc", "$.a1_$bc"},

    // and so are internal <ZWNJ> and <ZWJ> characters
    {"$.a\\u200Cbc",
     "$.a\xE2\x80\x8C"
     "bc"},
    {"$.a\\u200Dbc",
     "$.a\xE2\x80\x8D"
     "bc"},

    // and so are internal unicode combining marks
    {"$.a\\u0300bc",
     "$.a\xCC\x80"
     "bc"},
    {"$.a\\u030Fbc",
     "$.a\xCC\x8F"
     "bc"},
    {"$.a\\u036Fbc",
     "$.a\xCD\xAF"
     "bc"},

    // and so are internal unicode connector punctuation codepoints
    {"$.a\\uFE33bc",
     "$.a\xEF\xB8\xB3"
     "bc"},
};

INSTANTIATE_TEST_CASE_P(GoodECMAScriptIdentifiers, JsonGoodPathTestP,
                        ::testing::ValuesIn(good_ecmascript_identifiers));

TEST_F(JsonPathTest, BadECMAScriptIdentifiers) {
  // key names may not contain embedded quotes
  {
    SCOPED_TRACE("");
    bad_path("$.a\"bc", 6);
  }

  // key names may not start with a digit or punctuation
  {
    SCOPED_TRACE("");
    bad_identifier("1abc", 6);
  }
  {
    SCOPED_TRACE("");
    bad_identifier(";abc", 6);
  }

  // and not with the <ZWNJ> and <ZWJ> characters
  {
    SCOPED_TRACE("");
    bad_identifier("\\u200Cabc", 11);
  }

  // and not with a unicode combining mark
  {
    SCOPED_TRACE("");
    bad_identifier("\\u0300abc", 11);
  }
  {
    SCOPED_TRACE("");
    bad_identifier("\\u030Fabc", 11);
  }
  {
    SCOPED_TRACE("");
    bad_identifier("\\u036Fabc", 11);
  }

  // and not with unicode connector punctuation
  {
    SCOPED_TRACE("");
    bad_identifier("\\uFE33abc", 11);
  }
}

TEST_F(JsonPathTest, WrapperSeekTest) {
  // vacuous path
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("false", "$", "false", false);
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("[ false, true, 1 ]", "$", "[false, true, 1]", false);
  }

  // no match
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("false", "$.a", "", true);
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("[ false, true, 1 ]", "$[3]", "", true);
  }

  // first level retrieval
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("[ false, true, 1 ]", "$[2]", "1", false);
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "{ \"a\" : 1, \"b\" : { \"c\" : [ 1, 2, 3 ] }, "
        "\"d\" : 4 }",
        "$.b", "{\"c\": [1, 2, 3]}", false);
  }

  // second level retrieval
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("[ false, true, [ 1, null, 200, 300 ], 400 ]", "$[2][3]",
                     "300", false);
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "{ \"a\" : 1, \"b\" : { \"c\" : [ 1, 2, 3 ] }, "
        "\"d\" : 4 }",
        "$.b.c", "[1, 2, 3]", false);
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "[ false, {\"abc\": 500}, "
        "[ 1, null, 200, 300 ], 400 ]",
        "$[1].abc", "500", false);
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "{ \"a\" : 1, \"b\" : [ 100, 200, 300 ], "
        "\"d\" : 4 }",
        "$.b[2]", "300", false);
  }

  // wildcards
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "{ \"a\" : 1, \"b\" : [ 100, 200, 300 ], "
        "\"d\" : 4 }",
        "$.*", "[1, [100, 200, 300], 4]", false);
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "[ false, {\"a\": true}, {\"b\": 200}, "
        "{\"a\": 300} ]",
        "$[*].a", "[true, 300]", false);
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "{ \"b\": {\"c\": 100}, \"d\": {\"a\": 200}, "
        "\"e\": {\"a\": 300}}",
        "$.*.a", "[200, 300]", false);
  }

  //
  // ellipsis
  //
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "{ \"b\": {\"c\": 100}, \"d\": {\"a\": 200}, "
        "\"e\": {\"a\": 300}, \"f\": {\"g\": {\"a\": 500} } }",
        "$**.a", "[200, 300, 500]", false);
  }

  // ellipsis with array recursing into object
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "{ \"a\": 100, "
        "\"d\": [ {\"a\": 200}, "
        "{ \"e\": {\"a\": 300, \"f\": 500} }, "
        " { \"g\" : true, \"a\": 600 } ] }",
        "$.d**.a", "[200, 300, 600]", false);
  }

  // ellipsis with object recursing into arrays
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "{ \"a\": true, "
        " \"b\": { "
        " \"a\": 100,"
        " \"c\": [ "
        "200, { \"a\": 300 }, "
        "{ \"d\": { \"e\": { \"a\": 400 } }, \"f\": true }, "
        "500, [ { \"a\": 600 } ]"
        "]"
        "}, "
        " \"g\": { \"a\": 700 } }",
        "$.b**.a", "[100, 300, 400, 600]", false);
  }

  // daisy-chained ellipses
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "{ \"a\": { \"x\" : { \"b\": { \"y\": { \"b\": "
        "{ \"z\": { \"c\": 100 } } } } } } }",
        "$.a**.b**.c", "100", false);
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "{ "
        " \"c\": true"
        ", \"a\": { "
        " \"d\": [ "
        " { "
        " \"b\" : { "
        " \"e\": ["
        "{ \"c\": 100 "
        ", \"f\": { \"a\": 200, \"b\": { \"g\" : {  \"h\": "
        "{ \"c\": 300 } } } }"
        " }"
        " ]"
        " }"
        " }"
        " ]"
        " }"
        ", \"b\": true"
        " }",
        "$.a**.b**.c", "[100, 300]", false);
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "["
        "  100,"
        "  ["
        "    true,"
        "    false,"
        "    true,"
        "    false,"
        "    { \"a\": ["
        "                  300,"
        "                  400,"
        "                  ["
        "                     1, 2, 3, 4, 5,"
        "                     {"
        "                      \"b\": [ 500, 600, 700, 800, 900 ]"
        "                     }"
        "                  ]"
        "               ]"
        "    }"
        "  ],"
        "  200"
        "]",
        "$[1]**[2]**[3]", "[4, 800]", false);
  }

  // $[1][2][3].b[3] is a match for $[1]**[2]**[3]
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "["
        "  100,"
        "  ["
        "                  300,"
        "                  400,"
        "                  ["
        "                     1, 2, 3, 4, 5,"
        "                     {"
        "                      \"b\": [ 500, 600, 700, 800, 900 ]"
        "                     }"
        "                  ]"
        "  ],"
        "  200"
        "]",
        "$[1]**[2]**[3]", "[4, 800]", false);
  }

  /*
    $**[2]**.c matches

    $.a[ 2 ][ 1 ].c
    $.c.d[2][5].c
    $.d[2][4].d.c

    but not

    $.b[ 1 ][ 1 ].c
    $.e[2].c
  */
  {
    SCOPED_TRACE("");
    vet_wrapper_seek(
        "{"
        " \"a\": [ 0, 1, [ 0, { \"c\": 100 } ] ],"
        " \"b\": [ 0, [ 0, { \"c\": 200 } ] ],"
        " \"c\": { \"d\": [ 0, 1, [ 0, 1, 2, 3, 4, "
        "{ \"c\": 300 } ] ] },"
        " \"d\": [ 0, 1, [ 0, 1, 2, 3, { \"d\": "
        "{ \"c\": 400 } } ] ],"
        " \"e\": [ 0, 1, { \"c\": 500 } ]"
        "}",
        "$**[2]**.c", "[100, 300, 400, 500]", false);
  }

  // auto-wrapping
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("{ \"a\": 100 }", "$.a[ 0 ]", "100", false);
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("[ [ 100, 200, 300 ], 400, { \"c\": 500 } ]", "$[*][ 0 ]",
                     "[100, 400, {\"c\": 500}]", false);
  }

  // auto-wrapping only works for the 0th index
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("[ [ 100, 200, 300 ], 400, { \"c\": 500 } ]", "$[*][ 1 ]",
                     "200", false);
  }

  // verify more ellipsis and autowrapping cases.

  // these two should have the same result.
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("[1]", "$[0][0]", "1", false);
    SCOPED_TRACE("");
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("[1]", "$**[0]", "1", false);
  }

  // these two should have the same result.
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("{ \"a\": 1 }", "$.a[0]", "1", false);
    SCOPED_TRACE("");
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("{ \"a\": 1 }", "$**[0]", "[{\"a\": 1}, 1]", false);
  }

  // these two should have the same result.
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("{ \"a\": 1 }", "$[0].a", "1", false);
    SCOPED_TRACE("");
  }
  {
    SCOPED_TRACE("");
    vet_wrapper_seek("{ \"a\": 1 }", "$**.a", "1", false);
  }
}

TEST_F(JsonPathTest, RemoveDomTest) {
  {
    SCOPED_TRACE("");
    std::string json_text = "[100, 200, 300]";
    Json_dom_ptr dom = Json_dom::parse(json_text.data(), json_text.length(),
                                       false, nullptr, nullptr);
    auto array = static_cast<Json_array *>(dom.get());
    EXPECT_TRUE(array->remove(1));
    EXPECT_EQ("[100, 300]", format(array));
    EXPECT_FALSE(array->remove(2));
    EXPECT_EQ("[100, 300]", format(array));

    json_text = "{\"a\": 100, \"b\": 200, \"c\": 300}";
    dom = Json_dom::parse(json_text.data(), json_text.length(), false, nullptr,
                          nullptr);
    auto object = static_cast<Json_object *>(dom.get());
    EXPECT_TRUE(object->remove("b"));
    EXPECT_EQ("{\"a\": 100, \"c\": 300}", format(object));
    EXPECT_FALSE(object->remove("d"));
    EXPECT_EQ("{\"a\": 100, \"c\": 300}", format(object));
  }

  /*
    test the adding of parent pointers
  */

  // Json_dom.add_alias()

  Json_object object1;
  Json_boolean true_literal1(true);
  Json_boolean false_literal1(false);
  Json_null *null_literal1 = new (std::nothrow) Json_null();
  EXPECT_EQ(nullptr, null_literal1->parent());
  object1.add_clone(std::string("a"), &true_literal1);
  object1.add_clone(std::string("b"), &false_literal1);
  object1.add_alias(std::string("c"), null_literal1);
  EXPECT_EQ(&object1, null_literal1->parent());
  EXPECT_EQ("{\"a\": true, \"b\": false, \"c\": null}", format(&object1));
  SCOPED_TRACE("");
  EXPECT_TRUE(object1.remove("c"));
  EXPECT_EQ("{\"a\": true, \"b\": false}", format(&object1));
  EXPECT_FALSE(object1.remove("c"));
  EXPECT_EQ("{\"a\": true, \"b\": false}", format(&object1));

  // Json_dom.add_clone()

  Json_null null_literal2;
  EXPECT_EQ(nullptr, null_literal2.parent());
  std::string key("d");
  object1.add_clone(key, &null_literal2);
  Json_dom *clone = object1.get(key);
  EXPECT_EQ(&object1, clone->parent());

  // Json_array.append_clone()

  Json_array array;
  Json_boolean true_literal2(true);
  Json_boolean false_literal2(false);
  Json_null null_literal3;
  array.append_clone(&true_literal2);
  array.append_clone(&false_literal2);
  array.append_clone(&null_literal3);
  EXPECT_EQ("[true, false, null]", format(&array));
  Json_dom *cell = array[2];
  EXPECT_EQ(&array, cell->parent());

  // Json_array.append_alias()

  Json_boolean *true_literal3 = new (std::nothrow) Json_boolean(true);
  array.append_alias(true_literal3);
  EXPECT_EQ("[true, false, null, true]", format(&array));
  EXPECT_EQ(&array, true_literal3->parent());
  EXPECT_TRUE(array.remove(3));
  EXPECT_EQ("[true, false, null]", format(&array));
  EXPECT_FALSE(array.remove(3));
  EXPECT_EQ("[true, false, null]", format(&array));

  // Json_array.insert_clone()

  Json_boolean true_literal4(true);
  array.insert_clone(2, &true_literal4);
  EXPECT_EQ("[true, false, true, null]", format(&array));
  cell = array[2];
  EXPECT_EQ(&array, cell->parent());

  // Json_array.insert_alias()

  Json_boolean *false_literal3 = new (std::nothrow) Json_boolean(false);
  array.insert_alias(3, Json_dom_ptr(false_literal3));
  EXPECT_EQ("[true, false, true, false, null]", format(&array));
  EXPECT_EQ(&array, false_literal3->parent());
  EXPECT_TRUE(array.remove(3));
  EXPECT_EQ("[true, false, true, null]", format(&array));
  EXPECT_FALSE(array.remove(4));
  EXPECT_EQ("[true, false, true, null]", format(&array));

  // Json_array.insert_clone()
  Json_boolean true_literal5(true);
  array.insert_clone(5, &true_literal5);
  EXPECT_EQ("[true, false, true, null, true]", format(&array));
  EXPECT_EQ(&array, array[4]->parent());

  // Json_array.insert_alias()
  Json_boolean *false_literal4 = new (std::nothrow) Json_boolean(false);
  array.insert_alias(7, Json_dom_ptr(false_literal4));
  EXPECT_EQ("[true, false, true, null, true, false]", format(&array));
  EXPECT_EQ(&array, false_literal4->parent());
  EXPECT_EQ(&array, array[5]->parent());
  EXPECT_TRUE(array.remove(5));
  EXPECT_EQ("[true, false, true, null, true]", format(&array));
  EXPECT_FALSE(array.remove(5));
  EXPECT_EQ("[true, false, true, null, true]", format(&array));
}

// Tuples for the test of Json_dom.get_location()
static const Location_tuple location_tuples[] = {
    {"true", "$"},
    {"[true, false, null]", "$"},
    {"[true, false, null]", "$[1]"},
    {"{ \"a\": true}", "$"},
    {"{ \"a\": true}", "$.a"},
    {"{ \"a\": true, \"b\": [1, 2, 3] }", "$.b[2]"},
    {"[ 0, 1, { \"a\": true, \"b\": [1, 2, 3] } ]", "$[2].b[0]"},
};

/** Test good paths without column scope */
TEST_P(JsonGoodLocationTestP, GoodLocations) {
  Location_tuple param = GetParam();
  vet_dom_location(param.m_json_text, param.m_path_expression);
}

INSTANTIATE_TEST_CASE_P(LocationTesting, JsonGoodLocationTestP,
                        ::testing::ValuesIn(location_tuples));

// Tuples for the test of the only_needs_one arg of Json_wrapper.seek()
static const Ono_tuple ono_tuples[] = {
    {"[ { \"a\": 1  }, { \"a\": 2 }  ]", "$[*].a", 2},
    {"[ { \"a\": 1  }, { \"a\": 2 }  ]", "$**.a", 2},
    {"{ \"a\": { \"x\" : { \"b\": { \"y\": { \"b\": "
     "{ \"z\": { \"c\": 100 }, \"c\": 200 } } } } } }",
     "$.a**.b**.c", 2},
};

/** Test good paths without column scope */
TEST_P(JsonGoodOnoTestP, GoodOno) {
  Ono_tuple param = GetParam();
  vet_only_needs_one(param.m_json_text, param.m_path_expression,
                     param.m_expected_hits, thd());
}

INSTANTIATE_TEST_CASE_P(OnoTesting, JsonGoodOnoTestP,
                        ::testing::ValuesIn(ono_tuples));

// Tuples for tests of cloning
static const Clone_tuple clone_tuples[] = {
    {"$", "$[33]"},
    {"$[*].a", "$.a.b.c.d.e"},
    {"$.a.b.c[73]", "$**.abc.d.e.f.g"},
};

/** Test cloning. */
TEST_P(JsonGoodCloneTestP, GoodClone) {
  Clone_tuple param = GetParam();
  verify_clone(param.m_path_expression_1, param.m_path_expression_2);
}

INSTANTIATE_TEST_CASE_P(CloneTesting, JsonGoodCloneTestP,
                        ::testing::ValuesIn(clone_tuples));

/**
  A class used for parameterized test cases for the
  Json_path_leg::is_autowrap() function.
*/
class JsonPathLegAutowrapP
    : public ::testing::TestWithParam<std::pair<std::string, bool>> {};

TEST_P(JsonPathLegAutowrapP, Autowrap) {
  const auto param = GetParam();
  const std::string path_text = "$" + param.first + ".a";
  const bool expected_result = param.second;

  Json_path path;
  size_t idx = 0;
  EXPECT_FALSE(parse_path(path_text.length(), path_text.data(), &path, &idx));
  EXPECT_EQ(0U, idx);
  EXPECT_EQ(2U, path.leg_count());
  EXPECT_EQ(expected_result, (*path.begin())->is_autowrap());
}

static const std::pair<std::string, bool> autowrap_tuples[] = {
    // These should match non-arrays due to auto-wrapping.
    {"[0]", true},
    {"[last]", true},
    {"[last-0]", true},
    {"[0 to last]", true},
    {"[0 to last-0]", true},
    {"[0 to 0]", true},
    {"[0 to 1]", true},
    {"[0 to 100]", true},
    {"[last to 0]", true},
    {"[last to 1]", true},
    {"[last-0 to 1]", true},
    {"[last to 100]", true},
    {"[last to last]", true},
    {"[last-1 to last]", true},
    {"[last-100 to last]", true},
    {"[last-1 to 0]", true},
    {"[last-1 to 1]", true},

    // These should not match non-arrays.
    {"[*]", false},
    {".*", false},
    {"**", false},
    {".name", false},
    {".\"0\"", false},
    {"[1]", false},
    {"[100]", false},
    {"[last-1]", false},
    {"[last-100]", false},
    {"[0 to last-1]", false},
    {"[1 to last]", false},
    {"[last-2 to last-1]", false},
};

INSTANTIATE_TEST_CASE_P(AutowrapTesting, JsonPathLegAutowrapP,
                        ::testing::ValuesIn(autowrap_tuples));

}  // end namespace json_path_unittest
