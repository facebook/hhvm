/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_STRING_H_
#define incl_HPHP_EXT_STRING_H_

#include "util/zend/zend_html.h"
#include <runtime/base/base_includes.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_printf.h>
#include <runtime/base/bstring.h>
#include <langinfo.h>
#include <runtime/ext/ext_class.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// transformations and manipulations

String f_addcslashes(CStrRef str, CStrRef charlist);
String f_stripcslashes(CStrRef str);
String f_addslashes(CStrRef str);
String f_stripslashes(CStrRef str);
String f_bin2hex(CStrRef str);
Variant f_hex2bin(CStrRef str);
String f_nl2br(CStrRef str);
String f_quotemeta(CStrRef str);
String f_str_shuffle(CStrRef str);
String f_strrev(CStrRef str);
String f_strtolower(CStrRef str);
String f_strtoupper(CStrRef str);
String f_ucfirst(CStrRef str);
String f_lcfirst(CStrRef str);
String f_ucwords(CStrRef str);
String f_strip_tags(CStrRef str, CStrRef allowable_tags = "");
String f_trim(CStrRef str, CStrRef charlist = k_HPHP_TRIM_CHARLIST);
String f_ltrim(CStrRef str, CStrRef charlist = k_HPHP_TRIM_CHARLIST);
String f_rtrim(CStrRef str, CStrRef charlist = k_HPHP_TRIM_CHARLIST);
String f_chop(CStrRef str, CStrRef charlist = k_HPHP_TRIM_CHARLIST);
Variant f_explode(CStrRef delimiter, CStrRef str, int limit = 0x7FFFFFFF);

String f_implode(CVarRef arg1, CVarRef arg2 = null_variant);

String f_join(CVarRef glue, CVarRef pieces = null_variant);
Variant f_str_split(CStrRef str, int split_length = 1);
Variant f_chunk_split(CStrRef body, int chunklen = 76,
                      CStrRef end = "\r\n");

Variant f_strtok(CStrRef str, CVarRef token = null_variant);

Variant f_str_replace(CVarRef search, CVarRef replace, CVarRef subject,
                      VRefParam count = uninit_null());
Variant f_str_ireplace(CVarRef search, CVarRef replace, CVarRef subject,
                       VRefParam count = uninit_null());
Variant f_substr_replace(CVarRef str, CVarRef replacement, CVarRef start,
                         CVarRef length = 0x7FFFFFFF);

Variant f_substr(CStrRef str, int start, int length = 0x7FFFFFFF);
String f_str_pad(CStrRef input, int pad_length, CStrRef pad_string = " ",
                 int pad_type = k_STR_PAD_RIGHT);
String f_str_repeat(CStrRef input, int multiplier);
Variant f_wordwrap(CStrRef str, int width = 75, CStrRef wordbreak = "\n",
                   bool cut = false);

///////////////////////////////////////////////////////////////////////////////
// encoding/decoding

String f_html_entity_decode(CStrRef str, int quote_style = k_ENT_COMPAT,
                            CStrRef charset = "ISO-8859-1");
String f_htmlentities(CStrRef str, int quote_style = k_ENT_COMPAT,
                      CStrRef charset = "ISO-8859-1",
                      bool double_encode = true);
String f_htmlspecialchars_decode(CStrRef str,
                                 int quote_style = k_ENT_COMPAT);
String f_htmlspecialchars(CStrRef str, int quote_style = k_ENT_COMPAT,
                          CStrRef charset = "ISO-8859-1",
                          bool double_encode = true);
String f_fb_htmlspecialchars(CStrRef str, int quote_style = k_ENT_COMPAT,
                             CStrRef charset = "ISO-8859-1",
                             CArrRef extra = empty_array);
String f_quoted_printable_encode(CStrRef str);
String f_quoted_printable_decode(CStrRef str);
Variant f_convert_uudecode(CStrRef data);
Variant f_convert_uuencode(CStrRef data);
String f_str_rot13(CStrRef str);
int64_t f_crc32(CStrRef str);
String f_crypt(CStrRef str, CStrRef salt = "");
String f_md5(CStrRef str, bool raw_output = false);
String f_sha1(CStrRef str, bool raw_output = false);
Variant f_strtr(CStrRef str, CVarRef from, CVarRef to = null_variant);

String f_convert_cyr_string(CStrRef str, CStrRef from, CStrRef to);

Array f_get_html_translation_table(int table = 0,
                                   int quote_style = k_ENT_COMPAT);

String f_hebrev(CStrRef hebrew_text, int max_chars_per_line = 0);

String f_hebrevc(CStrRef hebrew_text, int max_chars_per_line = 0);

Variant f_setlocale(int _argc, int category, CVarRef locale, CArrRef _argv = null_array);
Array f_localeconv();

String f_nl_langinfo(int item);

///////////////////////////////////////////////////////////////////////////////
// input/output

Variant f_printf(int _argc, CStrRef format, CArrRef _argv = null_array);
Variant f_vprintf(CStrRef format, CArrRef args);
Variant f_sprintf(int _argc, CStrRef format, CArrRef _argv = null_array);
Variant f_vsprintf(CStrRef format, CArrRef args);

Variant f_sscanf(int _argc, CStrRef str, CStrRef format, CArrRef _argv = null_array);

String f_chr(int64_t ascii);
int64_t f_ord(CStrRef str);
Variant f_money_format(CStrRef format, double number);
String f_number_format(double number, int decimals = 0, CStrRef dec_point = ".",
                       CStrRef thousands_sep = ",");

///////////////////////////////////////////////////////////////////////////////
// analysis

int64_t f_strcmp(CStrRef str1, CStrRef str2);
Variant f_strncmp(CStrRef str1, CStrRef str2, int len);
int64_t f_strnatcmp(CStrRef str1, CStrRef str2);
int64_t f_strcasecmp(CStrRef str1, CStrRef str2);
Variant f_strncasecmp(CStrRef str1, CStrRef str2, int len);
int64_t f_strnatcasecmp(CStrRef str1, CStrRef str2);
int64_t f_strcoll(CStrRef str1, CStrRef str2);

Variant f_substr_compare(CStrRef main_str, CStrRef str, int offset,
                         int length = INT_MAX, bool case_insensitivity = false);

Variant f_strrchr(CStrRef haystack, CVarRef needle);
Variant f_strstr(CStrRef haystack, CVarRef needle, bool before_needle = false);
Variant f_stristr(CStrRef haystack, CVarRef needle);
Variant f_strpbrk(CStrRef haystack, CStrRef char_list);

Variant f_strchr(CStrRef haystack, CVarRef needle);

Variant f_strpos(CStrRef haystack, CVarRef needle, int offset = 0);
Variant f_stripos(CStrRef haystack, CVarRef needle, int offset = 0);
Variant f_strrpos(CStrRef haystack, CVarRef needle, int offset = 0);
Variant f_strripos(CStrRef haystack, CVarRef needle, int offset = 0);

Variant f_substr_count(CStrRef haystack, CStrRef needle, int offset = 0,
                       int length = 0x7FFFFFFF);
Variant f_strspn(CStrRef str1, CStrRef str2, int start = 0,
                 int length = 0x7FFFFFFF);
Variant f_strcspn(CStrRef str1, CStrRef str2, int start = 0,
                  int length = 0x7FFFFFFF);

Variant f_strlen(CVarRef vstr);

Variant f_count_chars(CStrRef str, int64_t mode = 0);

Variant f_str_word_count(CStrRef str, int64_t format = 0, CStrRef charlist = "");

int64_t f_levenshtein(CStrRef str1, CStrRef str2, int cost_ins = 1,
                      int cost_rep = 1, int cost_del = 1);
int64_t f_similar_text(CStrRef first, CStrRef second, VRefParam percent = uninit_null());
Variant f_soundex(CStrRef str);
Variant f_metaphone(CStrRef str, int phones = 0);

///////////////////////////////////////////////////////////////////////////////
// special

void f_parse_str(CStrRef str, VRefParam arr = uninit_null());

///////////////////////////////////////////////////////////////////////////////
}

#endif
