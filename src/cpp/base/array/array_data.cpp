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

#include <cpp/base/array/array_data.h>
#include <cpp/base/array/array_element.h>
#include <cpp/base/array/array_iterator.h>
#include <cpp/base/builtin_functions.h>
#include <cpp/base/type_array.h>
#include <cpp/base/type_string.h>

#include <cpp/base/array/empty_array.h>
#include <cpp/base/array/vector_long.h>
#include <cpp/base/array/vector_string.h>
#include <cpp/base/array/vector_variant.h>
#include <cpp/base/array/map_long.h>
#include <cpp/base/array/map_string.h>
#include <cpp/base/array/map_variant.h>

#include <lib/system/gen/php/classes/stdclass.h>
#include <cpp/base/variable_serializer.h>
#include <cpp/base/array/zend_array.h>
#include <cpp/base/runtime_option.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ArrayData *ArrayData::Create() {
  if (RuntimeOption::UseZendArray) {
    return StaticEmptyZendArray::Get();
  }
  return StaticEmptyArray::Get();
}

ArrayData *ArrayData::Create(CVarRef value) {
  if (RuntimeOption::UseZendArray) {
    ArrayData *ret = NEW(ZendArray)(1);
    ret->append(value, false);
    return ret;
  }
#ifndef FORCE_VARIANT_ARRAYS
  if (!value.isContagious()) {
    switch (value.getType()) {
    case KindOfByte:
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
      return NEW(VectorLong)(value.toInt64());
    case LiteralString:
    case KindOfString:
      return NEW(VectorString)(value.toString());
    default:
      break;
    }
  }
#endif
  return NEW(VectorVariant)(value);
}

ArrayData *ArrayData::Create(CVarRef name, CVarRef value) {
  if (RuntimeOption::UseZendArray) {
    ArrayData *ret = NEW(ZendArray)(1);
    ret->set(name, value, false);
    return ret;
  }
  if (!value.isContagious()) {
    bool isZero = name.isInteger() && name.toInt64() == 0;
#ifndef FORCE_VARIANT_ARRAYS
    switch (value.getType()) {
    case KindOfByte:
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
      if (isZero) {
        return NEW(VectorLong)(value.toInt64());
      }
      return NEW(MapLong)(name, value.toInt64());
    case LiteralString:
    case KindOfString:
      if (isZero) {
        return NEW(VectorString)(value.toString());
      }
      return NEW(MapString)(name, value.toString());
    default:
      if (isZero) {
        return NEW(VectorVariant)(value);
      }
      break;
    }
#else
    if (isZero) {
      return NEW(VectorVariant)(value);
    }
#endif
  }
  return NEW(MapVariant)(name, value);
}

ArrayData *ArrayData::Create(const std::vector<ArrayElement *> &elems,
                             bool replace /* = true */) {
  if (elems.empty()) {
    return Create();
  }

  if (RuntimeOption::UseZendArray) {
    uint size = elems.size();
    ArrayData *ret = NEW(ZendArray)(size);
    for (unsigned int i = 0; i < size; i++) {
      ArrayElement *elem = elems[i];
      if (elem->hasName()) {
        ret->set(elem->getName(), elem->getVariant(), false, elem->getHash());
      } else {
        ret->append(elem->getVariant(), false);
      }
      elem->release();
    }
    return ret;
  }

  DataType typeAll = KindOfNull;
  bool associative = false;
  uint size = elems.size();
  for (unsigned int i = 0; i < size; i++) {
    ArrayElement *elem = elems[i];

    DataType type = elem->getType();
    if (type != typeAll) {
      if (typeAll == KindOfNull) {
        typeAll = type;
      } else {
        typeAll = KindOfVariant;
      }
    }
    if (elem->hasName()) associative = true;

    // nothing more to find about
    if (typeAll == KindOfVariant && associative) break;
  }

  ArrayData *ret = NULL;
#ifndef FORCE_VARIANT_ARRAYS
  switch (typeAll) {
  case KindOfInt64:
    if (associative) {
      ret = NEW(MapLong)(elems, replace);
    } else {
      ret = NEW(VectorLong)(elems);
    }
    break;
  case KindOfString:
    if (associative) {
      ret = NEW(MapString)(elems, replace);
    } else {
      ret = NEW(VectorString)(elems);
    }
    break;
  case KindOfVariant:
    if (associative) {
      ret = NEW(MapVariant)(elems, replace);
    } else {
      ret = NEW(VectorVariant)(elems);
    }
    break;
  default:
    ASSERT(false);
    break;
  }
#else
  if (associative) {
    ret = NEW(MapVariant)(elems, replace);
  } else {
    ret = NEW(VectorVariant)(elems);
  }
#endif
  size = elems.size();
  for (unsigned int i = 0; i < size; i++) {
    elems[i]->release();
  }

  return ret;
}

ArrayData::~ArrayData() {
}

///////////////////////////////////////////////////////////////////////////////
// reads

Object ArrayData::toObject() const {
  Object ret = NEW(c_stdclass)();
  ret->o_set(Array(const_cast<ArrayData *>(this)));
  return ret;
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

  if (strict) {
    for (ArrayIter iter1(this), iter2(v2); iter1 && iter2; ++iter1, ++iter2) {
      Variant key1 = iter1.first();
      Variant key2 = iter2.first();
      if (!key1.same(key2)) return 1; // or -1

      Variant value1 = iter1.second();
      Variant value2 = iter2.second();
      if (!value1.same(value2)) return 1; // or -1
    }
  } else {
    for (ArrayIter iter(this); iter; ++iter) {
      Variant key = iter.first();
      if (!v2->exists(key)) return 1;

      Variant value1 = iter.second();
      Variant value2 = v2->get(key);
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
    Variant key = getKey(m_pos);
    Variant value = getValue(m_pos);
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
    for (ArrayIter iter(this); iter; ++iter) {
      Variant key = iter.first();
      if (key.isInteger()) {
        serializer->writeArrayKey(this, key.toInt64());
      } else {
        serializer->writeArrayKey(this, key.toString());
      }
      if (supportValueRef()) {
        serializer->writeArrayValue(this, iter.secondRef());
      } else {
        serializer->writeArrayValue(this, iter.second());
      }
    }
    serializer->writeArrayFooter(this);
  }
  serializer->decNestedLevel((void*)this);
}

void ArrayData::dump() {
  string out; dump(out); printf("%s", out.c_str());
}

void ArrayData::dump(std::string &out) {
  VariableSerializer vs(VariableSerializer::VarDump);
  Variant ret = vs.serialize(Array(this), true);
  out += "ArrayData(";
  out += boost::lexical_cast<string>(_count);
  out += "): ";
  out += ret.toString().data();
}


///////////////////////////////////////////////////////////////////////////////
}
