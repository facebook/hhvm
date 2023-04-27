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

#include "sql/dd/impl/tables/dd_properties.h"

#include <string>

#include "m_ctype.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/udf_registration_types.h"
#include "mysqld_error.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/dd/dd_version.h"  // dd::DD_VERSION
#include "sql/dd/impl/raw/raw_table.h"
#include "sql/dd/impl/transaction_impl.h"
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/dd/properties.h"
#include "sql/dd/string_type.h"  // dd::String_type, dd::Stringstream_type
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/sql_const.h"
#include "sql/stateless_allocator.h"
#include "sql/table.h"
#include "sql_string.h"

namespace dd {
namespace tables {

DD_properties &DD_properties::instance() {
  static DD_properties *s_instance = new (std::nothrow) DD_properties();
  return *s_instance;
}

// Setup the initial definition of mysql.dd_properties table.
DD_properties::DD_properties() : m_properties() {
  m_target_def.set_table_name("dd_properties");

  m_target_def.add_field(FIELD_PROPERTIES, "FIELD_PROPERTIES",
                         "properties MEDIUMBLOB");

  m_target_def.add_populate_statement(
      "INSERT INTO dd_properties (properties)"
      "VALUES ('DD_VERSION=0')");

  /*
    Initialize the descriptors of the valid keys. The keys are used for
    the following purposes:

      DD_VERSION                Actual DD version.
      IS_VERSION                Actual I_S version.
      PS_VERSION                Actual P_S version.
      NDBINFO_VERSION           Actual ndbinfo version.
      SDI_VERSION               Actual SDI version.
      LCTN                      L_C_T_N setting used during
                                --initialize.
      MYSQLD_VERSION_LO         Lowest server version which has
                                been using the data directory.
      MYSQLD_VERSION_HI         Highest server version which has
                                been using the data directory.
      MYSQLD_VERSION            Current server version.
      MINOR_DOWNGRADE_THRESHOLD The current DD can be used by
                                previous MRUs, unless their
                                target DD version is less than
                                the downgrade threshold.
      SYSTEM_TABLES             List of system tables with
                                definitions.
      UPGRADE_TARGET_SCHEMA     Temporary schema used during
                                upgrade.
      UPGRADE_ACTUAL_SCHEMA     Temporary schema used during
                                upgrade.
      MYSQLD_VERSION_UPGRADED   The server version of the last
                                completed successful upgrade.
  */
  m_property_desc = {
      {"DD_VERSION", Property_type::UNSIGNED_INT_32},
      {"IS_VERSION", Property_type::UNSIGNED_INT_32},
      {"PS_VERSION", Property_type::UNSIGNED_INT_32},
      {"NDBINFO_VERSION", Property_type::UNSIGNED_INT_32},
      {"SDI_VERSION", Property_type::UNSIGNED_INT_32},
      {"LCTN", Property_type::UNSIGNED_INT_32},
      {"MYSQLD_VERSION_LO", Property_type::UNSIGNED_INT_32},
      {"MYSQLD_VERSION_HI", Property_type::UNSIGNED_INT_32},
      {"MYSQLD_VERSION", Property_type::UNSIGNED_INT_32},
      {"MINOR_DOWNGRADE_THRESHOLD", Property_type::UNSIGNED_INT_32},
      {"SYSTEM_TABLES", Property_type::PROPERTIES},
      {"UPGRADE_TARGET_SCHEMA", Property_type::CHARACTER_STRING},
      {"UPGRADE_ACTUAL_SCHEMA", Property_type::CHARACTER_STRING},
      {"MYSQLD_VERSION_UPGRADED", Property_type::UNSIGNED_INT_32}};
}

// Read all properties from disk and populate the cache.
bool DD_properties::init_cached_properties(THD *thd) {
  // Early exit in case the properties are already initialized.
  if (!m_properties.empty()) return false;

  /*
    Start a DD transaction to get the properties. Please note that we
    must do this read using isolation level ISO_READ_UNCOMMITTED
    because the SE undo logs may not yet be available.
  */
  Transaction_ro trx(thd, ISO_READ_UNCOMMITTED);
  trx.otx.add_table<DD_properties>();

  if (trx.otx.open_tables()) {
    DBUG_ASSERT(false);
    return true;
  }

  Raw_table *raw_t = trx.otx.get_table(name());
  DBUG_ASSERT(raw_t);
  TABLE *t = raw_t->get_table();
  DBUG_ASSERT(t);
  t->use_all_columns();

  /*
    We should not read from this table until after it has been populated,
    so there should always be a row stored. We read the row, and populate
    the property cache based on its contents.
  */
  if (t->file->ha_rnd_init(true) || t->file->ha_rnd_next(t->record[0])) {
    DBUG_ASSERT(false);
    t->file->ha_rnd_end();
    return true;
  }

  String val;
  t->field[FIELD_PROPERTIES]->val_str(&val);
  m_properties.insert_values(val.c_ptr_safe());

  t->file->ha_rnd_end();
  return (m_properties.empty());
}

// Flush all properties from the cache to disk.
bool DD_properties::flush_cached_properties(THD *thd) {
  DBUG_ASSERT(!m_properties.empty());

  Update_dictionary_tables_ctx ctx(thd);
  ctx.otx.add_table<DD_properties>();

  if (ctx.otx.open_tables()) return true;

  Raw_table *raw_t = ctx.otx.get_table(name());
  DBUG_ASSERT(raw_t);
  TABLE *t = raw_t->get_table();
  DBUG_ASSERT(t);
  t->use_all_columns();
  bitmap_set_all(t->write_set);
  bitmap_set_all(t->read_set);

  int rc = 0;
  if ((rc = t->file->ha_rnd_init(true))) {
    t->file->print_error(rc, MYF(0));
    t->file->ha_rnd_end();
    return true;
  }

  /*
    If a row is already stored, then we update it. If no row is stored,
    it means we an error situation, since we should not write to the table
    until after its populate SQL statement has been executed.
  */
  if ((rc = t->file->ha_rnd_next(t->record[0]))) {
    DBUG_ASSERT(false);
    t->file->ha_rnd_end();
    return true;
  }

  store_record(t, record[1]);
  const String_type &prop_str = m_properties.raw_string();
  t->field[FIELD_PROPERTIES]->store(prop_str.c_str(), prop_str.length(),
                                    system_charset_info);
  rc = t->file->ha_update_row(t->record[1], t->record[0]);
  t->file->ha_rnd_end();

  if (rc && rc != HA_ERR_RECORD_IS_THE_SAME) {
    t->file->print_error(rc, MYF(0));
    return true;
  }

  return false;
}

/*
  Initialize the cache, and read the property value for the given key
  without checking or validating the key.
*/
bool DD_properties::unchecked_get(THD *thd, const String_type &key,
                                  String_type *value, bool *exists) {
  if (init_cached_properties(thd)) return true;

  *exists = m_properties.exists(key);
  if (*exists) m_properties.get(key, value);

  if (*exists == false && key == "DD_VERSION") {
    *exists = m_properties.exists("DD_version");
    if (*exists) m_properties.get("DD_version", value);
  }

  return false;
}

/*
  Initialize the cache, and set the property value for the given key
  without checking or validating the key. Flush the cache to disk in
  order to make the value persistent.
*/
bool DD_properties::unchecked_set(THD *thd, const String_type &key,
                                  const String_type &value) {
  // Read cached properties from disk, if not existing.
  if (init_cached_properties(thd)) return true;

  // Update the cached properties and the table.
  m_properties.set(key, value);
  return flush_cached_properties(thd);
}

// Read the integer property for the given key.
bool DD_properties::get(THD *thd, const String_type &key, uint *value,
                        bool *exists) {
  DBUG_ASSERT(m_property_desc.find(key) != m_property_desc.end() &&
              m_property_desc[key] == Property_type::UNSIGNED_INT_32);
  String_type val_str;
  return unchecked_get(thd, key, &val_str, exists) ||
         dd::Properties::from_str(val_str, value);
}

// Set the integer property for the given key.
bool DD_properties::set(THD *thd, const String_type &key, uint value) {
  DBUG_ASSERT(m_property_desc.find(key) != m_property_desc.end() &&
              m_property_desc[key] == Property_type::UNSIGNED_INT_32);
  return unchecked_set(thd, key, dd::Properties::to_str(value));
}

// Read the character string property for the given key.
bool DD_properties::get(THD *thd, const String_type &key, String_type *value,
                        bool *exists) {
  DBUG_ASSERT(m_property_desc.find(key) != m_property_desc.end() &&
              m_property_desc[key] == Property_type::CHARACTER_STRING);
  return unchecked_get(thd, key, value, exists);
}

// Set the character string property for the given key.
bool DD_properties::set(THD *thd, const String_type &key,
                        const String_type &value) {
  DBUG_ASSERT(m_property_desc.find(key) != m_property_desc.end() &&
              m_property_desc[key] == Property_type::CHARACTER_STRING);
  return unchecked_set(thd, key, value);
}

// Read the properties object for the given key.
bool DD_properties::get(THD *thd, const String_type &key,
                        std::unique_ptr<Properties> *properties, bool *exists) {
  DBUG_ASSERT(m_property_desc.find(key) != m_property_desc.end() &&
              m_property_desc[key] == Property_type::PROPERTIES);

  String_type property_string;
  if (unchecked_get(thd, key, &property_string, exists)) return true;

  properties->reset(Properties::parse_properties(property_string.c_str()));
  return false;
}

// Set the property object for the given key.
bool DD_properties::set(THD *thd, const String_type &key,
                        const dd::Properties &properties) {
  DBUG_ASSERT(m_property_desc.find(key) != m_property_desc.end() &&
              m_property_desc[key] == Property_type::PROPERTIES);
  return unchecked_set(thd, key, properties.raw_string());
}

// Initialize the cache, and remove the submitted key if it exists.
bool DD_properties::remove(THD *thd, const String_type &key) {
  // Read cached properties from disk, if not existing.
  if (init_cached_properties(thd)) return true;

  // Update the cached properties and the table.
  if (m_properties.exists(key)) (void)m_properties.remove(key);
  return flush_cached_properties(thd);
}

}  // namespace tables
}  // namespace dd
