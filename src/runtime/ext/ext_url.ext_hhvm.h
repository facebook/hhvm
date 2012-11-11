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
HPHP::Variant HPHP::f_base64_decode(HPHP::String const&, bool)
_ZN4HPHP15f_base64_decodeERKNS_6StringEb

(return value) => rax
_rv => rdi
data => rsi
strict => rdx
*/

TypedValue* fh_base64_decode(TypedValue* _rv, Value* data, bool strict) asm("_ZN4HPHP15f_base64_decodeERKNS_6StringEb");

/*
HPHP::String HPHP::f_base64_encode(HPHP::String const&)
_ZN4HPHP15f_base64_encodeERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

Value* fh_base64_encode(Value* _rv, Value* data) asm("_ZN4HPHP15f_base64_encodeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_get_headers(HPHP::String const&, int)
_ZN4HPHP13f_get_headersERKNS_6StringEi

(return value) => rax
_rv => rdi
url => rsi
format => rdx
*/

TypedValue* fh_get_headers(TypedValue* _rv, Value* url, int format) asm("_ZN4HPHP13f_get_headersERKNS_6StringEi");

/*
HPHP::Array HPHP::f_get_meta_tags(HPHP::String const&, bool)
_ZN4HPHP15f_get_meta_tagsERKNS_6StringEb

(return value) => rax
_rv => rdi
filename => rsi
use_include_path => rdx
*/

Value* fh_get_meta_tags(Value* _rv, Value* filename, bool use_include_path) asm("_ZN4HPHP15f_get_meta_tagsERKNS_6StringEb");

/*
HPHP::Variant HPHP::f_http_build_query(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP18f_http_build_queryERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
formdata => rsi
numeric_prefix => rdx
arg_separator => rcx
*/

TypedValue* fh_http_build_query(TypedValue* _rv, TypedValue* formdata, Value* numeric_prefix, Value* arg_separator) asm("_ZN4HPHP18f_http_build_queryERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_parse_url(HPHP::String const&, int)
_ZN4HPHP11f_parse_urlERKNS_6StringEi

(return value) => rax
_rv => rdi
url => rsi
component => rdx
*/

TypedValue* fh_parse_url(TypedValue* _rv, Value* url, int component) asm("_ZN4HPHP11f_parse_urlERKNS_6StringEi");


} // !HPHP

