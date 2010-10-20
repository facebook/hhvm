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

#include <runtime/base/shared/process_shared_variant.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/base/shared/shared_map.h>
#include <runtime/base/runtime_option.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

ProcessSharedVariant::ProcessSharedVariant(SharedMemoryString& source) {
  m_lock = putPtr((ProcessSharedVariantLock*)NULL);
  m_type = KindOfString;
  m_data.str = putPtr(&source);
}

ProcessSharedVariant::ProcessSharedVariant(CVarRef source,
                                           ProcessSharedVariantLock* lock)
  : m_lock(putPtr(lock)) {
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
  case KindOfStaticString:
  case KindOfString:
    {
      String s = source.toString();
      m_type = KindOfString;
      if (lock) {
        m_data.str = putPtr(SharedMemoryManager::GetSegment()
                            ->construct<SharedMemoryString>
                            (boost::interprocess::anonymous_instance)
                            (s.data(), s.size()));
      } else {
        // Just need this string to live long enough for the key lookup so
        // don't store in shared memory.
        m_data.str = putPtr(new SharedMemoryString(s.data(), s.size()));
      }
      break;
    }
  case KindOfArray:
    {
      ASSERT(lock);
      m_type = KindOfArray;

      ArrayData *arr = source.getArrayData();
      PointerSet seen;
      if (arr->hasInternalReference(seen)) {
        setSerializedArray();
        setShouldCache();
        String s = f_serialize(source);
        m_data.str = putPtr(SharedMemoryManager::GetSegment()
                            ->construct<SharedMemoryString>
                            (boost::interprocess::anonymous_instance)
                            (s.data(), s.size()));
        break;
      }

      uint i = 0;
      ProcessSharedVariantMapData* mapData = SharedMemoryManager::GetSegment()
        ->construct<ProcessSharedVariantMapData>
        (boost::interprocess::anonymous_instance)();
      m_data.map = putPtr(mapData);
      ProcessSharedVariantToIntMap* map = SharedMemoryManager::GetSegment()
        ->construct<ProcessSharedVariantToIntMap>
        (boost::interprocess::anonymous_instance)();
      mapData->map = putPtr(map);
      SharedMemoryVector<SharedVariant*>* keys =
        SharedMemoryManager::GetSegment()
        ->construct<SharedMemoryVector<SharedVariant*> >
        (boost::interprocess::anonymous_instance)();
      mapData->keys = putPtr(keys);
      SharedMemoryVector<SharedVariant*>* vals =
        SharedMemoryManager::GetSegment()
        ->construct<SharedMemoryVector<SharedVariant*> >
        (boost::interprocess::anonymous_instance)();
      mapData->vals = putPtr(vals);
      for (ArrayIterPtr it = source.begin(); !it->end(); it->next()) {
        ProcessSharedVariant* key = SharedMemoryManager::GetSegment()
          ->construct<ProcessSharedVariant>
          (boost::interprocess::anonymous_instance)
          (it->first(), getLock());
        ProcessSharedVariant* val = SharedMemoryManager::GetSegment()
          ->construct<ProcessSharedVariant>
          (boost::interprocess::anonymous_instance)
          (it->second(), getLock());
        if (val->shouldCache()) setShouldCache();
        (*map)[key] = i++;
        keys->push_back(putPtr(key));
        vals->push_back(putPtr(val));
      }
      break;
    }
  default:
    {
      m_type = KindOfObject;
      setShouldCache();
      String s = f_serialize(source);
      m_data.str = putPtr(SharedMemoryManager::GetSegment()
                          ->construct<SharedMemoryString>
                          (boost::interprocess::anonymous_instance)
                          (s.data(), s.size()));
      break;
    }
  }
}

Variant ProcessSharedVariant::toLocal() {
  ASSERT(getLock());
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
      if (getSerializedArray()) {
        SharedMemoryString* s = getString();
        return f_unserialize(String(s->c_str(), s->size(), AttachLiteral));
      }
      return NEW(SharedMap)(this);
    }
  default:
    {
      ASSERT(m_type == KindOfObject);
      SharedMemoryString* s = getString();
      return f_unserialize(String(s->c_str(), s->size(), AttachLiteral));
    }
  }
}

void ProcessSharedVariant::dump(std::string &out) {
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
    if (getSerializedArray()) {
      out += "array: ";
      out += getString()->c_str();
    } else {
      incRef();
      SharedMap(this).dump(out);
    }
    break;
  default:
    out += "object: ";
    out += getString()->c_str();
    break;
  }
  out += "\n";
}

ProcessSharedVariant::~ProcessSharedVariant() {
  switch (m_type) {
  case KindOfString:
  case KindOfObject:
    {
      if (getLock()) {
        SharedMemoryManager::GetSegment()->destroy_ptr(getString());
      }
    }
    break;
  case KindOfArray:
    {
      if (getSerializedArray()) {
        if (getLock()) {
          SharedMemoryManager::GetSegment()->destroy_ptr(getString());
        }
        break;
      }

      ASSERT(getLock());
      BOOST_FOREACH(SharedVariant* v, keys()) {
        getPtr(v)->decRef();
      }
      BOOST_FOREACH(SharedVariant* v, vals()) {
        getPtr(v)->decRef();
      }
      SharedMemoryManager::GetSegment()->destroy_ptr(&map());
      SharedMemoryManager::GetSegment()->destroy_ptr(&keys());
      SharedMemoryManager::GetSegment()->destroy_ptr(&vals());
      SharedMemoryManager::GetSegment()->destroy_ptr(getMapData());
    }
    break;
  default:
    break;
  }
}

void ProcessSharedVariant::loadElems(ArrayData *&elems,
                                     const SharedMap &sharedMap,
                                     bool keepRef /* = false */) {
  ASSERT(is(KindOfArray));
  const SharedMemoryVector<SharedVariant*>& ks = keys();
  uint count = ks.size();
  ArrayInit ai(count, false, keepRef);
  for (uint i = 0; i < count; i++) {
    SharedVariant *k = getPtr(ks[i]);
    ai.set(k->toLocal(), sharedMap.getValue(i), true);
  }
  elems = ai.create();
  if (elems->isStatic()) elems = elems->copy();
}

ProcessSharedVariantToIntMap::const_iterator
ProcessSharedVariant::lookup(CVarRef key) {
  ProcessSharedVariantToIntMap::const_iterator it;
  if (key.isString()) {
    SharedMemoryString ks;
    StringData* sd = key.getStringData();
    ks = SharedMemoryString(sd->data(), sd->size());
    ProcessSharedVariant svk(ks);
    it = map().find(&svk);
  } else {
    ProcessSharedVariant svk(key, NULL);
    it = map().find(&svk);
  }
  return it;
}

void ProcessSharedVariant::incRef() {
  getLock()->lock();
  ++m_ref;
  getLock()->unlock();
}

void ProcessSharedVariant::decRef() {
  ASSERT(m_ref);
  getLock()->lock();
  --m_ref;
  if (m_ref == 0) {
    getLock()->unlock();
    SharedMemoryManager::GetSegment()->destroy_ptr(this);
  } else {
    getLock()->unlock();
  }
}

bool ProcessSharedVariant::operator<(const SharedVariant& svother) const {
  ProcessSharedVariant const * other =
    dynamic_cast<ProcessSharedVariant const *>(&svother);
  ASSERT(other);
  if (m_type != other->m_type) {
    return m_type < other->m_type;
  }
  switch (m_type) {
  case KindOfInt64:  return m_data.num < other->m_data.num;
  case KindOfString: return *getString() < *other->getString();
  default:
    break;
  }
  // No other types are legitimate keys
  ASSERT(false);
  return false;
}

int ProcessSharedVariant::getIndex(CVarRef key) {
  ASSERT(is(KindOfArray));
  ProcessSharedVariantToIntMap::const_iterator it = lookup(key);
  if (it == map().end()) {
    return -1;
  }
  return it->second;
}

Variant ProcessSharedVariant::getKey(ssize_t pos) const {
  return getPtr(keys()[pos])->toLocal();
}

SharedVariant* ProcessSharedVariant::getKeySV(ssize_t pos) const {
  return getPtr(keys()[pos]);
}

SharedVariant* ProcessSharedVariant::getValue(ssize_t pos) const {
  return getPtr(vals()[pos]);
}

///////////////////////////////////////////////////////////////////////////////
}
