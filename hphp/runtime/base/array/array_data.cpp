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
#include "hphp/runtime/base/array/array_data.h"

#include "tbb/concurrent_hash_map.h"

#include "hphp/util/exception.h"
#include "hphp/runtime/base/array/array_init.h"
#include "hphp/runtime/base/array/array_iterator.h"
#include "hphp/runtime/base/type_conversions.h"
#include "hphp/runtime/base/builtin_functions.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/variable_serializer.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/shared/shared_map.h"
#include "hphp/runtime/base/array/policy_array.h"
#include "hphp/runtime/base/comparisons.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static_assert(
  sizeof(ArrayData) == 32,
  "Performance is sensitive to sizeof(ArrayData)."
  " Make sure you changed it with good reason and then update this assert.");

typedef tbb::concurrent_hash_map<const StringData *, ArrayData *,
                                 StringDataHashCompare> ArrayDataMap;
static ArrayDataMap s_arrayDataMap;

ArrayData *ArrayData::GetScalarArray(ArrayData *arr,
                                     const StringData *key /* = nullptr */) {
  if (!key) {
    key = StringData::GetStaticString(f_serialize(arr).get());
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

ArrayData *ArrayData::nonSmartCopy() const {
  throw FatalErrorException("nonSmartCopy not implemented.");
}

HOT_FUNC
void ArrayData::release() {
  if (isHphpArray()) {
    HphpArray* that = static_cast<HphpArray*>(this);
    that->release();
    return;
  }
  if (isSharedMap()) {
    SharedMap* that = static_cast<SharedMap*>(this);
    that->release();
    return;
  }
  if (isPolicyArray()) {
    auto that = static_cast<PolicyArray*>(this);
    that->release();
    return;
  }
  assert(m_kind == ArrayKind::kNameValueTableWrapper);
  // NameValueTableWrapper: nop.
}

///////////////////////////////////////////////////////////////////////////////
// reads

Object ArrayData::toObject() const {
  return Instance::FromArray(const_cast<ArrayData *>(this));
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

ArrayData *ArrayData::createLvalPtr(StringData* k, Variant *&ret, bool copy) {
  throw FatalErrorException("Unimplemented ArrayData::createLvalPtr");
}

ArrayData *ArrayData::getLvalPtr(StringData* k, Variant *&ret, bool copy) {
  throw FatalErrorException("Unimplemented ArrayData::getLvalPtr");
}

ArrayData *ArrayData::add(int64_t k, CVarRef v, bool copy) {
  assert(!exists(k));
  return set(k, v, copy);
}

ArrayData *ArrayData::add(StringData* k, CVarRef v, bool copy) {
  assert(!exists(k));
  return set(k, v, copy);
}

ArrayData *ArrayData::addLval(int64_t k, Variant *&ret, bool copy) {
  assert(!exists(k));
  return lval(k, ret, copy);
}

ArrayData *ArrayData::addLval(StringData* k, Variant *&ret, bool copy) {
  assert(!exists(k));
  return lval(k, ret, copy);
}

///////////////////////////////////////////////////////////////////////////////
// stack and queue operations

ArrayData *ArrayData::pop(Variant &value) {
  if (!empty()) {
    ssize_t pos = iter_end();
    value = getValue(pos);
    return remove(getKey(pos), getCount() > 1);
  }
  value = uninit_null();
  return this;
}

ArrayData *ArrayData::dequeue(Variant &value) {
  if (!empty()) {
    auto const pos = iter_begin();
    value = getValue(pos);
    ArrayData *ret = remove(getKey(pos), getCount() > 1);

    // In PHP, array_shift() will cause all numerically key-ed values re-keyed
    ret->renumber();
    return ret;
  }
  value = uninit_null();
  return this;
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
  assert(strongIterators() != 0 && fp.getContainer() == (ArrayData*)this);
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
  setStrongIterators(0);
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

ArrayData* ArrayData::escalateForSort() {
  if (getCount() > 1) {
    return copy();
  }
  return this;
}
void ArrayData::ksort(int sort_flags, bool ascending) {
  throw FatalErrorException("Unimplemented ArrayData::ksort");
}
void ArrayData::sort(int sort_flags, bool ascending) {
  throw FatalErrorException("Unimplemented ArrayData::sort");
}
void ArrayData::asort(int sort_flags, bool ascending) {
  throw FatalErrorException("Unimplemented ArrayData::asort");
}
void ArrayData::uksort(CVarRef cmp_function) {
  throw FatalErrorException("Unimplemented ArrayData::uksort");
}
void ArrayData::usort(CVarRef cmp_function) {
  throw FatalErrorException("Unimplemented ArrayData::usort");
}
void ArrayData::uasort(CVarRef cmp_function) {
  throw FatalErrorException("Unimplemented ArrayData::uasort");
}
ArrayData* ArrayData::copyWithStrongIterators() const {
  throw FatalErrorException("Unimplemented ArrayData::copyWithStrongIterators");
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
  return error && m_kind != ArrayKind::kNameValueTableWrapper ? getNotFound(k) :
         null_variant;
}

CVarRef ArrayData::getNotFound(const StringData* k, bool error) const {
  return error && m_kind != ArrayKind::kNameValueTableWrapper ? getNotFound(k) :
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

void ArrayData::dump() {
  string out; dump(out); fwrite(out.c_str(), out.size(), 1, stdout);
}

void ArrayData::dump(std::string &out) {
  VariableSerializer vs(VariableSerializer::Type::VarDump);
  String ret(vs.serialize(Array(this), true));
  out += "ArrayData(";
  out += boost::lexical_cast<string>(_count);
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
