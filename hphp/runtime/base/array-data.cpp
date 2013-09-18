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
#include "hphp/runtime/base/array-data.h"

#include "tbb/concurrent_hash_map.h"

#include "hphp/util/exception.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/shared-map.h"
#include "hphp/runtime/base/policy-array.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/vm/name-value-table-wrapper.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static_assert(
  sizeof(ArrayData) == 24,
  "Performance is sensitive to sizeof(ArrayData)."
  " Make sure you changed it with good reason and then update this assert.");

typedef tbb::concurrent_hash_map<const StringData *, ArrayData *,
                                 StringDataHashCompare> ArrayDataMap;
static ArrayDataMap s_arrayDataMap;

ArrayData *ArrayData::GetScalarArray(ArrayData *arr,
                                     const StringData *key /* = nullptr */) {
  if (!key) {
    key = makeStaticString(f_serialize(arr).get());
  } else {
    assert(key->isStatic());
    assert(key->same(f_serialize(arr).get()));
  }
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

// order: kPackedKind, kMixedKind, kSharedKind, kNvtwKind, kPolicyKind
extern const ArrayFunctions g_array_funcs = {
  // release
  { &HphpArray::ReleasePacked, &HphpArray::Release,
    &SharedMap::Release,
    &NameValueTableWrapper::Release,
    &PolicyArray::Release },
  // nvGetInt
  { &HphpArray::NvGetIntPacked, &HphpArray::NvGetInt,
    &SharedMap::NvGetInt,
    &NameValueTableWrapper::NvGetInt,
    &PolicyArray::NvGetInt },
  // nvGetStr
  { &HphpArray::NvGetStrPacked, &HphpArray::NvGetStr,
    &SharedMap::NvGetStr,
    &NameValueTableWrapper::NvGetStr,
    &PolicyArray::NvGetStr },
  // nvGetKey
  { &HphpArray::NvGetKeyPacked, &HphpArray::NvGetKey,
    &SharedMap::NvGetKey,
    &NameValueTableWrapper::NvGetKey,
    &PolicyArray::NvGetKey },
  // setInt
  { &HphpArray::SetIntPacked, &HphpArray::SetInt,
    &SharedMap::SetInt,
    &NameValueTableWrapper::SetInt,
    &PolicyArray::SetInt },
  // setStr
  { &HphpArray::SetStrPacked, &HphpArray::SetStr,
    &SharedMap::SetStr,
    &NameValueTableWrapper::SetStr,
    &PolicyArray::SetStr },
  // vsize
  { &VsizeNop, &VsizeNop,
    &VsizeNop,
    &NameValueTableWrapper::Vsize,
    &VsizeNop },
  // getValueRef
  { &HphpArray::GetValueRef, &HphpArray::GetValueRef,
    &SharedMap::GetValueRef,
    &NameValueTableWrapper::GetValueRef,
    &PolicyArray::GetValueRef },
  // noCopyOnWrite
  { false, false,
    false,
    true, // NameValueTableWrapper doesn't support COW.
    false },
  // isVectorData
  { &HphpArray::IsVectorDataPacked, &HphpArray::IsVectorData,
    &SharedMap::IsVectorData,
    &NameValueTableWrapper::IsVectorData,
    &PolicyArray::IsVectorData },
  // existsInt
  { &HphpArray::ExistsIntPacked, &HphpArray::ExistsInt,
    &SharedMap::ExistsInt,
    &NameValueTableWrapper::ExistsInt,
    &PolicyArray::ExistsInt },
  // existsStr
  { &HphpArray::ExistsStrPacked, &HphpArray::ExistsStr,
    &SharedMap::ExistsStr,
    &NameValueTableWrapper::ExistsStr,
    &PolicyArray::ExistsStr },
  // lvalInt
  { &HphpArray::LvalIntPacked, &HphpArray::LvalInt,
    &SharedMap::LvalInt,
    &NameValueTableWrapper::LvalInt,
    &PolicyArray::LvalInt },
  // lvalStr
  { &HphpArray::LvalStrPacked, &HphpArray::LvalStr,
    &SharedMap::LvalStr,
    &NameValueTableWrapper::LvalStr,
    &PolicyArray::LvalStr },
  // lvalNew
  { &HphpArray::LvalNewPacked, &HphpArray::LvalNew,
    &SharedMap::LvalNew,
    &NameValueTableWrapper::LvalNew,
    &PolicyArray::LvalNew },
  // setRefInt
  { &HphpArray::SetRefIntPacked, &HphpArray::SetRefInt,
    &SharedMap::SetRefInt,
    &NameValueTableWrapper::SetRefInt,
    &PolicyArray::SetRefInt },
  // setRefStr
  { &HphpArray::SetRefStrPacked, &HphpArray::SetRefStr,
    &SharedMap::SetRefStr,
    &NameValueTableWrapper::SetRefStr,
    &PolicyArray::SetRefStr },
  // addInt
  { &HphpArray::AddIntPacked, &HphpArray::AddInt,
    &SharedMap::SetInt, // reuse set
    &NameValueTableWrapper::SetInt, // reuse set
    &PolicyArray::AddInt },
  // addStr
  { &HphpArray::SetStrPacked, // reuse set
    &HphpArray::AddStr,
    &SharedMap::SetStr, // reuse set
    &NameValueTableWrapper::SetStr, // reuse set
    &PolicyArray::AddStr },
  // removeInt
  { &HphpArray::RemoveIntPacked, &HphpArray::RemoveInt,
    &SharedMap::RemoveInt,
    &NameValueTableWrapper::RemoveInt,
    &PolicyArray::RemoveInt },
  // removeStr
  { &HphpArray::RemoveStrPacked, &HphpArray::RemoveStr,
    &SharedMap::RemoveStr,
    &NameValueTableWrapper::RemoveStr,
    &PolicyArray::RemoveStr },
  // iterBegin
  { &HphpArray::IterBegin, &HphpArray::IterBegin,
    &SharedMap::IterBegin,
    &NameValueTableWrapper::IterBegin,
    &PolicyArray::IterBegin },
  // iterEnd
  { &HphpArray::IterEnd, &HphpArray::IterEnd,
    &SharedMap::IterEnd,
    &NameValueTableWrapper::IterEnd,
    &PolicyArray::IterEnd },
  // iterAdvance
  { &HphpArray::IterAdvance, &HphpArray::IterAdvance,
    &SharedMap::IterAdvance,
    &NameValueTableWrapper::IterAdvance,
    &PolicyArray::IterAdvance },
  // iterRewind
  { &HphpArray::IterRewind, &HphpArray::IterRewind,
    &SharedMap::IterRewind,
    &NameValueTableWrapper::IterRewind,
    &PolicyArray::IterRewind },
  // validFullPos
  { &HphpArray::ValidFullPos, &HphpArray::ValidFullPos,
    &SharedMap::ValidFullPos,
    &NameValueTableWrapper::ValidFullPos,
    &PolicyArray::ValidFullPos },
  // advanceFullPos
  { &HphpArray::AdvanceFullPos, &HphpArray::AdvanceFullPos,
    &SharedMap::AdvanceFullPos,
    &NameValueTableWrapper::AdvanceFullPos,
    &PolicyArray::AdvanceFullPos },
  // escalateForSort
  { &HphpArray::EscalateForSort, &HphpArray::EscalateForSort,
    &SharedMap::EscalateForSort,
    &NameValueTableWrapper::EscalateForSort,
    &PolicyArray::EscalateForSort },
  // ksort
  { &HphpArray::Ksort, &HphpArray::Ksort,
    &ArrayData::Ksort,
    &NameValueTableWrapper::Ksort,
    &ArrayData::Ksort },
  // sort
  { &HphpArray::Sort, &HphpArray::Sort,
    &ArrayData::Sort,
    &NameValueTableWrapper::Sort,
    &ArrayData::Sort },
  // asort
  { &HphpArray::Asort, &HphpArray::Asort,
    &ArrayData::Asort,
    &NameValueTableWrapper::Asort,
    &ArrayData::Asort },
  // uksort
  { &HphpArray::Uksort, &HphpArray::Uksort,
    &ArrayData::Uksort,
    &NameValueTableWrapper::Uksort,
    &ArrayData::Uksort },
  // usort
  { &HphpArray::Usort, &HphpArray::Usort,
    &ArrayData::Usort,
    &NameValueTableWrapper::Usort,
    &ArrayData::Usort },
  // uasort
  { &HphpArray::Uasort, &HphpArray::Uasort,
    &ArrayData::Uasort,
    &NameValueTableWrapper::Uasort,
    &ArrayData::Uasort },
  // copy
  { &HphpArray::CopyPacked, &HphpArray::Copy,
    &SharedMap::Copy,
    &NameValueTableWrapper::Copy,
    &PolicyArray::Copy },
  // copyWithStrongIterators
  { &HphpArray::CopyWithStrongIterators, &HphpArray::CopyWithStrongIterators,
    &SharedMap::CopyWithStrongIterators,
    &NameValueTableWrapper::CopyWithStrongIterators,
    &PolicyArray::CopyWithStrongIterators },
  // nonSmartCopy
  { &HphpArray::NonSmartCopy, &HphpArray::NonSmartCopy,
    &ArrayData::NonSmartCopy,
    &ArrayData::NonSmartCopy,
    &PolicyArray::NonSmartCopy },
  // append
  { &HphpArray::AppendPacked, &HphpArray::Append,
    &SharedMap::Append,
    &NameValueTableWrapper::Append,
    &PolicyArray::Append },
  // appendRef
  { &HphpArray::AppendRefPacked, &HphpArray::AppendRef,
    &SharedMap::AppendRef,
    &NameValueTableWrapper::AppendRef,
    &PolicyArray::AppendRef },
  // appendWithRef
  { &HphpArray::AppendWithRefPacked, &HphpArray::AppendWithRef,
    &SharedMap::AppendRef,
    &NameValueTableWrapper::AppendRef,
    &PolicyArray::AppendRef },
  // plus
  { &HphpArray::Plus, &HphpArray::Plus,
    &SharedMap::Plus,
    &NameValueTableWrapper::Plus,
    &PolicyArray::Plus },
  // merge
  { &HphpArray::Merge, &HphpArray::Merge,
    &SharedMap::Merge,
    &NameValueTableWrapper::Merge,
    &PolicyArray::Merge },
  // pop
  { &HphpArray::PopPacked, &HphpArray::Pop,
    &SharedMap::Pop,
    &NameValueTableWrapper::Pop,
    &PolicyArray::Pop },
  // dequeue
  { &HphpArray::DequeuePacked, &HphpArray::Dequeue,
    &SharedMap::Dequeue,
    &NameValueTableWrapper::Dequeue,
    &PolicyArray::Dequeue },
  // prepend
  { &HphpArray::PrependPacked, &HphpArray::Prepend,
    &SharedMap::Prepend,
    &NameValueTableWrapper::Prepend,
    &PolicyArray::Prepend },
  // renumber
  { &HphpArray::RenumberPacked, &HphpArray::Renumber,
    &SharedMap::Renumber,
    &NameValueTableWrapper::Renumber,
    &PolicyArray::Renumber },
  // onSetEvalScalar
  { &HphpArray::OnSetEvalScalarPacked, &HphpArray::OnSetEvalScalar,
    &SharedMap::OnSetEvalScalar,
    &NameValueTableWrapper::OnSetEvalScalar,
    &PolicyArray::OnSetEvalScalar },
  // escalate
  { &ArrayData::Escalate, &ArrayData::Escalate,
    &SharedMap::Escalate,
    &ArrayData::Escalate,
    &PolicyArray::Escalate },
  // getSharedVariant
  { &ArrayData::GetSharedVariant, &ArrayData::GetSharedVariant,
    &SharedMap::GetSharedVariant,
    &ArrayData::GetSharedVariant,
    &ArrayData::GetSharedVariant },
  // zSetInt
  { &HphpArray::ZSetInt, &HphpArray::ZSetInt,
    &ArrayData::ZSetInt,
    &ArrayData::ZSetInt,
    &ArrayData::ZSetInt },
  // zSetStr
  { &HphpArray::ZSetStr, &HphpArray::ZSetStr,
    &ArrayData::ZSetStr,
    &ArrayData::ZSetStr,
    &ArrayData::ZSetStr },
  // zAppend
  { &HphpArray::ZAppend, &HphpArray::ZAppend,
    &ArrayData::ZAppend,
    &ArrayData::ZAppend,
    &ArrayData::ZAppend },
};

///////////////////////////////////////////////////////////////////////////////

// In general, arrays can contain int-valued-strings, even though
// plain array access converts them to integers.  non-int-string
// assersions should go upstream of the ArrayData api.

bool ArrayData::IsValidKey(CStrRef k) {
  return IsValidKey(k.get());
}

bool ArrayData::IsValidKey(CVarRef k) {
  return k.isInteger() ||
         (k.isString() && IsValidKey(k.getStringData()));
}

// constructors/destructors

HOT_FUNC
ArrayData *ArrayData::Create() {
  return ArrayInit((ssize_t)0).create();
}

HOT_FUNC
ArrayData *ArrayData::Create(CVarRef value) {
  ArrayInit init(1);
  init.set(value);
  return init.create();
}

ArrayData *ArrayData::Create(CVarRef name, CVarRef value) {
  ArrayInit init(1);
  // There is no toKey() call on name.
  init.set(name, value, true);
  return init.create();
}

ArrayData *ArrayData::CreateRef(CVarRef value) {
  ArrayInit init(1);
  init.setRef(value);
  return init.create();
}

ArrayData *ArrayData::CreateRef(CVarRef name, CVarRef value) {
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

Object ArrayData::toObject() const {
  return ObjectData::FromArray(const_cast<ArrayData*>(this));
}

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
    return a->remove(a->getKey(pos), a->getCount() > 1);
  }
  value = uninit_null();
  return a;
}

ArrayData *ArrayData::Dequeue(ArrayData* a, Variant &value) {
  if (!a->empty()) {
    auto const pos = a->iter_begin();
    value = a->getValue(pos);
    ArrayData *ret = a->remove(a->getKey(pos), a->getCount() > 1);

    // In PHP, array_shift() will cause all numerically key-ed values re-keyed
    ret->renumber();
    return ret;
  }
  value = uninit_null();
  return a;
}

///////////////////////////////////////////////////////////////////////////////
// MutableArrayIter related functions

void ArrayData::newFullPos(FullPos &fp) {
  assert(!fp.getContainer());
  fp.setContainer(this);
  fp.setNext(strongIterators());
  setStrongIterators(&fp);
  fp.m_pos = m_pos;
}

void ArrayData::freeFullPos(FullPos &fp) {
  assert(strongIterators() && fp.getContainer() == this);
  // search for fp in our list, then remove it. Usually its the first one.
  FullPos* p = strongIterators();
  if (p == &fp) {
    setStrongIterators(p->getNext());
    fp.setContainer(nullptr);
    return;
  }
  for (; p->getNext(); p = p->getNext()) {
    if (p->getNext() == &fp) {
      p->setNext(p->getNext()->getNext());
      fp.setContainer(nullptr);
      return;
    }
  }
  // If the strong iterator list was empty or if fp could not be
  // found in the strong iterator list, then we are in a bad state
  assert(false);
}

void ArrayData::freeStrongIterators() {
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    r.front()->setContainer(nullptr);
  }
  setStrongIterators(nullptr);
}

void ArrayData::moveStrongIterators(ArrayData* dest, ArrayData* src) {
  for (FullPosRange r(src->strongIterators()); !r.empty(); r.popFront()) {
    r.front()->setContainer(dest);
  }
  // move pointer to list and flag in one copy
  dest->m_strongIterators = src->m_strongIterators;
  src->m_strongIterators = 0;
}

CVarRef ArrayData::endRef() {
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

void ArrayData::Uksort(ArrayData*, CVarRef cmp_function) {
  throw FatalErrorException("Unimplemented ArrayData::uksort");
}

void ArrayData::Usort(ArrayData*, CVarRef cmp_function) {
  throw FatalErrorException("Unimplemented ArrayData::usort");
}

void ArrayData::Uasort(ArrayData*, CVarRef cmp_function) {
  throw FatalErrorException("Unimplemented ArrayData::uasort");
}

ArrayData* ArrayData::CopyWithStrongIterators(const ArrayData* ad) {
  throw FatalErrorException("Unimplemented ArrayData::copyWithStrongIterators");
}

void ArrayData::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  throw FatalErrorException("Unimplemented ArrayData::ZSetInt");
}

void ArrayData::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  throw FatalErrorException("Unimplemented ArrayData::ZSetStr");
}

void ArrayData::ZAppend(ArrayData* ad, RefData* v) {
  throw FatalErrorException("Unimplemented ArrayData::ZAppend");
}

///////////////////////////////////////////////////////////////////////////////
// Default implementation of position-based iterations.

Variant ArrayData::reset() {
  m_pos = iter_begin();
  return m_pos != invalid_index ? getValue(m_pos) : Variant(false);
}

Variant ArrayData::prev() {
  if (m_pos != invalid_index) {
    m_pos = iter_rewind(m_pos);
    if (m_pos != invalid_index) {
      return getValue(m_pos);
    }
  }
  return Variant(false);
}

Variant ArrayData::next() {
  if (m_pos != invalid_index) {
    m_pos = iter_advance(m_pos);
    if (m_pos != invalid_index) {
      return getValue(m_pos);
    }
  }
  return Variant(false);
}

Variant ArrayData::end() {
  m_pos = iter_end();
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
    m_pos = iter_advance(m_pos);
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

bool ArrayData::hasInternalReference(PointerSet &vars,
                                     bool ds /* = false */) const {
  if (isSharedMap()) return false;
  for (ArrayIter iter(this); iter; ++iter) {
    CVarRef var = iter.secondRef();
    if (var.isReferenced()) {
      Variant *pvar = var.getRefData();
      if (vars.find(pvar) != vars.end()) {
        return true;
      }
      vars.insert(pvar);
    }
    if (var.isObject()) {
      ObjectData *pobj = var.getObjectData();
      if (vars.find(pobj) != vars.end()) {
        return true;
      }
      vars.insert(pobj);
      if (ds && pobj->instanceof(SystemLib::s_SerializableClass)) {
        return true;
      }
      if (pobj->hasInternalReference(vars, ds)) {
        return true;
      }
    } else if (var.isArray() &&
               var.getArrayData()->hasInternalReference(vars, ds)) {
      return true;
    }
  }
  return false;
}

CVarRef ArrayData::get(CVarRef k, bool error) const {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? get(getIntKey(cell), error)
                        : get(getStringKey(cell), error);
}

CVarRef ArrayData::getNotFound(int64_t k) {
  raise_notice("Undefined index: %" PRId64, k);
  return null_variant;
}

CVarRef ArrayData::getNotFound(const StringData* k) {
  raise_notice("Undefined index: %s", k->data());
  return null_variant;
}

CVarRef ArrayData::getNotFound(int64_t k, bool error) const {
  return error && m_kind != kNvtwKind ? getNotFound(k) :
         null_variant;
}

CVarRef ArrayData::getNotFound(const StringData* k, bool error) const {
  return error && m_kind != kNvtwKind ? getNotFound(k) :
         null_variant;
}

CVarRef ArrayData::getNotFound(CStrRef k) {
  raise_notice("Undefined index: %s", k.data());
  return null_variant;
}

CVarRef ArrayData::getNotFound(CVarRef k) {
  raise_notice("Undefined index: %s", k.toString().data());
  return null_variant;
}

void ArrayData::Renumber(ArrayData*) {
}

void ArrayData::OnSetEvalScalar(ArrayData*) {
  assert(false);
}

ArrayData* ArrayData::Escalate(const ArrayData* ad) {
  return const_cast<ArrayData*>(ad);
}

const char* ArrayData::kindToString(ArrayKind kind) {
  const char* names[] = {
    "PackedKind",
    "MixedKind",
    "SharedKind",
    "NvtwKind",
    "PolicyKind"
  };
  return names[kind];
}

void ArrayData::dump() {
  string out; dump(out); fwrite(out.c_str(), out.size(), 1, stdout);
}

void ArrayData::dump(std::string &out) {
  VariableSerializer vs(VariableSerializer::Type::VarDump);
  String ret(vs.serialize(Array(this), true));
  out += "ArrayData(";
  out += boost::lexical_cast<string>(m_count);
  out += "): ";
  out += string(ret.data(), ret.size());
}

void ArrayData::dump(std::ostream &out) {
  unsigned int i = 0;
  for (ArrayIter iter(this); iter; ++iter, i++) {
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    Variant key(iter.first());
    out << i << " #### " << key.toString()->toCPPString() << " #### ";
    Variant val(iter.second());
    try {
      String valS(vs.serialize(val, true));
      out << valS->toCPPString();
    } catch (const Exception &e) {
      out << "Exception: " << e.what();
    }
    out << std::endl;
  }
}

void ArrayData::getChildren(std::vector<TypedValue *> &out) {
  if (isSharedMap()) {
    SharedMap *sm = static_cast<SharedMap *>(this);
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
