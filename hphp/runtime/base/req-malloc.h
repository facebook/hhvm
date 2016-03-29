/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_RUNTIME_BASE_REQ_MALLOC_H_
#define incl_HPHP_RUNTIME_BASE_REQ_MALLOC_H_

#include "hphp/util/type-scan.h"

/*
 * req::malloc api for request-scoped memory
 *
 * This is the most generic entry point to the request local
 * allocator.  If you easily know the size of the allocation at free
 * time, it might be more efficient to use MM() apis directly.
 *
 * These functions behave like C's malloc/free, but get memory from
 * the current thread's MemoryManager instance.  At request-end, any
 * un-freed memory is explicitly freed (and in debug, garbage filled).
 * If any pointers to this memory survive beyond a request, they'll be
 * dangling pointers.
 *
 * These functions only guarantee 8-byte alignment for the returned
 * pointer.
 */

namespace HPHP { namespace req {

////////////////////////////////////////////////////////////////////////////////

/*
 * Interfaces to receive raw memory. Whenever possible, prefer the typed
 * interfaces below, such as make_raw<T>.
 */

void* malloc(size_t nbytes,
             type_scan::Index tyindex = type_scan::kIndexUnknown);

// Unknown type-index, but assert there's no pointers within.
inline void* malloc_noptrs(size_t nbytes) {
  return malloc(nbytes, type_scan::kIndexUnknownNoPtrs);
}

void* calloc(size_t count, size_t bytes,
             type_scan::Index tyindex = type_scan::kIndexUnknown);

void* realloc(void* ptr,
              size_t nbytes,
              type_scan::Index tyindex = type_scan::kIndexUnknown);

// Unknown type-index, but assert there's no pointers within.
inline void* realloc_noptrs(void* ptr, size_t nbytes) {
  return realloc(ptr, nbytes, type_scan::kIndexUnknownNoPtrs);
}

char* strndup(const char* str, size_t len);

void free(void* ptr);

/*
 * request-heap (de)allocators for non-POD C++-style stuff. Runs constructors
 * and destructors.
 *
 * Unlike the normal operator delete, req::destroy_raw() requires ~T() must
 * be nothrow and that p is not null.
 */
template<class T, class... Args> T* make_raw(Args&&...);
template<class T> void destroy_raw(T* p);

/*
 * Allocate an array of objects.  Similar to req::malloc, but with
 * support for constructors.
 *
 * Note that explicitly calling req::destroy_raw will run the destructors,
 * but if you let the allocator sweep it the destructors will not be
 * called.
 *
 * Unlike the normal operator delete, req::destroy_raw_array requires
 * ~T() must be nothrow.
 */
template<class T> T* make_raw_array(size_t count);
template<class T> void destroy_raw_array(T* t, size_t count);

/*
 * Allocate an array of objects, memset to 0. Does *not* run any constructors.
 */
template<class T> T* calloc_raw_array(size_t count);

//////////////////////////////////////////////////////////////////////

// STL-style allocator for the request-heap allocator.  (Unfortunately we
// can't use allocator_traits yet.)
//
// You can also use req::Allocator as a model of folly's
// SimpleAllocator where appropriate.
//

template <class T, typename Action = type_scan::Action::Auto>
struct Allocator {
  typedef T              value_type;
  typedef T*             pointer;
  typedef const T*       const_pointer;
  typedef T&             reference;
  typedef const T&       const_reference;
  typedef std::size_t    size_type;
  typedef std::ptrdiff_t difference_type;

  template <class U>
  struct rebind {
    typedef Allocator<U, Action> other;
  };

  pointer address(reference value) {
    return &value;
  }
  const_pointer address(const_reference value) const {
    return &value;
  }

  Allocator() noexcept {}
  Allocator(const Allocator&) noexcept {}
  template<class U, typename A> Allocator(const Allocator<U,A>&) noexcept {}
  ~Allocator() noexcept {}

  Allocator& operator=(const Allocator&) noexcept { return *this; }

  size_type max_size() const {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

  pointer allocate(size_type num, const void* = 0) {
    pointer ret = (pointer)req::malloc(
      num * sizeof(T),
      type_scan::getIndexForMalloc<T, Action>()
    );
    return ret;
  }

  template<class U, class... Args>
  void construct(U* p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }

  void destroy(pointer p) {
    p->~T();
  }

  void deallocate(pointer p, size_type num) {
    req::free((void*)p);
  }

  template<class U, typename A> bool operator==(const Allocator<U,A>&) const {
    return true;
  }

  template<class U, typename A> bool operator!=(const Allocator<U,A>&) const {
    return false;
  }
};

// Variant of Allocator which indicates to the GC type-scanning machinery T
// should be conservative scanned. Meant to be used for container internal
// allocations where we don't want to attempt to exactly scan the internal
// contents. Such containers often using type-punning and other tricks, which
// means an exact scan will fail to find valid pointers (where as conservative
// scan will). Where-ever possible, we'll use the container's public interface
// to scan the values it holds in an exact manner.
template<typename T>
using ConservativeAllocator = Allocator<T, type_scan::Action::Conservative<T>>;

/////////////////////////////////////////////////////////////////////

template<class T, class... Args> T* make_raw(Args&&... args) {
  auto const mem = req::malloc(sizeof(T), type_scan::getIndexForMalloc<T>());
  try {
    return new (mem) T(std::forward<Args>(args)...);
  } catch (...) {
    req::free(mem);
    throw;
  }
}

template<class T> void destroy_raw(T* t) {
  t->~T();
  req::free(t);
}

template<class T> T* make_raw_array(size_t count) {
  T* ret = static_cast<T*>(
    req::malloc(count * sizeof(T), type_scan::getIndexForMalloc<T>())
  );
  size_t i = 0;
  try {
    for (; i < count; ++i) {
      new (&ret[i]) T();
    }
  } catch (...) {
    size_t j = i;
    while (j-- > 0) {
      ret[j].~T();
    }
    req::free(ret);
    throw;
  }
  return ret;
}

template<class T>
void destroy_raw_array(T* t, size_t count) {
  size_t i = count;
  while (i-- > 0) {
    t[i].~T();
  }
  req::free(t);
}

template<class T> T* calloc_raw_array(size_t count) {
  return static_cast<T*>(
    req::calloc(count, sizeof(T), type_scan::getIndexForMalloc<T>())
  );
}

////////////////////////////////////////////////////////////////////////////////

}}

#endif
