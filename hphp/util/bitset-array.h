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

#include <hphp/util/assertions.h>

namespace HPHP {

struct BitsetArray;
struct BitsetWrapper;

struct BitsetRef {
  friend struct BitsetArray;
  friend struct BitsetWrapper;

  const BitsetRef& operator=(const BitsetRef&) = delete;

  size_t size() const { return (e - s) * bits_per_word; }

  friend bool operator==(const BitsetRef& o1, const BitsetRef& o2) {
    auto p1 = o1.s;
    auto p2 = o2.s;
    while (p1 != o1.e) {
      if (*p1 != *p2) return false;
      ++p1;
      ++p2;
    }
    assert(p2 == o2.e);
    return true;
  }

  friend bool operator!=(const BitsetRef& o1, const BitsetRef& o2) {
    return !(o1 == o2);
  }

  friend BitsetWrapper operator|(const BitsetRef& o1, const BitsetRef& o2);
  friend BitsetWrapper operator&(const BitsetRef& o1, const BitsetRef& o2);
  friend BitsetWrapper operator-(const BitsetRef& o1, const BitsetRef& o2);

  BitsetRef& operator|=(const BitsetRef& o) {
    perform(o,
            [](uint64_t* dst, const uint64_t *src) {
              *dst |= *src;
            });
    return *this;
  }
  BitsetRef& operator&=(const BitsetRef& o) {
    perform(o,
            [](uint64_t* dst, const uint64_t *src) {
              *dst &= *src;
            });
    return *this;
  }
  BitsetRef& operator-=(const BitsetRef& o) {
    perform(o,
            [](uint64_t* dst, const uint64_t *src) {
              *dst &= ~*src;
            });
    return *this;
  }

  void assign(const BitsetRef& o) {
    perform(o,
            [](uint64_t* dst, const uint64_t *src) {
              *dst = *src;
            });
  }

  void assign_add(const BitsetRef& o1, const BitsetRef& o2) {
    perform(o1, o2,
            [](uint64_t* dst, const uint64_t *src1, const uint64_t *src2) {
              *dst = *src1 | *src2;
            });
  }

  void assign_sub(const BitsetRef& o1, const BitsetRef& o2) {
    perform(o1, o2,
            [](uint64_t* dst, const uint64_t *src1, const uint64_t *src2) {
              *dst = *src1 & ~*src2;
            });
  }

  void assign_and(const BitsetRef& o1, const BitsetRef& o2) {
    perform(o1, o2,
            [](uint64_t* dst, const uint64_t *src1, const uint64_t *src2) {
              *dst = *src1 & *src2;
            });
  }

  BitsetRef next(int n = 1) const {
    return BitsetRef { s + (e - s) * n, e + (e - s) * n };
  }

  BitsetRef prev(int n = 1) const {
    return BitsetRef { s - (e - s) * n, e - (e - s) * n };
  }

  bool operator[](size_t bit) const { return test(bit); }
  bool test(size_t bit) const {
    auto idx = bit / bits_per_word;
    auto pos = bit - (idx * bits_per_word);

    assert(idx < (e - s));
    return (s[idx] >> pos) & 1;
  }

  void reset(size_t bit) {
    auto idx = bit / bits_per_word;
    auto pos = bit - (idx * bits_per_word);

    assert(idx < (e - s));
    s[idx] &= ~(ElemType{1} << pos);
  }

  void reset() {
    for (auto p = s; p != e; ++p) *p = 0;
  }

  void set(size_t bit) {
    auto idx = bit / bits_per_word;
    auto pos = bit - (idx * bits_per_word);

    assert(idx < (e - s));
    s[idx] |= ElemType{1} << pos;
  }

  void set() {
    for (auto p = s; p != e; ++p) *p = ~ElemType{};
  }

  bool any() const {
    for (auto p = s; p != e; ++p) if (*p) return true;
    return false;
  }

  bool none() const { return !any(); }
private:
  using ElemType = uint64_t;
  static constexpr auto bits_per_word = std::numeric_limits<ElemType>::digits;

  BitsetRef(ElemType* s, ElemType* e) : s{s}, e{e} {}
  size_t words() const { return e - s; }

  template <typename F>
  void perform(const BitsetRef& o, F&& f) {
    auto dst = s;
    auto src = o.s;
    while (dst != e) {
      f(dst, src);
      ++dst;
      ++src;
    }
    assert(src == o.e);
  }

  template <typename F>
  void perform(const BitsetRef& o1, const BitsetRef& o2, F&& f) {
    auto dst = s;
    auto src1 = o1.s;
    auto src2 = o2.s;
    while (dst != e) {
      f(dst, src1, src2);
      ++dst;
      ++src1;
      ++src2;
    }
    assert(src1 == o1.e);
    assert(src2 == o2.e);
  }

  ElemType *s;
  ElemType *e;
};

struct BitsetWrapper : BitsetRef {
  BitsetWrapper(BitsetWrapper&& o) : BitsetRef{o.s, o.e} {
    o.s = o.e = nullptr;
  }
  ~BitsetWrapper() { delete [] s; }

  friend BitsetWrapper operator|(const BitsetRef& o1, const BitsetRef& o2) {
    BitsetWrapper r{o1.words()};
    r.assign_add(o1, o2);
    return r;
  }

  friend BitsetWrapper operator&(const BitsetRef& o1, const BitsetRef& o2) {
    BitsetWrapper r{o1.words()};
    r.assign_and(o1, o2);
    return r;
  }

  friend BitsetWrapper operator-(const BitsetRef& o1, const BitsetRef& o2) {
    BitsetWrapper r{o1.words()};
    r.assign_sub(o1, o2);
    return r;
  }


private:
  BitsetWrapper(size_t words) : BitsetRef{nullptr, nullptr} {
    s = new uint64_t[words];
    e = s + words;
  }
};

struct BitsetArray {
  BitsetArray(size_t rows, size_t bits) : m_rows{rows}, m_rowData{nullptr} {
    m_rowElems =
      (bits + BitsetRef::bits_per_word - 1) / BitsetRef::bits_per_word;
    auto const words = m_rowElems * rows;
    m_rowData = new uint64_t[words];
    memset(m_rowData, 0, words * sizeof(*m_rowData));
  }

  BitsetRef row(size_t r) {
    assert(r < m_rows);
    auto const s = m_rowData + r * m_rowElems;
    return BitsetRef { s, s + m_rowElems };
  }

  const BitsetRef row(size_t r) const {
    assert(r < m_rows);
    auto const s = m_rowData + r * m_rowElems;
    return BitsetRef { s, s + m_rowElems };
  }

  ~BitsetArray() { delete [] m_rowData; }
private:
  size_t m_rows;
  size_t m_rowElems;
  BitsetRef::ElemType* m_rowData;
};

}
