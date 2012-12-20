/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_reflection.h>
#include <runtime/ext/ext_closure.h>
#include <runtime/base/externals.h>
#include <runtime/base/class_info.h>
#include <runtime/base/source_info.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/string_util.h>
#include <runtime/vm/translator/translator-inline.h>

#include <system/lib/systemlib.h>

namespace HPHP {

using VM::Transl::VMRegAnchor;

IMPLEMENT_DEFAULT_EXTENSION(Reflection);
///////////////////////////////////////////////////////////////////////////////

static StaticString s_name("name");
static StaticString s_version("version");
static StaticString s_info("info");
static StaticString s_ini("ini");
static StaticString s_constants("constants");
static StaticString s_constructor("constructor");
static StaticString s_functions("functions");
static StaticString s_classes("classes");
static StaticString s_access("access");
static StaticString s_public("public");
static StaticString s_protected("protected");
static StaticString s_private("private");
static StaticString s_file("file");
static StaticString s_line1("line1");
static StaticString s_line2("line2");
static StaticString s_doc("doc");
static StaticString s_modifiers("modifiers");
static StaticString s_class("class");
static StaticString s_ref("ref");
static StaticString s_index("index");
static StaticString s_type("type");
static StaticString s_nullable("nullable");
static StaticString s_msg("msg");
static StaticString s_default("default");
static StaticString s_defaultText("defaultText");
static StaticString s_params("params");
static StaticString s_final("final");
static StaticString s_abstract("abstract");
static StaticString s_internal("internal");
static StaticString s_hphp("hphp");
static StaticString s_static_variables("static_variables");
static StaticString s_extension("extension");
static StaticString s_interfaces("interfaces");
static StaticString s_traits("traits");
static StaticString s_interface("interface");
static StaticString s_trait("trait");
static StaticString s_methods("methods");
static StaticString s_properties("properties");
static StaticString s_private_properties("private_properties");
static StaticString s_attributes("attributes");
static StaticString s_function("function");

static StaticString s_trait_aliases("trait_aliases");
static StaticString s_varg("varg");
static StaticString s_closure("closure");
static StaticString s___invoke("__invoke");
static StaticString s_closure_in_braces("{closure}");
static StaticString s_closureobj("closureobj");

static const VM::Class* get_cls(CVarRef class_or_object) {
  VM::Class* cls = NULL;
  if (class_or_object.is(KindOfObject)) {
    ObjectData* obj = class_or_object.toCObjRef().get();
    cls = obj->getVMClass();
  } else {
    cls = VM::Unit::loadClass(class_or_object.toString().get());
  }
  return cls;
}

Array f_hphp_get_extension_info(CStrRef name) {
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

int get_modifiers(VM::Attr attrs, bool cls) {
  int php_modifier = 0;
  if (attrs & VM::AttrAbstract)  php_modifier |= cls ? 0x20 : 0x02;
  if (attrs & VM::AttrFinal)     php_modifier |= cls ? 0x40 : 0x04;
  if (attrs & VM::AttrStatic)    php_modifier |= 0x01;
  if (attrs & VM::AttrPublic)    php_modifier |= 0x100;
  if (attrs & VM::AttrProtected) php_modifier |= 0x200;
  if (attrs & VM::AttrPrivate)   php_modifier |= 0x400;
  return php_modifier;
}

static void set_attrs(Array& ret, int modifiers) {
  if (modifiers & 0x100) {
    ret.set(s_access, VarNR(s_public));
  } else if (modifiers & 0x200) {
    ret.set(s_access, VarNR(s_protected));
  } else if (modifiers & 0x400) {
    ret.set(s_access, VarNR(s_private));
  } else {
    ASSERT(false);
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

static void set_doc_comment(Array &ret, const char *comment) {
  if (comment) {
    ret.set(s_doc, comment);
  } else {
    ret.set(s_doc, false_varNR);
  }
}

static void set_doc_comment(Array &ret, const StringData* comment) {
  if (comment && comment->size()) {
    ret.set(s_doc, VarNR(comment));
  } else {
    ret.set(s_doc, false_varNR);
  }
}

static void set_property_info(Array &ret, ClassInfo::PropertyInfo *info,
                              const ClassInfo *cls) {
  ret.set(s_name, info->name);
  set_attrs(ret, get_modifiers(info->attribute, false) & ~0x66);
  ret.set(s_class, VarNR(cls->getName()));
  set_doc_comment(ret, info->docComment);
}


static void set_instance_prop_info(Array &ret, const VM::Class::Prop* prop) {
  ret.set(s_name, VarNR(prop->m_name));
  set_attrs(ret, get_modifiers(prop->m_attrs, false) & ~0x66);
  ret.set(s_class, VarNR(prop->m_class->name()));
  set_doc_comment(ret, prop->m_docComment);
}

static void set_static_prop_info(Array &ret, const VM::Class::SProp* prop) {
  ret.set(s_name, VarNR(prop->m_name));
  set_attrs(ret, get_modifiers(prop->m_attrs, false) & ~0x66);
  ret.set(s_class, VarNR(prop->m_class->name()));
  set_doc_comment(ret, prop->m_docComment);
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
      if (defText == NULL) defText = "";
      if (!p->type || !*p->type || !strcasecmp("null", defText)) {
        param.set(s_nullable, true_varNR);
      }
      if (p->value && *p->value) {
        if (*p->value == '\x01') {
          const char *sep = strchr(defText, ':');
          Object v(SystemLib::AllocStdClassObject());
          if (sep && sep[1] == ':') {
            String cls = String(defText, sep - defText, CopyString);
            String con = String(sep + 2, CopyString);
            v.o_set(s_class, cls);
            v.o_set(s_name, con);
          } else {
            v.o_set(s_msg, String("unable to eval ") + defText);
          }
          param.set(s_default, v);
        } else {
          param.set(s_default, f_unserialize(p->value));
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
      ASSERT(p->valueText && *p->valueText);
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

static void set_function_info(Array &ret, const VM::Func* func) {
  // return type
  if (func->attrs() & VM::AttrReference) {
    ret.set(s_ref,      true_varNR);
  }
  if (func->isBuiltin()) {
    ret.set(s_internal, true_varNR);
    if (func->info()->attribute & ClassInfo::HipHopSpecific) {
      ret.set(s_hphp,     true_varNR);
    }
  }

  // doc comments
  set_doc_comment(ret, func->docComment());

  // parameters
  {
    Array arr = Array::Create();
    const VM::Func::ParamInfoVec& params = func->params();
    for (int i = 0; i < func->numParams(); i++) {
      Array param = Array::Create();
      const VM::Func::ParamInfo& fpi = params[i];

      param.set(s_index, VarNR((int)i));
      VarNR name(func->localNames()[i]);
      param.set(s_name, name);
      const StringData* type = fpi.typeConstraint().exists() ?
        fpi.typeConstraint().typeName() : empty_string.get();
      param.set(s_type, VarNR(type));
      param.set(s_function, VarNR(func->name()));
      if (func->preClass()) {
        param.set(s_class, VarNR(func->cls() ? func->cls()->name() :
                                 func->preClass()->name()));
      }
      if (!fpi.typeConstraint().exists() ||
          fpi.typeConstraint().nullable()) {
        param.set(s_nullable, true_varNR);
      }

      if (fpi.hasDefaultValue()) {
        if (fpi.hasScalarDefaultValue()) {
          // Most of the time the default value is scalar, so we can
          // avoid evaling in the common case
          param.set(s_default, tvAsVariant((TypedValue*)&fpi.defaultValue()));
        } else {
          // Eval PHP code to get default value. Note that access of
          // undefined class constants can cause the eval() to
          // fatal. Zend lets such fatals propagate, so don't bother catching
          // exceptions here.
          CVarRef v = g_vmContext->getEvaledArg(fpi.phpCode());
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
    const VM::Func::SVInfoVec& staticVars = func->staticVars();
    for (unsigned int i = 0; i < staticVars.size(); i++) {
      const VM::Func::SVInfo &sv = staticVars[i];
      arr.set(VarNR(sv.name), VarNR(sv.phpCode));
    }
    ret.set(s_static_variables, VarNR(arr));
  }

  // user attributes
  {
    Array arr = Array::Create();
    VM::Func::UserAttributeMap::const_iterator it;
    for (it = func->userAttributes().begin();
         it != func->userAttributes().end(); ++it) {
      arr.set(String(const_cast<StringData*>(it->first)),
              tvAsCVarRef(&it->second));
    }
    ret.set(s_attributes, VarNR(arr));
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
}

static bool isConstructor(const VM::Func* func) {
  VM::PreClass* pcls = func->preClass();
  if (!pcls) return false;
  if (func->cls()) return func == func->cls()->getCtor();
  /* A same named function is not a constructor in a trait,
     or if the function was imported from a trait */
  if ((pcls->attrs() | func->attrs()) & VM::AttrTrait) return false;
  if (!strcasecmp("__construct", func->name()->data())) return true;
  return pcls->name()->isame(func->name());
}

static void set_method_info(Array &ret, const VM::Func* func) {
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
  if (hhvm) {
    const VM::Class* c = get_cls(cls);
    if (!c) return Array();
    if (c->clsInfo()) {
      /*
       * Default arguments for builtins arent setup correctly,
       * so use the ClassInfo instead.
       */
      return get_method_info(c->clsInfo(), name);
    }
    CStrRef method_name = name.toString();
    const VM::Func* func = c->lookupMethod(method_name.get());
    if (!func) {
      if (c->attrs() & VM::AttrAbstract) {
        VM::ClassSet::const_iterator it = c->allInterfaces().begin(),
          end = c->allInterfaces().end();
        while (it != end) {
          func = (*it)->lookupMethod(method_name.get());
          if (func) break;
          ++it;
        }
      }
      if (!func) return Array();
    }
    Array ret;
    set_method_info(ret, func);
    return ret;
  } else {
    String className;
    if (cls.isObject()) {
      className = cls.toObject()->o_getClassName();
    } else {
      className = cls.toString();
    }

    const ClassInfo *cls = ClassInfo::FindClassInterfaceOrTrait(className);
    if (cls == NULL) {
      if (!AutoloadHandler::s_instance->invokeHandler(className) ||
          !(cls = ClassInfo::FindClassInterfaceOrTrait(className))) {
        return Array();
      }
    }
    return get_method_info(cls, name);
  }
}

Array f_hphp_get_closure_info(CVarRef closure) {
  if (hhvm) {
    Array mi = f_hphp_get_method_info(closure.toObject()->o_getClassName(),
                                      s___invoke);
    mi.set(s_name, s_closure_in_braces);
    mi.set(s_closureobj, closure);
    mi.set(s_closure, empty_string);
    mi.remove(s_access);
    mi.remove(s_modifiers);
    mi.remove(s_class);

    Array &params = mi.lvalAt(s_params).asArrRef();
    for (int i = 0; i < params.size(); i++) {
      params.lvalAt(i).asArrRef().remove(s_class);
    }

    return mi;
  }

  const c_Closure *c = closure.asCObjRef().getTyped<const c_Closure>();
  Array fi = f_hphp_get_function_info(c->name());
  fi.set(s_name, s_closure_in_braces);
  fi.set(s_closureobj, closure);
  fi.remove(s_varg);
  return fi;
}

Variant f_hphp_get_class_constant(CVarRef cls, CVarRef name) {
  return get_class_constant(cls, name.toString().data());
}

static Array get_class_info(const ClassInfo *cls) {
  Array ret;
  CStrRef className = cls->getName();
  ret.set(s_name,       className);
  ret.set(s_extension,  empty_string);
  ret.set(s_parent,     cls->getParentClass());

  // interfaces
  {
    Array arr = Array::Create();
    const ClassInfo::InterfaceVec &interfaces = cls->getInterfacesVec();
    for (ClassInfo::InterfaceVec::const_iterator iter = interfaces.begin();
         iter != interfaces.end(); ++iter) {
      arr.set(*iter, 1);
    }
    ret.set(s_interfaces, VarNR(arr));
  }

  // traits
  {
    Array arr = Array::Create();
    const ClassInfo::TraitVec &traits = cls->getTraitsVec();
    for (ClassInfo::TraitVec::const_iterator iter = traits.begin();
         iter != traits.end(); ++iter) {
      arr.set(*iter, 1);
    }
    ret.set(s_traits, VarNR(arr));
  }

  // trait aliases
  {
    Array arr = Array::Create();
    const ClassInfo::TraitAliasVec &aliases = cls->getTraitAliasesVec();
    for (ClassInfo::TraitAliasVec::const_iterator iter = aliases.begin();
         iter != aliases.end(); ++iter) {
      arr.set(iter->first, iter->second);
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
    const ClassInfo::MethodVec &methods = cls->getMethodsVec();
    for (ClassInfo::MethodVec::const_iterator iter = methods.begin();
         iter != methods.end(); ++iter) {
      ClassInfo::MethodInfo *m = *iter;
      if ((m->attribute & ClassInfo::IsInherited) == 0) {
        Array info = Array::Create();
        set_method_info(info, m, cls);
        arr.set(StringUtil::ToLower(m->name), info);
      }
    }
    ret.set(s_methods, VarNR(arr));
  }

  // properties
  {
    Array arr = Array::Create();
    Array arrPriv = Array::Create();
    const ClassInfo::PropertyVec &properties = cls->getPropertiesVec();
    for (ClassInfo::PropertyVec::const_iterator iter = properties.begin();
         iter != properties.end(); ++iter) {
      ClassInfo::PropertyInfo *prop = *iter;
      Array info = Array::Create();
      set_property_info(info, prop, cls);
      if (prop->attribute & ClassInfo::IsPrivate) {
        ASSERT(prop->owner == cls);
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
    const ClassInfo::ConstantVec &constants = cls->getConstantsVec();
    for (ClassInfo::ConstantVec::const_iterator iter = constants.begin();
         iter != constants.end(); ++iter) {
      ClassInfo::ConstantInfo *info = *iter;
      if (info->valueText && *info->valueText) {
        arr.set(info->name, info->getValue());
      } else {
        arr.set(info->name, get_class_constant(className, info->name));
      }
    }
    ret.set(s_constants, VarNR(arr));
  }

  { // source info
    if (!set_source_info(ret, cls->getFile(), cls->getLine1(),
                         cls->getLine2())) {
      int line = 0;
      const char *file = SourceInfo::TheSourceInfo.
        getClassDeclaringFile(className, &line);
      set_source_info(ret, file, line, line);
    }
    set_doc_comment(ret, cls->getDocComment());
  }

  // user attributes
  {
    Array arr = Array::Create();
    const ClassInfo::UserAttributeVec &userAttrs = cls->getUserAttributeVec();
    for (ClassInfo::UserAttributeVec::const_iterator iter = userAttrs.begin();
         iter != userAttrs.end(); ++iter) {
      ClassInfo::UserAttributeInfo *info = *iter;
      arr.set(info->name, info->getValue());
    }
    ret.set(s_attributes, VarNR(arr));
  }

  return ret;
}

Array f_hphp_get_class_info(CVarRef name) {
  if (hhvm) {
    const VM::Class* cls = get_cls(name);
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

    typedef vector<VM::ClassPtr> ClassVec;
    // interfaces
    {
      Array arr = Array::Create();
      const ClassVec &interfaces = cls->declInterfaces();
      for (ClassVec::const_iterator it = interfaces.begin(),
             end = interfaces.end(); it != end; ++it) {
        arr.set(it->get()->nameRef(), VarNR(1));
      }
      ret.set(s_interfaces, VarNR(arr));
    }

    // traits
    {
      Array arr = Array::Create();
      const ClassVec &traits = cls->usedTraits();
      for (ClassVec::const_iterator it = traits.begin(),
             end = traits.end(); it != end; ++it) {
        arr.set(it->get()->nameRef(), VarNR(1));
      }
      ret.set(s_traits, VarNR(arr));
    }

    // trait aliases
    {
      Array arr = Array::Create();
      const VM::Class::TraitAliasVec& aliases = cls->traitAliases();
      for (int i = 0, s = aliases.size(); i < s; ++i) {
        arr.set(*(String*)&aliases[i].first, VarNR(aliases[i].second));
      }

      ret.set(s_trait_aliases, VarNR(arr));
    }

    // attributes
    {
      if (false) {
        ret.set(s_internal,  true_varNR);
      }
      if (false && ClassInfo::HipHopSpecific) {
        ret.set(s_hphp,      true_varNR);
      }
      if (cls->attrs() & VM::AttrFinal) {
        ret.set(s_final,     true_varNR);
      }
      if (cls->attrs() & VM::AttrAbstract) {
        ret.set(s_abstract,  true_varNR);
      }
      if (cls->attrs() & VM::AttrInterface) {
        ret.set(s_interface, true_varNR);
      }
      if (cls->attrs() & VM::AttrTrait) {
        ret.set(s_trait,     true_varNR);
      }
      ret.set(s_modifiers, VarNR(get_modifiers(cls->attrs(), true)));
    }

    // methods
    {
      Array arr = Array::Create();
      VM::Func* const* methods = cls->preClass()->methods();
      size_t const numMethods = cls->preClass()->numMethods();
      for (VM::Slot i = 0; i < numMethods; ++i) {
        const VM::Func* m = methods[i];
        if (isdigit(m->name()->data()[0])) continue;
        Array info = Array::Create();
        set_method_info(info, m);
        arr.set(StringUtil::ToLower(m->nameRef()), VarNR(info));
      }

      VM::Func* const* clsMethods = cls->methods();
      for (VM::Slot i = cls->traitsBeginIdx();
          i < cls->traitsEndIdx();
          ++i) {
        const VM::Func* m = clsMethods[i];
        if (isdigit(m->name()->data()[0])) continue;
        Array info = Array::Create();
        set_method_info(info, m);
        arr.set(StringUtil::ToLower(m->nameRef()), VarNR(info));
      }
      ret.set(s_methods, VarNR(arr));
    }

    // properties
    {
      Array arr = Array::Create();
      Array arrPriv = Array::Create();

      const VM::Class::Prop* properties = cls->declProperties();
      const size_t nProps = cls->numDeclProperties();

      for (VM::Slot i = 0; i < nProps; ++i) {
        const VM::Class::Prop& prop = properties[i];
        Array info = Array::Create();
        if ((prop.m_attrs & VM::AttrPrivate) == VM::AttrPrivate) {
          if (prop.m_class == cls) {
            set_instance_prop_info(info, &prop);
            arrPriv.set(*(String*)(&prop.m_name), VarNR(info));
          }
          continue;
        }
        set_instance_prop_info(info, &prop);
        arr.set(*(String*)(&prop.m_name), VarNR(info));
      }

      const VM::Class::SProp* staticProperties = cls->staticProperties();
      const size_t nSProps = cls->numStaticProperties();

      for (VM::Slot i = 0; i < nSProps; ++i) {
        const VM::Class::SProp& prop = staticProperties[i];
        Array info = Array::Create();
        if ((prop.m_attrs & VM::AttrPrivate) == VM::AttrPrivate) {
          if (prop.m_class == cls) {
            set_static_prop_info(info, &prop);
            arrPriv.set(*(String*)(&prop.m_name), VarNR(info));
          }
          continue;
        }
        set_static_prop_info(info, &prop);
        arr.set(*(String*)(&prop.m_name), VarNR(info));
      }

      ret.set(s_properties, VarNR(arr));
      ret.set(s_private_properties, VarNR(arrPriv));
    }

    // constants
    {
      Array arr = Array::Create();

      size_t numConsts = cls->numConstants();
      const VM::Class::Const* consts = cls->constants();

      for (size_t i = 0; i < numConsts; i++) {
        // Note: hphpc doesn't include inherited constants in
        // get_class_constants(), so mimic that behavior
        if (consts[i].m_class == cls) {
          TypedValue* value = cls->clsCnsGet(consts[i].m_name);
          arr.set(consts[i].nameRef(), tvAsVariant(value));
        }
      }

      ret.set(s_constants, VarNR(arr));
    }

    { // source info
      const VM::PreClass* pcls = cls->preClass();
      set_source_info(ret, pcls->unit()->filepath()->data(),
                      pcls->line1(), pcls->line2());
      set_doc_comment(ret, pcls->docComment());
    }

    // user attributes
    {
      Array arr = Array::Create();
      const VM::PreClass* pcls = cls->preClass();
      VM::PreClass::UserAttributeMap::const_iterator it;
      for (it = pcls->userAttributes().begin();
           it != pcls->userAttributes().end(); ++it) {
        arr.set(String(const_cast<StringData*>(it->first)),
                tvAsCVarRef(&it->second));
      }
      ret.set(s_attributes, VarNR(arr));
    }

    return ret;
  }

  String className;
  if (name.isObject()) {
    className = name.toObject()->o_getClassName();
  } else {
    className = name.toString();
  }

  const ClassInfo *cls = ClassInfo::FindClassInterfaceOrTrait(className);
  if (cls == NULL) return Array();
  return get_class_info(cls);
}

Array f_hphp_get_function_info(CStrRef name) {
  Array ret;
  if (hhvm) {
    const VM::Func* func = VM::Unit::loadFunc(name.get());
    if (!func) return ret;
    ret.set(s_name,       VarNR(func->name()));
    ret.set(s_closure,    empty_string);

    // setting parameters and static variables
    set_function_info(ret, func);
    set_source_info(ret, func->unit()->filepath()->data(),
                    func->line1(), func->line2());
    return ret;
  }

  const ClassInfo::MethodInfo *info = ClassInfo::FindFunction(name.data());
  if (info == NULL) {
    if (!AutoloadHandler::s_instance->autoloadFunc(name) ||
        !(info = ClassInfo::FindFunction(name.data()))) {
      return ret;
    }
  }

  ret.set(s_name,       info->name);
  ret.set(s_varg,       (bool)(info->attribute &
                               (ClassInfo::VariableArguments |
                                ClassInfo::RefVariableArguments |
                                ClassInfo::MixedVariableArguments)));
  ret.set(s_closure,    empty_string);

  // setting parameters and static variables
  set_function_info(ret, info, NULL);
  if (!set_source_info(ret, info->file, info->line1, info->line2)) {
    int line = 0;
    const char *file = SourceInfo::TheSourceInfo.
      getFunctionDeclaringFile(name.data(), &line);
    set_source_info(ret, file, line, line);
  }
  return ret;
}

Variant f_hphp_invoke(CStrRef name, CArrRef params) {
  return invoke(name.data(), params);
}

Variant f_hphp_invoke_method(CVarRef obj, CStrRef cls, CStrRef name,
                             CArrRef params) {
  if (!obj.isObject()) {
    if (hhvm) {
      return invoke_static_method(cls, name, params);
    } else {
      return invoke_static_method_bind(cls, name, params);
    }
  }
  if (!hhvm) {
    FrameInjection::GetStackFrame(0)->setStaticClassName(cls);
  }
  ObjectData *o = obj.toCObjRef().get();
  return o->o_invoke(name, params, -1);
}

bool f_hphp_instanceof(CObjRef obj, CStrRef name) {
  return obj.instanceof(name.data());
}

Object f_hphp_create_object(CStrRef name, CArrRef params) {
  if (hhvm) {
    return g_vmContext->createObject(name.get(), params);
  } else {
    return create_object(name, params);
  }
}

Variant f_hphp_get_property(CObjRef obj, CStrRef cls, CStrRef prop) {
  return obj->o_get(prop);
}

void f_hphp_set_property(CObjRef obj, CStrRef cls, CStrRef prop,
                         CVarRef value) {
  obj->o_set(prop, value);
}

Variant f_hphp_get_static_property(CStrRef cls, CStrRef prop) {
  if (hhvm) {
    VM::Class* class_ = VM::Unit::lookupClass(cls.get());
    if (class_ == NULL) {
      raise_error("Non-existent class %s", cls.get()->data());
    }
    VMRegAnchor _;
    bool visible, accessible;
    TypedValue* tv = class_->getSProp(arGetContextClass(
                                        g_vmContext->getFP()),
                                      prop.get(), visible, accessible);
    if (tv == NULL) {
      raise_error("Class %s does not have a property named %s",
                  cls.get()->data(), prop.get()->data());
    }
    if (!visible || !accessible) {
      raise_error("Invalid access to class %s's property %s",
                  cls.get()->data(), prop.get()->data());
    }
    return tvAsVariant(tv);
  } else {
    return get_static_property(cls, prop.data());
  }
}

void f_hphp_set_static_property(CStrRef cls, CStrRef prop, CVarRef value) {
  if (hhvm) {
    VM::Class* class_ = VM::Unit::lookupClass(cls.get());
    if (class_ == NULL) {
      raise_error("Non-existent class %s", cls.get()->data());
    }
    VMRegAnchor _;
    bool visible, accessible;
    TypedValue* tv = class_->getSProp(arGetContextClass(
                                        g_vmContext->getFP()),
                                      prop.get(), visible, accessible);
    if (tv == NULL) {
      raise_error("Class %s does not have a property named %s",
                  cls.get()->data(), prop.get()->data());
    }
    if (!visible || !accessible) {
      raise_error("Invalid access to class %s's property %s",
                  cls.get()->data(), prop.get()->data());
    }
    tvAsVariant(tv) = value;
  } else {
    throw NotImplementedException(__func__);
  }
}

String f_hphp_get_original_class_name(CStrRef name) {
  if (hhvm) {
    VM::Class* cls = VM::Unit::loadClass(name.get());
    if (!cls) return empty_string;
    return cls->nameRef();
  }
  const ClassInfo *cls = ClassInfo::FindClassInterfaceOrTrait(name);
  if (!cls) {
    AutoloadHandler::s_instance->invokeHandler(name);
    cls = ClassInfo::FindClassInterfaceOrTrait(name);
    if (!cls) return empty_string;
  }
  return cls->getName();
}

bool f_hphp_scalar_typehints_enabled() {
  return RuntimeOption::EnableHipHopSyntax;
}

///////////////////////////////////////////////////////////////////////////////
}
