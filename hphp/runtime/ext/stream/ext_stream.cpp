/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/file-await.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/ssl-socket.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/sockets/ext_sockets.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/network.h"

#include <algorithm>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>

#include <folly/portability/Sockets.h>
#include <folly/portability/Unistd.h>

#if defined(AF_UNIX)
#include <sys/un.h>
#endif

#define PHP_STREAM_COPY_ALL     (-1)

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static
req::ptr<StreamContext> get_stream_context(const Variant& stream_or_context);

static struct StreamExtension final : Extension {
  StreamExtension() : Extension("stream", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleInit() override;
} s_stream_extension;

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(stream_context_create,
                      const Variant& options /* = uninit_variant */,
                      const Variant& params /* = uninit_variant */) {
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
                      const OptResource& stream_or_context) {
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
                   const Variant& option /* = uninit_variant */,
                   const Variant& value /* = uninit_variant */) {
  auto context = get_stream_context(stream_or_context);
  if (!context) {
    raise_warning("Invalid stream/context parameter");
    return false;
  }
  if (wrapper_or_options.isArray() &&
      option.isNull() &&
      value.isNull()) {
    return stream_context_set_option0(context, wrapper_or_options.toArray());
  } else if (wrapper_or_options.isString() &&
             !option.isNull() &&
             option.isString() &&
             !value.isNull()) {
    return stream_context_set_option1(context, wrapper_or_options.toString(),
                                      option.toString(), value);
  } else {
    raise_warning("called with wrong number or type of parameters; please RTM");
    return false;
  }
}

Variant HHVM_FUNCTION(stream_context_get_default,
                      const Variant& options /* = uninit_variant */) {
  const Array& arrOptions = options.isNull() ? null_array : options.toArray();
  auto context = g_context->getStreamContext();
  if (!context) {
    context = req::make<StreamContext>(empty_dict_array(), empty_dict_array());
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
                      const OptResource& stream_or_context) {
  auto context = get_stream_context(stream_or_context);
  if (!context) {
    raise_warning("Invalid stream/context parameter");
    return false;
  }
  return context->getParams();
}

bool HHVM_FUNCTION(stream_context_set_params,
                   const OptResource& stream_or_context,
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
                      const OptResource& source,
                      const OptResource& dest,
                      int64_t maxlength /* = -1 */,
                      int64_t offset /* = 0 */) {
  if (maxlength == 0) return 0;

  auto srcFile = cast<File>(source);
  auto destFile = cast<File>(dest);
  if (maxlength < 0 && maxlength != PHP_STREAM_COPY_ALL) {
    raise_invalid_argument_warning("maxlength: %ld", maxlength);
    return false;
  }
  if (offset > 0 && !srcFile->seek(offset, SEEK_SET) ) {
    raise_warning("Failed to seek to position %ld in the stream", offset);
    return false;
  }
  int64_t cbytes = 0;

  if (maxlength == PHP_STREAM_COPY_ALL) {
    while (!srcFile->eof()) {
      // We read 0x10000 (64 KB at a time) because the size of a StringBuffer
      // is limited.
      String buf = srcFile->read(0x10000);

      if (buf.size() == 0) break;
      if (destFile->write(buf) != buf.size()) {
        return false;
      }

      cbytes += buf.size();
    }
  } else {
    while (cbytes < maxlength) {
      int64_t remaining = maxlength - cbytes;
      //srcFile->getChunkSize currently returns an int64_t
      auto chunkSize = srcFile->getChunkSize();
      String buf = srcFile->read(std::min(remaining, chunkSize));
      if (buf.size() == 0) break;
      if (destFile->write(buf) != buf.size()) {
        return false;
      }
      cbytes += buf.size();
    }
  }

  return cbytes;
}

Variant HHVM_FUNCTION(stream_get_contents,
                      const OptResource& handle,
                      int64_t maxlen /* = -1 */,
                      int64_t offset /* = -1 */) {
  if (maxlen < -1) {
    raise_invalid_argument_warning("maxlen: %ld", maxlen);
    return false;
  }

  if (maxlen == 0) {
    return init_null();
  }

  auto file = dyn_cast<File>(handle);
  if (!file) {
    raise_invalid_argument_warning(
      "stream_get_contents() expects parameter 1 to be a resource");
    return false;
  }

  if (offset >= 0 && !file->seek(offset, SEEK_SET) ) {
    raise_warning("Failed to seek to position %ld in the stream", offset);
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
                      const OptResource& handle,
                      int64_t length /* = 0 */,
                      const Variant& ending /* = uninit_variant */) {
  const String& strEnding = ending.isNull() ? null_string : ending.toString();
  return cast<File>(handle)->readRecord(strEnding, length);
}

Variant HHVM_FUNCTION(stream_get_meta_data,
                      const OptResource& stream) {
  if (auto f = dyn_cast_or_null<File>(stream)) {
    if (!f->isClosed()) return f->getMetaData();
  }
  if (auto d = dyn_cast_or_null<Directory>(stream)) {
    return d->getMetaData();
  }
  return false;
}

Array HHVM_FUNCTION(stream_get_transports) {
  return make_vec_array("tcp", "udp", "unix", "udg", "ssl", "tls");
}

Variant HHVM_FUNCTION(stream_resolve_include_path, const String& filename,
                      const Variant& /*context*/ /* = uninit_variant */) {
  if (!FileUtil::checkPathAndWarn(filename, __FUNCTION__ + 2, 1)) {
    return init_null();
  }

  struct stat s;
  String ret = resolveVmInclude(filename.get(), "", &s, /*allow_dir*/true);
  if (ret.isNull()) {
    return false;
  }
  return ret;
}

Variant HHVM_FUNCTION(stream_select,
                      Variant& read,
                      Variant& write,
                      Variant& except,
                      const Variant& vtv_sec,
                      int64_t tv_usec /* = 0 */) {
  return HHVM_FN(socket_select)(read, write, except,
                                vtv_sec, tv_usec);
}

Object HHVM_FUNCTION(stream_await,
                     const OptResource& stream,
                     int64_t events,
                     double timeout /*= 0.0 */) {
  return cast<File>(stream)->await((uint16_t)events, timeout);
}

bool HHVM_FUNCTION(stream_set_blocking,
                   const OptResource& stream,
                   bool mode) {
  if (isa<File>(stream)) {
    return cast<File>(stream)->setBlocking(mode);
  } else {
    return false;
  }
}

int64_t HHVM_FUNCTION(stream_set_read_buffer,
                      const OptResource& stream,
                      int64_t buffer) {
  if (buffer < 0) return -1;
  if (isa<File>(stream)) {
    auto plain_file = dyn_cast<PlainFile>(stream);
    if (!plain_file) {
      return -1;
    }
    FILE* file = plain_file->getStream();
    if (!file) {
      return -1;
    }
    if (buffer == 0) {
      // Use _IONBF (no buffer) macro to set no buffer
      return setvbuf(file, nullptr, _IONBF, 0);
    } else {
      // Use _IOFBF (full buffer) macro
      return setvbuf(file, nullptr, _IOFBF, (size_t)buffer);
    }
  } else {
    return -1;
  }
}

Variant HHVM_FUNCTION(stream_set_chunk_size,
                      const OptResource& stream,
                      int64_t chunk_size) {
  if (isa<File>(stream) && chunk_size > 0) {
    auto file = cast<File>(stream);
    int64_t orig_chunk_size = file->getChunkSize();
    file->setChunkSize(chunk_size);
    return orig_chunk_size;
  }
  return false;
}

const StaticString
  s_sec("sec"),
  s_usec("usec"),
  s_options("options"),
  s_notification("notification");

bool HHVM_FUNCTION(stream_set_timeout,
                   const OptResource& stream,
                   int64_t seconds,
                   int64_t microseconds /* = 0 */) {
  if (isa<Socket>(stream)) {
    return HHVM_FN(socket_set_option)
      (stream, SOL_SOCKET, SO_RCVTIMEO,
       make_dict_array(s_sec, seconds, s_usec, microseconds));
  } else if (isa<File>(stream)) {
    return cast<File>(stream)->setTimeout(
      (uint64_t)seconds * 1000000 + microseconds);
  } else {
    return false;
  }
}

int64_t HHVM_FUNCTION(stream_set_write_buffer,
                      const OptResource& stream,
                      int64_t buffer) {
  if (buffer < 0) return -1;
  auto plain_file = dyn_cast<PlainFile>(stream);
  if (!plain_file) {
    return -1;
  }
  FILE* file = plain_file->getStream();
  if (!file) {
    return -1;
  }
  if (buffer ==0) {
    // Use _IONBF (no buffer) macro to set no buffer
    return setvbuf(file, nullptr, _IONBF, 0);
  } else {
  // Use _IOFBF (full buffer) macro
    return setvbuf(file, nullptr, _IOFBF, (size_t)buffer);
  }
}

int64_t HHVM_FUNCTION(set_file_buffer,
                      const OptResource& stream,
                      int64_t buffer) {
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

///////////////////////////////////////////////////////////////////////////////
// stream socket functions

static Variant socket_accept_impl(
  const OptResource& socket,
  struct sockaddr *addr,
  socklen_t *addrlen
) {
  req::ptr<Socket> new_sock;
  req::ptr<SSLSocket> sslsock;
  if (isa<SSLSocket>(socket)) {
    auto sock = cast<SSLSocket>(socket);
    auto new_fd = accept(sock->fd(), addr, addrlen);
    double timeout = RequestInfo::s_requestInfo.getNoCheck()->
      m_reqInjectionData.getSocketDefaultTimeout();
    sslsock = SSLSocket::Create(new_fd, sock->getType(),
                                sock->getCryptoMethod(), sock->getAddress(),
                                sock->getPort(), timeout,
                                sock->getStreamContext());
    new_sock = sslsock;
  } else {
    auto sock = cast<Socket>(socket);
    auto new_fd = accept(sock->fd(), addr, addrlen);
    new_sock = req::make<StreamSocket>(new_fd, sock->getType());
  }

  if (!new_sock->valid()) {
    SOCKET_ERROR(new_sock, "unable to accept incoming connection", errno);
    new_sock.reset();
  }

  if (sslsock && !sslsock->onAccept()) {
    raise_warning("Failed to enable crypto");
    return false;
  }

  return Variant(std::move(new_sock));
}

static String get_sockaddr_name(struct sockaddr *sa, socklen_t sl) {
  char abuf[256];
  char* buf = nullptr;
  char textaddr[1024] = {'\0'};
  int textaddrlen = 0;

  switch (sa->sa_family) {
  case AF_INET:
    buf = inet_ntoa(((struct sockaddr_in*)sa)->sin_addr);
    if (buf) {
      textaddrlen = snprintf(textaddr, sizeof(textaddr), "%s:%d",
                             buf, ntohs(((struct sockaddr_in*)sa)->sin_port));
    }
    break;

   case AF_INET6:
    buf = (char*)inet_ntop(sa->sa_family,
                           &((struct sockaddr_in6*)sa)->sin6_addr,
                           (char *)&abuf, sizeof(abuf));
    if (buf) {
      textaddrlen = snprintf(textaddr, sizeof(textaddr), "%s:%d",
                             buf, ntohs(((struct sockaddr_in6*)sa)->sin6_port));
    }
    break;

   case AF_UNIX: {
     auto ua = (struct sockaddr_un*)sa;

     if (sl == sizeof(sa_family_t)) {
       /* unnamed socket. no text name. */
     } else if (ua->sun_path[0] == '\0') {
       /* abstract name. name is an arbitrary sequence of bytes. */
       int len = sl - sizeof(sa_family_t);
       textaddrlen = std::min(len, (int)sizeof(textaddr) - 1);
       memcpy(textaddr, ua->sun_path, textaddrlen);
     } else {
       /* normal name. */
       textaddrlen = std::min(sizeof(textaddr), strlen(ua->sun_path));
       memcpy(textaddr, ua->sun_path, textaddrlen);
     }
     break;
   }

  default:
    break;
  }

  if (textaddrlen) return String(textaddr, textaddrlen, CopyString);
  return String();
}

Variant HHVM_FUNCTION(stream_socket_accept,
                      const OptResource& server_socket,
                      double timeout,
                      Variant& peername) {
  auto sock = cast<Socket>(server_socket);
  pollfd p;
  int n;

  p.fd = sock->fd();
  p.events = (POLLIN|POLLERR|POLLHUP);
  p.revents = 0;
  IOStatusHelper io("socket_accept");
  if (timeout < 0.0) {
    timeout = RequestInfo::s_requestInfo.getNoCheck()->
      m_reqInjectionData.getSocketDefaultTimeout();
  }
  n = poll(&p, 1, (uint64_t)(timeout * 1000.0));
  if (n > 0) {
    struct sockaddr sa;
    socklen_t salen = sizeof(sa);
    auto new_sock = socket_accept_impl(server_socket, &sa, &salen);
    peername = get_sockaddr_name(&sa, salen);
    return new_sock;
  } else if (n < 0) {
    sock->setError(errno);
  } else {
    sock->setError(ETIMEDOUT);
  }
  return false;
}

Variant HHVM_FUNCTION(stream_socket_server,
                      const String& local_socket,
                      Variant& errnum,
                      Variant& errstr,
                      int64_t flags /* = 0 */,
                      const Variant& context /* = uninit_variant */) {
  HostURL hosturl(static_cast<const std::string>(local_socket));
  return socket_server_impl(hosturl, flags, errnum, errstr, context);
}

Variant HHVM_FUNCTION(stream_socket_client,
                      const String& remote_socket,
                      Variant& errnum,
                      Variant& errstr,
                      double timeout /* = -1.0 */,
                      int64_t flags /* = 0 */,
                      const Variant& context /* = uninit_variant */) {
  HostURL hosturl(static_cast<const std::string>(remote_socket));
  bool persistent = (flags & k_STREAM_CLIENT_PERSISTENT) ==
    k_STREAM_CLIENT_PERSISTENT;

  return sockopen_impl(hosturl, errnum, errstr, timeout, persistent, context);
}

bool HHVM_FUNCTION(stream_socket_enable_crypto,
                   const OptResource& socket,
                   bool enable,
                   int64_t cryptotype,
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
    case k_STREAM_CRYPTO_METHOD_TLS_CLIENT:
      crypto = SSLSocket::CryptoMethod::ClientTLS;
      break;
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
                      const OptResource& handle,
                      bool want_peer) {
  Variant address, port;
  bool ret;
  if (want_peer) {
    ret = HHVM_FN(socket_getpeername)(handle, address, port);
  } else {
    ret = HHVM_FN(socket_getsockname)(handle, address, port);
  }
  if (ret && port.isInteger()) {
    return address.toString() + ":" + port.toString();
  } else if (ret) {
    return address.toString();
  }
  return false;
}

Variant HHVM_FUNCTION(stream_socket_pair,
                      int64_t domain,
                      int64_t type,
                      int64_t protocol) {
  Variant fd;
  if (!socket_create_pair_impl(domain, type, protocol, fd, true)) {
    return false;
  }
  return fd;
}

Variant HHVM_FUNCTION(stream_socket_recvfrom,
                      const OptResource& socket,
                      int64_t length,
                      int64_t flags,
                      Variant& address) {
  Variant ret, host, port;
  Variant retval = HHVM_FN(socket_recvfrom)(socket, ret, length, flags,
                                            host, port);
  if (!same(retval, false) && retval.toInt64() >= 0) {
    auto sock = cast<Socket>(socket);
    if (sock->getType() == AF_INET6) {
      address = "[" + host.toString() + "]:" + (int)port.toInt64();
    } else {
      address = host.toString() + ":" + (int)port.toInt64();
    }
    return ret.toString(); // watch out, "ret", not "retval"
  }
  return false;
}

Variant HHVM_FUNCTION(stream_socket_sendto,
                      const OptResource& socket,
                      const String& data,
                      int64_t flags /* = 0 */,
                      const Variant& address /* = uninit_variant */) {
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
                   const OptResource& stream,
                   int64_t how) {
  return HHVM_FN(socket_shutdown)(stream, how);
}

static
req::ptr<StreamContext> get_stream_context(const Variant& stream_or_context) {
  if (!stream_or_context.isResource()) {
    return nullptr;
  }
  const OptResource& resource = stream_or_context.asCResRef();
  auto context = dyn_cast_or_null<StreamContext>(resource);
  if (context) return context;
  auto file = dyn_cast_or_null<File>(resource);
  if (file != nullptr) {
    auto context = file->getStreamContext();
    if (!context) {
      context = req::make<StreamContext>(empty_dict_array(), empty_dict_array());
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
    m_options = Array::CreateDict();
  }
  for (ArrayIter it(options); it; ++it) {
    Variant wrapper = it.first();
    if (!m_options.exists(wrapper)) {
      m_options.set(wrapper, Array::CreateDict());
    }
    assertx(m_options[wrapper].isArray());
    Array& opts = asArrRef(m_options.lval(wrapper));
    const Array& new_opts = it.second().toArray();
    for (ArrayIter it2(new_opts); it2; ++it2) {
      opts.set(it2.first(), it2.second());
    }
  }
}

void StreamContext::setOption(const String& wrapper,
                               const String& option,
                               const Variant& value) {
  if (m_options.isNull()) {
    m_options = Array::CreateDict();
  }
  if (!m_options.exists(wrapper)) {
    m_options.set(wrapper, Array::CreateDict());
  }
  assertx(m_options[wrapper].isArray());
  Array& opts = asArrRef(m_options.lval(wrapper));
  opts.set(option, value);
}

Array StreamContext::getOptions() const {
  if (m_options.isNull()) {
    return empty_dict_array();
  }
  return m_options;
}

bool StreamContext::validateParams(const Variant& params) {
  if (params.isNull() || !params.isArray()) {
    return false;
  }
  const Array& arr = params.toArray();
  for (ArrayIter it(arr); it; ++it) {
    if (!it.first().isString()) {
      return false;
    }
    if (it.first().toString() == s_options) {
      if (!StreamContext::validateOptions(it.second())) {
        return false;
      }
    }
  }
  return true;
}

void StreamContext::mergeParams(const Array& params) {
  if (m_params.isNull()) {
    m_params = Array::CreateDict();
  }
  if (params.exists(s_notification)) {
    m_params.set(s_notification, params[s_notification]);
  }
  if (params.exists(s_options)) {
    assertx(params[s_options].isArray());
    mergeOptions(params[s_options].toArray());
  }
}

Array StreamContext::getParams() const {
  Array params = m_params;
  if (params.isNull()) {
    params = Array::CreateDict();
  }
  params.set(s_options, getOptions());
  return params;
}

///////////////////////////////////////////////////////////////////////////////

#define REGISTER_SAME_CONSTANT(name) HHVM_RC_INT(name, k_ ## name)

void StreamExtension::moduleInit() {
  REGISTER_SAME_CONSTANT(PSFS_ERR_FATAL);
  REGISTER_SAME_CONSTANT(PSFS_FEED_ME);
  REGISTER_SAME_CONSTANT(PSFS_FLAG_FLUSH_CLOSE);
  REGISTER_SAME_CONSTANT(PSFS_FLAG_FLUSH_INC);
  REGISTER_SAME_CONSTANT(PSFS_FLAG_NORMAL);
  REGISTER_SAME_CONSTANT(PSFS_PASS_ON);

  REGISTER_SAME_CONSTANT(STREAM_CLIENT_CONNECT);
  REGISTER_SAME_CONSTANT(STREAM_CLIENT_ASYNC_CONNECT);
  REGISTER_SAME_CONSTANT(STREAM_CLIENT_PERSISTENT);
  REGISTER_SAME_CONSTANT(STREAM_META_TOUCH);
  REGISTER_SAME_CONSTANT(STREAM_META_OWNER_NAME);
  REGISTER_SAME_CONSTANT(STREAM_META_OWNER);
  REGISTER_SAME_CONSTANT(STREAM_META_GROUP_NAME);
  REGISTER_SAME_CONSTANT(STREAM_META_GROUP);
  REGISTER_SAME_CONSTANT(STREAM_META_ACCESS);
  REGISTER_SAME_CONSTANT(STREAM_BUFFER_NONE);
  REGISTER_SAME_CONSTANT(STREAM_BUFFER_LINE);
  REGISTER_SAME_CONSTANT(STREAM_BUFFER_FULL);
  REGISTER_SAME_CONSTANT(STREAM_SERVER_BIND);
  REGISTER_SAME_CONSTANT(STREAM_SERVER_LISTEN);
  REGISTER_SAME_CONSTANT(STREAM_CRYPTO_METHOD_TLS_CLIENT);
  REGISTER_SAME_CONSTANT(STREAM_CRYPTO_METHOD_TLS_SERVER);
  REGISTER_SAME_CONSTANT(STREAM_CRYPTO_METHOD_ANY_CLIENT);
  REGISTER_SAME_CONSTANT(STREAM_CRYPTO_METHOD_ANY_SERVER);
  REGISTER_SAME_CONSTANT(STREAM_ENFORCE_SAFE_MODE);
  REGISTER_SAME_CONSTANT(STREAM_IGNORE_URL);
  REGISTER_SAME_CONSTANT(STREAM_IPPROTO_ICMP);
  REGISTER_SAME_CONSTANT(STREAM_IPPROTO_IP);
  REGISTER_SAME_CONSTANT(STREAM_IPPROTO_RAW);
  REGISTER_SAME_CONSTANT(STREAM_IPPROTO_TCP);
  REGISTER_SAME_CONSTANT(STREAM_IPPROTO_UDP);
  REGISTER_SAME_CONSTANT(STREAM_IS_URL);
  REGISTER_SAME_CONSTANT(STREAM_MKDIR_RECURSIVE);
  REGISTER_SAME_CONSTANT(STREAM_MUST_SEEK);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_AUTH_REQUIRED);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_AUTH_RESULT);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_COMPLETED);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_CONNECT);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_FAILURE);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_FILE_SIZE_IS);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_MIME_TYPE_IS);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_PROGRESS);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_REDIRECTED);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_RESOLVE);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_SEVERITY_ERR);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_SEVERITY_INFO);
  REGISTER_SAME_CONSTANT(STREAM_NOTIFY_SEVERITY_WARN);
  REGISTER_SAME_CONSTANT(STREAM_OOB);
  REGISTER_SAME_CONSTANT(STREAM_PEEK);
  REGISTER_SAME_CONSTANT(STREAM_PF_INET);
  REGISTER_SAME_CONSTANT(STREAM_PF_INET6);
  REGISTER_SAME_CONSTANT(STREAM_PF_UNIX);
  REGISTER_SAME_CONSTANT(STREAM_REPORT_ERRORS);
  REGISTER_SAME_CONSTANT(STREAM_SHUT_RD);
  REGISTER_SAME_CONSTANT(STREAM_SHUT_RDWR);
  REGISTER_SAME_CONSTANT(STREAM_SHUT_WR);
  REGISTER_SAME_CONSTANT(STREAM_SOCK_DGRAM);
  REGISTER_SAME_CONSTANT(STREAM_SOCK_RAW);
  REGISTER_SAME_CONSTANT(STREAM_SOCK_RDM);
  REGISTER_SAME_CONSTANT(STREAM_SOCK_SEQPACKET);
  REGISTER_SAME_CONSTANT(STREAM_SOCK_STREAM);
  REGISTER_SAME_CONSTANT(STREAM_USE_PATH);

  HHVM_RC_INT(STREAM_AWAIT_READ, FileEventHandler::READ);
  HHVM_RC_INT(STREAM_AWAIT_WRITE, FileEventHandler::WRITE);
  HHVM_RC_INT(STREAM_AWAIT_READ_WRITE, FileEventHandler::READ_WRITE);

  HHVM_RC_INT(STREAM_AWAIT_ERROR, FileAwait::ERROR);
  HHVM_RC_INT(STREAM_AWAIT_TIMEOUT, FileAwait::TIMEOUT);
  HHVM_RC_INT(STREAM_AWAIT_READY, FileAwait::READY);
  HHVM_RC_INT(STREAM_AWAIT_CLOSED, FileAwait::CLOSED);

  REGISTER_SAME_CONSTANT(STREAM_URL_STAT_LINK);
  REGISTER_SAME_CONSTANT(STREAM_URL_STAT_QUIET);

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
  HHVM_FE(stream_resolve_include_path);
  HHVM_FE(stream_select);
  HHVM_FE(stream_await);
  HHVM_FE(stream_set_blocking);
  HHVM_FE(stream_set_read_buffer);
  HHVM_FE(stream_set_chunk_size);
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
}

///////////////////////////////////////////////////////////////////////////////
}
