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

  CLASSNAME_IS("StreamContext")
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  StreamContext(CArrRef options, CArrRef params)
    : m_options(options), m_params(params) {
  }

  static bool validateOptions(CVarRef options);
  void setOption(const String& wrapper, const String& option, CVarRef value);
  void mergeOptions(CArrRef options);
  Array getOptions() const;
  static bool validateParams(CVarRef params);
  void mergeParams(CArrRef params);
  Array getParams() const;

private:
  static StaticString s_options_key;
  static StaticString s_notification_key;

  Array m_options;
  Array m_params;
};

Variant f_stream_context_create(CArrRef options = null_array,
                                 CArrRef params = null_array);

Variant f_stream_context_get_options(CResRef stream_or_context);

bool f_stream_context_set_option(CResRef stream_or_context,
                                 CVarRef wrapper,
                                 const String& option = null_string,
                                 CVarRef value = null_variant);

Variant f_stream_context_get_params(CResRef stream_or_context);

bool f_stream_context_set_params(CResRef stream_or_context,
                                 CArrRef params);

///////////////////////////////////////////////////////////////////////////////

Variant f_stream_copy_to_stream(CResRef source, CResRef dest,
                                int maxlength = -1, int offset = 0);

bool f_stream_encoding(CResRef stream, const String& encoding = null_string);

void f_stream_bucket_append(CResRef brigade, CResRef bucket);

void f_stream_bucket_prepend(CResRef brigade, CResRef bucket);

Resource f_stream_bucket_make_writeable(CResRef brigade);

Resource f_stream_bucket_new(CResRef stream, const String& buffer);

bool f_stream_filter_register(const String& filtername, const String& classname);

bool f_stream_filter_remove(CResRef stream_filter);

Resource f_stream_filter_append(CResRef stream, const String& filtername,
                              int read_write = 0,
                              CVarRef params = null_variant);

Resource f_stream_filter_prepend(CResRef stream, const String& filtername,
                               int read_write = 0,
                               CVarRef params = null_variant);

Variant f_stream_get_contents(CResRef handle, int maxlen = -1,
                              int offset = -1);

Array f_stream_get_filters();

Variant f_stream_get_line(CResRef handle, int length = 0,
                          const String& ending = null_string);

Variant f_stream_get_meta_data(CResRef stream);

Array f_stream_get_transports();

Array f_stream_get_wrappers();
bool f_stream_is_local(CVarRef stream_or_url);
bool f_stream_register_wrapper(const String& protocol, const String& classname);
bool f_stream_wrapper_register(const String& protocol, const String& classname);
bool f_stream_wrapper_restore(const String& protocol);
bool f_stream_wrapper_unregister(const String& protocol);

Variant f_stream_resolve_include_path(const String& filename,
                                     CResRef context = null_resource);

Variant f_stream_select(VRefParam read, VRefParam write, VRefParam except,
                        CVarRef vtv_sec, int tv_usec = 0);

bool f_stream_set_blocking(CResRef stream, int mode);

bool f_stream_set_timeout(CResRef stream, int seconds, int microseconds = 0);

int64_t f_stream_set_write_buffer(CResRef stream, int buffer);

int64_t f_set_file_buffer(CResRef stream, int buffer);

///////////////////////////////////////////////////////////////////////////////
// stream sockets: ext_socket has better implementation of socket functions

Variant f_stream_socket_accept(CResRef server_socket, double timeout = -1.0,
                               VRefParam peername = uninit_null());

Variant f_stream_socket_server(const String& local_socket,
                               VRefParam errnum = uninit_null(),
                               VRefParam errstr = uninit_null(),
                               int flags = k_STREAM_SERVER_BIND|k_STREAM_SERVER_LISTEN,
                               CResRef context = null_resource);

Variant f_stream_socket_client(const String& remote_socket, VRefParam errnum = uninit_null(),
                              VRefParam errstr = uninit_null(), double timeout = -1.0,
                              int flags = 0, CResRef context = null_resource);

Variant f_stream_socket_enable_crypto(CResRef stream, bool enable,
                                      int crypto_type = 0,
                                      CResRef session_stream = null_resource);

Variant f_stream_socket_get_name(CResRef handle, bool want_peer);

Variant f_stream_socket_pair(int domain, int type, int protocol);

Variant f_stream_socket_recvfrom(CResRef socket, int length, int flags = 0,
                                 VRefParam address = uninit_null());

Variant f_stream_socket_sendto(CResRef socket, const String& data, int flags = 0,
                           const String& address = null_string);

bool f_stream_socket_shutdown(CResRef stream, int how);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_STREAM_H_
