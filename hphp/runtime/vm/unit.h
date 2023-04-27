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

#include "hphp/runtime/base/location.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/req-bitset.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/constant.h"
#include "hphp/runtime/vm/containers.h"
#include "hphp/runtime/vm/decl-dep.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/module.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/source-location.h"
#include "hphp/runtime/vm/type-alias.h"

#include "hphp/util/check-size.h"
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

enum MergeTypes : uint8_t {
  None        = 0,
  Function    = 1 << 0,
  NotFunction = 1 << 1,
  Everything  = 3,
};

constexpr MergeTypes operator&(MergeTypes a, MergeTypes b) { return MergeTypes((int)a & (int)b); }
constexpr MergeTypes operator^(MergeTypes a, MergeTypes b) { return MergeTypes((int)a ^ (int)b); }
constexpr MergeTypes operator~(MergeTypes a) { return MergeTypes((int)MergeTypes::Everything - (int)a); }

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

using RATArrayOrToken = TokenOrPtr<const RepoAuthType::Array>;

/*
 * Sum of all Unit::m_bclen
 */
extern ServiceData::ExportedCounter* g_hhbc_size;

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
    InitialMerged = 1,
    Merged        = 2,
  };

public:
  /*
   * Range types.
   */
  using PreClassPtrVec = VMCompactVector<PreClassPtr>;
  using TypeAliasVec = VMCompactVector<PreTypeAlias>;
  using ConstantVec = VMFixedVector<Constant>;
  using FuncVec = VMCompactVector<Func*>;
  using ModuleVec = VMCompactVector<Module>;

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
   * Serial number.
   */
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

  /*
   * When Eval.EnableDecl is set we may have multiple versions of the same unit
   * cached under a particular SHA-1 because of differing versions of
   * dependencies in different repos.
   */
  Unit* nextCachedByHash() const;
  void setNextCachedByHash(Unit* u);

  /////////////////////////////////////////////////////////////////////////////
  // Idle unit reaping

  using TouchClock = std::chrono::steady_clock;

  /*
   * Mark that this Unit has been touched by the given request.
   */
  void setLastTouchRequest(int64_t request);

  /*
   * Mark that this Unit has been touched at the given timestamp.
   */
  void setLastTouchTime(TouchClock::time_point);

  /*
   * Get the newest request which has touched this Unit, and the
   * latest timestamp of the touch.
   */
  std::pair<int64_t, TouchClock::time_point> getLastTouch() const;

  /////////////////////////////////////////////////////////////////////////////
  // Code locations.                                                    [const]

  bool getOffsetRanges(int line, OffsetFuncRangeVec& offsets) const;

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
   * Record that the line was covered.
   */
  void recordCoverage(int line);

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
  // Litstrs and NamedTypes/Funcs.                                      [const]

  /*
   * Size of the Unit's litstr table.
   */
  size_t numLitstrs() const;

  /*
   * Lookup a literal string by ID.
   */
  StringData* lookupLitstrId(Id id) const;

  const NamedType* lookupNamedTypeId(Id id) const;
  const NamedFunc* lookupNamedFuncId(Id id) const;
  NamedTypePair lookupNamedTypePairId(Id id) const;
  NamedFuncPair lookupNamedFuncPairId(Id id) const;

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

  /////////////////////////////////////////////////////////////////////////////
  // RAT Arrays.

  /*
   * Look up a RAT array by ID.
   */
  const RepoAuthType::Array* lookupRATArray(Id id) const;

  /////////////////////////////////////////////////////////////////////////////
  // PreClasses.

  PreClass* lookupPreClass(const StringData*) const;

  folly::Range<PreClassPtr*> preclasses();
  folly::Range<const PreClassPtr*> preclasses() const;

  /////////////////////////////////////////////////////////////////////////////
  // Funcs.

  Func* lookupFuncId(Id id) const;
  folly::Range<Func**> funcs();
  folly::Range<Func* const*> funcs() const;

  /*
   * Return the cached EntryPoint
   */
  Func* getEntryPoint() const;

  /*
   * Visit all functions and methods in this unit.
   */
  template<class Fn> void forEachFunc(Fn fn) const;

  /////////////////////////////////////////////////////////////////////////////
  // Constants.

  const Constant* lookupConstantId(Id id) const;
  folly::Range<Constant*> constants();
  folly::Range<const Constant*> constants() const;

  /////////////////////////////////////////////////////////////////////////////
  // Modules.

  const Module* lookupModuleId(Id id) const;
  folly::Range<Module*> modules();
  folly::Range<const Module*> modules() const;

  /////////////////////////////////////////////////////////////////////////////
  // Type aliases.

  const PreTypeAlias* lookupTypeAliasId(Id id) const;
  folly::Range<PreTypeAlias*> typeAliases();
  folly::Range<const PreTypeAlias*> typeAliases() const;

  /////////////////////////////////////////////////////////////////////////////
  // File attributes.

  const UserAttributeMap& fileAttributes() const;

  /////////////////////////////////////////////////////////////////////////////
  // Modules.

  const StringData* moduleName() const;

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

  bool isSystemLib() const;

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
   * Log any units which were changed on disk after having been loaded by the
   * current request.
   */
  void logTearing(int64_t nsecs);

  /*
   * Log information about decls observed during compilation and any potential
   * tearing that may be occurring.
   *
   * For a given unit `U' we say that:
   *   - rdep tearing has occurred if we previously loaded decls from different
   *     version of U in the same request. We have `torn' a reverse-dependency
   *     of U.
   *   - dep tearing has occurred if for a dependency `D' of U, we had
   *     previously loaded bytecode from a different version of `D' in the same
   *     request.
   */
  void logDeclInfo() const;

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

  // Total number of Units ever created
  static size_t createdUnitCount() { return s_createdUnits; }

  // Total number of currently allocated Units
  static size_t liveUnitCount() { return s_liveUnits; }

  static constexpr ptrdiff_t moduleNameOff() {
    return offsetof(Unit, m_moduleName);
  }

  const std::vector<DeclDep>& deps() const { return m_deps; }

  /////////////////////////////////////////////////////////////////////////////
  // Internal methods.

private:
  void initialMerge();
  void mergeImpl(MergeTypes mergeTypes);
  UnitExtended* getExtended();
  const UnitExtended* getExtended() const;

  /////////////////////////////////////////////////////////////////////////////
  // Data members.
  //
  // These are organized in reverse order of frequency of use.  Do not re-order
  // without checking perf!
private:
  LowStringPtr m_origFilepath{nullptr};

  /*
   * m_mergeState is read without a lock, but only written to under
   * unitInitLock (see unit.cpp).
   */
  std::atomic<uint8_t> m_mergeState{MergeState::Unmerged};
  bool m_interpretOnly : 1;
  bool m_extended : 1;
  bool m_ICE : 1; // was this unit the result of an internal compiler error

  FuncVec m_funcs;
  PreClassPtrVec m_preClasses;
  TypeAliasVec m_typeAliases;
  ConstantVec m_constants;
  ModuleVec m_modules;

  mutable VMCompactVector<UnsafeLockFreePtrWrapper<StringOrToken>> m_litstrs;
  mutable VMCompactVector<UnsafeLockFreePtrWrapper<ArrayOrToken>> m_arrays;
  mutable VMCompactVector<UnsafeLockFreePtrWrapper<RATArrayOrToken>> m_rats;

  /*
   * The remaining fields are cold, and arbitrarily ordered.
   */

  hphp_fast_map<
    const StringData*,
    PreClassPtr,
    string_data_hash,
    string_data_isame
  > m_nameToPreClass; // Lookup PreClass by name

  int64_t m_sn{-1};             // Note: could be 32-bit
  SHA1 m_sha1;
  SHA1 m_bcSha1;
  UserAttributeMap m_metaData;
  UserAttributeMap m_fileAttributes;
  std::unique_ptr<FatalInfo> m_fatalInfo{nullptr};
  const StringData* m_moduleName{nullptr};
  std::vector<DeclDep> m_deps;

  rds::Link<req::dynamic_bitset, rds::Mode::Normal> m_coverage;

  Id m_entryPointId{kInvalidId};

  static std::atomic<size_t> s_createdUnits;
  static std::atomic<size_t> s_liveUnits;
};

static_assert(CheckSize<Unit, use_lowptr ? 216 : 224>(), "");

struct UnitExtended : Unit {
  friend struct Unit;
  friend struct UnitEmitter;

  UnitExtended() { m_extended = true; }

  // Used by Unit prefetcher:
  SymbolRefs m_symbolRefsForPrefetch;
  std::atomic_flag m_symbolRefsPrefetched;

  std::atomic<int64_t> m_cacheRefCount{0};

  // Used by Eval.ReuseUnitsByHash:
  rds::Link<LowStringPtr, rds::Mode::Normal> m_perRequestFilepath;

  // Used by Eval.IdleUnitTimeoutSecs:
  std::atomic<int64_t> m_lastTouchRequest{0};
  std::atomic<TouchClock::time_point> m_lastTouchTime{TouchClock::time_point{}};

  std::atomic<Unit*> m_nextCachedByHash{nullptr};
};

static_assert(CheckSize<UnitExtended, use_lowptr ? 272 : 280>(), "");

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_UNIT_INL_H_
#include "hphp/runtime/vm/unit-inl.h"
#undef incl_HPHP_VM_UNIT_INL_H_
