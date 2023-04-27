/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/upgrade_57/schema.h"

#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "lex_string.h"
#include "m_ctype.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/psi/mysql_file.h"  // mysql_file_open
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/dd/cache/dictionary_client.h"  // Auto_releaser
#include "sql/dd/dd_schema.h"                // Schema_MDL_locker
#include "sql/dd/impl/utils.h"               // is_string_in_lowercase
#include "sql/log.h"                         // LogErr()
#include "sql/mysqld.h"                      // key_file_dbopt
#include "sql/sql_class.h"                   // THD
#include "sql/sql_table.h"                   // build_tablename
#include "sql/system_variables.h"
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_commit
#include "sql_string.h"

namespace dd {
namespace upgrade_57 {

/**
  Returns the collation id for the database specified.

  @param[in]  thd                        Thread handle.
  @param[in]  db_opt_path                Path for database.
  @param[out] schema_charset             Character set of database.

  @retval false  ON SUCCESS
  @retval true   ON FAILURE

*/
static bool load_db_schema_collation(THD *thd, const LEX_STRING *db_opt_path,
                                     const CHARSET_INFO **schema_charset) {
  IO_CACHE cache;
  File file;
  char buf[256];
  uint nbytes;

  if ((file = mysql_file_open(key_file_dbopt, db_opt_path->str, O_RDONLY,
                              MYF(0))) < 0) {
    LogErr(WARNING_LEVEL, ER_CANT_OPEN_DB_OPT_USING_DEFAULT_CHARSET,
           db_opt_path->str);
    return false;
  }

  if (init_io_cache(&cache, file, IO_SIZE, READ_CACHE, 0, false, MYF(0))) {
    LogErr(ERROR_LEVEL, ER_CANT_CREATE_CACHE_FOR_DB_OPT, db_opt_path->str);
    goto err;
  }

  while ((int)(nbytes = my_b_gets(&cache, (char *)buf, sizeof(buf))) > 0) {
    char *pos = buf + nbytes - 1;

    /* Remove end space and control characters */
    while (pos > buf && !my_isgraph(&my_charset_latin1, pos[-1])) pos--;

    *pos = 0;
    if ((pos = strrchr(buf, '='))) {
      if (!strncmp(buf, "default-character-set", (pos - buf))) {
        /*
           Try character set name, and if it fails try collation name, probably
           it's an old 4.1.0 db.opt file, which didn't have separate
           default-character-set and default-collation commands.
        */
        if (!(*schema_charset =
                  get_charset_by_csname(pos + 1, MY_CS_PRIMARY, MYF(0))) &&
            !(*schema_charset = get_charset_by_name(pos + 1, MYF(0)))) {
          LogErr(WARNING_LEVEL, ER_CANT_IDENTIFY_CHARSET_USING_DEFAULT,
                 db_opt_path->str);

          *schema_charset = thd->variables.collation_server;
        }
      } else if (!strncmp(buf, "default-collation", (pos - buf))) {
        if (!(*schema_charset = get_charset_by_name(pos + 1, MYF(0)))) {
          LogErr(WARNING_LEVEL, ER_CANT_IDENTIFY_CHARSET_USING_DEFAULT,
                 db_opt_path->str);
          *schema_charset = thd->variables.collation_server;
        }
      }
    }
  }

  end_io_cache(&cache);
  mysql_file_close(file, MYF(0));
  return false;

err:
  mysql_file_close(file, MYF(0));
  return true;
}

/**
   Update the Schemata:DD for every database present
   in the data directory.
*/

bool migrate_schema_to_dd(THD *thd, const char *dbname) {
  char dbopt_path_buff[FN_REFLEN + 1];
  char schema_name[NAME_LEN + 1];
  LEX_STRING dbopt_file_name;
  const CHARSET_INFO *schema_charset = thd->variables.collation_server;
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  // Construct the schema name from its canonical format.
  filename_to_tablename(dbname, schema_name, sizeof(schema_name));

  dbopt_file_name.str = dbopt_path_buff;
  dbopt_file_name.length = build_table_filename(dbopt_path_buff, FN_REFLEN - 1,
                                                schema_name, "db", ".opt", 0);

  if (!my_access(dbopt_file_name.str, F_OK)) {
    // Get the collation id for the database.
    if (load_db_schema_collation(thd, &dbopt_file_name, &schema_charset))
      return true;
  } else {
    LogErr(WARNING_LEVEL, ER_DB_OPT_NOT_FOUND_USING_DEFAULT_CHARSET, dbname);
  }

  // See comments regarding l_c_t_n in migrate_all_frm_to_dd().
  if (lower_case_table_names == 0)
    // Supported only for case sensitive file systems.
    DBUG_ASSERT(!lower_case_file_system);
  else if (lower_case_table_names == 1) {
    // Supported for any file system. All names must be in lower case.
    if (!is_string_in_lowercase(schema_name, system_charset_info)) {
      LogErr(ERROR_LEVEL, ER_SCHEMA_NAME_IN_UPPER_CASE_NOT_ALLOWED,
             schema_name);
      return true;
    }
  } else if (lower_case_table_names == 2)
    // Supported only for case insensitive file systems.
    DBUG_ASSERT(lower_case_file_system);
  else
    DBUG_ASSERT(false);

  // Disable autocommit option
  Disable_autocommit_guard autocommit_guard(thd);

  if (dd::create_schema(thd, schema_name, schema_charset, false, nullptr)) {
    trans_rollback_stmt(thd);
    // Full rollback in case we have THD::transaction_rollback_request.
    trans_rollback(thd);
    return true;
  }

  if (trans_commit_stmt(thd) || trans_commit(thd)) return true;

  return false;
}

/**
  Scans datadir for databases and lists all the database names.
*/

bool find_schema_from_datadir(std::vector<String_type> *db_name) {
  MY_DIR *a;
  uint i;
  FILEINFO *file;

  if (!(a = my_dir(mysql_real_data_home, MYF(MY_WANT_STAT)))) return true;

  for (i = 0; i < (uint)a->number_off_files; i++) {
    file = a->dir_entry + i;

    if (file->name[0] == '.') continue;

    if (MY_S_ISDIR(a->dir_entry[i].mystat->st_mode) &&
        strcmp(a->dir_entry[i].name, "#innodb_temp") != 0) {
      db_name->push_back(a->dir_entry[i].name);
      continue;
    }
  }

  my_dirend(a);
  return false;
}

}  // namespace upgrade_57
}  // namespace dd
