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

#ifndef incl_HPHP_VM_UNIT_EMITTER_H_
#define incl_HPHP_VM_UNIT_EMITTER_H_

#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "hphp/parser/location.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/vm/constant.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/repo-helpers.h"
#include "hphp/runtime/vm/repo-status.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/util/functional.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/sha1.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct FuncEmitter;
struct PreClassEmitter;
struct RecordEmitter;
struct StringData;

namespace Native {
struct FuncTable;
}

/*
 * Report capacity of RepoAuthoritative mode bytecode arena.
 *
 * Returns 0 if !RuntimeOption::RepoAuthoritative.
 */
size_t hhbc_arena_capacity();

/*
 * Whether we need to keep the extended line table (for debugging, or
 * dumping to hhas).
 */
bool needs_extended_line_table();

enum class SymbolRef : uint8_t {
  Include,
  Class,
  Function,
  Constant
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Pre-runtime representation of Unit used to emit bytecode and instantiate
 * runtime Units.
 */
struct UnitEmitter {
  friend struct UnitRepoProxy;

  /////////////////////////////////////////////////////////////////////////////
  // Initialization and execution.

  explicit UnitEmitter(const SHA1& sha1,
                       const SHA1& bcSha1,
                       const Native::FuncTable&,
                       bool useGlobalIds);
  UnitEmitter(UnitEmitter&&) = delete;
  ~UnitEmitter();

  void setSha1(const SHA1& sha1) { m_sha1 = sha1; }
  /*
   * Commit this unit to a repo.
   */
  void commit(UnitOrigin unitOrigin);

  /*
   * Insert this unit in a repo as part of transaction `txn'.
   */
  RepoStatus insert(UnitOrigin unitOrigin, RepoTxn& txn);

  /*
   * Instatiate a runtime Unit*.
   */
  std::unique_ptr<Unit> create(bool saveLineTable = false) const;

  template<class SerDe> void serdeMetaData(SerDe&);

  /*
   * Run the verifier on this unit.
   */
  bool check(bool verbose) const;


  /////////////////////////////////////////////////////////////////////////////
  // Basic data.

  /*
   * The SHA1 hash of the source for Unit.
   */
  const SHA1& sha1() const;

  /*
   * The SHA1 hash of the bytecode for Unit.
   */
  const SHA1& bcSha1() const;

  /*
   * Bytecode pointer and current emit position.
   */
  const unsigned char* bc() const;
  Offset bcPos() const;
  Offset offsetOf(const unsigned char* pc) const;

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
   * Look up a static string or array/arraytype by ID.
   */
  const StringData* lookupLitstr(Id id) const;
  const ArrayData* lookupArray(Id id) const;
  const RepoAuthType::Array* lookupArrayType(Id id) const;

  Id numArrays() const { return m_arrays.size(); }
  Id numLitstrs() const { return m_litstrs.size(); }

  bool useGlobalIds() const { return m_useGlobalIds; }
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

  /*
   * Clear and rebuild the array type table from the builder.
   */
   void repopulateArrayTypeTable(const ArrayTypeTable::Builder&);

  /////////////////////////////////////////////////////////////////////////////
  // FuncEmitters.

  /*
   * The Unit's pseudomain emitter.
   */
  FuncEmitter* getMain() const;

  /*
   * Const reference to all of the Unit's FuncEmitters.
   */
  auto const& fevec() const;

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
  void appendTopEmitter(std::unique_ptr<FuncEmitter>&& fe);

  /*
   * Create a new function for `fe'.
   *
   * This should only be called from fe->create(), and just constructs a new
   * Func* and adds it to unit.m_funcTable if required.
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
   * The id for the pre-class named clsName, or -1 if
   * there is no such pre-class
   */
  Id pceId(folly::StringPiece clsName);

  /*
   * Add a PreClassEmitter to the hoistability tracking data structures.
   *
   * @see: PreClass::Hoistable
   */
  void addPreClassEmitter(PreClassEmitter* pce);

  /*
   * Create a new PreClassEmitter and add it to all the PCE data structures.
   *
   * @see: PreClass::Hoistable
   */
  PreClassEmitter* newPreClassEmitter(const std::string& name,
                                      PreClass::Hoistable hoistable);
  /*
   * Create a new PreClassEmitter without adding it to the hoistability
   * tracking data structures.
   * It should be added later with addPreClassEmitter.
   */
  PreClassEmitter* newBarePreClassEmitter(const std::string& name,
                                          PreClass::Hoistable hoistable);

  RecordEmitter* newRecordEmitter(const std::string& name);

  /////////////////////////////////////////////////////////////////////////////
  // RecordEmitters.

  /*
   * Number of RecordEmitters in the Unit.
   */
  size_t numRecords() const;

  /*
   * The RecordEmitter for `recordId'.
   */
  const RecordEmitter* re(Id recordId) const;
  RecordEmitter* re(Id recordId);

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
  // Constants.

  /*
   * Const reference to all of the Unit's type aliases.
   */
  const std::vector<Constant>& constants() const;

  /*
   * Add a new constant to the Unit.
   */
  Id addConstant(const Constant& c);

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
   * Add a TypeAlias to the UnitEmitter's list of mergeables.
   */
  void pushMergeableId(Unit::MergeKind kind, const Id id);
  void insertMergeableId(Unit::MergeKind kind, int ix, const Id id);

  /*
   * Add a Record to the UnitEmitter's list of mergeables.
   */
  void pushMergeableRecord(const Id id);
  void insertMergeableRecord(int ix, const Id id);

  /////////////////////////////////////////////////////////////////////////////
  // Bytecode emit.
  //
  // These methods emit values to bc() at bcPos() (or pos, if given) and then
  // update bcPos(), realloc-ing the bytecode region if necessary.

  void emitOp(Op op);
  void emitByte(unsigned char n, int64_t pos = -1);

  void emitInt16(uint16_t n, int64_t pos = -1);
  void emitInt32(int n, int64_t pos = -1);
  void emitInt64(int64_t n, int64_t pos = -1);
  void emitDouble(double n, int64_t pos = -1);

  void emitIVA(bool) = delete;
  template<typename T> void emitIVA(T n);

  void emitNamedLocal(NamedLocal loc);


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
  bool m_ICE{false}; // internal compiler error
  bool m_useGlobalIds{0};
  TypedValue m_mainReturn;
  UserAttributeMap m_metaData;
  UserAttributeMap m_fileAttributes;
  CompactVector<
    std::pair<SymbolRef, CompactVector<std::string>>> m_symbol_refs;
  /*
   * name=>NativeFuncInfo for native funcs in this unit
   */
  const Native::FuncTable& m_nativeFuncs;

private:
  SHA1 m_sha1;
  SHA1 m_bcSha1;

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
  hphp_hash_map<const ArrayData*, Id> m_array2id;
  std::vector<const ArrayData*> m_arrays;

  /*
   * Unit local array type table.
   */
  ArrayTypeTable m_arrayTypeTable;

  /*
   * Type alias table.
   */
  std::vector<TypeAlias> m_typeAliases;

  /*
   * Constants table.
   */
  std::vector<Constant> m_constants;

  /*
   * FuncEmitter tables.
   */
  std::vector<std::unique_ptr<FuncEmitter> > m_fes;

  /*
   * PreClassEmitter table.
   */
  std::vector<PreClassEmitter*> m_pceVec;

  /*
   * RecordEmitter table.
   */
  std::vector<RecordEmitter*> m_reVec;

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

  /*
   * Source location tables.
   *
   * Each entry encodes an open-closed range of bytecode offsets.
   *
   * The m_sourceLocTab is keyed by the start of each half-open range.  This is
   * to allow appending new bytecode offsets that are part of the same range to
   * coalesce.
   *
   * The m_lineTable is keyed by the past-the-end offset.  This is the
   * format we'll want it in when we go to create a Unit.
   */
  std::vector<std::pair<Offset,SourceLoc>> m_sourceLocTab;
  LineTable m_lineTable;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Proxy for converting in-repo unit representations into UnitEmitters.
 */
struct UnitRepoProxy : public RepoProxy {
  friend struct Unit;
  friend struct UnitEmitter;

  explicit UnitRepoProxy(Repo& repo);
  ~UnitRepoProxy();
  void createSchema(int repoId, RepoTxn& txn); // throws(RepoExc)
  std::unique_ptr<Unit> load(const folly::StringPiece name, const SHA1& sha1,
                             const Native::FuncTable&);
  std::unique_ptr<UnitEmitter> loadEmitter(const folly::StringPiece name,
                                           const SHA1& sha1,
                                           const Native::FuncTable&);

  struct InsertUnitLineTableStmt : public RepoProxy::Stmt {
    InsertUnitLineTableStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn,
                int64_t unitSn,
                LineTable& lineTable); // throws(RepoExc)
  };
  struct GetUnitLineTableStmt : public RepoProxy::Stmt {
    GetUnitLineTableStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(int64_t unitSn, LineTable& lineTable);
  };

  struct InsertUnitTypeAliasStmt : public RepoProxy::Stmt {
    InsertUnitTypeAliasStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(const UnitEmitter& ue,
                RepoTxn& txn,
                int64_t unitSn,
                Id typeAliasId,
                const TypeAlias& typeAlias); // throws(RepoExc)
  };
  struct GetUnitTypeAliasesStmt : public RepoProxy::Stmt {
    GetUnitTypeAliasesStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };

  struct InsertUnitConstantStmt : public RepoProxy::Stmt {
    InsertUnitConstantStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(const UnitEmitter& ue,
                RepoTxn& txn,
                int64_t unitSn,
                Id constantId,
                const Constant& constant); // throws(RepoExc)
  };
  struct GetUnitConstantsStmt : public RepoProxy::Stmt {
    GetUnitConstantsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };

  struct InsertUnitStmt : public RepoProxy::Stmt {
    InsertUnitStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(const UnitEmitter& ue,
                RepoTxn& txn,
                int64_t& unitSn,
                const SHA1& sha1,
                const unsigned char* bc,
                size_t bclen); // throws(RepoExc)
  };
  struct GetUnitStmt : public RepoProxy::Stmt {
    GetUnitStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    RepoStatus get(UnitEmitter& ue, const SHA1& sha1);
  };
  struct InsertUnitLitstrStmt : public RepoProxy::Stmt {
    InsertUnitLitstrStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn, Id litstrId,
                const StringData* litstr); // throws(RepoExc)
  };
  struct GetUnitLitstrsStmt : public RepoProxy::Stmt {
    GetUnitLitstrsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue); // throws(RepoExc)
  };
  struct InsertUnitArrayTypeTableStmt : public RepoProxy::Stmt {
    InsertUnitArrayTypeTableStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn,
                const UnitEmitter& ue); // throws(RepoExc)
  };
  struct GetUnitArrayTypeTableStmt : public RepoProxy::Stmt {
    GetUnitArrayTypeTableStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue); // throws(RepoExc)
  };
  struct InsertUnitArrayStmt : public RepoProxy::Stmt {
    InsertUnitArrayStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn, Id arrayId,
                const std::string& array); // throws(RepoExc)
  };
  struct GetUnitArraysStmt : public RepoProxy::Stmt {
    GetUnitArraysStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue); // throws(RepoExc)
  };
  struct InsertUnitMergeableStmt : public RepoProxy::Stmt {
    InsertUnitMergeableStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn,
                int ix, Unit::MergeKind kind,
                Id id); // throws(RepoExc)
  };
  struct GetUnitMergeablesStmt : public RepoProxy::Stmt {
    GetUnitMergeablesStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue); // throws(RepoExc)
  };
  struct InsertUnitSourceLocStmt : public RepoProxy::Stmt {
    InsertUnitSourceLocStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn, Offset pastOffset, int line0,
                int char0, int line1, int char1); // throws(RepoExc)
  };
  struct GetSourceLocTabStmt : public RepoProxy::Stmt {
    GetSourceLocTabStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    RepoStatus get(int64_t unitSn, SourceLocTable& sourceLocTab);
  };

#define URP_IOP(o) URP_OP(Insert##o, insert##o)
#define URP_GOP(o) URP_OP(Get##o, get##o)
#define URP_OPS \
  URP_IOP(Unit) \
  URP_GOP(Unit) \
  URP_IOP(UnitLineTable) \
  URP_GOP(UnitLineTable) \
  URP_IOP(UnitTypeAlias) \
  URP_GOP(UnitTypeAliases) \
  URP_IOP(UnitLitstr) \
  URP_GOP(UnitLitstrs) \
  URP_IOP(UnitArrayTypeTable) \
  URP_GOP(UnitArrayTypeTable) \
  URP_IOP(UnitArray) \
  URP_GOP(UnitArrays) \
  URP_IOP(UnitConstant) \
  URP_GOP(UnitConstants) \
  URP_IOP(UnitMergeable) \
  URP_GOP(UnitMergeables) \
  URP_IOP(UnitSourceLoc) \
  URP_GOP(SourceLocTab)

#define URP_OP(c, o) \
  c##Stmt o[RepoIdCount];
  URP_OPS
#undef URP_OP
};

std::unique_ptr<UnitEmitter> createFatalUnit(
  StringData* filename,
  const SHA1& sha1,
  FatalOp op,
  StringData* err
);

template<class SerDe> void serdeLineTable(SerDe&, LineTable&);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_UNIT_EMITTER_INL_H_
#include "hphp/runtime/vm/unit-emitter-inl.h"
#undef incl_HPHP_VM_UNIT_EMITTER_INL_H_

#endif // incl_HPHP_VM_UNIT_EMITTER_H_
