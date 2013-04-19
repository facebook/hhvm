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

TypedValue* fh_fb_thrift_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP21f_fb_thrift_serializeERKNS_7VariantE");

TypedValue* fh_fb_thrift_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP23f_fb_thrift_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

TypedValue* fh_fb_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP14f_fb_serializeERKNS_7VariantE");

TypedValue* fh_fb_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP16f_fb_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

TypedValue* fh_fb_compact_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP22f_fb_compact_serializeERKNS_7VariantE");

TypedValue* fh_fb_compact_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP24f_fb_compact_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

void fh_fb_load_local_databases(Value* servers) asm("_ZN4HPHP25f_fb_load_local_databasesERKNS_5ArrayE");

Value* fh_fb_parallel_query(Value* _rv, Value* sql_map, int max_thread, bool combine_result, bool retry_query_on_fail, int connect_timeout, int read_timeout, bool timeout_in_ms) asm("_ZN4HPHP19f_fb_parallel_queryERKNS_5ArrayEibbiib");

Value* fh_fb_crossall_query(Value* _rv, Value* sql, int max_thread, bool retry_query_on_fail, int connect_timeout, int read_timeout, bool timeout_in_ms) asm("_ZN4HPHP19f_fb_crossall_queryERKNS_6StringEibiib");

bool fh_fb_utf8ize(TypedValue* input) asm("_ZN4HPHP12f_fb_utf8izeERKNS_14VRefParamValueE");

long fh_fb_utf8_strlen(Value* input) asm("_ZN4HPHP16f_fb_utf8_strlenERKNS_6StringE");

long fh_fb_utf8_strlen_deprecated(Value* input) asm("_ZN4HPHP27f_fb_utf8_strlen_deprecatedERKNS_6StringE");

TypedValue* fh_fb_utf8_substr(TypedValue* _rv, Value* str, int start, int length) asm("_ZN4HPHP16f_fb_utf8_substrERKNS_6StringEii");

bool fh_fb_could_include(Value* file) asm("_ZN4HPHP18f_fb_could_includeERKNS_6StringE");

bool fh_fb_intercept(Value* name, TypedValue* handler, TypedValue* data) asm("_ZN4HPHP14f_fb_interceptERKNS_6StringERKNS_7VariantES5_");

TypedValue* fh_fb_stubout_intercept_handler(TypedValue* _rv, Value* name, TypedValue* obj, Value* params, TypedValue* data, TypedValue* done) asm("_ZN4HPHP30f_fb_stubout_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE");

TypedValue* fh_fb_rpc_intercept_handler(TypedValue* _rv, Value* name, TypedValue* obj, Value* params, TypedValue* data, TypedValue* done) asm("_ZN4HPHP26f_fb_rpc_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE");

void fh_fb_renamed_functions(Value* names) asm("_ZN4HPHP22f_fb_renamed_functionsERKNS_5ArrayE");

bool fh_fb_rename_function(Value* orig_func_name, Value* new_func_name) asm("_ZN4HPHP20f_fb_rename_functionERKNS_6StringES2_");

bool fh_fb_autoload_map(TypedValue* map, Value* root) asm("_ZN4HPHP17f_fb_autoload_mapERKNS_7VariantERKNS_6StringE");

Value* fh_fb_call_user_func_safe(Value* _rv, int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP24f_fb_call_user_func_safeEiRKNS_7VariantERKNS_5ArrayE");

Value* fh_fb_call_user_func_array_safe(Value* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP30f_fb_call_user_func_array_safeERKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_fb_call_user_func_safe_return(TypedValue* _rv, int64_t _argc, TypedValue* function, TypedValue* def, Value* _argv) asm("_ZN4HPHP31f_fb_call_user_func_safe_returnEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fh_fb_get_code_coverage(TypedValue* _rv, bool flush) asm("_ZN4HPHP22f_fb_get_code_coverageEb");

void fh_fb_enable_code_coverage() asm("_ZN4HPHP25f_fb_enable_code_coverageEv");

TypedValue* fh_fb_disable_code_coverage(TypedValue* _rv) asm("_ZN4HPHP26f_fb_disable_code_coverageEv");

bool fh_fb_output_compression(bool new_value) asm("_ZN4HPHP23f_fb_output_compressionEb");

void fh_fb_set_exit_callback(TypedValue* function) asm("_ZN4HPHP22f_fb_set_exit_callbackERKNS_7VariantE");

Value* fh_fb_get_flush_stat(Value* _rv) asm("_ZN4HPHP19f_fb_get_flush_statEv");

long fh_fb_get_last_flush_size() asm("_ZN4HPHP24f_fb_get_last_flush_sizeEv");

TypedValue* fh_fb_lazy_stat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP14f_fb_lazy_statERKNS_6StringE");

TypedValue* fh_fb_lazy_lstat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP15f_fb_lazy_lstatERKNS_6StringE");

Value* fh_fb_lazy_realpath(Value* _rv, Value* filename) asm("_ZN4HPHP18f_fb_lazy_realpathERKNS_6StringE");

Value* fh_fb_gc_collect_cycles(Value* _rv) asm("_ZN4HPHP22f_fb_gc_collect_cyclesEv");

void fh_fb_gc_detect_cycles(Value* filename) asm("_ZN4HPHP21f_fb_gc_detect_cyclesERKNS_6StringE");

TypedValue* fh_fb_const_fetch(TypedValue* _rv, TypedValue* key) asm("_ZN4HPHP16f_fb_const_fetchERKNS_7VariantE");

} // namespace HPHP
