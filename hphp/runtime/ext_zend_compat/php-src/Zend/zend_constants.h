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

#ifndef ZEND_CONSTANTS_H
#define ZEND_CONSTANTS_H

#include "zend_globals.h"

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/ext_misc.h"

#define CONST_CS        (1<<0)        /* Case Sensitive */
#define CONST_PERSISTENT    (1<<1)        /* Persistent */
#define CONST_CT_SUBST      (1<<2)        /* Allow compile-time substitution */

#define  PHP_USER_CONSTANT INT_MAX  /* a constant defined in user space */

#define REGISTER_LONG_CONSTANT(name, lval, flags)  zend_register_long_constant((name), sizeof(name), (lval), (flags), module_number TSRMLS_CC)
#define REGISTER_DOUBLE_CONSTANT(name, dval, flags)  zend_register_double_constant((name), sizeof(name), (dval), (flags), module_number TSRMLS_CC)
#define REGISTER_STRING_CONSTANT(name, str, flags)  zend_register_string_constant((name), sizeof(name), (str), (flags), module_number TSRMLS_CC)
#define REGISTER_STRINGL_CONSTANT(name, str, len, flags)  zend_register_stringl_constant((name), sizeof(name), (str), (len), (flags), module_number TSRMLS_CC)

#define REGISTER_NS_LONG_CONSTANT(ns, name, lval, flags)  zend_register_long_constant(ZEND_NS_NAME(ns, name), sizeof(ZEND_NS_NAME(ns, name)), (lval), (flags), module_number TSRMLS_CC)
#define REGISTER_NS_DOUBLE_CONSTANT(ns, name, dval, flags)  zend_register_double_constant(ZEND_NS_NAME(ns, name), sizeof(ZEND_NS_NAME(ns, name)), (dval), (flags), module_number TSRMLS_CC)
#define REGISTER_NS_STRING_CONSTANT(ns, name, str, flags)  zend_register_string_constant(ZEND_NS_NAME(ns, name), sizeof(ZEND_NS_NAME(ns, name)), (str), (flags), module_number TSRMLS_CC)
#define REGISTER_NS_STRINGL_CONSTANT(ns, name, str, len, flags)  zend_register_stringl_constant(ZEND_NS_NAME(ns, name), sizeof(ZEND_NS_NAME(ns, name)), (str), (len), (flags), module_number TSRMLS_CC)

#define REGISTER_MAIN_LONG_CONSTANT(name, lval, flags)  zend_register_long_constant((name), sizeof(name), (lval), (flags), 0 TSRMLS_CC)
#define REGISTER_MAIN_DOUBLE_CONSTANT(name, dval, flags)  zend_register_double_constant((name), sizeof(name), (dval), (flags), 0 TSRMLS_CC)
#define REGISTER_MAIN_STRING_CONSTANT(name, str, flags)  zend_register_string_constant((name), sizeof(name), (str), (flags), 0 TSRMLS_CC)
#define REGISTER_MAIN_STRINGL_CONSTANT(name, str, len, flags)  zend_register_stringl_constant((name), sizeof(name), (str), (len), (flags), 0 TSRMLS_CC)

BEGIN_EXTERN_C()
ZEND_API inline int zend_get_constant_ex(const char *name, uint name_len, zval *result, zend_class_entry *scope, ulong flags TSRMLS_DC) {
  return 0;
}
ZEND_API inline void zend_register_long_constant(const char *name, uint name_len, long lval, int flags, int module_number TSRMLS_DC) {}
ZEND_API inline void zend_register_double_constant(const char *name, uint name_len, double dval, int flags, int module_number TSRMLS_DC) {}
ZEND_API inline void zend_register_string_constant(const char *name, uint name_len, char *strval, int flags, int module_number TSRMLS_DC) {}
ZEND_API inline void zend_register_stringl_constant(const char *name, uint name_len, char *strval, uint strlen, int flags, int module_number TSRMLS_DC) {}
END_EXTERN_C()

#define ZEND_CONSTANT_DTOR (void (*)(void *)) free_zend_constant

#endif
