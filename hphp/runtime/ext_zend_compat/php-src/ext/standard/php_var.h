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
   | Author: Jani Lehtimaki <jkl@njet.net>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_VAR_H
#define PHP_VAR_H

#include "ext/standard/basic_functions.h"
#include "ext/standard/php_smart_str.h"
#include "hphp/runtime/ext/ext_variable.h"

typedef HashTable* php_serialize_data_t;
typedef void* php_unserialize_data_t;

PHPAPI inline void php_var_serialize(smart_str *buf, zval **struc, php_serialize_data_t *var_hash TSRMLS_DC) {
  HPHP::String s = HPHP::f_serialize(tvAsVariant(*struc));
  smart_str_appendl(buf, s->data(), s->size());
}
PHPAPI inline int php_var_unserialize(zval **rval, const unsigned char **p, const unsigned char *max, php_unserialize_data_t *var_hash TSRMLS_DC) {
  HPHP::Variant ret = HPHP::f_unserialize(HPHP::String(*p, max - *p, HPHP::CopyString));
  MAKE_STD_ZVAL(*rval);
  tvDup(*ret.asTypedValue(), **rval);
  return !ret.isBoolean() || ret.toBoolean();
}

#define PHP_VAR_SERIALIZE_INIT(var_hash_ptr)  do{} while(0)
#define PHP_VAR_SERIALIZE_DESTROY(var_hash_ptr)  do{} while(0)
#define PHP_VAR_UNSERIALIZE_INIT(var_hash_ptr)  do{} while(0)
#define PHP_VAR_UNSERIALIZE_DESTROY(var_hash_ptr)  do{} while(0)

/* }}} */

#endif /* PHP_VAR_H */
