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

#include "zend.h"
// builtin-functions has to happen before zend_API since that defines getThis()
#include "hphp/runtime/base/builtin-functions.h"
#include "zend_API.h"
#include "zend_interfaces.h"
#include "zend_exceptions.h"

#include "hphp/runtime/base/array-init.h"

ZEND_API zend_class_entry *zend_ce_traversable;
ZEND_API zend_class_entry *zend_ce_aggregate;
ZEND_API zend_class_entry *zend_ce_iterator;
ZEND_API zend_class_entry *zend_ce_arrayaccess;
ZEND_API zend_class_entry *zend_ce_serializable;

ZEND_API zval* zend_call_method(zval **object_pp, zend_class_entry *obj_ce, zend_function **fn_proxy, const char *function_name, int function_name_len, zval **retval_ptr_ptr, int param_count, zval* arg1, zval* arg2 TSRMLS_DC) {
  HPHP::String f_name(function_name, function_name_len, HPHP::CopyString);
  HPHP::ArrayInit paramInit(2);
  paramInit.set(0, tvAsVariant(arg1->tv()));
  paramInit.set(1, tvAsVariant(arg2->tv()));
  const HPHP::Array params(paramInit.create());
  HPHP::Variant ret = HPHP::vm_call_user_func(f_name, params);
  auto ref = ret.asRef()->m_data.pref;
  ref->incRefCount();
  *retval_ptr_ptr = ref;
  return ref;
}
