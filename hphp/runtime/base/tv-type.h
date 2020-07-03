/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_TV_TYPE_H_
#define incl_HPHP_TV_TYPE_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE bool tvIsNull(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  return isNullType(tv.m_type);
}
ALWAYS_INLINE bool tvIsNull(const TypedValue* tv) {
  return tvIsNull(*tv);
}

#define CASE(ty)                                                        \
  template<typename T>                                                  \
  ALWAYS_INLINE enable_if_tv_val_t<T&&, bool> tvIs##ty(T&& tv) {        \
    return is##ty##Type(type(tv));                                      \
  }

CASE(Null)
CASE(Bool)
CASE(Int)
CASE(Double)
CASE(String)
CASE(Array)
CASE(ArrayLike)
CASE(HackArray)
CASE(Vec)
CASE(Dict)
CASE(Keyset)
CASE(Object)
CASE(Resource)
CASE(Func)
CASE(RFunc)
CASE(Class)
CASE(ClsMeth)
CASE(Record)

#undef CASE

// We don't expose isVArrayType or isDArrayType. They shouldn't be used.
ALWAYS_INLINE bool isHAMSafeDVArrayType(DataType dt) {
  if (RO::EvalHackArrDVArrs) return isVecType(dt) || isDictType(dt);
  auto const dtrc = dt_with_rc(dt);
  return dtrc == KindOfVArray || dtrc == KindOfDArray;
}

template<typename T>
ALWAYS_INLINE bool tvIsHAMSafeVArray(const T& tv) {
  if (RO::EvalHackArrDVArrs) return tvIsVec(tv);
  return tvIsArrayLike(tv) && val(tv).parr->isVArray();
}

template<typename T>
ALWAYS_INLINE bool tvIsHAMSafeDArray(const T& tv) {
  if (RO::EvalHackArrDVArrs) return tvIsDict(tv);
  return tvIsArrayLike(tv) && val(tv).parr->isDArray();
}

template<typename T>
ALWAYS_INLINE bool tvIsHAMSafeDVArray(const T& tv) {
  return isHAMSafeDVArrayType(type(tv));
}

template<typename T>
ALWAYS_INLINE int64_t tvAssertInt(T&& tv) {
  assertx(isIntType(type(tv)));
  return val(tv).num;
}

template<typename T>
ALWAYS_INLINE double tvAssertDouble(T&& tv) {
  assertx(isDoubleType(type(tv)));
  return val(tv).dbl;
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
