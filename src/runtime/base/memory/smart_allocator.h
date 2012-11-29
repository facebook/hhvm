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
#define ALLOCOBJIDX(I) (malloc(object_alloc_index_to_size(I)))
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
#define ALLOCOBJIDX(I) (ThreadInfo::s_threadInfo.getNoCheck()-> \
                        instanceIdxAllocator(I)->alloc())
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
 *   DECLARE_SMART_ALLOCATION(MyClass);
 * };
 *
 * IMPLEMENT_SMART_ALLOCATION(MyClass);
 */

typedef void (*AllocatorThreadLocalInit)(void);
std::set<AllocatorThreadLocalInit>& GetAllocatorInitList();
void InitAllocatorThreadLocal() ATTRIBUTE_COLD;

#define DECLARE_SMART_ALLOCATION(T)                                     \
  public:                                                               \
  typedef                                                               \
    ThreadLocalSingleton<SmartAllocator<T, SmartAllocatorImpl::T> >     \
    AllocatorType;                                                      \
  static void *SmaAllocatorInitSetup;                                   \
  void release();                                                       \

#define IMPLEMENT_SMART_ALLOCATION(T)                                   \
  void *T::SmaAllocatorInitSetup =                                      \
    SmartAllocatorInitSetup<T, SmartAllocatorImpl::T>();                \
  void T::release() {                                                   \
    DELETE(T)(this);                                                    \
  }                                                                     \

#define IMPLEMENT_SMART_ALLOCATION_HOT(T)                               \
  void *T::SmaAllocatorInitSetup =                                      \
    SmartAllocatorInitSetup<T, SmartAllocatorImpl::T>();                \
  HOT_FUNC void T::release() {                                          \
    DELETE(T)(this);                                                    \
  }                                                                     \

#define IMPLEMENT_SMART_ALLOCATION_CLS(C, T)                            \
  void *C::T::SmaAllocatorInitSetup =                                   \
    SmartAllocatorInitSetup<C::T, SmartAllocatorImpl::T>();             \
  void C::T::release() {                                                \
    DELETE(T)(this);                                                    \
  }                                                                     \

///////////////////////////////////////////////////////////////////////////////

#define SLAB_SIZE (2 << 20)

/**
 * Just a simple free-list based memory allocator.
 */
class SmartAllocatorImpl : boost::noncopyable {
public:
  enum Name {
    TestAllocator = -1,
    RefData,
    StringData,
    SharedMap,
    VectorArray,
    Variant,
    Bucket,
    ZendArray,
    HphpArray,
    ObjectData,
    GlobalVariables,
    TaintTraceNode,
    TaintTraceData,
    TestGlobals
  };

  struct Iterator;

  // Ensure we have room for freelist and _count tombstone
  static const size_t MinItemSize = 16;

public:
  SmartAllocatorImpl(Name name, int itemSize);
  ~SmartAllocatorImpl();

  Name getAllocatorType() const { return m_name; }
  int getItemSize() const { return m_itemSize;}
  static size_t itemSizeRoundup(size_t n) {
    return n >= MinItemSize ? n : MinItemSize;
  }

  /**
   * Allocation/deallocation of object memory.
   */
  void* alloc() { return alloc(m_itemSize); }
  void* alloc(size_t size);
  void dealloc(void *obj) {
    ASSERT(memset(obj, kSmartFreeFill, m_itemSize));
    m_free.push(obj);
    MemoryManager::TheMemoryManager()->getStats().usage -= m_itemSize;
  }
  void clear() { m_free.clear(); }

  /*
   * Returns whether the given pointer points into this smart
   * allocator (regardless of whether it is already freed).
   */
  bool isFromThisAllocator(void*) const { return false; }

  // keep these frequently used fields together.
private:
  GarbageList m_free;
  const int m_itemSize;
  const Name m_name;
};

/*
 * Object for iterating over all unfreed objects in smart allocator.
 *
 * It is legal to deallocate the currently pointed to element during
 * iteration (and will not affect the iteration state).  Other changes
 * to the allocator during iteration do not have guaranteed behavior.
 *
 * Nop'd out but still here to keep cycle-gc compiling.
 */
struct SmartAllocatorImpl::Iterator : private boost::noncopyable {
  explicit Iterator(const SmartAllocatorImpl*);
  void* current() const; // returns 0 when done
  void next();
};

///////////////////////////////////////////////////////////////////////////////
// This allocator is for known and fixed sized classes, like StringData or
// ArrayData.

template<typename T, SmartAllocatorImpl::Name TNameEnum>
class SmartAllocator : public SmartAllocatorImpl {
 public:
  /**
   * Specify how many items to allocate a time. The more, the less number of
   * times to grow the memory, but the higher chance of increasing memory
   * footprint.
   */
  SmartAllocator() : SmartAllocatorImpl(TNameEnum, sizeof(T)) {
    static_assert(sizeof(T) <= SLAB_SIZE, "slab too small");
  }

  void release(T *p) {
    if (p) {
      p->~T();
      dealloc(p);
    }
  }

  static void Create(void* storage) {
    new (storage) SmartAllocator<T, TNameEnum>();
  }
  static void Delete(SmartAllocator *p) {
    p->~SmartAllocator();
  }
  static void OnThreadExit(SmartAllocator *p) {
    p->~SmartAllocator();
  }
};

template<typename T, SmartAllocatorImpl::Name TNameEnum>
void *SmartAllocatorInitSetup() {
  ThreadLocalSingleton<SmartAllocator<T, TNameEnum> > tls;
  GetAllocatorInitList().insert((AllocatorThreadLocalInit)(tls.getCheck));
  return (void*)tls.getNoCheck;
}

///////////////////////////////////////////////////////////////////////////////
// This allocator is for unknown but fixed sized classes, like ObjectData.

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
  virtual void sweep();

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
};

template<int S>
class ObjectAllocator : public ObjectAllocatorBase {
public:
  static void Create(void* storage) {
    new (storage) ObjectAllocator<S>();
    static_assert(unsigned(S) <= SLAB_SIZE, "slab too small");
  }
  static void Delete(ObjectAllocator *p) {
    p->~ObjectAllocator();
  }
  static void OnThreadExit(ObjectAllocator *p) {
    p->~ObjectAllocator();
  }

  ObjectAllocator() : ObjectAllocatorBase(S) { }
};

///////////////////////////////////////////////////////////////////////////////
}

template<typename T, HPHP::SmartAllocatorImpl::Name TNameEnum>
inline void *operator new(size_t sizeT,
                          HPHP::SmartAllocator<T, TNameEnum> *a) {
  ASSERT(sizeT == sizeof(T));
  return a->alloc(HPHP::SmartAllocatorImpl::itemSizeRoundup(sizeof(T)));
}

inline void *operator new(size_t sizeT, HPHP::ObjectAllocatorBase *a) {
  ASSERT(sizeT <= size_t(a->getItemSize()));
  return a->alloc();
}

template<typename T, HPHP::SmartAllocatorImpl::Name TNameEnum>
inline void operator delete
(void *p, HPHP::SmartAllocator<T, TNameEnum> *a) {
  ASSERT(p);
  a->dealloc((T*)p);
}

inline void operator delete(void *p , HPHP::ObjectAllocatorBase *a) {
  ASSERT(p);
  a->dealloc(p);
}

///////////////////////////////////////////////////////////////////////////////

#endif // __HPHP_SMART_ALLOCATOR_H__
