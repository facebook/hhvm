/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#pragma once

#include "hphp/runtime/base/rds-symbol.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/types.h"

#include "hphp/util/alloc.h"
#include "hphp/util/optional.h"
#include "hphp/util/type-scan.h"

#include <folly/Range.h>

#include <atomic>
#include <cinttypes>
#include <cstdlib>
#include <string>
#include <type_traits>

#ifndef RDS_FIXED_PERSISTENT_BASE
// If RDS_FIXED_PERSISTENT_BASE is defined from compiler command line, don't
// mess with it.  This makes it possible not to use fixed persistent base when
// linking against jemalloc 5+.
#ifndef incl_HPHP_UTIL_ALLOC_H_
#error "please include alloc.h before determining RDS implementation!"
#endif
#ifdef USE_LOWPTR
#define RDS_FIXED_PERSISTENT_BASE 1
#endif
#endif

//////////////////////////////////////////////////////////////////////

namespace HPHP::rds {

//////////////////////////////////////////////////////////////////////

/*
 * The RDS (Request Data Segment) is a region of memory quickly accessible to
 * each hhvm thread that is running a PHP request.
 *
 * Essentially this is a per-thread memory region, along with an internal
 * dynamic link table to give the segment the same layout for each thread as
 * new data is allocated.
 *
 * The RDS starts with a small header that is statically layed out, followed by
 * the main "normal" segment, which is (logically) reset at the beginning of
 * every request. The next section, called "local", contains unshared but still
 * persistent data---this is data that is local to a thread but retains its
 * value across requests.  The final section contains shared "persistent" data,
 * which is data that retains the same values across requests.
 *
 * The shared persistent segment is allocated     RDS Layout:
 * within [1G, 4G) offset from the persistent
 * base (0 if RDS_FIXED_PERSISTENT_BASE is       +-------------+ <-- tl_base
 * defined as 1, which is safe if address from   |  Header     |
 * lower_malloc() is below 4G).                  +-------------+
 *                                               |             |
 * The normal region is perhaps analogous to     |  Normal     |
 * .bss, while the persistent region is          |    region   |
 * analogous to .rodata, and the local region    |             | growing higher
 * is similar to .data.                          +-------------+  vvv
 *                                               | \ \ \ \ \ \ |
 * When we're running in C++, the base of RDS    +-------------+  ^^^
 * is available via a thread local exported      |  Local      | growing lower
 * from this module (tl_base).  When running     |    region   |
 * in JIT-compiled code, a machine register      +-------------+ higher address
 * is reserved to always point at the base of    +-------------+
 * RDS.                                          | Persistent  |
 *                                               |     region  |
 *                                               +-------------+
 *
 * Every element in the "normal" segment has an associated generation number,
 * and the segment as a whole has a "current" generation number.  A particular
 * element is considered initialized if its generation number matches the
 * segment's current generation number.  If it does not, the element should be
 * considered to contain garbage and should be initialized as needed by the
 * element type.  Once done, it must be manually marked as initialized.
 *
 * Allocation/linking API:
 *
 *   You can allocate data from RDS in two primary ways, either by
 *   binding a Link, or anonymously.  The distinction is whether the
 *   allocated space is associated with some unique key that allows it
 *   to be re-found for any new attempts to allocate that symbol.
 *
 *   Anonymous allocations are created with rds::alloc.  Non-anonymous
 *   allocations can be created in two ways:
 *
 *     rds::bind(Symbol) uses an RDS-internal link table to find if
 *     there is an existing handle for the given symbol.
 *
 *     rds::Link<T>::bind allows the caller to make use of the
 *     uniqueness of other runtime structure (e.g. the Class
 *     structure) to avoid having a special key and needing to do
 *     lookups in the internal RDS link table.  The "key" for the
 *     allocation is the rds::Link<> object itself.
 *
 *   Finally, you can allocate anonymous single bits at a time with
 *   allocBit().
 *
 */

//////////////////////////////////////////////////////////////////////

/*
 * Lifetime-related hooks, exported to be called at the appropriate
 * times. If shouldRegister is false the resultant rds will be excluded from
 * the global rds list.
 */
void requestInit();
void requestExit();
void threadInit(bool shouldRegister = true);
void threadExit(bool shouldUnregister = true);
void processInit();

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
 * persistent, local, or non-persistent RDS.
 */
size_t usedBytes();
size_t usedLocalBytes();
size_t usedPersistentBytes();

folly::Range<const char*> normalSection();
folly::Range<const char*> localSection();

// Invoke F on each initialized allocation in the normal section. F is invoked
// with a void* pointer to the data, the size of the data, and the stored
// type-index.
template <typename F> void forEachNormalAlloc(F);
template <typename F> void forEachLocalAlloc(F);

/*
 * The thread-local pointer to the base of RDS.
 */
extern __thread void* tl_base;

/*
 * An async singal safe way to determine if the current thread has a fully
 * initialized RDS (including the initialization of rds::local).
 */
bool isFullyInitialized();

//////////////////////////////////////////////////////////////////////

enum class Mode : unsigned {
  Normal        = 1u << 0,
  Local         = 1u << 1,
  Persistent    = 1u << 2,

  NonPersistent = Normal | Local,
  NonLocal      = Normal | Persistent,
  NonNormal     = Local  | Persistent,

  Any           = Normal | Local | Persistent,
};

using ModeU = std::underlying_type<Mode>::type;

template<Mode Mask> constexpr bool maybe(Mode mode) {
  return !!(static_cast<ModeU>(mode) & static_cast<ModeU>(Mask));
}

template<Mode RHS> constexpr bool in(Mode LHS) {
  return !(static_cast<ModeU>(LHS) & ~static_cast<ModeU>(RHS));
}

constexpr bool pure(Mode mask) {
  return !(static_cast<ModeU>(mask) & (static_cast<ModeU>(mask) - 1));
}

/*
 * Handles into Request Data Segment.
 *
 * For `Normal` and `Local` mode, it is an offset from rds::tl_base. The offset
 * must be smaller than 1 << 30.
 *
 * For `Persistent` mode, the handle is an offset from `s_persistent_base`.
 * When `RDS_FIXED_PERSISTENT_BASE` is defined, `s_persistent_base` is always 0,
 * and the handle is the same as the address, and must be at least 1 << 30.
 */
using Handle = uint32_t;
constexpr Handle kUninitHandle = 0;
constexpr Handle kBeingBound = 1;
constexpr Handle kBeingBoundWithWaiters = 2;
constexpr Handle kMinPersistentHandle = 1u << 30;
constexpr Handle kMaxHandle = std::numeric_limits<Handle>::max();

/*
 * Normal segment element generation numbers.
 *
 * The segment's current generation number will never be kInvalidGenNumber;
 * thus, it is safe to use to mark an element as being uninitialized
 * unconditionally.
 */
using GenNumber = uint8_t;
constexpr GenNumber kInvalidGenNumber = 0;

/*
 * Tag passed to RDS handle initialization routines to indicate that the handle
 * is known to be normal.
 */
enum class NormalTag {};

/*
 * rds::Link<T,M> is a thin, typed wrapper around an rds::Handle.
 *
 * Note that nothing prevents using non-POD types with this.  But nothing here
 * is going to run the constructor.  (In the non-persistent region, the space
 * for T will contain garbage at the start of each request.)
 *
 * Links are atomic types.  All apis may be called concurrently by
 * multiple threads, and the alloc() api guarantees only a single
 * caller will actually allocate new space in RDS.
 */
template<class T, Mode M>
struct Link {
  explicit Link(Handle handle = kUninitHandle);

  /*
   * We allow copy construction or assignment from a Link of a narrower Mode,
   * but it is a compile-time error if we try to do it the other way.
   */
  Link(const Link<T,M>&);
  Link<T,M>& operator=(const Link<T,M>&);

  template<Mode OM> /* implicit */ Link(
      const Link<T,OM>&,
      typename std::enable_if<in<M>(OM), bool>::type = false);

  template<Mode OM> typename std::enable_if<in<M>(OM),Link<T,M>>::type&
  operator=(const Link<T,OM>&);

  /*
   * Ensure this Link is bound to an RDS allocation.
   *
   * Allocation is atomic and idempotent.  This ensures that only one thread
   * allocates the handle, and (in the case of persistent handles) that the
   * value is initialized as part of the operation.  The effect is that other
   * threads only ever see an unbound handle or a bound handle with a valid
   * value.
   *
   * rds::Link's are "keyed" by context---generally meaning that they're owned
   * by an object with an independent means of enforcing uniqueness.  The `sym`
   * argument should also be unique, but is used only for optimizations.
   *
   * If the Link is already bound, both `mode` and `Align` are ignored.  If the
   * mode is not persistent, `init_val` is ignored.
   *
   * Post: bound()
   */
  template<size_t Align = alignof(T)>
  void bind(Mode mode, Symbol sym, const T* init_val = nullptr);

  /*
   * Dereference a Link and access its RDS memory for the current thread.
   *
   * Pre: bound()
   */
  T& operator*() const;
  T* operator->() const;
  T* get() const;
  T* getNoProfile() const;

  /*
   * Whether this Link is bound to RDS memory or not (i.e., whether its
   * internal handle is valid).
   */
  bool bound() const;

  /*
   * Access to the underlying rds::Handle.
   */
  Handle handle() const;

  /*
   * Access to the underlying rds::Handle; returns kUninitHandle if
   * its not bound.
   */
  Handle maybeHandle() const;

  /*
   * Return the generation number of this element.
   *
   * Pre: bound() && isNormal()
   */
  GenNumber genNumber() const;

  /*
   * Return the handle for this element's generation number.
   *
   * Pre: bound() && isNormal()
   */
  Handle genNumberHandle() const;

  /*
   * Check whether this element is initialized.
   *
   * This is only an interesting designation for normal handles, where we need
   * to check the generation number.  Persistent handles are expected not to be
   * "published" until they are in some sort of initialized state (which might
   * simply be some nullish sentinel value).
   *
   * Pre: bound()
   */
  bool isInit() const;
  bool isInitNoProfile() const;

  /*
   * Manually mark this element as initialized or uninitialized.
   *
   * Pre: bound() && isNormal()
   */
  void markInit() const;
  void markUninit() const;

  /*
   * Initialize this element to `val'.
   *
   * Anything previously stored in the element is considered to be garbage, so
   * it is not destructed.  initWith() can thus be used to unconditionally
   * initialize something that might already be inited, but only if it's
   * trivially destructible.
   *
   * Post: isInit()
   */
  void initWith(const T& val) const;
  void initWith(T&& val) const;

  /*
   * Check which segment this element resides in.
   *
   * Pre: bound()
   */
  bool isNormal() const;
  bool isLocal() const;
  bool isPersistent() const;

  /*
   * For access from the JIT only.
   */
  static constexpr size_t handleOff() {
    return offsetof(Link, m_handle);
  }

private:
  template<class OT, Mode OM> friend struct Link;

  void checkSanity();

  Handle raw() const { return m_handle.load(std::memory_order_relaxed); }
  std::atomic<Handle> m_handle;
};

/*
 * Return a bound link to memory from RDS, using the given Symbol.
 *
 * Mode indicates whether the memory should be placed in the persistent region
 * or not, Align indicates the alignment requirements, and extraSize allows for
 * allocating additional space beyond sizeof(T), for variable-length structures
 * (not allowed for normal mode).  All three arguments are ignored if there is
 * already an allocation for the Symbol---they only affect the first caller for
 * the given Symbol.
 *
 * N indicates that the binding for `key' will always be in the "normal" RDS
 * region; it is allowed to be true only if `key' is only ever bound with
 * Mode::Normal.
 */
template<class T, Mode M, size_t Align = alignof(T)>
Link<T,M> bind(Symbol key, size_t extraSize = 0);

/*
 * Remove a bound link from RDS metadata. The actual space in RDS is
 * not reclaimed.
 */
void unbind(Symbol, Handle);

/*
 * Try to bind to a symbol in RDS, returning an unbound link if the
 * Symbol isn't already allocated in RDS.  Mode and alignment are not
 * specified---if the symbol is already bound, these are already
 * fixed.  The `T' must be the same `T' that the symbol is mapped to,
 * if it's already mapped.
 */
template<class T, Mode M = Mode::Any>
Link<T,M> attach(Symbol key);

/*
 * Allocate anonymous memory from RDS.
 *
 * The memory is not keyed on any Symbol, so the handle in the returned Link
 * will be unique.
 */
template<class T, Mode M = Mode::Normal, size_t Align = alignof(T)>
Link<T,M> alloc();

/*
 * Allocate a single anonymous bit from non-persistent RDS.  The bit
 * can be manipulated with testAndSetBit().
 *
 * Note: the returned integer is *not* an rds::Handle.
 */
size_t allocBit();
bool testAndSetBit(size_t bit);

/*
 * Table mapping handles to their symbols.  This excludes Profiling symbols.
 */
Optional<Symbol> reverseLink(Handle handle);

/*
 * Table mapping handles to their symbols.
 * Only returns a symbol if the handle is exact, unlike reverseLink(),
 * which returns true if the handle is within range of a symbol.
 */
Optional<Symbol> reverseLinkExact(Handle handle);

//////////////////////////////////////////////////////////////////////

/*
 * Retrieve the current generation number for the normal segment.
 */
GenNumber currentGenNumber();

/*
 * Retrieve the handle for the current generation number for the normal segment.
 */
Handle currentGenNumberHandle();

//////////////////////////////////////////////////////////////////////

/*
 * Dereference an un-typed rds::Handle which is guaranteed to be in one of the
 * `modes`, optionally specifying a specific RDS base to use.
 */
template<class T, Mode M, bool P = true> T& handleToRef(Handle h);
template<class T, Mode M, bool P = true> T& handleToRef(void* base, Handle h);

/*
 * Conversion between a pointer and an rds::Handle which is guaranteed to be in
 * one of the modes specified in `M`.
 */
template<class T = void, Mode M, bool P = true> T* handleToPtr(Handle h);
template<Mode M> Handle ptrToHandle(const void* ptr);
template<Mode M> Handle ptrToHandle(uintptr_t ptr);

/*
 * Whether `handle' looks valid---i.e., whether it lies within the RDS bounds.
 */
bool isValidHandle(Handle handle);

/*
 * Whether `handle' is from the normal RDS region.
 *
 * Pre: isValidHandle(handle)
 */
bool isNormalHandle(Handle handle);

/*
 * Whether `handle' is from the local RDS region.
 *
 * Pre: isValidHandle(handle)
 */
bool isLocalHandle(Handle handle);

/*
 * Whether `handle' is from the persistent RDS region.
 *
 * Pre: isValidHandle(handle)
 */
bool isPersistentHandle(Handle handle);

/*
 * The generation number associated with `handle'.
 *
 * Pre: isNormalHandle(handle)
 */
template <bool P = true>
GenNumber genNumberOf(Handle handle);

/*
 * The handle for the generation number associated with `handle'.
 *
 * Pre: isNormalHandle(handle)
 */
Handle genNumberHandleFrom(Handle handle);

/*
 * Whether the handle has been bound.
 */
bool isHandleBound(Handle handle);

/*
 * Whether the element associated with `handle' is initialized.
 */
bool isHandleInit(Handle handle);
bool isHandleInit(Handle handle, NormalTag);
bool isHandleInitNoProfile(Handle handle);
bool isHandleInitNoProfile(Handle handle, NormalTag);
/*
 * Mark the element associated with `handle' as being initialized.
 *
 * Pre: isNormalHandle(handle)
 */
void initHandle(Handle handle);

/*
 * Marks the element associated with the supplied handle as being
 * uninitialized. This happens automatically after every request, but can be
 * done manually with this.
 *
 * Pre: isNormalHandle(handle)
 */
void uninitHandle(Handle handle);

//////////////////////////////////////////////////////////////////////

bool shouldProfileAccesses();

Handle profileForHandle(Handle);

void markAccess(Handle);

struct Ordering {
  struct Item {
    std::string key;
    size_t size;
    size_t alignment;
  };
  std::vector<Item> persistent;
  std::vector<Item> local;
  std::vector<Item> normal;
};

Ordering profiledOrdering();

void setPreAssignments(const Ordering&);

//////////////////////////////////////////////////////////////////////

/*
 * Used to record information about the rds handle h in the
 * perf-data-pid.map (if enabled).
 * The type indicates the type of entry (eg ClsConstant), and the
 * msg identifies this particular entry (eg function-name:local-name)
 */
void recordRds(Handle h, size_t size,
               folly::StringPiece type, folly::StringPiece msg);
void recordRds(Handle h, size_t size, const Symbol& sym);

void visitSymbols(std::function<void(const Symbol&,Handle,uint32_t)> fun);

/*
 * Return a list of all the tl_bases for any threads that are using RDS
 */
std::vector<void*> allTLBases();

//////////////////////////////////////////////////////////////////////

extern rds::Link<bool, Mode::Persistent> s_persistentTrue;

}

#include "hphp/runtime/base/rds-inl.h"
