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

OptString HHVM_FUNCTION(addcslashes,
                     const OptString& str,
                     const OptString& charlist);
OptString HHVM_FUNCTION(addslashes,
                     const OptString& str);
OptString HHVM_FUNCTION(bin2hex,
                     const OptString& str);
Variant HHVM_FUNCTION(hex2bin,
                      const OptString& str);
OptString HHVM_FUNCTION(strrev,
                     const OptString& str);
OptString HHVM_FUNCTION(strtolower,
                     const OptString& str);
OptString HHVM_FUNCTION(strtoupper,
                     const OptString& str);
OptString HHVM_FUNCTION(trim,
                     const OptString& str,
                     const OptString& charlist = k_HPHP_TRIM_CHARLIST);
OptString HHVM_FUNCTION(ltrim,
                     const OptString& str,
                     const OptString& charlist = k_HPHP_TRIM_CHARLIST);
OptString HHVM_FUNCTION(rtrim,
                     const OptString& str,
                     const OptString& charlist = k_HPHP_TRIM_CHARLIST);
Variant HHVM_FUNCTION(explode,
                      const OptString& delimiter,
                      const OptString& str,
                      int64_t limit = k_PHP_INT_MAX);
TypedValue HHVM_FUNCTION(str_replace,
                         const Variant& search,
                         const Variant& replace,
                         const Variant& subject);
TypedValue HHVM_FUNCTION(str_ireplace,
                         const Variant& search,
                         const Variant& replace,
                         const Variant& subject);
OptString HHVM_FUNCTION(str_repeat,
                     const OptString& input,
                     int64_t multiplier);

///////////////////////////////////////////////////////////////////////////////
// encoding/decoding

OptString HHVM_FUNCTION(htmlentities,
                     const OptString& str,
                     int64_t quote_style = k_ENT_HTML_QUOTE_DOUBLE,
                     const OptString& charset = "UTF-8",
                     bool double_encode = true);
OptString HHVM_FUNCTION(md5,
                     const OptString& str,
                     bool raw_output = false);
StringRet HHVM_FUNCTION(sha1,
                     const OptString& str,
                     bool raw_output = false);
Variant HHVM_FUNCTION(strtr,
                      const OptString& str,
                      const Variant& from,
                      const Variant& to = uninit_variant);

///////////////////////////////////////////////////////////////////////////////
// input/output

Variant HHVM_FUNCTION(sscanf,
                      const OptString& str,
                      const OptString& format);

///////////////////////////////////////////////////////////////////////////////
// analysis

bool str_contains_any_of(const OptString& haystack, const OptString& char_list);

TypedValue HHVM_FUNCTION(strpos,
                         const OptString& haystack,
                         const Variant& needle,
                         int64_t offset = 0);
TypedValue HHVM_FUNCTION(stripos,
                         const OptString& haystack,
                         const Variant& needle,
                         int64_t offset = 0);
TypedValue HHVM_FUNCTION(strrpos,
                        const OptString& haystack,
                        const Variant& needle,
                        int64_t offset = 0);
TypedValue HHVM_FUNCTION(strripos,
                        const OptString& haystack,
                        const Variant& needle,
                        int64_t offset = 0);

///////////////////////////////////////////////////////////////////////////////

}
