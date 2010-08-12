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

#ifndef __HPHP_CLASS_INFO_H__
#define __HPHP_CLASS_INFO_H__

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <util/mutex.h>
#include <util/case_insensitive.h>

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
    IsSystem               = (1 <<  0), //    x             x
    IsRedeclared           = (1 <<  1), //    x             x
    IsVolatile             = (1 <<  2), //    x             x

    IsInterface            = (1 <<  3), //    x
    IsAbstract             = (1 <<  4), //    x                    x
    IsFinal                = (1 <<  5), //    x                    x

    IsPublic               = (1 <<  6), //           x             x
    IsProtected            = (1 <<  7), //           x             x
    IsPrivate              = (1 <<  8), //           x             x
    IsStatic               = (1 <<  9), //           x             x
    IsInherited            = (1 << 10), //                         x

    IsReference            = (1 << 11), //                  x      x     x
    IsOptional             = (1 << 12), //                               x

    // need a non-zero number for const char * maps
    IsNothing              = (1 << 13),

    HasDocComment          = (1 << 14), //                  x      x
    IsLazyInit             = (1 << 15), //    x
    HipHopSpecific         = (1 << 16), //    x             x

    VariableArguments      = (1 << 17), //                  x      x
    RefVariableArguments   = (1 << 18), //                  x      x
    MixedVariableArguments = (1 << 19), //                  x      x

    FunctionIsFoldable     = (1 << 20), //                  x
    NoEffect               = (1 << 21), //                  x
    NoInjection            = (1 << 22), //                  x      x
    HasOptFunction         = (1 << 23), //                  x
  };

  struct ConstantInfo {
    const char *name;
    unsigned int valueLen;
    const char *valueText;
    Variant value;
  };

  struct ParameterInfo {
    Attribute attribute;
    const char *name;
    const char *type;      // hinted type
    const char *value;     // serialized default value
    const char *valueText; // original PHP code
  };

  class MethodInfo {
  public:
    MethodInfo() : name(NULL), docComment(NULL) {}
    ~MethodInfo();
    Attribute attribute;
    const char *name;
    Variant (**invokeFn)(const Array& params);
    Variant (*invokeFailedFn)(const Array& params);

    std::vector<const ParameterInfo *> parameters;
    std::vector<const ConstantInfo *> staticVariables;

    const char *docComment;
  };

  class PropertyInfo {
  public:
    PropertyInfo() : name(NULL), docComment(NULL) {}
    Attribute attribute;
    const char *name;
    const char *docComment;
    const ClassInfo *owner;
    bool isVisible(const ClassInfo *context) const;
  };

  typedef hphp_const_char_imap<ClassInfo*>    ClassMap;
  typedef std::vector<const char *>           ClassVec;
  typedef hphp_const_char_iset                InterfaceMap;
  typedef std::vector<const char *>           InterfaceVec;
  typedef hphp_const_char_imap<MethodInfo*>   MethodMap;
  typedef std::vector<MethodInfo*>            MethodVec;
  typedef hphp_const_char_map<PropertyInfo *> PropertyMap;
  typedef std::vector<PropertyInfo *>         PropertyVec;
  typedef hphp_const_char_map<ConstantInfo*>  ConstantMap;
  typedef std::vector<ConstantInfo*>          ConstantVec;

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
  static const MethodInfo *FindFunction(const char *name);

  /**
   * Return a list of declared classes.
   */
  static Array GetClasses(bool declaredOnly);

  /**
   * Whether a class exists, without considering interfaces.
   */
  static bool HasClass(const char *name);

  /**
   * Locate one class.
   */
  static const ClassInfo *FindClass(const char *name);

  /**
   * Return a list of declared interfaces.
   */
  static Array GetInterfaces(bool declaredOnly);

  /**
   * Whether an interface exists.
   */
  static bool HasInterface(const char *name);

  /**
   * Locate one interface.
   */
  static const ClassInfo *FindInterface(const char *name);

  /**
   * Locate one constant (excluding dynamic and redeclared constants)
   */
  static const ConstantInfo *FindConstant(const char *name);

  /**
   * Get all statically known constants
   */
  static Array GetConstants();

  /**
   * Return all methods a class has, including the ones on base classes and
   * interfaces.
   *   type: 0: unknown; 1: class; 2: interface
   */
  static void GetClassMethods(MethodVec &ret, const char *classname,
                              int type = 0);

  /**
   * Return all properties a class has, including the ones on base classes and
   * the ones that were implicitly defined.
   */
  static void GetClassProperties(PropertyMap &props, const char *classname);
  static void GetClassProperties(PropertyVec &props, const char *classname);

  /**
   * Return lists of names for auto-complete purposes.
   */
  static void GetClassSymbolNames(CArrRef names, bool interface,
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

public:
  ClassInfo() : m_docComment(NULL), m_parentCache(NULL) {}
  virtual ~ClassInfo() {}

  Attribute getAttribute() const { return getCurrent()->m_attribute;}
  virtual const char *getName() const { return m_name;}
  const char *getDocComment() const { return m_docComment; }
  virtual const ClassInfo *getCurrent() const { return this; }
  virtual bool isClassInfoRedeclared() const { return false; }
  /**
   * Whether or not declaration is executed.
   */
  bool isDeclared() const;

  /**
   * Parents of this class.
   */
  virtual const char *getParentClass() const = 0;
  const ClassInfo *getParentClassInfo() const {
    if (m_parentCache) return m_parentCache;
    const char *parentName = getParentClass();
    ASSERT(parentName);
    return m_parentCache = FindClass(parentName);
  }
  virtual const InterfaceMap &getInterfaces() const = 0;
  virtual const InterfaceVec &getInterfacesVec() const = 0;
  bool derivesFrom(const char *name, bool considerInterface) const;

  void getAllParentsVec(ClassVec &parents) const; // recursive
  void getAllInterfacesVec(InterfaceVec &interfaces) const; // recursive

  /**
   * Method functions.
   */
  virtual const MethodMap &getMethods() const = 0;    // non-recursively
  virtual const MethodVec &getMethodsVec() const = 0; // non-recursively
  MethodInfo *getMethodInfo(const char *name) const;
  MethodInfo *hasMethod(const char *name, ClassInfo *&classInfo) const;
  static bool hasAccess(CStrRef className, CStrRef methodName,
                        bool staticCall, bool hasCallObject);
  static bool isSubClass(const std::string &className1,
                         const std::string &className2,
                         bool considerInterface);
  const char *getConstructor() const;

  /**
   * Property functions.
   */
  virtual const PropertyMap &getProperties() const = 0;  // non-recursively
  virtual const PropertyVec &getPropertiesVec() const = 0;  // non-recursively
  void getAllProperties(PropertyMap &props) const;        // recursively
  void getAllProperties(PropertyVec &props) const;        // recursively
  PropertyInfo *getPropertyInfo(const char *name) const;
  bool hasProperty(const char *name) const;

  /**
   * Constant functions.
   */
  virtual const ConstantMap &getConstants() const = 0;
  virtual const ConstantVec &getConstantsVec() const = 0;
  ConstantInfo *getConstantInfo(const char *name) const;
  bool hasConstant(const char *name) const;

protected:
  static Mutex s_mutex;
  static bool s_loaded;            // whether class map is loaded
  static ClassInfo *s_systemFuncs; // all system functions
  static ClassInfo *s_userFuncs;   // all user functions
  static ClassMap s_classes;       // all classes
  static ClassMap s_interfaces;    // all interfaces

  static ClassInfoHook *s_hook;

  Attribute m_attribute;
  const char *m_name;
  const char *m_docComment;
  mutable const ClassInfo *m_parentCache; // cache the found parent class

  // name is already lowered
  bool derivesFromImpl(const char *name, bool considerInterface) const;
  bool checkAccess(ClassInfo *defClass, MethodInfo *methodInfo,
                   bool staticCall, bool hasObject) const;
};

/**
 * Stores info about a class that appears once in the codebase.
 */
class ClassInfoUnique : public ClassInfo {
public:

  /**
   * Read one class's information from specified map pointer and move it.
   */
  ClassInfoUnique(const char **&p);

  // implementing ClassInfo
  const char *getParentClass() const { return m_parent;}
  const InterfaceMap &getInterfaces() const { return m_interfaces;}
  const InterfaceVec &getInterfacesVec() const { return m_interfacesVec;}
  const MethodMap &getMethods() const { return m_methods;}
  const MethodVec &getMethodsVec() const { return m_methodsVec;}
  const PropertyMap &getProperties() const { return m_properties;}
  const PropertyVec &getPropertiesVec() const { return m_propertiesVec;}
  const ConstantMap &getConstants() const { return m_constants;}
  const ConstantVec &getConstantsVec() const { return m_constantsVec;}

private:
  const char * m_parent;        // parent class name
  InterfaceMap m_interfaces;    // all interfaces
  InterfaceVec m_interfacesVec; // all interfaces in declaration order
  MethodMap    m_methods;       // all methods
  MethodVec    m_methodsVec;    // all methods in declaration order
  PropertyMap  m_properties;    // all properties
  PropertyVec  m_propertiesVec; // all properties in declaration order
  ConstantMap  m_constants;     // all constants
  ConstantVec  m_constantsVec;  // all constants in declaration order
};

/**
 * Stores info about a class that is redeclared.
 */
class ClassInfoRedeclared : public ClassInfo {
public:
  /**
   * Read one class's information from specified map pointer and move it.
   */
  ClassInfoRedeclared(const char **&p);

  // implementing ClassInfo
  virtual const ClassInfo *getCurrent() const { return current(); }
  virtual bool isClassInfoRedeclared() const { return true; }
  const char *getName() const { return current()->getName();}
  const char *getParentClass() const { return current()->getParentClass();}

  const InterfaceMap &getInterfaces() const {
    return current()->getInterfaces();
  }
  const InterfaceVec &getInterfacesVec() const {
    return current()->getInterfacesVec();
  }
  const MethodMap &getMethods() const {
    return current()->getMethods();
  }
  const MethodVec &getMethodsVec() const {
    return current()->getMethodsVec();
  }
  const PropertyMap &getProperties() const  {
    return current()->getProperties();
  }
  const PropertyVec &getPropertiesVec() const  {
    return current()->getPropertiesVec();
  }
  const ConstantMap &getConstants() const {
    return current()->getConstants();
  }
  const ConstantVec &getConstantsVec() const {
    return current()->getConstantsVec();
  }

private:
  std::vector<ClassInfo*> m_redeclaredClasses;
  int (*m_redeclaredIdGetter)();

  const ClassInfo* current() const {
    int id = m_redeclaredIdGetter();
    if (id >= 0) {
      return m_redeclaredClasses[id];
    }
    // Not sure what to do
    return this;
  }
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
  virtual Array getConstants() const = 0;
  virtual const ClassInfo::MethodInfo *findFunction(const char *name) const = 0;
  virtual const ClassInfo *findClass(const char *name) const = 0;
  virtual const ClassInfo *findInterface(const char *name) const = 0;
  virtual const ClassInfo::ConstantInfo *findConstant(const char *name)
    const = 0;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_CLASS_INFO_H__
