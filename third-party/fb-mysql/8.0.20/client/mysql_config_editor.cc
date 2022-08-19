/*
   Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file

  @brief
  MySQL Configuration Utility
*/

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>

#include "client/client_priv.h"
#include "my_aes.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_default.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_rnd.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/my_default_priv.h"
#include "print_version.h"
#include "welcome_copyright_notice.h"

#define MY_LINE_MAX 4096
#define MAX_COMMAND_LIMIT 100
/*
  Header length for the login file.
  4-byte (unused) + LOGIN_KEY_LEN
 */
#define MY_LOGIN_HEADER_LEN (4 + LOGIN_KEY_LEN)

static int g_fd;

/*
  Length of the contents in login file
  excluding the header part.
*/
static size_t file_size;
static const char *opt_user = nullptr, *opt_password = nullptr,
                  *opt_host = nullptr, *opt_login_path = "client",
                  *opt_socket = nullptr, *opt_port = nullptr;

static char my_login_file[FN_REFLEN];
static char my_key[LOGIN_KEY_LEN];

static bool opt_verbose, opt_all,
    tty_password = false, opt_warn, opt_remove_host, opt_remove_pass,
    opt_remove_user, opt_remove_socket, opt_remove_port,
    login_path_specified = false;

static int execute_commands(int command);
static int set_command(void);
static int remove_command(void);
static int print_command(void);
static void print_login_path(DYNAMIC_STRING *file_buf, const char *path_name);
static void remove_login_path(DYNAMIC_STRING *file_buf, const char *path_name);
static char *locate_login_path(DYNAMIC_STRING *file_buf, const char *path_name);
static bool check_and_create_login_file(void);
static void mask_password_and_print(char *buf);
static int reset_login_file(bool gen_key);

static int encrypt_buffer(const char *plain, int plain_len, char cipher[],
                          const int aes_len);
static int decrypt_buffer(const char *cipher, int cipher_len, char plain[]);
static int encrypt_and_write_file(DYNAMIC_STRING *file_buf);
static int read_and_decrypt_file(DYNAMIC_STRING *file_buf);
static int do_handle_options(int argc, char *argv[]);
static void remove_options(DYNAMIC_STRING *file_buf, const char *path_name);
static void remove_option(DYNAMIC_STRING *file_buf, const char *path_name,
                          const char *option_name);
bool generate_login_key(void);
static int read_login_key(void);
static int add_header(void);
static void my_perror(const char *msg);

static void verbose_msg(const char *fmt, ...)
    MY_ATTRIBUTE((format(printf, 1, 2)));
static void usage_program(void);
static void usage_command(int command);
extern "C" bool get_one_option(int optid, const struct my_option *opt,
                               char *argument);

enum commands {
  MY_CONFIG_SET,
  MY_CONFIG_REMOVE,
  MY_CONFIG_PRINT,
  MY_CONFIG_RESET,
  MY_CONFIG_HELP
};

extern "C" {
struct my_command_data {
  const int id;
  const char *name;
  const char *description;
  my_option *options;
  bool (*get_one_option_func)(int optid, const struct my_option *opt,
                              char *argument);
};
}

/* mysql_config_editor utility options. */
static struct my_option my_program_long_options[] = {
#ifdef DBUG_OFF
    {"debug", '#', "This is a non-debug version. Catch this and exit.", 0, 0, 0,
     GET_DISABLED, OPT_ARG, 0, 0, 0, 0, 0, 0},
#else
    {"debug", '#', "Output debug log. Often this is 'd:t:o,filename'.", nullptr,
     nullptr, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
#endif
    {"help", '?', "Display this help and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"verbose", 'v', "Write more information.", &opt_verbose, &opt_verbose,
     nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"version", 'V', "Output version information and exit.", nullptr, nullptr,
     nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

/* Command-specific options. */

/* SET command options. */
static struct my_option my_set_command_options[] = {
    {"help", '?', "Display this help and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"host", 'h', "Host name to be entered into the login file.", &opt_host,
     &opt_host, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"login-path", 'G',
     "Name of the login path to use in the login file. "
     "(Default : client)",
     &opt_login_path, &opt_login_path, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"password", 'p', "Prompt for password to be entered into the login file.",
     nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"user", 'u', "User name to be entered into the login file.", &opt_user,
     &opt_user, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"socket", 'S', "Socket path to be entered into login file.", &opt_socket,
     &opt_socket, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"port", 'P', "Port number to be entered into login file.", &opt_port,
     &opt_port, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"warn", 'w',
     "Warn and ask for confirmation if set command attempts to "
     "overwrite an existing login path (enabled by default).",
     &opt_warn, &opt_warn, nullptr, GET_BOOL, NO_ARG, 1, 0, 0, nullptr, 0,
     nullptr},
    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

/* REMOVE command options. */
static struct my_option my_remove_command_options[] = {
    {"help", '?', "Display this help and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"host", 'h', "Remove host name from the login path.", &opt_remove_host,
     &opt_remove_host, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"login-path", 'G',
     "Name of the login path from which options to "
     "be removed (entire path would be removed if none of user, password, "
     "host, socket, or port options are specified). (Default : client)",
     &opt_login_path, &opt_login_path, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"password", 'p', "Remove password from the login path.", &opt_remove_pass,
     &opt_remove_pass, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"user", 'u', "Remove user name from the login path.", &opt_remove_user,
     &opt_remove_user, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"warn", 'w',
     "Warn and ask for confirmation if remove command attempts "
     "to remove the default login path (client) if no login path is specified "
     "(enabled by default).",
     &opt_warn, &opt_warn, nullptr, GET_BOOL, NO_ARG, 1, 0, 0, nullptr, 0,
     nullptr},
    {"socket", 'S', "Remove socket path from the login path.",
     &opt_remove_socket, &opt_remove_socket, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"port", 'P', "Remove port number from the login path.", &opt_remove_port,
     &opt_remove_port, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

/* PRINT command options. */
static struct my_option my_print_command_options[] = {
    {"all", OPT_CONFIG_ALL, "Used with print command to print all login paths.",
     &opt_all, &opt_all, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"help", '?', "Display this help and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"login-path", 'G',
     "Name of the login path to use in the login file. "
     "(Default : client)",
     &opt_login_path, &opt_login_path, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

/* RESET command options. */
static struct my_option my_reset_command_options[] = {
    {"help", '?', "Display this help and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

/* HELP command options. */
static struct my_option my_help_command_options[] = {
    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

extern "C" {
static bool my_program_get_one_option(
    int optid, const struct my_option *opt MY_ATTRIBUTE((unused)),
    char *argument MY_ATTRIBUTE((unused))) {
  switch (optid) {
    case '#':
      DBUG_PUSH(argument ? argument : "d:t:o,/tmp/mysql_config_editor.trace");
      break;
    case 'V':
      print_version();
      exit(0);
      break;
    case '?':
      usage_program();
      exit(0);
      break;
  }
  return false;
}

static bool my_set_command_get_one_option(int optid, const struct my_option *,
                                          char *) {
  switch (optid) {
    case 'p':
      tty_password = true;
      break;
    case 'G':
      if (login_path_specified) {
        /* Error, we do not support multiple login paths. */
        my_perror(
            "Error: Use of multiple login paths is not supported. "
            "Exiting..");
        return true;
      }
      login_path_specified = true;
      break;
    case '?':
      usage_command(MY_CONFIG_SET);
      exit(0);
      break;
  }
  return false;
}

static bool my_remove_command_get_one_option(int optid,
                                             const struct my_option *, char *) {
  switch (optid) {
    case 'G':
      if (login_path_specified) {
        /* Error, we do not support multiple login paths. */
        my_perror(
            "Error: Use of multiple login paths is not supported. "
            "Exiting..");
        return true;
      }
      login_path_specified = true;
      break;
    case '?':
      usage_command(MY_CONFIG_REMOVE);
      exit(0);
      break;
  }
  return false;
}

static bool my_print_command_get_one_option(int optid, const struct my_option *,
                                            char *) {
  switch (optid) {
    case 'G':
      if (login_path_specified) {
        /* Error, we do not support multiple login paths. */
        my_perror(
            "Error: Use of multiple login paths is not supported. "
            "Exiting..");
        return true;
      }
      login_path_specified = true;
      break;
    case '?':
      usage_command(MY_CONFIG_PRINT);
      exit(0);
      break;
  }
  return false;
}

static bool my_reset_command_get_one_option(int optid, const struct my_option *,
                                            char *) {
  switch (optid) {
    case '?':
      usage_command(MY_CONFIG_RESET);
      exit(0);
      break;
  }
  return false;
}
}

static struct my_command_data command_data[] = {
    {MY_CONFIG_SET, "set", "Write a login path to the login file.",
     my_set_command_options, my_set_command_get_one_option},
    {MY_CONFIG_REMOVE, "remove", "Remove a login path from the login file.",
     my_remove_command_options, my_remove_command_get_one_option},
    {MY_CONFIG_PRINT, "print",
     "Print the contents of login file in unencrypted form.",
     my_print_command_options, my_print_command_get_one_option},
    {MY_CONFIG_RESET, "reset",
     "Empty the contents of the login file. The file is created\n"
     "if it does not exist.",
     my_reset_command_options, my_reset_command_get_one_option},
    {MY_CONFIG_HELP, "help", "Display a help message and exit.",
     my_help_command_options, nullptr},
    {0, nullptr, nullptr, nullptr, nullptr}};

int main(int argc, char *argv[]) {
  MY_INIT(argv[0]);
  DBUG_TRACE;
  int command, rc = 0;

  command = do_handle_options(argc, argv);

  if (command > -1) rc = execute_commands(command);

  if (rc != 0) {
    my_perror("operation failed.");
    return 1;
  }
  return 0;
}

/**
  Handle all the command line arguments.

  program_name [program options] [command [command options]]

*/
static int do_handle_options(int argc, char *argv[]) {
  DBUG_TRACE;

  const char *command_list[MAX_COMMAND_LIMIT + 1];
  char **saved_argv = argv;
  char **argv_cmd;
  char *ptr; /* for free. */
  int argc_cmd;
  int rc, i, command = -1;

  if (argc < 2) {
    usage_program();
    exit(1);
  }

  if (!(ptr = (char *)my_malloc(PSI_NOT_INSTRUMENTED,
                                (argc + 2) * sizeof(char *), MYF(MY_WME))))
    goto error;

  /* Handle program options. */

  /* Prepare a list of supported commands to be used by my_handle_options(). */
  for (i = 0; (command_data[i].name != nullptr) && (i < MAX_COMMAND_LIMIT); i++)
    command_list[i] = command_data[i].name;
  command_list[i] = nullptr;

  if ((rc = my_handle_options(&argc, &argv, my_program_long_options,
                              my_program_get_one_option, command_list, false)))
    exit(rc);

  if (argc == 0) /* No command specified. */
    goto done;

  for (i = 0; command_data[i].name != nullptr; i++) {
    if (!strcmp(command_data[i].name, argv[0])) {
      command = i;
      break;
    }
  }

  if (command == -1) goto error;

  /* Handle command options. */

  argv_cmd = (char **)ptr;
  argc_cmd = argc + 1;

  /* Prepare a command line (argv) using the rest of the options. */
  argv_cmd[0] = saved_argv[0];
  memcpy((uchar *)(argv_cmd + 1), (uchar *)(argv), (argc * sizeof(char *)));
  argv_cmd[argc_cmd] = nullptr;

  if ((rc = handle_options(&argc_cmd, &argv_cmd, command_data[command].options,
                           command_data[command].get_one_option_func)))
    exit(rc);

  /* Do not allow multiple commands. */
  if (argc_cmd > 1) goto error;

done:
  my_free(ptr);
  return command;

error:
  my_free(ptr);
  usage_program();
  exit(1);
}

static int execute_commands(int command) {
  DBUG_TRACE;
  int rc = 0;

  if ((rc = check_and_create_login_file())) goto done;

  switch (command_data[command].id) {
    case MY_CONFIG_SET:
      verbose_msg("Executing set command.\n");
      rc = set_command();
      break;

    case MY_CONFIG_REMOVE:
      verbose_msg("Executing remove command.\n");
      rc = remove_command();
      break;

    case MY_CONFIG_PRINT:
      verbose_msg("Executing print command.\n");
      rc = print_command();
      break;

    case MY_CONFIG_RESET:
      verbose_msg("Resetting login file.\n");
      rc = reset_login_file(true);
      break;

    case MY_CONFIG_HELP:
      verbose_msg("Printing usage info.\n");
      usage_program();
      break;

    default:
      my_perror("unknown command.");
      exit(1);
  }

done:
  my_close(g_fd, MYF(MY_WME));

  return rc;
}

/**
  Execute 'set' command.

  @return -1              Error
           0              Success
*/

static int set_command(void) {
  DBUG_TRACE;

  DYNAMIC_STRING file_buf, path_buf;

  init_dynamic_string(&path_buf, "", MY_LINE_MAX, MY_LINE_MAX);
  init_dynamic_string(&file_buf, "", file_size, 3 * MY_LINE_MAX);

  if (tty_password) opt_password = get_tty_password(NullS);

  if (file_size) {
    if (read_and_decrypt_file(&file_buf) == -1) goto error;
  }

  dynstr_append(&path_buf, "["); /* --login=path */
  if (opt_login_path)
    dynstr_append(&path_buf, opt_login_path);
  else
    dynstr_append(&path_buf, "client");
  dynstr_append(&path_buf, "]");

  if (opt_user) /* --user */
  {
    dynstr_append(&path_buf, "\nuser = ");
    dynstr_append(&path_buf, opt_user);
  }

  if (opt_password) /* --password */
  {
    dynstr_append(&path_buf, "\npassword = ");
    dynstr_append(&path_buf, opt_password);
  }

  if (opt_host) /* --host */
  {
    dynstr_append(&path_buf, "\nhost = ");
    dynstr_append(&path_buf, opt_host);
  }

  if (opt_socket) {
    dynstr_append(&path_buf, "\nsocket = ");
    dynstr_append(&path_buf, opt_socket);
  }

  if (opt_port) {
    dynstr_append(&path_buf, "\nport = ");
    dynstr_append(&path_buf, opt_port);
  }

  dynstr_append(&path_buf, "\n");

  /* Warn if login path already exists */
  if (opt_warn && ((locate_login_path(&file_buf, opt_login_path)) != nullptr)) {
    int choice;
    printf(
        "WARNING : \'%s\' path already exists and will be "
        "overwritten. \n Continue? (Press y|Y for Yes, any "
        "other key for No) : ",
        opt_login_path);
    choice = getchar();

    if (choice != (int)'y' && choice != (int)'Y') goto done; /* skip */
  }

  /* Remove the login path. */
  remove_login_path(&file_buf, opt_login_path);

  /* Append the new login path to the file buffer. */
  dynstr_append(&file_buf, path_buf.str);

  if (encrypt_and_write_file(&file_buf) == -1) goto error;

done:
  dynstr_free(&file_buf);
  dynstr_free(&path_buf);
  return 0;

error:
  dynstr_free(&file_buf);
  dynstr_free(&path_buf);
  return -1;
}

static int remove_command(void) {
  DBUG_TRACE;

  DYNAMIC_STRING file_buf, path_buf;

  init_dynamic_string(&path_buf, "", MY_LINE_MAX, MY_LINE_MAX);
  init_dynamic_string(&file_buf, "", file_size, 3 * MY_LINE_MAX);

  if (file_size) {
    if (read_and_decrypt_file(&file_buf) == -1) goto error;
  } else
    goto done; /* Nothing to remove, skip.. */

  /* Warn if no login path is specified. */
  if (opt_warn && ((locate_login_path(&file_buf, opt_login_path)) != nullptr) &&
      (login_path_specified == false)) {
    int choice;
    printf(
        "WARNING : No login path specified, so options from the default "
        "login path will be removed.\nContinue? (Press y|Y for Yes, "
        "any other key for No) : ");
    choice = getchar();

    if (choice != (int)'y' && choice != (int)'Y') goto done; /* skip */
  }

  remove_options(&file_buf, opt_login_path);

  if (encrypt_and_write_file(&file_buf) == -1) goto error;

done:
  dynstr_free(&file_buf);
  dynstr_free(&path_buf);
  return 0;

error:
  dynstr_free(&file_buf);
  dynstr_free(&path_buf);
  return -1;
}

/**
  Execute 'print' command.

  @return -1              Error
           0              Success
*/

static int print_command(void) {
  DBUG_TRACE;
  DYNAMIC_STRING file_buf;

  init_dynamic_string(&file_buf, "", file_size, 3 * MY_LINE_MAX);

  if (file_size) {
    if (read_and_decrypt_file(&file_buf) == -1) goto error;
  } else
    goto done; /* Nothing to print, skip..*/

  print_login_path(&file_buf, opt_login_path);

done:
  dynstr_free(&file_buf);
  return 0;

error:
  dynstr_free(&file_buf);
  return -1;
}

/**
  Create the login file if it does not exist, check
  and set its permissions and modes.

  @return  true           Error
           false          Success
*/

static bool check_and_create_login_file(void) {
  DBUG_TRACE;

  MY_STAT stat_info;

// This is a hack to make it compile. File permissions are different on Windows.
#ifdef _WIN32
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IRWXU 00700
#define S_IRWXG 00070
#define S_IRWXO 00007
#endif

  const int access_flag = O_RDWR;
  const ushort create_mode = (S_IRUSR | S_IWUSR);

  /* Get the login file name. */
  if (!my_default_get_login_file(my_login_file, sizeof(my_login_file))) {
    my_perror("failed to set login file name");
    goto error;
  }

  /*
    NOTE : MYSQL_TEST_LOGIN_FILE env must be a full path,
    where the directory structure must exist. However the
    login file will be created if it does not exist.
  */
#ifdef _WIN32
  if (!(getenv("MYSQL_TEST_LOGIN_FILE"))) {
    /* Check if 'MySQL' directory is in place. */
    MY_STAT stat_info_dir;
    char login_dir[FN_REFLEN];
    size_t size;

    dirname_part(login_dir, my_login_file, &size);
    /* Remove the trailing '\' */
    if (is_directory_separator(login_dir[--size])) login_dir[size] = 0;

    /* Now check if directory exists? */
    if (my_stat(login_dir, &stat_info_dir, MYF(0))) {
      verbose_msg("%s directory exists.\n", login_dir);
    } else {
      /* Create the login directory. */
      verbose_msg("%s directory doesn't exist, creating it.\n", login_dir);
      if (my_mkdir(login_dir, 0, MYF(0))) {
        my_perror("failed to create the directory");
        goto error;
      }
    }
  }
#endif

  /* Check for login file's existence and permissions (0600). */
  if (my_stat(my_login_file, &stat_info, MYF(0))) {
    verbose_msg("File exists.\n");

    file_size = (size_t)stat_info.st_size;

#ifdef _WIN32
    if (1)
#else
    if (!(stat_info.st_mode & (S_IXUSR | S_IRWXG | S_IRWXO)))
#endif
    {
      verbose_msg("File has the required permission.\nOpening the file.\n");
      if ((g_fd = my_open(my_login_file, access_flag, MYF(MY_WME))) == -1) {
        my_perror("couldn't open the file");
        goto error;
      }
    } else {
      verbose_msg("File does not have the required permission.\n");
      printf(
          "WARNING : Login file does not have the required"
          " file permissions.\nPlease set the mode to 600 &"
          " run the command again.\n");
      goto error;
    }
  } else {
    verbose_msg("File does not exist.\nCreating login file.\n");
    if ((g_fd = my_create(my_login_file, create_mode, access_flag,
                          MYF(MY_WME))) == -1) {
      my_perror("couldn't create the login file");
      goto error;
    } else {
      verbose_msg("Login file created.\n");
      my_close(g_fd, MYF(MY_WME));
      verbose_msg("Opening the file.\n");

      if ((g_fd = my_open(my_login_file, access_flag, MYF(MY_WME))) == -1) {
        my_perror("couldn't open the file");
        goto error;
      }
      file_size = 0;
    }
  }

  if (file_size == 0) {
    if (generate_login_key()) goto error;
    if (add_header() == -1) goto error;
  } else {
    if (read_login_key() == -1) goto error;
  }

  return false;

error:
  return true;
}

/**
  Print options under the specified login path. If '--all'
  option is used, print all the optins stored in the login
  file.

  @param [in] file_buf    Buffer storing the unscrambled login
                          file contents.
  @param [in] path_name   Path name.
*/

static void print_login_path(DYNAMIC_STRING *file_buf, const char *path_name) {
  DBUG_TRACE;

  char *start = nullptr, *end = nullptr, temp = '\0';

  if (file_buf->length == 0) goto done; /* Nothing to print. */

  if (opt_all) {
    start = file_buf->str;
    end = file_buf->str + file_buf->length;
  } else {
    start = locate_login_path(file_buf, path_name);
    if (!start) /* login path not found, skip..*/
      goto done;

    end = strstr(start, "\n[");
  }

  if (end) {
    temp = *end;
    *end = '\0';
  }

  mask_password_and_print(start);

  if (temp != '\0') *end = temp;

done:
  return;
}

/**
  Print the specified buffer by masking the actual
  password string.

  @param [in] buf          Buffer to be printed.
*/

static void mask_password_and_print(char *buf) {
  DBUG_TRACE;
  const char *password_str = "\npassword = ", *mask = "*****";
  char *next = nullptr;

  while ((next = strstr(buf, password_str)) != nullptr) {
    while (*buf != 0 && buf != next) putc(*(buf++), stdout);
    printf("%s", password_str);
    printf("%s\n", mask);
    if (*buf == '\n') /* Move past \n' */
      buf++;

    /* Ignore the password. */
    while (*buf && *(buf++) != '\n') {
    }

    if (!opt_all) break;
  }

  /* Now print the rest of the buffer. */
  while (*buf) putc(*(buf++), stdout);
  // And a new line.. if required.
  if (*(buf - 1) != '\n') putc('\n', stdout);
}

/**
  Remove multiple options from a login path.
*/
static void remove_options(DYNAMIC_STRING *file_buf, const char *path_name) {
  /* If nope of the options are specified remove the entire path. */
  if (!opt_remove_host && !opt_remove_pass && !opt_remove_user &&
      !opt_remove_socket && !opt_remove_port) {
    remove_login_path(file_buf, path_name);
    return;
  }

  if (opt_remove_user) remove_option(file_buf, path_name, "user");

  if (opt_remove_pass) remove_option(file_buf, path_name, "password");

  if (opt_remove_host) remove_option(file_buf, path_name, "host");

  if (opt_remove_socket) remove_option(file_buf, path_name, "socket");

  if (opt_remove_port) remove_option(file_buf, path_name, "port");
}

/**
  Remove an option from a login path.
*/
static void remove_option(DYNAMIC_STRING *file_buf, const char *path_name,
                          const char *option_name) {
  DBUG_TRACE;

  char *start = nullptr, *end = nullptr;
  char *search_str;
  size_t search_len, shift_len;
  bool option_found = false;

  search_str = (char *)my_malloc(PSI_NOT_INSTRUMENTED,
                                 (uint)strlen(option_name) + 2, MYF(MY_WME));
  sprintf(search_str, "\n%s", option_name);

  if ((start = locate_login_path(file_buf, path_name)) == nullptr)
    /* login path was not found, skip.. */
    goto done;

  end = strstr(start, "\n["); /* Next path. */

  if (end)
    search_len = end - start;
  else
    search_len = file_buf->length - (start - file_buf->str);

  while (search_len > 1) {
    if (!strncmp(start, search_str, strlen(search_str))) {
      /* Option found. */
      end = start;
      while (*(++end) != '\n') {
      }
      option_found = true;
      break;
    } else {
      /* Move to next line. */
      while ((--search_len > 1) && (*(++start) != '\n')) {
      }
    }
  }

  if (option_found) {
    shift_len = file_buf->length - (end - file_buf->str);

    file_buf->length -= (end - start);

    while (shift_len--) *(start++) = *(end++);
    *start = '\0';
  }

done:
  my_free(search_str);
}

/**
  Remove the specified login path from the login file.

  @param [in] file_buf    Buffer storing the unscrambled login
                          file contents.
  @param [in] path_name   Path name.
*/

static void remove_login_path(DYNAMIC_STRING *file_buf, const char *path_name) {
  DBUG_TRACE;

  char *start = nullptr, *end = nullptr;
  int to_move, len, diff;

  if ((start = locate_login_path(file_buf, path_name)) == nullptr)
    /* login path was not found, skip.. */
    goto done;

  end = strstr(start, "\n[");

  if (end) {
    end++; /* Move past '\n' */
    len = ((diff = (start - end)) > 0) ? diff : -diff;
    to_move = file_buf->length - (end - file_buf->str);
  } else {
    *start = '\0';
    file_buf->length = ((diff = (file_buf->str - start)) > 0) ? diff : -diff;
    goto done;
  }

  while (to_move--) *(start++) = *(end++);

  *start = '\0';
  file_buf->length -= len;

done:
  return;
}

/**
  Remove all the contents from the login file.

  @param [in] gen_key     Flag to control the generation of
                          a new key.

  @return -1              Error
           0              Success
*/

static int reset_login_file(bool gen_key) {
  DBUG_TRACE;

  if (my_chsize(g_fd, 0, 0, MYF(MY_WME))) {
    verbose_msg("Error while truncating the file.\n");
    goto error;
  }

  /* Seek to the beginning of the file. */
  if (my_seek(g_fd, 0L, SEEK_SET, MYF(MY_WME) == MY_FILEPOS_ERROR))
    goto error; /* Error. */

  if (gen_key && generate_login_key()) goto error; /* Generate a new key. */

  if (add_header() == -1) goto error;

  return 0;

error:
  return 0;
}

/**
  Find the specified login path in the login file buffer
  and return the starting address.

  @param [in] file_buf    Buffer storing the unscrambled login
                          file contents.
  @param [in] path_name   Path name.

  @return                 If found, the starting address of the
                          login path, NULL otherwise.
*/

static char *locate_login_path(DYNAMIC_STRING *file_buf,
                               const char *path_name) {
  DBUG_TRACE;

  char *addr = nullptr;
  DYNAMIC_STRING dy_path_name;

  init_dynamic_string(&dy_path_name, "", 512, 512);

  dynstr_append(&dy_path_name, "\n[");
  dynstr_append(&dy_path_name, path_name);
  dynstr_append(&dy_path_name, "]");

  /* First check if it is the very first login path. */
  if (file_buf->str == strstr(file_buf->str, dy_path_name.str + 1))
    addr = file_buf->str;
  /* If not, scan through the file. */
  else {
    addr = strstr(file_buf->str, dy_path_name.str);
    if (addr) addr++; /* Move past '\n' */
  }

  dynstr_free(&dy_path_name);
  return addr;
}

/**
  Encrypt the file buffer and write it to the login file.

  @param [in] file_buf    Buffer storing the unscrambled login
                          file contents.

  @return -1 Error
           0 Success

  @note The contents of the file buffer are encrypted
        on a line-by-line basis with each line having
        the following format :
        [\<first 4 bytes store cipher-length\>
        |\<Next cipher-length bytes store actual cipher\>]
*/

static int encrypt_and_write_file(DYNAMIC_STRING *file_buf) {
  DBUG_TRACE;

  bool done = false;
  char cipher[MY_LINE_MAX], *tmp = nullptr;
  uint bytes_read = 0, len = 0;
  int enc_len = 0;  // Can be negative.

  if (reset_login_file(false) == -1) goto error;

  /* Move past key first. */
  if (my_seek(g_fd, MY_LOGIN_HEADER_LEN, SEEK_SET, MYF(MY_WME)) !=
      (MY_LOGIN_HEADER_LEN))
    goto error; /* Error while seeking. */

  tmp = &file_buf->str[bytes_read];

  while (!done) {
    len = 0;

    while (*tmp++ != '\n')
      if (len < (file_buf->length - bytes_read))
        len++;
      else {
        done = true;
        break;
      }

    if (done) break;

    if ((enc_len = my_aes_get_size(len + 1, my_aes_128_ecb)) >
        (MY_LINE_MAX - (int)MAX_CIPHER_STORE_LEN)) {
      my_perror(
          "A parameter to mysql_config_editor exceeds the maximum "
          "accepted length. Please review the data you've supplied "
          "and try to shorten them permissible length.\n");
      goto error;
    }

    if (encrypt_buffer(&file_buf->str[bytes_read], ++len,
                       cipher + MAX_CIPHER_STORE_LEN, enc_len) < 0)
      goto error;

    bytes_read += len;

    /* Store cipher length first. */
    int4store(cipher, enc_len);

    if ((my_write(g_fd, (const uchar *)cipher, enc_len + MAX_CIPHER_STORE_LEN,
                  MYF(MY_WME))) != (enc_len + MAX_CIPHER_STORE_LEN))
      goto error;
  }

  verbose_msg("Successfully written encrypted data to the login file.\n");

  /* Update file_size */
  file_size = bytes_read;

  return 0;

error:
  my_perror("couldn't encrypt the file");
  return -1;
}

/**
  Read the login file, unscramble its contents and store
  them into the file buffer.

  @param [in] file_buf    Buffer for storing the unscrambled login
                          file contents.

  @return -1 Error
           0 Success
*/

static int read_and_decrypt_file(DYNAMIC_STRING *file_buf) {
  DBUG_TRACE;

  char cipher[MY_LINE_MAX], plain[MY_LINE_MAX];
  uchar len_buf[MAX_CIPHER_STORE_LEN];
  int cipher_len = 0, dec_len = 0;

  /* Move past key first. */
  if (my_seek(g_fd, MY_LOGIN_HEADER_LEN, SEEK_SET, MYF(MY_WME)) !=
      (MY_LOGIN_HEADER_LEN))
    goto error; /* Error while seeking. */

  /* First read the length of the cipher. */
  while (my_read(g_fd, len_buf, MAX_CIPHER_STORE_LEN, MYF(MY_WME)) ==
         MAX_CIPHER_STORE_LEN) {
    cipher_len = sint4korr(len_buf);

    if (cipher_len > MY_LINE_MAX) goto error;

    /* Now read 'cipher_len' bytes from the file. */
    if ((int)my_read(g_fd, (uchar *)cipher, cipher_len, MYF(MY_WME)) ==
        cipher_len) {
      if ((dec_len = decrypt_buffer(cipher, cipher_len, plain)) < 0) goto error;

      plain[dec_len] = 0;
      dynstr_append(file_buf, plain);
    }
  }

  verbose_msg("Successfully decrypted the login file.\n");
  return 0;

error:
  my_perror("couldn't decrypt the file");
  return -1;
}

/**
  Encrypt the given plain text.

  @param plain            Plain text to be encrypted
  @param plain_len        Length of the plain text
  @param [out] cipher     Encrypted cipher text
  @param aes_len          Length of the cypher

  @return                 -1 if error encountered,
                          length encrypted, otherwise.
*/

static int encrypt_buffer(const char *plain, int plain_len, char cipher[],
                          const int aes_len) {
  DBUG_TRACE;

  if (my_aes_encrypt((const unsigned char *)plain, plain_len,
                     (unsigned char *)cipher, (const unsigned char *)my_key,
                     LOGIN_KEY_LEN, my_aes_128_ecb, nullptr) == aes_len)
    return aes_len;

  verbose_msg("Error! Couldn't encrypt the buffer.\n");
  return -1; /* Error */
}

/**
  Decrypt the given cipher text.

  @param [in] cipher      Cipher text to be decrypted.
  @param [in] cipher_len  Length of the cipher text.
  @param [out] plain      Decrypted plain text.

  @return                 -1 if error encountered,
                          length decrypted, otherwise.
*/

static int decrypt_buffer(const char *cipher, int cipher_len, char plain[]) {
  DBUG_TRACE;
  int aes_length;

  if ((aes_length =
           my_aes_decrypt((const unsigned char *)cipher, cipher_len,
                          (unsigned char *)plain, (const unsigned char *)my_key,
                          LOGIN_KEY_LEN, my_aes_128_ecb, nullptr)) > 0)
    return aes_length;

  verbose_msg("Error! Couldn't decrypt the buffer.\n");
  return -1; /* Error */
}

/**
  Add unused bytes alongwith the to the login key
  to the login file.

  @return                 -1 if error encountered,
                          length written, otherwise.
*/

static int add_header(void) {
  DBUG_TRACE;

  /* Reserved for future use. */
  const char unused[] = {'\0', '\0', '\0', '\0'};

  /* Write 'unused' bytes first. */
  if ((my_write(g_fd, (const uchar *)unused, 4, MYF(MY_WME))) != 4) goto error;

  /* Write the login key. */
  if ((my_write(g_fd, (const uchar *)my_key, LOGIN_KEY_LEN, MYF(MY_WME))) !=
      LOGIN_KEY_LEN)
    goto error;

  verbose_msg("Key successfully written to the file.\n");
  return MY_LOGIN_HEADER_LEN;

error:
  my_perror("file write operation failed");
  return -1;
}

/**
  Algorithm to generate key.

  @retval true error
  @retval false success
*/

bool generate_login_key() {
  DBUG_TRACE;

  verbose_msg("Generating a new key.\n");
  /* Get a sequence of random non-printable ASCII */
  for (uint i = 0; i < LOGIN_KEY_LEN; i++) {
    bool failed;
    my_key[i] = (char)((int)(my_rnd_ssl(&failed) * 100000) % 32);
    if (failed) return true;
  }
  return false;
}

/**
  Read the stored login key.

  @return -1              Error
           0              Success
*/

static int read_login_key(void) {
  DBUG_TRACE;

  verbose_msg("Reading the login key.\n");
  /* Move past the unused buffer. */
  if (my_seek(g_fd, 4, SEEK_SET, MYF(MY_WME)) != 4)
    goto error; /* Error while seeking. */

  if (my_read(g_fd, (uchar *)my_key, LOGIN_KEY_LEN, MYF(MY_WME)) !=
      LOGIN_KEY_LEN)
    goto error; /* Error while reading. */

  verbose_msg("Login key read successfully.\n");
  return 0;

error:
  my_perror("file read operation failed");
  return -1;
}

static void verbose_msg(const char *fmt, ...) {
  DBUG_TRACE;
  va_list args;

  if (!opt_verbose) return;

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fflush(stderr);
}

static void my_perror(const char *msg) {
  char errbuf[MYSYS_STRERROR_SIZE];

  if (errno == 0)
    fprintf(stderr, "%s\n", (msg) ? msg : "");
  else
    fprintf(stderr, "%s : %s\n", (msg) ? msg : "",
            my_strerror(errbuf, sizeof(errbuf), errno));
  // reset errno
  errno = 0;
}

static void usage_command(int command) {
  print_version();
  puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2012"));
  puts("MySQL Configuration Utility.");
  printf("\nDescription: %s\n", command_data[command].description);
  printf("Usage: %s [program options] [%s [command options]]\n", my_progname,
         command_data[command].name);
  my_print_help(command_data[command].options);
  my_print_variables(command_data[command].options);
}

static void usage_program(void) {
  print_version();
  puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2012"));
  puts("MySQL Configuration Utility.");
  printf("Usage: %s [program options] [command [command options]]\n",
         my_progname);
  my_print_help(my_program_long_options);
  my_print_variables(my_program_long_options);
  puts(
      "\nWhere command can be any one of the following :\n\
       set [command options]     Sets user name/password/host name/socket/port\n\
                                 for a given login path (section).\n\
       remove [command options]  Remove a login path from the login file.\n\
       print [command options]   Print all the options for a specified\n\
                                 login path.\n\
       reset [command options]   Deletes the contents of the login file.\n\
       help                      Display this usage/help information.\n");
}
