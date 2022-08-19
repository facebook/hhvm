/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_initialize.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_rnd.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#include "scripts/sql_commands_help_data.h"
#include "scripts/sql_commands_system_data.h"
#include "scripts/sql_commands_system_tables.h"
#include "scripts/sql_commands_system_users.h"
#include "scripts/sys_schema/sql_commands.h"
#include "sql/mysqld.h"
#include "sql/server_component/log_builtins_filter_imp.h"  // verbosity
#include "sql/sql_bootstrap.h"

static const char *initialization_cmds[] = {"USE mysql;\n", nullptr};

#define INSERT_USER_CMD \
  "CREATE USER root@localhost IDENTIFIED BY '%s' PASSWORD EXPIRE;\n"
#define INSERT_USER_CMD_INSECURE "CREATE USER root@localhost;\n"
#define GENERATED_PASSWORD_LENGTH 12

char
    insert_user_buffer[sizeof(INSERT_USER_CMD) + GENERATED_PASSWORD_LENGTH * 2];

bool opt_initialize_insecure = false;
/** True if --initialize has actually created the directory */
bool mysql_initialize_directory_freshly_created = false;

static const char *initialization_data[] = {
    "FLUSH PRIVILEGES", insert_user_buffer,
    "GRANT ALL PRIVILEGES ON *.* TO root@localhost WITH GRANT OPTION;\n",
    "GRANT PROXY ON ''@'' TO 'root'@'localhost' WITH GRANT OPTION;\n", nullptr};

static const char **cmds[] = {initialization_cmds, mysql_system_tables,
                              initialization_data, mysql_system_data,
                              fill_help_tables,    mysql_system_users,
                              mysql_sys_schema,    nullptr};

/** keep in sync with the above array */
static const char *cmd_descs[] = {
    "Creating the system database",
    "Creating the system tables",
    "Filling in the system tables, part 1",
    "Filling in the system tables, part 2",
    "Filling in the mysql.help table",
    "Creating the system users for internal usage",
    "Creating the sys schema",
    nullptr};

static bool generate_password(char *password, int size) {
#define UPCHARS "QWERTYUIOPASDFGHJKLZXCVBNM"
#define LOWCHARS "qwertyuiopasdfghjklzxcvbnm"
#define NUMCHARS "1234567890"
#define SYMCHARS ",.-+*;:_!#%&/()=?><"
#define rnd_of(x) x[((int)(my_rnd_ssl(&failed) * 100)) % (sizeof(x) - 1)]

  static const char g_allowed_pwd_chars[] = LOWCHARS SYMCHARS UPCHARS NUMCHARS;
  static const char g_upper_case_chars[] = UPCHARS;
  static const char g_lower_case_chars[] = LOWCHARS;
  static const char g_numeric_chars[] = NUMCHARS;
  static const char g_special_chars[] = SYMCHARS;
  bool failed = false;
  char *ptr = password;
  bool had_upper = false, had_lower = false, had_numeric = false,
       had_special = false;

  for (; size > 0; --size) {
    char ch = rnd_of(g_allowed_pwd_chars);
    if (failed) return failed;
    /*
      Ensure we have a password that conforms to the strong
      password validation plugin ploicy by re-drawing specially
      the last 4 chars if there's need.
    */
    if (size == 4 && !had_lower) {
      ch = rnd_of(g_lower_case_chars);
      if (failed) return failed;
      had_lower = true;
    } else if (size == 3 && !had_numeric) {
      ch = rnd_of(g_numeric_chars);
      if (failed) return failed;
      had_numeric = true;
    } else if (size == 2 && !had_special) {
      ch = rnd_of(g_special_chars);
      if (failed) return failed;
      had_special = true;
    } else if (size == 1 && !had_upper) {
      ch = rnd_of(g_upper_case_chars);
      if (failed) return failed;
      had_upper = true;
    }

    if (!had_upper && strchr(g_upper_case_chars, ch))
      had_upper = true;
    else if (!had_lower && strchr(g_lower_case_chars, ch))
      had_lower = true;
    else if (!had_numeric && strchr(g_numeric_chars, ch))
      had_numeric = true;
    else if (!had_special && strchr(g_special_chars, ch))
      had_special = true;

    *ptr++ = ch;
  }
  return failed;
}

bool Compiled_in_command_iterator::begin(void) {
  m_cmds_ofs = m_cmd_ofs = 0;

  LogErr(INFORMATION_LEVEL, ER_SERVER_INIT_COMPILED_IN_COMMANDS,
         cmd_descs[m_cmds_ofs]);
  if (opt_initialize_insecure) {
    strcpy(insert_user_buffer, INSERT_USER_CMD_INSECURE);
    LogErr(WARNING_LEVEL, ER_INIT_ROOT_WITHOUT_PASSWORD);
  } else {
    char password[GENERATED_PASSWORD_LENGTH + 1];
    char escaped_password[GENERATED_PASSWORD_LENGTH * 2 + 1];
    ulong saved_verbosity = log_error_verbosity;

    if (generate_password(password, GENERATED_PASSWORD_LENGTH)) {
      LogErr(ERROR_LEVEL, ER_INIT_FAILED_TO_GENERATE_ROOT_PASSWORD);
      return true;
    }
    password[GENERATED_PASSWORD_LENGTH] = 0;

    /*
      Temporarily bump verbosity to print the password.
      It's safe to do it since we're the sole process running.
    */
    log_builtins_filter_update_verbosity((log_error_verbosity = 3));
    LogErr(INFORMATION_LEVEL, ER_INIT_GENERATING_TEMP_PASSWORD_FOR_ROOT,
           password);
    log_builtins_filter_update_verbosity(
        (log_error_verbosity = saved_verbosity));

    escape_string_for_mysql(&my_charset_bin, escaped_password,
                            sizeof(escaped_password), password,
                            GENERATED_PASSWORD_LENGTH);

    sprintf(insert_user_buffer, INSERT_USER_CMD, escaped_password);
  }

  return false;
}

int Compiled_in_command_iterator::next(std::string &query) {
  while (cmds[m_cmds_ofs] != nullptr &&
         cmds[m_cmds_ofs][m_cmd_ofs] == nullptr) {
    m_cmds_ofs++;
    if (cmds[m_cmds_ofs] != nullptr)
      LogErr(INFORMATION_LEVEL, ER_SERVER_INIT_COMPILED_IN_COMMANDS,
             cmd_descs[m_cmds_ofs]);
    m_cmd_ofs = 0;
  }

  if (cmds[m_cmds_ofs] == nullptr) {
    return READ_BOOTSTRAP_EOF;
  }

  query.assign(cmds[m_cmds_ofs][m_cmd_ofs++]);
  return READ_BOOTSTRAP_SUCCESS;
}

void Compiled_in_command_iterator::report_error_details(
    log_function_t /* log */) {
  /*
    Compiled in commands are represented in strings in a C array.
    There is no parsing involved to isolate each query,
    so ::next() never returns errors.
    Hence, there should never be an error to print.
  */
  DBUG_ASSERT(false);
  return;
}

void Compiled_in_command_iterator::end(void) {
  LogErr(INFORMATION_LEVEL, ER_INIT_BOOTSTRAP_COMPLETE);
}

/**
  Create the data directory

  Creates the data directory when --initialize is specified.
  The directory is created when it does not exist.
  If it exists, is empty and the process can write into it
  no action is taken and the directory is accepted.
  Otherwise an error is thrown.

  @param  data_home  the normalized path to the data directory
  @return status
  @retval true   failed to create. Error printed.
  @retval false  success
*/
bool initialize_create_data_directory(const char *data_home) {
  MY_DIR *dir;
  int flags =
#ifdef _WIN32
      0
#else
      S_IRWXU | S_IRGRP | S_IXGRP
#endif
      ;

  if (nullptr != (dir = my_dir(data_home, MYF(MY_DONT_SORT)))) {
    bool no_files = true;
    char path[FN_REFLEN];
    File fd;

    /* Ignore files that start with . or == 'lost+found'. */
    for (uint i = 0; i < dir->number_off_files; i++) {
      FILEINFO *file = dir->dir_entry + i;
      if (file->name[0] != '.' && strcmp(file->name, "lost+found")) {
        no_files = false;
        break;
      }
    }

    my_dirend(dir);

    if (!no_files) {
      LogErr(ERROR_LEVEL, ER_INIT_DATADIR_NOT_EMPTY_WONT_INITIALIZE);
      return true; /* purecov: inspected */
    }

    LogErr(INFORMATION_LEVEL, ER_INIT_DATADIR_EXISTS_WONT_INITIALIZE);

    if (nullptr == fn_format(path, "is_writable", data_home, "",
                             MY_UNPACK_FILENAME | MY_SAFE_PATH)) {
      LogErr(ERROR_LEVEL,
             ER_INIT_DATADIR_EXISTS_AND_PATH_TOO_LONG_WONT_INITIALIZE);
      return true; /* purecov: inspected */
    }
    if (-1 != (fd = my_create(path, 0, flags, MYF(MY_WME)))) {
      my_close(fd, MYF(MY_WME));
      my_delete(path, MYF(MY_WME));
    } else {
      LogErr(ERROR_LEVEL,
             ER_INIT_DATADIR_EXISTS_AND_NOT_WRITABLE_WONT_INITIALIZE);
      return true; /* purecov: inspected */
    }

    /* the data dir found is usable */
    return false;
  }

  LogErr(INFORMATION_LEVEL, ER_INIT_CREATING_DD, data_home);
  if (my_mkdir(data_home, flags, MYF(MY_WME)))
    return true; /* purecov: inspected */

  mysql_initialize_directory_freshly_created = true;
  return false;
}
