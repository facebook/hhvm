/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/double-to-int64.h"
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

#include "hphp/runtime/ext/extension.h"

#include <unicode/coll.h> // icu
#include <vector>

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

const Array null_array{};
const Array empty_array_ref{staticEmptyArray()};
const StaticString array_string("Array");
const StaticString vec_string("Vec");
const StaticString dict_string("Dict");
const StaticString keyset_string("Keyset");

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
    ArrayData::Create(
      name.isString() ? name.toKey(staticEmptyArray()) : name,
      var
    ),
    NoIncRef{}
  };
}

Array::~Array() {}

///////////////////////////////////////////////////////////////////////////////

Array& Array::operator=(const Variant& var) {
  return operator=(var.toArray());
}

Array& Array::operator=(Variant&& v) {
  if (isArrayLikeType(v.asTypedValue()->m_type)) {
    m_arr = req::ptr<ArrayData>::attach(v.asTypedValue()->m_data.parr);
    v.asTypedValue()->m_type = KindOfNull;
  } else {
    *this = const_cast<const Variant&>(v);
  }
  return *this;
}

void Array::escalate() {
  if (m_arr) {
    auto escalated = m_arr->escalate();
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
}

///////////////////////////////////////////////////////////////////////////////

Array Array::values() const {
  PackedArrayInit ai(size());
  for (ArrayIter iter(*this); iter; ++iter) {
    ai.appendWithRef(iter.secondVal());
  }
  return ai.toArray();
}

ArrayIter Array::begin(const String& /*context*/ /* = null_string */) const {
  return ArrayIter(*this);
}

///////////////////////////////////////////////////////////////////////////////
// PHP operations.

Array Array::operator+(ArrayData *data) const {
  return Array(*this).plusImpl(data);
}

Array Array::operator+(const Array& arr) const {
  return Array(*this).plusImpl(arr.get());
}

Array& Array::operator+=(ArrayData *data) {
  return plusImpl(data);
}

NEVER_INLINE
static void throw_bad_array_merge() {
  throw ExtendedException("Invalid operand type was used: "
                          "merging an array with NULL or non-array.");
}

Array& Array::operator+=(const Variant& var) {
  if (!var.isArray()) {
    throw_bad_array_merge();
  }
  return operator+=(var.getArrayData());
}

Array& Array::operator+=(const Array& arr) {
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

static int
CompareAsStrings(const Variant& v1, const Variant& v2, const void* /*data*/) {
  return HPHP::same(v1.toString(), v2.toString()) ? 0 : -1;
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
      auto const value = iter.secondVal();
      bool found = false;
      if (array->exists(key)) {
        if (by_value) {
          found = value_cmp_as_string_function(
            VarNR(value),
            array.rvalAt(key, AccessFlags::Key),
            value_data
          ) == 0;
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
                                : VarNR(array->atPos(pos)),
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
        auto const val = iter.secondVal();
        // Have to look up and down for matches
        for (int i = mid; i < max; i++) {
          ssize_t pos = opaque1.positions[perm1[i]];
          if (key_cmp_function(target, array->getKey(pos), key_data) != 0) {
            break;
          }
          if (value_cmp_as_string_function(
                VarNR(val),
                VarNR(array->atPos(pos)),
                value_data
              ) == 0) {
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
            if (value_cmp_as_string_function(
                  VarNR(val),
                  VarNR(array->atPos(pos)),
                  value_data
                ) == 0) {
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
      ret.setWithRef(iter.first(), iter.secondVal(), true);
    }
  }
  return ret;
}

Array& Array::merge(const Array& arr) {
  return mergeImpl(arr.get());
}

Array& Array::plusImpl(ArrayData *data) {
  if (m_arr == nullptr || data == nullptr) {
    throw_bad_array_merge();
  }
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatAdd();
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

Array& Array::mergeImpl(ArrayData *data) {
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
// Type conversions.

String Array::toString() const {
  if (m_arr == nullptr) return empty_string();
  if (m_arr->isPHPArray()) {
    raise_notice("Array to string conversion");
    return array_string;
  }
  assert(m_arr->isHackArray());
  if (m_arr->isVecArray()) {
    raise_notice("Vec to string conversion");
    return vec_string;
  }
  if (m_arr->isDict()) {
    raise_notice("Dict to string conversion");
    return dict_string;
  }
  assert(m_arr->isKeyset());
  raise_notice("Keyset to string conversion");
  return keyset_string;
}

///////////////////////////////////////////////////////////////////////////////
// Comparisons.

bool Array::same(const Array& v2) const {
  if (!m_arr) return !v2.get();
  if (m_arr->isPHPArray()) {
    if (UNLIKELY(!v2.isPHPArray())) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
                   v2.isHackArray())) {
        raiseHackArrCompatArrMixedCmp();
      }
      return false;
    }
    return ArrayData::Same(m_arr.get(), v2.get());
  }

  auto const nonHackArr = [&]{
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
                 v2.isPHPArray() && !v2.isNull())) {
      raiseHackArrCompatArrMixedCmp();
    }
  };

  if (m_arr->isVecArray()) {
    if (UNLIKELY(!v2.isVecArray())) {
      nonHackArr();
      return false;
    }
    return PackedArray::VecSame(m_arr.get(), v2.get());
  }
  if (m_arr->isDict()) {
    if (UNLIKELY(!v2.isDict())) {
      nonHackArr();
      return false;
    }
    return MixedArray::DictSame(m_arr.get(), v2.get());
  }
  if (m_arr->isKeyset()) {
    if (UNLIKELY(!v2.isKeyset())) {
      nonHackArr();
      return false;
    }
    return SetArray::Same(m_arr.get(), v2.get());
  }
  not_reached();
}

bool Array::same(const Object& /*v2*/) const {
  return false;
}

bool Array::equal(const Array& v2) const {
  if (isPHPArray()) {
    if (UNLIKELY(!v2.isPHPArray())) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices && m_arr)) {
        raiseHackArrCompatArrMixedCmp();
      }
      return false;
    }
    if (m_arr == nullptr || v2.get() == nullptr) {
      return HPHP::equal(toBoolean(), v2.toBoolean());
    }
    return ArrayData::Equal(m_arr.get(), v2.get());
  }

  auto const nonHackArr = [&]{
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
                 v2.isPHPArray() && !v2.isNull())) {
      raiseHackArrCompatArrMixedCmp();
    }
  };

  if (m_arr->isVecArray()) {
    if (UNLIKELY(!v2.isVecArray())) {
      nonHackArr();
      return false;
    }
    return PackedArray::VecEqual(m_arr.get(), v2.get());
  }
  if (m_arr->isDict()) {
    if (UNLIKELY(!v2.isDict())) {
      nonHackArr();
      return false;
    }
    return MixedArray::DictEqual(m_arr.get(), v2.get());
  }
  if (m_arr->isKeyset()) {
    if (UNLIKELY(!v2.isKeyset())) {
      nonHackArr();
      return false;
    }
    return SetArray::Equal(m_arr.get(), v2.get());
  }
  not_reached();
}

bool Array::equal(const Object& v2) const {
  if (LIKELY(isPHPArray())) {
    if (m_arr == nullptr || v2.get() == nullptr) {
      return HPHP::equal(toBoolean(), v2.toBoolean());
    }
  }
  return false;
}

bool Array::less(const Array& v2, bool flip /* = false */) const {
  if (isPHPArray()) {
    if (UNLIKELY(!v2.isPHPArray())) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices && m_arr)) {
        raiseHackArrCompatArrMixedCmp();
      }
      if (v2.isVecArray()) throw_vec_compare_exception();
      if (v2.isDict()) throw_dict_compare_exception();
      if (v2.isKeyset()) throw_keyset_compare_exception();
      not_reached();
    }
    if (m_arr == nullptr || v2.get() == nullptr) {
      return HPHP::less(toBoolean(), v2.toBoolean());
    }
    return flip
      ? ArrayData::Gt(v2.get(), m_arr.get())
      : ArrayData::Lt(m_arr.get(), v2.get());
  }
  if (m_arr->isVecArray()) {
    if (UNLIKELY(!v2.isVecArray())) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
                   v2.isPHPArray() && !v2.isNull())) {
        raiseHackArrCompatArrMixedCmp();
      }
      throw_vec_compare_exception();
    }
    return flip
      ? PackedArray::VecGt(v2.get(), m_arr.get())
      : PackedArray::VecLt(m_arr.get(), v2.get());
  }
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
               v2.isPHPArray() && !v2.isNull())) {
    raiseHackArrCompatArrMixedCmp();
  }
  if (m_arr->isDict()) throw_dict_compare_exception();
  if (m_arr->isKeyset()) throw_keyset_compare_exception();
  not_reached();
}

bool Array::less(const Object& v2) const {
  if (LIKELY(isPHPArray())) {
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices && m_arr)) {
      raiseHackArrCompatArrMixedCmp();
    }
    if (m_arr == nullptr || v2.get() == nullptr) {
      return HPHP::less(toBoolean(), v2.toBoolean());
    }
    check_collection_compare(v2.get());
    return true;
  }
  if (m_arr->isVecArray()) throw_vec_compare_exception();
  if (m_arr->isDict()) throw_dict_compare_exception();
  if (m_arr->isKeyset()) throw_keyset_compare_exception();
  not_reached();
}

bool Array::less(const Variant& v2) const {
  if (isPHPArray()) {
    if (m_arr == nullptr || v2.isNull()) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
                   ((bool)m_arr == !v2.isPHPArray()))) {
        raiseHackArrCompatArrMixedCmp();
      }
      return HPHP::less(toBoolean(), v2.toBoolean());
    }
  }
  return HPHP::more(v2, *this);
}

bool Array::more(const Array& v2, bool flip /* = true */) const {
  if (isPHPArray()) {
    if (UNLIKELY(!v2.isPHPArray())) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices && m_arr)) {
        raiseHackArrCompatArrMixedCmp();
      }
      if (v2.isVecArray()) throw_vec_compare_exception();
      if (v2.isDict()) throw_dict_compare_exception();
      if (v2.isKeyset()) throw_keyset_compare_exception();
      not_reached();
    }
    if (m_arr == nullptr || v2.get() == nullptr) {
      return HPHP::more(toBoolean(), v2.toBoolean());
    }
    return flip
      ? ArrayData::Lt(v2.get(), m_arr.get())
      : ArrayData::Gt(m_arr.get(), v2.get());
  }
  if (m_arr->isVecArray()) {
    if (UNLIKELY(!v2.isVecArray())) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
                   v2.isPHPArray() && !v2.isNull())) {
        raiseHackArrCompatArrMixedCmp();
      }
      throw_vec_compare_exception();
    }
    return flip
      ? PackedArray::VecGt(v2.get(), m_arr.get())
      : PackedArray::VecLt(m_arr.get(), v2.get());
  }
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
               v2.isPHPArray() && !v2.isNull())) {
    raiseHackArrCompatArrMixedCmp();
  }
  if (m_arr->isDict()) throw_dict_compare_exception();
  if (m_arr->isKeyset()) throw_keyset_compare_exception();
  not_reached();
}

bool Array::more(const Object& v2) const {
  if (LIKELY(isPHPArray())) {
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices && m_arr)) {
      raiseHackArrCompatArrMixedCmp();
    }
    if (m_arr == nullptr || v2.get() == nullptr) {
      return HPHP::more(toBoolean(), v2.toBoolean());
    }
    check_collection_compare(v2.get());
    return false;
  }
  if (isVecArray()) throw_vec_compare_exception();
  if (isDict()) throw_dict_compare_exception();
  if (isKeyset()) throw_keyset_compare_exception();
  not_reached();
}

bool Array::more(const Variant& v2) const {
  if (isPHPArray()) {
    if (m_arr == nullptr || v2.isNull()) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
                   ((bool)m_arr == !v2.isPHPArray()))) {
        raiseHackArrCompatArrMixedCmp();
      }
      return HPHP::more(toBoolean(), v2.toBoolean());
    }
  }
  return HPHP::less(v2, *this);
}

int Array::compare(const Array& v2, bool flip /* = false */) const {
  if (isPHPArray()) {
    if (UNLIKELY(!v2.isPHPArray())) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices && m_arr)) {
        raiseHackArrCompatArrMixedCmp();
      }
      if (v2.isVecArray()) throw_vec_compare_exception();
      if (v2.isDict()) throw_dict_compare_exception();
      if (v2.isKeyset()) throw_keyset_compare_exception();
      not_reached();
    }
    if (m_arr == nullptr || v2.get() == nullptr) {
      return HPHP::compare(toBoolean(), v2.toBoolean());
    }
    return flip
      ? -ArrayData::Compare(v2.get(), m_arr.get())
      : ArrayData::Compare(m_arr.get(), v2.get());
  }
  if (m_arr->isVecArray()) {
    if (UNLIKELY(!v2.isVecArray())) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
                   v2.isPHPArray() && !v2.isNull())) {
        raiseHackArrCompatArrMixedCmp();
      }
      throw_vec_compare_exception();
    }
    return flip
      ? -PackedArray::VecCmp(v2.get(), m_arr.get())
      : PackedArray::VecCmp(m_arr.get(), v2.get());
  }
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
               v2.isPHPArray() && !v2.isNull())) {
    raiseHackArrCompatArrMixedCmp();
  }
  if (m_arr->isDict()) throw_dict_compare_exception();
  if (m_arr->isKeyset()) throw_keyset_compare_exception();
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> ALWAYS_INLINE
const Variant& Array::rvalAtImpl(const T& key, AccessFlags flags) const {
  if (!m_arr) return uninit_variant;
  return m_arr->get(key, any(flags & AccessFlags::Error));
}

template<typename T> ALWAYS_INLINE
member_lval Array::lvalAtImpl(const T& key, AccessFlags) {
  if (!m_arr) m_arr = Ptr::attach(ArrayData::Create());
  auto const lval = m_arr->lval(key, m_arr->cowCheck());
  if (lval.arr_base() != m_arr) m_arr = Ptr::attach(lval.arr_base());
  assert(lval.has_ref());
  return lval;
}

template<typename T> ALWAYS_INLINE
member_lval Array::lvalAtRefImpl(const T& key, AccessFlags) {
  if (!m_arr) m_arr = Ptr::attach(ArrayData::Create());
  auto const lval = m_arr->lvalRef(key, m_arr->cowCheck());
  if (lval.arr_base() != m_arr) m_arr = Ptr::attach(lval.arr_base());
  assert(lval.has_ref());
  return lval;
}

template<typename T> ALWAYS_INLINE
bool Array::existsImpl(const T& key) const {
  if (m_arr) return m_arr->exists(key);
  return false;
}

template<typename T> ALWAYS_INLINE
void Array::removeImpl(const T& key) {
  if (m_arr) {
    ArrayData* escalated = m_arr->remove(key, m_arr->cowCheck());
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
}

template<typename T> ALWAYS_INLINE
void Array::setImpl(const T& key, TypedValue v) {
  if (!m_arr) {
    m_arr = Ptr::attach(ArrayData::Create(key, v));
  } else {
    auto const escalated = m_arr->set(key, tvToCell(v), m_arr->cowCheck());
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
}

template<typename T> ALWAYS_INLINE
void Array::setRefImpl(const T& key, Variant& v) {
  if (!m_arr) {
    m_arr = Ptr::attach(ArrayData::CreateRef(key, v));
  } else {
    escalate();
    auto const escalated = m_arr->setRef(key, v, m_arr->cowCheck());
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
}

template<typename T> ALWAYS_INLINE
void Array::addImpl(const T& key, TypedValue v) {
  if (!m_arr) {
    m_arr = Ptr::attach(ArrayData::Create(key, v));
  } else {
    auto const escalated = m_arr->add(key, tvToCell(v), m_arr->cowCheck());
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
}

///////////////////////////////////////////////////////////////////////////////

namespace detail {

/*
 * The "element not found" sentinel type for each type returned by an Array
 * element access or mutation function.
 */
template<typename T> T not_found();
template<> void not_found<void>() { return; }
template<> bool not_found<bool>() { return false; }

template<> const Variant& not_found<const Variant&>() { return uninit_variant; }
template<> Variant& not_found<Variant&>() { return lvalBlackHole(); }

template<> member_lval not_found<member_lval>() {
  return member_lval { nullptr, lvalBlackHole().asTypedValue() };
}

/*
 * Implementation wrapper for Array element functions.
 *
 * These handle key conversion and value type canonicalization, and then
 * dispatch to an Array::fooImpl() function.
 */
template<typename Fn, typename... Args> ALWAYS_INLINE
decltype(auto) elem(const Array& arr, Fn fn, bool is_key,
                    const String& key, Args&&... args) {
  if (is_key) return fn(key, std::forward<Args>(args)...);

  auto const ad = arr.get() ? arr.get() : staticEmptyArray();

  // The logic here is a specialization of cellToKey().
  if (key.isNull()) {
    if (!ad->useWeakKeys()) {
      throwInvalidArrayKeyException(uninit_variant.asTypedValue(), ad);
    }
    if (RuntimeOption::EvalHackArrCompatNotices) {
      raiseHackArrCompatImplicitArrayKey(uninit_variant.asTypedValue());
    }
    return fn(make_tv<KindOfPersistentString>(staticEmptyString()),
              std::forward<Args>(args)...);
  }

  int64_t n;
  if (ad->convertKey(key.get(), n)) {
    return fn(n, std::forward<Args>(args)...);
  }
  return fn(key, std::forward<Args>(args)...);
}

template<typename Fn, typename... Args> ALWAYS_INLINE
decltype(auto) elem(const Array& arr, Fn fn, bool is_key,
                    Cell key, Args&&... args) {
  if (isIntType(key.m_type)) {
    return fn(key.m_data.num, std::forward<Args>(args)...);
  }
  if (is_key) {
    return fn(key, std::forward<Args>(args)...);
  }
  auto const k = arr.convertKey(key);
  if (!isNullType(k.m_type)) {
    return fn(k, std::forward<Args>(args)...);
  }
  return not_found<decltype(fn(key, std::forward<Args>(args)...))>();
}

template<typename Fn, typename... Args> ALWAYS_INLINE
decltype(auto) elem(const Array& arr, Fn fn, bool is_key,
                    const Variant& key, Args&&... args) {
  return elem(arr, fn, is_key, *key.asCell(), std::forward<Args>(args)...);
}

/*
 * Conversion helpers.
 */
Variant& as_var(member_lval lval) { return tvAsVariant(lval.tv()); }

}

#define WRAP(name)                                              \
  [&] (auto&&... ts) -> decltype(auto) {                        \
    return this->name##Impl(std::forward<decltype(ts)>(ts)...); \
  }

#define FOR_EACH_KEY_TYPE(...)    \
  C(Cell, __VA_ARGS__)            \
  I(int, __VA_ARGS__)             \
  I(int64_t, __VA_ARGS__)         \
  V(const String&, __VA_ARGS__)   \
  V(const Variant&, __VA_ARGS__)

#define FK(flags) any(flags & AccessFlags::Key)

#define C(key_t, name, ret_t, var_ret_t, conv, cns)         \
  ret_t Array::name(key_t k, AccessFlags fl) cns {          \
    return detail::elem(*this, WRAP(name), FK(fl), k, fl);  \
  }
#define V(key_t, name, ret_t, var_ret_t, conv, cns)               \
  var_ret_t Array::name(key_t k, AccessFlags fl) cns {            \
    return conv(detail::elem(*this, WRAP(name), FK(fl), k, fl));  \
  }
#define I(key_t, name, ret_t, var_ret_t, conv, cns)     \
  var_ret_t Array::name(key_t k, AccessFlags fl) cns {  \
    return conv(name##Impl(int64_t(k), fl));            \
  }

FOR_EACH_KEY_TYPE(rvalAt, const Variant&, const Variant&, identity, const)
FOR_EACH_KEY_TYPE(lvalAt, member_lval, Variant&, detail::as_var, )
FOR_EACH_KEY_TYPE(lvalAtRef, member_lval, Variant&, detail::as_var, )

#undef I
#undef V
#undef C

#define C(key_t, ret_t, name, cns)                    \
  ret_t Array::name(key_t k, bool isKey) cns {        \
    return detail::elem(*this, WRAP(name), isKey, k); \
  }
#define V C
#define I(key_t, ret_t, name, cns)  \
  ret_t Array::name(key_t k) cns { return name##Impl(int64_t(k)); }

FOR_EACH_KEY_TYPE(bool, exists, const)
FOR_EACH_KEY_TYPE(void, remove, )

#undef I
#undef V
#undef C

#define C(key_t, name, value_t)                           \
  void Array::name(key_t k, value_t v, bool isKey) {      \
    return detail::elem(*this, WRAP(name), isKey, k, v);  \
  }
#define V(key_t, name, value_t)                           \
  void Array::name(key_t k, value_t v, bool isKey) {      \
    return detail::elem(*this, WRAP(name), isKey, k, v);  \
  }
#define I(key_t, name, value_t)           \
  void Array::name(key_t k, value_t v) {  \
    return name##Impl(int64_t(k), v);     \
  }

FOR_EACH_KEY_TYPE(set, TypedValue)
FOR_EACH_KEY_TYPE(add, TypedValue)
FOR_EACH_KEY_TYPE(setRef, Variant&)

#undef I
#undef V
#undef C

#define C(key_t, name) \
  void Array::name(key_t k, TypedValue v, bool isKey) {         \
    auto lval = lvalAt(k, isKey ? Flags::Key : Flags::None);    \
    tvSetWithRef(v, *lval.tv());                                \
    if (lval.type() == KindOfUninit) lval.type() = KindOfNull;  \
  }
#define V(key_t, name) \
  void Array::name(key_t k, TypedValue v, bool isKey) {         \
    lvalAt(k, isKey ? Flags::Key : Flags::None).setWithRef(v);  \
  }
#define I(key_t, name) \
  void Array::name(key_t k, TypedValue v) { \
    lvalAt(k).setWithRef(v);                \
  }

FOR_EACH_KEY_TYPE(setWithRef)

#undef I
#undef V
#undef C

#define C(key_t, name)
#define V(key_t, name)                                      \
  void Array::name(key_t k, const Variant& v, bool isKey) { \
    return name(k, *v.asTypedValue(), isKey);               \
  }
#define I(key_t, name)                          \
  void Array::name(key_t k, const Variant& v) { \
    return name(k, *v.asTypedValue());          \
  }

FOR_EACH_KEY_TYPE(set)
FOR_EACH_KEY_TYPE(setWithRef)
FOR_EACH_KEY_TYPE(add)

#undef I
#undef V
#undef C

#undef FOR_EACH_KEY_TYPE
#undef WRAP

///////////////////////////////////////////////////////////////////////////////

Variant& Array::lvalAt() {
  if (!m_arr) m_arr = Ptr::attach(ArrayData::Create());
  auto const lval = m_arr->lvalNew(m_arr->cowCheck());
  if (lval.arr_base() != m_arr) m_arr = Ptr::attach(lval.arr_base());
  assert(lval.tv());
  return tvAsVariant(lval.tv());
}

Variant& Array::lvalAtRef() {
  if (!m_arr) m_arr = Ptr::attach(ArrayData::Create());
  auto const lval = m_arr->lvalNewRef(m_arr->cowCheck());
  if (lval.arr_base() != m_arr) m_arr = Ptr::attach(lval.arr_base());
  assert(lval.tv());
  return tvAsVariant(lval.tv());
}

void Array::append(TypedValue v) {
  if (!m_arr) operator=(Create());
  assertx(m_arr);
  auto const escalated = m_arr->append(tvToInitCell(v), m_arr->cowCheck());
  if (escalated != m_arr) m_arr = Ptr::attach(escalated);
}

void Array::appendRef(Variant& v) {
  if (!m_arr) {
    m_arr = Ptr::attach(ArrayData::CreateRef(v));
  } else {
    auto const escalated = m_arr->appendRef(v, m_arr->cowCheck());
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
}

void Array::appendWithRef(TypedValue v) {
  if (!m_arr) m_arr = Ptr::attach(ArrayData::Create());
  auto const escalated = m_arr->appendWithRef(v, m_arr->cowCheck());
  if (escalated != m_arr) m_arr = Ptr::attach(escalated);
}

void Array::prepend(TypedValue v) {
  if (!m_arr) operator=(Create());
  assertx(m_arr);
  auto const escalated = m_arr->prepend(tvToInitCell(v), m_arr->cowCheck());
  if (escalated != m_arr) m_arr = Ptr::attach(escalated);
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

///////////////////////////////////////////////////////////////////////////////
// Sorting.

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
  return opaque->cmp_func(
    VarNR((*opaque->array)->atPos(pos1)),
    VarNR((*opaque->array)->atPos(pos2)),
    opaque->data
  );
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
      result = opaque->cmp_func(
        VarNR((*opaque->array)->atPos(pos1)),
        VarNR((*opaque->array)->atPos(pos2)),
        opaque->data
      );
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
      sorted.appendWithRef(m_arr->atPos(pos));
    } else {
      sorted.setWithRef(m_arr->nvGetKey(pos), m_arr->atPos(pos), true);
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

  for (unsigned int ki = 0; ki < data.size(); ki++) {
    SortData &opaque = data[ki];
    const Array& arr = *opaque.array;
    Array sorted;
    for (int i = 0; i < count; i++) {
      ssize_t pos = opaque.positions[indices[i]];
      Variant k(arr->getKey(pos));
      if (renumber && k.isInteger()) {
        sorted.append(arr->atPos(pos));
      } else {
        sorted.set(k, arr->atPos(pos));
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
                                const void* /*data*/) {
  if (HPHP::less(v1, v2)) return -1;
  if (tvEqual(*v1.asTypedValue(), *v2.asTypedValue())) return 0;
  return 1;
}
int Array::SortRegularDescending(const Variant& v1, const Variant& v2,
                                 const void* /*data*/) {
  if (HPHP::less(v1, v2)) return 1;
  if (tvEqual(*v1.asTypedValue(), *v2.asTypedValue())) return 0;
  return -1;
}

int Array::SortNumericAscending(const Variant& v1, const Variant& v2,
                                const void* /*data*/) {
  double d1 = v1.toDouble();
  double d2 = v2.toDouble();
  if (d1 < d2) return -1;
  if (d1 == d2) return 0;
  return 1;
}
int Array::SortNumericDescending(const Variant& v1, const Variant& v2,
                                 const void* /*data*/) {
  double d1 = v1.toDouble();
  double d2 = v2.toDouble();
  if (d1 < d2) return 1;
  if (d1 == d2) return 0;
  return -1;
}

int Array::SortStringAscending(const Variant& v1, const Variant& v2,
                               const void* /*data*/) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_strcmp(s1.data(), s1.size(), s2.data(), s2.size());
}

int Array::SortStringAscendingCase(const Variant& v1, const Variant& v2,
                                   const void* /*data*/) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return bstrcasecmp(s1.data(), s1.size(), s2.data(), s2.size());
}

int Array::SortStringDescending(const Variant& v1, const Variant& v2,
                                const void* /*data*/) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_strcmp(s2.data(), s2.size(), s1.data(), s1.size());
}

int Array::SortStringDescendingCase(const Variant& v1, const Variant& v2,
                                    const void* /*data*/) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return bstrcasecmp(s2.data(), s2.size(), s1.data(), s1.size());
}

int Array::SortLocaleStringAscending(const Variant& v1, const Variant& v2,
                                     const void* /*data*/) {
  String s1 = v1.toString();
  String s2 = v2.toString();

  return strcoll(s1.data(), s2.data());
}

int Array::SortLocaleStringDescending(const Variant& v1, const Variant& v2,
                                      const void* /*data*/) {
  String s1 = v1.toString();
  String s2 = v2.toString();

  return strcoll(s2.data(), s1.data());
}

int Array::SortNaturalAscending(const Variant& v1, const Variant& v2,
                                const void* /*data*/) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s1.data(), s1.size(), s2.data(), s2.size(), 0);
}

int Array::SortNaturalDescending(const Variant& v1, const Variant& v2,
                                 const void* /*data*/) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s2.data(), s2.size(), s1.data(), s1.size(), 0);
}

int Array::SortNaturalCaseAscending(const Variant& v1, const Variant& v2,
                                    const void* /*data*/) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s1.data(), s1.size(), s2.data(), s2.size(), 1);
}

int Array::SortNaturalCaseDescending(const Variant& v1, const Variant& v2,
                                     const void* /*data*/) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s2.data(), s2.size(), s1.data(), s1.size(), 1);
}

///////////////////////////////////////////////////////////////////////////////
}
