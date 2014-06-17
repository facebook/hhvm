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

#ifndef incl_HPHP_VM_UNIT_H_
#define incl_HPHP_VM_UNIT_H_

#include "hphp/parser/location.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/repo-helpers.h"
#include "hphp/runtime/vm/type-alias.h"

#include "hphp/util/md5.h"
#include "hphp/util/range.h"
#include "hphp/util/tiny-vector.h"

#include <memory>

namespace HPHP {

struct ActRec;
struct Func;
struct FuncEmitter;
struct Repo;
struct FuncDict;
struct Unit;
struct PreClassEmitter;

enum class UnitOrigin {
  File = 0,
  Eval = 1
};

enum UnitMergeKind {
  // UnitMergeKindClass is required to be 0 for correctness.
  UnitMergeKindClass = 0,
  UnitMergeKindUniqueDefinedClass = 1,
  // Top level, scalar defines in the unit
  UnitMergeKindDefine = 2,
  // Top level, scalar defines that will be loaded once
  // and preserved from request to request
  UnitMergeKindPersistentDefine = 3,
  UnitMergeKindGlobal = 4,
  // 5 is available
  UnitMergeKindReqDoc = 6,
  UnitMergeKindDone = 7,
  // We cannot add more kinds here; this has to fit in 3 bits.
};

enum UnitMergeState {
  UnitMergeStateUnmerged = 0,
  UnitMergeStateMerging = 1,
  UnitMergeStateMerged = 2,
  UnitMergeStateUniqueFuncs = 4,
  UnitMergeStateNeedsCompact = 8,
  UnitMergeStateEmpty = 32
};

ALWAYS_INLINE
bool isMergeKindReq(UnitMergeKind k) {
  return k == UnitMergeKindReqDoc;
}

struct UnitMergeInfo {
  typedef IterRange<Func* const*> FuncRange;
  typedef IterRange<Func**> MutableFuncRange;

  unsigned m_firstHoistableFunc;
  unsigned m_firstHoistablePreClass;
  unsigned m_firstMergeablePreClass;
  unsigned m_mergeablesSize;
  void*    m_mergeables[1];

  static UnitMergeInfo* alloc(size_t num);

  Func** funcBegin() const {
    return (Func**)m_mergeables;
  }
  Func** funcEnd() const {
    return funcBegin() + m_firstHoistablePreClass;
  }
  Func** funcHoistableBegin() const {
    return funcBegin() + m_firstHoistableFunc;
  }
  MutableFuncRange nonMainFuncs() const {
    return MutableFuncRange(funcBegin() + 1, funcEnd());
  }
  MutableFuncRange hoistableFuncs() const {
    return MutableFuncRange(funcHoistableBegin(), funcEnd());
  }
  FuncRange funcs() const {
    return FuncRange(funcBegin(), funcEnd());
  }
  MutableFuncRange mutableFuncs() {
    return MutableFuncRange(funcBegin(), funcEnd());
  }
  void*& mergeableObj(int ix) { return ((void**)m_mergeables)[ix]; }
  void* mergeableData(int ix) { return (char*)m_mergeables + ix*sizeof(void*); }
};

// Exception handler table entry.
class EHEnt {
 public:
  enum class Type {
    Catch,
    Fault
  };
  Type m_type;
  Offset m_base;
  Offset m_past;
  int m_iterId;
  bool m_itRef;
  int m_parentIndex;
  Offset m_fault;
  typedef std::vector<std::pair<Id, Offset> > CatchVec;
  CatchVec m_catches;

  template<class SerDe> void serde(SerDe& sd) {
    sd(m_type)
      (m_base)
      (m_past)
      (m_iterId)
      (m_fault)
      (m_itRef)
      (m_parentIndex)
      ;
    if (m_type == Type::Catch) {
      sd(m_catches);
    }
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
typedef TableEntry<SourceLoc> SourceLocEntry;
typedef std::vector<SourceLocEntry> SourceLocTable;
typedef std::map<int, OffsetRangeVec> LineToOffsetRangeVecMap;
typedef TableEntry<const Func*> FuncEntry;
typedef std::vector<FuncEntry> FuncTable;

//==============================================================================
// (const StringData*) versus (StringData*)
//
// All (const StringData*) values are static strings that came from e.g.
// makeStaticString().  Therefore no reference counting is required.
//
//==============================================================================

// Functions for differentiating global litstrId's from unit-local Id's.
const int kGlobalLitstrOffset = 0x40000000;
inline bool isGlobalLitstrId(Id id) { return id >= kGlobalLitstrOffset; }
inline Id encodeGlobalLitstrId(Id id) { return id + kGlobalLitstrOffset; }
inline Id decodeGlobalLitstrId(Id id) { return id - kGlobalLitstrOffset; }

/*
 * Global table of literal strings.  This can only be safely used when
 * the repo is built in WholeProgram mode and run in RepoAuthoritative
 * mode.
 */
class LitstrTable {
private:
  static LitstrTable* s_litstrTable;

public:
  static void init() {
    LitstrTable::s_litstrTable = new LitstrTable();
  }

  static LitstrTable& get() {
    return *LitstrTable::s_litstrTable;
  }

  ~LitstrTable() {}
  Id mergeLitstr(const StringData* litstr);
  size_t numLitstrs() { return m_namedInfo.size(); }
  StringData* lookupLitstrId(Id id) const;
  const NamedEntity* lookupNamedEntityId(Id id) const;
  const NamedEntityPair& lookupNamedEntityPairId(Id id) const;
  void insert(RepoTxn& txn, UnitOrigin uo);
  Mutex& mutex() { return m_mutex; }

  void setReading() { m_safeToRead = true; }
  void setWriting() { m_safeToRead = false; }

private:
  LitstrTable() {}
  typedef hphp_hash_map<const StringData*, Id,
                        string_data_hash, string_data_same> LitstrMap;

  LitstrMap m_litstr2id;
  std::vector<const StringData*> m_litstrs;
  std::vector<NamedEntityPair> m_namedInfo;
  Mutex m_mutex;
  std::atomic<bool> m_safeToRead;
};

/*
 * Metadata about a compilation unit.
 *
 * Contains the list of PreClasses and global functions, along with a
 * special function called the 'pseudo-main', which is logically
 * invoked (modulo optimizations that avoid it) during execution when
 * the unit is included/required.
 */
struct Unit {
  friend class UnitEmitter;
  friend class UnitRepoProxy;
  friend class FuncDict;

  typedef UnitMergeInfo::FuncRange FuncRange;
  typedef UnitMergeInfo::MutableFuncRange MutableFuncRange;

  typedef hphp_hash_map<const Class*, Func*,
                        pointer_hash<Class> > PseudoMainCacheMap;

  Unit();
  ~Unit();
  void* operator new(size_t sz);
  void operator delete(void* p, size_t sz);

  int repoId() const { return m_repoId; }
  int64_t sn() const { return m_sn; }

  PC entry() const { return m_bc; }
  Offset bclen() const { return m_bclen; }

  PC at(Offset off) const {
    assert(off >= 0 && off <= Offset(m_bclen));
    return m_bc + off;
  }

  Offset offsetOf(PC pc) const {
    assert(contains(pc));
    return pc - m_bc;
  }

  bool contains(PC pc) const {
    return pc >= m_bc && pc <= m_bc + m_bclen;
  }

  const StringData* filepath() const {
    assert(m_filepath);
    return m_filepath;
  }
  const StringData* dirpath() const {
    assert(m_dirpath);
    return m_dirpath;
  }

  MD5 md5() const { return m_md5; }

  static NamedEntity* GetNamedEntity(const StringData *str,
                                     bool allowCreate = true,
                                     String* normStr = nullptr) FLATTEN;

  static size_t GetNamedEntityTableSize();
  static Array getClassesInfo();
  static Array getInterfacesInfo();
  static Array getTraitsInfo();
  static Array getClassesWithAttrInfo(HPHP::Attr attrs, bool inverse = false);
  static Array getUserFunctions() { return getFunctions(false); }
  static Array getSystemFunctions() { return getFunctions(true); }

 private:
  static Array getFunctions(bool system);

 public:
  size_t numLitstrs() const {
    return m_namedInfo.size();
  }

  StringData* lookupLitstrId(Id id) const {
    if (isGlobalLitstrId(id)) {
      return LitstrTable::get().lookupLitstrId(decodeGlobalLitstrId(id));
    }
    assert(id >= 0 && id < Id(m_namedInfo.size()));
    return const_cast<StringData*>(m_namedInfo[id].first);
  }

  const NamedEntity* lookupNamedEntityId(Id id) const {
    if (isGlobalLitstrId(id)) {
      return LitstrTable::get().lookupNamedEntityId(decodeGlobalLitstrId(id));
    }
    return lookupNamedEntityPairId(id).second;
  }

  const NamedEntityPair& lookupNamedEntityPairId(Id id) const {
    if (isGlobalLitstrId(id)) {
      auto decodedId = decodeGlobalLitstrId(id);
      return LitstrTable::get().lookupNamedEntityPairId(decodedId);
    }
    assert(id < Id(m_namedInfo.size()));
    const NamedEntityPair &ne = m_namedInfo[id];
    assert(ne.first);
    assert(ne.first->data()[ne.first->size()] == 0);
    assert(ne.first->data()[0] != '\\');
    if (UNLIKELY(!ne.second)) {
      const_cast<const NamedEntity*&>(ne.second) = GetNamedEntity(ne.first);
    }
    return ne;
  }

  bool checkStringId(Id id) const;

  size_t numArrays() const {
    return m_arrays.size();
  }
  ArrayData* lookupArrayId(Id id) const {
    return const_cast<ArrayData*>(m_arrays.at(id));
  }

  static Func* lookupFunc(const NamedEntity *ne);
  static Func* lookupFunc(const StringData *funcName);
  static Func* loadFunc(const NamedEntity *ne, const StringData* name);
  static Func* loadFunc(const StringData* name);

  static Class* defClass(const HPHP::PreClass* preClass,
                         bool failIsFatal = true);
  static bool aliasClass(Class* original, const StringData* alias);
  void defTypeAlias(Id id);

  static const Cell* lookupCns(const StringData* cnsName);
  static const Cell* lookupPersistentCns(const StringData* cnsName);
  static const Cell* loadCns(const StringData* cnsName);
  static bool defCns(const StringData* cnsName, const TypedValue* value,
                     bool persistent = false);
  static uint64_t defCnsHelper(uint64_t ch,
                               const TypedValue* value,
                               const StringData* cnsName);
  static void defDynamicSystemConstant(const StringData* cnsName,
                                       const void* data);
  static bool defCnsDynamic(const StringData* cnsName, TypedValue* value);

  /*
   * Find the Class* for a defined class corresponding to the name
   * `clsName'.
   *
   * Returns: nullptr if the class of the given name is not yet
   * defined in this request.
   */
  static Class *lookupClass(const StringData *clsName) {
    return lookupClass(GetNamedEntity(clsName));
  }

  /*
   * Find the Class* for a defined class with name mapped to the
   * supplied NamedEntity.
   *
   * Returns: nullptr if the class is not yet defined in this request.
   */
  static Class *lookupClass(const NamedEntity *ne) {
    Class* cls;
    if (LIKELY((cls = ne->getCachedClass()) != nullptr)) {
      return cls;
    }
    return nullptr;
  }

  /*
   * Same as lookupClass, except if it's not defined *and* is unique,
   * return the Class* anyway.
   *
   * The point of this is that when jitting code before a unique class
   * is defined, we can often still burn the Class* into the TC, since
   * it will be defined by the time the code that needs the Class*
   * runs (via autoload or whatnot).
   */
  static Class *lookupUniqueClass(const NamedEntity *ne) {
    Class* cls = ne->clsList();
    if (LIKELY(cls != nullptr)) {
      if (cls->attrs() & AttrUnique && RuntimeOption::RepoAuthoritative) {
        return cls;
      }
      return cls->getCached();
    }
    return nullptr;
  }

  static Class *lookupUniqueClass(const StringData *clsName) {
    return lookupUniqueClass(GetNamedEntity(clsName));
  }

  static Class *loadClass(const NamedEntity *ne,
                          const StringData *name);

  static Class *loadClass(const StringData *name) {
    String normStr;
    auto ne = GetNamedEntity(name, true, &normStr);
    if (normStr) {
      name = normStr.get();
    }
    return loadClass(ne, name);
  }

  static Class *loadMissingClass(const NamedEntity *ne,
                                 const StringData *name);

  static Class* getClass(const StringData* name, bool tryAutoload) {
    String normStr;
    auto ne = GetNamedEntity(name, true, &normStr);
    if (normStr) {
      name = normStr.get();
    }
    return getClass(ne, name, tryAutoload);
  }

  static Class* getClass(const NamedEntity *ne, const StringData *name,
                         bool tryAutoload);
  static bool classExists(const StringData* name, bool autoload,
                          ClassKind kind);

  bool compileTimeFatal(const StringData*& msg, int& line) const;
  bool parseFatal(const StringData*& msg, int& line) const;
  const TypedValue *getMainReturn() const {
    assert(isMergeOnly());
    return &m_mainReturn;
  }

private:
  template <bool debugger>
  void mergeImpl(void* tcbase, UnitMergeInfo* mi);
public:
  Func* firstHoistable() const {
    return *m_mergeInfo->funcHoistableBegin();
  }
  Func* getMain(Class* cls = nullptr) const;
  // Ranges for iterating over functions.
  MutableFuncRange nonMainFuncs() const {
    return m_mergeInfo->nonMainFuncs();
  }
  MutableFuncRange hoistableFuncs() const {
    return m_mergeInfo->hoistableFuncs();
  }
  void renameFunc(const StringData* oldName, const StringData* newName);
  static void loadFunc(const Func *func);
  FuncRange funcs() const {
    return m_mergeInfo->funcs();
  }
  MutableFuncRange mutableFuncs() {
    return m_mergeInfo->mutableFuncs();
  }
  Func* lookupFuncId(Id id) const {
    assert(id < Id(m_mergeInfo->m_firstHoistablePreClass));
    return m_mergeInfo->funcBegin()[id];
  }
  size_t numPreClasses() const {
    return (size_t)m_preClasses.size();
  }
  PreClass* lookupPreClassId(Id id) const {
    assert(id < Id(m_preClasses.size()));
    return m_preClasses[id].get();
  }
  typedef std::vector<PreClassPtr> PreClassPtrVec;
  typedef Range<PreClassPtrVec> PreClassRange;
  void initialMerge();
  void merge();
  PreClassRange preclasses() const {
    return PreClassRange(m_preClasses);
  }
  bool mergeClasses() const;

  int getLineNumber(Offset pc) const;
  bool getSourceLoc(Offset pc, SourceLoc& sLoc) const;
  bool getOffsetRanges(int line, OffsetRangeVec& offsets) const;
  bool getOffsetRange(Offset pc, OffsetRange& range) const;

  Op getOpcode(size_t instrOffset) const {
    assert(instrOffset < m_bclen);
    return static_cast<Op>(m_bc[instrOffset]);
  }

  /*
   * Return the Func* for the code at offset off.
   *
   * Returns nullptr if the offset is not in a func body (but this
   * should be impossible).
   */
  const Func* getFunc(Offset pc) const;

  void setCacheId(unsigned id) {
    m_cacheOffset = id >> 3;
    m_cacheMask = 1 << (id & 7);
  }
  bool isInterpretOnly() const { return m_interpretOnly; }
  void setInterpretOnly() { m_interpretOnly = true; }
  bool isMergeOnly() const { return m_mergeOnly; }
  bool isEmpty() const { return m_mergeState & UnitMergeStateEmpty; }
  void* replaceUnit() const;

public:
  static Mutex s_classesMutex;

  struct PrintOpts {
    PrintOpts()
      : startOffset(kInvalidOffset)
      , stopOffset(kInvalidOffset)
      , showLines(true)
      , showFuncs(true)
      , indentSize(1)
    {}

    PrintOpts& range(Offset start, Offset stop) {
      startOffset = start;
      stopOffset = stop;
      return *this;
    }

    PrintOpts& noLineNumbers() {
      showLines = false;
      return *this;
    }

    PrintOpts& noFuncs() {
      showFuncs = false;
      return *this;
    }

    PrintOpts& indent(int i) {
      indentSize = i;
      return *this;
    }

    Offset startOffset;
    Offset stopOffset;
    bool showLines;
    bool showFuncs;
    int indentSize;
  };

  void prettyPrint(std::ostream&, PrintOpts = PrintOpts()) const;
  std::string toString() const;

public: // Translator field access
  static size_t bcOff() { return offsetof(Unit, m_bc); }

private:
  // List of (offset, sourceLoc) where offset is the offset of the first byte
  // code of the next source location if there is one, m_bclen otherwise.
  // Sorted by offset. sourceLocs are not assumed to be unique.
  SourceLocTable getSourceLocTable() const;
  // A map from all source lines that correspond to one or more byte codes.
  // The result from the map is a list of offset ranges, so a single line
  // with several sub-statements may correspond to the byte codes of all
  // of the sub-statements.
  LineToOffsetRangeVecMap getLineToOffsetRangeVecMap() const;

  /*
    Frequently used fields.
    Do not reorder without good reason
  */
  unsigned char const* m_bc{nullptr};
  size_t m_bclen{0};
  const StringData* m_filepath{nullptr};
  // List of (line, offset) where offset is the offset of the first byte code
  // of the next line if there is one, m_bclen otherwise.
  // Sorted by offset. line values are not assumed to be unique.
  LineTable m_lineTable;
  UnitMergeInfo* m_mergeInfo{nullptr};
  unsigned m_cacheOffset{0};
  int8_t m_repoId{-1};
  uint8_t m_mergeState{UnitMergeStateUnmerged};
  uint8_t m_cacheMask{0};
  bool m_mergeOnly{false};
  bool m_interpretOnly;
  // pseudoMain's return value, or KindOfUninit if its not known.
  TypedValue m_mainReturn;
  PreClassPtrVec m_preClasses;
  FixedVector<TypeAlias> m_typeAliases;
  /*
    End of freqently used fields
  */

  int64_t m_sn{-1};
  const StringData* m_dirpath{nullptr};
  MD5 m_md5;
  std::vector<NamedEntityPair> m_namedInfo;
  std::vector<const ArrayData*> m_arrays;
  SourceLocTable m_sourceLocTable;
  LineToOffsetRangeVecMap m_lineToOffsetRangeVecMap;
  FuncTable m_funcTable;
  mutable PseudoMainCacheMap *m_pseudoMainCache{nullptr};
};

int getLineNumber(const LineTable& table, Offset pc);
bool getSourceLoc(const SourceLocTable& table, Offset pc, SourceLoc& sLoc);

struct UnitEmitter {
  friend class UnitRepoProxy;

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
  void pushMergeableInclude(UnitMergeKind kind, const StringData* unitName);
  void insertMergeableInclude(int ix, UnitMergeKind kind, Id id);
  void pushMergeableDef(UnitMergeKind kind,
                        const StringData* name, const TypedValue& tv);
  void insertMergeableDef(int ix, UnitMergeKind kind,
                          Id id, const TypedValue& tv);

private:
  void setLines(const LineTable& lines);

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
  typedef std::vector<std::pair<UnitMergeKind, Id> > MergeableStmtVec;
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

//////////////////////////////////////////////////////////////////////

/*
 * Member functions of LitstrTable inlined for perf.  Must come after
 * Unit definition to break circular dependences.
 */
inline
StringData* LitstrTable::lookupLitstrId(Id id) const {
  assert(m_safeToRead);
  assert(id >= 0 && id < Id(s_litstrTable->m_litstrs.size()));
  return const_cast<StringData*>(s_litstrTable->m_litstrs[id]);
}

inline
const NamedEntity* LitstrTable::lookupNamedEntityId(Id id) const {
  assert(m_safeToRead);
  return lookupNamedEntityPairId(id).second;
}

inline
const NamedEntityPair& LitstrTable::lookupNamedEntityPairId(Id id) const {
  assert(m_safeToRead);
  assert(id >= 0 && id < Id(s_litstrTable->m_namedInfo.size()));
  const NamedEntityPair& ne = s_litstrTable->m_namedInfo[id];
  assert(ne.first);
  assert(ne.first->data()[ne.first->size()] == 0);
  assert(ne.first->data()[0] != '\\');
  if (UNLIKELY(!ne.second)) {
    const_cast<const NamedEntity*&>(ne.second) = Unit::GetNamedEntity(ne.first);
  }
  return ne;
}

//////////////////////////////////////////////////////////////////////

class UnitRepoProxy : public RepoProxy {
  friend class Unit;
  friend class UnitEmitter;
 public:
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
                int ix, UnitMergeKind kind,
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

//////////////////////////////////////////////////////////////////////

struct ConstPreClassMethodRanger {
  typedef Func* const* Iter;
  typedef const Func* Value;
  static Iter get(PreClassPtr pc) {
    return pc->methods();
  }
};

struct MutablePreClassMethodRanger {
  typedef Func** Iter;
  typedef Func* Value;
  static Func** get(PreClassPtr pc) {
    return pc->mutableMethods();
  }
};

template<typename FuncRange,
         typename GetMethods>
struct AllFuncsImpl {
  explicit AllFuncsImpl(const Unit* unit)
    : fr(unit->funcs())
    , mr(0, 0)
    , cr(unit->preclasses())
  {
    if (fr.empty()) skip();
  }
  bool empty() const { return fr.empty() && mr.empty() && cr.empty(); }
  typedef typename GetMethods::Value FuncPtr;
  FuncPtr front() const {
    assert(!empty());
    if (!fr.empty()) return fr.front();
    assert(!mr.empty());
    return mr.front();
  }
  FuncPtr popFront() {
    FuncPtr f = !fr.empty() ? fr.popFront() :
      !mr.empty() ? mr.popFront() : 0;
    assert(f);
    if (fr.empty() && mr.empty()) skip();
    return f;
  }

private:
  void skip() {
    assert(fr.empty());
    while (!cr.empty() && mr.empty()) {
      PreClassPtr c = cr.popFront();
      mr = Unit::FuncRange(GetMethods::get(c),
                           GetMethods::get(c) + c->numMethods());
    }
  }

  Unit::FuncRange fr;
  Unit::FuncRange mr;
  Unit::PreClassRange cr;
};

typedef AllFuncsImpl<Unit::FuncRange,ConstPreClassMethodRanger> AllFuncs;
typedef AllFuncsImpl<Unit::MutableFuncRange,MutablePreClassMethodRanger>
  MutableAllFuncs;

/*
 * Range over all defined classes.
 */
class AllClasses {
  NamedEntityMap::iterator m_next, m_end;
  Class* m_current;
  void next();
  void skip();

public:
  AllClasses();
  bool empty() const;
  Class* front() const;
  Class* popFront();
};

//////////////////////////////////////////////////////////////////////

}
#endif
