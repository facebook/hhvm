/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/sockets/ext_sockets.h"
#include "hphp/runtime/ext/stream/ext_stream-user-filters.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/file-await.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/ssl-socket.h"
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
#include <sys/stat.h>
#if defined(AF_UNIX)
#include <sys/un.h>
#include <algorithm>
#endif

#define PHP_STREAM_COPY_ALL     (-1)



namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static
req::ptr<StreamContext> get_stream_context(const Variant& stream_or_context);

#define REGISTER_CONSTANT(name, value)                                         \
  Native::registerConstant<KindOfInt64>(makeStaticString(#name), value)        \

static class StreamExtension final : public Extension {
public:
  StreamExtension() : Extension("stream") {}
  void moduleInit() override {
    REGISTER_CONSTANT(STREAM_CLIENT_CONNECT, k_STREAM_CLIENT_CONNECT);
    REGISTER_CONSTANT(STREAM_CLIENT_ASYNC_CONNECT,
                      k_STREAM_CLIENT_ASYNC_CONNECT);
    REGISTER_CONSTANT(STREAM_CLIENT_PERSISTENT, k_STREAM_CLIENT_PERSISTENT);
    REGISTER_CONSTANT(STREAM_META_TOUCH, k_STREAM_META_TOUCH);
    REGISTER_CONSTANT(STREAM_META_OWNER_NAME, k_STREAM_META_OWNER_NAME);
    REGISTER_CONSTANT(STREAM_META_OWNER, k_STREAM_META_OWNER);
    REGISTER_CONSTANT(STREAM_META_GROUP_NAME, k_STREAM_META_GROUP_NAME);
    REGISTER_CONSTANT(STREAM_META_GROUP, k_STREAM_META_GROUP);
    REGISTER_CONSTANT(STREAM_META_ACCESS, k_STREAM_META_ACCESS);
    REGISTER_CONSTANT(STREAM_BUFFER_NONE, k_STREAM_BUFFER_NONE);
    REGISTER_CONSTANT(STREAM_BUFFER_LINE, k_STREAM_BUFFER_LINE);
    REGISTER_CONSTANT(STREAM_BUFFER_FULL, k_STREAM_BUFFER_FULL);
    REGISTER_CONSTANT(STREAM_SERVER_BIND, k_STREAM_SERVER_BIND);
    REGISTER_CONSTANT(STREAM_SERVER_LISTEN, k_STREAM_SERVER_LISTEN);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_SSLv23_CLIENT,
                      k_STREAM_CRYPTO_METHOD_SSLv23_CLIENT);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_SSLv23_SERVER,
                      k_STREAM_CRYPTO_METHOD_SSLv23_SERVER);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_SSLv2_CLIENT,
                      k_STREAM_CRYPTO_METHOD_SSLv2_CLIENT);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_SSLv2_SERVER,
                      k_STREAM_CRYPTO_METHOD_SSLv2_SERVER);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_SSLv3_CLIENT,
                      k_STREAM_CRYPTO_METHOD_SSLv3_CLIENT);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_SSLv3_SERVER,
                      k_STREAM_CRYPTO_METHOD_SSLv3_SERVER);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_TLS_CLIENT,
                      k_STREAM_CRYPTO_METHOD_TLS_CLIENT);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_TLS_SERVER,
                      k_STREAM_CRYPTO_METHOD_TLS_SERVER);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_TLSv1_0_CLIENT,
                      k_STREAM_CRYPTO_METHOD_TLSv1_0_CLIENT);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_TLSv1_0_SERVER,
                      k_STREAM_CRYPTO_METHOD_TLSv1_0_SERVER);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_TLSv1_1_CLIENT,
                      k_STREAM_CRYPTO_METHOD_TLSv1_1_CLIENT);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_TLSv1_1_SERVER,
                      k_STREAM_CRYPTO_METHOD_TLSv1_1_SERVER);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT,
                      k_STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_TLSv1_2_SERVER,
                      k_STREAM_CRYPTO_METHOD_TLSv1_2_SERVER);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_ANY_CLIENT,
                      k_STREAM_CRYPTO_METHOD_ANY_CLIENT);
    REGISTER_CONSTANT(STREAM_CRYPTO_METHOD_ANY_SERVER,
                      k_STREAM_CRYPTO_METHOD_ANY_SERVER);
    REGISTER_CONSTANT(STREAM_ENFORCE_SAFE_MODE, k_STREAM_ENFORCE_SAFE_MODE);
    REGISTER_CONSTANT(STREAM_IGNORE_URL, k_STREAM_IGNORE_URL);
    REGISTER_CONSTANT(STREAM_IPPROTO_ICMP, k_STREAM_IPPROTO_ICMP);
    REGISTER_CONSTANT(STREAM_IPPROTO_IP, k_STREAM_IPPROTO_IP);
    REGISTER_CONSTANT(STREAM_IPPROTO_RAW, k_STREAM_IPPROTO_RAW);
    REGISTER_CONSTANT(STREAM_IPPROTO_TCP, k_STREAM_IPPROTO_TCP);
    REGISTER_CONSTANT(STREAM_IPPROTO_UDP, k_STREAM_IPPROTO_UDP);
    REGISTER_CONSTANT(STREAM_IS_URL, k_STREAM_IS_URL);
    REGISTER_CONSTANT(STREAM_MKDIR_RECURSIVE, k_STREAM_MKDIR_RECURSIVE);
    REGISTER_CONSTANT(STREAM_MUST_SEEK, k_STREAM_MUST_SEEK);
    REGISTER_CONSTANT(STREAM_NOTIFY_AUTH_REQUIRED,
                      k_STREAM_NOTIFY_AUTH_REQUIRED);
    REGISTER_CONSTANT(STREAM_NOTIFY_AUTH_RESULT, k_STREAM_NOTIFY_AUTH_RESULT);
    REGISTER_CONSTANT(STREAM_NOTIFY_COMPLETED, k_STREAM_NOTIFY_COMPLETED);
    REGISTER_CONSTANT(STREAM_NOTIFY_CONNECT, k_STREAM_NOTIFY_CONNECT);
    REGISTER_CONSTANT(STREAM_NOTIFY_FAILURE, k_STREAM_NOTIFY_FAILURE);
    REGISTER_CONSTANT(STREAM_NOTIFY_FILE_SIZE_IS, k_STREAM_NOTIFY_FILE_SIZE_IS);
    REGISTER_CONSTANT(STREAM_NOTIFY_MIME_TYPE_IS, k_STREAM_NOTIFY_MIME_TYPE_IS);
    REGISTER_CONSTANT(STREAM_NOTIFY_PROGRESS, k_STREAM_NOTIFY_PROGRESS);
    REGISTER_CONSTANT(STREAM_NOTIFY_REDIRECTED, k_STREAM_NOTIFY_REDIRECTED);
    REGISTER_CONSTANT(STREAM_NOTIFY_RESOLVE, k_STREAM_NOTIFY_RESOLVE);
    REGISTER_CONSTANT(STREAM_NOTIFY_SEVERITY_ERR, k_STREAM_NOTIFY_SEVERITY_ERR);
    REGISTER_CONSTANT(STREAM_NOTIFY_SEVERITY_INFO,
                      k_STREAM_NOTIFY_SEVERITY_INFO);
    REGISTER_CONSTANT(STREAM_NOTIFY_SEVERITY_WARN,
                      k_STREAM_NOTIFY_SEVERITY_WARN);
    REGISTER_CONSTANT(STREAM_OOB, k_STREAM_OOB);
    REGISTER_CONSTANT(STREAM_PEEK, k_STREAM_PEEK);
    REGISTER_CONSTANT(STREAM_PF_INET, k_STREAM_PF_INET);
    REGISTER_CONSTANT(STREAM_PF_INET6, k_STREAM_PF_INET6);
    REGISTER_CONSTANT(STREAM_PF_UNIX, k_STREAM_PF_UNIX);
    REGISTER_CONSTANT(STREAM_REPORT_ERRORS, k_STREAM_REPORT_ERRORS);
    REGISTER_CONSTANT(STREAM_SHUT_RD, k_STREAM_SHUT_RD);
    REGISTER_CONSTANT(STREAM_SHUT_RDWR, k_STREAM_SHUT_RDWR);
    REGISTER_CONSTANT(STREAM_SHUT_WR, k_STREAM_SHUT_WR);
    REGISTER_CONSTANT(STREAM_SOCK_DGRAM, k_STREAM_SOCK_DGRAM);
    REGISTER_CONSTANT(STREAM_SOCK_RAW, k_STREAM_SOCK_RAW);
    REGISTER_CONSTANT(STREAM_SOCK_RDM, k_STREAM_SOCK_RDM);
    REGISTER_CONSTANT(STREAM_SOCK_SEQPACKET, k_STREAM_SOCK_SEQPACKET);
    REGISTER_CONSTANT(STREAM_SOCK_STREAM, k_STREAM_SOCK_STREAM);
    REGISTER_CONSTANT(STREAM_USE_PATH, k_STREAM_USE_PATH);

    REGISTER_CONSTANT(STREAM_AWAIT_READ, FileEventHandler::READ);
    REGISTER_CONSTANT(STREAM_AWAIT_WRITE, FileEventHandler::WRITE);
    REGISTER_CONSTANT(STREAM_AWAIT_READ_WRITE, FileEventHandler::READ_WRITE);

    REGISTER_CONSTANT(STREAM_AWAIT_ERROR, FileAwait::ERROR);
    REGISTER_CONSTANT(STREAM_AWAIT_TIMEOUT, FileAwait::TIMEOUT);
    REGISTER_CONSTANT(STREAM_AWAIT_READY, FileAwait::READY);
    REGISTER_CONSTANT(STREAM_AWAIT_CLOSED, FileAwait::CLOSED);

    REGISTER_CONSTANT(STREAM_URL_STAT_LINK, k_STREAM_URL_STAT_LINK);
    REGISTER_CONSTANT(STREAM_URL_STAT_QUIET, k_STREAM_URL_STAT_QUIET);

    HHVM_FE(stream_context_create);
    HHVM_FE(stream_context_get_options);
    HHVM_FE(stream_context_set_option);
    HHVM_FE(stream_context_get_default);
    HHVM_FE(stream_context_get_params);
    HHVM_FE(stream_context_set_params);
    HHVM_FE(stream_copy_to_stream);
    HHVM_FE(stream_get_contents);
    HHVM_FE(stream_get_line);
    HHVM_FE(stream_get_meta_data);
    HHVM_FE(stream_get_transports);
    HHVM_FE(stream_get_wrappers);
    HHVM_FE(stream_is_local);
    HHVM_FE(stream_register_wrapper);
    HHVM_FE(stream_wrapper_register);
    HHVM_FE(stream_wrapper_restore);
    HHVM_FE(stream_wrapper_unregister);
    HHVM_FE(stream_resolve_include_path);
    HHVM_FE(stream_select);
    HHVM_FE(stream_await);
    HHVM_FE(stream_set_blocking);
    HHVM_FE(stream_set_timeout);
    HHVM_FE(stream_set_write_buffer);
    HHVM_FE(set_file_buffer);
    HHVM_FE(stream_socket_accept);
    HHVM_FE(stream_socket_server);
    HHVM_FE(stream_socket_client);
    HHVM_FE(stream_socket_enable_crypto);
    HHVM_FE(stream_socket_get_name);
    HHVM_FE(stream_socket_pair);
    HHVM_FE(stream_socket_recvfrom);
    HHVM_FE(stream_socket_sendto);
    HHVM_FE(stream_socket_shutdown);

    loadSystemlib();
  }
} s_stream_extension;

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(stream_context_create,
                      const Variant& options /* = null_variant */,
                      const Variant& params /* = null_variant */) {
  const Array& arrOptions = options.isNull() ? null_array : options.toArray();
  const Array& arrParams = params.isNull() ? null_array : params.toArray();

  if (!arrOptions.isNull() && !StreamContext::validateOptions(arrOptions)) {
    raise_warning("options should have the form "
                  "[\"wrappername\"][\"optionname\"] = $value");
    return Variant(
      req::make<StreamContext>(HPHP::null_array, HPHP::null_array));
  }
  return Variant(req::make<StreamContext>(arrOptions, arrParams));
}

Variant HHVM_FUNCTION(stream_context_get_options,
                      const Resource& stream_or_context) {
  auto context = get_stream_context(stream_or_context);
  if (!context) {
    raise_warning("Invalid stream/context parameter");
    return false;
  }
  return context->getOptions();
}

static bool stream_context_set_option0(const req::ptr<StreamContext>& context,
                                       const Array& options) {
  if (!StreamContext::validateOptions(options)) {
    raise_warning("options should have the form "
                  "[\"wrappername\"][\"optionname\"] = $value");
    return false;
  }
  context->mergeOptions(options);
  return true;
}

static bool stream_context_set_option1(const req::ptr<StreamContext>& context,
                                       const String& wrapper,
                                       const String& option,
                                       const Variant& value) {
  context->setOption(wrapper, option, value);
  return true;
}

bool HHVM_FUNCTION(stream_context_set_option,
                   const Variant& stream_or_context,
                   const Variant& wrapper_or_options,
                   const Variant& option /* = null_variant */,
                   const Variant& value /* = null_variant */) {
  auto context = get_stream_context(stream_or_context);
  if (!context) {
    raise_warning("Invalid stream/context parameter");
    return false;
  }
  if (wrapper_or_options.isArray() &&
      !option.isInitialized() &&
      !value.isInitialized()) {
    return stream_context_set_option0(context, wrapper_or_options.toArray());
  } else if (wrapper_or_options.isString() &&
             option.isInitialized() &&
             option.isString() &&
             value.isInitialized()) {
    return stream_context_set_option1(context, wrapper_or_options.toString(),
                                      option.toString(), value);
  } else {
    raise_warning("called with wrong number or type of parameters; please RTM");
    return false;
  }
}

Variant HHVM_FUNCTION(stream_context_get_default,
                      const Variant& options /* = null_variant */) {
  const Array& arrOptions = options.isNull() ? null_array : options.toArray();
  auto context = g_context->getStreamContext();
  if (!context) {
    context = req::make<StreamContext>(Array::Create(), Array::Create());
    g_context->setStreamContext(context);
  }
  if (!arrOptions.isNull() &&
      !stream_context_set_option0(context, arrOptions)) {
    return false;
  }
  return Variant(std::move(context));
}

Variant HHVM_FUNCTION(stream_context_set_default,
                      const Array& options) {
  return HHVM_FN(stream_context_get_default)(options);
}

Variant HHVM_FUNCTION(stream_context_get_params,
                      const Resource& stream_or_context) {
  auto context = get_stream_context(stream_or_context);
  if (!context) {
    raise_warning("Invalid stream/context parameter");
    return false;
  }
  return context->getParams();
}

bool HHVM_FUNCTION(stream_context_set_params,
                   const Resource& stream_or_context,
                   const Array& params) {
  auto context = get_stream_context(stream_or_context);
  if (!context || !StreamContext::validateParams(params)) {
    raise_warning("Invalid stream/context parameter");
    return false;
  }
  context->mergeParams(params);
  return true;
}

Variant HHVM_FUNCTION(stream_copy_to_stream,
                      const Resource& source,
                      const Resource& dest,
                      int maxlength /* = -1 */,
                      int offset /* = 0 */) {
  if (maxlength == 0) return 0;
  if (maxlength == PHP_STREAM_COPY_ALL) maxlength = 0;

  auto srcFile = cast<File>(source);
  auto destFile = cast<File>(dest);
  if (maxlength < 0) {
    throw_invalid_argument("maxlength: %d", maxlength);
    return false;
  }
  if (offset > 0 && !srcFile->seek(offset, SEEK_SET) ) {
    raise_warning("Failed to seek to position %d in the stream", offset);
    return false;
  }
  int cbytes = 0;
  if (maxlength == 0) maxlength = INT_MAX;
  while (cbytes < maxlength) {
    int remaining = maxlength - cbytes;
    String buf = srcFile->read(std::min(remaining, File::CHUNK_SIZE));
    if (buf.size() == 0) break;
    if (destFile->write(buf) != buf.size()) {
      return false;
    }
    cbytes += buf.size();
  }

  return cbytes;
}

Variant HHVM_FUNCTION(stream_get_contents,
                      const Resource& handle,
                      int maxlen /* = -1 */,
                      int offset /* = -1 */) {
  if (maxlen < -1) {
    throw_invalid_argument("maxlen: %d", maxlen);
    return false;
  }

  if (maxlen == 0) {
    return init_null();
  }

  auto file = dyn_cast<File>(handle);
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

Variant HHVM_FUNCTION(stream_get_line,
                      const Resource& handle,
                      int length /* = 0 */,
                      const Variant& ending /* = null_variant */) {
  const String& strEnding = ending.isNull() ? null_string : ending.toString();
  return cast<File>(handle)->readRecord(strEnding, length);
}

Variant HHVM_FUNCTION(stream_get_meta_data,
                      const Resource& stream) {
  if (auto f = dyn_cast_or_null<File>(stream)) {
    return f->getMetaData();
  }
  if (auto d = dyn_cast_or_null<Directory>(stream)) {
    return d->getMetaData();
  }
  return false;
}

Array HHVM_FUNCTION(stream_get_transports) {
  return make_packed_array("tcp", "udp", "unix", "udg");
}

Variant HHVM_FUNCTION(stream_resolve_include_path,
                      const String& filename,
                      const Variant& context /* = null_variant */) {
  struct stat s;
  String ret = resolveVmInclude(filename.get(), "", &s, true);
  if (ret.isNull()) {
    return false;
  }
  return ret;
}

Variant HHVM_FUNCTION(stream_select,
                      VRefParam read,
                      VRefParam write,
                      VRefParam except,
                      const Variant& vtv_sec,
                      int tv_usec /* = 0 */) {
  return HHVM_FN(socket_select)(ref(read), ref(write), ref(except),
                                vtv_sec, tv_usec);
}

Object HHVM_FUNCTION(stream_await,
                     const Resource& stream,
                     int64_t events,
                     double timeout /*= 0.0 */) {
  return cast<File>(stream)->await((uint16_t)events, timeout);
}

bool HHVM_FUNCTION(stream_set_blocking,
                   const Resource& stream,
                   int mode) {
  auto file = cast<File>(stream);
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

bool HHVM_FUNCTION(stream_set_timeout,
                   const Resource& stream,
                   int seconds,
                   int microseconds /* = 0 */) {
  if (isa<Socket>(stream)) {
    return HHVM_FN(socket_set_option)
      (stream, SOL_SOCKET, SO_RCVTIMEO,
       make_map_array(s_sec, seconds, s_usec, microseconds));
  }
  return false;
}

int64_t HHVM_FUNCTION(stream_set_write_buffer,
                      const Resource& stream,
                      int buffer) {
  auto plain_file = dyn_cast<PlainFile>(stream);
  if (!plain_file) {
    return -1;
  }
  FILE* file = plain_file->getStream();
  if (!file) {
    return -1;
  }

  switch (buffer) {
  case k_STREAM_BUFFER_NONE:
    return setvbuf(file, nullptr, _IONBF, 0);
  case k_STREAM_BUFFER_LINE:
    return setvbuf(file, nullptr, _IOLBF, BUFSIZ);
  case k_STREAM_BUFFER_FULL:
    return setvbuf(file, nullptr, _IOFBF, BUFSIZ);
  default:
    return -1;
  }
}

int64_t HHVM_FUNCTION(set_file_buffer,
                      const Resource& stream,
                      int buffer) {
  return HHVM_FN(stream_set_write_buffer)(stream, buffer);
}

///////////////////////////////////////////////////////////////////////////////
// Wrappers

Array HHVM_FUNCTION(stream_get_wrappers) {
  return Stream::enumWrappers();
}

bool HHVM_FUNCTION(stream_is_local,
                   const Variant& stream_or_url) {
  if (stream_or_url.isString()) {
    auto wrapper = Stream::getWrapperFromURI(stream_or_url.asCStrRef());
    return wrapper ? wrapper->m_isLocal : false;

  } else if (stream_or_url.isResource()) {
    auto file = dyn_cast_or_null<File>(stream_or_url);
    if (!file) {
      raise_warning("supplied resource is not a valid stream resource");
      return false;
    }
    return file->isLocal();
  }
  // Zend returns true for random data types...
  return true;
}


bool HHVM_FUNCTION(stream_register_wrapper,
                   const String& protocol,
                   const String& classname,
                   int flags) {
  return HHVM_FN(stream_wrapper_register)(protocol, classname, flags);
}

bool HHVM_FUNCTION(stream_wrapper_register,
                   const String& protocol,
                   const String& classname,
                   int flags) {
  auto const cls = Unit::loadClass(classname.get());
  if (!cls) {
    raise_warning("Undefined class: '%s'", classname.data());
    return false;
  }

  auto wrapper = std::unique_ptr<Stream::Wrapper>(
    new UserStreamWrapper(protocol, cls, flags));
  if (!Stream::registerRequestWrapper(protocol, std::move(wrapper))) {
    raise_warning("Unable to register protocol: %s\n", protocol.data());
    return false;
  }
  return true;
}

bool HHVM_FUNCTION(stream_wrapper_restore,
                   const String& protocol) {
  return Stream::restoreWrapper(protocol);
}

bool HHVM_FUNCTION(stream_wrapper_unregister,
                   const String& protocol) {
  return Stream::disableWrapper(protocol);
}

///////////////////////////////////////////////////////////////////////////////
// stream socket functions

static req::ptr<Socket> socket_accept_impl(
  const Resource& socket,
  struct sockaddr *addr,
  socklen_t *addrlen
) {
  auto sock = cast<Socket>(socket);
  auto new_sock = req::make<Socket>(
    accept(sock->fd(), addr, addrlen), sock->getType());
  if (!new_sock->valid()) {
    SOCKET_ERROR(new_sock, "unable to accept incoming connection", errno);
    new_sock.reset();
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

Variant HHVM_FUNCTION(stream_socket_accept,
                      const Resource& server_socket,
                      double timeout /* = -1.0 */,
                      VRefParam peername /* = null */) {
  auto sock = cast<Socket>(server_socket);
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
    auto new_sock = socket_accept_impl(server_socket, &sa, &salen);
    peername = get_sockaddr_name(&sa, salen);
    if (new_sock) return Resource(new_sock);
  } else if (n < 0) {
    sock->setError(errno);
  } else {
    sock->setError(ETIMEDOUT);
  }
  return false;
}

Variant HHVM_FUNCTION(stream_socket_server,
                      const String& local_socket,
                      VRefParam errnum /* = null */,
                      VRefParam errstr /* = null */,
                      int flags /* = 0 */,
                      const Variant& context /* = null_variant */) {
  HostURL hosturl(static_cast<const std::string>(local_socket));
  return socket_server_impl(hosturl, flags, errnum, errstr);
}

Variant HHVM_FUNCTION(stream_socket_client,
                      const String& remote_socket,
                      VRefParam errnum /* = null */,
                      VRefParam errstr /* = null */,
                      double timeout /* = -1.0 */,
                      int flags /* = 0 */,
                      const Variant& context /* = null_variant */) {
  HostURL hosturl(static_cast<const std::string>(remote_socket));
  return sockopen_impl(hosturl, errnum, errstr, timeout, false, context);
}

bool HHVM_FUNCTION(stream_socket_enable_crypto,
                   const Resource& socket,
                   bool enable,
                   int cryptotype,
                   const Variant& sessionstream) {
  auto sock = cast<SSLSocket>(socket);
  if (!enable) {
    return sock->disableCrypto();
  }

  if (!sessionstream.isNull()) {
    raise_warning("stream_socket_enable_crypto(): HHVM does not yet support "
                  "the session_stream parameter");
    return false;
  }

  if (!cryptotype) {
    raise_warning("stream_socket_enable_crypto(): When enabling encryption you "
                  "must specify the crypto type");
    return false;
  }

  SSLSocket::CryptoMethod crypto;
  switch (cryptotype) {
    case k_STREAM_CRYPTO_METHOD_SSLv2_CLIENT:
      crypto = SSLSocket::CryptoMethod::ClientSSLv2;
      break;
    case k_STREAM_CRYPTO_METHOD_SSLv3_CLIENT:
      crypto = SSLSocket::CryptoMethod::ClientSSLv3;
      break;
    case k_STREAM_CRYPTO_METHOD_SSLv23_CLIENT:
      crypto = SSLSocket::CryptoMethod::ClientSSLv23;
      break;
    case k_STREAM_CRYPTO_METHOD_TLS_CLIENT:
      crypto = SSLSocket::CryptoMethod::ClientTLS;
      break;
    case k_STREAM_CRYPTO_METHOD_SSLv2_SERVER:
    case k_STREAM_CRYPTO_METHOD_SSLv3_SERVER:
    case k_STREAM_CRYPTO_METHOD_SSLv23_SERVER:
    case k_STREAM_CRYPTO_METHOD_TLS_SERVER:
      raise_warning(
        "HHVM does not yet support SSL/TLS servers implemented in PHP");
      return false;
    default:
     return false;
  }

  if (
    cryptotype != k_STREAM_CRYPTO_METHOD_TLS_CLIENT
    && cryptotype != k_STREAM_CRYPTO_METHOD_TLS_SERVER
  ) {
    // Not done by PHP5/7, but using SSL nowadays is a very bad idea.
    raise_warning(
      "stream_socket_enable_crypto(): SSL is flawed and vulnerable; "
      "Migrate to TLS as soon as possible."
    );
  }

  return sock->enableCrypto(crypto);
}

Variant HHVM_FUNCTION(stream_socket_get_name,
                      const Resource& handle,
                      bool want_peer) {
  Variant address, port;
  bool ret;
  if (want_peer) {
    ret = HHVM_FN(socket_getpeername)(handle, ref(address), ref(port));
  } else {
    ret = HHVM_FN(socket_getsockname)(handle, ref(address), ref(port));
  }
  if (ret) {
    return address.toString() + ":" + port.toString();
  }
  return false;
}

Variant HHVM_FUNCTION(stream_socket_pair,
                      int domain,
                      int type,
                      int protocol) {
  Variant fd;
  if (!HHVM_FN(socket_create_pair)(domain, type, protocol, ref(fd))) {
    return false;
  }
  return fd;
}

Variant HHVM_FUNCTION(stream_socket_recvfrom,
                      const Resource& socket,
                      int length,
                      int flags /* = 0 */,
                      VRefParam address /* = null_string */) {
  Variant ret, host, port;
  Variant retval = HHVM_FN(socket_recvfrom)(socket, ref(ret), length, flags,
                                            ref(host), ref(port));
  if (!same(retval, false) && retval.toInt64() >= 0) {
    auto sock = cast<Socket>(socket);
    if (sock->getType() == AF_INET6) {
      address = "[" + host.toString() + "]:" + port.toInt32();
    } else {
      address = host.toString() + ":" + port.toInt32();
    }
    return ret.toString(); // watch out, "ret", not "retval"
  }
  return false;
}

Variant HHVM_FUNCTION(stream_socket_sendto,
                      const Resource& socket,
                      const String& data,
                      int flags /* = 0 */,
                      const Variant& address /* = null_variant */) {
  String host; int port;
  const String& strAddress = address.isNull()
                           ? null_string
                           : address.toString();

  if (strAddress == null_string) {
    auto sock = cast<Socket>(socket);
    host = sock->getAddress();
    port = sock->getPort();
  } else {
    HostURL hosturl(static_cast<std::string>(strAddress));
    host = hosturl.getHost();
    port = hosturl.getPort();
  }

  return HHVM_FN(socket_sendto)(socket, data, data.size(), flags, host, port);
}

bool HHVM_FUNCTION(stream_socket_shutdown,
                   const Resource& stream,
                   int how) {
  return HHVM_FN(socket_shutdown)(stream, how);
}

static
req::ptr<StreamContext> get_stream_context(const Variant& stream_or_context) {
  if (!stream_or_context.isResource()) {
    return nullptr;
  }
  const Resource& resource = stream_or_context.asCResRef();
  auto context = dyn_cast_or_null<StreamContext>(resource);
  if (context) return context;
  auto file = dyn_cast_or_null<File>(resource);
  if (file != nullptr) {
    auto context = file->getStreamContext();
    if (!file->getStreamContext()) {
      context = req::make<StreamContext>(Array::Create(), Array::Create());
      file->setStreamContext(context);
    }
    return context;
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
    return empty_array();
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
}
