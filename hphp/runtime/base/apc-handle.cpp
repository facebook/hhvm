/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/apc-handle.h"

#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/apc-object.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/runtime/base/array-iterator.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

APCHandle *Variant::getAPCHandle() const {
  if (m_type == KindOfRef) {
    return m_data.pref->var()->getAPCHandle();
  }
  if (m_type == KindOfString) {
    return m_data.pstr->getAPCHandle();
  }
  if (m_type == KindOfArray) {
    return m_data.parr->getAPCHandle();
  }
  return nullptr;
}

APCHandle* APCHandle::Create(CVarRef source,
                             bool serialized,
                             bool inner /* = false */,
                             bool unserializeObj /* = false*/) {
  // if a wrapper of an existing APC object is provided then just use
  // the wrapped APC object.
  // getAPCHandle() is responsible to check the conditions under which
  // a wrapped object can be returned
  auto wrapped = source.getAPCHandle();
  if (UNLIKELY(wrapped && !unserializeObj)) {
    wrapped->incRef();
    return wrapped;
  }
  return CreateSharedType(source, serialized, inner, unserializeObj);
}

APCHandle* APCHandle::CreateSharedType(CVarRef source,
                                       bool serialized,
                                       bool inner,
                                       bool unserializeObj) {
  auto type = source.getType(); // this gets rid of the ref, if it was one
  switch (type) {
    case KindOfBoolean: {
      auto value = new APCTypedValue(type,
          static_cast<int64_t>(source.getBoolean()));
      return value->getHandle();
    }
    case KindOfInt64: {
      auto value = new APCTypedValue(type, source.getInt64());
      return value->getHandle();
    }
    case KindOfDouble: {
      auto value = new APCTypedValue(type, source.getDouble());
      return value->getHandle();
    }
    case KindOfUninit:
    case KindOfNull: {
      auto value = new APCTypedValue(type);
      return value->getHandle();
    }

    case KindOfStaticString: {
      if (serialized) goto StringCase;

      auto value = new APCTypedValue(type, source.getStringData());
      return value->getHandle();
    }
StringCase:
    case KindOfString: {
      StringData* s = source.getStringData();
      if (serialized) {
        // It is priming, and there might not be the right class definitions
        // for unserialization.
        return APCObject::MakeShared(apc_reserialize(s));
      }

      auto const st = lookupStaticString(s);
      if (st) {
        APCTypedValue* value = new APCTypedValue(KindOfStaticString, st);
        return value->getHandle();
      }

      assert(!s->isStatic()); // would've been handled above
      return APCString::MakeShared(type, s);
    }

    case KindOfArray:
      return APCArray::MakeShared(source.getArrayData(),
                                  inner,
                                  unserializeObj);

    case KindOfResource:
      // TODO Task #2661075: Here and elsewhere in the runtime, we convert
      // Resources to the empty array during various serialization operations,
      // which does not match Zend behavior. We should fix this.
      return APCArray::MakeShared();

    case KindOfObject:
      return unserializeObj ? APCObject::Construct(source.getObjectData())
                            : APCObject::MakeShared(apc_serialize(source));

    default:
      return nullptr;
  }
}

Variant APCHandle::toLocal() {
  switch (m_type) {
    case KindOfBoolean:
      return APCTypedValue::fromHandle(this)->getBoolean();
    case KindOfInt64:
      return APCTypedValue::fromHandle(this)->getInt64();
    case KindOfDouble:
      return APCTypedValue::fromHandle(this)->getDouble();
    case KindOfUninit:
    case KindOfNull:
      return null_variant; // shortcut.. no point to forward
    case KindOfStaticString:
      return APCTypedValue::fromHandle(this)->getStringData();

    case KindOfString:
      return APCString::MakeString(this);

    case KindOfArray:
      return APCArray::MakeArray(this);

    case KindOfObject:
      return APCObject::MakeObject(this);

    default:
      assert(false);
      return null_variant;
  }
}

void APCHandle::deleteShared() {
  switch (m_type) {
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfUninit:
    case KindOfNull:
    case KindOfStaticString:
      delete APCTypedValue::fromHandle(this);
      break;

    case KindOfString:
      delete APCString::fromHandle(this);
      break;

    case KindOfArray:
      APCArray::Delete(this);
      break;

    case KindOfObject:
      APCObject::Delete(this);
      break;

    default:
      assert(false);
  }
}

//
// Stats API
//
void APCHandle::getStats(APCHandleStats *stats) const {
  stats->initStats();
  stats->variantCount = 1;
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfStaticString:
      stats->dataSize = sizeof(double);
      stats->dataTotalSize = sizeof(APCTypedValue);
      break;
    case KindOfObject:
      if (getIsObj()) {
        APCHandleStats childStats;
        APCObject::fromHandle(const_cast<APCHandle*>(this))->
                                                getSizeStats(&childStats);
        stats->addChildStats(&childStats);
        break;
      }
    // fall through
    case KindOfString:
      stats->dataSize =
          APCString::fromHandle(const_cast<APCHandle*>(this))->
                                                getStringData()->size();
      stats->dataTotalSize = sizeof(APCString) + stats->dataSize;
      break;
    default:
      assert(is(KindOfArray));
      if (getSerializedArray()) {
        stats->dataSize =
            APCString::fromHandle(const_cast<APCHandle*>(this))->
                                                getStringData()->size();
        stats->dataTotalSize = sizeof(APCArray) + stats->dataSize;
        break;
      }
      APCArray* arr = APCArray::fromHandle(const_cast<APCHandle*>(this));
      stats->dataTotalSize = sizeof(APCArray);
      if (isPacked()) {
        auto size = arr->size();
        stats->dataTotalSize += sizeof(APCHandle*) * size;
        for (size_t i = 0; i < size; i++) {
          APCHandle *v = arr->vals()[i];
          APCHandleStats childStats;
          v->getStats(&childStats);
          stats->addChildStats(&childStats);
        }
      } else {
        stats->dataTotalSize += sizeof(APCArray::Bucket) * arr->size() +
                                sizeof(int) * (arr->capacity() + 1);
        for (size_t i = 0, n = arr->size(); i < n; i++) {
          APCHandleStats childStats;
          arr->buckets()[i].key->getStats(&childStats);
          stats->addChildStats(&childStats);
          arr->buckets()[i].val->getStats(&childStats);
          stats->addChildStats(&childStats);
        }
      }
      break;
  }
}

int32_t APCHandle::getSpaceUsage() const {
  uint32_t size = sizeof(APCHandle);
  if (!IS_REFCOUNTED_TYPE(m_type)) return size;
  switch (m_type) {
    case KindOfObject:
      if (getIsObj()) {
        return size +
          APCObject::fromHandle(const_cast<APCHandle*>(this))->getSpaceUsage();
      }
    // fall through
    case KindOfString:
      size += sizeof(StringData) +
          APCString::fromHandle(const_cast<APCHandle*>(this))->
                                              getStringData()->size();
      break;
    default:
      assert(is(KindOfArray));
      if (getSerializedArray()) {
        size += sizeof(StringData) +
            APCString::fromHandle(const_cast<APCHandle*>(this))->
                                              getStringData()->size();
        break;
      }
      APCArray* arr = APCArray::fromHandle(const_cast<APCHandle*>(this));
      if (isPacked()) {
        auto size = arr->size();
        size += sizeof(APCArray) + size * sizeof(APCHandle*);
        for (size_t i = 0, n = arr->size(); i < n; i++) {
          size += arr->vals()[i]->getSpaceUsage();
        }
      } else {
        size += sizeof(APCArray::Bucket) * arr->size() +
                sizeof(int) * (arr->capacity() + 1);
        for (size_t i = 0, n = arr->size(); i < n; i++) {
          size += arr->buckets()[i].key->getSpaceUsage();
          size += arr->buckets()[i].val->getSpaceUsage();
        }
      }
      break;
  }
  return size;
}

///////////////////////////////////////////////////////////////////////////////

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
    CVarRef var = iter.secondRef();

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

inline
bool DataWalker::markVisited(
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

inline
void DataWalker::objectFeature(
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

inline
bool DataWalker::canStopWalk(DataFeature& features) const {
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

///////////////////////////////////////////////////////////////////////////////
}
