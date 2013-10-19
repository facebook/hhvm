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
   | Author: Jim Winstead <jimw@php.net>                                  |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef BASE64_H
#define BASE64_H

#include "hphp/runtime/ext/ext_url.h"

PHPAPI extern unsigned char *php_base64_encode(const unsigned char *, int, int *);
PHPAPI extern unsigned char *php_base64_decode_ex(const unsigned char *, int, int *, zend_bool);
PHPAPI extern unsigned char *php_base64_decode(const unsigned char *, int, int *);

#endif /* BASE64_H */
