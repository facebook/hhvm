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

Value* fh_stream_context_create(Value* _rv, Value* options, Value* params) asm("_ZN4HPHP23f_stream_context_createERKNS_5ArrayES2_");

Value* fh_stream_context_get_default(Value* _rv, Value* options) asm("_ZN4HPHP28f_stream_context_get_defaultERKNS_5ArrayE");

TypedValue* fh_stream_context_get_options(TypedValue* _rv, Value* stream_or_context) asm("_ZN4HPHP28f_stream_context_get_optionsERKNS_6ObjectE");

bool fh_stream_context_set_option(Value* stream_or_context, TypedValue* wrapper, Value* option, TypedValue* value) asm("_ZN4HPHP27f_stream_context_set_optionERKNS_6ObjectERKNS_7VariantERKNS_6StringES5_");

bool fh_stream_context_set_param(Value* stream_or_context, Value* params) asm("_ZN4HPHP26f_stream_context_set_paramERKNS_6ObjectERKNS_5ArrayE");

TypedValue* fh_stream_copy_to_stream(TypedValue* _rv, Value* source, Value* dest, int maxlength, int offset) asm("_ZN4HPHP23f_stream_copy_to_streamERKNS_6ObjectES2_ii");

bool fh_stream_encoding(Value* stream, Value* encoding) asm("_ZN4HPHP17f_stream_encodingERKNS_6ObjectERKNS_6StringE");

void fh_stream_bucket_append(Value* brigade, Value* bucket) asm("_ZN4HPHP22f_stream_bucket_appendERKNS_6ObjectES2_");

void fh_stream_bucket_prepend(Value* brigade, Value* bucket) asm("_ZN4HPHP23f_stream_bucket_prependERKNS_6ObjectES2_");

Value* fh_stream_bucket_make_writeable(Value* _rv, Value* brigade) asm("_ZN4HPHP30f_stream_bucket_make_writeableERKNS_6ObjectE");

Value* fh_stream_bucket_new(Value* _rv, Value* stream, Value* buffer) asm("_ZN4HPHP19f_stream_bucket_newERKNS_6ObjectERKNS_6StringE");

bool fh_stream_filter_register(Value* filtername, Value* classname) asm("_ZN4HPHP24f_stream_filter_registerERKNS_6StringES2_");

bool fh_stream_filter_remove(Value* stream_filter) asm("_ZN4HPHP22f_stream_filter_removeERKNS_6ObjectE");

Value* fh_stream_filter_append(Value* _rv, Value* stream, Value* filtername, int read_write, TypedValue* params) asm("_ZN4HPHP22f_stream_filter_appendERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE");

Value* fh_stream_filter_prepend(Value* _rv, Value* stream, Value* filtername, int read_write, TypedValue* params) asm("_ZN4HPHP23f_stream_filter_prependERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE");

TypedValue* fh_stream_get_contents(TypedValue* _rv, Value* handle, int maxlen, int offset) asm("_ZN4HPHP21f_stream_get_contentsERKNS_6ObjectEii");

Value* fh_stream_get_filters(Value* _rv) asm("_ZN4HPHP20f_stream_get_filtersEv");

TypedValue* fh_stream_get_line(TypedValue* _rv, Value* handle, int length, Value* ending) asm("_ZN4HPHP17f_stream_get_lineERKNS_6ObjectEiRKNS_6StringE");

TypedValue* fh_stream_get_meta_data(TypedValue* _rv, Value* stream) asm("_ZN4HPHP22f_stream_get_meta_dataERKNS_6ObjectE");

Value* fh_stream_get_transports(Value* _rv) asm("_ZN4HPHP23f_stream_get_transportsEv");

Value* fh_stream_resolve_include_path(Value* _rv, Value* filename, Value* context) asm("_ZN4HPHP29f_stream_resolve_include_pathERKNS_6StringERKNS_6ObjectE");

TypedValue* fh_stream_select(TypedValue* _rv, TypedValue* read, TypedValue* write, TypedValue* except, TypedValue* vtv_sec, int tv_usec) asm("_ZN4HPHP15f_stream_selectERKNS_14VRefParamValueES2_S2_RKNS_7VariantEi");

bool fh_stream_set_blocking(Value* stream, int mode) asm("_ZN4HPHP21f_stream_set_blockingERKNS_6ObjectEi");

bool fh_stream_set_timeout(Value* stream, int seconds, int microseconds) asm("_ZN4HPHP20f_stream_set_timeoutERKNS_6ObjectEii");

long fh_stream_set_write_buffer(Value* stream, int buffer) asm("_ZN4HPHP25f_stream_set_write_bufferERKNS_6ObjectEi");

long fh_set_file_buffer(Value* stream, int buffer) asm("_ZN4HPHP17f_set_file_bufferERKNS_6ObjectEi");

Value* fh_stream_get_wrappers(Value* _rv) asm("_ZN4HPHP21f_stream_get_wrappersEv");

bool fh_stream_register_wrapper(Value* protocol, Value* classname) asm("_ZN4HPHP25f_stream_register_wrapperERKNS_6StringES2_");

bool fh_stream_wrapper_register(Value* protocol, Value* classname) asm("_ZN4HPHP25f_stream_wrapper_registerERKNS_6StringES2_");

bool fh_stream_wrapper_restore(Value* protocol) asm("_ZN4HPHP24f_stream_wrapper_restoreERKNS_6StringE");

bool fh_stream_wrapper_unregister(Value* protocol) asm("_ZN4HPHP27f_stream_wrapper_unregisterERKNS_6StringE");

TypedValue* fh_stream_socket_accept(TypedValue* _rv, Value* server_socket, double timeout, TypedValue* peername) asm("_ZN4HPHP22f_stream_socket_acceptERKNS_6ObjectEdRKNS_14VRefParamValueE");

TypedValue* fh_stream_socket_server(TypedValue* _rv, Value* local_socket, TypedValue* errnum, TypedValue* errstr, int flags, Value* context) asm("_ZN4HPHP22f_stream_socket_serverERKNS_6StringERKNS_14VRefParamValueES5_iRKNS_6ObjectE");

TypedValue* fh_stream_socket_client(TypedValue* _rv, Value* remote_socket, TypedValue* errnum, TypedValue* errstr, double timeout, int flags, Value* context) asm("_ZN4HPHP22f_stream_socket_clientERKNS_6StringERKNS_14VRefParamValueES5_diRKNS_6ObjectE");

TypedValue* fh_stream_socket_enable_crypto(TypedValue* _rv, Value* stream, bool enable, int crypto_type, Value* session_stream) asm("_ZN4HPHP29f_stream_socket_enable_cryptoERKNS_6ObjectEbiS2_");

TypedValue* fh_stream_socket_get_name(TypedValue* _rv, Value* handle, bool want_peer) asm("_ZN4HPHP24f_stream_socket_get_nameERKNS_6ObjectEb");

TypedValue* fh_stream_socket_pair(TypedValue* _rv, int domain, int type, int protocol) asm("_ZN4HPHP20f_stream_socket_pairEiii");

TypedValue* fh_stream_socket_recvfrom(TypedValue* _rv, Value* socket, int length, int flags, Value* address) asm("_ZN4HPHP24f_stream_socket_recvfromERKNS_6ObjectEiiRKNS_6StringE");

TypedValue* fh_stream_socket_sendto(TypedValue* _rv, Value* socket, Value* data, int flags, Value* address) asm("_ZN4HPHP22f_stream_socket_sendtoERKNS_6ObjectERKNS_6StringEiS5_");

bool fh_stream_socket_shutdown(Value* stream, int how) asm("_ZN4HPHP24f_stream_socket_shutdownERKNS_6ObjectEi");

} // namespace HPHP
