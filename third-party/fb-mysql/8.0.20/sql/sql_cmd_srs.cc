// Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is also distributed with certain software (including
// but not limited to OpenSSL) that is licensed under separate terms,
// as designated in a particular file or component or in included license
// documentation.  The authors of MySQL hereby grant you an additional
// permission to link the program and your derivative works with the
// separately licensed software that they have included with MySQL.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.

/// @file
///
/// This file implements the CREATE/DROP SPATIAL REFERENCE SYSTEM statements.

#include "sql/sql_cmd_srs.h"

#include <string>

#include "scope_guard.h"         // create_scope_guard
#include "sql/auth/auth_acls.h"  // SUPER_ACL
#include "sql/binlog.h"          // mysql_bin_log
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/dd.h"  // dd::create_object
#include "sql/dd/impl/types/spatial_reference_system_impl.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/derror.h"           // ER_THD
#include "sql/gis/srid.h"         // gis::srid_t
#include "sql/lock.h"             // acquire_shared_global...
#include "sql/sql_backup_lock.h"  // acquire_shared_backup_lock
#include "sql/sql_class.h"        // THD
#include "sql/sql_prepare.h"      // Ed_connection
#include "sql/srs_fetcher.h"
#include "sql/strfunc.h"
#include "sql/thd_raii.h"  // Disable_autocommit_guard
#include "sql/transaction.h"

/// Issue a warning if an SRID is within one of the reserved ranges.
///
/// @param[in] srid The SRID to check.
/// @param[in] thd Thread context.
static void warn_if_in_reserved_range(gis::srid_t srid, THD *thd) {
  gis::srid_t min = 0;
  gis::srid_t max = 0;

  if (srid <= 32767) {
    // Reserved by EPSG, cf. OGP Publication 373-7-1 Geomatics Guidance Note
    // number 7, part 1 - August 2012, Sect 5.9.
    min = 0;
    max = 32767;
  } else if (srid >= 60000000 && srid <= 69999999) {
    // Reserved by EPSG, cf. OGP Publication 373-7-1 Geomatics Guidance Note
    // number 7, part 1 - August 2012, Sect 5.9.
    min = 60000000;
    max = 69999999;
  } else if (srid >= 2000000000 && srid <= 2147483647) {
    // Reserved by MySQL.
    min = 2000000000;
    max = 2147483647;
  }

  if (!(min == 0 && max == 0)) {
    push_warning_printf(thd, Sql_condition::SL_WARNING,
                        ER_WARN_RESERVED_SRID_RANGE,
                        ER_THD(thd, ER_WARN_RESERVED_SRID_RANGE), min, max);
  }
}

/// Check if an SRS is used, i.e., if any columns depend on it.
///
/// @param[in] srid The SRID.
/// @param[in] thd Thread context.
///
/// @retval true The SRS is used by at least one column.
/// @retval false The SRS is not used by any columns.
static bool srs_is_used(gis::srid_t srid, THD *thd) {
  // We can't drop an SRS if it is used by a column (i.e., the column type has
  // an SRID type modifier referencing this SRS). Ideally, we'd like a foreign
  // key constraint to enforce this, but while waiting for the data dictionary
  // to enforce those, we have to do a workaround.
  //
  // Ed_connection doesn't handle statements that return a result, so instead of
  // selecting the list of columns using this SRS from the I_S views, we use a
  // DO statement that will fail with an error if the SRID is in use. Since it
  // is a DO statement, it will never return any result on success.
  Ed_connection conn(thd);
  std::string query(
      "DO (SELECT ST_GEOMETRYTYPE('invalid') FROM "
      "INFORMATION_SCHEMA.ST_GEOMETRY_COLUMNS WHERE SRS_ID=");
  query.append(std::to_string(srid));
  query.append(");");
  MYSQL_LEX_STRING query_string;
  lex_string_strmake(thd->mem_root, &query_string, query.c_str(),
                     query.length());
  return conn.execute_direct(query_string);
}

bool Sql_cmd_create_srs::fill_srs(dd::Spatial_reference_system *srs) {
  DBUG_ASSERT(m_srs_name.str != nullptr);
  srs->set_name(m_srs_name.str);

  if (m_organization.str != nullptr) {
    srs->set_organization(m_organization.str);
    srs->set_organization_coordsys_id(m_organization_coordsys_id);
  } else {
    srs->set_organization(nullptr);
    srs->set_organization_coordsys_id(nullptr);
  }

  if (m_description.str != nullptr)
    srs->set_description(m_description.str);
  else
    srs->set_description(nullptr);

  DBUG_ASSERT(m_definition.str != nullptr);
  srs->set_definition(m_definition.str);

  return static_cast<dd::Spatial_reference_system_impl *>(srs)
      ->parse_definition();
}

/// Abort the current statement and transaction.
///
/// @param[in] thd Thread context.
///
/// @retval false Success.
/// @retval true An error occurred.
static bool rollback(THD *thd) {
  bool error = trans_rollback_stmt(thd);
  error |= trans_rollback(thd);
  return error;
}

/// Commit the current statement and transaction.
///
/// @param[in] thd Thread context.
///
/// @retval false Success.
/// @retval true An error occurred.
static bool commit(THD *thd) {
  if (mysql_bin_log.is_open() &&
      thd->binlog_query(THD::STMT_QUERY_TYPE, thd->query().str,
                        thd->query().length, true, false, false, 0)) {
    /* purecov: begin inspected */
    rollback(thd);
    return true;
    /* purecov: end */
  }

  return (trans_commit_stmt(thd) || trans_commit(thd));
}

bool Sql_cmd_create_srs::execute(THD *thd) {
  if (!(thd->security_context()->check_access(SUPER_ACL))) {
    my_error(ER_CMD_NEED_SUPER, MYF(0),
             m_or_replace ? "CREATE OR REPLACE SPATIAL REFERENCE SYSTEM"
                          : "CREATE SPATIAL REFERENCE SYSTEM");
    return true;
  }

  if (acquire_shared_global_read_lock_nsec(
          thd, thd->variables.lock_wait_timeout_nsec) ||
      acquire_shared_backup_lock_nsec(thd,
                                      thd->variables.lock_wait_timeout_nsec))
    return true;

  Disable_autocommit_guard dag(thd);
  dd::cache::Dictionary_client *dd_client = thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser(dd_client);
  auto rollback_guard = create_scope_guard([thd]() {
    if (rollback(thd)) DBUG_ASSERT(false); /* purecov: deadcode */
  });
  Srs_fetcher fetcher(thd);
  dd::Spatial_reference_system *srs = nullptr;
  if (fetcher.acquire_for_modification(m_srid, &srs))
    return true; /* purecov: inspected */

  if (srs != nullptr) {
    if (m_if_not_exists) {
      push_warning_printf(thd, Sql_condition::SL_WARNING,
                          ER_WARN_SRS_ID_ALREADY_EXISTS,
                          ER_THD(thd, ER_WARN_SRS_ID_ALREADY_EXISTS), m_srid);
      my_ok(thd);
      return false;
    }
    if (!m_or_replace) {
      my_error(ER_SRS_ID_ALREADY_EXISTS, MYF(0), m_srid);
      return true;
    }

    if (fill_srs(srs)) return true;  // Error has already been flagged.

    const dd::Spatial_reference_system *old_srs = nullptr;
    if (fetcher.acquire(m_srid, &old_srs)) return true; /* purecov: inspected */
    DBUG_ASSERT(old_srs != nullptr);
    if (srs_is_used(m_srid, thd) && !old_srs->can_be_modified_to(*srs)) {
      my_error(ER_CANT_MODIFY_SRS_USED_BY_COLUMN, MYF(0), m_srid);
      return true;
    }

    warn_if_in_reserved_range(m_srid, thd);

    if (dd_client->update(srs)) return true;  // Error has already been flagged.

    rollback_guard.commit();
    if (commit(thd)) return true; /* purecov: inspected */
    my_ok(thd);
    return false;
  }

  std::unique_ptr<dd::Spatial_reference_system> new_srs(
      dd::create_object<dd::Spatial_reference_system>());
  static_cast<dd::Spatial_reference_system_impl *>(new_srs.get())
      ->set_id(m_srid);
  if (fill_srs(new_srs.get())) return true;  // Error has already been flagged.

  warn_if_in_reserved_range(m_srid, thd);

  if (thd->dd_client()->store(new_srs.get())) return true;

  rollback_guard.commit();
  if (commit(thd)) return true; /* purecov: inspected */
  my_ok(thd);
  return false;
}

bool Sql_cmd_drop_srs::execute(THD *thd) {
  if (!(thd->security_context()->check_access(SUPER_ACL))) {
    my_error(ER_CMD_NEED_SUPER, MYF(0), "DROP SPATIAL REFERENCE SYSTEM");
    return true;
  }

  if (acquire_shared_global_read_lock_nsec(
          thd, thd->variables.lock_wait_timeout_nsec) ||
      acquire_shared_backup_lock_nsec(thd,
                                      thd->variables.lock_wait_timeout_nsec))
    return true;

  Disable_autocommit_guard dag(thd);
  dd::cache::Dictionary_client *dd_client = thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser(dd_client);
  auto rollback_guard = create_scope_guard([thd]() {
    if (rollback(thd)) DBUG_ASSERT(false); /* purecov: deadcode */
  });
  Srs_fetcher fetcher(thd);
  dd::Spatial_reference_system *srs = nullptr;
  if (fetcher.acquire_for_modification(m_srid, &srs))
    return true; /* purecov: inspected */

  if (srs == nullptr) {
    if (m_if_exists) {
      push_warning_printf(thd, Sql_condition::SL_WARNING, ER_WARN_SRS_NOT_FOUND,
                          ER_THD(thd, ER_SRS_NOT_FOUND), m_srid);
      my_ok(thd);
    } else {
      my_error(ER_SRS_NOT_FOUND, MYF(0), m_srid);
    }
    return !m_if_exists;
  }

  warn_if_in_reserved_range(m_srid, thd);

  if (srs_is_used(m_srid, thd)) {
    my_error(ER_CANT_MODIFY_SRS_USED_BY_COLUMN, MYF(0), m_srid);
    return true;
  }

  if (dd_client->drop(srs)) return true; /* purecov: inspected */
  rollback_guard.commit();
  if (commit(thd)) return true; /* purecov: inspected */
  my_ok(thd);
  return false;
}
