/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/reflection/ext_reflection.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-hash-set.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/native-prop-handler.h"

#include "hphp/runtime/server/source-root-info.h"

#include "hphp/runtime/ext/debugger/ext_debugger.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/core/ext_core_closure.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/extension-registry.h"

#include "hphp/system/systemlib.h"

#include <functional>
#include <boost/algorithm/string/predicate.hpp>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_name("name"),
  s___name("__name"),
  s_version("version"),
  s_info("info"),
  s_ini("ini"),
  s_constants("constants"),
  s_constructor("constructor"),
  s_functions("functions"),
  s_classes("classes"),
  s_access("access"),
  s_public("public"),
  s_protected("protected"),
  s_private("private"),
  s_file("file"),
  s_line1("line1"),
  s_line2("line2"),
  s_doc("doc"),
  s_modifiers("modifiers"),
  s_class("class"),
  s_prototype("prototype"),
  s_ref("ref"),
  s_inout("inout"),
  s_readonly("readonly"),
  s_index("index"),
  s_type("type"),
  s_nullable("nullable"),
  s_msg("msg"),
  s_is_optional("is_optional"),
  s_is_variadic("is_variadic"),
  s_default("default"),
  s_defaultValue("defaultValue"),
  s_defaultText("defaultText"),
  s_params("params"),
  s_final("final"),
  s_abstract("abstract"),
  s_instantiable("instantiable"),
  s_internal("internal"),
  s_is_async("is_async"),
  s_is_closure("is_closure"),
  s_is_generator("is_generator"),
  s_hphp("hphp"),
  s_static_variables("static_variables"),
  s_extension("extension"),
  s_interfaces("interfaces"),
  s_traits("traits"),
  s_interface("interface"),
  s_trait("trait"),
  s_methods("methods"),
  s_properties("properties"),
  s_private_properties("private_properties"),
  s_properties_index("properties_index"),
  s_private_properties_index("private_properties_index"),
  s_attributes("attributes"),
  s_function("function"),
  s_varg("varg"),
  s___invoke("__invoke"),
  s_return_type("return_type"),
  s_accessible("accessible"),
  s_type_hint("type_hint"),
  s_type_hint_builtin("type_hint_builtin"),
  s_type_hint_nullable("type_hint_nullable"),
  s_is_reified("is_reified"),
  s_is_soft("is_soft"),
  s_is_warn("is_warn"),
  s___construct("__construct");

Class* get_cls(const Variant& class_or_object) {
  if (class_or_object.is(KindOfObject)) {
    return class_or_object.asCObjRef()->getVMClass();
  } else if (class_or_object.isClass()) {
    return class_or_object.toClassVal();
  }

  return Class::load(class_or_object.toString().get());
}

const Func* get_method_func(const Class* cls, const String& meth_name) {
  const Func* func = cls->lookupMethod(meth_name.get());
  if (!func) {
    if (cls->attrs() & (AttrInterface | AttrAbstract  | AttrTrait)) {
      const Class::InterfaceMap& ifaces = cls->allInterfaces();
      for (int i = 0, size = ifaces.size(); i < size; i++) {
        func = ifaces[i]->lookupMethod(meth_name.get());
        if (func) { break; }
      }
    }
  }
  assertx(func == nullptr || func->isMethod());
  return func;
}

Variant default_arg_from_php_code(const Func::ParamInfo& fpi,
                                  const Func* func, unsigned argIdx) {
  assertx(fpi.hasDefaultValue());
  if (fpi.hasScalarDefaultValue()) {
    // Most of the time the default value is scalar, so we can
    // avoid evaling in the common case
    return tvAsVariant((TypedValue*)&fpi.defaultValue);
  }
  // Eval PHP code to get the default value. Note that eval here can throw a
  // fatal, e.g. due to the use of undefined class constants or static:: .
  // When this happens, instead of fataling the execution, set the default
  // value to null and raise a warning.
  auto const on_eval_exn = [&] {
    raise_warning("Failed to eval default value of %s()'s $%s argument for "
                  "reflection, assigning NULL instead",
                  func->fullNameStr().data(),
                  func->localNames()[argIdx]->data());
    return init_null_variant;
  };

  try {
    return g_context->getEvaledArg(
      fpi.phpCode,
      // We use cls() instead of implCls() because we want the namespace and
      // class context for which the closure is scoped, not that of the
      // Closure subclass (which, among other things, is always globally
      // namespaced).
      func->cls() ? func->cls()->nameStr() : func->nameStr(),
      func->unit()
    );
  } catch (const Exception&) {
    return on_eval_exn();
  } catch (const Object&) {
    if (RO::EvalFixDefaultArgReflection > 1) {
      return on_eval_exn();
    } else {
      if (RO::EvalFixDefaultArgReflection > 0) {
        raise_notice("Evaluation of default arg initializer in func=%s, arg=%s "
                     "threw a PHP exception--this reflection call won't throw "
                     "in future HHVMs!",
                     func->fullNameStr().data(),
                     func->localNames()[argIdx]->data());
      }
      throw;
    }
  }
}

Array HHVM_FUNCTION(hphp_get_extension_info, const String& name) {

  Extension *ext = ExtensionRegistry::get(name);

  return make_dict_array(
    s_name,      name,
    s_version,   ext ? ext->getVersion() : "",
    s_info,      empty_string(),
    s_ini,       empty_dict_array(),
    s_constants, empty_dict_array(),
    s_functions, empty_dict_array(),
    s_classes,   empty_dict_array()
  );
}

int get_modifiers(Attr attrs, bool cls, bool prop) {
  int php_modifier = 0;
  if (!prop) {
    // These bits have different meanings with properties
    if (attrs & AttrAbstract)  php_modifier |= cls ? 0x20 : 0x02;
    if (attrs & AttrFinal)     php_modifier |= cls ? 0x40 : 0x04;
  }
  if (!cls) {  // AttrPublic bits are not valid on class (have other meaning)
    if (attrs & AttrStatic)    php_modifier |= 0x01;
    if (attrs & AttrPublic)    php_modifier |= 0x100;
    if (attrs & AttrProtected) php_modifier |= 0x200;
    if (attrs & AttrPrivate)   php_modifier |= 0x400;
  }
  return php_modifier;
}

ALWAYS_INLINE
static Attr attrs_from_modifiers(int php_modifier, bool cls) {
  Attr attrs = (Attr) 0;
  if (php_modifier & (cls ? 0x20 : 0x02)) {
    attrs = (Attr)(attrs | AttrAbstract);
  }
  if (php_modifier & (cls ? 0x40 : 0x04)) {
    attrs = (Attr)(attrs | AttrFinal);
  }
  if (php_modifier & 0x01) { attrs = (Attr)(attrs | AttrStatic); }
  if (!cls) {  // AttrPublic bits are not valid on class (have other meaning)
    if (php_modifier & 0x100) { attrs = (Attr)(attrs | AttrPublic); }
    if (php_modifier & 0x200) { attrs = (Attr)(attrs | AttrProtected); }
    if (php_modifier & 0x400) { attrs = (Attr)(attrs | AttrPrivate); }
  }
  return attrs;
}

static void set_attrs(Array& ret, int modifiers) {
  if (modifiers & 0x100) {
    ret.set(s_access, VarNR(s_public).tv());
    ret.set(s_accessible, make_tv<KindOfBoolean>(true));
  } else if (modifiers & 0x200) {
    ret.set(s_access, VarNR(s_protected).tv());
    ret.set(s_accessible, make_tv<KindOfBoolean>(false));
  } else if (modifiers & 0x400) {
    ret.set(s_access, VarNR(s_private).tv());
    ret.set(s_accessible, make_tv<KindOfBoolean>(false));
  } else {
    assertx(false);
  }
  ret.set(s_modifiers, make_tv<KindOfInt64>(modifiers));
  if (modifiers & 0x1) {
    ret.set(s_static, make_tv<KindOfBoolean>(true));
  }
  if (modifiers & 0x44) {
    ret.set(s_final, make_tv<KindOfBoolean>(true));
  }
  if (modifiers & 0x22) {
    ret.set(s_abstract, make_tv<KindOfBoolean>(true));
  }
}

static void set_empty_doc_comment(Array& ret) {
  ret.set(s_doc, make_tv<KindOfBoolean>(false));
}

static void set_doc_comment(Array& ret,
                            const StringData* comment,
                            bool isBuiltin) {
  assertx(ret.isDict());
  if (comment == nullptr || comment->empty()) {
    set_empty_doc_comment(ret);
  } else if (isBuiltin && !HHVM_FUNCTION(hphp_debugger_attached)) {
    set_empty_doc_comment(ret);
  } else {
    assertx(!comment->isRefCounted());
    ret.set(s_doc, make_tv<KindOfPersistentString>(comment));
  }
}

static void set_instance_prop_info(Array& ret,
                                   const Class::Prop* prop,
                                   TypedValue default_val) {
  assertx(ret.isDict());
  ret.set(s_name, make_tv<KindOfPersistentString>(prop->name));
  ret.set(s_default, make_tv<KindOfBoolean>(true));
  ret.set(s_defaultValue, default_val);
  set_attrs(ret, get_modifiers(prop->attrs, false, true) & ~0x66);
  ret.set(s_class, make_tv<KindOfPersistentString>(prop->cls->name()));
  set_doc_comment(ret, prop->preProp->docComment(), prop->cls->isBuiltin());

  auto const user_type = prop->preProp->userType();
  if (user_type && user_type->size()) {
    ret.set(s_type, make_tv<KindOfPersistentString>(user_type));
  } else {
    ret.set(s_type, make_tv<KindOfBoolean>(false));
  }
}

static void set_dyn_prop_info(
    Array &ret,
    TypedValue name,
    const StringData* className) {
  assertx(ret.isDict());
  ret.set(s_name, name);
  set_attrs(ret, get_modifiers(AttrPublic, false, true) & ~0x66);
  ret.set(s_class, make_tv<KindOfPersistentString>(className));
  set_empty_doc_comment(ret);
  ret.set(s_type, make_tv<KindOfBoolean>(false));
}

static void set_static_prop_info(Array &ret, const Class::SProp* prop) {
  assertx(ret.isDict());
  ret.set(s_name, make_tv<KindOfPersistentString>(prop->name));
  ret.set(s_default, make_tv<KindOfBoolean>(true));
  ret.set(s_defaultValue, prop->val);
  set_attrs(ret, get_modifiers(prop->attrs, false, true) & ~0x66);
  ret.set(s_class, make_tv<KindOfPersistentString>(prop->cls->name()));
  set_doc_comment(ret, prop->preProp->docComment(), prop->cls->isBuiltin());
  auto const user_type = prop->preProp->userType();
  if (user_type && user_type->size()) {
    ret.set(s_type, make_tv<KindOfPersistentString>(user_type));
  } else {
    ret.set(s_type, make_tv<KindOfBoolean>(false));
  }
}

static bool resolveConstant(const char *p, int64_t len, Variant &cns) {
  // ltrim
  while (len && (*p == ' ')) {
    p++;
    len--;
  }
  // rtrim
  while (len && (p[len-1] == ' ')) {
    len--;
  }

  String cname(p, len, CopyString);

  if (!HHVM_FN(defined)(cname)) {
    cns = uninit_null();
    return false;
  }

  cns = HHVM_FN(constant)(cname);
  return true;
}

static bool isConstructor(const Func* func) {
  PreClass* pcls = func->preClass();
  if (!pcls || (pcls->attrs() & AttrInterface)) { return false; }
  if (func->implCls()) { return func == func->implCls()->getCtor(); }
  if (func->name() == s___construct.get()) { return true; }
  /* A same named function is not a constructor in a trait */
  if (pcls->attrs() & AttrTrait) return false;
  return pcls->name()->isame(func->name());
}

static const Class* get_prototype_class_from_interfaces(const Class *cls,
                                                        const Func *func) {
  // only looks at the interfaces if the method is public
  if (!func->isPublic()) return nullptr;
  const Class::InterfaceMap& interfaces = cls->allInterfaces();
  for (unsigned int i = 0, size = interfaces.size(); i < size; i++) {
    const Class* iface = interfaces[i];
    if (iface->preClass()->hasMethod(func->name())) return iface;
  }
  return nullptr;
}

Variant HHVM_FUNCTION(hphp_invoke, const String& name, const Variant& params) {
  return invoke(name, params);
}

static const StaticString s_invoke_not_instanceof_error(
  "Given object is not an instance of the class this method was declared in"
);

static const StaticString s_invoke_non_object(
  "Non-object passed to Invoke()"
);

static const StaticString s_canonical_class_in_repo_mode(
  "Do not attempt to get Canonical class in repo mode, it won't work as expected"
);

Variant HHVM_FUNCTION(hphp_invoke_method, const Variant& obj,
                                          const String& cls,
                                          const String& name,
                                          const Variant& params) {
  if (obj.isNull()) {
    return invoke_static_method(cls, name, params);
  }

  if (!obj.is(KindOfObject)) {
    Reflection::ThrowReflectionExceptionObject(s_invoke_non_object);
  }

  auto const providedClass = Class::load(cls.get());
  if (!providedClass) {
    raise_error("Call to undefined method %s::%s()", cls.data(), name.data());
  }
  auto const selectedFunc = providedClass->lookupMethod(name.get());
  if (!selectedFunc) {
    raise_error("Call to undefined method %s::%s()", cls.data(), name.data());
  }

  auto const objData = obj.asCObjRef().get();
  auto const implementingClass = selectedFunc->implCls();
  if (!objData->instanceof(implementingClass)) {
    Reflection::ThrowReflectionExceptionObject(s_invoke_not_instanceof_error);
  }

  // Get the CallCtx this way instead of using vm_decode_function() because
  // vm_decode_function() has no way to specify a class independent from the
  // class::function being called.
  // Note that this breaks the rules for name lookup (for protected and private)
  // but that's okay because so does Zend's implementation.
  CallCtx ctx;
  ctx.cls = providedClass;
  ctx.this_ = objData;
  ctx.func = selectedFunc;
  ctx.dynamic = true;

  return Variant::attach(
    g_context->invokeFunc(ctx, params, RuntimeCoeffects::fixme())
  );
}

Object HHVM_FUNCTION(hphp_create_object, const String& name,
                     const Variant& params) {
  return Object::attach(g_context->createObject(name.get(), params));
}

Object HHVM_FUNCTION(hphp_create_object_without_constructor,
                      const String& name) {
  return Object::attach(
    g_context->createObject(name.get(), init_null_variant, false)
  );
}

Variant HHVM_FUNCTION(hphp_get_property, const Object& obj, const String& cls,
                                         const String& prop) {
  /* It's possible to get a ReflectionProperty for a property which
   * no longer exists.  Silentyly fail to match PHP5 behavior
   */
  return obj->o_get(prop, false /* error */, cls);
}

void HHVM_FUNCTION(hphp_set_property, const Object& obj, const String& cls,
                                      const String& prop, const Variant& value) {
  if (!cls.empty() && RuntimeOption::EvalAuthoritativeMode) {
    raise_error(
      "We've already made many assumptions about private variables. "
      "You can't change accessibility in Whole Program mode"
    );
  }

  obj->o_set(prop, value, cls);
}

Variant HHVM_FUNCTION(hphp_get_static_property, const String& cls,
                                                const String& prop,
                                                bool force) {
  auto const sd = cls.get();
  auto const class_ = Class::lookup(sd);
  if (!class_) {
    raise_error("Non-existent class %s", sd->data());
  }
  VMRegAnchor _;
  CoeffectsAutoGuard _2;

  auto const lookup = class_->getSPropIgnoreLateInit(
    force ?
      // since force is true, set the MemberLookupContext so that
      // the module boundary check always succeeds
      MemberLookupContext(class_, class_->moduleName())
    : MemberLookupContext(arGetContextClass(vmfp()), vmfp()->func()),
    prop.get()
  );
  if (!lookup.val) {
    raise_error("Class %s does not have a property named %s",
                sd->data(), prop.get()->data());
  }
  if (!lookup.accessible) {
    raise_error("Invalid access to class %s's property %s",
                sd->data(), prop.get()->data());
  }
  return tvAsVariant(lookup.val);
}

void HHVM_FUNCTION(hphp_set_static_property, const String& cls,
                                             const String& prop,
                                             const Variant& value,
                                             bool force) {
  if (RuntimeOption::EvalAuthoritativeMode) {
    raise_error("Setting static properties through reflection is not "
      "allowed in RepoAuthoritative mode");
  }
  auto const sd = cls.get();
  auto const class_ = Class::lookup(sd);

  if (!class_) raise_error("Non-existent class %s", sd->data());

  VMRegAnchor _;
  CoeffectsAutoGuard _2;

  auto const ctx = force ?
    // since force is true, set the MemberLookupContext so that the module
    // boundary check will always succeed if prop is defined in class_
    MemberLookupContext(class_, class_->moduleName())
  : MemberLookupContext(arGetContextClass(vmfp()), vmfp()->func());
  auto const lookup = class_->getSPropIgnoreLateInit(
    ctx,
    prop.get()
  );
  if (!lookup.val) {
    raise_error("Class %s does not have a property named %s",
                sd->data(), prop.get()->data());
  }
  if (!lookup.accessible) {
    raise_error("Invalid access to class %s's property %s",
                sd->data(), prop.get()->data());
  }
  if (lookup.constant) {
    throw_cannot_modify_static_const_prop(sd->data(), prop.get()->data());
  }

  auto const& sprop = class_->staticProperties()[lookup.slot];
  auto const& tc = sprop.typeConstraint;
  // TODO(T61738946): We can remove the temporary here once we no longer coerce
  // class_meth types.
  auto tmp = value;
  if (RuntimeOption::EvalCheckPropTypeHints > 0 && tc.isCheckable()) {
    tc.verifyStaticProperty(tmp.asTypedValue(), class_, sprop.cls, prop.get());
  }
  tvAsVariant(lookup.val) = tmp;
}

namespace {

const StaticString s_classname("classname");

Array implTypeStructure(const Variant& cls_or_obj,
                        const Variant& cns_name,
                        bool no_throw) {
  SuppressClassConversionNotice suppressor;
  auto const cns_sd = cns_name.getStringDataOrNull();
  if (!cns_sd) {
    auto name = cls_or_obj.toString();

    auto const typeAlias = TypeAlias::load(name.get());

    if (!typeAlias) {
      raise_error("Non-existent type alias %s", name.get()->data());
    }

    auto const& preresolved = typeAlias->resolvedTypeStructure();
    if (!preresolved.isNull()) {
      assertx(!preresolved.empty());
      assertx(preresolved.isDict());
      return preresolved;
    }

    auto const& typeStructure = typeAlias->typeStructure();
    assertx(!typeStructure.empty());
    assertx(typeStructure.isDict());
    Array resolved;
    try {
      bool persistent = true;
      resolved = TypeStructure::resolve(name, typeStructure, persistent);
    } catch (Exception& ) {
      raise_error("resolving type alias %s failed. "
                  "Have you declared all classes in the type alias",
                  name.get()->data());
    }
    assertx(!resolved.empty());
    assertx(resolved.isDict());
    return resolved;
  }

  auto const cls = get_cls(cls_or_obj);

  if (!cls) {
    raise_error("Class undefined: %s", cls_or_obj.toString().get()->data());
  }

  auto const cls_sd = cls->name();

  if (!cls->hasTypeConstant(cns_sd, true)) {
    raise_error("Non-existent type constant %s::%s",
                cls_sd->data(), cns_sd->data());
  }

  auto ad = jit::loadClsTypeCnsHelper(cls, cns_sd, no_throw);
  return Array::attach(ad);
}

struct ReflectionException : SystemLib::ClassLoader<"ReflectionException"> {};

} // namespace

/*
 * cls_or_obj: the name of a class or an instance of the class;
 * cns_name: the name of the type constant of the class;
 *
 * If the type constant exists and is not abstract, this function
 * returns the shape representing the type associated with the type
 * constant.
 */
Array HHVM_FUNCTION(type_structure,
                    const Variant& cls_or_obj, const Variant& cns_name) {
  return implTypeStructure(cls_or_obj, cns_name, false);
}

Array HHVM_FUNCTION(type_structure_no_throw,
                    const Variant& cls_or_obj, const Variant& cns_name) {
  return implTypeStructure(cls_or_obj, cns_name, true);
}

String HHVM_FUNCTION(type_structure_classname,
                     const Variant& cls_or_obj, const Variant& cns_name) {
  auto const ts = implTypeStructure(cls_or_obj, cns_name, false);
  auto const classname = ts->get(s_classname.get(), true);
  assertx(isStringType(type(classname)));
  assertx(val(classname).pstr->isStatic());
  return String::attach(val(classname).pstr);
}

[[noreturn]]
void Reflection::ThrowReflectionExceptionObject(const Variant& message) {
  auto cls = ReflectionException::classof();
  Object inst { cls };
  tvDecRefGen(
    g_context->invokeFunc(cls->getCtor(),
                          make_vec_array(message),
                          inst.get())
  );
  throw_object(inst);
}


/////////////////////////////////////////////////////////////////////////////
// class ReflectionFuncHandle

static TypedValue HHVM_METHOD(ReflectionFunctionAbstract, getFileName) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  if (func->isBuiltin()) {
    return make_tv<KindOfBoolean>(false);
  }
  auto file = func->unit()->filepath();
  if (!file) file = staticEmptyString();
  if (file->data()[0] != '/') {
    auto path = SourceRootInfo::RelativeToPhpRoot(StrNR(file));
    return make_tv<KindOfString>(path.detach());
  }
  assertx(!file->isRefCounted());
  return make_tv<KindOfPersistentString>(file);
}

static Variant HHVM_METHOD(ReflectionFunctionAbstract, getStartLine) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  if (func->isBuiltin()) {
    return false;
  }
  return func->line1();
}

static Variant HHVM_METHOD(ReflectionFunctionAbstract, getEndLine) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  if (func->isBuiltin()) {
    return false;
  }
  return func->line2();
}

static TypedValue HHVM_METHOD(ReflectionFunctionAbstract, getDocComment) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  auto const comment = func->docComment();
  if (comment == nullptr || comment->empty()) {
    return make_tv<KindOfBoolean>(false);
  } else {
    assertx(!comment->isRefCounted());
    return make_tv<KindOfPersistentString>(const_cast<StringData*>(comment));
  }
}

static String HHVM_METHOD(ReflectionFunctionAbstract, getName) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  auto ret = const_cast<StringData*>(func->name());
  return String(ret);
}

static bool HHVM_METHOD(ReflectionFunctionAbstract, isHack) {
  return true;
}

static bool HHVM_METHOD(ReflectionFunctionAbstract, isInternal) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->isBuiltin();
}

static bool HHVM_METHOD(ReflectionFunctionAbstract, isGenerator) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->isGenerator();
}

static bool HHVM_METHOD(ReflectionFunctionAbstract, isAsync) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->isAsync();
}

static bool HHVM_METHOD(ReflectionFunctionAbstract, isInternalToModule) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->isInternal();
}

static bool HHVM_METHOD(ReflectionFunctionAbstract, isVariadic) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->hasVariadicCaptureParam();
}

static int64_t HHVM_METHOD(ReflectionFunctionAbstract, getNumberOfParameters) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->numParams();
}

ALWAYS_INLINE
static Array get_function_param_info(const Func* func) {
  const Func::ParamInfoVec& params = func->params();
  VecInit ai(func->numParams());

  for (int i = 0; i < func->numParams(); ++i) {
    Array param = Array::CreateDict();
    const Func::ParamInfo& fpi = params[i];

    param.set(s_index, make_tv<KindOfInt64>(i));
    param.set(s_name, make_tv<KindOfPersistentString>(func->localNames()[i]));

    auto const nonExtendedConstraint =
      fpi.typeConstraint.hasConstraint() &&
      !fpi.typeConstraint.isExtended();
    auto const type = nonExtendedConstraint
      ? fpi.typeConstraint.typeName()
      : staticEmptyString();

    param.set(s_type, make_tv<KindOfPersistentString>(type));
    const StringData* typeHint = fpi.userType
      ? fpi.userType
      : staticEmptyString();
    param.set(s_type_hint, make_tv<KindOfPersistentString>(typeHint));

    // callable typehint considered builtin; stdClass typehint is not
    auto const isBuiltinTC =
      fpi.typeConstraint.isCallable() || fpi.typeConstraint.isPrecise();
    param.set(s_type_hint_builtin, make_tv<KindOfBoolean>(isBuiltinTC));

    param.set(s_function, make_tv<KindOfPersistentString>(func->name()));
    if (func->preClass()) {
      param.set(
        s_class,
        make_tv<KindOfPersistentString>(
          func->implCls() ? func->implCls()->name()
                          : func->preClass()->name()
        )
      );
    }
    if (!nonExtendedConstraint || fpi.typeConstraint.isNullable()) {
      param.set(s_nullable, make_tv<KindOfBoolean>(true));
      param.set(s_type_hint_nullable, make_tv<KindOfBoolean>(true));
    } else {
      param.set(s_type_hint_nullable, make_tv<KindOfBoolean>(false));
    }

    if (fpi.phpCode) {
      Variant v = default_arg_from_php_code(fpi, func, i);
      param.set(s_default, v);
      param.set(s_defaultText, make_tv<KindOfPersistentString>(fpi.phpCode));
    }

    if (func->isInOut(i)) {
      param.set(s_inout, make_tv<KindOfBoolean>(true));
    }
    if (func->isReadonly(i)) {
      param.set(s_readonly, make_tv<KindOfBoolean>(true));
    }
    if (fpi.isVariadic()) {
      param.set(s_is_variadic, make_tv<KindOfBoolean>(true));
    }
    {
      DictInit userAttrs(fpi.userAttributes.size());
      for (auto const& attr : fpi.userAttributes) {
        userAttrs.set(StrNR{attr.first}, attr.second);
      }
      param.set(s_attributes, make_array_like_tv(userAttrs.create()));
    }
    ai.append(VarNR(param).tv());
  }

  auto arr = ai.toArray();

  bool isOptional = true;
  for (int i = func->numParams() - 1; i >= 0; i--) {
    auto& param = asArrRef(arr.lval(i));

    isOptional = isOptional && (param.exists(s_default) ||
                 param.exists(s_is_variadic));
    param.set(s_is_optional, isOptional);
  }
  return arr;
}

// helper for getParameters
static Array HHVM_METHOD(ReflectionFunctionAbstract, getParamInfo) {
  // FIXME: each parameter info should be HNI with a handle to the
  // Func::ParamInfo
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return get_function_param_info(func);
}

// helper for getReturnTypeText
static String HHVM_METHOD(ReflectionFunctionAbstract, getReturnTypeHint) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  auto retTypeSD = func->returnUserType();
  if (retTypeSD && retTypeSD->size()) {
    auto ret = const_cast<StringData*>(retTypeSD);
    return String(ret);
  }
  return String();
}

static Array HHVM_METHOD(ReflectionFunctionAbstract, getRetTypeInfo) {
  DictInit retTypeInfo{3};
  auto name = HHVM_MN(ReflectionFunctionAbstract, getReturnTypeHint)(this_);
  if (name && !name.empty()) {
    auto const func = ReflectionFuncHandle::GetFuncFor(this_);
    auto retType = func->returnTypeConstraint();
    if (retType.isNullable()) {
      retTypeInfo.set(s_type_hint_nullable, make_tv<KindOfBoolean>(true));
    } else {
      retTypeInfo.set(s_type_hint_nullable, make_tv<KindOfBoolean>(false));
    }

    auto const isBuiltinTC = retType.isCallable() || retType.isPrecise();
    retTypeInfo.set(s_type_hint_builtin, make_tv<KindOfBoolean>(isBuiltinTC));
  } else {
    name = staticEmptyString();
    retTypeInfo.set(s_type_hint_nullable, make_tv<KindOfBoolean>(false));
    retTypeInfo.set(s_type_hint_builtin, make_tv<KindOfBoolean>(false));
  }
  retTypeInfo.set(s_type_hint, name);
  return retTypeInfo.toArray();
}

namespace {

const Array reified_generics_info_to_array(const ReifiedGenericsInfo& info) {
  VecInit arr(info.m_typeParamInfo.size());
  for (auto tparam : info.m_typeParamInfo) {
    DictInit tparamArr(3);
    tparamArr.set(s_is_reified, make_tv<KindOfBoolean>(tparam.m_isReified));
    tparamArr.set(s_is_soft, make_tv<KindOfBoolean>(tparam.m_isSoft));
    tparamArr.set(s_is_warn, make_tv<KindOfBoolean>(tparam.m_isWarn));
    arr.append(tparamArr.toArray());
  }
  return arr.toArray();
}

} // namespace

static Array HHVM_METHOD(ReflectionFunctionAbstract, getReifiedTypeParamInfo) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return reified_generics_info_to_array(func->getReifiedGenericsInfo());
}

const StaticString s_pure("pure");
const StaticString s_defaults("defaults");

static Array HHVM_METHOD(ReflectionFunctionAbstract, getCoeffects) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  std::vector<LowStringPtr> result;
  for (auto const& name : func->staticCoeffectNames()) {
    if (name->equal(s_pure.get())) continue;
    result.push_back(name);
  }
  if (func->staticCoeffectNames().empty()) {
    result.push_back(makeStaticString(s_defaults.get()));
  }
  if (func->hasCoeffectRules()) {
    for (auto const& rule : func->getCoeffectRules()) {
      auto const name = rule.toString(func);
      if (name) result.push_back(makeStaticString(*name));
    }
  }
  VecInit arr(result.size());
  for (auto& name : result) arr.append(make_tv<KindOfPersistentString>(name));
  return arr.toArray();
}

static Variant HHVM_METHOD(ReflectionFunctionAbstract, getModule) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  auto const name = func->moduleName();
  if (!name || Module::isDefault(name)) return init_null_variant;
  return String::attach(const_cast<StringData*>(name));
}

static bool HHVM_METHOD(ReflectionFunctionAbstract, returnsReadonly) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->attrs() & AttrReadonlyReturn;
}

const StaticString
  s_Memoize("__Memoize"),
  s_MemoizeLSB("__MemoizeLSB"),
  s_systemlib_create_opaque_value("__SystemLib\\create_opaque_value"),
  s_KeyedByIC("KeyedByIC"),
  s_MakeICInaccessible("MakeICInaccessible"),
  s_SoftMakeICInaccessible("SoftMakeICInaccessible"),
  s_Uncategorized("Uncategorized");

ALWAYS_INLINE
static Array get_function_user_attributes(const Func* func) {
  auto userAttrs = func->userAttributes();

  DictInit ai(userAttrs.size());
  for (auto it = userAttrs.begin(); it != userAttrs.end(); ++it) {
    auto const attrName = StrNR(it->first).asString();
    // __Memoize and LSB attributes may contain EnumClassLabels
    if (func->isMemoizeWrapper() &&
        (attrName.get()->isame(s_Memoize.get()) ||
         attrName.get()->isame(s_MemoizeLSB.get()))) {
      assertx(tvIsVec(it->second));
      auto const ad = it->second.m_data.parr;
      VecInit args(ad->size());
      IterateV(ad, [&] (TypedValue tv) {
        if (!tvIsString(tv)) {
          args.append(tv);
        } else {
          auto const sd = tv.m_data.pstr;
          if (sd->same(s_KeyedByIC.get()) ||
              sd->same(s_MakeICInaccessible.get()) ||
              sd->same(s_SoftMakeICInaccessible.get()) ||
              sd->same(s_Uncategorized.get())) {
            if (RO::EvalEmitNativeEnumClassLabels) {
              args.append(make_tv<KindOfEnumClassLabel>(sd));
            } else {
              auto const func = Func::load(s_systemlib_create_opaque_value.get());
              assertx(func);
              VecInit v(2);
              // From ext_hh.php: __SystemLib\OpaqueValueId::EnumClassLabel
              v.append(make_tv<KindOfInt64>(0));
              v.append(tv);
              args.append(g_context->invokeFunc(
                func, v.toArray(), nullptr, nullptr,
                RuntimeCoeffects::pure(), false /* dynamic */
              ));
            }
          }
        }
      });
      ai.set(attrName, args.toArray());
    } else {
      ai.set(attrName, it->second);
    }
  }
  return ai.toArray();
}

static Array HHVM_METHOD(ReflectionFunctionAbstract, getAttributesNamespaced) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return get_function_user_attributes(func);
}

// ------------------------- class ReflectionMethod

// helper for __construct
static bool HHVM_METHOD(ReflectionMethod, __init,
                        const Variant& cls_or_object, const String& meth_name) {
  auto const cls = get_cls(cls_or_object);
  if (!cls || meth_name.isNull()) {
    // caller raises exception
    return false;
  }
  auto const func = get_method_func(cls, meth_name);
  if (!func) {
    // caller raises exception
    return false;
  }
  assertx(func->isMethod());
  ReflectionFuncHandle::Get(this_)->setFunc(func);
  return true;
}

static bool HHVM_METHOD(ReflectionMethod, isFinal) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->attrs() & AttrFinal;
}

static bool HHVM_METHOD(ReflectionMethod, isAbstract) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->attrs() & AttrAbstract;
}

static bool HHVM_METHOD(ReflectionMethod, isPublic) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->attrs() & AttrPublic;
}

static bool HHVM_METHOD(ReflectionMethod, isProtected) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->attrs() & AttrProtected;
}

static bool HHVM_METHOD(ReflectionMethod, isPrivate) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->attrs() & AttrPrivate;
}

static bool HHVM_METHOD(ReflectionMethod, isStatic) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->attrs() & AttrStatic;
}

static bool HHVM_METHOD(ReflectionMethod, isStaticInPrologue) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->isStaticInPrologue();
}

static bool HHVM_METHOD(ReflectionMethod, isConstructor) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return isConstructor(func);
}

static bool HHVM_METHOD(ReflectionMethod, isReadonly) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->attrs() & AttrReadonlyThis;
}

static int64_t HHVM_METHOD(ReflectionMethod, getModifiers) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return get_modifiers(func->attrs(), false, false);
}

static String HHVM_METHOD(ReflectionMethod, getCanonicalClassname) {
  if (RuntimeOption::EvalAuthoritativeMode) {
    Reflection::ThrowReflectionExceptionObject(s_canonical_class_in_repo_mode);
  }
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return String::attach(const_cast<StringData*>(func->preClass()->name()));
}

// private helper for getPrototype
static String HHVM_METHOD(ReflectionMethod, getPrototypeClassname) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  const Class *prototypeCls = nullptr;
  if (func->baseCls() != nullptr && func->baseCls() != func->implCls()) {
    prototypeCls = func->baseCls();
    const Class *result = get_prototype_class_from_interfaces(
      prototypeCls, func);
    if (result) { prototypeCls = result; }
  } else if (func->isMethod()) {
    // lookup the prototype in the interfaces
    prototypeCls = get_prototype_class_from_interfaces(func->implCls(), func);
  }
  if (prototypeCls) {
    auto ret = const_cast<StringData*>(prototypeCls->name());
    return String(ret);
  }
  return String();
}

// private helper for getDeclaringClass
static String HHVM_METHOD(ReflectionMethod, getDeclaringClassname) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  auto ret = const_cast<StringData*>(func->implCls()->name());
  return String(ret);
}

// ------------------------- class ReflectionFile

// helper for __construct
static String HHVM_METHOD(ReflectionFile, __init, const String& name) {
  if (name.isNull()) {
    Reflection::ThrowReflectionExceptionObject(
      "Tried to construct ReflectionFile but the name was null"
    );
  }

  const Unit* unit = lookupUnit(
    File::TranslatePath(name).get(),
    "",
    nullptr,
    nullptr,
    false
  );

  if (!unit) {
    Reflection::ThrowReflectionExceptionObject(folly::sformat(
      "File '{}' does not exist",
      name
    ));
  }

  ReflectionFileHandle::Get(this_)->setUnit(unit);

  return String::attach(const_cast<StringData*>(unit->filepath()));
}

static Array HHVM_METHOD(ReflectionFile, getAttributesNamespaced) {
  auto const unit = ReflectionFileHandle::GetUnitFor(this_);
  assertx(unit);

  auto fileAttrs = unit->fileAttributes();
  DictInit ai(fileAttrs.size());

  for (auto it = fileAttrs.begin(); it != fileAttrs.end(); ++it) {
    ai.set(StrNR(it->first), tvAsCVarRef(&it->second));
  }
  return ai.toArray();
}

// ------------------------- class ReflectionModule

// helper for __construct
static void HHVM_METHOD(ReflectionModule, __init, const String& name) {
  if (name.isNull()) {
    Reflection::ThrowReflectionExceptionObject(
      "Tried to construct ReflectionModule but the name was null"
    );
  }

  const Module* module = Module::load(makeStaticString(name.get()));
  if (!module) {
    Reflection::ThrowReflectionExceptionObject(folly::sformat(
      "Module '{}' does not exist",
      name
    ));
  }

  ReflectionModuleHandle::Get(this_)->setModule(module);
}

static Array HHVM_METHOD(ReflectionModule, getAttributesNamespaced) {
  auto const module = ReflectionModuleHandle::GetModuleFor(this_);
  assertx(module);

  auto userAttrs = module->userAttributes;
  DictInit ai(userAttrs.size());
  for (auto const& attr : userAttrs) {
    ai.set(StrNR(attr.first), attr.second);
  }
  return ai.toArray();
}

static TypedValue HHVM_METHOD(ReflectionModule, getDocComment) {
  auto const module = ReflectionModuleHandle::GetModuleFor(this_);
  assertx(module);

  auto const comment = module->docComment;
  if (comment == nullptr || comment->empty()) {
    return make_tv<KindOfBoolean>(false);
  } else {
    assertx(!comment->isRefCounted());
    return make_tv<KindOfPersistentString>(comment);
  }
}

static Array get_module_ruleset(const Module::RuleSet& ruleset) {
  VecInit arr((ruleset.global_rule ? 1 : 0) + ruleset.name_rules.size());

  if (ruleset.global_rule) {
    arr.append(make_tv<KindOfPersistentString>(makeStaticString("global")));
  }

  for (auto const& nr : ruleset.name_rules) {
    std::vector<folly::StringPiece> names;

    for (auto& n : nr.names) {
      names.push_back(n->slice());
    }

    std::string name;
    folly::join(".", names, name);

    if (nr.prefix) name.append(".*");

    arr.append(make_tv<KindOfPersistentString>(makeStaticString(name)));
    name.clear();
  }

  return arr.toArray();
}

static Variant HHVM_METHOD(ReflectionModule, getExports) {
  auto const module = ReflectionModuleHandle::GetModuleFor(this_);
  assertx(module);

  if (!module->exports) return init_null_variant;

  return get_module_ruleset(*module->exports);
}

static Variant HHVM_METHOD(ReflectionModule, getImports) {
  auto const module = ReflectionModuleHandle::GetModuleFor(this_);
  assertx(module);

  if (!module->imports) return init_null_variant;

  return get_module_ruleset(*module->imports);
}

// ------------------------- class ReflectionFunction

// helper for __construct
static bool HHVM_METHOD(ReflectionFunction, __initName, const String& name) {
  if (name.isNull()) { return false; }
  const Func* func = Func::load(name.get());
  if (!func) { return false; }
  ReflectionFuncHandle::Get(this_)->setFunc(func);
  return true;
}

// helper for __construct
static bool HHVM_METHOD(ReflectionFunction, __initClosure,
                        const Object& closure) {
  auto const cls = get_cls(closure);
  assertx(cls);
  if (!cls) { return false; }
  const Func* func = cls->lookupMethod(s___invoke.get());
  if (!func) {
    // caller raises exception
    return false;
  }
  assertx(func->isClosureBody());
  assertx(func->implCls()->isScopedClosure());
  ReflectionFuncHandle::Get(this_)->setFunc(func);
  return true;
}

const StaticString s_ExpectedClosureInstance("Expected closure instance");

// helper for getClosureScopeClass
static Variant HHVM_METHOD(ReflectionFunction, getClosureScopeClassname,
                           const Object& closure) {
  if (!closure->instanceof(c_Closure::classof())) {
    SystemLib::throwExceptionObject(s_ExpectedClosureInstance);
  }
  if (auto scope = c_Closure::fromObject(closure.get())->getScope()) {
    return String(const_cast<StringData*>(scope->name()));
  }
  return init_null_variant;
}

static Variant HHVM_METHOD(ReflectionFunction, getClosureThisObject,
                           const Object& closure) {
  if (!closure->instanceof(c_Closure::classof())) {
    SystemLib::throwExceptionObject(s_ExpectedClosureInstance);
  }
  auto const clos = c_Closure::fromObject(closure.get());
  if (clos->hasThis()) {
    return Object{clos->getThis()};
  }
  return init_null_variant;
}

/////////////////////////////////////////////////////////////////////////////
// class ReflectionClass

// helper for __construct
static String HHVM_METHOD(ReflectionClass, __init, const String& name) {
  return ReflectionClassHandle::Get(this_)->init(name);
}

static String HHVM_METHOD(ReflectionClass, getName) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->nameStr();
}

static String HHVM_METHOD(ReflectionClass, getParentName) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->parentStr();
}

static bool HHVM_METHOD(ReflectionClass, isHack) {
  return true;
}

static bool HHVM_METHOD(ReflectionClass, isInternal) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->attrs() & AttrBuiltin;
}

static bool HHVM_METHOD(ReflectionClass, isInstantiable) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return !(cls->attrs() & (AttrAbstract | AttrInterface | AttrTrait | AttrEnum |
                           AttrEnumClass))
    && (cls->getCtor()->attrs() & AttrPublic);
}

static bool HHVM_METHOD(ReflectionClass, isFinal) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->attrs() & AttrFinal;
}

static bool HHVM_METHOD(ReflectionClass, isInternalToModule) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->isInternal();
}

static Variant HHVM_METHOD(ReflectionClass, getModule) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  auto const name = cls->moduleName();
  if (!name || Module::isDefault(name)) return init_null_variant;
  return String::attach(const_cast<StringData*>(name));
}

static bool HHVM_METHOD(ReflectionClass, isAbstract) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->attrs() & AttrAbstract;
}

static bool HHVM_METHOD(ReflectionClass, isInterface) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->attrs() & AttrInterface;
}

static bool HHVM_METHOD(ReflectionClass, isTrait) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->attrs() & AttrTrait;
}

static bool HHVM_METHOD(ReflectionClass, isEnum) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->attrs() & (AttrEnum|AttrEnumClass);
}

static String HHVM_METHOD(ReflectionClass, getEnumUnderlyingType) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  if (!(cls->attrs() & (AttrEnum|AttrEnumClass))) {
    Reflection::ThrowReflectionExceptionObject(
      "Trying to read the Enum-type of a non-Enum");
  }

  // use the unresolved preclass type to yield the userland types
  return StrNR(cls->preClass()->enumBaseTy().typeName());
}

static int64_t HHVM_METHOD(ReflectionClass, getModifiers) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return get_modifiers(cls->attrs(), true, false);
}

static TypedValue HHVM_METHOD(ReflectionClass, getFileName) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  if (cls->attrs() & AttrBuiltin) {
    return make_tv<KindOfBoolean>(false);
  }
  auto file = cls->preClass()->unit()->filepath();
  if (!file) file = staticEmptyString();
  if (file->data()[0] != '/') {
    auto path = SourceRootInfo::RelativeToPhpRoot(StrNR(file));
    return make_tv<KindOfString>(path.detach());
  }
  assertx(!file->isRefCounted());
  return make_tv<KindOfPersistentString>(file);
}

static Variant HHVM_METHOD(ReflectionClass, getStartLine) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  if (cls->isBuiltin()) {
    return false;
  }
  return cls->preClass()->line1();
}

static Variant HHVM_METHOD(ReflectionClass, getEndLine) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  if (cls->isBuiltin()) {
    return false;
  }
  return cls->preClass()->line2();
}

static TypedValue HHVM_METHOD(ReflectionClass, getDocComment) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  auto const pcls = cls->preClass();
  auto const comment = pcls->docComment();
  if (comment == nullptr || comment->empty()) {
    return make_tv<KindOfBoolean>(false);
  } else {
    assertx(!comment->isRefCounted());
    return make_tv<KindOfPersistentString>(const_cast<StringData*>(comment));
  }
}

static Array HHVM_METHOD(ReflectionClass, getRequirementNames) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  if (!(cls->attrs() & (AttrTrait | AttrInterface))) {
    // requirements are applied to abstract/concrete classes when they use
    // a trait / implement an interface
    return empty_vec_array();
  }

  auto const& requirements = cls->allRequirements();
  auto numReqs = requirements.size();
  if (numReqs == 0) {
    return empty_vec_array();
  }

  VecInit pai(numReqs);
  for (int i = 0; i < numReqs; ++i) {
    auto const& req = requirements[i];
    pai.append(Variant{const_cast<StringData*>(req->name())});
  }
  return pai.toArray();
}

static Variant HHVM_METHOD(ReflectionClass, getRequiredClass) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  if (!isTrait(cls)) {
    // require class only applies to traits
    return init_null_variant;
  }

  auto const& requirements = cls->allRequirements();
  if (requirements.empty()) {
    return init_null_variant;
  }

  auto const& requirementsRange = requirements.range();

  assertx(
    std::count_if(
      requirementsRange.begin(),
      requirementsRange.end(),
      [](auto const& req) {
        return req->kind() == PreClass::RequirementKind::RequirementClass;
      }) <= 1
  );

  for (auto const& req: requirementsRange) {
    if (req->kind() == PreClass::RequirementKind::RequirementClass) {
      return String::attach(const_cast<StringData*>(req->name()));
    }
  }
  return init_null_variant;
}

static Array HHVM_METHOD(ReflectionClass, getInterfaceNames) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);

  auto st = req::make<c_Set>();
  auto const& allIfaces = cls->allInterfaces();
  st->reserve(allIfaces.size());

  for (auto const& interface: cls->declInterfaces()) {
    st->add(const_cast<StringData*>(interface->name()));
  }
  if (allIfaces.size() > cls->declInterfaces().size()) {
    for (int i = 0; i < allIfaces.size(); ++i) {
      auto const& interface = allIfaces[i];
      st->add(const_cast<StringData*>(interface->name()));
    }
  }
  return st->toValuesArray();
}

static Array HHVM_METHOD(ReflectionClass, getTraitNames) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  auto const& traits = cls->preClass()->usedTraits();
  VecInit ai(traits.size());
  for (const StringData* traitName : traits) {
    ai.append(Variant{const_cast<StringData*>(traitName)});
  }
  return ai.toArray();
}

static bool HHVM_METHOD(ReflectionClass, hasMethod, const String& name) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return (get_method_func(cls, name) != nullptr);
}

namespace {
  const Class* get_class_from_name(const String& name) {
    auto const cls = Class::load(name.get());
    if (!cls) {
      auto message = folly::sformat(
        "class {} could not be loaded",
        name.toCppString()
      );
      Reflection::ThrowReflectionExceptionObject(message);
    }
    return cls;
  }
}

// helper for getMethods: returns a keyset
static Array HHVM_STATIC_METHOD(
  ReflectionClass,
  getMethodOrder,
  const String& clsname,
  int64_t filter) {
  auto const cls = get_class_from_name(clsname);
  Attr mask = attrs_from_modifiers(filter, false);

  // At each step, we fetch from the PreClass is important because the
  // order in which getMethods returns matters
  req::StringFastSet visitedMethods;
  req::StringIFastSet visitedInterfaces;
  auto st = Array::CreateKeyset();

  auto add = [&] (const Func* m) {
    if (m->isGenerated()) return;
    if (!visitedMethods.insert(m->nameStr()).second) return;
    if (m->attrs() & mask) {
      st.append(m->nameStr().asString());
    }
  };

  std::function<void(const Class*)> collect;
  std::function<void(const Class*)> collectInterface;

  collect = [&] (const Class* clas) {
    if (!clas) return;

    auto const methods = clas->preClass()->methods();
    auto const numMethods = clas->preClass()->numMethods();

    auto numDeclMethods = clas->preClass()->numDeclMethods();
    if (numDeclMethods == -1) numDeclMethods = numMethods;

    // Add declared methods.
    for (Slot i = 0; i < numDeclMethods; ++i) {
      add(methods[i]);
    }

    // Recurse; we need to order the parent's methods before our trait methods.
    collect(clas->parent());

    for (Slot i = numDeclMethods; i < numMethods; ++i) {
      // For repo mode, where trait methods are flattened at compile-time.
      add(methods[i]);
    }
    for (Slot i = clas->traitsBeginIdx(); i < clas->traitsEndIdx(); ++i) {
      // For non-repo mode, where they are added at Class-creation time.
      add(clas->getMethod(i));
    }
  };

  collectInterface = [&] (const Class* iface) {
    if (!iface) return;
    if (!visitedInterfaces.insert(iface->nameStr()).second) return;

    size_t const numMethods = iface->preClass()->numMethods();
    Func* const* methods = iface->preClass()->methods();
    for (Slot i = 0; i < numMethods; ++i) {
      add(methods[i]);
    }

    for (auto const& parentIface: iface->declInterfaces()) {
      collectInterface(parentIface.get());
    }
    auto const& allIfaces = iface->allInterfaces();
    if (allIfaces.size() > iface->declInterfaces().size()) {
      for (int i = 0; i < allIfaces.size(); ++i) {
        collectInterface(allIfaces[i].get());
      }
    }
  };

  collect(const_cast<Class*>(cls));

  // concrete classes should already have all of their methods present
  if (((AttrPublic | AttrAbstract | AttrStatic) & mask) &&
      cls->attrs() & (AttrInterface | AttrAbstract | AttrTrait)) {
    for (auto const& interface: cls->declInterfaces()) {
      collectInterface(interface.get());
    }
    auto const& allIfaces = cls->allInterfaces();
    if (allIfaces.size() > cls->declInterfaces().size()) {
      for (int i = 0; i < allIfaces.size(); ++i) {
        auto const& interface = allIfaces[i];
        collectInterface(interface.get());
      }
    }
  }
  return st;
}

static bool HHVM_METHOD(ReflectionClass, hasConstant, const String& name) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->hasConstant(name.get());
}

static TypedValue HHVM_METHOD(ReflectionClass, getConstant, const String& name) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  auto value = cls->clsCnsGet(name.get());
  if (value.m_type == KindOfUninit) return make_tv<KindOfBoolean>(false);
  tvIncRefGen(value);
  return value;
}

static
void addClassConstantNames(const Class* cls,
                           const req::ptr<c_Set>& st,
                           size_t limit,
                           hphp_fast_set<const Class*>& seen) {
  assertx(cls && st && (st->size() < limit));

  if (!seen.emplace(cls).second) return;

  auto numConsts = cls->numConstants();
  const Class::Const* consts = cls->constants();
  for (size_t i = 0; i < numConsts; i++) {
    if (consts[i].cls == cls && !consts[i].isAbstractAndUninit()
        && consts[i].kind() == ConstModifiers::Kind::Value) {
      st->add(const_cast<StringData*>(consts[i].name.get()));
    }
  }

  auto const& allTraits = cls->usedTraitClasses();
  auto const numTraits = allTraits.size();
  for (int i = 0; i < numTraits && (st->size() < limit); ++i) {
    addClassConstantNames(allTraits[i].get(), st, limit, seen);
  }

  if ((st->size() < limit) && cls->parent()) {
    addClassConstantNames(cls->parent(), st, limit, seen);
  }

  auto const& allIfaces = cls->allInterfaces();
  auto const numIfaces = allIfaces.size();
  for (int i = 0; i < numIfaces && (st->size() < limit); ++i) {
    addClassConstantNames(allIfaces[i].get(), st, limit, seen);
  }
}

// helper for getConstants
static Array HHVM_STATIC_METHOD(
  ReflectionClass,
  getOrderedConstants,
  const String& clsname) {
  auto const cls = get_class_from_name(clsname);

  size_t numConsts = cls->numConstants();
  if (!numConsts) {
    return empty_dict_array();
  }

  auto st = req::make<c_Set>();
  st->reserve(numConsts);

  hphp_fast_set<const Class*> seen;
  addClassConstantNames(cls, st, numConsts, seen);
  assertx(st->size() <= numConsts);

  DictInit ai(numConsts);
  IterateV(st->arrayData(), [&](TypedValue k) {
    auto constName = val(k).pstr;
    auto value = cls->clsCnsGet(constName);
    assertx(type(value) != KindOfUninit);
    ai.set(constName, value);
  });
  return ai.toArray();
}

namespace {
// helper for getOrdered*Constants
template <typename Fn>
static Array orderedConstantsHelper(const Class* cls, Fn filterFn) {
  auto const numConsts = cls->numConstants();
  if (!numConsts) {
    return empty_dict_array();
  }

  auto st = KeysetInit{numConsts};
  auto const consts = cls->constants();
  for (size_t i = 0; i < numConsts; i++) {
    auto const& konst = consts[i];
    if (filterFn(konst)) {
      st.add(make_tv<KindOfPersistentString>(konst.name.get()));
    }
  }

  auto ret = st.create()->toDict(false /*copy*/);
  assertx(ret->size() <= numConsts);
  return Array::attach(std::move(ret));
}
}

static Array HHVM_STATIC_METHOD(
  ReflectionClass,
  getOrderedAbstractConstants,
  const String& clsname
) {
  return orderedConstantsHelper(
    get_class_from_name(clsname),
    [](Class::Const c) -> bool {
      return c.isAbstractAndUninit() && c.kind() == ConstModifiers::Kind::Value;
    }
  );
}

static Array HHVM_STATIC_METHOD(
  ReflectionClass,
  getOrderedTypeConstants,
  const String& clsname
) {
  return orderedConstantsHelper(
    get_class_from_name(clsname),
    [](Class::Const c) -> bool {
      return c.kind() == ConstModifiers::Kind::Type;
    }
  );
}

static Array HHVM_METHOD(ReflectionClass, getAttributesNamespaced) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  // UserAttributes are stored exclusively on the PreClass.
  auto const pcls = cls->preClass();

  auto userAttrs = pcls->userAttributes();
  DictInit ai(userAttrs.size());
  for (auto const& attr : userAttrs) {
    ai.set(StrNR(attr.first), attr.second);
  }
  return ai.toArray();
}

static Array HHVM_METHOD(ReflectionClass, getAttributesRecursiveNamespaced) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);

  Array ret = Array::CreateDict(); // no reasonable idea about sizing

  // UserAttributes are stored in the PreClass, so we must walk the parent
  // chain to get all of them; attribute specifications from child classes
  // win over parents.

  // const pointer to Class => pointer to a (const) Class
  Class* currentCls = const_cast<Class*>(cls);
  do {
    auto const pcls = currentCls->preClass();
    for (auto it = pcls->userAttributes().begin();
         it != pcls->userAttributes().end(); ++it) {
      if (!ret.exists(StrNR(it->first))) {
        ret.set(StrNR(it->first), it->second);
      }
    }
  } while ((currentCls = currentCls->parent()));

  return ret;
}

static Array HHVM_METHOD(ReflectionClass, getReifiedTypeParamInfo) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return reified_generics_info_to_array(cls->getReifiedGenericsInfo());
}

static Array HHVM_STATIC_METHOD(
  ReflectionClass,
  getClassPropertyInfo,
  const String& clsname) {
  /*
   * FIXME: This implementation is pretty horrible and should be rewritten
   * when ReflectionProperty is ported.
   */
  auto const cls = get_class_from_name(clsname);
  auto const properties = cls->declProperties();
  cls->initialize();
  auto const& propInitVec = cls->getPropData()
    ? *cls->getPropData()
    : cls->declPropInit();

  auto ret = Array::CreateDict();
  for (auto const& declProp : properties) {
    auto slot = declProp.serializationIdx;
    auto index = cls->propSlotToIndex(slot);
    auto const& prop = properties[slot];
    auto const default_val = propInitVec[index].val.tv();
    if (((prop.attrs & AttrPrivate) == AttrPrivate) && (prop.cls != cls)) {
      continue;
    }

    auto info = Array::CreateDict();
    set_instance_prop_info(info, &prop, default_val);
    ret.set(StrNR(prop.name), VarNR(info).tv());
  }

  // static properties
  auto const sProperties = cls->staticProperties();
  for (auto const& sProp : sProperties) {
    auto slot = sProp.serializationIdx;
    auto const& prop = sProperties[slot];
    if (((prop.attrs & AttrPrivate) == AttrPrivate) && (prop.cls != cls)) {
      continue;
    }

    auto info = Array::CreateDict();
    set_static_prop_info(info, &prop);
    ret.set(StrNR(prop.name), VarNR(info).tv());
  }
  return ret;
}

static Array HHVM_METHOD(ReflectionClass, getDynamicPropertyInfos,
                         const Object& obj) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  auto obj_data = obj.get();
  assertx(obj_data->getVMClass() == cls);
  if (!obj_data->hasDynProps()) {
    return empty_dict_array();
  }

  auto const dynPropArray = obj_data->dynPropArray();
  DictInit ret{dynPropArray->size()};
  IterateKV(dynPropArray.get(), [&](TypedValue k, TypedValue) {
    if (RuntimeOption::EvalNoticeOnReadDynamicProp) {
      auto const key = tvCastToString(k);
      obj_data->raiseReadDynamicProp(key.get());
    }
    auto info = Array::CreateDict();
    set_dyn_prop_info(info, k, cls->name());
    ret.setValidKey(k, VarNR(info).tv());
  });
  return ret.toArray();
}

static String HHVM_METHOD(ReflectionClass, getConstructorName) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  auto ctor = cls->getDeclaredCtor();
  if (!ctor) { return String(); }
  auto ret = const_cast<StringData*>(ctor->name());
  return String(ret);
}

void ReflectionClassHandle::wakeup(const Variant& content, ObjectData* obj) {
  if (!content.isString()) {
    throw Exception("Native data of ReflectionClass should be a class name");
  }

  String clsName = content.toString();
  String result = init(clsName);
  if (result.empty()) {
    auto msg = folly::format("Class {} does not exist", clsName).str();
    Reflection::ThrowReflectionExceptionObject(String(msg));
  }

  // It is possible that $name does not get serialized. If a class derives
  // from ReflectionClass and the return value of its __sleep() function does
  // not contain 'name', $name gets ignored. So, we restore $name here.
  obj->setProp(nullctx, s_name.get(), result.asTypedValue());
}

namespace {
  struct ReflectionExtensionLoader :
    SystemLib::ClassLoader<"ReflectionExtension"> {};
}

static Variant reflection_extension_name_get(const Object& this_) {
  auto cls = ReflectionExtensionLoader::classof();
  auto const name = this_->getProp(
    MemberLookupContext(cls, cls->moduleName()),
    s___name.get()
  );
  return tvCastToString(name.tv());
}

static Native::PropAccessor reflection_extension_Accessors[] = {
  {"name",              reflection_extension_name_get,
                        nullptr, nullptr, nullptr}, // name is read only
  {nullptr, nullptr, nullptr, nullptr, nullptr}
};

static Native::PropAccessorMap reflection_extension_accessorsMap
((Native::PropAccessor*)reflection_extension_Accessors);

struct reflection_extension_PropHandler :
  Native::MapPropHandler<reflection_extension_PropHandler> {

  static constexpr Native::PropAccessorMap& map =
    reflection_extension_accessorsMap;
};

/////////////////////////////////////////////////////////////////////////////
// class ReflectionTypeConstant

// helper for __construct
static bool HHVM_METHOD(ReflectionTypeConstant, __init,
                        const Variant& cls_or_obj, const String& const_name) {
  auto const cls = get_cls(cls_or_obj);
  if (!cls || const_name.isNull()) {
    // caller raises exception
    return false;
  }

  size_t numConsts = cls->numConstants();
  const Class::Const* consts = cls->constants();

  for (size_t i = 0; i < numConsts; i++) {
    if (const_name.same(consts[i].name)
        && consts[i].kind() == ConstModifiers::Kind::Type) {
      auto handle = ReflectionConstHandle::Get(this_);
      handle->setConst(&consts[i]);
      handle->setClass(cls);
      return true;
    }
  }

  // caller raises exception
  return false;
}

static String HHVM_METHOD(ReflectionTypeConstant, getName) {
  auto const cns = ReflectionConstHandle::GetConstFor(this_);
  auto ret = const_cast<StringData*>(cns->name.get());
  return String(ret);
}

static bool HHVM_METHOD(ReflectionTypeConstant, isAbstract) {
  return ReflectionConstHandle::GetConstFor(this_)->isAbstract();
}

// helper for getAssignedTypeText
static String HHVM_METHOD(ReflectionTypeConstant, getAssignedTypeHint) {
  auto const cns = ReflectionConstHandle::GetConstFor(this_);

  if (isStringType(cns->val.m_type)) {
    return String(cns->val.m_data.pstr);
  }

  if (isArrayLikeType(cns->val.m_type)) {
    auto const cls = cns->cls;
    // go to the preclass to find the unresolved TypeStructure to get
    // the original assigned type text
    auto const preCls = cls->preClass();
    auto typeCns = preCls->lookupConstant(cns->name);
    assertx(typeCns->kind() == ConstModifiers::Kind::Type);
    assertx(!typeCns->isAbstractAndUninit());
    assertx(isArrayLikeType(typeCns->val().m_type));
    return TypeStructure::toString(Array::attach(typeCns->val().m_data.parr),
      TypeStructure::TSDisplayType::TSDisplayTypeReflection);
  }

  return String();
}

// private helper for getDeclaringClass
static String HHVM_METHOD(ReflectionTypeConstant, getDeclaringClassname) {
  auto const cns = ReflectionConstHandle::GetConstFor(this_);
  auto cls = cns->cls;
  auto ret = const_cast<StringData*>(cls->name());
  return String(ret);
}

// private helper for getClass
static String HHVM_METHOD(ReflectionTypeConstant, getClassname) {
  auto const cls = ReflectionConstHandle::GetClassFor(this_);
  auto ret = const_cast<StringData*>(cls->name());
  return String(ret);
}

/////////////////////////////////////////////////////////////////////////////
// class ReflectionProperty

static void HHVM_METHOD(ReflectionProperty, __construct,
                        const Variant& cls_or_obj, const String& prop_name) {
  auto const cls = get_cls(cls_or_obj);
  if (!cls) {
    Reflection::ThrowReflectionExceptionObject(folly::sformat(
      "Class {} does not exist",
      cls_or_obj.toString().toCppString()
    ));
  }
  if (prop_name.isNull()) {
    Reflection::ThrowReflectionExceptionObject(folly::sformat(
      "Property {}:: does not exist",
      cls->name()->toCppString()
    ));
  }

  auto data = Native::data<ReflectionPropHandle>(this_);
  assertx(cls);
  // cls is always nonnull, pass cls->moduleName() so that
  // module boundary checks always succeed
  auto const ctx = MemberLookupContext(cls, cls->moduleName());
  // is there a declared instance property?
  auto lookup = cls->getDeclPropSlot(ctx, prop_name.get());
  auto propIdx = lookup.slot;
  if (propIdx != kInvalidSlot) {
    auto const prop = &cls->declProperties()[propIdx];
    data->setInstanceProp(prop);
    this_->setProp(nullctx, s_class.get(),
                   make_tv<KindOfPersistentString>(prop->cls->name()));
    this_->setProp(nullctx, s_name.get(),
                   make_tv<KindOfPersistentString>(prop->name));
    return;
  }

  // is there a declared static property?
  lookup = cls->findSProp(ctx, prop_name.get());
  propIdx = lookup.slot;
  if (propIdx != kInvalidSlot) {
    auto const prop = &cls->staticProperties()[propIdx];
    data->setStaticProp(prop);
    this_->setProp(nullctx, s_class.get(),
                   make_tv<KindOfPersistentString>(prop->cls->name()));
    this_->setProp(nullctx, s_name.get(),
                   make_tv<KindOfPersistentString>(prop->name));
    return;
  }

  // is there a dynamic property?
  if (cls_or_obj.is(KindOfObject)) {
    auto obj = cls_or_obj.asCObjRef().get();
    assertx(cls == obj->getVMClass());
    if (obj->getAttribute(ObjectData::HasDynPropArr) &&
        obj->dynPropArray().exists(
          obj->dynPropArray().convertKey<IntishCast::Cast>(prop_name))
        ){
      if (RuntimeOption::EvalNoticeOnReadDynamicProp) {
        obj->raiseReadDynamicProp(prop_name.get());
      }
      data->setDynamicProp();
      this_->setProp(nullctx, s_class.get(),
                     make_tv<KindOfPersistentString>(cls->name()));
      this_->setProp(nullctx, s_name.get(), prop_name.asTypedValue());
      return;
    }
  }

  Reflection::ThrowReflectionExceptionObject(folly::sformat(
    "Property {}::{} does not exist",
    cls->name()->toCppString(),
    prop_name.toCppString()
  ));
}

namespace {

[[noreturn]] void reflection_property_internal_error() {
  raise_fatal_error("Internal error: Failed to retrieve the reflection object");
}

}

static bool HHVM_METHOD(ReflectionProperty, isPublic) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Instance:
      return data->getProp()->attrs & AttrPublic;
    case ReflectionPropHandle::Type::Static:
      return data->getSProp()->attrs & AttrPublic;
    case ReflectionPropHandle::Type::Dynamic:
      return true;
    default:
      reflection_property_internal_error();
  }
}

static bool HHVM_METHOD(ReflectionProperty, isProtected) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Instance:
      return data->getProp()->attrs & AttrProtected;
    case ReflectionPropHandle::Type::Static:
      return data->getSProp()->attrs & AttrProtected;
    case ReflectionPropHandle::Type::Dynamic:
      return false;
    default:
      reflection_property_internal_error();
  }
}

static bool HHVM_METHOD(ReflectionProperty, isPrivate) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Instance:
      return data->getProp()->attrs & AttrPrivate;
    case ReflectionPropHandle::Type::Static:
      return data->getSProp()->attrs & AttrPrivate;
    case ReflectionPropHandle::Type::Dynamic:
      return false;
    default:
      reflection_property_internal_error();
  }
}

static bool HHVM_METHOD(ReflectionProperty, isStatic) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Static:
      return true;
    case ReflectionPropHandle::Type::Instance:
    case ReflectionPropHandle::Type::Dynamic:
      return false;
    default:
      reflection_property_internal_error();
  }
}

static bool HHVM_METHOD(ReflectionProperty, isInternalToModule) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Instance:
      return data->getProp()->attrs & AttrInternal;
    case ReflectionPropHandle::Type::Static:
      return data->getSProp()->attrs & AttrInternal;
    case ReflectionPropHandle::Type::Dynamic:
      return false;
    default:
      reflection_property_internal_error();
  }
}

static bool HHVM_METHOD(ReflectionProperty, isDefault) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Instance:
    case ReflectionPropHandle::Type::Static:
      return true;
    case ReflectionPropHandle::Type::Dynamic:
      return false;
    default:
      reflection_property_internal_error();
  }
}

static int64_t HHVM_METHOD(ReflectionProperty, getModifiers) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Instance:
      return get_modifiers(data->getProp()->attrs, false, true);
    case ReflectionPropHandle::Type::Static:
      return get_modifiers(data->getSProp()->attrs, false, true);
    case ReflectionPropHandle::Type::Dynamic:
      return get_modifiers(AttrPublic, false, true);
    default:
      reflection_property_internal_error();
  }
}

static TypedValue HHVM_METHOD(ReflectionProperty, getDocComment) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  const StringData *comment = nullptr;
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Instance:
      comment = data->getProp()->preProp->docComment();
      break;
    case ReflectionPropHandle::Type::Static:
      comment = data->getSProp()->preProp->docComment();
      break;
    case ReflectionPropHandle::Type::Dynamic:
      break;
    default:
      reflection_property_internal_error();
  }
  if (comment == nullptr || comment->empty()) {
    return tvReturn(false);
  } else {
    Variant vComment{comment, Variant::PersistentStrInit{}};
    return tvReturn(std::move(vComment));
  }
}

static String HHVM_METHOD(ReflectionProperty, getTypeText) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  const StringData *type = nullptr;
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Instance:
      type = data->getProp()->preProp->userType();
      break;
    case ReflectionPropHandle::Type::Static:
      type = data->getSProp()->preProp->userType();
      break;
    case ReflectionPropHandle::Type::Dynamic:
      break;
    default:
      reflection_property_internal_error();
  }
  if (type == nullptr || type->empty()) {
    return empty_string();
  } else {
    return StrNR(type);
  }
}

static TypedValue HHVM_METHOD(ReflectionProperty, getDefaultValue) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Instance: {
      auto const prop = data->getProp();
      // We can't get propIdx from prop->idx (that's not what that is) or by
      // doing prop - prop->cls->declProperties().begin() (the prop can be in
      // the prop vector of a child class but it will always point to the class
      // it was declared in); so if we don't want to store propIdx we have to
      // look it up by name.
      auto cls = prop->cls;
      // cls is never null here, pass cls->moduleName() so that
      // module boundary checks always succeed
      auto const propCtx = MemberLookupContext(cls, cls->moduleName());
      auto lookup = cls->getDeclPropSlot(propCtx, prop->name);
      auto propSlot = lookup.slot;
      assertx(propSlot != kInvalidSlot);
      auto propIndex = cls->propSlotToIndex(propSlot);
      cls->initialize();
      auto const& propInitVec = cls->getPropData()
        ? *cls->getPropData()
        : cls->declPropInit();
      auto val = VarNR{propInitVec[propIndex].val.tv()};
      return tvReturn(val);
    }
    case ReflectionPropHandle::Type::Static: {
      auto const prop = data->getSProp();
      prop->cls->initialize();
      return tvReturn(tvAsCVarRef(&prop->val));
    }
    case ReflectionPropHandle::Type::Dynamic:
      return make_tv<KindOfNull>();
    default:
      reflection_property_internal_error();
  }
}

static Array HHVM_METHOD(ReflectionProperty, getAttributesNamespaced) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  auto attrs = Array::CreateDict();
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Instance: {
      auto const prop = data->getProp()->preProp;
      for (auto attr : prop->userAttributes()) {
        attrs.set(StrNR(attr.first), attr.second);
      }
      return attrs;
    }
    case ReflectionPropHandle::Type::Static: {
      auto const prop = data->getSProp()->preProp;
      for (auto attr : prop->userAttributes()) {
        attrs.set(StrNR(attr.first), attr.second);
      }
      return attrs;
    }
    case ReflectionPropHandle::Type::Dynamic:
      return attrs;
    default:
      reflection_property_internal_error();
  }
}

static bool HHVM_METHOD(ReflectionProperty, isReadonly) {
  auto const data = Native::data<ReflectionPropHandle>(this_);
  switch (data->getType()) {
    case ReflectionPropHandle::Type::Instance:
      return data->getProp()->attrs & AttrIsReadonly;
    case ReflectionPropHandle::Type::Static:
      return data->getSProp()->attrs & AttrIsReadonly;
    case ReflectionPropHandle::Type::Dynamic:
      return false;
    default:
      reflection_property_internal_error();
  }
}

/////////////////////////////////////////////////////////////////////////////
// class ReflectionTypeAlias

// helper for __construct:
// caller throws exception when return value is false
static String HHVM_METHOD(ReflectionTypeAlias, __init, const String& name) {
  auto const typeAlias = TypeAlias::load(name.get());

  if (!typeAlias) {
    return empty_string();
  }

  ReflectionTypeAliasHandle::Get(this_)->setTypeAlias(typeAlias);
  return String::attach(const_cast<StringData*>(typeAlias->name()));
}

static Array HHVM_METHOD(ReflectionTypeAlias, getTypeStructure) {
  auto const req = ReflectionTypeAliasHandle::GetTypeAliasFor(this_);
  assertx(req);
  auto const typeStructure = req->typeStructure();
  assertx(!typeStructure.empty());
  assertx(typeStructure.isDict());
  return typeStructure;
}

static String HHVM_METHOD(ReflectionTypeAlias, getAssignedTypeText) {
  auto const req = ReflectionTypeAliasHandle::GetTypeAliasFor(this_);
  assertx(req);
  auto const typeStructure = req->typeStructure();
  assertx(!typeStructure.empty());
  assertx(typeStructure.isDict());
  return TypeStructure::toString(typeStructure,
    TypeStructure::TSDisplayType::TSDisplayTypeReflection);
}

static Array HHVM_METHOD(ReflectionTypeAlias, getAttributesNamespaced) {
  auto const req = ReflectionTypeAliasHandle::GetTypeAliasFor(this_);
  assertx(req);
  auto const userAttrs = req->userAttrs();

  DictInit ai(userAttrs.size());
  for (auto& attr : userAttrs) {
    ai.set(StrNR(attr.first), tvAsCVarRef(&attr.second));
  }
  return ai.toArray();
}

static String HHVM_METHOD(ReflectionTypeAlias, getFileName) {
  auto const req = ReflectionTypeAliasHandle::GetTypeAliasFor(this_);
  assertx(req);
  auto file = req->unit()->filepath();
  if (!file) file = staticEmptyString();
  if (file->data()[0] != '/') {
    return SourceRootInfo::RelativeToPhpRoot(StrNR(file));
  }
  return String::attach(const_cast<StringData*>(file));
}

///////////////////////////////////////////////////////////////////////////////
struct ReflectionExtension final : Extension {
  ReflectionExtension() : Extension("reflection", "$Id$", NO_ONCALL_YET) { }
  void moduleInit() override {
    HHVM_FE(hphp_create_object);
    HHVM_FE(hphp_create_object_without_constructor);
    HHVM_FE(hphp_get_extension_info);
    HHVM_FE(hphp_get_property);
    HHVM_FE(hphp_get_static_property);
    HHVM_FE(hphp_invoke);
    HHVM_FE(hphp_invoke_method);
    HHVM_FE(hphp_set_property);
    HHVM_FE(hphp_set_static_property);
    HHVM_FALIAS(HH\\type_structure, type_structure);
    HHVM_FALIAS(HH\\type_structure_no_throw, type_structure_no_throw);
    HHVM_FALIAS(HH\\type_structure_classname, type_structure_classname);

    HHVM_ME(ReflectionFunctionAbstract, getName);
    HHVM_ME(ReflectionFunctionAbstract, isHack);
    HHVM_ME(ReflectionFunctionAbstract, isInternal);
    HHVM_ME(ReflectionFunctionAbstract, isGenerator);
    HHVM_ME(ReflectionFunctionAbstract, isAsync);
    HHVM_ME(ReflectionFunctionAbstract, isInternalToModule);
    HHVM_ME(ReflectionFunctionAbstract, isVariadic);
    HHVM_ME(ReflectionFunctionAbstract, getFileName);
    HHVM_ME(ReflectionFunctionAbstract, getStartLine);
    HHVM_ME(ReflectionFunctionAbstract, getEndLine);
    HHVM_ME(ReflectionFunctionAbstract, getDocComment);
    HHVM_ME(ReflectionFunctionAbstract, getReturnTypeHint);
    HHVM_ME(ReflectionFunctionAbstract, getNumberOfParameters);
    HHVM_ME(ReflectionFunctionAbstract, getParamInfo);
    HHVM_ME(ReflectionFunctionAbstract, getAttributesNamespaced);
    HHVM_ME(ReflectionFunctionAbstract, getRetTypeInfo);
    HHVM_ME(ReflectionFunctionAbstract, getReifiedTypeParamInfo);
    HHVM_ME(ReflectionFunctionAbstract, getCoeffects);
    HHVM_ME(ReflectionFunctionAbstract, getModule);
    HHVM_ME(ReflectionFunctionAbstract, returnsReadonly);

    HHVM_ME(ReflectionMethod, __init);
    HHVM_ME(ReflectionMethod, isFinal);
    HHVM_ME(ReflectionMethod, isAbstract);
    HHVM_ME(ReflectionMethod, isPublic);
    HHVM_ME(ReflectionMethod, isProtected);
    HHVM_ME(ReflectionMethod, isPrivate);
    HHVM_ME(ReflectionMethod, isStatic);
    HHVM_ME(ReflectionMethod, isStaticInPrologue);
    HHVM_ME(ReflectionMethod, isConstructor);
    HHVM_ME(ReflectionMethod, isReadonly);
    HHVM_ME(ReflectionMethod, getModifiers);
    HHVM_ME(ReflectionMethod, getCanonicalClassname);
    HHVM_ME(ReflectionMethod, getPrototypeClassname);
    HHVM_ME(ReflectionMethod, getDeclaringClassname);

    HHVM_ME(ReflectionFile, __init);
    HHVM_ME(ReflectionFile, getAttributesNamespaced);

    HHVM_ME(ReflectionFunction, __initName);
    HHVM_ME(ReflectionFunction, __initClosure);
    HHVM_ME(ReflectionFunction, getClosureScopeClassname);
    HHVM_ME(ReflectionFunction, getClosureThisObject);

    HHVM_ME(ReflectionModule, __init);
    HHVM_ME(ReflectionModule, getAttributesNamespaced);
    HHVM_ME(ReflectionModule, getDocComment);
    HHVM_ME(ReflectionModule, getExports);
    HHVM_ME(ReflectionModule, getImports);

    HHVM_ME(ReflectionTypeConstant, __init);
    HHVM_ME(ReflectionTypeConstant, getName);
    HHVM_ME(ReflectionTypeConstant, isAbstract);
    HHVM_ME(ReflectionTypeConstant, getAssignedTypeHint);
    HHVM_ME(ReflectionTypeConstant, getDeclaringClassname);
    HHVM_ME(ReflectionTypeConstant, getClassname);

    HHVM_ME(ReflectionProperty, __construct);
    HHVM_ME(ReflectionProperty, isPublic);
    HHVM_ME(ReflectionProperty, isProtected);
    HHVM_ME(ReflectionProperty, isPrivate);
    HHVM_ME(ReflectionProperty, isStatic);
    HHVM_ME(ReflectionProperty, isInternalToModule);
    HHVM_ME(ReflectionProperty, isDefault);
    HHVM_ME(ReflectionProperty, isReadonly);
    HHVM_ME(ReflectionProperty, getModifiers);
    HHVM_ME(ReflectionProperty, getDocComment);
    HHVM_ME(ReflectionProperty, getTypeText);
    HHVM_ME(ReflectionProperty, getDefaultValue);
    HHVM_ME(ReflectionProperty, getAttributesNamespaced);

    HHVM_ME(ReflectionTypeAlias, __init);
    HHVM_ME(ReflectionTypeAlias, getTypeStructure);
    HHVM_ME(ReflectionTypeAlias, getAttributesNamespaced);
    HHVM_ME(ReflectionTypeAlias, getAssignedTypeText);
    HHVM_ME(ReflectionTypeAlias, getFileName);

    HHVM_ME(ReflectionClass, __init);
    HHVM_ME(ReflectionClass, getName);
    HHVM_ME(ReflectionClass, getParentName);
    HHVM_ME(ReflectionClass, isHack);
    HHVM_ME(ReflectionClass, isInternal);
    HHVM_ME(ReflectionClass, isInstantiable);
    HHVM_ME(ReflectionClass, isInterface);
    HHVM_ME(ReflectionClass, isTrait);
    HHVM_ME(ReflectionClass, isEnum);
    HHVM_ME(ReflectionClass, getEnumUnderlyingType);
    HHVM_ME(ReflectionClass, isAbstract);
    HHVM_ME(ReflectionClass, isFinal);
    HHVM_ME(ReflectionClass, isInternalToModule);
    HHVM_ME(ReflectionClass, getModifiers);
    HHVM_ME(ReflectionClass, getFileName);
    HHVM_ME(ReflectionClass, getStartLine);
    HHVM_ME(ReflectionClass, getEndLine);
    HHVM_ME(ReflectionClass, getDocComment);
    HHVM_ME(ReflectionClass, getInterfaceNames);
    HHVM_ME(ReflectionClass, getRequirementNames);
    HHVM_ME(ReflectionClass, getRequiredClass);
    HHVM_ME(ReflectionClass, getTraitNames);
    HHVM_ME(ReflectionClass, getModule);

    HHVM_ME(ReflectionClass, hasMethod);
    HHVM_STATIC_ME(ReflectionClass, getMethodOrder);

    HHVM_ME(ReflectionClass, hasConstant);
    HHVM_ME(ReflectionClass, getConstant);
    HHVM_STATIC_ME(ReflectionClass, getOrderedConstants);
    HHVM_STATIC_ME(ReflectionClass, getOrderedAbstractConstants);
    HHVM_STATIC_ME(ReflectionClass, getOrderedTypeConstants);

    HHVM_ME(ReflectionClass, getAttributesNamespaced);
    HHVM_ME(ReflectionClass, getAttributesRecursiveNamespaced);

    HHVM_ME(ReflectionClass, getReifiedTypeParamInfo);

    HHVM_STATIC_ME(ReflectionClass, getClassPropertyInfo);
    HHVM_ME(ReflectionClass, getDynamicPropertyInfos);
    HHVM_ME(ReflectionClass, getConstructorName);

    Native::registerNativeDataInfo<ReflectionFuncHandle>();
    Native::registerNativeDataInfo<ReflectionClassHandle>();
    Native::registerNativeDataInfo<ReflectionConstHandle>();
    Native::registerNativeDataInfo<ReflectionPropHandle>();
    Native::registerNativeDataInfo<ReflectionFileHandle>();
    Native::registerNativeDataInfo<ReflectionModuleHandle>();
    Native::registerNativeDataInfo<ReflectionTypeAliasHandle>(Native::NO_SWEEP);

    Native::registerNativePropHandler<reflection_extension_PropHandler>(
      ReflectionExtensionLoader::className());
  }

  std::vector<std::string> hackFiles() const {
    return {
      "reflection",
      "reflection-classes",
      "reflection-internals-functions",
      "reflection_hni"
    };
  }
} s_reflection_extension;

///////////////////////////////////////////////////////////////////////////////

namespace DebuggerReflection {

namespace {

void set_debugger_source_info(Array &ret, const StringData* file, int line1,
                              int line2) {
  if (!file) file = staticEmptyString();
  if (file->data()[0] != '/') {
    ret.set(s_file, SourceRootInfo::RelativeToPhpRoot(StrNR(file)));
  } else {
    assertx(!file->isRefCounted());
    ret.set(s_file, make_tv<KindOfPersistentString>(file));
  }
  ret.set(s_line1, make_tv<KindOfInt64>(line1));
  ret.set(s_line2, make_tv<KindOfInt64>(line2));
}

}

static void set_debugger_return_type_constraint(Array &ret, const StringData* retType) {
  assertx(ret.isDict());
  if (retType && retType->size()) {
    assertx(!retType->isRefCounted());
    ret.set(s_return_type, make_tv<KindOfPersistentString>(retType));
  } else {
    ret.set(s_return_type, make_tv<KindOfBoolean>(false));
  }
}

static void set_debugger_reflection_method_prototype_info(Array& ret,
                                                          const Func *func) {
  assertx(ret.isDict());
  const Class *prototypeCls = nullptr;
  if (func->baseCls() != nullptr && func->baseCls() != func->implCls()) {
    prototypeCls = func->baseCls();
    const Class *result = get_prototype_class_from_interfaces(
      prototypeCls, func);
    if (result) prototypeCls = result;
  } else if (func->isMethod()) {
    // lookup the prototype in the interfaces
    prototypeCls = get_prototype_class_from_interfaces(func->implCls(), func);
  }
  if (prototypeCls) {
    Array prototype = Array::CreateDict();
    prototype.set(s_class,
                  make_tv<KindOfPersistentString>(prototypeCls->name()));
    prototype.set(s_name, make_tv<KindOfPersistentString>(func->name()));
    ret.set(s_prototype, prototype);
  }
}

static void set_debugger_reflection_function_info(Array& ret,
                                                  const Func* func) {
  assertx(ret.isDict());
  // return type
  if (func->isBuiltin()) {
    ret.set(s_internal, make_tv<KindOfBoolean>(true));
  }
  set_debugger_return_type_constraint(ret, func->returnUserType());

  // doc comments
  set_doc_comment(ret, func->docComment(), func->isBuiltin());

  // parameters
  ret.set(s_params, get_function_param_info(func));

  // user attributes
  ret.set(s_attributes, get_function_user_attributes(func));

  ret.set(s_is_async, func->isAsync());
  ret.set(s_is_closure, func->isClosureBody());
  ret.set(s_is_generator, func->isGenerator());
}

static void set_debugger_reflection_method_info(Array& ret, const Func* func,
                                                const Class* cls) {
  assertx(ret.isDict());
  ret.set(s_name, make_tv<KindOfPersistentString>(func->name()));
  set_attrs(ret, get_modifiers(func->attrs(), false, false));

  if (isConstructor(func)) {
    ret.set(s_constructor, make_tv<KindOfBoolean>(true));
  }

  // If Func* is from a PreClass, it doesn't know about base classes etc.
  // Swap it out for the full version if possible.
  auto resolved_func = func->implCls()
    ? func
    : cls->lookupMethod(func->name());

  if (!resolved_func) {
    resolved_func = func;
  }

  ret.set(s_class,
          make_tv<KindOfPersistentString>(resolved_func->implCls()->name()));
  set_debugger_reflection_function_info(ret, resolved_func);
  set_debugger_source_info(ret, func->unit()->filepath(),
                           func->line1(), func->line2());
  set_debugger_reflection_method_prototype_info(ret, resolved_func);
}

Array get_function_info(const String& name) {
  if (name.get() == nullptr) return null_array;
  const Func* func = Func::load(name.get());
  if (!func) return null_array;
  auto ret = Array::CreateDict();
  ret.set(s_name,       make_tv<KindOfPersistentString>(func->name()));

  // setting parameters and static variables
  set_debugger_reflection_function_info(ret, func);
  set_debugger_source_info(ret, func->unit()->filepath(),
                           func->line1(), func->line2());
  return ret;
}

Array get_class_info(const String& name) {
  auto cls = get_cls(name);
  if (!cls) return null_array;

  auto ret = Array::CreateDict();
  ret.set(s_name, make_tv<KindOfPersistentString>(cls->name()));
  ret.set(s_extension, empty_string_tv());
  ret.set(s_parent, make_tv<KindOfPersistentString>(cls->preClass()->parent()));

  // interfaces
  {
    auto const& allIfaces = cls->allInterfaces();
    DictInit arr(allIfaces.size());
    for (auto const& interface: cls->declInterfaces()) {
      arr.set(interface->nameStr(), make_tv<KindOfInt64>(1));
    }
    if (allIfaces.size() > cls->declInterfaces().size()) {
      for (int i = 0; i < allIfaces.size(); ++i) {
        auto const& interface = allIfaces[i];
        arr.set(interface->nameStr(), make_tv<KindOfInt64>(1));
      }
    }
    ret.set(s_interfaces, make_array_like_tv(arr.create()));
  }

  // traits
  {
    auto const& traits = cls->preClass()->usedTraits();
    DictInit arr(traits.size());
    for (auto const& traitName : traits) {
      arr.set(StrNR(traitName), make_tv<KindOfInt64>(1));
    }
    ret.set(s_traits, make_array_like_tv(arr.create()));
  }

  // attributes
  {
    if (cls->attrs() & AttrBuiltin) {
      ret.set(s_internal,  make_tv<KindOfBoolean>(true));
    }
    if (cls->attrs() & AttrFinal) {
      ret.set(s_final,     make_tv<KindOfBoolean>(true));
    }
    if (cls->attrs() & AttrAbstract) {
      ret.set(s_abstract,  make_tv<KindOfBoolean>(true));
    }
    if (cls->attrs() & AttrInterface) {
      ret.set(s_interface, make_tv<KindOfBoolean>(true));
    }
    if (cls->attrs() & AttrTrait) {
      ret.set(s_trait,     make_tv<KindOfBoolean>(true));
    }
    ret.set(s_modifiers, make_tv<KindOfInt64>(
      get_modifiers(cls->attrs(), true, false))
    );

    if (cls->getCtor()->attrs() & AttrPublic &&
        !(cls->attrs() & AttrAbstract) &&
        !(cls->attrs() & AttrInterface) &&
        !(cls->attrs() & AttrTrait)) {
      ret.set(s_instantiable, make_tv<KindOfBoolean>(true));
    }
  }

  // methods
  {
    auto arr = Array::CreateDict();

    // Fetch from PreClass as:
    // - the order is important
    // - we want type profiling info
    // and neither of these are in the Class...
    Func* const* methods = cls->preClass()->methods();
    size_t const numMethods = cls->preClass()->numMethods();
    for (Slot i = 0; i < numMethods; ++i) {
      const Func* m = methods[i];
      if (m->isGenerated()) continue;

      auto exactName = m->nameStr();
      auto info = Array::CreateDict();
      set_debugger_reflection_method_info(info, m, cls);
      arr.set(exactName, VarNR(info).tv());
    }

    for (Slot i = cls->traitsBeginIdx(); i < cls->traitsEndIdx(); ++i) {
      const Func* m = cls->getMethod(i);
      if (m->isGenerated()) continue;

      auto exactName = m->nameStr();
      auto info = Array::CreateDict();
      set_debugger_reflection_method_info(info, m, cls);
      arr.set(exactName, VarNR(info).tv());
    }
    ret.set(s_methods, VarNR(arr).tv());
  }

  // properties
  {
    auto arr        = Array::CreateDict();
    auto arrPriv    = Array::CreateDict();
    auto arrIdx     = Array::CreateDict();
    auto arrPrivIdx = Array::CreateDict();

    auto const properties = cls->declProperties();
    auto const& propInitVec = cls->declPropInit();
    auto const nProps = cls->numDeclProperties();

    for (Slot slot = 0; slot < nProps; ++slot) {
      auto index = cls->propSlotToIndex(slot);
      auto const& prop = properties[slot];
      auto const default_val = propInitVec[index].val.tv();
      auto info = Array::CreateDict();
      if ((prop.attrs & AttrPrivate) == AttrPrivate) {
        if (prop.cls == cls) {
          set_instance_prop_info(info, &prop, default_val);
          arrPriv.set(StrNR(prop.name), VarNR(info).tv());
          arrPrivIdx.set(StrNR(prop.name), prop.serializationIdx);
        }
        continue;
      }
      set_instance_prop_info(info, &prop, default_val);
      arr.set(StrNR(prop.name), VarNR(info).tv());
      arrIdx.set(StrNR(prop.name), prop.serializationIdx);
    }

    for (auto const& prop : cls->staticProperties()) {
      auto info = Array::CreateDict();
      if ((prop.attrs & AttrPrivate) == AttrPrivate) {
        if (prop.cls == cls) {
          set_static_prop_info(info, &prop);
          arrPriv.set(StrNR(prop.name), VarNR(info).tv());
          arrPrivIdx.set(StrNR(prop.name), prop.serializationIdx);
        }
        continue;
      }
      set_static_prop_info(info, &prop);
      arr.set(StrNR(prop.name), VarNR(info).tv());
      arrIdx.set(StrNR(prop.name), prop.serializationIdx);
    }
    ret.set(s_properties, VarNR(arr).tv());
    ret.set(s_private_properties, VarNR(arrPriv).tv());
    ret.set(s_properties_index, VarNR(arrIdx).tv());
    ret.set(s_private_properties_index, VarNR(arrPrivIdx).tv());
  }

  // constants
  {
    size_t numConsts = cls->numConstants();
    const Class::Const* consts = cls->constants();
    DictInit arr(numConsts);
    for (size_t i = 0; i < numConsts; i++) {
      // Note: hphpc doesn't include inherited constants in
      // get_class_constants(), so mimic that behavior
      if (consts[i].cls == cls) {
        TypedValue value = cls->clsCnsGet(consts[i].name);
        assertx(value.m_type != KindOfUninit);
        arr.set(StrNR(consts[i].name), value);
      }
    }

    ret.set(s_constants, make_array_like_tv(arr.create()));
  }

  { // source info
    const PreClass* pcls = cls->preClass();
    set_debugger_source_info(ret, pcls->unit()->filepath(),
                             pcls->line1(), pcls->line2());
    set_doc_comment(ret, pcls->docComment(), pcls->isBuiltin());
  }

  // user attributes
  {
    const PreClass* pcls = cls->preClass();
    auto const& attrs = pcls->userAttributes();
    DictInit arr{attrs.size()};
    for (auto const& attr : attrs) {
      arr.set(StrNR(attr.first), attr.second);
    }
    ret.set(s_attributes, make_array_like_tv(arr.create()));
  }

  return ret;
}

}

///////////////////////////////////////////////////////////////////////////////
}
