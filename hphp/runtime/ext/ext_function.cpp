/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/ext_json.h"
#include "hphp/runtime/ext/ext_class.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/libevent-http-client.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/exception.h"
#include "hphp/util/util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

using HPHP::JIT::CallerFrame;
using HPHP::JIT::EagerCallerFrame;

const StaticString
  s_internal("internal"),
  s_user("user");

Array f_get_defined_functions() {
  return make_map_array(s_internal, ClassInfo::GetSystemFunctions(),
                     s_user, ClassInfo::GetUserFunctions());
}

bool f_function_exists(const String& function_name,
                       bool autoload /* = true */) {
  return
    function_exists(function_name) ||
    (autoload &&
     AutoloadHandler::s_instance->autoloadFunc(function_name.get()) &&
     function_exists(function_name));
}

const StaticString
  s__invoke("__invoke"),
  s_Closure__invoke("Closure::__invoke"),
  s_colon2("::");

bool f_is_callable(CVarRef v, bool syntax /* = false */,
                   VRefParam name /* = null */) {
  bool ret = true;
  if (LIKELY(!syntax)) {
    CallerFrame cf;
    ObjectData* obj = NULL;
    HPHP::Class* cls = NULL;
    StringData* invName = NULL;
    const HPHP::Func* f = vm_decode_function(v, cf(), false, obj, cls,
                                                 invName, false);
    if (f == NULL) {
      ret = false;
    }
    if (invName != NULL) {
      decRefStr(invName);
    }
    if (!name.isReferenced()) return ret;
  }

  auto const tv_func = v.asCell();
  if (IS_STRING_TYPE(tv_func->m_type)) {
    if (name.isReferenced()) name = tv_func->m_data.pstr;
    return ret;
  }

  if (tv_func->m_type == KindOfArray) {
    CArrRef arr = tv_func->m_data.parr;
    CVarRef clsname = arr.rvalAtRef(int64_t(0));
    CVarRef mthname = arr.rvalAtRef(int64_t(1));
    if (arr.size() != 2 ||
        &clsname == &null_variant ||
        &mthname == &null_variant) {
      name = v.toString();
      return false;
    }

    auto const tv_meth = mthname.asCell();
    if (!IS_STRING_TYPE(tv_meth->m_type)) {
      if (name.isReferenced()) name = v.toString();
      return false;
    }

    auto const tv_cls = clsname.asCell();
    if (tv_cls->m_type == KindOfObject) {
      name = tv_cls->m_data.pobj->o_getClassName();
    } else if (IS_STRING_TYPE(tv_cls->m_type)) {
      name = tv_cls->m_data.pstr;
    } else {
      name = v.toString();
      return false;
    }

    name = concat3(name, s_colon2, tv_meth->m_data.pstr);
    return ret;
  }

  if (tv_func->m_type == KindOfObject) {
    ObjectData *d = tv_func->m_data.pobj;
    const Func* invoke = d->getVMClass()->lookupMethod(s__invoke.get());
    if (name.isReferenced()) {
      if (d->instanceof(c_Closure::classof())) {
        // Hack to stop the mangled name from showing up
        name = s_Closure__invoke;
      } else {
        name = d->o_getClassName() + "::__invoke";
      }
    }
    return invoke != NULL;
  }

  return false;
}

Variant f_call_user_func(int _argc, CVarRef function,
                         CArrRef _argv /* = null_array */) {
  return vm_call_user_func(function, _argv);
}

Variant f_call_user_func_array(CVarRef function, CVarRef params) {
  return vm_call_user_func(function, params);
}

Variant f_check_user_func_async(CVarRef handles, int timeout /* = -1 */) {
  raise_error("%s is no longer supported", __func__);
  return uninit_null();
}

Variant f_end_user_func_async(CObjRef handle,
                              int default_strategy /*= k_GLOBAL_STATE_IGNORE*/,
                              CVarRef additional_strategies /* = null */) {
  raise_error("%s is no longer supported", __func__);
  return uninit_null();
}

const StaticString
  s_func("func"),
  s_args("args"),
  s_exception("exception"),
  s_ret("ret");

String f_call_user_func_serialized(const String& input) {
  Variant out;
  try {
    Variant in = unserialize_from_string(input);
    out.set(s_ret, vm_call_user_func(in[s_func], in[s_args].toArray()));
  } catch (Object &e) {
    out.set(s_exception, e);
  }
  return f_serialize(out);
}

Variant f_call_user_func_array_rpc(const String& host, int port,
                                   const String& auth,
                                   int timeout, CVarRef function,
                                   CArrRef params) {
  return f_call_user_func_rpc(0, host, port, auth, timeout, function, params);
}

Variant f_call_user_func_rpc(int _argc, const String& host, int port,
                             const String& auth,
                             int timeout, CVarRef function,
                             CArrRef _argv /* = null_array */) {
  string shost = host.data();
  if (!RuntimeOption::DebuggerRpcHostDomain.empty()) {
    unsigned int pos = shost.find(RuntimeOption::DebuggerRpcHostDomain);
    if (pos != shost.length() - RuntimeOption::DebuggerRpcHostDomain.size()) {
      shost += RuntimeOption::DebuggerRpcHostDomain;
    }
  }

  string url = "http://";
  url += shost;
  url += ":";
  url += lexical_cast<string>(port);
  url += "/call_user_func_serialized?auth=";
  url += auth.data();

  Array blob = make_map_array(s_func, function, s_args, _argv);
  String message = f_serialize(blob);

  vector<string> headers;
  LibEventHttpClientPtr http = LibEventHttpClient::Get(shost, port);
  if (!http->send(url, headers, timeout < 0 ? 0 : timeout, false,
                  message.data(), message.size())) {
    raise_error("Unable to send RPC request");
    return false;
  }

  int code = http->getCode();
  if (code <= 0) {
    raise_error("Server timed out or unable to find specified URL: %s",
                url.c_str());
    return false;
  }

  int len = 0;
  char *response = http->recv(len);
  String sresponse(response, len, AttachString);
  if (code != 200) {
    raise_error("Internal server error: %d %s", code,
                HttpProtocol::GetReasonString(code));
    return false;
  }

  // This double decoding can be avoided by modifying RPC server to directly
  // take PHP serialization format.
  Variant res = unserialize_from_string(f_json_decode(sresponse));
  if (!res.isArray()) {
    raise_error("Internal protocol error");
    return false;
  }

  if (res.toArray().exists(s_exception)) {
    throw res[s_exception];
  }
  return res[s_ret];
}

Variant f_forward_static_call_array(CVarRef function, CArrRef params) {
  return f_forward_static_call(0, function, params);
}

Variant f_forward_static_call(int _argc, CVarRef function,
                              CArrRef _argv /* = null_array */) {
  // Setting the bound parameter to true tells vm_call_user_func()
  // propogate the current late bound class
  return vm_call_user_func(function, _argv, true);
}

Variant f_get_called_class() {
  EagerCallerFrame cf;
  ActRec* ar = cf();
  if (ar) {
    if (ar->hasThis()) return Variant(ar->getThis()->o_getClassName());
    if (ar->hasClass()) return Variant(ar->getClass()->preClass()->name());
  }
  return Variant(false);
}

String f_create_function(const String& args, const String& code) {
  return g_vmContext->createFunction(args, code);
}

///////////////////////////////////////////////////////////////////////////////

Variant f_func_get_arg(int arg_num) {
  CallerFrame cf;
  ActRec* ar = cf.actRecForArgs();

  if (ar == NULL) {
    return false;
  }
  if (ar->hasVarEnv() && ar->getVarEnv()->isGlobalScope()) {
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

  const int numParams = ar->m_func->numParams();

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

Variant func_get_arg(int num_args, CArrRef params, CArrRef args, int pos) {
  if (num_args <= params.size()) {
    if (pos >= 0 && pos < num_args) {
      return params.rvalAt(pos);
    }
  } else {
    if (pos >= 0) {
      int index = pos - params.size();
      if (index < 0) {
        return params.rvalAt(pos);
      }
      if (index < args.size()) {
        return args.rvalAt(index);
      }
    }
  }
  return false;
}

Array hhvm_get_frame_args(const ActRec* ar, int offset) {
  if (ar == NULL) {
    return Array();
  }
  int numParams = ar->m_func->numParams();
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
  if (ar && ar->hasVarEnv() && ar->getVarEnv()->isGlobalScope()) {             \
    raise_warning(                                                             \
      "func_get_args():  Called from the global scope - no function context"   \
    );                                                                         \
    return false;                                                              \
  }                                                                            \
  return hhvm_get_frame_args(ar, offset);                                      \
} while(0)

Variant f_func_get_args() {
  FUNC_GET_ARGS_IMPL(0);
}

Array func_get_args(int num_args, CArrRef params, CArrRef args) {
  if (params.empty() && args.empty()) return Array::Create();
  if (args.empty()) {
    if (num_args < params.size()) {
      return params.slice(0, num_args, false);
    }
    return params;
  }

  Array derefArgs;
  for (ArrayIter iter(args); iter; ++iter) {
    derefArgs.append(iter.second());
  }

  if (params.empty()) return derefArgs;
  assert(num_args > params.size());
  Array ret = Array(params).merge(derefArgs);
  return ret;
}

Variant f_hphp_func_slice_args(int offset) {
  if (offset < 0) {
    offset = 0;
  }
  FUNC_GET_ARGS_IMPL(offset);
}

int64_t f_func_num_args() {
  EagerCallerFrame cf;
  ActRec* ar = cf.actRecForArgs();
  if (ar == NULL) {
    return -1;
  }
  if (ar->hasVarEnv() && ar->getVarEnv()->isGlobalScope()) {
    raise_warning(
      "func_num_args():  Called from the global scope - no function context"
    );
    return -1;
  }
  return ar->numArgs();
}

///////////////////////////////////////////////////////////////////////////////

void f_register_postsend_function(int _argc, CVarRef function, CArrRef _argv /* = null_array */) {
  g_context->registerShutdownFunction(function, _argv,
                                      ExecutionContext::PostSend);
}

void f_register_shutdown_function(int _argc, CVarRef function, CArrRef _argv /* = null_array */) {
  g_context->registerShutdownFunction(function, _argv,
                                      ExecutionContext::ShutDown);
}

void f_register_cleanup_function(int _argc, CVarRef function, CArrRef _argv /* = null_array */) {
  g_context->registerShutdownFunction(function, _argv,
                                      ExecutionContext::CleanUp);
}

///////////////////////////////////////////////////////////////////////////////
}
