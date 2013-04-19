/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
namespace HPHP {

TypedValue* fh_socket_create(TypedValue* _rv, int domain, int type, int protocol) asm("_ZN4HPHP15f_socket_createEiii");

TypedValue* fh_socket_create_listen(TypedValue* _rv, int port, int backlog) asm("_ZN4HPHP22f_socket_create_listenEii");

bool fh_socket_create_pair(int domain, int type, int protocol, TypedValue* fd) asm("_ZN4HPHP20f_socket_create_pairEiiiRKNS_14VRefParamValueE");

TypedValue* fh_socket_get_option(TypedValue* _rv, Value* socket, int level, int optname) asm("_ZN4HPHP19f_socket_get_optionERKNS_6ObjectEii");

bool fh_socket_getpeername(Value* socket, TypedValue* address, TypedValue* port) asm("_ZN4HPHP20f_socket_getpeernameERKNS_6ObjectERKNS_14VRefParamValueES5_");

bool fh_socket_getsockname(Value* socket, TypedValue* address, TypedValue* port) asm("_ZN4HPHP20f_socket_getsocknameERKNS_6ObjectERKNS_14VRefParamValueES5_");

bool fh_socket_set_block(Value* socket) asm("_ZN4HPHP18f_socket_set_blockERKNS_6ObjectE");

bool fh_socket_set_nonblock(Value* socket) asm("_ZN4HPHP21f_socket_set_nonblockERKNS_6ObjectE");

bool fh_socket_set_option(Value* socket, int level, int optname, TypedValue* optval) asm("_ZN4HPHP19f_socket_set_optionERKNS_6ObjectEiiRKNS_7VariantE");

bool fh_socket_connect(Value* socket, Value* address, int port) asm("_ZN4HPHP16f_socket_connectERKNS_6ObjectERKNS_6StringEi");

bool fh_socket_bind(Value* socket, Value* address, int port) asm("_ZN4HPHP13f_socket_bindERKNS_6ObjectERKNS_6StringEi");

bool fh_socket_listen(Value* socket, int backlog) asm("_ZN4HPHP15f_socket_listenERKNS_6ObjectEi");

TypedValue* fh_socket_select(TypedValue* _rv, TypedValue* read, TypedValue* write, TypedValue* except, TypedValue* vtv_sec, int tv_usec) asm("_ZN4HPHP15f_socket_selectERKNS_14VRefParamValueES2_S2_RKNS_7VariantEi");

TypedValue* fh_socket_server(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr) asm("_ZN4HPHP15f_socket_serverERKNS_6StringEiRKNS_14VRefParamValueES5_");

TypedValue* fh_socket_accept(TypedValue* _rv, Value* socket) asm("_ZN4HPHP15f_socket_acceptERKNS_6ObjectE");

TypedValue* fh_socket_read(TypedValue* _rv, Value* socket, int length, int type) asm("_ZN4HPHP13f_socket_readERKNS_6ObjectEii");

TypedValue* fh_socket_write(TypedValue* _rv, Value* socket, Value* buffer, int length) asm("_ZN4HPHP14f_socket_writeERKNS_6ObjectERKNS_6StringEi");

TypedValue* fh_socket_send(TypedValue* _rv, Value* socket, Value* buf, int len, int flags) asm("_ZN4HPHP13f_socket_sendERKNS_6ObjectERKNS_6StringEii");

TypedValue* fh_socket_sendto(TypedValue* _rv, Value* socket, Value* buf, int len, int flags, Value* addr, int port) asm("_ZN4HPHP15f_socket_sendtoERKNS_6ObjectERKNS_6StringEiiS5_i");

TypedValue* fh_socket_recv(TypedValue* _rv, Value* socket, TypedValue* buf, int len, int flags) asm("_ZN4HPHP13f_socket_recvERKNS_6ObjectERKNS_14VRefParamValueEii");

TypedValue* fh_socket_recvfrom(TypedValue* _rv, Value* socket, TypedValue* buf, int len, int flags, TypedValue* name, TypedValue* port) asm("_ZN4HPHP17f_socket_recvfromERKNS_6ObjectERKNS_14VRefParamValueEiiS5_S5_");

bool fh_socket_shutdown(Value* socket, int how) asm("_ZN4HPHP17f_socket_shutdownERKNS_6ObjectEi");

void fh_socket_close(Value* socket) asm("_ZN4HPHP14f_socket_closeERKNS_6ObjectE");

Value* fh_socket_strerror(Value* _rv, int errnum) asm("_ZN4HPHP17f_socket_strerrorEi");

long fh_socket_last_error(Value* socket) asm("_ZN4HPHP19f_socket_last_errorERKNS_6ObjectE");

void fh_socket_clear_error(Value* socket) asm("_ZN4HPHP20f_socket_clear_errorERKNS_6ObjectE");

TypedValue* fh_fsockopen(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr, double timeout) asm("_ZN4HPHP11f_fsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d");

TypedValue* fh_pfsockopen(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr, double timeout) asm("_ZN4HPHP12f_pfsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d");

TypedValue* fh_getaddrinfo(TypedValue* _rv, Value* host, Value* port, int family, int socktype, int protocol, int flags) asm("_ZN4HPHP13f_getaddrinfoERKNS_6StringES2_iiii");

} // namespace HPHP
