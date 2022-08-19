/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SSL_ACCEPTOR_CONTEXT_INCLUDED
#define SSL_ACCEPTOR_CONTEXT_INCLUDED

#include <my_rcu_lock.h>
#include <violite.h>
#include <string>
#include "sql/auth/auth_common.h"

class THD;
struct SHOW_VAR;

/** helper class to deal with optionally empty strings */
class OptionalString {
 public:
  OptionalString() : value_(), empty_(true) {}
  OptionalString(const char *s) : value_(s ? s : ""), empty_(!s) {}
  ~OptionalString() {}
  OptionalString(const OptionalString &) = default;

  const char *c_str() const { return empty_ ? nullptr : value_.c_str(); }
  OptionalString &assign(const char *s) {
    value_.assign(s ? s : "");
    empty_ = !s;
    return *this;
  }

 private:
  std::string value_;
  bool empty_;
};

/**
  Class to encapsulate the Server SSL acceptor context
*/
class SslAcceptorContext {
 public:
  /**
    Initialize the single instance of the acceptor

    @param use_ssl_arg Pass false if you don't want the actual
    SSL context created (as in when SSL is initially disabled)
    @retval true failure to init
    @retval false initialized ok
  */
  static bool singleton_init(bool use_ssl_arg);
  /** De-initialize the single instance of the acceptor */
  static void singleton_deinit();
  /**
    Re-initialize the single instance of the acceptor

    @param[out] error
    @param  force activate the SSL settings even if this will lead to
    disabling SSL
  */
  static void singleton_flush(enum enum_ssl_init_error *error, bool force);

  // info functions, to be called for the session vars

  static int show_ssl_ctx_sess_accept(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_sess_accept_good(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_sess_connect_good(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_sess_accept_renegotiate(THD *, SHOW_VAR *var,
                                                  char *buff);
  static int show_ssl_ctx_sess_connect_renegotiate(THD *, SHOW_VAR *var,
                                                   char *buff);
  static int show_ssl_ctx_sess_cb_hits(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_sess_hits(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_sess_cache_full(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_sess_misses(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_sess_timeouts(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_sess_number(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_sess_connect(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_sess_get_cache_size(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_get_verify_mode(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_get_verify_depth(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_ctx_get_session_cache_mode(THD *, SHOW_VAR *var,
                                                 char *buff);
  static int show_ssl_get_server_not_before(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_get_server_not_after(THD *, SHOW_VAR *var, char *buff);

  // info about the current set of ssl_ctx parameters
  static int show_ssl_get_ssl_ca(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_get_ssl_capath(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_get_ssl_cert(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_get_ssl_key(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_get_ssl_cipher(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_get_tls_ciphersuites(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_get_tls_version(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_get_ssl_crl(THD *, SHOW_VAR *var, char *buff);
  static int show_ssl_get_ssl_crlpath(THD *, SHOW_VAR *var, char *buff);

  /**
    Check if SSL was initialized

    retval true if the singleton holds a properly initialized SSL_CTX
  */
  static bool have_ssl();

  /**
    An RCU lock type for @ref SslAcceptorContext
  */
  typedef MyRcuLock<SslAcceptorContext> SslAcceptorContextLockType;

  /**
    The prefered way to *read* SSL parameters.
    This is a scope lock class. So initialize it and hold it for
    as long as you need to access the data.
    @note It's OK to release it and still use the data *ONLY*
    if you apply other means to protect your data (e.g. SSL context
    reference counts etc).

    @sa MyRcuLock, SslAcceptorContext
  */
  class AutoLock : SslAcceptorContextLockType::ReadLock {
   public:
    AutoLock()
        : SslAcceptorContextLockType::ReadLock(SslAcceptorContext::s_lock) {}
    ~AutoLock() {}

    /**
      Access to the SSL_CTX from the protected @ref SslAcceptorContext
    */
    operator SSL_CTX *() {
      const SslAcceptorContext *c = *this;
      return c->ssl_acceptor_fd->ssl_context;
    }
    /**
      Access to the SSL from the protected @ref SslAcceptorContext
    */
    operator SSL *() {
      const SslAcceptorContext *c = *this;
      return c->acceptor;
    }
    operator struct st_VioSSLFd *() {
      const SslAcceptorContext *c = *this;
      return c->ssl_acceptor_fd;
    }
    /**
      Check if the SSL context actually contains a valid SSL_CTX
      @retval true there's no valid SSL_CTX
      @retval false there's a valid SSL_CTX, SSL and st_VioSSLFd
    */
    bool empty() {
      const SslAcceptorContext *c = *this;
      return c->ssl_acceptor_fd == nullptr;
    }

    // functions to return the cached values for the parameters so that the
    // status vars work.

    const char *get_current_ca() {
      const SslAcceptorContext *c = *this;
      return c->current_ca_.c_str();
    }
    const char *get_current_capath() {
      const SslAcceptorContext *c = *this;
      return c->current_capath_.c_str();
    }
    const char *get_current_cert() {
      const SslAcceptorContext *c = *this;
      return c->current_cert_.c_str();
    }
    const char *get_current_key() {
      const SslAcceptorContext *c = *this;
      return c->current_key_.c_str();
    }
    const char *get_current_version() {
      const SslAcceptorContext *c = *this;
      return c->current_version_.c_str();
    }
    const char *get_current_crl() {
      const SslAcceptorContext *c = *this;
      return c->current_crl_.c_str();
    }
    const char *get_current_crlpath() {
      const SslAcceptorContext *c = *this;
      return c->current_crlpath_.c_str();
    }
    const char *get_current_cipher() {
      const SslAcceptorContext *c = *this;
      return c->current_cipher_.c_str();
    }
    const char *get_current_ciphersuites() {
      const SslAcceptorContext *c = *this;
      return c->current_ciphersuites_.c_str();
    }
  };

  ~SslAcceptorContext();

  /**
    A workaround for consumers that need to read the values

    This is a temporary workaround for the subsystems that
    are trying to access the mysql protocol TLS context parameters.
    TODO: to be removed once these migrate to access the system variables.

    To use pass a non-null pointer to an std::string to any of the args
    to receive a copy of the relevant value that you will then need to
    dispose of.

    @param [out] ca
    @param [out] capath
    @param [out] version
    @param [out] cert
    @param [out] cipher
    @param [out] ciphersuites
    @param [out] key
    @param [out] crl
    @param [out] crl_path
  */
  static void read_parameters(
      OptionalString *ca = nullptr, OptionalString *capath = nullptr,
      OptionalString *version = nullptr, OptionalString *cert = nullptr,
      OptionalString *cipher = nullptr, OptionalString *ciphersuites = nullptr,
      OptionalString *key = nullptr, OptionalString *crl = nullptr,
      OptionalString *crl_path = nullptr);

 protected:
  /**
    A protected constructor since it's only instantiated via the static
    functions.

    @param use_ssl_arg don't bother at all to try and construct an SSL_CTX and
    just make an empty SslAcceptorContext. Used to pass the --ssl option at
    startup.
    @param report_ssl_error  report any SSL errors resulting from trying to
    initialize the SSL_CTX to the server's error log.
    @param [out] out_error an optional slot to return the SSL_CTX initialization
    error location.
  */
  SslAcceptorContext(bool use_ssl_arg, bool report_ssl_error = true,
                     enum enum_ssl_init_error *out_error = nullptr);
  /** Disable the copy constructor */
  SslAcceptorContext(const SslAcceptorContext &) = delete;
  /** Disable assignment operator */
  SslAcceptorContext operator=(const SslAcceptorContext) = delete;

  /** Try to auto-detect the SSL key material files. Called by @ref
   * singleton_init */
  static ssl_artifacts_status auto_detect_ssl();

  /** Put up a warning in the error log if the CA used is self-signed. Called by
   * @ref singleton_init */
  static int warn_self_signed_ca();

 protected:
  /** SSL_CTX barerer */
  struct st_VioSSLFd *ssl_acceptor_fd;
  /**
    An SSL for @ref ssl_acceptor_fd to allow access to parameters not in
    SSL_CTX to be available even if the current connection is not encrypted.
  */
  SSL *acceptor;

  /**
    Copies of the current effective values for quick return via the status vars
  */
  OptionalString current_ca_, current_capath_, current_version_, current_cert_,
      current_cipher_, current_ciphersuites_, current_key_, current_crl_,
      current_crlpath_;

  /** singleton lock */
  static SslAcceptorContextLockType *s_lock;
};

#endif  // SSL_ACCEPTOR_CONTEXT_INCLUDED
