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
HPHP::Variant HPHP::f_fb_thrift_serialize(HPHP::Variant const&)
_ZN4HPHP21f_fb_thrift_serializeERKNS_7VariantE

(return value) => rax
_rv => rdi
thing => rsi
*/

TypedValue* fh_fb_thrift_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP21f_fb_thrift_serializeERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_fb_thrift_unserialize(HPHP::Variant const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP23f_fb_thrift_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_

(return value) => rax
_rv => rdi
thing => rsi
success => rdx
errcode => rcx
*/

TypedValue* fh_fb_thrift_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP23f_fb_thrift_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

/*
HPHP::Variant HPHP::f_fb_serialize(HPHP::Variant const&)
_ZN4HPHP14f_fb_serializeERKNS_7VariantE

(return value) => rax
_rv => rdi
thing => rsi
*/

TypedValue* fh_fb_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP14f_fb_serializeERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_fb_unserialize(HPHP::Variant const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP16f_fb_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_

(return value) => rax
_rv => rdi
thing => rsi
success => rdx
errcode => rcx
*/

TypedValue* fh_fb_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP16f_fb_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

/*
HPHP::Variant HPHP::f_fb_compact_serialize(HPHP::Variant const&)
_ZN4HPHP22f_fb_compact_serializeERKNS_7VariantE

(return value) => rax
_rv => rdi
thing => rsi
*/

TypedValue* fh_fb_compact_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP22f_fb_compact_serializeERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_fb_compact_unserialize(HPHP::Variant const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP24f_fb_compact_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_

(return value) => rax
_rv => rdi
thing => rsi
success => rdx
errcode => rcx
*/

TypedValue* fh_fb_compact_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP24f_fb_compact_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

/*
bool HPHP::f_fb_could_include(HPHP::String const&)
_ZN4HPHP18f_fb_could_includeERKNS_6StringE

(return value) => rax
file => rdi
*/

bool fh_fb_could_include(Value* file) asm("_ZN4HPHP18f_fb_could_includeERKNS_6StringE");

/*
bool HPHP::f_fb_intercept(HPHP::String const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP14f_fb_interceptERKNS_6StringERKNS_7VariantES5_

(return value) => rax
name => rdi
handler => rsi
data => rdx
*/

bool fh_fb_intercept(Value* name, TypedValue* handler, TypedValue* data) asm("_ZN4HPHP14f_fb_interceptERKNS_6StringERKNS_7VariantES5_");

/*
HPHP::Variant HPHP::f_fb_stubout_intercept_handler(HPHP::String const&, HPHP::Variant const&, HPHP::Array const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP30f_fb_stubout_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
name => rsi
obj => rdx
params => rcx
data => r8
done => r9
*/

TypedValue* fh_fb_stubout_intercept_handler(TypedValue* _rv, Value* name, TypedValue* obj, Value* params, TypedValue* data, TypedValue* done) asm("_ZN4HPHP30f_fb_stubout_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_fb_rpc_intercept_handler(HPHP::String const&, HPHP::Variant const&, HPHP::Array const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP26f_fb_rpc_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
name => rsi
obj => rdx
params => rcx
data => r8
done => r9
*/

TypedValue* fh_fb_rpc_intercept_handler(TypedValue* _rv, Value* name, TypedValue* obj, Value* params, TypedValue* data, TypedValue* done) asm("_ZN4HPHP26f_fb_rpc_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE");

/*
void HPHP::f_fb_renamed_functions(HPHP::Array const&)
_ZN4HPHP22f_fb_renamed_functionsERKNS_5ArrayE

names => rdi
*/

void fh_fb_renamed_functions(Value* names) asm("_ZN4HPHP22f_fb_renamed_functionsERKNS_5ArrayE");

/*
bool HPHP::f_fb_rename_function(HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_fb_rename_functionERKNS_6StringES2_

(return value) => rax
orig_func_name => rdi
new_func_name => rsi
*/

bool fh_fb_rename_function(Value* orig_func_name, Value* new_func_name) asm("_ZN4HPHP20f_fb_rename_functionERKNS_6StringES2_");

/*
bool HPHP::f_fb_autoload_map(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP17f_fb_autoload_mapERKNS_7VariantERKNS_6StringE

(return value) => rax
map => rdi
root => rsi
*/

bool fh_fb_autoload_map(TypedValue* map, Value* root) asm("_ZN4HPHP17f_fb_autoload_mapERKNS_7VariantERKNS_6StringE");

/*
bool HPHP::f_fb_utf8ize(HPHP::VRefParamValue const&)
_ZN4HPHP12f_fb_utf8izeERKNS_14VRefParamValueE

(return value) => rax
input => rdi
*/

bool fh_fb_utf8ize(TypedValue* input) asm("_ZN4HPHP12f_fb_utf8izeERKNS_14VRefParamValueE");

/*
long long HPHP::f_fb_utf8_strlen_deprecated(HPHP::String const&)
_ZN4HPHP27f_fb_utf8_strlen_deprecatedERKNS_6StringE

(return value) => rax
input => rdi
*/

long long fh_fb_utf8_strlen_deprecated(Value* input) asm("_ZN4HPHP27f_fb_utf8_strlen_deprecatedERKNS_6StringE");

/*
long long HPHP::f_fb_utf8_strlen(HPHP::String const&)
_ZN4HPHP16f_fb_utf8_strlenERKNS_6StringE

(return value) => rax
input => rdi
*/

long long fh_fb_utf8_strlen(Value* input) asm("_ZN4HPHP16f_fb_utf8_strlenERKNS_6StringE");

/*
HPHP::Variant HPHP::f_fb_utf8_substr(HPHP::String const&, int, int)
_ZN4HPHP16f_fb_utf8_substrERKNS_6StringEii

(return value) => rax
_rv => rdi
str => rsi
start => rdx
length => rcx
*/

TypedValue* fh_fb_utf8_substr(TypedValue* _rv, Value* str, int start, int length) asm("_ZN4HPHP16f_fb_utf8_substrERKNS_6StringEii");

/*
HPHP::Array HPHP::f_fb_call_user_func_safe(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP24f_fb_call_user_func_safeEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
function => rdx
_argv => rcx
*/

Value* fh_fb_call_user_func_safe(Value* _rv, long long _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP24f_fb_call_user_func_safeEiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_fb_call_user_func_safe_return(int, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP31f_fb_call_user_func_safe_returnEiRKNS_7VariantES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
function => rdx
def => rcx
_argv => r8
*/

TypedValue* fh_fb_call_user_func_safe_return(TypedValue* _rv, long long _argc, TypedValue* function, TypedValue* def, Value* _argv) asm("_ZN4HPHP31f_fb_call_user_func_safe_returnEiRKNS_7VariantES2_RKNS_5ArrayE");

/*
HPHP::Array HPHP::f_fb_call_user_func_array_safe(HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP30f_fb_call_user_func_array_safeERKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
function => rsi
params => rdx
*/

Value* fh_fb_call_user_func_array_safe(Value* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP30f_fb_call_user_func_array_safeERKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_fb_get_code_coverage(bool)
_ZN4HPHP22f_fb_get_code_coverageEb

(return value) => rax
_rv => rdi
flush => rsi
*/

TypedValue* fh_fb_get_code_coverage(TypedValue* _rv, bool flush) asm("_ZN4HPHP22f_fb_get_code_coverageEb");

/*
void HPHP::f_fb_enable_code_coverage()
_ZN4HPHP25f_fb_enable_code_coverageEv

*/

void fh_fb_enable_code_coverage() asm("_ZN4HPHP25f_fb_enable_code_coverageEv");

/*
HPHP::Variant HPHP::f_fb_disable_code_coverage()
_ZN4HPHP26f_fb_disable_code_coverageEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_fb_disable_code_coverage(TypedValue* _rv) asm("_ZN4HPHP26f_fb_disable_code_coverageEv");

/*
void HPHP::f_fb_load_local_databases(HPHP::Array const&)
_ZN4HPHP25f_fb_load_local_databasesERKNS_5ArrayE

servers => rdi
*/

void fh_fb_load_local_databases(Value* servers) asm("_ZN4HPHP25f_fb_load_local_databasesERKNS_5ArrayE");

/*
HPHP::Array HPHP::f_fb_parallel_query(HPHP::Array const&, int, bool, bool, int, int, bool)
_ZN4HPHP19f_fb_parallel_queryERKNS_5ArrayEibbiib

(return value) => rax
_rv => rdi
sql_map => rsi
max_thread => rdx
combine_result => rcx
retry_query_on_fail => r8
connect_timeout => r9
read_timeout => st0
timeout_in_ms => st8
*/

Value* fh_fb_parallel_query(Value* _rv, Value* sql_map, int max_thread, bool combine_result, bool retry_query_on_fail, int connect_timeout, int read_timeout, bool timeout_in_ms) asm("_ZN4HPHP19f_fb_parallel_queryERKNS_5ArrayEibbiib");

/*
HPHP::Array HPHP::f_fb_crossall_query(HPHP::String const&, int, bool, int, int, bool)
_ZN4HPHP19f_fb_crossall_queryERKNS_6StringEibiib

(return value) => rax
_rv => rdi
sql => rsi
max_thread => rdx
retry_query_on_fail => rcx
connect_timeout => r8
read_timeout => r9
timeout_in_ms => st0
*/

Value* fh_fb_crossall_query(Value* _rv, Value* sql, int max_thread, bool retry_query_on_fail, int connect_timeout, int read_timeout, bool timeout_in_ms) asm("_ZN4HPHP19f_fb_crossall_queryERKNS_6StringEibiib");

/*
void HPHP::f_fb_set_taint(HPHP::VRefParamValue const&, int)
_ZN4HPHP14f_fb_set_taintERKNS_14VRefParamValueEi

str => rdi
taint => rsi
*/

void fh_fb_set_taint(TypedValue* str, int taint) asm("_ZN4HPHP14f_fb_set_taintERKNS_14VRefParamValueEi");

/*
void HPHP::f_fb_unset_taint(HPHP::VRefParamValue const&, int)
_ZN4HPHP16f_fb_unset_taintERKNS_14VRefParamValueEi

str => rdi
taint => rsi
*/

void fh_fb_unset_taint(TypedValue* str, int taint) asm("_ZN4HPHP16f_fb_unset_taintERKNS_14VRefParamValueEi");

/*
bool HPHP::f_fb_get_taint(HPHP::String const&, int)
_ZN4HPHP14f_fb_get_taintERKNS_6StringEi

(return value) => rax
str => rdi
taint => rsi
*/

bool fh_fb_get_taint(Value* str, int taint) asm("_ZN4HPHP14f_fb_get_taintERKNS_6StringEi");

/*
HPHP::Array HPHP::f_fb_get_taint_warning_counts()
_ZN4HPHP29f_fb_get_taint_warning_countsEv

(return value) => rax
_rv => rdi
*/

Value* fh_fb_get_taint_warning_counts(Value* _rv) asm("_ZN4HPHP29f_fb_get_taint_warning_countsEv");

/*
void HPHP::f_fb_enable_html_taint_trace()
_ZN4HPHP28f_fb_enable_html_taint_traceEv

*/

void fh_fb_enable_html_taint_trace() asm("_ZN4HPHP28f_fb_enable_html_taint_traceEv");

/*
HPHP::Variant HPHP::f_fb_const_fetch(HPHP::Variant const&)
_ZN4HPHP16f_fb_const_fetchERKNS_7VariantE

(return value) => rax
_rv => rdi
key => rsi
*/

TypedValue* fh_fb_const_fetch(TypedValue* _rv, TypedValue* key) asm("_ZN4HPHP16f_fb_const_fetchERKNS_7VariantE");

/*
bool HPHP::f_fb_output_compression(bool)
_ZN4HPHP23f_fb_output_compressionEb

(return value) => rax
new_value => rdi
*/

bool fh_fb_output_compression(bool new_value) asm("_ZN4HPHP23f_fb_output_compressionEb");

/*
void HPHP::f_fb_set_exit_callback(HPHP::Variant const&)
_ZN4HPHP22f_fb_set_exit_callbackERKNS_7VariantE

function => rdi
*/

void fh_fb_set_exit_callback(TypedValue* function) asm("_ZN4HPHP22f_fb_set_exit_callbackERKNS_7VariantE");

/*
HPHP::Array HPHP::f_fb_get_flush_stat()
_ZN4HPHP19f_fb_get_flush_statEv

(return value) => rax
_rv => rdi
*/

Value* fh_fb_get_flush_stat(Value* _rv) asm("_ZN4HPHP19f_fb_get_flush_statEv");

/*
long long HPHP::f_fb_get_last_flush_size()
_ZN4HPHP24f_fb_get_last_flush_sizeEv

(return value) => rax
*/

long long fh_fb_get_last_flush_size() asm("_ZN4HPHP24f_fb_get_last_flush_sizeEv");

/*
HPHP::Variant HPHP::f_fb_lazy_stat(HPHP::String const&)
_ZN4HPHP14f_fb_lazy_statERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_fb_lazy_stat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP14f_fb_lazy_statERKNS_6StringE");

/*
HPHP::Variant HPHP::f_fb_lazy_lstat(HPHP::String const&)
_ZN4HPHP15f_fb_lazy_lstatERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_fb_lazy_lstat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP15f_fb_lazy_lstatERKNS_6StringE");

/*
HPHP::String HPHP::f_fb_lazy_realpath(HPHP::String const&)
_ZN4HPHP18f_fb_lazy_realpathERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

Value* fh_fb_lazy_realpath(Value* _rv, Value* filename) asm("_ZN4HPHP18f_fb_lazy_realpathERKNS_6StringE");

/*
HPHP::String HPHP::f_fb_gc_collect_cycles()
_ZN4HPHP22f_fb_gc_collect_cyclesEv

(return value) => rax
_rv => rdi
*/

Value* fh_fb_gc_collect_cycles(Value* _rv) asm("_ZN4HPHP22f_fb_gc_collect_cyclesEv");

/*
void HPHP::f_fb_gc_detect_cycles(HPHP::String const&)
_ZN4HPHP21f_fb_gc_detect_cyclesERKNS_6StringE

filename => rdi
*/

void fh_fb_gc_detect_cycles(Value* filename) asm("_ZN4HPHP21f_fb_gc_detect_cyclesERKNS_6StringE");


} // !HPHP

