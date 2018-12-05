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

#ifndef incl_HPHP_VM_CONTAINERS_H_
#define incl_HPHP_VM_CONTAINERS_H_

#include "hphp/util/alloc.h"
#include "hphp/util/compact-vector.h"
#include "hphp/util/fixed-vector.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"

#include <map>
#include <set>
#include <utility>
#include <vector>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>

namespace HPHP {

template<typename T> using vm_vector = std::vector<T, VMAllocator<T>>;
template<typename T> using vm_set = std::set<T, VMAllocator<T>>;

template<typename T> using VMCompactVector = CompactVector<T, VMAllocator<T>>;
template<typename T> using VMFixedVector = FixedVector<T, VMAllocator<T>>;

template<typename K, typename V, typename C = std::less<K>>
using vm_flat_map = boost::container::flat_map<K, V, C,
                                               VMAllocator<std::pair<K,V>>>;

}
#endif
