/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/zend/zend-html.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/util/bstring.h"
#include <langinfo.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// transformations and manipulations

extern const HPHP::StaticString k_HPHP_TRIM_CHARLIST;

constexpr int64_t k_ENT_HTML_QUOTE_NONE = 0;
constexpr int64_t k_ENT_HTML_QUOTE_SINGLE = 1;
constexpr int64_t k_ENT_HTML_QUOTE_DOUBLE = 2;
constexpr int64_t k_ENT_HTML_IGNORE_ERRORS = 4;
constexpr int64_t k_ENT_HTML_SUBSTITUTE_ERRORS = 8;
constexpr int64_t k_ENT_HTML_DOC_TYPE_MASK = (16|32);
constexpr int64_t k_ENT_HTML_DOC_HTML401 = 0;
constexpr int64_t k_ENT_HTML_DOC_XML1 = 16;
constexpr int64_t k_ENT_HTML_DOC_XHTML = 32;
constexpr int64_t k_ENT_HTML_DOC_HTML5 = (16|32);
constexpr int64_t k_ENT_HTML_SUBSTITUTE_DISALLOWED_CHARS = 128;
constexpr int64_t k_ENT_FB_UTF8 = 32768;
constexpr int64_t k_ENT_FB_UTF8_ONLY = 65536;

constexpr int64_t k_ENT_QUOTES = k_ENT_HTML_QUOTE_DOUBLE |
                                 k_ENT_HTML_QUOTE_SINGLE;

constexpr int64_t k_HTML_SPECIALCHARS = 0;
constexpr int64_t k_HTML_ENTITIES = 1;

constexpr int64_t k_STR_PAD_LEFT  = 0;
constexpr int64_t k_STR_PAD_RIGHT = 1;
constexpr int64_t k_STR_PAD_BOTH  = 2;

String HHVM_FUNCTION(addcslashes,
                     const String& str,
                     const String& charlist);
String HHVM_FUNCTION(addslashes,
                     const String& str);
String HHVM_FUNCTION(bin2hex,
                     const String& str);
Variant HHVM_FUNCTION(hex2bin,
                      const String& str);
String HHVM_FUNCTION(strrev,
                     const String& str);
String HHVM_FUNCTION(strtolower,
                     const String& str);
String HHVM_FUNCTION(strtoupper,
                     const String& str);
String HHVM_FUNCTION(trim,
                     const String& str,
                     const String& charlist = k_HPHP_TRIM_CHARLIST);
String HHVM_FUNCTION(ltrim,
                     const String& str,
                     const String& charlist = k_HPHP_TRIM_CHARLIST);
String HHVM_FUNCTION(rtrim,
                     const String& str,
                     const String& charlist = k_HPHP_TRIM_CHARLIST);
Variant HHVM_FUNCTION(explode,
                      const String& delimiter,
                      const String& str,
                      int64_t limit = k_PHP_INT_MAX);
TypedValue HHVM_FUNCTION(str_replace,
                         const Variant& search,
                         const Variant& replace,
                         const Variant& subject);
TypedValue HHVM_FUNCTION(str_ireplace,
                         const Variant& search,
                         const Variant& replace,
                         const Variant& subject);
String HHVM_FUNCTION(str_repeat,
                     const String& input,
                     int64_t multiplier);

///////////////////////////////////////////////////////////////////////////////
// encoding/decoding

String HHVM_FUNCTION(htmlentities,
                     const String& str,
                     int64_t quote_style = k_ENT_HTML_QUOTE_DOUBLE,
                     const String& charset = "UTF-8",
                     bool double_encode = true);
String HHVM_FUNCTION(md5,
                     const String& str,
                     bool raw_output = false);
StringRet HHVM_FUNCTION(sha1,
                     const String& str,
                     bool raw_output = false);
Variant HHVM_FUNCTION(strtr,
                      const String& str,
                      const Variant& from,
                      const Variant& to = uninit_variant);

///////////////////////////////////////////////////////////////////////////////
// input/output

Variant HHVM_FUNCTION(sscanf,
                      const String& str,
                      const String& format);

///////////////////////////////////////////////////////////////////////////////
// analysis

bool str_contains_any_of(const String& haystack, const String& char_list);

TypedValue HHVM_FUNCTION(strpos,
                         const String& haystack,
                         const Variant& needle,
                         int64_t offset = 0);
TypedValue HHVM_FUNCTION(stripos,
                         const String& haystack,
                         const Variant& needle,
                         int64_t offset = 0);
TypedValue HHVM_FUNCTION(strrpos,
                        const String& haystack,
                        const Variant& needle,
                        int64_t offset = 0);
TypedValue HHVM_FUNCTION(strripos,
                        const String& haystack,
                        const Variant& needle,
                        int64_t offset = 0);

///////////////////////////////////////////////////////////////////////////////

}
