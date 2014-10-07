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
#include "hphp/runtime/base/data-walker.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/array-iterator.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

void DataWalker::traverseData(ArrayData* data,
                              DataFeature& features,
                              PointerSet& visited) const {
  // shared arrays by definition do not contain circular references or
  // collections
  if (data->isSharedArray()) {
    // If not looking for references to objects/resources OR
    // if one was already found we can bail out
    if (!(m_features & LookupFeature::HasObjectOrResource) ||
        features.hasObjectOrResource()) {
      features.m_hasRefCountReference = true; // just in case, cheap enough...
      return;
    }
  }

  for (ArrayIter iter(data); iter; ++iter) {
    const Variant& var = iter.secondRef();

    if (var.isReferenced()) {
      Variant *pvar = var.getRefData();
      if (markVisited(pvar, features, visited)) {
        // don't recurse forever
        if (canStopWalk(features)) {
          return;
        }
        continue;
      }
      markVisited(pvar, features, visited);
    }

    DataType type = var.getType();
    // cheap enough, do it always
    features.m_hasRefCountReference = IS_REFCOUNTED_TYPE(type);
    if (type == KindOfObject) {
      features.m_hasObjectOrResource = true;
      traverseData(var.getObjectData(), features, visited);
    } else if (type == KindOfArray) {
      traverseData(var.getArrayData(), features, visited);
    } else if (type == KindOfResource) {
      features.m_hasObjectOrResource = true;
    }
    if (canStopWalk(features)) return;
  }
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
    traverseData(data->o_toArray().get(), features, visited);
  }
}

inline bool DataWalker::markVisited(
    void* pvar,
    DataFeature& features,
    PointerSet& visited) const {
  if (visited.find(pvar) != visited.end()) {
    features.m_circular = true;
    return true;
  }
  visited.insert(pvar);
  return false;
}

inline void DataWalker::objectFeature(
    ObjectData* pobj,
    DataFeature& features) const {
  // REVIEW: right now collections always stop the walk, not clear
  // if they should do so moving forward. Revisit...
  // Notice that worst case scenario here we will be serializing things
  // that we could keep in better format so it should not break anything
  if (pobj->isCollection()) {
    features.m_hasCollection = true;
  } else if ((m_features & LookupFeature::DetectSerializable) &&
             pobj->instanceof(SystemLib::s_SerializableClass)) {
    features.m_serializable = true;
  }
}

inline bool DataWalker::canStopWalk(DataFeature& features) const {
  auto refCountCheck =
    features.hasRefCountReference() ||
    !(m_features & LookupFeature::RefCountedReference);
  auto objectCheck =
    features.hasObjectOrResource() ||
    !(m_features & LookupFeature::HasObjectOrResource);
  auto defaultChecks =
      features.isCircular() || features.hasCollection() ||
      features.hasSerializableReference();
  return refCountCheck && objectCheck && defaultChecks;
}

//////////////////////////////////////////////////////////////////////

}
