/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/array/vector_variant.h>
#include <runtime/base/array/map_variant.h>
#include <system/gen/php/classes/stdclass.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/array/zend_array.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/macros.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ArrayData *ArrayData::Create() {
  return ArrayInit(0).create();
}

ArrayData *ArrayData::Create(CVarRef value) {
  ArrayInit init(1, true);
  init.set(0, value);
  return init.create();
}

ArrayData *ArrayData::Create(CVarRef name, CVarRef value) {
  ArrayInit init(1, false);
  // There is no toKey() call on name.
  init.set(0, name, value, -1, true);
  return init.create();
}

ArrayData::~ArrayData() {
}

void ArrayData::fetchValue(ssize_t pos, Variant & v) const {
  v = getValue(pos);
}

///////////////////////////////////////////////////////////////////////////////
// reads

Object ArrayData::toObject() const {
  return ObjectData::FromArray(const_cast<ArrayData *>(this));
}

CVarRef ArrayData::getValueRef(ssize_t pos) const {
  throw FatalErrorException("taking reference from an r-value");
}

bool ArrayData::isVectorData() const {
  for (ssize_t i = 0; i < size(); i++) {
    if (getIndex(i) != i) {
      return false;
    }
  }
  return true;
}

int ArrayData::compare(const ArrayData *v2, bool strict) const {
  ASSERT(v2);

  int count1 = size();
  int count2 = v2->size();
  if (count1 < count2) return -1;
  if (count1 > count2) return 1;
  if (count1 == 0) return 0;

  // prevent circular referenced objects/arrays or deep ones
  DECLARE_THREAD_INFO; RECURSION_INJECTION;

  if (strict) {
    for (ArrayIter iter1(this), iter2(v2); iter1 && iter2; ++iter1, ++iter2) {
      Variant key1(iter1.first());
      Variant key2(iter2.first());
      if (!key1.same(key2)) return 1; // or -1

      Variant value1(iter1.second());
      Variant value2(iter2.second());
      if (!value1.same(value2)) return 1; // or -1
    }
  } else {
    for (ArrayIter iter(this); iter; ++iter) {
      Variant key(iter.first());
      if (!v2->exists(key)) return 1;

      Variant value1(iter.second());
      Variant value2(v2->get(key));
      if (value1.more(value2)) return 1;
      if (value1.less(value2)) return -1;
    }
  }

  return 0;
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

void ArrayData::getFullPos(FullPos &pos) {
  pos.primary = ArrayData::invalid_index;
}
bool ArrayData::setFullPos(const FullPos &pos) {
  return false;
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
  if (serializer->incNestedLevel((void*)this)) {
    serializer->writeOverflow((void*)this);
  } else {
    serializer->writeArrayHeader(this, size());
    bool refValue = supportValueRef();
    for (ArrayIter iter(this); iter; ++iter) {
      Variant key(iter.first());
      if (key.isInteger()) {
        serializer->writeArrayKey(this, key.toInt64());
      } else {
        serializer->writeArrayKey(this, key.toString());
      }
      if (refValue) {
        serializer->writeArrayValue(this, iter.secondRef());
      } else {
        serializer->writeArrayValue(this, iter.second());
      }
    }
    serializer->writeArrayFooter(this);
  }
  serializer->decNestedLevel((void*)this);
}

bool ArrayData::hasInternalReference(PointerSet &vars) const {
  if (supportValueRef()) {
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

        if (pobj->o_toArray().get()->hasInternalReference(vars)) {
          return true;
        }
      } else if (var.isArray() &&
                 var.getArrayData()->hasInternalReference(vars)) {
        return true;
      }
    }
  }
  return false;
}

void ArrayData::dump() {
  string out; dump(out); printf("%s", out.c_str());
}

void ArrayData::dump(std::string &out) {
  VariableSerializer vs(VariableSerializer::VarDump);
  Variant ret(vs.serialize(Array(this), true));
  out += "ArrayData(";
  out += boost::lexical_cast<string>(_count);
  out += "): ";
  out += ret.toString().data();
}


///////////////////////////////////////////////////////////////////////////////
}
