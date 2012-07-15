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

#include <boost/noncopyable.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include <util/base.h>
#include <util/thread_local.h>
#include <util/stack_trace.h>
#include <util/lock.h>
#include <runtime/base/types.h>
#include <runtime/base/util/countable.h>
#include <runtime/base/memory/memory_usage_stats.h>

namespace HPHP {

#ifdef DEBUG_MEMORY_LEAK
#define DEBUGGING_SMART_ALLOCATOR 1
#endif

//#define DEBUGGING_SMART_ALLOCATOR 1
//#define SMART_ALLOCATOR_DEBUG_FREE

///////////////////////////////////////////////////////////////////////////////

/**
 * If a class is using SmartAllocator, all "new" and "delete" should be done
 * through these two macros in a form like this,
 *
 *   MyClass *obj = NEW(MyClass)(...);
 *   DELETE(MyClass)(obj);
 *
 * Note that these various allocation functions should only be used
 * for ObjectData-derived classes.  (If you need other
 * request-lifetime memory you need to do something else.)
 */

template<class T>
inline void smart_allocator_check_type() {
  static_assert((boost::is_base_of<ObjectData,T>::value),
                "Non-ObjectData allocated in smart heap");
}

#ifdef DEBUGGING_SMART_ALLOCATOR
#define NEW(T) new T
#define NEWOBJ(T) new T
#define NEWOBJSZ(T,SZ) new (malloc(SZ)) T
#define ALLOCOBJSZ(SZ) (malloc(SZ))
#define DELETE(T) delete
#define DELETEOBJSZ(SZ) free
#define DELETEOBJ(NS,T,OBJ) delete OBJ
#define RELEASEOBJ(NS,T,OBJ) ::operator delete(OBJ)
#define SWEEPOBJ(T) delete this
#else
#define NEW(T) new (T::AllocatorType::getNoCheck()) T
#define NEWOBJ(T) new                                     \
  ((smart_allocator_check_type<T>(), ThreadLocalSingleton \
    <ObjectAllocator<ObjectSizeClass<sizeof(T)>::value> > \
    ::getNoCheck())) T
#define NEWOBJSZ(T,SZ) \
  new ((smart_allocator_check_type<T>(), info->instanceSizeAllocator(SZ))) T
#define ALLOCOBJSZ(SZ) (ThreadInfo::s_threadInfo.getNoCheck()->\
                        instanceSizeAllocator(SZ)->alloc())
#define DELETE(T) T::AllocatorType::getNoCheck()->release
#define DELETEOBJSZ(SZ) (ThreadInfo::s_threadInfo.getNoCheck()->\
                         instanceSizeAllocator(SZ)->release)
#define DELETEOBJ(NS,T,OBJ) delete OBJ
#define RELEASEOBJ(NS,T,OBJ)                              \
  (ThreadLocalSingleton                                   \
    <ObjectAllocator<ObjectSizeClass<sizeof(T)>::value> > \
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

#define IMPLEMENT_SMART_ALLOCATION_HOT(T, F)                            \
  void *T::SmaAllocatorInitSetup =                                      \
    SmartAllocatorInitSetup<T, SmartAllocatorImpl::T, F>();             \
  HOT_FUNC void T::release() {                                          \
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
  void sweep() {                                                        \
  }                                                                     \

#define IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(T)                       \
  IMPLEMENT_SMART_ALLOCATION(T, SmartAllocatorImpl::NoCallbacks)        \

#define IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS_HOT(T)                   \
  IMPLEMENT_SMART_ALLOCATION_HOT(T, SmartAllocatorImpl::NoCallbacks)    \

#define IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS_CLS(C, T)                \
  IMPLEMENT_SMART_ALLOCATION_CLS(C, T, SmartAllocatorImpl::NoCallbacks) \

///////////////////////////////////////////////////////////////////////////////

#define MAX_OBJECT_COUNT_PER_SLAB 64
#define SLAB_SIZE (128 * 1024)

typedef hphp_hash_map<int64, int, int64_hash> BlockIndexMap;
typedef boost::dynamic_bitset<unsigned long long> FreeMap;

/**
 * A garbage list is a freelist of items that uses the space in the items
 * to store a singly linked list.
 */
class GarbageList {
public:
  GarbageList() : ptr(NULL), sz(0) {
    // We store the free list pointers right at the start of each
    // object.  The VM also stores a flag into the _count field to
    // know the object is deallocated---this assert just makes sure
    // they don't overlap.
    static_assert((FAST_REFCOUNT_OFFSET >= sizeof(void*)),
                  "FAST_REFCOUNT_OFFSET has to be larger than "
                  "sizeof(void*) to work correctly with GarbageList");
  }

  // Pops an item, or returns NULL
  void* maybePop() {
    void** ret = ptr;
    if (ret != NULL) {
      ptr = (void**)*ret;
      sz--;
    }
    return ret;
  }

  // Pushes an item on to the list. The item must be larger than
  // sizeof(void*)
  void push(void* val) {
    void** convval = (void**)val;
    *convval = ptr;
    ptr = convval;
    sz++;
  }

  // Number of items on the list.
  int size() const {
    return sz;
  }

  // Remove all items from this list
  void clear() {
    ptr = NULL;
    sz = 0;
  }

  class Iterator {
  public:
    Iterator(const GarbageList& l) : curptr(l.ptr) {}

    Iterator(const Iterator &other) : curptr(other.curptr) {}
    Iterator() : curptr(NULL) {}

    bool operator==(const Iterator &it) {
      return curptr == it.curptr;
    }

    bool operator!=(const Iterator &it) {
      return !operator==(it);
    }

    Iterator &operator++() {
      if (curptr) {
        curptr = (void**)*curptr;
      }
      return *this;
    }

    Iterator operator++(int) {
      Iterator ret(*this);
      operator++();
      return ret;
    }

    void* operator*() const {
      return curptr;
    }

  private:
    void** curptr;
  };

  Iterator begin() const {
    return Iterator(*this);
  }

  Iterator end() const {
    return Iterator();
  }

  typedef Iterator iterator;

private:
  void** ptr;
  int sz;
};

/**
 * Just a simple free-list based memory allocator.
 */
class SmartAllocatorImpl : boost::noncopyable {
public:
  enum Name {
    TestAllocator = -1,
#define SMART_ALLOCATOR_ENTRY(x) x,
#include "runtime/base/memory/smart_allocator.inc_gen"
#undef SMART_ALLOCATOR_ENTRY
  };

  enum Flag {
    NoCallbacks = 0,     // does not need to sweep
    NeedSweep = 1,       // needs to sweep to collect garbage
  };

  struct Iterator;

public:
  SmartAllocatorImpl(int nameEnum, int itemCount, int itemSize, int flag);
  virtual ~SmartAllocatorImpl();

  /**
   * Called by MemoryManager to store its usage stats pointer inside this
   * allocator for easy access during alloc/free time.
   */
  void registerStats(MemoryUsageStats *stats) { m_stats = stats;}
  MemoryUsageStats & getStats() { return *m_stats; }

  Name getAllocatorType() const { return m_nameEnum; }
  const char* getAllocatorName() const { return m_name; }
  int getItemSize() const { return m_itemSize;}
  int getItemCount() const { return m_itemCount;}

  /**
   * Allocation/deallocation of object memory.
   */
  void *alloc();
  void *allocHelper() NEVER_INLINE;
  void dealloc(void *obj) {
    ASSERT(assertValidHelper(obj));
#ifdef SMART_ALLOCATOR_DEBUG_FREE
    memset(obj, 0xfe, m_itemSize);
#endif
    m_freelist.push(obj);
    if (hhvm) {
      int tomb = RefCountTombstoneValue;
      memcpy((char*)obj + FAST_REFCOUNT_OFFSET, &tomb, sizeof tomb);
    }

    ASSERT(m_stats);
    m_stats->usage -= m_itemSize;
  }

  /*
   * Returns whether the given pointer points into this smart
   * allocator (regardless of whether it is already freed).
   */
  bool isFromThisAllocator(void*) const;

  /**
   * MemoryManager functions.
   */
  void rollbackObjects();
  void logStats();
  void checkMemory(bool detailed);

  /**
   * Delegated to type T.
   */
  virtual void sweep(void *p) = 0;
  virtual void dump(void *p) = 0;

private:
  bool assertValidHelper(void *obj) const;

  const Name m_nameEnum;
  const char* m_name;
  int m_itemCount;
  const int m_itemSize;
  int m_flag;

  std::vector<char *> m_blocks;
  BlockIndexMap m_blockIndex;
  int m_row; // outer index
  int m_col; // inner position
  int m_colMax;

  GarbageList m_freelist;

  int m_allocatedBlocks;  // how many blocks are left in the last batch
  int m_multiplier;       // allocate m_multiplier blocks at once
  int m_maxMultiplier;    // the max possible multiplier
  int m_targetMultiplier; // updated upon rollback

protected:

  MemoryUsageStats *m_stats;

  void prepareFreeMap(FreeMap& freeMap) const;
};

/*
 * Object for iterating over all unfreed objects in smart allocator.
 *
 * It is legal to deallocate the currently pointed to element during
 * iteration (and will not affect the iteration state).  Other changes
 * to the allocator during iteration do not have guaranteed behavior.
 */
struct SmartAllocatorImpl::Iterator : private boost::noncopyable {
  explicit Iterator(const SmartAllocatorImpl*);

  void* current() const; // Returns null if we're done.
  void next();

private:
  const SmartAllocatorImpl& m_sa;
  int m_row;
  int m_col;
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
    : SmartAllocatorImpl(TNameEnum, itemCount,
                         std::max(sizeof(T), sizeof(void*)), flag) {}

  void release(T *p) {
    if (p) {
      p->~T();
      dealloc(p);
    }
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
  inline ALWAYS_INLINE void operator delete(void *p) {                  \
    if (!hhvm || T::IsResourceClass) {                                  \
      RELEASEOBJ(NS, T, p);                                             \
      return;                                                           \
    }                                                                   \
    HPHP::VM::Instance* this_ = (HPHP::VM::Instance*)p;                 \
    HPHP::VM::Class* cls = this_->getVMClass();                         \
    size_t nProps = cls->numDeclProperties();                           \
    size_t builtinPropSize = cls->builtinPropSize();                    \
    TypedValue* propVec =                                               \
      (TypedValue *)((uintptr_t)this_ + sizeof(ObjectData) +            \
                     builtinPropSize);                                  \
    for (unsigned i = 0; i < nProps; ++i) {                             \
      TypedValue* prop = &propVec[i];                                   \
      tvRefcountedDecRef(prop);                                         \
    }                                                                   \
    DELETEOBJSZ(HPHP::VM::Instance::sizeForNProps(nProps) +             \
                builtinPropSize)(this_);                                \
  }

#define DECLARE_OBJECT_ALLOCATION(T)                                    \
  DECLARE_OBJECT_ALLOCATION_NO_SWEEP(T)                                 \
  virtual void sweep();                                                 \

#define IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP_CLS(NS,T)          \
  void *NS::T::ObjAllocatorInitSetup =                                  \
    ObjectAllocatorInitSetup<NS::T>();

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
  ASSERT(p);
  a->dealloc((T*)p);
}

inline void operator delete(void *p , HPHP::ObjectAllocatorBase *a) {
  ASSERT(p);
  a->dealloc(p);
}

///////////////////////////////////////////////////////////////////////////////

#endif // __HPHP_SMART_ALLOCATOR_H__
