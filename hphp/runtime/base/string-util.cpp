/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/string-util.h"
#include <algorithm>
#include <vector>
#include "hphp/zend/zend-html.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/container-functions.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// manipulations

String StringUtil::Pad(const String& input, int final_length,
                       const String& pad_string /* = " " */,
                       PadType type /* = PadType::Right */) {
  int len = input.size();
  char *ret = string_pad(input.data(), len, final_length, pad_string.data(),
                         pad_string.size(), static_cast<int>(type));
  if (ret) return String(ret, len, AttachString);
  return String();
}

String StringUtil::StripHTMLTags(const String& input,
                                 const String& allowable_tags /* = "" */) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_strip_tags(input.data(), len, allowable_tags.data(),
                                allowable_tags.size(), false);
  return String(ret, len, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
// splits/joins

Variant StringUtil::Explode(const String& input, const String& delimiter,
                            int limit /* = 0x7FFFFFFF */) {
  if (delimiter.empty()) {
    throw_invalid_argument("delimiter: (empty)");
    return false;
  }

  Array ret(Array::Create());

  if (input.empty()) {
    if (limit >= 0) {
      ret.append("");
    }
    return ret;
  }

  if (limit > 1) {
    int pos = input.find(delimiter);
    if (pos < 0) {
      ret.append(input);
    } else {
      int len = delimiter.size();
      int pos0 = 0;
      do {
        ret.append(input.substr(pos0, pos - pos0));
        pos += len;
        pos0 = pos;
      } while ((pos = input.find(delimiter, pos)) >= 0 && --limit > 1);

      if (pos0 <= input.size()) {
        ret.append(input.substr(pos0));
      }
    }
  } else if (limit < 0) {
    int pos = input.find(delimiter);
    if (pos >= 0) {
      std::vector<int> positions;
      int len = delimiter.size();
      int pos0 = 0;
      int found = 0;
      do {
        positions.push_back(pos0);
        positions.push_back(pos - pos0);
        pos += len;
        pos0 = pos;
        found++;
      } while ((pos = input.find(delimiter, pos)) >= 0);

      if (pos0 <= input.size()) {
        positions.push_back(pos0);
        positions.push_back(input.size() - pos0);
        found++;
      }
      int iMax = (found + limit) << 1;
      for (int i = 0; i < iMax; i += 2) {
        ret.append(input.substr(positions[i], positions[i+1]));
      }
    } // else we have negative limit and delimiter not found
  } else {
    ret.append(input);
  }

  return ret;
}

String StringUtil::Implode(const Variant& items, const String& delim) {
  if (!isContainer(items)) {
    throw_param_is_not_container();
  }
  int size = getContainerSize(items);
  if (size == 0) return "";

  String* sitems = (String*)smart_malloc(size * sizeof(String));
  int len = 0;
  int lenDelim = delim.size();
  int i = 0;
  for (ArrayIter iter(items); iter; ++iter) {
    new (&sitems[i]) String(iter.second().toString());
    len += sitems[i].size() + lenDelim;
    i++;
  }
  len -= lenDelim; // always one delimiter less than count of items
  assert(i == size);

  String s = String(len, ReserveString);
  char *buffer = s.bufferSlice().ptr;
  const char *sdelim = delim.data();
  char *p = buffer;
  for (int i = 0; i < size; i++) {
    String &item = sitems[i];
    if (i && lenDelim) {
      memcpy(p, sdelim, lenDelim);
      p += lenDelim;
    }
    int lenItem = item.size();
    if (lenItem) {
      memcpy(p, item.data(), lenItem);
      p += lenItem;
    }
    sitems[i].~String();
  }
  smart_free(sitems);
  assert(p - buffer == len);
  return s.setSize(len);
}

Variant StringUtil::Split(const String& str, int split_length /* = 1 */) {
  if (split_length <= 0) {
    throw_invalid_argument(
      "The length of each segment must be greater than zero"
    );
    return false;
  }

  Array ret;
  int len = str.size();
  if (split_length >= len) {
    ret.append(str);
  } else {
    for (int i = 0; i < len; i += split_length) {
      ret.append(str.substr(i, split_length));
    }
  }
  return ret;
}

Variant StringUtil::ChunkSplit(const String& body, int chunklen /* = 76 */,
                               const String& end /* = "\r\n" */) {
  if (chunklen <= 0) {
    throw_invalid_argument("chunklen: (non-positive)");
    return false;
  }

  String ret;
  int len = body.size();
  if (chunklen >= len) {
    ret = body;
    ret += end;
  } else {
    char *chunked = string_chunk_split(body.data(), len, end.c_str(),
                                       end.size(), chunklen);
    return String(chunked, len, AttachString);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// encoding/decoding

String StringUtil::HtmlEncode(const String& input, QuoteStyle quoteStyle,
                              const char *charset, bool dEncode, bool htmlEnt) {
  return HtmlEncode(input, static_cast<int64_t>(quoteStyle),
                    charset, dEncode, htmlEnt);
}

String StringUtil::HtmlEncode(const String& input, const int64_t qsBitmask,
                              const char *charset, bool dEncode, bool htmlEnt) {
  if (input.empty()) return input;

  assert(charset);
  bool utf8 = true;
  if (strcasecmp(charset, "ISO-8859-1") == 0) {
    utf8 = false;
  } else if (strcasecmp(charset, "UTF-8")) {
    throw NotImplementedException(charset);
  }

  int len = input.size();
  char *ret = string_html_encode(input.data(), len,
                                 qsBitmask, utf8, dEncode, htmlEnt);
  if (!ret) {
    return empty_string;
  }
  return String(ret, len, AttachString);
}

#define A1(v, ch) ((v)|((ch) & 64 ? 0 : 1uLL<<((ch)&63)))
#define A2(v, ch) ((v)|((ch) & 64 ? 1uLL<<((ch)&63) : 0))

static const AsciiMap mapNoQuotes = {
  {   A1(A1(A1(A1(A1(A1(0, '<'), '>'), '&'), '{'), '}'), '@'),
      A2(A2(A2(A2(A2(A2(0, '<'), '>'), '&'), '{'), '}'), '@') }
};

static const AsciiMap mapDoubleQuotes = {
  {   A1(A1(A1(A1(A1(A1(A1(0, '<'), '>'), '&'), '{'), '}'), '@'), '"'),
      A2(A2(A2(A2(A2(A2(A2(0, '<'), '>'), '&'), '{'), '}'), '@'), '"') }
};

static const AsciiMap mapBothQuotes = {
  { A1(A1(A1(A1(A1(A1(A1(A1(0, '<'), '>'), '&'), '{'), '}'), '@'), '"'), '\''),
    A2(A2(A2(A2(A2(A2(A2(A2(0, '<'), '>'), '&'), '{'), '}'), '@'), '"'), '\'') }
};

static const AsciiMap mapNothing = {};

String StringUtil::HtmlEncodeExtra(const String& input, QuoteStyle quoteStyle,
                                   const char *charset, bool nbsp,
                                   Array extra) {
  if (input.empty()) return input;

  assert(charset);
  int flags = STRING_HTML_ENCODE_UTF8;
  if (nbsp) {
    flags |= STRING_HTML_ENCODE_NBSP;
  }
  if (RuntimeOption::Utf8izeReplace) {
    flags |= STRING_HTML_ENCODE_UTF8IZE_REPLACE;
  }
  if (!*charset || strcasecmp(charset, "UTF-8") == 0) {
  } else if (strcasecmp(charset, "ISO-8859-1") == 0) {
    flags &= ~STRING_HTML_ENCODE_UTF8;
  } else {
    throw NotImplementedException(charset);
  }

  const AsciiMap *am;
  AsciiMap tmp;

  switch (quoteStyle) {
    case QuoteStyle::FBUtf8Only:
      am = &mapNothing;
      flags |= STRING_HTML_ENCODE_HIGH;
      break;
    case QuoteStyle::FBUtf8:
      am = &mapBothQuotes;
      flags |= STRING_HTML_ENCODE_HIGH;
      break;
    case QuoteStyle::Both:
      am = &mapBothQuotes;
      break;
    case QuoteStyle::Double:
      am = &mapDoubleQuotes;
      break;
    case QuoteStyle::No:
      am = &mapNoQuotes;
      break;
    default:
      am = &mapNothing;
      raise_error("Unknown quote style: %d", (int)quoteStyle);
  }

  if (quoteStyle != QuoteStyle::FBUtf8Only && extra.toBoolean()) {
    tmp = *am;
    am = &tmp;
    for (ArrayIter iter(extra); iter; ++iter) {
      String item = iter.second().toString();
      char c = item.data()[0];
      tmp.map[c & 64 ? 1 : 0] |= 1uLL << (c & 63);
    }
  }

  int len = input.size();
  char *ret = string_html_encode_extra(input.data(), len,
                                       (StringHtmlEncoding)flags, am);
  if (!ret) {
    raise_error("HtmlEncode called on too large input (%d)", len);
  }
  return String(ret, len, AttachString);
}

String StringUtil::HtmlDecode(const String& input, QuoteStyle quoteStyle,
                              const char *charset, bool all) {
  if (input.empty()) return input;

  assert(charset);

  int len = input.size();
  char *ret = string_html_decode(input.data(), len,
                                 quoteStyle != QuoteStyle::No,
                                 quoteStyle == QuoteStyle::Both,
                                 charset, all);
  if (!ret) {
    // null iff charset was not recognized
    throw NotImplementedException(charset);
    // (charset is not null, see assertion above)
  }

  return String(ret, len, AttachString);
}

String StringUtil::QuotedPrintableEncode(const String& input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_quoted_printable_encode(input.data(), len);
  return String(ret, len, AttachString);
}

String StringUtil::QuotedPrintableDecode(const String& input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_quoted_printable_decode(input.data(), len, false);
  return String(ret, len, AttachString);
}

String StringUtil::UUEncode(const String& input) {
  if (input.empty()) return input;

  int len;
  char *encoded = string_uuencode(input.data(), input.size(), len);
  return String(encoded, len, AttachString);
}

String StringUtil::UUDecode(const String& input) {
  if (!input.empty()) {
    int len;
    char *decoded = string_uudecode(input.data(), input.size(), len);
    if (decoded) {
      return String(decoded, len, AttachString);
    }
  }
  return String();
}

String StringUtil::Base64Encode(const String& input) {
  int len = input.size();
  char *ret = string_base64_encode(input.data(), len);
  return String(ret, len, AttachString);
}

String StringUtil::Base64Decode(const String& input,
                                bool strict /* = false */) {
  int len = input.size();
  char *ret = string_base64_decode(input.data(), len, strict);
  return String(ret, len, AttachString);
}

String StringUtil::UrlEncode(const String& input,
                             bool encodePlus /* = true */) {
  int len = input.size();
  char *ret;
  if (encodePlus) {
    ret = url_encode(input.data(), len);
  } else {
    ret = url_raw_encode(input.data(), len);
  }
  return String(ret, len, AttachString);
}

String StringUtil::UrlDecode(const String& input,
                             bool decodePlus /* = true */) {
  int len = input.size();
  char *ret;
  if (decodePlus) {
    ret = url_decode(input.data(), len);
  } else {
    ret = url_raw_decode(input.data(), len);
  }
  return String(ret, len, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
// formatting

String StringUtil::MoneyFormat(const char *format, double value) {
  assert(format);
  char *formatted = string_money_format(format, value);
  return formatted ? String(formatted, AttachString) : String();
}

///////////////////////////////////////////////////////////////////////////////
// hashing

String StringUtil::Translate(const String& input, const String& from,
                             const String& to) {
  if (input.empty()) return input;

  int len = input.size();
  String retstr(len, ReserveString);
  char *ret = retstr.bufferSlice().ptr;
  memcpy(ret, input.data(), len);
  auto trlen = std::min(from.size(), to.size());
  string_translate(ret, len, from.data(), to.data(), trlen);
  return retstr.setSize(len);
}

String StringUtil::ROT13(const String& input) {
  if (input.empty()) return input;
  return String(string_rot13(input.data(), input.size()),
                input.size(), AttachString);
}

int64_t StringUtil::CRC32(const String& input) {
  return string_crc32(input.data(), input.size());
}

String StringUtil::Crypt(const String& input, const char *salt /* = "" */) {
  return String(string_crypt(input.c_str(), salt), AttachString);
}

String StringUtil::MD5(const String& input, bool raw /* = false */) {
  Md5Digest md5(input.data(), input.size());
  auto const rawLen = sizeof(md5.digest);
  if (raw) return String((char*)md5.digest, rawLen, CopyString);
  auto const hexLen = rawLen * 2;
  String hex(hexLen, ReserveString);
  string_bin2hex((char*)md5.digest, rawLen, hex.bufferSlice().ptr);
  return hex.setSize(hexLen);
}

String StringUtil::SHA1(const String& input, bool raw /* = false */) {
  int len;
  char *ret = string_sha1(input.data(), input.size(), raw, len);
  return String(ret, len, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
}
