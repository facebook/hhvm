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

#include <tbb/concurrent_hash_map.h>

#include "hphp/util/exception.h"
#include "hphp/runtime/base/array-init.h"
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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static_assert(
  sizeof(ArrayData) == 24,
  "Performance is sensitive to sizeof(ArrayData)."
  " Make sure you changed it with good reason and then update this assert.");

typedef tbb::concurrent_hash_map<std::string, ArrayData*> ArrayDataMap;
static ArrayDataMap s_arrayDataMap;

ArrayData* ArrayData::GetScalarArray(ArrayData* arr) {
  auto key = f_serialize(arr).toCppString();
  return GetScalarArray(arr, key);
}

ArrayData *ArrayData::GetScalarArray(ArrayData *arr, const std::string& key) {
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
    &NameValueTableWrapper::Release,
    &ProxyArray::Release },
  // nvGetInt
  { &HphpArray::NvGetIntPacked, &HphpArray::NvGetInt,
    &APCLocalArray::NvGetInt,
    &NameValueTableWrapper::NvGetInt,
    &ProxyArray::NvGetInt },
  // nvGetStr
  { &HphpArray::NvGetStrPacked, &HphpArray::NvGetStr,
    &APCLocalArray::NvGetStr,
    &NameValueTableWrapper::NvGetStr,
    &ProxyArray::NvGetStr },
  // nvGetKey
  { &HphpArray::NvGetKeyPacked, &HphpArray::NvGetKey,
    &APCLocalArray::NvGetKey,
    &NameValueTableWrapper::NvGetKey,
    &ProxyArray::NvGetKey },
  // setInt
  { &HphpArray::SetIntPacked, &HphpArray::SetInt,
    &APCLocalArray::SetInt,
    &NameValueTableWrapper::SetInt,
    &ProxyArray::SetInt },
  // setStr
  { &HphpArray::SetStrPacked, &HphpArray::SetStr,
    &APCLocalArray::SetStr,
    &NameValueTableWrapper::SetStr,
    &ProxyArray::SetStr },
  // vsize
  { &VsizeNop, &VsizeNop,
    &VsizeNop,
    &NameValueTableWrapper::Vsize,
    &ProxyArray::Vsize },
  // getValueRef
  { &HphpArray::GetValueRef, &HphpArray::GetValueRef,
    &APCLocalArray::GetValueRef,
    &NameValueTableWrapper::GetValueRef,
    &ProxyArray::GetValueRef },
  // noCopyOnWrite
  { false, false,
    false,
    true, // NameValueTableWrapper doesn't support COW.
    false },
  // isVectorData
  { &HphpArray::IsVectorDataPacked, &HphpArray::IsVectorData,
    &APCLocalArray::IsVectorData,
    &NameValueTableWrapper::IsVectorData,
    &ProxyArray::IsVectorData },
  // existsInt
  { &HphpArray::ExistsIntPacked, &HphpArray::ExistsInt,
    &APCLocalArray::ExistsInt,
    &NameValueTableWrapper::ExistsInt,
    &ProxyArray::ExistsInt },
  // existsStr
  { &HphpArray::ExistsStrPacked, &HphpArray::ExistsStr,
    &APCLocalArray::ExistsStr,
    &NameValueTableWrapper::ExistsStr,
    &ProxyArray::ExistsStr },
  // lvalInt
  { &HphpArray::LvalIntPacked, &HphpArray::LvalInt,
    &APCLocalArray::LvalInt,
    &NameValueTableWrapper::LvalInt,
    &ProxyArray::LvalInt },
  // lvalStr
  { &HphpArray::LvalStrPacked, &HphpArray::LvalStr,
    &APCLocalArray::LvalStr,
    &NameValueTableWrapper::LvalStr,
    &ProxyArray::LvalStr },
  // lvalNew
  { &HphpArray::LvalNewPacked, &HphpArray::LvalNew,
    &APCLocalArray::LvalNew,
    &NameValueTableWrapper::LvalNew,
    &ProxyArray::LvalNew },
  // setRefInt
  { &HphpArray::SetRefIntPacked, &HphpArray::SetRefInt,
    &APCLocalArray::SetRefInt,
    &NameValueTableWrapper::SetRefInt,
    &ProxyArray::SetRefInt },
  // setRefStr
  { &HphpArray::SetRefStrPacked, &HphpArray::SetRefStr,
    &APCLocalArray::SetRefStr,
    &NameValueTableWrapper::SetRefStr,
    &ProxyArray::SetRefStr },
  // addInt
  { &HphpArray::AddIntPacked, &HphpArray::AddInt,
    &APCLocalArray::SetInt, // reuse set
    &NameValueTableWrapper::SetInt, // reuse set
    &ProxyArray::SetInt }, // reuse set
  // addStr
  { &HphpArray::SetStrPacked, // reuse set
    &HphpArray::AddStr,
    &APCLocalArray::SetStr, // reuse set
    &NameValueTableWrapper::SetStr, // reuse set
    &ProxyArray::SetStr }, // reuse set
  // removeInt
  { &HphpArray::RemoveIntPacked, &HphpArray::RemoveInt,
    &APCLocalArray::RemoveInt,
    &NameValueTableWrapper::RemoveInt,
    &ProxyArray::RemoveInt },
  // removeStr
  { &HphpArray::RemoveStrPacked, &HphpArray::RemoveStr,
    &APCLocalArray::RemoveStr,
    &NameValueTableWrapper::RemoveStr,
    &ProxyArray::RemoveStr },
  // iterBegin
  { &HphpArray::IterBegin, &HphpArray::IterBegin,
    &APCLocalArray::IterBegin,
    &NameValueTableWrapper::IterBegin,
    &ProxyArray::IterBegin },
  // iterEnd
  { &HphpArray::IterEnd, &HphpArray::IterEnd,
    &APCLocalArray::IterEnd,
    &NameValueTableWrapper::IterEnd,
    &ProxyArray::IterEnd },
  // iterAdvance
  { &HphpArray::IterAdvance, &HphpArray::IterAdvance,
    &APCLocalArray::IterAdvance,
    &NameValueTableWrapper::IterAdvance,
    &ProxyArray::IterAdvance },
  // iterRewind
  { &HphpArray::IterRewind, &HphpArray::IterRewind,
    &APCLocalArray::IterRewind,
    &NameValueTableWrapper::IterRewind,
    &ProxyArray::IterRewind },
  // validFullPos
  { &HphpArray::ValidFullPos, &HphpArray::ValidFullPos,
    &APCLocalArray::ValidFullPos,
    &NameValueTableWrapper::ValidFullPos,
    &ProxyArray::ValidFullPos },
  // advanceFullPos
  { &HphpArray::AdvanceFullPos, &HphpArray::AdvanceFullPos,
    &APCLocalArray::AdvanceFullPos,
    &NameValueTableWrapper::AdvanceFullPos,
    &ProxyArray::AdvanceFullPos },
  // escalateForSort
  { &HphpArray::EscalateForSort, &HphpArray::EscalateForSort,
    &APCLocalArray::EscalateForSort,
    &NameValueTableWrapper::EscalateForSort,
    &ProxyArray::EscalateForSort },
  // ksort
  { &HphpArray::Ksort, &HphpArray::Ksort,
    &ArrayData::Ksort,
    &NameValueTableWrapper::Ksort,
    &ProxyArray::Ksort },
  // sort
  { &HphpArray::Sort, &HphpArray::Sort,
    &ArrayData::Sort,
    &NameValueTableWrapper::Sort,
    &ProxyArray::Sort },
  // asort
  { &HphpArray::Asort, &HphpArray::Asort,
    &ArrayData::Asort,
    &NameValueTableWrapper::Asort,
    &ProxyArray::Asort },
  // uksort
  { &HphpArray::Uksort, &HphpArray::Uksort,
    &ArrayData::Uksort,
    &NameValueTableWrapper::Uksort,
    &ProxyArray::Uksort },
  // usort
  { &HphpArray::Usort, &HphpArray::Usort,
    &ArrayData::Usort,
    &NameValueTableWrapper::Usort,
    &ProxyArray::Usort },
  // uasort
  { &HphpArray::Uasort, &HphpArray::Uasort,
    &ArrayData::Uasort,
    &NameValueTableWrapper::Uasort,
    &ProxyArray::Uasort },
  // copy
  { &HphpArray::CopyPacked, &HphpArray::Copy,
    &APCLocalArray::Copy,
    &NameValueTableWrapper::Copy,
    &ProxyArray::Copy },
  // copyWithStrongIterators
  { &HphpArray::CopyWithStrongIterators, &HphpArray::CopyWithStrongIterators,
    &APCLocalArray::CopyWithStrongIterators,
    &NameValueTableWrapper::CopyWithStrongIterators,
    &ProxyArray::CopyWithStrongIterators },
  // nonSmartCopy
  { &HphpArray::NonSmartCopy, &HphpArray::NonSmartCopy,
    &ArrayData::NonSmartCopy,
    &ArrayData::NonSmartCopy,
    &ProxyArray::NonSmartCopy },
  // append
  { &HphpArray::AppendPacked, &HphpArray::Append,
    &APCLocalArray::Append,
    &NameValueTableWrapper::Append,
    &ProxyArray::Append },
  // appendRef
  { &HphpArray::AppendRefPacked, &HphpArray::AppendRef,
    &APCLocalArray::AppendRef,
    &NameValueTableWrapper::AppendRef,
    &ProxyArray::AppendRef },
  // appendWithRef
  { &HphpArray::AppendWithRefPacked, &HphpArray::AppendWithRef,
    &APCLocalArray::AppendRef,
    &NameValueTableWrapper::AppendRef,
    &ProxyArray::AppendRef },
  // plusEq
  { &HphpArray::PlusEq, &HphpArray::PlusEq,
    &APCLocalArray::PlusEq,
    &NameValueTableWrapper::PlusEq,
    &ProxyArray::PlusEq },
  // merge
  { &HphpArray::Merge, &HphpArray::Merge,
    &APCLocalArray::Merge,
    &NameValueTableWrapper::Merge,
    &ProxyArray::Merge },
  // pop
  { &HphpArray::PopPacked, &HphpArray::Pop,
    &APCLocalArray::Pop,
    &NameValueTableWrapper::Pop,
    &ProxyArray::Pop },
  // dequeue
  { &HphpArray::DequeuePacked, &HphpArray::Dequeue,
    &APCLocalArray::Dequeue,
    &NameValueTableWrapper::Dequeue,
    &ProxyArray::Dequeue },
  // prepend
  { &HphpArray::PrependPacked, &HphpArray::Prepend,
    &APCLocalArray::Prepend,
    &NameValueTableWrapper::Prepend,
    &ProxyArray::Prepend },
  // renumber
  { &HphpArray::RenumberPacked, &HphpArray::Renumber,
    &APCLocalArray::Renumber,
    &NameValueTableWrapper::Renumber,
    &ProxyArray::Renumber },
  // onSetEvalScalar
  { &HphpArray::OnSetEvalScalarPacked, &HphpArray::OnSetEvalScalar,
    &APCLocalArray::OnSetEvalScalar,
    &NameValueTableWrapper::OnSetEvalScalar,
    &ProxyArray::OnSetEvalScalar },
  // escalate
  { &ArrayData::Escalate, &ArrayData::Escalate,
    &APCLocalArray::Escalate,
    &ArrayData::Escalate,
    &ProxyArray::Escalate },
  // getAPCHandle
  { &ArrayData::GetAPCHandle, &ArrayData::GetAPCHandle,
    &APCLocalArray::GetAPCHandle,
    &ArrayData::GetAPCHandle,
    &ProxyArray::GetAPCHandle },
  // zSetInt
  { &HphpArray::ZSetInt, &HphpArray::ZSetInt,
    &ArrayData::ZSetInt,
    &ArrayData::ZSetInt,
    &ProxyArray::ZSetInt },
  // zSetStr
  { &HphpArray::ZSetStr, &HphpArray::ZSetStr,
    &ArrayData::ZSetStr,
    &ArrayData::ZSetStr,
    &ProxyArray::ZSetStr },
  // zAppend
  { &HphpArray::ZAppend, &HphpArray::ZAppend,
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

bool ArrayData::IsValidKey(CVarRef k) {
  return k.isInteger() ||
         (k.isString() && IsValidKey(k.getStringData()));
}

// constructors/destructors

ArrayData *ArrayData::Create() {
  return ArrayInit((ssize_t)0).create();
}

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

bool ArrayData::Uksort(ArrayData*, CVarRef cmp_function) {
  throw FatalErrorException("Unimplemented ArrayData::uksort");
}

bool ArrayData::Usort(ArrayData*, CVarRef cmp_function) {
  throw FatalErrorException("Unimplemented ArrayData::usort");
}

bool ArrayData::Uasort(ArrayData*, CVarRef cmp_function) {
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

CVarRef ArrayData::getNotFound(const String& k) {
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
