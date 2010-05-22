/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __EXT_STRING_H__
#define __EXT_STRING_H__

#include <runtime/base/base_includes.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_printf.h>
#include <runtime/base/zend/zend_html.h>
#include <langinfo.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// transformations and manipulations

inline String f_addcslashes(CStrRef str, CStrRef charlist) {
  return StringUtil::CEncode(str, charlist);
}
inline String f_stripcslashes(CStrRef str) {
  return StringUtil::CDecode(str);
}
inline String f_addslashes(CStrRef str) {
  return StringUtil::SqlEncode(str);
}
inline String f_stripslashes(CStrRef str) {
  return StringUtil::SqlDecode(str);
}
inline String f_bin2hex(CStrRef str) {
  return StringUtil::HexEncode(str);
}
inline String f_nl2br(CStrRef str) {
  return str.replace("\n", "<br />\n");
}
inline String f_quotemeta(CStrRef str) {
  return StringUtil::RegExEncode(str);
}
inline String f_str_shuffle(CStrRef str) {
  return StringUtil::Shuffle(str);
}
inline String f_strrev(CStrRef str) {
  return StringUtil::Reverse(str);
}
inline String f_strtolower(CStrRef str) {
  return StringUtil::ToLower(str);
}
inline String f_strtoupper(CStrRef str) {
  return StringUtil::ToUpper(str, StringUtil::ToUpperAll);
}
inline String f_ucfirst(CStrRef str) {
  return StringUtil::ToUpper(str, StringUtil::ToUpperFirst);
}
inline String f_ucwords(CStrRef str) {
  return StringUtil::ToUpper(str, StringUtil::ToUpperWords);
}
inline String f_strip_tags(CStrRef str, CStrRef allowable_tags = "") {
  return StringUtil::StripHTMLTags(str, allowable_tags);
}
inline String f_trim(CStrRef str, CStrRef charlist = k_HPHP_TRIM_CHARLIST) {
  return StringUtil::Trim(str, StringUtil::TrimBoth, charlist);
}
inline String f_ltrim(CStrRef str, CStrRef charlist = k_HPHP_TRIM_CHARLIST) {
  return StringUtil::Trim(str, StringUtil::TrimLeft, charlist);
}
inline String f_rtrim(CStrRef str, CStrRef charlist = k_HPHP_TRIM_CHARLIST) {
  return StringUtil::Trim(str, StringUtil::TrimRight, charlist);
}
inline String f_chop(CStrRef str, CStrRef charlist = k_HPHP_TRIM_CHARLIST) {
  return StringUtil::Trim(str, StringUtil::TrimRight, charlist);
}
inline Variant f_explode(CStrRef delimiter, CStrRef str, int limit = 0x7FFFFFFF) {
  return StringUtil::Explode(str, delimiter, limit);
}

String f_implode(CVarRef arg1, CVarRef arg2 = null_variant);

inline String f_join(CVarRef glue, CVarRef pieces = null_variant) {
  return f_implode(glue, pieces);
}
inline Variant f_str_split(CStrRef str, int split_length = 1) {
  return StringUtil::Split(str, split_length);
}
inline Variant f_chunk_split(CStrRef body, int chunklen = 76,
                             CStrRef end = "\r\n") {
  return StringUtil::ChunkSplit(body, chunklen, end);
}

Variant f_strtok(CStrRef str, CVarRef token = null_variant);

Variant f_str_replace(CVarRef search, CVarRef replace, CVarRef subject,
                      Variant count = null);
Variant f_str_ireplace(CVarRef search, CVarRef replace, CVarRef subject,
                       Variant count = null);
Variant f_substr_replace(CVarRef str, CVarRef replacement, CVarRef start,
                         CVarRef length = 0x7FFFFFFF);

inline Variant f_substr(CStrRef str, int start, int length = 0x7FFFFFFF) {
  String ret = str.substr(start, length);
  if (ret.isNull()) return false;
  return ret;
}
inline String f_str_pad(CStrRef input, int pad_length, CStrRef pad_string = " ",
                        int pad_type = k_STR_PAD_RIGHT) {
  return StringUtil::Pad(input, pad_length, pad_string,
                         (StringUtil::PadType)pad_type);
}
inline String f_str_repeat(CStrRef input, int multiplier) {
  return StringUtil::Repeat(input, multiplier);
}
inline Variant f_wordwrap(CStrRef str, int width = 75, CStrRef wordbreak = "\n",
                         bool cut = false) {
  String ret = StringUtil::WordWrap(str, width, wordbreak, cut);
  if (ret.isNull()) return false;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// encoding/decoding

inline String f_html_entity_decode(CStrRef str, int quote_style = k_ENT_COMPAT,
                                   CStrRef charset = "") {
  // dropping quote_style parameter, as I don't see why decoding needs to check
  // dropping charset parameter and see runtime/base/zend_html.h
  return StringUtil::HtmlDecode(str);
}
inline String f_htmlentities(CStrRef str, int quote_style = k_ENT_COMPAT,
                             CStrRef charset = "", bool double_encode = true) {
  // dropping charset and double_encode parameters and see runtime/base/zend_html.h
  return StringUtil::HtmlEncode(str, (StringUtil::QuoteStyle)quote_style);
}
inline String f_htmlspecialchars_decode(CStrRef str,
                                        int quote_style = k_ENT_COMPAT) {
  // dropping quote_style parameter, as I don't see why decoding needs to check
  return StringUtil::HtmlDecode(str);
}
inline String f_htmlspecialchars(CStrRef str, int quote_style = k_ENT_COMPAT,
                                 CStrRef charset = "",
                                 bool double_encode = true) {
  // dropping charset and double_encode parameters and see runtime/base/zend_html.h
  return StringUtil::HtmlEncode(str, (StringUtil::QuoteStyle)quote_style);
}
inline String f_quoted_printable_encode(CStrRef str) {
  return StringUtil::QuotedPrintableEncode(str);
}
inline String f_quoted_printable_decode(CStrRef str) {
  return StringUtil::QuotedPrintableDecode(str);
}
inline Variant f_convert_uudecode(CStrRef data) {
  String ret = StringUtil::UUDecode(data);
  if (ret.isNull()) {
    return false; // bad format
  }
  return ret;
}
inline Variant f_convert_uuencode(CStrRef data) {
  if (data.empty()) return false;
  return StringUtil::UUEncode(data);
}
inline String f_str_rot13(CStrRef str) {
  return StringUtil::ROT13(str);
}
inline int64 f_crc32(CStrRef str) {
  return (uint32)StringUtil::CRC32(str);
}
inline String f_crypt(CStrRef str, CStrRef salt = "") {
  return StringUtil::Crypt(str, salt);
}
inline String f_md5(CStrRef str, bool raw_output = false) {
  return StringUtil::MD5(str, raw_output);
}
inline String f_sha1(CStrRef str, bool raw_output = false) {
  return StringUtil::SHA1(str, raw_output);
}
Variant f_strtr(CStrRef str, CVarRef from, CVarRef to = null_variant);

String f_convert_cyr_string(CStrRef str, CStrRef from, CStrRef to);

inline Array f_get_html_translation_table(int table = 0,
                                          int quote_style = k_ENT_COMPAT) {
  return string_get_html_translation_table(table, quote_style);
}

String f_hebrev(CStrRef hebrew_text, int max_chars_per_line = 0);

String f_hebrevc(CStrRef hebrew_text, int max_chars_per_line = 0);

Variant f_setlocale(int _argc, int category, CVarRef locale, CArrRef _argv = null_array);
Array f_localeconv();

inline String f_nl_langinfo(int item) {
#ifdef MAC_OS_X
  return String();
#else
  return nl_langinfo(item);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// input/output

inline void f_echo(int _argc, CStrRef arg, CArrRef _argv = null_array) {
  echo(arg);
  for (int i = 0; i < _argv.size(); i++) echo(_argv[i]);
}
inline int f_print(CStrRef arg) {
  return print(arg);
}
inline Variant f_printf(int _argc, CStrRef format, CArrRef _argv = null_array) {
  int len = 0; char *output = string_printf(format.data(), format.size(),
                                            _argv, &len);
  if (output == NULL) return false;
  echo(output); free(output);
  return len;
}
inline Variant f_vprintf(CStrRef format, CArrRef args) {
  int len = 0; char *output = string_printf(format.data(), format.size(),
                                            args, &len);
  if (output == NULL) return false;
  echo(output); free(output);
  return len;
}
inline Variant f_sprintf(int _argc, CStrRef format, CArrRef _argv = null_array) {
  char *output = string_printf(format.data(), format.size(), _argv, NULL);
  if (output == NULL) return false;
  return String(output, AttachString);
}
inline Variant f_vsprintf(CStrRef format, CArrRef args) {
  char *output = string_printf(format.data(), format.size(), args, NULL);
  if (output == NULL) return false;
  return String(output, AttachString);
}

Variant f_sscanf(int _argc, CStrRef str, CStrRef format, CArrRef _argv = null_array);

inline String f_chr(int64 ascii) {
  char buf[2]; buf[0] = ascii; buf[1] = 0;
  return String(buf, 1, CopyString);
}
inline int64 f_ord(CStrRef str) {
  return (int64)(unsigned char)(*((const char *)str));
}
inline Variant f_money_format(CStrRef format, double number) {
  String s = StringUtil::MoneyFormat(format, number);
  if (s.isNull()) return false;
  return s;
}
String f_number_format(double number, int decimals = 0, CStrRef dec_point = ".",
                       CStrRef thousands_sep = ",");

///////////////////////////////////////////////////////////////////////////////
// analysis

inline int f_strcmp(CStrRef str1, CStrRef str2) {
  return string_strcmp(str1.data(), str1.size(), str2.data(), str2.size());
}
inline int f_strncmp(CStrRef str1, CStrRef str2, int len) {
  return string_strncmp(str1.data(), str1.size(), str2.data(), str2.size(),
                        len);
}
inline int f_strnatcmp(CStrRef str1, CStrRef str2) {
  return string_natural_cmp(str1.data(), str1.size(), str2.data(), str2.size(),
                            false);
}
inline int f_strcasecmp(CStrRef str1, CStrRef str2) {
  return string_strcasecmp(str1.data(), str1.size(), str2.data(), str2.size());
}
inline int f_strncasecmp(CStrRef str1, CStrRef str2, int len) {
  return string_strncasecmp(str1.data(), str1.size(), str2.data(), str2.size(),
                            len);
}
inline int f_strnatcasecmp(CStrRef str1, CStrRef str2) {
  return string_natural_cmp(str1.data(), str1.size(), str2.data(), str2.size(),
                            true);
}
inline int f_strcoll(CStrRef str1, CStrRef str2) {
  return strcoll(str1, str2);
}

Variant f_substr_compare(CStrRef main_str, CStrRef str, int offset,
                         int length = 0, bool case_insensitivity = false);

Variant f_strrchr(CStrRef haystack, CVarRef needle);
Variant f_strstr(CStrRef haystack, CVarRef needle);
Variant f_stristr(CStrRef haystack, CVarRef needle);
Variant f_strpbrk(CStrRef haystack, CStrRef char_list);

inline Variant f_strchr(CStrRef haystack, CVarRef needle) {
  return f_strstr(haystack, needle);
}

Variant f_strpos(CStrRef haystack, CVarRef needle, int offset = 0);
Variant f_stripos(CStrRef haystack, CVarRef needle, int offset = 0);
Variant f_strrpos(CStrRef haystack, CVarRef needle, int offset = -1);
Variant f_strripos(CStrRef haystack, CVarRef needle, int offset = -1);

Variant f_substr_count(CStrRef haystack, CStrRef needle, int offset = 0,
                       int length = 0x7FFFFFFF);
Variant f_strspn(CStrRef str1, CStrRef str2, int start = 0,
                 int length = 0x7FFFFFFF);
Variant f_strcspn(CStrRef str1, CStrRef str2, int start = 0,
                  int length = 0x7FFFFFFF);
inline int f_strlen(CStrRef str) {
  return str.size();
}

Variant f_count_chars(CStrRef str, int64 mode = 0);

Variant f_str_word_count(CStrRef str, int64 format = 0, CStrRef charlist = "");

inline int f_levenshtein(CStrRef str1, CStrRef str2, int cost_ins = 1,
                         int cost_rep = 1, int cost_del = 1) {
  return string_levenshtein(str1, str1.size(), str2, str2.size(),
                            cost_ins, cost_rep, cost_del);
}
inline int f_similar_text(CStrRef first, CStrRef second, Variant percent = null) {
  float p;
  int ret = string_similar_text(first, first.size(), second, second.size(),
                                &p);
  percent = p;
  return ret;
}
inline Variant f_soundex(CStrRef str) {
  if (str.empty()) return false;
  return String(string_soundex(str), AttachString);
}
inline Variant f_metaphone(CStrRef str, int phones = 0) {
  char *ret = string_metaphone(str, str.size(), 0, 1);
  if (ret) {
    return String(ret, AttachString);
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// special

void f_parse_str(CStrRef str, Variant arr = null);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_STRING_H__
