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

#include "hphp/runtime/base/array-init.h"
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
  return make_map_array(s_internal, Unit::getSystemFunctions(),
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

bool HHVM_FUNCTION(is_callable, const Variant& v, bool syntax /* = false */,
                   OutputArg name /* = null */) {
  return is_callable(v, syntax, name.get());
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
  return vm_call_user_func(function, params, /* forward */ false,
                           /* check ref */ true);
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
  return vm_call_user_func(function, params, /* forward */ false,
                           /* check ref */ true);
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(func_get_arg, int arg_num) {
  auto const ar = GetCallerFrameForArgs();

  if (ar == nullptr) {
    return false;
  }
  if (ar->func()->isPseudoMain()) {
    raise_warning(
      "func_get_arg():  Called from the global scope - no function context"
    );
    return false;
  }
  if (arg_num < 0) {
    raise_warning(
      "func_get_arg():  The argument number should be >= 0"
    );
    return false;
  }
  if (arg_num >= ar->numArgs()) {
    raise_warning(
      "func_get_arg():  Argument %d not passed to function", arg_num
    );
    return false;
  }

  const int numParams = ar->m_func->numNonVariadicParams();

  if (arg_num < numParams) {
    // Formal parameter. Value is on the stack.
    TypedValue* loc =
      (TypedValue*)(uintptr_t(ar) - (arg_num + 1) * sizeof(TypedValue));
    return tvAsVariant(loc);
  }

  const int numArgs = ar->numArgs();
  const int extraArgs = numArgs - numParams;

  // Not a formal parameter.  Value is potentially in the
  // ExtraArgs/VarEnv.
  const int extraArgNum = arg_num - numParams;
  if (extraArgNum < extraArgs) {
    return tvAsVariant(ar->getExtraArg(extraArgNum));
  }

  return false;
}

const StaticString s_call_user_func("call_user_func");
const StaticString s_call_user_func_array("call_user_func_array");

Array hhvm_get_frame_args(const ActRec* ar, int offset) {
  while (ar && (ar->func()->name()->isame(s_call_user_func.get()) ||
                ar->func()->name()->isame(s_call_user_func_array.get()))) {
    ar = g_context->getPrevVMState(ar);
  }
  if (ar == nullptr) {
    return Array::CreateVArray();
  }
  int numParams = ar->func()->numNonVariadicParams();
  int numArgs = ar->numArgs();
  bool variadic = ar->func()->hasVariadicCaptureParam() &&
    !(ar->func()->attrs() & AttrMayUseVV);
  auto local = reinterpret_cast<TypedValue*>(
    uintptr_t(ar) - sizeof(TypedValue)
  );
  if (variadic && numArgs > numParams) {
    auto arr = local - numParams;
    if (isArrayType(arr->m_type) && arr->m_data.parr->hasPackedLayout()) {
      numArgs = numParams + arr->m_data.parr->size();
    } else {
      numArgs = numParams;
    }
  }
  local -= offset;
  VArrayInit retInit(std::max(numArgs - offset, 0));
  for (int i = offset; i < numArgs; ++i) {
    if (i < numParams) {
      // This corresponds to one of the function's formal parameters, so it's
      // on the stack.
      retInit.append(tvAsCVarRef(local));
      --local;
    } else if (variadic) {
      retInit.append(tvAsCVarRef(local).asCArrRef()[i - numParams]);
    } else {
      // This is not a formal parameter, so it's in the ExtraArgs.
      retInit.append(tvAsCVarRef(ar->getExtraArg(i - numParams)));
    }
  }

  return retInit.toArray();
}

#define FUNC_GET_ARGS_IMPL(offset) do {                                        \
  EagerCallerFrame cf;                                                         \
  ActRec* ar = cf.actRecForArgs();                                             \
  if (!ar || ar->func()->isPseudoMain()) {                                     \
    raise_warning(                                                             \
      "func_get_args():  Called from the global scope - no function context"   \
    );                                                                         \
    return false;                                                              \
  }                                                                            \
  return hhvm_get_frame_args(ar, offset);                                      \
} while(0)

Variant HHVM_FUNCTION(func_get_args) {
  FUNC_GET_ARGS_IMPL(0);
}

// __SystemLib\func_slice_args
Variant HHVM_FUNCTION(SystemLib_func_slice_args, int offset) {
  if (offset < 0) {
    offset = 0;
  }
  FUNC_GET_ARGS_IMPL(offset);
}

///////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(register_postsend_function, const Variant& function,
                   const Array& params /* = null_array */) {
  g_context->registerShutdownFunction(function, params,
                                      ExecutionContext::PostSend);
}

void HHVM_FUNCTION(register_shutdown_function, const Variant& function,
                   const Array& params /* = null_array */) {
  g_context->registerShutdownFunction(function, params,
                                      ExecutionContext::ShutDown);
}

void StandardExtension::initFunction() {
  HHVM_FE(get_defined_functions);
  HHVM_FE(function_exists);
  HHVM_FE(is_callable);
  HHVM_FE(call_user_func);
  HHVM_FE(call_user_func_array);
  HHVM_FE(func_get_arg);
  HHVM_FE(func_get_args);
  HHVM_FALIAS(__SystemLib\\func_slice_args, SystemLib_func_slice_args);
  HHVM_FE(register_postsend_function);
  HHVM_FE(register_shutdown_function);

  loadSystemlib("std_function");
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
