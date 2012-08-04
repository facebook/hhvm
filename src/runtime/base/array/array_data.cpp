/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/array/array_data.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/array/zend_array.h>
#include <runtime/base/array/vector_array.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/macros.h>
#include <util/exception.h>
#include <tbb/concurrent_hash_map.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef tbb::concurrent_hash_map<const StringData *, ArrayData *,
                                 StringDataHashCompare> ArrayDataMap;
static ArrayDataMap s_arrayDataMap;

ArrayData *ArrayData::GetScalarArray(ArrayData *arr,
                                     const StringData *key /* = NULL */) {
  if (key == NULL) {
    key = StringData::GetStaticString(f_serialize(arr).get());
  } else {
    ASSERT(key->isStatic());
    ASSERT(key->same(f_serialize(arr).get()));
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

// constructors/destructors

HOT_FUNC
ArrayData *ArrayData::Create() {
  if (enable_vector_array && RuntimeOption::UseVectorArray) {
    return StaticEmptyVectorArray::Get();
  }
  return ArrayInit((ssize_t)0).create();
}

HOT_FUNC
ArrayData *ArrayData::Create(CVarRef value) {
  if (enable_vector_array && RuntimeOption::UseVectorArray) {
    VectorArray *va = NEW(VectorArray)(1);
    va->VectorArray::append(value, false);
    va->m_pos = 0;
    return va;
  }
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
  if (enable_vector_array && RuntimeOption::UseVectorArray) {
    VectorArray *va = NEW(VectorArray)(1);
    va->VectorArray::appendRef(value, false);
    va->m_pos = 0;
    return va;
  }
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

HOT_FUNC
ArrayData::~ArrayData() {
  // If there are any strong iterators pointing to this array, they need
  // to be invalidated.
  if (strongIterators()) {
    freeStrongIterators();
  }
}

ArrayData *ArrayData::nonSmartCopy() const {
  throw FatalErrorException("nonSmartCopy not implemented.");
}

///////////////////////////////////////////////////////////////////////////////
// reads

Object ArrayData::toObject() const {
  return hhvm
         ? VM::Instance::FromArray(const_cast<ArrayData *>(this))
         : ObjectData::FromArray(const_cast<ArrayData *>(this));
}

bool ArrayData::isVectorData() const {
  for (ssize_t i = 0, n = size(); i < n; i++) {
    if (getIndex(i) != i) {
      return false;
    }
  }
  return true;
}

int ArrayData::compare(const ArrayData *v2) const {
  ASSERT(v2);

  int count1 = size();
  int count2 = v2->size();
  if (count1 < count2) return -1;
  if (count1 > count2) return 1;
  if (count1 == 0) return 0;

  // prevent circular referenced objects/arrays or deep ones
  DECLARE_THREAD_INFO; check_recursion(info);

  for (ArrayIter iter(this); iter; ++iter) {
    Variant key(iter.first());
    if (!v2->exists(key)) return 1;

    Variant value1(iter.second());
    Variant value2(v2->get(key));
    if (value1.more(value2)) return 1;
    if (value1.less(value2)) return -1;
  }

  return 0;
}

bool ArrayData::equal(const ArrayData *v2, bool strict) const {
  ASSERT(v2);

  int count1 = size();
  int count2 = v2->size();
  if (count1 != count2) return false;
  if (count1 == 0) return true;

  // prevent circular referenced objects/arrays or deep ones
  DECLARE_THREAD_INFO; check_recursion(info);

  if (strict) {
    for (ArrayIter iter1(this), iter2(v2); iter1 && iter2; ++iter1, ++iter2) {
      Variant key1(iter1.first());
      Variant key2(iter2.first());
      if (!key1.same(key2)) return false;

      Variant value1(iter1.second());
      Variant value2(iter2.second());
      if (!value1.same(value2)) return false;
    }
  } else {
    for (ArrayIter iter(this); iter; ++iter) {
      Variant key(iter.first());
      if (!v2->exists(key)) return false;

      Variant value1(iter.second());
      Variant value2(v2->get(key));
      if (!value1.equal(value2)) return false;
    }
  }

  return true;
}

ArrayData *ArrayData::lvalPtr(CStrRef k, Variant *&ret, bool copy,
                              bool create) {
  throw FatalErrorException("Unimplemented ArrayData::lvalPtr");
}

ArrayData *ArrayData::lvalPtr(int64 k, Variant *&ret, bool copy,
                              bool create) {
  throw FatalErrorException("Unimplemented ArrayData::lvalPtr");
}

ArrayData *ArrayData::add(int64 k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  return set(k, v, copy);
}

ArrayData *ArrayData::add(CStrRef k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  return set(k, v, copy);
}

ArrayData *ArrayData::add(CVarRef k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  return set(k, v, copy);
}

ArrayData *ArrayData::addLval(int64 k, Variant *&ret, bool copy) {
  ASSERT(!exists(k));
  return lval(k, ret, copy);
}

ArrayData *ArrayData::addLval(CStrRef k, Variant *&ret, bool copy) {
  ASSERT(!exists(k));
  return lval(k, ret, copy);
}

ArrayData *ArrayData::addLval(CVarRef k, Variant *&ret, bool copy) {
  ASSERT(!exists(k));
  return lval(k, ret, copy);
}

ArrayData *ArrayData::set(litstr  k, CVarRef v, bool copy) {
  return set(String(k), v, copy);
}

ArrayData *ArrayData::setRef(litstr  k, CVarRef v, bool copy) {
  return setRef(String(k), v, copy);
}

ArrayData *ArrayData::remove(litstr  k, bool copy) {
  return remove(String(k), copy);
}

///////////////////////////////////////////////////////////////////////////////
// stack and queue operations

ArrayData *ArrayData::pop(Variant &value) {
  if (!empty()) {
    ssize_t pos = iter_end();
    value = getValue(pos);
    return remove(getKey(pos), getCount() > 1);
  }
  value = null;
  return NULL;
}

ArrayData *ArrayData::dequeue(Variant &value) {
  if (!empty()) {
    ssize_t pos = iter_begin();
    value = getValue(pos);
    ArrayData *ret = remove(getKey(pos), getCount() > 1);

    // In PHP, array_shift() will cause all numerically key-ed values re-keyed
    if (ret) {
      ret->renumber();
    } else {
      renumber();
    }

    return ret;
  }
  value = null;
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// MutableArrayIter related functions

void ArrayData::newFullPos(FullPos &fp) {
  ASSERT(fp.container == NULL);
  fp.container = this;
  fp.next = strongIterators();
  setStrongIterators(&fp);
  getFullPos(fp);
}

void ArrayData::freeFullPos(FullPos &fp) {
  ASSERT(strongIterators() != 0 && fp.container == (ArrayData*)this);
  // search for fp in our list, then remove it.  Usually its the first one.
  FullPos* p = strongIterators();
  if (p == &fp) {
    setStrongIterators(p->next);
    fp.container = NULL;
    return;
  }
  for (; p->next; p = p->next) {
    if (p->next == &fp) {
      p->next = p->next->next;
      fp.container = NULL;
      return;
    }
  }
  // If the strong iterator list was empty or if fp could not be
  // found in the strong iterator list, then we are in a bad state
  ASSERT(false);
}

void ArrayData::getFullPos(FullPos &fp) {
  ASSERT(fp.container == (ArrayData*)this);
  fp.pos = ArrayData::invalid_index;
}

bool ArrayData::setFullPos(const FullPos &fp) {
  ASSERT(fp.container == (ArrayData*)this);
  return false;
}

void ArrayData::freeStrongIterators() {
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    r.front()->container = NULL;
  }
  setStrongIterators(0);
}

void ArrayData::moveStrongIterators(ArrayData* dest, ArrayData* src) {
  for (FullPosRange r(src->strongIterators()); !r.empty(); r.popFront()) {
    r.front()->container = dest;
  }
  // move pointer to list and flag in one copy
  dest->m_strongIterators = src->m_strongIterators;
  src->m_strongIterators = 0;
}

CVarRef ArrayData::currentRef() {
  if (m_pos >= 0 && m_pos < size()) {
    return getValueRef(m_pos);
  }
  throw FatalErrorException("invalid ArrayData::m_pos");
}
CVarRef ArrayData::endRef() {
  if (m_pos >= 0 && m_pos < size()) {
    return getValueRef(size() - 1);
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

Variant ArrayData::reset()         { return value(m_pos = 0);}
Variant ArrayData::prev()          { return value(--m_pos);}
Variant ArrayData::next()          { return value(++m_pos);}
Variant ArrayData::end()           { return value(m_pos = size() - 1);}

Variant ArrayData::key() const {
  if (m_pos >= 0 && m_pos < size()) {
    return getKey(m_pos);
  }
  return null;
}

Variant ArrayData::value(ssize_t &pos) const {
  if (pos >= 0 && pos < size()) {
    return getValue(pos);
  }
  pos = ArrayData::invalid_index;
  return false;
}

Variant ArrayData::current() const {
  if (m_pos >= 0 && m_pos < size()) {
    return getValue(m_pos);
  }
  return false;
}

static StaticString s_value("value");
static StaticString s_key("key");

Variant ArrayData::each() {
  if (m_pos >= 0 && m_pos < size()) {
    Array ret;
    Variant key(getKey(m_pos));
    Variant value(getValue(m_pos));
    ret.set(1, value);
    ret.set(s_value, value);
    ret.set(0, key);
    ret.set(s_key, key);
    ++m_pos;
    return ret;
  }
  return false;
}

ssize_t ArrayData::iter_begin() const {
  if (empty()) return ArrayData::invalid_index;
  return 0;
}

ssize_t ArrayData::iter_end() const {
  if (empty()) return ArrayData::invalid_index;
  return size() - 1;
}

ssize_t ArrayData::iter_advance(ssize_t prev) const {
  ASSERT(prev >= 0 && prev < size());
  ssize_t next = prev + 1;
  if (next >= size()) return ArrayData::invalid_index;
  return next;
}

ssize_t ArrayData::iter_rewind(ssize_t prev) const {
  ASSERT(prev >= 0 && prev < size());
  ssize_t next = prev - 1;
  if (next < 0) return ArrayData::invalid_index;
  return next;
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
      if (pobj->o_instanceof("Serializable")) {
        if (ds) {
          // We want to detect Serializable object as well
          return true;
        } else {
          // Serializable object does not have internal reference issue
          return false;
        }
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

// nvGet has to search twice when using the ArrayData api so as not to
// conflate no-key with have-key && value == null_varaint.  Subclasses
// can easily do this with one key search.

TypedValue* ArrayData::nvGet(int64 k) const {
  return exists(k) ? (TypedValue*)&get(k, false) :
         NULL;
}

TypedValue* ArrayData::nvGet(const StringData* key) const {
  StrNR k(key);
  return exists(k) ? (TypedValue*)&get(k, false) :
         NULL;
}

void ArrayData::nvGetKey(TypedValue* out, ssize_t pos) {
  Variant k = getKey(pos);
  TypedValue* tv = k.asTypedValue();
  // copy w/out clobbering out->_count.
  out->m_type = tv->m_type;
  out->m_data.num = tv->m_data.num;
  if (tv->m_type != KindOfInt64) out->m_data.pstr->incRefCount();
}

TypedValue* ArrayData::nvGetValueRef(ssize_t pos) {
  return const_cast<TypedValue*>(getValueRef(pos).asTypedValue());
}

ArrayData* ArrayData::nvSet(int64 ki, int64 vi, bool copy) {
  return set(ki, VarNR(vi), copy);
}

ArrayData* ArrayData::nvSet(int64 ki, const TypedValue* v, bool copy) {
  return set(ki, tvAsCVarRef(v), copy);
}

ArrayData* ArrayData::nvSet(StringData* k, const TypedValue* v, bool copy) {
  return set(StrNR(k), tvAsCVarRef(v), copy);
}

TypedValue* ArrayData::nvGetCell(int64 k, bool error) const {
  return exists(k) ? get(k, false).getTypedAccessor() :
         error ? nvGetNotFound(k) :
         NULL;
}

TypedValue* ArrayData::nvGetCell(const StringData* key, bool error) const {
  StrNR k(key);
  return exists(k) ? get(k, false).getTypedAccessor() :
         error ? nvGetNotFound(key) :
         NULL;
}

CVarRef ArrayData::getNotFound(int64 k) {
  raise_notice("Undefined index: %lld", k);
  return null_variant;
}

CVarRef ArrayData::getNotFound(litstr k) {
  raise_notice("Undefined index: %s", k);
  return null_variant;
}

CVarRef ArrayData::getNotFound(CStrRef k) {
  raise_notice("Undefined index: %s", k.data());
  return null_variant;
}

CVarRef ArrayData::getNotFound(CVarRef k) {
  raise_notice("Undefined index: %s", k.toString().data());
  return null_variant;
}

TypedValue* ArrayData::nvGetNotFound(int64 k) {
  raise_notice("Undefined index: %lld", k);
  return NULL;
}

TypedValue* ArrayData::nvGetNotFound(const StringData* k) {
  raise_notice("Undefined index: %s", k->data());
  return NULL;
}

void ArrayData::dump() {
  string out; dump(out); fwrite(out.c_str(), out.size(), 1, stdout);
}

void ArrayData::dump(std::string &out) {
  VariableSerializer vs(VariableSerializer::VarDump);
  String ret(vs.serialize(Array(this), true));
  out += "ArrayData(";
  out += boost::lexical_cast<string>(_count);
  out += "): ";
  out += string(ret.data(), ret.size());
}

void ArrayData::dump(std::ostream &out) {
  unsigned int i = 0;
  for (ArrayIter iter(this); iter; ++iter, i++) {
    VariableSerializer vs(VariableSerializer::Serialize);
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

///////////////////////////////////////////////////////////////////////////////
}
