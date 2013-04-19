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

Value* fh_mb_list_encodings(Value* _rv) asm("_ZN4HPHP19f_mb_list_encodingsEv");

TypedValue* fh_mb_list_encodings_alias_names(TypedValue* _rv, Value* name) asm("_ZN4HPHP31f_mb_list_encodings_alias_namesERKNS_6StringE");

TypedValue* fh_mb_list_mime_names(TypedValue* _rv, Value* name) asm("_ZN4HPHP20f_mb_list_mime_namesERKNS_6StringE");

bool fh_mb_check_encoding(Value* var, Value* encoding) asm("_ZN4HPHP19f_mb_check_encodingERKNS_6StringES2_");

TypedValue* fh_mb_convert_case(TypedValue* _rv, Value* str, int mode, Value* encoding) asm("_ZN4HPHP17f_mb_convert_caseERKNS_6StringEiS2_");

TypedValue* fh_mb_convert_encoding(TypedValue* _rv, Value* str, Value* to_encoding, TypedValue* from_encoding) asm("_ZN4HPHP21f_mb_convert_encodingERKNS_6StringES2_RKNS_7VariantE");

TypedValue* fh_mb_convert_kana(TypedValue* _rv, Value* str, Value* option, Value* encoding) asm("_ZN4HPHP17f_mb_convert_kanaERKNS_6StringES2_S2_");

TypedValue* fh_mb_convert_variables(TypedValue* _rv, int64_t _argc, Value* to_encoding, TypedValue* from_encoding, TypedValue* vars, Value* _argv) asm("_ZN4HPHP22f_mb_convert_variablesEiRKNS_6StringERKNS_7VariantERKNS_14VRefParamValueERKNS_5ArrayE");

TypedValue* fh_mb_decode_mimeheader(TypedValue* _rv, Value* str) asm("_ZN4HPHP22f_mb_decode_mimeheaderERKNS_6StringE");

TypedValue* fh_mb_decode_numericentity(TypedValue* _rv, Value* str, TypedValue* convmap, Value* encoding) asm("_ZN4HPHP25f_mb_decode_numericentityERKNS_6StringERKNS_7VariantES2_");

TypedValue* fh_mb_detect_encoding(TypedValue* _rv, Value* str, TypedValue* encoding_list, TypedValue* strict) asm("_ZN4HPHP20f_mb_detect_encodingERKNS_6StringERKNS_7VariantES5_");

TypedValue* fh_mb_detect_order(TypedValue* _rv, TypedValue* encoding_list) asm("_ZN4HPHP17f_mb_detect_orderERKNS_7VariantE");

TypedValue* fh_mb_encode_mimeheader(TypedValue* _rv, Value* str, Value* charset, Value* transfer_encoding, Value* linefeed, int indent) asm("_ZN4HPHP22f_mb_encode_mimeheaderERKNS_6StringES2_S2_S2_i");

TypedValue* fh_mb_encode_numericentity(TypedValue* _rv, Value* str, TypedValue* convmap, Value* encoding) asm("_ZN4HPHP25f_mb_encode_numericentityERKNS_6StringERKNS_7VariantES2_");

TypedValue* fh_mb_get_info(TypedValue* _rv, Value* type) asm("_ZN4HPHP13f_mb_get_infoERKNS_6StringE");

TypedValue* fh_mb_http_input(TypedValue* _rv, Value* type) asm("_ZN4HPHP15f_mb_http_inputERKNS_6StringE");

TypedValue* fh_mb_http_output(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP16f_mb_http_outputERKNS_6StringE");

TypedValue* fh_mb_internal_encoding(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP22f_mb_internal_encodingERKNS_6StringE");

TypedValue* fh_mb_language(TypedValue* _rv, Value* language) asm("_ZN4HPHP13f_mb_languageERKNS_6StringE");

Value* fh_mb_output_handler(Value* _rv, Value* contents, int status) asm("_ZN4HPHP19f_mb_output_handlerERKNS_6StringEi");

bool fh_mb_parse_str(Value* encoded_string, TypedValue* result) asm("_ZN4HPHP14f_mb_parse_strERKNS_6StringERKNS_14VRefParamValueE");

TypedValue* fh_mb_preferred_mime_name(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP24f_mb_preferred_mime_nameERKNS_6StringE");

TypedValue* fh_mb_substr(TypedValue* _rv, Value* str, int start, int length, Value* encoding) asm("_ZN4HPHP11f_mb_substrERKNS_6StringEiiS2_");

TypedValue* fh_mb_strcut(TypedValue* _rv, Value* str, int start, int length, Value* encoding) asm("_ZN4HPHP11f_mb_strcutERKNS_6StringEiiS2_");

TypedValue* fh_mb_strimwidth(TypedValue* _rv, Value* str, int start, int width, Value* trimmarker, Value* encoding) asm("_ZN4HPHP15f_mb_strimwidthERKNS_6StringEiiS2_S2_");

TypedValue* fh_mb_stripos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* encoding) asm("_ZN4HPHP12f_mb_striposERKNS_6StringES2_iS2_");

TypedValue* fh_mb_strripos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* encoding) asm("_ZN4HPHP13f_mb_strriposERKNS_6StringES2_iS2_");

TypedValue* fh_mb_stristr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP12f_mb_stristrERKNS_6StringES2_bS2_");

TypedValue* fh_mb_strlen(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP11f_mb_strlenERKNS_6StringES2_");

TypedValue* fh_mb_strpos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* encoding) asm("_ZN4HPHP11f_mb_strposERKNS_6StringES2_iS2_");

TypedValue* fh_mb_strrpos(TypedValue* _rv, Value* haystack, Value* needle, TypedValue* offset, Value* encoding) asm("_ZN4HPHP12f_mb_strrposERKNS_6StringES2_RKNS_7VariantES2_");

TypedValue* fh_mb_strrchr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP12f_mb_strrchrERKNS_6StringES2_bS2_");

TypedValue* fh_mb_strrichr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP13f_mb_strrichrERKNS_6StringES2_bS2_");

TypedValue* fh_mb_strstr(TypedValue* _rv, Value* haystack, Value* needle, bool part, Value* encoding) asm("_ZN4HPHP11f_mb_strstrERKNS_6StringES2_bS2_");

TypedValue* fh_mb_strtolower(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP15f_mb_strtolowerERKNS_6StringES2_");

TypedValue* fh_mb_strtoupper(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP15f_mb_strtoupperERKNS_6StringES2_");

TypedValue* fh_mb_strwidth(TypedValue* _rv, Value* str, Value* encoding) asm("_ZN4HPHP13f_mb_strwidthERKNS_6StringES2_");

TypedValue* fh_mb_substitute_character(TypedValue* _rv, TypedValue* substrchar) asm("_ZN4HPHP25f_mb_substitute_characterERKNS_7VariantE");

TypedValue* fh_mb_substr_count(TypedValue* _rv, Value* haystack, Value* needle, Value* encoding) asm("_ZN4HPHP17f_mb_substr_countERKNS_6StringES2_S2_");

bool fh_mb_ereg_match(Value* pattern, Value* str, Value* option) asm("_ZN4HPHP15f_mb_ereg_matchERKNS_6StringES2_S2_");

TypedValue* fh_mb_ereg_replace(TypedValue* _rv, TypedValue* pattern, Value* replacement, Value* str, Value* option) asm("_ZN4HPHP17f_mb_ereg_replaceERKNS_7VariantERKNS_6StringES5_S5_");

TypedValue* fh_mb_eregi_replace(TypedValue* _rv, TypedValue* pattern, Value* replacement, Value* str, Value* option) asm("_ZN4HPHP18f_mb_eregi_replaceERKNS_7VariantERKNS_6StringES5_S5_");

long fh_mb_ereg_search_getpos() asm("_ZN4HPHP23f_mb_ereg_search_getposEv");

bool fh_mb_ereg_search_setpos(int position) asm("_ZN4HPHP23f_mb_ereg_search_setposEi");

TypedValue* fh_mb_ereg_search_getregs(TypedValue* _rv) asm("_ZN4HPHP24f_mb_ereg_search_getregsEv");

bool fh_mb_ereg_search_init(Value* str, Value* pattern, Value* option) asm("_ZN4HPHP21f_mb_ereg_search_initERKNS_6StringES2_S2_");

TypedValue* fh_mb_ereg_search(TypedValue* _rv, Value* pattern, Value* option) asm("_ZN4HPHP16f_mb_ereg_searchERKNS_6StringES2_");

TypedValue* fh_mb_ereg_search_pos(TypedValue* _rv, Value* pattern, Value* option) asm("_ZN4HPHP20f_mb_ereg_search_posERKNS_6StringES2_");

TypedValue* fh_mb_ereg_search_regs(TypedValue* _rv, Value* pattern, Value* option) asm("_ZN4HPHP21f_mb_ereg_search_regsERKNS_6StringES2_");

TypedValue* fh_mb_ereg(TypedValue* _rv, TypedValue* pattern, Value* str, TypedValue* regs) asm("_ZN4HPHP9f_mb_eregERKNS_7VariantERKNS_6StringERKNS_14VRefParamValueE");

TypedValue* fh_mb_eregi(TypedValue* _rv, TypedValue* pattern, Value* str, TypedValue* regs) asm("_ZN4HPHP10f_mb_eregiERKNS_7VariantERKNS_6StringERKNS_14VRefParamValueE");

TypedValue* fh_mb_regex_encoding(TypedValue* _rv, Value* encoding) asm("_ZN4HPHP19f_mb_regex_encodingERKNS_6StringE");

Value* fh_mb_regex_set_options(Value* _rv, Value* options) asm("_ZN4HPHP22f_mb_regex_set_optionsERKNS_6StringE");

TypedValue* fh_mb_split(TypedValue* _rv, Value* pattern, Value* str, int count) asm("_ZN4HPHP10f_mb_splitERKNS_6StringES2_i");

bool fh_mb_send_mail(Value* to, Value* subject, Value* message, Value* headers, Value* extra_cmd) asm("_ZN4HPHP14f_mb_send_mailERKNS_6StringES2_S2_S2_S2_");

} // namespace HPHP
