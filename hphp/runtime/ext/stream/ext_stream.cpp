/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/ext/stream/ext_stream-user-filters.h"
#include "hphp/runtime/ext/ext_socket.h"
#include "hphp/runtime/ext/ext_network.h"
#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/user-stream-wrapper.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/network.h"
#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#if defined(AF_UNIX)
#include <sys/un.h>
#include <algorithm>
#endif

#define PHP_STREAM_BUFFER_NONE  0   /* unbuffered */
#define PHP_STREAM_BUFFER_LINE  1   /* line buffered */
#define PHP_STREAM_BUFFER_FULL  2   /* fully buffered */
#define PHP_STREAM_COPY_ALL     (-1)

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static StreamContext* get_stream_context(const Resource& stream_or_context);

///////////////////////////////////////////////////////////////////////////////

Variant f_stream_context_create(const Array& options /* = null_array */,
                                const Array& params /* = null_array */) {
  if (!options.isNull() && !StreamContext::validateOptions(options)) {
    return false;
  }
  return Resource(NEWOBJ(StreamContext)(options, params));
}

Variant f_stream_context_get_options(const Resource& stream_or_context) {
  StreamContext* context = get_stream_context(stream_or_context);
  if (!context) {
    raise_warning("Invalid stream/context parameter");
    return false;
  }
  return context->getOptions();
}

bool f_stream_context_set_option0(StreamContext* context,
                                  const Array& options) {
  if (!StreamContext::validateOptions(options)) {
    raise_warning("options should have the form "
                  "[\"wrappername\"][\"optionname\"] = $value");
    return false;
  }
  context->mergeOptions(options);
  return true;
}

bool f_stream_context_set_option1(StreamContext* context,
                                  const String& wrapper,
                                  const String& option,
                                  const Variant& value) {
  context->setOption(wrapper, option, value);
  return true;
}

bool f_stream_context_set_option(const Resource& stream_or_context,
                                 const Variant& wrapper_or_options,
                                 const Variant& option /* = null_variant */,
                                 const Variant& value /* = null_variant */) {
  StreamContext* context = get_stream_context(stream_or_context);
  if (!context) {
    raise_warning("Invalid stream/context parameter");
    return false;
  }
  if (wrapper_or_options.isArray() &&
      !option.isInitialized() &&
      !value.isInitialized()) {
    return f_stream_context_set_option0(context, wrapper_or_options.toArray());
  } else if (wrapper_or_options.isString() &&
             option.isInitialized() &&
             option.isString() &&
             value.isInitialized()) {
    return f_stream_context_set_option1(context, wrapper_or_options.toString(),
                                        option.toString(), value);
  } else {
    raise_warning("called with wrong number or type of parameters; please RTM");
    return false;
  }
}

Variant f_stream_context_get_default(const Array& options /* = null_array */) {
  Resource &resource = g_context->getStreamContext();
  if (resource.isNull()) {
    resource = Resource(NEWOBJ(StreamContext)(Array::Create(),
                                              Array::Create()));
    g_context->setStreamContext(resource);
  }
  StreamContext *context = resource.getTyped<StreamContext>();
  if (!options.isNull() && !f_stream_context_set_option0(context, options)) {
    return false;
  }
  return resource;
}

Variant f_stream_context_set_default(const Array& options) {
  return f_stream_context_get_default(options);
}

Variant f_stream_context_get_params(const Resource& stream_or_context) {
  StreamContext* context = get_stream_context(stream_or_context);
  if (!context) {
    raise_warning("Invalid stream/context parameter");
    return false;
  }
  return context->getParams();
}

bool f_stream_context_set_params(const Resource& stream_or_context,
                                 const Array& params) {
  StreamContext* context = get_stream_context(stream_or_context);
  if (!context || !StreamContext::validateParams(params)) {
    raise_warning("Invalid stream/context parameter");
    return false;
  }
  context->mergeParams(params);
  return true;
}

Variant f_stream_copy_to_stream(const Resource& source, const Resource& dest,
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

Variant f_stream_get_contents(const Resource& handle, int maxlen /* = -1 */,
                              int offset /* = -1 */) {
  if (maxlen < -1) {
    throw_invalid_argument("maxlen: %d", maxlen);
    return false;
  }

  if (maxlen == 0) {
    return String();
  }

  File *file = handle.getTyped<File>(false /* nullOkay */,
                                     true /* badTypeOkay */);
  if (!file) {
    throw_invalid_argument(
      "stream_get_contents() expects parameter 1 to be a resource");
    return false;
  }

  if (offset >= 0 && !file->seek(offset, SEEK_SET) ) {
    raise_warning("Failed to seek to position %d in the stream", offset);
    return false;
  }

  String ret;
  if (maxlen != -1) {
    if (maxlen < 0) {
      return false;
    }
    ret = file->read(maxlen);
  } else {
    ret = file->read();
  }
  return ret;
}

Variant f_stream_get_line(const Resource& handle, int length /* = 0 */,
                          const String& ending /* = null_string */) {
  File *file = handle.getTyped<File>();
  return file->readRecord(ending, length);
}

Variant f_stream_get_meta_data(const Resource& stream) {
  File *f = stream.getTyped<File>(true, true);
  if (f) return f->getMetaData();
  return false;
}

Array f_stream_get_transports() {
  return make_packed_array("tcp", "udp", "unix", "udg");
}

Variant f_stream_resolve_include_path(const String& filename,
                                     const Resource& context /* = null_object */) {
  struct stat s;
  String ret = Eval::resolveVmInclude(filename.get(), "", &s);
  if (ret.isNull()) {
    return false;
  }
  return ret;
}

Variant f_stream_select(VRefParam read, VRefParam write, VRefParam except,
                        const Variant& vtv_sec, int tv_usec /* = 0 */) {
  return f_socket_select(ref(read), ref(write), ref(except), vtv_sec, tv_usec);
}

bool f_stream_set_blocking(const Resource& stream, int mode) {
  File *file = stream.getTyped<File>();
  int flags = fcntl(file->fd(), F_GETFL, 0);
  if (mode) {
    flags &= ~O_NONBLOCK;
  } else {
    flags |= O_NONBLOCK;
  }
  return fcntl(file->fd(), F_SETFL, flags) != -1;
}

const StaticString
  s_sec("sec"),
  s_usec("usec");

bool f_stream_set_timeout(const Resource& stream, int seconds,
                          int microseconds /* = 0 */) {
  if (stream.getTyped<Socket>(false, true)) {
    return f_socket_set_option
      (stream, SOL_SOCKET, SO_RCVTIMEO,
       make_map_array(s_sec, seconds, s_usec, microseconds));
  }
  return false;
}

int64_t f_stream_set_write_buffer(const Resource& stream, int buffer) {
  PlainFile *plain_file = stream.getTyped<PlainFile>(false, true);
  if (!plain_file) {
    return -1;
  }
  FILE* file = plain_file->getStream();
  if (!file) {
    return -1;
  }

  switch (buffer) {
  case PHP_STREAM_BUFFER_NONE:
    return setvbuf(file, nullptr, _IONBF, 0);
  case PHP_STREAM_BUFFER_LINE:
    return setvbuf(file, nullptr, _IOLBF, BUFSIZ);
  case PHP_STREAM_BUFFER_FULL:
    return setvbuf(file, nullptr, _IOFBF, BUFSIZ);
  default:
    return -1;
  }
}

int64_t f_set_file_buffer(const Resource& stream, int buffer) {
  return f_stream_set_write_buffer(stream, buffer);
}

///////////////////////////////////////////////////////////////////////////////
// Wrappers

Array f_stream_get_wrappers() {
  return Stream::enumWrappers();
}

bool f_stream_is_local(const Variant& stream_or_url) {
  if (stream_or_url.isString()) {
    auto wrapper = Stream::getWrapperFromURI(stream_or_url.asCStrRef());
    return wrapper->m_isLocal;

  } else if (stream_or_url.isResource()) {
    File* file = dynamic_cast<File*>(stream_or_url.asCResRef().get());
    if (!file) {
      raise_warning("supplied resource is not a valid stream resource");
      return false;
    }
    return file->m_isLocal;
  }
  // Zend returns true for random data types...
  return true;
}


bool f_stream_register_wrapper(const String& protocol, const String& classname,
                               int flags) {
  return f_stream_wrapper_register(protocol, classname, flags);
}

bool f_stream_wrapper_register(const String& protocol, const String& classname,
                               int flags) {
  std::unique_ptr<Stream::Wrapper> wrapper;
  try {
    wrapper = std::unique_ptr<Stream::Wrapper>(
                   new UserStreamWrapper(protocol, classname, flags));
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

bool f_stream_wrapper_restore(const String& protocol) {
  return Stream::restoreWrapper(protocol);
}

bool f_stream_wrapper_unregister(const String& protocol) {
  return Stream::disableWrapper(protocol);
}

///////////////////////////////////////////////////////////////////////////////
// stream socket functions

static Socket *socket_accept_impl(const Resource& socket, struct sockaddr *addr,
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

       if (sl == sizeof(sa_family_t)) {
         /* unnamed socket. no text name. */
       } else if (ua->sun_path[0] == '\0') {
         /* abstract name. name is an arbitrary sequence of bytes. */
         int len = sl - sizeof(sa_family_t);
         textaddrlen = len;
         textaddr = (char *)malloc(len);
         memcpy(textaddr, ua->sun_path, len);
       } else {
         /* normal name. */
         textaddrlen = strlen(ua->sun_path);
         textaddr = strndup(ua->sun_path, textaddrlen);
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

Variant f_stream_socket_accept(const Resource& server_socket,
                               double timeout /* = -1.0 */,
                               VRefParam peername /* = null */) {
  Socket *sock = server_socket.getTyped<Socket>();
  pollfd p;
  int n;

  p.fd = sock->fd();
  p.events = (POLLIN|POLLERR|POLLHUP);
  p.revents = 0;
  IOStatusHelper io("socket_accept");
  if (timeout == -1) {
    timeout = ThreadInfo::s_threadInfo.getNoCheck()->
      m_reqInjectionData.getSocketDefaultTimeout();
  }
  n = poll(&p, 1, (uint64_t)(timeout * 1000.0));
  if (n > 0) {
    struct sockaddr sa;
    socklen_t salen = sizeof(sa);
    Socket *new_sock = socket_accept_impl(server_socket, &sa, &salen);
    peername = get_sockaddr_name(&sa, salen);
    if (new_sock) return Resource(new_sock);
  } else if (n < 0) {
    sock->setError(errno);
  } else {
    sock->setError(ETIMEDOUT);
  }
  return false;
}

Variant f_stream_socket_server(const String& local_socket,
                               VRefParam errnum /* = null */,
                               VRefParam errstr /* = null */,
                               int flags /* = 0 */,
                               const Resource& context /* = null_object */) {
  HostURL hosturl(static_cast<const std::string>(local_socket));
  return socket_server_impl(hosturl, flags, errnum, errstr);
}

Variant f_stream_socket_client(const String& remote_socket,
                               VRefParam errnum /* = null */,
                               VRefParam errstr /* = null */,
                               double timeout /* = -1.0 */,
                               int flags /* = 0 */,
                               const Resource& context /* = null_object */) {
  HostURL hosturl(static_cast<const std::string>(remote_socket));
  return sockopen_impl(hosturl, errnum, errstr, timeout, false);
}

Variant f_stream_socket_get_name(const Resource& handle, bool want_peer) {
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

Variant f_stream_socket_recvfrom(const Resource& socket, int length,
                                 int flags /* = 0 */,
                                 VRefParam address /* = null_string */) {
  Variant ret, host, port;
  Variant retval = f_socket_recvfrom(socket, ref(ret), length, flags,
                                     ref(host), ref(port));
  if (!same(retval, false) && retval.toInt64() >= 0) {
    Socket *sock = socket.getTyped<Socket>();
    if (sock->getType() == AF_INET6) {
      address = "[" + host.toString() + "]:" + port.toInt32();
    } else {
      address = host.toString() + ":" + port.toInt32();
    }
    return ret.toString(); // watch out, "ret", not "retval"
  }
  return false;
}

Variant f_stream_socket_sendto(const Resource& socket, const String& data,
                               int flags /* = 0 */,
                               const String& address /* = null_string */) {
  String host; int port;

  if (address == null_string) {
    Socket *sock = socket.getTyped<Socket>();
    host = sock->getAddress();
    port = sock->getPort();
  } else {
    HostURL hosturl(static_cast<std::string>(address));
    host = hosturl.getHost();
    port = hosturl.getPort();
  }

  return f_socket_sendto(socket, data, data.size(), flags, host, port);
}

bool f_stream_socket_shutdown(const Resource& stream, int how) {
  return f_socket_shutdown(stream, how);
}

static StreamContext* get_stream_context(const Resource& stream_or_context) {
  StreamContext* context = stream_or_context.getTyped<StreamContext>(true, true);
  if (context != nullptr) {
    return context;
  }
  File *file = stream_or_context.getTyped<File>(true, true);
  if (file != nullptr) {
    Resource resource = file->getStreamContext();
    if (file->getStreamContext().isNull()) {
      resource =
        Resource(NEWOBJ(StreamContext)(Array::Create(), Array::Create()));
      file->setStreamContext(resource);
    }
    return resource.getTyped<StreamContext>();
  }
  return nullptr;
}

bool StreamContext::validateOptions(const Variant& options) {
  if (options.isNull() || !options.isArray()) {
    return false;
  }
  const Array& arr = options.toArray();
  for (ArrayIter it(arr); it; ++it) {
    if (!it.first().isString() || !it.second().isArray()) {
      return false;
    }
    const Array& opts = it.second().toArray();
    for (ArrayIter it2(opts); it2; ++it2) {
      if (!it2.first().isString()) {
        return false;
      }
    }
  }
  return true;
}

void StreamContext::mergeOptions(const Array& options) {
  if (m_options.isNull()) {
    m_options = Array::Create();
  }
  for (ArrayIter it(options); it; ++it) {
    Variant wrapper = it.first();
    if (!m_options.exists(wrapper)) {
      m_options.set(wrapper, Array::Create());
    }
    assert(m_options[wrapper].isArray());
    Array& opts = m_options.lvalAt(wrapper).toArrRef();
    Array new_opts = it.second().toArray();
    for (ArrayIter it2(new_opts); it2; ++it2) {
      opts.set(it2.first(), it2.second());
    }
  }
}

void StreamContext::setOption(const String& wrapper,
                               const String& option,
                               const Variant& value) {
  if (m_options.isNull()) {
    m_options = Array::Create();
  }
  if (!m_options.exists(wrapper)) {
    m_options.set(wrapper, Array::Create());
  }
  assert(m_options[wrapper].isArray());
  Array& opts = m_options.lvalAt(wrapper).toArrRef();
  opts.set(option, value);
}

Array StreamContext::getOptions() const {
  if (m_options.isNull()) {
    return Array::Create();
  }
  return m_options;
}

bool StreamContext::validateParams(const Variant& params) {
  if (params.isNull() || !params.isArray()) {
    return false;
  }
  const Array& arr = params.toArray();
  const String& options_key = String::FromCStr("options");
  for (ArrayIter it(arr); it; ++it) {
    if (!it.first().isString()) {
      return false;
    }
    if (it.first().toString() == options_key) {
      if (!StreamContext::validateOptions(it.second())) {
        return false;
      }
    }
  }
  return true;
}

void StreamContext::mergeParams(const Array& params) {
  if (m_params.isNull()) {
    m_params = Array::Create();
  }
  const String& notification_key = String::FromCStr("notification");
  if (params.exists(notification_key)) {
    m_params.set(notification_key, params[notification_key]);
  }
  const String& options_key = String::FromCStr("options");
  if (params.exists(options_key)) {
    assert(params[options_key].isArray());
    mergeOptions(params[options_key].toArray());
  }
}

Array StreamContext::getParams() const {
  Array params = m_params;
  if (params.isNull()) {
    params = Array::Create();
  }
  const String& options_key = String::FromCStr("options");
  params.set(options_key, getOptions());
  return params;
}

///////////////////////////////////////////////////////////////////////////////

class StreamExtension : public Extension {
 public:
  StreamExtension() : Extension("stream") {}
  virtual void moduleInit() {
    HHVM_FE(stream_get_filters);
    HHVM_FE(stream_filter_register);
    HHVM_FE(stream_filter_append);
    HHVM_FE(stream_filter_prepend);
    HHVM_FE(stream_filter_remove);
    HHVM_FE(stream_bucket_make_writeable);
    HHVM_FE(stream_bucket_append);
    HHVM_FE(stream_bucket_prepend);
    loadSystemlib("stream-user-filters");
  }
} s_stream_extension;

///////////////////////////////////////////////////////////////////////////////
}
