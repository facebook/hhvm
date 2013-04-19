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

Value* fh_hash_algos(Value* _rv) asm("_ZN4HPHP12f_hash_algosEv");

TypedValue* fh_hash(TypedValue* _rv, Value* algo, Value* data, bool raw_output) asm("_ZN4HPHP6f_hashERKNS_6StringES2_b");

TypedValue* fh_hash_file(TypedValue* _rv, Value* algo, Value* filename, bool raw_output) asm("_ZN4HPHP11f_hash_fileERKNS_6StringES2_b");

TypedValue* fh_hash_hmac(TypedValue* _rv, Value* algo, Value* data, Value* key, bool raw_output) asm("_ZN4HPHP11f_hash_hmacERKNS_6StringES2_S2_b");

TypedValue* fh_hash_hmac_file(TypedValue* _rv, Value* algo, Value* filename, Value* key, bool raw_output) asm("_ZN4HPHP16f_hash_hmac_fileERKNS_6StringES2_S2_b");

TypedValue* fh_hash_init(TypedValue* _rv, Value* algo, int options, Value* key) asm("_ZN4HPHP11f_hash_initERKNS_6StringEiS2_");

bool fh_hash_update(Value* context, Value* data) asm("_ZN4HPHP13f_hash_updateERKNS_6ObjectERKNS_6StringE");

bool fh_hash_update_file(Value* init_context, Value* filename, Value* stream_context) asm("_ZN4HPHP18f_hash_update_fileERKNS_6ObjectERKNS_6StringES2_");

long fh_hash_update_stream(Value* context, Value* handle, int length) asm("_ZN4HPHP20f_hash_update_streamERKNS_6ObjectES2_i");

Value* fh_hash_final(Value* _rv, Value* context, bool raw_output) asm("_ZN4HPHP12f_hash_finalERKNS_6ObjectEb");

long fh_furchash_hphp_ext(Value* key, int len, int nPart) asm("_ZN4HPHP19f_furchash_hphp_extERKNS_6StringEii");

bool fh_furchash_hphp_ext_supported() asm("_ZN4HPHP29f_furchash_hphp_ext_supportedEv");

long fh_hphp_murmurhash(Value* key, int len, int seed) asm("_ZN4HPHP17f_hphp_murmurhashERKNS_6StringEii");

} // namespace HPHP
