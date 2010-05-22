/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/string_util.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_html.h>
#include <runtime/base/zend/zend_url.h>
#include <runtime/base/runtime_error.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// manipulations

String StringUtil::ToLower(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_to_lower(input.data(), len);
  return String(ret, len, AttachString);
}

String StringUtil::ToUpper(CStrRef input, ToUpperType type /*= ToUpperAll */) {
  if (input.empty()) return input;

  int len = input.size();
  char *ret = NULL;
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
    ASSERT(false);
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

  Array ret;
  int pos = input.find(delimiter);
  if (limit >= 0) {
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
  } else if (pos >= 0) {
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

  } // else we have negative limit and delimiter not found, returning empty arr

  return ret;
}

String StringUtil::Implode(CArrRef items, CStrRef delim) {
  int size = items.size();
  if (size == 0) return "";

  vector<String> sitems;
  sitems.reserve(size);
  int len = 0;
  int lenDelim = delim.size();
  for (ArrayIter iter(items); iter; ++iter) {
    String item = iter.second().toString();
    sitems.push_back(item);
    len += lenDelim;
    len += item.size();
  }
  len -= lenDelim; // always one delimiter less than count of items
  ASSERT((int)sitems.size() == size);

  char *buffer = (char *)malloc(len + 1);
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
  }
  *p = '\0';
  ASSERT(p - buffer == len);
  return String(buffer, len, AttachString);
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
    char *chunked = string_chunk_split(body.data(), len, end, end.size(),
                                       chunklen);
    return String(chunked, len, AttachString);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// encoding/decoding

String StringUtil::CEncode(CStrRef input, CStrRef charlist) {
  String chars = charlist;
  if (chars.isNull()) {
    chars = String("\\\x00\x01..\x1f\x7f..\xff", 10, AttachLiteral);
  }
  if (input.empty() || chars.empty()) return input;
  int len = input.size();
  char *ret = string_addcslashes(input, len, chars.data(), chars.size());
  return String(ret, len, AttachString);
}

String StringUtil::CDecode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_stripcslashes(input, len);
  return String(ret, len, AttachString);
}

String StringUtil::SqlEncode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_addslashes(input, len);
  return String(ret, len, AttachString);
}

String StringUtil::SqlDecode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_stripslashes(input, len);
  return String(ret, len, AttachString);
}

String StringUtil::RegExEncode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_quotemeta(input, len);
  return String(ret, len, AttachString);
}

String StringUtil::HtmlEncode(CStrRef input, QuoteStyle quoteStyle) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_html_encode(input, len, quoteStyle != NoQuotes,
                                 quoteStyle == BothQuotes);
  return String(ret, len, AttachString);
}

String StringUtil::HtmlDecode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_html_decode(input, len);
  return String(ret, len, AttachString);
}

String StringUtil::QuotedPrintableEncode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_quoted_printable_encode(input, len);
  return String(ret, len, AttachString);
}

String StringUtil::QuotedPrintableDecode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_quoted_printable_decode(input, len);
  return String(ret, len, AttachString);
}

String StringUtil::HexEncode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_bin2hex(input, len);
  return String(ret, len, AttachString);
}

String StringUtil::HexDecode(CStrRef input) {
  if (input.empty()) return input;
  int len = input.size();
  char *ret = string_hex2bin(input, len);
  return String(ret, len, AttachString);
}

String StringUtil::UUEncode(CStrRef input) {
  if (input.empty()) return input;

  int len;
  char *encoded = string_uuencode(input, input.size(), len);
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
  ASSERT(format);
  char *formatted = string_money_format(format, value);
  return formatted ? String(formatted, AttachString) : String();
}

///////////////////////////////////////////////////////////////////////////////
// hashing

String StringUtil::Translate(CStrRef input, CStrRef from, CStrRef to) {
  if (input.empty()) return input;

  int len = input.size();
  char *ret = (char *)malloc(len + 1);
  memcpy(ret, input, len);
  ret[len] = '\0';
  int fromSize = from.size();
  int toSize = to.size();
  int trlen = (fromSize < toSize) ? fromSize : toSize;
  string_translate(ret, len, from, to, trlen);
  return String(ret, len, AttachString);
}

String StringUtil::ROT13(CStrRef input) {
  if (input.empty()) return input;
  return String(string_rot13(input, input.size()), input.size(), AttachString);
}

int64 StringUtil::CRC32(CStrRef input) {
  return string_crc32(input, input.size());
}

String StringUtil::Crypt(CStrRef input, const char *salt /* = "" */) {
  return String(string_crypt(input, salt), AttachString);
}

String StringUtil::MD5(CStrRef input, bool raw /* = false */) {
  int len;
  char *ret = string_md5(input, input.size(), raw, len);
  return String(ret, len, AttachString);
}

String StringUtil::SHA1(CStrRef input, bool raw /* = false */) {
  int len;
  char *ret = string_sha1(input, input.size(), raw, len);
  return String(ret, len, AttachString);
}

void StringUtil::InitLiteralStrings(StaticString literalStrings[],
                                    int nliteralStrings,
                                    const char *literalStringBuf,
                                    int literalStringBufSize,
                                    const char *literalStringLen,
                                    int literalStringLenSize) {
  int bufSize = literalStringBufSize;
  int lenSize = literalStringLenSize;
  static char *uncompressedBuf; // permanently allocated
  char *uncompressedLen;
  if (uncompressedBuf) {
    throw Exception("StringUtil::InitLiteralStrings called twice");
  }
  uncompressedBuf = gzdecode(literalStringBuf, bufSize);
  if (uncompressedBuf == NULL) {
    throw Exception("Bad literalStringBuf %p", literalStringBuf);
  }
  uncompressedLen = gzdecode(literalStringLen, lenSize);
  if (uncompressedLen == NULL) {
    throw Exception("Bad literalStringLen %p", literalStringLen);
  }

  const char *pb = uncompressedBuf;
  const char *pl = uncompressedLen;
  const char *endBuf = pb + bufSize;
  const char *endLen = pl + lenSize;

  for (int i = 0; i < nliteralStrings; i++) {
    int size;
    memcpy(&size, pl, sizeof(size));
    literalStrings[i] = StaticString(pb, size);
    pb += size;
    ASSERT(*pb == '\0');
    pb++;
    pl += sizeof(size);
  }
  if (pb != endBuf) {
    throw Exception("Bad literalStringBuf %p", literalStringBuf);
  }
  if (pl != endLen) {
    throw Exception("Bad literalStringLen %p", literalStringLen);
  }
  free(uncompressedLen);
}

///////////////////////////////////////////////////////////////////////////////
}
