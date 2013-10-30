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
#include "zend_compile.h"
#include "zend_constants.h"
// has to happen before zend_API since that defines getThis()
#include "hphp/runtime/base/externals.h"
#include "zend_API.h"
#include "zend_exceptions.h"

zend_bool zend_is_auto_global_quick(const char *name, uint name_len, ulong hashval TSRMLS_DC) /* {{{ */
{
  return HPHP::get_global_variables()->exists(HPHP::String(name, name_len, HPHP::CopyString));
}
/* }}} */

zend_bool zend_is_auto_global(const char *name, uint name_len TSRMLS_DC) /* {{{ */
{
  return zend_is_auto_global_quick(name, name_len, 0 TSRMLS_CC);
}
