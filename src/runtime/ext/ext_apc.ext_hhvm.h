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
bool HPHP::f_apc_add(HPHP::String const&, HPHP::Variant const&, long long, long long)
_ZN4HPHP9f_apc_addERKNS_6StringERKNS_7VariantExx

(return value) => rax
key => rdi
var => rsi
ttl => rdx
cache_id => rcx
*/

bool fh_apc_add(Value* key, TypedValue* var, long long ttl, long long cache_id) asm("_ZN4HPHP9f_apc_addERKNS_6StringERKNS_7VariantExx");

/*
bool HPHP::f_apc_store(HPHP::String const&, HPHP::Variant const&, long long, long long)
_ZN4HPHP11f_apc_storeERKNS_6StringERKNS_7VariantExx

(return value) => rax
key => rdi
var => rsi
ttl => rdx
cache_id => rcx
*/

bool fh_apc_store(Value* key, TypedValue* var, long long ttl, long long cache_id) asm("_ZN4HPHP11f_apc_storeERKNS_6StringERKNS_7VariantExx");

/*
HPHP::Variant HPHP::f_apc_fetch(HPHP::Variant const&, HPHP::VRefParamValue const&, long long)
_ZN4HPHP11f_apc_fetchERKNS_7VariantERKNS_14VRefParamValueEx

(return value) => rax
_rv => rdi
key => rsi
success => rdx
cache_id => rcx
*/

TypedValue* fh_apc_fetch(TypedValue* _rv, TypedValue* key, TypedValue* success, long long cache_id) asm("_ZN4HPHP11f_apc_fetchERKNS_7VariantERKNS_14VRefParamValueEx");

/*
HPHP::Variant HPHP::f_apc_delete(HPHP::Variant const&, long long)
_ZN4HPHP12f_apc_deleteERKNS_7VariantEx

(return value) => rax
_rv => rdi
key => rsi
cache_id => rdx
*/

TypedValue* fh_apc_delete(TypedValue* _rv, TypedValue* key, long long cache_id) asm("_ZN4HPHP12f_apc_deleteERKNS_7VariantEx");

/*
HPHP::Variant HPHP::f_apc_cache_info(long long, bool)
_ZN4HPHP16f_apc_cache_infoExb

(return value) => rax
_rv => rdi
cache_id => rsi
limited => rdx
*/

TypedValue* fh_apc_cache_info(TypedValue* _rv, long long cache_id, bool limited) asm("_ZN4HPHP16f_apc_cache_infoExb");

/*
bool HPHP::f_apc_clear_cache(long long)
_ZN4HPHP17f_apc_clear_cacheEx

(return value) => rax
cache_id => rdi
*/

bool fh_apc_clear_cache(long long cache_id) asm("_ZN4HPHP17f_apc_clear_cacheEx");

/*
HPHP::Variant HPHP::f_apc_inc(HPHP::String const&, long long, HPHP::VRefParamValue const&, long long)
_ZN4HPHP9f_apc_incERKNS_6StringExRKNS_14VRefParamValueEx

(return value) => rax
_rv => rdi
key => rsi
step => rdx
success => rcx
cache_id => r8
*/

TypedValue* fh_apc_inc(TypedValue* _rv, Value* key, long long step, TypedValue* success, long long cache_id) asm("_ZN4HPHP9f_apc_incERKNS_6StringExRKNS_14VRefParamValueEx");

/*
HPHP::Variant HPHP::f_apc_dec(HPHP::String const&, long long, HPHP::VRefParamValue const&, long long)
_ZN4HPHP9f_apc_decERKNS_6StringExRKNS_14VRefParamValueEx

(return value) => rax
_rv => rdi
key => rsi
step => rdx
success => rcx
cache_id => r8
*/

TypedValue* fh_apc_dec(TypedValue* _rv, Value* key, long long step, TypedValue* success, long long cache_id) asm("_ZN4HPHP9f_apc_decERKNS_6StringExRKNS_14VRefParamValueEx");

/*
bool HPHP::f_apc_cas(HPHP::String const&, long long, long long, long long)
_ZN4HPHP9f_apc_casERKNS_6StringExxx

(return value) => rax
key => rdi
old_cas => rsi
new_cas => rdx
cache_id => rcx
*/

bool fh_apc_cas(Value* key, long long old_cas, long long new_cas, long long cache_id) asm("_ZN4HPHP9f_apc_casERKNS_6StringExxx");

/*
HPHP::Variant HPHP::f_apc_exists(HPHP::Variant const&, long long)
_ZN4HPHP12f_apc_existsERKNS_7VariantEx

(return value) => rax
_rv => rdi
key => rsi
cache_id => rdx
*/

TypedValue* fh_apc_exists(TypedValue* _rv, TypedValue* key, long long cache_id) asm("_ZN4HPHP12f_apc_existsERKNS_7VariantEx");


} // !HPHP

