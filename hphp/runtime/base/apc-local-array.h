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

#ifndef incl_HPHP_APC_LOCAL_ARRAY_H_
#define incl_HPHP_APC_LOCAL_ARRAY_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/apc-array.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class APCArray;

/*
 * Wrapper for APCArray. It is what gets returned when an array is fetched
 * via APC. It has a pointer to the APCArray that it represents and it may
 * cache values locally depending on the type accessed and/or the operation.
 */
class APCLocalArray : public ArrayData, Sweepable {
  explicit APCLocalArray(APCArray* source)
    : ArrayData(kSharedKind)
    , m_arr(source)
    , m_localCache(nullptr) {
    m_size = m_arr->size();
    source->incRef();
  }

  ~APCLocalArray();

public:
  template<class... Args>
  static APCLocalArray* Make(Args&&... args) {
    return new (MM().smartMallocSize(sizeof(APCLocalArray)))
      APCLocalArray(std::forward<Args>(args)...);
  }

  static APCHandle *GetAPCHandle(const ArrayData* ad) {
    auto a = asSharedArray(ad);
    if (a->m_arr->shouldCache()) return nullptr;
    return a->m_arr->getHandle();
  }

  // these using directives ensure the full set of overloaded functions
  // are visible in this class, to avoid triggering implicit conversions
  // from a CVarRef key to int64.
  using ArrayData::exists;
  using ArrayData::lval;
  using ArrayData::lvalNew;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::remove;

  Variant getKey(ssize_t pos) const {
    return m_arr->getKey(pos);
  }

  CVarRef getValueRef(ssize_t pos) const;
  static CVarRef GetValueRef(const ArrayData* ad, ssize_t pos) {
    return asSharedArray(ad)->getValueRef(pos);
  }

  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static bool ExistsStr(const ArrayData* ad, const StringData* k);

  static ArrayData* LvalInt(ArrayData*, int64_t k, Variant *&ret,
                            bool copy);
  static ArrayData* LvalStr(ArrayData*, StringData* k, Variant *&ret,
                            bool copy);
  static ArrayData* LvalNew(ArrayData*, Variant *&ret, bool copy);

  static ArrayData* SetInt(ArrayData*, int64_t k, CVarRef v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, CVarRef v, bool copy);
  static ArrayData* SetRefInt(ArrayData*, int64_t k, CVarRef v, bool copy);
  static ArrayData* SetRefStr(ArrayData*, StringData* k, CVarRef v, bool copy);

  static ArrayData *RemoveInt(ArrayData* ad, int64_t k, bool copy);
  static ArrayData *RemoveStr(ArrayData* ad, const StringData* k, bool copy);

  static ArrayData* Copy(const ArrayData*);
  /**
   * Copy (escalate) the SharedArray without triggering local cache.
   */
  static ArrayData* Append(ArrayData* a, CVarRef v, bool copy);
  static ArrayData* AppendRef(ArrayData*, CVarRef v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, CVarRef v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData *elems);
  static ArrayData* Merge(ArrayData*, const ArrayData *elems);
  static ArrayData* Prepend(ArrayData*, CVarRef v, bool copy);

  /**
   * Non-Variant methods that override ArrayData
   */
  static TypedValue* NvGetInt(const ArrayData*, int64_t k);
  static TypedValue* NvGetStr(const ArrayData*, const StringData* k);
  static void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos);

  static bool IsVectorData(const ArrayData* ad);

  ssize_t iterAdvanceImpl(ssize_t prev) const {
    assert(prev >= 0 && prev < m_size);
    ssize_t next = prev + 1;
    return next < m_size ? next : invalid_index;
  }

  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t prev);
  static ssize_t IterRewind(const ArrayData*, ssize_t prev);

  static bool ValidFullPos(const ArrayData*, const FullPos& fp);
  static bool AdvanceFullPos(ArrayData*, FullPos& fp);

  static void Release(ArrayData*);

  static ArrayData* Escalate(const ArrayData*);
  static ArrayData* EscalateForSort(ArrayData*);

  // implements Sweepable.sweep()
  void sweep() FOLLY_OVERRIDE { m_arr->decRef(); }

private:
  ssize_t getIndex(int64_t k) const;
  ssize_t getIndex(const StringData* k) const;

  static APCLocalArray* asSharedArray(ArrayData* ad) {
    assert(ad->kind() == kSharedKind);
    return static_cast<APCLocalArray*>(ad);
  }

  static const APCLocalArray* asSharedArray(const ArrayData* ad) {
    assert(ad->kind() == kSharedKind);
    return static_cast<const APCLocalArray*>(ad);
  }

  ArrayData* loadElems() const;

public:
  void getChildren(std::vector<TypedValue *> &out);

  APCArray *m_arr;
  mutable TypedValue* m_localCache;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_APC_LOCAL_ARRAY_H_
