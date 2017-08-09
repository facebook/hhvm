/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_HPHP_PHP7_UTIL_H
#define incl_HPHP_PHP7_UTIL_H

#include "hphp/php7/zend/zend.h"

namespace HPHP { namespace php7 {

// helpers to cope with these functions taking non-const pointers
// in zend_ast.h

inline const zval* zend_ast_get_zval(const zend_ast* ast) {
  return &reinterpret_cast<const zend_ast_zval*>(ast)->val;
}

inline const zend_ast_list* zend_ast_get_list(const zend_ast* ast) {
  return reinterpret_cast<const zend_ast_list*>(ast);
}

inline const zend_ast_decl* zend_ast_get_decl(const zend_ast* ast) {
  return reinterpret_cast<const zend_ast_decl*>(ast);
}

inline std::string zval_to_string(const zval* zv) {
  std::string out;
  switch (Z_TYPE_P(zv)) {
    case IS_LONG:
      out.append(folly::to<std::string>(Z_LVAL_P(zv)));
      break;
    case IS_NULL:
    case IS_FALSE:
      break;
    case IS_TRUE:
      out.append("1");
      break;
    case IS_DOUBLE:
      out.append(folly::to<std::string>(Z_DVAL_P(zv)));
      break;
    case IS_STRING:
      out.append(Z_STRVAL_P(zv));
      break;
  }
  return out;
}

}} // HPHP::php7

#endif // incl_HPHP_PHP7_UTIL_H
