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
HPHP::String HPHP::f_addcslashes(HPHP::String const&, HPHP::String const&)
_ZN4HPHP13f_addcslashesERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_addcslashes(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP13f_addcslashesERKNS_6StringES2_");

/*
HPHP::String HPHP::f_stripcslashes(HPHP::String const&)
_ZN4HPHP15f_stripcslashesERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_stripcslashes(Value* _rv, Value* str) asm("_ZN4HPHP15f_stripcslashesERKNS_6StringE");

/*
HPHP::String HPHP::f_addslashes(HPHP::String const&)
_ZN4HPHP12f_addslashesERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_addslashes(Value* _rv, Value* str) asm("_ZN4HPHP12f_addslashesERKNS_6StringE");

/*
HPHP::String HPHP::f_stripslashes(HPHP::String const&)
_ZN4HPHP14f_stripslashesERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_stripslashes(Value* _rv, Value* str) asm("_ZN4HPHP14f_stripslashesERKNS_6StringE");

/*
HPHP::String HPHP::f_bin2hex(HPHP::String const&)
_ZN4HPHP9f_bin2hexERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_bin2hex(Value* _rv, Value* str) asm("_ZN4HPHP9f_bin2hexERKNS_6StringE");

/*
HPHP::Variant HPHP::f_hex2bin(HPHP::String const&)
_ZN4HPHP9f_hex2binERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_hex2bin(TypedValue* _rv, Value* str) asm("_ZN4HPHP9f_hex2binERKNS_6StringE");

/*
HPHP::String HPHP::f_nl2br(HPHP::String const&)
_ZN4HPHP7f_nl2brERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_nl2br(Value* _rv, Value* str) asm("_ZN4HPHP7f_nl2brERKNS_6StringE");

/*
HPHP::String HPHP::f_quotemeta(HPHP::String const&)
_ZN4HPHP11f_quotemetaERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_quotemeta(Value* _rv, Value* str) asm("_ZN4HPHP11f_quotemetaERKNS_6StringE");

/*
HPHP::String HPHP::f_str_shuffle(HPHP::String const&)
_ZN4HPHP13f_str_shuffleERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_str_shuffle(Value* _rv, Value* str) asm("_ZN4HPHP13f_str_shuffleERKNS_6StringE");

/*
HPHP::String HPHP::f_strrev(HPHP::String const&)
_ZN4HPHP8f_strrevERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_strrev(Value* _rv, Value* str) asm("_ZN4HPHP8f_strrevERKNS_6StringE");

/*
HPHP::String HPHP::f_strtolower(HPHP::String const&)
_ZN4HPHP12f_strtolowerERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_strtolower(Value* _rv, Value* str) asm("_ZN4HPHP12f_strtolowerERKNS_6StringE");

/*
HPHP::String HPHP::f_strtoupper(HPHP::String const&)
_ZN4HPHP12f_strtoupperERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_strtoupper(Value* _rv, Value* str) asm("_ZN4HPHP12f_strtoupperERKNS_6StringE");

/*
HPHP::String HPHP::f_ucfirst(HPHP::String const&)
_ZN4HPHP9f_ucfirstERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_ucfirst(Value* _rv, Value* str) asm("_ZN4HPHP9f_ucfirstERKNS_6StringE");

/*
HPHP::String HPHP::f_lcfirst(HPHP::String const&)
_ZN4HPHP9f_lcfirstERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_lcfirst(Value* _rv, Value* str) asm("_ZN4HPHP9f_lcfirstERKNS_6StringE");

/*
HPHP::String HPHP::f_ucwords(HPHP::String const&)
_ZN4HPHP9f_ucwordsERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_ucwords(Value* _rv, Value* str) asm("_ZN4HPHP9f_ucwordsERKNS_6StringE");

/*
HPHP::String HPHP::f_strip_tags(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12f_strip_tagsERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
allowable_tags => rdx
*/

Value* fh_strip_tags(Value* _rv, Value* str, Value* allowable_tags) asm("_ZN4HPHP12f_strip_tagsERKNS_6StringES2_");

/*
HPHP::String HPHP::f_trim(HPHP::String const&, HPHP::String const&)
_ZN4HPHP6f_trimERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_trim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP6f_trimERKNS_6StringES2_");

/*
HPHP::String HPHP::f_ltrim(HPHP::String const&, HPHP::String const&)
_ZN4HPHP7f_ltrimERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_ltrim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP7f_ltrimERKNS_6StringES2_");

/*
HPHP::String HPHP::f_rtrim(HPHP::String const&, HPHP::String const&)
_ZN4HPHP7f_rtrimERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_rtrim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP7f_rtrimERKNS_6StringES2_");

/*
HPHP::String HPHP::f_chop(HPHP::String const&, HPHP::String const&)
_ZN4HPHP6f_chopERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charlist => rdx
*/

Value* fh_chop(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP6f_chopERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_explode(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP9f_explodeERKNS_6StringES2_i

(return value) => rax
_rv => rdi
delimiter => rsi
str => rdx
limit => rcx
*/

TypedValue* fh_explode(TypedValue* _rv, Value* delimiter, Value* str, int limit) asm("_ZN4HPHP9f_explodeERKNS_6StringES2_i");

/*
HPHP::String HPHP::f_implode(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP9f_implodeERKNS_7VariantES2_

(return value) => rax
_rv => rdi
arg1 => rsi
arg2 => rdx
*/

Value* fh_implode(Value* _rv, TypedValue* arg1, TypedValue* arg2) asm("_ZN4HPHP9f_implodeERKNS_7VariantES2_");

/*
HPHP::String HPHP::f_join(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP6f_joinERKNS_7VariantES2_

(return value) => rax
_rv => rdi
glue => rsi
pieces => rdx
*/

Value* fh_join(Value* _rv, TypedValue* glue, TypedValue* pieces) asm("_ZN4HPHP6f_joinERKNS_7VariantES2_");

/*
HPHP::Variant HPHP::f_str_split(HPHP::String const&, int)
_ZN4HPHP11f_str_splitERKNS_6StringEi

(return value) => rax
_rv => rdi
str => rsi
split_length => rdx
*/

TypedValue* fh_str_split(TypedValue* _rv, Value* str, int split_length) asm("_ZN4HPHP11f_str_splitERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_chunk_split(HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP13f_chunk_splitERKNS_6StringEiS2_

(return value) => rax
_rv => rdi
body => rsi
chunklen => rdx
end => rcx
*/

TypedValue* fh_chunk_split(TypedValue* _rv, Value* body, int chunklen, Value* end) asm("_ZN4HPHP13f_chunk_splitERKNS_6StringEiS2_");

/*
HPHP::Variant HPHP::f_strtok(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP8f_strtokERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
str => rsi
token => rdx
*/

TypedValue* fh_strtok(TypedValue* _rv, Value* str, TypedValue* token) asm("_ZN4HPHP8f_strtokERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_str_replace(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP13f_str_replaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
search => rsi
replace => rdx
subject => rcx
count => r8
*/

TypedValue* fh_str_replace(TypedValue* _rv, TypedValue* search, TypedValue* replace, TypedValue* subject, TypedValue* count) asm("_ZN4HPHP13f_str_replaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_str_ireplace(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_str_ireplaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
search => rsi
replace => rdx
subject => rcx
count => r8
*/

TypedValue* fh_str_ireplace(TypedValue* _rv, TypedValue* search, TypedValue* replace, TypedValue* subject, TypedValue* count) asm("_ZN4HPHP14f_str_ireplaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_substr_replace(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP16f_substr_replaceERKNS_7VariantES2_S2_S2_

(return value) => rax
_rv => rdi
str => rsi
replacement => rdx
start => rcx
length => r8
*/

TypedValue* fh_substr_replace(TypedValue* _rv, TypedValue* str, TypedValue* replacement, TypedValue* start, TypedValue* length) asm("_ZN4HPHP16f_substr_replaceERKNS_7VariantES2_S2_S2_");

/*
HPHP::Variant HPHP::f_substr(HPHP::String const&, int, int)
_ZN4HPHP8f_substrERKNS_6StringEii

(return value) => rax
_rv => rdi
str => rsi
start => rdx
length => rcx
*/

TypedValue* fh_substr(TypedValue* _rv, Value* str, int start, int length) asm("_ZN4HPHP8f_substrERKNS_6StringEii");

/*
HPHP::String HPHP::f_str_pad(HPHP::String const&, int, HPHP::String const&, int)
_ZN4HPHP9f_str_padERKNS_6StringEiS2_i

(return value) => rax
_rv => rdi
input => rsi
pad_length => rdx
pad_string => rcx
pad_type => r8
*/

Value* fh_str_pad(Value* _rv, Value* input, int pad_length, Value* pad_string, int pad_type) asm("_ZN4HPHP9f_str_padERKNS_6StringEiS2_i");

/*
HPHP::String HPHP::f_str_repeat(HPHP::String const&, int)
_ZN4HPHP12f_str_repeatERKNS_6StringEi

(return value) => rax
_rv => rdi
input => rsi
multiplier => rdx
*/

Value* fh_str_repeat(Value* _rv, Value* input, int multiplier) asm("_ZN4HPHP12f_str_repeatERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_wordwrap(HPHP::String const&, int, HPHP::String const&, bool)
_ZN4HPHP10f_wordwrapERKNS_6StringEiS2_b

(return value) => rax
_rv => rdi
str => rsi
width => rdx
wordbreak => rcx
cut => r8
*/

TypedValue* fh_wordwrap(TypedValue* _rv, Value* str, int width, Value* wordbreak, bool cut) asm("_ZN4HPHP10f_wordwrapERKNS_6StringEiS2_b");

/*
HPHP::String HPHP::f_html_entity_decode(HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP20f_html_entity_decodeERKNS_6StringEiS2_

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
*/

Value* fh_html_entity_decode(Value* _rv, Value* str, int quote_style, Value* charset) asm("_ZN4HPHP20f_html_entity_decodeERKNS_6StringEiS2_");

/*
HPHP::String HPHP::f_htmlentities(HPHP::String const&, int, HPHP::String const&, bool)
_ZN4HPHP14f_htmlentitiesERKNS_6StringEiS2_b

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
double_encode => r8
*/

Value* fh_htmlentities(Value* _rv, Value* str, int quote_style, Value* charset, bool double_encode) asm("_ZN4HPHP14f_htmlentitiesERKNS_6StringEiS2_b");

/*
HPHP::String HPHP::f_htmlspecialchars_decode(HPHP::String const&, int)
_ZN4HPHP25f_htmlspecialchars_decodeERKNS_6StringEi

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
*/

Value* fh_htmlspecialchars_decode(Value* _rv, Value* str, int quote_style) asm("_ZN4HPHP25f_htmlspecialchars_decodeERKNS_6StringEi");

/*
HPHP::String HPHP::f_htmlspecialchars(HPHP::String const&, int, HPHP::String const&, bool)
_ZN4HPHP18f_htmlspecialcharsERKNS_6StringEiS2_b

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
double_encode => r8
*/

Value* fh_htmlspecialchars(Value* _rv, Value* str, int quote_style, Value* charset, bool double_encode) asm("_ZN4HPHP18f_htmlspecialcharsERKNS_6StringEiS2_b");

/*
HPHP::String HPHP::f_fb_htmlspecialchars(HPHP::String const&, int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP21f_fb_htmlspecialcharsERKNS_6StringEiS2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
str => rsi
quote_style => rdx
charset => rcx
extra => r8
*/

Value* fh_fb_htmlspecialchars(Value* _rv, Value* str, int quote_style, Value* charset, Value* extra) asm("_ZN4HPHP21f_fb_htmlspecialcharsERKNS_6StringEiS2_RKNS_5ArrayE");

/*
HPHP::String HPHP::f_quoted_printable_encode(HPHP::String const&)
_ZN4HPHP25f_quoted_printable_encodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_quoted_printable_encode(Value* _rv, Value* str) asm("_ZN4HPHP25f_quoted_printable_encodeERKNS_6StringE");

/*
HPHP::String HPHP::f_quoted_printable_decode(HPHP::String const&)
_ZN4HPHP25f_quoted_printable_decodeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_quoted_printable_decode(Value* _rv, Value* str) asm("_ZN4HPHP25f_quoted_printable_decodeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_convert_uudecode(HPHP::String const&)
_ZN4HPHP18f_convert_uudecodeERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_convert_uudecode(TypedValue* _rv, Value* data) asm("_ZN4HPHP18f_convert_uudecodeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_convert_uuencode(HPHP::String const&)
_ZN4HPHP18f_convert_uuencodeERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_convert_uuencode(TypedValue* _rv, Value* data) asm("_ZN4HPHP18f_convert_uuencodeERKNS_6StringE");

/*
HPHP::String HPHP::f_str_rot13(HPHP::String const&)
_ZN4HPHP11f_str_rot13ERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

Value* fh_str_rot13(Value* _rv, Value* str) asm("_ZN4HPHP11f_str_rot13ERKNS_6StringE");

/*
long HPHP::f_crc32(HPHP::String const&)
_ZN4HPHP7f_crc32ERKNS_6StringE

(return value) => rax
str => rdi
*/

long fh_crc32(Value* str) asm("_ZN4HPHP7f_crc32ERKNS_6StringE");

/*
HPHP::String HPHP::f_crypt(HPHP::String const&, HPHP::String const&)
_ZN4HPHP7f_cryptERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
salt => rdx
*/

Value* fh_crypt(Value* _rv, Value* str, Value* salt) asm("_ZN4HPHP7f_cryptERKNS_6StringES2_");

/*
HPHP::String HPHP::f_md5(HPHP::String const&, bool)
_ZN4HPHP5f_md5ERKNS_6StringEb

(return value) => rax
_rv => rdi
str => rsi
raw_output => rdx
*/

Value* fh_md5(Value* _rv, Value* str, bool raw_output) asm("_ZN4HPHP5f_md5ERKNS_6StringEb");

/*
HPHP::String HPHP::f_sha1(HPHP::String const&, bool)
_ZN4HPHP6f_sha1ERKNS_6StringEb

(return value) => rax
_rv => rdi
str => rsi
raw_output => rdx
*/

Value* fh_sha1(Value* _rv, Value* str, bool raw_output) asm("_ZN4HPHP6f_sha1ERKNS_6StringEb");

/*
HPHP::Variant HPHP::f_strtr(HPHP::String const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP7f_strtrERKNS_6StringERKNS_7VariantES5_

(return value) => rax
_rv => rdi
str => rsi
from => rdx
to => rcx
*/

TypedValue* fh_strtr(TypedValue* _rv, Value* str, TypedValue* from, TypedValue* to) asm("_ZN4HPHP7f_strtrERKNS_6StringERKNS_7VariantES5_");

/*
HPHP::String HPHP::f_convert_cyr_string(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_convert_cyr_stringERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
str => rsi
from => rdx
to => rcx
*/

Value* fh_convert_cyr_string(Value* _rv, Value* str, Value* from, Value* to) asm("_ZN4HPHP20f_convert_cyr_stringERKNS_6StringES2_S2_");

/*
HPHP::Array HPHP::f_get_html_translation_table(int, int)
_ZN4HPHP28f_get_html_translation_tableEii

(return value) => rax
_rv => rdi
table => rsi
quote_style => rdx
*/

Value* fh_get_html_translation_table(Value* _rv, int table, int quote_style) asm("_ZN4HPHP28f_get_html_translation_tableEii");

/*
HPHP::String HPHP::f_hebrev(HPHP::String const&, int)
_ZN4HPHP8f_hebrevERKNS_6StringEi

(return value) => rax
_rv => rdi
hebrew_text => rsi
max_chars_per_line => rdx
*/

Value* fh_hebrev(Value* _rv, Value* hebrew_text, int max_chars_per_line) asm("_ZN4HPHP8f_hebrevERKNS_6StringEi");

/*
HPHP::String HPHP::f_hebrevc(HPHP::String const&, int)
_ZN4HPHP9f_hebrevcERKNS_6StringEi

(return value) => rax
_rv => rdi
hebrew_text => rsi
max_chars_per_line => rdx
*/

Value* fh_hebrevc(Value* _rv, Value* hebrew_text, int max_chars_per_line) asm("_ZN4HPHP9f_hebrevcERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_setlocale(int, int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP11f_setlocaleEiiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
category => rdx
locale => rcx
_argv => r8
*/

TypedValue* fh_setlocale(TypedValue* _rv, int64_t _argc, int category, TypedValue* locale, Value* _argv) asm("_ZN4HPHP11f_setlocaleEiiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Array HPHP::f_localeconv()
_ZN4HPHP12f_localeconvEv

(return value) => rax
_rv => rdi
*/

Value* fh_localeconv(Value* _rv) asm("_ZN4HPHP12f_localeconvEv");

/*
HPHP::String HPHP::f_nl_langinfo(int)
_ZN4HPHP13f_nl_langinfoEi

(return value) => rax
_rv => rdi
item => rsi
*/

Value* fh_nl_langinfo(Value* _rv, int item) asm("_ZN4HPHP13f_nl_langinfoEi");

/*
HPHP::Variant HPHP::f_printf(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP8f_printfEiRKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

TypedValue* fh_printf(TypedValue* _rv, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP8f_printfEiRKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_vprintf(HPHP::String const&, HPHP::Array const&)
_ZN4HPHP9f_vprintfERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
format => rsi
args => rdx
*/

TypedValue* fh_vprintf(TypedValue* _rv, Value* format, Value* args) asm("_ZN4HPHP9f_vprintfERKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_sprintf(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP9f_sprintfEiRKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

TypedValue* fh_sprintf(TypedValue* _rv, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP9f_sprintfEiRKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_vsprintf(HPHP::String const&, HPHP::Array const&)
_ZN4HPHP10f_vsprintfERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
format => rsi
args => rdx
*/

TypedValue* fh_vsprintf(TypedValue* _rv, Value* format, Value* args) asm("_ZN4HPHP10f_vsprintfERKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_sscanf(int, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP8f_sscanfEiRKNS_6StringES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
str => rdx
format => rcx
_argv => r8
*/

TypedValue* fh_sscanf(TypedValue* _rv, int64_t _argc, Value* str, Value* format, Value* _argv) asm("_ZN4HPHP8f_sscanfEiRKNS_6StringES2_RKNS_5ArrayE");

/*
HPHP::String HPHP::f_chr(long)
_ZN4HPHP5f_chrEl

(return value) => rax
_rv => rdi
ascii => rsi
*/

Value* fh_chr(Value* _rv, long ascii) asm("_ZN4HPHP5f_chrEl");

/*
long HPHP::f_ord(HPHP::String const&)
_ZN4HPHP5f_ordERKNS_6StringE

(return value) => rax
str => rdi
*/

long fh_ord(Value* str) asm("_ZN4HPHP5f_ordERKNS_6StringE");

/*
HPHP::Variant HPHP::f_money_format(HPHP::String const&, double)
_ZN4HPHP14f_money_formatERKNS_6StringEd

(return value) => rax
_rv => rdi
format => rsi
number => xmm0
*/

TypedValue* fh_money_format(TypedValue* _rv, Value* format, double number) asm("_ZN4HPHP14f_money_formatERKNS_6StringEd");

/*
HPHP::String HPHP::f_number_format(double, int, HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_number_formatEdiRKNS_6StringES2_

(return value) => rax
_rv => rdi
number => xmm0
decimals => rsi
dec_point => rdx
thousands_sep => rcx
*/

Value* fh_number_format(Value* _rv, double number, int decimals, Value* dec_point, Value* thousands_sep) asm("_ZN4HPHP15f_number_formatEdiRKNS_6StringES2_");

/*
long HPHP::f_strcmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP8f_strcmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long fh_strcmp(Value* str1, Value* str2) asm("_ZN4HPHP8f_strcmpERKNS_6StringES2_");

/*
long HPHP::f_strncmp(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP9f_strncmpERKNS_6StringES2_i

(return value) => rax
str1 => rdi
str2 => rsi
len => rdx
*/

long fh_strncmp(Value* str1, Value* str2, int len) asm("_ZN4HPHP9f_strncmpERKNS_6StringES2_i");

/*
long HPHP::f_strnatcmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11f_strnatcmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long fh_strnatcmp(Value* str1, Value* str2) asm("_ZN4HPHP11f_strnatcmpERKNS_6StringES2_");

/*
long HPHP::f_strcasecmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP12f_strcasecmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long fh_strcasecmp(Value* str1, Value* str2) asm("_ZN4HPHP12f_strcasecmpERKNS_6StringES2_");

/*
long HPHP::f_strncasecmp(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP13f_strncasecmpERKNS_6StringES2_i

(return value) => rax
str1 => rdi
str2 => rsi
len => rdx
*/

long fh_strncasecmp(Value* str1, Value* str2, int len) asm("_ZN4HPHP13f_strncasecmpERKNS_6StringES2_i");

/*
long HPHP::f_strnatcasecmp(HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_strnatcasecmpERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long fh_strnatcasecmp(Value* str1, Value* str2) asm("_ZN4HPHP15f_strnatcasecmpERKNS_6StringES2_");

/*
long HPHP::f_strcoll(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9f_strcollERKNS_6StringES2_

(return value) => rax
str1 => rdi
str2 => rsi
*/

long fh_strcoll(Value* str1, Value* str2) asm("_ZN4HPHP9f_strcollERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_substr_compare(HPHP::String const&, HPHP::String const&, int, int, bool)
_ZN4HPHP16f_substr_compareERKNS_6StringES2_iib

(return value) => rax
_rv => rdi
main_str => rsi
str => rdx
offset => rcx
length => r8
case_insensitivity => r9
*/

TypedValue* fh_substr_compare(TypedValue* _rv, Value* main_str, Value* str, int offset, int length, bool case_insensitivity) asm("_ZN4HPHP16f_substr_compareERKNS_6StringES2_iib");

/*
HPHP::Variant HPHP::f_strchr(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP8f_strchrERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
*/

TypedValue* fh_strchr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP8f_strchrERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_strrchr(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP9f_strrchrERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
*/

TypedValue* fh_strrchr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP9f_strrchrERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_strstr(HPHP::String const&, HPHP::Variant const&, bool)
_ZN4HPHP8f_strstrERKNS_6StringERKNS_7VariantEb

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
before_needle => rcx
*/

TypedValue* fh_strstr(TypedValue* _rv, Value* haystack, TypedValue* needle, bool before_needle) asm("_ZN4HPHP8f_strstrERKNS_6StringERKNS_7VariantEb");

/*
HPHP::Variant HPHP::f_stristr(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP9f_stristrERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
*/

TypedValue* fh_stristr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP9f_stristrERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_strpbrk(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9f_strpbrkERKNS_6StringES2_

(return value) => rax
_rv => rdi
haystack => rsi
char_list => rdx
*/

TypedValue* fh_strpbrk(TypedValue* _rv, Value* haystack, Value* char_list) asm("_ZN4HPHP9f_strpbrkERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_strpos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP8f_strposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_strpos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP8f_strposERKNS_6StringERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_stripos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP9f_striposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_stripos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP9f_striposERKNS_6StringERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_strrpos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP9f_strrposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_strrpos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP9f_strrposERKNS_6StringERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_strripos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP10f_strriposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_strripos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP10f_strriposERKNS_6StringERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_substr_count(HPHP::String const&, HPHP::String const&, int, int)
_ZN4HPHP14f_substr_countERKNS_6StringES2_ii

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
length => r8
*/

TypedValue* fh_substr_count(TypedValue* _rv, Value* haystack, Value* needle, int offset, int length) asm("_ZN4HPHP14f_substr_countERKNS_6StringES2_ii");

/*
HPHP::Variant HPHP::f_strspn(HPHP::String const&, HPHP::String const&, int, int)
_ZN4HPHP8f_strspnERKNS_6StringES2_ii

(return value) => rax
_rv => rdi
str1 => rsi
str2 => rdx
start => rcx
length => r8
*/

TypedValue* fh_strspn(TypedValue* _rv, Value* str1, Value* str2, int start, int length) asm("_ZN4HPHP8f_strspnERKNS_6StringES2_ii");

/*
HPHP::Variant HPHP::f_strcspn(HPHP::String const&, HPHP::String const&, int, int)
_ZN4HPHP9f_strcspnERKNS_6StringES2_ii

(return value) => rax
_rv => rdi
str1 => rsi
str2 => rdx
start => rcx
length => r8
*/

TypedValue* fh_strcspn(TypedValue* _rv, Value* str1, Value* str2, int start, int length) asm("_ZN4HPHP9f_strcspnERKNS_6StringES2_ii");

/*
HPHP::Variant HPHP::f_strlen(HPHP::Variant const&)
_ZN4HPHP8f_strlenERKNS_7VariantE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_strlen(TypedValue* _rv, TypedValue* str) asm("_ZN4HPHP8f_strlenERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_count_chars(HPHP::String const&, long)
_ZN4HPHP13f_count_charsERKNS_6StringEl

(return value) => rax
_rv => rdi
str => rsi
mode => rdx
*/

TypedValue* fh_count_chars(TypedValue* _rv, Value* str, long mode) asm("_ZN4HPHP13f_count_charsERKNS_6StringEl");

/*
HPHP::Variant HPHP::f_str_word_count(HPHP::String const&, long, HPHP::String const&)
_ZN4HPHP16f_str_word_countERKNS_6StringElS2_

(return value) => rax
_rv => rdi
str => rsi
format => rdx
charlist => rcx
*/

TypedValue* fh_str_word_count(TypedValue* _rv, Value* str, long format, Value* charlist) asm("_ZN4HPHP16f_str_word_countERKNS_6StringElS2_");

/*
long HPHP::f_levenshtein(HPHP::String const&, HPHP::String const&, int, int, int)
_ZN4HPHP13f_levenshteinERKNS_6StringES2_iii

(return value) => rax
str1 => rdi
str2 => rsi
cost_ins => rdx
cost_rep => rcx
cost_del => r8
*/

long fh_levenshtein(Value* str1, Value* str2, int cost_ins, int cost_rep, int cost_del) asm("_ZN4HPHP13f_levenshteinERKNS_6StringES2_iii");

/*
long HPHP::f_similar_text(HPHP::String const&, HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_similar_textERKNS_6StringES2_RKNS_14VRefParamValueE

(return value) => rax
first => rdi
second => rsi
percent => rdx
*/

long fh_similar_text(Value* first, Value* second, TypedValue* percent) asm("_ZN4HPHP14f_similar_textERKNS_6StringES2_RKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_soundex(HPHP::String const&)
_ZN4HPHP9f_soundexERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_soundex(TypedValue* _rv, Value* str) asm("_ZN4HPHP9f_soundexERKNS_6StringE");

/*
HPHP::Variant HPHP::f_metaphone(HPHP::String const&, int)
_ZN4HPHP11f_metaphoneERKNS_6StringEi

(return value) => rax
_rv => rdi
str => rsi
phones => rdx
*/

TypedValue* fh_metaphone(TypedValue* _rv, Value* str, int phones) asm("_ZN4HPHP11f_metaphoneERKNS_6StringEi");

/*
void HPHP::f_parse_str(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP11f_parse_strERKNS_6StringERKNS_14VRefParamValueE

str => rdi
arr => rsi
*/

void fh_parse_str(Value* str, TypedValue* arr) asm("_ZN4HPHP11f_parse_strERKNS_6StringERKNS_14VRefParamValueE");


} // !HPHP

