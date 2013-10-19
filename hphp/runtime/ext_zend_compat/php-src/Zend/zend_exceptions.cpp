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
   |          Marcus Boerger <helly@php.net>                              |
   |          Sterling Hughes <sterling@php.net>                          |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "zend.h"
#include "zend_API.h"
#include "zend_interfaces.h"
#include "zend_exceptions.h"
#include "zend_globals.h"
#include "hphp/system/systemlib.h"

static zend_class_entry *default_exception_ce;

static zend_object_value zend_default_exception_new_ex(zend_class_entry *class_type, int skip_top_traces TSRMLS_DC) /* {{{ */
{
  zend_object *object;

  zend_object_value ret = zend_objects_new(&object, class_type TSRMLS_CC);

  object_properties_init(object, class_type);

  return ret;
}

static zend_object_value zend_default_exception_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
  return zend_default_exception_new_ex(class_type, 0 TSRMLS_CC);
}

void zend_register_default_exception(TSRMLS_D) {
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, "Exception", default_exception_functions);
  default_exception_ce = zend_register_internal_class(&ce TSRMLS_CC);
  default_exception_ce->create_object = zend_default_exception_new;
}

ZEND_API zend_class_entry *zend_exception_get_default(TSRMLS_D) {
  // TODO figure out how to do this once
  zend_register_default_exception();
  return default_exception_ce;
}

ZEND_API void zend_throw_exception_object(zval *exception TSRMLS_DC) /* {{{ */
{
  zend_class_entry *exception_ce;

  if (exception == NULL || Z_TYPE_P(exception) != IS_OBJECT) {
    zend_error(E_ERROR, "Need to supply an object when throwing an exception");
  }

  exception_ce = Z_OBJCE_P(exception);

  if (!exception_ce || !instanceof_function(exception_ce, default_exception_ce TSRMLS_CC)) {
    zend_error(E_ERROR, "Exceptions must be valid objects derived from the Exception base class");
  }
  zend_throw_exception_internal(exception TSRMLS_CC);
}
/* }}} */

void zend_throw_exception_internal(zval *exception TSRMLS_DC) /* {{{ */
{
  throw HPHP::Object(Z_OBJVAL_P(exception));
}
/* }}} */
