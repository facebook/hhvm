/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <string>

#include "my_byteorder.h"
#include "my_inttypes.h"
#include "sql/error_handler.h"
#include "sql/json_binary.h"
#include "sql/json_dom.h"
#include "sql/sql_class.h"
#include "sql/sql_time.h"
#include "sql_string.h"
#include "unittest/gunit/benchmark.h"
#include "unittest/gunit/test_utils.h"

namespace json_binary_unittest {

using namespace json_binary;

class JsonBinaryTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }
  my_testing::Server_initializer initializer;
  THD *thd() const { return initializer.thd(); }
};

/**
  Get a copy of the string value represented by val.
*/
static std::string get_string(const Value &val) {
  return std::string(val.get_data(), val.get_data_length());
}

static my_decimal create_decimal(double d) {
  my_decimal dec;
  EXPECT_EQ(E_DEC_OK, double2my_decimal(E_DEC_FATAL_ERROR, d, &dec));
  return dec;
}

static Json_dom_ptr parse_json(const char *json_text) {
  auto dom =
      Json_dom::parse(json_text, strlen(json_text), false, nullptr, nullptr);
  EXPECT_NE(nullptr, dom);
  return dom;
}

TEST_F(JsonBinaryTest, BasicTest) {
  Json_dom_ptr dom = parse_json("false");
  String buf;
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val1 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val1.is_valid());
  EXPECT_EQ(Value::LITERAL_FALSE, val1.type());

  dom = parse_json("-123");
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val2 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val2.is_valid());
  EXPECT_EQ(Value::INT, val2.type());
  EXPECT_EQ(-123LL, val2.get_int64());

  dom = parse_json("3.14");
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val3 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val3.is_valid());
  EXPECT_EQ(Value::DOUBLE, val3.type());
  EXPECT_EQ(3.14, val3.get_double());

  dom = parse_json("18446744073709551615");
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val4 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val4.is_valid());
  EXPECT_EQ(Value::UINT, val4.type());
  EXPECT_EQ(18446744073709551615ULL, val4.get_uint64());

  dom = parse_json("\"abc\"");
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val5 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val5.is_valid());
  EXPECT_EQ(Value::STRING, val5.type());
  EXPECT_EQ("abc", get_string(val5));

  dom = parse_json("[ 1, 2, 3 ]");
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val6 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val6.is_valid());
  EXPECT_EQ(Value::ARRAY, val6.type());
  EXPECT_FALSE(val6.large_format());
  EXPECT_EQ(3U, val6.element_count());
  for (int i = 0; i < 3; i++) {
    Value v = val6.element(i);
    EXPECT_EQ(Value::INT, v.type());
    EXPECT_EQ(i + 1, v.get_int64());
  }
  EXPECT_EQ(Value::ERROR, val6.element(3).type());

  dom = parse_json("[ 1, [ \"a\", [ 3.14 ] ] ]");
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  // Top-level doc is an array of size 2.
  Value val7 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val7.is_valid());
  EXPECT_EQ(Value::ARRAY, val7.type());
  EXPECT_EQ(2U, val7.element_count());
  // First element is the integer 1.
  Value v7_1 = val7.element(0);
  EXPECT_TRUE(v7_1.is_valid());
  EXPECT_EQ(Value::INT, v7_1.type());
  EXPECT_EQ(1, v7_1.get_int64());
  // The second element is a nested array of size 2.
  Value v7_2 = val7.element(1);
  EXPECT_TRUE(v7_2.is_valid());
  EXPECT_EQ(Value::ARRAY, v7_2.type());
  EXPECT_EQ(2U, v7_2.element_count());
  // The first element of the nested array is the string "a".
  Value v7_3 = v7_2.element(0);
  EXPECT_TRUE(v7_3.is_valid());
  EXPECT_EQ(Value::STRING, v7_3.type());
  EXPECT_EQ("a", get_string(v7_3));
  // The second element of the nested array is yet another array.
  Value v7_4 = v7_2.element(1);
  EXPECT_TRUE(v7_4.is_valid());
  EXPECT_EQ(Value::ARRAY, v7_4.type());
  // The second nested array has one element, the double 3.14.
  EXPECT_EQ(1U, v7_4.element_count());
  Value v7_5 = v7_4.element(0);
  EXPECT_TRUE(v7_5.is_valid());
  EXPECT_EQ(Value::DOUBLE, v7_5.type());
  EXPECT_EQ(3.14, v7_5.get_double());

  dom = parse_json("{\"key\" : \"val\"}");
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val8 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val8.is_valid());
  EXPECT_EQ(Value::OBJECT, val8.type());
  EXPECT_FALSE(val8.large_format());
  EXPECT_EQ(1U, val8.element_count());
  Value val8_k = val8.key(0);
  EXPECT_TRUE(val8_k.is_valid());
  EXPECT_EQ(Value::STRING, val8_k.type());
  EXPECT_EQ("key", get_string(val8_k));
  Value val8_v = val8.element(0);
  EXPECT_TRUE(val8_v.is_valid());
  EXPECT_EQ(Value::STRING, val8_v.type());
  EXPECT_EQ("val", get_string(val8_v));
  EXPECT_EQ(Value::ERROR, val8.key(1).type());
  EXPECT_EQ(Value::ERROR, val8.element(1).type());

  Value v8_v1 = val8.lookup("key");
  EXPECT_EQ(Value::STRING, v8_v1.type());
  EXPECT_TRUE(v8_v1.is_valid());
  EXPECT_EQ("val", get_string(v8_v1));

  dom = parse_json("{ \"a\" : \"b\", \"c\" : [ \"d\" ] }");
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val9 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val9.is_valid());
  EXPECT_EQ(Value::OBJECT, val9.type());
  EXPECT_EQ(2U, val9.element_count());
  Value v9_k1 = val9.key(0);
  EXPECT_EQ(Value::STRING, v9_k1.type());
  EXPECT_EQ("a", get_string(v9_k1));
  Value v9_v1 = val9.element(0);
  EXPECT_EQ(Value::STRING, v9_v1.type());
  EXPECT_EQ("b", get_string(v9_v1));
  Value v9_k2 = val9.key(1);
  EXPECT_EQ(Value::STRING, v9_k2.type());
  EXPECT_EQ("c", get_string(v9_k2));
  Value v9_v2 = val9.element(1);
  EXPECT_EQ(Value::ARRAY, v9_v2.type());
  EXPECT_EQ(1U, v9_v2.element_count());
  Value v9_v2_1 = v9_v2.element(0);
  EXPECT_EQ(Value::STRING, v9_v2_1.type());
  EXPECT_EQ("d", get_string(v9_v2_1));

  EXPECT_EQ("b", get_string(val9.lookup("a")));
  Value v9_c = val9.lookup("c");
  EXPECT_EQ(Value::ARRAY, v9_c.type());
  EXPECT_EQ(1U, v9_c.element_count());
  Value v9_c1 = v9_c.element(0);
  EXPECT_EQ(Value::STRING, v9_c1.type());
  EXPECT_EQ("d", get_string(v9_c1));

  char blob[4];
  int4store(blob, 0xCAFEBABEU);
  Json_opaque opaque(MYSQL_TYPE_TINY_BLOB, blob, 4);
  EXPECT_FALSE(serialize(thd(), &opaque, &buf));
  Value val10 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val10.is_valid());
  EXPECT_EQ(Value::OPAQUE, val10.type());
  EXPECT_EQ(MYSQL_TYPE_TINY_BLOB, val10.field_type());
  EXPECT_EQ(4U, val10.get_data_length());
  EXPECT_EQ(0xCAFEBABEU, uint4korr(val10.get_data()));

  dom = parse_json("[true,false,null,0,\"0\",\"\",{},[]]");
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val11 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val11.is_valid());
  EXPECT_EQ(Value::ARRAY, val11.type());
  EXPECT_EQ(8U, val11.element_count());
  EXPECT_EQ(Value::LITERAL_TRUE, val11.element(0).type());
  EXPECT_EQ(Value::LITERAL_FALSE, val11.element(1).type());
  EXPECT_EQ(Value::LITERAL_NULL, val11.element(2).type());
  EXPECT_EQ(Value::INT, val11.element(3).type());
  EXPECT_EQ(Value::STRING, val11.element(4).type());
  EXPECT_EQ(Value::STRING, val11.element(5).type());
  EXPECT_EQ(Value::OBJECT, val11.element(6).type());
  EXPECT_EQ(Value::ARRAY, val11.element(7).type());
  EXPECT_EQ(0, val11.element(3).get_int64());
  EXPECT_EQ("0", get_string(val11.element(4)));
  EXPECT_EQ("", get_string(val11.element(5)));
  EXPECT_EQ(0U, val11.element(6).element_count());
  EXPECT_EQ(0U, val11.element(7).element_count());

  dom = parse_json("{}");
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val12 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val12.is_valid());
  EXPECT_EQ(Value::OBJECT, val12.type());
  EXPECT_EQ(0U, val12.element_count());
  EXPECT_EQ(Value::ERROR, val12.lookup("").type());
  EXPECT_EQ(Value::ERROR, val12.lookup("key").type());
  EXPECT_FALSE(val12.lookup("no such key").is_valid());

  dom = parse_json("[]");
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val13 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val13.is_valid());
  EXPECT_EQ(Value::ARRAY, val13.type());
  EXPECT_EQ(0U, val13.element_count());

  dom = parse_json(
      "{\"key1\":1, \"key2\":2, \"key1\":3, \"key1\\u0000x\":4, "
      "\"key1\\u0000y\":5, \"a\":6, \"ab\":7, \"b\":8, \"\":9, "
      "\"\":10}");
  const std::string expected_keys[] = {"",
                                       "a",
                                       "b",
                                       "ab",
                                       "key1",
                                       "key2",
                                       std::string("key1\0x", 6),
                                       std::string("key1\0y", 6)};
  const int64 expected_values[] = {10, 6, 8, 7, 3, 2, 4, 5};
  EXPECT_FALSE(serialize(thd(), dom.get(), &buf));
  Value val14 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val14.is_valid());
  EXPECT_EQ(Value::OBJECT, val14.type());
  EXPECT_EQ(8U, val14.element_count());
  for (size_t i = 0; i < val14.element_count(); i++) {
    EXPECT_EQ(expected_keys[i], get_string(val14.key(i)));

    Value val = val14.element(i);
    EXPECT_EQ(Value::INT, val.type());
    EXPECT_EQ(expected_values[i], val.get_int64());

    Value val_lookup = val14.lookup(expected_keys[i]);
    EXPECT_EQ(Value::INT, val_lookup.type());
    EXPECT_EQ(expected_values[i], val_lookup.get_int64());
  }

  // Store a decimal.
  my_decimal md = create_decimal(123.45);
  EXPECT_EQ(5U, md.precision());
  EXPECT_EQ(2, md.frac);

  Json_decimal jd(md);
  EXPECT_FALSE(serialize(thd(), &jd, &buf));
  Value val15 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val15.is_valid());
  EXPECT_EQ(Value::OPAQUE, val15.type());
  EXPECT_EQ(MYSQL_TYPE_NEWDECIMAL, val15.field_type());

  my_decimal md_out;
  EXPECT_FALSE(Json_decimal::convert_from_binary(
      val15.get_data(), val15.get_data_length(), &md_out));
  EXPECT_EQ(5U, md_out.precision());
  EXPECT_EQ(2, md_out.frac);
  double d_out;
  EXPECT_EQ(E_DEC_OK, my_decimal2double(E_DEC_FATAL_ERROR, &md_out, &d_out));
  EXPECT_EQ(123.45, d_out);
}

TEST_F(JsonBinaryTest, EmptyDocument) {
  /*
    An empty binary document is interpreted as the JSON null literal.
    This is a special case to handle NULL values inserted into NOT
    NULL columns using INSERT IGNORE or similar mechanisms.
  */
  Value val = parse_binary("", 0);
  EXPECT_EQ(Value::LITERAL_NULL, val.type());
}

static MYSQL_TIME create_time() {
  const char *tstr = "13:14:15.654321";
  MYSQL_TIME t;
  MYSQL_TIME_STATUS status;
  EXPECT_FALSE(
      str_to_time(&my_charset_utf8mb4_bin, tstr, strlen(tstr), &t, 0, &status));
  return t;
}

static MYSQL_TIME create_date() {
  const char *dstr = "20140517";
  MYSQL_TIME d;
  MYSQL_TIME_STATUS status;
  EXPECT_FALSE(str_to_datetime(&my_charset_utf8mb4_bin, dstr, strlen(dstr), &d,
                               0, &status));
  return d;
}

static MYSQL_TIME create_datetime() {
  const char *dtstr = "2015-01-15 15:16:17.123456";
  MYSQL_TIME dt;
  MYSQL_TIME_STATUS status;
  EXPECT_FALSE(str_to_datetime(&my_charset_utf8mb4_bin, dtstr, strlen(dtstr),
                               &dt, 0, &status));
  return dt;
}

/*
  Test storing of TIME, DATE and DATETIME.
*/
TEST_F(JsonBinaryTest, DateAndTimeTest) {
  // Create an array that contains a TIME, a DATE and a DATETIME.
  Json_array array;
  Json_datetime tt(create_time(), MYSQL_TYPE_TIME);
  Json_datetime td(create_date(), MYSQL_TYPE_DATE);
  Json_datetime tdt(create_datetime(), MYSQL_TYPE_DATETIME);
  array.append_clone(&tt);
  array.append_clone(&td);
  array.append_clone(&tdt);

  // Store the array ...
  String buf;
  EXPECT_FALSE(serialize(thd(), &array, &buf));

  // ... and read it back.
  Value val = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val.is_valid());
  EXPECT_EQ(Value::ARRAY, val.type());
  EXPECT_EQ(3U, val.element_count());

  // The first element should be the TIME "13:14:15.654321".
  Value t_val = val.element(0);
  EXPECT_EQ(Value::OPAQUE, t_val.type());
  EXPECT_EQ(MYSQL_TYPE_TIME, t_val.field_type());
  const size_t json_datetime_packed_size = Json_datetime::PACKED_SIZE;
  EXPECT_EQ(json_datetime_packed_size, t_val.get_data_length());
  MYSQL_TIME t_out;
  Json_datetime::from_packed(t_val.get_data(), t_val.field_type(), &t_out);
  EXPECT_EQ(13U, t_out.hour);
  EXPECT_EQ(14U, t_out.minute);
  EXPECT_EQ(15U, t_out.second);
  EXPECT_EQ(654321U, t_out.second_part);
  EXPECT_FALSE(t_out.neg);
  EXPECT_EQ(MYSQL_TIMESTAMP_TIME, t_out.time_type);

  // The second element should be the DATE "2014-05-17".
  Value d_val = val.element(1);
  EXPECT_EQ(Value::OPAQUE, d_val.type());
  EXPECT_EQ(MYSQL_TYPE_DATE, d_val.field_type());
  EXPECT_EQ(json_datetime_packed_size, d_val.get_data_length());
  MYSQL_TIME d_out;
  Json_datetime::from_packed(d_val.get_data(), d_val.field_type(), &d_out);
  EXPECT_EQ(2014U, d_out.year);
  EXPECT_EQ(5U, d_out.month);
  EXPECT_EQ(17U, d_out.day);
  EXPECT_FALSE(d_out.neg);
  EXPECT_EQ(MYSQL_TIMESTAMP_DATE, d_out.time_type);

  // The third element should be the DATETIME "2015-01-15 15:16:17.123456".
  Value dt_val = val.element(2);
  EXPECT_EQ(Value::OPAQUE, dt_val.type());
  EXPECT_EQ(MYSQL_TYPE_DATETIME, dt_val.field_type());
  EXPECT_EQ(json_datetime_packed_size, dt_val.get_data_length());
  MYSQL_TIME dt_out;
  Json_datetime::from_packed(dt_val.get_data(), dt_val.field_type(), &dt_out);
  EXPECT_EQ(2015U, dt_out.year);
  EXPECT_EQ(1U, dt_out.month);
  EXPECT_EQ(15U, dt_out.day);
  EXPECT_EQ(15U, dt_out.hour);
  EXPECT_EQ(16U, dt_out.minute);
  EXPECT_EQ(17U, dt_out.second);
  EXPECT_EQ(123456U, dt_out.second_part);
  EXPECT_FALSE(dt_out.neg);
  EXPECT_EQ(MYSQL_TIMESTAMP_DATETIME, dt_out.time_type);
}

/*
  Validate that the contents of an array are as expected. The array
  should contain values that alternate between literal true, literal
  false, literal null and the string "a".
*/
void validate_array_contents(const Value &array, size_t expected_size) {
  EXPECT_EQ(Value::ARRAY, array.type());
  EXPECT_TRUE(array.is_valid());
  EXPECT_EQ(expected_size, array.element_count());
  for (size_t i = 0; i < array.element_count(); i++) {
    Value val = array.element(i);
    EXPECT_TRUE(val.is_valid());
    Value::enum_type t = val.type();
    if (i % 4 == 0)
      EXPECT_EQ(Value::LITERAL_TRUE, t);
    else if (i % 4 == 1)
      EXPECT_EQ(Value::LITERAL_FALSE, t);
    else if (i % 4 == 2)
      EXPECT_EQ(Value::LITERAL_NULL, t);
    else {
      EXPECT_EQ(Value::STRING, t);
      EXPECT_EQ("a", get_string(val));
    }
  }
}

/*
  Test some arrays and objects that exceed 64KB. Arrays and objects
  are stored in a different format if more than two bytes are required
  for the internal offsets.
*/
TEST_F(JsonBinaryTest, LargeDocumentTest) {
  Json_array array;
  Json_boolean literal_true(true);
  Json_boolean literal_false(false);
  Json_null literal_null;
  Json_string string("a");

  for (int i = 0; i < 20000; i++) {
    array.append_clone(&literal_true);
    array.append_clone(&literal_false);
    array.append_clone(&literal_null);
    array.append_clone(&string);
  }
  EXPECT_EQ(80000U, array.size());

  String buf;
  EXPECT_FALSE(serialize(thd(), &array, &buf));
  Value val1 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val1.large_format());
  {
    SCOPED_TRACE("");
    validate_array_contents(val1, array.size());
  }

  /*
    Extract the raw binary representation of the large array, and verify
    that it is valid.
  */
  String raw;
  EXPECT_FALSE(val1.raw_binary(thd(), &raw));
  {
    SCOPED_TRACE("");
    validate_array_contents(parse_binary(raw.ptr(), raw.length()),
                            array.size());
  }

  Json_array array2;
  array2.append_clone(&array);
  array2.append_clone(&array);
  EXPECT_FALSE(serialize(thd(), &array2, &buf));
  Value val2 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val2.is_valid());
  EXPECT_EQ(Value::ARRAY, val2.type());
  EXPECT_EQ(2U, val2.element_count());
  {
    SCOPED_TRACE("");
    validate_array_contents(val2.element(0), array.size());
  }
  {
    SCOPED_TRACE("");
    validate_array_contents(val2.element(1), array.size());
  }

  Json_object object;
  object.add_clone("a", &array);
  Json_string s_c("c");
  object.add_clone("b", &s_c);
  EXPECT_FALSE(serialize(thd(), &object, &buf));
  Value val3 = parse_binary(buf.ptr(), buf.length());
  EXPECT_TRUE(val3.is_valid());
  EXPECT_TRUE(val3.large_format());
  EXPECT_EQ(Value::OBJECT, val3.type());
  EXPECT_EQ(2U, val3.element_count());
  EXPECT_EQ("a", get_string(val3.key(0)));
  {
    SCOPED_TRACE("");
    validate_array_contents(val3.element(0), array.size());
  }
  EXPECT_EQ("b", get_string(val3.key(1)));
  EXPECT_EQ(Value::STRING, val3.element(1).type());
  EXPECT_EQ("c", get_string(val3.element(1)));

  {
    SCOPED_TRACE("");
    validate_array_contents(val3.lookup("a"), array.size());
  }
  EXPECT_EQ("c", get_string(val3.lookup("b")));

  /*
    Extract the raw binary representation of the large object, and verify
    that it is valid.
  */
  EXPECT_FALSE(val3.raw_binary(thd(), &raw));
  {
    SCOPED_TRACE("");
    Value val_a = parse_binary(raw.ptr(), raw.length()).lookup("a");
    validate_array_contents(val_a, array.size());
  }

  /*
    Bug#23031146: INSERTING 64K SIZE RECORDS TAKE TOO MUCH TIME

    If a big (>64KB) sub-document was located at a deep nesting level,
    serialization used to be very slow.
  */
  {
    SCOPED_TRACE("");
    // Wrap "array" in 50 more levels of arrays.
    constexpr size_t depth = 50;
    Json_array deeply_nested_array;
    Json_array *current_array = &deeply_nested_array;
    for (size_t i = 1; i < depth; i++) {
      Json_array *a = new (std::nothrow) Json_array();
      ASSERT_FALSE(current_array->append_alias(a));
      current_array = a;
    }
    current_array->append_clone(&array);
    // Serialize it. This used to take "forever".
    ASSERT_FALSE(serialize(thd(), &deeply_nested_array, &buf));
    // Parse the serialized DOM and verify its contents.
    Value val = parse_binary(buf.ptr(), buf.length());
    for (size_t i = 0; i < depth; i++) {
      ASSERT_EQ(Value::ARRAY, val.type());
      ASSERT_EQ(1U, val.element_count());
      val = val.element(0);
    }
    validate_array_contents(val, array.size());

    // Now test the same with object.
    Json_object deeply_nested_object;
    Json_object *current_object = &deeply_nested_object;
    for (size_t i = 1; i < depth; i++) {
      Json_object *o = new (std::nothrow) Json_object();
      ASSERT_FALSE(current_object->add_alias("key", o));
      current_object = o;
    }
    current_object->add_clone("key", &array);
    ASSERT_FALSE(serialize(thd(), &deeply_nested_object, &buf));
    val = parse_binary(buf.ptr(), buf.length());
    for (size_t i = 0; i < depth; i++) {
      ASSERT_EQ(Value::OBJECT, val.type());
      ASSERT_EQ(1U, val.element_count());
      ASSERT_EQ("key", get_string(val.key(0)));
      val = val.element(0);
    }
    validate_array_contents(val, array.size());
  }
}

/*
  Various tests for the Value::raw_binary() function.
*/
TEST_F(JsonBinaryTest, RawBinaryTest) {
  Json_array array;
  Json_string as("a string");
  array.append_clone(&as);
  Json_int ji(-123);
  array.append_clone(&ji);
  Json_uint jui(42);
  array.append_clone(&jui);
  Json_double jd(1.5);
  array.append_clone(&jd);
  Json_null jn;
  array.append_clone(&jn);
  Json_boolean jbt(true);
  array.append_clone(&jbt);
  Json_boolean jbf(false);
  array.append_clone(&jbf);
  Json_opaque jo(MYSQL_TYPE_BLOB, "abcd", 4);
  array.append_clone(&jo);

  Json_object object;
  object.add_clone("key", &jbt);
  array.append_clone(&object);

  Json_array array2;
  array2.append_clone(&jbf);
  array.append_clone(&array2);

  String buf;
  EXPECT_FALSE(json_binary::serialize(thd(), &array, &buf));
  Value v1 = parse_binary(buf.ptr(), buf.length());

  String raw;
  EXPECT_FALSE(v1.raw_binary(thd(), &raw));
  Value v1_copy = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::ARRAY, v1_copy.type());
  EXPECT_EQ(array.size(), v1_copy.element_count());

  EXPECT_FALSE(v1.element(0).raw_binary(thd(), &raw));
  Value v1_0 = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::STRING, v1_0.type());
  EXPECT_EQ("a string", std::string(v1_0.get_data(), v1_0.get_data_length()));

  EXPECT_FALSE(v1.element(1).raw_binary(thd(), &raw));
  Value v1_1 = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::INT, v1_1.type());
  EXPECT_EQ(-123, v1_1.get_int64());

  EXPECT_FALSE(v1.element(2).raw_binary(thd(), &raw));
  Value v1_2 = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::UINT, v1_2.type());
  EXPECT_EQ(42U, v1_2.get_uint64());

  EXPECT_FALSE(v1.element(3).raw_binary(thd(), &raw));
  Value v1_3 = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::DOUBLE, v1_3.type());
  EXPECT_EQ(1.5, v1_3.get_double());

  EXPECT_FALSE(v1.element(4).raw_binary(thd(), &raw));
  Value v1_4 = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::LITERAL_NULL, v1_4.type());

  EXPECT_FALSE(v1.element(5).raw_binary(thd(), &raw));
  Value v1_5 = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::LITERAL_TRUE, v1_5.type());

  EXPECT_FALSE(v1.element(6).raw_binary(thd(), &raw));
  Value v1_6 = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::LITERAL_FALSE, v1_6.type());

  EXPECT_FALSE(v1.element(7).raw_binary(thd(), &raw));
  Value v1_7 = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::OPAQUE, v1_7.type());
  EXPECT_EQ(MYSQL_TYPE_BLOB, v1_7.field_type());
  EXPECT_EQ("abcd", std::string(v1_7.get_data(), v1_7.get_data_length()));

  EXPECT_FALSE(v1.element(8).raw_binary(thd(), &raw));
  Value v1_8 = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::OBJECT, v1_8.type());
  EXPECT_EQ(object.cardinality(), v1_8.element_count());
  EXPECT_EQ(Value::LITERAL_TRUE, v1_8.lookup("key").type());

  EXPECT_FALSE(v1.element(8).key(0).raw_binary(thd(), &raw));
  Value v1_8_key = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::STRING, v1_8_key.type());
  EXPECT_EQ("key",
            std::string(v1_8_key.get_data(), v1_8_key.get_data_length()));

  EXPECT_FALSE(v1.element(8).element(0).raw_binary(thd(), &raw));
  Value v1_8_val = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::LITERAL_TRUE, v1_8_val.type());

  EXPECT_FALSE(v1.element(9).raw_binary(thd(), &raw));
  Value v1_9 = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::ARRAY, v1_9.type());
  EXPECT_EQ(array2.size(), v1_9.element_count());
  EXPECT_EQ(Value::LITERAL_FALSE, v1_9.element(0).type());

  EXPECT_FALSE(v1.element(9).element(0).raw_binary(thd(), &raw));
  Value v1_9_0 = parse_binary(raw.ptr(), raw.length());
  EXPECT_EQ(Value::LITERAL_FALSE, v1_9_0.type());
}

/*
  Create a JSON string of the given size, serialize it as a JSON binary, and
  then deserialize it and verify that we get the same string back.
*/
void serialize_deserialize_string(const THD *thd, size_t size) {
  SCOPED_TRACE(testing::Message() << "size = " << size);
  char *str = new char[size];
  memset(str, 'a', size);
  Json_string jstr(str, size);

  String buf;
  EXPECT_FALSE(json_binary::serialize(thd, &jstr, &buf));
  Value v = parse_binary(buf.ptr(), buf.length());
  EXPECT_EQ(Value::STRING, v.type());
  EXPECT_EQ(size, v.get_data_length());
  EXPECT_EQ(0, memcmp(str, v.get_data(), size));

  delete[] str;
}

/*
  Test strings of variable length. Test especially around the boundaries
  where the representation of the string length changes:

  - Strings of length 0-127 use 1 byte length fields.
  - Strings of length 128-16383 use 2 byte length fields.
  - Strings of length 16384-2097151 use 3 byte length fields.
  - Strings of length 2097152-268435455 use 4 byte length fields.
  - Strings of length 268435456-... use 5 byte length fields.

  We probably don't have enough memory to test the last category here...
*/
TEST_F(JsonBinaryTest, StringLengthTest) {
  const THD *thd = this->thd();
  serialize_deserialize_string(thd, 0);
  serialize_deserialize_string(thd, 1);
  serialize_deserialize_string(thd, 127);
  serialize_deserialize_string(thd, 128);
  serialize_deserialize_string(thd, 16383);
  serialize_deserialize_string(thd, 16384);
  serialize_deserialize_string(thd, 2097151);
  serialize_deserialize_string(thd, 2097152);
  serialize_deserialize_string(thd, 3000000);
}

/**
  Error handler which registers if an error has been raised. If an error is
  raised, it asserts that the error is ER_INVALID_JSON_BINARY_DATA.
*/
class Invalid_binary_handler : public Internal_error_handler {
 public:
  Invalid_binary_handler(THD *thd)
      : m_thd(thd), m_called(false), m_orig_handler(error_handler_hook) {
    error_handler_hook = my_message_sql;
    thd->push_internal_handler(this);
  }

  ~Invalid_binary_handler() override {
    EXPECT_EQ(this, m_thd->pop_internal_handler());
    error_handler_hook = m_orig_handler;
  }

  bool handle_condition(THD *, uint err, const char *,
                        Sql_condition::enum_severity_level *,
                        const char *) override {
    uint expected = ER_INVALID_JSON_BINARY_DATA;
    EXPECT_EQ(expected, err);
    m_called = true;
    return true;
  }

  bool is_called() const { return m_called; }

 private:
  THD *m_thd;
  bool m_called;
  decltype(error_handler_hook) m_orig_handler;
};

/**
  Run various operations on a corrupted binary value, to see that the
  json_binary library doesn't fall over when encountering a corrupted value.
*/
static void check_corrupted_binary(THD *thd, const char *data, size_t length) {
  /*
    A corrupted value may still be valid, so we cannot assert on the return
    value. Just exercise the code to see that nothing very bad happens.
  */
  Value val = parse_binary(data, length);
  val.is_valid();

  {
    /*
      Value::get_free_space() may or may not raise an error. If there is an
      error, we expect it to be ER_INVALID_JSON_BINARY_DATA.
    */
    Invalid_binary_handler handler(thd);
    size_t space;
    bool err = val.get_free_space(thd, &space);
    // If it returns true, an error should have been raised.
    EXPECT_EQ(err, handler.is_called());
  }

  /*
    Call Value::has_space() on every element if it is an array or object.
  */
  if (val.type() == Value::ARRAY || val.type() == Value::OBJECT) {
    for (size_t i = 0; i < val.element_count(); ++i) {
      size_t offset;
      val.has_space(i, 100, &offset);
    }
  }
}

/**
  Check that a corrupted binary value doesn't upset the parser in any serious
  way.

  @param thd  THD handle
  @param dom  a JSON DOM from which corrupted binary values are created
*/
static void check_corruption(THD *thd, const Json_dom *dom) {
  // First create a valid binary representation of the DOM.
  String buf;
  EXPECT_FALSE(json_binary::serialize(thd, dom, &buf));
  EXPECT_TRUE(json_binary::parse_binary(buf.ptr(), buf.length()).is_valid());

  /*
    Truncated values should always be detected by is_valid(). Except
    if it's truncated to an empty string, since parse_binary()
    interprets the empty string as the JSON null literal.
  */
  for (size_t i = 1; i < buf.length() - 1; ++i) {
    EXPECT_FALSE(json_binary::parse_binary(buf.ptr(), i).is_valid());
    check_corrupted_binary(thd, buf.ptr(), i);
  }

  /*
    Test various 1, 2 and 3 byte data corruptions. is_valid() may return true
    or false (not all corrupted documents are ill-formed), but we should not
    have any crashes or valgrind/asan warnings.
  */
  for (size_t i = 0; i < buf.length(); ++i) {
    String copy;
    copy.append(buf);
    char *data = copy.c_ptr_safe();
    for (size_t j = 1; j < 3 && i + j < buf.length(); ++j) {
      memset(data + i, 0x00, j);
      check_corrupted_binary(thd, data, copy.length());
      memset(data + i, 0x80, j);
      check_corrupted_binary(thd, data, copy.length());
      memset(data + i, 0xff, j);
      check_corrupted_binary(thd, data, copy.length());
    }
  }
}

/**
  Test that the parser is well-behaved when a binary value is corrupted.
*/
TEST_F(JsonBinaryTest, CorruptedBinaryTest) {
  Json_array a;
  a.append_alias(new (std::nothrow) Json_null);
  a.append_alias(new (std::nothrow) Json_boolean(true));
  a.append_alias(new (std::nothrow) Json_boolean(false));
  a.append_alias(new (std::nothrow) Json_uint(0));
  a.append_alias(new (std::nothrow) Json_uint(123));
  a.append_alias(new (std::nothrow) Json_uint(123000));
  a.append_alias(new (std::nothrow) Json_uint(123000000));
  a.append_alias(new (std::nothrow) Json_int(0));
  a.append_alias(new (std::nothrow) Json_int(123));
  a.append_alias(new (std::nothrow) Json_int(123000));
  a.append_alias(new (std::nothrow) Json_int(123000000));
  a.append_alias(new (std::nothrow) Json_int(-123000000));
  a.append_alias(new (std::nothrow) Json_string());
  a.append_alias(new (std::nothrow) Json_string(300, 'a'));
  a.append_alias(new (std::nothrow) Json_decimal(create_decimal(3.14)));
  Json_object *o = new (std::nothrow) Json_object;
  a.append_alias(o);
  o->add_clone("a1", &a);
  o->add_alias("s", new (std::nothrow) Json_opaque(MYSQL_TYPE_BLOB, 32, 'x'));
  o->add_alias("d", new (std::nothrow) Json_double(3.14));
  a.append_clone(&a);
  o->add_clone("a2", &a);

  check_corruption(thd(), &a);
  for (size_t i = 0; i < a.size(); ++i) {
    SCOPED_TRACE("");
    check_corruption(thd(), a[i]);
  }
}

/// How big is the serialized version of a Json_dom?
static size_t binary_size(const THD *thd, const Json_dom *dom) {
  StringBuffer<256> buf;
  EXPECT_FALSE(json_binary::serialize(thd, dom, &buf));
  return buf.length();
}

/// A tuple used by SpaceNeededTest for testing json_binary::space_needed().
struct SpaceNeededTuple {
  /**
    Constructor for test case with different space requirements in the
    large and small storage formats.
  */
  SpaceNeededTuple(Json_dom *dom, bool result, size_t needed_small,
                   size_t needed_large)
      : m_value(dom),
        m_result(result),
        m_needed_small(needed_small),
        m_needed_large(needed_large) {}
  /**
    Constructor for test case with same space requirement in the large
    and small storage formats.
  */
  SpaceNeededTuple(Json_dom *dom, bool result, size_t needed)
      : SpaceNeededTuple(dom, result, needed, needed) {}
  /// The value to pass to space_needed().
  Json_wrapper m_value;
  /// The expected return value from the function.
  bool m_result;
  /// The expected bytes needed to store the value in the small storage format.
  size_t m_needed_small;
  /// The expected bytes needed to store the value in the large storage format.
  size_t m_needed_large;
};

/// A class used for testing json_binary::space_needed().
class SpaceNeededTest : public ::testing::TestWithParam<SpaceNeededTuple> {
  my_testing::Server_initializer initializer;

 protected:
  void SetUp() override { initializer.SetUp(); }
  void TearDown() override { initializer.TearDown(); }
  THD *thd() const { return initializer.thd(); }
};

/*
  Define our own PrintTo because Google Test's default implementation causes
  valgrind warnings for reading uninitialized memory. (It reads every byte of
  the struct, but the struct contains some uninitialized bytes because of
  alignment.)
*/
void PrintTo(SpaceNeededTuple const &tuple, std::ostream *os) {
  *os << '{' << static_cast<uint>(tuple.m_value.type()) << ", "
      << tuple.m_result << ", " << tuple.m_needed_small << ", "
      << tuple.m_needed_large << '}';
}

/// Test json_binary::space_needed() for a given input.
TEST_P(SpaceNeededTest, SpaceNeeded) {
  SpaceNeededTuple param = GetParam();

  /*
    If the large and small storage size differ, it must mean that the
    value can be inlined in the large storage format.
  */
  if (param.m_needed_small != param.m_needed_large) {
    EXPECT_EQ(0U, param.m_needed_large);
  }

  size_t needed = 0;
  if (param.m_result) {
    for (bool large : {true, false}) {
      Invalid_binary_handler handler(thd());
      EXPECT_TRUE(space_needed(thd(), &param.m_value, large, &needed));
      EXPECT_TRUE(handler.is_called());
    }
    return;
  }

  needed = 0;
  EXPECT_FALSE(space_needed(thd(), &param.m_value, false, &needed));
  EXPECT_EQ(param.m_needed_small, needed);

  needed = 0;
  EXPECT_FALSE(space_needed(thd(), &param.m_value, true, &needed));
  EXPECT_EQ(param.m_needed_large, needed);

  const auto dom = param.m_value.to_dom(thd());

  if (param.m_needed_small > 0) {
    /*
      Not inlined. The size does not include the type byte, so expect
      one more byte.
    */
    EXPECT_EQ(param.m_needed_small + 1, binary_size(thd(), dom));
  } else {
    /*
      Inlined in the small storage format. Find the difference in size
      between an empty array and one with the value added. Expect the
      size of a small value entry, which is 3 bytes (1 byte for the
      type, 2 bytes for the inlined value).
    */
    Json_array a;
    size_t base_size = binary_size(thd(), &a);
    a.append_clone(dom);
    size_t full_size = binary_size(thd(), &a);
    EXPECT_EQ(base_size + 3, full_size);
  }

  if (param.m_needed_small > 0 && param.m_needed_large == 0) {
    /*
      Inlined in the large storage format only. See how much space is
      added. Expect the size of a large value entry, which is 5 bytes
      (1 byte for the type, 4 bytes for the inlined value).
    */
    Json_array a;
    a.append_alias(new (std::nothrow) Json_string(64 * 1024, 'a'));
    size_t base_size = binary_size(thd(), &a);
    a.append_clone(dom);
    size_t full_size = binary_size(thd(), &a);
    EXPECT_EQ(base_size + 5, full_size);
  }
}

static const SpaceNeededTuple space_needed_tuples[] = {
    /*
      Strings need space for the actual data and a variable length field
      that holds the length of the string. Each byte in the variable
      length field holds seven bits of the length value, so testing
      lengths around 2^(7*N) is important.
    */
    {new (std::nothrow) Json_string(""), false, 1},                  // 2^0-1
    {new (std::nothrow) Json_string("a"), false, 2},                 // 2^0
    {new (std::nothrow) Json_string(127, 'a'), false, 128},          // 2^7-1
    {new (std::nothrow) Json_string(128, 'a'), false, 130},          // 2^7
    {new (std::nothrow) Json_string(16383, 'a'), false, 16385},      // 2^14-1
    {new (std::nothrow) Json_string(16384, 'a'), false, 16387},      // 2^14
    {new (std::nothrow) Json_string(2097151, 'a'), false, 2097154},  // 2^21-1
    {new (std::nothrow) Json_string(2097152, 'a'), false, 2097156},  // 2^21

    // Literals are always inlined.
    {new (std::nothrow) Json_null, false, 0},
    {new (std::nothrow) Json_boolean(false), false, 0},
    {new (std::nothrow) Json_boolean(true), false, 0},

    // 16-bit integers are always inlined.
    {new (std::nothrow) Json_int(0), false, 0},
    {new (std::nothrow) Json_int(-1), false, 0},
    {new (std::nothrow) Json_int(1), false, 0},
    {new (std::nothrow) Json_int(INT_MIN16), false, 0},
    {new (std::nothrow) Json_int(INT_MAX16), false, 0},
    {new (std::nothrow) Json_uint(0), false, 0},
    {new (std::nothrow) Json_uint(1), false, 0},
    {new (std::nothrow) Json_uint(UINT_MAX16), false, 0},

    // 32-bit integers are inlined only in the large storage format.
    {new (std::nothrow) Json_int(INT_MIN32), false, 4, 0},
    {new (std::nothrow) Json_int(INT_MIN16 - 1), false, 4, 0},
    {new (std::nothrow) Json_int(INT_MAX16 + 1), false, 4, 0},
    {new (std::nothrow) Json_int(INT_MAX32), false, 4, 0},
    {new (std::nothrow) Json_uint(UINT_MAX16 + 1), false, 4, 0},
    {new (std::nothrow) Json_uint(UINT_MAX32), false, 4, 0},

    // Larger integers and doubles require 8 bytes.
    {new (std::nothrow) Json_int(INT_MIN64), false, 8},
    {new (std::nothrow) Json_int(static_cast<longlong>(INT_MIN32) - 1), false,
     8},
    {new (std::nothrow) Json_int(static_cast<longlong>(INT_MAX32) + 1), false,
     8},
    {new (std::nothrow) Json_int(INT_MAX64), false, 8},
    {new (std::nothrow) Json_uint(static_cast<ulonglong>(UINT_MAX32) + 1),
     false, 8},
    {new (std::nothrow) Json_uint(0xFFFFFFFFFFFFFFFFULL), false, 8},
    {new (std::nothrow) Json_double(0), false, 8},
    {new (std::nothrow) Json_double(3.14), false, 8},

    /*
      Opaque values need space for type info (one byte), a variable
      length field, and the actual data.
    */
    {new (std::nothrow) Json_opaque(MYSQL_TYPE_BLOB, ""), false, 2},
    {new (std::nothrow) Json_opaque(MYSQL_TYPE_BLOB, "a"), false, 3},
    {new (std::nothrow) Json_opaque(MYSQL_TYPE_BLOB, 127, 'a'), false, 129},
    {new (std::nothrow) Json_opaque(MYSQL_TYPE_BLOB, 128, 'a'), false, 131},
    {new (std::nothrow) Json_datetime(create_time(), MYSQL_TYPE_TIME), false,
     Json_datetime::PACKED_SIZE + 2},
    {new (std::nothrow) Json_datetime(create_date(), MYSQL_TYPE_DATE), false,
     Json_datetime::PACKED_SIZE + 2},
    {new (std::nothrow) Json_datetime(create_datetime(), MYSQL_TYPE_DATETIME),
     false, Json_datetime::PACKED_SIZE + 2},
    {new (std::nothrow) Json_datetime(create_datetime(), MYSQL_TYPE_TIMESTAMP),
     false, Json_datetime::PACKED_SIZE + 2},
    {new (std::nothrow) Json_decimal(create_decimal(12.3)), false, 6},

    // Arrays.
    {new (std::nothrow) Json_array, false, 4},
    {parse_json("[1.5]").release(), false, 15},

    // Objects
    {new (std::nothrow) Json_object, false, 4},
    {parse_json("{\"a\":1.5}").release(), false, 20},

    // Handle type == ERROR.
    {nullptr, true, 0},
};

INSTANTIATE_TEST_CASE_P(JsonBinary, SpaceNeededTest,
                        ::testing::ValuesIn(space_needed_tuples));

/**
  Helper function for testing Value::has_space(). Serializes a JSON
  array or JSON object and checks if has_space() reports the correct
  size and offset for an element in the array or object.
*/
static void test_has_space(THD *thd, const Json_dom *container,
                           Value::enum_type type, size_t size, size_t element,
                           size_t expected_offset) {
  StringBuffer<100> buf;
  EXPECT_FALSE(json_binary::serialize(thd, container, &buf));
  Value v1 = parse_binary(buf.ptr(), buf.length());
  Value v2 = v1.element(element);
  EXPECT_EQ(type, v2.type());
  size_t offset = 0;
  if (size > 0) {
    EXPECT_TRUE(v1.has_space(element, size - 1, &offset));
    EXPECT_EQ(expected_offset, offset);
  }
  offset = 0;
  EXPECT_TRUE(v1.has_space(element, size, &offset));
  EXPECT_EQ(expected_offset, offset);
  offset = 0;
  EXPECT_FALSE(v1.has_space(element, size + 1, &offset));
  EXPECT_EQ(0U, offset);
  offset = 0;
  EXPECT_TRUE(v1.has_space(element, 0, &offset));
  EXPECT_EQ(expected_offset, offset);
}

/**
  Test Value::has_size() by inserting a value into an array or an
  object and checking that has_size() returns the correct size and
  offset.
*/
static void test_has_space(THD *thd, const Json_dom *dom, Value::enum_type type,
                           size_t size) {
  {
    SCOPED_TRACE("array");
    Json_array a;
    a.append_clone(dom);

    /*
      The array contains the element count (2 bytes), byte size (2
      bytes) and a value entry (3 bytes) before the value.
    */
    size_t expected_offset = 2 + 2 + 3;
    {
      SCOPED_TRACE("first element");
      test_has_space(thd, &a, type, size, 0, expected_offset);
    }

    /*
      Insert a literal at the beginning of the array. The offset
      should increase by 3 (size of the value entry).
    */
    expected_offset += 3;
    {
      SCOPED_TRACE("second element");
      a.insert_alias(0, create_dom_ptr<Json_null>());
      test_has_space(thd, &a, type, size, 1, expected_offset);
    }

    /*
      Insert a double at the beginning of the array. Expect the offset
      to increase by 3 (size of the value entry) + 8 (size of the
      double).
    */
    expected_offset += 3 + 8;
    {
      SCOPED_TRACE("third element");
      a.insert_alias(0, create_dom_ptr<Json_double>(123.0));
      test_has_space(thd, &a, type, size, 2, expected_offset);
    }

    /*
      Insert some values at the end of the array. The offset should
      increase by 3 (one value entry) per added value.
    */
    expected_offset += 3;
    {
      SCOPED_TRACE("append literal");
      a.append_alias(new (std::nothrow) Json_boolean(true));
      test_has_space(thd, &a, type, size, 2, expected_offset);
    }
    expected_offset += 3;
    {
      SCOPED_TRACE("append double");
      a.append_alias(new (std::nothrow) Json_double(1.23));
      test_has_space(thd, &a, type, size, 2, expected_offset);
    }
  }

  /*
    Now test the same with an object.
  */
  {
    SCOPED_TRACE("object");
    Json_object o;
    o.add_clone("k", dom);

    /*
      The object contains the element count (2 bytes), byte size (2
      bytes), a key entry (4 bytes), a value entry (3 bytes) and a key
      (1 byte) before the value.
    */
    size_t expected_offset = 2 + 2 + 4 + 3 + 1;
    {
      SCOPED_TRACE("first element");
      test_has_space(thd, &o, type, size, 0, expected_offset);
    }

    /*
      Add a literal at the beginning of the object. The offset should
      increase by 4 (size of the key entry) + 3 (size of the value
      entry) + 1 (size of the key).
    */
    expected_offset += 4 + 3 + 1;
    {
      SCOPED_TRACE("second element");
      o.add_alias("b", new (std::nothrow) Json_null);
      test_has_space(thd, &o, type, size, 1, expected_offset);
    }

    /*
      Add a double at the beginning of the object. Expect the offset
      to increase by 4 (size of the key entry) + 3 (size of the value
      entry) + 1 (size of the key) + 8 (size of the double).
    */
    expected_offset += 4 + 3 + 1 + 8;
    {
      SCOPED_TRACE("third element");
      o.add_alias("a", new (std::nothrow) Json_double(123.0));
      test_has_space(thd, &o, type, size, 2, expected_offset);
    }

    /*
      Add some values at the end of the array. The offset should
      increase by 4 (one key entry) + 3 (one value entry) + 1 (one
      key) per added member.
    */
    expected_offset += 4 + 3 + 1;
    {
      SCOPED_TRACE("add literal");
      o.add_alias("x", new (std::nothrow) Json_boolean(true));
      test_has_space(thd, &o, type, size, 2, expected_offset);
    }
    expected_offset += 4 + 3 + 1;
    {
      SCOPED_TRACE("add double");
      o.add_alias("y", new (std::nothrow) Json_double(1.23));
      test_has_space(thd, &o, type, size, 2, expected_offset);
    }
  }
}

/**
  Various tests for Value::has_space().
*/
TEST_F(JsonBinaryTest, HasSpace) {
  {
    SCOPED_TRACE("empty string");
    Json_string jstr;
    test_has_space(thd(), &jstr, Value::STRING, 1);
  }
  {
    // Test longest possible string with 1-byte length field.
    SCOPED_TRACE("string(127)");
    Json_string jstr(127, 'a');
    test_has_space(thd(), &jstr, Value::STRING, 128);
  }
  {
    // Test shortest possible string with 2-byte length field.
    SCOPED_TRACE("string(128)");
    Json_string jstr(128, 'a');
    test_has_space(thd(), &jstr, Value::STRING, 130);
  }
  {
    SCOPED_TRACE("null literal");
    Json_null jnull;
    test_has_space(thd(), &jnull, Value::LITERAL_NULL, 0);
  }
  {
    SCOPED_TRACE("true literal");
    Json_boolean jtrue(true);
    test_has_space(thd(), &jtrue, Value::LITERAL_TRUE, 0);
  }
  {
    SCOPED_TRACE("false literal");
    Json_boolean jfalse(false);
    test_has_space(thd(), &jfalse, Value::LITERAL_FALSE, 0);
  }
  {
    SCOPED_TRACE("inlined uint");
    Json_uint u(123);
    EXPECT_TRUE(u.is_16bit());
    test_has_space(thd(), &u, Value::UINT, 0);
  }
  {
    SCOPED_TRACE("32-bit uint");
    Json_uint u(100000);
    EXPECT_FALSE(u.is_16bit());
    EXPECT_TRUE(u.is_32bit());
    test_has_space(thd(), &u, Value::UINT, 4);
  }
  {
    SCOPED_TRACE("64-bit uint");
    Json_uint u(5000000000ULL);
    EXPECT_FALSE(u.is_32bit());
    test_has_space(thd(), &u, Value::UINT, 8);
  }
  {
    SCOPED_TRACE("inlined int");
    Json_int i(123);
    EXPECT_TRUE(i.is_16bit());
    test_has_space(thd(), &i, Value::INT, 0);
  }
  {
    SCOPED_TRACE("32-bit int");
    Json_int i(100000);
    EXPECT_FALSE(i.is_16bit());
    EXPECT_TRUE(i.is_32bit());
    test_has_space(thd(), &i, Value::INT, 4);
  }
  {
    SCOPED_TRACE("64-bit uint");
    Json_int i(5000000000LL);
    EXPECT_FALSE(i.is_32bit());
    test_has_space(thd(), &i, Value::INT, 8);
  }
  {
    SCOPED_TRACE("double");
    Json_double d(3.14);
    test_has_space(thd(), &d, Value::DOUBLE, 8);
  }
  {
    SCOPED_TRACE("opaque");
    Json_opaque o(MYSQL_TYPE_BLOB, "abc", 3);
    // 1 byte for type, 1 byte for length, 3 bytes of blob data
    test_has_space(thd(), &o, Value::OPAQUE, 5);
  }
  {
    SCOPED_TRACE("empty array");
    Json_array a;
    /*
      An empty array has two bytes for element count and two bytes for
      total size in bytes.
    */
    test_has_space(thd(), &a, Value::ARRAY, 4);
  }
  {
    SCOPED_TRACE("non-empty array");
    auto a = parse_json("[null]");
    // Here we have an additional 3 bytes for the value entry.
    test_has_space(thd(), a.get(), Value::ARRAY, 4 + 3);
  }
  {
    SCOPED_TRACE("empty object");
    Json_object o;
    /*
      An empty object has two bytes for element count and two bytes for
      total size in bytes.
    */
    test_has_space(thd(), &o, Value::OBJECT, 4);
  }
  {
    SCOPED_TRACE("non-empty object");
    Json_object o;
    o.add_alias("a", new (std::nothrow) Json_null);
    /*
      Here we have an additional 4 bytes for the key entry, 3 bytes
      for the value entry, and 1 byte for the key.
    */
    test_has_space(thd(), &o, Value::OBJECT, 4 + 4 + 3 + 1);
  }
}

/**
  Helper function for microbenchmarks that test the performance of
  json_binary::serialize().

  @param dom             the Json_dom to serialize
  @param num_iterations  the number of iterations in the test
*/
static void serialize_benchmark(const Json_dom *dom, size_t num_iterations) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();
  const THD *thd = initializer.thd();

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    String buf;
    EXPECT_FALSE(json_binary::serialize(thd, dom, &buf));
  }

  StopBenchmarkTiming();

  initializer.TearDown();
}

/**
  Microbenchmark which tests the performance of serializing a JSON
  array with 10000 integers.
*/
static void BM_JsonBinarySerializeIntArray(size_t num_iterations) {
  StopBenchmarkTiming();

  Json_array array;
  for (int i = 0; i < 10000; ++i)
    array.append_alias(create_dom_ptr<Json_int>(i * 1000));

  serialize_benchmark(&array, num_iterations);
}
BENCHMARK(BM_JsonBinarySerializeIntArray)

/**
  Microbenchmark which tests the performance of serializing a JSON
  array with 10000 double values.
*/
static void BM_JsonBinarySerializeDoubleArray(size_t num_iterations) {
  StopBenchmarkTiming();

  Json_array array;
  for (int i = 0; i < 10000; ++i)
    array.append_alias(create_dom_ptr<Json_double>(i * 1000));

  serialize_benchmark(&array, num_iterations);
}
BENCHMARK(BM_JsonBinarySerializeDoubleArray)

/**
  Microbenchmark which tests the performance of serializing a JSON
  array with 10000 strings.
*/
static void BM_JsonBinarySerializeStringArray(size_t num_iterations) {
  StopBenchmarkTiming();

  Json_array array;
  for (int i = 0; i < 10000; ++i)
    array.append_alias(create_dom_ptr<Json_string>(std::to_string(i)));

  serialize_benchmark(&array, num_iterations);
}
BENCHMARK(BM_JsonBinarySerializeStringArray)

}  // namespace json_binary_unittest
