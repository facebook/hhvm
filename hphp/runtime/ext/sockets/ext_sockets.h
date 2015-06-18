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

#ifndef incl_HPHP_EXT_SOCKET_H_
#define incl_HPHP_EXT_SOCKET_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/util/network.h"
#include "hphp/runtime/ext/stream/ext_stream.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(socket_create,
                      int domain,
                      int type,
                      int protocol);
Variant HHVM_FUNCTION(socket_create_listen,
                      int port,
                      int backlog = 128);
bool HHVM_FUNCTION(socket_create_pair,
                   int domain,
                   int type,
                   int protocol,
                   VRefParam fd);
Variant HHVM_FUNCTION(socket_get_option,
                      const Resource& socket,
                      int level,
                      int optname);
bool HHVM_FUNCTION(socket_getpeername,
                   const Resource& socket,
                   VRefParam address,
                   VRefParam port = uninit_null());
bool HHVM_FUNCTION(socket_getsockname,
                   const Resource& socket,
                   VRefParam address,
                   VRefParam port = uninit_null());
bool HHVM_FUNCTION(socket_set_block,
                   const Resource& socket);
bool HHVM_FUNCTION(socket_set_nonblock,
                   const Resource& socket);
bool HHVM_FUNCTION(socket_set_option,
                   const Resource& socket,
                   int level,
                   int optname,
                   const Variant& optval);
bool HHVM_FUNCTION(socket_connect,
                   const Resource& socket,
                   const String& address,
                   int port = 0);
bool HHVM_FUNCTION(socket_bind,
                   const Resource& socket,
                   const String& address,
                   int port = 0);
bool HHVM_FUNCTION(socket_listen,
                   const Resource& socket,
                   int backlog = 0);
Variant HHVM_FUNCTION(socket_select,
                      VRefParam read,
                      VRefParam write,
                      VRefParam except,
                      const Variant& vtv_sec,
                      int tv_usec = 0);
Variant HHVM_FUNCTION(socket_server,
                      const String& hostname,
                      int port = -1,
                      VRefParam errnum = uninit_null(),
                      VRefParam errstr = uninit_null());
Variant HHVM_FUNCTION(socket_accept,
                      const Resource& socket);
Variant HHVM_FUNCTION(socket_read,
                      const Resource& socket,
                      int length,
                      int type = 0);
Variant HHVM_FUNCTION(socket_write,
                      const Resource& socket,
                      const String& buffer,
                      int length = 0);
Variant HHVM_FUNCTION(socket_send,
                      const Resource& socket,
                      const String& buf,
                      int len,
                      int flags);
Variant HHVM_FUNCTION(socket_sendto,
                      const Resource& socket,
                      const String& buf,
                      int len,
                      int flags,
                      const String& addr,
                      int port = -1);
Variant HHVM_FUNCTION(socket_recv,
                      const Resource& socket,
                      VRefParam buf,
                      int len,
                      int flags);
Variant HHVM_FUNCTION(socket_recvfrom,
                      const Resource& socket,
                      VRefParam buf,
                      int len,
                      int flags,
                      VRefParam name,
                      VRefParam port = -1);
bool HHVM_FUNCTION(socket_shutdown,
                   const Resource& socket,
                   int how = 0);
void HHVM_FUNCTION(socket_close,
                   const Resource& socket);
String HHVM_FUNCTION(socket_strerror,
                     int errnum);
int64_t HHVM_FUNCTION(socket_last_error,
                      const Variant& socket = null_variant);
void HHVM_FUNCTION(socket_clear_error,
                   const Variant& socket = null_variant);
Variant HHVM_FUNCTION(getaddrinfo,
                      const String& host,
                      const String& port,
                      int family = 0,
                      int socktype = 0,
                      int protocol = 0,
                      int flags = 0);

///////////////////////////////////////////////////////////////////////////////
// Zend defines the following in network, so these are not part of the
// socket extension.

Variant HHVM_FUNCTION(fsockopen,
                      const String& hostname,
                      int port = -1,
                      VRefParam errnum = uninit_null(),
                      VRefParam errstr = uninit_null(),
                      double timeout = -1.0);
Variant HHVM_FUNCTION(pfsockopen,
                      const String& hostname,
                      int port = -1,
                      VRefParam errnum = uninit_null(),
                      VRefParam errstr = uninit_null(),
                      double timeout = -1.0);

///////////////////////////////////////////////////////////////////////////////

Variant socket_server_impl(const HostURL &hosturl,
                           int flags = k_STREAM_SERVER_BIND |
                             k_STREAM_SERVER_LISTEN,
                           VRefParam errnum = uninit_null(),
                           VRefParam errstr = uninit_null());
Variant sockopen_impl(const HostURL &hosturl, VRefParam errnum,
                      VRefParam errstr, double timeout, bool persistent,
                      const Variant& context);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_SOCKET_H_
