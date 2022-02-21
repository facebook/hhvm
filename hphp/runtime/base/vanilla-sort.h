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

#ifndef incl_HPHP_PACKED_SORT_H_
#define incl_HPHP_PACKED_SORT_H_

#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/vanilla-vec.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

// VanillaLvalRef is the reference_type for our layout-agnostic array iterator.
// We'll get a copy of this type whenever we dereference the iterator. It needs
// to mimic certain "reference-like behavior":
//
//  *iter1 = *iter2     => copy the referenced data from *iter2 to *iter1
//  *iter1 = value_type => copy the raw value data from the value to *iter1
//
// VanillaLvalRef has no copy constructor because it doesn't make sense to
// "copy" a ref. All refs are created by de-referencing a VanillaLvalIterator.
//
// NOTE: We could implement these methods on tv_lval directly, but the behavior
// would be pretty counter-intuitive! lval1 = lval2 would actually change the
// data pointed to by lval1. Instead, we subclass tv_lval so we only get the
// behavior when using this one iterator type in the sorting context.
//
struct VanillaLvalRef : tv_lval {
  /* implicit */ VanillaLvalRef(tv_val lval): tv_lval(lval) {}
  VanillaLvalRef(const VanillaLvalRef& x) = delete;
  ALWAYS_INLINE void operator=(const VanillaLvalRef& x) { tvCopy(*x, *this); }
  ALWAYS_INLINE void operator=(const TypedValue& x) { tvCopy(x, *this); }
};

// VanillaLval is the value_type for our layout-agnostic array iterator.
// As a result, it needs to save the data at the given reference type:
//
// value_type = *iter => invoke VanillaLval(const tv_val&)
//
struct VanillaLval : TypedValue {
  ALWAYS_INLINE explicit VanillaLval(const tv_lval& x) : TypedValue(*x) {}
};

ALWAYS_INLINE void swap(VanillaLvalRef a, VanillaLvalRef b) {
  tvSwap(a, b);
}

///////////////////////////////////////////////////////////////////////////////

// VanillaLvalIterator is our layout-agnostic array iterator. It has a very
// simple and generic implementation: it stores (ArrayData* ad, int64_t index).
//
// Of course, this implementation is somewhat problematic for performance.
// sizeof(VanillaLvalIterator) is 16; using a 32-bit index type won't help.
// Dereferencing this iterator requires several arithmetic operations...
//
// To make HPHP::Sort::sort work, this iterator needs support a few operators:
//
//  iter1 - iter2           => compute distance (in #elements) between iters
//  iter1 == iter2          => compare iter indices
//  iter1 <= iter2          => compare iter indices
//  iter++, iter--          => update the iter's index
//  iter += difference_type => update the iter's index
//  reference_type = *iter1 => return an lval as a "layout-agnostic reference"
//
// For intuition for these operations, consider what a pointer would support.
//
struct VanillaLvalIterator {
  typedef std::random_access_iterator_tag iterator_category;
  typedef VanillaLval value_type;
  typedef int64_t difference_type;
  typedef VanillaLvalRef pointer;
  typedef VanillaLvalRef reference;

  reference operator*() const {
    assertx(0 <= m_index && m_index < m_arr->size());
    return VanillaLvalRef { VanillaVec::LvalUncheckedInt(m_arr, m_index) };
  }

  bool operator!=(const VanillaLvalIterator& other) const {
    assertx(m_arr == other.m_arr);
    return m_index != other.m_index;
  }

  bool operator==(const VanillaLvalIterator& other) const {
    assertx(m_arr == other.m_arr);
    return m_index == other.m_index;
  }

  bool operator<=(const VanillaLvalIterator& other) const {
    assertx(m_arr == other.m_arr);
    return m_index <= other.m_index;
  }

  VanillaLvalIterator& operator++() {
    m_index++;
    return *this;
  }

  VanillaLvalIterator& operator--() {
    m_index--;
    return *this;
  }

  difference_type operator-(const VanillaLvalIterator& other) const {
    assertx(m_index >= other.m_index);
    return m_index - other.m_index;
  }

  VanillaLvalIterator operator+(difference_type other) const {
    return VanillaLvalIterator { m_arr, m_index + other };
  }

  VanillaLvalIterator operator-(difference_type other) const {
    return VanillaLvalIterator { m_arr, m_index - other };
  }

  ArrayData* m_arr;
  int64_t m_index;
};

// VanillaLvalAccessor is a simple type used by HPHP::Sort::sort to implement
// array access. It operators on the "const" version of the reference type,
// so we can pass VanillaLvalRef to it transparently.
//
// We could use this accessor for more than we currently do! In particular, we
// could make the sort function use an accessor rather than dereferencing the
// iterator directly. That would let us move ArrayData* from the iterator type
// into the accessor (and would not affect pure-pointer iterators).
struct VanillaLvalAccessor {
  typedef tv_rval ElmT;
  bool isInt(ElmT elm) const { return type(elm) == KindOfInt64; }
  bool isStr(ElmT elm) const { return isStringType(type(elm)); }
  int64_t getInt(ElmT elm) const { return val(elm).num; }
  StringData* getStr(ElmT elm) const { return val(elm).pstr; }
  Variant getValue(ElmT elm) const { auto tv = *elm; return tvAsCVarRef(&tv); }
};

///////////////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_SORT_HELPERS_H_
