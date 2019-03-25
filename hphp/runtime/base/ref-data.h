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
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

struct Variant;

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
 */
struct RefData final : Countable, type_scan::MarkScannableCollectable<RefData> {
  /*
   * Some RefData's (static locals) are allocated in RDS, and
   * live until the end of the request.  In this case, we start with a
   * reference count to keep it alive.
   *
   * Note that the JIT accesses RDS RefDatas directly---if you need to
   * change how initialization works it keep that up to date.
   */
  void initInRDS() {
    // count=OneReference
    initHeader_16(HeaderKind::Ref, OneReference, 0);
  }

  /*
   * Create a RefData, allocated in the request local heap.
   */
  static RefData* Make(Cell v) {
    assertx(cellIsPlausible(v));
    return new (tl_heap->objMalloc(sizeof(RefData)))
      RefData(v.m_type, v.m_data.num);
  }

  ~RefData();

  /*
   * Deallocate a RefData.
   */
  void release() noexcept {
    assertx(kindIsValid());
    this->~RefData();
    tl_heap->objFree(this, sizeof(RefData));
    AARCH64_WALKABLE_FRAME();
  }

  ALWAYS_INLINE void decRefAndRelease() {
    assertx(kindIsValid());
    if (decReleaseCheck()) release();
  }
  bool kindIsValid() const { return m_kind == HeaderKind::Ref; }

  /*
   * This can never return a non-Cell.
   */
  const Cell* cell() const {
    assertx(kindIsValid());
    assertx(cellIsPlausible(m_cell));
    return &m_cell;
  }

  /*
   * This can never return a pointer to non-Cell when valid, but can
   * be used to obtain a pointer for writing, after initInRDS() when
   * m_cell still is uninitialized.
   */
  Cell* cell() {
    assertx(kindIsValid());
    return &m_cell;
  }

  const Variant* var() const {
    return reinterpret_cast<const Variant*>(cell());
  }
  Variant* var() {
    return reinterpret_cast<Variant*>(cell());
  }

  static constexpr int cellOffset() { return offsetof(RefData, m_cell); }

  void assertValid() const { assertx(kindIsValid()); }

  bool isReferenced() const {
    assertx(kindIsValid());
    return hasMultipleRefs();
  }

private:
  RefData(DataType t, int64_t datum) {
    // Initialize this value by laundering uninitNull -> Null.
    // count=OneReference
    initHeader_16(HeaderKind::Ref, OneReference, 0);
    if (!isNullType(t)) {
      m_cell.m_type = t;
      m_cell.m_data.num = datum;
    } else {
      m_cell.m_type = KindOfNull;
    }
  }

private:
  Cell m_cell;
};

ALWAYS_INLINE void decRefRef(RefData* ref) {
  ref->decRefAndRelease();
}

} // namespace HPHP

#endif //incl_HPHP_REF_DATA_H
