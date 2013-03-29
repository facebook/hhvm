/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __INSIDE_HPHP_COMPLEX_TYPES_H__
#error Directly including 'ref_data.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#ifndef __HPHP_REF_DATA_H
#define __HPHP_REF_DATA_H

namespace HPHP {

/**
 * We heap allocate a RefData when we make a reference to something.
 * A Variant or TypedValue can be KindOfRef and point to a RefData,
 * but the value held here must not be KindOfRef.
 */
class RefData {
  enum Magic : uint64_t { kMagic = 0xfacefaceb00cb00c };
public:
  enum NullInit { nullinit };
  RefData() { assert(m_magic = kMagic); }
  RefData(NullInit) {
    assert(m_magic = kMagic);
    _count = 1;
    m_tv.m_type = KindOfNull;
  }
  RefData(DataType t, int64_t datum) {
    assert(m_magic = kMagic);
    init(t, datum);
  }
  ~RefData();

  // Don't extend Countable but use these methods to directly
  // update _count, declared below.
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC

  // Memory allocator methods
  DECLARE_SMART_ALLOCATION(RefData);
  void dump() const;

  const TypedValue* tv() const {
    assert(m_magic == kMagic);
    return &m_tv;
  }
  TypedValue* tv() {
    assert(m_magic == kMagic);
    return &m_tv;
  }
  const Variant* var() const { return (const Variant*)tv(); }
  Variant* var() { return reinterpret_cast<Variant*>(tv()); }

  static constexpr size_t tvOffset() { return offsetof(RefData, m_tv); }

  void assertValid() const {
    assert(m_magic == kMagic);
  }

  // TODO: t2221110: get rid of this hack.
  static RefData* refDataFromVariantIfYouDare(const Variant* var) {
    RefData* ref = reinterpret_cast<RefData*>(uintptr_t(var) - tvOffset());
    ref->assertValid();
    return ref;
  }

private:
  // initialize this value by laundering uninitNull -> Null
  void init(DataType t, int64_t datum) {
    _count = 1;
    if (!IS_NULL_TYPE(t)) {
      m_tv.m_type = t;
      m_tv.m_data.num = datum;
    } else {
      m_tv.m_type = KindOfNull;
    }
  }

  static void compileTimeAsserts() {
    static_assert(offsetof(RefData, _count) == FAST_REFCOUNT_OFFSET, "");
  }

#if defined(DEBUG) || defined(PACKED_TV)
// don't overlap TypedValue with _count.  sizeof(*this) = 32.
private: Magic m_magic;
public:  mutable int32_t _count;
private: TypedValue m_tv;
  static void layoutAsserts() {
    static_assert(offsetof(RefData, m_tv) >
                  offsetof(RefData, _count) + sizeof(int32_t), "");
  };
#else
// overlap TypedValue with count
private:
  union {
    struct {
      void* _ptr;
      mutable int32_t _count;
    };
    TypedValue m_tv;
  };
  static void layoutAsserts() {
    static_assert(offsetof(RefData, _count) == TypedValueAux::auxOffset, "");
    static_assert(sizeof(RefData::_count) == TypedValueAux::auxSize, "");
  }
#endif
};

ALWAYS_INLINE inline void decRefRef(RefData* ref) {
  if (ref->decRefCount() == 0) ref->release();
}

} // namespace HPHP

#endif //__HPHP_REF_DATA_H
