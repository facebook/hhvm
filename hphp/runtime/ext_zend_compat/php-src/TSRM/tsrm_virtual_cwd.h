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
   |          Sascha Schumann <sascha@schumann.cx>                        |
   |          Pierre Joye <pierre@php.net>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef VIRTUAL_CWD_H
#define VIRTUAL_CWD_H

#include "TSRM.h"
#include <unistd.h>

#define CWD_API

CWD_API char *tsrm_realpath(const char *path, char *real_path TSRMLS_DC);

#define VCWD_GETCWD(buff, size) getcwd(buff, size)
#define VCWD_REALPATH(path, real_path) tsrm_realpath(path, real_path TSRMLS_CC)

#endif /* VIRTUAL_CWD_H */
