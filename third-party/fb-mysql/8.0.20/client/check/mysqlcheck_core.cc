/*
   Copyright (c) 2001, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <mysql_version.h>
#include <mysqld_error.h>
#include <sys/types.h>
#include <string>
#include <vector>

#include "client/check/mysqlcheck.h"
#include "client/client_priv.h"
#include "m_ctype.h"
#include "my_default.h"
#include "my_inttypes.h"

using namespace Mysql::Tools::Check;

using std::string;
using std::vector;

/* ALTER instead of repair. */
#define MAX_ALTER_STR_SIZE 128 * 1024
#define KEY_PARTITIONING_CHANGED_STR "KEY () partitioning changed"

static MYSQL *sock = nullptr;
static bool opt_alldbs = false, opt_check_only_changed = false,
            opt_extended = false, opt_databases = false, opt_fast = false,
            opt_medium_check = false, opt_quick = false, opt_all_in_1 = false,
            opt_silent = false, opt_auto_repair = false, ignore_errors = false,
            opt_frm = false, opt_fix_table_names = false,
            opt_fix_db_names = false, opt_upgrade = false,
            opt_write_binlog = true;
static uint verbose = 0;
static string opt_skip_database;
int what_to_do = 0;

void (*DBError)(MYSQL *mysql, const string &when);

static int first_error = 0;
vector<string> tables4repair, tables4rebuild, alter_table_cmds;

static int process_all_databases();
static int process_databases(const vector<string> &db_names);
static int process_selected_tables(const string &db,
                                   const vector<string> &table_names);
static int process_all_tables_in_db(const string &database);
static int process_one_db(const string &database);
static int use_db(const string &database);
static int handle_request_for_tables(const string &tables);
static void print_result();
static string escape_table_name(const string &src);

static int process_all_databases() {
  MYSQL_ROW row;
  MYSQL_RES *tableres;
  int result = 0;

  if (mysql_query(sock, "SHOW DATABASES") ||
      !(tableres = mysql_store_result(sock))) {
    my_printf_error(0, "Error: Couldn't execute 'SHOW DATABASES': %s", MYF(0),
                    mysql_error(sock));
    return 1;
  }
  while ((row = mysql_fetch_row(tableres))) {
    if (process_one_db(row[0])) result = 1;
  }
  mysql_free_result(tableres);
  return result;
}
/* process_all_databases */

static int process_databases(const vector<string> &db_names) {
  int result = 0;
  for (const string &db_name : db_names) {
    if (process_one_db(db_name)) result = 1;
  }
  return result;
} /* process_databases */

static int process_selected_tables(const string &db,
                                   const vector<string> &table_names) {
  if (use_db(db)) return 1;

  /*
    TODO (a bug): properly handle all-in-1 option:
    we should create and pass a table list to handle_request_for_tables().
  */
  for (const string &table_name : table_names) {
    handle_request_for_tables(escape_table_name(table_name));
  }

  return 0;
} /* process_selected_tables */

static inline void escape_str(const string &src, size_t start, size_t end,
                              string &res) {
  res += '`';
  for (size_t i = start; i < end; i++) {
    switch (src[i]) {
      case '`': /* Escape backtick character. */
        res += '`';
        /* Fall through. */
      default:
        res += src[i];
    }
  }
  res += '`';
}

static string escape_table_name(const string &src) {
  string res = "";

  escape_str(src, 0, src.length(), res);
  return res;
}

static string escape_db_table_name(const string &src, size_t dot_pos) {
  string res = "";

  /* Escape database name. */
  escape_str(src, 0, dot_pos - 1, res);
  /* Add a dot. */
  res += '.';
  /* Escape table name. */
  escape_str(src, dot_pos, src.length(), res);

  return res;
}

static int process_all_tables_in_db(const string &database) {
  MYSQL_RES *res = nullptr;
  MYSQL_ROW row;
  uint num_columns;

  if (use_db(database)) return 1;
  if ((mysql_query(sock, "SHOW /*!50002 FULL*/ TABLES") &&
       mysql_query(sock, "SHOW TABLES")) ||
      !(res = mysql_store_result(sock))) {
    my_printf_error(0, "Error: Couldn't get table list for database %s: %s",
                    MYF(0), database.c_str(), mysql_error(sock));
    return 1;
  }

  num_columns = mysql_num_fields(res);

  vector<string> table_names;

  while ((row = mysql_fetch_row(res))) {
    /* Skip views if we don't perform renaming. */
    if ((num_columns == 2) && (strcmp(row[1], "VIEW") == 0)) continue;

    table_names.push_back(row[0]);
  }
  mysql_free_result(res);

  process_selected_tables(database, table_names);
  return 0;
} /* process_all_tables_in_db */

static int run_query(const string &query) {
  if (mysql_query(sock, query.c_str())) {
    fprintf(stderr, "Failed to run query \"%s\"\n", query.c_str());
    fprintf(stderr, "Error: %s\n", mysql_error(sock));
    return 1;
  }
  return 0;
}

static int rebuild_table(const string &name) {
  int rc = 0;
  string query = "ALTER TABLE " + name + " FORCE";
  if (mysql_real_query(sock, query.c_str(), (ulong)query.length())) {
    fprintf(stderr, "Failed to %s\n", query.c_str());
    fprintf(stderr, "Error: %s\n", mysql_error(sock));
    rc = 1;
  } else
    printf("%s\nRunning  : %s\nstatus   : OK\n", name.c_str(), query.c_str());
  return rc;
}

static int process_one_db(const string &database) {
  if (opt_skip_database.length() > 0 && opt_alldbs &&
      database == opt_skip_database)
    return 0;

  return process_all_tables_in_db(database);
}

static int use_db(const string &database) {
  if (mysql_get_server_version(sock) >= FIRST_INFORMATION_SCHEMA_VERSION &&
      !my_strcasecmp(&my_charset_latin1, database.c_str(),
                     INFORMATION_SCHEMA_DB_NAME))
    return 1;
  if (mysql_get_server_version(sock) >= FIRST_PERFORMANCE_SCHEMA_VERSION &&
      !my_strcasecmp(&my_charset_latin1, database.c_str(),
                     PERFORMANCE_SCHEMA_DB_NAME_MACRO))
    return 1;
  if (mysql_select_db(sock, database.c_str())) {
    DBError(sock, "when selecting the database");
    return 1;
  }
  return 0;
} /* use_db */

static int disable_binlog() { return run_query("SET SQL_LOG_BIN=0"); }

static int handle_request_for_tables(const string &tables) {
  string operation, options;

  switch (what_to_do) {
    case DO_CHECK:
      operation = "CHECK";
      if (opt_quick) options += " QUICK";
      if (opt_fast) options += " FAST";
      if (opt_medium_check) options += " MEDIUM"; /* Default */
      if (opt_extended) options += " EXTENDED";
      if (opt_check_only_changed) options += " CHANGED";
      if (opt_upgrade) options += " FOR UPGRADE";
      break;
    case DO_REPAIR:
      operation = (opt_write_binlog) ? "REPAIR" : "REPAIR NO_WRITE_TO_BINLOG";
      if (opt_quick) options += " QUICK";
      if (opt_extended) options += " EXTENDED";
      if (opt_frm) options += " USE_FRM";
      break;
    case DO_ANALYZE:
      operation = (opt_write_binlog) ? "ANALYZE" : "ANALYZE NO_WRITE_TO_BINLOG";
      break;
    case DO_OPTIMIZE:
      operation =
          (opt_write_binlog) ? "OPTIMIZE" : "OPTIMIZE NO_WRITE_TO_BINLOG";
      break;
  }

  string query = operation + " TABLE " + tables + " " + options;

  if (mysql_real_query(sock, query.c_str(), (ulong)query.length())) {
    DBError(sock,
            "when executing '" + operation + " TABLE ... " + options + "'");
    return 1;
  }
  print_result();
  return 0;
}

static void print_result() {
  MYSQL_RES *res;
  MYSQL_ROW row;
  char prev[NAME_LEN * 3 + 2];
  char prev_alter[MAX_ALTER_STR_SIZE];
  uint i;
  size_t dot_pos;
  bool found_error = false, table_rebuild = false;

  res = mysql_use_result(sock);
  dot_pos = strlen(sock->db) + 1;

  prev[0] = '\0';
  prev_alter[0] = 0;

  for (i = 0; (row = mysql_fetch_row(res)); i++) {
    int changed = strcmp(prev, row[0]);
    bool status = !strcmp(row[2], "status");

    if (status) {
      /*
        if there was an error with the table, we have --auto-repair set,
        and this isn't a repair op, then add the table to the tables4repair
        list
      */
      if (found_error && opt_auto_repair && what_to_do != DO_REPAIR &&
          strcmp(row[3], "OK")) {
        if (table_rebuild) {
          if (prev_alter[0])
            alter_table_cmds.push_back(prev_alter);
          else
            tables4rebuild.push_back(escape_db_table_name(prev, dot_pos));
        } else {
          tables4repair.push_back(escape_db_table_name(prev, dot_pos));
        }
      }
      found_error = false;
      table_rebuild = false;
      prev_alter[0] = 0;
      if (opt_silent) continue;
    }
    if (status && changed)
      printf("%-50s %s", row[0], row[3]);
    else if (!status && changed) {
      if (opt_auto_repair && what_to_do != DO_REPAIR) {
        printf("%-50s To be repaired, cause follows:\nServer issued %-9s: %s",
               row[0], row[2], row[3]);
      } else {
        printf("%s\n%-9s: %s", row[0], row[2], row[3]);
      }
      if (opt_auto_repair && strcmp(row[2], "note")) {
        const char *alter_txt = strstr(row[3], "ALTER TABLE");
        found_error = true;
        if (alter_txt) {
          table_rebuild = true;
          if (!strncmp(row[3], KEY_PARTITIONING_CHANGED_STR,
                       strlen(KEY_PARTITIONING_CHANGED_STR)) &&
              strstr(alter_txt, "PARTITION BY")) {
            if (strlen(alter_txt) >= MAX_ALTER_STR_SIZE) {
              printf(
                  "Error: Alter command too long (>= %d),"
                  " please do \"%s\" or dump/reload to fix it!\n",
                  MAX_ALTER_STR_SIZE, alter_txt);
              table_rebuild = false;
              prev_alter[0] = 0;
            } else {
              strncpy(prev_alter, alter_txt, MAX_ALTER_STR_SIZE - 1);
              prev_alter[MAX_ALTER_STR_SIZE - 1] = 0;
            }
          }
        }
      }
    } else
      printf("%-9s: %s", row[2], row[3]);
    my_stpcpy(prev, row[0]);
    putchar('\n');
  }
  /* add the last table to be repaired to the list */
  if (found_error && opt_auto_repair && what_to_do != DO_REPAIR) {
    if (table_rebuild) {
      if (prev_alter[0])
        alter_table_cmds.push_back(prev_alter);
      else
        tables4rebuild.push_back(escape_db_table_name(prev, dot_pos));
    } else {
      tables4repair.push_back(escape_db_table_name(prev, dot_pos));
    }
  }
  mysql_free_result(res);
}

void Mysql::Tools::Check::mysql_check(
    MYSQL *connection, int what_to_do, bool opt_alldbs,
    bool opt_check_only_changed, bool opt_extended, bool opt_databases,
    bool opt_fast, bool opt_medium_check, bool opt_quick, bool opt_all_in_1,
    bool opt_silent, bool opt_auto_repair, bool ignore_errors, bool opt_frm,
    bool opt_fix_table_names, bool opt_fix_db_names, bool opt_upgrade,
    bool opt_write_binlog, uint verbose, std::string opt_skip_database,
    std::vector<std::string> arguments,
    void (*dberror)(MYSQL *mysql, const std::string &when)) {
  ::sock = connection;
  ::what_to_do = what_to_do;
  ::opt_alldbs = opt_alldbs;
  ::opt_check_only_changed = opt_check_only_changed;
  ::opt_extended = opt_extended;
  ::opt_databases = opt_databases;
  ::opt_fast = opt_fast;
  ::opt_medium_check = opt_medium_check;
  ::opt_quick = opt_quick;
  ::opt_all_in_1 = opt_all_in_1;
  ::opt_silent = opt_silent;
  ::opt_auto_repair = opt_auto_repair;
  ::ignore_errors = ignore_errors;
  ::opt_frm = opt_frm;
  ::opt_fix_table_names = opt_fix_table_names;
  ::opt_fix_db_names = opt_fix_db_names;
  ::opt_upgrade = opt_upgrade;
  ::opt_write_binlog = opt_write_binlog;
  ::verbose = verbose;
  ::opt_skip_database = opt_skip_database;
  ::DBError = dberror;

  if (!::opt_write_binlog) {
    if (disable_binlog()) {
      first_error = 1;
      return;
    }
  }

  if (::opt_alldbs) process_all_databases();
  /* Only one database and selected table(s) */
  else if (arguments.size() > 1 && !::opt_databases) {
    string db_name = arguments[0];
    arguments.erase(arguments.begin());
    process_selected_tables(db_name, arguments);
  }
  /* One or more databases, all tables */
  else
    process_databases(arguments);
  if (::opt_auto_repair) {
    if (!::opt_silent && !(tables4repair.empty() && tables4rebuild.empty()))
      puts("\nRepairing tables");
    ::what_to_do = DO_REPAIR;

    for (const string &table4repair : tables4repair) {
      handle_request_for_tables(table4repair);
    }
    for (const string &table4rebuild : tables4rebuild) {
      rebuild_table(table4rebuild);
    }
    for (const string &alter_table_cmd : alter_table_cmds) {
      run_query(alter_table_cmd);
    }
  }
}

Program::Program()
    : m_what_to_do(0),
      m_auto_repair(false),
      m_upgrade(false),
      m_verbose(false),
      m_ignore_errors(false),
      m_write_binlog(false),
      m_process_all_dbs(false),
      m_fix_table_names(false),
      m_fix_db_names(false),
      m_connection(nullptr),
      m_error_callback(nullptr) {}

int Program::check_databases(MYSQL *connection,
                             const vector<string> &databases) {
  this->m_connection = connection;
  this->m_process_all_dbs = false;
  return this->set_what_to_do(DO_CHECK)->execute(databases);
}

int Program::check_all_databases(MYSQL *connection) {
  this->m_connection = connection;
  this->m_process_all_dbs = true;
  return this->set_what_to_do(DO_CHECK)->execute(vector<string>());
}

Program *Program::enable_auto_repair(bool enable) {
  this->m_auto_repair = enable;
  return this;
}

Program *Program::enable_upgrade(bool enable) {
  this->m_upgrade = enable;
  return this;
}

Program *Program::enable_verbosity(bool enable) {
  this->m_verbose = enable;
  return this;
}

Program *Program::enable_writing_binlog(bool enable) {
  this->m_write_binlog = enable;
  return this;
}

Program *Program::enable_fixing_table_names(bool enable) {
  this->m_fix_table_names = enable;
  return this;
}

Program *Program::enable_fixing_db_names(bool enable) {
  this->m_fix_db_names = enable;
  return this;
}

Program *Program::set_ignore_errors(bool ignore) {
  this->m_ignore_errors = ignore;
  return this;
}

Program *Program::set_skip_database(string database) {
  this->m_database_to_skip = std::move(database);
  return this;
}

Program *Program::set_error_callback(
    void (*error_callback)(MYSQL *mysql, const string &when)) {
  this->m_error_callback = error_callback;
  return this;
}

Program *Program::set_what_to_do(int functionality) {
  this->m_what_to_do = functionality;
  return this;
}

/// @relates Mysql::Tools::Check::Program
int Program::execute(const vector<string> &positional_options) {
  Mysql::Tools::Check::mysql_check(
      this->m_connection,        // connection
      this->m_what_to_do,        // what_to_do
      this->m_process_all_dbs,   // opt_alldbs
      false,                     // opt_check_only_changed
      false,                     // opt_extended
      !this->m_process_all_dbs,  // opt_databases
      false,                     // opt_fast
      false,                     // opt_medium_check
      false,                     // opt_quick
      false,                     // opt_all_in_1
      false,                     // opt_silent
      this->m_auto_repair,       // opt_auto_repair
      this->m_ignore_errors,     // ignore_errors
      false,                     // opt_frm
      this->m_fix_table_names,   // opt_fix_table_names
      this->m_fix_db_names,      // opt_fix_db_names
      this->m_upgrade,           // opt_upgrade
      this->m_write_binlog,      // opt_write_binlog
      this->m_verbose,           // verbose
      this->m_database_to_skip, positional_options, this->m_error_callback);
  return 0;
}
