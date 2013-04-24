/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <runtime/base/server/server_stats.h>
#include <runtime/base/file/stream_wrapper.h>
#include <runtime/base/file/stream_wrapper_registry.h>
#include <runtime/base/file/user_stream_wrapper.h>
#include <memory>
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
#define PHP_STREAM_COPY_ALL     (-1)

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(StreamContext);
///////////////////////////////////////////////////////////////////////////////

StaticString StreamContext::s_class_name("StreamContext");

///////////////////////////////////////////////////////////////////////////////

Object f_stream_context_create(CArrRef options /* = null_array */,
                               CArrRef params /* = null_array */) {
  return Object(NEWOBJ(StreamContext)(options, params));
}

Object f_stream_context_get_default(CArrRef options /* = null_array */) {
  throw NotImplementedException(__func__);
}

Variant f_stream_context_get_options(CObjRef stream_or_context) {
  throw NotImplementedException(__func__);
}

bool f_stream_context_set_option(CObjRef stream_or_context,
                                 CVarRef wrapper,
                                 CStrRef option /* = null_string */,
                                 CVarRef value /* = null_variant */) {
  throw NotImplementedException(__func__);
}

bool f_stream_context_set_param(CObjRef stream_or_context,
                                CArrRef params) {
  throw NotImplementedException(__func__);
}

Variant f_stream_copy_to_stream(CObjRef source, CObjRef dest,
                                int maxlength /* = -1 */,
                                int offset /* = 0 */) {
  if (maxlength == 0) return 0;
  if (maxlength == PHP_STREAM_COPY_ALL) maxlength = 0;

  File *srcFile = source.getTyped<File>();
  File *destFile = dest.getTyped<File>();
  if (maxlength < 0) {
    throw_invalid_argument("maxlength: %d", maxlength);
    return false;
  }
  if (offset > 0 && !srcFile->seek(offset, SEEK_SET) ) {
    raise_warning("Failed to seek to position %d in the stream", offset);
    return false;
  }
  if (destFile->seekable()) {
    (void)destFile->seek(0, SEEK_END);
  }
  int cbytes = 0;
  if (maxlength == 0) maxlength = INT_MAX;
  while (cbytes < maxlength) {
    char buf[8193];
    int remaining = maxlength - cbytes;
    int toread =
      ((remaining >= (int)sizeof(buf)) ? sizeof(buf) - 1 : remaining);
    int rbytes = srcFile->readImpl(buf, toread);
    if (rbytes == 0) break;
    if (rbytes < 0) return false;
    buf[rbytes] = '\0';
    if (destFile->write(String(buf, rbytes, CopyString)) != rbytes) {
      return false;
    }
    cbytes += rbytes;
  }
  return cbytes;
}

bool f_stream_encoding(CObjRef stream, CStrRef encoding /* = null_string */) {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

void f_stream_bucket_append(CObjRef brigade, CObjRef bucket) {
  throw NotSupportedException(__func__, "stream bucket is not supported");
}

void f_stream_bucket_prepend(CObjRef brigade, CObjRef bucket) {
  throw NotSupportedException(__func__, "stream bucket is not supported");
}

Object f_stream_bucket_make_writeable(CObjRef brigade) {
  throw NotSupportedException(__func__, "stream bucket is not supported");
}

Object f_stream_bucket_new(CObjRef stream, CStrRef buffer) {
  throw NotSupportedException(__func__, "stream bucket is not supported");
}

bool f_stream_filter_register(CStrRef filtername, CStrRef classname) {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

bool f_stream_filter_remove(CObjRef stream_filter) {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

Object f_stream_filter_append(CObjRef stream, CStrRef filtername,
                              int read_write /* = 0 */,
                              CVarRef params /* = null_variant */) {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

Object f_stream_filter_prepend(CObjRef stream, CStrRef filtername,
                               int read_write /* = 0 */,
                               CVarRef params /* = null_variant */) {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

Variant f_stream_get_contents(CObjRef handle, int maxlen /* = 0 */,
                              int offset /* = 0 */) {
  if (maxlen < 0) {
    throw_invalid_argument("maxlen: %d", maxlen);
    return false;
  }

  File *file = handle.getTyped<File>();
  if (offset > 0 && !file->seek(offset, SEEK_SET) ) {
    raise_warning("Failed to seek to position %d in the stream", offset);
    return false;
  }

  String ret;
  if (maxlen) {
    ret = String(maxlen, ReserveString);
    char *buf = ret.mutableSlice().ptr;
    maxlen = file->readImpl(buf, maxlen);
    if (maxlen < 0) {
      return false;
    }
    ret.setSize(maxlen);
  } else {
    StringBuffer sb;
    sb.read(file);
    ret = sb.detach();
  }
  return ret;
}

Array f_stream_get_filters() {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

Variant f_stream_get_line(CObjRef handle, int length /* = 0 */,
                          CStrRef ending /* = null_string */) {
  File *file = handle.getTyped<File>();
  return file->readRecord(ending, length);
}

Variant f_stream_get_meta_data(CObjRef stream) {
  File *f = stream.getTyped<File>(true, true);
  if (f) return f->getMetaData();
  return false;
}

Array f_stream_get_transports() {
  return CREATE_VECTOR4("tcp", "udp", "unix", "udg");
}

String f_stream_resolve_include_path(CStrRef filename,
                                     CObjRef context /* = null_object */) {
  struct stat s;
  return Eval::resolveVmInclude(filename.get(), "", &s);
}

Variant f_stream_select(VRefParam read, VRefParam write, VRefParam except,
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

static const StaticString s_sec("sec");
static const StaticString s_usec("usec");

bool f_stream_set_timeout(CObjRef stream, int seconds,
                          int microseconds /* = 0 */) {
  if (stream.getTyped<Socket>(false, true)) {
    return f_socket_set_option
      (stream, SOL_SOCKET, SO_RCVTIMEO,
       CREATE_MAP2(s_sec, seconds, s_usec, microseconds));
  }
  return false;
}

int64_t f_stream_set_write_buffer(CObjRef stream, int buffer) {
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

int64_t f_set_file_buffer(CObjRef stream, int buffer) {
  return f_stream_set_write_buffer(stream, buffer);
}

///////////////////////////////////////////////////////////////////////////////
// Wrappers

Array f_stream_get_wrappers() {
  return Stream::enumWrappers();
}

bool f_stream_register_wrapper(CStrRef protocol, CStrRef classname) {
  return f_stream_wrapper_register(protocol, classname);
}

bool f_stream_wrapper_register(CStrRef protocol, CStrRef classname) {
  std::unique_ptr<Stream::Wrapper> wrapper;
  try {
    wrapper = std::unique_ptr<Stream::Wrapper>(
                   new UserStreamWrapper(protocol, classname));
  } catch (const InvalidArgumentException& e) {
    raise_warning("%s", e.what());
    return false;
  }
  if (!Stream::registerRequestWrapper(protocol, std::move(wrapper))) {
    raise_warning("Unable to register protocol: %s\n", protocol.data());
    return false;
  }
  return true;
}

bool f_stream_wrapper_restore(CStrRef protocol) {
  return Stream::restoreWrapper(protocol);
}

bool f_stream_wrapper_unregister(CStrRef protocol) {
  return Stream::disableWrapper(protocol);
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
  long textaddrlen = 0;

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

  if (textaddrlen) {
    return String(textaddr, textaddrlen, AttachString);
  }
  return String();
}

Variant f_stream_socket_accept(CObjRef server_socket,
                               double timeout /* = 0.0 */,
                               VRefParam peername /* = null */) {
  Socket *sock = server_socket.getTyped<Socket>();
  pollfd p;
  int n;

  p.fd = sock->fd();
  p.events = (POLLIN|POLLERR|POLLHUP);
  p.revents = 0;
  IOStatusHelper io("socket_accept");
  n = poll(&p, 1, (uint64_t)(timeout * 1000.0));
  if (n > 0) {
    struct sockaddr sa;
    socklen_t salen = sizeof(sa);
    Socket *new_sock = socket_accept_impl(server_socket, &sa, &salen);
    peername = get_sockaddr_name(&sa, salen);
    if (new_sock) return Object(new_sock);
  } else if (n < 0) {
    sock->setError(errno);
  } else {
    sock->setError(ETIMEDOUT);
  }
  return false;
}

Variant f_stream_socket_server(CStrRef local_socket,
                               VRefParam errnum /* = null */,
                               VRefParam errstr /* = null */,
                               int flags /* = 0 */,
                               CObjRef context /* = null_object */) {
  String protocol, host; int port;
  parse_socket(local_socket, protocol, host, port);
  return f_socket_server(protocol + "://" + host, port, errnum, errstr);
}

Variant f_stream_socket_client(CStrRef remote_socket,
                               VRefParam errnum /* = null */,
                               VRefParam errstr /* = null */,
                               double timeout /* = 0.0 */,
                               int flags /* = 0 */,
                               CObjRef context /* = null_object */) {
  String protocol, host; int port;
  parse_socket(remote_socket, protocol, host, port);
  return f_fsockopen(protocol + "://" + host, port, errnum, errstr, timeout);
}

Variant f_stream_socket_enable_crypto(CObjRef stream, bool enable,
                                      int crypto_type /* = 0 */,
                                      CObjRef session_stream /* = null_object */) {
  throw NotSupportedException(__func__, "no crypto support on sockets");
}

Variant f_stream_socket_get_name(CObjRef handle, bool want_peer) {
  Variant address, port;
  bool ret;
  if (want_peer) {
    ret = f_socket_getpeername(handle, ref(address), ref(port));
  } else {
    ret = f_socket_getsockname(handle, ref(address), ref(port));
  }
  if (ret) {
    return address.toString() + ":" + port.toString();
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

  if (address == null_string) {
    Socket *sock = socket.getTyped<Socket>();
    host = sock->getAddress();
    port = sock->getPort();
  } else {
    parse_host(address, host, port);
  }

  return f_socket_sendto(socket, data, data.size(), flags, host, port);
}

bool f_stream_socket_shutdown(CObjRef stream, int how) {
  return f_socket_shutdown(stream, how);
}

///////////////////////////////////////////////////////////////////////////////
}
