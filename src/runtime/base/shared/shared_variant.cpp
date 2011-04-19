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
#include <runtime/ext/ext_variable.h>
#include <runtime/ext/ext_apc.h>
#include <runtime/base/shared/shared_map.h>
#include <runtime/base/runtime_option.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SharedVariant::SharedVariant(CVarRef source, bool serialized,
                             bool inner /* = false */,
                             bool unserializeObj /* = false */)
  : m_count (1), m_shouldCache(false), m_flags(0){
  ASSERT(!serialized || source.isString());

  m_type = source.getType();

  switch (m_type) {
  case KindOfBoolean:
    {
      m_data.num = source.toBoolean();
      break;
    }
  case KindOfInt32:
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
  case KindOfString:
    {
      String s = source.toString();
      if (serialized) {
        m_type = KindOfObject;
        // It is priming, and there might not be the right class definitions
        // for unserialization.
        s = apc_reserialize(s);
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
          m_data.str = new StringData(s.data(), s.size(), CopyString);
          break;
        }
      }

      size_t size = arr->size();
      if (arr->isVectorData()) {
        setIsVector();
        m_data.vec = new VectorData(size);
        uint i = 0;
        for (ArrayIter it(arr); !it.end(); it.next(), i++) {
          SharedVariant* val = Create(it.secondRef(), false, true,
                                      unserializeObj);
          if (val->m_shouldCache) m_shouldCache = true;
          m_data.vec->vals[i] = val;
        }
      } else {
        m_data.map = new ImmutableMap(size);
        for (ArrayIter it(arr); !it.end(); it.next()) {
          SharedVariant* key = Create(it.first(), false, true,
                                      unserializeObj);
          SharedVariant* val = Create(it.secondRef(), false, true,
                                      unserializeObj);
          if (val->m_shouldCache) m_shouldCache = true;
          m_data.map->add(key, val);
        }
      }
      break;
    }
  case KindOfUninit:
  case KindOfNull:
    {
      break;
    }
  default:
    {
      ASSERT(source.isObject());
      m_shouldCache = true;
      if (unserializeObj) {
        // This assumes hasInternalReference(seen, true) is false
        ImmutableObj* obj = new ImmutableObj(source.getObjectData());
        m_data.obj = obj;
        setIsObj();
      } else {
        String s = apc_serialize(source);
        m_data.str = new StringData(s.data(), s.size(), CopyString);
      }
      break;
    }
  }
}

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
                                      AttachLiteral));
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
      ASSERT(m_type == KindOfObject);
      if (getIsObj()) {
        return m_data.obj->getObject();
      }
      return apc_unserialize(String(m_data.str->data(), m_data.str->size(),
                                    AttachLiteral));
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
        delete m_data.map;
      }
    }
    break;
  default:
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////

size_t SharedVariant::arrSize() const {
  ASSERT(is(KindOfArray));
  if (getIsVector()) return m_data.vec->size;
  return m_data.map->size();
}

int SharedVariant::getIndex(CVarRef key) {
  ASSERT(is(KindOfArray));
  switch (key.getType()) {
  case KindOfInt32:
  case KindOfInt64: {
    int64 num = key.getNumData();
    if (getIsVector()) {
      if (num < 0 || (size_t) num >= m_data.vec->size) return -1;
      return num;
    }
    return m_data.map->indexOf(num);
  }
  case KindOfStaticString:
  case KindOfString: {
    if (getIsVector()) return -1;
    StringData *sd = key.getStringData();
    return m_data.map->indexOf(sd);
  }
  default:
    // No other types are legitimate keys
    break;
  }
  return -1;
}

int SharedVariant::getIndex(CStrRef key) {
  ASSERT(is(KindOfArray));
  if (getIsVector()) return -1;
  StringData *sd = key.get();
  return m_data.map->indexOf(sd);
}

int SharedVariant::getIndex(litstr key) {
  ASSERT(is(KindOfArray));
  if (getIsVector()) return -1;
  StringData sd(key);
  return m_data.map->indexOf(&sd);
}

int SharedVariant::getIndex(int64 key) {
  ASSERT(is(KindOfArray));
  if (getIsVector()) {
    if (key < 0 || (size_t) key >= m_data.vec->size) return -1;
    return key;
  }
  return m_data.map->indexOf(key);
}

Variant SharedVariant::getKey(ssize_t pos) const {
  ASSERT(is(KindOfArray));
  if (getIsVector()) {
    ASSERT(pos < (ssize_t) m_data.vec->size);
    return pos;
  }
  return m_data.map->getKeyIndex(pos)->toLocal();
}

SharedVariant* SharedVariant::getValue(ssize_t pos) const {
  ASSERT(is(KindOfArray));
  if (getIsVector()) {
    ASSERT(pos < (ssize_t) m_data.vec->size);
    return m_data.vec->vals[pos];
  }
  return m_data.map->getValIndex(pos);
}

void SharedVariant::loadElems(ArrayData *&elems,
                              const SharedMap &sharedMap,
                              bool keepRef /* = false */) {
  ASSERT(is(KindOfArray));
  uint count = arrSize();
  bool isVector = getIsVector();
  ArrayInit ai(count, isVector, keepRef);
  for (uint i = 0; i < count; i++) {
    if (isVector) {
      ai.set(sharedMap.getValueRef(i));
    } else {
      ai.add(m_data.map->getKeyIndex(i)->toLocal(), sharedMap.getValueRef(i),
             true);
    }
  }
  elems = ai.create();
  if (elems->isStatic()) elems = elems->copy();
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
    return NULL;
  }
  setObjAttempted();
  PointerSet seen;
  ObjectData *obj = var.getObjectData();
  if (obj->o_instanceof("Serializable")) {
    // should also check the object itself
    return NULL;
  }
  CArrRef arr = obj->o_toArray();
  if (arr->hasInternalReference(seen, true)) {
    return NULL;
  }
  SharedVariant *tmp = new SharedVariant(var, false, true, true);
  tmp->setObjAttempted();
  return tmp;
}

int32 SharedVariant::getSpaceUsage() {
  int32 size = sizeof(SharedVariant);
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
    ASSERT(is(KindOfArray));
    if (getSerializedArray()) {
      size += sizeof(StringData) + m_data.str->size();
    } else if (getIsVector()) {
      size += sizeof(VectorData) +
              sizeof(SharedVariant*) * m_data.vec->size;
      for (size_t i = 0; i < m_data.vec->size; i++) {
        size += m_data.vec->vals[i]->getSpaceUsage();
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


void SharedVariant::getStats(SharedVariantStats *stats) {
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
    ASSERT(is(KindOfArray));
    if (getSerializedArray()) {
      stats->dataSize = m_data.str->size();
      stats->dataTotalSize = sizeof(SharedVariant) + sizeof(StringData) +
                             stats->dataSize;
      break;
    }
    if (getIsVector()) {
      stats->dataTotalSize = sizeof(SharedVariant) + sizeof(VectorData);
      stats->dataTotalSize += sizeof(SharedVariant*) * m_data.vec->size;
      for (size_t i = 0; i < m_data.vec->size; i++) {
        SharedVariant *v = m_data.vec->vals[i];
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
