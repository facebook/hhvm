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

#ifndef incl_HPHP_STRING_UTIL_H_
#define incl_HPHP_STRING_UTIL_H_

#include "hphp/runtime/base/type-string.h"

#include "hphp/util/assertions.h"

#include <cstdint>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Array;
struct Variant;

extern const StaticString k_HPHP_TRIM_CHARLIST;

/**
 * Utility string functions. These are mostly wrappers around zend/ string
 * functions, but in a safe and convenient form.
 */
class StringUtil {
public:
  enum class PadType {
    Left = 0,
    Right = 1,
    Both = 2
  };

  #define QUOTE_STYLES    \
    /* Use high values to avoid conflicts with currently unimplemented PHP ENT_
     * constants and for possible future new constants in PHP */  \
    QS(FBUtf8Only, 65536) \
    QS(FBUtf8, 32768)     \
    /* Order of the fields matters here if we're
     * matching on what flags are set */  \
    QS(Ignore, 4) /* k_ENT_IGNORE:   silently discard invalid chars */ \
    QS(Both, 3)   /* k_ENT_QUOTES:   escape both double and single quotes */  \
    QS(Double, 2) /* k_ENT_COMPAT:   escape double quotes only */   \
    QS(No, 0)     /* k_ENT_NOQUOTES: leave all quotes alone */  \

  #define QS(STYLE, VAL) STYLE = (VAL),
  enum class QuoteStyle {
    QUOTE_STYLES
  };
  #undef QS

  static bool is_set(int64_t flags, QuoteStyle qs) {
    auto as_int = static_cast<int64_t>(qs);
    return (as_int & flags) == as_int;
  }

  static QuoteStyle toQuoteStyle(int64_t flags) {
    #define QS(STYLE, VAL)  \
      if (is_set(flags, QuoteStyle::STYLE)) { return QuoteStyle::STYLE; }

    QUOTE_STYLES

    #undef QS
    not_reached();
  }

  static int64_t toQuoteStyleBitmask(int64_t flags) {
    int64_t bitmask = 0L;

    #define QS(STYLE, VAL)  \
      if (is_set(flags, QuoteStyle::STYLE)) { \
        bitmask |= static_cast<int64_t>(QuoteStyle::STYLE); \
      }

    QUOTE_STYLES

    return bitmask;
    #undef QS
    not_reached();
  }

public:
  /**
   * Manipulations. Note, all these functions will create a new string than
   * modifying input, although names of these functions sound like mutating.
   */
  static String Pad(const String& input, int final_length,
                    const String& pad_string = " ",
                    PadType type = PadType::Right);
  static String StripHTMLTags(const String& input,
                              const String& allowable_tags = "");

  /**
   * Split/joins.
   */
  static Variant Explode(const String& input, const String& delimiter,
                         int limit = 0x7FFFFFFF);
  static String  Implode(const Variant& items, const String& delim); // == Join()
  static Variant Split(const String& str, int split_length = 1);
  static Variant ChunkSplit(
    const String& body, int chunklen = 76,
    const String& end = "\r\n"); // for email (rfc822/2822)

  /**
   * Encoding/decoding.
   */
  static String HtmlEncode(const String& input, QuoteStyle quoteStyle,
                           const char *charset, bool dEncode, bool htmlEnt);
  static String HtmlEncode(const String& input, const int64_t qsBitmask,
                           const char *charset, bool dEncode, bool htmlEnt);
  static String HtmlEncodeExtra(const String& input, QuoteStyle quoteStyle,
                                const char *charset, bool nbsp, Array extra);
  static String HtmlDecode(const String& input, QuoteStyle quoteStyle,
                           const char *charset, bool all);
  static String QuotedPrintableEncode(const String& input);
  static String QuotedPrintableDecode(const String& input);
  static String UUEncode(const String& input);
  static String UUDecode(const String& input);
  static String Base64Encode(const String& input);
  static String Base64Decode(const String& input, bool strict = false);
  static String UrlEncode(const String& input, bool encodePlus = true);
  static String UrlDecode(const String& input, bool decodePlus = true);

  /**
   * Formatting.
   */
  static String MoneyFormat(const char *format, double value);

  /**
   * Hashing
   */
  static String Translate(const String& input, const String& from,
                          const String& to);
  static String ROT13(const String& input);
  static int64_t CRC32(const String& input);
  static String Crypt(const String& input, const char *salt = "");
  static String MD5(const String& input, bool raw = false);
  static String SHA1(const String& input, bool raw = false);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_STRING_UTIL_H_
