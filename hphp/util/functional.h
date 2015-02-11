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
#ifndef incl_HPHP_FUNCTIONAL_H_
#define incl_HPHP_FUNCTIONAL_H_

#include <cstring>
#include <string>

#include "hphp/util/hash.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Small function objects for use with STL containers and things of
 * that sort.
 */

//////////////////////////////////////////////////////////////////////

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
    return hash_string_cs_unsafe(s.c_str(), s.size());
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
    return hash_string_unsafe(s.c_str(), s.size());
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

//////////////////////////////////////////////////////////////////////
// Case-insensitive ones.

struct hashi {
  size_t operator()(const char *s) const {
    return hash_string_i(s, strlen(s));
  }
};
struct eqstri {
  bool operator()(const char* s1, const char* s2) const {
    return strcasecmp(s1, s2) == 0;
  }
};

struct string_hashi {
  size_t operator()(const std::string &s) const {
    return hash_string_i_unsafe(s.data(), s.size());
  }
};

template<typename S, typename S2=S>
struct stringlike_eqstri {
  bool operator()(const S &s1, const S2 &s2) const {
    return s1.size() == s2.size() &&
      strncasecmp(s1.data(), s2.data(), s1.size()) == 0;
  }
};
typedef stringlike_eqstri<std::string> string_eqstri;

struct string_lessi {
  bool operator()(const std::string &s1, const std::string &s2) const {
    return strcasecmp(s1.data(), s2.data()) < 0;
  }
};

//////////////////////////////////////////////////////////////////////

}

#endif
