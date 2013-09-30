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
   |          Stig Sæther Bakken <ssb@php.net>                            |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

/* Synced with php 3.0 revision 1.43 1999-06-16 [ssb] */

#ifndef PHP_STRING_H
#define PHP_STRING_H

PHPAPI char *php_strtolower(char *s, size_t len);
PHPAPI char *php_str_to_str_ex(char *haystack, int length, char *needle,
    int needle_len, char *str, int str_len, int *_new_length, int case_sensitivity, int *replace_count);
PHPAPI char *php_trim(char *c, int len, char *what, int what_len, zval *return_value, int mode TSRMLS_DC);

#endif /* PHP_STRING_H */
