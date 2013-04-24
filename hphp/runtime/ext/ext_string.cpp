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

#include <runtime/ext/ext_string.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_url.h>
#include <runtime/base/zend/zend_printf.h>
#include <runtime/base/zend/zend_scanf.h>
#include <runtime/base/bstring.h>
#include <runtime/base/util/request_local.h>
#include <util/lock.h>
#include <locale.h>
#include <runtime/base/server/http_request_handler.h>
#include <runtime/base/server/http_protocol.h>

namespace HPHP {

static Mutex s_mutex;
///////////////////////////////////////////////////////////////////////////////

String f_addcslashes(CStrRef str, CStrRef charlist) {
  return StringUtil::CEncode(str, charlist);
}
String f_stripcslashes(CStrRef str) {
  return StringUtil::CDecode(str);
}
String f_addslashes(CStrRef str) {
  return StringUtil::SqlEncode(str);
}
String f_stripslashes(CStrRef str) {
  return StringUtil::SqlDecode(str);
}
String f_bin2hex(CStrRef str) {
  return StringUtil::HexEncode(str);
}
Variant f_hex2bin(CStrRef str) {
  try {
    return StringUtil::HexDecode(str);
  } catch (...) {
    raise_warning("hex2bin: malformed input");
    return false;
  }
}
String f_nl2br(CStrRef str) {
  return str.replace("\n", "<br />\n");
}
String f_quotemeta(CStrRef str) {
  return StringUtil::RegExEncode(str);
}
String f_str_shuffle(CStrRef str) {
  return StringUtil::Shuffle(str);
}
String f_strrev(CStrRef str) {
  return StringUtil::Reverse(str);
}
String f_strtolower(CStrRef str) {
  return StringUtil::ToLower(str);
}
String f_strtoupper(CStrRef str) {
  return StringUtil::ToUpper(str, StringUtil::ToUpperAll);
}
String f_ucfirst(CStrRef str) {
  return StringUtil::ToUpper(str, StringUtil::ToUpperFirst);
}
String f_lcfirst(CStrRef str) {
  return StringUtil::ToLower(str, StringUtil::ToLowerFirst);
}
String f_ucwords(CStrRef str) {
  return StringUtil::ToUpper(str, StringUtil::ToUpperWords);
}
String f_strip_tags(CStrRef str, CStrRef allowable_tags /* = "" */) {
  return StringUtil::StripHTMLTags(str, allowable_tags);
}
String f_trim(CStrRef str, CStrRef charlist /* = k_HPHP_TRIM_CHARLIST */) {
  return StringUtil::Trim(str, StringUtil::TrimBoth, charlist);
}
String f_ltrim(CStrRef str, CStrRef charlist /* = k_HPHP_TRIM_CHARLIST */) {
  return StringUtil::Trim(str, StringUtil::TrimLeft, charlist);
}
String f_rtrim(CStrRef str, CStrRef charlist /* = k_HPHP_TRIM_CHARLIST */) {
  return StringUtil::Trim(str, StringUtil::TrimRight, charlist);
}
String f_chop(CStrRef str, CStrRef charlist /* = k_HPHP_TRIM_CHARLIST */) {
  return StringUtil::Trim(str, StringUtil::TrimRight, charlist);
}
Variant f_explode(CStrRef delimiter, CStrRef str, int limit /* = 0x7FFFFFFF */) {
  return StringUtil::Explode(str, delimiter, limit);
}

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

String f_join(CVarRef glue, CVarRef pieces /* = null_variant */) {
  return f_implode(glue, pieces);
}
Variant f_str_split(CStrRef str, int split_length /* = 1 */) {
  return StringUtil::Split(str, split_length);
}
Variant f_chunk_split(CStrRef body, int chunklen /* = 76 */,
                      CStrRef end /* = "\r\n" */) {
  return StringUtil::ChunkSplit(body, chunklen, end);
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
                      VRefParam count /* = null */) {
  int nCount = 0;
  Variant ret = str_replace(search, replace, subject, nCount, true);
  count = nCount;
  return ret;
}

Variant f_str_ireplace(CVarRef search, CVarRef replace, CVarRef subject,
                       VRefParam count /* = null */) {
  int nCount = 0;
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

Variant f_substr(CStrRef str, int start, int length /* = 0x7FFFFFFF */) {
  String ret = str.substr(start, length, true);
  if (ret.isNull()) return false;
  return ret;
}
String f_str_pad(CStrRef input, int pad_length, CStrRef pad_string /* = " " */,
                        int pad_type /* = k_STR_PAD_RIGHT */) {
  return StringUtil::Pad(input, pad_length, pad_string,
                         (StringUtil::PadType)pad_type);
}
String f_str_repeat(CStrRef input, int multiplier) {
  return StringUtil::Repeat(input, multiplier);
}
Variant f_wordwrap(CStrRef str, int width /* = 75 */, CStrRef wordbreak /* = "\n" */,
                   bool cut /* = false */) {
  String ret = StringUtil::WordWrap(str, width, wordbreak, cut);
  if (ret.isNull()) return false;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

Variant f_printf(int _argc, CStrRef format, CArrRef _argv /* = null_array */) {
  int len = 0; char *output = string_printf(format.data(), format.size(),
                                            _argv, &len);
  if (output == NULL) return false;
  echo(output); free(output);
  return len;
}
Variant f_vprintf(CStrRef format, CArrRef args) {
  int len = 0; char *output = string_printf(format.data(), format.size(),
                                            args, &len);
  if (output == NULL) return false;
  echo(output); free(output);
  return len;
}
Variant f_sprintf(int _argc, CStrRef format, CArrRef _argv /* = null_array */) {
  char *output = string_printf(format.data(), format.size(), _argv, NULL);
  if (output == NULL) return false;
  return String(output, AttachString);
}
Variant f_vsprintf(CStrRef format, CArrRef args) {
  char *output = string_printf(format.data(), format.size(), args, NULL);
  if (output == NULL) return false;
  return String(output, AttachString);
}

Variant f_sscanf(int _argc, CStrRef str, CStrRef format, CArrRef _argv /* = null_array */) {
  Variant ret;
  int result;
  result = string_sscanf(str.c_str(), format.c_str(), _argv.size(), ret);
  if (SCAN_ERROR_WRONG_PARAM_COUNT == result) return uninit_null();
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

String f_chr(int64_t ascii) {
  char buf[2]; buf[0] = ascii; buf[1] = 0;
  return String(buf, 1, CopyString);
}

int64_t f_ord(CStrRef str) {
  return (int64_t)(unsigned char)str[0];
}

Variant f_money_format(CStrRef format, double number) {
  String s = StringUtil::MoneyFormat(format.c_str(), number);
  if (s.isNull()) return false;
  return s;
}

String f_number_format(double number, int decimals /* = 0 */,
                       CStrRef dec_point /* = "." */,
                       CStrRef thousands_sep /* = "," */) {
  char ch_dec_point = '.';
  if (!dec_point.isNull()) {
    if (dec_point.size() >= 1) {
      ch_dec_point = dec_point[0];
    } else {
      ch_dec_point = 0;
    }
  }
  char ch_thousands_sep = ',';
  if (!thousands_sep.isNull()) {
    if (thousands_sep.size() >= 1) {
      ch_thousands_sep = thousands_sep[0];
    } else {
      ch_thousands_sep = 0;
    }
  }
  char *ret = string_number_format(number, decimals, ch_dec_point,
                                   ch_thousands_sep);
  return String(ret, AttachString);
}

int64_t f_strcmp(CStrRef str1, CStrRef str2) {
  return string_strcmp(str1.data(), str1.size(), str2.data(), str2.size());
}
Variant f_strncmp(CStrRef str1, CStrRef str2, int len) {
  if (len < 0) {
    raise_warning("Length must be greater than or equal to 0");
    return false;
  }
  return string_strncmp(str1.data(), str1.size(), str2.data(), str2.size(),
                        len);
}
int64_t f_strnatcmp(CStrRef str1, CStrRef str2) {
  return string_natural_cmp(str1.data(), str1.size(), str2.data(), str2.size(),
                            false);
}

int64_t f_strcasecmp(CStrRef str1, CStrRef str2) {
  return bstrcasecmp(str1.data(), str1.size(), str2.data(), str2.size());
}

Variant f_strncasecmp(CStrRef str1, CStrRef str2, int len) {
  if (len < 0) {
    raise_warning("Length must be greater than or equal to 0");
    return false;
  }
  return string_strncasecmp(str1.data(), str1.size(), str2.data(), str2.size(),
                            len);
}

int64_t f_strnatcasecmp(CStrRef str1, CStrRef str2) {
  return string_natural_cmp(str1.data(), str1.size(), str2.data(), str2.size(),
                            true);
}

int64_t f_strcoll(CStrRef str1, CStrRef str2) {
  return strcoll(str1.c_str(), str2.c_str());
}

Variant f_substr_compare(CStrRef main_str, CStrRef str, int offset,
                         int length /* = INT_MAX */,
                         bool case_insensitivity /* = false */) {
  int s1_len = main_str.size();
  int s2_len = str.size();

  if (offset < 0) {
    offset = s1_len + offset;
    if (offset < 0) offset = 0;
  }
  if (offset >= s1_len || length <= 0) {
    return false;
  }

  int cmp_len = s1_len - offset;
  if (cmp_len < s2_len) cmp_len = s2_len;
  if (cmp_len > length) cmp_len = length;

  const char *s1 = main_str.data();
  if (case_insensitivity) {
    return bstrcasecmp(s1 + offset, cmp_len, str.data(), cmp_len);
  }
  return string_ncmp(s1 + offset, str.data(), cmp_len);
}

Variant f_strrchr(CStrRef haystack, CVarRef needle) {
  Variant ret = f_strrpos(haystack, needle);
  if (same(ret, false)) {
    return false;
  }
  return haystack.substr(ret.toInt32());
}

Variant f_strstr(CStrRef haystack, CVarRef needle,
                 bool before_needle /* = false */) {
  Variant ret = f_strpos(haystack, needle);
  if (same(ret, false)) {
    return false;
  }
  if (before_needle) {
    return haystack.substr(0, ret.toInt32());
  } else {
    return haystack.substr(ret.toInt32());
  }
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
  const char *p = strpbrk(haystack.c_str(), char_list.c_str());
  if (p) {
    return String(p, CopyString);
  }
  return false;
}

Variant f_strchr(CStrRef haystack, CVarRef needle) {
  return f_strstr(haystack, needle);
}

Variant f_strpos(CStrRef haystack, CVarRef needle, int offset /* = 0 */) {
  if (offset < 0 || offset > haystack.size()) {
    raise_warning("Offset not contained in string");
    return false;
  }
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
  if (offset < 0 || offset > haystack.size()) {
    raise_warning("Offset not contained in string");
    return false;
  }
  int pos;
  if (needle.isString()) {
    pos = haystack.find(needle.toString(), offset, false);
  } else {
    pos = haystack.find(needle.toByte(), offset, false);
  }
  if (pos >= 0) return pos;
  return false;
}

Variant f_strrpos(CStrRef haystack, CVarRef needle, int offset /* = 0 */) {
  if (offset < -haystack.size() || offset > haystack.size()) {
    raise_warning("Offset is greater than the length of haystack string");
    return false;
  }
  int pos;
  if (needle.isString()) {
    pos = haystack.rfind(needle.toString(), offset);
  } else {
    pos = haystack.rfind(needle.toByte(), offset);
  }
  if (pos >= 0) return pos;
  return false;
}

Variant f_strripos(CStrRef haystack, CVarRef needle, int offset /* = 0 */) {
  if (offset < -haystack.size() || offset > haystack.size()) {
    raise_warning("Offset is greater than the length of haystack string");
    return false;
  }
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
  const char *s1 = str1.data();
  const char *s2 = str2.data();
  int s1_len = str1.size();
  int s2_len = str2.size();

  if (!string_substr_check(s1_len, start, length)) {
    return false;
  }

  s1 += start;
  for (int pos = 0; pos < length; ++pos) {
    if (memchr(s2, *(s1++), s2_len) == NULL) return pos;
  }

  return length;
}

Variant f_strcspn(CStrRef str1, CStrRef str2, int start /* = 0 */,
                  int length /* = 0x7FFFFFFF */) {
  const char *s1 = str1.data();
  const char *s2 = str2.data();
  int s1_len = str1.size();
  int s2_len = str2.size();

  if (!string_substr_check(s1_len, start, length)) {
    return false;
  }

  s1 += start;
  for (int pos = 0; pos < length; ++pos) {
    if (memchr(s2, *(s1++), s2_len) != NULL) return pos;
  }

  return length;
}

Variant f_strlen(CVarRef vstr) {
  Variant::TypedValueAccessor tva = vstr.getTypedAccessor();
  switch (Variant::GetAccessorType(tva)) {
  case KindOfString:
  case KindOfStaticString:
    return Variant(Variant::GetStringData(tva)->size());
  case KindOfArray:
    raise_warning("strlen() expects parameter 1 to be string, array given");
    return uninit_null();
  case KindOfObject:
    if (!f_method_exists(vstr, "__toString")) {
      raise_warning("strlen() expects parameter 1 to be string, object given");
      return uninit_null();
    } //else fallback to default
  default:
    CStrRef str = vstr.toString();
    return Variant(str.size());
  }
}

Variant f_count_chars(CStrRef str, int64_t mode /* = 0 */) {
  int chars[256];
  memset((void*)chars, 0, sizeof(chars));
  const unsigned char *buf = (const unsigned char *)str.data();
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
Variant f_str_word_count(CStrRef str, int64_t format /* = 0 */,
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
  const char *char_list = charlist.data();
  if (*char_list) {
    string_charmask(char_list, charlist.size(), ch);
  } else {
    char_list = NULL;
  }

  int word_count = 0;
  const char *s0 = str.data();
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
  return ret.isNull() ? Array::Create() : ret;
}

int64_t f_levenshtein(CStrRef str1, CStrRef str2, int cost_ins /* = 1 */,
                         int cost_rep /* = 1 */, int cost_del /* = 1 */) {
  return string_levenshtein(str1.data(), str1.size(), str2.data(), str2.size(),
                            cost_ins, cost_rep, cost_del);
}

int64_t f_similar_text(CStrRef first, CStrRef second,
                       VRefParam percent /* = uninit_null() */) {
  float p;
  int ret = string_similar_text(first.data(), first.size(),
                                second.data(), second.size(), &p);
  percent = p;
  return ret;
}

Variant f_soundex(CStrRef str) {
  if (str.empty()) return false;
  return String(string_soundex(str.c_str()), AttachString);
}

Variant f_metaphone(CStrRef str, int phones /* = 0 */) {
  char *ret = string_metaphone(str.data(), str.size(), 0, 1);
  if (ret) {
    return String(ret, AttachString);
  }
  return false;
}

String f_html_entity_decode(CStrRef str, int quote_style /* = k_ENT_COMPAT */,
                            CStrRef charset /* = "ISO-8859-1" */) {
  const char *scharset = charset.data();
  if (!*scharset) scharset = "UTF-8";
  return StringUtil::HtmlDecode(str, (StringUtil::QuoteStyle)quote_style,
                                scharset, true);
}

String f_htmlentities(CStrRef str, int quote_style /* = k_ENT_COMPAT */,
                      CStrRef charset /* = "ISO-8859-1" */,
                      bool double_encode /* = true */) {
  // dropping double_encode parameters and see runtime/base/zend_html.h
  const char *scharset = charset.data();
  if (!*scharset) scharset = "UTF-8";
  return StringUtil::HtmlEncode(str, (StringUtil::QuoteStyle)quote_style,
                                scharset, true);
}
String f_htmlspecialchars_decode(CStrRef str,
                                 int quote_style /* = k_ENT_COMPAT */) {
  return StringUtil::HtmlDecode(str, (StringUtil::QuoteStyle)quote_style,
                                "UTF-8", false);
}
String f_htmlspecialchars(CStrRef str, int quote_style /* = k_ENT_COMPAT */,
                          CStrRef charset /* = "ISO-8859-1" */,
                          bool double_encode /* = true */) {
  // dropping double_encode parameters and see runtime/base/zend_html.h
  const char *scharset = charset.data();
  if (!*scharset) scharset = "UTF-8";
  return StringUtil::HtmlEncode(str, (StringUtil::QuoteStyle)quote_style,
                                scharset, false);
}
String f_fb_htmlspecialchars(CStrRef str, int quote_style /* = k_ENT_COMPAT */,
                             CStrRef charset /* = "ISO-8859-1" */,
                             CArrRef extra /* = Array() */) {
  return StringUtil::HtmlEncodeExtra(str, (StringUtil::QuoteStyle)quote_style,
                                     charset.data(), false, extra);
}
String f_quoted_printable_encode(CStrRef str) {
  return StringUtil::QuotedPrintableEncode(str);
}
String f_quoted_printable_decode(CStrRef str) {
  return StringUtil::QuotedPrintableDecode(str);
}
Variant f_convert_uudecode(CStrRef data) {
  String ret = StringUtil::UUDecode(data);
  if (ret.isNull()) {
    return false; // bad format
  }
  return ret;
}
Variant f_convert_uuencode(CStrRef data) {
  if (data.empty()) return false;
  return StringUtil::UUEncode(data);
}
String f_str_rot13(CStrRef str) {
  return StringUtil::ROT13(str);
}
int64_t f_crc32(CStrRef str) {
  return (uint32_t)StringUtil::CRC32(str);
}
String f_crypt(CStrRef str, CStrRef salt /* = "" */) {
  return StringUtil::Crypt(str, salt.c_str());
}
String f_md5(CStrRef str, bool raw_output /* = false */) {
  return StringUtil::MD5(str, raw_output);
}
String f_sha1(CStrRef str, bool raw_output /* = false */) {
  return StringUtil::SHA1(str, raw_output);
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

  if (arr.empty()) {
    // Nothing to translate
    return str;
  }

  for (ArrayIter iter(arr); iter; ++iter) {
    String search = iter.first();
    int len = search.size();
    if (len < 1) return false;
    if (maxlen < len) maxlen = len;
    if (minlen == -1 || minlen > len) minlen = len;
  }

  const char *s = str.data();
  int slen = str.size();
  String key(maxlen, ReserveString);

  StringBuffer result(slen);
  for (int pos = 0; pos < slen; ) {
    if ((pos + maxlen) > slen) {
      maxlen = slen - pos;
    }
    bool found = false;
    memcpy(key.mutableSlice().ptr, s + pos, maxlen);
    for (int len = maxlen; len >= minlen; len--) {
      key.setSize(len);
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
  return result.detach();
}

void f_parse_str(CStrRef str, VRefParam arr /* = null */) {
  arr = Array::Create();
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

    const char *loc = slocale.c_str();
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

static const StaticString s_decimal_point("decimal_point");
static const StaticString s_thousands_sep("thousands_sep");
static const StaticString s_int_curr_symbol("int_curr_symbol");
static const StaticString s_currency_symbol("currency_symbol");
static const StaticString s_mon_decimal_point("mon_decimal_point");
static const StaticString s_mon_thousands_sep("mon_thousands_sep");
static const StaticString s_positive_sign("positive_sign");
static const StaticString s_negative_sign("negative_sign");
static const StaticString s_int_frac_digits("int_frac_digits");
static const StaticString s_frac_digits("frac_digits");
static const StaticString s_p_cs_precedes("p_cs_precedes");
static const StaticString s_p_sep_by_space("p_sep_by_space");
static const StaticString s_n_cs_precedes("n_cs_precedes");
static const StaticString s_n_sep_by_space("n_sep_by_space");
static const StaticString s_p_sign_posn("p_sign_posn");
static const StaticString s_n_sign_posn("n_sign_posn");
static const StaticString s_grouping("grouping");
static const StaticString s_mon_grouping("mon_grouping");

Array f_localeconv() {
  struct lconv currlocdata;
  {
    Lock lock(s_mutex);
    struct lconv *res = localeconv();
    currlocdata = *res;
  }

  Array ret;
#define SET_LOCALE_STRING(x) ret.set(s_ ## x, String(currlocdata.x, CopyString))
  SET_LOCALE_STRING(decimal_point);
  SET_LOCALE_STRING(thousands_sep);
  SET_LOCALE_STRING(int_curr_symbol);
  SET_LOCALE_STRING(currency_symbol);
  SET_LOCALE_STRING(mon_decimal_point);
  SET_LOCALE_STRING(mon_thousands_sep);
  SET_LOCALE_STRING(positive_sign);
  SET_LOCALE_STRING(negative_sign);
#define SET_LOCALE_INTEGER(x) ret.set(s_ ## x, currlocdata.x)
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
  ret.set(s_grouping, grouping);

  /* Grab the monetary grouping data out of the array */
  len = strlen(currlocdata.mon_grouping);
  for (int i = 0; i < len; i++) {
    mon_grouping.set(i, currlocdata.mon_grouping[i]);
  }
  ret.set(s_mon_grouping, mon_grouping);

  return ret;
}

String f_nl_langinfo(int item) {
  return nl_langinfo(item);
}

String f_convert_cyr_string(CStrRef str, CStrRef from, CStrRef to) {
  char ch_from = from[0];
  char ch_to = to[0];
  char *ret = string_convert_cyrillic_string(str.data(), str.size(),
                                             ch_from, ch_to);
  return String(ret, str.size(), AttachString);
}

#define ENT_HTML_QUOTE_NONE     0
#define ENT_HTML_QUOTE_SINGLE   1
#define ENT_HTML_QUOTE_DOUBLE   2

#define ENT_COMPAT    ENT_HTML_QUOTE_DOUBLE
#define ENT_QUOTES    (ENT_HTML_QUOTE_DOUBLE | ENT_HTML_QUOTE_SINGLE)
#define ENT_NOQUOTES  ENT_HTML_QUOTE_NONE

static const HtmlBasicEntity basic_entities[] = {
  { '"',  "&quot;",   6,  ENT_HTML_QUOTE_DOUBLE },
  { '\'', "&#039;",   6,  ENT_HTML_QUOTE_SINGLE },
  { '\'', "&#39;",    5,  ENT_HTML_QUOTE_SINGLE },
  { '<',  "&lt;",     4,  0 },
  { '>',  "&gt;",     4,  0 },
  { 0, NULL, 0, 0 }
};

static const StaticString s_amp("&");
static const StaticString s_ampsemi("&amp;");

Array f_get_html_translation_table(int table /* = 0 */, int quote_style /* = k_ENT_COMPAT */) {
  static entity_charset charset = determine_charset(nullptr); // get default one
  char ind[2]; ind[1] = 0;

  assert(charset != entity_charset_enum::cs_unknown);

  const int HTML_SPECIALCHARS = 0;
  const int HTML_ENTITIES = 1;

  using namespace entity_charset_enum;

  Array ret;
  switch (table) {
  case HTML_ENTITIES: {
    auto entity_map = html_get_entity_map();

    for (int j = 0; entity_map[j].charset != cs_terminator; j++) {
      const html_entity_map &em = entity_map[j];
      if (em.charset != charset)
        continue;

      for (int i = 0; i <= em.endchar - em.basechar; i++) {
        char buffer[16];

        if (em.table[i] == NULL)
          continue;
        /* what about wide chars here ?? */
        ind[0] = i + em.basechar;
        snprintf(buffer, sizeof(buffer), "&%s;", em.table[i]);
        ret.set(ind, String(buffer, CopyString));
      }
    }
    /* fall thru */
  }
  case HTML_SPECIALCHARS:
    for (int j = 0; basic_entities[j].charcode != 0; j++) {
      if (basic_entities[j].flags &&
          (quote_style & basic_entities[j].flags) == 0)
        continue;

      ind[0] = (unsigned char)basic_entities[j].charcode;
      ret.set(String(ind, 2, CopyString), basic_entities[j].entity);
    }
    ret.set(s_amp, s_ampsemi);
    break;
  }

  return ret;
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
