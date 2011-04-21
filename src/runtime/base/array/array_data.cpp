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
#include <runtime/base/runtime_option.h>
#include <runtime/base/macros.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ArrayData *ArrayData::Create() {
  return ArrayInit(0, false).create();
}

ArrayData *ArrayData::Create(CVarRef value) {
  ArrayInit init(1, true);
  init.set(value);
  return init.create();
}

ArrayData *ArrayData::Create(CVarRef name, CVarRef value) {
  ArrayInit init(1, false);
  // There is no toKey() call on name.
  init.set(name, value, true);
  return init.create();
}

ArrayData *ArrayData::CreateRef(CVarRef value) {
  ArrayInit init(1, true);
  init.setRef(value);
  return init.create();
}

ArrayData *ArrayData::CreateRef(CVarRef name, CVarRef value) {
  ArrayInit init(1, false);
  // There is no toKey() call on name.
  init.setRef(name, value, true);
  return init.create();
}

ArrayData::~ArrayData() {
  // If there are any strong iterators pointing to this array, they need
  // to be invalidated.
  if (!m_strongIterators.empty()) {
    freeStrongIterators();
  }
}

///////////////////////////////////////////////////////////////////////////////
// reads

Object ArrayData::toObject() const {
  return ObjectData::FromArray(const_cast<ArrayData *>(this));
}

bool ArrayData::isVectorData() const {
  for (ssize_t i = 0; i < size(); i++) {
    if (getIndex(i) != i) {
      return false;
    }
  }
  return true;
}

bool ArrayData::isGlobalArrayWrapper() const {
  return false;
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

void ArrayData::load(CVarRef k, Variant &v) const {
  if (exists(k)) v = get(k);
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
  m_strongIterators.push(&fp);
  fp.container = (ArrayData*)this;
  getFullPos(fp);
}

void ArrayData::freeFullPos(FullPos &fp) {
  ASSERT(fp.container == (ArrayData*)this);
  int sz = m_strongIterators.size();
  if (sz > 0) {
    // Common case: fp is at the end of the list
    if (m_strongIterators.get(sz - 1) == &fp) {
      m_strongIterators.pop();
      fp.container = NULL;
      return;
    }
    // Unusual case: somehow the strong iterator for an foreach loop
    // was freed before a strong iterator from a nested foreach loop,
    // so do a linear search for fp
    for (int k = sz - 2; k >= 0; --k) {
      if (m_strongIterators.get(k) == &fp) {
        // Swap fp with the last element in the list and then pop
        m_strongIterators.set(k, m_strongIterators.get(sz - 1));
        m_strongIterators.pop();
        fp.container = NULL;
        return;
      }
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
  int sz = m_strongIterators.size();
  for (int i = 0; i < sz; ++i) {
    m_strongIterators.get(i)->container = NULL;
  }
  m_strongIterators.clear();
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

Variant ArrayData::each() {
  if (m_pos >= 0 && m_pos < size()) {
    Array ret;
    Variant key(getKey(m_pos));
    Variant value(getValue(m_pos));
    ret.set(1, value);
    ret.set("value", value);
    ret.set(0, key);
    ret.set("key", key);
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

void ArrayData::serialize(VariableSerializer *serializer) const {
  if (size() == 0) {
    serializer->writeArrayHeader(this, 0);
    serializer->writeArrayFooter(this);
    return;
  }
  if (serializer->incNestedLevel((void*)this)) {
    serializer->writeOverflow((void*)this);
  } else {
    serializer->writeArrayHeader(this, size());
    for (ArrayIter iter(this); iter; ++iter) {
      Variant key(iter.first());
      if (key.isInteger()) {
        serializer->writeArrayKey(this, key.toInt64());
      } else {
        serializer->writeArrayKey(this, key.toString());
      }
      serializer->writeArrayValue(this, iter.secondRef());
    }
    serializer->writeArrayFooter(this);
  }
  serializer->decNestedLevel((void*)this);
}

bool ArrayData::hasInternalReference(PointerSet &vars,
                                     bool ds /* = false */) const {
  if (isSharedMap()) return false;
  for (ArrayIter iter(this); iter; ++iter) {
    CVarRef var = iter.secondRef();
    if (var.isReferenced()) {
      Variant *pvar = var.getVariantData();
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
      if (pobj->o_toArray().get()->hasInternalReference(vars, ds)) {
        return true;
      }
    } else if (var.isArray() &&
               var.getArrayData()->hasInternalReference(vars, ds)) {
      return true;
    }
  }
  return false;
}

void ArrayData::dump() {
  string out; dump(out); printf("%s", out.c_str());
}

void ArrayData::dump(std::string &out) {
  VariableSerializer vs(VariableSerializer::VarDump);
  String ret(vs.serialize(Array(this), true));
  out += "ArrayData(";
  out += boost::lexical_cast<string>(_count);
  out += "): ";
  out += ret.data();
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
    out << endl;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
