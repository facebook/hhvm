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

#ifndef __BASE_H__
#define __BASE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <errno.h>
#include <string.h>
#include <strings.h>
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
#include <deque>
#include <exception>
#include <ext/hash_map>
#include <ext/hash_set>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem/operations.hpp>

#include <util/hash.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// debugging

#include <assert.h>

#ifdef RELEASE
#ifndef ALWAYS_ASSERT
#define ASSERT(x)
#else
#define ASSERT(x) assert(x)
#endif
#else
#define ASSERT(x) assert(x)
#endif

///////////////////////////////////////////////////////////////////////////////
// system includes

#if __WORDSIZE == 64
#define WORDSIZE_IS_64
#endif

typedef unsigned char uchar;
typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;

#ifndef ULLONG_MAX
#define ULLONG_MAX 0xffffffffffffffffULL
#endif

///////////////////////////////////////////////////////////////////////////////
// stl classes

#define hphp_hash_map __gnu_cxx::hash_map
#define hphp_hash_set __gnu_cxx::hash_set
#define hphp_hash     __gnu_cxx::hash

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
    return __gnu_cxx::__stl_hash_string(s.c_str());
  }
  size_t hash(const std::string &s) const {
    return operator()(s);
  }
};

struct string_case_hash {
  size_t operator()(const std::string &s) const {
    return hash_string_i(s.c_str(), s.size());
  }
  size_t hash(const std::string &s) const {
    return operator()(s);
  }
};

struct string_case_eq {
  bool operator()(const std::string &lhs,
             const std::string &rhs) const {
    return !strcasecmp(lhs.c_str(), rhs.c_str());
  }
};

template<class type, class T> struct gnu_case_hash :
  public __gnu_cxx::hash_map<std::string, type, string_hash> {
};

struct int64_hash {
  size_t operator() (const int64 v) const {
    return (size_t)hash_int64(v);
  }
  size_t hash(const int64 v) const {
    return operator()(v);
  }
  bool equal(const int64 lhs, const int64 rhs) const {
    return lhs == rhs;
  }
};

template<typename T>
struct pointer_hash {
  size_t operator() (const T *const &p) const {
    return (size_t)hash_int64(intptr_t(p));
  }
  size_t hash(const T *const &p) const {
    return operator()(p);
  }
  bool equal(const T *const &lhs,
             const T *const &rhs) const {
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
  hphp_raw_ptr() : ptr(0) {}
  explicit hphp_raw_ptr(T *p) : ptr(p) {}

  hphp_raw_ptr(const boost::weak_ptr<T> &p) : ptr(p.lock().get()) {}

  template <class S>
  hphp_raw_ptr(const boost::shared_ptr<S> &p) : ptr(p.get()) {}
  template <class S>
  hphp_raw_ptr(const boost::weak_ptr<S> &p) : ptr(p.lock().get()) {}
  template <class S>
  hphp_raw_ptr(const hphp_raw_ptr<S> &p) : ptr(p.get()) {}

  boost::shared_ptr<T> lock() const {
    return ptr ? boost::static_pointer_cast<T>(ptr->shared_from_this()) :
      boost::shared_ptr<T>();
  }
  bool expired() const {
    return !ptr;
  }

  template <class S>
  operator boost::shared_ptr<S>() const {
    S *s = ptr; // just to verify the implicit conversion T->S
    return s ? boost::static_pointer_cast<S>(ptr->shared_from_this()) :
      boost::shared_ptr<S>();
  }

  T *operator->() const { ASSERT(ptr); return ptr; }
  T *get() const { return ptr; }
  operator bool() const { return !expired(); }
  void reset() { ptr = 0; }
private:
  T     *ptr;
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
IMPLEMENT_PTR_OPERATORS(hphp_raw_ptr, boost::shared_ptr);
IMPLEMENT_PTR_OPERATORS(boost::shared_ptr, hphp_raw_ptr);

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
typedef boost::shared_ptr<std::vector<std::string> > StringVecPtr;
typedef std::pair<std::string, std::string> StringPair;
typedef std::set<std::pair<std::string, std::string> > StringPairSet;
typedef std::vector<StringPairSet> StringPairSetVec;

///////////////////////////////////////////////////////////////////////////////
// boost

// Let us always use hphp's definition of DECLARE_BOOST_TYPES, esp. when it is
// used as an external library.
#ifdef DECLARE_BOOST_TYPES
#undef DECLARE_BOOST_TYPES
#endif

#define DECLARE_BOOST_TYPES(classname)                                  \
  class classname;                                                      \
  typedef boost::shared_ptr<classname> classname ## Ptr;                \
  typedef hphp_raw_ptr<classname> classname ## RawPtr;                  \
  typedef boost::weak_ptr<classname> classname ## WeakPtr;              \
  typedef boost::shared_ptr<const classname> classname ## ConstPtr;     \
  typedef std::vector<classname ## Ptr> classname ## PtrVec;            \
  typedef std::set<classname ## Ptr> classname ## PtrSet;               \
  typedef std::list<classname ## Ptr> classname ## PtrList;             \
  typedef std::deque<classname ## Ptr> classname ## PtrQueue;           \
  typedef gnu_case_hash<classname ## Ptr, classname>                    \
      StringTo ## classname ## PtrMap;                                  \
  typedef gnu_case_hash<classname ## PtrVec, classname>                 \
      StringTo ## classname ## PtrVecMap;                               \
  typedef gnu_case_hash<classname ## PtrSet, classname>                 \
      StringTo ## classname ## PtrSetMap;                               \

typedef boost::shared_ptr<FILE> FilePtr;

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
// Non-gcc compat
#define ATTRIBUTE_UNUSED __attribute__((unused))
#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 3) || __ICC >= 1200 || __GNUC__ > 4
#define ATTRIBUTE_COLD __attribute__((cold))
#else
#define ATTRIBUTE_COLD
#endif

///////////////////////////////////////////////////////////////////////////////
}

namespace boost {

template <typename T, typename U>
HPHP::hphp_raw_ptr<T> dynamic_pointer_cast(HPHP::hphp_raw_ptr<U> p) {
  return HPHP::hphp_raw_ptr<T>(dynamic_cast<T*>(p.get()));
}

template <typename T, typename U>
HPHP::hphp_raw_ptr<T> static_pointer_cast(HPHP::hphp_raw_ptr<U> p) {
  return HPHP::hphp_raw_ptr<T>(static_cast<T*>(p.get()));
}
}

#endif // __BASE_H__
