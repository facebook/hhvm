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
 * We heap allocate a RefData when we make a reference to something. A Variant
 * or TypedValue can be KindOfRef and point to a RefData, but the value held
 * here must not be KindOfRef.
 *
 * RefDatas are also used by the PHP extension compatibility layer to represent
 * "zvals". Because zvals can be shared by multiple things "by value", it was
 * necessary to add fields to RefData to support this. As a consequence, the
 * m_count field is not the "real" refcount of a RefData - instead the real
 * refcount can be computed by calling getRealCount() (which simply adds the
 * m_count and m_cow fields together). When the m_count field is decremented
 * to 0, the refdata_after_decref_helper() helper function gets called. This
 * help will either free the RefData (if the "real" refcount has reached 0) or
 * it will update the m_count and m_cow fields appropriately.
 *
 * For more info on the PHP extension compatibility layer, check out the
 * documentation at "doc/php.extension.compat.layer".
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

  int32_t getRealCount() const {
    assert(m_cow == 0 || (m_cow == 1 && m_count >= 1));
    return m_count + m_cow;
  }

  bool isReferenced() const {
    assert(m_cow == 0 || (m_cow == 1 && m_count >= 1));
    return m_count >= 2 && !m_cow;
  }

  /**
   * APIs to support the Zend emulation layer
   */

  // Default constructor, provided so that the PHP extension compatibility
  // layer can stack-allocate RefDatas when needed
  RefData() {
    // intentional use of = to only assign in debug builds
    assert(static_cast<bool>(m_magic = Magic::kMagic));
    m_tv.m_type = KindOfNull;
    m_count = 0;
    m_cowAndZ = 0;
  }

  bool zIsRef() const {
    assert(m_cow == 0 || (m_cow == 1 && m_count >= 1));
    return !m_cow && (m_count >= 2 || m_z);
  }

  void zSetIsRef() const {
    auto realCount = getRealCount();
    if (realCount >= 2) {
      m_count = realCount;
      m_cowAndZ = 0;
    } else {
      assert(!m_cow);
      m_z = 1;
    }
  }

  void zUnsetIsRef() const {
    auto realCount = getRealCount();
    if (realCount >= 2) {
      m_count = realCount - 1;
      m_cow = 1;
      m_z = 0;
    } else {
      assert(!m_cow);
      m_z = 0;
    }
  }

  void zSetIsRefTo(int val) const {
    if (val) {
      zSetIsRef();
    } else {
      zUnsetIsRef();
    }
  }

  int32_t zRefcount() {
    return getRealCount();
  }

  void zAddRef() {
    if (getRealCount() != 1) {
      ++m_count;
      return;
    }
    assert(!m_cow);
    assert(m_z == 0 || m_z == 1);
    m_count = m_z + 1;
    m_cow = !m_z;
    m_z = 0;
  }

  void zDelRef() {
    if (getRealCount() != 2) {
      assert(getRealCount() != 0);
      --m_count;
      return;
    }
    m_count = 1;
    m_z = !m_cow;
    m_cow = 0;
  }

  void zSetRefcount(int val) {
    if (val < 0) {
      val = 0;
    }
    bool zeroOrOne = (val <= 1);
    bool isRef = zIsRef();
    m_cow = !zeroOrOne && !isRef;
    m_z = zeroOrOne && isRef;
    m_count = val - m_cow;
    assert(zRefcount() == val);
    assert(zIsRef() == isRef);
  }

  void zInit() {
    m_count = 1;
    m_cowAndZ = 0;
    m_tv.m_type = KindOfNull;
    m_tv.m_data.num = 0;
  }

private:
  RefData(DataType t, int64_t datum) {
    // intentional use of = to only assign in debug builds
    assert(static_cast<bool>(m_magic = Magic::kMagic));
    // Initialize this value by laundering uninitNull -> Null.
    m_count = 1;
    m_cowAndZ = 0;
    if (!IS_NULL_TYPE(t)) {
      m_tv.m_type = t;
      m_tv.m_data.num = datum;
    } else {
      m_tv.m_type = KindOfNull;
    }
  }

public:
  ~RefData();

private:
  static void compileTimeAsserts() {
    static_assert(offsetof(RefData, m_count) ==
                  FAST_REFCOUNT_OFFSET, "");
    static_assert(sizeof(RefData::m_count) ==
                  TypedValueAux::auxSize, "");
  }

#if defined(DEBUG) || defined(PACKED_TV)
private:
  Magic m_magic;
  UNUSED int32_t m_padding;
public:
  mutable RefCount m_count;
private:
  TypedValue m_tv;
public:
  union {
    struct {
      mutable uint8_t m_cow;
      mutable uint8_t m_z;
    };
    mutable uint32_t m_cowAndZ;
  };
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
  // TODO Task #2893407: At present the m_cow and m_z fields cause the
  // RefData structure to take up more than 16 bytes. If we use int8_t
  // instead of int32_t for TypedValue::m_type and TypedValueAux::m_type,
  // we can fit all of RefData's fields into 16 bytes.
  union {
    struct {
      mutable uint8_t m_cow;
      mutable uint8_t m_z;
    };
    mutable uint32_t m_cowAndZ;
  };
#endif
};

ALWAYS_INLINE void decRefRef(RefData* ref) {
  if (ref->decRefCount() == 0) {
    if (ref->m_cow) {
      ref->m_count = 1;
      ref->m_cowAndZ = 0;
      return;
    }
    ref->release();
  }
}

/**
 * When m_count is decremented and reaches 0, this helper function is called.
 * If the "real" refcount (as given by getRealCount()) is 0 the RefData will
 * be freed, otherwise the RefData's m_count, m_cow, and m_z fields will be
 * updated appropriately.
 */
void refdata_after_decref_helper(RefData* ref);

} // namespace HPHP

#endif //incl_HPHP_REF_DATA_H
