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

#include "hphp/runtime/base/array-common.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

ssize_t ArrayCommon::ReturnInvalidIndex(const ArrayData*) {
  return 0;
}

bool ArrayCommon::ValidMArrayIter(const ArrayData* ad, const MArrayIter& fp) {
  assert(fp.getContainer() == ad);
  if (fp.getResetFlag()) return false;
  if (ad->hasPackedLayout()) {
    assert(PackedArray::checkInvariants(ad));
    return fp.m_pos != ad->getSize();
  } else if (ad->isKeyset()) {
    return false;
  } else {
    assert(MixedArray::asMixed(ad));
    return fp.m_pos != MixedArray::asMixed(ad)->iterLimit();
  }
}

ArrayData* ArrayCommon::Pop(ArrayData* a, Variant &value) {
  if (!a->empty()) {
    auto const pos = a->iter_last();
    value = a->getValue(pos);
    return a->remove(a->getKey(pos), a->cowCheck());
  }
  value = uninit_null();
  return a;
}

ArrayData* ArrayCommon::Dequeue(ArrayData* a, Variant &value) {
  if (!a->empty()) {
    auto const pos = a->iter_begin();
    value = a->getValue(pos);
    auto const ret = a->remove(a->getKey(pos), a->cowCheck());
    // In PHP, array_shift() will cause all numerically key-ed values re-keyed
    ret->renumber();
    return ret;
  }
  value = uninit_null();
  return a;
}

ArrayData* ArrayCommon::ToVec(ArrayData* a, bool) {
  auto const size = a->size();
  if (!size) return staticEmptyVecArray();
  VecArrayInit init{size};
  IterateV(
    a,
    [&](TypedValue v) {
      if (UNLIKELY(v.m_type == KindOfRef)) {
        if (v.m_data.pref->isReferenced()) {
          throwRefInvalidArrayValueException(init.toArray());
        }
      }
      init.append(v);
    }
  );
  return init.create();
}

ArrayData* ArrayCommon::ToDict(ArrayData* a, bool) {
  auto const size = a->size();
  if (!size) return staticEmptyDictArray();
  DictInit init{size};
  IterateKV(
    a,
    [&](Cell k, TypedValue v) {
      if (UNLIKELY(v.m_type == KindOfRef)) {
        if (v.m_data.pref->isReferenced()) {
          throwRefInvalidArrayValueException(init.toArray());
        }
      }
      init.setValidKey(k, v);
    }
  );
  return init.create();
}

ArrayData* ArrayCommon::ToKeyset(ArrayData* a, bool) {
  auto const size = a->size();
  if (!size) return staticEmptyKeysetArray();
  KeysetInit init{size};
  IterateV(
    a,
    [&](TypedValue v) {
      if (UNLIKELY(v.m_type == KindOfRef)) {
        if (v.m_data.pref->isReferenced()) {
          throwRefInvalidArrayValueException(init.toArray());
        }
        v = *v.m_data.pref->tv();
        assertx(v.m_type != KindOfRef);
      }

      if (LIKELY(isStringType(v.m_type))) {
        init.add(v.m_data.pstr);
      } else if (LIKELY(isIntType(v.m_type))) {
        init.add(v.m_data.num);
      } else {
        throwInvalidArrayKeyException(&v, init.toArray().get());
      }
    }
  );
  return init.create();
}

ArrayData* ArrayCommon::ToVArray(ArrayData* a, bool) {
  auto const size = a->size();
  if (!size) return staticEmptyArray();
  PackedArrayInit init{size};
  IterateV( a, [&](TypedValue v) { init.appendWithRef(v); });
  return init.create();
}

ArrayCommon::RefCheckResult
ArrayCommon::CheckForRefs(const ArrayData* ad) {
  auto result = RefCheckResult::Pass;
  IterateV(
    ad,
    [&](TypedValue v) {
      if (UNLIKELY(v.m_type == KindOfRef)) {
        auto const ref = v.m_data.pref;
        if (ref->isReferenced() || ref->tv()->m_data.parr == ad) {
          result = RefCheckResult::Fail;
          return true;
        }
        result = RefCheckResult::Collapse;
      }
      return false;
    }
  );
  return result;
}

//////////////////////////////////////////////////////////////////////

}
