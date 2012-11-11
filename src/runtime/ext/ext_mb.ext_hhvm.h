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
HPHP::Array HPHP::f_mb_list_encodings()
_ZN4HPHP19f_mb_list_encodingsEv

(return value) => rax
_rv => rdi
*/

Value* fh_mb_list_encodings(Value* _rv) asm("_ZN4HPHP19f_mb_list_encodingsEv");

/*
HPHP::Variant HPHP::f_mb_list_encodings_alias_names(HPHP::String const&)
_ZN4HPHP31f_mb_list_encodings_alias_namesERKNS_6StringE

(return value) => rax
_rv => rdi
name => rsi
*/

TypedValue* fh_mb_list_encodings_alias_names(TypedValue* _rv, Value* name) asm("_ZN4HPHP31f_mb_list_encodings_alias_namesERKNS_6StringE");

/*
HPHP::Variant HPHP::f_mb_list_mime_names(HPHP::String const&)
_ZN4HPHP20f_mb_list_mime_namesERKNS_6StringE

(return value) => rax
_rv => rdi
name => rsi
*/

TypedValue* fh_mb_list_mime_names(TypedValue* _rv, Value* name) asm("_ZN4HPHP20f_mb_list_mime_namesERKNS_6StringE");

/*
bool HPHP::f_mb_check_encoding(HPHP::String const&, HPHP::String const&)
_ZN4HPHP19f_mb_check_encodingERKNS_6StringES2_

(return value) => rax
var => rdi
encoding => rsi
*/

bool fh_mb_check_encoding(Value* var, Value* encoding) asm("_ZN4HPHP19f_mb_check_encodingERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_mb_convert_case(HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP17f_mb_convert_caseERKNS_6StringEiS2_

(return value) => rax
_rv => rdi
str => rsi
mode => rdx
encoding => rcx
*/

TypedValue* fh_mb_convert_case(TypedValue* _rv, Value* str, int mode, Value* encoding) asm("_ZN4HPHP17f_mb_convert_caseERKNS_6StringEiS2_");

/*
HPHP::Variant HPHP::f_mb_convert_encoding(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP21f_mb_convert_encodingERKNS_6StringES2_RKNS_7VariantE

(return value) => rax
_rv => rdi
str => rsi
to_encoding => rdx
from_encoding => rcx
*/

TypedValue* fh_mb_convert_encoding(TypedValue* _rv, Value* str, Value* to_encoding, TypedValue* from_encoding) asm("_ZN4HPHP21f_mb_convert_encodingERKNS_6StringES2_RKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mb_convert_kana(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP17f_mb_convert_kanaERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
str => rsi
option => rdx
encoding => rcx
*/

TypedValue* fh_mb_convert_kana(TypedValue* _rv, Value* str, Value* option, Value* encoding) asm("_ZN4HPHP17f_mb_convert_kanaERKNS_6StringES2_S2_");

/*
HPHP::Variant HPHP::f_mb_convert_variables(int, HPHP::String const&, HPHP::Variant const&, HPHP::VRefParamValue const&, HPHP::Array const&)
_ZN4HPHP22f_mb_convert_variablesEiRKNS_6StringERKNS_7VariantERKNS_14VRefParamValueERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
to_encoding => rdx
from_encoding => rcx
vars => r8
_argv => r9
*/

TypedValue* fh_mb_convert_variables(TypedValue* _rv, long long _argc, Value* to_encoding, TypedValue* from_encoding, TypedValue* vars, Value* _argv) asm("_ZN4HPHP22f_mb_convert_variablesEiRKNS_6StringERKNS_7VariantERKNS_14VRefParamValueERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_mb_decode_mimeheader(HPHP::String const&)
_ZN4HPHP22f_mb_decode_mimeheaderERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_mb_decode_mimeheader(TypedValue* _rv, Value* str) asm("_ZN4HPHP22f_mb_decode_mimeheaderERKNS_6StringE");

/*
HPHP::Variant HPHP::f_mb_decode_numericentity(HPHP::String const&, HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP25f_mb_decode_numericentityERKNS_6StringERKNS_7VariantES2_

(return value) => rax
_rv => rdi
str => rsi
convmap => rdx
encoding => rcx
*/

TypedValue* fh_mb_decode_numericentity(TypedValue* _rv, Value* str, TypedValue* convmap, Value* encoding) asm("_ZN4HPHP25f_mb_decode_numericentityERKNS_6StringERKNS_7VariantES2_");

/*
HPHP::Variant HPHP::f_mb_detect_encoding(HPHP::String const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP20f_mb_detect_encodingERKNS_6StringERKNS_7VariantES5_

(return value) => rax
_rv => rdi
str => rsi
encoding_list => rdx
strict => rcx
*/

TypedValue* fh_mb_detect_encoding(TypedValue* _rv, Value* str, TypedValue* encoding_list, TypedValue* strict) asm("_ZN4HPHP20f_mb_detect_encodingERKNS_6StringERKNS_7VariantES5_");

/*
HPHP::Variant HPHP::f_mb_detect_order(HPHP::Variant const&)
_ZN4HPHP17f_mb_detect_orderERKNS_7VariantE

(return value) => rax
_rv => rdi
encoding_list => rsi
*/

TypedValue* fh_mb_detect_order(TypedValue* _rv, TypedValue* encoding_list) asm("_ZN4HPHP17f_mb_detect_orderERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mb_encode_mimeheader(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP22f_mb_encode_mimeheaderERKNS_6StringES2_S2_S2_i

(return value) => rax
_rv => rdi
str => rsi
charset => rdx
transfer_encoding => rcx
linefeed => r8
indent => r9
*/

TypedValue* fh_mb_encode_mimeheader(TypedValue* _rv, Value* str, Value* charset, Value* transfer_encoding, Value* linefeed, int indent) asm("_ZN4HPHP22f_mb_encode_mimeheaderERKNS_6StringES2_S2_S2_i");

/*
HPHP::Variant HPHP::f_mb_encode_numericentity(HPHP::String const&, HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP25f_mb_encode_numericentityERKNS_6StringERKNS_7VariantES2_

(return value) => rax
_rv => rdi
str => rsi
convmap => rdx
encoding => rcx
*/

TypedValue* fh_mb_encode_numericentity(TypedValue* _rv, Value* str, TypedValue* convmap, Value* encoding) asm("_ZN4HPHP25f_mb_encode_numericentityERKNS_6StringERKNS_7VariantES2_");

/*
bool HPHP::f_mb_ereg_match(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_mb_ereg_matchERKNS_6StringES2_S2_

(return value) => rax
pattern => rdi
str => rsi
option => rdx
*/

bool fh_mb_ereg_match(Value* pattern, Value* str, Value* option) asm("_ZN4HPHP15f_mb_ereg_matchERKNS_6StringES2_S2_");

/*
HPHP::Variant HPHP::f_mb_ereg_replace(HPHP::Variant const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP17f_mb_ereg_replaceERKNS_7VariantERKNS_6StringES5_S5_

(return value) => rax
_rv => rdi
pattern => rsi
replacement => rdx
str => rcx
option => r8
*/

TypedValue* fh_mb_ereg_replace(TypedValue* _rv, TypedValue* pattern, Value* replacement, Value* str, Value* option) asm("_ZN4HPHP17f_mb_ereg_replaceERKNS_7VariantERKNS_6StringES5_S5_");

/*
long long HPHP::f_mb_ereg_search_getpos()
_ZN4HPHP23f_mb_ereg_search_getposEv

(return value) => rax
*/

long long fh_mb_ereg_search_getpos() asm("_ZN4HPHP23f_mb_ereg_search_getposEv");

/*
HPHP::Variant HPHP::f_mb_ereg_search_getregs()
_ZN4HPHP24f_mb_ereg_search_getregsEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_mb_ereg_search_getregs(TypedValue* _rv) asm("_ZN4HPHP24f_mb_ereg_search_getregsEv");

/*
bool HPHP::f_mb_ereg_search_init(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_mb_ereg_search_initERKNS_6StringES2_S2_

(return value) => rax
str => rdi
pattern => rsi
option => rdx
*/

bool fh_mb_ereg_search_init(Value* str, Value* pattern, Value* option) asm("_ZN4HPHP21f_mb_ereg_search_initERKNS_6StringES2_S2_");

/*
HPHP::Variant HPHP::f_mb_ereg_search_pos(HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_mb_ereg_search_posERKNS_6StringES2_

(return value) => rax
_rv => rdi
pattern => rsi
option => rdx
*/

TypedValue* fh_mb_ereg_search_pos(TypedValue* _rv, Value* pattern, Value* option) asm("_ZN4HPHP20f_mb_ereg_search_posERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_mb_ereg_search_regs(HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_mb_ereg_search_regsERKNS_6StringES2_

(return value) => rax
_rv => rdi
pattern => rsi
option => rdx
*/

TypedValue* fh_mb_ereg_search_regs(TypedValue* _rv, Value* pattern, Value* option) asm("_ZN4HPHP21f_mb_ereg_search_regsERKNS_6StringES2_");

/*
bool HPHP::f_mb_ereg_search_setpos(int)
_ZN4HPHP23f_mb_ereg_search_setposEi

(return value) => rax
position => rdi
*/

bool fh_mb_ereg_search_setpos(int position) asm("_ZN4HPHP23f_mb_ereg_search_setposEi");

/*
HPHP::Variant HPHP::f_mb_ereg_search(HPHP::String const&, HPHP::String const&)
_ZN4HPHP16f_mb_ereg_searchERKNS_6StringES2_

(return value) => rax
_rv => rdi
pattern => rsi
option => rdx
*/

TypedValue* fh_mb_ereg_search(TypedValue* _rv, Value* pattern, Value* option) asm("_ZN4HPHP16f_mb_ereg_searchERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_mb_ereg(HPHP::Variant const&, HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP9f_mb_eregERKNS_7VariantERKNS_6StringERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
pattern => rsi
str => rdx
regs => rcx
*/

TypedValue* fh_mb_ereg(TypedValue* _rv, TypedValue* pattern, Value* str, TypedValue* regs) asm("_ZN4HPHP9f_mb_eregERKNS_7VariantERKNS_6StringERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_mb_eregi_replace(HPHP::Variant const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP18f_mb_eregi_replaceERKNS_7VariantERKNS_6StringES5_S5_

(return value) => rax
_rv => rdi
pattern => rsi
replacement => rdx
str => rcx
option => r8
*/

TypedValue* fh_mb_eregi_replace(TypedValue* _rv, TypedValue* pattern, Value* replacement, Value* str, Value* option) asm("_ZN4HPHP18f_mb_eregi_replaceERKNS_7VariantERKNS_6StringES5_S5_");

/*
HPHP::Variant HPHP::f_mb_eregi(HPHP::Variant const&, HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP10f_mb_eregiERKNS_7VariantERKNS_6StringERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
pattern => rsi
str => rdx
regs => rcx
*/

TypedValue* fh_mb_eregi(TypedValue* _rv, TypedValue* pattern, Value* str, TypedValue* regs) asm("_ZN4HPHP10f_mb_eregiERKNS_7VariantERKNS_6StringERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_mb_get_info(HPHP::String const&)
_ZN4HPHP13f_mb_get_infoERKNS_6StringE

(return value) => rax
_rv => rdi
type => rsi
*/

TypedValue* fh_mb_get_info(TypedValue* _rv, Value* type) asm("_ZN4HPHP13f_mb_get_infoERKNS_6StringE");

/*
HPHP::Variant HPHP::f_mb_http_input(HPHP::String const&)
_ZN4HPHP15f_mb_http_inputERKNS_6StringE

(return value) => rax
_rv => rdi
type => rsi
*/

TypedValue* fh_mb_http_input(TypedValue* _rv, Value* type) asm("_ZN4HPHP15f_mb_http_inputERKNS_6StringE");

/*
HPHP::Variant HPHP::f_mb_http_output(HPHP::String const&)
_ZN4HPHP16f_mb_http_outputERKNS_6StringE

(return value) => rax
_rv => rdi
encoding => rsi
*/

TypedValue* fh_mb_http_output(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP16f_mb_http_outputERKNS_6StringE");

/*
HPHP::Variant HPHP::f_mb_internal_encoding(HPHP::String const&)
_ZN4HPHP22f_mb_internal_encodingERKNS_6StringE

(return value) => rax
_rv => rdi
encoding => rsi
*/

TypedValue* fh_mb_internal_encoding(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP22f_mb_internal_encodingERKNS_6StringE");

/*
HPHP::Variant HPHP::f_mb_language(HPHP::String const&)
_ZN4HPHP13f_mb_languageERKNS_6StringE

(return value) => rax
_rv => rdi
language => rsi
*/

TypedValue* fh_mb_language(TypedValue* _rv, Value* language) asm("_ZN4HPHP13f_mb_languageERKNS_6StringE");

/*
HPHP::String HPHP::f_mb_output_handler(HPHP::String const&, int)
_ZN4HPHP19f_mb_output_handlerERKNS_6StringEi

(return value) => rax
_rv => rdi
contents => rsi
status => rdx
*/

Value* fh_mb_output_handler(Value* _rv, Value* contents, int status) asm("_ZN4HPHP19f_mb_output_handlerERKNS_6StringEi");

/*
bool HPHP::f_mb_parse_str(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_mb_parse_strERKNS_6StringERKNS_14VRefParamValueE

(return value) => rax
encoded_string => rdi
result => rsi
*/

bool fh_mb_parse_str(Value* encoded_string, TypedValue* result) asm("_ZN4HPHP14f_mb_parse_strERKNS_6StringERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_mb_preferred_mime_name(HPHP::String const&)
_ZN4HPHP24f_mb_preferred_mime_nameERKNS_6StringE

(return value) => rax
_rv => rdi
encoding => rsi
*/

TypedValue* fh_mb_preferred_mime_name(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP24f_mb_preferred_mime_nameERKNS_6StringE");

/*
HPHP::Variant HPHP::f_mb_regex_encoding(HPHP::String const&)
_ZN4HPHP19f_mb_regex_encodingERKNS_6StringE

(return value) => rax
_rv => rdi
encoding => rsi
*/

TypedValue* fh_mb_regex_encoding(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP19f_mb_regex_encodingERKNS_6StringE");

/*
HPHP::String HPHP::f_mb_regex_set_options(HPHP::String const&)
_ZN4HPHP22f_mb_regex_set_optionsERKNS_6StringE

(return value) => rax
_rv => rdi
options => rsi
*/

Value* fh_mb_regex_set_options(Value* _rv, Value* options) asm("_ZN4HPHP22f_mb_regex_set_optionsERKNS_6StringE");

/*
bool HPHP::f_mb_send_mail(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP14f_mb_send_mailERKNS_6StringES2_S2_S2_S2_

(return value) => rax
to => rdi
subject => rsi
message => rdx
headers => rcx
extra_cmd => r8
*/

bool fh_mb_send_mail(Value* to, Value* subject, Value* message, Value* headers, Value* extra_cmd) asm("_ZN4HPHP14f_mb_send_mailERKNS_6StringES2_S2_S2_S2_");

/*
HPHP::Variant HPHP::f_mb_split(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP10f_mb_splitERKNS_6StringES2_i

(return value) => rax
_rv => rdi
pattern => rsi
str => rdx
count => rcx
*/

TypedValue* fh_mb_split(TypedValue* _rv, Value* pattern, Value* str, int count) asm("_ZN4HPHP10f_mb_splitERKNS_6StringES2_i");

/*
HPHP::Variant HPHP::f_mb_strcut(HPHP::String const&, int, int, HPHP::String const&)
_ZN4HPHP11f_mb_strcutERKNS_6StringEiiS2_

(return value) => rax
_rv => rdi
str => rsi
start => rdx
length => rcx
encoding => r8
*/

TypedValue* fh_mb_strcut(TypedValue* _rv, Value* str, int start, int length, Value* encoding) asm("_ZN4HPHP11f_mb_strcutERKNS_6StringEiiS2_");

/*
HPHP::Variant HPHP::f_mb_strimwidth(HPHP::String const&, int, int, HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_mb_strimwidthERKNS_6StringEiiS2_S2_

(return value) => rax
_rv => rdi
str => rsi
start => rdx
width => rcx
trimmarker => r8
encoding => r9
*/

TypedValue* fh_mb_strimwidth(TypedValue* _rv, Value* str, int start, int width, Value* trimmarker, Value* encoding) asm("_ZN4HPHP15f_mb_strimwidthERKNS_6StringEiiS2_S2_");

/*
HPHP::Variant HPHP::f_mb_stripos(HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP12f_mb_striposERKNS_6StringES2_iS2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
encoding => r8
*/

TypedValue* fh_mb_stripos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* encoding) asm("_ZN4HPHP12f_mb_striposERKNS_6StringES2_iS2_");

/*
HPHP::Variant HPHP::f_mb_stristr(HPHP::String const&, HPHP::String const&, bool, HPHP::String const&)
_ZN4HPHP12f_mb_stristrERKNS_6StringES2_bS2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
part => rcx
encoding => r8
*/

TypedValue* fh_mb_stristr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP12f_mb_stristrERKNS_6StringES2_bS2_");

/*
HPHP::Variant HPHP::f_mb_strlen(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11f_mb_strlenERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
encoding => rdx
*/

TypedValue* fh_mb_strlen(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP11f_mb_strlenERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_mb_strpos(HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP11f_mb_strposERKNS_6StringES2_iS2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
encoding => r8
*/

TypedValue* fh_mb_strpos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* encoding) asm("_ZN4HPHP11f_mb_strposERKNS_6StringES2_iS2_");

/*
HPHP::Variant HPHP::f_mb_strrchr(HPHP::String const&, HPHP::String const&, bool, HPHP::String const&)
_ZN4HPHP12f_mb_strrchrERKNS_6StringES2_bS2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
part => rcx
encoding => r8
*/

TypedValue* fh_mb_strrchr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP12f_mb_strrchrERKNS_6StringES2_bS2_");

/*
HPHP::Variant HPHP::f_mb_strrichr(HPHP::String const&, HPHP::String const&, bool, HPHP::String const&)
_ZN4HPHP13f_mb_strrichrERKNS_6StringES2_bS2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
part => rcx
encoding => r8
*/

TypedValue* fh_mb_strrichr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP13f_mb_strrichrERKNS_6StringES2_bS2_");

/*
HPHP::Variant HPHP::f_mb_strripos(HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP13f_mb_strriposERKNS_6StringES2_iS2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
encoding => r8
*/

TypedValue* fh_mb_strripos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* encoding) asm("_ZN4HPHP13f_mb_strriposERKNS_6StringES2_iS2_");

/*
HPHP::Variant HPHP::f_mb_strrpos(HPHP::String const&, HPHP::String const&, HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP12f_mb_strrposERKNS_6StringES2_RKNS_7VariantES2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
encoding => r8
*/

TypedValue* fh_mb_strrpos(TypedValue* _rv, Value* haystack, Value* needle, TypedValue* offset, Value* encoding) asm("_ZN4HPHP12f_mb_strrposERKNS_6StringES2_RKNS_7VariantES2_");

/*
HPHP::Variant HPHP::f_mb_strstr(HPHP::String const&, HPHP::String const&, bool, HPHP::String const&)
_ZN4HPHP11f_mb_strstrERKNS_6StringES2_bS2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
part => rcx
encoding => r8
*/

TypedValue* fh_mb_strstr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP11f_mb_strstrERKNS_6StringES2_bS2_");

/*
HPHP::Variant HPHP::f_mb_strtolower(HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_mb_strtolowerERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
encoding => rdx
*/

TypedValue* fh_mb_strtolower(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP15f_mb_strtolowerERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_mb_strtoupper(HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_mb_strtoupperERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
encoding => rdx
*/

TypedValue* fh_mb_strtoupper(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP15f_mb_strtoupperERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_mb_strwidth(HPHP::String const&, HPHP::String const&)
_ZN4HPHP13f_mb_strwidthERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
encoding => rdx
*/

TypedValue* fh_mb_strwidth(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP13f_mb_strwidthERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_mb_substitute_character(HPHP::Variant const&)
_ZN4HPHP25f_mb_substitute_characterERKNS_7VariantE

(return value) => rax
_rv => rdi
substrchar => rsi
*/

TypedValue* fh_mb_substitute_character(TypedValue* _rv, TypedValue* substrchar) asm("_ZN4HPHP25f_mb_substitute_characterERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mb_substr_count(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP17f_mb_substr_countERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
encoding => rcx
*/

TypedValue* fh_mb_substr_count(TypedValue* _rv, Value* haystack, Value* needle, Value* encoding) asm("_ZN4HPHP17f_mb_substr_countERKNS_6StringES2_S2_");

/*
HPHP::Variant HPHP::f_mb_substr(HPHP::String const&, int, int, HPHP::String const&)
_ZN4HPHP11f_mb_substrERKNS_6StringEiiS2_

(return value) => rax
_rv => rdi
str => rsi
start => rdx
length => rcx
encoding => r8
*/

TypedValue* fh_mb_substr(TypedValue* _rv, Value* str, int start, int length, Value* encoding) asm("_ZN4HPHP11f_mb_substrERKNS_6StringEiiS2_");


} // !HPHP

