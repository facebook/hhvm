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

#include "hphp/runtime/base/tv-val.h"
#include "hphp/util/type-traits.h"

#include "folly/Range.h"

#include <type_traits>

namespace HPHP {

void tvDecRefGen(TypedValue);
void tvDecRefCountable(TypedValue);

namespace tv_layout {

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
 * - quick_index: some type, implicitly coercible to index_t, designed to allow
 *                faster access to the collection. This may be the same as
 *                index_t
 *
 * Static members:
 * size_t max_index
 * The maximum index in the container accessible both by index_t and
 * quick_index
 *
 * Static functions:
 * size_t size_for(index_t size);
 * Produces the size in bytes to be allocated for a container of the given size.
 * This must be aligned to a multiple of 16 as there's several optimizations
 * that rely on this fact.
 *
 * quick_index quickIndex(index_t idx);
 * Produce the quick index corresponding to the given index
 *
 * Member functions:
 * void init(index_t size); Establishes any invariants the container needs to
 * operatate, for a container of the given size
 *
 * tv_val_offset offsetOf(index_t idx) const;
 * tv_val_offset offsetOf(quick_index idx) const; (optional)
 * Produces a tv_val_offset to the given index's typed value.
 *
 * void checkInvariants(index_t size) const;
 * Asserts if any invariant of the container is not met
 *
 * void scan(quick_index size, type_scan::Scanner&) const;
 * Scans the countable values in the container
 *
 * void release(quick_index size);
 * Decrefs the countable values in the container
 *
 * iterator iteratorAt(index_t);
 * const_iterator iteratorAt(index_t);
 * Produce an iterator starting at the given index
 */

template <typename Impl,
          typename Iterator,
          typename ConstIterator,
          typename Index,
          typename QuickIndex>
struct LayoutBase {
  using iterator = Iterator;
  using const_iterator = ConstIterator;

  using index_t = Index;
  using quick_index = QuickIndex;
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

  template <typename T>
  tv_lval at(T idx) {
    return impl().offsetOf(idx).apply(reinterpret_cast<char*>(this));
  }

  template <typename T>
  tv_rval at(T idx) const {
    return impl().offsetOf(idx).apply(reinterpret_cast<const char*>(this));
  }
};

/* TvArray provides a layout that is identical to an array of TypedValues */
struct TvArray : public LayoutBase<TvArray,
                                   TypedValue*,
                                   const TypedValue*,
                                   uint16_t,
                                   uint16_t> {
  static size_t constexpr max_index = std::numeric_limits<index_t>::max();

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
  void init(index_t size) {}

  iterator iteratorAt(index_t offset) {
    return &rep()[offset];
  }

  const_iterator iteratorAt(index_t offset) const {
    return &rep()[offset];
  }

  static quick_index quickIndex(index_t idx) {
    return idx;
  }

  static index_t offset2Idx(size_t offset) {
    return offset / sizeof(TypedValue);
  }

  void scan(index_t count, type_scan::Scanner& scanner) const {
    scanner.scan(*rep(), count * sizeof(TypedValue));
  }

  void release(index_t count) {
    for (auto lval : range(0, count)) {
      tvDecRefGen(lval);
    }
  }

  bool checkInvariants(index_t /*len*/) const {
    return true;
  }
};

////////////////////////////////////////////////////////////////////////////////

/*
 * 7-up packed layout
 *
 * This implements a flavor of an array layout but instead of wasting space on
 * padding for the type byte (out to a whole quardowrd), we aggregate 7 of these
 * types together (and then aggregate 7 values together) resulting in a
 * repeating layout (a "chunk") of 64 bytes. The layout is like:
 *
 *  ______________________ 8 bytes _______________________
 * /                                                      \
 * +------+------+------+------+------+------+------+------+ \
 * |  t1  |  t2  |  t3  |  t4  |  t5  |  t6  |  t7  |      |  |
 * +------+------+------+------+------+------+------+------+  |
 * |                           v1                          |  |
 * +-------------------------------------------------------+  | 64 bytes
 * |                           ...                         |  | (a "chunk")
 * +-------------------------------------------------------+  |
 * |                           v7                          |  |
 * +------+------+------+------+------+------+------+------+ /
 * |  t8  |  t9  |  t10 | 0x00 | 0x00 | 0x00 | 0x00 |      |
 * +------+------+------+------+------+------+------+------+
 *                         ^
 *                         |
 *                          \ trailing type bytes are required to be zero
 *                            except for the last which is always unconstrained
 *
 * Iterating over the 7-up layout efficiently requires that the container be
 * aligned to a 16-byte boundary, and checkInvariants ensures this is the case
 *
 * Yet another invariant of the 7-up layout is that any additional types in the
 * **last** chunk (which might have 1-7 valid types) must be initialized to
 * KindOfUninit. The 8th byte of the type word is always allowed to contain any
 * value. This allows us to unroll both type scanning and release in a way that
 * prevents having treat the last, possibly incomplete, type word in a special
 * way.
 */

namespace detail_7up {

template <bool is_const>
struct iterator_impl {
  using tv_val_t = tv_val<is_const>;

  using char_t = typename std::conditional<is_const, const char, char>::type;
  using datatype_t = typename tv_val_t::type_t;
  using value_t = typename tv_val_t::value_t;

  using value_type = TypedValue;
  using reference = TypedValue&;
  using pointer = void;
  using difference_type = ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  iterator_impl(datatype_t* type, value_t* value)
    : m_type(type)
    , m_value(value) {}

  TypedValue operator*() const {
    return TypedValue{*m_value, *m_type};
  }

  operator tv_val_t() const {
    return {m_type, m_value};
  }

  iterator_impl& operator++() {
    /* This is the nasty trick alluded to above--we rely on the alignment of
     * the entire container to detect if we're at the end of a chunk by
     * inspecting the least significant 3 bits of the type pointer. */
    auto const carry = ((reinterpret_cast<uintptr_t>(m_type) + 2) >> 3) & 1;

    m_type += 1 + (carry * (sizeof(Value) * 7 + sizeof(DataType)));
    m_value += 1 + carry;
    return *this;
  }

  iterator_impl operator++(int) {
    auto const ret = *this;
    ++(*this);
    return ret;
  }

  bool operator==(const iterator_impl& other) const {
    assertx(IMPLIES(m_type == other.m_type,
                    m_value == other.m_value));
    return m_type == other.m_type;
  }

  bool operator!=(const iterator_impl& other) const {
    return !(*this == other);
  }

private:
  datatype_t* m_type;
  value_t* m_value;
};

struct quick_index {
  uint8_t rem : 3;
  uint16_t quot : 13;

  operator uint16_t() const {
    return quot * 7 + rem;
  }
};

} // namespace detail_7up

struct Tv7Up : public LayoutBase<Tv7Up,
                                 detail_7up::iterator_impl<false>,
                                 detail_7up::iterator_impl<true>,
                                 uint16_t,
                                 detail_7up::quick_index> {
  /* see quick_index above for why this is the maximum index */
  static size_t constexpr max_index = ((1 << 13) - 1) * 7;

  void init(index_t size) {
    if (size == 0) return;
    auto const lastTypeWord =
      reinterpret_cast<uint64_t*>(this) + sizeof(Value) * ((size - 1) / 7);
    *lastTypeWord = uint64_t{0};
  }

  bool checkInvariants(index_t size) const {
    assertx(reinterpret_cast<uintptr_t>(this) % 16 == 0);
    /* we require any of the first bytes in the type word that do not correspond
     * to types in the container (i.e. if the last type word only has 4 types,
     * the next 3 bytes are required to be zero.) */
    DEBUG_ONLY auto const checkTypeByte = [&]{
      auto const chunks = size / 7;
      auto const rem = size % 7;
      auto type = reinterpret_cast<const char*>(this) +
        8 * sizeof(Value) * chunks;

      auto const last_type_byte = reinterpret_cast<const uint64_t*>(type);

      type += rem;
      SCOPE_ASSERT_DETAIL("type bytes") {
        return folly::sformat("{:08x}", *last_type_byte);
      };

      switch (rem) {
      case 1: assertx(*type++ == 0);
      case 2: assertx(*type++ == 0);
      case 3: assertx(*type++ == 0);
      case 4: assertx(*type++ == 0);
      case 5: assertx(*type++ == 0);
      case 6: assertx(*type++ == 0);
      case 0:
        break;
      default:
        assertx(false);
      };

      return true;
    };
    assertx(checkTypeByte());
    return true;
  }

  static size_t constexpr sizeFor(index_t len) {
    /* We need these sizes fixed here since we underestimate * the size required
     * for the 0-6 elements in the last (incomplete) chunk and rely on alignment
     * to make the buffer big enough.
     *
     * This works in this case since:
     *
     * extra |  size estimate: extra * 9  | aligned estimate | ground truth
     *   0   |             0              |        0         |     0
     *   1   |             9              |        16        |     16
     *   2   |             18             |        32        |     24
     *   3   |             27             |        32        |     32
     *   4   |             36             |        48        |     40
     *   5   |             45             |        48        |     48
     *   6   |             54             |        64        |     56
     *   7   |             63             |        64        |     64       */
    static_assert(sizeof(DataType) == 1, "");
    static_assert(sizeof(Value) == 8, "");

    auto const chunks = len / 7;
    auto const extra = len % 7;

    auto const chunkSize = 8 * sizeof(Value);

    auto const ret = chunks * chunkSize +
      extra * (sizeof(DataType) + sizeof(Value)); // <- the underestimate

    constexpr auto const mask = 16 - 1;
    return (ret + mask) & ~mask;
  }

  static tv_val_offset offsetOf(index_t idx) {
    auto const chunk = idx / 7;
    auto const slot = idx % 7;

    static_assert(8 * sizeof(DataType) == sizeof(Value), "");
    auto const chunkStart = 8 * sizeof(Value) * chunk;

    return {
      static_cast<ptrdiff_t>(chunkStart + slot),
      static_cast<ptrdiff_t>(chunkStart + sizeof(Value) * (1 + slot))
    };
  }

  static tv_val_offset offsetOf(quick_index idx) {
    static_assert(8 * sizeof(DataType) == sizeof(Value), "");
    auto const chunkStart = 8 * sizeof(Value) * idx.quot;

    return {
      static_cast<ptrdiff_t>(chunkStart + idx.rem),
      static_cast<ptrdiff_t>(chunkStart + sizeof(Value) * (1 + idx.rem))
    };
  }

  static quick_index quickIndex(size_t idx) {
    quick_index ret;
    ret.quot = idx / 7;
    ret.rem = idx % 7;
    return ret;
  }

  static index_t offset2Idx(size_t offset) {
    auto off = offset / sizeof(Value);
    return off - (off / 8) - 1;
  }

  iterator iteratorAt(index_t pos) {
    auto const lval = at(pos);
    return iterator{&lval.type(), &lval.val()};
  }

  const_iterator iteratorAt(index_t pos) const {
    auto const rval = at(pos);
    return const_iterator{&rval.type(), &rval.val()};
  }

  void scan(index_t count, type_scan::Scanner& scanner) const = delete;
  void scan(quick_index qi, type_scan::Scanner& scanner) const {
    // round the bottom 3 bits up-- qidx is now (size + 6 / 7) * 8
    auto const off = ((qi.quot << 3) + qi.rem + 0x07) & ~0x07;
    auto const last_type_word = reinterpret_cast<const uint64_t*>(this) + off;

    for (auto types = reinterpret_cast<const uint64_t*>(this);
         types != last_type_word;
         types += 8) {
      auto ts = *types;
      const Value* val = reinterpret_cast<const Value*>(types) + 1;

      if (ts & kRefCountedBit) scanner.scan(val->pcnt);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) scanner.scan(val->pcnt);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) scanner.scan(val->pcnt);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) scanner.scan(val->pcnt);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) scanner.scan(val->pcnt);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) scanner.scan(val->pcnt);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) scanner.scan(val->pcnt);
    }
  }

  void release(index_t count) = delete;
  void release(quick_index count) {
    // round the bottom 3 bits up-- qidx is now (size + 6 / 7) * 8
    auto const qidx = ((count.quot << 3) + count.rem + 0x07) & ~0x07;
    auto const last_type_word = reinterpret_cast<uint64_t*>(this) + qidx;

    for (auto types = reinterpret_cast<uint64_t*>(this);
         types != last_type_word;
         types += 8) {
      auto ts = *types;
      Value* val = reinterpret_cast<Value*>(types) + 1;

      auto const decRef = [](uint64_t ts, Value* val) {
        tvDecRefCountable(TypedValue{*val, static_cast<DataType>(ts)});
      };

      if (ts & kRefCountedBit) decRef(ts, val);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) decRef(ts, val);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) decRef(ts, val);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) decRef(ts, val);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) decRef(ts, val);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) decRef(ts, val);
      ts = ts >> 8; val++;
      if (ts & kRefCountedBit) decRef(ts, val);
    }
  }
};

} // namespace tv_layout
} // namespace HPHP

