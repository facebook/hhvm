/*
   Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQLCHECK_INCLUDED
#define MYSQLCHECK_INCLUDED

#include <sys/types.h>
#include <string>
#include <vector>

#include "mysql.h"

namespace Mysql {
namespace Tools {
namespace Check {

enum operations {
  DO_CHECK = 1,
  DO_REPAIR,
  DO_ANALYZE,
  DO_OPTIMIZE,
  DO_UPGRADE
};

extern void mysql_check(MYSQL *connection, int what_to_do, bool opt_alldbs,
                        bool opt_check_only_changed, bool opt_extended,
                        bool opt_databases, bool opt_fast,
                        bool opt_medium_check, bool opt_quick,
                        bool opt_all_in_1, bool opt_silent,
                        bool opt_auto_repair, bool ignore_errors, bool opt_frm,
                        bool opt_fix_table_names, bool opt_fix_db_names,
                        bool opt_upgrade, bool opt_write_binlog,
                        unsigned int verbose, std::string opt_skip_database,
                        std::vector<std::string> arguments,
                        void (*dberror)(MYSQL *mysql, const std::string &when));

/**
  This class is object wrapper to mysql_check function. It looks like
  it is implementing Abstract_program, but it is not explicitly implementing
  it now. This is to make future implementation of Abstract_program easier.
 */
class Program {
 public:
  /**
    Default constructor.
   */
  Program();

  /**
    Checks specified databases on MySQL server.
   */
  int check_databases(MYSQL *connection,
                      const std::vector<std::string> &databases);
  /**
    Checks all databases on MySQL server.
   */
  int check_all_databases(MYSQL *connection);

  /**
    Automatically try to fix table when upgrade is needed.
   */
  Program *enable_auto_repair(bool enable);
  /**
    Check and upgrade tables.
   */
  Program *enable_upgrade(bool enable);
  /**
    Turns on verbose messages.
   */
  Program *enable_verbosity(bool enable);
  /**
    Enables logging repairing queries to binlog.
   */
  Program *enable_writing_binlog(bool enable);
  /**
    Enables table name fixing for all encountered tables.
   */
  Program *enable_fixing_table_names(bool enable);
  /**
    Enables database name fixing for all encountered databases.
   */
  Program *enable_fixing_db_names(bool enable);
  /**
    Ignores all errors and don't print error messages.
   */
  Program *set_ignore_errors(bool ignore);
  /**
    Sets a name of database to ignore during process.
   */
  Program *set_skip_database(std::string database);
  /**
    Sets error callback to be called when error is encountered.
   */
  Program *set_error_callback(void (*error_callback)(MYSQL *mysql,
                                                     const std::string &when));

 private:
  /**
    Sets mysqlcheck program operation type to perform.
   */
  Program *set_what_to_do(int functionality);
  /**
    Starts mysqlcheck process.
   */
  int execute(const std::vector<std::string> &positional_options);

  int m_what_to_do;
  bool m_auto_repair;
  bool m_upgrade;
  bool m_verbose;
  bool m_ignore_errors;
  bool m_write_binlog;
  bool m_process_all_dbs;
  bool m_fix_table_names;
  bool m_fix_db_names;
  MYSQL *m_connection;
  std::string m_database_to_skip;
  void (*m_error_callback)(MYSQL *mysql, const std::string &when);
};

}  // namespace Check
}  // namespace Tools
}  // namespace Mysql

#endif
