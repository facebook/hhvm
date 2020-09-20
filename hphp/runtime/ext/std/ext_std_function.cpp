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
#include "hphp/runtime/ext/std/ext_std_function.h"

#include <algorithm>
#include <vector>

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/base/libevent-http-client.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/exception.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

using std::string;

const StaticString
  s_internal("internal"),
  s_user("user");

Array HHVM_FUNCTION(get_defined_functions) {
  return make_darray(s_internal, Unit::getSystemFunctions(),
                     s_user, Unit::getUserFunctions());
}

bool HHVM_FUNCTION(function_exists, const String& function_name,
                       bool autoload /* = true */) {
  return
    function_exists(function_name) ||
    (autoload &&
     AutoloadHandler::s_instance->autoloadFunc(function_name.get()) &&
     function_exists(function_name));
}

bool HHVM_FUNCTION(is_callable, const Variant& v, bool syntax /* = false */) {
  return is_callable(v, syntax, nullptr);
}

bool HHVM_FUNCTION(is_callable_with_name, const Variant& v, bool syntax,
                   Variant& name) {
  return is_callable(v, syntax, &name);
}

Variant HHVM_FUNCTION(call_user_func, const Variant& function,
                      const Array& params /* = null_array */) {
    auto const warning = "call_user_func() is deprecated and subject"
   " to removal from the Hack language";
   switch (RuntimeOption::DisableCallUserFunc) {
     case 0:  break;
     case 1:  raise_warning(warning); break;
     default: raise_error(warning);
   }
  return vm_call_user_func(function, params, /* check ref */ true);
}

Variant HHVM_FUNCTION(call_user_func_array, const Variant& function,
                      const Variant& params) {
  auto const warning = "call_user_func_array() is deprecated and subject"
  " to removal from the Hack language";
  switch (RuntimeOption::DisableCallUserFuncArray) {
    case 0:  break;
    case 1:  raise_warning(warning); break;
    default: raise_error(warning);
  }
  if (UNLIKELY(!isContainer(params))) {
    raise_warning("call_user_func_array() expects parameter 2 to be an array "
                  "or collection, %s given",
                  getDataTypeString(params.getType()).data());
    return init_null();
  }
  return vm_call_user_func(function, params, /* check ref */ true);
}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_call_user_func("call_user_func");
const StaticString s_call_user_func_array("call_user_func_array");

Array hhvm_get_frame_args(const ActRec* ar) {
  ARRPROV_USE_RUNTIME_LOCATION();
  while (ar && (ar->func()->name()->isame(s_call_user_func.get()) ||
                ar->func()->name()->isame(s_call_user_func_array.get()))) {
    ar = g_context->getPrevVMState(ar);
  }

  auto ret = Array::CreateVArray();
  if (!ar) return ret;

  int numNonVariadic = ar->func()->numNonVariadicParams();
  for (int i = 0; i < numNonVariadic; ++i) {
    auto const val = frame_local(ar, i);
    // If there's a default argument that is not passed, variadic arguments
    // must be empty
    if (type(val) == KindOfUninit) return ret;
    ret.append(tvAsCVarRef(*val));
  }
  if (!ar->func()->hasVariadicCaptureParam()) return ret;
  assertx(numNonVariadic == ret.size());
  auto const arr = frame_local(ar, numNonVariadic);
  if (tvIsHAMSafeVArray(arr)) {
    // If there are still args that haven't been accounted for, they have
    // been shuffled into a packed array stored in the variadic capture param.
    IterateVNoInc(val(arr).parr, [&](TypedValue v) { ret.append(v); });
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(register_postsend_function, const Variant& function) {
  g_context->registerShutdownFunction(function, ExecutionContext::PostSend);
}

void HHVM_FUNCTION(register_shutdown_function, const Variant& function) {
  g_context->registerShutdownFunction(function, ExecutionContext::ShutDown);
}

void StandardExtension::initFunction() {
  HHVM_FE(get_defined_functions);
  HHVM_FE(function_exists);
  HHVM_FE(is_callable);
  HHVM_FE(is_callable_with_name);
  HHVM_FE(call_user_func);
  HHVM_FE(call_user_func_array);
  HHVM_FE(register_postsend_function);
  HHVM_FE(register_shutdown_function);
  HHVM_FALIAS(HH\\fun_get_function, HH_fun_get_function);

  loadSystemlib("std_function");
}

///////////////////////////////////////////////////////////////////////////////

String HHVM_FUNCTION(HH_fun_get_function, TypedValue v) {
  if (!tvIsFunc(v)) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Argument 1 passed to {}() must be a fun",
      __FUNCTION__+5));
  }
  auto const f = val(v).pfunc;
  return f->nameStr();
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
