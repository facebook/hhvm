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
#include <runtime/base/shared/immutable_map.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ThreadSharedVariant;

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
    if (m_isVector) {
      ASSERT(pos < (ssize_t) m_data.vec->size);
      return pos;
    }
    return m_data.map->getKeyIndex(pos)->toLocal();
  }
  virtual SharedVariant* getValue(ssize_t pos) const {
    ASSERT(is(KindOfArray));
    if (m_isVector) {
      ASSERT(pos < (ssize_t) m_data.vec->size);
      return m_data.vec->vals[pos];
    }
    return m_data.map->getValIndex(pos);
  }

  // implementing LeakDetectable
  virtual void dump(std::string &out);

  StringData *getStringData() const {
    ASSERT(is(KindOfString));
    return m_data.str;
  }

protected:
  virtual ThreadSharedVariant *createAnother(CVarRef source, bool serialized,
                                             bool inner = false);

  virtual SharedVariant* getKeySV(ssize_t pos) const {
    ASSERT(is(KindOfArray));
    if (m_isVector) return NULL;
    else return m_data.map->getKeyIndex(pos);
  }

private:
  class VectorData {
  public:
    size_t size;
    ThreadSharedVariant **vals;

    VectorData(size_t s) : size(s) {
      vals = new ThreadSharedVariant *[s];
    }

    ~VectorData() {
      for (size_t i = 0; i < size; i++) {
        vals[i]->decRef();
      }
      delete [] vals;
    }
  };

  union {
    int64 num;
    double dbl;
    StringData *str;
    ImmutableMap* map;
    VectorData* vec;
  } m_data;

  bool m_isVector;
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
