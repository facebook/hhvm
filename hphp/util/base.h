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

#include "hphp/util/compilation-flags.h"
#include "hphp/util/hash.h"
#include "hphp/util/assertions.h"

#ifdef __INTEL_COMPILER
#define va_copy __builtin_va_copy
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef unsigned char uchar;

///////////////////////////////////////////////////////////////////////////////
// stl classes

struct cstr_hash {
  size_t operator()(const char* s) const {
    return hash_string_cs(s, strlen(s));
  }
};

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

///////////////////////////////////////////////////////////////////////////////

} // namespace HPHP

namespace HPHP {
  using std::string;
  using std::vector;
  using boost::lexical_cast;
  using std::dynamic_pointer_cast;
  using std::static_pointer_cast;
}

#endif
