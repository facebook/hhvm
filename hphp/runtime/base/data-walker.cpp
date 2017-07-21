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
                              PointerSet& visited) const {
  for (ArrayIter iter(data); iter; ++iter) {
    auto const rval = iter.secondRval();

    if (rval.type() == KindOfRef &&
        rval.val().pref->isReferenced()) {
      if (markVisited(rval.val().pref->var(), features, visited)) {
        if (canStopWalk(features)) return;
        continue; // don't recurse forever; we already went down this path
      }
      // Right now consider it circular even if the referenced variant only
      // showed up in one spot.  This could be revisted later.
      features.isCircular = true;
      if (canStopWalk(features)) return;
    }

    auto const inner = tvToCell(rval);
    // cheap enough, do it always
    features.hasRefCountReference = isRefcountedType(inner.type());
    if (inner.type() == KindOfObject) {
      features.hasObjectOrResource = true;
      traverseData(inner.val().pobj, features, visited);
    } else if (isArrayLikeType(inner.type())) {
      traverseData(inner.val().parr, features, visited);
    } else if (inner.type() == KindOfResource) {
      features.hasObjectOrResource = true;
    }
    if (canStopWalk(features)) return;
  }
}

void DataWalker::traverseData(
    ObjectData* data,
    DataFeature& features,
    PointerSet& visited) const {
  objectFeature(data, features, visited);
  if (markVisited(data, features, visited)) {
    return; // avoid infinite recursion
  }
  if (!canStopWalk(features)) {
    traverseData(data->toArray().get(), features, visited);
  }
}

inline bool DataWalker::markVisited(
    void* pvar,
    DataFeature& features,
    PointerSet& visited) const {
  if (!visited.insert(pvar).second) {
    features.isCircular = true;
    return true;
  }
  return false;
}

inline void DataWalker::objectFeature(ObjectData* pobj, DataFeature& features,
                                      PointerSet& /*visited*/) const {
  if (pobj->isCollection()) return;
  if ((m_features & LookupFeature::DetectSerializable) &&
       pobj->instanceof(SystemLib::s_SerializableClass)) {
    features.hasSerializable = true;
  }
}

inline bool DataWalker::canStopWalk(DataFeature& features) const {
  auto refCountCheck =
    features.hasRefCountReference ||
    !(m_features & LookupFeature::RefCountedReference);
  auto objectCheck =
    features.hasObjectOrResource ||
    !(m_features & LookupFeature::HasObjectOrResource);
  auto defaultChecks = features.isCircular || features.hasSerializable;
  return refCountCheck && objectCheck && defaultChecks;
}

//////////////////////////////////////////////////////////////////////

}
