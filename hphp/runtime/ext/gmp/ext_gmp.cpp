//#ifdef HAVE_LIBGMP
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

#define GMP_ROUND_ZERO 0
#define GMP_ROUND_PLUSINF 1
#define GMP_ROUND_MINUSINF 2

class GMPResource : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(GMPResource)
  CLASSNAME_IS("GMP integer")
  virtual const String& o_getClassNameHook() const { return classnameof(); }

public:
  explicit GMPResource(mpz_t data) { mpz_init_set(m_gmpMpz, data); }
  virtual ~GMPResource() { close(); }
  void close() { mpz_clear(m_gmpMpz); }
  mpz_t& getData() { return m_gmpMpz; }

private:
  mpz_t m_gmpMpz;
};
void GMPResource::sweep() { close(); }


///////////////////////////////////////////////////////////////////////////////
// functions
bool variantToGMPData(mpz_t gmpData,
                      const Variant& data,
                      const int64_t base = 10) {
  if (data.isResource()) {
    auto gmpRes = data.toResource().getTyped<GMPResource>();
    mpz_init_set(gmpData, gmpRes->getData());
  } else if (data.isString()) {
    String dataString = data.toString();

    if (mpz_init_set_str(gmpData, dataString.toCppString().c_str(), base) != 0) {
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


Variant HHVM_FUNCTION(gmp_abs,
                     const Variant& data) {
  mpz_t gmpData, gmpReturn;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_abs: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_abs(gmpReturn, gmpData);
  mpz_clear(gmpData);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_add,
                     const Variant& dataA,
                     const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_add: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_add: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_add(gmpReturn, gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_and,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_and: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_and: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_and(gmpReturn, gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


void HHVM_FUNCTION(gmp_clrbit,
                   Resource& data,
                   int64_t index) {
  auto gmpRes = data.getTyped<GMPResource>();
  mpz_clrbit(gmpRes->getData(), index);
}


Variant HHVM_FUNCTION(gmp_cmp,
                     const Variant& dataA,
                     const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB;
  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_and: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_and: Unable to convert variable to GMP - wrong type");
    return false;
  }

  int64_t cmp = mpz_cmp(gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);
  return cmp;
}


Variant HHVM_FUNCTION(gmp_com,
                     const Variant& data) {
  mpz_t gmpData, gmpReturn;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_com: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_com(gmpReturn, gmpData);
  mpz_clear(gmpData);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_div_q,
                      const Variant& dataA,
                      const Variant& dataB,
                      int64_t round = GMP_ROUND_ZERO) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_div_q: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_div_q: Unable to convert variable to GMP - wrong type");
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
      mpz_clears(gmpDataA, gmpDataB, gmpReturn, NULL);

      raise_warning("gmp_div_q: Invalid rounding mode");
      return false;
  }
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_div_qr,
                      const Variant& dataA,
                      const Variant& dataB,
                      int64_t round = GMP_ROUND_ZERO) {
  mpz_t gmpDataA, gmpDataB, gmpReturnQ, gmpReturnR;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_div_qr: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_div_qr: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_inits(gmpReturnQ, gmpReturnR);
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
      mpz_clears(gmpDataA, gmpDataB, gmpReturnQ, gmpReturnR, NULL);

      raise_warning("gmp_div_qr: Invalid rounding mode");
      return false;
  }
  mpz_clears(gmpDataA, gmpDataB, NULL);

  Array returnArray = Array::Create();
  returnArray.set(String("q"), NEWOBJ(GMPResource)(gmpReturnQ));
  returnArray.set(String("r"), NEWOBJ(GMPResource)(gmpReturnR));

  return returnArray;
}


Variant HHVM_FUNCTION(gmp_div_r,
                      const Variant& dataA,
                      const Variant& dataB,
                      int64_t round = GMP_ROUND_ZERO) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_div_r: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_div_r: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  switch (round)
  {
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
      mpz_clears(gmpDataA, gmpDataB, gmpReturn, NULL);

      raise_warning("gmp_div_r: Invalid rounding mode");
      return false;
  }
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_divexact,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_divexact: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_divexact: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_divexact(gmpReturn, gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_fact,
                      const Variant& data) {
  mpz_t gmpData, gmpReturn;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_fact: Unable to convert variable to GMP - wrong type");
    return false;
  }

  if (mpz_sgn(gmpData) < 0) {
    mpz_clear(gmpData);

    raise_warning("gmp_fact: Number has to be greater than or equal to 0");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_fac_ui(gmpReturn, mpz_get_ui(gmpData));
  mpz_clear(gmpData);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_gcd,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_gcd: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_gcd: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_gcd(gmpReturn, gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_gcdext,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturnG, gmpReturnS, gmpReturnT;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_gcdext: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_gcdext: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_inits(gmpReturnG, gmpReturnS, gmpReturnT, NULL);
  mpz_gcdext(gmpReturnG, gmpReturnS, gmpReturnT, gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  Array returnArray = Array::Create();
  returnArray.set(String("g"), NEWOBJ(GMPResource)(gmpReturnG));
  returnArray.set(String("s"), NEWOBJ(GMPResource)(gmpReturnS));
  returnArray.set(String("t"), NEWOBJ(GMPResource)(gmpReturnT));

  return returnArray;
}


Variant HHVM_FUNCTION(gmp_hamdist,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_hamdist: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (mpz_sgn(gmpDataA) < 0) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_hamdist: Number has to be greater than or equal to 0");
    return false;
  }

  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_hamdist: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (mpz_sgn(gmpDataB) < 0) {
    mpz_clears(gmpDataA, gmpDataB, NULL);

    raise_warning("gmp_hamdist: Number has to be greater than or equal to 0");
    return false;
  }

  int64_t hamdist = mpz_hamdist(gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);
  return hamdist;
}


Variant HHVM_FUNCTION(gmp_init,
                      const Variant& data,
                      const int64_t base = 0) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data, base)) {
    raise_warning("gmp_init: Unable to convert variable to GMP - wrong type");
    return false;
  }

  return NEWOBJ(GMPResource)(gmpData);
}


Variant HHVM_FUNCTION(gmp_intval,
                      const Variant& data) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_intval: Unable to convert variable to GMP - wrong type");
    return false;
  }

  int64_t result = mpz_get_si(gmpData);
  mpz_clear(gmpData);

  return result;
}


Variant HHVM_FUNCTION(gmp_invert,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_invert: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_invert: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_invert(gmpReturn, gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_jacobi,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_jacobi: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_jacobi: Unable to convert variable to GMP - wrong type");
    return false;
  }

  int64_t result = mpz_jacobi(gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return result;
}


Variant HHVM_FUNCTION(gmp_legendre,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_legendre: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_legendre: Unable to convert variable to GMP - wrong type");
    return false;
  }

  int64_t result = mpz_legendre(gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return result;
}


Variant HHVM_FUNCTION(gmp_mod,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_mod: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_mod: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_mod(gmpReturn, gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_mul,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_mul: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_mul: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_mul(gmpReturn, gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_neg,
                      const Variant& data) {
  mpz_t gmpData, gmpReturn;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_neg: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_neg(gmpReturn, gmpData);
  mpz_clear(gmpData);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_nextprime,
                      const Variant& data) {
  mpz_t gmpData, gmpReturn;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_nextprime: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_nextprime(gmpReturn, gmpData);
  mpz_clear(gmpData);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_or,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_or: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_or: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_ior(gmpReturn, gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


bool HHVM_FUNCTION(gmp_perfect_square,
                   const Variant& data) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_perfect_square: Unable to convert variable to GMP - wrong type");
    return false;
  }

  bool isPerfectSquare = (mpz_perfect_square_p(gmpData) != 0);
  mpz_clear(gmpData);

  return isPerfectSquare;
}


Variant HHVM_FUNCTION(gmp_popcount,
                      const Variant& data) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_popcount: Unable to convert variable to GMP - wrong type");
    return false;
  }

  int64_t population = mpz_popcount(gmpData);
  mpz_clear(gmpData);

  return population;
}


Variant HHVM_FUNCTION(gmp_pow,
                      const Variant& data,
                      int64_t exp) {
  mpz_t gmpData, gmpReturn;

  if (exp < 1) {
    raise_warning("gmp_pow: Exponent must be positive number");
    return false;
  }

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_pow: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_pow_ui(gmpReturn, gmpData, exp);
  mpz_clear(gmpData);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_powm,
                     const Variant& dataA,
                     const Variant& dataB,
                     const Variant& dataC) {
  mpz_t gmpDataA, gmpDataB, gmpDataC, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_powm: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_powm: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataC, dataC)) {
    mpz_clears(gmpDataA, gmpDataB, NULL);

    raise_warning("gmp_powm: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_powm(gmpReturn, gmpDataA, gmpDataB, gmpDataC);
  mpz_clears(gmpDataA, gmpDataB, gmpDataC, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_prob_prime,
                      const Variant& data,
                      const Variant& reps = 10) {
  mpz_t gmpData, gmpReps;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_prob_prime: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpReps, reps)) {
    mpz_clear(gmpData);

    raise_warning("gmp_prob_prime: Unable to convert variable to GMP - wrong type");
    return false;
  }

  int64_t probPrime = mpz_probab_prime_p(gmpData, mpz_get_si(gmpReps));
  mpz_clears(gmpData, gmpReps, NULL);

  return probPrime;
}


Variant HHVM_FUNCTION(gmp_scan0,
                      const Variant& data,
                      int64_t start) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_scan0: Unable to convert variable to GMP - wrong type");
    return false;
  }

  int64_t foundBit = mpz_scan0(gmpData, start);
  mpz_clear(gmpData);

  return foundBit;
}


Variant HHVM_FUNCTION(gmp_scan1,
                      const Variant& data,
                      int64_t start) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_scan1: Unable to convert variable to GMP - wrong type");
    return false;
  }

  int64_t foundBit = mpz_scan1(gmpData, start);
  mpz_clear(gmpData);

  return foundBit;
}


void HHVM_FUNCTION(gmp_setbit,
                   Resource& data,
                   int64_t index,
                   bool bitOn = true) {
  auto gmpRes = data.getTyped<GMPResource>();
  if (bitOn) {
    mpz_setbit(gmpRes->getData(), index);
  } else {
    mpz_clrbit(gmpRes->getData(), index);
  }
}


Variant HHVM_FUNCTION(gmp_sign,
                      const Variant& data) {
  mpz_t gmpData;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_sign: Unable to convert variable to GMP - wrong type");
    return false;
  }

  int64_t sign = mpz_sgn(gmpData);
  mpz_clear(gmpData);

  return sign;
}


Variant HHVM_FUNCTION(gmp_sqrt,
                      const Variant& data) {
  mpz_t gmpData, gmpReturn;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_sqrt: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_sqrt(gmpReturn, gmpData);
  mpz_clear(gmpData);

  return NEWOBJ(GMPResource)(gmpReturn);
}


Variant HHVM_FUNCTION(gmp_sqrtrem,
                      const Variant& data) {
  mpz_t gmpData, gmpSquareRoot, gmpRemainder;

  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_sqrtrem: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_inits(gmpSquareRoot, gmpRemainder, NULL);
  mpz_sqrtrem(gmpSquareRoot, gmpRemainder, gmpData);
  mpz_clear(gmpData);

  Array returnArray = Array::Create();
  returnArray.add(0, NEWOBJ(GMPResource)(gmpSquareRoot));
  returnArray.add(1, NEWOBJ(GMPResource)(gmpRemainder));

  return returnArray;
}


String HHVM_FUNCTION(gmp_strval,
                     const Variant& data,
                     const int64_t base = 10) {
  mpz_t gmpData;
  if (!variantToGMPData(gmpData, data, base)) {
    raise_warning("gmp_strval: Unable to convert variable to GMP - wrong type");
    return "0";
  }

  int charLength = mpz_sizeinbase(gmpData, abs(base)) + 1;
  if (mpz_sgn(gmpData) < 0) {
    ++charLength;
  }

  char *charArray = (char*) malloc(charLength);
  if (charArray == NULL || !mpz_get_str(charArray, base, gmpData)) {
    free(charArray);
    mpz_clear(gmpData);

    raise_warning("gmp_strval: Unable to convert convert to string");
    return "0";
  }

  mpz_clear(gmpData);
  String returnValue(charArray);
  free(charArray);

  return returnValue;
}


Variant HHVM_FUNCTION(gmp_sub,
                     const Variant& dataA,
                     const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_sub: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_sub: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_sub(gmpReturn, gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


bool HHVM_FUNCTION(gmp_testbit,
                   Variant& data,
                   int64_t index) {
  if (data.isResource()) {
    auto gmpRes = data.toResource().getTyped<GMPResource>();
    return mpz_tstbit(gmpRes->getData(), index);
  }

  mpz_t gmpData;
  if (!variantToGMPData(gmpData, data)) {
    raise_warning("gmp_setbit: Unable to convert variable to GMP - wrong type");
    return false;
  }

  bool isBitSet = mpz_tstbit(gmpData, index);
  mpz_clear(gmpData);

  return isBitSet;
}


Variant HHVM_FUNCTION(gmp_xor,
                      const Variant& dataA,
                      const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(gmpDataA, dataA)) {
    raise_warning("gmp_xor: Unable to convert variable to GMP - wrong type");
    return false;
  }
  if (!variantToGMPData(gmpDataB, dataB)) {
    mpz_clear(gmpDataA);

    raise_warning("gmp_xor: Unable to convert variable to GMP - wrong type");
    return false;
  }

  mpz_init(gmpReturn);
  mpz_xor(gmpReturn, gmpDataA, gmpDataB);
  mpz_clears(gmpDataA, gmpDataB, NULL);

  return NEWOBJ(GMPResource)(gmpReturn);
}


///////////////////////////////////////////////////////////////////////////////
// extension

const StaticString s_GMP_ROUND_ZERO("GMP_ROUND_ZERO");
const StaticString s_GMP_ROUND_PLUSINF("GMP_ROUND_PLUSINF");
const StaticString s_GMP_ROUND_MINUSINF("GMP_ROUND_MINUSINF");
const StaticString s_GMP_VERSION("GMP_VERSION");
const StaticString k_GMP_VERSION(gmp_version);

class GMPExtension : public Extension {
public:
  GMPExtension() : Extension("gmp", "1.0.0-hhvm") { };
  virtual void moduleInit() {
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

//#endif