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

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/base/type-string.h"
#include <folly/EvictingCacheMap.h>

#include <cstdint>
#include <cstddef>
#include <pcre.h>

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

namespace {
  constexpr int kLocalCacheSize = 1024;
}

class Array;
struct Variant;

class pcre_cache_entry {
  pcre_cache_entry(const pcre_cache_entry&);
  pcre_cache_entry& operator=(const pcre_cache_entry&);

public:
  pcre_cache_entry()
    : re(nullptr), extra(nullptr), preg_options(0), compile_options(0),
      num_subpats(0), subpat_names(nullptr), ref_count(0)
  {}

  ~pcre_cache_entry();

  void setStatic() {
    ref_count = StaticValue;
  }

  void incRefCount() {
    if (ref_count != StaticValue) {
      ++ref_count;
    }
  }

  bool decRefAndRelease() {
    if (ref_count != StaticValue) {
      if (--ref_count == 0) {
        delete this;
        return true;
      }
    }
    return false;
  }

  pcre *re;
  pcre_extra *extra; // Holds results of studying
  int preg_options:1;
  int compile_options:31;
  int num_subpats;
  char **subpat_names;
  int ref_count;
};

typedef SmartPtr<pcre_cache_entry> pcre_cache_entry_ptr;

class PCREglobals {
public:
  PCREglobals() : m_local_cache(kLocalCacheSize) { }
  // pcre ini_settings
  int64_t m_preg_backtrace_limit;
  int64_t m_preg_recursion_limit;
  folly::EvictingCacheMap<std::string, pcre_cache_entry_ptr> m_local_cache;
};

///////////////////////////////////////////////////////////////////////////////
// Cache management

/*
 * Initialize PCRE cache.
 */
void pcre_init();

/*
 * Clear PCRE cache.  Not thread safe - call only after parsing options.
 */
void pcre_reinit();

/*
 * Dump the contents of the PCRE cache to filename.
 */
void pcre_dump_cache(const std::string& filename);

///////////////////////////////////////////////////////////////////////////////
// PHP API

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
