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

#ifndef incl_HPHP_PREG_H_
#define incl_HPHP_PREG_H_

#include "hphp/runtime/base/type-string.h"

#include <cstdint>
#include <cstddef>

#define PREG_PATTERN_ORDER          1
#define PREG_SET_ORDER              2
#define PREG_OFFSET_CAPTURE         (1<<8)

#define PREG_SPLIT_NO_EMPTY         (1<<0)
#define PREG_SPLIT_DELIM_CAPTURE    (1<<1)
#define PREG_SPLIT_OFFSET_CAPTURE   (1<<2)

#define PREG_REPLACE_EVAL           (1<<0)

#define PREG_GREP_INVERT            (1<<0)

enum {
  PHP_PCRE_NO_ERROR = 0,
  PHP_PCRE_INTERNAL_ERROR,
  PHP_PCRE_BACKTRACK_LIMIT_ERROR,
  PHP_PCRE_RECURSION_LIMIT_ERROR,
  PHP_PCRE_BAD_UTF8_ERROR,
  PHP_PCRE_BAD_UTF8_OFFSET_ERROR
};

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Array;
struct Variant;

struct PCREglobals {
  // pcre ini_settings
  int64_t m_preg_backtrace_limit;
  int64_t m_preg_recursion_limit;
};

Variant preg_grep(const String& pattern, const Array& input, int flags = 0);

Variant preg_match(const String& pattern, const String& subject,
                   Variant& matches,
                   int flags = 0, int offset = 0);

Variant preg_match(const String& pattern,
                   const String& subject,
                   int flags = 0,
                   int offset = 0);

Variant preg_match_all(const String& pattern, const String& subject,
                       Variant& matches,
                       int flags = 0, int offset = 0);

Variant preg_match_all(const String& pattern, const String& subject,
                       int flags = 0, int offset = 0);

Variant preg_replace_impl(const Variant& pattern, const Variant& replacement,
                          const Variant& subject, int limit, Variant& count,
                          bool is_callable, bool is_filter);
int preg_replace(Variant& result,
                 const Variant& pattern,
                 const Variant& replacement,
                 const Variant& subject,
                 int limit = -1);
int preg_replace_callback(Variant& result,
                          const Variant& pattern,
                          const Variant& callback,
                          const Variant& subject,
                          int limit = -1);
int preg_filter(Variant& result,
                const Variant& pattern,
                const Variant& replacement,
                const Variant& subject,
                int limit = -1);

Variant preg_split(const String& pattern,
                   const String& subject,
                   int limit = -1,
                   int flags = 0);
String preg_quote(const String& str, const String& delimiter = null_string);
Variant php_split(const String& spliton, const String& str, int count,
                  bool icase);

int preg_last_error();

size_t preg_pcre_cache_size();

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PREG_H__
