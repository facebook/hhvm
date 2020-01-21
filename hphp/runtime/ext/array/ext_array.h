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

#ifndef incl_HPHP_EXT_ARRAY_H_
#define incl_HPHP_EXT_ARRAY_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/base/zend-collator.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

TypedValue HHVM_FUNCTION(array_chunk,
                         const Variant& input,
                         int size,
                         bool preserve_keys = false);
TypedValue HHVM_FUNCTION(array_combine,
                         const Variant& keys,
                         const Variant& values);
TypedValue HHVM_FUNCTION(array_fill_keys,
                         const Variant& keys,
                         const Variant& value);
TypedValue HHVM_FUNCTION(array_fill,
                         int start_index,
                         int num,
                         const Variant& value);
TypedValue HHVM_FUNCTION(array_flip,
                         const Variant& trans);
bool HHVM_FUNCTION(array_key_exists,
                   const Variant& key,
                   const Variant& search);
bool HHVM_FUNCTION(key_exists,
                   const Variant& key,
                   const Variant& search);
TypedValue HHVM_FUNCTION(array_keys,
                         TypedValue input);
TypedValue HHVM_FUNCTION(array_map,
                         const Variant& callback,
                         const Variant& arr1,
                         const Array& _argv = null_array);
TypedValue HHVM_FUNCTION(array_replace_recursive,
                         const Variant& array1,
                         const Variant& array2 = uninit_variant,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_replace,
                         const Variant& array1,
                         const Variant& array2 = uninit_variant,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_pad,
                         const Variant& input,
                         int pad_size,
                         const Variant& pad_value);
TypedValue HHVM_FUNCTION(array_product,
                         const Variant& array);
TypedValue HHVM_FUNCTION(array_push,
                         Variant& container,
                         const Variant& var,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_rand,
                         const Variant& input,
                         int num_req = 1);
TypedValue HHVM_FUNCTION(array_search,
                         const Variant& needle,
                         const Variant& haystack,
                         bool strict = false);
TypedValue HHVM_FUNCTION(array_shift,
                         Variant& array);
TypedValue HHVM_FUNCTION(array_splice,
                         Variant& input,
                         int offset,
                         const Variant& length = uninit_variant,
                         const Variant& replacement = uninit_variant);
TypedValue HHVM_FUNCTION(array_sum,
                         const Variant& array);
TypedValue HHVM_FUNCTION(array_unique,
                         const Variant& array,
                         int sort_flags = 2);
TypedValue HHVM_FUNCTION(array_unshift,
                         Variant& array,
                         const Variant& var,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_values,
                         const Variant& input);
bool HHVM_FUNCTION(shuffle,
                   Variant& array);
int64_t HHVM_FUNCTION(count,
                      const Variant& var,
                      int64_t mode = 0);
int64_t HHVM_FUNCTION(sizeof,
                      const Variant& var);
Variant HHVM_FUNCTION(each,
                      Variant& array);
Variant HHVM_FUNCTION(current,
                      const Variant& array);
Variant HHVM_FUNCTION(key,
                      const Variant& array);
Variant HHVM_FUNCTION(next,
                      Variant& array);
Variant HHVM_FUNCTION(prev,
                      Variant& array);
Variant HHVM_FUNCTION(reset,
                      Variant& array);
Variant HHVM_FUNCTION(end,
                      Variant& array);
bool HHVM_FUNCTION(in_array,
                   const Variant& needle,
                   const Variant& haystack,
                   bool strict = false);
TypedValue HHVM_FUNCTION(range,
                         const Variant& low,
                         const Variant& high,
                         const Variant& step = 1);
TypedValue HHVM_FUNCTION(array_diff,
                         const Variant& container1,
                         const Variant& container2,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_diff_key,
                         const Variant& container1,
                         const Variant& container2,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_udiff,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_diff_assoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_diff_uassoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& key_compare_func,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_udiff_assoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_udiff_uassoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Variant& key_compare_func,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_diff_ukey,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& key_compare_func,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_intersect,
                         const Variant& container1,
                         const Variant& container2,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_intersect_key,
                         const Variant& container1,
                         const Variant& container2,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_uintersect,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_intersect_assoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_intersect_uassoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& key_compare_func,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_uintersect_assoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_uintersect_uassoc,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& data_compare_func,
                         const Variant& key_compare_func,
                         const Array& args = null_array);
TypedValue HHVM_FUNCTION(array_intersect_ukey,
                         const Variant& array1,
                         const Variant& array2,
                         const Variant& key_compare_func,
                         const Array& args = null_array);
bool HHVM_FUNCTION(sort,
                   Variant& array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(rsort,
                   Variant& array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(asort,
                   Variant& array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(arsort,
                   Variant& array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(ksort,
                   Variant& array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(krsort,
                   Variant& array,
                   int sort_flags = 0);
bool HHVM_FUNCTION(usort,
                   Variant& array,
                   const Variant& cmp_function);
bool HHVM_FUNCTION(uasort,
                   Variant& array,
                   const Variant& cmp_function);
bool HHVM_FUNCTION(uksort,
                   Variant& array,
                   const Variant& cmp_function);
bool HHVM_FUNCTION(natsort,
                   Variant& array);
bool HHVM_FUNCTION(natcasesort,
                   Variant& array);
String HHVM_FUNCTION(i18n_loc_get_default);
bool HHVM_FUNCTION(i18n_loc_set_default,
                   const String& locale);
bool HHVM_FUNCTION(i18n_loc_set_attribute,
                   int64_t attr,
                   int64_t val);
bool HHVM_FUNCTION(i18n_loc_set_strength,
                   int64_t strength);
Variant HHVM_FUNCTION(i18n_loc_get_error_code);
TypedValue HHVM_FUNCTION(hphp_array_idx,
                         const Variant& search,
                         const Variant& key,
                         const Variant& def);

///////////////////////////////////////////////////////////////////////////////

inline int64_t countHelper(TypedValue tv) {
  return HHVM_FN(count)(tvAsVariant(&tv));
}

///////////////////////////////////////////////////////////////////////////////

#define getCheckedArrayRet(input, fail)                                        \
  auto const cell_##input = static_cast<const Variant&>(input).asTypedValue(); \
  if (UNLIKELY(!isArrayLikeType(cell_##input->m_type) &&                       \
    !isClsMethType(cell_##input->m_type))) {                                   \
    raise_expected_array_warning();                                            \
    return fail;                                                               \
  }                                                                            \
  if (isClsMethType(cell_##input->m_type)) raiseClsMethToVecWarningHelper();   \
  ArrNR arrNR_##input{isClsMethType(cell_##input->m_type) ?                    \
    clsMethToVecHelper(cell_##input->m_data.pclsmeth).detach() :               \
    cell_##input->m_data.parr};                                                \
  const Array& arr_##input = arrNR_##input.asArray();

#define getCheckedContainer(input)                                             \
  if (UNLIKELY(!isContainer(input) && !input.isClsMeth())) {                   \
    raise_expected_array_or_collection_warning();                              \
    return make_tv<KindOfNull>();                                              \
  }                                                                            \
  Variant var_##input(input);                                                  \
  tvCastToArrayInPlace<TypedValue*, IntishCast::Cast>(                         \
    var_##input.asTypedValue()                                                 \
  );                                                                           \
  assertx(var_##input.isArray());                                              \
  auto arr_##input = var_##input.toArray<IntishCast::Cast>();

#define getCheckedArray(input)        \
  getCheckedArrayRet(input, make_tv<KindOfNull>())
#define getCheckedArrayVariant(input) \
  getCheckedArrayRet(input, init_null())

}

#endif // incl_HPHP_EXT_ARRAY_H_
