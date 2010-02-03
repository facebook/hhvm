/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/ext/ext_function.h>
#include <cpp/base/class_info.h>
#include <util/exception.h>
#include <util/util.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array f_get_defined_functions() {
  Array ret;
  ret.set("internal", ClassInfo::GetSystemFunctions());
  ret.set("user", ClassInfo::GetUserFunctions());
  return ret;
}

bool f_function_exists(CStrRef function_name) {
  const ClassInfo::MethodInfo *info =
    ClassInfo::FindFunction(function_name.data());
  if (info) {
    if (info->attribute & ClassInfo::IsSystem) return true;
    if (info->attribute & ClassInfo::IsVolatile) {
      return ((Globals*)get_global_variables())->
             function_exists(function_name.data());
    } else {
      return true;
    }
  } else {
    return false;
  }
}

bool f_is_callable(CVarRef v, bool syntax /* = false */,
                   Variant name /* = null */) {
  if (v.isString()) {
    if (!name.isNull()) name = v;
    if (syntax) return true;

    string lowered = Util::toLower((const char *)v.toString());
    size_t c = lowered.find("::");
    if (c != 0 && c != string::npos && c+2 < lowered.size()) {
      string classname = lowered.substr(0, c);
      string methodname = lowered.substr(c+2);
      const ClassInfo *clsInfo = ClassInfo::FindClass(classname.c_str());
      if (clsInfo) {
        return clsInfo->hasMethod(methodname.c_str());
      }
      return false;
    }

    return f_function_exists(v.toString());
  }

  if (v.is(KindOfArray)) {
    Array arr = v.toArray();
    if (arr.size() == 2 && arr.exists(0LL) && arr.exists(1LL)) {
      Variant v0 = arr.rvalAt(0LL);
      Variant v1 = arr.rvalAt(1LL);
      if (v0.is(KindOfObject)) {
        v0 = v0.toObject()->o_getClassName();
      }
      if (v0.isString() && v1.isString()) {
        if (!name.isNull()) {
          name = v0.toString() + "::" + v1.toString();
        }
        if (same(v0, "self") || same(v0, "parent")) {
          throw NotImplementedException("augmenting class scope info");
        }
        const ClassInfo *clsInfo = ClassInfo::FindClass(v0.toString());
        if (clsInfo) {
          return clsInfo->hasMethod(v1.toString());
        }
        return false;
      }
    }
  }

  if (!name.isNull()) {
    name = v.toString();
  }
  return false;
}

Variant f_call_user_func(int _argc, CVarRef function, CArrRef _argv /* = null_array */) {
  return f_call_user_func_array(function, _argv);
}

String f_create_function(CStrRef args, CStrRef code) {
  throw NotSupportedException(__func__, "dynamic coding");
}

///////////////////////////////////////////////////////////////////////////////

Variant f_func_get_arg(int arg_num) {
  throw FatalErrorException("bad HPHP code generation");
}
Variant func_get_arg(int num_args, CArrRef params, CArrRef args, int pos) {
  FUNCTION_INJECTION(func_get_arg);
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
  return null;
}

Array f_func_get_args() {
  throw FatalErrorException("bad HPHP code generation");
}
Array func_get_args(int num_args, CArrRef params, Array &args) {
  FUNCTION_INJECTION(func_get_args);
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
