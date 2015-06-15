/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
const StaticString s_GMPData("GMPData");

// Array indexes for division functions
const StaticString s_GMP_s("s");
const StaticString s_GMP_t("t");
const StaticString s_GMP_g("g");

// Global defines Strings
const StaticString s_GMP_MAX_BASE("GMP_MAX_BASE");
const StaticString s_GMP_ROUND_ZERO("GMP_ROUND_ZERO");
const StaticString s_GMP_ROUND_PLUSINF("GMP_ROUND_PLUSINF");
const StaticString s_GMP_ROUND_MINUSINF("GMP_ROUND_MINUSINF");
const StaticString s_GMP_VERSION("GMP_VERSION");
const StaticString k_GMP_VERSION(gmp_version);

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
  "%s(): Bad base for conversion: %ld (should be between 2 and %d)";
const char* const cs_GMP_INVALID_EXPONENT_MUST_BE_POSITIVE =
  "%s(): Exponent must not be negative";
const char* const cs_GMP_INVALID_ROOT_MUST_BE_POSITIVE =
  "%s(): The root must be positive";
const char* const cs_GMP_INVALID_ROUNDING_MODE =
  "%s(): Invalid rounding mode";
const char* const cs_GMP_FAILED_TO_ALTER_BIT =
  "%s(): Failed to alter bit";
const char* const cs_GMP_INVALID_STARTING_INDEX_IS_NEGATIVE =
  "%s(): Starting index must be greater than or equal to zero";
const char* const cs_GMP_COULD_NOT_UNSERIALIZE_NUMBER =
  "Could not unserialize number";
const char* const cs_GMP_COULD_NOT_UNSERIALIZE_PROPERTIES =
  "Could not unserialize properties";
const char* const cs_GMP_ERROR_EVEN_ROOT_NEGATIVE_NUMBER =
  "%s(): Can't take even root of negative number";

// Function name strings
const char* const cs_GMP_FUNC_NAME_GMP_ABS            = "gmp_abs";
const char* const cs_GMP_FUNC_NAME_GMP_ADD            = "gmp_add";
const char* const cs_GMP_FUNC_NAME_GMP_AND            = "gmp_add";
const char* const cs_GMP_FUNC_NAME_GMP_CLRBIT         = "gmp_clrbit";
const char* const cs_GMP_FUNC_NAME_GMP_CMP            = "gmp_cmp";
const char* const cs_GMP_FUNC_NAME_GMP_COM            = "gmp_com";
const char* const cs_GMP_FUNC_NAME_GMP_DIV_Q          = "gmp_div_q";
const char* const cs_GMP_FUNC_NAME_GMP_DIV_R          = "gmp_div_r";
const char* const cs_GMP_FUNC_NAME_GMP_DIV_QR         = "gmp_div_qr";
const char* const cs_GMP_FUNC_NAME_GMP_DIVEXACT       = "gmp_divexact";
const char* const cs_GMP_FUNC_NAME_GMP_FACT           = "gmp_fact";
const char* const cs_GMP_FUNC_NAME_GMP_GCD            = "gmp_gcd";
const char* const cs_GMP_FUNC_NAME_GMP_GCDEXCT        = "gmp_gcdexct";
const char* const cs_GMP_FUNC_NAME_GMP_HAMDIST        = "gmp_hamdist";
const char* const cs_GMP_FUNC_NAME_GMP_INIT           = "gmp_init";
const char* const cs_GMP_FUNC_NAME_GMP_INTVAL         = "gmp_intval";
const char* const cs_GMP_FUNC_NAME_GMP_INVERT         = "gmp_invert";
const char* const cs_GMP_FUNC_NAME_GMP_JACOBI         = "gmp_jacobi";
const char* const cs_GMP_FUNC_NAME_GMP_LEGENDRE       = "gmp_legendre";
const char* const cs_GMP_FUNC_NAME_GMP_MOD            = "gmp_mod";
const char* const cs_GMP_FUNC_NAME_GMP_MUL            = "gmp_mul";
const char* const cs_GMP_FUNC_NAME_GMP_NEG            = "gmp_neg";
const char* const cs_GMP_FUNC_NAME_GMP_NEXTPRIME      = "gmp_nextprime";
const char* const cs_GMP_FUNC_NAME_GMP_OR             = "gmp_or";
const char* const cs_GMP_FUNC_NAME_GMP_PERFECT_SQUARE = "gmp_perfect_square";
const char* const cs_GMP_FUNC_NAME_GMP_POPCOUNT       = "gmp_popcount";
const char* const cs_GMP_FUNC_NAME_GMP_POW            = "gmp_pow";
const char* const cs_GMP_FUNC_NAME_GMP_POWM           = "gmp_powm";
const char* const cs_GMP_FUNC_NAME_GMP_PROB_PRIME     = "gmp_prob_prime";
const char* const cs_GMP_FUNC_NAME_GMP_RANDOM         = "gmp_random";
const char* const cs_GMP_FUNC_NAME_GMP_ROOT           = "gmp_root";
const char* const cs_GMP_FUNC_NAME_GMP_ROOTREM        = "gmp_rootrem";
const char* const cs_GMP_FUNC_NAME_GMP_SCAN0          = "gmp_scan0";
const char* const cs_GMP_FUNC_NAME_GMP_SCAN1          = "gmp_scan1";
const char* const cs_GMP_FUNC_NAME_GMP_SETBIT         = "gmp_setbit";
const char* const cs_GMP_FUNC_NAME_GMP_SIGN           = "gmp_sign";
const char* const cs_GMP_FUNC_NAME_GMP_SQRT           = "gmp_sqrt";
const char* const cs_GMP_FUNC_NAME_GMP_SQRTREM        = "gmp_sqrtrem";
const char* const cs_GMP_FUNC_NAME_GMP_STRVAL         = "gmp_strval";
const char* const cs_GMP_FUNC_NAME_GMP_SUB            = "gmp_sub";
const char* const cs_GMP_FUNC_NAME_GMP_TESTBIT        = "gmp_testbit";
const char* const cs_GMP_FUNC_NAME_GMP_XOR            = "gmp_xor";

///////////////////////////////////////////////////////////////////////////////
// classes

class GMPData {
public:
                 GMPData()
                 : m_isInit(false) {}
  virtual       ~GMPData() { close(); }
  GMPData&       operator=(const GMPData& source);

  void           close();
  void           setGMPMpz(const mpz_t data);
  mpz_t&         getGMPMpz() { return m_gmpMpz; }

private:
  bool           m_isInit;
  mpz_t          m_gmpMpz;
};


class GMP {
  static void initClass() {
    cls = Unit::lookupClass(s_GMP_GMP.get());
  }

public:
  static Object allocObject() {
    if (UNLIKELY(cls == nullptr)) {
      initClass();
    }
    return Object{cls};
  }

  static Object allocObject(const Variant& arg) {
    Object ret = allocObject();

    TypedValue dummy;
    g_context->invokeFunc(&dummy,
                          cls->getCtor(),
                          make_packed_array(arg),
                          ret.get());
    tvRefcountedDecRef(&dummy);

    return ret;
  }

  static HPHP::Class* cls;
};

} /* namespace HPHP */

#endif /* HPHP_RUNTIME_EXT_GMP_GMP_H */
