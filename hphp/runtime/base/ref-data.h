/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_REF_DATA_H
#define incl_HPHP_REF_DATA_H

#ifndef incl_HPHP_INSIDE_HPHP_COMPLEX_TYPES_H_
#error Directly including 'ref_data.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

namespace HPHP {

/**
 * We heap allocate a RefData when we make a reference to something.
 * A Variant or TypedValue can be KindOfRef and point to a RefData,
 * but the value held here must not be KindOfRef.
 *
 */
class RefData {
  enum class Magic : uint64_t { kMagic = 0xfacefaceb00cb00c };

public:

  /*
   * Create a RefData, allocated in the request local heap.
   */
  static RefData* Make(DataType t, int64_t datum) {
    return new (MM().smartMallocSize(sizeof(RefData))) RefData(t, datum);
  }

  /*
   * Deallocate a RefData.
   */
  void release() {
    this->~RefData();
    MM().smartFreeSize(this, sizeof(RefData));
  }

  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC

  // Memory allocator methods
  void dump() const;

  const TypedValue* tv() const {
    assert(m_magic == Magic::kMagic);
    return &m_tv;
  }
  TypedValue* tv() {
    assert(m_magic == Magic::kMagic);
    return &m_tv;
  }
  const Variant* var() const { return (const Variant*)tv(); }
  Variant* var() { return reinterpret_cast<Variant*>(tv()); }

  static constexpr size_t tvOffset() { return offsetof(RefData, m_tv); }

  void assertValid() const {
    assert(m_magic == Magic::kMagic);
  }

  // TODO: t2221110: get rid of this hack.
  static RefData* refDataFromVariantIfYouDare(const Variant* var) {
    RefData* ref = reinterpret_cast<RefData*>(uintptr_t(var) - tvOffset());
    ref->assertValid();
    return ref;
  }

private:
  RefData(DataType t, int64_t datum) {
    // intentional use of = to only assign in debug builds
    assert(static_cast<bool>(m_magic = Magic::kMagic));

    // Initialize this value by laundering uninitNull -> Null.
    m_count = 1;
    if (!IS_NULL_TYPE(t)) {
      m_tv.m_type = t;
      m_tv.m_data.num = datum;
    } else {
      m_tv.m_type = KindOfNull;
    }
  }

  ~RefData();

  static void compileTimeAsserts() {
    static_assert(offsetof(RefData, m_count) == FAST_REFCOUNT_OFFSET, "");
    static_assert(sizeof(RefData::m_count) == TypedValueAux::auxSize, "");
  }

#if defined(DEBUG) || defined(PACKED_TV)
private:
  Magic m_magic;
  UNUSED int32_t m_padding;
public:
  mutable RefCount m_count;
private:
  TypedValue m_tv;
#else
// count comes after actual TypedValue, overlapping TypedValue.m_aux
public:
  union {
    TypedValueAux m_tv;
    struct {
      void* shadow_data;
      int32_t shadow_type;
      mutable RefCount m_count; // refcount field
    };
  };
#endif
};

ALWAYS_INLINE void decRefRef(RefData* ref) {
  if (ref->decRefCount() == 0) ref->release();
}

} // namespace HPHP

#endif //incl_HPHP_REF_DATA_H
