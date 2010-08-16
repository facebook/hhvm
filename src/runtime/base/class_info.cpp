/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/class_info.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/externals.h>
#include <runtime/base/hphp_system.h>
#include <runtime/base/variable_unserializer.h>
#include <util/util.h>
#include <util/lock.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

Mutex ClassInfo::s_mutex;
bool ClassInfo::s_loaded = false;
ClassInfo *ClassInfo::s_systemFuncs = NULL;
ClassInfo *ClassInfo::s_userFuncs = NULL;
ClassInfo::ClassMap ClassInfo::s_classes;
ClassInfo::ClassMap ClassInfo::s_interfaces;
ClassInfoHook *ClassInfo::s_hook = NULL;

Array ClassInfo::GetSystemFunctions() {
  if (!s_loaded) Load();

  Array ret = Array::Create();
  if (s_systemFuncs) {
    const MethodVec &methods = s_systemFuncs->getMethodsVec();
    for (MethodVec::const_iterator iter = methods.begin();
         iter != methods.end(); ++iter) {
      ret.append((*iter)->name);
    }
  }
  return ret;
}

Array ClassInfo::GetUserFunctions() {
  if (!s_loaded) Load();

  Array ret = Array::Create();
  if (s_userFuncs) {
    const MethodVec &methods = s_userFuncs->getMethodsVec();
    for (MethodVec::const_iterator iter = methods.begin();
         iter != methods.end(); ++iter) {
      ret.append((*iter)->name);
    }
  }
  if (s_hook) {
    Array dyn = s_hook->getUserFunctions();
    if (!dyn.isNull()) {
      ret += dyn;
    }
  }
  return ret;
}

const ClassInfo::MethodInfo *ClassInfo::FindFunction(const char *name) {
  ASSERT(name);
  if (!s_loaded) Load();

  const MethodInfo *ret = s_systemFuncs->getMethodInfo(name);
  if (ret == NULL && s_hook) {
    ret = s_hook->findFunction(name);
  }
  if (ret == NULL) {
    ret = s_userFuncs->getMethodInfo(name);
  }
  return ret;
}

Array ClassInfo::GetClasses(bool declaredOnly) {
  if (!s_loaded) Load();

  Array ret;
  for (ClassMap::const_iterator iter = s_classes.begin();
       iter != s_classes.end(); ++iter) {
    if (!declaredOnly || iter->second->isDeclared()) {
      ret.append(iter->first);
    }
  }
  if (s_hook) {
    Array dyn = s_hook->getClasses();
    if (!dyn.isNull()) {
      ret += dyn;
    }
  }
  return ret;
}

bool ClassInfo::HasClass(const char *name) {
  ASSERT(name);
  if (!s_loaded) Load();

  if (s_hook && s_hook->findClass(name)) return true;

  return s_classes.find(name) != s_classes.end();
}

const ClassInfo *ClassInfo::FindClass(const char *name) {
  ASSERT(name);
  if (!s_loaded) Load();

  if (s_hook) {
    const ClassInfo *cl = s_hook->findClass(name);
    if (cl) return cl;
  }
  ClassMap::const_iterator iter = s_classes.find(name);
  if (iter != s_classes.end()) {
    return iter->second;
  }
  return NULL;
}

Array ClassInfo::GetInterfaces(bool declaredOnly) {
  if (!s_loaded) Load();

  Array ret;
  for (ClassMap::const_iterator iter = s_interfaces.begin();
       iter != s_interfaces.end(); ++iter) {
    if (!declaredOnly || iter->second->isDeclared()) {
      ret.append(iter->first);
    }
  }
  if (s_hook) {
    Array dyn = s_hook->getInterfaces();
    if (!dyn.isNull()) {
      ret += dyn;
    }
  }
  return ret;
}

bool ClassInfo::HasInterface(const char *name) {
  ASSERT(name);
  if (!s_loaded) Load();

  if (s_hook && s_hook->findInterface(name)) return true;
  return s_interfaces.find(name) != s_interfaces.end();
}

// NOTE: FindInterface() currently cannot find interfaces redeclared by
// classes.
const ClassInfo *ClassInfo::FindInterface(const char *name) {
  ASSERT(name);
  if (!s_loaded) Load();

  if (s_hook) {
    const ClassInfo *iface = s_hook->findInterface(name);
    if (iface) return iface;
  }
  ClassMap::const_iterator iter = s_interfaces.find(name);
  if (iter != s_interfaces.end()) {
    return iter->second;
  }
  return NULL;
}

const ClassInfo::ConstantInfo *ClassInfo::FindConstant(const char *name) {
  ASSERT(name);
  if (!s_loaded) Load();
  const ConstantInfo *info;
  info = s_systemFuncs->getConstantInfo(name);
  if (info) return info;
  if (s_hook) {
    info = s_hook->findConstant(name);
    if (info) return info;
  }
  info = s_userFuncs->getConstantInfo(name);
  return info;
}

Array ClassInfo::GetConstants() {
  if (!s_loaded) Load();
  Array res;
  Array dyn;
  {
    const ConstantMap &scm = s_systemFuncs->getConstants();
    for (ConstantMap::const_iterator it = scm.begin(); it != scm.end(); ++it) {
      res.set(it->first, it->second->value);
    }
  }
  {
    const ConstantMap &ucm = s_userFuncs->getConstants();
    for (ConstantMap::const_iterator it = ucm.begin(); it != ucm.end(); ++it) {
      res.set(it->first, it->second->value);
    }
  }
  if (s_hook) {
    dyn = s_hook->getConstants();
    if (!dyn.isNull()) {
      res += dyn;
    }
  }
  dyn = get_globals()->getDynamicConstants();
  if (!dyn.isNull()) {
    res += dyn;
  }
  return res;
}

void ClassInfo::GetClassMethods(MethodVec &ret, const char *classname,
                                int type /* = 0 */) {
  if (classname && *classname) {
    const ClassInfo *classInfo = NULL;
    switch (type) {
    case 0:
      classInfo = FindClass(classname);
      if (classInfo == NULL) {
        classInfo = FindInterface(classname);
        type = 2;
      }
      break;
    case 1:
      classInfo = FindClass(classname);
      break;
    case 2:
      classInfo = FindInterface(classname);
      break;
    default:
      ASSERT(false);
    }

    if (classInfo) {
      const ClassInfo::MethodVec &methods = classInfo->getMethodsVec();
      ret.insert(ret.end(), methods.begin(), methods.end());

      if (type != 2) {
        const char *parentClass = classInfo->getParentClass();
        if (parentClass && *parentClass) {
          GetClassMethods(ret, parentClass, 1);
        }
      }

      const ClassInfo::InterfaceVec &interfaces =
        classInfo->getInterfacesVec();
      for (unsigned int i = 0; i < interfaces.size(); i++) {
        GetClassMethods(ret, interfaces[i], 2);
      }
    }
  }
}

void ClassInfo::GetClassProperties(PropertyMap &props, const char *classname) {
  if (classname && *classname) {
    const ClassInfo *classInfo = FindClass(classname);
    if (classInfo) {
      classInfo->getAllProperties(props);
    }
  }
}

void ClassInfo::GetClassProperties(PropertyVec &props, const char *classname) {
  if (classname && *classname) {
    const ClassInfo *classInfo = FindClass(classname);
    if (classInfo) {
      classInfo->getAllProperties(props);
    }
  }
}

void ClassInfo::GetClassSymbolNames(CArrRef names, bool interface,
                                    std::vector<String> &classes,
                                    std::vector<String> *clsMethods,
                                    std::vector<String> *clsProperties,
                                    std::vector<String> *clsConstants) {
  if (clsMethods || clsProperties || clsConstants) {
    for (ArrayIter iter(names); iter; ++iter) {
      String clsname = iter.second().toString();
      classes.push_back(clsname);

      const ClassInfo *cls;
      if (interface) {
        cls = FindInterface(clsname.data());
      } else {
        cls = FindClass(clsname.data());
      }
      ASSERT(cls);
      if (clsMethods) {
        const ClassInfo::MethodVec &methods = cls->getMethodsVec();
        for (unsigned int i = 0; i < methods.size(); i++) {
          clsMethods->push_back(clsname + "::" + methods[i]->name);
        }
      }
      if (clsProperties) {
        const ClassInfo::PropertyVec &properties = cls->getPropertiesVec();
        for (ClassInfo::PropertyVec::const_iterator iter = properties.begin();
             iter != properties.end(); ++iter) {
          clsProperties->push_back(clsname + "::$" + (*iter)->name);
        }
      }
      if (clsConstants) {
        const ClassInfo::ConstantVec &constants = cls->getConstantsVec();
        for (ClassInfo::ConstantVec::const_iterator iter = constants.begin();
             iter != constants.end(); ++iter) {
          clsConstants->push_back(clsname + "::" + (*iter)->name);
        }
      }
    }
  } else {
    for (ArrayIter iter(names); iter; ++iter) {
      classes.push_back(iter.second().toString());
    }
  }
}

void ClassInfo::GetSymbolNames(std::vector<String> &classes,
                               std::vector<String> &functions,
                               std::vector<String> &constants,
                               std::vector<String> *clsMethods,
                               std::vector<String> *clsProperties,
                               std::vector<String> *clsConstants) {
  static unsigned int methodSize = 128;
  static unsigned int propSize   = 128;
  static unsigned int constSize  = 128;

  if (clsMethods) {
    clsMethods->reserve(methodSize);
  }
  if (clsProperties) {
    clsProperties->reserve(propSize);
  }
  if (clsConstants) {
    clsConstants->reserve(constSize);
  }

  GetClassSymbolNames(GetClasses(true), false, classes,
                      clsMethods, clsProperties, clsConstants);
  GetClassSymbolNames(GetInterfaces(true), true, classes,
                      clsMethods, clsProperties, clsConstants);

  if (clsMethods && methodSize < clsMethods->size()) {
    methodSize = clsMethods->size();
  }
  if (clsProperties && propSize < clsProperties->size()) {
    propSize = clsProperties->size();
  }
  if (constSize && constSize < clsConstants->size()) {
    constSize = clsConstants->size();
  }

  Array funcs1 = ClassInfo::GetSystemFunctions();
  Array funcs2 = ClassInfo::GetUserFunctions();
  functions.reserve(funcs1.size() + funcs2.size());
  for (ArrayIter iter(funcs1); iter; ++iter) {
    functions.push_back(iter.second().toString());
  }
  for (ArrayIter iter(funcs2); iter; ++iter) {
    functions.push_back(iter.second().toString());
  }

  Array consts = GetConstants();
  constants.reserve(consts.size());
  for (ArrayIter iter(consts); iter; ++iter) {
    constants.push_back(iter.first().toString());
  }
}

///////////////////////////////////////////////////////////////////////////////
// ClassInfo

bool ClassInfo::isDeclared() const {
  if (m_attribute & IsVolatile) {
    return get_globals()->class_exists(m_name);
  }
  return true;
}

void ClassInfo::getAllParentsVec(ClassVec &parents) const {
  const char *parent = getParentClass();
  if (parent && *parent) {
    parents.push_back(parent);
    const ClassInfo *info = FindClass(parent);
    if (info) info->getAllParentsVec(parents);
  }
}

void ClassInfo::getAllInterfacesVec(InterfaceVec &interfaces) const {
  const InterfaceVec &ifs = getInterfacesVec();
  for (unsigned int i = 0; i < ifs.size(); i++) {
    const char *intf = ifs[i];
    interfaces.push_back(intf);
    const ClassInfo *info = FindInterface(intf);
    if (info) info->getAllInterfacesVec(interfaces);
  }
}

bool ClassInfo::derivesFrom(const char *name, bool considerInterface) const {
  ASSERT(name);
  return derivesFromImpl(name, considerInterface);
}

bool ClassInfo::derivesFromImpl(const char *name, bool considerInterface) const {
  if (strcasecmp(name, getParentClass()) == 0) {
    return true;
  }

  // We don't support redeclared parents anyway.
  const ClassInfo *parent = getParentClassInfo();
  if (parent && parent->derivesFromImpl(name, considerInterface)) {
    return true;
  }

  if (considerInterface) {
    const InterfaceMap &interfaces = getInterfaces();
    for (InterfaceMap::const_iterator iter = interfaces.begin();
         iter != interfaces.end(); ++iter) {
      if (strcasecmp(name, *iter) == 0) {
        return true;
      }
      const ClassInfo *parent = FindInterface(*iter);
      if (parent && parent->derivesFromImpl(name, considerInterface)) {
        return true;
      }
    }
  }
  return false;
}

bool ClassInfo::isSubClass(const string &className1,
                           const string &className2,
                           bool considerInterface) {
  const ClassInfo *clsInfo1 = ClassInfo::FindClass(className1.c_str());
  if (!clsInfo1) return false;

  return clsInfo1->derivesFrom(className2.c_str(), considerInterface);
}

ClassInfo::MethodInfo *ClassInfo::getMethodInfo(const char *name) const {
  ASSERT(name);

  const MethodMap &methods = getMethods();
  MethodMap::const_iterator iter = methods.find(name);
  if (iter != methods.end()) {
    return iter->second;
  }
  return NULL;
}

ClassInfo::MethodInfo *ClassInfo::hasMethod(const char *name,
                                            ClassInfo* &classInfo) const {
  ASSERT(name);
  classInfo = (ClassInfo *)this;
  const MethodMap &methods = getMethods();
  MethodMap::const_iterator it = methods.find(name);
  if (it != methods.end()) {
    ClassInfo::MethodInfo *m = it->second;
    if (m->invokeFn) {
      return (*(m->invokeFn) != m->invokeFailedFn) ? m : NULL;
    }
    return m;
  }
  const ClassInfo *parent = getParentClassInfo();
  if (parent) return parent->hasMethod(name, classInfo);
  return NULL;
}

// internal function  className::methodName or callObject->methodName
bool  ClassInfo::hasAccess(CStrRef className, CStrRef methodName,
                           bool staticCall, bool hasCallObject) {
  const ClassInfo *clsInfo = ClassInfo::FindClass(className.c_str());
  if (!clsInfo || !clsInfo->isDeclared()) return false;
  ClassInfo *defClass;
  ClassInfo::MethodInfo *methodInfo =
    clsInfo->hasMethod(methodName.c_str(), defClass);
  if (!methodInfo) return false;
  const ClassInfo *ctxClass =
    ClassInfo::FindClass(FrameInjection::GetClassName(true));
  const Object ctxObject = FrameInjection::GetThis(true);
  bool hasObject = hasCallObject || !ctxObject.isNull();
  bool staticMethod = methodInfo->attribute & (ClassInfo::IsStatic);
  if (ctxClass) {
    return ctxClass->checkAccess(defClass, methodInfo, staticCall, hasObject);
  }
  return (staticMethod || hasObject || staticCall) &&
          methodInfo->attribute & (ClassInfo::IsPublic);
}

bool ClassInfo::checkAccess(ClassInfo *defClass,
                            MethodInfo *methodInfo,
                            bool staticCall,
                            bool hasObject) const {
  ASSERT(defClass && methodInfo);
  if (!strcasecmp(this->m_name, defClass->m_name)) {
    if (methodInfo->attribute & ClassInfo::IsStatic) return true;
    return hasObject;
  }

  if (methodInfo->attribute & ClassInfo::IsStatic) {
    if (methodInfo->attribute & ClassInfo::IsPublic) return true;
    if (methodInfo->attribute & ClassInfo::IsProtected) {
      return derivesFrom(defClass->m_name, false) ||
             defClass->derivesFrom(m_name, false);
    }
    return false;
  }
  if (!hasObject && !staticCall) return false;
  if (methodInfo->attribute & ClassInfo::IsPublic) return true;
  if (methodInfo->attribute & ClassInfo::IsProtected) {
    return derivesFrom(defClass->m_name, false) ||
           defClass->derivesFrom(m_name, false);
  }
  return false;
}

const char *ClassInfo::getConstructor() const {
  ClassInfo *defClass;
  if (hasMethod("__construct", defClass)) {
    return "__construct";
  }
  if (hasMethod(m_name, defClass)) {
    return m_name;
  }
  return NULL;
}

void ClassInfo::getAllProperties(PropertyMap &props) const {
  const PropertyMap &properties = getProperties();
  props.insert(properties.begin(), properties.end());

  const char *parentClass = getParentClass();
  if (parentClass && *parentClass) {
    GetClassProperties(props, parentClass);
  }
}

void ClassInfo::getAllProperties(PropertyVec &props) const {
  const PropertyVec &properties = getPropertiesVec();
  props.insert(props.end(), properties.begin(), properties.end());

  const char *parentClass = getParentClass();
  if (parentClass && *parentClass) {
    GetClassProperties(props, parentClass);
  }
}

ClassInfo::PropertyInfo *ClassInfo::getPropertyInfo(const char *name) const {
  ASSERT(name);
  const PropertyMap &properties = getProperties();
  PropertyMap::const_iterator iter = properties.find(name);
  if (iter != properties.end()) {
    return iter->second;
  }
  return NULL;
}

bool ClassInfo::hasProperty(const char *name) const {
  ASSERT(name);
  const PropertyMap &properties = getProperties();
  return properties.find(name) != properties.end();
}

ClassInfo::ConstantInfo *ClassInfo::getConstantInfo(const char *name) const {
  ASSERT(name);
  const ConstantMap &constants = getConstants();
  ConstantMap::const_iterator iter = constants.find(name);
  if (iter != constants.end()) {
    return iter->second;
  }
  return NULL;
}

bool ClassInfo::hasConstant(const char *name) const {
  ASSERT(name);
  const ConstantMap &constants = getConstants();
  return constants.find(name) != constants.end();
}

bool ClassInfo::PropertyInfo::isVisible(const ClassInfo *context) const {
  if ((attribute & ClassInfo::IsPublic) || context == owner) return true;
  if (!context) return false;
  if (attribute & ClassInfo::IsProtected) {
    return owner->derivesFrom(context->getName(), false) ||
           context->derivesFrom(owner->getName(), false);
  }
  ASSERT(attribute & ClassInfo::IsPrivate);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// load functions

ClassInfoUnique::ClassInfoUnique(const char **&p) {
  m_attribute = (Attribute)(int64)(*p++);
  m_name = *p++;
  m_parent = *p++;

  if (m_attribute & HasDocComment) {
    m_docComment = *p++;
  }

  while (*p) {
    const char *name = *p++;
    ASSERT(m_interfaces.find(name) == m_interfaces.end());
    m_interfaces.insert(name);
    m_interfacesVec.push_back(name);
  }
  p++;

  while (*p) {
    MethodInfo *method = new MethodInfo();
    method->attribute = (Attribute)(int64)(*p++);
    method->name = *p++;
    method->invokeFn = (Variant (**)(const Array& params))*p++;
    method->invokeFailedFn = (Variant (*)(const Array& params))*p++;
    if (method->attribute & HasDocComment) {
      method->docComment = *p++;
    }

    while (*p) {
      ParameterInfo *parameter = new ParameterInfo();
      parameter->attribute = (Attribute)(int64)(*p++);
      parameter->name = *p++;
      parameter->type = *p++;
      ASSERT(Util::toLower(parameter->type) == parameter->type);
      parameter->value = *p++;
      parameter->valueText = *p++;

      method->parameters.push_back(parameter);
    }
    p++;

    while (*p) {
      ConstantInfo *staticVariable = new ConstantInfo();
      staticVariable->name = *p++;
      staticVariable->valueLen = (int64)(*p++);
      staticVariable->valueText = *p++;
      istringstream in(std::string(staticVariable->valueText,
                                   staticVariable->valueLen));
      VariableUnserializer vu(in);
      try {
        staticVariable->value = vu.unserialize();
        staticVariable->value.setStatic();
      } catch (Exception &e) {
        ASSERT(false);
      }
      method->staticVariables.push_back(staticVariable);
    }
    p++;

    ASSERT(m_methods.find(method->name) == m_methods.end());
    m_methods[method->name] = method;
    m_methodsVec.push_back(method);
  }
  p++;

  while (*p) {
    PropertyInfo *property = new PropertyInfo();
    property->attribute = (Attribute)(int64)(*p++);
    property->name = *p++;
    property->owner = this;
    ASSERT(m_properties.find(property->name) == m_properties.end());
    m_properties[property->name] = property;
    m_propertiesVec.push_back(property);
  }
  p++;

  while (*p) {
    ConstantInfo *constant = new ConstantInfo();
    constant->name = *p++;
    constant->valueLen = (int64)(*p++);
    constant->valueText = *p++;

    if (constant->valueText) {
      istringstream in(std::string(constant->valueText, constant->valueLen));
      VariableUnserializer vu(in);
      try {
        constant->value = vu.unserialize();
        constant->value.setStatic();
      } catch (Exception &e) {
        ASSERT(false);
      }
    } else if (m_name) {
      if (!(m_attribute & IsVolatile) && !(m_attribute & IsLazyInit)) {
        const ObjectStaticCallbacks *cwo = get_object_static_callbacks(m_name);
        if (cwo) {
          constant->value = cwo->os_constant(constant->name);
        } else {
          ASSERT(false);
        }
      }
    } else {
      constant->value = get_constant(constant->name);
    }

    ASSERT(m_constants.find(constant->name) == m_constants.end());
    m_constants[constant->name] = constant;
    m_constantsVec.push_back(constant);
  }
  p++;
}

ClassInfoRedeclared::ClassInfoRedeclared(const char **&p) {
  m_attribute = (Attribute)(int64)(*p++);
  m_name = *p++;
  m_redeclaredIdGetter = (int (*)())*p++;
  while (*p) {
    ClassInfo *cls = new ClassInfoUnique(p);
    m_redeclaredClasses.push_back(cls);
  }
  p++;
}

void ClassInfo::Load() {
  Lock lock(s_mutex);
  if (s_loaded) return;

  const char **p = g_class_map;
  while (*p) {
    Attribute attribute = (Attribute)(int64)*p;
    ClassInfo *info = (attribute & IsRedeclared) ?
      static_cast<ClassInfo*>(new ClassInfoRedeclared(p)) :
      static_cast<ClassInfo*>(new ClassInfoUnique(p));

    if (info->m_name == NULL) {
      if (attribute & IsSystem) {
        ASSERT(s_systemFuncs == NULL);
        s_systemFuncs = info;
      } else {
        ASSERT(s_userFuncs == NULL);
        s_userFuncs = info;
      }
    } else if (attribute & IsInterface) {
      ASSERT(s_classes.find(info->m_name) == s_classes.end());
      ASSERT(s_interfaces.find(info->m_name) == s_interfaces.end());
      s_interfaces[info->m_name] = info;
    } else {
      ASSERT(s_classes.find(info->m_name) == s_classes.end());
      ASSERT(s_interfaces.find(info->m_name) == s_interfaces.end());
      s_classes[info->m_name] = info;
    }
  }

  ASSERT(s_systemFuncs);
  ASSERT(s_userFuncs);
  s_loaded = true;
}

ClassInfo::MethodInfo::~MethodInfo() {
  for (vector<const ParameterInfo *>::iterator it = parameters.begin();
       it != parameters.end(); ++it) {
    delete *it;
  }
  for (vector<const ConstantInfo *>::iterator it = staticVariables.begin();
       it != staticVariables.end(); ++it) {
    delete *it;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
