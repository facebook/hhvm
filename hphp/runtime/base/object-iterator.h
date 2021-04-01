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

#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/preclass.h"

#include <folly/ScopeGuard.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Template based iteration of object properties

/*
 * Iterate the properties of a (non-collection) ObjectData.
 *
 * Iterates all the declared properties, then any dynamic properties.
 * Properties are visited in the order they are laid out in memory, NOT the
 * order they appear in when the object is cast to an array or the order in
 * which they appear in a foreach over the object. If you want the array cast
 * order, see IteratePropToArrayOrder below.
 *
 * This cannot be used for iterating collections. If you iterate other objects
 * that act like arrays (e.g. ArrayIterator, Iterable, etc.) this will visit
 * their properties, NOT the values you would see if you foreached them. If
 * you need the foreach behavior, use the array-iterator.h APIs.
 *
 * This operation has no refcounting at all. If you may mutate the object or
 * the dynamic-prop array during iteration, you must guard the operation with
 * an inc-ref before and dec-ref after.
 */
template <typename DeclFn, typename DynFn>
void IteratePropMemOrder(const ObjectData* obj, DeclFn declFn, DynFn dynFn) {
  assertx(!obj->isCollection());

  auto cls = obj->getVMClass();
  auto const declProps = cls->declProperties();
  auto const nProps = cls->numDeclProperties();
  for (Slot i = 0; i < nProps; ++i) {
    if (ArrayData::call_helper(declFn, i, declProps[i],
                               obj->propRvalAtOffset(i))) {
      return;
    }
  }

  if (UNLIKELY(obj->getAttribute(ObjectData::HasDynPropArr))) {
    auto const mad = MixedArray::asMixed(obj->dynPropArray().get());
    MixedArray::IterateKV(mad, dynFn);
  }
}

/*
 * This is exactly like IteratePropMemOrder except that properties are visited
 * in the order they appear when an object is cast to an array. All the
 * warnings from IteratePropMemOrder are also applicable to this version.
 */
template <typename DeclFn, typename DynFn>
void IteratePropToArrayOrder(const ObjectData* obj, DeclFn declFn, DynFn dynFn) {
  assertx(!obj->isCollection());

  // The iteration order is going most-to-least-derived in the inheritance
  // hierarchy, visiting properties in declaration order (with the wrinkle
  // that overridden properties should appear only once, at the site of the
  // most-derived declaration).
  auto cls = obj->getVMClass();
  auto const declProps = cls->declProperties();
  for (auto const& prop : declProps) {
    auto slot = prop.serializationIdx;
    if (ArrayData::call_helper(
      declFn, slot, declProps[slot], obj->propRvalAtOffset(slot))) {
      return;
    }
  }

  if (UNLIKELY(obj->getAttribute(ObjectData::HasDynPropArr))) {
    auto const mad = MixedArray::asMixed(obj->dynPropArray().get());
    MixedArray::IterateKV(mad, dynFn);
  }
}

///////////////////////////////////////////////////////////////////////////////
}

