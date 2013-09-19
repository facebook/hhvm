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

#ifndef PHP_PCRE_H
#define PHP_PCRE_H

typedef struct {
  int preg_options;
#if HAVE_SETLOCALE
  char *locale;
  unsigned const char *tables;
#endif
  int compile_options;
  int refcount;
} pcre_cache_entry;

PHPAPI pcre_cache_entry* pcre_get_compiled_regex_cache(char *regex, int regex_len TSRMLS_DC);

PHPAPI void  php_pcre_match_impl(  pcre_cache_entry *pce, char *subject, int subject_len, zval *return_value,
  zval *subpats, int global, int use_flags, long flags, long start_offset TSRMLS_DC);

#endif /* PHP_PCRE_H */
