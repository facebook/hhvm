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

#ifndef incl_HPHP_TV_LAYOUT_H
#define incl_HPHP_TV_LAYOUT_H

#include "hphp/runtime/base/tv-val.h"
#include "hphp/util/type-traits.h"

#include "folly/Range.h"

#include <type_traits>

namespace HPHP { namespace tv_layout {

/* A TV layout represents a integer-indexed aggregate of TypedValues.
 *
 * The intended way to use one is to reinterpret_cast a region of memory
 * to the appropriate tv_layout type.
 *
 * The type must implement the following interface and extend LayoutBase:
 *
 * Member types:
 * - index_t: some integral type used to index the container
 * - iterator, const_iterator: a (const) forward iterator over the container,
 *                             producing TypedValues with the additional
 *                             constraint that tv_lval{iter} is well-formed as
 *                             long as the iterator is derefenecable
 *
 * Static functions:
 * size_t size_for(index_t size);
 * Produces the size in bytes to be allocated for a container of the given size.
 * This must be aligned to a multiple of 16 as there's several optimizations
 * that rely on this fact.
 *
 * Member functions:
 * tv_val_offset offsetOf(index_t idx) const;
 * Produces a tv_val_offset to the given index's typed value.
 *
 * void checkInvariants(index_t size) const;
 * Asserts if any invariant of the container is not met
 *
 * void scan(index_t size, type_scan::Scanner&) const;
 * Scans the countable values in the container
 *
 * iterator iteratorAt(index_t);
 * const_iterator iteratorAt(index_t);
 * Produce an iterator starting at the given index
 */

template <typename Impl,
          typename Iterator,
          typename ConstIterator,
          typename Index>
struct LayoutBase {
  using iterator = Iterator;
  using const_iterator = ConstIterator;
  using index_t = Index;
  static_assert(std::is_integral<index_t>::value, "");

  Impl& impl() { return *static_cast<Impl*>(this); }
  const Impl& impl() const { return *static_cast<const Impl*>(this); }

  /*
   * Produce a range over the conainer, for use in for-each loops
   */
  folly::Range<iterator>
  range(index_t begin, index_t end) {
    return folly::Range<iterator>{
      impl().iteratorAt(begin), impl().iteratorAt(end)
    };
  }

  folly::Range<const_iterator>
  range(index_t begin, index_t end) const {
    return folly::Range<const_iterator>{
      impl().iteratorAt(begin), impl().iteratorAt(end)
    };
  }

  /*
   * Iterate over the container _by tv_val_ as opposed to by TypedValue
   */
  template <typename Iter>
  std::enable_if_t<is_invocable<Iter, tv_lval>::value>
  foreach(index_t len, Iter&& iter) {
    auto it = impl().iteratorAt(0);
    auto end = impl().iteratorAt(len);
    while (it != end) {
      iter(tv_lval{it});
      ++it;
    }
  }

  template <typename Iter>
  std::enable_if_t<is_invocable<Iter, tv_rval>::value>
  foreach(index_t len, Iter&& iter) const {
    auto it = impl().iteratorAt(0);
    auto end = impl().iteratorAt(len);
    while (it != end) {
      iter(tv_rval{it});
      ++it;
    }
  }

  tv_lval at(index_t idx) {
    return impl().offsetOf(idx).apply(reinterpret_cast<char*>(this));
  }

  tv_rval at(index_t idx) const {
    return impl().offsetOf(idx).apply(reinterpret_cast<const char*>(this));
  }
};

/* TvArray provides a layout that is identical to an array of TypedValues */
struct TvArray : public LayoutBase<TvArray,
                                   TypedValue*,
                                   const TypedValue*,
                                   uint16_t> {
  static size_t sizeFor(index_t len) {
    return len * sizeof(TypedValue);
  }

  static tv_val_offset offsetOf(index_t idx) {
    auto const base = safe_cast<ptrdiff_t>(sizeof(TypedValue) * idx);
    static_assert(offsetof(TypedValue, m_data) == 0, "");
    return {
      base + static_cast<ptrdiff_t>(offsetof(TypedValue, m_type)),
      base
    };
  }

private:
  TypedValue* rep() {
    return reinterpret_cast<TypedValue*>(this);
  }

  const TypedValue* rep() const {
    return const_cast<TvArray*>(this)->rep();
  }

public:
  iterator iteratorAt(index_t offset) {
    return &rep()[offset];
  }

  const_iterator iteratorAt(index_t offset) const {
    return &rep()[offset];
  }

  void scan(index_t count, type_scan::Scanner& scanner) const {
    scanner.scan(*rep(), count * sizeof(TypedValue));
  }

  bool checkInvariants(index_t /*len*/) const {
    return true;
  }
};

} // namespace tv_layout
} // namespace HPHP

#endif // incl_HPHP_TV_LAYOUT_H
