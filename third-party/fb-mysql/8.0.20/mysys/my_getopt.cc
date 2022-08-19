/* Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_getopt.cc
*/

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <algorithm>
#include <array>
#include <bitset>
#include <type_traits>

#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_default.h"
#include "my_getopt.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_version.h"  // MYSQL_PERSIST_CONFIG_NAME
#include "mysys/mysys_priv.h"
#include "mysys_err.h"
#include "typelib.h"

typedef void (*init_func_p)(const struct my_option *option, void *variable,
                            longlong value);

my_error_reporter my_getopt_error_reporter = &my_message_local;

longlong getopt_ll(const char *arg, const struct my_option *optp, int *err);
ulonglong getopt_ull(const char *, const struct my_option *, int *);
double getopt_double(const char *arg, const struct my_option *optp, int *err);
bool get_bool_argument(const char *argument, bool *error);

static bool getopt_compare_strings(const char *, const char *, uint);
static void init_variables(const struct my_option *, init_func_p);
static void init_one_value(const struct my_option *, void *, longlong);
static void fini_one_value(const struct my_option *, void *, longlong);
static int setval(const struct my_option *, void *, const char *, bool);
static void setval_source(const struct my_option *, void *);
static char *check_struct_option(char *cur_arg, char *key_name);

/*
  The following three variables belong to same group and the number and
  order of their arguments must correspond to each other.
*/
static const char *special_opt_prefix[] = {"skip",    "disable", "enable",
                                           "maximum", "loose",   nullptr};
static const uint special_opt_prefix_lengths[] = {4, 7, 6, 7, 5, 0};
enum enum_special_opt {
  OPT_SKIP,
  OPT_DISABLE,
  OPT_ENABLE,
  OPT_MAXIMUM,
  OPT_LOOSE
};

char *disabled_my_option = const_cast<char *>("0");
static char enabled_my_option[] = "1";
static char space_char[] = " ";

/*
   This is a flag that can be set in client programs. false means that
   my_getopt will not print error messages, but the client should do
   it by itself
*/

bool my_getopt_print_errors = true;

/*
   This is a flag that can be set in client programs. true means that
   my_getopt will skip over options it does not know how to handle.
*/

bool my_getopt_skip_unknown = false;

static my_getopt_value getopt_get_addr;

void my_getopt_register_get_addr(my_getopt_value func_addr) {
  getopt_get_addr = func_addr;
}

bool is_key_cache_variable_suffix(const char *suffix) {
  static std::array<const char *, 4> key_cache_components = {
      {"key_buffer_size", "key_cache_block_size", "key_cache_division_limit",
       "key_cache_age_threshold"}};

  for (auto component : key_cache_components)
    if (!my_strcasecmp(&my_charset_latin1, component, suffix)) return true;

  return false;
}

/**
  Wrapper around my_handle_options() for interface compatibility.

  @param [in,out] argc       Command line options (count)
  @param [in,out] argv       Command line options (values)
  @param [in] longopts       Descriptor of all valid options
  @param [in] get_one_option Optional callback function to process each option,
                             can be NULL.

  @return Error in case of ambiguous or unknown options,
          0 on success.
*/
int handle_options(int *argc, char ***argv, const struct my_option *longopts,
                   my_get_one_option get_one_option) {
  return my_handle_options(argc, argv, longopts, get_one_option, nullptr,
                           false);
}

union ull_dbl {
  ulonglong ull;
  double dbl;
};

/**
  Returns an ulonglong value containing a raw
  representation of the given double value.
*/
ulonglong getopt_double2ulonglong(double v) {
  union ull_dbl u;
  u.dbl = v;
  static_assert(sizeof(ulonglong) >= sizeof(double), "");
  return u.ull;
}

/**
  Returns the double value which corresponds to
  the given raw representation.
*/
double getopt_ulonglong2double(ulonglong v) {
  union ull_dbl u;
  u.ull = v;
  return u.dbl;
}

/**
  Handle command line options.
  Sort options.
  Put options first, until special end of options (--),
  or until the end of argv. Parse options, check that the given option
  matches with one of the options in struct 'my_option'.
  Check that option was given an argument if it requires one
  Call the optional 'get_one_option()' function once for each option.

  Note that handle_options() can be invoked multiple times to
  parse a command line in several steps.
  In this case, use the global flag @c my_getopt_skip_unknown to indicate
  that options unknown in the current step should be preserved in the
  command line for later parsing in subsequent steps.

  For 'long' options (--a_long_option), @c my_getopt_skip_unknown is
  fully supported. Command line parameters such as:
  - "--a_long_option"
  - "--a_long_option=value"
  - "--a_long_option value"
  will be preserved as is when the option is not known.

  For 'short' options (-S), support for @c my_getopt_skip_unknown
  comes with some limitation, because several short options
  can also be specified together in the same command line argument,
  as in "-XYZ".

  The first use case supported is: all short options are declared.
  handle_options() will be able to interpret "-XYZ" as one of:
  - an unknown X option
  - "-X -Y -Z", three short options with no arguments
  - "-X -YZ", where Y is a short option with argument Z
  - "-XYZ", where X is a short option with argument YZ
  based on the full short options specifications.

  The second use case supported is: no short option is declared.
  handle_options() will reject "-XYZ" as unknown, to be parsed later.

  The use case that is explicitly not supported is to provide
  only a partial list of short options to handle_options().
  This function can not be expected to extract some option Y
  in the middle of the string "-XYZ" in these conditions,
  without knowing if X will be declared an option later.

  Note that this limitation only impacts parsing of several
  short options from the same command line argument,
  as in "mysqld -anW5".
  When each short option is properly separated out in the command line
  argument, for example in "mysqld -a -n -w5", the code would actually
  work even with partial options specs given at each stage.

  @param [in, out] argc      command line options (count)
  @param [in, out] argv      command line options (values)
  @param [in] longopts       descriptor of all valid options
  @param [in] get_one_option optional callback function to process each option,
                             can be NULL.
  @param [in] command_list   NULL-terminated list of strings (commands) which
                             (if set) is looked up for all non-option strings
                             found while parsing the command line parameters.
                             The parsing terminates if a match is found. At
                             exit, argv [out] would contain all the remaining
                             unparsed options along with the matched command.

  @param [in] ignore_unknown_option When set to true, options are continued to
                                    be read even when unknown options are
                                    encountered.

  @return error in case of ambiguous or unknown options,
          0 on success.
*/
int my_handle_options(int *argc, char ***argv, const struct my_option *longopts,
                      my_get_one_option get_one_option,
                      const char **command_list, bool ignore_unknown_option) {
  uint argvpos = 0, length;
  bool end_of_options = false, must_be_var, set_maximum_value, option_is_loose;
  char **pos, **pos_end, *optend, *opt_str, key_name[FN_REFLEN];
  char **arg_sep = nullptr, **persist_arg_sep = nullptr;
  const struct my_option *optp;
  void *value;
  bool is_cmdline_arg = true, is_persist_arg = true;
  int opt_found;

  /* handle_options() assumes arg0 (program name) always exists */
  DBUG_ASSERT(argc && *argc >= 1);
  DBUG_ASSERT(argv && *argv);
  (*argc)--; /* Skip the program name */
  (*argv)++; /*      --- || ----      */
  init_variables(longopts, init_one_value);

  /*
    Search for args_separator, if found, then the first part of the
    arguments are loaded from configs
  */
  for (pos = *argv, pos_end = pos + *argc; pos != pos_end; pos++) {
    if (my_getopt_is_args_separator(*pos)) {
      arg_sep = pos;
      is_cmdline_arg = false;
      break;
    }
  }
  /* search for persist_args_separator */
  if (arg_sep) {
    for (pos = arg_sep, pos_end = (*argv + *argc); pos != pos_end; pos++) {
      if (my_getopt_is_ro_persist_args_separator(*pos)) {
        persist_arg_sep = pos;
        is_persist_arg = false;
        break;
      }
    }
  }
  if (arg_sep) {
    /*
      All options which are between arg_sep and persist_arg_sep are
      command line options, thus update the variables_hash with these
      options. If persist_arg_sep is NULL then it means there are no
      read only persist options, what follows is only command line options.
    */
    pos = arg_sep + 1;
    while (*pos && pos != persist_arg_sep) {
      update_variable_source((const char *)*pos, nullptr);
      ++pos;
    }
  }
  if (persist_arg_sep) {
    /*
      All options which are between after persist_arg_sep are
      read from persistent file, thus update the variables_hash with
      these options with path set to "$datadir/mysqld-auto.cnf".
    */
    pos = persist_arg_sep + 1;
    char persist_dir[FN_REFLEN] = {0};
    fn_format(persist_dir, MYSQL_PERSIST_CONFIG_NAME, datadir_buffer, ".cnf",
              MY_UNPACK_FILENAME | MY_SAFE_PATH | MY_RELATIVE_PATH);
    while (pos && *pos) {
      update_variable_source((const char *)*pos, persist_dir);
      ++pos;
    }
  }
  for (pos = *argv, pos_end = pos + *argc; pos != pos_end; pos++) {
    char **first = pos;
    char *cur_arg = *pos;
    opt_found = false;
    if (!is_cmdline_arg && (my_getopt_is_args_separator(cur_arg))) {
      is_cmdline_arg = true;

      /* save the separator too if skip unkown options  */
      if (my_getopt_skip_unknown)
        (*argv)[argvpos++] = cur_arg;
      else
        (*argc)--;
      continue;
    }
    /* skip persist args separator */
    if (!is_persist_arg && my_getopt_is_ro_persist_args_separator(cur_arg)) {
      is_persist_arg = true;
      if (my_getopt_skip_unknown)
        (*argv)[argvpos++] = cur_arg;
      else
        (*argc)--;
      continue;
    }
    if (cur_arg[0] == '-' && cur_arg[1] && !end_of_options) /* must be opt */
    {
      char *argument = nullptr;
      must_be_var = false;
      set_maximum_value = false;
      option_is_loose = false;

      cur_arg++;           /* skip '-' */
      if (*cur_arg == '-') /* check for long option, */
      {
        if (!*++cur_arg) /* skip the double dash */
        {
          /* '--' means end of options, look no further */
          end_of_options = true;
          (*argc)--;
          continue;
        }
        opt_str = check_struct_option(cur_arg, key_name);
        optend = const_cast<char *>(strcend(opt_str, '='));
        length = (uint)(optend - opt_str);
        if (*optend == '=')
          optend++;
        else
          optend = nullptr;

        /*
         * For component system variables key_name is the component name and
         * opt_str is the variable_name. For structured system variables
         * opt_str will have key_cache_**** and key_name is the variable
         * instance name And for all other variable key_name will be 0.
         */
        if (*key_name) {
          std::string tmp_name(opt_str, 0, length);

          if (!is_key_cache_variable_suffix(tmp_name.c_str())) {
            opt_str = cur_arg;
            length = (uint)((optend - opt_str) - 1);
          }
        }
        /*
          Find first the right option. Return error in case of an ambiguous,
          or unknown option
        */
        optp = longopts;
        if (!(opt_found = findopt(opt_str, length, &optp))) {
          /*
            Didn't find any matching option. Let's see if someone called
            option with a special option prefix
          */
          if (!must_be_var) {
            if (optend)
              must_be_var = true; /* option is followed by an argument */
            for (int i = 0; special_opt_prefix[i]; i++) {
              if (!getopt_compare_strings(special_opt_prefix[i], opt_str,
                                          special_opt_prefix_lengths[i]) &&
                  (opt_str[special_opt_prefix_lengths[i]] == '-' ||
                   opt_str[special_opt_prefix_lengths[i]] == '_')) {
                /*
                  We were called with a special prefix, we can reuse opt_found
                */
                opt_str += special_opt_prefix_lengths[i] + 1;
                length -= special_opt_prefix_lengths[i] + 1;
                if (i == OPT_LOOSE) option_is_loose = true;
                if ((opt_found = findopt(opt_str, length, &optp))) {
                  switch (i) {
                    case OPT_SKIP:
                    case OPT_DISABLE: /* fall through */
                      /*
                        double negation is actually enable again,
                        for example: --skip-option=0 -> option = true
                      */
                      optend = (optend && *optend == '0' && !(*(optend + 1)))
                                   ? enabled_my_option
                                   : disabled_my_option;
                      break;
                    case OPT_ENABLE:
                      optend = (optend && *optend == '0' && !(*(optend + 1)))
                                   ? disabled_my_option
                                   : enabled_my_option;
                      break;
                    case OPT_MAXIMUM:
                      set_maximum_value = true;
                      must_be_var = true;
                      break;
                  }
                  break; /* break from the inner loop, main loop continues */
                }
                i = -1; /* restart the loop */
              }
            }
          }
          if (!opt_found) {
            if (my_getopt_skip_unknown) {
              /* Preserve all the components of this unknown option. */
              do {
                (*argv)[argvpos++] = *first++;
              } while (first <= pos);
              continue;
            }
            if (must_be_var) {
              if (my_getopt_print_errors)
                my_getopt_error_reporter(
                    option_is_loose ? WARNING_LEVEL : ERROR_LEVEL,
                    EE_UNKNOWN_VARIABLE, cur_arg);
              if (!option_is_loose && !ignore_unknown_option)
                return EXIT_UNKNOWN_VARIABLE;
            } else {
              if (my_getopt_print_errors)
                my_getopt_error_reporter(
                    option_is_loose ? WARNING_LEVEL : ERROR_LEVEL,
                    EE_UNKNOWN_OPTION, cur_arg);
              if (!(option_is_loose || ignore_unknown_option))
                return EXIT_UNKNOWN_OPTION;
            }
            if (option_is_loose || ignore_unknown_option) {
              (*argc)--;
              continue;
            }
          }
        }
        if ((optp->var_type & GET_TYPE_MASK) == GET_DISABLED) {
          if (my_getopt_print_errors)
            my_message_local(option_is_loose ? WARNING_LEVEL : ERROR_LEVEL,
                             EE_USING_DISABLED_OPTION, my_progname, opt_str);
          if (option_is_loose) {
            (*argc)--;
            continue;
          }
          return EXIT_OPTION_DISABLED;
        }
        {
          int error = 0;
          value =
              optp->var_type & GET_ASK_ADDR
                  ? (*getopt_get_addr)(key_name, strlen(key_name), optp, &error)
                  : optp->value;
          if (error) return error;
        }
        if (optp->arg_type == NO_ARG) {
          /*
            Due to historical reasons GET_BOOL var_types still accepts arguments
            despite the NO_ARG arg_type attribute. This can seems a bit
            unintuitive and care should be taken when refactoring this code.
          */
          if (optend && (optp->var_type & GET_TYPE_MASK) != GET_BOOL) {
            if (my_getopt_print_errors)
              my_getopt_error_reporter(ERROR_LEVEL, EE_OPTION_WITHOUT_ARGUMENT,
                                       my_progname, optp->name);
            return EXIT_NO_ARGUMENT_ALLOWED;
          }
          if ((optp->var_type & GET_TYPE_MASK) == GET_BOOL) {
            /*
              Set bool to true if no argument or if the user has used
              --enable-'option-name'.
              *optend was set to '0' if one used --disable-option
            */
            (*argc)--;
            if (!optend)
              *((bool *)value) = true;
            else {
              bool ret = false;
              bool error = false;
              ret = get_bool_argument(optend, &error);
              if (error) {
                my_getopt_error_reporter(WARNING_LEVEL,
                                         EE_OPTION_IGNORED_DUE_TO_INVALID_VALUE,
                                         my_progname, optp->name, optend);
                continue;
              } else
                *((bool *)value) = ret;
            }
            if (get_one_option &&
                get_one_option(
                    optp->id, optp,
                    *((bool *)value) ? enabled_my_option : disabled_my_option))
              return EXIT_ARGUMENT_INVALID;
            /* set variables source */
            setval_source(optp, (void *)optp->arg_source);
            continue;
          }
          argument = optend;
        } else if (optp->arg_type == REQUIRED_ARG && !optend) {
          /*
             Check if there are more arguments after this one,
             Note: options loaded from config file that requires value
             should always be in the form '--option=value'.
           */
          if (!is_cmdline_arg || !*++pos) {
            if (my_getopt_print_errors)
              my_getopt_error_reporter(ERROR_LEVEL, EE_OPTION_REQUIRES_ARGUMENT,
                                       my_progname, optp->name);
            return EXIT_ARGUMENT_REQUIRED;
          }
          argument = *pos;
          (*argc)--;
        } else
          argument = optend;

      } else /* must be short option */
      {
        for (optend = cur_arg; *optend; optend++) {
          opt_found = false;
          for (optp = longopts; optp->name; optp++) {
            if (optp->id && optp->id == (int)(uchar)*optend) {
              /* Option recognized. Find next what to do with it */
              opt_found = true;
              if ((optp->var_type & GET_TYPE_MASK) == GET_DISABLED) {
                if (my_getopt_print_errors)
                  my_message_local(ERROR_LEVEL, EE_USING_DISABLED_SHORT_OPTION,
                                   my_progname, optp->id);
                return EXIT_OPTION_DISABLED;
              }
              if ((optp->var_type & GET_TYPE_MASK) == GET_BOOL &&
                  optp->arg_type == NO_ARG) {
                *((bool *)optp->value) = true;
                if (get_one_option && get_one_option(optp->id, optp, argument))
                  return EXIT_UNSPECIFIED_ERROR;
                continue;
              } else if (optp->arg_type == REQUIRED_ARG ||
                         optp->arg_type == OPT_ARG) {
                if (*(optend + 1)) {
                  /* The rest of the option is option argument */
                  argument = optend + 1;
                  /* This is in effect a jump out of the outer loop */
                  optend = space_char;
                } else {
                  if (optp->arg_type == OPT_ARG) {
                    if (optp->var_type == GET_BOOL)
                      *((bool *)optp->value) = true;
                    if (get_one_option &&
                        get_one_option(optp->id, optp, argument))
                      return EXIT_UNSPECIFIED_ERROR;
                    continue;
                  }
                  /* Check if there are more arguments after this one */
                  if (!pos[1]) {
                    if (my_getopt_print_errors)
                      my_getopt_error_reporter(
                          ERROR_LEVEL, EE_SHORT_OPTION_REQUIRES_ARGUMENT,
                          my_progname, optp->id);
                    return EXIT_ARGUMENT_REQUIRED;
                  }
                  argument = *++pos;
                  (*argc)--;
                  /* the other loop will break, because *optend + 1 == 0 */
                }
              }
              int error;
              if ((error =
                       setval(optp, optp->value, argument, set_maximum_value)))
                return error;
              if (get_one_option && get_one_option(optp->id, optp, argument))
                return EXIT_UNSPECIFIED_ERROR;
              break;
            }
          }
          if (!opt_found) {
            if (my_getopt_skip_unknown) {
              /*
                We are currently parsing a single argv[] argument
                of the form "-XYZ".
                One or the argument found (say Y) is not an option.
                Hack the string "-XYZ" to make a "-YZ" substring in it,
                and push that to the output as an unrecognized parameter.
              */
              DBUG_ASSERT(optend > *pos);
              DBUG_ASSERT(optend >= cur_arg);
              DBUG_ASSERT(optend <= *pos + strlen(*pos));
              DBUG_ASSERT(*optend);
              optend--;
              optend[0] = '-'; /* replace 'X' or '-' by '-' */
              (*argv)[argvpos++] = optend;
              /*
                Do not continue to parse at the current "-XYZ" argument,
                skip to the next argv[] argument instead.
              */
              optend = space_char;
            } else {
              if (my_getopt_print_errors)
                my_getopt_error_reporter(ERROR_LEVEL, EE_UNKNOWN_SHORT_OPTION,
                                         my_progname, *optend);
              return EXIT_UNKNOWN_OPTION;
            }
          }
        }
        if (opt_found)
          (*argc)--; /* option handled (short), decrease argument count */
        continue;
      }
      int error;
      if ((error = setval(optp, value, argument, set_maximum_value)))
        return error;
      if (get_one_option && get_one_option(optp->id, optp, argument))
        return EXIT_UNSPECIFIED_ERROR;

      (*argc)--; /* option handled (long), decrease argument count */
    } else       /* non-option found */
    {
      if (command_list) {
        while (*command_list) {
          if (!strcmp(*command_list, cur_arg)) {
            /* Match found. */
            (*argv)[argvpos++] = cur_arg;

            /* Copy rest of the un-parsed elements & return. */
            while ((++pos) != pos_end) (*argv)[argvpos++] = *pos;
            goto done;
          }
          command_list++;
        }
      }
      (*argv)[argvpos++] = cur_arg;
    }
  }

done:
  /*
    Destroy the first, already handled option, so that programs that look
    for arguments in 'argv', without checking 'argc', know when to stop.
    Items in argv, before the destroyed one, are all non-option -arguments
    to the program, yet to be (possibly) handled.
  */
  (*argv)[argvpos] = nullptr;
  return 0;
}

/**
  @brief Check for struct options

  @param[in]   cur_arg     Current argument under processing from argv
  @param[in]   key_name    variable where to store the possible key name

  @details
  In case option is a struct option, returns a pointer to the current
  argument at the position where the struct option (key_name) ends, the
  next character after the dot. In case argument is not a struct option,
  returns a pointer to the argument.
  key_name will hold the name of the key, or 0 if not found.

  @return char*
  If struct option     Pointer to next character after dot.
  If no struct option  Pointer to the argument
*/

static char *check_struct_option(char *cur_arg, char *key_name) {
  char *dot_pos = const_cast<char *>(
      strcend(cur_arg + 1, '.')); /* Skip the first character */
  const char *equal_pos = strcend(cur_arg, '=');
  const char *space_pos = strcend(cur_arg, ' ');

  /*
     If the first dot is after an equal sign, then it is part
     of a variable value and the option is not a struct option.
     Also, if the last character in the string before the ending
     NULL, or the character right before equal sign is the first
     dot found, the option is not a struct option.
  */
  if ((equal_pos > dot_pos) && (space_pos > dot_pos)) {
    size_t len = std::min(size_t(dot_pos - cur_arg), size_t(FN_REFLEN - 1));
    strmake(key_name, cur_arg, len);
    return ++dot_pos;
  } else {
    key_name[0] = 0;
    return cur_arg;
  }
}

/**
   Parse a boolean command line argument

   "ON", "TRUE" and "1" will return true,
   other values will return false.

   @param argument The value argument
   @param [out] error Error indicator
   @return boolean value
*/
bool get_bool_argument(const char *argument, bool *error) {
  if (!my_strcasecmp(&my_charset_latin1, argument, "true") ||
      !my_strcasecmp(&my_charset_latin1, argument, "on") ||
      !my_strcasecmp(&my_charset_latin1, argument, "1"))
    return true;

  if (!my_strcasecmp(&my_charset_latin1, argument, "false") ||
      !my_strcasecmp(&my_charset_latin1, argument, "off") ||
      !my_strcasecmp(&my_charset_latin1, argument, "0"))
    return false;

  *error = true;
  return false;
}

/**
  Will set the source and file name from where this options is set in
  my_option struct.
*/
static void setval_source(const struct my_option *opts, void *value) {
  set_variable_source(opts->name, value);
}
/*
  function: setval

  Arguments: opts, argument
  Will set the option value to given value
*/

static int setval(const struct my_option *opts, void *value,
                  const char *argument, bool set_maximum_value) {
  int err = 0, res = 0;
  ulong var_type = opts->var_type & GET_TYPE_MASK;

  if (!argument) argument = enabled_my_option;

  /*
    Thus check applies only to options that have a defined value
    storage pointer.
    We do it for numeric types only, as empty value is a valid
    option for strings (the only way to reset back to default value).
    Note: it does not relate to OPT_ARG/REQUIRED_ARG/NO_ARG, since
    --param="" is not generally the same as --param.
    TODO: Add an option definition flag to signify whether empty value
    (i.e. --param="") is an acceptable value or an error and extend
    the check to all options.
  */
  if (!*argument &&
      (var_type == GET_INT || var_type == GET_UINT || var_type == GET_LONG ||
       var_type == GET_ULONG || var_type == GET_LL || var_type == GET_ULL ||
       var_type == GET_DOUBLE || var_type == GET_ENUM)) {
    my_getopt_error_reporter(ERROR_LEVEL, EE_OPTION_WITH_EMPTY_VALUE,
                             my_progname, opts->name);
    return EXIT_ARGUMENT_REQUIRED;
  }

  if (value) {
    if (set_maximum_value && !(value = opts->u_max_value)) {
      my_getopt_error_reporter(ERROR_LEVEL,
                               EE_FAILED_TO_ASSIGN_MAX_VALUE_TO_OPTION,
                               my_progname, opts->name);
      return EXIT_NO_PTR_TO_VARIABLE;
    }

    bool error = false;
    switch (var_type) {
      case GET_BOOL: /* If argument differs from 0, enable option, else disable
                      */
        *((bool *)value) = get_bool_argument(argument, &error);
        if (error)
          my_getopt_error_reporter(WARNING_LEVEL,
                                   EE_INCORRECT_BOOLEAN_VALUE_FOR_OPTION,
                                   opts->name, argument);
        break;
      case GET_INT:
        *((int *)value) = (int)getopt_ll(argument, opts, &err);
        break;
      case GET_UINT:
        *((uint *)value) = (uint)getopt_ull(argument, opts, &err);
        break;
      case GET_LONG:
        *((long *)value) = (long)getopt_ll(argument, opts, &err);
        break;
      case GET_ULONG:
        *((long *)value) = (long)getopt_ull(argument, opts, &err);
        break;
      case GET_LL:
        *((longlong *)value) = getopt_ll(argument, opts, &err);
        break;
      case GET_ULL:
        *((ulonglong *)value) = getopt_ull(argument, opts, &err);
        break;
      case GET_DOUBLE:
        *((double *)value) = getopt_double(argument, opts, &err);
        break;
      case GET_STR:
      case GET_PASSWORD:
        if (argument == enabled_my_option)
          break; /* string options don't use this default of "1" */
        *static_cast<const char **>(value) = argument;
        break;
      case GET_STR_ALLOC:
        if (argument == enabled_my_option)
          break; /* string options don't use this default of "1" */
        my_free(*((char **)value));
        if (!(*((char **)value) =
                  my_strdup(key_memory_defaults, argument, MYF(MY_WME)))) {
          res = EXIT_OUT_OF_MEMORY;
          goto ret;
        };
        break;
      case GET_ENUM: {
        int type = find_type(argument, opts->typelib, FIND_TYPE_BASIC);
        if (type == 0) {
          /*
            Accept an integer representation of the enumerated item.
          */
          char *endptr;
          ulong arg = strtoul(argument, &endptr, 10);
          if (*endptr || arg >= opts->typelib->count) {
            res = EXIT_ARGUMENT_INVALID;
            goto ret;
          }
          *(ulong *)value = arg;
        } else if (type < 0) {
          res = EXIT_AMBIGUOUS_OPTION;
          goto ret;
        } else
          *(ulong *)value = type - 1;
      } break;
      case GET_SET:
        *(static_cast<ulonglong *>(value)) =
            find_typeset(argument, opts->typelib, &err);
        if (err) {
          /* Accept an integer representation of the set */
          char *endptr;
          ulonglong arg = (ulonglong)strtol(argument, &endptr, 10);
          if (*endptr || (arg >> 1) >= (1ULL << (opts->typelib->count - 1))) {
            res = EXIT_ARGUMENT_INVALID;
            goto ret;
          };
          *static_cast<ulonglong *>(value) = arg;
          err = 0;
        }
        break;
      case GET_FLAGSET: {
        const char *flag_error;
        uint error_len;

        *(static_cast<ulonglong *>(value)) = find_set_from_flags(
            opts->typelib, opts->typelib->count,
            *static_cast<ulonglong *>(value), opts->def_value, argument,
            strlen(argument), &flag_error, &error_len);
        if (flag_error) {
          res = EXIT_ARGUMENT_INVALID;
          goto ret;
        };
      } break;
      case GET_NO_ARG: /* get_one_option has taken care of the value already */
      default:         /* dummy default to avoid compiler warnings */
        break;
    }
    if (err) {
      res = EXIT_UNKNOWN_SUFFIX;
      goto ret;
    };
  }
  setval_source(opts, (void *)opts->arg_source);
  return 0;

ret:
  my_getopt_error_reporter(ERROR_LEVEL, EE_FAILED_TO_SET_OPTION_VALUE,
                           my_progname, argument, opts->name);
  return res;
}

/**
  Find option

  IMPLEMENTATION
    Go through all options in the my_option struct. Return true
    if an option is found. sets opt_res to the option found, if any.

    @param         optpat   name of option to find (with - or _)
    @param         length   Length of optpat
    @param[in,out] opt_res  Options

    @retval 0    No matching options
    @retval 1    Found an option
*/

int findopt(const char *optpat, uint length, const struct my_option **opt_res) {
  for (const struct my_option *opt = *opt_res; opt->name; opt++)
    if (!getopt_compare_strings(opt->name, optpat, length) &&
        !opt->name[length]) {
      (*opt_res) = opt;
      return 1;
    }
  return 0;
}

/*
  function: compare_strings

  Works like strncmp, other than 1.) considers '-' and '_' the same.
  2.) Returns true if strings differ, false if they are equal
*/

bool getopt_compare_strings(const char *s, const char *t, uint length) {
  char const *end = s + length;
  for (; s != end; s++, t++) {
    if ((*s != '-' ? *s : '_') != (*t != '-' ? *t : '_')) return true;
  }
  return false;
}

/*
  function: eval_num_suffix

  Transforms a number with a suffix to real number. Suffix can
  be:
  * k|K for kilo
  * m|M for mega
  * g|G for giga
  * t|T for tera
  * p|P for peta
  * e|E for exa

 @tparam LLorULL longlong or ulonglong
 @param  [in]  argument    string containing number, plus possible suffix.
 @param  [out] error       set to non-zero in case of conversion errors.
 @param  [in]  option_name used for better error reporting in case of errors.
*/

template <typename LLorULL>
LLorULL eval_num_suffix(const char *argument, int *error,
                        const char *option_name) {
  char *endchar;
  LLorULL num;
  ulonglong result = 0;

  *error = 0;
  errno = 0;
  // Note: some platforms leave errno == 0, others set it to EINVAL
  // for input "X"
  if (std::is_unsigned<LLorULL>::value)
    num = my_strtoull(argument, &endchar, 10);
  else
    num = my_strtoll(argument, &endchar, 10);

  if (*endchar == '\0' && errno == 0) return num;

  bool is_negative = false;
  // Avoid left-shift of negative values.
  if (std::is_signed<LLorULL>::value && num < 0) {
    is_negative = true;
    if (static_cast<long long>(num) == LLONG_MIN)
      errno = ERANGE;  // This will overflow
    else
      num = -1 * num;
  }

  unsigned long long ull_num = num;

  const size_t num_input_bits = std::bitset<64>(ull_num).count();

  if (errno != ERANGE) {
    switch (*endchar) {
      case 'k':
      case 'K':
        result = ull_num << 10;
        break;
      case 'm':
      case 'M':
        result = ull_num << 20;
        break;
      case 'g':
      case 'G':
        result = ull_num << 30;
        break;
      case 't':
      case 'T':
        result = ull_num << 40;
        break;
      case 'p':
      case 'P':
        result = ull_num << 50;
        break;
      case 'e':
      case 'E':
        result = ull_num << 60;
        break;
      default:
        my_message_local(ERROR_LEVEL, EE_UNKNOWN_SUFFIX_FOR_VARIABLE, *endchar,
                         option_name, argument);
        *error = 1;
        return 0;
    }
  }

  const size_t num_output_bits = std::bitset<64>(result).count();

  // Check over/underflow for signed values.
  if (std::is_signed<LLorULL>::value) {
    if (is_negative) {
      if (result > LLONG_MAX + 1ULL) errno = ERANGE;
    } else {
      if (result > LLONG_MAX) errno = ERANGE;
    }
  }

  // If we have lost some bits, then there is overflow.
  if (num_input_bits != num_output_bits) {
    errno = ERANGE;
  }

  if (errno == ERANGE) {
    const uint ecode = std::is_unsigned<LLorULL>::value
                           ? EE_INCORRECT_UINT_VALUE_FOR_OPTION
                           : EE_INCORRECT_INT_VALUE_FOR_OPTION;

    my_getopt_error_reporter(ERROR_LEVEL, ecode, argument);
    *error = 1;
    return 0;
  }
  if (is_negative) return -1 * result;
  return result;
}

// Some platforms need explicit instantiation of these:
template longlong eval_num_suffix<longlong>(const char *, int *, const char *);
template ulonglong eval_num_suffix<ulonglong>(const char *, int *,
                                              const char *);

/*
  function: getopt_ll

  Evaluates and returns the value that user gave as an argument
  to a variable. Recognizes (case insensitive) K as KILO, M as MEGA,
  G as GIGA, T as in TERA, P as PETA and E as EXA bytes.
  Some values must be in certain blocks, as
  defined in the given my_option struct, this function will check
  that those values are honored.
  In case of an error, set error value in *err.
*/

longlong getopt_ll(const char *arg, const struct my_option *optp, int *err) {
  longlong num = eval_num_suffix<longlong>(arg, err, optp->name);
  return getopt_ll_limit_value(num, optp, nullptr);
}

/**
  Maximum possible value for an integer GET_* variable type
  @param  var_type  type of integer variable (GET_*)
  @returns  maximum possible value for this type
 */
ulonglong max_of_int_range(int var_type) {
  switch (var_type) {
    case GET_INT:
      return INT_MAX;
    case GET_LONG:
      return LONG_MAX;
    case GET_LL:
      return LLONG_MAX;
    case GET_UINT:
      return UINT_MAX;
    case GET_ULONG:
      return ULONG_MAX;
    case GET_ULL:
      return ULLONG_MAX;
    default:
      DBUG_ASSERT(0);
      return 0;
  }
}

/*
  function: getopt_ll_limit_value

  Applies min/max/block_size to a numeric value of an option.
  Returns "fixed" value.
*/

longlong getopt_ll_limit_value(longlong num, const struct my_option *optp,
                               bool *fix) {
  longlong old = num;
  bool adjusted = false;
  char buf1[255], buf2[255];
  ulonglong block_size = (optp->block_size ? (ulonglong)optp->block_size : 1L);
  const longlong max_of_type =
      (longlong)max_of_int_range(optp->var_type & GET_TYPE_MASK);

  if (num > 0 && ((ulonglong)num > (ulonglong)optp->max_value) &&
      optp->max_value) /* if max value is not set -> no upper limit */
  {
    num = (ulonglong)optp->max_value;
    adjusted = true;
  }

  if (num > max_of_type) {
    num = max_of_type;
    adjusted = true;
  }

  num = (num / block_size);
  num = (longlong)(num * block_size);

  if (num < optp->min_value) {
    num = optp->min_value;
    if (old < optp->min_value) adjusted = true;
  }

  if (fix)
    *fix = old != num;
  else if (adjusted)
    my_getopt_error_reporter(WARNING_LEVEL, EE_ADJUSTED_SIGNED_VALUE_FOR_OPTION,
                             optp->name, llstr(old, buf1), llstr(num, buf2));
  return num;
}

static inline bool is_negative_num(const char *num) {
  while (my_isspace(&my_charset_latin1, *num)) num++;

  return (*num == '-');
}

/*
  function: getopt_ull

  This is the same as getopt_ll, but is meant for unsigned long long
  values.
*/

ulonglong getopt_ull(const char *arg, const struct my_option *optp, int *err) {
  char buf[255];
  ulonglong num;

  /* If a negative number is specified as a value for the option. */
  if (arg == nullptr || is_negative_num(arg) == true) {
    num = (ulonglong)optp->min_value;
    my_getopt_error_reporter(WARNING_LEVEL,
                             EE_ADJUSTED_ULONGLONG_VALUE_FOR_OPTION, optp->name,
                             arg, ullstr(num, buf));
  } else
    num = eval_num_suffix<ulonglong>(arg, err, optp->name);

  return getopt_ull_limit_value(num, optp, nullptr);
}

ulonglong getopt_ull_limit_value(ulonglong num, const struct my_option *optp,
                                 bool *fix) {
  bool adjusted = false;
  ulonglong old = num;
  char buf1[255], buf2[255];
  const ulonglong max_of_type =
      max_of_int_range(optp->var_type & GET_TYPE_MASK);

  if (num > (ulonglong)optp->max_value &&
      optp->max_value) /* if max value is not set -> no upper limit */
  {
    num = (ulonglong)optp->max_value;
    adjusted = true;
  }

  if (num > max_of_type) {
    num = max_of_type;
    adjusted = true;
  }

  if (optp->block_size > 1) {
    num /= (ulonglong)optp->block_size;
    num *= (ulonglong)optp->block_size;
  }

  if (num < (ulonglong)optp->min_value) {
    num = (ulonglong)optp->min_value;
    if (old < (ulonglong)optp->min_value) adjusted = true;
  }

  if (fix)
    *fix = old != num;
  else if (adjusted)
    my_getopt_error_reporter(WARNING_LEVEL,
                             EE_ADJUSTED_UNSIGNED_VALUE_FOR_OPTION, optp->name,
                             ullstr(old, buf1), ullstr(num, buf2));

  return num;
}

double getopt_double_limit_value(double num, const struct my_option *optp,
                                 bool *fix) {
  bool adjusted = false;
  double old = num;
  double min, max;

  max = getopt_ulonglong2double(optp->max_value);
  min = getopt_ulonglong2double(optp->min_value);
  if (max && num > max) {
    num = max;
    adjusted = true;
  }
  if (num < min) {
    num = min;
    adjusted = true;
  }
  if (fix)
    *fix = adjusted;
  else if (adjusted)
    my_getopt_error_reporter(WARNING_LEVEL, EE_ADJUSTED_DOUBLE_VALUE_FOR_OPTION,
                             optp->name, old, num);
  return num;
}

/*
  Get double value withing ranges

  Evaluates and returns the value that user gave as an argument to a variable.

  RETURN
    decimal value of arg

    In case of an error, prints an error message and sets *err to
    EXIT_ARGUMENT_INVALID.  Otherwise err is not touched
*/

double getopt_double(const char *arg, const struct my_option *optp, int *err) {
  double num;
  int error;
  const char *end = arg + 1000; /* Big enough as *arg is \0 terminated */
  num = my_strtod(arg, &end, &error);
  if (end[0] != 0 || error) {
    my_getopt_error_reporter(ERROR_LEVEL, EE_INVALID_DECIMAL_VALUE_FOR_OPTION,
                             optp->name);
    *err = EXIT_ARGUMENT_INVALID;
    return 0.0;
  }
  return getopt_double_limit_value(num, optp, nullptr);
}

/*
  Init one value to it's default values

  SYNOPSIS
    init_one_value()
    option		Option to initialize
    value		Pointer to variable
*/

static void init_one_value(const struct my_option *option, void *variable,
                           longlong value) {
  DBUG_TRACE;
  switch ((option->var_type & GET_TYPE_MASK)) {
    case GET_BOOL:
      *((bool *)variable) = (bool)value;
      break;
    case GET_INT:
      *((int *)variable) =
          (int)getopt_ll_limit_value((int)value, option, nullptr);
      break;
    case GET_ENUM:
      *((ulong *)variable) = (ulong)value;
      break;
    case GET_UINT:
      *((uint *)variable) =
          (uint)getopt_ull_limit_value((uint)value, option, nullptr);
      break;
    case GET_LONG:
      *((long *)variable) =
          (long)getopt_ll_limit_value((long)value, option, nullptr);
      break;
    case GET_ULONG:
      *((ulong *)variable) =
          (ulong)getopt_ull_limit_value((ulong)value, option, nullptr);
      break;
    case GET_LL:
      *((longlong *)variable) = getopt_ll_limit_value(value, option, nullptr);
      break;
    case GET_ULL:
      *((ulonglong *)variable) =
          getopt_ull_limit_value((ulonglong)value, option, nullptr);
      break;
    case GET_SET:
    case GET_FLAGSET:
      *((ulonglong *)variable) = (ulonglong)value;
      break;
    case GET_DOUBLE:
      *((double *)variable) = getopt_ulonglong2double(value);
      break;
    case GET_STR:
    case GET_PASSWORD:
      /*
        Do not clear variable value if it has no default value.
        The default value may already be set.
        NOTE: To avoid compiler warnings, we first cast longlong to intptr,
        so that the value has the same size as a pointer.
      */
      if ((char *)(intptr)value) *((char **)variable) = (char *)(intptr)value;
      break;
    case GET_STR_ALLOC:
      /*
        Do not clear variable value if it has no default value.
        The default value may already be set.
        NOTE: To avoid compiler warnings, we first cast longlong to intptr,
        so that the value has the same size as a pointer.
      */
      if ((char *)(intptr)value) {
        char **pstr = (char **)variable;
        my_free(*pstr);
        *pstr =
            my_strdup(key_memory_defaults, (char *)(intptr)value, MYF(MY_WME));
      }
      break;
    default: /* dummy default to avoid compiler warnings */
      break;
  }
}

/*
  Init one value to it's default values

  SYNOPSIS
    init_one_value()
    option		Option to initialize
    value		Pointer to variable
*/

static void fini_one_value(const struct my_option *option, void *variable,
                           longlong value MY_ATTRIBUTE((unused))) {
  DBUG_TRACE;
  switch ((option->var_type & GET_TYPE_MASK)) {
    case GET_STR_ALLOC:
      my_free(*((char **)variable));
      *((char **)variable) = nullptr;
      break;
    default: /* dummy default to avoid compiler warnings */
      break;
  }
}

void my_cleanup_options(const struct my_option *options) {
  init_variables(options, fini_one_value);
}

/*
  initialize all variables to their default values

  SYNOPSIS
    init_variables()
    options		Array of options

  NOTES
    We will initialize the value that is pointed to by options->value.
    If the value is of type GET_ASK_ADDR, we will ask for the address
    for a value and initialize.
*/

static void init_variables(const struct my_option *options,
                           init_func_p init_one_value) {
  DBUG_TRACE;
  for (; options->name; options++) {
    void *value;
    DBUG_PRINT("options", ("name: '%s'", options->name));
    /*
      We must set u_max_value first as for some variables
      options->u_max_value == options->value and in this case we want to
      set the value to default value.
    */
    if (options->u_max_value)
      init_one_value(options, options->u_max_value, options->max_value);
    value = (options->var_type & GET_ASK_ADDR
                 ? (*getopt_get_addr)("", 0, options, nullptr)
                 : options->value);
    if (value) init_one_value(options, value, options->def_value);
  }
}

/**
  Prints variable or option name, replacing _ with - to given file stream
  parameter (by default to stdout).
  @param [in] optp      my_option parameter
  @param [in] file      stream where the output of optp parameter name
                        goes (by default to stdout).
*/
static uint print_name(const struct my_option *optp, FILE *file = stdout) {
  const char *s = optp->name;
  for (; *s; s++) putc(*s == '_' ? '-' : *s, file);
  return s - optp->name;
}

/*
  function: my_print_options

  Print help for all options and variables.
*/

void my_print_help(const struct my_option *options) {
  uint col, name_space = 22, comment_space = 57;
  const char *line_end;
  const struct my_option *optp;

  for (optp = options; optp->name; optp++) {
    if (optp->id && optp->id < 256) {
      printf("  -%c%s", optp->id, strlen(optp->name) ? ", " : "  ");
      col = 6;
    } else {
      printf("  ");
      col = 2;
    }
    if (strlen(optp->name)) {
      printf("--");
      col += 2 + print_name(optp);
      if (optp->arg_type == NO_ARG ||
          (optp->var_type & GET_TYPE_MASK) == GET_BOOL) {
        putchar(' ');
        col++;
      } else if ((optp->var_type & GET_TYPE_MASK) == GET_STR ||
                 (optp->var_type & GET_TYPE_MASK) == GET_PASSWORD ||
                 (optp->var_type & GET_TYPE_MASK) == GET_STR_ALLOC ||
                 (optp->var_type & GET_TYPE_MASK) == GET_ENUM ||
                 (optp->var_type & GET_TYPE_MASK) == GET_SET ||
                 (optp->var_type & GET_TYPE_MASK) == GET_FLAGSET) {
        printf("%s=name%s ", optp->arg_type == OPT_ARG ? "[" : "",
               optp->arg_type == OPT_ARG ? "]" : "");
        col += (optp->arg_type == OPT_ARG) ? 8 : 6;
      } else {
        printf("%s=#%s ", optp->arg_type == OPT_ARG ? "[" : "",
               optp->arg_type == OPT_ARG ? "]" : "");
        col += (optp->arg_type == OPT_ARG) ? 5 : 3;
      }
      if (col > name_space && optp->comment && *optp->comment) {
        putchar('\n');
        col = 0;
      }
    }
    for (; col < name_space; col++) putchar(' ');
    if (optp->comment && *optp->comment) {
      const char *comment = optp->comment, *end = strend(comment);

      while ((uint)(end - comment) > comment_space) {
        for (line_end = comment + comment_space; *line_end != ' '; line_end--) {
        }
        for (; comment != line_end; comment++) putchar(*comment);
        comment++; /* skip the space, as a newline will take it's place now */
        putchar('\n');
        for (col = 0; col < name_space; col++) putchar(' ');
      }
      printf("%s", comment);
    }
    putchar('\n');
    if ((optp->var_type & GET_TYPE_MASK) == GET_BOOL) {
      if (optp->def_value != 0) {
        printf("%*s(Defaults to on; use --skip-", name_space, "");
        print_name(optp);
        printf(" to disable.)\n");
      }
    }
  }
}

/**
 function: my_print_variables
 Print variables.
 @param [in] options    my_option list
*/
void my_print_variables(const struct my_option *options) {
  my_print_variables_ex(options, stdout);
}

/**
  function: my_print_variables_ex
  Print variables to given file parameter stream (by default to stdout).
  @param [in] options    my_options list
  @param [in] file       stream where the output goes.
*/

void my_print_variables_ex(const struct my_option *options, FILE *file) {
  uint name_space = 34, nr;
  size_t length;
  ulonglong llvalue;
  char buff[255];
  const struct my_option *optp;

  for (optp = options; optp->name; optp++) {
    length = strlen(optp->name) + 1;
    if (length > name_space) name_space = length;
  }

  fprintf(file, "\nVariables (--variable-name=value)\n");
  fprintf(file, "%-*s%s", name_space, "and boolean options {FALSE|TRUE}",
          "Value (after reading options)\n");
  for (length = 1; length < 75; length++)
    putc(length == name_space ? ' ' : '-', file);
  putc('\n', file);

  for (optp = options; optp->name; optp++) {
    void *value = (optp->var_type & GET_ASK_ADDR
                       ? (*getopt_get_addr)("", 0, optp, nullptr)
                       : optp->value);
    if (value) {
      length = print_name(optp, file);
      for (; length < name_space; length++) putc(' ', file);
      switch ((optp->var_type & GET_TYPE_MASK)) {
        case GET_SET:
          if (!(llvalue = *static_cast<ulonglong *>(value)))
            fprintf(file, "%s\n", "");
          else
            for (nr = 0; llvalue && nr < optp->typelib->count;
                 nr++, llvalue >>= 1) {
              if (llvalue & 1)
                fprintf(file, llvalue > 1 ? "%s," : "%s\n",
                        get_type(optp->typelib, nr));
            }
          break;
        case GET_FLAGSET:
          llvalue = *static_cast<ulonglong *>(value);
          for (nr = 0; llvalue && nr < optp->typelib->count;
               nr++, llvalue >>= 1) {
            fprintf(file, "%s%s=", (nr ? "," : ""),
                    get_type(optp->typelib, nr));
            fprintf(file, llvalue & 1 ? "on" : "off");
          }
          fprintf(file, "\n");
          break;
        case GET_ENUM:
          fprintf(file, "%s\n", get_type(optp->typelib, *(ulong *)value));
          break;
        case GET_STR:
        case GET_PASSWORD:
        case GET_STR_ALLOC: /* fall through */
          fprintf(file, "%s\n",
                  *((char **)value) ? *((char **)value) : "(No default value)");
          break;
        case GET_BOOL:
          fprintf(file, "%s\n", *((bool *)value) ? "TRUE" : "FALSE");
          break;
        case GET_INT:
          fprintf(file, "%d\n", *((int *)value));
          break;
        case GET_UINT:
          fprintf(file, "%u\n", *((uint *)value));
          break;
        case GET_LONG:
          fprintf(file, "%ld\n", *((long *)value));
          break;
        case GET_ULONG:
          fprintf(file, "%lu\n", *((ulong *)value));
          break;
        case GET_LL:
          fprintf(file, "%s\n", llstr(*((longlong *)value), buff));
          break;
        case GET_ULL:
          ullstr(*(static_cast<ulonglong *>(value)), buff);
          fprintf(file, "%s\n", buff);
          break;
        case GET_DOUBLE:
          fprintf(file, "%g\n", *(double *)value);
          break;
        case GET_NO_ARG:
          fprintf(file, "(No default value)\n");
          break;
        default:
          fprintf(file, "(Disabled)\n");
          break;
      }
    }
  }
}
