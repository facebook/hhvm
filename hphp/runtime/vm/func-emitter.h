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

#ifndef incl_HPHP_VM_FUNC_EMITTER_H_
#define incl_HPHP_VM_FUNC_EMITTER_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/user-attributes.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/repo-helpers.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit.h"

#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct PreClass;
struct StringData;

struct PreClassEmitter;
struct UnitClassEmitter;

///////////////////////////////////////////////////////////////////////////////

/*
 * Builder pattern Func-creation class.
 */
struct FuncEmitter {
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
  typedef std::vector<Func::SVInfo> SVInfoVec;
  typedef std::vector<EHEnt> EHEntVec;
  typedef std::vector<FPIEnt> FPIEntVec;

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
  void setEhTabIsSorted();

  Id newLocal();
  void appendParam(const StringData* name, const ParamInfo& info);
  void setParamFuncletOff(Id id, Offset off) {
    m_params[id].funcletOff = off;
  }
  void allocVarId(const StringData* name);
  Id lookupVarId(const StringData* name) const;
  bool hasVar(const StringData* name) const;
  Id numParams() const { return m_params.size(); }

  /*
   * Return type constraints will eventually be runtime-enforced
   * return types, but for now are unused.
   */
  void setReturnTypeConstraint(const TypeConstraint retTypeConstraint) {
    m_retTypeConstraint = retTypeConstraint;
  }
  const TypeConstraint& returnTypeConstraint() const {
    return m_retTypeConstraint;
  }

  /*
   * Return "user types" are string-format specifications of return
   * types only used for reflection purposes.
   */
  void setReturnUserType(const StringData* retUserType) {
    m_retUserType = retUserType;
  }
  const StringData* returnUserType() const {
    return m_retUserType;
  }

  /*
   * Returns whether this FuncEmitter represents an HNI function with
   * a native implementation.
   */
  bool isHNINative() const { return getReturnType() != KindOfInvalid; }

  Id numIterators() const { return m_numIterators; }
  void setNumIterators(Id numIterators);
  Id allocIterator();
  void freeIterator(Id id);
  Id numLiveIterators() { return m_nextFreeIterator; }
  void setNumLiveIterators(Id id) { m_nextFreeIterator = id; }

  Id allocUnnamedLocal();
  void freeUnnamedLocal(Id id);
  Id numLocals() const { return m_numLocals; }
  void setNumLocals(Id numLocals);
  const Func::NamedLocalsMap::Builder& localNameMap() const {
    return m_localNames;
  }

  void setMaxStackCells(int cells) { m_maxStackCells = cells; }
  void addStaticVar(Func::SVInfo svInfo);
  const SVInfoVec& svInfo() const { return m_staticVars; }

  UnitEmitter& ue() const { return m_ue; }
  PreClassEmitter* pce() const { return m_pce; }
  void setIds(int sn, Id id) {
    m_sn = sn;
    m_id = id;
  }
  int sn() const { return m_sn; }
  Id id() const {
    assert(m_pce == nullptr);
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
  bool isVariadic() const {
    return m_params.size() && m_params[(m_params.size() - 1)].isVariadic();
  }

  void setTop(bool top) { m_top = top; }
  bool top() const { return m_top; }

  bool isPseudoMain() const { return m_name->empty(); }

  void setIsClosureBody(bool isClosureBody) { m_isClosureBody = isClosureBody; }
  bool isClosureBody() const { return m_isClosureBody; }

  void setIsGenerator(bool isGenerator) { m_isGenerator = isGenerator; }
  bool isGenerator() const { return m_isGenerator; }

  bool isMethod() const {
    return !isPseudoMain() && (bool)pce();
  }

  void setIsPairGenerator(bool b) { m_isPairGenerator = b; }
  bool isPairGenerator() const { return m_isPairGenerator; }

  void setContainsCalls() { m_containsCalls = true; }

  void setIsAsync(bool isAsync) { m_isAsync = isAsync; }
  bool isAsync() const { return m_isAsync; }

  void addUserAttribute(const StringData* name, TypedValue tv);
  void setUserAttributes(UserAttributeMap map) {
    m_userAttributes = std::move(map);
  }
  const UserAttributeMap& getUserAttributes() const {
    return m_userAttributes;
  }
  bool hasUserAttribute(const StringData* name) const {
    auto it = m_userAttributes.find(name);
    return it != m_userAttributes.end();
  }
  int parseNativeAttributes(Attr &attrs) const;

  void commit(RepoTxn& txn) const;
  Func* create(Unit& unit, PreClass* preClass = nullptr) const;

  void setBuiltinFunc(const ClassInfo::MethodInfo* info,
      BuiltinFunction bif, BuiltinFunction nif, Offset base);
  void setBuiltinFunc(BuiltinFunction bif, BuiltinFunction nif,
                      Attr attrs, Offset base);

  void setOriginalFilename(const StringData* name) {
    m_originalFilename = name;
  }
  const StringData* originalFilename() const { return m_originalFilename; }

  /*
   * Return types used for HNI functions with a native C++
   * implementation.
   */
  void setReturnType(DataType dt) { m_returnType = dt; }
  DataType getReturnType() const { return m_returnType; }

  void setDocComment(const char *dc) {
    m_docComment = makeStaticString(dc);
  }
  const StringData* getDocComment() const {
    return m_docComment;
  }

  void setLocation(int l1, int l2) {
    m_line1 = l1;
    m_line2 = l2;
  }

  std::pair<int,int> getLocation() const {
    return std::make_pair(m_line1, m_line2);
  }

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
  LowStringPtr m_name;

  ParamInfoVec m_params;
  Func::NamedLocalsMap::Builder m_localNames;
  Id m_numLocals;
  int m_numUnnamedLocals;
  int m_activeUnnamedLocals;
  Id m_numIterators;
  Id m_nextFreeIterator;
  int m_maxStackCells;
  SVInfoVec m_staticVars;

  TypeConstraint m_retTypeConstraint;
  LowStringPtr m_retUserType;

  EHEntVec m_ehtab;
  bool m_ehTabSorted;
  FPIEntVec m_fpitab;

  Attr m_attrs;
  DataType m_returnType;
  bool m_top;
  LowStringPtr m_docComment;
  bool m_isClosureBody;
  bool m_isAsync;
  bool m_isGenerator;
  bool m_isPairGenerator;
  bool m_containsCalls;

  UserAttributeMap m_userAttributes;

  const ClassInfo::MethodInfo* m_info;
  BuiltinFunction m_builtinFuncPtr;
  BuiltinFunction m_nativeFuncPtr;

  LowStringPtr m_originalFilename;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Proxy for converting in-repo function representations into FuncEmitters.
 */
class FuncRepoProxy : public RepoProxy {
  friend class Func;
  friend class FuncEmitter;
public:
  explicit FuncRepoProxy(Repo& repo);
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
                RepoTxn& txn, int64_t unitSn, int funcSn, Id preClassId,
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

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_VM_FUNC_EMITTER_H_
