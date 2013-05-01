/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "util/zend/zend_html.h"
#include <runtime/base/string_util.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_url.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/builtin_functions.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// manipulations

String StringUtil::ToLower(CStrRef input, ToLowerType type /*= ToLowerAll */) {
  if (input.empty()) return input;

  int len = input.size();
  char *ret = nullptr;
  switch (type) {
  case ToLowerAll:
    ret = string_to_lower(input.data(), len);
    break;
  case ToLowerFirst:
    ret = string_to_lower_first(input.data(), len);
    break;
  case ToLowerWords:
    ret = string_to_lower_words(input.data(), len);
    break;
  default:
    assert(false);
    break;
  }
  return String(ret, len, AttachString);
}

String StringUtil::ToUpper(CStrRef input, ToUpperType type /*= ToUpperAll */) {
  if (input.empty()) return input;

  int len = input.size();
  char *ret = nullptr;
  switch (type) {
  case ToUpperAll:
    ret = string_to_upper(input.data(), len);
    break;
  case ToUpperFirst:
    ret = string_to_upper_first(input.data(), len);
    break;
  case ToUpperWords:
    ret = string_to_upper_words(input.data(), len);
    break;
  default:
    assert(false);
    break;
  }
  return String(ret, len, AttachString);
}

String StringUtil::Trim(CStrRef input, TrimType type  /* = TrimBoth */,
                        CStrRef charlist /* = k_HPHP_TRIM_CHARLIST */) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_trim(input.data(), len,
                          charlist.data(), charlist.length(), type);
  if (!ret) {
      return input;
  }
  return String(ret, len, AttachString);
}

String StringUtil::Pad(CStrRef input, int final_length,
                       CStrRef pad_string /* = " " */,
                       PadType type /* = PadRight */) {
  int len = input.size();
  char *ret = string_pad(input.data(), len, final_length, pad_string.data(),
                         pad_string.size(), type);
  if (ret) return String(ret, len, AttachString);
  return String();
}

String StringUtil::Reverse(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  return String(string_reverse(input.data(), len), len, AttachString);
}

String StringUtil::Repeat(CStrRef input, int count) {
  if (count < 0) {
    raise_warning("Second argument has to be greater than or equal to 0");
    return String();
  }
  if (count == 0) {
    return "";
  }
  if (!input.empty()) {
    int len = input.size();
    char *ret = string_repeat(input.data(), len, count);
    if (ret) {
      return String(ret, len, AttachString);
    }
  }
  return input;
}

String StringUtil::Shuffle(CStrRef input) {
  if (!input.empty()) {
    int len = input.size();
    char *ret = string_shuffle(input.data(), len);
    if (ret) {
      return String(ret, len, AttachString);
    }
  }
  return input;
}

String StringUtil::StripHTMLTags(CStrRef input,
                                 CStrRef allowable_tags /* = "" */) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_strip_tags(input.data(), len, allowable_tags.data(),
                                allowable_tags.size());
  return String(ret, len, AttachString);
}

String StringUtil::WordWrap(CStrRef input, int width,
                            CStrRef wordbreak /* = "\n" */,
                            bool cut /* = false */) {
  if (!input.empty()) {
    int len = input.size();
    char *ret = string_wordwrap(input.data(), len, width, wordbreak.data(),
                                wordbreak.size(), cut);
    if (ret) {
      return String(ret, len, AttachString);
    }
    return String();
  }
  return input;
}

///////////////////////////////////////////////////////////////////////////////
// splits/joins

Variant StringUtil::Explode(CStrRef input, CStrRef delimiter,
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
      vector<int> positions;
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

String StringUtil::Implode(CArrRef items, CStrRef delim) {
  int size = items.size();
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
  char *buffer = s.mutableSlice().ptr;
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

Variant StringUtil::Split(CStrRef str, int split_length /* = 1 */) {
  if (split_length <= 0) {
    throw_invalid_argument("split_length: (non-positive)");
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

Variant StringUtil::ChunkSplit(CStrRef body, int chunklen /* = 76 */,
                               CStrRef end /* = "\r\n" */) {
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

String StringUtil::CEncode(CStrRef input, CStrRef charlist) {
  String chars = charlist;
  if (chars.isNull()) {
    chars = String("\\\x00\x01..\x1f\x7f..\xff", 10, CopyString);
  }
  if (input.empty() || chars.empty()) return input;
  int len = input.size();
  char *ret = string_addcslashes(input.c_str(), len, chars.data(),
                                 chars.size());
  return String(ret, len, AttachString);
}

String StringUtil::CDecode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_stripcslashes(input.c_str(), len);
  return String(ret, len, AttachString);
}

String StringUtil::SqlEncode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_addslashes(input.c_str(), len);
  return String(ret, len, AttachString);
}

String StringUtil::SqlDecode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_stripslashes(input.c_str(), len);
  return String(ret, len, AttachString);
}

String StringUtil::RegExEncode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_quotemeta(input.c_str(), len);
  return String(ret, len, AttachString);
}

String StringUtil::HtmlEncode(CStrRef input, QuoteStyle quoteStyle,
                              const char *charset, bool nbsp) {
  if (input.empty()) return input;

  assert(charset);
  bool utf8 = true;
  if (strcasecmp(charset, "ISO-8859-1") == 0) {
    utf8 = false;
  } else if (strcasecmp(charset, "UTF-8")) {
    throw NotImplementedException(charset);
  }

  int len = input.size();
  char *ret = string_html_encode(input.data(), len, quoteStyle != NoQuotes,
                                 quoteStyle == BothQuotes, utf8, nbsp);
  if (!ret) {
    raise_error("HtmlEncode called on too large input (%d)", len);
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

String StringUtil::HtmlEncodeExtra(CStrRef input, QuoteStyle quoteStyle,
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
    case FBUtf8Only:
      am = &mapNothing;
      flags |= STRING_HTML_ENCODE_HIGH;
      break;
    case FBUtf8:
      am = &mapBothQuotes;
      flags |= STRING_HTML_ENCODE_HIGH;
      break;
    case BothQuotes:
      am = &mapBothQuotes;
      break;
    case DoubleQuotes:
      am = &mapDoubleQuotes;
      break;
    case NoQuotes:
      am = &mapNoQuotes;
      break;
    default:
      am = &mapNothing;
      raise_error("Unknown quote style: %d", (int)quoteStyle);
  }

  if (quoteStyle != FBUtf8Only && extra.toBoolean()) {
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

String StringUtil::HtmlDecode(CStrRef input, QuoteStyle quoteStyle,
                              const char *charset, bool all) {
  if (input.empty()) return input;

  assert(charset);

  int len = input.size();
  char *ret = string_html_decode(input.data(), len, quoteStyle != NoQuotes,
                                 quoteStyle == BothQuotes, charset, all);
  if (!ret) {
    // null iff charset was not recognized
    throw NotImplementedException(charset);
    // (charset is not null, see assertion above)
  }

  return String(ret, len, AttachString);
}

String StringUtil::QuotedPrintableEncode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_quoted_printable_encode(input.data(), len);
  return String(ret, len, AttachString);
}

String StringUtil::QuotedPrintableDecode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_quoted_printable_decode(input.data(), len, false);
  return String(ret, len, AttachString);
}

String StringUtil::HexEncode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_bin2hex(input.data(), len);
  return String(ret, len, AttachString);
}

String StringUtil::HexDecode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_hex2bin(input.data(), len);
  return String(ret, len, AttachString);
}

String StringUtil::UUEncode(CStrRef input) {
  if (input.empty()) return input;

  int len;
  char *encoded = string_uuencode(input.data(), input.size(), len);
  return String(encoded, len, AttachString);
}

String StringUtil::UUDecode(CStrRef input) {
  if (!input.empty()) {
    int len;
    char *decoded = string_uudecode(input.data(), input.size(), len);
    if (decoded) {
      return String(decoded, len, AttachString);
    }
  }
  return String();
}

String StringUtil::Base64Encode(CStrRef input) {
  int len = input.size();
  char *ret = string_base64_encode(input.data(), len);
  return String(ret, len, AttachString);
}

String StringUtil::Base64Decode(CStrRef input, bool strict /* = false */) {
  int len = input.size();
  char *ret = string_base64_decode(input.data(), len, strict);
  return String(ret, len, AttachString);
}

String StringUtil::UrlEncode(CStrRef input, bool encodePlus /* = true */) {
  int len = input.size();
  char *ret;
  if (encodePlus) {
    ret = url_encode(input.data(), len);
  } else {
    ret = url_raw_encode(input.data(), len);
  }
  return String(ret, len, AttachString);
}

String StringUtil::UrlDecode(CStrRef input, bool decodePlus /* = true */) {
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

String StringUtil::Translate(CStrRef input, CStrRef from, CStrRef to) {
  if (input.empty()) return input;

  int len = input.size();
  String retstr(len, ReserveString);
  char *ret = retstr.mutableSlice().ptr;
  memcpy(ret, input.data(), len);
  auto trlen = std::min(from.size(), to.size());
  string_translate(ret, len, from.data(), to.data(), trlen);
  return retstr.setSize(len);
}

String StringUtil::ROT13(CStrRef input) {
  if (input.empty()) return input;
  return String(string_rot13(input.data(), input.size()),
                input.size(), AttachString);
}

int64_t StringUtil::CRC32(CStrRef input) {
  return string_crc32(input.data(), input.size());
}

String StringUtil::Crypt(CStrRef input, const char *salt /* = "" */) {
  return String(string_crypt(input.c_str(), salt), AttachString);
}

String StringUtil::MD5(CStrRef input, bool raw /* = false */) {
  int len;
  char *ret = string_md5(input.data(), input.size(), raw, len);
  return String(ret, len, AttachString);
}

String StringUtil::SHA1(CStrRef input, bool raw /* = false */) {
  int len;
  char *ret = string_sha1(input.data(), input.size(), raw, len);
  return String(ret, len, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
}
