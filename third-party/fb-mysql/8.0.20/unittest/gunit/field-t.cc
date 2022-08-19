/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <sys/types.h>

#include "my_inttypes.h"
#include "sql/field.h"
#include "sql/my_decimal.h"
#include "sql/protocol.h"
#include "sql/sql_class.h"
#include "sql/sql_time.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/mysys_util.h"
#include "unittest/gunit/test_utils.h"

namespace field_unittests {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

class FieldTest : public ::testing::Test {
 protected:
  void SetUp() override { initializer.SetUp(); }
  void TearDown() override { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;

  Field_set *create_field_set(TYPELIB *tl);
};

static void compareMysqlTime(const MYSQL_TIME &first,
                             const MYSQL_TIME &second) {
  EXPECT_EQ(first.year, second.year);
  EXPECT_EQ(first.month, second.month);
  EXPECT_EQ(first.day, second.day);
  EXPECT_EQ(first.hour, second.hour);
  EXPECT_EQ(first.minute, second.minute);
  EXPECT_EQ(first.second, second.second);
  EXPECT_EQ(first.second_part, second.second_part);
  EXPECT_EQ(first.neg, second.neg);
  EXPECT_EQ(first.time_type, second.time_type);
}

class Mock_table : public TABLE {
 public:
  Mock_table(THD *thd) {
    null_row = false;
    read_set = nullptr;
    in_use = thd;
  }
};

// A mock Protocol class to be able to test Field::send_binary
// It just verifies that store_time has been passed what is expected
class Mock_protocol : public Protocol {
 private:
  MYSQL_TIME t;
  uint p;

 public:
  Mock_protocol(THD *) {}

  bool store_time(const MYSQL_TIME &time, uint precision) override {
    t = time;
    p = precision;
    return false;
  }

  void verify_time(const MYSQL_TIME &time, uint precision) {
    compareMysqlTime(time, t);
    EXPECT_EQ(precision, p);
  }

  // Lots of functions that require implementation
  int read_packet() override { return 0; }
  ulong get_client_capabilities() override { return 0; }
  bool has_client_capability(unsigned long) override { return false; }
  void end_partial_result_set() override {}
  int shutdown(bool server_shutdown MY_ATTRIBUTE((unused)) = false) override {
    return 0;
  }
  SSL_handle get_ssl() { return nullptr; }
  void start_row() override {}
  bool end_row() override { return false; }
  bool connection_alive() const override { return false; }
  void abort_row() override {}
  uint get_rw_status() override { return 0; }
  bool get_compression() override { return false; }
  bool start_result_metadata(uint, uint, const CHARSET_INFO *) override {
    return false;
  }

  char *get_compression_algorithm() override { return nullptr; }
  uint get_compression_level() override { return 0; }

  bool store_ps_status(ulong, uint, uint, ulong) override { return false; }
  bool send_parameters(List<Item_param> *, bool) override { return false; }

  void send_num_fields(uint) {}
  void send_num_rows(uint) {}
  bool send_field_metadata(Send_field *, const CHARSET_INFO *) override {
    return false;
  }
  bool send_ok(uint, uint, ulonglong, ulonglong, const char *) override {
    return false;
  }

  bool send_eof(uint, uint) override { return false; }
  bool send_error(uint, const char *, const char *) override { return false; }
  bool end_result_metadata() override { return false; }

  bool store_null() override { return false; }
  bool store_tiny(longlong, uint32) override { return false; }
  bool store_short(longlong, uint32) override { return false; }
  bool store_long(longlong, uint32) override { return false; }
  bool store_longlong(longlong, bool, uint32) override { return false; }
  bool store_decimal(const my_decimal *, uint, uint) override { return false; }
  bool store_string(const char *, size_t, const CHARSET_INFO *) override {
    return false;
  }
  bool store_float(float, uint32, uint32) override { return false; }
  bool store_double(double, uint32, uint32) override { return false; }
  bool store_datetime(const MYSQL_TIME &, uint) override { return false; }
  bool store_date(const MYSQL_TIME &) override { return false; }
  bool store_field(const Field *) override { return false; }
  enum enum_protocol_type type() const override { return PROTOCOL_LOCAL; }
  enum enum_vio_type connection_type() const override { return NO_VIO_TYPE; }
  int get_command(COM_DATA *, enum_server_command *) override { return -1; }
  bool flush() override { return true; }
};

TEST_F(FieldTest, FieldTimef) {
  uchar fieldBuf[7];
  MysqlTime time(0, 0, 0, 12, 23, 12, 123400, false, MYSQL_TIMESTAMP_TIME);

  Field_timef *field = new (thd()->mem_root)
      Field_timef(fieldBuf + 1, fieldBuf, false, Field::NONE, "f1", 4);
  // Test public member functions
  EXPECT_EQ(4UL, field->decimals());  // TS-TODO
  EXPECT_EQ(MYSQL_TYPE_TIME, field->type());
  EXPECT_EQ(MYSQL_TYPE_TIME2, field->binlog_type());

  longlong packed = TIME_to_longlong_packed(time);

  EXPECT_EQ(0, field->store_packed(packed));
  EXPECT_DOUBLE_EQ(122312.1234, field->val_real());
  EXPECT_EQ(122312, field->val_int());
  EXPECT_EQ(packed, field->val_time_temporal());

  my_decimal decval;
  my_decimal *dec = field->val_decimal(&decval);
  double res;
  my_decimal2double(0, dec, &res);
  EXPECT_DOUBLE_EQ(122312.1234, res);

  EXPECT_EQ(5UL, field->pack_length());
  EXPECT_EQ(5UL, field->pack_length_from_metadata(4));
  EXPECT_EQ(5UL, field->row_pack_length());

  String str(7);
  field->sql_type(str);
  EXPECT_STREQ("time(4)", str.c_ptr_safe());

  EXPECT_EQ(1, field->zero_pack());
  EXPECT_EQ(&my_charset_bin, field->sort_charset());

  // Test clone
  Field *copy = field->clone(thd()->mem_root);
  EXPECT_EQ(field->decimals(), copy->decimals());
  EXPECT_EQ(field->type(), copy->type());
  EXPECT_DOUBLE_EQ(field->val_real(), copy->val_real());
  EXPECT_EQ(field->val_int(), copy->val_int());
  EXPECT_EQ(field->val_time_temporal(), copy->val_time_temporal());
  EXPECT_EQ(0, field->cmp(field->ptr, copy->ptr));

  // Test reset
  EXPECT_EQ(0, field->reset());
  EXPECT_DOUBLE_EQ(0.0, field->val_real());
  EXPECT_EQ(0, field->val_int());

  // Test inherited member functions
  // Functions inherited from Field_time_common
  field->store_time(&time, 4);
  EXPECT_EQ(4UL, field->decimals());
  EXPECT_EQ(MYSQL_TYPE_TIME, field->type());
  EXPECT_DOUBLE_EQ(122312.1234, field->val_real());
  EXPECT_EQ(122312, field->val_int());
  EXPECT_EQ(packed, field->val_time_temporal());

  String timeStr(15);
  EXPECT_STREQ("12:23:12.1234", field->val_str(&timeStr, &timeStr)->c_ptr());

  field->store_time(&time, 0);
  EXPECT_DOUBLE_EQ(122312.1234, field->val_real());  // Correct?

  MYSQL_TIME dateTime;
  MysqlTime bigTime(0, 0, 0, 123, 45, 45, 555500, false, MYSQL_TIMESTAMP_TIME);
  EXPECT_EQ(0, field->store_time(&bigTime, 4));
  EXPECT_FALSE(field->get_date(&dateTime, 0));

  make_datetime((Date_time_format *)nullptr, &dateTime, &timeStr, 6);
  // Skip 'yyyy-mm-dd ' since that will depend on current time zone.
  EXPECT_STREQ("03:45:45.555500", timeStr.c_ptr() + 11);

  MYSQL_TIME t;
  EXPECT_FALSE(field->get_time(&t));
  compareMysqlTime(bigTime, t);

  Mock_protocol protocol(thd());
  EXPECT_EQ(protocol.connection_type(), NO_VIO_TYPE);
  EXPECT_FALSE(field->send_to_protocol(&protocol));
  protocol.verify_time(bigTime, 4);

  // Function inherited from Field_temporal
  EXPECT_TRUE(is_temporal_type(field->type()));
  EXPECT_EQ(STRING_RESULT, field->result_type());
  EXPECT_EQ(15UL, field->max_display_length());
  EXPECT_TRUE(field->str_needs_quotes());

  // Not testing is_equal() yet, will require a mock TABLE object
  //  Create_field cf(field, field);
  //  EXPECT_TRUE(field->is_equal(&cf));

  EXPECT_EQ(DECIMAL_RESULT, field->numeric_context_result_type());
  EXPECT_EQ(INT_RESULT, field->cmp_type());
  EXPECT_EQ(INT_RESULT, field->cmp_type());
  EXPECT_EQ(DERIVATION_NUMERIC, field->derivation());
  EXPECT_EQ(&my_charset_numeric, field->charset());
  EXPECT_TRUE(field->can_be_compared_as_longlong());
  EXPECT_TRUE(field->binary());
  // Below is not tested, because of ASSERT
  // EXPECT_EQ(TIMESTAMP_NO_AUTO_SET, field->get_auto_set_type());

  // Not testing make_field, it also needs a mock TABLE object

  EXPECT_EQ(TYPE_OK, field->store("12:23:12.123456", 15, &my_charset_numeric));
  EXPECT_DOUBLE_EQ(122312.1235, field->val_real());

  EXPECT_EQ(TYPE_OK, field->store_decimal(dec));
  EXPECT_DOUBLE_EQ(122312.1234, field->val_real());

  EXPECT_EQ(TYPE_OK, field->store(-234545, false));
  EXPECT_DOUBLE_EQ(-234545.0, field->val_real());

  {
    // Test that store() with a to big number gives right error
    Mock_error_handler error_handler(thd(), ER_TRUNCATED_WRONG_VALUE);
    EXPECT_EQ(TYPE_WARN_OUT_OF_RANGE, field->store(0x80000000, true));
    // Test that error handler was actually called
    EXPECT_EQ(1, error_handler.handle_called());
    // Test that field contains expecte max time value
    EXPECT_DOUBLE_EQ(8385959, field->val_real());  // Max time value
  }

  EXPECT_EQ(TYPE_OK, field->store(1234545.555555));
  EXPECT_DOUBLE_EQ(1234545.5556, field->val_real());

  // Some of the functions inherited from Field
  Field *f = field;
  EXPECT_EQ(TYPE_OK, f->store_time(&time, MYSQL_TIMESTAMP_TIME));
  EXPECT_DOUBLE_EQ(122312.1234, f->val_real());  // Why decimals  here?
  EXPECT_STREQ("12:23:12.1234", f->val_str(&timeStr)->c_ptr());
  EXPECT_STREQ("122312", f->val_int_as_str(&timeStr, false)->c_ptr());
  EXPECT_TRUE(f->eq(copy));
  EXPECT_TRUE(f->eq_def(copy));

  // Not testing store(const char, uint, const CHARSET_INFO *,
  // enum_check_fields) it requires a mock table

  Mock_table m_table(thd());
  f->table = &m_table;
  struct timeval tv;
  int warnings = 0;
  EXPECT_FALSE(f->get_timestamp(&tv, &warnings));
  EXPECT_EQ(123400, tv.tv_usec);

  destroy(field);
}

TEST_F(FieldTest, FieldTimefCompare) {
  const int nFields = 7;
  uchar fieldBufs[nFields][7];

  MysqlTime times[nFields] = {
      {0, 0, 0, 12, 23, 12, 100000, true, MYSQL_TIMESTAMP_TIME},
      {0, 0, 0, 0, 0, 0, 10000, true, MYSQL_TIMESTAMP_TIME},
      {0, 0, 0, 0, 0, 0, 0, false, MYSQL_TIMESTAMP_TIME},
      {0, 0, 0, 0, 0, 0, 999900, false, MYSQL_TIMESTAMP_TIME},
      {0, 0, 0, 0, 0, 0, 999990, false, MYSQL_TIMESTAMP_TIME},
      {0, 0, 0, 11, 59, 59, 999999, false, MYSQL_TIMESTAMP_TIME},
      {0, 0, 0, 12, 00, 00, 100000, false, MYSQL_TIMESTAMP_TIME}};

  Field *fields[nFields];
  uchar sortStrings[nFields][6];
  for (int i = 0; i < nFields; ++i) {
    char fieldName[3];
    sprintf(fieldName, "f%c", i);
    fields[i] = new (thd()->mem_root) Field_timef(
        fieldBufs[i] + 1, fieldBufs[i], false, Field::NONE, fieldName, 6);

    longlong packed = TIME_to_longlong_packed(times[i]);
    EXPECT_EQ(0, fields[i]->store_packed(packed));
    fields[i]->make_sort_key(sortStrings[i], fields[i]->pack_length());
  }

  for (int i = 0; i < nFields; ++i)
    for (int j = 0; j < nFields; ++j) {
      String tmp;
      if (i < j) {
        EXPECT_GT(
            0, memcmp(sortStrings[i], sortStrings[j], fields[i]->pack_length()))
            << fields[i]->val_str(&tmp)->c_ptr() << " < "
            << fields[j]->val_str(&tmp)->c_ptr();
        EXPECT_GT(0, fields[i]->cmp(fields[i]->ptr, fields[j]->ptr))
            << fields[i]->val_str(&tmp)->c_ptr() << " < "
            << fields[j]->val_str(&tmp)->c_ptr();
      } else if (i > j) {
        EXPECT_LT(
            0, memcmp(sortStrings[i], sortStrings[j], fields[i]->pack_length()))
            << fields[i]->val_str(&tmp)->c_ptr() << " > "
            << fields[j]->val_str(&tmp)->c_ptr();
        EXPECT_LT(0, fields[i]->cmp(fields[i]->ptr, fields[j]->ptr))
            << fields[i]->val_str(&tmp)->c_ptr() << " > "
            << fields[j]->val_str(&tmp)->c_ptr();
      } else {
        EXPECT_EQ(
            0, memcmp(sortStrings[i], sortStrings[j], fields[i]->pack_length()))
            << fields[i]->val_str(&tmp)->c_ptr() << " = "
            << fields[j]->val_str(&tmp)->c_ptr();
        EXPECT_EQ(0, fields[i]->cmp(fields[i]->ptr, fields[j]->ptr))
            << fields[i]->val_str(&tmp)->c_ptr() << " = "
            << fields[j]->val_str(&tmp)->c_ptr();
      }
    }
}

TEST_F(FieldTest, FieldTime) {
  uchar fieldBuf[7];
  MysqlTime bigTime(0, 0, 0, 123, 45, 45, 555500, false, MYSQL_TIMESTAMP_TIME);

  Field_time *field = new (thd()->mem_root)
      Field_time(fieldBuf + 1, fieldBuf, false, Field::NONE, "f1");
  EXPECT_EQ(0, field->store_time(&bigTime, 4));
  MYSQL_TIME t;
  EXPECT_FALSE(field->get_time(&t));
  compareMysqlTime(bigTime, t);
}

const char *type_names3[] = {"one", "two", "three", nullptr};
unsigned int type_lengths3[] = {3U, 3U, 5U, 0U};
TYPELIB tl3 = {3, "tl3", type_names3, type_lengths3};

const char *type_names4[] = {"one", "two", "three", "four", nullptr};
unsigned int type_lengths4[] = {3U, 3U, 5U, 4U, 0U};
TYPELIB tl4 = {4, "tl4", type_names4, type_lengths4};

Field_set *FieldTest::create_field_set(TYPELIB *tl) {
  Field_set *f =
      new (thd()->mem_root) Field_set(nullptr,              // ptr_arg
                                      42,                   // len_arg
                                      nullptr,              // null_ptr_arg
                                      '\0',                 // null_bit_arg
                                      Field::NONE,          // auto_flags_arg
                                      "f1",                 // field_name_arg
                                      1,                    // packlength_arg
                                      tl,                   // typelib_arg
                                      &my_charset_latin1);  // charset_arg
  f->table = new Fake_TABLE(f);
  return f;
}

// Bug#13871079 RQG_MYISAM_DML_ALTER_VALGRIND FAILS ON VALGRIND PN PB2
TEST_F(FieldTest, CopyFieldSet) {
  int err;
  char fields[] = "one,two";
  uint64_t typeset = find_typeset(fields, &tl3, &err);
  EXPECT_EQ(0, err);

  // Using two different TYPELIBs will set cf->do_copy to do_field_string().
  Field_set *f_to = create_field_set(&tl3);
  bitmap_set_all(f_to->table->write_set);
  uchar to_fieldval = 0;
  f_to->ptr = &to_fieldval;

  Field_set *f_from = create_field_set(&tl4);
  bitmap_set_all(f_from->table->write_set);
  bitmap_set_all(f_from->table->read_set);
  uchar from_fieldval = static_cast<uchar>(typeset);
  f_from->ptr = &from_fieldval;

  Copy_field *cf = new (thd()->mem_root) Copy_field;
  cf->set(f_to, f_from, false);
  cf->invoke_do_copy();

  // Copy_field DTOR is not invoked in all contexts, so we may leak memory.
  EXPECT_FALSE(cf->tmp.is_alloced());

  delete static_cast<Fake_TABLE *>(f_to->table);
  delete static_cast<Fake_TABLE *>(f_from->table);
}

/*
  Tests that make_sort_key() is well behaved and does not cause buffer
  overruns nor writes a too short key. We stop at the first error seen.

  - field - The field whose make_sort_key() method we test.

  - from - A buffer of size field->pack_length() that we will trick
  the field into using as its record buffer.

  - expected - A buffer of size field->pack_length() + 1, the first n
  bytes of which make_sort_key() is expected to fill out. The n + 1:th
  byte is expected to be untouched. We try all possible values of n.

  - min_key_length - Some Field classes assert on a certain minimum
    key length. To avoid that, pass the minimum length here.
*/
void test_make_sort_key(Field *field, uchar *from, const uchar *expected,
                        int min_key_length) {
  const uint pack_length = field->pack_length();
  Fake_TABLE table(field);
  table.s->db_low_byte_first = false;
  field->ptr = from;

  for (uint key_length = min_key_length; key_length <= pack_length;
       ++key_length) {
    uchar buff[MAX_FIELD_WIDTH + 1];
    memset(buff, 'a', pack_length + 1);
    field->make_sort_key(buff, key_length);

    // Check for a too short key.
    for (uint i = 0; i < key_length; ++i)
      ASSERT_FALSE(buff[i] == 'a') << "Too few bytes written at " << i
                                   << " with buffer size " << key_length << ".";

    // Check for wrong result
    for (uint i = 0; i < key_length; ++i)
      ASSERT_EQ(expected[i], buff[i])
          << "Wrong output at " << i << " with buffer size " << key_length
          << " and pack length " << pack_length << ".";

    EXPECT_EQ('a', buff[key_length])
        << "Buffer overrun"
        << " with buffer size " << key_length << ".";
  }

  // Try key_length == pack_length
  uchar buff[MAX_FIELD_WIDTH];
  memset(buff, 'a', pack_length + 1);
  field->make_sort_key(buff, pack_length);
  EXPECT_EQ('a', buff[pack_length]) << "Buffer overrun";
}

// Convenience function for large values.
void test_make_sort_key(Field *field) {
  const int pack_length = field->pack_length();

  uchar from[MAX_FIELD_WIDTH];
  memset(from, 'b', pack_length);

  uchar to[MAX_FIELD_WIDTH + 1];
  memset(to, 'b', pack_length + 1);

  test_make_sort_key(field, from, to, 1);
}

extern "C" {
static size_t mock_strnxfrm(const CHARSET_INFO *, uchar *, size_t, uint,
                            const uchar *, size_t, uint);
}

class Mock_collation : public MY_COLLATION_HANDLER {
 public:
  Mock_collation() { strnxfrm = mock_strnxfrm; }
};

class Mock_charset : public CHARSET_INFO {
  Mock_collation mock_collation;

 public:
  mutable bool strnxfrm_called;
  Mock_charset() : strnxfrm_called(false) {
    cset = &my_charset_8bit_handler;
    coll = &mock_collation;
    mbmaxlen = 1;
    pad_attribute = PAD_SPACE;
  }
  ~Mock_charset() { EXPECT_TRUE(strnxfrm_called); }
};

static size_t mock_strnxfrm(const CHARSET_INFO *charset, uchar *, size_t dstlen,
                            uint, const uchar *, size_t, uint) {
  // CHARSET_INFO is not polymorphic, hence the abomination.
  static_cast<const Mock_charset *>(charset)->strnxfrm_called = true;
  return dstlen;
}

static void test_integer_field(Field *field) {
  uchar from[MAX_FIELD_WIDTH], expected[MAX_FIELD_WIDTH];
  const int pack_length = field->pack_length();
  for (int i = 0; i < pack_length; ++i) {
    from[i] = '0' + i;
#ifdef WORDS_BIGENDIAN
    expected[i] = '0' + i;
#else
    expected[pack_length - 1 - i] = '0' + i;
#endif
  }
  test_make_sort_key(field, from, expected, pack_length);
}

// Tests all make_sort_key() implementations.

// We keep the same order of classes here as in field.h in order to make it
// easy to manually verify that all field types have been tested.

TEST_F(FieldTest, MakeSortKey) {
  {
    SCOPED_TRACE("Field_decimal");
    Field_decimal fd(nullptr, 64, nullptr, '\0', Field::NONE, "", 0, false,
                     false);
    test_make_sort_key(&fd);
  }
  {
    SCOPED_TRACE("Field_new_decimal");
    Field_new_decimal fnd(64, true, "", 0, false);
    test_make_sort_key(&fnd);
  }
  {
    SCOPED_TRACE("Field_tiny");
    Field_tiny ft(nullptr, 0, nullptr, '\0', Field::NONE, "", false, true);
    test_make_sort_key(&ft);
  }
  {
    SCOPED_TRACE("Field_short");
    Field_short fs(0, "", true);
    test_integer_field(&fs);
  }
  {
    SCOPED_TRACE("Field_long");
    Field_long fl(0, false, "", true);
    test_integer_field(&fl);
  }
  {
    SCOPED_TRACE("Field_longlong");
    Field_longlong fll(nullptr, 64, nullptr, '\0', Field::NONE, "", false,
                       true);
    test_integer_field(&fll);
  }
  {
    SCOPED_TRACE("Field_float");
    Field_float ff(nullptr, 0, nullptr, '\0', Field::NONE, "", 0, false, false);
    float from = 0.0;
    uchar to[] = {128, 0, 0, 0};
    test_make_sort_key(&ff, reinterpret_cast<uchar *>(&from), to, 4);
  }
  {
    SCOPED_TRACE("Field_double");
    Field_double fd(nullptr, 0, nullptr, '\0', Field::NONE, "", 0, false,
                    false);
    double from = 0.0;
    uchar expected[] = {128, 0, 0, 0, 0, 0, 0, 0};
    test_make_sort_key(&fd, reinterpret_cast<uchar *>(&from), expected, 8);
  }
  {
    SCOPED_TRACE("Field_null");
    CHARSET_INFO cs;
    cs.state = MY_CHARSET_UNDEFINED;  // Avoid valgrind warning.
    cs.mbmaxlen = 1;
    Field_null fn(nullptr, 0, Field::NONE, "", &cs);
    EXPECT_TRUE(fn.is_nullable());
    EXPECT_TRUE(fn.is_null());
    test_make_sort_key(&fn);
  }
  {
    SCOPED_TRACE("Field_timestamp");
    Field_timestamp fts(false, "");
    test_integer_field(&fts);
  }
  {
    SCOPED_TRACE("Field_timestampf");
    Field_timestampf ftsf(nullptr, nullptr, 0, Field::NONE, "",
                          DATETIME_MAX_DECIMALS);
    test_make_sort_key(&ftsf);
  }
  {
    SCOPED_TRACE("field_newdate");
    Field_newdate fnd(false, "");
    uchar from[] = {'3', '2', '1'};
    uchar expected[] = {'1', '2', '3'};
    test_make_sort_key(&fnd, from, expected, 3);
  }
  {
    SCOPED_TRACE("Field_time");
    Field_time ft("");
    uchar from[] = {3, 2, 1};
    uchar expected[] = {129, 2, 3};
    test_make_sort_key(&ft, from, expected, 3);
  }
  {
    SCOPED_TRACE("Field_timef");
    Field_timef ftf(false, "", 0);
    test_make_sort_key(&ftf);
  }
  {
    SCOPED_TRACE("Field_datetime");
    Field_datetime fdt(nullptr, nullptr, '\0', Field::NONE, nullptr);
    test_integer_field(&fdt);
  }
  {
    SCOPED_TRACE("Field_string");
    Mock_charset mock_charset;
    Field_string fs(nullptr, 0, nullptr, '\0', Field::NONE, "", &mock_charset);
    uchar to;
    fs.make_sort_key(&to, 666);
  }
  {
    SCOPED_TRACE("Field_varstring");
    Mock_charset mock_charset;
    Fake_TABLE_SHARE fake_share(0);
    uchar ptr[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    Field_varstring fvs(ptr, 0, 0, nullptr, '\0', Field::NONE, "", &fake_share,
                        &mock_charset);
    uchar to;
    fvs.make_sort_key(&to, 666);
  }
  {
    SCOPED_TRACE("Field_blob");
    CHARSET_INFO cs;
    cs.state = MY_CHARSET_UNDEFINED;  // Avoid valgrind warning.
    cs.mbmaxlen = 1;
    Field_blob fb(0, false, "", &cs, false);
  }
  {
    SCOPED_TRACE("Field_enum");
    for (int pack_length = 1; pack_length <= 8; ++pack_length)
      for (int key_length = 1; key_length <= 8; ++key_length) {
        Field_enum fe(nullptr, 0, nullptr, '\0', Field::NONE, "", pack_length,
                      nullptr, &my_charset_bin);
        uchar from[] = {'1', '2', '3', '4', '5', '6', '7', '8'};
        uchar expected[] =
#ifdef WORDS_BIGENDIAN
            {'1', '2', '3', '4', '5', '6', '7', '8'};
        test_make_sort_key(&fe, from, expected, key_length);
#else
            {'8', '7', '6', '5', '4', '3', '2', '1'};
        test_make_sort_key(&fe, from, expected + 8 - pack_length, key_length);
#endif
      }
  }
  {
    SCOPED_TRACE("Field_bit");
    Field_bit fb(nullptr, 0, nullptr, '\0', nullptr, '\0', Field::NONE, "");
  }
}

void testCopyInteger(bool is_big_endian, bool is_unsigned) {
  const uchar from_template[] = {'1', '2', '3', '4', '5', '6', '7', '8'},
              expected_for_big_endian[] = {'1', '2', '3', '4',
                                           '5', '6', '7', '8'},
              expected_for_little_endian[] = {'8', '7', '6', '5',
                                              '4', '3', '2', '1'};

  const size_t max_length = sizeof(from_template);
  for (uint from_length = 1; from_length < max_length; ++from_length)
    for (uint to_length = 1; to_length <= from_length; ++to_length) {
      uchar to[] = {'0', '0', '0', '0', '0', '0', '0', '0', '0'};
      uchar from[max_length];
      memcpy(from, from_template, max_length);

      if (is_big_endian)
        copy_integer<true>(to, to_length, from, from_length, is_unsigned);
      else
        copy_integer<false>(to, to_length, from, from_length, is_unsigned);

      EXPECT_EQ('0', to[to_length])
          << "Buffer overrun @ position " << to_length << ".";

      ASSERT_EQ(is_unsigned ? 0 : 128, to[0] & 128)
          << "Sign bit should" << (is_unsigned ? " not" : "") << " be flipped";

      const uchar *expected =
          is_big_endian ? expected_for_big_endian
                        : expected_for_little_endian + max_length - from_length;

      for (uint i = 1; i < to_length; ++i) {
        ASSERT_FALSE(to[i] == '\0')
            << "Too few bytes written @ position " << i
            << " when copying a size " << from_length << " integer into a size "
            << to_length << " integer.";

        ASSERT_EQ(expected[i], to[i])
            << "Result differs at position " << i << " when copying a size "
            << from_length << " integer into a size " << to_length
            << " integer.";
      }
    }
}

// Test of the copy_integer<>() function.
TEST_F(FieldTest, copyInteger) {
  {
    SCOPED_TRACE("Big endian unsigned");
    testCopyInteger(true, true);
  }
  {
    SCOPED_TRACE("Big endian signed");
    testCopyInteger(true, false);
  }
  {
    SCOPED_TRACE("Little endian unsigned");
    testCopyInteger(false, true);
  }
  {
    SCOPED_TRACE("Little endian signed");
    testCopyInteger(false, false);
  }
}

}  // namespace field_unittests

#include "unittest/gunit/field_date-t.cc"
#include "unittest/gunit/field_datetime-t.cc"
#include "unittest/gunit/field_long-t.cc"
#include "unittest/gunit/field_newdecimal-t.cc"
#include "unittest/gunit/field_timestamp-t.cc"
