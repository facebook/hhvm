/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
  auto const cell_##input = static_cast<CVarRef>(input).asCell(); \
  if (UNLIKELY(cell_##input->m_type != KindOfArray)) {            \
    throw_expected_array_exception();                             \
    return fail;                                                  \
  }                                                               \
  ArrNR arrNR_##input(cell_##input->m_data.parr);                 \
  CArrRef arr_##input = arrNR_##input.asArray();

#define getCheckedArray(input) getCheckedArrayRet(input, uninit_null())

Variant f_array_change_key_case(CVarRef input, bool upper /* = false */) {
  getCheckedArrayRet(input, false);
  return ArrayUtil::ChangeKeyCase(arr_input, !upper);
}
Variant f_array_chunk(CVarRef input, int size,
                      bool preserve_keys /* = false */) {
  getCheckedArray(input);
  return ArrayUtil::Chunk(arr_input, size, preserve_keys);
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

Variant f_array_column(CVarRef input, CVarRef val_key,
                       CVarRef idx_key /* = null_variant */) {
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

Variant f_array_combine(CVarRef keys, CVarRef values) {
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
    ret.setWithRef(iter1.secondRefPlus(), iter2.secondRefPlus());
  }
  return ret;
}

Variant f_array_count_values(CVarRef input) {
  getCheckedArray(input);
  return ArrayUtil::CountValues(arr_input);
}

Variant f_array_fill_keys(CVarRef keys, CVarRef value) {
  getCheckedArray(keys);
  return ArrayUtil::CreateArray(arr_keys, value);
}

Variant f_array_fill(int start_index, int num, CVarRef value) {
  return ArrayUtil::CreateArray(start_index, num, value);
}

Variant f_array_flip(CVarRef trans) {
  getCheckedArrayRet(trans, false);
  ArrayInit ret(arr_trans.size());
  for (ArrayIter iter(arr_trans); iter; ++iter) {
    CVarRef value(iter.secondRef());
    if (value.isString() || value.isInteger()) {
      ret.set(value, iter.first());
    } else {
      raise_warning("Can only flip STRING and INTEGER values!");
    }
  }
  return ret.toVariant();
}

bool f_array_key_exists(CVarRef key, CVarRef search) {
  const ArrayData *ad;

  auto const searchCell = search.asCell();
  if (LIKELY(searchCell->m_type == KindOfArray)) {
    ad = searchCell->m_data.parr;
  } else if (searchCell->m_type == KindOfObject) {
    ObjectData* obj = searchCell->m_data.pobj;
    if (obj->isCollection()) {
      return collectionOffsetContains(obj, key);
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

bool f_key_exists(CVarRef key, CVarRef search) {
  return f_array_key_exists(key, search);
}

Variant f_array_keys(CVarRef input, CVarRef search_value /* = null_variant */,
                     bool strict /* = false */) {
  const auto& cell_input = *input.asCell();
  if (UNLIKELY(!isContainer(cell_input))) {
    goto warn;
  }
  {
    ArrayIter iter(cell_input);
    if (LIKELY(!search_value.isInitialized())) {
      PackedArrayInit ai(getContainerSize(cell_input));
      for (; iter; ++iter) {
        ai.append(iter.first());
      }
      return ai.toArray();
    }

    Array ai = Array::attach(HphpArray::MakeReserve(0));
    for (; iter; ++iter) {
      if ((strict && HPHP::same(iter.secondRefPlus(), search_value)) ||
          (!strict && HPHP::equal(iter.secondRefPlus(), search_value))) {
        ai.append(iter.first());
      }
    }
    return ai;
  }
warn:
  raise_warning("array_keys() expects parameter 1 to be an array "
                "or collection");
  return uninit_null();
}

Variant f_array_map(int _argc, CVarRef callback, CVarRef arr1, CArrRef _argv /* = null_array */) {
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
      g_vmContext->invokeFuncFew((TypedValue*)&result, ctx, 1,
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
      g_vmContext->invokeFunc((TypedValue*)&result,
                              ctx.func, params, ctx.this_,
                              ctx.cls, nullptr, ctx.invName);
      ret.append(result);
    } else {
      ret.append(params);
    }
  }
  return ret;
}

static void php_array_merge(Array &arr1, CArrRef arr2) {
  arr1.merge(arr2);
}

static void php_array_merge_recursive(PointerSet &seen, bool check,
                                      Array &arr1, CArrRef arr2) {
  if (check) {
    if (seen.find((void*)arr1.get()) != seen.end()) {
      raise_warning("array_merge_recursive(): recursion detected");
      return;
    }
    seen.insert((void*)arr1.get());
  }

  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key(iter.first());
    CVarRef value(iter.secondRef());
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

Variant f_array_merge(int _argc, CVarRef array1,
                      CArrRef _argv /* = null_array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  php_array_merge(ret, arr_array1);
  for (ArrayIter iter(_argv); iter; ++iter) {
    Variant v = iter.second();
    if (!v.isArray()) {
      throw_expected_array_exception();
      return uninit_null();
    }
    CArrRef arr_v = v.asCArrRef();
    php_array_merge(ret, arr_v);
  }
  return ret;
}

Variant f_array_merge_recursive(int _argc, CVarRef array1,
                                CArrRef _argv /* = null_array */) {
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
    CArrRef arr_v = v.asCArrRef();
    php_array_merge_recursive(seen, false, ret, arr_v);
    assert(seen.empty());
  }
  return ret;
}

static void php_array_replace(Array &arr1, CArrRef arr2) {
  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key = iter.first();
    CVarRef value = iter.secondRef();
    arr1.setWithRef(key, value, true);
  }
}

static void php_array_replace_recursive(PointerSet &seen, bool check,
                                        Array &arr1, CArrRef arr2) {
  if (check) {
    if (seen.find((void*)arr1.get()) != seen.end()) {
      raise_warning("array_replace_recursive(): recursion detected");
      return;
    }
    seen.insert((void*)arr1.get());
  }

  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key = iter.first();
    CVarRef value = iter.secondRef();
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

Variant f_array_replace(int _argc, CVarRef array1,
                        CArrRef _argv /* = null_array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  php_array_replace(ret, arr_array1);
  for (ArrayIter iter(_argv); iter; ++iter) {
    CVarRef v = iter.secondRef();
    getCheckedArray(v);
    php_array_replace(ret, arr_v);
  }
  return ret;
}

Variant f_array_replace_recursive(int _argc, CVarRef array1,
                                  CArrRef _argv /* = null_array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  PointerSet seen;
  php_array_replace_recursive(seen, false, ret, arr_array1);
  assert(seen.empty());
  for (ArrayIter iter(_argv); iter; ++iter) {
    CVarRef v = iter.secondRef();
    getCheckedArray(v);
    php_array_replace_recursive(seen, false, ret, arr_v);
    assert(seen.empty());
  }
  return ret;
}

Variant f_array_pad(CVarRef input, int pad_size, CVarRef pad_value) {
  getCheckedArray(input);
  if (pad_size > 0) {
    return ArrayUtil::Pad(arr_input, pad_value, pad_size, true);
  }
  return ArrayUtil::Pad(arr_input, pad_value, -pad_size, false);
}

Variant f_array_pop(VRefParam array) {
  return array.pop();
}

Variant f_array_product(CVarRef array) {
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
                     CVarRef var, CArrRef _argv /* = null_array */) {

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

Variant f_array_rand(CVarRef input, int num_req /* = 1 */) {
  getCheckedArray(input);
  return ArrayUtil::RandomKeys(arr_input, num_req);
}

static Variant reduce_func(CVarRef result, CVarRef operand, const void *data) {
  CallCtx* ctx = (CallCtx*)data;
  Variant ret;
  TypedValue args[2] = { *result.asCell(), *operand.asCell() };
  g_vmContext->invokeFuncFew(ret.asTypedValue(), *ctx, 2, args);
  return ret;
}
Variant f_array_reduce(CVarRef input, CVarRef callback,
                       CVarRef initial /* = null_variant */) {
  getCheckedArray(input);
  CallCtx ctx;
  CallerFrame cf;
  vm_decode_function(callback, cf(), false, ctx);
  if (ctx.func == NULL) {
    return uninit_null();
  }
  return ArrayUtil::Reduce(arr_input, reduce_func, &ctx, initial);
}

Variant f_array_reverse(CVarRef array, bool preserve_keys /* = false */) {
  getCheckedArray(array);
  return ArrayUtil::Reverse(arr_array, preserve_keys);
}

Variant f_array_search(CVarRef needle, CVarRef haystack,
                       bool strict /* = false */) {
  getCheckedArrayRet(haystack, false);
  return arr_haystack.key(needle, strict);
}

Variant f_array_shift(VRefParam array) {
  return array.dequeue();
}

Variant f_array_slice(CVarRef array, int offset,
                      CVarRef length /* = null_variant */,
                      bool preserve_keys /* = false */) {
  getCheckedArray(array);
  int64_t len = length.isNull() ? 0x7FFFFFFF : length.toInt64();
  return ArrayUtil::Slice(arr_array, offset, len, preserve_keys);
}
Variant f_array_splice(VRefParam input, int offset,
                       CVarRef length /* = null_variant */,
                       CVarRef replacement /* = null_variant */) {
  getCheckedArray(input);
  Array ret(Array::Create());
  int64_t len = length.isNull() ? 0x7FFFFFFF : length.toInt64();
  input = ArrayUtil::Splice(arr_input, offset, len, replacement, &ret);
  return ret;
}
Variant f_array_sum(CVarRef array) {
  getCheckedArray(array);
  int64_t i;
  double d;
  if (ArrayUtil::Sum(arr_array, &i, &d) == KindOfInt64) {
    return i;
  } else {
    return d;
  }
}

int64_t f_array_unshift(int _argc, VRefParam array, CVarRef var, CArrRef _argv /* = null_array */) {
  if (array.toArray()->isVectorData()) {
    if (!_argv.empty()) {
      for (ssize_t pos = _argv->iter_end(); pos != ArrayData::invalid_index;
        pos = _argv->iter_rewind(pos)) {
        array.prepend(_argv->getValueRef(pos));
      }
    }
    array.prepend(var);
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
        CVarRef value(iter.secondRef());
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

Variant f_array_values(CVarRef input) {
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

static void walk_func(VRefParam value, CVarRef key, CVarRef userdata,
                      const void *data) {
  CallCtx* ctx = (CallCtx*)data;
  Variant sink;
  TypedValue args[3] = { *value->asRef(), *key.asCell(), *userdata.asCell() };
  g_vmContext->invokeFuncFew(sink.asTypedValue(), *ctx, 3, args);
}

bool f_array_walk_recursive(VRefParam input, CVarRef funcname,
                            CVarRef userdata /* = null_variant */) {
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

bool f_array_walk(VRefParam input, CVarRef funcname,
                  CVarRef userdata /* = null_variant */) {
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

static void compact(VarEnv* v, Array &ret, CVarRef var) {
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

Array f_compact(int _argc, CVarRef varname, CArrRef _argv /* = null_array */) {
  Array ret = Array::Create();
  VarEnv* v = g_vmContext->getVarEnv();
  if (v) {
    compact(v, ret, varname);
    compact(v, ret, _argv);
  }
  return ret;
}

static int php_count_recursive(CArrRef array) {
  long cnt = array.size();
  for (ArrayIter iter(array); iter; ++iter) {
    Variant value = iter.second();
    if (value.isArray()) {
      CArrRef arr_value = value.asCArrRef();
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

int64_t f_count(CVarRef var, bool recursive /* = false */) {
  switch (var.getType()) {
  case KindOfUninit:
  case KindOfNull:
    return 0;
  case KindOfObject:
    {
      Object obj = var.toObject();
      if (obj->isCollection()) {
        return obj->getCollectionSize();
      }
      if (obj.instanceof(SystemLib::s_CountableClass)) {
        return obj->o_invoke_few_args(s_count, 0).toInt64();
      }
    }
    break;
  case KindOfArray:
    if (recursive) {
      CArrRef arr_var = var.toCArrRef();
      return php_count_recursive(arr_var);
    }
    return var.getArrayData()->size();
  default:
    break;
  }
  return 1;
}

int64_t f_sizeof(CVarRef var, bool recursive /* = false */) {
  return f_count(var, recursive);
}

namespace {

enum class CowTag {
  Yes,
  No
};

template<CowTag Cow, class Op, class NonArrayRet>
static Variant iter_op_impl(VRefParam refParam, Op op, NonArrayRet nonArray) {
  auto& cell = *refParam.wrapped().asCell();
  if (cell.m_type != KindOfArray) {
    throw_bad_type_exception("expecting an array");
    return Variant(nonArray);
  }

  auto ad = cell.m_data.parr;
  if (Cow == CowTag::Yes) {
    if (ad->hasMultipleRefs() && !ad->isInvalid() && !ad->noCopyOnWrite()) {
      ad = ad->copy();
      cellSet(make_tv<KindOfArray>(ad), cell);
    }
  }
  return op(ad);
}

}

Variant f_each(VRefParam refParam) {
  return iter_op_impl<CowTag::Yes>(
    refParam,
    [] (ArrayData* ad) { return ad->each(); },
    Variant::NullInit()
  );
}

Variant f_current(VRefParam refParam) {
  return iter_op_impl<CowTag::No>(
    refParam,
    [] (ArrayData* ad) { return ad->current(); },
    false
  );
}

Variant f_pos(VRefParam refParam) {
  return f_current(refParam);
}

Variant f_key(VRefParam refParam) {
  return iter_op_impl<CowTag::No>(
    refParam,
    [] (ArrayData* ad) { return ad->key(); },
    false
  );
}

Variant f_next(VRefParam refParam) {
  return iter_op_impl<CowTag::Yes>(
    refParam,
    [] (ArrayData* ad) { return ad->next(); },
    false
  );
}

Variant f_prev(VRefParam refParam) {
  return iter_op_impl<CowTag::Yes>(
    refParam,
    [] (ArrayData* ad) { return ad->prev(); },
    false
  );
}

Variant f_reset(VRefParam refParam) {
  return iter_op_impl<CowTag::No>(
    refParam,
    [] (ArrayData* ad) { return ad->reset(); },
    false
  );
}

Variant f_end(VRefParam refParam) {
  return iter_op_impl<CowTag::No>(
    refParam,
    [] (ArrayData* ad) { return ad->end(); },
    false
  );
}

bool f_in_array(CVarRef needle, CVarRef haystack, bool strict /* = false */) {
  getCheckedArrayRet(haystack, false);
  return arr_haystack.valueExists(needle, strict);
}

Variant f_range(CVarRef low, CVarRef high, CVarRef step /* = 1 */) {
  bool is_step_double = false;
  double dstep = 1.0;
  if (step.isDouble()) {
    dstep = step.toDouble();
    is_step_double = true;
  } else if (step.isString()) {
    int64_t sn;
    double sd;
    DataType stype = step.toString()->isNumericWithVal(sn, sd, 0);
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
      DataType type1 = slow->isNumericWithVal(n1, d1, 0);
      DataType type2 = shigh->isNumericWithVal(n2, d2, 0);
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

static int cmp_func(CVarRef v1, CVarRef v2, const void *data) {
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

static void containerValuesToSetHelper(c_Set* st, CVarRef container) {
  Variant strHolder(empty_string.get());
  TypedValue* strTv = strHolder.asTypedValue();
  for (ArrayIter iter(container); iter; ++iter) {
    const auto& c = *const_cast<TypedValue*>(iter.secondRefPlus().asCell());
    addToSetHelper(st, c, strTv, true);
  }
}

static void containerKeysToSetHelper(c_Set* st, CVarRef container) {
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

Variant f_array_diff(int _argc, CVarRef container1, CVarRef container2,
                     CArrRef _argv /* = null_array */) {
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

Variant f_array_diff_key(int _argc, CVarRef container1, CVarRef container2,
                         CArrRef _argv /* = null_array */) {
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

Variant f_array_udiff(int _argc, CVarRef array1, CVarRef array2,
                      CVarRef data_compare_func,
                      CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, false COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_diff_assoc(int _argc, CVarRef array1, CVarRef array2,
                           CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA true,);
}

Variant f_array_diff_uassoc(int _argc, CVarRef array1, CVarRef array2,
                            CVarRef key_compare_func,
                            CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA true COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_udiff_assoc(int _argc, CVarRef array1, CVarRef array2,
                            CVarRef data_compare_func,
                            CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_udiff_uassoc(int _argc, CVarRef array1, CVarRef array2,
                             CVarRef data_compare_func,
                             CVarRef key_compare_func,
                             CArrRef _argv /* = null_array */) {
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

Variant f_array_diff_ukey(int _argc, CVarRef array1, CVarRef array2,
                          CVarRef key_compare_func,
                          CArrRef _argv /* = null_array */) {
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

static inline TypedValue* makeContainerListHelper(CVarRef a,
                                                  CArrRef argv,
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

Variant f_array_intersect(int _argc, CVarRef container1, CVarRef container2,
                          CArrRef _argv /* = null_array */) {
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

Variant f_array_intersect_key(int _argc, CVarRef container1, CVarRef container2,
                              CArrRef _argv /* = null_array */) {
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

Variant f_array_uintersect(int _argc, CVarRef array1, CVarRef array2,
                           CVarRef data_compare_func,
                           CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, false COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_intersect_assoc(int _argc, CVarRef array1, CVarRef array2,
                                CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA true,);
}

Variant f_array_intersect_uassoc(int _argc, CVarRef array1, CVarRef array2,
                                 CVarRef key_compare_func,
                                 CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA true COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_uintersect_assoc(int _argc, CVarRef array1, CVarRef array2,
                                 CVarRef data_compare_func,
                                 CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_uintersect_uassoc(int _argc, CVarRef array1, CVarRef array2,
                                  CVarRef data_compare_func,
                                  CVarRef key_compare_func,
                                  CArrRef _argv /* = null_array */) {
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

Variant f_array_intersect_ukey(int _argc, CVarRef array1, CVarRef array2,
                             CVarRef key_compare_func, CArrRef _argv /* = null_array */) {
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

class Collator : public RequestEventHandler {
public:
  String getLocale() {
    return m_locale;
  }
  intl_error &getErrorCodeRef() {
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
    m_errcode.clear();
    m_ucoll = ucol_open(locale.data(), &(m_errcode.code));
    if (m_ucoll == NULL) {
      raise_warning("failed to load %s locale from icu data", locale.data());
      return false;
    }
    if (U_FAILURE(m_errcode.code)) {
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
    m_errcode.clear();
    ucol_setAttribute(m_ucoll, (UColAttribute)attr,
                      (UColAttributeValue)val, &(m_errcode.code));
    if (U_FAILURE(m_errcode.code)) {
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
    return m_errcode.code;
  }

  virtual void requestInit() {
    m_locale = String(uloc_getDefault(), CopyString);
    m_errcode.clear();
    m_ucoll = ucol_open(m_locale.data(), &(m_errcode.code));
    assert(m_ucoll);
  }
  virtual void requestShutdown() {
    m_locale.reset();
    m_errcode.clear();
    if (m_ucoll) {
      ucol_close(m_ucoll);
      m_ucoll = NULL;
    }
  }

private:
  String     m_locale;
  UCollator *m_ucoll;
  intl_error m_errcode;
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
php_sort(VRefParam array, int sort_flags, bool ascending, bool use_collator) {
  if (array.isArray()) {
    Array& arr_array = array.wrapped().toArrRef();
    if (use_collator && sort_flags != SORT_LOCALE_STRING) {
      UCollator *coll = s_collator->getCollator();
      if (coll) {
        intl_error &errcode = s_collator->getErrorCodeRef();
        return collator_sort(array, sort_flags, ascending,
                             coll, &errcode);
      }
    }
    ArraySortTmp ast(arr_array);
    ast->sort(sort_flags, ascending);
    return true;
  }
  if (array.isObject()) {
    ObjectData* obj = array.getObjectData();
    if (obj->getCollectionType() == Collection::VectorType) {
      c_Vector* vec = static_cast<c_Vector*>(obj);
      vec->sort(sort_flags, ascending);
      return true;
    }
  }
  throw_expected_array_or_collection_exception();
  return false;
}

static bool
php_asort(VRefParam array, int sort_flags, bool ascending, bool use_collator) {
  if (array.isArray()) {
    Array& arr_array = array.wrapped().toArrRef();
    if (use_collator && sort_flags != SORT_LOCALE_STRING) {
      UCollator *coll = s_collator->getCollator();
      if (coll) {
        intl_error &errcode = s_collator->getErrorCodeRef();
        return collator_asort(array, sort_flags, ascending,
                              coll, &errcode);
      }
    }
    ArraySortTmp ast(arr_array);
    ast->asort(sort_flags, ascending);
    return true;
  }
  if (array.isObject()) {
    ObjectData* obj = array.getObjectData();
    if (obj->getCollectionType() == Collection::StableMapType) {
      c_StableMap* smp = static_cast<c_StableMap*>(obj);
      smp->asort(sort_flags, ascending);
      return true;
    }
  }
  throw_expected_array_or_collection_exception();
  return false;
}

static bool
php_ksort(VRefParam array, int sort_flags, bool ascending) {
  if (array.isArray()) {
    Array& arr_array = array.wrapped().toArrRef();
    ArraySortTmp ast(arr_array);
    ast->ksort(sort_flags, ascending);
    return true;
  }
  if (array.isObject()) {
    ObjectData* obj = array.getObjectData();
    if (obj->getCollectionType() == Collection::StableMapType) {
      c_StableMap* smp = static_cast<c_StableMap*>(obj);
      smp->ksort(sort_flags, ascending);
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

bool f_usort(VRefParam array, CVarRef cmp_function) {
  if (array.isArray()) {
    Array& arr_array = array.wrapped().toArrRef();
    ArraySortTmp ast(arr_array);
    return ast->usort(cmp_function);
  }
  if (array.isObject()) {
    ObjectData* obj = array.getObjectData();
    if (obj->getCollectionType() == Collection::VectorType) {
      c_Vector* vec = static_cast<c_Vector*>(obj);
      return vec->usort(cmp_function);
    }
  }
  throw_expected_array_or_collection_exception();
  return false;
}

bool f_uasort(VRefParam array, CVarRef cmp_function) {
  if (array.isArray()) {
    Array& arr_array = array.wrapped().toArrRef();
    ArraySortTmp ast(arr_array);
    return ast->uasort(cmp_function);
  }
  if (array.isObject()) {
    ObjectData* obj = array.getObjectData();
    if (obj->getCollectionType() == Collection::StableMapType) {
      c_StableMap* smp = static_cast<c_StableMap*>(obj);
      return smp->uasort(cmp_function);
    }
  }
  throw_expected_array_or_collection_exception();
  return false;
}

bool f_uksort(VRefParam array, CVarRef cmp_function) {
  if (array.isArray()) {
    Array& arr_array = array.wrapped().toArrRef();
    ArraySortTmp ast(arr_array);
    return ast->uksort(cmp_function);
  }
  if (array.isObject()) {
    ObjectData* obj = array.getObjectData();
    if (obj->getCollectionType() == Collection::StableMapType) {
      c_StableMap* smp = static_cast<c_StableMap*>(obj);
      return smp->uksort(cmp_function);
    }
  }
  throw_expected_array_or_collection_exception();
  return false;
}

bool f_array_multisort(int _argc, VRefParam ar1,
                       CArrRef _argv /* = null_array */) {
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

Variant f_array_unique(CVarRef array, int sort_flags /* = 2 */) {
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

Variant f_hphp_array_idx(CVarRef search, CVarRef key, CVarRef def) {
  if (!key.isNull()) {
    if (LIKELY(search.isArray())) {
      ArrayData *arr = search.getArrayData();
      VarNR index = key.toKey();
      if (!index.isNull()) {
        CVarRef ret = arr->get(index, false);
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
