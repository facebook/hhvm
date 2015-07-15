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
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/util/bstring.h"
#include <langinfo.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// transformations and manipulations

extern const HPHP::StaticString k_HPHP_TRIM_CHARLIST;
extern const int64_t k_STR_PAD_RIGHT;
extern const int64_t k_ENT_COMPAT;

String HHVM_FUNCTION(addcslashes,
                     const String& str,
                     const String& charlist);
String HHVM_FUNCTION(stripcslashes,
                     const String& str);
String HHVM_FUNCTION(addslashes,
                     const String& str);
String HHVM_FUNCTION(stripslashes,
                     const String& str);
String HHVM_FUNCTION(bin2hex,
                     const String& str);
Variant HHVM_FUNCTION(hex2bin,
                      const String& str);
String HHVM_FUNCTION(nl2br,
                     const String& str,
                     bool is_xhtml = true);
String HHVM_FUNCTION(quotemeta,
                     const String& str);
String HHVM_FUNCTION(str_shuffle,
                     const String& str);
String HHVM_FUNCTION(strrev,
                     const String& str);
String HHVM_FUNCTION(strtolower,
                     const String& str);
String HHVM_FUNCTION(strtoupper,
                     const String& str);
String HHVM_FUNCTION(ucfirst,
                     const String& str);
String HHVM_FUNCTION(lcfirst,
                     const String& str);
String HHVM_FUNCTION(ucwords,
                     const String& str);
String HHVM_FUNCTION(strip_tags,
                     const String& str,
                     const Variant& allowable_tags = "");
String HHVM_FUNCTION(trim,
                     const String& str,
                     const String& charlist = k_HPHP_TRIM_CHARLIST);
String HHVM_FUNCTION(ltrim,
                     const String& str,
                     const String& charlist = k_HPHP_TRIM_CHARLIST);
String HHVM_FUNCTION(rtrim,
                     const String& str,
                     const String& charlist = k_HPHP_TRIM_CHARLIST);
String HHVM_FUNCTION(chop,
                     const String& str,
                     const String& charlist = k_HPHP_TRIM_CHARLIST);
Variant HHVM_FUNCTION(explode,
                      const String& delimiter,
                      const String& str,
                      int limit = 0x7FFFFFFF);
String HHVM_FUNCTION(implode,
                     const Variant& arg1,
                     const Variant& arg2 = null_variant);
String HHVM_FUNCTION(join,
                     const Variant& arg1,
                     const Variant& arg2 = null_variant);
Variant HHVM_FUNCTION(str_split,
                      const String& str,
                      int64_t split_length = 1);
Variant HHVM_FUNCTION(chunk_split,
                      const String& body,
                      int chunklen = 76,
                      const String& end = "\r\n");
Variant HHVM_FUNCTION(strtok,
                      const String& str,
                      const Variant& token = null_variant);
Variant HHVM_FUNCTION(str_replace,
                      const Variant& search,
                      const Variant& replace,
                      const Variant& subject,
                      VRefParam count = uninit_null());
Variant HHVM_FUNCTION(str_ireplace,
                      const Variant& search,
                      const Variant& replace,
                      const Variant& subject,
                      VRefParam count = uninit_null());
Variant HHVM_FUNCTION(substr_replace,
                      const Variant& str,
                      const Variant& replacement,
                      const Variant& start,
                      const Variant& length = 0x7FFFFFFF);
String HHVM_FUNCTION(str_pad,
                     const String& input,
                     int pad_length,
                     const String& pad_string = " ",
                     int pad_type = k_STR_PAD_RIGHT);
String HHVM_FUNCTION(str_repeat,
                     const String& input,
                     int multiplier);

///////////////////////////////////////////////////////////////////////////////
// encoding/decoding

String HHVM_FUNCTION(html_entity_decode,
                     const String& str,
                     int quote_style = k_ENT_COMPAT,
                     const String& charset = "UTF-8");
String HHVM_FUNCTION(htmlentities,
                     const String& str,
                     int quote_style = k_ENT_COMPAT,
                     const String& charset = "UTF-8",
                     bool double_encode = true);
String HHVM_FUNCTION(htmlspecialchars_decode,
                     const String& str,
                     int quote_style = k_ENT_COMPAT);
String HHVM_FUNCTION(htmlspecialchars,
                     const String& str,
                     int quote_style = k_ENT_COMPAT,
                     const String& charset = "UTF-8",
                     bool double_encode = true);
String HHVM_FUNCTION(fb_htmlspecialchars,
                     const String& str,
                     int quote_style = k_ENT_COMPAT,
                     const String& charset = "ISO-8859-1",
                     const Variant& extra = empty_array_ref);
String HHVM_FUNCTION(quoted_printable_encode,
                     const String& str);
String HHVM_FUNCTION(quoted_printable_decode,
                     const String& str);
Variant HHVM_FUNCTION(convert_uudecode,
                      const String& data);
Variant HHVM_FUNCTION(convert_uuencode,
                      const String& data);
String HHVM_FUNCTION(str_rot13,
                     const String& str);
int64_t HHVM_FUNCTION(crc32,
                      const String& str);
String HHVM_FUNCTION(crypt,
                     const String& str,
                     const String& salt = "");
String HHVM_FUNCTION(md5,
                     const String& str,
                     bool raw_output = false);
String HHVM_FUNCTION(sha1,
                     const String& str,
                     bool raw_output = false);
Variant HHVM_FUNCTION(strtr,
                      const String& str,
                      const Variant& from,
                      const Variant& to = null_variant);
String HHVM_FUNCTION(convert_cyr_string,
                     const String& str,
                     const String& from,
                     const String& to);
Array HHVM_FUNCTION(get_html_translation_table,
                    int table = 0,
                    int quote_style = k_ENT_COMPAT,
                    const String& encoding = "UTF-8");
String HHVM_FUNCTION(hebrev,
                     const String& hebrew_text,
                     int max_chars_per_line = 0);
String HHVM_FUNCTION(hebrevc,
                     const String& hebrew_text,
                     int max_chars_per_line = 0);
Variant HHVM_FUNCTION(setlocale,
                      int category,
                      const Variant& locale,
                      const Array& _argv = null_array);
Array HHVM_FUNCTION(localeconv);
String HHVM_FUNCTION(nl_langinfo,
                     int item);

///////////////////////////////////////////////////////////////////////////////
// input/output

Variant f_printf(int _argc,
                 const String& format,
                 const Array& _argv = null_array);
Variant f_vprintf(const String& format,
                  const Array& args);
Variant f_sprintf(int _argc,
                  const String& format,
                  const Array& _argv = null_array);
Variant f_vsprintf(const String& format,
                   const Array& args);


Variant sscanfImpl(const String& str,
                   const String& format,
                   const std::vector<Variant*>& args);
TypedValue* HHVM_FN(sscanf)(ActRec* ar);
String HHVM_FUNCTION(chr, const Variant& ascii);
int64_t HHVM_FUNCTION(ord,
                      const String& str);
Variant HHVM_FUNCTION(money_format,
                      const String& format,
                      double number);
String HHVM_FUNCTION(number_format,
                     double number,
                     int decimals = 0,
                     const Variant& dec_point = ".",
                     const Variant& thousands_sep = ",");

///////////////////////////////////////////////////////////////////////////////
// analysis

bool str_contains_any_of(const String& haystack, const String& char_list);

int64_t HHVM_FUNCTION(strcmp,
                      const String& str1,
                      const String& str2);
Variant HHVM_FUNCTION(strncmp,
                      const String& str1,
                      const String& str2,
                      int len);
int64_t HHVM_FUNCTION(strnatcmp,
                      const String& str1,
                      const String& str2);
int64_t HHVM_FUNCTION(strcasecmp,
                      const String& str1,
                      const String& str2);
Variant HHVM_FUNCTION(strncasecmp,
                      const String& str1,
                      const String& str2,
                      int len);
int64_t HHVM_FUNCTION(strnatcasecmp,
                      const String& str1,
                      const String& str2);
int64_t HHVM_FUNCTION(strcoll,
                      const String& str1,
                      const String& str2);
Variant HHVM_FUNCTION(substr_compare,
                      const String& main_str,
                      const String& str,
                      int offset,
                      int length = INT_MAX,
                      bool case_insensitivity = false);
Variant HHVM_FUNCTION(strchr,
                      const String& haystack,
                      const Variant& needle);
Variant HHVM_FUNCTION(strrchr,
                      const String& haystack,
                      const Variant& needle);
Variant HHVM_FUNCTION(strstr,
                      const String& haystack,
                      const Variant& needle,
                      bool before_needle = false);
Variant HHVM_FUNCTION(stristr,
                      const String& haystack,
                      const Variant& needle,
                      bool before_needle = false);
Variant HHVM_FUNCTION(strpbrk,
                      const String& haystack,
                      const String& char_list);
Variant HHVM_FUNCTION(strpos,
                      const String& haystack,
                      const Variant& needle,
                      int offset = 0);
Variant HHVM_FUNCTION(stripos,
                      const String& haystack,
                      const Variant& needle,
                      int offset = 0);
Variant HHVM_FUNCTION(strrpos,
                      const String& haystack,
                      const Variant& needle,
                      int offset = 0);
Variant HHVM_FUNCTION(strripos,
                      const String& haystack,
                      const Variant& needle,
                      int offset = 0);
Variant HHVM_FUNCTION(substr_count,
                      const String& haystack,
                      const String& needle,
                      int offset = 0,
                      int length = 0x7FFFFFFF);
Variant HHVM_FUNCTION(strspn,
                      const String& str1,
                      const String& str2,
                      int start = 0,
                      int length = 0x7FFFFFFF);
Variant HHVM_FUNCTION(strcspn,
                      const String& str1,
                      const String& str2,
                      int start = 0,
                      int length = 0x7FFFFFFF);
Variant HHVM_FUNCTION(strlen,
                      const Variant& vstr);
Array HHVM_FUNCTION(str_getcsv,
                    const String& str,
                    const String& delimiter = ",",
                    const String& enclosure = "\"",
                    const String& escape = "\\");
Variant HHVM_FUNCTION(count_chars,
                      const String& str,
                      int64_t mode = 0);
Variant HHVM_FUNCTION(str_word_count,
                      const String& str,
                      int64_t format = 0,
                      const String& charlist = "");
int64_t HHVM_FUNCTION(levenshtein,
                      const String& str1,
                      const String& str2,
                      int cost_ins = 1,
                      int cost_rep = 1,
                      int cost_del = 1);
int64_t HHVM_FUNCTION(similar_text,
                      const String& first,
                      const String& second,
                      VRefParam percent = uninit_null());
Variant HHVM_FUNCTION(soundex,
                      const String& str);
Variant HHVM_FUNCTION(metaphone,
                      const String& str,
                      int phones = 0);

///////////////////////////////////////////////////////////////////////////////

}

#endif
