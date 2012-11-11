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
HPHP::Object HPHP::f_hphp_create_continuation(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP26f_hphp_create_continuationERKNS_6StringES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
clsname => rsi
funcname => rdx
origFuncName => rcx
args => r8
*/

Value* fh_hphp_create_continuation(Value* _rv, Value* clsname, Value* funcname, Value* origFuncName, Value* args) asm("_ZN4HPHP26f_hphp_create_continuationERKNS_6StringES2_S2_RKNS_5ArrayE");

/*
void HPHP::f_hphp_pack_continuation(HPHP::Object const&, long long, HPHP::Variant const&)
_ZN4HPHP24f_hphp_pack_continuationERKNS_6ObjectExRKNS_7VariantE

continuation => rdi
label => rsi
value => rdx
*/

void fh_hphp_pack_continuation(Value* continuation, long long label, TypedValue* value) asm("_ZN4HPHP24f_hphp_pack_continuationERKNS_6ObjectExRKNS_7VariantE");

/*
void HPHP::f_hphp_unpack_continuation(HPHP::Object const&)
_ZN4HPHP26f_hphp_unpack_continuationERKNS_6ObjectE

continuation => rdi
*/

void fh_hphp_unpack_continuation(Value* continuation) asm("_ZN4HPHP26f_hphp_unpack_continuationERKNS_6ObjectE");


} // !HPHP

