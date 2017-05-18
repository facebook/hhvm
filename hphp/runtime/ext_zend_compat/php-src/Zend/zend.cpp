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
#include "zend_extensions.h"
#include "zend_modules.h"
#include "zend_constants.h"
#include "zend_list.h"
#include "zend_API.h"
#include "zend_exceptions.h"
#include "zend_ini.h"
#include "zend_vm.h"
#include "zend_dtrace.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/runtime-error.h"

#ifdef ZTS
# define GLOBAL_FUNCTION_TABLE		global_function_table
# define GLOBAL_CLASS_TABLE			global_class_table
# define GLOBAL_CONSTANTS_TABLE		global_constants_table
# define GLOBAL_AUTO_GLOBALS_TABLE	global_auto_globals_table
#else
# define GLOBAL_FUNCTION_TABLE		CG(function_table)
# define GLOBAL_CLASS_TABLE			CG(class_table)
# define GLOBAL_AUTO_GLOBALS_TABLE	CG(auto_globals)
# define GLOBAL_CONSTANTS_TABLE		EG(zend_constants)
#endif

/* true multithread-shared globals */
ZEND_API zend_class_entry *zend_standard_class_def = NULL;

/* This probably should be initialized somewhere, but I didn't find a suitable
 * place so it's wrapped in a function for now */
zend_class_entry *get_zend_standard_class_def() {
  TSRMLS_FETCH();
  if (!zend_standard_class_def) {
    zend_class_entry class_entry;

    INIT_CLASS_ENTRY(class_entry, "stdClass", nullptr);
    zend_standard_class_def =
      zend_register_internal_class(&class_entry TSRMLS_CC);
  }
  return zend_standard_class_def;
}

ZEND_API void zend_make_printable_zval(zval *expr, zval *expr_copy, int *use_copy) {
  if (Z_TYPE_P(expr)==IS_STRING) {
    *use_copy = 0;
    return;
  }
  HPHP::StringData *str = tvCastToString(*expr->tv());
  ZVAL_STRING(expr_copy, str->data(), str->size());
  *use_copy = 1;
}

[[noreturn]]
void zend_error_noreturn(int type, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  HPHP::raise_message(static_cast<HPHP::ErrorMode>(type), format, ap);
  va_end(ap);
  // Stop GCC from complaining about a noreturn function possibly returning.
  throw HPHP::Exception("This should never be reached!");
}

ZEND_API void zend_error(int type, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  HPHP::raise_message(static_cast<HPHP::ErrorMode>(type), format, ap);
  va_end(ap);
}
