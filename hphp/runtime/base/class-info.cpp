/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/class-info.h"
#include <vector>
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

using std::string;

bool ClassInfo::s_loaded = false;
ClassInfo *ClassInfo::s_systemFuncs = nullptr;
ClassInfo::ClassMap ClassInfo::s_class_like;

const ClassInfo::MethodInfo *ClassInfo::FindSystemFunction(const String& name) {
  assert(!name.isNull());
  assert(s_loaded);

  return s_systemFuncs->getMethodInfo(name);
}

const ClassInfo::MethodInfo *ClassInfo::FindFunction(const String& name) {
  assert(!name.isNull());
  assert(s_loaded);

  const MethodInfo *ret = s_systemFuncs->getMethodInfo(name);
  return ret;
}

const ClassInfo *ClassInfo::FindClassInterfaceOrTrait(const String& name) {
  assert(!name.isNull());
  assert(s_loaded);

  ClassMap::const_iterator iter = s_class_like.find(name);
  if (iter != s_class_like.end()) {
    return iter->second->getDeclared();
  }

  return 0;
}

const ClassInfo *ClassInfo::FindClass(const String& name) {
  if (const ClassInfo *r = FindClassInterfaceOrTrait(name)) {
    return r->getAttribute() & (IsTrait|IsInterface) ? 0 : r;
  }
  return 0;
}

const ClassInfo *ClassInfo::FindInterface(const String& name) {
  if (const ClassInfo *r = FindClassInterfaceOrTrait(name)) {
    return r->getAttribute() & IsInterface ? r : 0;
  }
  return 0;
}

const ClassInfo *ClassInfo::FindTrait(const String& name) {
  if (const ClassInfo *r = FindClassInterfaceOrTrait(name)) {
    return r->getAttribute() & IsTrait ? r : 0;
  }
  return 0;
}

const ClassInfo *
ClassInfo::FindSystemClassInterfaceOrTrait(const String& name) {
  assert(!name.isNull());
  assert(s_loaded);

  ClassMap::const_iterator iter = s_class_like.find(name);
  if (iter != s_class_like.end()) {
    const ClassInfo *ci = iter->second;
    if (ci->m_attribute & IsSystem) return ci;
  }

  return 0;
}

const ClassInfo *ClassInfo::FindSystemClass(const String& name) {
  if (const ClassInfo *r = FindSystemClassInterfaceOrTrait(name)) {
    return r->getAttribute() & (IsTrait|IsInterface) ? 0 : r;
  }
  return 0;
}

const ClassInfo *ClassInfo::FindSystemInterface(const String& name) {
  if (const ClassInfo *r = FindSystemClassInterfaceOrTrait(name)) {
    return r->getAttribute() & IsInterface ? r : 0;
  }
  return 0;
}

const ClassInfo *ClassInfo::FindSystemTrait(const String& name) {
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
  if (value & IsInterface) {
    Array dyn = Unit::getInterfacesInfo();
    if (!dyn.isNull()) {
      ret.merge(dyn);
      // De-dup values, then renumber (for aesthetics).
      ret = ArrayUtil::StringUnique(ret).toArrRef();
      ret->renumber();
    }
  } else if (value & IsTrait) {
    Array dyn = Unit::getTraitsInfo();
    if (!dyn.isNull()) {
      ret.merge(dyn);
      // De-dup values, then renumber (for aesthetics).
      ret = ArrayUtil::StringUnique(ret).toArrRef();
      ret->renumber();
    }
  } else {
    Array dyn = Unit::getClassesInfo();
    if (!dyn.isNull()) {
      ret.merge(dyn);
      // De-dup values, then renumber (for aesthetics).
      ret = ArrayUtil::StringUnique(ret).toArrRef();
      ret->renumber();
    }
  }
  return ret;
}

ClassInfo::ConstantInfo::ConstantInfo() : valueLen(0) {
}

Variant ClassInfo::ConstantInfo::getValue() const {
  if (!svalue.empty()) {
    try {
      VariableUnserializer vu(svalue.data(), svalue.size(),
                              VariableUnserializer::Type::Serialize);
      return vu.unserialize();
    } catch (ResourceExceededException&) {
      throw;
    } catch (Exception&) {
      assert(false);
    }
  }
  return value;
}

void ClassInfo::ConstantInfo::setValue(const Variant& value) {
  VariableSerializer vs(VariableSerializer::Type::Serialize);
  String s = vs.serialize(value, true);
  svalue = std::string(s.data(), s.size());
}

void ClassInfo::ConstantInfo::setStaticValue(const Variant& v) {
  value = v;
  value.setEvalScalar();
}

void ClassInfo::InitializeSystemConstants() {
  assert(s_loaded);
  const ConstantMap &scm = s_systemFuncs->getConstants();
  for (ConstantMap::const_iterator it = scm.begin(); it != scm.end(); ++it) {
    ConstantInfo* ci = it->second;
    Variant v = ci->getValue();
    bool DEBUG_ONLY res = Unit::defCns(ci->name.get(),
                                           v.asTypedValue(), true);
    assert(res);
  }
}

Array ClassInfo::GetSystemConstants() {
  assert(s_loaded);
  Array res;
  const ConstantMap &scm = s_systemFuncs->getConstants();
  for (ConstantMap::const_iterator it = scm.begin(); it != scm.end(); ++it) {
    res.set(it->second->name, it->second->getValue());
  }
  return res;
}

ClassInfo::UserAttributeInfo::UserAttributeInfo() {
}

Variant ClassInfo::UserAttributeInfo::getValue() const {
  return value;
}

void ClassInfo::UserAttributeInfo::setStaticValue(const Variant& v) {
  value = v;
  value.setEvalScalar();
}

bool ClassInfo::GetClassMethods(MethodVec &ret, const String& classname,
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
    const String& parentClass = classInfo->getParentClass();
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
  const String& parentName = getParentClass();
  if (parentName.empty()) return nullptr;
  return FindClass(parentName);
}

ClassInfo::MethodInfo *ClassInfo::getMethodInfo(const String& name) const {
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

ClassInfo::MethodInfo *ClassInfo::hasMethod(const String& name,
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

///////////////////////////////////////////////////////////////////////////////
// load functions

static String staticString(const char *s) {
  if (!s) {
    return String();
  }
  return makeStaticString(s);
}

void ClassInfo::ReadUserAttributes(const char **&p,
                                   std::vector<const UserAttributeInfo*> &userAttrVec) {
  while (*p) {
    UserAttributeInfo *userAttr = new UserAttributeInfo();
    userAttr->name = staticString(*p++);

    const char *len = *p++;
    const char *valueText = *p++;
    int64_t valueLen = (int64_t)len;
    VariableUnserializer vu(valueText,
                            valueLen,
                            VariableUnserializer::Type::Serialize);
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
  attribute = (Attribute)(int64_t)(*p++);
  name = staticString(*p++);
  docComment = "";
  if (attribute & ClassInfo::IsRedeclared) {
    volatile_redec_offset = (int)(int64_t)(*p++);
    while (*p) {
      MethodInfo *m = new MethodInfo(p);
      parameters.push_back((ParameterInfo*)(void*)m);
    }
  } else {
    file = *p++;
    line1 = (int)(int64_t)(*p++);
    line2 = (int)(int64_t)(*p++);
    if (attribute & IsVolatile) {
      volatile_redec_offset = (int)(int64_t)(*p++);
    }

    docComment = *p++;

    if (attribute & IsSystem) {
      auto dt = static_cast<DataType>((int64_t)(*p++));
      returnType = dt == kInvalidDataType ? folly::none
                                          : MaybeDataType(dt);
    }
    while (*p) {
      ParameterInfo *parameter = new ParameterInfo();
      parameter->attribute = (Attribute)(int64_t)(*p++);
      parameter->name = *p++;
      parameter->type = *p++;
      if (attribute & IsSystem) {
        auto dt = static_cast<DataType>((int64_t)(*p++));
        parameter->argType = dt == kInvalidDataType ? folly::none
                                                    : MaybeDataType(dt);
      }
      parameter->value = *p++;
      parameter->valueLen = (int64_t)*p++;
      parameter->valueText = *p++;
      parameter->valueTextLen = (int64_t)*p++;

      ClassInfo::ReadUserAttributes(p, parameter->userAttrs);
      p++;

      parameters.push_back(parameter);
    }
    p++;

    while (*p) {
      ConstantInfo *staticVariable = new ConstantInfo();
      staticVariable->name = staticString(*p++);
      staticVariable->valueLen = (int64_t)(*p++);
      staticVariable->valueText = *p++;
      VariableUnserializer vu(staticVariable->valueText,
                              staticVariable->valueLen,
                              VariableUnserializer::Type::Serialize);
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
  m_attribute = (Attribute)(int64_t)(*p++);
  assert(!(m_attribute & IsRedeclared));

  // ClassInfoUnique is only created by ClassInfo::Load(), which is called
  // from hphp_process_init() in the thread-neutral initialization phase.
  // It is OK to create StaticStrings here, and throw the String wrapper away,
  // because the underlying static StringData will not be released.
  m_name = staticString(*p++);
  m_parent = staticString(*p++);
  m_parentInfo = 0;

  m_file = *p++;
  m_line1 = (int)(int64_t)(*p++);
  m_line2 = (int)(int64_t)(*p++);

  if (m_attribute & IsVolatile) {
    m_cdec_offset = (int)(int64_t)(*p++);
  }

  m_docComment = *p++;

  while (*p) {
    String iface_name = staticString(*p++);
    assert(m_interfaces.find(iface_name) == m_interfaces.end());
    m_interfaces.insert(iface_name);
    m_interfacesVec.push_back(iface_name);
  }
  p++;

  while (*p) {
    MethodInfo *method = new MethodInfo(p);

    assert(m_methods.find(method->name) == m_methods.end());
    m_methods[method->name] = method;
    m_methodsVec.push_back(method);
  }
  p++;

  while (*p) {
    PropertyInfo *property = new PropertyInfo();
    property->attribute = (Attribute)(int64_t)(*p++);
    property->name = staticString(*p++);
    auto dt = static_cast<DataType>((int)uintptr_t(*p++));
    property->type = dt == kInvalidDataType ? folly::none
                                            : MaybeDataType(dt);
    property->owner = this;
    assert(m_properties.find(property->name) == m_properties.end());
    m_properties[property->name] = property;
    m_propertiesVec.push_back(property);
  }
  p++;

  while (*p) {
    ConstantInfo *constant = new ConstantInfo();
    constant->name = staticString(*p++);
    const char *len_or_cw = *p++;
    constant->valueText = *p++;

    assert(constant->valueText);
    if (uintptr_t(constant->valueText) > 0x100) {
      // Serialized value from an IDL entry with a value: element
      constant->valueLen = (int64_t)len_or_cw;
      VariableUnserializer vu(constant->valueText,
                              constant->valueLen,
                              VariableUnserializer::Type::Serialize);
      try {
        constant->setStaticValue(vu.unserialize());
      } catch (ResourceExceededException&) {
        throw;
      } catch (Exception&) {
        assert(false);
      }
    } else {
      DataType dt = DataType((int)uintptr_t(constant->valueText) - 2);
      assert(dt != kInvalidDataType);
      constant->valueLen = 0;
      constant->valueText = nullptr;
      Variant v;
      v = ClassInfo::GetVariant(dt, len_or_cw);
      constant->setStaticValue(v);
    }

    assert(m_constants.find(constant->name) == m_constants.end());
    m_constants[constant->name] = constant;
    m_constantsVec.push_back(constant);
  }
  p++;

  while (*p) {
    UserAttributeInfo *userAttr = new UserAttributeInfo();
    userAttr->name = staticString(*p++);

    const char *len = *p++;
    const char *valueText = *p++;
    int64_t valueLen = (int64_t)len;
    VariableUnserializer vu(valueText,
                            valueLen,
                            VariableUnserializer::Type::Serialize);
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
    UNUSED Extension *ext = (Extension*)*p++;
    // TODO: Do something useful with this
    // For now, it's here to anchor "empty" extensions
  }
  p++;

  while (*p) {
    Attribute attribute = (Attribute)(int64_t)*p;
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
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
      delete (MethodInfo*)(void*)*it;
    }
  } else {
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
      delete *it;
    }
    for (auto it = staticVariables.begin();
         it != staticVariables.end(); ++it) {
      delete *it;
    }
  }
  for (auto it = userAttrs.begin(); it != userAttrs.end(); ++it) {
    delete *it;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
