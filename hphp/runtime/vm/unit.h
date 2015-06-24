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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/named-entity-pair-table.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/type-alias.h"

#include "hphp/util/fixed-vector.h"
#include "hphp/util/functional.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/util/md5.h"
#include "hphp/util/mutex.h"
#include "hphp/util/range.h"

#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Array;
struct ArrayData;
struct Class;
struct Func;
struct PreClass;
struct String;
struct StringData;

///////////////////////////////////////////////////////////////////////////////
// Unit enums.

/*
 * Where was a given Unit defined from?
 */
enum class UnitOrigin {
  File = 0,
  Eval = 1
};

///////////////////////////////////////////////////////////////////////////////
// Location tables.

/*
 * Delimiter pairs for a location in the source code.
 */
struct SourceLoc {
  /*
   * Constructors.
   */
  SourceLoc() {}
  explicit SourceLoc(const Location::Range& l);

  /*
   * Reset to, or check for, the invalid state.
   */
  void reset();
  bool valid() const;

  /*
   * Set to a parser Location.
   */
  void setLoc(const Location::Range* l);

  /*
   * Equality.
   */
  bool same(const SourceLoc* l) const;
  bool operator==(const SourceLoc& l) const;

  /*
   * Start and end lines and characters.
   *
   * The default {1, 1, 1, 1} is an invalid sentinel value.
   */
  int line0{1};
  int char0{1};
  int line1{1};
  int char1{1};
};

/*
 * Pair of (base, past) offsets.
 */
struct OffsetRange {
  OffsetRange() {}

  OffsetRange(Offset base, Offset past)
    : base(base)
    , past(past)
  {}

  Offset base{0};
  Offset past{0};
};

using OffsetRangeVec = std::vector<OffsetRange>;

/*
 * Generic entry for representing many-to-one mappings of Offset -> T.
 *
 * Each entry's `pastOffset' is expected to be the offset just past the range
 * of offsets which logically map to its `val'.  In this way, by maintaining a
 * relatively sparse set of entries in a vector, we can use least upper bound
 * searches on an offset key to find its corresponding T.
 *
 * The values of `pastOffset' in such a table are expected to be sorted and
 * unique, but the values of `val' need not be.
 */
template<typename T>
struct TableEntry {
  /*
   * Constructors.
   */
  TableEntry() {}

  TableEntry(Offset pastOffset, T val)
    : m_pastOffset(pastOffset)
    , m_val(val)
  {}

  /*
   * Accessors.
   */
  Offset pastOffset() const;
  T val() const;

  /*
   * Comparison.
   */
  bool operator<(const TableEntry& other) const;

  template<class SerDe> void serde(SerDe& sd);

private:
  Offset m_pastOffset{0};
  T m_val;
};

/*
 * Table specializations.
 */
using LineEntry      = TableEntry<int>;
using SourceLocEntry = TableEntry<SourceLoc>;
using FuncEntry      = TableEntry<const Func*>;

using LineTable      = std::vector<LineEntry>;
using SourceLocTable = std::vector<SourceLocEntry>;
using FuncTable      = std::vector<FuncEntry>;

/*
 * Get the line number or SourceLoc for Offset `pc' in `table'.
 */
int getLineNumber(const LineTable& table, Offset pc);
bool getSourceLoc(const SourceLocTable& table, Offset pc, SourceLoc& sLoc);
void stashLineTable(const Unit* unit, LineTable table);

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
  friend class UnitEmitter;
  friend class UnitRepoProxy;

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
   *  - non-hoistable functions that might be DefFunc'd in the pseudomain
   *  - hoistable functions (i.e., toplevel functions that need to be available
   *    from the beginning of the pseudomain)
   *  - all other mergeable objects, with the bottom three bits of the pointer
   *    tagged with a MergeKind
   *
   * Note that the non-hoistable function list may include functions which are
   * not mergeable, since DefFunc also uses this list as its mapping from ID's.
   */
  struct MergeInfo {
    typedef IterRange<Func* const*> FuncRange;
    typedef IterRange<Func**> MutableFuncRange;

    /*
     * Allocate a new MergeInfo with `num' mergeables.
     */
    static MergeInfo* alloc(size_t num);

    /*
     * Iterators.
     *
     * funcHoistableBegin() is in (funcBegin, funcEnd].
     */
    Func** funcBegin() const;
    Func** funcEnd() const;
    Func** funcHoistableBegin() const;

    /*
     * Ranges.
     *
     * All ranges end at funcEnd().
     */
    FuncRange funcs() const;
    MutableFuncRange mutableFuncs() const;
    MutableFuncRange nonMainFuncs() const;
    MutableFuncRange hoistableFuncs() const;

    /*
     * Get a reference or pointer to the mergeable at index `idx'.
     */
    void*& mergeableObj(int idx);
    void** mergeableData(int idx);

    unsigned m_firstHoistableFunc;
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
    PersistentDefine    = 3,  // Cross-request persistent toplevel defines.
    Global              = 4,  // Global variable declarations.
    // Unused           = 5,
    ReqDoc              = 6,
    Done                = 7,
    // We cannot add more kinds here; this has to fit in 3 bits.
  };

  /*
   * Range types.
   */
  typedef MergeInfo::FuncRange FuncRange;
  typedef MergeInfo::MutableFuncRange MutableFuncRange;
  typedef Range<std::vector<PreClassPtr>> PreClassRange;

  /*
   * Cache for pseudomains for this unit, keyed by Class context.
   */
  typedef hphp_hash_map<const Class*, Func*,
                        pointer_hash<Class>> PseudoMainCacheMap;


  /////////////////////////////////////////////////////////////////////////////
  // Construction and destruction.

  Unit();
  ~Unit();

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
   * MD5 of the Unit.
   */
  MD5 md5() const;

  /*
   * File and directory paths.
   */
  const StringData* filepath() const;
  const StringData* dirpath() const;


  /////////////////////////////////////////////////////////////////////////////
  // Bytecode.                                                          [const]

  /*
   * Start and size of the bytecode for the Unit.
   */
  PC entry() const;
  Offset bclen() const;

  /*
   * Convert between PC and Offset from entry().
   */
  PC at(Offset off) const;
  Offset offsetOf(PC pc) const;

  /*
   * Is `pc' in this Unit?
   */
  bool contains(PC pc) const;

  /*
   * Get the Op at `instrOffset'.
   */
  Op getOpcode(size_t instrOffset) const;


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
   * Return the Func* for the code at offset `pc'.
   *
   * Return nullptr if the offset is not in a Func body (but this should be
   * impossible).
   */
  const Func* getFunc(Offset pc) const;


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
  const NamedEntityPair& lookupNamedEntityPairId(Id id) const;


  /////////////////////////////////////////////////////////////////////////////
  // Arrays.                                                            [const]

  /*
   * Size of the Unit's scalar array table.
   */
  size_t numArrays() const;

  /*
   * Look up a scalar array by ID.
   */
  ArrayData* lookupArrayId(Id id) const;


  /////////////////////////////////////////////////////////////////////////////
  // Funcs and PreClasses.                                              [const]

  /*
   * Look up a Func or PreClass by ID.
   */
  Func* lookupFuncId(Id id) const;
  PreClass* lookupPreClassId(Id id) const;

  /*
   * Range over all Funcs or PreClasses in the Unit.
   */
  FuncRange funcs() const;
  PreClassRange preclasses() const;

  /*
   * Get a pseudomain for the Unit with the context class `cls'.
   *
   * We clone the toplevel pseudomain for each context class and cache the
   * results in m_pseudoMainCache.
   */
  Func* getMain(Class* cls = nullptr) const;

  /*
   * The first hoistable Func in the Unit.
   *
   * This is only used for the create_function() implementation, to access the
   * __lambda_func() we define for the user.
   */
  Func* firstHoistable() const;

  /*
   * Rename the Func in this Unit given by `oldName' to `newName'.
   *
   * This should only be called by ExecutionContext::createFunction().
   */
  void renameFunc(const StringData* oldName, const StringData* newName);


  /////////////////////////////////////////////////////////////////////////////
  // Func lookup.                                                      [static]

  /*
   * Look up the defined Func in this request with name `name', or with the
   * name mapped to the NamedEntity `ne'.
   *
   * Return nullptr if the function is not yet defined in this request.
   */
  static Func* lookupFunc(const NamedEntity* ne);
  static Func* lookupFunc(const StringData* name);

  /*
   * Look up, or autoload and define, the Func in this request with name
   * `name', or with the name mapped to the NamedEntity `ne'.
   *
   * @requires: NamedEntity::get(name) == ne
   */
  static Func* loadFunc(const NamedEntity* ne, const StringData* name);
  static Func* loadFunc(const StringData* name);

  /*
   * Load or reload `func'---i.e., bind (or rebind) it to the NamedEntity
   * corresponding to its name.
   */
  static void loadFunc(const Func* func);


  /////////////////////////////////////////////////////////////////////////////
  // Class lookup.                                                     [static]

  /*
   * Define a new Class from `preClass' for this request.
   *
   * Raises a fatal error in various conditions (e.g., Class already defined,
   * parent Class not defined, etc.) if `failIsFatal' is set).
   *
   * Also always fatals if a type alias already exists in this request with the
   * same name as that of `preClass', regardless of the value of `failIsFatal'.
   */
  static Class* defClass(const PreClass* preClass, bool failIsFatal = true);

  /*
   * Set the NamedEntity for `alias' to refer to the Class `original' in this
   * request.
   *
   * Raises a warning if `alias' already refers to a Class in this request.
   */
  static bool aliasClass(Class* original, const StringData* alias);

  /*
   * Look up the Class in this request with name `name', or with the name
   * mapped to the NamedEntity `ne'.
   *
   * Return nullptr if the class is not yet defined in this request.
   */
  static Class* lookupClass(const NamedEntity* ne);
  static Class* lookupClass(const StringData* name);

  /*
   * Same as lookupClass(), except that if the Class is not defined but is
   * unique, return it anyway.
   *
   * When jitting code before a unique class is defined, we can often still
   * burn the Class* into the TC, since it will be defined by the time the code
   * that needs the Class* runs (via autoload or whatnot).
   */
  static Class* lookupClassOrUniqueClass(const NamedEntity* ne);
  static Class* lookupClassOrUniqueClass(const StringData* name);

  /*
   * Look up, or autoload and define, the Class in this request with name
   * `name', or with the name mapped to the NamedEntity `ne'.
   *
   * @requires: NamedEntity::get(name) == ne
   */
  static Class* loadClass(const NamedEntity* ne, const StringData* name);
  static Class* loadClass(const StringData* name);

  /*
   * Autoload the Class with name `name' and bind it `ne' in this request.
   *
   * @requires: NamedEntity::get(name) == ne
   */
  static Class* loadMissingClass(const NamedEntity* ne, const StringData* name);

  /*
   * Same as lookupClass(), but if `tryAutoload' is set, call and return
   * loadMissingClass().
   */
  static Class* getClass(const NamedEntity* ne, const StringData* name,
                         bool tryAutoload);
  static Class* getClass(const StringData* name, bool tryAutoload);

  /*
   * Whether a Class with name `name' of type `kind' has been defined in this
   * request, autoloading it if `autoload' is set.
   */
  static bool classExists(const StringData* name,
                          bool autoload, ClassKind kind);


  /////////////////////////////////////////////////////////////////////////////
  // Constant lookup.                                                  [static]

  /*
   * Look up the value of the defined constant in this request with name
   * `cnsName'.
   *
   * Return nullptr if no such constant is defined.
   */
  static const Cell* lookupCns(const StringData* cnsName);

  /*
   * Look up the value of the persistent constant with name `cnsName'.
   *
   * Return nullptr if no such constant exists, or the constant is not
   * persistent.
   */
  static const Cell* lookupPersistentCns(const StringData* cnsName);

  /*
   * Look up, or autoload and define, the value of the constant with name
   * `cnsName' for this request.
   */
  static const Cell* loadCns(const StringData* cnsName);

  /*
   * Define a constant (either request-local or persistent) with name `cnsName'
   * and value `value'.
   *
   * May raise notices or warnings if a constant with the given name is already
   * defined or if value is invalid.
   */
  static bool defCns(const StringData* cnsName, const TypedValue* value,
                     bool persistent = false);

  using SystemConstantCallback = const Variant& (*)();
  /*
   * Define a constant with name `cnsName' which stores an arbitrary data
   * pointer in its TypedValue (with datatype KindOfUnit).
   *
   * The canonical examples are STDIN, STDOUT, and STDERR.
   */
  static bool defSystemConstantCallback(const StringData* cnsName,
                                        SystemConstantCallback callback);


  /////////////////////////////////////////////////////////////////////////////
  // Type aliases.

  /*
   * Define the type alias given by `id', binding it to the appropriate
   * NamedEntity for this request.
   */
  void defTypeAlias(Id id);


  /////////////////////////////////////////////////////////////////////////////
  // Merge.

  /*
   * Merge the Unit if it is not already merged.
   */
  void merge();

  /*
   * Is it sufficient to merge the Unit, and skip invoking its pseudomain?
   */
  bool isMergeOnly() const;

  /*
   * Is this Unit empty---i.e., does it define nothing and have no
   * side-effects?
   */
  bool isEmpty() const;

  /*
   * Get the return value of the pseudomain, or KindOfUnit if not known.
   *
   * @requires: isMergeOnly()
   */
  const TypedValue* getMainReturn() const;


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


  /////////////////////////////////////////////////////////////////////////////
  // Other methods.

  /*
   * Is this Unit a compile-time fatal?
   *
   * A compile-time fatal is encoded as a pseudomain that contains precisely:
   *
   *   String <id>; Fatal;
   *
   * Decode enough of pseudomain to determine whether it contains a
   * compile-time fatal, and if so, extract the error message and line number.
   *
   * Parse-time fatals are a subset of compile-time fatals.
   */
  bool compileTimeFatal(const StringData*& msg, int& line) const;
  bool parseFatal(const StringData*& msg, int& line) const;

  /*
   * Get or set whether this Unit is interpret-only.
   *
   * This is used by the debugger to signal to the JIT that eval'd commands
   * should not be jitted.
   */
  bool isInterpretOnly() const;
  void setInterpretOnly();

  /*
   * Replace the Unit?
   */
  void* replaceUnit() const;

  /*
   * Does this unit correspond to a file with "<?hh" at the top, irrespective of
   * EnableHipHopSyntax?
   */
  bool isHHFile() const;


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
  void mergeImpl(void* tcbase, MergeInfo* mi);


  /////////////////////////////////////////////////////////////////////////////
  // Data members.
  //
  // These are organized in reverse order of frequency of use.  Do not re-order
  // without checking perf!
private:
  unsigned char const* m_bc{nullptr};
  Offset m_bclen{0};
  LowStringPtr m_filepath{nullptr};
  MergeInfo* m_mergeInfo{nullptr};

  int8_t m_repoId{-1};
  /*
   * m_mergeState is read without a lock, but only written to under
   * unitInitLock (see unit.cpp).
   */
  uint8_t m_mergeState{MergeState::Unmerged};
  bool m_mergeOnly: 1;
  bool m_interpretOnly : 1;
  bool m_isHHFile : 1;
  LowStringPtr m_dirpath{nullptr};

  TypedValue m_mainReturn;
  std::vector<PreClassPtr> m_preClasses;
  FixedVector<TypeAlias> m_typeAliases;

  /*
   * The remaining fields are cold, and arbitrarily ordered.
   */

  int64_t m_sn{-1};             // Note: could be 32-bit
  MD5 m_md5;
  NamedEntityPairTable m_namedInfo;
  FixedVector<const ArrayData*> m_arrays;
  FuncTable m_funcTable;
  mutable PseudoMainCacheMap* m_pseudoMainCache{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
// TODO(#4717225): Rewrite these in iterators.h.

struct AllFuncs {
  explicit AllFuncs(const Unit* unit)
    : fr(unit->funcs())
    , mr(0, 0)
    , cr(unit->preclasses())
  {
    if (fr.empty()) skip();
  }

  bool empty() const {
    return fr.empty() && mr.empty() && cr.empty();
  }

  const Func* front() const {
    assert(!empty());
    if (!fr.empty()) return fr.front();
    assert(!mr.empty());
    return mr.front();
  }

  const Func* popFront() {
    auto f = !fr.empty() ? fr.popFront() :
             !mr.empty() ? mr.popFront() : 0;
    assert(f);
    if (fr.empty() && mr.empty()) skip();
    return f;
  }

private:
  void skip() {
    assert(fr.empty());
    while (!cr.empty() && mr.empty()) {
      auto c = cr.popFront();
      mr = Unit::FuncRange(c->methods(),
                           c->methods() + c->numMethods());
    }
  }

  Unit::FuncRange fr;
  Unit::FuncRange mr;
  Unit::PreClassRange cr;
};

class AllCachedClasses {
  NamedEntity::Map::iterator m_next, m_end;
  void skip();

public:
  AllCachedClasses();
  bool empty() const;
  Class* front();
  Class* popFront();
};

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_UNIT_INL_H_
#include "hphp/runtime/vm/unit-inl.h"
#undef incl_HPHP_VM_UNIT_INL_H_

#endif // incl_HPHP_VM_UNIT_H_
