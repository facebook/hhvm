/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/types/column_statistics_impl.h"

#include <string.h>

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "m_ctype.h"
#include "m_string.h"  // STRING_WITH_LEN
#include "my_dbug.h"
#include "mysql_com.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"                       // current_thd
#include "sql/dd/impl/dictionary_impl.h"           // Dictionary_impl
#include "sql/dd/impl/raw/object_keys.h"           // Primary_id_key
#include "sql/dd/impl/raw/raw_record.h"            // Raw_record
#include "sql/dd/impl/sdi_impl.h"                  // sdi read/write functions
#include "sql/dd/impl/tables/column_statistics.h"  // Column_statistics
#include "sql/dd/impl/transaction_impl.h"          // Open_dictionary_tables_ctx
#include "sql/histograms/histogram.h"              // histograms::Histogram
#include "sql/json_dom.h"                          // Json_*
#include "template_utils.h"

namespace dd {

///////////////////////////////////////////////////////////////////////////

String_type Column_statistics::create_name(const String_type &schema_name,
                                           const String_type &table_name,
                                           const String_type &column_name) {
  String_type output;

  output.assign(schema_name);
  output.push_back('\037');
  output.append(table_name);
  output.push_back('\037');

  /*
    Column names are always case insensitive, so convert it to lowercase.
    Lookups in the dictionary is always done using the name, so this should
    ensure that we always get back our object.
  */
  DBUG_ASSERT(column_name.length() <= NAME_LEN);
  char lowercase_name[NAME_LEN + 1];  // Max column length name + \0
  memcpy(lowercase_name, column_name.c_str(), column_name.length() + 1);
  my_casedn_str(system_charset_info, lowercase_name);
  output.append(lowercase_name, column_name.length());

  return output;
}

///////////////////////////////////////////////////////////////////////////

void Column_statistics::create_mdl_key(const String_type &schema_name,
                                       const String_type &table_name,
                                       const String_type &column_name,
                                       MDL_key *mdl_key) {
  /*
    Column names are always case insensitive, so convert it to lowercase.
    Lookups in MDL is always done using this method, so this should
    ensure that we always have consistent locks.
  */
  DBUG_ASSERT(column_name.length() <= NAME_LEN);
  char lowercase_name[NAME_LEN + 1];  // Max column length name + \0
  memcpy(lowercase_name, column_name.c_str(), column_name.length() + 1);
  my_casedn_str(system_charset_info, lowercase_name);

  mdl_key->mdl_key_init(MDL_key::COLUMN_STATISTICS, schema_name.c_str(),
                        table_name.c_str(), lowercase_name);
}

///////////////////////////////////////////////////////////////////////////
// Column_statistics_impl implementation.
///////////////////////////////////////////////////////////////////////////

bool Column_statistics_impl::restore_attributes(const Raw_record &r) {
  restore_id(r, dd::tables::Column_statistics::FIELD_ID);
  restore_name(r, dd::tables::Column_statistics::FIELD_NAME);

  m_schema_name = r.read_str(dd::tables::Column_statistics::FIELD_SCHEMA_NAME);
  m_table_name = r.read_str(dd::tables::Column_statistics::FIELD_TABLE_NAME);
  m_column_name = r.read_str(dd::tables::Column_statistics::FIELD_COLUMN_NAME);

  Json_wrapper wrapper;
  if (r.read_json(dd::tables::Column_statistics::FIELD_HISTOGRAM, &wrapper))
    return true; /* purecov: deadcode */

  Json_dom *json_dom = wrapper.to_dom(current_thd);
  if (json_dom->json_type() != enum_json_type::J_OBJECT)
    return true; /* purecov: deadcode */

  const Json_object *json_object = down_cast<const Json_object *>(json_dom);
  m_histogram = histograms::Histogram::json_to_histogram(
      &m_mem_root, {m_schema_name.data(), m_schema_name.size()},
      {m_table_name.data(), m_table_name.size()},
      {m_column_name.data(), m_column_name.size()}, *json_object);
  if (m_histogram == nullptr) return true; /* purecov: deadcode */
  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Column_statistics_impl::store_attributes(Raw_record *r) {
  Json_object json_object;
  m_histogram->histogram_to_json(&json_object);

  Json_wrapper wrapper(&json_object);
  wrapper.set_alias();

  return store_id(r, dd::tables::Column_statistics::FIELD_ID) ||
         store_name(r, dd::tables::Column_statistics::FIELD_NAME) ||
         r->store(dd::tables::Column_statistics::FIELD_CATALOG_ID,
                  Dictionary_impl::instance()->default_catalog_id()) ||
         r->store(dd::tables::Column_statistics::FIELD_SCHEMA_NAME,
                  m_schema_name) ||
         r->store(dd::tables::Column_statistics::FIELD_TABLE_NAME,
                  m_table_name) ||
         r->store(dd::tables::Column_statistics::FIELD_COLUMN_NAME,
                  m_column_name) ||
         r->store_json(dd::tables::Column_statistics::FIELD_HISTOGRAM, wrapper);
}

///////////////////////////////////////////////////////////////////////////

void Column_statistics_impl::serialize(Sdi_wcontext *wctx,
                                       Sdi_writer *w) const {
  /*
    We only write metadata about column statistics, which includes:
    - schema name
    - table name
    - column name
    - number of buckets originally specified by the user
  */
  w->StartObject();
  Entity_object_impl::serialize(wctx, w);
  write(w, m_schema_name, STRING_WITH_LEN("schema_name"));
  write(w, m_table_name, STRING_WITH_LEN("table_name"));
  write(w, m_column_name, STRING_WITH_LEN("column_name"));
  write(w, m_histogram->get_num_buckets_specified(),
        STRING_WITH_LEN("number_of_buckets_specified"));
  w->EndObject();
}

///////////////////////////////////////////////////////////////////////////

bool Column_statistics_impl::deserialize(Sdi_rcontext *rctx,
                                         const RJ_Value &val) {
  Entity_object_impl::deserialize(rctx, val);
  read(&m_schema_name, val, "schema_name");
  read(&m_table_name, val, "table_name");
  read(&m_column_name, val, "column_name");

  /*
    TODO: Re-create histogram data. This will be done in a later worklog, when
    we actually need to use the histogram data.
  */
  return false;
}

///////////////////////////////////////////////////////////////////////////
bool Column_statistics::update_id_key(Id_key *key, Object_id id) {
  key->update(id);
  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Column_statistics::update_name_key(Name_key *key,
                                        const String_type &name) {
  return dd::tables::Column_statistics::update_object_key(
      key, Dictionary_impl::instance()->default_catalog_id(), name);
}

///////////////////////////////////////////////////////////////////////////

const Object_table &Column_statistics_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Column_statistics_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<dd::tables::Column_statistics>();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
