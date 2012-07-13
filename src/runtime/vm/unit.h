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

#ifndef incl_VM_UNIT_H_
#define incl_VM_UNIT_H_

// Expects that runtime/vm/core_types.h is already included.
#include "runtime/base/runtime_option.h"
#include "runtime/vm/hhbc.h"
#include "runtime/vm/class.h"
#include "runtime/vm/repo_helpers.h"
#include "runtime/base/array/hphp_array.h"
#include "util/parser/location.h"
#include "runtime/base/md5.h"
#include "util/tiny_vector.h"

namespace HPHP {
namespace VM {

enum UnitOrigin {
  UnitOriginFile = 0,
  UnitOriginEval = 1
};

typedef const uchar* PC;

// Forward declarations.
class Func;
class FuncEmitter;
class Repo;
class FuncDict;
class Unit;
struct ActRec;

struct NamedEntity {
  Class* m_class;
  unsigned m_cachedFuncOffset;

  Class* const* clsList() const { return &m_class; }
  void setCachedFunc(Func *f);
  Func* getCachedFunc() const;
};

typedef std::pair<const StringData*,const NamedEntity*> NamedEntityPair;

// Exception handler table entry.
class EHEnt {
 public:
  enum EHType {
    EHType_Catch,
    EHType_Fault
  };
  EHType m_ehtype;
  Offset m_base;
  Offset m_past;
  int m_iterId;
  int m_parentIndex;
  Offset m_fault;
  typedef std::vector<std::pair<Id, Offset> > CatchVec;
  CatchVec m_catches;

  template<class SerDe> void serde(SerDe& sd) {
    sd(m_ehtype)
      (m_base)
      (m_past)
      (m_iterId)
      (m_fault)
      // eh.m_parentIndex is re-computed in sortEHTab, not serialized.
      ;
    if (m_ehtype == EHType_Catch) {
      sd(m_catches);
    }
  }
};

class EHEntComp {
 public:
  bool operator() (const EHEnt &eh1, const EHEnt &eh2) {
    if (eh1.m_base == eh2.m_base) {
      // for identical address ranges, Catch funclet is "outer" to Fault funclet
      if (eh1.m_past == eh2.m_past) {
        return eh1.m_ehtype == EHEnt::EHType_Catch;
      }
      return eh1.m_past > eh2.m_past;
    }
    return eh1.m_base < eh2.m_base;
  }
};

// Function paramater info region table entry.
class FPIEnt {
 public:
  Offset m_fpushOff;
  Offset m_fcallOff;
  Offset m_fpOff; // evaluation stack depth to current frame pointer
  int m_parentIndex;
  int m_fpiDepth;

  template<class SerDe> void serde(SerDe& sd) {
    sd(m_fpushOff)(m_fcallOff)(m_fpOff);
    // These fields are recomputed by sortFPITab:
    // m_parentIndex;
    // m_fpiDepth;
  }
};

class FPIEntComp {
 public:
  bool operator() (const FPIEnt &fpi1, const FPIEnt &fpi2) {
    return fpi1.m_fpushOff < fpi2.m_fpushOff;
  }
};

class SourceLoc {
 public:
  SourceLoc() : line0(1), char0(1), line1(1), char1(1) {}
  explicit SourceLoc(const Location& l) { setLoc(&l); }

  int line0;
  int char0;
  int line1;
  int char1;

  // {1, 1, 1, 1} is a special "invalid" value.
  void reset() {
    line0 = char0 = line1 = char1 = 1;
  }

  bool valid() {
    return line0 != 1 || char0 != 1 || line1 != 1 || char1 != 1;
  }

  void setLoc(const Location *l) {
    line0 = l->line0;
    char0 = l->char0;
    line1 = l->line1;
    char1 = l->char1;
  }

  bool same(const SourceLoc *l) const {
    return (this == l) ||
           (line0 == l->line0 && char0 == l->char0 &&
            line1 == l->line1 && char1 == l->char1);
  }

  bool operator==(const SourceLoc &l) const {
    return same(&l);
  }
};

class OffsetRange {
 public:
  OffsetRange() : m_base(0), m_past(0) {}
  OffsetRange(Offset base, Offset past) : m_base(base), m_past(past) {}
  Offset m_base;
  Offset m_past;
};
typedef std::vector<OffsetRange> OffsetRangeVec;

template<typename T>
class TableEntry {
 public:
  TableEntry() : m_pastOffset(0) {}

  TableEntry(Offset pastOffset, T val)
    : m_pastOffset(pastOffset), m_val(val) {}
  Offset pastOffset() const { return m_pastOffset; }
  T val() const { return m_val; }
  bool operator <(const TableEntry& other) const {
    return m_pastOffset < other.m_pastOffset;
  }

  template<class SerDe> void serde(SerDe& sd) { sd(m_pastOffset)(m_val); }

 private:
  Offset m_pastOffset;
  T m_val;
};

typedef TableEntry<int> LineEntry;
typedef std::vector<LineEntry> LineTable;
typedef TableEntry<const Func*> FuncEntry;
typedef std::vector<FuncEntry> FuncTable;

/*
 * Each Unit has one of these structs for each DefCns instruction. If
 * the value is not known at Unit emission time, the 'value' field
 * will be KindOfUninit. The 'owner' field is an opaque blob used by
 * Units and ExecutionContexes to identify which PreConsts belong to
 * them on destruction.
 *
 * If user code contains a call to define($s, ...) where $s is not a
 * string known at compile time, it will be left as a normal call to
 * the function define. If that code is ever executed, a PreConst will
 * be created by that request's g_vmContext and destroyed at the end
 * of the request.
 */
struct PreConst {
  TypedValue value;
  void* owner;
  const StringData* name;
};
typedef std::vector<PreConst> PreConstVec;

//==============================================================================
// (const StringData*) versus (StringData*)
//
// All (const StringData*) values are static strings that came from e.g.
// StringData::GetStaticString().  Therefore no reference counting is required.
//
//==============================================================================

// Compilation unit.
struct Unit {
  friend class UnitEmitter;
  friend class UnitRepoProxy;
  friend class FuncDict;
  friend class MetaHandle;

  typedef TinyVector<Func*> FuncVec;

  class MetaInfo {
   public:
    enum Kind {
      None,
      String,
      Class,
      NopOut,
      DataType
    };
    static const int VectorArg = 1 << 7;
    MetaInfo(Kind k, int a, Id d) : m_kind(k), m_arg(a), m_data(d) {
      ASSERT((int)m_arg == a);
    }
    MetaInfo() : m_kind(None), m_arg(-1), m_data(0) {}

    /*
     * m_arg indicates which input the MetaInfo applies to.
     *
     * For instructions taking vector immediates, it is an index into
     * the immediate elements, excluding any MW members (and including
     * the base).  (This is currently even if the instruction takes
     * other stack arguments.)
     */
    Kind  m_kind;
    uint8 m_arg;
    Id    m_data;
  };

  class MetaHandle {
    friend class Unit;
    /*
      The meta-data in Unit::m_bc_meta is stored as:

      Offset     <num entries>
      Offset     byte-code-offset-1
      Offset     byte-code-offset-2
      ...
      Offset     byte-code_offset-n
      Offset     INT_MAX # sentinel
      Offset     data-offset-1
      Offset     data-offset-2
      ...
      Offset     data-offset-n
      Offset     m_bc_meta_len # sentinel
      uint8      m_kind1
      uint8      m_arg1
      VSI        m_data1
      ...
      uint8      m_kind-n
      uint8      m_arg-n
      VSI        m_data-n
    */
   public:
    MetaHandle() : index(NULL), cur(0) {}
    bool findMeta(const Unit* unit, Offset offset);
    bool nextArg(MetaInfo& info);
   private:
    const Offset* index;
    unsigned cur;
    const uint8 *ptr;
  };

  Unit();
  ~Unit();

  int repoId() const { return m_repoId; }
  int64 sn() const { return m_sn; }

  PC entry() const { return m_bc; }
  Offset bclen() const { return m_bclen; }
  PC at(const Offset off) const {
    ASSERT(off >= 0 && off <= Offset(m_bclen));
    return m_bc + off;
  }
  Offset offsetOf(const Opcode* op) const {
    ASSERT(op >= m_bc && op <= (m_bc + m_bclen));
    return op - m_bc;
  }

  const StringData* filepath() const {
    ASSERT(m_filepath);
    return m_filepath;
  }
  CStrRef filepathRef() const {
    ASSERT(m_filepath);
    return *(String*)(&m_filepath);
  }
  const StringData* dirpath() const {
    ASSERT(m_dirpath);
    return m_dirpath;
  }

  MD5 md5() const { return m_md5; }

  static const NamedEntity* GetNamedEntity(const StringData *);
  static Array getUserFunctions();
  static Array getClassesInfo();
  static Array getInterfacesInfo();
  static Array getTraitsInfo();

  size_t numLitstrs() const {
    return m_namedInfo.size();
  }
  StringData* lookupLitstrId(Id id) const {
    ASSERT(id >= 0 && id < Id(m_namedInfo.size()));
    return const_cast<StringData*>(m_namedInfo[id].first);
  }

  const NamedEntity* lookupNamedEntityId(Id id) const {
    return lookupNamedEntityPairId(id).second;
  }

  const NamedEntityPair& lookupNamedEntityPairId(Id id) const {
    ASSERT(id < Id(m_namedInfo.size()));
    const NamedEntityPair &ne = m_namedInfo[id];
    ASSERT(ne.first);
    if (UNLIKELY(!ne.second)) {
      const_cast<const NamedEntity*&>(ne.second) = GetNamedEntity(ne.first);
    }
    return ne;
  }

  size_t numArrays() const {
    return m_arrays.size();
  }
  ArrayData* lookupArrayId(Id id) const {
    return const_cast<ArrayData*>(m_arrays.at(id));
  }

  static Func *lookupFunc(const NamedEntity *ne, const StringData* name);

  static Func *lookupFunc(const StringData *funcName);

  static Class* defClass(HPHP::VM::PreClass* preClass,
                         bool failIsFatal = true);

  static Class *lookupClass(const NamedEntity *ne) {
    Class *cls = *ne->clsList();
    if (LIKELY(cls != NULL)) cls = cls->getCached();
    return cls;
  }

  static Class *lookupClass(const StringData *clsName) {
    Class *cls = *GetNamedEntity(clsName)->clsList();
    if (LIKELY(cls != NULL)) cls = cls->getCached();
    return cls;
  }

  static Class *loadClass(const NamedEntity *ne,
                          const StringData *name);

  static Class *loadClass(const StringData *name) {
    return loadClass(GetNamedEntity(name), name);
  }

  static Class *loadMissingClass(const NamedEntity *ne,
                                 const StringData *name);

  static Class* getClass(const StringData* name, bool tryAutoload) {
    return getClass(GetNamedEntity(name), name, tryAutoload);
  }

  static Class* getClass(const NamedEntity *ne, const StringData *name,
                         bool tryAutoload);
  static bool classExists(const StringData* name, bool autoload,
                          Attr typeAttrs);

  Class *lookupClass(Id id) const {
    return lookupClass(lookupNamedEntityId(id));
  }

  const PreConst* lookupPreConstId(Id id) const {
    ASSERT(id < Id(m_preConsts.size()));
    return &m_preConsts[id];
  }

  bool compileTimeFatal(const StringData*& msg, int& line) const;
  Func* getMain() const {
    return m_funcs.front();
  }
  const TypedValue *getMainReturn() const {
    return &m_mainReturn;
  }
  Func* getLambda() const {
    ASSERT(m_funcs.size() == 2);
    return m_funcs[1];
  }
  void renameFunc(const StringData* oldName, const StringData* newName);
  void mergeFuncs() const;
  static void loadFunc(Func *func);
  const FuncVec& funcs() const {
    return m_funcs;
  }
  Func* lookupFuncId(Id id) const {
    ASSERT(id < Id(m_funcs.size()));
    return m_funcs[id];
  }

  size_t numPreClasses() const {
    return (size_t)m_preClasses.size();
  }
  PreClass* lookupPreClassId(Id id) const {
    ASSERT(id < Id(m_preClasses.size()));
    return m_preClasses[id].get();
  }
  void mergeClasses() const;
  void merge();

  void mergePreConsts() {
    if (LIKELY(RuntimeOption::RepoAuthoritative ||
               atomic_acquire_load(&m_preConstsMerged))) return;
    mergePreConstsWork();
  }
  void mergePreConstsWork();

  int getLineNumber(Offset pc) const;
  bool getSourceLoc(Offset pc, SourceLoc& sLoc) const;
  bool getOffsetRanges(int line, OffsetRangeVec& offsets) const;
  bool getOffsetRange(Offset pc, OffsetRange& range) const;

  Opcode getOpcode(size_t instrOffset) const {
    ASSERT(instrOffset < m_bclen);
    return (Opcode)m_bc[instrOffset];
  }

  const Func* getFunc(Offset pc) const;
  void enableIntercepts();

  bool isMergeOnly() const { return m_mainReturn._count; }
public:
  static Mutex s_classesMutex;

  std::string toString() const;
  static void dumpUnit(Unit* unit);
  void prettyPrint(std::ostream &out) const;
  void prettyPrint(std::ostream &out, size_t startOffset, size_t stopOffset)
    const;

public: // Translator field access
  static size_t bcOff() { return offsetof(Unit, m_bc); }

private:
  typedef hphp_hash_map<const StringData*, Id,
                        string_data_hash, string_data_same> ArrayIdMap;
  typedef std::vector<PreClassPtr> PreClassPtrVec;
  typedef std::vector<Func*> HoistableFuncVec;
  typedef std::vector<PreClass*> PreClassVec;

private:
  /*
   * pseudoMain's return value, or KindOfUninit if
   * its not known. Also use _count as a flag to
   * indicate that this is a mergeOnly unit
   */
  TypedValue m_mainReturn;
  int64 m_sn;
  uchar* m_bc;
  size_t m_bclen;
  uchar* m_bc_meta;
  size_t m_bc_meta_len;
  const StringData* m_filepath;
  const StringData* m_dirpath;
  MD5 m_md5;
  std::vector<NamedEntityPair> m_namedInfo;
  ArrayIdMap m_array2id;
  std::vector<const ArrayData*> m_arrays;
  FuncVec m_funcs;
  HoistableFuncVec m_hoistableFuncs;
  PreClassPtrVec m_preClasses;
  PreClassVec m_mergeablePreClasses;
  LineTable m_lineTable;
  FuncTable m_funcTable;
  int32 m_numHoistablePreClasses;
  int8 m_repoId;
  bool m_preConstsMerged;
  PreConstVec m_preConsts;
  SimpleMutex m_preConstsLock;
};

class UnitEmitter {
  friend class Peephole;
  friend class UnitRepoProxy;
 public:
  UnitEmitter(const MD5& md5);
  ~UnitEmitter();

  int repoId() const { return m_repoId; }
  void setRepoId(int repoId) { m_repoId = repoId; }
  int64 sn() const { return m_sn; }
  void setSn(int64 sn) { m_sn = sn; }
  Offset bcPos() const { return (Offset)m_bclen; }
  void setBc(const uchar* bc, size_t bclen);
  void setBcMeta(const uchar* bc_meta, size_t bc_meta_len);
  const StringData* getFilepath() { return m_filepath; }
  void setFilepath(const StringData* filepath) { m_filepath = filepath; }
  void setMainReturn(const TypedValue* v) { m_mainReturn = *v; }
  const MD5& md5() const { return m_md5; }
  Id addPreConst(const StringData* name, const TypedValue& value);
  Id mergeLitstr(const StringData* litstr);
  Id mergeArray(ArrayData* a, const StringData* key=NULL);
  FuncEmitter* getMain();
  void initMain(int line1, int line2);
  FuncEmitter* newFuncEmitter(const StringData* n, bool top);
  FuncEmitter* newMethodEmitter(const StringData* n, PreClassEmitter* pce);
  PreClassEmitter* newPreClassEmitter(const StringData* n,
                                      PreClass::Hoistable hoistable);
  PreClassEmitter* pce(Id preClassId) { return m_pceVec[preClassId]; }

  /*
   * Record source location information for the last chunk of bytecode
   * added to this UnitEmitter.  Adjacent regions associated with the
   * same source line will be collapsed as this is created.
   */
  void recordSourceLocation(const Location *sLoc, Offset start);

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
  void emitImpl(T n, int64 pos) {
    uchar *c = (uchar*)&n;
    if (pos == -1) {
      // Make sure m_bc is large enough.
      while (m_bclen + sizeof(T) > m_bcmax) {
        m_bc = (uchar*)realloc(m_bc, m_bcmax << 1);
        m_bcmax <<= 1;
      }
      memcpy(&m_bc[m_bclen], c, sizeof(T));
      m_bclen += sizeof(T);
    } else {
      ASSERT(pos + sizeof(T) <= m_bclen);
      for (uint i = 0; i < sizeof(T); ++i) {
        m_bc[pos + i] = c[i];
      }
    }
  }
 public:
  void emitOp(Op op, int64 pos = -1) {
    emitByte((uchar)op, pos);
  }
  void emitByte(uchar n, int64 pos = -1) { emitImpl(n, pos); }
  void emitInt32(int n, int64 pos = -1) { emitImpl(n, pos); }
  template<typename T> void emitIVA(T n) {
    if (LIKELY((n & 0x7f) == n)) {
      emitByte((unsigned char)n << 1);
    } else {
      ASSERT((n & 0x7fffffff) == n);
      emitInt32((n << 1) | 0x1);
    }
  }
  void emitInt64(int64 n, int64 pos = -1) { emitImpl(n, pos); }
  void emitDouble(double n, int64 pos = -1) { emitImpl(n, pos); }
  void commit(UnitOrigin unitOrigin);
  Func* newFunc(const FuncEmitter* fe, Unit& unit, Id id, int line1, int line2,
                Offset base, Offset past,
                const StringData* name, Attr attrs, bool top,
                const StringData* docComment, int numParams);
  Func* newFunc(const FuncEmitter* fe, Unit& unit, PreClass* preClass,
                int line1, int line2, Offset base, Offset past,
                const StringData* name, Attr attrs, bool top,
                const StringData* docComment, int numParams);
  Unit* create();

 private:
  void setLines(const LineTable& lines);

 private:
  int m_repoId;
  int64 m_sn;
  static const size_t BCMaxInit = 4096; // Initial bytecode size.
  size_t m_bcmax;
  uchar* m_bc;
  size_t m_bclen;
  uchar* m_bc_meta;
  size_t m_bc_meta_len;
  TypedValue m_mainReturn;
  const StringData* m_filepath;
  MD5 m_md5;
  typedef hphp_hash_map<const StringData*, Id,
                        string_data_hash, string_data_same> LitstrMap;
  LitstrMap m_litstr2id;
  std::vector<const StringData*> m_litstrs;
  Unit::ArrayIdMap m_array2id;
  typedef struct {
    const StringData* serialized;
    const ArrayData* array;
  } ArrayVecElm;
  typedef std::vector<ArrayVecElm> ArrayVec;
  ArrayVec m_arrays;
  int m_nextFuncSn;
  typedef std::vector<FuncEmitter*> FeVec;
  FeVec m_fes;
  typedef hphp_hash_map<const StringData*, FuncEmitter*, string_data_hash,
                        string_data_isame> FuncEmitterMap;
  FuncEmitterMap m_feMap;
  typedef hphp_hash_map<const FuncEmitter*, const Func*,
                        pointer_hash<FuncEmitter> > FMap;
  FMap m_fMap;
  typedef std::vector<PreClassEmitter*> PceVec;
  PceVec m_pceVec;
  typedef hphp_hash_set<const StringData*, string_data_hash,
                        string_data_isame> HoistedPreClassSet;
  HoistedPreClassSet m_hoistablePreClassSet;
  PceVec m_hoistablePceVec;
  PceVec m_remainingPceVec;
  bool m_allClassesHoistable;
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
  PreConstVec m_preConsts;
};

class UnitRepoProxy : public RepoProxy {
  friend class Unit;
  friend class UnitEmitter;
 public:
  UnitRepoProxy(Repo& repo);
  ~UnitRepoProxy();
  void createSchema(int repoId, RepoTxn& txn);
  Unit* load(const std::string& name, const MD5& md5);

#define URP_IOP(o) URP_OP(Insert##o, insert##o)
#define URP_GOP(o) URP_OP(Get##o, get##o)
#define URP_OPS \
  URP_IOP(Unit) \
  URP_GOP(Unit) \
  URP_IOP(UnitLitstr) \
  URP_GOP(UnitLitstrs) \
  URP_IOP(UnitArray) \
  URP_GOP(UnitArrays) \
  URP_IOP(UnitPreConst) \
  URP_GOP(UnitPreConsts) \
  URP_IOP(UnitSourceLoc) \
  URP_GOP(SourceLoc) \
  URP_GOP(SourceLocPastOffsets) \
  URP_GOP(SourceLocBaseOffset) \
  URP_GOP(BaseOffsetAtPCLoc) \
  URP_GOP(BaseOffsetAfterPCLoc)
  class InsertUnitStmt : public RepoProxy::Stmt {
   public:
    InsertUnitStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64& unitSn, const MD5& md5, const uchar* bc,
                size_t bclen, const uchar* bc_meta, size_t bc_meta_len,
                const TypedValue* mainReturn,
                const LineTable& lines);
  };
  class GetUnitStmt : public RepoProxy::Stmt {
   public:
    GetUnitStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(UnitEmitter& ue, const MD5& md5);
  };
  class InsertUnitLitstrStmt : public RepoProxy::Stmt {
   public:
    InsertUnitLitstrStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, Id litstrId,
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
    void insert(RepoTxn& txn, int64 unitSn, Id arrayId,
                const StringData* array);
  };
  class GetUnitArraysStmt : public RepoProxy::Stmt {
   public:
    GetUnitArraysStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
  class InsertUnitPreConstStmt : public RepoProxy::Stmt {
  public:
    InsertUnitPreConstStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, const PreConst& pc,
                Id id);
  };
  class GetUnitPreConstsStmt : public RepoProxy::Stmt {
  public:
    GetUnitPreConstsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void get(UnitEmitter& ue);
  };
  class InsertUnitSourceLocStmt : public RepoProxy::Stmt {
   public:
    InsertUnitSourceLocStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    void insert(RepoTxn& txn, int64 unitSn, Offset pastOffset, int line0,
                int char0, int line1, int char1);
  };
  class GetSourceLocStmt : public RepoProxy::Stmt {
   public:
    GetSourceLocStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64 unitSn, Offset pc, SourceLoc& sLoc);
  };
  class GetSourceLocPastOffsetsStmt : public RepoProxy::Stmt {
   public:
    GetSourceLocPastOffsetsStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64 unitSn, int line, OffsetRangeVec& ranges);
  };
  class GetSourceLocBaseOffsetStmt : public RepoProxy::Stmt {
   public:
    GetSourceLocBaseOffsetStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64 unitSn, OffsetRange& range);
  };
  class GetBaseOffsetAtPCLocStmt : public RepoProxy::Stmt {
   public:
    GetBaseOffsetAtPCLocStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64 unitSn, Offset pc, Offset& offset);
  };
  class GetBaseOffsetAfterPCLocStmt : public RepoProxy::Stmt {
   public:
    GetBaseOffsetAfterPCLocStmt(Repo& repo, int repoId) : Stmt(repo, repoId) {}
    bool get(int64 unitSn, Offset pc, Offset& offset);
  };
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

// hphp_compiler_parse() is defined in the compiler, but we must use
// dlsym() to get at it. CompileStringFn matches its signature.
typedef Unit*(*CompileStringFn)(const char*, int, const MD5&, const char*);

} } // HPHP::VM
#endif
