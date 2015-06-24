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

#ifndef incl_HPHP_VM_UNIT_EMITTER_H_
#define incl_HPHP_VM_UNIT_EMITTER_H_

#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "hphp/parser/location.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/repo-helpers.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/util/functional.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/util/md5.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct FuncEmitter;
struct PreClassEmitter;
struct StringData;

///////////////////////////////////////////////////////////////////////////////

/*
 * Pre-runtime representation of Unit used to emit bytecode and instantiate
 * runtime Units.
 */
struct UnitEmitter {
  friend class UnitRepoProxy;

  /////////////////////////////////////////////////////////////////////////////
  // Initialization and execution.

  explicit UnitEmitter(const MD5& md5);
  ~UnitEmitter();

  /*
   * Commit this unit to a repo.
   */
  void commit(UnitOrigin unitOrigin);

  /*
   * Insert this unit in a repo as part of transaction `txn'.
   */
  bool insert(UnitOrigin unitOrigin, RepoTxn& txn);

  /*
   * Instatiate a runtime Unit*.
   */
  std::unique_ptr<Unit> create();

  template<class SerDe> void serdeMetaData(SerDe&);


  /////////////////////////////////////////////////////////////////////////////
  // Basic data.

  /*
   * The MD5 hash of the Unit.
   */
  const MD5& md5() const;

  /*
   * Bytecode pointer and current emit position.
   */
  const unsigned char* bc() const;
  Offset bcPos() const;

  /*
   * Set the bytecode pointer by allocating a copy of `bc' with size `bclen'.
   *
   * Not safe to call with m_bc as the argument because we free our current
   * bytecode stream before allocating a copy of `bc'.
   */
  void setBc(const unsigned char* bc, size_t bclen);


  /////////////////////////////////////////////////////////////////////////////
  // Litstrs and Arrays.

  /*
   * Look up a static string or array by ID.
   */
  const StringData* lookupLitstr(Id id) const;
  const ArrayData* lookupArray(Id id) const;

  /*
   * Merge a literal string into either the global LitstrTable or the table for
   * the Unit.
   */
  Id mergeLitstr(const StringData* litstr);

  /*
   * Merge a literal string into the table for the Unit.
   */
  Id mergeUnitLitstr(const StringData* litstr);

  /*
   * Merge a scalar array into the Unit.
   */
  Id mergeArray(const ArrayData* a);
  Id mergeArray(const ArrayData* a, const ArrayData::ScalarArrayKey& key);


  /////////////////////////////////////////////////////////////////////////////
  // FuncEmitters.

  /*
   * The Unit's pseudomain emitter.
   */
  FuncEmitter* getMain();

  /*
   * Const reference to all of the Unit's FuncEmitters.
   */
  const std::vector<FuncEmitter*>& fevec() const;

  /*
   * Create the pseudomain emitter for the Unit.
   *
   * @requires: fevec().size() == 0
   */
  void initMain(int line1, int line2);

  /*
   * Create a trivial (i.e., Int 1; RetC) pseudomain emitter for the Unit.
   *
   * @requires: fevec().size() == 0
   */
  void addTrivialPseudoMain();

  /*
   * Create a new FuncEmitter and add it to the FE vector.
   */
  FuncEmitter* newFuncEmitter(const StringData* name);

  /*
   * Create a new FuncEmitter for the method given by `name' and `pce'.
   *
   * Does /not/ add it to the FE vector.
   */
  FuncEmitter* newMethodEmitter(const StringData* name, PreClassEmitter* pce);

  /*
   * Add `fe' to the FE vector.
   */
  void appendTopEmitter(FuncEmitter* fe);

  /*
   * Finish adding a FuncEmitter to the Unit and record its bytecode range.
   *
   * This can only be done once for each FuncEmitter, after it is added to the
   * FE vector.  None of the bytecode ranges of FuncEmitters added to the Unit
   * are allowed to overlap.
   *
   * Takes logical ownership of `fe'.
   */
  void recordFunction(FuncEmitter* fe);

  /*
   * Create a new function for `fe'.
   *
   * This should only be called from fe->create(), and just constructs a new
   * Func* and records it as emitted from `fe'.
   */
  Func* newFunc(const FuncEmitter* fe, Unit& unit, const StringData* name,
                Attr attrs, int numParams);


  /////////////////////////////////////////////////////////////////////////////
  // PreClassEmitters.

  /*
   * Number of PreClassEmitters in the Unit.
   */
  size_t numPreClasses() const;

  /*
   * The PreClassEmitter for `preClassId'.
   */
  const PreClassEmitter* pce(Id preClassId) const;
  PreClassEmitter* pce(Id preClassId);

  /*
   * Create a new PreClassEmitter and add it to the PCE vector.
   *
   * @see: PreClass::Hoistable
   */
  PreClassEmitter* newPreClassEmitter(const StringData* name,
                                      PreClass::Hoistable hoistable);


  /////////////////////////////////////////////////////////////////////////////
  // Type aliases.

  /*
   * Const reference to all of the Unit's type aliases.
   */
  const std::vector<TypeAlias>& typeAliases() const;

  /*
   * Add a new type alias to the Unit.
   */
  Id addTypeAlias(const TypeAlias& td);


  /////////////////////////////////////////////////////////////////////////////
  // Source locations.

  /*
   * Return a copy of the SrcLocTable for the Unit, if it has one; otherwise,
   * return an empty table.
   */
  SourceLocTable createSourceLocTable() const;

  /*
   * Does this Unit contain full source location information?
   *
   * Generally, UnitEmitters loaded from a production repo will have a
   * LineTable only instead of a full SourceLocTable.
   */
  bool hasSourceLocInfo() const;

  /*
   * Const reference to the Unit's LineTable.
   */
  const LineTable& lineTable() const;

  /*
   * Record source location information for the last chunk of bytecode added to
   * this UnitEmitter.
   *
   * Adjacent regions associated with the same source line will be collapsed as
   * this is created.
   */
  void recordSourceLocation(const Location::Range& sLoc, Offset start);


  /////////////////////////////////////////////////////////////////////////////
  // Mergeables.
  //
  // See unit.h for documentation of Unit merging.

  /*
   * Append a PreClassEmitter to the UnitEmitter's list of mergeables.
   */
  void pushMergeableClass(PreClassEmitter* e);

  /*
   * Add a Unit include to the UnitEmitter's list of mergeables.
   *
   * The `push' flavor first merges the litstr `unitName', and then appends the
   * mergeable object reference to the list.  The `insert' flavor inserts the
   * mergeable (kind, id) at `ix' in the list.
   */
  void pushMergeableInclude(Unit::MergeKind kind, const StringData* unitName);
  void insertMergeableInclude(int ix, Unit::MergeKind kind, Id id);

  /*
   * Add a constant definition to the UnitEmitter's list of mergeables.
   *
   * The `push' flavor first merges the litstr `unitName', and then appends the
   * mergeable object reference to the list.  The `insert' flavor inserts the
   * mergeable (kind, id) at `ix' in the list.
   *
   * The mergeable value is appended or inserted likewise.
   */
  void pushMergeableDef(Unit::MergeKind kind, const StringData* name,
                        const TypedValue& tv);
  void insertMergeableDef(int ix, Unit::MergeKind kind, Id id,
                          const TypedValue& tv);


  /////////////////////////////////////////////////////////////////////////////
  // Bytecode emit.
  //
  // These methods emit values to bc() at bcPos() and then updates bcPos(),
  // realloc-ing the bytecode region if necessary.

  void emitOp(Op op, int64_t pos = -1);
  void emitByte(unsigned char n, int64_t pos = -1);

  void emitInt32(int n, int64_t pos = -1);
  void emitInt64(int64_t n, int64_t pos = -1);
  void emitDouble(double n, int64_t pos = -1);

  template<typename T>
  void emitIVA(T n);


  /////////////////////////////////////////////////////////////////////////////
  // Other methods.

  /*
   * Is this a Unit for a systemlib?
   */
  bool isASystemLib() const;

private:
  /*
   * Bytecode emit implementation.
   */
  template<class T>
  void emitImpl(T n, int64_t pos);


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  // Initial bytecode size.
  static const size_t BCMaxInit = 4096;

public:
  int m_repoId{-1};
  int64_t m_sn{-1};
  const StringData* m_filepath{nullptr};

  bool m_mergeOnly{false};
  bool m_isHHFile{false};
  bool m_returnSeen{false};
  int m_preloadPriority{0};
  TypedValue m_mainReturn;

private:
  MD5 m_md5;

  unsigned char* m_bc;
  size_t m_bclen;
  size_t m_bcmax;

  int m_nextFuncSn;

  /*
   * Litstr tables.
   */
  hphp_hash_map<const StringData*, Id,
                string_data_hash, string_data_same> m_litstr2id;
  std::vector<const StringData*> m_litstrs;

  /*
   * Scalar array tables.
   */
  hphp_hash_map<ArrayData::ScalarArrayKey, Id,
                ArrayData::ScalarHash> m_array2id;
  std::vector<ArrayData*> m_arrays;

  /*
   * Type alias table.
   */
  std::vector<TypeAlias> m_typeAliases;

  /*
   * FuncEmitter tables.
   */
  std::vector<FuncEmitter*> m_fes;
  hphp_hash_map<const FuncEmitter*, const Func*,
                pointer_hash<FuncEmitter>> m_fMap;

  /*
   * PreClassEmitter table.
   */
  std::vector<PreClassEmitter*> m_pceVec;

  /*
   * Hoistability tables.
   */
  bool m_allClassesHoistable;
  hphp_hash_set<const StringData*,
                string_data_hash,
                string_data_isame> m_hoistablePreClassSet;
  std::list<Id> m_hoistablePceIdList;

  /*
   * Mergeables tables.
   */
  std::vector<std::pair<Unit::MergeKind, Id>> m_mergeableStmts;
  std::vector<std::pair<Id, TypedValue>> m_mergeableValues;

  /*
   * Source location tables.
   *
   * Each entry encodes an open-closed range of bytecode offsets.
   *
   * The m_sourceLocTab is keyed by the start of each half-open range.  This is
   * to allow appending new bytecode offsets that are part of the same range to
   * coalesce.
   *
   * The m_feTab and m_lineTable are keyed by the past-the-end offset.  This is
   * the format we'll want them in when we go to create a Unit.
   */
  std::vector<std::pair<Offset,SourceLoc>> m_sourceLocTab;
  std::vector<std::pair<Offset,const FuncEmitter*>> m_feTab;
  LineTable m_lineTable;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Proxy for converting in-repo unit representations into UnitEmitters.
 */
struct UnitRepoProxy : public RepoProxy {
  friend class Unit;
  friend class UnitEmitter;

  explicit UnitRepoProxy(Repo& repo);
  ~UnitRepoProxy();
  void createSchema(int repoId, RepoTxn& txn);
  std::unique_ptr<Unit> load(const std::string& name, const MD5& md5);
  std::unique_ptr<UnitEmitter> loadEmitter(const std::string& name,
                                           const MD5& md5);

  void insertUnitLineTable(int repoId, RepoTxn& txn, int64_t unitSn,
                           LineTable& lineTable);
  void getUnitLineTable(int repoId, int64_t unitSn, LineTable& lineTable);

  struct InsertUnitStmt : public RepoProxy::Stmt {
    InsertUnitStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(const UnitEmitter& ue,
                RepoTxn& txn,
                int64_t& unitSn,
                const MD5& md5,
                const unsigned char* bc,
                size_t bclen);
  };
  struct GetUnitStmt : public RepoProxy::Stmt {
    GetUnitStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(UnitEmitter& ue, const MD5& md5);
  };
  struct InsertUnitLitstrStmt : public RepoProxy::Stmt {
    InsertUnitLitstrStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn, Id litstrId,
                const StringData* litstr);
  };
  struct GetUnitLitstrsStmt : public RepoProxy::Stmt {
    GetUnitLitstrsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
  struct InsertUnitArrayStmt : public RepoProxy::Stmt {
    InsertUnitArrayStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn, Id arrayId,
                const std::string& array);
  };
  struct GetUnitArraysStmt : public RepoProxy::Stmt {
    GetUnitArraysStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
  struct InsertUnitMergeableStmt : public RepoProxy::Stmt {
    InsertUnitMergeableStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn,
                int ix, Unit::MergeKind kind,
                Id id, TypedValue* value);
  };
  struct GetUnitMergeablesStmt : public RepoProxy::Stmt {
    GetUnitMergeablesStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
  struct InsertUnitSourceLocStmt : public RepoProxy::Stmt {
    InsertUnitSourceLocStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn, Offset pastOffset, int line0,
                int char0, int line1, int char1);
  };
  struct GetSourceLocTabStmt : public RepoProxy::Stmt {
    GetSourceLocTabStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64_t unitSn, SourceLocTable& sourceLocTab);
  };

#define URP_IOP(o) URP_OP(Insert##o, insert##o)
#define URP_GOP(o) URP_OP(Get##o, get##o)
#define URP_OPS \
  URP_IOP(Unit) \
  URP_GOP(Unit) \
  URP_IOP(UnitLitstr) \
  URP_GOP(UnitLitstrs) \
  URP_IOP(UnitArray) \
  URP_GOP(UnitArrays) \
  URP_IOP(UnitMergeable) \
  URP_GOP(UnitMergeables) \
  URP_IOP(UnitSourceLoc) \
  URP_GOP(SourceLocTab)

#define URP_OP(c, o) \
  c##Stmt o[RepoIdCount];
  URP_OPS
#undef URP_OP

private:
  bool loadHelper(UnitEmitter& ue, const std::string&, const MD5&);
};

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_UNIT_EMITTER_INL_H_
#include "hphp/runtime/vm/unit-emitter-inl.h"
#undef incl_HPHP_VM_UNIT_EMITTER_INL_H_

#endif // incl_HPHP_VM_UNIT_EMITTER_H_
