/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_function.h>
#include <runtime/ext/ext_json.h>
#include <runtime/ext/ext_class.h>
#include <runtime/base/class_info.h>
#include <runtime/base/fiber_async_func.h>
#include <runtime/base/util/libevent_http_client.h>
#include <runtime/base/server/http_protocol.h>
#include <util/exception.h>
#include <util/util.h>

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static StaticString s_parent("parent");
static StaticString s_self("self");

Array f_get_defined_functions() {
  Array ret;
  ret.set("internal", ClassInfo::GetSystemFunctions());
  ret.set("user", ClassInfo::GetUserFunctions());
  return ret;
}

bool f_function_exists(CStrRef function_name) {
  return function_exists(function_name);
}

bool f_is_callable(CVarRef v, bool syntax /* = false */,
                   VRefParam name /* = null */) {
  bool ret = true;
  if (LIKELY(!syntax)) {
    MethodCallPackage mcp;
    String classname, methodname;
    bool doBind;
    ret = get_user_func_handler(v, true, mcp,
                                     classname, methodname, doBind, false);
    if (ret && mcp.ci->m_flags & (CallInfo::Protected|CallInfo::Private)) {
      classname = mcp.getClassName();
      if (!ClassInfo::HasAccess(classname, *mcp.name,
                                mcp.ci->m_flags & CallInfo::StaticMethod ||
                                !mcp.obj,
                                mcp.obj)) {
        ret = false;
      }
    }
    if (!name.isReferenced()) return ret;
  }

  Variant::TypedValueAccessor tv_func = v.getTypedAccessor();
  if (Variant::IsString(tv_func)) {
    if (name.isReferenced()) name = Variant::GetStringData(tv_func);
    return ret;
  }

  if (Variant::GetAccessorType(tv_func) == KindOfArray) {
    CArrRef arr = Variant::GetAsArray(tv_func);
    CVarRef clsname = arr.rvalAtRef(0LL);
    CVarRef mthname = arr.rvalAtRef(1LL);
    if (arr.size() != 2 ||
        &clsname == &null_variant ||
        &mthname == &null_variant) {
      name = v.toString();
      return false;
    }

    Variant::TypedValueAccessor tv_meth = mthname.getTypedAccessor();
    if (!Variant::IsString(tv_meth)) {
      if (name.isReferenced()) name = v.toString();
      return false;
    }

    Variant::TypedValueAccessor tv_cls = clsname.getTypedAccessor();
    if (Variant::GetAccessorType(tv_cls) == KindOfObject) {
      name = Variant::GetObjectData(tv_cls)->o_getClassName();
    } else if (Variant::IsString(tv_cls)) {
      name = Variant::GetStringData(tv_cls);
    } else {
      name = v.toString();
      return false;
    }

    name = concat3(name, "::", Variant::GetAsString(tv_meth));
    return ret;
  }

  if (Variant::GetAccessorType(tv_func) == KindOfObject) {
    ObjectData *d = Variant::GetObjectData(tv_func);
    void *extra;
    if (d->t___invokeCallInfoHelper(extra)) {
      name = d->o_getClassName() + "::__invoke";
      return ret;
    }
    if (name.isReferenced()) {
      name = v.toString();
    }
  }

  return false;
}

Variant f_call_user_func(int _argc, CVarRef function, CArrRef _argv /* = null_array */) {
  return f_call_user_func_array(function, _argv);
}

Object f_call_user_func_array_async(CVarRef function, CArrRef params) {
#ifdef CUFA_ASYNC_DEPRECATION_MSG
  raise_warning(CUFA_ASYNC_DEPRECATION_MSG);
#else
  raise_warning("call_user_func_array_async() is deprecated");
#endif

  return FiberAsyncFunc::Start(function, params);
}

Object f_call_user_func_async(int _argc, CVarRef function,
                               CArrRef _argv /* = null_array */) {
#ifdef CUF_ASYNC_DEPRECATION_MSG
  raise_warning(CUF_ASYNC_DEPRECATION_MSG);
#else
  raise_warning("call_user_func_async() is deprecated");
#endif

  return FiberAsyncFunc::Start(function, _argv);
}

Variant f_check_user_func_async(CVarRef handles, int timeout /* = -1 */) {
  if (handles.isArray()) {
    return FiberAsyncFunc::Status(handles, timeout);
  }
  Array ret = FiberAsyncFunc::Status(CREATE_VECTOR1(handles), timeout);
  return !ret.empty();
}

Variant f_end_user_func_async(CObjRef handle,
                              int default_strategy /* = k_GLOBAL_STATE_IGNORE */,
                              CVarRef additional_strategies /* = null */) {
  return FiberAsyncFunc::Result(handle,
                                (FiberAsyncFunc::Strategy)default_strategy,
                                additional_strategies);
}

String f_call_user_func_serialized(CStrRef input) {
  Variant out;
  try {
    Variant in = f_unserialize(input);
    out.set("ret", f_call_user_func_array(in["func"], in["args"]));
  } catch (Object &e) {
    out.set("exception", e);
  }
  return f_serialize(out);
}

Variant f_call_user_func_array_rpc(CStrRef host, int port, CStrRef auth,
                                   int timeout, CVarRef function,
                                   CArrRef params) {
  return f_call_user_func_rpc(0, host, port, auth, timeout, function, params);
}

Variant f_call_user_func_rpc(int _argc, CStrRef host, int port, CStrRef auth,
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

  Array blob = CREATE_MAP2("func", function, "args", _argv);
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
  Variant res = f_unserialize(f_json_decode(sresponse));
  if (!res.isArray()) {
    raise_error("Internal protocol error");
    return false;
  }

  if (res.toArray().exists("exception")) {
    throw res["exception"];
  }
  return res["ret"];
}

Variant f_forward_static_call_array(CVarRef function, CArrRef params) {
  return f_forward_static_call(0, function, params);
}

Variant f_forward_static_call(int _argc, CVarRef function, CArrRef _argv /* = null_array */) {
  CStrRef cls = FrameInjection::GetClassName();
  if (cls.empty()) {
    raise_error("Cannot call forward_static_call() "
                "when no class scope is active");
    return null;
  }
  return f_call_user_func_array(function, _argv, true);
}

Variant f_get_called_class() {
  CStrRef cls = FrameInjection::GetStaticClassName(
    ThreadInfo::s_threadInfo.getNoCheck());
  return cls.size() ? Variant(cls.get()) : Variant(false);
}

String f_create_function(CStrRef args, CStrRef code) {
  throw NotSupportedException(__func__, "dynamic coding");
}

///////////////////////////////////////////////////////////////////////////////

Variant f_func_get_arg(int arg_num) {
  throw FatalErrorException("bad HPHP code generation");
}
Variant func_get_arg(int num_args, CArrRef params, CArrRef args, int pos) {
  FUNCTION_INJECTION_BUILTIN(func_get_arg);
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

Array f_func_get_args() {
  throw FatalErrorException("bad HPHP code generation");
}
Array func_get_args(int num_args, CArrRef params, CArrRef args) {
  FUNCTION_INJECTION_BUILTIN(func_get_args);
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
  ASSERT(num_args > params.size());
  Array ret = Array(params).merge(derefArgs);
  return ret;
}

int f_func_num_args() {
  // we shouldn't be here, since HPHP code generation will inline this function
  ASSERT(false);
  return -1;
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

bool f_register_tick_function(int _argc, CVarRef function, CArrRef _argv /* = null_array */) {
  g_context->registerTickFunction(function, _argv);
  return true;
}

void f_unregister_tick_function(CVarRef function_name) {
  g_context->unregisterTickFunction(function_name);
}

///////////////////////////////////////////////////////////////////////////////
}
