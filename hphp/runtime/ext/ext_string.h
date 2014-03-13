/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/zend/zend-html.h"
#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/bstring.h"
#include <langinfo.h>
#include "hphp/runtime/ext/ext_class.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// transformations and manipulations

String f_addcslashes(const String& str, const String& charlist);
String f_stripcslashes(const String& str);
String f_addslashes(const String& str);
String f_stripslashes(const String& str);
String f_bin2hex(const String& str);
Variant f_hex2bin(const String& str);
String f_nl2br(const String& str, bool is_xhtml = true);
String f_quotemeta(const String& str);
String f_str_shuffle(const String& str);
String f_strrev(const String& str);
String f_strtolower(String str);
String f_strtoupper(String str);
String f_ucfirst(String str);
String f_lcfirst(String str);
String f_ucwords(String str);
String f_strip_tags(const String& str, const String& allowable_tags = "");
String f_trim(String str, const String& charlist = k_HPHP_TRIM_CHARLIST);
String f_ltrim(String str, const String& charlist = k_HPHP_TRIM_CHARLIST);
String f_rtrim(String str, const String& charlist = k_HPHP_TRIM_CHARLIST);
String f_chop(String str, const String& charlist = k_HPHP_TRIM_CHARLIST);
Variant f_explode(const String& delimiter, const String& str, int limit = 0x7FFFFFFF);

String f_implode(const Variant& arg1, const Variant& arg2 = null_variant);

String f_join(const Variant& glue, const Variant& pieces = null_variant);
Variant f_str_split(const String& str, int split_length = 1);
Variant f_chunk_split(const String& body, int chunklen = 76,
                      const String& end = "\r\n");

Variant f_strtok(const String& str, const Variant& token = null_variant);

Variant f_str_replace(const Variant& search, const Variant& replace, const Variant& subject,
                      VRefParam count = uninit_null());
Variant f_str_ireplace(const Variant& search, const Variant& replace, const Variant& subject,
                       VRefParam count = uninit_null());
Variant f_substr_replace(const Variant& str, const Variant& replacement, const Variant& start,
                         const Variant& length = 0x7FFFFFFF);

Variant f_substr(const String& str, int start, int length = 0x7FFFFFFF);
String f_str_pad(const String& input, int pad_length, const String& pad_string = " ",
                 int pad_type = k_STR_PAD_RIGHT);
String f_str_repeat(const String& input, int multiplier);

///////////////////////////////////////////////////////////////////////////////
// encoding/decoding

String f_html_entity_decode(const String& str, int quote_style = k_ENT_COMPAT,
                            const String& charset = "UTF-8");
String f_htmlentities(const String& str, int quote_style = k_ENT_COMPAT,
                      const String& charset = "UTF-8",
                      bool double_encode = true);
String f_htmlspecialchars_decode(const String& str,
                                 int quote_style = k_ENT_COMPAT);
String f_htmlspecialchars(const String& str, int quote_style = k_ENT_COMPAT,
                          const String& charset = "UTF-8",
                          bool double_encode = true);
String f_fb_htmlspecialchars(const String& str, int quote_style = k_ENT_COMPAT,
                             const String& charset = "ISO-8859-1",
                             const Array& extra = empty_array);
String f_quoted_printable_encode(const String& str);
String f_quoted_printable_decode(const String& str);
Variant f_convert_uudecode(const String& data);
Variant f_convert_uuencode(const String& data);
String f_str_rot13(const String& str);
int64_t f_crc32(const String& str);
String f_crypt(const String& str, const String& salt = "");
String f_md5(const String& str, bool raw_output = false);
String f_sha1(const String& str, bool raw_output = false);
Variant f_strtr(const String& str, const Variant& from, const Variant& to = null_variant);

String f_convert_cyr_string(const String& str, const String& from, const String& to);

Array f_get_html_translation_table(int table = 0,
                                   int quote_style = k_ENT_COMPAT,
                                   const String& encoding = "UTF-8");

String f_hebrev(const String& hebrew_text, int max_chars_per_line = 0);

String f_hebrevc(const String& hebrew_text, int max_chars_per_line = 0);

Variant f_setlocale(int _argc, int category, const Variant& locale, const Array& _argv = null_array);
Array f_localeconv();

String f_nl_langinfo(int item);

///////////////////////////////////////////////////////////////////////////////
// input/output

Variant f_printf(int _argc, const String& format, const Array& _argv = null_array);
Variant f_vprintf(const String& format, const Array& args);
Variant f_sprintf(int _argc, const String& format, const Array& _argv = null_array);
Variant f_vsprintf(const String& format, const Array& args);

Variant f_sscanf(int _argc, const String& str, const String& format, const Array& _argv = null_array);

String f_chr(int64_t ascii);
int64_t f_ord(const String& str);
Variant f_money_format(const String& format, double number);
String f_number_format(double number, int decimals = 0, const Variant& dec_point = ".",
                       const Variant& thousands_sep = ",");

///////////////////////////////////////////////////////////////////////////////
// analysis

int64_t f_strcmp(const String& str1, const String& str2);
Variant f_strncmp(const String& str1, const String& str2, int len);
int64_t f_strnatcmp(const String& str1, const String& str2);
int64_t f_strcasecmp(const String& str1, const String& str2);
Variant f_strncasecmp(const String& str1, const String& str2, int len);
int64_t f_strnatcasecmp(const String& str1, const String& str2);
int64_t f_strcoll(const String& str1, const String& str2);

Variant f_substr_compare(const String& main_str, const String& str, int offset,
                         int length = INT_MAX, bool case_insensitivity = false);

Variant f_strrchr(const String& haystack, const Variant& needle);
Variant f_strstr(const String& haystack, const Variant& needle, bool before_needle = false);
Variant f_stristr(const String& haystack, const Variant& needle);
Variant f_strpbrk(const String& haystack, const String& char_list);

Variant f_strchr(const String& haystack, const Variant& needle);

Variant f_strpos(const String& haystack, const Variant& needle, int offset = 0);
Variant f_stripos(const String& haystack, const Variant& needle, int offset = 0);
Variant f_strrpos(const String& haystack, const Variant& needle, int offset = 0);
Variant f_strripos(const String& haystack, const Variant& needle, int offset = 0);

Variant f_substr_count(const String& haystack, const String& needle, int offset = 0,
                       int length = 0x7FFFFFFF);
Variant f_strspn(const String& str1, const String& str2, int start = 0,
                 int length = 0x7FFFFFFF);
Variant f_strcspn(const String& str1, const String& str2, int start = 0,
                  int length = 0x7FFFFFFF);

Variant f_strlen(const Variant& vstr);

Array f_str_getcsv(const String& str,
                   const String& delimiter = ",",
                   const String& enclosure = "\"",
                   const String& escape = "\\");
Variant f_count_chars(const String& str, int64_t mode = 0);

Variant f_str_word_count(const String& str, int64_t format = 0, const String& charlist = "");

int64_t f_levenshtein(const String& str1, const String& str2, int cost_ins = 1,
                      int cost_rep = 1, int cost_del = 1);
int64_t f_similar_text(const String& first, const String& second, VRefParam percent = uninit_null());
Variant f_soundex(const String& str);
Variant f_metaphone(const String& str, int phones = 0);

///////////////////////////////////////////////////////////////////////////////
// special

void f_parse_str(const String& str, VRefParam arr = uninit_null());

///////////////////////////////////////////////////////////////////////////////
}

#endif
