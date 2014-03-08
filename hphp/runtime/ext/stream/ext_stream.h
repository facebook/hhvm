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

#ifndef incl_HPHP_EXT_STREAM_H_
#define incl_HPHP_EXT_STREAM_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/file-repository.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// stream context

class StreamContext : public ResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(StreamContext);

  CLASSNAME_IS("stream-context")
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  StreamContext(const Array& options, const Array& params)
    : m_options(options), m_params(params) {
  }

  static bool validateOptions(const Variant& options);
  void setOption(const String& wrapper, const String& option, const Variant& value);
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

Variant f_stream_context_create(const Array& options = null_array,
                                 const Array& params = null_array);

Variant f_stream_context_get_options(const Resource& stream_or_context);

bool f_stream_context_set_option(const Resource& stream_or_context,
                                 const Variant& wrapper,
                                 const String& option = null_string,
                                 const Variant& value = null_variant);

Variant f_stream_context_get_params(const Resource& stream_or_context);

bool f_stream_context_set_params(const Resource& stream_or_context,
                                 const Array& params);

///////////////////////////////////////////////////////////////////////////////

Variant f_stream_copy_to_stream(const Resource& source, const Resource& dest,
                                int maxlength = -1, int offset = 0);

bool f_stream_encoding(const Resource& stream, const String& encoding = null_string);

Variant f_stream_get_contents(const Resource& handle, int maxlen = -1,
                              int offset = -1);

Variant f_stream_get_line(const Resource& handle, int length = 0,
                          const String& ending = null_string);

Variant f_stream_get_meta_data(const Resource& stream);

Array f_stream_get_transports();

Array f_stream_get_wrappers();
bool f_stream_is_local(const Variant& stream_or_url);
bool f_stream_register_wrapper(const String& protocol, const String& classname,
                               int flags);
bool f_stream_wrapper_register(const String& protocol, const String& classname,
                               int flags);
bool f_stream_wrapper_restore(const String& protocol);
bool f_stream_wrapper_unregister(const String& protocol);

Variant f_stream_resolve_include_path(const String& filename,
                                     const Resource& context = null_resource);

Variant f_stream_select(VRefParam read, VRefParam write, VRefParam except,
                        const Variant& vtv_sec, int tv_usec = 0);

bool f_stream_set_blocking(const Resource& stream, int mode);

bool f_stream_set_timeout(const Resource& stream, int seconds, int microseconds = 0);

int64_t f_stream_set_write_buffer(const Resource& stream, int buffer);

int64_t f_set_file_buffer(const Resource& stream, int buffer);

///////////////////////////////////////////////////////////////////////////////
// stream sockets: ext_socket has better implementation of socket functions

Variant f_stream_socket_accept(const Resource& server_socket, double timeout = -1.0,
                               VRefParam peername = uninit_null());

Variant f_stream_socket_server(const String& local_socket,
                               VRefParam errnum = uninit_null(),
                               VRefParam errstr = uninit_null(),
                               int flags = k_STREAM_SERVER_BIND|k_STREAM_SERVER_LISTEN,
                               const Resource& context = null_resource);

Variant f_stream_socket_client(const String& remote_socket, VRefParam errnum = uninit_null(),
                              VRefParam errstr = uninit_null(), double timeout = -1.0,
                              int flags = 0, const Resource& context = null_resource);

Variant f_stream_socket_enable_crypto(const Resource& stream, bool enable,
                                      int crypto_type = 0,
                                      const Resource& session_stream = null_resource);

Variant f_stream_socket_get_name(const Resource& handle, bool want_peer);

Variant f_stream_socket_pair(int domain, int type, int protocol);

Variant f_stream_socket_recvfrom(const Resource& socket, int length, int flags = 0,
                                 VRefParam address = uninit_null());

Variant f_stream_socket_sendto(const Resource& socket, const String& data, int flags = 0,
                           const String& address = null_string);

bool f_stream_socket_shutdown(const Resource& stream, int how);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_STREAM_H_
