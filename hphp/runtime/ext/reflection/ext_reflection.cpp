/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/native-data.h"

#include "hphp/runtime/ext/debugger/ext_debugger.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/extension-registry.h"

#include "hphp/parser/parser.h"
#include "hphp/system/systemlib.h"

#include "hphp/runtime/vm/native-prop-handler.h"

#include <functional>

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
  s_trait_aliases("trait_aliases"),
  s_varg("varg"),
  s___invoke("__invoke"),
  s_return_type("return_type"),
  s_type_hint("type_hint"),
  s_accessible("accessible"),
  s_reflectionexception("ReflectionException"),
  s_reflectionextension("ReflectionExtension");

Class* get_cls(const Variant& class_or_object) {
  Class* cls = nullptr;
  if (class_or_object.is(KindOfObject)) {
    ObjectData* obj = class_or_object.toCObjRef().get();
    cls = obj->getVMClass();
  } else {
    cls = Unit::loadClass(class_or_object.toString().get());
  }
  return cls;
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
  assert(func == nullptr || func->isMethod());
  return func;
}

Variant default_arg_from_php_code(const Func::ParamInfo& fpi,
                                  const Func* func) {
  assert(fpi.hasDefaultValue());
  if (fpi.hasScalarDefaultValue()) {
    // Most of the time the default value is scalar, so we can
    // avoid evaling in the common case
    return tvAsVariant((TypedValue*)&fpi.defaultValue);
  } else {
    // Eval PHP code to get default value. Note that access of
    // undefined class constants can cause the eval() to
    // fatal. Zend lets such fatals propagate, so don't bother catching
    // exceptions here.
    return g_context->getEvaledArg(
      fpi.phpCode,
      // We use cls() instead of implCls() because we want the namespace and
      // class context for which the closure is scoped, not that of the Closure
      // subclass (which, among other things, is always globally namespaced).
      func->cls() ? func->cls()->nameStr() : func->nameStr()
    );
  }
}

Array HHVM_FUNCTION(hphp_get_extension_info, const String& name) {
  Array ret;

  Extension *ext = ExtensionRegistry::get(name);

  ret.set(s_name,      name);
  ret.set(s_version,   ext ? ext->getVersion() : "");
  ret.set(s_info,      empty_string_variant_ref);
  ret.set(s_ini,       Array::Create());
  ret.set(s_constants, Array::Create());
  ret.set(s_functions, Array::Create());
  ret.set(s_classes,   Array::Create());

  return ret;
}

int get_modifiers(Attr attrs, bool cls) {
  int php_modifier = 0;
  if (attrs & AttrAbstract)  php_modifier |= cls ? 0x20 : 0x02;
  if (attrs & AttrFinal)     php_modifier |= cls ? 0x40 : 0x04;
  if (attrs & AttrStatic)    php_modifier |= 0x01;
  if (!cls) {  // AttrPublic bits are not valid on class (have other meaning)
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
    ret.set(s_access, VarNR(s_public));
    ret.set(s_accessible, true_varNR);
  } else if (modifiers & 0x200) {
    ret.set(s_access, VarNR(s_protected));
    ret.set(s_accessible, false_varNR);
  } else if (modifiers & 0x400) {
    ret.set(s_access, VarNR(s_private));
    ret.set(s_accessible, false_varNR);
  } else {
    assert(false);
  }
  ret.set(s_modifiers, VarNR(modifiers));
  if (modifiers & 0x1) {
    ret.set(s_static,    true_varNR);
  }
  if (modifiers & 0x44) {
    ret.set(s_final,     true_varNR);
  }
  if (modifiers & 0x22) {
    ret.set(s_abstract,  true_varNR);
  }
}

static void set_empty_doc_comment(Array& ret) {
  ret.set(s_doc, false_varNR);
}

static void set_doc_comment(Array& ret,
                            const StringData* comment,
                            bool isBuiltin) {
  if (comment == nullptr || comment->empty()) {
    set_empty_doc_comment(ret);
  } else if (isBuiltin && !HHVM_FUNCTION(hphp_debugger_attached)) {
    set_empty_doc_comment(ret);
  } else {
    ret.set(s_doc, VarNR(comment));
  }
}

static void set_instance_prop_info(Array& ret,
                                   const Class::Prop* prop,
                                   const Variant& default_val) {
  ret.set(s_name, VarNR(prop->m_name));
  ret.set(s_default, true_varNR);
  ret.set(s_defaultValue, default_val);
  set_attrs(ret, get_modifiers(prop->m_attrs, false) & ~0x66);
  ret.set(s_class, VarNR(prop->m_class->name()));
  set_doc_comment(ret, prop->m_docComment, prop->m_class->isBuiltin());

  if (prop->m_typeConstraint && prop->m_typeConstraint->size()) {
    ret.set(s_type, VarNR(prop->m_typeConstraint));
  } else {
    ret.set(s_type, false_varNR);
  }
}

static void set_dyn_prop_info(
    Array &ret,
    const Variant& name,
    const StringData* className) {
  ret.set(s_name, name);
  set_attrs(ret, get_modifiers(AttrPublic, false) & ~0x66);
  ret.set(s_class, VarNR(className));
  set_empty_doc_comment(ret);
  ret.set(s_type, false_varNR);
}

static void set_static_prop_info(Array &ret, const Class::SProp* prop) {
  ret.set(s_name, VarNR(prop->m_name));
  ret.set(s_default, true_varNR);
  ret.set(s_defaultValue, tvAsCVarRef(&prop->m_val));
  set_attrs(ret, get_modifiers(prop->m_attrs, false) & ~0x66);
  ret.set(s_class, VarNR(prop->m_class->name()));
  set_doc_comment(ret, prop->m_docComment, prop->m_class->isBuiltin());
  if (prop->m_typeConstraint && prop->m_typeConstraint->size()) {
    ret.set(s_type, VarNR(prop->m_typeConstraint));
  } else {
    ret.set(s_type, false_varNR);
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

  if (!f_defined(cname)) {
    cns = uninit_null();
    return false;
  }

  cns = f_constant(cname);
  return true;
}

bool resolveDefaultParameterConstant(const char *value, int64_t valueLen,
                                     Variant &cns) {
  const char *p = value;
  const char *e = value + valueLen;
  const char *s;
  bool isLval = false;
  int64_t lval = 0;

  while ((s = strchr(p, '|'))) {
    isLval = true;
    if (!resolveConstant(p, s - p, cns)) {
      return false;
    }
    lval |= cns.toInt64();
    p = s + 1;
  }
  if (!resolveConstant(p, e - p, cns)) {
    return false;
  }
  if (isLval) {
    cns = cns.toInt64() | lval;
  }
  return true;
}

static bool isConstructor(const Func* func) {
  PreClass* pcls = func->preClass();
  if (!pcls || (pcls->attrs() & AttrInterface)) { return false; }
  if (func->implCls()) { return func == func->implCls()->getCtor(); }
  if (0 == strcasecmp("__construct", func->name()->data())) { return true; }
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
  return invoke(name.data(), params);
}

Variant HHVM_FUNCTION(hphp_invoke_method, const Variant& obj, const String& cls,
                                          const String& name, const Variant& params) {
  if (obj.isNull()) {
    return invoke_static_method(cls, name, params);
  }
  ObjectData *o = obj.toObject().get();
  return o->o_invoke(name, params);
}

Object HHVM_FUNCTION(hphp_create_object, const String& name, const Variant& params) {
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
  auto const class_ = Unit::lookupClass(sd);
  if (!class_) {
    raise_error("Non-existent class %s", sd->data());
  }
  VMRegAnchor _;

  auto const lookup = class_->getSProp(
    force ? class_ : arGetContextClass(vmfp()),
    prop.get()
  );
  if (!lookup.prop) {
    raise_error("Class %s does not have a property named %s",
                sd->data(), prop.get()->data());
  }
  if (!lookup.accessible) {
    raise_error("Invalid access to class %s's property %s",
                sd->data(), prop.get()->data());
  }
  return tvAsVariant(lookup.prop);
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
  auto const class_ = Unit::lookupClass(sd);

  if (!class_) raise_error("Non-existent class %s", sd->data());

  VMRegAnchor _;

  auto const lookup = class_->getSProp(
    force ? class_ : arGetContextClass(vmfp()),
    prop.get()
  );
  if (!lookup.prop) {
    raise_error("Class %s does not have a property named %s",
                cls.get()->data(), prop.get()->data());
  }
  if (!lookup.accessible) {
    raise_error("Invalid access to class %s's property %s",
                sd->data(), prop.get()->data());
  }
  tvAsVariant(lookup.prop) = value;
}

String HHVM_FUNCTION(hphp_get_original_class_name, const String& name) {
  Class* cls = Unit::loadClass(name.get());
  if (!cls) return empty_string();
  return cls->nameStr();
}

Object Reflection::AllocReflectionExceptionObject(const Variant& message) {
  Object inst{s_ReflectionExceptionClass};
  TypedValue ret;
  g_context->invokeFunc(&ret,
                        s_ReflectionExceptionClass->getCtor(),
                        make_packed_array(message),
                        inst.get());
  tvRefcountedDecRef(&ret);
  return inst;
}


HPHP::Class* Reflection::s_ReflectionExceptionClass = nullptr;

/////////////////////////////////////////////////////////////////////////////
// class ReflectionFuncHandle

const StaticString s_ReflectionFuncHandle("ReflectionFuncHandle");

static Variant HHVM_METHOD(ReflectionFunctionAbstract, getFileName) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  if (func->isBuiltin()) {
    return false;
  }
  auto file = func->unit()->filepath()->data();
  if (!file) { file = ""; }
  if (file[0] != '/') {
    return String(RuntimeOption::SourceRoot + file);
  } else {
    return String(file);
  }
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

static Variant HHVM_METHOD(ReflectionFunctionAbstract, getDocComment) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  auto const comment = func->docComment();
  if (comment == nullptr || comment->empty()) {
    return false_varNR;
  } else if (func->isBuiltin() && !HHVM_FUNCTION(hphp_debugger_attached)) {
    return false_varNR;
  } else {
    auto ret = const_cast<StringData*>(comment);
    return VarNR(ret);
  }
}

ALWAYS_INLINE
static Array get_function_static_variables(const Func* func) {
  auto const& staticVars = func->staticVars();

  auto size = staticVars.size();
  ArrayInit ai(size, ArrayInit::Mixed{});

  for (size_t i = 0; i < staticVars.size(); ++i) {
    const Func::SVInfo &sv = staticVars[i];
    auto const refData = rds::bindStaticLocal(func, sv.name);
    // FIXME: this should not require variant hops
    ai.setKeyUnconverted(
      VarNR(sv.name),
      refData->isUninitializedInRDS()
        ? null_variant
        : tvAsCVarRef(refData.get()->tv())
    );
  }
  return ai.toArray();
}

static Array HHVM_METHOD(ReflectionFunctionAbstract, getStaticVariables) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return get_function_static_variables(func);
}

static String HHVM_METHOD(ReflectionFunctionAbstract, getName) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  auto ret = const_cast<StringData*>(func->name());
  return String(ret);
}

static bool HHVM_METHOD(ReflectionFunctionAbstract, isHack) {
  if (RuntimeOption::EnableHipHopSyntax) {
    return true;
  }
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->unit()->isHHFile();
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

static bool HHVM_METHOD(ReflectionFunctionAbstract, isVariadic) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->hasVariadicCaptureParam();
}

static bool HHVM_METHOD(ReflectionFunctionAbstract, returnsReference) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->attrs() & AttrReference;
}

static int64_t HHVM_METHOD(ReflectionFunctionAbstract, getNumberOfParameters) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return func->numParams();
}

ALWAYS_INLINE
static Array get_function_param_info(const Func* func) {
  const Func::ParamInfoVec& params = func->params();
  PackedArrayInit ai(func->numParams());

  for (int i = 0; i < func->numParams(); ++i) {
    Array param = Array::Create();
    const Func::ParamInfo& fpi = params[i];

    param.set(s_index, VarNR((int)i));
    VarNR name(func->localNames()[i]);
    param.set(s_name, name);

    auto const nonExtendedConstraint =
      fpi.typeConstraint.hasConstraint() &&
      !fpi.typeConstraint.isExtended();
    auto const type = nonExtendedConstraint ? fpi.typeConstraint.typeName()
      : staticEmptyString();

    param.set(s_type, VarNR(type));
    const StringData* typeHint = fpi.userType ?
      fpi.userType : staticEmptyString();
    param.set(s_type_hint, VarNR(typeHint));
    param.set(s_function, VarNR(func->name()));
    if (func->preClass()) {
      param.set(
        s_class,
        VarNR(func->implCls() ? func->implCls()->name()
                              : func->preClass()->name())
      );
    }
    if (!nonExtendedConstraint || fpi.typeConstraint.isNullable()) {
      param.set(s_nullable, true_varNR);
    }

    if (fpi.phpCode) {
      Variant v = default_arg_from_php_code(fpi, func);
      param.set(s_default, v);
      param.set(s_defaultText, VarNR(fpi.phpCode));
    } else if (auto mi = func->methInfo()) {
      auto p = mi->parameters[i];
      auto defText = p->valueText;
      auto defTextLen = p->valueTextLen;
      if (defText == nullptr) {
        defText = "";
        defTextLen = 0;
      }
      if (p->value && *p->value) {
        if (*p->value == '\x01') {
          Variant v;
          if (resolveDefaultParameterConstant(defText, defTextLen, v)) {
            param.set(s_default, v);
          } else {
            auto obj = SystemLib::AllocStdClassObject();
            obj->o_set(s_msg, String("Unknown unserializable default value: ")
                       + defText);
            param.set(s_default, std::move(obj));
          }
        } else {
          param.set(s_default, unserialize_from_string(p->value));
        }
        param.set(s_defaultText, defText);
      }
    }

    if (func->byRef(i)) {
      param.set(s_ref, true_varNR);
    }
    if (fpi.isVariadic()) {
      param.set(s_is_variadic, true_varNR);
    }
    {
      Array userAttrs = Array::Create();
      for (auto it = fpi.userAttributes.begin();
           it != fpi.userAttributes.end(); ++it) {
        userAttrs.set(StrNR(it->first), tvAsCVarRef(&it->second));
      }
      param.set(s_attributes, VarNR(userAttrs));
    }
    ai.append(VarNR(param));
  }

  auto arr = ai.toArray();

  bool isOptional = true;
  for (int i = func->numParams() - 1; i >= 0; i--) {
    auto& param = arr.lvalAt(i).toArrRef();

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

ALWAYS_INLINE
static Array get_function_user_attributes(const Func* func) {
  auto userAttrs = func->userAttributes();

  ArrayInit ai(userAttrs.size(), ArrayInit::Mixed{});
  for (auto it = userAttrs.begin(); it != userAttrs.end(); ++it) {
    ai.set(String(StrNR(it->first)).toKey(), tvAsCVarRef(&it->second));
  }
  return ai.toArray();
}

static Array HHVM_METHOD(ReflectionFunctionAbstract, getAttributes) {
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
  assert(func->isMethod());
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

static bool HHVM_METHOD(ReflectionMethod, isConstructor) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return isConstructor(func);
}

static int HHVM_METHOD(ReflectionMethod, getModifiers) {
  auto const func = ReflectionFuncHandle::GetFuncFor(this_);
  return get_modifiers(func->attrs(), false);
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

// ------------------------- class ReflectionFunction

// helper for __construct
static bool HHVM_METHOD(ReflectionFunction, __initName, const String& name) {
  if (name.isNull()) { return false; }
  const Func* func = Unit::loadFunc(name.get());
  if (!func) { return false; }
  ReflectionFuncHandle::Get(this_)->setFunc(func);
  return true;
}

// helper for __construct
static bool HHVM_METHOD(ReflectionFunction, __initClosure,
                        const Object& closure) {
  auto const cls = get_cls(closure);
  assert(cls);
  if (!cls) { return false; }
  const Func* func = cls->lookupMethod(s___invoke.get());
  if (!func) {
    // caller raises exception
    return false;
  }
  assert(func->isClosureBody());
  assert(func->implCls()->isScopedClosure());
  ReflectionFuncHandle::Get(this_)->setFunc(func);
  return true;
}

// helper for getClosureScopeClass
static String HHVM_METHOD(ReflectionFunction, getClosureScopeClassname,
                          const Object& closure) {
  auto clos = unsafe_cast<c_Closure>(closure);
  if (clos->getScope()) {
    return String(const_cast<StringData*>(clos->getScope()->name()));
  }
  return String();
}

static Object HHVM_METHOD(ReflectionFunction, getClosureThisObject,
                          const Object& closure) {
  auto clos = unsafe_cast<c_Closure>(closure);
  if (clos->hasThis()) {
    return clos->getThis();
  }
  return nullptr;
}

// helper for getStaticVariables
static Array HHVM_METHOD(ReflectionFunction, getClosureUseVariables,
                         const Object& closure) {
  auto const cls = get_cls(closure);
  assert(cls);

  auto size = cls->numDeclProperties();
  ArrayInit ai(size, ArrayInit::Mixed{});

  auto clsName = cls->nameStr();

  for (Slot i = 0; i < size; ++i) {
    auto const& prop = cls->declProperties()[i];
    auto val = closure.get()->o_realProp(StrNR(prop.m_name),
                                         ObjectData::RealPropExist, clsName);
    assert(val);

    // Closure static locals are represented as special instance properties
    // with a mangled name.
    if (prop.m_name->data()[0] == '8') {
      static const char prefix[] = "86static_";
      assert(0 == strncmp(prop.m_name->data(), prefix, sizeof prefix - 1));
      String strippedName(prop.m_name->data() + sizeof prefix - 1,
                          prop.m_name->size() - sizeof prefix + 1,
                          CopyString);
      ai.setKeyUnconverted(VarNR(strippedName), *val);
    } else {
      if (val->isReferenced()) {
        ai.setRef(VarNR(prop.m_name), *val, false /* = keyConverted */);
      } else {
        ai.setKeyUnconverted(VarNR(prop.m_name), *val);
      }
    }
  }
  return ai.toArray();
}

/////////////////////////////////////////////////////////////////////////////
// class ReflectionClass

const StaticString s_ReflectionClassHandle("ReflectionClassHandle");

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
  if (RuntimeOption::EnableHipHopSyntax) {
    return true;
  }
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->preClass()->unit()->isHHFile();
}

static bool HHVM_METHOD(ReflectionClass, isInternal) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->attrs() & AttrBuiltin;
}

static bool HHVM_METHOD(ReflectionClass, isInstantiable) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return !(cls->attrs() & (AttrAbstract | AttrInterface | AttrTrait | AttrEnum))
    && (cls->getCtor()->attrs() & AttrPublic);
}

static bool HHVM_METHOD(ReflectionClass, isFinal) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->attrs() & AttrFinal;
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
  return cls->attrs() & AttrEnum;
}

static int HHVM_METHOD(ReflectionClass, getModifiers) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return get_modifiers(cls->attrs(), true);
}

static Variant HHVM_METHOD(ReflectionClass, getFileName) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  if (cls->attrs() & AttrBuiltin) {
    return false_varNR;
  }
  auto file = cls->preClass()->unit()->filepath()->data();
  if (!file) { file = ""; }
  if (file[0] != '/') {
    return String(RuntimeOption::SourceRoot + file);
  } else {
    return String(file);
  }
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

static Variant HHVM_METHOD(ReflectionClass, getDocComment) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  auto const pcls = cls->preClass();
  auto const comment = pcls->docComment();
  if (comment == nullptr || comment->empty()) {
    return false_varNR;
  } else if (pcls->isBuiltin() && !HHVM_FUNCTION(hphp_debugger_attached)) {
    return false_varNR;
  } else {
    auto ret = const_cast<StringData*>(comment);
    return VarNR(ret);
  }
}

static Array HHVM_METHOD(ReflectionClass, getRequirementNames) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  if (!(cls->attrs() & (AttrTrait | AttrInterface))) {
    // requirements are applied to abstract/concrete classes when they use
    // a trait / implement an interface
    return empty_array();
  }

  auto const& requirements = cls->allRequirements();
  auto numReqs = requirements.size();
  if (numReqs == 0) {
    return empty_array();
  }

  PackedArrayInit pai(numReqs);
  for (int i = 0; i < numReqs; ++i) {
    auto const& req = requirements[i];
    pai.append(const_cast<StringData*>(req->name()));
  }
  return pai.toArray();
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

  PackedArrayInit ai(st->size());
  for (ArrayIter iter(st.get()); iter; ++iter) {
    ai.append(iter.secondRefPlus());
  }
  return ai.toArray();
}

static Array HHVM_METHOD(ReflectionClass, getTraitNames) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  auto const& traits = cls->preClass()->usedTraits();
  PackedArrayInit ai(traits.size());
  for (const StringData* traitName : traits) {
    ai.append(const_cast<StringData*>(traitName));
  }
  return ai.toArray();
}

static Array get_trait_alias_info(const Class* cls) {
  auto const& aliases = cls->traitAliases();

  if (aliases.size()) {
    ArrayInit ai(aliases.size(), ArrayInit::Map{});

    for (auto const& namePair : aliases) {
      ai.set(StrNR(namePair.first), VarNR(namePair.second));
    }
    return ai.toArray();
  } else {
    // Even if we have alias rules, if we're in repo mode, they will be applied
    // during the trait flattening step, and we won't populate traitAliases()
    // on the Class.
    auto const& rules = cls->preClass()->traitAliasRules();

    ArrayInit ai(rules.size(), ArrayInit::Map{});

    for (auto const& rule : rules) {
      auto namePair = rule.asNamePair();
      ai.set(StrNR(namePair.first), VarNR(namePair.second));
    }
    return ai.toArray();
  }
}

static Array HHVM_METHOD(ReflectionClass, getTraitAliases) {
  return get_trait_alias_info(ReflectionClassHandle::GetClassFor(this_));
}

static bool HHVM_METHOD(ReflectionClass, hasMethod, const String& name) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return (get_method_func(cls, name) != nullptr);
}

// helper for getMethods: returns a Set
static Object HHVM_METHOD(ReflectionClass, getMethodOrder, int64_t filter) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  Attr mask = attrs_from_modifiers(filter, false);

  // At each step, we fetch from the PreClass is important because the
  // order in which getMethods returns matters
  StringISet visitedMethods;
  auto st = req::make<c_Set>();
  st->reserve(cls->numMethods());

  auto add = [&] (const Func* m) {
    if (m->isGenerated()) return;
    if (visitedMethods.count(m->nameStr())) return;

    visitedMethods.insert(m->nameStr());
    if (m->attrs() & mask) {
      st->add(HHVM_FN(strtolower)(m->nameStr()).get());
    }
  };

  std::function<void(const Class*)> collect;
  std::function<void(const Class*)> collectInterface;

  collect = [&] (const Class* cls) {
    if (!cls) return;

    auto const methods = cls->preClass()->methods();
    auto const numMethods = cls->preClass()->numMethods();

    auto numDeclMethods = cls->preClass()->numDeclMethods();
    if (numDeclMethods == -1) numDeclMethods = numMethods;

    // Add declared methods.
    for (Slot i = 0; i < numDeclMethods; ++i) {
      add(methods[i]);
    }

    // Recurse; we need to order the parent's methods before our trait methods.
    collect(cls->parent());

    for (Slot i = numDeclMethods; i < numMethods; ++i) {
      // For repo mode, where trait methods are flattened at compile-time.
      add(methods[i]);
    }
    for (Slot i = cls->traitsBeginIdx(); i < cls->traitsEndIdx(); ++i) {
      // For non-repo mode, where they are added at Class-creation time.
      add(cls->getMethod(i));
    }
  };

  collectInterface = [&] (const Class* iface) {
    if (!iface) return;

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
  return Object(std::move(st));
}

static bool HHVM_METHOD(ReflectionClass, hasConstant, const String& name) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  return cls->hasConstant(name.get());
}

static Variant HHVM_METHOD(ReflectionClass, getConstant, const String& name) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  auto value = cls->clsCnsGet(name.get());
  return (value.m_type == KindOfUninit) ? false_varNR : cellAsCVarRef(value);
}

static
void addClassConstantNames(const Class* cls,
                           const req::ptr<c_Set>& st,
                           size_t limit) {
  assert(cls && st && (st->size() < limit));

  auto numConsts = cls->numConstants();

  const Class::Const* consts = cls->constants();
  for (size_t i = 0; i < numConsts; i++) {
    if (consts[i].m_class == cls && !consts[i].isAbstract() &&
        !consts[i].isType()) {
      st->add(const_cast<StringData*>(consts[i].m_name.get()));
    }
  }
  if ((st->size() < limit) && cls->parent()) {
    addClassConstantNames(cls->parent(), st, limit);
  }

  auto const& allIfaces = cls->allInterfaces();
  auto const numIfaces = allIfaces.size();
  for (int i = 0; i < numIfaces && (st->size() < limit); ++i) {
    addClassConstantNames(allIfaces[i].get(), st, limit);
  }
}

// helper for getConstants
static Array HHVM_METHOD(ReflectionClass, getOrderedConstants) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);

  size_t numConsts = cls->numConstants();
  if (!numConsts) {
    return Array::Create();
  }

  auto st = req::make<c_Set>();
  st->reserve(numConsts);

  addClassConstantNames(cls, st, numConsts);
  assert(st->size() <= numConsts);

  ArrayInit ai(numConsts, ArrayInit::Mixed{});
  for (ArrayIter iter(st.get()); iter; ++iter) {
    auto constName = iter.first().getStringData();
    Cell value = cls->clsCnsGet(constName);
    assert(value.m_type != KindOfUninit);
    ai.add(constName, cellAsCVarRef(value));
  }
  return ai.toArray();
}

// helper for getAbstractConstantNames
static Array HHVM_METHOD(ReflectionClass, getOrderedAbstractConstants) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);

  size_t numConsts = cls->numConstants();
  if (!numConsts) {
    return Array::Create();
  }

  auto st = req::make<c_Set>();
  st->reserve(numConsts);

  const Class::Const* consts = cls->constants();
  for (size_t i = 0; i < numConsts; i++) {
    if (consts[i].isAbstract() && !consts[i].isType()) {
      st->add(const_cast<StringData*>(consts[i].m_name.get()));
    }
  }

  assert(st->size() <= numConsts);
  return st->t_toarray();
}



// helper for getTypeConstants/hasTypeConstant
static Array HHVM_METHOD(ReflectionClass, getOrderedTypeConstants) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);

  size_t numConsts = cls->numConstants();
  if (!numConsts) {
    return Array::Create();
  }

  auto st = req::make<c_Set>();
  st->reserve(numConsts);

  const Class::Const* consts = cls->constants();
  for (size_t i = 0; i < numConsts; i++) {
    if (consts[i].isType()) {
      st->add(const_cast<StringData*>(consts[i].m_name.get()));
    }
  }

  assert(st->size() <= numConsts);
  return st->t_toarray();
}

static Array HHVM_METHOD(ReflectionClass, getAttributes) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  // UserAttributes are stored exclusively on the PreClass.
  auto const pcls = cls->preClass();

  auto userAttrs = pcls->userAttributes();
  ArrayInit ai(userAttrs.size(), ArrayInit::Mixed{});

  for (auto it = userAttrs.begin(); it != userAttrs.end(); ++it) {
    ai.set(StrNR(it->first), tvAsCVarRef(&it->second));
  }
  return ai.toArray();
}

static Array HHVM_METHOD(ReflectionClass, getAttributesRecursive) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);

  Array ret = Array::Create(); // no reasonable idea about sizing

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
        ret.add(StrNR(it->first), tvAsCVarRef(&it->second));
      }
    }
  } while ((currentCls = currentCls->parent()));

  return ret;
}

static Array HHVM_METHOD(ReflectionClass, getClassPropertyInfo) {
  /*
   * FIXME: This implementation is pretty horrible and should be rewritten
   * when ReflectionProperty is ported.
   */
  auto const cls = ReflectionClassHandle::GetClassFor(this_);

  Array arrProp = Array::Create();
  Array arrPriv = Array::Create();
  Array arrIdx = Array::Create();
  Array arrPrivIdx = Array::Create();

  const Class::Prop* properties = cls->declProperties();
  cls->initialize();

  auto const& propInitVec = cls->getPropData()
    ? *cls->getPropData()
    : cls->declPropInit();

  const size_t nProps = cls->numDeclProperties();

  for (Slot i = 0; i < nProps; ++i) {
    const Class::Prop& prop = properties[i];
    Array info = Array::Create();
    auto const& default_val = tvAsCVarRef(&propInitVec[i]);
    if ((prop.m_attrs & AttrPrivate) == AttrPrivate) {
      if (prop.m_class == cls) {
        set_instance_prop_info(info, &prop, default_val);
        arrPriv.set(StrNR(prop.m_name), VarNR(info));
        arrPrivIdx.set(StrNR(prop.m_name), prop.m_idx);
      }
      continue;
    }
    set_instance_prop_info(info, &prop, default_val);
    arrProp.set(StrNR(prop.m_name), VarNR(info));
    arrIdx.set(StrNR(prop.m_name), prop.m_idx);
  }

  const Class::SProp* staticProperties = cls->staticProperties();
  const size_t nSProps = cls->numStaticProperties();

  for (Slot i = 0; i < nSProps; ++i) {
    auto const& prop = staticProperties[i];
    Array info = Array::Create();
    if ((prop.m_attrs & AttrPrivate) == AttrPrivate) {
      if (prop.m_class == cls) {
        set_static_prop_info(info, &prop);
        arrPriv.set(StrNR(prop.m_name), VarNR(info));
        arrPrivIdx.set(StrNR(prop.m_name), prop.m_idx);
      }
      continue;
    }
    set_static_prop_info(info, &prop);
    arrProp.set(StrNR(prop.m_name), VarNR(info));
    arrIdx.set(StrNR(prop.m_name), prop.m_idx);
  }

  ArrayInit ret(4, ArrayInit::Mixed{});
  ret.set(s_properties, VarNR(arrProp));
  ret.set(s_private_properties, VarNR(arrPriv));
  ret.set(s_properties_index, VarNR(arrIdx));
  ret.set(s_private_properties_index, VarNR(arrPrivIdx));
  return ret.toArray();
}

static Array HHVM_METHOD(ReflectionClass, getDynamicPropertyInfos,
                         const Object& obj) {
  auto const cls = ReflectionClassHandle::GetClassFor(this_);
  auto obj_data = obj.get();
  assert(obj_data->getVMClass() == cls);
  if (!obj_data->hasDynProps()) {
    return empty_array();
  }

  auto const dynPropArray = obj_data->dynPropArray().get();
  ArrayInit ret(dynPropArray->size(), ArrayInit::Mixed{});
  for (ArrayIter it(dynPropArray); !it.end(); it.next()) {
    Array info = Array::Create();
    set_dyn_prop_info(info, it.first(), cls->name());
    ret.set(it.first(), VarNR(info));
  }
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
      throw Reflection::AllocReflectionExceptionObject(String(msg));
    }

    // It is possible that $name does not get serialized. If a class derives
    // from ReflectionClass and the return value of its __sleep() function does
    // not contain 'name', $name gets ignored. So, we restore $name here.
    obj->o_set(s_name, result);
}

static Variant reflection_extension_name_get(const Object& this_) {
  auto name = this_->o_realProp(s___name.get(), ObjectData::RealPropUnchecked,
                                s_reflectionextension.get());
  return name->toString();
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

const StaticString s_ReflectionConstHandle("ReflectionConstHandle");

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
    if (const_name.same(consts[i].m_name) && consts[i].isType()) {
      ReflectionConstHandle::Get(this_)->setConst(&consts[i]);
      return true;
    }
  }

  // caller raises exception
  return false;
}

static String HHVM_METHOD(ReflectionTypeConstant, getName) {
  auto const cst = ReflectionConstHandle::GetConstFor(this_);
  auto ret = const_cast<StringData*>(cst->m_name.get());
  return String(ret);
}

static bool HHVM_METHOD(ReflectionTypeConstant, isAbstract) {
  auto const cst = ReflectionConstHandle::GetConstFor(this_);
  return cst->isAbstract();
}

// helper for getAssignedTypeText
static String HHVM_METHOD(ReflectionTypeConstant, getAssignedTypeHint) {
  auto const cst = ReflectionConstHandle::GetConstFor(this_);

  if (cst->m_val.m_type == KindOfStaticString ||
      cst->m_val.m_type == KindOfString) {
    return String(cst->m_val.m_data.pstr);
  }

  return String();
}

// private helper for getDeclaringClass
static String HHVM_METHOD(ReflectionTypeConstant, getDeclaringClassname) {
  auto const cst = ReflectionConstHandle::GetConstFor(this_);
  auto cls = cst->m_class;
  auto ret = const_cast<StringData*>(cls->name());
  return String(ret);
}

/////////////////////////////////////////////////////////////////////////////
// class ReflectionProperty

const StaticString s_ReflectionPropHandle("ReflectionPropHandle");
const StaticString s_ReflectionSPropHandle("ReflectionSPropHandle");

// helper for __construct:
// returns -1 if class not defined;
// returns -2 if class::property not found (then caller raises exception);
// returns the info array for ReflectionProperty if successfully initialized.
static Variant HHVM_METHOD(ReflectionProperty, __init,
                           const Variant& cls_or_obj, const String& prop_name) {
  auto const cls = get_cls(cls_or_obj);
  if (!cls) {
    // caller raises exception
    return Variant(-1);
  }
  if (prop_name.isNull()) {
    return Variant(-2);
  }

  cls->initialize();
  auto const& propInitVec = cls->getPropData()
    ? *cls->getPropData()
    : cls->declPropInit();

  const size_t nProps = cls->numDeclProperties();
  const Class::Prop* properties = cls->declProperties();

  // index for the candidate property
  Slot cInd = -1;
  const Class::Prop* cProp = nullptr;

  for (Slot i = 0; i < nProps; i++) {
    const Class::Prop& prop = properties[i];
    if (prop_name.same(prop.m_name)) {
      if (cls == prop.m_class.get()) {
        // found match for the exact child class
        cInd = i;
        cProp = &prop;
        break;
      } else if (!(prop.m_attrs & AttrPrivate)) {
        // only inherit non-private properties
        cInd = i;
        cProp = &prop;
      }
    }
  }

  if (cProp != nullptr) {
    ReflectionPropHandle::Get(this_)->setProp(cProp);
    Array info = Array::Create();
    auto const& default_val = tvAsCVarRef(&propInitVec[cInd]);
    set_instance_prop_info(info, cProp, default_val);
    return Variant(info);
  }

  // for static property
  const size_t nSProps = cls->numStaticProperties();
  const Class::SProp* staticProperties = cls->staticProperties();
  const Class::SProp* cSProp = nullptr;

  for (Slot i = 0; i < nSProps; i++) {
    const Class::SProp& sprop = staticProperties[i];
    if (prop_name.same(sprop.m_name)) {
      if (cls == sprop.m_class.get()) {
        cSProp = &sprop;
        break;
      } else if (!(sprop.m_attrs & AttrPrivate)) {
        cSProp = &sprop;
      }
    }
  }
  if (cSProp != nullptr) {
    ReflectionSPropHandle::Get(this_)->setSProp(cSProp);
    Array info = Array::Create();
    set_static_prop_info(info, cSProp);
    return Variant(info);
  }

  // check for dynamic properties
  if (cls_or_obj.is(KindOfObject)) {
    ObjectData* obj = cls_or_obj.toCObjRef().get();
    assert(cls == obj->getVMClass());
    if (obj->hasDynProps()) {
      auto const dynPropArray = obj->dynPropArray().get();
      for (ArrayIter it(dynPropArray); !it.end(); it.next()) {
        if (prop_name.same(it.first().getStringData())) {
          Array info = Array::Create();
          set_dyn_prop_info(info, it.first(), cls->name());
          return Variant(info);
        }
      }
    }
  }

  // caller raises exception
  return Variant(-2);
}

///////////////////////////////////////////////////////////////////////////////

class ReflectionExtension final : public Extension {
 public:
  ReflectionExtension() : Extension("reflection", "$Id$") { }
  void moduleInit() override {
    HHVM_FE(hphp_create_object);
    HHVM_FE(hphp_create_object_without_constructor);
    HHVM_FE(hphp_get_extension_info);
    HHVM_FE(hphp_get_original_class_name);
    HHVM_FE(hphp_get_property);
    HHVM_FE(hphp_get_static_property);
    HHVM_FE(hphp_invoke);
    HHVM_FE(hphp_invoke_method);
    HHVM_FE(hphp_set_property);
    HHVM_FE(hphp_set_static_property);

    HHVM_ME(ReflectionFunctionAbstract, getName);
    HHVM_ME(ReflectionFunctionAbstract, isHack);
    HHVM_ME(ReflectionFunctionAbstract, isInternal);
    HHVM_ME(ReflectionFunctionAbstract, isGenerator);
    HHVM_ME(ReflectionFunctionAbstract, isAsync);
    HHVM_ME(ReflectionFunctionAbstract, isVariadic);
    HHVM_ME(ReflectionFunctionAbstract, getFileName);
    HHVM_ME(ReflectionFunctionAbstract, getStartLine);
    HHVM_ME(ReflectionFunctionAbstract, getEndLine);
    HHVM_ME(ReflectionFunctionAbstract, getDocComment);
    HHVM_ME(ReflectionFunctionAbstract, getStaticVariables);
    HHVM_ME(ReflectionFunctionAbstract, returnsReference);
    HHVM_ME(ReflectionFunctionAbstract, getReturnTypeHint);
    HHVM_ME(ReflectionFunctionAbstract, getNumberOfParameters);
    HHVM_ME(ReflectionFunctionAbstract, getParamInfo);
    HHVM_ME(ReflectionFunctionAbstract, getAttributes);

    HHVM_ME(ReflectionMethod, __init);
    HHVM_ME(ReflectionMethod, isFinal);
    HHVM_ME(ReflectionMethod, isAbstract);
    HHVM_ME(ReflectionMethod, isPublic);
    HHVM_ME(ReflectionMethod, isProtected);
    HHVM_ME(ReflectionMethod, isPrivate);
    HHVM_ME(ReflectionMethod, isStatic);
    HHVM_ME(ReflectionMethod, isConstructor);
    HHVM_ME(ReflectionMethod, getModifiers);
    HHVM_ME(ReflectionMethod, getPrototypeClassname);
    HHVM_ME(ReflectionMethod, getDeclaringClassname);

    HHVM_ME(ReflectionFunction, __initName);
    HHVM_ME(ReflectionFunction, __initClosure);
    HHVM_ME(ReflectionFunction, getClosureUseVariables);
    HHVM_ME(ReflectionFunction, getClosureScopeClassname);
    HHVM_ME(ReflectionFunction, getClosureThisObject);

    HHVM_ME(ReflectionTypeConstant, __init);
    HHVM_ME(ReflectionTypeConstant, getName);
    HHVM_ME(ReflectionTypeConstant, isAbstract);
    HHVM_ME(ReflectionTypeConstant, getAssignedTypeHint);
    HHVM_ME(ReflectionTypeConstant, getDeclaringClassname);

    HHVM_ME(ReflectionProperty, __init);

    HHVM_ME(ReflectionClass, __init);
    HHVM_ME(ReflectionClass, getName);
    HHVM_ME(ReflectionClass, getParentName);
    HHVM_ME(ReflectionClass, isHack);
    HHVM_ME(ReflectionClass, isInternal);
    HHVM_ME(ReflectionClass, isInstantiable);
    HHVM_ME(ReflectionClass, isInterface);
    HHVM_ME(ReflectionClass, isTrait);
    HHVM_ME(ReflectionClass, isEnum);
    HHVM_ME(ReflectionClass, isAbstract);
    HHVM_ME(ReflectionClass, isFinal);
    HHVM_ME(ReflectionClass, getModifiers);
    HHVM_ME(ReflectionClass, getFileName);
    HHVM_ME(ReflectionClass, getStartLine);
    HHVM_ME(ReflectionClass, getEndLine);
    HHVM_ME(ReflectionClass, getDocComment);
    HHVM_ME(ReflectionClass, getInterfaceNames);
    HHVM_ME(ReflectionClass, getRequirementNames);
    HHVM_ME(ReflectionClass, getTraitNames);
    HHVM_ME(ReflectionClass, getTraitAliases);

    HHVM_ME(ReflectionClass, hasMethod);
    HHVM_ME(ReflectionClass, getMethodOrder);

    HHVM_ME(ReflectionClass, hasConstant);
    HHVM_ME(ReflectionClass, getConstant);
    HHVM_ME(ReflectionClass, getOrderedConstants);
    HHVM_ME(ReflectionClass, getOrderedAbstractConstants);
    HHVM_ME(ReflectionClass, getOrderedTypeConstants);

    HHVM_ME(ReflectionClass, getAttributes);
    HHVM_ME(ReflectionClass, getAttributesRecursive);

    HHVM_ME(ReflectionClass, getClassPropertyInfo);
    HHVM_ME(ReflectionClass, getDynamicPropertyInfos);
    HHVM_ME(ReflectionClass, getConstructorName);

    Native::registerNativeDataInfo<ReflectionFuncHandle>(
      s_ReflectionFuncHandle.get());
    Native::registerNativeDataInfo<ReflectionClassHandle>(
      s_ReflectionClassHandle.get());
    Native::registerNativeDataInfo<ReflectionConstHandle>(
      s_ReflectionConstHandle.get());
    Native::registerNativeDataInfo<ReflectionPropHandle>(
      s_ReflectionPropHandle.get());
    Native::registerNativeDataInfo<ReflectionSPropHandle>(
      s_ReflectionSPropHandle.get());

    Native::registerNativePropHandler
      <reflection_extension_PropHandler>(s_reflectionextension);

    loadSystemlib();
    loadSystemlib("reflection-classes");
    loadSystemlib("reflection-internals-functions");
    loadSystemlib("reflection_hni");

    Reflection::s_ReflectionExceptionClass =
        Unit::lookupClass(s_reflectionexception.get());
  }
} s_reflection_extension;

///////////////////////////////////////////////////////////////////////////////

namespace DebuggerReflection {

static bool set_debugger_source_info(Array &ret, const char *file, int line1,
                                     int line2) {
  if (!file) file = "";
  if (file[0] != '/') {
    ret.set(s_file, String(RuntimeOption::SourceRoot + file));
  } else {
    ret.set(s_file, file);
  }
  ret.set(s_line1, VarNR(line1));
  ret.set(s_line2, VarNR(line2));
  return file && *file;
}

static void set_debugger_return_type_constraint(Array &ret, const StringData* retType) {
  if (retType && retType->size()) {
    ret.set(s_return_type, VarNR(retType));
  } else {
    ret.set(s_return_type, false_varNR);
  }
}

static void set_debugger_reflection_method_prototype_info(Array& ret,
                                                          const Func *func) {
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
    Array prototype = Array::Create();
    prototype.set(s_class, VarNR(prototypeCls->name()));
    prototype.set(s_name, VarNR(func->name()));
    ret.set(s_prototype, prototype);
  }
}

static void set_debugger_reflection_function_info(Array& ret,
                                                  const Func* func) {
  // return type
  if (func->attrs() & AttrReference) {
    ret.set(s_ref,      true_varNR);
  }
  if (func->isBuiltin()) {
    ret.set(s_internal, true_varNR);
  }
  set_debugger_return_type_constraint(ret, func->returnUserType());

  // doc comments
  set_doc_comment(ret, func->docComment(), func->isBuiltin());

  // parameters
  ret.set(s_params, get_function_param_info(func));

  // static variables
  ret.set(s_static_variables, get_function_static_variables(func));

  // user attributes
  ret.set(s_attributes, get_function_user_attributes(func));

  ret.set(s_is_async, func->isAsync());
  ret.set(s_is_closure, func->isClosureBody());
  ret.set(s_is_generator, func->isGenerator());
}

static void set_debugger_reflection_method_info(Array& ret, const Func* func,
                                                const Class* cls) {
  ret.set(s_name, VarNR(func->name()));
  set_attrs(ret, get_modifiers(func->attrs(), false));

  if (isConstructor(func)) {
    ret.set(s_constructor, true_varNR);
  }

  // If Func* is from a PreClass, it doesn't know about base classes etc.
  // Swap it out for the full version if possible.
  auto resolved_func = func->implCls()
    ? func
    : cls->lookupMethod(func->name());

  if (!resolved_func) {
    resolved_func = func;
  }

  ret.set(s_class, VarNR(resolved_func->implCls()->name()));
  set_debugger_reflection_function_info(ret, resolved_func);
  set_debugger_source_info(ret, func->unit()->filepath()->data(),
                           func->line1(), func->line2());
  set_debugger_reflection_method_prototype_info(ret, resolved_func);
}

Array get_function_info(const String& name) {
  Array ret;
  if (name.get() == nullptr) return ret;
  const Func* func = Unit::loadFunc(name.get());
  if (!func) return ret;
  ret.set(s_name,       VarNR(func->name()));

  // setting parameters and static variables
  set_debugger_reflection_function_info(ret, func);
  set_debugger_source_info(ret, func->unit()->filepath()->data(),
                           func->line1(), func->line2());
  return ret;
}

Array get_class_info(const String& name) {
  auto cls = get_cls(name);
  if (!cls) return Array();

  Array ret;
  ret.set(s_name,      VarNR(cls->name()));
  ret.set(s_extension, empty_string_variant_ref);
  ret.set(s_parent,    VarNR(cls->parentStr()));

  // interfaces
  {
    Array arr = Array::Create();
    for (auto const& interface: cls->declInterfaces()) {
      arr.set(interface->nameStr(), VarNR(1));
    }
    auto const& allIfaces = cls->allInterfaces();
    if (allIfaces.size() > cls->declInterfaces().size()) {
      for (int i = 0; i < allIfaces.size(); ++i) {
        auto const& interface = allIfaces[i];
        arr.set(interface->nameStr(), VarNR(1));
      }
    }
    ret.set(s_interfaces, VarNR(arr));
  }

  // traits
  {
    Array arr = Array::Create();
    for (auto const& traitName : cls->preClass()->usedTraits()) {
      arr.set(StrNR(traitName), VarNR(1));
    }
    ret.set(s_traits, VarNR(arr));
  }

  // trait aliases
  {
    ret.set(s_trait_aliases, VarNR(get_trait_alias_info(cls)));
  }

  // attributes
  {
    if (cls->attrs() & AttrBuiltin) {
      ret.set(s_internal,  true_varNR);
    }
    if (cls->attrs() & AttrFinal) {
      ret.set(s_final,     true_varNR);
    }
    if (cls->attrs() & AttrAbstract) {
      ret.set(s_abstract,  true_varNR);
    }
    if (cls->attrs() & AttrInterface) {
      ret.set(s_interface, true_varNR);
    }
    if (cls->attrs() & AttrTrait) {
      ret.set(s_trait,     true_varNR);
    }
    ret.set(s_modifiers, VarNR(get_modifiers(cls->attrs(), true)));

    if (cls->getCtor()->attrs() & AttrPublic &&
        !(cls->attrs() & AttrAbstract) &&
        !(cls->attrs() & AttrInterface) &&
        !(cls->attrs() & AttrTrait)) {
      ret.set(s_instantiable, true_varNR);
    }
  }

  // methods
  {
    Array arr = Array::Create();

    // Fetch from PreClass as:
    // - the order is important
    // - we want type profiling info
    // and neither of these are in the Class...
    Func* const* methods = cls->preClass()->methods();
    size_t const numMethods = cls->preClass()->numMethods();
    for (Slot i = 0; i < numMethods; ++i) {
      const Func* m = methods[i];
      if (m->isGenerated()) continue;

      auto lowerName = HHVM_FN(strtolower)(m->nameStr());
      Array info = Array::Create();
      set_debugger_reflection_method_info(info, m, cls);
      arr.set(lowerName, VarNR(info));
    }

    for (Slot i = cls->traitsBeginIdx(); i < cls->traitsEndIdx(); ++i) {
      const Func* m = cls->getMethod(i);
      if (m->isGenerated()) continue;

      auto lowerName = HHVM_FN(strtolower)(m->nameStr());
      Array info = Array::Create();
      set_debugger_reflection_method_info(info, m, cls);
      arr.set(lowerName, VarNR(info));
    }
    ret.set(s_methods, VarNR(arr));
  }

  // properties
  {
    Array arr = Array::Create();
    Array arrPriv = Array::Create();
    Array arrIdx = Array::Create();
    Array arrPrivIdx = Array::Create();

    const Class::Prop* properties = cls->declProperties();
    auto const& propInitVec = cls->declPropInit();
    const size_t nProps = cls->numDeclProperties();

    for (Slot i = 0; i < nProps; ++i) {
      const Class::Prop& prop = properties[i];
      Array info = Array::Create();
      auto const& default_val = tvAsCVarRef(&propInitVec[i]);
      if ((prop.m_attrs & AttrPrivate) == AttrPrivate) {
        if (prop.m_class == cls) {
          set_instance_prop_info(info, &prop, default_val);
          arrPriv.set(StrNR(prop.m_name), VarNR(info));
          arrPrivIdx.set(StrNR(prop.m_name), prop.m_idx);
        }
        continue;
      }
      set_instance_prop_info(info, &prop, default_val);
      arr.set(StrNR(prop.m_name), VarNR(info));
      arrIdx.set(StrNR(prop.m_name), prop.m_idx);
    }

    const Class::SProp* staticProperties = cls->staticProperties();
    const size_t nSProps = cls->numStaticProperties();

    for (Slot i = 0; i < nSProps; ++i) {
      auto const& prop = staticProperties[i];
      Array info = Array::Create();
      if ((prop.m_attrs & AttrPrivate) == AttrPrivate) {
        if (prop.m_class == cls) {
          set_static_prop_info(info, &prop);
          arrPriv.set(StrNR(prop.m_name), VarNR(info));
          arrPrivIdx.set(StrNR(prop.m_name), prop.m_idx);
        }
        continue;
      }
      set_static_prop_info(info, &prop);
      arr.set(StrNR(prop.m_name), VarNR(info));
      arrIdx.set(StrNR(prop.m_name), prop.m_idx);
    }
    ret.set(s_properties, VarNR(arr));
    ret.set(s_private_properties, VarNR(arrPriv));
    ret.set(s_properties_index, VarNR(arrIdx));
    ret.set(s_private_properties_index, VarNR(arrPrivIdx));
  }

  // constants
  {
    Array arr = Array::Create();

    size_t numConsts = cls->numConstants();
    const Class::Const* consts = cls->constants();

    for (size_t i = 0; i < numConsts; i++) {
      // Note: hphpc doesn't include inherited constants in
      // get_class_constants(), so mimic that behavior
      if (consts[i].m_class == cls) {
        Cell value = cls->clsCnsGet(consts[i].m_name);
        assert(value.m_type != KindOfUninit);
        arr.set(StrNR(consts[i].m_name), cellAsCVarRef(value));
      }
    }

    ret.set(s_constants, VarNR(arr));
  }

  { // source info
    const PreClass* pcls = cls->preClass();
    set_debugger_source_info(ret, pcls->unit()->filepath()->data(),
                             pcls->line1(), pcls->line2());
    set_doc_comment(ret, pcls->docComment(), pcls->isBuiltin());
  }

  // user attributes
  {
    Array arr = Array::Create();
    const PreClass* pcls = cls->preClass();
    for (auto it = pcls->userAttributes().begin();
         it != pcls->userAttributes().end(); ++it) {
      arr.set(StrNR(it->first), tvAsCVarRef(&it->second));
    }
    ret.set(s_attributes, VarNR(arr));
  }

  return ret;
}

}

///////////////////////////////////////////////////////////////////////////////
}
