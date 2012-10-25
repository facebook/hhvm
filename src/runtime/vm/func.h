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

#ifndef incl_VM_FUNC_H_
#define incl_VM_FUNC_H_

#include "runtime/vm/bytecode.h"
#include "runtime/vm/type_constraint.h"
#include "runtime/vm/repo_helpers.h"
#include "runtime/vm/indexed_string_map.h"
#include "runtime/base/intercept.h"

namespace HPHP {
namespace VM {

static const int kNumFixedPrologues = 6;

typedef TypedValue*(*BuiltinFunction)(ActRec* ar);

// Function.
struct Func {
  friend class FuncEmitter;

  typedef hphp_hash_map<const StringData*, TypedValue, string_data_hash,
                        string_data_isame> UserAttributeMap;

  struct ParamInfo { // Parameter default value info.
    // construct a dummy ParamInfo
    ParamInfo() : m_funcletOff(InvalidAbsoluteOffset), m_phpCode(NULL) {
      TV_WRITE_UNINIT(&m_defVal);
    }

    template<class SerDe>
    void serde(SerDe& sd) {
      const StringData* tcName = m_typeConstraint.typeName();
      bool tcNullable          = m_typeConstraint.nullable();

      sd(m_funcletOff)
        (m_defVal)
        (m_phpCode)
        (tcName)
        (tcNullable)
        (m_userAttributes)
        ;

      if (SerDe::deserializing) {
        setTypeConstraint(TypeConstraint(tcName,
                                         tcNullable));
      }
    }

    void setFuncletOff(Offset funcletOff) { m_funcletOff = funcletOff; }
    Offset funcletOff() const { return m_funcletOff; }

    bool hasDefaultValue() const {
      return m_funcletOff != InvalidAbsoluteOffset;
    }
    bool hasNonNullDefaultValue() const {
      return hasDefaultValue() && m_defVal.m_type != KindOfNull;
    }
    bool hasScalarDefaultValue() const {
      return hasDefaultValue() && m_defVal.m_type != KindOfUninit;
    }
    void setDefaultValue(const TypedValue& defVal) { m_defVal = defVal; }
    const TypedValue& defaultValue() const { return m_defVal; }

    void setPhpCode(const StringData* phpCode) { m_phpCode = phpCode; }
    const StringData* phpCode() const { return m_phpCode; }

    void setTypeConstraint(const TypeConstraint& tc) { m_typeConstraint = tc; }
    const TypeConstraint& typeConstraint() const { return m_typeConstraint; }

    void addUserAttribute(const StringData* name, TypedValue tv) {
      m_userAttributes[name] = tv;
    }
    void setUserAttributes(const Func::UserAttributeMap& uaMap) {
      m_userAttributes = uaMap;
    }
    const Func::UserAttributeMap& userAttributes() const {
      return m_userAttributes;
    }

  private:
    Offset m_funcletOff; // If no default: InvalidAbsoluteOffset.
    TypedValue m_defVal; // Set to uninit null if there is no default value
                         // or if there is a non-scalar default value.
    const StringData* m_phpCode; // eval'able PHP code.
    TypeConstraint m_typeConstraint;

    Func::UserAttributeMap m_userAttributes;
  };
  struct SVInfo { // Static variable info.
    const StringData* name;
    const StringData* phpCode; // eval'able PHP or NULL if no default.

    template<class SerDe> void serde(SerDe& sd) { sd(name)(phpCode); }
  };

  typedef std::vector<ParamInfo> ParamInfoVec;
  typedef FixedVector<SVInfo> SVInfoVec;
  typedef FixedVector<EHEnt> EHEntVec;
  typedef FixedVector<FPIEnt> FPIEntVec;

  typedef uint32_t FuncId;
  static const FuncId InvalidId = -1LL;

  Func(Unit& unit, Id id, int line1, int line2, Offset base,
      Offset past, const StringData* name, Attr attrs, bool top,
      const StringData* docComment, int numParams);
  Func(Unit& unit, PreClass* preClass, int line1,
      int line2, Offset base, Offset past,
      const StringData* name, Attr attrs, bool top,
      const StringData* docComment, int numParams);
  ~Func();
  static void destroy(Func* func);

  Func* clone() const;

  void validate() const {
#ifdef DEBUG
    ASSERT(this && m_magic == kMagic);
#endif
  }

  FuncId getFuncId() const { return m_funcId; }
  void setFuncId(FuncId id);
  void setNewFuncId();

  void rename(const StringData* name);
  int numSlotsInFrame() const {
    return shared()->m_numLocals + shared()->m_numIterators * kNumIterCells;
  }
  Id lookupVarId(const StringData* name) const;
  bool checkIterScope(Offset o, Id iterId) const;
  const EHEnt* findEH(Offset o) const;
  Offset findFaultPCFromEH(Offset o) const;
  const FPIEnt* findFPI(Offset o) const;
  const FPIEnt* findPrecedingFPI(Offset o) const;

  void parametersCompat(const PreClass* preClass, const Func* imeth) const;

  // This can be thought of as "if I look up this Func's name while in fromUnit,
  // will I always get this Func back?" This is important for the translator: if
  // this condition holds, it allows for some translation-time optimizations by
  // making assumptions about where function calls will go.
  bool isNameBindingImmutable(const Unit* fromUnit) const;

  void setMaxStackCells(int cells) { m_maxStackCells = cells; }
  int maxStackCells() const {
    // All functions have to return something, which pushes at least 1 cell
    ASSERT(m_maxStackCells > 0);
    return m_maxStackCells;
  }

  bool byRef(int32 arg) const;
  bool mustBeRef(int32 arg) const;
  void prettyPrint(std::ostream& out) const;

  bool isPseudoMain() const { return m_name->empty(); }
  bool isBuiltin() const { return (bool)info(); }
  bool isMethod() const { return (bool)preClass(); }
  bool isTraitMethod() const {
    PreClass* pcls = preClass();
    return pcls && (pcls->attrs() & AttrTrait);
  }
  bool isNonClosureMethod() const {
    return isMethod() && !isClosureBody();
  }
  bool isStatic() const { return bool(m_attrs & AttrStatic); }
  bool isAbstract() const { return bool(m_attrs & AttrAbstract); }
  bool isUnique() const { return bool(m_attrs & AttrUnique); }
  bool isDestructor() const {
    return !strcmp(m_name->data(), "__destruct");
  }
  bool isPersistent() const { return m_attrs & AttrPersistent; }
  static bool isMagicCallMethodName(const StringData* name) {
    return name->isame(s___call) || name->isame(s___callStatic);
  }
  bool isMagicCallMethod() const {
    return m_name->isame(s___call);
  }
  bool isMagicCallStaticMethod() const {
    return m_name->isame(s___callStatic);
  }
  bool isMagic() const {
    return isMagicCallMethod() || isMagicCallStaticMethod();
  }
  static bool isSpecial(const StringData* methName) {
    return strncmp("86", methName->data(), 2) == 0;
  }
  bool isNoInjection() const { return bool(m_attrs & AttrNoInjection); }

  HphpArray* getStaticLocals() const;
  void getFuncInfo(ClassInfo::MethodInfo* mi) const;

  Unit* unit() const { return m_unit; }
  PreClass* preClass() const { return shared()->m_preClass; }
  Class* cls() const { return m_cls; }
  void setCls(Class* cls) {
    m_cls = cls;
    setFullName();
  }
  void setClsAndName(Class* cls, const StringData* name) {
    m_cls = cls;
    m_name = name;
    setFullName();
  }
  Class* baseCls() const { return m_baseCls; }
  void setBaseCls(Class* baseCls) { m_baseCls = baseCls; }
  bool hasPrivateAncestor() const { return m_hasPrivateAncestor; }
  void setHasPrivateAncestor(bool b) { m_hasPrivateAncestor = b; }
  Id id() const {
    ASSERT(preClass() == NULL);
    return shared()->m_id;
  }
  Offset base() const { return shared()->m_base; }
  const inline Opcode* getEntry() const {
    ASSERT(!isAbstract());
    return m_unit->entry() + shared()->m_base;
  }
  Offset past() const { return shared()->m_past; }
  int line1() const { return shared()->m_line1; }
  int line2() const { return shared()->m_line2; }
  const SVInfoVec& staticVars() const { return shared()->m_staticVars; }
  const StringData* name() const {
    ASSERT(m_name != NULL);
    return m_name;
  }
  CStrRef nameRef() const {
    ASSERT(m_name != NULL);
    return *(String*)(&m_name);
  }
  const StringData* fullName() const {
    if (m_fullName == NULL) return m_name;
    return m_fullName;
  }
  CStrRef fullNameRef() const {
    ASSERT(m_fullName != NULL);
    return *(String*)(&m_fullName);
  }
  // Assembly linkage.
  static size_t fullNameOffset() {
    return offsetof(Func, m_fullName);
  }
  static size_t sharedOffset() {
    return offsetof(Func, m_shared);
  }
  static size_t sharedBaseOffset() {
    return offsetof(SharedData, m_base);
  }
  static void enableIntercept();
  char &maybeIntercepted() const { return m_maybeIntercepted; }
  bool checkInterceptable() const {
    if (!m_maybeIntercepted) return false;
    return m_maybeIntercepted > 0 ||
    get_intercept_handler(fullNameRef(), &m_maybeIntercepted);
  }
  int numParams() const { return m_numParams; }
  const ParamInfoVec& params() const { return shared()->m_params; }
  int numLocals() const { return shared()->m_numLocals; }

  const StringData* const* localNames() const {
    return shared()->m_localNames.accessList();
  }
  Id numNamedLocals() const { return shared()->m_localNames.size(); }

  // Returns the name of a local variable, or null if this varid is an
  // unnamed local.
  const StringData* localVarName(Id id) const {
    ASSERT(id >= 0);
    return id < numNamedLocals() ? shared()->m_localNames[id] : 0;
  }

  int numIterators() const { return shared()->m_numIterators; }
  const EHEntVec& ehtab() const { return shared()->m_ehtab; }
  const FPIEntVec& fpitab() const { return shared()->m_fpitab; }
  Attr attrs() const { return m_attrs; }
  void setAttrs(Attr attrs) { m_attrs = attrs; }
  bool top() const { return shared()->m_top; }
  const StringData* docComment() const { return shared()->m_docComment; }
  bool isClosureBody() const { return shared()->m_isClosureBody; }
  bool isGenerator() const { return shared()->m_isGenerator; }
  bool isGeneratorFromClosure() const {
    return shared()->m_isGeneratorFromClosure;
  }
  const Func* getGeneratorBody(const StringData* name) const;
  bool hasStaticLocals() const { return !shared()->m_staticVars.empty(); }
  int numStaticLocals() const { return shared()->m_staticVars.size(); }
  const ClassInfo::MethodInfo* info() const { return shared()->m_info; }
  bool isIgnoreRedefinition() const {
    return shared()->m_info &&
    (shared()->m_info->attribute & ClassInfo::IgnoreRedefinition);
  }
  const BuiltinFunction& builtinFuncPtr() const {
    return shared()->m_builtinFuncPtr;
  }
  const UserAttributeMap& userAttributes() const {
    return shared()->m_userAttributes;
  }

  static void* allocFuncMem(const StringData* name, int numParams);

  void setPrologue(int index, unsigned char* tca) {
    m_prologueTable[index] = tca;
  }
  void setFuncBody(unsigned char* fb) {
    m_funcBody = fb;
  }
  unsigned char* getFuncBody() const {
    return m_funcBody;
  }
  unsigned char* getPrologue(int index) const {
    return m_prologueTable[index];
  }
  int numPrologues() const {
    return getMaxNumPrologues(m_numParams);
  }
  static int getMaxNumPrologues(int numParams) {
    // maximum number of prologues is numParams+2. The extra 2 are for
    // the case where the number of actual params equals numParams and
    // the case where the number of actual params is greater than
    // numParams.
    return numParams + 2;
  }
  void resetPrologues() {
    // Useful when killing code; forget what we've learned about the contents
    // of the translation cache.
    initPrologues(m_numParams);
  }

  const NamedEntity* getNamedEntity() const {
    ASSERT(!m_shared->m_preClass);
    return m_namedEntity;
  }
  Slot methodSlot() const {
    ASSERT(m_cls);
    return m_methodSlot;
  }
  void setMethodSlot(Slot s) {
    ASSERT(m_cls);
    m_methodSlot = s;
  }
  Func** getCachedAddr();
  Func* getCached() { return *getCachedAddr(); }
  void setCached();
  unsigned getCachedOffset() const { return m_cachedOffset; }

public: // Offset accessors for the translator.
#define X(f) static size_t f##Off() { return offsetof(Func, m_##f); }
  X(attrs);
  X(unit);
  X(cls);
  X(numParams);
  X(refBitVec);
  X(fullName);
  X(prologueTable);
  X(maybeIntercepted);
  X(maxStackCells);
#undef X

private:
  typedef IndexedStringMap<const StringData*,true,Id> NamedLocalsMap;

  struct SharedData : public Countable {
    PreClass* m_preClass;
    Id m_id;
    Offset m_base;
    Id m_numLocals;
    Id m_numIterators;
    Offset m_past;
    int m_line1;
    int m_line2;
    const ClassInfo::MethodInfo* m_info; // For builtins.
    uint64_t* m_refBitVec;
    BuiltinFunction m_builtinFuncPtr;
    ParamInfoVec m_params; // m_params[i] corresponds to parameter i.
    NamedLocalsMap m_localNames; // includes parameter names
    SVInfoVec m_staticVars;
    EHEntVec m_ehtab;
    FPIEntVec m_fpitab;
    const StringData* m_docComment;
    bool m_top : 1; // Defined at top level.
    bool m_isClosureBody : 1;
    bool m_isGenerator : 1;
    bool m_isGeneratorFromClosure : 1;
    UserAttributeMap m_userAttributes;
    SharedData(PreClass* preClass, Id id, Offset base,
        Offset past, int line1, int line2, bool top,
        const StringData* docComment);
    ~SharedData();
    void release();
  };
  typedef SmartPtr<SharedData> SharedDataPtr;

  static const int kBitsPerQword = 64;
  static const StringData* s___call;
  static const StringData* s___callStatic;
  static const int kMagic = 0xba5eba11;

private:
  void setFullName();
  void init(int numParams);
  void initPrologues(int numParams);
  void appendParam(bool ref, const ParamInfo& info);
  void allocVarId(const StringData* name);
  const SharedData* shared() const { return m_shared.get(); }
  SharedData* shared() { return m_shared.get(); }

private:
  static bool s_interceptsEnabled;

private:
  Unit* m_unit;
  Class* m_cls;      // The Class that provided this method implementation
  Class* m_baseCls;  // The first Class in the inheritance hierarchy that
                     // declared this method; note that this may be an abstract
                     // class that did not provide an implementation
  const StringData* m_name;
  const StringData* m_fullName;
  SharedDataPtr m_shared;
  union {
    const NamedEntity* m_namedEntity;
    Slot m_methodSlot;
  };
  uint64_t* m_refBitVec;
public: // used by Unit
  unsigned m_cachedOffset;
private:
#ifdef DEBUG
  int m_magic; // For asserts only.
#endif
  int m_maxStackCells;
  int m_numParams;
  Attr m_attrs;
  FuncId m_funcId;
  bool m_hasPrivateAncestor : 1; // This flag indicates if any of this
                                 // Class's ancestors provide a
                                 // "private" implementation for this
                                 // method
  // TODO(#1114385) intercept should work via invalidation.
  mutable char m_maybeIntercepted; // -1, 0, or 1.  Accessed atomically.
  unsigned char* volatile m_funcBody;
  // This must be the last field declared in this structure
  // and the Func class should not be inherited from.
  unsigned char* volatile m_prologueTable[kNumFixedPrologues];
};

class FuncEmitter {
public:
  typedef std::vector<Func::SVInfo> SVInfoVec;
  typedef std::vector<EHEnt> EHEntVec;
  typedef std::vector<FPIEnt> FPIEntVec;

  struct ParamInfo: public Func::ParamInfo {
    ParamInfo() : m_ref(false) {}

    void setRef(bool ref) { m_ref = ref; }
    bool ref() const { return m_ref; }

    template<class SerDe> void serde(SerDe& sd) {
      Func::ParamInfo* parent = this;
      parent->serde(sd);
      sd(m_ref);
    }

  private:
    bool m_ref; // True if parameter is passed by reference.
  };
  typedef std::vector<ParamInfo> ParamInfoVec;

  FuncEmitter(UnitEmitter& ue, int sn, Id id, const StringData* n);
  FuncEmitter(UnitEmitter& ue, int sn, const StringData* n,
      PreClassEmitter* pce);
  ~FuncEmitter();

  void init(int line1, int line2, Offset base, Attr attrs, bool top,
      const StringData* docComment);
  void finish(Offset past, bool load);

  template<class SerDe> void serdeMetaData(SerDe&);

  EHEnt& addEHEnt();
  FPIEnt& addFPIEnt();

  Id newLocal();
  void appendParam(const StringData* name, const ParamInfo& info);
  void setParamFuncletOff(Id id, Offset off) {
    m_params[id].setFuncletOff(off);
  }
  void allocVarId(const StringData* name);
  Id lookupVarId(const StringData* name) const;
  Id numParams() const { return m_params.size(); }

  Id allocIterator();
  void freeIterator(Id id);
  void setNumIterators(Id numIterators);
  Id numIterators() const { return m_numIterators; }

  Id allocUnnamedLocal();
  void freeUnnamedLocal(Id id);
  Id numLocals() const { return m_numLocals; }
  void setNumLocals(Id numLocals);

  void setMaxStackCells(int cells) { m_maxStackCells = cells; }
  void addStaticVar(Func::SVInfo svInfo);

  UnitEmitter& ue() const { return m_ue; }
  PreClassEmitter* pce() const { return m_pce; }
  void setIds(int sn, Id id) {
    m_sn = sn;
    m_id = id;
  }
  int sn() const { return m_sn; }
  Id id() const {
    ASSERT(m_pce == NULL);
    return m_id;
  }
  Offset base() const { return m_base; }
  Offset past() const { return m_past; }
  const StringData* name() const { return m_name; }
  const ParamInfoVec& params() const { return m_params; }
  const EHEntVec& ehtab() const { return m_ehtab; }
  EHEntVec& ehtab() { return m_ehtab; }
  const FPIEntVec& fpitab() const { return m_fpitab; }

  void setAttrs(Attr attrs) { m_attrs = attrs; }
  Attr attrs() const { return m_attrs; }

  void setTop(bool top) { m_top = top; }
  bool top() { return m_top; }

  bool isPseudoMain() const { return m_name->empty(); }

  void setIsClosureBody(bool isClosureBody) { m_isClosureBody = isClosureBody; }
  bool isClosureBody() const { return m_isClosureBody; }

  void setIsGenerator(bool isGenerator) { m_isGenerator = isGenerator; }
  bool isGenerator() const { return m_isGenerator; }

  void setIsGeneratorFromClosure(bool b) { m_isGeneratorFromClosure = b; }
  bool isGeneratorFromClosure() const { return m_isGeneratorFromClosure; }

  void addUserAttribute(const StringData* name, TypedValue tv);

  void commit(RepoTxn& txn) const;
  Func* create(Unit& unit, PreClass* preClass = NULL) const;

  void setBuiltinFunc(const ClassInfo::MethodInfo* info,
      BuiltinFunction funcPtr, Offset base);

private:
  void sortEHTab();
  void sortFPITab(bool load);

  UnitEmitter& m_ue;
  PreClassEmitter* m_pce;
  int m_sn;
  Id m_id;
  Offset m_base;
  Offset m_past;
  int m_line1;
  int m_line2;
  const StringData* m_name;

  ParamInfoVec m_params;
  Func::NamedLocalsMap::Builder m_localNames;
  Id m_numLocals;
  int m_numUnnamedLocals;
  int m_activeUnnamedLocals;
  Id m_numIterators;
  Id m_nextFreeIterator;
  int m_maxStackCells;
  SVInfoVec m_staticVars;

  EHEntVec m_ehtab;
  FPIEntVec m_fpitab;

  Attr m_attrs;
  bool m_top;
  const StringData* m_docComment;
  bool m_isClosureBody;
  bool m_isGenerator;
  bool m_isGeneratorFromClosure;

  Func::UserAttributeMap m_userAttributes;

  const ClassInfo::MethodInfo* m_info;
  BuiltinFunction m_builtinFuncPtr;
};

class FuncRepoProxy : public RepoProxy {
  friend class Func;
  friend class FuncEmitter;
public:
  FuncRepoProxy(Repo& repo);
  ~FuncRepoProxy();
  void createSchema(int repoId, RepoTxn& txn);

#define FRP_IOP(o) FRP_OP(Insert##o, insert##o)
#define FRP_GOP(o) FRP_OP(Get##o, get##o)
#define FRP_OPS \
  FRP_IOP(Func) \
  FRP_GOP(Funcs)
  class InsertFuncStmt : public RepoProxy::Stmt {
  public:
    InsertFuncStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(const FuncEmitter& fe,
                RepoTxn& txn, int64 unitSn, int funcSn, Id preClassId,
                const StringData* name, bool top);
  };
  class GetFuncsStmt : public RepoProxy::Stmt {
  public:
    GetFuncsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
#define FRP_OP(c, o) \
 public: \
  c##Stmt& o(int repoId) { return *m_##o[repoId]; } \
 private: \
  c##Stmt m_##o##Local; \
  c##Stmt m_##o##Central; \
  c##Stmt* m_##o[RepoIdCount];
FRP_OPS
#undef FRP_OP
};

} }

#endif
