/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/attr.h"
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
struct UnitEmitter;

///////////////////////////////////////////////////////////////////////////////

struct EHEntEmitter {
  EHEnt::Type m_type;
  bool m_itRef;
  Offset m_base;
  Offset m_past;
  int m_iterId;
  int m_parentIndex;
  Offset m_handler;
  Offset m_end;

  template<class SerDe> void serde(SerDe& sd);
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Bag of Func's fields used to emit Funcs.
 */
struct FuncEmitter {

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  struct ParamInfo : public Func::ParamInfo {
    ParamInfo()
      : byRef(false)
    {}

    template<class SerDe>
    void serde(SerDe& sd) {
      Func::ParamInfo* parent = this;
      parent->serde(sd);
      sd(byRef);
    }

    // Whether the parameter is passed by reference.  This field is absent from
    // Func::ParamInfo because we store it in a bitfield on Func.
    bool byRef;
  };

  typedef std::vector<ParamInfo> ParamInfoVec;
  typedef std::vector<Func::SVInfo> SVInfoVec;
  typedef std::vector<EHEntEmitter> EHEntVec;
  typedef std::vector<FPIEnt> FPIEntVec;


  /////////////////////////////////////////////////////////////////////////////
  // Initialization and execution.

  FuncEmitter(UnitEmitter& ue, int sn, Id id, const StringData* n);
  FuncEmitter(UnitEmitter& ue, int sn, const StringData* n,
              PreClassEmitter* pce);
  ~FuncEmitter();

  /*
   * Just set some fields when we start and stop emitting.
   */
  void init(int l1, int l2, Offset base_, Attr attrs_, bool top_,
            const StringData* docComment_);
  void finish(Offset past, bool load);

  /*
   * Commit this function to a repo.
   */
  void commit(RepoTxn& txn) const; // throws(RepoExc)

  /*
   * Instantiate a runtime Func*.
   */
  Func* create(Unit& unit, PreClass* preClass = nullptr) const;

  template<class SerDe> void serdeMetaData(SerDe&);


  /////////////////////////////////////////////////////////////////////////////
  // Metadata.

  /*
   * Get the associated Unit and PreClass emitters.
   */
  UnitEmitter& ue() const;
  PreClassEmitter* pce() const;

  /*
   * XXX: What are these for?
   */
  int sn() const;
  Id id() const;

  /*
   * XXX: Set the whatever these things are.
   */
  void setIds(int sn, Id id);


  /////////////////////////////////////////////////////////////////////////////
  // Locals, iterators, and parameters.

  /*
   * Count things.
   */
  Id numLocals() const;
  Id numIterators() const;
  Id numLiveIterators() const;
  Id numClsRefSlots() const;

  /*
   * Set things.
   */
  void setNumIterators(Id numIterators);
  void setNumLiveIterators(Id id);
  void setNumClsRefSlots(Id num);

  /*
   * Check existence of, look up, and allocate named locals.
   */
  bool hasVar(const StringData* name) const;
  Id lookupVarId(const StringData* name) const;
  void allocVarId(const StringData* name);

  /*
   * Allocate and free unnamed locals.
   */
  Id allocUnnamedLocal();
  void freeUnnamedLocal(Id id);

  /*
   * Allocate and free iterators.
   */
  Id allocIterator();
  void freeIterator(Id id);

  /*
   * Add a parameter and corresponding named local.
   */
  void appendParam(const StringData* name, const ParamInfo& info);

  /*
   * Get the local variable name -> id map.
   */
  const Func::NamedLocalsMap::Builder& localNameMap() const;


  /////////////////////////////////////////////////////////////////////////////
  // Unit tables.

  /*
   * Add entries to the EH and FPI tables, and return them by reference.
   */
  EHEntEmitter& addEHEnt();
  FPIEnt& addFPIEnt();

private:
  /*
   * Private table sort routines; called at finish()-time.
   */
  void sortEHTab();
  void sortFPITab(bool load);

public:
  /*
   * Declare that the EH table was created in sort-order and doesn't need to be
   * resorted at finish() time.
   */
  void setEHTabIsSorted();

  /////////////////////////////////////////////////////////////////////////////
  // Helper accessors.                                                  [const]

  /*
   * Is the function a pseudomain, a method, variadic (i.e., takes a `...'
   * parameter), or an HNI function with a native implementation?
   */
  bool isPseudoMain() const;
  bool isMethod() const;
  bool isVariadic() const;
  bool isVariadicByRef() const;

  /*
   * @returns: std::make_pair(line1, line2)
   */
  std::pair<int,int> getLocation() const;


  /////////////////////////////////////////////////////////////////////////////
  // Complex setters.
  //

  /*
   * Shorthand for setting `line1' and `line2' because typing is hard.
   */
  void setLocation(int l1, int l2);

  /*
   * Pulls native and system attributes out of the user attributes map.
   *
   * System attributes are returned by reference through `attrs_', and native
   * attributes are returned as an integer.
   */
  int parseNativeAttributes(Attr& attrs_) const;

  /*
   * Set some fields for builtin functions.
   */
  void setBuiltinFunc(Attr attrs_, Offset base_);


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  /*
   * Metadata.
   */
  UnitEmitter& m_ue;
  PreClassEmitter* m_pce;

  int m_sn;
  Id m_id;

public:
  /*
   * Func fields.
   */
  Offset base;
  Offset past;
  int line1;
  int line2;
  LowStringPtr name;
  bool top;
  Attr attrs;

  ParamInfoVec params;
  SVInfoVec staticVars;
  int maxStackCells;

  MaybeDataType hniReturnType;
  TypeConstraint retTypeConstraint;
  LowStringPtr retUserType;

  EHEntVec ehtab;
  FPIEntVec fpitab;

  union {
    uint8_t m_repoBoolBitset{0};
    struct {
      bool isMemoizeWrapper : 1;
      bool isClosureBody    : 1;
      bool isAsync          : 1;
      bool containsCalls    : 1;
      bool isNative         : 1;
      bool isGenerator      : 1;
      bool isPairGenerator  : 1;
    };
  };

  // These are not stored in the repo
  bool isMemoizeImpl{false};
  bool hasMemoizeSharedProp{false};

  LowStringPtr docComment;
  LowStringPtr originalFilename;

  UserAttributeMap userAttributes;

  StringData* memoizePropName;
  StringData* memoizeGuardPropName;
  int memoizeSharedPropIndex;
  RepoAuthType repoReturnType;
  RepoAuthType repoAwaitedReturnType;

  Id dynCallWrapperId{kInvalidId};

private:
  /*
   * FuncEmitter-managed state.
   */
  Func::NamedLocalsMap::Builder m_localNames;
  Id m_numLocals;
  int m_numUnnamedLocals;
  int m_activeUnnamedLocals;
  Id m_numIterators;
  Id m_nextFreeIterator;
  Id m_numClsRefSlots;
  bool m_ehTabSorted;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Proxy for converting in-repo function representations into FuncEmitters.
 */
struct FuncRepoProxy : public RepoProxy {
  friend struct Func;
  friend struct FuncEmitter;

  explicit FuncRepoProxy(Repo& repo);
  ~FuncRepoProxy();
  void createSchema(int repoId, RepoTxn& txn); // throws(RepoExc)

  struct InsertFuncStmt : public RepoProxy::Stmt {
    InsertFuncStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(const FuncEmitter& fe,
                RepoTxn& txn, int64_t unitSn, int funcSn, Id preClassId,
                const StringData* name, bool top); // throws(RepoExc)
  };

  struct GetFuncsStmt : public RepoProxy::Stmt {
    GetFuncsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue); // throws(RepoExc)
  };

  InsertFuncStmt insertFunc[RepoIdCount];
  GetFuncsStmt getFuncs[RepoIdCount];
};

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_FUNC_EMITTER_INL_H_
#include "hphp/runtime/vm/func-emitter-inl.h"
#undef incl_HPHP_VM_FUNC_EMITTER_INL_H_

#endif // incl_HPHP_VM_FUNC_EMITTER_H_
