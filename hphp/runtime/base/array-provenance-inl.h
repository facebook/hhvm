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

#ifndef incl_HPHP_ARRAY_PROVENANCE_INL_H_
#error "array-provenance-inl.h should only be included by array-provenance.h"
#endif

namespace HPHP { namespace arrprov {

///////////////////////////////////////////////////////////////////////////////

inline bool tvWantsTag(TypedValue tv) {
  return isVecType(type(tv)) || isDictType(type(tv));
}

inline void copyTag(const ArrayData* src, ArrayData* dest) {
  if (auto const tag = getTag(src)) {
    setTag(dest, *tag);
  } else if (auto const pctag = tagFromProgramCounter()) {
    setTag(dest, *pctag);
  }
}

inline void copyTagStatic(const ArrayData* src, ArrayData* dest) {
  if (!RuntimeOption::EvalArrayProvenance) return;
  if (auto const tag = getTag(src)) setTag(dest, *tag);
}

///////////////////////////////////////////////////////////////////////////////

}}
