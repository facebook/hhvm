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

#include "sql/dd/impl/types/foreign_key_impl.h"

#include <stddef.h>
#include <sstream>
#include <string>

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "m_string.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"                // ER_*
#include "sql/dd/impl/raw/raw_record.h"  // Raw_record
#include "sql/dd/impl/sdi_impl.h"        // sdi read/write functions
#include "sql/dd/impl/tables/foreign_key_column_usage.h"  // Foreign_key_column_usage
#include "sql/dd/impl/tables/foreign_keys.h"              // Foreign_keys
#include "sql/dd/impl/transaction_impl.h"  // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/foreign_key_element_impl.h"  // Foreign_key_element_impl
#include "sql/dd/impl/types/table_impl.h"                // Table_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/string_type.h"  // dd::String_type
#include "sql/dd/types/foreign_key_element.h"
#include "sql/dd/types/index.h"  // Index
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/weak_object.h"
#include "sql/error_handler.h"  // Internal_error_handler
#include "sql/sql_class.h"
#include "sql/sql_error.h"

using dd::tables::Foreign_key_column_usage;
using dd::tables::Foreign_keys;

namespace dd {

class Sdi_rcontext;
class Sdi_wcontext;

///////////////////////////////////////////////////////////////////////////
// Foreign_key_impl implementation.
///////////////////////////////////////////////////////////////////////////

Foreign_key_impl::Foreign_key_impl()
    : m_match_option(OPTION_NONE),
      m_update_rule(RULE_NO_ACTION),
      m_delete_rule(RULE_NO_ACTION),
      m_table(nullptr),
      m_elements() {}

Foreign_key_impl::Foreign_key_impl(Table_impl *table)
    : m_match_option(OPTION_NONE),
      m_update_rule(RULE_NO_ACTION),
      m_delete_rule(RULE_NO_ACTION),
      m_table(table),
      m_elements() {}

///////////////////////////////////////////////////////////////////////////

const Table &Foreign_key_impl::table() const { return *m_table; }

Table &Foreign_key_impl::table() { return *m_table; }

///////////////////////////////////////////////////////////////////////////

class Foreign_key_name_error_handler : public Internal_error_handler {
  const char *name;

 public:
  Foreign_key_name_error_handler(const char *name_arg) : name(name_arg) {}

  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (sql_errno == ER_DUP_ENTRY) {
      my_error(ER_FK_DUP_NAME, MYF(0), name);
      return true;
    }
    return false;
  }
};

bool Foreign_key_impl::store(Open_dictionary_tables_ctx *otx) {
  /*
    Translate ER_DUP_ENTRY errors to the more user-friendly ER_FK_DUP_NAME.
    We should not report ER_DUP_ENTRY in any other cases (that would be
    a code bug).
  */
  Foreign_key_name_error_handler error_handler(name().c_str());
  otx->get_thd()->push_internal_handler(&error_handler);
  bool error = Weak_object_impl::store(otx);
  otx->get_thd()->pop_internal_handler();
  return error;
}

///////////////////////////////////////////////////////////////////////////

bool Foreign_key_impl::validate() const {
  if (!m_table) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No table object associated with this foreign key.");
    return true;
  }

  if (m_referenced_table_catalog_name.empty()) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Referenced table catalog name is not set.");
    return true;
  }

  if (m_referenced_table_schema_name.empty()) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Referenced table schema name is not set.");
    return true;
  }

  if (m_referenced_table_name.empty()) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Referenced table name is not set.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Foreign_key_impl::restore_children(Open_dictionary_tables_ctx *otx) {
  return m_elements.restore_items(
      this, otx, otx->get_table<Foreign_key_element>(),
      Foreign_key_column_usage::create_key_by_foreign_key_id(this->id()));
}

///////////////////////////////////////////////////////////////////////////

bool Foreign_key_impl::store_children(Open_dictionary_tables_ctx *otx) {
  return m_elements.store_items(otx);
}

///////////////////////////////////////////////////////////////////////////

bool Foreign_key_impl::drop_children(Open_dictionary_tables_ctx *otx) const {
  return m_elements.drop_items(
      otx, otx->get_table<Foreign_key_element>(),
      Foreign_key_column_usage::create_key_by_foreign_key_id(this->id()));
}

///////////////////////////////////////////////////////////////////////////

bool Foreign_key_impl::restore_attributes(const Raw_record &r) {
  if (check_parent_consistency(m_table,
                               r.read_ref_id(Foreign_keys::FIELD_TABLE_ID)))
    return true;

  restore_id(r, Foreign_keys::FIELD_ID);
  restore_name(r, Foreign_keys::FIELD_NAME);

  m_unique_constraint_name =
      r.read_str(Foreign_keys::FIELD_UNIQUE_CONSTRAINT_NAME, "");

  m_match_option =
      (enum_match_option)r.read_int(Foreign_keys::FIELD_MATCH_OPTION);
  m_update_rule = (enum_rule)r.read_int(Foreign_keys::FIELD_UPDATE_RULE);
  m_delete_rule = (enum_rule)r.read_int(Foreign_keys::FIELD_DELETE_RULE);

  m_referenced_table_catalog_name =
      r.read_str(Foreign_keys::FIELD_REFERENCED_TABLE_CATALOG);
  m_referenced_table_schema_name =
      r.read_str(Foreign_keys::FIELD_REFERENCED_TABLE_SCHEMA);
  m_referenced_table_name = r.read_str(Foreign_keys::FIELD_REFERENCED_TABLE);

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Foreign_key_impl::store_attributes(Raw_record *r) {
  return store_id(r, Foreign_keys::FIELD_ID) ||
         r->store(Foreign_keys::FIELD_SCHEMA_ID, m_table->schema_id()) ||
         r->store(Foreign_keys::FIELD_TABLE_ID, m_table->id()) ||
         store_name(r, Foreign_keys::FIELD_NAME) ||
         r->store(Foreign_keys::FIELD_UNIQUE_CONSTRAINT_NAME,
                  m_unique_constraint_name, m_unique_constraint_name.empty()) ||
         r->store(Foreign_keys::FIELD_MATCH_OPTION, m_match_option) ||
         r->store(Foreign_keys::FIELD_UPDATE_RULE, m_update_rule) ||
         r->store(Foreign_keys::FIELD_DELETE_RULE, m_delete_rule) ||
         r->store(Foreign_keys::FIELD_REFERENCED_TABLE_CATALOG,
                  m_referenced_table_catalog_name) ||
         r->store(Foreign_keys::FIELD_REFERENCED_TABLE_SCHEMA,
                  m_referenced_table_schema_name) ||
         r->store(Foreign_keys::FIELD_REFERENCED_TABLE,
                  m_referenced_table_name);
}

///////////////////////////////////////////////////////////////////////////

static_assert(Foreign_keys::FIELD_REFERENCED_TABLE == 10,
              "Foreign_keys definition has changed. Check (de)ser memfuns!");
void Foreign_key_impl::serialize(Sdi_wcontext *wctx, Sdi_writer *w) const {
  w->StartObject();
  Entity_object_impl::serialize(wctx, w);

  write_enum(w, m_match_option, STRING_WITH_LEN("match_option"));
  write_enum(w, m_update_rule, STRING_WITH_LEN("update_rule"));
  write_enum(w, m_delete_rule, STRING_WITH_LEN("delete_rule"));

  write(w, m_unique_constraint_name, STRING_WITH_LEN("unique_constraint_name"));

  write(w, m_referenced_table_catalog_name,
        STRING_WITH_LEN("referenced_table_catalog_name"));

  write(w, m_referenced_table_schema_name,
        STRING_WITH_LEN("referenced_table_schema_name"));

  write(w, m_referenced_table_name, STRING_WITH_LEN("referenced_table_name"));

  serialize_each(wctx, w, m_elements, STRING_WITH_LEN("elements"));
  w->EndObject();
}

///////////////////////////////////////////////////////////////////////////

bool Foreign_key_impl::deserialize(Sdi_rcontext *rctx, const RJ_Value &val) {
  Entity_object_impl::deserialize(rctx, val);
  read_enum(&m_match_option, val, "match_option");
  read_enum(&m_update_rule, val, "update_rule");
  read_enum(&m_delete_rule, val, "delete_rule");

  read(&m_unique_constraint_name, val, "unique_constraint_name");

  read(&m_referenced_table_catalog_name, val, "referenced_table_catalog_name");
  read(&m_referenced_table_schema_name, val, "referenced_table_schema_name");
  read(&m_referenced_table_name, val, "referenced_table_name");
  deserialize_each(
      rctx, [this]() { return add_element(); }, val, "elements");
  return false;
}

///////////////////////////////////////////////////////////////////////////

/* purecov: begin inspected */
void Foreign_key_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "FOREIGN_KEY OBJECT: { "
     << "m_id: {OID: " << id() << "}; "
     << "m_name: " << name() << "; "
     << "m_unique_constraint_name: " << m_unique_constraint_name << "; "
     << "m_match_option: " << m_match_option << "; "
     << "m_update_rule: " << m_update_rule << "; "
     << "m_delete_rule: " << m_delete_rule << "; ";

  {
    for (const Foreign_key_element *e : elements()) {
      String_type ob;
      e->debug_print(ob);
      ss << ob;
    }
  }

  ss << " }";

  outb = ss.str();
}
/* purecov: end */

/////////////////////////////////////////////////////////////////////////

Foreign_key_element *Foreign_key_impl::add_element() {
  Foreign_key_element_impl *e =
      new (std::nothrow) Foreign_key_element_impl(this);
  m_elements.push_back(e);
  return e;
}

/////////////////////////////////////////////////////////////////////////

Foreign_key_impl::Foreign_key_impl(const Foreign_key_impl &src,
                                   Table_impl *parent)
    : Weak_object(src),
      Entity_object_impl(src),
      m_match_option(src.m_match_option),
      m_update_rule(src.m_update_rule),
      m_delete_rule(src.m_delete_rule),
      m_unique_constraint_name(src.m_unique_constraint_name),
      m_referenced_table_catalog_name(src.m_referenced_table_catalog_name),
      m_referenced_table_schema_name(src.m_referenced_table_schema_name),
      m_referenced_table_name(src.m_referenced_table_name),
      m_table(parent),
      m_elements() {
  m_elements.deep_copy(src.m_elements, this);
}

///////////////////////////////////////////////////////////////////////////

const Object_table &Foreign_key_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Foreign_key_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Foreign_keys>();

  otx->register_tables<Foreign_key_element>();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
