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

#ifndef __EXT_STREAM_H__
#define __EXT_STREAM_H__

#include <runtime/base/base_includes.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// stream context

class StreamContext : public ResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(StreamContext);

  // overriding ResourceData
  virtual const char *o_getClassName() const { return "StreamContext";}

  StreamContext(CArrRef options, CArrRef params)
    : m_options(options), m_params(params) {
  }
  Array m_options;
  Array m_params;
};

inline Object f_stream_context_create(CArrRef options = null_array,
                                      CArrRef params = null_array) {
  return Object(NEW(StreamContext)(options, params));
}

inline Object f_stream_context_get_default(CArrRef options = null_array) {
  throw NotImplementedException(__func__);
}

inline Array f_stream_context_get_options(CObjRef stream_or_context) {
  throw NotImplementedException(__func__);
}

inline bool f_stream_context_set_option(CObjRef stream_or_context,
                                        CVarRef wrapper,
                                        CStrRef option = null_string,
                                        CVarRef value = null_variant) {
  throw NotImplementedException(__func__);
}

inline bool f_stream_context_set_param(CObjRef stream_or_context,
                                       CArrRef params) {
  throw NotImplementedException(__func__);
}

///////////////////////////////////////////////////////////////////////////////

Variant f_stream_copy_to_stream(CObjRef source, CObjRef dest,
                                int maxlength = 0, int offset = 0);

inline bool f_stream_encoding(CObjRef stream, CStrRef encoding = null_string) {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

inline void f_stream_bucket_append(CObjRef brigade, CObjRef bucket) {
  throw NotSupportedException(__func__, "stream bucket is not supported");
}

inline void f_stream_bucket_prepend(CObjRef brigade, CObjRef bucket) {
  throw NotSupportedException(__func__, "stream bucket is not supported");
}

inline Object f_stream_bucket_make_writeable(CObjRef brigade) {
  throw NotSupportedException(__func__, "stream bucket is not supported");
}

inline Object f_stream_bucket_new(CObjRef stream, CStrRef buffer) {
  throw NotSupportedException(__func__, "stream bucket is not supported");
}

inline bool f_stream_filter_register(CStrRef filtername, CStrRef classname) {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

inline bool f_stream_filter_remove(CObjRef stream_filter) {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

inline Object f_stream_filter_append(CObjRef stream, CStrRef filtername,
                                     int read_write = 0,
                                     CVarRef params = null_variant) {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

inline Object f_stream_filter_prepend(CObjRef stream, CStrRef filtername,
                                      int read_write = 0,
                                      CVarRef params = null_variant) {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

Variant f_stream_get_contents(CObjRef handle, int maxlen = 0,
                              int offset = 0);

inline Array f_stream_get_filters() {
  throw NotSupportedException(__func__, "stream filter is not supported");
}

Variant f_stream_get_line(CObjRef handle, int length = 0,
                          CStrRef ending = null_string);

inline Array f_stream_get_meta_data(CObjRef stream) {
  File *f = stream.getTyped<File>(true);
  if (f) return f->getMetaData();
  return Array();
}

inline Array f_stream_get_transports() {
  return CREATE_VECTOR4("tcp", "udp", "unix", "udg");
}

inline Array f_stream_get_wrappers() {
  throw NotSupportedException(__func__, "stream protocol is not supported");
}

inline bool f_stream_register_wrapper(CStrRef protocol, CStrRef classname) {
  throw NotSupportedException(__func__, "stream protocol is not supported");
}

inline bool f_stream_wrapper_register(CStrRef protocol, CStrRef classname) {
  throw NotSupportedException(__func__, "stream protocol is not supported");
}

inline bool f_stream_wrapper_restore(CStrRef protocol) {
  throw NotSupportedException(__func__, "stream protocol is not supported");
}

inline bool f_stream_wrapper_unregister(CStrRef protocol) {
  throw NotSupportedException(__func__, "stream protocol is not supported");
}

inline String f_stream_resolve_include_path(CStrRef filename,
                                            CObjRef context = null_object) {
  throw NotSupportedException(__func__, "include path is not supported");
}

Variant f_stream_select(Variant read, Variant write, Variant except,
                        CVarRef vtv_sec, int tv_usec = 0);

bool f_stream_set_blocking(CObjRef stream, int mode);

bool f_stream_set_timeout(CObjRef stream, int seconds, int microseconds = 0);

int f_stream_set_write_buffer(CObjRef stream, int buffer);

inline int f_set_file_buffer(CObjRef stream, int buffer) {
  return f_stream_set_write_buffer(stream, buffer);
}

///////////////////////////////////////////////////////////////////////////////
// stream sockets: ext_socket has better implementation of socket functions

Variant f_stream_socket_accept(CObjRef server_socket, double timeout = 0.0,
                              Variant peername = null);

Variant f_stream_socket_server(CStrRef local_socket, Variant errnum = null,
                              Variant errstr = null,
                              int flags = 0, CObjRef context = null_object);

Variant f_stream_socket_client(CStrRef remote_socket, Variant errnum = null,
                              Variant errstr = null, double timeout = 0.0,
                              int flags = 0, CObjRef context = null_object);

inline Variant f_stream_socket_enable_crypto(CObjRef stream, bool enable,
                                             int crypto_type = 0,
                                             CObjRef session_stream = null_object) {
  throw NotSupportedException(__func__, "no crypto support on sockets");
}

Variant f_stream_socket_get_name(CObjRef handle, bool want_peer);

Variant f_stream_socket_pair(int domain, int type, int protocol);

Variant f_stream_socket_recvfrom(CObjRef socket, int length, int flags = 0,
                                CStrRef address = null_string);

Variant f_stream_socket_sendto(CObjRef socket, CStrRef data, int flags = 0,
                           CStrRef address = null_string);

bool f_stream_socket_shutdown(CObjRef stream, int how);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_STREAM_H__
