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
  @file mysys/my_default.cc
  Add all options from files named "group".cnf from the default_directories
  before the command line arguments.
  On Windows defaults will also search in the Windows directory for a file
  called 'group'.ini
  As long as the program uses the last argument for conflicting
  options one only have to add a call to "load_defaults" to enable
  use of default values.
  pre- and end 'blank space' are removed from options and values. The
  following escape sequences are recognized in values:  @code \b \t \n \r \\
  @endcode

  The following arguments are handled automatically;  If used, they must be
  first argument on the command line!
  --no-defaults	; no options are read, except for the ones provided in the
                   login file.
  --defaults-file=full-path-to-default-file	; Only this file will be read.
  --defaults-extra-file=full-path-to-default-file ; Read this file before ~/
  --defaults-group-suffix  ; Also read groups with concat(group, suffix)
  --print-defaults	  ; Print the modified command line and exit
  --login-path=login-path-name ; Read options under login-path-name from
                                the login file.
*/

#include "my_config.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "m_string.h"
#include "my_aes.h"
#include "my_alloc.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_default.h"
#include "my_dir.h"
#include "my_getopt.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "mysql/psi/mysql_file.h"
#include "mysql_version.h"  // MYSQL_PERSIST_CONFIG_NAME
#include "mysys/my_default_priv.h"
#include "mysys/mysys_priv.h"
#include "mysys_err.h"
#include "typelib.h"
#ifdef _WIN32
#include <winbase.h>
#endif

#include <algorithm>
#include <map>
#include <string>

#include "prealloced_array.h"

using std::string;

struct my_variable_sources {
  string m_config_file_name;
  enum_variable_source m_source;
};

/**
  Defines mapping between variable names (set as part of config files
  or command line) and its config file/source value.
  ex: If config file /etc/my.cnf has variables max_connections= 30 and
  $datadir/mysqld-auto.cnf has variables max_heap_table_size=887808
  then this variable will have following key/value pair.
  max_connections -> (/etc/my.cnf , enum_variable_source::GLOBAL)
  max_heap_table_size -> ($datadir/mysqld-auto.cnf ,
                          enum_variable_source::PERSISTED)
*/
static std::map<string, my_variable_sources> variables_hash;
/**
  Defines mapping between config files names and its corresponding enum values.
  ex: File /etc/my.cnf is mapped to enum_variable_source::GLOBAL
  ~/.my.cnf is mapped to enum_variable_source::USER
  command line options are mapped to enum_variable_source::COMMAND_LINE
*/
static std::map<string, enum_variable_source> default_paths;

/**
  Holds a reference to the directory where the persisted configuration
  file is located.

  This usually is the data directory.
  This is different from ::mysql_real_data_home mostly because it's passed down
  to mysys. And is also filled in a bit differently from ::mysql_real_data_home
  by Persisted_variables_cache::init()
*/
char datadir_buffer[FN_REFLEN] = {0};

#ifdef HAVE_PSI_INTERFACE
extern PSI_file_key key_file_cnf;
#endif
PSI_memory_key key_memory_defaults;

/**
   arguments separator

   load_defaults() loads arguments from config file and put them
   before the arguments from command line, this separator is used to
   separate the arguments loaded from config file and arguments user
   provided on command line.

   Options with value loaded from config file are always in the form
   '--option=value', while for command line options, the value can be
   given as the next argument. Thus we used a separator so that
   handle_options() can distinguish them.

   Note: any other places that does not need to distinguish them
   should skip the separator.

   The content of arguments separator does not matter, one should only
   check the pointer, use "----args-separator----" here to ease debug
   if someone misused it.

   The args seprator will only be added when
   my_getopt_use_args_seprator is set to TRUE before calling
   load_defaults();

   See BUG#25192
*/
static const char *args_separator = "----args-separator----";
inline static void set_args_separator(const char **arg) {
  DBUG_ASSERT(my_getopt_use_args_separator);
  *arg = args_separator;
}
/*
  persisted arguments separator

  This argument separator string is used to separate arguments loaded
  from persisted config file and command line arguments. This string
  is placed after command line options if there are any read only
  persisted variables in persisted config file.
*/
static const char *persist_args_separator = "----persist-args-separator----";
void set_persist_args_separator(char **arg) {
  *arg = const_cast<char *>(persist_args_separator);
}
bool my_getopt_is_ro_persist_args_separator(const char *arg) {
  return (arg == persist_args_separator);
}

/*
  This flag indicates that the argument separator string
  (args_separator) should be added to the list of arguments,
  in order to separate arguments received from config file
  and command line.
*/
bool my_getopt_use_args_separator = false;
bool my_getopt_is_args_separator(const char *arg) {
  return (arg == args_separator);
}

const char *my_defaults_file = nullptr;
const char *my_defaults_group_suffix = nullptr;
const char *my_defaults_extra_file = nullptr;

static const char *my_login_path = nullptr;

static char my_defaults_file_buffer[FN_REFLEN];
static char my_defaults_extra_file_buffer[FN_REFLEN];

static bool defaults_already_read = false;

/* Set to TRUE, if --no-defaults is found. */
bool no_defaults = false;

/* Which directories are searched for options (and in which order) */

#define MAX_DEFAULT_DIRS 6
#define DEFAULT_DIRS_SIZE (MAX_DEFAULT_DIRS + 1) /* Terminate with NULL */
static const char **default_directories = nullptr;

#ifdef _WIN32
static const char *f_extensions[] = {".ini", ".cnf", 0};
#else
static const char *f_extensions[] = {".cnf", nullptr};
#endif

extern "C" {
static int handle_default_option(void *in_ctx, const char *group_name,
                                 const char *option, const char *cnf_file);
}

/*
   This structure defines the context that we pass to callback
   function 'handle_default_option' used in search_default_file
   to process each option. This context is used if search_default_file
   was called from load_defaults.
*/

typedef Prealloced_array<char *, 100> My_args;
struct handle_option_ctx {
  MEM_ROOT *alloc;
  My_args *m_args;
  TYPELIB *group;
};

static int search_default_file(Process_option_func func, void *func_ctx,
                               const char *dir, const char *config_file,
                               bool is_login_file);
static int search_default_file_with_ext(
    Process_option_func func, void *func_ctx, const char *dir, const char *ext,
    const char *config_file, int recursion_level, bool is_login_file);
static bool mysql_file_getline(char *str, int size, MYSQL_FILE *file,
                               bool is_login_file);

/**
  Create the list of default directories.

  @verbatim
  The directories searched, in order, are:
  - Windows:     GetSystemWindowsDirectory()
  - Windows:     GetWindowsDirectory()
  - Windows:     C:/
  - Windows:     Directory above where the executable is located
  - Unix:        /etc/
  - Unix:        /etc/mysql/
  - Unix:        --sysconfdir=<path> (compile-time option)
  - ALL:         getenv("MYSQL_HOME")
  - ALL:         --defaults-extra-file=<path> (run-time option)
  - Unix:        ~/
  @endverbatim

  On all systems, if a directory is already in the list, it will be moved
  to the end of the list.  This avoids reading defaults files multiple times,
  while ensuring the correct precedence.

  @param alloc  MEM_ROOT where the list of directories is stored

  @retval NULL  Failure (out of memory, probably)
  @retval other Pointer to NULL-terminated array of default directories
*/

static const char **init_default_directories(MEM_ROOT *alloc);

static char *remove_end_comment(char *ptr);

/*
  Expand a file name so that the current working directory is added if
  the name is relative.

  RETURNS
   0   All OK
   2   Out of memory or path too long
   3   Not able to get working directory
 */

static int fn_expand(const char *filename, char *result_buf) {
  char dir[FN_REFLEN];
  const int flags = MY_UNPACK_FILENAME | MY_SAFE_PATH | MY_RELATIVE_PATH;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("filename: %s, result_buf: %p", filename, result_buf));
  if (my_getwd(dir, sizeof(dir), MYF(0))) return 3;
  DBUG_PRINT("debug", ("dir: %s", dir));
  if (fn_format(result_buf, filename, dir, "", flags) == nullptr) return 2;
  DBUG_PRINT("return", ("result: %s", result_buf));
  return 0;
}

/*
  Process config files in default directories.

  SYNOPSIS
  my_search_option_files()
  conf_file                   Basename for configuration file to search for.
                              If this is a path, then only this file is read.
  argc                        Pointer to argc of original program
  argv                        Pointer to argv of original program
  args_used                   Pointer to variable for storing the number of
                              arguments used.
  func                        Pointer to the function to process options
  func_ctx                    It's context. Usually it is the structure to
                              store additional options.
  default_directories         List of default directories.
  found_no_defaults           TRUE, if --no-defaults is specified.

  DESCRIPTION
    Process the default options from argc & argv
    Read through each found config file looks and calls 'func' to process
    each option. This function also reads options from login file.

  NOTES
    --defaults-group-suffix is only processed if we are called from
    load_defaults().


  RETURN
    0  ok
    1  given cinf_file doesn't exist
    2  out of memory
    3  Can't get current working directory

    The global variable 'my_defaults_group_suffix' is updated with value for
    --defaults_group_suffix
*/

int my_search_option_files(const char *conf_file, int *argc, char ***argv,
                           uint *args_used, Process_option_func func,
                           void *func_ctx, const char **default_directories,
                           bool is_login_file, bool found_no_defaults) {
  const char **dirs;
  char *forced_default_file, *forced_extra_defaults;
  DBUG_TRACE;

  /* Skip for login file. */
  if (!is_login_file) {
    /* Check if we want to force the use a specific default file */
    *args_used += get_defaults_options(
        *argc - *args_used, *argv + *args_used, &forced_default_file,
        &forced_extra_defaults, const_cast<char **>(&my_defaults_group_suffix),
        const_cast<char **>(&my_login_path), found_no_defaults);

    if (!my_defaults_group_suffix)
      my_defaults_group_suffix = getenv("MYSQL_GROUP_SUFFIX");

    if (forced_extra_defaults && !defaults_already_read) {
      int error =
          fn_expand(forced_extra_defaults, my_defaults_extra_file_buffer);
      if (error) return error;

      my_defaults_extra_file = my_defaults_extra_file_buffer;
    }

    if (forced_default_file && !defaults_already_read) {
      int error = fn_expand(forced_default_file, my_defaults_file_buffer);
      if (error) return error;
      my_defaults_file = my_defaults_file_buffer;
    }

    defaults_already_read = true;
    init_variable_default_paths();

    /*
      We can only handle 'defaults-group-suffix' if we are called from
      load_defaults() as otherwise we can't know the type of 'func_ctx'
    */

    if (my_defaults_group_suffix && func == handle_default_option) {
      /* Handle --defaults-group-suffix= */
      uint i;
      const char **extra_groups;
      const size_t instance_len = strlen(my_defaults_group_suffix);
      struct handle_option_ctx *ctx = (struct handle_option_ctx *)func_ctx;
      char *ptr;
      TYPELIB *group = ctx->group;

      if (!(extra_groups = (const char **)ctx->alloc->Alloc(
                (2 * group->count + 1) * sizeof(char *))))
        return 2;

      for (i = 0; i < group->count; i++) {
        size_t len;
        extra_groups[i] = group->type_names[i]; /** copy group */

        len = strlen(extra_groups[i]);
        if (!(ptr = (char *)ctx->alloc->Alloc((uint)(len + instance_len + 1))))
          return 2;

        extra_groups[i + group->count] = ptr;

        /** Construct new group */
        memcpy(ptr, extra_groups[i], len);
        memcpy(ptr + len, my_defaults_group_suffix, instance_len + 1);
      }

      group->count *= 2;
      group->type_names = extra_groups;
      group->type_names[group->count] = nullptr;
    }
  } else if (my_login_path && func == handle_default_option) {
    /* Handle --login_path= */
    uint i;
    size_t len;
    const char **extra_groups;
    size_t instance_len = 0;
    struct handle_option_ctx *ctx = (struct handle_option_ctx *)func_ctx;
    char *ptr;
    TYPELIB *group = ctx->group;

    if (!(extra_groups = (const char **)ctx->alloc->Alloc((group->count + 3) *
                                                          sizeof(char *))))
      return 2;

    for (i = 0; i < group->count; i++) {
      extra_groups[i] = group->type_names[i]; /** copy group */
    }

    extra_groups[i] = my_login_path;

    if (my_defaults_group_suffix && func == handle_default_option) {
      instance_len = strlen(my_defaults_group_suffix);
      len = strlen(extra_groups[i]);

      if (!(ptr = (char *)ctx->alloc->Alloc((uint)(len + instance_len + 1))))
        return 2;

      extra_groups[i + 1] = ptr;

      /** Construct new group */
      memcpy(ptr, extra_groups[i], len);
      memcpy(ptr + len, my_defaults_group_suffix, instance_len + 1);
      group->count += 1;
    }

    group->count += 1;
    group->type_names = extra_groups;
    group->type_names[group->count] = nullptr;
  }

  // If conf_file is an absolute path, we only read it
  if (dirname_length(conf_file)) {
    int error;
    if ((error = search_default_file(func, func_ctx, NullS, conf_file,
                                     is_login_file)) < 0)
      goto err;
  }
  // If my defaults file is set (from a previous run), we read it
  else if (my_defaults_file) {
    int error;
    if ((error = search_default_file_with_ext(
             func, func_ctx, "", "", my_defaults_file, 0, is_login_file)) < 0)
      goto err;
    if (error > 0) {
      my_message_local(ERROR_LEVEL, EE_FAILED_TO_OPEN_DEFAULTS_FILE,
                       my_defaults_file);
      goto err;
    }
  } else if (!found_no_defaults) {
    for (dirs = default_directories; *dirs; dirs++) {
      if (**dirs) {
        if (search_default_file(func, func_ctx, *dirs, conf_file,
                                is_login_file) < 0)
          goto err;
      } else if (my_defaults_extra_file) {
        int error;
        if ((error = search_default_file_with_ext(func, func_ctx, "", "",
                                                  my_defaults_extra_file, 0,
                                                  is_login_file)) < 0)
          goto err; /* Fatal error */
        if (error > 0) {
          my_message_local(ERROR_LEVEL, EE_FAILED_TO_OPEN_DEFAULTS_FILE,
                           my_defaults_extra_file);
          goto err;
        }
      }
    }
  }

  return 0;

err:
  my_message_local(ERROR_LEVEL, EE_FAILED_TO_HANDLE_DEFAULTS_FILE);
  return 1;
}

/*
  The option handler for load_defaults.

  SYNOPSIS
    handle_deault_option()
    in_ctx                  Handler context. In this case it is a
                            handle_option_ctx structure.
    group_name              The name of the group the option belongs to.
    option                  The very option to be processed. It is already
                            prepared to be used in argv (has -- prefix). If it
                            is NULL, we are handling a new group (section).
    cnf_file                Config file name with absolute file path.

  DESCRIPTION
    This handler checks whether a group is one of the listed and adds an option
    to the array if yes. Some other handler can record, for instance, all
    groups and their options, not knowing in advance the names and amount of
    groups.

  RETURN
    0 - ok
    1 - error occurred
*/

static int handle_default_option(void *in_ctx, const char *group_name,
                                 const char *option, const char *cnf_file) {
  char *tmp;
  struct handle_option_ctx *ctx = (struct handle_option_ctx *)in_ctx;

  if (!option) return 0;

  if (find_type(group_name, ctx->group, FIND_TYPE_NO_PREFIX)) {
    if (!(tmp = (char *)ctx->alloc->Alloc(strlen(option) + 1))) return 1;
    if (ctx->m_args->push_back(tmp)) return 1;
    my_stpcpy(tmp, option);
    update_variable_source(option, cnf_file);
  }
  return 0;
}

/*
  Gets options from the command line, however if --no-defaults
  option is used, --defaults-file & --defaults-extra-file options
  would be ignored.

  SYNOPSIS
    get_defaults_options()
    argc			Pointer to argc of original program
    argv			Pointer to argv of original program
    defaults                    --defaults-file option
    extra_defaults              --defaults-extra-file option
    group_suffix                --defaults-group-suffix option
    login_path                  --login-path option

  RETURN
    # Number of arguments used from *argv
      defaults and extra_defaults will be set to option of the appropriate
      items of argv array, or to NULL if there are no such options
*/

int get_defaults_options(int argc, char **argv, char **defaults,
                         char **extra_defaults, char **group_suffix,
                         char **login_path, bool found_no_defaults) {
  int org_argc = argc, prev_argc = 0, default_option_count = 0;
  *defaults = *extra_defaults = *group_suffix = *login_path = nullptr;

  while (argc >= 2 && argc != prev_argc) {
    /* Skip program name or previously handled argument */
    argv++;
    prev_argc = argc; /* To check if we found */
    /* --no-defaults is always the first option. */
    if (is_prefix(*argv, "--no-defaults") && !default_option_count) {
      argc--;
      default_option_count++;
      continue;
    }
    if (!*defaults && is_prefix(*argv, "--defaults-file=") &&
        !found_no_defaults) {
      *defaults = *argv + sizeof("--defaults-file=") - 1;
      argc--;
      default_option_count++;
      continue;
    }
    if (!*extra_defaults && is_prefix(*argv, "--defaults-extra-file=") &&
        !found_no_defaults) {
      *extra_defaults = *argv + sizeof("--defaults-extra-file=") - 1;
      argc--;
      default_option_count++;
      continue;
    }
    if (!*group_suffix && is_prefix(*argv, "--defaults-group-suffix=")) {
      *group_suffix = *argv + sizeof("--defaults-group-suffix=") - 1;
      argc--;
      default_option_count++;
      continue;
    }
    if (!*login_path && is_prefix(*argv, "--login-path=")) {
      *login_path = *argv + sizeof("--login-path=") - 1;
      argc--;
      default_option_count++;
      continue;
    }
  }
  return org_argc - argc;
}

/*
  Wrapper around my_load_defaults() for interface compatibility.

  SYNOPSIS
    load_defaults()
    conf_file			Basename for configuration file to search for.
                                If this is a path, then only this file is read.
    groups			Which [group] entrys to read.
                                Points to an null terminated array of pointers
    argc			Pointer to argc of original program
    argv			Pointer to argv of original program
    alloc			MEM_ROOT to allocate new argv on

  NOTES

    This function is NOT thread-safe as it uses a global pointer internally.
    See also notes for my_load_defaults().

  RETURN
    0 ok
    1 The given conf_file didn't exists
*/
int load_defaults(const char *conf_file, const char **groups, int *argc,
                  char ***argv, MEM_ROOT *alloc) {
  return my_load_defaults(conf_file, groups, argc, argv, alloc,
                          &default_directories);
}

/** A global to turn off or on reading the mylogin file. On by default */
bool my_defaults_read_login_file = true;
/*
  Read options from configurations files

  SYNOPSIS
    my_load_defaults()
    conf_file			Basename for configuration file to search for.
                                If this is a path, then only this file is read.
    groups			Which [group] entrys to read.
                                Points to an null terminated array of pointers
    argc			Pointer to argc of original program
    argv			Pointer to argv of original program
    alloc			MEM_ROOT to allocate new argv on
    default_directories         Pointer to a location where a pointer to the
  list of default directories will be stored

  IMPLEMENTATION

   Read options from configuration files and put them BEFORE the arguments
   that are already in argc and argv.  This way the calling program can
   easily command line options override options in configuration files

   NOTES
    In case of fatal error, the function will print a warning and do
    exit(1)

    argv will be replaced with a set of filtered arguments, allocated on
    the MEM_ROOT given in as "alloc". You must free this MEM_ROOT yourself.

   RETURN
     - If successful, 0 is returned. If 'default_directories' is not NULL,
     a pointer to the array of default directory paths is stored to a location
     it points to. That stored value must be passed to my_search_option_files()
     later.

     - 1 is returned if the given conf_file didn't exist. In this case, the
     value pointed to by default_directories is undefined.
*/

int my_load_defaults(const char *conf_file, const char **groups, int *argc,
                     char ***argv, MEM_ROOT *alloc,
                     const char ***default_directories) {
  My_args my_args(key_memory_defaults);
  TYPELIB group;
  bool found_print_defaults = false;
  uint args_used = 0;
  int error = 0;
  const char **ptr;
  const char **res;
  struct handle_option_ctx ctx;
  const char **dirs;
  char my_login_file[FN_REFLEN];
  bool found_no_defaults = false;
  uint args_sep = my_getopt_use_args_separator ? 1 : 0;
  DBUG_TRACE;

  if ((dirs = init_default_directories(alloc)) == nullptr) goto err;
  /*
    Check if the user doesn't want any default option processing
    --no-defaults is always the first option
  */
  if (*argc >= 2 && !strcmp(argv[0][1], "--no-defaults"))
    no_defaults = found_no_defaults = true;

  group.count = 0;
  group.name = "defaults";
  group.type_names = groups;

  for (; *groups; groups++) group.count++;

  ctx.alloc = alloc;
  ctx.m_args = &my_args;
  ctx.group = &group;

  if ((error = my_search_option_files(conf_file, argc, argv, &args_used,
                                      handle_default_option, (void *)&ctx, dirs,
                                      false, found_no_defaults))) {
    return error;
  }

  if (my_defaults_read_login_file) {
    /* Read options from login group. */
    if (my_default_get_login_file(my_login_file, sizeof(my_login_file)) &&
        (error = my_search_option_files(my_login_file, argc, argv, &args_used,
                                        handle_default_option, (void *)&ctx,
                                        dirs, true, found_no_defaults))) {
      free_root(alloc, MYF(0));
      return error;
    }
  }
  /*
    Here error contains <> 0 only if we have a fully specified conf_file
    or a forced default file
  */
  if (!(ptr = (const char **)alloc->Alloc(
            (my_args.size() + *argc + 1 + args_sep) * sizeof(char *))))
    goto err;
  res = ptr;

  /* copy name + found arguments + command line arguments to new array */
  res[0] = argv[0][0]; /* Name MUST be set */
  if (!my_args.empty())
    memcpy((res + 1), &my_args[0], my_args.size() * sizeof(char *));
  /* Skip --defaults-xxx options */
  (*argc) -= args_used;
  (*argv) += args_used;

  /*
    Check if we wan't to see the new argument list
    This options must always be the last of the default options
  */
  if (*argc >= 2 && !strcmp(argv[0][1], "--print-defaults")) {
    found_print_defaults = true;
    --*argc;
    ++*argv; /* skip argument */
  }

  if (my_getopt_use_args_separator) {
    /* set arguments separator for arguments from config file and
       command line */
    set_args_separator(&res[my_args.size() + 1]);
  }

  if (*argc)
    memcpy((uchar *)(res + 1 + my_args.size() + args_sep),
           (char *)((*argv) + 1), (*argc - 1) * sizeof(char *));
  res[my_args.size() + *argc + args_sep] = nullptr; /* last null */

  (*argc) += my_args.size() + args_sep;
  *argv = const_cast<char **>(res);

  if (default_directories) *default_directories = dirs;

  if (found_no_defaults) return 0;

  if (found_print_defaults) {
    int i;
    printf("%s would have been started with the following arguments:\n",
           **argv);
    for (i = 1; i < *argc; i++)
      if (!my_getopt_is_args_separator(
              (*argv)[i])) /* skip arguments separator */
      {
        if (strncmp((*argv)[i], "--password", 10) == 0)
          printf("%s ", "--password=*****");
        else
          printf("%s ", (*argv)[i]);
      }
    puts("");
    exit(0);
  }

  return 0;

err:
  my_message_local(ERROR_LEVEL, EE_FAILED_TO_HANDLE_DEFAULTS_FILE);
  exit(1);
  return 0; /* Keep compiler happy */
}

static int search_default_file(Process_option_func opt_handler,
                               void *handler_ctx, const char *dir,
                               const char *config_file, bool is_login_file) {
  const char **ext;
  const char *empty_list[] = {"", nullptr};
  bool have_ext = fn_ext(config_file)[0] != 0;
  const char **exts_to_use = have_ext ? empty_list : f_extensions;

  for (ext = exts_to_use; *ext; ext++) {
    int error;
    if ((error =
             search_default_file_with_ext(opt_handler, handler_ctx, dir, *ext,
                                          config_file, 0, is_login_file)) < 0)
      return error;
  }
  return 0;
}

/*
  Skip over keyword and get argument after keyword

  SYNOPSIS
   get_argument()
   keyword		Include directive keyword
   kwlen		Length of keyword
   ptr			Pointer to the keword in the line under process
   line			line number

  RETURN
   0	error
   #	Returns pointer to the argument after the keyword.
*/

static char *get_argument(const char *keyword, size_t kwlen, char *ptr,
                          char *name, uint line) {
  char *end;

  /* Skip over "include / includedir keyword" and following whitespace */

  for (ptr += kwlen - 1; my_isspace(&my_charset_latin1, ptr[0]); ptr++) {
  }

  /*
    Trim trailing whitespace from directory name
    The -1 below is for the newline added by fgets()
    Note that my_isspace() is true for \r and \n
  */
  for (end = ptr + strlen(ptr) - 1; my_isspace(&my_charset_latin1, *(end - 1));
       end--) {
  }
  end[0] = 0; /* Cut off end space */

  /* Print error msg if there is nothing after !include* directive */
  if (end <= ptr) {
    my_message_local(ERROR_LEVEL, EE_WRONG_DIRECTIVE_IN_CONFIG_FILE, keyword,
                     name, line);
    return nullptr;
  }
  return ptr;
}

/*
  Open a configuration file (if exists) and read given options from it

  SYNOPSIS
    search_default_file_with_ext()
    opt_handler                 Option handler function. It is used to process
                                every separate option.
    handler_ctx                 Pointer to the structure to store actual
                                parameters of the function.
    dir				directory to read
    ext				Extension for configuration file
    config_file                 Name of configuration file
    group			groups to read
    recursion_level             the level of recursion, got while processing
                                "!include" or "!includedir"
    is_login_file               TRUE, when login file is being processed.

  RETURN
    0   Success
    -1	Fatal error, abort
     1	File not found (Warning)
*/

static int search_default_file_with_ext(Process_option_func opt_handler,
                                        void *handler_ctx, const char *dir,
                                        const char *ext,
                                        const char *config_file,
                                        int recursion_level,
                                        bool is_login_file) {
  char name[FN_REFLEN + 10], buff[4096], curr_gr[4096], *ptr, *end;
  const char **tmp_ext;
  char *value, option[4096 + 2], tmp[FN_REFLEN];
  static const char includedir_keyword[] = "includedir";
  static const char include_keyword[] = "include";
  const int max_recursion_level = 10;
  MYSQL_FILE *fp;
  uint line = 0;
  bool found_group = false;
  uint i, rc;
  MY_DIR *search_dir;
  FILEINFO *search_file;

  if ((dir ? strlen(dir) : 0) + strlen(config_file) >= FN_REFLEN - 3)
    return 0; /* Ignore wrong paths */
  if (dir) {
    end = convert_dirname(name, dir, NullS);
    if (dir[0] == FN_HOMELIB) /* Add . to filenames in home */
      *end++ = '.';
    strxmov(end, config_file, ext, NullS);
  } else {
    my_stpcpy(name, config_file);
  }
  fn_format(name, name, "", "", MY_UNPACK_FILENAME);

  if ((rc = check_file_permissions(name, is_login_file)) < 2) return (int)rc;

  if (is_login_file) {
    if (!(fp = mysql_file_fopen(key_file_cnf, name, O_RDONLY | MY_FOPEN_BINARY,
                                MYF(0))))
      return 1; /* Ignore wrong files. */
  } else {
    if (!(fp = mysql_file_fopen(key_file_cnf, name, O_RDONLY, MYF(0))))
      return 1; /* Ignore wrong files */
  }

  while (mysql_file_getline(buff, sizeof(buff) - 1, fp, is_login_file)) {
    line++;
    /* Ignore comment and empty lines */
    for (ptr = buff; my_isspace(&my_charset_latin1, *ptr); ptr++) {
    }

    if (*ptr == '#' || *ptr == ';' || !*ptr) continue;

    /* Configuration File Directives */
    if (*ptr == '!') {
      if (recursion_level >= max_recursion_level) {
        for (end = ptr + strlen(ptr) - 1;
             my_isspace(&my_charset_latin1, *(end - 1)); end--) {
        }
        end[0] = 0;
        my_message_local(WARNING_LEVEL,
                         EE_SKIPPING_DIRECTIVE_DUE_TO_MAX_INCLUDE_RECURSION,
                         ptr, name, line);
        continue;
      }

      /* skip over `!' and following whitespace */
      for (++ptr; my_isspace(&my_charset_latin1, ptr[0]); ptr++) {
      }

      if ((!strncmp(ptr, includedir_keyword, sizeof(includedir_keyword) - 1)) &&
          my_isspace(&my_charset_latin1, ptr[sizeof(includedir_keyword) - 1])) {
        if (!(ptr = get_argument(includedir_keyword, sizeof(includedir_keyword),
                                 ptr, name, line)))
          goto err;

        if (!(search_dir = my_dir(ptr, MYF(MY_WME)))) goto err;

        for (i = 0; i < search_dir->number_off_files; i++) {
          search_file = search_dir->dir_entry + i;
          ext = fn_ext(search_file->name);

          /* check extension */
          for (tmp_ext = f_extensions; *tmp_ext; tmp_ext++) {
            if (!strcmp(ext, *tmp_ext)) break;
          }

          if (*tmp_ext) {
            fn_format(tmp, search_file->name, ptr, "",
                      MY_UNPACK_FILENAME | MY_SAFE_PATH);

            /* add the include file to the paths list with the class of the
             * including file */
            std::map<string, enum_variable_source>::iterator it =
                default_paths.find(name);
            /*
              The current file should always be a part of the paths.
              But that applies only for the server.
              For direct load_defaults() use all bets are off.
              Hence keeping it as a dynamic condition.
            */
            if (it != default_paths.end()) default_paths[tmp] = it->second;

            search_default_file_with_ext(opt_handler, handler_ctx, nullptr,
                                         nullptr, tmp, recursion_level + 1,
                                         is_login_file);
          }
        }

        my_dirend(search_dir);
      } else if ((!strncmp(ptr, include_keyword,
                           sizeof(include_keyword) - 1)) &&
                 my_isspace(&my_charset_latin1,
                            ptr[sizeof(include_keyword) - 1])) {
        if (!(ptr = get_argument(include_keyword, sizeof(include_keyword), ptr,
                                 name, line)))
          goto err;

        /* add the include file to the paths list with the class of the
         * including file */
        std::map<string, enum_variable_source>::iterator it =
            default_paths.find(name);
        /*
          The current file should always be a part of the paths.
          But that applies only for the server.
          For direct load_defaults() use all bets are off.
          Hence keeping it as a dynamic condition.
        */
        if (it != default_paths.end() &&
            fn_format(tmp, ptr, "", "", MY_UNPACK_FILENAME | MY_SAFE_PATH))
          default_paths[tmp] = it->second;

        search_default_file_with_ext(opt_handler, handler_ctx, nullptr, nullptr,
                                     ptr, recursion_level + 1, is_login_file);
      }

      continue;
    }

    if (*ptr == '[') /* Group name */
    {
      found_group = true;
      if (!(end = strchr(++ptr, ']'))) {
        my_message_local(ERROR_LEVEL,
                         EE_INCORRECT_GRP_DEFINITION_IN_CONFIG_FILE, name,
                         line);
        goto err;
      }
      /* Remove end space */
      for (; my_isspace(&my_charset_latin1, end[-1]); end--) {
      }

      end[0] = 0;

      strmake(curr_gr, ptr,
              std::min<size_t>((end - ptr) + 1, sizeof(curr_gr) - 1));

      /* signal that a new group is found */
      opt_handler(handler_ctx, curr_gr, nullptr, nullptr);

      continue;
    }
    if (!found_group) {
      my_message_local(ERROR_LEVEL, EE_OPTION_WITHOUT_GRP_IN_CONFIG_FILE, name,
                       line);
      goto err;
    }

    end = remove_end_comment(ptr);
    if ((value = strchr(ptr, '='))) end = value; /* Option without argument */
    for (; my_isspace(&my_charset_latin1, end[-1]); end--) {
    }

    if (!value) {
      strmake(my_stpcpy(option, "--"), ptr, (size_t)(end - ptr));
      if (opt_handler(handler_ctx, curr_gr, option, name)) goto err;
    } else {
      /* Remove pre- and end space */
      char *value_end;
      for (value++; my_isspace(&my_charset_latin1, *value); value++) {
      }

      value_end = strend(value);
      /*
        We don't have to test for value_end >= value as we know there is
        an '=' before
      */
      for (; my_isspace(&my_charset_latin1, value_end[-1]); value_end--) {
      }

      if (value_end < value) /* Empty string */
        value_end = value;

      /* remove quotes around argument */
      if ((*value == '\"' || *value == '\'') && /* First char is quote */
          (value + 1 < value_end) &&            /* String is longer than 1 */
          *value == value_end[-1]) /* First char is equal to last char */
      {
        value++;
        value_end--;
      }
      ptr = my_stpnmov(my_stpcpy(option, "--"), ptr, (size_t)(end - ptr));
      *ptr++ = '=';

      for (; value != value_end; value++) {
        if (*value == '\\' && value != value_end - 1) {
          switch (*++value) {
            case 'n':
              *ptr++ = '\n';
              break;
            case 't':
              *ptr++ = '\t';
              break;
            case 'r':
              *ptr++ = '\r';
              break;
            case 'b':
              *ptr++ = '\b';
              break;
            case 's':
              *ptr++ = ' '; /* space */
              break;
            case '\"':
              *ptr++ = '\"';
              break;
            case '\'':
              *ptr++ = '\'';
              break;
            case '\\':
              *ptr++ = '\\';
              break;
            default: /* Unknown; Keep '\' */
              *ptr++ = '\\';
              *ptr++ = *value;
              break;
          }
        } else
          *ptr++ = *value;
      }
      *ptr = 0;
      if (opt_handler(handler_ctx, curr_gr, option, name)) goto err;
    }
  }
  mysql_file_fclose(fp, MYF(0));
  return (0);

err:
  mysql_file_fclose(fp, MYF(0));
  return -1; /* Fatal error */
}

static char *remove_end_comment(char *ptr) {
  char quote = 0;  /* we are inside quote marks */
  char escape = 0; /* symbol is protected by escape chagacter */

  for (; *ptr; ptr++) {
    if ((*ptr == '\'' || *ptr == '\"') && !escape) {
      if (!quote)
        quote = *ptr;
      else if (quote == *ptr)
        quote = 0;
    }
    /* We are not inside a string */
    if (!quote && *ptr == '#') {
      *ptr = 0;
      return ptr;
    }
    escape = (quote && *ptr == '\\' && !escape);
  }
  return ptr;
}

/**
  Read one line from the specified file. In case
  of scrambled login file, the line read is first
  decrypted and then returned.

  @param [out] str           Buffer to store the read text.
  @param [in] size           At max, size-1 bytes to be read.
  @param [in] file           Source file.
  @param [in] is_login_file  TRUE, when login file is being processed.

  @return 1               Success
          0               Error
*/

static bool mysql_file_getline(char *str, int size, MYSQL_FILE *file,
                               bool is_login_file) {
  uchar cipher[4096], len_buf[MAX_CIPHER_STORE_LEN];
  static unsigned char my_key[LOGIN_KEY_LEN];
  int length = 0, cipher_len = 0;

  if (is_login_file) {
    if (mysql_file_ftell(file) == 0) {
      /* Move past unused bytes. */
      mysql_file_fseek(file, 4, SEEK_SET);
      if (mysql_file_fread(file, my_key, LOGIN_KEY_LEN, MYF(MY_WME)) !=
          LOGIN_KEY_LEN)
        return false;
    }

    if (mysql_file_fread(file, len_buf, MAX_CIPHER_STORE_LEN, MYF(MY_WME)) ==
        MAX_CIPHER_STORE_LEN) {
      cipher_len = sint4korr(len_buf);
      if (cipher_len > size) return false;
    } else
      return false;

    mysql_file_fread(file, cipher, cipher_len, MYF(MY_WME));
    if ((length =
             my_aes_decrypt(cipher, cipher_len, (unsigned char *)str, my_key,
                            LOGIN_KEY_LEN, my_aes_128_ecb, nullptr)) < 0) {
      /* Attempt to decrypt failed. */
      return false;
    }
    str[length] = 0;
    return true;
  } else {
    if (mysql_file_fgets(str, size, file))
      return true;
    else
      return false;
  }
}

void my_print_default_files(const char *conf_file) {
  const char *empty_list[] = {"", nullptr};
  bool have_ext = fn_ext(conf_file)[0] != 0;
  const char **exts_to_use = have_ext ? empty_list : f_extensions;
  char name[FN_REFLEN];
  const char **ext;

  puts(
      "\nDefault options are read from the following files in the given "
      "order:");

  if (dirname_length(conf_file))
    fputs(conf_file, stdout);
  else {
    const char **dirs;
    MEM_ROOT alloc;
    init_alloc_root(key_memory_defaults, &alloc, 512, 0);

    if ((dirs = init_default_directories(&alloc)) == nullptr) {
      fputs("Internal error initializing default directories list", stdout);
    } else {
      for (; *dirs; dirs++) {
        for (ext = exts_to_use; *ext; ext++) {
          const char *pos;
          char *end;
          if (**dirs)
            pos = *dirs;
          else if (my_defaults_extra_file)
            pos = my_defaults_extra_file;
          else
            continue;
          end = convert_dirname(name, pos, NullS);
          if (name[0] == FN_HOMELIB) /* Add . to filenames in home */
            *end++ = '.';

          if (my_defaults_extra_file == pos)
            end[(strlen(end) - 1)] = ' ';
          else
            strxmov(end, conf_file, *ext, " ", NullS);
          fputs(name, stdout);
        }
      }
    }

    free_root(&alloc, MYF(0));
  }
  puts("");
}

void print_defaults(const char *conf_file, const char **groups) {
  const char **groups_save = groups;
  my_print_default_files(conf_file);

  fputs("The following groups are read:", stdout);
  for (; *groups; groups++) {
    fputc(' ', stdout);
    fputs(*groups, stdout);
  }

  if (my_defaults_group_suffix) {
    groups = groups_save;
    for (; *groups; groups++) {
      fputc(' ', stdout);
      fputs(*groups, stdout);
      fputs(my_defaults_group_suffix, stdout);
    }
  }
  puts(
      "\nThe following options may be given as the first argument:\n\
--print-defaults        Print the program argument list and exit.\n\
--no-defaults           Don't read default options from any option file,\n\
                        except for login file.\n\
--defaults-file=#       Only read default options from the given file #.\n\
--defaults-extra-file=# Read this file after the global files are read.\n\
--defaults-group-suffix=#\n\
                        Also read groups with concat(group, suffix)\n\
--login-path=#          Read this path from the login file.");
}

/**
  Initialize all the mappings between default config file paths/
  command line options/persistent config file path/login file path
  and corresponding enum_variable_source values.
*/
void init_variable_default_paths() {
  char datadir[FN_REFLEN] = {0};
  string extradir =
      (my_defaults_extra_file ? my_defaults_extra_file : string());
  string explicitdir = (my_defaults_file ? my_defaults_file : string());

  string defsyscondir;
#if defined(DEFAULT_SYSCONFDIR)
  defsyscondir = DEFAULT_SYSCONFDIR;
#endif

#ifdef _WIN32
  char buffer[FN_REFLEN];

  /* windows supports ini/cnf extension for some config files */
  if (GetWindowsDirectory(buffer, sizeof(buffer))) {
    default_paths[string(buffer) + "\\my.ini"] = enum_variable_source::GLOBAL;
    default_paths[string(buffer) + "\\my.cnf"] = enum_variable_source::GLOBAL;
  }
  default_paths["C:\\my.ini"] = enum_variable_source::GLOBAL;
  default_paths["C:\\my.cnf"] = enum_variable_source::GLOBAL;
  if (GetModuleFileName(NULL, buffer, (DWORD)sizeof(buffer))) {
    char *end = strend(buffer), *last = NULL;
    for (; end > buffer; end--) {
      if (*end == FN_LIBCHAR) {
        if (last) {
          end[1] = 0;
          break;
        }
        last = end;
      }
    }
    default_paths[string(buffer) + "\\.mylogin.cnf"] =
        enum_variable_source::LOGIN;
  }
#else
  char *env = getenv("MYSQL_HOME");
  std::string mysql_home(env == nullptr ? "" : env);
  if (!mysql_home.empty()) {
    if (mysql_home.back() != '/') {
      mysql_home.push_back('/');
    }
    default_paths[mysql_home + "my.cnf"] = enum_variable_source::SERVER;
  }

  char buffer[FN_REFLEN] = "~/";
  unpack_filename(buffer, buffer);
  default_paths["/etc/my.cnf"] = enum_variable_source::GLOBAL;
  default_paths["/etc/mysql/my.cnf"] = enum_variable_source::GLOBAL;
  default_paths[string(buffer) + ".my.cnf"] = enum_variable_source::MYSQL_USER;
  default_paths[string(buffer) + ".mylogin.cnf"] = enum_variable_source::LOGIN;

#if defined(DEFAULT_SYSCONFDIR)
  default_paths[defsyscondir + "/my.cnf"] = enum_variable_source::GLOBAL;
#endif
#endif

  if (datadir_buffer[0])
    default_paths[string(datadir_buffer) + MYSQL_PERSIST_CONFIG_NAME + ".cnf"] =
        enum_variable_source::PERSISTED;
  else {
    convert_dirname(datadir, MYSQL_DATADIR, NullS);
    default_paths[string(datadir) + MYSQL_PERSIST_CONFIG_NAME + ".cnf"] =
        enum_variable_source::PERSISTED;
  }
  if (extradir.length()) default_paths[extradir] = enum_variable_source::EXTRA;
  if (explicitdir.length())
    default_paths[explicitdir] = enum_variable_source::EXPLICIT;

  default_paths[""] = enum_variable_source::COMMAND_LINE;
}

/**
  Track all options loaded from config files and command line options
  along with the path from where options are loaded. For command line
  options path is empty string.

  Ex:
  /etc/my.cnf has max_connections
  /$datadir/mysqld.auto.cnf has max_user_connections
  ./mysqld --server-id=47
  with this setup, variables_hash has 3 entires of the above options
  along with path of config files and its enum value which is as below:
  max_connections -> (/etc/my.cnf , enum_variable_source::GLOBAL)
  max_user_connections -> ($datadir/mysqld.auto.cnf ,
                           enum_variable_source::PERSISTED)
  server-id -> ("" , enum_variable_source::COMMAND_LINE)

   @param [in] opt_name       Pointer to option name. opt_name must be in
                              the form off --XXXXXX
   @param [in] value          Pointer to config file path
*/
void update_variable_source(const char *opt_name, const char *value) {
  string var_name = string(opt_name);
  string path = (value ? value : string(""));
  string prefix[] = {"loose_", "disable_", "enable_", "maximum_", "skip_"};
  uint prefix_count = sizeof(prefix) / sizeof(prefix[0]);

  /* opt_name must be of form --XXXXX which means min length must be 3 */
  if (var_name.length() < 3) return;

  std::size_t pos = var_name.find("=");
  /* strip the value part if present */
  if (pos != string::npos) var_name = var_name.substr(0, pos);

  /* opt_name must be of form --XXXXX which means it must start with -- */
  if (var_name.length() < 3 || var_name[0] != '-' || var_name[1] != '-') return;

  /* remove -- */
  var_name = var_name.substr(2);

  /* replace all '-' to '_' */
  while ((pos = var_name.find("-")) != string::npos)
    var_name.replace(pos, 1, "_");

  /*
    check if variable is prefixed with 'loose', 'skip', 'disable',
    'enable', 'maximum'
  */
  for (uint id = 0; id < prefix_count; id++) {
    if (!var_name.compare(0, prefix[id].size(), prefix[id])) {
      /* check if variables are prefixed with skip_ */
      if (id == 4) {
        bool skip_variable = false;
        string skip_variables[] = {"skip_name_resolve", "skip_networking",
                                   "skip_show_database",
                                   "skip_external_locking"};
        for (uint skip_index = 0;
             skip_index < sizeof(skip_variables) / sizeof(skip_variables[0]);
             ++skip_index) {
          if (var_name == skip_variables[skip_index]) {
            /*
             Do not trim the skip_ prefix for variables which
             start with skip
            */
            skip_variable = true;
            break;
          }
        }
        if (skip_variable == false)
          var_name = var_name.substr(prefix[4].size());
      } else
        var_name = var_name.substr(prefix[id].size());
    }
  }

  std::map<string, enum_variable_source>::iterator it =
      default_paths.find(path);
  if (it != default_paths.end()) {
    my_variable_sources source;
    std::pair<std::map<string, my_variable_sources>::iterator, bool> ret;

    source.m_config_file_name = path;
    source.m_source = it->second;
    ret = variables_hash.insert(
        std::pair<string, my_variable_sources>(var_name, source));
    /*
     If value exists replace it with new path. ex: if there exists
     same variables in my.cnf and mysqld-auto.cnf and specified in
     command line options, then final entry into this hash will be
     option name as key and mysqld-auto.cnf file path + PERSISTED
     as value.
    */
    if (ret.second == false) variables_hash[var_name] = source;
  }
}

/**
  This function will set value for my_option::arg_source by doing a
  lookup into variables_hash based on opt_name as key. If key is present
  corresponding value (config file, enum value) will be set in value.

   @param [in] opt_name       Pointer to option name.
   @param [out] value         Pointer to struct holding config file path
                              and variable source
*/
void set_variable_source(const char *opt_name, void *value) {
  string src_name = opt_name;
  std::size_t pos;

  /* replace all '-' to '_' */
  while ((pos = src_name.find("-")) != string::npos)
    src_name.replace(pos, 1, "_");

  std::map<string, my_variable_sources>::iterator it =
      variables_hash.find(src_name);
  if (it != variables_hash.end()) {
    if ((get_opt_arg_source *)value) {
      memcpy(((get_opt_arg_source *)value)->m_path_name,
             it->second.m_config_file_name.c_str(),
             it->second.m_config_file_name.length());
      ((get_opt_arg_source *)value)->m_source = it->second.m_source;
    }
  }
}

static int add_directory(MEM_ROOT *alloc, const char *dir, const char **dirs) {
  char buf[FN_REFLEN];
  size_t len;
  char *p;
  bool err MY_ATTRIBUTE((unused));

  len = normalize_dirname(buf, dir);
  if (!(p = strmake_root(alloc, buf, len))) return 1; /* Failure */
  /* Should never fail if DEFAULT_DIRS_SIZE is correct size */
  err = array_append_string_unique(p, dirs, DEFAULT_DIRS_SIZE);
  DBUG_ASSERT(err == false);

  return 0;
}

#ifdef _WIN32
/*
  This wrapper for GetSystemWindowsDirectory() will dynamically bind to the
  function if it is available, emulate it on NT4 Terminal Server by stripping
  the \SYSTEM32 from the end of the results of GetSystemDirectory(), or just
  return GetSystemDirectory().
 */

typedef UINT(WINAPI *GET_SYSTEM_WINDOWS_DIRECTORY)(LPSTR, UINT);

static size_t my_get_system_windows_directory(char *buffer, size_t size) {
  size_t count;
  GET_SYSTEM_WINDOWS_DIRECTORY
  func_ptr = (GET_SYSTEM_WINDOWS_DIRECTORY)GetProcAddress(
      GetModuleHandle("kernel32.dll"), "GetSystemWindowsDirectoryA");

  if (func_ptr) return func_ptr(buffer, (uint)size);

  /*
    Windows NT 4.0 Terminal Server Edition:
    To retrieve the shared Windows directory, call GetSystemDirectory and
    trim the "System32" element from the end of the returned path.
  */
  count = GetSystemDirectory(buffer, (uint)size);
  if (count > 8 && stricmp(buffer + (count - 8), "\\System32") == 0) {
    count -= 8;
    buffer[count] = '\0';
  }
  return count;
}

static const char *my_get_module_parent(char *buf, size_t size) {
  char *last = NULL;
  char *end;
  if (!GetModuleFileName(NULL, buf, (DWORD)size)) return NULL;
  end = strend(buf);

  /*
    Look for the second-to-last \ in the filename, but hang on
    to a pointer after the last \ in case we're in the root of
    a drive.
  */
  for (; end > buf; end--) {
    if (*end == FN_LIBCHAR) {
      if (last) {
        /* Keep the last '\' as this works both with D:\ and a directory */
        end[1] = 0;
        break;
      }
      last = end;
    }
  }

  return buf;
}
#endif /* _WIN32 */

static const char **init_default_directories(MEM_ROOT *alloc) {
  const char **dirs;
  char *env;
  int errors = 0;

  dirs = (const char **)alloc->Alloc(DEFAULT_DIRS_SIZE * sizeof(char *));
  if (dirs == nullptr) return nullptr;
  memset(dirs, 0, DEFAULT_DIRS_SIZE * sizeof(char *));

#ifdef _WIN32

  {
    char fname_buffer[FN_REFLEN];
    if (my_get_system_windows_directory(fname_buffer, sizeof(fname_buffer)))
      errors += add_directory(alloc, fname_buffer, dirs);

    if (GetWindowsDirectory(fname_buffer, sizeof(fname_buffer)))
      errors += add_directory(alloc, fname_buffer, dirs);

    errors += add_directory(alloc, "C:/", dirs);

    if (my_get_module_parent(fname_buffer, sizeof(fname_buffer)) != NULL)
      errors += add_directory(alloc, fname_buffer, dirs);
  }

#else

  errors += add_directory(alloc, "/etc/", dirs);
  errors += add_directory(alloc, "/etc/mysql/", dirs);

#if defined(DEFAULT_SYSCONFDIR)
  if (DEFAULT_SYSCONFDIR[0])
    errors += add_directory(alloc, DEFAULT_SYSCONFDIR, dirs);
#endif /* DEFAULT_SYSCONFDIR */

#endif

  if ((env = getenv("MYSQL_HOME"))) errors += add_directory(alloc, env, dirs);

  /* Placeholder for --defaults-extra-file=<path> */
  errors += add_directory(alloc, "", dirs);

#if !defined(_WIN32)
  errors += add_directory(alloc, "~/", dirs);
#endif

  return (errors > 0 ? nullptr : dirs);
}

/**
  Place the login file name in the specified buffer.

  @param [out] file_name       Buffer to hold login file name
  @param [in] file_name_size   Length of the buffer

  @return 1 - Success
          0 - Failure
*/

int my_default_get_login_file(char *file_name, size_t file_name_size) {
  size_t rc;

  if (getenv("MYSQL_TEST_LOGIN_FILE"))
    rc = snprintf(file_name, file_name_size, "%s",
                  getenv("MYSQL_TEST_LOGIN_FILE"));
#ifdef _WIN32
  else if (getenv("APPDATA"))
    rc = snprintf(file_name, file_name_size, "%s\\MySQL\\.mylogin.cnf",
                  getenv("APPDATA"));
#else
  else if (getenv("HOME"))
    rc = snprintf(file_name, file_name_size, "%s/.mylogin.cnf", getenv("HOME"));
#endif
  else {
    memset(file_name, 0, file_name_size);
    return 0;
  }
  /* Anything <= 0 will be treated as error. */
  if (rc <= 0) return 0;

  return 1;
}

/**
  Check file permissions of the option file.

  @param [in] file_name        Name of the option file.
  @param [in] is_login_file    TRUE, when login file is being processed.

  @return  0 - Non-allowable file permissions.
           1 - Failed to stat.
           2 - Success.
*/
int check_file_permissions(const char *file_name, bool is_login_file) {
#if !defined(_WIN32)
  MY_STAT stat_info;

  if (!my_stat(file_name, &stat_info, MYF(0))) return 1;
  /*
    Ignore .mylogin.cnf file if not exclusively readable/writable
    by current user.
  */
  if (is_login_file && (stat_info.st_mode & (S_IXUSR | S_IRWXG | S_IRWXO)) &&
      (stat_info.st_mode & S_IFMT) == S_IFREG) {
    my_message_local(WARNING_LEVEL, EE_CONFIG_FILE_PERMISSION_ERROR, file_name);
    return 0;
  }
  /*
    Ignore world-writable regular files.
    This is mainly done to protect us to not read a file created by
    the mysqld server, but the check is still valid in most context.
  */
  else if ((stat_info.st_mode & S_IWOTH) &&
           (stat_info.st_mode & S_IFMT) == S_IFREG)

  {
    my_message_local(WARNING_LEVEL, EE_IGNORE_WORLD_WRITABLE_CONFIG_FILE,
                     file_name);
    return 0;
  }
#endif
  return 2; /* Success */
}
