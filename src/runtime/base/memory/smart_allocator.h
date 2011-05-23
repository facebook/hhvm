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

#ifndef __HPHP_SMART_ALLOCATOR_H__
#define __HPHP_SMART_ALLOCATOR_H__

#include <util/base.h>
#include <util/thread_local.h>
#include <util/stack_trace.h>
#include <util/chunk_list.h>
#include <util/lock.h>
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
#define DELETEOBJ(NS,T,OBJ) delete OBJ
#define SWEEPOBJ(T) delete this
#else
#define NEW(T) new (T::AllocatorType::getNoCheck()) T
#define NEWOBJ(T) new                                  \
  (ThreadLocalSingleton                                \
    <ObjectAllocator<ItemSize<sizeof(T)>::value> >     \
    ::getNoCheck()) T
#define DELETE(T) T::AllocatorType::getNoCheck()->release
#define DELETEOBJ(NS,T,OBJ) OBJ->~T();                 \
  (ThreadLocalSingleton                                \
    <ObjectAllocator<ItemSize<sizeof(T)>::value> >     \
    ::getNoCheck())->release(OBJ)
#define SWEEPOBJ(T) this->~T()
#endif

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

typedef void (*AllocatorThreadLocalInit)(void);
std::set<AllocatorThreadLocalInit>& GetAllocatorInitList();
void InitAllocatorThreadLocal() ATTRIBUTE_COLD;

#define DECLARE_SMART_ALLOCATION(T, F)                                  \
  public:                                                               \
  typedef                                                               \
    ThreadLocalSingleton<SmartAllocator<T, SmartAllocatorImpl::T, F> >  \
    AllocatorType;                                                      \
  static void *SmaAllocatorInitSetup;                                   \
  void release();                                                       \

#define IMPLEMENT_SMART_ALLOCATION(T, F)                                \
  void *T::SmaAllocatorInitSetup =                                      \
    SmartAllocatorInitSetup<T, SmartAllocatorImpl::T, F>();             \
  void T::release() {                                                   \
    DELETE(T)(this);                                                    \
  }                                                                     \

#define IMPLEMENT_SMART_ALLOCATION_CLS(C, T, F)                         \
  void *C::T::SmaAllocatorInitSetup =                                   \
    SmartAllocatorInitSetup<C::T, SmartAllocatorImpl::T, F>();          \
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
  int64 maxBytes;  // what's request's max bytes allowed
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
  MemoryUsageStats & getStats() { return *m_stats; }

  int getItemSize() const { return m_itemSize;}
  int getItemCount() const { return m_itemCount;}

  /**
   * Allocation/deallocation of object memory.
   */
  void *alloc();
  void *allocHelper() NEVER_INLINE;
  void dealloc(void *obj) {
#ifdef SMART_ALLOCATOR_STACKTRACE
    if (!isValid(obj)) {
      Lock lock(s_st_mutex);
      if (s_st_allocs.find(obj) != s_st_allocs.end()) {
        printf("Object %p was allocated from a different thread: %s\n",
               obj, s_st_allocs[obj].toString().c_str());
      } else {
        printf("Object %p was not smart allocated\n", obj);
      }
    }
    s_st_allocs.erase(obj);
#endif
    ASSERT(isValid(obj));
    m_freelist.push_back(obj);
#ifdef SMART_ALLOCATOR_STACKTRACE
    {
      Lock lock(s_st_mutex);
      bool enabled = StackTrace::Enabled;
      StackTrace::Enabled = true;
      s_st_deallocs.operator[](obj);
      StackTrace::Enabled = enabled;
    }
#endif

    ASSERT(m_stats);
    m_stats->usage -= m_itemSize;
  }
  bool isValid(void *obj) const;

  /**
   * MemoryManager functions.
   */
  int calculateObjects(LinearAllocator &allocator, int &size);
  void backupObjects(LinearAllocator &allocator);
  void rollbackObjects(LinearAllocator &allocator);
  void logStats();
  void checkMemory(bool detailed);

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
  BlockIndexMap m_blockIndex;
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
  bool m_linearized; // No more restore needed for rollback

#ifdef SMART_ALLOCATOR_STACKTRACE
  static Mutex s_st_mutex;
  static std::map<void*, StackTrace> s_st_allocs;
  static std::map<void*, StackTrace> s_st_deallocs;
#endif

  MemoryUsageStats *m_stats;

  friend class PointerIterator;
  void prepareIterator(FreeMap &freeMap);
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
      dealloc(p);
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

  static SmartAllocator<T, TNameEnum, flag> *Create() {
    return new SmartAllocator<T, TNameEnum, flag>();
  }
  static void Delete(SmartAllocator *p) {
    delete p;
  }
  static void OnThreadExit(SmartAllocator *p) {
    delete p;
  }
};

template<typename T, int TNameEnum, int flag>
void *SmartAllocatorInitSetup() {
  ThreadLocalSingleton<SmartAllocator<T, TNameEnum, flag> > tls;
  GetAllocatorInitList().insert((AllocatorThreadLocalInit)(tls.getCheck));
  return (void*)tls.getNoCheck;
}

///////////////////////////////////////////////////////////////////////////////
// This allocator is for unknown but fixed sized classes, like ObjectData.
// NS::T::s_T_initializer allows private inner classes to be initialized,
// this is completely hidden by using a nested private llocatorInitializer

#define DECLARE_OBJECT_ALLOCATION_NO_SWEEP(T)                           \
  public:                                                               \
  static void *ObjAllocatorInitSetup;                                   \
  virtual void release();                                               \

#define DECLARE_OBJECT_ALLOCATION(T)                                    \
  DECLARE_OBJECT_ALLOCATION_NO_SWEEP(T)                                 \
  virtual void sweep();                                                 \

#define IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP_CLS(NS,T)          \
  void *NS::T::ObjAllocatorInitSetup =                                  \
    ObjectAllocatorInitSetup<NS::T>();                                  \
  void NS::T::release() {                                               \
    destruct();                                                         \
    DELETEOBJ(NS, T, this);                                             \
  }

#define IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(T)                 \
    IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP_CLS(HPHP,T)

#define IMPLEMENT_OBJECT_ALLOCATION_CLS(NS,T)                           \
  IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP_CLS(NS,T);               \
  void NS::T::sweep() {                                                 \
    SWEEPOBJ(T);                                                        \
  }

#define IMPLEMENT_OBJECT_ALLOCATION(T) IMPLEMENT_OBJECT_ALLOCATION_CLS(HPHP,T)

class ObjectAllocatorBase : public SmartAllocatorImpl {
public:
  ObjectAllocatorBase(int itemSize);

  void release(void *p) {
    if (p) {
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
  static void OnThreadExit(ObjectAllocator *p) {
    delete p;
  }

  ObjectAllocator() : ObjectAllocatorBase(S) { }
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

template<typename T, int TNameEnum, int F>
inline void operator delete
(void *p, HPHP::SmartAllocator<T, TNameEnum, F> *a) {
  a->release((T*)p);
}

inline void operator delete(void *p , HPHP::ObjectAllocatorBase *a) {
  a->release(p);
}

///////////////////////////////////////////////////////////////////////////////

#endif // __HPHP_SMART_ALLOCATOR_H__
