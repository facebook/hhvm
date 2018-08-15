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

#ifndef incl_HPHP_RUNTIME_BASE_REQ_HASH_MAP_H_
#define incl_HPHP_RUNTIME_BASE_REQ_HASH_MAP_H_

#include "hphp/runtime/base/req-malloc.h"
#include <folly/container/F14Map.h>

namespace HPHP { namespace req {

/*
 * hash_map wraps F14NodeMap, which allocates key+value indirectly,
 * without moving them during rehash. Use when you need similar reference
 * stability to std::unordered_map.
 */
template <class T,
          class U,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct hash_map : folly::F14NodeMap<
  T, U, V, W,
  Allocator<std::pair<const T,U>>
> {
  using Base = folly::F14NodeMap<
    T, U, V, W, Allocator<std::pair<const T, U>>
  >;
  hash_map() : Base() {}

  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T, U) {
    for (const auto& pair : *this) scanner.scan(pair);
  }
};

/*
 * value_map Wraps F14ValueMap. Only safe when you don't care when values move
 * on rehash or erase, and don't care about having contiguous values, like
 * F14VectorMap (below). This map favors hash lookups over iteration, and
 * small cheap-to-move elements. Over-reserving in this table can potentially
 * waste memory proportional to the value size, since values are stored in
 * hashtable chunks with no indirection.
 */
template <class T,
          class U,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct value_map : folly::F14ValueMap<
  T, U, V, W,
  ConservativeAllocator<std::pair<const T,U>>
> {
  using Base = folly::F14ValueMap<
    T, U, V, W, ConservativeAllocator<std::pair<const T, U>>
  >;
  using value_type = typename Base::value_type;
  value_map() : Base() {}

  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T, U) {
    Base::visitContiguousRanges(
      [&](value_type const* start, value_type const* end) {
        scanner.scan(*start, (const char*)end - (const char*)start);
      });
  }
};

/*
 * vector_map Wraps F14VectorMap, which store key+value contiguously,
 * and maintains insertion order, providing erase() is not called.
 * iterator walks in LIFO order, reverse_iterator walks in FIFO order.
 * The LIFO order of iterator is efficient when using this pattern:
 *   while (!m.empty()) {
 *     auto it = map.begin();
 *     ...
 *     map.erase(it);
 *   }
 *
 * This map favors iteration and value locality at the cost of some
 * indirection on hash lookups.
 */
template <class T,
          class U,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct vector_map : folly::F14VectorMap<
  T, U, V, W,
  ConservativeAllocator<std::pair<const T,U>>
> {
  using Base = folly::F14VectorMap<
    T, U, V, W, ConservativeAllocator<std::pair<const T, U>>
  >;
  using value_type = typename Base::value_type;
  vector_map() : Base() {}

  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T, U) {
    Base::visitContiguousRanges(
      [&](value_type const* start, value_type const* end) {
        scanner.scan(*start, (const char*)end - (const char*)start);
      });
  }
};

/*
 * fast_map chooses either value_map or vector_map, depending on value size,
 * using the same policy as F14FastMap
 *
 * Not using F14FastMap directly allows us to further customize req::value_map
 * and req::vector_map above without re-implementing the same customizations
 * in our own wrapper for F14FastMap.
 */
template <class Key, class Mapped,
          class Hasher = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
struct fast_map : std::conditional_t<
                       sizeof(std::pair<Key const, Mapped>) < 24,
                       value_map<Key, Mapped, Hasher, KeyEqual>,
                       vector_map<Key, Mapped, Hasher, KeyEqual>> {
  using Super = std::conditional_t<
      sizeof(std::pair<Key const, Mapped>) < 24,
      value_map<Key, Mapped, Hasher, KeyEqual>,
      vector_map<Key, Mapped, Hasher, KeyEqual>>;

  using Super::Super;
  fast_map() : Super() {}
};

}}
#endif
