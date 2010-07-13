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

#ifndef __HPHP_THREAD_SHARED_VARIANT_H__
#define __HPHP_THREAD_SHARED_VARIANT_H__

#include <runtime/base/types.h>
#include <util/lock.h>
#include <util/hash.h>
#include <util/atomic.h>
#include <runtime/base/shared/shared_variant.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ThreadSharedVariant;

typedef hphp_hash_map<int64, int, int64_hash> Int64ToIntMap;
typedef hphp_hash_map<StringData *, int, string_data_hash, string_data_equal>
        StringDataToIntMap;

///////////////////////////////////////////////////////////////////////////////

class ThreadSharedVariant : public SharedVariant {
public:
  ThreadSharedVariant(CVarRef source, bool serialized, bool inner = false);
  virtual ~ThreadSharedVariant();

  virtual void incRef() {
    atomic_inc(m_ref);
  }

  virtual void decRef() {
    ASSERT(m_ref);
    if (atomic_dec(m_ref) == 0) {
      delete this;
    }
  }

  Variant toLocal();

  virtual int64 intData() const {
    ASSERT(is(KindOfInt64));
    return m_data.num;
  }

  const char* stringData() const;
  size_t stringLength() const;

  size_t arrSize() const;
  int getIndex(CVarRef key);
  SharedVariant* get(CVarRef key);
  bool exists(CVarRef key);

  void loadElems(ArrayData *&elems, const SharedMap &sharedMap,
                 bool keepRef = false);

  virtual Variant getKey(ssize_t pos) const {
    ASSERT(is(KindOfArray));
    if (m_data.map->isVector) return pos;
    else return m_data.map->keys[pos]->toLocal();
  }
  virtual SharedVariant* getValue(ssize_t pos) const {
    ASSERT(is(KindOfArray));
    return m_data.map->vals[pos];
  }

  // implementing LeakDetectable
  virtual void dump(std::string &out);

protected:
  virtual ThreadSharedVariant *createAnother(CVarRef source, bool serialized,
                                             bool inner = false);

  virtual SharedVariant* getKeySV(ssize_t pos) const {
    ASSERT(is(KindOfArray));
    if (m_data.map->isVector) return NULL;
    else return m_data.map->keys[pos];
  }

private:
  class MapData {
  public:
    size_t size;
    bool isVector;
    Int64ToIntMap *intMap;
    StringDataToIntMap *strMap;
    ThreadSharedVariant **keys;
    ThreadSharedVariant **vals;

    MapData(size_t s, bool vector = false) : size(s), isVector(vector),
      intMap(NULL), strMap(NULL) {
      if (!vector) keys = new ThreadSharedVariant *[s];
      vals = new ThreadSharedVariant *[s];
    }

    ~MapData() {
      for (size_t i = 0; i < size; i++) {
        if (!isVector) keys[i]->decRef();
        vals[i]->decRef();
      }
      if (intMap) delete intMap;
      if (strMap) delete strMap;
      if (!isVector) delete [] keys;
      delete [] vals;
    }

    void setVec(int p, ThreadSharedVariant *val) {
      vals[p] = val;
    }

    void set(int p, ThreadSharedVariant *key, ThreadSharedVariant *val) {
      keys[p] = key;
      vals[p] = val;
      if (key->is(KindOfInt64)) {
        if (!intMap) intMap = new Int64ToIntMap(size);
        (*intMap)[key->m_data.num] = p;
      } else {
        ASSERT(key->is(KindOfString));
        if (!strMap) strMap = new StringDataToIntMap(size);
        (*strMap)[key->m_data.str] = p;
      }
    }
  };

  union {
    int64 num;
    double dbl;
    StringData *str;
    MapData* map;
  } m_data;
  bool m_owner;
};

class ThreadSharedVariantLockedRefs : public ThreadSharedVariant {
public:
  ThreadSharedVariantLockedRefs(CVarRef source, bool serialized, Mutex &lock,
                                bool inner = false)
    : ThreadSharedVariant(source, serialized, inner), m_lock(lock) {}

  virtual void incRef() {
    Lock lock(m_lock);
    ++m_ref;
  }

  virtual void decRef() {
    Lock lock(m_lock);
    ASSERT(m_ref);
    if (--m_ref == 0) {
      delete this;
    }
  }

protected:
  Mutex &m_lock;
  virtual ThreadSharedVariant *createAnother(CVarRef source, bool serialized,
                                             bool inner = false);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_THREAD_SHARED_VARIANT_H__ */
