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
#include <util/fixed_vector.h>
#include <util/range.h>
#include <runtime/vm/fixed_string_map.h>
#include <runtime/vm/indexed_string_map.h>

namespace HPHP {

// Forward declaration.
class ClassInfo;
class ClassInfoVM;
class ObjectData;

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
  friend class PreClassEmitter;
  friend class Peephole;

 public:
  enum Hoistable {
    NotHoistable,
    Mergeable,
    MaybeHoistable,
    AlwaysHoistable
  };

  struct Prop {
    Prop() {}
    Prop(PreClass* preClass, const StringData* n, Attr attrs,
         const StringData* docComment, const TypedValue& val);

    void prettyPrint(std::ostream& out) const;

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

  struct Const {
    Const() {}
    Const(PreClass* preClass, const StringData* n, const TypedValue& val,
          const StringData* phpCode);

    void prettyPrint(std::ostream& out) const;

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
    TraitPrecRule()
      : m_methodName(0)
      , m_selectedTraitName(0)
    {}

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

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_methodName)(m_selectedTraitName)(m_otherTraitNames);
    }

   private:
    const StringData*  m_methodName;
    const StringData*  m_selectedTraitName;
    TraitNameSet       m_otherTraitNames;
  };

  class TraitAliasRule {
   public:
    TraitAliasRule()
      : m_traitName(0)
      , m_origMethodName(0)
      , m_newMethodName(0)
      , m_modifiers(AttrNone)
    {}

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

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_traitName)(m_origMethodName)(m_newMethodName)(m_modifiers);
    }

   private:
    const StringData* m_traitName;
    const StringData* m_origMethodName;
    const StringData* m_newMethodName;
    Attr              m_modifiers;
  };

  typedef FixedVector<const StringData*> InterfaceVec;
  typedef FixedVector<const StringData*> UsedTraitVec;
  typedef FixedVector<TraitPrecRule> TraitPrecRuleVec;
  typedef FixedVector<TraitAliasRule> TraitAliasRuleVec;
  typedef hphp_hash_map<const StringData*, TypedValue, string_data_hash,
                        string_data_isame> UserAttributeMap;

  PreClass(Unit* unit, int line1, int line2, Offset o, const StringData* n,
           Attr attrs, const StringData* parent, const StringData* docComment,
           Id id, Hoistable hoistable);
  ~PreClass();
  void atomicRelease();

  Unit* unit() const { return m_unit; }
  int line1() const { return m_line1; }
  int line2() const { return m_line2; }
  Offset getOffset() const { return m_offset; }
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
  bool isPersistent() const { return m_attrs & AttrPersistent; }

  /*
   *  Funcs, Consts, and Props all behave similarly. Define raw accessors
   *  foo() and numFoos() for people munging by hand, and ranges.
   *    methods(); numMethods(); FuncRange allMethods();
   *    consts(); numConsts(); ConstRange allConsts();
   *    properties; numProperties(); PropRange allProperties();
   */

#define DEF_ACCESSORS(Type, TypeName, fields, Fields)                   \
  Type const* fields() const { return m_##fields.accessList(); }        \
  Type*       mutable##Fields() { return m_##fields.mutableAccessList(); } \
  size_t num##Fields()  const { return m_##fields.size(); }             \
  typedef IterRange<Type const*> TypeName##Range;                       \
  TypeName##Range all##Fields() const {                                 \
    return TypeName##Range(fields(), fields() + m_##fields.size() - 1); \
  }

  DEF_ACCESSORS(Func*, Func, methods, Methods)
  DEF_ACCESSORS(Const, Const, constants, Constants)
  DEF_ACCESSORS(Prop, Prop, properties, Properties)

#undef DEF_ACCESSORS

  bool hasMethod(const StringData* methName) const {
    return m_methods.contains(methName);
  }

  bool hasProp(const StringData* propName) const {
    return m_properties.contains(propName);
  }

  Func* lookupMethod(const StringData* methName) const {
    Func* f = m_methods.lookupDefault(methName, 0);
    ASSERT(f != NULL);
    return f;
  }

  const Prop* lookupProp(const StringData* propName) const {
    Slot s = m_properties.findIndex(propName);
    ASSERT(s != kInvalidSlot);
    return &m_properties[s];
  }

  BuiltinCtorFunction instanceCtor() { return m_InstanceCtor; }
  int builtinPropSize() { return m_builtinPropSize; }

  void prettyPrint(std::ostream& out) const;

  const NamedEntity* namedEntity() const { return m_namedEntity; }

private:
  typedef IndexedStringMap<Func*,false,Slot> MethodMap;
  typedef IndexedStringMap<Prop,true,Slot> PropMap;
  typedef IndexedStringMap<Const,true,Slot> ConstMap;

private:
  Unit* m_unit;
  const NamedEntity* m_namedEntity;
  int m_line1;
  int m_line2;
  Offset m_offset;
  Id m_id;
  int m_builtinPropSize;
  Attr m_attrs;
  Hoistable m_hoistable;
  const StringData* m_name;
  const StringData* m_parent;
  const StringData* m_docComment;
  BuiltinCtorFunction m_InstanceCtor;
  InterfaceVec m_interfaces;
  UsedTraitVec m_usedTraits;
  TraitPrecRuleVec m_traitPrecRules;
  TraitAliasRuleVec m_traitAliasRules;
  UserAttributeMap m_userAttributes;
  MethodMap m_methods;
  PropMap m_properties;
  ConstMap m_constants;
};
// It is possible for multiple Class'es to refer to the same PreClass, and we
// need to make sure that the PreClass lives for as long as an associated Class
// still exists (even if the associated Unit has been unloaded).  Therefore,
// use AtomicSmartPtr's to enforce this invariant.
typedef AtomicSmartPtr<PreClass> PreClassPtr;
class PreClassEmitter {
 public:
  typedef std::vector<FuncEmitter*> MethodVec;

  class Prop {
   public:
    Prop()
      : m_name(0)
      , m_mangledName(0)
      , m_attrs(AttrNone)
      , m_docComment(0)
    {}

    Prop(const PreClassEmitter* pce, const StringData* n, Attr attrs,
         const StringData* docComment, TypedValue* val);
    ~Prop();

    const StringData* name() const { return m_name; }
    const StringData* mangledName() const { return m_mangledName; }
    Attr attrs() const { return m_attrs; }
    const StringData* docComment() const { return m_docComment; }
    const TypedValue& val() const { return m_val; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_name)
        (m_mangledName)
        (m_attrs)
        (m_docComment)
        (m_val)
        ;
    }

   private:
    const StringData* m_name;
    const StringData* m_mangledName;
    Attr m_attrs;
    const StringData* m_docComment;
    TypedValue m_val;
  };

  class Const {
   public:
    Const()
      : m_name(0)
      , m_phpCode(0)
    {}
    Const(const StringData* n, TypedValue* val, const StringData* phpCode)
      : m_name(n), m_phpCode(phpCode) {
      memcpy(&m_val, val, sizeof(TypedValue));
    }
    ~Const() {}

    const StringData* name() const { return m_name; }
    const TypedValue& val() const { return m_val; }
    const StringData* phpCode() const { return m_phpCode; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_name)(m_val)(m_phpCode);
    }

   private:
    const StringData* m_name;
    TypedValue m_val;
    const StringData* m_phpCode;
  };

  PreClassEmitter(UnitEmitter& ue, Id id, const StringData* n,
                  PreClass::Hoistable hoistable);
  ~PreClassEmitter();

  void init(int line1, int line2, Offset offset, Attr attrs,
            const StringData* parent, const StringData* docComment);

  UnitEmitter& ue() const { return m_ue; }
  const StringData* name() const { return m_name; }
  Attr attrs() const { return m_attrs; }
  void setHoistable(PreClass::Hoistable h) { m_hoistable = h; }
  Id id() const { return m_id; }
  const MethodVec& methods() const { return m_methods; }

  void addInterface(const StringData* n);
  bool addMethod(FuncEmitter* method);
  bool addProperty(const StringData* n, Attr attrs,
                   const StringData* docComment, TypedValue* val);
  const Prop& lookupProp(const StringData* propName) const;
  bool addConstant(const StringData* n, TypedValue* val,
                   const StringData* phpCode);
  void addUsedTrait(const StringData* traitName);
  void addTraitPrecRule(const PreClass::TraitPrecRule &rule);
  void addTraitAliasRule(const PreClass::TraitAliasRule &rule);
  void addUserAttribute(const StringData* name, TypedValue tv);
  void commit(RepoTxn& txn) const;

  void setBuiltinClassInfo(const ClassInfo* info,
                           BuiltinCtorFunction ctorFunc,
                           int sz);

  PreClass* create(Unit& unit) const;

  template<class SerDe> void serdeMetaData(SerDe&);

 private:
  typedef IndexedStringMap<Prop,true,Slot> PropMap;
  typedef IndexedStringMap<Const,true,Slot> ConstMap;
  typedef hphp_hash_map<const StringData*, FuncEmitter*, string_data_hash,
                        string_data_isame> MethodMap;

  UnitEmitter& m_ue;
  int m_line1;
  int m_line2;
  Offset m_offset;
  const StringData* m_name;
  Attr m_attrs;
  const StringData* m_parent;
  const StringData* m_docComment;
  Id m_id;
  PreClass::Hoistable m_hoistable;
  BuiltinCtorFunction m_InstanceCtor;
  int m_builtinPropSize;

  std::vector<const StringData*> m_interfaces;
  std::vector<const StringData*> m_usedTraits;
  std::vector<PreClass::TraitPrecRule> m_traitPrecRules;
  std::vector<PreClass::TraitAliasRule> m_traitAliasRules;
  PreClass::UserAttributeMap m_userAttributes;
  MethodVec m_methods;
  MethodMap m_methodMap;
  PropMap::Builder m_propMap;
  ConstMap::Builder m_constMap;
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
  PCRP_GOP(PreClasses)
  class InsertPreClassStmt : public RepoProxy::Stmt {
   public:
    InsertPreClassStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(const PreClassEmitter& pce, RepoTxn& txn, int64 unitSn,
                Id preClassId, const StringData* name,
                PreClass::Hoistable hoistable);
  };
  class GetPreClassesStmt : public RepoProxy::Stmt {
   public:
    GetPreClassesStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
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
};

typedef AtomicSmartPtr<Class> ClassPtr;
class Class : public AtomicCountable {
public:
  friend class ExecutionContext;
  friend class HPHP::ObjectData;
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
    static PropInitVec* allocInRequestArena(const PropInitVec& src);

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
  static ClassPtr newClass(PreClass* preClass, Class* parent);
  Class(PreClass* preClass, Class* parent, unsigned classVecLen);
  void atomicRelease();

  static size_t sizeForNClasses(unsigned nClasses) {
    return offsetof(Class, m_classVec) + (sizeof(Class*) * nClasses);
  }

  Avail avail(Class *&parent, bool tryAutoload = false) const;
  Class* classof(const PreClass* preClass) const;
  bool classof(const Class* cls) const {
    if (UNLIKELY((attrs() & (AttrInterface | AttrTrait)) ||
                 (cls->attrs() & (AttrInterface | AttrTrait)))) {
      return (classof(cls->m_preClass.get()) == cls);
    }
    if (m_classVecLen >= cls->m_classVecLen) {
      return (m_classVec[cls->m_classVecLen-1] == cls);
    }
    return false;
  }
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
  typedef IterRange<Func*const*> MethodRange;
  MethodRange methodRange() const {
    return MethodRange(methods(), methods() + numMethods());
  }
  typedef IterRange<Func**> MutableMethodRange;
  MutableMethodRange mutableMethodRange() {
    return MutableMethodRange(m_methods.mutableAccessList(),
                              m_methods.mutableAccessList() + m_methods.size());
  }
  const SProp* staticProperties() const
    { return m_staticProperties.accessList(); }
  size_t numStaticProperties() const { return m_staticProperties.size(); }
  const Prop* declProperties() const { return m_declProperties.accessList(); }
  size_t numDeclProperties() const { return m_declProperties.size(); }
  const Const* constants() const { return m_constants.accessList(); }
  size_t numConstants() const { return m_constants.size(); }
  Attr attrs() const {
    ASSERT(Attr(m_attrCopy) == m_preClass->attrs());
    return Attr(m_attrCopy);
  }
  bool verifyPersistent() const;
  const Func* getCtor() const { return m_ctor; }
  const Func* getDtor() const { return m_dtor; }
  const Func* getToString() const { return m_toString; }
  const PreClass* preClass() const { return m_preClass.get(); }
  const ClassInfo* clsInfo() const { return m_clsInfo; }
  const PropInitVec* getPropData() const;
  bool needInitialization() const { return m_needInitialization; }
  bool callsCustomInstanceInit() const { return m_callsCustomInstanceInit; }
  const ClassSet& allInterfaces() const { return m_allInterfaces; }
  const std::vector<ClassPtr>& usedTraits() const {
    return m_usedTraits;
  }
  const TraitAliasVec& traitAliases() const { return m_traitAliases; }
  const InitVec& pinitVec() const { return m_pinitVec; }
  const PropInitVec& declPropInit() const { return m_declPropInit; }

  // ObjectData attributes, to be set during Instance initialization.
  int getODAttrs() const { return m_ODAttrs; }

  int builtinPropSize() { return m_builtinPropSize; }

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

  bool isPersistent() const { return m_attrCopy & AttrPersistent; }

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
  DataType clsCnsType(const StringData* clsCnsName) const;
  void initialize() const;
  Class* getCached() const;
  void setCached();

  // Returns kInvalidSlot if we can't find this property.
  Slot lookupDeclProp(const StringData* propName) const {
    return m_declProperties.findIndex(propName);
  }

  Slot getDeclPropIndex(Class* ctx, const StringData* key,
                        bool& accessible) const;

  TypedValue* getSProp(Class* ctx, const StringData* sPropName,
                       bool& visible, bool& accessible) const;

  static bool IsPropAccessible(const Prop& prop, Class* ctx);

  // Returns kInvalidSlot if we can't find this static property.
  Slot lookupSProp(const StringData* sPropName) const {
    return m_staticProperties.findIndex(sPropName);
  }

  TypedValue getStaticPropInitVal(const SProp& prop);

  void getClassInfo(ClassInfoVM* ci);

  size_t declPropOffset(Slot index) const {
    ASSERT(index >= 0);
    return sizeof(ObjectData) + m_builtinPropSize
      + index * sizeof(TypedValue);
  }
  static size_t preClassOff() { return offsetof(Class, m_preClass); }
  static Offset getMethodsOffset() { return offsetof(Class, m_methods); }
  typedef IndexedStringMap<Func*,false,Slot> MethodMap;

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

  typedef IndexedStringMap<Const,true,Slot> ConstMap;
  typedef IndexedStringMap<Prop,true,Slot> PropMap;
  typedef IndexedStringMap<SProp,true,Slot> SPropMap;
  typedef std::list<TraitMethod> TraitMethodList;
  typedef hphp_hash_map<const StringData*, TraitMethodList, string_data_hash,
                        string_data_isame> MethodToTraitListMap;

  void initialize(TypedValue*& sPropData) const;
  HphpArray* initClsCnsData() const;
  PropInitVec* initProps() const;
  TypedValue* initSProps() const;
  void setPropData(PropInitVec* propData) const;
  void setSPropData(TypedValue* sPropData) const;
  TypedValue* getSPropData() const;
  TypedValue* cnsNameToTV(const StringData* name, Slot& slot) const;

  void addImportTraitMethod(const TraitMethod &traitMethod,
                            const StringData  *methName);
  void importTraitMethod(const TraitMethod&  traitMethod,
                         const StringData*   methName,
                         MethodMap::Builder& curMethodMap);
  ClassPtr findSingleTraitWithMethod(const StringData* methName);
  void setImportTraitMethodModifiers(const StringData* methName,
                                     ClassPtr          traitCls,
                                     Attr              modifiers);
  void importTraitMethods(MethodMap::Builder& curMethodMap);
  void addTraitPropInitializers(bool staticProps);
  void applyTraitRules();
  void applyTraitPrecRule(const PreClass::TraitPrecRule& rule);
  void applyTraitAliasRule(const PreClass::TraitAliasRule& rule);
  void importTraitProps(PropMap::Builder& curPropMap,
                        SPropMap::Builder& curSPropMap);
  void importTraitInstanceProp(ClassPtr    trait,
                               Prop&       traitProp,
                               TypedValue& traitPropVal,
                               PropMap::Builder& curPropMap);
  void importTraitStaticProp(ClassPtr trait,
                             SProp&   traitProp,
                             PropMap::Builder& curPropMap,
                             SPropMap::Builder& curSPropMap);
  void addTraitAlias(const StringData* traitName,
                     const StringData* origMethName,
                     const StringData* newMethName);

  void checkInterfaceMethods();
  void methodOverrideCheck(const Func* parentMethod, const Func* method);

  static bool compatibleTraitPropInit(TypedValue& tv1, TypedValue& tv2);
  void removeSpareTraitAbstractMethods();

  void setParent();
  void setSpecial();
  void setMethods();
  void setODAttributes();
  void setConstants();
  void setProperties();
  void setInitializers();
  void setInterfaces();
  void setClassVec();
  void setUsedTraits();

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
  Func* m_dtor;
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
  unsigned m_needInitialization : 1;      // any __[ps]init() methods?
  unsigned m_callsCustomInstanceInit : 1; // should we always call __init__
                                          // on new instances?
  unsigned m_attrCopy : 30;               // cache of m_preClass->attrs().
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
