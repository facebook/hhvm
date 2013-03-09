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
#include <runtime/base/code_coverage.h>
#include <runtime/base/externals.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/variable_unserializer.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/strings.h>
#include <runtime/eval/runtime/file_repository.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/ext/ext_process.h>
#include <runtime/ext/ext_class.h>
#include <runtime/ext/ext_function.h>
#include <runtime/ext/ext_file.h>
#include <runtime/ext/ext_collection.h>
#include <util/logger.h>
#include <util/util.h>
#include <util/process.h>
#include <runtime/vm/repo.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/vm/unit.h>
#include <runtime/vm/event_hook.h>
#include <system/lib/systemlib.h>

#include <limits>

using namespace HPHP::MethodLookup;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// static strings

static StaticString s_offsetExists("offsetExists");
static StaticString s___autoload("__autoload");
static StaticString s___call("__call");
static StaticString s___callStatic("__callStatic");
static StaticString s_exception("exception");
static StaticString s_previous("previous");

StaticString s_self("self");
StaticString s_parent("parent");
StaticString s_static("static");
StaticString s_class("class");
StaticString s_function("function");
StaticString s_constant("constant");
StaticString s_failure("failure");

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
  if (AutoloadHandler::s_instance->autoloadConstant(name) &&
      isInitialized(v)) {
    return v;
  }
  raise_notice(Strings::UNDEFINED_CONSTANT,
               name.c_str(), name.c_str());
  return name;
}

String getUndefinedConstant(CStrRef name) {
  raise_notice(Strings::UNDEFINED_CONSTANT,
               name.c_str(), name.c_str());
  return name;
}

bool array_is_valid_callback(CArrRef arr) {
  if (arr.size() != 2 || !arr.exists(int64_t(0)) || !arr.exists(int64_t(1))) {
    return false;
  }
  Variant elem0 = arr.rvalAt(int64_t(0));
  if (!elem0.isString() && !elem0.isObject()) {
    return false;
  }
  Variant elem1 = arr.rvalAt(int64_t(1));
  if (!elem1.isString()) {
    return false;
  }
  return true;
}

const HPHP::VM::Func*
vm_decode_function(CVarRef function,
                   HPHP::VM::ActRec* ar,
                   bool forwarding,
                   ObjectData*& this_,
                   HPHP::VM::Class*& cls,
                   StringData*& invName,
                   bool warn /* = true */) {
  invName = nullptr;
  if (function.isString() || function.isArray()) {
    HPHP::VM::Class* ctx = nullptr;
    if (ar) ctx = arGetContextClass(ar);
    // Decode the 'function' parameter into this_, cls, name, pos, and
    // nameContainsClass.
    this_ = nullptr;
    cls = nullptr;
    String name;
    int pos = String::npos;
    bool nameContainsClass = false;
    if (function.isString()) {
      // If 'function' is a string we simply assign it to name and
      // leave this_ and cls set to NULL.
      name = function.toString();
      pos = name.find("::");
      nameContainsClass =
        (pos != 0 && pos != String::npos && pos + 2 < name.size());
    } else {
      // If 'function' is an array with exactly two indices 0 and 1, we
      // assign the value at index 1 to name and we use the value at index
      // 0 to populate cls (if the value is a string) or this_ and cls (if
      // the value is an object).
      assert(function.isArray());
      Array arr = function.toArray();
      if (!array_is_valid_callback(arr)) {
        if (warn) {
          throw_invalid_argument("function: not a valid callback array");
        }
        return nullptr;
      }
      Variant elem1 = arr.rvalAt(int64_t(1));
      name = elem1.toString();
      pos = name.find("::");
      nameContainsClass =
        (pos != 0 && pos != String::npos && pos + 2 < name.size());
      Variant elem0 = arr.rvalAt(int64_t(0));
      if (elem0.isString()) {
        String sclass = elem0.toString();
        if (sclass->isame(s_self.get())) {
          if (ctx) {
            cls = ctx;
          }
          if (!nameContainsClass) {
            forwarding = true;
          }
        } else if (sclass->isame(s_parent.get())) {
          if (ctx && ctx->parent()) {
            cls = ctx->parent();
          }
          if (!nameContainsClass) {
            forwarding = true;
          }
        } else if (sclass->isame(s_static.get())) {
          if (ar) {
            if (ar->hasThis()) {
              cls = ar->getThis()->getVMClass();
            } else if (ar->hasClass()) {
              cls = ar->getClass();
            }
          }
        } else {
          if (warn && nameContainsClass) {
            String nameClass = name.substr(0, pos);
            if (nameClass->isame(s_self.get())   ||
                nameClass->isame(s_parent.get()) ||
                nameClass->isame(s_static.get())) {
              raise_warning("behavior of call_user_func(array('%s', '%s')) "
                            "is undefined", sclass->data(), name->data());
            }
          }
          cls = VM::Unit::loadClass(sclass.get());
        }
        if (!cls) {
          if (warn) {
            throw_invalid_argument("function: class not found");
          }
          return nullptr;
        }
      } else {
        assert(elem0.isObject());
        this_ = elem0.getObjectData();
        cls = this_->getVMClass();
      }
    }

    HPHP::VM::Class* cc = cls;
    if (nameContainsClass) {
      String c = name.substr(0, pos);
      name = name.substr(pos + 2);
      if (c->isame(s_self.get())) {
        if (cls) {
          cc = cls;
        } else if (ctx) {
          cc = ctx;
        }
        if (!this_) {
          forwarding = true;
        }
      } else if (c->isame(s_parent.get())) {
        if (cls) {
          cc = cls->parent();
        } else if (ctx && ctx->parent()) {
          cc = ctx->parent();
        }
        if (!this_) {
          forwarding = true;
        }
      } else if (c->isame(s_static.get())) {
        if (ar) {
          if (ar->hasThis()) {
            cc = ar->getThis()->getVMClass();
          } else if (ar->hasClass()) {
            cc = ar->getClass();
          }
        }
      } else {
        cc = VM::Unit::loadClass(c.get());
      }
      if (!cc) {
        if (warn) {
          throw_invalid_argument("function: class not found");
        }
        return nullptr;
      }
      if (cls) {
        if (!cls->classof(cc)) {
          if (warn) {
            raise_warning("call_user_func expects parameter 1 to be a valid "
                          "callback, class '%s' is not a subclass of '%s'",
                          cls->preClass()->name()->data(),
                          cc->preClass()->name()->data());
          }
          return nullptr;
        }
      }
      // If there is not a current instance, cc trumps cls.
      if (!this_) {
        cls = cc;
      }
    }
    if (!cls) {
      HPHP::VM::Func* f = HPHP::VM::Unit::loadFunc(name.get());
      if (!f) {
        if (warn) {
          throw_invalid_argument("function: method '%s' not found",
                                 name->data());
        }
        return nullptr;
      }
      assert(f && f->preClass() == nullptr);
      return f;
    }
    assert(cls);
    CallType lookupType = this_ ? ObjMethod : ClsMethod;
    const HPHP::VM::Func* f =
      g_vmContext->lookupMethodCtx(cc, name.get(), ctx, lookupType);
    if (f && (f->attrs() & HPHP::VM::AttrStatic)) {
      // If we found a method and its static, null out this_
      this_ = nullptr;
    } else {
      if (!this_ && ar) {
        // If we did not find a static method AND this_ is null AND there is a
        // frame ar, check if the current instance from ar is compatible
        ObjectData* obj = ar->hasThis() ? ar->getThis() : nullptr;
        if (obj && obj->instanceof(cls)) {
          this_ = obj;
          cls = obj->getVMClass();
        }
      }
      if (!f) {
        if (this_) {
          // If this_ is non-null AND we could not find a method, try
          // looking up __call in cls's method table
          f = cls->lookupMethod(s___call.get());
          assert(!f || !(f->attrs() & HPHP::VM::AttrStatic));
        }
        if (!f && lookupType == ClsMethod) {
          f = cls->lookupMethod(s___callStatic.get());
          assert(!f || (f->attrs() & HPHP::VM::AttrStatic));
          this_ = nullptr;
        }
        if (f) {
          // We found __call or __callStatic!
          // Stash the original name into invName.
          invName = name.get();
          invName->incRefCount();
        } else {
          // Bail out if we couldn't find the method or __call
          if (warn) {
            throw_invalid_argument("function: method '%s' not found",
                                   name->data());
          }
          return nullptr;
        }
      }
    }
    assert(f && f->preClass());
    // If this_ is non-NULL, then this_ is the current instance and cls is
    // the class of the current instance.
    assert(!this_ || this_->getVMClass() == cls);
    // If we are doing a forwarding call and this_ is null, set cls
    // appropriately to propagate the current late bound class.
    if (!this_ && forwarding && ar) {
      HPHP::VM::Class* fwdCls = nullptr;
      ObjectData* obj = ar->hasThis() ? ar->getThis() : nullptr;
      if (obj) {
        fwdCls = obj->getVMClass();
      } else if (ar->hasClass()) {
        fwdCls = ar->getClass();
      }
      // Only forward the current late bound class if it is the same or
      // a descendent of cls
      if (fwdCls && fwdCls->classof(cls->preClass())) {
        cls = fwdCls;
      }
    }
    return f;
  }
  if (function.isObject()) {
    static StringData* invokeStr = StringData::GetStaticString("__invoke");
    this_ = function.asCObjRef().get();
    cls = nullptr;
    const HPHP::VM::Func *f = this_->getVMClass()->lookupMethod(invokeStr);
    if (f != nullptr && (f->attrs() & HPHP::VM::AttrStatic)) {
      // If __invoke is static, invoke it as such
      cls = this_->getVMClass();
      this_ = nullptr;
    }
    return f;
  }
  if (warn) {
    throw_invalid_argument("function: not string, closure, or array");
  }
  return nullptr;
}

Variant vm_call_user_func(CVarRef function, CArrRef params,
                          bool forwarding /* = false */) {
  ObjectData* obj = nullptr;
  HPHP::VM::Class* cls = nullptr;
  HPHP::VM::Transl::CallerFrame cf;
  StringData* invName = nullptr;
  const HPHP::VM::Func* f = vm_decode_function(function, cf(), forwarding,
                                               obj, cls, invName);
  if (f == nullptr) {
    return null;
  }
  Variant ret;
  g_vmContext->invokeFunc((TypedValue*)&ret, f, params, obj, cls,
                          nullptr, invName);
  return ret;
}

Variant f_call_user_func_array(CVarRef function, CArrRef params,
                               bool bound /* = false */) {
  return vm_call_user_func(function, params, bound);
}

Variant invoke(CStrRef function, CArrRef params, strhash_t hash /* = -1 */,
               bool tryInterp /* = true */, bool fatal /* = true */) {
  VM::Func* func = VM::Unit::loadFunc(function.get());
  if (func) {
    Variant ret;
    g_vmContext->invokeFunc(ret.asTypedValue(), func, params);
    return ret;
  }
  return invoke_failed(function.c_str(), params, fatal);
}

Variant invoke(const char *function, CArrRef params, strhash_t hash /* = -1*/,
               bool tryInterp /* = true */, bool fatal /* = true */) {
  String funcName(function, CopyString);
  return invoke(funcName, params, hash, tryInterp, fatal);
}

Variant invoke_static_method(CStrRef s, CStrRef method, CArrRef params,
                             bool fatal /* = true */) {
  HPHP::VM::Class* class_ = VM::Unit::lookupClass(s.get());
  if (class_ == nullptr) {
    o_invoke_failed(s.data(), method.data(), fatal);
    return null;
  }
  const HPHP::VM::Func* f = class_->lookupMethod(method.get());
  if (f == nullptr || !(f->attrs() & HPHP::VM::AttrStatic)) {
    o_invoke_failed(s.data(), method.data(), fatal);
    return null;
  }
  Variant ret;
  g_vmContext->invokeFunc((TypedValue*)&ret, f, params, nullptr, class_);
  return ret;
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

void NEVER_INLINE raise_null_object_prop() {
  raise_notice("Trying to get property of non-object");
}

void NEVER_INLINE throw_null_object_prop() {
  raise_error("Trying to set property of non-object");
}

void NEVER_INLINE throw_invalid_property_name(CStrRef name) {
  if (!name.size()) {
    throw EmptyObjectPropertyException();
  } else {
    throw NullStartObjectPropertyException();
  }
}

void throw_instance_method_fatal(const char *name) {
  if (!strstr(name, "::__destruct")) {
    raise_error("Non-static method %s() cannot be called statically", name);
  }
}

void throw_iterator_not_valid() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
    "Iterator is not valid"));
  throw e;
}

void throw_collection_modified() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
    "Collection was modified during iteration"));
  throw e;
}

void throw_collection_property_exception() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
    "Cannot access property on a collection"));
  throw e;
}

void throw_collection_compare_exception() {
  static const string msg(
    "Cannot use relational comparison operators (<, <=, >, >=) to compare "
    "a collection with an integer, double, string, array, or object");
  if (RuntimeOption::StrictCollections) {
    Object e(SystemLib::AllocRuntimeExceptionObject(msg));
    throw e;
  }
  raise_warning(msg);
}

void check_collection_compare(ObjectData* obj) {
  if (obj && obj->isCollection()) throw_collection_compare_exception();
}

void check_collection_compare(ObjectData* obj1, ObjectData* obj2) {
  if (obj1 && obj2 && (obj1->isCollection() || obj2->isCollection())) {
    throw_collection_compare_exception();
  }
}

void check_collection_cast_to_array() {
  if (RuntimeOption::WarnOnCollectionToArray) {
    raise_warning("Casting a collection to an array is an expensive operation "
                  "and should be avoided where possible. To convert a "
                  "collection to an array without raising a warning, use the "
                  "toArray() method.");
  }
}

Object create_object_only(CStrRef s) {
  return g_vmContext->createObjectOnly(s.get());
}

Object create_object(CStrRef s, CArrRef params, bool init /* = true */) {
  return g_vmContext->createObject(s.get(), params, init);
}

/*
 * This function is used when another thread is segfaulting---we just
 * want to wait forever to give it a chance to write a stacktrace file
 * (and maybe a core file).
 */
void pause_forever() {
  for (;;) sleep(300);
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
  assert(info->m_pendingException);
  info->m_pendingException = false;
  FatalErrorException e(info->m_exceptionMsg, info->m_exceptionStack.get());
  info->m_exceptionMsg.clear();
  info->m_exceptionStack.reset();
  throw e;
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
  assert(false);
  return null;
}

void throw_missing_arguments_nr(const char *fn, int num, int level /* = 0 */) {
  if (level == 2 || RuntimeOption::ThrowMissingArguments) {
    raise_error("Missing argument %d for %s()", num, fn);
  } else {
    raise_warning("Missing argument %d for %s()", num, fn);
  }
}

void throw_toomany_arguments_nr(const char *fn, int num, int level /* = 0 */) {
  if (level == 2 || RuntimeOption::ThrowTooManyArguments) {
    raise_error("Too many arguments for %s(), expected %d", fn, num);
  } else if (level == 1 || RuntimeOption::WarnTooManyArguments) {
    raise_warning("Too many arguments for %s(), expected %d", fn, num);
  }
}

void throw_wrong_arguments_nr(const char *fn, int count, int cmin, int cmax,
                           int level /* = 0 */) {
  if (cmin >= 0 && count < cmin) {
    throw_missing_arguments(fn, count + 1, level);
    return;
  }
  if (cmax >= 0 && count > cmax) {
    throw_toomany_arguments(fn, cmax, level);
    return;
  }
  assert(false);
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
  const char* fn = "(unknown)";
  HPHP::VM::ActRec *ar = g_vmContext->getStackFrame();
  if (ar) {
    fn = ar->m_func->name()->data();
  }
  throw_bad_type_exception("%s expects array(s)", fn);
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

void check_request_timeout_ex(int lc) {
  ThreadInfo *info = ThreadInfo::s_threadInfo.getNoCheck();
  check_request_timeout_info(info, lc);
}

void throw_infinite_recursion_exception() {
  if (!RuntimeOption::NoInfiniteRecursionDetection) {
    // Reset profiler otherwise it might recurse further causing segfault
    DECLARE_THREAD_INFO
    info->m_profiler = nullptr;
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
          g_vmContext->debugBacktrace(false, true, true).get();
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
      g_vmContext->debugBacktrace(false, true, true).get();
  }
}

void throw_call_non_object() {
  throw_call_non_object(nullptr);
}

void throw_call_non_object(const char *methodName) {
  std::string msg;

  if (methodName == nullptr) {
    msg = "Call to a member function on a non-object";
  } else {
    Util::string_printf(msg,
                        "Call to a member function %s() on a non-object",
                        methodName);
  }

  if (RuntimeOption::ThrowExceptionOnBadMethodCall) {
    Object e(SystemLib::AllocBadMethodCallExceptionObject(String(msg)));
    throw e;
  }

  throw FatalErrorException(msg.c_str());
}

Variant throw_assign_this() {
  throw FatalErrorException("Cannot re-assign $this");
}

void throw_unexpected_argument_type(int argNum, const char *fnName,
                                    const char *expected, CVarRef val) {
  const char *otype = nullptr;
  switch (val.getType()) {
  case KindOfUninit:
  case KindOfNull:    otype = "null";        break;
  case KindOfBoolean: otype = "bool";        break;
  case KindOfInt64:   otype = "int";         break;
  case KindOfDouble:  otype = "double";      break;
  case KindOfStaticString:
  case KindOfString:  otype = "string";      break;
  case KindOfArray:   otype = "array";       break;
  case KindOfObject:  otype = val.getObjectData()->o_getClassName(); break;
  default:
    assert(false);
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
    assert(false);
    break;
  }
  return "";
}

Variant unserialize_ex(const char* str, int len,
                       VariableUnserializer::Type type) {
  if (str == nullptr || len <= 0) {
    return false;
  }

  VariableUnserializer vu(str, len, type);
  Variant v;
  try {
    v = vu.unserialize();
  } catch (Exception &e) {
    raise_notice("Unable to unserialize: [%s]. %s.", str,
                 e.getMessage().c_str());
    return false;
  }
  return v;
}

Variant unserialize_ex(CStrRef str, VariableUnserializer::Type type) {
  return unserialize_ex(str.data(), str.size(), type);
}

String concat3(CStrRef s1, CStrRef s2, CStrRef s3) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  StringSlice r1 = s1.slice();
  StringSlice r2 = s2.slice();
  StringSlice r3 = s3.slice();
  int len = r1.len + r2.len + r3.len;
  StringData* str = NEW(StringData)(len);
  MutableSlice r = str->mutableSlice();
  memcpy(r.ptr,                   r1.ptr, r1.len);
  memcpy(r.ptr + r1.len,          r2.ptr, r2.len);
  memcpy(r.ptr + r1.len + r2.len, r3.ptr, r3.len);
  str->setSize(len);
  return str;
}

String concat4(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  StringSlice r1 = s1.slice();
  StringSlice r2 = s2.slice();
  StringSlice r3 = s3.slice();
  StringSlice r4 = s4.slice();
  int len = r1.len + r2.len + r3.len + r4.len;
  StringData* str = NEW(StringData)(len);
  MutableSlice r = str->mutableSlice();
  memcpy(r.ptr,                            r1.ptr, r1.len);
  memcpy(r.ptr + r1.len,                   r2.ptr, r2.len);
  memcpy(r.ptr + r1.len + r2.len,          r3.ptr, r3.len);
  memcpy(r.ptr + r1.len + r2.len + r3.len, r4.ptr, r4.len);
  str->setSize(len);
  return str;
}

String concat5(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);

  int len1 = s1.size();
  int len2 = s2.size();
  int len3 = s3.size();
  int len4 = s4.size();
  int len5 = s5.size();
  int len = len1 + len2 + len3 + len4 + len5;
  String s = String(len, ReserveString);
  char *buf = s.mutableSlice().ptr;
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  memcpy(buf + len1 + len2 + len3, s4.data(), len4);
  memcpy(buf + len1 + len2 + len3 + len4, s5.data(), len5);
  return s.setSize(len);
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
  String s = String(len, ReserveString);
  char *buf = s.mutableSlice().ptr;
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  memcpy(buf + len1 + len2 + len3, s4.data(), len4);
  memcpy(buf + len1 + len2 + len3 + len4, s5.data(), len5);
  memcpy(buf + len1 + len2 + len3 + len4 + len5, s6.data(), len6);
  return s.setSize(len);
}

bool empty(CVarRef v, bool    offset) {
  return empty(v, Variant(offset));
}
bool empty(CVarRef v, int64_t   offset) {
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
    ObjectData* obj = Variant::GetObjectData(tva);
    if (obj->isCollection()) {
      return collectionOffsetEmpty(obj, offset);
    } else {
      if (!Variant::GetArrayAccess(tva)->
          o_invoke(s_offsetExists, Array::Create(offset))) {
        return true;
      }
      return empty(v.rvalAt(offset));
    }
  } else if (Variant::IsString(tva)) {
    uint64_t pos = offset.toInt64();
    if (pos >= (uint64_t)Variant::GetStringData(tva)->size()) {
      return true;
    }
  }
  return empty(v.rvalAt(offset));
}

bool isset(CArrRef v, int64_t offset) {
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
bool isset(CVarRef v, int64_t   offset) {
  Variant::TypedValueAccessor tva = v.getTypedAccessor();
  if (LIKELY(Variant::GetAccessorType(tva) == KindOfArray)) {
    return isset(Variant::GetArrayData(tva)->get(offset));
  }
  if (Variant::GetAccessorType(tva) == KindOfObject) {
    ObjectData* obj = Variant::GetObjectData(tva);
    if (obj->isCollection()) {
      return collectionOffsetIsset(obj, offset);
    } else {
      return Variant::GetArrayAccess(tva)->
        o_invoke(s_offsetExists, Array::Create(offset), -1);
    }
  }
  if (Variant::IsString(tva)) {
    return (uint64_t)offset < (uint64_t)Variant::GetStringData(tva)->size();
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
    ObjectData* obj = Variant::GetObjectData(tva);
    if (obj->isCollection()) {
      return collectionOffsetIsset(obj, offset);
    } else {
      return Variant::GetArrayAccess(tva)->
        o_invoke(s_offsetExists, Array::Create(offset), -1);
    }
  }
  if (Variant::IsString(tva)) {
    uint64_t pos = offset.toInt64();
    return pos < (uint64_t)Variant::GetStringData(tva)->size();
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
  } else {
    ret = RuntimeOption::SourceRoot + path;
  }

  if (dir_component) {
    return f_dirname(ret);
  } else {
    return ret;
  }
}

Variant include_impl_invoke(CStrRef file, bool once, const char *currentDir) {
  if (file[0] == '/') {
    if (RuntimeOption::SandboxMode || !RuntimeOption::AlwaysUseRelativePath) {
      try {
        return invoke_file(file, once, currentDir);
      } catch(PhpFileDoesNotExistException &e) {}
    }

    String rel_path(Util::relativePath(RuntimeOption::SourceRoot,
                                       string(file.data())));

    // Don't try/catch - We want the exception to be passed along
    return invoke_file(rel_path, once, currentDir);
  } else {
    // Don't try/catch - We want the exception to be passed along
    return invoke_file(file, once, currentDir);
  }
}

Variant invoke_file(CStrRef s, bool once, const char *currentDir) {
  Variant r;
  if (invoke_file_impl(r, s, once, currentDir)) {
    return r;
  }
  if (!s.empty()) {
    return throw_missing_file(s.c_str());
  }
  // The gross hack which follows is here so that "hhvm foo.php" works
  // the same as "hhvm -f foo.php".
  // TODO Task #2171414: Find a less hacky way to accomplish this; we probably
  // should be handling this elsewhere at a higher level rather than within
  // the bowels of the invoke/include machinery
  SystemGlobals* g = (SystemGlobals*)get_global_variables();
  Variant& v_argc = g->GV(argc);
  Variant& v_argv = g->GV(argv);
  if (!more(v_argc, int64_t(1))) {
    return true;
  }
  v_argc--;
  v_argv.dequeue();
  String s2 = toString(v_argv.rvalAt(int64_t(0), AccessFlags::Error));
  if (invoke_file_impl(r, s2, once, "")) {
    return r;
  }
  return throw_missing_file(s2.c_str());
}

bool invoke_file_impl(Variant &res, CStrRef path, bool once,
                      const char *currentDir) {
  bool initial;
  HPHP::Eval::PhpFile* efile =
    g_vmContext->lookupPhpFile(path.get(), currentDir, &initial);
  HPHP::VM::Unit* u = nullptr;
  if (efile) u = efile->unit();
  if (u == nullptr) {
    return false;
  }
  if (!once || initial) {
    g_vmContext->invokeUnit((TypedValue*)(&res), u);
  }
  return true;
}

/**
 * Used by include_impl.  resolve_include() needs some way of checking the
 * existence of a file path, which for hphpc means attempting to invoke it.
 * This struct carries some context information needed for the invocation, as
 * well as a place for the return value of invoking the file.
 */
struct IncludeImplInvokeContext {
  bool once;
  const char* currentDir;

  Variant returnValue;
};

static bool include_impl_invoke_context(CStrRef file, void* ctx) {
  struct IncludeImplInvokeContext* context = (IncludeImplInvokeContext*)ctx;
  bool invoked_file = false;
  try {
    context->returnValue = include_impl_invoke(file, context->once,
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
      String path(currentDir);
      path += "/";
      path += file;
      String can_path(Util::canonicalize(path.c_str(), path.size()),
                      AttachString);

      if (tryFile(can_path, ctx)) {
        return can_path;
      }
    } else {
      String path(g_context->getCwd() + "/" + currentDir + file);
      String can_path(Util::canonicalize(path.c_str(), path.size()),
                      AttachString);

      if (tryFile(can_path, ctx)) {
        return can_path;
      }
    }
  }

  return String();
}

static Variant include_impl(CStrRef file, bool once,
                            const char *currentDir, bool required,
                            bool raiseNotice) {
  struct IncludeImplInvokeContext ctx = {once, currentDir};
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
                const char *currentDir /* = NULL */,
                bool raiseNotice /*= true*/) {
  return include_impl(file, once, currentDir, false, raiseNotice);
}

Variant require(CStrRef file, bool once /* = false */,
                const char *currentDir /* = NULL */,
                bool raiseNotice /*= true*/) {
  return include_impl(file, once, currentDir, true, raiseNotice);
}

///////////////////////////////////////////////////////////////////////////////
// class Limits

IMPLEMENT_REQUEST_LOCAL(AutoloadHandler, AutoloadHandler::s_instance);

void AutoloadHandler::requestInit() {
  m_running = false;
  m_handlers.reset();
  m_map.reset();
  m_map_root.reset();
}

void AutoloadHandler::requestShutdown() {
  m_handlers.reset();
  m_map.reset();
  m_map_root.reset();
}

bool AutoloadHandler::setMap(CArrRef map, CStrRef root) {
  this->m_map = map;
  this->m_map_root = root;
  return true;
}

class ClassExistsChecker {
 public:
  ClassExistsChecker() {}
  bool operator()(CStrRef name) const {
    return VM::Unit::lookupClass(name.get()) != nullptr;
  }
};

class ConstantExistsChecker {
 public:
  bool operator()(CStrRef name) const {
    if (ClassInfo::FindConstant(name)) return true;
    return g_vmContext->defined(name);
  }
};

template <class T>
AutoloadHandler::Result AutoloadHandler::loadFromMap(CStrRef name,
                                                     CStrRef kind,
                                                     bool toLower,
                                                     const T &checkExists) {
  assert(!m_map.isNull());
  while (true) {
    CVarRef &type_map = m_map.get()->get(kind);
    Variant::TypedValueAccessor tva = type_map.getTypedAccessor();
    if (Variant::GetAccessorType(tva) != KindOfArray) return Failure;
    String canonicalName = toLower ? StringUtil::ToLower(name) : name;
    CVarRef &file = Variant::GetArrayData(tva)->get(canonicalName);
    bool ok = false;
    if (file.isString()) {
      String fName = file.toCStrRef().get();
      if (fName.get()->data()[0] != '/') {
        if (!m_map_root.empty()) {
          fName = m_map_root + fName;
        }
      }
      try {
        VM::Transl::VMRegAnchor _;
        bool initial;
        VMExecutionContext* ec = g_vmContext;
        VM::Unit* u = ec->evalInclude(fName.get(), nullptr, &initial);
        if (u) {
          if (initial) {
            TypedValue retval;
            ec->invokeFunc(&retval, u->getMain(), Array(),
                           nullptr, nullptr, nullptr, nullptr, u);
            tvRefcountedDecRef(&retval);
          }
          ok = true;
        }
      } catch (...) {}
    }
    if (ok && checkExists(name)) {
      return Success;
    }
    CVarRef &func = m_map.get()->get(s_failure);
    if (!isset(func)) return Failure;
    // can throw, otherwise
    //  - true means the map was updated. try again
    //  - false means we should stop applying autoloaders (only affects classes)
    //  - anything else means keep going
    Variant action = f_call_user_func_array(func, CREATE_VECTOR2(kind, name));
    tva = action.getTypedAccessor();
    if (Variant::GetAccessorType(tva) == KindOfBoolean) {
      if (Variant::GetBoolean(tva)) continue;
      return StopAutoloading;
    }
    return ContinueAutoloading;
  }
}

bool AutoloadHandler::autoloadFunc(CStrRef name) {
  return !m_map.isNull() &&
    loadFromMap(name, s_function, true, function_exists) != Failure;
}

bool AutoloadHandler::autoloadConstant(CStrRef name) {
  return !m_map.isNull() &&
    loadFromMap(name, s_constant, false, ConstantExistsChecker()) != Failure;
}

/**
 * invokeHandler returns true if any autoload handlers were executed,
 * false otherwise. When this function returns true, it is the caller's
 * responsibility to check if the given class or interface exists.
 */
bool AutoloadHandler::invokeHandler(CStrRef className,
                                    bool forceSplStack /* = false */) {
  if (!m_map.isNull()) {
    ClassExistsChecker ce;
    Result res = loadFromMap(className, s_class, true, ce);
    if (res == ContinueAutoloading) {
      if (ce(className)) return true;
    } else {
      if (res != Failure) return res == Success;
    }
  }
  Array params(ArrayInit(1, ArrayInit::vectorInit).set(className).create());
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
      assert(ex.instanceof(SystemLib::s_ExceptionClass));
      if (autoloadException.isNull()) {
        autoloadException = ex;
      } else {
        Object cur = ex;
        Variant next = cur->o_get(s_previous, false, s_exception);
        while (next.isObject()) {
          cur = next.toObject();
          next = cur->o_get(s_previous, false, s_exception);
        }
        cur->o_set(s_previous, autoloadException, s_exception);
        autoloadException = ex;
      }
    }
    if (VM::Unit::lookupClass(className.get()) != nullptr) {
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
    Variant first = handler.getArrayData()->get(int64_t(0));
    if (first.isObject()) {
      // Add the object address as part of the signature
      int64_t data = (int64_t)first.getObjectData();
      lName += String((const char *)&data, sizeof(data), CopyString);
    }
  } else if (handler.isObject()) {
    // "lName" will just be "classname::__invoke",
    // add object address to differentiate the signature
    int64_t data = (int64_t)handler.getObjectData();
    lName += String((const char*)&data, sizeof(data), CopyString);
  }
  return lName;
}

bool function_exists(CStrRef function_name) {
  return HPHP::VM::Unit::lookupFunc(function_name.get()) != nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// debugger and code coverage instrumentation

inline void throw_exception_unchecked(CObjRef e) {
  if (!Eval::Debugger::InterruptException(e)) return;
  throw e;
}
void throw_exception(CObjRef e) {
  if (!e.instanceof(SystemLib::s_ExceptionClass)) {
    raise_error("Exceptions must be valid objects derived from the "
                "Exception base class");
  }
  throw_exception_unchecked(e);
}

///////////////////////////////////////////////////////////////////////////////
}
