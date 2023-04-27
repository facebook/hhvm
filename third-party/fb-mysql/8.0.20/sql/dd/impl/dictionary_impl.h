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

#ifndef DD__DICTIONARY_IMPL_INCLUDED
#define DD__DICTIONARY_IMPL_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <string>

#include "lex_string.h"
#include "sql/dd/dictionary.h"   // dd::Dictionary
#include "sql/dd/object_id.h"    // dd::Object_id
#include "sql/dd/string_type.h"  // dd::String_type
#include "sql/table.h"           // MYSQL_SCHEMA_NAME

class THD;

namespace dd {
class Object_table;
}  // namespace dd

namespace dd_schema_unittest {
class SchemaTest;
}

namespace my_testing {
class DD_initializer;
}

namespace dd_column_statistics_unittest {
template <typename T>
class ColumnStatisticsTest;
}
namespace dd {

///////////////////////////////////////////////////////////////////////////

enum class enum_dd_init_type;

///////////////////////////////////////////////////////////////////////////

class Dictionary_impl : public Dictionary {
  friend class dd_schema_unittest::SchemaTest;
  friend class my_testing::DD_initializer;
  template <typename T>
  friend class dd_column_statistics_unittest::ColumnStatisticsTest;

  /////////////////////////////////////////////////////////////////////////
  // Implementation details.
  /////////////////////////////////////////////////////////////////////////

 private:
  static Dictionary_impl *s_instance;

 public:
  static bool init(enum_dd_init_type dd_init);
  static bool shutdown();

  static Dictionary_impl *instance();

 private:
  Dictionary_impl() {}

 public:
  virtual ~Dictionary_impl() {}

 public:
  static uint get_target_dd_version();

  virtual uint get_actual_dd_version(THD *thd);

  static uint get_target_I_S_version();

  virtual uint get_actual_I_S_version(THD *thd);

  static uint get_target_P_S_version();

  virtual uint get_actual_P_S_version(THD *thd);

  virtual bool get_actual_ndbinfo_schema_version(THD *thd, uint *version);

  uint set_I_S_version(THD *thd, uint version);

  uint set_P_S_version(THD *thd, uint version);

  uint set_ndbinfo_schema_version(THD *thd, uint version);

  virtual const Object_table *get_dd_table(const String_type &schema_name,
                                           const String_type &table_name) const;

 public:
  virtual bool is_dd_schema_name(const String_type &schema_name) const {
    return (schema_name == MYSQL_SCHEMA_NAME.str);
  }

  virtual bool is_dd_table_name(const String_type &schema_name,
                                const String_type &table_name) const;

  virtual bool is_system_table_name(const String_type &schema_name,
                                    const String_type &table_name) const;

  virtual int table_type_error_code(const String_type &schema_name,
                                    const String_type &table_name) const;

  virtual bool is_dd_table_access_allowed(bool is_dd_internal_thread,
                                          bool is_ddl_statement,
                                          const char *schema_name,
                                          size_t schema_length,
                                          const char *table_name) const;

  virtual bool is_system_view_name(const char *schema_name,
                                   const char *table_name, bool *hidden) const;

  virtual bool is_system_view_name(const char *schema_name,
                                   const char *table_name) const {
    bool hidden;
    return is_system_view_name(schema_name, table_name, &hidden);
  }

 public:
  static Object_id default_catalog_id() { return DEFAULT_CATALOG_ID; }

  static Object_id dd_tablespace_id() { return DD_TABLESPACE_ID; }

  static const String_type &default_catalog_name() {
    return DEFAULT_CATALOG_NAME;
  }

 private:
  static Object_id DEFAULT_CATALOG_ID;
  static Object_id DD_TABLESPACE_ID;
  static const String_type DEFAULT_CATALOG_NAME;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__DICTIONARY_IMPL_INCLUDED
