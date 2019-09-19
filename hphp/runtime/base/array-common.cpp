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

ArrayData* ArrayCommon::Pop(ArrayData* a, Variant &value) {
  if (!a->empty()) {
    auto const pos = a->iter_last();
    value = a->getValue(pos);
    return a->remove(a->getKey(pos));
  }
  value = uninit_null();
  return a;
}

ArrayData* ArrayCommon::Dequeue(ArrayData* a, Variant &value) {
  if (!a->empty()) {
    auto const pos = a->iter_begin();
    value = a->getValue(pos);
    auto const ret = a->remove(a->getKey(pos));
    // In PHP, array_shift() will cause all numerically key-ed values re-keyed
    ret->renumber();
    return ret;
  }
  value = uninit_null();
  return a;
}

ArrayData* ArrayCommon::ToVec(ArrayData* a, bool) {
  auto const size = a->size();
  if (!size) return ArrayData::CreateVec();
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
  auto const out = init.create();
  return RuntimeOption::EvalArrayProvenance && out->isRefCounted()
    ? tagArrProv(out, a)
    : out;
}

ArrayData* ArrayCommon::ToDict(ArrayData* a, bool) {
  auto const size = a->size();
  if (!size) return ArrayData::CreateDict();
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
  auto const out = init.create();
  return RuntimeOption::EvalArrayProvenance && out->isRefCounted()
    ? tagArrProv(out, a)
    : out;
}

ArrayData* ArrayCommon::ToKeyset(ArrayData* a, bool) {
  auto const size = a->size();
  if (!size) return ArrayData::CreateKeyset();
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
  if (!size) return ArrayData::CreateVArray();
  VArrayInit init{size};
  IterateVNoInc(a, [&](TypedValue v) { init.appendWithRef(v); });
  return init.create();
}

ArrayData* ArrayCommon::ToDArray(ArrayData* a, bool) {
  if (a->isDArray()) return a;
  auto const size = a->size();
  if (!size) return ArrayData::CreateDArray();
  DArrayInit init{size};
  IterateKV(
    a,
    [&](Cell k, TypedValue v) {
      init.setUnknownKey(tvAsCVarRef(&k), tvAsCVarRef(&v));
    }
  );
  return init.create();
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
  auto const out = arr.detach();
  return RuntimeOption::EvalArrayProvenance && out->isRefCounted()
    ? tagArrProv(out)
    : out;
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
