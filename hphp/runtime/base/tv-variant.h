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

#ifndef incl_HPHP_TV_VARIANT_H_
#define incl_HPHP_TV_VARIANT_H_

#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/tv-val.h"

namespace HPHP {

struct Variant;

///////////////////////////////////////////////////////////////////////////////
/*
 * The functions in this file serve as escape hatches that allow us to call
 * into the Variant machinery.
 *
 * Ideally, we will use these as little as possible in the long term.  The
 * harsh reality is that we include this file pretty much everywhere.
 *
 * Except where noted, all of these functions assume that the TypedValue is
 * live (i.e., contains a valid, refcount-supported value).
 */

ALWAYS_INLINE Variant& tvAsVariant(TypedValue* tv) {
  assertx(tv != nullptr);
  assertx(tvIsPlausible(*tv));
  return reinterpret_cast<Variant&>(*tv);
}

ALWAYS_INLINE Variant& tvAsUninitializedVariant(TypedValue* tv) {
  // A special case, for use when constructing a variant and we don't assume
  // initialization.
  assertx(tv != nullptr);
  return reinterpret_cast<Variant&>(*tv);
}

ALWAYS_INLINE const Variant& tvAsCVarRef(const TypedValue* tv) {
  assertx(tv != nullptr);
  assertx(tvIsPlausible(*tv));
  return reinterpret_cast<const Variant&>(*tv);
}

ALWAYS_INLINE Variant& tvAsVariant(TypedValue& tv) {
  assertx(tvIsPlausible(tv));
  return reinterpret_cast<Variant&>(tv);
}

ALWAYS_INLINE const Variant& tvAsCVarRef(const TypedValue& tv) {
  assertx(tvIsPlausible(tv));
  return reinterpret_cast<const Variant&>(tv);
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
