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
   | Authors: Rasmus Lerdorf <rasmus@php.net>                             |
   |          Zeev Suraski <zeev@zend.com>                                |
   |          Pedro Melo <melo@ip.pt>                                     |
   |          Sterling Hughes <sterling@php.net>                          |
   |                                                                      |
   | Based on code from: Shawn Cokus <Cokus@math.washington.edu>          |
   +----------------------------------------------------------------------+
 */
/* $Id$ */

#ifndef PHP_RAND_H
#define PHP_RAND_H

#include <stdlib.h>
#include "basic_functions.h"

/* System Rand functions */
#ifndef RAND_MAX
#define RAND_MAX (1<<15)
#endif

/* In ZTS mode we rely on rand_r() so we must use RAND_MAX. */
#if !defined(ZTS) && (defined(HAVE_LRAND48) || defined(HAVE_RANDOM))
#define PHP_RAND_MAX 2147483647
#else
#define PHP_RAND_MAX RAND_MAX
#endif

/* MT Rand */
#define PHP_MT_RAND_MAX ((long) (0x7FFFFFFF)) /* (1<<31) - 1 */

PHPAPI long php_rand(TSRMLS_D);

#endif  /* PHP_RAND_H */
