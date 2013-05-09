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

#include <runtime/base/base_includes.h>
#include <runtime/eval/runtime/file_repository.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// stream context

class StreamContext : public ResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(StreamContext);

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassNameHook() const { return s_class_name; }

  StreamContext(CArrRef options, CArrRef params)
    : m_options(options), m_params(params) {
  }
  Array m_options;
  Array m_params;
};

Object f_stream_context_create(CArrRef options = null_array,
                               CArrRef params = null_array);

Object f_stream_context_get_default(CArrRef options = null_array);

Variant f_stream_context_get_options(CObjRef stream_or_context);

bool f_stream_context_set_option(CObjRef stream_or_context,
                                 CVarRef wrapper,
                                 CStrRef option = null_string,
                                 CVarRef value = null_variant);

bool f_stream_context_set_param(CObjRef stream_or_context,
                                CArrRef params);

///////////////////////////////////////////////////////////////////////////////

Variant f_stream_copy_to_stream(CObjRef source, CObjRef dest,
                                int maxlength = -1, int offset = 0);

bool f_stream_encoding(CObjRef stream, CStrRef encoding = null_string);

void f_stream_bucket_append(CObjRef brigade, CObjRef bucket);

void f_stream_bucket_prepend(CObjRef brigade, CObjRef bucket);

Object f_stream_bucket_make_writeable(CObjRef brigade);

Object f_stream_bucket_new(CObjRef stream, CStrRef buffer);

bool f_stream_filter_register(CStrRef filtername, CStrRef classname);

bool f_stream_filter_remove(CObjRef stream_filter);

Object f_stream_filter_append(CObjRef stream, CStrRef filtername,
                              int read_write = 0,
                              CVarRef params = null_variant);

Object f_stream_filter_prepend(CObjRef stream, CStrRef filtername,
                               int read_write = 0,
                               CVarRef params = null_variant);

Variant f_stream_get_contents(CObjRef handle, int maxlen = 0,
                              int offset = 0);

Array f_stream_get_filters();

Variant f_stream_get_line(CObjRef handle, int length = 0,
                          CStrRef ending = null_string);

Variant f_stream_get_meta_data(CObjRef stream);

Array f_stream_get_transports();

Array f_stream_get_wrappers();
bool f_stream_register_wrapper(CStrRef protocol, CStrRef classname);
bool f_stream_wrapper_register(CStrRef protocol, CStrRef classname);
bool f_stream_wrapper_restore(CStrRef protocol);
bool f_stream_wrapper_unregister(CStrRef protocol);

String f_stream_resolve_include_path(CStrRef filename,
                                     CObjRef context = null_object);

Variant f_stream_select(VRefParam read, VRefParam write, VRefParam except,
                        CVarRef vtv_sec, int tv_usec = 0);

bool f_stream_set_blocking(CObjRef stream, int mode);

bool f_stream_set_timeout(CObjRef stream, int seconds, int microseconds = 0);

int64_t f_stream_set_write_buffer(CObjRef stream, int buffer);

int64_t f_set_file_buffer(CObjRef stream, int buffer);

///////////////////////////////////////////////////////////////////////////////
// stream sockets: ext_socket has better implementation of socket functions

Variant f_stream_socket_accept(CObjRef server_socket, double timeout = 0.0,
                              VRefParam peername = uninit_null());

Variant f_stream_socket_server(CStrRef local_socket, VRefParam errnum = uninit_null(),
                              VRefParam errstr = uninit_null(),
                              int flags = 0, CObjRef context = null_object);

Variant f_stream_socket_client(CStrRef remote_socket, VRefParam errnum = uninit_null(),
                              VRefParam errstr = uninit_null(), double timeout = 0.0,
                              int flags = 0, CObjRef context = null_object);

Variant f_stream_socket_enable_crypto(CObjRef stream, bool enable,
                                      int crypto_type = 0,
                                      CObjRef session_stream = null_object);

Variant f_stream_socket_get_name(CObjRef handle, bool want_peer);

Variant f_stream_socket_pair(int domain, int type, int protocol);

Variant f_stream_socket_recvfrom(CObjRef socket, int length, int flags = 0,
                                CStrRef address = null_string);

Variant f_stream_socket_sendto(CObjRef socket, CStrRef data, int flags = 0,
                           CStrRef address = null_string);

bool f_stream_socket_shutdown(CObjRef stream, int how);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_STREAM_H_
