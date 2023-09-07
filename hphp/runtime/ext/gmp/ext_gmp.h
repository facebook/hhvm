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

#ifndef HPHP_RUNTIME_EXT_GMP_GMP_H
#define HPHP_RUNTIME_EXT_GMP_GMP_H

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/vm/native-data.h"

#include <gmp.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// header

#define GMP_ROUND_ZERO      0
#define GMP_ROUND_PLUSINF   1
#define GMP_ROUND_MINUSINF  2

#define GMP_DEFAULT_BASE    10

// The maximum base for input and output conversions is 62 from GMP 4.2 onwards
#if (((__GNU_MP_VERSION) >= 5) || ((__GNU_MP_VERSION) >= 4 \
                                 && (__GNU_MP_VERSION_MINOR) >= 2))
#  define GMP_MAX_BASE 62
#else
#  define GMP_MAX_BASE 36
#endif
#define GMP_MIN_BASE -36

// GMP class strings
const StaticString s_GMP_GMP("GMP");
const StaticString s_GMP_num("num");

// Array indexes for division functions
const StaticString s_GMP_s("s");
const StaticString s_GMP_t("t");
const StaticString s_GMP_g("g");

// Error strings
const char* const cs_GMP_INVALID_TYPE =
  "%s(): Unable to convert variable to GMP - wrong type";
const char* const cs_GMP_INVALID_STRING =
  "%s(): Unable to convert variable to GMP - string is not an integer";
const char* const cs_GMP_INVALID_OBJECT =
  "%s(): supplied object is not a valid GMP object";
const char* const cs_GMP_INVALID_VALUE_MUST_NOT_BE_ZERO =
  "%s(): Zero operand not allowed";
const char* const cs_GMP_INVALID_MODULUS_MUST_NOT_BE_ZERO =
  "%s(): Modulus may not be zero";
const char* const cs_GMP_INVALID_VALUE_MUST_BE_POSITIVE =
  "%s(): Value operand not allowed";
const char* const cs_GMP_INVALID_INDEX_IS_NEGATIVE =
  "%s(): Index must be greater than or equal to zero";
const char* const cs_GMP_INVALID_NUMBER_IS_NEGATIVE =
  "%s(): Number has to be greater than or equal to 0";
const char* const cs_GMP_INVALID_BASE_VALUE =
  "%s(): Bad base for conversion: %" PRId64 " (should be between 2 and %d)";
const char* const cs_GMP_INVALID_EXPONENT_MUST_BE_POSITIVE =
  "%s(): Exponent must not be negative";
const char* const cs_GMP_INVALID_ROOT_MUST_BE_POSITIVE =
  "%s(): The root must be positive";
const char* const cs_GMP_INVALID_ROUNDING_MODE =
  "%s(): Invalid rounding mode";
const char* const cs_GMP_INVALID_STARTING_INDEX_IS_NEGATIVE =
  "%s(): Starting index must be greater than or equal to zero";
const char* const cs_GMP_ERROR_EVEN_ROOT_NEGATIVE_NUMBER =
  "%s(): Can't take even root of negative number";

///////////////////////////////////////////////////////////////////////////////
// classes

struct GMPData {
  ~GMPData() { close(); }
  GMPData&       operator=(const GMPData& source);

  void           close();
  void           setGMPMpz(const mpz_t data);
  mpz_t&         getGMPMpz() { return m_gmpMpz; }

private:
  bool           m_isInit{false};
  mpz_t          m_gmpMpz;
};


struct GMP {
private:
  static Class* getClass() {
    return SystemLib::classLoad(s_GMP_GMP.get(), s_cls);
  }

public:
  static Object allocObject() {
    return Object{ getClass() };
  }

  static Object allocObject(const Variant& arg) {
    Object ret = allocObject();
    tvDecRefGen(
      g_context->invokeFunc(getClass()->getCtor(), make_vec_array(arg), ret.get())
    );
    return ret;
  }

  static HPHP::Class* s_cls;
};

} /* namespace HPHP */

#endif /* HPHP_RUNTIME_EXT_GMP_GMP_H */
