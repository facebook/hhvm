
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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

// Print all parameters in a default file that will be given to some program.

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "m_string.h"
#include "my_alloc.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_default.h"
#include "my_getopt.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "print_version.h"
#include "welcome_copyright_notice.h" /* ORACLE_WELCOME_COPYRIGHT_NOTICE */

const char *config_file = "my"; /* Default config file */
static char *my_login_path;
static bool *show_passwords;
uint verbose = 0, opt_defaults_file_used = 0;
const char *default_dbug_option = "d:t:o,/tmp/my_print_defaults.trace";

static struct my_option my_long_options[] = {
    /*
      NB: --config-file is troublesome, because get_defaults_options() doesn't
      know about it, but we pretend --config-file is like --defaults-file.  In
      fact they behave differently: see the comments at the top of
      mysys/default.c for how --defaults-file should behave.

      This --config-file option behaves as:
      - If it has a directory name part (absolute or relative), then only this
        file is read; no error is given if the file doesn't exist
      - If the file has no directory name part, the standard locations are
        searched for a file of this name (and standard filename extensions are
        added if the file has no extension)
    */
    {"config-file", 'c',
     "Deprecated, please use --defaults-file instead. "
     "Name of config file to read; if no extension is given, default "
     "extension (e.g., .ini or .cnf) will be added",
     &config_file, &config_file, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
#ifdef DBUG_OFF
    {"debug", '#', "This is a non-debug version. Catch this and exit", 0, 0, 0,
     GET_DISABLED, OPT_ARG, 0, 0, 0, 0, 0, 0},
#else
    {"debug", '#', "Output debug log", &default_dbug_option,
     &default_dbug_option, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
#endif
    {"defaults-file", 'c',
     "Like --config-file, except: if first option, "
     "then read this file only, do not read global or per-user config "
     "files; should be the first option",
     &config_file, &config_file, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"defaults-extra-file", 'e',
     "Read this file after the global config file and before the config "
     "file in the users home directory; should be the first option",
     &my_defaults_extra_file, &my_defaults_extra_file, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"defaults-group-suffix", 'g',
     "In addition to the given groups, read also groups with this suffix",
     &my_defaults_group_suffix, &my_defaults_group_suffix, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"extra-file", 'e', "Deprecated. Synonym for --defaults-extra-file.",
     &my_defaults_extra_file, &my_defaults_extra_file, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"no-defaults", 'n',
     "Ignore reading of default option file(s), "
     "except for login file.",
     nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"login-path", 'l', "Path to be read from under the login file.",
     &my_login_path, &my_login_path, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"show", 's', "Show passwords in plain text.", &show_passwords,
     &show_passwords, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"help", '?', "Display this help message and exit.", nullptr, nullptr,
     nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"verbose", 'v', "Increase the output level", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"version", 'V', "Output version information and exit.", nullptr, nullptr,
     nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

static void usage(bool version) {
  print_version();
  if (version) return;
  puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2000"));
  puts(
      "Prints all arguments that is give to some program using the default "
      "files");
  printf("Usage: %s [OPTIONS] groups\n", my_progname);
  my_print_help(my_long_options);
  my_print_default_files(config_file);
  my_print_variables(my_long_options);
  printf("\nExample usage:\n%s --defaults-file=example.cnf client mysql\n",
         my_progname);
}

static bool get_one_option(int optid,
                           const struct my_option *opt MY_ATTRIBUTE((unused)),
                           char *argument MY_ATTRIBUTE((unused))) {
  switch (optid) {
    case 'c':
      opt_defaults_file_used = 1;
      break;
    case 'n':
      break;
    case 'I':
    case '?':
      usage(false);
      exit(0);
    case 'v':
      verbose++;
      break;
    case 'V':
      usage(true);
      exit(0);
    case '#':
      DBUG_PUSH(argument ? argument : default_dbug_option);
      break;
  }
  return false;
}

static int get_options(int *argc, char ***argv) {
  int ho_error;

  if ((ho_error = handle_options(argc, argv, my_long_options, get_one_option)))
    exit(ho_error);

  if (*argc < 1) {
    usage(false);
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  int count, error, args_used;
  const char **load_default_groups;
  char *tmp_arguments[6];
  char **argument, **arguments, **org_argv;
  char *defaults, *extra_defaults, *group_suffix, *login_path;

  MY_INIT(argv[0]);

  org_argv = argv;
  args_used = get_defaults_options(argc, argv, &defaults, &extra_defaults,
                                   &group_suffix, &login_path, false);

  /* Copy defaults-xxx arguments & program name */
  count = args_used + 1;
  arguments = tmp_arguments;
  memcpy((char *)arguments, (char *)org_argv, count * sizeof(*org_argv));
  arguments[count] = nullptr;

  /* Check out the args */
  if (!(load_default_groups = static_cast<const char **>(my_malloc(
            PSI_NOT_INSTRUMENTED, (argc + 1) * sizeof(char *), MYF(MY_WME)))))
    exit(1);
  if (get_options(&argc, &argv)) exit(1);
  memcpy((char *)load_default_groups, (char *)argv, (argc + 1) * sizeof(*argv));

  MEM_ROOT alloc{PSI_NOT_INSTRUMENTED, 512};
  if ((error = load_defaults(config_file, load_default_groups, &count,
                             &arguments, &alloc))) {
    if (verbose && opt_defaults_file_used) {
      if (error == 1)
        fprintf(stderr, "WARNING: Defaults file '%s' not found!\n",
                config_file);
      /* This error is not available now. For the future */
      if (error == 2)
        fprintf(stderr, "WARNING: Defaults file '%s' is not a regular file!\n",
                config_file);
    }
    error = 2;
    exit(error);
  }

  for (argument = arguments + 1; *argument; argument++)
    if (!my_getopt_is_args_separator(*argument)) /* skip arguments separator */
    {
      if (!(show_passwords) && strncmp(*argument, "--password", 10) == 0)
        puts("--password=*****");
      else
        puts(*argument);
    }
  my_free(load_default_groups);

  exit(0);
}
