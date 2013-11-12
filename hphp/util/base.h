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

#ifndef incl_HPHP_BASE_H_
#define incl_HPHP_BASE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <cinttypes>

#include <errno.h>
#include <string.h>
#include "hphp/runtime/base/strings.h"
#include <unistd.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/poll.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <stack>
#include <string>
#include <map>
#include <list>
#include <set>
#include <memory>
#include <deque>
#include <exception>
#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/type_traits.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include "hphp/util/hash.h"
#include "hphp/util/assertions.h"

#ifdef __INTEL_COMPILER
#define va_copy __builtin_va_copy
#endif

#if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 4)) || \
  __INTEL_COMPILER || defined(__clang__)

#include <unordered_map>
#include <unordered_set>

#define hphp_hash     std::hash
namespace std {
template<>
struct hash<const char*> {
  size_t operator()(const char *s) const {
    return HPHP::hash_string_cs(s, strlen(s));
  }
};
}

namespace HPHP {
template <class _T,class _U,
          class _V = hphp_hash<_T>,class _W = std::equal_to<_T> >
struct hphp_hash_map : std::unordered_map<_T,_U,_V,_W> {
  hphp_hash_map() : std::unordered_map<_T,_U,_V,_W>(0) {}
};

template <class _T,
          class _V = hphp_hash<_T>,class _W = std::equal_to<_T> >
struct hphp_hash_set : std::unordered_set<_T,_V,_W> {
  hphp_hash_set() : std::unordered_set<_T,_V,_W>(0) {}
};
}

#else

#include <ext/hash_map>
#include <ext/hash_set>

#define hphp_hash_map __gnu_cxx::hash_map
#define hphp_hash_set __gnu_cxx::hash_set
#define hphp_hash     __gnu_cxx::hash

#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// debugging

const bool debug =
#ifdef DEBUG
  true
#else
  false
#endif
  ;

const bool packed_tv =
#ifdef PACKED_TV
  true
#else
  false
#endif
  ;

const bool memory_profiling =
#ifdef MEMORY_PROFILING
  true
#else
  false
#endif
  ;

/**
 * Guard bug-for-bug hphpi compatibility code with this predicate.
 */
const bool hphpiCompat = true;

///////////////////////////////////////////////////////////////////////////////
// system includes

#if __WORDSIZE == 64
#define WORDSIZE_IS_64
#endif

typedef unsigned char uchar;

#ifndef ULLONG_MAX
#define ULLONG_MAX 0xffffffffffffffffULL
#endif

///////////////////////////////////////////////////////////////////////////////
// stl classes

struct ltstr {
  bool operator()(const char *s1, const char *s2) const {
    return strcmp(s1, s2) < 0;
  }
};

struct eqstr {
  bool operator()(const char* s1, const char* s2) const {
    return strcmp(s1, s2) == 0;
  }
};

struct stdltstr {
  bool operator()(const std::string &s1, const std::string &s2) const {
    return strcmp(s1.c_str(), s2.c_str()) < 0;
  }
};

struct stdltistr {
  bool operator()(const std::string &s1, const std::string &s2) const {
    return strcasecmp(s1.c_str(), s2.c_str()) < 0;
  }
};

struct string_hash {
  size_t operator()(const std::string &s) const {
    return hash_string_cs(s.c_str(), s.size());
  }
  size_t hash(const std::string &s) const {
    return operator()(s);
  }
};

struct stringHashCompare {
  bool equal(const std::string &s1, const std::string &s2) const {
    return s1 == s2;
  }
  size_t hash(const std::string &s) const {
    return hash_string(s.c_str(), s.size());
  }
};

template<class type, class T> struct hphp_string_hash_map :
  public hphp_hash_map<std::string, type, string_hash> {
};

struct int64_hash {
  size_t operator() (const int64_t v) const {
    return (size_t)hash_int64(v);
  }
  size_t hash(const int64_t v) const {
    return operator()(v);
  }
  bool equal(const int64_t lhs, const int64_t rhs) const {
    return lhs == rhs;
  }
};

template<typename T>
struct pointer_hash {
  size_t operator() (const T *const p) const {
    return (size_t)hash_int64(intptr_t(p));
  }
  size_t hash(const T *const p) const {
    return operator()(p);
  }
  bool equal(const T *const lhs,
             const T *const rhs) const {
    return lhs == rhs;
  }
};

template<typename T>
struct smart_pointer_hash {
  size_t operator() (const T &p) const {
    return (size_t)hash_int64(intptr_t(p.get()));
  }
  size_t hash (const T &p) const {
    return operator()(p);
  }
  bool equal(const T &lhs, const T &rhs) const {
    return lhs.get() == rhs.get();
  }
};

template <class T> class hphp_raw_ptr {
public:
  hphp_raw_ptr() : px(0) {}
  explicit hphp_raw_ptr(T *p) : px(p) {}

  /* implicit */ hphp_raw_ptr(const std::weak_ptr<T> &p)
    : px(p.lock().get())
  {}

  template <class S>
  /* implicit */ hphp_raw_ptr(const std::shared_ptr<S> &p) : px(p.get()) {}
  template <class S>
  /* implicit */ hphp_raw_ptr(const std::weak_ptr<S> &p)
    : px(p.lock().get())
  {}
  template <class S>
  /* implicit */ hphp_raw_ptr(const hphp_raw_ptr<S> &p) : px(p.get()) {}

  std::shared_ptr<T> lock() const {
    return px ? std::static_pointer_cast<T>(px->shared_from_this()) :
      std::shared_ptr<T>();
  }
  bool expired() const {
    return !px;
  }

  template <class S>
  /* implicit */ operator std::shared_ptr<S>() const {
    S *s = px; // just to verify the implicit conversion T->S
    return s ? std::static_pointer_cast<S>(px->shared_from_this()) :
      std::shared_ptr<S>();
  }

  T *operator->() const { assert(px); return px; }
  T *get() const { return px; }
  explicit operator bool() const { return !expired(); }
  void reset() { px = 0; }
private:
  T     *px;
};

#define IMPLEMENT_PTR_OPERATORS(A, B) \
  template <class T, class U> \
  inline bool operator==(const A<T> &p1, const B<U> &p2) { \
    return p1.get() == p2.get(); \
  } \
  template <class T, class U> \
  inline bool operator!=(const A<T> &p1, const B<U> &p2) { \
    return p1.get() != p2.get(); \
  } \
  template <class T, class U> \
  inline bool operator<(const A<T> &p1, const B<U> &p2) { \
    return intptr_t(p1.get()) < intptr_t(p2.get()); \
  }

IMPLEMENT_PTR_OPERATORS(hphp_raw_ptr, hphp_raw_ptr);
IMPLEMENT_PTR_OPERATORS(hphp_raw_ptr, std::shared_ptr);
IMPLEMENT_PTR_OPERATORS(std::shared_ptr, hphp_raw_ptr);

template<typename T>
class hphp_const_char_map :
    public hphp_hash_map<const char *, T, hphp_hash<const char *>, eqstr> {
};

template<typename T>
class hphp_string_map :
    public hphp_hash_map<std::string, T, string_hash> {
};

typedef hphp_hash_set<std::string, string_hash> hphp_string_set;
typedef hphp_hash_set<const char *, hphp_hash<const char *>,
                      eqstr> hphp_const_char_set;

typedef hphp_hash_map<void*, void*, pointer_hash<void> > PointerMap;
typedef hphp_hash_map<void*, int, pointer_hash<void> > PointerCounterMap;
typedef hphp_hash_set<void*, pointer_hash<void> > PointerSet;

typedef std::vector<std::string> StringVec;
typedef std::shared_ptr<std::vector<std::string> > StringVecPtr;
typedef std::pair<std::string, std::string> StringPair;
typedef std::set<std::pair<std::string, std::string> > StringPairSet;
typedef std::vector<StringPairSet> StringPairSetVec;

// Convenience functions to avoid boilerplate checks for set/map<>::end() after
// set/map<>::find().

template<typename Set>
bool
setContains(const Set& m,
            const typename Set::key_type& k) {
  return m.find(k) != m.end();
}

template<typename Map>
bool
mapContains(const Map& m,
            const typename Map::key_type& k) {
  return m.find(k) != m.end();
}

template<typename Map>
typename Map::mapped_type
mapGet(const Map& m,
       const typename Map::key_type& k,
       const typename Map::mapped_type& defaultVal =
                      typename Map::mapped_type()) {
  typename Map::const_iterator i = m.find(k);
  if (i == m.end()) return defaultVal;
  return i->second;
}

template<typename Map>
bool
mapGet(const Map& m,
       const typename Map::key_type& k,
       typename Map::mapped_type* outResult) {
  typename Map::const_iterator i = m.find(k);
  if (i == m.end()) return false;
  if (outResult) *outResult = i->second;
  return true;
}

template<typename Map>
bool
mapGetPtr(Map& m,
          const typename Map::key_type& k,
          typename Map::mapped_type** outResult) {
  typename Map::iterator i = m.find(k);
  if (i == m.end()) return false;
  if (outResult) *outResult = &i->second;
  return true;
}

template<typename Map>
bool
mapGetKey(Map& m,
          const typename Map::key_type& k,
          typename Map::key_type* key_ptr) {
  typename Map::iterator i = m.find(k);
  if (i == m.end()) return false;
  if (key_ptr) *key_ptr = i->first;
  return true;
}

template<typename Map>
void
mapInsert(Map& m,
          const typename Map::key_type& k,
          const typename Map::mapped_type& d) {
  m.insert(typename Map::value_type(k, d));
}

// Known-unique insertion.
template<typename Map>
void
mapInsertUnique(Map& m,
                const typename Map::key_type& k,
                const typename Map::mapped_type& d) {
  assert(!mapContains(m, k));
  mapInsert(m, k, d);
}

// Deep-copy a container of dynamically allocated pointers. Assumes copy
// constructors do the right thing.
template<typename Container>
void
cloneMembers(Container& c) {
  for (typename Container::iterator i = c.begin();
       i != c.end(); ++i) {
    typedef typename Container::value_type Pointer;
    typedef typename boost::remove_pointer<Pointer>::type Inner;
    *i = new Inner(**i);
  }
}

template<typename Container>
void
destroyMapValues(Container& c) {
  for (typename Container::iterator i = c.begin();
       i != c.end(); ++i) {
    delete i->second;
  }
}


///////////////////////////////////////////////////////////////////////////////
// boost

// Let us always use hphp's definition of DECLARE_BOOST_TYPES, esp. when it is
// used as an external library.
#ifdef DECLARE_BOOST_TYPES
#undef DECLARE_BOOST_TYPES
#endif

#define DECLARE_BOOST_TYPES(classname)                                  \
  class classname;                                                      \
  typedef std::shared_ptr<classname> classname ## Ptr;                \
  typedef hphp_raw_ptr<classname> classname ## RawPtr;                  \
  typedef std::weak_ptr<classname> classname ## WeakPtr;              \
  typedef std::shared_ptr<const classname> classname ## ConstPtr;     \
  typedef std::vector<classname ## Ptr> classname ## PtrVec;            \
  typedef std::set<classname ## Ptr> classname ## PtrSet;               \
  typedef std::list<classname ## Ptr> classname ## PtrList;             \
  typedef hphp_string_hash_map<classname ## Ptr, classname>             \
      StringTo ## classname ## PtrMap;                                  \
  typedef hphp_string_hash_map<classname ## PtrVec, classname>          \
      StringTo ## classname ## PtrVecMap;                               \
  typedef hphp_string_hash_map<classname ## PtrSet, classname>          \
      StringTo ## classname ## PtrSetMap;                               \

typedef std::shared_ptr<FILE> FilePtr;

struct null_deleter {
  void operator()(void const *) const {
  }
};

struct file_closer {
  void operator()(FILE *f) const {
    if (f) fclose(f);
  }
};

///////////////////////////////////////////////////////////////////////////////

/*
 * DEBUG-only wrapper around boost::numeric_cast that converts any
 * thrown exceptions to a failed assertion.
 */
template <typename To, typename From>
To safe_cast(From val) {
  if (debug) {
    try {
      return boost::numeric_cast<To>(val);
    } catch (std::bad_cast& bce) {
      std::cerr << "conversion of " << val << " failed in "
                << __PRETTY_FUNCTION__ << " : "
                << bce.what() << std::endl;
      not_reached();
    }
  } else {
    return static_cast<To>(val);
  }
}

template<class T, size_t Sz>
size_t array_size(T (&t)[Sz]) {
  return Sz;
}

template<typename Out, typename In>
Out& readData(In*& it) {
  Out& r = *(Out*)it;
  (char*&)it += sizeof(Out);
  return r;
}
} // namespace HPHP

inline bool ptr_is_low_mem(void* ptr) {
  static_assert(sizeof(void*) == 8, "Unexpected pointer size");
  return !((uint64_t)ptr & 0xffffffff00000000ull);
}

namespace HPHP {
  using std::string;
  using std::vector;
  using boost::lexical_cast;
  using std::dynamic_pointer_cast;
  using std::static_pointer_cast;

  template <typename T, typename U>
  HPHP::hphp_raw_ptr<T> dynamic_pointer_cast(HPHP::hphp_raw_ptr<U> p) {
    return HPHP::hphp_raw_ptr<T>(dynamic_cast<T*>(p.get()));
  }

  template <typename T, typename U>
  HPHP::hphp_raw_ptr<T> static_pointer_cast(HPHP::hphp_raw_ptr<U> p) {
    return HPHP::hphp_raw_ptr<T>(static_cast<T*>(p.get()));
  }
}

#endif
