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

#include "hphp/runtime/base/builtin-functions.h"
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

TypedValue shapes_idx(const Class* self, ArrayArg arr,
                      TypedValue key, TypedValue def /*= uninit */) {
  if (isIntType(type(key))) {
    auto const result = arr->rval(val(key).num);
    if (result) return tvReturn(const_variant_ref{result});
  } else if (isStringType(type(key))) {
    auto const result = arr->rval(val(key).pstr);
    if (result) return tvReturn(const_variant_ref{result});
  } else {
    auto const message = folly::sformat(
      "Argument 2 passed to HH\\Shapes::idx() "
      "must be an instance of arraykey, {} given",
      getDataTypeString(type(key)).data()
    );
    raise_error(message);
    not_reached();
  }
  return tvReturn(tvAsCVarRef(&def));
}

static struct ShapesExtension final : Extension {
  ShapesExtension() : Extension("shapes") { }

  void moduleInit() override {
    HHVM_NAMED_STATIC_ME(HH\\Shapes, idx, shapes_idx);
    loadSystemlib("shapes");
  }
} s_shapes_extension;

///////////////////////////////////////////////////////////////////////////////

}  // namespace
}  // namespace HPHP
