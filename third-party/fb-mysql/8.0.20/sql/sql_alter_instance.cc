/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_alter_instance.h" /* Alter_instance class */

#include <utility>

#include "lex_string.h"
#include "m_string.h"
#include "mutex_lock.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h" /* my_error */
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/derror.h"  /* ER_THD */
#include "sql/handler.h" /* ha_resolve_by_legacy_type */
#include "sql/mysqld.h"
#include "sql/rpl_log_encryption.h"
#include "sql/sql_backup_lock.h" /* acquire_shared_backup_lock */
#include "sql/sql_class.h"       /* THD */
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_plugin_ref.h"
#include "sql/sql_table.h" /* write_to_binlog */

/*
  @brief
  Log current command to binlog

  @returns false on success,
           true on error

  In case of failure, appropriate error is logged.
*/

bool Alter_instance::log_to_binlog() {
  bool res = false;
  if (!m_thd->lex->no_write_to_binlog)
    res =
        write_bin_log(m_thd, false, m_thd->query().str, m_thd->query().length);

  return res;
}

/*
  @brief
  Executes master key rotation by calling SE api.

  @returns false on success
           true on error

  In case of failure, appropriate error
  is logged by function.
*/

bool Rotate_innodb_master_key::execute() {
  const LEX_CSTRING storage_engine = {STRING_WITH_LEN("innodb")};
  plugin_ref se_plugin;
  handlerton *hton;

  Security_context *sctx = m_thd->security_context();
  if (!sctx->check_access(SUPER_ACL) &&
      !sctx->has_global_grant(STRING_WITH_LEN("ENCRYPTION_KEY_ADMIN")).first) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
             "SUPER or ENCRYPTION_KEY_ADMIN");
    return true;
  }

  if ((se_plugin = ha_resolve_by_name(m_thd, &storage_engine, false))) {
    hton = plugin_data<handlerton *>(se_plugin);
  } else {
    my_error(ER_MASTER_KEY_ROTATION_SE_UNAVAILABLE, MYF(0));
    return true;
  }

  if (!hton->rotate_encryption_master_key) {
    my_error(ER_MASTER_KEY_ROTATION_NOT_SUPPORTED_BY_SE, MYF(0));
    return true;
  }

  /*
    Acquire shared backup lock to block concurrent backup. Acquire exclusive
    backup lock to block any concurrent DDL. The fact that we acquire both
    these locks also ensures that concurrent KEY rotation requests are blocked.
  */
  if (acquire_exclusive_backup_lock_nsec(
          m_thd, m_thd->variables.lock_wait_timeout_nsec, true) ||
      acquire_shared_backup_lock_nsec(
          m_thd, m_thd->variables.lock_wait_timeout_nsec)) {
    // MDL subsystem has to set an error in Diagnostics Area
    DBUG_ASSERT(m_thd->get_stmt_da()->is_error());
    return true;
  }

  if (hton->rotate_encryption_master_key()) {
    /* SE should have raised error */
    DBUG_ASSERT(m_thd->get_stmt_da()->is_error());
    return true;
  }

  if (log_to_binlog()) {
    /*
      Though we failed to write to binlog,
      there is no way we can undo this operation.
      So, covert error to a warning and let user
      know that something went wrong while trying
      to make entry in binlog.
    */
    m_thd->clear_error();
    m_thd->get_stmt_da()->reset_diagnostics_area();

    push_warning(m_thd, Sql_condition::SL_WARNING,
                 ER_MASTER_KEY_ROTATION_BINLOG_FAILED,
                 ER_THD(m_thd, ER_MASTER_KEY_ROTATION_BINLOG_FAILED));
  }

  my_ok(m_thd);
  return false;
}

bool Rotate_binlog_master_key::execute() {
  DBUG_TRACE;

  MUTEX_LOCK(lock, &LOCK_rotate_binlog_master_key);

  Security_context *sctx = m_thd->security_context();
  if (!sctx->check_access(SUPER_ACL) &&
      !sctx->has_global_grant(STRING_WITH_LEN("BINLOG_ENCRYPTION_ADMIN"))
           .first) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
             "SUPER or BINLOG_ENCRYPTION_ADMIN");
    return true;
  }

  if (!rpl_encryption.is_enabled()) {
    my_error(ER_RPL_ENCRYPTION_CANNOT_ROTATE_BINLOG_MASTER_KEY, MYF(0));
    return true;
  }

  if (rpl_encryption.remove_remaining_seqnos_from_keyring()) return true;

  if (rpl_encryption.rotate_master_key()) return true;

  my_ok(m_thd);
  return false;
}
