/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>
#include <cstring>

#include "sql/item_json_func.h"
#include "sql/json_diff.h"
#include "sql/json_dom.h"
#include "sql/sql_class.h"
#include "sql/sql_list.h"
#include "unittest/gunit/base_mock_field.h"
#include "unittest/gunit/benchmark.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/test_utils.h"

namespace item_json_func_unittest {

class ItemJsonFuncTest : public ::testing::Test {
 protected:
  void SetUp() override {
    initializer.SetUp();
    m_table.in_use = thd();
  }

  void TearDown() override {
    m_table.cleanup_partial_update();
    initializer.TearDown();
  }

  THD *thd() { return initializer.thd(); }

  my_testing::Server_initializer initializer;

  Base_mock_field_json m_field{};

  Fake_TABLE m_table{&m_field};
};

/**
  Parse a JSON text and return its DOM representation.
  @param json_text null-terminated string of JSON text
  @return a DOM representing the JSON document
*/
static Json_dom_ptr parse_json(const char *json_text) {
  auto dom = Json_dom::parse(json_text, std::strlen(json_text), false, nullptr,
                             nullptr);
  EXPECT_NE(nullptr, dom);
  return dom;
}

static Item_string *new_item_string(const char *str) {
  return new Item_string(str, std::strlen(str), &my_charset_utf8mb4_bin);
}

static void store_json(Field_json *field, const char *json_text) {
  if (json_text == nullptr) {
    EXPECT_EQ(TYPE_OK, set_field_to_null(field));
  } else {
    field->set_notnull();
    Json_wrapper doc(parse_json(json_text));
    EXPECT_EQ(TYPE_OK, field->store_json(&doc));
  }
}

/**
  Perform a partial update on a JSON column and verify the result.

  @param func   the JSON function to invoke
  @param field  the JSON column to update
  @param orig_json  text representation of the original JSON value
  @param new_json   text representation of the expected value in the
                    column after the partial update
  @param binary_update   whether binary diffs can be used
  @param logical_update  whether logical diffs can be used
*/
static void do_partial_update(Item_json_func *func, Field_json *field,
                              const char *orig_json, const char *new_json,
                              bool binary_update, bool logical_update) {
  const auto table = field->table;

  if (!func->fixed) {
    EXPECT_FALSE(func->fix_fields(table->in_use, nullptr));
    EXPECT_TRUE(func->supports_partial_update(field));
    func->mark_for_partial_update(field);
  }

  table->clear_partial_update_diffs();

  store_json(field, orig_json);

  EXPECT_TRUE(table->is_binary_diff_enabled(field));
  EXPECT_TRUE(table->is_logical_diff_enabled(field));

  Json_wrapper res1;
  EXPECT_FALSE(func->val_json(&res1));
  EXPECT_EQ(new_json == nullptr, func->null_value);
  EXPECT_EQ(binary_update, table->is_binary_diff_enabled(field));
  EXPECT_EQ(logical_update, table->is_logical_diff_enabled(field));

  if (new_json == nullptr) return;

  Json_wrapper new_doc(parse_json(new_json));
  EXPECT_EQ(0, res1.compare(new_doc));

  if (!logical_update) {
    EXPECT_EQ(nullptr, table->get_logical_diffs(field));
    return;
  }

  /*
    Take a copy of the JSON diffs, since the call to
    clear_partial_update_diffs() below will clear the original
    Json_diff_vector.
  */
  const auto thd = table->in_use;
  Json_diff_vector diffs(Json_diff_vector::allocator_type(thd->mem_root));
  for (const auto &diff : *table->get_logical_diffs(field)) {
    diffs.add_diff(diff.path(), diff.operation(), diff.value().clone_dom(thd));
  }

  /*
    apply_json_diffs() will try to collect binary diffs for the
    changes that it applies to the column, so we should clear the
    already collected diffs.
  */
  table->clear_partial_update_diffs();

  EXPECT_TRUE(table->is_binary_diff_enabled(field));
  EXPECT_EQ(enum_json_diff_status::SUCCESS, apply_json_diffs(field, &diffs));
  EXPECT_EQ(binary_update, table->is_binary_diff_enabled(field));

  Json_wrapper res2;
  EXPECT_FALSE(field->val_json(&res2));
  EXPECT_EQ(0, res2.compare(new_doc));

  // apply_json_diffs() should produce new JSON diffs.
  EXPECT_TRUE(table->is_logical_diff_enabled(field));
  const Json_diff_vector *new_diffs = table->get_logical_diffs(field);
  EXPECT_NE(nullptr, new_diffs);
  EXPECT_EQ(diffs.size(), new_diffs->size());

  // ... and applying those new diffs should produce the same result again ...
  diffs.clear();
  for (const auto &diff : *new_diffs) {
    diffs.add_diff(diff.path(), diff.operation(), diff.value().clone_dom(thd));
  }
  table->clear_partial_update_diffs();
  store_json(field, orig_json);
  EXPECT_EQ(enum_json_diff_status::SUCCESS, apply_json_diffs(field, &diffs));
  Json_wrapper res3;
  EXPECT_FALSE(field->val_json(&res3));
  EXPECT_EQ(0, res3.compare(new_doc));
}

/*
  Test partial update using various JSON functions.
*/
TEST_F(ItemJsonFuncTest, PartialUpdate) {
  m_field.make_writable();

  auto json_set = new Item_func_json_set(
      thd(), new Item_field(&m_field), new_item_string("$[1]"),
      new_item_string("abc"), new_item_string("$[2]"), new Item_int(100));

  EXPECT_FALSE(m_table.mark_column_for_partial_update(&m_field));
  EXPECT_FALSE(m_table.setup_partial_update(true));

  // Logical update OK, but not enough space for binary update.
  {
    SCOPED_TRACE("");
    do_partial_update(json_set, &m_field, "[1,2,3]", "[1,\"abc\",100]", false,
                      true);
  }

  // Both logical update and binary update OK.
  {
    SCOPED_TRACE("");
    do_partial_update(json_set, &m_field, "[4,\"XYZ\",5]", "[4,\"abc\",100]",
                      true, true);
  }

  // No-op update.
  {
    SCOPED_TRACE("");
    do_partial_update(json_set, &m_field, "[0,\"abc\",100]", "[0,\"abc\",100]",
                      true, true);
    EXPECT_EQ(0U, m_table.get_binary_diffs(&m_field)->size());
    EXPECT_EQ(0U, m_table.get_logical_diffs(&m_field)->size());
  }

  // The array grows, so only logical update is OK.
  {
    SCOPED_TRACE("");
    do_partial_update(json_set, &m_field, "[6,\"XYZ\"]", "[6,\"abc\",100]",
                      false, true);
  }

  // The root document is auto-wrapped, so no partial update at all.
  {
    SCOPED_TRACE("");
    do_partial_update(json_set, &m_field, "true", "[true,\"abc\",100]", false,
                      false);
  }

  // A sub-document is auto-wrapped. OK for logical update, but not for binary.
  {
    SCOPED_TRACE("");
    auto wrap_set = new Item_func_json_set(
        thd(), new Item_field(&m_field), new_item_string("$.x[2]"),
        new Item_int(2), new_item_string("$.x[1]"), new Item_int(1));
    do_partial_update(wrap_set, &m_field, "{\"x\":123}", "{\"x\":[123,1]}",
                      false, true);
  }

  // Replacing the root of the document leads to full update.
  {
    SCOPED_TRACE("");
    auto replace_root = new Item_func_json_replace(
        thd(), new Item_field(&m_field), new_item_string("$"), new Item_int(1));
    do_partial_update(replace_root, &m_field, "{\"a\":[1,2,3]}", "1", false,
                      false);
  }

  // A nested call.
  {
    auto inner_func =
        new Item_func_json_set(thd(), new Item_field(&m_field),
                               new_item_string("$.a[1]"), new Item_int(1));

    auto outer_func = new Item_func_json_replace(
        thd(), inner_func, new_item_string("$.b"), new Item_int(2));

    {
      SCOPED_TRACE("");
      do_partial_update(outer_func, &m_field, "{\"a\":[1,2,3]}",
                        "{\"a\":[1,1,3]}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(outer_func, &m_field, "{\"a\":[1,2,3],\"b\":47}",
                        "{\"a\":[1,1,3],\"b\":2}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(outer_func, &m_field, "{\"a\":8}", "{\"a\":[8,1]}",
                        false, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(outer_func, &m_field, nullptr, nullptr, false, false);
    }
  }

  // A nested call where the inner function causes a full update.
  {
    auto inner_func = new Item_func_json_set(
        thd(), new Item_field(&m_field), new_item_string("$"),
        new Item_func_json_array(thd(), new Item_int(1), new Item_int(2)));
    auto outer_func = new Item_func_json_set(
        thd(), inner_func, new_item_string("$[1]"), new Item_int(3));

    SCOPED_TRACE("");
    do_partial_update(outer_func, &m_field, "[true,false]", "[1,3]", false,
                      false);
  }

  // Returning NULL should cause full update.
  {
    SCOPED_TRACE("");
    auto null_path = new Item_func_json_set(thd(), new Item_field(&m_field),
                                            new Item_null(), new Item_int(1));
    do_partial_update(null_path, &m_field, "[1,2,3]", nullptr, false, false);
  }

  // Input document being NULL should cause full update.
  {
    SCOPED_TRACE("");
    auto null_doc =
        new Item_func_json_set(thd(), new Item_field(&m_field),
                               new_item_string("$.a.b.c"), new Item_int(1));
    do_partial_update(null_doc, &m_field, nullptr, nullptr, false, false);
  }

  // Setting object member.
  {
    auto set_member =
        new Item_func_json_set(thd(), new Item_field(&m_field),
                               new_item_string("$.a"), new Item_int(1));

    // Existing member can be replaced with both binary and logical update.
    {
      SCOPED_TRACE("");
      do_partial_update(set_member, &m_field, "{\"a\":\"b\"}", "{\"a\":1}",
                        true, true);
    }

    // Non-existing member can be added with logical update.
    {
      SCOPED_TRACE("");
      do_partial_update(set_member, &m_field, "{}", "{\"a\":1}", false, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(set_member, &m_field, "[5,6,7]", "[5,6,7]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(set_member, &m_field, "123", "123", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(set_member, &m_field, nullptr, nullptr, false, false);
    }
  }

  // Replacing object member.
  {
    auto replace =
        new Item_func_json_replace(thd(), new Item_field(&m_field),
                                   new_item_string("$.a"), new Item_int(1));

    // Existing member can be replaced with both binary and logical update.
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "{\"a\":\"b\"}", "{\"a\":1}", true,
                        true);
    }

    // Replacing non-existing member is a no-op.
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "{}", "{}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "[5,6,7]", "[5,6,7]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "123", "123", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, nullptr, nullptr, false, false);
    }
  }

  // Setting array element.
  {
    auto set_element =
        new Item_func_json_set(thd(), new Item_field(&m_field),
                               new_item_string("$[1]"), new Item_int(1));
    {
      SCOPED_TRACE("");
      do_partial_update(set_element, &m_field, "[4,5,6]", "[4,1,6]", true,
                        true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(set_element, &m_field, "[]", "[1]", false, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(set_element, &m_field, "[2]", "[2,1]", false, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(set_element, &m_field, "{\"a\":2}", "[{\"a\":2},1]",
                        false, false);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(set_element, &m_field, "123", "[123,1]", false, false);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(set_element, &m_field, nullptr, nullptr, false, false);
    }
  }
  {
    auto set_element =
        new Item_func_json_set(thd(), new Item_field(&m_field),
                               new_item_string("$.a[1]"), new Item_int(1));
    {
      SCOPED_TRACE("");
      do_partial_update(set_element, &m_field, "{\"a\":[4,5,6]}",
                        "{\"a\":[4,1,6]}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(set_element, &m_field, "{\"a\":[]}", "{\"a\":[1]}",
                        false, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(set_element, &m_field, "{\"a\":{\"b\":2}}",
                        "{\"a\":[{\"b\":2},1]}", false, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(set_element, &m_field, "{\"a\":123}", "{\"a\":[123,1]}",
                        false, true);
    }
  }

  // Replacing array element.
  {
    auto replace =
        new Item_func_json_replace(thd(), new Item_field(&m_field),
                                   new_item_string("$[1]"), new Item_int(1));
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "[4,5,6]", "[4,1,6]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "[]", "[]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "[2]", "[2]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "{\"a\":2}", "{\"a\":2}", true,
                        true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "123", "123", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, nullptr, nullptr, false, false);
    }
  }
  {
    auto replace =
        new Item_func_json_replace(thd(), new Item_field(&m_field),
                                   new_item_string("$.a[1]"), new Item_int(1));
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "{\"a\":[4,5,6]}", "{\"a\":[4,1,6]}",
                        true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "{\"a\":[]}", "{\"a\":[]}", true,
                        true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "{\"a\":{\"b\":2}}",
                        "{\"a\":{\"b\":2}}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(replace, &m_field, "{\"a\":123}", "{\"a\":123}", true,
                        true);
    }
  }

  // Remove an element in an array.
  {
    auto remove = new Item_func_json_remove(thd(), new Item_field(&m_field),
                                            new_item_string("$[1]"));
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "{}", "{}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "[]", "[]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, nullptr, nullptr, false, false);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "[1,2,3]", "[1,3]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "[1,2]", "[1]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "[1]", "[1]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "{\"a\":1}", "{\"a\":1}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "123", "123", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, nullptr, nullptr, false, false);
    }
  }

  // Remove a member from an object.
  {
    auto remove = new Item_func_json_remove(thd(), new Item_field(&m_field),
                                            new_item_string("$.x"));
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "{}", "{}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "{\"a\":1}", "{\"a\":1}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "{\"x\":1}", "{}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field,
                        "{\"a\":\"b\",\"c\":\"d\",\"x\":\"y\",\"z\":\"w\"}",
                        "{\"a\":\"b\",\"c\":\"d\",\"z\":\"w\"}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "[1,2,3]", "[1,2,3]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "123", "123", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, nullptr, nullptr, false, false);
    }
  }

  // Remove multiple paths.
  {
    auto remove = new Item_func_json_remove(thd(), new Item_field(&m_field),
                                            new_item_string("$.a.b"),
                                            new_item_string("$.c[1]"));
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "{}", "{}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field,
                        "{\"a\":{\"b\":\"c\"}, \"b\":{\"c\":\"d\"}, "
                        "\"c\":[1,2,3]}",
                        "{\"a\":{}, \"b\":{\"c\":\"d\"}, \"c\":[1,3]}", true,
                        true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field,
                        "{\"a\":{\"b\":\"c\"}, \"b\":{\"c\":\"d\"}}",
                        "{\"a\":{}, \"b\":{\"c\":\"d\"}}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field,
                        "{\"b\":{\"c\":\"d\"}, \"c\":[1,2,3]}",
                        "{\"b\":{\"c\":\"d\"}, \"c\":[1,3]}", true, true);
    }
  }

  // JSON_REMOVE with NULL as path.
  {
    SCOPED_TRACE("");
    auto remove = new Item_func_json_remove(thd(), new Item_field(&m_field),
                                            new Item_null());
    do_partial_update(remove, &m_field, "[1,2]", nullptr, false, false);
  }

  // Mixed JSON_REMOVE/JSON_SET.
  {
    auto set =
        new Item_func_json_set(thd(), new Item_field(&m_field),
                               new_item_string("$.a"), new_item_string("abc"));
    auto remove = new Item_func_json_remove(thd(), set, new_item_string("$.b"));

    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "{}", "{\"a\":\"abc\"}", false, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, nullptr, nullptr, false, false);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "{\"b\":123}", "{\"a\":\"abc\"}",
                        false, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "{\"a\":\"xyz\",\"b\":123}",
                        "{\"a\":\"abc\"}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, nullptr, nullptr, false, false);
    }
  }

  // Remove with auto-wrap.
  {
    auto remove = new Item_func_json_remove(
        thd(), new Item_field(&m_field), new_item_string("$[0][0].a[0][0].b"));
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "{}", "{}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "[]", "[]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, "{\"a\":{\"b\":1,\"c\":2}}",
                        "{\"a\":{\"c\":2}}", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field,
                        "[{\"a\":[{\"b\":1,\"c\":2},123]}, 456]",
                        "[{\"a\":[{\"c\":2},123]}, 456]", true, true);
    }
    {
      SCOPED_TRACE("");
      do_partial_update(remove, &m_field, nullptr, nullptr, false, false);
    }
  }

  // Append or prepend when setting with out-of-bounds array indexes.
  {
    SCOPED_TRACE("");
    auto set = new Item_func_json_set(
        thd(), new Item_field(&m_field), new_item_string("$[2]"),
        new Item_int(88), new_item_string("$[last-2]"), new Item_int(99));
    do_partial_update(set, &m_field, "[]", "[99,88]", false, true);
    do_partial_update(set, &m_field, "[1]", "[99,1,88]", false, true);
    do_partial_update(set, &m_field, "[1,2]", "[99,2,88]", false, true);
    do_partial_update(set, &m_field, "[1,2,3]", "[99,2,88]", true, true);
    do_partial_update(set, &m_field, "[1,2,3,4]", "[1,99,88,4]", true, true);
    do_partial_update(set, &m_field, nullptr, nullptr, false, false);
  }
}

TEST(FieldJSONTest, TruncatedSortKey) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();

  const Json_wrapper doc(parse_json("47"));

  Base_mock_field_json field;
  Fake_TABLE table(&field);
  field.make_writable();
  EXPECT_EQ(TYPE_OK, field.store_json(&doc));

  uchar reference[1024];
  size_t reference_len = field.make_sort_key(reference, sizeof(reference));
  ASSERT_LT(reference_len, sizeof(reference));

  for (size_t max_len = 0; max_len < 100; ++max_len) {
    std::unique_ptr<uchar[]> buf(new uchar[max_len]);
    size_t len = field.make_sort_key(buf.get(), max_len);
    EXPECT_EQ(len, std::min(max_len, reference_len));
    EXPECT_EQ(0, memcmp(buf.get(), reference, len));
  }
  initializer.TearDown();
}

/**
  Microbenchmark which tests the performance of the JSON_SEARCH function.
*/
static void BM_JsonSearch(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  const Json_wrapper doc(
      parse_json("[\"Apple\", \"Orange\", \"Peach\","
                 "{\"key1\":\"Apple\", \"key2\":\"Orange\","
                 "\"key3\":\"Peach\"}, 1, 2, 3, 4, 5, 6]"));

  Base_mock_field_json field;
  Fake_TABLE table(&field);
  field.make_writable();
  EXPECT_EQ(TYPE_OK, field.store_json(&doc));

  auto search = new Item_func_json_search(table.in_use, new Item_field(&field),
                                          new_item_string("all"),
                                          new_item_string("Apple"));
  EXPECT_FALSE(search->fix_fields(table.in_use, nullptr));

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    Json_wrapper wr;
    EXPECT_FALSE(search->val_json(&wr));
    EXPECT_FALSE(search->null_value);
    EXPECT_EQ(2U, wr.length());
  }

  StopBenchmarkTiming();
  initializer.TearDown();
}
BENCHMARK(BM_JsonSearch)

/**
  Microbenchmark which tests the performance of the JSON_SEARCH
  function when it's called with arguments that contain wildcards.
*/
static void BM_JsonSearch_Wildcard(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  const Json_wrapper doc(
      parse_json("[{\"key1\": \"abc\","
                 "  \"key2\": \"def\","
                 "  \"key3\": \"abc\"},"
                 " {\"key1\": \"def\","
                 "  \"key2\": \"abc\","
                 "  \"key3\": \"def\"},"
                 " {\"key4\": {\"key4\":\"abc\"}}]"));

  Base_mock_field_json field;
  Fake_TABLE table(&field);
  field.make_writable();
  EXPECT_EQ(TYPE_OK, field.store_json(&doc));

  List<Item> args;
  args.push_back(new Item_field(&field));
  args.push_back(new_item_string("all"));
  args.push_back(new_item_string("abc"));
  args.push_back(new_item_string(""));  // escape character
  args.push_back(new_item_string("$[*].key1"));
  args.push_back(new_item_string("$**.key4"));

  auto search = new Item_func_json_search(table.in_use, args);
  EXPECT_FALSE(search->fix_fields(table.in_use, nullptr));

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    Json_wrapper wr;
    EXPECT_FALSE(search->val_json(&wr));
    EXPECT_FALSE(search->null_value);
    EXPECT_EQ(2U, wr.length());
  }

  StopBenchmarkTiming();
  initializer.TearDown();
}
BENCHMARK(BM_JsonSearch_Wildcard)

}  // namespace item_json_func_unittest
