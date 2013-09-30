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
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef BASIC_FUNCTIONS_H
#define BASIC_FUNCTIONS_H

#include "hphp/runtime/ext/ext_misc.h"

#if SIZEOF_INT == 4
/* Most 32-bit and 64-bit systems have 32-bit ints */
typedef unsigned int php_uint32;
typedef signed int php_int32;
#elif SIZEOF_LONG == 4
/* 16-bit systems? */
typedef unsigned long php_uint32;
typedef signed long php_int32;
#else
#error Need type which holds 32 bits
#endif

#define MT_N (624)

PHPAPI inline double php_get_nan(void) {
  return HPHP::k_NAN;
}
PHPAPI inline double php_get_inf(void) {
  return HPHP::k_INF;
}

#endif /* BASIC_FUNCTIONS_H */
