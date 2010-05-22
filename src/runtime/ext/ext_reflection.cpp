/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <system/gen/php/classes/reflection.h>
#include <runtime/base/string_util.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(Reflection);
///////////////////////////////////////////////////////////////////////////////

Array f_hphp_get_extension_info(CStrRef name) {
  Array ret;

  ret.set("name",      name);
  ret.set("version",   "");
  ret.set("info",      "");
  ret.set("ini",       Array::Create());
  ret.set("constants", Array::Create());
  ret.set("functions", Array::Create());
  ret.set("classes",   Array::Create());

  return ret;
}

static void set_access(Array &ret, int attribute) {
  if (attribute & ClassInfo::IsPublic) {
    ret.set("access", "public");
  } else if (attribute & ClassInfo::IsProtected) {
    ret.set("access", "protected");
  } else if (attribute & ClassInfo::IsPrivate) {
    ret.set("access", "private");
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

static void set_property_info(Array &ret, ClassInfo::PropertyInfo *info,
                              const ClassInfo *cls) {
  ret.set("name", info->name);
  set_access(ret, info->attribute);
  ret.set("modifiers", get_modifiers(info->attribute, false));
  ret.set("static", (bool)(info->attribute & ClassInfo::IsStatic));
  ret.set("class", cls->getName());
  const char *dc = info->docComment;
  if (dc) {
    ret.set("doc", dc);
  } else {
    ret.set("doc", false);
  }
}

static void set_function_info(Array &ret, const ClassInfo::MethodInfo *info,
                              const char *classname) {
  // return type
  ret.set("ref", (bool)(info->attribute & ClassInfo::IsReference));

  const char *dc = info->docComment;
  if (dc) {
    ret.set("doc", dc);
  } else {
    ret.set("doc", false);
  }

  // parameters
  {
    Array arr = Array::Create();
    for (unsigned int i = 0; i < info->parameters.size(); i++) {
      Array param = Array::Create();
      const ClassInfo::ParameterInfo *p = info->parameters[i];
      param.set("index", (int)i);
      param.set("name", p->name);
      param.set("type", p->type);
      if (classname) {
        param.set("class", classname);
      }
      if (p->type && *p->type) {
        param.set("nullable", false);
      } else {
        param.set("nullable", true);
      }
      if (p->value && *p->value) {
        ASSERT(p->attribute & ClassInfo::IsOptional);
        param.set("default", f_unserialize(p->value));
      } else {
        ASSERT((p->attribute & ClassInfo::IsOptional) == 0);
      }
      param.set("ref", (bool)(p->attribute & ClassInfo::IsReference));
      arr.append(param);
    }
    ret.set("params", arr);
  }

  // static variables
  {
    Array arr = Array::Create();
    for (unsigned int i = 0; i < info->staticVariables.size(); i++) {
      const ClassInfo::ConstantInfo *p = info->staticVariables[i];
      ASSERT(p->valueText && *p->valueText);
      arr.set(p->name, p->valueText);
    }
    ret.set("static_variables", arr);
  }
}

static void set_method_info(Array &ret, ClassInfo::MethodInfo *info,
                            const ClassInfo *cls) {
  ret.set("name", info->name);
  set_access(ret, info->attribute);
  ret.set("modifiers", get_modifiers(info->attribute, false));
  ret.set("static",   (bool)(info->attribute & ClassInfo::IsStatic));
  ret.set("final",    (bool)(info->attribute & ClassInfo::IsFinal));
  ret.set("abstract", (bool)(info->attribute & ClassInfo::IsAbstract));
  ret.set("internal", (bool)(cls->getAttribute() & ClassInfo::IsSystem));
  ret.set("class", cls->getName());
  set_function_info(ret, info, cls->getName());
}

Array f_hphp_get_class_info(CVarRef name) {
  String className;
  if (name.isObject()) {
    className = name.toObject()->o_getClassName();
  } else {
    className = name.toString();
  }

  const ClassInfo *cls = ClassInfo::FindClass(className.data());
  if (cls == NULL) {
    cls = ClassInfo::FindInterface(className.data());
  }

  Array ret;
  if (cls == NULL) {
    return ret;
  }

  ret.set("name",       cls->getName());
  ret.set("extension",  "");
  ret.set("parent",     cls->getParentClass());

  // interfaces
  {
    Array arr = Array::Create();
    const ClassInfo::InterfaceMap &interfaces = cls->getInterfaces();
    for (ClassInfo::InterfaceMap::const_iterator iter = interfaces.begin();
         iter != interfaces.end(); ++iter) {
      arr.set(*iter, 1);
    }
    ret.set("interfaces", arr);
  }

  // attributes
  {
    int attribute = cls->getAttribute();
    ret.set("internal",   (bool)(attribute & ClassInfo::IsSystem));
    ret.set("abstract",   (bool)(attribute & ClassInfo::IsAbstract));
    ret.set("interface",  (bool)(attribute & ClassInfo::IsInterface));
    ret.set("final",      (bool)(attribute & ClassInfo::IsFinal));
    ret.set("modifiers",  get_modifiers(attribute, true));
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
    ret.set("methods", arr);
  }

  // properties
  {
    Array arr = Array::Create();
    const ClassInfo::PropertyMap &properties = cls->getProperties();
    for (ClassInfo::PropertyMap::const_iterator iter = properties.begin();
         iter != properties.end(); ++iter) {
      Array info = Array::Create();
      set_property_info(info, iter->second, cls);
      arr.set(iter->first, info);
    }
    ret.set("properties", arr);
  }

  // constants
  {
    Array arr = Array::Create();
    const ClassInfo::ConstantMap &constants = cls->getConstants();
    for (ClassInfo::ConstantMap::const_iterator iter = constants.begin();
         iter != constants.end(); ++iter) {
      if (iter->second->valueText && *iter->second->valueText) {
        arr.set(iter->second->name, iter->second->value);
      } else {
        arr.set(iter->second->name,
                get_class_constant(className.data(), iter->second->name));
      }
    }
    ret.set("constants", arr);
  }

  { // source info
    const char *file =
      SourceInfo::TheSourceInfo.getClassDeclaringFile(className.data());
    if (!file) file = "";
    if (file[0] != '/') {
      ret.set("file", String(RuntimeOption::SourceRoot + file));
    } else {
      ret.set("file", file);
    }
    ret.set("line1", 0);
    ret.set("line2", 0);
    const char *dc = cls->getDocComment();
    if (dc) {
      ret.set("doc", dc);
    } else {
      ret.set("doc", false);
    }
  }

  return ret;
}

Array f_hphp_get_function_info(CStrRef name) {
  Array ret;

  const ClassInfo::MethodInfo *info = ClassInfo::FindFunction(name.data());
  if (info == NULL) {
    return ret;
  }

  ret.set("name",       info->name);
  ret.set("internal",   (bool)(info->attribute & ClassInfo::IsSystem));
  ret.set("closure",    "");

  // setting parameters and static variables
  set_function_info(ret, info, NULL);

  const char *file =
    SourceInfo::TheSourceInfo.getFunctionDeclaringFile(name.data());
  if (!file) file = "";
  if (file[0] != '/') {
    ret.set("file", String(RuntimeOption::SourceRoot + file));
  } else {
    ret.set("file", file);
  }
  ret.set("line1", 0);
  ret.set("line2", 0);
  const char *dc = info->docComment;
  if (dc) {
    ret.set("doc", dc);
  } else {
    ret.set("doc", false);
  }

  return ret;
}

Variant f_hphp_invoke(CStrRef name, CArrRef params) {
  return invoke(name.data(), params);
}

Variant f_hphp_invoke_method(CVarRef obj, CStrRef cls, CStrRef name,
                             CArrRef params) {
  if (!obj.isObject()) {
    return invoke_static_method(cls.data(), name.data(), params);
  }
  return obj.toObject()->o_invoke(name.data(), params, -1);
}

bool f_hphp_instanceof(CObjRef obj, CStrRef name) {
  return obj.instanceof(name.data());
}

Object f_hphp_create_object(CStrRef name, CArrRef params) {
  return create_object(name.data(), params);
}

Variant f_hphp_get_property(CObjRef obj, CStrRef cls, CStrRef prop) {
  return obj->o_get(prop.data(), -1);
}

void f_hphp_set_property(CObjRef obj, CStrRef cls, CStrRef prop,
                         CVarRef value) {
  obj->o_set(prop.data(), -1, value);
}

Variant f_hphp_get_static_property(CStrRef cls, CStrRef prop) {
  return get_static_property(cls.data(), prop.data());
}

void f_hphp_set_static_property(CStrRef cls, CStrRef prop, CVarRef value) {
  throw NotImplementedException(__func__);
}

///////////////////////////////////////////////////////////////////////////////
}
