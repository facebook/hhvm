/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/double-to-int64.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/zend-collator.h"
#include "hphp/runtime/base/zend-sort.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/util/logger.h"

#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define SORT_DESC               3
#define SORT_ASC                4

const StaticString s_count("count");

enum class CaseMode {
  LOWER = 0,
  UPPER = 1,
};

TypedValue HHVM_FUNCTION(array_change_key_case,
                         const Variant& input,
                         int64_t case_ /* = 0 */) {
  getCheckedContainer(input);
  return tvReturn(ArrayUtil::ChangeKeyCase(arr_input,
                                           (CaseMode)case_ == CaseMode::LOWER));
}

TypedValue HHVM_FUNCTION(array_chunk,
                         const Variant& input,
                         int chunkSize,
                         bool preserve_keys /* = false */) {
  const auto& cellInput = *input.asCell();
  if (UNLIKELY(!isContainer(cellInput))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection as argument 1", __FUNCTION__+2);
    return make_tv<KindOfNull>();
  }

  if (chunkSize < 1) {
    throw_invalid_argument("size: %d", chunkSize);
    return make_tv<KindOfNull>();
  }

  auto const retSize = (getContainerSize(cellInput) / chunkSize) + 1;
  PackedArrayInit ret(retSize);
  Array chunk;
  int current = 0;
  for (ArrayIter iter(cellInput); iter; ++iter) {
    if (preserve_keys) {
      chunk.setWithRef(iter.first(), iter.secondValPlus(), true);
    } else {
      chunk.appendWithRef(iter.secondValPlus());
    }
    if ((++current % chunkSize) == 0) {
      ret.append(chunk);
      chunk.clear();
    }
  }
  if (!chunk.empty()) {
    ret.append(chunk);
  }

  return tvReturn(ret.toVariant());
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

TypedValue HHVM_FUNCTION(array_column,
                         const Variant& input,
                         const Variant& val_key,
                         const Variant& idx_key /* = uninit_variant */) {

  getCheckedContainer(input);
  Variant val = val_key, idx = idx_key;
  if (!array_column_coerce_key(val, "column") ||
      !array_column_coerce_key(idx, "index")) {
    return make_tv<KindOfBoolean>(false);
  }
  ArrayInit ret(arr_input.size(), ArrayInit::Map{});
  for (ArrayIter it(arr_input); it; ++it) {
    Array sub;
    if (UNLIKELY(RuntimeOption::PHP7_Builtins && it.second().isObject())) {
      sub = it.second().toObject().toArray();
    } else if (it.second().isArray()) {
      sub = it.second().toArray();
    } else {
      continue;
    }

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
      ret.setUnknownKey(sub[idx].toString(), elem);
    } else {
      ret.setUnknownKey(sub[idx], elem);
    }
  }
  return tvReturn(ret.toVariant());
}

TypedValue HHVM_FUNCTION(array_combine,
                         const Variant& keys,
                         const Variant& values) {
  const auto& cell_keys = *keys.asCell();
  const auto& cell_values = *values.asCell();
  if (UNLIKELY(!isContainer(cell_keys) || !isContainer(cell_values))) {
    raise_warning("Invalid operand type was used: array_combine expects "
                  "arrays or collections");
    return make_tv<KindOfNull>();
  }
  auto keys_size = getContainerSize(cell_keys);
  if (UNLIKELY(keys_size != getContainerSize(cell_values))) {
    raise_warning("array_combine(): Both parameters should have an equal "
                  "number of elements");
    return make_tv<KindOfBoolean>(false);
  }
  Array ret = Array::attach(MixedArray::MakeReserveMixed(keys_size));
  for (ArrayIter iter1(cell_keys), iter2(cell_values);
       iter1; ++iter1, ++iter2) {
    auto const key = tvToCell(iter1.secondRvalPlus());
    if (key.type() == KindOfInt64 || isStringType(key.type())) {
      ret.setWithRef(iter1.secondValPlus(),
                     iter2.secondValPlus());
    } else {
      ret.setWithRef(String::attach(tvCastToString(key.tv())),
                     iter2.secondValPlus());
    }
  }
  return tvReturn(std::move(ret));
}

TypedValue HHVM_FUNCTION(array_count_values,
                         ArrayArg input) {
  return tvReturn(ArrayUtil::CountValues(ArrNR(input.get())));
}

TypedValue HHVM_FUNCTION(array_fill_keys,
                         const Variant& keys,
                         const Variant& value) {
  SuppressHackArrCompatNotices suppress;

  folly::Optional<ArrayInit> ai;
  auto ok = IterateV(
    *keys.asCell(),
    [&](ArrayData* adata) {
      ai.emplace(adata->size(), ArrayInit::Mixed{});
    },
    [&](TypedValue v) {
      auto const inner = tvToCell(v);
      if (isIntType(inner.m_type) || isStringType(inner.m_type)) {
        ai->setUnknownKey(VarNR(inner), value);
      } else {
        raise_hack_strict(RuntimeOption::StrictArrayFillKeys,
                          "strict_array_fill_keys",
                          "keys must be ints or strings");
        ai->setUnknownKey(String::attach(tvCastToString(v)), value);
      }
    },
    [&](ObjectData* coll) {
      if (coll->collectionType() == CollectionType::Pair) {
        ai.emplace(2, ArrayInit::Mixed{});
      }
    }
  );

  if (!ok) {
    raise_warning("Invalid operand type was used: array_fill_keys expects "
                  "an array or collection");
    return make_tv<KindOfNull>();
  }
  assert(ai.hasValue());
  return tvReturn(ai->toVariant());
}

TypedValue HHVM_FUNCTION(array_fill,
                         int start_index,
                         int num,
                         const Variant& value) {
  if (num < 0) {
    throw_invalid_argument("Number of elements can't be negative");
    return make_tv<KindOfBoolean>(false);
  } else if (num == 0) {
    return tvReturn(empty_array());
  }

  if (start_index == 0) {
    PackedArrayInit pai(num, CheckAllocation{});
    for (size_t k = 0; k < num; k++) {
      pai.append(value);
    }
    return tvReturn(pai.toVariant());
  } else {
    ArrayInit ret(num, ArrayInit::Mixed{}, CheckAllocation{});
    ret.set(start_index, value);
    for (int i = num - 1; i > 0; i--) {
      ret.append(value);
    }
    return tvReturn(ret.toVariant());
  }
}

TypedValue HHVM_FUNCTION(array_flip,
                         const Variant& trans) {
  SuppressHackArrCompatNotices suppress;

  auto const& transCell = *trans.asCell();
  if (UNLIKELY(!isContainer(transCell))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection", __FUNCTION__+2);
    return make_tv<KindOfNull>();
  }

  ArrayInit ret(getContainerSize(transCell), ArrayInit::Mixed{});
  for (ArrayIter iter(transCell); iter; ++iter) {
    auto const inner = tvToCell(iter.secondRvalPlus());
    if (inner.type() == KindOfInt64 || isStringType(inner.type())) {
      ret.setUnknownKey(VarNR(inner.tv()), iter.first());
    } else {
      raise_warning("Can only flip STRING and INTEGER values!");
    }
  }
  return tvReturn(ret.toVariant());
}

bool HHVM_FUNCTION(array_key_exists,
                   const Variant& key,
                   const Variant& search) {
  const ArrayData *ad;

  auto const searchCell = search.asCell();
  if (LIKELY(isArrayLikeType(searchCell->m_type))) {
    ad = searchCell->m_data.parr;
  } else if (searchCell->m_type == KindOfObject) {
    ObjectData* obj = searchCell->m_data.pobj;
    if (obj->isCollection()) {
      return collections::contains(obj, key);
    }
    return HHVM_FN(array_key_exists)(key, search.toArray());
  } else {
    throw_bad_type_exception("array_key_exists expects an array or an object; "
                             "false returned.");
    return false;
  }

  auto const cell = key.asCell();

  switch (cell->m_type) {
    case KindOfUninit:
    case KindOfNull:
      if (RuntimeOption::EvalHackArrCompatNotices && ad->useWeakKeys()) {
        raiseHackArrCompatImplicitArrayKey(cell);
      }
      return ad->useWeakKeys() && ad->exists(staticEmptyString());

    case KindOfBoolean:
    case KindOfDouble:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
      if (!ad->useWeakKeys()) throwInvalidArrayKeyException(cell, ad);
      if (RuntimeOption::EvalHackArrCompatNotices) {
        raiseHackArrCompatImplicitArrayKey(cell);
      }
      raise_warning("Array key should be either a string or an integer");
      return false;

    case KindOfPersistentString:
    case KindOfString: {
      int64_t n = 0;
      if (ad->convertKey(cell->m_data.pstr, n)) {
        return ad->exists(n);
      }
      return ad->exists(StrNR(cell->m_data.pstr));
    }
    case KindOfInt64:
      return ad->exists(cell->m_data.num);
    case KindOfRef:
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
    Array ai = Array::attach(PackedArray::MakeReserve(0));
    for (ArrayIter iter(cell_input); iter; ++iter) {
      if ((strict &&
           tvSame(iter.secondValPlus(), *search_value.asTypedValue())) ||
          (!strict &&
           tvEqual(iter.secondValPlus(), *search_value.asTypedValue()))) {
        ai.append(iter.first());
      }
    }
    return ai;
  }
}

static
Variant HHVM_FUNCTION(array_keys, int64_t argc,
                                  const Variant& input,
                                  const Variant& search_value /*=null*/,
                                  bool strict /*=false*/) {
  return array_keys_helper(
    input,
    argc < 2 ? uninit_variant : search_value,
    strict
  );
}

static bool couldRecur(const Variant& v, const ArrayData* arr) {
  return v.isReferenced() ||
    arr->kind() == ArrayData::kGlobalsKind ||
    arr->kind() == ArrayData::kProxyKind;
}

static void php_array_merge_recursive(PointerSet &seen, bool check,
                                      Array &arr1, const Array& arr2) {
  auto const arr1_ptr = (void*)arr1.get();
  if (check) {
    if (seen.find(arr1_ptr) != seen.end()) {
      raise_warning("array_merge_recursive(): recursion detected");
      return;
    }
    seen.insert(arr1_ptr);
  }

  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key(iter.first());
    if (key.isNumeric()) {
      arr1.appendWithRef(iter.secondVal());
    } else if (arr1.exists(key, true)) {
      // There is no need to do toKey() conversion, for a key that is already
      // in the array.
      Variant &v = arr1.lvalAt(key, AccessFlags::Key);
      auto subarr1 = v.toArray().toPHPArray();
      php_array_merge_recursive(
        seen, couldRecur(v, subarr1.get()), subarr1,
        Array::attach(tvCastToArrayLike(iter.secondVal()))
      );
      v.unset(); // avoid contamination of the value that was strongly bound
      v = subarr1;
    } else {
      arr1.setWithRef(key, iter.secondVal(), true);
    }
  }

  if (check) {
    seen.erase(arr1_ptr);
  }
}

TypedValue HHVM_FUNCTION(array_map,
                         const Variant& callback,
                         const Variant& arr1,
                         const Array& _argv) {
  VMRegGuard _;
  CallCtx ctx;
  ctx.func = nullptr;
  if (!callback.isNull()) {
    CallerFrame cf;
    vm_decode_function(callback, cf(), false, ctx);
  }
  const auto& cell_arr1 = *arr1.asCell();
  if (UNLIKELY(!isContainer(cell_arr1))) {
    raise_warning("array_map(): Argument #2 should be an array or collection");
    return make_tv<KindOfNull>();
  }
  if (LIKELY(_argv.empty())) {
    // Handle the common case where the caller passed two
    // params (a callback and a container)
    if (!ctx.func) {
      if (isArrayLikeType(cell_arr1.m_type)) {
        return tvReturn(arr1);
      } else {
        return tvReturn(arr1.toArray());
      }
    }
    ArrayInit ret(getContainerSize(cell_arr1), ArrayInit::Map{});
    bool keyConverted = isArrayLikeType(cell_arr1.m_type);
    if (!keyConverted) {
      auto col_type = cell_arr1.m_data.pobj->collectionType();
      keyConverted = !collectionAllowsIntStringKeys(col_type);
    }
    for (ArrayIter iter(arr1); iter; ++iter) {
      auto result = Variant::attach(
        g_context->invokeFuncFew(
          ctx, 1, tvToCell(iter.secondRvalPlus().tv_ptr())
        )
      );
      // if keyConverted is false, it's possible that ret will have fewer
      // elements than cell_arr1; keys int(1) and string('1') may both be
      // present
      ret.add(iter.first(), result, keyConverted);
    }
    return tvReturn(ret.toVariant());
  }

  // Handle the uncommon case where the caller passed a callback
  // and two or more containers
  req::vector<ArrayIter> iters;
  iters.reserve(_argv.size() + 1);
  size_t maxLen = getContainerSize(cell_arr1);
  iters.emplace_back(cell_arr1);
  for (ArrayIter it(_argv); it; ++it) {
    auto const c = tvToCell(it.secondValPlus());
    if (UNLIKELY(!isContainer(c))) {
      raise_warning("array_map(): Argument #%d should be an array or "
                    "collection", (int)(iters.size() + 2));
      iters.emplace_back(Array::attach(tvCastToArrayLike(c)));
    } else {
      iters.emplace_back(c);
      size_t len = getContainerSize(c);
      if (len > maxLen) maxLen = len;
    }
  }
  PackedArrayInit ret_ai(maxLen);
  for (size_t k = 0; k < maxLen; k++) {
    PackedArrayInit params_ai(iters.size());
    for (auto& iter : iters) {
      if (iter) {
        params_ai.append(iter.secondValPlus());
        ++iter;
      } else {
        params_ai.append(init_null_variant);
      }
    }
    Array params = params_ai.toArray();
    if (ctx.func) {
      auto result = Variant::attach(
        g_context->invokeFunc(ctx.func, params, ctx.this_,
                              ctx.cls, nullptr, ctx.invName)
      );
      ret_ai.append(result);
    } else {
      ret_ai.append(params);
    }
  }
  return tvReturn(ret_ai.toVariant());
}

TypedValue HHVM_FUNCTION(array_merge,
                         int64_t numArgs,
                         const Variant& array1,
                         const Variant& array2 /* = uninit_variant */,
                         const Array& args /* = null array */) {
  getCheckedContainer(array1);
  Array ret = Array::Create();
  ret.merge(arr_array1);

  if (UNLIKELY(numArgs < 2)) return tvReturn(std::move(ret));

  getCheckedArray(array2);
  ret.merge(arr_array2);

  for (ArrayIter iter(args); iter; ++iter) {
    Variant v = iter.second();
    if (!v.isArray()) {
      throw_expected_array_exception("array_merge");
      return make_tv<KindOfNull>();
    }
    const Array& arr_v = v.asCArrRef();
    ret.merge(arr_v);
  }
  return tvReturn(std::move(ret));
}

TypedValue HHVM_FUNCTION(array_merge_recursive,
                         int64_t numArgs,
                         const Variant& array1,
                         const Variant& array2 /* = uninit_variant */,
                         const Array& args /* = null array */) {
  getCheckedArray(array1);
  auto in1 = array1.asCArrRef();
  Array ret = Array::Create();
  PointerSet seen;
  php_array_merge_recursive(seen, false, ret, arr_array1);
  assert(seen.empty());

  if (UNLIKELY(numArgs < 2)) return tvReturn(std::move(ret));

  getCheckedArray(array2);
  php_array_merge_recursive(seen, false, ret, arr_array2);
  assert(seen.empty());

  for (ArrayIter iter(args); iter; ++iter) {
    Variant v = iter.second();
    if (!v.isArray()) {
      throw_expected_array_exception("array_merge_recursive");
      return make_tv<KindOfNull>();
    }
    const Array& arr_v = v.asCArrRef();
    php_array_merge_recursive(seen, false, ret, arr_v);
    assert(seen.empty());
  }
  return tvReturn(std::move(ret));
}

static void php_array_replace(Array &arr1, const Array& arr2) {
  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key = iter.first();
    arr1.setWithRef(key, iter.secondVal(), true);
  }
}

static void php_array_replace_recursive(PointerSet &seen, bool check,
                                        Array &arr1, const Array& arr2) {
  if (arr1.get() == arr2.get()) {
    // This is an optimization, but it also avoids an assert in
    // setWithRef (Variant::setWithRef asserts that its source
    // and destination are not the same).
    // If the arrays are self recursive, this does change the behavior
    // slightly - it skips the "recursion detected" warning.
    return;
  }

  auto const arr1_ptr = (void*)arr1.get();
  if (check) {
    if (seen.find(arr1_ptr) != seen.end()) {
      raise_warning("array_replace_recursive(): recursion detected");
      return;
    }
    seen.insert(arr1_ptr);
  }

  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key = iter.first();
    auto const rval = tvToCell(iter.secondRval());
    if (arr1.exists(key, true) && isArrayLikeType(rval.type())) {
      Variant& v = arr1.lvalAt(key, AccessFlags::Key);
      if (v.isArray()) {
        Array subarr1 = v.toArray().toPHPArray();
        php_array_replace_recursive(seen, couldRecur(v, subarr1.get()),
                                    subarr1, ArrNR(rval.val().parr));
        v = subarr1;
      } else {
        arr1.set(key, iter.secondVal(), true);
      }
    } else {
      arr1.setWithRef(key, iter.secondVal(), true);
    }
  }

  if (check) {
    seen.erase(arr1_ptr);
  }
}

TypedValue HHVM_FUNCTION(array_replace,
                         const Variant& array1,
                         const Variant& array2 /* = uninit_variant */,
                         const Array& args /* = null array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  php_array_replace(ret, arr_array1);

  if (UNLIKELY(array2.isNull() && args.empty())) {
    return tvReturn(std::move(ret));
  }

  getCheckedArray(array2);
  php_array_replace(ret, arr_array2);

  for (ArrayIter iter(args); iter; ++iter) {
    auto const v = VarNR(iter.secondVal());
    getCheckedArray(v);
    php_array_replace(ret, arr_v);
  }
  return tvReturn(std::move(ret));
}

TypedValue HHVM_FUNCTION(array_replace_recursive,
                         const Variant& array1,
                         const Variant& array2 /* = uninit_variant */,
                         const Array& args /* = null array */) {
  getCheckedArray(array1);
  Array ret = Array::Create();
  PointerSet seen;
  php_array_replace_recursive(seen, false, ret, arr_array1);
  assert(seen.empty());

  if (UNLIKELY(array2.isNull() && args.empty())) {
    return tvReturn(std::move(ret));
  }

  getCheckedArray(array2);
  php_array_replace_recursive(seen, false, ret, arr_array2);
  assert(seen.empty());

  for (ArrayIter iter(args); iter; ++iter) {
    auto const v = VarNR(iter.secondVal());
    getCheckedArray(v);
    php_array_replace_recursive(seen, false, ret, arr_v);
    assert(seen.empty());
  }
  return tvReturn(std::move(ret));
}

TypedValue HHVM_FUNCTION(array_pad,
                         const Variant& input,
                         int pad_size,
                         const Variant& pad_value) {
  getCheckedArray(input);
  auto arr =
    UNLIKELY(input.isHackArray()) ? arr_input.toPHPArray() : arr_input;
  if (pad_size > 0) {
    return tvReturn(ArrayUtil::PadRight(arr, pad_value, pad_size));
  } else {
    return tvReturn(ArrayUtil::PadLeft(arr, pad_value, -pad_size));
  }
}

TypedValue HHVM_FUNCTION(array_pop,
                         VRefParam containerRef) {
  const auto* container = containerRef->asCell();
  if (UNLIKELY(!isMutableContainer(*container))) {
    raise_warning("array_pop() expects parameter 1 to be an "
                  "array or mutable collection");
    return make_tv<KindOfNull>();
  }
  if (!getContainerSize(containerRef)) {
    return make_tv<KindOfNull>();
  }
  if (isArrayLikeType(container->m_type)) {
    if (auto ref = containerRef.getVariantOrNull()) {
      return tvReturn(ref->asArrRef().pop());
    }
    auto ad = container->m_data.parr;
    if (ad->size()) {
      auto last = ad->iter_last();
      return tvReturn(ad->getValue(last));
    }
    return make_tv<KindOfNull>();
  }
  assert(container->m_type == KindOfObject);
  return tvReturn(collections::pop(container->m_data.pobj));
}

TypedValue HHVM_FUNCTION(array_product,
                         const Variant& input) {
  if (UNLIKELY(!isContainer(input))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection as argument 1",
                  __FUNCTION__+2);
    return make_tv<KindOfNull>();
  }

  int64_t i = 1;
  ArrayIter iter(input);
  for (; iter; ++iter) {
    auto const rval = tvToCell(iter.secondRvalPlus());

    switch (rval.type()) {
      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfRef:
        i *= cellToInt(rval.tv());
        continue;

      case KindOfDouble:
        goto DOUBLE;

      case KindOfPersistentString:
      case KindOfString: {
        int64_t ti;
        double td;
        if (rval.val().pstr->isNumericWithVal(ti, td, 1) == KindOfInt64) {
          i *= ti;
          continue;
        } else {
          goto DOUBLE;
        }
      }

      case KindOfPersistentVec:
      case KindOfVec:
      case KindOfPersistentDict:
      case KindOfDict:
      case KindOfPersistentKeyset:
      case KindOfKeyset:
      case KindOfPersistentArray:
      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
        continue;
    }
    not_reached();
  }
  return make_tv<KindOfInt64>(i);

DOUBLE:
  double d = i;
  for (; iter; ++iter) {
    auto const rval = tvToCell(iter.secondRvalPlus());
    switch (rval.type()) {
      DT_UNCOUNTED_CASE:
      case KindOfString:
      case KindOfRef:
        d *= cellToDouble(rval.tv());

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
        continue;
    }
    not_reached();
  }
  return make_tv<KindOfDouble>(d);
}

TypedValue HHVM_FUNCTION(array_push,
                         VRefParam container,
                         const Variant& var,
                         const Array& args /* = null array */) {
  if (LIKELY(container->isArray())) {
    auto ref = container.getVariantOrNull();
    if (!ref) {
      return make_tv<KindOfInt64>(
        1 + args.size() + container->asCArrRef().size()
      );
    }

    /*
     * Important note: this *must* cast the parr in the inner cell to
     * the Array&---we can't copy it to the stack or anything because we
     * might escalate.
     */
    Array& arr_array = ref->asArrRef();
    arr_array.append(var);
    for (ArrayIter iter(args); iter; ++iter) {
      arr_array.append(iter.second());
    }
    return make_tv<KindOfInt64>(arr_array.size());
  }

  if (container.isObject()) {
    ObjectData* obj = container.getObjectData();
    if (obj->isCollection()) {
      switch (obj->collectionType()) {
        case CollectionType::Vector: {
          c_Vector* vec = static_cast<c_Vector*>(obj);
          vec->reserve(vec->size() + args.size() + 1);
          vec->add(var);
          for (ArrayIter iter(args); iter; ++iter) {
            vec->add(iter.second());
          }
          return make_tv<KindOfInt64>(vec->size());
        }
        case CollectionType::Set: {
          c_Set* set = static_cast<c_Set*>(obj);
          set->reserve(set->size() + args.size() + 1);
          set->add(var);
          for (ArrayIter iter(args); iter; ++iter) {
            set->add(iter.second());
          }
          return make_tv<KindOfInt64>(set->size());
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
  return make_tv<KindOfNull>();
}

TypedValue HHVM_FUNCTION(array_rand,
                         const Variant& input,
                         int num_req /* = 1 */) {
  getCheckedArray(input);
  return tvReturn(ArrayUtil::RandomKeys(arr_input, num_req));
}

TypedValue HHVM_FUNCTION(array_reverse,
                         const Variant& input,
                         bool preserve_keys /* = false */) {

  getCheckedContainer(input);
  return tvReturn(ArrayUtil::Reverse(arr_input, preserve_keys));
}

TypedValue HHVM_FUNCTION(array_shift,
                         VRefParam array) {
  const auto* cell_array = array->asCell();
  if (UNLIKELY(!isMutableContainer(*cell_array))) {
    raise_warning("array_shift() expects parameter 1 to be an "
                  "array or mutable collection");
    return make_tv<KindOfNull>();
  }
  if (!getContainerSize(array)) {
    return make_tv<KindOfNull>();
  }
  if (isArrayLikeType(cell_array->m_type)) {
    if (auto ref = array.getVariantOrNull()) {
      return tvReturn(ref->asArrRef().dequeue());
    }
    auto ad = cell_array->m_data.parr;
    if (ad->size()) {
      auto first = ad->iter_begin();
      return tvReturn(ad->getValue(first));
    }
    return make_tv<KindOfNull>();
  }
  assertx(cell_array->m_type == KindOfObject);
  return tvReturn(collections::shift(cell_array->m_data.pobj));
}

TypedValue HHVM_FUNCTION(array_slice,
                         TypedValue cell_input,
                         int64_t offset,
                         const Variant& length /* = uninit_variant */,
                         bool preserve_keys /* = false */) {
  if (UNLIKELY(!isContainer(cell_input))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection as argument 1",
                  __FUNCTION__+2);
    return make_tv<KindOfNull>();
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
    return make_tv<KindOfPersistentArray>(staticEmptyArray());
  }

  bool input_is_packed = isPackedContainer(cell_input);

  // If the slice covers the entire input container, we can just nop when
  // preserve_keys is true, or when preserve_keys is false but the container
  // is packed so we know the keys already map to [0,N].
  if (offset == 0 && len == num_in && (preserve_keys || input_is_packed)) {
    if (isArrayType(cell_input.m_type)) {
      return tvReturn(Variant{cell_input.m_data.parr});
    }
    if (isArrayLikeType(cell_input.m_type)) {
      return tvReturn(ArrNR{cell_input.m_data.parr}.asArray().toPHPArray());
    }
    return tvReturn(cell_input.m_data.pobj->toArray());
  }

  int pos = 0;
  ArrayIter iter(cell_input);
  for (; pos < offset && iter; ++pos, ++iter) {}

  if (input_is_packed && (offset == 0 || !preserve_keys)) {
    PackedArrayInit ret(len);
    for (; pos < (offset + len) && iter; ++pos, ++iter) {
      ret.appendWithRef(iter.secondValPlus());
    }
    return tvReturn(ret.toVariant());
  }

  // Otherwise PackedArrayInit can't be used because non-numeric keys are
  // preserved even when preserve_keys is false
  Array ret = Array::attach(PackedArray::MakeReserve(len));
  for (; pos < (offset + len) && iter; ++pos, ++iter) {
    Variant key(iter.first());
    bool doAppend = !preserve_keys && key.isNumeric();
    if (doAppend) {
      ret.appendWithRef(iter.secondValPlus());
    } else {
      ret.setWithRef(key, iter.secondValPlus(), true);
    }
  }
  return tvReturn(std::move(ret));
}

Variant array_splice(VRefParam input, int offset,
                     const Variant& length, const Variant& replacement) {
  getCheckedArrayVariant(input);
  if (arr_input.isHackArray()) {
    throw_expected_array_exception("array_splice");
    return init_null();
  }
  Array ret = Array::Create();
  int64_t len = length.isNull() ? 0x7FFFFFFF : length.toInt64();
  input.assignIfRef(ArrayUtil::Splice(arr_input, offset, len, replacement, &ret));
  return ret;
}

TypedValue HHVM_FUNCTION(array_splice,
                         VRefParam input,
                         int offset,
                         const Variant& length,
                         const Variant& replacement) {
  return tvReturn(array_splice(input, offset, length, replacement));
}

TypedValue HHVM_FUNCTION(array_sum,
                         const Variant& input) {
  if (UNLIKELY(!isContainer(input))) {
    raise_warning("Invalid operand type was used: %s expects "
                  "an array or collection as argument 1",
                  __FUNCTION__+2);
    return make_tv<KindOfNull>();
  }

  int64_t i = 0;
  ArrayIter iter(input);
  for (; iter; ++iter) {
    auto const rval = tvToCell(iter.secondRvalPlus());

    switch (rval.type()) {
      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfRef:
        i += cellToInt(rval.tv());
        continue;

      case KindOfDouble:
        goto DOUBLE;

      case KindOfPersistentString:
      case KindOfString: {
        int64_t ti;
        double td;
        if (rval.val().pstr->isNumericWithVal(ti, td, 1) == KindOfInt64) {
          i += ti;
          continue;
        } else {
          goto DOUBLE;
        }
      }

      case KindOfPersistentVec:
      case KindOfVec:
      case KindOfPersistentDict:
      case KindOfDict:
      case KindOfPersistentKeyset:
      case KindOfKeyset:
      case KindOfPersistentArray:
      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
        continue;
    }
    not_reached();
  }
  return make_tv<KindOfInt64>(i);

DOUBLE:
  double d = i;
  for (; iter; ++iter) {
    auto const rval = tvToCell(iter.secondRvalPlus());
    switch (rval.type()) {
      DT_UNCOUNTED_CASE:
      case KindOfString:
      case KindOfRef:
        d += cellToDouble(rval.tv());

      case KindOfVec:
      case KindOfDict:
      case KindOfKeyset:
      case KindOfArray:
      case KindOfObject:
      case KindOfResource:
        continue;
    }
    not_reached();
  }
  return make_tv<KindOfDouble>(d);
}

TypedValue HHVM_FUNCTION(array_unshift,
                         VRefParam array,
                         const Variant& var,
                         const Array& args /* = null array */) {
  const auto* cell_array = array->asCell();
  if (UNLIKELY(!isContainer(*cell_array))) {
    raise_warning("%s() expects parameter 1 to be an array, Vector, or Set",
                  __FUNCTION__+2 /* remove the "f_" prefix */);
    return make_tv<KindOfNull>();
  }
  if (isArrayLikeType(cell_array->m_type)) {
    auto ref_array = array.getVariantOrNull();
    if (!ref_array) {
      return make_tv<KindOfInt64>(
        cell_array->m_data.parr->size() + args.size() + 1
      );
    }
    if (cell_array->m_data.parr->isVectorData()) {
      if (!args.empty()) {
        auto pos_limit = args->iter_end();
        for (ssize_t pos = args->iter_last(); pos != pos_limit;
             pos = args->iter_rewind(pos)) {
          ref_array->asArrRef().prepend(args->atPos(pos));
        }
      }
      ref_array->asArrRef().prepend(var);
    } else {
      {
        auto newArray = Array::attach(
          MixedArray::MakeReserveSame(cell_array->m_data.parr, 0));
        newArray.append(var);
        if (!args.empty()) {
          auto pos_limit = args->iter_end();
          for (ssize_t pos = args->iter_begin(); pos != pos_limit;
               pos = args->iter_advance(pos)) {
            newArray.append(args->atPos(pos));
          }
        }
        if (cell_array->m_data.parr->isKeyset()) {
          for (ArrayIter iter(array.toArray()); iter; ++iter) {
            Variant key(iter.first());
            newArray.append(key);
          }
        } else {
          for (ArrayIter iter(array.toArray()); iter; ++iter) {
            Variant key(iter.first());
            if (key.isInteger()) {
              newArray.appendWithRef(iter.secondVal());
            } else {
              newArray.setWithRef(key, iter.secondVal(), true);
            }
          }
        }
        *ref_array = std::move(newArray);
      }
      // Reset the array's internal pointer
      ref_array->asArrRef()->reset();
    }
    return make_tv<KindOfInt64>(ref_array->asArrRef().size());
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
          vec->addFront(tvToCell(args->atPos(pos)));
        }
      }
      vec->addFront(*var.asCell());
      return make_tv<KindOfInt64>(vec->size());
    }
    case CollectionType::Set: {
      auto* st = static_cast<c_Set*>(obj);
      if (!args.empty()) {
        auto pos_limit = args->iter_end();
        for (ssize_t pos = args->iter_last(); pos != pos_limit;
             pos = args->iter_rewind(pos)) {
          st->addFront(tvToCell(args->atPos(pos)));
        }
      }
      st->addFront(*var.asCell());
      return make_tv<KindOfInt64>(st->size());
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
  return make_tv<KindOfNull>();
}

Variant array_values(const Variant& input) {
  auto const cell = *input.asCell();
  if (isArrayType(cell.m_type)) {
    if (cell.m_data.parr->isPacked()) {
      return input;
    }
    if (cell.m_data.parr->isMixed()) {
      if (MixedArray::IsStrictVector(cell.m_data.parr)) {
        return input;
      }
    } else if (cell.m_data.parr->isApcArray() &&
               APCLocalArray::IsVectorData(cell.m_data.parr)) {
      return input;
    }
  }

  folly::Optional<PackedArrayInit> ai;
  auto ok = IterateV(cell,
                     [&](ArrayData* adata) {
                       ai.emplace(adata->size());
                     },
                     [&](TypedValue v) {
                       ai->appendWithRef(v);
                     },
                     [&](ObjectData* coll) {
                       if (coll->collectionType() == CollectionType::Pair) {
                         ai.emplace(2);
                       }
                     });

  if (!ok) {
    raise_warning("array_values() expects parameter 1 to be an array "
                  "or collection");
    return init_null();
  }

  assert(ai.hasValue());
  return ai->toVariant();
}

TypedValue HHVM_FUNCTION(array_values,
                         const Variant& input) {
  return tvReturn(array_values(input));
}

static void walk_func(Variant& value,
                      const Variant& key,
                      const Variant& userdata,
                      const void *data) {
  CallCtx* ctx = (CallCtx*)data;
  int nargs = userdata.isInitialized() ? 3 : 2;
  TypedValue args[3] = { *value.asRef(), *key.asCell(), *userdata.asCell() };
  tvDecRefGen(
    g_context->invokeFuncFew(*ctx, nargs, args)
  );
}

bool HHVM_FUNCTION(array_walk_recursive,
                   VRefParam input,
                   const Variant& funcname,
                   const Variant& userdata /* = uninit_variant */) {
  if (!input.isPHPArray()) {
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
  Variant var(input, Variant::WithRefBind{});
  ArrayUtil::Walk(var, walk_func, &ctx, true, &seen, userdata);
  return true;
}

bool HHVM_FUNCTION(array_walk,
                   VRefParam input,
                   const Variant& funcname,
                   const Variant& userdata /* = uninit_variant */) {
  if (!input.isPHPArray()) {
    throw_expected_array_exception("array_walk");
    return false;
  }
  CallCtx ctx;
  CallerFrame cf;
  vm_decode_function(funcname, cf(), false, ctx);
  if (ctx.func == NULL) {
    return false;
  }
  Variant var(input, Variant::WithRefBind{});
  ArrayUtil::Walk(var, walk_func, &ctx, false, NULL, userdata);
  return true;
}

static void compact(PointerSet& seen, VarEnv* v, Array &ret,
                    const Variant& var) {
  if (var.isArray()) {
    auto adata = var.getArrayData();
    auto check = couldRecur(var, adata);
    if (check) {
      if (seen.find(adata) != seen.end()) {
        raise_warning("compact(): recursion detected");
        return;
      }
      seen.insert(adata);
    }
    for (ArrayIter iter(adata); iter; ++iter) {
      compact(seen, v, ret, VarNR(iter.secondVal()));
    }
    if (check) seen.erase(adata);
  } else {
    String varname = var.toString();
    if (!varname.empty() && v->lookup(varname.get()) != NULL) {
      ret.set(varname, tvAsVariant(v->lookup(varname.get())));
    }
  }
}

Array HHVM_FUNCTION(compact,
                    const Variant& varname,
                    const Array& args /* = null array */) {
  Array ret = Array::attach(PackedArray::MakeReserve(args.size() + 1));
  VarEnv* v = g_context->getOrCreateVarEnv();
  if (v) {
    PointerSet seen;
    compact(seen, v, ret, varname);
    if (!args.empty()) compact(seen, v, ret, args);
  }
  return ret;
}

static int php_count_recursive(const Array& array) {
  long cnt = array.size();
  for (ArrayIter iter(array); iter; ++iter) {
    Variant value = iter.second();
    if (value.isArray()) {
      const Array& arr_value = value.asCArrRef();
      check_recursion_throw();
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
  array.assignIfRef(ArrayUtil::Shuffle(array));
  return true;
}

enum class CountMode {
  NORMAL = 0,
  RECURSIVE = 1,
};

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
    case KindOfPersistentString:
    case KindOfString:
    case KindOfResource:
      return 1;

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
      if ((CountMode)mode == CountMode::RECURSIVE) {
        const Array& arr_var = var.toCArrRef();
        return php_count_recursive(arr_var);
      }
      return var.getArrayData()->size();

    case KindOfObject:
      {
        Object obj = var.toObject();
        if (obj->isCollection()) {
          return collections::getSize(obj.get());
        }
        if (obj.instanceof(SystemLib::s_CountableClass)) {
          return obj->o_invoke_few_args(s_count, 0).toInt64();
        }
      }
      return 1;

    case KindOfRef:
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
static Variant iter_op_impl(VRefParam refParam, OpPtr op, const String& objOp,
                            NonArrayRet nonArray,
                            bool(ArrayData::*pred)() const =
                              &ArrayData::isInvalid) {
  auto& cell = *refParam.wrapped().asCell();
  if (!isArrayLikeType(cell.m_type)) {
    if (cell.m_type == KindOfObject) {
      auto obj = refParam.wrapped().toObject();
      if (obj->instanceof(SystemLib::s_ArrayObjectClass)) {
        return obj->o_invoke_few_args(objOp, 0);
      }
    }
    throw_bad_type_exception("expecting an array");
    return Variant(nonArray);
  }

  auto ad = cell.m_data.parr;
  auto constexpr doCow = !std::is_same<DoCow, NoCow>::value;
  if (doCow && ad->cowCheck() && !(ad->*pred)() &&
      !ad->noCopyOnWrite()) {
    ad = ad->copy();
    if (LIKELY(refParam.isRefData())) {
      cellMove(make_array_like_tv(ad), *refParam.getRefData()->tv());
    } else {
      req::ptr<ArrayData> tmp(ad, req::ptr<ArrayData>::NoIncRef{});
      return (ad->*op)();
    }
  }
  return (ad->*op)();
}

}

const StaticString
  s___each("__each"),
  s___current("__current"),
  s___key("__key"),
  s___next("__next"),
  s___prev("__prev"),
  s___reset("__reset"),
  s___end("__end");


Variant HHVM_FUNCTION(each,
                      VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::each,
    s___each,
    Variant::NullInit()
  );
}

Variant HHVM_FUNCTION(current,
                      VRefParam refParam) {
  return iter_op_impl<NoCow>(
    refParam,
    &ArrayData::current,
    s___current,
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
    s___key,
    false
  );
}

Variant HHVM_FUNCTION(next,
                      VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::next,
    s___next,
    false
  );
}

Variant HHVM_FUNCTION(prev,
                      VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::prev,
    s___prev,
    false
  );
}

Variant HHVM_FUNCTION(reset,
                      VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::reset,
    s___reset,
    false,
    &ArrayData::isHead
  );
}

Variant HHVM_FUNCTION(end,
                      VRefParam refParam) {
  return iter_op_impl(
    refParam,
    &ArrayData::end,
    s___end,
    false,
    &ArrayData::isTail
  );
}

bool HHVM_FUNCTION(in_array,
                   const Variant& needle,
                   const Variant& haystack,
                   bool strict /* = false */) {
  bool ret = false;
  auto ok = strict ?
    IterateV(*haystack.asCell(),
             [](ArrayData*) { return false; },
             [&](TypedValue v) -> bool {
               if (tvSame(v, *needle.asTypedValue())) {
                 ret = true;
                 return true;
               }
               return false;
             },
             [](ObjectData*) { return false; }) :
    IterateV(*haystack.asCell(),
             [](ArrayData*) { return false; },
             [&](TypedValue v) -> bool {
               if (tvEqual(v, *needle.asTypedValue())) {
                 ret = true;
                 return true;
               }
               return false;
             },
             [](ObjectData*) { return false; });

  if (UNLIKELY(!ok)) {
    raise_warning("in_array() expects parameter 2 to be an array "
                  "or collection");
  }
  return ret;
}

Variant array_search(const Variant& needle,
                     const Variant& haystack,
                     bool strict /* = false */) {
  Variant ret = false;
  auto ok = strict ?
    IterateKV(*haystack.asCell(),
              [](ArrayData*) { return false; },
              [&](Cell k, TypedValue v) -> bool {
                if (tvSame(v, *needle.asTypedValue())) {
                  ret = VarNR(k);
                  return true;
                }
                return false;
              },
              [](ObjectData*) { return false; }) :
    IterateKV(*haystack.asCell(),
              [](ArrayData*) { return false; },
              [&](Cell k, TypedValue v) -> bool {
                if (tvEqual(v, *needle.asTypedValue())) {
                  ret = VarNR(k);
                  return true;
                }
                return false;
              },
              [](ObjectData*) { return false; });

  if (UNLIKELY(!ok)) {
    raise_warning("array_search() expects parameter 2 to be an array "
                  "or collection");
    return init_null();
  }

  return ret;
}

TypedValue HHVM_FUNCTION(array_search,
                         const Variant& needle,
                         const Variant& haystack,
                         bool strict /* = false */) {
  return tvReturn(array_search(needle, haystack, strict));
}

TypedValue HHVM_FUNCTION(range,
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
        return tvReturn(ArrayUtil::Range(d1, d2, dstep));
      }

      int64_t lstep = double_to_int64(dstep);
      if (type1 == KindOfInt64 || type2 == KindOfInt64) {
        if (type1 != KindOfInt64) n1 = slow.toInt64();
        if (type2 != KindOfInt64) n2 = shigh.toInt64();
        return tvReturn(ArrayUtil::Range(n1, n2, lstep));
      }

      return tvReturn(ArrayUtil::Range((unsigned char)slow.charAt(0),
                                       (unsigned char)shigh.charAt(0), lstep));
    }
  }

  if (low.is(KindOfDouble) || high.is(KindOfDouble) || is_step_double) {
    return tvReturn(ArrayUtil::Range(low.toDouble(), high.toDouble(), dstep));
  }

  int64_t lstep = double_to_int64(dstep);
  return tvReturn(ArrayUtil::Range(low.toInt64(), high.toInt64(), lstep));
}
///////////////////////////////////////////////////////////////////////////////
// diff/intersect helpers

static int cmp_func(const Variant& v1, const Variant& v2, const void *data) {
  auto callback = static_cast<const Variant*>(data);
  return vm_call_user_func(*callback, make_packed_array(v1, v2)).toInt32();
}

// PHP 5.x does different things when diffing against the same array,
// particularly when the comparison function is outside the norm of
// return -1, 0, 1 specification. To do what PHP 5.x in these cases,
// use the RuntimeOption
#define COMMA ,
#define diff_intersect_body(type, vararg, intersect_params)     \
  getCheckedArray(array1);                                      \
  if (!arr_array1.size()) {                                     \
    return tvReturn(empty_array());                             \
  }                                                             \
  Array ret = Array::Create();                                  \
  if (RuntimeOption::EnableZendSorting) {                       \
    getCheckedArray(array2);                                    \
    if (arr_array1.same(arr_array2)) {                          \
      return tvReturn(std::move(ret));                          \
    }                                                           \
  }                                                             \
  ret = arr_array1.type(array2, intersect_params);              \
  if (ret.size()) {                                             \
    for (ArrayIter iter(vararg); iter; ++iter) {                \
      ret = ret.type(iter.second(), intersect_params);          \
      if (!ret.size()) break;                                   \
    }                                                           \
  }                                                             \
  return tvReturn(std::move(ret));

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
    if (LIKELY(isStringType(c.m_type))) {
      s = c.m_data.pstr;
    } else {
      s = tvCastToString(c);
      decRefStr(strTv->m_data.pstr);
      strTv->m_data.pstr = s;
    }
    int64_t n;
    if (convertIntLikeStrs && s->isStrictlyInteger(n)) {
      if (RuntimeOption::EvalHackArrCompatNotices) raise_intish_index_cast();
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
  if (LIKELY(isStringType(c.m_type))) {
    s = c.m_data.pstr;
  } else {
    s = tvCastToString(c);
    decRefStr(strTv->m_data.pstr);
    strTv->m_data.pstr = s;
  }
  int64_t n;
  if (convertIntLikeStrs && s->isStrictlyInteger(n)) {
    if (RuntimeOption::EvalHackArrCompatNotices) raise_intish_index_cast();
    return st->contains(n);
  }
  return st->contains(s);
}

static void containerValuesToSetHelper(const req::ptr<c_Set>& st,
                                       const Variant& container) {
  Variant strHolder(empty_string_variant());
  TypedValue* strTv = strHolder.asTypedValue();
  for (ArrayIter iter(container); iter; ++iter) {
    auto const c = tvToCell(iter.secondValPlus());
    addToSetHelper(st, c, strTv, true);
  }
}

static void containerKeysToSetHelper(const req::ptr<c_Set>& st,
                                     const Variant& container) {
  Variant strHolder(empty_string_variant());
  TypedValue* strTv = strHolder.asTypedValue();
  bool isKey = isArrayLikeType(container.asCell()->m_type);
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
    return make_tv<KindOfNull>(); \
  } \
  bool moreThanTwo = !args.empty(); \
  size_t largestSize = getContainerSize(c2); \
  if (UNLIKELY(moreThanTwo)) { \
    int pos = 3; \
    for (ArrayIter argvIter(args); argvIter; ++argvIter, ++pos) { \
      auto const c = tvToCell(argvIter.secondVal()); \
      if (!isContainer(c)) { \
        raise_warning("%s() expects parameter %d to be an array or collection",\
                      __FUNCTION__+2, /* remove the "f_" prefix */ \
                      pos); \
        return make_tv<KindOfNull>(); \
      } \
      size_t sz = getContainerSize(c); \
      if (sz > largestSize) { \
        largestSize = sz; \
      } \
    } \
  } \
  /* If container1 is empty, we can stop here and return the empty array */ \
  if (!getContainerSize(c1)) { \
    return make_tv<KindOfPersistentArray>(staticEmptyArray()); \
  } \
  /* If all of the containers (except container1) are empty, we can just \
     return container1 (converting it to an array if needed) */ \
  if (!largestSize) { \
    if (isArrayLikeType(c1.m_type)) { \
      return tvReturn(container1); \
    } else { \
      return tvReturn(container1.toArray()); \
    } \
  } \
  Array ret = Array::Create();

TypedValue HHVM_FUNCTION(array_diff,
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
      containerValuesToSetHelper(st, VarNR(argvIter.secondVal()));
    }
  }
  // Loop over container1, only copying over key/value pairs where the value
  // is not present in the Set. When checking if a value is present in the
  // Set, any value that is not an integer or string is cast to a string, and
  // we convert int-like strings to integers.
  Variant strHolder(empty_string_variant());
  TypedValue* strTv = strHolder.asTypedValue();
  bool isKey = isArrayLikeType(c1.m_type);
  for (ArrayIter iter(container1); iter; ++iter) {
    auto const c = tvToCell(iter.secondValPlus());
    if (checkSetHelper(st, c, strTv, true)) continue;
    ret.setWithRef(iter.first(), iter.secondValPlus(), isKey);
  }
  return tvReturn(std::move(ret));
}

TypedValue HHVM_FUNCTION(array_diff_key,
                         const Variant& container1,
                         const Variant& container2,
                         const Array& args /* = null array */) {
  ARRAY_DIFF_PRELUDE()
  // If we're only dealing with two containers and if they are both arrays,
  // we can avoid creating an intermediate Set
  if (!moreThanTwo &&
      isArrayLikeType(c1.m_type) &&
      isArrayLikeType(c2.m_type)) {
    auto ad2 = c2.m_data.parr;
    for (ArrayIter iter(container1); iter; ++iter) {
      auto key = iter.first();
      const auto& c = *key.asCell();
      if (c.m_type == KindOfInt64) {
        if (ad2->exists(c.m_data.num)) continue;
      } else {
        assert(isStringType(c.m_type));
        if (ad2->exists(c.m_data.pstr)) continue;
      }
      ret.setWithRef(key, iter.secondValPlus(), true);
    }
    return tvReturn(std::move(ret));
  }
  // Put all of the keys from all the containers (except container1) into a
  // Set. All types aside from integer and string will be cast to string, and
  // we also convert int-like strings to integers.
  auto st = req::make<c_Set>();
  st->reserve(largestSize);
  containerKeysToSetHelper(st, container2);
  if (UNLIKELY(moreThanTwo)) {
    for (ArrayIter argvIter(args); argvIter; ++argvIter) {
      containerKeysToSetHelper(st, VarNR(argvIter.secondVal()));
    }
  }
  // Loop over container1, only copying over key/value pairs where the key is
  // not present in the Set. When checking if a key is present in the Set, any
  // key that is not an integer or string is cast to a string, and we convert
  // int-like strings to integers.
  Variant strHolder(empty_string_variant());
  TypedValue* strTv = strHolder.asTypedValue();
  bool isKey = isArrayLikeType(c1.m_type);
  for (ArrayIter iter(container1); iter; ++iter) {
    auto key = iter.first();
    const auto& c = *key.asCell();
    if (checkSetHelper(st, c, strTv, !isKey)) continue;
    ret.setWithRef(key, iter.secondValPlus(), isKey);
  }
  return tvReturn(std::move(ret));
}

#undef ARRAY_DIFF_PRELUDE

TypedValue HHVM_FUNCTION(array_udiff,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Array& args /* = null array */) {
  Variant func = data_compare_func;
  Array extra = args;
  if (!extra.empty()) {
    extra.prepend(func);
    func = extra.pop();
  }
  diff_intersect_body(diff, extra, false COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func);
}

TypedValue HHVM_FUNCTION(array_diff_assoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Array& args /* = null array */) {
  diff_intersect_body(diff, args, true COMMA true);
}

TypedValue HHVM_FUNCTION(array_diff_uassoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& key_compare_func,
                         const Array& args /* = null array */) {
  Variant func = key_compare_func;
  Array extra = args;
  if (!extra.empty()) {
    extra.prepend(func);
    func = extra.pop();
  }
  diff_intersect_body(diff, extra, true COMMA true COMMA cmp_func COMMA &func);
}

TypedValue HHVM_FUNCTION(array_udiff_assoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Array& args /* = null array */) {
  Variant func = data_compare_func;
  Array extra = args;
  if (!extra.empty()) {
    extra.prepend(func);
    func = extra.pop();
  }
  diff_intersect_body(diff, extra, true COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func);
}

TypedValue HHVM_FUNCTION(array_udiff_uassoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Variant& key_compare_func,
                         const Array& args /* = null array */) {
  Variant data_func = data_compare_func;
  Variant key_func = key_compare_func;
  Array extra = args;
  if (!extra.empty()) {
    extra.prepend(key_func);
    extra.prepend(data_func);
    key_func = extra.pop();
    data_func = extra.pop();
  }
  diff_intersect_body(diff, extra, true
                      COMMA true COMMA cmp_func COMMA &key_func
                      COMMA cmp_func COMMA &data_func);
}

TypedValue HHVM_FUNCTION(array_diff_ukey,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& key_compare_func,
                         const Array& args /* = null array */) {
  Variant func = key_compare_func;
  Array extra = args;
  if (!extra.empty()) {
    extra.prepend(func);
    func = extra.pop();
  }
  diff_intersect_body(diff, extra, true COMMA false COMMA cmp_func COMMA &func);
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
  TypedValue* containers = req::make_raw_array<TypedValue>(count);
  tvCopy(*a.asCell(), containers[0]);
  int pos = 1;
  for (ArrayIter argvIter(argv); argvIter; ++argvIter, ++pos) {
    cellCopy(tvToCell(argvIter.secondVal()), containers[pos]);
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
    mp->set(c.m_data.num, *intOneTv);
  } else {
    StringData* s;
    if (LIKELY(isStringType(c.m_type))) {
      s = c.m_data.pstr;
    } else {
      s = tvCastToString(c);
      decRefStr(strTv->m_data.pstr);
      strTv->m_data.pstr = s;
    }
    int64_t n;
    if (convertIntLikeStrs && s->isStrictlyInteger(n)) {
      if (RuntimeOption::EvalHackArrCompatNotices) raise_intish_index_cast();
      mp->set(n, *intOneTv);
    } else {
      mp->set(s, *intOneTv);
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
    if (LIKELY(isStringType(c.m_type))) {
      s = c.m_data.pstr;
    } else {
      s = tvCastToString(c);
      decRefStr(strTv->m_data.pstr);
      strTv->m_data.pstr = s;
    }
    int64_t n;
    if (convertIntLikeStrs && s->isStrictlyInteger(n)) {
      if (RuntimeOption::EvalHackArrCompatNotices) raise_intish_index_cast();
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
    auto const c = tvToCell(iter.secondValPlus());
    // For each value v in containers[0], we add the key/value pair (v, 1)
    // to the map. If a value (after various conversions) occurs more than
    // once in the container, we'll simply overwrite the old entry and that's
    // fine.
    addToIntersectMapHelper(mp, c, &intOneTv, strTv, true);
  }
  for (int pos = 1; pos < count; ++pos) {
    for (ArrayIter iter(tvAsCVarRef(&containers[pos])); iter; ++iter) {
      auto const c = tvToCell(iter.secondValPlus());
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
    auto const rval = tvToCell(iter.secondRvalPlus());
    assert(rval.type() == KindOfInt64);
    if (rval.val().num == count) {
      st->add(*iter.first().asCell());
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
  bool isKey = isArrayLikeType(containers[0].m_type);
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
    isKey = isArrayLikeType(containers[pos].m_type);
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
    auto const rval = tvToCell(iter.secondRvalPlus());
    assert(rval.type() == KindOfInt64);
    if (rval.val().num == count) {
      st->add(*iter.first().asCell());
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
    return make_tv<KindOfNull>(); \
  } \
  bool moreThanTwo = !args.empty(); \
  /* Keep track of which input container was the smallest (excluding \
     container1) */ \
  int smallestPos = 0; \
  size_t smallestSize = getContainerSize(c2); \
  if (UNLIKELY(moreThanTwo)) { \
    int pos = 1; \
    for (ArrayIter argvIter(args); argvIter; ++argvIter, ++pos) { \
      auto const c = tvToCell(argvIter.secondVal()); \
      if (!isContainer(c)) { \
        raise_warning("%s() expects parameter %d to be an array or collection",\
                      __FUNCTION__+2, /* remove the "f_" prefix */ \
                      pos+2); \
        return make_tv<KindOfNull>(); \
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
  if (!getContainerSize(c1) || !smallestSize) { \
    return make_tv<KindOfPersistentArray>(staticEmptyArray()); \
  } \
  Array ret = Array::Create();

TypedValue HHVM_FUNCTION(array_intersect,
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
  bool isKey = isArrayLikeType(c1.m_type);
  for (ArrayIter iter(container1); iter; ++iter) {
    auto const c = tvToCell(iter.secondValPlus());
    if (!checkSetHelper(st, c, strTv, true)) continue;
    ret.setWithRef(iter.first(), iter.secondValPlus(), isKey);
  }
  return tvReturn(std::move(ret));
}

TypedValue HHVM_FUNCTION(array_intersect_key,
                         const Variant& container1,
                         const Variant& container2,
                         const Array& args /* = null array */) {
  ARRAY_INTERSECT_PRELUDE()
  // If we're only dealing with two containers and if they are both arrays,
  // we can avoid creating an intermediate Set
  if (!moreThanTwo &&
      isArrayLikeType(c1.m_type) &&
      isArrayLikeType(c2.m_type)) {
    auto ad2 = c2.m_data.parr;
    for (ArrayIter iter(container1); iter; ++iter) {
      auto key = iter.first();
      const auto& c = *key.asCell();
      if (c.m_type == KindOfInt64) {
        if (!ad2->exists(c.m_data.num)) continue;
      } else {
        assert(isStringType(c.m_type));
        if (!ad2->exists(c.m_data.pstr)) continue;
      }
      ret.setWithRef(key, iter.secondValPlus(), true);
    }
    return tvReturn(std::move(ret));
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
  bool isKey = isArrayLikeType(c1.m_type);
  for (ArrayIter iter(container1); iter; ++iter) {
    auto key = iter.first();
    const auto& c = *key.asCell();
    if (!checkSetHelper(st, c, strTv, !isKey)) continue;
    ret.setWithRef(key, iter.secondValPlus(), isKey);
  }
  return tvReturn(std::move(ret));
}

#undef ARRAY_INTERSECT_PRELUDE

TypedValue HHVM_FUNCTION(array_uintersect,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Array& args /* = null array */) {
  Variant func = data_compare_func;
  Array extra = args;
  if (!extra.empty()) {
    extra.prepend(func);
    func = extra.pop();
  }
  diff_intersect_body(intersect, extra, false COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func);
}

TypedValue HHVM_FUNCTION(array_intersect_assoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Array& args /* = null array */) {
  diff_intersect_body(intersect, args, true COMMA true);
}

TypedValue HHVM_FUNCTION(array_intersect_uassoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& key_compare_func,
                         const Array& args /* = null array */) {
  Variant func = key_compare_func;
  Array extra = args;
  if (!extra.empty()) {
    extra.prepend(func);
    func = extra.pop();
  }
  diff_intersect_body(intersect, extra, true COMMA true
                      COMMA cmp_func COMMA &func);
}

TypedValue HHVM_FUNCTION(array_uintersect_assoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Array& args /* = null array */) {
  Variant func = data_compare_func;
  Array extra = args;
  if (!extra.empty()) {
    extra.prepend(func);
    func = extra.pop();
  }
  diff_intersect_body(intersect, extra, true COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func);
}

TypedValue HHVM_FUNCTION(array_uintersect_uassoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Variant& key_compare_func,
                         const Array& args /* = null array */) {
  Variant data_func = data_compare_func;
  Variant key_func = key_compare_func;
  Array extra = args;
  if (!extra.empty()) {
    extra.prepend(key_func);
    extra.prepend(data_func);
    key_func = extra.pop();
    data_func = extra.pop();
  }
  diff_intersect_body(intersect, extra, true COMMA true COMMA cmp_func
                      COMMA &key_func COMMA cmp_func COMMA &data_func);
}

TypedValue HHVM_FUNCTION(array_intersect_ukey,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& key_compare_func,
                         const Array& args /* = null array */) {
  Variant func = key_compare_func;
  Array extra = args;
  if (!extra.empty()) {
    extra.prepend(func);
    func = extra.pop();
  }
  diff_intersect_body(intersect, extra, true COMMA false
                      COMMA cmp_func COMMA &func);
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
struct ArraySortTmp {
  explicit ArraySortTmp(TypedValue* arr, SortFunction sf) : m_arr(arr) {
    m_ad = arr->m_data.parr->escalateForSort(sf);
    assert(m_ad == arr->m_data.parr || m_ad->hasExactlyOneRef());
  }
  ~ArraySortTmp() {
    if (m_ad != m_arr->m_data.parr) {
      Array tmp = Array::attach(m_arr->m_data.parr);
      m_arr->m_data.parr = m_ad;
      m_arr->m_type = m_ad->toDataType();
    }
  }
  ArrayData* operator->() { return m_ad; }
 private:
  TypedValue* m_arr;
  ArrayData* m_ad;
};
}

static bool
php_sort(VRefParam container, int sort_flags,
         bool ascending, bool use_zend_sort) {
  if (container.isArray()) {
    auto ref = container.getVariantOrNull();
    if (!ref) return true;
    if (use_zend_sort) {
      return zend_sort(*ref, sort_flags, ascending);
    }
    SortFunction sf = getSortFunction(SORTFUNC_SORT, ascending);
    ArraySortTmp ast(ref->asTypedValue(), sf);
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
          bool ascending, bool use_zend_sort) {
  if (container.isArray()) {
    auto ref = container.getVariantOrNull();
    if (!ref) return true;
    if (use_zend_sort) {
      return zend_asort(*ref, sort_flags, ascending);
    }
    SortFunction sf = getSortFunction(SORTFUNC_ASORT, ascending);
    ArraySortTmp ast(ref->asTypedValue(), sf);
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
          bool use_zend_sort) {
  if (container.isArray()) {
    auto ref = container.getVariantOrNull();
    if (!ref) return true;
    if (use_zend_sort) {
      return zend_ksort(*ref, sort_flags, ascending);
    }
    SortFunction sf = getSortFunction(SORTFUNC_KRSORT, ascending);
    ArraySortTmp ast(ref->asTypedValue(), sf);
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
  bool use_zend_sort = RuntimeOption::EnableZendSorting;
  return php_sort(array, sort_flags, true, use_zend_sort);
}

bool HHVM_FUNCTION(rsort,
                   VRefParam array,
                   int sort_flags /* = 0 */) {
  bool use_zend_sort = RuntimeOption::EnableZendSorting;
  return php_sort(array, sort_flags, false, use_zend_sort);
}

bool HHVM_FUNCTION(asort,
                   VRefParam array,
                   int sort_flags /* = 0 */) {
  bool use_zend_sort = RuntimeOption::EnableZendSorting;
  return php_asort(array, sort_flags, true, use_zend_sort);
}

bool HHVM_FUNCTION(arsort,
                   VRefParam array,
                   int sort_flags /* = 0 */) {
  bool use_zend_sort = RuntimeOption::EnableZendSorting;
  return php_asort(array, sort_flags, false, use_zend_sort);
}

bool HHVM_FUNCTION(ksort,
                   VRefParam array,
                   int sort_flags /* = 0 */) {
  bool use_zend_sort = RuntimeOption::EnableZendSorting;
  return php_ksort(array, sort_flags, true, use_zend_sort);
}

bool HHVM_FUNCTION(krsort,
                   VRefParam array,
                   int sort_flags /* = 0 */) {
  bool use_zend_sort = RuntimeOption::EnableZendSorting;
  return php_ksort(array, sort_flags, false, use_zend_sort);
}

// NOTE: PHP's implementation of natsort and natcasesort accepts ArrayAccess
// objects as well, which does not make much sense, and which is not supported
// here.

bool HHVM_FUNCTION(natsort, VRefParam array) {
  return php_asort(array, SORT_NATURAL, true, false);
}

bool HHVM_FUNCTION(natcasesort, VRefParam array) {
  return php_asort(array, SORT_NATURAL_CASE, true, false);
}

bool HHVM_FUNCTION(usort,
                   VRefParam container,
                   const Variant& cmp_function) {
  if (container.isArray()) {
    auto sort = [](TypedValue* arr_array, const Variant& cmp_function) -> bool {
      if (RuntimeOption::EnableZendSorting) {
        tvAsVariant(arr_array).asArrRef().sort(cmp_func, false, true,
                                               &cmp_function);
        return true;
      } else {
        ArraySortTmp ast(arr_array, SORTFUNC_USORT);
        return ast->usort(cmp_function);
      }
    };
    auto ref = container.getVariantOrNull();
    if (LIKELY(ref != nullptr)) {
      return sort(ref->asTypedValue(), cmp_function);
    }
    auto tmp = container.wrapped();
    return sort(tmp.asTypedValue(), cmp_function);
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
    auto sort = [](TypedValue* arr_array, const Variant& cmp_function) -> bool {
      if (RuntimeOption::EnableZendSorting) {
        tvAsVariant(arr_array).asArrRef().sort(cmp_func, false, false,
                                               &cmp_function);
        return true;
      } else {
        ArraySortTmp ast(arr_array, SORTFUNC_UASORT);
        return ast->uasort(cmp_function);
      }
    };
    auto ref = container.getVariantOrNull();
    if (LIKELY(ref != nullptr)) {
      return sort(ref->asTypedValue(), cmp_function);
    }
    auto tmp = container.wrapped();
    return sort(tmp.asTypedValue(), cmp_function);
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
    auto sort = [](TypedValue* arr_array, const Variant& cmp_function) -> bool {
      ArraySortTmp ast(arr_array, SORTFUNC_UKSORT);
      return ast->uksort(cmp_function);
    };
    auto ref = container.getVariantOrNull();
    if (LIKELY(ref != nullptr)) {
      return sort(ref->asTypedValue(), cmp_function);
    }
    auto tmp = container.wrapped();
    return sort(tmp.asTypedValue(), cmp_function);
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

TypedValue HHVM_FUNCTION(array_unique,
                         const Variant& array,
                         int sort_flags /* = 2 */) {
  SuppressHackArrCompatNotices suppress;
  // NOTE, PHP array_unique accepts ArrayAccess objects as well,
  // which is not supported here.
  getCheckedArray(array);
  switch (sort_flags) {
  case SORT_STRING:
  case SORT_LOCALE_STRING:
    return tvReturn(ArrayUtil::StringUnique(arr_array));
  case SORT_NUMERIC:
    return tvReturn(ArrayUtil::NumericUnique(arr_array));
  case SORT_REGULAR:
  default:
    return tvReturn(ArrayUtil::RegularSortUnique(arr_array));
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

TypedValue HHVM_FUNCTION(hphp_array_idx,
                         const Variant& search,
                         const Variant& key,
                         const Variant& def) {
  if (!key.isNull()) {
    if (LIKELY(search.isArray())) {
      ArrayData *arr = search.getArrayData();
      auto const index = key.toKey(arr).tv();
      if (!isNullType(index.m_type)) {
        const Variant& ret = arr->get(index, false);
        return tvReturn((&ret != &uninit_variant) ? ret : def);
      }
    } else {
      raise_error("hphp_array_idx: search must be an array");
    }
  }
  return tvReturn(def);
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
  if (tv == nullptr || !tvAsVariant(tv).isPHPArray()) {
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
      int n = getArg<KindOfInt64>(ar, i);
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

// HH\\dict
Array HHVM_FUNCTION(HH_dict, const Variant& input) {
  return input.toDict();
}

// HH\\keyset
Array HHVM_FUNCTION(HH_keyset, const Variant& input) {
  return input.toKeyset();
}

// HH\\vec
Array HHVM_FUNCTION(HH_vec, const Variant& input) {
  return input.toVecArray();
}

// HH\\varray
Array HHVM_FUNCTION(HH_varray, const Variant& input) {
  return input.toVArray();
}

// HH\\darray
Array HHVM_FUNCTION(HH_darray, const Variant& input) {
  return input.toDArray();
}

TypedValue HHVM_FUNCTION(HH_array_key_cast, const Variant& input) {
  switch (input.getType()) {
    case KindOfPersistentString:
    case KindOfString: {
      int64_t n;
      auto const& str = input.asCStrRef();
      if (str.get()->isStrictlyInteger(n)) {
        return tvReturn(n);
      }
      return tvReturn(str);
    }

    case KindOfInt64:
    case KindOfBoolean:
    case KindOfDouble:
    case KindOfResource:
      return tvReturn(input.toInt64());

    case KindOfUninit:
    case KindOfNull:
      return tvReturn(staticEmptyString());

    case KindOfPersistentVec:
    case KindOfVec:
      SystemLib::throwInvalidArgumentExceptionObject(
        "Vecs cannot be cast to an array-key"
      );
    case KindOfPersistentDict:
    case KindOfDict:
      SystemLib::throwInvalidArgumentExceptionObject(
        "Dicts cannot be cast to an array-key"
      );
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      SystemLib::throwInvalidArgumentExceptionObject(
        "Keysets cannot be cast to an array-key"
      );
    case KindOfPersistentArray:
    case KindOfArray:
      SystemLib::throwInvalidArgumentExceptionObject(
        "Arrays cannot be cast to an array-key"
      );
    case KindOfObject:
      SystemLib::throwInvalidArgumentExceptionObject(
        "Objects cannot be cast to an array-key"
      );

    case KindOfRef:
      break;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

struct ArrayExtension final : Extension {
  ArrayExtension() : Extension("array") {}
  void moduleInit() override {
    HHVM_RC_INT_SAME(UCOL_DEFAULT);

    HHVM_RC_INT_SAME(UCOL_PRIMARY);
    HHVM_RC_INT_SAME(UCOL_SECONDARY);
    HHVM_RC_INT_SAME(UCOL_TERTIARY);
    HHVM_RC_INT_SAME(UCOL_DEFAULT_STRENGTH);
    HHVM_RC_INT_SAME(UCOL_QUATERNARY);
    HHVM_RC_INT_SAME(UCOL_IDENTICAL);

    HHVM_RC_INT_SAME(UCOL_OFF);
    HHVM_RC_INT_SAME(UCOL_ON);

    HHVM_RC_INT_SAME(UCOL_SHIFTED);
    HHVM_RC_INT_SAME(UCOL_NON_IGNORABLE);

    HHVM_RC_INT_SAME(UCOL_LOWER_FIRST);
    HHVM_RC_INT_SAME(UCOL_UPPER_FIRST);

    HHVM_RC_INT_SAME(UCOL_FRENCH_COLLATION);
    HHVM_RC_INT_SAME(UCOL_ALTERNATE_HANDLING);
    HHVM_RC_INT_SAME(UCOL_CASE_FIRST);
    HHVM_RC_INT_SAME(UCOL_CASE_LEVEL);
    HHVM_RC_INT_SAME(UCOL_NORMALIZATION_MODE);
    HHVM_RC_INT_SAME(UCOL_STRENGTH);
    HHVM_RC_INT_SAME(UCOL_HIRAGANA_QUATERNARY_MODE);
    HHVM_RC_INT_SAME(UCOL_NUMERIC_COLLATION);

    HHVM_RC_INT(ARRAY_FILTER_USE_BOTH, 1);
    HHVM_RC_INT(ARRAY_FILTER_USE_KEY, 2);

    HHVM_RC_INT(CASE_LOWER, static_cast<int64_t>(CaseMode::LOWER));
    HHVM_RC_INT(CASE_UPPER, static_cast<int64_t>(CaseMode::UPPER));

    HHVM_RC_INT(COUNT_NORMAL, static_cast<int64_t>(CountMode::NORMAL));
    HHVM_RC_INT(COUNT_RECURSIVE, static_cast<int64_t>(CountMode::RECURSIVE));

    HHVM_RC_INT_SAME(SORT_ASC);
    HHVM_RC_INT_SAME(SORT_DESC);
    HHVM_RC_INT_SAME(SORT_FLAG_CASE);
    HHVM_RC_INT_SAME(SORT_LOCALE_STRING);
    HHVM_RC_INT_SAME(SORT_NATURAL);
    HHVM_RC_INT_SAME(SORT_NUMERIC);
    HHVM_RC_INT_SAME(SORT_REGULAR);
    HHVM_RC_INT_SAME(SORT_STRING);

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
    HHVM_FALIAS(HH\\dict, HH_dict);
    HHVM_FALIAS(HH\\vec, HH_vec);
    HHVM_FALIAS(HH\\keyset, HH_keyset);
    HHVM_FALIAS(HH\\varray, HH_varray);
    HHVM_FALIAS(HH\\darray, HH_darray);
    HHVM_FALIAS(HH\\array_key_cast, HH_array_key_cast);

    loadSystemlib();
  }
} s_array_extension;

}
