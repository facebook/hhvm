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

#include "hphp/runtime/base/smart-containers.h"
#include <memory>

namespace HPHP {
namespace jit {

template<class T>
using vector = std::vector<T>;

template <class T, class Container = std::deque<T>>
using stack = std::stack<T, Container>;

template <class T, class Compare = std::less<T>>
using priority_queue = std::priority_queue<T, vector<T>, Compare>;

template <class T>
using list = std::list<T>;

// subclass and pass 0 to the constructor to override the default
// (large) buckets size.
template <class T, class U, class V=std::hash<T>, class W=std::equal_to<T>>
struct hash_map: std::unordered_map<T,U,V,W> {
  hash_map() : std::unordered_map<T,U,V,W>(0) {}
};

template <class T, class V = std::hash<T>, class W = std::equal_to<T>>
struct hash_set: std::unordered_set<T,V,W> {
  hash_set() : std::unordered_set<T,V,W>(0) {}
};

template<class T>
using unique_ptr = std::unique_ptr<T>;

template<class K, class Pred = std::less<K>>
using flat_set = boost::container::flat_set<K, Pred>;

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

}
}
#endif
