/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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
#include <memory>

#include "base64.h"
#include "my_byteorder.h"
#include "my_inttypes.h"
#include "sql/json_binary.h"
#include "sql/json_diff.h"
#include "sql/json_dom.h"
#include "sql/json_path.h"
#include "sql/my_decimal.h"
#include "sql/sql_class.h"
#include "sql/sql_time.h"
#include "sql_string.h"
#include "template_utils.h"  // down_cast
#include "unittest/gunit/base_mock_field.h"
#include "unittest/gunit/benchmark.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/mysys_util.h"
#include "unittest/gunit/test_utils.h"

/**
 Test Json_dom class hierarchy API, cf. json_dom.h
 */
namespace json_dom_unittest {

class JsonDomTest : public ::testing::Test {
 protected:
  Base_mock_field_json m_field{};
  Fake_TABLE m_table{&m_field};
  void SetUp() override {
    initializer.SetUp();
    m_field.make_writable();
    m_table.in_use = thd();
  }
  void TearDown() override {
    m_table.cleanup_partial_update();
    initializer.TearDown();
  }
  my_testing::Server_initializer initializer;
  THD *thd() const { return initializer.thd(); }
};

/**
   Format a Json_dom object to JSON text using  Json_wrapper's
   to_string functionality.

   @param d The DOM object to be formatted
*/
static std::string format(const Json_dom &d) {
  String buffer;
  Json_wrapper w(d.clone());
  EXPECT_FALSE(w.to_string(&buffer, true, "format"));

  return std::string(buffer.ptr(), buffer.length());
}

static std::string format(const Json_dom *ptr) { return format(*ptr); }

static std::string format(const Json_dom_ptr &ptr) { return format(*ptr); }

/**
  Parse a JSON text and return its DOM representation.
  @param json_text null-terminated string of JSON text
  @return a DOM representing the JSON document
*/
static Json_dom_ptr parse_json(const char *json_text) {
  auto dom = Json_dom::parse(json_text, std::strlen(json_text), false, nullptr,
                             nullptr);
  EXPECT_FALSE(dom == nullptr);
  return dom;
}

/**
  Parse a JSON path.
  @param json_path  the JSON path text to parse
  @return the parsed path
*/
static Json_path parse_path(const char *json_path) {
  Json_path path;
  size_t bad_index;
  EXPECT_FALSE(parse_path(std::strlen(json_path), json_path, &path, &bad_index))
      << "bad index: " << bad_index;
  return path;
}

TEST_F(JsonDomTest, BasicTest) {
  String buffer;
  /* string scalar */
  const std::string std_s("abc");
  Json_string s(std_s);
  EXPECT_EQ(std_s, s.value());
  EXPECT_EQ(enum_json_type::J_STRING, s.json_type());
  EXPECT_TRUE(s.is_scalar());
  EXPECT_EQ(1U, s.depth());
  EXPECT_FALSE(s.is_number());
  EXPECT_EQ(std::string("\"abc\""), format(s));

  /*
    Escaping in strings, cf. ECMA-404 The JSON Data Interchange Format
  */
  Json_array a;
  /* double quote and backslash */
  Json_string js1(std::string("a\"b\\c"));
  a.append_clone(&js1);
  EXPECT_EQ(std::string("[\"a\\\"b\\\\c\"]"), format(a));

  a.clear();
  /* Printable control characters */
  Json_string js2(std::string("a\b\f\n\r\tb"));
  a.append_clone(&js2);
  EXPECT_EQ(7U, static_cast<Json_string *>(a[0])->size());
  EXPECT_EQ(std::string("[\"a\\b\\f\\n\\r\\tb\"]"), format(a));

  a.clear();
  /* Unprintable control characters and non-ASCII Unicode characters */
  Json_string js3(
      std::string("丳\x13"
                  "丽\x3"));
  a.append_clone(&js3);
  EXPECT_EQ(std::string("[\"丳\\u0013丽\\u0003\"]"), format(a));

  /* boolean scalar */
  const Json_boolean jb(true);
  EXPECT_EQ(enum_json_type::J_BOOLEAN, jb.json_type());
  EXPECT_EQ(true, jb.value());
  EXPECT_EQ(std::string("true"), format(jb));

  /* Integer scalar */
  const Json_int ji(-123);
  EXPECT_EQ(enum_json_type::J_INT, ji.json_type());
  EXPECT_EQ(-123, ji.value());
  EXPECT_EQ(std::string("-123"), format(ji));

  const Json_int max_32_int(2147483647);
  EXPECT_EQ(std::string("2147483647"), format(max_32_int));

  const Json_int max_64_int(9223372036854775807LL);
  EXPECT_EQ(std::string("9223372036854775807"), format(max_64_int));

  const Json_uint max_64_uint(18446744073709551615ULL);
  EXPECT_EQ(enum_json_type::J_UINT, max_64_uint.json_type());
  EXPECT_EQ(std::string("18446744073709551615"), format(max_64_uint));

  /* Double scalar */
  const Json_double jdb(-123.45);
  EXPECT_EQ(enum_json_type::J_DOUBLE, jdb.json_type());
  EXPECT_EQ(-123.45, jdb.value());
  EXPECT_EQ(std::string("-123.45"), format(jdb));

  /* Simple array with strings */
  a.clear();
  EXPECT_EQ(enum_json_type::J_ARRAY, a.json_type());
  EXPECT_FALSE(a.is_scalar());
  EXPECT_EQ(0U, a.size());
  Json_string js4(std::string("val1"));
  a.append_clone(&js4);
  Json_string js5(std::string("val2"));
  a.append_clone(&js5);
  EXPECT_EQ(2U, a.size());
  EXPECT_EQ(std::string("[\"val1\", \"val2\"]"), format(a));
  EXPECT_EQ(2U, a.depth());
  Json_dom *elt0 = a[0];
  Json_dom *elt1 = a[a.size() - 1];
  EXPECT_EQ(std::string("\"val1\""), format(elt0));
  EXPECT_EQ(std::string("\"val2\""), format(elt1));

  /* Simple object with string values, iterator and array cloning */
  Json_object o;
  EXPECT_EQ(enum_json_type::J_OBJECT, o.json_type());
  EXPECT_FALSE(a.is_scalar());
  EXPECT_EQ(0U, o.cardinality());
  Json_null null;
  EXPECT_EQ(enum_json_type::J_NULL, null.json_type());
  o.add_clone(std::string("key1"), &null);
  o.add_clone(std::string("key2"), &a);

  const std::string key_expected[] = {std::string("key1"), std::string("key2")};
  const std::string value_expected[] = {std::string("null"),
                                        std::string("[\"val1\", \"val2\"]")};

  int idx = 0;

  for (Json_object::const_iterator i = o.begin(); i != o.end(); ++i) {
    EXPECT_EQ(key_expected[idx], i->first);
    EXPECT_EQ(value_expected[idx], format(i->second));
    idx++;
  }

  /* Test uniqueness of keys */
  Json_string js6(std::string("js6"));
  o.add_clone("key1", &js6);
  EXPECT_EQ(2U, o.cardinality());
  EXPECT_EQ(std::string("{\"key1\": \"js6\", \"key2\": [\"val1\", \"val2\"]}"),
            format(o));
  EXPECT_EQ(3U, o.depth());
  o.add_clone("key1", &null);
  EXPECT_EQ(std::string("{\"key1\": null, \"key2\": [\"val1\", \"val2\"]}"),
            format(o));

  /* Nested array inside object and object inside array,
   * and object cloning
   */
  Json_array level3;
  level3.append_clone(&o);
  Json_int ji2(123);
  level3.insert_clone(0U, &ji2);
  EXPECT_EQ(std::string("[123, {\"key1\": null, \"key2\": "
                        "[\"val1\", \"val2\"]}]"),
            format(level3));
  EXPECT_EQ(4U, level3.depth());

  /* Array access: index */
  Json_dom *const elt = level3[1];
  EXPECT_EQ(std::string("{\"key1\": null, \"key2\": "
                        "[\"val1\", \"val2\"]}"),
            format(elt));

  /* Object access: key look-up */
  EXPECT_EQ(enum_json_type::J_OBJECT, elt->json_type());
  Json_object *const object_elt = down_cast<Json_object *>(elt);
  EXPECT_TRUE(object_elt != nullptr);
  const Json_dom *const elt2 = object_elt->get(std::string("key1"));
  EXPECT_EQ(std::string("null"), format(elt2));

  /* Clear object. */
  object_elt->clear();
  EXPECT_EQ(0U, object_elt->cardinality());

  /* Array remove element */
  EXPECT_TRUE(level3.remove(1));
  EXPECT_EQ(std::string("[123]"), format(level3));
  EXPECT_FALSE(level3.remove(level3.size()));
  EXPECT_EQ(std::string("[123]"), format(level3));

  /* Decimal scalar, including cloning */
  my_decimal m;
  EXPECT_FALSE(double2my_decimal(0, 3.14, &m));

  const Json_decimal jd(m);
  EXPECT_EQ(enum_json_type::J_DECIMAL, jd.json_type());
  EXPECT_TRUE(jd.is_number());
  EXPECT_TRUE(jd.is_scalar());
  const my_decimal m_out = *jd.value();
  double m_d;
  double m_out_d;

  decimal2double(&m, &m_d);
  decimal2double(&m_out, &m_out_d);
  EXPECT_EQ(m_d, m_out_d);

  a.append_clone(&jd);
  EXPECT_EQ(std::string("[\"val1\", \"val2\", 3.14]"), format(a));
  EXPECT_EQ(std::string("[\"val1\", \"val2\", 3.14]"), format(a.clone()));

  /* Array insert beyond end appends at end */
  a.clear();
  a.insert_alias(0, create_dom_ptr<Json_int>(0));
  a.insert_alias(2, create_dom_ptr<Json_int>(2));
  EXPECT_EQ(std::string("[0, 2]"), format(a));
  a.clear();
  a.insert_alias(0, create_dom_ptr<Json_int>(0));
  a.insert_alias(1, create_dom_ptr<Json_int>(1));
  EXPECT_EQ(std::string("[0, 1]"), format(a));

  /* Array clear, null type, boolean literals, including cloning */
  a.clear();
  Json_null jn;
  Json_boolean jbf(false);
  Json_boolean jbt(true);
  a.append_clone(&jn);
  a.append_clone(&jbf);
  a.append_clone(&jbt);
  EXPECT_EQ(std::string("[null, false, true]"), format(a));
  EXPECT_EQ(std::string("[null, false, true]"), format(a.clone()));

  /* DATETIME scalar */
  MYSQL_TIME dt;
  std::memset(&dt, 0, sizeof dt);
  MYSQL_TIME_STATUS status;
  EXPECT_FALSE(str_to_datetime(&my_charset_utf8mb4_bin, "19990412", 8, &dt,
                               (my_time_flags_t)0, &status));
  const Json_datetime scalar(dt, MYSQL_TYPE_DATETIME);
  EXPECT_EQ(enum_json_type::J_DATETIME, scalar.json_type());

  const MYSQL_TIME *dt_out = scalar.value();

  EXPECT_FALSE(std::memcmp(&dt, dt_out, sizeof(MYSQL_TIME)));
  EXPECT_EQ(std::string("\"1999-04-12\""), format(scalar));

  a.clear();
  a.append_clone(&scalar);
  EXPECT_EQ(std::string("[\"1999-04-12\"]"), format(a));

  EXPECT_FALSE(str_to_datetime(&my_charset_utf8mb4_bin,
                               "14-11-15 12.04.55.123456", 24, &dt,
                               (my_time_flags_t)0, &status));

  Json_datetime scalar2(dt, MYSQL_TYPE_DATETIME);
  EXPECT_EQ(std::string("\"2014-11-15 12:04:55.123456\""), format(scalar2));

  /* Opaque type storage scalar */
  const uint32 i = 0xCAFEBABE;
  char i_as_char[4];
  int4store(i_as_char, i);
  Json_opaque opaque(MYSQL_TYPE_TINY_BLOB, i_as_char, sizeof(i_as_char));
  EXPECT_EQ(enum_json_type::J_OPAQUE, opaque.json_type());
  EXPECT_EQ(i, uint4korr(opaque.value()));
  EXPECT_EQ(MYSQL_TYPE_TINY_BLOB, opaque.type());
  EXPECT_EQ(sizeof(i_as_char), opaque.size());
  EXPECT_EQ(std::string("\"base64:type249:vrr+yg==\""), format(opaque));

  const char *encoded = "vrr+yg==";
  char *buff = new char[static_cast<size_t>(
      base64_needed_decoded_length(static_cast<int>(std::strlen(encoded))))];
  EXPECT_EQ(4, base64_decode(encoded, std::strlen(encoded), buff, nullptr, 0));
  EXPECT_EQ(0xCAFEBABE, uint4korr(buff));
  delete[] buff;

  /* Build DOM from JSON text using rapdjson */
  const char *sample_doc =
      "{\"abc\": 3, \"foo\": [1, 2, {\"foo\": 3.24}, null]}";
  auto dom = parse_json(sample_doc);
  EXPECT_EQ(4U, dom->depth());
  EXPECT_EQ(std::string(sample_doc), format(dom));

  const char *sample_array = "[3, {\"abc\": \"\\u0000inTheText\"}]";
  dom = parse_json(sample_array);
  EXPECT_EQ(3U, dom->depth());
  EXPECT_EQ(std::string(sample_array), format(dom));

  const char *sample_scalar_doc = "2";
  dom = parse_json(sample_scalar_doc);
  EXPECT_EQ(std::string(sample_scalar_doc), format(dom));

  const char *max_uint_scalar = "18446744073709551615";
  dom = parse_json(max_uint_scalar);
  EXPECT_EQ(std::string(max_uint_scalar), format(dom));

  /*
    Test that duplicate keys are eliminated, and that the returned
    keys are in the expected order (sorted on length before
    contents).
  */
  const char *sample_object =
      "{\"key1\":1, \"key2\":2, \"key1\":3, "
      "\"key1\\u0000x\":4, \"key1\\u0000y\":5, \"a\":6, \"ab\":7, \"b\":8, "
      "\"\":9, \"\":10}";
  const std::string expected[8][2] = {
      {"", "10"},
      {"a", "6"},
      {"b", "8"},
      {"ab", "7"},
      {"key1", "3"},
      {"key2", "2"},
      {std::string("key1\0x", 6), "4"},
      {std::string("key1\0y", 6), "5"},
  };
  dom = parse_json(sample_object);
  const Json_object *obj = down_cast<const Json_object *>(dom.get());
  EXPECT_EQ(8U, obj->cardinality());
  idx = 0;

  for (Json_object::const_iterator it = obj->begin(); it != obj->end(); ++it) {
    EXPECT_EQ(expected[idx][0], it->first);
    EXPECT_EQ(expected[idx][1], format(it->second));
    idx++;
  }

  EXPECT_EQ(8, idx);

  /* Try to build DOM for JSON text using rapidjson on invalid text
     Included so we test error recovery
  */
  const char *half_object_item = "{\"label\": ";
  dom = Json_dom::parse(half_object_item, std::strlen(half_object_item), false,
                        nullptr, nullptr);
  EXPECT_EQ(nullptr, dom);

  const char *half_array_item = "[1,";
  dom = Json_dom::parse(half_array_item, std::strlen(half_array_item), false,
                        nullptr, nullptr);
  EXPECT_EQ(nullptr, dom);
}

/*
  Test that special characters are escaped when a Json_string is
  converted to text, so that it is possible to parse the resulting
  string. The JSON parser requires all characters in the range [0x00,
  0x1F] and the characters " (double-quote) and \ (backslash) to be
  escaped.
*/
TEST_F(JsonDomTest, EscapeSpecialChars) {
  // Create a JSON string with all characters in the range [0, 127].
  char input[128];
  for (size_t i = 0; i < sizeof(input); ++i) input[i] = static_cast<char>(i);
  const Json_string str(input, sizeof(input));

  // Now convert that value from JSON to text and back to JSON.
  Json_dom_ptr dom = parse_json(format(str).c_str());
  EXPECT_EQ(enum_json_type::J_STRING, dom->json_type());

  // Expect to get the same string back, including all the special characters.
  const Json_string *str2 = down_cast<const Json_string *>(dom.get());
  EXPECT_EQ(str.value(), str2->value());
}

void vet_wrapper_length(const THD *thd, const char *text,
                        size_t expected_length) {
  Json_wrapper dom_wrapper(parse_json(text));

  EXPECT_EQ(expected_length, dom_wrapper.length())
      << "Wrapped DOM: " << text << "\n";

  String serialized_form;
  EXPECT_FALSE(
      json_binary::serialize(thd, dom_wrapper.to_dom(thd), &serialized_form));
  json_binary::Value binary = json_binary::parse_binary(
      serialized_form.ptr(), serialized_form.length());
  Json_wrapper binary_wrapper(binary);

  json_binary::Value::enum_type binary_type = binary.type();

  if ((binary_type == json_binary::Value::ARRAY) ||
      (binary_type == json_binary::Value::OBJECT)) {
    EXPECT_EQ(expected_length, binary.element_count())
        << "BINARY: " << text << " and data = " << binary.get_data() << "\n";
  }
  EXPECT_EQ(expected_length, binary_wrapper.length())
      << "Wrapped BINARY: " << text << "\n";
}

TEST_F(JsonDomTest, WrapperTest) {
  // Constructors, assignment, copy constructors, aliasing
  Json_dom *d = new (std::nothrow) Json_null();
  Json_wrapper w(d);
  const THD *thd = this->thd();
  EXPECT_EQ(w.to_dom(thd), d);
  Json_wrapper w_2(w);
  EXPECT_NE(w.to_dom(thd), w_2.to_dom(thd));  // deep copy

  Json_wrapper w_2b;
  EXPECT_TRUE(w_2b.empty());
  w_2b = w;
  EXPECT_NE(w.to_dom(thd), w_2b.to_dom(thd));  // deep copy

  w.set_alias();  // d is now "free" again
  Json_wrapper w_3(w);
  EXPECT_EQ(w.to_dom(thd), w_3.to_dom(thd));  // alias copy
  w_3 = w;
  EXPECT_EQ(w.to_dom(thd), w_3.to_dom(thd));  // alias copy

  Json_wrapper w_4(d);  // give d a new owner
  Json_wrapper w_5;
  w_5 = std::move(w_4);  // takes over d
  EXPECT_EQ(w_4.to_dom(thd), w_5.to_dom(thd));

  Json_wrapper w_6;
  EXPECT_EQ(enum_json_type::J_ERROR, w_6.type());
  EXPECT_EQ(nullptr, w_6.to_dom(thd));
  EXPECT_EQ(nullptr, w_6.clone_dom(thd));
  EXPECT_EQ(0U, w_6.length());

  Json_dom *i = new (std::nothrow) Json_int(1);
  Json_wrapper w_7(i);
  w_5 = std::move(w_7);  // should deallocate w_5's original

  // scalars
  vet_wrapper_length(thd, "false", 1);
  vet_wrapper_length(thd, "true", 1);
  vet_wrapper_length(thd, "null", 1);
  vet_wrapper_length(thd, "1.1", 1);
  vet_wrapper_length(thd, "\"hello world\"", 1);

  // objects
  vet_wrapper_length(thd, "{}", 0);
  vet_wrapper_length(thd, "{ \"a\" : 100 }", 1);
  vet_wrapper_length(thd, "{ \"a\" : 100, \"b\" : 200 }", 2);

  // arrays
  vet_wrapper_length(thd, "[]", 0);
  vet_wrapper_length(thd, "[ 100 ]", 1);
  vet_wrapper_length(thd, "[ 100, 200 ]", 2);

  // nested objects
  vet_wrapper_length(thd, "{ \"a\" : 100, \"b\" : { \"c\" : 300 } }", 2);

  // nested arrays
  vet_wrapper_length(thd, "[ 100, [ 200, 300 ] ]", 2);
}

void vet_merge(const char *left_text, const char *right_text,
               std::string expected) {
  Json_dom_ptr result_dom =
      merge_doms(parse_json(left_text), parse_json(right_text));
  EXPECT_EQ(expected, format(*result_dom));
}

TEST_F(JsonDomTest, MergeTest) {
  // merge 2 scalars
  {
    SCOPED_TRACE("");
    vet_merge("1", "true", "[1, true]");
  }

  // merge a scalar with an array
  {
    SCOPED_TRACE("");
    vet_merge("1", "[true, false]", "[1, true, false]");
  }

  // merge an array with a scalar
  {
    SCOPED_TRACE("");
    vet_merge("[true, false]", "1", "[true, false, 1]");
  }

  // merge a scalar with an object
  {
    SCOPED_TRACE("");
    vet_merge("1", "{\"a\": 2}", "[1, {\"a\": 2}]");
  }

  // merge an object with a scalar
  {
    SCOPED_TRACE("");
    vet_merge("{\"a\": 2}", "1", "[{\"a\": 2}, 1]");
  }

  // merge 2 arrays
  {
    SCOPED_TRACE("");
    vet_merge("[1, 2]", "[3, 4]", "[1, 2, 3, 4]");
  }

  // merge 2 objects
  {
    SCOPED_TRACE("");
    vet_merge("{\"a\": 1, \"b\": 2 }", "{\"c\": 3, \"d\": 4 }",
              "{\"a\": 1, \"b\": 2, \"c\": 3, \"d\": 4}");
  }

  // merge an array with an object
  {
    SCOPED_TRACE("");
    vet_merge("[1, 2]", "{\"c\": 3, \"d\": 4 }",
              "[1, 2, {\"c\": 3, \"d\": 4}]");
  }

  // merge an object with an array
  {
    SCOPED_TRACE("");
    vet_merge("{\"c\": 3, \"d\": 4 }", "[1, 2]",
              "[{\"c\": 3, \"d\": 4}, 1, 2]");
  }

  // merge two objects which share a key. scalar + scalar
  {
    SCOPED_TRACE("");
    vet_merge("{\"a\": 1, \"b\": 2 }", "{\"b\": 3, \"d\": 4 }",
              "{\"a\": 1, \"b\": [2, 3], \"d\": 4}");
  }

  // merge two objects which share a key. scalar + array
  {
    SCOPED_TRACE("");
    vet_merge("{\"a\": 1, \"b\": 2 }", "{\"b\": [3, 4], \"d\": 4 }",
              "{\"a\": 1, \"b\": [2, 3, 4], \"d\": 4}");
  }

  // merge two objects which share a key. array + scalar
  {
    SCOPED_TRACE("");
    vet_merge("{\"a\": 1, \"b\": [2, 3] }", "{\"b\": 4, \"d\": 4 }",
              "{\"a\": 1, \"b\": [2, 3, 4], \"d\": 4}");
  }

  // merge two objects which share a key. scalar + object
  {
    SCOPED_TRACE("");
    vet_merge("{\"a\": 1, \"b\": 2 }",
              "{\"b\": {\"e\": 7, \"f\": 8}, \"d\": 4 }",
              "{\"a\": 1, \"b\": [2, {\"e\": 7, \"f\": 8}], \"d\": 4}");
  }

  // merge two objects which share a key. object + scalar
  {
    SCOPED_TRACE("");
    vet_merge("{\"b\": {\"e\": 7, \"f\": 8}, \"d\": 4 }",
              "{\"a\": 1, \"b\": 2 }",
              "{\"a\": 1, \"b\": [{\"e\": 7, \"f\": 8}, 2], \"d\": 4}");
  }

  // merge two objects which share a key. array + array
  {
    SCOPED_TRACE("");
    vet_merge("{\"a\": 1, \"b\": [2, 9] }", "{\"b\": [10, 11], \"d\": 4 }",
              "{\"a\": 1, \"b\": [2, 9, 10, 11], \"d\": 4}");
  }

  // merge two objects which share a key. array + object
  {
    SCOPED_TRACE("");
    vet_merge("{\"a\": 1, \"b\": [2, 9] }",
              "{\"b\": {\"e\": 7, \"f\": 8}, \"d\": 4 }",
              "{\"a\": 1, \"b\": [2, 9, {\"e\": 7, \"f\": 8}], \"d\": 4}");
  }

  // merge two objects which share a key. object + array
  {
    SCOPED_TRACE("");
    vet_merge("{\"b\": {\"e\": 7, \"f\": 8}, \"d\": 4 }",
              "{\"a\": 1, \"b\": [2, 9] }",
              "{\"a\": 1, \"b\": [{\"e\": 7, \"f\": 8}, 2, 9], \"d\": 4}");
  }

  // merge two objects which share a key. object + object
  {
    SCOPED_TRACE("");
    vet_merge("{\"b\": {\"e\": 7, \"f\": 8}, \"d\": 4 }",
              "{\"a\": 1, \"b\": {\"e\": 20, \"g\": 21 } }",
              "{\"a\": 1, \"b\": {\"e\": [7, 20], \"f\": 8, \"g\": 21}, "
              "\"d\": 4}");
  }
}

/// Create a JSON path for accessing an array element at the given position.
static Json_path array_accessor(size_t idx) {
  Json_path path;
  path.append(Json_path_leg(idx));
  return path;
}

/**
  Verify that applying the given binary diffs on the original binary
  value produces a binary string identical to the updated binary value.

  @param field     the column that is updated
  @param diffs     the binary diffs to apply
  @param original  the original binary data
  @param updated   the updated binary data
*/
static void verify_binary_diffs(Field_json *field,
                                const Binary_diff_vector *diffs,
                                const String &original, const String &updated) {
  SCOPED_TRACE("verify_binary_diffs");
  EXPECT_EQ(original.length(), updated.length());
  std::unique_ptr<char[]> buffer(new char[original.length()]);
  std::memcpy(buffer.get(), original.ptr(), original.length());
  const Binary_diff *prev = nullptr;
  for (const auto &diff : *diffs) {
    EXPECT_LE(diff.offset(), original.length());
    EXPECT_LE(diff.offset() + diff.length(), original.length());
    std::memcpy(buffer.get() + diff.offset(), diff.new_data(field),
                diff.length());
    /*
      The diff vector should be ordered on offset and not have any overlapping
      or adjacent areas.
    */
    if (prev != nullptr) {
      EXPECT_LT(prev->offset() + prev->length(), diff.offset());
    }
    prev = &diff;
  }
  EXPECT_EQ(0, std::memcmp(buffer.get(), updated.ptr(), updated.length()));
}

/**
  Tests for Json_wrapper::attempt_binary_update().
*/
TEST_F(JsonDomTest, AttemptBinaryUpdate) {
  auto dom = parse_json("[\"abc\", 123, \"def\", -70000]");

  String buffer;
  EXPECT_FALSE(json_binary::serialize(thd(), dom.get(), &buffer));

  json_binary::Value binary =
      json_binary::parse_binary(buffer.ptr(), buffer.length());

  EXPECT_FALSE(m_table.mark_column_for_partial_update(&m_field));
  EXPECT_FALSE(m_table.setup_partial_update(true));

  // Verify that the table interface for partial update works.
  EXPECT_TRUE(m_table.has_binary_diff_columns());
  EXPECT_TRUE(m_table.is_marked_for_partial_update(&m_field));
  EXPECT_TRUE(m_table.is_binary_diff_enabled(&m_field));
  EXPECT_NE(nullptr, m_table.get_binary_diffs(&m_field));
  m_table.disable_binary_diffs_for_current_row(&m_field);
  EXPECT_FALSE(m_table.has_binary_diff_columns());
  EXPECT_TRUE(m_table.is_marked_for_partial_update(&m_field));
  EXPECT_FALSE(m_table.is_binary_diff_enabled(&m_field));
  EXPECT_EQ(nullptr, m_table.get_binary_diffs(&m_field));
  m_table.clear_partial_update_diffs();
  EXPECT_TRUE(m_table.has_binary_diff_columns());
  EXPECT_TRUE(m_table.is_marked_for_partial_update(&m_field));
  EXPECT_TRUE(m_table.is_binary_diff_enabled(&m_field));
  EXPECT_NE(nullptr, m_table.get_binary_diffs(&m_field));

  const Binary_diff_vector *diffs = m_table.get_binary_diffs(&m_field);
  EXPECT_TRUE(diffs != nullptr);
  EXPECT_EQ(0U, diffs->size());

  // Not enough space for a four-character string anywhere in the array.
  for (size_t i = 0; i < 4; ++i) {
    SCOPED_TRACE("");
    Json_wrapper doc(binary);
    EXPECT_EQ(TYPE_OK, m_field.store_json(&doc));
    Json_wrapper jstr(new (std::nothrow) Json_string("abcd"));
    m_table.clear_partial_update_diffs();
    auto path = array_accessor(i);
    String shadow;
    bool success = true;
    bool replaced = true;
    EXPECT_FALSE(doc.attempt_binary_update(&m_field, path, &jstr, true, &shadow,
                                           &success, &replaced));
    EXPECT_FALSE(success);
    EXPECT_FALSE(replaced);
    EXPECT_EQ(0U, diffs->size());
    EXPECT_EQ(0, doc.compare(Json_wrapper(binary)));
    EXPECT_EQ(0U, m_table.get_logical_diffs(&m_field)->size());
  }

  // Enough space for an inlinable value anywhere in the array.
  for (size_t i = 0; i < 4; ++i) {
    SCOPED_TRACE("");
    Json_wrapper doc(binary);
    EXPECT_EQ(TYPE_OK, m_field.store_json(&doc));
    EXPECT_FALSE(m_field.val_json(&doc));
    Json_wrapper jint(new (std::nothrow) Json_int(456));
    m_table.clear_partial_update_diffs();
    auto path = array_accessor(i);
    String shadow;
    bool success = false;
    bool replaced = false;
    EXPECT_FALSE(doc.attempt_binary_update(&m_field, path, &jint, true, &shadow,
                                           &success, &replaced));
    EXPECT_TRUE(success);
    EXPECT_TRUE(replaced);
    EXPECT_EQ(1U, diffs->size());

    Json_array_ptr array(down_cast<Json_array *>(dom->clone().release()));
    array->remove(i);
    array->insert_clone(i, jint.to_dom(thd()));
    EXPECT_EQ(0, doc.compare(Json_wrapper(std::move(array))));

    EXPECT_EQ(TYPE_OK, m_field.store_json(&doc));
    verify_binary_diffs(&m_field, diffs, buffer, shadow);
  }

  // Enough space for a four-byte integer only on $[0], $[2] and $[3].
  for (size_t i = 0; i < 4; ++i) {
    SCOPED_TRACE("");
    Json_wrapper doc(binary);
    EXPECT_EQ(TYPE_OK, m_field.store_json(&doc));
    EXPECT_FALSE(m_field.val_json(&doc));
    Json_wrapper jint(new (std::nothrow) Json_uint(80000));
    m_table.clear_partial_update_diffs();
    auto path = array_accessor(i);
    String shadow;
    bool success = false;
    bool replaced = false;
    EXPECT_FALSE(doc.attempt_binary_update(&m_field, path, &jint, true, &shadow,
                                           &success, &replaced));
    EXPECT_EQ(i != 1, success);
    if (!success) {
      EXPECT_FALSE(replaced);
      EXPECT_EQ(0U, diffs->size());
      EXPECT_EQ(0U, m_table.get_logical_diffs(&m_field)->size());
      continue;
    }
    EXPECT_TRUE(replaced);
    EXPECT_EQ(2U, diffs->size()) << i;

    Json_array_ptr array(down_cast<Json_array *>(dom->clone().release()));
    array->remove(i);
    array->insert_clone(i, jint.to_dom(thd()));
    EXPECT_EQ(0, doc.compare(Json_wrapper(std::move(array))));

    EXPECT_EQ(TYPE_OK, m_field.store_json(&doc));
    verify_binary_diffs(&m_field, diffs, buffer, shadow);
  }

  {
    SCOPED_TRACE("");
    Json_wrapper doc(binary);
    EXPECT_EQ(TYPE_OK, m_field.store_json(&doc));
    EXPECT_FALSE(m_field.val_json(&doc));
    Json_wrapper jint(new (std::nothrow) Json_int(456));
    m_table.clear_partial_update_diffs();
    String shadow;
    auto path = array_accessor(100);

    /*
      Partial update is not performed with a non-existing path when JSON_SET
      semantics are used.
    */
    bool success = false;
    bool replaced = false;
    EXPECT_FALSE(doc.attempt_binary_update(&m_field, path, &jint, false,
                                           &shadow, &success, &replaced));
    EXPECT_FALSE(success);
    EXPECT_FALSE(replaced);
    EXPECT_EQ(0U, diffs->size());
    EXPECT_EQ(0U, m_table.get_logical_diffs(&m_field)->size());

    /*
      JSON_REPLACE is a no-op if a non-existing path is given, so expect
      partial update to be successful.
    */
    EXPECT_FALSE(doc.attempt_binary_update(&m_field, path, &jint, true, &shadow,
                                           &success, &replaced));
    EXPECT_TRUE(success);
    EXPECT_FALSE(replaced);
    EXPECT_EQ(0U, diffs->size());
    EXPECT_EQ(0, doc.compare(Json_wrapper(binary)));
    EXPECT_EQ(0U, m_table.get_logical_diffs(&m_field)->size());

    /*
      If we replace the top-level document (empty path), we do a full update.
      Expect the attempt to do partial update to fail.
    */
    EXPECT_FALSE(doc.attempt_binary_update(&m_field, Json_path(), &jint, false,
                                           &shadow, &success, &replaced));
    EXPECT_FALSE(success);
    EXPECT_FALSE(replaced);
  }

  {
    SCOPED_TRACE("");
    m_table.clear_partial_update_diffs();
    Json_wrapper doc(binary);
    EXPECT_EQ(TYPE_OK, m_field.store_json(&doc));
    EXPECT_FALSE(m_field.val_json(&doc));
    auto array = down_cast<Json_array *>(dom->clone().release());
    Json_wrapper array_wrapper(array);
    String shadow;
    // Replace all elements with short strings which fit at the old location.
    Json_wrapper jstr(new (std::nothrow) Json_string("x"));
    for (size_t i = 0; i < 4; ++i) {
      auto path = array_accessor(i);
      bool success = false;
      bool replaced = false;
      EXPECT_FALSE(doc.attempt_binary_update(&m_field, path, &jstr, false,
                                             &shadow, &success, &replaced));
      EXPECT_TRUE(success);
      EXPECT_TRUE(replaced);
      array->remove(i);
      array->insert_clone(i, jstr.to_dom(thd()));
      EXPECT_EQ(0, doc.compare(array_wrapper));

      EXPECT_EQ(TYPE_OK, m_field.store_json(&doc));
      verify_binary_diffs(&m_field, diffs, buffer, shadow);
    }
    /*
      We expect one diff for the first element (type/offset unchanged, data
      changed), two diffs for the second element (type/offset changed, data
      changed), one diff for the third element (type/offset unchanged, data
      changed), and two diffs for the fourth element (type/offset changed, data
      changed).

      However, the first, third, fourth and fifth diff are touching adjacent
      areas and are merged into a single diff. So there are three diffs.
    */
    EXPECT_EQ(3U, diffs->size());
  }
}

/**
  Test attempt_binary_update() with all types.
*/
TEST_F(JsonDomTest, AttemptBinaryUpdate_AllTypes) {
  // Make the table ready for partial update.
  EXPECT_FALSE(m_table.mark_column_for_partial_update(&m_field));
  EXPECT_FALSE(m_table.setup_partial_update(true));

  my_decimal decimal;
  EXPECT_FALSE(double2my_decimal(0, 3.14, &decimal));

  MYSQL_TIME dt;
  std::memset(&dt, 0, sizeof(dt));
  MYSQL_TIME_STATUS status;
  EXPECT_FALSE(str_to_datetime(&my_charset_utf8mb4_bin, "20170223", 8, &dt,
                               static_cast<my_time_flags_t>(0), &status));

  MYSQL_TIME tm;
  std::memset(&dt, 0, sizeof(tm));
  EXPECT_FALSE(str_to_time(&my_charset_utf8mb4_bin, "17:28:25", 8, &tm,
                           static_cast<my_time_flags_t>(0), &status));

  Json_dom *doms[] = {
      new (std::nothrow) Json_null,
      new (std::nothrow) Json_boolean(true),
      new (std::nothrow) Json_boolean(false),
      new (std::nothrow) Json_int(0),
      new (std::nothrow) Json_int(1000),
      new (std::nothrow) Json_int(100000),
      new (std::nothrow) Json_int(100000000),
      new (std::nothrow) Json_uint(0),
      new (std::nothrow) Json_uint(1000),
      new (std::nothrow) Json_uint(100000),
      new (std::nothrow) Json_uint(100000000),
      new (std::nothrow) Json_double(3.14),
      new (std::nothrow) Json_string(""),
      new (std::nothrow) Json_string("xyz"),
      new (std::nothrow) Json_decimal(decimal),
      new (std::nothrow) Json_datetime(dt, MYSQL_TYPE_DATETIME),
      new (std::nothrow) Json_datetime(dt, MYSQL_TYPE_DATE),
      new (std::nothrow) Json_datetime(dt, MYSQL_TYPE_TIMESTAMP),
      new (std::nothrow) Json_datetime(tm, MYSQL_TYPE_TIME),
      new (std::nothrow) Json_opaque(MYSQL_TYPE_BLOB, 5, 'x'),
      new (std::nothrow) Json_array(),
      parse_json("[1,2,3]").release(),
      new (std::nothrow) Json_object(),
      parse_json("{\"a\":\"b\"}").release(),
  };

  for (auto dom : doms) {
    m_table.clear_partial_update_diffs();

    Json_array_ptr original_dom(new (std::nothrow) Json_array);
    original_dom->append_alias(new (std::nothrow) Json_string(20, 'x'));

    /*
      Write an array with one element into the JSON column. Make sure the
      element is large enough to allow partial update with all the values in
      the doms array.
    */
    Json_wrapper doc(original_dom->clone());
    EXPECT_EQ(TYPE_OK, m_field.store_json(&doc));
    EXPECT_FALSE(m_field.val_json(&doc));

    StringBuffer<STRING_BUFFER_USUAL_SIZE> original;
    EXPECT_FALSE(doc.to_binary(thd(), &original));

    Json_wrapper new_value(dom->clone());

    StringBuffer<STRING_BUFFER_USUAL_SIZE> buffer;
    bool success = false;
    bool replaced = false;

    // First try with non-existing path and replace logic. Should be a no-op.
    EXPECT_FALSE(doc.attempt_binary_update(&m_field, array_accessor(100),
                                           &new_value, true, &buffer, &success,
                                           &replaced));
    EXPECT_TRUE(success);
    EXPECT_FALSE(replaced);
    EXPECT_EQ(0U, m_table.get_binary_diffs(&m_field)->size());
    EXPECT_EQ(0U, m_table.get_logical_diffs(&m_field)->size());
    m_table.clear_partial_update_diffs();

    // Try with non-existing path and JSON_SET logic. Requires full update.
    EXPECT_FALSE(doc.attempt_binary_update(&m_field, array_accessor(100),
                                           &new_value, false, &buffer, &success,
                                           &replaced));
    EXPECT_FALSE(success);
    EXPECT_FALSE(replaced);
    EXPECT_EQ(0U, m_table.get_binary_diffs(&m_field)->size());
    m_table.clear_partial_update_diffs();

    // Try with a valid path. Expect success.
    EXPECT_FALSE(doc.attempt_binary_update(&m_field, array_accessor(0),
                                           &new_value, true, &buffer, &success,
                                           &replaced));
    EXPECT_TRUE(success);
    EXPECT_TRUE(replaced);
    EXPECT_NE(0U, m_table.get_binary_diffs(&m_field)->size());

    String str;
    new_value.to_string(&str, true, "test");
    // Verify the updated document.
    {
      Json_array_ptr a(new (std::nothrow) Json_array);
      a->append_clone(dom);
      EXPECT_EQ(0, doc.compare(Json_wrapper(std::move(a))));
    }

    // Verify the binary diffs.
    EXPECT_EQ(TYPE_OK, m_field.store_json(&doc));
    verify_binary_diffs(&m_field, m_table.get_binary_diffs(&m_field), original,
                        buffer);

    delete dom;
  }
}

/**
  Test attempt_binary_update() with invalid input.
*/
TEST_F(JsonDomTest, AttemptBinaryUpdate_Error) {
  EXPECT_FALSE(m_table.mark_column_for_partial_update(&m_field));
  EXPECT_FALSE(m_table.setup_partial_update(true));
  Json_wrapper doc(parse_json("[1,2,3,4]"));
  EXPECT_EQ(TYPE_OK, m_field.store_json(&doc));
  EXPECT_FALSE(m_field.val_json(&doc));

  /*
    Create an invalid JSON value, which we will attempt to add to the document.
  */
  Json_wrapper error;
  EXPECT_TRUE(error.empty());
  EXPECT_EQ(enum_json_type::J_ERROR, error.type());

  // Expect the calls to attempt_binary_update() to fail.
  const auto old_error_handler_hook = error_handler_hook;
  error_handler_hook = my_message_sql;
  String buffer;
  bool success;
  bool replaced;
  {
    my_testing::Mock_error_handler handler(thd(), ER_INVALID_JSON_BINARY_DATA);
    EXPECT_TRUE(doc.attempt_binary_update(&m_field, array_accessor(0), &error,
                                          true, &buffer, &success, &replaced));
    EXPECT_EQ(1, handler.handle_called());
  }
  {
    my_testing::Mock_error_handler handler(thd(), ER_INVALID_JSON_BINARY_DATA);
    EXPECT_TRUE(doc.attempt_binary_update(&m_field, array_accessor(0), &error,
                                          false, &buffer, &success, &replaced));
    EXPECT_EQ(1, handler.handle_called());
  }
  error_handler_hook = old_error_handler_hook;
}

/**
  Test that applying a set of diffs on a JSON document gives the expected
  return status.
*/
void test_apply_json_diffs(Field_json *field, const Json_diff_vector &diffs,
                           const char *orig_json,
                           enum_json_diff_status expected_status) {
  SCOPED_TRACE("test_apply_json_diffs");
  field->table->clear_partial_update_diffs();
  if (orig_json == nullptr) {
    EXPECT_EQ(TYPE_OK, set_field_to_null(field));
  } else {
    field->set_notnull();
    Json_wrapper doc(parse_json(orig_json).release());
    EXPECT_FALSE(field->store_json(&doc));
  }
  EXPECT_EQ(orig_json == nullptr, field->is_null());
  EXPECT_EQ(expected_status, apply_json_diffs(field, &diffs));
}

/**
  Test that applying a set of diffs on a JSON document is successful and gives
  the expected results.
*/
void test_apply_json_diffs(Field_json *field, const Json_diff_vector &diffs,
                           const char *orig_json, const char *new_json) {
  SCOPED_TRACE("test_apply_json_diffs");
  test_apply_json_diffs(field, diffs, orig_json,
                        enum_json_diff_status::SUCCESS);
  Json_wrapper doc;
  EXPECT_FALSE(field->val_json(&doc));
  EXPECT_EQ(0, doc.compare(Json_wrapper(parse_json(new_json).release())));

  TABLE *table = field->table;
  if (table->is_binary_diff_enabled(field)) {
    StringBuffer<STRING_BUFFER_USUAL_SIZE> original;
    EXPECT_FALSE(json_binary::serialize(
        table->in_use, parse_json(orig_json).get(), &original));
    StringBuffer<STRING_BUFFER_USUAL_SIZE> updated;
    EXPECT_FALSE(doc.to_binary(table->in_use, &updated));
    verify_binary_diffs(field, table->get_binary_diffs(field), original,
                        updated);
  }
}

static void do_apply_json_diffs_tests(Field_json *field) {
  Json_diff_vector diffs(
      Json_diff_vector::allocator_type(field->table->in_use->mem_root));

  const auto expect_rejected = [field, &diffs](const char *json_text) {
    SCOPED_TRACE("");
    test_apply_json_diffs(field, diffs, json_text,
                          enum_json_diff_status::REJECTED);
  };

  const auto expect_success = [field, &diffs](const char *orig_json,
                                              const char *new_json) {
    SCOPED_TRACE("");
    test_apply_json_diffs(field, diffs, orig_json, new_json);
  };

  {
    SCOPED_TRACE("");
    diffs.clear();
    expect_success("[1,2,3]", "[1,2,3]");
    expect_rejected(nullptr);
  }

  {
    SCOPED_TRACE("");
    diffs.clear();
    diffs.add_diff(parse_path("$.a"), enum_json_diff_operation::REPLACE,
                   create_dom_ptr<Json_int>(3));
    expect_success("{\"a\": 1, \"b\": 2}", "{\"a\": 3, \"b\": 2}");
    expect_rejected("{\"b\": 2}");
    expect_rejected("[1,2,3]");
    expect_rejected("123");
    expect_rejected(nullptr);
  }

  {
    SCOPED_TRACE("");
    diffs.clear();
    diffs.add_diff(parse_path("$.a[1]"), enum_json_diff_operation::REPLACE,
                   create_dom_ptr<Json_int>(3));
    expect_success("{\"a\": [1,2], \"b\": 2}", "{\"a\":[1,3], \"b\": 2}");
    expect_rejected("{\"a\": 2}");
    expect_rejected("{\"b\": 2}");
    expect_rejected("{\"a\": {\"b\":2}}");
    expect_rejected("[1,2,3]");
    expect_rejected("123");
    expect_rejected(nullptr);
  }

  {
    SCOPED_TRACE("");
    diffs.clear();
    diffs.add_diff(parse_path("$.a[2]"), enum_json_diff_operation::INSERT,
                   create_dom_ptr<Json_int>(3));
    expect_success("{\"a\":[]}", "{\"a\":[3]}");
    expect_success("{\"a\":[1]}", "{\"a\":[1,3]}");
    expect_success("{\"a\":[1,2]}", "{\"a\":[1,2,3]}");
    expect_success("{\"a\":[1,2,4]}", "{\"a\":[1,2,3,4]}");
    expect_rejected("{\"a\": 1, \"b\": 2}");
    expect_rejected("[]");
    expect_rejected(nullptr);
  }

  {
    SCOPED_TRACE("");
    diffs.clear();
    diffs.add_diff(parse_path("$.a.b"), enum_json_diff_operation::INSERT,
                   create_dom_ptr<Json_int>(3));
    expect_success("{\"a\":{\"c\":1}}", "{\"a\":{\"b\":3,\"c\":1}}");
    expect_rejected("{}");
    expect_rejected("[]");
    expect_rejected("{\"a\":{\"b\":1}}");
    expect_rejected(nullptr);
  }

  {
    SCOPED_TRACE("");
    diffs.clear();
    diffs.add_diff(parse_path("$.a.b"), enum_json_diff_operation::REMOVE);
    expect_success("{\"a\":{\"b\":3,\"c\":1}}", "{\"a\":{\"c\":1}}");
    expect_rejected("{}");
    expect_rejected("[]");
    expect_rejected(nullptr);
  }

  {
    SCOPED_TRACE("");
    diffs.clear();
    diffs.add_diff(parse_path("$[2]"), enum_json_diff_operation::REMOVE);
    expect_success("[1,2,3,4]", "[1,2,4]");
    expect_success("[1,2,3]", "[1,2]");
    expect_rejected("[1,2]");
    expect_rejected("[]");
    expect_rejected("{}");
    expect_rejected(nullptr);
  }

  {
    SCOPED_TRACE("");
    diffs.clear();
    diffs.add_diff(parse_path("$[2]"), enum_json_diff_operation::REMOVE);
    diffs.add_diff(parse_path("$[3]"), enum_json_diff_operation::REMOVE);
    expect_success("[1,2,3,4,5,6]", "[1,2,4,6]");
    expect_rejected("[1,2,3,4]");
    expect_rejected("[]");
    expect_rejected(nullptr);
  }

  {
    SCOPED_TRACE("");
    diffs.clear();
    diffs.add_diff(parse_path("$[2][3]"), enum_json_diff_operation::REMOVE);
    expect_success("[1,2,[3,4,5,6,7]]", "[1,2,[3,4,5,7]]");
    expect_rejected("[]");
    expect_rejected("[1,2,3,4,5,6]");
    expect_rejected(nullptr);
  }

  {
    SCOPED_TRACE("");
    diffs.clear();
    diffs.add_diff(parse_path("$[0][0]"), enum_json_diff_operation::REMOVE);
    expect_success("[[1]]", "[[]]");
    expect_success("[[1,2,3],4,5]", "[[2,3],4,5]");
    expect_rejected("[1]");
    expect_rejected("{\"a\":1}");
    expect_rejected("[{\"a\":1}]");
    expect_rejected(nullptr);
  }

  {
    SCOPED_TRACE("");
    for (auto op :
         {enum_json_diff_operation::REPLACE, enum_json_diff_operation::INSERT,
          enum_json_diff_operation::REMOVE}) {
      diffs.clear();
      diffs.add_diff(parse_path("$"), op, create_dom_ptr<Json_int>(1));
      expect_rejected("[1,2,3]");
      expect_rejected(nullptr);
    }
  }
}

TEST_F(JsonDomTest, ApplyJsonDiffs) { do_apply_json_diffs_tests(&m_field); }

TEST_F(JsonDomTest, ApplyJsonDiffs_CollectBinaryDiffs) {
  EXPECT_FALSE(m_table.mark_column_for_partial_update(&m_field));
  EXPECT_FALSE(m_table.setup_partial_update(true));
  do_apply_json_diffs_tests(&m_field);
}

/**
  Run a microbenchmarks that tests how fast Json_wrapper::seek() is on
  a wrapper that wraps a Json_dom.

  @param num_iterations   the number of iterations to run
  @param path             the JSON path to search for
  @param need_only_one    true if the search should stop after the first match
  @param expected_matches the number of expected matches
*/
static void benchmark_dom_seek(size_t num_iterations, const Json_path &path,
                               bool need_only_one, size_t expected_matches) {
  StopBenchmarkTiming();

  Json_object o;
  for (size_t i = 0; i < 1000; ++i)
    o.add_alias(std::to_string(i), new (std::nothrow) Json_object());

  Json_wrapper wr(&o);
  wr.set_alias();

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    Json_wrapper_vector hits(PSI_NOT_INSTRUMENTED);
    wr.seek(path, path.leg_count(), &hits, true, need_only_one);
    EXPECT_EQ(expected_matches, hits.size());
  }

  StopBenchmarkTiming();
}

/**
  Microbenchmark which tests how fast a lookup with an ellipsis is in
  a wrapper that wraps a Json_dom.
*/
static void BM_JsonDomSearchEllipsis(size_t num_iterations) {
  benchmark_dom_seek(num_iterations, parse_path("$**.\"432\""), false, 1);
}
BENCHMARK(BM_JsonDomSearchEllipsis)

/**
  Microbenchmark which tests how fast a lookup with an ellipsis is in
  a wrapper that wraps a Json_dom, with the `need_only_one` flag set.
*/
static void BM_JsonDomSearchEllipsis_OnlyOne(size_t num_iterations) {
  benchmark_dom_seek(num_iterations, parse_path("$**.\"432\""), true, 1);
}
BENCHMARK(BM_JsonDomSearchEllipsis_OnlyOne)

/**
  Microbenchmark which tests how fast a lookup of a JSON object member
  is in a wrapper that wraps a Json_dom.
*/
static void BM_JsonDomSearchKey(size_t num_iterations) {
  benchmark_dom_seek(num_iterations, parse_path("$.\"432\""), false, 1);
}
BENCHMARK(BM_JsonDomSearchKey)

/**
  Run a microbenchmarks that tests how fast Json_wrapper::seek() is on
  a wrapper that wraps a binary JSON value.

  @param num_iterations   the number of iterations to run
  @param path             the JSON path to search for
  @param need_only_one    true if the search should stop after the first match
  @param expected_matches the number of expected matches
*/
static void benchmark_binary_seek(size_t num_iterations, const Json_path &path,
                                  bool need_only_one, size_t expected_matches) {
  StopBenchmarkTiming();

  Json_object o;
  for (size_t i = 0; i < 1000; ++i)
    o.add_alias(std::to_string(i), new (std::nothrow) Json_object());

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  String buffer;
  EXPECT_FALSE(json_binary::serialize(initializer.thd(), &o, &buffer));
  json_binary::Value val =
      json_binary::parse_binary(buffer.ptr(), buffer.length());

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    Json_wrapper wr(val);
    Json_wrapper_vector hits(PSI_NOT_INSTRUMENTED);
    wr.seek(path, path.leg_count(), &hits, true, need_only_one);
    EXPECT_EQ(expected_matches, hits.size());
  }

  StopBenchmarkTiming();

  initializer.TearDown();
}

/**
  Microbenchmark which tests how fast a lookup with an ellipsis is in
  a Json_wrapper which wraps a binary JSON value.
*/
static void BM_JsonBinarySearchEllipsis(size_t num_iterations) {
  benchmark_binary_seek(num_iterations, parse_path("$**.\"432\""), false, 1);
}
BENCHMARK(BM_JsonBinarySearchEllipsis)

/**
  Microbenchmark which tests how fast a lookup with an ellipsis is in
  a Json_wrapper which wraps a binary JSON value, with the `need_only_one`
  flag set.
*/
static void BM_JsonBinarySearchEllipsis_OnlyOne(size_t num_iterations) {
  benchmark_binary_seek(num_iterations, parse_path("$**.\"432\""), true, 1);
}
BENCHMARK(BM_JsonBinarySearchEllipsis_OnlyOne)

/**
  Microbenchmark which tests how fast a lookup of a JSON object member
  is in a Json_wrapper which wraps a binary JSON value.
*/
static void BM_JsonBinarySearchKey(size_t num_iterations) {
  benchmark_binary_seek(num_iterations, parse_path("$.\"432\""), false, 1);
}
BENCHMARK(BM_JsonBinarySearchKey)

/**
  Microbenchmark which tests the performance of
  Json_wrapper::to_string() when it's called on a JSON string value
  with no special characters that need quoting.
*/
static void BM_JsonStringToString_Plain(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  const Json_string str("This is a plain string with no special characters!");
  const Json_wrapper wr(str.clone());
  const size_t quoted_length = str.size() + 2;

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    String buf;
    wr.to_string(&buf, true, "test");
    EXPECT_EQ(quoted_length, buf.length());
  }

  StopBenchmarkTiming();

  initializer.TearDown();
}
BENCHMARK(BM_JsonStringToString_Plain)

/**
  Microbenchmark which tests the performance of
  Json_wrapper::to_string() when it's called on a JSON string value
  which contains some special characters that need quoting.
*/
static void BM_JsonStringToString_SpecialChars(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  const Json_string str("This\nstring\nspans\nmultiple\nlines.\f\nabc\x1Dxyz");
  const Json_wrapper wr(str.clone());

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    String buf;
    wr.to_string(&buf, true, "test");
    EXPECT_LT(str.size(), buf.length());
  }

  StopBenchmarkTiming();

  initializer.TearDown();
}
BENCHMARK(BM_JsonStringToString_SpecialChars)

/**
  Microbenchmark which tests the performance of
  Json_wrapper::to_string() when it's called on a JSON object with
  nested values.
*/
static void BM_JsonObjectToString(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  const Json_wrapper wr(
      parse_json("{\"name\": \"John Doe\", \"age\": 42, "
                 "\"points\": [1, 3.14e0, 2.7, null], "
                 "\"id\": \"xyzxyzxyzxyz\"}"));

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    String buf;
    wr.to_string(&buf, true, "test");
    EXPECT_LT(0U, buf.length());
  }

  StopBenchmarkTiming();

  initializer.TearDown();
}
BENCHMARK(BM_JsonObjectToString)

/**
  Tests the performance of Json_wrapper::to_string() when it's called
  on a JSON array with boolean elements.
*/
static void BM_JsonBooleanArrayToString(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  Json_array_ptr array = create_dom_ptr<Json_array>();
  for (size_t i = 0; i < 1000; ++i) {
    array->append_alias(create_dom_ptr<Json_boolean>(i % 2 == 0));
  }
  Json_wrapper wrapper(std::move(array));

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    String buf;
    wrapper.to_string(&buf, true, "test");
    EXPECT_LT(0U, buf.length());
  }

  StopBenchmarkTiming();

  initializer.TearDown();
}
BENCHMARK(BM_JsonBooleanArrayToString)

/**
  Tests the performance of Json_wrapper::to_string() when it's called
  on a JSON array with double elements.
*/
static void BM_JsonDoubleArrayToString(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  Json_array_ptr array = create_dom_ptr<Json_array>();
  for (size_t i = 0; i < 1000; ++i) {
    array->append_alias(create_dom_ptr<Json_double>(i));
  }
  Json_wrapper wrapper(std::move(array));

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    String buf;
    wrapper.to_string(&buf, true, "test");
    EXPECT_LT(0U, buf.length());
  }

  StopBenchmarkTiming();

  initializer.TearDown();
}
BENCHMARK(BM_JsonDoubleArrayToString)

/**
  Tests the performance of Json_wrapper::to_string() when it's called
  on a JSON array with decimal elements.
*/
static void BM_JsonDecimalArrayToString(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  Json_array_ptr array = create_dom_ptr<Json_array>();
  for (size_t i = 0; i < 1000; ++i) {
    my_decimal decimal;
    EXPECT_FALSE(double2my_decimal(0, i, &decimal));
    array->append_alias(create_dom_ptr<Json_decimal>(decimal));
  }
  Json_wrapper wrapper(std::move(array));

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    String buf;
    wrapper.to_string(&buf, true, "test");
    EXPECT_LT(0U, buf.length());
  }

  StopBenchmarkTiming();

  initializer.TearDown();
}
BENCHMARK(BM_JsonDecimalArrayToString)

/**
  Tests the performance of Json_wrapper::to_string() when it's called
  on a JSON array with date elements.
*/
static void BM_JsonDateArrayToString(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  Json_array_ptr array = create_dom_ptr<Json_array>();
  MysqlTime date(2018, 11, 20, 0, 0, 0, 0, false, MYSQL_TIMESTAMP_DATE);
  for (size_t i = 0; i < 1000; ++i) {
    array->append_alias(create_dom_ptr<Json_datetime>(date, MYSQL_TYPE_DATE));
  }
  Json_wrapper wrapper(std::move(array));

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    String buf;
    wrapper.to_string(&buf, true, "test");
    EXPECT_LT(0U, buf.length());
  }

  StopBenchmarkTiming();

  initializer.TearDown();
}
BENCHMARK(BM_JsonDateArrayToString)

/**
  Tests the performance of Json_wrapper_object_iterator when iterating
  over a JSON object in the DOM representation.
*/
static void BM_JsonWrapperObjectIteratorDOM(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  Json_object object;
  for (size_t i = 0; i < 1000; ++i) {
    object.add_alias("JSON object member number " + std::to_string(i),
                     create_dom_ptr<Json_null>());
  }

  Json_wrapper wrapper(&object);
  wrapper.set_alias();

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    for (const auto &it : Json_object_wrapper(wrapper)) {
      EXPECT_NE(0U, it.first.length);
      EXPECT_EQ(enum_json_type::J_NULL, it.second.type());
    }
  }

  StopBenchmarkTiming();

  initializer.TearDown();
}
BENCHMARK(BM_JsonWrapperObjectIteratorDOM)

/**
  Tests the performance of Json_wrapper_object_iterator when iterating
  over a JSON object in the binary representation.
*/
static void BM_JsonWrapperObjectIteratorBinary(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  Json_object dom_object;
  for (size_t i = 0; i < 1000; ++i) {
    dom_object.add_alias("JSON object member number " + std::to_string(i),
                         create_dom_ptr<Json_null>());
  }

  String serialized_object;
  EXPECT_FALSE(json_binary::serialize(initializer.thd(), &dom_object,
                                      &serialized_object));
  Json_wrapper wrapper(json_binary::parse_binary(serialized_object.ptr(),
                                                 serialized_object.length()));
  EXPECT_EQ(enum_json_type::J_OBJECT, wrapper.type());

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    for (const auto &it : Json_object_wrapper(wrapper)) {
      EXPECT_NE(0U, it.first.length);
      EXPECT_EQ(enum_json_type::J_NULL, it.second.type());
    }
  }

  StopBenchmarkTiming();

  initializer.TearDown();
}
BENCHMARK(BM_JsonWrapperObjectIteratorBinary)

}  // namespace json_dom_unittest
