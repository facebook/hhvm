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

#include <runtime/base/complex_types.h>
#include <runtime/base/types.h>
#include <runtime/base/comparisons.h>
#include <util/exception.h>
#include <runtime/base/array/small_array.h>
#include <runtime/base/shared/shared_map.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/variable_unserializer.h>
#include <runtime/base/comparisons.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_qsort.h>
#include <runtime/base/zend/zend_printf.h>
#include <runtime/base/array/array_util.h>
#include <runtime/base/runtime_option.h>
#include <runtime/ext/ext_iconv.h>
#include <unicode/coll.h> // icu
#include <util/parser/hphp.tab.hpp>

using namespace std;

namespace HPHP {

const Array null_array = Array();

IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(Array);
///////////////////////////////////////////////////////////////////////////////
// constructors

Array Array::Create(CVarRef name, CVarRef var) {
  return ArrayData::Create(name.isString() ? name.toKey() : name, var);
}

///////////////////////////////////////////////////////////////////////////////
// operators

Array &Array::operator=(ArrayData *data) {
  SmartPtr<ArrayData>::operator=(data);
  return *this;
}

Array &Array::operator=(CArrRef arr) {
  SmartPtr<ArrayData>::operator=(arr.m_px);
  return *this;
}

Array &Array::operator=(CVarRef var) {
  return operator=(var.toArray());
}

Array &Array::operator=(const StaticArray &arr) {
  if (m_px != arr.m_px) {
    if (m_px && m_px->decRefCount() == 0) {
      m_px->release();
    }
    m_px = arr.m_px;
  }
  return *this;
}

Array Array::operator+(ArrayData *data) const {
  return Array(m_px).operator+=(data);
}

Array Array::operator+(CVarRef var) const {
  if (var.getType() != KindOfArray) {
    throw BadArrayMergeException();
  }
  return operator+(var.getArrayData());
}

Array Array::operator+(CArrRef arr) const {
  return operator+(arr.m_px);
}

Array &Array::operator+=(ArrayData *data) {
  return mergeImpl(data, ArrayData::Plus);
}
Array &Array::operator+=(CVarRef var) {
  if (var.getType() != KindOfArray) {
    throw BadArrayMergeException();
  }
  return operator+=(var.getArrayData());
}

Array &Array::operator+=(CArrRef arr) {
  return mergeImpl(arr.m_px, ArrayData::Plus);
}

Array Array::diff(CVarRef array, bool by_key, bool by_value,
                  PFUNC_CMP key_cmp_function /* = NULL */,
                  const void *key_data /* = NULL */,
                  PFUNC_CMP value_cmp_function /* = NULL */,
                  const void *value_data /* = NULL */) const {
  if (!array.isArray()) {
    throw_bad_array_exception();
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
    throw_bad_array_exception();
    return Array();
  }
  return diffImpl(array.getArrayData(), by_key, by_value, true,
                  key_cmp_function, key_data,
                  value_cmp_function, value_data);
}

int Array::CompareAsStrings(CVarRef v1, CVarRef v2, const void *data) {
  return equalAsStr(v1, v2) ? 0 : -1;
}

Array Array::diffImpl(CArrRef array, bool by_key, bool by_value, bool match,
                      PFUNC_CMP key_cmp_function,
                      const void *key_data,
                      PFUNC_CMP value_cmp_function,
                      const void *value_data) const {
  ASSERT(by_key || by_value);
  ASSERT(by_key || key_cmp_function == NULL);
  ASSERT(by_value || value_cmp_function == NULL);
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
        ret.addLval(key, true).setWithRef(value);
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
      ret.addLval(iter.first(), true).setWithRef(iter.secondRef());
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// manipulations

Array &Array::merge(CArrRef arr) {
  return mergeImpl(arr.m_px, ArrayData::Merge);
}

Array &Array::mergeImpl(ArrayData *data, ArrayData::ArrayOp op) {
  if (m_px == NULL || data == NULL) {
    throw BadArrayMergeException();
  }
  if (!data->empty()) {
    if (op != ArrayData::Merge && m_px->empty()) {
      SmartPtr<ArrayData>::operator=(data);
    } else if (m_px != data || op == ArrayData::Merge) {
      ArrayData *escalated = m_px->append(data, op, m_px->getCount() > 1);
      if (escalated) {
        SmartPtr<ArrayData>::operator=(escalated);
      }
    }
  } else if (op == ArrayData::Merge) {
    m_px->renumber();
  }
  return *this;
}

Array Array::slice(int offset, int length, bool preserve_keys) const {
  if (m_px == NULL) return Array();
  return ArrayUtil::Slice(m_px, offset, length, preserve_keys);
}

///////////////////////////////////////////////////////////////////////////////
// type conversions

Object Array::toObject() const {
  return HPHP::toObject(m_px);
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

bool Array::same(CArrRef v2) const {
  if (m_px == NULL && v2.get() == NULL) return true;
  if (m_px && v2.get()) {
    return m_px->equal(v2.get(), true);
  }
  return false;
}

bool Array::same(CObjRef v2) const {
  return false;
}

bool Array::equal(CArrRef v2) const {
  if (m_px == NULL || v2.get() == NULL) {
    return HPHP::equal(toBoolean(), v2.toBoolean());
  }
  return m_px->equal(v2.get(), false);
}

bool Array::equal(CObjRef v2) const {
  if (m_px == NULL || v2.get() == NULL) {
    return HPHP::equal(toBoolean(), v2.toBoolean());
  }
  return false;
}

bool Array::less(CArrRef v2, bool flip /* = false */) const {
  if (m_px == NULL || v2.get() == NULL) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  if (flip) {
    return v2.get()->compare(m_px) > 0;
  }
  return m_px->compare(v2.get()) < 0;
}

bool Array::less(CObjRef v2) const {
  if (m_px == NULL || v2.get() == NULL) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  return false;
}

bool Array::less(CVarRef v2) const {
  if (m_px == NULL || v2.isNull()) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  if (v2.getType() == KindOfArray) {
    return m_px->compare(v2.toArray().get()) < 0;
  }
  return v2.more(*this);
}

bool Array::more(CArrRef v2, bool flip /* = true */) const {
  if (m_px == NULL || v2.get() == NULL) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  if (flip) {
    return v2.get()->compare(m_px) < 0;
  }
  return m_px->compare(v2.get()) > 0;
}

bool Array::more(CObjRef v2) const {
  if (m_px == NULL || v2.get() == NULL) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  return true;
}

bool Array::more(CVarRef v2) const {
  if (m_px == NULL || v2.isNull()) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  if (v2.getType() == KindOfArray) {
    return v2.toArray().get()->compare(m_px) < 0;
  }
  return v2.less(*this);
}

///////////////////////////////////////////////////////////////////////////////
// iterator

ArrayIter Array::begin(CStrRef context /* = null_string */,
                       bool setIterDirty /* = false */) const {
  if (setIterDirty && m_px) m_px->iter_dirty_set();
  return m_px;
}

void Array::escalate(bool mutableIteration /* = false */) {
  if (m_px) {
    SmartPtr<ArrayData>::operator=(m_px->escalate(mutableIteration));
  }
}

///////////////////////////////////////////////////////////////////////////////
// offset functions

Variant Array::rvalAt(bool key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get(key ? 1LL : 0LL, flags & AccessFlags::Error);
  return null_variant;
}

CVarRef Array::rvalAtRef(bool key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get(key ? 1LL : 0LL, flags & AccessFlags::Error);
  return null_variant;
}

Variant Array::rvalAt(int key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get((int64)key, flags & AccessFlags::Error);
  return null_variant;
}

CVarRef Array::rvalAtRef(int key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get((int64)key, flags & AccessFlags::Error);
  return null_variant;
}

Variant Array::rvalAt(int64 key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get(key, flags & AccessFlags::Error);
  return null_variant;
}

CVarRef Array::rvalAtRef(int64 key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get(key, flags & AccessFlags::Error);
  return null_variant;
}

Variant Array::rvalAt(double key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get((int64)key, flags & AccessFlags::Error);
  return null_variant;
}

CVarRef Array::rvalAtRef(double key, ACCESSPARAMS_IMPL) const {
  if (m_px) return m_px->get((int64)key, flags & AccessFlags::Error);
  return null_variant;
}

CVarRef Array::rvalAtRef(litstr key, ACCESSPARAMS_IMPL) const {
  if (m_px) {
    bool error = flags & AccessFlags::Error;
    if (flags & AccessFlags::Key) return m_px->get(key, error);
    int64 n;
    int len = strlen(key);
    if (!is_strictly_integer(key, len, n)) {
      return m_px->get(key, error);
    } else {
      return m_px->get(n, error);
    }
  }
  return null_variant;
}

Variant Array::rvalAt(litstr key, ACCESSPARAMS_IMPL) const {
  return Array::rvalAtRef(key, flags);
}

CVarRef Array::rvalAtRef(CStrRef key, ACCESSPARAMS_IMPL) const {
  if (m_px) {
    bool error = flags & AccessFlags::Error;
    if (flags & AccessFlags::Key) return m_px->get(key, error);
    if (key.isNull()) return m_px->get(empty_string, error);
    int64 n;
    if (!key->isStrictlyInteger(n)) {
      return m_px->get(key, error);
    } else {
      return m_px->get(n, error);
    }
  }
  return null_variant;
}

Variant Array::rvalAt(CStrRef key, ACCESSPARAMS_IMPL) const {
  return Array::rvalAtRef(key, flags);
}

CVarRef Array::rvalAtRef(CVarRef key, ACCESSPARAMS_IMPL) const {
  if (!m_px) return null_variant;
  switch (key.m_type) {
  case KindOfUninit:
  case KindOfNull:
    return m_px->get(empty_string, flags & AccessFlags::Error);
  case KindOfBoolean:
  case KindOfInt32:
  case KindOfInt64:
    return m_px->get(key.m_data.num, flags & AccessFlags::Error);
  case KindOfDouble:
    return m_px->get((int64)key.m_data.dbl, flags & AccessFlags::Error);
  case KindOfStaticString:
  case KindOfString: {
    int64 n;
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
    if (key.isResource()) {
      return m_px->get(key.toInt64(), flags & AccessFlags::Error);
    }
    throw_bad_type_exception("Invalid type used as key");
    break;
  case KindOfVariant:
    return rvalAtRef(*(key.m_data.pvar), flags);
  default:
    ASSERT(false);
    break;
  }
  return null_variant;
}

Variant Array::rvalAt(CVarRef key, ACCESSPARAMS_IMPL) const {
  return Array::rvalAtRef(key, flags);
}

Variant *Array::lvalPtr(CStrRef key, bool forWrite, bool create) {
  if (create) {
    if (!m_px) {
      SmartPtr<ArrayData>::operator=(ArrayData::Create());
    }
    return &lvalAt(key, AccessFlags::Key);
  }
  Variant *ret = NULL;
  if (m_px) {
    ArrayData *escalated = m_px->lvalPtr(key, ret,
                                         forWrite && m_px->getCount() > 1,
                                         false);
    if (escalated) {
      SmartPtr<ArrayData>::operator=(escalated);
    }
  }
  return ret;
}

Variant *Array::lvalPtr(int64 key, bool forWrite, bool create) {
  if (create) {
    if (!m_px) {
      SmartPtr<ArrayData>::operator=(ArrayData::Create());
    }
    return &lvalAt(key, AccessFlags::None);
  }
  Variant *ret = NULL;
  if (m_px) {
    ArrayData *escalated = m_px->lvalPtr(key, ret,
                                         forWrite && m_px->getCount() > 1,
                                         false);
    if (escalated) {
      SmartPtr<ArrayData>::operator=(escalated);
    }
  }
  return ret;
}

Variant &Array::lvalAt() {
  if (!m_px) {
    SmartPtr<ArrayData>::operator=(ArrayData::Create());
  }
  Variant *ret = NULL;
  ArrayData *arr = m_px;
  ArrayData *escalated = arr->lvalNew(ret, arr->getCount() > 1);
  if (escalated) {
    SmartPtr<ArrayData>::operator=(escalated);
  }
  ASSERT(ret);
  return *ret;
}

Variant &Array::lvalAt(litstr  key, ACCESSPARAMS_IMPL) {
  if (flags & AccessFlags::Key) return lvalAtImpl(String(key), flags);
  return lvalAtImpl(String(key).toKey(), flags);
}

Variant &Array::lvalAt(CStrRef key, ACCESSPARAMS_IMPL) {
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

CVarRef Array::set(litstr  key, CVarRef v, bool isKey /* = false */) {
  if (isKey) return setImpl(String(key), v);
  return setImpl(String(key).toKey(), v);
}
CVarRef Array::set(CStrRef key, CVarRef v, bool isKey /* = false */) {
  if (isKey) return setImpl(key, v);
  return setImpl(key.toKey(), v);
}

CVarRef Array::set(CVarRef key, CVarRef v, bool isKey /* = false */) {
  if (key.getRawType() == KindOfInt64) {
    return setImpl(key.getNumData(), v);
  }
  if (isKey) return setImpl(key, v);
  VarNR k(key.toKey());
  if (!k.isNull()) {
    return setImpl(k, v);
  }
  return Variant::lvalBlackHole();
}

CVarRef Array::add(litstr  key, CVarRef v, bool isKey /* = false */) {
  if (isKey) return addImpl(String(key), v);
  return addImpl(String(key).toKey(), v);
}

CVarRef Array::add(CStrRef key, CVarRef v, bool isKey /* = false */) {
  if (isKey) return addImpl(key, v);
  return addImpl(key.toKey(), v);
}

CVarRef Array::add(CVarRef key, CVarRef v, bool isKey /* = false */) {
  if (key.getRawType() == KindOfInt64) {
    return addImpl(key.getNumData(), v);
  }
  if (isKey) return addImpl(key, v);
  VarNR k(key.toKey());
  if (!k.isNull()) {
    return addImpl(k, v);
  }
  return Variant::lvalBlackHole();
}

Variant &Array::addLval(litstr  key, bool isKey /* = false */) {
  if (isKey) return addLvalImpl(String(key));
  return addLvalImpl(String(key).toKey());
}

Variant &Array::addLval(CStrRef key, bool isKey /* = false */) {
  if (isKey) return addLvalImpl(key);
  return addLvalImpl(key.toKey());
}

Variant &Array::addLval(CVarRef key, bool isKey /* = false */) {
  if (key.getRawType() == KindOfInt64) {
    return addLvalImpl(key.getNumData());
  }
  if (isKey) return addLvalImpl(key);
  VarNR k(key.toKey());
  if (!k.isNull()) {
    return addLvalImpl(k);
  }
  return Variant::lvalBlackHole();
}

Variant Array::refvalAt(CStrRef key, bool isString /* = false */) {
  return ref(lvalAt(key, AccessFlags::IsKey(isString)));
}

Variant Array::argvalAt(bool byRef, CStrRef key, bool isString /* = false */) {
  if (byRef) {
    return ref(lvalAt(key, AccessFlags::IsKey(isString)));
  } else {
    return rvalAt(key);
  }
}

///////////////////////////////////////////////////////////////////////////////
// membership functions

bool Array::valueExists(CVarRef search_value,
                        bool strict /* = false */) const {
  for (ArrayIter iter(*this); iter; ++iter) {
    if ((strict && iter.secondRef().same(search_value)) ||
        (!strict && iter.secondRef().equal(search_value))) {
      return true;
    }
  }
  return false;
}

Variant Array::key(CVarRef search_value, bool strict /* = false */) const {
  for (ArrayIter iter(*this); iter; ++iter) {
    if ((strict && iter.secondRef().same(search_value)) ||
        (!strict && iter.secondRef().equal(search_value))) {
      return iter.first();
    }
  }
  return false; // PHP uses "false" over null in many places
}

Array Array::keys(CVarRef search_value /* = null_variant */,
                  bool strict /* = false */) const {
  if (search_value.isNull()) {
    ArrayInit ai(size(), true);
    for (ArrayIter iter(*this); iter; ++iter) {
      ai.set(iter.first());
    }
    return ai.create();
  } else {
    Array ret = Array::Create();
    for (ArrayIter iter(*this); iter; ++iter) {
      if ((strict && iter.secondRef().same(search_value)) ||
          (!strict && iter.secondRef().equal(search_value))) {
        ret.append(iter.first());
      }
    }
    return ret;
  }
}

Array Array::values() const {
  ArrayInit ai(size());
  for (ArrayIter iter(*this); iter; ++iter) {
    ai.set(iter.secondRef());
  }
  return ai.create();
}

bool Array::exists(litstr key, bool isKey /* = false */) const {
  if (isKey) return existsImpl(key);
  return existsImpl(String(key).toKey());
}

bool Array::exists(CStrRef key, bool isKey /* = false */) const {
  if (isKey) return existsImpl(key);
  return existsImpl(key.toKey());
}

bool Array::exists(CVarRef key, bool isKey /* = false */) const {
  switch(key.getType()) {
  case KindOfBoolean:
  case KindOfInt32:
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

void Array::remove(litstr  key, bool isString /* = false */) {
  if (isString) {
    removeImpl(key);
  } else {
    removeImpl(String(key).toKey());
  }
}
void Array::remove(CStrRef key, bool isString /* = false */) {
  if (isString) {
    removeImpl(key);
  } else {
    removeImpl(key.toKey());
  }
}

void Array::remove(CVarRef key) {
  switch(key.getType()) {
  case KindOfBoolean:
  case KindOfInt32:
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

Variant Array::append(CVarRef v) {
  if (!m_px) {
    SmartPtr<ArrayData>::operator=(ArrayData::Create(v));
  } else {
    ArrayData *escalated = m_px->append(v, (m_px->getCount() > 1));
    if (escalated) {
      SmartPtr<ArrayData>::operator=(escalated);
    }
  }
  return v;
}

Variant Array::appendWithRef(CVarRef v) {
  if (!m_px) {
    SmartPtr<ArrayData>::operator=(ArrayData::Create());
  }
  ArrayData *escalated = m_px->appendWithRef(v, (m_px->getCount() > 1));
  if (escalated) {
    SmartPtr<ArrayData>::operator=(escalated);
  }
  return v;
}

Variant Array::appendOpEqual(int op, CVarRef v) {
  if (!m_px) {
    SmartPtr<ArrayData>::operator=(ArrayData::Create());
  }
  Variant *cv = NULL;
  ArrayData *escalated = m_px->lvalNew(cv, m_px->getCount() > 1);
  if (escalated) {
    SmartPtr<ArrayData>::operator=(escalated);
  }
  ASSERT(cv);
  switch (op) {
  case T_CONCAT_EQUAL: return concat_assign((*cv), v);
  case T_PLUS_EQUAL:   return ((*cv) += v);
  case T_MINUS_EQUAL:  return ((*cv) -= v);
  case T_MUL_EQUAL:    return ((*cv) *= v);
  case T_DIV_EQUAL:    return ((*cv) /= v);
  case T_MOD_EQUAL:    return ((*cv) %= v);
  case T_AND_EQUAL:    return ((*cv) &= v);
  case T_OR_EQUAL:     return ((*cv) |= v);
  case T_XOR_EQUAL:    return ((*cv) ^= v);
  case T_SL_EQUAL:     return ((*cv) <<= v);
  case T_SR_EQUAL:     return ((*cv) >>= v);
  default:
    throw FatalErrorException(0, "invalid operator %d", op);
  }
}

Variant Array::pop() {
  if (m_px) {
    Variant ret;
    ArrayData *newarr = m_px->pop(ret);
    if (newarr) {
      SmartPtr<ArrayData>::operator=(newarr);
    }
    return ret;
  }
  return null_variant;
}

Variant Array::dequeue() {
  if (m_px) {
    Variant ret;
    ArrayData *newarr = m_px->dequeue(ret);
    if (newarr) {
      SmartPtr<ArrayData>::operator=(newarr);
    }
    return ret;
  }
  return null_variant;
}

void Array::prepend(CVarRef v) {
  if (!m_px) {
    operator=(Create());
  }
  ASSERT(m_px);

  ArrayData *newarr = m_px->prepend(v, (m_px->getCount() > 1));
  if (newarr) {
    SmartPtr<ArrayData>::operator=(newarr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// output functions

void Array::serialize(VariableSerializer *serializer) const {
  if (m_px) {
    m_px->serialize(serializer);
  } else {
    serializer->writeNull();
  }
}

void Array::unserialize(VariableUnserializer *uns) {
  int64 size = uns->readInt();
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
    bool isAPC = (uns->getType() == VariableUnserializer::APCSerialize);
    for (int64 i = 0; i < size; i++) {
      Variant key(uns->unserializeKey());
      if (!key.isString() && !key.isInteger()) {
        throw Exception("Invalid key");
      }
      Variant &value = isAPC ? addLval(key, true) :
        lvalAt(key, AccessFlags::Key);
      value.unserialize(uns);
    }
  }

  sep = uns->readChar();
  if (sep != '}') {
    throw Exception("Expected '}' but got '%c'", sep);
  }
}

Array Array::fiberMarshal(FiberReferenceMap &refMap) const {
  if (m_px) {
    Array ret = Array::Create();
    if (m_px->isGlobalArrayWrapper()) {
      ret = get_global_array_wrapper();
    } else if (m_px->isSharedMap()) {
      return ((SharedMap *)m_px)->fiberCopy();
    } else {
      for (ArrayIter iter(*this); iter; ++iter) {
        ret.set(iter.first().fiberMarshal(refMap),
                ref(iter.secondRef().fiberMarshal(refMap)));
      }
    }
    return ret;
  }
  return Array();
}

Array Array::fiberUnmarshal(FiberReferenceMap &refMap) const {
  if (m_px) {
    Array ret = Array::Create();
    if (m_px->isGlobalArrayWrapper()) {
      ret = get_global_array_wrapper();
    } else if (m_px->isSharedMap()) {
      return ((SharedMap *)m_px)->fiberCopy();
    } else {
      for (ArrayIter iter(*this); iter; ++iter) {
        ret.set(iter.first().fiberUnmarshal(refMap),
                ref(iter.secondRef().fiberUnmarshal(refMap)));
      }
    }
    return ret;
  }
  return Array();
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
  ASSERT(cmp_func);

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
      sorted.append(m_px->getValueRef(pos));
    } else {
      sorted.set(m_px->getKey(pos), m_px->getValueRef(pos));
    }
  }
  operator=(sorted);
}

bool Array::MultiSort(std::vector<SortData> &data, bool renumber) {
  ASSERT(!data.empty());

  int count = -1;
  for (unsigned int k = 0; k < data.size(); k++) {
    SortData &opaque = data[k];

    ASSERT(opaque.array);
    ASSERT(opaque.cmp_func);
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
  if (v1.less(v2)) return -1;
  if (v1.equal(v2)) return 0;
  return 1;
}
int Array::SortRegularDescending(CVarRef v1, CVarRef v2, const void *data) {
  if (v1.less(v2)) return 1;
  if (v1.equal(v2)) return 0;
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
  return strcmp(s1.data(), s2.data());
}
int Array::SortStringDescending(CVarRef v1, CVarRef v2, const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return strcmp(s2.data(), s1.data());
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
