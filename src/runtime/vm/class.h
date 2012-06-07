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
#include <runtime/vm/repo_helpers.h>
#include <runtime/base/array/hphp_array.h>
#include <runtime/ext_hhvm/ext_hhvm.h>
#include <util/parser/location.h>
#include <runtime/vm/fixed_string_map.h>
#include <runtime/vm/indexed_string_map.h>

namespace HPHP {

// Forward declaration.
class ClassInfo;
class ClassInfoVM;

namespace VM {

// Forward declarations.
class Func;
class FuncEmitter;
class Unit;
class UnitEmitter;
class Class;
class Instance;
class NamedEntity;
class PreClass;

typedef hphp_hash_set<const StringData*, string_data_hash,
                      string_data_isame> TraitNameSet;
typedef hphp_hash_set<const Class*, pointer_hash<Class> > ClassSet;

typedef Instance*(*BuiltinCtorFunction)(Class*);

class PreClass : public AtomicCountable {
 public:
  class Prop {
   public:
    Prop(PreClass* preClass, const StringData* n, Attr attrs,
         const StringData* docComment, const TypedValue& val);
    ~Prop();

    void prettyPrint(std::ostream& out);

    PreClass* preClass() const { return m_preClass; }
    const StringData* name() const { return m_name; }
    CStrRef nameRef() const { return *(String*)&m_name; }
    const StringData* mangledName() const { return m_mangledName; }
    CStrRef mangledNameRef() const { return *(String*)(&m_mangledName); }
    Attr attrs() const { return m_attrs; }
    const StringData* docComment() const { return m_docComment; }
    const TypedValue& val() const { return m_val; }
   private:
    PreClass* m_preClass;
    const StringData* m_name;
    const StringData* m_mangledName;
    Attr m_attrs;
    const StringData* m_docComment;
    TypedValue m_val;
  };
  class Const {
   public:
    Const(PreClass* preClass, const StringData* n, const TypedValue& val,
          const StringData* phpCode);
    ~Const();

    void prettyPrint(std::ostream& out);

    PreClass* preClass() const { return m_preClass; }
    const StringData* name() const { return m_name; }
    CStrRef nameRef() const { return *(String*)&m_name; }
    const TypedValue& val() const { return m_val; }
    const StringData* phpCode() const { return m_phpCode; }
   private:
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

  typedef std::vector<const StringData*> InterfaceVec;
  typedef std::vector<const StringData*> UsedTraitVec;
  typedef std::vector<TraitPrecRule> TraitPrecRuleVec;
  typedef std::vector<TraitAliasRule> TraitAliasRuleVec;
  typedef std::vector<Func*> MethodVec;
  typedef FixedStringMap<Func*, false> MethodMap;
  typedef std::vector<Prop*> PropertyVec;
  typedef hphp_hash_map<const StringData*, Prop*, string_data_hash,
                        string_data_same> PropertyMap;
  typedef std::vector<Const*> ConstantVec;
  typedef hphp_hash_map<const StringData*, unsigned, string_data_hash,
                        string_data_same> ConstantMap;
  typedef hphp_hash_map<const StringData*, TypedValue, string_data_hash,
                        string_data_isame> UserAttributeMap;

  PreClass(Unit* unit, int line1, int line2, Offset o, const StringData* n,
           Attr attrs, const StringData* parent, const StringData* docComment,
           Id id, bool hoistable);
  ~PreClass();
  void atomicRelease();

  Unit* unit() const { return m_unit; }
  int line1() const { return m_line1; }
  int line2() const { return m_line2; }
  const StringData* name() const { return m_name; }
  CStrRef nameRef() const { return *(String*)(&m_name); }
  Attr attrs() const { return m_attrs; }
  static Offset attrsOffset() { return offsetof(PreClass, m_attrs); }
  const StringData* parent() const { return m_parent; }
  CStrRef parentRef() const { return *(String*)(&m_parent); }
  const StringData* docComment() const { return m_docComment; }
  Id id() const { return m_id; }
  const InterfaceVec& interfaces() const { return m_interfaces; }
  const UsedTraitVec& usedTraits() const { return m_usedTraits; }
  const TraitPrecRuleVec& traitPrecRules() const { return m_traitPrecRules; }
  const TraitAliasRuleVec& traitAliasRules() const { return m_traitAliasRules; }
  const UserAttributeMap& userAttributes() const { return m_userAttributes; }
  const MethodVec& methods() const { return m_methods; }
  bool hasMethod(const StringData* methName) const {
    return m_methodMap.find(methName) != NULL;
  }
  Func* lookupMethod(const StringData* methName) const {
    Func **f = m_methodMap.find(methName);
    ASSERT(f != NULL);
    return *f;
  }

  BuiltinCtorFunction instanceCtor() { return m_InstanceCtor; }
  int builtinPropSize() { return m_builtinPropSize; }

  const PropertyVec& propertyVec() const { return m_propertyVec; }
  bool hasProp(const StringData* propName) const {
    return m_propertyMap.find(propName) != m_propertyMap.end();
  }
  Prop* lookupProp(const StringData* propName) const {
    PropertyMap::const_iterator it = m_propertyMap.find(propName);
    ASSERT(it != m_propertyMap.end());
    return it->second;
  }
  const ConstantVec& constantVec() const { return m_constantVec; }

  void prettyPrint(std::ostream& out) const;

  const NamedEntity* namedEntity() const { return m_namedEntity; }
 private:
  friend class PreClassEmitter;
  friend class Peephole;
  Unit* m_unit;
  const NamedEntity* m_namedEntity;
  int m_line1;
  int m_line2;
  Offset m_offset;
  Id m_id;
  int m_builtinPropSize;
  Attr m_attrs;
  bool m_hoistable;
  const StringData* m_name;
  const StringData* m_parent;
  const StringData* m_docComment;
  BuiltinCtorFunction m_InstanceCtor;
  InterfaceVec m_interfaces;
  UsedTraitVec m_usedTraits;
  TraitPrecRuleVec m_traitPrecRules;
  TraitAliasRuleVec m_traitAliasRules;
  UserAttributeMap m_userAttributes;
  MethodVec m_methods;
  MethodMap m_methodMap;
  PropertyVec m_propertyVec;
  PropertyMap m_propertyMap;
  ConstantVec m_constantVec;
  ConstantMap m_constantMap;
};
// It is possible for multiple Class'es to refer to the same PreClass, and we
// need to make sure that the PreClass lives for as long as an associated Class
// still exists (even if the associated Unit has been unloaded).  Therefore,
// use AtomicSmartPtr's to enforce this invariant.
typedef AtomicSmartPtr<PreClass> PreClassPtr;
class PreClassEmitter {
 public:
  class Prop {
   public:
    Prop(const PreClassEmitter* pce, const StringData* n, Attr attrs,
         const StringData* docComment, TypedValue* val);
    ~Prop();

    const StringData* name() const { return m_name; }
    const StringData* mangledName() const { return m_mangledName; }
    Attr attrs() const { return m_attrs; }
    const StringData* docComment() const { return m_docComment; }
    const TypedValue& val() const { return m_val; }
   private:
    const StringData* m_name;
    const StringData* m_mangledName;
    Attr m_attrs;
    const StringData* m_docComment;
    TypedValue m_val;
  };
  class Const {
   public:
    Const(const StringData* n, TypedValue* val, const StringData* phpCode)
      : m_name(n), m_phpCode(phpCode) {
      memcpy(&m_val, val, sizeof(TypedValue));
    }
    ~Const() {}

    const StringData* name() const { return m_name; }
    const TypedValue& val() const { return m_val; }
    const StringData* phpCode() const { return m_phpCode; }
   private:
    const StringData* m_name;
    TypedValue m_val;
    const StringData* m_phpCode;
  };

  typedef std::vector<Prop*> PropertyVec;
  typedef hphp_hash_map<const StringData*, Prop*, string_data_hash,
                        string_data_same> PropertyMap;
  typedef std::vector<Const*> ConstantVec;
  typedef hphp_hash_map<const StringData*, unsigned, string_data_hash,
                        string_data_same> ConstantMap;

  PreClassEmitter(UnitEmitter& ue, int line1, int line2, Offset o,
                  const StringData* n, Attr attrs, const StringData* parent,
                  const StringData* docComment, Id id, bool hoistable);
  ~PreClassEmitter();

  UnitEmitter& ue() const { return m_ue; }
  const StringData* name() const { return m_name; }
  Attr attrs() const { return m_attrs; }
  Id id() const { return m_id; }
  typedef std::vector<FuncEmitter*> MethodVec;
  const MethodVec& methods() const { return m_methods; }

  void addInterface(const StringData* n);
  bool addMethod(FuncEmitter* method);
  bool addProperty(const StringData* n, Attr attrs,
                   const StringData* docComment, TypedValue* val);
  Prop* lookupProp(const StringData* propName) const;
  bool addConstant(const StringData* n, TypedValue* val,
                   const StringData* phpCode);
  void addUsedTrait(const StringData* traitName);
  void addTraitPrecRule(PreClass::TraitPrecRule &rule);
  void addTraitAliasRule(PreClass::TraitAliasRule &rule);
  void addUserAttribute(const StringData* name, TypedValue tv);
  void commit(RepoTxn& txn) const;

  void setBuiltinClassInfo(const ClassInfo* info,
                           BuiltinCtorFunction ctorFunc,
                           int sz);

  PreClass* create(Unit& unit) const;

 private:
  UnitEmitter& m_ue;
  int m_line1;
  int m_line2;
  Offset m_offset;
  const StringData* m_name;
  Attr m_attrs;
  const StringData* m_parent;
  const StringData* m_docComment;
  Id m_id;
  bool m_hoistable;
  BuiltinCtorFunction m_InstanceCtor;
  int m_builtinPropSize;

  PreClass::InterfaceVec m_interfaces;
  PreClass::UsedTraitVec m_usedTraits;
  PreClass::TraitPrecRuleVec m_traitPrecRules;
  PreClass::TraitAliasRuleVec m_traitAliasRules;
  PreClass::UserAttributeMap m_userAttributes;
  MethodVec m_methods;
  typedef hphp_hash_map<const StringData*, FuncEmitter*, string_data_hash,
                        string_data_isame> MethodMap;
  MethodMap m_methodMap;
  PropertyVec m_propertyVec;
  PropertyMap m_propertyMap;
  ConstantVec m_constantVec;
  ConstantMap m_constantMap;
};

class PreClassRepoProxy : public RepoProxy {
  friend class PreClass;
  friend class PreClassEmitter;
 public:
  PreClassRepoProxy(Repo& repo);
  ~PreClassRepoProxy();
  void createSchema(int repoId, RepoTxn& txn);

#define PCRP_IOP(o) PCRP_OP(Insert##o, insert##o)
#define PCRP_GOP(o) PCRP_OP(Get##o, get##o)
#define PCRP_OPS \
  PCRP_IOP(PreClass) \
  PCRP_GOP(PreClasses) \
  PCRP_IOP(PreClassInterface) \
  PCRP_GOP(PreClassInterfaces) \
  PCRP_IOP(PreClassTrait) \
  PCRP_GOP(PreClassTraits) \
  PCRP_IOP(PreClassTraitPrec) \
  PCRP_GOP(PreClassTraitPrecs) \
  PCRP_IOP(PreClassTraitPrecOther) \
  PCRP_GOP(PreClassTraitPrecOthers) \
  PCRP_IOP(PreClassTraitAlias) \
  PCRP_GOP(PreClassTraitAliases) \
  PCRP_IOP(PreClassUserAttribute) \
  PCRP_GOP(PreClassUserAttributes) \
  PCRP_IOP(PreClassProperty) \
  PCRP_GOP(PreClassProperties) \
  PCRP_IOP(PreClassConstant) \
  PCRP_GOP(PreClassConstants)
  class InsertPreClassStmt : public RepoProxy::Stmt {
   public:
    InsertPreClassStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, Id preClassId, int line1, int line2,
                Offset offset, const StringData* name, Attr attrs,
                const StringData* parent, const StringData* docComment,
                bool hoistable);
  };
  class GetPreClassesStmt : public RepoProxy::Stmt {
   public:
    GetPreClassesStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
  class InsertPreClassInterfaceStmt : public RepoProxy::Stmt {
   public:
    InsertPreClassInterfaceStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, Id preClassId, int interfaceSn,
                const StringData* name);
  };
  class GetPreClassInterfacesStmt : public RepoProxy::Stmt {
   public:
    GetPreClassInterfacesStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(PreClassEmitter& pce);
  };
  class InsertPreClassTraitStmt : public RepoProxy::Stmt {
   public:
    InsertPreClassTraitStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, Id preClassId, int traitSn,
                const StringData* name);
  };
  class GetPreClassTraitsStmt : public RepoProxy::Stmt {
   public:
    GetPreClassTraitsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(PreClassEmitter& pce);
  };
  class InsertPreClassTraitPrecStmt : public RepoProxy::Stmt {
   public:
    InsertPreClassTraitPrecStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, Id preClassId, int traitPrecSn,
                const StringData* methodName,
                const StringData* selectedTraitName);
  };
  class GetPreClassTraitPrecsStmt : public RepoProxy::Stmt {
   public:
    GetPreClassTraitPrecsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(PreClassEmitter& pce);
  };
  class InsertPreClassTraitPrecOtherStmt : public RepoProxy::Stmt {
   public:
    InsertPreClassTraitPrecOtherStmt(Repo& repo, int repoId)
      : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, Id preClassId, int traitPrecSn,
                int otherSn, const StringData* other);
  };
  class GetPreClassTraitPrecOthersStmt : public RepoProxy::Stmt {
   public:
    GetPreClassTraitPrecOthersStmt(Repo& repo, int repoId)
      : Stmt(repo, repoId) {}
    void get(PreClassEmitter& pce, int traitPrecSn,
             PreClass::TraitPrecRule& tpr);
  };
  class InsertPreClassTraitAliasStmt : public RepoProxy::Stmt {
   public:
    InsertPreClassTraitAliasStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, Id preClassId, int traitAliasSn,
                const StringData* name, const StringData* origMethodName,
                const StringData* newMethodName, Attr modifiers);
  };
  class GetPreClassTraitAliasesStmt : public RepoProxy::Stmt {
   public:
    GetPreClassTraitAliasesStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(PreClassEmitter& pce);
  };
  class InsertPreClassUserAttributeStmt : public RepoProxy::Stmt {
   public:
    InsertPreClassUserAttributeStmt(Repo& repo, int repoId)
      : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, Id preClassId,
                const StringData* name, const TypedValue& tv);
  };
  class GetPreClassUserAttributesStmt : public RepoProxy::Stmt {
   public:
    GetPreClassUserAttributesStmt(Repo& repo, int repoId)
      : Stmt(repo, repoId) {}
    void get(PreClassEmitter& pce);
  };
  class InsertPreClassPropertyStmt : public RepoProxy::Stmt {
   public:
    InsertPreClassPropertyStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, Id preClassId, int propertySn,
                const StringData* name, Attr attrs,
                const StringData* docComment, const TypedValue& val);
  };
  class GetPreClassPropertiesStmt : public RepoProxy::Stmt {
   public:
    GetPreClassPropertiesStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(PreClassEmitter& pce);
  };
  class InsertPreClassConstantStmt : public RepoProxy::Stmt {
   public:
    InsertPreClassConstantStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, Id preClassId, int constantSn,
                const StringData* name, const TypedValue& val,
                const StringData* phpCode);
  };
  class GetPreClassConstantsStmt : public RepoProxy::Stmt {
   public:
    GetPreClassConstantsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(PreClassEmitter& pce);
  };
#define PCRP_OP(c, o) \
 public: \
  c##Stmt& o(int repoId) { return *m_##o[repoId]; } \
 private: \
  c##Stmt m_##o##Local; \
  c##Stmt m_##o##Central; \
  c##Stmt* m_##o[RepoIdCount];
  PCRP_OPS
#undef PCRP_OP
 private:
  int m_dummy; // Used to avoid a syntax error in the ctor initializer list.
};

typedef AtomicSmartPtr<Class> ClassPtr;
struct Class : public AtomicCountable {
  friend class ExecutionContext;
  friend class Instance;

  enum Avail {
    AvailFalse,
    AvailTrue,
    AvailFail
  };

  struct Prop {
    // m_name is "" for inaccessible properties (i.e. private properties
    // declared by parents).
    const StringData* m_name;
    const StringData* m_mangledName;
    const StringData* m_originalMangledName;
    Class* m_class; // First parent class that declares this property.
    Attr m_attrs;
    const StringData* m_docComment;
  };

  struct SProp {
    const StringData* m_name;
    Attr m_attrs;
    const StringData* m_docComment;
    Class* m_class; // Most derived class that declared this property.
    TypedValue m_val; // Used if (m_class == this).
  };

  struct Const {
    Class* m_class; // Most derived class that declared this constant.
    const StringData* m_name;
    TypedValue m_val;
    const StringData* m_phpCode;
    CStrRef nameRef() const { return *(String*)&m_name; }
  };

  class PropInitVec {
  public:
    PropInitVec();
    const PropInitVec& operator=(const PropInitVec&);
    ~PropInitVec();
    static PropInitVec* allocSmartAllocated(const PropInitVec& src);

    typedef TypedValue* iterator;
    iterator begin() { return m_data; }
    iterator end() { return m_data + m_size; }
    TypedValue& operator[](size_t i) { ASSERT(i < m_size); return m_data[i]; }
    const TypedValue& operator[](size_t i) const {
      ASSERT(i < m_size); return m_data[i];
    }
    void push_back(const TypedValue& v);
    size_t size() const { return m_size; }

  private:
    PropInitVec(const PropInitVec&);
    TypedValue* m_data;
    unsigned m_size;
    bool m_smart;
  };

  typedef std::vector<const Func*> InitVec;
  typedef std::vector<std::pair<const StringData*, const StringData*> >
          TraitAliasVec;

public:
  // Call newClass() instead of directly calling new.
  static ClassPtr newClass(PreClass* preClass, Class* parent, bool failIsFatal);
  Class(PreClass* preClass, Class* parent, unsigned classVecLen,
        bool failIsFatal, bool& fail);
  void atomicRelease();

  static size_t sizeForNClasses(unsigned nClasses) {
    return offsetof(Class, m_classVec) + (sizeof(Class*) * nClasses);
  }

  Avail avail(Class *&parent, bool tryAutoload = false) const;
  Class* classof(const PreClass* preClass) const;
  bool classof(const Class* cls) const;
  const StringData* name() const {
    return m_preClass->name();
  }
  Class* parent() const {
    return m_parent.get();
  }
  CStrRef nameRef() const {
    return m_preClass->nameRef();
  }
  CStrRef parentRef() const {
    return m_preClass->parentRef();
  }

  Func* const* methods() const { return m_methods.accessList(); }
  size_t numMethods() const { return m_methods.size(); }
  const SProp* staticProperties() const
    { return m_staticProperties.accessList(); }
  size_t numStaticProperties() const { return m_staticProperties.size(); }
  const Prop* declProperties() const { return m_declProperties.accessList(); }
  size_t numDeclProperties() const { return m_declProperties.size(); }
  const Const* constants() const { return m_constants.accessList(); }
  size_t numConstants() const { return m_constants.size(); }
  Attr attrs() const { return m_preClass->attrs(); }
  const Func* getCtor() const { return m_ctor; }
  const Func* getToString() const { return m_toString; }
  const PreClass* preClass() const { return m_preClass.get(); }
  const ClassInfo* clsInfo() const { return m_clsInfo; }
  const PropInitVec* getPropData() const;
  void setPropData(PropInitVec* propData) const;
  HphpArray* getSPropData() const;
  void setSPropData(HphpArray* sPropData) const;
  bool needInitialization() const { return m_needInitialization; }
  bool needInstanceInit() const { return m_needInstanceInit; }
  bool derivesFromBuiltin() const { return m_derivesFromBuiltin; }
  const ClassSet& allInterfaces() const { return m_allInterfaces; }
  const std::vector<ClassPtr>& usedTraits() const {
    return m_usedTraits;
  }
  const TraitAliasVec& traitAliases() const { return m_traitAliases; }
  const InitVec& pinitVec() const { return m_pinitVec; }
  const PropInitVec& declPropInit() const { return m_declPropInit; }

  // ObjectData attributes, to be set during Instance initialization.
  int getODAttrs() const { return m_ODAttrs; }

  // Interfaces this class declares in its "implements" clause.
  const std::vector<ClassPtr>& declInterfaces() const {
    return m_declInterfaces;
  }

  // These two fields are used by getClassInfo to locate all trait
  // methods present in methods(): indices [traitsBeginIdx, traitsEndIdx)
  Slot traitsBeginIdx() const { return m_traitsBeginIdx; }
  Slot traitsEndIdx() const   { return m_traitsEndIdx; }

  Func* lookupMethod(const StringData* methName) const {
    return m_methods.lookupDefault(methName, 0);
  }

  /*
   * We have a call site for an object method, which previously
   * invoked func, but this call has a different Class (*this).  See
   * if we can figure out the correct Func to call.
   */
  const Func* wouldCall(const Func* func) const;

  // Finds the base class defining the given method (NULL if none).
  // Note: for methods imported via traits, the base class is the one that
  // uses/imports the trait.
  Class* findMethodBaseClass(const StringData* methName);

  void getMethodNames(const Class* ctx, HphpArray* methods) const;

  // Returns true iff this class declared the given method.
  // For trait methods, the class declaring them is the one that uses/imports
  // the trait.
  bool declaredMethod(const Func* method);

  TypedValue* clsCnsGet(const StringData* clsCnsName) const;
  void initialize(HphpArray*& sPropData) const;
  void initialize() const;
  Class* getCached() const;
  void setCached();

  // Returns kInvalidSlot if we can't find this property.
  Slot lookupDeclProp(const StringData* propName) const {
    return m_declProperties.findSlot(propName);
  }

  Slot getDeclPropIndex(Class* ctx, const StringData* key,
                        bool& accessible) const;

  TypedValue* getSProp(Class* ctx, const StringData* sPropName,
                       bool& visible, bool& accessible) const;

  // Returns kInvalidSlot if we can't find this static property.
  Slot lookupSProp(const StringData* sPropName) const {
    return m_staticProperties.findSlot(sPropName);
  }

  TypedValue getStaticPropInitVal(const SProp& prop);

  void getClassInfo(ClassInfoVM* ci);

public: // Offset accessors for the translator
  size_t declPropOffset(Slot index) const {
    ASSERT(index >= 0);
    return sizeof(ObjectData) + m_builtinPropSize
      + index * sizeof(TypedValue);
  }
  static size_t preClassOff() { return offsetof(Class, m_preClass); }

public:
  static hphp_hash_map<const StringData*, const HhbcExtClassInfo*,
                       string_data_hash, string_data_isame> s_extClassHash;

private:
  struct TraitMethod {
    ClassPtr          m_trait;
    Func*             m_method;
    Attr              m_modifiers;
    TraitMethod(ClassPtr trait, Func* method, Attr modifiers) :
        m_trait(trait), m_method(method), m_modifiers(modifiers) { }
  };

  typedef IndexedStringMap<Func*,false> MethodMap;
  typedef IndexedStringMap<Const,true> ConstMap;
  typedef IndexedStringMap<Prop,true> PropMap;
  typedef IndexedStringMap<SProp,true> SPropMap;
  typedef std::list<TraitMethod> TraitMethodList;
  typedef hphp_hash_map<const StringData*, TraitMethodList, string_data_hash,
                        string_data_isame> MethodToTraitListMap;

private:
  HphpArray* initClsCnsData() const;
  PropInitVec* initProps() const;
  HphpArray* initSProps() const;

  void addImportTraitMethod(const TraitMethod &traitMethod,
                            const StringData  *methName);
  bool importTraitMethod(const TraitMethod&  traitMethod,
                         const StringData*   methName,
                         MethodMap::Builder& curMethodMap,
                         bool                failIsFatal);
  ClassPtr findSingleTraitWithMethod(const StringData* methName);
  void setImportTraitMethodModifiers(const StringData* methName,
                                     ClassPtr          traitCls,
                                     Attr              modifiers);
  bool importTraitMethods(MethodMap::Builder& curMethodMap, bool failIsFatal);
  void addTraitPropInitializers(bool staticProps);
  bool applyTraitRules(bool failIsFatal);
  bool applyTraitPrecRule(const PreClass::TraitPrecRule& rule,
                          bool                           failIsFatal);
  bool applyTraitAliasRule(const PreClass::TraitAliasRule& rule,
                           bool                            failIsFatal);
  bool importTraitProps(PropMap::Builder& curPropMap,
                        SPropMap::Builder& curSPropMap,
                        bool failIsFatal);
  bool importTraitInstanceProp(ClassPtr    trait,
                               Prop&       traitProp,
                               TypedValue& traitPropVal,
                               PropMap::Builder& curPropMap,
                               bool        failIsFatal);
  bool importTraitStaticProp(ClassPtr trait,
                             SProp&   traitProp,
                             PropMap::Builder& curPropMap,
                             SPropMap::Builder& curSPropMap,
                             bool     failIsFatal);
  void addTraitAlias(const StringData* traitName,
                     const StringData* origMethName,
                     const StringData* newMethName);

  bool checkInterfaceMethods(bool failIsFatal);
  bool methodOverrideOK(const Func* parentMethod, const Func* method,
                        bool FailIsFatal);

  static bool compatibleTraitPropInit(TypedValue& tv1, TypedValue& tv2);
  void removeSpareTraitAbstractMethods();

  bool setParent(bool failIsFatal);
  bool setSpecial(bool failIsFatal);
  bool setMethods(bool failIsFatal);
  bool setODAttributes(bool failIsFatal);
  bool setConstants(bool failIsFatal);
  bool setProperties(bool failIsFatal);
  bool setInitializers(bool failIsFatal);
  bool setInterfaces(bool failIsFatal);
  bool setClassVec(bool failIsFatal);
  bool setUsedTraits(bool failIsFatal);

private:
  PreClassPtr m_preClass;
  ClassPtr m_parent;
  std::vector<ClassPtr> m_declInterfaces; // interfaces this class declares in
                                          // its "implements" clause
  ClassSet m_allInterfaces; // all interfaces a non-abstract class deriving
                            // from this one (including itself) must implement
  std::vector<ClassPtr> m_usedTraits;
  TraitAliasVec m_traitAliases;

  // Methods.
  //
  // The m_methods map contains an entry for every method that can be
  // called in the context of this Class (but no private methods for
  // parent classes).
  MethodMap m_methods;

  Slot m_traitsBeginIdx;
  Slot m_traitsEndIdx;
  Func* m_ctor;
  Func* m_toString;

  // Vector of 86pinit() methods that need to be called to complete instance
  // property initialization, and a pointer to a 86sinit() method that needs to
  // be called to complete static property initialization (or NULL).  Such
  // initialization is triggered only once, the first time one of the following
  // happens:
  //
  // + An instance of this class is created.
  // + A static property of this class is accessed.
  InitVec m_pinitVec;
  InitVec m_sinitVec;
  const ClassInfo* m_clsInfo;
  bool m_needInitialization; // True if there are any __[ps]init() methods.
  bool m_needInstanceInit; // True if we should always call __init__
                           // on new instances of this class
  bool m_derivesFromBuiltin;
  int m_ODAttrs;

  int m_builtinPropSize;
  int m_declPropNumAccessible;
  unsigned m_classVecLen;
public: // used by Unit
  unsigned m_cachedOffset;
private:
  unsigned m_propDataCache;
  unsigned m_propSDataCache;

  BuiltinCtorFunction m_InstanceCtor;

  ConstMap m_constants;

  // Properties.
  //
  // Each Instance is created with enough trailing space to directly store the
  // vector of declared properties.  To look up a property by name and
  // determine whether it is declared, use m_declPropMap.  If the declared
  // property index is already known (as may be the case when executing via the
  // TC), property metadata in m_declPropInfo can be directly accessed.
  //
  // m_declPropInit is indexed by the Slot values from
  // m_declProperties, and contains initialization information.

  PropMap m_declProperties;
  PropInitVec m_declPropInit;

  SPropMap m_staticProperties;

  MethodToTraitListMap m_importMethToTraitMap;

public: // used in Unit
  Class* m_nextClass;
private:
  // Vector of Class pointers that encodes the inheritance hierarchy,
  // including this Class as the last element. This vector enables
  // fast type compatibility checks in translated code when accessing
  // declared properties.
  Class* m_classVec[1]; // Dynamically sized; must come last.
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
