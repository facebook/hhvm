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

bool fh_apc_store(Value* key, TypedValue* var, long ttl, long cache_id) asm("_ZN4HPHP11f_apc_storeERKNS_6StringERKNS_7VariantEll");

bool fh_apc_add(Value* key, TypedValue* var, long ttl, long cache_id) asm("_ZN4HPHP9f_apc_addERKNS_6StringERKNS_7VariantEll");

TypedValue* fh_apc_fetch(TypedValue* _rv, TypedValue* key, TypedValue* success, long cache_id) asm("_ZN4HPHP11f_apc_fetchERKNS_7VariantERKNS_14VRefParamValueEl");

TypedValue* fh_apc_delete(TypedValue* _rv, TypedValue* key, long cache_id) asm("_ZN4HPHP12f_apc_deleteERKNS_7VariantEl");

bool fh_apc_clear_cache(long cache_id) asm("_ZN4HPHP17f_apc_clear_cacheEl");

TypedValue* fh_apc_inc(TypedValue* _rv, Value* key, long step, TypedValue* success, long cache_id) asm("_ZN4HPHP9f_apc_incERKNS_6StringElRKNS_14VRefParamValueEl");

TypedValue* fh_apc_dec(TypedValue* _rv, Value* key, long step, TypedValue* success, long cache_id) asm("_ZN4HPHP9f_apc_decERKNS_6StringElRKNS_14VRefParamValueEl");

bool fh_apc_cas(Value* key, long old_cas, long new_cas, long cache_id) asm("_ZN4HPHP9f_apc_casERKNS_6StringElll");

TypedValue* fh_apc_exists(TypedValue* _rv, TypedValue* key, long cache_id) asm("_ZN4HPHP12f_apc_existsERKNS_7VariantEl");

TypedValue* fh_apc_cache_info(TypedValue* _rv, long cache_id, bool limited) asm("_ZN4HPHP16f_apc_cache_infoElb");

Value* fh_apc_sma_info(Value* _rv, bool limited) asm("_ZN4HPHP14f_apc_sma_infoEb");

bool fh_apc_define_constants(Value* key, Value* constants, bool case_sensitive, long cache_id) asm("_ZN4HPHP22f_apc_define_constantsERKNS_6StringES2_bl");

bool fh_apc_load_constants(Value* key, bool case_sensitive, long cache_id) asm("_ZN4HPHP20f_apc_load_constantsERKNS_6StringEbl");

bool fh_apc_compile_file(Value* filename, bool atomic, long cache_id) asm("_ZN4HPHP18f_apc_compile_fileERKNS_6StringEbl");

Value* fh_apc_filehits(Value* _rv) asm("_ZN4HPHP14f_apc_filehitsEv");

TypedValue* fh_apc_delete_file(TypedValue* _rv, TypedValue* keys, long cache_id) asm("_ZN4HPHP17f_apc_delete_fileERKNS_7VariantEl");

TypedValue* fh_apc_bin_dump(TypedValue* _rv, long cache_id, TypedValue* filter) asm("_ZN4HPHP14f_apc_bin_dumpElRKNS_7VariantE");

bool fh_apc_bin_load(Value* data, long flags, long cache_id) asm("_ZN4HPHP14f_apc_bin_loadERKNS_6StringEll");

TypedValue* fh_apc_bin_dumpfile(TypedValue* _rv, long cache_id, TypedValue* filter, Value* filename, long flags, Value* context) asm("_ZN4HPHP18f_apc_bin_dumpfileElRKNS_7VariantERKNS_6StringElRKNS_6ObjectE");

bool fh_apc_bin_loadfile(Value* filename, Value* context, long flags, long cache_id) asm("_ZN4HPHP18f_apc_bin_loadfileERKNS_6StringERKNS_6ObjectEll");

} // namespace HPHP
