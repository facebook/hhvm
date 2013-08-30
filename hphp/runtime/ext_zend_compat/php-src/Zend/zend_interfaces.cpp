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
#include "zend_interfaces.h"

#include "hphp/runtime/base/builtin-functions.h"

ZEND_API zval* zend_call_method(zval **object_pp, zend_class_entry *obj_ce, zend_function **fn_proxy, const char *function_name, int function_name_len, zval **retval_ptr_ptr, int param_count, zval* arg1, zval* arg2 TSRMLS_DC) {
  HPHP::String f_name(function_name, function_name_len, HPHP::CopyString);
  HPHP::ArrayInit paramInit(2);
  paramInit.set(0, tvAsVariant(arg1));
  paramInit.set(1, tvAsVariant(arg2));
  const HPHP::Array params(paramInit.create());
  HPHP::Variant ret = HPHP::vm_call_user_func(f_name, params);
  *retval_ptr_ptr = ret.asTypedValue();
  return ret.asTypedValue();
}
