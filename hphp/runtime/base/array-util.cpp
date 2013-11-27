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

#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/ext/ext_math.h"
#include "hphp/runtime/ext/ext_json.h"
#include "hphp/runtime/ext/ext_string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// compositions

Variant ArrayUtil::CreateArray(CArrRef keys, CVarRef value) {
  ArrayInit ai(keys.size());
  for (ArrayIter iter(keys); iter; ++iter) {
    ai.set(iter.secondRef(), value);
  }
  return ai.create();
}

Variant ArrayUtil::CreateArray(int start_index, int num, CVarRef value) {
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

Variant ArrayUtil::Chunk(CArrRef input, int size,
                         bool preserve_keys /* = false */) {
  if (size < 1) {
    throw_invalid_argument("size: %d", size);
    return init_null();
  }

  Array ret = Array::Create();
  Array chunk;
  int current = 0;
  for (ArrayIter iter(input); iter; ++iter) {
    if (preserve_keys) {
      chunk.setWithRef(iter.first(), iter.secondRef(), true);
    } else {
      chunk.appendWithRef(iter.secondRef());
    }
    if ((++current % size) == 0) {
      ret.append(chunk);
      chunk.clear();
    }
  }

  if (!chunk.empty()) {
    ret.append(chunk);
  }
  return ret;
}

Variant ArrayUtil::Slice(CArrRef input, int offset, int64_t length,
                         bool preserve_keys) {
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

  Array out_hash = Array::Create();
  int pos = 0;
  ArrayIter iter(input);
  for (; pos < offset && iter; ++pos, ++iter) {}
  for (; pos < offset + length && iter; ++pos, ++iter) {
    Variant key(iter.first());
    bool doAppend = !preserve_keys && key.isNumeric();
    CVarRef v = iter.secondRef();
    if (doAppend) {
      out_hash.appendWithRef(v);
    } else {
      out_hash.setWithRef(key, v, true);
    }
  }
  return out_hash;
}

Variant ArrayUtil::Splice(CArrRef input, int offset, int64_t length /* = 0 */,
                          CVarRef replacement /* = null_variant */,
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

  Array out_hash = Array::Create();
  int pos = 0;
  ArrayIter iter(input);
  for (; pos < offset && iter; ++pos, ++iter) {
    Variant key(iter.first());
    CVarRef v = iter.secondRef();
    if (key.isNumeric()) {
      out_hash.appendWithRef(v);
    } else {
      out_hash.setWithRef(key, v, true);
    }
  }

  for (; pos < offset + length && iter; ++pos, ++iter) {
    if (removed) {
      Variant key(iter.first());
      CVarRef v = iter.secondRef();
      if (key.isNumeric()) {
        removed->appendWithRef(v);
      } else {
        removed->setWithRef(key, v, true);
      }
    }
  }

  Array arr = replacement.toArray();
  if (!arr.empty()) {
    for (ArrayIter iter(arr); iter; ++iter) {
      CVarRef v = iter.secondRef();
      out_hash.appendWithRef(v);
    }
  }

  for (; iter; ++iter) {
    Variant key(iter.first());
    CVarRef v = iter.secondRef();
    if (key.isNumeric()) {
      out_hash.appendWithRef(v);
    } else {
      out_hash.setWithRef(key, v, true);
    }
  }

  return out_hash;
}

Variant ArrayUtil::Pad(CArrRef input, CVarRef pad_value, int pad_size,
                       bool pad_right /* = true */) {
  int input_size = input.size();
  if (input_size >= pad_size) {
    return input;
  }

  Array ret = Array::Create();
  if (pad_right) {
    ret = input;
    for (int i = input_size; i < pad_size; i++) {
      ret.append(pad_value);
    }
  } else {
    for (int i = input_size; i < pad_size; i++) {
      ret.append(pad_value);
    }
    for (ArrayIter iter(input); iter; ++iter) {
      Variant key(iter.first());
      if (key.isNumeric()) {
        ret.appendWithRef(iter.secondRef());
      } else {
        ret.setWithRef(key, iter.secondRef(), true);
      }
    }
  }
  return ret;
}

Variant ArrayUtil::Range(unsigned char low, unsigned char high,
                         int64_t step /* = 1 */) {
  if (step <= 0) {
    throw_invalid_argument("step exceeds the specified range");
    return false;
  }

  Array ret;
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

Variant ArrayUtil::Range(double low, double high, double step /* = 1.0 */) {
  Array ret;
  if (low > high) { // Negative steps
    if (low - high < step || step <= 0) {
      throw_invalid_argument("step exceeds the specified range");
      return false;
    }
    for (; low >= (high - DOUBLE_DRIFT_FIX); low -= step) {
      ret.append(low);
    }
  } else if (high > low) { // Positive steps
    if (high - low < step || step <= 0) {
      throw_invalid_argument("step exceeds the specified range");
      return false;
    }
    for (; low <= (high + DOUBLE_DRIFT_FIX); low += step) {
      ret.append(low);
    }
  } else {
    ret.append(low);
  }
  return ret;
}

Variant ArrayUtil::Range(double low, double high, int64_t step /* = 1 */) {
  Array ret;
  if (low > high) { // Negative steps
    if (low - high < step || step <= 0) {
      throw_invalid_argument("step exceeds the specified range");
      return false;
    }
    for (; low >= high; low -= step) {
      ret.append((int64_t)low);
    }
  } else if (high > low) { // Positive steps
    if (high - low < step || step <= 0) {
      throw_invalid_argument("step exceeds the specified range");
      return false;
    }
    for (; low <= high; low += step) {
      ret.append((int64_t)low);
    }
  } else {
    ret.append((int64_t)low);
  }
  return ret;
}

const StaticString s_default("(default)");

Variant ArrayUtil::FromHdf(const Hdf &hdf) {
  if (hdf.firstChild().exists()) {
    Array ret = Array::Create();

    const char *value = hdf.get();
    if (value) {
      ret.set(s_default, String(value, CopyString));
    }

    for (Hdf child = hdf.firstChild(); child.exists(); child = child.next()) {
      ret.set(String(child.getName()), FromHdf(child));
    }
    return ret;
  }

  const char *value = hdf.get("");
  if (strcasecmp(value, "false") == 0 ||
      strcasecmp(value, "no") == 0 ||
      strcasecmp(value, "off") == 0) {
    return false;
  }
  if (strcasecmp(value, "true") == 0 ||
      strcasecmp(value, "yes") == 0 ||
      strcasecmp(value, "on") == 0) {
    return true;
  }

  int64_t lval; double dval;
  int len = strlen(value);
  DataType ret = is_numeric_string(value, len, &lval, &dval, 0);
  switch (ret) {
  case KindOfInt64:  return lval;
  case KindOfDouble: return dval;
  default: break;
  }
  return String(value, len, CopyString);
}

void ArrayUtil::ToHdf(const Array &arr, Hdf &hdf) {
  for (ArrayIter iter(arr); iter; ++iter) {
    CVarRef value(iter.secondRef());
    if (value.isArray()) {
      Hdf child = hdf[iter.first().toString().data()];
      ToHdf(iter.secondRef().toArray(), child);
    } else if (value.isBoolean()) {
      hdf[iter.first().toString().data()] =
        iter.secondRef().toBoolean() ? "true" : "false";
    } else {
      hdf[iter.first().toString().data()] = iter.secondRef().toString().data();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// information and calculations

DataType ArrayUtil::Sum(CArrRef input, int64_t *isum, double *dsum) {
  int64_t i = 0;
  ArrayIter iter(input);
  for (; iter; ++iter) {
    CVarRef entry(iter.secondRef());
    switch (entry.getType()) {
    case KindOfDouble: {
      goto DOUBLE;
    }
    case KindOfStaticString:
    case KindOfString: {
      int64_t ti;
      double td;
      if (entry.getStringData()->isNumericWithVal(ti, td, 1) ==
          KindOfInt64) {
        i += ti;
        break;
      } else {
        goto DOUBLE;
      }
    }
    case KindOfArray:
    case KindOfObject:
    case KindOfResource: {
      break;
    }
    default: {
      i += entry.toInt64();
      break;
    }
    }
  }
  *isum = i;
  return KindOfInt64;

DOUBLE:
  double d = i;
  for (; iter; ++iter) {
    CVarRef entry(iter.secondRef());
    if (!entry.is(KindOfArray) && !entry.is(KindOfObject) &&
        !entry.is(KindOfResource)) {
      d += entry.toDouble();
    }
  }
  *dsum = d;
  return KindOfDouble;
}

DataType ArrayUtil::Product(CArrRef input, int64_t *iprod, double *dprod) {
  int64_t i = 1;
  ArrayIter iter(input);
  for (; iter; ++iter) {
    CVarRef entry(iter.secondRef());
    switch (entry.getType()) {
    case KindOfDouble: {
      goto DOUBLE;
    }
    case KindOfStaticString:
    case KindOfString: {
      int64_t ti;
      double td;
      if (entry.getStringData()->isNumericWithVal(ti, td, 1) ==
          KindOfInt64) {
        i *= ti;
        break;
      } else {
        goto DOUBLE;
      }
    }
    case KindOfArray:
    case KindOfObject:
    case KindOfResource: {
      break;
    }
    default: {
      i *= entry.toInt64();
      break;
    }
    }
  }
  *iprod = i;
  return KindOfInt64;

DOUBLE:
  double d = i;
  for (; iter; ++iter) {
    CVarRef entry(iter.secondRef());
    if (!entry.is(KindOfArray) && !entry.is(KindOfObject) &&
        !entry.is(KindOfResource)) {
      d *= entry.toDouble();
    }
  }
  *dprod = d;
  return KindOfDouble;
}

Variant ArrayUtil::CountValues(CArrRef input) {
  Array ret = Array::Create();
  for (ArrayIter iter(input); iter; ++iter) {
    CVarRef entry(iter.secondRef());
    if (entry.isInteger() || entry.isString()) {
      if (!ret.exists(entry)) {
        ret.set(entry, 1);
      } else {
        ret.set(entry, ret[entry].toInt64() + 1);
      }
    } else {
      raise_warning("Can only count STRING and INTEGER values!");
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// manipulations

Variant ArrayUtil::ChangeKeyCase(CArrRef input, bool lower) {
  Array ret = Array::Create();
  for (ArrayIter iter(input); iter; ++iter) {
    Variant key(iter.first());
    if (key.isString()) {
      if (lower) {
        ret.set(f_strtolower(key.toString()), iter.secondRef());
      } else {
        ret.set(f_strtoupper(key.toString()), iter.secondRef());
      }
    } else {
      ret.set(key, iter.secondRef());
    }
  }
  return ret;
}

Variant ArrayUtil::Reverse(CArrRef input, bool preserve_keys /* = false */) {
  if (input.empty()) {
    return input;
  }

  Array ret = Array::Create();
  for (ssize_t pos = input->iter_end(); pos != ArrayData::invalid_index;
       pos = input->iter_rewind(pos)) {
    Variant key(input->getKey(pos));
    if (preserve_keys || key.isString()) {
      ret.setWithRef(key, input->getValueRef(pos), true);
    } else {
      ret.appendWithRef(input->getValueRef(pos));
    }
  }
  return ret;
}

static void php_array_data_shuffle(std::vector<ssize_t> &indices) {
  int n_elems = indices.size();
  if (n_elems > 1) {
    int n_left = n_elems;
    while (--n_left) {
      int rnd_idx = f_rand(0, n_left);
      if (rnd_idx != n_left) {
        ssize_t temp = indices[n_left];
        indices[n_left] = indices[rnd_idx];
        indices[rnd_idx] = temp;
      }
    }
  }
}

Variant ArrayUtil::Shuffle(CArrRef input) {
  int count = input.size();
  if (count == 0) {
    return input;
  }

  std::vector<ssize_t> indices;
  indices.reserve(count);
  for (ssize_t pos = input->iter_begin(); pos != ArrayData::invalid_index;
       pos = input->iter_advance(pos)) {
    indices.push_back(pos);
  }
  php_array_data_shuffle(indices);

  Array ret = Array::Create();
  for (int i = 0; i < count; i++) {
    ssize_t pos = indices[i];
    ret.appendWithRef(input->getValueRef(pos));
  }
  return ret;
}

Variant ArrayUtil::RandomKeys(CArrRef input, int num_req /* = 1 */) {
  int count = input.size();
  if (num_req <= 0 || num_req > count) {
    raise_warning("Second argument has to be between 1 and the "
                  "number of elements in the array");
    return init_null();
  }

  std::vector<ssize_t> indices;
  indices.reserve(count);
  for (ssize_t pos = input->iter_begin(); pos != ArrayData::invalid_index;
       pos = input->iter_advance(pos)) {
    indices.push_back(pos);
  }
  php_array_data_shuffle(indices);

  if (num_req == 1) {
    return input->getKey(indices[0]);
  }

  Array ret = Array::Create();
  for (int i = 0; i < num_req; i++) {
    ssize_t pos = indices[i];
    ret.append(input->getKey(pos));
  }
  return ret;
}

Variant ArrayUtil::StringUnique(CArrRef input) {
  Array seenValues;
  Array ret = Array::Create();
  for (ArrayIter iter(input); iter; ++iter) {
    CVarRef entry(iter.secondRef());
    String str(entry.toString());
    if (!seenValues.exists(str)) {
      seenValues.set(str, 1);
      ret.set(iter.first(), entry);
    }
  }
  return ret;
}

Variant ArrayUtil::NumericUnique(CArrRef input) {
  std::set<double> seenValues;
  Array ret = Array::Create();
  for (ArrayIter iter(input); iter; ++iter) {
    CVarRef entry(iter.secondRef());
    double value = entry.toDouble();
    std::pair<std::set<double>::iterator, bool> res =
      seenValues.insert(value);
    if (res.second) { // it was inserted
      ret.set(iter.first(), entry);
    }
  }
  return ret;
}

Variant ArrayUtil::RegularSortUnique(CArrRef input) {
  /* The output of this function in PHP strictly depends on the implementation
   * of the sort function and on whether values that compare as equal end
   * up in contiguous positions in the sorted array (which is not really
   * well-defined in case of mixed strings/numbers). To get the same result
   * as in PHP we thus need to replicate the PHP algorithm more closely than
   * in the other versions of array_unique.
   */
  if (input.size() <= 1) return input;

  Array::SortData opaque;
  vector<int> indices;
  Array::SortImpl(indices, input, opaque, Array::SortRegularAscending, false);

  vector<bool> duplicates(indices.size(), false);
  int lastIdx = indices[0];
  Variant last = input->getValue(opaque.positions[lastIdx]);
  for (unsigned int i = 1; i < indices.size(); ++i) {
    int currentIdx = indices[i];
    Variant current = input->getValue(opaque.positions[currentIdx]);
    if (equal(current, last)) {
      if (currentIdx > lastIdx) {
        duplicates[currentIdx] = true;
        continue;
      }
      duplicates[lastIdx] = true;
    }
    lastIdx = currentIdx;
    last = current;
  }

  Array ret = Array::Create();
  int i = 0;
  for (ArrayIter iter(input); iter; ++iter, ++i) {
    if (!duplicates[i]) ret.set(iter.first(), iter.secondRef());
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// iterations

void ArrayUtil::Walk(VRefParam input, PFUNC_WALK walk_function,
                     const void *data, bool recursive /* = false */,
                     PointerSet *seen /* = NULL */,
                     CVarRef userdata /* = null_variant */) {
  assert(walk_function);

  Variant k;
  Variant v;
  for (MutableArrayIter iter = input->begin(&k, v); iter.advance(); ) {
    if (recursive && v.is(KindOfArray)) {
      assert(seen);
      ArrayData *arr = v.getArrayData();

      if (v.isReferenced()) {
        if (seen->find((void*)arr) != seen->end()) {
          raise_warning("array_walk_recursive(): recursion detected");
          return;
        }
        seen->insert((void*)arr);
      }

      Walk(directRef(v), walk_function, data, recursive, seen, userdata);
      if (v.isReferenced()) {
        seen->erase((void*)arr);
      }
    } else {
      walk_function(directRef(v), k, userdata, data);
    }
  }
}

Variant ArrayUtil::Reduce(CArrRef input, PFUNC_REDUCE reduce_function,
                          const void *data,
                          CVarRef initial /* = null_variant */) {
  Variant result(initial);
  for (ArrayIter iter(input); iter; ++iter) {
    result = reduce_function(result, iter.second(), data);
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
}
