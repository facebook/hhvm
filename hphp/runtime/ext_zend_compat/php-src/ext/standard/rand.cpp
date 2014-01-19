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
   | Based on code from: Richard J. Wagner <rjwagner@writeme.com>         |
   |                     Makoto Matsumoto <matumoto@math.keio.ac.jp>      |
   |                     Takuji Nishimura                                 |
   |                     Shawn Cokus <Cokus@math.washington.edu>          |
   +----------------------------------------------------------------------+
 */
/* $Id$ */

#include <stdlib.h>

// has to be before zend_API since that defines getThis()
#include "hphp/runtime/ext/ext_math.h"
#include "php.h"
#include "php_rand.h"

#include "basic_functions.h"

PHPAPI long php_rand(TSRMLS_D) {
  return HPHP::f_rand();
}
