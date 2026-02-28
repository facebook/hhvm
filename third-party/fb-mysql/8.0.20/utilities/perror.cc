/*
   Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

/* Return error-text for system error messages and handler messages */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "m_string.h"
#include "my_getopt.h"
#include "my_sys.h"
#include "mysys_err.h"
#include "print_version.h"
#include "welcome_copyright_notice.h" /* ORACLE_WELCOME_COPYRIGHT_NOTICE */

static bool verbose;

#include "my_base.h"
#include "my_compiler.h"
#include "mysys/my_handler_errors.h"

static struct my_option my_long_options[] = {
    {"help", '?', "Displays this help and exits.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"info", 'I', "Synonym for --help.", nullptr, nullptr, nullptr, GET_NO_ARG,
     NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"silent", 's', "Only print the error message.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"verbose", 'v', "Print error code and message (default).", &verbose,
     &verbose, nullptr, GET_BOOL, NO_ARG, 1, 0, 0, nullptr, 0, nullptr},
    {"version", 'V', "Displays version information and exits.", nullptr,
     nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

static void usage(void) {
  print_version();
  puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2000"));
  printf(
      "Print a description for a system error code or a MySQL error code.\n");
  printf(
      "If you want to get the error for a negative error code, you should "
      "use\n-- before the first error code to tell perror that there was no "
      "more options.\n\n");
  printf("Usage: %s [OPTIONS] [ERRORCODE [ERRORCODE...]]\n", my_progname);
  my_print_help(my_long_options);
  my_print_variables(my_long_options);
}

static bool get_one_option(int optid,
                           const struct my_option *opt MY_ATTRIBUTE((unused)),
                           char *argument MY_ATTRIBUTE((unused))) {
  switch (optid) {
    case 's':
      verbose = false;
      break;
    case 'V':
      print_version();
      exit(0);
    case 'I':
    case '?':
      usage();
      exit(0);
  }
  return false;
}

static int get_options(int *argc, char ***argv) {
  int ho_error;

  if ((ho_error = handle_options(argc, argv, my_long_options, get_one_option)))
    exit(ho_error);

  if (!*argc) {
    usage();
    return 1;
  }
  return 0;
} /* get_options */

static const char *get_ha_error_msg(int code) {
  /*
    If you got compilation error here, check that every HA_ERR_xxx constant
    has a corresponding error message in handler_error_messages[] list
    (check mysys/my_handler_errors.h and include/my_base.h).
  */
  static_assert(
      HA_ERR_FIRST + array_elements(handler_error_messages) == HA_ERR_LAST + 1,
      "Wrong number of elements in handler_error_messages.");
  if (code >= HA_ERR_FIRST && code <= HA_ERR_LAST)
    return handler_error_messages[code - HA_ERR_FIRST];

  return NullS;
}

typedef struct {
  const char *name;
  uint code;
  const char *text;
  /* SQLSTATE */
  const char *odbc_state;
  const char *jdbc_state;
  uint error_index;
} st_error;

static st_error global_error_names[] = {
#include <mysqld_ername.h>

    {nullptr, 0, nullptr, nullptr, nullptr, 0}};

/**
  Lookup an error by code in the global_error_names array.
  @param code the code to lookup
  @param [out] name_ptr the error name, when found
  @param [out] msg_ptr the error text, when found
  @return 1 when found, otherwise 0
*/
int get_ER_error_msg_by_code(uint code, const char **name_ptr,
                             const char **msg_ptr) {
  st_error *tmp_error;

  /* handle "global errors" */
  if ((code >= EE_ERROR_FIRST) && (code <= EE_ERROR_LAST)) {
    *name_ptr = nullptr;
    *msg_ptr = EE(code);
    return 1;
  }

  tmp_error = &global_error_names[0];

  while (tmp_error->name != nullptr) {
    if (tmp_error->code == code) {
      *name_ptr = tmp_error->name;
      *msg_ptr = tmp_error->text;
      return 1;
    }
    tmp_error++;
  }

  return 0;
}

/**
  Lookup an error by symbol in the global_error_names array.
  @param symbol the symbol to lookup
  @return code >0 when found, otherwise 0
*/
int get_ER_error_msg_by_symbol(const char *symbol) {
  st_error *tmp_error = &global_error_names[0];

  while (tmp_error->name != nullptr) {
    if (0 == strcmp(tmp_error->name, symbol)) return tmp_error->code;
    tmp_error++;
  }

  return 0;
}

#if defined(_WIN32)
static bool print_win_error_msg(DWORD error, bool verbose) {
  LPTSTR s;
  if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL, error, 0, (LPTSTR)&s, 0, NULL)) {
    if (verbose)
      printf("Win32 error code %d: %s", error, s);
    else
      puts(s);
    LocalFree(s);
    return 0;
  }
  return 1;
}
#endif

static const char *get_handler_error_message(int nr) {
  return handler_error_messages[nr - HA_ERR_FIRST];
}

/*
  Register handler error messages for usage with my_error()

  NOTES
    This is safe to call multiple times as my_error_register()
    will ignore calls to register already registered error numbers.
*/

void my_handler_error_register() {
  /*
    If you got compilation error here, check that every HA_ERR_xxx constant has
    a corresponding error message in handler_error_messages[] list
    (check mysys/ma_handler_errors.h and include/my_base.h).
  */
  static_assert(
      HA_ERR_FIRST + array_elements(handler_error_messages) == HA_ERR_LAST + 1,
      "Wrong number of elements in handler_error_messages.");
  my_error_register(get_handler_error_message, HA_ERR_FIRST,
                    HA_ERR_FIRST + array_elements(handler_error_messages) - 1);
}

void my_handler_error_unregister(void) {
  my_error_unregister(
      HA_ERR_FIRST, HA_ERR_FIRST + array_elements(handler_error_messages) - 1);
}

int main(int argc, char *argv[]) {
  int error, code, found;
  const char *msg;
  const char *name;
  char *unknown_error = nullptr;
#if defined(_WIN32)
  bool skip_win_message = 0;
#endif
  MY_INIT(argv[0]);

  if (get_options(&argc, &argv)) exit(1);

  my_handler_error_register();

  error = 0;
  {
    /*
      On some system, like Linux, strerror(unknown_error) returns a
      string 'Unknown Error'.  To avoid printing it we try to find the
      error string by asking for an impossible big error message.

      On Solaris 2.8 it might return NULL
    */
    if ((msg = strerror(10000)) == nullptr) msg = "Unknown Error";

    /*
      Allocate a buffer for unknown_error since strerror always returns
      the same pointer on some platforms such as Windows
    */
    unknown_error = static_cast<char *>(malloc(strlen(msg) + 1));
    my_stpcpy(unknown_error, msg);

    for (; argc-- > 0; argv++) {
      found = 0;

      code = atoi(*argv);

      if ((*argv != nullptr) && (strlen(*argv) > 3)) {
        if (0 == strncmp(*argv, "MY-", 3))
          code = atoi(((char *)*argv) + 3);
        else if (0 == strncmp(*argv, "ER_", 3))
          code = get_ER_error_msg_by_symbol((char *)*argv);
      }

      msg = strerror(code);

      /*
        We don't print the OS error message if it is the same as the
        unknown_error message we retrieved above, or it starts with
        'Unknown Error' (without regard to case).
      */
      if (msg &&
          my_strnncoll(&my_charset_latin1, (const uchar *)msg, 13,
                       (const uchar *)"Unknown Error", 13) &&
          (!unknown_error || strcmp(msg, unknown_error))) {
        found = 1;
        if (verbose)
          printf("OS error code %3d:  %s\n", code, msg);
        else
          puts(msg);
      }
      if ((msg = get_ha_error_msg(code))) {
        found = 1;
        if (verbose)
          printf("MySQL error code MY-%06d (handler): %s\n", code, msg);
        else
          puts(msg);
      }
      if (get_ER_error_msg_by_code(code, &name, &msg)) {
        found = 1;
        if (verbose) {
          if (name != nullptr)
            printf("MySQL error code MY-%06d (%s): %s\n", code, name, msg);
          else
            printf("MySQL error code MY-%06d: %s\n", code, msg);
        } else
          puts(msg);
      }
      if (!found) {
#if defined(_WIN32)
        if (!(skip_win_message = !print_win_error_msg((DWORD)code, verbose))) {
#endif
          fprintf(stderr, "Illegal error code: %d\n", code);
          error = 1;
#if defined(_WIN32)
        }
#endif
      }
#if defined(_WIN32)
      if (!skip_win_message) print_win_error_msg((DWORD)code, verbose);
#endif
    }
  }

  /* if we allocated a buffer for unknown_error, free it now */
  if (unknown_error) free(unknown_error);

  exit(error);
}
