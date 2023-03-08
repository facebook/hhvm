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
#include "folly/container/F14Set.h"
#include <functional>
#include <string>

namespace HPHP {

// Similar ref/iter stability as std::unordered_set, and allocates each
// instance of T separately, never moving.
template <class T, class V=std::hash<T>, class W=std::equal_to<T>>
using hphp_hash_set = folly::F14NodeSet<T,V,W>;

template <class T, class V=std::hash<T>, class W=std::equal_to<T>>
using hphp_vector_set = folly::F14VectorSet<T,V,W>;

using hphp_vector_string_set = hphp_vector_set<std::string, string_hash>;

// Fast sets do not have ref/iter stability on rehash, but allocate space
// for values in bulk. Will use F14ValueSet or F14VectorSet depending on
// sizeof(T).
template <class T, class V=std::hash<T>, class W=std::equal_to<T>>
using hphp_fast_set = folly::F14FastSet<T,V,W>;

using hphp_fast_string_set = hphp_fast_set<std::string, string_hash>;

// std::string keyed tables, stable entries do not move on rehash.
using hphp_string_set = hphp_hash_set<std::string, string_hash>;

using hphp_string_iset =
  hphp_hash_set<std::string, string_hashi, string_eqstri>;

using hphp_fast_string_iset =
  hphp_hash_set<std::string, string_hashi, string_eqstri>;

// c_str-keyed tables, entries do not move on rehash.
using hphp_const_char_iset = hphp_hash_set<const char *, hashi, eqstri>;

}
