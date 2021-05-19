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
#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/request-info.h"
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

void Array::setEvalScalar() const {
  Array* thisPtr = const_cast<Array*>(this);
  if (!m_arr) thisPtr->m_arr = Ptr::attach(ArrayData::CreateDict());
  if (!m_arr->isStatic()) {
    thisPtr->m_arr = ArrayData::GetScalarArray(std::move(*thisPtr));
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

///////////////////////////////////////////////////////////////////////////////

ArrayIter Array::begin(const String& /*context*/ /* = null_string */) const {
  return ArrayIter(*this);
}

///////////////////////////////////////////////////////////////////////////////
// PHP operations.

NEVER_INLINE
static void throw_bad_array_merge() {
  throw ExtendedException("Invalid operand type was used: "
                          "merging an array with NULL or non-array.");
}

Array Array::diff(const Variant& array, bool by_key, bool by_value,
                  PFUNC_CMP key_cmp_function /* = NULL */,
                  const void *key_data /* = NULL */,
                  PFUNC_CMP value_cmp_function /* = NULL */,
                  const void *value_data /* = NULL */) const {
  if (!array.isArray()) {
    raise_expected_array_warning();
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
    raise_expected_array_warning();
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
  assertx(by_key || by_value);
  assertx(by_key || key_cmp_function == nullptr);
  assertx(by_value || value_cmp_function == nullptr);
  PFUNC_CMP value_cmp_as_string_function = value_cmp_function;
  if (!value_cmp_function) {
    value_cmp_function = SortStringAscending;
    value_cmp_as_string_function = CompareAsStrings;
  }

  Array ret = Array::CreateDict();
  if (by_key && !key_cmp_function) {
    // Fast case
    for (ArrayIter iter(*this); iter; ++iter) {
      Variant key(iter.first());
      auto const value = iter.secondVal();
      bool found = false;
      if (array->exists(array.convertKey<IntishCast::Cast>(key))) {
        if (by_value) {
          found = value_cmp_as_string_function(
            VarNR(value),
            VarNR(array.lookup(key, AccessFlags::Key)),
            value_data
          ) == 0;
        } else {
          found = true;
        }
      }
      if (found == match) {
        // this set never intish casted, even when *this or array is a
        // hack array
        ret.set(key, value, true);
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
                                : VarNR(array->nvGetVal(pos)),
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
                VarNR(array->nvGetVal(pos)),
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
                  VarNR(array->nvGetVal(pos)),
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
      // This never intish casted
      ret.set(iter.first(), iter.secondVal(), true);
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Type conversions.

String Array::toString() const {
  if (m_arr == nullptr) return empty_string();
  if (m_arr->isVecType()) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Vec to string conversion"
    );
  }
  if (m_arr->isDictType()) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Dict to string conversion"
    );
  }
  assertx(m_arr->isKeysetType());
  SystemLib::throwInvalidOperationExceptionObject(
    "Keyset to string conversion"
  );
}

////////////////////////////////////////////////////////////////////////////////

void Array::setLegacyArray(bool isLegacy) {
  auto const ad = get();
  auto const result = ad->setLegacyArray(ad->cowCheck(), isLegacy);
  if (result != ad) m_arr = Ptr::attach(result);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> ALWAYS_INLINE
TypedValue Array::lookupImpl(const T& key, AccessFlags flags) const {
  return m_arr ? m_arr->get(key, any(flags & AccessFlags::Error))
               : make_tv<KindOfUninit>();
}

template<typename T> ALWAYS_INLINE
tv_lval Array::lvalImpl(const T& key, AccessFlags) {
  if (!m_arr) m_arr = Ptr::attach(ArrayData::CreateDict());
  auto const lval = m_arr->lval(key);
  if (lval.arr != m_arr) m_arr = Ptr::attach(lval.arr);
  assertx(lval);
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
    ArrayData* escalated = m_arr->remove(key);
    if (escalated != m_arr) m_arr = Ptr::attach(escalated);
  }
}

template<typename T> ALWAYS_INLINE
void Array::setImpl(const T& key, TypedValue v) {
  if (!m_arr) {
    // NOTE: DictInit doesn't support set(TypedValue key, TypedValue val) yet.
    ArrayInit init(1, ArrayInit::Map{});
    init.set(key, v);
    m_arr = Ptr::attach(init.toArray().detach());
  } else {
    m_arr.mutateInPlace([&](ArrayData* ad) {
      return ad->setMove(key, tvToInit(v));
    });
    tvIncRefGen(v);
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

template<> TypedValue not_found<TypedValue>() {
  return make_tv<KindOfUninit>();
}
template<> tv_lval not_found<tv_lval>() {
  return lvalBlackHole().asTypedValue();
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

  auto const ad = arr.get() ? arr.get() : ArrayData::CreateDict();

  // The logic here is a specialization of tvToKey().
  if (key.isNull()) {
    throwInvalidArrayKeyException(&immutable_uninit_base, ad);
  }

  return fn(key, std::forward<Args>(args)...);
}

template<typename Fn, typename... Args> ALWAYS_INLINE
decltype(auto) elem(const Array& arr, Fn fn, bool is_key,
                    TypedValue key, Args&&... args) {
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
  return elem(arr, fn, is_key, *key.asTypedValue(), std::forward<Args>(args)...);
}

}

#define WRAP(name)                                              \
  [&] (auto&&... ts) -> decltype(auto) {                        \
    return this->name##Impl(std::forward<decltype(ts)>(ts)...); \
  }

#define FOR_EACH_KEY_TYPE(...)    \
  C(TypedValue, __VA_ARGS__)            \
  I(int, __VA_ARGS__)             \
  I(int64_t, __VA_ARGS__)         \
  V(const String&, __VA_ARGS__)   \
  V(const Variant&, __VA_ARGS__)

#define FK(flags) any(flags & AccessFlags::Key)

#define C(key_t, name, ret_t, cns)                          \
  ret_t Array::name(key_t k, AccessFlags fl) cns {          \
    return detail::elem(*this, WRAP(name), FK(fl), k, fl);  \
  }
#define V C
#define I(key_t, name, ret_t, cns)                  \
  ret_t Array::name(key_t k, AccessFlags fl) cns {  \
    return name##Impl(int64_t(k), fl);              \
  }

FOR_EACH_KEY_TYPE(lookup, TypedValue, const)
FOR_EACH_KEY_TYPE(lval, tv_lval, )

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

#undef I
#undef V
#undef C

#undef FOR_EACH_KEY_TYPE
#undef WRAP

///////////////////////////////////////////////////////////////////////////////

void Array::append(TypedValue v) {
  if (!m_arr) operator=(CreateDict());
  assertx(m_arr);
  m_arr.mutateInPlace([&](ArrayData* ad) {
    return ad->appendMove(tvToInit(v));
  });
  tvIncRefGen(v);
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
    VarNR((*opaque->array)->nvGetVal(pos1)),
    VarNR((*opaque->array)->nvGetVal(pos2)),
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
        VarNR((*opaque->array)->nvGetVal(pos1)),
        VarNR((*opaque->array)->nvGetVal(pos2)),
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
  assertx(cmp_func);

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
  Array sorted = Array::CreateDict();
  if (m_arr && m_arr->isLegacyArray()) sorted.setLegacyArray(true);

  SortData opaque;
  std::vector<int> indices;
  SortImpl(indices, *this, opaque, cmp_func, by_key, data);
  int count = size();
  for (int i = 0; i < count; i++) {
    ssize_t pos = opaque.positions[indices[i]];
    if (renumber) {
      sorted.append(m_arr->nvGetVal(pos));
    } else {
      sorted.set(m_arr->nvGetKey(pos), m_arr->nvGetVal(pos), true);
    }
  }
  operator=(sorted);
}

bool Array::MultiSort(std::vector<SortData> &data) {
  assertx(!data.empty());

  int count = -1;
  for (unsigned int k = 0; k < data.size(); k++) {
    SortData &opaque = data[k];

    assertx(opaque.array);
    assertx(opaque.cmp_func);
    int size = opaque.array->size();
    if (count == -1) {
      count = size;
    } else if (count != size) {
      raise_invalid_argument_warning("arrays: (inconsistent sizes)");
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
  if (count < 0) not_reached();

  int *indices = (int *)malloc(sizeof(int) * count);
  for (int i = 0; i < count; i++) {
    indices[i] = i;
  }

  zend_qsort(indices, count, sizeof(int), multi_compare_func, (void *)&data);

  for (unsigned int ki = 0; ki < data.size(); ki++) {
    SortData &opaque = data[ki];
    const Array& arr = *opaque.array;
    Array sorted = Array::CreateDict();
    if (arr->isLegacyArray()) sorted.setLegacyArray(true);
    int64_t nextKI = 0;
    for (int i = 0; i < count; i++) {
      ssize_t pos = opaque.positions[indices[i]];
      Variant k(arr->getKey(pos));
      if (k.isInteger()) {
        sorted.set(nextKI++, arr->nvGetVal(pos));
      } else {
        sorted.set(k, arr->nvGetVal(pos), true);
      }
    }
    *opaque.original = sorted;
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
