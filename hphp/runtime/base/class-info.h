/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_CLASS_INFO_H_
#define incl_HPHP_CLASS_INFO_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/util/mutex.h"
#include "hphp/util/case-insensitive.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ClassInfoHook;

/**
 * Though called "ClassInfo", we consider global scope as a virtual "class".
 * Therefore, this is the place we store meta information of both global
 * functions and class methods and properties.
 */
class ClassInfo {
public:
  enum Attribute {                      //  class   prop   func  method param
    ZendParamModeNull      = (1 <<  0), //                  x      x
    IsRedeclared           = (1 <<  1), //    x             x
    IsVolatile             = (1 <<  2), //    x             x

    IsInterface            = (1 <<  3), //    x
    IsClosure              = (1 <<  3), //                  x      x
    IsAbstract             = (1 <<  4), //    x                    x
    IsFinal                = (1 <<  5), //    x                    x

    IsPublic               = (1 <<  6), //           x             x
    IsProtected            = (1 <<  7), //           x             x
    IsPrivate              = (1 <<  8), //           x             x
    IsStatic               = (1 <<  9), //           x             x
    IsCppAbstract          = (1 << 10), //    x
    HasCall                = IsPublic,  //    x
    AllowOverride          = IsPrivate, //                  x
    IsReference            = (1 << 11), //                  x      x     x
    IsConstructor          = (1 << 12), //                         x

    // need a non-zero number for const char * maps
    IsNothing              = (1 << 13),

    ZendCompat             = (1 << 14), //                  x      x

    HasGeneratorAsBody     = (1 << 15), //                  x      x
    IsCppSerializable      = (1 << 15), //    x
    HipHopSpecific         = (1 << 16), //    x             x

    VariableArguments      = (1 << 17), //                  x      x
    RefVariableArguments   = (1 << 18), //                  x      x
    MixedVariableArguments = (1 << 19), //                  x      x

    FunctionIsFoldable     = (1 << 20), //                  x
    NoEffect               = (1 << 21), //                  x
    NoInjection            = (1 << 22), //                  x      x
    HasOptFunction         = (1 << 23), //                  x
    AllowIntercept         = (1 << 24), //                  x      x
    NoProfile              = (1 << 25), //                  x      x
    ContextSensitive       = (1 << 26), //                  x
    NoDefaultSweep         = (1 << 27), //    x
    IsSystem               = (1 << 28), //    x             x

    IsTrait                = (1 << 29), //    x
    ZendParamModeFalse     = (1 << 30), //                  x      x
    NeedsActRec            = (1u << 31),//                  x      x
  };

  class ConstantInfo {
  public:
    ConstantInfo();

    String name;
    unsigned int valueLen;
    const char *valueText;
    const void* callback;

    CVarRef getDeferredValue() const;
    Variant getValue() const;
    bool isDeferred() const { return deferred; }
    bool isCallback() const { return callback != nullptr; }
    void setValue(CVarRef value);
    void setStaticValue(CVarRef value);

    bool isDynamic() const {
      return deferred;
    }
  private:
    bool deferred;
    Variant value;
    std::string svalue; // serialized, only used by eval
  };

  class UserAttributeInfo {
  public:
    UserAttributeInfo();

    String name;

    Variant getValue() const;
    void setStaticValue(CVarRef value);

  private:
    Variant value;
  };

  struct ParameterInfo {
    ~ParameterInfo();
    Attribute attribute;
    DataType argType;      // hinted arg type
    const char *name;
    const char *type;      // hinted type string
    const char *value;     // serialized default value
    int64_t valueLen;
    const char *valueText; // original PHP code
    int64_t valueTextLen;
    std::vector<const UserAttributeInfo *> userAttrs;
  };

  struct MethodInfo {
    MethodInfo() : docComment(nullptr) {}
    explicit MethodInfo(const char **&p);
    ~MethodInfo();
    MethodInfo *getDeclared();
    Attribute attribute;
    int volatile_redec_offset;
    String name;

    std::vector<const ParameterInfo *> parameters;
    std::vector<const ConstantInfo *> staticVariables;
    std::vector<const UserAttributeInfo *> userAttrs;

    const char *docComment;
    const char *file;
    int line1;
    int line2;
    DataType returnType;
  };

  class PropertyInfo {
  public:
    PropertyInfo() : docComment(nullptr) {}
    Attribute attribute;
    String name;
    DataType type;
    const char *docComment;
    const ClassInfo *owner;
  };

  typedef StringIMap<ClassInfo *>                 ClassMap;
  typedef std::vector<String>                     ClassVec;
  typedef StringISet                              InterfaceSet;
  typedef std::vector<String>                     InterfaceVec;
  typedef StringISet                              TraitSet;
  typedef std::vector<String>                     TraitVec;
  typedef StringIMap<MethodInfo *>                MethodMap;
  typedef std::vector<MethodInfo *>               MethodVec;
  typedef StringMap<PropertyInfo *>               PropertyMap;
  typedef std::vector<PropertyInfo *>             PropertyVec;
  typedef StringMap<ConstantInfo *>               ConstantMap;
  typedef std::vector<ConstantInfo *>             ConstantVec;
  typedef std::vector<UserAttributeInfo *>        UserAttributeVec;
  typedef std::vector<std::pair<String, String> > TraitAliasVec;

public:
  /**
   * Load everything.
   */
  static void Load();

  /**
   * Return a list of PHP library functions.
   */
  static Array GetSystemFunctions();

  /**
   * Return a list of user defined functions.
   */
  static Array GetUserFunctions();

  /**
   * Locate one function.
   */
  static const MethodInfo *FindSystemFunction(const String& name);
  static const MethodInfo *FindFunction(const String& name);

  /**
   * Return a list of declared classes.
   */
  static Array GetClasses() { return GetClassLike(IsInterface|IsTrait, 0); }

  /**
   * Locate one class.
   */
  static const ClassInfo *FindClass(const String& name);

  /**
   * Locate one system class.
   */
  static const ClassInfo *FindSystemClass(const String& name);

  /**
   * Return a list of declared interfaces.
   */
  static Array GetInterfaces() { return GetClassLike(IsInterface, IsInterface); }

  /**
   * Return a list of declared traits.
   */
  static Array GetTraits() { return GetClassLike(IsTrait, IsTrait); }

  /**
   * Locate one interface.
   */
  static const ClassInfo *FindInterface(const String& name);

  /**
   * Locate one system interface.
   */
  static const ClassInfo *FindSystemInterface(const String& name);

  /**
   * Locate one trait.
   */
  static const ClassInfo *FindTrait(const String& name);

  /**
   * Locate one system trait.
   */
  static const ClassInfo *FindSystemTrait(const String& name);

  /**
   * Locate either a class, interface, or trait.
   */
  static const ClassInfo *FindClassInterfaceOrTrait(const String& name);

  /**
   * Locate either a system class, system interface, or system trait.
   */
  static const ClassInfo *FindSystemClassInterfaceOrTrait(const String& name);

  /**
   * Get all statically known system constants, unless explicitly
   * specified to get the dynamic ones.
   */
  static Array GetSystemConstants(bool get_dynamic_constants = false);
  static void InitializeSystemConstants();

  /**
   * Return all methods a class has, including the ones on base classes and
   * interfaces.
   *   type: 0: unknown; 1: class; 2: interface
   */
  static bool GetClassMethods(MethodVec &ret, const String& classname,
                              int type = 0);
  static bool GetClassMethods(MethodVec &ret, const ClassInfo *classInfo);

  /**
   * Read user attributes in from the class map.
   */
  static void ReadUserAttributes(const char **&p,
                                 std::vector<const UserAttributeInfo*> &attrs);

  /**
   * Return lists of names for auto-complete purposes.
   */
  static void GetClassSymbolNames(CArrRef names, bool interface, bool trait,
                                  std::vector<String> &classes,
                                  std::vector<String> *clsMethods,
                                  std::vector<String> *clsProperties,
                                  std::vector<String> *clsConstants);
  static void GetSymbolNames(std::vector<String> &classes,
                             std::vector<String> &functions,
                             std::vector<String> &constants,
                             std::vector<String> *clsMethods,
                             std::vector<String> *clsProperties,
                             std::vector<String> *clsConstants);

  static void SetHook(ClassInfoHook *hook) { s_hook = hook; }

  static Variant GetVariant(DataType type, const void *addr) {
    switch (type) {
      case KindOfBoolean:
        return *(bool*)addr;
      case KindOfInt64:
        return *(int64_t*)addr;
      case KindOfDouble:
        return *(double*)addr;
      case KindOfString:
        return *(String*)addr;
      case KindOfArray:
        return *(Array*)addr;
      case KindOfObject:
        return *(Object*)addr;
      case KindOfResource:
        return *(Resource*)addr;
      default:
        assert(false);
        return uninit_null();
    }
  }

public:
  ClassInfo() : m_cdec_offset(0), m_docComment(nullptr) {}
  virtual ~ClassInfo() {}

  inline const ClassInfo *checkCurrent() const {
    assert(!(m_attribute & IsRedeclared));
    return this;
  }

  Attribute getAttribute() const { return checkCurrent()->m_attribute;}
  const char *getFile() const { return checkCurrent()->m_file;}
  int getLine1() const { return checkCurrent()->m_line1;}
  int getLine2() const { return checkCurrent()->m_line2;}
  virtual const ClassInfo* getCurrentOrNull() const { return this; }
  virtual const String& getName() const { return m_name;}
  const char *getDocComment() const { return m_docComment; }

  virtual void postInit();

  /**
   * Parents of this class.
   */
  virtual const String& getParentClass() const = 0;
  const ClassInfo *getParentClassInfo() const;
  virtual const InterfaceSet &getInterfaces() const = 0;
  virtual const InterfaceVec &getInterfacesVec() const = 0;
  virtual const TraitSet &getTraits() const = 0;
  virtual const TraitVec &getTraitsVec() const = 0;
  virtual const TraitAliasVec &getTraitAliasesVec() const = 0;

  /**
   * Method functions.
   */
  virtual const MethodMap &getMethods() const = 0;    // non-recursively
  virtual const MethodVec &getMethodsVec() const = 0; // non-recursively
  MethodInfo *getMethodInfo(const String& name) const;
  MethodInfo *hasMethod(const String& name,
                        ClassInfo *&classInfo,
                        bool interfaces = false) const;

  /**
   * Property functions.
   */
  virtual const PropertyMap &getProperties() const = 0;    // non-recursively
  virtual const PropertyVec &getPropertiesVec() const = 0; // non-recursively

  /**
   * Constant functions.
   */
  virtual const ConstantMap &getConstants() const = 0;
  virtual const ConstantVec &getConstantsVec() const = 0;

  virtual const UserAttributeVec &getUserAttributeVec() const = 0;

  static ClassInfo *GetSystem() { return s_systemFuncs; }
  static const ClassMap GetClassesMap() { return s_class_like; }

protected:
  static bool s_loaded;            // whether class map is loaded
  static ClassInfo *s_systemFuncs; // all system functions

  static ClassInfoHook *s_hook;

  Attribute m_attribute;
  int m_cdec_offset;
  String m_name;
  const char *m_file;
  int m_line1;
  int m_line2;
  const char *m_docComment;

private:
  static ClassMap s_class_like;       // all classes, interfaces and traits
  static Array GetClassLike(unsigned mask, unsigned value);
  const ClassInfo *getDeclared() const;
};

/**
 * Stores info about a class that appears once in the codebase.
 */
class ClassInfoUnique : public ClassInfo {
public:

  /**
   * Read one class's information from specified map pointer and move it.
   */
  explicit ClassInfoUnique(const char **&p);
  virtual ~ClassInfoUnique();

  // implementing ClassInfo
  const String& getParentClass() const { return m_parent;}
  const ClassInfo *getParentClassInfo() const;
  const InterfaceSet &getInterfaces() const { return m_interfaces;}
  const InterfaceVec &getInterfacesVec() const { return m_interfacesVec;}
  const TraitSet &getTraits() const { return m_traits;}
  const TraitVec &getTraitsVec() const { return m_traitsVec;}
  const TraitAliasVec &getTraitAliasesVec() const { return m_traitAliasesVec;}
  const MethodMap &getMethods() const { return m_methods;}
  const MethodVec &getMethodsVec() const { return m_methodsVec;}
  const PropertyMap &getProperties() const { return m_properties;}
  const PropertyVec &getPropertiesVec() const { return m_propertiesVec;}
  const ConstantMap &getConstants() const { return m_constants;}
  const ConstantVec &getConstantsVec() const { return m_constantsVec;}
  const UserAttributeVec &getUserAttributeVec() const { return m_userAttrVec;}

  virtual void postInit();

private:
  String        m_parent;          // parent class name
  const ClassInfo *m_parentInfo;   // parent class info (or null)
  InterfaceSet  m_interfaces;      // all interfaces
  InterfaceVec  m_interfacesVec;   // all interfaces in declaration order
  TraitSet      m_traits;          // all used traits
  TraitVec      m_traitsVec;       // all used traits
  TraitAliasVec m_traitAliasesVec; // all trait aliases
  MethodMap     m_methods;         // all methods
  MethodVec     m_methodsVec;      // all methods in declaration order
  PropertyMap   m_properties;      // all properties
  PropertyVec   m_propertiesVec;   // all properties in declaration order
  ConstantMap   m_constants;       // all constants
  ConstantVec   m_constantsVec;    // all constants in declaration order
  UserAttributeVec m_userAttrVec;
};

/**
 * Interface for a hook into class info for eval. This way I can avoid
 * a dependency on eval.
 */
class ClassInfoHook {
public:
  virtual ~ClassInfoHook() {};
  virtual Array getUserFunctions() const = 0;
  virtual Array getClasses() const = 0;
  virtual Array getInterfaces() const = 0;
  virtual Array getTraits() const = 0;
  virtual const ClassInfo::MethodInfo*
  findFunction(const String& name) const = 0;
  virtual const ClassInfo *findClassLike(const String& name) const = 0;
  virtual const ClassInfo::ConstantInfo*
  findConstant(const String& name) const = 0;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_CLASS_INFO_H_
