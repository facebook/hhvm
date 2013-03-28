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
HPHP::Object HPHP::f_stream_context_create(HPHP::Array const&, HPHP::Array const&)
_ZN4HPHP23f_stream_context_createERKNS_5ArrayES2_

(return value) => rax
_rv => rdi
options => rsi
params => rdx
*/

Value* fh_stream_context_create(Value* _rv, Value* options, Value* params) asm("_ZN4HPHP23f_stream_context_createERKNS_5ArrayES2_");

/*
HPHP::Object HPHP::f_stream_context_get_default(HPHP::Array const&)
_ZN4HPHP28f_stream_context_get_defaultERKNS_5ArrayE

(return value) => rax
_rv => rdi
options => rsi
*/

Value* fh_stream_context_get_default(Value* _rv, Value* options) asm("_ZN4HPHP28f_stream_context_get_defaultERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_stream_context_get_options(HPHP::Object const&)
_ZN4HPHP28f_stream_context_get_optionsERKNS_6ObjectE

(return value) => rax
_rv => rdi
stream_or_context => rsi
*/

TypedValue* fh_stream_context_get_options(TypedValue* _rv, Value* stream_or_context) asm("_ZN4HPHP28f_stream_context_get_optionsERKNS_6ObjectE");

/*
bool HPHP::f_stream_context_set_option(HPHP::Object const&, HPHP::Variant const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP27f_stream_context_set_optionERKNS_6ObjectERKNS_7VariantERKNS_6StringES5_

(return value) => rax
stream_or_context => rdi
wrapper => rsi
option => rdx
value => rcx
*/

bool fh_stream_context_set_option(Value* stream_or_context, TypedValue* wrapper, Value* option, TypedValue* value) asm("_ZN4HPHP27f_stream_context_set_optionERKNS_6ObjectERKNS_7VariantERKNS_6StringES5_");

/*
bool HPHP::f_stream_context_set_param(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP26f_stream_context_set_paramERKNS_6ObjectERKNS_5ArrayE

(return value) => rax
stream_or_context => rdi
params => rsi
*/

bool fh_stream_context_set_param(Value* stream_or_context, Value* params) asm("_ZN4HPHP26f_stream_context_set_paramERKNS_6ObjectERKNS_5ArrayE");

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
bool HPHP::f_stream_encoding(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_stream_encodingERKNS_6ObjectERKNS_6StringE

(return value) => rax
stream => rdi
encoding => rsi
*/

bool fh_stream_encoding(Value* stream, Value* encoding) asm("_ZN4HPHP17f_stream_encodingERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_stream_bucket_append(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP22f_stream_bucket_appendERKNS_6ObjectES2_

brigade => rdi
bucket => rsi
*/

void fh_stream_bucket_append(Value* brigade, Value* bucket) asm("_ZN4HPHP22f_stream_bucket_appendERKNS_6ObjectES2_");

/*
void HPHP::f_stream_bucket_prepend(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP23f_stream_bucket_prependERKNS_6ObjectES2_

brigade => rdi
bucket => rsi
*/

void fh_stream_bucket_prepend(Value* brigade, Value* bucket) asm("_ZN4HPHP23f_stream_bucket_prependERKNS_6ObjectES2_");

/*
HPHP::Object HPHP::f_stream_bucket_make_writeable(HPHP::Object const&)
_ZN4HPHP30f_stream_bucket_make_writeableERKNS_6ObjectE

(return value) => rax
_rv => rdi
brigade => rsi
*/

Value* fh_stream_bucket_make_writeable(Value* _rv, Value* brigade) asm("_ZN4HPHP30f_stream_bucket_make_writeableERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_stream_bucket_new(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP19f_stream_bucket_newERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
stream => rsi
buffer => rdx
*/

Value* fh_stream_bucket_new(Value* _rv, Value* stream, Value* buffer) asm("_ZN4HPHP19f_stream_bucket_newERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_stream_filter_register(HPHP::String const&, HPHP::String const&)
_ZN4HPHP24f_stream_filter_registerERKNS_6StringES2_

(return value) => rax
filtername => rdi
classname => rsi
*/

bool fh_stream_filter_register(Value* filtername, Value* classname) asm("_ZN4HPHP24f_stream_filter_registerERKNS_6StringES2_");

/*
bool HPHP::f_stream_filter_remove(HPHP::Object const&)
_ZN4HPHP22f_stream_filter_removeERKNS_6ObjectE

(return value) => rax
stream_filter => rdi
*/

bool fh_stream_filter_remove(Value* stream_filter) asm("_ZN4HPHP22f_stream_filter_removeERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_stream_filter_append(HPHP::Object const&, HPHP::String const&, int, HPHP::Variant const&)
_ZN4HPHP22f_stream_filter_appendERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE

(return value) => rax
_rv => rdi
stream => rsi
filtername => rdx
read_write => rcx
params => r8
*/

Value* fh_stream_filter_append(Value* _rv, Value* stream, Value* filtername, int read_write, TypedValue* params) asm("_ZN4HPHP22f_stream_filter_appendERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE");

/*
HPHP::Object HPHP::f_stream_filter_prepend(HPHP::Object const&, HPHP::String const&, int, HPHP::Variant const&)
_ZN4HPHP23f_stream_filter_prependERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE

(return value) => rax
_rv => rdi
stream => rsi
filtername => rdx
read_write => rcx
params => r8
*/

Value* fh_stream_filter_prepend(Value* _rv, Value* stream, Value* filtername, int read_write, TypedValue* params) asm("_ZN4HPHP23f_stream_filter_prependERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE");

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
HPHP::Array HPHP::f_stream_get_filters()
_ZN4HPHP20f_stream_get_filtersEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_filters(Value* _rv) asm("_ZN4HPHP20f_stream_get_filtersEv");

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
HPHP::Variant HPHP::f_stream_get_meta_data(HPHP::Object const&)
_ZN4HPHP22f_stream_get_meta_dataERKNS_6ObjectE

(return value) => rax
_rv => rdi
stream => rsi
*/

TypedValue* fh_stream_get_meta_data(TypedValue* _rv, Value* stream) asm("_ZN4HPHP22f_stream_get_meta_dataERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_stream_get_transports()
_ZN4HPHP23f_stream_get_transportsEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_transports(Value* _rv) asm("_ZN4HPHP23f_stream_get_transportsEv");

/*
HPHP::Array HPHP::f_stream_get_wrappers()
_ZN4HPHP21f_stream_get_wrappersEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_wrappers(Value* _rv) asm("_ZN4HPHP21f_stream_get_wrappersEv");

/*
bool HPHP::f_stream_register_wrapper(HPHP::String const&, HPHP::String const&)
_ZN4HPHP25f_stream_register_wrapperERKNS_6StringES2_

(return value) => rax
protocol => rdi
classname => rsi
*/

bool fh_stream_register_wrapper(Value* protocol, Value* classname) asm("_ZN4HPHP25f_stream_register_wrapperERKNS_6StringES2_");

/*
bool HPHP::f_stream_wrapper_register(HPHP::String const&, HPHP::String const&)
_ZN4HPHP25f_stream_wrapper_registerERKNS_6StringES2_

(return value) => rax
protocol => rdi
classname => rsi
*/

bool fh_stream_wrapper_register(Value* protocol, Value* classname) asm("_ZN4HPHP25f_stream_wrapper_registerERKNS_6StringES2_");

/*
bool HPHP::f_stream_wrapper_restore(HPHP::String const&)
_ZN4HPHP24f_stream_wrapper_restoreERKNS_6StringE

(return value) => rax
protocol => rdi
*/

bool fh_stream_wrapper_restore(Value* protocol) asm("_ZN4HPHP24f_stream_wrapper_restoreERKNS_6StringE");

/*
bool HPHP::f_stream_wrapper_unregister(HPHP::String const&)
_ZN4HPHP27f_stream_wrapper_unregisterERKNS_6StringE

(return value) => rax
protocol => rdi
*/

bool fh_stream_wrapper_unregister(Value* protocol) asm("_ZN4HPHP27f_stream_wrapper_unregisterERKNS_6StringE");

/*
HPHP::String HPHP::f_stream_resolve_include_path(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP29f_stream_resolve_include_pathERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
filename => rsi
context => rdx
*/

Value* fh_stream_resolve_include_path(Value* _rv, Value* filename, Value* context) asm("_ZN4HPHP29f_stream_resolve_include_pathERKNS_6StringERKNS_6ObjectE");

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
long HPHP::f_stream_set_write_buffer(HPHP::Object const&, int)
_ZN4HPHP25f_stream_set_write_bufferERKNS_6ObjectEi

(return value) => rax
stream => rdi
buffer => rsi
*/

long fh_stream_set_write_buffer(Value* stream, int buffer) asm("_ZN4HPHP25f_stream_set_write_bufferERKNS_6ObjectEi");

/*
long HPHP::f_set_file_buffer(HPHP::Object const&, int)
_ZN4HPHP17f_set_file_bufferERKNS_6ObjectEi

(return value) => rax
stream => rdi
buffer => rsi
*/

long fh_set_file_buffer(Value* stream, int buffer) asm("_ZN4HPHP17f_set_file_bufferERKNS_6ObjectEi");

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
HPHP::Variant HPHP::f_stream_socket_enable_crypto(HPHP::Object const&, bool, int, HPHP::Object const&)
_ZN4HPHP29f_stream_socket_enable_cryptoERKNS_6ObjectEbiS2_

(return value) => rax
_rv => rdi
stream => rsi
enable => rdx
crypto_type => rcx
session_stream => r8
*/

TypedValue* fh_stream_socket_enable_crypto(TypedValue* _rv, Value* stream, bool enable, int crypto_type, Value* session_stream) asm("_ZN4HPHP29f_stream_socket_enable_cryptoERKNS_6ObjectEbiS2_");

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

