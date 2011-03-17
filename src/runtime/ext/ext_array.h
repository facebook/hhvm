/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __EXT_ARRAY_H__
#define __EXT_ARRAY_H__

#include <runtime/base/base_includes.h>
#include <runtime/base/array/array_util.h>
#include <runtime/base/zend/zend_collator.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

extern const int64 k_UCOL_DEFAULT;
extern const int64 k_UCOL_PRIMARY;
extern const int64 k_UCOL_SECONDARY;
extern const int64 k_UCOL_TERTIARY;
extern const int64 k_UCOL_DEFAULT_STRENGTH;
extern const int64 k_UCOL_QUATERNARY;
extern const int64 k_UCOL_IDENTICAL;
extern const int64 k_UCOL_OFF;
extern const int64 k_UCOL_ON;
extern const int64 k_UCOL_SHIFTED;
extern const int64 k_UCOL_NON_IGNORABLE;
extern const int64 k_UCOL_LOWER_FIRST;
extern const int64 k_UCOL_UPPER_FIRST;
extern const int64 k_UCOL_FRENCH_COLLATION;
extern const int64 k_UCOL_ALTERNATE_HANDLING;
extern const int64 k_UCOL_CASE_FIRST;
extern const int64 k_UCOL_CASE_LEVEL;
extern const int64 k_UCOL_NORMALIZATION_MODE;
extern const int64 k_UCOL_STRENGTH;
extern const int64 k_UCOL_HIRAGANA_QUATERNARY_MODE;
extern const int64 k_UCOL_NUMERIC_COLLATION;

inline Variant f_array_change_key_case(CVarRef input, bool upper = false) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  CArrRef arr_input = input.toArrNR();
  return ArrayUtil::ChangeKeyCase(arr_input, !upper);
}
inline Variant f_array_chunk(CVarRef input, int size,
                             bool preserve_keys = false) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_input = input.toArrNR();
  return ArrayUtil::Chunk(arr_input, size, preserve_keys);
}
inline Variant f_array_combine(CVarRef keys, CVarRef values) {
  if (!keys.isArray() || !values.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_keys = keys.toArrNR();
  CArrRef arr_values = values.toArrNR();
  return ArrayUtil::Combine(arr_keys, arr_values);
}
inline Variant f_array_count_values(CVarRef input) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_input = input.toArrNR();
  return ArrayUtil::CountValues(arr_input);
}
inline Variant f_array_fill_keys(CVarRef keys, CVarRef value) {
  if (!keys.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_keys = keys.toArrNR();
  return ArrayUtil::CreateArray(arr_keys, value);
}
inline Variant f_array_fill(int start_index, int num, CVarRef value) {
  return ArrayUtil::CreateArray(start_index, num, value);
}

Variant f_array_filter(CVarRef input, CVarRef callback = null_variant);

inline Variant f_array_flip(CVarRef trans) {
  if (!trans.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  CArrRef arr_trans = trans.toArrNR();
  return ArrayUtil::Flip(arr_trans);
}

bool f_array_key_exists(CVarRef key, CVarRef search);
inline bool f_key_exists(CVarRef key, CVarRef search) {
  return f_array_key_exists(key, search);
}

inline Variant f_array_keys(CVarRef input, CVarRef search_value = null_variant,
                            bool strict = false) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_input = input.toArrNR();
  return arr_input.keys(search_value, strict);
}

Variant f_array_map(int _argc, CVarRef callback, CVarRef arr1, CArrRef _argv = null_array);

Variant f_array_merge_recursive(int _argc, CVarRef array1, CArrRef _argv = null_array);

Variant f_array_merge(int _argc, CVarRef array1, CArrRef _argv = null_array);

Variant f_array_replace_recursive(int _argc, CVarRef array1, CArrRef _argv = null_array);

Variant f_array_replace(int _argc, CVarRef array1, CArrRef _argv = null_array);

inline Variant f_array_pad(CVarRef input, int pad_size, CVarRef pad_value) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_input = input.toArrNR();
  if (pad_size > 0) {
    return ArrayUtil::Pad(arr_input, pad_value, pad_size, true);
  }
  return ArrayUtil::Pad(arr_input, pad_value, -pad_size, false);
}

inline Variant f_array_pop(Variant array) {
  return array.pop();
}
inline Variant f_array_product(CVarRef array) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr = array.toArrNR();
  if (arr.empty()) {
    return 0; // to be consistent with PHP
  }
  int64 i;
  double d;
  if (ArrayUtil::Product(arr, &i, &d) == KindOfInt64) {
    return i;
  } else {
    return d;
  }
}

Variant f_array_push(int _argc, Variant array, CVarRef var, CArrRef _argv = null_array);

inline Variant f_array_rand(CVarRef input, int num_req = 1) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_input = input.toArrNR();
  return ArrayUtil::RandomKeys(arr_input, num_req);
}

Variant f_array_reduce(CVarRef input, CVarRef callback,
                       CVarRef initial = null_variant);

inline Variant f_array_reverse(CVarRef array, bool preserve_keys = false) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr = array.toArrNR();
  return ArrayUtil::Reverse(arr, preserve_keys);
}
inline Variant f_array_search(CVarRef needle, CVarRef haystack,
                              bool strict = false) {
  if (!haystack.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  CArrRef arr_haystack = haystack.toArrNR();
  return arr_haystack.key(needle, strict);
}
inline Variant f_array_shift(Variant array) {
  return array.dequeue();
}
inline Variant f_array_slice(CVarRef array, int offset,
                             CVarRef length = null_variant,
                             bool preserve_keys = false) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  int64 len = length.isNull() ? 0x7FFFFFFF : length.toInt64();
  CArrRef arr = array.toArrNR();
  return ArrayUtil::Slice(arr, offset, len, preserve_keys);
}
inline Variant f_array_splice(Variant input, int offset,
                            CVarRef length = null_variant,
                            CVarRef replacement = null_variant) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  Array ret(Array::Create());
  int64 len = length.isNull() ? 0x7FFFFFFF : length.toInt64();
  CArrRef arr_input = input.toArrNR();
  input = ArrayUtil::Splice(arr_input, offset, len, replacement, &ret);
  return ret;
}
inline Variant f_array_sum(CVarRef array) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  int64 i;
  double d;
  CArrRef arr = array.toArrNR();
  if (ArrayUtil::Sum(arr, &i, &d) == KindOfInt64) {
    return i;
  } else {
    return d;
  }
}

Variant f_array_unique(CVarRef array, int sort_flags = 2);

int f_array_unshift(int _argc, Variant array, CVarRef var, CArrRef _argv = null_array);

inline Variant f_array_values(CVarRef input) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_input = input.toArrNR();
  return arr_input.values();
}

bool f_array_walk_recursive(Variant input, CVarRef funcname,
                            CVarRef userdata = null_variant);

bool f_array_walk(Variant input, CVarRef funcname,
                  CVarRef userdata = null_variant);

/**
 * LVariableTable parameter is added by HPHP.
 */
Array f_compact(int _argc, CVarRef varname, CArrRef _argv = null_array);
Array compact(RVariableTable *variables, int _argc, CVarRef varname,
              CArrRef _argv = null_array);
Array compact(LVariableTable *variables, int _argc, CVarRef varname,
              CArrRef _argv = null_array);

inline bool f_shuffle(Variant array) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  array = ArrayUtil::Shuffle(array);
  return true;
}

int f_count(CVarRef var, bool recursive = false);

inline int f_sizeof(CVarRef var, bool recursive = false) {
  return f_count(var, recursive);
}
inline Variant f_each(Variant array) {
  array.array_iter_dirty_check();
  return array.array_iter_each();
}
inline Variant f_current(CVarRef array) {
  array.array_iter_dirty_check();
  return array.array_iter_current();
}
inline Variant f_hphp_current_ref(Variant array) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  return ref(array.array_iter_current_ref());
}
inline Variant f_next(Variant array) {
  array.array_iter_dirty_check();
  return array.array_iter_next();
}
inline Variant f_pos(Variant array) {
  array.array_iter_dirty_check();
  return array.array_iter_current();
}
inline Variant f_prev(Variant array) {
  array.array_iter_dirty_check();
  return array.array_iter_prev();
}
inline Variant f_reset(Variant array) {
  array.array_iter_dirty_reset();
  return array.array_iter_reset();
}
inline Variant f_end(Variant array) {
  array.array_iter_dirty_reset();
  return array.array_iter_end();
}
inline Variant f_key(Variant array) {
  array.array_iter_dirty_check();
  return array.array_iter_key();
}

Variant f_hphp_get_iterator(Variant iterable, bool isMutable);

inline bool f_in_array(CVarRef needle, CVarRef haystack, bool strict = false) {
  if (!haystack.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  CArrRef arr_haystack = haystack.toArrNR();
  return arr_haystack.valueExists(needle, strict);
}

Variant f_range(CVarRef low, CVarRef high, CVarRef step = 1);

Variant f_array_diff(int _argc, CVarRef array1, CVarRef array2, CArrRef _argv = null_array);
Variant f_array_udiff(int _argc, CVarRef array1, CVarRef array2,
                      CVarRef data_compare_func, CArrRef _argv = null_array);
Variant f_array_diff_assoc(int _argc, CVarRef array1, CVarRef array2, CArrRef _argv = null_array);
Variant f_array_diff_uassoc(int _argc, CVarRef array1, CVarRef array2,
                            CVarRef key_compare_func, CArrRef _argv = null_array);
Variant f_array_udiff_assoc(int _argc, CVarRef array1, CVarRef array2,
                            CVarRef data_compare_func, CArrRef _argv = null_array);
Variant f_array_udiff_uassoc(int _argc, CVarRef array1, CVarRef array2,
                             CVarRef data_compare_func,
                             CVarRef key_compare_func, CArrRef _argv = null_array);
Variant f_array_diff_key(int _argc, CVarRef array1, CVarRef array2, CArrRef _argv = null_array);
Variant f_array_diff_ukey(int _argc, CVarRef array1, CVarRef array2,
                          CVarRef key_compare_func, CArrRef _argv = null_array);

Variant f_array_intersect(int _argc, CVarRef array1, CVarRef array2, CArrRef _argv = null_array);
Variant f_array_uintersect(int _argc, CVarRef array1, CVarRef array2,
                           CVarRef data_compare_func, CArrRef _argv = null_array);
Variant f_array_intersect_assoc(int _argc, CVarRef array1, CVarRef array2, CArrRef _argv = null_array);
Variant f_array_intersect_uassoc(int _argc, CVarRef array1, CVarRef array2,
                                 CVarRef key_compare_func, CArrRef _argv = null_array);
Variant f_array_uintersect_assoc(int _argc, CVarRef array1, CVarRef array2,
                                 CVarRef data_compare_func, CArrRef _argv = null_array);
Variant f_array_uintersect_uassoc(int _argc, CVarRef array1, CVarRef array2,
                                  CVarRef data_compare_func,
                                  CVarRef key_compare_func, CArrRef _argv = null_array);
Variant f_array_intersect_key(int _argc, CVarRef array1, CVarRef array2, CArrRef _argv = null_array);
Variant f_array_intersect_ukey(int _argc, CVarRef array1, CVarRef array2,
                               CVarRef key_compare_func, CArrRef _argv = null_array);

bool f_sort(Variant array, int sort_flags = 0, bool use_collator = false);
bool f_rsort(Variant array, int sort_flags = 0, bool use_collator = false);
bool f_asort(Variant array, int sort_flags = 0, bool use_collator = false);
bool f_arsort(Variant array, int sort_flags = 0, bool use_collator = false);
bool f_ksort(Variant array, int sort_flags = 0);
bool f_krsort(Variant array, int sort_flags = 0);
bool f_usort(Variant array, CVarRef cmp_function);
bool f_uasort(Variant array, CVarRef cmp_function);
bool f_uksort(Variant array, CVarRef cmp_function);
Variant f_natsort(Variant array);
Variant f_natcasesort(Variant array);

bool f_array_multisort(int _argc, Variant ar1, CArrRef _argv = null_array);

String f_i18n_loc_get_default();
bool f_i18n_loc_set_default(CStrRef locale);
bool f_i18n_loc_set_attribute(int64 attr, int64 val);
bool f_i18n_loc_set_strength(int64 strength);
Variant f_i18n_loc_get_error_code();

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_ARRAY_H__
