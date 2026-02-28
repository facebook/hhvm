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

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file sql/ssl_acceptor_context.cc
  @ref SslAcceptorContext implementation.
*/
#include "sql/ssl_acceptor_context.h"

#include <stdio.h>
#include <string.h>

#include "my_dir.h"
#include "my_getopt.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/status_var.h"
#include "mysqld_error.h"
#include "sql/auth/auth_common.h"
#include "sql/mysqld.h"
#include "sql/options_mysqld.h"
#include "sql/sql_class.h"
#include "sql/sql_initialize.h"
#include "sql/sys_vars.h"
#include "sql/sys_vars_shared.h"
#include "violite.h"

/**
  SSL context options

  Ideally these would have been static members of the @ref SslAcceptorContext
  class, but since Sys_var classes are such that they need these as parameters
  to global instances we do the next best thing and make these static so that
  the visibility is confined to the current file
*/
static const char *opt_ssl_ca = nullptr;
static const char *opt_ssl_key = nullptr;
static const char *opt_ssl_cert = nullptr;
static char *opt_ssl_capath = nullptr, *opt_ssl_cipher = nullptr,
            *opt_tls_ciphersuites = nullptr, *opt_ssl_crl = nullptr,
            *opt_ssl_crlpath = nullptr, *opt_tls_version = nullptr;

static PolyLock_mutex lock_ssl_ctx(&LOCK_tls_ctx_options);

SslAcceptorContext::SslAcceptorContextLockType *SslAcceptorContext::s_lock =
    nullptr;

void SslAcceptorContext::singleton_deinit() {
  delete s_lock;
  s_lock = nullptr;
}

void SslAcceptorContext::singleton_flush(enum enum_ssl_init_error *error,
                                         bool force) {
  SslAcceptorContext *newC = new SslAcceptorContext(true, false, error);
  if (*error != SSL_INITERR_NOERROR && !force) {
    delete newC;
    return;
  }
  s_lock->write_wait_and_delete(newC);
}

int SslAcceptorContext::show_ssl_ctx_sess_accept(THD *, SHOW_VAR *var,
                                                 char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_accept(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_accept_good(THD *, SHOW_VAR *var,
                                                      char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_accept_good(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_connect_good(THD *, SHOW_VAR *var,
                                                       char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_connect_good(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_accept_renegotiate(THD *,
                                                             SHOW_VAR *var,
                                                             char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_accept_renegotiate(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_connect_renegotiate(THD *,
                                                              SHOW_VAR *var,
                                                              char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_connect_renegotiate(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_cb_hits(THD *, SHOW_VAR *var,
                                                  char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_cb_hits(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_hits(THD *, SHOW_VAR *var,
                                               char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_hits(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_cache_full(THD *, SHOW_VAR *var,
                                                     char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_cache_full(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_misses(THD *, SHOW_VAR *var,
                                                 char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_misses(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_timeouts(THD *, SHOW_VAR *var,
                                                   char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_timeouts(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_number(THD *, SHOW_VAR *var,
                                                 char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_number(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_connect(THD *, SHOW_VAR *var,
                                                  char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_connect(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_sess_get_cache_size(THD *, SHOW_VAR *var,
                                                         char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_sess_get_cache_size(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_get_verify_mode(THD *, SHOW_VAR *var,
                                                     char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_get_verify_mode(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_get_verify_depth(THD *, SHOW_VAR *var,
                                                      char *buff) {
  AutoLock c;

  var->type = SHOW_LONG;
  var->value = buff;
  *((long *)buff) = (c.empty() ? 0 : SSL_CTX_get_verify_depth(c));

  return 0;
}

int SslAcceptorContext::show_ssl_ctx_get_session_cache_mode(THD *,
                                                            SHOW_VAR *var,
                                                            char *) {
  AutoLock c;

  var->type = SHOW_CHAR;
  if (c.empty())
    var->value = const_cast<char *>("NONE");
  else
    switch (SSL_CTX_get_session_cache_mode(c)) {
      case SSL_SESS_CACHE_OFF:
        var->value = const_cast<char *>("OFF");
        break;
      case SSL_SESS_CACHE_CLIENT:
        var->value = const_cast<char *>("CLIENT");
        break;
      case SSL_SESS_CACHE_SERVER:
        var->value = const_cast<char *>("SERVER");
        break;
      case SSL_SESS_CACHE_BOTH:
        var->value = const_cast<char *>("BOTH");
        break;
      case SSL_SESS_CACHE_NO_AUTO_CLEAR:
        var->value = const_cast<char *>("NO_AUTO_CLEAR");
        break;
      case SSL_SESS_CACHE_NO_INTERNAL_LOOKUP:
        var->value = const_cast<char *>("NO_INTERNAL_LOOKUP");
        break;
      default:
        var->value = const_cast<char *>("Unknown");
        break;
    }

  return 0;
}

static char *my_asn1_time_to_string(ASN1_TIME *time, char *buf, int len) {
  int n_read;
  char *res = nullptr;
  BIO *bio = BIO_new(BIO_s_mem());

  if (bio == nullptr) return nullptr;

  if (!ASN1_TIME_print(bio, time)) goto end;

  n_read = BIO_read(bio, buf, len - 1);

  if (n_read > 0) {
    buf[n_read] = 0;
    res = buf;
  }

end:
  BIO_free(bio);
  return res;
}

int SslAcceptorContext::show_ssl_get_server_not_before(THD *, SHOW_VAR *var,
                                                       char *buff) {
  AutoLock c;
  var->type = SHOW_CHAR;
  if (!c.empty()) {
    X509 *cert = SSL_get_certificate(c);
    ASN1_TIME *not_before = X509_get_notBefore(cert);

    if (not_before == nullptr) {
      var->value = empty_c_string;
      return 0;
    }

    var->value =
        my_asn1_time_to_string(not_before, buff, SHOW_VAR_FUNC_BUFF_SIZE);
    if (var->value == nullptr) {
      var->value = empty_c_string;
      return 1;
    }
  } else
    var->value = empty_c_string;

  return 0;
}

int SslAcceptorContext::show_ssl_get_server_not_after(THD *, SHOW_VAR *var,
                                                      char *buff) {
  AutoLock c;

  var->type = SHOW_CHAR;
  if (!c.empty()) {
    X509 *cert = SSL_get_certificate(c);
    ASN1_TIME *not_after = X509_get_notAfter(cert);

    if (not_after == nullptr) {
      var->value = empty_c_string;
      return 0;
    }

    var->value =
        my_asn1_time_to_string(not_after, buff, SHOW_VAR_FUNC_BUFF_SIZE);
    if (var->value == nullptr) {
      var->value = empty_c_string;
      return 1;
    }
  } else
    var->value = empty_c_string;

  return 0;
}

int SslAcceptorContext::show_ssl_get_ssl_ca(THD *, SHOW_VAR *var, char *buff) {
  AutoLock c;
  const char *s = c.get_current_ca();

  var->type = SHOW_CHAR;
  strncpy(buff, s ? s : "", SHOW_VAR_FUNC_BUFF_SIZE);
  buff[SHOW_VAR_FUNC_BUFF_SIZE - 1] = 0;
  var->value = buff;
  return 0;
}

int SslAcceptorContext::show_ssl_get_ssl_capath(THD *, SHOW_VAR *var,
                                                char *buff) {
  AutoLock c;
  const char *s = c.get_current_capath();

  var->type = SHOW_CHAR;
  strncpy(buff, s ? s : "", SHOW_VAR_FUNC_BUFF_SIZE);
  buff[SHOW_VAR_FUNC_BUFF_SIZE - 1] = 0;
  var->value = buff;
  return 0;
}

int SslAcceptorContext::show_ssl_get_ssl_cert(THD *, SHOW_VAR *var,
                                              char *buff) {
  AutoLock c;
  const char *s = c.get_current_cert();

  var->type = SHOW_CHAR;
  strncpy(buff, s ? s : "", SHOW_VAR_FUNC_BUFF_SIZE);
  buff[SHOW_VAR_FUNC_BUFF_SIZE - 1] = 0;
  var->value = buff;
  return 0;
}

int SslAcceptorContext::show_ssl_get_ssl_key(THD *, SHOW_VAR *var, char *buff) {
  AutoLock c;
  const char *s = c.get_current_key();
  var->type = SHOW_CHAR;
  strncpy(buff, s ? s : "", SHOW_VAR_FUNC_BUFF_SIZE);
  buff[SHOW_VAR_FUNC_BUFF_SIZE - 1] = 0;
  var->value = buff;
  return 0;
}

int SslAcceptorContext::show_ssl_get_ssl_cipher(THD *, SHOW_VAR *var,
                                                char *buff) {
  AutoLock c;
  const char *s = c.get_current_cipher();

  var->type = SHOW_CHAR;
  strncpy(buff, s ? s : "", SHOW_VAR_FUNC_BUFF_SIZE);
  buff[SHOW_VAR_FUNC_BUFF_SIZE - 1] = 0;
  var->value = buff;
  return 0;
}

int SslAcceptorContext::show_ssl_get_tls_ciphersuites(THD *, SHOW_VAR *var,
                                                      char *buff) {
  AutoLock c;
  const char *s = c.get_current_ciphersuites();

  var->type = SHOW_CHAR;
  strncpy(buff, s ? s : "", SHOW_VAR_FUNC_BUFF_SIZE);
  buff[SHOW_VAR_FUNC_BUFF_SIZE - 1] = 0;
  var->value = buff;
  return 0;
}

int SslAcceptorContext::show_ssl_get_tls_version(THD *, SHOW_VAR *var,
                                                 char *buff) {
  AutoLock c;
  const char *s = c.get_current_version();

  var->type = SHOW_CHAR;
  strncpy(buff, s ? s : "", SHOW_VAR_FUNC_BUFF_SIZE);
  buff[SHOW_VAR_FUNC_BUFF_SIZE - 1] = 0;
  var->value = buff;
  return 0;
}

int SslAcceptorContext::show_ssl_get_ssl_crl(THD *, SHOW_VAR *var, char *buff) {
  AutoLock c;
  const char *s = c.get_current_crl();

  var->type = SHOW_CHAR;
  strncpy(buff, s ? s : "", SHOW_VAR_FUNC_BUFF_SIZE);
  buff[SHOW_VAR_FUNC_BUFF_SIZE - 1] = 0;
  var->value = buff;
  return 0;
}

int SslAcceptorContext::show_ssl_get_ssl_crlpath(THD *, SHOW_VAR *var,
                                                 char *buff) {
  AutoLock c;
  const char *s = c.get_current_crlpath();

  var->type = SHOW_CHAR;
  strncpy(buff, s ? s : "", SHOW_VAR_FUNC_BUFF_SIZE);
  buff[SHOW_VAR_FUNC_BUFF_SIZE - 1] = 0;
  var->value = buff;
  return 0;
}

bool SslAcceptorContext::have_ssl() {
  AutoLock c;
  return !c.empty();
}

bool SslAcceptorContext::singleton_init(bool use_ssl_arg) {
  ssl_artifacts_status auto_detection_status;

  /*
    No need to take the ssl_ctx_lock lock here since it's being called
    from singleton_init().
  */
  if (use_ssl_arg) {
    auto_detection_status = SslAcceptorContext::auto_detect_ssl();
    if (auto_detection_status == SSL_ARTIFACTS_AUTO_DETECTED)
      LogErr(INFORMATION_LEVEL, ER_SSL_TRYING_DATADIR_DEFAULTS,
             DEFAULT_SSL_CA_CERT, DEFAULT_SSL_SERVER_CERT,
             DEFAULT_SSL_SERVER_KEY);
    if (do_auto_cert_generation(auto_detection_status, &opt_ssl_ca,
                                &opt_ssl_key, &opt_ssl_cert) == false)
      return true;
  }

  /*
    No real need for opt_use_ssl to be enabled in bootstrap mode,
    but we want the SSL material generation and/or validation (if supplied).
    So we keep it on.

    For openssl, we don't hush the option since it would indicate a failure
    in auto-generation, bad key material explicitly specified or
    auto-generation disabled explcitly while SSL is still on.
  */
  SslAcceptorContext *news = new SslAcceptorContext(use_ssl_arg);

  s_lock = new SslAcceptorContext::SslAcceptorContextLockType(news);
  if (!s_lock) {
    LogErr(WARNING_LEVEL, ER_SSL_LIBRARY_ERROR,
           "Error initializing the SSL context system structure");
    return true;
  }

  if (news->have_ssl() && warn_self_signed_ca()) return true;

  return false;
}

/**
  Verifies the server certificate for formal validity and against the
    CA certificates if specified.

  This verifies things like expiration dates, full certificate chains
  present etc.

  @param ctx The listening SSL context with all certificates installed
  @param ssl An SSL handle to extract the certificate from.
  @retval NULL No errors found
  @retval non-null The text of the error from the library
*/
static const char *verify_store_cert(SSL_CTX *ctx, SSL *ssl) {
  const char *result = nullptr;
  X509 *cert = SSL_get_certificate(ssl);
  X509_STORE_CTX *sctx = X509_STORE_CTX_new();

  if (nullptr != sctx &&
      0 != X509_STORE_CTX_init(sctx, SSL_CTX_get_cert_store(ctx), cert,
                               nullptr) &&
      !X509_verify_cert(sctx)) {
    result = X509_verify_cert_error_string(X509_STORE_CTX_get_error(sctx));
  }
  if (sctx != nullptr) X509_STORE_CTX_free(sctx);
  return result;
}

SslAcceptorContext::SslAcceptorContext(bool use_ssl_arg, bool report_ssl_error,
                                       enum enum_ssl_init_error *out_error)
    : ssl_acceptor_fd(nullptr), acceptor(nullptr) {
  enum enum_ssl_init_error error_num = SSL_INITERR_NOERROR;

  read_parameters(&current_ca_, &current_capath_, &current_version_,
                  &current_cert_, &current_cipher_, &current_ciphersuites_,
                  &current_key_, &current_crl_, &current_crlpath_);

  if (use_ssl_arg) {
    ssl_acceptor_fd = new_VioSSLAcceptorFd(
        current_key_.c_str(), current_cert_.c_str(), current_ca_.c_str(),
        current_capath_.c_str(), current_cipher_.c_str(),
        current_ciphersuites_.c_str(), &error_num, current_crl_.c_str(),
        current_crlpath_.c_str(),
        process_tls_version(current_version_.c_str()));

    if (!ssl_acceptor_fd && report_ssl_error)
      LogErr(WARNING_LEVEL, ER_SSL_LIBRARY_ERROR, sslGetErrString(error_num));

    if (ssl_acceptor_fd) acceptor = SSL_new(ssl_acceptor_fd->ssl_context);

    if (ssl_acceptor_fd && acceptor) {
      const char *error =
          verify_store_cert(ssl_acceptor_fd->ssl_context, acceptor);

      if (error && report_ssl_error)
        LogErr(WARNING_LEVEL, ER_SSL_SERVER_CERT_VERIFY_FAILED, error);
    }
  }
  if (out_error) *out_error = error_num;
}

SslAcceptorContext::~SslAcceptorContext() {
  if (acceptor) SSL_free(acceptor);
  if (ssl_acceptor_fd) free_vio_ssl_fd(ssl_acceptor_fd);
}

ssl_artifacts_status SslAcceptorContext::auto_detect_ssl() {
  MY_STAT cert_stat, cert_key, ca_stat;
  uint result = 1;
  ssl_artifacts_status ret_status = SSL_ARTIFACTS_VIA_OPTIONS;

  /*
    No need to take the ssl_ctx_lock lock here since it's being called
    from singleton_init().
  */
  if ((!opt_ssl_cert || !opt_ssl_cert[0]) &&
      (!opt_ssl_key || !opt_ssl_key[0]) && (!opt_ssl_ca || !opt_ssl_ca[0]) &&
      (!opt_ssl_capath || !opt_ssl_capath[0]) &&
      (!opt_ssl_crl || !opt_ssl_crl[0]) &&
      (!opt_ssl_crlpath || !opt_ssl_crlpath[0])) {
    result =
        result << (my_stat(DEFAULT_SSL_SERVER_CERT, &cert_stat, MYF(0)) ? 1 : 0)
               << (my_stat(DEFAULT_SSL_SERVER_KEY, &cert_key, MYF(0)) ? 1 : 0)
               << (my_stat(DEFAULT_SSL_CA_CERT, &ca_stat, MYF(0)) ? 1 : 0);

    switch (result) {
      case 8:
        opt_ssl_ca = DEFAULT_SSL_CA_CERT;
        opt_ssl_cert = DEFAULT_SSL_SERVER_CERT;
        opt_ssl_key = DEFAULT_SSL_SERVER_KEY;
        ret_status = SSL_ARTIFACTS_AUTO_DETECTED;
        break;
      case 4:
      case 2:
        ret_status = SSL_ARTIFACT_TRACES_FOUND;
        break;
      default:
        ret_status = SSL_ARTIFACTS_NOT_FOUND;
        break;
    };
  }

  return ret_status;
}

static int warn_one(const char *file_name) {
  char *issuer = nullptr;
  char *subject = nullptr;
  X509 *ca_cert;
  BIO *bio;
  FILE *fp;

  if (!(fp = my_fopen(file_name, O_RDONLY | MY_FOPEN_BINARY, MYF(MY_WME)))) {
    LogErr(ERROR_LEVEL, ER_CANT_OPEN_CA);
    return 1;
  }

  bio = BIO_new(BIO_s_file());
  if (!bio) {
    LogErr(ERROR_LEVEL, ER_FAILED_TO_ALLOCATE_SSL_BIO);
    my_fclose(fp, MYF(0));
    return 1;
  }
  BIO_set_fp(bio, fp, BIO_NOCLOSE);
  ca_cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
  BIO_free(bio);

  if (!ca_cert) {
    /* We are not interested in anything other than X509 certificates */
    my_fclose(fp, MYF(MY_WME));
    return 0;
  }

  issuer = X509_NAME_oneline(X509_get_issuer_name(ca_cert), nullptr, 0);
  subject = X509_NAME_oneline(X509_get_subject_name(ca_cert), nullptr, 0);

  /* Suppressing warning which is not relevant during initialization */
  if (!strcmp(issuer, subject) &&
      !(opt_initialize || opt_initialize_insecure)) {
    LogErr(WARNING_LEVEL, ER_CA_SELF_SIGNED, file_name);
  }

  OPENSSL_free(issuer);
  OPENSSL_free(subject);
  X509_free(ca_cert);
  my_fclose(fp, MYF(MY_WME));
  return 0;
}

int SslAcceptorContext::warn_self_signed_ca() {
  int ret_val = 0;
  /* */
  if (opt_ssl_ca && opt_ssl_ca[0]) {
    if (warn_one(opt_ssl_ca)) return 1;
  }
  if (opt_ssl_capath && opt_ssl_capath[0]) {
    /* We have ssl-capath. So search all files in the dir */
    MY_DIR *ca_dir;
    uint file_count;
    DYNAMIC_STRING file_path;
    char dir_separator[FN_REFLEN];
    size_t dir_path_length;

    init_dynamic_string(&file_path, opt_ssl_capath, FN_REFLEN, FN_REFLEN);
    dir_separator[0] = FN_LIBCHAR;
    dir_separator[1] = 0;
    dynstr_append(&file_path, dir_separator);
    dir_path_length = file_path.length;

    if (!(ca_dir =
              my_dir(opt_ssl_capath, MY_WANT_STAT | MY_DONT_SORT | MY_WME))) {
      LogErr(ERROR_LEVEL, ER_CANT_ACCESS_CAPATH);
      return 1;
    }

    for (file_count = 0; file_count < ca_dir->number_off_files; file_count++) {
      if (!MY_S_ISDIR(ca_dir->dir_entry[file_count].mystat->st_mode)) {
        file_path.length = dir_path_length;
        dynstr_append(&file_path, ca_dir->dir_entry[file_count].name);
        if ((ret_val = warn_one(file_path.str))) break;
      }
    }
    my_dirend(ca_dir);
    dynstr_free(&file_path);

    ca_dir = nullptr;
    memset(&file_path, 0, sizeof(file_path));
  }
  return ret_val;
}

void SslAcceptorContext::read_parameters(
    OptionalString *ca, OptionalString *capath, OptionalString *version,
    OptionalString *cert, OptionalString *cipher, OptionalString *ciphersuites,
    OptionalString *key, OptionalString *crl, OptionalString *crl_path) {
  AutoRLock lock(&lock_ssl_ctx);
  if (ca) ca->assign(opt_ssl_ca);
  if (capath) capath->assign(opt_ssl_capath);
  if (version) version->assign(opt_tls_version);
  if (cert) cert->assign(opt_ssl_cert);
  if (cipher) cipher->assign(opt_ssl_cipher);
  if (ciphersuites) ciphersuites->assign(opt_tls_ciphersuites);
  if (key) key->assign(opt_ssl_key);
  if (crl) crl->assign(opt_ssl_crl);
  if (crl_path) crl_path->assign(opt_ssl_crlpath);
}

/*
  If you are adding new system variable for SSL communication, please take a
  look at do_auto_cert_generation() function in sql_authentication.cc and
  add new system variable in checks if required.
*/

static Sys_var_charptr Sys_ssl_ca(
    "ssl_ca", "CA file in PEM format (check OpenSSL docs, implies --ssl)",
    PERSIST_AS_READONLY GLOBAL_VAR(opt_ssl_ca),
    CMD_LINE(REQUIRED_ARG, OPT_SSL_CA), IN_FS_CHARSET, DEFAULT(nullptr),
    &lock_ssl_ctx);

static Sys_var_charptr Sys_ssl_capath(
    "ssl_capath", "CA directory (check OpenSSL docs, implies --ssl)",
    PERSIST_AS_READONLY GLOBAL_VAR(opt_ssl_capath),
    CMD_LINE(REQUIRED_ARG, OPT_SSL_CAPATH), IN_FS_CHARSET, DEFAULT(nullptr),
    &lock_ssl_ctx);

static Sys_var_charptr Sys_tls_version(
    "tls_version",
    "TLS version, permitted values are TLSv1, TLSv1.1, TLSv1.2, TLSv1.3",
    PERSIST_AS_READONLY GLOBAL_VAR(opt_tls_version),
    CMD_LINE(REQUIRED_ARG, OPT_TLS_VERSION), IN_FS_CHARSET,
#ifdef HAVE_TLSv13
    "TLSv1,TLSv1.1,TLSv1.2,TLSv1.3",
#else
    "TLSv1,TLSv1.1,TLSv1.2",
#endif /* HAVE_TLSv13 */
    &lock_ssl_ctx);

static Sys_var_charptr Sys_ssl_cert(
    "ssl_cert", "X509 cert in PEM format (implies --ssl)",
    PERSIST_AS_READONLY GLOBAL_VAR(opt_ssl_cert),
    CMD_LINE(REQUIRED_ARG, OPT_SSL_CERT), IN_FS_CHARSET, DEFAULT(nullptr),
    &lock_ssl_ctx);

static Sys_var_charptr Sys_ssl_cipher(
    "ssl_cipher", "SSL cipher to use (implies --ssl)",
    PERSIST_AS_READONLY GLOBAL_VAR(opt_ssl_cipher),
    CMD_LINE(REQUIRED_ARG, OPT_SSL_CIPHER), IN_FS_CHARSET, DEFAULT(nullptr),
    &lock_ssl_ctx);

static Sys_var_charptr Sys_tls_ciphersuites(
    "tls_ciphersuites", "TLS v1.3 ciphersuite to use (implies --ssl)",
    PERSIST_AS_READONLY GLOBAL_VAR(opt_tls_ciphersuites),
    CMD_LINE(REQUIRED_ARG, OPT_TLS_CIPHERSUITES), IN_FS_CHARSET,
    DEFAULT(nullptr), &lock_ssl_ctx);

static Sys_var_charptr Sys_ssl_key("ssl_key",
                                   "X509 key in PEM format (implies --ssl)",
                                   PERSIST_AS_READONLY GLOBAL_VAR(opt_ssl_key),
                                   CMD_LINE(REQUIRED_ARG, OPT_SSL_KEY),
                                   IN_FS_CHARSET, DEFAULT(nullptr),
                                   &lock_ssl_ctx);

static Sys_var_charptr Sys_ssl_crl(
    "ssl_crl", "CRL file in PEM format (check OpenSSL docs, implies --ssl)",
    PERSIST_AS_READONLY GLOBAL_VAR(opt_ssl_crl),
    CMD_LINE(REQUIRED_ARG, OPT_SSL_CRL), IN_FS_CHARSET, DEFAULT(nullptr),
    &lock_ssl_ctx);

static Sys_var_charptr Sys_ssl_crlpath(
    "ssl_crlpath", "CRL directory (check OpenSSL docs, implies --ssl)",
    PERSIST_AS_READONLY GLOBAL_VAR(opt_ssl_crlpath),
    CMD_LINE(REQUIRED_ARG, OPT_SSL_CRLPATH), IN_FS_CHARSET, DEFAULT(nullptr),
    &lock_ssl_ctx);
