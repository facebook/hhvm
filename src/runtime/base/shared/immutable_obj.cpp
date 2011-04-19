/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/shared/shared_variant.h>
#include <runtime/base/shared/immutable_obj.h>
#include <runtime/base/externals.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/array_iterator.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ImmutableObj::ImmutableObj(ObjectData *obj) {
  // This function assumes the object and object/array down the tree have no
  // internal refernece and does not implements serializable interface.
  ASSERT(!obj->o_instanceof("Serializable"));
  m_cls = obj->o_getClassName()->copy(true);
  Array props;
  obj->o_getArray(props);
  if (props.empty()) {
    m_props = NULL;
    m_propCount = 0;
  }
  m_props = (Prop*)malloc(sizeof(Prop) * props.size());
  m_propCount = 0;
  for (ArrayIter it(props); !it.end(); it.next()) {
    ASSERT(m_propCount < props.size());
    Variant key(it.first());
    ASSERT(key.isString());
    CVarRef value = it.secondRef();
    SharedVariant *val = NULL;
    if (!value.isNull()) {
      val = new SharedVariant(value, false, true, true);
    }
    m_props[m_propCount].val = val;
    m_props[m_propCount].name = key.getStringData()->copy(true);
    m_propCount++;
  }
}

Object ImmutableObj::getObject() {
  Object obj;
  try {
    obj = create_object(m_cls->data(), Array::Create(), false);
  } catch (ClassNotFoundException &e) {
    ASSERT(false);
  }
  ArrayInit ai(m_propCount);
  for (int i = 0; i < m_propCount; i++) {
    ai.add(String(m_props[i].name->copy()),
           m_props[i].val ? m_props[i].val->toLocal() : null_variant,
           true);
  }
  Array v = ai.create();
  obj->o_setArray(v);
  obj->t___wakeup();
  return obj;
}

ImmutableObj::~ImmutableObj() {
  for (int i = 0; i < m_propCount; i++) {
    m_props[i].name->destruct();
    if (m_props[i].val) m_props[i].val->decRef();
  }
  if (m_props) {
    free(m_props);
  }
  m_cls->destruct();
}

void ImmutableObj::getSizeStats(SharedVariantStats *stats) {
  stats->initStats();
  if (m_cls->isStatic()) {
    stats->dataTotalSize += sizeof(ImmutableObj) + sizeof(Prop) * m_propCount;
  } else {
    stats->dataSize += m_cls->size();
    stats->dataTotalSize += sizeof(ImmutableObj) + sizeof(Prop) * m_propCount +
                            sizeof(StringData) + m_cls->size();
  }

  for (int i = 0; i < m_propCount; i++) {
    StringData *sd = m_props[i].name;
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

int32 ImmutableObj::getSpaceUsage() {
  int32 size = sizeof(ImmutableObj) + sizeof(Prop) * m_propCount;
  if (!m_cls->isStatic()) {
    size += sizeof(StringData) + m_cls->size();
  }

  for (int i = 0; i < m_propCount; i++) {
    StringData *sd = m_props[i].name;
    if (!sd->isStatic()) {
      size += sizeof(StringData) + sd->size();
    }
    if (m_props[i].val) {
      size += m_props[i].val->getSpaceUsage();
    }
  }
  return size;
}

///////////////////////////////////////////////////////////////////////////////
}
