/* Copyright (c) 2011, 2020, Oracle and/or its affiliates. All rights reserved.

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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <stdarg.h>
#include <string.h>

#include "my_dbug.h"
#include "my_inttypes.h"

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include "crypt_genhash_impl.h"
#include "errmsg.h"
#include "m_ctype.h"
#include "mysql/client_authentication.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysys_err.h"
#include "sql_common.h"
#include "sql_string.h"
#if defined(_WIN32) && !defined(_OPENSSL_Applink) && \
    defined(HAVE_OPENSSL_APPLINK_C)
#include <openssl/applink.c>
#endif
#include "client_async_authentication.h"
#include "mysql/plugin.h"
#include "sha2.h"
#include "violite.h"

#define MAX_CIPHER_LENGTH 1024

#define SHA2_SCRAMBLE_LENGTH SHA256_DIGEST_LENGTH

mysql_mutex_t g_public_key_mutex;

int sha256_password_init(char *, size_t, int, va_list) {
  mysql_mutex_init(0, &g_public_key_mutex, MY_MUTEX_INIT_SLOW);
  return 0;
}

int sha256_password_deinit(void) {
  mysql_reset_server_public_key();
  mysql_mutex_destroy(&g_public_key_mutex);
  return 0;
}

static RSA *g_public_key = nullptr;

/**
  Reads and parse RSA public key data from a file.

  @param mysql connection handle with file path data

  @return Pointer to the RSA public key storage buffer
*/
static RSA *rsa_init(MYSQL *mysql) {
  RSA *key = nullptr;

  mysql_mutex_lock(&g_public_key_mutex);
  key = g_public_key;
  mysql_mutex_unlock(&g_public_key_mutex);

  if (key != nullptr) return key;

  FILE *pub_key_file = nullptr;

  if (mysql->options.extension != nullptr &&
      mysql->options.extension->server_public_key_path != nullptr &&
      mysql->options.extension->server_public_key_path[0] != '\0') {
    pub_key_file =
        fopen(mysql->options.extension->server_public_key_path, "rb");
  }
  /* No public key is used; return 0 without errors to indicate this. */
  else
    return nullptr;

  if (pub_key_file == nullptr) {
    /*
      If a key path was submitted but no key located then we print an error
      message. Else we just report that there is no public key.
    */
    my_message_local(WARNING_LEVEL, EE_FAILED_TO_LOCATE_SERVER_PUBLIC_KEY,
                     mysql->options.extension->server_public_key_path);

    return nullptr;
  }

  mysql_mutex_lock(&g_public_key_mutex);
  key = g_public_key =
      PEM_read_RSA_PUBKEY(pub_key_file, nullptr, nullptr, nullptr);
  mysql_mutex_unlock(&g_public_key_mutex);
  fclose(pub_key_file);
  if (g_public_key == nullptr) {
    ERR_clear_error();
    my_message_local(WARNING_LEVEL, EE_PUBLIC_KEY_NOT_IN_PEM_FORMAT,
                     mysql->options.extension->server_public_key_path);
    return nullptr;
  }

  return key;
}

/**
  Authenticate the client using the RSA or TLS and a SHA256 salted password.

  @param vio Provides plugin access to communication channel
  @param mysql Client connection handler

  @return Error status
    @retval CR_ERROR An error occurred.
    @retval CR_OK Authentication succeeded.
*/

int sha256_password_auth_client(MYSQL_PLUGIN_VIO *vio, MYSQL *mysql) {
  bool uses_password = mysql->passwd[0] != 0;
  unsigned char encrypted_password[MAX_CIPHER_LENGTH];
  static char request_public_key = '\1';
  RSA *public_key = nullptr;
  bool got_public_key_from_server = false;
  bool connection_is_secure = false;
  unsigned char scramble_pkt[20];
  unsigned char *pkt;

  DBUG_TRACE;

  /*
    Get the scramble from the server because we need it when sending encrypted
    password.
  */
  if (vio->read_packet(vio, &pkt) != SCRAMBLE_LENGTH + 1) {
    DBUG_PRINT("info", ("Scramble is not of correct length."));
    return CR_ERROR;
  }
  if (pkt[SCRAMBLE_LENGTH] != '\0') {
    DBUG_PRINT("info", ("Missing protocol token in scramble data."));
    return CR_ERROR;
  }
  /*
    Copy the scramble to the stack or it will be lost on the next use of the
    net buffer.
  */
  memcpy(scramble_pkt, pkt, SCRAMBLE_LENGTH);

  if (mysql_get_ssl_cipher(mysql) != nullptr) connection_is_secure = true;

  /* If connection isn't secure attempt to get the RSA public key file */
  if (!connection_is_secure) {
    public_key = rsa_init(mysql);
  }

  if (!uses_password) {
    /* We're not using a password */
    static const unsigned char zero_byte = '\0';
    if (vio->write_packet(vio, &zero_byte, 1)) return CR_ERROR;
  } else {
    /* Password is a 0-terminated byte array ('\0' character included) */
    unsigned int passwd_len =
        static_cast<unsigned int>(strlen(mysql->passwd) + 1);
    if (!connection_is_secure) {
      /*
        If no public key; request one from the server.
      */
      if (public_key == nullptr) {
        if (vio->write_packet(vio, (const unsigned char *)&request_public_key,
                              1))
          return CR_ERROR;

        int packet_len = 0;
        unsigned char *packet;
        if ((packet_len = vio->read_packet(vio, &packet)) == -1)
          return CR_ERROR;
        BIO *bio = BIO_new_mem_buf(packet, packet_len);
        public_key = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (public_key == nullptr) {
          ERR_clear_error();
          return CR_ERROR;
        }
        got_public_key_from_server = true;
      }

      /*
        An arbitrary limitation based on the assumption that passwords
        larger than e.g. 15 symbols don't contribute to security.
        Note also that it's furter restricted to RSA_size() - 41 down
        below, so this leaves 471 bytes of possible RSA key sizes which
        should be reasonably future-proof.
        We avoid heap allocation for speed reasons.
      */
      char passwd_scramble[512];

      if (passwd_len > sizeof(passwd_scramble)) {
        /* password too long for the buffer */
        if (got_public_key_from_server) RSA_free(public_key);
        return CR_ERROR;
      }
      memmove(passwd_scramble, mysql->passwd, passwd_len);

      /* Obfuscate the plain text password with the session scramble */
      xor_string(passwd_scramble, passwd_len - 1, (char *)scramble_pkt,
                 SCRAMBLE_LENGTH);
      /* Encrypt the password and send it to the server */
      int cipher_length = RSA_size(public_key);
      /*
        When using RSA_PKCS1_OAEP_PADDING the password length must be less
        than RSA_size(rsa) - 41.
      */
      if (passwd_len + 41 >= (unsigned)cipher_length) {
        /* password message is to long */
        if (got_public_key_from_server) RSA_free(public_key);
        return CR_ERROR;
      }
      RSA_public_encrypt(passwd_len, (unsigned char *)passwd_scramble,
                         encrypted_password, public_key,
                         RSA_PKCS1_OAEP_PADDING);
      if (got_public_key_from_server) RSA_free(public_key);

      if (vio->write_packet(vio, (uchar *)encrypted_password, cipher_length))
        return CR_ERROR;
    } else {
      /* The vio is encrypted already; just send the plain text passwd */
      if (vio->write_packet(vio, (uchar *)mysql->passwd, passwd_len))
        return CR_ERROR;
    }
  }

  return CR_OK;
}

/**
  Non blocking version of sha256_password_auth_client
*/
net_async_status sha256_password_auth_client_nonblocking(MYSQL_PLUGIN_VIO *vio,
                                                         MYSQL *mysql,
                                                         int *result) {
  DBUG_TRACE;
  net_async_status status = NET_ASYNC_NOT_READY;
  unsigned char encrypted_password[MAX_CIPHER_LENGTH];
  static char request_public_key = '\1';
  static RSA *public_key = nullptr;
  bool got_public_key_from_server = false;
  int io_result;
  bool connection_is_secure = (mysql_get_ssl_cipher(mysql) != nullptr);
  unsigned char scramble_pkt[20];
  unsigned char *pkt;
  unsigned int passwd_len =
      static_cast<unsigned int>(strlen(mysql->passwd) + 1);

  mysql_async_auth *ctx = ASYNC_DATA(mysql)->connect_context->auth_context;
  switch (static_cast<client_auth_sha256_password_plugin_status>(
      ctx->client_auth_plugin_state)) {
    case client_auth_sha256_password_plugin_status::SHA256_READING_PASSWORD:
      status = vio->read_packet_nonblocking(vio, &pkt, &io_result);
      if (status == NET_ASYNC_NOT_READY) {
        return NET_ASYNC_NOT_READY;
      }
      if (io_result != SCRAMBLE_LENGTH + 1) {
        DBUG_PRINT("info", ("Scramble is not of correct length."));
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
      if (pkt[SCRAMBLE_LENGTH] != '\0') {
        DBUG_PRINT("info", ("Missing protocol token in scramble data."));
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
      memcpy(scramble_pkt, pkt, SCRAMBLE_LENGTH);
      if (connection_is_secure)
        ctx->client_auth_plugin_state =
            client_auth_sha256_password_plugin_status::
                SHA256_SEND_PLAIN_PASSWORD;
      else
        ctx->client_auth_plugin_state =
            client_auth_sha256_password_plugin_status::
                SHA256_REQUEST_PUBLIC_KEY;
      return NET_ASYNC_NOT_READY;
    case client_auth_sha256_password_plugin_status::SHA256_REQUEST_PUBLIC_KEY: {
      public_key = rsa_init(mysql);
      /* If no public key; request one from the server. */
      if (public_key == nullptr) {
        status = vio->write_packet_nonblocking(
            vio, (const unsigned char *)&request_public_key, 1, &io_result);
        if (status == NET_ASYNC_NOT_READY) {
          return NET_ASYNC_NOT_READY;
        }
        if (io_result) {
          *result = CR_ERROR;
          return NET_ASYNC_COMPLETE;
        }
      }
      set_mysql_extended_error(mysql, CR_AUTH_PLUGIN_ERR, unknown_sqlstate,
                               ER_CLIENT(CR_AUTH_PLUGIN_ERR), "sha256_password",
                               "Authentication requires SSL encryption");
      *result = CR_ERROR;
      return NET_ASYNC_COMPLETE;
    }
      ctx->client_auth_plugin_state =
          client_auth_sha256_password_plugin_status::SHA256_READ_PUBLIC_KEY;
      /* FALLTHROUGH */
    case client_auth_sha256_password_plugin_status::SHA256_READ_PUBLIC_KEY:
      if (public_key == nullptr) {
        status = vio->read_packet_nonblocking(vio, &pkt, &io_result);
        if (status == NET_ASYNC_NOT_READY) {
          return NET_ASYNC_NOT_READY;
        }
        if (io_result <= 0) {
          *result = CR_ERROR;
          return NET_ASYNC_COMPLETE;
        }
        BIO *bio = BIO_new_mem_buf(pkt, io_result);
        public_key = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (public_key == nullptr) {
          ERR_clear_error();
          *result = CR_ERROR;
          return NET_ASYNC_COMPLETE;
        }
        got_public_key_from_server = true;
      }
      ctx->client_auth_plugin_state =
          client_auth_sha256_password_plugin_status::
              SHA256_SEND_ENCRYPTED_PASSWORD;
      /* FALLTHROUGH */
    case client_auth_sha256_password_plugin_status::
        SHA256_SEND_ENCRYPTED_PASSWORD: {
      char passwd_scramble[512];

      if (passwd_len > sizeof(passwd_scramble)) {
        /* password too long for the buffer */
        if (got_public_key_from_server) RSA_free(public_key);
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
      memmove(passwd_scramble, mysql->passwd, passwd_len);

      /* Obfuscate the plain text password with the session scramble */
      xor_string(passwd_scramble, passwd_len - 1, (char *)scramble_pkt,
                 SCRAMBLE_LENGTH);
      /* Encrypt the password and send it to the server */
      int cipher_length = RSA_size(public_key);
      /*
        When using RSA_PKCS1_OAEP_PADDING the password length must be less
        than RSA_size(rsa) - 41.
      */
      if (passwd_len + 41 >= (unsigned)cipher_length) {
        /* password message is to long */
        if (got_public_key_from_server) RSA_free(public_key);
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
      RSA_public_encrypt(passwd_len, (unsigned char *)passwd_scramble,
                         encrypted_password, public_key,
                         RSA_PKCS1_OAEP_PADDING);
      if (got_public_key_from_server) RSA_free(public_key);
      status = vio->write_packet_nonblocking(vio, (uchar *)encrypted_password,
                                             cipher_length, &io_result);
      if (status == NET_ASYNC_NOT_READY) {
        return NET_ASYNC_NOT_READY;
      }
      if (io_result < 0) {
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
    } break;
    case client_auth_sha256_password_plugin_status::
        SHA256_SEND_PLAIN_PASSWORD: {
      status = vio->write_packet_nonblocking(vio, (uchar *)mysql->passwd,
                                             passwd_len, &io_result);
      if (status == NET_ASYNC_NOT_READY) {
        return NET_ASYNC_NOT_READY;
      }
      if (io_result < 0) {
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
    } break;
    default:
      assert(0);
  }
  *result = CR_OK;
  return NET_ASYNC_COMPLETE;
}
/* caching_sha2_password */

int caching_sha2_password_init(char *, size_t, int, va_list) { return 0; }

int caching_sha2_password_deinit(void) { return 0; }

static bool is_secure_transport(MYSQL *mysql) {
  if (!mysql || !mysql->net.vio) return false;
  switch (mysql->net.vio->type) {
    case VIO_TYPE_SSL: {
      if (mysql_get_ssl_cipher(mysql) == nullptr) return false;
    }
    // Fall through
    case VIO_TYPE_SHARED_MEMORY:
    // Fall through
    case VIO_TYPE_SOCKET:
      return true;
    default:
      return false;
  }
  return false;
}

static char request_public_key = '\2';
static char fast_auth_success = '\3';
static char perform_full_authentication = '\4';

/**
  Authenticate the client using the RSA or TLS and a SHA2 salted password.

  @param vio Provides plugin access to communication channel
  @param mysql Client connection handler

  @return Error status
    @retval CR_ERROR An error occurred.
    @retval CR_OK Authentication succeeded.
*/
int caching_sha2_password_auth_client(MYSQL_PLUGIN_VIO *vio, MYSQL *mysql) {
  bool uses_password = mysql->passwd[0] != 0;
  unsigned char encrypted_password[MAX_CIPHER_LENGTH];
  // static char request_public_key= '\1';
  RSA *public_key = nullptr;
  bool got_public_key_from_server = false;
  bool connection_is_secure = false;
  unsigned char scramble_pkt[20];
  unsigned char *pkt;

  DBUG_TRACE;

  /*
    Get the scramble from the server because we need it when sending encrypted
    password.
  */
  if (vio->read_packet(vio, &pkt) != SCRAMBLE_LENGTH + 1) {
    DBUG_PRINT("info", ("Scramble is not of correct length."));
    return CR_ERROR;
  }
  if (pkt[SCRAMBLE_LENGTH] != '\0') {
    DBUG_PRINT("info", ("Missing protocol token in scramble data."));
    return CR_ERROR;
  }

  /*
    Copy the scramble to the stack or it will be lost on the next use of the
    net buffer.
  */
  memcpy(scramble_pkt, pkt, SCRAMBLE_LENGTH);

  connection_is_secure = is_secure_transport(mysql);

  if (!uses_password) {
    /* We're not using a password */
    static const unsigned char zero_byte = '\0';
    if (vio->write_packet(vio, &zero_byte, 1)) return CR_ERROR;
    return CR_OK;
  } else {
    /* Password is a 0-terminated byte array ('\0' character included) */
    unsigned int passwd_len =
        static_cast<unsigned int>(strlen(mysql->passwd) + 1);
    int pkt_len = 0;
    {
      /* First try with SHA2 scramble */
      unsigned char sha2_scramble[SHA2_SCRAMBLE_LENGTH];
      if (generate_sha256_scramble(sha2_scramble, SHA2_SCRAMBLE_LENGTH,
                                   mysql->passwd, passwd_len - 1,
                                   (char *)scramble_pkt, SCRAMBLE_LENGTH)) {
        set_mysql_extended_error(mysql, CR_AUTH_PLUGIN_ERR, unknown_sqlstate,
                                 ER_CLIENT(CR_AUTH_PLUGIN_ERR),
                                 "caching_sha2_password",
                                 "Failed to generate scramble");
        return CR_ERROR;
      }

      if (vio->write_packet(vio, sha2_scramble, SHA2_SCRAMBLE_LENGTH))
        return CR_ERROR;

      if ((pkt_len = vio->read_packet(vio, &pkt)) == -1) return CR_ERROR;

      if (pkt_len == 1 && *pkt == fast_auth_success) return CR_OK;

      /* An OK packet would follow */
    }

    if (pkt_len != 1 || *pkt != perform_full_authentication) {
      DBUG_PRINT("info", ("Unexpected reply from server."));
      return CR_ERROR;
    }

    /* If connection isn't secure attempt to get the RSA public key file */
    if (!connection_is_secure) {
      public_key = rsa_init(mysql);

      if (public_key == nullptr && mysql->options.extension &&
          mysql->options.extension->get_server_public_key) {
        // If no public key; request one from the server.
        if (vio->write_packet(vio, (const unsigned char *)&request_public_key,
                              1))
          return CR_ERROR;

        if ((pkt_len = vio->read_packet(vio, &pkt)) <= 0) return CR_ERROR;
        BIO *bio = BIO_new_mem_buf(pkt, pkt_len);
        public_key = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (public_key == nullptr) {
          ERR_clear_error();
          DBUG_PRINT("info", ("Failed to parse public key"));
          return CR_ERROR;
        }
        got_public_key_from_server = true;
      }

      if (public_key) {
        /*
           An arbitrary limitation based on the assumption that passwords
           larger than e.g. 15 symbols don't contribute to security.
           Note also that it's further restricted to RSA_size() - 11 down
           below, so this leaves 471 bytes of possible RSA key sizes which
           should be reasonably future-proof.
           We avoid heap allocation for speed reasons.
         */
        char passwd_scramble[512];

        if (passwd_len > sizeof(passwd_scramble)) {
          /* password too long for the buffer */
          if (got_public_key_from_server) RSA_free(public_key);
          DBUG_PRINT("info", ("Password is too long."));
          return CR_ERROR;
        }
        memmove(passwd_scramble, mysql->passwd, passwd_len);

        /* Obfuscate the plain text password with the session scramble */
        xor_string(passwd_scramble, passwd_len - 1, (char *)scramble_pkt,
                   SCRAMBLE_LENGTH);
        /* Encrypt the password and send it to the server */
        int cipher_length = RSA_size(public_key);
        /*
           When using RSA_PKCS1_OAEP_PADDING the password length must be less
           than RSA_size(rsa) - 41.
         */
        if (passwd_len + 41 >= (unsigned)cipher_length) {
          /* password message is to long */
          if (got_public_key_from_server) RSA_free(public_key);
          DBUG_PRINT("info", ("Password is too long to be encrypted using "
                              "given public key."));
          return CR_ERROR;
        }
        RSA_public_encrypt(passwd_len, (unsigned char *)passwd_scramble,
                           encrypted_password, public_key,
                           RSA_PKCS1_OAEP_PADDING);
        if (got_public_key_from_server) RSA_free(public_key);

        if (vio->write_packet(vio, (uchar *)encrypted_password, cipher_length))
          return CR_ERROR;
      } else {
        set_mysql_extended_error(mysql, CR_AUTH_PLUGIN_ERR, unknown_sqlstate,
                                 ER_CLIENT(CR_AUTH_PLUGIN_ERR),
                                 "caching_sha2_password",
                                 "Authentication requires secure connection.");
        return CR_ERROR;
      }
    } else {
      /* The vio is encrypted already; just send the plain text passwd */
      if (vio->write_packet(vio, (uchar *)mysql->passwd, passwd_len))
        return CR_ERROR;
    }
  }

  return CR_OK;
}

/**
  non blocking version of caching_sha2_password_auth_client
*/
net_async_status caching_sha2_password_auth_client_nonblocking(
    MYSQL_PLUGIN_VIO *vio, MYSQL *mysql, int *result) {
  DBUG_TRACE;
  int io_result;
  net_async_status status = NET_ASYNC_NOT_READY;
  static unsigned char encrypted_password[MAX_CIPHER_LENGTH];
  static RSA *public_key = nullptr;
  bool connection_is_secure = is_secure_transport(mysql);
  bool got_public_key_from_server = false;
  static unsigned char scramble_pkt[20];
  static int cipher_length = 0;
  static unsigned int passwd_len = 0;
  unsigned char *pkt;
  mysql_async_auth *ctx = ASYNC_DATA(mysql)->connect_context->auth_context;

  switch (static_cast<client_auth_caching_sha2_password_plugin_status>(
      ctx->client_auth_plugin_state)) {
    case client_auth_caching_sha2_password_plugin_status::
        CACHING_SHA2_READING_PASSWORD:
      /*
        Get the scramble from the server because we need it when sending
        encrypted password.
      */
      status = vio->read_packet_nonblocking(vio, &pkt, &io_result);
      if (status == NET_ASYNC_NOT_READY) {
        return NET_ASYNC_NOT_READY;
      }
      if (io_result != SCRAMBLE_LENGTH + 1) {
        DBUG_PRINT("info", ("Scramble is not of correct length."));
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
      if (pkt[SCRAMBLE_LENGTH] != '\0') {
        DBUG_PRINT("info", ("Missing protocol token in scramble data."));
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
      /*
        Copy the scramble to the stack or it will be lost on the next use
        of the net buffer.
      */
      memcpy(scramble_pkt, pkt, SCRAMBLE_LENGTH);
      ctx->client_auth_plugin_state =
          client_auth_caching_sha2_password_plugin_status::
              CACHING_SHA2_WRITING_RESPONSE;
      /* FALLTHROUGH */
    case client_auth_caching_sha2_password_plugin_status::
        CACHING_SHA2_WRITING_RESPONSE:
      if (mysql->passwd[0] == 0) {
        /* We're not using a password */
        static const unsigned char zero_byte = '\0';
        status = vio->write_packet_nonblocking(vio, &zero_byte, 1, &io_result);
        if (status == NET_ASYNC_NOT_READY) {
          return NET_ASYNC_NOT_READY;
        }
        if (io_result) {
          *result = CR_ERROR;
          return NET_ASYNC_COMPLETE;
        }
        *result = CR_OK;
        return NET_ASYNC_COMPLETE;
      } else {
        /* Password is a 0-terminated byte array ('\0' character included) */
        passwd_len = static_cast<unsigned int>(strlen(mysql->passwd) + 1);
        /* First try with SHA2 scramble */
        unsigned char sha2_scramble[SHA2_SCRAMBLE_LENGTH];
        if (generate_sha256_scramble(sha2_scramble, SHA2_SCRAMBLE_LENGTH,
                                     mysql->passwd, passwd_len - 1,
                                     (char *)scramble_pkt, SCRAMBLE_LENGTH)) {
          set_mysql_extended_error(mysql, CR_AUTH_PLUGIN_ERR, unknown_sqlstate,
                                   ER_CLIENT(CR_AUTH_PLUGIN_ERR),
                                   "caching_sha2_password",
                                   "Failed to generate scramble");
          *result = CR_ERROR;
          return NET_ASYNC_COMPLETE;
        }
        status = vio->write_packet_nonblocking(
            vio, sha2_scramble, SHA2_SCRAMBLE_LENGTH, &io_result);
        if (status == NET_ASYNC_NOT_READY) {
          return NET_ASYNC_NOT_READY;
        }
        if (io_result) {
          *result = CR_ERROR;
          return NET_ASYNC_COMPLETE;
        }
      }
      ctx->client_auth_plugin_state =
          client_auth_caching_sha2_password_plugin_status::
              CACHING_SHA2_CHALLENGE_RESPONSE;
      /* FALLTHROUGH */
    case client_auth_caching_sha2_password_plugin_status::
        CACHING_SHA2_CHALLENGE_RESPONSE:
      status = vio->read_packet_nonblocking(vio, &pkt, &io_result);
      if (status == NET_ASYNC_NOT_READY) {
        return NET_ASYNC_NOT_READY;
      }
      if (io_result == -1) {
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
      if (io_result == 1 && *pkt == fast_auth_success) {
        *result = CR_OK;
        return NET_ASYNC_COMPLETE;
      }
      if (io_result != 1 || *pkt != perform_full_authentication) {
        DBUG_PRINT("info", ("Unexpected reply from server."));
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
      if (!connection_is_secure)
        ctx->client_auth_plugin_state =
            client_auth_caching_sha2_password_plugin_status::
                CACHING_SHA2_REQUEST_PUBLIC_KEY;
      else
        ctx->client_auth_plugin_state =
            client_auth_caching_sha2_password_plugin_status::
                CACHING_SHA2_SEND_PLAIN_PASSWORD;
      return NET_ASYNC_NOT_READY;
      /* FALLTHROUGH */
    case client_auth_caching_sha2_password_plugin_status::
        CACHING_SHA2_REQUEST_PUBLIC_KEY:
      /* If connection isn't secure attempt to get the RSA public key file */
      {
        public_key = rsa_init(mysql);

        if (public_key == nullptr && mysql->options.extension &&
            mysql->options.extension->get_server_public_key) {
          status = vio->write_packet_nonblocking(
              vio, (const unsigned char *)&request_public_key, 1, &io_result);
          if (status == NET_ASYNC_NOT_READY) {
            return NET_ASYNC_NOT_READY;
          }
          if (io_result) {
            *result = CR_ERROR;
            return NET_ASYNC_COMPLETE;
          }
        }
      }
      ctx->client_auth_plugin_state =
          client_auth_caching_sha2_password_plugin_status::
              CACHING_SHA2_READ_PUBLIC_KEY;
      /* FALLTHROUGH */
    case client_auth_caching_sha2_password_plugin_status::
        CACHING_SHA2_READ_PUBLIC_KEY: {
      if (public_key == nullptr && mysql->options.extension &&
          mysql->options.extension->get_server_public_key) {
        status = vio->read_packet_nonblocking(vio, &pkt, &io_result);
        if (status == NET_ASYNC_NOT_READY) {
          return NET_ASYNC_NOT_READY;
        }
        if (io_result <= 0) {
          *result = CR_ERROR;
          return NET_ASYNC_COMPLETE;
        }
        int pkt_len = 0;
        BIO *bio = BIO_new_mem_buf(pkt, pkt_len);
        public_key = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if (public_key == nullptr) {
          ERR_clear_error();
          DBUG_PRINT("info", ("Failed to parse public key"));
          *result = CR_ERROR;
          return NET_ASYNC_COMPLETE;
        }
        got_public_key_from_server = true;
      }
      if (public_key) {
        char passwd_scramble[512];
        if (passwd_len > sizeof(passwd_scramble)) {
          /* password too long for the buffer */
          if (got_public_key_from_server) RSA_free(public_key);
          DBUG_PRINT("info", ("Password is too long."));
          *result = CR_ERROR;
          return NET_ASYNC_COMPLETE;
        }
        memmove(passwd_scramble, mysql->passwd, passwd_len);
        /* Obfuscate the plain text password with the session scramble */
        xor_string(passwd_scramble, passwd_len - 1, (char *)scramble_pkt,
                   SCRAMBLE_LENGTH);
        /* Encrypt the password and send it to the server */
        cipher_length = RSA_size(public_key);
        /*
          When using RSA_PKCS1_OAEP_PADDING the password length
          must be less than RSA_size(rsa) - 41.
        */
        if (passwd_len + 41 >= (unsigned)cipher_length) {
          /* password message is to long */
          if (got_public_key_from_server) RSA_free(public_key);
          DBUG_PRINT("info", ("Password is too long to be encrypted using "
                              "given public key."));
          *result = CR_ERROR;
          return NET_ASYNC_COMPLETE;
        }
        RSA_public_encrypt(passwd_len, (unsigned char *)passwd_scramble,
                           encrypted_password, public_key,
                           RSA_PKCS1_OAEP_PADDING);
        if (got_public_key_from_server) RSA_free(public_key);
      } else {
        set_mysql_extended_error(mysql, CR_AUTH_PLUGIN_ERR, unknown_sqlstate,
                                 ER_CLIENT(CR_AUTH_PLUGIN_ERR),
                                 "caching_sha2_password",
                                 "Authentication requires secure connection.");
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
    }
      ctx->client_auth_plugin_state =
          client_auth_caching_sha2_password_plugin_status::
              CACHING_SHA2_SEND_ENCRYPTED_PASSWORD;
    /* FALLTHROUGH */
    case client_auth_caching_sha2_password_plugin_status::
        CACHING_SHA2_SEND_ENCRYPTED_PASSWORD: {
      status = vio->write_packet_nonblocking(vio, (uchar *)encrypted_password,
                                             cipher_length, &io_result);
      if (status == NET_ASYNC_NOT_READY) {
        return NET_ASYNC_NOT_READY;
      }
      if (io_result < 0) {
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
    } break;
    case client_auth_caching_sha2_password_plugin_status::
        CACHING_SHA2_SEND_PLAIN_PASSWORD: {
      status = vio->write_packet_nonblocking(vio, (uchar *)mysql->passwd,
                                             passwd_len, &io_result);
      if (status == NET_ASYNC_NOT_READY) {
        return NET_ASYNC_NOT_READY;
      }
      if (io_result < 0) {
        *result = CR_ERROR;
        return NET_ASYNC_COMPLETE;
      }
    } break;
    default:
      assert(0);
  }
  *result = CR_OK;
  return NET_ASYNC_COMPLETE;
}

void STDCALL mysql_reset_server_public_key(void) {
  DBUG_TRACE;
  mysql_mutex_lock(&g_public_key_mutex);
  if (g_public_key) RSA_free(g_public_key);
  g_public_key = nullptr;
  mysql_mutex_unlock(&g_public_key_mutex);
}
