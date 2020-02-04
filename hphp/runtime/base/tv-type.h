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
CASE(DArray)
CASE(VArray)
CASE(ArrayLike)
CASE(HackArray)
CASE(Vec)
CASE(Dict)
CASE(Keyset)
CASE(Object)
CASE(Resource)
CASE(Func)
CASE(Class)
CASE(ClsMeth)
CASE(Record)

#undef CASE

template<typename T>
ALWAYS_INLINE bool tvIsVecOrVArray(T&& tv) {
  return RuntimeOption::EvalHackArrDVArrs ? tvIsVec(tv) : tvIsArray(tv);
}

template<typename T>
ALWAYS_INLINE bool tvIsDictOrDArray(T&& tv) {
  return RuntimeOption::EvalHackArrDVArrs ? tvIsDict(tv) : tvIsArray(tv);
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
