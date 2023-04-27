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

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file include/violite.h
  Vio Lite.
  Purpose: include file for Vio that will work with C and C++.
*/

#ifndef vio_violite_h_
#define vio_violite_h_

#include "my_config.h"

#include <stddef.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <sys/types.h>

#include <string>

#include "my_inttypes.h"
#include "my_psi_config.h"  // IWYU pragma: keep
#include "mysql/components/services/my_io_bits.h"
#include "mysql/components/services/my_thread_bits.h"
#include "mysql/components/services/mysql_socket_bits.h"
#include "mysql_com.h"

#include "mysql/psi/mysql_socket.h"

struct Vio;

/* Simple vio interface in C;  The functions are implemented in violite.c */

#if !defined(_WIN32) && !defined(HAVE_KQUEUE) && !defined(__SUNPRO_CC)
#define USE_PPOLL_IN_VIO
#endif

#if defined(__cplusplus) && defined(USE_PPOLL_IN_VIO)
#include <signal.h>
#include <atomic>
#elif defined(__cplusplus) && defined(HAVE_KQUEUE)
#include <sys/event.h>
#include <atomic>
#endif

#ifdef HAVE_PSI_INTERFACE
void init_vio_psi_keys();
#endif

#ifndef MYSQL_VIO
struct Vio;

typedef Vio Vio;
#define MYSQL_VIO Vio *
#endif

enum enum_vio_type : int {
  /**
    Type of the connection is unknown.
  */
  NO_VIO_TYPE = 0,
  /**
    Used in case of TCP/IP connections.
  */
  VIO_TYPE_TCPIP = 1,
  /**
    Used for Unix Domain socket connections. Unix only.
  */
  VIO_TYPE_SOCKET = 2,
  /**
    Used for named pipe connections. Windows only.
  */
  VIO_TYPE_NAMEDPIPE = 3,
  /**
    Used in case of SSL connections.
  */
  VIO_TYPE_SSL = 4,
  /**
    Used for shared memory connections. Windows only.
  */
  VIO_TYPE_SHARED_MEMORY = 5,
  /**
    Used internally by the prepared statements
  */
  VIO_TYPE_LOCAL = 6,
  /**
    Implicitly used by plugins that doesn't support any other VIO_TYPE.
  */
  VIO_TYPE_PLUGIN = 7,

  FIRST_VIO_TYPE = VIO_TYPE_TCPIP,
  /*
    If a new type is added, please update LAST_VIO_TYPE. In addition, please
    change get_vio_type_name() in vio/vio.c to return correct name for it.
  */
  LAST_VIO_TYPE = VIO_TYPE_PLUGIN
};

/**
  Convert a vio type to a printable string.
  @param vio_type the type
  @param[out] str the string
  @param[out] len the string length
*/
void get_vio_type_name(enum enum_vio_type vio_type, const char **str, int *len);

/**
  VIO I/O events.
*/
enum enum_vio_io_event {
  VIO_IO_EVENT_READ,
  VIO_IO_EVENT_WRITE,
  VIO_IO_EVENT_CONNECT
};

#define VIO_SOCKET_ERROR ((ssize_t)-1)
#define VIO_SOCKET_WANT_READ ((ssize_t)-2)
#define VIO_SOCKET_WANT_WRITE ((ssize_t)-3)
#define VIO_SOCKET_READ_TIMEOUT ((ssize_t)-4)
#define VIO_SOCKET_WRITE_TIMEOUT ((ssize_t)-5)

#define VIO_LOCALHOST 1            /* a localhost connection */
#define VIO_BUFFERED_READ 2        /* use buffered read */
#define VIO_READ_BUFFER_SIZE 16384 /* size of read buffer */
#define OPENSSL_ERROR_LENGTH 512   /* Openssl error code max length */

MYSQL_VIO vio_new(my_socket sd, enum enum_vio_type type, uint flags);
MYSQL_VIO mysql_socket_vio_new(MYSQL_SOCKET mysql_socket,
                               enum enum_vio_type type, uint flags);

#ifdef _WIN32
MYSQL_VIO vio_new_win32pipe(HANDLE hPipe);
MYSQL_VIO vio_new_win32shared_memory(HANDLE handle_file_map, HANDLE handle_map,
                                     HANDLE event_server_wrote,
                                     HANDLE event_server_read,
                                     HANDLE event_client_wrote,
                                     HANDLE event_client_read,
                                     HANDLE event_conn_closed);
#else
#define HANDLE void *
#endif /* _WIN32 */

void vio_delete(MYSQL_VIO vio);
int vio_shutdown(MYSQL_VIO vio);
bool vio_reset(MYSQL_VIO vio, enum enum_vio_type type, my_socket sd, void *ssl,
               uint flags);
bool vio_is_blocking(Vio *vio);
int vio_set_blocking(Vio *vio, bool set_blocking_mode);
int vio_set_blocking_flag(Vio *vio, bool set_blocking_flag);
size_t vio_read(MYSQL_VIO vio, uchar *buf, size_t size);
size_t vio_read_buff(MYSQL_VIO vio, uchar *buf, size_t size);
size_t vio_write(MYSQL_VIO vio, const uchar *buf, size_t size);
/* setsockopt TCP_NODELAY at IPPROTO_TCP level, when possible */
int vio_fastsend(MYSQL_VIO vio);
/* setsockopt SO_KEEPALIVE at SOL_SOCKET level, when possible */
int vio_keepalive(MYSQL_VIO vio, bool onoff);
/* Whenever we should retry the last read/write operation. */
bool vio_should_retry(MYSQL_VIO vio);
/* Check that operation was timed out */
bool vio_was_timeout(MYSQL_VIO vio);
#ifndef DBUG_OFF
/* Short text description of the socket for those, who are curious.. */
#define VIO_DESCRIPTION_SIZE 30 /* size of description */
void vio_description(MYSQL_VIO vio, char *buf);
#endif  // DBUG_OFF
/* Return the type of the connection */
enum enum_vio_type vio_type(const MYSQL_VIO vio);
/* Return last error number */
int vio_errno(MYSQL_VIO vio);
/* Get socket number */
my_socket vio_fd(MYSQL_VIO vio);
/* Remote peer's address and name in text form */
bool vio_peer_addr(MYSQL_VIO vio, char *buf, uint16 *port, size_t buflen);
/* Wait for an I/O event notification. */
int vio_io_wait(MYSQL_VIO vio, enum enum_vio_io_event event, timeout_t timeout);
bool vio_is_connected(MYSQL_VIO vio);
#ifndef DBUG_OFF
ssize_t vio_pending(MYSQL_VIO vio);
#endif
/* Set timeout for a network operation. */
int vio_timeout(MYSQL_VIO vio, uint which, timeout_t timeout);
/* Connect to a peer. */
bool vio_socket_connect(MYSQL_VIO vio, struct sockaddr *addr, socklen_t len,
                        bool nonblocking, timeout_t timeout,
                        bool *connect_done = nullptr);

void vio_get_normalized_ip(const struct sockaddr *src, size_t src_length,
                           struct sockaddr *dst, size_t *dst_length);

bool vio_get_normalized_ip_string(const struct sockaddr *addr,
                                  size_t addr_length, char *ip_string,
                                  size_t ip_string_size);

bool vio_is_no_name_error(int err_code);

int vio_getnameinfo(const struct sockaddr *sa, char *hostname,
                    size_t hostname_size, char *port, size_t port_size,
                    int flags);

extern "C" {
#include <openssl/opensslv.h>
}
#if OPENSSL_VERSION_NUMBER < 0x0090700f
#define DES_cblock des_cblock
#define DES_key_schedule des_key_schedule
#define DES_set_key_unchecked(k, ks) des_set_key_unchecked((k), *(ks))
#define DES_ede3_cbc_encrypt(i, o, l, k1, k2, k3, iv, e) \
  des_ede3_cbc_encrypt((i), (o), (l), *(k1), *(k2), *(k3), (iv), (e))
#endif

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define HAVE_OPENSSL11 1
#endif  // OPENSSL_VERSION_NUMBER

#define HEADER_DES_LOCL_H dummy_something

#include <openssl/err.h>
#include <openssl/ssl.h>

enum enum_ssl_init_error {
  SSL_INITERR_NOERROR = 0,
  SSL_INITERR_CERT,
  SSL_INITERR_KEY,
  SSL_INITERR_NOMATCH,
  SSL_INITERR_BAD_PATHS,
  SSL_INITERR_CIPHERS,
  SSL_INITERR_MEMFAIL,
  SSL_INITERR_NO_USABLE_CTX,
  SSL_INITERR_DHFAIL,
  SSL_TLS_VERSION_INVALID,
  SSL_FIPS_MODE_INVALID,
  SSL_FIPS_MODE_FAILED,
  SSL_INITERR_ECDHFAIL,
  SSL_INITERR_LASTERR
};
const char *sslGetErrString(enum enum_ssl_init_error err);

struct st_VioSSLFd {
  SSL_CTX *ssl_context;
  /*
    "owned" indicates whether SSL_CTX_free should be called on this instance
    or not. If mysql_take_ssl_context_ownership is called, the context
    becomes owned by the application and not by the MySQL client library (ie,
    owned becomes false).  Likewise, if a context is supplied by
    mysql_options(..., MYSQL_OPT_SSL_CONTEXT, ...) then it is also not owned
    by the client library.
  */
  bool owned;
};

typedef int (*cert_validator_ptr)(int preverify_ok, X509_STORE_CTX *x509_ctx);

int sslaccept(struct st_VioSSLFd *, MYSQL_VIO, long timeout,
              unsigned long *errptr);
int sslconnect(struct st_VioSSLFd *, MYSQL_VIO, long timeout,
               SSL_SESSION *ssl_session, unsigned long *errptr, SSL **ssl,
               const char *sni_servername = nullptr,
               const cert_validator_ptr validator_callback = nullptr,
               void *validator_callback_context = nullptr,
               int validator_context_index = -1);

struct st_VioSSLFd *new_VioSSLConnectorFdFromContext(
    SSL_CTX *context, enum enum_ssl_init_error *error);
struct st_VioSSLFd *new_VioSSLConnectorFd(
    const char *key_file, const char *cert_file, const char *ca_file,
    const char *ca_path, const char *cipher, const char *ciphersuites,
    enum enum_ssl_init_error *error, const char *crl_file, const char *crl_path,
    const long ssl_ctx_flags);

long process_tls_version(const char *tls_version);

#if !defined(OPENSSL_IS_BORINGSSL)
int set_fips_mode(const uint fips_mode, char *err_string);
#endif

uint get_fips_mode();

struct st_VioSSLFd *new_VioSSLAcceptorFd(
    const char *key_file, const char *cert_file, const char *ca_file,
    const char *ca_path, const char *cipher, const char *ciphersuites,
    enum enum_ssl_init_error *error, const char *crl_file, const char *crl_path,
    const long ssl_ctx_flags);
void free_vio_ssl_fd(struct st_VioSSLFd *fd);

void vio_ssl_end();

void ssl_start(void);
void vio_end(void);

#if !defined(DONT_MAP_VIO)
#define vio_delete(vio) (vio)->viodelete(vio)
#define vio_errno(vio) (vio)->vioerrno(vio)
#define vio_read(vio, buf, size) ((vio)->read)(vio, buf, size)
#define vio_write(vio, buf, size) ((vio)->write)(vio, buf, size)
#define vio_fastsend(vio) (vio)->fastsend(vio)
#define vio_keepalive(vio, set_keep_alive) \
  (vio)->viokeepalive(vio, set_keep_alive)
#define vio_should_retry(vio) (vio)->should_retry(vio)
#define vio_was_timeout(vio) (vio)->was_timeout(vio)
#define vio_shutdown(vio) ((vio)->vioshutdown)(vio)
#define vio_peer_addr(vio, buf, prt, buflen) \
  (vio)->peer_addr(vio, buf, prt, buflen)
#define vio_io_wait(vio, event, timeout) (vio)->io_wait(vio, event, timeout)
#define vio_is_connected(vio) (vio)->is_connected(vio)
#define vio_is_blocking(vio) (vio)->is_blocking(vio)
#define vio_set_blocking(vio, val) (vio)->set_blocking(vio, val)
#define vio_set_blocking_flag(vio, val) (vio)->set_blocking_flag(vio, val)
#endif /* !defined(DONT_MAP_VIO) */

/* This enumerator is used in parser - should be always visible */
enum SSL_type {
  SSL_TYPE_NOT_SPECIFIED = -1,
  SSL_TYPE_NONE,
  SSL_TYPE_ANY,
  SSL_TYPE_X509,
  SSL_TYPE_SPECIFIED
};

/*
 This structure is for every connection on both sides.
 Note that it has a non-default move assignment operator, so if adding more
 members, you'll need to update operator=.
*/
struct Vio {
  MYSQL_SOCKET mysql_socket;          /* Instrumented socket */
  bool localhost = {false};           /* Are we from localhost? */
  enum_vio_type type = {NO_VIO_TYPE}; /* Type of connection */

  timeout_t read_timeout = {UINT_MAX};  /* Timeout value (ms) for read ops. */
  timeout_t write_timeout = {UINT_MAX}; /* Timeout value (ms) for write ops. */
  int retry_count = {1};                /* Retry count */
  bool inactive = {false};              /* Connection has been shutdown */

  struct sockaddr_storage local;  /* Local internet address */
  struct sockaddr_storage remote; /* Remote internet address */
  size_t addrLen = {0};           /* Length of remote address */
  char *read_buffer = {nullptr};  /* buffer for vio_read_buff */
  char *read_pos = {nullptr};     /* start of unfetched data in the
                                     read buffer */
  char *read_end = {nullptr};     /* end of unfetched data */

#ifdef USE_PPOLL_IN_VIO
  my_thread_t thread_id = {0};  // Thread PID
  sigset_t signal_mask;         // Signal mask
  /*
    Flag to indicate whether we are in poll or shutdown.
    A true value of flag indicates either the socket
    has called  shutdown or it is sleeping on a poll call.
    False value of this flag means that the socket is
    not sleeping on a poll call.
    This flag provides synchronization between two threads
    one entering vio_io_wait and another entering vio_shutdown
    for the same socket. If the other thread is waiting on poll
    sleep, it wakes up the thread by sending a signal via
    pthread_kill. Also it ensures that no other thread enters in
    to a poll call if it's socket has undergone shutdown.

  */
  std::atomic_flag poll_shutdown_flag = ATOMIC_FLAG_INIT;
#elif defined HAVE_KQUEUE
  int kq_fd = {-1};
  std::atomic_flag kevent_wakeup_flag = ATOMIC_FLAG_INIT;
#endif
  const char *timeout_err_msg = {nullptr}; /* Timeout error message. */

#ifdef HAVE_SETNS
  /**
    Socket network namespace.
  */
  char network_namespace[256];
#endif
  /*
     VIO vtable interface to be implemented by VIO's like SSL, Socket,
     Named Pipe, etc.
  */

  /*
     viodelete is responsible for cleaning up the VIO object by freeing
     internal buffers, closing descriptors, handles.
  */
  void (*viodelete)(MYSQL_VIO) = {nullptr};
  int (*vioerrno)(MYSQL_VIO) = {nullptr};
  size_t (*read)(MYSQL_VIO, uchar *, size_t) = {nullptr};
  size_t (*write)(MYSQL_VIO, const uchar *, size_t) = {nullptr};
  int (*timeout)(MYSQL_VIO, uint, bool) = {nullptr};
  int (*viokeepalive)(MYSQL_VIO, bool) = {nullptr};
  int (*fastsend)(MYSQL_VIO) = {nullptr};
  bool (*peer_addr)(MYSQL_VIO, char *, uint16 *, size_t) = {nullptr};
  void (*in_addr)(MYSQL_VIO, struct sockaddr_storage *) = {nullptr};
  bool (*should_retry)(MYSQL_VIO) = {nullptr};
  bool (*was_timeout)(MYSQL_VIO) = {nullptr};
  /*
     vioshutdown is resposnible to shutdown/close the channel, so that no
     further communications can take place, however any related buffers,
     descriptors, handles can remain valid after a shutdown.
  */
  int (*vioshutdown)(MYSQL_VIO) = {nullptr};
  bool (*is_connected)(MYSQL_VIO) = {nullptr};
  bool (*has_data)(MYSQL_VIO) = {nullptr};
  int (*io_wait)(MYSQL_VIO, enum enum_vio_io_event, timeout_t) = {nullptr};
  bool (*connect)(MYSQL_VIO, struct sockaddr *, socklen_t, int) = {nullptr};
#ifdef _WIN32
#ifdef __clang__
  OVERLAPPED overlapped = {0, 0, {{0, 0}}, nullptr};
#else
  // MSVC, at least up to 2015, gives an internal error on the above.
  OVERLAPPED overlapped = {0};
#endif
  HANDLE hPipe{nullptr};
#endif
  void *ssl_arg = {nullptr};
  struct PSI_socket_locker *m_psi_read_locker = {nullptr};
  PSI_socket_locker_state m_psi_read_state;
  struct PSI_socket_locker *m_psi_write_locker = {nullptr};
  PSI_socket_locker_state m_psi_write_state;
#if defined(_WIN32)
  HANDLE handle_file_map = {nullptr};
  char *handle_map = {nullptr};
  HANDLE event_server_wrote = {nullptr};
  HANDLE event_server_read = {nullptr};
  HANDLE event_client_wrote = {nullptr};
  HANDLE event_client_read = {nullptr};
  HANDLE event_conn_closed = {nullptr};
  size_t shared_memory_remain = {0};
  char *shared_memory_pos = {nullptr};

#endif /* _WIN32 */
  bool (*is_blocking)(Vio *vio) = {nullptr};
  int (*set_blocking)(Vio *vio, bool val) = {nullptr};
  int (*set_blocking_flag)(Vio *vio, bool val) = {nullptr};
  /* Indicates whether socket or SSL based communication is blocking or not. */
  bool is_blocking_flag = {true};

 private:
  friend Vio *internal_vio_create(uint flags);
  friend void internal_vio_delete(Vio *vio);
  friend bool vio_reset(Vio *vio, enum_vio_type type, my_socket sd, void *ssl,
                        uint flags);

  explicit Vio(uint flags);
  ~Vio();

 public:
  Vio(const Vio &) = delete;
  Vio &operator=(const Vio &) = delete;
  Vio &operator=(Vio &&vio);
};

#define SSL_handle SSL *

#endif /* vio_violite_h_ */
