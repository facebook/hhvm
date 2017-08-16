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

#ifndef incl_HPHP_ARRAY_HELPERS_H_
#define incl_HPHP_ARRAY_HELPERS_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * Initialize a new array element.
 */
ALWAYS_INLINE void initElem(TypedValue& elem, Cell v) {
  cellDup(v, elem);
  if (UNLIKELY(elem.m_type == KindOfUninit)) {
    elem.m_type = KindOfNull;
  }
}

/*
 * Modify an array element, with semantics like those in tv-mutate.h.
 *
 * These functions all promote uninit null values to init null values, except
 * for setElemWithRef() which asserts that `v' is init instead.
 */
ALWAYS_INLINE void setElemNoRef(Cell& elem, Cell v) {
  if (UNLIKELY(v.m_type == KindOfUninit)) {
    v.m_type = KindOfNull;
  }
  cellSet(v, elem);
}
ALWAYS_INLINE void setElem(TypedValue& elem, Cell v) {
  setElemNoRef(*tvToCell(&elem), v);
}

ALWAYS_INLINE void setElemWithRef(TypedValue& elem, TypedValue v) {
  assertx(UNLIKELY(v.m_type != KindOfUninit));
  tvSetWithRef(v, elem);
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
