/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_RDS_H_
#define incl_HPHP_RUNTIME_RDS_H_

#include <cstdlib>
#include <cinttypes>
#include <boost/variant.hpp>

#include "hphp/util/util.h"
#include "hphp/runtime/base/types.h"

namespace HPHP {
  struct Func;
  struct ActRec;
  struct Array;
  struct StringData;
  struct TypedValue;
  struct Class;
  struct NamedEntity;
}

//////////////////////////////////////////////////////////////////////

namespace HPHP { namespace RDS {

//////////////////////////////////////////////////////////////////////

/*
 * The RDS (Request Data Segment) is a region of memory quickly
 * accessible to each hhvm thread that is running a PHP request.
 *
 * Essentially this is a per-thread memory region, along with an
 * internal dynamic link table to give the segment the same layout for
 * each thread as new data is allocated.
 *
 * The RDS starts with a small header that is statically layed out,
 * followed by the main segment, which is initialized to zero at the
 * start of each request.  The next section, contains "persistent"
 * data, which is data that retains the same values across requests.
 *
 * The persistent segment is implemented       RDS Layout:
 * by mapping the same physical pages to
 * different virtual addresses, so they          +------------+ <-- tl_base
 * are all accessible from the                   |  Header    |
 * per-thread RDS base.  The normal              +------------+
 * region is perhaps analogous to .bss,          |            |
 * while the persistent region is                |  Normal    |
 * analagous to .rodata.                         |    region  |
 *                                               |            |
 * When we're running in C++, the base           +------------+
 * of RDS is available via a thread              | Persistent | higher
 * local exported from this module               |     region |   addresses
 * (tl_base).  When running in                   +------------+
 * JIT-compiled code, a machine register
 * is reserved to always point at the base of RDS.
 *
 *
 * Allocation/linking API:
 *
 *   You can allocate data from RDS in two primary ways, either by
 *   binding a Link, or anonymously.  The distinction is whether the
 *   allocated space is associated with some unique key that allows it
 *   to be re-found for any new attempts to allocate that symbol.
 *
 *   Anonymous allocations are created with RDS::alloc.  Non-anonymous
 *   allocations can be created in two ways:
 *
 *     RDS::bind(Symbol) uses an RDS-internal link table to find if
 *     there is an existing handle for the given symbol.
 *
 *     RDS::Link<T>::bind allows the caller to make use of the
 *     uniqueness of other runtime structure (e.g. the Class
 *     structure) to avoid having a special key and needing to do
 *     lookups in the internal RDS link table.  The "key" for the
 *     allocation is the RDS::Link<> object itself.
 *
 *   Finally, you can allocate anonymous single bits at a time with
 *   allocBit().
 *
 *
 * Side note: this module originally only contained caches for things
 * like method call targets, so it was referred to as "target cache".
 * There are probably still a few references to that name still
 * around, so it seems worth mentioning ...
 */

//////////////////////////////////////////////////////////////////////

/*
 * Lifetime-related hooks, exported to be called at the appropriate
 * times.
 */
void requestInit();
void requestExit();
void threadInit();
void threadExit();

/*
 * Flushing RDS means to madvise the memory away.  Should only be done
 * while a request is not in flight on this thread.
 *
 * This is done to conserve memory if a particular thread is unlikely
 * to need to serve another PHP request for a while.
 */
void flush();

/*
 * Return the number of bytes that have been allocated from either
 * persistent or non-persistent RDS.
 */
size_t usedBytes();
size_t usedPersistentBytes();

/*
 * The thread-local pointer to the base of RDS.
 */
extern __thread void* tl_base;

//////////////////////////////////////////////////////////////////////

/*
 * Statically layed-out header that goes at the front of RDS.
 */
struct Header {
  /*
   * Surprise flags.  May be written by other threads.  At various
   * points, the runtime will check whether this word is non-zero, and
   * if so go to a slow path to handle unusual conditions (e.g. OOM).
   */
  ssize_t conditionFlags;
};

/*
 * Access to the statically layed out header.
 */
Header* header();

/*
 * Values for dynamically defined constants are stored as key value
 * pairs in an array, accessible here.
 */
Array& s_constants();

constexpr ptrdiff_t kConditionFlagsOff   = offsetof(Header, conditionFlags);

//////////////////////////////////////////////////////////////////////

/*
 * RDS symbols are centrally registered here.
 *
 * All StringData*'s below must be static strings.
 */

/*
 * Symbol for function static locals.  These are RefData's allocated
 * in RDS.
 */
struct StaticLocal { FuncId funcId;
                     const StringData* name; };

/*
 * Class constant values are TypedValue's stored in RDS.
 */
struct ClsConstant { const StringData* clsName;
                     const StringData* cnsName; };

/*
 * SPropCache allocations.  These cache static properties accesses
 * within the class that declares the static property.
 */
struct StaticProp { const StringData* name; };

/*
 * StaticMethod{F,}Cache allocations.  These are used to cache static
 * method dispatch targets in a given class context.  The `name' field
 * here is a string that encodes the target class, property, and
 * source context.
 */
struct StaticMethod  { const StringData* name; };
struct StaticMethodF { const StringData* name; };

typedef boost::variant< StaticLocal
                      , ClsConstant
                      , StaticProp
                      , StaticMethod
                      , StaticMethodF
                      > Symbol;

//////////////////////////////////////////////////////////////////////

enum class Mode { Normal, Persistent };

/*
 * RDS::Link<T> is a thin, typed wrapper around an RDS::Handle.
 *
 * Note that nothing prevents using non-POD types with this.  But
 * nothing here is going to run the constructor.  (In the
 * non-persistent region, the space for T will be zero'd at the
 * start of each request.)
 *
 * Links are atomic types.  All apis may be called concurrently by
 * multiple threads, and the alloc() api guarantees only a single
 * caller will actually allocate new space in RDS.
 */
template<class T>
struct Link {
  explicit Link(Handle handle);
  Link(const Link&);
  ~Link() = default;

  Link& operator=(const Link& r);

  /*
   * Ensure this Link is bound to an RDS allocation.  If it is not,
   * allocate it using this Link itself as the symbol.
   *
   * This function internally synchronizes to avoid double-allocating.
   * It is legal to call it repeatedly with a link that may already be
   * bound.  The `mode' parameter and `Align' parameters are ignored
   * if the link is already bound, and only affects the call that
   * allocates RDS memory.
   *
   * Post: bound()
   */
  template<size_t Align = alignof(T)> void bind(Mode mode = Mode::Normal);

  /*
   * Dereference a Link and access its RDS memory for the current
   * thread.
   *
   * Pre: bound()
   */
  T& operator*() const;
  T* operator->() const;
  T* get() const;

  /*
   * Returns: whether this Link is bound to RDS memory or not.
   * (I.e. is its internal handle valid.)
   */
  bool bound() const;

  /*
   * Access to the underlying RDS::Handle.
   */
  Handle handle() const;

private:
  std::atomic<Handle> m_handle;
};

/*
 * Return a bound link to memory from RDS, using the given Symbol.
 *
 * Mode indicates whether the memory should be placed in the
 * persistent region or not, and Align indicates the alignment
 * requirements.  Both arguments are ignored if there is already an
 * allocation for the Symbol---they only affect the first caller for
 * the given Symbol.
 */
template<class T, size_t Align = alignof(T)>
Link<T> bind(Symbol key, Mode mode = Mode::Normal);

/*
 * Allocate anonymous memory from RDS.
 *
 * The memory is not keyed on any Symbol, so the handle in the
 * returned Link will be unique.
 */
template<class T, size_t Align = alignof(T)>
Link<T> alloc(Mode mode = Mode::Normal);

/*
 * Allocate a single anonymous bit from non-persistent RDS.  The bit
 * can be manipulated with testAndSetBit().
 *
 * Note: the returned integer is *not* an RDS::Handle.
 */
size_t allocBit();
bool testAndSetBit(size_t bit);

//////////////////////////////////////////////////////////////////////

/*
 * Dereference an un-typed RDS::Handle.
 */
template<class T>
T& handleToRef(Handle h) {
  void* vp = static_cast<char*>(tl_base) + h;
  return *static_cast<T*>(vp);
}

/*
 * Returns: whether the supplied handle is from the persistent RDS
 * region.
 */
bool isPersistentHandle(Handle handle);

/*
 * TODO(#2879005): get rid of this function.  It duplicates similar
 * functions in hhbctranslator and class.
 */
bool classIsPersistent(const Class* cls);

//////////////////////////////////////////////////////////////////////

/*
 * Target caches are a use case of RDS.  These are chunks of memory
 * used to cache things like method dispatch targets.
 *
 * TODO(#2879005): this part should probably go back in JIT::
 */

/*
 * Some caches have a Lookup != k, because the TC passes a container
 * with other necessary info to Cache::lookup. E.g., when looking up
 * function preludes, we also need the number of arguments. Since
 * the current ActRec encapsulates both, we pass in an ActRec to
 * Cache::lookup even though CallCache maps Funcs to TCAs.
 *
 * KNLines must be a power of two.
 */
template<typename Key, typename Value, class LookupKey,
  int KNLines = 4,
  typename ReturnValue = Value>
class Cache {
public:
  static const int kNumLines = KNLines;

  struct Pair {
    Key   m_key;
    Value m_value;
  } m_pairs[kNumLines];

  inline Pair* keyToPair(Key k) {
    if (kNumLines == 1) {
      return &m_pairs[0];
    }
    assert(HPHP::Util::isPowerOfTwo(kNumLines));
    return m_pairs + (hashKey(k) & (kNumLines - 1));
  }

private:
  // Each instance needs to implement this
  static int hashKey(Key k);

public:
  typedef Key CacheKey;
  typedef LookupKey CacheLookupKey;
  typedef Value CacheValue;

  static Handle alloc() {
    return HPHP::RDS::alloc<Cache,sizeof(Pair)>(Mode::Normal).handle();
  }
  static ReturnValue lookup(Handle chand, LookupKey lookup,
                            const void* extraKey = nullptr);
};

struct StaticMethodCache {
  const Func* m_func;
  const Class* m_cls;
  static Handle alloc(const StringData* cls,
                      const StringData* meth,
                      const char* ctxName);
  static const Func* lookupIR(Handle chand,
                              const NamedEntity* ne, const StringData* cls,
                              const StringData* meth, TypedValue* vmfp,
                              TypedValue* vmsp);
  static const Func* lookup(Handle chand,
                            const NamedEntity* ne, const StringData* cls,
                            const StringData* meth);
};

struct StaticMethodFCache {
  const Func* m_func;
  int m_static;

  static Handle alloc(const StringData* cls,
                      const StringData* meth,
                      const char* ctxName);
  static const Func* lookupIR(Handle chand, const Class* cls,
                              const StringData* meth, TypedValue* vmfp);
};

typedef Cache<StringData*,const Class*,StringData*> ClassCache;
typedef Cache<const StringData*,const Func*,StringData*> FuncCache;

template<> Handle FuncCache::alloc();
template<> const Func* FuncCache::lookup(Handle,
  StringData*, const void* extraKey);
template<> const Class* ClassCache::lookup(Handle,
  StringData*, const void* extraKey);

/*
 * In order to handle fb_rename_function (when it is enabled), we need
 * to invalidate dynamic function call caches (the FuncCache).  This
 * hook is called when fb_rename_function is used.
 */
void invalidateForRenameFunction(const StringData* name);

/*
 * Static properties.
 *
 * We only cache statically known property name references from within
 * the class.  Current statistics shows in class references dominating
 * by 91.5% of all static property access.
 */
struct SPropCache {
  TypedValue* m_tv;  // public; it is used from TC and we assert the offset

  static Handle alloc(const StringData* sd);

  template<bool raiseOnError>
  static TypedValue* lookup(Handle handle, const Class* cls,
                            const StringData* nm, Class* ctx);

  template<bool raiseOnError>
  static TypedValue* lookupSProp(const Class *cls, const StringData *name,
                                 Class* ctx);
};

struct MethodCache {
  struct Pair {
    uintptr_t m_key;
    const Func* m_value;
  } m_pairs[1];

  inline Pair* keyToPair(uintptr_t k) {
    return &m_pairs[0];
  }

  static int hashKey(uintptr_t);

  static Handle alloc() {
    return ::HPHP::RDS::alloc<MethodCache,alignof(Pair)>(Mode::Normal)
      .handle();
  }
};

void methodCacheSlowPath(MethodCache::Pair* mce,
                         ActRec* ar,
                         StringData* name,
                         Class* cls);

//////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/base/rds-inl.h"

#endif
