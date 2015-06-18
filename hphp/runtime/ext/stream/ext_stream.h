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

#ifndef incl_HPHP_EXT_STREAM_H_
#define incl_HPHP_EXT_STREAM_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP {

  const int64_t k_STREAM_CLIENT_CONNECT = 4;
  const int64_t k_STREAM_CLIENT_ASYNC_CONNECT = 2;
  const int64_t k_STREAM_CLIENT_PERSISTENT = 1;
  const int64_t k_STREAM_META_TOUCH = 1;
  const int64_t k_STREAM_META_OWNER_NAME = 2;
  const int64_t k_STREAM_META_OWNER = 3;
  const int64_t k_STREAM_META_GROUP_NAME = 4;
  const int64_t k_STREAM_META_GROUP = 5;
  const int64_t k_STREAM_META_ACCESS = 6;
  const int64_t k_STREAM_BUFFER_NONE = 0;    /* unbuffered */
  const int64_t k_STREAM_BUFFER_LINE = 1;   /* line buffered */
  const int64_t k_STREAM_BUFFER_FULL = 2;    /* fully buffered */
  const int64_t k_STREAM_SERVER_BIND = 4;
  const int64_t k_STREAM_SERVER_LISTEN = 8;
  const int64_t k_STREAM_CRYPTO_METHOD_SSLv23_CLIENT = 7;
  const int64_t k_STREAM_CRYPTO_METHOD_SSLv23_SERVER = 6;
  const int64_t k_STREAM_CRYPTO_METHOD_SSLv2_CLIENT = 3;
  const int64_t k_STREAM_CRYPTO_METHOD_SSLv2_SERVER = 2;
  const int64_t k_STREAM_CRYPTO_METHOD_SSLv3_CLIENT = 5;
  const int64_t k_STREAM_CRYPTO_METHOD_SSLv3_SERVER = 4;
  const int64_t k_STREAM_CRYPTO_METHOD_TLS_CLIENT = 57;
  const int64_t k_STREAM_CRYPTO_METHOD_TLS_SERVER = 56;
  const int64_t k_STREAM_CRYPTO_METHOD_TLSv1_0_CLIENT = 9;
  const int64_t k_STREAM_CRYPTO_METHOD_TLSv1_0_SERVER = 8;
  const int64_t k_STREAM_CRYPTO_METHOD_TLSv1_1_CLIENT = 17;
  const int64_t k_STREAM_CRYPTO_METHOD_TLSv1_1_SERVER = 16;
  const int64_t k_STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT = 33;
  const int64_t k_STREAM_CRYPTO_METHOD_TLSv1_2_SERVER = 32;
  const int64_t k_STREAM_CRYPTO_METHOD_ANY_CLIENT = 63;
  const int64_t k_STREAM_CRYPTO_METHOD_ANY_SERVER = 62;
  const int64_t k_STREAM_ENFORCE_SAFE_MODE = 4;
  const int64_t k_STREAM_IGNORE_URL = 2;
  const int64_t k_STREAM_IPPROTO_ICMP = 1;
  const int64_t k_STREAM_IPPROTO_IP = 0;
  const int64_t k_STREAM_IPPROTO_RAW = 255;
  const int64_t k_STREAM_IPPROTO_TCP = 6;
  const int64_t k_STREAM_IPPROTO_UDP = 17;
  const int64_t k_STREAM_IS_URL = 1;
  const int64_t k_STREAM_MKDIR_RECURSIVE = 1;
  const int64_t k_STREAM_MUST_SEEK = 16;
  const int64_t k_STREAM_NOTIFY_AUTH_REQUIRED = 3;
  const int64_t k_STREAM_NOTIFY_AUTH_RESULT = 10;
  const int64_t k_STREAM_NOTIFY_COMPLETED = 8;
  const int64_t k_STREAM_NOTIFY_CONNECT = 2;
  const int64_t k_STREAM_NOTIFY_FAILURE = 9;
  const int64_t k_STREAM_NOTIFY_FILE_SIZE_IS = 5;
  const int64_t k_STREAM_NOTIFY_MIME_TYPE_IS = 4;
  const int64_t k_STREAM_NOTIFY_PROGRESS = 7;
  const int64_t k_STREAM_NOTIFY_REDIRECTED = 6;
  const int64_t k_STREAM_NOTIFY_RESOLVE = 1;
  const int64_t k_STREAM_NOTIFY_SEVERITY_ERR = 2;
  const int64_t k_STREAM_NOTIFY_SEVERITY_INFO = 0;
  const int64_t k_STREAM_NOTIFY_SEVERITY_WARN = 1;
  const int64_t k_STREAM_OOB = 1;
  const int64_t k_STREAM_PEEK = 2;
  const int64_t k_STREAM_PF_INET = 2;
  const int64_t k_STREAM_PF_INET6 = 10;
  const int64_t k_STREAM_PF_UNIX = 1;
  const int64_t k_STREAM_REPORT_ERRORS = 8;
  const int64_t k_STREAM_SHUT_RD = 0;
  const int64_t k_STREAM_SHUT_RDWR = 2;
  const int64_t k_STREAM_SHUT_WR = 1;
  const int64_t k_STREAM_SOCK_DGRAM = 2;
  const int64_t k_STREAM_SOCK_RAW = 3;
  const int64_t k_STREAM_SOCK_RDM = 4;
  const int64_t k_STREAM_SOCK_SEQPACKET = 5;
  const int64_t k_STREAM_SOCK_STREAM = 1;
  const int64_t k_STREAM_URL_STAT_LINK = 1;
  const int64_t k_STREAM_URL_STAT_QUIET = 2;
  const int64_t k_STREAM_USE_PATH = 1;

///////////////////////////////////////////////////////////////////////////////
// stream context

struct StreamContext final : ResourceData {
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(StreamContext);

  CLASSNAME_IS("stream-context")
  const String& o_getClassNameHook() const override { return classnameof(); }

  StreamContext(const Array& options, const Array& params)
    : m_options(options), m_params(params) {
  }

  static bool validateOptions(const Variant& options);
  void setOption(const String& wrapper, const String& option,
                 const Variant& value);
  void mergeOptions(const Array& options);
  Array getOptions() const;
  static bool validateParams(const Variant& params);
  void mergeParams(const Array& params);
  Array getParams() const;

private:
  static StaticString s_options_key;
  static StaticString s_notification_key;

  Array m_options;
  Array m_params;
};

Variant HHVM_FUNCTION(stream_context_create,
                      const Variant& options = null_variant,
                      const Variant& params = null_variant);

Variant HHVM_FUNCTION(stream_context_get_options,
                      const Resource& stream_or_context);

bool HHVM_FUNCTION(stream_context_set_option,
                   const Variant& stream_or_context,
                   const Variant& wrapper,
                   const Variant& option = null_variant,
                   const Variant& value = null_variant);

Variant HHVM_FUNCTION(stream_context_get_default,
                      const Variant& options /* = null_variant */);

Variant HHVM_FUNCTION(stream_context_get_params,
                      const Resource& stream_or_context);

bool HHVM_FUNCTION(stream_context_set_params,
                   const Resource& stream_or_context,
                   const Array& params);

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(stream_copy_to_stream,
                      const Resource& source,
                      const Resource& dest,
                      int maxlength = -1,
                      int offset = 0);

Variant HHVM_FUNCTION(stream_get_contents,
                      const Resource& handle,
                      int maxlen = -1,
                      int offset = -1);

Variant HHVM_FUNCTION(stream_get_line,
                      const Resource& handle,
                      int length = 0,
                      const Variant& ending = null_variant);

Variant HHVM_FUNCTION(stream_get_meta_data,
                      const Resource& stream);

Array HHVM_FUNCTION(stream_get_transports);

Array HHVM_FUNCTION(stream_get_wrappers);
bool HHVM_FUNCTION(stream_is_local,
                   const Variant& stream_or_url);
bool HHVM_FUNCTION(stream_register_wrapper,
                   const String& protocol,
                   const String& classname,
                   int flags);
bool HHVM_FUNCTION(stream_wrapper_register,
                   const String& protocol,
                   const String& classname,
                   int flags);
bool HHVM_FUNCTION(stream_wrapper_restore,
                   const String& protocol);
bool HHVM_FUNCTION(stream_wrapper_unregister,
                   const String& protocol);

Variant HHVM_FUNCTION(stream_resolve_include_path,
                      const String& filename,
                      const Variant& context = null_variant);

Variant HHVM_FUNCTION(stream_select,
                      VRefParam read,
                      VRefParam write,
                      VRefParam except,
                      const Variant& vtv_sec,
                      int tv_usec = 0);

Object HHVM_FUNCTION(stream_await,
                     const Resource& stream,
                     int64_t events,
                     double timeout = 0.0);

bool HHVM_FUNCTION(stream_set_blocking,
                   const Resource& stream,
                   int mode);

bool HHVM_FUNCTION(stream_set_timeout,
                   const Resource& stream,
                   int seconds,
                   int microseconds = 0);

int64_t HHVM_FUNCTION(stream_set_write_buffer,
                      const Resource& stream,
                      int buffer);

int64_t HHVM_FUNCTION(set_file_buffer,
                      const Resource& stream,
                      int buffer);

///////////////////////////////////////////////////////////////////////////////
// stream sockets: ext_socket has better implementation of socket functions

Variant HHVM_FUNCTION(stream_socket_accept,
                      const Resource& server_socket,
                      double timeout = -1.0,
                      VRefParam peername = uninit_null());

Variant HHVM_FUNCTION(stream_socket_server,
                      const String& local_socket,
                      VRefParam errnum = uninit_null(),
                      VRefParam errstr = uninit_null(),
                      int flags = k_STREAM_SERVER_BIND|k_STREAM_SERVER_LISTEN,
                      const Variant& context = null_variant);

Variant HHVM_FUNCTION(stream_socket_client,
                      const String& remote_socket,
                      VRefParam errnum = uninit_null(),
                      VRefParam errstr = uninit_null(),
                      double timeout = -1.0,
                      int flags = 0,
                      const Variant& context = null_variant);

bool HHVM_FUNCTION(stream_socket_enable_crypto,
                   const Resource& socket,
                   bool enable,
                   int cryptotype,
                   const Variant& sessionstream);

Variant HHVM_FUNCTION(stream_socket_get_name,
                      const Resource& handle,
                      bool want_peer);

Variant HHVM_FUNCTION(stream_socket_pair,
                      int domain,
                      int type,
                      int protocol);

Variant HHVM_FUNCTION(stream_socket_recvfrom,
                      const Resource& socket,
                      int length,
                      int flags = 0,
                      VRefParam address = uninit_null());

Variant HHVM_FUNCTION(stream_socket_sendto,
                      const Resource& socket,
                      const String& data,
                      int flags = 0,
                      const Variant& address = null_variant);

bool HHVM_FUNCTION(stream_socket_shutdown,
                   const Resource& stream,
                   int how);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_STREAM_H_
