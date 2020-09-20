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

#pragma once

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

// We don't expose isVArrayType or isDArrayType. They shouldn't be used.
ALWAYS_INLINE bool isHAMSafeDArrayType(DataType t) {
  return RO::EvalHackArrDVArrs ? isDictType(t) : dt_with_rc(t) == KindOfDArray;
}
ALWAYS_INLINE bool isHAMSafeVArrayType(DataType t) {
  return RO::EvalHackArrDVArrs ? isVecType(t) : dt_with_rc(t) == KindOfVArray;
}
ALWAYS_INLINE bool isHAMSafeDVArrayType(DataType t) {
  if (RO::EvalHackArrDVArrs) return isVecType(t) || isDictType(t);
  auto const dtrc = dt_with_rc(t);
  return dtrc == KindOfVArray || dtrc == KindOfDArray;
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
CASE(VecOrVArray)
CASE(DictOrDArray)
CASE(HAMSafeVArray)
CASE(HAMSafeDArray)
CASE(HAMSafeDVArray)
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
CASE(RClsMeth)
CASE(Record)

#undef CASE

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

