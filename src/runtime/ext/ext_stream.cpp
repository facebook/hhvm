/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <runtime/ext/ext_stream.h>
#include <runtime/ext/ext_socket.h>
#include <runtime/ext/ext_network.h>
#include <runtime/base/file/socket.h>
#include <runtime/base/file/plain_file.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/zend/zend_printf.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#if defined(AF_UNIX)
#include <sys/un.h>
#endif

#define PHP_STREAM_BUFFER_NONE  0   /* unbuffered */
#define PHP_STREAM_BUFFER_LINE  1   /* line buffered */
#define PHP_STREAM_BUFFER_FULL  2   /* fully buffered */

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(StreamContext);
///////////////////////////////////////////////////////////////////////////////

Variant f_stream_copy_to_stream(CObjRef source, CObjRef dest,
                                int maxlength /* = 0 */,
                                int offset /* = 0 */) {
  Variant ret = f_stream_get_contents(source, maxlength, offset);
  if (same(ret, false)) {
    return false;
  }

  return dest.getTyped<File>()->write(ret.toString());
}

Variant f_stream_get_contents(CObjRef handle, int maxlen /* = 0 */,
                              int offset /* = 0 */) {
  if (maxlen < 0) {
    throw_invalid_argument("maxlen: %d", maxlen);
    return false;
  }

  File *file = handle.getTyped<File>();
  if (offset > 0 && file->seek(offset, SEEK_SET) < 0) {
    raise_warning("Failed to seek to position %ld in the stream", offset);
    return false;
  }

  String ret;
  if (maxlen) {
    char *buf = (char *)malloc(maxlen + 1);
    maxlen = file->readImpl(buf, maxlen);
    if (maxlen < 0) {
      free(buf);
      return false;
    }
    buf[maxlen] = '\0';
    ret = String(buf, maxlen, AttachString);
  } else {
    StringBuffer sb;
    sb.read(file);
    ret = sb.detach();
  }
  return ret;
}

Variant f_stream_get_line(CObjRef handle, int length /* = 0 */,
                          CStrRef ending /* = null_string */) {
  File *file = handle.getTyped<File>();
  String record = file->readRecord(ending, length);
  if (record.empty()) {
    return false;
  }
  return record;
}

Variant f_stream_select(Variant read, Variant write, Variant except,
                        CVarRef vtv_sec, int tv_usec /* = 0 */) {
  return f_socket_select(ref(read), ref(write), ref(except), vtv_sec, tv_usec);
}

bool f_stream_set_blocking(CObjRef stream, int mode) {
  File *file = stream.getTyped<File>();
  int flags = fcntl(file->fd(), F_GETFL, 0);
  if (mode) {
    flags &= ~O_NONBLOCK;
  } else {
    flags |= O_NONBLOCK;
  }
  return fcntl(file->fd(), F_SETFL, flags) != -1;
}

bool f_stream_set_timeout(CObjRef stream, int seconds,
                          int microseconds /* = 0 */) {
  if (stream.getTyped<Socket>(false, true)) {
    return f_socket_set_option
      (stream, SOL_SOCKET, SO_RCVTIMEO,
       CREATE_MAP2("sec", seconds, "usec", microseconds));
  }
  return false;
}

int f_stream_set_write_buffer(CObjRef stream, int buffer) {
  PlainFile *file = stream.getTyped<PlainFile>(false, true);
  if (file) {
    switch (buffer) {
    case PHP_STREAM_BUFFER_NONE:
      return setvbuf(file->getStream(), NULL, _IONBF, 0);
    case PHP_STREAM_BUFFER_LINE:
      return setvbuf(file->getStream(), NULL, _IOLBF, BUFSIZ);
    case PHP_STREAM_BUFFER_FULL:
      return setvbuf(file->getStream(), NULL, _IOFBF, BUFSIZ);
    default:
      break;
    }
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
// stream socket functions

static void parse_host(CStrRef address, String &host, int &port) {
  int pos = address.find(':');
  if (pos >= 0) {
    host = address.substr(0, pos);
    port = address.substr(pos + 1).toInt16();
  } else {
    host = address;
    port = 0;
  }
}

static void parse_socket(CStrRef socket, String &protocol, String &host,
                         int &port) {
  String address;
  int pos = socket.find("://");
  if (pos >= 0) {
    protocol = socket.substr(0, pos);
    address = socket.substr(pos + 3);
  } else {
    protocol = "tcp";
    address = socket;
  }

  parse_host(address, host, port);
}

static Socket *socket_accept_impl(CObjRef socket, struct sockaddr *addr,
                                  socklen_t *addrlen) {
  Socket *sock = socket.getTyped<Socket>();
  Socket *new_sock = new Socket(accept(sock->fd(), addr, addrlen),
                                sock->getType());
  if (!new_sock->valid()) {
    SOCKET_ERROR(new_sock, "unable to accept incoming connection", errno);
    delete new_sock;
    return NULL;
  }
  return new_sock;
}

static String get_sockaddr_name(struct sockaddr *sa, socklen_t sl) {
  char abuf[256];
  char *buf = NULL;
  char *textaddr = NULL;
  long textaddrlen;

  switch (sa->sa_family) {
  case AF_INET:
    buf = inet_ntoa(((struct sockaddr_in*)sa)->sin_addr);
    if (buf) {
      textaddrlen = spprintf(&textaddr, 0, "%s:%d",
      buf, ntohs(((struct sockaddr_in*)sa)->sin_port));
    }
    break;

   case AF_INET6:
    buf = (char*)inet_ntop(sa->sa_family,
                           &((struct sockaddr_in6*)sa)->sin6_addr,
                           (char *)&abuf, sizeof(abuf));
    if (buf) {
      textaddrlen = spprintf(&textaddr, 0, "%s:%d",
      buf, ntohs(((struct sockaddr_in6*)sa)->sin6_port));
    }
    break;

   case AF_UNIX:
     {
       struct sockaddr_un *ua = (struct sockaddr_un*)sa;

       if (ua->sun_path[0] == '\0') {
         /* abstract name */
         int len = strlen(ua->sun_path + 1) + 1;
         textaddrlen = len;
         textaddr = (char *)malloc((len + 1) );
         memcpy(textaddr, ua->sun_path, len);
         textaddr[len] = '\0';
       } else {
         textaddrlen = strlen(ua->sun_path);
         textaddr = strndup((ua->sun_path), textaddrlen);
       }
       break;
    }

  default:
    break;
  }

  if (textaddr) {
    return String(textaddr, textaddrlen, AttachString);
  }
  return String();
}

Variant f_stream_socket_accept(CObjRef server_socket,
                               double timeout /* = 0.0 */,
                               Variant peername /* = null */) {
  Socket *sock = server_socket.getTyped<Socket>();
  pollfd p;
  int n;

  p.fd = sock->fd();
  p.events = (POLLIN|POLLERR|POLLHUP);
  p.revents = 0;
  n = poll(&p, 1, (uint64)(timeout * 1000.0));
  if (n > 0) {
    struct sockaddr sa;
    socklen_t salen = sizeof(sa);
    Socket *new_sock = socket_accept_impl(server_socket, &sa, &salen);
    peername = get_sockaddr_name(&sa, salen);
    if (new_sock) return Object(new_sock);
  }
  return false;
}

Variant f_stream_socket_server(CStrRef local_socket,
                               Variant errnum /* = null */,
                               Variant errstr /* = null */,
                               int flags /* = 0 */,
                               CObjRef context /* = null_object */) {
  String protocol, host; int port;
  parse_socket(local_socket, protocol, host, port);
  return f_socket_server(protocol + "://" + host, port, errnum, errstr);
}

Variant f_stream_socket_client(CStrRef remote_socket,
                               Variant errnum /* = null */,
                               Variant errstr /* = null */,
                               double timeout /* = 0.0 */,
                               int flags /* = 0 */,
                               CObjRef context /* = null_object */) {
  String protocol, host; int port;
  parse_socket(remote_socket, protocol, host, port);
  return f_fsockopen(protocol + "://" + host, port, errnum, errstr, timeout);
}

Variant f_stream_socket_get_name(CObjRef handle, bool want_peer) {
  Variant address;
  bool ret;
  if (want_peer) {
    ret = f_socket_getpeername(handle, ref(address));
  } else {
    ret = f_socket_getsockname(handle, ref(address));
  }
  if (ret) {
    return address.toString();
  }
  return false;
}

Variant f_stream_socket_pair(int domain, int type, int protocol) {
  Variant fd;
  if (!f_socket_create_pair(domain, type, protocol, ref(fd))) {
    return false;
  }
  return fd;
}

Variant f_stream_socket_recvfrom(CObjRef socket, int length,
                                 int flags /* = 0 */,
                                 CStrRef address /* = null_string */) {
  String host; int port;
  parse_host(address, host, port);
  Variant ret;
  Variant retval = f_socket_recvfrom(socket, ref(ret), length, flags,
                                     host, port);
  if (!same(retval, false) && retval.toInt64() >= 0) {
    return ret.toString(); // watch out, "ret", not "retval"
  }
  return false;
}

Variant f_stream_socket_sendto(CObjRef socket, CStrRef data,
                               int flags /* = 0 */,
                               CStrRef address /* = null_string */) {
  String host; int port;
  parse_host(address, host, port);
  return f_socket_sendto(socket, data, data.size(), flags, host, port);
}

bool f_stream_socket_shutdown(CObjRef stream, int how) {
  return f_socket_shutdown(stream, how);
}

///////////////////////////////////////////////////////////////////////////////
}
