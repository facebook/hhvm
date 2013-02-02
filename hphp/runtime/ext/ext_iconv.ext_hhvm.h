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
HPHP::Variant HPHP::f_iconv_mime_encode(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19f_iconv_mime_encodeERKNS_6StringES2_RKNS_7VariantE

(return value) => rax
_rv => rdi
field_name => rsi
field_value => rdx
preferences => rcx
*/

TypedValue* fh_iconv_mime_encode(TypedValue* _rv, Value* field_name, Value* field_value, TypedValue* preferences) asm("_ZN4HPHP19f_iconv_mime_encodeERKNS_6StringES2_RKNS_7VariantE");

/*
HPHP::Variant HPHP::f_iconv_mime_decode(HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP19f_iconv_mime_decodeERKNS_6StringEiS2_

(return value) => rax
_rv => rdi
encoded_string => rsi
mode => rdx
charset => rcx
*/

TypedValue* fh_iconv_mime_decode(TypedValue* _rv, Value* encoded_string, int mode, Value* charset) asm("_ZN4HPHP19f_iconv_mime_decodeERKNS_6StringEiS2_");

/*
HPHP::Variant HPHP::f_iconv_mime_decode_headers(HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP27f_iconv_mime_decode_headersERKNS_6StringEiS2_

(return value) => rax
_rv => rdi
encoded_headers => rsi
mode => rdx
charset => rcx
*/

TypedValue* fh_iconv_mime_decode_headers(TypedValue* _rv, Value* encoded_headers, int mode, Value* charset) asm("_ZN4HPHP27f_iconv_mime_decode_headersERKNS_6StringEiS2_");

/*
HPHP::Variant HPHP::f_iconv_get_encoding(HPHP::String const&)
_ZN4HPHP20f_iconv_get_encodingERKNS_6StringE

(return value) => rax
_rv => rdi
type => rsi
*/

TypedValue* fh_iconv_get_encoding(TypedValue* _rv, Value* type) asm("_ZN4HPHP20f_iconv_get_encodingERKNS_6StringE");

/*
bool HPHP::f_iconv_set_encoding(HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_iconv_set_encodingERKNS_6StringES2_

(return value) => rax
type => rdi
charset => rsi
*/

bool fh_iconv_set_encoding(Value* type, Value* charset) asm("_ZN4HPHP20f_iconv_set_encodingERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_iconv(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP7f_iconvERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
in_charset => rsi
out_charset => rdx
str => rcx
*/

TypedValue* fh_iconv(TypedValue* _rv, Value* in_charset, Value* out_charset, Value* str) asm("_ZN4HPHP7f_iconvERKNS_6StringES2_S2_");

/*
HPHP::Variant HPHP::f_iconv_strlen(HPHP::String const&, HPHP::String const&)
_ZN4HPHP14f_iconv_strlenERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charset => rdx
*/

TypedValue* fh_iconv_strlen(TypedValue* _rv, Value* str, Value* charset) asm("_ZN4HPHP14f_iconv_strlenERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_iconv_strpos(HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP14f_iconv_strposERKNS_6StringES2_iS2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
charset => r8
*/

TypedValue* fh_iconv_strpos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* charset) asm("_ZN4HPHP14f_iconv_strposERKNS_6StringES2_iS2_");

/*
HPHP::Variant HPHP::f_iconv_strrpos(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_iconv_strrposERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
charset => rcx
*/

TypedValue* fh_iconv_strrpos(TypedValue* _rv, Value* haystack, Value* needle, Value* charset) asm("_ZN4HPHP15f_iconv_strrposERKNS_6StringES2_S2_");

/*
HPHP::Variant HPHP::f_iconv_substr(HPHP::String const&, int, int, HPHP::String const&)
_ZN4HPHP14f_iconv_substrERKNS_6StringEiiS2_

(return value) => rax
_rv => rdi
str => rsi
offset => rdx
length => rcx
charset => r8
*/

TypedValue* fh_iconv_substr(TypedValue* _rv, Value* str, int offset, int length, Value* charset) asm("_ZN4HPHP14f_iconv_substrERKNS_6StringEiiS2_");

/*
HPHP::String HPHP::f_ob_iconv_handler(HPHP::String const&, int)
_ZN4HPHP18f_ob_iconv_handlerERKNS_6StringEi

(return value) => rax
_rv => rdi
contents => rsi
status => rdx
*/

Value* fh_ob_iconv_handler(Value* _rv, Value* contents, int status) asm("_ZN4HPHP18f_ob_iconv_handlerERKNS_6StringEi");


} // !HPHP

