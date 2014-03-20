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
#include "hphp/runtime/base/array-data.h"

#include <vector>
#include <array>
#include <boost/lexical_cast.hpp>
#include <tbb/concurrent_hash_map.h>

#include "hphp/util/exception.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/empty-array.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/vm/name-value-table-wrapper.h"
#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/hphp-array.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static_assert(
  sizeof(ArrayData) == 16,
  "Performance is sensitive to sizeof(ArrayData)."
  " Make sure you changed it with good reason and then update this assert.");

typedef tbb::concurrent_hash_map<std::string, ArrayData*> ArrayDataMap;
static ArrayDataMap s_arrayDataMap;

ArrayData* ArrayData::GetScalarArray(ArrayData* arr) {
  if (arr->empty()) return HphpArray::GetStaticEmptyArray();
  auto key = f_serialize(arr).toCppString();
  return GetScalarArray(arr, key);
}

ArrayData* ArrayData::GetScalarArray(ArrayData* arr, const std::string& key) {
  if (arr->empty()) return HphpArray::GetStaticEmptyArray();
  assert(key == f_serialize(arr).toCppString());
  ArrayDataMap::accessor acc;
  if (s_arrayDataMap.insert(acc, key)) {
    ArrayData *ad = arr->nonSmartCopy();
    ad->setStatic();
    ad->onSetEvalScalar();
    acc->second = ad;
  }
  return acc->second;
}

///////////////////////////////////////////////////////////////////////////////

static size_t VsizeNop(const ArrayData* ad) {
  assert(false);
  return ad->getSize();
}

// order: kPackedKind, kMixedKind, kSharedKind, kNvtwKind
extern const ArrayFunctions g_array_funcs = {
  // release
  { &HphpArray::ReleasePacked, &HphpArray::Release,
    &APCLocalArray::Release,
    &EmptyArray::Release,
    &NameValueTableWrapper::Release,
    &ProxyArray::Release },
  // nvGetInt
  { &HphpArray::NvGetIntPacked, &HphpArray::NvGetInt,
    &APCLocalArray::NvGetInt,
    reinterpret_cast<TypedValue*(*)(const ArrayData*, int64_t)>(
      &EmptyArray::ReturnNull
    ),
    &NameValueTableWrapper::NvGetInt,
    &ProxyArray::NvGetInt },
  // nvGetStr
  { &HphpArray::NvGetStrPacked, &HphpArray::NvGetStr,
    &APCLocalArray::NvGetStr,
    reinterpret_cast<TypedValue*(*)(const ArrayData*, const StringData*)>(
      &EmptyArray::ReturnNull
    ),
    &NameValueTableWrapper::NvGetStr,
    &ProxyArray::NvGetStr },
  // nvGetKey
  { &HphpArray::NvGetKeyPacked, &HphpArray::NvGetKey,
    &APCLocalArray::NvGetKey,
    &EmptyArray::NvGetKey,
    &NameValueTableWrapper::NvGetKey,
    &ProxyArray::NvGetKey },
  // setInt
  { &HphpArray::SetIntPacked, &HphpArray::SetInt,
    &APCLocalArray::SetInt,
    &EmptyArray::SetInt,
    &NameValueTableWrapper::SetInt,
    &ProxyArray::SetInt },
  // setStr
  { &HphpArray::SetStrPacked, &HphpArray::SetStr,
    &APCLocalArray::SetStr,
    &EmptyArray::SetStr,
    &NameValueTableWrapper::SetStr,
    &ProxyArray::SetStr },
  // vsize
  { &VsizeNop, &VsizeNop,
    &VsizeNop,
    &VsizeNop,
    &NameValueTableWrapper::Vsize,
    &ProxyArray::Vsize },
  // getValueRef
  { &HphpArray::GetValueRef, &HphpArray::GetValueRef,
    &APCLocalArray::GetValueRef,
    &EmptyArray::GetValueRef,
    &NameValueTableWrapper::GetValueRef,
    &ProxyArray::GetValueRef },
  // noCopyOnWrite
  { false, false,
    false,
    false,
    true, // NameValueTableWrapper doesn't support COW.
    false },
  // isVectorData
  {
    reinterpret_cast<bool (*)(const ArrayData*)>(
      // TODO(#3983912): move shared helpers to consolidated location
      &EmptyArray::ReturnTrue
    ),
    &HphpArray::IsVectorData,
    &APCLocalArray::IsVectorData,
    reinterpret_cast<bool (*)(const ArrayData*)>(
      &EmptyArray::ReturnTrue
    ),
    &NameValueTableWrapper::IsVectorData,
    &ProxyArray::IsVectorData },
  // existsInt
  { &HphpArray::ExistsIntPacked, &HphpArray::ExistsInt,
    &APCLocalArray::ExistsInt,
    reinterpret_cast<bool (*)(const ArrayData*, int64_t)>(
      &EmptyArray::ReturnFalse
    ),
    &NameValueTableWrapper::ExistsInt,
    &ProxyArray::ExistsInt },
  // existsStr
  {
    reinterpret_cast<bool (*)(const ArrayData*, const StringData*)>(
      // TODO(#3983912): move shared helpers to consolidated location
      &EmptyArray::ReturnFalse
    ),
    &HphpArray::ExistsStr,
    &APCLocalArray::ExistsStr,
    reinterpret_cast<bool (*)(const ArrayData*, const StringData*)>(
      &EmptyArray::ReturnFalse
    ),
    &NameValueTableWrapper::ExistsStr,
    &ProxyArray::ExistsStr },
  // lvalInt
  { &HphpArray::LvalIntPacked, &HphpArray::LvalInt,
    &APCLocalArray::LvalInt,
    &EmptyArray::LvalInt,
    &NameValueTableWrapper::LvalInt,
    &ProxyArray::LvalInt },
  // lvalStr
  { &HphpArray::LvalStrPacked, &HphpArray::LvalStr,
    &APCLocalArray::LvalStr,
    &EmptyArray::LvalStr,
    &NameValueTableWrapper::LvalStr,
    &ProxyArray::LvalStr },
  // lvalNew
  { &HphpArray::LvalNewPacked, &HphpArray::LvalNew,
    &APCLocalArray::LvalNew,
    &EmptyArray::LvalNew,
    &NameValueTableWrapper::LvalNew,
    &ProxyArray::LvalNew },
  // setRefInt
  { &HphpArray::SetRefIntPacked, &HphpArray::SetRefInt,
    &APCLocalArray::SetRefInt,
    &EmptyArray::SetRefInt,
    &NameValueTableWrapper::SetRefInt,
    &ProxyArray::SetRefInt },
  // setRefStr
  { &HphpArray::SetRefStrPacked, &HphpArray::SetRefStr,
    &APCLocalArray::SetRefStr,
    &EmptyArray::SetRefStr,
    &NameValueTableWrapper::SetRefStr,
    &ProxyArray::SetRefStr },
  // addInt
  { &HphpArray::AddIntPacked, &HphpArray::AddInt,
    &APCLocalArray::SetInt, // reuse set
    &EmptyArray::SetInt, // reuse set
    &NameValueTableWrapper::SetInt, // reuse set
    &ProxyArray::SetInt }, // reuse set
  // addStr
  { &HphpArray::SetStrPacked, // reuse set
    &HphpArray::AddStr,
    &APCLocalArray::SetStr, // reuse set
    &EmptyArray::SetStr, // reuse set
    &NameValueTableWrapper::SetStr, // reuse set
    &ProxyArray::SetStr }, // reuse set
  // removeInt
  { &HphpArray::RemoveIntPacked, &HphpArray::RemoveInt,
    &APCLocalArray::RemoveInt,
    reinterpret_cast<ArrayData* (*)(ArrayData*, int64_t, bool)>(
      &EmptyArray::ReturnFirstArg
    ),
    &NameValueTableWrapper::RemoveInt,
    &ProxyArray::RemoveInt },
  // removeStr
  { &HphpArray::RemoveStrPacked, &HphpArray::RemoveStr,
    &APCLocalArray::RemoveStr,
    reinterpret_cast<ArrayData* (*)(ArrayData*, const StringData*, bool)>(
      &EmptyArray::ReturnFirstArg
    ),
    &NameValueTableWrapper::RemoveStr,
    &ProxyArray::RemoveStr },
  // iterBegin
  { &HphpArray::IterBegin, &HphpArray::IterBegin,
    &APCLocalArray::IterBegin,
    &EmptyArray::ReturnInvalidIndex,
    &NameValueTableWrapper::IterBegin,
    &ProxyArray::IterBegin },
  // iterEnd
  { &HphpArray::IterEnd, &HphpArray::IterEnd,
    &APCLocalArray::IterEnd,
    &EmptyArray::ReturnInvalidIndex,
    &NameValueTableWrapper::IterEnd,
    &ProxyArray::IterEnd },
  // iterAdvance
  { &HphpArray::IterAdvance, &HphpArray::IterAdvance,
    &APCLocalArray::IterAdvance,
    &EmptyArray::IterAdvance,
    &NameValueTableWrapper::IterAdvance,
    &ProxyArray::IterAdvance },
  // iterRewind
  { &HphpArray::IterRewind, &HphpArray::IterRewind,
    &APCLocalArray::IterRewind,
    &EmptyArray::IterRewind,
    &NameValueTableWrapper::IterRewind,
    &ProxyArray::IterRewind },
  // validMArrayIter
  { &HphpArray::ValidMArrayIter, &HphpArray::ValidMArrayIter,
    &APCLocalArray::ValidMArrayIter,
    &EmptyArray::ValidMArrayIter,
    &NameValueTableWrapper::ValidMArrayIter,
    &ProxyArray::ValidMArrayIter },
  // advanceMArrayIter
  { &HphpArray::AdvanceMArrayIter, &HphpArray::AdvanceMArrayIter,
    &APCLocalArray::AdvanceMArrayIter,
    &EmptyArray::AdvanceMArrayIter,
    &NameValueTableWrapper::AdvanceMArrayIter,
    &ProxyArray::AdvanceMArrayIter },
  // escalateForSort
  { &HphpArray::EscalateForSort, &HphpArray::EscalateForSort,
    &APCLocalArray::EscalateForSort,
    reinterpret_cast<ArrayData* (*)(ArrayData*)>(
      &EmptyArray::ReturnFirstArg
    ),
    &NameValueTableWrapper::EscalateForSort,
    &ProxyArray::EscalateForSort },
  // ksort
  { &HphpArray::Ksort, &HphpArray::Ksort,
    &ArrayData::Ksort,
    reinterpret_cast<void (*)(ArrayData*, int, bool)>(
      &EmptyArray::NoOp
    ),
    &NameValueTableWrapper::Ksort,
    &ProxyArray::Ksort },
  // sort
  { &HphpArray::Sort, &HphpArray::Sort,
    &ArrayData::Sort,
    reinterpret_cast<void (*)(ArrayData*, int, bool)>(
      &EmptyArray::NoOp
    ),
    &NameValueTableWrapper::Sort,
    &ProxyArray::Sort },
  // asort
  { &HphpArray::Asort, &HphpArray::Asort,
    &ArrayData::Asort,
    reinterpret_cast<void (*)(ArrayData*, int, bool)>(
      &EmptyArray::NoOp
    ),
    &NameValueTableWrapper::Asort,
    &ProxyArray::Asort },
  // uksort
  { &HphpArray::Uksort, &HphpArray::Uksort,
    &ArrayData::Uksort,
    reinterpret_cast<bool (*)(ArrayData*, const Variant&)>(
      &EmptyArray::ReturnTrue
    ),
    &NameValueTableWrapper::Uksort,
    &ProxyArray::Uksort },
  // usort
  { &HphpArray::Usort, &HphpArray::Usort,
    &ArrayData::Usort,
    reinterpret_cast<bool (*)(ArrayData*, const Variant&)>(
      &EmptyArray::ReturnTrue
    ),
    &NameValueTableWrapper::Usort,
    &ProxyArray::Usort },
  // uasort
  { &HphpArray::Uasort, &HphpArray::Uasort,
    &ArrayData::Uasort,
    reinterpret_cast<bool (*)(ArrayData*, const Variant&)>(
      &EmptyArray::ReturnTrue
    ),
    &NameValueTableWrapper::Uasort,
    &ProxyArray::Uasort },
  // copy
  { &HphpArray::CopyPacked, &HphpArray::Copy,
    &APCLocalArray::Copy,
    &EmptyArray::Copy,
    &NameValueTableWrapper::Copy,
    &ProxyArray::Copy },
  // copyWithStrongIterators
  { &HphpArray::CopyWithStrongIterators, &HphpArray::CopyWithStrongIterators,
    &APCLocalArray::CopyWithStrongIterators,
    &EmptyArray::CopyWithStrongIterators,
    &NameValueTableWrapper::CopyWithStrongIterators,
    &ProxyArray::CopyWithStrongIterators },
  // nonSmartCopy
  { &HphpArray::NonSmartCopy, &HphpArray::NonSmartCopy,
    &ArrayData::NonSmartCopy,
    &EmptyArray::NonSmartCopy,
    &ProxyArray::NonSmartCopy },
  // append
  { &HphpArray::AppendPacked, &HphpArray::Append,
    &APCLocalArray::Append,
    &EmptyArray::Append,
    &NameValueTableWrapper::Append,
    &ProxyArray::Append },
  // appendRef
  { &HphpArray::AppendRefPacked, &HphpArray::AppendRef,
    &APCLocalArray::AppendRef,
    &EmptyArray::AppendRef,
    &NameValueTableWrapper::AppendRef,
    &ProxyArray::AppendRef },
  // appendWithRef
  { &HphpArray::AppendWithRefPacked, &HphpArray::AppendWithRef,
    &APCLocalArray::AppendRef,
    &EmptyArray::AppendWithRef,
    &NameValueTableWrapper::AppendRef,
    &ProxyArray::AppendRef },
  // plusEq
  { &HphpArray::PlusEq, &HphpArray::PlusEq,
    &APCLocalArray::PlusEq,
    &EmptyArray::PlusEq,
    &NameValueTableWrapper::PlusEq,
    &ProxyArray::PlusEq },
  // merge
  { &HphpArray::Merge, &HphpArray::Merge,
    &APCLocalArray::Merge,
    &EmptyArray::Merge,
    &NameValueTableWrapper::Merge,
    &ProxyArray::Merge },
  // pop
  { &HphpArray::PopPacked, &HphpArray::Pop,
    &APCLocalArray::Pop,
    &EmptyArray::PopOrDequeue,
    &NameValueTableWrapper::Pop,
    &ProxyArray::Pop },
  // dequeue
  { &HphpArray::DequeuePacked, &HphpArray::Dequeue,
    &APCLocalArray::Dequeue,
    &EmptyArray::PopOrDequeue,
    &NameValueTableWrapper::Dequeue,
    &ProxyArray::Dequeue },
  // prepend
  { &HphpArray::PrependPacked, &HphpArray::Prepend,
    &APCLocalArray::Prepend,
    &EmptyArray::Prepend,
    &NameValueTableWrapper::Prepend,
    &ProxyArray::Prepend },
  // renumber
  { &HphpArray::RenumberPacked, &HphpArray::Renumber,
    &APCLocalArray::Renumber,
    reinterpret_cast<void (*)(ArrayData*)>(
      &EmptyArray::NoOp
    ),
    &NameValueTableWrapper::Renumber,
    &ProxyArray::Renumber },
  // onSetEvalScalar
  { &HphpArray::OnSetEvalScalarPacked, &HphpArray::OnSetEvalScalar,
    &APCLocalArray::OnSetEvalScalar,
    &EmptyArray::OnSetEvalScalar,
    &NameValueTableWrapper::OnSetEvalScalar,
    &ProxyArray::OnSetEvalScalar },
  // escalate
  { &ArrayData::Escalate, &ArrayData::Escalate,
    &APCLocalArray::Escalate,
    &ArrayData::Escalate,
    &ArrayData::Escalate,
    &ProxyArray::Escalate },
  // getAPCHandle
  { &ArrayData::GetAPCHandle, &ArrayData::GetAPCHandle,
    &APCLocalArray::GetAPCHandle,
    reinterpret_cast<APCHandle* (*)(const ArrayData*)>(
      &EmptyArray::ReturnNull
    ),
    &ArrayData::GetAPCHandle,
    &ProxyArray::GetAPCHandle },
  // zSetInt
  { &HphpArray::ZSetInt, &HphpArray::ZSetInt,
    &ArrayData::ZSetInt,
    &ArrayData::ZSetInt,
    &ArrayData::ZSetInt,
    &ProxyArray::ZSetInt },
  // zSetStr
  { &HphpArray::ZSetStr, &HphpArray::ZSetStr,
    &ArrayData::ZSetStr,
    &ArrayData::ZSetStr,
    &ArrayData::ZSetStr,
    &ProxyArray::ZSetStr },
  // zAppend
  { &HphpArray::ZAppend, &HphpArray::ZAppend,
    &ArrayData::ZAppend,
    &ArrayData::ZAppend,
    &ArrayData::ZAppend,
    &ProxyArray::ZAppend },
};

///////////////////////////////////////////////////////////////////////////////

// In general, arrays can contain int-valued-strings, even though
// plain array access converts them to integers.  non-int-string
// assersions should go upstream of the ArrayData api.

bool ArrayData::IsValidKey(const String& k) {
  return IsValidKey(k.get());
}

bool ArrayData::IsValidKey(const Variant& k) {
  return k.isInteger() ||
         (k.isString() && IsValidKey(k.getStringData()));
}

// constructors/destructors

ArrayData *ArrayData::Create() {
  return ArrayInit((ssize_t)0).create();
}

ArrayData *ArrayData::Create(const Variant& value) {
  ArrayInit init(1);
  init.set(value);
  return init.create();
}

ArrayData *ArrayData::Create(const Variant& name, const Variant& value) {
  ArrayInit init(1);
  // There is no toKey() call on name.
  init.set(name, value, true);
  return init.create();
}

ArrayData *ArrayData::CreateRef(const Variant& value) {
  ArrayInit init(1);
  init.setRef(value);
  return init.create();
}

ArrayData *ArrayData::CreateRef(const Variant& name, const Variant& value) {
  ArrayInit init(1);
  // There is no toKey() call on name.
  init.setRef(name, value, true);
  return init.create();
}

ArrayData *ArrayData::NonSmartCopy(const ArrayData*) {
  throw FatalErrorException("nonSmartCopy not implemented.");
}

///////////////////////////////////////////////////////////////////////////////
// reads

int ArrayData::compare(const ArrayData *v2) const {
  assert(v2);

  auto const count1 = size();
  auto const count2 = v2->size();
  if (count1 < count2) return -1;
  if (count1 > count2) return 1;
  if (count1 == 0) return 0;

  // prevent circular referenced objects/arrays or deep ones
  DECLARE_THREAD_INFO; check_recursion(info);

  for (ArrayIter iter(this); iter; ++iter) {
    auto key = iter.first();
    if (!v2->exists(key)) return 1;
    auto value1 = iter.second();
    auto value2 = v2->get(key);
    if (HPHP::more(value1, value2)) return 1;
    if (HPHP::less(value1, value2)) return -1;
  }

  return 0;
}

bool ArrayData::equal(const ArrayData *v2, bool strict) const {
  assert(v2);

  if (this == v2) return true;
  auto const count1 = size();
  auto const count2 = v2->size();
  if (count1 != count2) return false;
  if (count1 == 0) return true;

  // prevent circular referenced objects/arrays or deep ones
  DECLARE_THREAD_INFO; check_recursion(info);

  if (strict) {
    for (ArrayIter iter1(this), iter2(v2); iter1; ++iter1, ++iter2) {
      assert(iter2);
      if (!same(iter1.first(), iter2.first())
          || !same(iter1.second(), iter2.secondRef())) return false;
    }
  } else {
    for (ArrayIter iter(this); iter; ++iter) {
      Variant key(iter.first());
      if (!v2->exists(key)) return false;
      if (!tvEqual(*iter.second().asTypedValue(),
                   *v2->get(key).asTypedValue())) {
        return false;
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// stack and queue operations

ArrayData *ArrayData::Pop(ArrayData* a, Variant &value) {
  if (!a->empty()) {
    ssize_t pos = a->iter_end();
    value = a->getValue(pos);
    return a->remove(a->getKey(pos), a->hasMultipleRefs());
  }
  value = uninit_null();
  return a;
}

ArrayData *ArrayData::Dequeue(ArrayData* a, Variant &value) {
  if (!a->empty()) {
    auto const pos = a->iter_begin();
    value = a->getValue(pos);
    ArrayData *ret = a->remove(a->getKey(pos), a->hasMultipleRefs());

    // In PHP, array_shift() will cause all numerically key-ed values re-keyed
    ret->renumber();
    return ret;
  }
  value = uninit_null();
  return a;
}

//////////////////////////////////////////////////////////////////////

const Variant& ArrayData::endRef() {
  if (m_pos != invalid_index) {
    return getValueRef(iter_end());
  }
  throw FatalErrorException("invalid ArrayData::m_pos");
}

void ArrayData::Ksort(ArrayData*, int sort_flags, bool ascending) {
  throw FatalErrorException("Unimplemented ArrayData::ksort");
}

void ArrayData::Sort(ArrayData*, int sort_flags, bool ascending) {
  throw FatalErrorException("Unimplemented ArrayData::sort");
}

void ArrayData::Asort(ArrayData*, int sort_flags, bool ascending) {
  throw FatalErrorException("Unimplemented ArrayData::asort");
}

bool ArrayData::Uksort(ArrayData*, const Variant& cmp_function) {
  throw FatalErrorException("Unimplemented ArrayData::uksort");
}

bool ArrayData::Usort(ArrayData*, const Variant& cmp_function) {
  throw FatalErrorException("Unimplemented ArrayData::usort");
}

bool ArrayData::Uasort(ArrayData*, const Variant& cmp_function) {
  throw FatalErrorException("Unimplemented ArrayData::uasort");
}

ArrayData* ArrayData::CopyWithStrongIterators(const ArrayData* ad) {
  throw FatalErrorException("Unimplemented ArrayData::copyWithStrongIterators");
}

ArrayData* ArrayData::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  throw FatalErrorException("Unimplemented ArrayData::ZSetInt");
}

ArrayData* ArrayData::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  throw FatalErrorException("Unimplemented ArrayData::ZSetStr");
}

ArrayData* ArrayData::ZAppend(ArrayData* ad, RefData* v) {
  throw FatalErrorException("Unimplemented ArrayData::ZAppend");
}

///////////////////////////////////////////////////////////////////////////////
// Default implementation of position-based iterations.

Variant ArrayData::reset() {
  setPosition(iter_begin());
  return m_pos != invalid_index ? getValue(m_pos) : Variant(false);
}

Variant ArrayData::prev() {
  if (m_pos != invalid_index) {
    setPosition(iter_rewind(m_pos));
    if (m_pos != invalid_index) {
      return getValue(m_pos);
    }
  }
  return Variant(false);
}

Variant ArrayData::next() {
  if (m_pos != invalid_index) {
    setPosition(iter_advance(m_pos));
    if (m_pos != invalid_index) {
      return getValue(m_pos);
    }
  }
  return Variant(false);
}

Variant ArrayData::end() {
  setPosition(iter_end());
  return m_pos != invalid_index ? getValue(m_pos) : Variant(false);
}

Variant ArrayData::key() const {
  return m_pos != invalid_index ? getKey(m_pos) : uninit_null();
}

Variant ArrayData::value(int32_t &pos) const {
  return pos != invalid_index ? getValue(pos) : Variant(false);
}

Variant ArrayData::current() const {
  return m_pos != invalid_index ? getValue(m_pos) : Variant(false);
}

const StaticString
  s_value("value"),
  s_key("key");

Variant ArrayData::each() {
  if (m_pos != invalid_index) {
    ArrayInit ret(4);
    Variant key(getKey(m_pos));
    Variant value(getValue(m_pos));
    ret.set(1, value);
    ret.set(s_value, value);
    ret.set(0, key);
    ret.set(s_key, key);
    setPosition(iter_advance(m_pos));
    return ret.toVariant();
  }
  return Variant(false);
}

///////////////////////////////////////////////////////////////////////////////
// helpers

void ArrayData::serializeImpl(VariableSerializer *serializer) const {
  serializer->writeArrayHeader(size(), isVectorData());
  for (ArrayIter iter(this); iter; ++iter) {
    serializer->writeArrayKey(iter.first());
    serializer->writeArrayValue(iter.secondRef());
  }
  serializer->writeArrayFooter();
}

void ArrayData::serialize(VariableSerializer *serializer,
                          bool skipNestCheck /* = false */) const {
  if (size() == 0) {
    serializer->writeArrayHeader(0, isVectorData());
    serializer->writeArrayFooter();
    return;
  }
  if (!skipNestCheck) {
    if (serializer->incNestedLevel((void*)this)) {
      serializer->writeOverflow((void*)this);
    } else {
      serializeImpl(serializer);
    }
    serializer->decNestedLevel((void*)this);
  } else {
    // If isObject, the array is temporary and we should not check or save
    // its pointer.
    serializeImpl(serializer);
  }
}

const Variant& ArrayData::get(const Variant& k, bool error) const {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? get(getIntKey(cell), error)
                        : get(getStringKey(cell), error);
}

const Variant& ArrayData::getNotFound(int64_t k) {
  raise_notice("Undefined index: %" PRId64, k);
  return null_variant;
}

const Variant& ArrayData::getNotFound(const StringData* k) {
  raise_notice("Undefined index: %s", k->data());
  return null_variant;
}

const Variant& ArrayData::getNotFound(int64_t k, bool error) const {
  return error && m_kind != kNvtwKind ? getNotFound(k) :
         null_variant;
}

const Variant& ArrayData::getNotFound(const StringData* k, bool error) const {
  return error && m_kind != kNvtwKind ? getNotFound(k) :
         null_variant;
}

const Variant& ArrayData::getNotFound(const String& k) {
  raise_notice("Undefined index: %s", k.data());
  return null_variant;
}

const Variant& ArrayData::getNotFound(const Variant& k) {
  raise_notice("Undefined index: %s", k.toString().data());
  return null_variant;
}

void ArrayData::Renumber(ArrayData*) {
}

void ArrayData::OnSetEvalScalar(ArrayData*) {
  assert(false);
}

// TODO(#3983912): combine with EmptyArray::ReturnFirstArg
ArrayData* ArrayData::Escalate(const ArrayData* ad) {
  return const_cast<ArrayData*>(ad);
}

const char* ArrayData::kindToString(ArrayKind kind) {
  std::array<const char*,6> names = {{
    "PackedKind",
    "MixedKind",
    "SharedKind",
    "EmptyKind",
    "NvtwKind",
    "ProxyKind",
  }};
  static_assert(names.size() == kNumKinds, "add new kinds here");
  return names[kind];
}

void ArrayData::getChildren(std::vector<TypedValue *> &out) {
  if (isSharedArray()) {
    APCLocalArray *sm = static_cast<APCLocalArray *>(this);
    sm->getChildren(out);
    return;
  }
  for (ssize_t pos = iter_begin();
      pos != ArrayData::invalid_index;
      pos = iter_advance(pos)) {
    TypedValue *tv = nvGetValueRef(pos);
    out.push_back(tv);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
