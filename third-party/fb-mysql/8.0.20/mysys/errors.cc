/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/errors.cc
*/

#include "my_config.h"
#include "my_loglevel.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_dbug.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysys_err.h"

const struct my_glob_errors globerrs[GLOBERRS] = {
    {"EE_CANTCREATEFILE", "Can't create/write to file '%s' (OS errno %d - %s)"},
    {"EE_READ", "Error reading file '%s' (OS errno %d - %s)"},
    {"EE_WRITE", "Error writing file '%s' (OS errno %d - %s)"},
    {"EE_BADCLOSE", "Error on close of '%s' (OS errno %d - %s)"},
    {"EE_OUTOFMEMORY", "Out of memory (Needed %u bytes)"},
    {"EE_DELETE", "Error on delete of '%s' (OS errno %d - %s)"},
    {"EE_LINK", "Error on rename of '%s' to '%s' (OS errno %d - %s)"},
    {"", ""},
    {"EE_EOFERR",
     "Unexpected EOF found when reading file '%s' (OS errno %d - %s)"},
    {"EE_CANTLOCK", "Can't lock file (OS errno %d - %s)"},
    {"EE_CANTUNLOCK", "Can't unlock file (OS errno %d - %s)"},
    {"EE_DIR", "Can't read dir of '%s' (OS errno %d - %s)"},
    {"EE_STAT", "Can't get stat of '%s' (OS errno %d - %s)"},
    {"EE_CANT_CHSIZE", "Can't change size of file (OS errno %d - %s)"},
    {"EE_CANT_OPEN_STREAM", "Can't open stream from handle (OS errno %d - %s)"},
    {"EE_GETWD", "Can't get working directory (OS errno %d - %s)"},
    {"EE_SETWD", "Can't change dir to '%s' (OS errno %d - %s)"},
    {"EE_LINK_WARNING", "Warning: '%s' had %d links"},
    {"EE_OPEN_WARNING", "Warning: %d files and %d streams are left open"},
    {"EE_DISK_FULL",
     "Disk is full writing '%s' (OS errno %d - %s). Waiting for someone to "
     "free space..."},
    {"EE_CANT_MKDIR", "Can't create directory '%s' (OS errno %d - %s)"},
    {"EE_UNKNOWN_CHARSET",
     "Character set '%s' is not a compiled character set and is not specified "
     "in the '%s' file"},
    {"EE_OUT_OF_FILERESOURCES",
     "Out of resources when opening file '%s' (OS errno %d - %s)"},
    {"EE_CANT_READLINK", "Can't read value for symlink '%s' (Error %d - %s)"},
    {"EE_CANT_SYMLINK",
     "Can't create symlink '%s' pointing at '%s' (Error %d - %s)"},
    {"EE_REALPATH", "Error on realpath() on '%s' (Error %d - %s)"},
    {"EE_SYNC", "Can't sync file '%s' to disk (OS errno %d - %s)"},
    {"EE_UNKNOWN_COLLATION",
     "Collation '%s' is not a compiled collation and is not specified in the "
     "'%s' file"},
    {"EE_FILENOTFOUND", "File '%s' not found (OS errno %d - %s)"},
    {"EE_FILE_NOT_CLOSED", "File '%s' (fileno: %d) was not closed"},
    {"EE_CHANGE_OWNERSHIP",
     "Cannot change ownership of the file '%s' (OS errno %d - %s)"},
    {"EE_CHANGE_PERMISSIONS",
     "Cannot change permissions of the file '%s' (OS errno %d - %s)"},
    {"EE_CANT_SEEK", "Cannot seek in file '%s' (OS errno %d - %s)"},
    {"EE_CAPACITY_EXCEEDED", "Memory capacity exceeded (capacity %llu bytes)"},
    {"EE_DISK_FULL_WITH_RETRY_MSG",
     "Disk is full writing '%s' (OS errno %d - %s). Waiting for someone to "
     "free space... Retry in %d secs. Message reprinted in %d secs."},
    {"EE_FAILED_TO_CREATE_TIMER", "Failed to create timer (OS errno %d)."},
    {"EE_FAILED_TO_DELETE_TIMER", "Failed to delete timer (OS errno %d)."},
    {"EE_FAILED_TO_CREATE_TIMER_QUEUE",
     "Failed to create timer queue (OS errno %d)."},
    {"EE_FAILED_TO_START_TIMER_NOTIFY_THREAD",
     "Failed to start timer notify thread."},
    {"EE_FAILED_TO_CREATE_TIMER_NOTIFY_THREAD_INTERRUPT_EVENT",
     "Failed to create event to interrupt timer notifier thread (OS errno "
     "%d)."},
    {"EE_EXITING_TIMER_NOTIFY_THREAD",
     "Failed to register timer event with queue (OS errno %d), exiting timer "
     "notifier thread."},
    {"EE_WIN_LIBRARY_LOAD_FAILED",
     "LoadLibrary(\"kernel32.dll\") failed: GetLastError returns %lu."},
    {"EE_WIN_RUN_TIME_ERROR_CHECK", "%s."},
    {"EE_FAILED_TO_DETERMINE_LARGE_PAGE_SIZE",
     "Failed to determine large page size."},
    {"EE_FAILED_TO_KILL_ALL_THREADS",
     "Error in my_thread_global_end(): %d thread(s) did not exit."},
    {"EE_FAILED_TO_CREATE_IO_COMPLETION_PORT",
     "Failed to create IO completion port (OS errno %d)."},
    {"EE_FAILED_TO_OPEN_DEFAULTS_FILE",
     "Failed to open required defaults file: %s"},
    {"EE_FAILED_TO_HANDLE_DEFAULTS_FILE",
     "Fatal error in defaults handling. Program aborted!"},
    {"EE_WRONG_DIRECTIVE_IN_CONFIG_FILE",
     "Wrong '!%s' directive in config file %s at line %d."},
    {"EE_SKIPPING_DIRECTIVE_DUE_TO_MAX_INCLUDE_RECURSION",
     "Skipping '%s' directive as maximum include recursion level was"
     " reached in file %s at line %d."},
    {"EE_INCORRECT_GRP_DEFINITION_IN_CONFIG_FILE",
     "Wrong group definition in config file %s at line %d."},
    {"EE_OPTION_WITHOUT_GRP_IN_CONFIG_FILE",
     "Found option without preceding group in config file %s at line %d."},
    {"EE_CONFIG_FILE_PERMISSION_ERROR",
     "%s should be readable/writable only by current user."},
    {"EE_IGNORE_WORLD_WRITABLE_CONFIG_FILE",
     "World-writable config file '%s' is ignored."},
    {"EE_USING_DISABLED_OPTION", "%s: Option '%s' was used, but is disabled."},
    {"EE_USING_DISABLED_SHORT_OPTION",
     "%s: Option '-%c' was used, but is disabled."},
    {"EE_USING_PASSWORD_ON_CLI_IS_INSECURE",
     "Using a password on the command line interface can be insecure."},
    {"EE_UNKNOWN_SUFFIX_FOR_VARIABLE",
     "Unknown suffix '%c' used for variable '%s' (value '%s')."},
    {"EE_SSL_ERROR_FROM_FILE", "SSL error: %s from '%s'."},
    {"EE_SSL_ERROR", "SSL error: %s."},
    {"EE_NET_SEND_ERROR_IN_BOOTSTRAP", "%d  %s."},
    {"EE_PACKETS_OUT_OF_ORDER",
     "Packets out of order (found %u, expected %u)."},
    {"EE_UNKNOWN_PROTOCOL_OPTION", "Unknown option to protocol: %s."},
    {"EE_FAILED_TO_LOCATE_SERVER_PUBLIC_KEY",
     "Failed to locate server public key '%s'."},
    {"EE_PUBLIC_KEY_NOT_IN_PEM_FORMAT",
     "Public key is not in Privacy Enhanced Mail format: '%s'."},
    {"EE_DEBUG_INFO", "%s."},
    {"EE_UNKNOWN_VARIABLE", "unknown variable '%s'."},
    {"EE_UNKNOWN_OPTION", "unknown option '--%s'."},
    {"EE_UNKNOWN_SHORT_OPTION", "%s: unknown option '-%c'."},
    {"EE_OPTION_WITHOUT_ARGUMENT",
     "%s: option '--%s' cannot take an argument."},
    {"EE_OPTION_REQUIRES_ARGUMENT", "%s: option '--%s' requires an argument."},
    {"EE_SHORT_OPTION_REQUIRES_ARGUMENT",
     "%s: option '-%c' requires an argument."},
    {"EE_OPTION_IGNORED_DUE_TO_INVALID_VALUE",
     "%s: ignoring option '--%s' due to invalid value '%s'."},
    {"EE_OPTION_WITH_EMPTY_VALUE", "%s: Empty value for '%s' specified."},
    {"EE_FAILED_TO_ASSIGN_MAX_VALUE_TO_OPTION",
     "%s: Maximum value of '%s' cannot be set."},
    {"EE_INCORRECT_BOOLEAN_VALUE_FOR_OPTION",
     "option '%s': boolean value '%s' was not recognized. Set to OFF."},
    {"EE_FAILED_TO_SET_OPTION_VALUE",
     "%s: Error while setting value '%s' to '%s'."},
    {"EE_INCORRECT_INT_VALUE_FOR_OPTION", "Incorrect integer value: '%s'."},
    {"EE_INCORRECT_UINT_VALUE_FOR_OPTION",
     "Incorrect unsigned integer value: '%s'."},
    {"EE_ADJUSTED_SIGNED_VALUE_FOR_OPTION",
     "option '%s': signed value %s adjusted to %s."},
    {"EE_ADJUSTED_UNSIGNED_VALUE_FOR_OPTION",
     "option '%s': unsigned value %s adjusted to %s."},
    {"EE_ADJUSTED_ULONGLONG_VALUE_FOR_OPTION",
     "option '%s': value %s adjusted to %s."},
    {"EE_ADJUSTED_DOUBLE_VALUE_FOR_OPTION",
     "option '%s': value %g adjusted to %g."},
    {"EE_INVALID_DECIMAL_VALUE_FOR_OPTION",
     "Invalid decimal value for option '%s'."},
    {"EE_COLLATION_PARSER_ERROR", "%s."},
    {"EE_FAILED_TO_RESET_BEFORE_PRIMARY_IGNORABLE_CHAR",
     "Failed to reset before a primary ignorable character %s."},
    {"EE_FAILED_TO_RESET_BEFORE_TERTIARY_IGNORABLE_CHAR",
     "Failed to reset before a territory ignorable character %s."},
    {"EE_SHIFT_CHAR_OUT_OF_RANGE", "Shift character out of range: %s."},
    {"EE_RESET_CHAR_OUT_OF_RANGE", "Reset character out of range: %s."},
    {"EE_UNKNOWN_LDML_TAG", "Unknown LDML tag: '%.*s'."},
    {"EE_FAILED_TO_RESET_BEFORE_SECONDARY_IGNORABLE_CHAR",
     "Failed to reset before a secondary ignorable character %s."}};

/*
 We cannot call my_error/my_printf_error here in this function.
  Those functions will set status variable in diagnostic area
  and there is no provision to reset them back.
  Here we are waiting for free space and will wait forever till
  space is created. So just giving warning in the error file
  should be enough.
*/
void wait_for_free_space(const char *filename, int errors) {
  size_t time_to_sleep = MY_WAIT_FOR_USER_TO_FIX_PANIC;

  if (!(errors % MY_WAIT_GIVE_USER_A_MESSAGE)) {
    char errbuf[MYSYS_STRERROR_SIZE];
    my_message_local(
        ERROR_LEVEL, EE_DISK_FULL_WITH_RETRY_MSG, filename, my_errno(),
        my_strerror(errbuf, sizeof(errbuf), my_errno()),
        MY_WAIT_FOR_USER_TO_FIX_PANIC,
        MY_WAIT_GIVE_USER_A_MESSAGE * MY_WAIT_FOR_USER_TO_FIX_PANIC);
  }
  DBUG_EXECUTE_IF("simulate_no_free_space_error", { time_to_sleep = 1; });
  DBUG_EXECUTE_IF("force_wait_for_disk_space", { time_to_sleep = 1; });
  DBUG_EXECUTE_IF("simulate_io_thd_wait_for_disk_space",
                  { time_to_sleep = 1; });
  DBUG_EXECUTE_IF("simulate_random_io_thd_wait_for_disk_space",
                  { time_to_sleep = 1; });
  // Answer more promptly to a KILL signal
  do {
    (void)sleep(1);
  } while (--time_to_sleep > 0 && !is_killed_hook(nullptr));
}

const char *get_global_errname(int nr) {
  return globerrs[nr - EE_ERROR_FIRST].errname;
}

const char *get_global_errmsg(int nr) {
  return globerrs[nr - EE_ERROR_FIRST].errdesc;
}
