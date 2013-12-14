/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/builtin-functions.h"

#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/file-repository.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/ext/ext_process.h"
#include "hphp/runtime/ext/ext_class.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/util/logger.h"
#include "hphp/util/util.h"
#include "hphp/util/process.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/system/systemlib.h"
#include "folly/Format.h"

#include <limits>

using namespace HPHP::MethodLookup;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// static strings

const StaticString
  s_offsetExists("offsetExists"),
  s___autoload("__autoload"),
  s___call("__call"),
  s___callStatic("__callStatic"),
  s___invoke("__invoke"),
  s_exception("exception"),
  s_previous("previous"),
  s_self("self"),
  s_parent("parent"),
  s_static("static"),
  s_class("class"),
  s_function("function"),
  s_constant("constant"),
  s_type("type"),
  s_failure("failure");

///////////////////////////////////////////////////////////////////////////////

typedef smart::unique_ptr<CufIter> SmartCufIterPtr;

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

const HPHP::Func*
vm_decode_function(CVarRef function,
                   ActRec* ar,
                   bool forwarding,
                   ObjectData*& this_,
                   HPHP::Class*& cls,
                   StringData*& invName,
                   bool warn /* = true */) {
  invName = nullptr;
  if (function.isString() || function.isArray()) {
    HPHP::Class* ctx = nullptr;
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
          cls = Unit::loadClass(sclass.get());
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

    HPHP::Class* cc = cls;
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
        cc = Unit::loadClass(c.get());
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
      HPHP::Func* f = HPHP::Unit::loadFunc(name.get());
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
    CallType lookupType = this_ ? CallType::ObjMethod : CallType::ClsMethod;
    const HPHP::Func* f =
      g_vmContext->lookupMethodCtx(cc, name.get(), ctx, lookupType);
    if (f && (f->attrs() & AttrStatic)) {
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
          assert(!f || !(f->attrs() & AttrStatic));
        }
        if (!f && lookupType == CallType::ClsMethod) {
          f = cls->lookupMethod(s___callStatic.get());
          assert(!f || (f->attrs() & AttrStatic));
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
      HPHP::Class* fwdCls = nullptr;
      ObjectData* obj = ar->hasThis() ? ar->getThis() : nullptr;
      if (obj) {
        fwdCls = obj->getVMClass();
      } else if (ar->hasClass()) {
        fwdCls = ar->getClass();
      }
      // Only forward the current late bound class if it is the same or
      // a descendent of cls
      if (fwdCls && fwdCls->classof(cls)) {
        cls = fwdCls;
      }
    }
    return f;
  }
  if (function.isObject()) {
    this_ = function.asCObjRef().get();
    cls = nullptr;
    const HPHP::Func *f = this_->getVMClass()->lookupMethod(s___invoke.get());
    if (f != nullptr &&
        ((f->attrs() & AttrStatic) && !f->isClosureBody())) {
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

Variant vm_call_user_func(CVarRef function, CVarRef params,
                          bool forwarding /* = false */) {
  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  HPHP::JIT::CallerFrame cf;
  StringData* invName = nullptr;
  const HPHP::Func* f = vm_decode_function(function, cf(), forwarding,
                                           obj, cls, invName);
  if (f == nullptr || (!isContainer(params) && !params.isNull())) {
    return uninit_null();
  }
  Variant ret;
  g_vmContext->invokeFunc((TypedValue*)&ret, f, params, obj, cls,
                          nullptr, invName, ExecutionContext::InvokeCuf);
  return ret;
}

/*
 * Helper method from converting between a PHP function and a CufIter.
 */
static bool vm_decode_function_cufiter(CVarRef function,
                                       SmartCufIterPtr& cufIter) {
  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  HPHP::JIT::CallerFrame cf;
  StringData* invName = nullptr;
  // Don't warn here, let the caller decide what to do if the func is nullptr.
  const HPHP::Func* func = vm_decode_function(function, cf(), false,
                                              obj, cls, invName, false);
  if (func == nullptr) {
    return false;
  }

  cufIter = smart::make_unique<CufIter>();
  cufIter->setFunc(func);
  cufIter->setName(invName);
  if (obj) {
    cufIter->setCtx(obj);
    obj->incRefCount();
  } else {
    cufIter->setCtx(cls);
  }

  return true;
}

/*
 * Wraps calling an (autoload) PHP function from a CufIter.
 */
static Variant vm_call_user_func_cufiter(const CufIter& cufIter,
                                         CArrRef params) {
  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = cufIter.name();
  const HPHP::Func* f = cufIter.func();
  if (cufIter.ctx()) {
    if (uintptr_t(cufIter.ctx()) & 1) {
      cls = (Class*)(uintptr_t(cufIter.ctx()) & ~1);
    } else {
      obj = (ObjectData*)cufIter.ctx();
    }
  }
  assert(!obj || !cls);
  if (invName) {
    invName->incRefCount();
  }
  Variant ret;
  g_vmContext->invokeFunc((TypedValue*)&ret, f, params, obj, cls,
                          nullptr, invName, ExecutionContext::InvokeCuf);
  return ret;
}

Variant invoke(const String& function, CVarRef params,
               strhash_t hash /* = -1 */, bool tryInterp /* = true */,
               bool fatal /* = true */) {
  Func* func = Unit::loadFunc(function.get());
  if (func && (isContainer(params) || params.isNull())) {
    Variant ret;
    g_vmContext->invokeFunc(ret.asTypedValue(), func, params);
    return ret;
  }
  return invoke_failed(function.c_str(), fatal);
}

Variant invoke(const char *function, CVarRef params, strhash_t hash /* = -1 */,
               bool tryInterp /* = true */, bool fatal /* = true */) {
  String funcName(function, CopyString);
  return invoke(funcName, params, hash, tryInterp, fatal);
}

Variant invoke_static_method(const String& s, const String& method,
                             CVarRef params, bool fatal /* = true */) {
  HPHP::Class* class_ = Unit::lookupClass(s.get());
  if (class_ == nullptr) {
    o_invoke_failed(s.data(), method.data(), fatal);
    return uninit_null();
  }
  const HPHP::Func* f = class_->lookupMethod(method.get());
  if (f == nullptr || !(f->attrs() & AttrStatic) ||
    (!isContainer(params) && !params.isNull())) {
    o_invoke_failed(s.data(), method.data(), fatal);
    return uninit_null();
  }
  Variant ret;
  g_vmContext->invokeFunc((TypedValue*)&ret, f, params, nullptr, class_);
  return ret;
}

Variant invoke_failed(CVarRef func,
                      bool fatal /* = true */) {
  if (func.isObject()) {
    return o_invoke_failed(
        func.objectForCall()->o_getClassName().c_str(),
        "__invoke", fatal);
  } else {
    return invoke_failed(func.toString().c_str(), fatal);
  }
}

Variant invoke_failed(const char *func,
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

void NEVER_INLINE throw_null_get_object_prop() {
  raise_error("Trying to get property of non-object");
}

void NEVER_INLINE throw_null_object_prop() {
  raise_error("Trying to set property of non-object");
}

void NEVER_INLINE throw_invalid_property_name(const String& name) {
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
    "Cannot access a property on a collection"));
  throw e;
}

void throw_collection_compare_exception() {
  static const string msg(
    "Cannot use relational comparison operators (<, <=, >, >=) to compare "
    "a collection with an integer, double, string, array, or object");
  Object e(SystemLib::AllocRuntimeExceptionObject(msg));
  throw e;
}

void throw_param_is_not_container() {
  static const string msg("Parameter must be an array or collection");
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(msg));
  throw e;
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

Object create_object_only(const String& s) {
  return g_vmContext->createObjectOnly(s.get());
}

Object create_object(const String& s, CArrRef params, bool init /* = true */) {
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

ssize_t check_request_surprise(ThreadInfo *info) {
  RequestInjectionData &p = info->m_reqInjectionData;
  bool do_timedout, do_memExceeded, do_signaled;

  ssize_t flags = p.fetchAndClearFlags();
  do_timedout = (flags & RequestInjectionData::TimedOutFlag) &&
    !p.getDebugger();
  do_memExceeded = (flags & RequestInjectionData::MemExceededFlag);
  do_signaled = (flags & RequestInjectionData::SignaledFlag);

  // Start with any pending exception that might be on the thread.
  Exception* pendingException = info->m_pendingException;
  info->m_pendingException = nullptr;

  if (do_timedout && !pendingException) {
    pendingException = generate_request_timeout_exception();
  }
  if (do_memExceeded && !pendingException) {
    pendingException = generate_memory_exceeded_exception();
  }
  if (do_signaled) f_pcntl_signal_dispatch();

  if (pendingException) {
    pendingException->throwException();
  }
  return flags;
}

void throw_missing_arguments_nr(const char *fn, int expected, int got,
                                int level /* = 0 */,
                                TypedValue *rv /* = nullptr */) {
  if (rv != nullptr) {
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  if (level == 2 || RuntimeOption::ThrowMissingArguments) {
    if (expected == 1) {
      raise_error(Strings::MISSING_ARGUMENT, fn, got);
    } else {
      raise_error(Strings::MISSING_ARGUMENTS, fn, expected, got);
    }
  } else {
    if (expected == 1) {
      raise_warning(Strings::MISSING_ARGUMENT, fn, got);
    } else {
      raise_warning(Strings::MISSING_ARGUMENTS, fn, expected, got);
    }
  }
}

void throw_toomany_arguments_nr(const char *fn, int num, int level /* = 0 */,
                                TypedValue *rv /* = nullptr */) {
  if (rv != nullptr) {
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  if (level == 2 || RuntimeOption::ThrowTooManyArguments) {
    raise_error("Too many arguments for %s(), expected %d", fn, num);
  } else if (level == 1 || RuntimeOption::WarnTooManyArguments) {
    raise_warning("Too many arguments for %s(), expected %d", fn, num);
  }
}

void throw_wrong_arguments_nr(const char *fn, int count, int cmin, int cmax,
                              int level /* = 0 */,
                              TypedValue *rv /* = nullptr */) {
  if (rv != nullptr) {
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  if (cmin >= 0 && count < cmin) {
    throw_missing_arguments_nr(fn, cmin, count, level);
    return;
  }
  if (cmax >= 0 && count > cmax) {
    throw_toomany_arguments_nr(fn, cmax, level);
    return;
  }
  assert(false);
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

void throw_expected_array_exception() {
  const char* fn = "(unknown)";
  ActRec *ar = g_vmContext->getStackFrame();
  if (ar) {
    fn = ar->m_func->name()->data();
  }
  throw_bad_type_exception("%s expects array(s)", fn);
}

void throw_expected_array_or_collection_exception() {
  const char* fn = "(unknown)";
  ActRec *ar = g_vmContext->getStackFrame();
  if (ar) {
    fn = ar->m_func->name()->data();
  }
  throw_bad_type_exception("%s expects array(s) or collection(s)", fn);
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
  return uninit_null();
}

void throw_infinite_recursion_exception() {
  if (!RuntimeOption::NoInfiniteRecursionDetection) {
    // Reset profiler otherwise it might recurse further causing segfault
    DECLARE_THREAD_INFO
    info->m_profiler = nullptr;
    throw UncatchableException("infinite recursion detected");
  }
}

Exception* generate_request_timeout_exception() {
  Exception* ret = nullptr;
  ThreadInfo *info = ThreadInfo::s_threadInfo.getNoCheck();
  RequestInjectionData &data = info->m_reqInjectionData;

  bool cli = RuntimeOption::ClientExecutionMode();
  std::string exceptionMsg = cli ?
    "Maximum execution time of " :
    "entire web request took longer than ";
  exceptionMsg += folly::to<std::string>(data.getTimeout());
  exceptionMsg += cli ? " seconds exceeded" : " seconds and timed out";
  ArrayHolder exceptionStack;
  if (RuntimeOption::InjectedStackTrace) {
    exceptionStack = g_vmContext->debugBacktrace(false, true, true).get();
  }
  ret = new RequestTimeoutException(exceptionMsg, exceptionStack.get());

  return ret;
}

Exception* generate_memory_exceeded_exception() {
  ArrayHolder exceptionStack;
  if (RuntimeOption::InjectedStackTrace) {
    exceptionStack = g_vmContext->debugBacktrace(false, true, true).get();
  }
  return new RequestMemoryExceededException(
    "request has exceeded memory limit", exceptionStack.get());
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
  case KindOfResource:
  case KindOfDouble: {
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    return vs.serialize(value, true);
  }
  default:
    assert(false);
    break;
  }
  return "";
}

Variant unserialize_ex(const char* str, int len,
                       VariableUnserializer::Type type,
                       CArrRef class_whitelist /* = null_array */) {
  if (str == nullptr || len <= 0) {
    return false;
  }

  VariableUnserializer vu(str, len, type, false, class_whitelist);
  Variant v;
  try {
    v = vu.unserialize();
  } catch (FatalErrorException &e) {
    throw;
  } catch (Exception &e) {
    raise_notice("Unable to unserialize: [%s]. %s.", str,
                 e.getMessage().c_str());
    return false;
  }
  return v;
}

Variant unserialize_ex(const String& str,
                       VariableUnserializer::Type type,
                       CArrRef class_whitelist /* = null_array */) {
  return unserialize_ex(str.data(), str.size(), type, class_whitelist);
}

String concat3(const String& s1, const String& s2, const String& s3) {
  StringSlice r1 = s1.slice();
  StringSlice r2 = s2.slice();
  StringSlice r3 = s3.slice();
  int len = r1.len + r2.len + r3.len;
  StringData* str = StringData::Make(len);
  auto const r = str->bufferSlice();
  memcpy(r.ptr,                   r1.ptr, r1.len);
  memcpy(r.ptr + r1.len,          r2.ptr, r2.len);
  memcpy(r.ptr + r1.len + r2.len, r3.ptr, r3.len);
  str->setSize(len);
  return str;
}

String concat4(const String& s1, const String& s2, const String& s3,
               const String& s4) {
  StringSlice r1 = s1.slice();
  StringSlice r2 = s2.slice();
  StringSlice r3 = s3.slice();
  StringSlice r4 = s4.slice();
  int len = r1.len + r2.len + r3.len + r4.len;
  StringData* str = StringData::Make(len);
  auto const r = str->bufferSlice();
  memcpy(r.ptr,                            r1.ptr, r1.len);
  memcpy(r.ptr + r1.len,                   r2.ptr, r2.len);
  memcpy(r.ptr + r1.len + r2.len,          r3.ptr, r3.len);
  memcpy(r.ptr + r1.len + r2.len + r3.len, r4.ptr, r4.len);
  str->setSize(len);
  return str;
}

Variant include_impl_invoke(const String& file, bool once,
                            const char *currentDir) {
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

Variant invoke_file(const String& s, bool once, const char *currentDir) {
  Variant r;
  if (invoke_file_impl(r, s, once, currentDir)) {
    return r;
  }
  return throw_missing_file(s.c_str());
}

bool invoke_file_impl(Variant &res, const String& path, bool once,
                      const char *currentDir) {
  bool initial;
  HPHP::Eval::PhpFile* efile =
    g_vmContext->lookupPhpFile(path.get(), currentDir, &initial);
  HPHP::Unit* u = nullptr;
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

static bool include_impl_invoke_context(const String& file, void* ctx) {
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
String resolve_include(const String& file, const char* currentDir,
                       bool (*tryFile)(const String& file, void*), void* ctx) {
  const char* c_file = file->data();

  if (!File::IsPlainFilePath(file)) {
    // URIs don't have an include path
    if (tryFile(file, ctx)) {
      return file;
    }

  } else if (c_file[0] == '/') {
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
      String includePath = includePaths[i].toString();

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

static Variant include_impl(const String& file, bool once,
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

Variant include(const String& file, bool once /* = false */,
                const char *currentDir /* = NULL */,
                bool raiseNotice /*= true*/) {
  return include_impl(file, once, currentDir, false, raiseNotice);
}

Variant require(const String& file, bool once /* = false */,
                const char *currentDir /* = NULL */,
                bool raiseNotice /*= true*/) {
  return include_impl(file, once, currentDir, true, raiseNotice);
}

///////////////////////////////////////////////////////////////////////////////
// class AutoloadHandler

IMPLEMENT_REQUEST_LOCAL(AutoloadHandler, AutoloadHandler::s_instance);

void AutoloadHandler::requestInit() {
  assert(m_map.get() == nullptr);
  assert(m_map_root.get() == nullptr);
  assert(m_loading.get() == nullptr);
  m_spl_stack_inited = false;
  new (&m_handlers) smart::deque<HandlerBundle>();
}

void AutoloadHandler::requestShutdown() {
  m_map.reset();
  m_map_root.reset();
  m_loading.reset();
  // m_spl_stack_inited will be re-initialized by the next requestInit
  // m_handlers will be re-initialized by the next requestInit
}

bool AutoloadHandler::setMap(CArrRef map, const String& root) {
  this->m_map = map;
  this->m_map_root = root;
  return true;
}

class ClassExistsChecker {
 public:
  ClassExistsChecker() {}
  bool operator()(const String& name) const {
    return Unit::lookupClass(name.get()) != nullptr;
  }
};

class ConstantExistsChecker {
 public:
  bool operator()(const String& name) const {
    return Unit::lookupCns(name.get()) != nullptr;
  }
};

template <class T>
AutoloadHandler::Result AutoloadHandler::loadFromMap(const String& name,
                                                     const String& kind,
                                                     bool toLower,
                                                     const T &checkExists) {
  assert(!m_map.isNull());
  while (true) {
    CVarRef &type_map = m_map.get()->get(kind);
    auto const typeMapCell = type_map.asCell();
    if (typeMapCell->m_type != KindOfArray) return Failure;
    String canonicalName = toLower ? f_strtolower(name) : name;
    CVarRef &file = typeMapCell->m_data.parr->get(canonicalName);
    bool ok = false;
    if (file.isString()) {
      String fName = file.toCStrRef().get();
      if (fName.get()->data()[0] != '/') {
        if (!m_map_root.empty()) {
          fName = m_map_root + fName;
        }
      }
      try {
        JIT::VMRegAnchor _;
        bool initial;
        VMExecutionContext* ec = g_vmContext;
        Unit* u = ec->evalInclude(fName.get(), nullptr, &initial);
        if (u) {
          if (initial) {
            TypedValue retval;
            ec->invokeFunc(&retval, u->getMain(), init_null_variant,
                           nullptr, nullptr, nullptr, nullptr,
                           ExecutionContext::InvokePseudoMain);
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
    if (func.isNull()) return Failure;
    // can throw, otherwise
    //  - true means the map was updated. try again
    //  - false means we should stop applying autoloaders (only affects classes)
    //  - anything else means keep going
    Variant action = vm_call_user_func(func, make_packed_array(kind, name));
    auto const actionCell = action.asCell();
    if (actionCell->m_type == KindOfBoolean) {
      if (actionCell->m_data.num) continue;
      return StopAutoloading;
    }
    return ContinueAutoloading;
  }
}

bool AutoloadHandler::autoloadFunc(StringData* name) {
  return !m_map.isNull() &&
    loadFromMap(name, s_function, true, function_exists) != Failure;
}

bool AutoloadHandler::autoloadConstant(StringData* name) {
  return !m_map.isNull() &&
    loadFromMap(name, s_constant, false, ConstantExistsChecker()) != Failure;
}

bool AutoloadHandler::autoloadType(const String& name) {
  return !m_map.isNull() &&
    loadFromMap(name, s_type, true,
      [] (const String& name) {
        return Unit::GetNamedEntity(name.get())->
          getCachedTypeAlias() != nullptr;
      }
    ) != Failure;
}

/**
 * invokeHandler returns true if any autoload handlers were executed,
 * false otherwise. When this function returns true, it is the caller's
 * responsibility to check if the given class or interface exists.
 */
bool AutoloadHandler::invokeHandler(const String& className,
                                    bool forceSplStack /* = false */) {

  if (className.empty()) {
    return false;
  }

  if (!m_map.isNull()) {
    ClassExistsChecker ce;
    Result res = loadFromMap(className, s_class, true, ce);
    if (res == ContinueAutoloading) {
      if (ce(className)) return true;
    } else {
      if (res != Failure) return res == Success;
    }
  }
  // If we end up in a recursive autoload loop where we try to load the same
  // class twice, just fail the load to mimic PHP as many frameworks rely on it
  // unless we are forcing a restart (due to spl_autoload_call) in which case
  // it's allowed to re-enter. This means we can still overflow the stack if
  // there is a loop when using spl_autoload_call directly but this is parity.
  if (!forceSplStack && m_loading.valueExists(className)) {
    return false;
  }

  m_loading.append(className);

  // The below code can throw so make sure we clean up the state from this load
  SCOPE_EXIT {
    String l_className = m_loading.pop();
    assert(l_className == className);
  };

  Array params = PackedArrayInit(1).append(className).toArray();
  if (!m_spl_stack_inited && !forceSplStack) {
    if (function_exists(s___autoload)) {
      invoke(s___autoload, params, -1, true, false);
      return true;
    }
    return false;
  }
  if (!m_spl_stack_inited || m_handlers.empty()) {
    return false;
  }
  Object autoloadException;
  for (const HandlerBundle& hb : m_handlers) {
    try {
      vm_call_user_func_cufiter(*hb.m_cufIter, params);
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
    if (Unit::lookupClass(className.get()) != nullptr) {
      break;
    }
  }
  if (!autoloadException.isNull()) {
    throw autoloadException;
  }
  return true;
}

Array AutoloadHandler::getHandlers() {
  if (!m_spl_stack_inited) {
    return null_array;
  }

  PackedArrayInit handlers(m_handlers.size());

  for (const HandlerBundle& hb : m_handlers) {
    handlers.append(hb.m_handler);
  }

  return handlers.toArray();
}

bool AutoloadHandler::CompareBundles::operator()(
  const HandlerBundle& hb) {
  auto const& lhs = *m_cufIter;
  auto const& rhs = *hb.m_cufIter;

  if (lhs.ctx() != rhs.ctx()) {
    // We only consider ObjectData* for equality (not a Class*) so if either is
    // an object these are not considered equal.
    if (!(uintptr_t(lhs.ctx()) & 1) || !(uintptr_t(rhs.ctx()) & 1)) {
      return false;
    }
  }

  return lhs.func() == rhs.func();
}

bool AutoloadHandler::addHandler(CVarRef handler, bool prepend) {
  SmartCufIterPtr cufIter = nullptr;
  if (!vm_decode_function_cufiter(handler, cufIter)) {
    return false;
  }

  m_spl_stack_inited = true;

  // Zend doesn't modify the order of the list if the handler is already
  // registered.
  auto const& compareBundles = CompareBundles(cufIter.get());
  if (std::find_if(m_handlers.begin(), m_handlers.end(), compareBundles) !=
      m_handlers.end()) {
    return true;
  }

  if (!prepend) {
    m_handlers.emplace_back(handler, cufIter);
  } else {
    m_handlers.emplace_front(handler, cufIter);
  }

  return true;
}

bool AutoloadHandler::isRunning() {
  return !m_loading.empty();
}

void AutoloadHandler::removeHandler(CVarRef handler) {
  SmartCufIterPtr cufIter = nullptr;
  if (!vm_decode_function_cufiter(handler, cufIter)) {
    return;
  }

  // Use find_if instead of remove_if since we know there can only be one match
  // in the vector.
  auto const& compareBundles = CompareBundles(cufIter.get());
  m_handlers.erase(
    std::find_if(m_handlers.begin(), m_handlers.end(), compareBundles));
}

void AutoloadHandler::removeAllHandlers() {
  m_spl_stack_inited = false;
  m_handlers.clear();
}

bool function_exists(const String& function_name) {
  auto f = HPHP::Unit::lookupFunc(function_name.get());
  return (f != nullptr) &&
         (f->builtinFuncPtr() != Native::unimplementedWrapper);
}

///////////////////////////////////////////////////////////////////////////////
// debugger and code coverage instrumentation

void throw_exception(CObjRef e) {
  if (!e.instanceof(SystemLib::s_ExceptionClass)) {
    raise_error("Exceptions must be valid objects derived from the "
                "Exception base class");
  }
  DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionThrownHook(e.get()));
  throw e;
}

///////////////////////////////////////////////////////////////////////////////
}
