/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/ext/ext_process.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/system/systemlib.h"
#include "folly/Format.h"
#include "hphp/util/text-util.h"
#include "hphp/util/string-vsnprintf.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/backtrace.h"

#include <limits>
#include <algorithm>

namespace HPHP {

using std::string;

///////////////////////////////////////////////////////////////////////////////
// static strings

const StaticString
  s_offsetExists("offsetExists"),
  s___call("__call"),
  s___callStatic("__callStatic"),
  s___invoke("__invoke"),
  s_self("self"),
  s_parent("parent"),
  s_static("static");

///////////////////////////////////////////////////////////////////////////////

bool array_is_valid_callback(const Array& arr) {
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
vm_decode_function(const Variant& function,
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
        if (sclass.get()->isame(s_self.get())) {
          if (ctx) {
            cls = ctx;
          }
          if (!nameContainsClass) {
            forwarding = true;
          }
        } else if (sclass.get()->isame(s_parent.get())) {
          if (ctx && ctx->parent()) {
            cls = ctx->parent();
          }
          if (!nameContainsClass) {
            forwarding = true;
          }
        } else if (sclass.get()->isame(s_static.get())) {
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
            if (nameClass.get()->isame(s_self.get())   ||
                nameClass.get()->isame(s_static.get())) {
              raise_warning("behavior of call_user_func(array('%s', '%s')) "
                            "is undefined", sclass.data(), name.data());
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
      if (c.get()->isame(s_self.get())) {
        if (cls) {
          cc = cls;
        } else if (ctx) {
          cc = ctx;
        }
        if (!this_) {
          forwarding = true;
        }
      } else if (c.get()->isame(s_parent.get())) {
        if (cls) {
          cc = cls->parent();
        } else if (ctx && ctx->parent()) {
          cc = ctx->parent();
        }
        if (!this_) {
          forwarding = true;
        }
      } else if (c.get()->isame(s_static.get())) {
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
                                 name.data());
        }
        return nullptr;
      }
      assert(f && f->preClass() == nullptr);
      return f;
    }
    assert(cls);
    CallType lookupType = this_ ? CallType::ObjMethod : CallType::ClsMethod;
    const HPHP::Func* f =
      g_context->lookupMethodCtx(cc, name.get(), ctx, lookupType);
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
        if (f && (cc == cls || cc->lookupMethod(f->name()))) {
          // We found __call or __callStatic!
          // Stash the original name into invName.
          invName = name.get();
          invName->incRefCount();
        } else {
          // Bail out if we couldn't find the method or __call
          if (warn) {
            throw_invalid_argument("function: method '%s' not found",
                                   name.data());
          }
          return nullptr;
        }
      }
    }

    if (f->isClosureBody() && !this_) {
      if (warn) {
        raise_warning("cannot invoke closure body without closure object");
      }
      return nullptr;
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
    if (warn && f == nullptr) {
      raise_warning("call_user_func() expects parameter 1 to be a valid "
                    "callback, object of class %s given (no __invoke "
                    "method found)", this_->getVMClass()->name()->data());
    }
    return f;
  }
  if (warn) {
    throw_invalid_argument("function: not string, closure, or array");
  }
  return nullptr;
}

Variant vm_call_user_func(const Variant& function, const Variant& params,
                          bool forwarding /* = false */) {
  ObjectData* obj = nullptr;
  Class* cls = nullptr;
  CallerFrame cf;
  StringData* invName = nullptr;
  const Func* f = vm_decode_function(function, cf(), forwarding,
                                     obj, cls, invName);
  if (f == nullptr || (!isContainer(params) && !params.isNull())) {
    return uninit_null();
  }
  Variant ret;
  g_context->invokeFunc((TypedValue*)&ret, f, params, obj, cls,
                          nullptr, invName, ExecutionContext::InvokeCuf);
  return ret;
}

static Variant invoke_failed(const char *func,
                             bool fatal /* = true */) {
  if (fatal) {
    throw ExtendedException("(1) call the function without enough arguments OR "
                            "(2) Unable to find function \"%s\" OR "
                            "(3) function was not in invoke table OR "
                            "(4) function was renamed to something else.",
                            func);
  }
  raise_warning("call_user_func to non-existent function %s", func);
  return false;
}

static Variant invoke(const String& function, const Variant& params,
                      strhash_t hash, bool tryInterp,
                      bool fatal) {
  Func* func = Unit::loadFunc(function.get());
  if (func && (isContainer(params) || params.isNull())) {
    Variant ret;
    g_context->invokeFunc(ret.asTypedValue(), func, params);
    return ret;
  }
  return invoke_failed(function.c_str(), fatal);
}

// Declared in externals.h.  If you're considering calling this
// function for some new code, please reconsider.
Variant invoke(const char *function, const Variant& params, strhash_t hash /* = -1 */,
               bool tryInterp /* = true */, bool fatal /* = true */) {
  String funcName(function, CopyString);
  return invoke(funcName, params, hash, tryInterp, fatal);
}

Variant invoke_static_method(const String& s, const String& method,
                             const Variant& params, bool fatal /* = true */) {
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
  g_context->invokeFunc((TypedValue*)&ret, f, params, nullptr, class_);
  return ret;
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
    raise_error("Cannot access empty property");
  }
  raise_error("Cannot access property started with '\\0'");
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
  Object e(SystemLib::AllocInvalidOperationExceptionObject(msg));
  throw e;
}

void throw_param_is_not_container() {
  static const string msg("Parameter must be an array or collection");
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(msg));
  throw e;
}

void throw_cannot_modify_immutable_object(const char* className) {
  auto msg = folly::format(
    "Cannot modify immutable object of type {}",
    className
  ).str();
  Object e(SystemLib::AllocInvalidOperationExceptionObject(msg));
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
  return g_context->createObjectOnly(s.get());
}

Object create_object(const String& s, const Array& params, bool init /* = true */) {
  return g_context->createObject(s.get(), params, init);
}

/*
 * This function is used when another thread is segfaulting---we just
 * want to wait forever to give it a chance to write a stacktrace file
 * (and maybe a core file).
 */
void pause_forever() {
  for (;;) sleep(300);
}

void throw_wrong_argument_count_nr(const char *fn, int expected, int got,
                                   const char *expectDesc,
                                   int level /* = 0 */,
                                   TypedValue *rv /* = nullptr */) {
  if (rv != nullptr) {
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  if (level == 2) {
    if (expected == 1) {
      raise_error(Strings::MISSING_ARGUMENT, fn, expectDesc, got);
    } else {
      raise_error(Strings::MISSING_ARGUMENTS, fn, expectDesc, expected, got);
    }
  } else {
    if (expected == 1) {
      raise_warning(Strings::MISSING_ARGUMENT, fn, expectDesc, got);
    } else {
      raise_warning(Strings::MISSING_ARGUMENTS, fn, expectDesc, expected, got);
    }
  }
}

void throw_missing_arguments_nr(const char *fn, int expected, int got,
                                int level /* = 0 */,
                                TypedValue *rv /* = nullptr */) {
  throw_wrong_argument_count_nr(fn, expected, got, "exactly", level, rv);
}

void throw_toomany_arguments_nr(const char *fn, int expected, int got,
                                int level /* = 0 */,
                                TypedValue *rv /* = nullptr */) {
  throw_wrong_argument_count_nr(fn, expected, got, "exactly", level, rv);
}

void throw_wrong_arguments_nr(const char *fn, int count, int cmin, int cmax,
                              int level /* = 0 */,
                              TypedValue *rv /* = nullptr */) {
  if (cmin >= 0 && count < cmin) {
    if (cmin != cmax) {
      throw_wrong_argument_count_nr(fn, cmin, count, "at least", level, rv);
    } else {
      throw_wrong_argument_count_nr(fn, cmin, count, "exactly", level, rv);
    }
    return;
  }
  if (cmax >= 0 && count > cmax) {
    if (cmin != cmax) {
      throw_wrong_argument_count_nr(fn, cmax, count, "at most", level, rv);
    } else {
      throw_wrong_argument_count_nr(fn, cmax, count, "exactly", level, rv);
    }
    return;
  }
  if (rv != nullptr) {
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  assert(false);
}

void throw_bad_type_exception(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string msg;
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  raise_warning("Invalid operand type was used: %s", msg.c_str());
}

void throw_expected_array_exception() {
  const char* fn = "(unknown)";
  ActRec *ar = g_context->getStackFrame();
  if (ar) {
    fn = ar->m_func->name()->data();
  }
  throw_bad_type_exception("%s expects array(s)", fn);
}

void throw_expected_array_or_collection_exception() {
  const char* fn = "(unknown)";
  ActRec *ar = g_context->getStackFrame();
  if (ar) {
    fn = ar->m_func->name()->data();
  }
  throw_bad_type_exception("%s expects array(s) or collection(s)", fn);
}

void throw_invalid_argument(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string msg;
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_warning("Invalid argument: %s", msg.c_str());
}

Variant throw_fatal_unset_static_property(const char *s, const char *prop) {
  raise_error("Attempt to unset static property %s::$%s", s, prop);
  return uninit_null();
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
  Array exceptionStack = createBacktrace(BacktraceArgs()
                                         .withSelf()
                                         .withThis());
  ret = new RequestTimeoutException(exceptionMsg, exceptionStack);
  return ret;
}

Exception* generate_memory_exceeded_exception() {
  Array exceptionStack = createBacktrace(BacktraceArgs()
                                         .withSelf()
                                         .withThis());
  return new RequestMemoryExceededException(
    "request has exceeded memory limit", exceptionStack);
}

Variant unserialize_ex(const char* str, int len,
                       VariableUnserializer::Type type,
                       const Array& class_whitelist /* = null_array */) {
  if (str == nullptr || len <= 0) {
    return false;
  }

  VariableUnserializer vu(str, len, type, true, class_whitelist);
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
                       const Array& class_whitelist /* = null_array */) {
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

static bool invoke_file_impl(Variant& res, const String& path, bool once,
                             const char *currentDir) {
  bool initial;
  auto const u = lookupUnit(path.get(), currentDir, &initial);
  if (u == nullptr) return false;
  if (!once || initial) {
    g_context->invokeUnit(res.asTypedValue(), u);
  }
  return true;
}

static NEVER_INLINE Variant throw_missing_file(const char* file) {
  throw PhpFileDoesNotExistException(file);
}

static Variant invoke_file(const String& s,
                           bool once,
                           const char *currentDir) {
  Variant r;
  if (invoke_file_impl(r, s, once, currentDir)) {
    return r;
  }
  return throw_missing_file(s.c_str());
}

Variant include_impl_invoke(const String& file, bool once,
                            const char *currentDir) {
  if (file[0] == '/') {
    if (RuntimeOption::SandboxMode || !RuntimeOption::AlwaysUseRelativePath) {
      try {
        return invoke_file(file, once, currentDir);
      } catch(PhpFileDoesNotExistException &e) {}
    }

    String rel_path(FileUtil::relativePath(RuntimeOption::SourceRoot,
                                           string(file.data())));

    // Don't try/catch - We want the exception to be passed along
    return invoke_file(rel_path, once, currentDir);
  } else {
    // Don't try/catch - We want the exception to be passed along
    return invoke_file(file, once, currentDir);
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
  const char* c_file = file.data();

  if (!File::IsPlainFilePath(file)) {
    // URIs don't have an include path
    if (tryFile(file, ctx)) {
      return file;
    }

  } else if (c_file[0] == '/') {
    String can_path = FileUtil::canonicalize(file);

    if (tryFile(can_path, ctx)) {
      return can_path;
    }

  } else if ((c_file[0] == '.' && (c_file[1] == '/' || (
    c_file[1] == '.' && c_file[2] == '/')))) {

    String path(String(g_context->getCwd() + "/" + file));
    String can_path = FileUtil::canonicalize(path);

    if (tryFile(can_path, ctx)) {
      return can_path;
    }

  } else {
    auto includePaths = ThreadInfo::s_threadInfo.getNoCheck()->
      m_reqInjectionData.getIncludePaths();
    unsigned int path_count = includePaths.size();

    for (int i = 0; i < (int)path_count; i++) {
      String path("");
      String includePath(includePaths[i]);

      if (includePath[0] != '/') {
        path += (g_context->getCwd() + "/");
      }

      path += includePath;

      if (path[path.size() - 1] != '/') {
        path += "/";
      }

      path += file;
      String can_path = FileUtil::canonicalize(path);

      if (tryFile(can_path, ctx)) {
        return can_path;
      }
    }

    if (currentDir[0] == '/') {
      String path(currentDir);
      path += "/";
      path += file;
      String can_path = FileUtil::canonicalize(path);

      if (tryFile(can_path, ctx)) {
        return can_path;
      }
    } else {
      String path(g_context->getCwd() + "/" + currentDir + file);
      String can_path = FileUtil::canonicalize(path);

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
      raise_notice("Tried to invoke %s but file not found.", file.data());
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

Variant require(const String& file,
                bool once,
                const char* currentDir,
                bool raiseNotice) {
  return include_impl(file, once, currentDir, true, raiseNotice);
}

bool function_exists(const String& function_name) {
  auto f = Unit::lookupFunc(function_name.get());
  return (f != nullptr) &&
         (f->builtinFuncPtr() != Native::unimplementedWrapper);
}

///////////////////////////////////////////////////////////////////////////////
// debugger and code coverage instrumentation

void throw_exception(const Object& e) {
  if (!e.instanceof(SystemLib::s_ExceptionClass)) {
    raise_error("Exceptions must be valid objects derived from the "
                "Exception base class");
  }
  DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionThrownHook(e.get()));
  throw e;
}

///////////////////////////////////////////////////////////////////////////////
}
