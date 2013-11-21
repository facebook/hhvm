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

#include "hphp/runtime/base/apc-object.h"

#include <cstdlib>

#include "hphp/util/logger.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/ext_apc.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
APCObject::APCObject(ObjectData* obj)
  : m_handle(KindOfObject)
  , m_cls{obj->getVMClass()}
{
  // This function assumes the object and object/array down the tree
  // have no internal references and do not implement the serializable
  // interface.
  assert(!obj->instanceof(SystemLib::s_SerializableClass));

  m_handle.setIsObj();
  m_handle.mustCache();

  Array props;
  obj->o_getArray(props, false);
  m_propCount = 0;
  if (props.empty()) {
    m_props = nullptr;
    return;
  }

  m_props = static_cast<Prop*>(malloc(sizeof(Prop) * props.size()));

  for (ArrayIter it(props); !it.end(); it.next()) {
    assert(m_propCount < props.size());
    Variant key(it.first());
    assert(key.isString());
    CVarRef value = it.secondRef();
    APCHandle *val = nullptr;
    if (!value.isNull()) {
      val = APCHandle::Create(value, false, true, true);
    }

    const String& keySD = key.asCStrRef();
    auto& prop = m_props[m_propCount];

    if (!keySD->empty() && *keySD->data() == '\0') {
      int32_t subLen = keySD.find('\0', 1) + 1;
      String cls = keySD.substr(1, subLen - 2);
      if (cls.size() == 1 && cls[0] == '*') {
        // Protected.
        prop.ctx = ClassOrString{nullptr};
      } else {
        // Private.
        prop.ctx = ClassOrString{Unit::lookupClass(cls.get())};
      }

      prop.name = makeStaticString(keySD.substr(subLen));
    } else {
      prop.ctx = ClassOrString{nullptr};
      prop.name = makeStaticString(keySD.get());
    }

    prop.val = val;

    ++m_propCount;
  }
}

ALWAYS_INLINE
APCObject::~APCObject() {
  if (!m_props) return;
  for (int i = 0; i < m_propCount; i++) {
    if (m_props[i].val) m_props[i].val->decRef();
    assert(m_props[i].name->isStatic());
  }
  free(m_props);
}

//////////////////////////////////////////////////////////////////////

APCHandle* APCObject::MakeShared(ObjectData* data) {
  auto const apcObj = new APCObject(data);
  return apcObj->getHandle();
}

APCHandle* APCObject::MakeAPCObject(APCHandle* obj, CVarRef value) {
  if (!value.is(KindOfObject) || obj->getObjAttempted()) {
    return nullptr;
  }
  obj->setObjAttempted();
  ObjectData *o = value.getObjectData();
  if (o->instanceof(SystemLib::s_SerializableClass)) {
    // should also check the object itself
    return nullptr;
  }
  PointerSet seen;
  if (o->hasInternalReference(seen, true)) {
    return nullptr;
  }
  APCHandle* tmp = APCHandle::Create(value, false, true, true);
  tmp->setObjAttempted();
  return tmp;
}

Variant APCObject::MakeObject(APCHandle* handle) {
  if (handle->getIsObj()) {
    return APCObject::fromHandle(handle)->createObject();
  }
  StringData* serObj = APCString::fromHandle(handle)->getStringData();
  return apc_unserialize(serObj->data(), serObj->size());
}

void APCObject::Delete(APCHandle* handle) {
  (handle->getIsObj()) ? delete APCObject::fromHandle(handle)
                       : delete APCString::fromHandle(handle);
}

Object APCObject::createObject() const {
  Object obj;

  const Class* klass;
  if (auto const c = m_cls.cls()) {
    klass = c;
  } else {
    klass = Unit::loadClass(m_cls.name());
    if (!klass) {
      Logger::Error("APCObject::getObject(): Cannot find class %s",
                    m_cls.name()->data());
      return obj;
    }
  }
  obj = ObjectData::newInstance(const_cast<Class*>(klass));
  obj.get()->clearNoDestruct();

  if (auto prop = m_props) {
    auto const propEnd = prop + m_propCount;
    for (; prop != propEnd; ++prop) {
      auto const key = prop->name;

      const Class* ctx = nullptr;
      if (prop->ctx.isNull()) {
        ctx = klass;
      } else {
        if (auto const cls = prop->ctx.cls()) {
          ctx = prop->ctx.cls();
        } else {
          ctx = Unit::lookupClass(prop->ctx.name());
          if (!ctx) continue;
        }
      }

      auto val = prop->val ? prop->val->toLocal() : null_variant;
      obj->setProp(const_cast<Class*>(ctx), key, val.asTypedValue(), false);
    }
  }

  obj->invokeWakeup();
  return obj;
}

//
// Stats API
//

void APCObject::getSizeStats(APCHandleStats* stats) const {
  stats->initStats();
  stats->dataTotalSize += sizeof(APCObject) + sizeof(Prop) * m_propCount;

  for (int i = 0; i < m_propCount; i++) {
    auto const sd = m_props[i].name;
    if (!sd->isStatic()) {
      stats->dataSize += sd->size();
      stats->dataTotalSize += sizeof(StringData) + sd->size();
    }
    if (m_props[i].val) {
      APCHandleStats childStats;
      m_props[i].val->getStats(&childStats);
      stats->addChildStats(&childStats);
    }
  }
}

int32_t APCObject::getSpaceUsage() const {
  int32_t size = sizeof(APCObject) + sizeof(Prop) * m_propCount;

  for (int i = 0; i < m_propCount; i++) {
    auto const sd = m_props[i].name;
    if (!sd->isStatic()) {
      size += sizeof(StringData) + sd->size();
    }
    if (m_props[i].val) {
      size += m_props[i].val->getSpaceUsage();
    }
  }
  return size;
}

//////////////////////////////////////////////////////////////////////

}
