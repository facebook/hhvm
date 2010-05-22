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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ThreadSharedVariantMapData;
class ThreadSharedVariant;
struct ThreadSharedVariantHash;

template<class T>
class ptreq {
public:
  bool operator()(const T& x, const T& y) const {
    if (x && y) {
      return *x == *y;
    }
    return x == y;
  }
};

class ThreadSharedVariantHash {
public:
  size_t operator()(ThreadSharedVariant* v) const;
};

typedef hphp_hash_map<ThreadSharedVariant*, int, ThreadSharedVariantHash,
                      ptreq<ThreadSharedVariant*> > ThreadSharedVariantToIntMap;

///////////////////////////////////////////////////////////////////////////////

class ThreadSharedVariant : public SharedVariant {
 public:
  ThreadSharedVariant(CVarRef source, bool serialized);
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
  bool operator==(const SharedVariant& svother) const;
  ssize_t hash() const;

  const char* stringData() const;
  size_t stringLength() const;

  size_t arrSize() const;
  int getIndex(CVarRef key);
  SharedVariant* get(CVarRef key);
  bool exists(CVarRef key);

  void loadElems(ArrayData *&elems);

  virtual SharedVariant* getKey(ssize_t pos) const {
    ASSERT(is(KindOfArray));
    return keys()[pos];
  }
  virtual SharedVariant* getValue(ssize_t pos) const {
    ASSERT(is(KindOfArray));
    return vals()[pos];
  }

  // implementing LeakDetectable
  virtual void dump(std::string &out);

protected:
  virtual ThreadSharedVariant *createAnother(CVarRef source, bool serialized);

private:
  union {
    int64 num;
    double dbl;
    StringData *str;
    ThreadSharedVariantMapData* map;
  } m_data;
  bool m_owner;

  const ThreadSharedVariantToIntMap &map() const;
  SharedVariant** keys() const;
  SharedVariant** vals() const;

  ThreadSharedVariant(StringData *source);
  ThreadSharedVariant(int64 num);
  ThreadSharedVariantToIntMap::const_iterator lookup(CVarRef key);
};

class ThreadSharedVariantMapData {
public:
  ThreadSharedVariantToIntMap map;
  SharedVariant** keys;
  SharedVariant** vals;
};

class ThreadSharedVariantLockedRefs : public ThreadSharedVariant {
public:
  ThreadSharedVariantLockedRefs(CVarRef source, bool serialized, Mutex &lock)
    : ThreadSharedVariant(source, serialized), m_lock(lock) {}

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
  virtual ThreadSharedVariant *createAnother(CVarRef source, bool serialized);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_THREAD_SHARED_VARIANT_H__ */
