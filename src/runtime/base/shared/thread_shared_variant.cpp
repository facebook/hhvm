/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/shared/thread_shared_variant.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/base/shared/shared_map.h>
#include <runtime/base/runtime_option.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

ThreadSharedVariant::ThreadSharedVariant(StringData *source) : m_owner(false) {
  m_type = KindOfString;
  m_data.str = source;
}

ThreadSharedVariant::ThreadSharedVariant(int64 num) : m_owner(false) {
  m_type = KindOfInt64;
  m_data.num = num;
}

ThreadSharedVariant::ThreadSharedVariant(CVarRef source, bool serialized)
  : m_owner(true) {
  ASSERT(!serialized || source.isString());

  m_ref = 1;

  switch (source.getType()) {
  case KindOfBoolean:
    {
      m_type = KindOfBoolean;
      m_data.num = source.toBoolean();
      break;
    }
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    {
      m_type = KindOfInt64;
      m_data.num = source.toInt64();
      break;
    }
  case KindOfDouble:
    {
      m_type = KindOfDouble;
      m_data.dbl = source.toDouble();
      break;
    }
  case LiteralString:
  case KindOfString:
    {
      String s = source.toString();
      m_type = serialized ? KindOfObject : KindOfString;
      m_data.str = s->copy(true);
      break;
    }
  case KindOfArray:
    {
      m_type = KindOfArray;
      size_t size = source.getArrayData()->size();
      ThreadSharedVariantMapData* mapData = new ThreadSharedVariantMapData();
      ThreadSharedVariantToIntMap map;
      SharedVariant** keys = new SharedVariant*[size];
      SharedVariant** vals = new SharedVariant*[size];

      uint i = 0;
      for (ArrayIterPtr it = source.begin(); !it->end(); it->next()) {
        ThreadSharedVariant* key
          = createAnother(it->first(), false);
        ThreadSharedVariant* val
          = createAnother(it->second(), false);
        keys[i] = key;
        vals[i] = val;
        map[key] = i++;
      }
      m_data.map = mapData;
      mapData->map.swap(map);
      mapData->keys = keys;
      mapData->vals = vals;
      break;
    }
  default:
    {
      m_type = KindOfObject;
      String s = f_serialize(source);
      m_data.str = new StringData(s.data(), s.size(), CopyString);
      break;
    }
  }
}

Variant ThreadSharedVariant::toLocal() {
  ASSERT(m_owner);
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
  case KindOfString:
    {
      return NEW(StringData)(this);
    }
  case KindOfArray:
    {
      return NEW(SharedMap)(this);
    }
  default:
    {
      return f_unserialize(String(m_data.str->data(), m_data.str->size(),
                                  AttachLiteral));
    }
  }
}

void ThreadSharedVariant::dump(std::string &out) {
  out += "ref(";
  out += boost::lexical_cast<string>(m_ref);
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
  case KindOfString:
    out += "string(";
    out += boost::lexical_cast<string>(stringLength());
    out += "): ";
    out += stringData();
    break;
  case KindOfArray:
    SharedMap(this).dump(out);
    break;
  default:
    out += "object: ";
    out += m_data.str->data();
    break;
  }
  out += "\n";
}

ThreadSharedVariant::~ThreadSharedVariant() {
  switch (m_type) {
  case KindOfString:
  case KindOfObject:
    if (m_owner) {
      delete m_data.str;
    }
    break;
  case KindOfArray:
    {
      ASSERT(m_owner);
      ThreadSharedVariantMapData* map = m_data.map;
      size_t size = map->map.size();
      for (size_t i = 0; i < size; i++) {
        map->keys[i]->decRef();
        map->vals[i]->decRef();
      }
      delete [] map->keys;
      delete [] map->vals;
      delete map;
    }
    break;
  default:
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////

bool ThreadSharedVariant::operator==(const SharedVariant& svother) const {
  ThreadSharedVariant const *other =
    dynamic_cast<ThreadSharedVariant const *>(&svother);
  ASSERT(other);
  if (m_type != other->m_type) {
    return false;
  }
  switch (m_type) {
  case KindOfInt64:
    return m_data.num == other->m_data.num;
  case KindOfString:
    return m_data.str->compare(other->m_data.str) == 0;
  default:
    break;
  }
  // No other types are legitimate keys
  ASSERT(false);
  return false;
}

ssize_t ThreadSharedVariant::hash() const {
  switch (m_type) {
  case KindOfInt64:
    return hash_int64(m_data.num);
  case KindOfString:
    return hash_string(m_data.str->data(), m_data.str->size());
  default:
    break;
  }
  // No other types are legitimate keys
  ASSERT(false);
  return -1;
}

const char *ThreadSharedVariant::stringData() const {
  ASSERT(is(KindOfString));
  return m_data.str->data();
}

size_t ThreadSharedVariant::stringLength() const {
  ASSERT(is(KindOfString));
  return m_data.str->size();
}

const ThreadSharedVariantToIntMap &ThreadSharedVariant::map() const {
  return m_data.map->map;
}
SharedVariant** ThreadSharedVariant::keys() const {
  return m_data.map->keys;
}
SharedVariant** ThreadSharedVariant::vals() const {
  return m_data.map->vals;
}

size_t ThreadSharedVariant::arrSize() const {
  return map().size();
}

int ThreadSharedVariant::getIndex(CVarRef key) {
  ASSERT(is(KindOfArray));
  ThreadSharedVariantToIntMap::const_iterator it = lookup(key);
  if (it == map().end()) {
    return -1;
  }
  return it->second;
}

SharedVariant* ThreadSharedVariant::get(CVarRef key) {
  int idx = getIndex(key);
  if (idx != -1) {
    return vals()[idx];
  }
  return NULL;
}

bool ThreadSharedVariant::exists(CVarRef key) {
  ASSERT(is(KindOfArray));
  ThreadSharedVariantToIntMap::const_iterator it = lookup(key);
  return it != map().end();
}

void ThreadSharedVariant::loadElems(ArrayData *&elems) {
  ASSERT(is(KindOfArray));
  SharedVariant** ks = keys();
  SharedVariant** vs = vals();
  uint count = map().size();
  if (count == 0 && RuntimeOption::UseZendArray) {
    elems = StaticEmptyZendArray::Get()->copy();
    return;
  }
  ArrayInit ai(count);
  for (uint i = 0; i < count; i++) {
    ai.set(i, ks[i]->toLocal(), vs[i]->toLocal());
  }
  elems = ai.create();
}

ThreadSharedVariantToIntMap::const_iterator
ThreadSharedVariant::lookup(CVarRef key) {
  ThreadSharedVariantToIntMap::const_iterator it;
  switch (key.getType()) {
  case KindOfString: {
    ThreadSharedVariant svk(key.getStringData());
    it = map().find(&svk);
    break;
  }
  case LiteralString: {
    StringData sd(key.getLiteralString(), AttachLiteral);
    ThreadSharedVariant svk(&sd);
    it = map().find(&svk);
    break;
  }
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64: {
    int64 num = key.getNumData();
    ThreadSharedVariant svk(num);
    it = map().find(&svk);
    break;
  }
  default:
    // No other types are legitimate keys
    it = map().end();
    break;
  }
  return it;
}

size_t ThreadSharedVariantHash::operator()(ThreadSharedVariant* v) const {
  return v->hash();
}

ThreadSharedVariant *ThreadSharedVariant::createAnother
(CVarRef source, bool serialized) {
  return new ThreadSharedVariant(source, serialized);
}

ThreadSharedVariant *ThreadSharedVariantLockedRefs::createAnother
(CVarRef source, bool serialized) {
  return new ThreadSharedVariantLockedRefs(source, serialized, m_lock);
}

///////////////////////////////////////////////////////////////////////////////
}
