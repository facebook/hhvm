/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/std/ext_std_misc.h"

#include "hphp/runtime/base/type-string.h"

#include "hphp/util/assertions.h"

#include <cstdint>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Array;
struct Variant;

extern const StaticString k_HPHP_TRIM_CHARLIST;

/**
 * Utility string functions. These are mostly wrappers around zend/ string
 * functions, but in a safe and convenient form.
 */
struct StringUtil {
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
    QS(Xhtml, 32) /* k_ENT_XHTML */ \
    QS(Xml1, 16)  /* k_ENT_XML1 */ \
    QS(Substitute, 8) /* k_ENT_SUBSTITUTE: replace invalid chars with FFFD */ \
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
  static OptString Pad(const OptString& input, int final_length,
                    const OptString& pad_string = " ",
                    PadType type = PadType::Right);
  static OptString StripHTMLTags(const OptString& input,
                              const OptString& allowable_tags = "");

  /**
   * Split/joins.
   */
  static Variant Explode(const OptString& input, const OptString& delimiter,
                         int64_t limit = k_PHP_INT_MAX);
    static OptString Implode(const Variant& items, const OptString& delim,
                          const bool checkIsContainer = true); // == Join()
  static Variant Split(const OptString& str, int64_t split_length = 1);
  static Variant ChunkSplit(
    const OptString& body, int chunklen = 76,
    const OptString& end = "\r\n"); // for email (rfc822/2822)

  /**
   * Encoding/decoding.
   */
  static OptString HtmlEncode(const OptString& input, QuoteStyle quoteStyle,
                           const char *charset, bool dEncode, bool htmlEnt);
  static OptString HtmlEncode(const OptString& input, const int64_t qsBitmask,
                           const char *charset, bool dEncode, bool htmlEnt);
  static OptString HtmlEncodeExtra(const OptString& input, QuoteStyle quoteStyle,
                                const char *charset, bool nbsp, Array extra);
  static OptString HtmlDecode(const OptString& input, QuoteStyle quoteStyle,
                           const char *charset, bool all);
  static OptString QuotedPrintableEncode(const OptString& input);
  static OptString QuotedPrintableDecode(const OptString& input);
  static OptString UUEncode(const OptString& input);
  static OptString UUDecode(const OptString& input);
  static OptString Base64Encode(const OptString& input);
  static OptString Base64Decode(const OptString& input, bool strict = false);
  static OptString UrlEncode(const OptString& input, bool encodePlus = true);
  static OptString UrlDecode(const OptString& input, bool decodePlus = true);

  /**
   * Determine if a string looks like a file URL. Does not check for validity.
   */
  static bool IsFileUrl(const OptString& input);

  /**
   * Determine if a string is a valid local file URL. If it is, the decoded
   * path part is returned. If it is not, an empty string is returned.
   */
  static OptString DecodeFileUrl(const OptString& input);

  /**
   * Formatting.
   */
  static OptString MoneyFormat(const char *format, double value);

  /**
   * Hashing
   */
  static OptString Translate(const OptString& input, const OptString& from,
                          const OptString& to);
  static OptString ROT13(const OptString& input);
  static OptString Crypt(const OptString& input, const char *salt = "");
  static OptString MD5(const OptString& input, bool raw = false);
  static OptString MD5(const char *data, uint32_t size, bool raw = false);
  static OptString SHA1(const OptString& input, bool raw = false);
};

size_t safe_address(size_t nmemb, size_t size, size_t offset);

///////////////////////////////////////////////////////////////////////////////
}
