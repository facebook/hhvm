/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/sdi_api.h"

#include <sys/types.h>
#include <algorithm>
#include <type_traits>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/psi/mysql_file.h"  // mysql_file_x
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"            // CREATE_ACL
#include "sql/dd/cache/dictionary_client.h"  // dd::Dictionary_client
#include "sql/dd/dd.h"
#include "sql/dd/dd_table.h"        // has_primary_key()
#include "sql/dd/impl/sdi.h"        // dd::deserialize
#include "sql/dd/impl/sdi_utils.h"  // dd::sdi_utils::handle_errors
#include "sql/dd/impl/types/object_table_definition_impl.h"  // Object_table_definition::fs_collation()
#include "sql/dd/sdi_file.h"      // dd::sdi_file::load
#include "sql/dd/types/schema.h"  // dd::Schema
#include "sql/dd/types/table.h"   // dd::Table
#include "sql/dd_sql_view.h"      // update_referencing_views_metadata()
#include "sql/mdl.h"              // MDL_request
#include "sql/mysqld.h"           // lower_case_table_names
#include "sql/sql_base.h"         // open_tables()
#include "sql/sql_class.h"        // THD
#include "sql/sql_error.h"
#include "sql/sql_servers.h"
#include "sql/strfunc.h"  // casedn
#include "sql/table.h"    // TABLE_LIST
#include "thr_lock.h"

namespace dd {
namespace sdi {
Import_target::Import_target(String_type &&path, bool in_datadir)
    : m_path{std::move(path)},
      m_in_datadir{in_datadir},
      m_table_object{dd::create_object<dd::Table>()} {}

bool Import_target::commit() const {
  DBUG_EXECUTE_IF(
      "sdi_import_commit_fail",
      return (m_in_datadir &&
              mysql_file_delete(key_file_sdi, "no_such_file", MYF(MY_FAE))););
  return (
      m_in_datadir &&
      mysql_file_delete(key_file_sdi, m_tmp_sdi_filename.c_str(), MYF(MY_FAE)));
}

bool Import_target::rollback() const {
  return (m_in_datadir && !m_tmp_sdi_filename.empty() &&
          mysql_file_rename(key_file_sdi, m_tmp_sdi_filename.c_str(),
                            m_path.c_str(), MYF(MY_FAE)));
}

const dd::String_type *Import_target::can_table_name() const {
  return m_lc_tname ? m_lc_tname.get() : &m_table_object->name();
}

const dd::String_type *Import_target::can_schema_name() const {
  return m_lc_sname ? m_lc_sname.get() : &m_schema_name_in_sdi;
}

bool Import_target::load(THD *thd, String_type *shared_buffer) {
  if (dd::sdi_file::load(thd, m_path, shared_buffer)) {
    return true;
  }

  if (dd::deserialize(thd, *shared_buffer, m_table_object.get(),
                      &m_schema_name_in_sdi)) {
    return true;
  }

  if (!dd::has_primary_key(*m_table_object) &&
      thd->variables.sql_require_primary_key) {
    my_error(ER_TABLE_WITHOUT_PK, MYF(0));
    return true;
  }

  const CHARSET_INFO *dd_charset_info =
      Object_table_definition_impl::fs_name_collation();
  if (lower_case_table_names == 1) {
    m_table_object->set_name(casedn(dd_charset_info, m_table_object->name()));
    m_schema_name_in_sdi =
        casedn(dd_charset_info, std::move(m_schema_name_in_sdi));
  }

  if (lower_case_table_names == 2) {
    m_lc_tname.reset(
        new dd::String_type(casedn(dd_charset_info, m_table_object->name())));
    m_lc_sname.reset(
        new dd::String_type(casedn(dd_charset_info, m_schema_name_in_sdi)));
  }

  if (!m_in_datadir) {
    return false;
  }

  dd::String_type tmpname{m_path};
  tmpname.insert(tmpname.length() - 4, "_import");
  if (mysql_file_rename(key_file_sdi, m_path.c_str(), tmpname.c_str(),
                        MYF(MY_FAE))) {
    return true;
  }
  m_tmp_sdi_filename = std::move(tmpname);

  return false;
}

TABLE_LIST Import_target::make_table_list() const {
  return TABLE_LIST(can_schema_name()->c_str(),  // schema_name, with case
                    can_schema_name()->length(),
                    can_table_name()->c_str(),  // table_name, with case
                    can_table_name()->length(),
                    m_table_object->name().c_str(),  // alias, lower_cased
                    TL_IGNORE);
}

bool Import_target::store_in_dd(THD *thd) const {
  dd::cache::Dictionary_client &dc = *thd->dd_client();
  const Schema *schema = nullptr;

  if (dc.acquire(m_schema_name_in_sdi, &schema)) {
    return true;
  }
  if (schema == nullptr) {
    my_error(ER_IMP_SCHEMA_DOES_NOT_EXIST, MYF(0),
             m_schema_name_in_sdi.c_str());
    return true;
  }
  // Since deserialization does no longer does this automatically, we
  // must do it here to make the table object valid.
  m_table_object->set_schema_id(schema->id());

  const Table *existing = nullptr;
  if (dc.acquire(schema->name(), m_table_object->name(), &existing)) {
    return true;
  }

  if (existing != nullptr) {
    my_error(ER_IMP_TABLE_ALREADY_EXISTS, MYF(0), schema->name().c_str(),
             m_table_object->name().c_str());
    return true;
  }

  if (dd::sdi_file::check_data_files_exist(m_schema_name_in_sdi,
                                           m_table_object->name())) {
    return true;
  }

  if (dc.store(m_table_object.get())) {
    return true;
  }

  TABLE_LIST tl = make_table_list();

  //   tl.init_one_table(schema->name().c_str(),
  //                     schema->name().length(),
  //                     m_table_object->name().c_str(),
  //                     m_table_object->name().length(),
  //                     m_table_object->name().c_str(),
  //                     TL_IGNORE);

  Uncommitted_tables_guard uncommitted_tables(thd);
  if (update_referencing_views_metadata(thd, &tl, false, &uncommitted_tables)) {
    return true;
  }

  TABLE_LIST *tlp = &tl;
  uint dummy = 0;
  // Downgrade all errors from open to warnings as we don't want to
  // fail import here, but rather give the user a chance to repair the
  // table.
  dd::sdi_utils::handle_errors(
      thd,
      [](uint, const char *, Sql_condition::enum_severity_level *level,
         const char *) {
        (*level) = Sql_condition::SL_WARNING;
        return false;
      },
      [&]() { return open_tables(thd, &tlp, &dummy, 0); });

  close_thread_tables(thd);

  return false;
}

bool check_privileges(THD *thd, const Import_target &t) {
  //   const char *table_name= t.can_table_name()->c_str();
  //   size_t table_len= t.can_table_name()->length();

  // const char *schema_name= t.can_schema_name()->c_str();
  // size_t schema_len= t.can_schema_name()->length();

  TABLE_LIST tl = t.make_table_list();

  //   tl.init_one_table(schema_name, schema_len, table_name, table_len,
  //                     table_name, TL_IGNORE);

  // Note! Passing the tl.grant.privilege and tl.grant.m_internal is NOT
  // optional here. The call to check_grant() below depends on info being
  // recorded there to correctly determine if we have CREATE privilege on
  // the table.
  bool create_access_denied =
      check_access(thd, CREATE_ACL, tl.db, &tl.grant.privilege,
                   &tl.grant.m_internal, false, false);
  if (create_access_denied) {
    return true;
  }

  bool create_grant_denied =
      check_grant(thd, CREATE_ACL, &tl, false /* any_combination_will_do */,
                  1 /* number */, false /* no_errors */);
  if (create_grant_denied) {
    return true;
  }

  return false;
}

MDL_request *mdl_request(const Import_target &t, MEM_ROOT *mem_root) {
  MDL_request *req = new (mem_root) MDL_request;
  MDL_REQUEST_INIT(req, MDL_key::TABLE, t.can_schema_name()->c_str(),
                   t.can_table_name()->c_str(), MDL_EXCLUSIVE, MDL_TRANSACTION);
  return req;
}
}  // namespace sdi
}  // namespace dd
