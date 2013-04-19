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

TypedValue* fh_icu_match(TypedValue* _rv, Value* pattern, Value* subject, TypedValue* matches, long flags) asm("_ZN4HPHP11f_icu_matchERKNS_6StringES2_RKNS_14VRefParamValueEl");

Value* fh_icu_transliterate(Value* _rv, Value* str, bool remove_accents) asm("_ZN4HPHP19f_icu_transliterateERKNS_6StringEb");

Value* fh_icu_tokenize(Value* _rv, Value* text) asm("_ZN4HPHP14f_icu_tokenizeERKNS_6StringE");

} // namespace HPHP
