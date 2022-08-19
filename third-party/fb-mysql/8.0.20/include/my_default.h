/* Copyright (c) 2012, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MY_DEFAULT_INCLUDED
#define MY_DEFAULT_INCLUDED

/**
  @file include/my_default.h
*/

#include <sys/types.h>

#include "my_inttypes.h"
#include "my_macros.h"

struct MEM_ROOT;

extern const char *my_defaults_extra_file;
extern const char *my_defaults_group_suffix;
extern const char *my_defaults_file;
extern bool my_getopt_use_args_separator;
extern bool my_defaults_read_login_file;
extern bool no_defaults;
extern char datadir_buffer[];

/* Define the type of function to be passed to process_default_option_files */
typedef int (*Process_option_func)(void *ctx, const char *group_name,
                                   const char *option, const char *cnf_file);
void set_persist_args_separator(char **arg);
bool my_getopt_is_args_separator(const char *arg);
bool my_getopt_is_ro_persist_args_separator(const char *arg);
int get_defaults_options(int argc, char **argv, char **defaults,
                         char **extra_defaults, char **group_suffix,
                         char **login_path, bool found_no_defaults);

// extern "C" since it is an (undocumented) part of the libmysql ABI.
extern "C" int my_load_defaults(const char *conf_file, const char **groups,
                                int *argc, char ***argv, MEM_ROOT *alloc,
                                const char ***);
int check_file_permissions(const char *file_name, bool is_login_file);
int load_defaults(const char *conf_file, const char **groups, int *argc,
                  char ***argv, MEM_ROOT *alloc);
int my_search_option_files(const char *conf_file, int *argc, char ***argv,
                           uint *args_used, Process_option_func func,
                           void *func_ctx, const char **default_directories,
                           bool is_login_file, bool found_no_defaults);
void my_print_default_files(const char *conf_file);
void print_defaults(const char *conf_file, const char **groups);
void init_variable_default_paths();
void update_variable_source(const char *opt_name, const char *config_file);
void set_variable_source(const char *opt_name, void *value);

#endif  // MY_DEFAULT_INCLUDED
