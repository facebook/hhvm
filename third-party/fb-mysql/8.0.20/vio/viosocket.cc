/*
   Copyright (c) 2001, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#ifndef _WIN32
#include <netdb.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "template_utils.h"
#include "vio/vio_priv.h"

#ifdef FIONREAD_IN_SYS_FILIO
#include <sys/filio.h>
#endif
#ifndef _WIN32
#include <netinet/tcp.h>
#endif
#ifdef HAVE_POLL_H
#include <poll.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#include "mysql/psi/mysql_socket.h"

int vio_errno(Vio *vio MY_ATTRIBUTE((unused))) {
/* These transport types are not Winsock based. */
#ifdef _WIN32
  if (vio->type == VIO_TYPE_NAMEDPIPE || vio->type == VIO_TYPE_SHARED_MEMORY)
    return GetLastError();
#endif

  /* Mapped to WSAGetLastError() on Win32. */
  return socket_errno;
}

/**
  Attempt to wait for an I/O event on a socket.

  @param vio      VIO object representing a connected socket.
  @param event    The type of I/O event (read or write) to wait for.

  @return Return value is -1 on failure, 0 on success.
*/

int vio_socket_io_wait(Vio *vio, enum enum_vio_io_event event) {
  timeout_t timeout;
  int ret;

  DBUG_ASSERT(event == VIO_IO_EVENT_READ || event == VIO_IO_EVENT_WRITE);

  /* Choose an appropriate timeout. */
  if (event == VIO_IO_EVENT_READ)
    timeout = vio->read_timeout;
  else
    timeout = vio->write_timeout;

  /* Wait for input data to become available. */
  switch (vio_io_wait(vio, event, timeout)) {
    case -1:
      /* Upon failure, vio_read/write() shall return -1. */
      ret = -1;
      break;
    case 0:
      /* The wait timed out. */
      ret = -1;
      break;
    default:
      /* A positive value indicates an I/O event. */
      ret = 0;
      break;
  }

  return ret;
}

/*
  Define a stub MSG_DONTWAIT if unavailable. In this case, fcntl
  (or a equivalent) is used to enable non-blocking operations.
  The flag must be supported in both send and recv operations.
*/
#if defined(__linux__)
#define VIO_USE_DONTWAIT 1
#define VIO_DONTWAIT MSG_DONTWAIT
#else
#define VIO_DONTWAIT 0
#endif

size_t vio_read(Vio *vio, uchar *buf, size_t size) {
  ssize_t ret;
  int flags = 0;
  DBUG_TRACE;

  /* Ensure nobody uses vio_read_buff and vio_read simultaneously. */
  DBUG_ASSERT(vio->read_end == vio->read_pos);

  /* If timeout is enabled, do not block if data is unavailable. */
  if (!timeout_is_infinite(vio->read_timeout)) flags = VIO_DONTWAIT;

  while ((ret = mysql_socket_recv(vio->mysql_socket, (SOCKBUF_T *)buf, size,
                                  flags)) == -1) {
    int error = socket_errno;

    /* Error encountered that is unrelated to blocking; percolate it up. */
#if SOCKET_EAGAIN == SOCKET_EWOULDBLOCK
    if (error != SOCKET_EAGAIN)
#else
    if (error != SOCKET_EAGAIN && error != SOCKET_EWOULDBLOCK)
#endif
      break;
    /*
      Nonblocking with either EAGAIN or EWOULDBLOCK. Don't call
      io_wait. 0 bytes are available.
    */
    DBUG_ASSERT(error == SOCKET_EAGAIN || error == SOCKET_EWOULDBLOCK);
    if (!vio_is_blocking(vio)) {
      DBUG_PRINT("info", ("vio_read on nonblocking socket read no bytes"));
      return -1;
    }

    /* Wait for input data to become available. */
    if ((ret = vio_socket_io_wait(vio, VIO_IO_EVENT_READ))) break;
  }
  return ret;
}

/*
  Buffered read: if average read size is small it may
  reduce number of syscalls.
*/

size_t vio_read_buff(Vio *vio, uchar *buf, size_t size) {
  size_t rc;
#define VIO_UNBUFFERED_READ_MIN_SIZE 2048
  DBUG_TRACE;
  DBUG_PRINT("enter", ("sd: %d  buf: %p  size: %u",
                       mysql_socket_getfd(vio->mysql_socket), buf, (uint)size));

  if (vio->read_pos < vio->read_end) {
    rc = std::min<size_t>(vio->read_end - vio->read_pos, size);
    memcpy(buf, vio->read_pos, rc);
    vio->read_pos += rc;
    /*
      Do not try to read from the socket now even if rc < size:
      vio_read can return -1 due to an error or non-blocking mode, and
      the safest way to handle it is to move to a separate branch.
    */
  } else if (size < VIO_UNBUFFERED_READ_MIN_SIZE) {
    rc = vio_read(vio, (uchar *)vio->read_buffer, VIO_READ_BUFFER_SIZE);
    if (rc != 0 && rc != (size_t)-1) {
      if (rc > size) {
        vio->read_pos = vio->read_buffer + size;
        vio->read_end = vio->read_buffer + rc;
        rc = size;
      }
      memcpy(buf, vio->read_buffer, rc);
    }
  } else
    rc = vio_read(vio, buf, size);
  return rc;
#undef VIO_UNBUFFERED_READ_MIN_SIZE
}

bool vio_buff_has_data(Vio *vio) { return (vio->read_pos != vio->read_end); }

size_t vio_write(Vio *vio, const uchar *buf, size_t size) {
  ssize_t ret;
  int flags = 0;
  DBUG_TRACE;

  /* If timeout is enabled, do not block. */
  if (!timeout_is_infinite(vio->write_timeout)) flags = VIO_DONTWAIT;

  while ((ret = mysql_socket_send(vio->mysql_socket,
                                  pointer_cast<const SOCKBUF_T *>(buf), size,
                                  flags)) == -1) {
    int error = socket_errno;

    /* The operation would block? */
#if SOCKET_EAGAIN == SOCKET_EWOULDBLOCK
    if (error != SOCKET_EAGAIN)
#else
    if (error != SOCKET_EAGAIN && error != SOCKET_EWOULDBLOCK)
#endif
      break;

    if (!vio_is_blocking(vio)) {
      DBUG_PRINT("info", ("vio_write on nonblocking socket written no bytes"));
      return -1;
    }

    /* Wait for the output buffer to become writable.*/
    if ((ret = vio_socket_io_wait(vio, VIO_IO_EVENT_WRITE))) break;
  }

  return ret;
}

// WL#4896: Not covered
int vio_set_blocking(Vio *vio, bool status) {
  DBUG_TRACE;

#ifdef _WIN32

  {
    int ret;
    u_long arg = status ? 0 : 1;
    ret = ioctlsocket(mysql_socket_getfd(vio->mysql_socket), FIONBIO, &arg);
    return ret;
  }
#else
  {
    int flags;

    if ((flags = fcntl(mysql_socket_getfd(vio->mysql_socket), F_GETFL, NULL)) <
        0)
      return -1;

    /*
      Always set/clear the flag to avoid inheritance issues. This is
      a issue mainly on Mac OS X Tiger (version 10.4) where although
      the O_NONBLOCK flag is inherited from the parent socket, the
      actual non-blocking behavior is not inherited.
    */
    if (status)
      flags &= ~O_NONBLOCK;
    else
      flags |= O_NONBLOCK;

    if (fcntl(mysql_socket_getfd(vio->mysql_socket), F_SETFL, flags) == -1)
      return -1;
  }
#endif

  return 0;
}

int vio_set_blocking_flag(Vio *vio, bool status) {
  DBUG_TRACE;
  int ret = 0;
  /*
    Asynchronous communication in client is allowed only for below
    types of connections.
  */
  if (VIO_TYPE_TCPIP == vio->type || VIO_TYPE_SOCKET == vio->type ||
      VIO_TYPE_SSL == vio->type) {
    vio->is_blocking_flag = status;
    ret = vio_set_blocking(vio, status);
  }
  return ret;
}

bool vio_is_blocking(Vio *vio) {
  DBUG_TRACE;
  return vio->is_blocking_flag;
}

int vio_socket_timeout(Vio *vio, uint which MY_ATTRIBUTE((unused)),
                       bool old_mode) {
  int ret = 0;
  DBUG_TRACE;

#if defined(_WIN32)
  {
    int optname;
    DWORD timeout = 0;
    const char *optval = (const char *)&timeout;

    /*
      The default socket timeout value is zero, which means an infinite
      timeout. Values less than 500 milliseconds are interpreted to be of
      500 milliseconds. Hence, the VIO behavior for zero timeout, which is
      intended to cause the send or receive operation to fail immediately
      if no data is available, is not supported on WIN32 and neither is
      necessary as it's not possible to set the VIO timeout value to zero.

      Assert that the VIO timeout is either positive or set to infinite.
    */
    DBUG_ASSERT(which || vio->read_timeout);
    DBUG_ASSERT(!which || vio->write_timeout);

    if (which) {
      optname = SO_SNDTIMEO;
      if (vio->write_timeout > 0) timeout = vio->write_timeout;
    } else {
      optname = SO_RCVTIMEO;
      if (vio->read_timeout > 0) timeout = vio->read_timeout;
    }

    ret = mysql_socket_setsockopt(vio->mysql_socket, SOL_SOCKET, optname,
                                  optval, sizeof(timeout));
  }
#else
  /*
    The MSG_DONTWAIT trick is not used with SSL sockets as the send and
    receive I/O operations are wrapped through SSL-specific functions
    (SSL_read and SSL_write) which are not equivalent to the standard
    recv(2) and send(2) used in vio_read() and vio_write(). Hence, the
    socket blocking mode is changed and vio_io_wait() is used to wait
    for I/O or timeout.
  */
#ifdef VIO_USE_DONTWAIT
  if (vio->type == VIO_TYPE_SSL)
#endif
  {
    /* Deduce what should be the new blocking mode of the socket. */
    bool new_mode = timeout_is_infinite(vio->write_timeout) &&
                    timeout_is_infinite(vio->read_timeout);

    /* If necessary, update the blocking mode. */
    if (new_mode != old_mode) ret = vio_set_blocking(vio, new_mode);
  }
#endif

  return ret;
}

int vio_fastsend(Vio *vio) {
  int r = 0;
  DBUG_TRACE;

#if defined(IPTOS_THROUGHPUT)
  {
    int tos = IPTOS_THROUGHPUT;
    r = mysql_socket_setsockopt(vio->mysql_socket, IPPROTO_IP, IP_TOS,
                                (void *)&tos, sizeof(tos));
  }
#endif /* IPTOS_THROUGHPUT */
  if (!r) {
#ifdef _WIN32
    BOOL nodelay = 1;
#else
    int nodelay = 1;
#endif

    r = mysql_socket_setsockopt(vio->mysql_socket, IPPROTO_TCP, TCP_NODELAY,
                                IF_WIN((const char *), (void *)) & nodelay,
                                sizeof(nodelay));
  }
  if (r) {
    DBUG_PRINT("warning", ("Couldn't set socket option for fast send"));
    r = -1;
  }
  DBUG_PRINT("exit", ("%d", r));
  return r;
}

int vio_keepalive(Vio *vio, bool set_keep_alive) {
  int r = 0;
  uint opt = 0;
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("sd: %d  set_keep_alive: %d",
              mysql_socket_getfd(vio->mysql_socket), (int)set_keep_alive));
  if (vio->type != VIO_TYPE_NAMEDPIPE) {
    if (set_keep_alive) opt = 1;
    r = mysql_socket_setsockopt(vio->mysql_socket, SOL_SOCKET, SO_KEEPALIVE,
                                (char *)&opt, sizeof(opt));
  }
  return r;
}

/**
  Indicate whether a I/O operation must be retried later.

  @param vio  A VIO object

  @return Whether a I/O operation should be deferred.
  @retval true    Temporary failure, retry operation.
  @retval false   Indeterminate failure.
*/

bool vio_should_retry(Vio *vio) { return (vio_errno(vio) == SOCKET_EINTR); }

/**
  Indicate whether a I/O operation timed out.

  @param vio  A VIO object

  @return Whether a I/O operation timed out.
  @retval true    Operation timed out.
  @retval false   Not a timeout failure.
*/

bool vio_was_timeout(Vio *vio) { return (vio_errno(vio) == SOCKET_ETIMEDOUT); }

#ifdef USE_PPOLL_IN_VIO
static void vio_wait_until_woken(Vio *vio) {
  while (vio->poll_shutdown_flag.test_and_set()) {
    // Wait until the vio is woken up from poll.
  }
}
#elif defined HAVE_KQUEUE
static const int WAKEUP_EVENT_ID = 0xFACEFEED;

static void vio_wait_until_woken(Vio *vio) {
  if (vio->kq_fd != -1) {
    struct kevent kev;

    EV_SET(&kev, WAKEUP_EVENT_ID, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
    int nev = kevent(vio->kq_fd, &kev, 1, nullptr, 0, nullptr);
    if (nev != -1) {
      while (vio->kevent_wakeup_flag.test_and_set()) {
        // Wait until the vio is woken up from kevent.
      }
    }
  }
}
#endif

int vio_shutdown(Vio *vio) {
  int r = 0;
  DBUG_TRACE;

  if (vio->inactive == false) {
    DBUG_ASSERT(vio->type == VIO_TYPE_TCPIP || vio->type == VIO_TYPE_SOCKET ||
                vio->type == VIO_TYPE_SSL);

    DBUG_ASSERT(mysql_socket_getfd(vio->mysql_socket) >= 0);
    if (mysql_socket_shutdown(vio->mysql_socket, SHUT_RDWR)) r = -1;

#ifdef USE_PPOLL_IN_VIO
    if (vio->thread_id != 0 && vio->poll_shutdown_flag.test_and_set()) {
      // Send signal to wake up from poll.
      if (pthread_kill(vio->thread_id, SIGALRM) == 0)
        vio_wait_until_woken(vio);
      else
        perror("Error in pthread_kill");
    }
#elif defined HAVE_KQUEUE
    if (vio->kq_fd != -1 && vio->kevent_wakeup_flag.test_and_set())
      vio_wait_until_woken(vio);
#endif

    if (mysql_socket_close(vio->mysql_socket)) r = -1;
#ifdef HAVE_KQUEUE
    if (vio->kq_fd == -1 || close(vio->kq_fd)) r = -1;
    vio->kq_fd = -1;
#endif
  }

  if (r) {
    DBUG_PRINT("vio_error", ("close() failed, error: %d", socket_errno));
    /* FIXME: error handling (not critical for MySQL) */
  }
  vio->inactive = true;
  vio->mysql_socket = MYSQL_INVALID_SOCKET;
  return r;
}

#ifndef DBUG_OFF
void vio_description(Vio *vio, char *buf) {
  switch (vio->type) {
    case VIO_TYPE_SOCKET:
      snprintf(buf, VIO_DESCRIPTION_SIZE, "socket (%d)",
               mysql_socket_getfd(vio->mysql_socket));
      break;
#ifdef _WIN32
    case VIO_TYPE_NAMEDPIPE:
      my_stpcpy(buf, "named pipe");
      break;
    case VIO_TYPE_SHARED_MEMORY:
      my_stpcpy(buf, "shared memory");
      break;
#endif
    default:
      snprintf(buf, VIO_DESCRIPTION_SIZE, "TCP/IP (%d)",
               mysql_socket_getfd(vio->mysql_socket));
      break;
  }
}
#endif  // DBUG_OFF

enum enum_vio_type vio_type(const Vio *vio) { return vio->type; }

my_socket vio_fd(Vio *vio) { return mysql_socket_getfd(vio->mysql_socket); }

/**
  Convert a sock-address (AF_INET or AF_INET6) into the "normalized" form,
  which is the IPv4 form for IPv4-mapped or IPv4-compatible IPv6 addresses.

  @note Background: when IPv4 and IPv6 are used simultaneously, IPv4
  addresses may be written in a form of IPv4-mapped or IPv4-compatible IPv6
  addresses. That means, one address (a.b.c.d) can be written in three forms:
    - IPv4: a.b.c.d;
    - IPv4-compatible IPv6: @code ::a.b.c.d @endcode;
    - IPv4-mapped IPv4: @code ::ffff:a.b.c.d @endcode;

  Having three forms of one address makes it a little difficult to compare
  addresses with each other (the IPv4-compatible IPv6-address of foo.bar
  will be different from the IPv4-mapped IPv6-address of foo.bar).

  @note This function can be made public when it's needed.

  @param [in] src         source IP address (AF_INET or AF_INET6).
  @param [in] src_length  length of the src.
  @param [out] dst        a buffer to store normalized IP address
                          (sockaddr_storage).
  @param [out] dst_length actual length of the normalized IP address.
*/
void vio_get_normalized_ip(const struct sockaddr *src, size_t src_length,
                           struct sockaddr *dst, size_t *dst_length) {
  switch (src->sa_family) {
    case AF_INET:
      memcpy(dst, src, src_length);
      *dst_length = src_length;
      break;

    case AF_INET6: {
      const struct sockaddr_in6 *src_addr6 = (const struct sockaddr_in6 *)src;
      const struct in6_addr *src_ip6 = &(src_addr6->sin6_addr);
      const uint32 *src_ip6_int32 =
          pointer_cast<const uint32 *>(src_ip6->s6_addr);

      if (IN6_IS_ADDR_V4MAPPED(src_ip6) || IN6_IS_ADDR_V4COMPAT(src_ip6)) {
        struct sockaddr_in *dst_ip4 = (struct sockaddr_in *)dst;

        /*
          This is an IPv4-mapped or IPv4-compatible IPv6 address. It should
          be converted to the IPv4 form.
        */

        *dst_length = sizeof(struct sockaddr_in);

        memset(dst_ip4, 0, *dst_length);
        dst_ip4->sin_family = AF_INET;
        dst_ip4->sin_port = src_addr6->sin6_port;

        /*
          In an IPv4 mapped or compatible address, the last 32 bits represent
          the IPv4 address. The byte orders for IPv6 and IPv4 addresses are
          the same, so a simple copy is possible.
        */
        dst_ip4->sin_addr.s_addr = src_ip6_int32[3];
      } else {
        /* This is a "native" IPv6 address. */

        memcpy(dst, src, src_length);
        *dst_length = src_length;
      }

      break;
    }
  }
}

/**
  Return the normalized IP address string for a sock-address.

  The idea is to return an IPv4-address for an IPv4-mapped and
  IPv4-compatible IPv6 address.

  The function writes the normalized IP address to the given buffer.
  The buffer should have enough space, otherwise error flag is returned.
  The system constant INET6_ADDRSTRLEN can be used to reserve buffers of
  the right size.

  @param [in] addr            sockaddr object (AF_INET or AF_INET6).
  @param [in] addr_length     length of the addr.
  @param [out] ip_string      buffer to write normalized IP address.
  @param [in] ip_string_size  size of the ip_string.

  @return Error status.
  @retval true in case of error (the ip_string buffer is not enough).
  @retval false on success.
*/

bool vio_get_normalized_ip_string(const struct sockaddr *addr,
                                  size_t addr_length, char *ip_string,
                                  size_t ip_string_size) {
  struct sockaddr_storage norm_addr_storage;
  struct sockaddr *norm_addr = (struct sockaddr *)&norm_addr_storage;
  size_t norm_addr_length;
  int err_code;

  vio_get_normalized_ip(addr, addr_length, norm_addr, &norm_addr_length);

  err_code = vio_getnameinfo(norm_addr, ip_string, ip_string_size, nullptr, 0,
                             NI_NUMERICHOST);

  if (!err_code) return false;

  DBUG_PRINT("error", ("getnameinfo() failed with %d (%s).", (int)err_code,
                       (const char *)gai_strerror(err_code)));
  return true;
}

/**
  Return IP address and port of a VIO client socket.

  The function returns an IPv4 address if IPv6 support is disabled.

  The function returns an IPv4 address if the client socket is associated
  with an IPv4-compatible or IPv4-mapped IPv6 address. Otherwise, the native
  IPv6 address is returned.
*/

bool vio_peer_addr(Vio *vio, char *ip_buffer, uint16 *port,
                   size_t ip_buffer_size) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("Client socked fd: %d",
                       (int)mysql_socket_getfd(vio->mysql_socket)));

  if (vio->localhost) {
    /*
      Initialize vio->remote and vio->addLen. Set vio->remote to IPv4 loopback
      address.
    */
    struct in_addr *ip4 = &((struct sockaddr_in *)&(vio->remote))->sin_addr;

    vio->remote.ss_family = AF_INET;
    vio->addrLen = sizeof(struct sockaddr_in);

    ip4->s_addr = htonl(INADDR_LOOPBACK);

    /* Initialize ip_buffer and port. */

    my_stpcpy(ip_buffer, "127.0.0.1");
    *port = 0;
  } else {
    int err_code;
    char port_buffer[NI_MAXSERV];

    struct sockaddr_storage addr_storage;
    struct sockaddr *addr = (struct sockaddr *)&addr_storage;
    socket_len_t addr_length = sizeof(addr_storage);

    /* Get sockaddr by socked fd. */

    err_code = mysql_socket_getpeername(vio->mysql_socket, addr, &addr_length);

    if (err_code) {
      DBUG_PRINT("exit", ("getpeername() gave error: %d", socket_errno));
      return true;
    }

    /* Normalize IP address. */

    vio_get_normalized_ip(addr, addr_length, (struct sockaddr *)&vio->remote,
                          &vio->addrLen);

    /* Get IP address & port number. */

    err_code = vio_getnameinfo((struct sockaddr *)&vio->remote, ip_buffer,
                               ip_buffer_size, port_buffer, NI_MAXSERV,
                               NI_NUMERICHOST | NI_NUMERICSERV);

    if (err_code) {
      DBUG_PRINT("exit",
                 ("getnameinfo() gave error: %s", gai_strerror(err_code)));
      return true;
    }

    *port = (uint16)strtol(port_buffer, nullptr, 10);
  }

  DBUG_PRINT("exit", ("Client IP address: %s; port: %d",
                      (const char *)ip_buffer, (int)*port));
  return false;
}

/**
  Retrieve the amount of data that can be read from a socket.

  @param vio          A VIO object.
  @param [out] bytes  The amount of bytes available.

  @retval false   Success.
  @retval true    Failure.
*/
// WL#4896: Not covered
static bool socket_peek_read(Vio *vio, uint *bytes) {
  my_socket sd = mysql_socket_getfd(vio->mysql_socket);
#if defined(_WIN32)
  u_long len;
  if (ioctlsocket(sd, FIONREAD, &len)) return true;
  *bytes = len;
  return false;
#elif defined(FIONREAD_IN_SYS_IOCTL) || defined(FIONREAD_IN_SYS_FILIO)
  int len;
  if (ioctl(sd, FIONREAD, &len) < 0) return true;
  *bytes = len;
  return false;
#else
  char buf[1024];
  ssize_t res = recv(sd, &buf, sizeof(buf), MSG_PEEK);
  if (res < 0) return true;
  *bytes = res;
  return false;
#endif
}

#ifndef _WIN32

/**
  Set of event flags grouped by operations.
*/

/*
  Linux specific flag used to detect connection shutdown. The flag is
  also used for half-closed notification, which here is interpreted as
  if there is data available to be read from the socket.
*/
#ifndef POLLRDHUP
#define POLLRDHUP 0
#endif

/* Data may be read. */
#define MY_POLL_SET_IN (POLLIN | POLLPRI)
/* Data may be written. */
#define MY_POLL_SET_OUT (POLLOUT)
/* An error or hangup. */
#define MY_POLL_SET_ERR (POLLERR | POLLHUP | POLLNVAL)

#endif

/**
  Wait for an I/O event on a VIO socket.

  @param vio      VIO object representing a connected socket.
  @param event    The type of I/O event to wait for.
  @param timeout  Interval (in milliseconds) to wait for an I/O event.
                  A negative timeout value means an infinite timeout.

  @remark sock_errno is set to SOCKET_ETIMEDOUT on timeout.

  @return A three-state value which indicates the operation status.
  @retval -1  Failure, socket_errno indicates the error.
  @retval  0  The wait has timed out.
  @retval  1  The requested I/O event has occurred.
*/

#if !defined(_WIN32) && !defined(HAVE_KQUEUE)
int vio_io_wait(Vio *vio, enum enum_vio_io_event event, timeout_t timeout) {
  int ret;
  int retry_count = 0;
#ifndef DBUG_OFF
  short revents = 0;
#endif
  struct pollfd pfd;
  my_socket sd = mysql_socket_getfd(vio->mysql_socket);
  MYSQL_SOCKET_WAIT_VARIABLES(locker, state) /* no ';' */
  DBUG_TRACE;

  memset(&pfd, 0, sizeof(pfd));

  pfd.fd = sd;

  /*
    Set the poll bitmask describing the type of events.
    The error flags are only valid in the revents bitmask.
  */
  switch (event) {
    case VIO_IO_EVENT_READ:
      pfd.events = MY_POLL_SET_IN;
#ifndef DBUG_OFF
      revents = MY_POLL_SET_IN | MY_POLL_SET_ERR | POLLRDHUP;
#endif
      break;
    case VIO_IO_EVENT_WRITE:
    case VIO_IO_EVENT_CONNECT:
      pfd.events = MY_POLL_SET_OUT;
#ifndef DBUG_OFF
      revents = MY_POLL_SET_OUT | MY_POLL_SET_ERR;
#endif
      break;
  }

  MYSQL_START_SOCKET_WAIT(locker, &state, vio->mysql_socket, PSI_SOCKET_SELECT,
                          0);

#ifdef USE_PPOLL_IN_VIO
  // Check if shutdown is in progress, if so return -1
  if (vio->poll_shutdown_flag.test_and_set()) return -1;

  timespec ts;
  timespec *ts_ptr = nullptr;

  if (!timeout_is_infinite(timeout)) {
    uint timeout_ms = timeout_to_millis(timeout);
    ts = {timeout_ms / 1000, (timeout_ms % 1000) * 1000000};
    ts_ptr = &ts;
  }
#endif

  /*
    Wait for the I/O event and return early in case of
    error or timeout.
  */
  do {
#ifdef USE_PPOLL_IN_VIO
    /*
      vio->signal_mask is only useful when thread_id != 0.
      thread_id is only set for servers, so signal_mask is unused for client
      libraries.
    */
    ret = ppoll(&pfd, 1, ts_ptr,
                vio->thread_id != 0 ? &vio->signal_mask : nullptr);
#else
    ret = poll(&pfd, 1, timeout);
#endif
  } while (ret < 0 && vio_should_retry(vio) &&
           (retry_count++ < vio->retry_count));

#ifdef USE_PPOLL_IN_VIO
  vio->poll_shutdown_flag.clear();
#endif

  switch (ret) {
    case -1:
      /* On error, -1 is returned. */
      break;
    case 0:
      /*
        Set errno to indicate a timeout error.
        (This is not compiled in on WIN32.)
      */
      errno = SOCKET_ETIMEDOUT;
      break;
    default:
      /* Ensure that the requested I/O event has completed. */
      DBUG_ASSERT(pfd.revents & revents);
      break;
  }

  MYSQL_END_SOCKET_WAIT(locker, 0);
  return ret;
}

#elif defined(_WIN32)
int vio_io_wait(Vio *vio, enum enum_vio_io_event event, timeout_t timeout) {
  int ret;
  int retry_count = 0;
  struct timeval tm;
  my_socket fd;
  fd_set readfds, writefds, exceptfds;
  MYSQL_SOCKET_WAIT_VARIABLES(locker, state) /* no ';' */
  DBUG_TRACE;

  fd = mysql_socket_getfd(vio->mysql_socket);

  if (fd == INVALID_SOCKET) return -1;

  /* Convert the timeout, in milliseconds, to seconds and microseconds. */
  if (!timeout_is_infinite(timeout)) {
    uint timeout_ms = timeout_to_millis(timeout);
    tm.tv_sec = timeout_ms / 1000;
    tm.tv_usec = (timeout_ms % 1000) * 1000;
  }

  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_ZERO(&exceptfds);

  /* Always receive notification of exceptions. */
  FD_SET(fd, &exceptfds);

  switch (event) {
    case VIO_IO_EVENT_READ:
      /* Readiness for reading. */
      FD_SET(fd, &readfds);
      break;
    case VIO_IO_EVENT_WRITE:
    case VIO_IO_EVENT_CONNECT:
      /* Readiness for writing. */
      FD_SET(fd, &writefds);
      break;
  }

  MYSQL_START_SOCKET_WAIT(locker, &state, vio->mysql_socket, PSI_SOCKET_SELECT,
                          0);

  /* The first argument is ignored on Windows. */
  do {
    ret = select((int)(fd + 1), &readfds, &writefds, &exceptfds,
                 (timeout >= 0) ? &tm : NULL);
  } while (ret < 0 && vio_should_retry(vio) &&
           (retry_count++ < vio->retry_count));

  MYSQL_END_SOCKET_WAIT(locker, 0);

  /* Set error code to indicate a timeout error. */
  if (ret == 0) WSASetLastError(SOCKET_ETIMEDOUT);

  /* Error or timeout? */
  if (ret <= 0) return ret;

  /* The requested I/O event is ready? */
  switch (event) {
    case VIO_IO_EVENT_READ:
      ret = (FD_ISSET(fd, &readfds) ? 1 : 0);
      break;
    case VIO_IO_EVENT_WRITE:
    case VIO_IO_EVENT_CONNECT:
      ret = (FD_ISSET(fd, &writefds) ? 1 : 0);
      break;
  }

  /* Error conditions pending? */
  ret |= (FD_ISSET(fd, &exceptfds) ? 1 : 0);

  /* Not a timeout, ensure that a condition was met. */
  DBUG_ASSERT(ret);

  return ret;
}
#elif defined(HAVE_KQUEUE)
int vio_io_wait(Vio *vio, enum enum_vio_io_event event, timeout_t timeout) {
  int nev;
  static const int MAX_EVENT = 2;
  struct kevent kev_set[MAX_EVENT];
  struct kevent kev_event[MAX_EVENT];

  my_socket fd = mysql_socket_getfd(vio->mysql_socket);
  MYSQL_SOCKET_WAIT_VARIABLES(locker, state) /* no ';' */
  DBUG_TRACE;

  if (vio->kq_fd == -1) return -1;

  EV_SET(&kev_set[1], WAKEUP_EVENT_ID, EVFILT_USER,
         EV_ADD | EV_ENABLE | EV_DISPATCH | EV_CLEAR, 0, 0, nullptr);
  switch (event) {
    case VIO_IO_EVENT_READ:
      EV_SET(&kev_set[0], fd, EVFILT_READ,
             EV_ADD | EV_ENABLE | EV_DISPATCH | EV_CLEAR, 0, 0, nullptr);
      break;
    case VIO_IO_EVENT_WRITE:
    case VIO_IO_EVENT_CONNECT:
      EV_SET(&kev_set[0], fd, EVFILT_WRITE,
             EV_ADD | EV_ENABLE | EV_DISPATCH | EV_CLEAR, 0, 0, nullptr);
      break;
  }
  MYSQL_START_SOCKET_WAIT(locker, &state, vio->mysql_socket, PSI_SOCKET_SELECT,
                          0);

  timespec ts;
  if (!timeout_is_infinite(timeout)) {
    uint timeout_ms = timeout_to_millis(timeout);
    ts = {static_cast<long>(timeout_ms / 1000),
          (static_cast<long>(timeout_ms) % 1000) * 1000000};
  }

  // Check if shutdown is in progress, if so return -1.
  if (vio->kevent_wakeup_flag.test_and_set()) return -1;

  int retry_count = 0;
  do {
    nev = kevent(vio->kq_fd, kev_set, MAX_EVENT, kev_event, MAX_EVENT,
                 !timeout_is_infinite(timeout) ? &ts : nullptr);
  } while (nev < 0 && vio_should_retry(vio) &&
           (retry_count++ < vio->retry_count));

  vio->kevent_wakeup_flag.clear();

  if (nev == -1) {
    // On error, -1 is returned.
    DBUG_PRINT("error", ("kevent returned error %d\n", errno));
  } else if (nev == 0) {
    // Timeout in kevent.
    errno = SOCKET_ETIMEDOUT;
  } else {
    for (int i = 0; i < nev; i++) {
      if (!(kev_event[i].flags & (EV_ERROR | EV_EOF))) {
        // Ensure that the requested I/O event has completed.
        DBUG_ASSERT(event == VIO_IO_EVENT_READ
                        ? kev_event[i].filter & EVFILT_READ
                        : kev_event[i].filter & EVFILT_WRITE);
      }

      // Shutdown or kill in progress, indicate error.
      if (kev_event[i].filter == EVFILT_USER) {
        nev = -1;
        break;
      }
    }
  }

  MYSQL_END_SOCKET_WAIT(locker, 0);
  return nev;
}
#endif  // HAVE_KQUEUE

/**
  Connect to a peer address.

  @param vio       A VIO object.
  @param addr      Socket address containing the peer address.
  @param len       Length of socket address.
  @param nonblocking flag to represent if socket is blocking or nonblocking
  @param timeout   Interval to wait until a connection is established.
  @param [out] connect_done Indication if connect actually completed or not.
                   If set to true this means there's no need to wait for
                   connect to complete anymore.

  @retval false   A connection was successfully established.
  @retval true    A fatal error. See socket_errno.
*/

bool vio_socket_connect(Vio *vio, struct sockaddr *addr, socklen_t len,
                        bool nonblocking, timeout_t timeout,
                        bool *connect_done) {
  int ret, wait;
  int retry_count = 0;
  DBUG_TRACE;

  /* Only for socket-based transport types. */
  DBUG_ASSERT(vio->type == VIO_TYPE_SOCKET || vio->type == VIO_TYPE_TCPIP);

  /* If timeout is not infinite, set socket to non-blocking mode. */
  if ((!timeout_is_infinite(timeout) || nonblocking) &&
      vio_set_blocking(vio, false))
    return true;

  /* Initiate the connection. */
  do {
    ret = mysql_socket_connect(vio->mysql_socket, addr, len);
  } while (ret < 0 && vio_should_retry(vio) &&
           (retry_count++ < vio->retry_count));

  if (connect_done) *connect_done = (ret == 0);

#ifdef _WIN32
  wait = (ret == SOCKET_ERROR) && (WSAGetLastError() == WSAEINPROGRESS ||
                                   WSAGetLastError() == WSAEWOULDBLOCK);
#else
  wait = (ret == -1) && (errno == EINPROGRESS || errno == EALREADY);
#endif

  /*
    The connection is in progress. The vio_io_wait() call can be used
    to wait up to a specified period of time for the connection to
    succeed.

    If vio_io_wait() returns 0 (after waiting however many seconds),
    the socket never became writable (host is probably unreachable.)
    Otherwise, if vio_io_wait() returns 1, then one of two conditions
    exist:

    1. An error occurred. Use getsockopt() to check for this.
    2. The connection was set up successfully: getsockopt() will
       return 0 as an error.
  */
  if (!nonblocking && wait &&
      (vio_io_wait(vio, VIO_IO_EVENT_CONNECT, timeout) == 1)) {
    int error;
    IF_WIN(int, socklen_t) optlen = sizeof(error);
    IF_WIN(char, void) *optval = (IF_WIN(char, void) *)&error;

    /*
      At this point, we know that something happened on the socket.
      But this does not means that everything is alright. The connect
      might have failed. We need to retrieve the error code from the
      socket layer. We must return success only if we are sure that
      it was really a success. Otherwise we might prevent the caller
      from trying another address to connect to.
    */
    if (connect_done) *connect_done = true;
    if (!(ret = mysql_socket_getsockopt(vio->mysql_socket, SOL_SOCKET, SO_ERROR,
                                        optval, &optlen))) {
#ifdef _WIN32
      WSASetLastError(error);
#else
      errno = error;
#endif
      ret = (error != 0);
    }
  }

  /* If necessary, restore the blocking mode, but only if connect succeeded. */
  if (!nonblocking && !timeout_is_infinite(timeout) && (ret == 0)) {
    if (vio_set_blocking(vio, true)) return true;
  }

  if (nonblocking && wait) {
    if (connect_done) *connect_done = false;
    return false;
  } else {
    return (ret != 0);
  }
}

/**
  Determine if the endpoint of a connection is still available.

  @remark The socket is assumed to be disconnected if an EOF
          condition is encountered.

  @param vio      The VIO object.

  @retval true    EOF condition not found.
  @retval false   EOF condition is signaled.
*/

bool vio_is_connected(Vio *vio) {
  uint bytes = 0;
  DBUG_TRACE;

  /*
    The first step of detecting an EOF condition is verifying
    whether there is data to read. Data in this case would be
    the EOF. An exceptional condition event and/or errors are
    interpreted as if there is data to read.
  */
  if (!vio_io_wait(vio, VIO_IO_EVENT_READ, timeout_from_seconds(0)))
    return true;

  /*
    The second step is read() or recv() from the socket returning
    0 (EOF). Unfortunately, it's not possible to call read directly
    as we could inadvertently read meaningful connection data.
    Simulate a read by retrieving the number of bytes available to
    read -- 0 meaning EOF. In the presence of unrecoverable errors,
    the socket is assumed to be disconnected.
  */
  while (socket_peek_read(vio, &bytes)) {
    if (socket_errno != SOCKET_EINTR) return false;
  }

  /* There might be buffered data at the SSL layer. */
  if (!bytes && vio->type == VIO_TYPE_SSL)
    bytes = SSL_pending((SSL *)vio->ssl_arg);

  return bytes ? true : false;
}

#ifndef DBUG_OFF

/**
  Number of bytes in the read or socket buffer

  @remark An EOF condition might count as one readable byte.

  @return number of bytes in one of the buffers or < 0 if error.
*/

ssize_t vio_pending(Vio *vio) {
  uint bytes = 0;

  /* Data pending on the read buffer. */
  if (vio->read_pos < vio->read_end) return vio->read_end - vio->read_pos;

  /* Skip non-socket based transport types. */
  if (vio->type == VIO_TYPE_TCPIP || vio->type == VIO_TYPE_SOCKET) {
    /* Obtain number of readable bytes in the socket buffer. */
    if (socket_peek_read(vio, &bytes)) return -1;
  }

  return (ssize_t)bytes;
}

#endif

/**
  Checks if the error code, returned by vio_getnameinfo(), means it was the
  "No-name" error.

  Windows-specific note: getnameinfo() returns WSANO_DATA instead of
  EAI_NODATA or EAI_NONAME when no reverse mapping is available at the host
  (i.e. Windows can't get hostname by IP-address). This error should be
  treated as EAI_NONAME.

  @return if the error code is actually EAI_NONAME.
  @retval true if the error code is EAI_NONAME.
  @retval false otherwise.
*/

bool vio_is_no_name_error(int err_code) {
#ifdef _WIN32

  return err_code == WSANO_DATA || err_code == EAI_NONAME;

#else

  return err_code == EAI_NONAME;

#endif
}

/**
  This is a wrapper for the system getnameinfo(), because different OS
  differ in the getnameinfo() implementation:
    - Solaris 10 requires that the 2nd argument (salen) must match the
      actual size of the struct sockaddr_storage passed to it;
    - Mac OS X has sockaddr_in::sin_len and sockaddr_in6::sin6_len and
      requires them to be filled.
*/

int vio_getnameinfo(const struct sockaddr *sa, char *hostname,
                    size_t hostname_size, char *port, size_t port_size,
                    int flags) {
  int sa_length = 0;

  switch (sa->sa_family) {
    case AF_INET:
      sa_length = sizeof(struct sockaddr_in);
#ifdef HAVE_SOCKADDR_IN_SIN_LEN
      ((struct sockaddr_in *)sa)->sin_len = sa_length;
#endif /* HAVE_SOCKADDR_IN_SIN_LEN */
      break;

    case AF_INET6:
      sa_length = sizeof(struct sockaddr_in6);
#ifdef HAVE_SOCKADDR_IN6_SIN6_LEN
      ((struct sockaddr_in6 *)sa)->sin6_len = sa_length;
#endif /* HAVE_SOCKADDR_IN6_SIN6_LEN */
      break;
  }

  return getnameinfo(sa, sa_length, hostname, hostname_size, port, port_size,
                     flags);
}
