/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

// There are apparently two versions of toupper/tolower: the C version in
// cctype.h and the C++ version in cctype. Make sure we include this C header
// first to get the C version, which at least with glibc has much better
// behaviour in a tight loop due to inlining; this makes strtoupper/strtolower
// several times faster. See https://github.com/facebook/hhvm/issues/7133
#include <ctype.h>

#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/util/bstring.h"
#include "hphp/runtime/ext/hash/hash_murmur.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/actrec-args.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/zend-scanf.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/ext/std/ext_std_classobj.h"
#include "hphp/runtime/ext/std/ext_std_math.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/util/lock.h"
#include "hphp/util/concurrent-lru-cache.h"
#include "hphp/zend/html-table.h"

#include <folly/Unicode.h>
#include <bitset>
#include <locale.h>

namespace HPHP {

static Mutex s_mutex;

const StaticString
  s_HPHP_TRIM_CHARLIST("HPHP_TRIM_CHARLIST"),
  k_HPHP_TRIM_CHARLIST("\n\r\t\x0b\x00 ", 6);

///////////////////////////////////////////////////////////////////////////////

template <class Op> ALWAYS_INLINE
String stringForEachBuffered(uint32_t bufLen, const String& str, Op action) {
  StringBuffer sb(bufLen);
  auto sl = str.slice();
  const char* src = sl.begin();
  const char* end = sl.end();

  for (; src < end; ++src) {
    action(sb, src, end);
  }

  return sb.detach();
}

template <bool mutate, class Op> ALWAYS_INLINE
String stringForEach(uint32_t len, const String& str, Op action) {
  String ret = mutate ? str : String(len, ReserveString);

  auto srcSlice = str.slice();

  const char* src = srcSlice.begin();
  const char* end = srcSlice.end();

  char* dst = ret.mutableData();

  for (; src != end; ++src, ++dst) {
    *dst = action(*src);
  }

  if (!mutate) ret.setSize(len);
  return ret;
}

template <class Op> ALWAYS_INLINE
String stringForEachFast(const String& str, Op action) {
  if (str.empty()) {
    return str;
  }

  return stringForEach<false>(str.size(), str, action);
}

String HHVM_FUNCTION(addcslashes,
                     const String& str,
                     const String& charlist) {
  if (str.empty() || (!charlist.isNull() && charlist.empty())) {
    return str;
  }

  int masklen = charlist.isNull() ? 10 : charlist.size();
  const char* list = charlist.isNull() ? "\\\x00\x01..\x1f\x7f..\xff"
                                       : charlist.c_str();

  char flags[256];
  string_charmask(list, masklen, flags);

  return stringForEachBuffered(str.size(), str,
    [&] (StringBuffer& ret, const char* src, const char* end) {
      int c = (unsigned char)*src;

      if (flags[c]) {
        ret.append('\\');
        if ((c < 32) || (c > 126)) {
          switch (c) {
            case '\n': ret.append('n'); break;
            case '\t': ret.append('t'); break;
            case '\r': ret.append('r'); break;
            case '\a': ret.append('a'); break;
            case '\v': ret.append('v'); break;
            case '\b': ret.append('b'); break;
            case '\f': ret.append('f'); break;
            default: ret.append((char)('0' + (c / 64))); c %= 64;
                     ret.append((char)('0' + (c /  8))); c %=  8;
                     ret.append((char)('0' + c));
          }
          return;
        }
      }
      ret.append((char)c);
    });
}

String HHVM_FUNCTION(stripcslashes,
                     const String& str) {
  if (str.empty()) {
    return str;
  }

  return stringForEachBuffered(str.size(), str,
    [&] (StringBuffer& ret, const char*& src, const char* end) {
      char c;
      const char* p;

      if (*src != '\\' || src + 1 == end) {
        ret.append(*src);
        return;
      }

      switch (*++src) {
        case 'n': ret.append('\n'); break;
        case 'r': ret.append('\r'); break;
        case 'a': ret.append('\a'); break;
        case 't': ret.append('\t'); break;
        case 'v': ret.append('\v'); break;
        case 'b': ret.append('\b'); break;
        case 'f': ret.append('\f'); break;
        case '\\': ret.append('\\'); break;
        case 'x':
          if (src + 1 == end || !isxdigit(src[1])) {
            ret.append(*src);
            break;
          }

          for (c = 0, ++src, p = src + 2; src < p && src < end &&
               isxdigit(*src); ++src) {
            c *= 16;
            c += (*src < 'A' ? *src - '0' :
                  *src < 'a' ? *src - 'A' + 10 :
                               *src - 'a' + 10);
          }
          ret.append(c);
          src--;
          break;

        default:
          if (*src < '0' || '7' < *src) {
            // The character after the slash is nothing special. Append it
            // unchanged to the result.
            ret.append(*src);
            break;
          }

          // Decode a base 8 number up to 3 characters long.
          for (c = 0, p = src + 3; src < p && src < end && '0' <= *src &&
               *src < '8'; ++src) {
            c *= 8;
            c += (*src - '0');
          }
          ret.append(c);
          src--;
      }
    });
}

String HHVM_FUNCTION(addslashes,
                     const String& str) {
  if (str.empty()) {
    return str;
  }

  return stringForEachBuffered(str.size(), str,
    [&] (StringBuffer& ret, const char* src, const char* end) {
      switch (*src) {
        case '\0':
          ret.append('\\');
          ret.append('0');
          break;
        case '\\': case '\"': case '\'':
          ret.append('\\');
          /* fall through */
        default:
          ret.append(*src);
      }
    });
}

String HHVM_FUNCTION(stripslashes,
                     const String& str) {
  if (str.empty()) {
    return str;
  }

  return stringForEachBuffered(str.size(), str,
    [&] (StringBuffer& ret, const char*& src, const char* end) {
      if (*src == '\\' && *++src == '0') {
        ret.append('\0');
        return;
      }
      if (src < end) {
        ret.append(*src);
      }
    });
}

String HHVM_FUNCTION(bin2hex,
                     const String& str) {
  if (str.empty()) {
    return str;
  }

  return stringForEachBuffered(str.size(), str,
    [&] (StringBuffer& ret, const char* src, const char* end) {
      static char hexconvtab[] = "0123456789abcdef";
      ret.append(hexconvtab[(unsigned char)*src >> 4]);
      ret.append(hexconvtab[(unsigned char)*src & 15]);
    });
}

Variant HHVM_FUNCTION(hex2bin,
                      const String& str) {
  if (str.empty()) {
    return str;
  }

  if (str.size() % 2) {
    raise_warning("hex2bin: malformed input");
    return false;
  }

  StringBuffer ret(str.size() / 2 + 1);

  auto sl = str.slice();
  const char* src = sl.begin();
  const char* end = sl.end();

  for (; src != end; ++src) {
    int val;
    if (isdigit(*src))                   val = 16 * (*src++ - '0');
    else if ('a' <= *src && *src <= 'f') val = 16 * (*src++ - 'a' + 10);
    else if ('A' <= *src && *src <= 'F') val = 16 * (*src++ - 'A' + 10);
    else {
      raise_warning("hex2bin: malformed input");
      return false;
    }

    if (isdigit(*src))                   val += (*src - '0');
    else if ('a' <= *src && *src <= 'f') val += (*src - 'a' + 10);
    else if ('A' <= *src && *src <= 'F') val += (*src - 'A' + 10);
    else {
      raise_warning("hex2bin: malformed input");
      return false;
    }

    ret.append((char)val);
  }

  return ret.detach();
}

const StaticString
  s_br("<br />"),
  s_non_xhtml_br("<br>");

String HHVM_FUNCTION(nl2br,
                     const String& str,
                     bool is_xhtml /* = true */) {
  if (str.empty()) {
    return str;
  }
  String htmlType;

  if (is_xhtml) {
    htmlType = s_br;
  } else {
    htmlType = s_non_xhtml_br;
  }

  return stringForEachBuffered(str.size(), str,
    [&] (StringBuffer& ret, const char*& src, const char* end) {
      // PHP treats a carriage return beside a newline as the same break
      // no matter what order they're in.  Don't do it for two of the same in
      // a row, though...
      switch (*src) {
      case '\n':
        ret.append(htmlType);
        // skip next if carriage return
        if (*(src + 1) == '\r') {
          ret.append(*src);
          ++src;
        }
        ret.append(*src);
        break;
      case '\r':
        ret.append(htmlType);
        // skip next if newline
        if (*(src + 1) == '\n') {
          ret.append(*src);
          ++src;
        }
        /* fall through */
      default:
        ret.append(*src);
      }
    });
}

String HHVM_FUNCTION(quotemeta,
                     const String& str) {
  if (str.empty()) {
    return str;
  }

  return stringForEachBuffered(str.size(), str,
    [&] (StringBuffer& ret, const char* src, const char* end) {
      switch (*src) {
        case '.': case '\\': case '+': case '*': case '?': case '[': case ']':
        case '^': case '$': case '(': case ')':
          ret.append('\\');
          /* fall through */
        default:
          ret.append(*src);
      }
    });
}

String HHVM_FUNCTION(str_shuffle,
                     const String& str) {
  if (str.size() <= 1) {
    return str;
  }

  String ret(str, CopyString);
  char* buf  = ret.get()->mutableData();
  int left   = ret.size();

  while (--left) {
    int idx = HHVM_FN(rand)(0, left);
    if (idx != left) {
      char temp = buf[left];
      buf[left] = buf[idx];
      buf[idx] = temp;
    }
  }
  return ret;
}

String HHVM_FUNCTION(strrev,
                     const String& str) {
  auto len = str.size();

  String ret(len, ReserveString);

  const char* data = str.data();
  char* dest = ret.get()->mutableData();

  for (int i = 0; i < len; ++i) {
    dest[i] = data[len - i - 1];
  }

  ret.setSize(len);
  return ret;
}

String HHVM_FUNCTION(strtolower,
                     const String& str) {
  return stringForEachFast(str, tolower);
}

String HHVM_FUNCTION(strtoupper,
                     const String& str) {
  return stringForEachFast(str, toupper);
}

template <class OpTo, class OpIs> ALWAYS_INLINE
String stringToCaseFirst(const String& str, OpTo tocase, OpIs iscase) {
  if (str.empty() || iscase(str[0])) {
    return str;
  }

  String ret(str, CopyString);
  char* first = ret.get()->mutableData();

  *first = tocase(*first);
  return ret;
}

String HHVM_FUNCTION(ucfirst,
                     const String& str) {
  return stringToCaseFirst(str, toupper, isupper);
}

String HHVM_FUNCTION(lcfirst,
                     const String& str) {
  return stringToCaseFirst(str, tolower, islower);
}

String HHVM_FUNCTION(ucwords,
                     const String& str,
                     const String& delimiters /* = " \t\r\n\f\v"*/) {
  if (str.empty()) {
    return str;
  }

  std::bitset<256> delimiters_set;
  int delimiters_len = delimiters.length();
  for (int i = 0; i < delimiters_len; i++) {
    delimiters_set.set(delimiters[i]);
  }

  String strcopy(str, CopyString);
  char* string = strcopy.mutableData();
  *string = toupper(*string);
  char last = ' ';

  return stringForEach<true>(strcopy.size(), strcopy, [&] (char c) {
    char ret = delimiters_set.test(last) ? toupper(c) : c;
    last = c;
    return ret;
  });
}

String HHVM_FUNCTION(strip_tags,
                     const String& str,
                     const Variant& allowable_tags /* = "" */) {
  return StringUtil::StripHTMLTags(str, allowable_tags.toString());
}

template <bool left, bool right> ALWAYS_INLINE
String stringTrim(const String& str, const String& charlist) {
  char flags[256];
  string_charmask(charlist.c_str(), charlist.size(), flags);

  auto len = str.size();
  int start = 0, end = len - 1;

  if (left) {
    for (; start < len && flags[(unsigned char)str[start]]; ++start)
      /* do nothing */;
  }

  if (right) {
    for (; end >= start && flags[(unsigned char)str[end]]; --end) {}
  }

  return str.substr(start, end - start + 1);
}

String HHVM_FUNCTION(trim,
                     const String& str,
                     const String& charlist /* = k_HPHP_TRIM_CHARLIST */) {
  return stringTrim<true,true>(str, charlist);
}

String HHVM_FUNCTION(ltrim,
                     const String& str,
                     const String& charlist /* = k_HPHP_TRIM_CHARLIST */) {
  return stringTrim<true,false>(str, charlist);
}

String HHVM_FUNCTION(rtrim,
                     const String& str,
                     const String& charlist /* = k_HPHP_TRIM_CHARLIST */) {
  return stringTrim<false,true>(str, charlist);
}

String HHVM_FUNCTION(chop,
                      const String& str,
                      const String& charlist /* = k_HPHP_TRIM_CHARLIST */) {
  return stringTrim<false,true>(str, charlist);
}

Variant HHVM_FUNCTION(explode,
                      const String& delimiter,
                      const String& str,
                      int limit /* = 0x7FFFFFFF */) {
  return StringUtil::Explode(str, delimiter, limit);
}

String HHVM_FUNCTION(implode,
                     const Variant& arg1,
                     const Variant& arg2 /* = null_variant */) {
  Array items;
  String delim;
  if (isContainer(arg1)) {
    items = arg1;
    delim = arg2.toString();
  } else if (isContainer(arg2)) {
    items = arg2;
    delim = arg1.toString();
  } else {
    throw_bad_type_exception("implode() expects a container as "
                             "one of the arguments");
    return String();
  }
  return StringUtil::Implode(items, delim, false);
}

String HHVM_FUNCTION(join,
                     const Variant& arg1,
                     const Variant& arg2 /* = null_variant */) {
  return HHVM_FN(implode)(arg1, arg2);
}

TypedValue HHVM_FUNCTION(str_split, const String& str, int64_t split_length) {
  return tvReturn(StringUtil::Split(str, split_length));
}

TypedValue HHVM_FUNCTION(chunk_split, const String& body,
                      int chunklen, const String& end) {
  return tvReturn(StringUtil::ChunkSplit(body, chunklen, end));
}

struct TokenizerData final : RequestEventHandler {
  String str;
  int pos;
  int mask[256];

  void requestInit() override {
    str.reset();
    pos = 0;
    memset(&mask, 0, sizeof(mask));
  }
  void requestShutdown() override {
    requestInit();
  }
  void vscan(IMarker& mark) const override {
    mark(str);
  }
};
IMPLEMENT_STATIC_REQUEST_LOCAL(TokenizerData, s_tokenizer_data);

static Variant strtok(const String& str, const Variant& token) {
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

  // reset mask
  for (int i = 0; i < stoken.size(); i++) {
    mask[(unsigned char)stoken.data()[i]] = 0;
  }

  if (pos0 == sstr.size()) {
    return false;
  }

  String ret(s0 + pos0, i - pos0, CopyString);
  s_tokenizer_data->pos = i + 1;

  return ret;
}

TypedValue HHVM_FUNCTION(strtok, const String& str, const Variant& token) {
  return tvReturn(strtok(str, token));
}

namespace {

Variant str_replace(const Variant& search, const Variant& replace,
                    const String& subject, int &count, bool caseSensitive) {
  count = 0;
  if (search.isArray()) {
    String ret = subject;
    int c = 0;

    Array searchArr = search.toArray();
    if (replace.isArray()) {
      Array replArr = replace.toArray();
      ArrayIter replIter(replArr);
      for (ArrayIter iter(searchArr); iter; ++iter) {
        if (replIter) {
          ret = string_replace(ret, iter.second().toString(),
                               replIter.second().toString(),
                               c, caseSensitive);
          ++replIter;
        } else {
          ret = string_replace(ret, iter.second().toString(),
                               "", c, caseSensitive);
        }
        count +=c;
      }
      return ret;
    }

    String repl = replace.toString();
    for (ArrayIter iter(searchArr); iter; ++iter) {
      ret = string_replace(ret, iter.second().toString(), repl, c,
                           caseSensitive);
      count += c;
    }
    return ret;
  }

  if (replace.isArray()) {
    raise_notice("Array to string conversion");
  }
  return string_replace(subject, search.toString(), replace.toString(), count,
                        caseSensitive);
}

Variant str_replace(const Variant& search, const Variant& replace,
                    const Variant& subject, int &count, bool caseSensitive) {
  count = 0;
  if (subject.isArray()) {
    Array arr = subject.toArray();
    Array ret = Array::Create();
    int c;
    for (ArrayIter iter(arr); iter; ++iter) {
      if (iter.second().isArray() || iter.second().is(KindOfObject)) {
        ret.set(iter.first(), iter.second());
        continue;
      }

      auto const replaced = str_replace(
        search, replace, iter.second().toString(), c, caseSensitive
      ).toString();
      ret.set(iter.first(), replaced);
      count += c;
    }
    return ret;
  }
  return str_replace(search, replace, subject.toString(), count,
                     caseSensitive);
}

Variant str_replace(const Variant& search, const Variant& replace,
                    const Variant& subject, VRefParam count) {
  int nCount = 0;
  Variant ret;
  if (LIKELY(search.isString() && replace.isString() && subject.isString())) {
    // Short-cut for the most common (and simplest) case
    ret = string_replace(subject.asCStrRef(), search.asCStrRef(),
                         replace.asCStrRef(), nCount, true);
  } else {
    // search, replace, and subject can all be arrays. str_replace() reduces all
    // the valid combinations to multiple string_replace() calls.
    ret = str_replace(search, replace, subject, nCount, true);
  }
  if (auto ref = count.getRefDataOrNull()) *ref->var() = nCount;
  return ret;
}

Variant str_ireplace(const Variant& search, const Variant& replace,
                     const Variant& subject, VRefParam count) {
  int nCount = 0;
  Variant ret = str_replace(search, replace, subject, nCount, false);
  if (auto ref = count.getRefDataOrNull()) *ref->var() = nCount;
  return ret;
}

} // namespace

TypedValue HHVM_FUNCTION(str_replace,
                         const Variant& search, const Variant& replace,
                         const Variant& subject, VRefParam count) {
  return tvReturn(str_replace(search, replace, subject, count));
}

TypedValue HHVM_FUNCTION(str_ireplace,
                         const Variant& search, const Variant& replace,
                         const Variant& subject, VRefParam count) {
  return tvReturn(str_ireplace(search, replace, subject, count));
}

static Variant substr_replace(const Variant& str, const Variant& replacement,
                              const Variant& start, const Variant& length) {
  if (!str.isArray()) {
    String repl;
    if (replacement.isArray()) {
      repl = replacement.asCArrRef()[0].toString();
    } else {
      repl = replacement.toString();
    }
    if (start.isArray()) {
      if (!length.isArray()) {
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
    return string_replace(str.toString(), start.toInt32(), length.toInt32(),
                          repl);
  }

  // 'start' and 'length' can be arrays (in which case we step through them in
  // sync with stepping through 'str'), or not arrays, in which case we convert
  // them to ints and always use those.
  Array ret;
  Array strArr = str.toArray();
  folly::Optional<int> opStart;
  folly::Optional<int> opLength;
  if (!start.isArray()) {
    opStart = start.toInt32();
  }
  if (!length.isArray()) {
    opLength = length.toInt32();
  }

  Array startArr = start.toArray();
  Array lengthArr = length.toArray();
  ArrayIter startIter(startArr);
  ArrayIter lengthIter(lengthArr);

  if (replacement.isArray()) {
    Array replArr = replacement.toArray();
    ArrayIter replIter(replArr);
    for (ArrayIter iter(strArr); iter; ++iter) {
      auto str = iter.second().toString();
      // If 'start' or 'length' are arrays and we've gone past the end, default
      // to 0 for start and the length of the input string for length.
      int nStart =
        (opStart.hasValue()
         ? opStart.value()
         : (startIter ? startIter.second().toInt32() : 0));
      int nLength =
        (opLength.hasValue()
         ? opLength.value()
         : (lengthIter ? lengthIter.second().toInt32() : str.length()));
      if (startIter) ++startIter;
      if (lengthIter) ++lengthIter;

      String repl;
      if (replIter) {
        repl = replIter.second().toString();
        ++replIter;
      } else {
        repl = empty_string();
      }
      auto s2 = string_replace(str, nStart, nLength, repl);
      ret.append(s2);
    }
  } else {
    String repl = replacement.toString();
    for (ArrayIter iter(strArr); iter; ++iter) {
      auto str = iter.second().toString();
      int nStart =
        (opStart.hasValue()
         ? opStart.value()
         : (startIter ? startIter.second().toInt32() : 0));
      int nLength =
        (opLength.hasValue()
         ? opLength.value()
         : (lengthIter ? lengthIter.second().toInt32() : str.length()));
      if (startIter) ++startIter;
      if (lengthIter) ++lengthIter;

      auto s2 = string_replace(str, nStart, nLength, repl);
      ret.append(s2);
    }
  }
  return ret;
}

TypedValue HHVM_FUNCTION(substr_replace,
                         const Variant& str, const Variant& replacement,
                         const Variant& start, const Variant& length) {
  return tvReturn(substr_replace(str, replacement, start, length));
}

/*
 * Calculates and adjusts "start" and "length" according to string's length.
 * This function determines how those two parameters are interpreted in
 * f_substr.
 */
static bool string_substr_check(int len, int& f, int& l) {
  assertx(len >= 0);

  if (l < 0 && -l > len) {
    return false;
  }
  if (f >= len) {
    return false;
  }

  if (l > len) {
    l = len;
  }

  if (f < 0 && -f > len) {
    f = 0;
    if (len == 0) {
      return false;
    }
  }

  if (l < 0 && l + len < f) {
    return false;
  }

  // If "from" position is negative, count start position from the end.
  if (f < 0) {
    f += len;
  }
  assertx(f >= 0);

  // If "length" position is negative, set it to the length needed to stop that
  // many chars from the end of the string.
  if (l < 0) {
    l += len - f;
    if (l < 0) {
      l = 0;
    }
  }
  assertx(l >= 0);

  return true;
}

TypedValue HHVM_FUNCTION(substr, const String& str, int start, int length) {
  if (!string_substr_check(str.size(), start, length)) {
    if (RuntimeOption::PHP7_Substr && str.size() == start) {
      return make_tv<KindOfPersistentString>(empty_string_ref.get());
    } else {
      return make_tv<KindOfBoolean>(false);
    }
  }
  return tvReturn(str.substr(start, length));
}

String HHVM_FUNCTION(str_pad,
                     const String& input,
                     int pad_length,
                     const String& pad_string /* = " " */,
                     int pad_type /* = k_STR_PAD_RIGHT */) {
  return StringUtil::Pad(input, pad_length, pad_string,
                         (StringUtil::PadType)pad_type);
}

String HHVM_FUNCTION(str_repeat,
                     const String& input,
                     int64_t multiplier) {
  if (input.empty()) {
    return input;
  }

  if (multiplier < 0) {
    raise_warning("Second argument has to be greater than or equal to 0");
    return String();
  }

  if (multiplier == 0) {
    return empty_string();
  }

  if (input.size() == 1) {
    String ret(multiplier, ReserveString);

    memset(ret.mutableData(), *input.data(), multiplier);
    ret.setSize(multiplier);
    return ret;
  }

  auto size = multiplier * size_t(input.size());
  if (multiplier >= StringData::MaxSize || size > StringData::MaxSize) {
    throw
      FatalErrorException(0, "String length exceeded 2^31-2: %" PRIu64, size);
  }

  StringBuffer ret(input.size() * multiplier);

  while (multiplier--) {
    ret.append(input);
  }

  return ret.detach();
}

///////////////////////////////////////////////////////////////////////////////

Variant sscanfImpl(const String& str,
                   const String& format,
                   const req::vector<Variant*>& args) {
  Variant ret;
  int result;
  result = string_sscanf(str.c_str(), format.c_str(), args.size(), ret);
  if (SCAN_ERROR_WRONG_PARAM_COUNT == result) return init_null();
  if (args.empty()) return ret;

  if (ret.isArray()) {
    auto& retArray = ret.toArrRef();
    for (int i = 0; i < retArray.size(); i++) {
      auto var = args.at(i);
      if (var) {
        *var->getRefData() = retArray[i];
      }
    }
    return retArray.size();
  }
  if (ret.isNull()) return 0;
  return ret;
}

TypedValue* HHVM_FN(sscanf)(ActRec* ar) {
  String str{getArg<KindOfString>(ar, 0)};
  if (ar->numArgs() < 1) {
    return arReturn(ar, init_null());
  }
  String format{getArg<KindOfString>(ar, 1)};

  req::vector<Variant*> args;
  if (ar->numArgs() > 2) args.reserve(ar->numArgs() - 2);
  for (int i = 2; i < ar->numArgs(); ++i) {
    args.push_back(getArg<KindOfRef>(ar, i));
  }
  return arReturn(ar, sscanfImpl(str, format, args));
}

String HHVM_FUNCTION(chr, const Variant& ascii) {
  // This is the only known occurance of ParamCoerceModeNullByte,
  // so we treat it specially using an explicit tvCoerce call
  Variant v(ascii);
  auto tv = v.asTypedValue();
  char c = 0;
  if (tvCoerceParamToInt64InPlace(tv)) {
    c = tv->m_data.num & 0xFF;
  }
  return String::FromChar(c);
}

int64_t HHVM_FUNCTION(ord,
                      const String& str) {
  return (int64_t)(unsigned char)str[0];
}

Variant HHVM_FUNCTION(money_format,
                      const String& format,
                      double number) {
  String s = StringUtil::MoneyFormat(format.c_str(), number);
  if (s.isNull()) return false;
  return s;
}

String HHVM_FUNCTION(number_format,
                     double number,
                     int decimals /* = 0 */,
                     const Variant& dec_point_in /* = "." */,
                     const Variant& thousands_sep_in /* = "," */) {

  String dec_point(".");
  if (!dec_point_in.isNull()) {
    dec_point = dec_point_in.toString();
  }
  String thousands_sep(",");
  if (!thousands_sep_in.isNull()) {
    thousands_sep = thousands_sep_in.toString();
  }

  return string_number_format(number, decimals, dec_point, thousands_sep);
}

int64_t HHVM_FUNCTION(strcmp,
                      const String& str1,
                      const String& str2) {
  return string_strcmp(str1.data(), str1.size(), str2.data(), str2.size());
}

TypedValue HHVM_FUNCTION(strncmp,
                         const String& str1,
                         const String& str2,
                         int len) {
  if (len < 0) {
    raise_warning("Length must be greater than or equal to 0");
    return make_tv<KindOfBoolean>(false);
  }
  return tvReturn(string_strncmp(
    str1.data(), str1.size(),
    str2.data(), str2.size(), len
  ));
}

int64_t HHVM_FUNCTION(strnatcmp,
                      const String& str1,
                      const String& str2) {
  return string_natural_cmp(str1.data(), str1.size(), str2.data(), str2.size(),
                            false);
}

int64_t HHVM_FUNCTION(strcasecmp,
                      const String& str1,
                      const String& str2) {
  return bstrcasecmp(str1.data(), str1.size(), str2.data(), str2.size());
}

TypedValue HHVM_FUNCTION(strncasecmp,
                         const String& str1,
                         const String& str2,
                         int len) {
  if (len < 0) {
    raise_warning("Length must be greater than or equal to 0");
    return make_tv<KindOfBoolean>(false);
  }
  return tvReturn(string_strncasecmp(
    str1.data(), str1.size(),
    str2.data(), str2.size(), len
  ));
}

int64_t HHVM_FUNCTION(strnatcasecmp,
                      const String& str1,
                      const String& str2) {
  return string_natural_cmp(str1.data(), str1.size(), str2.data(), str2.size(),
                            true);
}

int64_t HHVM_FUNCTION(strcoll,
                      const String& str1,
                      const String& str2) {
  return strcoll(str1.c_str(), str2.c_str());
}

TypedValue HHVM_FUNCTION(substr_compare,
                         const String& main_str,
                         const String& str,
                         int offset,
                         int length /* = INT_MAX */,
                         bool case_insensitivity /* = false */) {
  int s1_len = main_str.size();
  int s2_len = str.size();

  if (length <= 0) {
    raise_warning("The length must be greater than zero");
    return make_tv<KindOfBoolean>(false);
  }

  if (offset < 0) {
    offset = s1_len + offset;
    if (offset < 0) offset = 0;
  }

  if (offset >= s1_len) {
    raise_warning("The start position cannot exceed initial string length");
    return make_tv<KindOfBoolean>(false);
  }

  int cmp_len = s1_len - offset;
  if (cmp_len < s2_len) cmp_len = s2_len;
  if (cmp_len > length) cmp_len = length;

  const char *s1 = main_str.data();
  if (case_insensitivity) {
    return tvReturn(bstrcasecmp(s1 + offset, cmp_len, str.data(), cmp_len));
  }
  return tvReturn(string_ncmp(s1 + offset, str.data(), cmp_len));
}

TypedValue HHVM_FUNCTION(strstr,
                         const String& haystack,
                         const Variant& needle,
                         bool before_needle /* = false */) {
  auto const tv = HHVM_FN(strpos)(haystack, needle);
  auto const& ret = tvAsCVarRef(&tv);
  assert(!isRefcountedType(tv.m_type));

  if (same(ret, false)) {
    return make_tv<KindOfBoolean>(false);
  }
  if (before_needle) {
    return tvReturn(haystack.substr(0, ret.toInt32()));
  } else {
    return tvReturn(haystack.substr(ret.toInt32()));
  }
}

TypedValue HHVM_FUNCTION(stristr,
                         const String& haystack,
                         const Variant& needle,
                         bool before_needle /* = false */) {
  auto const tv = HHVM_FN(stripos)(haystack, needle);
  auto const& ret = tvAsCVarRef(&tv);
  assert(!isRefcountedType(tv.m_type));

  if (same(ret, false)) {
    return make_tv<KindOfBoolean>(false);
  }
  if (before_needle) {
    return tvReturn(haystack.substr(0, ret.toInt32()));
  }
  return tvReturn(haystack.substr(ret.toInt32()));
}

template<bool existence_only>
static NEVER_INLINE
Variant strpbrk_char_list_has_nulls_slow(const String& haystack,
                                         const String& char_list) {

  auto const charListSz = char_list.size();
  auto const charListData = char_list.c_str();
  assert(memchr(charListData, '\0', charListSz) != nullptr);

  // in order to use strcspn, remove all null byte(s) from char_list
  auto charListWithoutNull = (char*) req::malloc_noptrs(charListSz);
  SCOPE_EXIT { req::free(charListWithoutNull); };

  auto copy_ptr = charListWithoutNull;
  auto const charListStop = charListData + char_list.size();
  for (auto ptr = charListData; ptr != charListStop; ++ptr) {
    if (*ptr != '\0') { *copy_ptr++ = *ptr; }
  }
  assert((copy_ptr - charListWithoutNull) < charListSz);
  // at least one of charListData chars was null, so there must be room:
  *copy_ptr = '\0';

  // Use strcspn instead of strpbrk because the latter doesn't report when
  // its terminated due to a null byte in haystack in any manageable way.
  auto haySize = haystack.size();
  auto hayData = haystack.c_str();

  size_t idx = strcspn(hayData, charListWithoutNull);
  if (idx < haySize) {
    // we know that char_list contains null bytes, being terminated because
    // haystack has null bytes is just dandy
    if (existence_only) { return true; }
    return String(hayData + idx, haySize - idx, CopyString);
  }
  return false;
}

template<bool existence_only>
static ALWAYS_INLINE
Variant strpbrk_impl(const String& haystack, const String& char_list) {
  if (char_list.empty()) {
    throw_invalid_argument("char_list: (empty)");
    return false;
  }
  if (haystack.empty()) {
    return false;
  }

  auto charListData = char_list.c_str();

  // equivalent to rawmemchr(charListData, '\0') ... charListData must be
  // null-terminated
  auto firstNull = charListData;
  while (*firstNull != '\0') { ++firstNull; }

  auto const hasNullByte = (firstNull - charListData) < char_list.size();

  if (UNLIKELY(hasNullByte)) {
    if ((firstNull - charListData) == (char_list.size() - 1)) {
      // the first null is the last character in char_list
    } else if (firstNull == charListData) {
      // the first null is the first character in char_list
      auto secondNull = firstNull + 1;
      while (*secondNull != '\0') { ++secondNull; }

      if ((secondNull - charListData) != char_list.size()) {
        return
          strpbrk_char_list_has_nulls_slow<existence_only>(haystack, char_list);
      }
      ++charListData; // we can remember the null byte
    } else {
      return
        strpbrk_char_list_has_nulls_slow<existence_only>(haystack, char_list);
    }
  }

  // Use strcspn instead of strpbrk because the latter doesn't report when
  // it's terminated due to a null byte in haystack in any manageable way.
  auto haySize = haystack.size();
  auto hayData = haystack.c_str();
retry:
  size_t idx = strcspn(hayData, charListData);
  if (idx < haySize) {
    if (UNLIKELY(hayData[idx] == '\0' && !hasNullByte)) {
      hayData += idx + 1;
      haySize -= idx + 1;
      goto retry;
    }
    if (existence_only) { return true; }

    return String(hayData + idx, haySize - idx, CopyString);
  }
  return false;
}

bool str_contains_any_of(const String& haystack, const String& char_list) {
  return strpbrk_impl<true>(haystack, char_list).toBooleanVal();
}

TypedValue HHVM_FUNCTION(strpbrk,
                         const String& haystack,
                         const String& char_list) {
  return tvReturn(strpbrk_impl<false>(haystack, char_list));
}

TypedValue HHVM_FUNCTION(strpos,
                         const String& haystack,
                         const Variant& needle,
                         int offset /* = 0 */) {
  if (offset < 0 || offset > haystack.size()) {
    raise_warning("Offset not contained in string");
    return make_tv<KindOfBoolean>(false);
  }
  int pos;
  if (needle.isString()) {
    String n(needle.toString());
    if (n.length() == 0) {
      raise_warning("Empty delimiter");
      return make_tv<KindOfBoolean>(false);
    }
    pos = haystack.find(n, offset);
  } else {
    pos = haystack.find(needle.toByte(), offset);
  }
  if (pos >= 0) return make_tv<KindOfInt64>(pos);
  return make_tv<KindOfBoolean>(false);
}

TypedValue HHVM_FUNCTION(stripos,
                         const String& haystack,
                         const Variant& needle,
                         int offset /* = 0 */) {
  if (offset < 0 || offset > haystack.size()) {
    raise_warning("Offset not contained in string");
    return make_tv<KindOfBoolean>(false);
  }
  int pos;
  if (needle.isString()) {
    pos = haystack.find(needle.toString(), offset, false);
  } else {
    pos = haystack.find(needle.toByte(), offset, false);
  }
  if (pos >= 0) return make_tv<KindOfInt64>(pos);
  return make_tv<KindOfBoolean>(false);
}

static bool is_valid_strrpos_args(
    const String& haystack,
    const Variant& needle,
    int offset) {
  if (haystack.size() == 0) {
    return false;
  }
  if (needle.isString() && needle.toString().size() == 0) {
    return false;
  }
  if (offset < -haystack.size() || offset > haystack.size()) {
    raise_warning("Offset is greater than the length of haystack string");
    return false;
  }
  return true;
}

TypedValue HHVM_FUNCTION(strchr,
                         const String& haystack,
                         const Variant& needle) {
  return HHVM_FN(strstr)(haystack, needle);
}

TypedValue HHVM_FUNCTION(strrchr,
                         const String& haystack,
                         const Variant& needle) {
  if (haystack.size() == 0) {
    return make_tv<KindOfBoolean>(false);
  }

  int pos;
  if (needle.isString() && needle.toString().size() > 0) {
    pos = haystack.rfind(needle.toString().data()[0], false);
  } else {
    pos = haystack.rfind(needle.toByte(), false);
  }
  if (pos < 0) return make_tv<KindOfBoolean>(false);
  return tvReturn(haystack.substr(pos));
}

TypedValue HHVM_FUNCTION(strrpos,
                         const String& haystack,
                         const Variant& needle,
                         int offset /* = 0 */) {
  if (!is_valid_strrpos_args(haystack, needle, offset)) {
    return make_tv<KindOfBoolean>(false);
  }
  int pos;
  if (needle.isString()) {
    pos = haystack.rfind(needle.toString(), offset);
  } else {
    pos = haystack.rfind(needle.toByte(), offset);
  }
  if (pos >= 0) return make_tv<KindOfInt64>(pos);
  return make_tv<KindOfBoolean>(false);
}

TypedValue HHVM_FUNCTION(strripos,
                         const String& haystack,
                         const Variant& needle,
                         int offset /* = 0 */) {
  if (!is_valid_strrpos_args(haystack, needle, offset)) {
    return make_tv<KindOfBoolean>(false);
  }
  int pos;
  if (needle.isString()) {
    pos = haystack.rfind(needle.toString(), offset, false);
  } else {
    pos = haystack.rfind(needle.toByte(), offset, false);
  }
  if (pos >= 0) return make_tv<KindOfInt64>(pos);
  return make_tv<KindOfBoolean>(false);
}

TypedValue HHVM_FUNCTION(substr_count,
                         const String& haystack,
                         const String& needle,
                         int offset /* = 0 */,
                         int length /* = 0x7FFFFFFF */) {
  int lenNeedle = needle.size();
  if (lenNeedle == 0) {
    throw_invalid_argument("needle: (empty)");
    return make_tv<KindOfBoolean>(false);
  }

  if (offset < 0 || offset > haystack.size()) {
    throw_invalid_argument("offset: (out of range)");
    return make_tv<KindOfBoolean>(false);
  }
  if (length == 0x7FFFFFFF) {
    length = haystack.size() - offset;
  } else if (length <= 0 || length > haystack.size() - offset) {
    throw_invalid_argument("length: (out of range)");
    return make_tv<KindOfBoolean>(false);
  }

  int count = 0;
  int posMax = offset + length - lenNeedle;
  for (int pos = haystack.find(needle, offset);
       pos != -1 && pos <= posMax;
       pos = haystack.find(needle, pos + lenNeedle)) {
    ++count;
  }
  return make_tv<KindOfInt64>(count);
}

namespace {
  bool string_strspn_check(int inputLength, int &start, int &scanLength) {
    if (start < 0) {
      start += inputLength;
      if (start < 0) {
        start = 0;
      }
    } else if (start > inputLength) {
      return false;
    }

    if (scanLength < 0) {
      scanLength += (inputLength - start);
      if (scanLength < 0) {
        scanLength = 0;
      }
    }
    if (scanLength > inputLength - start) {
      scanLength = inputLength - start;
    }
    return true;
  }
}

TypedValue HHVM_FUNCTION(strspn,
                         const String& str1,
                         const String& str2,
                         int start /* = 0 */,
                         int length /* = 0x7FFFFFFF */) {
  const char *s1 = str1.data();
  const char *s2 = str2.data();
  int s1_len = str1.size();
  int s2_len = str2.size();

  if (!string_strspn_check(s1_len, start, length)) {
    return make_tv<KindOfBoolean>(false);
  }

  s1 += start;
  for (int pos = 0; pos < length; ++pos) {
    if (memchr(s2, *(s1++), s2_len) == nullptr) {
      return make_tv<KindOfInt64>(pos);
    }
  }

  return make_tv<KindOfInt64>(length);
}

TypedValue HHVM_FUNCTION(strcspn,
                         const String& str1,
                         const String& str2,
                         int start /* = 0 */,
                         int length /* = 0x7FFFFFFF */) {
  const char *s1 = str1.data();
  const char *s2 = str2.data();
  int s1_len = str1.size();
  int s2_len = str2.size();

  if (!string_strspn_check(s1_len, start, length)) {
    return make_tv<KindOfBoolean>(false);
  }

  s1 += start;
  for (int pos = 0; pos < length; ++pos) {
    if (memchr(s2, *(s1++), s2_len) != nullptr) {
      return make_tv<KindOfInt64>(pos);
    }
  }

  return make_tv<KindOfInt64>(length);
}

TypedValue HHVM_FUNCTION(strlen,
                         const Variant& vstr) {
  auto const cell = vstr.asCell();
  switch (cell->m_type) {
    case KindOfPersistentString:
    case KindOfString:
      return make_tv<KindOfInt64>(cell->m_data.pstr->size());

    case KindOfPersistentArray:
    case KindOfArray:
      raise_warning("strlen() expects parameter 1 to be string, "
                    "array given");
      return make_tv<KindOfNull>();

    case KindOfResource:
      raise_warning("strlen() expects parameter 1 to be string, "
                    "resource given");
      return make_tv<KindOfNull>();

    case KindOfObject:
      if (!HHVM_FN(method_exists)(vstr, "__toString")) {
        raise_warning("strlen() expects parameter 1 to be string, "
                      "object given");
        return make_tv<KindOfNull>();
      }
      // else fallback to default
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble: {
      const String& str = vstr.toString();
      return make_tv<KindOfInt64>(str.size());
    }

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

Array HHVM_FUNCTION(str_getcsv,
                    const String& str,
                    const String& delimiter /* = "," */,
                    const String& enclosure /* = "\"" */,
                    const String& escape /* = "\\" */) {
  if (str.empty()) {
    return Array::Create(null_variant);
  }

  auto check_arg = [](const String& arg, char default_arg) {
    return arg.size() > 0 ? arg[0] : default_arg;
  };

  char delimiter_char = check_arg(delimiter, ',');
  char enclosure_char = check_arg(enclosure, '"');
  char escape_char = check_arg(escape, '\\');

  auto dummy = req::make<PlainFile>();
  return dummy->readCSV(0, delimiter_char, enclosure_char, escape_char, &str);
}

Variant HHVM_FUNCTION(count_chars,
                      const String& str,
                      int64_t mode /* = 0 */) {
  int chars[256];
  memset((void*)chars, 0, sizeof(chars));
  const unsigned char *buf = (const unsigned char *)str.data();
  for (int len = str.size(); len > 0; len--) {
    chars[*buf++]++;
  }

  Array retarr = Array::Create();
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

  throw_invalid_argument("mode: %" PRId64, mode);
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
Variant HHVM_FUNCTION(str_word_count,
                      const String& str,
                      int64_t format /* = 0 */,
                      const String& charlist /* = "" */) {
  int str_len = str.size();
  switch (format) {
  case 1:
  case 2:
    if (!str_len) {
      return empty_array();
    }
    break;
  case 0:
    if (!str_len) {
      return 0LL;
    }
    break;
  default:
    throw_invalid_argument("format: %" PRId64, format);
    return false;
  }

  char ch[256];
  const char *char_list = charlist.data();
  if (*char_list) {
    string_charmask(char_list, charlist.size(), ch);
  } else {
    char_list = nullptr;
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

int64_t HHVM_FUNCTION(levenshtein,
                      const String& str1,
                      const String& str2,
                      int cost_ins /* = 1 */,
                      int cost_rep /* = 1 */,
                      int cost_del /* = 1 */) {
  return string_levenshtein(str1.data(), str1.size(), str2.data(), str2.size(),
                            cost_ins, cost_rep, cost_del);
}

int64_t HHVM_FUNCTION(similar_text,
                      const String& first,
                      const String& second,
                      VRefParam percent /* = uninit_null() */) {
  float p;
  int ret = string_similar_text(first.data(), first.size(),
                                second.data(), second.size(), &p);
  percent.assignIfRef(p);
  return ret;
}

Variant HHVM_FUNCTION(soundex,
                      const String& str) {
  if (str.empty()) return false;
  return string_soundex(str);
}

Variant HHVM_FUNCTION(metaphone,
                      const String& str,
                      int phones /* = 0 */) {
  return string_metaphone(str.data(), str.size(), 0, 1);
}

String HHVM_FUNCTION(html_entity_decode,
                     const String& str,
                     int flags /* = k_ENT_HTML_QUOTE_DOUBLE */,
                     const String& charset /* = "UTF-8" */) {
  const char *scharset = charset.data();
  if (!*scharset) scharset = "ISO-8859-1";
  return StringUtil::HtmlDecode(str, StringUtil::toQuoteStyle(flags),
                                scharset, true);
}

String HHVM_FUNCTION(htmlentities,
                     const String& str,
                     int flags /* = k_ENT_HTML_QUOTE_DOUBLE */,
                     const String& charset /* = "UTF-8" */,
                     bool double_encode /* = true */) {
  // dropping double_encode parameters and see runtime/base/zend-html.h
  const char *scharset = charset.data();
  if (!*scharset) scharset = "ISO-8859-1";
  return StringUtil::HtmlEncode(str, StringUtil::toQuoteStyleBitmask(flags),
                                scharset, double_encode, true);
}

String HHVM_FUNCTION(htmlspecialchars_decode,
                     const String& str,
                     int flags /* = k_ENT_HTML_QUOTE_DOUBLE */) {
  return StringUtil::HtmlDecode(str, StringUtil::toQuoteStyle(flags),
                                "UTF-8", false);
}

String HHVM_FUNCTION(htmlspecialchars,
                     const String& str,
                     int flags /* = k_ENT_HTML_QUOTE_DOUBLE */,
                     const String& charset /* = "UTF-8" */,
                     bool double_encode /* = true */) {
  // dropping double_encode parameters and see runtime/base/zend-html.h
  const char *scharset = charset.data();
  if (!*scharset) scharset = "ISO-8859-1";
  return StringUtil::HtmlEncode(str, StringUtil::toQuoteStyleBitmask(flags),
                                scharset, double_encode, false);
}

String HHVM_FUNCTION(fb_htmlspecialchars,
                     const String& str,
                     int flags /* = k_ENT_HTML_QUOTE_DOUBLE */,
                     const String& charset /* = "ISO-8859-1" */,
                     const Variant& extra /* = empty_array_ref */) {
  if (!extra.isNull() && !extra.isArray()) {
    throw_expected_array_exception("fb_htmlspecialchars");
  }
  const Array& arr_extra = extra.isNull() ? empty_array_ref : extra.toArray();
  return StringUtil::HtmlEncodeExtra(str, StringUtil::toQuoteStyle(flags),
                                     charset.data(), false, arr_extra);
}

String HHVM_FUNCTION(quoted_printable_encode,
                     const String& str) {
  return StringUtil::QuotedPrintableEncode(str);
}

String HHVM_FUNCTION(quoted_printable_decode,
                     const String& str) {
  return StringUtil::QuotedPrintableDecode(str);
}

Variant HHVM_FUNCTION(convert_uudecode,
                      const String& data) {
  String ret = StringUtil::UUDecode(data);
  if (ret.isNull()) {
    raise_warning(
      "convert_uudecode(): "
      "The given parameter is not a valid uuencoded string");
    return false; // bad format
  }
  return ret;
}

Variant HHVM_FUNCTION(convert_uuencode,
                      const String& data) {
  if (data.empty()) return false;
  return StringUtil::UUEncode(data);
}

String HHVM_FUNCTION(str_rot13,
                     const String& str) {
  return StringUtil::ROT13(str);
}

int64_t HHVM_FUNCTION(crc32,
                      const String& str) {
  return (uint32_t)StringUtil::CRC32(str);
}

String HHVM_FUNCTION(crypt,
                     const String& str,
                     const String& salt /* = "" */) {
  return StringUtil::Crypt(str, salt.c_str());
}

String HHVM_FUNCTION(md5,
                     const String& str,
                     bool raw_output /* = false */) {
  return StringUtil::MD5(str, raw_output);
}

String HHVM_FUNCTION(sha1,
                     const String& str,
                     bool raw_output /* = false */) {
  return StringUtil::SHA1(str, raw_output);
}

// The WuManberReplacement class, related data structures and hash function
// are ported from php_strtr_array_* as implemented in PHP 5.6.10.

#define SHIFT_TAB_BITS  13 // should be >= HASH_TAB_BITS
#define HASH_TAB_BITS   10 // should be less than sizeof(uint16_t)
#define SHIFT_TAB_SIZE  (1U << SHIFT_TAB_BITS)
#define HASH_TAB_SIZE   (1U << HASH_TAB_BITS)
#define SHIFT_TAB_MASK ((uint16_t)(SHIFT_TAB_SIZE - 1))
#define HASH_TAB_MASK ((uint16_t)(HASH_TAB_SIZE - 1))

struct PatAndRepl {
  uint16_t hash(int start, int len) const;

  const std::string getPat() const {
    return pat;
  }

  const std::string getRepl() const {
    return repl;
  }

  PatAndRepl(const String& pat, const String& repl)
  : pat(pat.data(), pat.size()), repl(repl.data(), repl.size()) { }

private:
  std::string pat;
  std::string repl;
};

using ShiftTab   = std::array<size_t, SHIFT_TAB_SIZE>;
using HashTab    = std::array<int, HASH_TAB_SIZE+1>;
using PrefixVec  = std::vector<uint16_t>;
using PatternVec = std::vector<PatAndRepl>;

struct WuManberReplacement {
private:
  PrefixVec   prefix;   // prefixes hashes by pat suffix hash order
  size_t      m;        // minimum pattern length
  int         B;        // size of suffixes
  int         Bp;       // size of prefixes
  PatternVec  patterns; // list of patterns and replacements
  ShiftTab    shift;    // table mapping hash to allowed shift
  HashTab     hash;     // table mapping hash to pos in patterns
  bool        valid;    // can translation occur

  bool initPatterns(const Array& pats);
  void initTables();

public:
  WuManberReplacement(const Array &arr, size_t minLen)
  : m(minLen), B(MIN(m,2)), Bp(MIN(m,2)),
    valid(initPatterns(arr)) { }

  Variant translate(String source) const;
};

static inline uint16_t strtr_hash(const char *str, int len) {
    uint16_t  res = 0;
    for (int i = 0; i < len; i++) {
        res = res * 33 + (unsigned char)str[i];
    }

    return res;
}

struct strtr_compare_hash_suffix {

  strtr_compare_hash_suffix(size_t m, int B)
  : m(m), B(B) { }

  bool operator() (const PatAndRepl &a, const PatAndRepl &b) {
    uint16_t  hash_a = a.hash(m - B, B) & HASH_TAB_MASK,
              hash_b = b.hash(m - B, B) & HASH_TAB_MASK;

    if (hash_a > hash_b) {
      return false;
    }
    if (hash_a < hash_b) {
      return true;
    }
    // longer patterns must be sorted first
    if (a.getPat().size() > b.getPat().size()) {
      return true;
    }
    if (a.getPat().size() < b.getPat().size()) {
      return false;
    }
    return false;
  }

private:
  size_t m;
  int B;
};

uint16_t inline PatAndRepl::hash(int start, int len) const {
  assert(pat.size() >= start + len);
  return strtr_hash(pat.data() + start, len);
};

bool WuManberReplacement::initPatterns(const Array& arr) {
  patterns.reserve(arr.size());
  for (ArrayIter iter(arr); iter; ++iter) {
    String pattern = iter.first().toString();
    if (pattern.size() == 0) { // empty string given as pattern
      patterns.clear();
      return false;
    }
    patterns.emplace_back(pattern, iter.second().toString());
  }

  initTables();

  return true;
}

void WuManberReplacement::initTables() {
  size_t max_shift = m - B + 1;
  hash.fill(-1);
  shift.fill(max_shift);
  prefix.reserve(patterns.size());

  strtr_compare_hash_suffix comparator(m, B);
  std::sort(patterns.begin(), patterns.end(), comparator);

  {
    uint16_t last_h = -1; // assumes not all bits are used
    // patterns is already ordered by hash.
    // Make hash[h] de index of the first pattern in
    // patterns that has hash
    int size = patterns.size();
    for(int i = 0; i != size; ++i) {
      // init hash tab
      uint16_t h = patterns[i].hash(m - B, B) & HASH_TAB_MASK;
      if (h != last_h) {
        hash[h] = i;
        last_h = h;
      }
      // init shift tab
      for (int j = 0; j < max_shift; j++) {
        uint16_t h = patterns[i].hash( j, B ) & SHIFT_TAB_MASK;
        assert((long long) m - (long long) j - B >= 0);
        shift[h] = MIN(shift[h], m - j - B);
      }
      // init prefix
      prefix.push_back(patterns[i].hash(0, Bp));
    }
  }

  hash[HASH_TAB_SIZE] = patterns.size();  // OK, we allocated SIZE+1
  for (int i = HASH_TAB_SIZE - 1; i >= 0; i--) {
    if (hash[i] == -1) {
      hash[i] = hash[i + 1];
    }
  }
}

Variant WuManberReplacement::translate(String source) const {
  size_t  pos      = 0,
          nextwpos = 0,
          lastpos  = source.size() - m;

  if (!valid) {
    return false;
  }

  // all patterns are longer than the source
  if (m > source.size()) {
    return source;
  }

  StringBuffer  result(source.size());
  while (pos <= lastpos) {
    uint16_t h = strtr_hash(source.data() + pos + m - B, B) & SHIFT_TAB_MASK;
    size_t shift_pos = shift[h];

    if (shift_pos > 0) {
      pos += shift_pos;
    } else {
      uint16_t  h2        = h & HASH_TAB_MASK,
                prefix_h  = strtr_hash(source.data() + pos, Bp);
      int offset_start  = hash[h2],
          offset_end    = hash[h2 + 1], // exclusive
          i             = 0;

      for (i = offset_start; i < offset_end; i++) {
        if (prefix[i] != prefix_h) {
          continue;
        }

        const PatAndRepl *pnr = &patterns[i];
        if (pnr->getPat().size() > source.size() - pos ||
            memcmp(pnr->getPat().data(), source.data() + pos,
                   pnr->getPat().size()) != 0) {
          continue;
        }

        result.append(source.data() + nextwpos, pos - nextwpos);
        result.append(pnr->getRepl());
        pos += pnr->getPat().size();
        nextwpos = pos;
        goto end_outer_loop;
      }

      pos++;
end_outer_loop: ;
    }
  }

  result.append(source.data() + nextwpos, source.size() - nextwpos );

  return result.detach();
}

bool strtr_slow(const Array& arr, StringBuffer& result, String& key,
                const char*s, int& pos, int minlen, int maxlen) {

  memcpy(key.mutableData(), s + pos, maxlen);
  for (int len = maxlen; len >= minlen; len--) {
    key.setSize(len);
    auto const& var = arr->get(arr.convertKey(key));
    if (&var != &null_variant) {
      String replace = var.toString();
      if (!replace.empty()) {
        result.append(replace);
      }
      pos += len;
      return true;
    }
  }
  return false;
}

Variant strtr_fast(const String& str, const Array& arr,
                   int minlen, int maxlen) {
  using PatternMask = uint64_t[256];
  auto mask = req::calloc_raw_array<PatternMask>(maxlen);
  SCOPE_EXIT { req::free(mask); };

  int pattern_id = 0;
  for (ArrayIter iter(arr); iter; ++iter, pattern_id++) {
    auto const search = iter.first().toString();
    auto slice = search.slice();

    for (auto i = 0; i < slice.size(); i++) {
      mask[i][(unsigned char)slice.data()[i]] |= (1LL << pattern_id);
    }
  }
  auto s = str.data();
  auto slen = str.size();
  StringBuffer result(slen);
  String key(maxlen, ReserveString);

  for (auto i = 0; i < slen;) {
    if ((i + maxlen) > slen) {
      maxlen = slen - i;
    }
    uint64_t match = ~0x0ULL;
    bool possible_match = false;

    for (auto pos = 0; pos < maxlen; pos++) {
      match &= mask[pos][(unsigned char)s[i + pos]];
      if (!match) break;
      if (pos >= minlen - 1) {
        possible_match = true;
        break;
      }
    }
    bool found = false;
    if (possible_match) {
      found = strtr_slow(arr, result, key, s, i, minlen, maxlen);
    }
    if (!found) {
      result.append(s[i++]);
    }
  }
  return result.detach();
}

static constexpr int kBitsPerQword = CHAR_BIT * sizeof(uint64_t);

using WuManberPtr   = std::shared_ptr<const WuManberReplacement>;
using WuManberCache = ConcurrentLRUCache<int64_t, WuManberPtr>;
static WuManberCache wuManberCache(10);

Variant HHVM_FUNCTION(strtr,
                      const String& str,
                      const Variant& from,
                      const Variant& to /* = null_variant */) {
  if (str.empty()) {
    return str;
  }

  if (!to.isNull()) {
    return StringUtil::Translate(str, from.toString(), to.toString());
  }

  if (!from.isArray()) {
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
    auto const search = iter.first().toString();
    auto const len = search.size();
    if (len < 1) return false;
    if (maxlen < len) maxlen = len;
    if (minlen == -1 || minlen > len) minlen = len;
  }

  if (arr.size() <= kBitsPerQword && maxlen <= 16) {
    return strtr_fast(str, arr, minlen, maxlen);
  }

  if (arr.size() < 1000) {
    const WuManberReplacement replacer(arr, minlen);
    return replacer.translate(str);
  }

  // wu manber cost is mostly in preprocessing the patterns into
  // tables.  this hash is much faster than initializing and most
  // codebases likely only have a few constant sets with more than
  // a thousand patterns.
  int64_t hash = 0;
  for (ArrayIter iter(arr); iter; ++iter) {
    String pattern = iter.first().toString();
    String replacement = iter.second().toString();
    hash = murmur_hash_64A(pattern.data(), pattern.size(), hash);
    hash = murmur_hash_64A(replacement.data(), replacement.size(), hash);
  }
  WuManberCache::ConstAccessor got;
  WuManberPtr replacer;
  if (wuManberCache.find(got, hash)) {
    replacer = *got;
  } else {
    replacer.reset(new WuManberReplacement(arr, minlen));
    wuManberCache.insert(hash, replacer);
  }
  return replacer->translate(str);
}

Variant HHVM_FUNCTION(setlocale,
                      int category,
                      const Variant& locale,
                      const Array& _argv /* = null_array */) {
  Array argv = _argv;
  if (locale.isArray()) {
    if (!argv.empty()) throw_invalid_argument("locale: not string)");
    argv = locale; // ignore _argv
  }

  for (int i = -1; i < argv.size(); i++) {
    String slocale;
    if (i == -1) {
      if (locale.isArray()) continue;
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
      loc = nullptr;
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

const StaticString
  s_decimal_point("decimal_point"),
  s_thousands_sep("thousands_sep"),
  s_int_curr_symbol("int_curr_symbol"),
  s_currency_symbol("currency_symbol"),
  s_mon_decimal_point("mon_decimal_point"),
  s_mon_thousands_sep("mon_thousands_sep"),
  s_positive_sign("positive_sign"),
  s_negative_sign("negative_sign"),
  s_int_frac_digits("int_frac_digits"),
  s_frac_digits("frac_digits"),
  s_p_cs_precedes("p_cs_precedes"),
  s_p_sep_by_space("p_sep_by_space"),
  s_n_cs_precedes("n_cs_precedes"),
  s_n_sep_by_space("n_sep_by_space"),
  s_p_sign_posn("p_sign_posn"),
  s_n_sign_posn("n_sign_posn"),
  s_grouping("grouping"),
  s_mon_grouping("mon_grouping");

Array HHVM_FUNCTION(localeconv) {
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

String HHVM_FUNCTION(nl_langinfo,
                     int item) {
#ifdef _MSC_VER
  raise_warning("nl_langinfo is not yet implemented on Windows!");
  return "";
#else
  return nl_langinfo(item);
#endif
}

String HHVM_FUNCTION(convert_cyr_string,
                     const String& str,
                     const String& from,
                     const String& to) {
  return string_convert_cyrillic_string(str, from[0], to[0]);
}


const StaticString
  s_amp("&"),
  s_ampsemi("&amp;");


#define ENT_HTML_QUOTE_NONE     0
#define ENT_HTML_QUOTE_SINGLE   1
#define ENT_HTML_QUOTE_DOUBLE   2

static const HtmlBasicEntity basic_entities_noapos[] = {
  { '"',  "&quot;",   6,  ENT_HTML_QUOTE_DOUBLE },
  { '\'', "&#039;",   6,  ENT_HTML_QUOTE_SINGLE },
  { '<',  "&lt;",     4,  0 },
  { '>',  "&gt;",     4,  0 },
  { 0,    nullptr,    0,  0 }
};

static const HtmlBasicEntity basic_entities_apos[] = {
  { '"',  "&quot;",   6,  ENT_HTML_QUOTE_DOUBLE },
  { '\'', "&apos;",   6,  ENT_HTML_QUOTE_SINGLE },
  { '<',  "&lt;",     4,  0 },
  { '>',  "&gt;",     4,  0 },
  { 0,     nullptr,   0,  0 }
};

const HtmlBasicEntity* get_basic_table(bool all, entity_doctype doctype) {
  if (doctype == entity_doctype::xhtml) {
    return all ? basic_entities_noapos : basic_entities_apos;
  }

  if (doctype == entity_doctype::html401) {
    return basic_entities_noapos;
  }

  return basic_entities_apos;
}

#define ENT_HTML_DOC_TYPE_MASK  (16|32)
#define ENT_HTML_DOC_HTML401    0
#define ENT_HTML_DOC_XML1       16
#define ENT_HTML_DOC_XHTML      32
#define ENT_HTML_DOC_HTML5      (16|32)

entity_doctype determine_doctype(int flags) {
  int mask = flags & ENT_HTML_DOC_TYPE_MASK;
  switch (mask) {
    case ENT_HTML_DOC_HTML401: return entity_doctype::html401;
    case ENT_HTML_DOC_XML1: return entity_doctype::xml1;
    case ENT_HTML_DOC_XHTML: return entity_doctype::xhtml;
    case ENT_HTML_DOC_HTML5: return entity_doctype::html5;
  }
  not_reached();
}

String encode_as_utf8(int code_point) {
  auto res = folly::codePointToUtf8(code_point);
  return String::FromCStr(res.data());
}

Array HHVM_FUNCTION(get_html_translation_table,
                    int table /* = 0 */,
                    int flags /* = k_ENT_HTML_QUOTE_DOUBLE */,
                    const String& encoding /* = "UTF-8" */) {
  using namespace entity_charset_enum;
  auto charset = determine_charset(encoding.data());
  if (charset == cs_unknown) {
    charset = cs_utf_8;
    if (!encoding.empty()) {
      raise_warning("get_html_translation_table(): charset `%s' not supported"
                    ", assuming utf-8", encoding.data());
    }
  }
  auto doctype = determine_doctype(flags);

  bool all = (table == k_HTML_ENTITIES);

  Array ret;
  switch (table) {
  case k_HTML_ENTITIES: {
    if (charset == cs_utf_8) {
      auto entity_map = get_doctype_entity_table(doctype);
      for (const auto& item : *entity_map) {
        auto key = encode_as_utf8(item.first);

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "&%s;", item.second.c_str());
        ret.set(key, String(buffer, CopyString));
      }
      if (doctype == entity_doctype::html5) {
        for (const auto& item: *get_multicode_table()) {
          auto codes = item.first;
          String key = encode_as_utf8(codes.first);
          key += encode_as_utf8(codes.second);

          char buffer[32];
          snprintf(buffer, sizeof(buffer), "&%s", item.second.c_str());
          ret.set(key, String(buffer, CopyString));
        }
      }
    } else {
      const auto& entity_map = get_doctype_entity_table(doctype);
      auto charset_table = get_charset_table(charset);
      for (const auto& item : *charset_table) {
        const auto iter = entity_map->find(item.second);
        if (iter != entity_map->end()) {
          char buffer[16];
          snprintf(buffer, sizeof(buffer), "&%s;", iter->second.c_str());

          auto key = String::FromChar(item.first);
          ret.set(key, String(buffer, CopyString));
        }
      }
    }
    /* fall thru */
  }
  case k_HTML_SPECIALCHARS:
    const auto& basic_table = get_basic_table(all, doctype);
    for (int j = 0; basic_table[j].charcode != 0; j++) {
      const auto& item = basic_table[j];
      if (item.flags && (flags & item.flags) == 0)
        continue;

      ret.set(String::FromChar(item.charcode), item.entity);
    }
    ret.set(s_amp, s_ampsemi);
    break;
  }

  return ret;
}

String HHVM_FUNCTION(hebrev,
                     const String& hebrew_text,
                     int max_chars_per_line /* = 0 */) {
  if (hebrew_text.empty()) return hebrew_text;
  return string_convert_hebrew_string(hebrew_text, max_chars_per_line, false);
}

String HHVM_FUNCTION(hebrevc,
                     const String& hebrew_text,
                     int max_chars_per_line /* = 0 */) {
  if (hebrew_text.empty()) return hebrew_text;
  return string_convert_hebrew_string(hebrew_text, max_chars_per_line, true);
}

///////////////////////////////////////////////////////////////////////////////

struct StringExtension final : Extension {
  StringExtension() : Extension("string") {}
  void moduleInit() override {
    setlocale(LC_CTYPE, "");

    HHVM_FE(addcslashes);
    HHVM_FE(stripcslashes);
    HHVM_FE(addslashes);
    HHVM_FE(stripslashes);
    HHVM_FE(bin2hex);
    HHVM_FE(hex2bin);
    HHVM_FE(nl2br);
    HHVM_FE(quotemeta);
    HHVM_FE(str_shuffle);
    HHVM_FE(strrev);
    HHVM_FE(strtolower);
    HHVM_FE(strtoupper);
    HHVM_FE(ucfirst);
    HHVM_FE(lcfirst);
    HHVM_FE(ucwords);
    HHVM_FE(strip_tags);
    HHVM_FE(trim);
    HHVM_FE(ltrim);
    HHVM_FE(rtrim);
    HHVM_FE(chop);
    HHVM_FE(explode);
    HHVM_FE(implode);
    HHVM_FE(join);
    HHVM_FE(str_split);
    HHVM_FE(chunk_split);
    HHVM_FE(strtok);
    HHVM_FE(str_replace);
    HHVM_FE(str_ireplace);
    HHVM_FE(substr_replace);
    HHVM_FE(substr);
    HHVM_FE(str_pad);
    HHVM_FE(str_repeat);
    HHVM_FE(html_entity_decode);
    HHVM_FE(htmlentities);
    HHVM_FE(htmlspecialchars_decode);
    HHVM_FE(htmlspecialchars);
    HHVM_FE(fb_htmlspecialchars);
    HHVM_FE(quoted_printable_encode);
    HHVM_FE(quoted_printable_decode);
    HHVM_FE(convert_uudecode);
    HHVM_FE(convert_uuencode);
    HHVM_FE(str_rot13);
    HHVM_FE(crc32);
    HHVM_FE(crypt);
    HHVM_FE(md5);
    HHVM_FE(sha1);
    HHVM_FE(strtr);
    HHVM_FE(convert_cyr_string);
    HHVM_FE(get_html_translation_table);
    HHVM_FE(hebrev);
    HHVM_FE(hebrevc);
    HHVM_FE(setlocale);
    HHVM_FE(localeconv);
    HHVM_FE(nl_langinfo);
    HHVM_FE(sscanf);
    HHVM_FE(chr);
    HHVM_FE(ord);
    HHVM_FE(money_format);
    HHVM_FE(number_format);
    HHVM_FE(strcmp);
    HHVM_FE(strncmp);
    HHVM_FE(strnatcmp);
    HHVM_FE(strcasecmp);
    HHVM_FE(strncasecmp);
    HHVM_FE(strnatcasecmp);
    HHVM_FE(strcoll);
    HHVM_FE(substr_compare);
    HHVM_FE(strchr);
    HHVM_FE(strrchr);
    HHVM_FE(strstr);
    HHVM_FE(stristr);
    HHVM_FE(strpbrk);
    HHVM_FE(strpos);
    HHVM_FE(stripos);
    HHVM_FE(strrpos);
    HHVM_FE(strripos);
    HHVM_FE(substr_count);
    HHVM_FE(strspn);
    HHVM_FE(strcspn);
    HHVM_FE(strlen);
    HHVM_FE(str_getcsv);
    HHVM_FE(count_chars);
    HHVM_FE(str_word_count);
    HHVM_FE(levenshtein);
    HHVM_FE(similar_text);
    HHVM_FE(soundex);
    HHVM_FE(metaphone);

    HHVM_RC_INT(ENT_COMPAT, k_ENT_HTML_QUOTE_DOUBLE);
    HHVM_RC_INT(ENT_NOQUOTES, k_ENT_HTML_QUOTE_NONE);
    HHVM_RC_INT(ENT_QUOTES, k_ENT_QUOTES);
    HHVM_RC_INT(ENT_IGNORE, k_ENT_HTML_IGNORE_ERRORS);
    HHVM_RC_INT(ENT_SUBSTITUTE, k_ENT_HTML_SUBSTITUTE_ERRORS);
    HHVM_RC_INT(ENT_HTML401, k_ENT_HTML_DOC_HTML401);
    HHVM_RC_INT(ENT_XML1, k_ENT_HTML_DOC_XML1);
    HHVM_RC_INT(ENT_XHTML, k_ENT_HTML_DOC_XHTML);
    HHVM_RC_INT(ENT_HTML5, k_ENT_HTML_DOC_HTML5);
    HHVM_RC_INT(ENT_FB_UTF8, k_ENT_FB_UTF8);
    HHVM_RC_INT(ENT_FB_UTF8_ONLY, k_ENT_FB_UTF8_ONLY);

    HHVM_RC_INT(HTML_SPECIALCHARS, k_HTML_SPECIALCHARS);
    HHVM_RC_INT(HTML_ENTITIES, k_HTML_ENTITIES);

    HHVM_RC_INT(STR_PAD_LEFT, k_STR_PAD_LEFT);
    HHVM_RC_INT(STR_PAD_RIGHT, k_STR_PAD_RIGHT);
    HHVM_RC_INT(STR_PAD_BOTH, k_STR_PAD_BOTH);

    HHVM_RC_INT_SAME(LC_CTYPE);
    HHVM_RC_INT_SAME(LC_NUMERIC);
    HHVM_RC_INT_SAME(LC_TIME);
    HHVM_RC_INT_SAME(LC_COLLATE);
    HHVM_RC_INT_SAME(LC_MONETARY);
    HHVM_RC_INT_SAME(LC_ALL);
#ifdef LC_MESSAGES
    HHVM_RC_INT_SAME(LC_MESSAGES);
#endif

#ifdef YESEXPR
    HHVM_RC_INT_SAME(YESEXPR);
#endif
#ifdef NOEXPR
    HHVM_RC_INT_SAME(NOEXPR);
#endif
    HHVM_RC_INT(CHAR_MAX, std::numeric_limits<char>::max());

    HHVM_RC_STR(HPHP_TRIM_CHARLIST, k_HPHP_TRIM_CHARLIST);

#ifdef ABDAY_1
    HHVM_RC_INT_SAME(ABDAY_1);
    HHVM_RC_INT_SAME(ABDAY_2);
    HHVM_RC_INT_SAME(ABDAY_3);
    HHVM_RC_INT_SAME(ABDAY_4);
    HHVM_RC_INT_SAME(ABDAY_5);
    HHVM_RC_INT_SAME(ABDAY_6);
    HHVM_RC_INT_SAME(ABDAY_7);
#endif
#ifdef DAY_1
    HHVM_RC_INT_SAME(DAY_1);
    HHVM_RC_INT_SAME(DAY_2);
    HHVM_RC_INT_SAME(DAY_3);
    HHVM_RC_INT_SAME(DAY_4);
    HHVM_RC_INT_SAME(DAY_5);
    HHVM_RC_INT_SAME(DAY_6);
    HHVM_RC_INT_SAME(DAY_7);
#endif
#ifdef ABMON_1
    HHVM_RC_INT_SAME(ABMON_1);
    HHVM_RC_INT_SAME(ABMON_2);
    HHVM_RC_INT_SAME(ABMON_3);
    HHVM_RC_INT_SAME(ABMON_4);
    HHVM_RC_INT_SAME(ABMON_5);
    HHVM_RC_INT_SAME(ABMON_6);
    HHVM_RC_INT_SAME(ABMON_7);
    HHVM_RC_INT_SAME(ABMON_8);
    HHVM_RC_INT_SAME(ABMON_9);
    HHVM_RC_INT_SAME(ABMON_10);
    HHVM_RC_INT_SAME(ABMON_11);
    HHVM_RC_INT_SAME(ABMON_12);
#endif
#ifdef MON_1
    HHVM_RC_INT_SAME(MON_1);
    HHVM_RC_INT_SAME(MON_2);
    HHVM_RC_INT_SAME(MON_3);
    HHVM_RC_INT_SAME(MON_4);
    HHVM_RC_INT_SAME(MON_5);
    HHVM_RC_INT_SAME(MON_6);
    HHVM_RC_INT_SAME(MON_7);
    HHVM_RC_INT_SAME(MON_8);
    HHVM_RC_INT_SAME(MON_9);
    HHVM_RC_INT_SAME(MON_10);
    HHVM_RC_INT_SAME(MON_11);
    HHVM_RC_INT_SAME(MON_12);
#endif

    // These are ostensibly bools,
    // but for historical reasons are expressed as ints
    HHVM_RC_INT(CRYPT_BLOWFISH, 1);
    HHVM_RC_INT(CRYPT_EXT_DES, 0);
    HHVM_RC_INT(CRYPT_MD5, 1);
    HHVM_RC_INT(CRYPT_STD_DES, 1);

    HHVM_RC_INT(CRYPT_SALT_LENGTH, 12);

    loadSystemlib();
  }
} s_string_extension;

}
