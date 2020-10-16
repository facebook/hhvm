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

#pragma once

#include "hphp/parser/location.h"

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/req-bitset.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/constant.h"
#include "hphp/runtime/vm/containers.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/named-entity-pair-table.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/record.h"
#include "hphp/runtime/vm/source-location.h"
#include "hphp/runtime/vm/type-alias.h"

#include "hphp/util/compact-vector.h"
#include "hphp/util/fixed-vector.h"
#include "hphp/util/functional.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/lock-free-ptr-wrapper.h"
#include "hphp/util/mutex.h"
#include "hphp/util/service-data.h"
#include "hphp/util/sha1.h"

#include <map>
#include <ostream>
#include <string>
#include <vector>
#include <atomic>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Array;
struct ArrayData;
struct Class;
struct Func;
struct PreClass;
struct String;
struct StringData;
struct UnitExtended;

///////////////////////////////////////////////////////////////////////////////
// Unit enums.

/*
 * Where was a given Unit defined from?
 */
enum class UnitOrigin {
  File = 0,
  Eval = 1
};

/////////////////////////////////////////////////////////////////////////////
//
// Fatal info

/*
 * Information regarding parse/runtime errors in units
 */
struct FatalInfo {
  Location::Range m_fatalLoc;
  FatalOp m_fatalOp;
  std::string m_fatalMsg;
};

/////////////////////////////////////////////////////////////////////////////
//
// Symbol references

// Symbols referenced by an Unit. Used during prod builds to
// automatically important files, and in Sandboxes to prefetch units.
enum class SymbolRef : uint8_t {
  Include,
  Class,
  Function,
  Constant
};

using SymbolRefs =
  CompactVector<std::pair<SymbolRef, CompactVector<std::string>>>;

/*
 * Table specializations.
 */
using FuncTable      = VMCompactVector<const Func*>;

/*
 * Sum of all Unit::m_bclen
 */
extern ServiceData::ExportedTimeSeries* g_hhbc_size;

///////////////////////////////////////////////////////////////////////////////

/*
 * Metadata about a compilation unit.
 *
 * Contains the list of PreClasses and global functions, along with a special
 * function called the 'pseudomain', which is logically invoked (modulo
 * optimizations that avoid it) during execution when the unit is included or
 * required.
 */
struct Unit {
  friend struct UnitExtended;
  friend struct UnitEmitter;
  friend struct UnitRepoProxy;

  /////////////////////////////////////////////////////////////////////////////
  // Types.

private:
  /*
   * The Unit's current merge state.
   *
   * Merging is the process by which functions, classes, and constants defined
   * in a pseudomain are added to the unit in advance (or, optimistically,
   * instead of) running the pseudomain's code.  This is necessary for
   * correctness in a number of cases---e.g., toplevel functions defined in the
   * pseudomain need to be available before the line where the definition
   * occurs.
   *
   * Whenever we want to evaluate a Unit, we call merge() on it, and then
   * invoke its pseudomain only if necessary.
   */
  enum MergeState : uint8_t {
    Unmerged      = 0,
    Merging       = 1 << 0,
    Merged        = 1 << 1,
    UniqueFuncs   = 1 << 2,
    NeedsCompact  = 1 << 3,
    Empty         = 1 << 5
  };

public:
  /*
   * Information on all the mergeable defs within a Unit.
   *
   * Allocated with a variable-length pointer array in m_mergeables, structured
   * as follows:
   *  - the Unit's pseudomain
   *  - hoistable functions (i.e., toplevel functions that need to be available
   *    from the beginning of the pseudomain)
   *  - all other mergeable objects, with the bottom three bits of the pointer
   *    tagged with a MergeKind
   */
  struct MergeInfo {
    using FuncRange = folly::Range<Func* const*>;
    using MutableFuncRange = folly::Range<Func**>;

    /*
     * Allocate a new MergeInfo with `num' mergeables.
     */
    static MergeInfo* alloc(size_t num);

    /*
     * Iterators.
     *
     * funcNonMain() is in (funcBegin, funcEnd].
     */
    Func** funcBegin() const;
    Func** funcEnd() const;

    /*
     * Ranges.
     *
     * All ranges end at funcEnd().
     */
    FuncRange funcs() const;
    MutableFuncRange mutableFuncs() const;

    /*
     * Get a reference or pointer to the mergeable at index `idx'.
     */
    void*& mergeableObj(int idx);

    unsigned m_firstHoistablePreClass;
    unsigned m_firstMergeablePreClass;
    unsigned m_mergeablesSize;
    void*    m_mergeables[1];
  };

  /*
   * Type of a mergeable object.
   *
   * This is encoded in the lowest three bits of a pointer to the object.
   */
  enum class MergeKind {
    Class               = 0,  // Class is required to be 0 for correctness.
    UniqueDefinedClass  = 1,
    Define              = 2,  // Toplevel scalar define.
    TypeAlias           = 3,
    Record              = 4,
    Done                = 5,
    // We can add two more kind here; this has to fit in 3 bits.
  };

  /*
   * Range types.
   */
  using FuncRange = MergeInfo::FuncRange;
  using MutableFuncRange = MergeInfo::MutableFuncRange;
  using PreClassPtrVec = VMCompactVector<PreClassPtr>;
  using TypeAliasVec = VMCompactVector<PreTypeAlias>;
  using ConstantVec = VMFixedVector<Constant>;

  /////////////////////////////////////////////////////////////////////////////
  // Construction and destruction.

  Unit();
  virtual ~Unit();

  /*
   * New and delete using low memory.
   */
  void* operator new(size_t sz);
  void operator delete(void* p, size_t sz);

  /////////////////////////////////////////////////////////////////////////////
  // Basic accessors.                                                   [const]

  /*
   * Repo ID and serial number.
   */
  int repoID() const;
  int64_t sn() const;

  /*
   * SHA1 of the source code for Unit.
   */
  SHA1 sha1() const;

  /*
   * SHA1 of the bytecode for Unit.
   */
  SHA1 bcSha1() const;

  /*
   * Was this unit created in response to an internal compiler error?
   */
  bool isICE() const;

  /////////////////////////////////////////////////////////////////////////////
  // File paths.

  /*
   * Obtain the filepath this Unit was original created from. This is
   * guaranteed to be the same across all requests and never
   * change. Use this when you're using the path as a key that
   * shouldn't change.
   */
  const StringData* origFilepath() const;

  /*
   * Obtain the filepath this Unit is currently bound to in this
   * request, or nullptr if none.
   */
  const StringData* perRequestFilepath() const;

  /*
   * If this Unit has a per-request filepath bound to it, return
   * it. Otherwise return origFilepath(). Use this for displaying the
   * path to the user.
   */
  const StringData* filepath() const;

  /*
   * If this Unit is using per-request file paths, return the
   * rds::Handle where the path is bound. If you're in an active
   * request, the handle is guaranteed to be initialized.
   */
  rds::Handle perRequestFilepathHandle() const;

  /*
   * If this Unit is using per-request file paths, return true. False
   * otherwise.
   */
  bool hasPerRequestFilepath() const;

  /*
   * Bind the given filepath as the Unit's pre-request filepath. The
   * Unit should not have an already bound filepath and
   * EvalReuseUnitsByHash should be set.
   */
  void bindPerRequestFilepath(const StringData*);

  /*
   * Mark this Unit as having per-request filepaths. This allocates a
   * new rds handle to store the path. EvalReuseUnitsByHash must be
   * set.
   */
  void makeFilepathPerRequest();

  /////////////////////////////////////////////////////////////////////////////
  // Unit cache ref-counting

  /*
   * Mark that this Unit is referenced by a cache entry. Only valid
   * for non-RepoAuth mode.
   */
  void acquireCacheRefCount();

  /*
   * Mark that this Unit is no longer referenced by a cache
   * entry. Only valid for non-RepoAuth mode. Returns true if the Unit
   * is no longer referenced by an entries and should be deleted,
   * false otherwise.
   */
  bool releaseCacheRefCount();

  /*
   * Whether this Unit is referenced by a cache entry. Only valid for
   * non-RepoAuth mode.
   */
  bool hasCacheRef() const;

  /////////////////////////////////////////////////////////////////////////////
  // Bytecode.                                                          [const]

  /*
   * Start and size of the bytecode for the Unit.
   */
  PC entry() const;
  Offset bclen() const;

  /////////////////////////////////////////////////////////////////////////////
  // Code locations.                                                    [const]

  /*
   * Get the line number corresponding to `pc'.
   *
   * Return -1 if not found.
   */
  int getLineNumber(Offset pc) const;

  /*
   * Get the SourceLoc corresponding to `pc'.
   *
   * Return false if not found, else true.
   */
  bool getSourceLoc(Offset pc, SourceLoc& sLoc) const;

  /*
   * Get the Offset range(s) corresponding to `pc' or `line'.
   *
   * Return false if not found, else true.
   */
  bool getOffsetRange(Offset pc, OffsetRange& range) const;
  bool getOffsetRanges(int line, OffsetRangeVec& offsets) const;

  /*
   * Get next line with executable code starting from input `line'.
   *
   * Return -1 if not found.
   */
  int getNearestLineWithCode(int line) const;

  /*
   * Return the Func* for the code at offset `pc'.
   *
   * Return nullptr if the offset is not in a Func body (but this should be
   * impossible).
   */
  const Func* getFunc(Offset pc) const;

  /*
   * Check whether the coverage map has been enabled for this unit.
   */
  bool isCoverageEnabled() const;

  /*
   * Enable or disable the coverage map for this unit.
   *
   * Pre: !RO::RepoAuthoritative && RO::EvalEnablePerFileCoverage
   */
  void enableCoverage();
  void disableCoverage();

  /*
   * Reset the coverage map.
   *
   * Pre: isCoverageEnabled()
   */
  void clearCoverage();

  /*
   * Record that the bytecode at off was covered.
   */
  void recordCoverage(Offset off);

  /*
   * Generate a vec array where each entry is an integer indicating a covered
   * line from this file.
   */
  Array reportCoverage() const;

  /*
   * Return an RDS handle that when initialized indicates that coverage is
   * enabled for this unit.
   *
   * Pre: !RO::RepoAuthoritative && RO::EvalEnablePerFileCoverage
   */
  rds::Handle coverageDataHandle() const;

  /////////////////////////////////////////////////////////////////////////////
  // Litstrs and NamedEntitys.                                          [const]

  /*
   * Size of the Unit's litstr table.
   *
   * This excludes litstrs that are instead found in the global table---thus,
   * it is not a source of truth for the number of litstrs a Unit needs, only
   * those it happens to own.
   */
  size_t numLitstrs() const;

  /*
   * Is `id' a valid litstr in LitstrTable or the Unit's local
   * NamedEntityPairTable?
   */
  bool isLitstrId(Id id) const;

  /*
   * Dispatch to either the global LitstrTable or the Unit's local
   * NamedEntityPairTable, depending on whether `id' is global.
   *
   * @see: NamedEntityPairTable
   */
  StringData* lookupLitstrId(Id id) const;
  const NamedEntity* lookupNamedEntityId(Id id) const;
  NamedEntityPair lookupNamedEntityPairId(Id id) const;

  /////////////////////////////////////////////////////////////////////////////
  // Arrays.                                                            [const]

  /*
   * Size of the Unit's scalar array table.
   */
  size_t numArrays() const;

  /*
   * Look up a scalar array by ID.
   */
  const ArrayData* lookupArrayId(Id id) const;

  /*
   * Look up a RepoAuthType::Array by ID
   */
   const RepoAuthType::Array* lookupArrayTypeId(Id id) const;

  /////////////////////////////////////////////////////////////////////////////
  // Funcs, PreClasses, and RecordDescs.                                [const]

  /*
   * Look up a Func or PreClass or PreRecordDesc by ID.
   */
  Func* lookupFuncId(Id id) const;
  PreClass* lookupPreClassId(Id id) const;
  PreRecordDesc* lookupPreRecordId(Id id) const;

  /*
   * Range over all Funcs or PreClasses or RecordDescs in the Unit.
   */
  FuncRange funcs() const;
  folly::Range<PreClassPtr*> preclasses();
  folly::Range<const PreClassPtr*> preclasses() const;
  folly::Range<PreRecordDescPtr*> prerecords();
  folly::Range<const PreRecordDescPtr*> prerecords() const;

  // Return the cached EntryPoint
  Func* getCachedEntryPoint() const;

  /*
   * Visit all functions and methods in this unit.
   */
  template<class Fn> void forEachFunc(Fn fn) const;

  /////////////////////////////////////////////////////////////////////////////
  // RecordDesc lookup.                                               [static]

  /*
   * Define a new RecordDesc from `record' for this request.
   *
   * Raises a fatal error in various conditions (e.g., RecordDesc already
   * defined, etc.) if `failIsFatal' is set).
   *
   * Also always fatals if a type alias already exists in this request with the
   * same name as that of `record', regardless of the value of `failIsFatal'.
   */
  static RecordDesc* defRecordDesc(PreRecordDesc* record,
                                   bool failIsFatal = true);

  /*
   * Look up the RecordDesc in this request with name `name', or with the name
   * mapped to the NamedEntity `ne'.
   *
   * Return nullptr if the record is not yet defined in this request.
   */
  static RecordDesc* lookupRecordDesc(const NamedEntity* ne);
  static RecordDesc* lookupRecordDesc(const StringData* name);

  /*
   * Finds a record which is guaranteed to be unique.
   * The record has not necessarily been loaded in the current request.
   *
   * Return nullptr if there is no such record.
   */
  static const RecordDesc* lookupUniqueRecDesc(const StringData* name);

  /*
   * Autoload the RecordDesc with name `name' and bind it `ne' in this request.
   *
   * @requires: NamedEntity::get(name) == ne
   */
  static RecordDesc* loadMissingRecordDesc(const NamedEntity* ne,
                                           const StringData* name);

  /*
   * Same as lookupRecordDesc(), but if `tryAutoload' is set, call and return
   * loadMissingRecordDesc().
   */
  static RecordDesc* getRecordDesc(const NamedEntity* ne,
                                   const StringData* name,
                                   bool tryAutoload);
  static RecordDesc* getRecordDesc(const StringData* name, bool tryAutoload);

  /////////////////////////////////////////////////////////////////////////////
  // Constants.

  folly::Range<Constant*> constants();
  folly::Range<const Constant*> constants() const;

  /*
   * Define the constant given by `id'
   */
  void defCns(Id id);

  static Variant getCns(const StringData* name);

  /////////////////////////////////////////////////////////////////////////////
  // Constant lookup.                                                  [static]

  /*
   * Look up the value of the defined constant in this request with name
   * `cnsName'.
   *
   * Return nullptr if no such constant is defined.
   */
  static TypedValue lookupCns(const StringData* cnsName);

  /*
   * Look up the value of the persistent constant with name `cnsName'.
   *
   * Return nullptr if no such constant exists, or the constant is not
   * persistent.
   */
  static const TypedValue* lookupPersistentCns(const StringData* cnsName);

  /*
   * Look up, or autoload and define, the value of the constant with name
   * `cnsName' for this request.
   */
  static TypedValue loadCns(const StringData* cnsName);

  /*
   * Define a constant with name `cnsName' with a magic callback. The
   * TypedValue should be KindOfUninit, with a Native::ConstantCallback in
   * its m_data.pcnt.
   *
   * The canonical examples are STDIN, STDOUT, and STDERR.
   */
  static bool defNativeConstantCallback(const StringData* cnsName, TypedValue cell);

  /////////////////////////////////////////////////////////////////////////////
  // Type aliases.

  folly::Range<PreTypeAlias*> typeAliases();
  folly::Range<const PreTypeAlias*> typeAliases() const;

  /*
   * Look up without autoloading a type alias named `name'. Returns nullptr
   * if one cannot be found.
   *
   * If the type alias is found and `persistent' is provided, it will be set to
   * whether or not the TypeAlias's RDS handle is persistent.
   */
  static const TypeAlias* lookupTypeAlias(const StringData* name,
                                          bool* persistent = nullptr);

  /*
   * Look up or attempt to autoload a type alias named `name'. Returns nullptr
   * if one cannot be found or autoloaded.
   *
   * If the type alias is found and `persistent' is provided, it will be set to
   * whether or not the TypeAlias's RDS handle is persistent.
   */
  static const TypeAlias* loadTypeAlias(const StringData* name,
                                        bool* persistent = nullptr);

  /*
   * Define the type alias given by `id', binding it to the appropriate
   * NamedEntity for this request.
   *
   * Raises a fatal error if type alias already defined or cannot be defined
   * unless failIsFatal is unset
   *
   * Returns:
   *   Persistent: Type alias is successfully defined and is persistent
   *   Normal: Type alias is successfully defined and is not persistent
   *   Fail: Type alias is not successfully defined
   */
  enum class DefTypeAliasResult { Fail, Normal, Persistent };
  Unit::DefTypeAliasResult defTypeAlias(Id id, bool failIsFatal = true);

  /////////////////////////////////////////////////////////////////////////////
  // File attributes.

  const UserAttributeMap& fileAttributes() const;

  /////////////////////////////////////////////////////////////////////////////
  // Merge.

  /*
   * Merge the Unit if it is not already merged.
   */
  void merge();

  /*
   * Is this Unit empty---i.e., does it define nothing and have no
   * side-effects?
   */
  bool isEmpty() const;

  /////////////////////////////////////////////////////////////////////////////
  // Info arrays.                                                      [static]

  /*
   * Generate class info arrays.
   */
  static Array getClassesInfo();
  static Array getInterfacesInfo();
  static Array getTraitsInfo();

  /*
   * Generate function info arrays.
   */
  static Array getUserFunctions();
  static Array getSystemFunctions();

  /////////////////////////////////////////////////////////////////////////////
  // Pretty printer.                                                    [const]

  std::string toString() const;

  /////////////////////////////////////////////////////////////////////////////
  // Other methods.

  /*
   * Get or set whether this Unit is interpret-only.
   *
   * This is used by the debugger to signal to the JIT that eval'd commands
   * should not be jitted.
   */
  bool isInterpretOnly() const;
  void setInterpretOnly();

  /*
   * Get parse/runtime failure information if this unit is created as
   * a result of one.
   */
  const FatalInfo* getFatalInfo() const;

  UserAttributeMap metaData() const;

  /*
   * Atomically "claim" the symbol refs in this Unit for
   * prefetching. Returns nullptr if no symbol refs are present, or if
   * they have already been claimed. If a valid pointer is returned,
   * you have exclusive access to the symbol refs and no future call
   * to this function will return them.
   */
  SymbolRefs* claimSymbolRefsForPrefetch();

  // Return true, and set the m_serialized flag, iff this Unit hasn't
  // been serialized yet (see prof-data-serialize.cpp).
  bool serialize() const {
    if (m_serialized) return false;
    const_cast<Unit*>(this)->m_serialized = true;
    return true;
  }

  // Total number of currently allocated Units
  static size_t liveUnitCount() { return s_liveUnits; }

  /////////////////////////////////////////////////////////////////////////////
  // Offset accessors.                                                 [static]

  static constexpr ptrdiff_t bcOff() {
    return offsetof(Unit, m_bc);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Internal methods.

private:
  void initialMerge();
  template<bool debugger>
  void mergeImpl(MergeInfo* mi);
  UnitExtended* getExtended();
  const UnitExtended* getExtended() const;
  MergeInfo* mergeInfo() const {
    return m_mergeInfo.load(std::memory_order_acquire);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Data members.
  //
  // These are organized in reverse order of frequency of use.  Do not re-order
  // without checking perf!
private:
  unsigned char const* m_bc{nullptr};
  Offset m_bclen{0};
  LowStringPtr m_origFilepath{nullptr};
  std::atomic<MergeInfo*> m_mergeInfo{nullptr};

  int8_t m_repoId{-1};
  /*
   * m_mergeState is read without a lock, but only written to under
   * unitInitLock (see unit.cpp).
   */
  std::atomic<uint8_t> m_mergeState{MergeState::Unmerged};
  bool m_interpretOnly : 1;
  bool m_extended : 1;
  bool m_serialized : 1;
  bool m_ICE : 1; // was this unit the result of an internal compiler error

  PreClassPtrVec m_preClasses;
  TypeAliasVec m_typeAliases;
  ConstantVec m_constants;
  CompactVector<PreRecordDescPtr> m_preRecords;
  /*
   * Cached the EntryPoint for an unit, since compactMergeInfo() inside of
   * mergeImpl will drop the original EP.
   */
  Func* m_cachedEntryPoint{nullptr};

  /*
   * The remaining fields are cold, and arbitrarily ordered.
   */

  int64_t m_sn{-1};             // Note: could be 32-bit
  SHA1 m_sha1;
  SHA1 m_bcSha1;
  VMFixedVector<const ArrayData*> m_arrays;
  mutable LockFreePtrWrapper<VMCompactVector<LineInfo>> m_lineMap;
  UserAttributeMap m_metaData;
  UserAttributeMap m_fileAttributes;
  std::unique_ptr<FatalInfo> m_fatalInfo{nullptr};

  rds::Link<req::dynamic_bitset, rds::Mode::Normal> m_coverage;

  static std::atomic<size_t> s_liveUnits;
};

struct UnitExtended : Unit {
  friend struct Unit;
  friend struct UnitEmitter;

  UnitExtended() { m_extended = true; }

  NamedEntityPairTable m_namedInfo;
  ArrayTypeTable m_arrayTypeTable;
  FuncTable m_funcTable;

  // Used by Unit prefetcher:
  SymbolRefs m_symbolRefsForPrefetch;
  std::atomic_flag m_symbolRefsPrefetched;

  std::atomic<int64_t> m_cacheRefCount{0};

  // Used by Eval.ReuseUnitsByHash:
  rds::Link<LowStringPtr, rds::Mode::Normal> m_perRequestFilepath;
};

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_UNIT_INL_H_
#include "hphp/runtime/vm/unit-inl.h"
#undef incl_HPHP_VM_UNIT_INL_H_
