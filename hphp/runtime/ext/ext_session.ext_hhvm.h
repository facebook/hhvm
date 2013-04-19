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

void fh_session_write_close() asm("_ZN4HPHP21f_session_write_closeEv");

void fh_session_set_cookie_params(long lifetime, Value* path, Value* domain, TypedValue* secure, TypedValue* httponly) asm("_ZN4HPHP27f_session_set_cookie_paramsElRKNS_6StringES2_RKNS_7VariantES5_");

Value* fh_session_get_cookie_params(Value* _rv) asm("_ZN4HPHP27f_session_get_cookie_paramsEv");

Value* fh_session_name(Value* _rv, Value* newname) asm("_ZN4HPHP14f_session_nameERKNS_6StringE");

TypedValue* fh_session_module_name(TypedValue* _rv, Value* newname) asm("_ZN4HPHP21f_session_module_nameERKNS_6StringE");

bool fh_session_set_save_handler(Value* open, Value* close, Value* read, Value* write, Value* destroy, Value* gc) asm("_ZN4HPHP26f_session_set_save_handlerERKNS_6StringES2_S2_S2_S2_S2_");

Value* fh_session_save_path(Value* _rv, Value* newname) asm("_ZN4HPHP19f_session_save_pathERKNS_6StringE");

Value* fh_session_id(Value* _rv, Value* newid) asm("_ZN4HPHP12f_session_idERKNS_6StringE");

bool fh_session_regenerate_id(bool delete_old_session) asm("_ZN4HPHP23f_session_regenerate_idEb");

Value* fh_session_cache_limiter(Value* _rv, Value* new_cache_limiter) asm("_ZN4HPHP23f_session_cache_limiterERKNS_6StringE");

long fh_session_cache_expire(Value* new_cache_expire) asm("_ZN4HPHP22f_session_cache_expireERKNS_6StringE");

TypedValue* fh_session_encode(TypedValue* _rv) asm("_ZN4HPHP16f_session_encodeEv");

bool fh_session_decode(Value* data) asm("_ZN4HPHP16f_session_decodeERKNS_6StringE");

bool fh_session_start() asm("_ZN4HPHP15f_session_startEv");

bool fh_session_destroy() asm("_ZN4HPHP17f_session_destroyEv");

TypedValue* fh_session_unset(TypedValue* _rv) asm("_ZN4HPHP15f_session_unsetEv");

void fh_session_commit() asm("_ZN4HPHP16f_session_commitEv");

bool fh_session_register(int64_t _argc, TypedValue* var_names, Value* _argv) asm("_ZN4HPHP18f_session_registerEiRKNS_7VariantERKNS_5ArrayE");

bool fh_session_unregister(Value* varname) asm("_ZN4HPHP20f_session_unregisterERKNS_6StringE");

bool fh_session_is_registered(Value* varname) asm("_ZN4HPHP23f_session_is_registeredERKNS_6StringE");

} // namespace HPHP
