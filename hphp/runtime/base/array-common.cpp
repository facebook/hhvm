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
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

ssize_t ArrayCommon::ReturnInvalidIndex(const ArrayData*) {
  return 0;
}

bool ArrayCommon::ValidMArrayIter(const ArrayData* ad, const MArrayIter& fp) {
  assertx(fp.getContainer() == ad);
  if (fp.getResetFlag()) return false;
  if (ad->hasPackedLayout()) {
    assertx(PackedArray::checkInvariants(ad));
    return fp.m_pos != ad->getSize();
  } else if (ad->isKeyset()) {
    return false;
  } else {
    assertx(MixedArray::asMixed(ad));
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
  IterateVNoInc(
    a,
    [&](TypedValue v) {
      if (UNLIKELY(isRefType(v.m_type))) {
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
  IterateKVNoInc(
    a,
    [&](Cell k, TypedValue v) {
      if (UNLIKELY(isRefType(v.m_type))) {
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
  IterateVNoInc(
    a,
    [&](TypedValue v) {
      if (UNLIKELY(isRefType(v.m_type))) {
        if (v.m_data.pref->isReferenced()) {
          throwRefInvalidArrayValueException(init.toArray());
        }
        v = *v.m_data.pref->cell();
        assertx(!isRefType(v.m_type));
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
  if (a->isVArray()) return a;
  auto const size = a->size();
  if (!size) return staticEmptyVArray();
  VArrayInit init{size};
  IterateVNoInc(a, [&](TypedValue v) { init.appendWithRef(v); });
  return init.create();
}

ArrayData* ArrayCommon::ToDArray(ArrayData* a, bool) {
  if (a->isDArray()) return a;
  auto const size = a->size();
  if (!size) return staticEmptyDArray();
  DArrayInit init{size};
  IterateKV(
    a,
    [&](Cell k, TypedValue v) {
      init.setUnknownKey(tvAsCVarRef(&k), tvAsCVarRef(&v));
    }
  );
  return init.create();
}

ArrayData* ArrayCommon::ToShape(ArrayData* a, bool copy) {
  auto arr = RuntimeOption::EvalHackArrDVArrs
    ? ArrayCommon::ToDict(a, copy)
    : ArrayCommon::ToDArray(a, copy);
  arr = arr->toShapeInPlaceIfCompatible();
  return arr;
}

ArrayCommon::RefCheckResult
ArrayCommon::CheckForRefs(const ArrayData* ad) {
  auto result = RefCheckResult::Pass;
  IterateVNoInc(
    ad,
    [&](TypedValue v) {
      if (UNLIKELY(isRefType(v.m_type))) {
        auto const ref = v.m_data.pref;
        if (ref->isReferenced() || ref->cell()->m_data.parr == ad) {
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

namespace {

template <typename E, typename C, typename A>
ALWAYS_INLINE
ArrayData* castObjToHackArrImpl(ObjectData* obj,
                                E empty,
                                C cast,
                                A add,
                                const char* msg) {
  if (LIKELY(obj->isCollection())) {
    if (auto ad = collections::asArray(obj)) {
      return cast(ArrNR{ad}.asArray()).detach();
    }
    return cast(collections::toArray(obj)).detach();
  }

  // iterableObject can re-enter, so bump the ref-count to prevent it from
  // possibly being freed.
  obj->incRefCount();
  SCOPE_EXIT { decRefObj(obj); };

  bool isIter;
  auto iterObj = obj->iterableObject(isIter);
  if (!isIter) SystemLib::throwInvalidOperationExceptionObject(msg);

  auto arr = empty();
  for (ArrayIter iter(iterObj); iter; ++iter) add(arr, iter);
  return arr.detach();
}

}

ArrayData* castObjToVec(ObjectData* obj) {
  return castObjToHackArrImpl(
    obj,
    Array::CreateVec,
    [](const Array& arr) { return arr.toVec(); },
    [](Array& arr, ArrayIter& iter) { arr.append(iter.second()); },
    "Non-iterable object to vec conversion"
  );
}

ArrayData* castObjToDict(ObjectData* obj) {
  return castObjToHackArrImpl(
    obj,
    Array::CreateDict,
    [](const Array& arr) { return arr.toDict(); },
    [](Array& arr, ArrayIter& iter) { arr.set(iter.first(), iter.second()); },
    "Non-iterable object to dict conversion"
  );
}

ArrayData* castObjToKeyset(ObjectData* obj) {
  return castObjToHackArrImpl(
    obj,
    Array::CreateKeyset,
    [](const Array& arr) { return arr.toKeyset(); },
    [](Array& arr, ArrayIter& iter) { arr.append(iter.second()); },
    "Non-iterable object to keyset conversion"
  );
}

ArrayData* castObjToVArray(ObjectData* obj) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  return castObjToHackArrImpl(
    obj,
    Array::CreateVArray,
    [](const Array& arr) { return arr.toVArray(); },
    [](Array& arr, ArrayIter& iter) { arr.append(iter.second()); },
    "Non-iterable object to varray conversion"
  );
}


ArrayData* castObjToDArray(ObjectData* obj) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  return castObjToHackArrImpl(
    obj,
    Array::CreateDArray,
    [](const Array& arr) { return arr.toDArray(); },
    [](Array& arr, ArrayIter& iter) { arr.set(iter.first(), iter.second()); },
    "Non-iterable object to darray conversion"
  );
}

//////////////////////////////////////////////////////////////////////

}
