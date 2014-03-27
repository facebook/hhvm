/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#include "php.h"
#include "php_streams.h"
#include "php_main.h"
#include "php_globals.h"
#include "php_ini.h"
#include "basic_functions.h"
#include "zend_operators.h"
#include "ext/standard/php_lcg.h"

#include "zend.h"

#include "zend_globals.h"
#include "SAPI.h"

#include "hphp/runtime/ext/ext_misc.h"

PHPAPI php_basic_globals basic_globals;

PHPAPI double php_get_nan(void) {
  return HPHP::k_NAN;
}
PHPAPI double php_get_inf(void) {
  return HPHP::k_INF;
}

PHP_MINIT_FUNCTION(basic) /* {{{ */
{
  PHP_MINIT(lcg)(INIT_FUNC_ARGS_PASSTHRU);
  return SUCCESS;
}
/* }}} */

PHP_MSHUTDOWN_FUNCTION(basic) /* {{{ */
{
  return SUCCESS;
}
/* }}} */

PHP_RINIT_FUNCTION(basic) /* {{{ */
{
  return SUCCESS;
}
/* }}} */

PHP_RSHUTDOWN_FUNCTION(basic) /* {{{ */
{
  return SUCCESS;
}
/* }}} */

PHP_MINFO_FUNCTION(basic) /* {{{ */
{
}
/* }}} */

const zend_function_entry basic_functions[] = { /* {{{ */
  PHP_FE_END
};
/* }}} */

zend_module_entry basic_functions_module = { /* {{{ */
  STANDARD_MODULE_HEADER,
  "standard_zend_compat",               /* extension name */
  basic_functions,                      /* function list */
  PHP_MINIT(basic),                     /* process startup */
  PHP_MSHUTDOWN(basic),                 /* process shutdown */
  PHP_RINIT(basic),                     /* request startup */
  PHP_RSHUTDOWN(basic),                 /* request shutdown */
  PHP_MINFO(basic),                     /* extension info */
  PHP_VERSION,                          /* extension version */
  STANDARD_MODULE_PROPERTIES
};
