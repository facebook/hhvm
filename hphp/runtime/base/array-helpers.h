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

// TODO(#3888164): These helpers only exist because we might try to set an
// Uninit value in an array. We should restructure code-gen to guard against
// this possibility so we can eliminate these checks.

///////////////////////////////////////////////////////////////////////////////

/*
 * Initialize a new array element.
 */
ALWAYS_INLINE void initElem(TypedValue& elem, TypedValue v, bool move = false) {
  if (UNLIKELY(v.m_type == KindOfUninit)) {
    elem.m_type = KindOfNull;
  } else if (move) {
    tvCopy(v, elem);
  } else {
    tvDup(v, elem);
  }
}

/*
 * Modify an array element, with semantics like those in tv-mutate.h
 * (i.e. promote uninit null values to init null values).
 */
template<typename C> ALWAYS_INLINE
enable_if_lval_t<C&&, void> setElem(C&& elem, TypedValue v, bool move = false) {
  if (UNLIKELY(v.m_type == KindOfUninit)) {
    v.m_type = KindOfNull;
  }
  if (move) {
    tvMove(v, elem);
  } else {
    tvSet(v, elem);
  }
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
