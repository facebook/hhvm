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
   |          Colin Viebrock <colin@easydns.com>                          |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "php.h"
#include "php_ini.h"
#include "php_globals.h"
#include "ext/standard/head.h"
#include "SAPI.h"
#include <time.h>
#include "php_main.h"
#include "zend_globals.h"       /* needs ELS */
#include "zend_extensions.h"
#include "zend_highlight.h"
#include "info.h"
#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#include "hphp/util/assertions.h"

void php_info_print_table_header(int num_cols, ...) {
  not_implemented();
}
void php_info_print_table_row(int num_cols, ...) {
  not_implemented();
}
void php_info_print_table_start(void) {
  not_implemented();
}
void php_info_print_table_end(void) {
  not_implemented();
}
