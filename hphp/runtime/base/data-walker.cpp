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
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/object-iterator.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

void DataWalker::traverseData(ArrayData* data,
                              DataFeature& features,
                              PointerSet& visited,
                              PointerMap* seenArrs) const {
  // Static and Uncounted arrays are never circular, never contain
  // objects or resources, so there's no need to traverse them.
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

  IterateV(data, [&](TypedValue rval) {
    visitTypedValue(rval, features, visited, seenArrs);
  });
}

void DataWalker::traverseData(
    ObjectData* data,
    DataFeature& features,
    PointerSet& visited) const {
  objectFeature(data, features);
  if (markVisited(data, features, visited)) {
    return; // avoid infinite recursion
  }
  if (canStopWalk(features)) return;

  if (data->isCollection()) {
    auto const arr = collections::asArray(data);
    if (arr) {
      traverseData(arr, features, visited);
      return;
    }
    assertx(data->collectionType() == CollectionType::Pair);
    auto const pair = static_cast<c_Pair*>(data);
    visitTypedValue(*pair->get(0), features, visited);
    visitTypedValue(*pair->get(1), features, visited);
    return;
  }

  IteratePropMemOrder(
    data,
    [&](Slot slot, const Class::Prop& prop, tv_rval val) {
      visitTypedValue(val.tv(), features, visited);
    },
    [&](TypedValue key_tv, TypedValue val) {
      visitTypedValue(val, features, visited);
    }
  );
}

ALWAYS_INLINE
bool DataWalker::visitTypedValue(TypedValue rval,
                                 DataFeature& features,
                                 PointerSet& visited,
                                 PointerMap* seenArrs) const {
  auto const serialize_funcs = RuntimeOption::EvalAPCSerializeFuncs;
  auto const serialize_clsmeth = RO::EvalAPCSerializeClsMeth;

  if (rval.m_type == KindOfObject) {
    features.hasNonPersistable = true;
    traverseData(rval.m_data.pobj, features, visited);
  } else if (isArrayLikeType(rval.m_type)) {
    traverseData(rval.m_data.parr, features, visited, seenArrs);
  } else if (rval.m_type == KindOfResource) {
    features.hasNonPersistable = true;
  } else if (rval.m_type == KindOfRFunc) {
    features.hasNonPersistable = true;
  } else if (rval.m_type == KindOfFunc) {
    if (!serialize_funcs) features.hasNonPersistable = true;
    if (rval.m_data.pfunc->isMethCaller()) features.hasNonPersistable = true;
    if (!rval.m_data.pfunc->isPersistent()) features.hasNonPersistable = true;
  } else if (rval.m_type == KindOfClass) {
    if (!rval.m_data.pclass->isPersistent()) {
      features.hasNonPersistable = true;
    }
  } else if (rval.m_type == KindOfClsMeth) {
    if (!serialize_clsmeth) features.hasNonPersistable = true;
    if (!rval.m_data.pclsmeth->getCls()->isPersistent()) {
      features.hasNonPersistable = true;
    }
  }
  return canStopWalk(features);
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
  if (m_feature == LookupFeature::DetectSerializable &&
      pobj->instanceof(SystemLib::s_SerializableClass)) {
    features.hasSerializable = true;
  }
}

inline bool DataWalker::canStopWalk(DataFeature& features) const {
  /*
   * This encodes some assumptions about our callers and our implementation:
   *
   * Those who pass DetectNonPersistable need to know if the data isCircular,
   * and only if it's not do they care whether it hasNonPersistable; because of
   * this we can only stop early if both are true for these callers.
   *
   * On the other hand, those that pass DetectSerializable react the same to
   * a result of isCircular and hasSerializable, so for these callers we can
   * stop if either is true (callers not passing DetectSerializable are not
   * affected by this logic since we only do the check needed to set
   * hasSerializable when passed DetectSerializable).
   */
  auto nonPersistableCheck =
    features.hasNonPersistable ||
    m_feature != LookupFeature::DetectNonPersistable;
  auto defaultChecks = features.isCircular || features.hasSerializable;
  return nonPersistableCheck && defaultChecks;
}

//////////////////////////////////////////////////////////////////////

}
