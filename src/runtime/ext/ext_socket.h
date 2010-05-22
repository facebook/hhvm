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

#ifndef __EXT_SOCKET_H__
#define __EXT_SOCKET_H__

#include <runtime/base/base_includes.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_socket_create(int domain, int type, int protocol);
Variant f_socket_create_listen(int port, int backlog = 128);
bool f_socket_create_pair(int domain, int type, int protocol, Variant fd);
Variant f_socket_get_option(CObjRef socket, int level, int optname);
bool f_socket_getpeername(CObjRef socket, Variant address,
                          Variant port = null);
bool f_socket_getsockname(CObjRef socket, Variant address,
                          Variant port = null);
bool f_socket_set_block(CObjRef socket);
bool f_socket_set_nonblock(CObjRef socket);
bool f_socket_set_option(CObjRef socket, int level, int optname,
                         CVarRef optval);
bool f_socket_connect(CObjRef socket, CStrRef address, int port = 0);
bool f_socket_bind(CObjRef socket, CStrRef address, int port = 0);
bool f_socket_listen(CObjRef socket, int backlog = 0);
Variant f_socket_select(Variant read, Variant write, Variant except,
                        CVarRef vtv_sec, int tv_usec = 0);
Variant f_socket_server(CStrRef hostname, int port = -1, Variant errnum = null,
                        Variant errstr = null);
Variant f_socket_accept(CObjRef socket);
Variant f_socket_read(CObjRef socket, int length, int type = 0);
Variant f_socket_write(CObjRef socket, CStrRef buffer, int length = 0);
Variant f_socket_send(CObjRef socket, CStrRef buf, int len, int flags);
Variant f_socket_sendto(CObjRef socket, CStrRef buf, int len, int flags,
                        CStrRef addr, int port = 0);
Variant f_socket_recv(CObjRef socket, Variant buf, int len, int flags);
Variant f_socket_recvfrom(CObjRef socket, Variant buf, int len, int flags,
                          Variant name, Variant port = 0);
bool f_socket_shutdown(CObjRef socket, int how = 0);
void f_socket_close(CObjRef socket);
String f_socket_strerror(int errnum);
int f_socket_last_error(CObjRef socket = null_object);
void f_socket_clear_error(CObjRef socket = null_object);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_SOCKET_H__
