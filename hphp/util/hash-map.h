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

#include "hphp/util/functional.h"

#include <folly/concurrency/ConcurrentHashMap.h>
#include <folly/container/F14Map.h>

#include <functional>
#include <string>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

// Similar ref/iter stability as std::unordered_map, and allocates each
// instance of pair<T,U> separately, never moving.
template <class T, class U, class V=std::hash<T>, class W=std::equal_to<T>>
using hphp_hash_map = folly::F14NodeMap<T,U,V,W>;

// Fast maps do not have ref/iter stability on rehash, but allocate space
// for values in bulk. Will use F14ValueMap or F14VectorMap depending on
// sizeof(pair<T,U>).
template <class T, class U, class V=std::hash<T>, class W=std::equal_to<T>>
using hphp_fast_map = folly::F14FastMap<T,U,V,W>;

template <class T, class U, class V=std::hash<T>, class W=std::equal_to<T>>
using hphp_vector_map = folly::F14VectorMap<T,U,V,W>;

//////////////////////////////////////////////////////////////////////

// std::string keyed tables, stable entries do not move on rehash.
template<typename T>
using hphp_string_map = hphp_hash_map<std::string, T, string_hash>;

template<typename T>
using hphp_fast_string_map = hphp_fast_map<std::string, T, string_hash>;

template<typename T>
using hphp_string_imap =
  hphp_hash_map<std::string, T, string_hashi, string_eqstri>;

template<typename T>
using hphp_fast_string_imap =
  hphp_fast_map<std::string, T, string_hashi, string_eqstri>;

// c_str-keyed tables, entries do not move on rehash
template<typename T>
using hphp_const_char_imap = hphp_hash_map<const char *, T, hashi, eqstri>;

template<class T>
using hphp_const_char_map = hphp_hash_map<const char*, T, cstr_hash, eqstr>;

//////////////////////////////////////////////////////////////////////

// folly::ConcurrentHashMapSIMD is only defined on some platforms, so
// fallback to ConcurrentHashMap if not available.
#if FOLLY_SSE_PREREQ(4, 2) && !FOLLY_MOBILE
template <typename K, typename V,
          typename H = std::hash<K>,
          typename C = std::equal_to<K>>
using folly_concurrent_hash_map_simd = folly::ConcurrentHashMapSIMD<K, V, H, C>;
#else
template <typename K, typename V,
          typename H = std::hash<K>,
          typename C = std::equal_to<K>>
using folly_concurrent_hash_map_simd = folly::ConcurrentHashMap<K, V, H, C>;
#endif

//////////////////////////////////////////////////////////////////////

}
