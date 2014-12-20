/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

class Variant;

/*
 * We heap allocate a RefData when we make a reference to something.
 * A Variant or TypedValue can be KindOfRef and point to a RefData,
 * but the value held here must not be KindOfRef.
 *
 * RefData's are also used to implement static locals, but in this
 * case the RefData itself is allocated in RDS rather than on
 * the heap.  Note that generally speaking a RefData should never
 * contain KindOfUninit, *except* uninitialized RefDatas for this
 * RDS case.
 *
 * RefDatas are also used by the PHP extension compatibility layer to
 * represent "zvals". Because zvals can be shared by multiple things
 * "by value", it was necessary to add fields to RefData to support
 * this. As a consequence, the m_count field is not the "real"
 * refcount of a RefData - instead the real refcount can be computed
 * by calling getRealCount() (which simply adds the m_count and m_cow
 * fields together). When the m_count field is decremented to 0, the
 * release() method gets called. This will either free the RefData
 * (if the "real" refcount has reached 0) or it will update the
 * m_count and m_cow fields appropriately.
 *
 * For more info on the PHP extension compatibility layer, check out
 * the documentation at "doc/php.extension.compat.layer".
 */
struct RefData {
  /*
   * Some RefData's (static locals) are allocated in RDS, and
   * live until the end of the request.  In this case, we start with a
   * reference count to keep it alive.
   *
   * Note that the JIT accesses RDS RefDatas directly---if you need to
   * change how initialization works it keep that up to date.
   */
  void initInRDS() {
    assert(isUninitializedInRDS());
    m_kind = HeaderKind::Ref;
    m_count = 1;
    assert(!m_cow && !m_z); // because RDS is pre-zeroed
  }

  /*
   * For RefDatas in RDS, we need a way to check if they are
   * initialized while avoiding the usual m_kind assertions (m_kind
   * will be zero if it's not initialized).  This function does that.
   */
  bool isUninitializedInRDS() const {
    return m_tv.m_type == KindOfUninit;
  }

  /*
   * Create a RefData, allocated in the request local heap.
   */
  static RefData* Make(TypedValue tv) {
    return new (MM().smartMallocSizeLogged(sizeof(RefData)))
      RefData(tv.m_type, tv.m_data.num);
  }

  ~RefData();

  /*
   * Deallocate a RefData.
   */
  void release() {
    assert(!hasMultipleRefs());
    if (UNLIKELY(m_cow)) {
      m_count = 1;
      m_cow = m_z = 0;
      return;
    }
    this->~RefData();
    MM().smartFreeSizeLogged(this, sizeof(RefData));
  }

  void releaseMem() const {
    MM().smartFreeSizeLogged(const_cast<RefData*>(this), sizeof(RefData));
  }

  IMPLEMENT_COUNTABLENF_METHODS_NO_STATIC

  /*
   * Note, despite the name, this can never return a non-Cell.
   */
  const Cell* tv() const {
    assert(m_kind == HeaderKind::Ref);
    return &m_tv;
  }
  Cell* tv() {
    assert(m_kind == HeaderKind::Ref);
    return &m_tv;
  }

  const Variant* var() const { return (const Variant*)tv(); }
  Variant* var() { return reinterpret_cast<Variant*>(tv()); }

  static constexpr int tvOffset() { return offsetof(RefData, m_tv); }

  void assertValid() const {
    assert(m_kind == HeaderKind::Ref);
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
  RefData() : m_kind(HeaderKind::Ref) {
    m_tv.m_type = KindOfNull;
    m_count = 0;
    m_cow = m_z = 0;
  }

  bool zIsRef() const {
    assert(m_cow == 0 || (m_cow == 1 && m_count >= 1));
    return !m_cow && (m_count >= 2 || m_z);
  }

  void zSetIsRef() const {
    auto realCount = getRealCount();
    if (realCount >= 2) {
      m_count = realCount;
      m_cow = m_z = 0;
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
    assert(m_z < 2);
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
    m_cow = m_z = 0;
    m_tv.m_type = KindOfNull;
    m_tv.m_data.num = 0;
  }

private:
  RefData(DataType t, int64_t datum) : m_kind(HeaderKind::Ref) {
    // Initialize this value by laundering uninitNull -> Null.
    m_count = 1;
    m_cow = m_z = 0;
    if (!IS_NULL_TYPE(t)) {
      m_tv.m_type = t;
      m_tv.m_data.num = datum;
    } else {
      m_tv.m_type = KindOfNull;
    }
  }

  static void compileTimeAsserts() {
    static_assert(offsetof(RefData, m_kind) == HeaderKindOffset, "");
    static_assert(offsetof(RefData, m_count) == FAST_REFCOUNT_OFFSET, "");
    static_assert(sizeof(RefData::m_count) == TypedValueAux::auxSize, "");
    static_assert(sizeof(DataType) == 1, "required for m_cow/z packing");
  }

private:
#if defined(PACKED_TV)
  UNUSED uint8_t m_padding[9];
  mutable uint8_t m_cow;
  mutable uint8_t m_z;
  HeaderKind m_kind;
  mutable RefCount m_count;
  TypedValue m_tv;
#else
  // count overlaps TypedValue.m_aux
  union {
    TypedValue m_tv;
    struct {
      void* shadow_data;
      DataType shadow_type;
      mutable uint8_t m_cow;
      mutable uint8_t m_z;
      HeaderKind m_kind;
      mutable RefCount m_count; // refcount field
    };
  };
#endif
};

ALWAYS_INLINE void decRefRef(RefData* ref) {
  ref->decRefAndRelease();
}

} // namespace HPHP

#endif //incl_HPHP_REF_DATA_H
