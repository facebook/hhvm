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

#include <stdio.h>
#include "zend.h"
#include "zend_API.h"
#include "zend_globals.h"
#include "zend_constants.h"
#include "zend_list.h"

ZEND_API void _zval_copy_ctor_func(zval *zvalue ZEND_FILE_LINE_DC) {
  if (HPHP::IS_STRING_TYPE(zvalue->tv()->m_type)) {
    zvalue->tv()->m_data.pstr =
      HPHP::StringData::Make(zvalue->tv()->m_data.pstr, HPHP::CopyString);
    zvalue->tv()->m_data.pstr->incRefCount();
  } else if (zvalue->tv()->m_type == HPHP::KindOfArray) {
    zvalue->tv()->m_data.parr = zvalue->tv()->m_data.parr->copy();
    zvalue->tv()->m_data.parr->incRefCount();
  } else if (IS_REFCOUNTED_TYPE(zvalue->tv()->m_type)) {
    zvalue->tv()->m_data.pstr->incRefCount();
  }
}

ZEND_API void _zval_ptr_dtor_wrapper(zval **zval_ptr) {
  zval_ptr_dtor(zval_ptr);
}   

ZEND_API void _zval_ptr_dtor(zval **zval_ptr ZEND_FILE_LINE_DC) {
  HPHP::decRefRef(*zval_ptr);
}

ZEND_API void zval_add_ref(zval **p) {
  (*p)->zAddRef();
}

