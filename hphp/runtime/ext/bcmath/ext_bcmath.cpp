/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/bcmath/bcmath.h"

#include "hphp/util/string-vsnprintf.h"

#include <folly/ScopeGuard.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static RDS_LOCAL(int64_t, s_initial_precision);

struct bcmath_data {
  bcmath_data() {
    // we can't really call bc_init_numbers() that calls into this constructor
    data._zero_ = _bc_new_num_ex (1,0,1);
    data._one_  = _bc_new_num_ex (1,0,1);
    data._one_->n_value[0] = 1;
    data._two_  = _bc_new_num_ex (1,0,1);
    data._two_->n_value[0] = 2;
    data.bc_precision = *s_initial_precision;
  }
  ~bcmath_data() {
    bc_free_num(&data._zero_);
    bc_free_num(&data._one_);
    bc_free_num(&data._two_);
  }
  BCMathGlobals data;
};
static RDS_LOCAL(bcmath_data, s_globals);

namespace {

InitFiniNode r_shutdown(
  [] { s_globals.destroy(); },
  InitFiniNode::When::RequestFini
);

int64_t adjust_scale(int64_t scale) {
  if (scale < 0) {
    scale = BCG(bc_precision);
    if (scale < 0) scale = 0;
  }
  if ((uint64_t)scale > StringData::MaxSize) return StringData::MaxSize;
  return scale;
}

void php_str2num(bc_num *num, const char *str) {
  const char *p;
  if (!(p = strchr(str, '.'))) {
    bc_str2num(num, (char*)str, 0);
  } else {
    bc_str2num(num, (char*)str, strlen(p + 1));
  }
}

String php_num2str(bc_num num) {
  auto const str = bc_num2str(num);
  SCOPE_EXIT { bc_free(str); };
  return String{str, CopyString};
}

}

///////////////////////////////////////////////////////////////////////////////

static bool HHVM_FUNCTION(bcscale, int64_t scale) {
  BCG(bc_precision) = scale < 0 ? 0 : scale;
  return true;
}

static String HHVM_FUNCTION(bcadd, const String& left, const String& right,
                            int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, result;
  bc_init_num(&first);
  SCOPE_EXIT { bc_free_num(&first); };
  bc_init_num(&second);
  SCOPE_EXIT { bc_free_num(&second); };
  bc_init_num(&result);
  SCOPE_EXIT { bc_free_num(&result); };
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_add(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  return php_num2str(result);
}

static String HHVM_FUNCTION(bcsub, const String& left, const String& right,
                            int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, result;
  bc_init_num(&first);
  SCOPE_EXIT { bc_free_num(&first); };
  bc_init_num(&second);
  SCOPE_EXIT { bc_free_num(&second); };
  bc_init_num(&result);
  SCOPE_EXIT { bc_free_num(&result); };
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_sub(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  return php_num2str(result);
}

static int64_t HHVM_FUNCTION(bccomp, const String& left, const String& right,
                             int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second;
  bc_init_num(&first);
  SCOPE_EXIT { bc_free_num(&first); };
  bc_init_num(&second);
  SCOPE_EXIT { bc_free_num(&second); };
  bc_str2num(&first, (char*)left.data(), scale);
  bc_str2num(&second, (char*)right.data(), scale);
  return bc_compare(first, second);
}

static String HHVM_FUNCTION(bcmul, const String& left, const String& right,
                            int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, result;
  bc_init_num(&first);
  SCOPE_EXIT { bc_free_num(&first); };
  bc_init_num(&second);
  SCOPE_EXIT { bc_free_num(&second); };
  bc_init_num(&result);
  SCOPE_EXIT { bc_free_num(&result); };
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_multiply(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  return php_num2str(result);
}

static Variant HHVM_FUNCTION(bcdiv, const String& left, const String& right,
               int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, result;
  bc_init_num(&first);
  SCOPE_EXIT { bc_free_num(&first); };
  bc_init_num(&second);
  SCOPE_EXIT { bc_free_num(&second); };
  bc_init_num(&result);
  SCOPE_EXIT { bc_free_num(&result); };
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  if (bc_divide(first, second, &result, scale) == -1) {
    raise_warning("Division by zero");
    return init_null();
  }
  return php_num2str(result);
}

static Variant HHVM_FUNCTION(bcmod, const String& left, const String& right) {
  bc_num first, second, result;
  bc_init_num(&first);
  SCOPE_EXIT { bc_free_num(&first); };
  bc_init_num(&second);
  SCOPE_EXIT { bc_free_num(&second); };
  bc_init_num(&result);
  SCOPE_EXIT { bc_free_num(&result); };
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  if (bc_modulo(first, second, &result, 0) == -1) {
    raise_warning("Division by zero");
    return init_null();
  }
  return php_num2str(result);
}

static String HHVM_FUNCTION(bcpow, const String& left, const String& right,
                           int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, result;
  bc_init_num(&first);
  SCOPE_EXIT { bc_free_num(&first); };
  bc_init_num(&second);
  SCOPE_EXIT { bc_free_num(&second); };
  bc_init_num(&result);
  SCOPE_EXIT { bc_free_num(&result); };
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_raise(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  return php_num2str(result);
}

static Variant HHVM_FUNCTION(bcpowmod, const String& left, const String& right,
                             const String& modulus, int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, mod, result;
  bc_init_num(&first);
  SCOPE_EXIT { bc_free_num(&first); };
  bc_init_num(&second);
  SCOPE_EXIT { bc_free_num(&second); };
  bc_init_num(&mod);
  SCOPE_EXIT { bc_free_num(&mod); };
  bc_init_num(&result);
  SCOPE_EXIT { bc_free_num(&result); };
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  php_str2num(&mod, (char*)modulus.data());
  if (bc_raisemod(first, second, mod, &result, scale) == -1) {
    return false;
  }
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  return php_num2str(result);
}

static Variant HHVM_FUNCTION(bcsqrt, const String& operand,
                             int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num result;
  bc_init_num(&result);
  SCOPE_EXIT { bc_free_num(&result); };
  php_str2num(&result, (char*)operand.data());
  Variant ret;
  if (bc_sqrt(&result, scale) != 0) {
    if (result->n_scale > scale) {
      result->n_scale = scale;
    }
    ret = php_num2str(result);
  } else {
    raise_warning("Square root of negative number");
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

struct bcmathExtension final : Extension {
  bcmathExtension() : Extension("bcmath", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleRegisterNative() override {
    HHVM_FE(bcscale);
    HHVM_FE(bcadd);
    HHVM_FE(bcsub);
    HHVM_FE(bccomp);
    HHVM_FE(bcmul);
    HHVM_FE(bcdiv);
    HHVM_FE(bcmod);
    HHVM_FE(bcpow);
    HHVM_FE(bcpowmod);
    HHVM_FE(bcsqrt);
  }

  void threadInit() override {
    IniSetting::Bind(this, IniSetting::Mode::Request,
                     "bcmath.scale", "0",
                     s_initial_precision.get());
  }

} s_bcmath_extension;

///////////////////////////////////////////////////////////////////////////////
}

struct BCMathGlobals *get_bcmath_globals() {
  return &HPHP::s_globals.get()->data;
}

void* bc_malloc(size_t total) {
  if (UNLIKELY(total > INT_MAX)) {
    HPHP::SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat(
        "Trying to allocate {} bytes (limit {}) within bcmath",
        total, INT_MAX
      )
    );
  }

  if (UNLIKELY(total > HPHP::kMaxSmallSize &&
               HPHP::tl_heap->preAllocOOM(total))) {
    HPHP::check_non_safepoint_surprise();
  }

  auto const ptr = HPHP::req::malloc_untyped(total);
  if (UNLIKELY(!ptr)) throw std::bad_alloc{};
  return ptr;
}

void bc_free(void* ptr) {
  HPHP::req::free(ptr);
}

void bc_rt_warn (char *format ,...)
{
  std::string msg;
  va_list ap;
  va_start(ap, format);
  HPHP::string_vsnprintf(msg, format, ap);
  va_end(ap);
  HPHP::raise_warning("bc math warning: %s", msg.c_str());
}

void bc_rt_error (char *format ,...)
{
  std::string msg;
  va_list ap;
  va_start(ap, format);
  HPHP::string_vsnprintf(msg, format, ap);
  va_end(ap);
  HPHP::raise_recoverable_error("bc math error: %s", msg.c_str());
}

///////////////////////////////////////////////////////////////////////////////
