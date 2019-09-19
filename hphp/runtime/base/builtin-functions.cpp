/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"

#include "hphp/runtime/debugger/debugger.h"

#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/string/ext_string.h"

#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/method-lookup.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/system/systemlib.h"

#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/string-vsnprintf.h"
#include "hphp/util/text-util.h"

#include <folly/Format.h>

#include <algorithm>

namespace HPHP {

using std::string;

///////////////////////////////////////////////////////////////////////////////
// static strings

const StaticString
  s_offsetExists("offsetExists"),
  s___invoke("__invoke"),
  s_self("self"),
  s_parent("parent"),
  s_static("static");

const StaticString s_cmpWithCollection(
  "Cannot use relational comparison operators (<, <=, >, >=, <=>) to compare "
  "a collection with an integer, double, string, array, or object"
);
const StaticString s_cmpWithVec(
  "Cannot use relational comparison operators (<, <=, >, >=, <=>) to compare "
  "a vec with a non-vec"
);
const StaticString s_cmpWithDict(
  "Cannot use relational comparison operators (<, <=, >, >=, <=>) to compare "
  "dicts"
);
const StaticString s_cmpWithKeyset(
  "Cannot use relational comparison operators (<, <=, >, >=, <=>) to compare "
  "keysets"
);
const StaticString s_cmpWithRecord(
  "Cannot use relational comparison operators (<, <=, >, >=, <=>) to compare "
  "records"
);
const StaticString s_cmpWithClsMeth(
  "Cannot use relational comparison operators (<, <=, >, >=, <=>) to compare "
  "a clsmeth with a non-clsmeth"
);
const StaticString s_cmpWithNonRecord(
  "Cannot compare records with non-records"
);

///////////////////////////////////////////////////////////////////////////////

bool array_is_valid_callback(const Array& arr) {
  if (!arr.isPHPArray() && !arr.isVecArray() && !arr.isDict()) return false;
  if (arr.size() != 2 || !arr.exists(int64_t(0)) || !arr.exists(int64_t(1))) {
    return false;
  }
  auto const elem0 = arr.rvalAt(0).unboxed();
  if (!isStringType(elem0.type()) && !isObjectType(elem0.type()) &&
      !isClassType(elem0.type())) {
    return false;
  }
  auto const elem1 = arr.rvalAt(1).unboxed();
  if (!isStringType(elem1.type()) && !isFuncType(elem1.type())) {
    return false;
  }
  return true;
}

const StaticString
  s__invoke("__invoke"),
  s_Closure__invoke("Closure::__invoke"),
  s_colon2("::");

bool is_callable(const Variant& v) {
  CallCtx ctx;
  ctx.invName = nullptr;
  vm_decode_function(v, ctx, DecodeFlags::LookupOnly);
  if (ctx.invName != nullptr) {
    decRefStr(ctx.invName);
  }
  return ctx.func != nullptr && !ctx.func->isAbstract();
}

bool is_callable(const Variant& v, bool syntax_only, Variant* name) {
  bool ret = true;
  if (LIKELY(!syntax_only)) {
    ret = is_callable(v);
    if (LIKELY(!name)) return ret;
  }

  auto const tv_func = v.toCell();
  if (isFuncType(tv_func->m_type)) {
    auto func_name = tv_func->m_data.pfunc->fullDisplayName();
    if (name) *name = Variant{func_name, Variant::PersistentStrInit{}};
    return true;
  }

  if (isClsMethType(tv_func->m_type)) {
    auto const clsmeth = tv_func->m_data.pclsmeth;
    if (name) {
      *name = concat3(
        clsmeth->getCls()->nameStr(), "::", clsmeth->getFunc()->nameStr());
    }
    return true;
  }

  if (isStringType(tv_func->m_type)) {
    if (name) *name = v;
    return ret;
  }

  if (isArrayType(tv_func->m_type) ||
      isVecType(tv_func->m_type) ||
      isDictType(tv_func->m_type)) {
    auto const arr = Array(tv_func->m_data.parr);
    auto const clsname = arr.rvalAt(int64_t(0)).unboxed();
    auto const mthname = arr.rvalAt(int64_t(1)).unboxed();

    if (arr.size() != 2 ||
        clsname.is_dummy() ||
        (!isStringType(mthname.type()) && !isFuncType(mthname.type()))) {
      if (name) *name = array_string;
      return false;
    }

    StringData* clsString = nullptr;
    if (isObjectType(clsname.type())) {
      clsString = clsname.val().pobj->getClassName().get();
    } else if (isStringType(clsname.type())) {
      clsString = clsname.val().pstr;
    } else if (isClassType(clsname.type())) {
      clsString = const_cast<StringData*>(clsname.val().pclass->name());
    } else {
      if (name) *name = array_string;
      return false;
    }

    if (isFuncType(mthname.type())) {
      if (name) {
        *name = Variant{mthname.val().pfunc->fullDisplayName(),
                               Variant::PersistentStrInit{}};
      }
      return true;
    }

    if (name) {
      *name = concat3(String{clsString},
                             s_colon2,
                             String{mthname.val().pstr});
    }
    return ret;
  }

  if (tv_func->m_type == KindOfObject) {
    ObjectData *d = tv_func->m_data.pobj;
    const Func* invoke = d->getVMClass()->lookupMethod(s__invoke.get());
    if (name) {
      if (d->instanceof(c_Closure::classof())) {
        // Hack to stop the mangled name from showing up
        *name = s_Closure__invoke;
      } else {
        *name = d->getClassName().asString() + "::__invoke";
      }
    }
    return invoke != nullptr;
  }

  return false;
}

namespace {
Class* vm_decode_class_from_name(
  const String& clsName,
  const String& funcName,
  bool nameContainsClass,
  ActRec* ar,
  bool& forwarding,
  Class* ctx,
  DecodeFlags flags) {
  Class* cls = nullptr;
  if (clsName.get()->isame(s_self.get())) {
    if (ctx) {
      cls = ctx;
    }
    if (!nameContainsClass) {
      forwarding = true;
    }
  } else if (clsName.get()->isame(s_parent.get())) {
    if (ctx && ctx->parent()) {
      cls = ctx->parent();
    }
    if (!nameContainsClass) {
      forwarding = true;
    }
  } else if (clsName.get()->isame(s_static.get())) {
    if (ar && ar->func()->cls()) {
      if (ar->hasThis()) {
        cls = ar->getThis()->getVMClass();
      } else {
        cls = ar->getClass();
      }
      if (flags != DecodeFlags::NoWarn && cls) {
        if (RuntimeOption::EvalWarnOnSkipFrameLookup) {
          raise_warning(
            "vm_decode_function() used to decode a LSB class "
            "method on %s",
            cls->name()->data()
          );
        }
      }
    }
  } else {
    if (flags == DecodeFlags::Warn && nameContainsClass) {
      String nameClass = funcName.substr(0, funcName.find("::"));
      if (nameClass.get()->isame(s_self.get())   ||
          nameClass.get()->isame(s_static.get())) {
        raise_warning("behavior of call_user_func(array('%s', '%s')) "
                      "is undefined", clsName.data(), funcName.data());
      }
    }
    cls = Unit::loadClass(clsName.get());
  }
  return cls;
}

const Func* vm_decode_func_from_name(
  const String& funcName,
  ActRec* ar,
  bool forwarding,
  ObjectData*& this_,
  Class*& cls,
  Class* ctx,
  Class* cc,
  StringData*& invName,
  DecodeFlags flags) {
  CallType lookupType = this_ ? CallType::ObjMethod : CallType::ClsMethod;
  auto f = lookupMethodCtx(cc, funcName.get(), ctx, lookupType);
  if (f && (f->attrs() & AttrStatic)) {
    // If we found a method and its static, null out this_
    this_ = nullptr;
  } else {
    if (!this_ && ar && ar->func()->cls() && ar->hasThis()) {
      // If we did not find a static method AND this_ is null AND there is a
      // frame ar, check if the current instance from ar is compatible
      auto const obj = ar->getThis();
      if (obj->instanceof(cls)) {
        this_ = obj;
        cls = obj->getVMClass();
      }
      if (flags != DecodeFlags::NoWarn && this_) {
        if (RuntimeOption::EvalWarnOnSkipFrameLookup) {
          raise_warning(
            "vm_decode_function() used to decode a method on $this, an "
            "instance of %s, from the caller, %s",
            cls->name()->data(),
            ar->func()->fullName()->data()
          );
        }
      }
    }
    if (!f) {
      if (this_) {
        // If this_ is non-null AND we could not find a method, try
        // looking up __call in cls's method table
        f = cls->lookupMethod(s___call.get());
        assertx(!f || !(f->attrs() & AttrStatic));
      }
      if (!f && lookupType == CallType::ClsMethod) {
        this_ = nullptr;
      }
      if (f && (cc == cls || cc->lookupMethod(f->name()))) {
        // We found __call!
        // Stash the original name into invName.
        invName = funcName.get();
        invName->incRefCount();
      } else {
        // Bail out if we couldn't find the method or __call
        if (flags == DecodeFlags::Warn) {
          throw_invalid_argument("function: method '%s' not found",
                                 funcName.data());
        }
        return nullptr;
      }
    }
  }

  if (!this_ && !f->isStaticInPrologue()) {
    if (flags == DecodeFlags::Warn) throw_missing_this(f);
    if (flags != DecodeFlags::LookupOnly) {
      if (f->attrs() & AttrRequiresThis) return nullptr;
      raise_warning(
        "Decoding instance method %s without $this!", funcName.data());
    }
  }

  assertx(f && f->preClass());
  // If this_ is non-NULL, then this_ is the current instance and cls is
  // the class of the current instance.
  assertx(!this_ || this_->getVMClass() == cls);
  // If we are doing a forwarding call and this_ is null, set cls
  // appropriately to propagate the current late bound class.
  if (!this_ && forwarding && ar && ar->func()->cls()) {
    auto const fwdCls = ar->hasThis() ?
      ar->getThis()->getVMClass() : ar->getClass();

    // Only forward the current late bound class if it is the same or
    // a descendent of cls
    if (fwdCls->classof(cls)) {
      cls = fwdCls;
    }

    if (flags != DecodeFlags::NoWarn && fwdCls) {
      if (RuntimeOption::EvalWarnOnSkipFrameLookup) {
        raise_warning(
          "vm_decode_function() forwarded the calling context, %s",
          fwdCls->name()->data()
        );
      }
    }
  }

  if (flags != DecodeFlags::NoWarn && !f->isPublic()) {
    if (RuntimeOption::EvalWarnOnSkipFrameLookup) {
      raise_warning(
        "vm_decode_function() used to decode a %s method: %s",
        f->attrs() & AttrPrivate ? "private" : "protected",
        f->fullDisplayName()->data()
      );
    }
  }
  return f;
}
}

std::pair<Class*, Func*> decode_for_clsmeth(
  const String& clsName,
  const String& funcName,
  ActRec* ar,
  StringData*& invName,
  DecodeFlags flags /* = DecodeFlags::Warn */) {
  int pos = funcName.find("::");
  bool nameContainsClass =
    (pos != 0 && pos != String::npos && pos + 2 < funcName.size());
  bool forwarding = false;
  HPHP::Class* ctx = nullptr;
  if (ar) ctx = arGetContextClass(ar);
  auto cls = vm_decode_class_from_name(
    clsName, funcName, nameContainsClass, ar, forwarding, ctx, flags);
  if (!cls) {
    if (flags == DecodeFlags::Warn) {
      throw_invalid_argument("function: class not found");
    }
    return std::make_pair(nullptr, nullptr);
  }

  // For clsmeth, we want to return the class user gave,
  // not the class where func is associated with
  auto c = cls;
  ObjectData* thiz = nullptr;
  auto const func = vm_decode_func_from_name(
    funcName, ar, forwarding, thiz, c, ctx, c, invName, flags);
  return std::make_pair(cls, const_cast<Func*>(func));
}

const HPHP::Func*
vm_decode_function(const_variant_ref function,
                   ActRec* ar,
                   ObjectData*& this_,
                   HPHP::Class*& cls,
                   StringData*& invName,
                   bool& dynamic,
                   DecodeFlags flags /* = DecodeFlags::Warn */,
                   bool genericsAlreadyGiven /* = false */) {
  bool forwarding = false;
  invName = nullptr;
  dynamic = true;

  if (function.isFunc()) {
    dynamic = false;
    this_ = nullptr;
    cls = nullptr;
    return function.toFuncVal();
  }

  if (function.isClsMeth()) {
    dynamic = false;
    this_ = nullptr;
    auto const clsmeth = function.toClsMethVal();
    cls = clsmeth->getCls();
    return clsmeth->getFunc();
  }

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
      assertx(function.isArray());
      Array arr = function.toArray();
      if (!array_is_valid_callback(arr)) {
        if (flags == DecodeFlags::Warn) {
          throw_invalid_argument("function: not a valid callback array");
        }
        return nullptr;
      }

      Variant elem0 = arr[0];
      Variant elem1 = arr[1];
      if (elem1.isFunc()) {
        if (elem0.isObject()) {
          this_ = elem0.getObjectData();
          cls = this_->getVMClass();
        } else if (elem0.isClass()) {
          cls = elem0.toClassVal();
        } else {
          raise_error("calling an ill-formed array without resolved "
                      "object/class pointer");
          return nullptr;
        }
        return elem1.toFuncVal();
      }

      assertx(elem1.isString());
      name = elem1.toString();
      pos = name.find("::");
      nameContainsClass =
        (pos != 0 && pos != String::npos && pos + 2 < name.size());
      if (elem0.isString()) {
        cls = vm_decode_class_from_name(
          elem0.toString(), name, nameContainsClass, ar, forwarding, ctx,
          flags);
        if (!cls) {
          if (flags == DecodeFlags::Warn) {
            throw_invalid_argument("function: class not found");
          }
          return nullptr;
        }
      } else if (elem0.isClass()) {
        cls = elem0.toClassVal();
      } else {
        assertx(elem0.isObject());
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
        if (ar && ar->func()->cls()) {
          if (ar->hasThis()) {
            cc = ar->getThis()->getVMClass();
          } else {
            cc = ar->getClass();
          }
        }
        if (flags != DecodeFlags::NoWarn && cc) {
          if (RuntimeOption::EvalWarnOnSkipFrameLookup) {
            raise_warning(
              "vm_decode_function() used to decode a LSB class "
              "method on %s",
              cc->name()->data()
            );
          }
        }
      } else {
        cc = Unit::loadClass(c.get());
      }
      if (!cc) {
        if (flags == DecodeFlags::Warn) {
          throw_invalid_argument("function: class not found");
        }
        return nullptr;
      }
      if (cls) {
        if (!cls->classof(cc)) {
          if (flags == DecodeFlags::Warn) {
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
        if (flags == DecodeFlags::Warn) {
          throw_invalid_argument("function: method '%s' not found",
                                 name.data());
        }
        return nullptr;
      }
      assertx(f && f->preClass() == nullptr);
      if (f->hasReifiedGenerics() && !genericsAlreadyGiven) {
        throw_invalid_argument("You may not call the reified function '%s' "
                               "without reified arguments",
                               f->fullName()->data());
        return nullptr;
      }
      return f;
    }
    assertx(cls);
    return vm_decode_func_from_name(
      name, ar, forwarding, this_, cls, ctx, cc, invName, flags);
  }
  if (function.isObject()) {
    this_ = function.toCObjRef().get();
    cls = nullptr;
    dynamic = false;
    const HPHP::Func *f = this_->getVMClass()->lookupMethod(s___invoke.get());
    if (f != nullptr && f->isStaticInPrologue()) {
      // If __invoke is static, invoke it as such
      cls = this_->getVMClass();
      this_ = nullptr;
    }
    if (flags == DecodeFlags::Warn && f == nullptr) {
      raise_warning("call_user_func() expects parameter 1 to be a valid "
                    "callback, object of class %s given (no __invoke "
                    "method found)", this_->getVMClass()->name()->data());
    }
    return f;
  }
  if (flags == DecodeFlags::Warn) {
    throw_invalid_argument("function: not string, closure, or array");
  }
  return nullptr;
}

Variant vm_call_user_func(const_variant_ref function, const Variant& params,
                          bool checkRef /* = false */,
                          bool allowDynCallNoPointer /* = false */) {
  CallCtx ctx;
  vm_decode_function(function, ctx);
  if (ctx.func == nullptr || (!isContainer(params) && !params.isNull())) {
    return uninit_null();
  }
  auto ret = Variant::attach(
    g_context->invokeFunc(ctx.func, params, ctx.this_, ctx.cls,
                          ctx.invName, ctx.dynamic, checkRef,
                          allowDynCallNoPointer)
  );
  if (UNLIKELY(isRefType(ret.getRawType()))) {
    tvUnbox(*ret.asTypedValue());
  }
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

static Variant
invoke(const String& function, const Variant& params, strhash_t /*hash*/,
       bool /*tryInterp*/, bool fatal /* = true */,
       bool allowDynCallNoPointer /* = false */) {
  Func* func = Unit::loadFunc(function.get());
  if (func && (isContainer(params) || params.isNull())) {
    auto ret = Variant::attach(
      g_context->invokeFunc(func, params, nullptr, nullptr, nullptr, true,
                            false, allowDynCallNoPointer)

    );
    if (UNLIKELY(isRefType(ret.getRawType()))) {
      tvUnbox(*ret.asTypedValue());
    }
    return ret;
  }
  return invoke_failed(function.c_str(), fatal);
}

// Declared in externals.h.  If you're considering calling this
// function for some new code, please reconsider.
Variant invoke(const char *function, const Variant& params, strhash_t hash /* = -1 */,
               bool tryInterp /* = true */, bool fatal /* = true */,
               bool allowDynCallNoPointer /* = false */) {
  String funcName(function, CopyString);
  return invoke(funcName, params, hash, tryInterp, fatal,
                allowDynCallNoPointer);
}

Variant invoke_static_method(const String& s, const String& method,
                             const Variant& params, bool fatal /* = true */) {
  HPHP::Class* class_ = Unit::lookupClass(s.get());
  if (class_ == nullptr) {
    o_invoke_failed(s.data(), method.data(), fatal);
    return uninit_null();
  }
  const HPHP::Func* f = class_->lookupMethod(method.get());
  if (f == nullptr || !f->isStaticInPrologue() ||
    (!isContainer(params) && !params.isNull())) {
    o_invoke_failed(s.data(), method.data(), fatal);
    return uninit_null();
  }
  auto ret = Variant::attach(
    g_context->invokeFunc(f, params, nullptr, class_)
  );
  if (UNLIKELY(isRefType(ret.getRawType()))) {
    tvUnbox(*ret.asTypedValue());
  }
  return ret;
}

Variant o_invoke_failed(const char *cls, const char *meth,
                        bool fatal /* = true */) {
   if (fatal) {
    string msg = "Unknown method ";
    msg += cls;
    msg += "::";
    msg += meth;
    raise_fatal_error(msg.c_str());
  } else {
    raise_warning("call_user_func to non-existent method %s::%s", cls, meth);
    return false;
  }
}

template<class EF, class NF>
void missing_this_check_helper(const Func* f, EF ef, NF nf) {
  if (f->attrs() & AttrRequiresThis) {
    ef();
    return;
  }

  if (!f->isStatic()) {
    nf();
    return;
  }
}

void throw_has_this_need_static(const Func* f) {
  auto const msg = folly::sformat(
    "Static method {}() cannot be called on instance",
    f->fullName()->data()
  );
  SystemLib::throwBadMethodCallExceptionObject(msg);
}

void throw_missing_this(const Func* f) {
  auto const msg = folly::sformat(
    "Non-static method {}() cannot be called statically",
    f->fullName()->data()
  );
  SystemLib::throwBadMethodCallExceptionObject(msg);
}

void NEVER_INLINE throw_invalid_property_name(const String& name) {
  if (!name.size()) {
    raise_error("Cannot access empty property");
  }
  raise_error("Cannot access property started with '\\0'");
}

void throw_instance_method_fatal(const char *name) {
  raise_error("Non-static method %s() cannot be called statically", name);
}

void throw_call_reified_func_without_generics(const Func* f) {
  auto const msg = folly::sformat(
    "Cannot call the reified function '{}' without the reified generics",
    f->fullName()->data()
  );
  SystemLib::throwBadMethodCallExceptionObject(msg);
}

void throw_iterator_not_valid() {
  SystemLib::throwInvalidOperationExceptionObject(
    "Iterator is not valid");
}

void throw_collection_property_exception() {
  SystemLib::throwInvalidOperationExceptionObject(
    "Cannot access a property on a collection");
}

void throw_invalid_collection_parameter() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Parameter must be an array or an instance of Traversable");
}

void throw_invalid_operation_exception(StringData* str) {
  SystemLib::throwInvalidOperationExceptionObject(Variant{str});
}

void throw_arithmetic_error(StringData* str) {
  SystemLib::throwArithmeticErrorObject(Variant{str});
}

void throw_division_by_zero_error(StringData *str) {
  SystemLib::throwDivisionByZeroErrorObject(Variant{str});
}

void throw_division_by_zero_exception() {
  SystemLib::throwDivisionByZeroExceptionObject();
}

void throw_collection_compare_exception() {
  SystemLib::throwInvalidOperationExceptionObject(s_cmpWithCollection);
}

void throw_vec_compare_exception() {
  SystemLib::throwInvalidOperationExceptionObject(s_cmpWithVec);
}

void throw_dict_compare_exception() {
  SystemLib::throwInvalidOperationExceptionObject(s_cmpWithDict);
}

void throw_record_compare_exception() {
  SystemLib::throwInvalidOperationExceptionObject(s_cmpWithRecord);
}

void throw_rec_non_rec_compare_exception() {
  SystemLib::throwInvalidOperationExceptionObject(s_cmpWithNonRecord);
}

void throw_keyset_compare_exception() {
  SystemLib::throwInvalidOperationExceptionObject(s_cmpWithKeyset);
}

void throw_clsmeth_compare_exception() {
  SystemLib::throwInvalidOperationExceptionObject(s_cmpWithClsMeth);
}

void throw_param_is_not_container() {
  static const string msg("Parameter must be an array or collection");
  SystemLib::throwInvalidArgumentExceptionObject(msg);
}

void throw_invalid_inout_base() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Parameters marked inout must be contained in locals, vecs, dicts, "
    "keysets, and arrays"
  );
}

void throw_cannot_modify_immutable_object(const char* className) {
  auto msg = folly::sformat(
    "Cannot modify immutable object of type {}",
    className
  );
  SystemLib::throwInvalidOperationExceptionObject(msg);
}

void throw_cannot_modify_const_object(const char* className) {
  auto msg = folly::sformat(
    "Cannot modify const object of type {}",
    className
  );
  SystemLib::throwInvalidOperationExceptionObject(msg);
}

void throw_object_forbids_dynamic_props(const char* className) {
  auto msg = folly::sformat(
    "Class {} does not allow use of dynamic (non-declared) properties",
    className
  );
  SystemLib::throwInvalidOperationExceptionObject(msg);
}

void throw_cannot_modify_const_prop(const char* className,
                                    const char* propName)
{
  auto msg = folly::sformat(
    "Cannot modify const property {} of class {} after construction",
    propName, className
  );
  SystemLib::throwInvalidOperationExceptionObject(msg);
}

void throw_cannot_modify_static_const_prop(const char* className,
                                           const char* propName)
{
  auto msg = folly::sformat(
   "Cannot modify static const property {} of class {}.",
   propName, className
  );
  SystemLib::throwInvalidOperationExceptionObject(msg);
}

NEVER_INLINE
void throw_late_init_prop(const Class* cls,
                          const StringData* propName,
                          bool isSProp) {
  SystemLib::throwInvalidOperationExceptionObject(
    folly::sformat(
      "Accessing <<__LateInit>> {} '{}::{}' before initialization",
      isSProp ? "static property" : "property",
      cls->name(),
      propName
    )
  );
}

NEVER_INLINE
void throw_parameter_wrong_type(TypedValue tv,
                                const Func* callee,
                                unsigned int arg_num,
                                DataType expected_type) {
  auto msg = param_type_error_message(
    callee->displayName()->data(), arg_num, expected_type,
    type(tv));

  if (RuntimeOption::PHP7_EngineExceptions) {
    SystemLib::throwTypeErrorObject(msg);
  }
  SystemLib::throwRuntimeExceptionObject(msg);
}

void raise_soft_late_init_prop(const Class* cls,
                               const StringData* propName,
                               bool isSProp) {
  raise_notice(
    "Accessing <<__SoftLateInit>> %s '%s::%s' before initialization. "
    "Providing default.",
    isSProp ? "static property" : "property",
    cls->name()->data(),
    propName->data()
  );
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
  return Object::attach(g_context->createObjectOnly(s.get()));
}

Object init_object(const String& s, const Array& params, ObjectData* o) {
  return Object{g_context->initObject(s.get(), params, o)};
}

Object
create_object(const String& s, const Array& params, bool init /* = true */) {
  return Object::attach(g_context->createObject(s.get(), params, init));
}

void throw_object(const Object& e) {
  throw req::root<Object>(e);
}

#if ((__GNUC__ != 4) || (__GNUC_MINOR__ != 8))
void throw_object(Object&& e) {
  throw req::root<Object>(std::move(e));
}
#endif

/*
 * This function is used when another thread is segfaulting---we just
 * want to wait forever to give it a chance to write a stacktrace file
 * (and maybe a core file).
 */
void pause_forever() {
  for (;;) sleep(300);
}

bool is_constructor_name(const char* fn) {
  auto len = strlen(fn);
  const char construct[] = "__construct";
  auto clen = sizeof(construct) - 1;

  if (len >= clen && !strcasecmp(fn + len - clen, construct)) {
    if (len == clen || (len > clen + 2 &&
                        fn[len - clen - 1] == ':' &&
                        fn[len - clen - 2] == ':')) {
      return true;
    }
  }
  return false;
}

void throw_missing_arguments_nr(const char *fn, int expected, int got) {
  SystemLib::throwRuntimeExceptionObject(folly::sformat(
    "{}() expects exactly {} parameter{}, {} given",
    fn,
    expected,
    expected == 1 ? "" : "s",
    got
  ));
}

void throw_bad_type_exception(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string msg;
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  raise_warning("Invalid operand type was used: %s", msg.c_str());
}

void throw_expected_array_exception(const char* fn /*=nullptr*/) {
  if (!fn) {
    if (auto ar = g_context->getStackFrame()) {
     fn = ar->m_func->name()->data();
    } else {
     fn = "(unknown)";
    }
  }
  throw_bad_type_exception("%s expects array(s)", fn);
}

void throw_expected_array_or_collection_exception(const char* fn /*=nullptr*/) {
  if (!fn) {
    if (auto ar = g_context->getStackFrame()) {
      fn = ar->m_func->name()->data();
    } else {
      fn = "(unknown)";
    }
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

Variant unserialize_ex(const char* str, int len,
                       VariableUnserializer::Type type,
                       const Array& options /* = null_array */) {
  if (str == nullptr || len <= 0) {
    return false;
  }

  VariableUnserializer vu(str, len, type, true, options);
  Variant v;
  try {
    v = vu.unserialize();
  } catch (FatalErrorException& e) {
    throw;
  } catch (InvalidAllowedClassesException& e) {
    raise_warning(
      "unserialize(): allowed_classes option should be array or boolean"
    );
    return false;
  } catch (Exception& e) {
    raise_notice("Unable to unserialize: [%.*s]. %s.",
                 std::min(len, 1000), str, e.getMessage().c_str());
    return false;
  }
  return v;
}

Variant unserialize_ex(const String& str,
                       VariableUnserializer::Type type,
                       const Array& options /* = null_array */) {
  return unserialize_ex(str.data(), str.size(), type, options);
}

String concat3(const String& s1, const String& s2, const String& s3) {
  auto r1 = s1.slice();
  auto r2 = s2.slice();
  auto r3 = s3.slice();
  auto len = r1.size() + r2.size() + r3.size();
  auto str = String::attach(StringData::Make(len));
  auto const r = str.mutableData();
  memcpy(r,                         r1.data(), r1.size());
  memcpy(r + r1.size(),             r2.data(), r2.size());
  memcpy(r + r1.size() + r2.size(), r3.data(), r3.size());
  str.setSize(len);
  return str;
}

String concat4(const String& s1, const String& s2, const String& s3,
               const String& s4) {
  auto r1 = s1.slice();
  auto r2 = s2.slice();
  auto r3 = s3.slice();
  auto r4 = s4.slice();
  auto len = r1.size() + r2.size() + r3.size() + r4.size();
  auto str = String::attach(StringData::Make(len));
  auto const r = str.mutableData();
  memcpy(r,                                     r1.data(), r1.size());
  memcpy(r + r1.size(),                         r2.data(), r2.size());
  memcpy(r + r1.size() + r2.size(),             r3.data(), r3.size());
  memcpy(r + r1.size() + r2.size() + r3.size(), r4.data(), r4.size());
  str.setSize(len);
  return str;
}

static bool invoke_file_impl(Variant& res, const String& path, bool once,
                             const char *currentDir,
                             bool callByHPHPInvoke) {
  bool initial;
  auto const u = lookupUnit(path.get(), currentDir, &initial,
                            Native::s_noNativeFuncs, false);
  if (u == nullptr) return false;
  if (!once || initial) {
    *res.asTypedValue() = g_context->invokeUnit(u, callByHPHPInvoke);
  }
  return true;
}

static NEVER_INLINE Variant throw_missing_file(const char* file) {
  throw PhpFileDoesNotExistException(file);
}

static Variant invoke_file(const String& s,
                           bool once,
                           const char *currentDir,
                           bool callByHPHPInvoke) {
  Variant r;
  if (invoke_file_impl(r, s, once, currentDir, callByHPHPInvoke)) {
    return r;
  }
  return throw_missing_file(s.c_str());
}

Variant include_impl_invoke(const String& file, bool once,
                            const char *currentDir, bool callByHPHPInvoke) {
  if (FileUtil::isAbsolutePath(file.toCppString())) {
    if (RuntimeOption::SandboxMode || !RuntimeOption::AlwaysUseRelativePath) {
      try {
        return invoke_file(file, once, currentDir, callByHPHPInvoke);
      } catch(PhpFileDoesNotExistException& e) {}
    }

    try {
      String rel_path(FileUtil::relativePath(RuntimeOption::SourceRoot,
                                           string(file.data())));

      // Don't try/catch - We want the exception to be passed along
      return invoke_file(rel_path, once, currentDir, callByHPHPInvoke);
    } catch(PhpFileDoesNotExistException& e) {
      throw PhpFileDoesNotExistException(file.c_str());
    }
  } else {
    // Don't try/catch - We want the exception to be passed along
    return invoke_file(file, once, currentDir, callByHPHPInvoke);
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

  auto const getCwd = [] () -> String {
    if (LIKELY(!g_context.isNull())) return g_context->getCwd();
    return String(Process::CurrentWorkingDirectory, CopyString);
  };

  if (!File::IsPlainFilePath(file)) {
    // URIs don't have an include path
    if (tryFile(file, ctx)) {
      return file;
    }

  } else if (FileUtil::isAbsolutePath(file.toCppString())) {
    String can_path = FileUtil::canonicalize(file);

    if (tryFile(can_path, ctx)) {
      return can_path;
    }

  } else if ((c_file[0] == '.' && (c_file[1] == '/' || (
    c_file[1] == '.' && c_file[2] == '/')))) {

    String path(String(getCwd() + "/" + file));
    String can_path = FileUtil::canonicalize(path);

    if (tryFile(can_path, ctx)) {
      return can_path;
    }

  } else {
    if (!RequestInfo::s_requestInfo.isNull()) {
      auto const& includePaths = RID().getIncludePaths();

      for (auto const& includePath : includePaths) {
        String path("");
        auto const is_stream_wrapper =
          includePath.find("://") != std::string::npos;

        if (!is_stream_wrapper && !FileUtil::isAbsolutePath(includePath)) {
          path += (getCwd() + "/");
        }

        path += includePath;

        if (path[path.size() - 1] != '/') {
          path += "/";
        }

        path += file;

        String can_path;
        if (!is_stream_wrapper) {
          can_path = FileUtil::canonicalize(path);
        } else {
          can_path = String(path.c_str());
        }

        if (tryFile(can_path, ctx)) {
          return can_path;
        }
      }
    }

    if (FileUtil::isAbsolutePath(currentDir)) {
      String path(currentDir);
      path += "/";
      path += file;
      String can_path = FileUtil::canonicalize(path);

      if (tryFile(can_path, ctx)) {
        return can_path;
      }
    } else {
      String path(getCwd() + "/" + currentDir + file);
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
      raise_fatal_error(ms.data());
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
         (f->arFuncPtr() != Native::unimplementedWrapper);
}

///////////////////////////////////////////////////////////////////////////////
// debugger and code coverage instrumentation

void throw_exception(const Object& e) {
  if (!e.instanceof(SystemLib::s_ThrowableClass)) {
    raise_error("Exceptions must implement the Throwable interface.");
  }
  DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionThrownHook(e.get()));
  throw req::root<Object>(e);
}

///////////////////////////////////////////////////////////////////////////////
}
