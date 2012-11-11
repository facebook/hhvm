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
HPHP::Variant HPHP::f_stream_copy_to_stream(HPHP::Object const&, HPHP::Object const&, int, int)
_ZN4HPHP23f_stream_copy_to_streamERKNS_6ObjectES2_ii

(return value) => rax
_rv => rdi
source => rsi
dest => rdx
maxlength => rcx
offset => r8
*/

TypedValue* fh_stream_copy_to_stream(TypedValue* _rv, Value* source, Value* dest, int maxlength, int offset) asm("_ZN4HPHP23f_stream_copy_to_streamERKNS_6ObjectES2_ii");

/*
HPHP::Variant HPHP::f_stream_get_contents(HPHP::Object const&, int, int)
_ZN4HPHP21f_stream_get_contentsERKNS_6ObjectEii

(return value) => rax
_rv => rdi
handle => rsi
maxlen => rdx
offset => rcx
*/

TypedValue* fh_stream_get_contents(TypedValue* _rv, Value* handle, int maxlen, int offset) asm("_ZN4HPHP21f_stream_get_contentsERKNS_6ObjectEii");

/*
HPHP::Variant HPHP::f_stream_get_line(HPHP::Object const&, int, HPHP::String const&)
_ZN4HPHP17f_stream_get_lineERKNS_6ObjectEiRKNS_6StringE

(return value) => rax
_rv => rdi
handle => rsi
length => rdx
ending => rcx
*/

TypedValue* fh_stream_get_line(TypedValue* _rv, Value* handle, int length, Value* ending) asm("_ZN4HPHP17f_stream_get_lineERKNS_6ObjectEiRKNS_6StringE");

/*
HPHP::Variant HPHP::f_stream_select(HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::Variant const&, int)
_ZN4HPHP15f_stream_selectERKNS_14VRefParamValueES2_S2_RKNS_7VariantEi

(return value) => rax
_rv => rdi
read => rsi
write => rdx
except => rcx
vtv_sec => r8
tv_usec => r9
*/

TypedValue* fh_stream_select(TypedValue* _rv, TypedValue* read, TypedValue* write, TypedValue* except, TypedValue* vtv_sec, int tv_usec) asm("_ZN4HPHP15f_stream_selectERKNS_14VRefParamValueES2_S2_RKNS_7VariantEi");

/*
bool HPHP::f_stream_set_blocking(HPHP::Object const&, int)
_ZN4HPHP21f_stream_set_blockingERKNS_6ObjectEi

(return value) => rax
stream => rdi
mode => rsi
*/

bool fh_stream_set_blocking(Value* stream, int mode) asm("_ZN4HPHP21f_stream_set_blockingERKNS_6ObjectEi");

/*
bool HPHP::f_stream_set_timeout(HPHP::Object const&, int, int)
_ZN4HPHP20f_stream_set_timeoutERKNS_6ObjectEii

(return value) => rax
stream => rdi
seconds => rsi
microseconds => rdx
*/

bool fh_stream_set_timeout(Value* stream, int seconds, int microseconds) asm("_ZN4HPHP20f_stream_set_timeoutERKNS_6ObjectEii");

/*
long long HPHP::f_stream_set_write_buffer(HPHP::Object const&, int)
_ZN4HPHP25f_stream_set_write_bufferERKNS_6ObjectEi

(return value) => rax
stream => rdi
buffer => rsi
*/

long long fh_stream_set_write_buffer(Value* stream, int buffer) asm("_ZN4HPHP25f_stream_set_write_bufferERKNS_6ObjectEi");

/*
HPHP::Variant HPHP::f_stream_socket_accept(HPHP::Object const&, double, HPHP::VRefParamValue const&)
_ZN4HPHP22f_stream_socket_acceptERKNS_6ObjectEdRKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
server_socket => rsi
timeout => xmm0
peername => rdx
*/

TypedValue* fh_stream_socket_accept(TypedValue* _rv, Value* server_socket, double timeout, TypedValue* peername) asm("_ZN4HPHP22f_stream_socket_acceptERKNS_6ObjectEdRKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_stream_socket_server(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, int, HPHP::Object const&)
_ZN4HPHP22f_stream_socket_serverERKNS_6StringERKNS_14VRefParamValueES5_iRKNS_6ObjectE

(return value) => rax
_rv => rdi
local_socket => rsi
errnum => rdx
errstr => rcx
flags => r8
context => r9
*/

TypedValue* fh_stream_socket_server(TypedValue* _rv, Value* local_socket, TypedValue* errnum, TypedValue* errstr, int flags, Value* context) asm("_ZN4HPHP22f_stream_socket_serverERKNS_6StringERKNS_14VRefParamValueES5_iRKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_stream_socket_client(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, double, int, HPHP::Object const&)
_ZN4HPHP22f_stream_socket_clientERKNS_6StringERKNS_14VRefParamValueES5_diRKNS_6ObjectE

(return value) => rax
_rv => rdi
remote_socket => rsi
errnum => rdx
errstr => rcx
timeout => xmm0
flags => r8
context => r9
*/

TypedValue* fh_stream_socket_client(TypedValue* _rv, Value* remote_socket, TypedValue* errnum, TypedValue* errstr, double timeout, int flags, Value* context) asm("_ZN4HPHP22f_stream_socket_clientERKNS_6StringERKNS_14VRefParamValueES5_diRKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_stream_socket_get_name(HPHP::Object const&, bool)
_ZN4HPHP24f_stream_socket_get_nameERKNS_6ObjectEb

(return value) => rax
_rv => rdi
handle => rsi
want_peer => rdx
*/

TypedValue* fh_stream_socket_get_name(TypedValue* _rv, Value* handle, bool want_peer) asm("_ZN4HPHP24f_stream_socket_get_nameERKNS_6ObjectEb");

/*
HPHP::Variant HPHP::f_stream_socket_pair(int, int, int)
_ZN4HPHP20f_stream_socket_pairEiii

(return value) => rax
_rv => rdi
domain => rsi
type => rdx
protocol => rcx
*/

TypedValue* fh_stream_socket_pair(TypedValue* _rv, int domain, int type, int protocol) asm("_ZN4HPHP20f_stream_socket_pairEiii");

/*
HPHP::Variant HPHP::f_stream_socket_recvfrom(HPHP::Object const&, int, int, HPHP::String const&)
_ZN4HPHP24f_stream_socket_recvfromERKNS_6ObjectEiiRKNS_6StringE

(return value) => rax
_rv => rdi
socket => rsi
length => rdx
flags => rcx
address => r8
*/

TypedValue* fh_stream_socket_recvfrom(TypedValue* _rv, Value* socket, int length, int flags, Value* address) asm("_ZN4HPHP24f_stream_socket_recvfromERKNS_6ObjectEiiRKNS_6StringE");

/*
HPHP::Variant HPHP::f_stream_socket_sendto(HPHP::Object const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP22f_stream_socket_sendtoERKNS_6ObjectERKNS_6StringEiS5_

(return value) => rax
_rv => rdi
socket => rsi
data => rdx
flags => rcx
address => r8
*/

TypedValue* fh_stream_socket_sendto(TypedValue* _rv, Value* socket, Value* data, int flags, Value* address) asm("_ZN4HPHP22f_stream_socket_sendtoERKNS_6ObjectERKNS_6StringEiS5_");

/*
bool HPHP::f_stream_socket_shutdown(HPHP::Object const&, int)
_ZN4HPHP24f_stream_socket_shutdownERKNS_6ObjectEi

(return value) => rax
stream => rdi
how => rsi
*/

bool fh_stream_socket_shutdown(Value* stream, int how) asm("_ZN4HPHP24f_stream_socket_shutdownERKNS_6ObjectEi");


} // !HPHP

