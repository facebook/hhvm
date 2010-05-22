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

#ifndef __HPHP_SMART_ALLOCATOR_H__
#define __HPHP_SMART_ALLOCATOR_H__

#include <util/base.h>
#include <util/thread_local.h>
#include <util/stack_trace.h>
#include <util/chunk_list.h>
#include <runtime/base/memory/linear_allocator.h>
#include <boost/dynamic_bitset.hpp>

namespace HPHP {

#ifdef DEBUG_MEMORY_LEAK
#define DEBUGGING_SMART_ALLOCATOR 1
#endif

// #define DEBUGGING_SMART_ALLOCATOR 1
//#define SMART_ALLOCATOR_STACKTRACE 1

///////////////////////////////////////////////////////////////////////////////
/**
 * If a class is using SmartAllocator, all "new" and "delete" should be done
 * through these two macros in a form like this,
 *
 *   MyClass *obj = NEW(MyClass)(...);
 *   DELETE(MyClass)(obj);
 */

#ifdef DEBUGGING_SMART_ALLOCATOR
#define NEW(T) new T
#define NEWOBJ(T) new T
#define DELETE(T) delete
#define DELETE_EX_CLS(NS,T) delete this
#define DELETE_OBJECT(T) delete this
#else
#define NEW(T) new (T::Allocator.get()) T
#define NEWOBJ(T) new (info->m_allocators[T::AllocatorSeqno]) T
#define DELETE(T) T::Allocator->release
#define DELETE_EX_CLS(NS,T) this->~T(); NS::T::Allocator->release(this)
#define DELETE_OBJECT(T) this->~T()
#endif
#define DELETE_EX(T) DELETE_EX_CLS(,T)

///////////////////////////////////////////////////////////////////////////////
/**
 * To use this allocator, simply add DECLARE_SMART_ALLOCATION macro to .h and
 * add IMPLEMENT_SMART_ALLOCATION macro to .cpp. For example,
 *
 * class MyClass {
 *   DECLARE_SMART_ALLOCATION(MyClass, SmartAllocatorImpl::NoCallbacks);
 * };
 *
 * IMPLEMENT_SMART_ALLOCATION(MyClass, SmartAllocatorImpl::NoCallbacks);
 */

#define DECLARE_SMART_ALLOCATION(T, F)                                  \
  public:                                                               \
  typedef SmartAllocator<T, SmartAllocatorImpl::T, F> AllocatorType;    \
  static DECLARE_THREAD_LOCAL(AllocatorType, Allocator);                \
  void release();                                                       \

#define IMPLEMENT_SMART_ALLOCATION(T, F)                                \
  IMPLEMENT_THREAD_LOCAL(T::AllocatorType, T::Allocator);               \
  void T::release() {                                                   \
    DELETE(T)(this);                                                    \
  }                                                                     \

#define IMPLEMENT_SMART_ALLOCATION_CLS(C, T, F)                         \
  IMPLEMENT_THREAD_LOCAL(C::T::AllocatorType, C::T::Allocator);         \
  void C::T::release() {                                                \
    DELETE(T)(this);                                                    \
  }                                                                     \

#define DECLARE_SMART_ALLOCATION_NOCALLBACKS(T)                         \
  DECLARE_SMART_ALLOCATION(T, SmartAllocatorImpl::NoCallbacks);         \
  bool calculate(int &size) {                                           \
    ASSERT(false);                                                      \
    return false;                                                       \
  }                                                                     \
  void backup(LinearAllocator &allocator) {                             \
    ASSERT(false);                                                      \
  }                                                                     \
  void restore(const char *&data) {                                     \
    ASSERT(false);                                                      \
  }                                                                     \
  void sweep() {                                                        \
  }                                                                     \

#define IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(T)                       \
  IMPLEMENT_SMART_ALLOCATION(T, SmartAllocatorImpl::NoCallbacks)        \

#define IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS_CLS(C, T)                \
  IMPLEMENT_SMART_ALLOCATION_CLS(C, T, SmartAllocatorImpl::NoCallbacks) \

///////////////////////////////////////////////////////////////////////////////

/**
 * Usage stats, all in bytes.
 */
struct MemoryUsageStats {
  int64 usage;     // how many bytes are currently being used
  int64 alloc;     // how many bytes are currently malloc-ed
  int64 peakUsage; // how many bytes have been dispensed at maximum
  int64 peakAlloc; // how many bytes malloc-ed at maximum
};

///////////////////////////////////////////////////////////////////////////////

#define MAX_OBJECT_COUNT_PER_SLAB 64
#define SLAB_SIZE (128 * 1024)

typedef ChunkList<void *, SLAB_SIZE> FreeList;

typedef hphp_hash_map<int64, int, int64_hash> BlockIndexMap;
typedef boost::dynamic_bitset<unsigned long long> FreeMap;

/**
 * Just a simple free-list based memory allocator.
 */
class SmartAllocatorImpl {
public:
  enum Name {
#define SMART_ALLOCATOR_ENTRY(x) x,
#include "smart_allocator.inc"
#undef SMART_ALLOCATOR_ENTRY
  };

  enum Flag {
    NoCallbacks = 0,     // nothing is needed from linear memory allocator
    NeedRestore = 1,     // needs restore from linear memory allocator
    RestoreDisabled = 2, // registered after checkpoint
    NeedRestoreOnce = 4, // needs restore out-of-line memory only once
    NeedSweep = 8,       // needs to collect garbage
  };

public:
  SmartAllocatorImpl(int nameEnum, int itemCount, int itemSize, int flag);
  virtual ~SmartAllocatorImpl();

  /**
   * Called by MemoryManager to store its usage stats pointer inside this
   * allocator for easy access during alloc/free time.
   */
  void registerStats(MemoryUsageStats *stats) { m_stats = stats;}

  int getItemSize() const { return m_itemSize;}
  int getItemCount() const { return m_itemCount;}

  /**
   * Allocation/deallocation of object memory.
   */
  void *alloc();
  void dealloc(void *obj);
  bool isValid(void *obj) const;

  /**
   * MemoryManager functions.
   */
  int calculateObjects(LinearAllocator &allocator, int &size);
  void backupObjects(LinearAllocator &allocator);
  void rollbackObjects(LinearAllocator &allocator);
  void logStats();
  void checkMemory(bool detailed);

  void disableDealloc() { m_dealloc = false;}
  void disableRestore() { m_flag |= RestoreDisabled;}

  /**
   * Delegated to type T.
   */
  virtual int calculate(void *p, int &size) = 0;
  virtual void backup(void *p, LinearAllocator &allocator) = 0;
  virtual void restore(void *p, const char *&data) = 0;
  virtual void sweep(void *p) = 0;
  virtual void dump(void *p) = 0;

 private:
  const char *m_name;
  int m_itemCount;
  int m_itemSize;
  int m_flag;

  std::vector<char *> m_blocks;
  int m_row; // outer index
  int m_col; // inner position
  int m_colMax;

  FreeList m_freelist;

  // checkpoint members
  std::vector<char *> m_backupBlocks;
  FreeList m_backupFreelist;
  int m_rowChecked;
  int m_colChecked;
  int m_linearSize;
  int m_linearCount;

  int m_allocatedBlocks;  // how many blocks are left in the last batch
  int m_multiplier;       // allocate m_multiplier blocks at once
  int m_maxMultiplier;    // the max possible multiplier
  int m_targetMultiplier; // updated upon rollback

  class PointerIterator {
  public:
    PointerIterator(SmartAllocatorImpl *allocator);
    void clear();

    void begin();
    void *get() { return m_px;}
    void next();

  private:
    SmartAllocatorImpl *m_allocator;
    int m_itemSize;
    int m_itemCount;
    char *m_px;
    bool m_prepared;
    BlockIndexMap m_blockIndex;
    FreeMap m_freeMap;

    int m_curRow;
    int m_offset;
    int m_curFree;

    bool search();
  };

  PointerIterator m_iter;

  void copyMemoryBlocks(std::vector<char *> &dest,
                        const std::vector<char *> &src,
                        int lastCol, int lastBlockSize);

protected:
  bool m_dealloc;
  bool m_linearized; // No more restore needed for rollback

#ifdef SMART_ALLOCATOR_STACKTRACE
  std::map<void*, StackTrace> m_st_allocs;
  std::map<void*, StackTrace> m_st_deallocs;
#endif

  MemoryUsageStats *m_stats;

  friend class PointerIterator;
  void prepareIterator(BlockIndexMap &indexMap, FreeMap &freeMap);
};

///////////////////////////////////////////////////////////////////////////////
// This allocator is for known and fixed sized classes, like StringData or
// ArrayData.

template<typename T, int TNameEnum, int flag>
class SmartAllocator : public SmartAllocatorImpl {
 public:
  /**
   * Specify how many items to allocate a time. The more, the less number of
   * times to grow the memory, but the higher chance of increasing memory
   * footprint.
   */
  SmartAllocator(int itemCount = -1)
    : SmartAllocatorImpl(TNameEnum, itemCount, sizeof(T), flag) {}

  void release(T *p) {
    if (p) {
      p->~T();
      if (m_dealloc) {
        dealloc(p);
      }
    }
  }

  virtual int calculate(void *p, int &size) {
    ASSERT(p);
    return ((T*)p)->calculate(size);
  }

  virtual void backup(void *p, LinearAllocator &allocator) {
    ASSERT(p);
    ((T*)p)->backup(allocator);
  }

  virtual void restore(void *p, const char *&data) {
    ASSERT(p);
    ((T*)p)->restore(data);
  }

  virtual void sweep(void *p) {
    ASSERT(p);
    ((T*)p)->sweep();
  }

  virtual void dump(void *p) {
    if (p == NULL) {
      printf("(null)");
    } else {
      ((T*)p)->dump();
    }
  }
};

///////////////////////////////////////////////////////////////////////////////
// This allocator is for unknown but fixed sized classes, like ObjectData.
// NS::T::s_T_initializer allows private inner classes to be initialized,
// this is completely hidden by using a nested private llocatorInitializer

#define DECLARE_OBJECT_ALLOCATION(T)                                    \
  public:                                                               \
  static int AllocatorSeqno;                                            \
  static ObjectAllocatorWrapper Allocator;                              \
  virtual void release();                                               \
  virtual void sweep();

#define IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP_CLS(NS,T)              \
  static ThreadLocalSingleton<ObjectAllocator<ItemSize<sizeof(T)>::value> > \
    g_ ## T ## _allocator;                                                  \
  static ObjectAllocatorBase *get_ ## T ## _allocator() {                   \
    return g_ ## T ## _allocator.get();                                     \
  }                                                                         \
  int NS::T::AllocatorSeqno =                                               \
    ObjectAllocatorWrapper::getAllocatorSeqno(sizeof(T));                   \
  ObjectAllocatorWrapper NS::T::Allocator(NS::T::AllocatorSeqno,            \
                                          get_ ## T ## _allocator);         \
  void NS::T::release() {                                                   \
    destruct();                                                             \
    DELETE_EX_CLS(NS, T);                                                   \
  }

#define IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(T)                 \
    IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP_CLS(HPHP,T)

#define IMPLEMENT_OBJECT_ALLOCATION_CLS(NS,T)                           \
  IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP_CLS(NS,T);               \
  void NS::T::sweep() {                                                 \
    DELETE_OBJECT(T);                                                   \
  }

#define IMPLEMENT_OBJECT_ALLOCATION(T) IMPLEMENT_OBJECT_ALLOCATION_CLS(HPHP,T)

class ObjectAllocatorBase : public SmartAllocatorImpl {
public:
  ObjectAllocatorBase(int itemSize);

  void release(void *p) {
    if (p && m_dealloc) {
      dealloc(p);
    }
  }

  virtual int calculate(void *p, int &size);
  virtual void backup(void *p, LinearAllocator &allocator);
  virtual void restore(void *p, const char *&data);
  virtual void sweep(void *p);
  virtual void dump(void *p);
};

template<int S>
class ObjectAllocator : public ObjectAllocatorBase {
public:
  static ObjectAllocator<S> *Create() {
    return new ObjectAllocator<S>();
  }
  static void Delete(ObjectAllocator *p) {
    delete p;
  }

  ObjectAllocator() : ObjectAllocatorBase(S) { }
};

class ObjectAllocatorWrapper;
class ObjectAllocatorCollector {
public:
  static std::map<int, ObjectAllocatorWrapper *> &getWrappers() {
    static std::map<int, ObjectAllocatorWrapper *> wrappers;
    return wrappers;
  }
};

class ObjectAllocatorWrapper {
public:
  ObjectAllocatorWrapper(int seqno, ObjectAllocatorBase *(*get)(void))
  : m_get(get) {
    ObjectAllocatorCollector::getWrappers()[seqno] = this;
  }

  ObjectAllocatorBase *operator->() const {
    return m_get();
  }

  ObjectAllocatorBase *get() const {
    return m_get();
  }

  static int getAllocatorSeqno(int size);

private:
  ObjectAllocatorBase *(*m_get)(void);
};

///////////////////////////////////////////////////////////////////////////////
}

template<typename T, int TNameEnum, int F>
inline void *operator new
(size_t sizeT, HPHP::SmartAllocator<T, TNameEnum, F> *a) {
  return a->alloc();
}

inline void *operator new(size_t sizeT, HPHP::ObjectAllocatorBase *a) {
  return a->alloc();
}

///////////////////////////////////////////////////////////////////////////////

#endif // __HPHP_SMART_ALLOCATOR_H__
