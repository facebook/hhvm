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

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/util/exception.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-qsort.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/ext_iconv.h"
#include <unicode/coll.h> // icu
#include "hphp/parser/hphp.tab.hpp"

namespace HPHP {

const Array null_array = Array();
const Array empty_array = HphpArray::GetStaticEmptyArray();

void Array::setEvalScalar() const {
  Array* thisPtr = const_cast<Array*>(this);
  if (!m_px) *thisPtr = ArrayData::Create();
  if (!m_px->isStatic()) {
    ArrayData *ad = ArrayData::GetScalarArray(m_px);
    *thisPtr = ad;
  }
}

///////////////////////////////////////////////////////////////////////////////
// constructors

Array Array::Create(CVarRef name, CVarRef var) {
  return ArrayData::Create(name.isString() ? name.toKey() : name, var);
}

Array::~Array() {}

///////////////////////////////////////////////////////////////////////////////
// operators

Array &Array::operator=(ArrayData *data) {
  ArrayBase::operator=(data);
  return *this;
}

Array &Array::operator=(CArrRef arr) {
  ArrayBase::operator=(arr.m_px);
  return *this;
}

Array &Array::operator=(CVarRef var) {
  return operator=(var.toArray());
}

// Move assign
Array &Array::operator =  (Variant&& v) {
  if (v.m_type == KindOfArray) {
    m_px = v.m_data.parr;
    v.reset();
  } else {
    *this = const_cast<CVarRef>(v);
  }
  return *this;
}

Array Array::operator+(ArrayData *data) const {
  return Array(m_px).plusImpl(data);
}

Array Array::operator+(CArrRef arr) const {
  return Array(m_px).plusImpl(arr.m_px);
}

Array &Array::operator+=(ArrayData *data) {
  return plusImpl(data);
}

Array &Array::operator+=(CVarRef var) {
  if (var.getType() != KindOfArray) {
    throw BadArrayMergeException();
  }
  return operator+=(var.getArrayData());
}

Array &Array::operator+=(CArrRef arr) {
  return plusImpl(arr.m_px);
}

Array Array::diff(CVarRef array, bool by_key, bool by_value,
                  PFUNC_CMP key_cmp_function /* = NULL */,
                  const void *key_data /* = NULL */,
                  PFUNC_CMP value_cmp_function /* = NULL */,
                  const void *value_data /* = NULL */) const {
  if (!array.isArray()) {
    throw_expected_array_exception();
    return Array();
  }
  return diffImpl(array.getArrayData(), by_key, by_value, false,
                  key_cmp_function, key_data,
                  value_cmp_function, value_data);
}

Array Array::intersect(CVarRef array, bool by_key, bool by_value,
                       PFUNC_CMP key_cmp_function /* = NULL */,
                       const void *key_data /* = NULL */,
                       PFUNC_CMP value_cmp_function /* = NULL */,
                       const void *value_data /* = NULL */) const {
  if (!array.isArray()) {
    throw_expected_array_exception();
    return Array();
  }
  return diffImpl(array.getArrayData(), by_key, by_value, true,
                  key_cmp_function, key_data,
                  value_cmp_function, value_data);
}

int Array::CompareAsStrings(CVarRef v1, CVarRef v2, const void *data) {
  return HPHP::same(HPHP::toString(v1), HPHP::toString(v2)) ? 0 : -1;
}

Array Array::diffImpl(CArrRef array, bool by_key, bool by_value, bool match,
                      PFUNC_CMP key_cmp_function,
                      const void *key_data,
                      PFUNC_CMP value_cmp_function,
                      const void *value_data) const {
  assert(by_key || by_value);
  assert(by_key || key_cmp_function == nullptr);
  assert(by_value || value_cmp_function == nullptr);
  PFUNC_CMP value_cmp_as_string_function = value_cmp_function;
  if (!value_cmp_function) {
    value_cmp_function = SortStringAscending;
    value_cmp_as_string_function = CompareAsStrings;
  }

  Array ret = Array::Create();
  if (by_key && !key_cmp_function) {
    // Fast case
    for (ArrayIter iter(*this); iter; ++iter) {
      Variant key(iter.first());
      CVarRef value(iter.secondRef());
      bool found = false;
      if (array->exists(key)) {
        if (by_value) {
          found = value_cmp_as_string_function(
            value, array.rvalAt(key, AccessFlags::Key), value_data) == 0;
        } else {
          found = true;
        }
      }
      if (found == match) {
        ret.setWithRef(key, value, true);
      }
    }
    return ret;
  }

  if (!key_cmp_function) {
    key_cmp_function = SortRegularAscending;
  }

  vector<int> perm1;
  SortData opaque1;
  int bottom = 0;
  int top = array.size();
  PFUNC_CMP cmp;
  const void *cmp_data;
  if (by_key) {
    cmp = key_cmp_function;
    cmp_data = key_data;
  } else {
    cmp = value_cmp_function;
    cmp_data = value_data;
  }
  SortImpl(perm1, array, opaque1, cmp, by_key, cmp_data);

  for (ArrayIter iter(*this); iter; ++iter) {
    Variant target;
    if (by_key) {
      target = iter.first();
    } else {
      target = iter.second();
    }

    int mid = -1;
    int min = bottom;
    int max = top;
    while (min < max) {
      mid = (max + min) / 2;
      ssize_t pos = opaque1.positions[perm1[mid]];
      int cmp_res =  cmp(target,
                         by_key ? array->getKey(pos)
                                : array->getValueRef(pos),
                         cmp_data);
      if (cmp_res > 0) { // outer is bigger
        min = mid + 1;
      } else if (cmp_res == 0) {
        break;
      } else {
        max = mid;
      }
    }
    bool found = false;
    if (min < max) { // found
      // if checking both, check value
      if (by_key && by_value) {
        CVarRef val(iter.secondRef());
        // Have to look up and down for matches
        for (int i = mid; i < max; i++) {
          ssize_t pos = opaque1.positions[perm1[i]];
          if (key_cmp_function(target, array->getKey(pos), key_data) != 0) {
            break;
          }
          if (value_cmp_as_string_function(val, array->getValueRef(pos),
                                           value_data) == 0) {
            found = true;
            break;
          }
        }
        if (!found) {
          for (int i = mid-1; i >= min; i--) {
            ssize_t pos = opaque1.positions[perm1[i]];
            if (key_cmp_function(target, array->getKey(pos), key_data) != 0) {
              break;
            }
            if (value_cmp_as_string_function(val, array->getValueRef(pos),
                                             value_data) == 0) {
              found = true;
              break;
            }
          }
        }
      } else {
        // found at mid
        found = true;
      }
    }

    if (found == match) {
      ret.setWithRef(iter.first(), iter.secondRef(), true);
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// manipulations

Array &Array::merge(CArrRef arr) {
  return mergeImpl(arr.m_px);
}

Array &Array::plusImpl(ArrayData *data) {
  if (m_px == nullptr || data == nullptr) {
    throw BadArrayMergeException();
  }
  if (!data->empty()) {
    if (m_px->empty()) {
      ArrayBase::operator=(data);
    } else if (m_px != data) {
      auto const escalated = m_px->plusEq(data);
      if (escalated != m_px) {
        ArrayBase::operator=(Array::attach(escalated));
      }
    }
  }
  return *this;
}

Array &Array::mergeImpl(ArrayData *data) {
  if (m_px == nullptr || data == nullptr) {
    throw BadArrayMergeException();
  }
  if (!data->empty()) {
    ArrayBase::operator=(Array::attach(m_px->merge(data)));
  } else {
    m_px->renumber();
  }
  return *this;
}

Array Array::slice(int offset, int length, bool preserve_keys) const {
  if (m_px == nullptr) return Array();
  return ArrayUtil::Slice(m_px, offset, length, preserve_keys).toArray();
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

bool Array::same(CArrRef v2) const {
  if (m_px == nullptr && v2.get() == nullptr) return true;
  if (m_px && v2.get()) {
    return m_px->equal(v2.get(), true);
  }
  return false;
}

bool Array::same(CObjRef v2) const {
  return false;
}

bool Array::equal(CArrRef v2) const {
  if (m_px == nullptr || v2.get() == nullptr) {
    return HPHP::equal(toBoolean(), v2.toBoolean());
  }
  return m_px->equal(v2.get(), false);
}

bool Array::equal(CObjRef v2) const {
  if (m_px == nullptr || v2.get() == nullptr) {
    return HPHP::equal(toBoolean(), v2.toBoolean());
  }
  return false;
}

bool Array::less(CArrRef v2, bool flip /* = false */) const {
  if (m_px == nullptr || v2.get() == nullptr) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  if (flip) {
    return v2.get()->compare(m_px) > 0;
  }
  return m_px->compare(v2.get()) < 0;
}

bool Array::less(CObjRef v2) const {
  if (m_px == nullptr || v2.get() == nullptr) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  check_collection_compare(v2.get());
  return true;
}

bool Array::less(CVarRef v2) const {
  if (m_px == nullptr || v2.isNull()) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  if (v2.getType() == KindOfArray) {
    return m_px->compare(v2.toArray().get()) < 0;
  }
  return HPHP::more(v2, *this);
}

bool Array::more(CArrRef v2, bool flip /* = true */) const {
  if (m_px == nullptr || v2.get() == nullptr) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  if (flip) {
    return v2.get()->compare(m_px) < 0;
  }
  return m_px->compare(v2.get()) > 0;
}

bool Array::more(CObjRef v2) const {
  if (m_px == nullptr || v2.get() == nullptr) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  check_collection_compare(v2.get());
  return false;
}

bool Array::more(CVarRef v2) const {
  if (m_px == nullptr || v2.isNull()) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  if (v2.getType() == KindOfArray) {
    return v2.toArray().get()->compare(m_px) < 0;
  }
  return HPHP::less(v2, *this);
}

///////////////////////////////////////////////////////////////////////////////
// iterator

ArrayIter Array::begin(const String& context /* = null_string */) const {
  return ArrayIter(m_px);
}

void Array::escalate() {
  if (m_px) ArrayBase::operator=(m_px->escalate());
}

///////////////////////////////////////////////////////////////////////////////
// offset functions

Variant Array::rvalAt(int key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get((int64_t)key, flags & AccessFlags::Error);
  return null_variant;
}

CVarRef Array::rvalAtRef(int key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get((int64_t)key, flags & AccessFlags::Error);
  return null_variant;
}

Variant Array::rvalAt(int64_t key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get(key, flags & AccessFlags::Error);
  return null_variant;
}

CVarRef Array::rvalAtRef(int64_t key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get(key, flags & AccessFlags::Error);
  return null_variant;
}

CVarRef Array::rvalAtRef(const String& key, ACCESSPARAMS_IMPL) const {
  if (m_px) {
    bool error = flags & AccessFlags::Error;
    if (flags & AccessFlags::Key) return m_px->get(key, error);
    if (key.isNull()) return m_px->get(empty_string, error);
    int64_t n;
    if (!key->isStrictlyInteger(n)) {
      return m_px->get(key, error);
    } else {
      return m_px->get(n, error);
    }
  }
  return null_variant;
}

Variant Array::rvalAt(const String& key, ACCESSPARAMS_IMPL) const {
  return Array::rvalAtRef(key, flags);
}

CVarRef Array::rvalAtRef(CVarRef key, ACCESSPARAMS_IMPL) const {
  if (!m_px) return null_variant;
  switch (key.m_type) {
  case KindOfUninit:
  case KindOfNull:
    return m_px->get(empty_string, flags & AccessFlags::Error);
  case KindOfBoolean:
  case KindOfInt64:
    return m_px->get(key.m_data.num, flags & AccessFlags::Error);
  case KindOfDouble:
    return m_px->get((int64_t)key.m_data.dbl, flags & AccessFlags::Error);
  case KindOfStaticString:
  case KindOfString: {
    int64_t n;
    if (!(flags & AccessFlags::Key) &&
        key.m_data.pstr->isStrictlyInteger(n)) {
      return m_px->get(n, flags & AccessFlags::Error);
    } else {
      return m_px->get(key.asCStrRef(), flags & AccessFlags::Error);
    }
  }
  case KindOfArray:
    throw_bad_type_exception("Invalid type used as key");
    break;
  case KindOfObject:
    throw_bad_type_exception("Invalid type used as key");
    break;
  case KindOfResource:
    return m_px->get(key.toInt64(), flags & AccessFlags::Error);
  case KindOfRef:
    return rvalAtRef(*(key.m_data.pref->var()), flags);
  default:
    assert(false);
    break;
  }
  return null_variant;
}

Variant Array::rvalAt(CVarRef key, ACCESSPARAMS_IMPL) const {
  return Array::rvalAtRef(key, flags);
}

Variant &Array::lvalAt() {
  if (!m_px) ArrayBase::operator=(ArrayData::Create());
  Variant *ret = nullptr;
  ArrayData *arr = m_px;
  ArrayData *escalated = arr->lvalNew(ret, arr->hasMultipleRefs());
  if (escalated != arr) ArrayBase::operator=(escalated);
  assert(ret);
  return *ret;
}

Variant &Array::lvalAt(const String& key, ACCESSPARAMS_IMPL) {
  if (flags & AccessFlags::Key) return lvalAtImpl(key, flags);
  return lvalAtImpl(key.toKey(), flags);
}

Variant &Array::lvalAt(CVarRef key, ACCESSPARAMS_IMPL) {
  if (flags & AccessFlags::Key) return lvalAtImpl(key, flags);
  VarNR k(key.toKey());
  if (!k.isNull()) {
    return lvalAtImpl(k, flags);
  }
  return Variant::lvalBlackHole();
}

template<typename T>
ALWAYS_INLINE
void Array::setImpl(const T &key, CVarRef v) {
  if (!m_px) {
    ArrayData *data = ArrayData::Create(key, v);
    ArrayBase::operator=(data);
  } else {
    ArrayData *escalated = m_px->set(key, v, (m_px->hasMultipleRefs()));
    if (escalated != m_px) ArrayBase::operator=(escalated);
  }
}

template<typename T>
ALWAYS_INLINE
void Array::setRefImpl(const T &key, CVarRef v) {
  if (!m_px) {
    ArrayData *data = ArrayData::CreateRef(key, v);
    ArrayBase::operator=(data);
  } else {
    escalate();
    ArrayData *escalated = m_px->setRef(key, v, (m_px->hasMultipleRefs()));
    if (escalated != m_px) ArrayBase::operator=(escalated);
  }
}

template<typename T>
ALWAYS_INLINE
void Array::addImpl(const T &key, CVarRef v) {
  if (!m_px) {
    ArrayData *data = ArrayData::Create(key, v);
    ArrayBase::operator=(data);
  } else {
    ArrayData *escalated = m_px->add(key, v, (m_px->hasMultipleRefs()));
    if (escalated != m_px) ArrayBase::operator=(escalated);
  }
}

void Array::set(int64_t key, CVarRef v) {
  setImpl(key, v);
}

void Array::set(const String& key, CVarRef v, bool isKey /* = false */) {
  if (isKey) return setImpl(key, v);
  setImpl(key.toKey(), v);
}

void Array::set(CVarRef key, CVarRef v, bool isKey /* = false */) {
  if (key.getRawType() == KindOfInt64) return setImpl(key.getNumData(), v);
  if (isKey) return setImpl(key, v);
  VarNR k(key.toKey());
  if (!k.isNull()) setImpl(k, v);
}

void Array::setRef(int64_t key, CVarRef v) {
  setRefImpl(key, v);
}

void Array::setRef(const String& key, CVarRef v, bool isKey /* = false */) {
  if (isKey) return setRefImpl(key, v);
  setRefImpl(key.toKey(), v);
}

void Array::setRef(CVarRef key, CVarRef v, bool isKey /* = false */) {
  if (key.getRawType() == KindOfInt64) return setRefImpl(key.getNumData(), v);
  if (isKey) return setRefImpl(key, v);
  VarNR k(key.toKey());
  if (!k.isNull()) setRefImpl<Variant>(k, v);
}

void Array::add(int64_t key, CVarRef v) {
  addImpl(key, v);
}

void Array::add(const String& key, CVarRef v, bool isKey /* = false */) {
  if (isKey) return addImpl(key, v);
  addImpl(key.toKey(), v);
}

void Array::add(CVarRef key, CVarRef v, bool isKey /* = false */) {
  if (key.getRawType() == KindOfInt64) return addImpl(key.getNumData(), v);
  if (isKey) return addImpl(key, v);
  VarNR k(key.toKey());
  if (!k.isNull()) addImpl(k, v);
}

///////////////////////////////////////////////////////////////////////////////
// membership functions

bool Array::valueExists(CVarRef search_value,
                        bool strict /* = false */) const {
  for (ArrayIter iter(*this); iter; ++iter) {
    if ((strict && HPHP::same(iter.secondRef(), search_value)) ||
        (!strict && HPHP::equal(iter.secondRef(), search_value))) {
      return true;
    }
  }
  return false;
}

Variant Array::key(CVarRef search_value, bool strict /* = false */) const {
  for (ArrayIter iter(*this); iter; ++iter) {
    if ((strict && HPHP::same(iter.secondRef(), search_value)) ||
        (!strict && HPHP::equal(iter.secondRef(), search_value))) {
      return iter.first();
    }
  }
  return false; // PHP uses "false" over null in many places
}

Array Array::values() const {
  PackedArrayInit ai(size());
  for (ArrayIter iter(*this); iter; ++iter) {
    ai.appendWithRef(iter.secondRef());
  }
  return ai.toArray();
}

bool Array::exists(const String& key, bool isKey /* = false */) const {
  if (isKey) return existsImpl(key);
  return existsImpl(key.toKey());
}

bool Array::exists(CVarRef key, bool isKey /* = false */) const {
  switch(key.getType()) {
  case KindOfBoolean:
  case KindOfInt64:
    return existsImpl(key.toInt64());
  default:
    break;
  }
  if (isKey) return existsImpl(key);
  VarNR k(key.toKey());
  if (!k.isNull()) {
    return existsImpl(k);
  }
  return false;
}

void Array::remove(const String& key, bool isString /* = false */) {
  if (isString) {
    removeImpl(key);
  } else {
    removeImpl(key.toKey());
  }
}

void Array::remove(CVarRef key) {
  switch(key.getType()) {
  case KindOfBoolean:
  case KindOfInt64:
    removeImpl(key.toInt64());
    return;
  default:
    break;
  }
  VarNR k(key.toKey());
  if (!k.isNull()) {
    removeImpl(k);
  }
}

void Array::removeAll() {
  operator=(Create());
}

CVarRef Array::append(CVarRef v) {
  if (!m_px) {
    ArrayBase::operator=(ArrayData::Create(v));
  } else {
    ArrayData *escalated = m_px->append(v, (m_px->hasMultipleRefs()));
    if (escalated != m_px) ArrayBase::operator=(escalated);
  }
  return v;
}

CVarRef Array::appendRef(CVarRef v) {
  if (!m_px) {
    ArrayBase::operator=(ArrayData::CreateRef(v));
  } else {
    ArrayData *escalated = m_px->appendRef(v, (m_px->hasMultipleRefs()));
    if (escalated != m_px) ArrayBase::operator=(escalated);
  }
  return v;
}

CVarRef Array::appendWithRef(CVarRef v) {
  if (!m_px) ArrayBase::operator=(ArrayData::Create());
  ArrayData *escalated = m_px->appendWithRef(v, (m_px->hasMultipleRefs()));
  if (escalated != m_px) ArrayBase::operator=(escalated);
  return v;
}

Variant Array::pop() {
  if (m_px) {
    Variant ret;
    ArrayData *newarr = m_px->pop(ret);
    if (newarr != m_px) ArrayBase::operator=(newarr);
    return ret;
  }
  return null_variant;
}

Variant Array::dequeue() {
  if (m_px) {
    Variant ret;
    ArrayData *newarr = m_px->dequeue(ret);
    if (newarr != m_px) ArrayBase::operator=(newarr);
    return ret;
  }
  return null_variant;
}

void Array::prepend(CVarRef v) {
  if (!m_px) operator=(Create());
  assert(m_px);
  ArrayData *newarr = m_px->prepend(v, (m_px->hasMultipleRefs()));
  if (newarr != m_px) ArrayBase::operator=(newarr);
}

///////////////////////////////////////////////////////////////////////////////
// output functions

void Array::serialize(VariableSerializer *serializer,
                      bool isObject /* = false */) const {
  if (m_px) {
    m_px->serialize(serializer, isObject);
  } else {
    serializer->writeNull();
  }
}

void Array::unserialize(VariableUnserializer *uns) {
  int64_t size = uns->readInt();
  char sep = uns->readChar();
  if (sep != ':') {
    throw Exception("Expected ':' but got '%c'", sep);
  }
  sep = uns->readChar();
  if (sep != '{') {
    throw Exception("Expected '{' but got '%c'", sep);
  }

  if (size == 0) {
    operator=(Create());
  } else {
    // Pre-allocate an ArrayData of the given size, to avoid escalation in
    // the middle, which breaks references.
    operator=(ArrayInit(size).create());
    for (int64_t i = 0; i < size; i++) {
      Variant key(uns->unserializeKey());
      if (!key.isString() && !key.isInteger()) {
        throw Exception("Invalid key");
      }
      // for apc, we know the key can't exist, but ignore that optimization
      assert(uns->getType() != VariableUnserializer::Type::APCSerialize ||
             !exists(key, true));
      Variant &value = lvalAt(key, AccessFlags::Key);
      value.unserialize(uns);
    }
  }

  sep = uns->readChar();
  if (sep != '}') {
    throw Exception("Expected '}' but got '%c'", sep);
  }
}

void Array::dump() {
  if (m_px) {
    m_px->dump();
  } else {
    printf("(null)\n");
  }
}

///////////////////////////////////////////////////////////////////////////////
// sorting

static int array_compare_func(const void *n1, const void *n2, const void *op) {
  int index1 = *(int*)n1;
  int index2 = *(int*)n2;
  Array::SortData *opaque = (Array::SortData*)op;
  ssize_t pos1 = opaque->positions[index1];
  ssize_t pos2 = opaque->positions[index2];
  if (opaque->by_key) {
    return opaque->cmp_func((*opaque->array)->getKey(pos1),
                            (*opaque->array)->getKey(pos2),
                            opaque->data);
  }
  return opaque->cmp_func((*opaque->array)->getValueRef(pos1),
                          (*opaque->array)->getValueRef(pos2),
                          opaque->data);
}

static int multi_compare_func(const void *n1, const void *n2, const void *op) {
  int index1 = *(int*)n1;
  int index2 = *(int*)n2;
  const std::vector<Array::SortData> *opaques =
    (const std::vector<Array::SortData> *)op;
  for (unsigned int i = 0; i < opaques->size(); i++) {
    const Array::SortData *opaque = &opaques->at(i);
    ssize_t pos1 = opaque->positions[index1];
    ssize_t pos2 = opaque->positions[index2];
    int result;
    if (opaque->by_key) {
      result = opaque->cmp_func((*opaque->array)->getKey(pos1),
                                (*opaque->array)->getKey(pos2),
                                opaque->data);
    } else {
      result = opaque->cmp_func((*opaque->array)->getValueRef(pos1),
                                (*opaque->array)->getValueRef(pos2),
                                opaque->data);
    }
    if (result != 0) return result;
  }
  return 0;
}

void Array::SortImpl(vector<int> &indices, CArrRef source,
                     Array::SortData &opaque, Array::PFUNC_CMP cmp_func,
                     bool by_key, const void *data /* = NULL */) {
  assert(cmp_func);

  int count = source.size();
  if (count == 0) {
    return;
  }
  indices.reserve(count);
  for (int i = 0; i < count; i++) {
    indices.push_back(i);
  }

  opaque.array = &source;
  opaque.by_key = by_key;
  opaque.cmp_func = cmp_func;
  opaque.data = data;
  opaque.positions.reserve(count);
  for (ssize_t pos = source->iter_begin(); pos != ArrayData::invalid_index;
       pos = source->iter_advance(pos)) {
    opaque.positions.push_back(pos);
  }
  zend_qsort(&indices[0], count, sizeof(int), array_compare_func, &opaque);
}

void Array::sort(PFUNC_CMP cmp_func, bool by_key, bool renumber,
                 const void *data /* = NULL */) {
  Array sorted = Array::Create();
  SortData opaque;
  vector<int> indices;
  SortImpl(indices, *this, opaque, cmp_func, by_key, data);
  int count = size();
  for (int i = 0; i < count; i++) {
    ssize_t pos = opaque.positions[indices[i]];
    if (renumber) {
      sorted.appendWithRef(m_px->getValueRef(pos));
    } else {
      sorted.setWithRef(m_px->getKey(pos), m_px->getValueRef(pos), true);
    }
  }
  operator=(sorted);
}

bool Array::MultiSort(std::vector<SortData> &data, bool renumber) {
  assert(!data.empty());

  int count = -1;
  for (unsigned int k = 0; k < data.size(); k++) {
    SortData &opaque = data[k];

    assert(opaque.array);
    assert(opaque.cmp_func);
    int size = opaque.array->size();
    if (count == -1) {
      count = size;
    } else if (count != size) {
      throw_invalid_argument("arrays: (inconsistent sizes)");
      return false;
    }

    opaque.positions.reserve(size);
    CArrRef arr = *opaque.array;
    if (!arr.empty()) {
      for (ssize_t pos = arr->iter_begin(); pos != ArrayData::invalid_index;
           pos = arr->iter_advance(pos)) {
        opaque.positions.push_back(pos);
      }
    }
  }
  if (count == 0) {
    return true;
  }

  int *indices = (int *)malloc(sizeof(int) * count);
  for (int i = 0; i < count; i++) {
    indices[i] = i;
  }

  zend_qsort(indices, count, sizeof(int), multi_compare_func, (void *)&data);

  for (unsigned int k = 0; k < data.size(); k++) {
    SortData &opaque = data[k];
    CArrRef arr = *opaque.array;

    Array sorted;
    for (int i = 0; i < count; i++) {
      ssize_t pos = opaque.positions[indices[i]];
      Variant k(arr->getKey(pos));
      if (renumber && k.isInteger()) {
        sorted.append(arr->getValueRef(pos));
      } else {
        sorted.set(k, arr->getValueRef(pos));
      }
    }
    *opaque.original = sorted;
  }

  free(indices);
  return true;
}

int Array::SortRegularAscending(CVarRef v1, CVarRef v2, const void *data) {
  if (HPHP::less(v1, v2)) return -1;
  if (tvEqual(*v1.asTypedValue(), *v2.asTypedValue())) return 0;
  return 1;
}
int Array::SortRegularDescending(CVarRef v1, CVarRef v2, const void *data) {
  if (HPHP::less(v1, v2)) return 1;
  if (tvEqual(*v1.asTypedValue(), *v2.asTypedValue())) return 0;
  return -1;
}

int Array::SortNumericAscending(CVarRef v1, CVarRef v2, const void *data) {
  double d1 = v1.toDouble();
  double d2 = v2.toDouble();
  if (d1 < d2) return -1;
  if (d1 == d2) return 0;
  return 1;
}
int Array::SortNumericDescending(CVarRef v1, CVarRef v2, const void *data) {
  double d1 = v1.toDouble();
  double d2 = v2.toDouble();
  if (d1 < d2) return 1;
  if (d1 == d2) return 0;
  return -1;
}

int Array::SortStringAscending(CVarRef v1, CVarRef v2, const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_strcmp(s1.data(), s1.size(), s2.data(), s2.size());
}

int Array::SortStringAscendingCase(CVarRef v1, CVarRef v2, const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return bstrcasecmp(s1.data(), s1.size(), s2.data(), s2.size());
}

int Array::SortStringDescending(CVarRef v1, CVarRef v2, const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_strcmp(s2.data(), s2.size(), s1.data(), s1.size());
}

int Array::SortStringDescendingCase(CVarRef v1, CVarRef v2, const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return bstrcasecmp(s2.data(), s2.size(), s1.data(), s1.size());
}

int Array::SortLocaleStringAscending(CVarRef v1, CVarRef v2,
                                     const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();

  return strcoll(s1.data(), s2.data());
}

int Array::SortLocaleStringDescending(CVarRef v1, CVarRef v2,
                                      const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();

  return strcoll(s2.data(), s1.data());
}

int Array::SortNatural(CVarRef v1, CVarRef v2, const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s1.data(), s1.size(), s2.data(), s2.size(), 0);
}

int Array::SortNaturalCase(CVarRef v1, CVarRef v2, const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s1.data(), s1.size(), s2.data(), s2.size(), 1);
}

///////////////////////////////////////////////////////////////////////////////
}
