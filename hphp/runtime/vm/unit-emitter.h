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

#include "hphp/runtime/vm/unit.h"

#include "hphp/util/md5.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct UnitEmitter {
  friend class UnitRepoProxy;

  /////////////////////////////////////////////////////////////////////////////
  // Constructor and destructor.

  explicit UnitEmitter(const MD5& md5);
  ~UnitEmitter();

  bool isASystemLib() const {
    static const char systemlib_prefix[] = "/:systemlib";
    return !strncmp(getFilepath()->data(),
                    systemlib_prefix,
                    sizeof systemlib_prefix - 1);
  }

  void addTrivialPseudoMain();

  int repoId() const { return m_repoId; }
  void setRepoId(int repoId) { m_repoId = repoId; }
  int64_t sn() const { return m_sn; }
  void setSn(int64_t sn) { m_sn = sn; }
  const unsigned char* bc() const { return m_bc; }
  Offset bcPos() const { return (Offset)m_bclen; }
  void setBc(const unsigned char* bc, size_t bclen);
  const StringData* getFilepath() const { return m_filepath; }
  void setFilepath(const StringData* filepath) { m_filepath = filepath; }

  void setMainReturn(const TypedValue* v) { m_mainReturn = *v; }
  void setMergeOnly(bool b) { m_mergeOnly = b; }
  const MD5& md5() const { return m_md5; }
  Id addTypeAlias(const TypeAlias& td);

  Id mergeLitstr(const StringData* litstr);
  Id mergeUnitLitstr(const StringData* litstr);
  Id mergeArray(const ArrayData* a);
  Id mergeArray(const ArrayData* a, const std::string& key);

  const StringData* lookupLitstr(Id id) const;
  const ArrayData* lookupArray(Id id) const;
  FuncEmitter* getMain();
  void initMain(int line1, int line2);
  FuncEmitter* newFuncEmitter(const StringData* n);
  void appendTopEmitter(FuncEmitter* func);
  FuncEmitter* newMethodEmitter(const StringData* n, PreClassEmitter* pce);
  PreClassEmitter* newPreClassEmitter(const StringData* n,
                                      PreClass::Hoistable hoistable);
  PreClassEmitter* pce(Id preClassId) { return m_pceVec[preClassId]; }
  const PreClassEmitter* pce(Id preClassId) const {
    return m_pceVec[preClassId];
  }
  size_t numPreClasses() const { return m_pceVec.size(); }
  const std::vector<FuncEmitter*>& fevec() const { return m_fes; }
  const std::vector<TypeAlias>& typeAliases() const { return m_typeAliases; }

  /*
   * Record source location information for the last chunk of bytecode
   * added to this UnitEmitter.  Adjacent regions associated with the
   * same source line will be collapsed as this is created.
   */
  void recordSourceLocation(const Location *sLoc, Offset start);

  /*
   * Return the SrcLocTable for this unit emitter, if it has one.
   * Otherwise an empty table is returned.
   */
  SourceLocTable createSourceLocTable() const;

  /*
   * Returns whether this unit emitter contains full SourceLoc
   * information.
   */
  bool hasSourceLocInfo() const { return !m_sourceLocTab.empty(); }

  /*
   * Returns access to this UnitEmitter's LineTable.  Generally
   * UnitEmitters loaded from a production repo will have a line table
   * instead of a full SourceLocTable.
   */
  const LineTable& lineTable() const { return m_lineTable; }

  /*
   * Adds a new FuncEmitter to the unit.  You can only do this once
   * for the FuncEmitter (after you are done setting it up).  Also,
   * all FuncEmitter's added to the unit must not overlap.
   *
   * Takes ownership of `fe'.
   */
  void recordFunction(FuncEmitter *fe);

private:
  template<class T>
  void emitImpl(T n, int64_t pos) {
    auto *c = (unsigned char*)&n;
    if (pos == -1) {
      // Make sure m_bc is large enough.
      while (m_bclen + sizeof(T) > m_bcmax) {
        m_bc = (unsigned char*)realloc(m_bc, m_bcmax << 1);
        m_bcmax <<= 1;
      }
      memcpy(&m_bc[m_bclen], c, sizeof(T));
      m_bclen += sizeof(T);
    } else {
      assert(pos + sizeof(T) <= m_bclen);
      for (uint i = 0; i < sizeof(T); ++i) {
        m_bc[pos + i] = c[i];
      }
    }
  }

public:
  void emitOp(Op op, int64_t pos = -1) {
    emitByte((unsigned char)op, pos);
  }
  void emitByte(unsigned char n, int64_t pos = -1) { emitImpl(n, pos); }
  void emitInt32(int n, int64_t pos = -1) { emitImpl(n, pos); }
  template<typename T> void emitIVA(T n) {
    if (LIKELY((n & 0x7f) == n)) {
      emitByte((unsigned char)n << 1);
    } else {
      assert((n & 0x7fffffff) == n);
      emitInt32((n << 1) | 0x1);
    }
  }
  void emitInt64(int64_t n, int64_t pos = -1) { emitImpl(n, pos); }
  void emitDouble(double n, int64_t pos = -1) { emitImpl(n, pos); }
  bool insert(UnitOrigin unitOrigin, RepoTxn& txn);
  void commit(UnitOrigin unitOrigin);
  Func* newFunc(const FuncEmitter* fe, Unit& unit, PreClass* preClass,
                int line1, int line2, Offset base, Offset past,
                const StringData* name, Attr attrs, bool top,
                const StringData* docComment, int numParams,
                bool needsNextClonedClosure);
  Unit* create();
  void returnSeen() { m_returnSeen = true; }
  void pushMergeableClass(PreClassEmitter* e);
  void pushMergeableInclude(Unit::MergeKind kind, const StringData* unitName);
  void insertMergeableInclude(int ix, Unit::MergeKind kind, Id id);
  void pushMergeableDef(Unit::MergeKind kind,
                        const StringData* name, const TypedValue& tv);
  void insertMergeableDef(int ix, Unit::MergeKind kind,
                          Id id, const TypedValue& tv);

private:
  void setLines(const LineTable& lines);


  /////////////////////////////////////////////////////////////////////////////
  // Properties.

private:
  int m_repoId;
  int64_t m_sn;
  static const size_t BCMaxInit = 4096; // Initial bytecode size.
  size_t m_bcmax;
  unsigned char* m_bc;
  size_t m_bclen;
  TypedValue m_mainReturn;
  const StringData* m_filepath;
  MD5 m_md5;
  typedef hphp_hash_map<const StringData*, Id,
                        string_data_hash, string_data_same> LitstrMap;
  LitstrMap m_litstr2id;
  std::vector<const StringData*> m_litstrs;
  typedef hphp_hash_map<std::string, Id, string_hash> ArrayIdMap;
  ArrayIdMap m_array2id;
  struct ArrayVecElm {
    std::string serialized;
    const ArrayData* array;
  };
  typedef std::vector<ArrayVecElm> ArrayVec;
  ArrayVec m_arrays;
  int m_nextFuncSn;
  bool m_mergeOnly;
  typedef std::vector<FuncEmitter*> FeVec;
  FeVec m_fes;
  typedef hphp_hash_map<const FuncEmitter*, const Func*,
                        pointer_hash<FuncEmitter> > FMap;
  FMap m_fMap;
  typedef std::vector<PreClassEmitter*> PceVec;
  typedef std::list<Id> IdList;
  PceVec m_pceVec;
  typedef hphp_hash_set<const StringData*, string_data_hash,
                        string_data_isame> HoistedPreClassSet;
  HoistedPreClassSet m_hoistablePreClassSet;
  IdList m_hoistablePceIdList;
  typedef std::vector<std::pair<Unit::MergeKind, Id> > MergeableStmtVec;
  MergeableStmtVec m_mergeableStmts;
  std::vector<std::pair<Id,TypedValue> > m_mergeableValues;
  bool m_allClassesHoistable;
  bool m_returnSeen;
  /*
   * m_sourceLocTab and m_feTab are interval maps.  Each entry encodes
   * an open-closed range of bytecode offsets.
   *
   * The m_sourceLocTab is keyed by the start of each half-open range.
   * This is to allow appending new bytecode offsets that are part of
   * the same range to coalesce.
   *
   * The m_feTab is keyed by the past-the-end offset.  This is the
   * format we'll want it in when we go to create a Unit.
   */
  std::vector<std::pair<Offset,SourceLoc> > m_sourceLocTab;
  std::vector<std::pair<Offset,const FuncEmitter*> > m_feTab;
  LineTable m_lineTable;
  std::vector<TypeAlias> m_typeAliases;
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
  Unit* load(const std::string& name, const MD5& md5);
  std::unique_ptr<UnitEmitter> loadEmitter(const std::string& name,
                                           const MD5& md5);

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
  URP_GOP(SourceLoc) \
  URP_GOP(SourceLocTab) \
  URP_GOP(SourceLocPastOffsets) \
  URP_GOP(SourceLocBaseOffset) \
  URP_GOP(BaseOffsetAtPCLoc) \
  URP_GOP(BaseOffsetAfterPCLoc)
  class InsertUnitStmt : public RepoProxy::Stmt {
   public:
    InsertUnitStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn,
                int64_t& unitSn,
                const MD5& md5,
                const unsigned char* bc,
                size_t bclen,
                const TypedValue* mainReturn,
                bool mergeOnly,
                const LineTable& lines,
                const std::vector<TypeAlias>&);
  };
  class GetUnitStmt : public RepoProxy::Stmt {
   public:
    GetUnitStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(UnitEmitter& ue, const MD5& md5);
  };
  class InsertUnitLitstrStmt : public RepoProxy::Stmt {
   public:
    InsertUnitLitstrStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn, Id litstrId,
                const StringData* litstr);
  };
  class GetUnitLitstrsStmt : public RepoProxy::Stmt {
   public:
    GetUnitLitstrsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
  class InsertUnitArrayStmt : public RepoProxy::Stmt {
   public:
    InsertUnitArrayStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn, Id arrayId,
                const std::string& array);
  };
  class GetUnitArraysStmt : public RepoProxy::Stmt {
   public:
    GetUnitArraysStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
  class InsertUnitMergeableStmt : public RepoProxy::Stmt {
   public:
    InsertUnitMergeableStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn,
                int ix, Unit::MergeKind kind,
                Id id, TypedValue *value);
  };
  class GetUnitMergeablesStmt : public RepoProxy::Stmt {
   public:
    GetUnitMergeablesStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
  class InsertUnitSourceLocStmt : public RepoProxy::Stmt {
   public:
    InsertUnitSourceLocStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64_t unitSn, Offset pastOffset, int line0,
                int char0, int line1, int char1);
  };
  class GetSourceLocStmt : public RepoProxy::Stmt {
   public:
    GetSourceLocStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64_t unitSn, Offset pc, SourceLoc& sLoc);
  };
  class GetSourceLocTabStmt : public RepoProxy::Stmt {
   public:
    GetSourceLocTabStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64_t unitSn, SourceLocTable& sourceLocTab);
  };
  class GetSourceLocPastOffsetsStmt : public RepoProxy::Stmt {
   public:
    GetSourceLocPastOffsetsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64_t unitSn, int line, OffsetRangeVec& ranges);
  };
  class GetSourceLocBaseOffsetStmt : public RepoProxy::Stmt {
   public:
    GetSourceLocBaseOffsetStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64_t unitSn, OffsetRange& range);
  };
  class GetBaseOffsetAtPCLocStmt : public RepoProxy::Stmt {
   public:
    GetBaseOffsetAtPCLocStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64_t unitSn, Offset pc, Offset& offset);
  };
  class GetBaseOffsetAfterPCLocStmt : public RepoProxy::Stmt {
   public:
    GetBaseOffsetAfterPCLocStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64_t unitSn, Offset pc, Offset& offset);
  };

private:
  bool loadHelper(UnitEmitter& ue, const std::string&, const MD5&);

#define URP_OP(c, o) \
 public: \
  c##Stmt& o(int repoId) { return *m_##o[repoId]; } \
 private: \
  c##Stmt m_##o##Local; \
  c##Stmt m_##o##Central; \
  c##Stmt* m_##o[RepoIdCount];
  URP_OPS
#undef URP_OP
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_VM_UNIT_EMITTER_H_
