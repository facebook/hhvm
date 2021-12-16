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

#include "hphp/runtime/ext/thrift/adapter.h"

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/thrift/util.h"
#include "hphp/runtime/vm/class.h"

namespace HPHP {
namespace thrift {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_fieldAdapter("field_adapter");

Class* getFieldAdapter(const Array& spec) {
  const auto adapterName = spec.lookup(s_fieldAdapter, AccessFlags::Key);
  if (isNullType(adapterName.type())) {
    return nullptr;
  }
  const auto adapterNameString = tvCastToString(adapterName);
  const auto adapter = Class::load(adapterNameString.get());
  if (!adapter) {
    thrift_error(
        folly::sformat("adapter class {} doesn't exist", adapterNameString),
        ERR_INVALID_DATA);
  }
  return adapter;
}

void
setThriftType(Variant value, const Object& obj, const String& fieldName) {
  auto setter_name = "set_" + fieldName;
  obj->o_invoke_few_args(setter_name, RuntimeCoeffects::defaults(), 1,value);
}

Variant getThriftType(const Object& obj, const String& fieldName) {
  auto getter_name = "get_" + fieldName;
  return obj->o_invoke_few_args(getter_name, RuntimeCoeffects::defaults(), 0);
}

///////////////////////////////////////////////////////////////////////////////
} // namespace thrift
} // namespace HPHP
