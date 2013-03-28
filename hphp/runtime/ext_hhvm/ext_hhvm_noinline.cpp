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
#include <runtime/ext/ext.h>

/*
 * This file contains non-inline definitions of some extension
 * functions that are only defined inline in hphpc.
 *
 * In HHVM we need actual addresses for these functions to jump to.
 */

namespace HPHP {

Variant fni_call_user_func_array(Variant const& function, Array const& params) {
  return f_call_user_func_array(function, params);
}

bool fni_is_bool(Variant const& var) {
  return f_is_bool(var);
}

bool fni_is_int(Variant const& var) {
  return f_is_int(var);
}

bool fni_is_integer(Variant const& var) {
  return f_is_integer(var);
}

bool fni_is_long(Variant const& var) {
  return f_is_long(var);
}

bool fni_is_double(Variant const& var) {
  return f_is_double(var);
}

bool fni_is_float(Variant const& var) {
  return f_is_float(var);
}

bool fni_is_numeric(Variant const& var) {
  return f_is_numeric(var);
}

bool fni_is_real(Variant const& var) {
  return f_is_real(var);
}

bool fni_is_string(Variant const& var) {
  return f_is_string(var);
}

bool fni_is_scalar(Variant const& var) {
  return f_is_scalar(var);
}

bool fni_is_array(Variant const& var) {
  return f_is_array(var);
}

bool fni_is_resource(Variant const& var) {
  return f_is_resource(var);
}

bool fni_is_null(Variant const& var) {
  return f_is_null(var);
}

Variant fni_unserialize(String const& str) {
  return f_unserialize(str);
}

}
