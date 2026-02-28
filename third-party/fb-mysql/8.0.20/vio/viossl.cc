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

/*
  Note that we can't have assertion on file descriptors;  The reason for
  this is that during mysql shutdown, another thread can close a file
  we are working on.  In this case we should just return read errors from
  the file descriptior.
*/

#include <errno.h>
#include <stddef.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "mysql/psi/mysql_socket.h"
#include "vio/vio_priv.h"

/*
  BIO_set_callback_ex was added in openSSL 1.1.1
  For older openSSL, use the deprecated BIO_set_callback.
*/
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
#define HAVE_BIO_SET_CALLBACK_EX
#endif

/* clang-format off */
/**
  @page page_protocol_basic_tls TLS

  The MySQL Protocol also supports encryption and authentication via TLS.
  The encryption is transparent to the rest of the protocol and is applied
  after the data is compressed right before the data is written to
  the network layer.

  The TLS suppport is announced in
  @ref page_protocol_connection_phase_packets_protocol_handshake sent by the
  server via ::CLIENT_SSL and is enabled if the client returns the same
  capability.

  For an unencrypted connection the server starts with its
  @ref page_protocol_connection_phase_packets_protocol_handshake :

  ~~~~~~~
  36 00 00 00 0a 35 2e 35    2e 32 2d 6d 32 00 52 00    6....5.5.2-m2.R.
  00 00 22 3d 4e 50 29 75    39 56 00 ff ff 08 02 00    .."=NP)u9V......
  00 00 00 00 00 00 00 00    00 00 00 00 00 29 64 40    .............)d@
  52 5c 55 78 7a 7c 21 29    4b 00                      R\Uxz|!)K.
  ~~~~~~~

  ... and the client returns its
  @ref page_protocol_connection_phase_packets_protocol_handshake_response

  ~~~~~~~
  3a 00 00 01 05 a6 03 00    00 00 00 01 08 00 00 00    :...............
  00 00 00 00 00 00 00 00    00 00 00 00 00 00 00 00    ................
  00 00 00 00 72 6f 6f 74    00 14 14 63 6b 70 99 8a    ....root...ckp..
  b6 9e 96 87 a2 30 9a 40    67 2b 83 38 85 4b          .....0.@g+.8.K
  ~~~~~~~

  If client wants to do TLS and the server supports it, it would send a
  @ref page_protocol_connection_phase_packets_protocol_ssl_request with
  ::CLIENT_SSL capability enabled.

  ~~~~~~~
  20 00 00 01 05 ae 03 00    00 00 00 01 08 00 00 00     ...............
  00 00 00 00 00 00 00 00    00 00 00 00 00 00 00 00    ................
  00 00 00 00                                           ....
  ~~~~~~~

  Then the rest of the communication is swithed to TLS:
  ~~~~~~~
  16 03 01 00 5e 01 00 00    5a 03 01 4c a3 49 2e 7a    ....^...Z..L.I.z
  b5 06 75 68 5c 30 36 73    f1 82 79 70 58 4c 64 bb    ..uh\06s..ypXLd.
  47 7e 90 cd 9b 30 c5 66    65 da 35 00 00 2c 00 39    G~...0.fe.5..,.9
  00 38 00 35 00 16 00 13    00 0a 00 33 00 32 00 2f    .8.5.......3.2./
  00 9a 00 99 00 96 00 05    00 04 00 15 00 12 00 09    ................
  00 14 00 11 00 08 00 06    00 03 02 01 00 00 04 00    ................
  23 00 00                                              #..
  ~~~~~~~

  The preceding packet is from SSL_connect() which does the
  [TLS handshake](https://en.wikipedia.org/wiki/Transport_Layer_Security#TLS_handshake)

  Once the TLS tunnel is established the normal communication continues
  starting with the client sending the
  @ref page_protocol_connection_phase_packets_protocol_handshake_response

  See @ref sect_protocol_connection_phase_initial_handshake_ssl_handshake
  for a diagram of the exchange.

  @sa cli_establish_ssl, parse_client_handshake_packet
*/
/* clang-format on */

#ifndef DBUG_OFF

static void report_errors(SSL *ssl) {
  unsigned long l;
  const char *file;
  const char *data;
  int line, flags = 0;
  char buf[512];

  DBUG_TRACE;

  while ((l = ERR_get_error_line_data(&file, &line, &data, &flags))) {
    DBUG_PRINT("error", ("OpenSSL: %s:%s:%d:%s\n", ERR_error_string(l, buf),
                         file, line, (flags & ERR_TXT_STRING) ? data : ""));
  }

  if (ssl)
    DBUG_PRINT("error",
               ("error: %s", ERR_error_string(SSL_get_error(ssl, l), buf)));

  DBUG_PRINT("info", ("socket_errno: %d", socket_errno));
}

#endif

/**
  Obtain the equivalent system error status for the last SSL I/O operation.

  @param ssl_error  The result code of the failed TLS/SSL I/O operation.
*/

static void ssl_set_sys_error(int ssl_error) {
  int error = 0;

  switch (ssl_error) {
    case SSL_ERROR_ZERO_RETURN:
      error = SOCKET_ECONNRESET;
      break;
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
#ifdef SSL_ERROR_WANT_CONNECT
    case SSL_ERROR_WANT_CONNECT:
#endif
#ifdef SSL_ERROR_WANT_ACCEPT
    case SSL_ERROR_WANT_ACCEPT:
#endif
      error = SOCKET_EWOULDBLOCK;
      break;
    case SSL_ERROR_SSL:
      /* Protocol error. */
#ifdef EPROTO
      error = EPROTO;
#else
      error = SOCKET_ECONNRESET;
#endif
      break;
    case SSL_ERROR_SYSCALL:
    case SSL_ERROR_NONE:
    default:
      break;
  };

  /* Set error status to a equivalent of the SSL error. */
  if (error) {
#ifdef _WIN32
    WSASetLastError(error);
#else
    errno = error;
#endif
  }
}

/**
  Check if an operation should be retried and handle errors

  This function does the following:
    - it indicates whether a SSL I/O operation must be retried later;
    - if DBUG is enabled it prints all the errors in the thread's queue to DBUG
    - it clears the OpenSSL error queue, thus the next OpenSSL-operation can be
      performed even after failed OpenSSL-call.

  Note that this is not done for SSL_ERROR_WANT_READ/SSL_ERROR_WANT_WRITE
  since these are not treated as errors and a call to the function is retried.

  When SSL_ERROR_SSL is returned the ERR code of the top error in the queue is
  peeked and returned to the caller so they can call ERR_error_string_n() and
  retrieve the right error message.

  @param vio  VIO object representing a SSL connection.
  @param ret  Value returned by a SSL I/O function.
  @param [out] event             The type of I/O event to wait/retry.
  @param [out] ssl_errno_holder  The SSL error code.

  @return Whether a SSL I/O operation should be deferred.
  @retval true    Temporary failure, retry operation.
  @retval false   Indeterminate failure.
*/

static bool ssl_should_retry(Vio *vio, int ret, enum enum_vio_io_event *event,
                             unsigned long *ssl_errno_holder) {
  int ssl_error, err_error;
  SSL *ssl = static_cast<SSL *>(vio->ssl_arg);
  bool should_retry = true;

  /* Retrieve the result for the SSL I/O operation. */
  ssl_error = SSL_get_error(ssl, ret);

  /* Retrieve the result for the SSL I/O operation. */
  switch (ssl_error) {
    case SSL_ERROR_WANT_READ:
      err_error = ssl_error;  // for backward compatibility.
      *event = VIO_IO_EVENT_READ;
      break;
    case SSL_ERROR_WANT_WRITE:
      err_error = ssl_error;  // for backward compatibility.
      *event = VIO_IO_EVENT_WRITE;
      break;
    default:
      /* first save the top ERR error */
      err_error = ERR_get_error();
      /* now report all remaining errors on and/or clear the error stack */
#ifndef DBUG_OFF /* Debug build */
      /* Note: the OpenSSL error queue gets cleared in report_errors(). */
      report_errors(ssl);
#else /* Release build */
      ERR_clear_error();
#endif
      should_retry = false;
      ssl_set_sys_error(ssl_error);
      break;
  }

  *ssl_errno_holder = err_error;

  return should_retry;
}

size_t vio_ssl_read(Vio *vio, uchar *buf, size_t size) {
  int ret;
  SSL *ssl = static_cast<SSL *>(vio->ssl_arg);
  unsigned long ssl_errno_not_used;

  DBUG_TRACE;

  while (true) {
    enum enum_vio_io_event event = VIO_IO_EVENT_READ;

    /*
      Check that the SSL thread's error queue is cleared. Otherwise
      SSL_read() returns an error from the error queue, when SSL_read() failed
      because it would block.
    */
    DBUG_ASSERT(ERR_peek_error() == 0);

    ret = SSL_read(ssl, buf, (int)size);

    if (ret >= 0) break;

    /* Process the SSL I/O error. */
    if (!ssl_should_retry(vio, ret, &event, &ssl_errno_not_used)) break;

    if (!vio->is_blocking_flag) {
      switch (event) {
        case VIO_IO_EVENT_READ:
          return VIO_SOCKET_WANT_READ;
        case VIO_IO_EVENT_WRITE:
          return VIO_SOCKET_WANT_WRITE;
        default:
          return VIO_SOCKET_ERROR;
      }
    }

    /* Attempt to wait for an I/O event. */
    if (vio_socket_io_wait(vio, event)) {
      if (vio_was_timeout(vio)) {
        return event == VIO_IO_EVENT_READ ? VIO_SOCKET_READ_TIMEOUT
                                          : VIO_SOCKET_WRITE_TIMEOUT;
      } else {
        return VIO_SOCKET_ERROR;
      }
    }
  }

  return ret < 0 ? -1 : ret;
}

size_t vio_ssl_write(Vio *vio, const uchar *buf, size_t size) {
  int ret;
  SSL *ssl = static_cast<SSL *>(vio->ssl_arg);
  unsigned long ssl_errno_not_used;

  DBUG_TRACE;

  while (true) {
    enum enum_vio_io_event event = VIO_IO_EVENT_READ;

    /*
      check that the SSL thread's error queue is cleared. Otherwise
      SSL_write() returns an error from the error queue, when SSL_write() failed
      because it would block.
    */
    DBUG_ASSERT(ERR_peek_error() == 0);

    ret = SSL_write(ssl, buf, (int)size);

    if (ret >= 0) break;

    /* Process the SSL I/O error. */
    if (!ssl_should_retry(vio, ret, &event, &ssl_errno_not_used)) break;

    if (!vio->is_blocking_flag) {
      switch (event) {
        case VIO_IO_EVENT_READ:
          return VIO_SOCKET_WANT_READ;
        case VIO_IO_EVENT_WRITE:
          return VIO_SOCKET_WANT_WRITE;
        default:
          return VIO_SOCKET_ERROR;
      }
    }

    /* Attempt to wait for an I/O event. */
    if (vio_socket_io_wait(vio, event)) {
      if (vio_was_timeout(vio)) {
        return event == VIO_IO_EVENT_READ ? VIO_SOCKET_READ_TIMEOUT
                                          : VIO_SOCKET_WRITE_TIMEOUT;
      } else {
        return VIO_SOCKET_ERROR;
      }
    }
  }

  return ret < 0 ? -1 : ret;
}

int vio_ssl_shutdown(Vio *vio) {
  int r = 0;
  SSL *ssl = (SSL *)vio->ssl_arg;
  DBUG_TRACE;

  if (ssl) {
    /*
    THE SSL standard says that SSL sockets must send and receive a close_notify
    alert on socket shutdown to avoid truncation attacks. However, this can
    cause problems since we often hold a lock during shutdown and this IO can
    take an unbounded amount of time to complete. Since our packets are self
    describing with length, we aren't vunerable to these attacks. Therefore,
    we just shutdown by closing the socket (quiet shutdown).
    */
    SSL_set_quiet_shutdown(ssl, 1);

    switch ((r = SSL_shutdown(ssl))) {
      case 1:
        /* Shutdown successful */
        break;
      case 0:
        /*
          Shutdown not yet finished - since the socket is going to
          be closed there is no need to call SSL_shutdown() a second
          time to wait for the other side to respond
        */
        break;
      default: /* Shutdown failed */
        DBUG_PRINT("vio_error",
                   ("SSL_shutdown() failed, error: %d", SSL_get_error(ssl, r)));
        break;
    }
  }
  return vio_shutdown(vio);
}

void vio_ssl_delete(Vio *vio) {
  if (!vio) return; /* It must be safe to delete null pointer */

  if (vio->inactive == false)
    vio_ssl_shutdown(vio); /* Still open, close connection first */

  if (vio->ssl_arg) {
    SSL_free((SSL *)vio->ssl_arg);
    vio->ssl_arg = nullptr;
  }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  ERR_remove_thread_state(0);
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */

  vio_delete(vio);
}

/** SSL handshake handler. */
extern "C" {
typedef int (*ssl_handshake_func_t)(SSL *);
}

/**
  Loop and wait until a SSL handshake is completed.

  @param vio    VIO object representing a SSL connection.
  @param ssl    SSL structure for the connection.
  @param func   SSL handshake handler.
  @param [out] ssl_errno_holder  The SSL error code.

  @return Return value is 1 on success.
*/

static ssize_t ssl_handshake_loop(Vio *vio, SSL *ssl, ssl_handshake_func_t func,
                                  unsigned long *ssl_errno_holder) {
  DBUG_TRACE;
  ssize_t ret = -1;

  vio->ssl_arg = ssl;

  /* Initiate the SSL handshake. */
  while (true) {
    enum enum_vio_io_event event;

    /*
      check that the SSL thread's error queue is cleared. Otherwise
      SSL-handshake-function returns an error from the error queue, when the
      function failed because it would block.
    */
    DBUG_ASSERT(ERR_peek_error() == 0);

    int handshake_ret;
    handshake_ret = func(ssl);

    if (handshake_ret >= 1) {
      ret = 0;
      break;
    }

    /* Process the SSL I/O error. */
    if (!ssl_should_retry(vio, handshake_ret, &event, ssl_errno_holder)) break;

    if (!vio->is_blocking_flag) {
      switch (event) {
        case VIO_IO_EVENT_READ:
          return VIO_SOCKET_WANT_READ;
        case VIO_IO_EVENT_WRITE:
          return VIO_SOCKET_WANT_WRITE;
        default:
          return VIO_SOCKET_ERROR;
      }
    }

    /* Wait for I/O so that the handshake can proceed. */
    if (vio_socket_io_wait(vio, event)) {
      if (vio_was_timeout(vio)) {
        return event == VIO_IO_EVENT_READ ? VIO_SOCKET_READ_TIMEOUT
                                          : VIO_SOCKET_WRITE_TIMEOUT;
      } else {
        return VIO_SOCKET_ERROR;
      }
    }
  }

  vio->ssl_arg = nullptr;

  return ret;
}

#if defined(HAVE_PSI_SOCKET_INTERFACE) && !defined(OPENSSL_IS_BORINGSSL)
long pfs_ssl_bio_callback_ex(BIO *b, int oper, const char * /* argp */,
                             size_t len, int /* argi */, long /* argl */,
                             int ret, size_t *processed) {
  Vio *vio;
  static const char *method_name = "open_ssl::bio::socket";

  /*
    Note:
    Normally, typical instrumentation for the performance schema would
    - define a local PSI_socket_locker_state variable
    - define a local PSI_socket_locker pointer
    - call PSI_SOCKET_CALL(start_socket_wait)
    - perform the operation
    - call PSI_SOCKET_CALL(end_socket_wait)

    Now, because this is done in a function callback attached to SSL itself,
    invocation of the start and end event are done in different invocations,
    so some state needs to be preserved between:
    - BIO_CB_READ and BIO_CB_READ|BIO_CB_RETURN,
    - BIO_CB_WRITE and BIO_CB_WRITE|BIO_CB_RETURN

    This state is preserved in attributes of vio:
    - m_psi_read_state and m_psi_read_locker for BIO_CB_READ
    - m_psi_write_state and m_psi_write_locker for BIO_CB_WRITE

    Raw socket operations are not supposed to be re-entrant,
    so we assert here that:
    - there is not a current read operation when performing a read
    - there is not a current write operation when performing a write.
    for extra safety.
  */
  switch (oper) {
    case BIO_CB_READ:
      vio = reinterpret_cast<Vio *>(BIO_get_callback_arg(b));
      DBUG_ASSERT(vio->m_psi_read_locker == nullptr);
      if (vio->mysql_socket.m_psi != nullptr) {
        vio->m_psi_read_locker = PSI_SOCKET_CALL(start_socket_wait)(
            &vio->m_psi_read_state, vio->mysql_socket.m_psi, PSI_SOCKET_RECV,
            len, method_name, oper);
      }
      break;
    case BIO_CB_READ | BIO_CB_RETURN:
      vio = reinterpret_cast<Vio *>(BIO_get_callback_arg(b));
      if (vio->m_psi_read_locker != nullptr) {
        PSI_SOCKET_CALL(end_socket_wait)(vio->m_psi_read_locker, *processed);
        vio->m_psi_read_locker = nullptr;
      }
      break;
    case BIO_CB_WRITE:
      vio = reinterpret_cast<Vio *>(BIO_get_callback_arg(b));
      DBUG_ASSERT(vio->m_psi_write_locker == nullptr);
      if (vio->mysql_socket.m_psi != nullptr) {
        vio->m_psi_write_locker = PSI_SOCKET_CALL(start_socket_wait)(
            &vio->m_psi_write_state, vio->mysql_socket.m_psi, PSI_SOCKET_SEND,
            len, method_name, oper);
      }
      break;
    case BIO_CB_WRITE | BIO_CB_RETURN:
      vio = reinterpret_cast<Vio *>(BIO_get_callback_arg(b));
      if (vio->m_psi_write_locker != nullptr) {
        PSI_SOCKET_CALL(end_socket_wait)(vio->m_psi_write_locker, *processed);
        vio->m_psi_write_locker = nullptr;
      }
      break;
    case BIO_CB_CTRL:
    case BIO_CB_CTRL | BIO_CB_RETURN:
    case BIO_CB_FREE:
    case BIO_CB_FREE | BIO_CB_RETURN:
      break;
    case BIO_CB_PUTS:
    case BIO_CB_PUTS | BIO_CB_RETURN:
    case BIO_CB_GETS:
    case BIO_CB_GETS | BIO_CB_RETURN:
    default:
      DBUG_ASSERT(false);
  }

  return ret;
}
#endif /* HAVE_PSI_SOCKET_INTERFACE */

#if defined(HAVE_PSI_SOCKET_INTERFACE) && !defined(OPENSSL_IS_BORINGSSL)
#ifndef HAVE_BIO_SET_CALLBACK_EX
/**
  Forward openSSL old style callback to openSSL 1.1.1 style callback.
*/
long pfs_ssl_bio_callback(BIO *b, int oper, const char *argp, int argi,
                          long argl, long ret) {
  size_t len = argi;
  /*
    For pre events:
    - irrelevant (not used in pfs_ssl_bio_callback_ex)
    For post (BIO_CB_RETURN) events,
    the number of bytes:
    - actually read, per the return value of recv()
    - actually written, per the return value of send()
  */
  size_t processed = (ret >= 0) ? ret : 0;

  return pfs_ssl_bio_callback_ex(b, oper, argp, len, argi, argl, ret,
                                 &processed);
}
#endif /* HAVE_BIO_SET_CALLBACK_EX */
#endif /* HAVE_PSI_SOCKET_INTERFACE */

#if defined(HAVE_PSI_SOCKET_INTERFACE) && !defined(OPENSSL_IS_BORINGSSL)
static void pfs_ssl_setup_instrumentation(Vio *vio, const SSL *ssl) {
  BIO *rbio = SSL_get_rbio(ssl);
  DBUG_ASSERT(rbio != nullptr);
  DBUG_ASSERT(BIO_method_type(rbio) == BIO_TYPE_SOCKET);

  BIO *wbio = SSL_get_wbio(ssl);
  DBUG_ASSERT(wbio != nullptr);
  DBUG_ASSERT(BIO_method_type(wbio) == BIO_TYPE_SOCKET);

  char *cb_arg = reinterpret_cast<char *>(vio);
  DBUG_ASSERT(cb_arg != nullptr);

  BIO_set_callback_arg(rbio, cb_arg);

#ifdef HAVE_BIO_SET_CALLBACK_EX
  BIO_set_callback_ex(rbio, pfs_ssl_bio_callback_ex);
#else
  BIO_set_callback(rbio, pfs_ssl_bio_callback);
#endif

  if (rbio != wbio) {
    BIO_set_callback_arg(wbio, cb_arg);

#ifdef HAVE_BIO_SET_CALLBACK_EX
    BIO_set_callback_ex(wbio, pfs_ssl_bio_callback_ex);
#else
    BIO_set_callback(wbio, pfs_ssl_bio_callback);
#endif
  }
}
#endif /* HAVE_PSI_SOCKET_INTERFACE */

static int ssl_do(struct st_VioSSLFd *ptr, Vio *vio, long timeout,
                  SSL_SESSION *ssl_session, ssl_handshake_func_t func,
                  unsigned long *ssl_errno_holder, SSL **sslptr,
                  const char *sni_servername,
                  const cert_validator_ptr validator_callback,
                  void *validator_callback_context,
                  int validator_context_index) {
  SSL *ssl = nullptr;
  my_socket sd = mysql_socket_getfd(vio->mysql_socket);

  /* Declared here to make compiler happy */
#if !defined(DBUG_OFF)
  int j, n;
#endif

  DBUG_TRACE;
  DBUG_PRINT("enter", ("ptr: %p, sd: %d  ctx: %p", ptr, sd, ptr->ssl_context));

  if (!sslptr) {
    sslptr = &ssl;
  }

  if (*sslptr == nullptr) {
    if (!(ssl = SSL_new(ptr->ssl_context))) {
      DBUG_PRINT("error", ("SSL_new failure"));
      *ssl_errno_holder = ERR_get_error();
      return 1;
    }

    if (ssl_session != nullptr) {
      if (!SSL_set_session(ssl, ssl_session)) {
#ifndef DBUG_OFF
        DBUG_PRINT("error", ("SSL_set_session failed"));
        report_errors(ssl);
#endif
        ERR_clear_error();
      } else {
        DBUG_PRINT("info", ("reused existing session %p", ssl_session));
      }
    }

// Check if the openssl supports SNI, and only then set the tlsext_host_name
#if OPENSSL_VERSION_NUMBER >= 0x00908070L && !defined(OPENSSL_NO_TLSEXT)
    if (sni_servername) {
      if (!SSL_set_tlsext_host_name(ssl, const_cast<char *>(sni_servername))) {
        DBUG_PRINT("error", ("SSL_set_tlsext_host_name failure"));
        *ssl_errno_holder = ERR_get_error();
        return 1;
      }
    }
#endif /* OPENSSL_VERSION_NUMBER >= 0x00908070 && !def(OPENSSL_NO_TLSEXT) */

    if (validator_callback && validator_context_index != -1) {
      SSL_set_verify(ssl, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE,
                     validator_callback);
      if (validator_callback_context) {
        SSL_set_ex_data(ssl, validator_context_index,
                        validator_callback_context);
      }
    }

    DBUG_PRINT("info", ("ssl: %p timeout: %ld", ssl, timeout));
    SSL_SESSION_set_timeout(SSL_get_session(ssl), timeout);
    SSL_set_fd(ssl, sd);
#if defined(SSL_OP_NO_COMPRESSION)
    SSL_set_options(ssl, SSL_OP_NO_COMPRESSION); /* OpenSSL >= 1.0 only */
#elif OPENSSL_VERSION_NUMBER >= 0x00908000L /* workaround for OpenSSL 0.9.8 */
    sk_SSL_COMP_zero(SSL_COMP_get_compression_methods());
#endif

#if !defined(DBUG_OFF)
    {
      STACK_OF(SSL_COMP) *ssl_comp_methods = nullptr;
      ssl_comp_methods = SSL_COMP_get_compression_methods();
      n = sk_SSL_COMP_num(ssl_comp_methods);
      DBUG_PRINT("info", ("Available compression methods:\n"));
      if (n == 0)
        DBUG_PRINT("info", ("NONE\n"));
      else
        for (j = 0; j < n; j++) {
          SSL_COMP *c = sk_SSL_COMP_value(ssl_comp_methods, j);
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(OPENSSL_IS_BORINGSSL)
          DBUG_PRINT("info", ("  %d: %s\n", c->id, c->name));
#else  /* OPENSSL_VERSION_NUMBER < 0x10100000L */
          DBUG_PRINT("info",
                     ("  %d: %s\n", SSL_COMP_get_id(c), SSL_COMP_get0_name(c)));
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
        }
    }
#endif

    *sslptr = ssl;

#if defined(HAVE_PSI_SOCKET_INTERFACE) && !defined(OPENSSL_IS_BORINGSSL)
    pfs_ssl_setup_instrumentation(vio, ssl);
#endif /* HAVE_PSI_SOCKET_INTERFACE */

  } else {
    ssl = *sslptr;
  }

  ssize_t loop_ret;
  if ((loop_ret = ssl_handshake_loop(vio, ssl, func, ssl_errno_holder))) {
    if (loop_ret == VIO_SOCKET_WANT_READ || loop_ret == VIO_SOCKET_WANT_WRITE) {
      return (int)loop_ret;  // Don't free SSL
    }

    DBUG_PRINT("error", ("SSL_connect/accept failure"));
    SSL_free(ssl);
    *sslptr = nullptr;
    return (int)loop_ret;
  }

  /*
    Connection succeeded. Install new function handlers,
    change type, set sd to the fd used when connecting
    and set pointer to the SSL structure
  */
  struct sockaddr_storage remote = vio->remote;
  if (vio_reset(vio, VIO_TYPE_SSL, SSL_get_fd(ssl), ssl, 0)) return 1;
  vio->remote = remote;
  if (sslptr != &ssl) {
    *sslptr = nullptr;
  }

#ifndef DBUG_OFF
  {
    /* Print some info about the peer */
    X509 *cert;
    char buf[512];

    DBUG_PRINT("info", ("SSL connection succeeded"));
    DBUG_PRINT("info", ("Using cipher: '%s'", SSL_get_cipher_name(ssl)));

    if ((cert = SSL_get_peer_certificate(ssl))) {
      DBUG_PRINT("info", ("Peer certificate:"));
      X509_NAME_oneline(X509_get_subject_name(cert), buf, sizeof(buf));
      DBUG_PRINT("info", ("\t subject: '%s'", buf));
      X509_NAME_oneline(X509_get_issuer_name(cert), buf, sizeof(buf));
      DBUG_PRINT("info", ("\t issuer: '%s'", buf));
      X509_free(cert);
    } else
      DBUG_PRINT("info", ("Peer does not have certificate."));

    if (SSL_get_shared_ciphers(ssl, buf, sizeof(buf))) {
      DBUG_PRINT("info", ("shared_ciphers: '%s'", buf));
    } else
      DBUG_PRINT("info", ("no shared ciphers!"));
  }
#endif

  return 0;
}

int sslaccept(struct st_VioSSLFd *ptr, Vio *vio, long timeout,
              unsigned long *ssl_errno_holder) {
  DBUG_TRACE;
  vio_set_blocking(vio, false);
  int ret = ssl_do(ptr, vio, timeout, nullptr, SSL_accept, ssl_errno_holder,
                   nullptr, nullptr, nullptr, nullptr, 0);
  return ret;
}

int sslconnect(struct st_VioSSLFd *ptr, Vio *vio, long timeout,
               SSL_SESSION *ssl_session, unsigned long *ssl_errno_holder,
               SSL **ssl, const char *sni_servername,
               const cert_validator_ptr validator_callback,
               void *validator_callback_context, int validator_context_index) {
  DBUG_TRACE;
  // Setting non blocking mode since openssl/boringssl does not support
  // timeout with blocking mode on handshake
  vio_set_blocking(vio, false);

  int ret = ssl_do(ptr, vio, timeout, ssl_session, SSL_connect,
                   ssl_errno_holder, ssl, sni_servername, validator_callback,
                   validator_callback_context, validator_context_index);
  return ret;
}

bool vio_ssl_has_data(Vio *vio) {
  return SSL_pending(static_cast<SSL *>(vio->ssl_arg)) > 0 ? true : false;
}
