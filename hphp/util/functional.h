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

#include <cstring>
#include <string>

#include <folly/portability/String.h>
#include <folly/Range.h>

#include "hphp/util/bstring.h"
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
  using is_transparent = void;

  bool operator()(const std::string &s1, const std::string &s2) const {
    return strcasecmp(s1.c_str(), s2.c_str()) < 0;
  }
  bool operator()(const std::string &s1, folly::StringPiece s2) const {
    return bstrcasecmp(s1, s2) < 0;
  }
  bool operator()(folly::StringPiece s1, const std::string &s2) const {
    return bstrcasecmp(s1, s2) < 0;
  }
  bool operator()(const std::string &s1, const char* s2) const {
    return strcasecmp(s1.c_str(), s2) < 0;
  }
  bool operator()(const char* s1, const std::string &s2) const {
    return strcasecmp(s1, s2.c_str()) < 0;
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

template <typename T>
struct integralHashCompare {
  bool equal(T s1, T s2) const {
    static_assert(std::is_integral<T>::value, "");
    return s1 == s2;
  }
  size_t hash(T s) const {
    static_assert(sizeof(T) <= sizeof(int64_t), "");
    return hash_int64(int64_t(s));
  }
};

struct stringHashCompare {
  bool equal(const std::string& s1, const std::string& s2) const {
    return s1 == s2;
  }
  size_t hash(const std::string& s) const {
    return hash_string_cs_unsafe(s.c_str(), s.size());
  }
};

struct stringiHashCompare {
  bool equal(const std::string& s1, const std::string& s2) const {
    return s1.size() == s2.size() &&
      strncasecmp(s1.data(), s2.data(), s1.size()) == 0;
  }
  size_t hash(const std::string& s) const {
    return hash_string_i_unsafe(s.c_str(), s.size());
  }
};

template <typename T, typename U, typename THash, typename UHash>
struct pairHashCompare {
  THash thash;
  UHash uhash;

  using PairType = std::pair<T, U>;

  size_t operator() (const PairType& pair) const {
    return hash(pair);
  }

  size_t hash(const PairType& pair) const {
    return hash_int64_pair(thash.hash(pair.first), uhash.hash(pair.second));
  }

  bool equal(const PairType& a, const PairType& b) const {
    return thash.equal(a.first, b.first) && uhash.equal(a.second, b.second);
  }
};

template<typename T>
struct int_hash {
  static_assert(std::is_integral<T>::value);
  size_t operator() (T v) const {
    return hash_int64(static_cast<uint64_t>(v));
  }
};

using int64_hash = int_hash<int64_t>;
using uint64_hash = int_hash<uint64_t>;
using int32_hash = int_hash<int32_t>;
using uint32_hash = int_hash<uint32_t>;

template<typename T>
struct pointer_hash {
  size_t operator() (const T *const p) const {
    return hash_int64(intptr_t(p));
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
