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

#include "hphp/runtime/ext/extension.h"
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

Variant HHVM_FUNCTION(array_change_key_case,
                      const Variant& input,
                      int64_t case_ = 0);
Variant HHVM_FUNCTION(array_chunk,
                      const Variant& input,
                      int size,
                      bool preserve_keys = false);
Variant HHVM_FUNCTION(array_column,
                      const Variant& arr,
                      const Variant& val_key,
                      const Variant& idx_key = null_variant);
Variant HHVM_FUNCTION(array_combine,
                      const Variant& keys,
                      const Variant& values);
Variant HHVM_FUNCTION(array_count_values,
                      const Variant& input);
Variant HHVM_FUNCTION(array_fill_keys,
                      const Variant& keys,
                      const Variant& value);
Variant HHVM_FUNCTION(array_fill,
                      int start_index,
                      int num,
                      const Variant& value);
Variant HHVM_FUNCTION(array_flip,
                      const Variant& trans);
bool HHVM_FUNCTION(array_key_exists,
                   const Variant& key,
                   const Variant& search);
bool HHVM_FUNCTION(key_exists,
                   const Variant& key,
                   const Variant& search);
Variant array_keys_helper(const Variant& input,
                          const Variant& search_value = uninit_null(),
                          bool strict = false);
TypedValue* HHVM_FN(array_keys)(ActRec* ar);
Variant HHVM_FUNCTION(array_map, const Variant& callback,
                                 const Variant& arr1,
                                 const Array& _argv = null_array);
Variant HHVM_FUNCTION(array_merge_recursive,
                      int64_t numArgs,
                      const Variant& array1,
                      const Variant& array2 = null_variant,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_merge,
                      int64_t numArgs,
                      const Variant& array1,
                      const Variant& array2 = null_variant,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_replace_recursive,
                      const Variant& array1,
                      const Variant& array2 = null_variant,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_replace,
                      const Variant& array1,
                      const Variant& array2 = null_variant,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_pad,
                      const Variant& input,
                      int pad_size,
                      const Variant& pad_value);
Variant HHVM_FUNCTION(array_pop,
                      VRefParam array);
Variant HHVM_FUNCTION(array_product,
                      const Variant& array);
Variant HHVM_FUNCTION(array_push,
                      VRefParam container,
                      const Variant& var,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_rand,
                      const Variant& input,
                      int num_req = 1);
Variant HHVM_FUNCTION(array_reverse,
                      const Variant& array,
                      bool preserve_keys = false);
Variant HHVM_FUNCTION(array_search,
                      const Variant& needle,
                      const Variant& haystack,
                      bool strict = false);
Variant HHVM_FUNCTION(array_shift,
                      VRefParam array);
Variant HHVM_FUNCTION(array_slice,
                      const Variant& array,
                      int64_t offset,
                      const Variant& length = null_variant,
                      bool preserve_keys = false);
Variant HHVM_FUNCTION(array_splice,
                      VRefParam input,
                      int offset,
                      const Variant& length = null_variant,
                      const Variant& replacement = null_variant);
Variant HHVM_FUNCTION(array_sum,
                      const Variant& array);
Variant HHVM_FUNCTION(array_unique,
                      const Variant& array,
                      int sort_flags = 2);
Variant HHVM_FUNCTION(array_unshift,
                      VRefParam array,
                      const Variant& var,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_values,
                      const Variant& input);
bool HHVM_FUNCTION(array_walk_recursive,
                   VRefParam input,
                   const Variant& funcname,
                   const Variant& userdata = null_variant);
bool HHVM_FUNCTION(array_walk,
                   VRefParam input,
                   const Variant& funcname,
                    const Variant& userdata = null_variant);
Array HHVM_FUNCTION(compact,
                    const Variant& varname,
                    const Array& args = null_array);
// __SystemLib\\compact_sl
Array HHVM_FUNCTION(__SystemLib_compact_sl,
                    const Variant& varname,
                    const Array& args = null_array);
bool HHVM_FUNCTION(shuffle,
                   VRefParam array);
int64_t HHVM_FUNCTION(count,
                      const Variant& var,
                      int64_t mode = 0);
int64_t HHVM_FUNCTION(sizeof,
                      const Variant& var,
                      int64_t mode = 0);
Variant HHVM_FUNCTION(each,
                      VRefParam array);
Variant HHVM_FUNCTION(current,
                      VRefParam array);
Variant HHVM_FUNCTION(pos,
                      VRefParam array);
Variant HHVM_FUNCTION(key,
                      VRefParam array);
Variant HHVM_FUNCTION(next,
                      VRefParam array);
Variant HHVM_FUNCTION(prev,
                      VRefParam array);
Variant HHVM_FUNCTION(reset,
                      VRefParam array);
Variant HHVM_FUNCTION(end,
                      VRefParam array);
bool HHVM_FUNCTION(in_array,
                   const Variant& needle,
                   const Variant& haystack,
                   bool strict = false);
Variant HHVM_FUNCTION(range,
                      const Variant& low,
                      const Variant& high,
                      const Variant& step = 1);
Variant HHVM_FUNCTION(array_diff,
                      const Variant& container1,
                      const Variant& container2,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_diff_key,
                      const Variant& container1,
                      const Variant& container2,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_udiff,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_diff_assoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_diff_uassoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& key_compare_func,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_udiff_assoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_udiff_uassoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Variant& key_compare_func,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_diff_ukey,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& key_compare_func,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_intersect,
                      const Variant& container1,
                      const Variant& container2,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_intersect_key,
                      const Variant& container1,
                      const Variant& container2,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_uintersect,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_intersect_assoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_intersect_uassoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& key_compare_func,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_uintersect_assoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_uintersect_uassoc,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& data_compare_func,
                      const Variant& key_compare_func,
                      const Array& args = null_array);
Variant HHVM_FUNCTION(array_intersect_ukey,
                      const Variant& array1,
                      const Variant& array2,
                      const Variant& key_compare_func,
                      const Array& args = null_array);
bool HHVM_FUNCTION(sort,
                   VRefParam array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(rsort,
                   VRefParam array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(asort,
                   VRefParam array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(arsort,
                   VRefParam array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(ksort,
                   VRefParam array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(krsort,
                   VRefParam array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(usort,
                   VRefParam array,
                   const Variant& cmp_function);
bool HHVM_FUNCTION(uasort,
                   VRefParam array,
                   const Variant& cmp_function);
bool HHVM_FUNCTION(uksort,
                   VRefParam array,
                   const Variant& cmp_function);
Variant HHVM_FUNCTION(natsort,
                      VRefParam array);
Variant HHVM_FUNCTION(natcasesort,
                      VRefParam array);
String HHVM_FUNCTION(i18n_loc_get_default);
bool HHVM_FUNCTION(i18n_loc_set_default,
                   const String& locale);
bool HHVM_FUNCTION(i18n_loc_set_attribute,
                   int64_t attr,
                   int64_t val);
bool HHVM_FUNCTION(i18n_loc_set_strength,
                   int64_t strength);
Variant HHVM_FUNCTION(i18n_loc_get_error_code);
Variant HHVM_FUNCTION(hphp_array_idx,
                      const Variant& search,
                      const Variant& key,
                      const Variant& def);
TypedValue* HHVM_FN(array_multisort)(ActRec* ar);

///////////////////////////////////////////////////////////////////////////////

inline int64_t countHelper(TypedValue tv) {
  return HHVM_FN(count)(tvAsVariant(&tv));
}

///////////////////////////////////////////////////////////////////////////////

#define getCheckedArrayRet(input, fail)                           \
  auto const cell_##input = static_cast<const Variant&>(input).asCell(); \
  if (UNLIKELY(cell_##input->m_type != KindOfArray)) {            \
    throw_expected_array_exception();                             \
    return fail;                                                  \
  }                                                               \
  ArrNR arrNR_##input(cell_##input->m_data.parr);                 \
  const Array& arr_##input = arrNR_##input.asArray();

#define getCheckedArrayColumnRet(input, fail)                     \
  auto const cell_##input = static_cast<const Variant&>(input).asCell(); \
  if (UNLIKELY(cell_##input->m_type != KindOfArray)) {            \
    if (cell_##input->m_type == KindOfString ||                   \
        cell_##input->m_type == KindOfStaticString) {             \
      throw_bad_type_exception("array_column() expects parameter" \
                               " 1 to be array, string given");   \
    } else if (cell_##input->m_type == KindOfInt64) {             \
      throw_bad_type_exception("array_column() expects parameter" \
                               " 1 to be array, integer given");  \
    } else {                                                      \
      throw_expected_array_exception();                           \
    }                                                             \
    return fail;                                                  \
  }                                                               \
  ArrNR arrNR_##input(cell_##input->m_data.parr);                 \
  Array arr_##input = arrNR_##input.asArray();


#define getCheckedArray(input) getCheckedArrayRet(input, uninit_null())

}

#endif // incl_HPHP_EXT_ARRAY_H_
