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

#include "zend.h"
#include "zend_constants.h"
#include "zend_execute.h"
#include "zend_variables.h"
#include "zend_operators.h"
#include "zend_globals.h"

ZEND_API int zend_get_constant_ex(const char *name, uint name_len, zval *result, zend_class_entry *scope, ulong flags TSRMLS_DC) {
  return 0;
}
ZEND_API void zend_register_long_constant(const char *name, uint name_len, long lval, int flags, int module_number TSRMLS_DC) {
}
ZEND_API void zend_register_double_constant(const char *name, uint name_len, double dval, int flags, int module_number TSRMLS_DC) {
}
ZEND_API void zend_register_string_constant(const char *name, uint name_len, char *strval, int flags, int module_number TSRMLS_DC) {
}
ZEND_API void zend_register_stringl_constant(const char *name, uint name_len, char *strval, uint strlen, int flags, int module_number TSRMLS_DC) {
}
