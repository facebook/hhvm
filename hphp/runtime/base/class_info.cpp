/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/array/array_util.h>
#include <runtime/base/class_info.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/externals.h>
#include <runtime/base/hphp_system.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/variable_unserializer.h>
#include <util/util.h>
#include <util/lock.h>
#include <util/logger.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

bool ClassInfo::s_loaded = false;
ClassInfo *ClassInfo::s_systemFuncs = nullptr;
ClassInfo::ClassMap ClassInfo::s_class_like;
ClassInfoHook *ClassInfo::s_hook = nullptr;

Array ClassInfo::GetSystemFunctions() {
  assert(s_loaded);

  Array ret = Array::Create();
  if (s_systemFuncs) {
    const MethodVec &methods = s_systemFuncs->getMethodsVec();
    for (unsigned i = 0; i < methods.size(); i++) {
      ret.append(methods[i]->name);
    }
  }
  return ret;
}

Array ClassInfo::GetUserFunctions() {
  assert(s_loaded);

  Array ret = Array::Create();
  if (s_hook) {
    Array dyn = s_hook->getUserFunctions();
    if (!dyn.isNull()) {
      ret.merge(dyn);
      // De-dup values, then renumber (for aesthetics).
      ret = ArrayUtil::StringUnique(ret).toArrRef();
      ret->renumber();
    }
  }
  return ret;
}

const ClassInfo::MethodInfo *ClassInfo::FindSystemFunction(CStrRef name) {
  assert(!name.isNull());
  assert(s_loaded);

  return s_systemFuncs->getMethodInfo(name);
}

const ClassInfo::MethodInfo *ClassInfo::FindFunction(CStrRef name) {
  assert(!name.isNull());
  assert(s_loaded);

  const MethodInfo *ret = s_systemFuncs->getMethodInfo(name);
  if (ret == nullptr && s_hook) {
    ret = s_hook->findFunction(name);
  }
  return ret;
}

const ClassInfo *ClassInfo::FindClassInterfaceOrTrait(CStrRef name) {
  assert(!name.isNull());
  assert(s_loaded);

  if (s_hook) {
    const ClassInfo *r = s_hook->findClassLike(name);
    if (r) return r;
  }

  ClassMap::const_iterator iter = s_class_like.find(name);
  if (iter != s_class_like.end()) {
    return iter->second->getDeclared();
  }

  return 0;
}

const ClassInfo *ClassInfo::FindClass(CStrRef name) {
  if (const ClassInfo *r = FindClassInterfaceOrTrait(name)) {
    return r->getAttribute() & (IsTrait|IsInterface) ? 0 : r;
  }
  return 0;
}

const ClassInfo *ClassInfo::FindInterface(CStrRef name) {
  if (const ClassInfo *r = FindClassInterfaceOrTrait(name)) {
    return r->getAttribute() & IsInterface ? r : 0;
  }
  return 0;
}

const ClassInfo *ClassInfo::FindTrait(CStrRef name) {
  if (const ClassInfo *r = FindClassInterfaceOrTrait(name)) {
    return r->getAttribute() & IsTrait ? r : 0;
  }
  return 0;
}

const ClassInfo *ClassInfo::FindSystemClassInterfaceOrTrait(CStrRef name) {
  assert(!name.isNull());
  assert(s_loaded);

  ClassMap::const_iterator iter = s_class_like.find(name);
  if (iter != s_class_like.end()) {
    const ClassInfo *ci = iter->second;
    if (ci->m_attribute & IsSystem) return ci;
  }

  return 0;
}

const ClassInfo *ClassInfo::FindSystemClass(CStrRef name) {
  if (const ClassInfo *r = FindSystemClassInterfaceOrTrait(name)) {
    return r->getAttribute() & (IsTrait|IsInterface) ? 0 : r;
  }
  return 0;
}

const ClassInfo *ClassInfo::FindSystemInterface(CStrRef name) {
  if (const ClassInfo *r = FindSystemClassInterfaceOrTrait(name)) {
    return r->getAttribute() & IsInterface ? r : 0;
  }
  return 0;
}

const ClassInfo *ClassInfo::FindSystemTrait(CStrRef name) {
  if (const ClassInfo *r = FindSystemClassInterfaceOrTrait(name)) {
    return r->getAttribute() & IsTrait ? r : 0;
  }
  return 0;
}

Array ClassInfo::GetClassLike(unsigned mask, unsigned value) {
  assert(s_loaded);

  Array ret = Array::Create();
  for (ClassMap::const_iterator iter = s_class_like.begin();
       iter != s_class_like.end(); ++iter) {
    const ClassInfo *info = iter->second->getDeclared();
    if (!info || (info->m_attribute & mask) != value) continue;
    ret.append(info->m_name);
  }
  if (s_hook) {
    if (value & IsInterface) {
      Array dyn = s_hook->getInterfaces();
      if (!dyn.isNull()) {
        ret.merge(dyn);
        // De-dup values, then renumber (for aesthetics).
        ret = ArrayUtil::StringUnique(ret).toArrRef();
        ret->renumber();
      }
    } else if (value & IsTrait) {
      Array dyn = s_hook->getTraits();
      if (!dyn.isNull()) {
        ret.merge(dyn);
        // De-dup values, then renumber (for aesthetics).
        ret = ArrayUtil::StringUnique(ret).toArrRef();
        ret->renumber();
      }
    } else {
      Array dyn = s_hook->getClasses();
      if (!dyn.isNull()) {
        ret.merge(dyn);
        // De-dup values, then renumber (for aesthetics).
        ret = ArrayUtil::StringUnique(ret).toArrRef();
        ret->renumber();
      }
    }
  }
  return ret;
}

const ClassInfo::ConstantInfo *ClassInfo::FindConstant(CStrRef name) {
  assert(!name.isNull());
  assert(s_loaded);
  const ConstantInfo *info;
  info = s_systemFuncs->getConstantInfo(name);
  if (info) return info;
  if (s_hook) {
    info = s_hook->findConstant(name);
    if (info) return info;
  }
  return nullptr;
}

ClassInfo::ConstantInfo::ConstantInfo() :
    valueLen(0), callback(nullptr), deferred(true) {
}

Variant ClassInfo::ConstantInfo::getValue() const {
  if (deferred) {
    if (callback) {
      CVarRef (*f)()=(CVarRef(*)())callback;
      return (*f)();
    }
    SystemGlobals* g = get_global_variables();
    return g->stgv_Variant[valueLen];
  }
  if (!svalue.empty()) {
    try {
      VariableUnserializer vu(svalue.data(), svalue.size(),
                              VariableUnserializer::Serialize);
      return vu.unserialize();
    } catch (Exception &e) {
      assert(false);
    }
  }
  return value;
}

void ClassInfo::ConstantInfo::setValue(CVarRef value) {
  VariableSerializer vs(VariableSerializer::Serialize);
  String s = vs.serialize(value, true);
  svalue = string(s.data(), s.size());
  deferred = false;
}

void ClassInfo::ConstantInfo::setStaticValue(CVarRef v) {
  value = v;
  value.setEvalScalar();
  deferred = false;
}

Array ClassInfo::GetSystemConstants() {
  assert(s_loaded);
  Array res;
  const ConstantMap &scm = s_systemFuncs->getConstants();
  for (ConstantMap::const_iterator it = scm.begin(); it != scm.end(); ++it) {
    if (it->second->valueLen) {
      res.set(it->second->name, it->second->getValue());
    }
  }
  return res;
}

Array ClassInfo::GetConstants() {
  assert(s_loaded);
  Array res;
  Array dyn;
  {
    const ConstantMap &scm = s_systemFuncs->getConstants();
    for (ConstantMap::const_iterator it = scm.begin(); it != scm.end(); ++it) {
      res.set(it->second->name, it->second->getValue());
    }
  }
  if (s_hook) {
    dyn = s_hook->getConstants();
    if (!dyn.isNull()) {
      res.merge(dyn);
    }
  }
  return res;
}

ClassInfo::UserAttributeInfo::UserAttributeInfo() {
}

Variant ClassInfo::UserAttributeInfo::getValue() const {
  return value;
}

void ClassInfo::UserAttributeInfo::setStaticValue(CVarRef v) {
  value = v;
  value.setEvalScalar();
}

bool ClassInfo::GetClassMethods(MethodVec &ret, CStrRef classname,
                                int type /* = 0 */) {
  if (classname.empty()) return false;

  const ClassInfo *classInfo = nullptr;
  switch (type) {
    case 0:
      classInfo = FindClassInterfaceOrTrait(classname);
      break;
    case 1:
      classInfo = FindClass(classname);
      break;
    case 2:
      classInfo = FindInterface(classname);
      break;
    case 3:
      classInfo = FindTrait(classname);
      break;
    default:
      assert(false);
  }

  if (!classInfo) return false;
  return GetClassMethods(ret, classInfo);
}

bool ClassInfo::GetClassMethods(MethodVec &ret, const ClassInfo *classInfo) {
  const ClassInfo::MethodVec &methods = classInfo->getMethodsVec();
  ret.insert(ret.end(), methods.begin(), methods.end());

  if (!(classInfo->getAttribute() & (IsInterface|IsTrait))) {
    CStrRef parentClass = classInfo->getParentClass();
    if (!parentClass.empty()) {
      if (!GetClassMethods(ret, parentClass, 1)) return false;
    }
  }

  const ClassInfo::InterfaceVec &interfaces =
    classInfo->getInterfacesVec();
  for (unsigned int i = 0; i < interfaces.size(); i++) {
    if (!GetClassMethods(ret, interfaces[i], 2)) return false;
  }

  return true;
}

void ClassInfo::GetClassProperties(PropertyMap &props, CStrRef classname) {
  if (!classname.empty()) {
    const ClassInfo *classInfo = FindClass(classname);
    if (classInfo) {
      classInfo->getAllProperties(props);
    }
  }
}

void ClassInfo::GetClassProperties(PropertyVec &props, CStrRef classname) {
  if (!classname.empty()) {
    const ClassInfo *classInfo = FindClass(classname);
    if (classInfo) {
      classInfo->getAllProperties(props);
    }
  }
}

void ClassInfo::GetClassSymbolNames(CArrRef names, bool interface, bool trait,
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
      } else if (trait) {
        cls = FindTrait(clsname.data());
      } else {
        try {
          cls = FindClass(clsname.data());
        } catch (Exception &e) {
          Logger::Error("Caught exception %s", e.getMessage().c_str());
          continue;
        } catch(...) {
          Logger::Error("Caught unknown exception");
          continue;
        }
      }
      assert(cls);
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

  GetClassSymbolNames(GetClasses(), false, false, classes,
                      clsMethods, clsProperties, clsConstants);
  GetClassSymbolNames(GetInterfaces(), true, false, classes,
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

const ClassInfo *ClassInfo::getDeclared() const {
  if (m_attribute & IsRedeclared) {
    return getCurrentOrNull();
  } else if (m_attribute & IsVolatile) {
    return *(bool*)((char*)get_global_variables() + m_cdec_offset) ? this : 0;
  } else {
    return this;
  }
}

const ClassInfo *ClassInfo::getParentClassInfo() const {
  CStrRef parentName = getParentClass();
  if (parentName.empty()) return nullptr;
  return FindClass(parentName);
}

void ClassInfo::getAllParentsVec(ClassVec &parents) const {
  CStrRef parent = getParentClass();
  if (!parent.empty()) {
    parents.push_back(parent);
    const ClassInfo *info = FindClass(parent);
    if (info) info->getAllParentsVec(parents);
  }
}

void ClassInfo::getAllInterfacesVec(InterfaceVec &interfaces) const {
  CStrRef parent = getParentClass();
  if (!parent.empty()) {
    const ClassInfo *info = FindClass(parent);
    if (info) info->getAllInterfacesVec(interfaces);
  }

  const InterfaceVec &ifs = getInterfacesVec();
  for (unsigned int i = 0; i < ifs.size(); i++) {
    CStrRef intf = ifs[i];
    interfaces.push_back(intf);
    const ClassInfo *info = FindInterface(intf);
    if (info) info->getAllInterfacesVec(interfaces);
  }
}

bool ClassInfo::derivesFrom(CStrRef name, bool considerInterface) const {
  assert(!name.isNull());
  return derivesFromImpl(name, considerInterface);
}

bool ClassInfo::derivesFromImpl(CStrRef name, bool considerInterface) const {
  if (name->isame(getParentClass().get())) {
    return true;
  }

  const ClassInfo *parent = getParentClassInfo();
  if (parent && parent->derivesFromImpl(name, considerInterface)) {
    return true;
  }

  if (considerInterface) {
    const InterfaceSet &interfaces = getInterfaces();
    for (InterfaceSet::const_iterator iter = interfaces.begin();
         iter != interfaces.end(); ++iter) {
      if (name->isame(iter->get())) {
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

bool ClassInfo::IsSubClass(CStrRef className1, CStrRef className2,
                           bool considerInterface) {
  const ClassInfo *clsInfo1 = ClassInfo::FindClass(className1);
  if (!clsInfo1) return false;

  return clsInfo1->derivesFrom(className2, considerInterface);
}

ClassInfo::MethodInfo *ClassInfo::getMethodInfo(CStrRef name) const {
  assert(!name.isNull());

  const MethodMap &methods = getMethods();
  MethodMap::const_iterator iter = methods.find(name);
  if (iter != methods.end()) {
    ClassInfo::MethodInfo *m = iter->second;
    if (m->attribute & (IsVolatile|IsRedeclared)) {
      return m->getDeclared();
    }
    return m;
  }
  return nullptr;
}

ClassInfo::MethodInfo *ClassInfo::hasMethod(CStrRef name,
                                            ClassInfo* &classInfo,
                                            bool interfaces /* = false */)
const {
  assert(!name.isNull());
  classInfo = (ClassInfo *)this;
  const MethodMap &methods = getMethods();
  MethodMap::const_iterator it = methods.find(name);
  if (it != methods.end()) {
    assert(!(it->second->attribute & (IsVolatile|IsRedeclared)));
    return it->second;
  }
  ClassInfo::MethodInfo *result = nullptr;
  const ClassInfo *parent = getParentClassInfo();
  if (parent) result = parent->hasMethod(name, classInfo);
  if (result || !interfaces || !(m_attribute & IsAbstract)) return result;
  // TODO: consider caching the iface lookups
  const InterfaceVec &ifaces = getInterfacesVec();
  for (InterfaceVec::const_iterator it = ifaces.begin();
       it != ifaces.end(); ++it) {
    const ClassInfo *iface = FindInterface(*it);
    if (iface) result = iface->hasMethod(name, classInfo, true);
    if (result) return result;
  }
  return nullptr;
}

// internal function  className::methodName or callObject->methodName
bool ClassInfo::HasAccess(CStrRef className, CStrRef methodName,
                          bool staticCall, bool hasCallObject) {
  // It has to be either a static call or a call with an object.
  assert(staticCall || hasCallObject);
  const ClassInfo *clsInfo = ClassInfo::FindClass(className);
  if (!clsInfo) return false;
  ClassInfo *defClass;
  ClassInfo::MethodInfo *methodInfo =
    clsInfo->hasMethod(methodName, defClass);
  if (!methodInfo) return false;
  if (methodInfo->attribute & ClassInfo::IsPublic) return true;
  CStrRef ctxName = g_vmContext->getContextClassName();
  if (ctxName->size() == 0) {
    return false;
  }
  const ClassInfo *ctxClass = ClassInfo::FindClass(ctxName);
  bool hasObject = hasCallObject || g_vmContext->getThis();
  if (ctxClass) {
    return ctxClass->checkAccess(defClass, methodInfo, staticCall, hasObject);
  }
  return false;
}

bool ClassInfo::checkAccess(ClassInfo *defClass,
                            MethodInfo *methodInfo,
                            bool staticCall,
                            bool hasObject) const {
  assert(defClass && methodInfo);
  if ((m_name->isame(defClass->m_name.get()))) {
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
  if (!(m_attribute & IsTrait) && hasMethod(m_name, defClass)) {
    return m_name;
  }
  return nullptr;
}

void ClassInfo::getAllProperties(PropertyMap &props) const {
  const PropertyMap &properties = getProperties();
  props.insert(properties.begin(), properties.end());

  CStrRef parentClass = getParentClass();
  if (!parentClass.empty()) {
    GetClassProperties(props, parentClass);
  }
}

void ClassInfo::getAllProperties(PropertyVec &props) const {
  const PropertyVec &properties = getPropertiesVec();
  props.insert(props.end(), properties.begin(), properties.end());

  CStrRef parentClass = getParentClass();
  if (!parentClass.empty()) {
    GetClassProperties(props, parentClass);
  }
}

void ClassInfo::filterProperties(Array &props, Attribute toRemove) const {
  const PropertyVec &properties = getPropertiesVec();
  for (unsigned i = 0; i < properties.size(); i++) {
    if (properties[i]->attribute & toRemove) {
      props.remove(properties[i]->name, true);
    }
  }
  const ClassInfo *parent = getParentClassInfo();
  if (parent) {
    parent->filterProperties(props, toRemove);
  }
}

ClassInfo::PropertyInfo *ClassInfo::getPropertyInfo(CStrRef name) const {
  assert(!name.isNull());
  const PropertyMap &properties = getProperties();
  PropertyMap::const_iterator iter = properties.find(name);
  if (iter != properties.end()) {
    return iter->second;
  }
  return nullptr;
}

bool ClassInfo::hasProperty(CStrRef name) const {
  assert(!name.isNull());
  const PropertyMap &properties = getProperties();
  return properties.find(name) != properties.end();
}

ClassInfo::ConstantInfo *ClassInfo::getConstantInfo(CStrRef name) const {
  assert(!name.isNull());
  const ConstantMap &constants = getConstants();
  ConstantMap::const_iterator iter = constants.find(name);
  if (iter != constants.end()) {
    return iter->second;
  }
  return nullptr;
}

bool ClassInfo::hasConstant(CStrRef name) const {
  assert(!name.isNull());
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
  assert(attribute & ClassInfo::IsPrivate);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// load functions

static String makeStaticString(const char *s) {
  if (!s) {
    return null_string;
  }
  return StringData::GetStaticString(s);
}

void ClassInfo::ReadUserAttributes(const char **&p,
                                   std::vector<const UserAttributeInfo*> &userAttrVec) {
  while (*p) {
    UserAttributeInfo *userAttr = new UserAttributeInfo();
    userAttr->name = makeStaticString(*p++);

    const char *len = *p++;
    const char *valueText = *p++;
    int64 valueLen = (int64)len;
    VariableUnserializer vu(valueText,
                            valueLen,
                            VariableUnserializer::Serialize);
    userAttr->setStaticValue(vu.unserialize());

    userAttrVec.push_back(userAttr);
  }
}

ClassInfo::MethodInfo *ClassInfo::MethodInfo::getDeclared() {
  assert(!(attribute & ClassInfo::IsRedeclared));
  assert(!(attribute & ClassInfo::IsVolatile));
  return this;
}

ClassInfo::MethodInfo::MethodInfo(const char **&p) {
  attribute = (Attribute)(int64)(*p++);
  name = makeStaticString(*p++);
  docComment = 0;
  if (attribute & ClassInfo::IsRedeclared) {
    volatile_redec_offset = (int)(int64)(*p++);
    while (*p) {
      MethodInfo *m = new MethodInfo(p);
      parameters.push_back((ParameterInfo*)(void*)m);
    }
  } else {
    file = *p++;
    line1 = (int)(int64)(*p++);
    line2 = (int)(int64)(*p++);
    if (attribute & IsVolatile) {
      volatile_redec_offset = (int)(int64)(*p++);
    }

    if (attribute & HasDocComment) {
      docComment = *p++;
    }

    if (attribute & IsSystem) {
      returnType = (DataType)(int64)(*p++);
    }
    while (*p) {
      ParameterInfo *parameter = new ParameterInfo();
      parameter->attribute = (Attribute)(int64)(*p++);
      parameter->name = *p++;
      parameter->type = *p++;
      if (attribute & IsSystem) {
        parameter->argType = (DataType)(int64)(*p++);
      }
      parameter->value = *p++;
      parameter->valueText = *p++;

      ClassInfo::ReadUserAttributes(p, parameter->userAttrs);
      p++;

      parameters.push_back(parameter);
    }
    p++;

    while (*p) {
      ConstantInfo *staticVariable = new ConstantInfo();
      staticVariable->name = makeStaticString(*p++);
      staticVariable->valueLen = (int64)(*p++);
      staticVariable->valueText = *p++;
      VariableUnserializer vu(staticVariable->valueText,
                              staticVariable->valueLen,
                              VariableUnserializer::Serialize);
      try {
        staticVariable->setStaticValue(vu.unserialize());
      } catch (Exception &e) {
        assert(false);
      }
      staticVariables.push_back(staticVariable);
    }
    p++;

    ClassInfo::ReadUserAttributes(p, userAttrs);
  }

  p++;
}

ClassInfoUnique::ClassInfoUnique(const char **&p) {
  m_attribute = (Attribute)(int64)(*p++);
  assert(!(m_attribute & IsRedeclared));

  // ClassInfoUnique is only created by ClassInfo::Load(), which is called
  // from hphp_process_init() in the thread-neutral initialization phase.
  // It is OK to create StaticStrings here, and throw the smart ptrs away,
  // because the underlying static StringData will not be released.
  m_name = makeStaticString(*p++);
  m_parent = makeStaticString(*p++);
  m_parentInfo = 0;

  m_file = *p++;
  m_line1 = (int)(int64)(*p++);
  m_line2 = (int)(int64)(*p++);

  if (m_attribute & IsVolatile) {
    m_cdec_offset = (int)(int64)(*p++);
  }

  if (m_attribute & HasDocComment) {
    m_docComment = *p++;
  }

  while (*p) {
    String iface_name = makeStaticString(*p++);
    assert(m_interfaces.find(iface_name) == m_interfaces.end());
    m_interfaces.insert(iface_name);
    m_interfacesVec.push_back(iface_name);
  }
  p++;

  if (m_attribute & ClassInfo::UsesTraits) {
    while (*p) {
      String trait_name = makeStaticString(*p++);
      assert(m_traits.find(trait_name) == m_traits.end());
      m_traits.insert(trait_name);
      m_traitsVec.push_back(trait_name);
    }
    p++;
  }

  if (m_attribute & ClassInfo::HasAliasedMethods) {
    while (*p) {
      String new_name = makeStaticString(*p++);
      String old_name = makeStaticString(*p++);
      m_traitAliasesVec.push_back(std::pair<String, String>(
        new_name, old_name));
    }
    p++;
  }

  while (*p) {
    MethodInfo *method = new MethodInfo(p);

    assert(m_methods.find(method->name) == m_methods.end());
    m_methods[method->name] = method;
    m_methodsVec.push_back(method);
  }
  p++;

  while (*p) {
    PropertyInfo *property = new PropertyInfo();
    property->attribute = (Attribute)(int64)(*p++);
    property->name = makeStaticString(*p++);
    property->owner = this;
    assert(m_properties.find(property->name) == m_properties.end());
    m_properties[property->name] = property;
    m_propertiesVec.push_back(property);
  }
  p++;

  while (*p) {
    ConstantInfo *constant = new ConstantInfo();
    constant->name = makeStaticString(*p++);
    const char *len_or_cw = *p++;
    constant->valueText = *p++;

    if (uintptr_t(constant->valueText) > 0x100) {
      constant->valueLen = (int64)len_or_cw;
      VariableUnserializer vu(constant->valueText,
                              constant->valueLen,
                              VariableUnserializer::Serialize);
      try {
        constant->setStaticValue(vu.unserialize());
      } catch (Exception &e) {
        assert(false);
      }
    } else if (constant->valueText) {
      DataType dt = DataType((int)uintptr_t(constant->valueText) - 2);
      constant->valueLen = 0;
      constant->valueText = nullptr;
      Variant v;
      if (dt == KindOfUnknown) {
        constant->valueLen = intptr_t(len_or_cw);
      } else {
        v = ClassInfo::GetVariant(dt, len_or_cw);
        constant->setStaticValue(v);
      }
    } else {
      constant->callback = (void*)len_or_cw;
    }

    assert(m_constants.find(constant->name) == m_constants.end());
    m_constants[constant->name] = constant;
    m_constantsVec.push_back(constant);
  }
  p++;

  while (*p) {
    UserAttributeInfo *userAttr = new UserAttributeInfo();
    userAttr->name = makeStaticString(*p++);

    const char *len = *p++;
    const char *valueText = *p++;
    int64 valueLen = (int64)len;
    VariableUnserializer vu(valueText,
                            valueLen,
                            VariableUnserializer::Serialize);
    userAttr->setStaticValue(vu.unserialize());

    m_userAttrVec.push_back(userAttr);
  }
  p++;
}

ClassInfoUnique::~ClassInfoUnique() {
  for (auto it = m_userAttrVec.begin(); it != m_userAttrVec.end(); ++it) {
    delete *it;
  }
}

const ClassInfo *ClassInfoUnique::getParentClassInfo() const {
  if (m_parentInfo) return m_parentInfo;
  if (m_parent.empty()) return nullptr;
  return FindClass(m_parent);
}

void ClassInfoUnique::postInit() {
  if (m_parent.empty()) return;
  const ClassInfo *ci = FindClassInterfaceOrTrait(m_parent);
  if (!ci) return;
  if ((m_attribute & IsInterface) !=
      (ci->getAttribute() & (IsInterface|IsTrait|IsRedeclared))) {
    return;
  }
  m_parentInfo = ci;
}

void ClassInfo::Load() {
  if (s_loaded) return;
  const char **p = g_class_map;
  while (*p) {
    Attribute attribute = (Attribute)(int64)*p;
    always_assert(!(attribute & IsRedeclared));
    ClassInfo *info = new ClassInfoUnique(p);

    if (info->m_name.empty()) {
      if (attribute & IsSystem) {
        assert(s_systemFuncs == nullptr);
        s_systemFuncs = info;
      } else {
        always_assert(false);
      }
    } else {
      ClassInfo *&i = s_class_like[info->m_name];
      assert(!i);
      i = info;
    }
  }

  assert(s_systemFuncs);
  s_loaded = true;

  for (ClassMap::iterator it = s_class_like.begin(), end = s_class_like.end();
       it != end; ++it) {
    it->second->postInit();
  }
}

void ClassInfo::postInit() {}

ClassInfo::ParameterInfo::~ParameterInfo() {
  for (auto it = userAttrs.begin(); it != userAttrs.end(); ++it) {
    delete *it;
  }
}

ClassInfo::MethodInfo::~MethodInfo() {
  if (attribute & ClassInfo::IsRedeclared) {
    for (vector<const ParameterInfo *>::iterator it = parameters.begin();
         it != parameters.end(); ++it) {
      delete (MethodInfo*)(void*)*it;
    }
  } else {
    for (vector<const ParameterInfo *>::iterator it = parameters.begin();
         it != parameters.end(); ++it) {
      delete *it;
    }
    for (vector<const ConstantInfo *>::iterator it = staticVariables.begin();
         it != staticVariables.end(); ++it) {
      delete *it;
    }
  }
  for (auto it = userAttrs.begin(); it != userAttrs.end(); ++it) {
    delete *it;
  }
}

void ClassInfo::GetArray(const ObjectData *obj,
                         Array &props, GetArrayKind kind) {
  HPHP::VM::Instance *inst = static_cast<HPHP::VM::Instance*>(
    const_cast<ObjectData*>(obj));
  inst->HPHP::VM::Instance::o_getArray(props, !(kind & GetArrayPrivate));
}

void ClassInfo::SetArray(ObjectData *obj, CArrRef props) {
  HPHP::VM::Instance *inst = static_cast<HPHP::VM::Instance*>(obj);
  inst->HPHP::VM::Instance::o_setArray(props);
}

///////////////////////////////////////////////////////////////////////////////
}
