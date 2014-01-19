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

#ifndef incl_HPHP_EXT_SOCKET_H_
#define incl_HPHP_EXT_SOCKET_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/util/network.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_socket_create(int domain, int type, int protocol);
Variant f_socket_create_listen(int port, int backlog = 128);
bool f_socket_create_pair(int domain, int type, int protocol, VRefParam fd);
Variant f_socket_get_option(CResRef socket, int level, int optname);
bool f_socket_getpeername(CResRef socket, VRefParam address,
                          VRefParam port = uninit_null());
bool f_socket_getsockname(CResRef socket, VRefParam address,
                          VRefParam port = uninit_null());
bool f_socket_set_block(CResRef socket);
bool f_socket_set_nonblock(CResRef socket);
bool f_socket_set_option(CResRef socket, int level, int optname,
                         CVarRef optval);
bool f_socket_connect(CResRef socket, const String& address, int port = 0);
bool f_socket_bind(CResRef socket, const String& address, int port = 0);
bool f_socket_listen(CResRef socket, int backlog = 0);
Variant f_socket_select(VRefParam read, VRefParam write, VRefParam except,
                        CVarRef vtv_sec, int tv_usec = 0);
Variant f_socket_server(const String& hostname, int port = -1,
                        VRefParam errnum = uninit_null(),
                        VRefParam errstr = uninit_null());
Variant socket_server_impl(const Util::HostURL &hosturl,
                           int flags = k_STREAM_SERVER_BIND|k_STREAM_SERVER_LISTEN,
                           VRefParam errnum = uninit_null(),
                           VRefParam errstr = uninit_null());
Variant f_socket_accept(CResRef socket);
Variant f_socket_read(CResRef socket, int length, int type = 0);
Variant f_socket_write(CResRef socket, const String& buffer, int length = 0);
Variant f_socket_send(CResRef socket, const String& buf, int len, int flags);
Variant f_socket_sendto(CResRef socket, const String& buf, int len, int flags,
                        const String& addr, int port = -1);
Variant f_socket_recv(CResRef socket, VRefParam buf, int len, int flags);
Variant f_socket_recvfrom(CResRef socket, VRefParam buf, int len, int flags,
                          VRefParam name, VRefParam port = -1);
bool f_socket_shutdown(CResRef socket, int how = 0);
void f_socket_close(CResRef socket);
String f_socket_strerror(int errnum);
int64_t f_socket_last_error(CResRef socket = null_resource);
void f_socket_clear_error(CResRef socket = null_resource);
Variant f_getaddrinfo(const String& host, const String& port, int family = 0,
                      int socktype = 0, int protocol = 0, int flags = 0);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_SOCKET_H_
