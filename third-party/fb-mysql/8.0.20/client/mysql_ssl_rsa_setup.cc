/*
   Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <mysql_version.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "client/logger.h"
#include "client/path.h"
#ifdef _WIN32
#include "m_ctype.h"
#endif
#include "my_alloc.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_default.h"
#include "my_dir.h"
#include "my_getopt.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "print_version.h"
#include "welcome_copyright_notice.h" /* ORACLE_WELCOME_COPYRIGHT_NOTICE */

#if HAVE_CHOWN
#include <pwd.h>
#endif

/* Forward declarations */

using namespace std;
typedef string Sql_string_t;

static Sql_string_t create_string(const char *ptr);

/* Global Variables */
enum certs {
  CA_CERT = 0,
  CA_KEY,
  CA_REQ,
  SERVER_CERT,
  SERVER_KEY,
  SERVER_REQ,
  CLIENT_CERT,
  CLIENT_KEY,
  CLIENT_REQ,
  PRIVATE_KEY,
  PUBLIC_KEY,
  OPENSSL_RND
};

enum extfiles { CAV3_EXT = 0, CERTV3_EXT };

Sql_string_t cert_files[] = {
    create_string("ca.pem"),          create_string("ca-key.pem"),
    create_string("ca-req.pem"),      create_string("server-cert.pem"),
    create_string("server-key.pem"),  create_string("server-req.pem"),
    create_string("client-cert.pem"), create_string("client-key.pem"),
    create_string("client-req.pem"),  create_string("private_key.pem"),
    create_string("public_key.pem"),  create_string(".rnd")};

Sql_string_t ext_files[] = {create_string("cav3.ext"),
                            create_string("certv3.ext")};

#define MAX_PATH_LEN \
  (FN_REFLEN - strlen(FN_DIRSEP) - cert_files[SERVER_CERT].length() - 1)
/*
  Higest number of fixed characters in subject line is 47:
  MySQL_SERVER_<suffix>_Auto_Generated_Server_Certificate
  Maximum size of subject is 64. So suffix can't be longer
  than 17 characters.
*/
#define MAX_SUFFIX_LEN 17

Log info(cout, "NOTE");
Log error(cerr, "ERROR");

char **defaults_argv = nullptr;
static char *opt_datadir = nullptr;
static char default_data_dir[] = MYSQL_DATADIR;
static char *opt_suffix = nullptr;
static char default_suffix[] = MYSQL_SERVER_VERSION;
#if HAVE_CHOWN
static char *opt_userid = nullptr;
struct passwd *user_info = nullptr;
#endif /* HAVE_CHOWN */
Path dir_string;
Sql_string_t suffix_string;
bool opt_verbose;

static const char *load_default_groups[] = {"mysql_ssl_rsa_setup", "mysqld",
                                            nullptr};

static struct my_option my_options[] = {
    {"help", '?', "Display this help and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"verbose", 'v', "Be more verbose when running program", &opt_verbose,
     nullptr, nullptr, GET_BOOL, NO_ARG, false, 0, 0, nullptr, 0, nullptr},
    {"version", 'V', "Print program version and exit", nullptr, nullptr,
     nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"datadir", 'd', "Directory to store generated files.", &opt_datadir,
     &opt_datadir, nullptr, GET_STR_ALLOC, REQUIRED_ARG,
     (longlong)&default_data_dir, 0, 0, nullptr, 0, nullptr},
    {"suffix", 's', "Suffix to be added in certificate subject line",
     &opt_suffix, &opt_suffix, nullptr, GET_STR_ALLOC, REQUIRED_ARG,
     (longlong)&default_suffix, 0, 0, nullptr, 0, nullptr},
#if HAVE_CHOWN
    {"uid", 0, "The effective user id to be used for file permission",
     &opt_userid, &opt_userid, nullptr, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
#endif /* HAVE_CHOWN */
    /* END TOKEN */
    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr}};

/* Helper Functions */

/**
  The string class will break if constructed with a NULL pointer. This wrapper
  provides a systematic protection when importing char pointers.
 */
static Sql_string_t create_string(const char *ptr) {
  return (ptr ? Sql_string_t(ptr) : Sql_string_t(""));
}

static int execute_command(const Sql_string_t &command,
                           const Sql_string_t &error_message) {
  stringstream cmd_string;

  cmd_string << command;
  if (!opt_verbose) {
#ifndef _WIN32
    cmd_string << " > /dev/null 2>&1";
#else
    cmd_string << " > NUL 2>&1";
#endif /* _WIN32 */
  }

  info << "Executing : " << cmd_string.str() << endl;
  if (system(cmd_string.str().c_str())) {
    error << error_message << endl;
    return 1;
  }

  return 0;
}

static int set_file_pair_permission(const Sql_string_t &priv,
                                    const Sql_string_t &pub) {
  if (my_chmod(priv.c_str(), USER_READ | USER_WRITE, MYF(MY_FAE + MY_WME)) ||
      my_chmod(pub.c_str(), USER_READ | USER_WRITE | GROUP_READ | OTHERS_READ,
               MYF(MY_FAE + MY_WME))) {
    error << "Error setting file permissions for" << priv.c_str() << " and "
          << pub.c_str() << endl;
    return 1;
  }
#if HAVE_CHOWN
  if (user_info) {
    if (chown(priv.c_str(), user_info->pw_uid, user_info->pw_gid) ||
        chown(pub.c_str(), user_info->pw_uid, user_info->pw_gid)) {
      error << "Failed to change file permission" << endl;
      return 1;
    }
  }
#endif /* HAVE_CHOWN */
  return 0;
}

static bool file_exists(const Sql_string_t &filename) {
  MY_STAT file_stat;
  if (my_stat(filename.c_str(), &file_stat, MYF(0)) == nullptr) return false;

  return true;
}

static int remove_file(const Sql_string_t &filename, bool report_error = true) {
  if (my_delete(filename.c_str(), MYF(0))) {
    if (report_error) error << "Error deleting : " << filename << endl;
    return 1;
  }
  return 0;
}

static void free_resources() {
  if (opt_datadir) my_free(opt_datadir);
  if (opt_suffix) my_free(opt_suffix);
#if HAVE_CHOWN
  if (opt_userid) my_free(opt_userid);
#endif
}

class RSA_priv {
 public:
  RSA_priv(uint32_t key_size = 2048) : m_key_size(key_size) {}

  ~RSA_priv() {}

  Sql_string_t operator()(const Sql_string_t &key_file) {
    stringstream command;
    command << "openssl genrsa "
            << " -out " << key_file << " " << m_key_size;

    return command.str();
  }

 private:
  uint32_t m_key_size;
};

class RSA_pub {
 public:
  Sql_string_t operator()(const Sql_string_t &priv_key_file,
                          const Sql_string_t &pub_key_file) {
    stringstream command;
    command << "openssl rsa -in " << priv_key_file << " -pubout -out "
            << pub_key_file;
    return command.str();
  }
};

class X509_key {
 public:
  X509_key(const Sql_string_t &version, uint32_t validity = 10 * 365L)
      : m_validity(validity) {
    m_subj_prefix << "-subj /CN=MySQL_Server_" << version;
  }

  Sql_string_t operator()(Sql_string_t suffix, const Sql_string_t &key_file,
                          const Sql_string_t &req_file) {
    stringstream command;
    command << "openssl req -newkey rsa:2048 -days " << m_validity
            << " -nodes -keyout " << key_file << " " << m_subj_prefix.str()
            << suffix << " -out " << req_file << " && openssl rsa -in "
            << key_file << " -out " << key_file;

    return command.str();
  }

 private:
  uint32_t m_validity;
  stringstream m_subj_prefix;
};

class X509v3_ext_writer {
 public:
  X509v3_ext_writer() {
    m_cav3_ext_options << "basicConstraints=CA:TRUE" << std::endl;

    m_certv3_ext_options << "basicConstraints=CA:FALSE" << std::endl;
  }
  ~X509v3_ext_writer() {}

  bool operator()(const Sql_string_t &cav3_ext_file,
                  const Sql_string_t &certv3_ext_file) {
    if (!cav3_ext_file.length() || !certv3_ext_file.length()) return true;

    std::ofstream ext_file;

    ext_file.open(cav3_ext_file.c_str(), std::ios::out | std::ios::trunc);
    if (!ext_file.is_open()) return true;
    ext_file << m_cav3_ext_options.str();
    ext_file.close();

    ext_file.open(certv3_ext_file.c_str(), std::ios::out | std::ios::trunc);
    if (!ext_file.is_open()) {
      remove_file(cav3_ext_file.c_str(), false);
      return true;
    }
    ext_file << m_certv3_ext_options.str();
    ext_file.close();

    return false;
  }

 private:
  stringstream m_cav3_ext_options;
  stringstream m_certv3_ext_options;
};

class X509_cert {
 public:
  X509_cert(uint32_t validity = 10 * 365L) : m_validity(validity) {}

  ~X509_cert() {}

  Sql_string_t operator()(const Sql_string_t &req_file,
                          const Sql_string_t &cert_file, uint32_t serial,
                          bool self_signed, const Sql_string_t &sign_key_file,
                          const Sql_string_t &sign_cert_file,
                          const Sql_string_t &ext_file) {
    stringstream command;
    command << "openssl x509 -sha256 -days " << m_validity;
    command << " -extfile " << ext_file;
    command << " -set_serial " << serial << " -req -in " << req_file;
    if (self_signed)
      command << " -signkey " << sign_key_file;
    else
      command << " -CA " << sign_cert_file << " -CAkey " << sign_key_file;
    command << " -out " << cert_file;

    return command.str();
  }

 protected:
  uint32_t m_validity;
};

static void usage(void) {
  print_version();
  cout << (ORACLE_WELCOME_COPYRIGHT_NOTICE("2015")) << endl
       << "MySQL SSL Certificate and RSA Key Generation Utility" << endl
       << "Usage : " << my_progname << " [OPTIONS]" << endl;

  my_print_help(my_options);
  my_print_variables(my_options);
}

extern "C" {
static bool my_arguments_get_one_option(int optid, const struct my_option *,
                                        char *) {
  switch (optid) {
    case '?':
      usage();
      free_resources();
      exit(0);
    case 'V':
      print_version();
      free_resources();
      exit(0);
  }
  return false;
}
}

static inline bool is_not_alnum_underscore(char c) {
  return !(isalnum(c) || c == '_');
}

static bool check_suffix() {
  return (strcmp(opt_suffix, default_suffix) &&
          (find_if(suffix_string.begin(), suffix_string.end(),
                   is_not_alnum_underscore) != suffix_string.end()));
}

int main(int argc, char *argv[]) {
  int ret_val = 0;
  Sql_string_t openssl_check("openssl version");
  bool save_skip_unknown = my_getopt_skip_unknown;
  MEM_ROOT alloc{PSI_NOT_INSTRUMENTED, 512};

  MY_INIT(argv[0]);
  DBUG_TRACE;
  DBUG_PROCESS(argv[0]);

  /* Parse options : Command Line/Config file */

#ifdef _WIN32
  /* Convert command line parameters from UTF16LE to UTF8MB4. */
  my_win_translate_command_line_args(&my_charset_utf8mb4_bin, &argc, &argv);
#endif
  my_getopt_use_args_separator = true;
  if (load_defaults("my", load_default_groups, &argc, &argv, &alloc)) {
    my_end(0);
    free_resources();
    exit(1);
  }

  MY_MODE file_creation_mode = get_file_perm(USER_READ | USER_WRITE);
  MY_MODE saved_umask = umask(~(file_creation_mode));

  defaults_argv = argv;
  my_getopt_use_args_separator = false;
  my_getopt_skip_unknown = true;

  if (handle_options(&argc, &argv, my_options, my_arguments_get_one_option)) {
    error << "Error parsing options" << endl;
    ret_val = 1;
    goto end;
  }

  my_getopt_skip_unknown = save_skip_unknown;

  /* Process opt_verbose */
  if (opt_verbose != true) info.enabled(false);

  /* Process opt_datadir */

  dir_string.path(create_string(opt_datadir));
  if (dir_string.to_str().length() > MAX_PATH_LEN) {
    error << "Dir path is too long" << endl;
    ret_val = 1;
    goto end;
  }

  if (!dir_string.normalize_path() || !dir_string.exists()) {
    error << "Failed to access directory pointed by --datadir. "
          << "Please make sure that directory exists and is "
          << "accessible by mysql_ssl_rsa_setup. Supplied value : "
          << dir_string.to_str() << endl;
    ret_val = 1;
    goto end;
  }

  info << "Destination directory: " << dir_string.to_str() << endl;

  /* Process opt_suffix */

  suffix_string.append(opt_suffix);
  if (suffix_string.length() > MAX_SUFFIX_LEN) {
    error << "Maximum number of characters allowed as the value for "
          << "--suffix are " << MAX_SUFFIX_LEN << endl;
    ret_val = 1;
    goto end;
  }

  if (check_suffix()) {
    error << "Invalid string for --suffix option. Either use default value for "
          << "the option or provide a string with alphanumeric characters "
          << "and/or _ only." << endl;
    ret_val = 1;
    goto end;
  }

  if ((ret_val = execute_command(openssl_check,
                                 "Could not find OpenSSL on the system"))) {
    goto end;
  } else {
    char save_wd[FN_REFLEN];
    bool files_exist = false;
    Sql_string_t verify("openssl verify -CAfile ");

    if (my_getwd(save_wd, FN_REFLEN - 1, MYF(MY_WME))) {
      error << "Error saving current working directory" << endl;
      ret_val = 1;
      goto end;
    }

    if (my_setwd(dir_string.to_str().c_str(), MYF(MY_WME))) {
      error << "Error changing working directory" << endl;
      ret_val = 1;
      goto end;
    }
#if HAVE_CHOWN
    if (opt_userid && geteuid() == 0) {
      user_info = getpwnam(opt_userid);
      if (!user_info) {
        error << "Error fetching user information" << endl;
        ret_val = 1;
        goto end;
      }
    }
#endif /* HAVE_CHOWN */

    /*
      SSL Certificate Generation.
      1. Check for ca.pem, server_cert.pem and server_key.pem at
         the directory location provided by --dir.
      2. If none of these files are present, generate following
         files:
         ca.pem, ca_key.pem
         server_cert.pem, server_key.pem
         client_cert.pem, client_key.pem
      3. If everything goes smoothly, set permission on files.
    */

    files_exist = file_exists(cert_files[CA_CERT]) ||
                  file_exists(cert_files[SERVER_CERT]) ||
                  file_exists(cert_files[SERVER_KEY]);

    if (files_exist) {
      info << "Certificate files are present in given dir. Skipping generation."
           << endl;
    } else {
      Sql_string_t empty_string("");
      X509_key x509_key(suffix_string);
      X509_cert x509_cert;
      X509v3_ext_writer x509v3_ext_writer;

      /* Delete existing files if any */
      remove_file(cert_files[CA_REQ], false);
      remove_file(cert_files[SERVER_REQ], false);
      remove_file(cert_files[CLIENT_REQ], false);
      remove_file(cert_files[CLIENT_CERT], false);
      remove_file(cert_files[CLIENT_KEY], false);
      remove_file(cert_files[OPENSSL_RND], false);

      /* Remove existing v3 extension files */
      remove_file(ext_files[CAV3_EXT], false);
      remove_file(ext_files[CERTV3_EXT], false);

      /* Create v3 extension files */
      if (x509v3_ext_writer(ext_files[CAV3_EXT], ext_files[CERTV3_EXT]))
        goto end;

      /* Generate CA Key and Certificate */
      if ((ret_val =
               execute_command(x509_key("_Auto_Generated_CA_Certificate",
                                        cert_files[CA_KEY], cert_files[CA_REQ]),
                               "Error generating ca_key.pem and ca_req.pem")))
        goto end;

      if ((ret_val = execute_command(
               x509_cert(cert_files[CA_REQ], cert_files[CA_CERT], 1, true,
                         cert_files[CA_KEY], empty_string, ext_files[CAV3_EXT]),
               "Error generating ca_cert.pem")))
        goto end;

      /* Generate Server Key and Certificate */
      if ((ret_val = execute_command(
               x509_key("_Auto_Generated_Server_Certificate",
                        cert_files[SERVER_KEY], cert_files[SERVER_REQ]),
               "Error generating server_key.pem and server_req.pem")))
        goto end;

      if ((ret_val = execute_command(
               x509_cert(cert_files[SERVER_REQ], cert_files[SERVER_CERT], 2,
                         false, cert_files[CA_KEY], cert_files[CA_CERT],
                         ext_files[CERTV3_EXT]),
               "Error generating server_cert.pem")))
        goto end;

      /* Generate Client Key and Certificate */
      if ((ret_val = execute_command(
               x509_key("_Auto_Generated_Client_Certificate",
                        cert_files[CLIENT_KEY], cert_files[CLIENT_REQ]),
               "Error generating client_key.pem and client_req.pem")))
        goto end;

      if ((ret_val = execute_command(
               x509_cert(cert_files[CLIENT_REQ], cert_files[CLIENT_CERT], 3,
                         false, cert_files[CA_KEY], cert_files[CA_CERT],
                         ext_files[CERTV3_EXT]),
               "Error generating client_cert.pem")))
        goto end;

      /* Verify generated certificates */
      verify.append(cert_files[CA_CERT]);
      verify.append(" ");
      verify.append(cert_files[SERVER_CERT]);
      verify.append(" ");
      verify.append(cert_files[CLIENT_CERT]);
      if ((ret_val = execute_command(
               verify, "Verification of X509 certificates failed.")))
        goto end;

      /* Set permission */
      if ((ret_val = (set_file_pair_permission(cert_files[CA_KEY],
                                               cert_files[CA_CERT]) |
                      set_file_pair_permission(cert_files[SERVER_KEY],
                                               cert_files[SERVER_CERT]) |
                      set_file_pair_permission(cert_files[CLIENT_KEY],
                                               cert_files[CLIENT_CERT]))))
        goto end;

      /* Remove request files : Flag an error if we can't delete them. */
      if ((ret_val = remove_file(cert_files[CA_REQ]))) goto end;

      if ((ret_val = remove_file(cert_files[SERVER_REQ]))) goto end;

      if ((ret_val = remove_file(cert_files[CLIENT_REQ]))) goto end;

      remove_file(cert_files[OPENSSL_RND], false);

      /* Remove existing v3 extension files */
      remove_file(ext_files[CAV3_EXT], false);
      remove_file(ext_files[CERTV3_EXT], false);
    }

    /*
      RSA Key pair generation.
      1. Check if private_key.pem or public_key.pem are present at
         the directory location provided by --dir.
      2. If not, generate private_key.pem, public_key.pem and
         set permission after successful generation.
    */

    files_exist = file_exists(cert_files[PRIVATE_KEY]) ||
                  file_exists(cert_files[PUBLIC_KEY]);

    if (files_exist) {
      info << "RSA key files are present in given dir. Skipping generation."
           << endl;
    } else {
      RSA_priv rsa_priv;
      RSA_pub rsa_pub;

      /* Remove existing file if any */
      remove_file(cert_files[OPENSSL_RND], false);

      if ((ret_val = execute_command(rsa_priv(cert_files[PRIVATE_KEY]),
                                     "Error generating private_key.pem")))
        goto end;

      if ((ret_val = execute_command(
               rsa_pub(cert_files[PRIVATE_KEY], cert_files[PUBLIC_KEY]),
               "Error generating public_key.pem")))
        goto end;
      /* Set Permission */
      if ((ret_val = set_file_pair_permission(cert_files[PRIVATE_KEY],
                                              cert_files[PUBLIC_KEY])))
        goto end;

      remove_file(cert_files[OPENSSL_RND], false);
    }

    if (my_setwd(save_wd, MYF(MY_WME))) {
      error << "Error changing working directory" << endl;
      ret_val = 1;
      goto end;
    }
  }

  info << "Success!" << endl;

end:

  umask(saved_umask);
  free_resources();

  return ret_val;
}
