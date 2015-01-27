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
#include "hphp/runtime/ext/array/ext_array_idl.h"

#include <vector>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/ext/array/ext_array.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {

Variant f_array_map(int _argc, const Variant& callback, const Variant& arr1,
                    const Array& _argv /* = null_array */) {
  CallCtx ctx;
  ctx.func = NULL;
  if (!callback.isNull()) {
    EagerCallerFrame cf;
    vm_decode_function(callback, cf(), false, ctx);
  }
  const auto& cell_arr1 = *arr1.asCell();
  if (UNLIKELY(!isContainer(cell_arr1))) {
    raise_warning("array_map(): Argument #2 should be an array or collection");
    return init_null();
  }
  if (LIKELY(_argv.empty())) {
    // Handle the common case where the caller passed two
    // params (a callback and a container)
    if (!ctx.func) {
      if (cell_arr1.m_type == KindOfArray) {
        return arr1;
      } else {
        return arr1.toArray();
      }
    }
    ArrayInit ret(getContainerSize(cell_arr1), ArrayInit::Map{});
    bool keyConverted = (cell_arr1.m_type == KindOfArray);
    if (!keyConverted) {
      auto col_type = cell_arr1.m_data.pobj->getCollectionType();
      assert(col_type != Collection::Type::InvalidType);
      keyConverted = !Collection::isTypeWithPossibleIntStringKeys(col_type);
    }
    for (ArrayIter iter(arr1); iter; ++iter) {
      Variant result;
      g_context->invokeFuncFew((TypedValue*)&result, ctx, 1,
                               iter.secondRefPlus().asCell());
      // if keyConverted is false, it's possible that ret will have fewer
      // elements than cell_arr1; keys int(1) and string('1') may both be
      // present
      ret.add(iter.first(), result, keyConverted);
    }
    return ret.toVariant();
  }

  // Handle the uncommon case where the caller passed a callback
  // and two or more containers
  ArrayIter* iters =
    (ArrayIter*)smart_malloc(sizeof(ArrayIter) * (_argv.size() + 1));
  size_t numIters = 0;
  SCOPE_EXIT {
    while (numIters--) iters[numIters].~ArrayIter();
    smart_free(iters);
  };
  size_t maxLen = getContainerSize(cell_arr1);
  (void) new (&iters[numIters]) ArrayIter(cell_arr1);
  ++numIters;
  for (ArrayIter it(_argv); it; ++it, ++numIters) {
    const auto& c = *it.secondRefPlus().asCell();
    if (UNLIKELY(!isContainer(c))) {
      raise_warning("array_map(): Argument #%d should be an array or "
                    "collection", (int)(numIters + 2));
      (void) new (&iters[numIters]) ArrayIter(it.secondRefPlus().toArray());
    } else {
      (void) new (&iters[numIters]) ArrayIter(c);
      size_t len = getContainerSize(c);
      if (len > maxLen) maxLen = len;
    }
  }
  PackedArrayInit ret_ai(maxLen);
  for (size_t k = 0; k < maxLen; k++) {
    PackedArrayInit params_ai(numIters);
    for (size_t i = 0; i < numIters; ++i) {
      if (iters[i]) {
        params_ai.append(iters[i].secondRefPlus());
        ++iters[i];
      } else {
        params_ai.append(init_null_variant);
      }
    }
    Array params = params_ai.toArray();
    if (ctx.func) {
      Variant result;
      g_context->invokeFunc((TypedValue*)&result,
                              ctx.func, params, ctx.this_,
                              ctx.cls, nullptr, ctx.invName);
      ret_ai.append(result);
    } else {
      ret_ai.append(params);
    }
  }
  return ret_ai.toVariant();
}

}
