/*
   Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _my_getopt_h
#define _my_getopt_h

/**
  @file include/my_getopt.h
*/

#include <stdio.h>
#include <sys/types.h>

#include <mysql/components/services/system_variable_source_type.h> /* enum_variable_source */
#include "my_config.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_sys.h" /* loglevel */

#define GET_NO_ARG 1
#define GET_BOOL 2
#define GET_INT 3
#define GET_UINT 4
#define GET_LONG 5
#define GET_ULONG 6
#define GET_LL 7
#define GET_ULL 8
#define GET_STR 9
#define GET_STR_ALLOC 10
#define GET_DISABLED 11
#define GET_ENUM 12
#define GET_SET 13
#define GET_DOUBLE 14
#define GET_FLAGSET 15
#define GET_PASSWORD 16

#if SIZEOF_INT == 4
#define GET_INT32 GET_INT
#define GET_UINT32 GET_UINT
#elif SIZEOF_LONG == 4
#define GET_INT32 GET_LONG
#define GET_UINT32 GET_ULONG
#else
#error Neither int or long is of 4 bytes width
#endif

#define GET_ASK_ADDR 128
#define GET_TYPE_MASK 127

/**
  Enumeration of the my_option::arg_type attributes.
  It should be noted that for historical reasons variables with the combination
  arg_type=NO_ARG, my_option::var_type=GET_BOOL still accepts
  arguments. This is someone counter intuitive and care should be taken
  if the code is refactored.
*/
enum get_opt_arg_type { NO_ARG, OPT_ARG, REQUIRED_ARG };

struct get_opt_arg_source {
  /**
    config file path OR compiled default values
  */
  char m_path_name[FN_REFLEN];
  enum enum_variable_source m_source;
};

struct TYPELIB;

struct my_option {
  const char *name;               /**< Name of the option. name=NULL
                                     marks the end of the my_option[]
                                     array.
                                   */
  int id;                         /**< For 0<id<=255 it's means one
                                     character for a short option
                                     (like -A), if >255 no short option
                                     is created, but a long option still
                                     can be identified uniquely in the
                                     my_get_one_option() callback.
                                     If an opton needs neither special
                                     treatment in the my_get_one_option()
                                     nor one-letter short equivalent
                                     use id=0.
                                     id=-1 is a special case and is used
                                     to generate deprecation warnings for
                                     plugin options. It should not be
                                     used for anything else.
                                   */
  const char *comment;            /**< option comment, for autom. --help.
                                     if it's NULL the option is not
                                     visible in --help.
                                   */
  void *value;                    /**< A pointer to the variable value */
  void *u_max_value;              /**< The user def. max variable value */
  TYPELIB *typelib;               /**< Pointer to possible values */
  ulong var_type;                 /**< GET_BOOL, GET_ULL, etc */
  enum get_opt_arg_type arg_type; /**< e.g. REQUIRED_ARG or OPT_ARG */
  longlong def_value;             /**< Default value */
  longlong min_value;             /**< Min allowed value (for numbers) */
  ulonglong max_value;            /**< Max allowed value (for numbers) */
  struct get_opt_arg_source *arg_source; /**< Represents source/path from where
                                            this variable is set. */
  long block_size; /**< Value should be a mult. of this (for numbers) */
  void *app_type;  /**< To be used by an application */
};

typedef bool (*my_get_one_option)(int, const struct my_option *, char *);
/**
  Used to retrieve a reference to the object (variable) that holds the value
  for the given option. For example, if var_type is GET_UINT, the function
  must return a pointer to a variable of type uint. A argument is stored in
  the location pointed to by the returned pointer.
*/
typedef void *(*my_getopt_value)(const char *, size_t, const struct my_option *,
                                 int *);

extern char *disabled_my_option;
extern bool my_getopt_print_errors;
extern bool my_getopt_skip_unknown;
extern bool log_slave_updates_supplied;
extern bool slave_preserve_commit_order_supplied;
extern my_error_reporter my_getopt_error_reporter;

extern longlong getopt_ll(const char *arg, const struct my_option *optp,
                          int *err);
extern ulonglong getopt_ull(const char *, const struct my_option *, int *);
extern double getopt_double(const char *arg, const struct my_option *optp,
                            int *err);
extern bool get_bool_argument(const char *argument, bool *error);

extern "C" int handle_options(int *argc, char ***argv,
                              const struct my_option *longopts,
                              my_get_one_option);
extern int my_handle_options(int *argc, char ***argv,
                             const struct my_option *longopts,
                             my_get_one_option, const char **command_list,
                             bool ignore_unknown_option);
extern void my_cleanup_options(const struct my_option *options);
extern void my_print_help(const struct my_option *options);
extern void my_print_variables(const struct my_option *options);
extern void my_print_variables_ex(const struct my_option *options, FILE *file);
extern void my_getopt_register_get_addr(my_getopt_value);

ulonglong getopt_ull_limit_value(ulonglong num, const struct my_option *optp,
                                 bool *fix);
longlong getopt_ll_limit_value(longlong, const struct my_option *, bool *fix);
double getopt_double_limit_value(double num, const struct my_option *optp,
                                 bool *fix);
ulonglong max_of_int_range(int var_type);

ulonglong getopt_double2ulonglong(double);
double getopt_ulonglong2double(ulonglong);
int findopt(const char *, uint, const struct my_option **);

bool is_key_cache_variable_suffix(const char *suffix);

bool get_bool_argument(const char *argument, bool *error);
// Declared here, so we can unit test it.
template <typename LLorULL>
LLorULL eval_num_suffix(const char *argument, int *error,
                        const char *option_name);

#endif /* _my_getopt_h */
