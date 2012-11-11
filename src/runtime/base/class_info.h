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

#ifndef __HPHP_CLASS_INFO_H__
#define __HPHP_CLASS_INFO_H__

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <util/mutex.h>
#include <util/case_insensitive.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ClassInfoHook;
struct ClassPropTableEntry;

/**
 * Though called "ClassInfo", we consider global scope as a virtual "class".
 * Therefore, this is the place we store meta information of both global
 * functions and class methods and properties.
 */
class ClassInfo {
public:
  enum Attribute {                      //  class   prop   func  method param
    IsOverride             = (1 <<  0), //           x
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
    HasCall                = IsPublic,  //    x
    HasCallStatic          = IsProtected,//   x
    IgnoreRedefinition     = IsPrivate, //                  x
    IsReference            = (1 << 11), //                  x      x     x
    IsConstructor          = (1 << 12), //                         x

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
    AllowIntercept         = (1 << 24), //                  x      x
    NoProfile              = (1 << 25), //                  x      x
    ContextSensitive       = (1 << 26), //                  x
    NoDefaultSweep         = (1 << 27), //    x
    IsSystem               = (1 << 28), //    x             x

    IsTrait                = (1 << 29), //    x
    UsesTraits             = (1 << 30), //    x
    HasAliasedMethods      = (1u << 31),//    x
    NeedsActRec            = (1u << 31),//                  x      x
  };

  enum GetArrayKind {
    GetArrayNone = 0,
    GetArrayPrivate = 1,
    GetArrayDynamic = 2,

    GetArrayAll = GetArrayPrivate|GetArrayDynamic,
    GetArrayPublic = GetArrayDynamic
  };

  class ConstantInfo {
  public:
    ConstantInfo();

    String name;
    unsigned int valueLen;
    const char *valueText;
    const ObjectStaticCallbacks *callbacks;

    Variant getValue() const;
    void setValue(CVarRef value);
    void setStaticValue(CVarRef value);

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
    const char *valueText; // original PHP code
    std::vector<const UserAttributeInfo *> userAttrs;
  };

  struct MethodInfo {
    MethodInfo() : docComment(NULL) {}
    MethodInfo(const char **&p);
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
    PropertyInfo() : docComment(NULL) {}
    Attribute attribute;
    String name;
    const char *docComment;
    const ClassInfo *owner;
    bool isVisible(const ClassInfo *context) const;
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
  static const MethodInfo *FindSystemFunction(CStrRef name);
  static const MethodInfo *FindFunction(CStrRef name);

  /**
   * Return a list of declared classes.
   */
  static Array GetClasses() { return GetClassLike(IsInterface|IsTrait, 0); }

  static bool HasClassInterfaceOrTrait(CStrRef name) {
    return FindClassInterfaceOrTrait(name);
  }

  /**
   * Locate one class.
   */
  static const ClassInfo *FindClass(CStrRef name);

  /**
   * Locate one system class.
   */
  static const ClassInfo *FindSystemClass(CStrRef name);

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
  static const ClassInfo *FindInterface(CStrRef name);

  /**
   * Locate one system interface.
   */
  static const ClassInfo *FindSystemInterface(CStrRef name);

  /**
   * Locate one trait.
   */
  static const ClassInfo *FindTrait(CStrRef name);

  /**
   * Locate one system trait.
   */
  static const ClassInfo *FindSystemTrait(CStrRef name);

  /**
   * Locate either a class, interface, or trait.
   */
  static const ClassInfo *FindClassInterfaceOrTrait(CStrRef name);

  /**
   * Locate either a system class, system interface, or system trait.
   */
  static const ClassInfo *FindSystemClassInterfaceOrTrait(CStrRef name);

  /**
   * Locate one constant (excluding dynamic and redeclared constants)
   */
  static const ConstantInfo *FindConstant(CStrRef name);

  /**
   * Get all statically known constants
   */
  static Array GetConstants();

  /**
   * Return all methods a class has, including the ones on base classes and
   * interfaces.
   *   type: 0: unknown; 1: class; 2: interface
   */
  static bool GetClassMethods(MethodVec &ret, CStrRef classname, int type = 0);
  static bool GetClassMethods(MethodVec &ret, const ClassInfo *classInfo);

  /**
   * Return all properties a class has, including the ones on base classes and
   * the ones that were implicitly defined.
   */
  static void GetClassProperties(PropertyMap &props, CStrRef classname);
  static void GetClassProperties(PropertyVec &props, CStrRef classname);

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

  static void GetArray(const ObjectData *obj, const ClassPropTable *ct,
                       Array &props, GetArrayKind kind);
  static void GetArray(const ObjectData *obj, const ClassPropTable *ct,
                       Array &props, bool pubOnly) {
    GetArray(obj, ct, props, pubOnly ? GetArrayPublic : GetArrayAll);
  }
  static void SetArray(ObjectData *obj, const ClassPropTable *ct,
                       CArrRef props);
  static void SetHook(ClassInfoHook *hook) { s_hook = hook; }

public:
  ClassInfo() : m_cdec_offset(0), m_docComment(NULL) {}
  virtual ~ClassInfo() {}

  inline const ClassInfo *checkCurrent() const {
    ASSERT(!(m_attribute & IsRedeclared));
    return this;
  }

  Attribute getAttribute() const { return checkCurrent()->m_attribute;}
  const char *getFile() const { return checkCurrent()->m_file;}
  int getLine1() const { return checkCurrent()->m_line1;}
  int getLine2() const { return checkCurrent()->m_line2;}
  virtual const ClassInfo* getCurrentOrNull() const { return this; }
  virtual CStrRef getName() const { return m_name;}
  const char *getDocComment() const { return m_docComment; }

  virtual void postInit();

  /**
   * Parents of this class.
   */
  virtual CStrRef getParentClass() const = 0;
  const ClassInfo *getParentClassInfo() const;
  virtual const InterfaceSet &getInterfaces() const = 0;
  virtual const InterfaceVec &getInterfacesVec() const = 0;
  virtual const TraitSet &getTraits() const = 0;
  virtual const TraitVec &getTraitsVec() const = 0;
  virtual const TraitAliasVec &getTraitAliasesVec() const = 0;
  bool derivesFrom(CStrRef name, bool considerInterface) const;

  void getAllParentsVec(ClassVec &parents) const; // recursive
  void getAllInterfacesVec(InterfaceVec &interfaces) const; // recursive

  /**
   * Method functions.
   */
  virtual const MethodMap &getMethods() const = 0;    // non-recursively
  virtual const MethodVec &getMethodsVec() const = 0; // non-recursively
  MethodInfo *getMethodInfo(CStrRef name) const;
  MethodInfo *hasMethod(CStrRef name,
                        ClassInfo *&classInfo,
                        bool interfaces = false) const;
  static bool HasAccess(CStrRef className, CStrRef methodName,
                        bool staticCall, bool hasCallObject);
  static bool IsSubClass(CStrRef className1, CStrRef className2,
                         bool considerInterface);
  const char *getConstructor() const;

  /**
   * Property functions.
   */
  virtual const PropertyMap &getProperties() const = 0;    // non-recursively
  virtual const PropertyVec &getPropertiesVec() const = 0; // non-recursively
  void getAllProperties(PropertyMap &props) const;         // recursively
  void getAllProperties(PropertyVec &props) const;         // recursively
  // Remove properties with the given attribute from the array, recursively.
  void filterProperties(Array &props, Attribute toRemove) const;
  PropertyInfo *getPropertyInfo(CStrRef name) const;
  bool hasProperty(CStrRef name) const;

  /**
   * Constant functions.
   */
  virtual const ConstantMap &getConstants() const = 0;
  virtual const ConstantVec &getConstantsVec() const = 0;
  ConstantInfo *getConstantInfo(CStrRef name) const;
  bool hasConstant(CStrRef name) const;

  virtual const UserAttributeVec &getUserAttributeVec() const = 0;

protected:
  static bool s_loaded;            // whether class map is loaded
  static ClassInfo *s_systemFuncs; // all system functions
  static ClassInfo *s_userFuncs;   // all user functions

  static ClassInfoHook *s_hook;

  Attribute m_attribute;
  int m_cdec_offset;
  String m_name;
  const char *m_file;
  int m_line1;
  int m_line2;
  const char *m_docComment;

  bool derivesFromImpl(CStrRef name, bool considerInterface) const;
  bool checkAccess(ClassInfo *defClass, MethodInfo *methodInfo,
                   bool staticCall, bool hasObject) const;

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
  ClassInfoUnique(const char **&p);
  virtual ~ClassInfoUnique();

  // implementing ClassInfo
  CStrRef getParentClass() const { return m_parent;}
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
 * Stores info about a class that is redeclared.
 */
class ClassInfoRedeclared : public ClassInfo {
public:
  /**
   * Read one class's information from specified map pointer and move it.
   */
  ClassInfoRedeclared(const char **&p);

  // implementing ClassInfo
  virtual CStrRef getName() const { return current()->getName();}
  CStrRef getParentClass() const { return current()->getParentClass();}

  const InterfaceSet &getInterfaces() const {
    return current()->getInterfaces();
  }
  const InterfaceVec &getInterfacesVec() const {
    return current()->getInterfacesVec();
  }
  const TraitSet &getTraits() const {
    return current()->getTraits();
  }
  const TraitVec &getTraitsVec() const {
    return current()->getTraitsVec();
  }
  const TraitAliasVec &getTraitAliasesVec() const {
    return current()->getTraitAliasesVec();
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
  const UserAttributeVec &getUserAttributeVec() const {
    return current()->getUserAttributeVec();
  }

  virtual void postInit();
private:
  std::vector<ClassInfo*> m_redeclaredClasses;
  int m_redeclaredIdOffset;

  const ClassInfo* getCurrentOrNull() const;
  const ClassInfo* current() const {
    not_reached();
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
  virtual Array getTraits() const = 0;
  virtual Array getConstants() const = 0;
  virtual const ClassInfo::MethodInfo *findFunction(CStrRef name) const = 0;
  virtual const ClassInfo *findClassLike(CStrRef name) const = 0;
  virtual const ClassInfo::ConstantInfo *findConstant(CStrRef name) const = 0;
};

struct ClassPropTableEntry {
  enum PropFlags {
    Private = 1,
    Protected = 2,
    Public = 4,
    Static = 8,
    Override = 16,
    Constant = 32,
    Last = 64,
    FastInit = 128
  };

  strhash_t     hash;
  int16_t       next;
  uint16_t      init_offset; // change to uint32 if we overflow
  uint16_t      prop_offset;
  uint8_t       flags; // PropFlags
  int8_t        type;  // DataType
  int32_t       offset;
  StaticString *keyName;
  bool isPublic() const { return flags & Public; }
  bool isPrivate() const { return flags & Private; }
  bool isOverride() const { return flags & Override; }
  bool isStatic() const { return flags & Static; }
  bool isConstant() const { return flags & Constant; }
  bool isLast() const { return flags & Last; }
  bool isFastInit() const { return flags & FastInit; }

  static Variant GetVariant(DataType type, const void *addr) {
    switch (type) {
      case KindOfBoolean:
        return *(bool*)addr;
      case KindOfInt64:
        return *(int64*)addr;
      case KindOfDouble:
        return *(double*)addr;
      case KindOfString:
        return *(String*)addr;
      case KindOfArray:
        return *(Array*)addr;
      case KindOfObject:
        return *(Object*)addr;
      default:
        ASSERT(false);
        return null;
    }
  }

  Variant getVariant(const void *addr) const {
    return GetVariant(DataType(type), addr);
  }
};

class ClassPropTable {
public:
  int m_size_mask;
  int m_offset;
  int m_static_size_mask;
  int m_static_offset;
  int m_const_size_mask;
  int m_const_offset;

  int m_lazy_inits_list;
  int m_lazy_init_offset;

  const int *m_hash_entries;
  const ClassPropTable *m_parent;
  const ClassPropTableEntry *m_entries;
  const int64 *m_static_inits;

  Variant getInitVal(const ClassPropTableEntry *prop) const;
  CVarRef getInitV(int id) const {
    return *(Variant*)m_static_inits[id];
  }
  CStrRef getInitS(int id) const {
    return getInitV(id).asCStrRef();
  }
  void *getInitP(int id) const {
    return (void*)m_static_inits[id];
  }

  const int *privates() const {
    return m_hash_entries + m_size_mask + 1;
  }

  const int *lazy_inits() const {
    return m_hash_entries + m_lazy_inits_list;
  }
};

#define GET_PROPERTY_OFFSET(c, n) \
((offsetof(c, n) - (int64)static_cast<ObjectData *>((c*)0)))

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_CLASS_INFO_H__
