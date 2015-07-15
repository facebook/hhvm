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

#include "hphp/runtime/base/type-array.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/zend-qsort.h"
#include "hphp/runtime/base/zend-string.h"

#include "hphp/parser/hphp.tab.hpp"

#include "hphp/util/exception.h"

#include <unicode/coll.h> // icu
#include <vector>

namespace HPHP {

const Array null_array{};
const Array empty_array_ref{staticEmptyArray()};
const StaticString array_string("Array");

void Array::setEvalScalar() const {
  Array* thisPtr = const_cast<Array*>(this);
  if (!m_arr) thisPtr->m_arr = Ptr::attach(ArrayData::Create());
  if (!m_arr->isStatic()) {
    thisPtr->m_arr = ArrayData::GetScalarArray(get());
  }
}

void Array::compileTimeAssertions() {
  static_assert(sizeof(Array) == sizeof(req::ptr<ArrayData>), "Fix this.");
}

void ArrNR::compileTimeAssertions() {
  static_assert(offsetof(ArrNR, m_px) == kExpectedMPxOffset, "");
}

///////////////////////////////////////////////////////////////////////////////
// constructors

Array Array::Create(const Variant& name, const Variant& var) {
  return Array{
    ArrayData::Create(name.isString() ? name.toKey() : name, var),
    NoIncRef{}
  };
}

Array::~Array() {}

///////////////////////////////////////////////////////////////////////////////
// operators

Array &Array::operator=(const Variant& var) {
  return operator=(var.toArray());
}

// Move assign
Array& Array::operator=(Variant&& v) {
  if (v.asTypedValue()->m_type == KindOfArray) {
    m_arr = req::ptr<ArrayData>::attach(v.asTypedValue()->m_data.parr);
    v.asTypedValue()->m_type = KindOfNull;
  } else {
    *this = const_cast<const Variant&>(v);
  }
  return *this;
}

Array Array::operator+(ArrayData *data) const {
  return Array(*this).plusImpl(data);
}

Array Array::operator+(const Array& arr) const {
  return Array(*this).plusImpl(arr.get());
}

Array &Array::operator+=(ArrayData *data) {
  return plusImpl(data);
}

NEVER_INLINE
static void throw_bad_array_merge() {
  throw ExtendedException("Invalid operand type was used: "
                          "merging an array with NULL or non-array.");
}

Array &Array::operator+=(const Variant& var) {
  if (var.getType() != KindOfArray) {
    throw_bad_array_merge();
  }
  return operator+=(var.getArrayData());
}

Array &Array::operator+=(const Array& arr) {
  return plusImpl(arr.get());
}

Array Array::diff(const Variant& array, bool by_key, bool by_value,
                  PFUNC_CMP key_cmp_function /* = NULL */,
                  const void *key_data /* = NULL */,
                  PFUNC_CMP value_cmp_function /* = NULL */,
                  const void *value_data /* = NULL */) const {
  if (!array.isArray()) {
    throw_expected_array_exception();
    return Array();
  }
  return diffImpl(array.toArray(), by_key, by_value, false,
                  key_cmp_function, key_data,
                  value_cmp_function, value_data);
}

Array Array::intersect(const Variant& array, bool by_key, bool by_value,
                       PFUNC_CMP key_cmp_function /* = NULL */,
                       const void *key_data /* = NULL */,
                       PFUNC_CMP value_cmp_function /* = NULL */,
                       const void *value_data /* = NULL */) const {
  if (!array.isArray()) {
    throw_expected_array_exception();
    return Array();
  }
  return diffImpl(array.toArray(), by_key, by_value, true,
                  key_cmp_function, key_data,
                  value_cmp_function, value_data);
}

static int CompareAsStrings(const Variant& v1, const Variant& v2, const void *data) {
  return HPHP::same(HPHP::toString(v1), HPHP::toString(v2)) ? 0 : -1;
}

Array Array::diffImpl(const Array& array, bool by_key, bool by_value, bool match,
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
      const Variant& value(iter.secondRef());
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

  std::vector<int> perm1;
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
        const Variant& val(iter.secondRef());
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

String Array::toString() const {
  if (m_arr == nullptr) return empty_string();
  raise_notice("Array to string conversion");
  return array_string;
}

Array &Array::merge(const Array& arr) {
  return mergeImpl(arr.get());
}

Array &Array::plusImpl(ArrayData *data) {
  if (m_arr == nullptr || data == nullptr) {
    throw_bad_array_merge();
  }
  if (!data->empty()) {
    if (m_arr->empty()) {
      m_arr = data;
    } else if (m_arr != data) {
      auto const escalated = m_arr->plusEq(data);
      if (escalated != m_arr) {
        m_arr = Ptr::attach(escalated);
      }
    }
  }
  return *this;
}

Array &Array::mergeImpl(ArrayData *data) {
  if (m_arr == nullptr || data == nullptr) {
    throw_bad_array_merge();
  }
  if (!data->empty()) {
    auto const escalated = m_arr->merge(data);
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  } else {
    m_arr->renumber();
  }
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

bool Array::same(const Array& v2) const {
  if (m_arr == nullptr && v2.get() == nullptr) return true;
  if (m_arr && v2.get()) {
    return m_arr->equal(v2.get(), true);
  }
  return false;
}

bool Array::same(const Object& v2) const {
  return false;
}

bool Array::equal(const Array& v2) const {
  if (m_arr == nullptr || v2.get() == nullptr) {
    return HPHP::equal(toBoolean(), v2.toBoolean());
  }
  return m_arr->equal(v2.get(), false);
}

bool Array::equal(const Object& v2) const {
  if (m_arr == nullptr || v2.get() == nullptr) {
    return HPHP::equal(toBoolean(), v2.toBoolean());
  }
  return false;
}

bool Array::less(const Array& v2, bool flip /* = false */) const {
  if (m_arr == nullptr || v2.get() == nullptr) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  if (flip) {
    return v2.get()->compare(get()) > 0;
  }
  return m_arr->compare(v2.get()) < 0;
}

bool Array::less(const Object& v2) const {
  if (m_arr == nullptr || v2.get() == nullptr) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  check_collection_compare(v2.get());
  return true;
}

bool Array::less(const Variant& v2) const {
  if (m_arr == nullptr || v2.isNull()) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  if (v2.getType() == KindOfArray) {
    return m_arr->compare(v2.toArray().get()) < 0;
  }
  return HPHP::more(v2, *this);
}

bool Array::more(const Array& v2, bool flip /* = true */) const {
  if (m_arr == nullptr || v2.get() == nullptr) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  if (flip) {
    return v2.get()->compare(get()) < 0;
  }
  return m_arr->compare(v2.get()) > 0;
}

bool Array::more(const Object& v2) const {
  if (m_arr == nullptr || v2.get() == nullptr) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  check_collection_compare(v2.get());
  return false;
}

bool Array::more(const Variant& v2) const {
  if (m_arr == nullptr || v2.isNull()) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  if (v2.getType() == KindOfArray) {
    return v2.toArray().get()->compare(get()) < 0;
  }
  return HPHP::less(v2, *this);
}

///////////////////////////////////////////////////////////////////////////////
// iterator

ArrayIter Array::begin(const String& context /* = null_string */) const {
  return ArrayIter(*this);
}

void Array::escalate() {
  if (m_arr) {
    auto escalated = m_arr->escalate();
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
}

///////////////////////////////////////////////////////////////////////////////
// offset functions

Variant Array::rvalAt(int key, ACCESSPARAMS_IMPL) const {
  if (m_arr) return m_arr->get((int64_t)key, flags & AccessFlags::Error);
  return init_null();
}

const Variant& Array::rvalAtRef(int key, ACCESSPARAMS_IMPL) const {
  if (m_arr) return m_arr->get((int64_t)key, flags & AccessFlags::Error);
  return null_variant;
}

Variant Array::rvalAt(int64_t key, ACCESSPARAMS_IMPL) const {
  if (m_arr) return m_arr->get(key, flags & AccessFlags::Error);
  return init_null();
}

const Variant& Array::rvalAtRef(int64_t key, ACCESSPARAMS_IMPL) const {
  if (m_arr) return m_arr->get(key, flags & AccessFlags::Error);
  return null_variant;
}

const Variant& Array::rvalAtRef(const String& key, ACCESSPARAMS_IMPL) const {
  if (m_arr) {
    bool error = flags & AccessFlags::Error;
    if (flags & AccessFlags::Key) return m_arr->get(key, error);
    if (key.isNull()) return m_arr->get(staticEmptyString(), error);
    int64_t n;
    if (!key.get()->isStrictlyInteger(n)) {
      return m_arr->get(key, error);
    } else {
      return m_arr->get(n, error);
    }
  }
  return null_variant;
}

Variant Array::rvalAt(const String& key, ACCESSPARAMS_IMPL) const {
  return Array::rvalAtRef(key, flags);
}

const Variant& Array::rvalAtRef(const Variant& key, ACCESSPARAMS_IMPL) const {
  if (!m_arr) return null_variant;
  switch (key.getRawType()) {
    case KindOfUninit:
    case KindOfNull:
      return m_arr->get(staticEmptyString(), flags & AccessFlags::Error);

    case KindOfBoolean:
    case KindOfInt64:
      return m_arr->get(key.asTypedValue()->m_data.num,
                       flags & AccessFlags::Error);

    case KindOfDouble:
      return m_arr->get((int64_t)key.asTypedValue()->m_data.dbl,
                       flags & AccessFlags::Error);

    case KindOfStaticString:
    case KindOfString:
      {
        int64_t n;
        if (!(flags & AccessFlags::Key) &&
            key.asTypedValue()->m_data.pstr->isStrictlyInteger(n)) {
          return m_arr->get(n, flags & AccessFlags::Error);
        }
      }
      return m_arr->get(key.asCStrRef(), flags & AccessFlags::Error);

    case KindOfArray:
    case KindOfObject:
      throw_bad_type_exception("Invalid type used as key");
      return null_variant;

    case KindOfResource:
      return m_arr->get(key.toInt64(), flags & AccessFlags::Error);

    case KindOfRef:
      return rvalAtRef(*(key.asTypedValue()->m_data.pref->var()), flags);

    case KindOfClass:
      break;
  }
  not_reached();
}

Variant Array::rvalAt(const Variant& key, ACCESSPARAMS_IMPL) const {
  return Array::rvalAtRef(key, flags);
}

Variant &Array::lvalAt() {
  if (!m_arr) m_arr = Ptr::attach(ArrayData::Create());
  Variant *ret = nullptr;
  auto arr = m_arr;
  ArrayData *escalated = arr->lvalNew(ret, arr->hasMultipleRefs());
  if (escalated != arr) m_arr = Ptr::attach(escalated);
  assert(ret);
  return *ret;
}

Variant &Array::lvalAtRef() {
  if (!m_arr) m_arr = Ptr::attach(ArrayData::Create());
  Variant *ret = nullptr;
  auto arr = m_arr;
  ArrayData *escalated = arr->lvalNewRef(ret, arr->hasMultipleRefs());
  if (escalated != arr) m_arr = Ptr::attach(escalated);
  assert(ret);
  return *ret;
}

Variant &Array::lvalAt(const String& key, ACCESSPARAMS_IMPL) {
  if (flags & AccessFlags::Key) return lvalAtImpl(key, flags);
  return lvalAtImpl(key.toKey(), flags);
}

Variant &Array::lvalAt(const Variant& key, ACCESSPARAMS_IMPL) {
  if (flags & AccessFlags::Key) return lvalAtImpl(key, flags);
  VarNR k(key.toKey());
  if (!k.isNull()) {
    return lvalAtImpl(k, flags);
  }
  return lvalBlackHole();
}

template<typename T>
ALWAYS_INLINE
void Array::setImpl(const T &key, const Variant& v) {
  if (!m_arr) {
    m_arr = Ptr::attach(ArrayData::Create(key, v));
  } else {
    ArrayData *escalated = m_arr->set(key, v, (m_arr->hasMultipleRefs()));
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
}

template<typename T>
ALWAYS_INLINE
void Array::setRefImpl(const T &key, Variant& v) {
  if (!m_arr) {
    m_arr = Ptr::attach(ArrayData::CreateRef(key, v));
  } else {
    escalate();
    ArrayData *escalated = m_arr->setRef(key, v, (m_arr->hasMultipleRefs()));
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
}

template<typename T>
ALWAYS_INLINE
void Array::addImpl(const T &key, const Variant& v) {
  if (!m_arr) {
    m_arr = Ptr::attach(ArrayData::Create(key, v));
  } else {
    ArrayData *escalated = m_arr->add(key, v, (m_arr->hasMultipleRefs()));
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
}

void Array::set(int64_t key, const Variant& v) {
  setImpl(key, v);
}

void Array::set(const String& key, const Variant& v, bool isKey /* = false */) {
  if (isKey) return setImpl(key, v);
  setImpl(key.toKey(), v);
}

void Array::set(const Variant& key, const Variant& v, bool isKey /* = false */) {
  if (key.getRawType() == KindOfInt64) return setImpl(key.getNumData(), v);
  if (isKey) return setImpl(key, v);
  VarNR k(key.toKey());
  if (!k.isNull()) setImpl(k, v);
}

void Array::setRef(int64_t key, Variant& v) {
  setRefImpl(key, v);
}

void Array::setRef(const String& key, Variant& v, bool isKey /* = false */) {
  if (isKey) return setRefImpl(key, v);
  setRefImpl(key.toKey(), v);
}

void Array::setRef(const Variant& key, Variant& v, bool isKey /* = false */) {
  if (key.getRawType() == KindOfInt64) return setRefImpl(key.getNumData(), v);
  if (isKey) return setRefImpl(key, v);
  VarNR k(key.toKey());
  if (!k.isNull()) setRefImpl<Variant>(k, v);
}

void Array::add(int64_t key, const Variant& v) {
  addImpl(key, v);
}

void Array::add(const String& key, const Variant& v, bool isKey /* = false */) {
  if (isKey) return addImpl(key, v);
  addImpl(key.toKey(), v);
}

void Array::add(const Variant& key, const Variant& v, bool isKey /* = false */) {
  if (key.getRawType() == KindOfInt64) return addImpl(key.getNumData(), v);
  if (isKey) return addImpl(key, v);
  VarNR k(key.toKey());
  if (!k.isNull()) addImpl(k, v);
}

///////////////////////////////////////////////////////////////////////////////
// membership functions

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

bool Array::exists(const Variant& key, bool isKey /* = false */) const {
  if (IS_BOOL_TYPE(key.getType()) ||
      IS_INT_TYPE(key.getType())) {
    return existsImpl(key.toInt64());
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

void Array::remove(const Variant& key) {
  if (IS_BOOL_TYPE(key.getType()) ||
      IS_INT_TYPE(key.getType())) {
    removeImpl(key.toInt64());
    return;
  }
  VarNR k(key.toKey());
  if (!k.isNull()) {
    removeImpl(k);
  }
}

const Variant& Array::append(const Variant& v) {
  if (!m_arr) {
    m_arr = Ptr::attach(ArrayData::Create(v));
  } else {
    ArrayData *escalated = m_arr->append(v, (m_arr->hasMultipleRefs()));
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
  return v;
}

const Variant& Array::appendRef(Variant& v) {
  if (!m_arr) {
    m_arr = Ptr::attach(ArrayData::CreateRef(v));
  } else {
    ArrayData *escalated = m_arr->appendRef(v, (m_arr->hasMultipleRefs()));
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
  return v;
}

const Variant& Array::appendWithRef(const Variant& v) {
  if (!m_arr) m_arr = Ptr::attach(ArrayData::Create());
  ArrayData *escalated = m_arr->appendWithRef(v, (m_arr->hasMultipleRefs()));
  if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  return v;
}

Variant Array::pop() {
  if (m_arr) {
    Variant ret;
    ArrayData *newarr = m_arr->pop(ret);
    if (newarr != m_arr) m_arr = Ptr::attach(newarr);
    return ret;
  }
  return init_null();
}

Variant Array::dequeue() {
  if (m_arr) {
    Variant ret;
    ArrayData *newarr = m_arr->dequeue(ret);
    if (newarr != m_arr) m_arr = Ptr::attach(newarr);
    return ret;
  }
  return init_null();
}

void Array::prepend(const Variant& v) {
  if (!m_arr) operator=(Create());
  assert(m_arr);
  ArrayData *newarr = m_arr->prepend(v, (m_arr->hasMultipleRefs()));
  if (newarr != m_arr) m_arr = Ptr::attach(newarr);
}

///////////////////////////////////////////////////////////////////////////////
// output functions

void Array::serialize(VariableSerializer *serializer,
                      bool isObject /* = false */) const {
  if (m_arr) {
    m_arr->serialize(serializer, isObject);
  } else {
    serializer->writeNull();
  }
}

void Array::unserialize(VariableUnserializer *uns) {
  int64_t size = uns->readInt();
  uns->expectChar(':');
  uns->expectChar('{');

  if (size == 0) {
    operator=(Create());
  } else {
    if (UNLIKELY(size < 0 || size > std::numeric_limits<int>::max())) {
      throw Exception("Array size out of bounds");
    }
    auto const scale = computeScaleFromSize(size);
    auto const allocsz = computeAllocBytes(scale);

    // For large arrays, do a naive pre-check for OOM.
    if (UNLIKELY(allocsz > kMaxSmallSize && MM().preAllocOOM(allocsz))) {
      check_request_surprise_unlikely();
    }

    // Pre-allocate an ArrayData of the given size, to avoid escalation in the
    // middle, which breaks references.
    operator=(ArrayInit(size, ArrayInit::Mixed{}).toArray());
    for (int64_t i = 0; i < size; i++) {
      Variant key;
      unserializeVariant(key, uns, UnserializeMode::Key);
      if (!key.isString() && !key.isInteger()) {
        throw Exception("Invalid key");
      }
      // for apc, we know the key can't exist, but ignore that optimization
      assert(uns->type() != VariableUnserializer::Type::APCSerialize ||
             !exists(key, true));

      Variant &value = lvalAt(key, AccessFlags::Key);
      if (UNLIKELY(IS_REFCOUNTED_TYPE(value.getRawType()))) {
        uns->putInOverwrittenList(value);
      }
      unserializeVariant(value, uns);

      if (i < (size - 1)) {
        auto lastChar = uns->peekBack();
        if ((lastChar != ';' && lastChar != '}')) {
          throw Exception("Array element not terminated properly");
        }
      }
    }
  }

  check_request_surprise_unlikely();

  uns->expectChar('}');
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

void Array::SortImpl(std::vector<int> &indices, const Array& source,
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
  auto pos_limit = source->iter_end();
  for (ssize_t pos = source->iter_begin(); pos != pos_limit;
       pos = source->iter_advance(pos)) {
    opaque.positions.push_back(pos);
  }
  zend_qsort(&indices[0], count, sizeof(int), array_compare_func, &opaque);
}

void Array::sort(PFUNC_CMP cmp_func, bool by_key, bool renumber,
                 const void *data /* = NULL */) {
  Array sorted = Array::Create();
  SortData opaque;
  std::vector<int> indices;
  SortImpl(indices, *this, opaque, cmp_func, by_key, data);
  int count = size();
  for (int i = 0; i < count; i++) {
    ssize_t pos = opaque.positions[indices[i]];
    if (renumber) {
      sorted.appendWithRef(m_arr->getValueRef(pos));
    } else {
      sorted.setWithRef(m_arr->getKey(pos), m_arr->getValueRef(pos), true);
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
    const Array& arr = *opaque.array;
    if (!arr.empty()) {
      auto pos_limit = arr->iter_end();
      for (ssize_t pos = arr->iter_begin(); pos != pos_limit;
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
    const Array& arr = *opaque.array;
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
    if (opaque.original->getRawType() == KindOfRef) {
      *opaque.original->getRefData() = sorted;
    }
  }

  free(indices);
  return true;
}

int Array::SortRegularAscending(const Variant& v1, const Variant& v2,
                                const void *data) {
  if (HPHP::less(v1, v2)) return -1;
  if (tvEqual(*v1.asTypedValue(), *v2.asTypedValue())) return 0;
  return 1;
}
int Array::SortRegularDescending(const Variant& v1, const Variant& v2,
                                 const void *data) {
  if (HPHP::less(v1, v2)) return 1;
  if (tvEqual(*v1.asTypedValue(), *v2.asTypedValue())) return 0;
  return -1;
}

int Array::SortNumericAscending(const Variant& v1, const Variant& v2,
                                const void *data) {
  double d1 = v1.toDouble();
  double d2 = v2.toDouble();
  if (d1 < d2) return -1;
  if (d1 == d2) return 0;
  return 1;
}
int Array::SortNumericDescending(const Variant& v1, const Variant& v2,
                                 const void *data) {
  double d1 = v1.toDouble();
  double d2 = v2.toDouble();
  if (d1 < d2) return 1;
  if (d1 == d2) return 0;
  return -1;
}

int Array::SortStringAscending(const Variant& v1, const Variant& v2,
                               const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_strcmp(s1.data(), s1.size(), s2.data(), s2.size());
}

int Array::SortStringAscendingCase(const Variant& v1, const Variant& v2,
                                   const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return bstrcasecmp(s1.data(), s1.size(), s2.data(), s2.size());
}

int Array::SortStringDescending(const Variant& v1, const Variant& v2,
                                const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_strcmp(s2.data(), s2.size(), s1.data(), s1.size());
}

int Array::SortStringDescendingCase(const Variant& v1, const Variant& v2,
                                    const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return bstrcasecmp(s2.data(), s2.size(), s1.data(), s1.size());
}

int Array::SortLocaleStringAscending(const Variant& v1, const Variant& v2,
                                     const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();

  return strcoll(s1.data(), s2.data());
}

int Array::SortLocaleStringDescending(const Variant& v1, const Variant& v2,
                                      const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();

  return strcoll(s2.data(), s1.data());
}

int Array::SortNaturalAscending(const Variant& v1, const Variant& v2,
                                const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s1.data(), s1.size(), s2.data(), s2.size(), 0);
}

int Array::SortNaturalDescending(const Variant& v1, const Variant& v2,
                                 const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s2.data(), s2.size(), s1.data(), s1.size(), 0);
}

int Array::SortNaturalCaseAscending(const Variant& v1, const Variant& v2,
                                    const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s1.data(), s1.size(), s2.data(), s2.size(), 1);
}

int Array::SortNaturalCaseDescending(const Variant& v1, const Variant& v2,
                                     const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s2.data(), s2.size(), s1.data(), s1.size(), 1);
}

///////////////////////////////////////////////////////////////////////////////
}
