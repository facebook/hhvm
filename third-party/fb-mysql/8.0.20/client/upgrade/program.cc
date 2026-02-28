/*
   Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <iostream>
#include <string>
#include <vector>

#include "client/base/abstract_connection_program.h"

using namespace Mysql::Tools::Base;
using std::string;
using std::vector;

const char *load_default_groups[] = {
    "client",        /* Read settings how to connect to server */
    "mysql_upgrade", /* Read special settings for mysql_upgrade*/
    nullptr};

namespace Mysql {
namespace Tools {
namespace Upgrade {

string deprecation_msg =
    "The mysql_upgrade client is now deprecated. The actions executed by the "
    "upgrade client are now done by the server.\nTo upgrade, please start the "
    "new MySQL binary with the older data directory. Repairing user tables is "
    "done automatically. Restart is not required after upgrade.\nThe upgrade "
    "process automatically starts on running a new MySQL binary with an older "
    "data directory. To avoid accidental upgrades, please use the "
    "--upgrade=NONE option with the MySQL binary. The option --upgrade=FORCE "
    "is also provided to run the server upgrade sequence on demand.\nIt may be "
    "possible that the server upgrade fails due to a number of reasons. In "
    "that case, the upgrade sequence will run again during the next MySQL "
    "server start. If the server upgrade fails repeatedly, the server can be "
    "started with the --upgrade=MINIMAL option to start the server without "
    "executing the upgrade sequence, thus allowing users to manually rectify "
    "the problem.";

class Program : public Base::Abstract_connection_program {
 public:
  Program()
      : Abstract_connection_program(),
        m_mysql_connection(nullptr),
        m_temporary_verbose(false) {}

  string get_version() { return "2.0"; }

  int get_first_release_year() { return 2000; }

  string get_description() {
    return "MySQL utility for upgrading databases to new MySQL versions "
           "(deprecated).\n" +
           deprecation_msg;
  }

  void short_usage() {
    std::cout << "Usage: " << get_name() << " [OPTIONS]" << std::endl;
  }

  int get_error_code() { return 0; }

  /**
    Returns 0 (success) unconditionally, along with a deprecation message.

    @return 0 (success)
   */
  int execute(const vector<string> &) {
    return this->print_message(0, deprecation_msg);
  }

  void create_options() {
    this->create_new_option(
            &this->m_check_version, "version-check",
            "Run this program only if its \'server version\' "
            "matches the version of the server to which it's connecting, "
            "(enabled by default); use --skip-version-check to avoid this "
            "check. "
            "Note: the \'server version\' of the program is the version of the "
            "MySQL server with which it was built/distributed.")
        ->set_short_character('k')
        ->set_value(true);

    this->create_new_option(
            &this->m_upgrade_systables_only, "upgrade-system-tables",
            "Only upgrade the system tables, do not try to upgrade the data.")
        ->set_short_character('s');

    this->create_new_option(&this->m_verbose, "verbose",
                            "Display more output about the process.")
        ->set_short_character('v')
        ->set_value(true);

    this->create_new_option(
        &this->m_write_binlog, "write-binlog",
        "Write all executed SQL statements to binary log. Disabled by default; "
        "use when statements should be sent to replication slaves.");

    this->create_new_option(&this->m_ignore_errors, "force",
                            "Force execution of SQL statements even if "
                            "mysql_upgrade has already "
                            "been executed for the current version of MySQL.")
        ->set_short_character('f');

    this->create_new_option(&this->m_skip_sys_schema, "skip-sys-schema",
                            "Do not upgrade/install the sys schema.")
        ->set_value(false);
  }

  void error(const Message_data &message) {
    std::cerr << "Upgrade process encountered error and will not continue."
              << std::endl;

    exit(message.get_code());
  }

 private:
  /**
    Prints error occurred in main routine.
   */
  int print_error(int exit_code, string message) {
    std::cout << "Error occurred: " << message << std::endl;
    return exit_code;
  }

  // Print message and exit
  int print_message(int exit_code, string message) {
    std::cout << message << std::endl;
    return exit_code;
  }

  MYSQL *m_mysql_connection;
  bool m_write_binlog;
  bool m_upgrade_systables_only;
  bool m_skip_sys_schema;
  bool m_check_version;
  bool m_ignore_errors;
  bool m_verbose;
  /**
    Enabled during some queries execution to force printing all notes and
    warnings regardless "verbose" option.
   */
  bool m_temporary_verbose;
};

}  // namespace Upgrade
}  // namespace Tools
}  // namespace Mysql

int main(int argc, char **argv) {
  ::Mysql::Tools::Upgrade::Program program;
  program.run(argc, argv);
  return 0;
}
