/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/types/check_constraint_impl.h"

#include "mysqld_error.h"                          // ER_*
#include "sql/dd/impl/raw/raw_record.h"            // Raw_record
#include "sql/dd/impl/sdi_impl.h"                  // sdi read/write functions
#include "sql/dd/impl/tables/check_constraints.h"  // Check_constraints
#include "sql/dd/impl/transaction_impl.h"          // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/table_impl.h"          // Table_impl
#include "sql/error_handler.h"                     // Internal_error_handler

namespace dd {
class Object_key;
}  // namespace dd

using dd::tables::Check_constraints;

namespace dd {

///////////////////////////////////////////////////////////////////////////
// Check_constraint_impl implementation.
///////////////////////////////////////////////////////////////////////////

Check_constraint_impl::Check_constraint_impl() {}

Check_constraint_impl::Check_constraint_impl(Table_impl *table)
    : m_table(table) {}

///////////////////////////////////////////////////////////////////////////

const Table &Check_constraint_impl::table() const { return *m_table; }

Table &Check_constraint_impl::table() { return *m_table; }

///////////////////////////////////////////////////////////////////////////

bool Check_constraint_impl::validate() const {
  if (!m_table) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "No table is associated with this check constraint object.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

/**
  Error handler to convert ER_DUP_ENTRY error to more user friendly error
  ER_CHECK_CONSTRAINT_DUP_NAME.
*/
class Check_constraint_name_error_handler : public Internal_error_handler {
 public:
  Check_constraint_name_error_handler(const char *name) : m_name(name) {}

  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (sql_errno == ER_DUP_ENTRY) {
      my_error(ER_CHECK_CONSTRAINT_DUP_NAME, MYF(0), m_name);
      return true;
    }
    return false;
  }

 private:
  const char *m_name;
};

bool Check_constraint_impl::store(Open_dictionary_tables_ctx *otx) {
  /*
    Translate ER_DUP_ENTRY errors to the more user-friendly
    ER_CHECK_CONSTRAINT_DUP_NAME error.
  */
  Check_constraint_name_error_handler error_handler(name().c_str());
  otx->get_thd()->push_internal_handler(&error_handler);
  bool error = Weak_object_impl::store(otx);
  otx->get_thd()->pop_internal_handler();
  return error;
}

///////////////////////////////////////////////////////////////////////////

bool Check_constraint_impl::restore_attributes(const Raw_record &r) {
  if (check_parent_consistency(
          m_table, r.read_ref_id(Check_constraints::FIELD_TABLE_ID)))
    return true;

  restore_id(r, Check_constraints::FIELD_ID);
  restore_name(r, Check_constraints::FIELD_NAME);
  m_schema_id = r.read_int(Check_constraints::FIELD_SCHEMA_ID);
  m_constraint_state = static_cast<enum_constraint_state>(
      r.read_int(Check_constraints::FIELD_ENFORCED));
  m_check_clause = r.read_str(Check_constraints::FIELD_CHECK_CLAUSE);
  m_check_clause_utf8 = r.read_str(Check_constraints::FIELD_CHECK_CLAUSE_UTF8);

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Check_constraint_impl::store_attributes(dd::Raw_record *r) {
  return store_id(r, Check_constraints::FIELD_ID) ||
         (is_alter_mode()  // Store alias name in alter mode.
              ? r->store(Check_constraints::FIELD_NAME, m_alias_name)
              : store_name(r, Check_constraints::FIELD_NAME)) ||
         r->store(Check_constraints::FIELD_SCHEMA_ID, m_table->schema_id()) ||
         r->store(Check_constraints::FIELD_TABLE_ID, m_table->id()) ||
         r->store(Check_constraints::FIELD_ENFORCED, m_constraint_state) ||
         r->store(Check_constraints::FIELD_CHECK_CLAUSE, m_check_clause) ||
         r->store(Check_constraints::FIELD_CHECK_CLAUSE_UTF8,
                  m_check_clause_utf8);
}

///////////////////////////////////////////////////////////////////////////

static_assert(Check_constraints::FIELD_CHECK_CLAUSE_UTF8 == 6,
              "Check_constraints definition has changed, review (de)serialize "
              "member functions!");
void Check_constraint_impl::serialize(Sdi_wcontext *wctx, Sdi_writer *w) const {
  w->StartObject();
  Entity_object_impl::serialize(wctx, w);

  write_enum(w, m_constraint_state, STRING_WITH_LEN("state"));
  write_binary(wctx, w, m_check_clause, STRING_WITH_LEN("check_clause"));
  write(w, m_check_clause_utf8, STRING_WITH_LEN("check_clause_utf8"));

  w->EndObject();
}

///////////////////////////////////////////////////////////////////////////

bool Check_constraint_impl::deserialize(Sdi_rcontext *rctx,
                                        const RJ_Value &val) {
  Entity_object_impl::deserialize(rctx, val);

  read_enum(&m_constraint_state, val, "state");
  read_binary(rctx, &m_check_clause, val, "check_clause");
  read(&m_check_clause_utf8, val, "check_clause_utf8");
  return false;
}

///////////////////////////////////////////////////////////////////////////

void Check_constraint_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "CHECK CONSTRAINT OBJECT: { "
     << "m_id: {OID: " << id() << "}; "
     << "m_schema_id: {OID: " << m_table->schema_id() << "}; "
     << "m_table: {OID: " << m_table->id() << "}; "
     << "m_name: " << name() << ";"
     << "m_constraint_state: " << m_constraint_state << "; "
     << "m_check_clause: <excluded from output>; "
     << "m_check_clause_utf8: " << m_check_clause_utf8 << "; "
     << "m_alter_mode: " << m_alter_mode << "; "
     << "m_alias_name: " << m_alias_name << "; "
     << " }";

  outb = ss.str();
}

///////////////////////////////////////////////////////////////////////////

Check_constraint_impl::Check_constraint_impl(const Check_constraint_impl &src,
                                             Table_impl *parent)
    : Weak_object(src),
      Entity_object_impl(src),
      m_constraint_state(src.m_constraint_state),
      m_check_clause(src.m_check_clause),
      m_check_clause_utf8(src.m_check_clause_utf8),
      m_alter_mode(src.m_alter_mode),
      m_alias_name(src.m_alias_name),
      m_schema_id(src.m_schema_id),
      m_table(parent) {}

///////////////////////////////////////////////////////////////////////////

const Object_table &Check_constraint_impl::object_table() const {
  return DD_table::instance();
}

///////////////////////////////////////////////////////////////////////////

void Check_constraint_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Check_constraints>();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
