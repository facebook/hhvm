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

#include <atomic>
#include <cstdlib>
#include <cinttypes>
#include <boost/variant.hpp>

#include "hphp/runtime/base/types.h"

namespace HPHP {
  struct Array;
  struct StringData;
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
  std::atomic<ssize_t> conditionFlags;
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

//////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/base/rds-inl.h"

#endif
