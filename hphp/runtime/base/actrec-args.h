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

#ifndef incl_HPHP_ACTREC_ARGS_H
#define incl_HPHP_ACTREC_ARGS_H

#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/version.h"
#include "hphp/util/exception.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

/**
 * Get numbererd arg (zero based) as a TypedValue*
 */
inline
TypedValue* getArg(ActRec *ar, unsigned arg) {
  if (arg >= ar->numArgs()) {
    return nullptr;
  }
  unsigned funcParams = ar->func()->numParams();
  if (arg < funcParams) {
    auto args = reinterpret_cast<TypedValue*>(ar);
    return args - (arg + 1);
  }
  return ar->getExtraArg(arg - funcParams);
}

/**
 * Get numbered arg (zero based) as a Variant
 */
inline Variant getArgVariant(ActRec *ar, unsigned arg,
                             Variant def = uninit_null()) {
  auto tv = getArg(ar, arg);
  return tv ? tvAsVariant(tv) : def;
}

/**
 * Get a reference value from the stack
 */
template <DataType DType>
typename std::enable_if<DType == KindOfRef, VRefParam>::type
getArg(ActRec *ar, unsigned arg) {
  auto tv = getArg(ar, arg);
  if (!tv) {
    raise_warning("Required parameter %d not passed", (int)arg);
    return directRef(Variant());
  }
  if (tv->m_type != KindOfRef) {
    raise_warning("Argument %d not passed as reference", (int)arg);
  }
  return directRef(tvAsVariant(tv));
}

/**
 * Get numbered arg (zero based) and return data (coerce if needed)
 *
 * e.g.: double dval = getArg<KindOfDouble>(ar, 0);
 *
 * Throws warning and returns 0/nullptr if arg not passed
 */
template <DataType DType>
typename std::enable_if<DType != KindOfRef,
  typename DataTypeCPPType<DType>::type>::type
getArg(ActRec *ar, unsigned arg) {
  auto tv = getArg(ar, arg);
  if (!tv) {
    raise_warning("Required parameter %d not passed", (int)arg);
    return 0L;
  }
  if (!tvCoerceParamInPlace(tv, DType)) {
    raise_param_type_warning(ar->func()->name()->data(),
                             arg + 1, DType, tv->m_type);
    tvCastInPlace(tv, DType);
  }
  return unpack_tv<DType>(tv);
}

/**
 * Get numbered arg (zero based) and return data (coerce if needed)
 *
 * e.g. int64_t lval = getArg<KindOfInt64>(ar, 1, 42);
 *
 * Returns default value (42 in example) if arg not passed
 */
template <DataType DType>
typename DataTypeCPPType<DType>::type
getArg(ActRec *ar, unsigned arg,
       typename DataTypeCPPType<DType>::type def) {
  TypedValue *tv = getArg(ar, arg);
  if (!tv) {
    return def;
  }
  if (!tvCoerceParamInPlace(tv, DType)) {
    raise_param_type_warning(ar->func()->name()->data(),
                             arg + 1, DType, tv->m_type);
    tvCastInPlace(tv, DType);
  }
  return unpack_tv<DType>(tv);
}

struct IncoercibleArgumentException : Exception {};

/**
 * Get numbered arg (zero based) and return data (coerce if needed)
 *
 * e.g.: double dval = getArg<KindOfDouble>(ar, 0);
 *
 * Raise warning and throw IncoercibleArgumentException if argument
 * is not provided/not coercible
 */
template <DataType DType>
typename std::enable_if<DType != KindOfRef,
  typename DataTypeCPPType<DType>::type>::type
getArgStrict(ActRec *ar, unsigned arg) {
  auto tv = getArg(ar, arg);
  if (!tv) {
    raise_warning("Required parameter %d not passed", (int)arg);
    throw IncoercibleArgumentException();
  }
  if (!tvCoerceParamInPlace(tv, DType)) {
    raise_param_type_warning(ar->func()->name()->data(),
                             arg + 1, DType, tv->m_type);
    throw IncoercibleArgumentException();
  }
  return unpack_tv<DType>(tv);
}

/**
 * Get numbered arg (zero based) and return data (coerce if needed)
 *
 * e.g. int64_t lval = getArg<KindOfInt64>(ar, 1, 42);
 *
 * Raise warning and throw IncoercibleArgumentException if argument
 * is not coercible
 *
 * Returns default value (42 in example) if arg not passed
 */
template <DataType DType>
typename DataTypeCPPType<DType>::type
getArgStrict(ActRec *ar, unsigned arg,
       typename DataTypeCPPType<DType>::type def) {
  TypedValue *tv = getArg(ar, arg);
  if (!tv) {
    return def;
  }
  if (!tvCoerceParamInPlace(tv, DType)) {
    raise_param_type_warning(ar->func()->name()->data(),
                             arg + 1, DType, tv->m_type);
    throw IncoercibleArgumentException();
  }
  return unpack_tv<DType>(tv);
}

/**
 * Parse typed values from the function call
 * based on an expect format.
 *
 * e.g.:
 *   int64_t lval;
 *   double dval;
 *   TypedValye *tv = nullptr;
 *   if (!parseArgs(ar, "ld|v", &lval, &dval, &tv)) {
 *     return false;
 *   }
 */
bool parseArgs(ActRec *ar, const char *format, ...);

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
#endif // incl_HPHP_ACTREC_ARGS_H
