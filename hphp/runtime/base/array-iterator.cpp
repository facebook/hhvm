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
#include "hphp/runtime/base/array-iterator.h"

#include <algorithm>

#include <folly/Likely.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/tv-refcount.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

const StaticString
  s_rewind("rewind"),
  s_valid("valid"),
  s_next("next"),
  s_key("key"),
  s_current("current");

//////////////////////////////////////////////////////////////////////

ArrayIter::ArrayIter(const ArrayData* data) {
  arrInit(data);
}

ArrayIter::ArrayIter(const Array& array) {
  arrInit(array.get());
}

ArrayIter::ArrayIter(ObjectData* obj) {
  objInit<true>(obj);
}

ArrayIter::ArrayIter(ObjectData* obj, NoInc) {
  objInit<false>(obj);
}

ArrayIter::ArrayIter(const Object& obj) {
  objInit<true>(obj.get());
}

ArrayIter::ArrayIter(const TypedValue c) {
  tvInit(c);
}

ArrayIter::ArrayIter(const Variant& v) {
  tvInit(*v.asTypedValue());
}

ArrayIter::ArrayIter(const ArrayIter& iter) {
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_end = iter.m_end;
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) const_cast<ArrayData*>(ad)->incRefCount();
  } else {
    ObjectData* obj = getObject();
    assertx(obj);
    obj->incRefCount();
  }
}

template <bool incRef /* = true */>
void ArrayIter::arrInit(const ArrayData* arr) {
  setArrayData(arr);
  if (arr && incRef) arr->incRefCount();
}

template <bool incRef>
void ArrayIter::objInit(ObjectData* obj) {
  assertx(obj);

  if (LIKELY(obj->isCollection())) {
    if (auto ad = collections::asArray(obj)) {
      ad->incRefCount();
      if (!incRef) decRefObj(obj);
      setArrayData(ad);
    } else {
      assertx(obj->collectionType() == CollectionType::Pair);
      auto arr = collections::toArray(obj);
      if (!incRef) decRefObj(obj);
      setArrayData(arr.detach());
    }
    return;
  }

  assertx(obj->instanceof(SystemLib::s_HH_IteratorClass));
  setObject(obj);
  if (incRef) obj->incRefCount();
  try {
    obj->o_invoke_few_args(s_rewind, RuntimeCoeffects::fixme(), 0);
  } catch (...) {
    // Regardless of whether the incRef template parameter is true or false,
    // at this point, this ArrayIter "owns" a reference to the object and is
    // responsible for dec-ref-ing it when the iterator is destroyed.
    //
    // Normally, the destructor takes care of this case, but we'll never invoke
    // it if the exception is thrown before the constructor finishes, so we
    // must manually dec-ref the object here.
    decRefObj(obj);
    kill();
    throw;
  }
}

void ArrayIter::tvInit(const TypedValue c) {
  assertx(tvIsPlausible(c));
  if (LIKELY(isArrayLikeType(c.m_type))) {
    arrInit(c.m_data.parr);
  } else if (LIKELY(c.m_type == KindOfObject)) {
    objInit<true>(c.m_data.pobj);
  } else if (RO::EvalIsCompatibleClsMethType && isClsMethType(c.m_type)) {
    raiseClsMethToVecWarningHelper();
    arrInit<false>(clsMethToVecHelper(c.m_data.pclsmeth).detach());
  } else {
    arrInit(nullptr);
  }
}

void ArrayIter::rewind() {
  assertx(hasArrayData());
  if (auto* data = getArrayData()) {
    m_pos = data->iter_begin();
  }
}

void ArrayIter::kill() {
  if (!debug) return;
  // ArrayIter is not POD, so we memset each POD field separately.
  memset(&m_data, kIterTrashFill, sizeof(m_data));
  memset(&m_pos, kIterTrashFill, sizeof(m_pos));
  memset(&m_end, kIterTrashFill, sizeof(m_end));
}

void ArrayIter::destruct() {
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) decRefArr(const_cast<ArrayData*>(ad));
    return kill();
  }
  ObjectData* obj = getObject();
  assertx(obj);
  decRefObj(obj);
  kill();
}

ArrayIter& ArrayIter::operator=(const ArrayIter& iter) {
  reset();
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_end = iter.m_end;
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) const_cast<ArrayData*>(ad)->incRefCount();
  } else {
    ObjectData* obj = getObject();
    assertx(obj);
    obj->incRefCount();
  }
  return *this;
}

ArrayIter& ArrayIter::operator=(ArrayIter&& iter) {
  reset();
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_end = iter.m_end;
  iter.m_data = nullptr;
  return *this;
}

bool ArrayIter::endHelper() const  {
  auto obj = getObject();
  return !obj->o_invoke_few_args(s_valid, RuntimeCoeffects::fixme(), 0).toBoolean();
}

void ArrayIter::nextHelper() {
  auto obj = getObject();
  obj->o_invoke_few_args(s_next, RuntimeCoeffects::fixme(), 0);
}

Variant ArrayIter::firstHelper() {
  auto obj = getObject();
  return obj->o_invoke_few_args(s_key, RuntimeCoeffects::fixme(), 0);
}

Variant ArrayIter::second() {
  if (LIKELY(hasArrayData())) return getArrayData()->getValue(m_pos);
  return getObject()->o_invoke_few_args(s_current, RuntimeCoeffects::fixme(), 0);
}

TypedValue ArrayIter::secondVal() const {
  if (LIKELY(hasArrayData())) return getArrayData()->nvGetVal(m_pos);
  raise_fatal_error("taking reference on iterator objects");
}

TypedValue ArrayIter::secondValPlus() const {
  if (LIKELY(hasArrayData())) return getArrayData()->nvGetVal(m_pos);
  throw_param_is_not_container();
}

///////////////////////////////////////////////////////////////////////////////

}
