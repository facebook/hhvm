/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_reflection.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_debugger.h"
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

IMPLEMENT_DEFAULT_EXTENSION(Reflection);
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
  s_closure_scope_class("closure_scope_class");

static const Class* get_cls(CVarRef class_or_object) {
  Class* cls = nullptr;
  if (class_or_object.is(KindOfObject)) {
    ObjectData* obj = class_or_object.toCObjRef().get();
    cls = obj->getVMClass();
  } else {
    cls = Unit::loadClass(class_or_object.toString().get());
  }
  return cls;
}

Array f_hphp_get_extension_info(const String& name) {
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

int get_modifiers(int attribute, bool cls) {
  int php_modifier = 0;
  if (attribute & ClassInfo::IsAbstract)  php_modifier |= cls ? 0x20 : 0x02;
  if (attribute & ClassInfo::IsFinal)     php_modifier |= cls ? 0x40 : 0x04;
  if (attribute & ClassInfo::IsStatic)    php_modifier |= 0x01;
  if (attribute & ClassInfo::IsPublic)    php_modifier |= 0x100;
  if (attribute & ClassInfo::IsProtected) php_modifier |= 0x200;
  if (attribute & ClassInfo::IsPrivate)   php_modifier |= 0x400;
  return php_modifier;
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

static void set_doc_comment(Array& ret, const char* comment) {
  if (f_hphp_debugger_attached()) {
    ret.set(s_doc, comment);
  } else {
    set_empty_doc_comment(ret);
  }
}

static void set_return_type_constraint(Array &ret, const StringData* retType) {
  if (retType && retType->size()) {
    ret.set(s_return_type, VarNR(retType));
  } else {
    ret.set(s_return_type, false_varNR);
  }
}

static void set_property_info(Array &ret, ClassInfo::PropertyInfo *info,
                              const ClassInfo *cls) {
  ret.set(s_name, info->name);
  set_attrs(ret, get_modifiers(info->attribute, false) & ~0x66);
  ret.set(s_class, VarNR(cls->getName()));
  set_doc_comment(ret, info->docComment);
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

static void set_function_info(Array &ret, const ClassInfo::MethodInfo *info,
                              const String *classname) {
  // return type
  if (info->attribute & ClassInfo::IsReference) {
    ret.set(s_ref,      true_varNR);
  }
  if (info->attribute & ClassInfo::IsSystem) {
    ret.set(s_internal, true_varNR);
  }
  if (info->attribute & ClassInfo::HipHopSpecific) {
    ret.set(s_hphp,     true_varNR);
  }
  if (info->attribute & ClassInfo::IsClosure) {
    ret.set(s_is_closure, true_varNR);
  }
  if (info->attribute & ClassInfo::HasGeneratorAsBody) {
    ret.set(s_is_generator, true_varNR);
  }

  // doc comments
  set_doc_comment(ret, info->docComment);

  // parameters
  {
    Array arr = Array::Create();
    for (unsigned int i = 0; i < info->parameters.size(); i++) {
      Array param = Array::Create();
      const ClassInfo::ParameterInfo *p = info->parameters[i];
      param.set(s_index, VarNR((int)i));
      param.set(s_name, p->name);
      param.set(s_type, p->type);
      param.set(s_function, info->name);

      if (classname) {
        param.set(s_class, VarNR(*classname));
      }

      const char *defText = p->valueText;
      int64_t defTextLen = p->valueTextLen;
      if (defText == nullptr) {
        defText = "";
        defTextLen = 0;
      }

      if (!p->type || !*p->type || !strcasecmp("null", defText)) {
        param.set(s_nullable, true_varNR);
      }

      if (p->value && *p->value) {
        if (*p->value == '\x01') {
          if ((defTextLen > 2) &&
              !strcmp(defText + defTextLen - 2, "()")) {
            const char *sep = strchr(defText, ':');
            Object obj = SystemLib::AllocStdClassObject();
            if (sep && sep[1] == ':') {
              String cls = String(defText, sep - defText, CopyString);
              String con = String(sep + 2, CopyString);
              obj->o_set(s_class, cls);
              obj->o_set(s_name, con);
            } else {
              obj->o_set(s_name, String(defText, defTextLen, CopyString));
            }
            param.set(s_default, Variant(obj));
          } else {
            Variant v;
            if (resolveDefaultParameterConstant(defText, defTextLen, v)) {
              param.set(s_default, v);
            } else {
              Object obj = SystemLib::AllocStdClassObject();
              obj->o_set(s_msg, String("Unknown unserializable default value: ")
                                   + defText);
              param.set(s_default, Variant(obj));
            }
          }
        } else {
          param.set(s_default, unserialize_from_string(p->value));
        }
        param.set(s_defaultText, defText);
      }

      if (p->attribute & ClassInfo::IsReference) {
        param.set(s_ref, true_varNR);
      }

      {
        Array userAttrs = Array::Create();
        for (unsigned int i = 0; i < p->userAttrs.size(); ++i) {
          const ClassInfo::UserAttributeInfo *ai = p->userAttrs[i];
          userAttrs.set(ai->name, ai->getValue());
        }
        param.set(s_attributes, VarNR(userAttrs));
      }
      arr.append(param);
    }
    ret.set(s_params, arr);
  }

  // static variables
  {
    Array arr = Array::Create();
    for (unsigned int i = 0; i < info->staticVariables.size(); i++) {
      const ClassInfo::ConstantInfo *p = info->staticVariables[i];
      assert(p->valueText && *p->valueText);
      arr.set(p->name, p->valueText);
    }
    ret.set(s_static_variables, arr);
  }

  // user attributes
  {
    Array arr = Array::Create();
    for (unsigned i = 0; i < info->userAttrs.size(); ++i) {
      const ClassInfo::UserAttributeInfo *ai = info->userAttrs[i];
      arr.set(ai->name, ai->getValue());
    }
    ret.set(s_attributes, VarNR(arr));
  }
}

static void set_function_info(Array &ret, const Func* func) {
  // return type
  if (func->attrs() & AttrReference) {
    ret.set(s_ref,      true_varNR);
  }
  if (func->isBuiltin()) {
    ret.set(s_internal, true_varNR);
    if (func->methInfo() &&
        func->methInfo()->attribute & ClassInfo::HipHopSpecific) {
      ret.set(s_hphp,     true_varNR);
    }
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
          CVarRef v = g_vmContext->getEvaledArg(
            fpi.phpCode(),
            func->cls() ? func->cls()->nameRef() : func->nameRef()
          );
          param.set(s_default, v);
        }
        param.set(s_defaultText, VarNR(fpi.phpCode()));
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

  // closure info
  ret.set(s_is_closure, func->isClosureBody());
  // Interestingly this isn't the same as calling isGenerator() because calling
  // isGenerator() on the outside function for a generator returns false.
  ret.set(s_is_generator, func->hasGeneratorAsBody());
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

static const ClassInfo* get_method_prototype_class(ClassInfo::MethodInfo *info,
                                                   const ClassInfo *cls,
                                                   bool lowestLevel) {
  const ClassInfo *parentClassInfo = cls->getParentClassInfo();
  if (parentClassInfo) {
    const ClassInfo *result = get_method_prototype_class(info,
                                                         parentClassInfo,
                                                         false);
    if (result) return result;
  }
  if (!lowestLevel) {
    const ClassInfo::MethodMap& methods = cls->getMethods();
    ClassInfo::MethodMap::const_iterator iter = methods.find(info->name);
    if (iter != methods.end() &&
      !(iter->second->attribute & ClassInfo::IsPrivate)) return cls;
  }
  return nullptr;
}

static const ClassInfo* get_prototype_class_from_interfaces(
  ClassInfo::MethodInfo *info, const ClassInfo *cls, bool lowestLevel) {
  // only looks at the interfaces if the method is public
  if (!(info->attribute & ClassInfo::IsPublic)) return nullptr;

  const ClassInfo::InterfaceVec &interfaces = cls->getInterfacesVec();
  for (unsigned int i = 0; i < interfaces.size(); i++) {
    const ClassInfo *interface = ClassInfo::FindInterface(interfaces[i]);
    if (interface) {
      const ClassInfo *result = get_prototype_class_from_interfaces(info,
                                                                    interface,
                                                                    false);
      if (result) return result;
    }
  }
  if (!lowestLevel) {
    const ClassInfo::MethodMap& methods = cls->getMethods();
    if (methods.find(info->name) != methods.end()) return cls;
  }
  return nullptr;
}

static void set_method_prototype_info(Array &ret, ClassInfo::MethodInfo *info,
                                      const ClassInfo *cls) {
  const ClassInfo *prototypeCls = get_method_prototype_class(info, cls, true);
  if (prototypeCls) {
    const ClassInfo *result = get_prototype_class_from_interfaces(info,
                                                                  prototypeCls,
                                                                  true);
    if (result) prototypeCls = result;
  }
  if (!prototypeCls) {
    prototypeCls = get_prototype_class_from_interfaces(info, cls, true);
  }
  if (prototypeCls) {
    Array prototype = Array::Create();
    prototype.set(s_class, VarNR(prototypeCls->getName()));
    prototype.set(s_name, VarNR(info->name));
    ret.set(s_prototype, prototype);
  }
}

static void set_method_info(Array &ret, ClassInfo::MethodInfo *info,
                            const ClassInfo *cls) {
  ret.set(s_name, info->name);
  set_attrs(ret, get_modifiers(info->attribute, false));
  if (info->attribute & ClassInfo::IsConstructor) {
    ret.set(s_constructor, true_varNR);
  }

  ret.set(s_class, VarNR(cls->getName()));
  set_function_info(ret, info, &cls->getName());
  set_source_info(ret, info->file, info->line1, info->line2);
  set_method_prototype_info(ret, info, cls);
}

static bool isConstructor(const Func* func) {
  PreClass* pcls = func->preClass();
  if (!pcls) return false;
  if (func->cls()) return func == func->cls()->getCtor();
  /* A same named function is not a constructor in a trait,
     or if the function was imported from a trait */
  if ((pcls->attrs() | func->attrs()) & AttrTrait) return false;
  if (!strcasecmp("__construct", func->name()->data())) return true;
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

static void set_method_info(Array &ret, const Func* func) {
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
  set_method_prototype_info(ret, func);
}

static Array get_method_info(const ClassInfo *cls, CVarRef name) {
  if (!cls) return Array();
  ClassInfo *owner;
  ClassInfo::MethodInfo *meth = cls->hasMethod(
    name.toString(), owner,
    cls->getAttribute() & (ClassInfo::IsInterface|ClassInfo::IsAbstract));
  if (!meth) return Array();

  Array ret;
  set_method_info(ret, meth, owner);
  return ret;
}

Array f_hphp_get_method_info(CVarRef cls, CVarRef name) {
  const Class* c = get_cls(cls);
  if (!c) return Array();
  if (c->clsInfo()) {
    /*
     * Default arguments for builtins arent setup correctly,
     * so use the ClassInfo instead.
     */
    return get_method_info(c->clsInfo(), name);
  }
  const String& method_name = name.toString();
  const Func* func = c->lookupMethod(method_name.get());
  if (!func) {
    if ((c->attrs() & AttrAbstract) || (c->attrs() & AttrInterface)) {
      const Class::InterfaceMap& ifaces = c->allInterfaces();
      for (int i = 0, size = ifaces.size(); i < size; i++) {
        func = ifaces[i]->lookupMethod(method_name.get());
        if (func) break;
      }
    }
    if (!func) return Array();
  }
  Array ret;
  set_method_info(ret, func);
  return ret;
}

Array f_hphp_get_closure_info(CVarRef closure) {
  auto const asObj = closure.toObject();

  Array mi = f_hphp_get_method_info(asObj->o_getClassName(), s___invoke);
  mi.set(s_name, s_closure_in_braces);
  mi.set(s_closureobj, closure);
  mi.set(s_closure, empty_string);
  mi.remove(s_access);
  mi.remove(s_accessible);
  mi.remove(s_modifiers);
  mi.remove(s_class);

  // grab the use variables and their values
  auto const cls = get_cls(asObj);
  Array static_vars = Array::Create();
  for (Slot i = 0; i < cls->numDeclProperties(); ++i) {
    auto const& prop = cls->declProperties()[i];
    auto val = asObj.o_get(StrNR(prop.m_name), true /* error */,
                           cls->nameRef());
    static_vars.set(VarNR(prop.m_name), val);
  }
  mi.set(s_static_variables, static_vars);

  auto clos = asObj.getTyped<c_Closure>();
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

Variant f_hphp_get_class_constant(CVarRef cls, CVarRef name) {
  return cellAsCVarRef(
    g_vmContext->lookupClsCns(cls.toString().get(),
                              name.toString().get())
  );
}

static Array get_class_info(const ClassInfo *cls) {
  Array ret;
  const String& className = cls->getName();
  ret.set(s_name,       className);
  ret.set(s_extension,  empty_string);
  ret.set(s_parent,     cls->getParentClass());

  // interfaces
  {
    Array arr = Array::Create();
    for (auto const& inter : cls->getInterfacesVec()) {
      arr.set(inter, 1);
    }
    ret.set(s_interfaces, VarNR(arr));
  }

  // traits
  {
    Array arr = Array::Create();
    for (auto const& trait : cls->getTraitsVec()) {
      arr.set(trait, 1);
    }
    ret.set(s_traits, VarNR(arr));
  }

  // trait aliases
  {
    Array arr = Array::Create();
    for (auto const& alias : cls->getTraitAliasesVec()) {
      arr.set(alias.first, alias.second);
    }
    ret.set(s_trait_aliases, VarNR(arr));
  }

  // attributes
  {
    int attribute = cls->getAttribute();
    if (attribute & ClassInfo::IsSystem) {
      ret.set(s_internal,  true_varNR);
    }
    if (attribute & ClassInfo::HipHopSpecific) {
      ret.set(s_hphp,      true_varNR);
    }
    if (attribute & ClassInfo::IsFinal) {
      ret.set(s_final,     true_varNR);
    }
    if (attribute & ClassInfo::IsAbstract) {
      ret.set(s_abstract,  true_varNR);
    }
    if (attribute & ClassInfo::IsInterface) {
      ret.set(s_interface, true_varNR);
    }
    if (attribute & ClassInfo::IsTrait) {
      ret.set(s_trait,     true_varNR);
    }
    ret.set(s_modifiers, VarNR(get_modifiers(attribute, true)));
  }

  // methods
  {
    Array arr = Array::Create();
    for (auto const& meth : cls->getMethodsVec()) {
      Array info = Array::Create();
      set_method_info(info, meth, cls);
      arr.set(f_strtolower(meth->name), info);
    }
    ret.set(s_methods, VarNR(arr));
  }

  // properties
  {
    Array arr = Array::Create();
    Array arrPriv = Array::Create();
    for (auto const& prop : cls->getPropertiesVec()) {
      Array info = Array::Create();
      info.add(s_default, true_varNR);
      set_property_info(info, prop, cls);

      if (prop->attribute & ClassInfo::IsPrivate) {
        assert(prop->owner == cls);
        arrPriv.set(prop->name, info);
      } else {
        arr.set(prop->name, info);
      }
    }
    ret.set(s_properties, VarNR(arr));
    ret.set(s_private_properties, VarNR(arrPriv));
  }

  // constants
  {
    Array arr = Array::Create();
    for (auto const& cons : cls->getConstantsVec()) {
      arr.set(cons->name, cons->getValue());
    }
    ret.set(s_constants, VarNR(arr));
  }

  { // source info
    set_source_info(ret, cls->getFile(), cls->getLine1(),
                    cls->getLine2());
    set_doc_comment(ret, cls->getDocComment());
  }

  // user attributes
  {
    Array arr = Array::Create();
    for (auto const& user_attr : cls->getUserAttributeVec()) {
      arr.set(user_attr->name, user_attr->getValue());
    }
    ret.set(s_attributes, VarNR(arr));
  }

  return ret;
}

Array f_hphp_get_class_info(CVarRef name) {
  const Class* cls = get_cls(name);
  if (!cls) return Array();
  if (cls->clsInfo()) {
    /*
     * Default arguments for builtins arent setup correctly,
     * so use the ClassInfo instead.
     */
    return get_class_info(cls->clsInfo());
  }

  Array ret;
  ret.set(s_name,      VarNR(cls->name()));
  ret.set(s_extension, empty_string);
  ret.set(s_parent,    cls->parentRef());

  // interfaces
  {
    Array arr = Array::Create();
    for (auto const& interface : cls->declInterfaces()) {
      arr.set(interface->nameRef(), VarNR(1));
    }
    ret.set(s_interfaces, VarNR(arr));
  }

  // traits
  {
    Array arr = Array::Create();
    for (auto const& trait : cls->usedTraits()) {
      arr.set(trait->nameRef(), VarNR(1));
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
    Func* const* methods = cls->preClass()->methods();
    size_t const numMethods = cls->preClass()->numMethods();
    for (Slot i = 0; i < numMethods; ++i) {
      const Func* m = methods[i];
      if (m->isGenerated()) continue;
      Array info = Array::Create();
      if (RuntimeOption::EvalRuntimeTypeProfile) {
        set_type_profiling_info(info, cls, m);
      }
      set_method_info(info, m);
      arr.set(f_strtolower(m->nameRef()), VarNR(info));
    }

    Func* const* clsMethods = cls->methods();
    for (Slot i = cls->traitsBeginIdx();
         i < cls->traitsEndIdx();
         ++i) {
      const Func* m = clsMethods[i];
      if (m->isGenerated()) continue;
      Array info = Array::Create();
      set_method_info(info, m);
      arr.set(f_strtolower(m->nameRef()), VarNR(info));
    }
    ret.set(s_methods, VarNR(arr));
  }

  // properties
  {
    Array arr = Array::Create();
    Array arrPriv = Array::Create();

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
        }
        continue;
      }
      set_instance_prop_info(info, &prop, default_val);
      arr.set(*(String*)(&prop.m_name), VarNR(info));
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
        }
        continue;
      }
      set_static_prop_info(info, &prop);
      arr.set(*(String*)(&prop.m_name), VarNR(info));
    }

    if (name.isObject()) {
      auto obj = name.toObject();
      if (obj->hasDynProps()) {
        for (ArrayIter it(obj->dynPropArray().get()); !it.end(); it.next()) {
          Array info = Array::Create();
          set_dyn_prop_info(info, it.first(), cls->name());
          arr.set(it.first(), VarNR(info));
        }
      }
    }

    ret.set(s_properties, VarNR(arr));
    ret.set(s_private_properties, VarNR(arrPriv));
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

Array f_hphp_get_function_info(const String& name) {
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

Variant f_hphp_invoke(const String& name, CVarRef params) {
  return invoke(name.data(), params);
}

Variant f_hphp_invoke_method(CVarRef obj, const String& cls, const String& name,
                             CVarRef params) {
  if (!obj.isObject()) {
    return invoke_static_method(cls, name, params);
  }
  ObjectData *o = obj.toCObjRef().get();
  return o->o_invoke(name, params);
}

bool f_hphp_instanceof(CObjRef obj, const String& name) {
  return obj.instanceof(name.data());
}

Object f_hphp_create_object(const String& name, CVarRef params) {
  return g_vmContext->createObject(name.get(), params);
}

Object f_hphp_create_object_without_constructor(const String& name) {
  return g_vmContext->createObject(name.get(), init_null_variant, false);
}

Variant f_hphp_get_property(CObjRef obj, const String& cls, const String& prop) {
  return obj->o_get(prop, true /* error */, cls);
}

void f_hphp_set_property(CObjRef obj, const String& cls, const String& prop,
                         CVarRef value) {
  if (!cls.empty() && RuntimeOption::RepoAuthoritative) {
    raise_error(
      "We've already made many assumptions about private variables. "
      "You can't change accessibility in Whole Program mode"
    );
  }
  obj->o_set(prop, value, cls);
}

Variant f_hphp_get_static_property(const String& cls, const String& prop, bool force) {
  StringData* sd = cls.get();
  Class* class_ = Unit::lookupClass(sd);
  if (!class_) {
    raise_error("Non-existent class %s", sd->data());
  }
  VMRegAnchor _;
  bool visible, accessible;
  TypedValue* tv = class_->getSProp(
    force ? class_ : arGetContextClass(g_vmContext->getFP()),
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

void f_hphp_set_static_property(const String& cls, const String& prop, CVarRef value,
                                bool force) {
  StringData* sd = cls.get();
  Class* class_ = Unit::lookupClass(sd);
  if (!class_) {
    raise_error("Non-existent class %s", sd->data());
  }
  VMRegAnchor _;
  bool visible, accessible;
  TypedValue* tv = class_->getSProp(
    force ? class_ : arGetContextClass(g_vmContext->getFP()),
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

String f_hphp_get_original_class_name(const String& name) {
  Class* cls = Unit::loadClass(name.get());
  if (!cls) return empty_string;
  return cls->nameRef();
}

bool f_hphp_scalar_typehints_enabled() {
  return RuntimeOption::EnableHipHopSyntax;
}

///////////////////////////////////////////////////////////////////////////////
}
