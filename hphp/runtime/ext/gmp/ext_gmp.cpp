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

#include "ext_gmp.h"

#include <cstdlib>
#include <float.h>

namespace HPHP {

HPHP::Class* GMP::cls = nullptr;

///////////////////////////////////////////////////////////////////////////////
// helpers
static String mpzToString(mpz_t gmpData, const int64_t base) {
  int charLength = mpz_sizeinbase(gmpData, std::abs(base)) + 1;
  if (mpz_sgn(gmpData) < 0) {
    ++charLength;
  }

  char *charStr = (char*)req::malloc(charLength);
  mpz_get_str(charStr, base, gmpData);

  String returnValue(charStr);
  req::free(charStr);

  return returnValue;
}


static Object mpzToGMPObject(mpz_t gmpData) {
  Object ret(GMP::allocObject());

  auto data = Native::data<GMPData>(ret);
  data->setGMPMpz(gmpData);

  return ret;
}


static bool setMpzFromString(mpz_t gmpData,
                             const String& stringInput,
                             int64_t base) {
  String strNum(stringInput);
  int64_t strLength = strNum.length();

  if (strLength == 0) {
    return false;
  }

  int8_t negativePad;
  std::string negativeSign;
  if (strNum[0] == '-') {
    negativePad = 1;
    negativeSign = "-";
  } else if (strNum[0] == '+') {
      // We don't allow leading pluses to match PHP
      return false;
  } else {
    negativePad = 0;
    negativeSign = "";
  }

  // We can't rely on GMP's auto-base detection as it doesn't handle cases
  // where a base is specified AND a prefix is specified. So, we strip out
  // all prefixes and detect the bases ourselves.
  if (strLength > (negativePad + 2) && strNum[negativePad] == '0') {
    if ((base == 0 || base == 16)
        && (strNum[negativePad + 1] == 'x'
            || strNum[negativePad + 1] == 'X')) {
      base = 16;
      strNum = negativeSign + strNum.substr(negativePad + 2);
    } else if ((base == 0 || base == 2)
               && (strNum[negativePad + 1] == 'b'
                   || strNum[negativePad + 1] == 'B')) {
      base = 2;
      strNum = negativeSign + strNum.substr(negativePad + 2);
    } else if (base == 0 || base == 8) {
      base = 8;
      strNum = negativeSign + strNum.substr(negativePad + 1);
    }
  }

  if (mpz_init_set_str(gmpData, strNum.c_str(), base) == -1) {
    mpz_clear(gmpData);
    if (base == 0 && strNum.get()->isNumeric()) {
      mpz_init_set_si(gmpData, strNum.toInt64());
    } else {
      return false;
    }

  }
  return true;
}


static bool variantToGMPData(const char* const fnCaller,
                             mpz_t gmpData,
                             const Variant& data,
                             const int64_t base = 0) {
  switch (data.getType()) {

  case KindOfObject: {
    Object gmpObject = data.toObject();
    if (!gmpObject.instanceof(s_GMP_GMP)) {
      raise_warning(cs_GMP_INVALID_OBJECT, fnCaller);
      return false;
    }

    auto gmpObjectData = Native::data<GMPData>(gmpObject);
    if (!gmpObjectData) {
      raise_warning(cs_GMP_INVALID_OBJECT, fnCaller);
      return false;
    }

    mpz_init_set(gmpData, gmpObjectData->getGMPMpz());
    return true;
  }

  case KindOfString:
  case KindOfStaticString:
    if (!setMpzFromString(gmpData, data.toString(), base)) {
      raise_warning(cs_GMP_INVALID_STRING, fnCaller);
      return false;
    }
    return true;

  case KindOfInt64:
  case KindOfBoolean:
    mpz_init_set_si(gmpData, data.toInt64());
    return true;

  case KindOfDouble:
    if (data.toDouble() > DBL_MAX || data.toDouble() < DBL_MIN) {
      raise_warning(cs_GMP_INVALID_TYPE, fnCaller);
      return false;
    }
    mpz_init_set_d(gmpData, data.toDouble());
    return true;

  case KindOfUninit:
  case KindOfNull:
  case KindOfArray:
  case KindOfRef:
  case KindOfClass:
  case KindOfResource:
    raise_warning(cs_GMP_INVALID_TYPE, fnCaller);
    return false;
  }

  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
// extension functions
static Variant HHVM_FUNCTION(gmp_abs,
                             const Variant& data) {
  mpz_t gmpReturn, gmpData;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_ABS, gmpData, data)) {
    return false;
  }

  mpz_init(gmpReturn);
  mpz_abs(gmpReturn, gmpData);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpReturn);
  mpz_clear(gmpData);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_add,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_ADD, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_ADD, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_add(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_and,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_AND, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_AND, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_and(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = mpzToGMPObject(gmpReturn);

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

  Object gmpObject = data.toObject();
  if (!gmpObject.instanceof(s_GMP_GMP)) {
    raise_warning(cs_GMP_INVALID_OBJECT, cs_GMP_FUNC_NAME_GMP_CLRBIT);
    return;
  }

  auto gmpData = Native::data<GMPData>(gmpObject);
  if (!gmpData) {
    raise_warning(cs_GMP_INVALID_OBJECT, cs_GMP_FUNC_NAME_GMP_CLRBIT);
    return;
  }

  mpz_clrbit(gmpData->getGMPMpz(), index);
}


static Variant HHVM_FUNCTION(gmp_cmp,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_CMP, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_CMP, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
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

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_COM, gmpData, data)) {
    return false;
  }

  mpz_init(gmpReturn);
  mpz_com(gmpReturn, gmpData);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpReturn);
  mpz_clear(gmpData);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_div_q,
                             const Variant& dataA,
                             const Variant& dataB,
                             int64_t round = GMP_ROUND_ZERO) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_DIV_Q, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_DIV_Q, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
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

  Variant ret = mpzToGMPObject(gmpReturn);

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

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_DIV_QR, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_DIV_QR, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
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

  PackedArrayInit returnArray(2);
  returnArray.appendWithRef(mpzToGMPObject(gmpReturnQ));
  returnArray.appendWithRef(mpzToGMPObject(gmpReturnR));

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

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_DIV_R, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_DIV_R, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
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

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_divexact,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_DIVEXACT, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_DIVEXACT, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  if (mpz_sgn(gmpDataB) == 0) {
    raise_warning(cs_GMP_INVALID_VALUE_MUST_NOT_BE_ZERO,
                  cs_GMP_FUNC_NAME_GMP_DIVEXACT);
    mpz_clear(gmpDataA);
    mpz_clear(gmpDataB);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_divexact(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_fact,
                             const Variant& data) {
  mpz_t gmpReturn;

  if (data.isObject()) {
    mpz_t gmpData;
    if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_FACT, gmpData, data)) {
      return false;
    }

    if (mpz_sgn(gmpData) < 0) {
      mpz_clear(gmpData);

      raise_warning(cs_GMP_INVALID_VALUE_MUST_BE_POSITIVE,
                    cs_GMP_FUNC_NAME_GMP_FACT);
      return false;
    }

    mpz_init(gmpReturn);
    mpz_fac_ui(gmpReturn, mpz_get_ui(gmpData));

    mpz_clear(gmpData);
  } else {
    if (data.toInt64() < 0) {
      raise_warning(cs_GMP_INVALID_VALUE_MUST_BE_POSITIVE,
                    cs_GMP_FUNC_NAME_GMP_FACT);
      return false;
    }

    mpz_init(gmpReturn);
    mpz_fac_ui(gmpReturn, data.toInt64());
  }
  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_gcd,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_GCD, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_GCD, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_gcd(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_gcdext,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturnG, gmpReturnS, gmpReturnT;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_GCDEXCT, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_GCDEXCT, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  mpz_init(gmpReturnG);
  mpz_init(gmpReturnS);
  mpz_init(gmpReturnT);

  mpz_gcdext(gmpReturnG, gmpReturnS, gmpReturnT, gmpDataA, gmpDataB);

  ArrayInit returnArray(3, ArrayInit::Map{});
  returnArray.set(s_GMP_g, mpzToGMPObject(gmpReturnG));
  returnArray.set(s_GMP_s, mpzToGMPObject(gmpReturnS));
  returnArray.set(s_GMP_t, mpzToGMPObject(gmpReturnT));

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

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_HAMDIST, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_HAMDIST, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  int64_t hamdist = mpz_hamdist(gmpDataA, gmpDataB);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);

  return hamdist;
}


static Variant HHVM_FUNCTION(gmp_init,
                             const Variant& data,
                             const int64_t base /* = 0 */) {
  mpz_t gmpData;

  if (base < GMP_MIN_BASE || base == -1 || base == 1 || base > GMP_MAX_BASE) {
    raise_warning(cs_GMP_INVALID_BASE_VALUE,
                  cs_GMP_FUNC_NAME_GMP_INIT,
                  base,
                  GMP_MAX_BASE);
    return false;
  }

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_INIT, gmpData, data, base)) {
    return false;
  }

  Variant ret = mpzToGMPObject(gmpData);

  mpz_clear(gmpData);

  return ret;
}


static int64_t HHVM_FUNCTION(gmp_intval,
                             const Variant& data) {
  mpz_t gmpData;

  if (data.isArray()
      || data.isResource()
      || (data.isString() && data.toString().empty())
      || (data.isObject() && !data.toObject().instanceof(s_GMP_GMP))
      || !variantToGMPData(cs_GMP_FUNC_NAME_GMP_INTVAL, gmpData, data)) {
    return data.toInt64();
  }

  int64_t result = mpz_get_si(gmpData);

  mpz_clear(gmpData);

  return result;
}


static Variant HHVM_FUNCTION(gmp_invert,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_INVERT, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_INVERT, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  mpz_init(gmpReturn);
  if (!mpz_invert(gmpReturn, gmpDataA, gmpDataB)) {
    mpz_clear(gmpDataA);
    mpz_clear(gmpDataB);
    mpz_clear(gmpReturn);
    return false;
  }

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_jacobi,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_JACOBI, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_JACOBI, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
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

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_LEGENDRE, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_LEGENDRE, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
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

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_MOD, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_MOD, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  if (mpz_sgn(gmpDataB) == 0) {
    raise_warning(cs_GMP_INVALID_VALUE_MUST_NOT_BE_ZERO,
                  cs_GMP_FUNC_NAME_GMP_MOD);
    mpz_clear(gmpDataA);
    mpz_clear(gmpDataB);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_mod(gmpReturn, gmpDataA, gmpDataB);

  Variant ret;

  ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_mul,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_MUL, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_MUL, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_mul(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_neg,
                             const Variant& data) {
  mpz_t gmpReturn, gmpData;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_NEG, gmpData, data)) {
    return false;
  }

  mpz_init(gmpReturn);
  mpz_neg(gmpReturn, gmpData);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpData);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_nextprime,
                             const Variant& data) {
  mpz_t gmpReturn, gmpData;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_NEXTPRIME, gmpData, data)) {
    return false;
  }

  mpz_init(gmpReturn);
  mpz_nextprime(gmpReturn, gmpData);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpData);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_or,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_OR, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_OR, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_ior(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static bool HHVM_FUNCTION(gmp_perfect_square,
                          const Variant& data) {
  mpz_t gmpData;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_PERFECT_SQUARE, gmpData, data)) {
    return false;
  }

  bool isPerfectSquare = (mpz_perfect_square_p(gmpData) != 0);

  mpz_clear(gmpData);

  return isPerfectSquare;
}


static Variant HHVM_FUNCTION(gmp_popcount,
                             const Variant& data) {
  mpz_t gmpData;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_POPCOUNT, gmpData, data)) {
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

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_POW, gmpData, data)) {
    return false;
  }

  mpz_init(gmpReturn);
  mpz_pow_ui(gmpReturn, gmpData, exp);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpData);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_powm,
                             const Variant& dataA,
                             const Variant& dataB,
                             const Variant& dataC) {
  mpz_t gmpDataA, gmpDataB, gmpDataC, gmpReturn;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_POWM, gmpDataB, dataB)) {
    return false;
  }
  if (mpz_sgn(gmpDataB) < 0) {
    mpz_clear(gmpDataB);

    raise_warning(cs_GMP_INVALID_EXPONENT_MUST_BE_POSITIVE,
                  cs_GMP_FUNC_NAME_GMP_POWM);
    return false;
  }

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_POWM, gmpDataC, dataC)) {
    mpz_clear(gmpDataB);
    return false;
  }
  if (mpz_sgn(gmpDataC) == 0) {
    mpz_clear(gmpDataB);
    mpz_clear(gmpDataC);

    raise_warning(cs_GMP_INVALID_MODULUS_MUST_NOT_BE_ZERO,
                  cs_GMP_FUNC_NAME_GMP_POWM);
    return false;
  }

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_POWM, gmpDataA, dataA)) {
    mpz_clear(gmpDataC);
    mpz_clear(gmpDataB);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_powm(gmpReturn, gmpDataA, gmpDataB, gmpDataC);

  Variant ret = mpzToGMPObject(gmpReturn);

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

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_PROB_PRIME, gmpData, data)) {
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


static Variant HHVM_FUNCTION(gmp_root,
                             const Variant& data,
                             int64_t root) {
  if (root < 1) {
    raise_warning(cs_GMP_INVALID_ROOT_MUST_BE_POSITIVE,
                  cs_GMP_FUNC_NAME_GMP_ROOT);
    return false;
  }

  mpz_t gmpData, gmpReturn;
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_ROOT, gmpData, data)) {
    return false;
  }

  if (root % 2 == 0 && mpz_sgn(gmpData) < 0) {
    mpz_clear(gmpData);
    raise_warning(cs_GMP_ERROR_EVEN_ROOT_NEGATIVE_NUMBER,
                  cs_GMP_FUNC_NAME_GMP_ROOT);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_root(gmpReturn, gmpData, (uint32_t)root);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpReturn);
  mpz_clear(gmpData);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_rootrem,
                             const Variant& data,
                             int64_t root) {
  if (root < 1) {
    raise_warning(cs_GMP_INVALID_ROOT_MUST_BE_POSITIVE,
                  cs_GMP_FUNC_NAME_GMP_ROOTREM);
    return false;
  }

  mpz_t gmpData, gmpReturn0, gmpReturn1;
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_ROOTREM, gmpData, data)) {
    return false;
  }

  if (root % 2 == 0 && mpz_sgn(gmpData) < 0) {
    mpz_clear(gmpData);
    raise_warning(cs_GMP_ERROR_EVEN_ROOT_NEGATIVE_NUMBER,
                  cs_GMP_FUNC_NAME_GMP_ROOTREM);
    return false;
  }

  mpz_init(gmpReturn0);
  mpz_init(gmpReturn1);

  mpz_rootrem(gmpReturn0, gmpReturn1, gmpData, (uint32_t)root);

  PackedArrayInit returnArray(2);
  returnArray.appendWithRef(mpzToGMPObject(gmpReturn0));
  returnArray.appendWithRef(mpzToGMPObject(gmpReturn1));

  mpz_clear(gmpData);
  mpz_clear(gmpReturn0);
  mpz_clear(gmpReturn1);

  return returnArray.toVariant();
}


static Variant HHVM_FUNCTION(gmp_scan0,
                             const Variant& data,
                             int64_t start) {
  if (start < 0) {
    raise_warning(cs_GMP_INVALID_STARTING_INDEX_IS_NEGATIVE,
                  cs_GMP_FUNC_NAME_GMP_SCAN0);
    return false;
  }

  mpz_t gmpData;
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_SCAN0, gmpData, data)) {
    return false;
  }

  int64_t foundBit = mpz_scan0(gmpData, start);

  mpz_clear(gmpData);

  return foundBit;
}


static Variant HHVM_FUNCTION(gmp_scan1,
                             const Variant& data,
                             int64_t start) {
  if (start < 0) {
    raise_warning(cs_GMP_INVALID_STARTING_INDEX_IS_NEGATIVE,
                  cs_GMP_FUNC_NAME_GMP_SCAN1);
    return false;
  }

  mpz_t gmpData;
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_SCAN1, gmpData, data)) {
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

  Object gmpObject = data.toObject();
  if (!gmpObject.instanceof(s_GMP_GMP)) {
    raise_warning(cs_GMP_INVALID_OBJECT, cs_GMP_FUNC_NAME_GMP_SETBIT);
    return;
  }

  auto gmpData = Native::data<GMPData>(gmpObject);
  if (!gmpData) {
    raise_warning(cs_GMP_INVALID_OBJECT, cs_GMP_FUNC_NAME_GMP_SETBIT);
    return;
  }

  if (bitOn) {
    mpz_setbit(gmpData->getGMPMpz(), index);
  } else {
    mpz_clrbit(gmpData->getGMPMpz(), index);
  }
}


static Variant HHVM_FUNCTION(gmp_sign,
                             const Variant& data) {
  mpz_t gmpData;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_SIGN, gmpData, data)) {
    return false;
  }

  int64_t sign = mpz_sgn(gmpData);

  mpz_clear(gmpData);

  return sign;
}


static Variant HHVM_FUNCTION(gmp_sqrt,
                             const Variant& data) {
  mpz_t gmpReturn, gmpData;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_SQRT, gmpData, data)) {
    return false;
  }

  if (mpz_sgn(gmpData) < 0) {
    raise_warning(cs_GMP_INVALID_NUMBER_IS_NEGATIVE,
                  cs_GMP_FUNC_NAME_GMP_SQRT);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_sqrt(gmpReturn, gmpData);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpData);
  mpz_clear(gmpReturn);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_sqrtrem,
                             const Variant& data) {
  mpz_t gmpData, gmpSquareRoot, gmpRemainder;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_SQRTREM, gmpData, data)) {
    return false;
  }

  if (mpz_sgn(gmpData) < 0) {
    raise_warning(cs_GMP_INVALID_NUMBER_IS_NEGATIVE,
                  cs_GMP_FUNC_NAME_GMP_SQRTREM);
    return false;
  }

  mpz_init(gmpSquareRoot);
  mpz_init(gmpRemainder);

  mpz_sqrtrem(gmpSquareRoot, gmpRemainder, gmpData);

  PackedArrayInit returnArray(2);
  returnArray.appendWithRef(mpzToGMPObject(gmpSquareRoot));
  returnArray.appendWithRef(mpzToGMPObject(gmpRemainder));

  mpz_clear(gmpData);
  mpz_clear(gmpSquareRoot);
  mpz_clear(gmpRemainder);

  return returnArray.toVariant();
}


static Variant HHVM_FUNCTION(gmp_strval,
                             const Variant& data,
                             const int64_t base /* = 10 */) {
  mpz_t gmpData;

  if (base < GMP_MIN_BASE || (base > -2 && base < 2) || base > GMP_MAX_BASE) {
    raise_warning(cs_GMP_INVALID_BASE_VALUE,
                  cs_GMP_FUNC_NAME_GMP_STRVAL,
                  base,
                  GMP_MAX_BASE);
    return false;
  }

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_STRVAL, gmpData, data)) {
    return false;
  }

  String returnValue(mpzToString(gmpData, base));

  mpz_clear(gmpData);

  return returnValue;
}


static Variant HHVM_FUNCTION(gmp_sub,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_SUB, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_SUB, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_sub(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}


static bool HHVM_FUNCTION(gmp_testbit,
                          const Variant& data,
                          int64_t index) {
  mpz_t gmpData;

  if (index < 0) {
    raise_warning(cs_GMP_INVALID_INDEX_IS_NEGATIVE,
                  cs_GMP_FUNC_NAME_GMP_TESTBIT);
    return false;
  }

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_TESTBIT, gmpData, data)) {
    return false;
  }

  bool ret = mpz_tstbit(gmpData, index);

  mpz_clear(gmpData);

  return ret;
}


static Variant HHVM_FUNCTION(gmp_xor,
                             const Variant& dataA,
                             const Variant& dataB) {
  mpz_t gmpDataA, gmpDataB, gmpReturn;

  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_XOR, gmpDataA, dataA)) {
    return false;
  }
  if (!variantToGMPData(cs_GMP_FUNC_NAME_GMP_XOR, gmpDataB, dataB)) {
    mpz_clear(gmpDataA);
    return false;
  }

  mpz_init(gmpReturn);
  mpz_xor(gmpReturn, gmpDataA, gmpDataB);

  Variant ret = mpzToGMPObject(gmpReturn);

  mpz_clear(gmpDataA);
  mpz_clear(gmpDataB);
  mpz_clear(gmpReturn);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// extension class methods
static String HHVM_METHOD(GMP, serialize) {
  auto data = Native::data<GMPData>(this_);

  // We first serialize our number as a string, then we need to serialize
  // any other public properties as an array after that.

  StringBuffer retBuf(SmallStringReserve);
  VariableSerializer serializer(VariableSerializer::Type::Serialize);

  Variant numValue(mpzToString(data->getGMPMpz(), 10));
  Variant objProps(this_->toArray(true));

  retBuf.append(serializer.serializeValue(numValue, true));
  retBuf.append(serializer.serializeValue(objProps, true));

  return retBuf.detach();
}


static void HHVM_METHOD(GMP, unserialize,
                        const Variant& data) {
  String serialData = data.toString();
  VariableUnserializer unserializer(serialData.data(),
                                    serialData.length(),
                                    VariableUnserializer::Type::Serialize);

  mpz_t gmpData;

  // First value is a string (our number value)
  Variant num = unserializer.unserialize();
  if (!num.isString() || !setMpzFromString(gmpData, num.toString(), 10)) {
    throw Exception(cs_GMP_COULD_NOT_UNSERIALIZE_NUMBER);
  }

  // Second value is an array of object properties optionally set by the user
  Variant props = unserializer.unserialize();
  if (!props.isArray()) {
    mpz_clear(gmpData);
    throw Exception(cs_GMP_COULD_NOT_UNSERIALIZE_PROPERTIES);
  }

  auto gmpObjectData = Native::data<GMPData>(this_);
  gmpObjectData->setGMPMpz(gmpData);
  this_->o_setArray(props.toArray());

  mpz_clear(gmpData);
}


static Array HHVM_METHOD(GMP, __debugInfo) {
  auto gmpObjectData = Native::data<GMPData>(this_);
  Array ret = this_->toArray(true);

  String num = mpzToString(gmpObjectData->getGMPMpz(), 10);
  ret.set(s_GMP_num, num);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// GMPData class methods
GMPData& GMPData::operator=(const GMPData& source) {
  if (this != &source) {
    setGMPMpz(source.m_gmpMpz);
  }
  return *this;
}


void GMPData::close() {
  if (m_isInit) {
    mpz_clear(m_gmpMpz);
    m_isInit = false;
  }
}


void GMPData::setGMPMpz(const mpz_t data) {
   if (m_isInit) {
     mpz_set(m_gmpMpz, data);
   } else {
     m_isInit = true;
     mpz_init_set(m_gmpMpz, data);
   }
}

///////////////////////////////////////////////////////////////////////////////
// extension
class GMPExtension final : public Extension {
public:
  GMPExtension() : Extension("gmp", "2.0.0-hhvm") { };
  void moduleInit() override {
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
    HHVM_FE(gmp_root);
    HHVM_FE(gmp_rootrem);
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

    Native::registerNativeDataInfo<GMPData>(s_GMPData.get());
    HHVM_ME(GMP, serialize);
    HHVM_ME(GMP, unserialize);
    HHVM_ME(GMP, __debugInfo);

    loadSystemlib();
  }
} s_gmp_extension;

///////////////////////////////////////////////////////////////////////////////
}
