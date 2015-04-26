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

#ifndef incl_HPHP_VM_CONTAINERS_H_
#define incl_HPHP_VM_CONTAINERS_H_

#include <array>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/version.hpp>

#include "hphp/util/sparse-id-containers.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * Create some short-hand type aliases for sparse-id-containers using pointers
 * to IR datastructures, where the ids are extracted from pointer types
 * (IRInstruction* and SSATmp*).
 */

namespace jit_containers_detail {

template<class T>
struct idptr_extract {
  auto operator()(T t) const -> decltype(t->id()) { return t->id(); }
};

}

template<class T>
using sparse_idptr_set = sparse_id_set<
  uint32_t,
  const T*,
  jit_containers_detail::idptr_extract<const T*>
>;

template<class K, class V>
using sparse_idptr_map = sparse_id_map<
  uint32_t,
  V,
  const K*,
  jit_containers_detail::idptr_extract<const K*>
>;

//////////////////////////////////////////////////////////////////////

template<class T, std::size_t N>
using array = std::array<T, N>;

template<class T>
using vector = std::vector<T>;

template<class T, class Container = std::deque<T>>
using stack = std::stack<T,Container>;

template<class T, class Container = std::deque<T>>
using queue = std::queue<T,Container>;

template<class T, class Compare = std::less<T>>
using priority_queue = std::priority_queue<T,vector<T>,Compare>;

template<class T>
using list = std::list<T>;

// subclass and pass 0 to the constructor to override the default
// (large) buckets size.
template<class T, class U, class V=std::hash<T>, class W=std::equal_to<T>>
struct hash_map: std::unordered_map<T,U,V,W> {
  hash_map() : std::unordered_map<T,U,V,W>(0) {}
};

template<class T, class V = std::hash<T>, class W = std::equal_to<T>>
struct hash_set: std::unordered_set<T,V,W> {
  hash_set() : std::unordered_set<T,V,W>(0) {}
};

template<class T, class C = std::less<T>>
using set = std::set<T, C>;

template<class T>
using unique_ptr = std::unique_ptr<T>;

template<class K, class Pred = std::less<K>>
#if defined(BOOST_VERSION) && BOOST_VERSION > 105100 && BOOST_VERSION < 105500
// There's some leak in boost's flat_set that caused serious memory problems to
// be reported externally: https://github.com/facebook/hhvm/issues/4268. The
// bug looks to be https://svn.boost.org/trac/boost/ticket/9166 but it's not
// totally clear. There were a ton of leaks fixed in 1.55 -- but FB is using
// 1.51 internally and we aren't hitting the leak. So also unclear where it was
// *introduced*. So for now just picking those two bounds; they may need to be
// adjusted with future reports.
//
// It sounds like the leak might affect other boost containers as well, but we
// only definitively observed it mattering for flat_set.
using flat_set = std::set<K, Pred>;
#else
using flat_set = boost::container::flat_set<K, Pred>;
#endif

template<class K, class V, class Pred = std::less<K>>
using flat_map = boost::container::flat_map<K,V,Pred>;

template<class K, class V, class Pred = std::less<K>>
using flat_multimap = boost::container::flat_multimap<K,V,Pred>;

template<class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T, class... Args> T* make(Args&&... args) {
  return new T(std::forward<Args>(args)...);
}

template<class T> void destroy(T* t) {
  delete t;
}

//////////////////////////////////////////////////////////////////////

}}

#endif
