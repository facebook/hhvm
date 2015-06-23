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
#include "hphp/runtime/ext/std/ext_std_function.h"

#include <algorithm>
#include <vector>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/base/class-info.h"
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
                   VRefParam name /* = null */) {
  return is_callable(v, syntax, name.isReferenced() ?
                     name.wrapped().asTypedValue()->m_data.pref : nullptr);
}

Variant HHVM_FUNCTION(call_user_func, const Variant& function,
                      const Array& params /* = null_array */) {
  return vm_call_user_func(function, params);
}

Variant HHVM_FUNCTION(call_user_func_array, const Variant& function,
                      const Array& params) {
  return vm_call_user_func(function, params);
}

Variant HHVM_FUNCTION(check_user_func_async, const Variant& handles,
                     int timeout /* = -1 */) {
  raise_error("%s is no longer supported", __func__);
  return init_null();
}

Variant HHVM_FUNCTION(end_user_func_async, const Object& handle,
                     int default_strategy /*= k_GLOBAL_STATE_IGNORE*/,
                     const Variant& additional_strategies /* = null */) {
  raise_error("%s is no longer supported", __func__);
  return init_null();
}

Variant HHVM_FUNCTION(forward_static_call_array, const Variant& function,
                      const Array& params) {
  return HHVM_FN(forward_static_call)(function, params);
}

Variant HHVM_FUNCTION(forward_static_call, const Variant& function,
                              const Array& params /* = null_array */) {
  // Setting the bound parameter to true tells vm_call_user_func()
  // propogate the current late bound class
  return vm_call_user_func(function, params, true);
}

String HHVM_FUNCTION(create_function, const String& args, const String& code) {
  return g_context->createFunction(args, code);
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
static Variant func_get_arg_impl(int arg_num) {
  CallerFrame cf;
  ActRec* ar = cf.actRecForArgs();

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

Variant HHVM_FUNCTION(func_get_arg, int arg_num) {
  raise_disallowed_dynamic_call(
    "func_get_arg should not be called dynamically");
  return func_get_arg_impl(arg_num);
}
Variant HHVM_FUNCTION(SystemLib_func_get_arg_sl, int arg_num) {
  return func_get_arg_impl(arg_num);
}

Array hhvm_get_frame_args(const ActRec* ar, int offset) {
  if (ar == nullptr) {
    return Array();
  }
  int numParams = ar->m_func->numNonVariadicParams();
  int numArgs = ar->numArgs();

  PackedArrayInit retInit(std::max(numArgs - offset, 0));
  auto local = reinterpret_cast<TypedValue*>(
    uintptr_t(ar) - sizeof(TypedValue)
  );
  local -= offset;
  for (int i = offset; i < numArgs; ++i) {
    if (i < numParams) {
      // This corresponds to one of the function's formal parameters, so it's
      // on the stack.
      retInit.append(tvAsCVarRef(local));
      --local;
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
  raise_disallowed_dynamic_call(
    "func_get_args should not be called dynamically");
  FUNC_GET_ARGS_IMPL(0);
}
// __SystemLib\func_get_args_sl
Variant HHVM_FUNCTION(SystemLib_func_get_args_sl) {
  FUNC_GET_ARGS_IMPL(0);
}

// __SystemLib\func_slice_args
Variant HHVM_FUNCTION(SystemLib_func_slice_args, int offset) {
  if (offset < 0) {
    offset = 0;
  }
  FUNC_GET_ARGS_IMPL(offset);
}

ALWAYS_INLINE
static int64_t func_num_args_impl() {
  EagerCallerFrame cf;
  ActRec* ar = cf.actRecForArgs();
  if (ar == nullptr) {
    return -1;
  }
  if (ar->func()->isPseudoMain()) {
    raise_warning(
      "func_num_args():  Called from the global scope - no function context"
    );
    return -1;
  }
  return ar->numArgs();
}

int64_t HHVM_FUNCTION(func_num_args) {
  raise_disallowed_dynamic_call(
    "func_num_args should not be called dynamically");
  return func_num_args_impl();
}
int64_t HHVM_FUNCTION(SystemLib_func_num_arg_) {
  return func_num_args_impl();
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
  HHVM_FE(check_user_func_async);
  HHVM_FE(end_user_func_async);
  HHVM_FE(forward_static_call_array);
  HHVM_FE(forward_static_call);
  HHVM_FE(create_function);
  HHVM_FE(func_get_arg);
  HHVM_FALIAS(__SystemLib\\func_get_arg_sl, SystemLib_func_get_arg_sl);
  HHVM_FE(func_get_args);
  HHVM_FALIAS(__SystemLib\\func_get_args_sl, SystemLib_func_get_args_sl);
  HHVM_FALIAS(__SystemLib\\func_slice_args, SystemLib_func_slice_args);
  HHVM_FE(func_num_args);
  HHVM_FALIAS(__SystemLib\\func_num_arg_, SystemLib_func_num_arg_);
  HHVM_FE(register_postsend_function);
  HHVM_FE(register_shutdown_function);

  loadSystemlib("std_function");
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
