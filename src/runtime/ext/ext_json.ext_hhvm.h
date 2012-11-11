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
HPHP::String HPHP::f_json_encode(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP13f_json_encodeERKNS_7VariantES2_

(return value) => rax
_rv => rdi
value => rsi
options => rdx
*/

Value* fh_json_encode(Value* _rv, TypedValue* value, TypedValue* options) asm("_ZN4HPHP13f_json_encodeERKNS_7VariantES2_");

/*
HPHP::Variant HPHP::f_json_decode(HPHP::String const&, bool, HPHP::Variant const&)
_ZN4HPHP13f_json_decodeERKNS_6StringEbRKNS_7VariantE

(return value) => rax
_rv => rdi
json => rsi
assoc => rdx
options => rcx
*/

TypedValue* fh_json_decode(TypedValue* _rv, Value* json, bool assoc, TypedValue* options) asm("_ZN4HPHP13f_json_decodeERKNS_6StringEbRKNS_7VariantE");


} // !HPHP

