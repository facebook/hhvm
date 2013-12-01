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

Variant f_array_change_key_case(CVarRef input, bool upper = false);
Variant f_array_chunk(CVarRef input, int size,
                      bool preserve_keys = false);
Variant f_array_column(CVarRef arr, CVarRef val_key,
                       CVarRef idx_key = null_variant);
Variant f_array_combine(CVarRef keys, CVarRef values);
Variant f_array_count_values(CVarRef input);
Variant f_array_fill_keys(CVarRef keys, CVarRef value);
Variant f_array_fill(int start_index, int num, CVarRef value);
Variant f_array_flip(CVarRef trans);
bool f_array_key_exists(CVarRef key, CVarRef search);
bool f_key_exists(CVarRef key, CVarRef search);

Variant f_array_keys(CVarRef input, CVarRef search_value = null_variant,
                     bool strict = false);
Variant f_array_map(int _argc, CVarRef callback, CVarRef arr1, CArrRef _argv = null_array);

Variant f_array_merge_recursive(int _argc, CVarRef array1, CArrRef _argv = null_array);

Variant f_array_merge(int _argc, CVarRef array1, CArrRef _argv = null_array);

Variant f_array_replace_recursive(int _argc, CVarRef array1, CArrRef _argv = null_array);

Variant f_array_replace(int _argc, CVarRef array1, CArrRef _argv = null_array);

Variant f_array_pad(CVarRef input, int pad_size, CVarRef pad_value);
Variant f_array_pop(VRefParam array);
Variant f_array_product(CVarRef array);
Variant f_array_push(int _argc, VRefParam container, CVarRef var,
                     CArrRef _argv = null_array);

Variant f_array_rand(CVarRef input, int num_req = 1);
Variant f_array_reduce(CVarRef input, CVarRef callback,
                       CVarRef initial = null_variant);

Variant f_array_reverse(CVarRef array, bool preserve_keys = false);
Variant f_array_search(CVarRef needle, CVarRef haystack,
                              bool strict = false);
Variant f_array_shift(VRefParam array);
Variant f_array_slice(CVarRef array, int offset,
                             CVarRef length = null_variant,
                             bool preserve_keys = false);
Variant f_array_splice(VRefParam input, int offset,
                       CVarRef length = null_variant,
                       CVarRef replacement = null_variant);
Variant f_array_sum(CVarRef array);
Variant f_array_unique(CVarRef array, int sort_flags = 2);

int64_t f_array_unshift(int _argc, VRefParam array, CVarRef var, CArrRef _argv = null_array);

Variant f_array_values(CVarRef input);
bool f_array_walk_recursive(VRefParam input, CVarRef funcname,
                            CVarRef userdata = null_variant);

bool f_array_walk(VRefParam input, CVarRef funcname,
                  CVarRef userdata = null_variant);

Array f_compact(int _argc, CVarRef varname, CArrRef _argv = null_array);

bool f_shuffle(VRefParam array);
int64_t f_count(CVarRef var, bool recursive = false);

int64_t f_sizeof(CVarRef var, bool recursive = false);
Variant f_each(VRefParam array);
Variant f_current(VRefParam array);
Variant f_pos(VRefParam array);
Variant f_key(VRefParam array);
Variant f_next(VRefParam array);
Variant f_prev(VRefParam array);
Variant f_reset(VRefParam array);
Variant f_end(VRefParam array);

bool f_in_array(CVarRef needle, CVarRef haystack, bool strict = false);
Variant f_range(CVarRef low, CVarRef high, CVarRef step = 1);

Variant f_array_diff(int _argc, CVarRef container1, CVarRef container2, CArrRef _argv = null_array);
Variant f_array_diff_key(int _argc, CVarRef container1, CVarRef container2, CArrRef _argv = null_array);
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
Variant f_array_diff_ukey(int _argc, CVarRef array1, CVarRef array2,
                          CVarRef key_compare_func, CArrRef _argv = null_array);

Variant f_array_intersect(int _argc, CVarRef container1, CVarRef container2, CArrRef _argv = null_array);
Variant f_array_intersect_key(int _argc, CVarRef container1, CVarRef container2, CArrRef _argv = null_array);
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
Variant f_array_intersect_ukey(int _argc, CVarRef array1, CVarRef array2,
                               CVarRef key_compare_func, CArrRef _argv = null_array);

bool f_sort(VRefParam array, int sort_flags = 0, bool use_collator = false);
bool f_rsort(VRefParam array, int sort_flags = 0, bool use_collator = false);
bool f_asort(VRefParam array, int sort_flags = 0, bool use_collator = false);
bool f_arsort(VRefParam array, int sort_flags = 0, bool use_collator = false);
bool f_ksort(VRefParam array, int sort_flags = 0);
bool f_krsort(VRefParam array, int sort_flags = 0);
bool f_usort(VRefParam array, CVarRef cmp_function);
bool f_uasort(VRefParam array, CVarRef cmp_function);
bool f_uksort(VRefParam array, CVarRef cmp_function);
Variant f_natsort(VRefParam array);
Variant f_natcasesort(VRefParam array);

bool f_array_multisort(int _argc, VRefParam ar1, CArrRef _argv = null_array);

String f_i18n_loc_get_default();
bool f_i18n_loc_set_default(const String& locale);
bool f_i18n_loc_set_attribute(int64_t attr, int64_t val);
bool f_i18n_loc_set_strength(int64_t strength);
Variant f_i18n_loc_get_error_code();

Variant f_hphp_array_idx(CVarRef search, CVarRef key, CVarRef def);

Array ArrayObject_toArray(const ObjectData* obj);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ARRAY_H_
