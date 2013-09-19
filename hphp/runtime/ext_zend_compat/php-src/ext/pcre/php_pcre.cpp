/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Andrei Zmievski <andrei@php.net>                             |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#include "php.h"
#include "php_ini.h"
#include "php_globals.h"
#include "php_pcre.h"
#include "ext/standard/info.h"
#include "ext/standard/php_smart_str.h"
#if HAVE_PCRE || HAVE_BUNDLED_PCRE

#include <pcre.h>

#include "ext/standard/php_string.h"

#include "hphp/runtime/base/preg.h"

#define PREG_PATTERN_ORDER      1
#define PREG_SET_ORDER        2
#define PREG_OFFSET_CAPTURE      (1<<8)

#define  PREG_SPLIT_NO_EMPTY      (1<<0)
#define PREG_SPLIT_DELIM_CAPTURE  (1<<1)
#define PREG_SPLIT_OFFSET_CAPTURE  (1<<2)

#define PREG_REPLACE_EVAL      (1<<0)

#define PREG_GREP_INVERT      (1<<0)

#define PCRE_CACHE_SIZE 4096

enum {
  PHP_PCRE_NO_ERROR = 0,
  PHP_PCRE_INTERNAL_ERROR,
  PHP_PCRE_BACKTRACK_LIMIT_ERROR,
  PHP_PCRE_RECURSION_LIMIT_ERROR,
  PHP_PCRE_BAD_UTF8_ERROR,
  PHP_PCRE_BAD_UTF8_OFFSET_ERROR
};

PHPAPI void php_pcre_match_impl(pcre_cache_entry *pce, char *subject, int subject_len, zval *return_value,
  zval *subpats, int global, int use_flags, long flags, long start_offset TSRMLS_DC)
{
  not_implemented();
}

PHPAPI pcre_cache_entry* pcre_get_compiled_regex_cache(char *regex, int regex_len TSRMLS_DC)
{
  not_implemented();
  return nullptr;
}

#endif /* HAVE_PCRE || HAVE_BUNDLED_PCRE */
