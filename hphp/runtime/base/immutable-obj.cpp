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

#include "hphp/runtime/base/immutable-obj.h"

#include <cstdlib>

#include "hphp/util/logger.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/shared-variant.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/builtin-functions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

ImmutableObj::ImmutableObj(ObjectData* obj)
  : m_cls(obj->o_getClassName().get())
{
  assert(m_cls->isStatic());

  // This function assumes the object and object/array down the tree
  // have no internal references and do not implement the serializable
  // interface.
  assert(!obj->instanceof(SystemLib::s_SerializableClass));

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
    SharedVariant *val = nullptr;
    if (!value.isNull()) {
      val = new SharedVariant(value, false, true, true);
    }

    auto const keySD = key.getStringData();

    m_props[m_propCount].val = val;
    m_props[m_propCount].name = LIKELY(keySD->isStatic())
      ? keySD
      : StringData::MakeMalloced(keySD->data(), keySD->size());
    m_propCount++;
  }
}

ImmutableObj::~ImmutableObj() {
  assert(m_cls->isStatic());

  if (m_props) {
    for (int i = 0; i < m_propCount; i++) {
      if (m_props[i].val) m_props[i].val->decRef();
      if (UNLIKELY(!m_props[i].name->isStatic())) {
        m_props[i].name->destruct();
      }
    }
    free(m_props);
  }
}

Object ImmutableObj::getObject() const {
  Object obj;
  try {
    obj = create_object_only(m_cls);
  } catch (ClassNotFoundException& e) {
    Logger::Error("ImmutableObj::getObject(): Cannot find class %s",
                  m_cls->data());
    return obj;
  }
  obj.get()->clearNoDestruct();

  ArrayInit ai(m_propCount);
  for (int i = 0; i < m_propCount; i++) {
    auto const name = m_props[i].name;
    ai.add(
      String(name->isStatic() ? name
                              : StringData::Make(name->slice(), CopyString)),
      m_props[i].val ? m_props[i].val->toLocal() : null_variant,
      true
    );
  }

  Array v = ai.create();
  obj->o_setArray(v);
  obj->invokeWakeup();
  return obj;
}

void ImmutableObj::getSizeStats(SharedVariantStats* stats) const {
  stats->initStats();
  stats->dataTotalSize += sizeof(ImmutableObj) + sizeof(Prop) * m_propCount;

  for (int i = 0; i < m_propCount; i++) {
    auto const sd = m_props[i].name;
    if (!sd->isStatic()) {
      stats->dataSize += sd->size();
      stats->dataTotalSize += sizeof(StringData) + sd->size();
    }
    if (m_props[i].val) {
      SharedVariantStats childStats;
      m_props[i].val->getStats(&childStats);
      stats->addChildStats(&childStats);
    }
  }
}

int32_t ImmutableObj::getSpaceUsage() const {
  int32_t size = sizeof(ImmutableObj) + sizeof(Prop) * m_propCount;

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
