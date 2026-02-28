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

// Maintenance of MySQL databases.

#include <fcntl.h>
#include <mysql.h>
#include <mysqld_error.h> /* to check server error codes */
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <string>

#include "client/client_priv.h"
#include "compression.h"
#include "m_ctype.h"
#include "my_alloc.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_default.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_thread.h" /* because of signal()	*/
#include "mysql/service_mysql_alloc.h"
#include "print_version.h"
#include "sql_common.h"
#include "typelib.h"
#include "welcome_copyright_notice.h" /* ORACLE_WELCOME_COPYRIGHT_NOTICE */

#define MAX_MYSQL_VAR 1024
#define SHUTDOWN_DEF_TIMEOUT 3600 /* Wait for shutdown */
#define MAX_TRUNC_LENGTH 3

const char *host = nullptr;
char *user = nullptr, *opt_password = nullptr;
const char *default_charset = MYSQL_AUTODETECT_CHARSET_NAME;
char truncated_var_names[MAX_MYSQL_VAR][MAX_TRUNC_LENGTH];
char ex_var_names[MAX_MYSQL_VAR][FN_REFLEN];
ulonglong last_values[MAX_MYSQL_VAR];
static int interval = 0;
static bool option_force = false, interrupted = false, new_line = false,
            opt_compress = false, opt_relative = false, opt_verbose = false,
            opt_vertical = false, tty_password = false, opt_nobeep;
static bool debug_info_flag = false, debug_check_flag = false;
static uint tcp_port = 0, option_wait = 0, option_silent = 0, nr_iterations;
static uint opt_count_iterations = 0, my_end_arg;
static char *opt_bind_addr = nullptr;
static ulong opt_connect_timeout, opt_shutdown_timeout;
static char *unix_port = nullptr;
static char *opt_plugin_dir = nullptr, *opt_default_auth = nullptr;
static uint opt_enable_cleartext_plugin = 0;
static bool using_opt_enable_cleartext_plugin = false;
static bool opt_show_warnings = false;
static uint opt_zstd_compress_level = default_zstd_compression_level;
static char *opt_compress_algorithm = nullptr;
#if defined(_WIN32)
static char *shared_memory_base_name = 0;
#endif
static uint opt_protocol = 0;
static myf error_flags; /* flags to pass to my_printf_error, like ME_BELL */

/*
  When using extended-status relatively, ex_val_max_len is the estimated
  maximum length for any relative value printed by extended-status. The
  idea is to try to keep the length of output as short as possible.
*/

static uint ex_val_max_len[MAX_MYSQL_VAR];
static bool ex_status_printed = false; /* First output is not relative. */
static uint ex_var_count, max_var_length, max_val_length;

#include "sslopt-vars.h"

#include "caching_sha2_passwordopt-vars.h"

static void usage(void);
extern "C" bool get_one_option(int optid, const struct my_option *opt,
                               char *argument);
static bool sql_connect(MYSQL *mysql, uint wait);
static int execute_commands(MYSQL *mysql, int argc, char **argv);
static char **mask_password(int argc, char ***argv);
static int drop_db(MYSQL *mysql, const char *db);
extern "C" void endprog(int signal_number);
static void nice_time(ulong sec, char *buff);
static void print_header(MYSQL_RES *result);
static void print_top(MYSQL_RES *result);
static void print_row(MYSQL_RES *result, MYSQL_ROW cur, uint row);
static void print_relative_row(MYSQL_RES *result, MYSQL_ROW cur, uint row);
static void print_relative_row_vert(MYSQL_RES *result, MYSQL_ROW cur, uint row);
static void print_relative_header();
static void print_relative_line();
static void truncate_names();
static bool get_pidfile(MYSQL *mysql, char *pidfile);
static bool wait_pidfile(char *pidfile, time_t last_modified,
                         struct stat *pidfile_status);
static void store_values(MYSQL_RES *result);
static void print_warnings(MYSQL *mysql);

/*
  The order of commands must be the same as command_names,
  except ADMIN_ERROR
*/
enum commands {
  ADMIN_ERROR,
  ADMIN_CREATE,
  ADMIN_DROP,
  ADMIN_SHUTDOWN,
  ADMIN_RELOAD,
  ADMIN_REFRESH,
  ADMIN_VER,
  ADMIN_PROCESSLIST,
  ADMIN_STATUS,
  ADMIN_KILL,
  ADMIN_DEBUG,
  ADMIN_VARIABLES,
  ADMIN_FLUSH_LOGS,
  ADMIN_FLUSH_HOSTS,
  ADMIN_FLUSH_TABLES,
  ADMIN_PASSWORD,
  ADMIN_PING,
  ADMIN_EXTENDED_STATUS,
  ADMIN_FLUSH_STATUS,
  ADMIN_FLUSH_PRIVILEGES,
  ADMIN_START_SLAVE,
  ADMIN_STOP_SLAVE,
  ADMIN_FLUSH_THREADS
};
static const char *command_names[] = {"create",
                                      "drop",
                                      "shutdown",
                                      "reload",
                                      "refresh",
                                      "version",
                                      "processlist",
                                      "status",
                                      "kill",
                                      "debug",
                                      "variables",
                                      "flush-logs",
                                      "flush-hosts",
                                      "flush-tables",
                                      "password",
                                      "ping",
                                      "extended-status",
                                      "flush-status",
                                      "flush-privileges",
                                      "start-slave",
                                      "stop-slave",
                                      "flush-threads",
                                      NullS};

static TYPELIB command_typelib = {array_elements(command_names) - 1, "commands",
                                  command_names, nullptr};

static struct my_option my_long_options[] = {
    {"bind-address", 0, "IP address to bind to.", (uchar **)&opt_bind_addr,
     (uchar **)&opt_bind_addr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"count", 'c',
     "Number of iterations to make. This works with -i (--sleep) only.",
     &nr_iterations, &nr_iterations, nullptr, GET_UINT, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
#ifdef DBUG_OFF
    {"debug", '#', "This is a non-debug version. Catch this and exit.", 0, 0, 0,
     GET_DISABLED, OPT_ARG, 0, 0, 0, 0, 0, 0},
    {"debug-check", OPT_DEBUG_CHECK,
     "This is a non-debug version. Catch this and exit.", 0, 0, 0, GET_DISABLED,
     NO_ARG, 0, 0, 0, 0, 0, 0},
    {"debug-info", OPT_DEBUG_INFO,
     "This is a non-debug version. Catch this and exit.", 0, 0, 0, GET_DISABLED,
     NO_ARG, 0, 0, 0, 0, 0, 0},
#else
    {"debug", '#', "Output debug log. Often this is 'd:t:o,filename'.", nullptr,
     nullptr, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"debug-check", OPT_DEBUG_CHECK,
     "Check memory and open file usage at exit.", &debug_check_flag,
     &debug_check_flag, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"debug-info", OPT_DEBUG_INFO, "Print some debug info at exit.",
     &debug_info_flag, &debug_info_flag, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
#endif
    {"force", 'f',
     "Don't ask for confirmation on drop database; with multiple commands, "
     "continue even if an error occurs.",
     &option_force, &option_force, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"compress", 'C', "Use compression in server/client protocol.",
     &opt_compress, &opt_compress, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"character-sets-dir", OPT_CHARSETS_DIR,
     "Directory for character set files.", &charsets_dir, &charsets_dir,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"default-character-set", OPT_DEFAULT_CHARSET,
     "Set the default character set.", &default_charset, &default_charset,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"help", '?', "Display this help and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"host", 'h', "Connect to host.", &host, &host, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"no-beep", 'b', "Turn off beep on error.", &opt_nobeep, &opt_nobeep,
     nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"password", 'p',
     "Password to use when connecting to server. If password is not given it's "
     "asked from the tty.",
     nullptr, nullptr, nullptr, GET_PASSWORD, OPT_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
#ifdef _WIN32
    {"pipe", 'W', "Use named pipes to connect to server.", 0, 0, 0, GET_NO_ARG,
     NO_ARG, 0, 0, 0, 0, 0, 0},
#endif
    {"port", 'P',
     "Port number to use for connection or 0 for default to, in "
     "order of preference, my.cnf, $MYSQL_TCP_PORT, "
#if MYSQL_PORT_DEFAULT == 0
     "/etc/services, "
#endif
     "built-in default (" STRINGIFY_ARG(MYSQL_PORT) ").",
     &tcp_port, &tcp_port, nullptr, GET_UINT, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"protocol", OPT_MYSQL_PROTOCOL,
     "The protocol to use for connection (tcp, socket, pipe, memory).", nullptr,
     nullptr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"relative", 'r',
     "Show difference between current and previous values when used with -i. "
     "Currently only works with extended-status.",
     &opt_relative, &opt_relative, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
#if defined(_WIN32)
    {"shared-memory-base-name", OPT_SHARED_MEMORY_BASE_NAME,
     "Base name of shared memory.", &shared_memory_base_name,
     &shared_memory_base_name, 0, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, 0, 0,
     0},
#endif
    {"silent", 's', "Silently exit if one can't connect to server.", nullptr,
     nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"socket", 'S', "The socket file to use for connection.", &unix_port,
     &unix_port, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"sleep", 'i', "Execute commands repeatedly with a sleep between.",
     &interval, &interval, nullptr, GET_INT, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
#include "sslopt-longopts.h"

#include "caching_sha2_passwordopt-longopts.h"

    {"user", 'u', "User for login if not current user.", &user, &user, nullptr,
     GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"verbose", 'v', "Write more information.", &opt_verbose, &opt_verbose,
     nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"version", 'V', "Output version information and exit.", nullptr, nullptr,
     nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"vertical", 'E',
     "Print output vertically. Is similar to --relative, but prints output "
     "vertically.",
     &opt_vertical, &opt_vertical, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"wait", 'w', "Wait and retry if connection is down.", nullptr, nullptr,
     nullptr, GET_UINT, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"connect_timeout", OPT_CONNECT_TIMEOUT, "", &opt_connect_timeout,
     &opt_connect_timeout, nullptr, GET_ULONG, REQUIRED_ARG, 3600 * 12, 0,
     3600 * 12, nullptr, 1, nullptr},
    {"shutdown_timeout", OPT_SHUTDOWN_TIMEOUT, "", &opt_shutdown_timeout,
     &opt_shutdown_timeout, nullptr, GET_ULONG, REQUIRED_ARG,
     SHUTDOWN_DEF_TIMEOUT, 0, 3600 * 12, nullptr, 1, nullptr},
    {"plugin_dir", OPT_PLUGIN_DIR, "Directory for client-side plugins.",
     &opt_plugin_dir, &opt_plugin_dir, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"default_auth", OPT_DEFAULT_AUTH,
     "Default authentication client-side plugin to use.", &opt_default_auth,
     &opt_default_auth, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"enable_cleartext_plugin", OPT_ENABLE_CLEARTEXT_PLUGIN,
     "Enable/disable the clear text authentication plugin.",
     &opt_enable_cleartext_plugin, &opt_enable_cleartext_plugin, nullptr,
     GET_BOOL, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"show_warnings", OPT_SHOW_WARNINGS, "Show warnings after execution",
     &opt_show_warnings, &opt_show_warnings, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"compression-algorithms", 0,
     "Use compression algorithm in server/client protocol. Valid values "
     "are any combination of 'zstd','zlib','uncompressed'.",
     &opt_compress_algorithm, &opt_compress_algorithm, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"zstd-compression-level", 0,
     "Use this compression level in the client/server protocol, in case "
     "--compression-algorithms=zstd. Valid range is between 1 and 22, "
     "inclusive. Default is 3.",
     &opt_zstd_compress_level, &opt_zstd_compress_level, nullptr, GET_UINT,
     REQUIRED_ARG, 3, 1, 22, nullptr, 0, nullptr},

    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

static const char *load_default_groups[] = {"mysqladmin", "client", nullptr};

bool get_one_option(int optid,
                    const struct my_option *opt MY_ATTRIBUTE((unused)),
                    char *argument) {
  int error = 0;

  switch (optid) {
    case 'c':
      opt_count_iterations = 1;
      break;
    case 'p':
      if (argument == disabled_my_option) {
        // Don't require password
        static char empty_password[] = {'\0'};
        DBUG_ASSERT(empty_password[0] ==
                    '\0');  // Check that it has not been overwritten
        argument = empty_password;
      }
      if (argument) {
        char *start = argument;
        my_free(opt_password);
        opt_password = my_strdup(PSI_NOT_INSTRUMENTED, argument, MYF(MY_FAE));
        while (*argument) *argument++ = 'x'; /* Destroy argument */
        if (*start) start[1] = 0;            /* Cut length of argument */
        tty_password = false;
      } else
        tty_password = true;
      break;
    case 's':
      option_silent++;
      break;
    case 'W':
#ifdef _WIN32
      opt_protocol = MYSQL_PROTOCOL_PIPE;
#endif
      break;
    case '#':
      DBUG_PUSH(argument ? argument : "d:t:o,/tmp/mysqladmin.trace");
      break;
#include "sslopt-case.h"

    case 'V':
      print_version();
      exit(0);
      break;
    case 'w':
      if (argument) {
        if ((option_wait = atoi(argument)) <= 0) option_wait = 1;
      } else
        option_wait = ~(uint)0;
      break;
    case '?':
    case 'I': /* Info */
      error++;
      break;
    case OPT_CHARSETS_DIR:
      charsets_dir = argument;
      break;
    case OPT_MYSQL_PROTOCOL:
      opt_protocol =
          find_type_or_exit(argument, &sql_protocol_typelib, opt->name);
      break;
    case OPT_ENABLE_CLEARTEXT_PLUGIN:
      using_opt_enable_cleartext_plugin = true;
      break;
  }
  if (error) {
    usage();
    exit(1);
  }
  return false;
}

int main(int argc, char *argv[]) {
  int error = 0, ho_error, temp_argc;
  int first_command;
  bool can_handle_passwords;
  MYSQL mysql;
  char **commands, **temp_argv;

  MY_INIT(argv[0]);
  mysql_init(&mysql);
  my_getopt_use_args_separator = true;
  MEM_ROOT alloc{PSI_NOT_INSTRUMENTED, 512};
  if (load_defaults("my", load_default_groups, &argc, &argv, &alloc))
    return EXIT_FAILURE;
  my_getopt_use_args_separator = false;

  if ((ho_error =
           handle_options(&argc, &argv, my_long_options, get_one_option))) {
    mysql_close(&mysql);
    my_end(my_end_arg);
    return ho_error;
  }

  if (debug_info_flag) my_end_arg = MY_CHECK_ERROR | MY_GIVE_INFO;
  if (debug_check_flag) my_end_arg = MY_CHECK_ERROR;

  if (argc == 0) {
    usage();
    return EXIT_FAILURE;
  }

  temp_argv = mask_password(argc, &argv);
  temp_argc = argc;

  commands = temp_argv;
  if (tty_password) opt_password = get_tty_password(NullS);

  (void)signal(SIGINT, endprog);  /* Here if abort */
  (void)signal(SIGTERM, endprog); /* Here if abort */

  if (opt_bind_addr) mysql_options(&mysql, MYSQL_OPT_BIND, opt_bind_addr);
  if (opt_compress) mysql_options(&mysql, MYSQL_OPT_COMPRESS, NullS);
  if (opt_connect_timeout) {
    uint tmp = opt_connect_timeout;
    mysql_options(&mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&tmp);
  }
  if (SSL_SET_OPTIONS(&mysql)) {
    fprintf(stderr, "%s", SSL_SET_OPTIONS_ERROR);
    return EXIT_FAILURE;
  }
  if (opt_protocol)
    mysql_options(&mysql, MYSQL_OPT_PROTOCOL, (char *)&opt_protocol);
#if defined(_WIN32)
  if (shared_memory_base_name)
    mysql_options(&mysql, MYSQL_SHARED_MEMORY_BASE_NAME,
                  shared_memory_base_name);
#endif
  mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, default_charset);
  error_flags = (myf)(opt_nobeep ? 0 : ME_BELL);

  if (opt_compress_algorithm)
    mysql_options(&mysql, MYSQL_OPT_COMPRESSION_ALGORITHMS,
                  opt_compress_algorithm);

  mysql_options(&mysql, MYSQL_OPT_ZSTD_COMPRESSION_LEVEL,
                &opt_zstd_compress_level);

  if (opt_plugin_dir && *opt_plugin_dir)
    mysql_options(&mysql, MYSQL_PLUGIN_DIR, opt_plugin_dir);

  if (opt_default_auth && *opt_default_auth)
    mysql_options(&mysql, MYSQL_DEFAULT_AUTH, opt_default_auth);

  mysql_options(&mysql, MYSQL_OPT_CONNECT_ATTR_RESET, nullptr);
  mysql_options4(&mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "program_name",
                 "mysqladmin");
  if (using_opt_enable_cleartext_plugin)
    mysql_options(&mysql, MYSQL_ENABLE_CLEARTEXT_PLUGIN,
                  (char *)&opt_enable_cleartext_plugin);

  first_command = find_type(argv[0], &command_typelib, FIND_TYPE_BASIC);
  can_handle_passwords = first_command == ADMIN_PASSWORD ? true : false;
  mysql_options(&mysql, MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS,
                &can_handle_passwords);

  set_server_public_key(&mysql);
  set_get_server_public_key_option(&mysql);
  if (sql_connect(&mysql, option_wait)) {
    /*
      We couldn't get an initial connection and will definitely exit.
      The following just determines the exit-code we'll give.
    */

    unsigned int err = mysql_errno(&mysql);
    if (err >= CR_MIN_ERROR && err <= CR_MAX_ERROR)
      error = 1;
    else {
      /* Return 0 if all commands are PING */
      for (; argc > 0; argv++, argc--) {
        if (find_type(argv[0], &command_typelib, FIND_TYPE_BASIC) !=
            ADMIN_PING) {
          error = 1;
          break;
        }
      }
    }
  } else {
    /*
      --count=0 aborts right here. Otherwise iff --sleep=t ("interval")
      is given a t!=0, we get an endless loop, or n iterations if --count=n
      was given an n!=0. If --sleep wasn't given, we get one iteration.

      To wit, --wait loops the connection-attempts, while --sleep loops
      the command execution (endlessly if no --count is given).
    */

    while (!interrupted && (!opt_count_iterations || nr_iterations)) {
      new_line = false;

      if ((error = execute_commands(&mysql, argc, commands))) {
        /*
          Unknown/malformed command always aborts and can't be --forced.
          If the user got confused about the syntax, proceeding would be
          dangerous ...
        */
        if (error > 0) break;

        /*
          Command was well-formed, but failed on the server. Might succeed
          on retry (if conditions on server change etc.), but needs --force
          to retry.
        */
        if (!option_force) break;
      } /* if((error= ... */

      if (interval) /* --sleep=interval given */
      {
        if (opt_count_iterations && --nr_iterations == 0) break;

        /*
          If connection was dropped (unintentionally, or due to SHUTDOWN),
          re-establish it if --wait ("retry-connect") was given and user
          didn't signal for us to die. Otherwise, signal failure.
        */

        if (mysql.net.vio == nullptr) {
          if (option_wait && !interrupted) {
            sleep(1);
            sql_connect(&mysql, option_wait);
            /*
              continue normally and decrease counters so that
              "mysqladmin --count=1 --wait=1 shutdown"
              cannot loop endlessly.
            */
          } else {
            /*
              connexion broke, and we have no order to re-establish it. fail.
            */
            if (!option_force) error = 1;
            break;
          }
        } /* lost connection */

        sleep(interval);
        if (new_line) puts("");
      } else
        break; /* no --sleep, done looping */
    }          /* command-loop */
  }            /* got connection */

  mysql_close(&mysql);
  my_free(opt_password);
  my_free(user);
#if defined(_WIN32)
  my_free(shared_memory_base_name);
#endif
  temp_argc--;
  while (temp_argc >= 0) {
    my_free(temp_argv[temp_argc]);
    temp_argc--;
  }
  my_free(temp_argv);
  my_end(my_end_arg);
  return error ? EXIT_FAILURE : EXIT_SUCCESS;
}

void endprog(int signal_number MY_ATTRIBUTE((unused))) { interrupted = true; }

/**
   @brief connect to server, optionally waiting for same to come up

   @param  mysql     connection struct
   @param  wait      wait for server to come up?
                     (0: no, ~0: forever, n: cycles)

   @return Operation result
   @retval 0         success
   @retval 1         failure
*/

static bool sql_connect(MYSQL *mysql, uint wait) {
  bool info = false;

  for (;;) {
    if (mysql_real_connect(mysql, host, user, opt_password, NullS, tcp_port,
                           unix_port, CLIENT_REMEMBER_OPTIONS)) {
      mysql->reconnect = true;
      if (info) {
        fputs("\n", stderr);
        (void)fflush(stderr);
      }
      return false;
    }

    if (!wait)  // was or reached 0, fail
    {
      if (!option_silent)  // print diagnostics
      {
        if (!host) host = LOCAL_HOST;
        my_printf_error(0, "connect to server at '%s' failed\nerror: '%s'",
                        error_flags, host, mysql_error(mysql));
        if (mysql_errno(mysql) == CR_CONNECTION_ERROR) {
          fprintf(stderr,
                  "Check that mysqld is running and that the socket: '%s' "
                  "exists!\n",
                  unix_port ? unix_port : mysql_unix_port);
        } else if (mysql_errno(mysql) == CR_CONN_HOST_ERROR ||
                   mysql_errno(mysql) == CR_UNKNOWN_HOST) {
          fprintf(stderr, "Check that mysqld is running on %s", host);
          fprintf(stderr, " and that the port is %d.\n",
                  tcp_port ? tcp_port : mysql_port);
          fprintf(stderr, "You can check this by doing 'telnet %s %d'\n", host,
                  tcp_port ? tcp_port : mysql_port);
        }
      }
      return true;
    }

    if (wait != (uint)~0) wait--; /* count down, one less retry */

    if ((mysql_errno(mysql) != CR_CONN_HOST_ERROR) &&
        (mysql_errno(mysql) != CR_CONNECTION_ERROR)) {
      /*
        Error is worse than "server doesn't answer (yet?)";
        fail even if we still have "wait-coins" unless --force
        was also given.
      */
      fprintf(stderr, "Got error: %s\n", mysql_error(mysql));
      if (!option_force) return true;
    } else if (!option_silent) {
      if (!info) {
        info = true;
        fputs("Waiting for MySQL server to answer", stderr);
        (void)fflush(stderr);
      } else {
        putc('.', stderr);
        (void)fflush(stderr);
      }
    }
    sleep(5);
  }
}

/**
   @brief Execute all commands

   @details We try to execute all commands we were given, in the order
            given, but return with non-zero as soon as we encounter trouble.
            By that token, individual commands can be considered a conjunction
            with boolean short-cut.

   @return success?
   @retval 0       Yes!  ALL commands worked!
   @retval 1       No, one failed and will never work (malformed): fatal error!
   @retval -1      No, one failed on the server, may work next time!
*/

static int execute_commands(MYSQL *mysql, int argc, char **argv) {
  const char *status;
  /*
    MySQL documentation relies on the fact that mysqladmin will
    execute commands in the order specified, e.g.
    mysqladmin -u root flush-privileges password "newpassword"
    to reset a lost root password.
    If this behaviour is ever changed, Docs should be notified.
  */

  struct rand_struct rand_st;

  for (; argc > 0; argv++, argc--) {
    int option;
    bool log_warnings = true;
    switch (option = find_type(argv[0], &command_typelib, FIND_TYPE_BASIC)) {
      case ADMIN_CREATE: {
        char buff[FN_REFLEN + 20];
        if (argc < 2) {
          my_printf_error(0, "Too few arguments to create", error_flags);
          return 1;
        }
        sprintf(buff, "create database `%.*s`", FN_REFLEN, argv[1]);
        if (mysql_query(mysql, buff)) {
          my_printf_error(0, "CREATE DATABASE failed; error: '%-.200s'",
                          error_flags, mysql_error(mysql));
          return -1;
        }
        argc--;
        argv++;
        break;
      }
      case ADMIN_DROP: {
        if (argc < 2) {
          my_printf_error(0, "Too few arguments to drop", error_flags);
          return 1;
        }
        if (drop_db(mysql, argv[1])) return -1;
        argc--;
        argv++;
        break;
      }
      case ADMIN_SHUTDOWN: {
        char pidfile[FN_REFLEN];
        bool got_pidfile = false;
        time_t last_modified = 0;
        struct stat pidfile_status;

        /*
          Only wait for pidfile on local connections
          If pidfile doesn't exist, continue without pid file checking
        */
        if (mysql->unix_socket &&
            (got_pidfile = !get_pidfile(mysql, pidfile)) &&
            !stat(pidfile, &pidfile_status))
          last_modified = pidfile_status.st_mtime;

        if (mysql_shutdown(mysql, SHUTDOWN_DEFAULT)) {
          my_printf_error(0, "shutdown failed; error: '%s'", error_flags,
                          mysql_error(mysql));
          return -1;
        }

        argc = 1; /* force SHUTDOWN to be the last command    */
        if (got_pidfile) {
          if (opt_verbose)
            printf(
                "Shutdown signal sent to server;  Waiting for pid file to "
                "disappear\n");

          /* Wait until pid file is gone */
          if (wait_pidfile(pidfile, last_modified, &pidfile_status)) return -1;
        }
        /* Do not try to print warning as server has gone away */
        log_warnings = false;
        break;
      }
      case ADMIN_FLUSH_PRIVILEGES:
      case ADMIN_RELOAD:
        if (mysql_query(mysql, "flush privileges")) {
          my_printf_error(0, "reload failed; error: '%s'", error_flags,
                          mysql_error(mysql));
          return -1;
        }
        break;
      case ADMIN_REFRESH:
        if (mysql_refresh(mysql, (uint) ~(REFRESH_GRANT | REFRESH_STATUS |
                                          REFRESH_READ_LOCK | REFRESH_SLAVE |
                                          REFRESH_MASTER))) {
          my_printf_error(0, "refresh failed; error: '%s'", error_flags,
                          mysql_error(mysql));
          return -1;
        }
        break;
      case ADMIN_FLUSH_THREADS:
        if (mysql_refresh(mysql, (uint)REFRESH_THREADS)) {
          my_printf_error(0, "refresh failed; error: '%s'", error_flags,
                          mysql_error(mysql));
          return -1;
        }
        break;
      case ADMIN_VER:
        new_line = true;
        print_version();
        puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2000"));
        printf("Server version\t\t%s\n", mysql_get_server_info(mysql));
        printf("Protocol version\t%d\n", mysql_get_proto_info(mysql));
        printf("Connection\t\t%s\n", mysql_get_host_info(mysql));
        if (mysql->unix_socket)
          printf("UNIX socket\t\t%s\n", mysql->unix_socket);
        else
          printf("TCP port\t\t%d\n", mysql->port);
        status = mysql_stat(mysql);
        {
          char buff[40];
          ulong sec;
          char *pos = strchr(const_cast<char *>(status), ' ');
          *pos++ = 0;
          printf("%s\t\t\t", status); /* print label */
          if ((status = str2int(pos, 10, 0, LONG_MAX, (long *)&sec))) {
            nice_time(sec, buff);
            puts(buff);                      /* print nice time */
            while (*status == ' ') status++; /* to next info */
          }
        }
        putc('\n', stdout);
        if (status) puts(status);
        break;
      case ADMIN_PROCESSLIST: {
        MYSQL_RES *result;
        MYSQL_ROW row;

        if (mysql_query(mysql, (opt_verbose ? "show full processlist"
                                            : "show processlist")) ||
            !(result = mysql_store_result(mysql))) {
          my_printf_error(0, "process list failed; error: '%s'", error_flags,
                          mysql_error(mysql));
          return -1;
        }
        print_header(result);
        while ((row = mysql_fetch_row(result))) print_row(result, row, 0);
        print_top(result);
        mysql_free_result(result);
        new_line = true;
        break;
      }
      case ADMIN_STATUS:
        status = mysql_stat(mysql);
        if (status) puts(status);
        break;
      case ADMIN_KILL: {
        uint error = 0;
        char *pos;
        if (argc < 2) {
          my_printf_error(0, "Too few arguments to 'kill'", error_flags);
          return 1;
        }
        pos = argv[1];
        for (;;) {
          /* We don't use mysql_kill(), since it only handles 32-bit IDs. */
          char buff[26], *out; /* "KILL " + max 20 digs + NUL */
          out = strxmov(buff, "KILL ", NullS);
          ullstr(my_strtoull(pos, nullptr, 0), out);

          if (mysql_query(mysql, buff)) {
            /* out still points to just the number */
            my_printf_error(0, "kill failed on %s; error: '%s'", error_flags,
                            out, mysql_error(mysql));
            error = 1;
          }
          if (!(pos = strchr(pos, ','))) break;
          pos++;
        }
        argc--;
        argv++;
        if (error) return -1;
        break;
      }
      case ADMIN_DEBUG:
        if (mysql_dump_debug_info(mysql)) {
          my_printf_error(0, "debug failed; error: '%s'", error_flags,
                          mysql_error(mysql));
          return -1;
        }
        break;
      case ADMIN_VARIABLES: {
        MYSQL_RES *res;
        MYSQL_ROW row;

        new_line = true;
        if (mysql_query(mysql, "show /*!40003 GLOBAL */ variables") ||
            !(res = mysql_store_result(mysql))) {
          my_printf_error(0, "unable to show variables; error: '%s'",
                          error_flags, mysql_error(mysql));
          return -1;
        }
        print_header(res);
        while ((row = mysql_fetch_row(res))) print_row(res, row, 0);
        print_top(res);
        mysql_free_result(res);
        break;
      }
      case ADMIN_EXTENDED_STATUS: {
        MYSQL_RES *res;
        MYSQL_ROW row;
        uint rownr = 0;
        void (*func)(MYSQL_RES *, MYSQL_ROW, uint);

        new_line = true;
        if (mysql_query(mysql, "show /*!50002 GLOBAL */ status") ||
            !(res = mysql_store_result(mysql))) {
          my_printf_error(0, "unable to show status; error: '%s'", error_flags,
                          mysql_error(mysql));
          return -1;
        }

        if (mysql_num_rows(res) >= MAX_MYSQL_VAR) {
          my_printf_error(0,
                          "Too many rows returned: '%lu'. "
                          "Expecting no more than '%d' rows",
                          error_flags, mysql_num_rows(res), MAX_MYSQL_VAR);
          return -1;
        }

        if (!opt_vertical)
          print_header(res);
        else {
          if (!ex_status_printed) {
            store_values(res);
            truncate_names(); /* Does some printing also */
          } else {
            print_relative_line();
            print_relative_header();
            print_relative_line();
          }
        }

        /*      void (*func) (MYSQL_RES*, MYSQL_ROW, uint); */
        if (opt_relative && !opt_vertical)
          func = print_relative_row;
        else if (opt_vertical)
          func = print_relative_row_vert;
        else
          func = print_row;

        while ((row = mysql_fetch_row(res))) (*func)(res, row, rownr++);
        if (opt_vertical) {
          if (ex_status_printed) {
            putchar('\n');
            print_relative_line();
          }
        } else
          print_top(res);

        ex_status_printed = true; /* From now on the output will be relative */
        mysql_free_result(res);
        break;
      }
      case ADMIN_FLUSH_LOGS: {
        std::string command;
        if (argc > 1) {
          bool first_arg = true;
          for (command = "FLUSH "; argc > 1; argc--, argv++) {
            if (!first_arg) command += ",";

            if (!my_strcasecmp(&my_charset_latin1, argv[1], "binary"))
              command += " BINARY LOGS";
            else if (!my_strcasecmp(&my_charset_latin1, argv[1], "engine"))
              command += " ENGINE LOGS";
            else if (!my_strcasecmp(&my_charset_latin1, argv[1], "error"))
              command += " ERROR LOGS";
            else if (!my_strcasecmp(&my_charset_latin1, argv[1], "general"))
              command += " GENERAL LOGS";
            else if (!my_strcasecmp(&my_charset_latin1, argv[1], "relay"))
              command += " RELAY LOGS";
            else if (!my_strcasecmp(&my_charset_latin1, argv[1], "slow"))
              command += " SLOW LOGS";
            else {
              /*
                Not a valid log type, assume it's the next command.
                Remove the trailing comma if any of the log types is specified
                or flush all if no specific log type is specified.
              */
              if (!first_arg)
                command.resize(command.size() - 1);
              else
                command = "FLUSH LOGS";
              break;
            }

            if (first_arg) first_arg = false;
          }
        } else
          command = "FLUSH LOGS";
        if (mysql_query(mysql, command.c_str())) {
          my_printf_error(0, "refresh failed; error: '%s'", error_flags,
                          mysql_error(mysql));
          return -1;
        }
        break;
      }
      case ADMIN_FLUSH_HOSTS: {
        if (mysql_query(mysql, "flush hosts")) {
          my_printf_error(0, "refresh failed; error: '%s'", error_flags,
                          mysql_error(mysql));
          return -1;
        }
        break;
      }
      case ADMIN_FLUSH_TABLES: {
        if (mysql_query(mysql, "flush tables")) {
          my_printf_error(0, "refresh failed; error: '%s'", error_flags,
                          mysql_error(mysql));
          return -1;
        }
        break;
      }
      case ADMIN_FLUSH_STATUS: {
        if (mysql_query(mysql, "flush status")) {
          my_printf_error(0, "refresh failed; error: '%s'", error_flags,
                          mysql_error(mysql));
          return -1;
        }
        break;
      }
      case ADMIN_PASSWORD: {
        char buff[128];
        time_t start_time;
        char *typed_password = nullptr, *verified = nullptr, *tmp = nullptr;
        bool log_off = true, err = false;
        size_t password_len;

        /* Do initialization the same way as we do in mysqld */
        start_time = time((time_t *)nullptr);
        randominit(&rand_st, (ulong)start_time, (ulong)start_time / 2);

        if (argc < 1) {
          my_printf_error(0, "Too few arguments to change password",
                          error_flags);
          return 1;
        } else if (argc == 1) {
          /* prompt for password */
          typed_password = get_tty_password("New password: ");
          verified = get_tty_password("Confirm new password: ");
          if (strcmp(typed_password, verified) != 0) {
            my_printf_error(0, "Passwords don't match", MYF(ME_BELL));
            err = true;
            goto error;
          }
          /* escape quotes if password has any special characters */
          password_len = strlen(typed_password);
          tmp = (char *)my_malloc(PSI_NOT_INSTRUMENTED, password_len * 2 + 1,
                                  MYF(MY_WME));
          mysql_real_escape_string(mysql, tmp, typed_password,
                                   (ulong)password_len);
          typed_password = tmp;
        } else {
          typed_password = argv[1];
        }

        if (typed_password[0]) {
#ifdef _WIN32
          size_t pw_len = strlen(typed_password);
          if (pw_len > 1 && typed_password[0] == '\'' &&
              typed_password[pw_len - 1] == '\'')
            printf(
                "Warning: single quotes were not trimmed from the password by"
                " your command\nline client, as you might have expected.\n");
#endif
          /* turn logging off if we can */
          if (mysql_query(mysql, "set sql_log_off=1")) {
            if (opt_verbose)
              fprintf(stderr, "Note: Can't turn off logging; '%s'",
                      mysql_error(mysql));
            log_off = false;
          }

          /*
            In case the password_expired flag is set ('Y'), then there is no way
            to determine the password format. So, assume that setting the
            password using the server's default authentication format
            (mysql_native_password) will work.
            TODO: make sure this always uses SSL and then let the server
            calculate the scramble.
          */
        }

        /* Warn about password being set in non ssl connection */
        {
          uint ssl_mode = 0;
          if (!mysql_get_option(mysql, MYSQL_OPT_SSL_MODE, &ssl_mode) &&
              ssl_mode <= SSL_MODE_PREFERRED) {
            fprintf(
                stderr,
                "Warning: Since password will be sent to server in "
                "plain text, use ssl connection to ensure password safety.\n");
          }
        }

        memset(buff, 0, sizeof(buff));
        sprintf(buff, "ALTER USER USER() IDENTIFIED BY '%s'", typed_password);

        if (mysql_query(mysql, buff)) {
          if (mysql_errno(mysql) != 1290) {
            my_printf_error(0, "unable to change password; error: '%s'",
                            error_flags, mysql_error(mysql));
            err = true;
            goto error;
          } else {
            /*
              We don't try to execute 'update mysql.user set..'
              because we can't perfectly find out the host
             */
            my_printf_error(0,
                            "\n"
                            "You cannot use 'password' command as mysqld runs\n"
                            " with grant tables disabled (was started with"
                            " --skip-grant-tables).\n"
                            "Use: \"mysqladmin flush-privileges password '*'\""
                            " instead",
                            error_flags);
            err = true;
            goto error;
          }
        }
        /*
          We may call set sql_log_off after this so check for warnings here.
        */
        if (opt_show_warnings) {
          print_warnings(mysql);
          log_warnings = false;
        }
        if (log_off && mysql_query(mysql, "set sql_log_off=0")) {
          if (opt_verbose)
            fprintf(stderr, "Note: Can't turn on logging; '%s'",
                    mysql_error(mysql));
        }
      error:
        /* free up memory from prompted password */
        if (typed_password != argv[1]) {
          my_free(typed_password);
          my_free(verified);
        }
        if (err) return -1;

        argc--;
        argv++;
        break;
      }

      case ADMIN_START_SLAVE:
        if (mysql_query(mysql, "START SLAVE")) {
          my_printf_error(0, "Error starting slave: %s", error_flags,
                          mysql_error(mysql));
          return -1;
        } else
          puts("Slave started");
        break;
      case ADMIN_STOP_SLAVE:
        if (mysql_query(mysql, "STOP SLAVE")) {
          my_printf_error(0, "Error stopping slave: %s", error_flags,
                          mysql_error(mysql));
          return -1;
        } else
          puts("Slave stopped");
        break;

      case ADMIN_PING:
        mysql->reconnect = false; /* We want to know of reconnects */
        if (!mysql_ping(mysql)) {
          if (option_silent < 2) puts("mysqld is alive");
        } else {
          if (mysql_errno(mysql) == CR_SERVER_GONE_ERROR) {
            mysql->reconnect = true;
            if (!mysql_ping(mysql))
              puts("connection was down, but mysqld is now alive");
          } else {
            my_printf_error(0, "mysqld doesn't answer to ping, error: '%s'",
                            error_flags, mysql_error(mysql));
            return -1;
          }
        }
        mysql->reconnect = true; /* Automatic reconnect is default */
        break;
      default:
        my_printf_error(0, "Unknown command: '%-.60s'", error_flags, argv[0]);
        return 1;
    }
    if (opt_show_warnings && log_warnings) print_warnings(mysql);
  }
  return 0;
}

/**
   @brief Masking the password if it is passed as command line argument.

   @details It works in Linux and changes cmdline in ps and /proc/pid/cmdline,
            but it won't work for history file of shell.
            The command line arguments are copied to another array and the
            password in the argv is masked. This function is called just after
            "handle_options" because in "handle_options", the agrv pointers
            are altered which makes freeing of dynamically allocated memory
            difficult. The password masking is done before all other operations
            in order to minimise the time frame of password visibility via
   cmdline.

   @param argc            command line options (count)
   @param argv            command line options (values)

   @return temp_argv      copy of argv
*/

static char **mask_password(int argc, char ***argv) {
  char **temp_argv;
  temp_argv = (char **)(my_malloc(PSI_NOT_INSTRUMENTED, sizeof(char *) * argc,
                                  MYF(MY_WME)));
  argc--;
  while (argc > 0) {
    temp_argv[argc] =
        my_strdup(PSI_NOT_INSTRUMENTED, (*argv)[argc], MYF(MY_FAE));
    if (find_type((*argv)[argc - 1], &command_typelib, FIND_TYPE_BASIC) ==
        ADMIN_PASSWORD) {
      char *start = (*argv)[argc];
      while (*start) *start++ = 'x';
      start = (*argv)[argc];
      if (*start) start[1] = 0; /* Cut length of argument */
    }
    argc--;
  }
  temp_argv[argc] = my_strdup(PSI_NOT_INSTRUMENTED, (*argv)[argc], MYF(MY_FAE));
  return (temp_argv);
}

static void usage(void) {
  print_version();
  puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2000"));
  puts("Administration program for the mysqld daemon.");
  printf("Usage: %s [OPTIONS] command command....\n", my_progname);
  my_print_help(my_long_options);
  my_print_variables(my_long_options);
  print_defaults("my", load_default_groups);
  puts(
      "\nWhere command is a one or more of: (Commands may be shortened)\n\
  create databasename	Create a new database\n\
  debug			Instruct server to write debug information to log\n\
  drop databasename	Delete a database and all its tables\n\
  extended-status       Gives an extended status message from the server\n\
  flush-hosts           Flush all cached hosts\n\
  flush-logs            Flush all logs\n\
  flush-status		Clear status variables\n\
  flush-tables          Flush all tables\n\
  flush-threads         Flush the thread cache\n\
  flush-privileges      Reload grant tables (same as reload)\n\
  kill id,id,...	Kill mysql threads");
  puts(
      "\
  password [new-password] Change old password to new-password in current format");
  puts(
      "\
  ping			Check if mysqld is alive\n\
  processlist		Show list of active threads in server\n\
  reload		Reload grant tables\n\
  refresh		Flush all tables and close and open logfiles\n\
  shutdown		Take server down\n\
  status		Gives a short status message from the server\n\
  start-slave		Start slave\n\
  stop-slave		Stop slave\n\
  variables             Prints variables available\n\
  version		Get version info from server");
}

static int drop_db(MYSQL *mysql, const char *db) {
  char name_buff[FN_REFLEN + 20], buf[10];
  char *input;

  if (!option_force) {
    puts("Dropping the database is potentially a very bad thing to do.");
    puts("Any data stored in the database will be destroyed.\n");
    printf("Do you really want to drop the '%s' database [y/N] ", db);
    fflush(stdout);
    input = fgets(buf, sizeof(buf) - 1, stdin);
    if (!input || ((*input != 'y') && (*input != 'Y'))) {
      puts("\nOK, aborting database drop!");
      return -1;
    }
  }
  sprintf(name_buff, "drop database `%.*s`", FN_REFLEN, db);
  if (mysql_query(mysql, name_buff)) {
    my_printf_error(0, "DROP DATABASE %s failed;\nerror: '%s'", error_flags, db,
                    mysql_error(mysql));
    return 1;
  }
  printf("Database \"%s\" dropped\n", db);
  return 0;
}

static void nice_time(ulong sec, char *buff) {
  ulong tmp;

  if (sec >= 3600L * 24) {
    tmp = sec / (3600L * 24);
    sec -= 3600L * 24 * tmp;
    buff = longlong10_to_str(tmp, buff, 10);
    buff = my_stpcpy(buff, tmp > 1 ? " days " : " day ");
  }
  if (sec >= 3600L) {
    tmp = sec / 3600L;
    sec -= 3600L * tmp;
    buff = longlong10_to_str(tmp, buff, 10);
    buff = my_stpcpy(buff, tmp > 1 ? " hours " : " hour ");
  }
  if (sec >= 60) {
    tmp = sec / 60;
    sec -= 60 * tmp;
    buff = longlong10_to_str(tmp, buff, 10);
    buff = my_stpcpy(buff, " min ");
  }
  my_stpcpy(longlong10_to_str(sec, buff, 10), " sec");
}

static void print_header(MYSQL_RES *result) {
  MYSQL_FIELD *field;

  print_top(result);
  mysql_field_seek(result, 0);
  putchar('|');
  while ((field = mysql_fetch_field(result))) {
    printf(" %-*s|", (int)field->max_length + 1, field->name);
  }
  putchar('\n');
  print_top(result);
}

static void print_top(MYSQL_RES *result) {
  uint i, length;
  MYSQL_FIELD *field;

  putchar('+');
  mysql_field_seek(result, 0);
  while ((field = mysql_fetch_field(result))) {
    if ((length = (uint)strlen(field->name)) > field->max_length)
      field->max_length = length;
    else
      length = field->max_length;
    for (i = length + 2; i-- > 0;) putchar('-');
    putchar('+');
  }
  putchar('\n');
}

/* 3.rd argument, uint row, is not in use. Don't remove! */
static void print_row(MYSQL_RES *result, MYSQL_ROW cur,
                      uint row MY_ATTRIBUTE((unused))) {
  uint i, length;
  MYSQL_FIELD *field;

  putchar('|');
  mysql_field_seek(result, 0);
  for (i = 0; i < mysql_num_fields(result); i++) {
    field = mysql_fetch_field(result);
    length = field->max_length;
    printf(" %-*s|", length + 1, cur[i] ? (char *)cur[i] : "");
  }
  putchar('\n');
}

static void print_relative_row(MYSQL_RES *result, MYSQL_ROW cur, uint row) {
  ulonglong tmp;
  char buff[22];
  MYSQL_FIELD *field;

  mysql_field_seek(result, 0);
  field = mysql_fetch_field(result);
  printf("| %-*s|", (int)field->max_length + 1, cur[0]);

  field = mysql_fetch_field(result);
  tmp = cur[1] ? my_strtoull(cur[1], nullptr, 10) : (ulonglong)0;
  printf(" %-*s|\n", (int)field->max_length + 1,
         llstr((tmp - last_values[row]), buff));
  last_values[row] = tmp;
}

static void print_relative_row_vert(MYSQL_RES *result MY_ATTRIBUTE((unused)),
                                    MYSQL_ROW cur, uint row) {
  uint length;
  ulonglong tmp;
  char buff[22];

  if (!row) putchar('|');

  tmp = cur[1] ? my_strtoull(cur[1], nullptr, 10) : (ulonglong)0;
  printf(" %-*s|", ex_val_max_len[row] + 1,
         llstr((tmp - last_values[row]), buff));

  /* Find the minimum row length needed to output the relative value */
  if ((length = (uint)strlen(buff) > ex_val_max_len[row]) && ex_status_printed)
    ex_val_max_len[row] = length;
  last_values[row] = tmp;
}

static void store_values(MYSQL_RES *result) {
  uint i;
  MYSQL_ROW row;
  MYSQL_FIELD *field;

  field = mysql_fetch_field(result);
  max_var_length = field->max_length;
  field = mysql_fetch_field(result);
  max_val_length = field->max_length;

  for (i = 0; (row = mysql_fetch_row(result)); i++) {
    my_stpcpy(ex_var_names[i], row[0]);
    last_values[i] = my_strtoull(row[1], nullptr, 10);
    ex_val_max_len[i] = 2; /* Default print width for values */
  }
  ex_var_count = i;
  return;
}

static void print_relative_header() {
  uint i;

  putchar('|');
  for (i = 0; i < ex_var_count; i++)
    printf(" %-*s|", ex_val_max_len[i] + 1, truncated_var_names[i]);
  putchar('\n');
}

static void print_relative_line() {
  uint i;

  putchar('+');
  for (i = 0; i < ex_var_count; i++) {
    uint j;
    for (j = 0; j < ex_val_max_len[i] + 2; j++) putchar('-');
    putchar('+');
  }
  putchar('\n');
}

static void truncate_names() {
  uint i;
  char *ptr, top_line[MAX_TRUNC_LENGTH + 4 + NAME_LEN + 22 + 1], buff[22];

  ptr = top_line;
  *ptr++ = '+';
  ptr = strfill(ptr, max_var_length + 2, '-');
  *ptr++ = '+';
  ptr = strfill(ptr, MAX_TRUNC_LENGTH + 2, '-');
  *ptr++ = '+';
  ptr = strfill(ptr, max_val_length + 2, '-');
  *ptr++ = '+';
  *ptr = 0;
  puts(top_line);

  for (i = 0; i < ex_var_count; i++) {
    uint sfx = 1, j;
    printf("| %-*s|", max_var_length + 1, ex_var_names[i]);
    ptr = ex_var_names[i];
    /* Make sure no two same truncated names will become */
    for (j = 0; j < i; j++)
      if (*truncated_var_names[j] == *ptr) sfx++;

    truncated_var_names[i][0] = *ptr; /* Copy first var char */
    longlong10_to_str(sfx, truncated_var_names[i] + 1, 10);
    printf(" %-*s|", MAX_TRUNC_LENGTH + 1, truncated_var_names[i]);
    printf(" %-*s|\n", max_val_length + 1, llstr(last_values[i], buff));
  }
  puts(top_line);
  return;
}

static bool get_pidfile(MYSQL *mysql, char *pidfile) {
  MYSQL_RES *result;
  if (mysql_query(mysql, "SELECT @@datadir, @@pid_file")) {
    my_printf_error(mysql_errno(mysql),
                    "The query to get the server's pid file failed,"
                    " error: '%s'. Continuing.",
                    error_flags, mysql_error(mysql));
  }
  result = mysql_store_result(mysql);
  if (result) {
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row) {
      char datadir[FN_REFLEN];
      char pidfile_option[FN_REFLEN];
      my_stpcpy(datadir, row[0]);
      my_stpcpy(pidfile_option, row[1]);
      (void)my_load_path(pidfile, pidfile_option, datadir);
    }
    mysql_free_result(result);
    return row == nullptr; /* Error if row = 0 */
  }
  return true; /* Error */
}

/*
  Return 1 if pid file didn't disappear or change
*/

static bool wait_pidfile(char *pidfile, time_t last_modified,
                         struct stat *pidfile_status) {
  char buff[FN_REFLEN];
  int error = 1;
  uint count = 0;
  DBUG_TRACE;

  system_filename(buff, pidfile);
  do {
    int fd;
    if ((fd = my_open(buff, O_RDONLY, MYF(0))) < 0) {
      error = 0;
      break;
    }
    (void)my_close(fd, MYF(0));
    if (last_modified && !stat(pidfile, pidfile_status)) {
      if (last_modified != pidfile_status->st_mtime) {
        /* File changed;  Let's assume that mysqld did restart */
        if (opt_verbose)
          printf(
              "pid file '%s' changed while waiting for it to "
              "disappear!\nmysqld did probably restart\n",
              buff);
        error = 0;
        break;
      }
    }
    if (count++ == opt_shutdown_timeout) break;
    sleep(1);
  } while (!interrupted);

  if (error) {
    DBUG_PRINT("warning", ("Pid file didn't disappear"));
    fprintf(stderr,
            "Warning;  Aborted waiting on pid file: '%s' after %d seconds\n",
            buff, count - 1);
  }
  return error;
}

/*
  Print warning(s) generated by a statement execution
*/
static void print_warnings(MYSQL *mysql) {
  const char *query;
  MYSQL_RES *result = nullptr;
  MYSQL_ROW cur;
  uint64_t num_rows;
  uint error;

  /* Save current error before calling "show warnings" */
  error = mysql_errno(mysql);

  /* Get the warnings */
  query = "show warnings";
  if (mysql_real_query(mysql, query, static_cast<ulong>(strlen(query))) ||
      !(result = mysql_store_result(mysql))) {
    my_printf_error(0, "Could not print warnings; error: '%-.200s'",
                    error_flags, mysql_error(mysql));
  }

  /* Bail out when no warnings */
  if (!(num_rows = mysql_num_rows(result))) goto end;

  cur = mysql_fetch_row(result);

  /*
    Don't print a duplicate of the current error.  It is possible for SHOW
    WARNINGS to return multiple errors with the same code, but different
    messages.  To be safe, skip printing the duplicate only if it is the only
    warning.
  */
  if (!cur || (num_rows == 1 && error == (uint)strtoul(cur[1], nullptr, 10)))
    goto end;

  do {
    printf("%s (Code %s): %s\n", cur[0], cur[1], cur[2]);
  } while ((cur = mysql_fetch_row(result)));

end:
  mysql_free_result(result);
}
