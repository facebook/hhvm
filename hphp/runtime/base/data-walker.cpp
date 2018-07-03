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
#include "hphp/runtime/base/data-walker.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/collections.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

void DataWalker::traverseData(ArrayData* data,
                              DataFeature& features,
                              PointerSet& visited,
                              PointerMap* seenArrs) const {
  // Static and Uncounted arrays are never circular, never contain
  // KindOfRef, and never contain objects or resources, so there's no
  // need to traverse them.
  if (!data->isRefCounted()) return;

  // At this point we're just using seenArrs to keep track of arrays
  // we've seen, and prevent traversing them multiple times. If we're
  // not using jemalloc, we'll also use this map to compute the size
  // of the uncounted array to avoid another recursive walk later, but
  // otherwise we only need to record arrays that might occur more
  // than once. The values will be filled in as we create uncounted
  // arrays for the keys.
  if (seenArrs && (!use_jemalloc || data->hasMultipleRefs()) &&
      !seenArrs->emplace(data, nullptr).second) {
    return;
  }

  auto const fn = [&] (TypedValue rval) {
    if (isRefType(rval.m_type)) {
      if (rval.m_data.pref->isReferenced()) {
        if (markVisited(rval.m_data.pref, features, visited)) {
          // Don't recurse forever; we already went down this path, and
          // stop the walk if we've already got everything we need.
          return canStopWalk(features);
        }
        // Right now consider it circular even if the referenced variant
        // only showed up in one spot.  This could be revisted later.
        features.isCircular = true;
        if (canStopWalk(features)) return true;
      }
      rval = *rval.m_data.pref->tv();
    }

    if (rval.m_type == KindOfObject) {
      features.hasObjectOrResource = true;
      traverseData(rval.m_data.pobj, features, visited);
    } else if (isArrayLikeType(rval.m_type)) {
      traverseData(rval.m_data.parr, features, visited, seenArrs);
    } else if (rval.m_type == KindOfResource) {
      features.hasObjectOrResource = true;
    }
    return canStopWalk(features);
  };
  IterateV<decltype(fn), false>(data, fn);
}

void DataWalker::traverseData(
    ObjectData* data,
    DataFeature& features,
    PointerSet& visited) const {
  objectFeature(data, features);
  if (markVisited(data, features, visited)) {
    return; // avoid infinite recursion
  }
  if (!canStopWalk(features)) {
    // Use asArray to avoid int-like string key coercion (which can hide
    // values).
    auto const arr = data->isCollection()
      ? collections::asArray(data)
      : nullptr;
    traverseData(arr ? arr : data->toArray().get(), features, visited);
  }
}

inline bool DataWalker::markVisited(
    HeapObject* ptr,
    DataFeature& features,
    PointerSet& visited) const {
  if (!visited.insert(ptr).second) {
    features.isCircular = true;
    return true;
  }
  return false;
}

inline void DataWalker::objectFeature(ObjectData* pobj,
                                      DataFeature& features) const {
  if (pobj->isCollection()) return;
  if ((m_features & LookupFeature::DetectSerializable) &&
       pobj->instanceof(SystemLib::s_SerializableClass)) {
    features.hasSerializable = true;
  }
}

inline bool DataWalker::canStopWalk(DataFeature& features) const {
  auto objectCheck =
    features.hasObjectOrResource ||
    !(m_features & LookupFeature::HasObjectOrResource);
  auto defaultChecks = features.isCircular || features.hasSerializable;
  return objectCheck && defaultChecks;
}

//////////////////////////////////////////////////////////////////////

}
