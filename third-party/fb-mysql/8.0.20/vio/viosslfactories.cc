/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <string>

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#if !defined(HAVE_PSI_INTERFACE)
#include "mysql/psi/mysql_rwlock.h"
#endif
#include "mysql/service_mysql_alloc.h"
#include "mysys_err.h"
#include "vio/vio_priv.h"

#include <openssl/dh.h>

#if OPENSSL_VERSION_NUMBER < 0x10002000L
#include <openssl/ec.h>
#endif /* OPENSSL_VERSION_NUMBER < 0x10002000L */

#define TLS_VERSION_OPTION_SIZE 256

using std::string;

/*
  1. Cipher preference order: P1 > A1 > A2 > D1
  2. Blocked ciphers are not allowed
*/

static const string mandatory_p1 = {
    "ECDHE-ECDSA-AES128-GCM-SHA256:"
    "ECDHE-ECDSA-AES256-GCM-SHA384:"
    "ECDHE-RSA-AES128-GCM-SHA256:"
    "ECDHE-ECDSA-AES128-SHA256:"
    "ECDHE-RSA-AES128-SHA256"};

static const string optional_a1 = {
    "ECDHE-RSA-AES256-GCM-SHA384:"
    "ECDHE-ECDSA-AES256-SHA384:"
    "ECDHE-RSA-AES256-SHA384:"
    "DHE-RSA-AES128-GCM-SHA256:"
    "DHE-DSS-AES128-GCM-SHA256:"
    "DHE-RSA-AES128-SHA256:"
    "DHE-DSS-AES128-SHA256:"
    "DHE-DSS-AES256-GCM-SHA384:"
    "DHE-RSA-AES256-SHA256:"
    "DHE-DSS-AES256-SHA256:"
    "DHE-RSA-AES256-GCM-SHA384"};

static const std::string optional_a2 = {
    "DH-DSS-AES128-GCM-SHA256:"
    "ECDH-ECDSA-AES128-GCM-SHA256:"
    "DH-DSS-AES256-GCM-SHA384:"
    "ECDH-ECDSA-AES256-GCM-SHA384:"
    "DH-DSS-AES128-SHA256:"
    "ECDH-ECDSA-AES128-SHA256:"
    "DH-DSS-AES256-SHA256:"
    "ECDH-ECDSA-AES256-SHA384:"
    "DH-RSA-AES128-GCM-SHA256:"
    "ECDH-RSA-AES128-GCM-SHA256:"
    "DH-RSA-AES256-GCM-SHA384:"
    "ECDH-RSA-AES256-GCM-SHA384:"
    "DH-RSA-AES128-SHA256:"
    "ECDH-RSA-AES128-SHA256:"
    "DH-RSA-AES256-SHA256:"
    "ECDH-RSA-AES256-SHA384"};

static const std::string optional_d1 = {
    "ECDHE-RSA-AES128-SHA:"
    "ECDHE-ECDSA-AES128-SHA:"
    "ECDHE-RSA-AES256-SHA:"
    "ECDHE-ECDSA-AES256-SHA:"
    "DHE-DSS-AES128-SHA:"
    "DHE-RSA-AES128-SHA:"
    "DHE-DSS-AES256-SHA:"
    "DHE-RSA-AES256-SHA:"
    "DH-DSS-AES128-SHA:"
    "ECDH-ECDSA-AES128-SHA:"
    "AES256-SHA:"
    "DH-DSS-AES256-SHA:"
    "ECDH-ECDSA-AES256-SHA:"
    "DH-RSA-AES128-SHA:"
    "ECDH-RSA-AES128-SHA:"
    "DH-RSA-AES256-SHA:"
    "ECDH-RSA-AES256-SHA:"
    "CAMELLIA256-SHA:"
    "CAMELLIA128-SHA:"
    "AES128-GCM-SHA256:"
    "AES256-GCM-SHA384:"
    "AES128-SHA256:"
    "AES256-SHA256:"
    "AES128-SHA"};

static const string tls_cipher_blocked = {
    "!aNULL:"
    "!eNULL:"
    "!EXPORT:"
    "!LOW:"
    "!MD5:"
    "!DES:"
    "!RC2:"
    "!RC4:"
    "!PSK:"
    "!DES-CBC3-SHA:"
    "!DHE-DSS-DES-CBC3-SHA:"
    "!DHE-RSA-DES-CBC3-SHA:"
    "!ECDH-RSA-DES-CBC3-SHA:"
    "!ECDH-ECDSA-DES-CBC3-SHA:"
    "!ECDHE-RSA-DES-CBC3-SHA:"
    "!ECDHE-ECDSA-DES-CBC3-SHA:"
    "!DH-RSA-DES-CBC3-SHA:"
    "!DH-DSS-DES-CBC3-SHA"};

static bool ssl_initialized = false;

/*
  Diffie-Hellman key.
  Generated using: >openssl dhparam -5 -C 2048

  -----BEGIN DH PARAMETERS-----
  MIIBCAKCAQEAil36wGZ2TmH6ysA3V1xtP4MKofXx5n88xq/aiybmGnReZMviCPEJ
  46+7VCktl/RZ5iaDH1XNG1dVQmznt9pu2G3usU+k1/VB4bQL4ZgW4u0Wzxh9PyXD
  glm99I9Xyj4Z5PVE4MyAsxCRGA1kWQpD9/zKAegUBPLNqSo886Uqg9hmn8ksyU9E
  BV5eAEciCuawh6V0O+Sj/C3cSfLhgA0GcXp3OqlmcDu6jS5gWjn3LdP1U0duVxMB
  h/neTSCSvtce4CAMYMjKNVh9P1nu+2d9ZH2Od2xhRIqMTfAS1KTqF3VmSWzPFCjG
  mjxx/bg6bOOjpgZapvB6ABWlWmRmAAWFtwIBBQ==
  -----END DH PARAMETERS-----
 */
static unsigned char dh2048_p[] = {
    0x8A, 0x5D, 0xFA, 0xC0, 0x66, 0x76, 0x4E, 0x61, 0xFA, 0xCA, 0xC0, 0x37,
    0x57, 0x5C, 0x6D, 0x3F, 0x83, 0x0A, 0xA1, 0xF5, 0xF1, 0xE6, 0x7F, 0x3C,
    0xC6, 0xAF, 0xDA, 0x8B, 0x26, 0xE6, 0x1A, 0x74, 0x5E, 0x64, 0xCB, 0xE2,
    0x08, 0xF1, 0x09, 0xE3, 0xAF, 0xBB, 0x54, 0x29, 0x2D, 0x97, 0xF4, 0x59,
    0xE6, 0x26, 0x83, 0x1F, 0x55, 0xCD, 0x1B, 0x57, 0x55, 0x42, 0x6C, 0xE7,
    0xB7, 0xDA, 0x6E, 0xD8, 0x6D, 0xEE, 0xB1, 0x4F, 0xA4, 0xD7, 0xF5, 0x41,
    0xE1, 0xB4, 0x0B, 0xE1, 0x98, 0x16, 0xE2, 0xED, 0x16, 0xCF, 0x18, 0x7D,
    0x3F, 0x25, 0xC3, 0x82, 0x59, 0xBD, 0xF4, 0x8F, 0x57, 0xCA, 0x3E, 0x19,
    0xE4, 0xF5, 0x44, 0xE0, 0xCC, 0x80, 0xB3, 0x10, 0x91, 0x18, 0x0D, 0x64,
    0x59, 0x0A, 0x43, 0xF7, 0xFC, 0xCA, 0x01, 0xE8, 0x14, 0x04, 0xF2, 0xCD,
    0xA9, 0x2A, 0x3C, 0xF3, 0xA5, 0x2A, 0x83, 0xD8, 0x66, 0x9F, 0xC9, 0x2C,
    0xC9, 0x4F, 0x44, 0x05, 0x5E, 0x5E, 0x00, 0x47, 0x22, 0x0A, 0xE6, 0xB0,
    0x87, 0xA5, 0x74, 0x3B, 0xE4, 0xA3, 0xFC, 0x2D, 0xDC, 0x49, 0xF2, 0xE1,
    0x80, 0x0D, 0x06, 0x71, 0x7A, 0x77, 0x3A, 0xA9, 0x66, 0x70, 0x3B, 0xBA,
    0x8D, 0x2E, 0x60, 0x5A, 0x39, 0xF7, 0x2D, 0xD3, 0xF5, 0x53, 0x47, 0x6E,
    0x57, 0x13, 0x01, 0x87, 0xF9, 0xDE, 0x4D, 0x20, 0x92, 0xBE, 0xD7, 0x1E,
    0xE0, 0x20, 0x0C, 0x60, 0xC8, 0xCA, 0x35, 0x58, 0x7D, 0x3F, 0x59, 0xEE,
    0xFB, 0x67, 0x7D, 0x64, 0x7D, 0x8E, 0x77, 0x6C, 0x61, 0x44, 0x8A, 0x8C,
    0x4D, 0xF0, 0x12, 0xD4, 0xA4, 0xEA, 0x17, 0x75, 0x66, 0x49, 0x6C, 0xCF,
    0x14, 0x28, 0xC6, 0x9A, 0x3C, 0x71, 0xFD, 0xB8, 0x3A, 0x6C, 0xE3, 0xA3,
    0xA6, 0x06, 0x5A, 0xA6, 0xF0, 0x7A, 0x00, 0x15, 0xA5, 0x5A, 0x64, 0x66,
    0x00, 0x05, 0x85, 0xB7,
};

static unsigned char dh2048_g[] = {
    0x05,
};

static DH *get_dh2048(void) {
  DH *dh;
  if ((dh = DH_new())) {
    BIGNUM *p = BN_bin2bn(dh2048_p, sizeof(dh2048_p), nullptr);
    BIGNUM *g = BN_bin2bn(dh2048_g, sizeof(dh2048_g), nullptr);
    if (!p || !g
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
        || !DH_set0_pqg(dh, p, nullptr, g)
#endif /* OPENSSL_VERSION_NUMBER >= 0x10100000L */
    ) {
      /* DH_free() will free 'p' and 'g' at once. */
      DH_free(dh);
      return nullptr;
    }
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    dh->p = p;
    dh->g = g;
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  }
  return (dh);
}

static void report_errors() {
  unsigned long l;
  const char *file;
  const char *data;
  int line, flags;

  DBUG_TRACE;

  while ((l = ERR_get_error_line_data(&file, &line, &data, &flags)) > 0) {
#ifndef DBUG_OFF /* Avoid warning */
    char buf[200];
    DBUG_PRINT("error", ("OpenSSL: %s:%s:%d:%s\n", ERR_error_string(l, buf),
                         file, line, (flags & ERR_TXT_STRING) ? data : ""));
#endif
  }
}

static const char *ssl_error_string[] = {
    "No error",
    "Unable to get certificate",
    "Unable to get private key",
    "Private key does not match the certificate public key",
    "SSL_CTX_set_default_verify_paths failed",
    "Failed to set ciphers to use",
    "SSL_CTX_new failed",
    "SSL context is not usable without certificate and private key",
    "SSL_CTX_set_tmp_dh failed",
    "TLS version is invalid",
    "Failed to set ecdh information"};

const char *sslGetErrString(enum enum_ssl_init_error e) {
  DBUG_ASSERT(SSL_INITERR_NOERROR < e && e < SSL_INITERR_LASTERR);
  return ssl_error_string[e];
}

static bool check_same_file(const char *cert_file, const char *key_file) {
  MY_STAT cert_stat;
  MY_STAT key_stat;

  if (!my_stat(cert_file, &cert_stat, MYF(0)) ||
      !my_stat(key_file, &key_stat, MYF(0))) {
    DBUG_PRINT("error", ("Unable to stat the files."));
    return false;
  }

  return cert_stat.st_ino == key_stat.st_ino &&
         cert_stat.st_dev == key_stat.st_dev;
}

static int vio_load_cert(SSL_CTX *ctx, const char *cert_file, BIO *bio) {
  X509 *cert = NULL;

  if (BIO_read_filename(bio, const_cast<char *>(cert_file)) != 1 ||
      !PEM_read_bio_X509(bio, &cert, NULL, NULL)) {
    return 1;
  }

  int ret = SSL_CTX_use_certificate(ctx, cert);
  X509_free(cert);

  return ret <= 0;
}

static int vio_load_key(SSL_CTX *ctx, const char *key_file, bool same_file,
                        BIO *bio) {
  // read key file if not in same file as cert
  if (!same_file) {
    if (BIO_read_filename(bio, const_cast<char *>(key_file)) != 1) {
      DBUG_PRINT("error", ("BIO_WRITE for key %s failed", key_file));
      return 1;
    }
  } else {
    if (BIO_reset(bio) < 0) {
      DBUG_PRINT("error", ("BIO_reset failed ret"));
      return 1;
    }
  }

  EVP_PKEY *key = NULL;
  if (!PEM_read_bio_PrivateKey(bio, &key, NULL, NULL)) return 1;

  int ret = SSL_CTX_use_PrivateKey(ctx, key);
  EVP_PKEY_free(key);

  return ret <= 0;
}

static int vio_set_cert_stuff(SSL_CTX *ctx, const char *cert_file,
                              const char *key_file,
                              enum enum_ssl_init_error *error) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("ctx: %p  cert_file: %s  key_file: %s", ctx, cert_file,
                       key_file));

  *error = SSL_INITERR_NOERROR;
  bool same_file = false;

  if (cert_file == NULL) {
    if (key_file == NULL) return 0;

    cert_file = key_file;
    same_file = true;
  } else if (key_file == NULL) {
    key_file = cert_file;
    same_file = true;
  } else
    same_file = check_same_file(key_file, cert_file);

  const char *error_file = NULL;
  BIO *bio = BIO_new(BIO_s_file());
  if (bio == NULL)
    *error = SSL_INITERR_MEMFAIL;
  else {
    // load the cert
    if (vio_load_cert(ctx, cert_file, bio)) {
      *error = SSL_INITERR_CERT;
      error_file = cert_file;
    }
    // load the key
    else if (vio_load_key(ctx, key_file, same_file, bio)) {
      *error = SSL_INITERR_KEY;
      error_file = key_file;
    }

    /*
      If we are using DSA, we can copy the parameters from the private key
      Now we know that a key and cert have been set against the SSL context
    */
    else if (!SSL_CTX_check_private_key(ctx)) {
      *error = SSL_INITERR_NOMATCH;
    }

    BIO_free_all(bio);
  }

  if (*error != SSL_INITERR_NOERROR) {
    char err_string[MYSQL_ERRMSG_SIZE];
    if (error_file)
      snprintf(err_string, sizeof(err_string), "%s from '%s'",
               sslGetErrString(*error), error_file);
    else
      snprintf(err_string, sizeof(err_string), "%s", sslGetErrString(*error));
    DBUG_PRINT("error", ("%s", err_string));
    DBUG_EXECUTE("error", ERR_print_errors_fp(DBUG_FILE););
    my_message_local(ERROR_LEVEL, EE_SSL_ERROR_FROM_FILE,
                     sslGetErrString(*error), cert_file);
    return 1;
  }

  return 0;
}

/*
  OpenSSL 1.1 supports native platform threads,
  so we don't need the following callback functions.
*/
#if OPENSSL_VERSION_NUMBER < 0x10100000L
/* OpenSSL specific */

#ifdef HAVE_PSI_INTERFACE
static PSI_rwlock_key key_rwlock_openssl;

static PSI_rwlock_info openssl_rwlocks[] = {
    {&key_rwlock_openssl, "CRYPTO_dynlock_value::lock", 0, 0, nullptr}};
#endif

typedef struct CRYPTO_dynlock_value {
  mysql_rwlock_t lock;
} openssl_lock_t;

/* Array of locks used by openssl internally for thread synchronization.
   The number of locks is equal to CRYPTO_num_locks.
*/
static openssl_lock_t *openssl_stdlocks;

/*OpenSSL callback functions for multithreading. We implement all the functions
  as we are using our own locking mechanism.
*/
static void openssl_lock(int mode, openssl_lock_t *lock,
                         const char *file MY_ATTRIBUTE((unused)),
                         int line MY_ATTRIBUTE((unused))) {
  int err;
  char const *what;

  switch (mode) {
    case CRYPTO_LOCK | CRYPTO_READ:
      what = "read lock";
      err = mysql_rwlock_rdlock(&lock->lock);
      break;
    case CRYPTO_LOCK | CRYPTO_WRITE:
      what = "write lock";
      err = mysql_rwlock_wrlock(&lock->lock);
      break;
    case CRYPTO_UNLOCK | CRYPTO_READ:
    case CRYPTO_UNLOCK | CRYPTO_WRITE:
      what = "unlock";
      err = mysql_rwlock_unlock(&lock->lock);
      break;
    default:
      /* Unknown locking mode. */
      DBUG_PRINT("error",
                 ("Fatal OpenSSL: %s:%d: interface problem (mode=0x%x)\n", file,
                  line, mode));

      fprintf(stderr, "Fatal: OpenSSL interface problem (mode=0x%x)", mode);
      fflush(stderr);
      abort();
  }
  if (err) {
    DBUG_PRINT("error", ("Fatal OpenSSL: %s:%d: can't %s OpenSSL lock\n", file,
                         line, what));

    fprintf(stderr, "Fatal: can't %s OpenSSL lock", what);
    fflush(stderr);
    abort();
  }
}

static void openssl_lock_function(int mode, int n,
                                  const char *file MY_ATTRIBUTE((unused)),
                                  int line MY_ATTRIBUTE((unused))) {
  if (n < 0 || n > CRYPTO_num_locks()) {
    /* Lock number out of bounds. */
    DBUG_PRINT("error", ("Fatal OpenSSL: %s:%d: interface problem (n = %d)",
                         file, line, n));

    fprintf(stderr, "Fatal: OpenSSL interface problem (n = %d)", n);
    fflush(stderr);
    abort();
  }
  openssl_lock(mode, &openssl_stdlocks[n], file, line);
}

static openssl_lock_t *openssl_dynlock_create(
    const char *file MY_ATTRIBUTE((unused)), int line MY_ATTRIBUTE((unused))) {
  openssl_lock_t *lock;

  DBUG_PRINT("info", ("openssl_dynlock_create: %s:%d", file, line));

  lock = (openssl_lock_t *)my_malloc(PSI_NOT_INSTRUMENTED,
                                     sizeof(openssl_lock_t), MYF(0));

#ifdef HAVE_PSI_INTERFACE
  mysql_rwlock_init(key_rwlock_openssl, &lock->lock);
#else
  mysql_rwlock_init(0, &lock->lock);
#endif
  return lock;
}

static void openssl_dynlock_destroy(openssl_lock_t *lock,
                                    const char *file MY_ATTRIBUTE((unused)),
                                    int line MY_ATTRIBUTE((unused))) {
  DBUG_PRINT("info", ("openssl_dynlock_destroy: %s:%d", file, line));

  mysql_rwlock_destroy(&lock->lock);
  my_free(lock);
}

static unsigned long openssl_id_function() {
  return (unsigned long)my_thread_self();
}

// End of mutlithreading callback functions

static void init_ssl_locks() {
  int i = 0;
#ifdef HAVE_PSI_INTERFACE
  const char *category = "sql";
  int count = static_cast<int>(array_elements(openssl_rwlocks));
  mysql_rwlock_register(category, openssl_rwlocks, count);
#endif

  openssl_stdlocks = (openssl_lock_t *)OPENSSL_malloc(CRYPTO_num_locks() *
                                                      sizeof(openssl_lock_t));
  for (i = 0; i < CRYPTO_num_locks(); ++i)
#ifdef HAVE_PSI_INTERFACE
    mysql_rwlock_init(key_rwlock_openssl, &openssl_stdlocks[i].lock);
#else
    mysql_rwlock_init(0, &openssl_stdlocks[i].lock);
#endif
}

static void set_lock_callback_functions(bool init) {
  CRYPTO_set_locking_callback(init ? openssl_lock_function : NULL);
  CRYPTO_set_id_callback(init ? openssl_id_function : NULL);
  CRYPTO_set_dynlock_create_callback(init ? openssl_dynlock_create : NULL);
  CRYPTO_set_dynlock_destroy_callback(init ? openssl_dynlock_destroy : NULL);
  CRYPTO_set_dynlock_lock_callback(init ? openssl_lock : NULL);
}

static void init_lock_callback_functions() {
  set_lock_callback_functions(true);
}

static void deinit_lock_callback_functions() {
  set_lock_callback_functions(false);
}

#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */

void vio_ssl_end() {
  if (ssl_initialized) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    ERR_remove_thread_state(0);
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
    ERR_free_strings();
    EVP_cleanup();

    CRYPTO_cleanup_all_ex_data();

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    deinit_lock_callback_functions();

    for (int i = 0; i < CRYPTO_num_locks(); ++i)
      mysql_rwlock_destroy(&openssl_stdlocks[i].lock);
    OPENSSL_free(openssl_stdlocks);

#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
    ssl_initialized = false;
  }
}

void ssl_start() {
  if (!ssl_initialized) {
    ssl_initialized = true;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    // if locks already initialized, don't override
    if (CRYPTO_get_locking_callback()) {
      DBUG_PRINT("info", ("Openssl crypto library already initialized."));
      return;
    }
    init_ssl_locks();
    init_lock_callback_functions();
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  }
}

#if !defined(OPENSSL_IS_BORINGSSL)
/**
  Set fips mode in openssl library,
  When we set fips mode ON/STRICT, it will perform following operations:
  1. Check integrity of openssl library
  2. Run fips related tests.
  3. Disable non fips complaint algorithms
  4. Should be set par process before openssl library initialization
  5. When FIPs mode ON(1/2), calling weak algorithms  may results into process
  abort.

  @param [in]  fips_mode     0 for fips mode off, 1/2 for fips mode ON
  @param [out] err_string    If fips mode set fails, err_string will have detail
  failure reason.

  @returns openssl set fips mode errors
    @retval non 1 for Error
    @retval 1 Success
*/
int set_fips_mode(const uint fips_mode, char err_string[OPENSSL_ERROR_LENGTH]) {
  int rc = -1;
  unsigned int fips_mode_old = -1;
  unsigned long err_library = 0;
  if (fips_mode > 2) {
    goto EXIT;
  }
  fips_mode_old = FIPS_mode();
  if (fips_mode_old == fips_mode) {
    rc = 1;
    goto EXIT;
  }
  if (!(rc = FIPS_mode_set(fips_mode))) {
    /*
      If OS doesn't have FIPS enabled openssl library and user sets FIPS mode
      ON, It fails with proper error. But in the same time it doesn't allow to
      perform any cryptographic operation. Now if FIPS mode set fails with
      error, setting old working FIPS mode value in the OpenSSL library. It will
      allow successful cryptographic operation and will not abort the server.
    */
    FIPS_mode_set(fips_mode_old);
    err_library = ERR_get_error();
    ERR_error_string_n(err_library, err_string, OPENSSL_ERROR_LENGTH - 1);
    err_string[OPENSSL_ERROR_LENGTH - 1] = '\0';
  }
EXIT:
  return rc;
}

/**
  Get fips mode from openssl library,

  @returns openssl current fips mode
*/
uint get_fips_mode() { return FIPS_mode(); }
#endif

long process_tls_version(const char *tls_version) {
  const char *separator = ",";
  char *token, *lasts = nullptr;

#ifdef HAVE_TLSv13
  const char *tls_version_name_list[] = {"TLSv1", "TLSv1.1", "TLSv1.2",
                                         "TLSv1.3"};
  const char ctx_flag_default[] = "TLSv1,TLSv1.1,TLSv1.2,TLSv1.3";
  const long tls_ctx_list[] = {SSL_OP_NO_TLSv1, SSL_OP_NO_TLSv1_1,
                               SSL_OP_NO_TLSv1_2, SSL_OP_NO_TLSv1_3};
  long tls_ctx_flag = SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_2 |
                      SSL_OP_NO_TLSv1_3;
#else
  const char *tls_version_name_list[] = {"TLSv1", "TLSv1.1", "TLSv1.2"};
  const char ctx_flag_default[] = "TLSv1,TLSv1.1,TLSv1.2";
  const long tls_ctx_list[] = {SSL_OP_NO_TLSv1, SSL_OP_NO_TLSv1_1,
                               SSL_OP_NO_TLSv1_2};
  long tls_ctx_flag = SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_2;
#endif /* HAVE_TLSv13 */
  const unsigned int tls_versions_count = array_elements(tls_version_name_list);
  char tls_version_option[TLS_VERSION_OPTION_SIZE] = "";
  int tls_found = 0;

  if (!tls_version ||
      !my_strcasecmp(&my_charset_latin1, tls_version, ctx_flag_default))
    return 0;

  if (strlen(tls_version) - 1 > sizeof(tls_version_option)) return -1;

  strncpy(tls_version_option, tls_version, sizeof(tls_version_option));
  token = my_strtok_r(tls_version_option, separator, &lasts);
  while (token) {
    for (unsigned int i = 0; i < tls_versions_count; i++) {
      if (!my_strcasecmp(&my_charset_latin1, tls_version_name_list[i], token)) {
        tls_found = 1;
        tls_ctx_flag &= ~tls_ctx_list[i];
        break;
      }
    }
    token = my_strtok_r(nullptr, separator, &lasts);
  }

  if (!tls_found)
    return -1;
  else
    return tls_ctx_flag;
}

/************************ VioSSLFd **********************************/
static struct st_VioSSLFd *new_VioSSLFd(
    const char *key_file, const char *cert_file, const char *ca_file,
    const char *ca_path, const char *cipher,
    const char *ciphersuites MY_ATTRIBUTE((unused)), bool is_client,
    enum enum_ssl_init_error *error, const char *crl_file, const char *crl_path,
    const long ssl_ctx_flags) {
  DH *dh;
  struct st_VioSSLFd *ssl_fd;
  long ssl_ctx_options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
  int ret_set_cipherlist = 0;
  string cipher_list;
#if OPENSSL_VERSION_NUMBER < 0x10002000L
  EC_KEY *eckey = nullptr;
#endif /* OPENSSL_VERSION_NUMBER < 0x10002000L */
  DBUG_TRACE;
  DBUG_PRINT(
      "enter",
      ("key_file: '%s'  cert_file: '%s'  ca_file: '%s'  ca_path: '%s'  "
       "cipher: '%s' crl_file: '%s' crl_path: '%s' ssl_ctx_flags: '%ld' ",
       key_file ? key_file : "NULL", cert_file ? cert_file : "NULL",
       ca_file ? ca_file : "NULL", ca_path ? ca_path : "NULL",
       cipher ? cipher : "NULL", crl_file ? crl_file : "NULL",
       crl_path ? crl_path : "NULL", ssl_ctx_flags));

  if (ssl_ctx_flags < 0) {
    *error = SSL_TLS_VERSION_INVALID;
    DBUG_PRINT("error", ("TLS version invalid : %s", sslGetErrString(*error)));
    report_errors();
    return nullptr;
  }

  ssl_ctx_options = (ssl_ctx_options | ssl_ctx_flags) &
                    (SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 |
                     SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_2
#ifdef HAVE_TLSv13
                     | SSL_OP_NO_TLSv1_3
#endif /* HAVE_TLSv13 */
                     | SSL_OP_NO_TICKET);
  if (!(ssl_fd = ((struct st_VioSSLFd *)my_malloc(
            key_memory_vio_ssl_fd, sizeof(struct st_VioSSLFd), MYF(0)))))
    return nullptr;
  ssl_fd->owned = true;

  if (!(ssl_fd->ssl_context = SSL_CTX_new(is_client ?
#ifdef HAVE_TLSv13
                                                    TLS_client_method()
                                                    : TLS_server_method()
#else  /* HAVE_TLSv13 */
                                                    SSLv23_client_method()
                                                    : SSLv23_server_method()
#endif /* HAVE_TLSv13 */
                                              ))) {
    *error = SSL_INITERR_MEMFAIL;
    DBUG_PRINT("error", ("%s", sslGetErrString(*error)));
    report_errors();
    my_free(ssl_fd);
    return nullptr;
  }

#if defined(HAVE_TLSv13)
  /*
    Set OpenSSL TLS v1.3 ciphersuites.
    Note that an empty list is permissible.
  */
  if (nullptr != ciphersuites) {
    /*
      Note: if TLSv1.3 is enabled but TLSv1.3 ciphersuite list is empty
      (that's permissible and mentioned in the documentation),
      the connection will fail with "no ciphers available" error.
    */
    if (0 == SSL_CTX_set_ciphersuites(ssl_fd->ssl_context, ciphersuites)) {
      *error = SSL_INITERR_CIPHERS;
      goto error;
    }
  }
#endif /* HAVE_TLSv13 */

  /*
    We explicitly prohibit weak ciphers.
    NOTE: SSL_CTX_set_cipher_list will return 0 if
    none of the provided ciphers could be selected
  */
  cipher_list += tls_cipher_blocked + ":";

  /*
    If ciphers are specified explicitly by caller, use them.
    Otherwise, fallback to the default list.

    In either case, we make sure we stay within the valid bounds.
    Note that we have already consumed tls_cipher_blocked
    worth of space.
  */
  if (cipher == nullptr) {
    cipher_list += mandatory_p1 + ":" + optional_a1 + ":" + optional_a2 + ":" +
                   optional_d1;
  } else
    cipher_list.append(cipher);

  if (ret_set_cipherlist ==
      SSL_CTX_set_cipher_list(ssl_fd->ssl_context, cipher_list.c_str())) {
    *error = SSL_INITERR_CIPHERS;
    goto error;
  }

  /* Load certs from the trusted ca */
  if (SSL_CTX_load_verify_locations(ssl_fd->ssl_context, ca_file, ca_path) <=
      0) {
    DBUG_PRINT("warning", ("SSL_CTX_load_verify_locations failed"));
    if (ca_file || ca_path) {
      /* fail only if ca file or ca path were supplied and looking into
         them fails. */
      DBUG_PRINT("warning", ("SSL_CTX_load_verify_locations failed"));
      *error = SSL_INITERR_BAD_PATHS;
      goto error;
    }

    /* otherwise go use the defaults */
    if (SSL_CTX_set_default_verify_paths(ssl_fd->ssl_context) == 0) {
      *error = SSL_INITERR_BAD_PATHS;
      goto error;
    }
  }

  if (crl_file || crl_path) {
    X509_STORE *store = SSL_CTX_get_cert_store(ssl_fd->ssl_context);
    /* Load crls from the trusted ca */
    if (X509_STORE_load_locations(store, crl_file, crl_path) == 0 ||
        X509_STORE_set_flags(
            store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL) == 0) {
      DBUG_PRINT("warning", ("X509_STORE_load_locations for CRL failed"));
      *error = SSL_INITERR_BAD_PATHS;
      goto error;
    }
  }

  if (vio_set_cert_stuff(ssl_fd->ssl_context, cert_file, key_file, error)) {
    DBUG_PRINT("warning", ("vio_set_cert_stuff failed"));
    goto error;
  }

  /* Server specific check : Must have certificate and key file */
  if (!is_client && !key_file && !cert_file) {
    *error = SSL_INITERR_NO_USABLE_CTX;
    goto error;
  }

  /* DH stuff */
  dh = get_dh2048();
  if (SSL_CTX_set_tmp_dh(ssl_fd->ssl_context, dh) == 0) {
    DH_free(dh);
    *error = SSL_INITERR_DHFAIL;
    goto error;
  }
  DH_free(dh);

  /* ECDH stuff */
#if OPENSSL_VERSION_NUMBER < 0x10002000L
  /* We choose P-256 curve. */
  eckey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
  if (!eckey) {
    *error = SSL_INITERR_ECDHFAIL;
    goto error;
  }
  if (SSL_CTX_set_tmp_ecdh(ssl_fd->ssl_context, eckey) != 1) {
    EC_KEY_free(eckey);
    *error = SSL_INITERR_ECDHFAIL;
    goto error;
  }
#else
  if (SSL_CTX_set_ecdh_auto(ssl_fd->ssl_context, 1) == 0) {
    *error = SSL_INITERR_ECDHFAIL;
    goto error;
  }
#endif /* OPENSSL_VERSION_NUMBER < 0x10002000L */

  SSL_CTX_set_options(ssl_fd->ssl_context, ssl_ctx_options);
  if (!is_client) {
    SSL_CTX_set_session_cache_mode(ssl_fd->ssl_context, SSL_SESS_CACHE_OFF);
  }

  DBUG_PRINT("exit", ("OK 1"));

  return ssl_fd;

error:
  DBUG_PRINT("error", ("%s", sslGetErrString(*error)));
  report_errors();
  free_vio_ssl_fd(ssl_fd);
  return nullptr;
}

/************************ VioSSLConnectorFd
 * **********************************/
struct st_VioSSLFd *new_VioSSLConnectorFd(
    const char *key_file, const char *cert_file, const char *ca_file,
    const char *ca_path, const char *cipher, const char *ciphersuites,
    enum enum_ssl_init_error *error, const char *crl_file, const char *crl_path,
    const long ssl_ctx_flags) {
  struct st_VioSSLFd *ssl_fd;
  int verify = SSL_VERIFY_PEER;

  /*
    Turn off verification of servers certificate if both
    ca_file and ca_path is set to NULL
  */
  if (ca_file == nullptr && ca_path == nullptr) verify = SSL_VERIFY_NONE;

  if (!(ssl_fd = new_VioSSLFd(key_file, cert_file, ca_file, ca_path, cipher,
                              ciphersuites, true, error, crl_file, crl_path,
                              ssl_ctx_flags))) {
    return nullptr;
  }

  /* Init the VioSSLFd as a "connector" ie. the client side */

  SSL_CTX_set_verify(ssl_fd->ssl_context, verify, nullptr);

  return ssl_fd;
}

/********************** VioSSLConnectorFdFromContext ************************/
struct st_VioSSLFd *new_VioSSLConnectorFdFromContext(
    SSL_CTX *context, enum enum_ssl_init_error *error) {
  struct st_VioSSLFd *ssl_fd;
  if (!(ssl_fd = ((struct st_VioSSLFd *)my_malloc(
            key_memory_vio_ssl_fd, sizeof(struct st_VioSSLFd), MYF(0))))) {
    *error = SSL_INITERR_MEMFAIL;
    return nullptr;
  }
  ssl_fd->ssl_context = context;
  ssl_fd->owned = false;

  return ssl_fd;
}

/************************ VioSSLAcceptorFd **********************************/
struct st_VioSSLFd *new_VioSSLAcceptorFd(
    const char *key_file, const char *cert_file, const char *ca_file,
    const char *ca_path, const char *cipher, const char *ciphersuites,
    enum enum_ssl_init_error *error, const char *crl_file, const char *crl_path,
    const long ssl_ctx_flags) {
  struct st_VioSSLFd *ssl_fd;
  int verify = SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE;
  if (!(ssl_fd = new_VioSSLFd(key_file, cert_file, ca_file, ca_path, cipher,
                              ciphersuites, false, error, crl_file, crl_path,
                              ssl_ctx_flags))) {
    return nullptr;
  }
  /* Init the the VioSSLFd as a "acceptor" ie. the server side */

  /* Set max number of cached sessions, returns the previous size */
  SSL_CTX_sess_set_cache_size(ssl_fd->ssl_context, 128);

  SSL_CTX_set_verify(ssl_fd->ssl_context, verify, nullptr);

  /*
    Set session_id - an identifier for this server session
    Use the ssl_fd pointer
   */
  SSL_CTX_set_session_id_context(ssl_fd->ssl_context,
                                 (const unsigned char *)ssl_fd, sizeof(ssl_fd));

  return ssl_fd;
}

void free_vio_ssl_fd(struct st_VioSSLFd *fd) {
  if (fd->owned) {
    SSL_CTX_free(fd->ssl_context);
  }
  my_free(fd);
}
