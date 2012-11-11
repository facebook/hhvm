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
bool HPHP::f_ctype_alnum(HPHP::Variant const&)
_ZN4HPHP13f_ctype_alnumERKNS_7VariantE

(return value) => rax
text => rdi
*/

bool fh_ctype_alnum(TypedValue* text) asm("_ZN4HPHP13f_ctype_alnumERKNS_7VariantE");

/*
bool HPHP::f_ctype_alpha(HPHP::Variant const&)
_ZN4HPHP13f_ctype_alphaERKNS_7VariantE

(return value) => rax
text => rdi
*/

bool fh_ctype_alpha(TypedValue* text) asm("_ZN4HPHP13f_ctype_alphaERKNS_7VariantE");

/*
bool HPHP::f_ctype_cntrl(HPHP::Variant const&)
_ZN4HPHP13f_ctype_cntrlERKNS_7VariantE

(return value) => rax
text => rdi
*/

bool fh_ctype_cntrl(TypedValue* text) asm("_ZN4HPHP13f_ctype_cntrlERKNS_7VariantE");

/*
bool HPHP::f_ctype_digit(HPHP::Variant const&)
_ZN4HPHP13f_ctype_digitERKNS_7VariantE

(return value) => rax
text => rdi
*/

bool fh_ctype_digit(TypedValue* text) asm("_ZN4HPHP13f_ctype_digitERKNS_7VariantE");

/*
bool HPHP::f_ctype_graph(HPHP::Variant const&)
_ZN4HPHP13f_ctype_graphERKNS_7VariantE

(return value) => rax
text => rdi
*/

bool fh_ctype_graph(TypedValue* text) asm("_ZN4HPHP13f_ctype_graphERKNS_7VariantE");

/*
bool HPHP::f_ctype_lower(HPHP::Variant const&)
_ZN4HPHP13f_ctype_lowerERKNS_7VariantE

(return value) => rax
text => rdi
*/

bool fh_ctype_lower(TypedValue* text) asm("_ZN4HPHP13f_ctype_lowerERKNS_7VariantE");

/*
bool HPHP::f_ctype_print(HPHP::Variant const&)
_ZN4HPHP13f_ctype_printERKNS_7VariantE

(return value) => rax
text => rdi
*/

bool fh_ctype_print(TypedValue* text) asm("_ZN4HPHP13f_ctype_printERKNS_7VariantE");

/*
bool HPHP::f_ctype_punct(HPHP::Variant const&)
_ZN4HPHP13f_ctype_punctERKNS_7VariantE

(return value) => rax
text => rdi
*/

bool fh_ctype_punct(TypedValue* text) asm("_ZN4HPHP13f_ctype_punctERKNS_7VariantE");

/*
bool HPHP::f_ctype_space(HPHP::Variant const&)
_ZN4HPHP13f_ctype_spaceERKNS_7VariantE

(return value) => rax
text => rdi
*/

bool fh_ctype_space(TypedValue* text) asm("_ZN4HPHP13f_ctype_spaceERKNS_7VariantE");

/*
bool HPHP::f_ctype_upper(HPHP::Variant const&)
_ZN4HPHP13f_ctype_upperERKNS_7VariantE

(return value) => rax
text => rdi
*/

bool fh_ctype_upper(TypedValue* text) asm("_ZN4HPHP13f_ctype_upperERKNS_7VariantE");

/*
bool HPHP::f_ctype_xdigit(HPHP::Variant const&)
_ZN4HPHP14f_ctype_xdigitERKNS_7VariantE

(return value) => rax
text => rdi
*/

bool fh_ctype_xdigit(TypedValue* text) asm("_ZN4HPHP14f_ctype_xdigitERKNS_7VariantE");


} // !HPHP

