/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/complex-types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

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

  enum class QuoteStyle {
    Double       = 2,  // k_ENT_COMPAT:   escape double quotes only
    Both         = 3,  // k_ENT_QUOTES:   escape both double and single quotes
    No           = 0,  // k_ENT_NOQUOTES: leave all quotes alone
    // Use high values to avoid conflicts with currently unimplemented PHP ENT_
    // constants and for possible future new constants in PHP
    FBUtf8       = 32768,
    FBUtf8Only   = 65536
  };

public:
  /**
   * Manipulations. Note, all these functions will create a new string than
   * modifying input, although names of these functions sound like mutating.
   */
  static String Pad(CStrRef input, int final_length,
                    CStrRef pad_string = " ", PadType type = PadType::Right);
  static String StripHTMLTags(CStrRef input, CStrRef allowable_tags = "");

  /**
   * Split/joins.
   */
  static Variant Explode(CStrRef input, CStrRef delimiter,
                         int limit = 0x7FFFFFFF);
  static String  Implode(CArrRef items, CStrRef delim); // == Join()
  static Variant Split(CStrRef str, int split_length = 1);
  static Variant ChunkSplit(CStrRef body, int chunklen = 76,
                            CStrRef end = "\r\n"); // for email (rfc822/2822)

  /**
   * Encoding/decoding.
   */
  static String HtmlEncode(CStrRef input, QuoteStyle quoteStyle,
                           const char *charset, bool nbsp);
  static String HtmlEncodeExtra(CStrRef input, QuoteStyle quoteStyle,
                                const char *charset, bool nbsp, Array extra);
  static String HtmlDecode(CStrRef input, QuoteStyle quoteStyle,
                           const char *charset, bool all);
  static String QuotedPrintableEncode(CStrRef input);
  static String QuotedPrintableDecode(CStrRef input);
  static String UUEncode(CStrRef input);
  static String UUDecode(CStrRef input);
  static String Base64Encode(CStrRef input);
  static String Base64Decode(CStrRef input, bool strict = false);
  static String UrlEncode(CStrRef input, bool encodePlus = true);
  static String UrlDecode(CStrRef input, bool decodePlus = true);

  /**
   * Formatting.
   */
  static String MoneyFormat(const char *format, double value);

  /**
   * Hashing
   */
  static String Translate(CStrRef input, CStrRef from, CStrRef to);
  static String ROT13(CStrRef input);
  static int64_t CRC32(CStrRef input);
  static String Crypt(CStrRef input, const char *salt = "");
  static String MD5(CStrRef input, bool raw = false);
  static String SHA1(CStrRef input, bool raw = false);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_STRING_UTIL_H_
