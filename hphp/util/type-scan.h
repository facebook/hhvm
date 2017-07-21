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

#ifndef incl_HPHP_UTIL_TYPE_SCAN_H_
#define incl_HPHP_UTIL_TYPE_SCAN_H_

#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "hphp/util/assertions.h"
#include "hphp/util/portability.h"

/*
 * "Type scanners" is machinery to automatically generate functions to walk
 * instances of arbitrary types. These functions report pointers to other
 * allocated types. These will be used by the marking phase of the garbage
 * collector. This avoids having to write manual scan functions for the large
 * number of types which can be allocated in the request heap.
 *
 * Basic use: Call type_scan::init() as early as possible. Tag any allocation
 * out of the request heap by calling getIndexForMalloc<T>() (where T is the
 * type being allocated) and storing the returned type-index somewhere it can be
 * retrieved later. Then when trying to scan that allocation, retrieve the
 * type-index, and call the Scanner::scanByIndex() passing in the type-index. If
 * one wishes to scan a type which isn't request heap allocated (for example, a
 * root), use Scanner::scan<T>() instead.
 *
 * That's the basic use case. One can customize this greatly by using custom
 * actions at the allocation site (see below), and adding custom annotations to
 * the types being scanned. Most users don't need to concern themselves with any
 * of this, as most of the complexity is hidden behind the allocator interface
 * and standard container replacements.
 *
 * Note that there are certain constructs that can appear in types which the
 * scanner generator cannot handle automatically. They include (but may not be
 * exhaustive):
 *
 * - Pointers to void
 * - Unions (if the different members do not have the same runtime layout)
 * - Virtual inheritance
 * - Arrays of indeterminate size
 *
 * If any of these occur, there will be an error while generating the functions,
 * and annotations will be required to resolve (see below).
 */

namespace HPHP { namespace type_scan {

////////////////////////////////////////////////////////////////////////////////

/*
 * Type index used to represent a specific type allocated with optional
 * attributes. The same type can have multiple type indices depending on the
 * context.
 *
 * "kIndexUnknown" and "kIndexUnknownNoPtrs" are special type indices
 * representing an unknown type, with the later being an assertion that the type
 * contains no pointers. These are used for contexts where the type you're
 * allocating isn't statically known, and also for allocations before the
 * type-scanning infrastructure is initialized. The type index "kIndexUnknown"
 * always implies conservative scanning.
 */

// Use 16-bits to represent the type index for now. That's more than enough for
// our purposes currently. If we ever exceed it, the generated scanners will
// fail to compile with a static assertion.
using Index = std::uint16_t;
constexpr const Index kIndexUnknown = 0;
constexpr const Index kIndexUnknownNoPtrs = 1;

////////////////////////////////////////////////////////////////////////////////

}}

// Various ugly internal implementation details put here not to muddle the
// external interface. Look in here if you're interested in implementation
// comments.
#include "hphp/util/type-scan-detail.h"

namespace HPHP { namespace type_scan { namespace Action {

////////////////////////////////////////////////////////////////////////////////

/*
 * When obtaining a type-index via getIndexForMalloc<T>(), one can optionally
 * provide an "action" to go along with this type-index. This action influences
 * the nature of the scanner function generated for the type. This is why a
 * particular type can have multiple type-indices, they might have different
 * actions. These actions are empty types which are passed in as template
 * parameters.
 */

// The default, automatically generate a scanner function.
struct Auto {};

// Don't generate a scanner function for this type-index, be a no-op. This is
// preferable to using kIndexUnknownNoPtrs when you know the type, since it
// preserves the actual type.
struct Ignore {};

// Conservative scan this type. If a list of types is provided in the template
// instantiation, only conservative scan if the scanner generator believes any
// of the given types potentially have pointers to request allocated memory. If
// not, ignore the type. If the list is empty, always conservative scan.
template <typename... T> struct Conservative {};

// Normally, if a scanner function for T receives a pointer to a block of memory
// larger than sizeof(T), it assumes an array of T had been allocated, and runs
// the scanner function for each element. If "WithSuffix<U>" action
// (instantiated on type U) is used in this case, it is assumed only for the
// first sizeof(T) bytes is a T. The rest of the block is assumed to contain an
// array of type U, and a scanner appropriate for U is used on that
// portion. This is meant to be used for types which utilize "flexible array
// members" where some variable amount of allocated after the main object.
template <typename T> struct WithSuffix {};

////////////////////////////////////////////////////////////////////////////////

}

// The type scanners need to know which types are "countable". A countable type
// is one with a reference count that is explicitly managed. The ultimate goal
// for the type scanners is to find all the pointers to countable types. To mark
// a type as being countable, instantiate MarkCountable<> on the type. Its
// usually easiest to have the type T derive from MarkCountable<T>.
template <typename T> struct MarkCountable {};

// Normally countable types are never scanned, even if explicitly
// requested. However, you may want to scan a countable type in certain contexts
// (for example, a countable type which can be both allocated in memory and the
// stack). In that case, use this marker instead.
template <typename T> struct MarkScannableCountable {};

// Obtain a type index for the given type T and an optional action. Asserts that
// this index will be used to scan T, and that T is being allocated here.
template <typename T, typename Action = Action::Auto>
inline Index getIndexForMalloc() {
  // Why do this instead of detail::Indexer<>::s_index ? Because otherwise Clang
  // decides not to emit all the debug information related to the Indexer.
  detail::Indexer<typename std::remove_cv<T>::type, Action> temp;
  return temp.s_index;
}

// Obtain a type index for the given type T. Asserts that this index will be
// used only to scan the T, and that T is *not* being allocated here.
template <typename T>
inline Index getIndexForScan() {
  // Why do this instead of detail::Indexer<>::s_index ? Because otherwise Clang
  // decides not to emit all the debug information related to the Indexer.
  detail::Indexer<typename std::remove_cv<T>::type, detail::ScanAction> temp;
  return temp.s_index;
}

// Obtain the name of the type associated with the given type index.
inline const char* getName(Index index) {
  assert(index < detail::g_metadata_table_size);
  return detail::g_metadata_table[index].m_name;
}

// Return true if any of the generated scanners is non-conservative. This will
// return false before init() is called, as only conservative scanning is done
// until that.
inline bool hasNonConservative() {
  return detail::g_metadata_table_size > 2;
}

// Return true if index is a valid type or if everything is conservative
inline bool isKnownType(Index index) {
  return !hasNonConservative() || index != kIndexUnknown;
}

inline bool hasScanner(Index index) {
  assert(index < detail::g_metadata_table_size);
  return detail::g_metadata_table[index].m_scan !=
         detail::g_metadata_table[kIndexUnknownNoPtrs].m_scan;
}

// Initialize the type scanner infrastructure. Before this is done,
// getIndexForMalloc() will always return kIndexUnknown and any attempts to scan
// will use conservative scanning. For this reason, its important to call init()
// as early as possible.
void init();

// Thrown by init() if initialization fails.
struct InitException: std::runtime_error {
  using std::runtime_error::runtime_error;
};

/*
 * Scanner is what actually performs the scanning (one cannot call the generated
 * functions directly). A scanner is also passed into any type custom scanner
 * functions.  Once instantiated, one can call scan functions on it to gather
 * pointers, then retrieve the pointers once done. The same Scanner can be
 * re-used this way multiple times.
 */
struct Scanner {
  // Enqueue a pointer into this scanner to be reported later. This is meant to
  // be called from type custom scanner functions to report interesting
  // pointers.
  template <typename T> void enqueue(const T* ptr) {
    // Don't allow void*
    static_assert(!detail::IsVoid<T>::value,
                  "Trying to enqueue void pointer(s). "
                  "Please provide a more specific type.");
    // Certain types are statically uninteresting, so don't enqueue pointers to
    // those.
    if (detail::Uninteresting<T*>::value) return;
    m_ptrs.emplace_back(ptr);
  }

  // Enqueue the address of a pointer
  template <typename T> void enqAddr(const T** addr) {
    // Don't allow void**
    static_assert(!detail::IsVoid<T>::value,
                  "Trying to enqueue void pointer(s). "
                  "Please provide a more specific type.");
    // Certain types are statically uninteresting, so don't enqueue pointers to
    // those.
    if (detail::Uninteresting<T*>::value) return;
    m_addrs.emplace_back((const void**)addr);
  }

  /*
   * scan() overloads:
   *
   * Scan an instance of a statically known type. This should be used from
   * within type custom scanners, or for things like roots inside the GC. For
   * scanning objects in the request heap, scanByIndex() should instead by used.
   *
   * There's various overloads for scan() to customize behavior statically based
   * on the type.
   */

  // Overload for types where we statically know the type isn't interesting, so
  // do nothing.
  template <typename T>
  typename std::enable_if<detail::Uninteresting<T>::value>::type
  scan(const T&, DEBUG_ONLY std::size_t size = sizeof(T)) {
    // Even though this function is a nop, still try to catch errors like trying
    // to scan an unbounded array.
    static_assert(!detail::UnboundedArray<T>::value,
                  "Trying to scan unbounded array");
    assert(size % sizeof(T) == 0);
  }

  // Overload for interesting pointer types. "Scanning" a pointer is just
  // enqueuing it, so do that.
  template <typename T>
  typename std::enable_if<std::is_pointer<T>::value &&
                          !detail::Uninteresting<T>::value>::type
  scan(const T& ptr, DEBUG_ONLY std::size_t size = sizeof(T)) {
    // No pointers to void or unbounded arrays.
    static_assert(!detail::IsVoid<T>::value,
                  "Trying to scan void pointer(s). "
                  "Please provide a more specific type.");
    static_assert(!detail::UnboundedArray<T>::value,
                  "Trying to scan unbounded array");

    assert(size == sizeof(T));
    m_addrs.emplace_back((const void**)&ptr);
  }

  // Overload for interesting non-pointer types.
  template <typename T> typename std::enable_if<
    !std::is_pointer<T>::value && !detail::Uninteresting<T>::value
  >::type
  scan(const T& val, std::size_t size = sizeof(T)) {
    static_assert(!detail::IsVoid<T>::value,
                  "Trying to scan void pointer(s). "
                  "Please provide a more specific type.");
    static_assert(!detail::UnboundedArray<T>::value,
                  "Trying to scan unbounded array");
    assert(size % sizeof(T) == 0);
    scanByIndex(getIndexForScan<T>(), &val, size);
  }

  // Report a range to be conservative scanned. Meant to be called from a type
  // custom scanner.
  void conservative(const void* ptr, std::size_t size) {
    m_conservative.emplace_back(ptr, size);
  }

  // Scan a region of memory using the given type-index.
  void scanByIndex(Index index, const void* ptr, std::size_t size) {
    assert(index < detail::g_metadata_table_size);
    detail::g_metadata_table[index].m_scan(*this, ptr, size);
  }

  // Called once all the scanning is done. Callbacks report different
  // pointer types:
  //   F1 - called to report enqueued raw pointers (no address available)
  //   F2 - called to report conservative ranges
  //   F3 - called to report addresses of pointers
  // Afterwards, all the state is cleared, and the scanner can be re-used.
  template <typename F1, typename F2, typename F3>
  void finish(F1&& f1, F2&& f2, F3&& f3) {
    for (auto ptr : m_ptrs) {
      f1(ptr);
    }
    for (auto r : m_conservative) {
      f2(r.first, r.second);
    }
    for (auto addr : m_addrs) {
      f3(addr);
    }
    m_addrs.clear();
    m_ptrs.clear();
    m_conservative.clear();
  }

  // These are logically private, but they're public so that the generated
  // functions can manipulate them directly.
  std::vector<const void**> m_addrs; // pointer locations
  std::vector<const void*> m_ptrs; // pointer values
  std::vector<std::pair<const void*, std::size_t>> m_conservative;
};

/*
 * Type annotations to change generated function behavior:
 *
 * The below macros provide ways to describe certain information about types to
 * the function generator which changes how the function is scanned. There are
 * certain constructs which the scanner generator cannot handle, so in those
 * cases an annotation is required to resolve that.
 *
 * All these annotations must be placed within the type's definition. However,
 * it doesn't matter if they're placed in a public/protected/private section.
 * If a custom scanner function is provided, the type *must* have external
 * linkage (IE, not in an anonymous namespace).
*/

// Provide a custom scanner function for this entire type. The generated
// function will not attempt to do anything with the type, but just call this
// function. It is the custom scanner's responsibility to scan/enqueue all
// members. Note that the generated function will still attempt to scan any
// bases normally.
//
// Warning: these functions will not be called unless exact scanners were
// generated and are being used. Conservative-scan will not call them,
// so the underlying fields must be conservative-scannable to start with.
// Importantly, std containers are not conservatively scannable.
#define TYPE_SCAN_CUSTOM(...)                                           \
  static constexpr const                                                \
  HPHP::type_scan::detail::Custom<__VA_ARGS__>                          \
  ATTRIBUTE_USED ATTRIBUTE_UNUSED                                       \
    TYPE_SCAN_CUSTOM_GUARD_NAME{};                                      \
  void TYPE_SCAN_CUSTOM_NAME(HPHP::type_scan::Scanner& scanner) const   \
    ATTRIBUTE_USED ATTRIBUTE_UNUSED EXTERNALLY_VISIBLE

// Provide a custom scanner for a single field. The generated function will only
// call this scanner for this specific field, and scan the rest normally.
#define TYPE_SCAN_CUSTOM_FIELD(FIELD)                                   \
  void TYPE_SCAN_BUILD_NAME(TYPE_SCAN_CUSTOM_FIELD_NAME, FIELD)(        \
    HPHP::type_scan::Scanner& scanner) const                            \
    ATTRIBUTE_USED ATTRIBUTE_UNUSED EXTERNALLY_VISIBLE

// Provide a custom scanner for a list of base classes. The generated function
// will use this scanner instead of scanning the specified bases. This is useful
// for base classes which you cannot modify the definition of (or if using
// virtual inheritance).
#define TYPE_SCAN_CUSTOM_BASES(...)                                     \
  static constexpr const                                                \
  HPHP::type_scan::detail::CustomBase<__VA_ARGS__>                      \
  ATTRIBUTE_USED ATTRIBUTE_UNUSED                                       \
    TYPE_SCAN_CUSTOM_BASES_NAME{};                                      \
  void TYPE_SCAN_CUSTOM_BASES_SCANNER_NAME(                             \
    HPHP::type_scan::Scanner& scanner) const                            \
    ATTRIBUTE_USED ATTRIBUTE_UNUSED EXTERNALLY_VISIBLE

// Ignore everything about this type, but scan any base classes as normal.
#define TYPE_SCAN_IGNORE_ALL                                            \
  static constexpr const                                                \
  HPHP::type_scan::detail::IgnoreField                                  \
  ATTRIBUTE_USED ATTRIBUTE_UNUSED                                       \
    TYPE_SCAN_IGNORE_NAME{}

// Ignore a single field, but scan the rest as normal.
#define TYPE_SCAN_IGNORE_FIELD(FIELD)                                   \
  static constexpr const                                                \
  HPHP::type_scan::detail::IgnoreField                                  \
  ATTRIBUTE_USED ATTRIBUTE_UNUSED                                       \
    TYPE_SCAN_BUILD_NAME(TYPE_SCAN_IGNORE_FIELD_NAME, FIELD){}

// Ignore the specified list of base classes.
#define TYPE_SCAN_IGNORE_BASES(...)                                     \
  static constexpr const                                                \
  HPHP::type_scan::detail::IgnoreBase<__VA_ARGS__>                      \
  ATTRIBUTE_USED ATTRIBUTE_UNUSED                                       \
    TYPE_SCAN_IGNORE_BASE_NAME{}

// Conservative scan the entire type, but scan any base classes as normal.
#define TYPE_SCAN_CONSERVATIVE_ALL                                      \
  static constexpr const                                                \
  HPHP::type_scan::detail::ConservativeField                            \
  ATTRIBUTE_USED ATTRIBUTE_UNUSED                                       \
    TYPE_SCAN_CONSERVATIVE_NAME{}

// Conservative scan a single field, but scan the rest as normal.
#define TYPE_SCAN_CONSERVATIVE_FIELD(FIELD)                             \
  static constexpr const                                                \
  HPHP::type_scan::detail::ConservativeField                            \
  ATTRIBUTE_USED ATTRIBUTE_UNUSED                                       \
    TYPE_SCAN_BUILD_NAME(TYPE_SCAN_CONSERVATIVE_FIELD_NAME, FIELD){}

// Mark a field as being a "flexible array". IE, an array without a size as the
// last member. This needs to be marked explicitly as sometimes the flexible
// array field starts within the object, sometimes not. Only one field in a type
// can be marked as such.
#define TYPE_SCAN_FLEXIBLE_ARRAY_FIELD(FIELD)                           \
  static constexpr const                                                \
  HPHP::type_scan::detail::FlexibleArrayField                           \
  ATTRIBUTE_USED ATTRIBUTE_UNUSED                                       \
    TYPE_SCAN_BUILD_NAME(TYPE_SCAN_FLEXIBLE_ARRAY_FIELD_NAME, FIELD){}

// "Silence" a base class from a forbidden template error. There's a set of
// template types which are forbidden from containing request heap allocated
// objects, and the scanner generator will attempt to verify this. This opts out
// a list of base classes from that particular check for this specific type.
#define TYPE_SCAN_SILENCE_FORBIDDEN_BASES(...)                          \
  static constexpr const                                                \
  HPHP::type_scan::detail::SilenceForbiddenBase<__VA_ARGS__>            \
  ATTRIBUTE_USED ATTRIBUTE_UNUSED                                       \
    TYPE_SCAN_SILENCE_FORBIDDEN_BASE_NAME{}

////////////////////////////////////////////////////////////////////////////////

}}

#endif
