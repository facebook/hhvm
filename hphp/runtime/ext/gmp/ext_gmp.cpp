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

#include "hphp/runtime/base/base-includes.h"

#include "gmp.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// header

#define GMP_ROUND_ZERO      0
#define GMP_ROUND_PLUSINF   1
#define GMP_ROUND_MINUSINF  2

#define GMP_DEFAULT_BASE    10


/* The maximum base for input and output conversions is 62 from GMP 4.2
* onwards. */
#if ((__GNU_MP_VERSION >= 5) || (__GNU_MP_VERSION >= 4 \
                                 && __GNU_MP_VERSION_MINOR >= 2))
#  define GMP_MAX_BASE 62
#else
#  define GMP_MAX_BASE 36
#endif

const StaticString s_gmp_s("s");
const StaticString s_gmp_t("t");
const StaticString s_gmp_g("g");
const StaticString s_gmp_0("0");
const StaticString s_gmp_1("1");
const StaticString s_GMP_MAX_BASE("GMP_MAX_BASE");
const StaticString s_GMP_ROUND_ZERO("GMP_ROUND_ZERO");
const StaticString s_GMP_ROUND_PLUSINF("GMP_ROUND_PLUSINF");
const StaticString s_GMP_ROUND_MINUSINF("GMP_ROUND_MINUSINF");
const StaticString s_GMP_VERSION("GMP_VERSION");
const StaticString k_GMP_VERSION(gmp_version);

const char* const cs_GMP_INVALID_TYPE =
  "%s(): Unable to convert variable to GMP - wrong type!";
const char* const cs_GMP_INVALID_ROUNDING_MODE =
  "%s(): Invalid rounding mode";
const char* const cs_GMP_INVALID_VALUE_MUST_NOT_BE_ZERO =
  "%s(): Zero operand not allowed";
const char* const cs_GMP_INVALID_VALUE_MUST_BE_POSITIVE =
  "%s(): Value operand not allowed";
const char* const cs_GMP_INVALID_INDEX_IS_NEGATIVE =
  "%s(): Index must be greater than or equal to zero";
const char* const cs_GMP_INVALID_BASE_VALUE =
  "%s(): Bad base for conversion: %ld (should be between 2 and %d)";
const char* const cs_GMP_INVALID_EXPONENT_MUST_BE_POSITIVE =
  "%s(): Exponent must not be positive negative";

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
// class
class GMPResource : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(GMPResource)
  CLASSNAME_IS("GMP integer")
  virtual const String& o_getClassNameHook() const { return classnameof(); }

public:
  explicit GMPResource(mpz_t data) { mpz_init_set(m_gmpMpz, data); }
  virtual ~GMPResource() { close(); }
  void     close() { mpz_clear(m_gmpMpz); }
  mpz_t&   getData() { return m_gmpMpz; }

private:
  mpz_t    m_gmpMpz;
};

void GMPResource::sweep() { close(); }

///////////////////////////////////////////////////////////////////////////////
// functions
static bool variantToGMPData(mpz_t gmpData,
                             const Variant& data,
                             const int64_t paramBase = -1) {
  if (data.isResource()) {
    auto gmpRes = data.toResource().getTyped<GMPResource>(false, true);
    if (!gmpRes) {
      return false;
    }
    mpz_init_set(gmpData, gmpRes->getData());
  } else if (data.isString()) {
    String dataString = data.toString();
    int64_t base = paramBase;
    int strLength = dataString.length();

    //Figure out what Base to use based on the ~*data*~
    if (strLength > 1 && dataString[0] == '0') {
      if (strLength > 2) {
        if (dataString[1] == 'x' || dataString[1] == 'X') {
          base = 16;
          dataString = dataString.substr(2);
        } else if (base < 12 && (dataString[1] == 'b'
                                 || dataString[1] == 'B')) {
          base = 2;
          dataString = dataString.substr(2);
        }
      }
      if (base == -1) {
        base = 8;
      }
    } else if (strLength == 0) {
      dataString = s_gmp_0.get();
    }

    if (base == -1) {
      base = GMP_DEFAULT_BASE;
    }

    if (mpz_init_set_str(gmpData, dataString.c_str(), base) == -1) {
      mpz_clear(gmpData);
      return false;
    }
  } else if (data.isNumeric() || data.isBoolean()) {
    mpz_init_set_si(gmpData, data.toInt64());
  } else {
    return false;
  }

  return true;
}


static Variant HHVM_FUNCTION(gmp_abs,
                             const Variant& data) {
  mpz_t gmpReturn, gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_ABS);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_abs(gmpReturn, gmpData);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpReturn);
  mpz_clear(gmpData);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_add,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_ADD);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_ADD);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_add(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_and,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_AND);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_AND);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_and(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static void HHVM_FUNCTION(gmp_clrbit,
                          VRefParam& data,
                          int64_t index) {

  if (index < 0) {
    raise_warning(cs_GMP_INVALID_INDEX_IS_NEGATIVE,
                  cs_GMP_FUNC_NAME_GMP_CLRBIT);
    return;
  }

  auto gmpRes = data.toResource().getTyped<GMPResource>();
  if (!gmpRes) {
    raise_warning("%s: Failed to clear bit", cs_GMP_FUNC_NAME_GMP_CLRBIT);
    return;
  }

  mpz_clrbit(gmpRes->getData(), index);
}


static Variant HHVM_FUNCTION(gmp_cmp,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_CMP);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_CMP);
    return false;
  }

  int64_t cmp = mpz_cmp(gmpDataA, gmpDataB);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);

  return cmp;
}


static Variant HHVM_FUNCTION(gmp_com,
                             const Variant& data) {
  mpz_t gmpReturn, gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_COM);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_com(gmpReturn, gmpData);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpReturn);
  mpz_clear(gmpData);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_div_q,
                             const Variant& dataA,
                             const Variant& dataB,
                             int64_t round = GMP_ROUND_ZERO) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_DIV_Q);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_DIV_Q);
    return false;
  }

  if (mpz_sgn(gmpDataB) == 0) {
    mpz_clear(gmpDataA);
    mpz_clear(gmpDataB);

    raise_warning(cs_GMP_INVALID_VALUE_MUST_NOT_BE_ZERO,
                  cs_GMP_FUNC_NAME_GMP_DIV_Q);
    return false;
  }

  mpz_init(gmpReturn);
  switch (round)
  {
    case GMP_ROUND_ZERO:
      mpz_tdiv_q(gmpReturn, gmpDataA, gmpDataB);
      break;

    case GMP_ROUND_PLUSINF:
      mpz_cdiv_q(gmpReturn, gmpDataA, gmpDataB);
      break;

    case GMP_ROUND_MINUSINF:
      mpz_fdiv_q(gmpReturn, gmpDataA, gmpDataB);
      break;

    default:
      mpz_clear(gmpDataA);
      mpz_clear(gmpDataB);
      mpz_clear(gmpReturn);

      raise_warning(cs_GMP_INVALID_ROUNDING_MODE, cs_GMP_FUNC_NAME_GMP_DIV_Q);
      return false;
  }

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_div_qr,
                             const Variant& dataA,
                             const Variant& dataB,
                             int64_t round = GMP_ROUND_ZERO) {
  mpz_t gmpDataA, gmpDataB, gmpReturnQ, gmpReturnR;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_DIV_QR);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_DIV_QR);
    return false;
  }

  if (mpz_sgn(gmpDataB) == 0) {
    mpz_clear(gmpDataA);
    mpz_clear(gmpDataB);

    raise_warning(cs_GMP_INVALID_VALUE_MUST_NOT_BE_ZERO,
                  cs_GMP_FUNC_NAME_GMP_DIV_QR);
    return false;
  }

  mpz_init(gmpReturnQ);
  mpz_init(gmpReturnR);

  switch (round)
  {
    case GMP_ROUND_ZERO:
      mpz_tdiv_qr(gmpReturnQ, gmpReturnR, gmpDataA, gmpDataB);
      break;

    case GMP_ROUND_PLUSINF:
      mpz_cdiv_qr(gmpReturnQ, gmpReturnR, gmpDataA, gmpDataB);
      break;

    case GMP_ROUND_MINUSINF:
      mpz_fdiv_qr(gmpReturnQ, gmpReturnR, gmpDataA, gmpDataB);
      break;

    default:
      mpz_clear(gmpDataA);
      mpz_clear(gmpDataB);
      mpz_clear(gmpReturnQ);
      mpz_clear(gmpReturnR);

      raise_warning(cs_GMP_INVALID_ROUNDING_MODE, cs_GMP_FUNC_NAME_GMP_DIV_QR);
      return false;
  }

  ArrayInit returnArray(2, ArrayInit::Map{});
  returnArray.set(0, NEWOBJ(GMPResource)(gmpReturnQ));
  returnArray.set(1, NEWOBJ(GMPResource)(gmpReturnR));

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturnQ);
  mpz_clear(gmpReturnR);

  return returnArray.toVariant();
}


static Variant HHVM_FUNCTION(gmp_div_r,
                             const Variant& dataA,
                             const Variant& dataB,
                             int64_t round = GMP_ROUND_ZERO) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_DIV_R);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_DIV_R);
    return false;
  }

  if (mpz_sgn(gmpDataB) == 0) {
    mpz_clear(gmpDataA);
    mpz_clear(gmpDataB);

    raise_warning(cs_GMP_INVALID_VALUE_MUST_NOT_BE_ZERO,
                  cs_GMP_FUNC_NAME_GMP_DIV_R);
    return false;
  }

  mpz_init(gmpReturn);
  switch (round) {
  case GMP_ROUND_ZERO:
    mpz_tdiv_r(gmpReturn, gmpDataA, gmpDataB);
    break;

  case GMP_ROUND_PLUSINF:
    mpz_cdiv_r(gmpReturn, gmpDataA, gmpDataB);
    break;

  case GMP_ROUND_MINUSINF:
    mpz_fdiv_r(gmpReturn, gmpDataA, gmpDataB);
    break;

  default:
    mpz_clear(gmpDataA);
    mpz_clear(gmpDataB);
    mpz_clear(gmpReturn);

    raise_warning(cs_GMP_INVALID_ROUNDING_MODE, cs_GMP_FUNC_NAME_GMP_DIV_R);
    return false;
  }

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_divexact,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_DIVEXACT);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_DIVEXACT);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_divexact(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_fact,
                             const Variant& data) {
  mpz_t gmpReturn, gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_FACT);
    return false;
  }

  if (mpz_sgn(gmpData) < 0) {
    mpz_clear(gmpData);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_FACT);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_fac_ui(gmpReturn, mpz_get_ui(gmpData));

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpData);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_gcd,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_GCD);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_GCD);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_gcd(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_gcdext,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturnG, gmpReturnS, gmpReturnT;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_GCDEXCT);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_GCDEXCT);
    return false;
  }

  mpz_init(gmpReturnG);
  mpz_init(gmpReturnS);
  mpz_init(gmpReturnT);

  mpz_gcdext(gmpReturnG, gmpReturnS, gmpReturnT, gmpDataA, gmpDataB);

  ArrayInit returnArray(3, ArrayInit::Map{});
  returnArray.set(s_gmp_g, NEWOBJ(GMPResource)(gmpReturnG));
  returnArray.set(s_gmp_s, NEWOBJ(GMPResource)(gmpReturnS));
  returnArray.set(s_gmp_t, NEWOBJ(GMPResource)(gmpReturnT));

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturnG);
  mpz_clear(gmpReturnS);
  mpz_clear(gmpReturnT);

  return returnArray.toVariant();
}


static Variant HHVM_FUNCTION(gmp_hamdist,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_HAMDIST);
    return false;
  }
  if (mpz_sgn(gmpDataA) < 0) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_VALUE_MUST_BE_POSITIVE,
                  cs_GMP_FUNC_NAME_GMP_HAMDIST);
    return false;
  }

  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_HAMDIST);
    return false;
  }
  if (mpz_sgn(gmpDataB) < 0) {
    mpz_clear(gmpDataA);
    mpz_clear(gmpDataB);

    raise_warning(cs_GMP_INVALID_VALUE_MUST_BE_POSITIVE,
                  cs_GMP_FUNC_NAME_GMP_HAMDIST);
    return false;
  }

  int64_t hamdist = mpz_hamdist(gmpDataA, gmpDataB);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);

  return hamdist;
}


static Variant HHVM_FUNCTION(gmp_init,
                             const Variant& data,
                             const int64_t base = -1) {
  mpz_t gmpData;

  if (base < -1 || base == 0 || base == 1 || base > GMP_MAX_BASE) {
    raise_warning(cs_GMP_INVALID_BASE_VALUE,
                  cs_GMP_FUNC_NAME_GMP_INIT,
                  base,
                  GMP_MAX_BASE);
    return false;
  }

  if (!variantToGMPData(gmpData, data, base)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_INIT);
    return false;
  }

  Variant ret = NEWOBJ(GMPResource)(gmpData);

  mpz_clear(gmpData);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_intval,
                             const Variant& data) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_INTVAL);
    return false;
  }

  int64_t result = mpz_get_si(gmpData);

  mpz_clear(gmpData);

  return result;
}


static Variant HHVM_FUNCTION(gmp_invert,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_INVERT);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_INVERT);
    return false;
  }

  mpz_init(gmpReturn);
  if (!mpz_invert(gmpReturn, gmpDataA, gmpDataB)) {
    mpz_clear(gmpDataA);
    mpz_clear(gmpDataB);
    mpz_clear(gmpReturn);
    return false;
  }

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_jacobi,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_JACOBI);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_JACOBI);
    return false;
  }

  int64_t result = mpz_jacobi(gmpDataA, gmpDataB);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);

  return result;
}


static Variant HHVM_FUNCTION(gmp_legendre,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_LEGENDRE);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_LEGENDRE);
    return false;
  }

  int64_t result = mpz_legendre(gmpDataA, gmpDataB);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);

  return result;
}


static Variant HHVM_FUNCTION(gmp_mod,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_MOD);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_MOD);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_mod(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_mul,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_MUL);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_MUL);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_mul(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_neg,
                             const Variant& data) {
  mpz_t gmpReturn, gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_NEG);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_neg(gmpReturn, gmpData);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpData);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_nextprime,
                             const Variant& data) {
  mpz_t gmpReturn, gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_NEXTPRIME);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_nextprime(gmpReturn, gmpData);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpData);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_or,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_OR);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_OR);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_ior(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static bool HHVM_FUNCTION(gmp_perfect_square,
                          const Variant& data) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_PERFECT_SQUARE);
    return false;
  }

  bool isPerfectSquare = (mpz_perfect_square_p(gmpData) != 0);

  mpz_clear(gmpData);

  return isPerfectSquare;
}


static Variant HHVM_FUNCTION(gmp_popcount,
                             const Variant& data) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_POPCOUNT);
    return false;
  }

  int64_t population = mpz_popcount(gmpData);
  mpz_clear(gmpData);

  return population;
}


static Variant HHVM_FUNCTION(gmp_pow,
                             const Variant& data,
                             int64_t exp) {
  mpz_t gmpReturn, gmpData;

  if (exp < 0) {
    raise_warning(cs_GMP_INVALID_EXPONENT_MUST_BE_POSITIVE,
                  cs_GMP_FUNC_NAME_GMP_POW);
    return false;
  }

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_POW);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_pow_ui(gmpReturn, gmpData, exp);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpData);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_powm,
                             const Variant& dataA,
                             const Variant& dataB,
                             const Variant& dataC) {
  mpz_t gmpDataA, gmpDataB, gmpDataC, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_POWM);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_POWM);
    return false;
  }

  if (mpz_sgn(gmpDataB) < 0) {
    mpz_clear(gmpDataA);
    mpz_clear(gmpDataB);

    raise_warning(cs_GMP_INVALID_EXPONENT_MUST_BE_POSITIVE,
                  cs_GMP_FUNC_NAME_GMP_POWM);
    return false;
  }

  if (!variantToGMPData(gmpDataC, dataC)) {
    mpz_clear(gmpDataA);
    mpz_clear(gmpDataB);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_POWM);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_powm(gmpReturn, gmpDataA, gmpDataB, gmpDataC);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpDataC);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_prob_prime,
                             const Variant& data,
                             int64_t reps = 10) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_PROB_PRIME);
    return false;
  }

  int64_t probPrime = mpz_probab_prime_p(gmpData, reps);

  mpz_clear(gmpData);

  return probPrime;
}


static void HHVM_FUNCTION(gmp_random,
                          int64_t limiter) {
  throw_not_implemented(cs_GMP_FUNC_NAME_GMP_RANDOM);
}


static Variant HHVM_FUNCTION(gmp_scan0,
                             const Variant& data,
                             int64_t start) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_SCAN0);
    return false;
  }

  int64_t foundBit = mpz_scan0(gmpData, start);

  mpz_clear(gmpData);

  return foundBit;
}


static Variant HHVM_FUNCTION(gmp_scan1,
                             const Variant& data,
                             int64_t start) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_SCAN1);
    return false;
  }

  int64_t foundBit = mpz_scan1(gmpData, start);

  mpz_clear(gmpData);

  return foundBit;
}


static void HHVM_FUNCTION(gmp_setbit,
                          VRefParam& data,
                          int64_t index,
                          bool bitOn /* = true*/) {
  if (index < 0) {
    raise_warning(cs_GMP_INVALID_INDEX_IS_NEGATIVE,
                  cs_GMP_FUNC_NAME_GMP_SETBIT);
    return;
  }

  auto gmpRes = data.toResource().getTyped<GMPResource>();
  if (!gmpRes) {
    raise_warning("%s: Failed to alter bit", cs_GMP_FUNC_NAME_GMP_SETBIT);
    return;
  }

  if (bitOn) {
    mpz_setbit(gmpRes->getData(), index);
  } else {
    mpz_clrbit(gmpRes->getData(), index);
  }
}


static Variant HHVM_FUNCTION(gmp_sign,
                             const Variant& data) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_SIGN);
    return false;
  }

  int64_t sign = mpz_sgn(gmpData);

  mpz_clear(gmpData);

  return sign;
}


static Variant HHVM_FUNCTION(gmp_sqrt,
                             const Variant& data) {
  mpz_t gmpReturn, gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_SQRT);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_sqrt(gmpReturn, gmpData);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpData);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_sqrtrem,
                             const Variant& data) {
  mpz_t gmpData, gmpSquareRoot, gmpRemainder;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_SQRTREM);
    return false;
  }

  mpz_init(gmpSquareRoot);
  mpz_init(gmpRemainder);

  mpz_sqrtrem(gmpSquareRoot, gmpRemainder, gmpData);

  ArrayInit returnArray(2, ArrayInit::Map{});
  returnArray.set(0, NEWOBJ(GMPResource)(gmpSquareRoot));
  returnArray.set(1, NEWOBJ(GMPResource)(gmpRemainder));

  mpz_clear(gmpData);
  mpz_clear(gmpSquareRoot);
  mpz_clear(gmpRemainder);

  return returnArray.toVariant();
}


static String HHVM_FUNCTION(gmp_strval,
                            const Variant& data,
                            const int64_t base = 10) {
  mpz_t gmpData;

  if (base < 1 || base > GMP_MAX_BASE) {
    raise_warning(cs_GMP_INVALID_BASE_VALUE,
                  cs_GMP_FUNC_NAME_GMP_STRVAL,
                  base,
                  GMP_MAX_BASE);
    return s_gmp_0;
  }

  if (!variantToGMPData(gmpData, data, base)) {
    return s_gmp_0;
  }

  int charLength = mpz_sizeinbase(gmpData, abs(base)) + 1;
  if (mpz_sgn(gmpData) < 0) {
    ++charLength;
  }

  char *charStr = (char*) smart_malloc(charLength);
  if (!mpz_get_str(charStr, base, gmpData)) {
    smart_free(charStr);
    mpz_clear(gmpData);

    return s_gmp_0;
  }

  String returnValue(charStr);

  smart_free(charStr);
  mpz_clear(gmpData);

  return returnValue;
}


static Variant HHVM_FUNCTION(gmp_sub,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_SUB);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_SUB);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_sub(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static bool HHVM_FUNCTION(gmp_testbit,
                          const Variant& data,
                          int64_t index) {

  if (index < 0) {
    raise_warning(cs_GMP_INVALID_INDEX_IS_NEGATIVE,
                  cs_GMP_FUNC_NAME_GMP_TESTBIT);
    return false;
  }

  if (data.isResource()) {
    auto gmpRes = data.toResource().getTyped<GMPResource>();
    return mpz_tstbit(gmpRes->getData(), index);
  }

  mpz_t gmpData;
  if (!variantToGMPData(gmpData, data)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_TESTBIT);
    return false;
  }

  bool isBitSet = mpz_tstbit(gmpData, index);
  mpz_clear(gmpData);

  return isBitSet;
}


static Variant HHVM_FUNCTION(gmp_xor,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_XOR);
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning(cs_GMP_INVALID_TYPE, cs_GMP_FUNC_NAME_GMP_XOR);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_xor(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = NEWOBJ(GMPResource)(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


///////////////////////////////////////////////////////////////////////////////
// extension

class GMPExtension : public Extension {
public:
  GMPExtension() : Extension("gmp", "1.0.0-hhvm") { };
  virtual void moduleInit() {
    Native::registerConstant<KindOfInt64>(
      s_GMP_MAX_BASE.get(), GMP_MAX_BASE
    );
    Native::registerConstant<KindOfInt64>(
      s_GMP_ROUND_ZERO.get(), GMP_ROUND_ZERO
    );
    Native::registerConstant<KindOfInt64>(
      s_GMP_ROUND_PLUSINF.get(), GMP_ROUND_PLUSINF
    );
    Native::registerConstant<KindOfInt64>(
      s_GMP_ROUND_MINUSINF.get(), GMP_ROUND_MINUSINF
    );
    Native::registerConstant<KindOfStaticString>(
      s_GMP_VERSION.get(), k_GMP_VERSION.get()
    );

    HHVM_FE(gmp_abs);
    HHVM_FE(gmp_add);
    HHVM_FE(gmp_and);
    HHVM_FE(gmp_clrbit);
    HHVM_FE(gmp_cmp);
    HHVM_FE(gmp_com);
    HHVM_FE(gmp_div_q);
    HHVM_FALIAS(gmp_div, gmp_div_q);
    HHVM_FE(gmp_div_qr);
    HHVM_FE(gmp_div_r);
    HHVM_FE(gmp_divexact);
    HHVM_FE(gmp_fact);
    HHVM_FE(gmp_gcd);
    HHVM_FE(gmp_gcdext);
    HHVM_FE(gmp_hamdist);
    HHVM_FE(gmp_init);
    HHVM_FE(gmp_intval);
    HHVM_FE(gmp_invert);
    HHVM_FE(gmp_jacobi);
    HHVM_FE(gmp_legendre);
    HHVM_FE(gmp_mod);
    HHVM_FE(gmp_mul);
    HHVM_FE(gmp_neg);
    HHVM_FE(gmp_nextprime);
    HHVM_FE(gmp_or);
    HHVM_FE(gmp_perfect_square);
    HHVM_FE(gmp_popcount);
    HHVM_FE(gmp_pow);
    HHVM_FE(gmp_powm);
    HHVM_FE(gmp_prob_prime);
    HHVM_FE(gmp_random);
    HHVM_FE(gmp_scan0);
    HHVM_FE(gmp_scan1);
    HHVM_FE(gmp_setbit);
    HHVM_FE(gmp_sign);
    HHVM_FE(gmp_sqrt);
    HHVM_FE(gmp_sqrtrem);
    HHVM_FE(gmp_strval);
    HHVM_FE(gmp_sub);
    HHVM_FE(gmp_testbit);
    HHVM_FE(gmp_xor);

    loadSystemlib();
  }
} s_gmp_extension;


///////////////////////////////////////////////////////////////////////////////
}
