/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/apc-collection.h"
#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

APCObject::ClassOrName make_class(const Class* c) {
  if (classHasPersistentRDS(c)) return c;
  return c->preClass()->name();
}

const StaticString s___wakeup("__wakeup");

}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
APCObject::APCObject(ClassOrName cls, uint32_t propCount)
  : m_handle(APCKind::SharedObject)
  , m_cls{cls}
  , m_propCount{propCount}
  , m_persistent{0}
  , m_no_wakeup{0}
  , m_fast_init{0}
{}

APCHandle::Pair APCObject::Construct(ObjectData* objectData) {
  // This function assumes the object and object/array down the tree have no
  // internal references and do not implement the serializable interface.
  assert(!objectData->instanceof(SystemLib::s_SerializableClass));

  auto cls = objectData->getVMClass();
  auto clsOrName = make_class(cls);
  if (clsOrName.right()) return ConstructSlow(objectData, clsOrName);

  // We have a persistent Class. Build an array of APCHandle* to mirror the
  // declared properties in the object.
  auto const propInfo = cls->declProperties();
  auto const hasDynProps = objectData->hasDynProps();
  auto const numRealProps = propInfo.size();
  auto const numApcProps = numRealProps + hasDynProps;
  auto size = sizeof(APCObject) + sizeof(APCHandle*) * numApcProps;
  auto const apcObj = new (std::malloc(size)) APCObject(clsOrName, numApcProps);
  apcObj->m_persistent = 1;

  // Set a few more flags for faster fetching: whether or not the object has a
  // wakeup method, and whether or not we can use a fast path that avoids
  // default-initializing properties before setting them to their APC values.
  if (!cls->lookupMethod(s___wakeup.get())) apcObj->m_no_wakeup = 1;
  if (!cls->instanceCtor() && !cls->callsCustomInstanceInit()) {
    apcObj->m_fast_init = 1;
  }

  auto const apcPropVec = apcObj->persistentProps();
  auto const objPropVec = objectData->propVec();
  const TypedValueAux* propInit = nullptr;

  for (unsigned i = 0; i < numRealProps; ++i) {
    auto const attrs = propInfo[i].attrs;
    assert((attrs & AttrStatic) == 0);

    const TypedValue* objProp;
    if (attrs & AttrBuiltin) {
      // Special properties like the Memoize cache should be set to their
      // default value, not the current value.
      if (propInit == nullptr) {
        propInit = cls->pinitVec().empty() ? &cls->declPropInit()[0]
                                           : &(*cls->getPropData())[0];
      }

      objProp = propInit + i;
    } else {
      objProp = objPropVec + i;
    }

    auto val = APCHandle::Create(tvAsCVarRef(objProp), false,
                                 APCHandleLevel::Inner, true);
    size += val.size;
    apcPropVec[i] = val.handle;
  }

  if (UNLIKELY(hasDynProps)) {
    auto val = APCHandle::Create(objectData->dynPropArray(), false,
                                 APCHandleLevel::Inner, true);
    size += val.size;
    apcPropVec[numRealProps] = val.handle;
  }

  return {apcObj->getHandle(), size};
}

NEVER_INLINE
APCHandle::Pair APCObject::ConstructSlow(ObjectData* objectData,
                                         ClassOrName name) {
  Array odProps;
  objectData->o_getArray(odProps);
  auto const propCount = odProps.size();

  auto size = sizeof(APCObject) + sizeof(Prop) * propCount;
  auto const apcObj = new (std::malloc(size)) APCObject(name, propCount);
  if (!propCount) return {apcObj->getHandle(), size};

  auto prop = apcObj->props();
  for (ArrayIter it(odProps); !it.end(); it.next(), ++prop) {
    Variant key(it.first());
    assert(key.isString());
    const Variant& value = it.secondRef();
    if (!value.isNull()) {
      auto val = APCHandle::Create(value, false, APCHandleLevel::Inner, true);
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
  auto const numProps = m_propCount;

  if (m_persistent) {
    auto props = persistentProps();
    for (unsigned i = 0; i < numProps; ++i) {
      if (props[i]) props[i]->unreference();
    }
    return;
  }

  for (auto i = uint32_t{0}; i < numProps; ++i) {
    if (props()[i].val) props()[i].val->unreference();
    assert(props()[i].name->isStatic());
  }
}

void APCObject::Delete(APCHandle* handle) {
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
  ObjectData* o = value.getObjectData();
  DataWalker walker(DataWalker::LookupFeature::DetectSerializable);
  DataWalker::DataFeature features = walker.traverseData(o);
  if (features.isCircular || features.hasSerializable) {
    return {nullptr, 0};
  }
  auto tmp = APCHandle::Create(value, false, APCHandleLevel::Inner, true);
  tmp.handle->setObjAttempted();
  return tmp;
}

Variant APCObject::MakeLocalObject(const APCHandle* handle) {
  auto apcObj = APCObject::fromHandle(handle);
  return apcObj->m_persistent ? apcObj->createObject()
                              : apcObj->createObjectSlow();
}

Object APCObject::createObject() const {
  auto cls = m_cls.left();
  assert(cls != nullptr);

  auto obj = Object::attach(
    m_fast_init ? ObjectData::newInstanceNoPropInit(const_cast<Class*>(cls))
                : ObjectData::newInstance(const_cast<Class*>(cls))
  );

  auto const numProps = cls->numDeclProperties();
  auto const objProp = obj->propVec();
  auto const apcProp = persistentProps();

  if (m_fast_init) {
    for (unsigned i = 0; i < numProps; ++i) {
      new (objProp + i) Variant(apcProp[i]->toLocal());
    }
  } else {
    for (unsigned i = 0; i < numProps; ++i) {
      tvAsVariant(&objProp[i]) = apcProp[i]->toLocal();
    }
  }

  if (UNLIKELY(numProps < m_propCount)) {
    auto dynProps = apcProp[numProps];
    assert(dynProps->kind() == APCKind::StaticArray ||
           dynProps->kind() == APCKind::UncountedArray ||
           dynProps->kind() == APCKind::SharedArray);
    obj->setDynPropArray(dynProps->toLocal().asCArrRef());
  }

  if (!m_no_wakeup) obj->invokeWakeup();
  return obj;
}

Object APCObject::createObjectSlow() const {
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
