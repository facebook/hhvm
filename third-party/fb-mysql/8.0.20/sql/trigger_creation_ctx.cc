/*
   Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "sql/trigger_creation_ctx.h"

#include <stddef.h>
#include <atomic>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysqld_error.h"
#include "sql/derror.h"
#include "sql/log.h"
#include "sql/sql_class.h"
#include "sql/sql_db.h"  // get_default_db_collation()
#include "sql/sql_error.h"
#include "sql/system_variables.h"

Trigger_creation_ctx *Trigger_creation_ctx::create(
    THD *thd, const LEX_CSTRING &db_name, const LEX_CSTRING &table_name,
    const LEX_CSTRING &client_cs_name, const LEX_CSTRING &connection_cl_name,
    const LEX_CSTRING &db_cl_name) {
  const CHARSET_INFO *client_cs;
  const CHARSET_INFO *connection_cl;
  const CHARSET_INFO *db_cl = nullptr;

  bool invalid_creation_ctx = false;

  if (resolve_charset(client_cs_name.str, thd->variables.character_set_client,
                      &client_cs)) {
    LogErr(WARNING_LEVEL, ER_TRIGGER_INVALID_VALUE, (const char *)db_name.str,
           (const char *)table_name.str, "character_set_client",
           (const char *)client_cs_name.str);

    invalid_creation_ctx = true;
  }

  if (resolve_collation(connection_cl_name.str,
                        thd->variables.collation_connection, &connection_cl)) {
    LogErr(WARNING_LEVEL, ER_TRIGGER_INVALID_VALUE, (const char *)db_name.str,
           (const char *)table_name.str, "collation_connection",
           (const char *)connection_cl_name.str);

    invalid_creation_ctx = true;
  }

  if (resolve_collation(db_cl_name.str, nullptr, &db_cl)) {
    LogErr(WARNING_LEVEL, ER_TRIGGER_INVALID_VALUE, (const char *)db_name.str,
           (const char *)table_name.str, "database_collation",
           (const char *)db_cl_name.str);

    invalid_creation_ctx = true;
  }

  if (invalid_creation_ctx) {
    push_warning_printf(
        thd, Sql_condition::SL_WARNING, ER_TRG_INVALID_CREATION_CTX,
        ER_THD(thd, ER_TRG_INVALID_CREATION_CTX), (const char *)db_name.str,
        (const char *)table_name.str);
  }

  /*
    If we failed to resolve the database collation, load the default one
    from the disk.
  */

  if (db_cl == nullptr && get_default_db_collation(thd, db_name.str, &db_cl)) {
    DBUG_ASSERT(thd->is_error() || thd->killed);
    return nullptr;
  }

  db_cl = db_cl ? db_cl : thd->collation();

  return new (thd->mem_root)
      Trigger_creation_ctx(client_cs, connection_cl, db_cl);
}

Stored_program_creation_ctx *Trigger_creation_ctx::clone(MEM_ROOT *mem_root) {
  return new (mem_root)
      Trigger_creation_ctx(m_client_cs, m_connection_cl, m_db_cl);
}

Object_creation_ctx *Trigger_creation_ctx::create_backup_ctx(THD *thd) const {
  return new (thd->mem_root) Trigger_creation_ctx(thd);
}

void Trigger_creation_ctx::delete_backup_ctx() { destroy(this); }
