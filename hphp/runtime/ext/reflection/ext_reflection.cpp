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
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/debugger/ext_debugger.h"
#include "hphp/runtime/ext/ext_misc.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/vm/runtime-type-profiler.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/parser/parser.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

using JIT::VMRegAnchor;

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_name("name"),
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
  s_reorder_parent_properties("reorder_parent_properties"),
  s_attributes("attributes"),
  s_function("function"),
  s_trait_aliases("trait_aliases"),
  s_varg("varg"),
  s_closure("closure"),
  s___invoke("__invoke"),
  s_closure_in_braces("{closure}"),
  s_closureobj("closureobj"),
  s_return_type("return_type"),
  s_type_hint("type_hint"),
  s_type_profiling("type_profiling"),
  s_accessible("accessible"),
  s_closure_scope_class("closure_scope_class"),
  s_reflectionexception("ReflectionException");

static Class* get_cls(const Variant& class_or_object) {
  Class* cls = nullptr;
  if (class_or_object.is(KindOfObject)) {
    ObjectData* obj = class_or_object.toCObjRef().get();
    cls = obj->getVMClass();
  } else {
    cls = Unit::loadClass(class_or_object.toString().get());
  }
  return cls;
}

Array HHVM_FUNCTION(hphp_get_extension_info, const String& name) {
  Array ret;

  Extension *ext = Extension::GetExtension(name);

  ret.set(s_name,      name);
  ret.set(s_version,   ext ? ext->getVersion() : "");
  ret.set(s_info,      empty_string);
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
  if (attrs & AttrPublic)    php_modifier |= 0x100;
  if (attrs & AttrProtected) php_modifier |= 0x200;
  if (attrs & AttrPrivate)   php_modifier |= 0x400;
  return php_modifier;
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

static bool set_source_info(Array &ret, const char *file, int line1,
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

static void set_empty_doc_comment(Array& ret) {
  ret.set(s_doc, false_varNR);
}

static void set_doc_comment(Array& ret,
                            const StringData* comment,
                            bool isBuiltin) {
  if (comment == nullptr || comment->empty()) {
    set_empty_doc_comment(ret);
  } else if (isBuiltin && !f_hphp_debugger_attached()) {
    set_empty_doc_comment(ret);
  } else {
    ret.set(s_doc, VarNR(comment));
  }
}

static void set_return_type_constraint(Array &ret, const StringData* retType) {
  if (retType && retType->size()) {
    ret.set(s_return_type, VarNR(retType));
  } else {
    ret.set(s_return_type, false_varNR);
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

static bool resolveDefaultParameterConstant(const char *value,
                                            int64_t valueLen,
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

static void set_function_info(Array &ret, const Func* func) {
  // return type
  if (func->attrs() & AttrReference) {
    ret.set(s_ref,      true_varNR);
  }
  if (func->isBuiltin()) {
    ret.set(s_internal, true_varNR);
  }
  set_return_type_constraint(ret, func->returnUserType());

  // doc comments
  set_doc_comment(ret, func->docComment(), func->isBuiltin());

  // parameters
  {
    Array arr = Array::Create();
    const Func::ParamInfoVec& params = func->params();
    for (int i = 0; i < func->numParams(); i++) {
      Array param = Array::Create();
      const Func::ParamInfo& fpi = params[i];

      param.set(s_index, VarNR((int)i));
      VarNR name(func->localNames()[i]);
      param.set(s_name, name);

      auto const nonExtendedConstraint =
        fpi.typeConstraint().hasConstraint() &&
        !fpi.typeConstraint().isExtended();
      auto const type = nonExtendedConstraint ? fpi.typeConstraint().typeName()
                                              : empty_string.get();

      param.set(s_type, VarNR(type));
      const StringData* typeHint = fpi.userType() ?
        fpi.userType() : empty_string.get();
      param.set(s_type_hint, VarNR(typeHint));
      param.set(s_function, VarNR(func->name()));
      if (func->preClass()) {
        param.set(s_class, VarNR(func->cls() ? func->cls()->name() :
                                 func->preClass()->name()));
      }
      if (!nonExtendedConstraint || fpi.typeConstraint().isNullable()) {
        param.set(s_nullable, true_varNR);
      }

      if (fpi.phpCode()) {
        assert(fpi.hasDefaultValue());
        if (fpi.hasScalarDefaultValue()) {
          // Most of the time the default value is scalar, so we can
          // avoid evaling in the common case
          param.set(s_default, tvAsVariant((TypedValue*)&fpi.defaultValue()));
        } else {
          // Eval PHP code to get default value. Note that access of
          // undefined class constants can cause the eval() to
          // fatal. Zend lets such fatals propagate, so don't bother catching
          // exceptions here.
          const Variant& v = g_context->getEvaledArg(
            fpi.phpCode(),
            func->cls() ? func->cls()->nameRef() : func->nameRef()
          );
          param.set(s_default, v);
        }
        param.set(s_defaultText, VarNR(fpi.phpCode()));
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
              Object obj = SystemLib::AllocStdClassObject();
              obj->o_set(s_msg, String("Unknown unserializable default value: ")
                                   + defText);
              param.set(s_default, Variant(obj));
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
      {
        Array userAttrs = Array::Create();
        for (auto it = fpi.userAttributes().begin();
             it != fpi.userAttributes().end(); ++it) {
          userAttrs.set(String(const_cast<StringData*>(it->first)),
                        tvAsCVarRef(&it->second));
        }
        param.set(s_attributes, VarNR(userAttrs));
      }
      arr.append(VarNR(param));
    }
    ret.set(s_params, VarNR(arr));
  }

  // static variables
  {
    Array arr = Array::Create();
    const Func::SVInfoVec& staticVars = func->staticVars();
    for (unsigned int i = 0; i < staticVars.size(); i++) {
      const Func::SVInfo &sv = staticVars[i];
      arr.set(VarNR(sv.name), VarNR(sv.phpCode));
    }
    ret.set(s_static_variables, VarNR(arr));
  }

  // user attributes
  {
    Array arr = Array::Create();
    for (auto it = func->userAttributes().begin();
         it != func->userAttributes().end(); ++it) {
      arr.set(String(const_cast<StringData*>(it->first)),
              tvAsCVarRef(&it->second));
    }
    ret.set(s_attributes, VarNR(arr));
  }

  ret.set(s_is_async, func->isAsync());
  ret.set(s_is_closure, func->isClosureBody());
  ret.set(s_is_generator, func->isGenerator());
}

static void set_type_profiling_info(Array& info, const Class* cls,
                                    const Func* method) {
  if (RuntimeOption::EvalRuntimeTypeProfile) {
    VMRegAnchor _;
    if (cls) {
      Func* objMethod = cls->lookupMethod(method->fullName());
      info.set(s_type_profiling, getPercentParamInfoArray(objMethod));
    } else {
      info.set(s_type_profiling, getPercentParamInfoArray(method));
    }
  }
}

static bool isConstructor(const Func* func) {
  PreClass* pcls = func->preClass();
  if (!pcls) return false;
  if (func->cls()) return func == func->cls()->getCtor();
  if (!strcasecmp("__construct", func->name()->data())) return true;
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

static void set_method_prototype_info(Array &ret, const Func *func) {
  const Class *prototypeCls = nullptr;
  if (func->baseCls() != nullptr && func->baseCls() != func->cls()) {
    prototypeCls = func->baseCls();
    const Class *result = get_prototype_class_from_interfaces(
                            prototypeCls, func);
    if (result) prototypeCls = result;
  } else if (func->isMethod()) {
    // lookup the prototype in the interfaces
    prototypeCls = get_prototype_class_from_interfaces(func->cls(), func);
  }
  if (prototypeCls) {
    Array prototype = Array::Create();
    prototype.set(s_class, VarNR(prototypeCls->nameRef()));
    prototype.set(s_name, VarNR(func->nameRef()));
    ret.set(s_prototype, prototype);
  }
}

static void set_method_info(Array &ret, const Func* func, const Class* cls) {
  if (RuntimeOption::EvalRuntimeTypeProfile && !ret.exists(s_type_profiling)) {
    ret.set(s_type_profiling, Array());
  }
  ret.set(s_name, VarNR(func->nameRef()));
  set_attrs(ret, get_modifiers(func->attrs(), false));

  if (isConstructor(func)) {
    ret.set(s_constructor, true_varNR);
  }

  ret.set(s_class, VarNR(func->cls() ? func->cls()->name() :
                         func->preClass()->name()));
  set_function_info(ret, func);
  set_source_info(ret, func->unit()->filepath()->data(),
                  func->line1(), func->line2());
  // If Func* is from a PreClass, it doesn't know about base classes etc.
  // Swap it out for the full version if possible.
  auto resolved_func = cls->lookupMethod(func->name());
  set_method_prototype_info(ret,
                            resolved_func ? resolved_func : func);
}

Array HHVM_FUNCTION(hphp_get_method_info, const Variant& class_or_object,
                    const String &meth_name) {
  auto const cls = get_cls(class_or_object);
  if (!cls) return Array();
  const Func* func = cls->lookupMethod(meth_name.get());
  if (!func) {
    if ((cls->attrs() & AttrAbstract) || (cls->attrs() & AttrInterface)) {
      const Class::InterfaceMap& ifaces = cls->allInterfaces();
      for (int i = 0, size = ifaces.size(); i < size; i++) {
        func = ifaces[i]->lookupMethod(meth_name.get());
        if (func) break;
      }
    }
    if (!func) return Array();
  }
  Array ret;
  set_method_info(ret, func, cls);
  return ret;
}

Array HHVM_FUNCTION(hphp_get_closure_info, const Object& closure) {
  Array mi = HHVM_FN(hphp_get_method_info)(closure->o_getClassName(), s___invoke);
  mi.set(s_name, s_closure_in_braces);
  mi.set(s_closureobj, closure);
  mi.set(s_closure, empty_string);
  mi.remove(s_access);
  mi.remove(s_accessible);
  mi.remove(s_modifiers);
  mi.remove(s_class);

  // grab the use variables and their values
  auto const cls = get_cls(closure);
  Array static_vars = Array::Create();
  for (Slot i = 0; i < cls->numDeclProperties(); ++i) {
    auto const& prop = cls->declProperties()[i];
    auto val = closure.o_get(StrNR(prop.m_name), false /* error */,
                             cls->nameRef());

    // Closure static locals are represented as special instance
    // properties with a mangled name.
    if (prop.m_name->data()[0] == '8') {
      static const char prefix[] = "86static_";
      assert(!strncmp(prop.m_name->data(), prefix, sizeof prefix - 1));
      String strippedName(prop.m_name->data() + sizeof prefix - 1,
                          prop.m_name->size() - sizeof prefix + 1,
                          CopyString);
      static_vars.set(VarNR(strippedName), val);
    } else {
      static_vars.set(VarNR(prop.m_name), val);
    }
  }
  mi.set(s_static_variables, static_vars);

  auto clos = closure.getTyped<c_Closure>();
  if (auto const cls = clos->getClass()) {
    mi.set(s_closure_scope_class, cls->nameRef());
  } else if (auto const thiz = clos->getThis()) {
    mi.set(s_closure_scope_class, thiz->o_getClassName());
  } else {
    mi.set(s_closure_scope_class, null_variant);
  }

  Array &params = mi.lvalAt(s_params).asArrRef();
  for (int i = 0; i < params.size(); i++) {
    params.lvalAt(i).asArrRef().remove(s_class);
  }

  return mi;
}

Array HHVM_FUNCTION(hphp_get_class_info, const Variant& name) {
  auto cls = get_cls(name);
  if (!cls) return Array();

  Array ret;
  ret.set(s_name,      VarNR(cls->name()));
  ret.set(s_extension, empty_string);
  ret.set(s_parent,    cls->parentRef());

  // interfaces
  {
    Array arr = Array::Create();
    for (auto const& interface: cls->declInterfaces()) {
      arr.set(interface->nameRef(), VarNR(1));
    }
    auto const& allIfaces = cls->allInterfaces();
    if (allIfaces.size() > cls->declInterfaces().size()) {
      for (int i = 0; i < allIfaces.size(); ++i) {
        auto const& interface = allIfaces[i];
        arr.set(interface->nameRef(), VarNR(1));
      }
    }
    ret.set(s_interfaces, VarNR(arr));
  }

  // traits
  {
    Array arr = Array::Create();
    for (auto const& traitName : cls->preClass()->usedTraits()) {
      auto& nameRef = *(String*)(&traitName);
      arr.set(nameRef, VarNR(1));
    }
    ret.set(s_traits, VarNR(arr));
  }

  // trait aliases
  {
    Array arr = Array::Create();
    const Class::TraitAliasVec& aliases = cls->traitAliases();
    for (int i = 0, s = aliases.size(); i < s; ++i) {
      arr.set(*(String*)&aliases[i].first, VarNR(aliases[i].second));
    }

    ret.set(s_trait_aliases, VarNR(arr));
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

      Array info = Array::Create();
      if (RuntimeOption::EvalRuntimeTypeProfile) {
        set_type_profiling_info(info, cls, m);
      }
      set_method_info(info, m, cls);
      arr.set(f_strtolower(m->nameRef()), VarNR(info));
    }

    Func* const* clsMethods = cls->methods();
    for (Slot i = cls->traitsBeginIdx();
         i < cls->traitsEndIdx();
         ++i) {
      const Func* m = clsMethods[i];
      if (m->isGenerated()) continue;
      Array info = Array::Create();
      set_method_info(info, m, cls);
      arr.set(f_strtolower(m->nameRef()), VarNR(info));
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
          arrPriv.set(*(String*)(&prop.m_name), VarNR(info));
          arrPrivIdx.set(*(String*)(&prop.m_name), prop.m_idx);
        }
        continue;
      }
      set_instance_prop_info(info, &prop, default_val);
      arr.set(*(String*)(&prop.m_name), VarNR(info));
      arrIdx.set(*(String*)(&prop.m_name), prop.m_idx);
    }

    const Class::SProp* staticProperties = cls->staticProperties();
    const size_t nSProps = cls->numStaticProperties();

    for (Slot i = 0; i < nSProps; ++i) {
      auto const& prop = staticProperties[i];
      Array info = Array::Create();
      if ((prop.m_attrs & AttrPrivate) == AttrPrivate) {
        if (prop.m_class == cls) {
          set_static_prop_info(info, &prop);
          arrPriv.set(*(String*)(&prop.m_name), VarNR(info));
          arrPrivIdx.set(*(String*)(&prop.m_name), prop.m_idx);
        }
        continue;
      }
      set_static_prop_info(info, &prop);
      arr.set(*(String*)(&prop.m_name), VarNR(info));
      arrIdx.set(*(String*)(&prop.m_name), prop.m_idx);
    }

    if (name.isObject()) {
      auto obj = name.toObject();
      if (obj->hasDynProps()) {
        int curIdx = nProps + nSProps;
        for (ArrayIter it(obj->dynPropArray().get()); !it.end(); it.next()) {
          Array info = Array::Create();
          set_dyn_prop_info(info, it.first(), cls->name());
          arr.set(it.first(), VarNR(info));
          arrIdx.set(it.first(), curIdx++);
        }
      }
    }

    ret.set(s_properties, VarNR(arr));
    ret.set(s_private_properties, VarNR(arrPriv));
    ret.set(s_properties_index, VarNR(arrIdx));
    ret.set(s_private_properties_index, VarNR(arrPrivIdx));
    ret.set(s_reorder_parent_properties, false_varNR);
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
        arr.set(consts[i].nameRef(), cellAsCVarRef(value));
      }
    }

    ret.set(s_constants, VarNR(arr));
  }

  { // source info
    const PreClass* pcls = cls->preClass();
    set_source_info(ret, pcls->unit()->filepath()->data(),
                    pcls->line1(), pcls->line2());
    set_doc_comment(ret, pcls->docComment(), pcls->isBuiltin());
  }

  // user attributes
  {
    Array arr = Array::Create();
    const PreClass* pcls = cls->preClass();
    for (auto it = pcls->userAttributes().begin();
         it != pcls->userAttributes().end(); ++it) {
      arr.set(String(const_cast<StringData*>(it->first)),
              tvAsCVarRef(&it->second));
    }
    ret.set(s_attributes, VarNR(arr));
  }

  return ret;
}

Array HHVM_FUNCTION(hphp_get_function_info, const String& name) {
  Array ret;
  if (name.get() == nullptr) return ret;
  const Func* func = Unit::loadFunc(name.get());
  if (!func) return ret;
  ret.set(s_name,       VarNR(func->name()));
  ret.set(s_closure,    empty_string);
  if (RuntimeOption::EvalRuntimeTypeProfile) {
    ret.set(s_type_profiling, getPercentParamInfoArray(func));
  }

  // setting parameters and static variables
  set_function_info(ret, func);
  set_source_info(ret, func->unit()->filepath()->data(),
                  func->line1(), func->line2());
  return ret;
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
  return g_context->createObject(name.get(), params);
}

Object HHVM_FUNCTION(hphp_create_object_without_constructor,
                      const String& name) {
  return g_context->createObject(name.get(), init_null_variant, false);
}

Variant HHVM_FUNCTION(hphp_get_property, const Object& obj, const String& cls,
                                         const String& prop) {
  return obj->o_get(prop, true /* error */, cls);
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
  StringData* sd = cls.get();
  Class* class_ = Unit::lookupClass(sd);
  if (!class_) {
    raise_error("Non-existent class %s", sd->data());
  }
  VMRegAnchor _;
  bool visible, accessible;
  TypedValue* tv = class_->getSProp(
    force ? class_ : arGetContextClass(g_context->getFP()),
    prop.get(), visible, accessible
  );
  if (tv == nullptr) {
    raise_error("Class %s does not have a property named %s",
                sd->data(), prop.get()->data());
  }
  if (!visible || !accessible) {
    raise_error("Invalid access to class %s's property %s",
                sd->data(), prop.get()->data());
  }
  return tvAsVariant(tv);
}

void HHVM_FUNCTION(hphp_set_static_property, const String& cls,
                                             const String& prop, const Variant& value,
                                             bool force) {
  StringData* sd = cls.get();
  Class* class_ = Unit::lookupClass(sd);
  if (!class_) {
    raise_error("Non-existent class %s", sd->data());
  }
  VMRegAnchor _;
  bool visible, accessible;
  TypedValue* tv = class_->getSProp(
    force ? class_ : arGetContextClass(g_context->getFP()),
    prop.get(), visible, accessible
  );
  if (tv == nullptr) {
    raise_error("Class %s does not have a property named %s",
                cls.get()->data(), prop.get()->data());
  }
  if (!visible || !accessible) {
    raise_error("Invalid access to class %s's property %s",
                sd->data(), prop.get()->data());
  }
  tvAsVariant(tv) = value;
}

String HHVM_FUNCTION(hphp_get_original_class_name, const String& name) {
  Class* cls = Unit::loadClass(name.get());
  if (!cls) return empty_string;
  return cls->nameRef();
}

bool HHVM_FUNCTION(hphp_scalar_typehints_enabled) {
  return RuntimeOption::EnableHipHopSyntax;
}

ObjectData* Reflection::AllocReflectionExceptionObject(const Variant& message) {
  ObjectData* inst = ObjectData::newInstance(s_ReflectionExceptionClass);
  TypedValue ret;
  {
    /* Increment refcount across call to ctor, so the object doesn't */
    /* get destroyed when ctor's frame is torn down */
    CountableHelper cnt(inst);
    g_context->invokeFunc(&ret,
                            s_ReflectionExceptionClass->getCtor(),
                            make_packed_array(message),
                            inst);
  }
  tvRefcountedDecRef(&ret);
  return inst;
}

HPHP::Class* Reflection::s_ReflectionExceptionClass = nullptr;

///////////////////////////////////////////////////////////////////////////////

class ReflectionExtension : public Extension {
 public:
  ReflectionExtension() : Extension("reflection", "$Id$") { }
  virtual void moduleInit() {
    HHVM_FE(hphp_create_object);
    HHVM_FE(hphp_create_object_without_constructor);
    HHVM_FE(hphp_get_class_info);
    HHVM_FE(hphp_get_closure_info);
    HHVM_FE(hphp_get_extension_info);
    HHVM_FE(hphp_get_function_info);
    HHVM_FE(hphp_get_method_info);
    HHVM_FE(hphp_get_original_class_name);
    HHVM_FE(hphp_get_property);
    HHVM_FE(hphp_get_static_property);
    HHVM_FE(hphp_invoke);
    HHVM_FE(hphp_invoke_method);
    HHVM_FE(hphp_scalar_typehints_enabled);
    HHVM_FE(hphp_set_property);
    HHVM_FE(hphp_set_static_property);

    loadSystemlib();
    loadSystemlib("reflection-classes");
    loadSystemlib("reflection-internals-functions");

    Reflection::s_ReflectionExceptionClass =
        Unit::lookupClass(s_reflectionexception.get());
  }
} s_reflection_extension;

///////////////////////////////////////////////////////////////////////////////
}
