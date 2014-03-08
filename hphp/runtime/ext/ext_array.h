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

#ifndef incl_HPHP_EXT_ARRAY_H_
#define incl_HPHP_EXT_ARRAY_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/base/zend-collator.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_UCOL_DEFAULT;
extern const int64_t k_UCOL_PRIMARY;
extern const int64_t k_UCOL_SECONDARY;
extern const int64_t k_UCOL_TERTIARY;
extern const int64_t k_UCOL_DEFAULT_STRENGTH;
extern const int64_t k_UCOL_QUATERNARY;
extern const int64_t k_UCOL_IDENTICAL;
extern const int64_t k_UCOL_OFF;
extern const int64_t k_UCOL_ON;
extern const int64_t k_UCOL_SHIFTED;
extern const int64_t k_UCOL_NON_IGNORABLE;
extern const int64_t k_UCOL_LOWER_FIRST;
extern const int64_t k_UCOL_UPPER_FIRST;
extern const int64_t k_UCOL_FRENCH_COLLATION;
extern const int64_t k_UCOL_ALTERNATE_HANDLING;
extern const int64_t k_UCOL_CASE_FIRST;
extern const int64_t k_UCOL_CASE_LEVEL;
extern const int64_t k_UCOL_NORMALIZATION_MODE;
extern const int64_t k_UCOL_STRENGTH;
extern const int64_t k_UCOL_HIRAGANA_QUATERNARY_MODE;
extern const int64_t k_UCOL_NUMERIC_COLLATION;

Variant f_array_change_key_case(const Variant& input, int64_t case_ = 0);
Variant f_array_chunk(const Variant& input, int size,
                      bool preserve_keys = false);
Variant f_array_column(const Variant& arr, const Variant& val_key,
                       const Variant& idx_key = null_variant);
Variant f_array_combine(const Variant& keys, const Variant& values);
Variant f_array_count_values(const Variant& input);
Variant f_array_fill_keys(const Variant& keys, const Variant& value);
Variant f_array_fill(int start_index, int num, const Variant& value);
Variant f_array_flip(const Variant& trans);
bool f_array_key_exists(const Variant& key, const Variant& search);
bool f_key_exists(const Variant& key, const Variant& search);

Variant f_array_keys(const Variant& input, const Variant& search_value = null_variant,
                     bool strict = false);
Variant f_array_map(int _argc, const Variant& callback, const Variant& arr1,
                    const Array& _argv = null_array);

Variant f_array_merge_recursive(int _argc, const Variant& array1, const Array& _argv = null_array);

Variant f_array_merge(int _argc, const Variant& array1, const Array& _argv = null_array);

Variant f_array_replace_recursive(int _argc, const Variant& array1, const Array& _argv = null_array);

Variant f_array_replace(int _argc, const Variant& array1, const Array& _argv = null_array);

Variant f_array_pad(const Variant& input, int pad_size, const Variant& pad_value);
Variant f_array_pop(VRefParam array);
Variant f_array_product(const Variant& array);
Variant f_array_push(int _argc, VRefParam container, const Variant& var,
                     const Array& _argv = null_array);

Variant f_array_rand(const Variant& input, int num_req = 1);
Variant f_array_reduce(const Variant& input, const Variant& callback,
                       const Variant& initial = null_variant);

Variant f_array_reverse(const Variant& array, bool preserve_keys = false);
Variant f_array_search(const Variant& needle, const Variant& haystack,
                              bool strict = false);
Variant f_array_shift(VRefParam array);
Variant f_array_slice(const Variant& array, int offset,
                             const Variant& length = null_variant,
                             bool preserve_keys = false);
Variant f_array_splice(VRefParam input, int offset,
                       const Variant& length = null_variant,
                       const Variant& replacement = null_variant);
Variant f_array_sum(const Variant& array);
Variant f_array_unique(const Variant& array, int sort_flags = 2);

Variant f_array_unshift(int _argc, VRefParam array, const Variant& var, const Array& _argv = null_array);

Variant f_array_values(const Variant& input);
bool f_array_walk_recursive(VRefParam input, const Variant& funcname,
                            const Variant& userdata = null_variant);

bool f_array_walk(VRefParam input, const Variant& funcname,
                  const Variant& userdata = null_variant);

Array f_compact(int _argc, const Variant& varname, const Array& _argv = null_array);

bool f_shuffle(VRefParam array);
int64_t f_count(const Variant& var, int64_t mode = 0);

int64_t f_sizeof(const Variant& var, int64_t mode = 0);
Variant f_each(VRefParam array);
Variant f_current(VRefParam array);
Variant f_pos(VRefParam array);
Variant f_key(VRefParam array);
Variant f_next(VRefParam array);
Variant f_prev(VRefParam array);
Variant f_reset(VRefParam array);
Variant f_end(VRefParam array);

bool f_in_array(const Variant& needle, const Variant& haystack, bool strict = false);
Variant f_range(const Variant& low, const Variant& high, const Variant& step = 1);

Variant f_array_diff(int _argc, const Variant& container1, const Variant& container2, const Array& _argv = null_array);
Variant f_array_diff_key(int _argc, const Variant& container1, const Variant& container2, const Array& _argv = null_array);
Variant f_array_udiff(int _argc, const Variant& array1, const Variant& array2,
                      const Variant& data_compare_func, const Array& _argv = null_array);
Variant f_array_diff_assoc(int _argc, const Variant& array1, const Variant& array2, const Array& _argv = null_array);
Variant f_array_diff_uassoc(int _argc, const Variant& array1, const Variant& array2,
                            const Variant& key_compare_func, const Array& _argv = null_array);
Variant f_array_udiff_assoc(int _argc, const Variant& array1, const Variant& array2,
                            const Variant& data_compare_func, const Array& _argv = null_array);
Variant f_array_udiff_uassoc(int _argc, const Variant& array1, const Variant& array2,
                             const Variant& data_compare_func,
                             const Variant& key_compare_func, const Array& _argv = null_array);
Variant f_array_diff_ukey(int _argc, const Variant& array1, const Variant& array2,
                          const Variant& key_compare_func, const Array& _argv = null_array);

Variant f_array_intersect(int _argc, const Variant& container1, const Variant& container2, const Array& _argv = null_array);
Variant f_array_intersect_key(int _argc, const Variant& container1, const Variant& container2, const Array& _argv = null_array);
Variant f_array_uintersect(int _argc, const Variant& array1, const Variant& array2,
                           const Variant& data_compare_func, const Array& _argv = null_array);
Variant f_array_intersect_assoc(int _argc, const Variant& array1, const Variant& array2, const Array& _argv = null_array);
Variant f_array_intersect_uassoc(int _argc, const Variant& array1, const Variant& array2,
                                 const Variant& key_compare_func, const Array& _argv = null_array);
Variant f_array_uintersect_assoc(int _argc, const Variant& array1, const Variant& array2,
                                 const Variant& data_compare_func, const Array& _argv = null_array);
Variant f_array_uintersect_uassoc(int _argc, const Variant& array1, const Variant& array2,
                                  const Variant& data_compare_func,
                                  const Variant& key_compare_func, const Array& _argv = null_array);
Variant f_array_intersect_ukey(int _argc, const Variant& array1, const Variant& array2,
                               const Variant& key_compare_func, const Array& _argv = null_array);

bool f_sort(VRefParam array, int sort_flags = 0, bool use_collator = false);
bool f_rsort(VRefParam array, int sort_flags = 0, bool use_collator = false);
bool f_asort(VRefParam array, int sort_flags = 0, bool use_collator = false);
bool f_arsort(VRefParam array, int sort_flags = 0, bool use_collator = false);
bool f_ksort(VRefParam array, int sort_flags = 0);
bool f_krsort(VRefParam array, int sort_flags = 0);
bool f_usort(VRefParam array, const Variant& cmp_function);
bool f_uasort(VRefParam array, const Variant& cmp_function);
bool f_uksort(VRefParam array, const Variant& cmp_function);
Variant f_natsort(VRefParam array);
Variant f_natcasesort(VRefParam array);

bool f_array_multisort(int _argc, VRefParam ar1, const Array& _argv = null_array);

String f_i18n_loc_get_default();
bool f_i18n_loc_set_default(const String& locale);
bool f_i18n_loc_set_attribute(int64_t attr, int64_t val);
bool f_i18n_loc_set_strength(int64_t strength);
Variant f_i18n_loc_get_error_code();

Variant f_hphp_array_idx(const Variant& search, const Variant& key, const Variant& def);

Array ArrayObject_toArray(const ObjectData* obj);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ARRAY_H_
