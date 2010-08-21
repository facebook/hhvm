/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/array/array_util.h>
#include <runtime/base/string_util.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/runtime_error.h>
#include <runtime/ext/ext_math.h>
#include <runtime/ext/ext_json.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// compositions

Variant ArrayUtil::CreateArray(CArrRef keys, CVarRef value) {
  Array ret = Array::Create();
  for (ArrayIter iter(keys); iter; ++iter) {
    ret.set(iter.second(), value);
  }
  return ret;
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

Variant ArrayUtil::Combine(CArrRef keys, CArrRef values) {
  if (keys.size() != values.size()) {
    throw_invalid_argument("keys and values not same count");
    return false;
  }
  if (keys.empty()) {
    throw_invalid_argument("keys and values empty");
    return false;
  }

  Array ret = Array::Create();
  for (ArrayIter iter1(keys), iter2(values); iter1; ++iter1, ++iter2) {
    if (values->supportValueRef()) {
      CVarRef v(iter2.secondRef());
      if (v.isReferenced()) {
        ret.set(iter1.second(), ref(v));
      } else {
        ret.set(iter1.second(), v);
      }
    } else {
      ret.set(iter1.second(), iter2.second());
    }
  }
  return ret;
}

Variant ArrayUtil::Chunk(CArrRef input, int size,
                         bool preserve_keys /* = false */) {
  if (size < 1) {
    throw_invalid_argument("size: %d", size);
    return null;
  }

  Array ret = Array::Create();
  Array chunk;
  int current = 0;
  for (ArrayIter iter(input); iter; ++iter) {
    if (preserve_keys) {
      chunk.set(iter.first(), iter.second());
    } else {
      chunk.append(iter.second());
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

Variant ArrayUtil::Slice(CArrRef input, int offset, int length,
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
  bool supportRef = input->supportValueRef();
  for (; pos < offset && iter; ++pos, ++iter) {}
  for (; pos < offset + length && iter; ++pos, ++iter) {
    bool doAppend = !preserve_keys && iter.first().isNumeric();
    if (supportRef) {
      CVarRef v = iter.secondRef();
      if (v.isReferenced()) v.setContagious();
      if (doAppend) {
        out_hash.append(v);
      } else {
        out_hash.set(iter.first(), v);
      }
    } else {
      if (doAppend) {
        out_hash.append(iter.second());
      } else {
        out_hash.set(iter.first(), iter.second());
      }
    }
  }
  return out_hash;
}

Variant ArrayUtil::Splice(CArrRef input, int offset, int length /* = 0 */,
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
    if (iter.first().isNumeric()) {
      out_hash.append(iter.second());
    } else {
      out_hash.set(iter.first(), iter.second());
    }
  }

  for (; pos < offset + length && iter; ++pos, ++iter) {
    if (removed) {
      if (iter.first().isNumeric()) {
        removed->append(iter.second());
      } else {
        removed->set(iter.first(), iter.second());
      }
    }
  }

  Array arr = replacement.toArray();
  if (!arr.empty()) {
    for (ArrayIter iter(arr); iter; ++iter) {
      out_hash.append(iter.second());
    }
  }

  for (; iter; ++iter) {
    if (iter.first().isNumeric()) {
      out_hash.append(iter.second());
    } else {
      out_hash.set(iter.first(), iter.second());
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
      if (iter.first().isNumeric()) {
        ret.append(iter.second());
      } else {
        ret.set(iter.first(), iter.second());
      }
    }
  }
  return ret;
}

Variant ArrayUtil::Range(unsigned char low, unsigned char high,
                         int64 step /* = 1 */) {
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

Variant ArrayUtil::Range(double low, double high, int64 step /* = 1 */) {
  Array ret;
  if (low > high) { // Negative steps
    if (low - high < step || step <= 0) {
      throw_invalid_argument("step exceeds the specified range");
      return false;
    }
    for (; low >= high; low -= step) {
      ret.append((int64)low);
    }
  } else if (high > low) { // Positive steps
    if (high - low < step || step <= 0) {
      throw_invalid_argument("step exceeds the specified range");
      return false;
    }
    for (; low <= high; low += step) {
      ret.append((int64)low);
    }
  } else {
    ret.append((int64)low);
  }
  return ret;
}

Variant ArrayUtil::FromHdf(const Hdf &hdf) {
  if (hdf.firstChild().exists()) {
    Array ret = Array::Create();

    const char *value = hdf.get();
    if (value) {
      ret.set("(default)", String(value, CopyString));
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

  int64 lval; double dval;
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
    Variant value(iter.second());
    if (value.isArray()) {
      Hdf child = hdf[iter.first().toString().data()];
      ToHdf(iter.second().toArray(), child);
    } else if (value.isBoolean()) {
      hdf[iter.first().toString().data()] =
        iter.second().toBoolean() ? "true" : "false";
    } else {
      hdf[iter.first().toString().data()] = iter.second().toString().data();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// information and calculations

DataType ArrayUtil::Sum(CArrRef input, int64 *isum, double *dsum) {
  int64 i = 0;
  ArrayIter iter(input);
  for (; iter; ++iter) {
    Variant entry(iter.second());
    switch (entry.getType()) {
    case KindOfDouble: {
      goto DOUBLE;
    }
    case KindOfStaticString:
    case LiteralString:
    case KindOfString: {
      String s = entry.toString();
      int64 ti;
      double td;
      if (is_numeric_string(s.data(), s.size(), &ti, &td, 1) == KindOfInt64) {
        i += ti;
        break;
      } else {
        goto DOUBLE;
      }
    }
    case KindOfArray:
    case KindOfObject: {
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
    Variant entry(iter.second());
    if (!entry.is(KindOfArray) && !entry.is(KindOfObject)) {
      d += entry.toDouble();
    }
  }
  *dsum = d;
  return KindOfDouble;
}

DataType ArrayUtil::Product(CArrRef input, int64 *iprod, double *dprod) {
  int64 i = 1;
  ArrayIter iter(input);
  for (; iter; ++iter) {
    Variant entry(iter.second());
    switch (entry.getType()) {
    case KindOfDouble: {
      goto DOUBLE;
    }
    case KindOfStaticString:
    case LiteralString:
    case KindOfString: {
      String s = entry.toString();
      int64 ti;
      double td;
      if (is_numeric_string(s.data(), s.size(), &ti, &td, 1) == KindOfInt64) {
        i *= ti;
        break;
      } else {
        goto DOUBLE;
      }
    }
    case KindOfArray:
    case KindOfObject: {
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
    Variant entry(iter.second());
    if (!entry.is(KindOfArray) && !entry.is(KindOfObject)) {
      d *= entry.toDouble();
    }
  }
  *dprod = d;
  return KindOfDouble;
}

Variant ArrayUtil::CountValues(CArrRef input) {
  Array ret = Array::Create();
  for (ArrayIter iter(input); iter; ++iter) {
    Variant entry(iter.second());
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
        ret.set(StringUtil::ToLower(key.toString()), iter.second());
      } else {
        ret.set(StringUtil::ToUpper(key.toString()), iter.second());
      }
    } else {
      ret.set(key, iter.second());
    }
  }
  return ret;
}

Variant ArrayUtil::Flip(CArrRef input) {
  Array ret = Array::Create();
  for (ArrayIter iter(input); iter; ++iter) {
    Variant value(iter.second());
    if (value.isString() || value.isInteger()) {
      ret.set(value, iter.first());
    } else {
      raise_warning("Can only flip STRING and INTEGER values!");
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
      ret.set(key, input->getValue(pos));
    } else {
      ret.append(input->getValue(pos));
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
    ret.append(input->getValue(pos));
  }
  return ret;
}

Variant ArrayUtil::RandomKeys(CArrRef input, int num_req /* = 1 */) {
  int count = input.size();
  if (num_req <= 0 || num_req > count) {
    return null;
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

Variant ArrayUtil::RandomValues(CArrRef input, int num_req /* = 1 */) {
  int count = input.size();
  if (num_req <= 0 || num_req > count) {
    return null;
  }

  std::vector<ssize_t> indices;
  indices.reserve(count);
  for (ssize_t pos = input->iter_begin(); pos != ArrayData::invalid_index;
       pos = input->iter_advance(pos)) {
    indices.push_back(pos);
  }
  php_array_data_shuffle(indices);

  if (num_req == 1) {
    return input->getValue(indices[0]);
  }

  Array ret = Array::Create();
  for (int i = 0; i < num_req; i++) {
    ssize_t pos = indices[i];
    ret.append(input->getValue(pos));
  }
  return ret;
}

Variant ArrayUtil::Filter(CArrRef input, PFUNC_FILTER filter /* = NULL */,
                          const void *data /* = NULL */) {
  Array ret = Array::Create();
  for (ArrayIter iter(input); iter; ++iter) {
    Variant value(iter.second());
    if ((filter && filter(value, data)) || (!filter && value.toBoolean())) {
      ret.set(iter.first(), iter.second());
    }
  }
  return ret;
}

Variant ArrayUtil::Unique(CArrRef input) {
  Array seenValues;
  Array ret = Array::Create();
  for (ArrayIter iter(input); iter; ++iter) {
    Variant entry(iter.second());
    String str(entry.toString());
    if (!seenValues.exists(str)) {
      seenValues.set(str, 1);
      ret.set(iter.first(), entry);
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// iterations

void ArrayUtil::Walk(Variant input, PFUNC_WALK walk_function,
                     const void *data, bool recursive /* = false */,
                     PointerSet *seen /* = NULL */,
                     CVarRef userdata /* = null_variant */) {
  ASSERT(walk_function);
  input.escalate(); // so we can safely take secondRef()
  for (ArrayIter iter(input); iter; ++iter) {
    CVarRef v = iter.secondRef();
    if (recursive && v.is(KindOfArray)) {
      ASSERT(seen);
      Array arr = v.toArray();

      if (v.isReferenced()) {
        if (seen->find((void*)arr.get()) != seen->end()) {
          raise_warning("array_walk_recursive(): recursion detected");
          return;
        }
        seen->insert((void*)arr.get());
      }

      Walk(arr, walk_function, data, recursive, seen, userdata);
    } else {
      walk_function(ref(v), iter.first(), userdata, data);
    }
  }
}

Variant ArrayUtil::Map(CArrRef inputs, PFUNC_MAP map_function,
                       const void *data) {
  Array ret = Array::Create();

  if (inputs.size() == 1) {
    Array arr = inputs.begin().second().toArray();
    if (!arr.empty()) {
      for (ssize_t k = arr->iter_begin(); k != ArrayData::invalid_index;
           k = arr->iter_advance(k)) {
        Array params;
        params.append(arr->getValue(k));
        Variant result;
        if (map_function) {
          result = map_function(params, data);
        } else {
          result = params;
        }
        ret.set(arr->getKey(k), result);
      }
    }
  } else {
    int maxlen = 0;
    vector<vector<ssize_t> > positions;
    positions.resize(inputs.size());
    int i = 0;
    for (ArrayIter iter(inputs); iter; ++iter, ++i) {
      Array arr = iter.second().toArray();
      int count = arr.size();
      if (count > maxlen) maxlen = count;

      if (count > 0) {
        positions[i].reserve(count);
        for (ssize_t pos = arr->iter_begin(); pos != ArrayData::invalid_index;
             pos = arr->iter_advance(pos)) {
          positions[i].push_back(pos);
        }
      }
    }

    for (int k = 0; k < maxlen; k++) {
      Array params;
      int i = 0;
      for (ArrayIter iter(inputs); iter; ++iter, ++i) {
        Array arr = iter.second().toArray();
        if (k < arr.size()) {
          params.append(arr->getValue(positions[i][k]));
        } else {
          params.append(null);
        }
      }

      Variant result;
      if (map_function) {
        result = map_function(params, data);
      } else {
        result = params;
      }

      ret.append(result);
    }
  }

  return ret;
}

Variant ArrayUtil::Reduce(CArrRef input, PFUNC_REDUCE reduce_function,
                          const void *data,
                          CVarRef initial /* = null_variant */) {
  if (input.empty()) {
    return initial;
  }

  ArrayIter iter(input);
  Variant result;
  if (initial.isNull()) {
    result = iter.second();
  } else {
    result = initial;
  }

  for (++iter; iter; ++iter) {
    result = reduce_function(result, iter.second(), data);
  }
  return result;
}

void ArrayUtil::InitScalarArrays(Array arrs[], int nArrs,
                                 const char *scalarArrayData,
                                 int scalarArrayDataSize) {
  int len = scalarArrayDataSize;
  char *uncompressed = gzdecode(scalarArrayData, len);
  if (uncompressed == NULL) {
    throw Exception("Bad scalarArrayData %p", scalarArrayData);
  }
  String s = String(uncompressed, len, AttachString);
  Variant v(f_unserialize(s));
  ASSERT(v.isArray());
  Array scalarArrays =  v;
  ASSERT(scalarArrays.size() == nArrs);
  for (int i = 0; i < nArrs; i++) {
    arrs[i] = scalarArrays[i];
    arrs[i].setStatic();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
