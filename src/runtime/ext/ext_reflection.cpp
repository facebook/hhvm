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
#include <runtime/base/externals.h>
#include <runtime/base/class_info.h>
#include <runtime/base/source_info.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/string_util.h>

#include <system/lib/systemlib.h>

namespace HPHP {
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
static StaticString s_static("static");
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
static StaticString s_parent("parent");
static StaticString s_interfaces("interfaces");
static StaticString s_traits("traits");
static StaticString s_interface("interface");
static StaticString s_trait("trait");
static StaticString s_methods("methods");
static StaticString s_properties("properties");

static StaticString s_trait_aliases("trait_aliases");
static StaticString s_varg("varg");
static StaticString s_closure("closure");

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

static void set_access(Array &ret, int attribute) {
  if (attribute & ClassInfo::IsPublic) {
    ret.set(s_access, s_public);
  } else if (attribute & ClassInfo::IsProtected) {
    ret.set(s_access, s_protected);
  } else if (attribute & ClassInfo::IsPrivate) {
    ret.set(s_access, s_private);
  } else {
    ASSERT(false);
  }
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

static void set_property_info(Array &ret, ClassInfo::PropertyInfo *info,
                              const ClassInfo *cls) {
  ret.set(s_name, info->name);
  set_access(ret, info->attribute);
  ret.set(s_modifiers, VarNR(get_modifiers(info->attribute, false)));
  if (info->attribute & ClassInfo::IsStatic) {
    ret.set(s_static, true_varNR);
  }
  ret.set(s_class, VarNR(cls->getName()));
  set_doc_comment(ret, info->docComment);
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
      if (classname) {
        param.set(s_class, VarNR(*classname));
      }
      const char *defText = p->valueText;
      if (defText == NULL) defText = "";
      if (!p->type || !*p->type || !strcasecmp("null", defText)) {
        param.set(s_nullable, true_varNR);
      }
      if (p->value && *p->value) {
        ASSERT(p->attribute & ClassInfo::IsOptional);
        if (*p->value == '\x01') {
          Object v(SystemLib::AllocStdClassObject());
          v.o_set(s_msg, String("unable to eval ") + defText);
          param.set(s_default, v);
        } else {
          param.set(s_default, f_unserialize(p->value));
        }
        param.set(s_defaultText, defText);
      } else {
        ASSERT((p->attribute & ClassInfo::IsOptional) == 0);
      }
      if (p->attribute & ClassInfo::IsReference) {
        param.set(s_ref, true_varNR);
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
}

static void set_method_info(Array &ret, ClassInfo::MethodInfo *info,
                            const ClassInfo *cls) {
  ret.set(s_name, info->name);
  set_access(ret, info->attribute);
  ret.set(s_modifiers, VarNR(get_modifiers(info->attribute, false)));

  if (info->attribute & ClassInfo::IsStatic) {
    ret.set(s_static,    true_varNR);
  }
  if (info->attribute & ClassInfo::IsFinal) {
    ret.set(s_final,     true_varNR);
  }
  if (info->attribute & ClassInfo::IsAbstract) {
    ret.set(s_abstract,  true_varNR);
  }
  if (info->attribute & ClassInfo::IsConstructor) {
    ret.set(s_constructor, true_varNR);
  }

  ret.set(s_class, VarNR(cls->getName()));
  set_function_info(ret, info, &cls->getName());
  set_source_info(ret, info->file, info->line1, info->line2);
}

Array f_hphp_get_method_info(CVarRef cname, CVarRef name) {
  String className;
  if (cname.isObject()) {
    className = cname.toObject()->o_getClassName();
  } else {
    className = cname.toString();
  }

  Array ret;

  const ClassInfo *cls = ClassInfo::FindClassInterfaceOrTrait(className);
  if (!cls) return ret;

  ClassInfo *owner;
  ClassInfo::MethodInfo *meth = cls->hasMethod(
    name.toString(), owner,
    cls->getAttribute() & (ClassInfo::IsInterface|ClassInfo::IsAbstract));
  if (!meth) return ret;

  set_method_info(ret, meth, owner);
  return ret;
}

Array f_hphp_get_class_info(CVarRef name) {
  String className;
  if (name.isObject()) {
    className = name.toObject()->o_getClassName();
  } else {
    className = name.toString();
  }

  Array ret;

  const ClassInfo *cls = ClassInfo::FindClassInterfaceOrTrait(className);
  if (cls == NULL) {
    return ret;
  }

  ret.set(s_name,       cls->getName());
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
    const ClassInfo::PropertyVec &properties = cls->getPropertiesVec();
    for (ClassInfo::PropertyVec::const_iterator iter = properties.begin();
         iter != properties.end(); ++iter) {
      ClassInfo::PropertyInfo *prop = *iter;
      Array info = Array::Create();
      set_property_info(info, prop, cls);
      arr.set(prop->name, info);
    }
    ret.set(s_properties, VarNR(arr));
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

  return ret;
}

Array f_hphp_get_function_info(CStrRef name) {
  Array ret;

  const ClassInfo::MethodInfo *info = ClassInfo::FindFunction(name.data());
  if (info == NULL) {
    return ret;
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
    return invoke_static_method(cls, name, params);
  }
  return obj.toObject()->o_invoke(name, params, -1);
}

bool f_hphp_instanceof(CObjRef obj, CStrRef name) {
  return obj.instanceof(name.data());
}

Object f_hphp_create_object(CStrRef name, CArrRef params) {
  if (hhvm) {
    return g_context->createObject(name.get(), params);
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
    VM::Class* class_ = g_context->lookupClass(cls.get());
    if (class_ == NULL) {
      raise_error("Non-existent class %s", cls.get()->data());
    }
    bool visible, accessible;
    TypedValue* tv = class_->getSProp(g_context->m_fp->m_func->m_preClass,
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
    VM::Class* class_ = g_context->lookupClass(cls.get());
    if (class_ == NULL) {
      raise_error("Non-existent class %s", cls.get()->data());
    }
    bool visible, accessible;
    TypedValue* tv = class_->getSProp(g_context->m_fp->m_func->m_preClass,
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
  const ClassInfo *cls = ClassInfo::FindClassInterfaceOrTrait(name);
  if (!cls) {
    AutoloadHandler::s_instance->invokeHandler(name);
    cls = ClassInfo::FindClassInterfaceOrTrait(name);
    if (!cls) return empty_string;
  }
  return cls->getName();
}

///////////////////////////////////////////////////////////////////////////////
}
