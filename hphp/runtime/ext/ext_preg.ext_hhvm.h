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

TypedValue* fh_preg_grep(TypedValue* _rv, Value* pattern, Value* input, int flags) asm("_ZN4HPHP11f_preg_grepERKNS_6StringERKNS_5ArrayEi");

TypedValue* fh_preg_match(TypedValue* _rv, Value* pattern, Value* subject, TypedValue* matches, int flags, int offset) asm("_ZN4HPHP12f_preg_matchERKNS_6StringES2_RKNS_14VRefParamValueEii");

TypedValue* fh_preg_match_all(TypedValue* _rv, Value* pattern, Value* subject, TypedValue* matches, int flags, int offset) asm("_ZN4HPHP16f_preg_match_allERKNS_6StringES2_RKNS_14VRefParamValueEii");

TypedValue* fh_preg_replace(TypedValue* _rv, TypedValue* pattern, TypedValue* replacement, TypedValue* subject, int limit, TypedValue* count) asm("_ZN4HPHP14f_preg_replaceERKNS_7VariantES2_S2_iRKNS_14VRefParamValueE");

TypedValue* fh_preg_replace_callback(TypedValue* _rv, TypedValue* pattern, TypedValue* callback, TypedValue* subject, int limit, TypedValue* count) asm("_ZN4HPHP23f_preg_replace_callbackERKNS_7VariantES2_S2_iRKNS_14VRefParamValueE");

TypedValue* fh_preg_split(TypedValue* _rv, TypedValue* pattern, TypedValue* subject, int limit, int flags) asm("_ZN4HPHP12f_preg_splitERKNS_7VariantES2_ii");

Value* fh_preg_quote(Value* _rv, Value* str, Value* delimiter) asm("_ZN4HPHP12f_preg_quoteERKNS_6StringES2_");

long fh_preg_last_error() asm("_ZN4HPHP17f_preg_last_errorEv");

Value* fh_ereg_replace(Value* _rv, Value* pattern, Value* replacement, Value* str) asm("_ZN4HPHP14f_ereg_replaceERKNS_6StringES2_S2_");

Value* fh_eregi_replace(Value* _rv, Value* pattern, Value* replacement, Value* str) asm("_ZN4HPHP15f_eregi_replaceERKNS_6StringES2_S2_");

TypedValue* fh_ereg(TypedValue* _rv, Value* pattern, Value* str, TypedValue* regs) asm("_ZN4HPHP6f_eregERKNS_6StringES2_RKNS_14VRefParamValueE");

TypedValue* fh_eregi(TypedValue* _rv, Value* pattern, Value* str, TypedValue* regs) asm("_ZN4HPHP7f_eregiERKNS_6StringES2_RKNS_14VRefParamValueE");

TypedValue* fh_split(TypedValue* _rv, Value* pattern, Value* str, int limit) asm("_ZN4HPHP7f_splitERKNS_6StringES2_i");

TypedValue* fh_spliti(TypedValue* _rv, Value* pattern, Value* str, int limit) asm("_ZN4HPHP8f_splitiERKNS_6StringES2_i");

Value* fh_sql_regcase(Value* _rv, Value* str) asm("_ZN4HPHP13f_sql_regcaseERKNS_6StringE");

} // namespace HPHP
