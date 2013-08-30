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

#ifndef ZEND_VARIABLES_H
#define ZEND_VARIABLES_H

BEGIN_EXTERN_C()

ZEND_API inline void _zval_dtor_func(zval *zvalue ZEND_FILE_LINE_DC) {
  HPHP::tvRefcountedDecRef(zvalue);
}

zend_always_inline void _zval_dtor(zval *zvalue ZEND_FILE_LINE_DC)
{
  if (zvalue->m_type <= IS_BOOL) {
    return;
  }
  _zval_dtor_func(zvalue ZEND_FILE_LINE_RELAY_CC);
}

ZEND_API inline void _zval_copy_ctor_func(zval *zvalue ZEND_FILE_LINE_DC) {
  HPHP::tvRefcountedIncRef(zvalue);
}

zend_always_inline void _zval_copy_ctor(zval *zvalue ZEND_FILE_LINE_DC)
{
  if (zvalue->m_type <= IS_BOOL) {
    return;
  }
  _zval_copy_ctor_func(zvalue ZEND_FILE_LINE_RELAY_CC);
}

ZEND_API inline void _zval_ptr_dtor(zval **zval_ptr ZEND_FILE_LINE_DC) {
  _zval_dtor(*zval_ptr);
}
#define zval_copy_ctor(zvalue) _zval_copy_ctor((zvalue) ZEND_FILE_LINE_CC)
#define zval_dtor(zvalue) _zval_dtor((zvalue) ZEND_FILE_LINE_CC)
#define zval_ptr_dtor(zval_ptr) _zval_ptr_dtor((zval_ptr) ZEND_FILE_LINE_CC)

ZEND_API inline void zval_add_ref(zval **p) {
  HPHP::tvRefcountedIncRef(*p);
}

END_EXTERN_C()

#endif
