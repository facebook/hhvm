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
   | Authors: Jani Lehtimäki <jkl@njet.net>                               |
   |          Thies C. Arntzen <thies@thieso.net>                         |
   |          Sascha Schumann <sascha@schumann.cx>                        |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

/* {{{ includes
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "php.h"
#include "php_string.h"
#include "php_var.h"
#include "php_smart_str.h"
#include "basic_functions.h"

#include "hphp/runtime/base/complex-types.h"
#include "ext/standard/php_smart_str.h"
#include "hphp/runtime/ext/ext_variable.h"

#define COMMON (Z_ISREF_PP(struc) ? "&" : "")
/* }}} */

PHPAPI void php_var_serialize(smart_str *buf, zval **struc, php_serialize_data_t *var_hash TSRMLS_DC) {
  HPHP::String s = HPHP::f_serialize(HPHP::tvAsVariant((*struc)->tv()));
  smart_str_appendl(buf, s->data(), s->size());
}
PHPAPI int php_var_unserialize(zval **rval, const unsigned char **p, const unsigned char *max, php_unserialize_data_t *var_hash TSRMLS_DC) {
  HPHP::Variant ret = HPHP::f_unserialize(HPHP::String((const char*) *p, max - *p, HPHP::CopyString));
  MAKE_STD_ZVAL(*rval);
  HPHP::cellDup(*ret.asCell(), *(*rval)->tv());
  return !ret.isBoolean() || ret.toBoolean();
}
PHPAPI void var_destroy(php_unserialize_data_t *var_hash) {
  not_implemented();
}
