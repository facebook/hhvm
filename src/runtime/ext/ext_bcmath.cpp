/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/ext_bcmath.h>
#include <runtime/ext/bcmath/bcmath.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(bcmath);
///////////////////////////////////////////////////////////////////////////////

class bcmath_data {
public:
  bcmath_data() {
    // we can't really call bc_init_numbers() that calls into this constructor
    data._zero_ = _bc_new_num_ex (1,0,1);
    data._one_  = _bc_new_num_ex (1,0,1);
    data._one_->n_value[0] = 1;
    data._two_  = _bc_new_num_ex (1,0,1);
    data._two_->n_value[0] = 2;
    data.bc_precision = 0;
  }
  BCMathGlobals data;
};
static IMPLEMENT_THREAD_LOCAL(bcmath_data, s_globals);

///////////////////////////////////////////////////////////////////////////////

static void php_str2num(bc_num *num, const char *str) {
  const char *p;
  if (!(p = strchr(str, '.'))) {
    bc_str2num(num, (char*)str, 0);
  } else {
    bc_str2num(num, (char*)str, strlen(p + 1));
  }
}

bool f_bcscale(int64 scale) {
  BCG(bc_precision) = scale < 0 ? 0 : scale;
  return true;
}

String f_bcadd(CStrRef left, CStrRef right, int64 scale /* = -1 */) {
  if (scale < 0) scale = BCG(bc_precision);
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_add(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  String ret(bc_num2str(result), AttachString);
  bc_free_num(&first);
  bc_free_num(&second);
  bc_free_num(&result);
  return ret;
}

String f_bcsub(CStrRef left, CStrRef right, int64 scale /* = -1 */) {
  if (scale < 0) scale = BCG(bc_precision);
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_sub(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  String ret(bc_num2str(result), AttachString);
  bc_free_num(&first);
  bc_free_num(&second);
  bc_free_num(&result);
  return ret;
}

int64 f_bccomp(CStrRef left, CStrRef right, int64 scale /* = -1 */) {
  if (scale < 0) scale = BCG(bc_precision);
  bc_num first, second;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_str2num(&first, (char*)left.data(), scale);
  bc_str2num(&second, (char*)right.data(), scale);
  int64 ret = bc_compare(first, second);
  bc_free_num(&first);
  bc_free_num(&second);
  return ret;
}

String f_bcmul(CStrRef left, CStrRef right, int64 scale /* = -1 */) {
  if (scale < 0) scale = BCG(bc_precision);
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_multiply(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  String ret(bc_num2str(result), AttachString);
  bc_free_num(&first);
  bc_free_num(&second);
  bc_free_num(&result);
  return ret;
}

String f_bcdiv(CStrRef left, CStrRef right, int64 scale /* = -1 */) {
  if (scale < 0) scale = BCG(bc_precision);
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  if (bc_divide(first, second, &result, scale) == -1) {
    raise_warning("Division by zero");
    return String();
  }
  String ret(bc_num2str(result), AttachString);
  bc_free_num(&first);
  bc_free_num(&second);
  bc_free_num(&result);
  return ret;
}

String f_bcmod(CStrRef left, CStrRef right) {
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  if (bc_modulo(first, second, &result, 0) == -1) {
    raise_warning("Division by zero");
    return String();
  }
  String ret(bc_num2str(result), AttachString);
  bc_free_num(&first);
  bc_free_num(&second);
  bc_free_num(&result);
  return ret;
}

String f_bcpow(CStrRef left, CStrRef right, int64 scale /* = -1 */) {
  if (scale < 0) scale = BCG(bc_precision);
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_raise(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  String ret(bc_num2str(result), AttachString);
  bc_free_num(&first);
  bc_free_num(&second);
  bc_free_num(&result);
  return ret;
}

Variant f_bcpowmod(CStrRef left, CStrRef right, CStrRef modulus,
                  int64 scale /* = -1 */) {
  if (scale < 0) scale = BCG(bc_precision);
  bc_num first, second, mod, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&mod);
  bc_init_num(&result);
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  php_str2num(&mod, (char*)modulus.data());
  if (bc_raisemod(first, second, mod, &result, scale) == -1) {
    return false;
  }
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  String ret(bc_num2str(result), AttachString);
  bc_free_num(&first);
  bc_free_num(&second);
  bc_free_num(&mod);
  bc_free_num(&result);
  return ret;
}

Variant f_bcsqrt(CStrRef operand, int64 scale /* = -1 */) {
  if (scale < 0) scale = BCG(bc_precision);
  bc_num result;
  bc_init_num(&result);
  php_str2num(&result, (char*)operand.data());
  Variant ret;
  if (bc_sqrt(&result, scale) != 0) {
    if (result->n_scale > scale) {
      result->n_scale = scale;
    }
    ret = String(bc_num2str(result), AttachString);
  } else {
    raise_warning("Square root of negative number");
  }
  bc_free_num(&result);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" {
  struct BCMathGlobals *get_bcmath_globals() {
    return &HPHP::s_globals.get()->data;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
