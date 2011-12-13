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

#ifndef incl_VM_CLASS_H_
#define incl_VM_CLASS_H_

#include <runtime/vm/core_types.h>
#include <runtime/base/array/hphp_array.h>
#include <runtime/ext_hhvm/ext_hhvm.h>
#include <util/parser/location.h>

namespace HPHP {

// Forward declaration.
class ClassInfoVM;

namespace VM {

// Forward declarations.
class Func;
class Unit;
class Class;

typedef hphp_hash_set<const StringData*, string_data_hash,
                      string_data_isame> TraitNameSet;

class PreClass : public Countable {
public:
  class Prop {
  public:
    Prop(PreClass* preClass, const StringData* n, Attr attrs,
         const StringData* docComment, TypedValue* val);
    ~Prop();

  private:
    const StringData* mangleName() const;
  public:
    void prettyPrint(std::ostream& out);

    PreClass* m_preClass;
    const StringData* m_name;
    const StringData* m_mangledName;
    Attr m_attrs;
    const StringData* m_docComment;
    TypedValue m_val;
  };
  class Const {
  public:
    Const(PreClass* preClass, const StringData* n, TypedValue* val,
          const StringData* phpCode);
    ~Const();

    void prettyPrint(std::ostream& out);

    PreClass* m_preClass;
    const StringData* m_name;
    TypedValue m_val;
    const StringData* m_phpCode;
  };

  class TraitPrecRule {
  public:
    TraitPrecRule(const StringData* selectedTraitName,
                  const StringData* methodName) :
        m_methodName(methodName), m_selectedTraitName(selectedTraitName),
        m_otherTraitNames() {}

    void addOtherTraitName(const StringData* traitName) {
      m_otherTraitNames.insert(traitName);
    }
    const StringData* getMethodName() const { return m_methodName; }
    const StringData* getSelectedTraitName() const {
      return m_selectedTraitName;
    }
    void getOtherTraitNames(TraitNameSet& nameSet) const {
      nameSet = m_otherTraitNames;
    }
  private:
    const StringData*  m_methodName;
    const StringData*  m_selectedTraitName;
    TraitNameSet       m_otherTraitNames;
  };

  class TraitAliasRule {
  public:
    TraitAliasRule(const StringData* traitName,
                   const StringData* origMethodName,
                   const StringData* newMethodName,
                   Attr              modifiers) :
        m_traitName(traitName),
        m_origMethodName(origMethodName),
        m_newMethodName(newMethodName),
        m_modifiers(modifiers) {}
    const StringData* getTraitName() const      { return m_traitName; }
    const StringData* getOrigMethodName() const { return m_origMethodName; }
    const StringData* getNewMethodName() const  { return m_newMethodName; }
    Attr              getModifiers() const      { return m_modifiers; }
  private:
    const StringData* m_traitName;
    const StringData* m_origMethodName;
    const StringData* m_newMethodName;
    Attr              m_modifiers;
  };

  PreClass(Unit* unit, const Location* sLoc, Offset o, const StringData* n,
           Attr attrs, const StringData* parent, const StringData* docComment,
           Id id, bool hoistable);
  ~PreClass();
  void release();
  IMPLEMENT_ATOMIC_COUNTABLE_METHODS

  void addInterface(const StringData* n);
  bool addMethod(Func* method);
  bool hasMethod(const StringData* methName) {
    return m_methodMap.find(methName) != m_methodMap.end();
  }
  bool addProperty(const StringData* n, Attr attrs,
                   const StringData* docComment, TypedValue* val);
  bool addConstant(const StringData* n, TypedValue* val,
                   const StringData* phpCode);
  void addUsedTrait(const StringData* traitName) {
    m_usedTraits.push_back(traitName);
  }
  void addTraitPrecRule(TraitPrecRule &rule) {
    m_traitPrecRules.push_back(rule);
  }
  void addTraitAliasRule(TraitAliasRule &rule) {
    m_traitAliasRules.push_back(rule);
  }

  void prettyPrint(std::ostream& out) const;

  Unit* m_unit;
  int m_line1;
  int m_line2;
  Offset m_offset;
  const StringData* m_name;
  Attr m_attrs;
  const StringData* m_parent;
  const StringData* m_docComment;
  std::vector<const StringData*> m_interfaces;
  std::vector<const StringData*> m_usedTraits;
  std::vector<TraitPrecRule>     m_traitPrecRules;
  std::vector<TraitAliasRule>    m_traitAliasRules;
  Id m_id;
  bool m_hoistable;
  Func* m_ctor;
  typedef std::vector<Func*> MethodVec;
  MethodVec m_methods;
  typedef hphp_hash_map<const StringData*, Func*, string_data_hash,
                        string_data_isame> MethodMap;
  MethodMap m_methodMap;
  typedef std::vector<Prop*> PropertyVec;
  PropertyVec m_propertyVec;
  typedef hphp_hash_map<const StringData*, Prop*, string_data_hash,
                        string_data_same> PropertyMap;
  PropertyMap m_propertyMap;
  typedef std::vector<Const*> ConstantVec;
  ConstantVec m_constantVec;
  typedef hphp_hash_map<const StringData*, unsigned, string_data_hash,
                        string_data_same> ConstantMap;
  ConstantMap m_constantMap;
};
// It is possible for multiple Class'es to refer to the same PreClass, and we
// need to make sure that the PreClass lives for as long as an associated Class
// still exists (even if the associated Unit has been unloaded).  Therefore,
// use SmartPtr's to to enforce this invariant.
typedef SmartPtr<PreClass> PreClassPtr;

typedef SmartPtr<Class> ClassPtr;
struct Class : public Countable {
  static hphp_hash_map<const StringData*, const HhbcExtClassInfo*,
                       string_data_hash, string_data_isame> s_extClassHash;
  class Prop {
  public:
    // m_name is "" for inaccessible properties (i.e. private properties
    // declared by parents).
    const StringData* m_name;
    const StringData* m_mangledName;
    Class* m_class; // First parent class that declares this property.
    Attr m_attrs;
    const StringData* m_docComment;
  };
  class SProp {
  public:
    const StringData* m_name;
    Attr m_attrs;
    const StringData* m_docComment;
    Class* m_class; // Most derived class that declared this property.
    TypedValue m_val; // Used if (m_class == this).
  };
  class Const {
  public:
    Class* m_class; // Most derived class that declared this constant.
    const StringData* m_name;
    TypedValue m_val;
    const StringData* m_phpCode;
  };
  // Call newClass() instead of directly calling new.
  static ClassPtr newClass(PreClass* preClass, Class* parent, bool failIsFatal);
  Class(PreClass* preClass, Class* parent, unsigned classVecLen,
        bool failIsFatal, bool& fail);
  void release();
  IMPLEMENT_ATOMIC_COUNTABLE_METHODS

  static size_t sizeForNClasses(unsigned nClasses) {
    return offsetof(Class, m_classVec) + (sizeof(Class*) * nClasses);
  }

  enum Equiv {
    EquivFalse,
    EquivTrue,
    EquivFail
  };
  Equiv equiv(const PreClass* preClass, bool tryAutoload = false) const;
  Class* classof(const PreClass* preClass) const;
  const StringData* name() const {
    return m_preClass->m_name;
  }

  PreClassPtr m_preClass;
  ClassPtr m_parent;
  std::vector<ClassPtr> m_interfaces;
  std::vector<ClassPtr> m_usedTraits;
  std::vector<std::pair<const StringData*, const StringData*> > m_traitAliases;

  // Methods.
  //
  // The m_methods vector contains an entry for every method that can be called
  // in the context of this Class (but no private methods for parent classes).
  // Use m_methodMap to look up a method by name.
  typedef hphp_hash_map<const StringData*, unsigned,
                        string_data_hash, string_data_isame> MethodMap;
  MethodMap m_methodMap; // Map name --> m_methods index.
  struct MethodEntry {
    Func* func;
    Class* baseClass;     // Class where this method is first declared
    bool ancestorPrivate; // Indicates if an ancestor has a private method
    Attr attrs;       // usually same as func's, but may be != for trait aliases
  };
  std::vector<MethodEntry> m_methods;
  MethodEntry m_ctor;

  const Func* lookupMethod(const StringData* methName) const {
    unsigned idx;
    if (!mapGet(m_methodMap, methName, &idx)) {
      return NULL;
    }
    return m_methods[idx].func;
  }

  const MethodEntry* lookupMethodEntry(const StringData* methName) const {
    unsigned idx;
    if (!mapGet(m_methodMap, methName, &idx)) {
      return NULL;
    }
    return &m_methods[idx];
  }

  // Finds the base class defining the given method (NULL if none).
  // Note: for methods imported via traits, the base class is the one that
  // uses/imports the trait.
  Class* findMethodBaseClass(const StringData* methName) {
    const MethodEntry* mentry = lookupMethodEntry(methName);
    if (mentry == NULL) return NULL;
    return mentry->baseClass;
  }

  // ObjectData attributes, to be set during Instance initialization.
  int m_ODAttrs;

  // Vector of 86pinit() methods that need to be called to complete instance
  // property initialization, and a pointer to a 86sinit() method that needs to
  // be called to complete static property initialization (or NULL).  Such
  // initialization is triggered only once, the first time one of the following
  // happens:
  //
  // + An instance of this class is created.
  // + A static property of this class is accessed.
  typedef std::vector<const Func*> InitVec;
  InitVec m_pinitVec;
  const Func* m_sinit;
  bool m_needInitialization; // True if there are any __[ps]init() methods.
  void initialize(HphpArray*& sPropData) const;
  void initialize() const;

  int m_builtinPropSize;
  Instance* (*m_InstanceCtor)(Class* cls);
  bool m_isCppExtClass;
  bool m_derivesFromBuiltin;
  Class* m_baseBuiltinCls;
  void addBuiltinClassInfo(const HhbcExtClassInfo *info);

  // Class constants.
  typedef std::vector<Const> ConstantVec;
  ConstantVec m_constantVec;
  typedef hphp_hash_map<const StringData*, unsigned, string_data_hash,
                        string_data_same> ConstantMap;
  ConstantMap m_constantMap;

private:
  HphpArray* initClsCnsData();
public:
  TypedValue* clsCnsGet(const StringData* clsCnsName);

  // Properties.
  //
  // Each Instance is created with enough trailing space to directly store the
  // vector of declared properties.  To look up a property by name and
  // determine whether it is declared, use m_declPropMap.  If the declared
  // property index is already known (as may be the case when executing via the
  // TC), property metadata in m_declPropInfo can be directly accessed.
  typedef std::vector<Prop> PropInfoVec;
  PropInfoVec m_declPropInfo;
  typedef std::vector<TypedValue> PropInitVec;
  PropInitVec m_declPropInit;
  int m_declPropNumAccessible;
  typedef hphp_hash_map<const StringData*, unsigned,
                        string_data_hash, string_data_same> PropMap;
  PropMap m_declPropMap;

private:
  PropInitVec* initProps() const;
public:

  int lookupDeclProp(const StringData* propName) const {
    PropMap::const_iterator it = m_declPropMap.find(propName);
    if (it != m_declPropMap.end()) {
      return it->second;
    }
    return -1;
  }

  // Static properties.
  typedef std::vector<SProp> SPropInfoVec;
  SPropInfoVec m_sPropInfo;
  typedef hphp_hash_map<const StringData*, unsigned,
                        string_data_hash, string_data_same> SPropMap;
  SPropMap m_sPropMap;

private:
  HphpArray* initSProps() const;
public:
  TypedValue* getSProp(PreClass* ctx, const StringData* sPropName,
                       bool& visible, bool& accessible) const;

  int lookupSProp(const StringData* sPropName) const {
    PropMap::const_iterator it = m_sPropMap.find(sPropName);
    if (it != m_sPropMap.end()) {
      return it->second;
    }
    return -1;
  }

  TypedValue getStaticPropInitVal(const SProp& prop);

  HphpArray* getStaticLocals();

  void getClassInfo(ClassInfoVM* ci);

private:

  struct TraitMethod {
    ClassPtr          m_trait;
    Func*             m_method;
    Attr              m_modifiers;
    TraitMethod(ClassPtr trait, Func* method, Attr modifiers) :
        m_trait(trait), m_method(method), m_modifiers(modifiers) { }
  };

  typedef std::list<TraitMethod> TraitMethodList;
  typedef hphp_hash_map<const StringData*, TraitMethodList, string_data_hash,
                        string_data_isame> MethodToTraitListMap;
  MethodToTraitListMap m_importMethToTraitMap;

  void addImportTraitMethod(const TraitMethod &traitMethod,
                            const StringData  *methName);
  bool importTraitMethod(const TraitMethod &traitMethod,
                         const StringData  *methName,
                         bool               failIsFatal);
  ClassPtr findSingleTraitWithMethod(const StringData* methName);
  void setImportTraitMethodModifiers(const StringData* methName,
                                     ClassPtr          traitCls,
                                     Attr              modifiers);
  bool importTraitMethods(bool failIsFatal);
  bool applyTraitRules(bool failIsFatal);
  bool applyTraitPrecRule(const PreClass::TraitPrecRule& rule,
                          bool                           failIsFatal);
  bool applyTraitAliasRule(const PreClass::TraitAliasRule& rule,
                           bool                            failIsFatal);
  bool importTraitProps(bool failIsFatal);
  bool importTraitInstanceProp(ClassPtr    trait,
                               Prop&       traitProp,
                               TypedValue& traitPropVal,
                               bool        failIsFatal);
  bool importTraitStaticProp(ClassPtr trait,
                             SProp&   traitProp,
                             bool     failIsFatal);
  void addTraitAlias(const StringData* traitName,
                     const StringData* origMethName,
                     const StringData* newMethName);

  static bool compatibleTraitPropInit(TypedValue& tv1, TypedValue& tv2);
  void removeImplTraitAbstractMethods();

  bool validateParent(bool failIsFatal);
  bool setCtor(bool failIsFatal);
  bool setMethods(bool failIsFatal);
  bool setODAttributes(bool failIsFatal);
  bool setConstants(bool failIsFatal);
  bool setProperties(bool failIsFatal);
  bool setInitializers(bool failIsFatal);
  bool setInterfaces(bool failIsFatal);
  bool setClassVec(bool failIsFatal);
  bool setUsedTraits(bool failIsFatal);

public:
  unsigned m_classVecLen;
  // Vector of Class pointers that encodes the inheritance hierarchy, including
  // this Class as the last element.  This vector enables fast type
  // compatibility checks in translated code when accessing declared
  // properties.
  Class* m_classVec[1]; // Dynamically sized; must come last.

  friend class ExecutionContext;
};

struct class_hash {
  size_t operator()(const Class* c) const {
    return hash_int64((intptr_t)c);
  }
};

struct class_same {
  bool operator()(const Class* c1, const Class* c2) const {
    ASSERT(c1 && c2);
    return (void*)c1 == (void*)c2;
  }
};

} } // HPHP::VM

#endif
