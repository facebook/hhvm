/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
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
