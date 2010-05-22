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

#ifndef __HPHP_PROCESS_SHARED_VARIANT_H__
#define __HPHP_PROCESS_SHARED_VARIANT_H__

#include <runtime/base/types.h>
#include <util/shared_memory_allocator.h>
#include <runtime/base/memory/unsafe_pointer.h>
#include <runtime/base/shared/shared_variant.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef boost::interprocess::interprocess_mutex ProcessSharedVariantLock;

/**
 * We could directly use boost::interprocess::offset_ptr<SharedVariant> as
 * SharedVariantOffPtr, but we ran into a compilation error with boost-1.41+.
 * This one level of indirection helped.
 */
class SharedVariantOffPtr {
public:
  SharedVariantOffPtr(SharedVariant *sv) : p(sv) {}
  boost::interprocess::offset_ptr<SharedVariant> p;
};
class SharedVariantOffPtrLess {
public:
  bool operator()(const SharedVariantOffPtr &x,
                  const SharedVariantOffPtr &y) const {
    if (x.p && y.p) {
      return *x.p < *y.p;
    }
    return x.p < y.p;
  }
};

typedef SharedMemoryMapWithComp
<SharedVariantOffPtr, int, SharedVariantOffPtrLess>
ProcessSharedVariantToIntMap;

class ProcessSharedVariantMapData {
public:
  ProcessSharedVariantToIntMap* map;
  SharedMemoryVector<SharedVariant*>* keys;
  SharedMemoryVector<SharedVariant*>* vals;
};

class ProcessSharedVariant : public SharedVariant {
 public:
  ProcessSharedVariant(CVarRef source, ProcessSharedVariantLock* lock);
  ProcessSharedVariant(SharedMemoryString& source);
  virtual ~ProcessSharedVariant();

  virtual void incRef();
  virtual void decRef();

  Variant toLocal();
  bool operator<(const SharedVariant& svother) const;

  const char* stringData() const {
    ASSERT(is(KindOfString));
    return getString()->c_str();
  }
  size_t stringLength() const {
    ASSERT(is(KindOfString));
    return getString()->size();
  }

  size_t arrSize() const {
    return map().size();
  }

  int getIndex(CVarRef key);
  SharedVariant* get(CVarRef key);
  bool exists(CVarRef key);
  void loadElems(ArrayData *&elems);

  virtual SharedVariant* getKey(ssize_t pos) const;
  virtual SharedVariant* getValue(ssize_t pos) const;

  SharedMemoryString* getString() const {
    return getPtr(m_data.str);
  }

  // implementing LeakDetectable
  virtual void dump(std::string &out);

 protected:
  union {
    int64 num;
    double dbl;
    SharedMemoryString* str;
    ProcessSharedVariantMapData* map;
  } m_data;
  ProcessSharedVariantLock* m_lock;

  template<class T>
  T getPtr(T p) const {
    return (T)((size_t)p+(size_t)this);
  }
  template<class T>
  T putPtr(T p) const {
    return (T)((size_t)p-(size_t)this);
  }

  ProcessSharedVariantMapData* getMapData() const {
    return getPtr(m_data.map);
  }
  ProcessSharedVariantLock* getLock() const {
    return getPtr(m_lock);
  }

  const ProcessSharedVariantToIntMap& map() const {
    return *getPtr(getMapData()->map);
  }

  const SharedMemoryVector<SharedVariant*>& keys() const {
    ASSERT(is(KindOfArray));
    return *getPtr(getMapData()->keys);
  }
  const SharedMemoryVector<SharedVariant*>& vals() const {
    ASSERT(is(KindOfArray));
    return *getPtr(getMapData()->vals);
  }
  ProcessSharedVariantToIntMap::const_iterator lookup(CVarRef key);
};


///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_PROCESS_SHARED_VARIANT_H__ */
