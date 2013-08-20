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

#ifndef ZEND_EXECUTE_H
#define ZEND_EXECUTE_H

#include "zend_compile.h"
#include "zend_hash.h"
#include "zend_operators.h"
#include "zend_variables.h"

#include "hphp/runtime/vm/jit/translator-inline.h"

BEGIN_EXTERN_C()
ZEND_API int zend_lookup_class(const char *name, int name_length, zend_class_entry ***ce TSRMLS_DC);

/* services */
ZEND_API const char *get_active_class_name(const char **space TSRMLS_DC);
ZEND_API inline const char *get_active_function_name(TSRMLS_D) {
  HPHP::Transl::VMRegAnchor _;
  return HPHP::liveFunc()->name()->data();
}

END_EXTERN_C()

#endif /* ZEND_EXECUTE_H */
