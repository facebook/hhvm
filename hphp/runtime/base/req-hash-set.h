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

#ifndef incl_HPHP_RUNTIME_BASE_REQ_HASH_SET_H_
#define incl_HPHP_RUNTIME_BASE_REQ_HASH_SET_H_

#include "hphp/runtime/base/req-malloc.h"
#include <folly/container/F14Set.h>

namespace HPHP { namespace req {

/*
 * hash_set wraps F14NodeSet, which allocates values indirectly,
 * without moving them during rehash. Use when you need similar reference
 * stability to std::unordered_set.
 */
template <class T,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct hash_set : folly::F14NodeSet<T,V,W,Allocator<T>> {
  using Base = folly::F14NodeSet<T,V,W,Allocator<T>>;
  hash_set() : Base() {}

  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    for (const auto& key : *this) scanner.scan(key);
  }
};

/*
 * value_set Wraps F14ValueSet, which stores values directly in the hashtable.
 * Only safe when you don't care when values move on rehash or erase, and
 * don't need contiguous values, like F14VectorSet (below). This set favors
 * hash lookups over iteration, and small cheap-to-move elements.
 */
template <class T,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct value_set : folly::F14ValueSet<T,V,W,ConservativeAllocator<T>> {
  using Base = folly::F14ValueSet<T,V,W,ConservativeAllocator<T>>;
  using value_type = typename Base::value_type;
  value_set() : Base() {}

  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    Base::visitContiguousRanges(
      [&](value_type const* start, value_type const* end) {
        scanner.scan(*start, (const char*)end - (const char*)start);
      });
  }
};

/*
 * vector_set Wraps F14VectorSet, which stores keys contiguously,
 * and maintains insertion order when erase() is not called.
 * iterator walks in LIFO order, reverse_iterator walks in FIFO order.
 *
 * The LIFO order of iterator is efficient when using this pattern:
 *   while (!m.empty()) {
 *     auto it = set.begin();
 *     ...
 *     set.erase(it);
 *   }
 *
 * This set favors iteration and value locality at the cost of some
 * indirection on hash lookups.
 */
template <class T,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct vector_set : folly::F14VectorSet<T,V,W,ConservativeAllocator<T>> {
  using Base = folly::F14VectorSet<T,V,W,ConservativeAllocator<T>>;
  using const_reference = typename Base::const_reference;
  using const_iterator = typename Base::const_iterator;
  using value_type = typename Base::value_type;
  vector_set() : Base() {}

  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    Base::visitContiguousRanges(
      [&](value_type const* start, value_type const* end) {
        scanner.scan(*start, (const char*)end - (const char*)start);
      });
  }
};

/*
 * fast_set chooses either value_set or vector_set, depending on value size,
 * using the same policy as F14FastSet
 *
 * Not using F14FastSet directly allows us to further customize req::value_set
 * and req::vector_set above without re-implementing the same customizations
 * in our own wrapper for F14FastSet.
 */
template <class Key,
          class Hasher = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
struct fast_set : std::conditional_t<
                       sizeof(Key) < 24,
                       value_set<Key, Hasher, KeyEqual>,
                       vector_set<Key, Hasher, KeyEqual>> {
  using Super = std::conditional_t<
      sizeof(Key) < 24,
      value_set<Key, Hasher, KeyEqual>,
      vector_set<Key, Hasher, KeyEqual>>;

  using Super::Super;
  fast_set() : Super() {}
};

}}
#endif
