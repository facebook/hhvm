/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/shared_variant.h"
#include "hphp/runtime/ext/ext_variable.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/runtime/base/shared_map.h"
#include "hphp/runtime/base/runtime_option.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SharedVariant::SharedVariant(CVarRef source, bool serialized,
                             bool inner /* = false */,
                             bool unserializeObj /* = false */)
  : m_shouldCache(false), m_flags(0) {
  assert(!serialized || source.isString());
  m_count = 1;
  m_type = source.getType();
  switch (m_type) {
  case KindOfBoolean:
    {
      m_data.num = source.toBoolean();
      break;
    }
  case KindOfInt64:
    {
      m_type = KindOfInt64;
      m_data.num = source.toInt64();
      break;
    }
  case KindOfDouble:
    {
      m_data.dbl = source.toDouble();
      break;
    }
  case KindOfStaticString:
    {
      if (serialized) goto StringCase;
      m_data.str = source.getStringData();
      break;
    }
StringCase:
  case KindOfString:
    {
      String s = source.toString();
      if (serialized) {
        m_type = KindOfObject;
        // It is priming, and there might not be the right class definitions
        // for unserialization.
        s = apc_reserialize(s);
      }
      StringData* st = StringData::LookupStaticString(s.get());
      if (st) {
        m_data.str = st;
        m_type = KindOfStaticString;
        break;
      }
      m_data.str = s->copy(true);
      break;
    }
  case KindOfArray:
    {
      ArrayData *arr = source.getArrayData();

      if (!inner) {
        // only need to call hasInternalReference() on the toplevel array
        PointerSet seen;
        if (arr->hasInternalReference(seen)) {
          setSerializedArray();
          m_shouldCache = true;
          String s = apc_serialize(source);
          m_data.str = new StringData(s.data(), s.size(), CopyMalloc);
          break;
        }
      }

      if (arr->isVectorData()) {
        setIsVector();
        m_data.vec = new (arr->size()) VectorData();
        for (ArrayIter it(arr); !it.end(); it.next()) {
          SharedVariant* val = Create(it.secondRef(), false, true,
                                      unserializeObj);
          if (val->m_shouldCache) m_shouldCache = true;
          m_data.vec->vals()[m_data.vec->m_size++] = val;
        }
      } else {
        m_data.map = ImmutableMap::Create(arr, unserializeObj, m_shouldCache);
      }
      break;
    }
  case KindOfUninit:
  case KindOfNull:
    {
      break;
    }
  case KindOfResource:
    {
      // TODO Task #2661075: Here and elsewhere in the runtime, we convert
      // Resources to the empty array during various serialization operations,
      // which does not match Zend behavior. We should fix this.
      m_type = KindOfArray;
      setIsVector();
      m_data.vec = new (0) VectorData();
      break;
    }
  default:
    {
      assert(source.isObject());
      m_shouldCache = true;
      if (unserializeObj) {
        // This assumes hasInternalReference(seen, true) is false
        ImmutableObj* obj = new ImmutableObj(source.getObjectData());
        m_data.obj = obj;
        setIsObj();
      } else {
        String s = apc_serialize(source);
        m_data.str = new StringData(s.data(), s.size(), CopyMalloc);
      }
      break;
    }
  }
  assert(m_type != KindOfResource);
}

HOT_FUNC
Variant SharedVariant::toLocal() {
  switch (m_type) {
  case KindOfBoolean:
    {
      return (bool)m_data.num;
    }
  case KindOfInt64:
    {
      return m_data.num;
    }
  case KindOfDouble:
    {
      return m_data.dbl;
    }
  case KindOfStaticString:
    {
      return m_data.str;
    }
  case KindOfString:
    {
      return NEW(StringData)(this);
    }
  case KindOfArray:
    {
      if (getSerializedArray()) {
        return apc_unserialize(String(m_data.str->data(), m_data.str->size(),
                                      CopyString));
      }
      return NEW(SharedMap)(this);
    }
  case KindOfUninit:
  case KindOfNull:
    {
      return null_variant;
    }
  default:
    {
      assert(m_type == KindOfObject);
      if (getIsObj()) {
        return m_data.obj->getObject();
      }
      return apc_unserialize(String(m_data.str->data(), m_data.str->size(),
                                    CopyString));
    }
  }
}

void SharedVariant::dump(std::string &out) {
  out += "ref(";
  out += boost::lexical_cast<string>(m_count);
  out += ") ";
  switch (m_type) {
  case KindOfBoolean:
    out += "boolean: ";
    out += m_data.num ? "true" : "false";
    break;
  case KindOfInt64:
    out += "int: ";
    out += boost::lexical_cast<string>(m_data.num);
    break;
  case KindOfDouble:
    out += "double: ";
    out += boost::lexical_cast<string>(m_data.dbl);
    break;
  case KindOfStaticString:
  case KindOfString:
    out += "string(";
    out += boost::lexical_cast<string>(stringLength());
    out += "): ";
    out += stringData();
    break;
  case KindOfArray:
    if (getSerializedArray()) {
      out += "array: ";
      out += m_data.str->data();
    } else {
      SharedMap(this).dump(out);
    }
    break;
  case KindOfUninit:
  case KindOfNull:
    out += "null";
    break;
  default:
    out += "object: ";
    out += m_data.str->data();
    break;
  }
  out += "\n";
}

SharedVariant::~SharedVariant() {
  switch (m_type) {
  case KindOfObject:
    if (getIsObj()) {
      delete m_data.obj;
      break;
    }
    // otherwise fall through
  case KindOfString:
    m_data.str->destruct();
    break;
  case KindOfArray:
    {
      if (getSerializedArray()) {
        m_data.str->destruct();
        break;
      }

      if (getIsVector()) {
        delete m_data.vec;
      } else {
        ImmutableMap::Destroy(m_data.map);
      }
    }
    break;
  default:
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////

HOT_FUNC
int SharedVariant::getIndex(const StringData* key) {
  assert(is(KindOfArray));
  if (getIsVector()) return -1;
  return m_data.map->indexOf(key);
}

int SharedVariant::getIndex(int64_t key) {
  assert(is(KindOfArray));
  if (getIsVector()) {
    if (key < 0 || (size_t) key >= m_data.vec->m_size) return -1;
    return key;
  }
  return m_data.map->indexOf(key);
}

Variant SharedVariant::getKey(ssize_t pos) const {
  assert(is(KindOfArray));
  if (getIsVector()) {
    assert(pos < (ssize_t) m_data.vec->m_size);
    return pos;
  }
  return m_data.map->getKeyIndex(pos)->toLocal();
}

HOT_FUNC
SharedVariant* SharedVariant::getValue(ssize_t pos) const {
  assert(is(KindOfArray));
  if (getIsVector()) {
    assert(pos < (ssize_t) m_data.vec->m_size);
    return m_data.vec->vals()[pos];
  }
  return m_data.map->getValIndex(pos);
}

ArrayData* SharedVariant::loadElems(const SharedMap &sharedMap,
                                    bool mapInit /* = false */) {
  assert(is(KindOfArray));
  uint count = arrSize();
  bool isVector = getIsVector();

  auto ai =
    mapInit ? ArrayInit(count, ArrayInit::mapInit) :
    isVector ? ArrayInit(count, ArrayInit::vectorInit) :
    ArrayInit(count);

  if (isVector) {
    for (uint i = 0; i < count; i++) {
      ai.set(sharedMap.getValueRef(i));
    }
  } else {
    for (uint i = 0; i < count; i++) {
      ai.add(m_data.map->getKeyIndex(i)->toLocal(), sharedMap.getValueRef(i),
             true);
    }
  }
  ArrayData* elems = ai.create();
  if (elems->isStatic()) elems = elems->copy();
  return elems;
}

int SharedVariant::countReachable() const {
  int count = 1;
  if (getType() == KindOfArray) {
    int size = arrSize();
    if (!getIsVector()) {
      count += size; // for keys
    }
    for (int i = 0; i < size; i++) {
      SharedVariant* p = getValue(i);
      count += p->countReachable(); // for values
    }
  }
  return count;
}

SharedVariant *SharedVariant::Create
(CVarRef source, bool serialized, bool inner /* = false */,
 bool unserializeObj /* = false*/) {
  SharedVariant *wrapped = source.getSharedVariant();
  if (wrapped && !unserializeObj) {
    wrapped->incRef();
    // static cast should be enough
    return (SharedVariant *)wrapped;
  }
  return new SharedVariant(source, serialized, inner, unserializeObj);
}

SharedVariant* SharedVariant::convertObj(CVarRef var) {
  if (!var.is(KindOfObject) || getObjAttempted()) {
    return nullptr;
  }
  setObjAttempted();
  ObjectData *obj = var.getObjectData();
  if (obj->instanceof(SystemLib::s_SerializableClass)) {
    // should also check the object itself
    return nullptr;
  }
  PointerSet seen;
  if (obj->hasInternalReference(seen, true)) {
    return nullptr;
  }
  SharedVariant *tmp = new SharedVariant(var, false, true, true);
  tmp->setObjAttempted();
  return tmp;
}

int32_t SharedVariant::getSpaceUsage() const {
  int32_t size = sizeof(SharedVariant);
  if (!IS_REFCOUNTED_TYPE(m_type)) return size;
  switch (m_type) {
  case KindOfObject:
    if (getIsObj()) {
      return size + m_data.obj->getSpaceUsage();
    }
    // fall through
  case KindOfString:
    size += sizeof(StringData) + m_data.str->size();
    break;
  default:
    assert(is(KindOfArray));
    if (getSerializedArray()) {
      size += sizeof(StringData) + m_data.str->size();
    } else if (getIsVector()) {
      size += sizeof(VectorData) +
              sizeof(SharedVariant*) * m_data.vec->m_size;
      for (size_t i = 0; i < m_data.vec->m_size; i++) {
        size += m_data.vec->vals()[i]->getSpaceUsage();
      }
    } else {
      ImmutableMap *map = m_data.map;
      size += map->getStructSize();
      for (int i = 0; i < map->size(); i++) {
        size += map->getKeyIndex(i)->getSpaceUsage();
        size += map->getValIndex(i)->getSpaceUsage();
      }
    }
    break;
  }
  return size;
}


void SharedVariant::getStats(SharedVariantStats *stats) const {
  stats->initStats();
  stats->variantCount = 1;
  switch (m_type) {
  case KindOfUninit:
  case KindOfNull:
  case KindOfBoolean:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfStaticString:
    stats->dataSize = sizeof(m_data.dbl);
    stats->dataTotalSize = sizeof(SharedVariant);
    break;
  case KindOfObject:
    if (getIsObj()) {
      SharedVariantStats childStats;
      m_data.obj->getSizeStats(&childStats);
      stats->addChildStats(&childStats);
      break;
    }
    // fall through
  case KindOfString:
    stats->dataSize = m_data.str->size();
    stats->dataTotalSize = sizeof(SharedVariant) + sizeof(StringData) +
                           stats->dataSize;
    break;
  default:
    assert(is(KindOfArray));
    if (getSerializedArray()) {
      stats->dataSize = m_data.str->size();
      stats->dataTotalSize = sizeof(SharedVariant) + sizeof(StringData) +
                             stats->dataSize;
      break;
    }
    if (getIsVector()) {
      stats->dataTotalSize = sizeof(SharedVariant) + sizeof(VectorData);
      stats->dataTotalSize += sizeof(SharedVariant*) * m_data.vec->m_size;
      for (size_t i = 0; i < m_data.vec->m_size; i++) {
        SharedVariant *v = m_data.vec->vals()[i];
        SharedVariantStats childStats;
        v->getStats(&childStats);
        stats->addChildStats(&childStats);
      }
    } else {
      ImmutableMap *map = m_data.map;
      stats->dataTotalSize = sizeof(SharedVariant) + map->getStructSize();
      for (int i = 0; i < map->size(); i++) {
        SharedVariantStats childStats;
        map->getKeyIndex(i)->getStats(&childStats);
        stats->addChildStats(&childStats);
        map->getValIndex(i)->getStats(&childStats);
        stats->addChildStats(&childStats);
      }
    }
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
