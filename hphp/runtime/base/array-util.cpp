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

#include "hphp/runtime/base/array-util.h"

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/request-info.h"

#include "hphp/runtime/ext/std/ext_std_math.h"
#include "hphp/runtime/ext/string/ext_string.h"

#include <folly/Optional.h>

#include <set>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// compositions

Variant ArrayUtil::Splice(const Array& input, int offset, int64_t length /* = 0 */,
                          const Variant& replacement /* = uninit_variant */,
                          Array *removed /* = NULL */) {
  int num_in = input.size();
  if (offset > num_in) {
    offset = num_in;
  } else if (offset < 0 && (offset = (num_in + offset)) < 0) {
    offset = 0;
  }

  if (length < 0) {
    length = num_in - offset + length;
  } else if (((unsigned)offset + (unsigned)length) > (unsigned)num_in) {
    length = num_in - offset;
  }

  Array out_hash = Array::CreateDArray();
  int pos = 0;
  ArrayIter iter(input);
  for (; pos < offset && iter; ++pos, ++iter) {
    Variant key(iter.first());
    auto const v = iter.secondVal();
    if (key.isNumeric()) {
      out_hash.append(v);
    } else {
      out_hash.set(key, v, true);
    }
  }

  for (; pos < offset + length && iter; ++pos, ++iter) {
    if (removed) {
      Variant key(iter.first());
      auto const v = iter.secondVal();
      if (key.isNumeric()) {
        removed->append(v);
      } else {
        removed->set(key, v, true);
      }
    }
  }

  Array arr = replacement.toArray();
  if (!arr.empty()) {
    for (ArrayIter iterb(arr); iterb; ++iterb) {
      auto const v = iterb.secondVal();
      out_hash.append(v);
    }
  }

  for (; iter; ++iter) {
    Variant key(iter.first());
    auto const v = iter.secondVal();
    if (key.isNumeric()) {
      out_hash.append(v);
    } else {
      out_hash.set(key, v, true);
    }
  }

  return out_hash;
}

Variant ArrayUtil::PadRight(const Array& input, const Variant& pad_value,
                            int pad_size) {
  int input_size = input.size();
  if (input_size >= pad_size) {
    return input;
  }

  Array ret = input;
  for (int i = input_size; i < pad_size; i++) {
    ret.append(pad_value);
  }
  return ret;
}

Variant ArrayUtil::PadLeft(const Array& input, const Variant& pad_value,
                           int pad_size) {
  int input_size = input.size();
  if (input_size >= pad_size) {
    return input;
  }

  ArrayData* data;
  if (input->isVecType() || input->isVArray()) {
    data = PackedArray::MakeReserveVArray(pad_size);
  } else {
    data = MixedArray::MakeReserveDArray(pad_size);
  }
  auto ret = Array::attach(data);
  for (int i = input_size; i < pad_size; i++) {
    ret.append(pad_value);
  }
  for (ArrayIter iter(input); iter; ++iter) {
    Variant key(iter.first());
    auto const v = iter.secondVal();
    if (key.isNumeric()) {
      ret.append(v);
    } else {
      ret.set(key, v, true);
    }
  }
  return ret;
}

Variant ArrayUtil::Range(unsigned char low, unsigned char high,
                         int64_t step /* = 1 */) {
  if (step <= 0) {
    raise_invalid_argument_warning("step exceeds the specified range");
    return false;
  }

  auto ret = Array::CreateVArray();
  if (low > high) { // Negative Steps
    for (; low >= high; low -= (unsigned int)step) {
      ret.append(String::FromChar(low));
      if (((signed int)low - step) < 0) {
        break;
      }
    }
  } else if (high > low) { // Positive Steps
    for (; low <= high; low += (unsigned int)step) {
      ret.append(String::FromChar(low));
      if (((signed int)low + step) > 255) {
        break;
      }
    }
  } else {
    ret.append(String::FromChar(low));
  }
  return ret;
}

#define DOUBLE_DRIFT_FIX 0.000000000000001

namespace {
// Some inputs can cause us to allocate gigantic arrays, so we have to make sure
// we're not exceeding the memory limit.
void rangeCheckAlloc(double estNumSteps) {
  // An array can hold at most INT_MAX elements
  if (estNumSteps > std::numeric_limits<int32_t>::max()) {
    tl_heap->forceOOM();
    check_non_safepoint_surprise();
    return;
  }

  int32_t numElms = static_cast<int32_t>(estNumSteps);
  if (tl_heap->preAllocOOM(MixedArray::computeAllocBytesFromMaxElms(numElms))) {
    check_non_safepoint_surprise();
  }
}
}

Variant ArrayUtil::Range(double low, double high, double step /* = 1.0 */) {
  auto ret = Array::CreateVArray();
  double element;
  int64_t i;
  if (low > high) { // Negative steps
    if (low - high < step || step <= 0) {
      raise_invalid_argument_warning("step exceeds the specified range");
      return false;
    }
    rangeCheckAlloc((low - high) / step);
    for (i = 0, element = low; element >= (high - DOUBLE_DRIFT_FIX);
         i++, element = low - (i * step)) {
      ret.append(element);
    }
  } else if (high > low) { // Positive steps
    if (high - low < step || step <= 0) {
      raise_invalid_argument_warning("step exceeds the specified range");
      return false;
    }
    rangeCheckAlloc((high - low) / step);
    for (i = 0, element = low; element <= (high + DOUBLE_DRIFT_FIX);
         i++, element = low + (i * step)) {
      ret.append(element);
    }
  } else {
    ret.append(low);
  }
  return ret;
}

Variant ArrayUtil::Range(int64_t low, int64_t high, int64_t step /* = 1 */) {
  auto ret = Array::CreateVArray();
  if (low > high) { // Negative steps
    if (low - high < step || step <= 0) {
      raise_invalid_argument_warning("step exceeds the specified range");
      return false;
    }
    rangeCheckAlloc((low - high) / step);
    for (; low >= high; low -= step) {
      ret.append(low);
    }
  } else if (high > low) { // Positive steps
    if (high - low < step || step <= 0) {
      raise_invalid_argument_warning("step exceeds the specified range");
      return false;
    }
    rangeCheckAlloc((high - low) / step);
    for (; low <= high; low += step) {
      ret.append(low);
    }
  } else {
    ret.append(low);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// information and calculations

Variant ArrayUtil::CountValues(const Array& input) {
  Array ret = Array::CreateDArray();
  for (ArrayIter iter(input); iter; ++iter) {
    auto const inner = iter.secondVal();
    if (isIntType(type(inner)) || isStringType(type(inner)) ||
        isFuncType(type(inner)) || isClassType(type(inner))) {
      auto const inner_key = ret.convertKey<IntishCast::Cast>(inner);
      if (!ret.exists(inner_key)) {
        ret.set(inner_key, make_tv<KindOfInt64>(1));
      } else {
        ret.set(inner_key,
                make_tv<KindOfInt64>(ret[inner_key].toInt64() + 1));
      }
    } else {
      raise_warning("Can only count STRING and INTEGER values!");
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// manipulations

Variant ArrayUtil::ChangeKeyCase(const Array& input, bool lower) {
  Array ret = Array::CreateDArray();
  for (ArrayIter iter(input); iter; ++iter) {
    Variant key(iter.first());
    if (key.isString()) {
      if (lower) {
        ret.set(HHVM_FN(strtolower)(key.toString()), iter.secondVal());
      } else {
        ret.set(HHVM_FN(strtoupper)(key.toString()), iter.secondVal());
      }
    } else {
      ret.set(key, iter.secondVal());
    }
  }
  return ret;
}

Variant ArrayUtil::Reverse(const Array& input, bool preserve_keys /* = false */) {
  if (input.empty()) {
    return empty_darray();
  }

  auto ret = Array::CreateDArray();
  auto pos_limit = input->iter_end();
  for (ssize_t pos = input->iter_last(); pos != pos_limit;
       pos = input->iter_rewind(pos)) {
    auto const key = input->nvGetKey(pos);
    if (preserve_keys || isStringType(key.m_type)) {
      ret.set(key, input->nvGetVal(pos), true);
    } else {
      ret.append(input->nvGetVal(pos));
    }
  }
  return ret;
}

static void php_array_data_shuffle(std::vector<ssize_t> &indices) {
  int n_elems = indices.size();
  if (n_elems > 1) {
    int n_left = n_elems;
    while (--n_left) {
      int rnd_idx = HHVM_FN(rand)(0, n_left);
      if (rnd_idx != n_left) {
        ssize_t temp = indices[n_left];
        indices[n_left] = indices[rnd_idx];
        indices[rnd_idx] = temp;
      }
    }
  }
}

Variant ArrayUtil::Shuffle(const Array& input) {
  int count = input.size();
  if (count == 0) {
    return input;
  }

  std::vector<ssize_t> indices;
  indices.reserve(count);
  auto pos_limit = input->iter_end();
  for (ssize_t pos = input->iter_begin(); pos != pos_limit;
       pos = input->iter_advance(pos)) {
    indices.push_back(pos);
  }
  php_array_data_shuffle(indices);

  if (input.isVec()) {
    VecInit ret(count);
    for (int i = 0; i < count; i++) {
      ssize_t pos = indices[i];
      ret.append(input->nvGetVal(pos));
    }
    return ret.toVariant();
  } else if (input.isDict()) {
    DictInit ret(count);
    for (int i = 0; i < count; i++) {
      ssize_t pos = indices[i];
      ret.append(input->nvGetVal(pos));
    }
    return ret.toVariant();
  } else if (input.isKeyset()) {
    KeysetInit ret(count);
    for (int i = 0; i < count; i++) {
      ssize_t pos = indices[i];
      ret.add(input->nvGetVal(pos));
    }
    return ret.toVariant();
  } else if (input.isVArray()) {
    VArrayInit ret(count);
    for (int i = 0; i < count; i++) {
      ssize_t pos = indices[i];
      ret.append(input->nvGetVal(pos));
    }
    return ret.toVariant();
  } else {
    DArrayInit ret(count);
    for (int i = 0; i < count; i++) {
      ssize_t pos = indices[i];
      ret.append(input->nvGetVal(pos));
    }
    return ret.toVariant();
  }
}

Variant ArrayUtil::RandomKeys(const Array& input, int num_req /* = 1 */) {
  int count = input.size();
  if (num_req <= 0 || num_req > count) {
    raise_warning("Second argument has to be between 1 and the "
                  "number of elements in the array");
    return init_null();
  }

  if (num_req == 1) {
    // Iterating through the counter is correct but a bit inefficient
    // compared to being able to access the right offset into array data,
    // but necessary for this code to be agnostic to the array's internal
    // representation.  Assuming uniform distribution, we'll expect to
    // iterate through half of the array's data.
    ssize_t index = HHVM_FN(rand)(0, count-1);
    ssize_t pos = input->iter_begin();
    while (index--) {
      pos = input->iter_advance(pos);
    }
    return input->getKey(pos);
  }

  std::vector<ssize_t> indices;
  indices.reserve(count);
  auto pos_limit = input->iter_end();
  for (ssize_t pos = input->iter_begin(); pos != pos_limit;
       pos = input->iter_advance(pos)) {
    indices.push_back(pos);
  }
  php_array_data_shuffle(indices);

  VArrayInit ret(num_req);
  for (int i = 0; i < num_req; i++) {
    ssize_t pos = indices[i];
    ret.append(input->getKey(pos));
  }
  return ret.toVariant();
}

Variant ArrayUtil::StringUnique(const Array& input) {
  Array seenValues = Array::CreateKeyset();
  Array ret = Array::CreateDArray();
  for (ArrayIter iter(input); iter; ++iter) {
    auto const str = tvCastToString(iter.secondVal());
    if (!seenValues.exists(str)) {
      seenValues.append(str);
      ret.set(ret.convertKey<IntishCast::Cast>(iter.first()),
                                                       iter.secondVal());
    }
  }
  return ret;
}

Variant ArrayUtil::NumericUnique(const Array& input) {
  std::set<double> seenValues;
  Array ret = Array::CreateDArray();
  for (ArrayIter iter(input); iter; ++iter) {
    auto const value = tvCastToDouble(iter.secondVal());
    std::pair<std::set<double>::iterator, bool> res =
      seenValues.insert(value);
    if (res.second) { // it was inserted
      ret.set(ret.convertKey<IntishCast::Cast>(iter.first()),
                                                       iter.secondVal());
    }
  }
  return ret;
}

Variant ArrayUtil::RegularSortUnique(const Array& input) {
  /* The output of this function in PHP strictly depends on the implementation
   * of the sort function and on whether values that compare as equal end
   * up in contiguous positions in the sorted array (which is not really
   * well-defined in case of mixed strings/numbers). To get the same result
   * as in PHP we thus need to replicate the PHP algorithm more closely than
   * in the other versions of array_unique.
   */
  if (input.size() <= 1) return input;

  Array::SortData opaque;
  std::vector<int> indices;
  Array::SortImpl(indices, input, opaque, Array::SortRegularAscending, false);

  int duplicates_count = 0;
  std::vector<bool> duplicates(indices.size(), false);
  int lastIdx = indices[0];
  Variant last = input->getValue(opaque.positions[lastIdx]);
  for (unsigned int i = 1; i < indices.size(); ++i) {
    int currentIdx = indices[i];
    Variant current = input->getValue(opaque.positions[currentIdx]);
    if (equal(current, last)) {
      ++duplicates_count;
      if (currentIdx > lastIdx) {
        duplicates[currentIdx] = true;
        continue;
      }
      duplicates[lastIdx] = true;
    }
    lastIdx = currentIdx;
    last = current;
  }

  ArrayInit ret(indices.size() - duplicates_count, ArrayInit::Map{});
  int i = 0;
  for (ArrayIter iter(input); iter; ++iter, ++i) {
    if (!duplicates[i]) {
      ret.setValidKey(*iter.first().asTypedValue(), iter.secondVal());
    }
  }
  return ret.toVariant();
}

///////////////////////////////////////////////////////////////////////////////
// iterations

Variant ArrayUtil::Reduce(const Array& input, PFUNC_REDUCE reduce_function,
                          const void *data,
                          const Variant& initial /* = uninit_variant */) {
  Variant result(initial);
  for (ArrayIter iter(input); iter; ++iter) {
    result = reduce_function(result, iter.second(), data);
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
}
