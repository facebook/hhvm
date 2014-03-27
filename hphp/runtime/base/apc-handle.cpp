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
#include "hphp/runtime/base/apc-handle.h"

#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/apc-object.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/hphp-array.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static APCHandle* getAPCHandle(const Variant& source) {
  auto const cell = source.asCell();
  if (cell->m_type == KindOfString) {
    return cell->m_data.pstr->getAPCHandle();
  }
  if (cell->m_type == KindOfArray) {
    return cell->m_data.parr->getAPCHandle();
  }
  return nullptr;
}

APCHandle* APCHandle::Create(const Variant& source,
                             bool serialized,
                             bool inner /* = false */,
                             bool unserializeObj /* = false*/) {
  // if a wrapper of an existing APC object is provided then just use
  // the wrapped APC object.
  // getAPCHandle() is responsible to check the conditions under which
  // a wrapped object can be returned
  auto wrapped = getAPCHandle(source);
  if (UNLIKELY(wrapped && !unserializeObj && !wrapped->getUncounted())) {
    wrapped->reference();
    return wrapped;
  }
  return CreateSharedType(source, serialized, inner, unserializeObj);
}

APCHandle* APCHandle::CreateSharedType(const Variant& source,
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
      if (!inner && apcExtension::UseUncounted) {
        StringData* st = StringData::MakeUncounted(s->slice());
        APCTypedValue* value = new APCTypedValue(st);
        return value->getHandle();
      }
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
  assert(!getUncounted());
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

///////////////////////////////////////////////////////////////////////////////

APCHandle* APCTypedValue::MakeSharedArray(ArrayData* array) {
  assert(apcExtension::UseUncounted);
  auto value = new APCTypedValue(HphpArray::MakeUncounted(array));
  return value->getHandle();
}

void APCTypedValue::deleteUncounted() {
  assert(m_handle.getUncounted());
  DataType type = m_handle.getType();
  assert(type == KindOfString || type == KindOfArray);
  if (type == KindOfString) {
    m_data.str->destructStatic();
  } else if (type == KindOfArray) {
    HphpArray::ReleaseUncounted(m_data.arr);
  }
  delete this;
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
