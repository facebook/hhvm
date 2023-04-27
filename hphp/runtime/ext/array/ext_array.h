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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/base/zend-collator.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(array_key_exists,
                   const Variant& key,
                   const Variant& search);
TypedValue HHVM_FUNCTION(array_keys,
                         TypedValue input);
TypedValue HHVM_FUNCTION(array_search,
                         const Variant& needle,
                         const Variant& haystack,
                         bool strict = false);
TypedValue HHVM_FUNCTION(array_splice,
                         Variant& input,
                         int64_t offset,
                         const Variant& length = uninit_variant,
                         const Variant& replacement = uninit_variant);
TypedValue HHVM_FUNCTION(array_values,
                         const Variant& input);
int64_t HHVM_FUNCTION(count,
                      const Variant& var,
                      int64_t mode = 0);
bool HHVM_FUNCTION(ksort,
                   Variant& array,
                   int64_t sort_flags = 0);
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
  if (UNLIKELY(!isArrayLikeType(cell_##input->m_type))) {                      \
    raise_expected_array_warning();                                            \
    return fail;                                                               \
  }                                                                            \
  ArrNR arrNR_##input{cell_##input->m_data.parr};                              \
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
