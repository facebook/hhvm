/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2013 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_COMPILE_H
#define ZEND_COMPILE_H

#include "zend.h"

#ifdef HAVE_STDARG_H
# include <stdarg.h>
#endif

typedef struct _zend_arg_info {
  const char *name;
  zend_uint name_len;
  const char *class_name;
  zend_uint class_name_len;
  zend_uchar type_hint;
  zend_bool allow_null;
  zend_bool pass_by_reference;
} zend_arg_info;

#define ZEND_RETURN_VALUE        0
#define ZEND_RETURN_REFERENCE      1

typedef void* zend_function;

#include "zend_globals_macros.h"

#define ZEND_SEND_BY_VAL     0
#define ZEND_SEND_BY_REF     1
#define ZEND_SEND_PREFER_REF 2

#endif /* ZEND_COMPILE_H */
