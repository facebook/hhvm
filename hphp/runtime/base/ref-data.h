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

#ifndef incl_HPHP_REF_DATA_H
#define incl_HPHP_REF_DATA_H

#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/member-val.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

struct Variant;

struct RefBits {
  // only need 1 bit each for cow and z, but filling out the bitfield
  // and assigning all field members at the same time causes causes
  // gcc and clang to coalesce mutations into byte-sized ops.
  union {
    uint16_t bits;
    struct {
      mutable uint8_t cow:1;
      mutable uint8_t z:7;
    };
  };
  explicit operator uint16_t() const { return bits; }
};

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
 * this. As a consequence, the count field is not the "real"
 * refcount of a RefData - instead the real refcount can be computed
 * by calling getRealCount() (which simply adds the count and cow
 * fields together). When the count field is decremented to 0, the
 * release() method gets called. This will either free the RefData
 * (if the "real" refcount has reached 0) or it will update the
 * count and cow fields appropriately.
 *
 * For more info on the PHP extension compatibility layer, check out
 * the documentation at "doc/php.extension.compat.layer".
 */
struct RefData final : Countable, type_scan::MarkScannableCountable<RefData> {
  /*
   * Some RefData's (static locals) are allocated in RDS, and
   * live until the end of the request.  In this case, we start with a
   * reference count to keep it alive.
   *
   * Note that the JIT accesses RDS RefDatas directly---if you need to
   * change how initialization works it keep that up to date.
   */
  void initInRDS() {
    // cow=z=0, count=InitialValue
    initHeader_16(HeaderKind::Ref, InitialValue, 0);
  }

  /*
   * Create a RefData, allocated in the request local heap.
   */
  static RefData* Make(TypedValue tv) {
    return new (MM().mallocSmallSize(sizeof(RefData)))
      RefData(tv.m_type, tv.m_data.num);
  }

  ~RefData();

  /*
   * Deallocate a RefData.
   */
  void release() noexcept {
    assert(kindIsValid());
    auto& bits = aux16<RefBits>();
    if (one_bit_refcount) {
      assertx(bits.cow == 0);
      assertx(bits.z == 0);
    } else if (UNLIKELY(bits.cow)) {
      m_count = static_cast<RefCount>(1);
      bits.cow = bits.z = 0;
      AARCH64_WALKABLE_FRAME();
      return;
    }
    this->~RefData();
    MM().freeSmallSize(this, sizeof(RefData));
    AARCH64_WALKABLE_FRAME();
  }

  void releaseMem() const {
    MM().freeSmallSize(const_cast<RefData*>(this), sizeof(RefData));
  }

  ALWAYS_INLINE void decRefAndRelease() {
    assert(kindIsValid());
    if (decReleaseCheck()) release();
  }
  bool kindIsValid() const { return m_kind == HeaderKind::Ref; }

  /*
   * Note, despite the name, this can never return a non-Cell.
   */
  const Cell* tv() const {
    assert(kindIsValid());
    return &m_tv;
  }
  Cell* tv() {
    assert(kindIsValid());
    return &m_tv;
  }

  const Variant* var() const {
    return reinterpret_cast<const Variant*>(tv());
  }
  Variant* var() {
    return reinterpret_cast<Variant*>(tv());
  }

  static constexpr int tvOffset() { return offsetof(RefData, m_tv); }
  static constexpr int cowZOffset() { return offsetof(RefData, m_aux16); }

  void assertValid() const { assert(kindIsValid()); }

  int32_t getRealCount() const {
    auto bits = aux16<RefBits>();
    assert(kindIsValid());
    assert(bits.cow == 0 || (bits.cow == 1 && m_count >= 1));
    return m_count + bits.cow;
  }

  bool isReferenced() const {
    assert(kindIsValid());
    auto bits = aux16<RefBits>();
    assert(bits.cow == 0 || (bits.cow == 1 && m_count >= 1));
    return m_count >= 2 && !bits.cow;
  }

  /**
   * APIs to support the Zend emulation layer
   */

  // Default constructor, provided so that the PHP extension compatibility
  // layer can stack-allocate RefDatas when needed
  RefData() {
    // cow=z=0, count=0
    initHeader_16(HeaderKind::Ref, static_cast<RefCount>(0), 0);
    m_tv.m_type = KindOfNull;
    assert(!aux16<RefBits>().cow && !aux16<RefBits>().z);
  }

  bool zIsRef() const {
    always_assert(!one_bit_refcount);
    assert(kindIsValid());
    auto bits = aux16<RefBits>();
    assert(bits.cow == 0 || (bits.cow == 1 && m_count >= 1));
    return !bits.cow && (m_count >= 2 || bits.z);
  }

  void zSetIsRef() const {
    always_assert(!one_bit_refcount);
    auto& bits = aux16<RefBits>();
    auto realCount = getRealCount();
    if (realCount >= 2) {
      m_count = static_cast<RefCount>(realCount);
      bits.cow = bits.z = 0;
    } else {
      assert(!bits.cow);
      bits.cow = 0;
      bits.z = 1;
    }
  }

  void zUnsetIsRef() const {
    always_assert(!one_bit_refcount);
    auto& bits = aux16<RefBits>();
    auto realCount = getRealCount();
    if (realCount >= 2) {
      m_count = static_cast<RefCount>(realCount - 1);
      bits.cow = 1;
      bits.z = 0;
    } else {
      assert(!bits.cow);
      bits.cow = bits.z = 0;
    }
  }

  void zSetIsRefTo(int val) const {
    always_assert(!one_bit_refcount);
    if (val) {
      zSetIsRef();
    } else {
      zUnsetIsRef();
    }
  }

  int32_t zRefcount() {
    always_assert(!one_bit_refcount);
    return getRealCount();
  }

  void zAddRef() {
    always_assert(!one_bit_refcount);
    if (getRealCount() != 1) {
      ++m_count;
      return;
    }
    auto& bits = aux16<RefBits>();
    assert(!bits.cow);
    assert(bits.z < 2);
    m_count = static_cast<RefCount>(bits.z + 1);
    bits.cow = !bits.z;
    bits.z = 0;
  }

  void zDelRef() {
    always_assert(!one_bit_refcount);
    if (getRealCount() != 2) {
      assert(getRealCount() != 0);
      --m_count;
      return;
    }
    auto& bits = aux16<RefBits>();
    m_count = static_cast<RefCount>(1);
    bits.z = !bits.cow;
    bits.cow = 0;
  }

  void zSetRefcount(int val) {
    always_assert(!one_bit_refcount);
    assert(kindIsValid());
    if (val < 0) {
      val = 0;
    }
    bool zeroOrOne = (val <= 1);
    bool isRef = zIsRef();
    auto& bits = aux16<RefBits>();
    bits.cow = !zeroOrOne && !isRef;
    bits.z = zeroOrOne && isRef;
    m_count = static_cast<RefCount>(val - bits.cow);
    assert(zRefcount() == val);
    assert(zIsRef() == isRef);
  }

  void zInit() {
    always_assert(!one_bit_refcount);
    auto& bits = aux16<RefBits>();
    m_count = static_cast<RefCount>(1);
    bits.cow = bits.z = 0;
    m_tv.m_type = KindOfNull;
    m_tv.m_data.num = 0;
  }

private:
  RefData(DataType t, int64_t datum) {
    // Initialize this value by laundering uninitNull -> Null.
    // cow=z=0, count=InitialValue
    initHeader_16(HeaderKind::Ref, InitialValue, 0);
    if (!isNullType(t)) {
      m_tv.m_type = t;
      m_tv.m_data.num = datum;
    } else {
      m_tv.m_type = KindOfNull;
    }
  }

private:
  TypedValue m_tv;
};

ALWAYS_INLINE void decRefRef(RefData* ref) {
  ref->decRefAndRelease();
}

} // namespace HPHP

#endif //incl_HPHP_REF_DATA_H
