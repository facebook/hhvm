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

/*
HPHP::Variant HPHP::f_fsockopen(HPHP::String const&, int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, double)
_ZN4HPHP11f_fsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d

(return value) => rax
_rv => rdi
hostname => rsi
port => rdx
errnum => rcx
errstr => r8
timeout => xmm0
*/

TypedValue* fh_fsockopen(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr, double timeout) asm("_ZN4HPHP11f_fsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d");

/*
HPHP::Variant HPHP::f_pfsockopen(HPHP::String const&, int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, double)
_ZN4HPHP12f_pfsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d

(return value) => rax
_rv => rdi
hostname => rsi
port => rdx
errnum => rcx
errstr => r8
timeout => xmm0
*/

TypedValue* fh_pfsockopen(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr, double timeout) asm("_ZN4HPHP12f_pfsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d");

/*
HPHP::Variant HPHP::f_socket_create(int, int, int)
_ZN4HPHP15f_socket_createEiii

(return value) => rax
_rv => rdi
domain => rsi
type => rdx
protocol => rcx
*/

TypedValue* fh_socket_create(TypedValue* _rv, int domain, int type, int protocol) asm("_ZN4HPHP15f_socket_createEiii");

/*
HPHP::Variant HPHP::f_socket_create_listen(int, int)
_ZN4HPHP22f_socket_create_listenEii

(return value) => rax
_rv => rdi
port => rsi
backlog => rdx
*/

TypedValue* fh_socket_create_listen(TypedValue* _rv, int port, int backlog) asm("_ZN4HPHP22f_socket_create_listenEii");

/*
bool HPHP::f_socket_create_pair(int, int, int, HPHP::VRefParamValue const&)
_ZN4HPHP20f_socket_create_pairEiiiRKNS_14VRefParamValueE

(return value) => rax
domain => rdi
type => rsi
protocol => rdx
fd => rcx
*/

bool fh_socket_create_pair(int domain, int type, int protocol, TypedValue* fd) asm("_ZN4HPHP20f_socket_create_pairEiiiRKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_socket_get_option(HPHP::Object const&, int, int)
_ZN4HPHP19f_socket_get_optionERKNS_6ObjectEii

(return value) => rax
_rv => rdi
socket => rsi
level => rdx
optname => rcx
*/

TypedValue* fh_socket_get_option(TypedValue* _rv, Value* socket, int level, int optname) asm("_ZN4HPHP19f_socket_get_optionERKNS_6ObjectEii");

/*
bool HPHP::f_socket_getpeername(HPHP::Object const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP20f_socket_getpeernameERKNS_6ObjectERKNS_14VRefParamValueES5_

(return value) => rax
socket => rdi
address => rsi
port => rdx
*/

bool fh_socket_getpeername(Value* socket, TypedValue* address, TypedValue* port) asm("_ZN4HPHP20f_socket_getpeernameERKNS_6ObjectERKNS_14VRefParamValueES5_");

/*
bool HPHP::f_socket_getsockname(HPHP::Object const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP20f_socket_getsocknameERKNS_6ObjectERKNS_14VRefParamValueES5_

(return value) => rax
socket => rdi
address => rsi
port => rdx
*/

bool fh_socket_getsockname(Value* socket, TypedValue* address, TypedValue* port) asm("_ZN4HPHP20f_socket_getsocknameERKNS_6ObjectERKNS_14VRefParamValueES5_");

/*
bool HPHP::f_socket_set_block(HPHP::Object const&)
_ZN4HPHP18f_socket_set_blockERKNS_6ObjectE

(return value) => rax
socket => rdi
*/

bool fh_socket_set_block(Value* socket) asm("_ZN4HPHP18f_socket_set_blockERKNS_6ObjectE");

/*
bool HPHP::f_socket_set_nonblock(HPHP::Object const&)
_ZN4HPHP21f_socket_set_nonblockERKNS_6ObjectE

(return value) => rax
socket => rdi
*/

bool fh_socket_set_nonblock(Value* socket) asm("_ZN4HPHP21f_socket_set_nonblockERKNS_6ObjectE");

/*
bool HPHP::f_socket_set_option(HPHP::Object const&, int, int, HPHP::Variant const&)
_ZN4HPHP19f_socket_set_optionERKNS_6ObjectEiiRKNS_7VariantE

(return value) => rax
socket => rdi
level => rsi
optname => rdx
optval => rcx
*/

bool fh_socket_set_option(Value* socket, int level, int optname, TypedValue* optval) asm("_ZN4HPHP19f_socket_set_optionERKNS_6ObjectEiiRKNS_7VariantE");

/*
bool HPHP::f_socket_connect(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP16f_socket_connectERKNS_6ObjectERKNS_6StringEi

(return value) => rax
socket => rdi
address => rsi
port => rdx
*/

bool fh_socket_connect(Value* socket, Value* address, int port) asm("_ZN4HPHP16f_socket_connectERKNS_6ObjectERKNS_6StringEi");

/*
bool HPHP::f_socket_bind(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP13f_socket_bindERKNS_6ObjectERKNS_6StringEi

(return value) => rax
socket => rdi
address => rsi
port => rdx
*/

bool fh_socket_bind(Value* socket, Value* address, int port) asm("_ZN4HPHP13f_socket_bindERKNS_6ObjectERKNS_6StringEi");

/*
bool HPHP::f_socket_listen(HPHP::Object const&, int)
_ZN4HPHP15f_socket_listenERKNS_6ObjectEi

(return value) => rax
socket => rdi
backlog => rsi
*/

bool fh_socket_listen(Value* socket, int backlog) asm("_ZN4HPHP15f_socket_listenERKNS_6ObjectEi");

/*
HPHP::Variant HPHP::f_socket_select(HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::Variant const&, int)
_ZN4HPHP15f_socket_selectERKNS_14VRefParamValueES2_S2_RKNS_7VariantEi

(return value) => rax
_rv => rdi
read => rsi
write => rdx
except => rcx
vtv_sec => r8
tv_usec => r9
*/

TypedValue* fh_socket_select(TypedValue* _rv, TypedValue* read, TypedValue* write, TypedValue* except, TypedValue* vtv_sec, int tv_usec) asm("_ZN4HPHP15f_socket_selectERKNS_14VRefParamValueES2_S2_RKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_socket_server(HPHP::String const&, int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP15f_socket_serverERKNS_6StringEiRKNS_14VRefParamValueES5_

(return value) => rax
_rv => rdi
hostname => rsi
port => rdx
errnum => rcx
errstr => r8
*/

TypedValue* fh_socket_server(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr) asm("_ZN4HPHP15f_socket_serverERKNS_6StringEiRKNS_14VRefParamValueES5_");

/*
HPHP::Variant HPHP::f_socket_accept(HPHP::Object const&)
_ZN4HPHP15f_socket_acceptERKNS_6ObjectE

(return value) => rax
_rv => rdi
socket => rsi
*/

TypedValue* fh_socket_accept(TypedValue* _rv, Value* socket) asm("_ZN4HPHP15f_socket_acceptERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_socket_read(HPHP::Object const&, int, int)
_ZN4HPHP13f_socket_readERKNS_6ObjectEii

(return value) => rax
_rv => rdi
socket => rsi
length => rdx
type => rcx
*/

TypedValue* fh_socket_read(TypedValue* _rv, Value* socket, int length, int type) asm("_ZN4HPHP13f_socket_readERKNS_6ObjectEii");

/*
HPHP::Variant HPHP::f_socket_write(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP14f_socket_writeERKNS_6ObjectERKNS_6StringEi

(return value) => rax
_rv => rdi
socket => rsi
buffer => rdx
length => rcx
*/

TypedValue* fh_socket_write(TypedValue* _rv, Value* socket, Value* buffer, int length) asm("_ZN4HPHP14f_socket_writeERKNS_6ObjectERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_socket_send(HPHP::Object const&, HPHP::String const&, int, int)
_ZN4HPHP13f_socket_sendERKNS_6ObjectERKNS_6StringEii

(return value) => rax
_rv => rdi
socket => rsi
buf => rdx
len => rcx
flags => r8
*/

TypedValue* fh_socket_send(TypedValue* _rv, Value* socket, Value* buf, int len, int flags) asm("_ZN4HPHP13f_socket_sendERKNS_6ObjectERKNS_6StringEii");

/*
HPHP::Variant HPHP::f_socket_sendto(HPHP::Object const&, HPHP::String const&, int, int, HPHP::String const&, int)
_ZN4HPHP15f_socket_sendtoERKNS_6ObjectERKNS_6StringEiiS5_i

(return value) => rax
_rv => rdi
socket => rsi
buf => rdx
len => rcx
flags => r8
addr => r9
port => st0
*/

TypedValue* fh_socket_sendto(TypedValue* _rv, Value* socket, Value* buf, int len, int flags, Value* addr, int port) asm("_ZN4HPHP15f_socket_sendtoERKNS_6ObjectERKNS_6StringEiiS5_i");

/*
HPHP::Variant HPHP::f_socket_recv(HPHP::Object const&, HPHP::VRefParamValue const&, int, int)
_ZN4HPHP13f_socket_recvERKNS_6ObjectERKNS_14VRefParamValueEii

(return value) => rax
_rv => rdi
socket => rsi
buf => rdx
len => rcx
flags => r8
*/

TypedValue* fh_socket_recv(TypedValue* _rv, Value* socket, TypedValue* buf, int len, int flags) asm("_ZN4HPHP13f_socket_recvERKNS_6ObjectERKNS_14VRefParamValueEii");

/*
HPHP::Variant HPHP::f_socket_recvfrom(HPHP::Object const&, HPHP::VRefParamValue const&, int, int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP17f_socket_recvfromERKNS_6ObjectERKNS_14VRefParamValueEiiS5_S5_

(return value) => rax
_rv => rdi
socket => rsi
buf => rdx
len => rcx
flags => r8
name => r9
port => st0
*/

TypedValue* fh_socket_recvfrom(TypedValue* _rv, Value* socket, TypedValue* buf, int len, int flags, TypedValue* name, TypedValue* port) asm("_ZN4HPHP17f_socket_recvfromERKNS_6ObjectERKNS_14VRefParamValueEiiS5_S5_");

/*
bool HPHP::f_socket_shutdown(HPHP::Object const&, int)
_ZN4HPHP17f_socket_shutdownERKNS_6ObjectEi

(return value) => rax
socket => rdi
how => rsi
*/

bool fh_socket_shutdown(Value* socket, int how) asm("_ZN4HPHP17f_socket_shutdownERKNS_6ObjectEi");

/*
void HPHP::f_socket_close(HPHP::Object const&)
_ZN4HPHP14f_socket_closeERKNS_6ObjectE

socket => rdi
*/

void fh_socket_close(Value* socket) asm("_ZN4HPHP14f_socket_closeERKNS_6ObjectE");

/*
HPHP::String HPHP::f_socket_strerror(int)
_ZN4HPHP17f_socket_strerrorEi

(return value) => rax
_rv => rdi
errnum => rsi
*/

Value* fh_socket_strerror(Value* _rv, int errnum) asm("_ZN4HPHP17f_socket_strerrorEi");

/*
long long HPHP::f_socket_last_error(HPHP::Object const&)
_ZN4HPHP19f_socket_last_errorERKNS_6ObjectE

(return value) => rax
socket => rdi
*/

long long fh_socket_last_error(Value* socket) asm("_ZN4HPHP19f_socket_last_errorERKNS_6ObjectE");

/*
void HPHP::f_socket_clear_error(HPHP::Object const&)
_ZN4HPHP20f_socket_clear_errorERKNS_6ObjectE

socket => rdi
*/

void fh_socket_clear_error(Value* socket) asm("_ZN4HPHP20f_socket_clear_errorERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_getaddrinfo(HPHP::String const&, HPHP::String const&, int, int, int, int)
_ZN4HPHP13f_getaddrinfoERKNS_6StringES2_iiii

(return value) => rax
_rv => rdi
host => rsi
port => rdx
family => rcx
socktype => r8
protocol => r9
flags => st0
*/

TypedValue* fh_getaddrinfo(TypedValue* _rv, Value* host, Value* port, int family, int socktype, int protocol, int flags) asm("_ZN4HPHP13f_getaddrinfoERKNS_6StringES2_iiii");


} // !HPHP

