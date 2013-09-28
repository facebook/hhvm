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
   | Authors: Marcus Boerger <helly@php.net>                              |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_INTERFACES_H
#define ZEND_INTERFACES_H

#include "zend.h"
#include "zend_API.h"

BEGIN_EXTERN_C()

ZEND_API zval* zend_call_method(zval **object_pp, zend_class_entry *obj_ce, zend_function **fn_proxy, const char *function_name, int function_name_len, zval **retval_ptr_ptr, int param_count, zval* arg1, zval* arg2 TSRMLS_DC);

#define zend_call_method_with_0_params(obj, obj_ce, fn_proxy, function_name, retval) \
  zend_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval, 0, NULL, NULL TSRMLS_CC)

#define zend_call_method_with_1_params(obj, obj_ce, fn_proxy, function_name, retval, arg1) \
  zend_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval, 1, arg1, NULL TSRMLS_CC)

#define zend_call_method_with_2_params(obj, obj_ce, fn_proxy, function_name, retval, arg1, arg2) \
  zend_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval, 2, arg1, arg2 TSRMLS_CC)

END_EXTERN_C()

#endif /* ZEND_INTERFACES_H */
