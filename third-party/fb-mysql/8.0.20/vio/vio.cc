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

#include <sys/types.h>
#include <new>

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_psi_config.h"
#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/mysql_socket.h"
#include "mysql/psi/psi_memory.h"  // IWYU pragma: keep
#include "mysql/service_mysql_alloc.h"
#include "template_utils.h"
#include "vio/vio_priv.h"

PSI_memory_key key_memory_vio_ssl_fd;
PSI_memory_key key_memory_vio;
PSI_memory_key key_memory_vio_read_buffer;

#ifdef HAVE_PSI_INTERFACE
static PSI_memory_info all_vio_memory[] = {
    {&key_memory_vio_ssl_fd, "ssl_fd", 0, 0, PSI_DOCUMENT_ME},
    {&key_memory_vio, "vio", 0, 0, PSI_DOCUMENT_ME},
    {&key_memory_vio_read_buffer, "read_buffer", 0, 0, PSI_DOCUMENT_ME},
};

void init_vio_psi_keys() {
  const char *category = "vio";
  int count;

  count = array_elements(all_vio_memory);
  mysql_memory_register(category, all_vio_memory, count);
}
#endif

Vio *internal_vio_create(uint flags);
void internal_vio_delete(Vio *vio);

#ifdef _WIN32

/**
  Stub io_wait method that defaults to indicate that
  requested I/O event is ready.

  Used for named pipe and shared memory VIO types.

  @param vio      Unused.
  @param event    Unused.
  @param timeout  Unused.

  @retval 1       The requested I/O event has occurred.
*/

static int no_io_wait(Vio *vio MY_ATTRIBUTE((unused)),
                      enum enum_vio_io_event event MY_ATTRIBUTE((unused)),
                      int timeout MY_ATTRIBUTE((unused))) {
  return 1;
}

#endif

extern "C" {
static bool has_no_data(Vio *vio MY_ATTRIBUTE((unused))) { return false; }
}  // extern "C"

Vio::Vio(uint flags) {
  mysql_socket = MYSQL_INVALID_SOCKET;
  local = sockaddr_storage();
  remote = sockaddr_storage();
#ifdef USE_PPOLL_IN_VIO
  sigemptyset(&signal_mask);
#elif defined(HAVE_KQUEUE)
  kq_fd = -1;
#endif
  if (flags & VIO_BUFFERED_READ)
    read_buffer = (char *)my_malloc(key_memory_vio_read_buffer,
                                    VIO_READ_BUFFER_SIZE, MYF(MY_WME));
}

Vio::~Vio() {
  my_free(read_buffer);
  read_buffer = nullptr;
#ifdef HAVE_KQUEUE
  if (kq_fd != -1) close(kq_fd);
#endif
}

Vio &Vio::operator=(Vio &&vio) {
  this->~Vio();

  mysql_socket = vio.mysql_socket;
  localhost = vio.localhost;
  type = vio.type;
  read_timeout = vio.read_timeout;
  write_timeout = vio.write_timeout;
  retry_count = vio.retry_count;
  inactive = vio.inactive;

  local = vio.local;
  remote = vio.remote;
  addrLen = vio.addrLen;
  read_buffer = vio.read_buffer;
  read_pos = vio.read_pos;
  read_end = vio.read_end;

  is_blocking_flag = vio.is_blocking_flag;

#ifdef USE_PPOLL_IN_VIO
  thread_id = vio.thread_id;
  signal_mask = vio.signal_mask;
  if (vio.poll_shutdown_flag.test_and_set())
    poll_shutdown_flag.test_and_set();
  else
    poll_shutdown_flag.clear();
#elif defined(HAVE_KQUEUE)
  kq_fd = vio.kq_fd;
  if (vio.kevent_wakeup_flag.test_and_set())
    kevent_wakeup_flag.test_and_set();
  else
    kevent_wakeup_flag.clear();
#endif

  viodelete = vio.viodelete;
  vioerrno = vio.vioerrno;
  read = vio.read;
  write = vio.write;
  timeout = vio.timeout;
  viokeepalive = vio.viokeepalive;
  fastsend = vio.fastsend;
  peer_addr = vio.peer_addr;
  in_addr = vio.in_addr;
  should_retry = vio.should_retry;
  was_timeout = vio.was_timeout;

  vioshutdown = vio.vioshutdown;
  is_connected = vio.is_connected;
  has_data = vio.has_data;
  io_wait = vio.io_wait;
  connect = vio.connect;

  is_blocking = vio.is_blocking;
  set_blocking = vio.set_blocking;

#ifdef _WIN32
  overlapped = vio.overlapped;
  hPipe = vio.hPipe;
#endif

  ssl_arg = vio.ssl_arg;

#ifdef _WIN32
  handle_file_map = vio.handle_file_map;
  handle_map = vio.handle_map;
  event_server_wrote = vio.event_client_wrote;
  event_server_read = vio.event_server_read;
  event_client_wrote = vio.event_client_wrote;
  event_client_read = vio.event_client_read;
  event_conn_closed = vio.event_conn_closed;
  shared_memory_remain = vio.shared_memory_remain;
  shared_memory_pos = vio.shared_memory_pos;
#endif

  // These are the only elements touched by the destructor.
  vio.read_buffer = nullptr;
#ifdef HAVE_KQUEUE
  vio.kq_fd = -1;
#endif

  return *this;
}

/*
 * Helper to fill most of the Vio* with defaults.
 */

static bool vio_init(Vio *vio, enum enum_vio_type type, my_socket sd,
                     uint flags) {
  DBUG_PRINT("enter vio_init", ("type: %d sd: %d  flags: %d", type, sd, flags));

  mysql_socket_setfd(&vio->mysql_socket, sd);

#ifdef HAVE_KQUEUE
  DBUG_ASSERT(type == VIO_TYPE_TCPIP || type == VIO_TYPE_SOCKET ||
              type == VIO_TYPE_SSL);
  vio->kq_fd = kqueue();
  if (vio->kq_fd == -1) {
    DBUG_PRINT("vio_init", ("kqueue failed with errno: %d", errno));
    return true;
  }
#endif

  vio->localhost = flags & VIO_LOCALHOST;
  vio->type = type;

#ifdef HAVE_SETNS
  vio->network_namespace[0] = '\0';
#endif

#ifdef _WIN32
  if (type == VIO_TYPE_NAMEDPIPE) {
    vio->viodelete = vio_delete;
    vio->vioerrno = vio_errno;
    vio->read = vio_read_pipe;
    vio->write = vio_write_pipe;
    vio->fastsend = vio_fastsend;
    vio->viokeepalive = vio_keepalive;
    vio->should_retry = vio_should_retry;
    vio->was_timeout = vio_was_timeout;
    vio->vioshutdown = vio_shutdown_pipe;
    vio->peer_addr = vio_peer_addr;
    vio->io_wait = no_io_wait;
    vio->is_connected = vio_is_connected_pipe;
    vio->has_data = has_no_data;
    vio->is_blocking = vio_is_blocking;
    vio->set_blocking = vio_set_blocking;
    vio->is_blocking_flag = true;
    return false;
  }
  if (type == VIO_TYPE_SHARED_MEMORY) {
    vio->viodelete = vio_delete_shared_memory;
    vio->vioerrno = vio_errno;
    vio->read = vio_read_shared_memory;
    vio->write = vio_write_shared_memory;
    vio->fastsend = vio_fastsend;
    vio->viokeepalive = vio_keepalive;
    vio->should_retry = vio_should_retry;
    vio->was_timeout = vio_was_timeout;
    vio->vioshutdown = vio_shutdown_shared_memory;
    vio->peer_addr = vio_peer_addr;
    vio->io_wait = no_io_wait;
    vio->is_connected = vio_is_connected_shared_memory;
    vio->has_data = has_no_data;
    vio->is_blocking = vio_is_blocking;
    vio->set_blocking = vio_set_blocking;
    vio->is_blocking_flag = true;
    return false;
  }
#endif /* _WIN32 */
  if (type == VIO_TYPE_SSL) {
    vio->viodelete = vio_ssl_delete;
    vio->vioerrno = vio_errno;
    vio->read = vio_ssl_read;
    vio->write = vio_ssl_write;
    vio->fastsend = vio_fastsend;
    vio->viokeepalive = vio_keepalive;
    vio->should_retry = vio_should_retry;
    vio->was_timeout = vio_was_timeout;
    vio->vioshutdown = vio_ssl_shutdown;
    vio->peer_addr = vio_peer_addr;
    vio->io_wait = vio_io_wait;
    vio->is_connected = vio_is_connected;
    vio->has_data = vio_ssl_has_data;
    vio->timeout = vio_socket_timeout;
    vio->is_blocking = vio_is_blocking;
    vio->set_blocking = vio_set_blocking;
    vio->set_blocking_flag = vio_set_blocking_flag;
    vio->is_blocking_flag = true;
    return false;
  }
  vio->viodelete = vio_delete;
  vio->vioerrno = vio_errno;
  vio->read = vio->read_buffer ? vio_read_buff : vio_read;
  vio->write = vio_write;
  vio->fastsend = vio_fastsend;
  vio->viokeepalive = vio_keepalive;
  vio->should_retry = vio_should_retry;
  vio->was_timeout = vio_was_timeout;
  vio->vioshutdown = vio_shutdown;
  vio->peer_addr = vio_peer_addr;
  vio->io_wait = vio_io_wait;
  vio->is_connected = vio_is_connected;
  vio->timeout = vio_socket_timeout;
  vio->has_data = vio->read_buffer ? vio_buff_has_data : has_no_data;
  vio->is_blocking = vio_is_blocking;
  vio->set_blocking = vio_set_blocking;
  vio->set_blocking_flag = vio_set_blocking_flag;
  vio->is_blocking_flag = true;

  return false;
}

/**
  Reinitialize an existing Vio object.

  @remark Used to rebind an initialized socket-based Vio object
          to another socket-based transport type. For example,
          rebind a TCP/IP transport to SSL.

  @remark If new socket handle passed to vio_reset() is not equal
          to the socket handle stored in Vio then socket handle will
          be closed before storing new value. If handles are equal
          then old socket is not closed. This is important for
          vio_reset() usage in ssl_do().

  @remark If any error occurs then Vio members won't be altered thus
          preserving socket handle stored in Vio and not taking
          ownership over socket handle passed as parameter.

  @param vio    A VIO object.
  @param type   A socket-based transport type.
  @param sd     The socket.
  @param ssl    An optional SSL structure.
  @param flags  Flags passed to new_vio.

  @return Return value is zero on success.
*/

bool vio_reset(Vio *vio, enum enum_vio_type type, my_socket sd,
               void *ssl MY_ATTRIBUTE((unused)), uint flags) {
  int ret = false;
  Vio new_vio(flags);
  DBUG_TRACE;

  /* The only supported rebind is from a socket-based transport type. */
  DBUG_ASSERT(vio->type == VIO_TYPE_TCPIP || vio->type == VIO_TYPE_SOCKET);

  if (vio_init(&new_vio, type, sd, flags)) return true;

  /* Preserve perfschema info for this connection */
  new_vio.mysql_socket.m_psi = vio->mysql_socket.m_psi;

  new_vio.ssl_arg = ssl;

  /*
    Propagate the timeout values. Necessary to also propagate
    the underlying proprieties associated with the timeout,
    such as the socket blocking mode.
  */
  if (!timeout_is_infinite(vio->read_timeout))
    ret |= vio_timeout(&new_vio, 0, vio->read_timeout);

  if (!timeout_is_infinite(vio->write_timeout))
    ret |= vio_timeout(&new_vio, 1, vio->write_timeout);

  /* Propagate the timeout error message */
  new_vio.timeout_err_msg = vio->timeout_err_msg;

  if (!ret) {
    /*
      vio_reset() succeeded
      free old resources and then overwrite VIO structure
    */

    /*
      Close socket only when it is not equal to the new one.
    */
    if (sd != mysql_socket_getfd(vio->mysql_socket)) {
      if (vio->inactive == false) vio->vioshutdown(vio);
    }
#ifdef HAVE_KQUEUE
    else {
      /*
      Must set the fd to -1, otherwise the destructor would
      close it again possibly closing socket or file opened
      by other threads concurrently.
      */
      close(vio->kq_fd);
      vio->kq_fd = -1;
    }
#endif
    /*
      Overwrite existing Vio structure
    */
    *vio = std::move(new_vio);
  }

  return ret;
}

Vio *internal_vio_create(uint flags) {
  void *rawmem = my_malloc(key_memory_vio, sizeof(Vio), MYF(MY_WME));
  if (rawmem == nullptr) return nullptr;
  return new (rawmem) Vio(flags);
}

/* Create a new VIO for socket or TCP/IP connection. */

Vio *mysql_socket_vio_new(MYSQL_SOCKET mysql_socket, enum_vio_type type,
                          uint flags) {
  Vio *vio;
  my_socket sd = mysql_socket_getfd(mysql_socket);
  DBUG_TRACE;
  DBUG_PRINT("enter", ("sd: %d", sd));

  if ((vio = internal_vio_create(flags))) {
    if (vio_init(vio, type, sd, flags)) {
      internal_vio_delete(vio);
      return nullptr;
    }
    vio->mysql_socket = mysql_socket;
  }
  return vio;
}

/* Open the socket or TCP/IP connection and read the fnctl() status */

Vio *vio_new(my_socket sd, enum enum_vio_type type, uint flags) {
  Vio *vio;
  MYSQL_SOCKET mysql_socket = MYSQL_INVALID_SOCKET;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("sd: %d", sd));

  mysql_socket_setfd(&mysql_socket, sd);
  vio = mysql_socket_vio_new(mysql_socket, type, flags);

  return vio;
}

#ifdef _WIN32

Vio *vio_new_win32pipe(HANDLE hPipe) {
  Vio *vio;
  DBUG_TRACE;
  if ((vio = internal_vio_create(VIO_LOCALHOST))) {
    if (vio_init(vio, VIO_TYPE_NAMEDPIPE, 0, VIO_LOCALHOST)) {
      internal_vio_delete(vio);
      return nullptr;
    }

    /* Create an object for event notification. */
    vio->overlapped.hEvent = CreateEvent(NULL, false, false, NULL);
    if (vio->overlapped.hEvent == NULL) {
      internal_vio_delete(vio);
      return NULL;
    }
    vio->hPipe = hPipe;
  }
  return vio;
}

Vio *vio_new_win32shared_memory(HANDLE handle_file_map, HANDLE handle_map,
                                HANDLE event_server_wrote,
                                HANDLE event_server_read,
                                HANDLE event_client_wrote,
                                HANDLE event_client_read,
                                HANDLE event_conn_closed) {
  Vio *vio;
  DBUG_TRACE;
  if ((vio = internal_vio_create(VIO_LOCALHOST))) {
    if (vio_init(vio, VIO_TYPE_SHARED_MEMORY, 0, VIO_LOCALHOST)) {
      internal_vio_delete(vio);
      return nullptr;
    }
    vio->handle_file_map = handle_file_map;
    vio->handle_map = reinterpret_cast<char *>(handle_map);
    vio->event_server_wrote = event_server_wrote;
    vio->event_server_read = event_server_read;
    vio->event_client_wrote = event_client_wrote;
    vio->event_client_read = event_client_read;
    vio->event_conn_closed = event_conn_closed;
    vio->shared_memory_remain = 0;
    vio->shared_memory_pos = reinterpret_cast<char *>(handle_map);
  }
  return vio;
}
#endif

/**
  Set timeout for a network send or receive operation.

  @note A non-infinite timeout causes the socket to be
          set to non-blocking mode. On infinite timeouts,
          the socket is set to blocking mode.

  @note A negative timeout means an infinite timeout.

  @param vio      A VIO object.
  @param which    Whether timeout is for send (1) or receive (0).
  @param timeout  Timeout interval.

  @return false on success, true otherwise.
*/

int vio_timeout(Vio *vio, uint which, timeout_t timeout) {
  bool old_mode;

  /* Deduce the current timeout status mode. */
  old_mode = timeout_is_infinite(vio->write_timeout) &&
             timeout_is_infinite(vio->read_timeout);

  if (which)
    vio->write_timeout = timeout;
  else
    vio->read_timeout = timeout;

  /* VIO-specific timeout handling. Might change the blocking mode. */
  return vio->timeout ? vio->timeout(vio, which, old_mode) : 0;
}

void internal_vio_delete(Vio *vio) {
  if (!vio) return; /* It must be safe to delete null pointers. */
  if (vio->inactive == false) vio->vioshutdown(vio);
  vio->~Vio();
  my_free(vio);
}

void vio_delete(Vio *vio) { internal_vio_delete(vio); }

/*
  Cleanup memory allocated by vio or the
  components below it when application finish

*/
void vio_end(void) { vio_ssl_end(); }

struct vio_string {
  const char *m_str;
  int m_len;
};

/**
  Names for each VIO TYPE.
  Indexed by enum_vio_type.
  If you add more, please update audit_log.cc
*/
static const vio_string vio_type_names[] = {{"", 0},
                                            {STRING_WITH_LEN("TCP/IP")},
                                            {STRING_WITH_LEN("Socket")},
                                            {STRING_WITH_LEN("Named Pipe")},
                                            {STRING_WITH_LEN("SSL/TLS")},
                                            {STRING_WITH_LEN("Shared Memory")},
                                            {STRING_WITH_LEN("Internal")},
                                            {STRING_WITH_LEN("Plugin")}};

void get_vio_type_name(enum enum_vio_type vio_type, const char **str,
                       int *len) {
  int index;

  if ((vio_type >= FIRST_VIO_TYPE) && (vio_type <= LAST_VIO_TYPE)) {
    index = vio_type;
  } else {
    index = 0;
  }
  *str = vio_type_names[index].m_str;
  *len = vio_type_names[index].m_len;
  return;
}
