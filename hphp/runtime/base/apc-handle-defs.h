/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_APC_HANDLE_DEFS_H_
#define incl_HPHP_APC_HANDLE_DEFS_H_

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/ext/ext_collections.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void APCHandle::unreferenceRoot(size_t size) {
  if (!isUncounted()) {
    realDecRef();
  } else {
    if (size == 0) {
      // it's unlikely we have a nested uncounted array but in case, we
      // compute the size on delete. For uncounted strings this is a
      // pretty cheap operation
      size = getMemSize(this);
    }
    g_context->enqueueAPCHandle(this, size);
  }
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Traverse a php object calling the 'checker' function on each element.
 * If the 'checker' function returns true the walk stops.
 * This is a shallow walk so only one level is analyzed.
 * ByRef elements are dereferenced given that is the APC behavior.
 * Return true if the walk was aborted, false otherwise.
 */
template<class Fun>
bool traverseData(const Variant& data, Fun checker) {
  switch (data.getType()) {
  case KindOfArray: {
    ArrayData* arr = data.getArrayData();
    for (ArrayIter iter(arr); iter; ++iter) {
      const Variant& var = iter.secondRef();
      if (checker(var)) return true;
    }
    return false;
  }
  case KindOfObject: {
    ObjectData* obj = data.getObjectData();
    auto colType = obj->getCollectionType();
    switch (colType) {
    case Collection::Type::InvalidType:
      return traverseData(obj->o_toArray(), checker);
    case Collection::Type::VectorType:
    case Collection::Type::ImmVectorType:
      return traverseData(
          static_cast<BaseVector*>(obj)->arrayData(), checker);
    case Collection::Type::MapType:
    case Collection::Type::ImmMapType:
    case Collection::Type::SetType:
    case Collection::Type::ImmSetType:
      return traverseData(
          static_cast<HashCollection*>(obj)->arrayData()->asArrayData(),
          checker);
    case Collection::Type::PairType:
      if (checker(tvAsCVarRef(static_cast<c_Pair*>(obj)->get(0)))) {
        return true;
      }
      return checker(tvAsCVarRef(static_cast<c_Pair*>(obj)->get(1)));
    }
  }
  case KindOfResource:
    return checker(data.getResourceData());
  default:
    // do nothing
    return false;
  }
}

/*
 * Traverse a php object calling the 'checker' function on each element
 * recursively.
 * If the 'checker' function returns true the walk stops.
 * Perform a depth-first walk.
 */
template<class Fun>
bool traverseDataRecursive(const Variant& data, Fun checker) {
  bool result = traverseData(data,
      [&](const Variant& v) {
        if (!checker(v)) {
          return traverseDataRecursive(v, checker);
        }
        return true;
      }
  );
  return result;
}

}

#endif
