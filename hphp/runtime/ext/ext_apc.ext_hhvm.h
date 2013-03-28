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
bool HPHP::f_apc_add(HPHP::String const&, HPHP::Variant const&, long, long)
_ZN4HPHP9f_apc_addERKNS_6StringERKNS_7VariantEll

(return value) => rax
key => rdi
var => rsi
ttl => rdx
cache_id => rcx
*/

bool fh_apc_add(Value* key, TypedValue* var, long ttl, long cache_id) asm("_ZN4HPHP9f_apc_addERKNS_6StringERKNS_7VariantEll");

/*
bool HPHP::f_apc_store(HPHP::String const&, HPHP::Variant const&, long, long)
_ZN4HPHP11f_apc_storeERKNS_6StringERKNS_7VariantEll

(return value) => rax
key => rdi
var => rsi
ttl => rdx
cache_id => rcx
*/

bool fh_apc_store(Value* key, TypedValue* var, long ttl, long cache_id) asm("_ZN4HPHP11f_apc_storeERKNS_6StringERKNS_7VariantEll");

/*
HPHP::Variant HPHP::f_apc_fetch(HPHP::Variant const&, HPHP::VRefParamValue const&, long)
_ZN4HPHP11f_apc_fetchERKNS_7VariantERKNS_14VRefParamValueEl

(return value) => rax
_rv => rdi
key => rsi
success => rdx
cache_id => rcx
*/

TypedValue* fh_apc_fetch(TypedValue* _rv, TypedValue* key, TypedValue* success, long cache_id) asm("_ZN4HPHP11f_apc_fetchERKNS_7VariantERKNS_14VRefParamValueEl");

/*
HPHP::Variant HPHP::f_apc_delete(HPHP::Variant const&, long)
_ZN4HPHP12f_apc_deleteERKNS_7VariantEl

(return value) => rax
_rv => rdi
key => rsi
cache_id => rdx
*/

TypedValue* fh_apc_delete(TypedValue* _rv, TypedValue* key, long cache_id) asm("_ZN4HPHP12f_apc_deleteERKNS_7VariantEl");

/*
bool HPHP::f_apc_compile_file(HPHP::String const&, bool, long)
_ZN4HPHP18f_apc_compile_fileERKNS_6StringEbl

(return value) => rax
filename => rdi
atomic => rsi
cache_id => rdx
*/

bool fh_apc_compile_file(Value* filename, bool atomic, long cache_id) asm("_ZN4HPHP18f_apc_compile_fileERKNS_6StringEbl");

/*
HPHP::Variant HPHP::f_apc_cache_info(long, bool)
_ZN4HPHP16f_apc_cache_infoElb

(return value) => rax
_rv => rdi
cache_id => rsi
limited => rdx
*/

TypedValue* fh_apc_cache_info(TypedValue* _rv, long cache_id, bool limited) asm("_ZN4HPHP16f_apc_cache_infoElb");

/*
bool HPHP::f_apc_clear_cache(long)
_ZN4HPHP17f_apc_clear_cacheEl

(return value) => rax
cache_id => rdi
*/

bool fh_apc_clear_cache(long cache_id) asm("_ZN4HPHP17f_apc_clear_cacheEl");

/*
bool HPHP::f_apc_define_constants(HPHP::String const&, HPHP::String const&, bool, long)
_ZN4HPHP22f_apc_define_constantsERKNS_6StringES2_bl

(return value) => rax
key => rdi
constants => rsi
case_sensitive => rdx
cache_id => rcx
*/

bool fh_apc_define_constants(Value* key, Value* constants, bool case_sensitive, long cache_id) asm("_ZN4HPHP22f_apc_define_constantsERKNS_6StringES2_bl");

/*
bool HPHP::f_apc_load_constants(HPHP::String const&, bool, long)
_ZN4HPHP20f_apc_load_constantsERKNS_6StringEbl

(return value) => rax
key => rdi
case_sensitive => rsi
cache_id => rdx
*/

bool fh_apc_load_constants(Value* key, bool case_sensitive, long cache_id) asm("_ZN4HPHP20f_apc_load_constantsERKNS_6StringEbl");

/*
HPHP::Array HPHP::f_apc_sma_info(bool)
_ZN4HPHP14f_apc_sma_infoEb

(return value) => rax
_rv => rdi
limited => rsi
*/

Value* fh_apc_sma_info(Value* _rv, bool limited) asm("_ZN4HPHP14f_apc_sma_infoEb");

/*
HPHP::Array HPHP::f_apc_filehits()
_ZN4HPHP14f_apc_filehitsEv

(return value) => rax
_rv => rdi
*/

Value* fh_apc_filehits(Value* _rv) asm("_ZN4HPHP14f_apc_filehitsEv");

/*
HPHP::Variant HPHP::f_apc_delete_file(HPHP::Variant const&, long)
_ZN4HPHP17f_apc_delete_fileERKNS_7VariantEl

(return value) => rax
_rv => rdi
keys => rsi
cache_id => rdx
*/

TypedValue* fh_apc_delete_file(TypedValue* _rv, TypedValue* keys, long cache_id) asm("_ZN4HPHP17f_apc_delete_fileERKNS_7VariantEl");

/*
HPHP::Variant HPHP::f_apc_inc(HPHP::String const&, long, HPHP::VRefParamValue const&, long)
_ZN4HPHP9f_apc_incERKNS_6StringElRKNS_14VRefParamValueEl

(return value) => rax
_rv => rdi
key => rsi
step => rdx
success => rcx
cache_id => r8
*/

TypedValue* fh_apc_inc(TypedValue* _rv, Value* key, long step, TypedValue* success, long cache_id) asm("_ZN4HPHP9f_apc_incERKNS_6StringElRKNS_14VRefParamValueEl");

/*
HPHP::Variant HPHP::f_apc_dec(HPHP::String const&, long, HPHP::VRefParamValue const&, long)
_ZN4HPHP9f_apc_decERKNS_6StringElRKNS_14VRefParamValueEl

(return value) => rax
_rv => rdi
key => rsi
step => rdx
success => rcx
cache_id => r8
*/

TypedValue* fh_apc_dec(TypedValue* _rv, Value* key, long step, TypedValue* success, long cache_id) asm("_ZN4HPHP9f_apc_decERKNS_6StringElRKNS_14VRefParamValueEl");

/*
bool HPHP::f_apc_cas(HPHP::String const&, long, long, long)
_ZN4HPHP9f_apc_casERKNS_6StringElll

(return value) => rax
key => rdi
old_cas => rsi
new_cas => rdx
cache_id => rcx
*/

bool fh_apc_cas(Value* key, long old_cas, long new_cas, long cache_id) asm("_ZN4HPHP9f_apc_casERKNS_6StringElll");

/*
HPHP::Variant HPHP::f_apc_exists(HPHP::Variant const&, long)
_ZN4HPHP12f_apc_existsERKNS_7VariantEl

(return value) => rax
_rv => rdi
key => rsi
cache_id => rdx
*/

TypedValue* fh_apc_exists(TypedValue* _rv, TypedValue* key, long cache_id) asm("_ZN4HPHP12f_apc_existsERKNS_7VariantEl");

/*
HPHP::Variant HPHP::f_apc_bin_dump(long, HPHP::Variant const&)
_ZN4HPHP14f_apc_bin_dumpElRKNS_7VariantE

(return value) => rax
_rv => rdi
cache_id => rsi
filter => rdx
*/

TypedValue* fh_apc_bin_dump(TypedValue* _rv, long cache_id, TypedValue* filter) asm("_ZN4HPHP14f_apc_bin_dumpElRKNS_7VariantE");

/*
bool HPHP::f_apc_bin_load(HPHP::String const&, long, long)
_ZN4HPHP14f_apc_bin_loadERKNS_6StringEll

(return value) => rax
data => rdi
flags => rsi
cache_id => rdx
*/

bool fh_apc_bin_load(Value* data, long flags, long cache_id) asm("_ZN4HPHP14f_apc_bin_loadERKNS_6StringEll");

/*
HPHP::Variant HPHP::f_apc_bin_dumpfile(long, HPHP::Variant const&, HPHP::String const&, long, HPHP::Object const&)
_ZN4HPHP18f_apc_bin_dumpfileElRKNS_7VariantERKNS_6StringElRKNS_6ObjectE

(return value) => rax
_rv => rdi
cache_id => rsi
filter => rdx
filename => rcx
flags => r8
context => r9
*/

TypedValue* fh_apc_bin_dumpfile(TypedValue* _rv, long cache_id, TypedValue* filter, Value* filename, long flags, Value* context) asm("_ZN4HPHP18f_apc_bin_dumpfileElRKNS_7VariantERKNS_6StringElRKNS_6ObjectE");

/*
bool HPHP::f_apc_bin_loadfile(HPHP::String const&, HPHP::Object const&, long, long)
_ZN4HPHP18f_apc_bin_loadfileERKNS_6StringERKNS_6ObjectEll

(return value) => rax
filename => rdi
context => rsi
flags => rdx
cache_id => rcx
*/

bool fh_apc_bin_loadfile(Value* filename, Value* context, long flags, long cache_id) asm("_ZN4HPHP18f_apc_bin_loadfileERKNS_6StringERKNS_6ObjectEll");


} // !HPHP

