/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <stddef.h>

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "m_string.h"
#include "my_inttypes.h"
#include "sql/dd/dd.h"
#include "sql/dd/impl/properties_impl.h"
#include "sql/dd/impl/sdi.h"
#include "sql/dd/impl/sdi_impl.h"
#include "sql/dd/impl/types/column_impl.h"
#include "sql/dd/impl/types/entity_object_impl.h"
#include "sql/dd/impl/types/index_impl.h"
#include "sql/dd/impl/types/table_impl.h"
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/sdi_file.h"
#include "sql/dd/types/column.h"
#include "sql/dd/types/column_statistics.h"
#include "sql/dd/types/column_type_element.h"
#include "sql/dd/types/foreign_key.h"
#include "sql/dd/types/foreign_key_element.h"
#include "sql/dd/types/index.h"
#include "sql/dd/types/index_element.h"
#include "sql/dd/types/partition.h"
#include "sql/dd/types/partition_index.h"
#include "sql/dd/types/partition_value.h"
#include "sql/dd/types/table.h"
#include "sql/dd/types/tablespace.h"
#include "sql/dd/types/tablespace_file.h"
#include "sql/histograms/equi_height.h"
#include "sql/histograms/histogram.h"
#include "sql/histograms/value_map.h"

namespace {
int FANOUT = 3;
}

namespace dd {
class Sdi_wcontext;
class Sdi_rcontext;
}  // namespace dd

namespace sdi_unittest {

typedef dd::Foreign_key Foreign_key;  // To avoid the one from sql_class.h

::dd::Sdi_wcontext *get_wctx();
::dd::Sdi_rcontext *get_rctx();

bool equal_prefix_chars_driver(const dd::String_type &a,
                               const dd::String_type &b, size_t prefix);

// Mocking functions

static void mock_properties(dd::Properties &p, uint64 size) {
  for (uint64 i = 0; i < size; ++i) {
    dd::String_type key =
        (dynamic_cast<dd::Properties_impl &>(p)).valid_key_at(i);
    p.set(key, i);
  }
}

static void mock_dd_obj(dd::Column_type_element *cte) {
  if (cte) {
    cte->set_name("mock_column_type_element");
  }
}

static void mock_dd_obj(dd::Column *c) {
  static dd::Object_id curid = 10000;
  dynamic_cast<dd::Entity_object_impl *>(c)->set_id(curid++);
  c->set_type(dd::enum_column_types::ENUM);
  c->set_char_length(42);
  c->set_numeric_precision(42);
  c->set_numeric_scale(42);
  c->set_datetime_precision(42);
  c->set_default_value("mocked default column value");
  c->set_default_option("mocked default option");
  c->set_update_option("mocked update option");
  c->set_comment("mocked column comment");
  c->set_srs_id({4326});
  mock_properties(c->se_private_data(), FANOUT);

  for (int i = 0; i < FANOUT; ++i) {
    mock_dd_obj(c->add_element());
  }
  if (c->ordinal_position() == 0) {
    dynamic_cast<dd::Column_impl *>(c)->set_ordinal_position(1);
  }
}

static void mock_column_statistics_obj(dd::Column_statistics *c,
                                       MEM_ROOT *mem_root) {
  c->set_schema_name("my_schema");
  c->set_table_name("my_table");
  c->set_column_name("my_column");

  histograms::Value_map<longlong> int_values(&my_charset_latin1,
                                             histograms::Value_map_type::INT);
  int_values.add_values(0LL, 10);

  histograms::Equi_height<longlong> *equi_height = new (mem_root)
      histograms::Equi_height<longlong>(mem_root, "my_schema", "my_table",
                                        "my_column",
                                        histograms::Value_map_type::INT);

  EXPECT_FALSE(equi_height->build_histogram(int_values, 1024));
  c->set_histogram(equi_height);
}

static void mock_dd_obj(dd::Index_element *ie) {
  ie->set_length(42);
  ie->set_order(dd::Index_element::ORDER_DESC);
}

static void mock_dd_obj(dd::Index *i, dd::Column *c = nullptr) {
  static dd::Object_id curid = 10000;
  dynamic_cast<dd::Entity_object_impl *>(i)->set_id(curid++);
  i->set_comment("mocked index comment");
  mock_properties(i->options(), FANOUT);
  mock_properties(i->se_private_data(), FANOUT);
  i->set_engine("mocked index engine");
  i->set_type(dd::Index::IT_MULTIPLE);
  i->set_algorithm(dd::Index::IA_HASH);

  mock_dd_obj(i->add_element(c));

  if (i->ordinal_position() == 0) {
    dynamic_cast<dd::Index_impl *>(i)->set_ordinal_position(1);
  }
}

static void mock_dd_obj(dd::Foreign_key_element *fke) {
  fke->referenced_column_name("mocked referenced column name");
}

static void mock_dd_obj(dd::Foreign_key *fk) {
  fk->set_match_option(Foreign_key::OPTION_PARTIAL);
  fk->set_update_rule(Foreign_key::RULE_CASCADE);
  fk->set_delete_rule(Foreign_key::RULE_CASCADE);
  fk->set_referenced_table_name("mocked referenced table name");
  for (int i = 0; i < FANOUT; ++i) {
    mock_dd_obj(fk->add_element());
  }
}

static void mock_dd_obj(dd::Partition_index *pi) {
  mock_properties(pi->options(), FANOUT);
  mock_properties(pi->se_private_data(), FANOUT);
}

static void mock_dd_obj(dd::Partition_value *pv) {
  pv->set_list_num(42);
  pv->set_column_num(42);
  pv->set_value_utf8("mocked partition value");
}

static void mock_dd_obj(dd::Partition *p, dd::Index *ix = nullptr) {
  p->set_number(42);
  p->set_engine("mocked partition engine");
  p->set_comment("mocked comment");
  mock_properties(p->options(), FANOUT);
  mock_properties(p->se_private_data(), FANOUT);

  for (int j = 0; j < FANOUT; ++j) {
    mock_dd_obj(p->add_value());
    mock_dd_obj(p->add_index(ix));
  }
}

static void mock_dd_obj(dd::Table *t) {
  mock_properties(t->options(), FANOUT);
  t->set_created(42);
  t->set_last_altered(42);

  t->set_engine("mocked table engine");
  t->set_comment("mocked table comment");
  mock_properties(t->se_private_data(), FANOUT);

  t->set_partition_type(dd::Table::PT_RANGE);
  t->set_default_partitioning(dd::Table::DP_NUMBER);
  t->set_partition_expression("mocked partition expression");
  t->set_subpartition_type(dd::Table::ST_LINEAR_HASH);
  t->set_default_subpartitioning(dd::Table::DP_YES);
  t->set_subpartition_expression("mocked subpartition expression");

  for (int i = 0; i < FANOUT; ++i) {
    mock_dd_obj(t->add_foreign_key());
    dd::Column *c = t->add_column();
    mock_dd_obj(c);
    dd::Index *ix = t->add_index();
    mock_dd_obj(ix, c);
    dd::Partition *p = t->add_partition();
    mock_dd_obj(p, ix);
  }
}

static void mock_dd_obj(dd::Tablespace_file *f) {
  f->set_filename("mock_tablespace_file");
  mock_properties(f->se_private_data(), FANOUT);
}

static void mock_dd_obj(dd::Tablespace *ts) {
  ts->set_comment("Mocked tablespace");
  mock_properties(ts->options(), FANOUT);
  mock_properties(ts->se_private_data(), FANOUT);
  ts->set_engine("mocked storage engine name");
  for (int i = 0; i < FANOUT; i++) {
    mock_dd_obj(ts->add_file());
  }
}

class SdiTest : public ::testing::Test {
 protected:
  void SetUp() {}

  void TearDown() {}

  SdiTest() {}

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(SdiTest);
};

bool diff(const dd::String_type &expected, dd::String_type actual) {
  if (actual == expected) {
    return false;
  }
  typedef dd::String_type::const_iterator csit_t;
  typedef std::pair<csit_t, csit_t> diff_t_;
  csit_t expected_end = expected.end();
  actual.resize(expected.size());
  csit_t actual_end = actual.end();
  diff_t_ diff =
      std::mismatch(expected.begin(), expected.end(), actual.begin());

  std::cout << dd::String_type(expected.begin(), diff.first) << "\n@ offset "
            << diff.first - expected.begin() << ":\n< "
            << dd::String_type(diff.first, expected_end) << "\n---\n> "
            << dd::String_type(diff.second, actual_end) << std::endl;
  return true;
}

template <typename T>
dd::String_type serialize_drv(const T *dd_obj) {
  dd::RJ_StringBuffer buf;
  dd::Sdi_writer w(buf);
  dd::String_type s = "driver schema";
  dd::Sdi_wcontext *wctx = get_wctx();
  dd_obj->serialize(wctx, &w);
  return dd::String_type(buf.GetString(), buf.GetSize());
}

template <typename T>
T *deserialize_drv(const dd::String_type &sdi) {
  T *dst_obj = dd::create_object<T>();
  dd::RJ_Document doc;
  doc.Parse<0>(sdi.c_str());
  dd::Sdi_rcontext *rctx = get_rctx();
  dst_obj->deserialize(rctx, doc);
  return dst_obj;
}

template <class T>
dd::String_type api_serialize(const T *tp) {
  return dd::serialize(*tp);
}

dd::String_type api_serialize(const dd::Table *table) {
  return dd::serialize(nullptr, *table, "api_schema");
}

template <typename T>
void verify(T *dd_obj) {
  dd::String_type sdi = serialize_drv(dd_obj);
  // std::cout << "Verifying json: \n" << sdi << std::endl;
  ASSERT_GT(sdi.size(), 0u);
  std::unique_ptr<T> dst_obj{deserialize_drv<T>(sdi)};
  dd::String_type dst_sdi = serialize_drv(dst_obj.get());
  EXPECT_EQ(dst_sdi, sdi);
}

template <typename T>
void simple_test() {
  std::unique_ptr<T> dd_obj(dd::create_object<T>());
  mock_dd_obj(dd_obj.get());
  verify(dd_obj.get());
}

template <typename AP>
void api_test(const AP &ap) {
  typedef typename AP::element_type T;
  dd::Sdi_type sdi = api_serialize(ap.get());
  std::unique_ptr<T> d(dd::create_object<T>());
  dd::deserialize(nullptr, sdi, d.get());

  dd::Sdi_type d_sdi = api_serialize(d.get());

  EXPECT_EQ(d_sdi.size(), sdi.size());
  EXPECT_EQ(d_sdi, sdi);
  ASSERT_FALSE(diff(sdi, d_sdi));
}

// Test cases

TEST(SdiTest, Column_type_element) { simple_test<dd::Column_type_element>(); }

TEST(SdiTest, Column) { simple_test<dd::Column>(); }

TEST(SdiTest, Column_statistics) {
  std::unique_ptr<dd::Column_statistics> dd_obj(
      dd::create_object<dd::Column_statistics>());

  MEM_ROOT mem_root;
  init_alloc_root(PSI_NOT_INSTRUMENTED, &mem_root, 256, 0);

  mock_column_statistics_obj(dd_obj.get(), &mem_root);

  /*
    We only support proper serialization of column statistics. Proper
    deserialization (which will include re-generating histogram data) will be
    available later.
  */
  dd::String_type sdi = serialize_drv(dd_obj.get());
  EXPECT_FALSE(sdi.empty());

  std::unique_ptr<dd::Column_statistics> deserialized{
      deserialize_drv<dd::Column_statistics>(sdi)};

  EXPECT_TRUE(dd_obj.get()->schema_name() == deserialized.get()->schema_name());
  EXPECT_TRUE(dd_obj.get()->table_name() == deserialized.get()->table_name());
  EXPECT_TRUE(dd_obj.get()->column_name() == deserialized.get()->column_name());
  free_root(&mem_root, MYF(0));
}

TEST(SdiTest, Index_element) { simple_test<dd::Index_element>(); }

TEST(SdiTest, Index) { simple_test<dd::Index>(); }

TEST(SdiTest, Foreign_key_element) { simple_test<dd::Foreign_key_element>(); }

TEST(SdiTest, Foreign_key) { simple_test<dd::Foreign_key>(); }

TEST(SdiTest, Partition_index) { simple_test<dd::Partition_index>(); }

TEST(SdiTest, Partition_value) { simple_test<dd::Partition_value>(); }

TEST(SdiTest, Partition) { simple_test<dd::Partition>(); }

TEST(SdiTest, Table) { simple_test<dd::Table>(); }

TEST(SdiTest, Tablespace_file) { simple_test<dd::Tablespace_file>(); }

TEST(SdiTest, Tablespace) { simple_test<dd::Tablespace>(); }

TEST(SdiTest, Table_API) {
  std::unique_ptr<dd::Table> t(dd::create_object<dd::Table>());
  mock_dd_obj(t.get());
  // std::cout << "Serialized table:\n" << serialize_ap(t) << std::endl;
  api_test(t);
}

TEST(SdiTest, Tablespace_API) {
  std::unique_ptr<dd::Tablespace> ts(dd::create_object<dd::Tablespace>());
  mock_dd_obj(ts.get());

  api_test(ts);
}

#ifdef DBUG_OFF
TEST(SdiTest, Serialization_perf) {
  std::unique_ptr<dd::Table> t(dd::create_object<dd::Table>());
  FANOUT = 20;
  mock_dd_obj(t.get());

  for (int i = 0; i < 1; ++i) {
    dd::String_type sdi = dd::serialize(nullptr, *t, "perftest");
    EXPECT_GT(sdi.size(), 100000u);
  }
}
#endif /* DBUG_OFF */

TEST(SdiTest, CharPromotion) {
  signed char x = 127;
  unsigned char ux = x;
  EXPECT_EQ(127u, ux);

  unsigned short usx = x;
  EXPECT_EQ(127u, usx);

  unsigned int uix = x;
  EXPECT_EQ(127u, usx);

  unsigned char tmp = 0xe0;
  x = static_cast<signed char>(tmp);
  EXPECT_EQ(-32, x);

  ux = x;
  EXPECT_EQ(224u, ux);

  usx = x;
  short sx = x;
  EXPECT_EQ(-32, sx);
  unsigned short usy = sx;
  EXPECT_EQ(usx, usy);
  EXPECT_EQ(65504u, usx);

  uix = x;
  int ix = x;
  EXPECT_EQ(-32, ix);
  unsigned int uiy = ix;
  EXPECT_EQ(uix, uiy);
  EXPECT_EQ(4294967264u, uix);
}

TEST(SdiTest, Utf8Filename) {
  dd::Table_impl x{};
  x.set_name("\xe0\xa0\x80");
  x.set_id(42);
  dd::String_type path = dd::sdi_file::sdi_filename(x.id(), x.name(), "foobar");
  std::replace(path.begin(), path.end(), '\\', '/');
  EXPECT_EQ("./foobar/@0800_42.sdi", path);
}

TEST(SdiTest, Utf8FilenameTrunc) {
  dd::String_type name{"\xe0\xa0\x80"};
  for (int i = 0; i < 16; ++i) {
    name.append("\xe0\xa0\x80");
  }
  EXPECT_EQ(51u, name.length());

  dd::Table_impl x{};
  x.set_name(name);
  x.set_id(42);
  dd::String_type fn = dd::sdi_file::sdi_filename(x.id(), x.name(), "foobar");
  std::replace(fn.begin(), fn.end(), '\\', '/');
  EXPECT_EQ(96u, fn.length());
  EXPECT_EQ(
      "./foobar/"
      "@0800@0800@0800@0800@0800@0800@0800@0800@0800@0800@0800@0800@0800@0800@"
      "0800@0800_42.sdi",
      fn);
}

TEST(SdiTest, EqualPrefixCharsAscii) {
  ASSERT_TRUE(equal_prefix_chars_driver("a", "a", 16));
  ASSERT_FALSE(equal_prefix_chars_driver("a", "b", 16));
  ASSERT_FALSE(equal_prefix_chars_driver("aaa", "aaab", 16));
  ASSERT_TRUE(equal_prefix_chars_driver("abcdefghijklmnon_aaa",
                                        "abcdefghijklmnon_bbb", 16));
}

TEST(SdiTest, EqualPrefixCharsUtf8) {
  ASSERT_TRUE(equal_prefix_chars_driver("\xe0\xa0\x80", "\xe0\xa0\x80", 16));
  ASSERT_FALSE(
      equal_prefix_chars_driver("\xe0\xa0\x80", "\xf0\x90\x80\x80", 16));
  ASSERT_FALSE(equal_prefix_chars_driver(
      "\xe0\xa0\x80\xe0\xa0\x80\xe0\xa0\x80",
      "\xe0\xa0\x80\xe0\xa0\x80\xe0\xa0\x80\xf0\x90\x80\x80", 16));

  ASSERT_TRUE(equal_prefix_chars_driver(
      "abc\xe0\xa0\x80"
      "def\xe0\xa0\x80ghi\xe0\xa0\x80jkl\xe0\xa0\x80_aaa",
      "abc\xe0\xa0\x80"
      "def\xe0\xa0\x80ghi\xe0\xa0\x80jkl\xe0\xa0\x80_bbb",
      16));
}

// Verify that default null, implicit non-null (by setting a value)
// and explicit null (by setting to null) is correctly preserved
// through serialization and deserialization.
TEST(SdiTest, Bug_25792649) {
  dd::Column_impl cobj;
  cobj.set_ordinal_position(1);

  ASSERT_TRUE(cobj.is_numeric_scale_null());
  ASSERT_TRUE(cobj.is_datetime_precision_null());
  ASSERT_TRUE(cobj.is_default_value_null());
  ASSERT_TRUE(cobj.is_default_value_utf8_null());
  ASSERT_TRUE(cobj.is_generation_expression_null());
  ASSERT_TRUE(cobj.is_generation_expression_utf8_null());

  dd::String_type sdi = serialize_drv(&cobj);
  dd::RJ_Document doc;
  doc.Parse<0>(sdi.c_str());
  dd::Sdi_rcontext *rctx = get_rctx();
  dd::Column_impl dst;
  dst.deserialize(rctx, doc);
  ASSERT_TRUE(dst.is_numeric_scale_null());
  ASSERT_TRUE(dst.is_datetime_precision_null());
  ASSERT_TRUE(dst.is_default_value_null());
  ASSERT_TRUE(dst.is_default_value_utf8_null());
  ASSERT_TRUE(dst.is_generation_expression_null());
  ASSERT_TRUE(dst.is_generation_expression_utf8_null());

  cobj.set_numeric_scale(10);
  cobj.set_datetime_precision(10);
  cobj.set_default_value("This is my default");
  cobj.set_default_value_utf8("This is my utf8 default");
  cobj.set_generation_expression("This is my generation expression");
  cobj.set_generation_expression_utf8("This is my utf8 generation expression");

  ASSERT_FALSE(cobj.is_numeric_scale_null());
  ASSERT_FALSE(cobj.is_datetime_precision_null());
  ASSERT_FALSE(cobj.is_default_value_null());
  ASSERT_FALSE(cobj.is_default_value_utf8_null());
  ASSERT_FALSE(cobj.is_generation_expression_null());
  ASSERT_FALSE(cobj.is_generation_expression_utf8_null());

  sdi = serialize_drv(&cobj);
  doc.Parse<0>(sdi.c_str());
  dd::Column_impl dst2;
  dst2.deserialize(rctx, doc);

  ASSERT_FALSE(dst2.is_numeric_scale_null());
  ASSERT_FALSE(dst2.is_datetime_precision_null());
  ASSERT_FALSE(dst2.is_default_value_null());
  ASSERT_FALSE(dst2.is_default_value_utf8_null());
  ASSERT_FALSE(dst2.is_generation_expression_null());
  ASSERT_FALSE(dst2.is_generation_expression_utf8_null());

  cobj.set_numeric_scale_null(true);
  cobj.set_datetime_precision_null(true);
  cobj.set_default_value_null(true);
  cobj.set_default_value_utf8_null(true);
  cobj.set_generation_expression("");
  cobj.set_generation_expression_utf8("");

  ASSERT_TRUE(cobj.is_numeric_scale_null());
  ASSERT_TRUE(cobj.is_datetime_precision_null());
  ASSERT_TRUE(cobj.is_default_value_null());
  ASSERT_TRUE(cobj.is_default_value_utf8_null());
  ASSERT_TRUE(cobj.is_generation_expression_null());
  ASSERT_TRUE(cobj.is_generation_expression_utf8_null());

  sdi = serialize_drv(&cobj);
  doc.Parse<0>(sdi.c_str());
  dd::Column_impl dst3;
  dst3.deserialize(rctx, doc);

  ASSERT_TRUE(dst3.is_numeric_scale_null());
  ASSERT_TRUE(dst3.is_datetime_precision_null());
  ASSERT_TRUE(dst3.is_default_value_null());
  ASSERT_TRUE(dst3.is_default_value_utf8_null());
  ASSERT_TRUE(dst3.is_generation_expression_null());
  ASSERT_TRUE(dst3.is_generation_expression_utf8_null());
}
}  // namespace sdi_unittest
