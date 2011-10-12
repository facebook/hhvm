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
#include <runtime/base/array/arg_array.h>
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
static StaticString s_exception("exception");
static StaticString s_previous("previous");

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

/* get_user_func_handler takes a Variant 'function', and sets
 * up MethodCallPackage 'mcp' appropriately for the given function.
 * It also sets 'doBind' to indicate whether the caller needs to
 * set the late bound class.
 * Note that classname and methodname are needed even though they
 * dont really pass any information back; the mcp can end up with
 * references to them, which need to survive until the mcp is used
 * (after this function returns).
 */
bool get_user_func_handler(CVarRef function, bool skip,
                           MethodCallPackage &mcp,
                           String &classname, String &methodname,
                           bool &doBind, bool warn /* = true */) {
  doBind = false;
  mcp.noFatal();

  Variant::TypedValueAccessor tv_func = function.getTypedAccessor();
  if (Variant::GetAccessorType(tv_func) == KindOfObject) {
    mcp.functionNamedCall(Variant::GetObjectData(tv_func));
    if (LIKELY(mcp.ci != 0)) return true;
    if (warn) raise_warning("call_user_func to non-callback object");
    return false;
  }

  if (Variant::IsString(tv_func)) {
    StringData *sfunc = Variant::GetStringData(tv_func);
    const char *base = sfunc->data();
    const char *cc = strstr(base, "::");
    if (cc && cc != base && cc[2]) {
      methodname = String(cc + 2, sfunc->size() - (cc - base) - 2, CopyString);
      if (cc - base == 4 && !strncasecmp(base, "self", 4)) {
        if (LIKELY(mcp.dynamicNamedCall(FrameInjection::GetClassName(skip),
                                        methodname))) {
          return true;
        }
      } else if (cc - base == 6 &&
                 !strncasecmp(base, "parent", 6)) {
        CStrRef cls = FrameInjection::GetParentClassName(skip);
        if (cls.empty()) {
          raise_warning("cannot access parent:: when current "
                        "class scope has no parent");
          return false;
        }
        if (LIKELY(mcp.dynamicNamedCall(cls, methodname))) return true;
      } else if (cc - base == 6 &&
                 !strncasecmp(base, "static", 6)) {
        ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
        if (LIKELY(mcp.dynamicNamedCall(FrameInjection::GetStaticClassName(ti),
                                        methodname))) {
          return true;
        }
      } else {
        classname = String(base, cc - base, CopyString);
        if (LIKELY(mcp.dynamicNamedCall(classname, methodname))) {
          doBind = !mcp.isObj || (mcp.ci->m_flags & CallInfo::StaticMethod);
          return true;
        }
      }
      if (warn) {
        raise_warning("call_user_func to non-existent function %s", base);
      }
      return false;
    }
    mcp.functionNamedCall(Variant::GetAsString(tv_func));
    if (LIKELY(mcp.ci != 0)) return true;
    if (warn) raise_warning("call_user_func to non-existent function %s", base);
    return false;
  }

  if (LIKELY(Variant::GetAccessorType(tv_func) == KindOfArray)) {
    CArrRef arr = Variant::GetAsArray(tv_func);
    CVarRef clsname = arr.rvalAtRef(0LL);
    CVarRef mthname = arr.rvalAtRef(1LL);
    if (arr.size() != 2 ||
        &clsname == &null_variant ||
        &mthname == &null_variant) {
      if (warn) throw_invalid_argument("function: not a valid callback array");
      return false;
    }

    Variant::TypedValueAccessor tv_meth = mthname.getTypedAccessor();
    if (!Variant::IsString(tv_meth)) {
      if (warn) throw_invalid_argument("function: methodname not string");
      return false;
    }

    Variant::TypedValueAccessor tv_cls = clsname.getTypedAccessor();

    if (Variant::GetAccessorType(tv_cls) == KindOfObject) {
      ObjectData *obj = Variant::GetObjectData(tv_cls);

      StringData *smeth = Variant::GetStringData(tv_meth);
      const char *base = smeth->data();
      const char *cc = strstr(base, "::");
      if (UNLIKELY(cc && cc != base && cc[2])) {
        methodname = String(cc + 2, smeth->size() - (cc - base) - 2,
                            CopyString);
        mcp.methodCallEx(obj, methodname);

        if (cc - base == 4 && !strncasecmp(base, "self", 4)) {
          classname = obj->o_getClassName();
        } else if (cc - base == 6 &&
                   !strncasecmp(base, "parent", 6)) {
          classname = obj->o_getParentName();
          if (classname.empty()) {
            if (warn) raise_warning("cannot access parent:: when current "
                                    "class scope has no parent");
            return false;
          }
        } else if (cc - base == 6 &&
                   !strncasecmp(base, "static", 6)) {
          ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
          classname = FrameInjection::GetStaticClassName(ti);
        } else {
          doBind = true;
          classname = String(base, cc - base, CopyString);
        }
        if (LIKELY(obj->o_get_call_info_ex(classname, mcp))) {
          if (!(mcp.ci->m_flags & CallInfo::StaticMethod)) {
            doBind = false;
          }
          return true;
        }
        if (warn) {
          if (!obj->o_instanceof(classname)) {
            raise_warning("class '%s' is not a subclass of '%s'",
                          obj->o_getClassName().data(), classname.data());
          } else {
            raise_warning("class '%s' does not have a method '%s'",
                          classname.data(), methodname.data());
          }
        }
        return false;
      }

      methodname = smeth;
      if (LIKELY(mcp.methodCall(obj, methodname))) {
        if (mcp.ci->m_flags & CallInfo::StaticMethod) {
          doBind = true;
          classname = obj->o_getClassName();
        }
        return true;
      }
      return false;
    } else {
      if (UNLIKELY(!Variant::IsString(tv_cls))) {
        if (warn) throw_invalid_argument("function: classname not string");
        return false;
      }
      StringData *sclass = Variant::GetStringData(tv_cls);
      if (sclass->isame(s_self.get())) {
        classname = FrameInjection::GetClassName(skip);
      } else if (sclass->isame(s_parent.get())) {
        classname = FrameInjection::GetParentClassName(skip);
        if (classname.empty()) {
          if (warn) raise_warning("cannot access parent:: when current "
                                  "class scope has no parent");
          return false;
        }
      } else {
        if (sclass->isame(s_static.get())) {
          ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
          classname = FrameInjection::GetStaticClassName(ti);
        } else {
          classname = sclass;
          doBind = true;
        }
      }

      StringData *smeth = Variant::GetStringData(tv_meth);
      methodname = smeth;
      if (LIKELY(mcp.dynamicNamedCall(classname, methodname))) {
        doBind &= !mcp.isObj || (mcp.ci->m_flags & CallInfo::StaticMethod);
        return true;
      }

      const char *base = smeth->data();
      const char *cc = strstr(base, "::");
      if (UNLIKELY(cc && cc != base && cc[2])) {
        methodname = String(cc + 2, smeth->size() - (cc - base) - 2,
                            CopyString);
        if (cc - base == 4 && !strncasecmp(base, "self", 4)) {
          doBind = false;
        } else if (cc - base == 6 &&
                   !strncasecmp(base, "parent", 6)) {
          classname = ObjectData::GetParentName(classname);
          if (classname.empty()) {
            if (warn) raise_warning("cannot access parent:: when current "
                                    "class scope has no parent");
            return false;
          }
          doBind = false;
        } else if (cc - base == 6 &&
                   !strncasecmp(base, "static", 6)) {
          ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
          CStrRef cls = FrameInjection::GetStaticClassName(ti);
          if (UNLIKELY(!classname.get()->isame(cls.get()) &&
                       !f_is_subclass_of(classname, cls))) {
            if (warn) raise_warning("class '%s' is not a subclass of '%s'",
                                    classname.data(), cls.data());
            return false;
          }
          doBind = false;
          classname = cls;
        } else {
          CStrRef cls = String(base, cc - base, CopyString);
          if (UNLIKELY(!classname.get()->isame(cls.get()) &&
                       !f_is_subclass_of(classname, cls))) {
            if (warn) raise_warning("class '%s' is not a subclass of '%s'",
                                    classname.data(), cls.data());
            return false;
          }
          doBind = true;
          classname = cls;
        }
        if (LIKELY(mcp.dynamicNamedCall(classname, methodname))) {
          doBind &= !mcp.isObj || (mcp.ci->m_flags & CallInfo::StaticMethod);
          return true;
        }
      } else {
        // nothing to do. we already checked this case.
      }

      if (warn) raise_warning("call_user_func to non-existent function %s::%s",
                              classname.data(), methodname.data());
      return false;
    }
    if (warn) raise_warning("call_user_func to non-existent function");
    return false;
  }
  if (warn) throw_invalid_argument("function: not string or array");
  return false;
}

bool get_callable_user_func_handler(CVarRef function,
                                    MethodCallPackage &mcp,
                                    String &classname, String &methodname,
                                    bool &doBind) {
  bool ret = get_user_func_handler(function, true, mcp,
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

  return ret;
}

Variant f_call_user_func_array(CVarRef function, CArrRef params,
                               bool bound /* = false */) {
  MethodCallPackage mcp;
  String classname, methodname;
  bool doBind;
  if (UNLIKELY(!get_user_func_handler(function, true, mcp,
                                      classname, methodname, doBind))) {
    return null;
  }

  if (doBind && !bound) {
    FrameInjection::StaticClassNameHelper scn(
      ThreadInfo::s_threadInfo.getNoCheck(), classname);
    ASSERT(!mcp.m_isFunc);
    return mcp.ci->getMeth()(mcp, params);
  } else {
    if (mcp.m_isFunc) {
      return mcp.ci->getFunc()(mcp.extra, params);
    } else {
      return mcp.ci->getMeth()(mcp, params);
    }
  }
}

Variant call_user_func_few_args(CVarRef function, int count, ...) {
  ASSERT(count <= CALL_USER_FUNC_FEW_ARGS_COUNT);
  va_list ap;
  va_start(ap, count);
  CVarRef a0 = (count > 0) ? *va_arg(ap, const Variant *) : null_variant;
  CVarRef a1 = (count > 1) ? *va_arg(ap, const Variant *) : null_variant;
  CVarRef a2 = (count > 2) ? *va_arg(ap, const Variant *) : null_variant;
  CVarRef a3 = (count > 3) ? *va_arg(ap, const Variant *) : null_variant;
  CVarRef a4 = (count > 4) ? *va_arg(ap, const Variant *) : null_variant;
  CVarRef a5 = (count > 5) ? *va_arg(ap, const Variant *) : null_variant;
  va_end(ap);

  MethodCallPackage mcp;
  String classname, methodname;
  bool doBind;
  if (UNLIKELY(!get_user_func_handler(function, false, mcp,
                                      classname, methodname, doBind))) {
    return null;
  }

  if (doBind) {
    FrameInjection::StaticClassNameHelper scn(
      ThreadInfo::s_threadInfo.getNoCheck(), classname);
    ASSERT(!mcp.m_isFunc);
    if (UNLIKELY(!mcp.ci->getMethFewArgs())) {
      return mcp.ci->getMeth()(
        mcp, Array(ArrayInit::CreateParams(count, &a0, &a1, &a2,
                                           &a3, &a4, &a5)));
    }
    return mcp.ci->getMethFewArgs()(mcp, count, a0, a1, a2, a3, a4, a5);
  } else {
    void *extra = mcp.m_isFunc ? mcp.extra : (void*)&mcp;
    if (UNLIKELY(!mcp.ci->getFuncFewArgs())) {
      return mcp.ci->getFunc()(
        extra, Array(ArrayInit::CreateParams(count, &a0, &a1, &a2,
                                             &a3, &a4, &a5)));
    }
    return mcp.ci->getFuncFewArgs()(extra, count, a0, a1, a2, a3, a4, a5);
  }
}

Variant invoke_func_few_handler(void *extra, CArrRef params,
                                Variant (*few_args)(
                                  void *extra, int count,
                                  INVOKE_FEW_ARGS_IMPL_ARGS)) {
  VariantVector<INVOKE_FEW_ARGS_COUNT> args;
  int s = params.size();
  if (LIKELY(s != 0)) {
    int i = s > INVOKE_FEW_ARGS_COUNT ? INVOKE_FEW_ARGS_COUNT : s;
    ArrayData *ad(params.get());
    ssize_t pos = ad->iter_begin();
    do {
      args.pushWithRef(ad->getValueRef(pos));
      pos = ad->iter_advance(pos);
    } while (--i);
  }
  return few_args(extra, s, INVOKE_FEW_ARGS_PASS_ARR_ARGS);
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
  if (LIKELY(get_call_info(ci, extra, function, hash))) {
    return (ci->getFunc())(extra, params);
  }
  return invoke_failed(function, params, fatal);
}

Variant invoke(CVarRef function, CArrRef params,
               bool tryInterp /* = true */, bool fatal /* = true */) {
  const CallInfo *ci;
  void *extra;
  if (LIKELY(get_call_info(ci, extra, function))) {
    return (ci->getFunc())(extra, params);
  }
  return invoke_failed(function, params, fatal);
}

Variant invoke_builtin(const char *s, CArrRef params, int64 hash, bool fatal) {
  const CallInfo *ci;
  void *extra;
  if (LIKELY(get_call_info_builtin(ci, extra, s, hash))) {
    return (ci->getFunc())(extra, params);
  } else {
    return invoke_failed(s, params, fatal);
  }
}

Variant invoke_static_method(CStrRef s, CStrRef method, CArrRef params,
                             bool fatal /* = true */) {
  MethodCallPackage mcp;
  if (!fatal) mcp.noFatal();
  mcp.dynamicNamedCall(s, method, -1);
  if (LIKELY(mcp.ci != NULL)) {
    return (mcp.ci->getMeth())(mcp, params);
  } else {
    o_invoke_failed(s.data(), method.data(), fatal);
    return null;
  }
}

Variant invoke_failed(CVarRef func, CArrRef params,
                      bool fatal /* = true */) {
  if (func.isObject()) {
    return o_invoke_failed(
        func.objectForCall()->o_getClassName().c_str(),
        "__invoke", fatal);
  } else {
    return invoke_failed(func.toString().c_str(), params, fatal);
  }
}

Variant invoke_failed(const char *func, CArrRef params,
                      bool fatal /* = true */) {
  INTERCEPT_INJECTION_ALWAYS("?", func, params, strongBind(r));

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
  if (RuntimeOption::UseArgArray) {
    if (count == 0) return Array::Create();
    ArgArray *args = NEW(ArgArray)(count);
    ArgArray::Argument *argp = args->getStack();
    if (count > 0) {
      argp->m_val.assign(a0);
      argp++;
    }
    if (count > 1) {
      argp->m_val.assign(a1);
      argp++;
    }
    if (count > 2) {
      argp->m_val.assign(a2);
      argp++;
    }
#if INVOKE_FEW_ARGS_COUNT > 3
    if (count > 3) {
      argp->m_val.assign(a3);
      argp++;
    }
    if (count > 4) {
      argp->m_val.assign(a4);
      argp++;
    }
    if (count > 5) {
      argp->m_val.assign(a5);
      argp++;
    }
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
    if (count > 6) {
      argp->m_val.assign(a6);
      argp++;
    }
    if (count > 7) {
      argp->m_val.assign(a7);
      argp++;
    }
    if (count > 8) {
      argp->m_val.assign(a8);
      argp++;
    }
    if (count > 9) {
      argp->m_val.assign(a9);
      argp++;
    }
#endif
    if (count > 10) ASSERT(false);
    return args;
  }
  switch (count) {
  case 0: {
    return Array::Create();
  }
  case 1: {
    return Array(ArrayInit(1).set(a0).create());
  }
  case 2: {
    return Array(ArrayInit(2).set(a0).set(a1).create());
  }
  case 3: {
    return Array(ArrayInit(3).set(a0).set(a1).set(a2).create());
  }
#if INVOKE_FEW_ARGS_COUNT > 3
  case 4: {
    return Array(ArrayInit(4).set(a0).set(a1).set(a2).
                              set(a3).create());
  }
  case 5: {
    return Array(ArrayInit(5).set(a0).set(a1).set(a2).
                              set(a3).set(a4).create());
  }
  case 6: {
    return Array(ArrayInit(6).set(a0).set(a1).set(a2).
                              set(a3).set(a4).set(a5).create());
  }
#endif
#if INVOKE_FEW_ARGS_COUNT > 6
  case 7: {
    return Array(ArrayInit(7).set(a0).set(a1).set(a2).
                              set(a3).set(a4).set(a5).
                                    set(a6).create());
  }
  case 8: {
    return Array(ArrayInit(8).set(a0).set(a1).set(a2).
                              set(a3).set(a4).set(a5).
                              set(a6).set(a7).create());
  }
  case 9: {
    return Array(ArrayInit(9).set(a0).set(a1).set(a2).
                              set(a3).set(a4).set(a5).
                              set(a6).set(a7).set(a8).create());
  }
  case 10: {
    return Array(ArrayInit(10).set(a0).set(a1).set(a2).
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

void throw_instance_method_fatal(const char *name) {
  if (!strstr(name, "::__destruct")) {
    raise_error("Non-static method %s() cannot be called statically", name);
  }
}

Object create_object(CStrRef s, CArrRef params, bool init /* = true */,
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

  ssize_t flags = p.fetchAndClearFlags();
  do_timedout = (flags & RequestInjectionData::TimedOutFlag) && !p.debugger;
  do_memExceeded = (flags & RequestInjectionData::MemExceededFlag);
  do_signaled = (flags & RequestInjectionData::SignaledFlag);

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

bool get_call_info(const CallInfo *&ci, void *&extra, CVarRef func) {
  Variant::TypedValueAccessor tv_func = func.getTypedAccessor();
  if (Variant::GetAccessorType(tv_func) == KindOfObject) {
    ObjectData *d = Variant::GetObjectData(tv_func);
    ci = d->t___invokeCallInfoHelper(extra);
    return ci != NULL;
  }
  if (LIKELY(Variant::IsString(tv_func))) {
    StringData *sd = Variant::GetStringData(tv_func);
    return get_call_info(ci, extra, sd->data(), sd->hash());
  }
  return false;
}

bool get_call_info_no_eval(const CallInfo *&ci, void *&extra, CStrRef name) {
  return get_call_info_no_eval(ci, extra, name->data(), name->hash());
}

void get_call_info_or_fail(const CallInfo *&ci, void *&extra, CVarRef func) {
  if (UNLIKELY(!get_call_info(ci, extra, func))) {
    if (func.isObject()) {
      o_invoke_failed(
        func.objectForCall()->o_getClassName().c_str(),
        "__invoke", true);
    } else {
      throw InvalidFunctionCallException(func.toString().data());
    }
  }
}

void get_call_info_or_fail(const CallInfo *&ci, void *&extra, CStrRef name) {
  if (UNLIKELY(!get_call_info(ci, extra, name->data(), name->hash()))) {
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
  Variant::TypedValueAccessor tva = v.getTypedAccessor();
  if (LIKELY(Variant::GetAccessorType(tva) == KindOfArray)) {
    return empty(Variant::GetArrayData(tva)->get(offset));
  }
  return empty(v, VarNR(offset));
}
bool empty(CVarRef v, double  offset) {
  return empty(v, VarNR(offset));
}
bool empty(CVarRef v, CArrRef offset) {
  return empty(v, VarNR(offset));
}
bool empty(CVarRef v, CObjRef offset) {
  return empty(v, VarNR(offset));
}
bool empty(CVarRef v, litstr offset, bool isString /* = false */) {
  Variant::TypedValueAccessor tva = v.getTypedAccessor();
  if (LIKELY(Variant::GetAccessorType(tva) == KindOfArray)) {
    return empty(Variant::GetAsArray(tva).
                 rvalAtRef(offset, AccessFlags::IsKey(isString)));
  }
  return empty(v, Variant(offset));
}

bool empty(CVarRef v, CStrRef offset, bool isString /* = false */) {
  Variant::TypedValueAccessor tva = v.getTypedAccessor();
  if (LIKELY(Variant::GetAccessorType(tva) == KindOfArray)) {
    return empty(Variant::GetAsArray(tva).
                 rvalAtRef(offset, AccessFlags::IsKey(isString)));
  }
  return empty(v, VarNR(offset));
}

bool empty(CVarRef v, CVarRef offset) {
  Variant::TypedValueAccessor tva = v.getTypedAccessor();
  if (LIKELY(Variant::GetAccessorType(tva) == KindOfArray)) {
    return empty(Variant::GetAsArray(tva).rvalAtRef(offset));
  }
  if (Variant::GetAccessorType(tva) == KindOfObject) {
    if (!Variant::GetArrayAccess(tva)->
        o_invoke(s_offsetExists, Array::Create(offset))) {
      return true;
    }
    // fall through to check for 'empty'ness of the value.
  } else if (Variant::IsString(tva)) {
    uint64 pos = offset.toInt64();
    if (pos >= (uint64)Variant::GetStringData(tva)->size()) {
      return true;
    }
  }
  return empty(v.rvalAt(offset));
}

bool isset(CArrRef v, int64 offset) {
  return isset(v.rvalAtRef(offset));
}
bool isset(CArrRef v, CArrRef offset) {
  return isset(v, VarNR(offset));
}
bool isset(CArrRef v, CObjRef offset) {
  return isset(v, VarNR(offset));
}
bool isset(CArrRef v, CStrRef offset, bool isString /* = false */) {
  return isset(v.rvalAtRef(offset, AccessFlags::IsKey(isString)));
}
bool isset(CArrRef v, litstr offset, bool isString /* = false */) {
  return isset(v.rvalAtRef(offset, AccessFlags::IsKey(isString)));
}
bool isset(CArrRef v, CVarRef offset) {
  return isset(v.rvalAtRef(offset));
}

bool isset(CVarRef v, bool    offset) {
  return isset(v, VarNR(offset));
}
bool isset(CVarRef v, int64   offset) {
  Variant::TypedValueAccessor tva = v.getTypedAccessor();
  if (LIKELY(Variant::GetAccessorType(tva) == KindOfArray)) {
    return isset(Variant::GetArrayData(tva)->get(offset));
  }
  if (Variant::GetAccessorType(tva) == KindOfObject) {
    return Variant::GetArrayAccess(tva)->
      o_invoke(s_offsetExists, Array::Create(offset), -1);
  }
  if (Variant::IsString(tva)) {
    return (uint64)offset < (uint64)Variant::GetStringData(tva)->size();
  }
  return false;
}
bool isset(CVarRef v, double  offset) {
  return isset(v, VarNR(offset));
}
bool isset(CVarRef v, CArrRef offset) {
  return isset(v, VarNR(offset));
}
bool isset(CVarRef v, CObjRef offset) {
  return isset(v, VarNR(offset));
}
bool isset(CVarRef v, CVarRef offset) {
  Variant::TypedValueAccessor tva = v.getTypedAccessor();
  if (LIKELY(Variant::GetAccessorType(tva) == KindOfArray)) {
    return isset(Variant::GetAsArray(tva).rvalAtRef(offset));
  }
  if (Variant::GetAccessorType(tva) == KindOfObject) {
    return Variant::GetArrayAccess(tva)->
      o_invoke(s_offsetExists, Array::Create(offset), -1);
  }
  if (Variant::IsString(tva)) {
    uint64 pos = offset.toInt64();
    return pos < (uint64)Variant::GetStringData(tva)->size();
  }
  return false;
}
bool isset(CVarRef v, litstr offset, bool isString /* = false */) {
  Variant::TypedValueAccessor tva = v.getTypedAccessor();
  if (LIKELY(Variant::GetAccessorType(tva) == KindOfArray)) {
    return isset(Variant::GetAsArray(tva).rvalAtRef(
                   offset, AccessFlags::IsKey(isString)));
  }
  if (Variant::GetAccessorType(tva) == KindOfObject ||
      Variant::IsString(tva)) {
    return isset(v, Variant(offset));
  }
  return false;
}

bool isset(CVarRef v, CStrRef offset, bool isString /* = false */) {
  Variant::TypedValueAccessor tva = v.getTypedAccessor();
  if (LIKELY(Variant::GetAccessorType(tva) == KindOfArray)) {
    return isset(Variant::GetAsArray(tva).rvalAtRef(
                   offset, AccessFlags::IsKey(isString)));
  }
  if (Variant::GetAccessorType(tva) == KindOfObject ||
      Variant::IsString(tva)) {
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
  m_handlers.reset();
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

/**
 * invokeHandler returns true if any autoload handlers were executed,
 * false otherwise. When this function returns true, it is the caller's
 * responsibility to check if the given class or interface exists.
 */
bool AutoloadHandler::invokeHandler(CStrRef className,
                                    const bool *declared /* = NULL */,
                                    bool forceSplStack /* = false */) {
  Array params(ArrayInit(1).set(className).create());
  bool l_running = m_running;
  m_running = true;
  if (m_handlers.isNull() && !forceSplStack) {
    if (function_exists(s___autoload)) {
      invoke(s___autoload, params, -1, true, false);
      m_running = l_running;
      return true;
    }
    m_running = l_running;
    return false;
  }
  if (empty(m_handlers)) {
    m_running = l_running;
    return false;
  }
  Object autoloadException;
  for (ArrayIter iter(m_handlers); iter; ++iter) {
    try {
      f_call_user_func_array(iter.second(), params);
    } catch (Object& ex) {
      ASSERT(ex.instanceof(s_exception));
      if (autoloadException.isNull()) {
        autoloadException = ex;
      } else {
        Object cur = ex;
        Variant next = cur->o_get(s_previous, false, s_exception);
        while (next.isObject()) {
          cur = next.toObject();
          next = cur->o_get(s_previous, false, s_exception);
        }
        cur->o_set(s_previous, autoloadException, false, s_exception);
        autoloadException = ex;
      }
    }
    // TODO: f_class_exists() does not check interfaces. We need to
    // fix this to check for both classes and interfaces.
    if (declared ? *declared : f_class_exists(className, false)) {
      break;
    }
  }
  m_running = l_running;
  if (!autoloadException.isNull()) {
    throw autoloadException;
  }
  return true;
}

bool AutoloadHandler::addHandler(CVarRef handler, bool prepend) {
  String name = getSignature(handler);
  if (name.isNull()) return false;

  if (m_handlers.isNull()) {
    m_handlers = Array::Create();
  }

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
  m_handlers.reset();
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
  return ClassInfo::FindFunction(name);
}

void checkClassExists(CStrRef name, Globals *g, bool nothrow /* = false */) {
  if (g->class_exists(name)) return;
  AutoloadHandler::s_instance->invokeHandler(name);
  if (nothrow) return;
  if (!g->class_exists(name)) {
    string msg = "unknown class ";
    msg += name.c_str();
    throw_fatal(msg.c_str());
  }
}

bool autoloadClassThrow(CStrRef name, bool *declared) {
  if (autoloadClassNoThrow(name, declared)) return true;
  string msg = "unknown class ";
  msg += name.c_str();
  throw_fatal(msg.c_str());
  return false;
}

bool autoloadClassNoThrow(CStrRef name, bool *declared) {
  AutoloadHandler::s_instance->invokeHandler(name, declared);
  return declared && *declared;
}

bool autoloadInterfaceThrow(CStrRef name, bool *declared) {
  if (autoloadInterfaceNoThrow(name, declared)) return true;
  string msg = "unknown interface ";
  msg += name.c_str();
  throw_fatal(msg.c_str());
  return false;
}

bool autoloadInterfaceNoThrow(CStrRef name, bool *declared) {
  AutoloadHandler::s_instance->invokeHandler(name, declared);
  return declared && *declared;
}

Variant &get_static_property_lval(CStrRef s, const char *prop) {
  Variant *ret = get_static_property_lv(s, prop);
  if (ret) return *ret;
  return Variant::lvalBlackHole();
}

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
  return strongBind(ret);
}

bool MethodCallPackage::methodCall(ObjectData *self, CStrRef method,
                                   int64 prehash /* = -1 */) {
  isObj = true;
  rootObj = self;
  name = &method;
  return self->o_get_call_info(*this, prehash);
}

HOT_FUNC
bool MethodCallPackage::methodCall(CVarRef self, CStrRef method,
                                   int64 prehash /* = -1 */) {
  isObj = true;
  ObjectData *s = self.objectForCall();
  rootObj = s;
  name = &method;
  return s->o_get_call_info(*this, prehash);
}

bool MethodCallPackage::dynamicNamedCall(CVarRef self, CStrRef method,
                                         int64 prehash /* = -1 */) {
  name = &method;
  if (self.is(KindOfObject)) {
    isObj = true;
    rootObj = self.getObjectData();
    return rootObj->o_get_call_info(*this, prehash);
  } else {
    String str = self.toString();
    ObjectData *obj = FrameInjection::GetThis();
    if (!obj || !obj->o_instanceof(str)) {
      rootCls = str.get();
      return get_call_info_static_method(*this);
    } else {
      isObj = true;
      rootObj = obj;
      return obj->o_get_call_info_ex(str->data(), *this, prehash);
    }
  }
}

bool MethodCallPackage::dynamicNamedCall(CStrRef self, CStrRef method,
    int64 prehash /* = -1 */) {
  rootCls = self.get();
  name = &method;
  ObjectData *obj = FrameInjection::GetThis();
  if (!obj || !obj->o_instanceof(self)) {
    return get_call_info_static_method(*this);
  } else {
    isObj = true;
    rootObj = obj;
    return obj->o_get_call_info_ex(self, *this, prehash);
  }
}

void MethodCallPackage::functionNamedCall(CVarRef func) {
  m_isFunc = true;
  get_call_info(ci, extra, func);
}

void MethodCallPackage::functionNamedCall(CStrRef func) {
  m_isFunc = true;
  get_call_info(ci, extra, func.data());
}

void MethodCallPackage::functionNamedCall(ObjectData *func) {
  m_isFunc = true;
  ci = func->t___invokeCallInfoHelper(extra);
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
  rootCls = FrameInjection::GetStaticClassName(ti).get();
  get_call_info_static_method(*this);
}

HOT_FUNC
const CallInfo *MethodCallPackage::bindClass(FrameInjection &fi) {
  if (ci->m_flags & CallInfo::StaticMethod) {
    fi.setStaticClassName(obj->getRoot()->o_getClassName());
  }
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
