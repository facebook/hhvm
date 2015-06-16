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

#include "hphp/runtime/base/apc-object.h"

#include <cstdlib>

#include "hphp/util/logger.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-collection.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

Either<const Class*,const StringData*> make_class(const Class* c) {
  if (classHasPersistentRDS(c)) return c;
  return c->preClass()->name();
}

}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
APCObject::APCObject(ObjectData* obj, uint32_t propCount)
  : m_handle(KindOfObject)
  , m_cls{make_class(obj->getVMClass())}
  , m_propCount{propCount}
{}

APCHandle::Pair APCObject::Construct(ObjectData* objectData) {
  // This function assumes the object and object/array down the tree
  // have no internal references and do not implement the serializable
  // interface.
  assert(!objectData->instanceof(SystemLib::s_SerializableClass));

  Array odProps;
  objectData->o_getArray(odProps);
  auto const propCount = odProps.size();

  auto size = sizeof(APCObject) + sizeof(Prop) * propCount;
  auto const apcObj = new (std::malloc(size)) APCObject(objectData, propCount);
  if (!propCount) return {apcObj->getHandle(), size};

  auto prop = apcObj->props();
  for (ArrayIter it(odProps); !it.end(); it.next(), ++prop) {
    Variant key(it.first());
    assert(key.isString());
    const Variant& value = it.secondRef();
    if (!value.isNull()) {
      auto val = APCHandle::Create(value, false, true, true);
      prop->val = val.handle;
      size += val.size;
    } else {
      prop->val = nullptr;
    }

    const String& keySD = key.asCStrRef();

    if (!keySD.empty() && *keySD.data() == '\0') {
      int32_t subLen = keySD.find('\0', 1) + 1;
      String cls = keySD.substr(1, subLen - 2);
      if (cls.size() == 1 && cls[0] == '*') {
        // Protected.
        prop->ctx = nullptr;
      } else {
        // Private.
        auto* ctx = Unit::lookupClass(cls.get());
        if (ctx && ctx->attrs() & AttrUnique) {
          prop->ctx = ctx;
        } else {
          prop->ctx = makeStaticString(cls.get());
        }
      }
      prop->name = makeStaticString(keySD.substr(subLen));
    } else {
      prop->ctx = nullptr;
      prop->name = makeStaticString(keySD.get());
    }
  }
  assert(prop == apcObj->props() + propCount);

  return {apcObj->getHandle(), size};
}

ALWAYS_INLINE
APCObject::~APCObject() {
  for (auto i = uint32_t{0}; i < m_propCount; ++i) {
    if (props()[i].val) props()[i].val->unreference();
    assert(props()[i].name->isStatic());
  }
}

void APCObject::Delete(APCHandle* handle) {
  if (handle->isSerializedObj()) {
    delete APCString::fromHandle(handle);
    return;
  }

  auto const obj = fromHandle(handle);
  obj->~APCObject();
  // No need to run Prop destructors.
  std::free(obj);
}

//////////////////////////////////////////////////////////////////////

APCHandle::Pair APCObject::MakeAPCObject(APCHandle* obj, const Variant& value) {
  if (!value.is(KindOfObject) || obj->objAttempted()) {
    return {nullptr, 0};
  }
  obj->setObjAttempted();
  ObjectData *o = value.getObjectData();
  DataWalker walker(DataWalker::LookupFeature::DetectSerializable);
  DataWalker::DataFeature features = walker.traverseData(o);
  if (features.isCircular || features.hasSerializable) {
    return {nullptr, 0};
  }
  auto tmp = APCHandle::Create(value, false, true, true);
  tmp.handle->setObjAttempted();
  return tmp;
}

Variant APCObject::MakeObject(const APCHandle* handle) {
  if (handle->isSerializedObj()) {
    auto const serObj = APCString::fromHandle(handle)->getStringData();
    return apc_unserialize(serObj->data(), serObj->size());
  }
  if (handle->isAPCCollection()) {
    return APCCollection::fromHandle(handle)->createObject();
  }
  return APCObject::fromHandle(handle)->createObject();
}

Object APCObject::createObject() const {
  const Class* klass;
  if (auto const c = m_cls.left()) {
    klass = c;
  } else {
    klass = Unit::loadClass(m_cls.right());
    if (!klass) {
      Logger::Error("APCObject::getObject(): Cannot find class %s",
                    m_cls.right()->data());
      return Object{};
    }
  }
  Object obj{const_cast<Class*>(klass)};
  obj->clearNoDestruct();

  auto prop = props();
  auto const propEnd = prop + m_propCount;
  for (; prop != propEnd; ++prop) {
    auto const key = prop->name;

    const Class* ctx = nullptr;
    if (prop->ctx.isNull()) {
      ctx = klass;
    } else {
      if (auto const cls = prop->ctx.left()) {
        ctx = cls;
      } else {
        ctx = Unit::lookupClass(prop->ctx.right());
        if (!ctx) continue;
      }
    }

    auto val = prop->val ? prop->val->toLocal() : init_null();
    obj->setProp(const_cast<Class*>(ctx), key, val.asTypedValue(), false);
  }

  obj->invokeWakeup();
  return obj;
}

//////////////////////////////////////////////////////////////////////

}
