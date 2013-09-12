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
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef INFO_H
#define INFO_H

BEGIN_EXTERN_C()
PHPAPI inline void php_info_print_table_header(int num_cols, ...) {}
PHPAPI inline void php_info_print_table_row(int num_cols, ...) {}
PHPAPI inline void php_info_print_table_start(void) {}
PHPAPI inline void php_info_print_table_end(void) {}
END_EXTERN_C()

#endif /* INFO_H */
