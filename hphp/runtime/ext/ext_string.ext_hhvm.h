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

Value* fh_addcslashes(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP13f_addcslashesERKNS_6StringES2_");

Value* fh_stripcslashes(Value* _rv, Value* str) asm("_ZN4HPHP15f_stripcslashesERKNS_6StringE");

Value* fh_addslashes(Value* _rv, Value* str) asm("_ZN4HPHP12f_addslashesERKNS_6StringE");

Value* fh_stripslashes(Value* _rv, Value* str) asm("_ZN4HPHP14f_stripslashesERKNS_6StringE");

Value* fh_bin2hex(Value* _rv, Value* str) asm("_ZN4HPHP9f_bin2hexERKNS_6StringE");

TypedValue* fh_hex2bin(TypedValue* _rv, Value* str) asm("_ZN4HPHP9f_hex2binERKNS_6StringE");

Value* fh_nl2br(Value* _rv, Value* str) asm("_ZN4HPHP7f_nl2brERKNS_6StringE");

Value* fh_quotemeta(Value* _rv, Value* str) asm("_ZN4HPHP11f_quotemetaERKNS_6StringE");

Value* fh_str_shuffle(Value* _rv, Value* str) asm("_ZN4HPHP13f_str_shuffleERKNS_6StringE");

Value* fh_strrev(Value* _rv, Value* str) asm("_ZN4HPHP8f_strrevERKNS_6StringE");

Value* fh_strtolower(Value* _rv, Value* str) asm("_ZN4HPHP12f_strtolowerERKNS_6StringE");

Value* fh_strtoupper(Value* _rv, Value* str) asm("_ZN4HPHP12f_strtoupperERKNS_6StringE");

Value* fh_ucfirst(Value* _rv, Value* str) asm("_ZN4HPHP9f_ucfirstERKNS_6StringE");

Value* fh_lcfirst(Value* _rv, Value* str) asm("_ZN4HPHP9f_lcfirstERKNS_6StringE");

Value* fh_ucwords(Value* _rv, Value* str) asm("_ZN4HPHP9f_ucwordsERKNS_6StringE");

Value* fh_strip_tags(Value* _rv, Value* str, Value* allowable_tags) asm("_ZN4HPHP12f_strip_tagsERKNS_6StringES2_");

Value* fh_trim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP6f_trimERKNS_6StringES2_");

Value* fh_ltrim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP7f_ltrimERKNS_6StringES2_");

Value* fh_rtrim(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP7f_rtrimERKNS_6StringES2_");

Value* fh_chop(Value* _rv, Value* str, Value* charlist) asm("_ZN4HPHP6f_chopERKNS_6StringES2_");

TypedValue* fh_explode(TypedValue* _rv, Value* delimiter, Value* str, int limit) asm("_ZN4HPHP9f_explodeERKNS_6StringES2_i");

Value* fh_implode(Value* _rv, TypedValue* arg1, TypedValue* arg2) asm("_ZN4HPHP9f_implodeERKNS_7VariantES2_");

Value* fh_join(Value* _rv, TypedValue* glue, TypedValue* pieces) asm("_ZN4HPHP6f_joinERKNS_7VariantES2_");

TypedValue* fh_str_split(TypedValue* _rv, Value* str, int split_length) asm("_ZN4HPHP11f_str_splitERKNS_6StringEi");

TypedValue* fh_chunk_split(TypedValue* _rv, Value* body, int chunklen, Value* end) asm("_ZN4HPHP13f_chunk_splitERKNS_6StringEiS2_");

TypedValue* fh_strtok(TypedValue* _rv, Value* str, TypedValue* token) asm("_ZN4HPHP8f_strtokERKNS_6StringERKNS_7VariantE");

TypedValue* fh_str_replace(TypedValue* _rv, TypedValue* search, TypedValue* replace, TypedValue* subject, TypedValue* count) asm("_ZN4HPHP13f_str_replaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE");

TypedValue* fh_str_ireplace(TypedValue* _rv, TypedValue* search, TypedValue* replace, TypedValue* subject, TypedValue* count) asm("_ZN4HPHP14f_str_ireplaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE");

TypedValue* fh_substr_replace(TypedValue* _rv, TypedValue* str, TypedValue* replacement, TypedValue* start, TypedValue* length) asm("_ZN4HPHP16f_substr_replaceERKNS_7VariantES2_S2_S2_");

TypedValue* fh_substr(TypedValue* _rv, Value* str, int start, int length) asm("_ZN4HPHP8f_substrERKNS_6StringEii");

Value* fh_str_pad(Value* _rv, Value* input, int pad_length, Value* pad_string, int pad_type) asm("_ZN4HPHP9f_str_padERKNS_6StringEiS2_i");

Value* fh_str_repeat(Value* _rv, Value* input, int multiplier) asm("_ZN4HPHP12f_str_repeatERKNS_6StringEi");

TypedValue* fh_wordwrap(TypedValue* _rv, Value* str, int width, Value* wordbreak, bool cut) asm("_ZN4HPHP10f_wordwrapERKNS_6StringEiS2_b");

TypedValue* fh_printf(TypedValue* _rv, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP8f_printfEiRKNS_6StringERKNS_5ArrayE");

TypedValue* fh_vprintf(TypedValue* _rv, Value* format, Value* args) asm("_ZN4HPHP9f_vprintfERKNS_6StringERKNS_5ArrayE");

TypedValue* fh_sprintf(TypedValue* _rv, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP9f_sprintfEiRKNS_6StringERKNS_5ArrayE");

TypedValue* fh_vsprintf(TypedValue* _rv, Value* format, Value* args) asm("_ZN4HPHP10f_vsprintfERKNS_6StringERKNS_5ArrayE");

TypedValue* fh_sscanf(TypedValue* _rv, int64_t _argc, Value* str, Value* format, Value* _argv) asm("_ZN4HPHP8f_sscanfEiRKNS_6StringES2_RKNS_5ArrayE");

Value* fh_chr(Value* _rv, long ascii) asm("_ZN4HPHP5f_chrEl");

long fh_ord(Value* str) asm("_ZN4HPHP5f_ordERKNS_6StringE");

TypedValue* fh_money_format(TypedValue* _rv, Value* format, double number) asm("_ZN4HPHP14f_money_formatERKNS_6StringEd");

Value* fh_number_format(Value* _rv, double number, int decimals, Value* dec_point, Value* thousands_sep) asm("_ZN4HPHP15f_number_formatEdiRKNS_6StringES2_");

long fh_strcmp(Value* str1, Value* str2) asm("_ZN4HPHP8f_strcmpERKNS_6StringES2_");

TypedValue* fh_strncmp(TypedValue* _rv, Value* str1, Value* str2, int len) asm("_ZN4HPHP9f_strncmpERKNS_6StringES2_i");

long fh_strnatcmp(Value* str1, Value* str2) asm("_ZN4HPHP11f_strnatcmpERKNS_6StringES2_");

long fh_strcasecmp(Value* str1, Value* str2) asm("_ZN4HPHP12f_strcasecmpERKNS_6StringES2_");

TypedValue* fh_strncasecmp(TypedValue* _rv, Value* str1, Value* str2, int len) asm("_ZN4HPHP13f_strncasecmpERKNS_6StringES2_i");

long fh_strnatcasecmp(Value* str1, Value* str2) asm("_ZN4HPHP15f_strnatcasecmpERKNS_6StringES2_");

long fh_strcoll(Value* str1, Value* str2) asm("_ZN4HPHP9f_strcollERKNS_6StringES2_");

TypedValue* fh_substr_compare(TypedValue* _rv, Value* main_str, Value* str, int offset, int length, bool case_insensitivity) asm("_ZN4HPHP16f_substr_compareERKNS_6StringES2_iib");

TypedValue* fh_strrchr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP9f_strrchrERKNS_6StringERKNS_7VariantE");

TypedValue* fh_strrpos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP9f_strrposERKNS_6StringERKNS_7VariantEi");

TypedValue* fh_strstr(TypedValue* _rv, Value* haystack, TypedValue* needle, bool before_needle) asm("_ZN4HPHP8f_strstrERKNS_6StringERKNS_7VariantEb");

TypedValue* fh_strpos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP8f_strposERKNS_6StringERKNS_7VariantEi");

TypedValue* fh_stristr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP9f_stristrERKNS_6StringERKNS_7VariantE");

TypedValue* fh_stripos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP9f_striposERKNS_6StringERKNS_7VariantEi");

TypedValue* fh_strpbrk(TypedValue* _rv, Value* haystack, Value* char_list) asm("_ZN4HPHP9f_strpbrkERKNS_6StringES2_");

TypedValue* fh_strchr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP8f_strchrERKNS_6StringERKNS_7VariantE");

TypedValue* fh_strripos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP10f_strriposERKNS_6StringERKNS_7VariantEi");

TypedValue* fh_substr_count(TypedValue* _rv, Value* haystack, Value* needle, int offset, int length) asm("_ZN4HPHP14f_substr_countERKNS_6StringES2_ii");

TypedValue* fh_strspn(TypedValue* _rv, Value* str1, Value* str2, int start, int length) asm("_ZN4HPHP8f_strspnERKNS_6StringES2_ii");

TypedValue* fh_strcspn(TypedValue* _rv, Value* str1, Value* str2, int start, int length) asm("_ZN4HPHP9f_strcspnERKNS_6StringES2_ii");

TypedValue* fh_strlen(TypedValue* _rv, TypedValue* vstr) asm("_ZN4HPHP8f_strlenERKNS_7VariantE");

TypedValue* fh_count_chars(TypedValue* _rv, Value* str, long mode) asm("_ZN4HPHP13f_count_charsERKNS_6StringEl");

TypedValue* fh_str_word_count(TypedValue* _rv, Value* str, long format, Value* charlist) asm("_ZN4HPHP16f_str_word_countERKNS_6StringElS2_");

long fh_levenshtein(Value* str1, Value* str2, int cost_ins, int cost_rep, int cost_del) asm("_ZN4HPHP13f_levenshteinERKNS_6StringES2_iii");

long fh_similar_text(Value* first, Value* second, TypedValue* percent) asm("_ZN4HPHP14f_similar_textERKNS_6StringES2_RKNS_14VRefParamValueE");

TypedValue* fh_soundex(TypedValue* _rv, Value* str) asm("_ZN4HPHP9f_soundexERKNS_6StringE");

TypedValue* fh_metaphone(TypedValue* _rv, Value* str, int phones) asm("_ZN4HPHP11f_metaphoneERKNS_6StringEi");

Value* fh_html_entity_decode(Value* _rv, Value* str, int quote_style, Value* charset) asm("_ZN4HPHP20f_html_entity_decodeERKNS_6StringEiS2_");

Value* fh_htmlentities(Value* _rv, Value* str, int quote_style, Value* charset, bool double_encode) asm("_ZN4HPHP14f_htmlentitiesERKNS_6StringEiS2_b");

Value* fh_htmlspecialchars_decode(Value* _rv, Value* str, int quote_style) asm("_ZN4HPHP25f_htmlspecialchars_decodeERKNS_6StringEi");

Value* fh_htmlspecialchars(Value* _rv, Value* str, int quote_style, Value* charset, bool double_encode) asm("_ZN4HPHP18f_htmlspecialcharsERKNS_6StringEiS2_b");

Value* fh_fb_htmlspecialchars(Value* _rv, Value* str, int quote_style, Value* charset, Value* extra) asm("_ZN4HPHP21f_fb_htmlspecialcharsERKNS_6StringEiS2_RKNS_5ArrayE");

Value* fh_quoted_printable_encode(Value* _rv, Value* str) asm("_ZN4HPHP25f_quoted_printable_encodeERKNS_6StringE");

Value* fh_quoted_printable_decode(Value* _rv, Value* str) asm("_ZN4HPHP25f_quoted_printable_decodeERKNS_6StringE");

TypedValue* fh_convert_uudecode(TypedValue* _rv, Value* data) asm("_ZN4HPHP18f_convert_uudecodeERKNS_6StringE");

TypedValue* fh_convert_uuencode(TypedValue* _rv, Value* data) asm("_ZN4HPHP18f_convert_uuencodeERKNS_6StringE");

Value* fh_str_rot13(Value* _rv, Value* str) asm("_ZN4HPHP11f_str_rot13ERKNS_6StringE");

long fh_crc32(Value* str) asm("_ZN4HPHP7f_crc32ERKNS_6StringE");

Value* fh_crypt(Value* _rv, Value* str, Value* salt) asm("_ZN4HPHP7f_cryptERKNS_6StringES2_");

Value* fh_md5(Value* _rv, Value* str, bool raw_output) asm("_ZN4HPHP5f_md5ERKNS_6StringEb");

Value* fh_sha1(Value* _rv, Value* str, bool raw_output) asm("_ZN4HPHP6f_sha1ERKNS_6StringEb");

TypedValue* fh_strtr(TypedValue* _rv, Value* str, TypedValue* from, TypedValue* to) asm("_ZN4HPHP7f_strtrERKNS_6StringERKNS_7VariantES5_");

void fh_parse_str(Value* str, TypedValue* arr) asm("_ZN4HPHP11f_parse_strERKNS_6StringERKNS_14VRefParamValueE");

TypedValue* fh_setlocale(TypedValue* _rv, int64_t _argc, int category, TypedValue* locale, Value* _argv) asm("_ZN4HPHP11f_setlocaleEiiRKNS_7VariantERKNS_5ArrayE");

Value* fh_localeconv(Value* _rv) asm("_ZN4HPHP12f_localeconvEv");

Value* fh_nl_langinfo(Value* _rv, int item) asm("_ZN4HPHP13f_nl_langinfoEi");

Value* fh_convert_cyr_string(Value* _rv, Value* str, Value* from, Value* to) asm("_ZN4HPHP20f_convert_cyr_stringERKNS_6StringES2_S2_");

Value* fh_get_html_translation_table(Value* _rv, int table, int quote_style) asm("_ZN4HPHP28f_get_html_translation_tableEii");

Value* fh_hebrev(Value* _rv, Value* hebrew_text, int max_chars_per_line) asm("_ZN4HPHP8f_hebrevERKNS_6StringEi");

Value* fh_hebrevc(Value* _rv, Value* hebrew_text, int max_chars_per_line) asm("_ZN4HPHP9f_hebrevcERKNS_6StringEi");

} // namespace HPHP
