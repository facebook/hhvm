/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/externals.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/variable_unserializer.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/execution_context.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/eval/runtime/code_coverage.h>
#include <runtime/ext/ext_process.h>
#include <runtime/ext/ext_class.h>
#include <runtime/ext/ext_function.h>
#include <runtime/ext/ext_file.h>
#include <util/logger.h>
#include <util/util.h>
#include <util/process.h>

#include <limits>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// static strings

static StaticString s_offsetExists("offsetExists");
static StaticString s___autoload("__autoload");
static StaticString s_self("self");
static StaticString s_parent("parent");
static StaticString s_static("static");

///////////////////////////////////////////////////////////////////////////////

bool class_exists(CStrRef class_name, bool autoload /* = true */) {
  return f_class_exists(class_name, autoload);
}

String get_static_class_name(CVarRef objOrClassName) {
  if (objOrClassName.isString()) {
    return objOrClassName.toString();
  }
  if (objOrClassName.isObject()) {
    return objOrClassName.toObject()->o_getClassName();
  }
  raise_error("Class name must be a valid object or a string");
  return "";
}

Variant getDynamicConstant(CVarRef v, CStrRef name) {
  if (isInitialized(v)) return v;
  raise_notice("Use of undefined constant %s - assumed '%s'",
               name.c_str(), name.c_str());
  return name;
}

String getUndefinedConstant(CStrRef name) {
  raise_notice("Use of undefined constant %s - assumed '%s'",
               name.c_str(), name.c_str());
  return name;
}

Variant f_call_user_func_array(CVarRef function, CArrRef params,
                               bool bound /* = false */) {
  if (function.isString() || function.instanceof("closure")) {
    String sfunction = function.toString();
    int c = sfunction.find("::");
    if (c != 0 && c != String::npos && c + 2 < sfunction.size()) {
#ifdef ENABLE_LATE_STATIC_BINDING
      if (!bound) {
        return invoke_static_method_bind(sfunction.substr(0, c),
                                         sfunction.substr(c + 2), params,
                                         false);
      }
#endif /* ENABLE_LATE_STATIC_BINDING */
      return invoke_static_method(sfunction.substr(0, c),
                                  sfunction.substr(c + 2), params,
                                  false);
    }
    return invoke(sfunction, params, -1, true, false);
  } else if (function.is(KindOfArray)) {
    Array arr = function.toArray();
    if (!(arr.size() == 2 && arr.exists(0LL) && arr.exists(1LL))) {
      throw_invalid_argument("function: not a valid callback array");
      return null;
    }
    Variant classname = arr.rvalAt(0LL);
    Variant methodname = arr.rvalAt(1LL);
    if (!methodname.isString()) {
      throw_invalid_argument("function: methodname not string");
      return null;
    }
    String method = methodname.toString();
    if (classname.is(KindOfObject)) {
      int c = method.find("::");
      if (c != 0 && c != String::npos && c + 2 < method.size()) {
        String cls = method.substr(0, c);
        if (cls->same(s_self.get())) {
          cls = FrameInjection::GetClassName(true);
        } else if (cls->same(s_parent.get())) {
          cls = FrameInjection::GetParentClassName(true);
        }
        return classname.getObjectData()->o_invoke_ex
          (cls, method.substr(c + 2), params, false);
      }
      ObjectData *obj = classname.getObjectData();
#ifdef ENABLE_LATE_STATIC_BINDING
      FrameInjection::StaticClassNameHelper scn(
        ThreadInfo::s_threadInfo.getNoCheck(),
        bound ? FrameInjection::GetClassName(true) : obj->o_getClassName());
#endif
      return obj->o_invoke(method, params, -1, false);
    } else {
      if (!classname.isString()) {
        throw_invalid_argument("function: classname not string");
        return null;
      }
      String sclass = classname.toString();
      if (sclass->same(s_self.get())) {
        sclass = FrameInjection::GetClassName(true);
      } else if (sclass->same(s_parent.get())) {
        sclass = FrameInjection::GetParentClassName(true);
      }
      ObjectData *obj = FrameInjection::GetThis(true);
      if (obj && obj->o_instanceof(sclass)) {
        return obj->o_invoke_ex(sclass, method, params, false);
      }
#ifdef ENABLE_LATE_STATIC_BINDING
      if (!bound) {
        return invoke_static_method_bind(sclass, method, params, false);
      }
#endif /* ENABLE_LATE_STATIC_BINDING */
      return invoke_static_method(sclass, method, params, false);
    }
  }
  throw_invalid_argument("function: not string or array");
  return null;
}

bool get_user_func_handler(CVarRef function, MethodCallPackage &mcp,
                           String &classname, String &methodname) {
  mcp.noFatal();
  if (function.isString() || function.instanceof("closure")) {
    String sfunction = function.toString();
    int c = sfunction.find("::");
    if (c != 0 && c != String::npos && c + 2 < sfunction.size()) {
      classname = sfunction.substr(0, c);
      methodname = sfunction.substr(c + 2);
#ifdef ENABLE_LATE_STATIC_BINDING
      if (classname->same(s_static.get())) {
        ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
        classname = FrameInjection::GetStaticClassName(ti);
      }
#endif
      mcp.dynamicNamedCall(classname, methodname);
      if (mcp.ci) return true;
      raise_warning("call_user_func to non-existent function %s",
                    sfunction.data());
      return false;
    }
    methodname = sfunction;
    mcp.functionNamedCall(methodname);
    if (mcp.ci) return true;
    raise_warning("call_user_func to non-existent function %s",
                  sfunction.data());
    return false;
  } else if (function.is(KindOfArray)) {
    Array arr = function.toArray();
    if (!(arr.size() == 2 && arr.exists(0LL) && arr.exists(1LL))) {
      throw_invalid_argument("function: not a valid callback array");
      return false;
    }
    Variant clsname = arr.rvalAt(0LL);
    Variant mthname = arr.rvalAt(1LL);
    if (!mthname.isString()) {
      throw_invalid_argument("function: methodname not string");
      return false;
    }
    String method = mthname.toString();
    if (clsname.is(KindOfObject)) {
      Object obj = clsname.toObject();
      int c = method.find("::");
      if (c != 0 && c != String::npos && c + 2 < method.size()) {
        String cls = method.substr(0, c);
        if (cls->same(s_self.get())) {
          cls = FrameInjection::GetClassName(true);
        } else if (cls->same(s_parent.get())) {
          cls = FrameInjection::GetParentClassName(true);
        }
        methodname = method.substr(c + 2);
        mcp.methodCallEx(obj, methodname);
        if (obj->o_get_call_info_ex(cls, mcp)) {
          return true;
        }
        return false;
      }
      methodname = method;
      mcp.methodCall(obj, methodname);
      if (mcp.ci) return true;
      return false;
    } else {
      if (!clsname.isString()) {
        throw_invalid_argument("function: classname not string");
        return false;
      }
      String sclass = clsname.toString();
      if (sclass->same(s_self.get())) {
        sclass = FrameInjection::GetClassName(true);
      } else if (sclass->same(s_parent.get())) {
        sclass = FrameInjection::GetParentClassName(true);
      }
      ObjectData *obj = FrameInjection::GetThis(true);
      if (obj && obj->o_instanceof(sclass)) {
        methodname = method;
        mcp.methodCallEx(obj, methodname);
        if (obj->o_get_call_info_ex(sclass, mcp)) {
          return true;
        }
        return false;
      }
#ifdef ENABLE_LATE_STATIC_BINDING
      if (sclass->same(s_static.get())) {
        ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
        sclass = FrameInjection::GetStaticClassName(ti);
      }
#endif
      classname = sclass;
      methodname = method;
      mcp.dynamicNamedCall(classname, methodname);
      if (mcp.ci) return true;
      raise_warning("call_user_func to non-existent function %s::%s",
                    sclass.data(), method.data());
      return false;
    }
    raise_warning("call_user_func to non-existent function");
    return false;
  }
  throw_invalid_argument("function: not string or array");
  return false;
}

Variant invoke(CStrRef function, CArrRef params, int64 hash /* = -1 */,
               bool tryInterp /* = true */, bool fatal /* = true */) {
  StringData *sd = function.get();
  ASSERT(sd && sd->data());
  return invoke(sd->data(), params, hash < 0 ? sd->hash() : hash,
                tryInterp, fatal);
}

Variant invoke(const char *function, CArrRef params, int64 hash /* = -1*/,
               bool tryInterp /* = true */, bool fatal /* = true */) {
  const CallInfo *ci;
  void *extra;
  if (get_call_info(ci, extra, function, hash)) {
    return (ci->getFunc())(extra, params);
  }
  return invoke_failed(function, params, hash, fatal);
}

Variant invoke_builtin(const char *s, CArrRef params, int64 hash, bool fatal) {
  const CallInfo *ci;
  void *extra;
  if (get_call_info_builtin(ci, extra, s, hash)) {
    return (ci->getFunc())(extra, params);
  } else {
    return invoke_failed(s, params, hash, fatal);
  }
}

Variant invoke_static_method(CStrRef s, CStrRef method, CArrRef params,
                             bool fatal /* = true */) {
  MethodCallPackage mcp;
  if (!fatal) mcp.noFatal();
  mcp.dynamicNamedCall(s, method, -1);
  if (mcp.ci) {
    return (mcp.ci->getMeth())(mcp, params);
  } else {
    o_invoke_failed(s.data(), method.data(), fatal);
    return null;
  }
}

Variant invoke_failed(const char *func, CArrRef params, int64 hash,
                      bool fatal /* = true */) {
  INTERCEPT_INJECTION_ALWAYS("?", func, params, ref(r));

  if (fatal) {
    throw InvalidFunctionCallException(func);
  } else {
    raise_warning("call_user_func to non-existent function %s", func);
    return false;
  }
}

Variant o_invoke_failed(const char *cls, const char *meth,
                        bool fatal /* = true */) {
   if (fatal) {
    string msg = "Unknown method ";
    msg += cls;
    msg += "::";
    msg += meth;
    throw FatalErrorException(msg.c_str());
  } else {
    raise_warning("call_user_func to non-existent method %s::%s", cls, meth);
    return false;
  }
}

Array collect_few_args(int count, INVOKE_FEW_ARGS_IMPL_ARGS) {
  switch (count) {
  case 0: {
    return Array();
  }
  case 1: {
    return Array(ArrayInit(1, true).set(a0).create());
  }
  case 2: {
    return Array(ArrayInit(2, true).set(a0).set(a1).create());
  }
  case 3: {
    return Array(ArrayInit(3, true).set(a0).set(a1).set(a2).create());
  }
#if INVOKE_FEW_ARGS_COUNT > 3
  case 4: {
    return Array(ArrayInit(4, true).set(a0).set(a1).set(a2).
                                    set(a3).create());
  }
  case 5: {
    return Array(ArrayInit(5, true).set(a0).set(a1).set(a2).
                                    set(a3).set(a4).create());
  }
  case 6: {
    return Array(ArrayInit(6, true).set(a0).set(a1).set(a2).
                                    set(a3).set(a4).set(a5).create());
  }
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
  case 7: {
    return Array(ArrayInit(7, true).set(a0).set(a1).set(a2).
                                    set(a3).set(a4).set(a5).
                                    set(a6).create());
  }
  case 8: {
    return Array(ArrayInit(8, true).set(a0).set(a1).set(a2).
                                    set(a3).set(a4).set(a5).
                                    set(a6).set(a7).create());
  }
  case 9: {
    return Array(ArrayInit(9, true).set(a0).set(a1).set(a2).
                                    set(a3).set(a4).set(a5).
                                    set(a6).set(a7).set(a8).create());
  }
  case 10: {
    return Array(ArrayInit(10, true).set(a0).set(a1).set(a2).
                                     set(a3).set(a4).set(a5).
                                     set(a6).set(a7).set(a8).
                                     set(a9).create());
  }
#endif
  default:
    ASSERT(false);
  }
  return null;
}

Array collect_few_args_ref(int count, INVOKE_FEW_ARGS_IMPL_ARGS) {
  switch (count) {
  case 0: {
    return Array();
  }
  case 1: {
    return Array(ArrayInit(1, true).setRef(a0).create());
  }
  case 2: {
    return Array(ArrayInit(2, true).setRef(a0).setRef(a1).create());
  }
  case 3: {
    return Array(ArrayInit(3, true).setRef(a0).setRef(a1).setRef(a2).create());
  }
#if INVOKE_FEW_ARGS_COUNT > 3
  case 4: {
    return Array(ArrayInit(4, true).setRef(a0).setRef(a1).setRef(a2).
                                    setRef(a3).create());
  }
  case 5: {
    return Array(ArrayInit(5, true).setRef(a0).setRef(a1).setRef(a2).
                                    setRef(a3).setRef(a4).create());
  }
  case 6: {
    return Array(ArrayInit(6, true).setRef(a0).setRef(a1).setRef(a2).
                                    setRef(a3).setRef(a4).setRef(a5).create());
  }
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
  case 7: {
    return Array(ArrayInit(7, true).setRef(a0).setRef(a1).setRef(a2).
                                    setRef(a3).setRef(a4).setRef(a5).
                                    setRef(a6).create());
  }
  case 8: {
    return Array(ArrayInit(8, true).setRef(a0).setRef(a1).setRef(a2).
                                    setRef(a3).setRef(a4).setRef(a5).
                                    setRef(a6).setRef(a7).create());
  }
  case 9: {
    return Array(ArrayInit(9, true).setRef(a0).setRef(a1).setRef(a2).
                                    setRef(a3).setRef(a4).setRef(a5).
                                    setRef(a6).setRef(a7).setRef(a8).create());
  }
  case 10: {
    return Array(ArrayInit(10, true).setRef(a0).setRef(a1).setRef(a2).
                                     setRef(a3).setRef(a4).setRef(a5).
                                     setRef(a6).setRef(a7).setRef(a8).
                                     setRef(a9).create());
  }
#endif
  default:
    ASSERT(false);
  }
  return null;
}

void throw_instance_method_fatal(const char *name) {
  if (!strstr(name, "::__destruct")) {
    raise_error("Non-static method %s() cannot be called statically", name);
  }
}

Object create_object(const char *s, CArrRef params, bool init /* = true */,
                     ObjectData *root /* = NULL */) {
  Object o(create_object_only(s, root));
  if (init) {
    MethodCallPackage mcp;
    mcp.construct(o);
    if (mcp.ci) {
      (mcp.ci->getMeth())(mcp, params);
    }
  }
  return o;
}

void pause_and_exit() {
  // NOTE: This is marked as __attribute__((noreturn)) in base/types.h
  // Signal sent, nothing can be trusted, don't do anything, as we might
  // write bad data, including calling exit handlers or destructors until the
  // signal handler (StackTrace) has had a chance to exit.
  sleep(300);
  // Should abort first, but it not try to exit
  pthread_exit(0);
}

void check_request_surprise(ThreadInfo *info) {
  RequestInjectionData &p = info->m_reqInjectionData;
  bool do_timedout, do_memExceeded, do_signaled;

  p.surpriseMutex.lock();

  // Even though we checked surprise outside of the lock, we don't need to
  // check again, because the only code that can ever set surprised to false
  // is right here - and this function is never called from another thread.

  p.surprised = false;

  do_timedout = p.timedout && !p.debugger;
  do_memExceeded = p.memExceeded;
  do_signaled = p.signaled;

  p.timedout = false;
  p.memExceeded = false;
  p.signaled = false;

  p.surpriseMutex.unlock();

  if (do_timedout && !info->m_pendingException) {
    generate_request_timeout_exception();
  }
  if (do_memExceeded && !info->m_pendingException) {
    generate_memory_exceeded_exception();
  }
  if (do_signaled) f_pcntl_signal_dispatch();
}

void throw_pending_exception(ThreadInfo *info) {
  ASSERT(info->m_pendingException);
  info->m_pendingException = false;
  FatalErrorException e(info->m_exceptionMsg, info->m_exceptionStack);
  info->m_exceptionMsg.clear();
  info->m_exceptionStack.reset();
  throw e;
}

void get_call_info_or_fail(const CallInfo *&ci, void *&extra, CStrRef name) {
  if (!get_call_info(ci, extra, name->data(), name->hash())) {
    throw InvalidFunctionCallException(name->data());
  }
}

Variant throw_missing_arguments(const char *fn, int num, int level /* = 0 */) {
  if (level == 2 || RuntimeOption::ThrowMissingArguments) {
    raise_error("Missing argument %d for %s()", num, fn);
  } else {
    raise_warning("Missing argument %d for %s()", num, fn);
  }
  return null;
}

Variant throw_toomany_arguments(const char *fn, int num, int level /* = 0 */) {
  if (level == 2 || RuntimeOption::ThrowTooManyArguments) {
    raise_error("Too many arguments for %s(), expected %d", fn, num);
  } else if (level == 1 || RuntimeOption::WarnTooManyArguments) {
    raise_warning("Too many arguments for %s(), expected %d", fn, num);
  }
  return null;
}

Variant throw_wrong_arguments(const char *fn, int count, int cmin, int cmax,
                              int level /* = 0 */) {
  if (cmin >= 0 && count < cmin) {
    return throw_missing_arguments(fn, count + 1, level);
  }
  if (cmax >= 0 && count > cmax) {
    return throw_toomany_arguments(fn, cmax, level);
  }
  ASSERT(false);
  return null;
}

Variant throw_missing_typed_argument(const char *fn,
                                     const char *type, int arg) {
  if (!type) {
    raise_error("Argument %d passed to %s() must be an array, none given",
                arg, fn);
  } else {
    raise_error("Argument %d passed to %s() must be "
                "an instance of %s, none given", arg, fn, type);
  }
  return null;
}

void throw_bad_type_exception(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string msg;
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  if (RuntimeOption::ThrowBadTypeExceptions) {
    throw InvalidOperandException(msg.c_str());
  }

  raise_warning("Invalid operand type was used: %s", msg.c_str());
}

void throw_bad_array_exception() {
  FrameInjection *fi = FrameInjection::GetStackFrame(0);
  throw_bad_type_exception("%s expects array(s)",
                           fi ? fi->getFunction() : "(unknown)");
}

void throw_invalid_argument(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string msg;
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  if (RuntimeOption::ThrowInvalidArguments) {
    throw InvalidArgumentException(msg.c_str());
  }

  raise_warning("Invalid argument: %s", msg.c_str());
}

Variant throw_fatal_unset_static_property(const char *s, const char *prop) {
  raise_error("Attempt to unset static property %s::$%s", s, prop);
  return null;
}

void check_request_timeout_info(ThreadInfo *info, int lc) {
  check_request_timeout(info);
  if (info->m_pendingException) {
    throw_pending_exception(info);
  }
  if (RuntimeOption::MaxLoopCount > 0 && lc > RuntimeOption::MaxLoopCount) {
    throw FatalErrorException(0, "loop iterated over %d times",
        RuntimeOption::MaxLoopCount);
  }
}

void check_request_timeout_ex(const FrameInjection &fi, int lc) {
  ThreadInfo *info = fi.getThreadInfo();
  check_request_timeout_info(info, lc);
}

void throw_infinite_recursion_exception() {
  if (!RuntimeOption::NoInfiniteRecursionDetection) {
    // Reset profiler otherwise it might recurse further causing segfault
    DECLARE_THREAD_INFO
    info->m_profiler = NULL;
    throw UncatchableException("infinite recursion detected");
  }
}
void generate_request_timeout_exception() {
  ThreadInfo *info = ThreadInfo::s_threadInfo.getNoCheck();
  RequestInjectionData &data = info->m_reqInjectionData;
  if (data.timeoutSeconds > 0) {
    // This extra checking is needed, because there may be a race condition
    // a TimeoutThread sets flag "true" right after an old request finishes and
    // right before a new requets resets "started". In this case, we flag
    // "timedout" back to "false".
    if (time(0) - data.started >= data.timeoutSeconds) {
      info->m_pendingException = true;
      info->m_exceptionMsg = "entire web request took longer than ";
      info->m_exceptionMsg +=
        boost::lexical_cast<std::string>(data.timeoutSeconds);
      info->m_exceptionMsg += " seconds and timed out";
      if (RuntimeOption::InjectedStackTrace) {
        info->m_exceptionStack =
          ArrayPtr(new Array(FrameInjection::GetBacktrace(false, true)));
      }
    }
  }
}

void generate_memory_exceeded_exception() {
  ThreadInfo *info = ThreadInfo::s_threadInfo.getNoCheck();
  info->m_pendingException = true;
  info->m_exceptionMsg = "request has exceeded memory limit";
  if (RuntimeOption::InjectedStackTrace) {
    info->m_exceptionStack =
      ArrayPtr(new Array(FrameInjection::GetBacktrace(false, true)));
  }
}

void throw_call_non_object() {
  throw FatalErrorException("Call to a member function on a non-object");
}

Variant throw_assign_this() {
  throw FatalErrorException("Cannot re-assign $this");
}

void throw_unexpected_argument_type(int argNum, const char *fnName,
                                    const char *expected, CVarRef val) {
  const char *otype = NULL;
  switch (val.getType()) {
  case KindOfUninit:
  case KindOfNull:    otype = "null";        break;
  case KindOfBoolean: otype = "bool";        break;
  case KindOfInt32:
  case KindOfInt64:   otype = "int";         break;
  case KindOfDouble:  otype = "double";      break;
  case KindOfStaticString:
  case KindOfString:  otype = "string";      break;
  case KindOfArray:   otype = "array";       break;
  case KindOfObject:  otype = val.getObjectData()->o_getClassName(); break;
  default:
    ASSERT(false);
  }
  raise_recoverable_error
    ("Argument %d passed to %s must be an instance of %s, %s given",
     argNum, fnName, expected, otype);
}

Object f_clone(CVarRef v) {
  if (v.isObject()) {
    Object clone = Object(v.toObject()->clone());
    clone->t___clone();
    return clone;
  }
  raise_error("Cannot clone non-object");
  return Object();
}

String f_serialize(CVarRef value) {
  switch (value.getType()) {
  case KindOfUninit:
  case KindOfNull:
    return "N;";
  case KindOfBoolean:
    return value.getBoolean() ? "b:1;" : "b:0;";
  case KindOfInt32:
  case KindOfInt64: {
    StringBuffer sb;
    sb.append("i:");
    sb.append(value.getInt64());
    sb.append(';');
    return sb.detach();
  }
  case KindOfStaticString:
  case KindOfString: {
    StringData *str = value.getStringData();
    StringBuffer sb;
    sb.append("s:");
    sb.append(str->size());
    sb.append(":\"");
    sb.append(str->data(), str->size());
    sb.append("\";");
    return sb.detach();
  }
  case KindOfArray: {
    ArrayData *arr = value.getArrayData();
    if (arr->empty()) return "a:0:{}";
    // fall-through
  }
  case KindOfObject:
  case KindOfDouble: {
    VariableSerializer vs(VariableSerializer::Serialize);
    return vs.serialize(value, true);
  }
  default:
    ASSERT(false);
    break;
  }
  return "";
}

Variant unserialize_ex(CStrRef str, VariableUnserializer::Type type) {
  if (str.empty()) {
    return false;
  }

  VariableUnserializer vu(str.data(), str.size(), type);
  Variant v;
  try {
    v = vu.unserialize();
  } catch (Exception &e) {
    raise_notice("Unable to unserialize: [%s]. [%s] %s.", (const char *)str,
                 e.getStackTrace().hexEncode().c_str(),
                 e.getMessage().c_str());
    return false;
  }
  return v;
}

String concat3(CStrRef s1, CStrRef s2, CStrRef s3) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);

  int len1 = s1.size();
  int len2 = s2.size();
  int len3 = s3.size();
  int len = len1 + len2 + len3;
  char *buf = (char *)malloc(len + 1);
  if (buf == NULL) {
    throw FatalErrorException(0, "malloc failed: %d", len);
  }
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  buf[len] = 0;

  return String(buf, len, AttachString);
}

String concat4(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);

  int len1 = s1.size();
  int len2 = s2.size();
  int len3 = s3.size();
  int len4 = s4.size();
  int len = len1 + len2 + len3 + len4;
  char *buf = (char *)malloc(len + 1);
  if (buf == NULL) {
    throw FatalErrorException(0, "malloc failed: %d", len);
  }
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  memcpy(buf + len1 + len2 + len3, s4.data(), len4);
  buf[len] = 0;

  return String(buf, len, AttachString);
}

String concat5(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);

  int len1 = s1.size();
  int len2 = s2.size();
  int len3 = s3.size();
  int len4 = s4.size();
  int len5 = s5.size();
  int len = len1 + len2 + len3 + len4 + len5;
  char *buf = (char *)malloc(len + 1);
  if (buf == NULL) {
    throw FatalErrorException(0, "malloc failed: %d", len);
  }
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  memcpy(buf + len1 + len2 + len3, s4.data(), len4);
  memcpy(buf + len1 + len2 + len3 + len4, s5.data(), len5);
  buf[len] = 0;
  return String(buf, len, AttachString);
}

String concat6(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5,
               CStrRef s6) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);

  int len1 = s1.size();
  int len2 = s2.size();
  int len3 = s3.size();
  int len4 = s4.size();
  int len5 = s5.size();
  int len6 = s6.size();
  int len = len1 + len2 + len3 + len4 + len5 + len6;
  char *buf = (char *)malloc(len + 1);
  if (buf == NULL) {
    throw FatalErrorException(0, "malloc failed: %d", len);
  }
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  memcpy(buf + len1 + len2 + len3, s4.data(), len4);
  memcpy(buf + len1 + len2 + len3 + len4, s5.data(), len5);
  memcpy(buf + len1 + len2 + len3 + len4 + len5, s6.data(), len6);
  buf[len] = 0;
  return String(buf, len, AttachString);
}

bool empty(CVarRef v, bool    offset) {
  return empty(v, Variant(offset));
}
bool empty(CVarRef v, int64   offset) {
  if (!v.isArray()) {
    return empty(v, Variant(offset));
  }
  return !toBoolean(v.toArrNR().rvalAtRef(offset));
}
bool empty(CVarRef v, double  offset) {
  return empty(v, Variant(offset));
}
bool empty(CVarRef v, CArrRef offset) {
  return empty(v, Variant(offset));
}
bool empty(CVarRef v, CObjRef offset) {
  return empty(v, Variant(offset));
}

bool empty(CVarRef v, litstr offset, bool isString /* = false */) {
  if (!v.isArray()) {
    return empty(v, Variant(offset));
  }
  return !toBoolean(v.toArrNR().rvalAtRef(offset,
                                          AccessFlags::IsKey(isString)));
}

bool empty(CVarRef v, CStrRef offset, bool isString /* = false */) {
  if (!v.isArray()) {
    return empty(v, Variant(offset));
  }
  return !toBoolean(v.toArrNR().rvalAtRef(offset,
                                          AccessFlags::IsKey(isString)));
}

bool empty(CVarRef v, CVarRef offset) {
  if (v.isArray()) {
    return !toBoolean(v.toArrNR().rvalAtRef(offset));
  } else if (v.is(KindOfObject)) {
    if (!v.getArrayAccess()->o_invoke(s_offsetExists, Array::Create(offset))) {
      return true;
    }
    // fall through to check for 'empty'ness of the value.
  } else if (v.isString()) {
    int pos = offset.toInt32();
    if (pos < 0 || pos >= v.getStringData()->size()) {
      return true;
    }
  }
  return !toBoolean(v.rvalAt(offset));
}

bool isset(CVarRef v, bool    offset) {
  return isset(v, Variant(offset));
}
bool isset(CVarRef v, int64   offset) {
  if (v.isArray()) {
    return isset(v.toArrNR().rvalAtRef(offset));
  }
  if (v.isObject()) {
    return v.getArrayAccess()->o_invoke(s_offsetExists,
                                        Array::Create(offset), -1);
  }
  if (v.isString()) {
    int pos = (int)offset;
    return pos >= 0 && pos < v.getStringData()->size();
  }
  return false;
}
bool isset(CVarRef v, double  offset) {
  return isset(v, Variant(offset));
}
bool isset(CVarRef v, CArrRef offset) {
  return isset(v, Variant(offset));
}
bool isset(CVarRef v, CObjRef offset) {
  return isset(v, Variant(offset));
}

bool isset(CVarRef v, CVarRef offset) {
  if (v.isArray()) {
    return isset(v.toArrNR().rvalAtRef(offset));
  }
  if (v.isObject()) {
    return v.getArrayAccess()->o_invoke(s_offsetExists,
                                        Array::Create(offset), -1);
  }
  if (v.isString()) {
    int pos = offset.toInt32();
    return pos >= 0 && pos < v.getStringData()->size();
  }
  return false;
}

bool isset(CVarRef v, litstr offset, bool isString /* = false */) {
  if (v.isArray()) {
    return isset(v.toArrNR().rvalAtRef(offset,
                                       AccessFlags::IsKey(isString)));
  }
  if (v.isObject() || v.isString()) {
    return isset(v, Variant(offset));
  }
  return false;
}

bool isset(CVarRef v, CStrRef offset, bool isString /* = false */) {
  if (v.isArray()) {
    return isset(v.toArrNR().rvalAtRef(offset,
                                       AccessFlags::IsKey(isString)));
  }
  if (v.isObject() || v.isString()) {
    return isset(v, Variant(offset));
  }
  return false;
}

String get_source_filename(litstr path, bool dir_component /* = false */) {
  String ret;
  if (path[0] == '/') {
    ret = path;
  } else if (RuntimeOption::SourceRoot.empty()) {
    ret = Process::GetCurrentDirectory() + "/" + path;
  } else {
    ret = RuntimeOption::SourceRoot + path;
  }

  if (dir_component) {
    return f_dirname(ret);
  } else {
    return ret;
  }
}

Variant include_impl_invoke(CStrRef file, bool once, LVariableTable* variables,
                            const char *currentDir) {
  if (file[0] == '/') {
    if (RuntimeOption::SandboxMode || !RuntimeOption::AlwaysUseRelativePath) {
      try {
        return invoke_file(file, once, variables, currentDir);
      } catch(PhpFileDoesNotExistException &e) {}
    }
    string server_root = RuntimeOption::SourceRoot;
    if (server_root.empty()) {
      server_root = string(g_context->getCwd()->data());
      if (server_root.empty() || server_root[server_root.size() - 1] != '/') {
        server_root += "/";
      }
    }

    String rel_path(Util::relativePath(server_root, string(file.data())));

    // Don't try/catch - We want the exception to be passed along
    return invoke_file(rel_path, once, variables, currentDir);
  } else {
    // Don't try/catch - We want the exception to be passed along
    return invoke_file(file, once, variables, currentDir);
  }
}

/**
 * Used by include_impl.  resolve_include() needs some way of checking the
 * existence of a file path, which for hphpc means attempting to invoke it.
 * This struct carries some context information needed for the invocation, as
 * well as a place for the return value of invoking the file.
 */
struct IncludeImplInvokeContext {
  bool once;
  LVariableTable* variables;
  const char* currentDir;

  Variant returnValue;
};

static bool include_impl_invoke_context(CStrRef file, void* ctx) {
  struct IncludeImplInvokeContext* context = (IncludeImplInvokeContext*)ctx;
  bool invoked_file = false;
  try {
    context->returnValue = include_impl_invoke(file, context->once,
                                               context->variables,
                                               context->currentDir);
    invoked_file = true;
  } catch (PhpFileDoesNotExistException& e) {
    context->returnValue = false;
  }
  return invoked_file;
}

/**
 * tryFile is a pointer to a function that resolve_include() will use to
 * determine if a path references a real file.  ctx is a pointer to some context
 * information that will be passed through to tryFile.  (It's a hacky closure)
 */
String resolve_include(CStrRef file, const char* currentDir,
                       bool (*tryFile)(CStrRef file, void*), void* ctx) {
  const char* c_file = file->data();

  if (c_file[0] == '/') {
    String can_path(Util::canonicalize(file.c_str(), file.size()),
                    AttachString);

    if (tryFile(can_path, ctx)) {
      return can_path;
    }

  } else if ((c_file[0] == '.' && (c_file[1] == '/' || (
    c_file[1] == '.' && c_file[2] == '/')))) {

    String path(String(g_context->getCwd() + "/" + file));
    String can_path(Util::canonicalize(path.c_str(), path.size()),
                    AttachString);

    if (tryFile(can_path, ctx)) {
      return can_path;
    }

  } else {

    Array includePaths = g_context->getIncludePathArray();
    unsigned int path_count = includePaths.size();

    for (int i = 0; i < (int)path_count; i++) {
      String path("");
      String includePath = includePaths[i];

      if (includePath[0] != '/') {
        path += (g_context->getCwd() + "/");
      }

      path += includePath;

      if (path[path.size() - 1] != '/') {
        path += "/";
      }

      path += file;
      String can_path(Util::canonicalize(path.c_str(), path.size()),
                      AttachString);

      if (tryFile(can_path, ctx)) {
        return can_path;
      }
    }

    if (currentDir[0] == '/') {
      // We are in hphpi, which passes an absolute path
      String path(currentDir);
      path += "/";
      path += file;
      String can_path(Util::canonicalize(path.c_str(), path.size()),
                      AttachString);

      if (tryFile(can_path, ctx)) {
        return can_path;
      }
    } else {
      // Regular hphp
      String path(g_context->getCwd() + "/" + currentDir + file);
      String can_path(Util::canonicalize(path.c_str(), path.size()),
                      AttachString);

      if (tryFile(can_path, ctx)) {
        return can_path;
      }
    }
  }

  return String((StringData*)NULL);
}

static Variant include_impl(CStrRef file, bool once,
                            LVariableTable* variables,
                            const char *currentDir, bool required,
                            bool raiseNotice) {
  struct IncludeImplInvokeContext ctx = {once, variables, currentDir};
  String can_path = resolve_include(file, currentDir,
                                    include_impl_invoke_context, (void*)&ctx);

  if (can_path.isNull()) {
    // Failure
    if (raiseNotice) {
      raise_notice("Tried to invoke %s but file not found.", file->data());
    }
    if (required) {
      String ms = "Required file that does not exist: ";
      ms += file;
      throw FatalErrorException(ms.data());
    }
    return false;
  }

  return ctx.returnValue;
}

Variant include(CStrRef file, bool once /* = false */,
                LVariableTable* variables /* = NULL */,
                const char *currentDir /* = NULL */,
                bool raiseNotice /*= true*/) {
  return include_impl(file, once, variables, currentDir, false, raiseNotice);
}

Variant require(CStrRef file, bool once /* = false */,
                LVariableTable* variables /* = NULL */,
                const char *currentDir /* = NULL */,
                bool raiseNotice /*= true*/) {
  return include_impl(file, once, variables, currentDir, true, raiseNotice);
}

///////////////////////////////////////////////////////////////////////////////
// class Limits

IMPLEMENT_REQUEST_LOCAL(AutoloadHandler, AutoloadHandler::s_instance);

void AutoloadHandler::requestInit() {
  m_running = false;
  m_handlers.clear();
}

void AutoloadHandler::requestShutdown() {
  m_handlers.reset();
}

void AutoloadHandler::fiberInit(AutoloadHandler *handler,
                                FiberReferenceMap &refMap) {
  m_running = handler->m_running;
  m_handlers = handler->m_handlers.fiberMarshal(refMap);
}

void AutoloadHandler::fiberExit(AutoloadHandler *handler,
                                FiberReferenceMap &refMap,
                                FiberAsyncFunc::Strategy default_strategy) {
  refMap.unmarshal(m_handlers, handler->m_handlers, default_strategy);
}

bool AutoloadHandler::invokeHandler(CStrRef className, bool checkDeclared,
                                    const bool *declared /*= NULL*/,
                                    bool autoloadExists /*= false*/) {
  Array params(ArrayInit(1, true).set(className).create());
  bool l_running = m_running;
  m_running = true;
  if (m_handlers.empty()) {

    if (checkDeclared) {
      autoloadExists = function_exists(s___autoload);
    }
    if (autoloadExists) {
      invoke(s___autoload, params, -1, true, false);
      m_running = l_running;
      return true;
    }
    m_running = l_running;
    return false;
  } else {
    for (ArrayIter iter(m_handlers); iter; ++iter) {
      f_call_user_func_array(iter.second(), params);
      if (checkDeclared) {
        if (f_class_exists(className, false)) {
          break;
        }
      } else if (declared && *declared) {
        break;
      }
    }
    m_running = l_running;
    return true;
  }
}

bool AutoloadHandler::addHandler(CVarRef handler, bool prepend) {
  String name = getSignature(handler);
  if (name.isNull()) return false;
  if (!prepend) {
    // The following ensures that the handler is added at the end
    m_handlers.remove(name, true);
    m_handlers.add(name, handler, true);
  } else {
    // This adds the handler at the beginning
    m_handlers = CREATE_MAP1(name, handler) + m_handlers;
  }
  return true;
}

bool AutoloadHandler::isRunning() {
  return m_running;
}

void AutoloadHandler::removeHandler(CVarRef handler) {
  String name = getSignature(handler);
  if (name.isNull()) return;
  m_handlers.remove(name, true);
}

void AutoloadHandler::removeAllHandlers() {
  m_handlers.clear();
}

String AutoloadHandler::getSignature(CVarRef handler) {
  Variant name;
  if (!f_is_callable(handler, false, ref(name))) {
    return null_string;
  }
  String lName = StringUtil::ToLower(name);
  if (handler.isArray()) {
    Variant first = handler.getArrayData()->get(0LL);
    if (first.isObject()) {
      // Add the object address as part of the signature
      int64 data = (int64)first.getObjectData();
      lName += String((const char *)&data, sizeof(data), CopyString);
    }
  }
  return lName;
}

bool function_exists(CStrRef function_name) {
  String name = get_renamed_function(function_name);
  const ClassInfo::MethodInfo *info = ClassInfo::FindFunction(name);
  if (info) {
    if (info->attribute & ClassInfo::IsSystem) return true;
    if (info->attribute & ClassInfo::IsVolatile) {
      return ((Globals *)get_global_variables())->function_exists(name);
    } else {
      return true;
    }
  } else {
    return false;
  }
}

void checkClassExists(CStrRef name, Globals *g, bool nothrow /* = false */) {
  if (g->class_exists(name)) return;
  AutoloadHandler::s_instance->invokeHandler(name, true);
  if (nothrow) return;
  if (!g->class_exists(name)) {
    string msg = "unknown class ";
    msg += name.c_str();
    throw_fatal(msg.c_str());
  }
}

bool checkClassExists(CStrRef name, const bool *declared, bool autoloadExists,
                      bool nothrow /* = false */) {
  if (declared && *declared) return true;
  AutoloadHandler::s_instance->invokeHandler(name, false, declared,
                                             autoloadExists);
  if (declared && *declared) return true;
  if (nothrow) return false;
  string msg = "unknown class ";
  msg += name.c_str();
  throw_fatal(msg.c_str());
  return false;
}

bool checkInterfaceExists(CStrRef name, const bool *declared,
                          bool autoloadExists, bool nothrow /* = false */) {
  if (*declared) return true;
  AutoloadHandler::s_instance->invokeHandler(name, false, declared,
                                             autoloadExists);
  if (!*declared) {
    if (nothrow) return false;
    string msg = "unknown interface ";
    msg += name.c_str();
    throw_fatal(msg.c_str());
  }
  return true;
}

Variant &get_static_property_lval(const char *s, const char *prop) {
  Variant *ret = get_static_property_lv(s, prop);
  if (ret) return *ret;
  return Variant::lvalBlackHole();
}

#ifdef ENABLE_LATE_STATIC_BINDING
Variant invoke_static_method_bind(CStrRef s, CStrRef method,
                                  CArrRef params, bool fatal /* = true */) {
  ThreadInfo *info = ThreadInfo::s_threadInfo.getNoCheck();
  String cls = s;
  bool isStatic = cls->isame(s_static.get());
  if (isStatic) {
    cls = FrameInjection::GetStaticClassName(info);
  } else {
    FrameInjection::SetStaticClassName(info, cls);
  }
  Variant ret = invoke_static_method(cls, method, params, fatal);
  if (!isStatic) {
    FrameInjection::ResetStaticClassName(info);
  }
  return ref(ret);
}
#endif /* ENABLE_LATE_STATIC_BINDING */

void MethodCallPackage::methodCall(ObjectData *self, CStrRef method,
                                   int64 prehash /* = -1 */) {
  isObj = true;
  rootObj = self;
  name = &method;
  self->o_get_call_info(*this, prehash);
}

void MethodCallPackage::methodCall(CVarRef self, CStrRef method,
                                   int64 prehash /* = -1 */) {
  isObj = true;
  ObjectData *s = self.objectForCall();
  rootObj = s;
  name = &method;
  s->o_get_call_info(*this, prehash);
}

void MethodCallPackage::methodCallWithIndex(ObjectData *self, CStrRef method,
                                            MethodIndex mi,
                                            int64 prehash /* = -1 */) {
  isObj = true;
  rootObj = self;
  name = &method;
  self->o_get_call_info_with_index(*this, mi, prehash);
}

void MethodCallPackage::methodCallWithIndex(CVarRef self, CStrRef method,
                                            MethodIndex mi,
                                            int64 prehash /* = -1 */) {
  isObj = true;
  ObjectData *s = self.objectForCall();
  rootObj = s;
  name = &method;
  s->o_get_call_info_with_index(*this, mi, prehash);
}

void MethodCallPackage::dynamicNamedCall(CVarRef self, CStrRef method,
                                         int64 prehash /* = -1 */) {
  name = &method;
  if (self.is(KindOfObject)) {
    isObj = true;
    rootObj = self.getObjectData();
    rootObj->o_get_call_info(*this, prehash);
  } else {
    String str = self.toString();
    ObjectData *obj = FrameInjection::GetThis();
    if (!obj || !obj->o_instanceof(str)) {
      rootCls = str.get();
      get_call_info_static_method(*this);
    } else {
      isObj = true;
      rootObj = obj;
      obj->o_get_call_info_ex(str->data(), *this, prehash);
    }
  }
}

void MethodCallPackage::dynamicNamedCallWithIndex(CVarRef self, CStrRef method,
    MethodIndex mi, int64 prehash /* = -1 */) {
  name = &method;
  if (self.is(KindOfObject)) {
    isObj = true;
    rootObj = self.getObjectData();
    rootObj->o_get_call_info_with_index(*this, mi, prehash);
  } else {
    String str = self.toString();
    ObjectData *obj = FrameInjection::GetThis();
    if (!obj || !obj->o_instanceof(str)) {
      rootCls = str.get();
      get_call_info_static_method_with_index(*this, mi);
    } else {
      isObj = true;
      rootObj = obj;
      obj->o_get_call_info_with_index_ex(str->data(), *this, mi, prehash);
    }
  }
}

void MethodCallPackage::dynamicNamedCall(CStrRef self, CStrRef method,
    int64 prehash /* = -1 */) {
  rootCls = self.get();
  name = &method;
  ObjectData *obj = FrameInjection::GetThis();
  if (!obj || !obj->o_instanceof(self)) {
    get_call_info_static_method(*this);
  } else {
    isObj = true;
    rootObj = obj;
    obj->o_get_call_info_ex(self, *this, prehash);
  }
}

void MethodCallPackage::dynamicNamedCallWithIndex(CStrRef self, CStrRef method,
    MethodIndex mi, int64 prehash /* = -1 */) {
  rootCls = self.get();
  name = &method;
  ObjectData *obj = FrameInjection::GetThis();
  if (!obj || !obj->o_instanceof(self)) {
    get_call_info_static_method_with_index(*this, mi);
  } else {
    isObj = true;
    rootObj = obj;
    obj->o_get_call_info_with_index_ex(self, *this, mi, prehash);
  }
}

void MethodCallPackage::functionNamedCall(CStrRef func) {
  m_isFunc = true;
  get_call_info(ci, extra, func);
}

void MethodCallPackage::construct(CObjRef self) {
  rootObj = self.get();
  self->getConstructor(*this);
}

void MethodCallPackage::fail() {
  if (m_fatal) {
    o_invoke_failed(isObj ? rootObj->o_getClassName() : String(rootCls),
                    *name, true);
  }
}
String MethodCallPackage::getClassName() {
  if (isObj) {
    return rootObj->o_getClassName();
  } else {
    return rootCls;
  }
}
void MethodCallPackage::lateStaticBind(ThreadInfo *ti) {
#ifdef ENABLE_LATE_STATIC_BINDING
  rootCls = FrameInjection::GetStaticClassName(ti).get();
  get_call_info_static_method(*this);
#else
  m_fatal = true;
  fail();
#endif
}

const CallInfo *MethodCallPackage::bindClass(FrameInjection &fi) {
#ifdef ENABLE_LATE_STATIC_BINDING
  if (ci->m_flags & CallInfo::StaticMethod) {
    fi.setStaticClassName(obj->getRoot()->o_getClassName());
  }
#endif
  return ci;
}

///////////////////////////////////////////////////////////////////////////////
// debugger and code coverage instrumentation

void throw_exception(CObjRef e) {
  if (!Eval::Debugger::InterruptException(e)) return;
  throw e;
}

bool set_line(int line0, int char0 /* = 0 */, int line1 /* = 0 */,
              int char1 /* = 0 */) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  FrameInjection *frame = ti->m_top;
  if (frame) {
    frame->setLine(line0);
    if (RuntimeOption::EnableDebugger && ti->m_reqInjectionData.debugger) {
      Eval::InterruptSite site(frame, Object(), char0, line1, char1);
      Eval::Debugger::InterruptFileLine(site);
      if (site.isJumping()) {
        return false;
      }
    }
    if (RuntimeOption::RecordCodeCoverage) {
      Eval::CodeCoverage::Record(frame->getFileName().data(), line0, line1);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
