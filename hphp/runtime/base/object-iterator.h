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

#ifndef incl_HPHP_OBJECT_ITERATOR_H_
#define incl_HPHP_OBJECT_ITERATOR_H_

#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/vm/class.h"

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
 * which they appear in a foreach over the object.
 *
 * This cannot be used for iterating collections. If you iterate other objects
 * that act like arrays (e.g. ArrayIterator, Iterable, etc.) this will visit
 * their properties, NOT the values you would see if you foreached them. If
 * you need the foreach behavior, use the array-iterator.h APIs.
 *
 * If incRef is false, then you promise neither you nor any other code will
 * mutate or manipulate refcounts of either obj or its dynamic prop arrray,
 * if any. If incRef is true then mutating the object is safe, but while you
 * will see the new values of declared properties you have yet to visit, you
 * may or may not see the new values of dynamic properties.
 */
template <typename DeclFn, typename DynFn, bool incRef = true>
void IteratePropMemOrder(const ObjectData* obj, DeclFn declFn, DynFn dynFn) {
  assertx(!obj->isCollection());

  if (incRef) obj->incRefCount();
  SCOPE_EXIT { if (incRef) decRefObj(const_cast<ObjectData*>(obj)); };

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
    // If we increffed the object, we still have to incref the dyn prop
    // array since a write to it can cause it to grow or cow.
    MixedArray::IterateKV<DynFn, incRef>(
      MixedArray::asMixed(obj->dynPropArray().get()),
      dynFn
    );
  }
}

template <typename DeclFn, typename DynFn>
ALWAYS_INLINE
void IteratePropMemOrderNoInc(const ObjectData* obj, DeclFn declFn,
                              DynFn dynFn) {
  IteratePropMemOrder<DeclFn, DynFn, false>(obj, std::move(declFn),
                                            std::move(dynFn));
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_OBJECT_ITERATOR_H_
