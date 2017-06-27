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
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE bool cellIsNull(Cell tv) {
  assert(cellIsPlausible(tv));
  return isNullType(tv.m_type);
}
ALWAYS_INLINE bool cellIsNull(const Cell* tv) {
  return cellIsNull(*tv);
}

ALWAYS_INLINE bool tvIsString(const TypedValue* tv) {
  return isStringType(tv->m_type);
}

ALWAYS_INLINE bool tvIsArray(const TypedValue* tv) {
  return isArrayType(tv->m_type);
}

ALWAYS_INLINE bool tvIsHackArray(const TypedValue* tv) {
  return isHackArrayType(tv->m_type);
}

ALWAYS_INLINE bool tvIsVecArray(const TypedValue* tv) {
  return isVecType(tv->m_type);
}

ALWAYS_INLINE bool tvIsDict(const TypedValue* tv) {
  return isDictType(tv->m_type);
}

ALWAYS_INLINE bool tvIsKeyset(const TypedValue* tv) {
  return isKeysetType(tv->m_type);
}

ALWAYS_INLINE bool tvIsReferenced(TypedValue tv) {
  return tv.m_type == KindOfRef &&
         tv.m_data.pref->isReferenced();
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
