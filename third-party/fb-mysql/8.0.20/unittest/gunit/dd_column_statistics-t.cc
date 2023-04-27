/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "my_inttypes.h"
#include "sql/dd/impl/dictionary_impl.h"
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/column_statistics_impl.h"
#include "sql/histograms/equi_height.h"
#include "sql/histograms/singleton.h"
#include "sql/histograms/value_map.h"
#include "sql/histograms/value_map_type.h"
#include "test_utils.h"
#include "unittest/gunit/dd.h"
#include "unittest/gunit/test_utils.h"

namespace dd_column_statistics_unittest {

using namespace dd;
using namespace dd_unittest;

using dd_unittest::Mock_dd_field_longlong;
using dd_unittest::Mock_dd_field_varstring;

using my_testing::Server_initializer;

using ::testing::Invoke;
using ::testing::WithArgs;

void add_values(histograms::Value_map<longlong> &value_map) {
  value_map.add_values(0LL, 10);
}

void add_values(histograms::Value_map<ulonglong> &value_map) {
  value_map.add_values(0ULL, 10);
}

void add_values(histograms::Value_map<double> &value_map) {
  value_map.add_values(0.0, 10);
}

void add_values(histograms::Value_map<String> &value_map) {
  value_map.add_values(String(), 10);
}

void add_values(histograms::Value_map<MYSQL_TIME> &value_map) {
  MYSQL_TIME my_time;
  my_time.year = 2017;
  my_time.month = 1;
  my_time.day = 1;
  my_time.hour = 10;
  my_time.minute = 0;
  my_time.second = 0;
  my_time.second_part = 0;
  my_time.neg = false;
  my_time.time_type = MYSQL_TIMESTAMP_DATETIME;
  value_map.add_values(my_time, 10);
}

void add_values(histograms::Value_map<my_decimal> &value_map) {
  my_decimal my_decimal;
  double2my_decimal(0, 0.0, &my_decimal);
  value_map.add_values(my_decimal, 10);
}

template <class T>
void equi_height_test(histograms::Value_map_type value_map_type) {
  List<Field> m_field_list;
  Fake_TABLE_SHARE dummy_share(1);  // Keep Field_varstring constructor happy.

  // Add fields
  Mock_dd_field_longlong id;
  Mock_dd_field_longlong catalog_id;
  Mock_dd_field_varstring name(255, &dummy_share);
  Mock_dd_field_varstring schema_name(64, &dummy_share);
  Mock_dd_field_varstring table_name(64, &dummy_share);
  Mock_dd_field_varstring column_name(64, &dummy_share);
  Base_mock_field_json histogram;

  m_field_list.push_back(&id);
  m_field_list.push_back(&catalog_id);
  m_field_list.push_back(&name);
  m_field_list.push_back(&schema_name);
  m_field_list.push_back(&table_name);
  m_field_list.push_back(&column_name);
  m_field_list.push_back(&histogram);

  // Create table object (and table share implicitly).
  Fake_TABLE table(m_field_list);
  bitmap_set_all(table.write_set);
  dd::Raw_record r(&table);

  MEM_ROOT mem_root(PSI_NOT_INSTRUMENTED, 256);

  dd::Column_statistics_impl column_statistics;

  {
    /*
      Create a new scope, so that value_map goes out of scope before the
      MEM_ROOT is freed.
    */
    histograms::Value_map<T> value_map(&my_charset_latin1, value_map_type);
    add_values(value_map);

    histograms::Equi_height<T> equi_height(&mem_root, "schema", "table",
                                           "column", value_map_type);

    EXPECT_FALSE(equi_height.build_histogram(value_map, 1024));

    // Set the attributes
    column_statistics.set_histogram(&equi_height);
    column_statistics.set_schema_name("schema");
    column_statistics.set_table_name("table");
    column_statistics.set_column_name("column");

    /*
      Note: We cannot mock away and make expectations store_json/val_json since
      the function is not virtual.
    */
    ON_CALL(catalog_id, store(_, _))
        .WillByDefault(
            Invoke(&catalog_id, &Mock_dd_field_longlong::fake_store));

    ON_CALL(name, store(_, _, _))
        .WillByDefault(
            WithArgs<0>(Invoke(&name, &Mock_dd_field_varstring::fake_store)));

    ON_CALL(schema_name, store(_, _, _))
        .WillByDefault(WithArgs<0>(
            Invoke(&schema_name, &Mock_dd_field_varstring::fake_store)));
    ON_CALL(schema_name, val_str(_, _))
        .WillByDefault(WithArgs<1>(
            Invoke(&schema_name, &Mock_dd_field_varstring::fake_val_str)));

    ON_CALL(table_name, store(_, _, _))
        .WillByDefault(WithArgs<0>(
            Invoke(&table_name, &Mock_dd_field_varstring::fake_store)));
    ON_CALL(table_name, val_str(_, _))
        .WillByDefault(WithArgs<1>(
            Invoke(&table_name, &Mock_dd_field_varstring::fake_val_str)));

    ON_CALL(column_name, store(_, _, _))
        .WillByDefault(WithArgs<0>(
            Invoke(&column_name, &Mock_dd_field_varstring::fake_store)));
    ON_CALL(column_name, val_str(_, _))
        .WillByDefault(WithArgs<1>(
            Invoke(&column_name, &Mock_dd_field_varstring::fake_val_str)));

    EXPECT_CALL(catalog_id, store(_, _)).Times(1);
    EXPECT_CALL(name, store(_, _, _)).Times(1);
    EXPECT_CALL(schema_name,
                store(column_statistics.schema_name().c_str(), _, _))
        .Times(1);
    EXPECT_CALL(table_name, store(column_statistics.table_name().c_str(), _, _))
        .Times(1);
    EXPECT_CALL(column_name,
                store(column_statistics.column_name().c_str(), _, _))
        .Times(1);

    // Store attributes
    EXPECT_FALSE(column_statistics.store_attributes(&r));

    EXPECT_CALL(id, val_int()).Times(1);
    EXPECT_CALL(name, val_str(_, _)).Times(1);
    EXPECT_CALL(schema_name, val_str(_, _)).Times(1);
    EXPECT_CALL(table_name, val_str(_, _)).Times(1);
    EXPECT_CALL(column_name, val_str(_, _)).Times(1);

    dd::Column_statistics_impl column_statistics_restored;
    EXPECT_FALSE(column_statistics_restored.restore_attributes(r));

    // Verify that the stored and restored contents are the same.
    EXPECT_EQ(std::strcmp(column_statistics.schema_name().c_str(),
                          column_statistics_restored.schema_name().c_str()),
              0);
    EXPECT_EQ(std::strcmp(column_statistics.table_name().c_str(),
                          column_statistics_restored.table_name().c_str()),
              0);
    EXPECT_EQ(std::strcmp(column_statistics.column_name().c_str(),
                          column_statistics_restored.column_name().c_str()),
              0);

    // Check if the histogram contents is still the same
    EXPECT_EQ(column_statistics.histogram()->get_num_buckets(),
              column_statistics_restored.histogram()->get_num_buckets());

    EXPECT_EQ(
        column_statistics.histogram()->get_num_buckets_specified(),
        column_statistics_restored.histogram()->get_num_buckets_specified());

    EXPECT_EQ(
        column_statistics.histogram()->get_character_set()->number,
        column_statistics_restored.histogram()->get_character_set()->number);

    EXPECT_DOUBLE_EQ(
        column_statistics.histogram()->get_null_values_fraction(),
        column_statistics_restored.histogram()->get_null_values_fraction());

    EXPECT_DOUBLE_EQ(
        column_statistics.histogram()->get_sampling_rate(),
        column_statistics_restored.histogram()->get_sampling_rate());
  }
}

template <class T>
void singleton_test(histograms::Value_map_type value_map_type) {
  List<Field> m_field_list;
  Fake_TABLE_SHARE dummy_share(1);  // Keep Field_varstring constructor happy.

  // Add fields
  Mock_dd_field_longlong id;
  Mock_dd_field_longlong catalog_id;
  Mock_dd_field_varstring name(255, &dummy_share);
  Mock_dd_field_varstring schema_name(64, &dummy_share);
  Mock_dd_field_varstring table_name(64, &dummy_share);
  Mock_dd_field_varstring column_name(64, &dummy_share);
  Base_mock_field_json histogram;

  m_field_list.push_back(&id);
  m_field_list.push_back(&catalog_id);
  m_field_list.push_back(&name);
  m_field_list.push_back(&schema_name);
  m_field_list.push_back(&table_name);
  m_field_list.push_back(&column_name);
  m_field_list.push_back(&histogram);

  // Create table object (and table share implicitly).
  Fake_TABLE table(m_field_list);
  bitmap_set_all(table.write_set);
  dd::Raw_record r(&table);

  MEM_ROOT mem_root(PSI_NOT_INSTRUMENTED, 256);

  dd::Column_statistics_impl column_statistics;

  {
    /*
      Create a new scope, so that value_map goes out of scope before the
      MEM_ROOT is freed.
    */
    histograms::Value_map<T> value_map(&my_charset_latin1, value_map_type);
    add_values(value_map);

    histograms::Singleton<T> singleton(&mem_root, "schema", "table", "column",
                                       value_map_type);

    EXPECT_FALSE(singleton.build_histogram(value_map, 1024));

    // Set the attributes
    column_statistics.set_histogram(&singleton);
    column_statistics.set_schema_name("schema");
    column_statistics.set_table_name("table");
    column_statistics.set_column_name("column");

    /*
      Note: We cannot mock away and make expectations store_json/val_json since
      the function is not virtual.
    */
    ON_CALL(catalog_id, store(_, _))
        .WillByDefault(
            Invoke(&catalog_id, &Mock_dd_field_longlong::fake_store));

    ON_CALL(name, store(_, _, _))
        .WillByDefault(
            WithArgs<0>(Invoke(&name, &Mock_dd_field_varstring::fake_store)));

    ON_CALL(schema_name, store(_, _, _))
        .WillByDefault(WithArgs<0>(
            Invoke(&schema_name, &Mock_dd_field_varstring::fake_store)));
    ON_CALL(schema_name, val_str(_, _))
        .WillByDefault(WithArgs<1>(
            Invoke(&schema_name, &Mock_dd_field_varstring::fake_val_str)));

    ON_CALL(table_name, store(_, _, _))
        .WillByDefault(WithArgs<0>(
            Invoke(&table_name, &Mock_dd_field_varstring::fake_store)));
    ON_CALL(table_name, val_str(_, _))
        .WillByDefault(WithArgs<1>(
            Invoke(&table_name, &Mock_dd_field_varstring::fake_val_str)));

    ON_CALL(column_name, store(_, _, _))
        .WillByDefault(WithArgs<0>(
            Invoke(&column_name, &Mock_dd_field_varstring::fake_store)));
    ON_CALL(column_name, val_str(_, _))
        .WillByDefault(WithArgs<1>(
            Invoke(&column_name, &Mock_dd_field_varstring::fake_val_str)));

    EXPECT_CALL(catalog_id, store(_, _)).Times(1);
    EXPECT_CALL(name, store(_, _, _)).Times(1);
    EXPECT_CALL(schema_name,
                store(column_statistics.schema_name().c_str(), _, _))
        .Times(1);
    EXPECT_CALL(table_name, store(column_statistics.table_name().c_str(), _, _))
        .Times(1);
    EXPECT_CALL(column_name,
                store(column_statistics.column_name().c_str(), _, _))
        .Times(1);

    // Store attributes
    EXPECT_FALSE(column_statistics.store_attributes(&r));

    EXPECT_CALL(id, val_int()).Times(1);
    EXPECT_CALL(name, val_str(_, _)).Times(1);
    EXPECT_CALL(schema_name, val_str(_, _)).Times(1);
    EXPECT_CALL(table_name, val_str(_, _)).Times(1);
    EXPECT_CALL(column_name, val_str(_, _)).Times(1);

    dd::Column_statistics_impl column_statistics_restored;
    EXPECT_FALSE(column_statistics_restored.restore_attributes(r));

    // Verify that the stored and restored contents are the same.
    EXPECT_EQ(std::strcmp(column_statistics.schema_name().c_str(),
                          column_statistics_restored.schema_name().c_str()),
              0);
    EXPECT_EQ(std::strcmp(column_statistics.table_name().c_str(),
                          column_statistics_restored.table_name().c_str()),
              0);
    EXPECT_EQ(std::strcmp(column_statistics.column_name().c_str(),
                          column_statistics_restored.column_name().c_str()),
              0);

    // Check if the histogram contents is still the same
    EXPECT_EQ(column_statistics.histogram()->get_num_buckets(),
              column_statistics_restored.histogram()->get_num_buckets());

    EXPECT_EQ(
        column_statistics.histogram()->get_character_set()->number,
        column_statistics_restored.histogram()->get_character_set()->number);

    EXPECT_DOUBLE_EQ(
        column_statistics.histogram()->get_null_values_fraction(),
        column_statistics_restored.histogram()->get_null_values_fraction());
  }
}

TEST(ColumnStatisticsTest, StoreAndRestoreAttributesEquiHeight) {
  // Dictionary_impl *m_dict;                // Dictionary instance.
  my_testing::Server_initializer m_init;  // Server initializer.
  m_init.SetUp();
  equi_height_test<longlong>(histograms::Value_map_type::INT);
  equi_height_test<longlong>(histograms::Value_map_type::SET);
  equi_height_test<longlong>(histograms::Value_map_type::ENUM);
  equi_height_test<ulonglong>(histograms::Value_map_type::UINT);
  equi_height_test<String>(histograms::Value_map_type::STRING);
  equi_height_test<my_decimal>(histograms::Value_map_type::DECIMAL);
  equi_height_test<MYSQL_TIME>(histograms::Value_map_type::DATE);
  equi_height_test<MYSQL_TIME>(histograms::Value_map_type::TIME);
  equi_height_test<MYSQL_TIME>(histograms::Value_map_type::DATETIME);
  equi_height_test<double>(histograms::Value_map_type::DOUBLE);
  m_init.TearDown();
}

TEST(ColumnStatisticsTest, StoreAndRestoreAttributesSingleton) {
  my_testing::Server_initializer m_init;  // Server initializer.
  m_init.SetUp();
  singleton_test<longlong>(histograms::Value_map_type::INT);
  singleton_test<longlong>(histograms::Value_map_type::SET);
  singleton_test<longlong>(histograms::Value_map_type::ENUM);
  singleton_test<ulonglong>(histograms::Value_map_type::UINT);
  singleton_test<String>(histograms::Value_map_type::STRING);
  singleton_test<my_decimal>(histograms::Value_map_type::DECIMAL);
  singleton_test<MYSQL_TIME>(histograms::Value_map_type::DATE);
  singleton_test<MYSQL_TIME>(histograms::Value_map_type::TIME);
  singleton_test<MYSQL_TIME>(histograms::Value_map_type::DATETIME);
  singleton_test<double>(histograms::Value_map_type::DOUBLE);
  m_init.TearDown();
}

}  // namespace dd_column_statistics_unittest
