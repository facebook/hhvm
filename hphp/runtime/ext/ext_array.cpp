/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include "hphp/runtime/ext/ext_array.h"

#include <vector>

#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/zend-collator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/util/logger.h"

#define SORT_DESC               3
#define SORT_ASC                4
namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_count("count");

const int64_t k_UCOL_DEFAULT = UCOL_DEFAULT;

const int64_t k_UCOL_PRIMARY = UCOL_PRIMARY;
const int64_t k_UCOL_SECONDARY = UCOL_SECONDARY;
const int64_t k_UCOL_TERTIARY = UCOL_TERTIARY;
const int64_t k_UCOL_DEFAULT_STRENGTH = UCOL_DEFAULT_STRENGTH;
const int64_t k_UCOL_QUATERNARY = UCOL_QUATERNARY;
const int64_t k_UCOL_IDENTICAL = UCOL_IDENTICAL;

const int64_t k_UCOL_OFF = UCOL_OFF;
const int64_t k_UCOL_ON = UCOL_ON;

const int64_t k_UCOL_SHIFTED = UCOL_SHIFTED;
const int64_t k_UCOL_NON_IGNORABLE = UCOL_NON_IGNORABLE;

const int64_t k_UCOL_LOWER_FIRST = UCOL_LOWER_FIRST;
const int64_t k_UCOL_UPPER_FIRST = UCOL_UPPER_FIRST;

const int64_t k_UCOL_FRENCH_COLLATION = UCOL_FRENCH_COLLATION;
const int64_t k_UCOL_ALTERNATE_HANDLING = UCOL_ALTERNATE_HANDLING;
const int64_t k_UCOL_CASE_FIRST = UCOL_CASE_FIRST;
const int64_t k_UCOL_CASE_LEVEL = UCOL_CASE_LEVEL;
const int64_t k_UCOL_NORMALIZATION_MODE = UCOL_NORMALIZATION_MODE;
const int64_t k_UCOL_STRENGTH = UCOL_STRENGTH;
const int64_t k_UCOL_HIRAGANA_QUATERNARY_MODE = UCOL_HIRAGANA_QUATERNARY_MODE;
const int64_t k_UCOL_NUMERIC_COLLATION = UCOL_NUMERIC_COLLATION;

using HPHP::JIT::CallerFrame;
using HPHP::JIT::EagerCallerFrame;

#define getCheckedArrayRet(input, fail)                           \
  auto const cell_##input = static_cast<const Variant&>(input).asCell(); \
  if (UNLIKELY(cell_##input->m_type != KindOfArray)) {            \
    throw_expected_array_exception();                             \
    return fail;                                                  \
  }                                                               \
  ArrNR arrNR_##input(cell_##input->m_data.parr);                 \
  const Array& arr_##input = arrNR_##input.asArray();

#define getCheckedArray(input) getCheckedArrayRet(input, uninit_null())

Variant f_array_change_key_case(const Variant& input, int64_t case_ /* = 0 */) {
  getCheckedArrayRet(input, false);
  return ArrayUtil::ChangeKeyCase(arr_input, !case_);
}

Variant f_array_chunk(const Variant& input, int chunkSize,
                      bool preserve_keys /* = false */) {

  const auto& cellInput = *input.asCell();
  if (UNLIKELY(!isContainer(cellInput))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection as argument 1", __FUNCTION__+2);
    return init_null();
  }

  if (chunkSize < 1) {
    throw_invalid_argument("size: %d", chunkSize);
    return init_null();
  }

  Array ret = Array::Create();
  Array chunk;
  int current = 0;
  for (ArrayIter iter(cellInput); iter; ++iter) {
    if (preserve_keys) {
      chunk.setWithRef(iter.first(), iter.secondRefPlus(), true);
    } else {
      chunk.appendWithRef(iter.secondRefPlus());
    }
    if ((++current % chunkSize) == 0) {
      ret.append(chunk);
      chunk.clear();
    }
  }
  if (!chunk.empty()) {
    ret.append(chunk);
  }

  return ret;
}

static inline bool array_column_coerce_key(Variant &key, const char *name) {
  /* NULL has a special meaning for each field */
  if (key.isNull()) {
    return true;
  }

  /* Custom coercion rules for key types */
  if (key.isInteger() || key.isDouble()) {
    key = key.toInt64();
    return true;
  } else if (key.isString() || key.isObject()) {
    key = key.toString();
    return true;
  } else {
    raise_warning("The %s key should be either a string or an integer", name);
    return false;
  }
}

Variant f_array_column(const Variant& input, const Variant& val_key,
                       const Variant& idx_key /* = null_variant */) {
  /* Be strict about array type */
  getCheckedArrayRet(input, uninit_null());
  Variant val = val_key, idx = idx_key;
  if (!array_column_coerce_key(val, "column") ||
      !array_column_coerce_key(idx, "index")) {
    return false;
  }
  Array ret = Array::Create();
  for(auto it = arr_input.begin(); !it.end(); it.next()) {
    if (!it.second().isArray()) {
      continue;
    }
    Array sub = it.second().toArray();

    Variant elem;
    if (val.isNull()) {
      elem = sub;
    } else if (sub.exists(val)) {
      elem = sub[val];
    } else {
      // skip subarray without named element
      continue;
    }

    if (idx.isNull() || !sub.exists(idx)) {
      ret.append(elem);
    } else if (sub[idx].isObject()) {
      ret.set(sub[idx].toString(), elem);
    } else {
      ret.set(sub[idx], elem);
    }
  }
  return ret;
}

Variant f_array_combine(const Variant& keys, const Variant& values) {
  const auto& cell_keys = *keys.asCell();
  const auto& cell_values = *values.asCell();
  if (UNLIKELY(!isContainer(cell_keys) || !isContainer(cell_values))) {
    raise_warning("Invalid operand type was used: array_combine expects "
                  "arrays or collections");
    return uninit_null();
  }
  if (UNLIKELY(getContainerSize(cell_keys) != getContainerSize(cell_values))) {
    raise_warning("array_combine(): Both parameters should have an equal "
                  "number of elements");
    return false;
  }
  Array ret = ArrayData::Create();
  for (ArrayIter iter1(cell_keys), iter2(cell_values);
       iter1; ++iter1, ++iter2) {
    const Variant& key = iter1.secondRefPlus();
    if (key.isInteger() || key.isString()) {
      ret.setWithRef(key, iter2.secondRefPlus());
    } else {
      ret.setWithRef(key.toString(), iter2.secondRefPlus());
    }
  }
  return ret;
}

Variant f_array_count_values(const Variant& input) {
  getCheckedArray(input);
  return ArrayUtil::CountValues(arr_input);
}

Variant f_array_fill_keys(const Variant& keys, const Variant& value) {
  const auto& cell_keys = *keys.asCell();
  if (UNLIKELY(!isContainer(cell_keys))) {
    raise_warning("Invalid operand type was used: array_fill_keys expects "
                  "an array or collection");
    return uninit_null();
  }

  auto size = getContainerSize(cell_keys);
  if (!size) return empty_array;

  ArrayInit ai(size);
  for (ArrayIter iter(cell_keys); iter; ++iter) {
    auto& key = iter.secondRefPlus();
    // This is intentionally different to the $foo[$invalid_key] coercion.
    // See tests/slow/ext_array/array_fill_keys_tostring.php for examples.
    if (LIKELY(key.isInteger() || key.isString())) {
      ai.set(key, value);
    } else if (RuntimeOption::EnableHipHopSyntax) {
      // @todo (fredemmott): Use the Zend toString() behavior, but retain the
      // warning/error behind a separate config setting
      raise_warning("array_fill_keys: keys must be ints or strings");
      ai.set(key, value);
    } else {
      ai.set(key.toString(), value);
    }
  }
  return ai.create();
}

Variant f_array_fill(int start_index, int num, const Variant& value) {
  if (num <= 0) {
    throw_invalid_argument("num: [non-positive]");
    return false;
  }

  Array ret;
  ret.set(start_index, value);
  for (int i = num - 1; i > 0; i--) {
    ret.append(value);
  }
  return ret;
}

Variant f_array_flip(const Variant& trans) {

  auto const& transCell = *trans.asCell();
  if (UNLIKELY(!isContainer(transCell))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection", __FUNCTION__+2);
    return uninit_null();
  }

  ArrayInit ret(getContainerSize(transCell));
  for (ArrayIter iter(transCell); iter; ++iter) {
    const Variant& value(iter.secondRefPlus());
    if (value.isString() || value.isInteger()) {
      ret.set(value, iter.first());
    } else {
      raise_warning("Can only flip STRING and INTEGER values!");
    }
  }
  return ret.toVariant();
}

bool f_array_key_exists(const Variant& key, const Variant& search) {
  const ArrayData *ad;

  auto const searchCell = search.asCell();
  if (LIKELY(searchCell->m_type == KindOfArray)) {
    ad = searchCell->m_data.parr;
  } else if (searchCell->m_type == KindOfObject) {
    ObjectData* obj = searchCell->m_data.pobj;
    if (obj->isCollection()) {
      return collectionContains(obj, key);
    }
    return f_array_key_exists(key, toArray(search));
  } else {
    throw_bad_type_exception("array_key_exists expects an array or an object; "
                             "false returned.");
    return false;
  }

  auto const cell = key.asCell();
  switch (cell->m_type) {
  case KindOfString:
  case KindOfStaticString: {
    int64_t n = 0;
    StringData *sd = cell->m_data.pstr;
    if (sd->isStrictlyInteger(n)) {
      return ad->exists(n);
    }
    return ad->exists(StrNR(sd));
  }
  case KindOfInt64:
    return ad->exists(cell->m_data.num);
  case KindOfUninit:
  case KindOfNull:
    return ad->exists(empty_string);
  default:
    break;
  }
  raise_warning("Array key should be either a string or an integer");
  return false;
}

bool f_key_exists(const Variant& key, const Variant& search) {
  return f_array_key_exists(key, search);
}

Variant f_array_keys(const Variant& input, const Variant& search_value /* = null_variant */,
                     bool strict /* = false */) {
  const auto& cell_input = *input.asCell();
  if (UNLIKELY(!isContainer(cell_input))) {
    raise_warning("array_keys() expects parameter 1 to be an array "
                  "or collection");
    return uninit_null();
  }

  if (LIKELY(!search_value.isInitialized())) {
    PackedArrayInit ai(getContainerSize(cell_input));
    for (ArrayIter iter(cell_input); iter; ++iter) {
      ai.append(iter.first());
    }
    return ai.toArray();
  } else {
    Array ai = Array::attach(HphpArray::MakeReserve(0));
    for (ArrayIter iter(cell_input); iter; ++iter) {
      if ((strict && HPHP::same(iter.secondRefPlus(), search_value)) ||
          (!strict && HPHP::equal(iter.secondRefPlus(), search_value))) {
        ai.append(iter.first());
      }
    }
    return ai;
  }
}

Variant f_array_map(int _argc, const Variant& callback, const Variant& arr1, const Array& _argv /* = null_array */) {
  CallCtx ctx;
  ctx.func = NULL;
  if (!callback.isNull()) {
    EagerCallerFrame cf;
    vm_decode_function(callback, cf(), false, ctx);
  }
  const auto& cell_arr1 = *arr1.asCell();
  if (UNLIKELY(!isContainer(cell_arr1))) {
    raise_warning("array_map(): Argument #2 should be an array or collection");
    return uninit_null();
  }
  if (LIKELY(_argv.empty())) {
    // Handle the common case where the caller passed two
    // params (a callback and a container)
    if (!ctx.func) {
      if (cell_arr1.m_type == KindOfArray) {
        return arr1;
      } else {
        return arr1.toArray();
      }
    }
    Array ret = Array::Create();
    for (ArrayIter iter(arr1); iter; ++iter) {
      Variant result;
      g_context->invokeFuncFew((TypedValue*)&result, ctx, 1,
                                 iter.secondRefPlus().asCell());
      ret.add(iter.first(), result, true);
    }
    return ret;
  }

  // Handle the uncommon case where the caller passed a callback
  // and two or more containers
  ArrayIter* iters =
    (ArrayIter*)smart_malloc(sizeof(ArrayIter) * (_argv.size() + 1));
  size_t numIters = 0;
  SCOPE_EXIT {
    while (numIters--) iters[numIters].~ArrayIter();
    smart_free(iters);
  };
  size_t maxLen = getContainerSize(cell_arr1);
  (void) new (&iters[numIters]) ArrayIter(cell_arr1);
  ++numIters;
  for (ArrayIter it(_argv); it; ++it, ++numIters) {
    const auto& c = *it.secondRefPlus().asCell();
    if (UNLIKELY(!isContainer(c))) {
      raise_warning("array_map(): Argument #%d should be an array or "
                    "collection", (int)(numIters + 2));
      (void) new (&iters[numIters]) ArrayIter(it.secondRefPlus().toArray());
    } else {
      (void) new (&iters[numIters]) ArrayIter(c);
      size_t len = getContainerSize(c);
      if (len > maxLen) maxLen = len;
    }
  }
  Array ret = Array::Create();
  for (size_t k = 0; k < maxLen; k++) {
    Array params;
    for (size_t i = 0; i < numIters; ++i) {
      if (iters[i]) {
        params.append(iters[i].secondRefPlus());
        ++iters[i];
      } else {
        params.append(init_null_variant);
      }
    }
    if (ctx.func) {
      Variant result;
      g_context->invokeFunc((TypedValue*)&result,
                              ctx.func, params, ctx.this_,
                              ctx.cls, nullptr, ctx.invName);
      ret.append(result);
    } else {
      ret.append(params);
    }
  }
  return ret;
}

static void php_array_merge(Array &arr1, const Array& arr2) {
  arr1.merge(arr2);
}

static void php_array_merge_recursive(PointerSet &seen, bool check,
                                      Array &arr1, const Array& arr2) {
  if (check) {
    if (seen.find((void*)arr1.get()) != seen.end()) {
      raise_warning("array_merge_recursive(): recursion detected");
      return;
    }
    seen.insert((void*)arr1.get());
  }

  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key(iter.first());
    const Variant& value(iter.secondRef());
    if (key.isNumeric()) {
      arr1.appendWithRef(value);
    } else if (arr1.exists(key, true)) {
      // There is no need to do toKey() conversion, for a key that is already
      // in the array.
      Variant &v = arr1.lvalAt(key, AccessFlags::Key);
      Array subarr1(v.toArray()->copy());
      php_array_merge_recursive(seen, v.isReferenced(), subarr1,
                                value.toArray());
      v.unset(); // avoid contamination of the value that was strongly bound
      v = subarr1;
    } else {
      arr1.setWithRef(key, value, true);
    }
  }

  if (check) {
    seen.erase((void*)arr1.get());
  }
}

Variant f_array_merge(int _argc, const Variant& array1,
                      const Array& _argv /* = null_array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  php_array_merge(ret, arr_array1);
  for (ArrayIter iter(_argv); iter; ++iter) {
    Variant v = iter.second();
    if (!v.isArray()) {
      throw_expected_array_exception();
      return uninit_null();
    }
    const Array& arr_v = v.asCArrRef();
    php_array_merge(ret, arr_v);
  }
  return ret;
}

Variant f_array_merge_recursive(int _argc, const Variant& array1,
                                const Array& _argv /* = null_array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  PointerSet seen;
  php_array_merge_recursive(seen, false, ret, arr_array1);
  assert(seen.empty());
  for (ArrayIter iter(_argv); iter; ++iter) {
    Variant v = iter.second();
    if (!v.isArray()) {
      throw_expected_array_exception();
      return uninit_null();
    }
    const Array& arr_v = v.asCArrRef();
    php_array_merge_recursive(seen, false, ret, arr_v);
    assert(seen.empty());
  }
  return ret;
}

static void php_array_replace(Array &arr1, const Array& arr2) {
  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key = iter.first();
    const Variant& value = iter.secondRef();
    arr1.setWithRef(key, value, true);
  }
}

static void php_array_replace_recursive(PointerSet &seen, bool check,
                                        Array &arr1, const Array& arr2) {
  if (check) {
    if (seen.find((void*)arr1.get()) != seen.end()) {
      raise_warning("array_replace_recursive(): recursion detected");
      return;
    }
    seen.insert((void*)arr1.get());
  }

  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key = iter.first();
    const Variant& value = iter.secondRef();
    if (arr1.exists(key, true) && value.isArray()) {
      Variant &v = arr1.lvalAt(key, AccessFlags::Key);
      if (v.isArray()) {
        Array subarr1 = v.toArray();
        const ArrNR& arr_value = value.toArrNR();
        php_array_replace_recursive(seen, v.isReferenced(), subarr1,
                                    arr_value);
        v = subarr1;
      } else {
        arr1.set(key, value, true);
      }
    } else {
      arr1.setWithRef(key, value, true);
    }
  }

  if (check) {
    seen.erase((void*)arr1.get());
  }
}

Variant f_array_replace(int _argc, const Variant& array1,
                        const Array& _argv /* = null_array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  php_array_replace(ret, arr_array1);
  for (ArrayIter iter(_argv); iter; ++iter) {
    const Variant& v = iter.secondRef();
    getCheckedArray(v);
    php_array_replace(ret, arr_v);
  }
  return ret;
}

Variant f_array_replace_recursive(int _argc, const Variant& array1,
                                  const Array& _argv /* = null_array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  PointerSet seen;
  php_array_replace_recursive(seen, false, ret, arr_array1);
  assert(seen.empty());
  for (ArrayIter iter(_argv); iter; ++iter) {
    const Variant& v = iter.secondRef();
    getCheckedArray(v);
    php_array_replace_recursive(seen, false, ret, arr_v);
    assert(seen.empty());
  }
  return ret;
}

Variant f_array_pad(const Variant& input, int pad_size, const Variant& pad_value) {
  getCheckedArray(input);
  if (pad_size > 0) {
    return ArrayUtil::Pad(arr_input, pad_value, pad_size, true);
  }
  return ArrayUtil::Pad(arr_input, pad_value, -pad_size, false);
}

Variant f_array_pop(VRefParam containerRef) {
  const auto* container = containerRef->asCell();
  if (UNLIKELY(!isMutableContainer(*container))) {
    raise_warning(
      "%s() expects parameter 1 to be an array or mutable collection",
      __FUNCTION__+2 /* remove the "f_" prefix */);
    return uninit_null();
  }
  if (!getContainerSize(containerRef)) {
    return uninit_null();
  }
  if (container->m_type == KindOfArray) {
    return containerRef.wrapped().toArrRef().pop();
  }
  assert(container->m_type == KindOfObject);
  auto* obj = container->m_data.pobj;
  assert(obj->isCollection());
  switch (obj->getCollectionType()) {
    case Collection::VectorType: return static_cast<c_Vector*>(obj)->t_pop();
    case Collection::MapType:    return static_cast<c_Map*>(obj)->pop();
    case Collection::SetType:    return static_cast<c_Set*>(obj)->pop();
    default:                     break;
  }
  assert(false);
  return uninit_null();
}

Variant f_array_product(const Variant& array) {
  getCheckedArray(array);
  int64_t i;
  double d;
  if (ArrayUtil::Product(arr_array, &i, &d) == KindOfInt64) {
    return i;
  } else {
    return d;
  }
}

Variant f_array_push(int _argc, VRefParam container,
                     const Variant& var, const Array& _argv /* = null_array */) {

  if (LIKELY(container->isArray())) {
    auto const array_cell = container.wrapped().asCell();
    assert(array_cell->m_type == KindOfArray);

    /*
     * Important note: this *must* cast the parr in the inner cell to
     * the Array&---we can't copy it to the stack or anything because we
     * might escalate.
     */
    Array& arr_array = *reinterpret_cast<Array*>(&array_cell->m_data.parr);
    arr_array.append(var);
    for (ArrayIter iter(_argv); iter; ++iter) {
      arr_array.append(iter.second());
    }
    return arr_array.size();
  }

  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    auto collection_type = obj->getCollectionType();
    if (collection_type == Collection::VectorType) {
      c_Vector* vec = static_cast<c_Vector*>(obj);
      vec->reserve(vec->size() + _argc + 1);
      vec->t_add(var);
      for (ArrayIter iter(_argv); iter; ++iter) {
        vec->t_add(iter.second());
      }
      return vec->size();
    } else if (collection_type == Collection::SetType) {
      c_Set* set = static_cast<c_Set*>(obj);
      set->reserve(set->size() + _argc + 1);
      set->t_add(var);
      for (ArrayIter iter(_argv); iter; ++iter) {
        set->t_add(iter.second());
      }
      return set->size();
    }
    // other collection types are unsupported:
    //  - mapping collections require a key
    //  - frozen collections don't allow insertion
  }

  throw_expected_array_or_collection_exception();
  return uninit_null();
}

Variant f_array_rand(const Variant& input, int num_req /* = 1 */) {
  getCheckedArray(input);
  return ArrayUtil::RandomKeys(arr_input, num_req);
}

static Variant reduce_func(const Variant& result, const Variant& operand, const void *data) {
  CallCtx* ctx = (CallCtx*)data;
  Variant ret;
  TypedValue args[2] = { *result.asCell(), *operand.asCell() };
  g_context->invokeFuncFew(ret.asTypedValue(), *ctx, 2, args);
  return ret;
}
Variant f_array_reduce(const Variant& input, const Variant& callback,
                       const Variant& initial /* = null_variant */) {
  getCheckedArray(input);
  CallCtx ctx;
  CallerFrame cf;
  vm_decode_function(callback, cf(), false, ctx);
  if (ctx.func == NULL) {
    return uninit_null();
  }
  return ArrayUtil::Reduce(arr_input, reduce_func, &ctx, initial);
}

Variant f_array_reverse(const Variant& input, bool preserve_keys /* = false */) {

  const auto& cell_input = *input.asCell();
  if (UNLIKELY(!isContainer(cell_input))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection as argument 1",
                  __FUNCTION__+2);
    return uninit_null();
  }

  if (LIKELY(cell_input.m_type == KindOfArray)) {
    ArrNR arrNR(cell_input.m_data.parr);
    const Array& arr = arrNR.asArray();
    return ArrayUtil::Reverse(arr, preserve_keys);
  }

  // For collections, we convert to their array representation and then
  // reverse it, rather than building a reversed version of ArrayIter
  assert(cell_input.m_type == KindOfObject);
  ObjectData* obj = cell_input.m_data.pobj;
  assert(obj && obj->isCollection());
  return ArrayUtil::Reverse(obj->o_toArray(), preserve_keys);
}

Variant f_array_shift(VRefParam array) {
  const auto* cell_array = array->asCell();
  if (UNLIKELY(!isContainer(*cell_array))) {
    raise_warning(
      "%s() expects parameter 1 to be an array or mutable collection",
      __FUNCTION__+2 /* remove the "f_" prefix */);
    return uninit_null();
  }
  if (cell_array->m_type == KindOfArray) {
    return array.wrapped().toArrRef().dequeue();
  }
  assert(cell_array->m_type == KindOfObject);
  auto* obj = cell_array->m_data.pobj;
  assert(obj->isCollection());
  switch (obj->getCollectionType()) {
    case Collection::VectorType: {
      auto* vec = static_cast<c_Vector*>(obj);
      if (!vec->size()) return uninit_null();
      return vec->popFront();
    }
    case Collection::MapType: {
      auto* mp = static_cast<BaseMap*>(obj);
      if (!mp->size()) return uninit_null();
      return mp->popFront();
    }
    case Collection::SetType: {
      auto* st = static_cast<c_Set*>(obj);
      if (!st->size()) return uninit_null();
      return st->popFront();
    }
    default: {
      raise_warning(
        "%s() expects parameter 1 to be an array or mutable collection",
        __FUNCTION__+2 /* remove the "f_" prefix */);
      return uninit_null();
    }
  }
}

Variant f_array_slice(const Variant& input, int offset,
                      const Variant& length /* = null_variant */,
                      bool preserve_keys /* = false */) {
  const auto& cell_input = *input.asCell();
  if (UNLIKELY(!isContainer(cell_input))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection as argument 1",
                  __FUNCTION__+2);
    return uninit_null();
  }
  int64_t len = length.isNull() ? 0x7FFFFFFF : length.toInt64();

  int num_in = getContainerSize(cell_input);
  if (offset > num_in) {
    offset = num_in;
  } else if (offset < 0 && (offset = (num_in + offset)) < 0) {
    offset = 0;
  }

  if (len < 0) {
    len = num_in - offset + len;
  } else if (((unsigned)offset + (unsigned)len) > (unsigned)num_in) {
    len = num_in - offset;
  }

  if (len <= 0) {
    return empty_array;
  }

  // PackedArrayInit can't be used because non-numeric keys are preserved
  // even when preserve_keys is false
  Array ret = Array::attach(HphpArray::MakeReserve(len));
  int pos = 0;
  ArrayIter iter(input);
  for (; pos < offset && iter; ++pos, ++iter) {}
  for (; pos < (offset + len) && iter; ++pos, ++iter) {
    Variant key(iter.first());
    bool doAppend = !preserve_keys && key.isNumeric();
    const Variant& v = iter.secondRefPlus();
    if (doAppend) {
      ret.appendWithRef(v);
    } else {
      ret.setWithRef(key, v, true);
    }
  }
  return ret;
}

Variant f_array_splice(VRefParam input, int offset,
                       const Variant& length /* = null_variant */,
                       const Variant& replacement /* = null_variant */) {
  getCheckedArray(input);
  Array ret(Array::Create());
  int64_t len = length.isNull() ? 0x7FFFFFFF : length.toInt64();
  input = ArrayUtil::Splice(arr_input, offset, len, replacement, &ret);
  return ret;
}

Variant f_array_sum(const Variant& array) {
  getCheckedArray(array);
  int64_t i;
  double d;
  if (ArrayUtil::Sum(arr_array, &i, &d) == KindOfInt64) {
    return i;
  } else {
    return d;
  }
}

Variant f_array_unshift(int _argc, VRefParam array, const Variant& var, const Array& _argv /* = null_array */) {
  const auto* cell_array = array->asCell();
  if (UNLIKELY(!isContainer(*cell_array))) {
    raise_warning("%s() expects parameter 1 to be an array, Vector, or Set",
                  __FUNCTION__+2 /* remove the "f_" prefix */);
    return uninit_null();
  }
  if (cell_array->m_type == KindOfArray) {
    if (array.toArray()->isVectorData()) {
      if (!_argv.empty()) {
        for (ssize_t pos = _argv->iter_end(); pos != ArrayData::invalid_index;
          pos = _argv->iter_rewind(pos)) {
          array.wrapped().toArrRef().prepend(_argv->getValueRef(pos));
        }
      }
      array.wrapped().toArrRef().prepend(var);
    } else {
      {
        Array newArray;
        newArray.append(var);
        if (!_argv.empty()) {
          for (ssize_t pos = _argv->iter_begin();
               pos != ArrayData::invalid_index;
               pos = _argv->iter_advance(pos)) {
            newArray.append(_argv->getValueRef(pos));
          }
        }
        for (ArrayIter iter(array.toArray()); iter; ++iter) {
          Variant key(iter.first());
          const Variant& value(iter.secondRef());
          if (key.isInteger()) {
            newArray.appendWithRef(value);
          } else {
            newArray.setWithRef(key, value, true);
          }
        }
        array = newArray;
      }
      // Reset the array's internal pointer
      if (array.is(KindOfArray)) {
        f_reset(array);
      }
    }
    return array.toArray().size();
  }
  // Handle collections
  assert(cell_array->m_type == KindOfObject);
  auto* obj = cell_array->m_data.pobj;
  assert(obj->isCollection());
  switch (obj->getCollectionType()) {
    case Collection::VectorType: {
      auto* vec = static_cast<c_Vector*>(obj);
      if (!_argv.empty()) {
        for (ssize_t pos = _argv->iter_end(); pos != ArrayData::invalid_index;
             pos = _argv->iter_rewind(pos)) {
          vec->addFront(cvarToCell(&_argv->getValueRef(pos)));
        }
      }
      vec->addFront(cvarToCell(&var));
      return vec->size();
    }
    case Collection::SetType: {
      auto* st = static_cast<c_Set*>(obj);
      if (!_argv.empty()) {
        for (ssize_t pos = _argv->iter_end(); pos != ArrayData::invalid_index;
             pos = _argv->iter_rewind(pos)) {
          st->addFront(cvarToCell(&_argv->getValueRef(pos)));
        }
      }
      st->addFront(cvarToCell(&var));
      return st->size();
    }
    default: {
      raise_warning("%s() expects parameter 1 to be an array, Vector, or Set",
                    __FUNCTION__+2 /* remove the "f_" prefix */);
      return uninit_null();
    }
  }
}

Variant f_array_values(const Variant& input) {
  const auto& cell_input = *input.asCell();
  if (!isContainer(cell_input)) {
    raise_warning("array_values() expects parameter 1 to be an array "
                  "or collection");
    return uninit_null();
  }
  PackedArrayInit ai(getContainerSize(cell_input));
  for (ArrayIter iter(cell_input); iter; ++iter) {
    ai.appendWithRef(iter.secondRefPlus());
  }
  return ai.toArray();
}

static void walk_func(VRefParam value, const Variant& key, const Variant& userdata,
                      const void *data) {
  CallCtx* ctx = (CallCtx*)data;
  Variant sink;
  TypedValue args[3] = { *value->asRef(), *key.asCell(), *userdata.asCell() };
  g_context->invokeFuncFew(sink.asTypedValue(), *ctx, 3, args);
}

bool f_array_walk_recursive(VRefParam input, const Variant& funcname,
                            const Variant& userdata /* = null_variant */) {
  if (!input.isArray()) {
    throw_expected_array_exception();
    return false;
  }
  CallCtx ctx;
  CallerFrame cf;
  vm_decode_function(funcname, cf(), false, ctx);
  if (ctx.func == NULL) {
    return false;
  }
  PointerSet seen;
  ArrayUtil::Walk(input, walk_func, &ctx, true, &seen, userdata);
  return true;
}

bool f_array_walk(VRefParam input, const Variant& funcname,
                  const Variant& userdata /* = null_variant */) {
  if (!input.isArray()) {
    throw_expected_array_exception();
    return false;
  }
  CallCtx ctx;
  CallerFrame cf;
  vm_decode_function(funcname, cf(), false, ctx);
  if (ctx.func == NULL) {
    return false;
  }
  ArrayUtil::Walk(input, walk_func, &ctx, false, NULL, userdata);
  return true;
}

static void compact(VarEnv* v, Array &ret, const Variant& var) {
  if (var.isArray()) {
    for (ArrayIter iter(var.getArrayData()); iter; ++iter) {
      compact(v, ret, iter.second());
    }
  } else {
    String varname = var.toString();
    if (!varname.empty() && v->lookup(varname.get()) != NULL) {
      ret.set(varname, *reinterpret_cast<Variant*>(v->lookup(varname.get())));
    }
  }
}

Array f_compact(int _argc, const Variant& varname, const Array& _argv /* = null_array */) {
  Array ret = Array::Create();
  VarEnv* v = g_context->getVarEnv();
  if (v) {
    compact(v, ret, varname);
    compact(v, ret, _argv);
  }
  return ret;
}

static int php_count_recursive(const Array& array) {
  long cnt = array.size();
  for (ArrayIter iter(array); iter; ++iter) {
    Variant value = iter.second();
    if (value.isArray()) {
      const Array& arr_value = value.asCArrRef();
      cnt += php_count_recursive(arr_value);
    }
  }
  return cnt;
}

bool f_shuffle(VRefParam array) {
  if (!array.isArray()) {
    throw_expected_array_exception();
    return false;
  }
  array = ArrayUtil::Shuffle(array);
  return true;
}

int64_t f_count(const Variant& var, int64_t mode /* = 0 */) {
  switch (var.getType()) {
  case KindOfUninit:
  case KindOfNull:
    return 0;
  case KindOfObject:
    {
      Object obj = var.toObject();
      if (obj->isCollection()) {
        return getCollectionSize(obj.get());
      }
      if (obj.instanceof(SystemLib::s_CountableClass)) {
        return obj->o_invoke_few_args(s_count, 0).toInt64();
      }
    }
    break;
  case KindOfArray:
    if (mode) {
      const Array& arr_var = var.toCArrRef();
      return php_count_recursive(arr_var);
    }
    return var.getArrayData()->size();
  default:
    break;
  }
  return 1;
}

int64_t f_sizeof(const Variant& var, int64_t mode /* = 0 */) {
  return f_count(var, mode);
}

namespace {

enum class NoCow {};
template<class DoCow = void, class NonArrayRet, class OpPtr>
static Variant iter_op_impl(VRefParam refParam, OpPtr op, NonArrayRet nonArray,
                            bool(ArrayData::*pred)() const =
                              &ArrayData::isInvalid) {
  auto& cell = *refParam.wrapped().asCell();
  if (cell.m_type != KindOfArray) {
    throw_bad_type_exception("expecting an array");
    return Variant(nonArray);
  }

  auto ad = cell.m_data.parr;
  auto constexpr doCow = !std::is_same<DoCow, NoCow>::value;
  if (doCow && ad->hasMultipleRefs() && !(ad->*pred)() &&
      !ad->noCopyOnWrite()) {
    ad = ad->copy();
    cellSet(make_tv<KindOfArray>(ad), cell);
  }
  return (ad->*op)();
}

}

Variant f_each(VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::each,
    Variant::NullInit()
  );
}

Variant f_current(VRefParam refParam) {
  return iter_op_impl<NoCow>(
    refParam,
    &ArrayData::current,
    false
  );
}

Variant f_pos(VRefParam refParam) {
  return f_current(refParam);
}

Variant f_key(VRefParam refParam) {
  return iter_op_impl<NoCow>(
    refParam,
    &ArrayData::key,
    false
  );
}

Variant f_next(VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::next,
    false
  );
}

Variant f_prev(VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::prev,
    false
  );
}

Variant f_reset(VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::reset,
    false,
    &ArrayData::isHead
  );
}

Variant f_end(VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::end,
    false,
    &ArrayData::isTail
  );
}

bool f_in_array(const Variant& needle, const Variant& haystack, bool strict /* = false */) {
  const auto& cell_haystack = *haystack.asCell();
  if (UNLIKELY(!isContainer(cell_haystack))) {
    raise_warning("in_array() expects parameter 2 to be an array "
                  "or collection");
    return false;
  }

  ArrayIter iter(cell_haystack);
  if (strict) {
    for (; iter; ++iter) {
      if (HPHP::same(iter.secondRefPlus(), needle)) {
        return true;
      }
    }
  } else {
    for (; iter; ++iter) {
      if (HPHP::equal(iter.secondRefPlus(), needle)) {
        return true;
      }
    }
  }
  return false;
}

Variant f_array_search(const Variant& needle, const Variant& haystack,
                       bool strict /* = false */) {
  const auto& cell_haystack = *haystack.asCell();
  if (UNLIKELY(!isContainer(cell_haystack))) {
    raise_warning("array_search() expects parameter 2 to be an array "
                  "or collection");
    return uninit_null();
  }

  ArrayIter iter(cell_haystack);
  if (strict) {
    for (; iter; ++iter) {
      if (HPHP::same(iter.secondRefPlus(), needle)) {
        return iter.first();
      }
    }
  } else {
    for (; iter; ++iter) {
      if (HPHP::equal(iter.secondRefPlus(), needle)) {
        return iter.first();
      }
    }
  }
  return false;
}

Variant f_range(const Variant& low, const Variant& high, const Variant& step /* = 1 */) {
  bool is_step_double = false;
  double dstep = 1.0;
  if (step.isDouble()) {
    dstep = step.toDouble();
    is_step_double = true;
  } else if (step.isString()) {
    int64_t sn;
    double sd;
    DataType stype = step.toString().get()->isNumericWithVal(sn, sd, 0);
    if (stype == KindOfDouble) {
      is_step_double = true;
      dstep = sd;
    } else if (stype == KindOfInt64) {
      dstep = (double)sn;
    } else {
      dstep = step.toDouble();
    }
  } else {
    dstep = step.toDouble();
  }
  /* We only want positive step values. */
  if (dstep < 0.0) dstep *= -1;
  if (low.isString() && high.isString()) {
    String slow = low.toString();
    String shigh = high.toString();
    if (slow.size() >= 1 && shigh.size() >=1) {
      int64_t n1, n2;
      double d1, d2;
      DataType type1 = slow.get()->isNumericWithVal(n1, d1, 0);
      DataType type2 = shigh.get()->isNumericWithVal(n2, d2, 0);
      if (type1 == KindOfDouble || type2 == KindOfDouble || is_step_double) {
        if (type1 != KindOfDouble) d1 = slow.toDouble();
        if (type2 != KindOfDouble) d2 = shigh.toDouble();
        return ArrayUtil::Range(d1, d2, dstep);
      }

      int64_t lstep = (int64_t) dstep;
      if (type1 == KindOfInt64 || type2 == KindOfInt64) {
        if (type1 != KindOfInt64) n1 = slow.toInt64();
        if (type2 != KindOfInt64) n2 = shigh.toInt64();
        return ArrayUtil::Range((double)n1, (double)n2, lstep);
      }

      return ArrayUtil::Range((unsigned char)slow.charAt(0),
                              (unsigned char)shigh.charAt(0), lstep);
    }
  }

  if (low.is(KindOfDouble) || high.is(KindOfDouble) || is_step_double) {
    return ArrayUtil::Range(low.toDouble(), high.toDouble(), dstep);
  }

  int64_t lstep = (int64_t) dstep;
  return ArrayUtil::Range(low.toDouble(), high.toDouble(), lstep);
}
///////////////////////////////////////////////////////////////////////////////
// diff/intersect helpers

static int cmp_func(const Variant& v1, const Variant& v2, const void *data) {
  Variant *callback = (Variant *)data;
  return vm_call_user_func(*callback, make_packed_array(v1, v2)).toInt32();
}

#define COMMA ,
#define diff_intersect_body(type,intersect_params,user_setup)   \
  getCheckedArray(array1);                                      \
  if (!arr_array1.size()) return arr_array1;                    \
  user_setup                                                    \
  Array ret = arr_array1.type(array2, intersect_params);        \
  if (ret.size()) {                                             \
    for (ArrayIter iter(_argv); iter; ++iter) {                 \
      ret = ret.type(iter.second(), intersect_params);          \
      if (!ret.size()) break;                                   \
    }                                                           \
  }                                                             \
  return ret;

///////////////////////////////////////////////////////////////////////////////
// diff functions

static inline void addToSetHelper(c_Set* st, const Cell& c, TypedValue* strTv,
                                  bool convertIntLikeStrs) {
  if (c.m_type == KindOfInt64) {
    st->add(c.m_data.num);
  } else {
    StringData* s;
    if (LIKELY(IS_STRING_TYPE(c.m_type))) {
      s = c.m_data.pstr;
    } else {
      s = tvCastToString(&c);
      decRefStr(strTv->m_data.pstr);
      strTv->m_data.pstr = s;
    }
    int64_t n;
    if (convertIntLikeStrs && s->isStrictlyInteger(n)) {
      st->add(n);
    } else {
      st->add(s);
    }
  }
}

static inline bool checkSetHelper(c_Set* st, const Cell& c, TypedValue* strTv,
                                  bool convertIntLikeStrs) {
  if (c.m_type == KindOfInt64) {
    return st->contains(c.m_data.num);
  }
  StringData* s;
  if (LIKELY(IS_STRING_TYPE(c.m_type))) {
    s = c.m_data.pstr;
  } else {
    s = tvCastToString(&c);
    decRefStr(strTv->m_data.pstr);
    strTv->m_data.pstr = s;
  }
  int64_t n;
  if (convertIntLikeStrs && s->isStrictlyInteger(n)) {
    return st->contains(n);
  }
  return st->contains(s);
}

static void containerValuesToSetHelper(c_Set* st, const Variant& container) {
  Variant strHolder(empty_string.get());
  TypedValue* strTv = strHolder.asTypedValue();
  for (ArrayIter iter(container); iter; ++iter) {
    const auto& c = *const_cast<TypedValue*>(iter.secondRefPlus().asCell());
    addToSetHelper(st, c, strTv, true);
  }
}

static void containerKeysToSetHelper(c_Set* st, const Variant& container) {
  Variant strHolder(empty_string.get());
  TypedValue* strTv = strHolder.asTypedValue();
  bool isKey = container.asCell()->m_type == KindOfArray;
  for (ArrayIter iter(container); iter; ++iter) {
    auto key = iter.first();
    const auto& c = *const_cast<TypedValue*>(key.asCell());
    addToSetHelper(st, c, strTv, !isKey);
  }
}

#define ARRAY_DIFF_PRELUDE() \
  /* Check to make sure all inputs are containers */ \
  const auto& c1 = *container1.asCell(); \
  const auto& c2 = *container2.asCell(); \
  if (UNLIKELY(!isContainer(c1) || !isContainer(c2))) { \
    raise_warning("%s() expects parameter %d to be an array or collection", \
                  __FUNCTION__+2, /* remove the "f_" prefix */ \
                  isContainer(c1) ? 2 : 1); \
    return uninit_null(); \
  } \
  bool moreThanTwo = (_argc > 2); \
  size_t largestSize = getContainerSize(c2); \
  if (UNLIKELY(moreThanTwo)) { \
    int pos = 3; \
    for (ArrayIter argvIter(_argv); argvIter; ++argvIter, ++pos) { \
      const auto& c = *argvIter.secondRef().asCell(); \
      if (!isContainer(c)) { \
        raise_warning("%s() expects parameter %d to be an array or collection",\
                      __FUNCTION__+2, /* remove the "f_" prefix */ \
                      pos); \
        return uninit_null(); \
      } \
      size_t sz = getContainerSize(c); \
      if (sz > largestSize) { \
        largestSize = sz; \
      } \
    } \
  } \
  /* If container1 is empty, we can stop here and return the empty array */ \
  if (!getContainerSize(c1)) return empty_array; \
  /* If all of the containers (except container1) are empty, we can just \
     return container1 (converting it to an array if needed) */ \
  if (!largestSize) { \
    if (c1.m_type == KindOfArray) { \
      return container1; \
    } else { \
      return container1.toArray(); \
    } \
  } \
  Array ret = Array::Create();

Variant f_array_diff(int _argc, const Variant& container1, const Variant& container2,
                     const Array& _argv /* = null_array */) {
  ARRAY_DIFF_PRELUDE()
  // Put all of the values from all the containers (except container1 into a
  // Set. All types aside from integer and string will be cast to string, and
  // we also convert int-like strings to integers.
  c_Set* st;
  Object setObj = st = NEWOBJ(c_Set)();
  st->reserve(largestSize);
  containerValuesToSetHelper(st, container2);
  if (UNLIKELY(moreThanTwo)) {
    for (ArrayIter argvIter(_argv); argvIter; ++argvIter) {
      const auto& container = argvIter.secondRef();
      containerValuesToSetHelper(st, container);
    }
  }
  // Loop over container1, only copying over key/value pairs where the value
  // is not present in the Set. When checking if a value is present in the
  // Set, any value that is not an integer or string is cast to a string, and
  // we convert int-like strings to integers.
  Variant strHolder(empty_string.get());
  TypedValue* strTv = strHolder.asTypedValue();
  bool isKey = c1.m_type == KindOfArray;
  for (ArrayIter iter(container1); iter; ++iter) {
    const auto& val = iter.secondRefPlus();
    const auto& c = *val.asCell();
    if (checkSetHelper(st, c, strTv, true)) continue;
    ret.setWithRef(iter.first(), val, isKey);
  }
  return ret;
}

Variant f_array_diff_key(int _argc, const Variant& container1, const Variant& container2,
                         const Array& _argv /* = null_array */) {
  ARRAY_DIFF_PRELUDE()
  // If we're only dealing with two containers and if they are both arrays,
  // we can avoid creating an intermediate Set
  if (!moreThanTwo && c1.m_type == KindOfArray && c2.m_type == KindOfArray) {
    auto ad2 = c2.m_data.parr;
    for (ArrayIter iter(container1); iter; ++iter) {
      auto key = iter.first();
      const auto& c = *key.asCell();
      if (c.m_type == KindOfInt64) {
        if (ad2->exists(c.m_data.num)) continue;
      } else {
        assert(IS_STRING_TYPE(c.m_type));
        if (ad2->exists(c.m_data.pstr)) continue;
      }
      ret.setWithRef(key, iter.secondRefPlus(), true);
    }
    return ret;
  }
  // Put all of the keys from all the containers (except container1) into a
  // Set. All types aside from integer and string will be cast to string, and
  // we also convert int-like strings to integers.
  c_Set* st;
  Object setObj = st = NEWOBJ(c_Set)();
  st->reserve(largestSize);
  containerKeysToSetHelper(st, container2);
  if (UNLIKELY(moreThanTwo)) {
    for (ArrayIter argvIter(_argv); argvIter; ++argvIter) {
      const auto& container = argvIter.secondRef();
      containerKeysToSetHelper(st, container);
    }
  }
  // Loop over container1, only copying over key/value pairs where the key is
  // not present in the Set. When checking if a key is present in the Set, any
  // key that is not an integer or string is cast to a string, and we convert
  // int-like strings to integers.
  Variant strHolder(empty_string.get());
  TypedValue* strTv = strHolder.asTypedValue();
  bool isKey = c1.m_type == KindOfArray;
  for (ArrayIter iter(container1); iter; ++iter) {
    auto key = iter.first();
    const auto& c = *key.asCell();
    if (checkSetHelper(st, c, strTv, !isKey)) continue;
    ret.setWithRef(key, iter.secondRefPlus(), isKey);
  }
  return ret;
}

#undef ARRAY_DIFF_PRELUDE

Variant f_array_udiff(int _argc, const Variant& array1, const Variant& array2,
                      const Variant& data_compare_func,
                      const Array& _argv /* = null_array */) {
  diff_intersect_body(diff, false COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_diff_assoc(int _argc, const Variant& array1, const Variant& array2,
                           const Array& _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA true,);
}

Variant f_array_diff_uassoc(int _argc, const Variant& array1, const Variant& array2,
                            const Variant& key_compare_func,
                            const Array& _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA true COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_udiff_assoc(int _argc, const Variant& array1, const Variant& array2,
                            const Variant& data_compare_func,
                            const Array& _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_udiff_uassoc(int _argc, const Variant& array1, const Variant& array2,
                             const Variant& data_compare_func,
                             const Variant& key_compare_func,
                             const Array& _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA true COMMA cmp_func COMMA &key_func
                      COMMA cmp_func COMMA &data_func,
                      Variant data_func = data_compare_func;
                      Variant key_func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(key_func);
                        extra.prepend(data_func);
                        key_func = extra.pop();
                        data_func = extra.pop();
                      });
}

Variant f_array_diff_ukey(int _argc, const Variant& array1, const Variant& array2,
                          const Variant& key_compare_func,
                          const Array& _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA false COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

///////////////////////////////////////////////////////////////////////////////
// intersect functions

static inline TypedValue* makeContainerListHelper(const Variant& a,
                                                  const Array& argv,
                                                  int count,
                                                  int smallestPos) {
  assert(count == argv.size() + 1);
  assert(0 <= smallestPos);
  assert(smallestPos < count);
  // Allocate a TypedValue array and copy 'a' and the contents of 'argv'
  TypedValue* containers =
    (TypedValue*)smart_malloc(count * sizeof(TypedValue));
  tvCopy(*a.asCell(), containers[0]);
  int pos = 1;
  for (ArrayIter argvIter(argv); argvIter; ++argvIter, ++pos) {
    const auto& c = *argvIter.secondRef().asCell();
    tvCopy(c, containers[pos]);
  }
  // Perform a swap so that the smallest container occurs at the first
  // position in the TypedValue array; this helps improve the performance
  // of containerValuesIntersectHelper()
  if (smallestPos != 0) {
    TypedValue tmp;
    tvCopy(containers[0], tmp);
    tvCopy(containers[smallestPos], containers[0]);
    tvCopy(tmp, containers[smallestPos]);
  }
  return containers;
}

static inline void addToIntersectMapHelper(c_Map* mp,
                                           const Cell& c,
                                           TypedValue* intOneTv,
                                           TypedValue* strTv,
                                           bool convertIntLikeStrs) {
  if (c.m_type == KindOfInt64) {
    mp->set(c.m_data.num, intOneTv);
  } else {
    StringData* s;
    if (LIKELY(IS_STRING_TYPE(c.m_type))) {
      s = c.m_data.pstr;
    } else {
      s = tvCastToString(&c);
      decRefStr(strTv->m_data.pstr);
      strTv->m_data.pstr = s;
    }
    int64_t n;
    if (convertIntLikeStrs && s->isStrictlyInteger(n)) {
      mp->set(n, intOneTv);
    } else {
      mp->set(s, intOneTv);
    }
  }
}

static inline void updateIntersectMapHelper(c_Map* mp,
                                            const Cell& c,
                                            int pos,
                                            TypedValue* strTv,
                                            bool convertIntLikeStrs) {
  if (c.m_type == KindOfInt64) {
    auto val = mp->get(c.m_data.num);
    if (val && val->m_data.num == pos) {
      assert(val->m_type == KindOfInt64);
      ++val->m_data.num;
    }
  } else {
    StringData* s;
    if (LIKELY(IS_STRING_TYPE(c.m_type))) {
      s = c.m_data.pstr;
    } else {
      s = tvCastToString(&c);
      decRefStr(strTv->m_data.pstr);
      strTv->m_data.pstr = s;
    }
    int64_t n;
    if (convertIntLikeStrs && s->isStrictlyInteger(n)) {
      auto val = mp->get(n);
      if (val && val->m_data.num == pos) {
        assert(val->m_type == KindOfInt64);
        ++val->m_data.num;
      }
    } else {
      auto val = mp->get(s);
      if (val && val->m_data.num == pos) {
        assert(val->m_type == KindOfInt64);
        ++val->m_data.num;
      }
    }
  }
}

static void containerValuesIntersectHelper(c_Set* st,
                                           TypedValue* containers,
                                           int count) {
  assert(count >= 2);
  c_Map* mp;
  Object mapObj = mp = NEWOBJ(c_Map)();
  Variant strHolder(empty_string.get());
  TypedValue* strTv = strHolder.asTypedValue();
  TypedValue intOneTv = make_tv<KindOfInt64>(1);
  for (ArrayIter iter(tvAsCVarRef(&containers[0])); iter; ++iter) {
    const auto& c = *const_cast<TypedValue*>(iter.secondRefPlus().asCell());
    // For each value v in containers[0], we add the key/value pair (v, 1)
    // to the map. If a value (after various conversions) occurs more than
    // once in the container, we'll simply overwrite the old entry and that's
    // fine.
    addToIntersectMapHelper(mp, c, &intOneTv, strTv, true);
  }
  for (int pos = 1; pos < count; ++pos) {
    for (ArrayIter iter(tvAsCVarRef(&containers[pos])); iter; ++iter) {
      const auto& c = *const_cast<TypedValue*>(iter.secondRefPlus().asCell());
      // We check if the value is present as a key in the map. If an entry
      // exists and its value equals pos, we increment it, otherwise we do
      // nothing. This is essential so that we don't accidentally double-count
      // a key (after various conversions) that occurs in the container more
      // than once.
      updateIntersectMapHelper(mp, c, pos, strTv, true);
    }
  }
  for (ArrayIter iter(mapObj); iter; ++iter) {
    // For each key in the map, we copy the key to the set if the
    // corresponding value is equal to pos exactly (which means it
    // was present in all of the containers).
    const auto& val = *iter.secondRefPlus().asCell();
    assert(val.m_type == KindOfInt64);
    if (val.m_data.num == count) {
      st->add(iter.first().asCell());
    }
  }
}

static void containerKeysIntersectHelper(c_Set* st,
                                         TypedValue* containers,
                                         int count) {
  assert(count >= 2);
  c_Map* mp;
  Object mapObj = mp = NEWOBJ(c_Map)();
  Variant strHolder(empty_string.get());
  TypedValue* strTv = strHolder.asTypedValue();
  TypedValue intOneTv = make_tv<KindOfInt64>(1);
  bool isKey = containers[0].m_type == KindOfArray;
  for (ArrayIter iter(tvAsCVarRef(&containers[0])); iter; ++iter) {
    auto key = iter.first();
    const auto& c = *key.asCell();
    // For each key k in containers[0], we add the key/value pair (k, 1)
    // to the map. If a key (after various conversions) occurs more than
    // once in the container, we'll simply overwrite the old entry and
    // that's fine.
    addToIntersectMapHelper(mp, c, &intOneTv, strTv, !isKey);
  }
  for (int pos = 1; pos < count; ++pos) {
    isKey = containers[pos].m_type == KindOfArray;
    for (ArrayIter iter(tvAsCVarRef(&containers[pos])); iter; ++iter) {
      auto key = iter.first();
      const auto& c = *key.asCell();
      updateIntersectMapHelper(mp, c, pos, strTv, !isKey);
    }
  }
  for (ArrayIter iter(mapObj); iter; ++iter) {
    // For each key in the map, we copy the key to the set if the
    // corresponding value is equal to pos exactly (which means it
    // was present in all of the containers).
    const auto& val = *iter.secondRefPlus().asCell();
    assert(val.m_type == KindOfInt64);
    if (val.m_data.num == count) {
      st->add(iter.first().asCell());
    }
  }
}

#define ARRAY_INTERSECT_PRELUDE() \
  /* Check to make sure all inputs are containers */ \
  const auto& c1 = *container1.asCell(); \
  const auto& c2 = *container2.asCell(); \
  if (!isContainer(c1) || !isContainer(c2)) { \
    raise_warning("%s() expects parameter %d to be an array or collection", \
                  __FUNCTION__+2, /* remove the "f_" prefix */ \
                  isContainer(c1) ? 2 : 1); \
    return uninit_null(); \
  } \
  bool moreThanTwo = (_argc > 2); \
  /* Keep track of which input container was the smallest (excluding \
     container1) */ \
  int smallestPos = 0; \
  size_t smallestSize = getContainerSize(c2); \
  if (UNLIKELY(moreThanTwo)) { \
    int pos = 1; \
    for (ArrayIter argvIter(_argv); argvIter; ++argvIter, ++pos) { \
      const auto& c = *argvIter.secondRef().asCell(); \
      if (!isContainer(c)) { \
        raise_warning("%s() expects parameter %d to be an array or collection",\
                      __FUNCTION__+2, /* remove the "f_" prefix */ \
                      pos+2); \
        return uninit_null(); \
      } \
      size_t sz = getContainerSize(c); \
      if (sz < smallestSize) { \
        smallestSize = sz; \
        smallestPos = pos; \
      } \
    } \
  } \
  /* If any of the containers were empty, we can stop here and return the \
     empty array */ \
  if (!getContainerSize(c1) || !smallestSize) return empty_array; \
  Array ret = Array::Create();

Variant f_array_intersect(int _argc, const Variant& container1, const Variant& container2,
                          const Array& _argv /* = null_array */) {
  ARRAY_INTERSECT_PRELUDE()
  // Build up a Set containing the values that are present in all the
  // containers (except container1)
  c_Set* st;
  Object setObj = st = NEWOBJ(c_Set)();
  if (LIKELY(!moreThanTwo)) {
    // There is only one container (not counting container1) so we can
    // just call containerValuesToSetHelper() to build the Set.
    containerValuesToSetHelper(st, container2);
  } else {
    // We're dealing with three or more containers. Copy all of the containers
    // (except the first) into a TypedValue array.
    int count = _argv.size() + 1;
    TypedValue* containers =
      makeContainerListHelper(container2, _argv, count, smallestPos);
    SCOPE_EXIT { smart_free(containers); };
    // Build a Set of the values that were present in all of the containers
    containerValuesIntersectHelper(st, containers, count);
  }
  // Loop over container1, only copying over key/value pairs where the value
  // is present in the Set. When checking if a value is present in the Set,
  // any value that is not an integer or string is cast to a string, and we
  // convert int-like strings to integers.
  Variant strHolder(empty_string.get());
  TypedValue* strTv = strHolder.asTypedValue();
  bool isKey = c1.m_type == KindOfArray;
  for (ArrayIter iter(container1); iter; ++iter) {
    const auto& val = iter.secondRefPlus();
    const auto& c = *val.asCell();
    if (!checkSetHelper(st, c, strTv, true)) continue;
    ret.setWithRef(iter.first(), val, isKey);
  }
  return ret;
}

Variant f_array_intersect_key(int _argc, const Variant& container1, const Variant& container2,
                              const Array& _argv /* = null_array */) {
  ARRAY_INTERSECT_PRELUDE()
  // If we're only dealing with two containers and if they are both arrays,
  // we can avoid creating an intermediate Set
  if (!moreThanTwo && c1.m_type == KindOfArray && c2.m_type == KindOfArray) {
    auto ad2 = c2.m_data.parr;
    for (ArrayIter iter(container1); iter; ++iter) {
      auto key = iter.first();
      const auto& c = *key.asCell();
      if (c.m_type == KindOfInt64) {
        if (!ad2->exists(c.m_data.num)) continue;
      } else {
        assert(IS_STRING_TYPE(c.m_type));
        if (!ad2->exists(c.m_data.pstr)) continue;
      }
      ret.setWithRef(key, iter.secondRefPlus(), true);
    }
    return ret;
  }
  // Build up a Set containing the keys that are present in all the containers
  // (except container1)
  c_Set* st;
  Object setObj = st = NEWOBJ(c_Set)();
  if (LIKELY(!moreThanTwo)) {
    // There is only one container (not counting container1) so we can just
    // call containerKeysToSetHelper() to build the Set.
    containerKeysToSetHelper(st, container2);
  } else {
    // We're dealing with three or more containers. Copy all of the containers
    // (except the first) into a TypedValue array.
    int count = _argv.size() + 1;
    TypedValue* containers =
      makeContainerListHelper(container2, _argv, count, smallestPos);
    SCOPE_EXIT { smart_free(containers); };
    // Build a Set of the keys that were present in all of the containers
    containerKeysIntersectHelper(st, containers, count);
  }
  // Loop over container1, only copying over key/value pairs where the key
  // is present in the Set. When checking if a key is present in the Set,
  // any value that is not an integer or string is cast to a string, and we
  // convert int-like strings to integers.
  Variant strHolder(empty_string.get());
  TypedValue* strTv = strHolder.asTypedValue();
  bool isKey = c1.m_type == KindOfArray;
  for (ArrayIter iter(container1); iter; ++iter) {
    auto key = iter.first();
    const auto& c = *key.asCell();
    if (!checkSetHelper(st, c, strTv, !isKey)) continue;
    ret.setWithRef(key, iter.secondRefPlus(), isKey);
  }
  return ret;
}

#undef ARRAY_INTERSECT_PRELUDE

Variant f_array_uintersect(int _argc, const Variant& array1, const Variant& array2,
                           const Variant& data_compare_func,
                           const Array& _argv /* = null_array */) {
  diff_intersect_body(intersect, false COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_intersect_assoc(int _argc, const Variant& array1, const Variant& array2,
                                const Array& _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA true,);
}

Variant f_array_intersect_uassoc(int _argc, const Variant& array1, const Variant& array2,
                                 const Variant& key_compare_func,
                                 const Array& _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA true COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_uintersect_assoc(int _argc, const Variant& array1, const Variant& array2,
                                 const Variant& data_compare_func,
                                 const Array& _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_uintersect_uassoc(int _argc, const Variant& array1, const Variant& array2,
                                  const Variant& data_compare_func,
                                  const Variant& key_compare_func,
                                  const Array& _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA true COMMA cmp_func COMMA &key_func
                      COMMA cmp_func COMMA &data_func,
                      Variant data_func = data_compare_func;
                      Variant key_func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(key_func);
                        extra.prepend(data_func);
                        key_func = extra.pop();
                        data_func = extra.pop();
                      });
}

Variant f_array_intersect_ukey(int _argc, const Variant& array1, const Variant& array2,
                             const Variant& key_compare_func, const Array& _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA false COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

///////////////////////////////////////////////////////////////////////////////
// sorting functions

struct Collator final : RequestEventHandler {
  String getLocale() {
    return m_locale;
  }
  Intl::IntlError &getErrorRef() {
    return m_errcode;
  }
  bool setLocale(const String& locale) {
    if (m_locale.same(locale)) {
      return true;
    }
    if (m_ucoll) {
      ucol_close(m_ucoll);
      m_ucoll = NULL;
    }
    m_errcode.clearError();
    UErrorCode error = U_ZERO_ERROR;
    m_ucoll = ucol_open(locale.data(), &error);
    if (m_ucoll == NULL) {
      raise_warning("failed to load %s locale from icu data", locale.data());
      return false;
    }
    if (U_FAILURE(error)) {
      m_errcode.setError(error);
      ucol_close(m_ucoll);
      m_ucoll = NULL;
      return false;
    }
    m_locale = locale;
    return true;
  }

  UCollator *getCollator() {
    return m_ucoll;
  }

  bool setAttribute(int64_t attr, int64_t val) {
    if (!m_ucoll) {
      Logger::Verbose("m_ucoll is NULL");
      return false;
    }
    m_errcode.clearError();
    UErrorCode error = U_ZERO_ERROR;
    ucol_setAttribute(m_ucoll, (UColAttribute)attr,
                      (UColAttributeValue)val, &error);
    if (U_FAILURE(error)) {
      m_errcode.setError(error);
      Logger::Verbose("Error setting attribute value");
      return false;
    }
    return true;
  }

  bool setStrength(int64_t strength) {
    if (!m_ucoll) {
      Logger::Verbose("m_ucoll is NULL");
      return false;
    }
    ucol_setStrength(m_ucoll, (UCollationStrength)strength);
    return true;
  }

  Variant getErrorCode() {
    if (!m_ucoll) {
      Logger::Verbose("m_ucoll is NULL");
      return false;
    }
    return m_errcode.getErrorCode();
  }

  void requestInit() override {
    m_locale = String(uloc_getDefault(), CopyString);
    m_errcode.clearError();
    UErrorCode error = U_ZERO_ERROR;
    m_ucoll = ucol_open(m_locale.data(), &error);
    if (U_FAILURE(error)) {
      m_errcode.setError(error);
    }
    assert(m_ucoll);
  }
  void requestShutdown() override {
    m_locale.reset();
    m_errcode.clearError(false);
    if (m_ucoll) {
      ucol_close(m_ucoll);
      m_ucoll = NULL;
    }
  }

private:
  String     m_locale;
  UCollator *m_ucoll;
  Intl::IntlError m_errcode;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(Collator, s_collator);

static Array::PFUNC_CMP get_cmp_func(int sort_flags, bool ascending) {
  switch (sort_flags) {
  case SORT_NATURAL:
    return Array::SortNatural;
  case SORT_NATURAL_CASE:
    return Array::SortNaturalCase;
  case SORT_NUMERIC:
    return ascending ?
      Array::SortNumericAscending : Array::SortNumericDescending;
  case SORT_STRING:
    return ascending ?
      Array::SortStringAscending : Array::SortStringDescending;
  case SORT_STRING_CASE:
    return ascending ?
      Array::SortStringAscendingCase : Array::SortStringDescendingCase;
  case SORT_LOCALE_STRING:
    return ascending ?
      Array::SortLocaleStringAscending : Array::SortLocaleStringDescending;
  case SORT_REGULAR:
  default:
    return ascending ?
      Array::SortRegularAscending : Array::SortRegularDescending;
  }
}

class ArraySortTmp {
 public:
  explicit ArraySortTmp(Array& arr) : m_arr(arr) {
    m_ad = arr.get()->escalateForSort();
    m_ad->incRefCount();
  }
  ~ArraySortTmp() {
    if (m_ad != m_arr.get()) {
      m_arr = m_ad;
      m_ad->decRefCount();
    }
  }
  ArrayData* operator->() { return m_ad; }
 private:
  Array& m_arr;
  ArrayData* m_ad;
};

static bool
php_sort(VRefParam container, int sort_flags,
         bool ascending, bool use_collator) {
  if (container.isArray()) {
    Array& arr_array = container.wrapped().toArrRef();
    if (use_collator && sort_flags != SORT_LOCALE_STRING) {
      UCollator *coll = s_collator->getCollator();
      if (coll) {
        Intl::IntlError &errcode = s_collator->getErrorRef();
        return collator_sort(container, sort_flags, ascending,
                             coll, &errcode);
      }
    }
    ArraySortTmp ast(arr_array);
    ast->sort(sort_flags, ascending);
    return true;
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->getCollectionType() == Collection::VectorType) {
      c_Vector* vec = static_cast<c_Vector*>(obj);
      vec->sort(sort_flags, ascending);
      return true;
    }
    // other collections are not supported:
    //  - mapping types require associative sort
    //  - frozen types are not to be modified
    //  - set types are not ordered
  }
  throw_expected_array_or_collection_exception();
  return false;
}

static bool
php_asort(VRefParam container, int sort_flags,
          bool ascending, bool use_collator) {
  if (container.isArray()) {
    Array& arr_array = container.wrapped().toArrRef();
    if (use_collator && sort_flags != SORT_LOCALE_STRING) {
      UCollator *coll = s_collator->getCollator();
      if (coll) {
        Intl::IntlError &errcode = s_collator->getErrorRef();
        return collator_asort(container, sort_flags, ascending,
                              coll, &errcode);
      }
    }
    ArraySortTmp ast(arr_array);
    ast->asort(sort_flags, ascending);
    return true;
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->getCollectionType() == Collection::MapType) {
      BaseMap* mp = static_cast<BaseMap*>(obj);
      mp->asort(sort_flags, ascending);
      return true;
    }
  }
  throw_expected_array_or_collection_exception();
  return false;
}

static bool
php_ksort(VRefParam container, int sort_flags, bool ascending) {
  if (container.isArray()) {
    Array& arr_array = container.wrapped().toArrRef();
    ArraySortTmp ast(arr_array);
    ast->ksort(sort_flags, ascending);
    return true;
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->getCollectionType() == Collection::MapType) {
      BaseMap* mp = static_cast<BaseMap*>(obj);
      mp->ksort(sort_flags, ascending);
      return true;
    }
  }
  throw_expected_array_or_collection_exception();
  return false;
}

bool f_sort(VRefParam array, int sort_flags /* = 0 */,
            bool use_collator /* = false */) {
  return php_sort(array, sort_flags, true, use_collator);
}

bool f_rsort(VRefParam array, int sort_flags /* = 0 */,
             bool use_collator /* = false */) {
  return php_sort(array, sort_flags, false, use_collator);
}

bool f_asort(VRefParam array, int sort_flags /* = 0 */,
             bool use_collator /* = false */) {
  return php_asort(array, sort_flags, true, use_collator);
}

bool f_arsort(VRefParam array, int sort_flags /* = 0 */,
              bool use_collator /* = false */) {
  return php_asort(array, sort_flags, false, use_collator);
}

bool f_ksort(VRefParam array, int sort_flags /* = 0 */) {
  return php_ksort(array, sort_flags, true);
}

bool f_krsort(VRefParam array, int sort_flags /* = 0 */) {
  return php_ksort(array, sort_flags, false);
}

// NOTE: PHP's implementation of natsort and natcasesort accepts ArrayAccess
// objects as well, which does not make much sense, and which is not supported
// here.

Variant f_natsort(VRefParam array) {
  return php_asort(array, SORT_NATURAL, true, false);
}

Variant f_natcasesort(VRefParam array) {
  return php_asort(array, SORT_NATURAL_CASE, true, false);
}

bool f_usort(VRefParam container, const Variant& cmp_function) {
  if (container.isArray()) {
    Array& arr_array = container.wrapped().toArrRef();
    ArraySortTmp ast(arr_array);
    return ast->usort(cmp_function);
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->getCollectionType() == Collection::VectorType) {
      c_Vector* vec = static_cast<c_Vector*>(obj);
      return vec->usort(cmp_function);
    }
    // other collections are not supported:
    //  - mapping types require associative sort
    //  - frozen types are not to be modified
    //  - set types are not ordered
  }
  throw_expected_array_or_collection_exception();
  return false;
}

bool f_uasort(VRefParam container, const Variant& cmp_function) {
  if (container.isArray()) {
    Array& arr_array = container.wrapped().toArrRef();
    ArraySortTmp ast(arr_array);
    return ast->uasort(cmp_function);
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->getCollectionType() == Collection::MapType) {
      BaseMap* mp = static_cast<BaseMap*>(obj);
      return mp->uasort(cmp_function);
    }
  }
  throw_expected_array_or_collection_exception();
  return false;
}

bool f_uksort(VRefParam container, const Variant& cmp_function) {
  if (container.isArray()) {
    Array& arr_array = container.wrapped().toArrRef();
    ArraySortTmp ast(arr_array);
    return ast->uksort(cmp_function);
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->getCollectionType() == Collection::MapType) {
      BaseMap* mp = static_cast<BaseMap*>(obj);
      return mp->uksort(cmp_function);
    }
  }
  throw_expected_array_or_collection_exception();
  return false;
}

bool f_array_multisort(int _argc, VRefParam ar1,
                       const Array& _argv /* = null_array */) {
  getCheckedArrayRet(ar1, false);
  std::vector<Array::SortData> data;
  std::vector<Array> arrays;
  arrays.reserve(1 + _argv.size()); // so no resize would happen

  Array::SortData sd;
  sd.original = &ar1;
  arrays.push_back(arr_ar1);
  sd.array = &arrays.back();
  sd.by_key = false;

  int sort_flags = SORT_REGULAR;
  bool ascending = true;
  for (int i = 0; i < _argv.size(); i++) {
    Variant *v = &((Array&)_argv).lvalAt(i);
    auto const cell = v->asCell();
    if (cell->m_type == KindOfArray) {
      sd.cmp_func = get_cmp_func(sort_flags, ascending);
      data.push_back(sd);

      sort_flags = SORT_REGULAR;
      ascending = true;

      sd.original = v;
      arrays.push_back(Array(cell->m_data.parr));
      sd.array = &arrays.back();
    } else {
      int n = v->toInt32();
      if (n == SORT_ASC) {
        ascending = true;
      } else if (n == SORT_DESC) {
        ascending = false;
      } else {
        sort_flags = n;
      }
    }
  }

  sd.cmp_func = get_cmp_func(sort_flags, ascending);
  data.push_back(sd);

  return Array::MultiSort(data, true);
}

Variant f_array_unique(const Variant& array, int sort_flags /* = 2 */) {
  // NOTE, PHP array_unique accepts ArrayAccess objects as well,
  // which is not supported here.
  getCheckedArray(array);
  switch (sort_flags) {
  case SORT_STRING:
  case SORT_LOCALE_STRING:
    return ArrayUtil::StringUnique(arr_array);
  case SORT_NUMERIC:
    return ArrayUtil::NumericUnique(arr_array);
  case SORT_REGULAR:
  default:
    return ArrayUtil::RegularSortUnique(arr_array);
  }
}

String f_i18n_loc_get_default() {
  return s_collator->getLocale();
}

bool f_i18n_loc_set_default(const String& locale) {
  return s_collator->setLocale(locale);
}

bool f_i18n_loc_set_attribute(int64_t attr, int64_t val) {
  return s_collator->setAttribute(attr, val);
}

bool f_i18n_loc_set_strength(int64_t strength) {
  return s_collator->setStrength(strength);
}

Variant f_i18n_loc_get_error_code() {
  return s_collator->getErrorCode();
}

Variant f_hphp_array_idx(const Variant& search, const Variant& key, const Variant& def) {
  if (!key.isNull()) {
    if (LIKELY(search.isArray())) {
      ArrayData *arr = search.getArrayData();
      VarNR index = key.toKey();
      if (!index.isNull()) {
        const Variant& ret = arr->get(index, false);
        return (&ret != &null_variant) ? ret : def;
      }
    } else {
      raise_error("hphp_array_idx: search must be an array");
    }
  }
  return def;
}

///////////////////////////////////////////////////////////////////////////////
}
