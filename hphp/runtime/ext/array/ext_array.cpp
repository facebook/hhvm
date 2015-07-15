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

#include "hphp/runtime/ext/array/ext_array.h"

#include "hphp/runtime/base/actrec-args.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/zend-collator.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/logger.h"

#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define SORT_DESC               3
#define SORT_ASC                4

#define DEFINE_CONSTANT(name)                                                  \
  const int64_t k_##name = name;                                               \
  const StaticString s_##name(#name)                                           \

const StaticString s_count("count");

DEFINE_CONSTANT(UCOL_DEFAULT);

DEFINE_CONSTANT(UCOL_PRIMARY);
DEFINE_CONSTANT(UCOL_SECONDARY);
DEFINE_CONSTANT(UCOL_TERTIARY);
DEFINE_CONSTANT(UCOL_DEFAULT_STRENGTH);
DEFINE_CONSTANT(UCOL_QUATERNARY);
DEFINE_CONSTANT(UCOL_IDENTICAL);

DEFINE_CONSTANT(UCOL_OFF);
DEFINE_CONSTANT(UCOL_ON);

DEFINE_CONSTANT(UCOL_SHIFTED);
DEFINE_CONSTANT(UCOL_NON_IGNORABLE);

DEFINE_CONSTANT(UCOL_LOWER_FIRST);
DEFINE_CONSTANT(UCOL_UPPER_FIRST);

DEFINE_CONSTANT(UCOL_FRENCH_COLLATION);
DEFINE_CONSTANT(UCOL_ALTERNATE_HANDLING);
DEFINE_CONSTANT(UCOL_CASE_FIRST);
DEFINE_CONSTANT(UCOL_CASE_LEVEL);
DEFINE_CONSTANT(UCOL_NORMALIZATION_MODE);
DEFINE_CONSTANT(UCOL_STRENGTH);
DEFINE_CONSTANT(UCOL_HIRAGANA_QUATERNARY_MODE);
DEFINE_CONSTANT(UCOL_NUMERIC_COLLATION);

#undef DEFINE_CONSTANT

Variant HHVM_FUNCTION(array_change_key_case,
                      const Variant& input,
                      int64_t case_ /* = 0 */) {
  getCheckedArrayRet(input, false);
  return ArrayUtil::ChangeKeyCase(arr_input, !case_);
}

Variant HHVM_FUNCTION(array_chunk,
                      const Variant& input,
                      int chunkSize,
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

  auto const retSize = (getContainerSize(cellInput) / chunkSize) + 1;
  PackedArrayInit ret(retSize);
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

  return ret.toVariant();
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
    raise_warning("array_column(): The %s key should be either a string "
                  "or an integer", name);
    return false;
  }
}

Variant HHVM_FUNCTION(array_column,
                      const Variant& input,
                      const Variant& val_key,
                      const Variant& idx_key /* = null_variant */) {
  /* Be strict about array type */
  getCheckedArrayColumnRet(input, uninit_null());
  Variant val = val_key, idx = idx_key;
  if (!array_column_coerce_key(val, "column") ||
      !array_column_coerce_key(idx, "index")) {
    return false;
  }
  ArrayInit ret(arr_input.size(), ArrayInit::Map{});
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
      ret.setKeyUnconverted(sub[idx].toString(), elem);
    } else {
      ret.setKeyUnconverted(sub[idx], elem);
    }
  }
  return ret.toVariant();
}

Variant HHVM_FUNCTION(array_combine,
                      const Variant& keys,
                      const Variant& values) {
  const auto& cell_keys = *keys.asCell();
  const auto& cell_values = *values.asCell();
  if (UNLIKELY(!isContainer(cell_keys) || !isContainer(cell_values))) {
    raise_warning("Invalid operand type was used: array_combine expects "
                  "arrays or collections");
    return init_null();
  }
  auto keys_size = getContainerSize(cell_keys);
  if (UNLIKELY(keys_size != getContainerSize(cell_values))) {
    raise_warning("array_combine(): Both parameters should have an equal "
                  "number of elements");
    return false;
  }
  Array ret = Array::attach(MixedArray::MakeReserveMixed(keys_size));
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

Variant HHVM_FUNCTION(array_count_values,
                      const Variant& input) {
  getCheckedArray(input);
  return ArrayUtil::CountValues(arr_input);
}

Variant HHVM_FUNCTION(array_fill_keys,
                      const Variant& keys,
                      const Variant& value) {
  const auto& cell_keys = *keys.asCell();
  if (UNLIKELY(!isContainer(cell_keys))) {
    raise_warning("Invalid operand type was used: array_fill_keys expects "
                  "an array or collection");
    return init_null();
  }

  auto size = getContainerSize(cell_keys);
  if (!size) return empty_array();

  ArrayInit ai(size, ArrayInit::Mixed{});
  for (ArrayIter iter(cell_keys); iter; ++iter) {
    auto& key = iter.secondRefPlus();
    // This is intentionally different to the $foo[$invalid_key] coercion.
    // See tests/slow/ext_array/array_fill_keys_tostring.php for examples.
    if (LIKELY(key.isInteger() || key.isString())) {
      ai.setKeyUnconverted(key, value);
    } else {
      raise_hack_strict(RuntimeOption::StrictArrayFillKeys,
                        "strict_array_fill_keys",
                        "keys must be ints or strings");
      ai.setKeyUnconverted(key.toString(), value);
    }
  }
  return ai.toVariant();
}

Variant HHVM_FUNCTION(array_fill,
                      int start_index,
                      int num,
                      const Variant& value) {
  if (num < 0) {
    throw_invalid_argument("Number of elements can't be negative");
    return false;
  }

  if (start_index == 0) {
    PackedArrayInit pai(num, CheckAllocation{});
    for (size_t k = 0; k < num; k++) {
      pai.append(value);
    }
    return pai.toVariant();
  } else {
    ArrayInit ret(num, ArrayInit::Mixed{}, CheckAllocation{});
    ret.set(start_index, value);
    for (int i = num - 1; i > 0; i--) {
      ret.append(value);
    }
    return ret.toVariant();
  }
}

Variant HHVM_FUNCTION(array_flip,
                      const Variant& trans) {
  auto const& transCell = *trans.asCell();
  if (UNLIKELY(!isContainer(transCell))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection", __FUNCTION__+2);
    return init_null();
  }

  ArrayInit ret(getContainerSize(transCell), ArrayInit::Mixed{});
  for (ArrayIter iter(transCell); iter; ++iter) {
    const Variant& value(iter.secondRefPlus());
    if (value.isString() || value.isInteger()) {
      ret.setKeyUnconverted(value, iter.first());
    } else {
      raise_warning("Can only flip STRING and INTEGER values!");
    }
  }
  return ret.toVariant();
}

bool HHVM_FUNCTION(array_key_exists,
                   const Variant& key,
                   const Variant& search) {
  const ArrayData *ad;

  auto const searchCell = search.asCell();
  if (LIKELY(searchCell->m_type == KindOfArray)) {
    ad = searchCell->m_data.parr;
  } else if (searchCell->m_type == KindOfObject) {
    ObjectData* obj = searchCell->m_data.pobj;
    if (obj->isCollection()) {
      return collections::contains(obj, key);
    }
    return HHVM_FN(array_key_exists)(key, toArray(search));
  } else {
    throw_bad_type_exception("array_key_exists expects an array or an object; "
                             "false returned.");
    return false;
  }

  auto const cell = key.asCell();
  switch (cell->m_type) {
    case KindOfUninit:
    case KindOfNull:
      return ad->exists(staticEmptyString());

    case KindOfInt64:
      return ad->exists(cell->m_data.num);

    case KindOfStaticString:
    case KindOfString: {
      int64_t n = 0;
      StringData *sd = cell->m_data.pstr;
      if (sd->isStrictlyInteger(n)) {
        return ad->exists(n);
      }
      return ad->exists(StrNR(sd));
    }

    case KindOfBoolean:
    case KindOfDouble:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
      raise_warning("Array key should be either a string or an integer");
      return false;

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

bool HHVM_FUNCTION(key_exists,
                   const Variant& key,
                   const Variant& search) {
  return HHVM_FN(array_key_exists)(key, search);
}

Variant array_keys_helper(const Variant& input,
                          const Variant& search_value /* = uninit_null */,
                          bool strict /* = false */) {
  const auto& cell_input = *input.asCell();
  if (UNLIKELY(!isContainer(cell_input))) {
    raise_warning("array_keys() expects parameter 1 to be an array "
                  "or collection");
    return init_null();
  }

  if (LIKELY(!search_value.isInitialized())) {
    PackedArrayInit ai(getContainerSize(cell_input));
    for (ArrayIter iter(cell_input); iter; ++iter) {
      ai.append(iter.first());
    }
    return ai.toVariant();
  } else {
    Array ai = Array::attach(MixedArray::MakeReserve(0));
    for (ArrayIter iter(cell_input); iter; ++iter) {
      if ((strict && HPHP::same(iter.secondRefPlus(), search_value)) ||
          (!strict && HPHP::equal(iter.secondRefPlus(), search_value))) {
        ai.append(iter.first());
      }
    }
    return ai;
  }
}

TypedValue* HHVM_FN(array_keys)(ActRec* ar) {
  int32_t argc = ar->numArgs();
  if (argc < 1 || argc > 3) {
    throw_wrong_arguments_nr(ar->m_func->name()->data(), argc, 1, 3);
    return arReturn(ar, init_null());
  }

  Variant key = getArgVariant(ar, 0);
  Variant search_value = argc > 1 ? getArgVariant(ar, 1) : uninit_null();
  bool strict = argc > 2 ? getArg<KindOfBoolean>(ar, 2) : false;
  return arReturn(ar, array_keys_helper(key, search_value, strict));
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
      auto subarr1 = v.toArray().copy();
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

Variant HHVM_FUNCTION(array_map, const Variant& callback,
                                 const Variant& arr1,
                                 const Array& _argv /* = null_array */) {
  CallCtx ctx;
  ctx.func = nullptr;
  if (!callback.isNull()) {
    CallerFrame cf;
    vm_decode_function(callback, cf(), false, ctx);
  }
  const auto& cell_arr1 = *arr1.asCell();
  if (UNLIKELY(!isContainer(cell_arr1))) {
    raise_warning("array_map(): Argument #2 should be an array or collection");
    return init_null();
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
    ArrayInit ret(getContainerSize(cell_arr1), ArrayInit::Map{});
    bool keyConverted = (cell_arr1.m_type == KindOfArray);
    if (!keyConverted) {
      auto col_type = cell_arr1.m_data.pobj->collectionType();
      keyConverted = !collectionAllowsIntStringKeys(col_type);
    }
    for (ArrayIter iter(arr1); iter; ++iter) {
      Variant result;
      g_context->invokeFuncFew((TypedValue*)&result, ctx, 1,
                               iter.secondRefPlus().asCell());
      // if keyConverted is false, it's possible that ret will have fewer
      // elements than cell_arr1; keys int(1) and string('1') may both be
      // present
      ret.add(iter.first(), result, keyConverted);
    }
    return ret.toVariant();
  }

  // Handle the uncommon case where the caller passed a callback
  // and two or more containers
  ArrayIter* iters =
    (ArrayIter*)req::malloc(sizeof(ArrayIter) * (_argv.size() + 1));
  size_t numIters = 0;
  SCOPE_EXIT {
    while (numIters--) iters[numIters].~ArrayIter();
    req::free(iters);
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
  PackedArrayInit ret_ai(maxLen);
  for (size_t k = 0; k < maxLen; k++) {
    PackedArrayInit params_ai(numIters);
    for (size_t i = 0; i < numIters; ++i) {
      if (iters[i]) {
        params_ai.append(iters[i].secondRefPlus());
        ++iters[i];
      } else {
        params_ai.append(init_null_variant);
      }
    }
    Array params = params_ai.toArray();
    if (ctx.func) {
      Variant result;
      g_context->invokeFunc((TypedValue*)&result,
                              ctx.func, params, ctx.this_,
                              ctx.cls, nullptr, ctx.invName);
      ret_ai.append(result);
    } else {
      ret_ai.append(params);
    }
  }
  return ret_ai.toVariant();
}

Variant HHVM_FUNCTION(array_merge,
                      int64_t numArgs,
                      const Variant& array1,
                      const Variant& array2 /* = null_variant */,
                      const Array& args /* = null array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  php_array_merge(ret, arr_array1);

  if (UNLIKELY(numArgs < 2)) return ret;

  getCheckedArray(array2);
  php_array_merge(ret, arr_array2);

  for (ArrayIter iter(args); iter; ++iter) {
    Variant v = iter.second();
    if (!v.isArray()) {
      throw_expected_array_exception("array_merge");
      return init_null();
    }
    const Array& arr_v = v.asCArrRef();
    php_array_merge(ret, arr_v);
  }
  return ret;
}

Variant HHVM_FUNCTION(array_merge_recursive,
                      int64_t numArgs,
                      const Variant& array1,
                      const Variant& array2 /* = null_variant */,
                      const Array& args /* = null array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  PointerSet seen;
  php_array_merge_recursive(seen, false, ret, arr_array1);
  assert(seen.empty());

  if (UNLIKELY(numArgs < 2)) return ret;

  getCheckedArray(array2);
  php_array_merge_recursive(seen, false, ret, arr_array2);
  assert(seen.empty());

  for (ArrayIter iter(args); iter; ++iter) {
    Variant v = iter.second();
    if (!v.isArray()) {
      throw_expected_array_exception("array_merge_recursive");
      return init_null();
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

Variant HHVM_FUNCTION(array_replace,
                      const Variant& array1,
                      const Variant& array2 /* = null_variant */,
                      const Array& args /* = null array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  php_array_replace(ret, arr_array1);

  if (UNLIKELY(array2.isNull() && args.empty())) return ret;

  getCheckedArray(array2);
  php_array_replace(ret, arr_array2);

  for (ArrayIter iter(args); iter; ++iter) {
    const Variant& v = iter.secondRef();
    getCheckedArray(v);
    php_array_replace(ret, arr_v);
  }
  return ret;
}

Variant HHVM_FUNCTION(array_replace_recursive,
                      const Variant& array1,
                      const Variant& array2 /* = null_variant */,
                      const Array& args /* = null array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  PointerSet seen;
  php_array_replace_recursive(seen, false, ret, arr_array1);
  assert(seen.empty());

  if (UNLIKELY(array2.isNull() && args.empty())) return ret;

  getCheckedArray(array2);
  php_array_replace_recursive(seen, false, ret, arr_array2);
  assert(seen.empty());

  for (ArrayIter iter(args); iter; ++iter) {
    const Variant& v = iter.secondRef();
    getCheckedArray(v);
    php_array_replace_recursive(seen, false, ret, arr_v);
    assert(seen.empty());
  }
  return ret;
}

Variant HHVM_FUNCTION(array_pad,
                      const Variant& input,
                      int pad_size,
                      const Variant& pad_value) {
  getCheckedArray(input);
  if (pad_size > 0) {
    return ArrayUtil::Pad(arr_input, pad_value, pad_size, true);
  }
  return ArrayUtil::Pad(arr_input, pad_value, -pad_size, false);
}

Variant HHVM_FUNCTION(array_pop,
                      VRefParam containerRef) {
  const auto* container = containerRef->asCell();
  if (UNLIKELY(!isMutableContainer(*container))) {
    raise_warning("array_pop() expects parameter 1 to be an "
                  "array or mutable collection");
    return init_null();
  }
  if (!getContainerSize(containerRef)) {
    return init_null();
  }
  if (container->m_type == KindOfArray) {
    return containerRef.wrapped().toArrRef().pop();
  }
  assert(container->m_type == KindOfObject);
  return collections::pop(container->m_data.pobj);
}

Variant HHVM_FUNCTION(array_product,
                      const Variant& input) {
  if (UNLIKELY(!isContainer(input))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection as argument 1",
                  __FUNCTION__+2);
    return init_null();
  }

  int64_t i = 1;
  ArrayIter iter(input);
  for (; iter; ++iter) {
    const Variant& entry(iter.secondRefPlus());

    switch (entry.getType()) {
      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfRef:
        i *= entry.toInt64();
        continue;

      case KindOfDouble:
        goto DOUBLE;

      case KindOfStaticString:
      case KindOfString: {
        int64_t ti;
        double td;
        if (entry.getStringData()->isNumericWithVal(ti, td, 1) ==
            KindOfInt64) {
          i *= ti;
          continue;
        } else {
          goto DOUBLE;
        }
      }

      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
        continue;

      case KindOfClass:
        break;
    }
    not_reached();
  }
  return i;

DOUBLE:
  double d = i;
  for (; iter; ++iter) {
    const Variant& entry(iter.secondRefPlus());
    switch (entry.getType()) {
      DT_UNCOUNTED_CASE:
      case KindOfString:
      case KindOfRef:
        d *= entry.toDouble();

      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
        continue;

      case KindOfClass:
        break;
    }
    not_reached();
  }
  return d;
}

Variant HHVM_FUNCTION(array_push,
                      VRefParam container,
                      const Variant& var,
                      const Array& args /* = null array */) {

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
    for (ArrayIter iter(args); iter; ++iter) {
      arr_array.append(iter.second());
    }
    return arr_array.size();
  }

  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->isCollection()) {
      switch (obj->collectionType()) {
        case CollectionType::Vector: {
          c_Vector* vec = static_cast<c_Vector*>(obj);
          vec->reserve(vec->size() + args.size() + 1);
          vec->t_add(var);
          for (ArrayIter iter(args); iter; ++iter) {
            vec->t_add(iter.second());
          }
          return vec->size();
        }
        case CollectionType::Set: {
          c_Set* set = static_cast<c_Set*>(obj);
          set->reserve(set->size() + args.size() + 1);
          set->t_add(var);
          for (ArrayIter iter(args); iter; ++iter) {
            set->t_add(iter.second());
          }
          return set->size();
        }
        case CollectionType::Map:
        case CollectionType::Pair:
        case CollectionType::ImmVector:
        case CollectionType::ImmMap:
        case CollectionType::ImmSet:
          // other collection types are unsupported:
          //  - mapping collections require a key
          //  - immutable collections don't allow insertion
          break;
      }
    }
  }
  throw_expected_array_or_collection_exception("array_push");
  return init_null();
}

Variant HHVM_FUNCTION(array_rand,
                      const Variant& input,
                      int num_req /* = 1 */) {
  getCheckedArray(input);
  return ArrayUtil::RandomKeys(arr_input, num_req);
}

Variant HHVM_FUNCTION(array_reverse,
                      const Variant& input,
                      bool preserve_keys /* = false */) {

  const auto& cell_input = *input.asCell();
  if (UNLIKELY(!isContainer(cell_input))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection as argument 1",
                  __FUNCTION__+2);
    return init_null();
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
  return ArrayUtil::Reverse(obj->toArray(), preserve_keys);
}

Variant HHVM_FUNCTION(array_shift,
                      VRefParam array) {
  const auto* cell_array = array->asCell();
  if (UNLIKELY(!isMutableContainer(*cell_array))) {
    raise_warning("array_shift() expects parameter 1 to be an "
                  "array or mutable collection");
    return init_null();
  }
  if (!getContainerSize(array)) {
    return init_null();
  }
  if (cell_array->m_type == KindOfArray) {
    return array.wrapped().toArrRef().dequeue();
  }
  assertx(cell_array->m_type == KindOfObject);
  return collections::shift(cell_array->m_data.pobj);
}

Variant HHVM_FUNCTION(array_slice,
                      const Variant& input,
                      int64_t offset,
                      const Variant& length /* = null_variant */,
                      bool preserve_keys /* = false */) {
  const auto& cell_input = *input.asCell();
  if (UNLIKELY(!isContainer(cell_input))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection as argument 1",
                  __FUNCTION__+2);
    return init_null();
  }
  int64_t len = length.isNull() ? 0x7FFFFFFF : length.toInt64();

  const int64_t num_in = getContainerSize(cell_input);
  if (offset > num_in) {
    offset = num_in;
  } else if (offset < 0 && (offset = (num_in + offset)) < 0) {
    offset = 0;
  }

  auto const maxLen = num_in - offset;
  if (len < 0) {
    len = maxLen + len;
  } else if (len > maxLen) {
    len = maxLen;
  }

  if (len <= 0) {
    return empty_array();
  }

  bool input_is_packed = isPackedContainer(cell_input);

  // If the slice covers the entire input container, we can just nop when
  // preserve_keys is true, or when preserve_keys is false but the container
  // is packed so we know the keys already map to [0,N].
  if (offset == 0 && len == num_in && (preserve_keys || input_is_packed)) {
    return input.toArray();
  }

  int pos = 0;
  ArrayIter iter(input);
  for (; pos < offset && iter; ++pos, ++iter) {}

  if (input_is_packed && (offset == 0 || !preserve_keys)) {
    PackedArrayInit ret(len);
    for (; pos < (offset + len) && iter; ++pos, ++iter) {
      ret.appendWithRef(iter.secondRefPlus());
    }
    return ret.toVariant();
  } else {
    // Otherwise PackedArrayInit can't be used because non-numeric keys are
    // preserved even when preserve_keys is false
    Array ret = Array::attach(MixedArray::MakeReserve(len));
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
}

Variant HHVM_FUNCTION(array_splice,
                      VRefParam input,
                      int offset,
                      const Variant& length /* = null_variant */,
                      const Variant& replacement /* = null_variant */) {
  getCheckedArray(input);
  Array ret(Array::Create());
  int64_t len = length.isNull() ? 0x7FFFFFFF : length.toInt64();
  input = ArrayUtil::Splice(arr_input, offset, len, replacement, &ret);
  return ret;
}

Variant HHVM_FUNCTION(array_sum,
                      const Variant& input) {
  if (UNLIKELY(!isContainer(input))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection as argument 1",
                  __FUNCTION__+2);
    return init_null();
  }

  int64_t i = 0;
  ArrayIter iter(input);
  for (; iter; ++iter) {
    const Variant& entry(iter.secondRefPlus());

    switch (entry.getType()) {
      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfRef:
        i += entry.toInt64();
        continue;

      case KindOfDouble:
        goto DOUBLE;

      case KindOfStaticString:
      case KindOfString: {
        int64_t ti;
        double td;
        if (entry.getStringData()->isNumericWithVal(ti, td, 1) ==
            KindOfInt64) {
          i += ti;
          continue;
        } else {
          goto DOUBLE;
        }
      }

      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
        continue;

      case KindOfClass:
        break;
    }
    not_reached();
  }
  return i;

DOUBLE:
  double d = i;
  for (; iter; ++iter) {
    const Variant& entry(iter.secondRef());
    switch (entry.getType()) {
      DT_UNCOUNTED_CASE:
      case KindOfString:
      case KindOfRef:
        d += entry.toDouble();

      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
        continue;

      case KindOfClass:
        break;
    }
    not_reached();
  }
  return d;
}

Variant HHVM_FUNCTION(array_unshift,
                      VRefParam array,
                      const Variant& var,
                      const Array& args /* = null array */) {
  const auto* cell_array = array->asCell();
  if (UNLIKELY(!isContainer(*cell_array))) {
    raise_warning("%s() expects parameter 1 to be an array, Vector, or Set",
                  __FUNCTION__+2 /* remove the "f_" prefix */);
    return init_null();
  }
  if (cell_array->m_type == KindOfArray) {
    if (array.toArray()->isVectorData()) {
      if (!args.empty()) {
        auto pos_limit = args->iter_end();
        for (ssize_t pos = args->iter_last(); pos != pos_limit;
             pos = args->iter_rewind(pos)) {
          array.wrapped().toArrRef().prepend(args->getValueRef(pos));
        }
      }
      array.wrapped().toArrRef().prepend(var);
    } else {
      {
        Array newArray;
        newArray.append(var);
        if (!args.empty()) {
          auto pos_limit = args->iter_end();
          for (ssize_t pos = args->iter_begin(); pos != pos_limit;
               pos = args->iter_advance(pos)) {
            newArray.append(args->getValueRef(pos));
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
        HHVM_FN(reset)(array);
      }
    }
    return array.toArray().size();
  }
  // Handle collections
  assert(cell_array->m_type == KindOfObject);
  auto* obj = cell_array->m_data.pobj;
  assert(obj->isCollection());
  switch (obj->collectionType()) {
    case CollectionType::Vector: {
      auto* vec = static_cast<c_Vector*>(obj);
      if (!args.empty()) {
        auto pos_limit = args->iter_end();
        for (ssize_t pos = args->iter_last(); pos != pos_limit;
             pos = args->iter_rewind(pos)) {
          vec->addFront(args->getValueRef(pos).asCell());
        }
      }
      vec->addFront(var.asCell());
      return vec->size();
    }
    case CollectionType::Set: {
      auto* st = static_cast<c_Set*>(obj);
      if (!args.empty()) {
        auto pos_limit = args->iter_end();
        for (ssize_t pos = args->iter_last(); pos != pos_limit;
             pos = args->iter_rewind(pos)) {
          st->addFront(args->getValueRef(pos).asCell());
        }
      }
      st->addFront(var.asCell());
      return st->size();
    }
    case CollectionType::Map:
    case CollectionType::Pair:
    case CollectionType::ImmVector:
    case CollectionType::ImmMap:
    case CollectionType::ImmSet:
      break;
  }
  raise_warning("%s() expects parameter 1 to be an array, Vector, or Set",
                __FUNCTION__+2 /* remove the "f_" prefix */);
  return init_null();
}

Variant HHVM_FUNCTION(array_values,
                      const Variant& input) {
  const auto& cell_input = *input.asCell();
  if (!isContainer(cell_input)) {
    raise_warning("array_values() expects parameter 1 to be an array "
                  "or collection");
    return init_null();
  }
  PackedArrayInit ai(getContainerSize(cell_input));
  for (ArrayIter iter(cell_input); iter; ++iter) {
    ai.appendWithRef(iter.secondRefPlus());
  }
  return ai.toVariant();
}

static void walk_func(VRefParam value,
                      const Variant& key,
                      const Variant& userdata,
                      const void *data) {
  CallCtx* ctx = (CallCtx*)data;
  Variant sink;
  int nargs = userdata.isInitialized() ? 3 : 2;
  TypedValue args[3] = { *value->asRef(), *key.asCell(), *userdata.asCell() };
  g_context->invokeFuncFew(sink.asTypedValue(), *ctx, nargs, args);
}

bool HHVM_FUNCTION(array_walk_recursive,
                   VRefParam input,
                   const Variant& funcname,
                   const Variant& userdata /* = null_variant */) {
  if (!input.isArray()) {
    throw_expected_array_exception("array_walk_recursive");
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

bool HHVM_FUNCTION(array_walk,
                   VRefParam input,
                   const Variant& funcname,
                   const Variant& userdata /* = null_variant */) {
  if (!input.isArray()) {
    throw_expected_array_exception("array_walk");
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

Array HHVM_FUNCTION(compact,
                    const Variant& varname,
                    const Array& args /* = null array */) {
  raise_disallowed_dynamic_call("compact should not be called dynamically");
  Array ret = Array::attach(MixedArray::MakeReserve(args.size() + 1));
  VarEnv* v = g_context->getOrCreateVarEnv();
  if (v) {
    compact(v, ret, varname);
    compact(v, ret, args);
  }
  return ret;
}

// __SystemLib\\compact_sl
Array HHVM_FUNCTION(__SystemLib_compact_sl,
                    const Variant& varname,
                    const Array& args /* = null array */) {
  Array ret = Array::attach(MixedArray::MakeReserve(args.size() + 1));
  VarEnv* v = g_context->getOrCreateVarEnv();
  if (v) {
    compact(v, ret, varname);
    compact(v, ret, args);
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

bool HHVM_FUNCTION(shuffle,
                   VRefParam array) {
  if (!array.isArray()) {
    throw_expected_array_exception("shuffle");
    return false;
  }
  array = ArrayUtil::Shuffle(array);
  return true;
}

int64_t HHVM_FUNCTION(count,
                      const Variant& var,
                      int64_t mode /* = 0 */) {
  switch (var.getType()) {
    case KindOfUninit:
    case KindOfNull:
      return 0;

    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfStaticString:
    case KindOfString:
    case KindOfResource:
      return 1;

    case KindOfArray:
      if (mode) {
        const Array& arr_var = var.toCArrRef();
        return php_count_recursive(arr_var);
      }
      return var.getArrayData()->size();

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
      return 1;

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

int64_t HHVM_FUNCTION(sizeof,
                      const Variant& var,
                      int64_t mode /* = 0 */) {
  return HHVM_FN(count)(var, mode);
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
    cellMove(make_tv<KindOfArray>(ad), cell);
  }
  return (ad->*op)();
}

}

Variant HHVM_FUNCTION(each,
                      VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::each,
    Variant::NullInit()
  );
}

Variant HHVM_FUNCTION(current,
                      VRefParam refParam) {
  return iter_op_impl<NoCow>(
    refParam,
    &ArrayData::current,
    false
  );
}

Variant HHVM_FUNCTION(pos,
                      VRefParam refParam) {
  return HHVM_FN(current)(refParam);
}

Variant HHVM_FUNCTION(key,
                      VRefParam refParam) {
  return iter_op_impl<NoCow>(
    refParam,
    &ArrayData::key,
    false
  );
}

Variant HHVM_FUNCTION(next,
                      VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::next,
    false
  );
}

Variant HHVM_FUNCTION(prev,
                      VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::prev,
    false
  );
}

Variant HHVM_FUNCTION(reset,
                      VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::reset,
    false,
    &ArrayData::isHead
  );
}

Variant HHVM_FUNCTION(end,
                      VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::end,
    false,
    &ArrayData::isTail
  );
}

bool HHVM_FUNCTION(in_array,
                   const Variant& needle,
                   const Variant& haystack,
                   bool strict /* = false */) {
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

Variant HHVM_FUNCTION(array_search,
                      const Variant& needle,
                      const Variant& haystack,
                      bool strict /* = false */) {
  const auto& cell_haystack = *haystack.asCell();
  if (UNLIKELY(!isContainer(cell_haystack))) {
    raise_warning("array_search() expects parameter 2 to be an array "
                  "or collection");
    return init_null();
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

Variant HHVM_FUNCTION(range,
                      const Variant& low,
                      const Variant& high,
                      const Variant& step /* = 1 */) {
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

// PHP 5.x does different things when diffing against the same array,
// particularly when the comparison function is outside the norm of
// return -1, 0, 1 specification. To do what PHP 5.x in these cases,
// use the RuntimeOption
#define COMMA ,
#define diff_intersect_body(type,intersect_params,user_setup)   \
  getCheckedArray(array1);                                      \
  if (!arr_array1.size()) return arr_array1;                    \
  Array ret = Array::Create();                                  \
  if (RuntimeOption::EnableZendSorting) {                       \
    getCheckedArray(array2);                                    \
    if (arr_array1.same(arr_array2)) {                          \
      return ret;                                               \
    }                                                           \
  }                                                             \
  user_setup                                                    \
  ret = arr_array1.type(array2, intersect_params);              \
  if (ret.size()) {                                             \
    for (ArrayIter iter(args); iter; ++iter) {                  \
      ret = ret.type(iter.second(), intersect_params);          \
      if (!ret.size()) break;                                   \
    }                                                           \
  }                                                             \
  return ret;

///////////////////////////////////////////////////////////////////////////////
// diff functions

static inline void addToSetHelper(const req::ptr<c_Set>& st,
                                  const Cell c,
                                  TypedValue* strTv,
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

static inline bool checkSetHelper(const req::ptr<c_Set>& st,
                                  const Cell c,
                                  TypedValue* strTv,
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

static void containerValuesToSetHelper(const req::ptr<c_Set>& st,
                                       const Variant& container) {
  Variant strHolder(empty_string_variant());
  TypedValue* strTv = strHolder.asTypedValue();
  for (ArrayIter iter(container); iter; ++iter) {
    auto const& c = *iter.secondRefPlus().asCell();
    addToSetHelper(st, c, strTv, true);
  }
}

static void containerKeysToSetHelper(const req::ptr<c_Set>& st,
                                     const Variant& container) {
  Variant strHolder(empty_string_variant());
  TypedValue* strTv = strHolder.asTypedValue();
  bool isKey = container.asCell()->m_type == KindOfArray;
  for (ArrayIter iter(container); iter; ++iter) {
    addToSetHelper(st, *iter.first().asCell(), strTv, !isKey);
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
    return init_null(); \
  } \
  bool moreThanTwo = !args.empty(); \
  size_t largestSize = getContainerSize(c2); \
  if (UNLIKELY(moreThanTwo)) { \
    int pos = 3; \
    for (ArrayIter argvIter(args); argvIter; ++argvIter, ++pos) { \
      const auto& c = *argvIter.secondRef().asCell(); \
      if (!isContainer(c)) { \
        raise_warning("%s() expects parameter %d to be an array or collection",\
                      __FUNCTION__+2, /* remove the "f_" prefix */ \
                      pos); \
        return init_null(); \
      } \
      size_t sz = getContainerSize(c); \
      if (sz > largestSize) { \
        largestSize = sz; \
      } \
    } \
  } \
  /* If container1 is empty, we can stop here and return the empty array */ \
  if (!getContainerSize(c1)) return empty_array(); \
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

Variant HHVM_FUNCTION(array_diff,
                      const Variant& container1,
                      const Variant& container2,
                      const Array& args /* = null array */) {
  ARRAY_DIFF_PRELUDE()
  // Put all of the values from all the containers (except container1 into a
  // Set. All types aside from integer and string will be cast to string, and
  // we also convert int-like strings to integers.
  auto st = req::make<c_Set>();
  st->reserve(largestSize);
  containerValuesToSetHelper(st, container2);
  if (UNLIKELY(moreThanTwo)) {
    for (ArrayIter argvIter(args); argvIter; ++argvIter) {
      const auto& container = argvIter.secondRef();
      containerValuesToSetHelper(st, container);
    }
  }
  // Loop over container1, only copying over key/value pairs where the value
  // is not present in the Set. When checking if a value is present in the
  // Set, any value that is not an integer or string is cast to a string, and
  // we convert int-like strings to integers.
  Variant strHolder(empty_string_variant());
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

Variant HHVM_FUNCTION(array_diff_key,
                      const Variant& container1,
                      const Variant& container2,
                      const Array& args /* = null array */) {
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
  auto st = req::make<c_Set>();
  st->reserve(largestSize);
  containerKeysToSetHelper(st, container2);
  if (UNLIKELY(moreThanTwo)) {
    for (ArrayIter argvIter(args); argvIter; ++argvIter) {
      const auto& container = argvIter.secondRef();
      containerKeysToSetHelper(st, container);
    }
  }
  // Loop over container1, only copying over key/value pairs where the key is
  // not present in the Set. When checking if a key is present in the Set, any
  // key that is not an integer or string is cast to a string, and we convert
  // int-like strings to integers.
  Variant strHolder(empty_string_variant());
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

Variant HHVM_FUNCTION(array_udiff,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Array& args /* = null array */) {
  diff_intersect_body(diff, false COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = args;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant HHVM_FUNCTION(array_diff_assoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Array& args /* = null array */) {
  diff_intersect_body(diff, true COMMA true,);
}

Variant HHVM_FUNCTION(array_diff_uassoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& key_compare_func,
                      const Array& args /* = null array */) {
  diff_intersect_body(diff, true COMMA true COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = args;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant HHVM_FUNCTION(array_udiff_assoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Array& args /* = null array */) {
  diff_intersect_body(diff, true COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = args;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant HHVM_FUNCTION(array_udiff_uassoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Variant& key_compare_func,
                      const Array& args /* = null array */) {
  diff_intersect_body(diff, true COMMA true COMMA cmp_func COMMA &key_func
                      COMMA cmp_func COMMA &data_func,
                      Variant data_func = data_compare_func;
                      Variant key_func = key_compare_func;
                      Array extra = args;
                      if (!extra.empty()) {
                        extra.prepend(key_func);
                        extra.prepend(data_func);
                        key_func = extra.pop();
                        data_func = extra.pop();
                      });
}

Variant HHVM_FUNCTION(array_diff_ukey,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& key_compare_func,
                      const Array& args /* = null array */) {
  diff_intersect_body(diff, true COMMA false COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = args;
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
    (TypedValue*)req::malloc(count * sizeof(TypedValue));
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

static inline void addToIntersectMapHelper(const req::ptr<c_Map>& mp,
                                           const Cell c,
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

static inline void updateIntersectMapHelper(const req::ptr<c_Map>& mp,
                                            const Cell c,
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

static void containerValuesIntersectHelper(const req::ptr<c_Set>& st,
                                           TypedValue* containers,
                                           int count) {
  assert(count >= 2);
  auto mp = req::make<c_Map>();
  Variant strHolder(empty_string_variant());
  TypedValue* strTv = strHolder.asTypedValue();
  TypedValue intOneTv = make_tv<KindOfInt64>(1);
  for (ArrayIter iter(tvAsCVarRef(&containers[0])); iter; ++iter) {
    const auto& c = *iter.secondRefPlus().asCell();
    // For each value v in containers[0], we add the key/value pair (v, 1)
    // to the map. If a value (after various conversions) occurs more than
    // once in the container, we'll simply overwrite the old entry and that's
    // fine.
    addToIntersectMapHelper(mp, c, &intOneTv, strTv, true);
  }
  for (int pos = 1; pos < count; ++pos) {
    for (ArrayIter iter(tvAsCVarRef(&containers[pos])); iter; ++iter) {
      const auto& c = *iter.secondRefPlus().asCell();
      // We check if the value is present as a key in the map. If an entry
      // exists and its value equals pos, we increment it, otherwise we do
      // nothing. This is essential so that we don't accidentally double-count
      // a key (after various conversions) that occurs in the container more
      // than once.
      updateIntersectMapHelper(mp, c, pos, strTv, true);
    }
  }
  for (ArrayIter iter(mp.get()); iter; ++iter) {
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

static void containerKeysIntersectHelper(const req::ptr<c_Set>& st,
                                         TypedValue* containers,
                                         int count) {
  assert(count >= 2);
  auto mp = req::make<c_Map>();
  Variant strHolder(empty_string_variant());
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
  for (ArrayIter iter(mp.get()); iter; ++iter) {
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
    return init_null(); \
  } \
  bool moreThanTwo = !args.empty(); \
  /* Keep track of which input container was the smallest (excluding \
     container1) */ \
  int smallestPos = 0; \
  size_t smallestSize = getContainerSize(c2); \
  if (UNLIKELY(moreThanTwo)) { \
    int pos = 1; \
    for (ArrayIter argvIter(args); argvIter; ++argvIter, ++pos) { \
      const auto& c = *argvIter.secondRef().asCell(); \
      if (!isContainer(c)) { \
        raise_warning("%s() expects parameter %d to be an array or collection",\
                      __FUNCTION__+2, /* remove the "f_" prefix */ \
                      pos+2); \
        return init_null(); \
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
  if (!getContainerSize(c1) || !smallestSize) return empty_array(); \
  Array ret = Array::Create();

Variant HHVM_FUNCTION(array_intersect,
                      const Variant& container1,
                      const Variant& container2,
                      const Array& args /* = null array */) {
  ARRAY_INTERSECT_PRELUDE()
  // Build up a Set containing the values that are present in all the
  // containers (except container1)
  auto st = req::make<c_Set>();
  if (LIKELY(!moreThanTwo)) {
    // There is only one container (not counting container1) so we can
    // just call containerValuesToSetHelper() to build the Set.
    containerValuesToSetHelper(st, container2);
  } else {
    // We're dealing with three or more containers. Copy all of the containers
    // (except the first) into a TypedValue array.
    int count = args.size() + 1;
    TypedValue* containers =
      makeContainerListHelper(container2, args, count, smallestPos);
    SCOPE_EXIT { req::free(containers); };
    // Build a Set of the values that were present in all of the containers
    containerValuesIntersectHelper(st, containers, count);
  }
  // Loop over container1, only copying over key/value pairs where the value
  // is present in the Set. When checking if a value is present in the Set,
  // any value that is not an integer or string is cast to a string, and we
  // convert int-like strings to integers.
  Variant strHolder(empty_string_variant());
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

Variant HHVM_FUNCTION(array_intersect_key,
                      const Variant& container1,
                      const Variant& container2,
                      const Array& args /* = null array */) {
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
  auto st = req::make<c_Set>();
  if (LIKELY(!moreThanTwo)) {
    // There is only one container (not counting container1) so we can just
    // call containerKeysToSetHelper() to build the Set.
    containerKeysToSetHelper(st, container2);
  } else {
    // We're dealing with three or more containers. Copy all of the containers
    // (except the first) into a TypedValue array.
    int count = args.size() + 1;
    TypedValue* containers =
      makeContainerListHelper(container2, args, count, smallestPos);
    SCOPE_EXIT { req::free(containers); };
    // Build a Set of the keys that were present in all of the containers
    containerKeysIntersectHelper(st, containers, count);
  }
  // Loop over container1, only copying over key/value pairs where the key
  // is present in the Set. When checking if a key is present in the Set,
  // any value that is not an integer or string is cast to a string, and we
  // convert int-like strings to integers.
  Variant strHolder(empty_string_variant());
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

Variant HHVM_FUNCTION(array_uintersect,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Array& args /* = null array */) {
  diff_intersect_body(intersect, false COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = args;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant HHVM_FUNCTION(array_intersect_assoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Array& args /* = null array */) {
  diff_intersect_body(intersect, true COMMA true,);
}

Variant HHVM_FUNCTION(array_intersect_uassoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& key_compare_func,
                      const Array& args /* = null array */) {
  diff_intersect_body(intersect, true COMMA true COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = args;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant HHVM_FUNCTION(array_uintersect_assoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Array& args /* = null array */) {
  diff_intersect_body(intersect, true COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = args;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant HHVM_FUNCTION(array_uintersect_uassoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Variant& key_compare_func,
                      const Array& args /* = null array */) {
  diff_intersect_body(intersect, true COMMA true COMMA cmp_func COMMA &key_func
                      COMMA cmp_func COMMA &data_func,
                      Variant data_func = data_compare_func;
                      Variant key_func = key_compare_func;
                      Array extra = args;
                      if (!extra.empty()) {
                        extra.prepend(key_func);
                        extra.prepend(data_func);
                        key_func = extra.pop();
                        data_func = extra.pop();
                      });
}

Variant HHVM_FUNCTION(array_intersect_ukey,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& key_compare_func,
                      const Array& args /* = null array */) {
  diff_intersect_body(intersect, true COMMA false COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = args;
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

namespace {
class ArraySortTmp {
 public:
  explicit ArraySortTmp(Array& arr, SortFunction sf) : m_arr(arr) {
    m_ad = arr.get()->escalateForSort(sf);
    assert(m_ad == arr.get() || m_ad->hasExactlyOneRef());
  }
  ~ArraySortTmp() {
    if (m_ad != m_arr.get()) {
      m_arr = Array::attach(m_ad);
    }
  }
  ArrayData* operator->() { return m_ad; }
 private:
  Array& m_arr;
  ArrayData* m_ad;
};
}

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
    SortFunction sf = getSortFunction(SORTFUNC_SORT, ascending);
    ArraySortTmp ast(arr_array, sf);
    ast->sort(sort_flags, ascending);
    return true;
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->isCollection() &&
        obj->collectionType() == CollectionType::Vector) {
      c_Vector* vec = static_cast<c_Vector*>(obj);
      vec->sort(sort_flags, ascending);
      return true;
    }
    // other collections are not supported:
    //  - Maps and Sets require associative sort
    //  - Immutable collections are not to be modified
  }
  throw_expected_array_or_collection_exception(ascending ? "sort" : "rsort");
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
    SortFunction sf = getSortFunction(SORTFUNC_ASORT, ascending);
    ArraySortTmp ast(arr_array, sf);
    ast->asort(sort_flags, ascending);
    return true;
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->isCollection()) {
      auto type = obj->collectionType();
      if (type == CollectionType::Map || type == CollectionType::Set) {
        HashCollection* hc = static_cast<HashCollection*>(obj);
        hc->asort(sort_flags, ascending);
        return true;
      }
    }
  }
  throw_expected_array_or_collection_exception(ascending ? "asort" : "arsort");
  return false;
}

static bool
php_ksort(VRefParam container, int sort_flags, bool ascending,
          bool use_collator) {
  if (container.isArray()) {
    Array& arr_array = container.wrapped().toArrRef();
    if (use_collator && sort_flags != SORT_LOCALE_STRING) {
      UCollator *coll = s_collator->getCollator();
      if (coll) {
        Intl::IntlError &errcode = s_collator->getErrorRef();
        return collator_ksort(container, sort_flags, ascending,
                              coll, &errcode);
      }
    }
    SortFunction sf = getSortFunction(SORTFUNC_KRSORT, ascending);
    ArraySortTmp ast(arr_array, sf);
    ast->ksort(sort_flags, ascending);
    return true;
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->isCollection()) {
      auto type = obj->collectionType();
      if (type == CollectionType::Map || type == CollectionType::Set) {
        HashCollection* hc = static_cast<HashCollection*>(obj);
        hc->ksort(sort_flags, ascending);
        return true;
      }
    }
  }
  throw_expected_array_or_collection_exception(ascending ? "ksort" : "krsort");
  return false;
}

bool HHVM_FUNCTION(sort,
                  VRefParam array,
                  int sort_flags /* = 0 */) {
  bool use_collator = RuntimeOption::EnableZendSorting;
  return php_sort(array, sort_flags, true, use_collator);
}

bool HHVM_FUNCTION(rsort,
                   VRefParam array,
                   int sort_flags /* = 0 */) {
  bool use_collator = RuntimeOption::EnableZendSorting;
  return php_sort(array, sort_flags, false, use_collator);
}

bool HHVM_FUNCTION(asort,
                   VRefParam array,
                   int sort_flags /* = 0 */) {
  bool use_collator = RuntimeOption::EnableZendSorting;
  return php_asort(array, sort_flags, true, use_collator);
}

bool HHVM_FUNCTION(arsort,
                   VRefParam array,
                   int sort_flags /* = 0 */) {
  bool use_collator = RuntimeOption::EnableZendSorting;
  return php_asort(array, sort_flags, false, use_collator);
}

bool HHVM_FUNCTION(ksort,
                   VRefParam array,
                   int sort_flags /* = 0 */) {
  bool use_collator = RuntimeOption::EnableZendSorting;
  return php_ksort(array, sort_flags, true, use_collator);
}

bool HHVM_FUNCTION(krsort,
                   VRefParam array,
                   int sort_flags /* = 0 */) {
  bool use_collator = RuntimeOption::EnableZendSorting;
  return php_ksort(array, sort_flags, false, use_collator);
}

// NOTE: PHP's implementation of natsort and natcasesort accepts ArrayAccess
// objects as well, which does not make much sense, and which is not supported
// here.

Variant HHVM_FUNCTION(natsort,
                      VRefParam array) {
  return php_asort(array, SORT_NATURAL, true, false);
}

Variant HHVM_FUNCTION(natcasesort,
                      VRefParam array) {
  return php_asort(array, SORT_NATURAL_CASE, true, false);
}

bool HHVM_FUNCTION(usort,
                   VRefParam container,
                   const Variant& cmp_function) {
  if (container.isArray()) {
    Array& arr_array = container.wrapped().toArrRef();
    if (RuntimeOption::EnableZendSorting) {
      arr_array.sort(cmp_func, false, true, &cmp_function);
      return true;
    } else {
      ArraySortTmp ast(arr_array, SORTFUNC_USORT);
      return ast->usort(cmp_function);
    }
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->isCollection()) {
      if (obj->collectionType() == CollectionType::Vector) {
        c_Vector* vec = static_cast<c_Vector*>(obj);
        return vec->usort(cmp_function);
      }
    }
    // other collections are not supported:
    //  - Maps and Sets require associative sort
    //  - Immutable collections are not to be modified
  }
  throw_expected_array_or_collection_exception("usort");
  return false;
}

bool HHVM_FUNCTION(uasort,
                   VRefParam container,
                   const Variant& cmp_function) {
  if (container.isArray()) {
    Array& arr_array = container.wrapped().toArrRef();
    if (RuntimeOption::EnableZendSorting) {
      arr_array.sort(cmp_func, false, false, &cmp_function);
      return true;
    } else {
      ArraySortTmp ast(arr_array, SORTFUNC_UASORT);
      return ast->uasort(cmp_function);
    }
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->isCollection()) {
      auto type = obj->collectionType();
      if (type == CollectionType::Map || type == CollectionType::Set) {
        HashCollection* hc = static_cast<HashCollection*>(obj);
        return hc->uasort(cmp_function);
      }
    }
    // other collections are not supported:
    //  - Vectors require a non-associative sort
    //  - Immutable collections are not to be modified
  }
  throw_expected_array_or_collection_exception("uasort");
  return false;
}

bool HHVM_FUNCTION(uksort,
                   VRefParam container,
                   const Variant& cmp_function) {
  if (container.isArray()) {
    Array& arr_array = container.wrapped().toArrRef();
    ArraySortTmp ast(arr_array, SORTFUNC_UKSORT);
    return ast->uksort(cmp_function);
  }
  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->isCollection()) {
      auto type = obj->collectionType();
      if (type == CollectionType::Map || type == CollectionType::Set) {
        HashCollection* hc = static_cast<HashCollection*>(obj);
        return hc->uksort(cmp_function);
      }
    }
    // other collections are not supported:
    //  - Vectors require a non-associative sort
    //  - Immutable collections are not to be modified
  }
  throw_expected_array_or_collection_exception("uksort");
  return false;
}

Variant HHVM_FUNCTION(array_unique,
                      const Variant& array,
                      int sort_flags /* = 2 */) {
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

String HHVM_FUNCTION(i18n_loc_get_default) {
  return s_collator->getLocale();
}

bool HHVM_FUNCTION(i18n_loc_set_default,
                   const String& locale) {
  return s_collator->setLocale(locale);
}

bool HHVM_FUNCTION(i18n_loc_set_attribute,
                   int64_t attr,
                   int64_t val) {
  return s_collator->setAttribute(attr, val);
}

bool HHVM_FUNCTION(i18n_loc_set_strength,
                   int64_t strength) {
  return s_collator->setStrength(strength);
}

Variant HHVM_FUNCTION(i18n_loc_get_error_code) {
  return s_collator->getErrorCode();
}

Variant HHVM_FUNCTION(hphp_array_idx,
                      const Variant& search,
                      const Variant& key,
                      const Variant& def) {
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

static Array::PFUNC_CMP get_cmp_func(int sort_flags, bool ascending) {
  switch (sort_flags) {
  case SORT_NATURAL:
    return ascending ?
      Array::SortNaturalAscending : Array::SortNaturalDescending;
  case SORT_NATURAL_CASE:
    return ascending ?
      Array::SortNaturalCaseAscending: Array::SortNaturalCaseDescending;
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

TypedValue* HHVM_FN(array_multisort)(ActRec* ar) {
  TypedValue* tv = getArg(ar, 0);
  if (tv == nullptr || !tvAsVariant(tv).isArray()) {
    throw_expected_array_exception("array_multisort");
    return arReturn(ar, false);
  }

  std::vector<Array::SortData> data;
  std::vector<Array> arrays;
  arrays.reserve(ar->numArgs()); // so no resize would happen

  Array::SortData sd;
  sd.original = &tvAsVariant(tv);
  arrays.push_back(Array(sd.original->getArrayData()));
  sd.array = &arrays.back();
  sd.by_key = false;

  int sort_flags = SORT_REGULAR;
  bool ascending = true;
  for (int i = 1; i < ar->numArgs(); i++) {
    tv = getArg(ar, i);
    if (tvAsVariant(tv).isArray()) {
      sd.cmp_func = get_cmp_func(sort_flags, ascending);
      data.push_back(sd);

      sort_flags = SORT_REGULAR;
      ascending = true;

      sd.original = &tvAsVariant(tv);
      arrays.push_back(Array(sd.original->getArrayData()));
      sd.array = &arrays.back();
    } else {
      int n = toInt32(getArg<KindOfInt64>(ar, i));
      if (n == SORT_ASC) {
      } else if (n == SORT_DESC) {
        ascending = false;
      } else {
        sort_flags = n;
      }
    }
  }

  sd.cmp_func = get_cmp_func(sort_flags, ascending);
  data.push_back(sd);

  return arReturn(ar, Array::MultiSort(data, true));
}

///////////////////////////////////////////////////////////////////////////////

#define REGISTER_CONSTANT(name)                                                \
  Native::registerConstant<KindOfInt64>(s_##name.get(), k_##name)              \

#define REGISTER_CONSTANT_VALUE(option, value)                                 \
  Native::registerConstant<KindOfInt64>(makeStaticString("ARRAY_" #option),    \
                                       (value));

class ArrayExtension final : public Extension {
public:
  ArrayExtension() : Extension("array") {}
  void moduleInit() override {
    REGISTER_CONSTANT(UCOL_DEFAULT);
    REGISTER_CONSTANT(UCOL_PRIMARY);
    REGISTER_CONSTANT(UCOL_SECONDARY);
    REGISTER_CONSTANT(UCOL_TERTIARY);
    REGISTER_CONSTANT(UCOL_DEFAULT_STRENGTH);
    REGISTER_CONSTANT(UCOL_QUATERNARY);
    REGISTER_CONSTANT(UCOL_IDENTICAL);
    REGISTER_CONSTANT(UCOL_OFF);
    REGISTER_CONSTANT(UCOL_ON);
    REGISTER_CONSTANT(UCOL_SHIFTED);
    REGISTER_CONSTANT(UCOL_NON_IGNORABLE);
    REGISTER_CONSTANT(UCOL_LOWER_FIRST);
    REGISTER_CONSTANT(UCOL_UPPER_FIRST);
    REGISTER_CONSTANT(UCOL_FRENCH_COLLATION);
    REGISTER_CONSTANT(UCOL_ALTERNATE_HANDLING);
    REGISTER_CONSTANT(UCOL_CASE_FIRST);
    REGISTER_CONSTANT(UCOL_CASE_LEVEL);
    REGISTER_CONSTANT(UCOL_NORMALIZATION_MODE);
    REGISTER_CONSTANT(UCOL_STRENGTH);
    REGISTER_CONSTANT(UCOL_HIRAGANA_QUATERNARY_MODE);
    REGISTER_CONSTANT(UCOL_NUMERIC_COLLATION);

    REGISTER_CONSTANT_VALUE(FILTER_USE_BOTH, 1);
    REGISTER_CONSTANT_VALUE(FILTER_USE_KEY,  2);

    HHVM_FE(array_change_key_case);
    HHVM_FE(array_chunk);
    HHVM_FE(array_column);
    HHVM_FE(array_combine);
    HHVM_FE(array_count_values);
    HHVM_FE(array_fill_keys);
    HHVM_FE(array_fill);
    HHVM_FE(array_flip);
    HHVM_FE(array_key_exists);
    HHVM_FE(key_exists);
    HHVM_FE(array_keys);
    HHVM_FALIAS(__SystemLib\\array_map, array_map);
    HHVM_FE(array_merge_recursive);
    HHVM_FE(array_merge);
    HHVM_FE(array_replace_recursive);
    HHVM_FE(array_replace);
    HHVM_FE(array_pad);
    HHVM_FE(array_pop);
    HHVM_FE(array_product);
    HHVM_FE(array_push);
    HHVM_FE(array_rand);
    HHVM_FE(array_reverse);
    HHVM_FE(array_search);
    HHVM_FE(array_shift);
    HHVM_FE(array_slice);
    HHVM_FE(array_splice);
    HHVM_FE(array_sum);
    HHVM_FE(array_unique);
    HHVM_FE(array_unshift);
    HHVM_FE(array_values);
    HHVM_FE(array_walk_recursive);
    HHVM_FE(array_walk);
    HHVM_FE(compact);
    HHVM_FALIAS(__SystemLib\\compact_sl, __SystemLib_compact_sl);
    HHVM_FE(shuffle);
    HHVM_FE(count);
    HHVM_FE(sizeof);
    HHVM_FE(each);
    HHVM_FE(current);
    HHVM_FE(next);
    HHVM_FE(pos);
    HHVM_FE(prev);
    HHVM_FE(reset);
    HHVM_FE(end);
    HHVM_FE(key);
    HHVM_FE(in_array);
    HHVM_FE(range);
    HHVM_FE(array_diff);
    HHVM_FE(array_udiff);
    HHVM_FE(array_diff_assoc);
    HHVM_FE(array_diff_uassoc);
    HHVM_FE(array_udiff_assoc);
    HHVM_FE(array_udiff_uassoc);
    HHVM_FE(array_diff_key);
    HHVM_FE(array_diff_ukey);
    HHVM_FE(array_intersect);
    HHVM_FE(array_uintersect);
    HHVM_FE(array_intersect_assoc);
    HHVM_FE(array_intersect_uassoc);
    HHVM_FE(array_uintersect_assoc);
    HHVM_FE(array_uintersect_uassoc);
    HHVM_FE(array_intersect_key);
    HHVM_FE(array_intersect_ukey);
    HHVM_FE(sort);
    HHVM_FE(rsort);
    HHVM_FE(asort);
    HHVM_FE(arsort);
    HHVM_FE(ksort);
    HHVM_FE(krsort);
    HHVM_FE(usort);
    HHVM_FE(uasort);
    HHVM_FE(uksort);
    HHVM_FE(natsort);
    HHVM_FE(natcasesort);
    HHVM_FE(i18n_loc_get_default);
    HHVM_FE(i18n_loc_set_default);
    HHVM_FE(i18n_loc_set_attribute);
    HHVM_FE(i18n_loc_set_strength);
    HHVM_FE(i18n_loc_get_error_code);
    HHVM_FE(hphp_array_idx);
    HHVM_FE(array_multisort);

    loadSystemlib();
  }
} s_array_extension;

#undef REGISTER_CONSTANT

}
