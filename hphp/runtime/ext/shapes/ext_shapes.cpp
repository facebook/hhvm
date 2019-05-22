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

#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
namespace {

///////////////////////////////////////////////////////////////////////////////

TypedValue shapes_idx(const Class* self, TypedValue arr, TypedValue key,
                      const Variant& def /*= uninit Variant */) {
  auto const dvarrs = RuntimeOption::EvalHackArrDVArrs;
  auto const array_type_error = "Shapes::idx: $shape must be a darray";
  auto const index_type_error = "Shapes::idx: $index must be an arraykey";
  auto const ad = [&] {
    if (LIKELY(dvarrs ? isDictType(type(arr)) : isArrayType(type(arr)))) {
      return val(arr).parr;
    } else if (isObjectType(type(arr)) && val(arr).pobj->isCollection()) {
      return collections::asArray(val(arr).pobj);
    }
    SystemLib::throwInvalidArgumentExceptionObject(array_type_error);
    not_reached();
  }();
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatTypeHintNotices) &&
      !dvarrs && !ad->isDArray()) {
    SystemLib::throwInvalidArgumentExceptionObject(array_type_error);
  }
  if (isIntType(type(key))) {
    auto const result = ad->get(val(key).num, false);
    if (!result.is_dummy()) return tvReturn(const_variant_ref{result});
  } else if (isStringType(type(key))) {
    auto const result = ad->get(val(key).pstr, false);
    if (!result.is_dummy()) return tvReturn(const_variant_ref{result});
  } else {
    SystemLib::throwInvalidArgumentExceptionObject(index_type_error);
  }
  return tvReturn(def);
}

static struct ShapesExtension final : Extension {
  ShapesExtension() : Extension("shapes", get_PHP_VERSION().c_str()) { }

  void moduleInit() override {
    HHVM_NAMED_STATIC_ME(HH\\Shapes, idx, shapes_idx);
    loadSystemlib("shapes");
  }
} s_shapes_extension;

///////////////////////////////////////////////////////////////////////////////

}  // namespace
}  // namespace HPHP
