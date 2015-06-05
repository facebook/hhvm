/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <string>
#include <boost/variant.hpp>
#include <folly/Range.h>

#include "hphp/runtime/base/types.h"

namespace HPHP {
  struct Array;
  struct StringData;
}

//////////////////////////////////////////////////////////////////////

namespace HPHP { namespace rds {

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
 * followed by the main "normal" segment, which is initialized to zero
 * at the start of each request.  The next section, called "local",
 * contains unshared but still persistent data---this is data that is
 * local to a thread but retains its value across requests.  The final
 * section contains shared "persistent" data, which is data that
 * retains the same values across requests.
 *
 * The shared persistent segment is           RDS Layout:
 * implemented by mapping the same physical
 * pages to different virtual addresses, so      +-------------+ <-- tl_base
 * are all accessible from the                   |  Header     |
 * per-thread RDS base.  The normal              +-------------+
 * region is perhaps analogous to .bss,          |             |
 * while the persistent region is                |  Normal     |
 * analogous to .rodata, and the local region    |    region   |
 * is similar to .data.                          |             | growing higher
 *                                               +-------------+  vvv
 * When we're running in C++, the base of RDS    | \ \ \ \ \ \ |
 * is available via a thread local exported      +-------------+  ^^^
 * from this module (tl_base).  When running     |  Local      | growing lower
 * in JIT-compiled code, a machine register      |    region   |
 * is reserved to always point at the base of    +-------------+
 * RDS.                                          | Persistent  |
 *                                               |     region  | higher
 *                                               +-------------+   addresses
 *
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
 * persistent, local, or non-persistent RDS.
 */
size_t usedBytes();
size_t usedLocalBytes();
size_t usedPersistentBytes();

folly::Range<const char*> normalSection();
folly::Range<const char*> localSection();
folly::Range<const char*> persistentSection();

/*
 * The thread-local pointer to the base of RDS.
 */
extern __thread void* tl_base;

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

/*
 * Profiling translations may store various kinds of junk under
 * symbols that are keyed on translation id.  These generally should
 * go in Mode::Local or Mode::Persistent, depending on the use case.
 */
struct Profile { TransID transId;
                 Offset bcOff;
                 const StringData* name; };

using Symbol = boost::variant< StaticLocal
                             , ClsConstant
                             , StaticProp
                             , StaticMethod
                             , StaticMethodF
                             , Profile
                             >;

//////////////////////////////////////////////////////////////////////

enum class Mode { Normal, Local, Persistent };

/*
 * rds::Link<T> is a thin, typed wrapper around an rds::Handle.
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
   * Access to the underlying rds::Handle.
   */
  Handle handle() const;

  bool isPersistent() const;

  /*
   * For access from the JIT only.
   */
  static constexpr size_t handleOff() {
    return offsetof(Link, m_handle);
  }

private:
  std::atomic<Handle> m_handle;
};

/*
 * Return a bound link to memory from RDS, using the given Symbol.
 *
 * Mode indicates whether the memory should be placed in the persistent region
 * or not, Align indicates the alignment requirements, and extraSize allows for
 * allocating additional space beyond sizeof(T), for variable-length
 * structures.  All three arguments are ignored if there is already an
 * allocation for the Symbol---they only affect the first caller for the given
 * Symbol.
 */
template<class T, size_t Align = alignof(T)>
Link<T> bind(Symbol key, Mode mode = Mode::Normal, size_t extraSize = 0);

/*
 * Try to bind to a symbol in RDS, returning an unbound link if the
 * Symbol isn't already allocated in RDS.  Mode and alignment are not
 * specified---if the symbol is already bound, these are already
 * fixed.  The `T' must be the same `T' that the symbol is mapped to,
 * if it's already mapped.
 */
template<class T>
Link<T> attach(Symbol key);

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
 * Note: the returned integer is *not* an rds::Handle.
 */
size_t allocBit();
bool testAndSetBit(size_t bit);

//////////////////////////////////////////////////////////////////////

/*
 * Dereference an un-typed rds::Handle, optionally specifying a
 * specific RDS base to use.
 */
template<class T> T& handleToRef(Handle h);
template<class T> T& handleToRef(void* base, Handle h);

/*
 * Returns: whether the supplied handle is from the persistent RDS
 * region.
 */
bool isPersistentHandle(Handle handle);

/*
 * Used to record information about the rds handle h in the
 * perf-data-pid.map (if enabled).
 * The type indicates the type of entry (eg StaticLocal), and the
 * msg identifies this particular entry (eg function-name:local-name)
 */
void recordRds(Handle h, size_t size,
               const std::string& type, const std::string& msg);
void recordRds(Handle h, size_t size, const Symbol& sym);

/*
 * Return a list of all the tl_bases for any threads that are using RDS
 */
std::vector<void*> allTLBases();

/*
 * Values for dynamically defined constants are stored as key value
 * pairs in an array, accessible here.
 */
Array& s_constants();

//////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/base/rds-inl.h"

#endif
