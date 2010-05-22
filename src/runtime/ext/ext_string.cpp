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

#include <runtime/ext/ext_string.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_url.h>
#include <runtime/base/zend/zend_printf.h>
#include <runtime/base/zend/zend_scanf.h>
#include <runtime/base/util/request_local.h>
#include <util/lock.h>
#include <locale.h>
#include <runtime/base/server/http_request_handler.h>
#include <runtime/base/server/http_protocol.h>

using namespace std;

namespace HPHP {

static Mutex s_mutex;
///////////////////////////////////////////////////////////////////////////////

String f_implode(CVarRef arg1, CVarRef arg2 /* = null_variant */) {
  Array items;
  String delim;
  if (arg1.is(KindOfArray)) {
    items = arg1.toArray();
    delim = arg2.toString();
  } else if (arg2.is(KindOfArray)) {
    items = arg2.toArray();
    delim = arg1.toString();
  } else {
    throw_bad_type_exception("arguments need at least one array");
    return String();
  }
  return StringUtil::Implode(items, delim);
}

class TokenizerData : public RequestEventHandler {
public:
  String str;
  int pos;
  int mask[256];

  virtual void requestInit() {
    str.reset();
    pos = 0;
    memset(&mask, 0, sizeof(mask));
  }
  virtual void requestShutdown() {
    requestInit();
  }
};
IMPLEMENT_STATIC_REQUEST_LOCAL(TokenizerData, s_tokenizer_data);

Variant f_strtok(CStrRef str, CVarRef token /* = null_variant */) {
  String stoken;
  if (!token.isNull()) {
    s_tokenizer_data->str = str;
    s_tokenizer_data->pos = 0;
    stoken = token.toString();
  } else {
    stoken = str;
  }

  String sstr = s_tokenizer_data->str;
  int pos = s_tokenizer_data->pos;
  if (pos >= sstr.size()) {
    return false;
  }

  // set up mask
  int *mask = s_tokenizer_data->mask;
  for (int i = 0; i < stoken.size(); i++) {
    mask[(unsigned char)stoken.data()[i]] = 1;
  }

  // skip leading delimiters
  const char *s0 = sstr.data();
  int i = pos;
  for (; i < sstr.size(); i++) {
    if (!mask[(unsigned char)s0[i]]) {
      break;
    }
  }
  int pos0 = i;
  for (; i < sstr.size(); i++) {
    if (mask[(unsigned char)s0[i]]) {
      break;
    }
  }

  String ret(s0 + pos0, i - pos0, CopyString);
  s_tokenizer_data->pos = i + 1;

  // reset mask
  for (int i = 0; i < stoken.size(); i++) {
    mask[(unsigned char)stoken.data()[i]] = 0;
  }
  return ret;
}

static Variant str_replace(CVarRef search, CVarRef replace, CStrRef subject,
                           int &count, bool caseSensitive) {
  if (search.is(KindOfArray)) {
    String ret = subject;

    Array searchArr = search.toArray();
    if (replace.is(KindOfArray)) {
      Array replArr = replace.toArray();
      ArrayIter replIter(replArr);
      for (ArrayIter iter(searchArr); iter; ++iter) {
        if (replIter) {
          ret = ret.replace(iter.second().toString(),
                            replIter.second().toString(),
                            count, caseSensitive);
          ++replIter;
        } else {
          ret = ret.replace(iter.second().toString(),
                            "", count, caseSensitive);
        }
      }
      return ret;
    }

    String repl = replace.toString();
    for (ArrayIter iter(searchArr); iter; ++iter) {
      ret = ret.replace(iter.second().toString(), repl, count, caseSensitive);
    }
    return ret;
  }

  if (replace.is(KindOfArray)) {
    raise_notice("Array to string conversion");
  }
  return subject.replace(search.toString(), replace.toString(), count,
                         caseSensitive);
}

static Variant str_replace(CVarRef search, CVarRef replace, CVarRef subject,
                           int &count, bool caseSensitive) {
  if (subject.is(KindOfArray)) {
    Array arr = subject.toArray();
    Array ret;
    for (ArrayIter iter(arr); iter; ++iter) {
      String replaced = str_replace(search, replace, iter.second().toString(),
                                    count, caseSensitive);
      ret.set(iter.first(), replaced);
    }
    return ret;
  }
  return str_replace(search, replace, subject.toString(), count,
                     caseSensitive);
}

Variant f_str_replace(CVarRef search, CVarRef replace, CVarRef subject,
                      Variant count /* = null */) {
  int nCount;
  Variant ret = str_replace(search, replace, subject, nCount, true);
  count = nCount;
  return ret;
}

Variant f_str_ireplace(CVarRef search, CVarRef replace, CVarRef subject,
                       Variant count /* = null */) {
  int nCount;
  Variant ret = str_replace(search, replace, subject, nCount, false);
  count = nCount;
  return ret;
}

Variant f_substr_replace(CVarRef str, CVarRef replacement, CVarRef start,
                         CVarRef length /* = 0x7FFFFFFF */) {
  if (!str.is(KindOfArray)) {
    String repl;
    if (replacement.is(KindOfArray)) {
      repl = replacement[0].toString();
    } else {
      repl = replacement.toString();
    }
    if (start.is(KindOfArray)) {
      if (!length.is(KindOfArray)) {
        throw_invalid_argument("start and length should be of same type - "
                               "numerical or array");
        return str;
      }
      Array startArr = start.toArray();
      Array lengthArr = length.toArray();
      if (startArr.size() != lengthArr.size()) {
        throw_invalid_argument("start and length: (different item count)");
        return str;
      }
      throw_invalid_argument("start and length as arrays not implemented");
      return str;
    }
    return str.toString().replace(start.toInt32(), length.toInt32(), repl);
  }

  Array ret;
  Array strArr = str.toArray();
  Array startArr = start.toArray();
  Array lengthArr = length.toArray();
  ArrayIter startIter(startArr);
  ArrayIter lengthIter(lengthArr);

  if (replacement.is(KindOfArray)) {
    Array replArr = replacement.toArray();
    ArrayIter replIter(replArr);
    for (ArrayIter iter(strArr); iter;
         ++iter, ++startIter, ++lengthIter) {
      int nStart = startIter.second().toInt32();
      int nLength = lengthIter.second().toInt32();
      String repl("");
      if (replIter) {
        repl = replIter.second().toString();
        ++replIter;
      }
      ret.append(iter.second().toString().replace(nStart, nLength, repl));
    }
  } else {
    String repl = replacement.toString();
    for (ArrayIter iter(strArr); iter;
         ++iter, ++startIter, ++lengthIter) {
      int nStart = startIter.second().toInt32();
      int nLength = lengthIter.second().toInt32();
      ret.append(iter.second().toString().replace(nStart, nLength, repl));
    }
  }
  return ret;
}


///////////////////////////////////////////////////////////////////////////////

Variant f_sscanf(int _argc, CStrRef str, CStrRef format, CArrRef _argv /* = null_array */) {
  Variant ret;
  int result;
  result = string_sscanf(str, format, _argv.size(), ret);
  if (SCAN_ERROR_WRONG_PARAM_COUNT == result) return null;
  if (_argv.empty()) return ret;

  if (ret.isArray()) {
    Array retArray = ret.toArray();
    for (int i = 0; i < retArray.size(); i++) {
      ((Array&)_argv).lvalAt(i) = retArray[i];
    }
    return retArray.size();
  }
  if (ret.isNull()) return 0;
  return ret;
}

String f_number_format(double number, int decimals /* = 0 */,
                       CStrRef dec_point /* = "." */,
                       CStrRef thousands_sep /* = "," */) {
  char ch_dec_point = '.';
  if (!dec_point.isNull()) {
    if (dec_point.size() >= 1) {
      ch_dec_point = ((const char *)dec_point)[0];
    } else {
      ch_dec_point = 0;
    }
  }
  char ch_thousands_sep = ',';
  if (!thousands_sep.isNull()) {
    if (thousands_sep.size() >= 1) {
      ch_thousands_sep = ((const char *)thousands_sep)[0];
    } else {
      ch_thousands_sep = 0;
    }
  }
  char *ret = string_number_format(number, decimals, ch_dec_point,
                                   ch_thousands_sep);
  return String(ret, AttachString);
}

Variant f_substr_compare(CStrRef main_str, CStrRef str, int offset,
                         int length /* = 0 */,
                         bool case_insensitivity /* = false */) {
  int s1_len = main_str.size();
  int s2_len = str.size();

  if (offset < 0) {
    offset = s1_len + offset;
    if (offset < 0) offset = 0;
  }
  if (offset > s1_len || length > s1_len - offset) {
    return false;
  }

  int cmp_len = length;
  if (length == 0) {
    cmp_len = s1_len - offset;
    if (cmp_len < s2_len) cmp_len = s2_len;
  }

  const char *s1 = main_str.data();
  if (case_insensitivity) {
    return string_ncasecmp(s1 + offset, str, cmp_len);
  }
  return string_ncmp(s1 + offset, str, cmp_len);
}

Variant f_strrchr(CStrRef haystack, CVarRef needle) {
  Variant ret = f_strrpos(haystack, needle);
  if (same(ret, false)) {
    return false;
  }
  return haystack.substr(ret.toInt32());
}

Variant f_strstr(CStrRef haystack, CVarRef needle) {
  Variant ret = f_strpos(haystack, needle);
  if (same(ret, false)) {
    return false;
  }
  return haystack.substr(ret.toInt32());
}

Variant f_stristr(CStrRef haystack, CVarRef needle) {
  Variant ret = f_stripos(haystack, needle);
  if (same(ret, false)) {
    return false;
  }
  return haystack.substr(ret.toInt32());
}

Variant f_strpbrk(CStrRef haystack, CStrRef char_list) {
  if (char_list.empty()) {
    throw_invalid_argument("char_list: (empty)");
    return false;
  }
  const char *p = strpbrk(haystack, char_list);
  if (p) {
    return String(p, CopyString);
  }
  return false;
}

Variant f_strpos(CStrRef haystack, CVarRef needle, int offset /* = 0 */) {
  int pos;
  if (needle.isString()) {
    String n(needle.toString());
    if (n.length() == 0) {
      raise_warning("Empty delimiter");
      return false;
    }
    pos = haystack.find(n, offset);
  } else {
    pos = haystack.find(needle.toByte(), offset);
  }
  if (pos >= 0) return pos;
  return false;
}

Variant f_stripos(CStrRef haystack, CVarRef needle, int offset /* = 0 */) {
  int pos;
  if (needle.isString()) {
    pos = haystack.find(needle.toString(), offset, false);
  } else {
    pos = haystack.find(needle.toByte(), offset, false);
  }
  if (pos >= 0) return pos;
  return false;
}

Variant f_strrpos(CStrRef haystack, CVarRef needle, int offset /* = -1 */) {
  if (offset < 0) offset += haystack.size();
  int pos;
  if (needle.isString()) {
    pos = haystack.rfind(needle.toString(), offset);
  } else {
    pos = haystack.rfind(needle.toByte(), offset);
  }
  if (pos >= 0) return pos;
  return false;
}

Variant f_strripos(CStrRef haystack, CVarRef needle, int offset /* = -1 */) {
  if (offset < 0) offset += haystack.size();
  int pos;
  if (needle.isString()) {
    pos = haystack.rfind(needle.toString(), offset, false);
  } else {
    pos = haystack.rfind(needle.toByte(), offset, false);
  }
  if (pos >= 0) return pos;
  return false;
}

Variant f_substr_count(CStrRef haystack, CStrRef needle, int offset /* = 0 */,
                       int length /* = 0x7FFFFFFF */) {
  int lenNeedle = needle.size();
  if (lenNeedle == 0) {
    throw_invalid_argument("needle: (empty)");
    return false;
  }

  if (offset < 0 || offset > haystack.size()) {
    throw_invalid_argument("offset: (out of range)");
    return false;
  }
  if (length == 0x7FFFFFFF) {
    length = haystack.size() - offset;
  } else if (length <= 0 || length > haystack.size() - offset) {
    throw_invalid_argument("length: (out of range)");
    return false;
  }

  int count = 0;
  int posMax = offset + length - lenNeedle;
  for (int pos = haystack.find(needle, offset);
       pos != -1 && pos <= posMax;
       pos = haystack.find(needle, pos + lenNeedle)) {
    ++count;
  }
  return count;
}

Variant f_strspn(CStrRef str1, CStrRef str2, int start /* = 0 */,
                 int length /* = 0x7FFFFFFF */) {
  String s = str1.substr(start, length);
  if (s.isNull()) return false;
  return string_span(s, s.size(), str2, str2.size());
}

Variant f_strcspn(CStrRef str1, CStrRef str2, int start /* = 0 */,
                  int length /* = 0x7FFFFFFF */) {
  String s = str1.substr(start, length);
  if (s.isNull()) return false;
  return string_cspan(s, s.size(), str2, str2.size());
}

Variant f_count_chars(CStrRef str, int64 mode /* = 0 */) {
  int chars[256];
  memset((void*)chars, 0, sizeof(chars));
  const unsigned char *buf = (const unsigned char *)(const char *)str;
  for (int len = str.size(); len > 0; len--) {
    chars[*buf++]++;
  }

  Array retarr;
  char retstr[256];
  int retlen = 0;
  switch (mode) {
  case 0:
    for (int inx = 0; inx < 256; inx++) {
      retarr.set(inx, chars[inx]);
    }
    return retarr;
  case 1:
    for (int inx = 0; inx < 256; inx++) {
      if (chars[inx] != 0) {
        retarr.set(inx, chars[inx]);
      }
    }
    return retarr;
  case 2:
    for (int inx = 0; inx < 256; inx++) {
      if (chars[inx] == 0) {
        retarr.set(inx, chars[inx]);
      }
    }
    return retarr;
  case 3:
    for (int inx = 0; inx < 256; inx++) {
      if (chars[inx] != 0) {
        retstr[retlen++] = inx;
      }
    }
    return String(retstr, retlen, CopyString);
  case 4:
    for (int inx = 0; inx < 256; inx++) {
      if (chars[inx] == 0) {
        retstr[retlen++] = inx;
      }
    }
    return String(retstr, retlen, CopyString);
  }

  throw_invalid_argument("mode: %d", mode);
  return false;
}

/**
 * Counts the number of words inside a string. If format of 1 is specified,
 * then the function will return an array containing all the words
 * found inside the string. If format of 2 is specified, then the function
 * will return an associated array where the position of the word is the key
 * and the word itself is the value.
 *
 * For the purpose of this function, 'word' is defined as a locale dependent
 * string containing alphabetic characters, which also may contain, but not
 * start with "'" and "-" characters.
 */
Variant f_str_word_count(CStrRef str, int64 format /* = 0 */,
                         CStrRef charlist /* = "" */) {
  int str_len = str.size();
  switch (format) {
  case 1:
  case 2:
    if (!str_len) {
      return Array::Create();
    }
    break;
  case 0:
    if (!str_len) {
      return 0LL;
    }
    break;
  default:
    throw_invalid_argument("format: %d", format);
    return false;
  }

  char ch[256];
  const char *char_list = charlist;
  if (*char_list) {
    string_charmask(charlist, charlist.size(), ch);
  } else {
    char_list = NULL;
  }

  int word_count = 0;
  const char *s0 = str;
  const char *p = s0;
  const char *e = p + str_len;

  // first character cannot be ' or -, unless explicitly allowed by the user
  if ((*p == '\'' && (!char_list || !ch[(int)'\''])) ||
      (*p == '-' && (!char_list || !ch[(int)'-']))) {
    p++;
  }

  // last character cannot be -, unless explicitly allowed by the user
  if (*(e - 1) == '-' && (!char_list || !ch[(int)'-'])) {
    e--;
  }

  Array ret;
  while (p < e) {
    const char *s = p;
    while (p < e &&
           (isalpha(*p) || (char_list && ch[(unsigned char)*p]) ||
            *p == '\'' || *p == '-')) {
      p++;
    }
    if (p > s) {
      switch (format) {
      case 1:
        ret.append(String(s, p - s, CopyString));
        break;
      case 2:
        ret.set((int)(s - s0), String(s, p - s, CopyString));
        break;
      default:
        word_count++;
        break;
      }
    }
    p++;
  }

  if (!format) {
    return word_count;
  }
  return ret;
}

Variant f_strtr(CStrRef str, CVarRef from, CVarRef to /* = null_variant */) {
  if (str.empty()) {
    return str;
  }

  if (!to.isNull()) {
    return StringUtil::Translate(str, from.toString(), to.toString());
  }

  if (!from.is(KindOfArray)) {
    throw_invalid_argument("2nd argument: (not array)");
    return false;
  }

  int maxlen = 0;
  int minlen = -1;
  Array arr = from.toArray();
  for (ArrayIter iter(arr); iter; ++iter) {
    String search = iter.first();
    int len = search.size();
    if (len < 1) return false;
    if (maxlen < len) maxlen = len;
    if (minlen == -1 || minlen > len) minlen = len;
  }

  const char *s = str.data();
  int slen = str.size();
  char *key = (char *)malloc(maxlen+1);

  StringBuffer result(slen);
  for (int pos = 0; pos < slen; ) {
    if ((pos + maxlen) > slen) {
      maxlen = slen - pos;
    }
    bool found = false;
    memcpy(key, s + pos, maxlen);
    for (int len = maxlen; len >= minlen; len--) {
      key[len] = 0;
      if (arr.exists(key)) {
        String replace = arr[key].toString();
        if (!replace.empty()) {
          result += replace;
        }
        pos += len;
        found = true;
        break;
      }
    }
    if (!found) {
      result += s[pos++];
    }
  }
  free(key);

  return String(result);
}

void f_parse_str(CStrRef str, Variant arr /* = null */) {
  HttpProtocol::DecodeParameters(arr, str.data(), str.size());
}

Variant f_setlocale(int _argc, int category, CVarRef locale, CArrRef _argv /* = null_array */) {
  Array argv = _argv;
  if (locale.is(KindOfArray)) {
    if (!argv.empty()) throw_invalid_argument("locale: not string)");
    argv = locale; // ignore _argv
  }

  for (int i = -1; i < argv.size(); i++) {
    String slocale;
    if (i == -1) {
      if (locale.is(KindOfArray)) continue;
      slocale = locale.toString();
    } else {
      slocale = argv[i].toString();
    }

    const char *loc = slocale;
    if (slocale.size() >= 255) {
      throw_invalid_argument("locale name is too long: %s", loc);
      return false;
    }
    if (strcmp("0", loc) == 0) {
      loc = NULL;
    }
    {
      Lock lock(s_mutex);
      const char *retval = setlocale(category, loc);
      if (retval) {
        return String(retval, CopyString);
      }
    }
  }
  return false;
}

Array f_localeconv() {
  struct lconv currlocdata;
  {
    Lock lock(s_mutex);
    struct lconv *res = localeconv();
    currlocdata = *res;
  }

  Array ret;
#define SET_LOCALE_STRING(x)  ret.set(#x, String(currlocdata.x, CopyString))
  SET_LOCALE_STRING(decimal_point);
  SET_LOCALE_STRING(thousands_sep);
  SET_LOCALE_STRING(int_curr_symbol);
  SET_LOCALE_STRING(currency_symbol);
  SET_LOCALE_STRING(mon_decimal_point);
  SET_LOCALE_STRING(mon_thousands_sep);
  SET_LOCALE_STRING(positive_sign);
  SET_LOCALE_STRING(negative_sign);
#define SET_LOCALE_INTEGER(x) ret.set(#x, currlocdata.x)
  SET_LOCALE_INTEGER(int_frac_digits);
  SET_LOCALE_INTEGER(frac_digits);
  SET_LOCALE_INTEGER(p_cs_precedes);
  SET_LOCALE_INTEGER(p_sep_by_space);
  SET_LOCALE_INTEGER(n_cs_precedes);
  SET_LOCALE_INTEGER(n_sep_by_space);
  SET_LOCALE_INTEGER(p_sign_posn);
  SET_LOCALE_INTEGER(n_sign_posn);

  Array grouping, mon_grouping;

  /* Grab the grouping data out of the array */
  int len = strlen(currlocdata.grouping);
  for (int i = 0; i < len; i++) {
    grouping.set(i, currlocdata.grouping[i]);
  }
  ret.set("grouping", grouping);

  /* Grab the monetary grouping data out of the array */
  len = strlen(currlocdata.mon_grouping);
  for (int i = 0; i < len; i++) {
    mon_grouping.set(i, currlocdata.mon_grouping[i]);
  }
  ret.set("mon_grouping", mon_grouping);

  return ret;
}

String f_convert_cyr_string(CStrRef str, CStrRef from, CStrRef to) {
  char ch_from = ((const char *)from)[0];
  char ch_to = ((const char *)to)[0];
  char *ret = string_convert_cyrillic_string(str.data(), str.size(),
                                             ch_from, ch_to);
  return String(ret, str.size(), AttachString);
}

String f_hebrev(CStrRef hebrew_text, int max_chars_per_line /* = 0 */) {
  if (hebrew_text.empty()) return hebrew_text;
  int len = hebrew_text.size();
  char *ret = string_convert_hebrew_string(hebrew_text.data(), len,
                                           max_chars_per_line, false);
  return String(ret, len, AttachString);
}

String f_hebrevc(CStrRef hebrew_text, int max_chars_per_line /* = 0 */) {
  if (hebrew_text.empty()) return hebrew_text;
  int len = hebrew_text.size();
  char *ret = string_convert_hebrew_string(hebrew_text.data(), len,
                                           max_chars_per_line, true);
  return String(ret, len, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
}
