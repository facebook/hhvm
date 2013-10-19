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
#include "hphp/runtime/base/apc-variant.h"

#include "folly/ScopeGuard.h"

#include "hphp/runtime/ext/ext_variable.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/apc-object.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

APCVariant::APCVariant(CVarRef source, bool serialized,
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

      auto const st = lookupStaticString(s.get());
      if (st) {
        m_data.str = st;
        m_type = KindOfStaticString;
        break;
      }

      assert(!s->isStatic()); // would've been handled above
      m_data.str = StringData::MakeMalloced(s->data(), s->size());
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
          m_data.str = StringData::MakeMalloced(s.data(), s.size());
          break;
        }
      }

      if (arr->isVectorData()) {
        setPacked();
        size_t num_elems = arr->size();
        m_data.packed = new (num_elems) APCPackedArray(num_elems);
        size_t i = 0;
        for (ArrayIter it(arr); !it.end(); it.next()) {
          APCVariant* val = Create(it.secondRef(), false, true,
                                      unserializeObj);
          if (val->m_shouldCache) m_shouldCache = true;
          m_data.packed->vals()[i++] = val;
        }
        assert(i == num_elems);
      } else {
        m_data.array = APCArray::Create(arr, unserializeObj,
                                              m_shouldCache);
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
      setPacked();
      auto const num_elems = 0;
      m_data.packed = new (num_elems) APCPackedArray(num_elems);
      break;
    }
  default:
    {
      assert(source.isObject());
      m_shouldCache = true;
      if (unserializeObj) {
        // This assumes hasInternalReference(seen, true) is false
        APCObject* obj = new APCObject(source.getObjectData());
        m_data.obj = obj;
        setIsObj();
      } else {
        String s = apc_serialize(source);
        m_data.str = StringData::MakeMalloced(s.data(), s.size());
      }
      break;
    }
  }
  assert(m_type != KindOfResource);
}

// Defined here for inlining into MakeSVSlowPath below.
ALWAYS_INLINE void StringData::enlist() {
  assert(isShared());
  auto& head = MM().m_strings;
  // insert after head
  auto const next = head.next;
  auto& payload = *sharedPayload();
  assert(uintptr_t(next) != kMallocFreeWord);
  payload.node.next = next;
  payload.node.prev = &head;
  next->prev = head.next = &payload.node;
}

HOT_FUNC NEVER_INLINE
StringData* StringData::MakeSVSlowPath(APCVariant* shared, uint32_t len) {
  auto const data       = shared->stringData();
  auto const hash       = shared->rawStringData()->m_hash & STRHASH_MASK;
  auto const capAndHash = static_cast<uint64_t>(hash) << 32;

  auto const sd = static_cast<StringData*>(
    MM().smartMallocSize(sizeof(StringData) + sizeof(SharedPayload))
  );

  sd->m_data        = const_cast<char*>(data);
  sd->m_lenAndCount = len;
  sd->m_capAndHash  = capAndHash;

  sd->sharedPayload()->shared = shared;
  sd->enlist();
  shared->incRef();

  assert(sd->m_len == len);
  assert(sd->m_count == 0);
  assert(sd->m_cap == 0); // cap == 0 means shared
  assert(sd->m_hash == hash);
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::Make(APCVariant* shared) {
  // No need to check if len > MaxSize, because if it were we'd never
  // have made the StringData in the APCVariant without throwing.
  assert(size_t(shared->stringLength()) <= size_t(MaxSize));

  auto const len        = shared->stringLength();
  if (UNLIKELY(len > SmallStringReserve)) {
    return MakeSVSlowPath(shared, len);
  }

  auto const psrc       = shared->stringData();
  auto const hash       = shared->rawStringData()->m_hash & STRHASH_MASK;
  assert(hash != 0);

  auto const needed = static_cast<uint32_t>(sizeof(StringData) + len + 1);
  auto const cap    = MemoryManager::smartSizeClass(needed);
  auto const sd     = static_cast<StringData*>(MM().smartMallocSize(cap));
  auto const pdst   = reinterpret_cast<char*>(sd + 1);

  auto const capAndHash = static_cast<uint64_t>(hash) << 32 |
    (cap - sizeof(StringData));

  sd->m_data        = pdst;
  sd->m_lenAndCount = len;
  sd->m_capAndHash  = capAndHash;

  pdst[len] = 0;
  auto const mcret = memcpy(pdst, psrc, len);
  auto const ret   = reinterpret_cast<StringData*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.

  // Note: this return value thing is doing a dead lea into %rsi in
  // the caller for some reason.

  assert(ret == sd);
  assert(ret->m_len == len);
  assert(ret->m_count == 0);
  assert(ret->m_cap == cap - sizeof(StringData));
  assert(ret->m_hash == hash);
  assert(ret->isFlat());
  assert(ret->checkSane());
  return ret;
}

HOT_FUNC
Variant APCVariant::toLocal() {
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
      return StringData::Make(this);
    }
  case KindOfArray:
    {
      if (getSerializedArray()) {
        return apc_unserialize(m_data.str->data(), m_data.str->size());
      }
      return APCLocalArray::Make(this);
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
      return apc_unserialize(m_data.str->data(), m_data.str->size());
    }
  }
}

void APCVariant::dump(std::string &out) {
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
      auto sm = APCLocalArray::Make(this);
      SCOPE_EXIT { sm->release(); };
      sm->dump(out);
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

APCVariant::~APCVariant() {
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

      if (isPacked()) {
        delete m_data.packed;
      } else {
        APCArray::Destroy(m_data.array);
      }
    }
    break;
  default:
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////

HOT_FUNC
int APCVariant::getIndex(const StringData* key) {
  assert(is(KindOfArray));
  if (isPacked()) return -1;
  return m_data.array->indexOf(key);
}

int APCVariant::getIndex(int64_t key) {
  assert(is(KindOfArray));
  if (isPacked()) {
    if (key < 0 || (size_t) key >= m_data.packed->size()) return -1;
    return key;
  }
  return m_data.array->indexOf(key);
}

Variant APCVariant::getKey(ssize_t pos) const {
  assert(is(KindOfArray));
  if (isPacked()) {
    assert(pos < (ssize_t) m_data.packed->size());
    return pos;
  }
  return m_data.array->getKeyIndex(pos)->toLocal();
}

HOT_FUNC
APCVariant* APCVariant::getValue(ssize_t pos) const {
  assert(is(KindOfArray));
  if (isPacked()) {
    assert(pos < (ssize_t) m_data.packed->size());
    return m_data.packed->vals()[pos];
  }
  return m_data.array->getValIndex(pos);
}

ArrayData* APCVariant::loadElems(const APCLocalArray &array) {
  assert(is(KindOfArray));
  auto count = arrSize();
  ArrayData* elems;
  if (isPacked()) {
    PackedArrayInit ai(count);
    for (uint i = 0; i < count; i++) {
      ai.append(array.getValueRef(i));
    }
    elems = ai.create();
  } else {
    ArrayInit ai(count);
    for (uint i = 0; i < count; i++) {
      ai.add(m_data.array->getKeyIndex(i)->toLocal(), array.getValueRef(i),
             true);
    }
    elems = ai.create();
  }
  if (elems->isStatic()) elems = elems->copy();
  return elems;
}

int APCVariant::countReachable() const {
  int count = 1;
  if (getType() == KindOfArray) {
    int size = arrSize();
    if (!isPacked()) {
      count += size; // for keys
    }
    for (int i = 0; i < size; i++) {
      APCVariant* p = getValue(i);
      count += p->countReachable(); // for values
    }
  }
  return count;
}

APCVariant *APCVariant::Create
(CVarRef source, bool serialized, bool inner /* = false */,
 bool unserializeObj /* = false*/) {
  APCVariant *wrapped = source.getSharedVariant();
  if (wrapped && !unserializeObj) {
    wrapped->incRef();
    // static cast should be enough
    return (APCVariant *)wrapped;
  }
  return new APCVariant(source, serialized, inner, unserializeObj);
}

APCVariant* APCVariant::convertObj(CVarRef var) {
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
  APCVariant *tmp = new APCVariant(var, false, true, true);
  tmp->setObjAttempted();
  return tmp;
}

int32_t APCVariant::getSpaceUsage() const {
  int32_t size = sizeof(APCVariant);
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
    } else if (isPacked()) {
      auto size = m_data.packed->size();
      size += sizeof(APCPackedArray) + size * sizeof(APCVariant*);
      for (size_t i = 0, n = m_data.packed->size(); i < n; i++) {
        size += m_data.packed->vals()[i]->getSpaceUsage();
      }
    } else {
      auto array = m_data.array;
      size += array->getStructSize();
      for (size_t i = 0, n = array->size(); i < n; i++) {
        size += array->getKeyIndex(i)->getSpaceUsage();
        size += array->getValIndex(i)->getSpaceUsage();
      }
    }
    break;
  }
  return size;
}


void APCVariant::getStats(APCVariantStats *stats) const {
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
    stats->dataTotalSize = sizeof(APCVariant);
    break;
  case KindOfObject:
    if (getIsObj()) {
      APCVariantStats childStats;
      m_data.obj->getSizeStats(&childStats);
      stats->addChildStats(&childStats);
      break;
    }
    // fall through
  case KindOfString:
    stats->dataSize = m_data.str->size();
    stats->dataTotalSize = sizeof(APCVariant) + sizeof(StringData) +
                           stats->dataSize;
    break;
  default:
    assert(is(KindOfArray));
    if (getSerializedArray()) {
      stats->dataSize = m_data.str->size();
      stats->dataTotalSize = sizeof(APCVariant) + sizeof(StringData) +
                             stats->dataSize;
      break;
    }
    if (isPacked()) {
      stats->dataTotalSize = sizeof(APCVariant) +
                             sizeof(APCPackedArray);
      auto size = m_data.packed->size();
      stats->dataTotalSize += sizeof(APCVariant*) * size;
      for (size_t i = 0; i < size; i++) {
        APCVariant *v = m_data.packed->vals()[i];
        APCVariantStats childStats;
        v->getStats(&childStats);
        stats->addChildStats(&childStats);
      }
    } else {
      auto array = m_data.array;
      stats->dataTotalSize = sizeof(APCVariant) + array->getStructSize();
      for (size_t i = 0, n = array->size(); i < n; i++) {
        APCVariantStats childStats;
        array->getKeyIndex(i)->getStats(&childStats);
        stats->addChildStats(&childStats);
        array->getValIndex(i)->getStats(&childStats);
        stats->addChildStats(&childStats);
      }
    }
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
