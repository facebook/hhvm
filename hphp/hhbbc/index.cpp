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
#include "hphp/hhbbc/index.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_map.h>

#include <folly/Format.h>
#include <folly/Hash.h>
#include <folly/Lazy.h>
#include <folly/MapUtil.h>
#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/concurrency/ConcurrentHashMap.h>

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"

#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/trait-method-import-data.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/options-util.h"
#include "hphp/hhbbc/parallel.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-builtins.h"
#include "hphp/hhbbc/type-structure.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/wide-func.h"

#include "hphp/util/algorithm.h"
#include "hphp/util/assertions.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/lock-free-lazy.h"
#include "hphp/util/match.h"

#include "hphp/zend/zend-string.h"

namespace HPHP {
namespace HHBBC {

TRACE_SET_MOD(hhbbc_index);

//////////////////////////////////////////////////////////////////////

using namespace extern_worker;

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_construct("__construct");
const StaticString s_toBoolean("__toBoolean");
const StaticString s_invoke("__invoke");
const StaticString s_Closure("Closure");
const StaticString s_AsyncGenerator("HH\\AsyncGenerator");
const StaticString s_Generator("Generator");
const StaticString s_Awaitable("HH\\Awaitable");

//////////////////////////////////////////////////////////////////////

// HHBBC consumes a LOT of memory, so we keep representation types small.
template <typename T, size_t Expected, size_t Actual = sizeof(T)>
constexpr bool CheckSize() { static_assert(Expected == Actual); return true; };
static_assert(CheckSize<php::Block, 24>(), "");
static_assert(CheckSize<php::Local, use_lowptr ? 12 : 16>(), "");
static_assert(CheckSize<php::Param, use_lowptr ? 64 : 96>(), "");
static_assert(CheckSize<php::Func, use_lowptr ? 184 : 224>(), "");

// Likewise, we also keep the bytecode and immediate types small.
static_assert(CheckSize<Bytecode, use_lowptr ? 32 : 40>(), "");
static_assert(CheckSize<MKey, 16>(), "");
static_assert(CheckSize<IterArgs, 16>(), "");
static_assert(CheckSize<FCallArgs, 8>(), "");
static_assert(CheckSize<RepoAuthType, 8>(), "");

//////////////////////////////////////////////////////////////////////

/*
 * One-to-many case sensitive map, where the keys are static strings
 * and the values are some kind of pointer.
 */
template<class T> using SStringToMany = std::unordered_multimap<SString, T*>;

/*
 * One-to-one case insensitive map, where the keys are static strings
 * and the values are some T.
 *
 * Elements are not stable under insert/erase.
 */
template<class T> using ISStringToOneT =
  hphp_fast_map<
    SString,
    T,
    string_data_hash,
    string_data_isame
  >;

/*
 * One-to-one case sensitive map, where the keys are static strings
 * and the values are some T.
 *
 * Elements are not stable under insert/erase.
 *
 * Static strings are always uniquely defined by their pointer, so
 * pointer hashing/comparison is sufficient.
 */
template<class T> using SStringToOneT = hphp_fast_map<SString, T>;

/*
 * One-to-one case sensitive concurrent map, where the keys are static
 * strings and the values are some T.
 *
 * Concurrent insertions and lookups are supported.
 *
 * Static strings are always uniquely defined by their pointer, so
 * pointer hashing/comparison is sufficient.
 */
template<class T> using SStringToOneConcurrentT =
  folly_concurrent_hash_map_simd<SString, T>;

/*
 * One-to-one case insensitive concurrent map, where the keys are
 * static strings and the values are some T.
 *
 * Concurrent insertions and lookups are supported.
 */
template<class T> using ISStringToOneConcurrentT =
  folly_concurrent_hash_map_simd<SString, T,
                                 string_data_hash, string_data_isame>;

/*
 * Case sensitive static string set.
 */
using SStringSet = hphp_fast_set<SString>;
/*
 * Case insensitive static string set.
 */
using ISStringSet = hphp_fast_set<SString, string_data_hash, string_data_isame>;

//////////////////////////////////////////////////////////////////////

template<typename T> using UniquePtrRef = Ref<std::unique_ptr<T>>;

//////////////////////////////////////////////////////////////////////

Dep operator|(Dep a, Dep b) {
  return static_cast<Dep>(
    static_cast<uintptr_t>(a) | static_cast<uintptr_t>(b)
  );
}

bool has_dep(Dep m, Dep t) {
  return static_cast<uintptr_t>(m) & static_cast<uintptr_t>(t);
}

/*
 * Maps functions to contexts that depend on information about that
 * function, with information about the type of dependency.
 */
using DepMap =
  tbb::concurrent_hash_map<
    DependencyContext,
    hphp_fast_map<
      DependencyContext,
      Dep,
      DependencyContextHash,
      DependencyContextEquals
    >,
    DependencyContextHashCompare
  >;

//////////////////////////////////////////////////////////////////////

/*
 * Each ClassInfo has a table of public static properties with these entries.
 * The `initializerType' is for use during refine_public_statics, and
 * inferredType will always be a supertype of initializerType.
 */
struct PublicSPropEntry {
  Type inferredType;
  Type initializerType;
  const php::Prop* prop;
  uint32_t refinements;
  /*
   * This flag is set during analysis to indicate that we resolved the
   * initial value (and updated it on the php::Class). This doesn't
   * need to be atomic, because only one thread can resolve the value
   * (the one processing the 86sinit), and it's been joined by the
   * time we read the flag in refine_public_statics.
   */
  bool initialValueResolved;
  bool everModified;
};

//////////////////////////////////////////////////////////////////////

/*
 * Represents a method, without requiring an explicit pointer to a
 * php::Func (so can be used across remote workers).
 */
struct MethRef {
  MethRef() = default;
  explicit MethRef(const php::Func& f)
    : cls{f.cls->name}, idx{f.clsIdx} {}
  MethRef(SString cls, uint32_t idx)
    : cls{cls}, idx{idx} {}

  SString cls{nullptr};
  // Index in the class' methods table.
  uint32_t idx{std::numeric_limits<uint32_t>::max()};

  bool operator==(const MethRef& o) const {
    return cls->isame(o.cls) && idx == o.idx;
  }
  bool operator!=(const MethRef& o) const {
    return !(*this == o);
  }
  bool operator<(const MethRef& o) const {
    if (!cls->isame(o.cls)) return string_data_lti{}(cls, o.cls);
    return idx < o.idx;
  }

  struct Hash {
    size_t operator()(const MethRef& m) const {
      return folly::hash::hash_combine(m.cls->hash(), m.idx);
    }
  };

  template <typename SerDe> void serde(SerDe& sd) {
    sd(cls)(idx);
  }
};

/*
 * Entries in the ClassInfo method table need to track some additional
 * information.
 *
 * The reason for this is that we need to record attributes of the
 * class hierarchy.
 *
 * We store a lot of these, so we go to some effort to keep it as
 * small as possible.
 */
struct MethTabEntry {
  MethTabEntry()
    : MethTabEntry{MethRef{}, Attr{}} {}
  explicit MethTabEntry(const php::Func& f)
    : MethTabEntry{MethRef{f}, f.attrs} {}
  MethTabEntry(const php::Func& f, Attr a)
    : MethTabEntry{MethRef{f}, a} {}
  MethTabEntry(MethRef meth, Attr a)
    : cls{TopLevel, meth.cls}
    , clsIdx{meth.idx}
    , attrs{a} {}

  MethRef meth() const { return MethRef{cls.ptr(), clsIdx}; }
  void setMeth(MethRef m) { cls.set(cls.tag(), m.cls); clsIdx = m.idx; }

  // There's a private method further up the class hierarchy with the
  // same name.
  bool hasPrivateAncestor() const { return cls.tag() & HasPrivateAncestor; }
  // This method came from the ClassInfo that owns the MethTabEntry,
  // or one of its used traits.
  bool topLevel() const { return cls.tag() & TopLevel; }
  // This method isn't overridden by methods in any regular classes.
  bool noOverrideRegular() const { return cls.tag() & NoOverrideRegular; }

  void setHasPrivateAncestor() {
    cls.set(Bits(cls.tag() | HasPrivateAncestor), cls.ptr());
  }
  void setTopLevel() {
    cls.set(Bits(cls.tag() | TopLevel), cls.ptr());
  }
  void setNoOverrideRegular() {
    cls.set(Bits(cls.tag() | NoOverrideRegular), cls.ptr());
  }

  void clearHasPrivateAncestor() {
    cls.set(Bits(cls.tag() & ~HasPrivateAncestor), cls.ptr());
  }
  void clearTopLevel() {
    cls.set(Bits(cls.tag() & ~TopLevel), cls.ptr());
  }
  void clearNoOverrideRegular() {
    cls.set(Bits(cls.tag() & ~NoOverrideRegular), cls.ptr());
  }

private:
  // Logically, a MethTabEntry stores a MethRef. However doing so
  // makes the MethTabEntry larger, due to alignment. So instead we
  // store the MethRef fields (which lets the clsIdx and attrs share
  // the same 64-bits). Moreover, we can store the special bits in the
  // cls name pointer.
  enum Bits : uint8_t {
    HasPrivateAncestor = (1u << 0),
    TopLevel = (1u << 1),
    NoOverrideRegular = (1u << 2)
  };
  CompactTaggedPtr<const StringData, Bits> cls;
  uint32_t clsIdx;

public:
  // A method could be imported from a trait, and its attributes
  // changed.
  Attr attrs;

  template <typename SerDe> void serde(SerDe& sd) {
    if constexpr (SerDe::deserializing) {
      SString clsname;
      Bits bits;
      sd(clsname)(clsIdx)(attrs)(bits);
      cls.set(bits, clsname);
    } else {
      sd(cls.ptr())(clsIdx)(attrs)(cls.tag());
    }
  }
};

// Don't indeliberably make this larger
static_assert(CheckSize<MethTabEntry, 16>(), "");

//////////////////////////////////////////////////////////////////////

using ContextRetTyMap = tbb::concurrent_hash_map<
  CallContext,
  Type,
  CallContextHashCompare
>;

//////////////////////////////////////////////////////////////////////

template<class Filter>
PropState make_unknown_propstate(const Index& index,
                                 const php::Class* cls,
                                 Filter filter) {
  auto ret = PropState{};
  for (auto& prop : cls->properties) {
    if (filter(prop)) {
      auto& elem = ret[prop.name];
      elem.ty = adjust_type_for_prop(
        index,
        *cls,
        &prop.typeConstraint,
        TCell
      );
      if (prop.attrs & AttrSystemInitialValue) {
        auto initial = loosen_all(from_cell(prop.val));
        if (!initial.subtypeOf(BUninit)) elem.ty |= initial;
      }
      elem.tc = &prop.typeConstraint;
      elem.attrs = prop.attrs;
      elem.everModified = true;
    }
  }

  return ret;
}

}

/*
 * Currently inferred information about a PHP function.
 *
 * Nothing in this structure can ever be untrue.  The way the
 * algorithm works, whatever is in here must be factual (even if it is
 * not complete information), because we may deduce other facts based
 * on it.
 */
struct res::Func::FuncInfo {
  const php::Func* func = nullptr;
  /*
   * The best-known return type of the function, if we have any
   * information.  May be TBottom if the function is known to never
   * return (e.g. always throws).
   */
  Type returnTy = TInitCell;

  /*
   * If the function always returns the same parameter, this will be
   * set to its id; otherwise it will be NoLocalId.
   */
  LocalId retParam{NoLocalId};

  /*
   * The number of times we've refined returnTy.
   */
  uint32_t returnRefinements{0};

  /*
   * Whether the function is effectFree.
   */
  bool effectFree{false};

  /*
   * Bitset representing which parameters definitely don't affect the
   * result of the function, assuming it produces one. Note that
   * the parameter type verification does not count as a use in this context.
   */
  std::bitset<64> unusedParams;

  /*
   * If this function is a method on a regular class.
   */
  std::atomic<bool> regularClassMethod{false};

  /*
   * List of all func families this function belongs to.
   */
  CompactVector<FuncFamily*> families;
};

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * Known information about a particular constant:
 *  - if system is true, it's a system constant and other definitions
 *    will be ignored.
 *  - for non-system constants, if func is non-null it's the unique
 *    pseudomain defining the constant; otherwise there was more than
 *    one definition, or a non-pseudomain definition, and the type will
 *    be TInitCell
 *  - readonly is true if we've only seen uses of the constant, and no
 *    definitions (this could change during the first pass, but not after
 *    that).
 */

struct ConstInfo {
  const php::Func* func;
  Type                          type;
  bool                          system;
  bool                          readonly;
};

using FuncFamily       = res::Func::FuncFamily;
using FuncInfo         = res::Func::FuncInfo;

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

/*
 * Sometimes function resolution can't determine which function
 * something will call, but can restrict it to a family of functions.
 *
 * For example, if you want to call an abstract function on a base
 * class with all unique derived classes, we will resolve the function
 * to a FuncFamily that contains references to all the possible
 * overriding-functions.
 *
 * In general, a FuncFamily can contain functions which are used by a
 * regular class or not. In some contexts, we only care about the
 * subset which are used by a regular class, and in some contexts we
 * care about them all. To save memory, we use a single FuncFamily for
 * both cases. The users of the FuncFamily must skip over which funcs
 * it does not care about.
 *
 * Since we cache information related to the func list, if the "all"
 * case and the "regular-only" case are potentially different, we
 * allocated space for both possibilities. If we determine they'll
 * always be the same, we do not. For example, if the possible func
 * list only contains methods on regular classes, the distinction is
 * irrelevant.
 */
struct res::Func::FuncFamily {
  // A PossibleFunc is a php::Func* with an additional bit that
  // indicates whether that func is present on a regular class or
  // not. This lets us skip over that func if we only care about the
  // regular subset of the list.
  struct PossibleFunc {
    PossibleFunc(const php::Func* f, bool r) : m_func{r, f} {}
    const php::Func* ptr() const { return m_func.ptr(); }
    bool inRegular() const { return (bool)m_func.tag(); }
    bool operator==(const PossibleFunc& o) const { return m_func == o.m_func; }
  private:
    CompactTaggedPtr<const php::Func, uint8_t> m_func;
  };
  using PFuncVec = CompactVector<PossibleFunc>;

  // We have a lot of FuncFamilies, and most of them have the same
  // "static" information (doesn't change as a result of
  // analysis). So, we store unique groups of static info separately
  // and FuncFamilies point to the same ones.
  struct StaticInfo {
    Optional<uint32_t> m_numInOut;
    Optional<RuntimeCoeffects> m_requiredCoeffects;
    Optional<CompactVector<CoeffectRule>> m_coeffectRules;
    PrepKindVec m_paramPreps;
    uint32_t m_minNonVariadicParams;
    uint32_t m_maxNonVariadicParams;
    TriBool m_isReadonlyReturn;
    TriBool m_isReadonlyThis;
    TriBool m_supportsAER;
    bool m_maybeReified : 1;
    bool m_maybeCaresAboutDynCalls : 1;
    bool m_maybeBuiltin : 1;

    bool operator==(const StaticInfo& o) const;
    size_t hash() const;
  };

  // State in the FuncFamily which might vary depending on whether
  // we're considering the regular subset or not.
  struct Info {
    LockFreeLazy<Type> m_returnTy;
    const StaticInfo* m_static{nullptr};
  };

  FuncFamily(PFuncVec&& v, bool add) : m_v{std::move(v)}
  { if (add) m_regular = std::make_unique<Info>(); }

  FuncFamily(FuncFamily&&) = delete;
  FuncFamily(const FuncFamily&) = delete;
  FuncFamily& operator=(FuncFamily&&) = delete;
  FuncFamily& operator=(const FuncFamily&) = delete;

  const PFuncVec& possibleFuncs() const {
    return m_v;
  };

  Info& infoFor(bool regularOnly) {
    if (regularOnly && m_regular) return *m_regular;
    return m_all;
  }
  const Info& infoFor(bool regularOnly) const {
    if (regularOnly && m_regular) return *m_regular;
    return m_all;
  }

  Info m_all;
  // Only allocated if we determined the distinction is relevant. If
  // this is nullptr, m_all can be used for both cases.
  std::unique_ptr<Info> m_regular;
  PFuncVec m_v;
};

bool FuncFamily::StaticInfo::operator==(const FuncFamily::StaticInfo& o) const {
  return
    std::tie(m_numInOut, m_requiredCoeffects, m_coeffectRules,
             m_paramPreps, m_minNonVariadicParams,
             m_maxNonVariadicParams,
             m_isReadonlyReturn, m_isReadonlyThis, m_supportsAER,
             m_maybeReified, m_maybeCaresAboutDynCalls,
             m_maybeBuiltin) ==
    std::tie(o.m_numInOut, o.m_requiredCoeffects, o.m_coeffectRules,
             o.m_paramPreps, o.m_minNonVariadicParams,
             o.m_maxNonVariadicParams,
             o.m_isReadonlyReturn, o.m_isReadonlyThis, o.m_supportsAER,
             o.m_maybeReified, o.m_maybeCaresAboutDynCalls,
             o.m_maybeBuiltin);
}

size_t FuncFamily::StaticInfo::hash() const {
  auto hash = folly::hash::hash_combine(
    m_numInOut,
    m_requiredCoeffects,
    m_minNonVariadicParams,
    m_maxNonVariadicParams,
    m_isReadonlyReturn,
    m_isReadonlyThis,
    m_supportsAER,
    m_maybeReified,
    m_maybeCaresAboutDynCalls,
    m_maybeBuiltin
  );
  hash = folly::hash::hash_range(
    m_paramPreps.begin(),
    m_paramPreps.end(),
    hash
  );
  if (m_coeffectRules) {
    hash = folly::hash::hash_range(
      m_coeffectRules->begin(),
      m_coeffectRules->end(),
      hash
    );
  }
  return hash;
}

namespace {

struct PFuncVecHasher {
  size_t operator()(const FuncFamily::PFuncVec& v) const {
    return folly::hash::hash_range(
      v.begin(),
      v.end(),
      0,
      [] (FuncFamily::PossibleFunc pf) {
        return hash_int64_pair(
          pointer_hash<const php::Func>{}(pf.ptr()),
          pf.inRegular()
        );
      }
    );
  }
};
struct FuncFamilyPtrHasher {
  using is_transparent = void;
  size_t operator()(const std::unique_ptr<FuncFamily>& ff) const {
    return PFuncVecHasher{}(ff->possibleFuncs());
  }
  size_t operator()(const FuncFamily::PFuncVec& pf) const {
    return PFuncVecHasher{}(pf);
  }
};
struct FuncFamilyPtrEquals {
  using is_transparent = void;
  bool operator()(const std::unique_ptr<FuncFamily>& a,
                  const std::unique_ptr<FuncFamily>& b) const {
    return a->possibleFuncs() == b->possibleFuncs();
  }
  bool operator()(const FuncFamily::PFuncVec& pf,
                  const std::unique_ptr<FuncFamily>& ff) const {
    return pf == ff->possibleFuncs();
  }
};

struct FFStaticInfoPtrHasher {
  using is_transparent = void;
  size_t operator()(const std::unique_ptr<FuncFamily::StaticInfo>& i) const {
    return i->hash();
  }
  size_t operator()(const FuncFamily::StaticInfo& i) const {
    return i.hash();
  }
};
struct FFStaticInfoPtrEquals {
  using is_transparent = void;
  bool operator()(const std::unique_ptr<FuncFamily::StaticInfo>& a,
                  const std::unique_ptr<FuncFamily::StaticInfo>& b) const {
    return *a == *b;
  }
  bool operator()(const FuncFamily::StaticInfo& a,
                  const std::unique_ptr<FuncFamily::StaticInfo>& b) const {
    return a == *b;
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * A method family table entry in a compact format. Can represent a
 * FuncFamily, a single php::Func, or emptiness. This represents the
 * possible resolutions of a call to a method with same name. It also
 * stores whether the entry is "complete" or "incomplete". An
 * incomplete entry means the possible resolutions includes the
 * possibility of the method not existing. A complete entry guarantees
 * it has to be one of the methods. This is (right now) irrelevant for
 * FuncFamily, but matters for php::Func, as it determines whether you
 * can fold away the call (if it's incomplete, the call might fatal).
 *
 * We create a lot of these, so we use some trickery to keep it
 * pointer sized.
 */
struct FuncFamilyOrSingle {
  FuncFamilyOrSingle() : m_ptr{Type::Empty, nullptr} {}
  explicit FuncFamilyOrSingle(FuncFamily* ff, bool incomplete)
    : m_ptr{incomplete ? Type::FuncFamilyIncomplete : Type::FuncFamily, ff} {}
  FuncFamilyOrSingle(const php::Func* f, bool incomplete)
    : m_ptr{incomplete ? Type::SingleIncomplete : Type::Single, (void*)f} {}

  // If this represents a FuncFamily, return it (or nullptr
  // otherwise).
  FuncFamily* funcFamily() const {
    return
      (m_ptr.tag() == Type::FuncFamily ||
       m_ptr.tag() == Type::FuncFamilyIncomplete)
        ? (FuncFamily*)m_ptr.ptr()
        : nullptr;
  }

  // If this represents a single php::Func, return it (or nullptr
  // otherwise).
  const php::Func* func() const {
    return
      (m_ptr.tag() == Type::Single || m_ptr.tag() == Type::SingleIncomplete)
        ? (const php::Func*)m_ptr.ptr()
        : nullptr;
  }

  // Return true if this entry represents nothing at all (for example,
  // if the method is guaranteed to not exist).
  bool isEmpty() const { return m_ptr.tag() == Type::Empty; }

  // NB: empty entries are neither incomplete nor complete. Check
  // isEmpty() first if that matters.

  // Return true if this resolution includes the possibility of no
  // method.
  bool isIncomplete() const {
    return
      m_ptr.tag() == Type::FuncFamilyIncomplete ||
      m_ptr.tag() == Type::SingleIncomplete;
  }
  // Return true if the method would resolve to exactly one of the
  // possibilities.
  bool isComplete() const {
    return
      m_ptr.tag() == Type::FuncFamily ||
      m_ptr.tag() == Type::Single;
  }

private:
  enum class Type : uint8_t {
    Empty,
    FuncFamily,
    FuncFamilyIncomplete,
    Single,
    SingleIncomplete
  };
  CompactTaggedPtr<void, Type> m_ptr;
};

std::string show(const FuncFamilyOrSingle& fam) {
  if (auto const ff = fam.funcFamily()) {
    auto const f = ff->possibleFuncs().front().ptr();
    return folly::sformat(
      "func-family {}::{}{}",
      f->cls->name, f->name,
      fam.isIncomplete() ? " (incomplete)" : ""
    );
  }
  if (auto const f = fam.func()) {
    return folly::sformat(
      "func {}::{}{}",
      f->cls->name, f->name,
      fam.isIncomplete() ? " (incomplete)" : ""
    );
  }
  return "empty";
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

/*
 * Known information about an instantiatiable class.
 */
struct ClassInfo {
  /*
   * A pointer to the underlying php::Class that we're storing
   * information about.
   */
  const php::Class* cls = nullptr;

  /*
   * The info for the parent of this Class.
   */
  ClassInfo* parent = nullptr;

  /*
   * A vector of the declared interfaces class info structures.  This is in
   * declaration order mirroring the php::Class interfaceNames vector, and does
   * not include inherited interfaces.
   */
  CompactVector<const ClassInfo*> declInterfaces;

  /*
   * A (case-insensitive) map from interface names supported by this class to
   * their ClassInfo structures, flattened across the hierarchy.
   */
  ISStringToOneT<const ClassInfo*> implInterfaces;

  /*
   * A vector of the included enums, in class order, mirroring the
   * php::Class includedEnums vector.
   */
  CompactVector<const ClassInfo*> includedEnums;

  struct ConstIndex {
    const php::Const& operator*() const {
      return cls->constants[idx];
    }
    const php::Const* operator->() const {
      return get();
    }
    const php::Const* get() const {
      return &cls->constants[idx];
    }
    const php::Class* cls;
    uint32_t idx;
  };

  /*
   * A (case-sensitive) map from class constant name to the php::Class* and
   * index into the constants vector that it came from. This map is flattened
   * across the inheritance hierarchy. Use a vector_map for stable iteration.
   */
  hphp_vector_map<SString, ConstIndex> clsConstants;

  /*
   * A vector of the used traits, in class order, mirroring the
   * php::Class usedTraitNames vector.
   */
  CompactVector<const ClassInfo*> usedTraits;

  /*
   * A list of extra properties supplied by this class's used traits.
   */
  CompactVector<php::Prop> traitProps;

  /*
   * A (case-sensitive) map from class method names to the php::Func
   * associated with it. This map is flattened across the inheritance
   * hierarchy. There's a lot of these, so we use a sorted_vector_map
   * to minimize wasted space.
   */
  folly::sorted_vector_map<SString, MethTabEntry> methods;

  /*
   * A (case-sensitive) map from class method names to associated
   * FuncFamilyOrSingle objects that represent the set of
   * possibly-overriding methods.
   *
   * In addition to the set of methods, a bit is also set indicating
   * whether the set of "complete" or not. A complete set means the
   * ultimate method will definitely be one in the set. An incomplete
   * set means that the ultimate method will either be one in the set,
   * or won't resolve to anything (a missing function).
   *
   * We do not encode "special" methods in these, as their semantics
   * are special and it's not useful.
   *
   * For every method present in this ClassInfo's method table, there
   * will be an entry in methodFamilies. For regular classes, this
   * suffices for looking up information for both all subclasses and
   * the regular subset. For non-regular classes, the results for
   * "all" and the regular subset may differ. In that case, there is a
   * separate "aux" table containing the results for the regular
   * subset. If there is no associated entry in the aux table, the
   * result is the same as the entry in the normal table (this is a
   * common case and saves on memory). For regular classes the aux
   * table is always empty.
   *
   * If a method is marked as AttrNoOverride, it will not have an
   * entry in these maps. If a method is marked as noOverrideRegular,
   * it will not have an entry in the aux map (if it would have
   * otherwise). In either case, the resolved method is assumed to be
   * the same method in this ClassInfo's method table.
   *
   * The above is true for all class types. For abstract classes and
   * interfaces, however, there may be more entries here than present
   * in the methods table. These correspond to methods implemented by
   * *all* regular subclasses of the abstract class/interface. For
   * that reason, they will only be present in the regular variant of
   * the map. This is needed to preserve monotonicity (see
   * expand_abstract_func_families).
   */
  folly::sorted_vector_map<SString, FuncFamilyOrSingle> methodFamilies;
  folly::sorted_vector_map<SString, FuncFamilyOrSingle> methodFamiliesAux;

  /*
   * For classes (abstract and non-abstract), this is the subclasses
   * of this class, including the class itself.
   *
   * For interfaces, this is the list of classes that implement this
   * interface, including the interface itself.
   *
   * For traits, this is the list of classes that use the trait where
   * the trait wasn't flattened into the class (including the trait
   * itself). Note that unlike the other cases, a class being on a
   * trait's subclass list does *not* imply a "is-a" relationship at
   * runtime. You usually want to avoid iterating the subclass list of
   * a trait.
   *
   * The elements in this vector are sorted by their pointer value.
   */
  CompactVector<ClassInfo*> subclassList;

  /*
   * A vector of ClassInfo that encodes the inheritance hierarchy,
   * unless if this ClassInfo represents an interface.
   *
   * This is the list of base classes for this class in inheritance
   * order.
   */
  CompactVector<ClassInfo*> baseList;

  /*
   * Information about interfaces' relationship to each other. Used to
   * speed up subtypeOf and couldBe operations involving
   * interfaces. Since interfaces are a minority, we heap allocate the
   * information (so only need a pointer when we don't need
   * it). Furthermore, we lazily calculate the information (we may not
   * need the information ever for a given interface).
   */
  struct InterfaceInfo {
    // The set of interfaces or base classes which have some non-empty
    // intersection with this interface. If an interface is in this
    // set, one of it's implementations also implements this
    // ClassInfo. If a non-interface is in this set, it or one of it's
    // base classes implements this ClassInfo. This set can
    // potentially be large and is needed rarely, so it is lazily
    // calculated (on top of InterfaceInfo being lazily
    // calculated). We keep two variants. The first only includes
    // information from the interface's implementations which are
    // regular classes. The second uses all of the interface's
    // implementations (and the interface itself).
    using CouldBeSet = hphp_fast_set<const ClassInfo*>;
    mutable LockFreeLazy<CouldBeSet> lazyCouldBe;
    mutable LockFreeLazy<CouldBeSet> lazyCouldBeNonRegular;

    // The set of interfaces which this interface is a subtype
    // of. That is, every implementation of this interface also
    // implements those interfaces. Every interface is a subtypeOf
    // itself, but itself is not stored here (space optimization).
    hphp_fast_set<const ClassInfo*> subtypeOf;

    // Non-nullptr if there's a single class which is a super class of
    // all implementations of this interface, nullptr otherwise.
    const ClassInfo* commonBase;
  };
  // Don't access this directly, use interfaceInfo().
  LockFreeLazyPtr<InterfaceInfo> lazyInterfaceInfo;
  LockFreeLazyPtrNoDelete<ClassInfo> lazyEquivalent;

  // Obtain the InterfaceInfo or CouldBeSet for this interface
  // (calculating it if necessary). This class must be an interface.
  const InterfaceInfo& interfaceInfo();
  const InterfaceInfo::CouldBeSet& couldBe();
  const InterfaceInfo::CouldBeSet& couldBeNonRegular();

  /*
   * Obtain an equivalent ClassInfo for an interface or abstract class
   * when ignoring all non-regular subclasses. This is used for
   * canonicalizing types. The class must have at least one
   * non-regular subclass (so check before calling).
   */
  const ClassInfo* withoutNonRegularEquivalent();

  /*
   * Property types for public static properties, declared on this exact class
   * (i.e. not flattened in the hierarchy).
   *
   * These maps always have an entry for each public static property declared
   * in this class, so it can also be used to check if this class declares a
   * public static property of a given name.
   *
   * Note: the effective type we can assume a given static property may hold is
   * not just the value in these maps.
   */
  SStringToOneT<PublicSPropEntry> publicStaticProps;

  /*
   * Flags to track if this class is mocked, or if any of its derived classes
   * are mocked.
   */
  bool isMocked{false};
  bool isSubMocked{false};

  /*
   * Track if this class has a property which might redeclare a property in a
   * parent class with an inequivalent type-hint.
   */
  bool hasBadRedeclareProp{true};

  /*
   * Track if this class has any properties with initial values that might
   * violate their type-hints.
   */
  bool hasBadInitialPropValues{true};

  /*
   * Track if this class has any const props (including inherited ones).
   */
  bool hasConstProp{false};

  /*
   * Track if any derived classes (including this one) have any const props.
   */
  bool subHasConstProp{false};

  /*
   * Track if this class has a reified parent.
   */
  bool hasReifiedParent{false};

  /*
   * True if there's at least one regular/non-regular class on
   * subclassList (not including this class).
   */
  bool hasRegularSubclass{false};
  bool hasNonRegularSubclass{false};

  /*
   * Return true if this is derived from o.
   */
  bool derivedFrom(const ClassInfo& o) const {
    if (this == &o) return true;
    // If o is an interface, see if this declared it.
    if (o.cls->attrs & AttrInterface) return implInterfaces.count(o.cls->name);
    // Nothing derives from traits, and we already known they're not
    // the same.
    if (o.cls->attrs & AttrTrait) return false;
    // Otherwise check for direct inheritance.
    if (baseList.size() >= o.baseList.size()) {
      return baseList[o.baseList.size() - 1] == &o;
    }
    return false;
  }

  /*
   * Given two ClassInfos, return the most specific ancestor they have
   * in common, or nullptr if they have no common ancestor.
   */
  static const ClassInfo* commonAncestor(const ClassInfo* c1,
                                         const ClassInfo* c2) {
    if (c1 == c2) return c1;
    const ClassInfo* ancestor = nullptr;
    auto it1 = c1->baseList.begin();
    auto it2 = c2->baseList.begin();
    while (it1 != c1->baseList.end() && it2 != c2->baseList.end()) {
      if (*it1 != *it2) break;
      ancestor = *it1;
      ++it1;
      ++it2;
    }
    return ancestor;
  }
};

/*
 * Known information about an instantiable class.
 *
 * This class mirrors the ClassInfo struct, but is produced and used
 * by remote workers. As needed, this struct will gain more and more
 * of ClassInfo's fields (but stored in a more remote worker friendly
 * way). Local calculations will continue to use ClassInfo. Once
 * everything is converted to use remote workers, this struct will
 * subsume ClassInfo entirely (and be renamed).
 */
struct ClassInfo2 {
  /*
   * The name of the underlying php::Class that this ClassInfo
   * represents.
   */
  LSString name{nullptr};

  /*
   * The name of the parent of this class (or nullptr if none).
   */
  LSString parent{nullptr};

  /*
   * A vector of the declared interfaces names. This is in declaration
   * order mirroring the php::Class interfaceNames vector, and does
   * not include inherited interfaces.
   */
  CompactVector<SString> declInterfaces;

  /*
   * A (case-insensitive) set of interface names implemented by this
   * class, flattened across the hierarchy.
   */
  ISStringSet implInterfaces;

  /*
   * A vector of the included enums names, in class order, mirroring
   * the php::Class includedEnums vector.
   */
  CompactVector<SString> includedEnums;

  /*
   * Represents a class constant, pointing to where the constant was
   * originally declared (the class name and it's position in the
   * class' constant table).
   */
  struct ConstIndex {
    SString cls;
    uint32_t idx;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(cls)(idx);
    }
  };

  /*
   * A (case-sensitive) map from class constant name to the ConstIndex
   * representing the constant. This map is flattened across the
   * inheritance hierarchy.
   */
  SStringToOneT<ConstIndex> clsConstants;

  /*
   * A vector of the used traits name, in class order, mirroring the
   * php::Class usedTraitNames vector.
   */
  CompactVector<SString> usedTraits;

  /*
   * A list of extra properties supplied by this class's used traits.
   */
  CompactVector<php::Prop> traitProps;

  /*
   * A (case-sensitive) map from class method names to the
   * MethTabEntry associated with it. This map is flattened across the
   * inheritance hierarchy. MethTabEntry represents the php::Func,
   * along with some metadata specific to the method on this specific
   * class.
   */
  SStringToOneT<MethTabEntry> methods;

  /*
   * A vector of names that encodes the inheritance hierarchy, unless
   * if this class is an interface.
   *
   * This is the list of base classes for this class in inheritance
   * order.
   */
  CompactVector<SString> baseList;

  /*
   * Set of "extra" methods for this class. These are methods which
   * aren't formally part of this class, but must be analyzed along
   * with the class' methods. Examples are unflattened trait methods,
   * and the invoke methods of their associated closures.
   */
  hphp_fast_set<MethRef, MethRef::Hash> extraMethods;

  /*
   * A vector of names of all the closures associated with this class.
   */
  CompactVector<SString> closures;

  /*
   * Track if this class has any const props (including inherited ones).
   */
  bool hasConstProp{false};

  /*
   * Track if this class has a reified parent.
   */
  bool hasReifiedParent{false};

  template <typename SerDe> void serde(SerDe& sd) {
    sd(name)
      (parent)
      (declInterfaces)
      (implInterfaces, string_data_lti{})
      (clsConstants, string_data_lt{})
      (includedEnums)
      (usedTraits)
      (traitProps)
      (methods, string_data_lt{})
      (baseList)
      (extraMethods, std::less<MethRef>{})
      (closures)
      (hasConstProp)
      (hasReifiedParent)
      ;
  }
};

namespace {

ClassInfo::InterfaceInfo::CouldBeSet couldBeSetBuilder(const ClassInfo* cinfo,
                                                       bool nonRegular) {
  assertx(cinfo->cls->attrs & AttrInterface);
  // For every implementation of this interface, add all of the other
  // interfaces this implementation implements, and also all of its
  // parent classes.
  ClassInfo::InterfaceInfo::CouldBeSet couldBe;
  for (auto const sub : cinfo->subclassList) {
    if (!nonRegular && !is_regular_class(*sub->cls)) continue;
    for (auto const& [_, impl] : sub->implInterfaces) couldBe.emplace(impl);

    auto c = sub;
    do {
      // If we already added it, all subsequent parents are also
      // added, so we can stop.
      if (!couldBe.emplace(c).second) break;
      c = c->parent;
    } while (c);
  }
  return couldBe;
}

}

const ClassInfo::InterfaceInfo::CouldBeSet& ClassInfo::couldBe() {
  return interfaceInfo().lazyCouldBe.get(
    [this] { return couldBeSetBuilder(this, false); }
  );
}

const ClassInfo::InterfaceInfo::CouldBeSet& ClassInfo::couldBeNonRegular() {
  return interfaceInfo().lazyCouldBeNonRegular.get(
    [this] { return couldBeSetBuilder(this, true); }
  );
}

const ClassInfo::InterfaceInfo& ClassInfo::interfaceInfo() {
  assertx(cls->attrs & AttrInterface);
  return lazyInterfaceInfo.get(
    [this] {
      auto info = std::make_unique<ClassInfo::InterfaceInfo>();

      // Start out with the info from the first implementation.
      assertx(!subclassList.empty());
      size_t idx = 0;
      while (idx < subclassList.size()) {
        auto const sub = subclassList[idx++];
        if (!is_regular_class(*sub->cls)) continue;
        info->commonBase = sub;
        for (auto const& [_, impl] : sub->implInterfaces) {
          info->subtypeOf.emplace(impl);
        }
        break;
      }

      // Update the common base and subtypeOf list for every
      // implementation. We're only a subtype of an interface if
      // *every* implementation of us implements that interface, so
      // the set can only shrink.
      while (idx < subclassList.size()) {
        auto const sub = subclassList[idx++];
        if (!is_regular_class(*sub->cls)) continue;
        if (info->commonBase) {
          info->commonBase = commonAncestor(info->commonBase, sub);
        }
        folly::erase_if(
          info->subtypeOf,
          [&] (const ClassInfo* i) {
            return !sub->implInterfaces.count(i->cls->name);
          }
        );
        if (!info->commonBase && info->subtypeOf.empty()) break;
      }

      return info.release();
    }
  );
}

const ClassInfo* ClassInfo::withoutNonRegularEquivalent() {
  assertx(cls->attrs & (AttrInterface | AttrAbstract));
  assertx(hasRegularSubclass);
  return &lazyEquivalent.get(
    [this] {
      // Remove the non-regular classes, which will automatically
      // canonicalize this class to its "best" equivalent (which may
      // be itself).
      auto const without = res::Class::removeNonRegular(
        std::array<res::Class, 1>{res::Class { this } }
      );
      // We shouldn't get anything other than one result back. It
      // shouldn't be zero because we already checked it has at least
      // one regular subclass, and we shouldn't get intersections
      // because the common base class should be a subtype of any
      // interfaces.
      always_assert(without.size() == 1);
      auto const w = without.front();
      assertx(w.val.right());
      return w.val.right();
    }
  );
}

//////////////////////////////////////////////////////////////////////

namespace {

uint32_t numNVArgs(const php::Func& f) {
  uint32_t cnt = f.params.size();
  return cnt && f.params[cnt - 1].isVariadic ? cnt - 1 : cnt;
}

PrepKind func_param_prep(const php::Func* f, uint32_t paramId) {
  auto const sz = f->params.size();
  if (paramId >= sz) return PrepKind{TriBool::No, TriBool::No};
  PrepKind kind;
  kind.inOut = yesOrNo(f->params[paramId].inout);
  kind.readonly = yesOrNo(f->params[paramId].readonly);
  return kind;
}

}

//////////////////////////////////////////////////////////////////////

namespace res {

bool Class::same(const Class& o) const {
  return val == o.val;
}

bool Class::exactSubtypeOfExact(const Class& o,
                                bool nonRegularL,
                                bool nonRegularR) const {
  // Unresolved classes are never exact, so they shouldn't be passed.
  assertx(!val.left());
  assertx(!o.val.left());
  // Otherwise two exact resolved classes are only subtypes of another
  // if they're the same. One additional complication is if the class
  // isn't regular and we're not considering non-regular classes. In
  // that case, the class is actually Bottom, and we need to apply the
  // rules of subtyping to Bottom (Bottom is a subtype of everything,
  // but nothing is a subtype of it).
  auto const c1 = val.right();
  auto const c2 = o.val.right();
  auto const bottomL = !nonRegularL && !is_regular_class(*c1->cls);
  auto const bottomR = !nonRegularR && !is_regular_class(*c2->cls);
  return bottomL || (!bottomR && c1 == c2);
}

bool Class::exactSubtypeOf(const Class& o,
                           bool nonRegularL,
                           bool nonRegularR) const {
  // Unresolved classes are never exact, so it should not show up on
  // the lhs.
  assertx(!val.left());

  auto const c1 = val.right();
  // If we want to exclude non-regular classes on either side, and the
  // lhs is not regular, there's no subtype relation. If nonRegularL
  // is false, then lhs is just a bottom (and bottom is a subtype of
  // everything), and if nonRegularR is false, then the rhs doesn't
  // contain any non-regular classes, so lhs cannot be part of it.
  if ((!nonRegularL || !nonRegularR) && !is_regular_class(*c1->cls)) {
    return !nonRegularL;
  }

  if (auto const rname = o.val.left()) {
    // The lhs is resolved, but the rhs is not. The lhs is a subtype
    // of rhs if any of its bases have the same name.
    for (auto const base : c1->baseList) {
      if (base->cls->name->isame(rname)) return true;
    }
    return c1->implInterfaces.count(rname);
  }

  // Otherwise just do an inheritance check.
  return c1->derivedFrom(*o.val.right());
}

bool Class::subSubtypeOf(const Class& o,
                         bool nonRegularL,
                         bool nonRegularR) const {
  // An unresolved class is only a subtype of another if they're both
  // unresolved, have the same name, and the lhs doesn't contain
  // non-regular classes if the rhs doesn't.
  if (auto const lname = val.left()) {
    if (auto const rname = o.val.left()) {
      // If both classes are unresolved, we can't really say for sure
      // if they're subclasses or not. However, as a special case, if
      // their names are the same (and if the lhs doesn't contain
      // non-regular classes if the rhs doesn't).
      return (!nonRegularL || nonRegularR) && lname->isame(rname);
    }
    // The lhs is unresolved, but the rhs is resolved. We can use the
    // rhs subclass list to see if the lhs is on it.
    auto const c2 = o.val.right();
    if (c2->cls->attrs & AttrTrait) {
      return nonRegularR && c2->cls->name->isame(lname);
    }
    for (auto const sub : c2->subclassList) {
      if (!nonRegularR && !is_regular_class(*sub->cls)) continue;
      if (sub->cls->name->isame(lname)) return true;
    }
    return false;
  } else if (auto const rname = o.val.left()) {
    auto const c1 = val.right();
    // The lhs is resolved, but the rhs is not. The lhs is a subtype
    // of rhs if any of its bases have the same name.
    for (auto const base : c1->baseList) {
      if (base->cls->name->isame(rname)) return true;
    }
    return c1->implInterfaces.count(rname);
  }

  auto const c1 = val.right();
  auto const c2 = o.val.right();

  // If the lhs might contain non-regular types, we'll just do a
  // normal derivedFrom check (there's no distinguishing regular and
  // non-regular classes here). However, if the rhs does not contain
  // non-regular types (or if the lhs doesn't actually contain any),
  // then lhs can't be a subtype of rhs (by definition the lhs has at
  // least one class which can't be in the rhs).
  if (nonRegularL) {
    if (nonRegularR ||
        (is_regular_class(*c1->cls) && !c1->hasNonRegularSubclass)) {
      return c1->derivedFrom(*c2);
    }
    return false;
  }

  if (c1->cls->attrs & AttrInterface) {
    // lhs is an interface. Since this is the "sub" variant, it means
    // any implementation of the interface (not the interface class
    // itself).

    // Ooops, the interface has no regular implementations. This means
    // lhs is a bottom, and a bottom is a subtype of everything.
    if (!c1->hasRegularSubclass) return true;

    // An interface can never be a subtype of a trait.
    if (c2->cls->attrs & AttrTrait) return false;

    auto const& info = c1->interfaceInfo();
    if (c2->cls->attrs & AttrInterface) {
      // If both are interfaces, we can use the InterfaceInfo to see
      // if lhs is a subtype of the rhs.
      return info.subtypeOf.count(c2);
    }

    // lhs is an interface, but rhs is not. The interface can only be
    // a subtype of the non-interface if it has a common base which is
    // a subtype of the rhs.
    return info.commonBase && info.commonBase->derivedFrom(*c2);
  }
  // Since this is the "sub" variant, and we're only considering
  // regular classes, a Trait as the lhs is a bottom (since Traits
  // never have subclasses).
  if (c1->cls->attrs & AttrTrait) return true;

  if (c1->cls->attrs & AttrAbstract) {
    // No regular subclasses of the abstract class. This is a bottom.
    if (!c1->hasRegularSubclass) return true;
    // Do an inheritance check first. If it passes, we're gone. If
    // not, we need to do a more expensive check.
    if (c1->derivedFrom(*c2)) return true;
    // For abstract classes, the inheritance check isn't absolute. To
    // be precise we need to check every (regular) subclass of the
    // abstract class.
    for (auto const sub : c1->subclassList) {
      if (!is_regular_class(*sub->cls)) continue;
      if (!sub->derivedFrom(*c2)) return false;
    }
    return true;
  }

  // If lhs is a regular non-abstract class, we can just use the
  // standard inheritance checks.
  return c1->derivedFrom(*c2);
}

bool Class::exactCouldBeExact(const Class& o,
                              bool nonRegularL,
                              bool nonRegularR) const {
  // Unresolved classes can never be exact, so they shouldn't show up
  // on either side.
  assertx(!val.left());
  assertx(!o.val.left());
  // Two resolved exact classes can only be each other if they're the
  // same class. The only complication is if the class isn't regular
  // and we're not considering non-regular classes. In that case, the
  // class is actually Bottom, a Bottom can never could-be anything
  // (not even itself).
  auto const c1 = val.right();
  auto const c2 = o.val.right();
  if (c1 != c2) return false;
  auto const bottomL = !nonRegularL && !is_regular_class(*c1->cls);
  auto const bottomR = !nonRegularR && !is_regular_class(*c2->cls);
  return !bottomL && !bottomR;
}

bool Class::exactCouldBe(const Class& o,
                         bool nonRegularL,
                         bool nonRegularR) const {
  // Unresolved classes can never be exact, so they shouldn't show up
  // on the lhs.
  assertx(!val.left());

  // Otherwise the check is very similar to exactSubtypeOf (except for
  // the handling of bottoms).
  auto const c1 = val.right();
  if ((!nonRegularL || !nonRegularR) && !is_regular_class(*c1->cls)) {
    return false;
  }

  if (auto const rname = o.val.left()) {
    // The lhs is resolved, but the rhs is not. The lhs is a subtype
    // of rhs if any of its bases have the same name (and therefore
    // could be).
    for (auto const base : c1->baseList) {
      if (base->cls->name->isame(rname)) return true;
    }
    return c1->implInterfaces.count(rname);
  }

  return c1->derivedFrom(*o.val.right());
}

bool Class::subCouldBe(const Class& o,
                       bool nonRegularL,
                       bool nonRegularR) const {
  // If we only want to consider regular classes on either side. If
  // true, this means that any possible intersection between the
  // classes can only include regular classes. If either side doesn't
  // have any regular classes, then no intersection is possible.
  auto const eitherRegOnly = !nonRegularL || !nonRegularR;

  if (auto const lname = val.left()) {
    // Two unresolved classes can always potentially be each other.
    if (o.val.left()) return true;

    // The lhs is unresolved, and the rhs is resolved. The lhs could
    // be the rhs if the rhs has a subclass which has a parent or
    // implemented interface with the same name as the lhs.
    auto const c2 = o.val.right();
    if (c2->cls->attrs & AttrTrait) {
      if (eitherRegOnly) return false;
      for (auto const base : c2->baseList) {
        if (base->cls->name->isame(lname)) return true;
      }
      return c2->implInterfaces.count(lname);
    }

    for (auto const sub : c2->subclassList) {
      if (eitherRegOnly && !is_regular_class(*sub->cls)) continue;
      for (auto const base : sub->baseList) {
        if (eitherRegOnly && !is_regular_class(*base->cls)) continue;
        if (base->cls->name->isame(lname)) return true;
      }
      if (!eitherRegOnly && sub->implInterfaces.count(lname)) {
        return true;
      }
    }
    return false;
  } else if (auto const rname = o.val.left()) {
    // This is the same as above, but with the lhs and rhs flipped
    // (for the necessary symmetry of couldBe).
    auto const c1 = val.right();
    if (c1->cls->attrs & AttrTrait) {
      if (eitherRegOnly) return false;
      for (auto const base : c1->baseList) {
        if (base->cls->name->isame(rname)) return true;
      }
      return c1->implInterfaces.count(rname);
    }

    for (auto const sub : c1->subclassList) {
      if (eitherRegOnly && !is_regular_class(*sub->cls)) continue;
      for (auto const base : sub->baseList) {
        if (eitherRegOnly && !is_regular_class(*base->cls)) continue;
        if (base->cls->name->isame(rname)) return true;
      }
      if (!eitherRegOnly && sub->implInterfaces.count(rname)) {
        return true;
      }
    }
    return false;
  }

  auto const c1 = val.right();
  auto const c2 = o.val.right();
  if (c1->cls->attrs & AttrInterface) {
    // Check if interface has any regular implementations if that's
    // all we care about.
    if (eitherRegOnly && !c1->hasRegularSubclass) return false;

    if (c2->cls->attrs & AttrInterface) {
      // Do similar implementation check for other side.
      if (eitherRegOnly && !c2->hasRegularSubclass) return false;

      // Both classes are interfaces. The appropriate could-be sets
      // for the interfaces determine if there's any intersection
      // between them. Since couldBe() is symmetric, we can use either
      // interface's set. We arbitrarily use the interface with the
      // smaller subclass list. By forcing any ordering like this, we
      // should reduce the number of could-be sets we need to create.
      auto const smaller = c1->subclassList.size() <= c2->subclassList.size()
        ? c1 : c2;
      auto const larger = c1->subclassList.size() <= c2->subclassList.size()
        ? c2 : c1;

      // First do the check *only* considering regular classes,
      // regardless of what was requested. If this passes, it's always
      // true, so we can skip creating the set which includes
      // non-regular classes.
      if (smaller->couldBe().count(larger)) return true;
      // It didn't pass. If we're only considering regular classes,
      // then there's nothing to check further.
      if (eitherRegOnly) return false;
      // Otherwise there could be non-regular classes in the
      // intersection (but not any regular classes since we ruled that
      // out already). If the interface has no non-regular
      // implementations, the only possible candidate is the interface
      // itself, so do an implements check.
      if (!smaller->hasNonRegularSubclass) {
        return smaller->implInterfaces.count(larger->cls->name);
      }
      // The smaller interface has non-regular implementations. Do the
      // check against it's could-be set which includes non-regular
      // classes.
      return smaller->couldBeNonRegular().count(larger);
    }
    if (c2->cls->attrs & AttrTrait) {
      // An interface and a trait can only intersect if the trait
      // implements the interface, and we're including non-regular
      // classes (since a trait is always a single non-regular class).
      return !eitherRegOnly && c2->implInterfaces.count(c1->cls->name);
    }

    // c2 is either a normal class or an abstract class:

    if (eitherRegOnly && !c2->hasRegularSubclass) {
      // c2 doesn't have any regular subclasses and only regular class
      // intersections have been requested. If c2 is abstract, no
      // intersection is possible. Otherwise the only intersection is
      // c2 itself, so do an implements check against that.
      if (c2->cls->attrs & AttrAbstract) return false;
      return c2->implInterfaces.count(c1->cls->name);
    }
    // First do the check *only* considering regular classes,
    // regardless of what was requested. If this passes, it's always
    // true, so we don't need to do any further checking.
    if (c1->couldBe().count(c2)) return true;
    // No intersection considering just regular classes. If the
    // intersection can only contain regular classes, or if the
    // interface has no regular implementations, we know there's no
    // intersection at all.
    if (eitherRegOnly || !c1->hasNonRegularSubclass) return false;
    // Otherwise check against the interface's could-be set which
    // includes non-regular classes.
    return c1->couldBeNonRegular().count(c2);
  }

  if (c2->cls->attrs & AttrInterface) {
    // Check if interface contains at least one regular subclass if
    // that's all we care about.
    if (eitherRegOnly && !c2->hasRegularSubclass) return false;

    // c1 cannot be an interface because we already checked that
    // above.

    if (c1->cls->attrs & AttrTrait) {
      // A trait only intersects an interface if the trait implements
      // the interface. Traits are non-regular and have no subclasses,
      // so if we only want regular classes in the intersection, there
      // is no intersection.
      return !eitherRegOnly && c1->implInterfaces.count(c2->cls->name);
    }

    // c1 is either a normal class or an abstract class:

    if (!nonRegularL && !c1->hasRegularSubclass) {
      // c1 doesn't have any regular subclasses and only regular class
      // intersections have been requested. If c1 is abstract, no
      // intersection is possible. Otherwise the only intersection is
      // c1 itself, so do an implements check against that.
      if (c1->cls->attrs & AttrAbstract) return false;
      return c1->implInterfaces.count(c2->cls->name);
    }
    // First do the check *only* considering regular classes,
    // regardless of what was requested. If this passes, it's always
    // true, so we don't need to do any further checking.
    if (c2->couldBe().count(c1)) return true;
    // No intersection considering just regular classes. If the
    // intersection can only contain regular classes, or if the
    // interface has no regular implementations, we know there's no
    // intersection at all.
    if (eitherRegOnly || !c2->hasNonRegularSubclass) return false;
    // Otherwise check against the interface's could-be set which
    // includes non-regular classes.
    return c2->couldBeNonRegular().count(c1);
  }

  // A trait can only intersect with itself, and only if we're
  // including non-regular classes in the intersection.
  if (c1->cls->attrs & AttrTrait) return !eitherRegOnly && c1 == c2;
  if (c2->cls->attrs & AttrTrait) return false;

  // Check if either class only contains non-regular subclasses and
  // we're only looking for regular intersections.
  if (eitherRegOnly) {
    if ((c1->cls->attrs & AttrAbstract) && !c1->hasRegularSubclass) {
      return false;
    }
    if ((c2->cls->attrs & AttrAbstract) && !c2->hasRegularSubclass) {
      return false;
    }
  }

  // Both types are non-interfaces so they "could be" if they are in
  // an inheritance relationship.
  if (c1->baseList.size() >= c2->baseList.size()) {
    return c1->baseList[c2->baseList.size() - 1] == c2;
  } else {
    return c2->baseList[c1->baseList.size() - 1] == c1;
  }
}

SString Class::name() const {
  return val.match(
    [] (SString s) { return s; },
    [] (ClassInfo* ci) { return ci->cls->name.get(); }
  );
}

Optional<res::Class> Class::withoutNonRegular() const {
  return val.match(
    [&] (SString) -> Optional<res::Class> { return *this; },
    [&] (ClassInfo* cinfo) -> Optional<res::Class> {
      // Regular classes are always unchanged
      if (is_regular_class(*cinfo->cls)) return *this;
      // Non-regular class with no regular subclasses just becomes Bottom
      if (!cinfo->hasRegularSubclass) return std::nullopt;
      // Traits can have things on their subclass list (any
      // unflattened users), but still becomes Bottom.
      if (cinfo->cls->attrs & AttrTrait) return std::nullopt;
      if (!(cinfo->cls->attrs & (AttrInterface | AttrAbstract))) return *this;
      // Interfaces or abstract classes need to be canonicalized to
      // their equivalent (which may be themself).
      return Class {
        const_cast<ClassInfo*>(cinfo->withoutNonRegularEquivalent())
      };
    }
  );
}

bool Class::mightBeRegular() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) { return is_regular_class(*cinfo->cls); }
  );
}

bool Class::mightBeNonRegular() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) { return !is_regular_class(*cinfo->cls); }
  );
}

bool Class::couldBeOverridden() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return !(cinfo->cls->attrs & (AttrTrait|AttrNoOverride));
    }
  );
}

bool Class::couldBeOverriddenByRegular() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return !(cinfo->cls->attrs & (AttrTrait|AttrNoOverrideRegular));
    }
  );
}

bool Class::mightContainNonRegular() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return
        !is_regular_class(*cinfo->cls) || cinfo->hasNonRegularSubclass;
    }
  );
}

bool Class::couldHaveMagicBool() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      if (cinfo->cls->attrs & AttrInterface) {
        for (auto const sub : cinfo->subclassList) {
          if (has_magic_bool_conversion(sub->baseList[0]->cls->name)) {
            return true;
          }
        }
        return false;
      }
      return has_magic_bool_conversion(cinfo->baseList[0]->cls->name);
    }
  );
}

bool Class::couldHaveMockedSubClass() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return cinfo->isSubMocked;
    }
  );
}

bool Class::couldBeMocked() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return cinfo->isMocked;
    }
  );
}

bool Class::couldHaveReifiedGenerics() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return cinfo->cls->hasReifiedGenerics;
    }
  );
}

bool Class::mustHaveReifiedGenerics() const {
  return val.match(
    [] (SString) { return false; },
    [] (ClassInfo* cinfo) {
      return cinfo->cls->hasReifiedGenerics;
    }
  );
}

bool Class::couldHaveReifiedParent() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return cinfo->hasReifiedParent;
    }
  );
}

bool Class::mustHaveReifiedParent() const {
  return val.match(
    [] (SString) { return false; },
    [] (ClassInfo* cinfo) {
      return cinfo->hasReifiedParent;
    }
  );
}

bool Class::mightCareAboutDynConstructs() const {
  if (RuntimeOption::EvalForbidDynamicConstructs > 0) {
    return val.match(
      [] (SString) { return true; },
      [] (ClassInfo* cinfo) {
        return !(cinfo->cls->attrs & AttrDynamicallyConstructible);
      }
    );
  }
  return false;
}

bool Class::couldHaveConstProp() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) { return cinfo->hasConstProp; }
  );
}

bool Class::subCouldHaveConstProp() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) { return cinfo->subHasConstProp; }
  );
}
Optional<res::Class> Class::parent() const {
  if (!val.right()) return std::nullopt;
  auto parent = val.right()->parent;
  if (!parent) return std::nullopt;
  return res::Class { parent };
}

const php::Class* Class::cls() const {
  return val.right() ? val.right()->cls : nullptr;
}

void
Class::forEachSubclass(const std::function<void(const php::Class*)>& f) const {
  auto const cinfo = val.right();
  assertx(cinfo);
  for (auto const& s : cinfo->subclassList) f(s->cls);
}

std::string show(const Class& c) {
  return c.val.match(
    [] (SString s) {
      return folly::sformat("{}*", s);
    },
    [] (ClassInfo* cinfo) {
      return cinfo->cls->name->toCppString();
    }
  );
}

ClassInfo* Class::commonAncestor(ClassInfo* a, ClassInfo* b) {
  if (a == b) return a;
  if (!a || !b) return nullptr;
  ClassInfo* ancestor = nullptr;
  // Walk the arrays of base classes until they match. For common
  // ancestors to exist they must be on both sides of the baseList
  // at the same positions
  auto it1 = a->baseList.begin();
  auto it2 = b->baseList.begin();
  while (it1 != a->baseList.end() && it2 != b->baseList.end()) {
    if (*it1 != *it2) break;
    ancestor = *it1;
    ++it1; ++it2;
  }
  return ancestor;
}

// Call the given callable for every class which is a subclass of
// *all* the classes in the range. If the range includes nothing but
// unresolved classes, they will be passed, as-is, to the callable. If
// the range includes a mix of resolved and unresolved classes, the
// unresolved classes will be used to narrow the classes passed to the
// callable, but the unresolved classes themself will not be passed to
// the callable .If the callable returns false, iteration is
// stopped. If includeNonRegular is true, non-regular subclasses are
// visited (normally they are skipped).
template <typename F>
void Class::visitEverySub(folly::Range<const Class*> classes,
                          bool includeNonRegular,
                          const F& f) {
  assertx(!classes.empty());

  // Simple case: if there's only one class, just iterate over the
  // subclass list.
  if (classes.size() == 1) {
    auto const cinfo = classes.front().val.right();
    if (!cinfo) {
      f(classes.front());
    } else if (cinfo->cls->attrs & AttrTrait) {
      if (includeNonRegular) f(Class { cinfo });
    } else {
      for (auto const sub : cinfo->subclassList) {
        if (!includeNonRegular && !is_regular_class(*sub->cls)) continue;
        if (!f(Class { sub })) break;
      }
    }
    return;
  }

  // Otherwise we need to find all of the classes in common:
  CompactVector<ClassInfo*> common;

  // Find the first resolved class, and use that to initialize the
  // list of subclasses.
  auto const numClasses = classes.size();
  size_t resolvedIdx = 0;
  while (resolvedIdx < numClasses) {
    auto const cinfo = classes[resolvedIdx].val.right();
    if (!cinfo) {
      ++resolvedIdx;
      continue;
    }

    if (cinfo->cls->attrs & AttrTrait) {
      if (includeNonRegular) common.emplace_back(cinfo);
    } else if (includeNonRegular ||
               (!cinfo->hasNonRegularSubclass &&
                is_regular_class(*cinfo->cls))) {
      common = cinfo->subclassList;
    } else {
      for (auto const sub : cinfo->subclassList) {
        if (!is_regular_class(*sub->cls)) continue;
        common.emplace_back(sub);
      }
    }
    break;
  }

  // We didn't find any resolved classes. This list is nothing but
  // unresolved classes, so just provide them to the callable and then
  // we're done.
  if (resolvedIdx == numClasses) {
    assertx(common.empty());
    for (auto const c : classes) {
      assertx(c.val.left());
      if (!f(c)) break;
    }
    return;
  }

  // Otherwise we found a resolved class. Now process the rest of the
  // resolved classes, removing any subclasses from the list which
  // aren't a subclass of all of the classes.
  CompactVector<ClassInfo*> newCommon;
  // We start again from 0 to process any unresolved classes we might
  // have skipped over above.
  for (size_t idx = 0; idx < numClasses; ++idx) {
    assertx(!common.empty());
    // Don't process the class we selected above twice.
    if (idx == resolvedIdx) continue;
    newCommon.clear();

    // NB: We don't need to check includeNonRegular here. If it's
    // false, we won't have any non-regular classes in common
    // initially, so none will be part of any intersection.
    if (auto const cinfo = classes[idx].val.right()) {
      // If this class is resolved, intersect the subclass list with
      // the common set of classes.
      assertx(idx > resolvedIdx);
      std::set_intersection(
        begin(common),
        end(common),
        begin(cinfo->subclassList),
        end(cinfo->subclassList),
        std::back_inserter(newCommon)
      );
    } else {
      // If this class is unresolved, we can remove any classes from
      // the common set which couldn't be the unresolved class.
      for (auto const c : common) {
        Class resolved{ c };
        if (resolved.exactCouldBe(classes[idx],
                                  includeNonRegular,
                                  includeNonRegular)) {
          newCommon.emplace_back(c);
        }
      }
    }
    std::swap(common, newCommon);
    if (common.empty()) return;
  }
  assertx(!common.empty());

  // We have the final list. Iterate over these and report them to the
  // callable.
  for (auto const c : common) {
    assertx(IMPLIES(!includeNonRegular, is_regular_class(*c->cls)));
    if (!f(Class { c })) return;
  }
}

// Given a list of classes, put them in canonical form for a
// DCls::IsectSet. It is assumed that couldBe is true between all of
// the classes in the list, but nothing is assumed otherwise.
TinyVector<Class, 2> Class::canonicalizeIsects(const TinyVector<Class, 8>& in,
                                               bool nonRegular) {
  auto const size = in.size();
  if (size == 0) return {};
  if (size < 2) return { in.front() };

  // Canonical ordering:
  auto const compare = [] (Class a, Class b) {
    auto const c1 = a.val.right();
    auto const c2 = b.val.right();
    // Resolved classes always come before unresolved classes.
    if (!c1) {
      if (c2) return 1;
      // Two unresolved classes are just compared by name.
      return a.val.left()->compare(b.val.left());
    } else if (!c2) {
      return -1;
    }

    // "Smaller" classes (those with less subclasses) should come
    // first.
    auto const s1 = c1->subclassList.size();
    auto const s2 = c2->subclassList.size();
    if (s1 < s2) return -1;
    if (s1 > s2) return 1;

    // Regular classes come first, followed by abstract classes,
    // interfaces, then traits.
    auto const weight = [] (const ClassInfo* c) {
      if (c->cls->attrs & AttrAbstract) return 1;
      if (c->cls->attrs & AttrInterface) return 2;
      if (c->cls->attrs & AttrTrait) return 3;
      return 0;
    };
    auto const w1 = weight(c1);
    auto const w2 = weight(c2);
    if (w1 < w2) return -1;
    if (w1 > w2) return 1;

    // All else being equal, compare the name.
    return c1->cls->name->compare(c2->cls->name);
  };

  // Remove any class which is a superclass of another. Such classes
  // are redundant because there's a "smaller" class which already
  // implies it. This also gets rid of duplicates. This is a naive
  // O(N^2) algorithm but it's fine because the lists do not get very
  // large at all.
  TinyVector<Class, 2> out;
  for (int i = 0; i < size; ++i) {
    // For every pair of classes:
    auto const c1 = in[i];
    auto const subtypeOf = [&] {
      for (int j = 0; j < size; ++j) {
        auto const c2 = in[j];
        if (i == j || !c2.subSubtypeOf(c1, nonRegular, nonRegular)) continue;
        // c2 is a subtype of c1. If c1 is not a subtype of c2, then
        // c2 is preferred and we return true to drop c1.
        if (!c1.subSubtypeOf(c2, nonRegular, nonRegular)) return true;
        // They're both subtypes of each other, so they're actually
        // equivalent. We only want to keep one, so use the sorting
        // order and keep the "lesser" one.
        auto const cmp = compare(c1, c2);
        if (cmp > 0 || (cmp == 0 && i > j)) return true;
      }
      return false;
    }();
    if (!subtypeOf) out.emplace_back(c1);
  }

  // Finally sort the list
  std::sort(
    out.begin(),
    out.end(),
    [&] (Class a, Class b) { return compare(a, b) < 0; }
  );
  return out;
}

TinyVector<Class, 2> Class::combine(folly::Range<const Class*> classes1,
                                    folly::Range<const Class*> classes2,
                                    bool isSub1,
                                    bool isSub2,
                                    bool nonRegular1,
                                    bool nonRegular2) {
  TinyVector<Class, 8> common;
  Optional<ClassInfo*> commonBase;
  Optional<hphp_fast_set<ClassInfo*>> commonInterfaces;

  // The algorithm for unioning together two intersection lists is
  // simple. For every class which is "in" either the first or second
  // list, track the interfaces which are implemented by all of the
  // classes, and the common base class amongst all of them. Build a
  // list of these classes and normalize them.

  auto const processNormal = [&] (ClassInfo* cinfo) {
    // NB: isSub and nonRegular is irrelevant here... Everything that
    // we look at in the base class here must also be true for all of
    // its children.

    // Set commonBase, or update it depending on whether this is the
    // first class processed.
    if (!commonBase) {
      commonBase = cinfo;
    } else {
      commonBase = commonAncestor(*commonBase, cinfo);
    }

    // Likewise, initialize the set of common interfaces, or remove
    // any which aren't present.
    if (!commonInterfaces) {
      commonInterfaces.emplace();
      for (auto const i : cinfo->implInterfaces) {
        commonInterfaces->emplace(const_cast<ClassInfo*>(i.second));
      }
    } else {
      folly::erase_if(
        *commonInterfaces,
        [&] (ClassInfo* i) {
          return !cinfo->implInterfaces.count(i->cls->name);
        }
      );
    }
  };

  // Process an interface's implementations (but not the interface
  // itself).
  auto const processIface = [&] (ClassInfo* cinfo) {
    assertx(cinfo->cls->attrs & AttrInterface);
    auto& info = cinfo->interfaceInfo();

    // We assume !nonRegular here since if it was true, we'd be
    // processing the interface in processNormal(). info.subtypeOf and
    // info.commonBase are calculated ignoring non-regular
    // implementations.

    // The logic for processing an interface is similar to processSub,
    // except we use the interface's common base (if any).
    auto const ifaceCommon = const_cast<ClassInfo*>(info.commonBase);
    if (!commonBase) {
      commonBase = ifaceCommon;
    } else {
      commonBase = commonAncestor(*commonBase, ifaceCommon);
    }

    // Instead of implInterfaces, we use the set of interfaces it is a
    // subtype of.
    if (!commonInterfaces) {
      commonInterfaces.emplace();
      for (auto const i : info.subtypeOf) {
        commonInterfaces->emplace(const_cast<ClassInfo*>(i));
      }
      commonInterfaces->emplace(cinfo);
    } else {
      folly::erase_if(
        *commonInterfaces,
        [&] (ClassInfo* i) { return i != cinfo && !info.subtypeOf.count(i); }
      );
    }
  };

  auto const processList = [&] (folly::Range<const Class*> classes,
                                bool isSub,
                                bool nonRegular) {
    if (classes.size() == 1) {
      // If the list is just a single class, we can process things
      // more efficiently.
      auto const cinfo = classes[0].val.right();
      // We dealt with lists of all unresolved classes specially
      // below, so shouldn't get here.
      assertx(cinfo);
      if (cinfo->cls->attrs & (AttrAbstract|AttrInterface)) {
        // Are we including non-regular classes? If we are, we can
        // process this like any other.
        if (nonRegular) {
          processNormal(cinfo);
          return;
        }
        // We're non-regular. Do we care about sub-classes? If not,
        // there's nothing more to do. We're not processing this
        // class, nor its sub-classes.
        if (!isSub) return;
        // Otherwise we're not processing the base class, but we are
        // its sub-classes. For interfaces we can deal with this
        // specially.
        if (cinfo->cls->attrs & AttrInterface) {
          processIface(cinfo);
          return;
        }
        // For abstract classes, however, we'll fall through and use
        // visitEverySub.
      } else if (cinfo->cls->attrs & AttrTrait) {
        // Traits have no subclasses, so isSub doesn't matter. Process
        // it if we're including non-regular classes.
        if (nonRegular) processNormal(cinfo);
        return;
      } else {
        // A regular class. Always process it.
        processNormal(cinfo);
        return;
      }
    }

    // The list has multiple classes or we have an abstract class and
    // we only care about its subclasses. This is more expensive, we
    // need to visit every subclass in the intersection of the classes
    // on the list.
    visitEverySub(
      classes,
      nonRegular,
      [&] (res::Class c) {
        // visitEverySub will only report an unresolved class if the
        // entire list is unresolved, and we deal with that case
        // specially below and shouldn't get here.
        assertx(c.val.right());
        // We'll only "visit" exact sub-classes, so only use
        // processNormal here.
        processNormal(c.val.right());
        // No point in continuing if there's nothing in common left.
        return *commonBase || !commonInterfaces->empty();
      }
    );
  };

  assertx(!classes1.empty());
  assertx(!classes2.empty());
  assertx(IMPLIES(!isSub1, classes1.size() == 1));
  assertx(IMPLIES(!isSub2, classes2.size() == 1));

  // If either side is composed of nothing but unresolved classes, we
  // need to deal with that specially (because we cannot know their
  // subclasses, the above logic doesn't work). If either side has
  // *some* (but not all) unresolved classes, that is fine, because
  // visitEverySub will handle that for us.
  auto const allUnresolved1 = std::all_of(
    classes1.begin(), classes1.end(),
    [] (res::Class c) { return (bool)c.val.left(); }
  );
  auto const allUnresolved2 = std::all_of(
    classes2.begin(), classes2.end(),
    [] (res::Class c) { return (bool)c.val.left(); }
  );

  if (!allUnresolved1 && !allUnresolved2) {
    // There's resolved classes on both sides. We can use the normal
    // process logic.
    processList(classes1, isSub1, nonRegular1);
    processList(classes2, isSub2, nonRegular2);
    // Combine the common classes
    if (commonBase && *commonBase) {
      common.emplace_back(Class { *commonBase });
    }
    if (commonInterfaces) {
      for (auto const i : *commonInterfaces) {
        common.emplace_back(Class { i });
      }
    }
  } else {
    // Either side (maybe both) is made up of unresolved
    // classes. Instead of the above subclass based logic, only keep
    // the classes (on either side) which are a subtype of a class on
    // the opposite side.
    auto const either = nonRegular1 || nonRegular2;
    for (auto const c1 : classes1) {
      auto const subtypeOf = std::any_of(
        classes2.begin(), classes2.end(),
        [&] (res::Class c2) {
          return c2.subSubtypeOf(c1, either, either);
        }
      );
      if (subtypeOf) common.emplace_back(c1);
    }
    for (auto const c2 : classes2) {
      auto const subtypeOf = std::any_of(
        classes1.begin(), classes1.end(),
        [&] (res::Class c1) {
          return c1.subSubtypeOf(c2, either, either);
        }
      );
      if (subtypeOf) common.emplace_back(c2);
    }
  }

  // Finally canonicalize the set
  return canonicalizeIsects(common, nonRegular1 || nonRegular2);
}

TinyVector<Class, 2>
Class::removeNonRegular(folly::Range<const Class*> classes) {
  TinyVector<Class, 8> common;
  Optional<ClassInfo*> commonBase;
  Optional<hphp_fast_set<ClassInfo*>> commonInterfaces;

  // Iterate over every exact member of the class list, filtering out
  // non-regular classes, and rebuild the common base and common
  // interface.
  visitEverySub(
    classes,
    false,
    [&] (res::Class c) {
      // Unresolved classes are always "regular" (we can't tell
      // otherwise), so they remain as is.
      if (c.val.left()) {
        common.emplace_back(c);
        return true;
      }

      // Must have a cinfo because we checked above all the classes
      // were resolved.
      auto const cinfo = c.val.right();
      assertx(cinfo);
      assertx(is_regular_class(*cinfo->cls));

      if (!commonBase) {
        commonBase = cinfo;
      } else {
        commonBase = commonAncestor(*commonBase, cinfo);
      }

      if (!commonInterfaces) {
        commonInterfaces.emplace();
        for (auto const i : cinfo->implInterfaces) {
          commonInterfaces->emplace(const_cast<ClassInfo*>(i.second));
        }
      } else {
        folly::erase_if(
          *commonInterfaces,
          [&] (ClassInfo* i) {
            return !cinfo->implInterfaces.count(i->cls->name);
          }
        );
      }

      // Stop iterating if there's no longer anything in common.
      return *commonBase || !commonInterfaces->empty();
    }
  );

  if (commonBase && *commonBase) {
    common.emplace_back(Class { *commonBase });
  }
  if (commonInterfaces) {
    for (auto const i : *commonInterfaces) {
      common.emplace_back(Class { i });
    }
  }

  // Canonicalize the common base classes/interfaces.
  return canonicalizeIsects(common, false);
}

TinyVector<Class, 2> Class::intersect(folly::Range<const Class*> classes1,
                                      folly::Range<const Class*> classes2,
                                      bool nonRegular1,
                                      bool nonRegular2,
                                      bool& nonRegularOut) {
  TinyVector<Class, 8> common;

  // The algorithm for intersecting two intersection lists is similar
  // to unioning, except we only need to consider the classes which
  // are subclasses of all the classes in *both* lists.

  assertx(!classes1.empty());
  assertx(!classes2.empty());

  auto const bothNonRegular = nonRegular1 && nonRegular2;
  nonRegularOut = bothNonRegular;

  // Even if both the lhs and rhs contain non-regular classes, the
  // intersection may not. We check if the intersection contains any
  // non-regular classes so we can inform the caller to set up the
  // type appropriately.
  auto isectContainsNonRegular = false;

  // Estimate the sizes of each side by summing their subclass list
  // lengths. We want to iterate over the "smaller" set of classes.
  size_t size1 = 0;
  size_t size2 = 0;
  for (auto const c : classes1) {
    if (auto const cinfo = c.val.right()) {
      assertx(!cinfo->subclassList.empty());
      size1 += cinfo->subclassList.size();
    }
  }
  for (auto const c : classes2) {
    if (auto const cinfo = c.val.right()) {
      assertx(!cinfo->subclassList.empty());
      size2 += cinfo->subclassList.size();
    }
  }

  auto const process = [&] (folly::Range<const Class*> lhs,
                            folly::Range<const Class*> rhs) {
    Optional<ClassInfo*> commonBase;
    Optional<hphp_fast_set<ClassInfo*>> commonInterfaces;

    // Since we're calculating the intersection, we only have to visit
    // one list, and check against the other.
    visitEverySub(
      lhs,
      bothNonRegular,
      [&] (res::Class c) {
        auto const cinfo = c.val.right();
        // We shouldn't use visitEverySub if the class list is nothing
        // but unresolved classes, so we should never get an
        // unresolved class in the callback.
        assertx(cinfo);
        assertx(IMPLIES(!bothNonRegular, is_regular_class(*cinfo->cls)));

        // Could this class be a class in the other list? If not, ignore
        // it (it's not part of the intersection result).
        for (auto const other : rhs) {
          if (!c.exactCouldBe(other, bothNonRegular, bothNonRegular)) {
            return true;
          }
        }

        // Otherwise it is part of the intersection, and we need to
        // update the common base and interfaces likewise.

        if (!commonBase) {
          commonBase = cinfo;
        } else {
          commonBase = commonAncestor(*commonBase, cinfo);
        }

        if (!commonInterfaces) {
          commonInterfaces.emplace();
          for (auto const i : cinfo->implInterfaces) {
            commonInterfaces->emplace(const_cast<ClassInfo*>(i.second));
          }
        } else {
          folly::erase_if(
            *commonInterfaces,
            [&] (ClassInfo* i) {
              return !cinfo->implInterfaces.count(i->cls->name);
            }
          );
        }

        if (bothNonRegular &&
            !isectContainsNonRegular &&
            !is_regular_class(*cinfo->cls)) {
          isectContainsNonRegular = true;
        }

        // Stop iterating if there's no longer anything in common.
        return *commonBase || !commonInterfaces->empty();
      }
    );

    if (commonBase && *commonBase) {
      common.emplace_back(Class { *commonBase });
    }
    if (commonInterfaces) {
      for (auto const i : *commonInterfaces) {
        common.emplace_back(Class { i });
      }
    }

    // If the common set is empty at this point, the intersection is
    // empty anyways, so we don't need to worry about any unresolved
    // classes. Otherwise add unresolved classes on both sides to the
    // common set. Canonicalization will remove them if they're
    // redundant.
    if (!common.empty()) {
      for (auto const c : classes1) {
        if (c.val.left()) common.emplace_back(c);
      }
      for (auto const c : classes2) {
        if (c.val.left()) common.emplace_back(c);
      }
    }
  };

  // The first parameter is the class range we'll call visitEverySub
  // on, so use the smaller of the two ranges. Don't use a range with
  // a "size" of 0, which means it's nothing but unresolved classes.
  if (size1 == 0) {
    if (size2 > 0) {
      process(classes2, classes1);
    } else {
      // If both ranges are nothing but unresolved classes, we don't
      // need to process them at all. The intersection is just the
      // combined list of classes (canonicalization will remove any
      // redundancies).
      for (auto const c : classes1) {
        if (c.val.left()) common.emplace_back(c);
      }
      for (auto const c : classes2) {
        if (c.val.left()) common.emplace_back(c);
      }
    }
  } else if (size2 == 0 || size1 <= size2) {
    process(classes1, classes2);
  } else {
    process(classes2, classes1);
  }

  // Canonicalize the common base classes/interfaces.
  assertx(IMPLIES(!bothNonRegular, !isectContainsNonRegular));
  nonRegularOut = isectContainsNonRegular;
  return canonicalizeIsects(common, isectContainsNonRegular);
}

bool Class::couldBeIsect(folly::Range<const Class*> classes1,
                         folly::Range<const Class*> classes2,
                         bool nonRegular1,
                         bool nonRegular2) {
  assertx(!classes1.empty());
  assertx(!classes2.empty());

  auto const bothNonReg = nonRegular1 && nonRegular2;

  // Otherwise decompose the first class list into each of it's exact
  // subclasses, and do a could-be check against every class on the
  // second list. This is precise since the lhs is always exact.
  auto couldBe = false;
  visitEverySub(
    classes1,
    bothNonReg,
    [&] (res::Class c) {
      if (!c.val.left()) {
        for (auto const o : classes2) {
          if (!c.exactCouldBe(o, bothNonReg, bothNonReg)) return true;
        }
      } else {
        for (auto const o : classes2) {
          if (!c.subCouldBe(o, bothNonReg, bothNonReg)) return true;
        }
      }
      couldBe = true;
      return false;
    }
  );
  return couldBe;
}

Func::Func(Rep val)
  : val(val)
{}

SString Func::name() const {
  return match<SString>(
    val,
    [&] (FuncName s)   { return s.name; },
    [&] (MethodName s) { return s.name; },
    [&] (FuncInfo* fi) { return fi->func->name; },
    [&] (Method m)     { return m.func->name; },
    [&] (MethodFamily fam) {
      return fam.family->possibleFuncs().front().ptr()->name;
    },
    [&] (MethodOrMissing m) { return m.func->name; },
    [&] (Missing m) { return m.name; },
    [&] (const Isect& i) {
      assertx(i.families.size() > 1);
      return i.families[0]->possibleFuncs().front().ptr()->name;
    }
  );
}

const php::Func* Func::exactFunc() const {
  using Ret = const php::Func*;
  return match<Ret>(
    val,
    [&](FuncName)                    { return Ret{}; },
    [&](MethodName)                  { return Ret{}; },
    [&](FuncInfo* fi)                { return fi->func; },
    [&](Method m)                    { return m.func; },
    [&](MethodFamily)                { return Ret{}; },
    [&](MethodOrMissing)             { return Ret{}; },
    [&](Missing)                     { return Ret{}; },
    [&](const Isect&)                { return Ret{}; }
  );
}

bool Func::isFoldable() const {
  return match<bool>(
    val,
    [&](FuncName)   { return false; },
    [&](MethodName) { return false; },
    [&](FuncInfo* fi) {
      return fi->func->attrs & AttrIsFoldable;
    },
    [&](Method m) { return m.func->attrs & AttrIsFoldable; },
    [&](MethodFamily)    { return false; },
    [&](MethodOrMissing) { return false; },
    [&](Missing)         { return false; },
    [&](const Isect&)    { return false; }
  );
}

bool Func::couldHaveReifiedGenerics() const {
  return match<bool>(
    val,
    [&](FuncName s) { return true; },
    [&](MethodName) { return true; },
    [&](FuncInfo* fi) { return fi->func->isReified; },
    [&](Method m) { return m.func->isReified; },
    [&](MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly).m_static->m_maybeReified;
    },
    [&](MethodOrMissing m) { return m.func->isReified; },
    [&](Missing) { return false; },
    [&](const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_static->m_maybeReified) return false;
      }
      return true;
    }
  );
}

bool Func::mightCareAboutDynCalls() const {
  if (RuntimeOption::EvalNoticeOnBuiltinDynamicCalls && mightBeBuiltin()) {
    return true;
  }
  auto const mightCareAboutFuncs =
    RuntimeOption::EvalForbidDynamicCallsToFunc > 0;
  auto const mightCareAboutInstMeth =
    RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0;
  auto const mightCareAboutClsMeth =
    RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;

  return match<bool>(
    val,
    [&](FuncName) { return mightCareAboutFuncs; },
    [&](MethodName) {
      return mightCareAboutClsMeth || mightCareAboutInstMeth;
    },
    [&](FuncInfo* fi) {
      return dyn_call_error_level(fi->func) > 0;
    },
    [&](Method m) { return dyn_call_error_level(m.func) > 0; },
    [&](MethodFamily fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_static->m_maybeCaresAboutDynCalls;
    },
    [&](MethodOrMissing m) { return dyn_call_error_level(m.func) > 0; },
    [&](Missing m) { return false; },
    [&](const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_static->m_maybeCaresAboutDynCalls) {
          return false;
        }
      }
      return true;
    }
  );
}

bool Func::mightBeBuiltin() const {
  return match<bool>(
    val,
    // Builtins are always uniquely resolvable unless renaming is
    // involved.
    [&](FuncName s) { return s.renamable; },
    [&](MethodName) { return true; },
    [&](FuncInfo* fi) { return fi->func->attrs & AttrBuiltin; },
    [&](Method m) { return m.func->attrs & AttrBuiltin; },
    [&](MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly).m_static->m_maybeBuiltin;
    },
    [&](MethodOrMissing m) { return m.func->attrs & AttrBuiltin; },
    [&](Missing m) { return false; },
    [&](const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_static->m_maybeBuiltin) return false;
      }
      return true;
    }
  );
}

uint32_t Func::minNonVariadicParams() const {
  return match<uint32_t>(
    val,
    [&] (FuncName) { return 0; },
    [&] (MethodName) { return 0; },
    [&] (FuncInfo* fi) { return numNVArgs(*fi->func); },
    [&] (Method m) { return numNVArgs(*m.func); },
    [&] (MethodFamily fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_static->m_minNonVariadicParams;
    },
    [&] (MethodOrMissing m) { return numNVArgs(*m.func); },
    [&] (Missing) { return 0; },
    [&] (const Isect& i) {
      uint32_t nv = 0;
      for (auto const ff : i.families) {
        nv = std::max(
          nv,
          ff->infoFor(i.regularOnly).m_static->m_minNonVariadicParams
        );
      }
      return nv;
    }
  );
}

uint32_t Func::maxNonVariadicParams() const {
  return match<uint32_t>(
    val,
    [&] (FuncName) { return std::numeric_limits<uint32_t>::max(); },
    [&] (MethodName) { return std::numeric_limits<uint32_t>::max(); },
    [&] (FuncInfo* fi) { return numNVArgs(*fi->func); },
    [&] (Method m) { return numNVArgs(*m.func); },
    [&] (MethodFamily fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_static->m_maxNonVariadicParams;
    },
    [&] (MethodOrMissing m) { return numNVArgs(*m.func); },
    [&] (Missing) { return 0; },
    [&] (const Isect& i) {
      auto nv = std::numeric_limits<uint32_t>::max();
      for (auto const ff : i.families) {
        nv = std::min(
          nv,
          ff->infoFor(i.regularOnly).m_static->m_maxNonVariadicParams
        );
      }
      return nv;
    }
  );
}

const RuntimeCoeffects* Func::requiredCoeffects() const {
  return match<const RuntimeCoeffects*>(
    val,
    [&] (FuncName) { return nullptr; },
    [&] (MethodName) { return nullptr; },
    [&] (FuncInfo* fi) { return &fi->func->requiredCoeffects; },
    [&] (Method m) { return &m.func->requiredCoeffects; },
    [&] (MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly)
        .m_static->m_requiredCoeffects.get_pointer();
    },
    [&] (MethodOrMissing m) { return &m.func->requiredCoeffects; },
    [&] (Missing) { return nullptr; },
    [&] (const Isect& i) {
      const RuntimeCoeffects* coeffects = nullptr;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (!info.m_requiredCoeffects) continue;
        assertx(IMPLIES(coeffects, *coeffects == *info.m_requiredCoeffects));
        if (!coeffects) coeffects = info.m_requiredCoeffects.get_pointer();
      }
      return coeffects;
    }
  );
}

const CompactVector<CoeffectRule>* Func::coeffectRules() const {
  return match<const CompactVector<CoeffectRule>*>(
    val,
    [&] (FuncName) { return nullptr; },
    [&] (MethodName) { return nullptr; },
    [&] (FuncInfo* fi) { return &fi->func->coeffectRules; },
    [&] (Method m) { return &m.func->coeffectRules; },
    [&] (MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly)
        .m_static->m_coeffectRules.get_pointer();
    },
    [&] (MethodOrMissing m) { return &m.func->coeffectRules; },
    [&] (Missing) { return nullptr; },
    [&] (const Isect& i) {
      const CompactVector<CoeffectRule>* coeffects = nullptr;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (!info.m_coeffectRules) continue;
        assertx(
          IMPLIES(
            coeffects,
            std::is_permutation(
              begin(*coeffects),
              end(*coeffects),
              begin(*info.m_coeffectRules),
              end(*info.m_coeffectRules)
            )
          )
        );
        if (!coeffects) coeffects = info.m_coeffectRules.get_pointer();
      }
      return coeffects;
    }
  );
}

std::string show(const Func& f) {
  auto ret = f.name()->toCppString();
  match<void>(
    f.val,
    [&](Func::FuncName s)        { if (s.renamable) ret += '?'; },
    [&](Func::MethodName)        {},
    [&](FuncInfo*)               { ret += "*"; },
    [&](Func::Method)            { ret += "*"; },
    [&](Func::MethodFamily)      { ret += "+"; },
    [&](Func::MethodOrMissing)   { ret += "-"; },
    [&](Func::Missing)           { ret += "!"; },
    [&](const Func::Isect&)      { ret += "&"; }
  );
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

using IfaceSlotMap = hphp_fast_map<const php::Class*, Slot>;

// Inferred class constant type from a 86cinit.
struct ClsConstInfo {
  Type type;
  size_t refinements = 0;
};

struct Index::IndexData {
  explicit IndexData(Index* index) : m_index{index} {}
  IndexData(const IndexData&) = delete;
  IndexData& operator=(const IndexData&) = delete;
  ~IndexData() {
    if (compute_iface_vtables.joinable()) {
      compute_iface_vtables.join();
    }
  }

  Index* m_index;

  bool frozen{false};
  bool ever_frozen{false};

  // If non-nullptr, log information about each pass into it.
  StructuredLogEntry* sample;

  // Async state:
  std::unique_ptr<coro::TicketExecutor> executor;
  std::unique_ptr<Client> client;
  DisposeCallback disposeClient;

  // Global configeration, stored in extern-worker.
  std::unique_ptr<coro::AsyncValue<Ref<Config>>> configRef;

  // Maps unit/class/func name to the extern-worker ref representing
  // php::Program data for that.
  SStringToOneT<UniquePtrRef<php::Unit>>   unitRefs;
  ISStringToOneT<UniquePtrRef<php::Class>> classRefs;
  ISStringToOneT<UniquePtrRef<php::Func>>  funcRefs;

  // Maps class name to the extern-worker ref representing the class's
  // associated ClassInfo2. Only has entries for instantiable classes.
  ISStringToOneT<UniquePtrRef<ClassInfo2>> classInfoRefs;

  std::unique_ptr<php::Program> program;

  ISStringToOneT<php::Class*>      classes;
  SStringToMany<php::Func>         methods;
  ISStringToOneT<php::Func*>       funcs;
  ISStringToOneT<php::TypeAlias*>  typeAliases;
  ISStringToOneT<php::Class*>      enums;
  SStringToOneT<php::Constant*>    constants;
  SStringToOneT<php::Module*>      modules;
  SStringToOneT<php::Unit*>        units;

  /*
   * Func families representing methods with a particular name (across
   * all classes).
   */
  struct MethodFamilyEntry {
    FuncFamilyOrSingle m_all;
    FuncFamilyOrSingle m_regular;
  };
  SStringToOneT<MethodFamilyEntry> methodFamilies;

  // Map from each class to all the closures that are allocated in
  // functions of that class.
  hphp_fast_map<
    const php::Class*,
    CompactVector<const php::Class*>
  > classClosureMap;

  hphp_fast_map<
    const php::Class*,
    hphp_fast_set<const php::Func*>
  > classExtraMethodMap;

  /*
   * Map from each class name to ClassInfo objects if one exists.
   *
   * It may not exists if we would fatal when defining the class. That could
   * happen for if the inheritance is bad or __Sealed or other things.
   */
  ISStringToOneT<ClassInfo*> classInfo;

  /*
   * All the ClassInfos, stored in no particular order.
   */
  std::vector<std::unique_ptr<ClassInfo>> allClassInfos;

  std::vector<FuncInfo> funcInfo;
  std::atomic<uint32_t> nextFuncId{};

  // Private instance and static property types are stored separately
  // from ClassInfo, because you don't need to resolve a class to get
  // at them.
  hphp_hash_map<
    const php::Class*,
    PropState
  > privatePropInfo;
  hphp_hash_map<
    const php::Class*,
    PropState
  > privateStaticPropInfo;

  /*
   * Public static property information:
   */

  // If this is true, we've seen mutations to public static
  // properties. Once this is true, it's no longer legal to report a
  // pessimistic static property set (unknown class and
  // property). Doing so is a monotonicity violation.
  bool seenPublicSPropMutations{false};

  // The set of gathered public static property mutations for each function. The
  // inferred types for the public static properties is the union of all these
  // mutations. If a function is not analyzed in a particular analysis round,
  // its mutations are left unchanged from the previous round.
  folly_concurrent_hash_map_simd<
    const php::Func*,
    PublicSPropMutations,
    pointer_hash<const php::Func>> publicSPropMutations;

  // All FuncFamilies. These are stored globally so we can avoid
  // generating duplicates.
  folly_concurrent_hash_map_simd<
    std::unique_ptr<FuncFamily>,
    bool,
    FuncFamilyPtrHasher,
    FuncFamilyPtrEquals
  > funcFamilies;

  folly_concurrent_hash_map_simd<
    std::unique_ptr<FuncFamily::StaticInfo>,
    bool,
    FFStaticInfoPtrHasher,
    FFStaticInfoPtrEquals
  > funcFamilyStaticInfos;

  /*
   * Map from interfaces to their assigned vtable slots, computed in
   * compute_iface_vtables().
   */
  IfaceSlotMap ifaceSlotMap;

  hphp_hash_map<
    const php::Class*,
    CompactVector<Type>
  > closureUseVars;

  struct ClsConstTypesHasher {
    bool operator()(const std::pair<const php::Class*, SString>& k) const {
      return hash_int64_pair(uintptr_t(k.first), k.second->hash());
    }
  };
  struct ClsConstTypesEquals {
    bool operator()(const std::pair<const php::Class*, SString>& a,
                    const std::pair<const php::Class*, SString>& b) const {
      return a.first == b.first && a.second->same(b.second);
    }
  };
  folly_concurrent_hash_map_simd<
    std::pair<const php::Class*, SString>,
    ClsConstInfo,
    ClsConstTypesHasher,
    ClsConstTypesEquals
  > clsConstTypes;

  // Cache for lookup_class_constant
  folly_concurrent_hash_map_simd<
    std::pair<const php::Class*, SString>,
    ClsConstLookupResult<>,
    ClsConstTypesHasher,
    ClsConstTypesEquals
  > clsConstLookupCache;

  bool useClassDependencies{};
  DepMap dependencyMap;

  /*
   * If a function is effect-free when called with a particular set of
   * literal arguments, and produces a literal result, there will be
   * an entry here representing the type.
   *
   * The map isn't just an optimization; we can't call
   * analyze_func_inline during the optimization phase, because the
   * bytecode could be modified while we do so.
   */
  ContextRetTyMap foldableReturnTypeMap;

  /*
   * Call-context sensitive return types are cached here.  This is not
   * an optimization.
   *
   * The reason we need to retain this information about the
   * calling-context-sensitive return types is that once the Index is
   * frozen (during the final optimization pass), calls to
   * lookup_return_type with a CallContext can't look at the bytecode
   * bodies of functions other than the calling function.  So we need
   * to know what we determined the last time we were alloewd to do
   * that so we can return it again.
   */
  ContextRetTyMap contextualReturnTypes{};

  /*
   * Lazily calculate the class that should be used for wait-handles.
   */
  LockFreeLazy<res::Class> lazyWaitHandleCls;

  std::thread compute_iface_vtables;
};

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

using IndexData = Index::IndexData;

std::mutex closure_use_vars_mutex;
std::mutex private_propstate_mutex;

DependencyContext make_dep(const php::Func* func) {
  return DependencyContext{DependencyContextType::Func, func};
}
DependencyContext make_dep(const php::Class* cls) {
  return DependencyContext{DependencyContextType::Class, cls};
}
DependencyContext make_dep(const php::Prop* prop) {
  return DependencyContext{DependencyContextType::Prop, prop};
}
DependencyContext make_dep(const FuncFamily* family) {
  return DependencyContext{DependencyContextType::FuncFamily, family};
}

DependencyContext dep_context(IndexData& data, const Context& ctx) {
  if (!ctx.cls || !data.useClassDependencies) return make_dep(ctx.func);
  auto const cls = ctx.cls->closureContextCls
    ? data.classes.at(ctx.cls->closureContextCls)
    : ctx.cls;
  if (is_used_trait(*cls)) return make_dep(ctx.func);
  return make_dep(cls);
}

template <typename T>
void add_dependency(IndexData& data,
                    T src,
                    const Context& dst,
                    Dep newMask) {
  if (data.frozen) return;

  auto d = dep_context(data, dst);
  DepMap::accessor acc;
  data.dependencyMap.insert(acc, make_dep(src));
  auto& current = acc->second[d];
  current = current | newMask;

  // We should only have a return type dependency on func families.
  assertx(
    IMPLIES(
      acc->first.tag() == DependencyContextType::FuncFamily,
      newMask == Dep::ReturnTy
    )
  );
}

std::mutex func_info_mutex;

FuncInfo* create_func_info(IndexData& data, const php::Func* f) {
  auto fi = &data.funcInfo[f->idx];
  if (UNLIKELY(fi->func == nullptr)) {
    if (f->isNative) {
      std::lock_guard<std::mutex> g{func_info_mutex};
      if (fi->func) {
        assertx(fi->func == f);
        return fi;
      }
      // We'd infer this anyway when we look at the bytecode body
      // (NativeImpl) for the HNI function, but just initializing it
      // here saves on whole-program iterations.
      fi->returnTy = native_function_return_type(f);
    }
    fi->func = f;
  }

  assertx(fi->func == f);
  return fi;
}

FuncInfo* func_info(IndexData& data, const php::Func* f) {
  auto const fi = &data.funcInfo[f->idx];
  return fi;
}

template <typename T>
void find_deps(IndexData& data,
               T src,
               Dep mask,
               DependencyContextSet& deps) {
  auto const srcDep = make_dep(src);

  // We should only ever have return type dependencies on func family.
  assertx(
    IMPLIES(
      srcDep.tag() == DependencyContextType::FuncFamily,
      mask == Dep::ReturnTy
    )
  );

  DepMap::const_accessor acc;
  if (data.dependencyMap.find(acc, srcDep)) {
    for (auto const& kv : acc->second) {
      if (has_dep(kv.second, mask)) deps.insert(kv.first);
    }
  }
}

//////////////////////////////////////////////////////////////////////

// Obtain the php::Func* represented by a MethRef. Doesn't work inside
// remote worker jobs.
const php::Func* func_from_meth_ref(const IndexData& index,
                                    const MethRef& meth) {
  auto const cls = index.classes.at(meth.cls);
  assertx(meth.idx < cls->methods.size());
  return cls->methods[meth.idx].get();
}

//////////////////////////////////////////////////////////////////////

struct TraitMethod {
  using class_type = std::pair<const ClassInfo2*, const php::Class*>;
  using method_type = const php::Func*;
  using origin_type = SString;

  TraitMethod(class_type trait_, method_type method_, Attr modifiers_)
      : trait(trait_)
      , method(method_)
      , modifiers(modifiers_)
    {}

  class_type trait;
  method_type method;
  Attr modifiers;
};

struct TMIOps {
  using string_type = LSString;
  using class_type = TraitMethod::class_type;
  using method_type = TraitMethod::method_type;
  using origin_type = TraitMethod::origin_type;

  struct TMIException : std::exception {
    explicit TMIException(std::string msg) : msg(msg) {}
    const char* what() const noexcept override { return msg.c_str(); }
  private:
    std::string msg;
  };

  // Return the name for the trait class.
  static const string_type clsName(class_type traitCls) {
    return traitCls.first->name;
  }

  // Return the name of the trait where the method was originally defined
  static origin_type originalClass(method_type meth) {
    return meth->originalClass ? meth->originalClass : meth->cls->name;
  }

  // Is-a methods.
  static bool isAbstract(Attr modifiers) {
    return modifiers & AttrAbstract;
  }

  // Whether to exclude methods with name `methName' when adding.
  static bool exclude(string_type methName) {
    return Func::isSpecial(methName);
  }

  // Errors.
  static void errorDuplicateMethod(class_type cls,
                                   string_type methName,
                                   const std::vector<const StringData*>&) {
    auto const& m = cls.second->methods;
    if (std::find_if(m.begin(), m.end(),
                     [&] (auto const& f) {
                       return f->name->same(methName);
                     }) != m.end()) {
      // the duplicate methods will be overridden by the class method.
      return;
    }
    throw TMIException(folly::sformat("DuplicateMethod: {}", methName));
  }
};

using TMIData = TraitMethodImportData<TraitMethod, TMIOps>;

//////////////////////////////////////////////////////////////////////

uint32_t func_num_inout(const php::Func* func) {
  if (!func->hasInOutArgs) return 0;
  uint32_t count = 0;
  for (auto& p : func->params) count += p.inout;
  return count;
}

TriBool func_supports_AER(const php::Func* func) {
  // Async functions always support async eager return, and no other
  // functions support it yet.
  return yesOrNo(func->isAsync && !func->isGenerator);
}

//////////////////////////////////////////////////////////////////////

template <typename T, typename R>
void add_symbol_to_index(R& map, T t, const char* type) {
  auto const name = t->name;
  auto const ret = map.emplace(name, std::move(t));
  always_assert_flog(
    ret.second,
    "More than one {} with the name {} "
    "(should have been caught by parser)",
    type,
    name
  );
}

template <typename T, typename R, typename E>
void add_symbol_to_index(R& map, T t, const char* type, const E& other) {
  auto const it = other.find(t->name);
  always_assert_flog(
    it == other.end(),
    "More than one symbol with the name {} "
    "(should have been caught by parser)",
    t->name
  );
  add_symbol_to_index(map, std::move(t), type);
}

// We want const qualifiers on various index data structures for php
// object pointers, but during index creation time we need to
// manipulate some of their attributes (changing the representation).
// This little wrapper keeps the const_casting out of the main line of
// code below.
void attribute_setter(const Attr& attrs, bool set, Attr attr) {
  attrSetter(const_cast<Attr&>(attrs), set, attr);
}

void add_system_constants_to_index(IndexData& index) {
  for (auto cnsPair : Native::getConstants()) {
    assertx(cnsPair.second.m_type != KindOfUninit ||
            cnsPair.second.dynamic());
    auto pc = new php::Constant {
      cnsPair.first,
      cnsPair.second,
      AttrUnique | AttrPersistent
    };
    add_symbol_to_index(index.constants, pc, "constant");
  }
}

void add_unit_to_index(IndexData& index, php::Unit& unit) {
  always_assert_flog(
    index.units.emplace(unit.filename, &unit).second,
    "More than one unit with the same name {} "
    "(should have been caught by parser)",
    unit.filename
  );

  for (auto& ta : unit.typeAliases) {
    add_symbol_to_index(
      index.typeAliases,
      ta.get(),
      "type alias",
      index.classes
    );
  }

  for (auto& c : unit.constants) {
    add_symbol_to_index(index.constants, c.get(), "constant");
  }

  for (auto& m : unit.modules) {
    add_symbol_to_index(index.modules, m.get(), "module");
  }
}

void add_class_to_index(IndexData& index, php::Class& c) {
  assertx(!(c.attrs & AttrNoOverride));
  assertx(!(c.attrs & AttrNoOverrideRegular));

  if (c.attrs & AttrEnum) {
    add_symbol_to_index(index.enums, &c, "enum");
  }

  add_symbol_to_index(index.classes, &c, "class", index.typeAliases);

  for (auto& m : c.methods) {
    attribute_setter(m->attrs, false, AttrNoOverride);
    index.methods.insert({m->name, m.get()});
    m->idx = index.nextFuncId++;
  }
}

void add_func_to_index(IndexData& index, php::Func& func) {
  add_symbol_to_index(index.funcs, &func, "function");
  func.idx = index.nextFuncId++;
}

void add_program_to_index(IndexData& index) {
  trace_time timer{"add program to index", index.sample};
  timer.ignore_client_stats();

  auto& program = *index.program;
  for (auto const& u : program.units) {
    add_unit_to_index(index, *u);
  }
  for (auto const& c : program.classes) {
    add_class_to_index(index, *c);
  }
  for (auto const& f : program.funcs) {
    add_func_to_index(index, *f);
  }

  for (auto const& c : program.classes) {
    if (!c->closureContextCls) continue;
    auto& s = index.classClosureMap[index.classes.at(c->closureContextCls)];
    s.emplace_back(c.get());
  }
}

//////////////////////////////////////////////////////////////////////

void compute_subclass_list_rec(ClassInfo* cinfo,
                               ClassInfo* csub) {
  if (csub->cls->attrs & AttrNoExpandTrait) return;
  for (auto const ctrait : csub->usedTraits) {
    auto const ct = const_cast<ClassInfo*>(ctrait);
    ct->subclassList.push_back(cinfo);
    compute_subclass_list_rec(cinfo, ct);
  }
}

void compute_subclass_list(IndexData& index) {
  trace_time _("compute subclass list", index.sample);

  for (auto& cinfo : index.allClassInfos) {
    for (auto& cparent : cinfo->baseList) {
      cparent->subclassList.push_back(cinfo.get());
    }
    compute_subclass_list_rec(cinfo.get(), cinfo.get());
    // Also add classes to their interface's subclassLists
    for (auto& ipair : cinfo->implInterfaces) {
      auto impl = const_cast<ClassInfo*>(ipair.second);
      impl->subclassList.emplace_back(cinfo.get());
    }
  }

  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      auto& sub = cinfo->subclassList;
      std::sort(begin(sub), end(sub));
      sub.erase(std::unique(begin(sub), end(sub)), end(sub));
      sub.shrink_to_fit();
      assertx(!cinfo->hasRegularSubclass);
      assertx(!cinfo->hasNonRegularSubclass);
      for (auto const s : sub) {
        if (s == cinfo.get()) continue;
        if (is_regular_class(*s->cls)) {
          cinfo->hasRegularSubclass = true;
        } else {
          cinfo->hasNonRegularSubclass = true;
        }
        if (cinfo->hasRegularSubclass && cinfo->hasNonRegularSubclass) break;
      }
    }
  );
}

//////////////////////////////////////////////////////////////////////

/*
 * Given a func list, canonicalize it, and return the appropriate
 * FuncFamilyOrSingle for both the entire list, and for the subset of
 * funcs on regular classes. If 'complete' is true, then the func list
 * is exhaustive list of possible resolutions for a lookup of
 * `cinfo::name` (this is propagated into the FuncFamilyOrSingle
 * entries). If `base` is nullptr, it's assumed we're building a
 * name-only method table. This only affects the "completeness" of the
 * regular subset result.
 *
 * The func list is assumed to only contain uniques (no multiple
 * entries with the same func, even if their "inRegular" status
 * differ).
 */
std::pair<FuncFamilyOrSingle, FuncFamilyOrSingle>
make_method_family_entry(IndexData& index,
                         FuncFamily::PFuncVec funcs,
                         const ClassInfo* base,
                         SString name,
                         bool complete) {
  // Name-only tables are never complete
  assertx(IMPLIES(!base, !complete));
  assertx(!funcs.empty());

  // Canonicalize the func list by sorting it. We've already assumed
  // the list only contains unique functions. We order functions which
  // are "inRegular" before ones which are not (this simplifies the
  // logic below). After that, we sort by class name.
  std::sort(
    begin(funcs), end(funcs),
    [] (FuncFamily::PossibleFunc a, const FuncFamily::PossibleFunc b) {
      if (a.inRegular() && !b.inRegular()) return true;
      if (!a.inRegular() && b.inRegular()) return false;
      return string_data_lti{}(a.ptr()->cls->name, b.ptr()->cls->name);
    }
  );
  funcs.shrink_to_fit();

  if (Trace::moduleEnabled(Trace::hhbbc_index, 4)) {
    FTRACE(4, "func family: {}::{} (for {}::{}):\n",
           funcs.front().ptr()->cls->name, name,
           base ? base->cls->name->data() : "*", name);
    for (auto const DEBUG_ONLY func : funcs) {
      FTRACE(
        4, "  {}::{}{}\n",
        func.ptr()->cls->name, func.ptr()->name,
        func.inRegular() ? "" : " (not regular)"
      );
    }
  }

  // Create a func family for this func list, allocating space for the
  // "extra" results for the regular subset. If the func family
  // already exists, that's returned.
  auto const ff = [&] (bool extra) {
    assertx(funcs.size() > 1);
    auto it = index.funcFamilies.find(funcs);
    if (it != index.funcFamilies.end()) return it->first.get();
    return index.funcFamilies.insert(
      std::make_unique<FuncFamily>(std::move(funcs), extra),
      false
    ).first->first.get();
  };

  if (funcs.size() > 1) {
    if (funcs.back().inRegular()) {
      // The list is sorted so that all the "inRegular" funcs come
      // first. If the last func is "inRegular", then every func on
      // the list is like that. Therefore there's no need to allocate
      // space to distinguish the regular subset.
      auto const funcFamily = ff(false);
      return std::make_pair(
        FuncFamilyOrSingle{funcFamily, !complete},
        FuncFamilyOrSingle{funcFamily, !base}
      );
    } else if (!funcs.front().inRegular()) {
      // On the other hand, if the first func isn't "inRegular", then
      // none of them are. Again, no need to allocate space to
      // distinguish the regular subset since the "regular" result is
      // empty.
      return std::make_pair(
        FuncFamilyOrSingle{ff(false), !complete},
        FuncFamilyOrSingle{}
      );
    } else if (!funcs[1].inRegular()) {
      // The first func is "inRegular", but the 2nd one isn't. This
      // means only one func (the first one) is "inRegular", so the
      // regular subset result is just that func. Since the regular
      // result isn't using the func family, we don't need the extra
      // space.
      auto const f = funcs.front().ptr();
      return std::make_pair(
        FuncFamilyOrSingle{ff(false), !complete},
        FuncFamilyOrSingle{f, !base}
      );
    } else {
      // For the rest of the cases, we have a mix of "inRegular" and
      // not "inRegular". Both get the func family, but we need to
      // allocate extra space because walking the list may produce
      // different results depending on whether you're filtering on
      // the regular subset.
      auto const funcFamily = ff(true);
      return std::make_pair(
        FuncFamilyOrSingle{funcFamily, !complete},
        FuncFamilyOrSingle{funcFamily, !base}
      );
    }
  }

  // There's only one func on the list, so no func family is
  // necessary.
  assertx(funcs.size() == 1);
  auto const f = funcs.front();
  if (f.inRegular()) {
    // The single func is on a regular class, so it applies to both
    // all and the regular subset.
    return std::make_pair(
      FuncFamilyOrSingle{f.ptr(), !complete},
      FuncFamilyOrSingle{f.ptr(), !base}
    );
  } else {
    // The single func isn't on any regular class, so it applies to
    // only the all result. The regular subset is empty.
    return std::make_pair(
      FuncFamilyOrSingle{f.ptr(), !complete},
      FuncFamilyOrSingle{}
    );
  }
}

/*
 * Iterate over the subclasses of `base` and construct
 * FuncFamilyOrSingle entries for the set of methods with
 * `name`. Entries for both all entries, and for just the subset on
 * regular classes will be returned.  If `justRegular` is true, then
 * the caller is only interested in the regular class subset (in which
 * case both results will be the same).
 */
std::pair<FuncFamilyOrSingle, FuncFamilyOrSingle>
define_func_family(IndexData& index,
                   const ClassInfo& base,
                   SString name,
                   bool justRegular) {
  hphp_fast_map<const php::Func*, bool> funcSet;
  // Whether there's any sub-classes which don't have a method with
  // the name.
  auto complete = true;

  for (auto const sub : base.subclassList) {
    // Ignore non-regular classes if we only want the regular subset.
    if (justRegular && !is_regular_class(*sub->cls)) continue;
    auto const it = sub->methods.find(name);
    if (it == end(sub->methods)) {
      // This subclass doesn't have the method. This can only happen
      // if the base class is an interface.  They don't get inherited,
      // nor does an interface inherit anything (this can also happen
      // with special methods, but we explicitly don't handle
      // those). Mark this list as being non-complete.
      assertx(base.cls->attrs & AttrInterface);
      complete = false;
      continue;
    }
    auto const& mte = it->second;
    auto const f = func_from_meth_ref(index, mte.meth());
    auto const isRegular =
      justRegular ||
      /* A private method on a non-regular class might still be
       * callable in a "regular only" context. */
      ((mte.attrs & AttrPrivate) && mte.topLevel()) ||
      is_regular_class(*sub->cls);
    funcSet[f] |= isRegular;
  }

  if (funcSet.empty()) {
    // If there wasn't any functions found, we should have encountered
    // at least one subclass which didn't contain it. If not, it means
    // the method should have been marked as not being overridden and
    // we shouldn't be trying to construct a method family for it.
    assertx(!complete);
    return std::make_pair(
      FuncFamilyOrSingle{},
      FuncFamilyOrSingle{}
    );
  }

  FuncFamily::PFuncVec funcs;
  funcs.reserve(funcSet.size());
  auto allRegular = true;
  for (auto const& [func, inRegular] : funcSet) {
    allRegular &= inRegular;
    funcs.emplace_back(func, inRegular);
  }
  assertx(IMPLIES(justRegular, allRegular));

  // Make FuncFamilyOrSingle entries for the entire list and the
  // regular subset.
  auto const [all, regular] = make_method_family_entry(
    index,
    std::move(funcs),
    &base,
    name,
    complete
  );

  // We should have gotten at least something for "all" and it's
  // incompleteness should reflect that was passed to
  // make_method_family_entry.
  assertx(!all.isEmpty());
  assertx(all.isIncomplete() == !complete);

  // "regular" is always complete (or empty which is neither complete
  // nor incomplete).
  assertx(regular.isEmpty() || regular.isComplete());

  // "regular" is always a subset of "all". If "all" is a single func,
  // "regular" must be a single func or empty.
  assertx(IMPLIES(all.func(), !regular.funcFamily()));

  // If "all" and "regular" both have func families, they must be the
  // same func family.
  assertx(IMPLIES(all.funcFamily() && regular.funcFamily(),
                  all.funcFamily() == regular.funcFamily()));
  // If "all" and "regular" both are single funcs, it must be the same
  // single func.
  assertx(IMPLIES(all.func() && regular.func(),
                  all.func() == regular.func()));

  // If "all" has a func family, but "regular" does not, then that
  // func family should not have allocated space for regular results.
  assertx(IMPLIES(all.funcFamily() && !regular.funcFamily(),
                  !all.funcFamily()->m_regular));

  // If we only saw regular classes, then "all" and "regular" should
  // be the same.
  assertx(IMPLIES(allRegular, all.funcFamily() == regular.funcFamily()));
  assertx(IMPLIES(allRegular, all.func() == regular.func()));
  // Likewise, if allRegular was specified, then the list contains
  // nothing but methods on regular classes, so we shouldn't have
  // allocated the extra space on the func family (there's no point,
  // the results for regular/non-regular will always be identical).
  assertx(
    IMPLIES(
      allRegular && regular.funcFamily(),
      !regular.funcFamily()->m_regular
    )
  );

  return std::make_pair(all, regular);
}

/*
 * Calculate FuncFamilyOrSingle entries corresponding *all* methods in
 * the program with the given name and insert them in the global
 * name-only method family map.
 */
void define_name_only_func_family(IndexData& index, SString name) {
  hphp_fast_map<const php::Func*, bool> funcSet;

  auto const range = index.methods.equal_range(name);
  for (auto it = range.first; it != range.second; ++it) {
    auto const func = it->second;
    assertx(func->cls);
    // Only include methods for classes which have a ClassInfo,
    // which means they're instantiatable.
    auto const cinfoIt = index.classInfo.find(func->cls->name);
    if (cinfoIt == index.classInfo.end()) continue;
    auto const cinfo = cinfoIt->second;
    if (!cinfo->methods.count(name)) continue;
    // We need to know if the method is present *anywhere* on a
    // regular class (not just the class which defines it). We already
    // calculated this earlier when we built the normal method family
    // data.
    auto const inRegular = func_info(index, func)->regularClassMethod.load();
    funcSet[func] |= inRegular;
    // If we asserted that this method isn't used anywhere by a
    // regular class, it's defining class shouldn't be regular.
    assertx(IMPLIES(!inRegular, !is_regular_class(*func->cls)));
  }
  // The set can be empty if every class which defines a method with
  // that name doesn't have a ClassInfo. If so, bail out.
  if (funcSet.empty()) return;

  FuncFamily::PFuncVec funcs;
  funcs.reserve(funcSet.size());
  auto allRegular = true;
  for (auto const& [func, inRegular] : funcSet) {
    allRegular &= inRegular;
    funcs.emplace_back(func, inRegular);
  }

  // Create the entries
  auto const [all, regular] = make_method_family_entry(
    index,
    std::move(funcs),
    nullptr,
    name,
    false
  );
  FTRACE(
    4,
    "method family *::{}:\n"
    "  all: {}\n"
    "  regular: {}\n",
    name, show(all), show(regular)
  );

  // Shouldn't have an empty entry because we know there's at least
  // one method (we made sure the func list was non-empty above).
  assertx(!all.isEmpty());
  // Name-only tables are always incomplete
  assertx(all.isIncomplete());
  // "regular" is always incomplete (like "all"), except if it's empty
  // (which is neither incomplete nor complete).
  assertx(regular.isEmpty() || regular.isIncomplete());

  // If both "all" and "regular" contain a func family, they must be
  // the same func family.
  assertx(IMPLIES(all.funcFamily() && regular.funcFamily(),
                  all.funcFamily() == regular.funcFamily()));
  // If both "all" and "regular" contain a single func, it must be the
  // same func.
  assertx(IMPLIES(all.func() && regular.func(), all.func() == regular.func()));

  // "regular" is a subset of "all", so if "all" is a func, then
  // "regular" must be a func or empty.
  assertx(IMPLIES(all.func(), regular.func() || regular.isEmpty()));
  // If "all" has a func family and "regular" does not, then the func
  // family shouldn't have extra space allocated for the regular
  // subset result.
  assertx(IMPLIES(all.funcFamily() && !regular.funcFamily(),
                  !all.funcFamily()->m_regular));

  // If we observed that all of the methods are on regular classes,
  // then "all" and "regular" should be the same. Moreover, if there's
  // a func family, that func family should not have space allocated
  // for the non-regular subset (there's no point since the results
  // will always be the same).
  assertx(IMPLIES(allRegular, all.funcFamily() == regular.funcFamily()));
  assertx(IMPLIES(allRegular, all.func() == regular.func()));
  assertx(
    IMPLIES(
      allRegular && regular.funcFamily(),
      !regular.funcFamily()->m_regular
    )
  );

  // No fancy logic here to avoid redundant entries. We don't have the
  // AttrNoOverride or noOverrideRegular bits to help us, and there's
  // not enough name-only entries for the space savings to matter.
  auto const famIt = index.methodFamilies.find(name);
  // An entry in the table should have been pre-created for us (this
  // lets us avoid mutating the map from multiple threads), and
  // shouldn't have had an entry already.
  assertx(famIt != end(index.methodFamilies));
  assertx(famIt->second.m_all.isEmpty());
  assertx(famIt->second.m_regular.isEmpty());
  famIt->second.m_all = all;
  famIt->second.m_regular = regular;
}

void expand_abstract_func_families(IndexData& index, ClassInfo* cinfo) {
  assertx(cinfo->cls->attrs & (AttrInterface|AttrAbstract));

  /*
   * Interfaces can cause monotonicity violations. Suppose we have two
   * interfaces: I2 and I2. I1 declares a method named Foo. Every
   * class which implements I2 also implements I1 (therefore I2
   * implies I1). During analysis, a type is initially Obj<=I1 and we
   * resolve a call to Foo using I1's func families. After further
   * optimization, we narrow the type to Obj<=I2. Now when we go to
   * resolve a call to Foo using I2's func families, we find
   * nothing. Foo is declared in I1, not in I2, and interface methods
   * are not inherited. We use the fall back name-only tables, which
   * might give us a worse type than before. This is a monotonicity
   * violation because refining the object type gave us worse
   * analysis.
   *
   * To avoid this, we expand an interface's (and abstract class'
   * which has similar issues) func families to include all methods
   * defined by *all* of it's (regular) implementations. So, in the
   * example above, we'd expand I2's func families to include Foo,
   * since all of I2's implements should define a Foo method (since
   * they also all implement I1).
   */
  hphp_fast_set<SString> extras;

  // First find a regular subclass we can use to initialize our list.
  auto it = begin(cinfo->subclassList);
  while (true) {
    if (it == end(cinfo->subclassList)) return;
    auto const sub = *it++;
    if (!is_regular_class(*sub->cls)) continue;
    assertx(sub != cinfo);
    for (auto const& elem : sub->methods) {
      // Don't include ctors or special methods. Not useful.
      if (elem.first == s_construct.get() ||
          is_special_method_name(elem.first)) {
        continue;
      }
      if (elem.second.hasPrivateAncestor()) continue;
      if (cinfo->methods.count(elem.first)) continue;
      if (cinfo->methodFamilies.count(elem.first)) continue;
      extras.emplace(elem.first);
    }
    if (extras.empty()) return;
    break;
  }

  // Then iterate over the rest and remove methods which aren't
  // implemented in every subclass.
  while (it != end(cinfo->subclassList)) {
    auto const sub = *it++;
    if (!is_regular_class(*sub->cls)) continue;
    assertx(sub != cinfo);
    folly::erase_if(
      extras,
      [&] (SString meth) {
        auto const it = sub->methods.find(meth);
        if (it == end(sub->methods)) return true;
        return it->second.hasPrivateAncestor();
      }
    );
    if (extras.empty()) return;
  }

  if (Trace::moduleEnabled(Trace::hhbbc_index, 4)) {
    FTRACE(4, "Adding extra func-families to {}:\n", cinfo->cls->name);
    for (auto const DEBUG_ONLY extra : extras) {
      FTRACE(4, "  {}\n", extra);
    }
  }

  std::vector<std::pair<SString, FuncFamilyOrSingle>> expanded;
  for (auto const name : extras) {
    // We only care about methods on regular classes, so pass
    // "justRegular" as true here. We should only get results for the
    // regular subset.
    auto const [a, r] = define_func_family(index, *cinfo, name, true);
    // "all" and "regular" should be identical here, since we asked to
    // only consider regular classes. We already know these methods
    // are present on all regular subclasses.
    assertx(!a.isEmpty());
    assertx(!r.isEmpty());
    assertx(a.isComplete());
    assertx(r.isComplete());
    assertx(a.funcFamily() == r.funcFamily());
    assertx(a.func() == r.func());
    FTRACE(
      4,
      "extra method family {}::{}: {}\n",
      cinfo->cls->name, name, show(r)
    );
    assertx(!cinfo->methodFamilies.count(name));
    expanded.emplace_back(name, r);
  }

  // Sort the list so we can insert it into methodFamilies (which is a
  // sorted_vector_map) in bulk.
  assertx(!expanded.empty());
  std::sort(
    begin(expanded), end(expanded),
    [] (auto const& p1, auto const& p2) { return p1.first < p2.first; }
  );
  cinfo->methodFamilies.insert(
    folly::sorted_unique, begin(expanded), end(expanded)
  );
}

// Calculate the StaticInfo for the given FuncFamily, and assign it
// the pointer to the unique allocation corresponding to it. If `all`
// is true, then all possible funcs should be considered. Otherwise,
// only the regular subset will.
void build_func_family_static_info(IndexData& index, FuncFamily* ff, bool all) {
  auto const& possible = ff->possibleFuncs();
  assertx(possible.size() > 1);

  // Calculate the StaticInfo from all possible functions:

  auto info =  [&] {
    FuncFamily::StaticInfo info;

    // Find the first func, taking into account whether we care about
    // non-regular funcs or not.
    auto const func = [&] {
      for (auto const pf : possible) {
        if (all || pf.inRegular()) return pf.ptr();
      }
      always_assert(false);
    }();

    info.m_numInOut = func_num_inout(func);
    info.m_isReadonlyReturn = yesOrNo(func->isReadonlyReturn);
    info.m_isReadonlyThis = yesOrNo(func->isReadonlyThis);
    info.m_maybeReified = func->isReified;
    info.m_maybeCaresAboutDynCalls = (dyn_call_error_level(func) > 0);
    info.m_maybeBuiltin = (func->attrs & AttrBuiltin);
    info.m_minNonVariadicParams =
      info.m_maxNonVariadicParams = numNVArgs(*func);
    info.m_requiredCoeffects = func->requiredCoeffects;
    info.m_coeffectRules = func->coeffectRules;
    info.m_supportsAER = func_supports_AER(func);

    for (size_t i = 0; i < func->params.size(); ++i) {
      info.m_paramPreps.emplace_back(func_param_prep(func, i));
    }
    return info;
  }();

  auto const addToParamPreps = [&] (const php::Func* f) {
    if (f->params.size() > info.m_paramPreps.size()) {
      info.m_paramPreps.resize(
        f->params.size(),
        PrepKind{TriBool::No, TriBool::No}
      );
    }
    for (size_t i = 0; i < info.m_paramPreps.size(); ++i) {
      auto const prep = func_param_prep(f, i);
      info.m_paramPreps[i].inOut |= prep.inOut;
      info.m_paramPreps[i].readonly |= prep.readonly;
    }
  };

  for (auto const& pf : possible) {
    // Skip non-regular funcs if we don't care about those.
    if (!all && !pf.inRegular()) continue;
    auto const func = pf.ptr();

    if (info.m_numInOut && *info.m_numInOut != func_num_inout(func)) {
      info.m_numInOut.reset();
    }
    info.m_isReadonlyReturn |= yesOrNo(func->isReadonlyReturn);
    info.m_isReadonlyThis |= yesOrNo(func->isReadonlyThis);
    info.m_maybeReified |= func->isReified;
    info.m_maybeCaresAboutDynCalls |= (dyn_call_error_level(func) > 0);
    info.m_maybeBuiltin |= (func->attrs & AttrBuiltin);
    addToParamPreps(func);

    if (info.m_supportsAER != TriBool::Maybe) {
      info.m_supportsAER |= func_supports_AER(func);
    }

    auto const numNV = numNVArgs(*func);
    info.m_minNonVariadicParams =
      std::min(info.m_minNonVariadicParams, numNV);
    info.m_maxNonVariadicParams =
      std::max(info.m_maxNonVariadicParams, numNV);

    if (info.m_requiredCoeffects &&
        *info.m_requiredCoeffects != func->requiredCoeffects) {
      info.m_requiredCoeffects.reset();
    }

    if (info.m_coeffectRules) {
      if (!std::is_permutation(
            info.m_coeffectRules->begin(),
            info.m_coeffectRules->end(),
            func->coeffectRules.begin(),
            func->coeffectRules.end())) {
        info.m_coeffectRules.reset();
      }
    }
  }

  // Modify the info to make it more likely to match an existing one:

  // Any param beyond the size of m_paramPreps is implicitly
  // TriBool::No, so we can drop trailing entries which are
  // TriBool::No.
  while (!info.m_paramPreps.empty()) {
    auto& back = info.m_paramPreps.back();
    if (back.inOut != TriBool::No || back.readonly != TriBool::No) break;
    info.m_paramPreps.pop_back();
  }

  // Sort the coeffect rules to increase matching.
  if (info.m_coeffectRules) {
    std::sort(info.m_coeffectRules->begin(), info.m_coeffectRules->end());
  }

  // See if the info already exists in the set. If it doesn't exist,
  // add it. Otherwise use the already created one.
  auto const staticInfo = [&] {
    auto const it = index.funcFamilyStaticInfos.find(info);
    if (it != index.funcFamilyStaticInfos.end()) return it->first.get();
    return index.funcFamilyStaticInfos.insert(
      std::make_unique<FuncFamily::StaticInfo>(std::move(info)),
      false
    ).first->first.get();
  }();

  // Set the static info in the appropriate place. If we asked for the
  // regular subset, we should have space allocated to put it there.
  if (all) {
    ff->m_all.m_static = staticInfo;
  } else {
    assertx(ff->m_regular);
    ff->m_regular->m_static = staticInfo;
  }
}

void define_func_families(IndexData& index) {
  trace_time tracer("define_func_families", index.sample);

  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      std::vector<std::pair<SString, FuncFamilyOrSingle>> entries;
      std::vector<std::pair<SString, FuncFamilyOrSingle>> aux;

      assertx(cinfo->methodFamilies.empty());
      assertx(cinfo->methodFamiliesAux.empty());

      for (auto const& [name, mte] : cinfo->methods) {
        auto const func = func_from_meth_ref(index, mte.meth());

        // If this is a regular class, record that this func is used
        // by a regular class. This will be used when building the
        // name-only tables below. Private methods on non-regular
        // classes can be called even from regular contexts, to
        // include those as well.
        if (is_regular_class(*cinfo->cls) ||
            ((mte.attrs & AttrPrivate) && mte.topLevel())) {
          create_func_info(index, func)->regularClassMethod = true;
        }

        // Don't construct func families for special methods, as it's
        // not particularly useful.
        if (is_special_method_name(name)) continue;
        // If the method is known not to be overridden, we don't need
        // a func family.
        if (mte.attrs & AttrNoOverride) continue;

        auto const [a, r] = define_func_family(
          index, *cinfo, name, false
        );

        if (is_regular_class(*cinfo->cls)) {
          // We know that AttrNoOverride is false, therefore the
          // method must be overridden in a subclass, so we have to
          // get at least 2 funcs (so a FuncFamily).
          assertx(a.funcFamily());
          // Since this class is regular, this method should at least
          // be on the regular subset results.
          assertx(!r.isEmpty());
          // We can only get incomplete results if this is an
          // interface.
          assertx(a.isComplete());
          assertx(r.isComplete());

          // If there's no override by a regular class, the only func
          // on the regular subset should be the method on this
          // class. Otherwise, we should have a func family, and "all"
          // and "regular" should always have the same func family.
          if (mte.noOverrideRegular()) {
            assertx(r.func() == func);
          } else {
            assertx(r.funcFamily() == a.funcFamily());
          }

          FTRACE(4, "method family {}::{}: {}\n",
                 cinfo->cls->name, name, show(a));
          entries.emplace_back(name, a);
        } else {
          // The method is on this class, so the results for "all"
          // should have at least have it (so cannot be empty).
          assertx(!a.isEmpty());
          // Result can be incomplete, but only if this is an
          // interface.
          assertx(IMPLIES(a.isIncomplete(), cinfo->cls->attrs & AttrInterface));
          // If we got a single func, it should be the func on this
          // class. The only way we'd get a single func is if the
          // results are incomplete.
          assertx(IMPLIES(a.func(), a.isIncomplete()));
          assertx(IMPLIES(a.func(), a.func() == func));

          // If there's no override by a regular class, we'll either
          // have an empty set, or a single func the same as this
          // class' func. If there's no regular subclasses, it will be
          // empty. Otherwise it will be the single func (we know it's
          // not overridden so all the existing regular subclasses
          // must have the same func).
          if (mte.noOverrideRegular()) {
            assertx(r.isEmpty() || r.func() == func);
            assertx(IMPLIES(r.isEmpty(), !cinfo->hasRegularSubclass));
          } else {
            // It's possible to get an empty or incomplete set for the
            // regular subset if this class is an interface.
            assertx(IMPLIES(r.isEmpty(), cinfo->cls->attrs & AttrInterface));
            assertx(
              IMPLIES(r.isIncomplete(), cinfo->cls->attrs & AttrInterface)
            );
            // Since it was overridden it's either incomplete, or if
            // it's a single func, a different func than the one in
            // this class.
            assertx(
              r.isIncomplete() ||
              ((mte.attrs & AttrPrivate) && mte.topLevel()) ||
              r.func() != func
            );
          }

          // Only bother making an aux entry if it's different from
          // the normal entry, and if we'll actually use it (if
          // noOverrideRegular, we won't even check).
          auto const addAux =
            !mte.noOverrideRegular() &&
            (r.isIncomplete() ||
             a.funcFamily() != r.funcFamily() ||
             a.func() != r.func());

          FTRACE(
            4, "method family {}::{}:\n"
            "  all: {}\n"
            "{}",
            cinfo->cls->name, name,
            show(a),
            addAux ? folly::sformat("  aux: {}\n", show(r)) : ""
          );
          entries.emplace_back(name, a);
          if (addAux) aux.emplace_back(name, r);
        }
      }

      // Sort the lists of new entries, so we can insert them into the
      // method family maps (which are sorted_vector_maps) in bulk.
      std::sort(
        begin(entries), end(entries),
        [] (auto const& p1, auto const& p2) { return p1.first < p2.first; }
      );
      std::sort(
        begin(aux), end(aux),
        [] (auto const& p1, auto const& p2) { return p1.first < p2.first; }
      );
      if (!entries.empty()) {
        cinfo->methodFamilies.insert(
          folly::sorted_unique, begin(entries), end(entries)
        );
      }
      if (!aux.empty()) {
        cinfo->methodFamiliesAux.insert(
          folly::sorted_unique, begin(aux), end(aux)
        );
      }

      // If this is an interface or abstract class, we may want to add
      // additional entries to maintain monotonicity.
      if (cinfo->cls->attrs & (AttrInterface|AttrAbstract)) {
        expand_abstract_func_families(index, cinfo.get());
      }

      cinfo->methodFamilies.shrink_to_fit();
      cinfo->methodFamiliesAux.shrink_to_fit();
    }
  );

  // Then calculate func families for methods with particular names
  // across all classes:
  {
    // Build a list of all unique method names. Pre-allocate entries
    // in methodFamilies for each one. This lets us insert into the
    // map from multiple threads safely (we don't have to mutate the
    // actual map).
    std::vector<SString> allMethods;
    for (auto const& [name, _] : index.methods) {
      // index.methods is a multi-map, so we might have multiple
      // entries with the same name (but they'll be contiguous).
      if (!allMethods.empty() && allMethods.back() == name) continue;
      // We don't bother with constructing name-only func families for
      // constructors, or special methods because it's not
      // particularly useful.
      if (name == s_construct.get() || is_special_method_name(name)) continue;
      allMethods.emplace_back(name);
      always_assert(
        index.methodFamilies.emplace(name, IndexData::MethodFamilyEntry{}).second
      );
    }

    // Populate the maps
    parallel::for_each(
      allMethods,
      [&] (SString m) { define_name_only_func_family(index, m); }
    );

    // Now clean any empty entries out of the maps. These correspond
    // to method names which didn't end up in any func families.
    folly::erase_if(
      index.methodFamilies,
      [&] (const std::pair<SString, IndexData::MethodFamilyEntry>& p) {
        return p.second.m_all.isEmpty() && p.second.m_regular.isEmpty();
      }
    );
  }

  // Now that all of the FuncFamilies have been created, generate the
  // back links from FuncInfo to their FuncFamilies.
  std::vector<FuncFamily*> work;
  work.reserve(index.funcFamilies.size());
  for (auto const& kv : index.funcFamilies) work.emplace_back(kv.first.get());

  // Different threads can touch the same FuncInfo, so use sharded
  // locking scheme.
  std::array<std::mutex, 256> locks;
  parallel::for_each(
    work,
    [&] (FuncFamily* ff) {
      build_func_family_static_info(index, ff, true);
      if (ff->m_regular) build_func_family_static_info(index, ff, false);
      for (auto const pf : ff->possibleFuncs()) {
        auto finfo = create_func_info(index, pf.ptr());
        auto& lock = locks[pointer_hash<FuncInfo>{}(finfo) % locks.size()];
        std::lock_guard<std::mutex> _{lock};
        finfo->families.emplace_back(ff);
      }
    }
  );

  parallel::for_each(
    index.funcInfo,
    [&] (FuncInfo& fi) { fi.families.shrink_to_fit(); }
  );
}

/*
 * ConflictGraph maintains lists of interfaces that conflict with each other
 * due to being implemented by the same class.
 */
struct ConflictGraph {
  void add(const php::Class* i, const php::Class* j) {
    if (i == j) return;
    map[i].insert(j);
  }

  hphp_fast_map<const php::Class*,
                hphp_fast_set<const php::Class*>> map;
};

/*
 * Trace information about interface conflict sets and the vtables computed
 * from them.
 */
void trace_interfaces(const IndexData& index, const ConflictGraph& cg) {
  // Compute what the vtable for each Class will look like, and build up a list
  // of all interfaces.
  struct Cls {
    const ClassInfo* cinfo;
    std::vector<const php::Class*> vtable;
  };
  std::vector<Cls> classes;
  std::vector<const php::Class*> ifaces;
  size_t total_slots = 0, empty_slots = 0;
  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->cls->attrs & AttrInterface) {
      ifaces.emplace_back(cinfo->cls);
      continue;
    }
    if (cinfo->cls->attrs & (AttrTrait | AttrEnum | AttrEnumClass)) continue;

    classes.emplace_back(Cls{cinfo.get()});
    auto& vtable = classes.back().vtable;
    for (auto& pair : cinfo->implInterfaces) {
      auto it = index.ifaceSlotMap.find(pair.second->cls);
      assertx(it != end(index.ifaceSlotMap));
      auto const slot = it->second;
      if (slot >= vtable.size()) vtable.resize(slot + 1);
      vtable[slot] = pair.second->cls;
    }

    total_slots += vtable.size();
    for (auto iface : vtable) if (iface == nullptr) ++empty_slots;
  }

  Slot max_slot = 0;
  for (auto const& pair : index.ifaceSlotMap) {
    max_slot = std::max(max_slot, pair.second);
  }

  // Sort the list of class vtables so the largest ones come first.
  auto class_cmp = [&](const Cls& a, const Cls& b) {
    return a.vtable.size() > b.vtable.size();
  };
  std::sort(begin(classes), end(classes), class_cmp);

  // Sort the list of interfaces so the biggest conflict sets come first.
  auto iface_cmp = [&](const php::Class* a, const php::Class* b) {
    return cg.map.at(a).size() > cg.map.at(b).size();
  };
  std::sort(begin(ifaces), end(ifaces), iface_cmp);

  std::string out;
  folly::format(&out, "{} interfaces, {} classes\n",
                ifaces.size(), classes.size());
  folly::format(&out,
                "{} vtable slots, {} empty vtable slots, max slot {}\n",
                total_slots, empty_slots, max_slot);
  folly::format(&out, "\n{:-^80}\n", " interface slots & conflict sets");
  for (auto iface : ifaces) {
    auto cgIt = cg.map.find(iface);
    if (cgIt == end(cg.map)) break;
    auto& conflicts = cgIt->second;

    folly::format(&out, "{:>40} {:3} {:2} [", iface->name,
                  conflicts.size(),
                  folly::get_default(index.ifaceSlotMap, iface));
    auto sep = "";
    for (auto conflict : conflicts) {
      folly::format(&out, "{}{}", sep, conflict->name);
      sep = ", ";
    }
    folly::format(&out, "]\n");
  }

  folly::format(&out, "\n{:-^80}\n", " class vtables ");
  for (auto& item : classes) {
    if (item.vtable.empty()) break;

    folly::format(&out, "{:>30}: [", item.cinfo->cls->name);
    auto sep = "";
    for (auto iface : item.vtable) {
      folly::format(&out, "{}{}", sep, iface ? iface->name->data() : "null");
      sep = ", ";
    }
    folly::format(&out, "]\n");
  }

  Trace::traceRelease("%s", out.c_str());
}

/*
 * Find the lowest Slot that doesn't conflict with anything in the conflict set
 * for iface.
 */
Slot find_min_slot(const php::Class* iface,
                   const IfaceSlotMap& slots,
                   const ConflictGraph& cg) {
  auto const& cit = cg.map.find(iface);
  if (cit == cg.map.end() || cit->second.empty()) {
    // No conflicts. This is the only interface implemented by the classes that
    // implement it.
    return 0;
  }

  boost::dynamic_bitset<> used;

  for (auto const& c : cit->second) {
    auto const it = slots.find(c);
    if (it == slots.end()) continue;
    auto const slot = it->second;

    if (used.size() <= slot) used.resize(slot + 1);
    used.set(slot);
  }
  used.flip();
  return used.any() ? used.find_first() : used.size();
}

/*
 * Compute vtable slots for all interfaces. No two interfaces implemented by
 * the same class will share the same vtable slot.
 */
void compute_iface_vtables(IndexData& index) {
  trace_time tracer("compute interface vtables", index.sample);

  ConflictGraph cg;
  std::vector<const php::Class*>             ifaces;
  hphp_fast_map<const php::Class*, int> iface_uses;

  // Build up the conflict sets.
  for (auto& cinfo : index.allClassInfos) {
    // Gather interfaces.
    if (cinfo->cls->attrs & AttrInterface) {
      ifaces.emplace_back(cinfo->cls);
      // Make sure cg.map has an entry for every interface - this simplifies
      // some code later on.
      cg.map[cinfo->cls];
      continue;
    }

    // Only worry about classes with methods that can be called.
    if (cinfo->cls->attrs & (AttrTrait | AttrEnum | AttrEnumClass)) continue;

    for (auto& ipair : cinfo->implInterfaces) {
      ++iface_uses[ipair.second->cls];
      for (auto& jpair : cinfo->implInterfaces) {
        cg.add(ipair.second->cls, jpair.second->cls);
      }
    }
  }

  if (ifaces.size() == 0) return;

  // Sort interfaces by usage frequencies.
  // We assign slots greedily, so sort the interface list so the most
  // frequently implemented ones come first.
  auto iface_cmp = [&](const php::Class* a, const php::Class* b) {
    return iface_uses[a] > iface_uses[b];
  };
  std::sort(begin(ifaces), end(ifaces), iface_cmp);

  // Assign slots, keeping track of the largest assigned slot and the total
  // number of uses for each slot.
  Slot max_slot = 0;
  hphp_fast_map<Slot, int> slot_uses;
  for (auto* iface : ifaces) {
    auto const slot = find_min_slot(iface, index.ifaceSlotMap, cg);
    index.ifaceSlotMap[iface] = slot;
    max_slot = std::max(max_slot, slot);

    // Interfaces implemented by the same class never share a slot, so normal
    // addition is fine here.
    slot_uses[slot] += iface_uses[iface];
  }

  // Make sure we have an initialized entry for each slot for the sort below.
  for (Slot slot = 0; slot < max_slot; ++slot) {
    assertx(slot_uses.count(slot));
  }

  // Finally, sort and reassign slots so the most frequently used slots come
  // first. This slightly reduces the number of wasted vtable vector entries at
  // runtime.
  auto const slots = sort_keys_by_value(
    slot_uses,
    [&] (int a, int b) { return a > b; }
  );

  std::vector<Slot> slots_permute(max_slot + 1, 0);
  for (size_t i = 0; i <= max_slot; ++i) slots_permute[slots[i]] = i;

  // re-map interfaces to permuted slots
  for (auto& pair : index.ifaceSlotMap) {
    pair.second = slots_permute[pair.second];
  }

  if (Trace::moduleEnabledRelease(Trace::hhbbc_iface)) {
    trace_interfaces(index, cg);
  }
}

void find_mocked_classes(IndexData& index) {
  trace_time tracer("find mocked classes", index.sample);

  for (auto& cinfo : index.allClassInfos) {
    if (is_mock_class(cinfo->cls) && cinfo->parent) {
      cinfo->parent->isMocked = true;
    }
  }

  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      for (auto const sub : cinfo->subclassList) {
        if (sub->isMocked) {
          cinfo->isSubMocked = true;
          break;
        }
      }
    }
  );
}

void mark_const_props(IndexData& index) {
  trace_time tracer("mark const props", index.sample);

  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      assertx(!cinfo->subHasConstProp);
      for (auto const sub : cinfo->subclassList) {
        if (sub->hasConstProp) {
          cinfo->subHasConstProp = true;
          break;
        }
      }
    }
  );
}

void mark_no_override_classes(IndexData& index) {
  trace_time tracer("mark no override classes", index.sample);

  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      // We cleared all the NoOverride flags while building the
      // index. Set them as necessary.
      assertx(!(cinfo->cls->attrs & AttrNoOverride));
      assertx(!(cinfo->cls->attrs & AttrNoOverrideRegular));
      assertx(!cinfo->subclassList.empty());

      if (cinfo->subclassList.size() == 1) {
        assertx(cinfo->subclassList[0] == cinfo.get());
        attribute_setter(cinfo->cls->attrs, true, AttrNoOverride);
        attribute_setter(cinfo->cls->attrs, true, AttrNoOverrideRegular);
      } else if (!cinfo->hasRegularSubclass) {
        attribute_setter(cinfo->cls->attrs, true, AttrNoOverrideRegular);
      }
    }
  );
}

void mark_no_override_methods(IndexData& index) {
  trace_time tracer("mark no override methods", index.sample);

  // We reset the override flags from all methods when adding the
  // units to the index. Set them to true now. We'll reset them below
  // if we detect an override. We need to do this in a separate
  // parallel pass to avoid race conditions with setting it to false
  // below.
  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      for (auto& m : cinfo->methods) {
        assertx(!(m.second.attrs & AttrNoOverride));
        assertx(!m.second.noOverrideRegular());
        if (is_special_method_name(m.first)) continue;
        attribute_setter(m.second.attrs, true, AttrNoOverride);
        attribute_setter(
          func_from_meth_ref(index, m.second.meth())->attrs,
          true,
          AttrNoOverride
        );
        m.second.setNoOverrideRegular();
      }
    }
  );

  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      for (auto& mte : cinfo->methods) {
        auto const name = mte.first;
        auto& meth = mte.second;

        if (!(meth.attrs & AttrNoOverride)) continue;
        assertx(meth.noOverrideRegular());

        auto& funcAttrs = func_from_meth_ref(index, meth.meth())->attrs;

        // Look up method in every subclass. If there's a different
        // method with the same name, there's an override so clear the
        // bits.
        for (auto const sub : cinfo->subclassList) {
          if (sub == cinfo.get()) continue;

          auto const clear = [&] (const MethTabEntry* subMeth) {
            // A private method on a non-regular class can be called
            // even from regular contexts.
            auto const reg =
              is_regular_class(*sub->cls) ||
              (subMeth && (subMeth->attrs & AttrPrivate) &&
               subMeth->topLevel());
            if (reg) meth.clearNoOverrideRegular();
            attribute_setter(meth.attrs, false, AttrNoOverride);
            attribute_setter(funcAttrs, false, AttrNoOverride);
            FTRACE(4, "Clearing AttrNoOverride{} on {}::{} because of {}\n",
                   reg ? " and noOverrideRegular" : "",
                   meth.meth().cls, name, sub->cls->name);
            return reg;
          };

          auto const it = sub->methods.find(name);
          if (it == end(sub->methods)) {
            // Method doesn't exist in subclass. This can happen with
            // special functions or interface methods, for example.
            assertx(is_special_method_name(name) ||
                    !is_regular_class(*cinfo->cls));
            if (clear(nullptr)) break;
            continue;
          }

          // Check if method in subclass is the same as in parent.
          if (it->second.meth() != meth.meth()) {
            if (clear(&it->second)) break;
            continue;
          }
        }
      }
    }
  );
}

const StaticString s__Reified("__Reified");

/*
 * Emitter adds a 86reifiedinit method to all classes that have reified
 * generics. All base classes also need to have this method so that when we
 * call parent::86reifeidinit(...), there is a stopping point.
 * Since while emitting we do not know whether a base class will have
 * reified parents, during JIT time we need to add 86reifiedinit
 * unless AttrNoReifiedInit attribute is set. At this phase,
 * we set AttrNoReifiedInit attribute on classes do not have any
 * reified classes that extend it.
 */
void clean_86reifiedinit_methods(IndexData& index) {
  trace_time tracer("clean 86reifiedinit methods", index.sample);
  hphp_fast_set<const php::Class*> needsinit;

  // Find all classes that still need their 86reifiedinit methods
  for (auto const& cinfo : index.allClassInfos) {
    auto const& ual = cinfo->cls->userAttributes;
    // Each class that has at least one reified generic has an attribute
    // __Reified added by the emitter
    auto has_reification = ual.find(s__Reified.get()) != ual.end();
    if (!has_reification) continue;
    // Add the base class for this reified class
    needsinit.emplace(cinfo->baseList[0]->cls);
  }

  // Add AttrNoReifiedInit to the base classes that do not need this method
  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->parent == nullptr && needsinit.count(cinfo->cls) == 0) {
      FTRACE(2, "Adding AttrNoReifiedInit on class {}\n", cinfo->cls->name);
      attribute_setter(cinfo->cls->attrs, true, AttrNoReifiedInit);
    }
  }
}

//////////////////////////////////////////////////////////////////////

void check_invariants(const IndexData& index, const ClassInfo* cinfo) {
  // AttrNoOverride is a superset of AttrNoOverrideRegular
  always_assert(
    IMPLIES(!(cinfo->cls->attrs & AttrNoOverrideRegular),
            !(cinfo->cls->attrs & AttrNoOverride))
  );

  // Override attrs and what we know about the subclasses should be in
  // agreement.
  if (cinfo->cls->attrs & AttrNoOverride) {
    always_assert(!cinfo->hasRegularSubclass);
    always_assert(!cinfo->hasNonRegularSubclass);
  } else if (cinfo->cls->attrs & AttrNoOverrideRegular) {
    always_assert(!cinfo->hasRegularSubclass);
  }

  for (size_t idx = 0; idx < cinfo->cls->methods.size(); ++idx) {
    // Each method in a class has an entry in its ClassInfo method
    // table.
    auto const& m = cinfo->cls->methods[idx];
    auto const it = cinfo->methods.find(m->name);
    always_assert(it != cinfo->methods.end());
    always_assert(it->second.meth().cls->isame(cinfo->cls->name));
    always_assert(it->second.meth().idx == idx);

    // Every method (except for constructors and special methods
    // should be in the global name-only tables.
    auto const nameIt = index.methodFamilies.find(m->name);
    if (m->name == s_construct.get() || is_special_method_name(m->name)) {
      always_assert(nameIt == end(index.methodFamilies));
      continue;
    }
    always_assert(nameIt != end(index.methodFamilies));

    auto const& entry = nameIt->second;
    // The global name-only tables are never complete.
    always_assert(entry.m_all.isIncomplete());
    always_assert(entry.m_regular.isEmpty() || entry.m_regular.isIncomplete());

    // "all" should always be non-empty and contain this method.
    always_assert(!entry.m_all.isEmpty());
    if (auto const ff = entry.m_all.funcFamily()) {
      always_assert(ff->possibleFuncs().size() > 1);
      // The FuncFamily shouldn't have a section for regular results
      // if "regular" isn't using it.
      if (entry.m_regular.func() || entry.m_regular.isEmpty()) {
        always_assert(!ff->m_regular);
      } else {
        // "all" and "regular" always share the same func family.
        always_assert(entry.m_regular.funcFamily() == ff);
      }
    } else {
      auto const func = entry.m_all.func();
      always_assert(func);
      always_assert(func == m.get());
      // "regular" is always a subset of "all", so it can either be a
      // single func (the same as "all"), or empty.
      always_assert(entry.m_regular.func() || entry.m_regular.isEmpty());
      if (auto const func2 = entry.m_regular.func()) {
        always_assert(func == func2);
      }
    }

    // If this is a regular class, "regular" should be non-empty and
    // contain this method.
    if (auto const ff = entry.m_regular.funcFamily()) {
      always_assert(ff->possibleFuncs().size() > 1);
    } else if (auto const func = entry.m_regular.func()) {
      if (is_regular_class(*cinfo->cls)) {
        always_assert(func == m.get());
      }
    } else {
      always_assert(!is_regular_class(*cinfo->cls));
    }
  }

  // Interface ClassInfo method table should only contain methods from
  // the interface itself.
  if (cinfo->cls->attrs & AttrInterface) {
    always_assert(cinfo->cls->methods.size() == cinfo->methods.size());
  }

  // If a class isn't overridden, it shouldn't have any func families
  // (because the method table is sufficient).
  if (cinfo->cls->attrs & AttrNoOverride) {
    always_assert(cinfo->methodFamilies.empty());
    always_assert(cinfo->methodFamiliesAux.empty());
  }

  // The auxiliary method families map is only used by non-regular
  // classes.
  if (is_regular_class(*cinfo->cls)) {
    always_assert(cinfo->methodFamiliesAux.empty());
  }

  for (auto const& [name, mte] : cinfo->methods) {
    // Interface method tables should only contain its own methods.
    if (cinfo->cls->attrs & AttrInterface) {
      always_assert(mte.meth().cls->isame(cinfo->cls->name));
    } else {
      // Non-interface method tables should not contain any methods
      // defined by an interface.
      auto const func = func_from_meth_ref(index, mte.meth());
      always_assert(!(func->cls->attrs & AttrInterface));
    }

    // AttrNoOverride implies noOverrideRegular
    always_assert(IMPLIES(mte.attrs & AttrNoOverride, mte.noOverrideRegular()));
    if (!is_special_method_name(name)) {
      // If the class isn't overridden, none of it's methods can be
      // either.
      always_assert(IMPLIES(cinfo->cls->attrs & AttrNoOverride,
                            mte.attrs & AttrNoOverride));
    }

    auto const famIt = cinfo->methodFamilies.find(name);
    // Don't store method families for special methods, or if there's
    // no override.
    if (is_special_method_name(name) || (mte.attrs & AttrNoOverride)) {
      always_assert(famIt == end(cinfo->methodFamilies));
      always_assert(!cinfo->methodFamiliesAux.count(name));
      continue;
    } else {
      always_assert(famIt != end(cinfo->methodFamilies));
    }
    auto const& entry = famIt->second;

    if (is_regular_class(*cinfo->cls)) {
      // "all" should only be a func family. It can't be empty,
      // because we know there's at least one method in it (the one in
      // cinfo->methods). It can't be a single func, because one of
      // the methods must be the cinfo->methods method, and we know it
      // isn't AttrNoOverride, so there *must* be another method. So,
      // it must be a func family.
      always_assert(entry.funcFamily());
      // This is a regular class, so we cannot have an incomplete
      // entry (can only happen with interfaces).
      always_assert(entry.isComplete());
    } else {
      // This class isn't AttrNoOverride, and since the method is on
      // this class, it should at least contain that.
      always_assert(!entry.isEmpty());
      // Only interfaces can have incomplete entries.
      always_assert(
        IMPLIES(entry.isIncomplete(), cinfo->cls->attrs & AttrInterface)
      );
      // If we got a single func, it should be the func on this
      // class. Since this isn't AttrNoOverride, it implies the entry
      // should be incomplete.
      always_assert(IMPLIES(entry.func(), entry.isIncomplete()));
      always_assert(
        IMPLIES(entry.func(),
                entry.func() == func_from_meth_ref(index, mte.meth()))
      );

      // The "aux" entry is optional. If it isn't present, it's the
      // same as the normal table.
      auto const auxIt = cinfo->methodFamiliesAux.find(name);
      if (auxIt != end(cinfo->methodFamiliesAux)) {
        auto const& aux = auxIt->second;

        // We shouldn't store in the aux table if the entry is the
        // same or if there's no override.
        always_assert(!mte.noOverrideRegular());
        always_assert(
          aux.isIncomplete() ||
          aux.func() != entry.func() ||
          aux.funcFamily() != entry.funcFamily()
        );

        // Normally the aux should be non-empty and complete. However
        // if this class is an interface, they could be.
        always_assert(
          IMPLIES(aux.isEmpty(), cinfo->cls->attrs & AttrInterface)
        );
        always_assert(
          IMPLIES(aux.isIncomplete(), cinfo->cls->attrs & AttrInterface)
        );

        // Since we know this was overridden (it wouldn't be in the
        // aux table otherwise), it must either be incomplete, or if
        // it has a single func, it cannot be the same func as this
        // class.
        always_assert(
          aux.isIncomplete() ||
          ((mte.attrs & AttrPrivate) && mte.topLevel()) ||
          aux.func() != func_from_meth_ref(index, mte.meth())
        );

        // Aux entry is a subset of the normal entry. If they both
        // have a func family or func, they must be the same. If the
        // normal entry has a func family, but aux doesn't, that func
        // family shouldn't have extra space allocated.
        always_assert(IMPLIES(entry.func(), !aux.funcFamily()));
        always_assert(IMPLIES(entry.funcFamily() && aux.funcFamily(),
                              entry.funcFamily() == aux.funcFamily()));
        always_assert(IMPLIES(entry.func() && aux.func(),
                              entry.func() == aux.func()));
        always_assert(IMPLIES(entry.funcFamily() && !aux.funcFamily(),
                              !entry.funcFamily()->m_regular));
      }
    }
  }

  // "Aux" entries should only exist for methods on this class, and
  // with a corresponding methodFamilies entry.
  for (auto const& [name, _] : cinfo->methodFamiliesAux) {
    always_assert(cinfo->methods.count(name));
    always_assert(cinfo->methodFamilies.count(name));
  }

  // We should only have func families for methods declared on this
  // class (except for interfaces and abstract classes).
  for (auto const& [name, entry] : cinfo->methodFamilies) {
    if (cinfo->methods.count(name)) continue;
    // Interfaces and abstract classes can have func families for
    // methods not defined on this class.
    always_assert(cinfo->cls->attrs & (AttrInterface|AttrAbstract));
    // We don't expand func families for these.
    always_assert(name != s_construct.get() && !is_special_method_name(name));

    // We only expand entries for interfaces and abstract classes if
    // it appears in every regular subclass. Therefore it cannot be
    // empty and is complete.
    always_assert(!entry.isEmpty());
    always_assert(entry.isComplete());
    if (auto const ff = entry.funcFamily()) {
      always_assert(!ff->m_regular);
    } else if (auto const func = entry.func()) {
      always_assert(func->cls != cinfo->cls);
    }
  }

  // The subclassList is non-empty, contains this ClassInfo, and
  // contains only unique elements.
  always_assert(!cinfo->subclassList.empty());
  always_assert(
    std::find(
      begin(cinfo->subclassList),
      end(cinfo->subclassList),
      cinfo
    ) != end(cinfo->subclassList)
  );
  auto cpy = cinfo->subclassList;
  std::sort(begin(cpy), end(cpy));
  cpy.erase(std::unique(begin(cpy), end(cpy)), end(cpy));
  always_assert(cpy.size() == cinfo->subclassList.size());

  // The baseList is non-empty, and the last element is this class.
  always_assert(!cinfo->baseList.empty());
  always_assert(cinfo->baseList.back() == cinfo);
}

void check_invariants(const IndexData& data, const FuncFamily& ff) {
  // FuncFamily should always have more than one func on it.
  always_assert(ff.possibleFuncs().size() > 1);

  SString name{nullptr};
  FuncFamily::PossibleFunc last{nullptr, false};
  for (auto const pf : ff.possibleFuncs()) {
    // Should only contain methods
    always_assert(pf.ptr()->cls);

    // Every method on the list should have the same name.
    if (!name) {
      name = pf.ptr()->name;
    } else {
      always_assert(name == pf.ptr()->name);
    }

    // Verify the list is sorted and doesn't contain any duplicates.
    hphp_fast_set<const php::Func*> seen;
    if (last.ptr()) {
      always_assert(
        [&] {
          if (last.inRegular() && !pf.inRegular()) return true;
          if (!last.inRegular() && pf.inRegular()) return false;
          return string_data_lti{}(last.ptr()->cls->name, pf.ptr()->cls->name);
        }()
      );
    }
    always_assert(seen.emplace(pf.ptr()).second);
    last = pf;
  }

  if (!ff.possibleFuncs().front().inRegular() ||
      ff.possibleFuncs().back().inRegular()) {
    // If there's no funcs on a regular class, or if all functions are
    // on a regular class, we don't need to keep separate information
    // for the regular subset (it either doesn't exist, or it's equal to
    // the entire list).
    always_assert(!ff.m_regular);
  }
}

void check_invariants(const IndexData& data) {
  if (!debug) return;

  trace_time timer{"check-invariants"};

  parallel::for_each(
    data.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      check_invariants(data, cinfo.get());
    }
  );

  std::vector<const FuncFamily*> funcFamilies;
  funcFamilies.reserve(data.funcFamilies.size());
  for (auto const& [ff, _] : data.funcFamilies) {
    funcFamilies.emplace_back(ff.get());
  }
  parallel::for_each(
    funcFamilies,
    [&] (const FuncFamily* ff) { check_invariants(data, *ff); }
  );
}

//////////////////////////////////////////////////////////////////////

Type adjust_closure_context(const Index& index, const CallContext& ctx) {
  if (ctx.callee->cls && ctx.callee->cls->closureContextCls) {
    auto withClosureContext = Context {
      index.lookup_func_unit(*ctx.callee),
      ctx.callee,
      index.lookup_closure_context(*ctx.callee->cls)
    };
    if (auto const rcls = index.selfCls(withClosureContext)) {
      return setctx(subObj(*rcls));
    }
    return TObj;
  }
  return ctx.context;
}

Type context_sensitive_return_type(IndexData& data,
                                   CallContext callCtx,
                                   Type returnType) {
  constexpr auto max_interp_nexting_level = 2;
  static __thread uint32_t interp_nesting_level;
  auto const finfo = func_info(data, callCtx.callee);

  auto const adjustedCtx = adjust_closure_context(*data.m_index, callCtx);
  returnType = return_with_context(std::move(returnType), adjustedCtx);

  auto const checkParam = [&] (int i) {
    auto const constraint = finfo->func->params[i].typeConstraint;
    if (constraint.hasConstraint() &&
        !constraint.isTypeVar() &&
        !constraint.isTypeConstant()) {
      auto ctx = Context {
        data.units.at(finfo->func->unit),
        finfo->func,
        finfo->func->cls
      };
      auto t = data.m_index->lookup_constraint(ctx, constraint);
      return callCtx.args[i].strictlyMoreRefined(t);
    }
    return callCtx.args[i].strictSubtypeOf(TInitCell);
  };

  // TODO(#3788877): more heuristics here would be useful.
  auto const tryContextSensitive = [&] {
    if (finfo->func->noContextSensitiveAnalysis ||
        finfo->func->params.empty() ||
        interp_nesting_level + 1 >= max_interp_nexting_level ||
        returnType == TBottom) {
      return false;
    }

    if (finfo->retParam != NoLocalId &&
        callCtx.args.size() > finfo->retParam &&
        checkParam(finfo->retParam)) {
      return true;
    }

    if (!options.ContextSensitiveInterp) return false;

    if (callCtx.args.size() < finfo->func->params.size()) return true;
    for (auto i = 0; i < finfo->func->params.size(); i++) {
      if (checkParam(i)) return true;
    }
    return false;
  }();

  if (!tryContextSensitive) return returnType;

  {
    ContextRetTyMap::const_accessor acc;
    if (data.contextualReturnTypes.find(acc, callCtx)) {
      if (data.frozen || acc->second == TBottom || is_scalar(acc->second)) {
        return acc->second;
      }
    }
  }

  if (data.frozen) return returnType;

  auto contextType = [&] {
    ++interp_nesting_level;
    SCOPE_EXIT { --interp_nesting_level; };

    auto const func = finfo->func;
    auto const wf = php::WideFunc::cns(func);
    auto const calleeCtx = AnalysisContext {
      data.units.at(func->unit),
      wf,
      func->cls
    };
    auto const ty = analyze_func_inline(
      *data.m_index,
      calleeCtx,
      adjustedCtx,
      callCtx.args
    ).inferredReturn;
    return return_with_context(ty, adjustedCtx);
  }();

  if (!interp_nesting_level) {
    FTRACE(3,
           "Context sensitive type: {}\n"
           "Context insensitive type: {}\n",
           show(contextType), show(returnType));
  }

  if (!returnType.subtypeOf(BUnc)) {
    // If the context insensitive return type could be non-static, staticness
    // could be a result of temporary context sensitive bytecode optimizations.
    contextType = loosen_staticness(std::move(contextType));
  }

  auto ret = intersection_of(std::move(returnType), std::move(contextType));

  if (!interp_nesting_level) {
    FTRACE(3, "Context sensitive result: {}\n", show(ret));
  }

  ContextRetTyMap::accessor acc;
  if (data.contextualReturnTypes.insert(acc, callCtx) ||
      ret.strictSubtypeOf(acc->second)) {
    acc->second = ret;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

template<typename F> auto
visit_parent_cinfo(const ClassInfo* cinfo, F fun) -> decltype(fun(cinfo)) {
  for (auto ci = cinfo; ci != nullptr; ci = ci->parent) {
    if (auto const ret = fun(ci)) return ret;
    if (ci->cls->attrs & AttrNoExpandTrait) continue;
    for (auto ct : ci->usedTraits) {
      if (auto const ret = visit_parent_cinfo(ct, fun)) {
        return ret;
      }
    }
  }
  return {};
}

Type lookup_public_prop_impl(
  const IndexData& data,
  const ClassInfo* cinfo,
  SString propName
) {
  // Find a property declared in this class (or a parent) with the same name.
  const php::Class* knownCls = nullptr;
  auto const prop = visit_parent_cinfo(
    cinfo,
    [&] (const ClassInfo* ci) -> const php::Prop* {
      for (auto const& prop : ci->cls->properties) {
        if (prop.name == propName) {
          knownCls = ci->cls;
          return &prop;
        }
      }
      return nullptr;
    }
  );

  if (!prop) return TCell;
  // Make sure its non-static and public. Otherwise its another function's
  // problem.
  if (prop->attrs & (AttrStatic | AttrPrivate)) return TCell;

  // Get a type corresponding to its declared type-hint (if any).
  auto ty = adjust_type_for_prop(
    *data.m_index, *knownCls, &prop->typeConstraint, TCell
  );
  // We might have to include the initial value which might be outside of the
  // type-hint.
  auto initialTy = loosen_all(from_cell(prop->val));
  if (!initialTy.subtypeOf(TUninit) && (prop->attrs & AttrSystemInitialValue)) {
    ty |= initialTy;
  }
  return ty;
}

// Test if the given property (declared in `cls') is accessible in the
// given context (null if we're not in a class).
bool static_is_accessible(const ClassInfo* clsCtx,
                          const ClassInfo* cls,
                          const php::Prop& prop) {
  assertx(prop.attrs & AttrStatic);
  switch (prop.attrs & (AttrPublic|AttrProtected|AttrPrivate)) {
    case AttrPublic:
      // Public is accessible everywhere
      return true;
    case AttrProtected:
      // Protected is accessible from both derived classes and parent
      // classes
      return clsCtx && (clsCtx->derivedFrom(*cls) || cls->derivedFrom(*clsCtx));
    case AttrPrivate:
      // Private is only accessible from within the declared class
      return clsCtx == cls;
  }
  always_assert(false);
}

// Return true if the given class can possibly throw when its
// initialized. Initialization can happen when an object of that class
// is instantiated, or (more importantly) when static properties are
// accessed.
bool class_init_might_raise(IndexData& data,
                            Context ctx,
                            const ClassInfo* cinfo) {
  // Check this class and all of its parents for possible inequivalent
  // redeclarations or bad initial values.
  do {
    // Be conservative for now if we have unflattened traits.
    if (!cinfo->traitProps.empty()) return true;
    if (cinfo->hasBadRedeclareProp) return true;
    if (cinfo->hasBadInitialPropValues) {
      add_dependency(data, cinfo->cls, ctx, Dep::PropBadInitialValues);
      return true;
    }
    cinfo = cinfo->parent;
  } while (cinfo);
  return false;
}

/*
 * Calculate the effects of applying the given type against the
 * type-constraints for the given prop. This includes the subtype
 * which will succeed (if any), and if the type-constraint check might
 * throw.
 */
PropMergeResult<> prop_tc_effects(const Index& index,
                                  const ClassInfo* ci,
                                  const php::Prop& prop,
                                  const Type& val,
                                  bool checkUB) {
  assertx(prop.typeConstraint.validForProp());

  using R = PropMergeResult<>;

  // If we're not actually checking property type-hints, everything
  // goes
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return R{ val, TriBool::No };

  auto const ctx = Context { nullptr, nullptr, ci->cls };

  auto const check = [&] (const TypeConstraint& tc, const Type& t) {
    // If the type as is satisfies the constraint, we won't throw and
    // the type is unchanged.
    if (index.satisfies_constraint(ctx, t, tc)) return R{ t, TriBool::No };
    // Otherwise adjust the type. If we get a Bottom we'll definitely
    // throw. We already know the type doesn't completely satisfy the
    // constraint, so we'll at least maybe throw.
    auto adjusted = adjust_type_for_prop(index, *ctx.cls, &tc, t);
    auto const throws = yesOrMaybe(adjusted.subtypeOf(BBottom));
    return R{ std::move(adjusted), throws };
  };

  // First check the main type-constraint.
  auto result = check(prop.typeConstraint, val);
  // If we're not checking generics upper-bounds, or if we already
  // know we'll fail, we're done.
  if (!checkUB ||
      RuntimeOption::EvalEnforceGenericsUB <= 0 ||
      result.throws == TriBool::Yes) {
    return result;
  }

  // Otherwise check every generic upper-bound. We'll feed the
  // narrowed type into each successive round. If we reach the point
  // where we'll know we'll definitely fail, just stop.
  for (auto ub : prop.ubs) {
    applyFlagsToUB(ub, prop.typeConstraint);
    auto r = check(ub, result.adjusted);
    result.throws &= r.throws;
    result.adjusted = std::move(r.adjusted);
    if (result.throws == TriBool::Yes) break;
  }

  return result;
}

/*
 * Lookup data for the static property named `propName', starting from
 * the specified class `start'. If `propName' is nullptr, then any
 * accessible static property in the class hierarchy is considered. If
 * `startOnly' is specified, if the property isn't found in `start',
 * it is treated as a lookup failure. Otherwise the lookup continues
 * in all parent classes of `start', until a property is found, or
 * until all parent classes have been exhausted (`startOnly' is used
 * to avoid redundant class hierarchy walks). `clsCtx' is the current
 * context, converted to a ClassInfo* (or nullptr if not in a class).
*/
PropLookupResult<> lookup_static_impl(IndexData& data,
                                      Context ctx,
                                      const ClassInfo* clsCtx,
                                      const PropertiesInfo& privateProps,
                                      const ClassInfo* start,
                                      SString propName,
                                      bool startOnly) {
  ITRACE(
    6, "lookup_static_impl: {} {} {}\n",
    clsCtx ? clsCtx->cls->name->toCppString() : std::string{"-"},
    start->cls->name,
    propName ? propName->toCppString() : std::string{"*"}
  );
  Trace::Indent _;

  auto const type = [&] (const php::Prop& prop,
                         const ClassInfo* ci) {
    switch (prop.attrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPublic:
      case AttrProtected: {
        if (ctx.unit) add_dependency(data, &prop, ctx, Dep::PublicSProp);
        auto const it = ci->publicStaticProps.find(propName);
        assertx(it != end(ci->publicStaticProps));
        return remove_uninit(it->second.inferredType);
      }
      case AttrPrivate: {
        assertx(clsCtx == ci);
        auto const elem = privateProps.readPrivateStatic(prop.name);
        if (!elem) return TInitCell;
        return remove_uninit(elem->ty);
      }
    }
    always_assert(false);
  };

  auto const initMightRaise = class_init_might_raise(data, ctx, start);

  auto const fromProp = [&] (const php::Prop& prop,
                             const ClassInfo* ci) {
    // The property was definitely found. Compute its attributes
    // from the prop metadata.
    return PropLookupResult<>{
      type(prop, ci),
      propName,
      TriBool::Yes,
      yesOrNo(prop.attrs & AttrIsConst),
      yesOrNo(prop.attrs & AttrIsReadonly),
      yesOrNo(prop.attrs & AttrLateInit),
      yesOrNo(prop.attrs & AttrInternal),
      initMightRaise
    };
  };

  auto const notFound = [&] {
    // The property definitely wasn't found.
    return PropLookupResult<>{
      TBottom,
      propName,
      TriBool::No,
      TriBool::No,
      TriBool::No,
      TriBool::No,
      TriBool::No,
      false
    };
  };

  if (!propName) {
    // We don't statically know the prop name. Walk up the hierarchy
    // and union the data for any accessible static property.
    ITRACE(4, "no prop name, considering all accessible\n");
    auto result = notFound();
    visit_parent_cinfo(
      start,
      [&] (const ClassInfo* ci) {
        for (auto const& prop : ci->cls->properties) {
          if (!(prop.attrs & AttrStatic) ||
              !static_is_accessible(clsCtx, ci, prop)) {
            ITRACE(
              6, "skipping inaccessible {}::${}\n",
              ci->cls->name, prop.name
            );
            continue;
          }
          auto const r = fromProp(prop, ci);
          ITRACE(6, "including {}:${} {}\n", ci->cls->name, prop.name, show(r));
          result |= r;
        }
        // If we're only interested in the starting class, don't walk
        // up to the parents.
        return startOnly;
      }
    );
    return result;
  }

  // We statically know the prop name. Walk up the hierarchy and stop
  // at the first matching property and use that data.
  assertx(!startOnly);
  auto const result = visit_parent_cinfo(
    start,
    [&] (const ClassInfo* ci) -> Optional<PropLookupResult<>> {
      for (auto const& prop : ci->cls->properties) {
        if (prop.name != propName) continue;
        // We have a matching prop. If its not static or not
        // accessible, the access will not succeed.
        if (!(prop.attrs & AttrStatic) ||
            !static_is_accessible(clsCtx, ci, prop)) {
          ITRACE(
            6, "{}::${} found but inaccessible, stopping\n",
            ci->cls->name, propName
          );
          return notFound();
        }
        // Otherwise its a match
        auto const r = fromProp(prop, ci);
        ITRACE(6, "found {}:${} {}\n", ci->cls->name, propName, show(r));
        return r;
      }
      return std::nullopt;
    }
  );
  if (!result) {
    // We walked up to all of the base classes and didn't find a
    // property with a matching name. The access will fail.
    ITRACE(6, "nothing found\n");
    return notFound();
  }
  return *result;
}

/*
 * Lookup the static property named `propName', starting from the
 * specified class `start'. If an accessible property is found, then
 * merge the given type `val' into the already known type for that
 * property. If `propName' is nullptr, then any accessible static
 * property in the class hierarchy is considered. If `startOnly' is
 * specified, if the property isn't found in `start', then the nothing
 * is done. Otherwise the lookup continues in all parent classes of
 * `start', until a property is found, or until all parent classes
 * have been exhausted (`startOnly' is to avoid redundant class
 * hierarchy walks). `clsCtx' is the current context, converted to a
 * ClassInfo* (or nullptr if not in a class). If `ignoreConst' is
 * false, then AttrConst properties will not have their type
 * modified. `mergePublic' is a lambda with the logic to merge a type
 * for a public property (this is needed to avoid cyclic
 * dependencies).
 */
template <typename F>
PropMergeResult<> merge_static_type_impl(IndexData& data,
                                         Context ctx,
                                         F mergePublic,
                                         PropertiesInfo& privateProps,
                                         const ClassInfo* clsCtx,
                                         const ClassInfo* start,
                                         SString propName,
                                         const Type& val,
                                         bool checkUB,
                                         bool ignoreConst,
                                         bool mustBeReadOnly,
                                         bool startOnly) {
  ITRACE(
    6, "merge_static_type_impl: {} {} {} {}\n",
    clsCtx ? clsCtx->cls->name->toCppString() : std::string{"-"},
    start->cls->name,
    propName ? propName->toCppString() : std::string{"*"},
    show(val)
  );
  Trace::Indent _;

  assertx(!val.subtypeOf(BBottom));

  // Perform the actual merge for a given property, returning the
  // effects of that merge.
  auto const merge = [&] (const php::Prop& prop, const ClassInfo* ci) {
    // First calculate the effects of the type-constraint.
    auto const effects = prop_tc_effects(*data.m_index, ci, prop, val, checkUB);
    // No point in merging if the type-constraint will always fail.
    if (effects.throws == TriBool::Yes) {
      ITRACE(
        6, "tc would throw on {}::${} with {}, skipping\n",
        ci->cls->name, prop.name, show(val)
      );
      return effects;
    }
    assertx(!effects.adjusted.subtypeOf(BBottom));

    ITRACE(
      6, "merging {} into {}::${}\n",
      show(effects.adjusted), ci->cls->name, prop.name
    );

    switch (prop.attrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPublic:
      case AttrProtected:
        mergePublic(ci, prop, unctx(effects.adjusted));
         // If the property is internal, accessing it may throw
         // TODO(T131951529): we can do better by checking modules here
        if ((prop.attrs & AttrInternal) && effects.throws == TriBool::No) {
          return PropMergeResult<>{
            effects.adjusted,
            TriBool::Maybe
          };
        }
        return effects;
      case AttrPrivate: {
        assertx(clsCtx == ci);
        privateProps.mergeInPrivateStaticPreAdjusted(
          prop.name,
          unctx(effects.adjusted)
        );
        return effects;
      }
    }
    always_assert(false);
  };

  // If we don't find a property, then the mutation will definitely
  // fail.
  auto const notFound = [&] {
    return PropMergeResult<>{
      TBottom,
      TriBool::Yes
    };
  };

  if (!propName) {
    // We don't statically know the prop name. Walk up the hierarchy
    // and merge the type for any accessible static property.
    ITRACE(6, "no prop name, considering all accessible\n");
    auto result = notFound();
    visit_parent_cinfo(
      start,
      [&] (const ClassInfo* ci) {
        for (auto const& prop : ci->cls->properties) {
          if (!(prop.attrs & AttrStatic) ||
              !static_is_accessible(clsCtx, ci, prop)) {
            ITRACE(
              6, "skipping inaccessible {}::${}\n",
              ci->cls->name, prop.name
            );
            continue;
          }
          if (!ignoreConst && (prop.attrs & AttrIsConst)) {
            ITRACE(6, "skipping const {}::${}\n", ci->cls->name, prop.name);
            continue;
          }
          if (mustBeReadOnly && !(prop.attrs & AttrIsReadonly)) {
            ITRACE(6, "skipping mutable property that must be readonly {}::${}\n",
              ci->cls->name, prop.name);
            continue;
          }
          result |= merge(prop, ci);
        }
        return startOnly;
      }
    );
    return result;
  }

  // We statically know the prop name. Walk up the hierarchy and stop
  // at the first matching property and merge the type there.
  assertx(!startOnly);
  auto result = visit_parent_cinfo(
    start,
    [&] (const ClassInfo* ci) -> Optional<PropMergeResult<>> {
      for (auto const& prop : ci->cls->properties) {
        if (prop.name != propName) continue;
        // We found a property with the right name, but its
        // inaccessible from this context (or not even static). This
        // mutation will fail, so we don't need to modify the type.
        if (!(prop.attrs & AttrStatic) ||
            !static_is_accessible(clsCtx, ci, prop)) {
          ITRACE(
            6, "{}::${} found but inaccessible, stopping\n",
            ci->cls->name, propName
          );
          return notFound();
        }
        // Mutations to AttrConst properties will fail as well, unless
        // it we want to override that behavior.
        if (!ignoreConst && (prop.attrs & AttrIsConst)) {
          ITRACE(
            6, "{}:${} found but const, stopping\n",
            ci->cls->name, propName
          );
          return notFound();
        }
        if (mustBeReadOnly && !(prop.attrs & AttrIsReadonly)) {
          ITRACE(
            6, "{}:${} found but is mutable and must be readonly, stopping\n",
            ci->cls->name, propName
          );
          return notFound();
        }
        return merge(prop, ci);
      }
      return std::nullopt;
    }
  );
  if (!result) {
    ITRACE(6, "nothing found\n");
    return notFound();
  }

  // If the mutation won't throw, we still need to check if the class
  // initialization can throw. If we might already throw (or
  // definitely will throw), this doesn't matter.
  if (result->throws == TriBool::No) {
    return PropMergeResult<>{
      std::move(result->adjusted),
      maybeOrNo(class_init_might_raise(data, ctx, start))
    };
  }
  return *result;
}

//////////////////////////////////////////////////////////////////////

// Class flattening:

const StaticString s___Sealed("__Sealed");
const StaticString s___EnableMethodTraitDiamond("__EnableMethodTraitDiamond");

/*
 * Extern-worker job to build ClassInfo2s (which involves flattening
 * data across the hierarchy) and flattening traits.
 */
struct FlattenJob {
  static std::string name() { return "hhbbc-flatten"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
  }
  static void fini() {}

  /*
   * Metadata representing results of flattening. This is information
   * that the local coordinator (as opposed to later remote jobs) will
   * need to consume.
   */
  struct OutputMeta {
    // Classes which have been determined to be uninstantiable
    // (therefore have no result output data).
    ISStringSet uninstantiable;
    // New closures produced from trait flattening, grouped by the
    // unit they belong to. Such new closures will require "fixups" in
    // the php::Program data.
    struct NewClosures {
      SString unit;
      std::vector<SString> names;
      template <typename SerDe> void serde(SerDe& sd) {
        sd(unit)(names);
      }
    };
    std::vector<NewClosures> newClosures;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(uninstantiable, string_data_lti{})(newClosures);
    }
  };

  /*
   * Job returns a list of (potentially modified) php::Class, a list
   * of new ClassInfo2, and metadata for the entire job. The order of
   * the lists reflects the order of the input classes (skipping over
   * classes marked as uninstantiable in the metadata).
   */
  using Output = Multi<
    Variadic<std::unique_ptr<php::Class>>,
    Variadic<std::unique_ptr<ClassInfo2>>,
    OutputMeta
  >;

  /*
   * Job takes a list of classes which are to be flattened. In
   * addition to this, it also takes a list of classes which are
   * dependencies of the classes to be flattened. (A class might be
   * one of the inputs *and* a dependency, in which case it should
   * just be on the input list). It is expected that *all*
   * dependencies are provided.
   */
  static Output run(Variadic<std::unique_ptr<php::Class>> classes,
                    Variadic<std::unique_ptr<php::Class>> deps) {
    LocalIndex index;

    // Some classes might be dependencies of another. Moreover, some
    // classes might share dependencies. Topologically sort all of the
    // classes and process them in that order. Information will flow
    // from parent classes to their children.
    auto const worklist = prepare(
      index,
      [&] {
        ISStringToOneT<php::Class*> out;
        out.reserve(classes.vals.size() + deps.vals.size());
        for (auto const& c : classes.vals) {
          always_assert(out.emplace(c->name, c.get()).second);
        }
        for (auto const& c : deps.vals) {
          always_assert(out.emplace(c->name, c.get()).second);
        }
        return out;
      }()
    );

    std::vector<std::unique_ptr<php::Class>> newClosures;
    newClosures.reserve(worklist.size());

    for (auto const cls : worklist) {
      Trace::Bump bumper{
        Trace::hhbbc_index, kSystemLibBump, is_systemlib_part(cls->unit)
      };

      ITRACE(2, "flatten class: {}\n", cls->name);
      Trace::Indent indent;

      index.m_ctx = cls;
      SCOPE_EXIT { index.m_ctx = nullptr; };

      auto state = std::make_unique<State>();
      // Attempt to make the ClassInfo2 for this class. If we can't,
      // it means the class is not instantiable.
      auto cinfo = make_info(index, *cls, *state);
      if (!cinfo) {
        ITRACE(4, "{} is not instantiable\n", cls->name);
        always_assert(index.m_uninstantiable.emplace(cls->name).second);
        continue;
      }

      ITRACE(5, "adding state for class '{}' to local index\n", cls->name);
      assertx(cinfo->name->isame(cls->name));

      // We might look up this class when flattening itself, so add it
      // to the local index before we start.
      always_assert(index.m_classes.emplace(cls->name, cls).second);

      auto const [cinfoIt, cinfoSuccess] =
        index.m_classInfos.emplace(cls->name, std::move(cinfo));
      always_assert(cinfoSuccess);

      auto const [stateIt, stateSuccess] =
        index.m_states.emplace(cls->name, std::move(state));
      always_assert(stateSuccess);

      auto closures =
        flatten_traits(index, *cls, *cinfoIt->second, *stateIt->second);

      assertx(IMPLIES(is_closure(*cls), cinfoIt->second->closures.empty()));
      std::sort(
        begin(cinfoIt->second->closures),
        end(cinfoIt->second->closures),
        string_data_lti{}
      );

      // Trait flattening may produce new closures, so those need to
      // be added to the local index as well.
      for (auto& [c, i] : closures) {
        ITRACE(5, "adding state for closure '{}' to local index\n", c->name);
        assertx(c->name->isame(i->name));
        assertx(is_closure(*c));
        assertx(i->closures.empty());
        always_assert(index.m_classes.emplace(c->name, c.get()).second);
        always_assert(index.m_classInfos.emplace(c->name, std::move(i)).second);
        newClosures.emplace_back(std::move(c));
      }
    }

    // Format the output data and put it in a deterministic order.
    Variadic<std::unique_ptr<php::Class>> outClasses;
    Variadic<std::unique_ptr<ClassInfo2>> outInfos;
    OutputMeta outMeta;
    ISStringSet outNames;

    outClasses.vals.reserve(classes.vals.size() + newClosures.size());
    outInfos.vals.reserve(classes.vals.size() + newClosures.size());
    outMeta.newClosures.reserve(classes.vals.size());
    outNames.reserve(classes.vals.size());

    for (auto& cls : classes.vals) {
      auto const cinfoIt = index.m_classInfos.find(cls->name);
      if (cinfoIt == index.m_classInfos.end()) {
        always_assert(index.uninstantiable(cls->name));
        outMeta.uninstantiable.emplace(cls->name);
        continue;
      }
      outNames.emplace(cls->name);
      outClasses.vals.emplace_back(std::move(cls));
      outInfos.vals.emplace_back(std::move(cinfoIt->second));
    }

    std::sort(
      begin(newClosures),
      end(newClosures),
      [] (const std::unique_ptr<php::Class>& a,
          const std::unique_ptr<php::Class>& b) {
        if (a->unit != b->unit) return string_data_lt{}(a->unit, b->unit);
        return string_data_lti{}(a->name, b->name);
      }
    );

    for (auto& clo : newClosures) {
      assertx(is_closure(*clo));
      assertx(clo->closureContextCls);
      assertx(clo->unit);

      if (!outNames.count(clo->closureContextCls)) continue;

      auto& outNewClosures = outMeta.newClosures;
      if (outNewClosures.empty() || outNewClosures.back().unit != clo->unit) {
        outNewClosures.emplace_back();
        outNewClosures.back().unit = clo->unit;
      }
      outNewClosures.back().names.emplace_back(clo->name);

      auto& cinfo = index.m_classInfos.at(clo->name);
      outClasses.vals.emplace_back(std::move(clo));
      outInfos.vals.emplace_back(std::move(cinfo));
    }

    return std::make_tuple(
      std::move(outClasses),
      std::move(outInfos),
      std::move(outMeta)
    );
  }

private:
  /*
   * State which needs to be propagated from a dependency to a child
   * class during flattening, but not required after flattening (so
   * doesn't belong in ClassInfo2).
   */
  struct State {
    struct PropTuple {
      SString name;
      SString src;
      php::Prop prop;
    };
    // Maintain order of properties as we inherit them.
    CompactVector<PropTuple> m_props;
    SStringToOneT<size_t> m_propIndices;
    CompactVector<php::Const> m_traitCns;
    SStringSet m_cnsFromTrait;
    SStringToOneT<size_t> m_methodIndices;

    size_t& methodIdx(SString context, SString cls, SString name) {
      auto const it = m_methodIndices.find(name);
      always_assert_flog(
        it != m_methodIndices.end(),
        "While processing '{}', "
        "tried to access missing method index for '{}::{}'",
        context, cls, name
      );
      return it->second;
    }

    size_t methodIdx(SString context, SString cls, SString name) const {
      return const_cast<State*>(this)->methodIdx(context, cls, name);
    }
  };

  /*
   * LocalIndex is similar to Index, but for this job. It maps names
   * to class information needed during flattening. It also verifies
   * we don't try to access information about a class until it's
   * actually available (which shouldn't happen if our dataflow is
   * correct).
   */
  struct LocalIndex {
    const php::Class* m_ctx{nullptr};

    ISStringToOneT<const php::Class*> m_classes;
    ISStringToOneT<std::unique_ptr<ClassInfo2>> m_classInfos;
    ISStringToOneT<std::unique_ptr<State>> m_states;

    hphp_fast_map<
      const php::Class*,
      hphp_fast_set<const php::Class*>
    > m_classClosures;

    ISStringSet m_uninstantiable;

    const php::Class& cls(SString name) const {
      if (m_ctx->name->isame(name)) return *m_ctx;
      auto const it = m_classes.find(name);
      always_assert_flog(
        it != m_classes.end(),
        "While processing '{}', tried to access missing class '{}' from index",
        m_ctx->name,
        name
      );
      return *it->second;
    }

    const ClassInfo2& classInfo(SString name) const {
      auto const it = m_classInfos.find(name);
      always_assert_flog(
        it != m_classInfos.end(),
        "While processing '{}', tried to access missing class-info for '{}' "
        "from index",
        m_ctx->name,
        name
      );
      return *it->second;
    }

    const State& state(SString name) const {
      auto const it = m_states.find(name);
      always_assert_flog(
        it != m_states.end(),
        "While processing '{}', tried to access missing flatten state for '{}' "
        "from index",
        m_ctx->name,
        name
      );
      return *it->second;
    }

    const hphp_fast_set<const php::Class*>* classClosures(
      const php::Class& cls
    ) const {
      auto const it = m_classClosures.find(&cls);
      if (it == m_classClosures.end()) return nullptr;
      return &it->second;
    }

    bool uninstantiable(SString name) const {
      return m_uninstantiable.count(name);
    }

    const php::Func& meth(const MethRef& r) const {
      auto const& mcls = cls(r.cls);
      assertx(r.idx < mcls.methods.size());
      return *mcls.methods[r.idx];
    }
    const php::Func& meth(const MethTabEntry& mte) const {
      return meth(mte.meth());
    }

    const php::Const& cns(const ClassInfo2::ConstIndex& idx) const {
      auto const& c = cls(idx.cls);
      assertx(idx.idx < c.constants.size());
      return c.constants[idx.idx];
    }

    size_t methodIdx(SString cls, SString name) const {
      return state(cls).methodIdx(m_ctx->name, cls, name);
    }
  };

  /*
   * Calculate the order in which the classes should be flattened,
   * taking into account dependencies.
   */
  static std::vector<php::Class*> prepare(
    LocalIndex& index,
    const ISStringToOneT<php::Class*>& classes
  ) {
    auto const get = [&] (SString name) -> php::Class& {
      auto const it = classes.find(name);
      always_assert_flog(
        it != classes.end(),
        "Tried to access missing class '{}' while calculating flattening order",
        name
      );
      return *it->second;
    };

    for (auto const [_, cls] : classes) {
      if (!cls->closureContextCls) continue;
      assertx(is_closure(*cls));
      auto const it = classes.find(cls->closureContextCls);
      // Closure's context may not necessarily be present.
      if (it == classes.end()) continue;
      auto& ctx = *it->second;
      index.m_classClosures[&ctx].emplace(cls);
    }

    auto const forEachDep = [&] (php::Class& c, auto const& f) {
      if (c.parentName) f(get(c.parentName));
      for (auto const i : c.interfaceNames)    f(get(i));
      for (auto const e : c.includedEnumNames) f(get(e));
      for (auto const t : c.usedTraitNames)    f(get(t));
      if (auto const closures = index.classClosures(c)) {
        for (auto const clo : *closures) {
          f(const_cast<php::Class&>(*clo));
        }
      }
    };

    /*
     * Perform a standard topological sort:
     *
     * - For each class, calculate the number of classes which depend on it.
     *
     * - Any class which has a use count of zero is not depended on by
     *   anyone and goes onto the intitial worklist.
     *
     * - For every class on the worklist, push it onto the output
     *   list, and decrement the use count of all of it's
     *   dependencies.
     *
     * - For any class which now has a use count of zero, push it onto
     *   the worklist and repeat above step until all classes are
     *   pushed onto the output list.
     *
     * - Reverse the list.
     *
     * - This does not handle cycles, but we should not encounter any
     *   here, as such cycles should be detected earlier and not be
     *   scheduled in a job.
     */
    hphp_fast_map<const php::Class*, size_t> uses;
    uses.reserve(classes.size());
    for (auto const& [_, cls] : classes) {
      forEachDep(*cls, [&] (const php::Class& d) { ++uses[&d]; });
    }

    std::vector<php::Class*> worklist;
    for (auto const [_, cls] : classes) {
      if (!uses[cls]) worklist.emplace_back(cls);
    }
    always_assert(!worklist.empty());
    std::sort(
      worklist.begin(),
      worklist.end(),
      [] (const php::Class* c1, const php::Class* c2) {
        return string_data_lti{}(c1->name, c2->name);
      }
    );

    std::vector<php::Class*> ordered;
    ordered.reserve(classes.size());
    do {
      auto const cls = worklist.back();
      assertx(!uses[cls]);
      worklist.pop_back();
      forEachDep(
        *cls,
        [&] (php::Class& d) {
          if (!--uses.at(&d)) worklist.emplace_back(&d);
        }
      );
      ordered.emplace_back(cls);
    } while (!worklist.empty());

    for (auto const& [_, cls] : classes) always_assert(!uses.at(cls));
    std::reverse(ordered.begin(), ordered.end());
    return ordered;
  }

  static std::unique_ptr<ClassInfo2> make_info(const LocalIndex& index,
                                               php::Class& cls,
                                               State& state) {
    if (debug && is_closure(cls)) {
      always_assert(cls.parentName->isame(s_Closure.get()));
      always_assert(cls.interfaceNames.empty());
      always_assert(cls.includedEnumNames.empty());
      always_assert(cls.usedTraitNames.empty());
      always_assert(cls.requirements.empty());
      always_assert(cls.constants.empty());
      always_assert(cls.userAttributes.empty());
      always_assert(!(cls.attrs & (AttrTrait | AttrInterface)));
    }

    auto cinfo = std::make_unique<ClassInfo2>();
    cinfo->name = cls.name;
    cinfo->hasConstProp = cls.hasConstProp;
    cinfo->hasReifiedParent = cls.hasReifiedGenerics;

    if (cls.parentName) {
      if (index.uninstantiable(cls.parentName)) {
        ITRACE(2,
               "Making class-info failed for `{}' because "
               "its parent `{}' is uninstantiable\n",
               cls.name, cls.parentName);
        return nullptr;
      }
      auto const& parent = index.cls(cls.parentName);
      auto const& parentInfo = index.classInfo(cls.parentName);

      if (parent.attrs & (AttrInterface | AttrTrait)) {
        ITRACE(2,
               "Making class-info failed for `{}' because "
               "its parent `{}' is not a class\n",
               cls.name, cls.parentName);
        return nullptr;
      }
      if (!enforce_sealing(*cinfo, parent)) return nullptr;

      cinfo->parent = cls.parentName;
      cinfo->baseList = parentInfo.baseList;
      cinfo->implInterfaces = parentInfo.implInterfaces;
      cinfo->hasConstProp |= parentInfo.hasConstProp;
      cinfo->hasReifiedParent |= parentInfo.hasReifiedParent;
    }
    cinfo->baseList.emplace_back(cls.name);

    for (auto const iname : cls.interfaceNames) {
      if (index.uninstantiable(iname)) {
        ITRACE(2,
               "Making class-info failed for `{}' because "
               "{} is uninstantiable\n",
               cls.name, iname);
        return nullptr;
      }
      auto const& iface = index.cls(iname);
      auto const& ifaceInfo = index.classInfo(iname);

      if (!(iface.attrs & AttrInterface)) {
        ITRACE(2,
               "Making class-info failed for `{}' because `{}' "
               "is not an interface\n",
               cls.name, iname);
        return nullptr;
      }
      if (!enforce_sealing(*cinfo, iface)) return nullptr;

      cinfo->declInterfaces.emplace_back(iname);
      cinfo->implInterfaces.insert(
        ifaceInfo.implInterfaces.begin(),
        ifaceInfo.implInterfaces.end()
      );
      cinfo->hasReifiedParent |= ifaceInfo.hasReifiedParent;
    }

    for (auto const ename : cls.includedEnumNames) {
      if (index.uninstantiable(ename)) {
        ITRACE(2,
               "Making class-info failed for `{}' because "
               "{} is uninstantiable\n",
               cls.name, ename);
        return nullptr;
      }
      auto const& e = index.cls(ename);
      auto const& einfo = index.classInfo(ename);

      auto const wantAttr = cls.attrs & (AttrEnum | AttrEnumClass);
      if (!(e.attrs & wantAttr)) {
        ITRACE(2,
               "Making class-info failed for `{}' because `{}' "
               "is not an enum{}\n",
               cls.name, ename,
               wantAttr & AttrEnumClass ? " class" : "");
        return nullptr;
      }
      if (!enforce_sealing(*cinfo, e)) return nullptr;

      cinfo->includedEnums.emplace_back(ename);
      cinfo->implInterfaces.insert(
        einfo.implInterfaces.begin(),
        einfo.implInterfaces.end()
      );
    }

    for (auto const tname : cls.usedTraitNames) {
      if (index.uninstantiable(tname)) {
        ITRACE(2,
               "Making class-info failed for `{}' because "
               "{} is uninstantiable\n",
               cls.name, tname);
        return nullptr;
      }
      auto const& trait = index.cls(tname);
      auto const& traitInfo = index.classInfo(tname);

      if (!(trait.attrs & AttrTrait)) {
        ITRACE(2,
               "Making class-info failed for `{}' because `{}' "
               "is not a trait\n",
               cls.name, tname);
        return nullptr;
      }
      if (!enforce_sealing(*cinfo, trait)) return nullptr;

      cinfo->usedTraits.emplace_back(tname);
      cinfo->implInterfaces.insert(
        traitInfo.implInterfaces.begin(),
        traitInfo.implInterfaces.end()
      );
      cinfo->hasConstProp |= traitInfo.hasConstProp;
      cinfo->hasReifiedParent |= traitInfo.hasReifiedParent;
    }

    if (cls.attrs & AttrInterface) cinfo->implInterfaces.emplace(cls.name);

    if (!build_methods(index, cls, *cinfo, state))    return nullptr;
    if (!build_properties(index, cls, *cinfo, state)) return nullptr;
    if (!build_constants(index, cls, *cinfo, state))  return nullptr;

    if (auto const closures = index.classClosures(cls)) {
      for (auto const clo : *closures) {
        assertx(is_closure(*clo));
        assertx(
          clo->closureContextCls && clo->closureContextCls->isame(cls.name)
        );
        // Ensure closures have been processed already
        if (debug) index.cls(clo->name);
        cinfo->closures.emplace_back(clo->name);
      }
    }
    assertx(IMPLIES(is_closure(cls), cinfo->closures.empty()));

    ITRACE(2, "new class-info: {}\n", cls.name);
    if (Trace::moduleEnabled(Trace::hhbbc_index, 3)) {
      if (cinfo->parent) {
        ITRACE(3, "           parent: {}\n", cinfo->parent);
      }
      for (auto const DEBUG_ONLY base : cinfo->baseList) {
        ITRACE(3, "             base: {}\n", base);
      }
      for (auto const DEBUG_ONLY iface : cinfo->declInterfaces) {
        ITRACE(3, "  decl implements: {}\n", iface);
      }
      for (auto const DEBUG_ONLY iface : cinfo->implInterfaces) {
        ITRACE(3, "       implements: {}\n", iface);
      }
      for (auto const DEBUG_ONLY e : cinfo->includedEnums) {
        ITRACE(3, "             enum: {}\n", e);
      }
      for (auto const DEBUG_ONLY trait : cinfo->usedTraits) {
        ITRACE(3, "             uses: {}\n", trait);
      }
      for (auto const DEBUG_ONLY closure : cinfo->closures) {
        ITRACE(3, "          closure: {}\n", closure);
      }
    }

    return cinfo;
  }

  static bool enforce_sealing(const ClassInfo2& cinfo,
                              const php::Class& parent) {
    if (!(parent.attrs & AttrSealed)) return true;
    auto const it = parent.userAttributes.find(s___Sealed.get());
    assertx(it != parent.userAttributes.end());
    assertx(tvIsArrayLike(it->second));
    auto allowed = false;
    IterateV(
      it->second.m_data.parr,
      [&] (TypedValue v) {
        assertx(tvIsStringLike(v));
        if (tvAssertStringLike(v)->isame(cinfo.name)) {
          allowed = true;
          return true;
        }
        return false;
      }
    );
    if (!allowed) {
      ITRACE(
        2,
        "Making class-info failed for `{}' because "
        "`{}' is sealed\n",
        cinfo.name, parent.name
      );
    }
    return allowed;
  }

  static bool build_properties(const LocalIndex& index,
                               const php::Class& cls,
                               ClassInfo2& cinfo,
                               State& state) {
    if (cls.parentName) {
      auto const& parentState = index.state(cls.parentName);
      state.m_props = parentState.m_props;
      state.m_propIndices = parentState.m_propIndices;
    }

    for (auto const iface : cinfo.declInterfaces) {
      if (!merge_properties(cinfo, state, index.state(iface))) {
        return false;
      }
    }
    for (auto const trait : cinfo.usedTraits) {
      if (!merge_properties(cinfo, state, index.state(trait))) {
        return false;
      }
    }
    for (auto const e : cinfo.includedEnums) {
      if (!merge_properties(cinfo, state, index.state(e))) {
        return false;
      }
    }

    if (cls.attrs & AttrInterface) return true;

    for (auto const& p : cls.properties) {
      if (!add_property(cinfo, state, p.name, p, cinfo.name, false)) {
        return false;
      }
    }

    // There's no need to do this work if traits have been flattened
    // already, or if the top level class has no traits.  In those
    // cases, we might be able to rule out some instantiations, but it
    // doesn't seem worth it.
    if (cls.attrs & AttrNoExpandTrait) return true;

    for (auto const traitName : cinfo.usedTraits) {
      auto const& trait = index.cls(traitName);
      auto const& traitInfo = index.classInfo(traitName);
      for (auto const& p : trait.properties) {
        if (!add_property(cinfo, state, p.name, p, cinfo.name, true)) {
          return false;
        }
      }
      for (auto const& p : traitInfo.traitProps) {
        if (!add_property(cinfo, state, p.name, p, cinfo.name, true)) {
          return false;
        }
      }
    }

    return true;
  }

  static bool add_property(ClassInfo2& cinfo,
                           State& state,
                           SString name,
                           const php::Prop& prop,
                           SString src,
                           bool trait) {
    auto const [it, emplaced] =
      state.m_propIndices.emplace(name, state.m_props.size());
    if (emplaced) {
      state.m_props.emplace_back(State::PropTuple{name, src, prop});
      if (trait) cinfo.traitProps.emplace_back(prop);
      return true;
    }
    assertx(it->second < state.m_props.size());
    auto& prevTuple = state.m_props[it->second];
    auto const& prev = prevTuple.prop;
    auto const prevSrc = prevTuple.src;

    if (cinfo.name->isame(prevSrc)) {
      if ((prev.attrs ^ prop.attrs) &
          (AttrStatic | AttrPublic | AttrProtected | AttrPrivate) ||
          (!(prop.attrs & AttrSystemInitialValue) &&
           !(prev.attrs & AttrSystemInitialValue) &&
           !Class::compatibleTraitPropInit(prev.val, prop.val))) {
        ITRACE(2,
               "Adding property failed for `{}' because "
               "two declarations of `{}' at the same level had "
               "different attributes\n",
               cinfo.name, prop.name);
        return false;
      }
      return true;
    }

    if (!(prev.attrs & AttrPrivate)) {
      if ((prev.attrs ^ prop.attrs) & AttrStatic) {
        ITRACE(2,
               "Adding property failed for `{}' because "
               "`{}' was defined both static and non-static\n",
               cinfo.name, prop.name);
        return false;
      }
      if (prop.attrs & AttrPrivate) {
        ITRACE(2,
               "Adding property failed for `{}' because "
               "`{}' was re-declared private\n",
               cinfo.name, prop.name);
        return false;
      }
      if (prop.attrs & AttrProtected && !(prev.attrs & AttrProtected)) {
        ITRACE(2,
               "Adding property failed for `{}' because "
               "`{}' was redeclared protected from public\n",
               cinfo.name, prop.name);
        return false;
      }
    }

    if (trait) cinfo.traitProps.emplace_back(prop);
    prevTuple = State::PropTuple{name, src, prop};
    return true;
  }

  static bool merge_properties(ClassInfo2& cinfo,
                               State& dst,
                               const State& src) {
    for (auto const& [name, src, prop] : src.m_props) {
      if (!add_property(cinfo, dst, name, prop, src, false)) {
        return false;
      }
    }
    return true;
  }

  static bool build_constants(const LocalIndex& index,
                              php::Class& cls,
                              ClassInfo2& cinfo,
                              State& state) {
    if (cls.parentName) {
      cinfo.clsConstants = index.classInfo(cls.parentName).clsConstants;
      state.m_cnsFromTrait = index.state(cls.parentName).m_cnsFromTrait;
    }

    for (auto const iname : cinfo.declInterfaces) {
      auto const& iface = index.classInfo(iname);
      auto const& ifaceState = index.state(iname);
      for (auto const& [cnsName, cnsIdx] : iface.clsConstants) {
        auto const added = add_constant(
          index, cinfo, state, cnsName,
          cnsIdx, ifaceState.m_cnsFromTrait.count(cnsName)
        );
        if (!added) return false;
      }
    }

    auto const addShallowConstants = [&] {
      auto const numConstants = cls.constants.size();
      for (uint32_t idx = 0; idx < numConstants; ++idx) {
        auto const added = add_constant(
          index, cinfo, state,
          cls.constants[idx].name,
          ClassInfo2::ConstIndex { cls.name, idx },
          false
        );
        if (!added) return false;
      }
      return true;
    };

    auto const addTraitConstants = [&] {
      for (auto const tname : cinfo.usedTraits) {
        auto const& trait = index.classInfo(tname);
        for (auto const& [cnsName, cnsIdx] : trait.clsConstants) {
          auto const added = add_constant(
            index, cinfo, state, cnsName,
            cnsIdx, true
          );
          if (!added) return false;
        }
      }
      return true;
    };

    if (RO::EvalTraitConstantInterfaceBehavior) {
      // trait constants must be inserted before constants shallowly
      // declared on the class to match the interface semantics
      if (!addTraitConstants()) return false;
      if (!addShallowConstants()) return false;
    } else {
      if (!addShallowConstants()) return false;
      if (!addTraitConstants()) return false;
    }

    for (auto const ename : cinfo.includedEnums) {
      auto const& e = index.classInfo(ename);
      for (auto const& [cnsName, cnsIdx] : e.clsConstants) {
        auto const added = add_constant(
          index, cinfo, state, cnsName,
          cnsIdx, false
        );
        if (!added) return false;
      }
    }

    auto const addTraitConst = [&] (const php::Const& c) {
     /*
      * Only copy in constants that win. Otherwise, in the runtime, if
      * we have a constant from an interface implemented by a trait
      * that wins over this fromTrait constant, we won't know which
      * trait it came from, and therefore won't know which constant
      * should win. Dropping losing constants here works because if
      * they fatal with constants in declared interfaces, we catch that
      * above.
      */
      auto const& existing = cinfo.clsConstants.find(c.name);
      if (existing->second.cls->isame(c.cls)) {
        state.m_traitCns.emplace_back(c);
        state.m_traitCns.back().isFromTrait = true;
      }
    };
    for (auto const tname : cinfo.usedTraits) {
      auto const& trait      = index.cls(tname);
      auto const& traitState = index.state(tname);
      for (auto const& c : trait.constants)       addTraitConst(c);
      for (auto const& c : traitState.m_traitCns) addTraitConst(c);
    }

    if (cls.attrs & (AttrAbstract | AttrInterface | AttrTrait)) return true;

    std::vector<SString> sortedClsConstants;
    sortedClsConstants.reserve(cinfo.clsConstants.size());
    for (auto const& [name, _] : cinfo.clsConstants) {
      sortedClsConstants.emplace_back(name);
    }
    std::sort(
      sortedClsConstants.begin(),
      sortedClsConstants.end(),
      string_data_lt{}
    );

    for (auto const name : sortedClsConstants) {
      auto& cnsIdx = cinfo.clsConstants.find(name)->second;
      if (cnsIdx.cls->isame(cls.name)) continue;

      auto const& cns = index.cns(cnsIdx);
      if (!cns.isAbstract || !cns.val) continue;

      if (cns.val->m_type == KindOfUninit) {
        auto const& cnsCls = index.cls(cnsIdx.cls);
        assertx(!cnsCls.methods.empty());
        assertx(cnsCls.methods.back()->name == s_86cinit.get());
        auto const& cnsCInit = *cnsCls.methods.back();

        if (cls.methods.empty() ||
            cls.methods.back()->name != s_86cinit.get()) {
          ClonedClosures clonedClosures;
          auto cloned = clone(
            index,
            cnsCInit,
            cnsCInit.name,
            cnsCInit.attrs,
            cls,
            clonedClosures,
            true
          );
          assertx(cloned);
          assertx(clonedClosures.empty());
          assertx(cloned->cls == &cls);
          cloned->clsIdx = cls.methods.size();
          auto const DEBUG_ONLY emplaced =
            cinfo.methods.emplace(cloned->name, MethTabEntry { *cloned });
          assertx(emplaced.second);
          cls.methods.emplace_back(std::move(cloned));
        } else {
          auto const DEBUG_ONLY succeeded =
            append_86cinit(cls.methods.back().get(), cnsCInit);
          assertx(succeeded);
        }
      }

      // This is similar to trait constant flattening
      auto copy = cns;
      copy.cls = cls.name;
      copy.isAbstract = false;
      state.m_cnsFromTrait.erase(copy.name);

      cnsIdx.cls = cls.name;
      cnsIdx.idx = cls.constants.size();
      cls.constants.emplace_back(std::move(copy));
    }

    return true;
  }

  static bool add_constant(const LocalIndex& index,
                           ClassInfo2& cinfo,
                           State& state,
                           SString name,
                           const ClassInfo2::ConstIndex& cnsIdx,
                           bool fromTrait) {
    auto [it, emplaced] = cinfo.clsConstants.emplace(name, cnsIdx);
    if (emplaced) {
      if (fromTrait) {
        always_assert(state.m_cnsFromTrait.emplace(name).second);
      } else {
        always_assert(!state.m_cnsFromTrait.count(name));
      }
      return true;
    }
    auto& existingIdx = it->second;

    // Same constant (from an interface via two different paths) is ok
    if (existingIdx.cls->isame(cnsIdx.cls)) return true;

    auto const& existingCnsCls = index.cls(existingIdx.cls);
    auto const& existing = index.cns(existingIdx);
    auto const& cns = index.cns(cnsIdx);

    if (existing.kind != cns.kind) {
      ITRACE(
        2,
        "Adding constant failed for `{}' because `{}' was defined by "
        "`{}' as a {} and by `{}' as a {}\n",
        cinfo.name,
        name,
        cnsIdx.cls,
        ConstModifiers::show(cns.kind),
        existingIdx.cls,
        ConstModifiers::show(existing.kind)
      );
      return false;
    }

    // Ignore abstract constants
    if (cns.isAbstract && !cns.val) return true;
    // If the existing constant in the map is concrete, then don't
    // overwrite it with an incoming abstract constant's default
    if (!existing.isAbstract && cns.isAbstract) return true;

    if (existing.val) {
      /*
       * A constant from a declared interface collides with a constant
       * (Excluding constants from interfaces a trait implements).
       *
       * Need this check otherwise constants from traits that conflict
       * with declared interfaces will silently lose and not conflict
       * in the runtime.
       *
       * Type and Context constants can be overridden.
       */
      auto const& cnsCls = index.cls(cnsIdx.cls);
      if (cns.kind == ConstModifiers::Kind::Value &&
          !existing.isAbstract &&
          (existingCnsCls.attrs & AttrInterface) &&
          !((cnsCls.attrs & AttrInterface) && fromTrait)) {
        for (auto const iface : cinfo.declInterfaces) {
          if (existingIdx.cls->isame(iface)) {
            ITRACE(
              2,
              "Adding constant failed for `{}' because "
              "`{}' was defined by both `{}' and `{}'\n",
              cinfo.name,
              name,
              cnsIdx.cls,
              existingIdx.cls
            );
            return false;
          }
        }
      }

      // Constants from traits silently lose
      if (!RO::EvalTraitConstantInterfaceBehavior && fromTrait) return true;

      if ((cnsCls.attrs & AttrInterface ||
           (RO::EvalTraitConstantInterfaceBehavior &&
            (cnsCls.attrs & AttrTrait))) &&
          (existing.isAbstract ||
           cns.kind == ConstModifiers::Kind::Type)) {
        // Because existing has val, this covers the case where it is
        // abstract with default allow incoming to win.  Also, type
        // constants from interfaces may be overridden even if they're
        // not abstract.
      } else {
        // A constant from an interface or from an included enum
        // collides with an existing constant.
        if (cnsCls.attrs & (AttrInterface | AttrEnum | AttrEnumClass) ||
            (RO::EvalTraitConstantInterfaceBehavior &&
             (cnsCls.attrs & AttrTrait))) {
          ITRACE(
            2,
            "Adding constant failed for `{}' because "
            "`{}' was defined by both `{}' and `{}'\n",
            cinfo.name,
            name,
            cnsIdx.cls,
            existingIdx.cls
          );
          return false;
        }
      }
    }

    if (fromTrait) {
      state.m_cnsFromTrait.emplace(name);
    } else {
      state.m_cnsFromTrait.erase(name);
    }
    existingIdx = cnsIdx;
    return true;
  }

  /*
   * Make a flattened table of the methods on this class.
   *
   * Duplicate method names override parent methods, unless the parent
   * method is final and the class is not a __MockClass, in which case
   * this class definitely would fatal if ever defined.
   *
   * Note: we're leaving non-overridden privates in their subclass
   * method table, here. This isn't currently "wrong", because calling
   * it would be a fatal, but note that resolve_method needs to be
   * pretty careful about privates and overriding in general.
   */
  static bool build_methods(const LocalIndex& index,
                            const php::Class& cls,
                            ClassInfo2& cinfo,
                            State& state) {
    // Interface methods are just stubs which return null. They don't
    // get inherited by their implementations.
    if (cls.attrs & AttrInterface) {
      uint32_t idx = cinfo.methods.size();
      assertx(!idx);
      for (auto const& m : cls.methods) {
        auto const res = cinfo.methods.emplace(m->name, MethTabEntry { *m });
        always_assert(res.second);
        always_assert(state.m_methodIndices.emplace(m->name, idx++).second);
        ITRACE(4, "  {}: adding method {}::{}\n",
               cls.name, cls.name, m->name);
      }
      return true;
    }

    auto const overridden = [&] (MethTabEntry& existing,
                                 MethRef meth,
                                 Attr attrs) {
      auto const& existingMeth = index.meth(existing);
      if (existingMeth.attrs & AttrFinal) {
        if (!is_mock_class(&cls)) {
          ITRACE(
            2,
            "Adding methods failed for `{}' because "
            "it tried to override final method `{}::{}'\n",
            cls.name,
            existing.meth().cls,
            existingMeth.name
          );
          return false;
        }
      }
      ITRACE(
        4,
        "{}: overriding method {}::{} with {}::{}\n",
        cls.name,
        existing.meth().cls,
        existingMeth.name,
        meth.cls,
        existingMeth.name
      );
      if (existingMeth.attrs & AttrPrivate) {
        existing.setHasPrivateAncestor();
      }
      existing.setMeth(meth);
      existing.attrs = attrs;
      existing.setTopLevel();
      return true;
    };

    // If there's a parent, start by copying its methods
    if (cls.parentName) {
      assertx(!(cls.attrs & AttrInterface));
      auto const& parentInfo = index.classInfo(cls.parentName);
      for (auto const& mte : parentInfo.methods) {
        // Don't inherit the 86* methods
        if (HPHP::Func::isSpecial(mte.first)) continue;

        auto const emplaced = cinfo.methods.emplace(mte);
        always_assert(emplaced.second);
        emplaced.first->second.clearTopLevel();

        always_assert(
          state.m_methodIndices.emplace(
            mte.first,
            index.methodIdx(cls.parentName, mte.first)
          ).second
        );

        ITRACE(
          4,
          "{}: inheriting method {}::{}\n",
          cls.name,
          cls.parentName,
          mte.first
        );
      }
    }

    auto idx = cinfo.methods.size();

    // Now add our methods.
    for (auto const& m : cls.methods) {
      auto const emplaced = cinfo.methods.emplace(m->name, MethTabEntry { *m });
      if (emplaced.second) {
        ITRACE(
          4,
          "{}: adding method {}::{}\n",
          cls.name,
          cls.name,
          m->name
        );
        always_assert(state.m_methodIndices.emplace(m->name, idx++).second);
        continue;
      }
      if ((m->attrs & AttrTrait) && (m->attrs & AttrAbstract)) {
        // Abstract methods from traits never override anything.
        continue;
      }
      if (!overridden(emplaced.first->second, MethRef { *m }, m->attrs)) {
        return false;
      }
    }

    // If our traits were previously flattened, we're done.
    if (cls.attrs & AttrNoExpandTrait) return true;

    try {
      TMIData tmid;
      for (auto const tname : cinfo.usedTraits) {
        assertx(!(cls.attrs & AttrInterface));
        auto const& tcls = index.cls(tname);
        auto const& t = index.classInfo(tname);
        std::vector<std::pair<SString, const MethTabEntry*>>
          methods(t.methods.size());
        for (auto const& [name, mte] : t.methods) {
          if (HPHP::Func::isSpecial(name)) continue;
          auto const idx = index.methodIdx(tname, name);
          assertx(!methods[idx].first);
          methods[idx] = std::make_pair(name, &mte);
        }

        for (auto const& [name, mte] : methods) {
          if (!name) continue;
          auto const& meth = index.meth(*mte);
          tmid.add(
            TraitMethod { std::make_pair(&t, &tcls), &meth, mte->attrs },
            name
          );
        }
        for (auto const cloname : t.closures) {
          auto const& clo = index.cls(cloname);
          auto const invoke = find_method(&clo, s_invoke.get());
          assertx(invoke);
          cinfo.extraMethods.emplace(MethRef { *invoke });
        }
      }

      auto const traitMethods = tmid.finish(
        std::make_pair(&cinfo, &cls),
        cls.userAttributes.count(s___EnableMethodTraitDiamond.get())
      );

      // Import the methods.
      for (auto const& mdata : traitMethods) {
        auto const method = mdata.tm.method;
        auto attrs = mdata.tm.modifiers;

        if (attrs == AttrNone) {
          attrs = method->attrs;
        } else {
          auto const attrMask =
            (Attr)(AttrPublic | AttrProtected | AttrPrivate |
                   AttrAbstract | AttrFinal);
          attrs = (Attr)((attrs        &  attrMask) |
                         (method->attrs & ~attrMask));
        }

        auto const emplaced = cinfo.methods.emplace(
          mdata.name,
          MethTabEntry { *method, attrs }
        );
        if (emplaced.second) {
          ITRACE(
            4,
            "{}: adding trait method {}::{} as {}\n",
            cls.name,
            method->cls->name, method->name, mdata.name
          );
          always_assert(
            state.m_methodIndices.emplace(mdata.name, idx++).second
          );
        } else {
          if (attrs & AttrAbstract) continue;
          if (emplaced.first->second.meth().cls->isame(cls.name)) continue;
          if (!overridden(emplaced.first->second, MethRef { *method }, attrs)) {
            return false;
          }
          state.methodIdx(index.m_ctx->name, cinfo.name, mdata.name) = idx++;
        }
        cinfo.extraMethods.emplace(MethRef { *method });
      }
    } catch (const TMIOps::TMIException& exn) {
      ITRACE(
        2,
        "Adding methods failed for `{}' importing traits: {}\n",
        cls.name, exn.what()
      );
      return false;
    }

    return true;
  }

  using ClonedClosures =
    hphp_fast_map<const php::Class*, std::unique_ptr<php::Class>>;

  static SString rename_closure(const php::Class& closure,
                                const php::Class& newContext) {
    auto n = closure.name->slice();
    auto const p = n.find(';');
    if (p != std::string::npos) n = n.subpiece(0, p);
    return makeStaticString(folly::sformat("{};{}", n, newContext.name));
  }

  static std::unique_ptr<php::Class>
  clone_closure(const LocalIndex& index,
                const php::Class& closure,
                const php::Class& newContext,
                ClonedClosures& clonedClosures) {
    auto clone = std::make_unique<php::Class>(closure);
    assertx(clone->closureContextCls);

    clone->name = rename_closure(closure, newContext);
    clone->closureContextCls = newContext.name;
    clone->unit = newContext.unit;

    ITRACE(4, "- cloning closure {} as {} (with context {})\n",
           closure.name, clone->name, newContext.name);

    for (size_t i = 0, numMeths = clone->methods.size(); i < numMeths; ++i) {
      auto meth = std::move(clone->methods[i]);
      meth->idx = 0; // Set later
      meth->cls = clone.get();
      assertx(meth->clsIdx == i);
      if (!meth->originalFilename) meth->originalFilename = meth->unit;
      if (!meth->originalUnit)     meth->originalUnit = meth->unit;
      if (!meth->originalClass)    meth->originalClass = closure.name;
      meth->unit = newContext.unit;

      clone->methods[i] =
        clone_closures(index, std::move(meth), clonedClosures);
      if (!clone->methods[i]) return nullptr;
    }

    return clone;
  }

  static std::unique_ptr<php::Func>
  clone_closures(const LocalIndex& index,
                 std::unique_ptr<php::Func> cloned,
                 ClonedClosures& clonedClosures) {
    if (!cloned->hasCreateCl) return cloned;

    auto const onClosure = [&] (LSString& closureName) {
      auto const& cls = index.cls(closureName);
      assertx(is_closure(cls));

      // CreateCls are allowed to refer to the same closure within the
      // same func. If this is a duplicate, use the already cloned
      // closure name.
      if (auto const it = clonedClosures.find(&cls);
          it != clonedClosures.end()) {
        closureName = it->second->name;
        return true;
      }

      // Otherwise clone the closure (which gives it a new name), and
      // update the name in the CreateCl to match.
      auto closure = clone_closure(
        index,
        cls,
        cloned->cls->closureContextCls
          ? index.cls(cloned->cls->closureContextCls)
          : *cloned->cls,
        clonedClosures
      );
      if (!closure) return false;
      closureName = closure->name;
      always_assert(clonedClosures.emplace(&cls, std::move(closure)).second);
      return true;
    };

    auto mf = php::WideFunc::mut(cloned.get());
    for (size_t bid = 0; bid < mf.blocks().size(); bid++) {
      auto const b = mf.blocks()[bid].mutate();
      for (size_t ix = 0; ix < b->hhbcs.size(); ix++) {
        auto& bc = b->hhbcs[ix];
        switch (bc.op) {
          case Op::CreateCl: {
            if (!onClosure(bc.CreateCl.str2)) return nullptr;
            break;
          }
          default:
            break;
        }
      }
    }

    return cloned;
  }

  static std::unique_ptr<php::Func> clone(const LocalIndex& index,
                                          const php::Func& orig,
                                          SString name,
                                          Attr attrs,
                                          const php::Class& dstCls,
                                          ClonedClosures& clonedClosures,
                                          bool internal = false) {
    auto cloned = std::make_unique<php::Func>(orig);
    cloned->name = name;
    cloned->attrs = attrs;
    if (!internal) cloned->attrs |= AttrTrait;
    cloned->idx = 0; // Set later
    cloned->cls = const_cast<php::Class*>(&dstCls);
    cloned->unit = dstCls.unit;

    if (!cloned->originalFilename) cloned->originalFilename = orig.unit;
    if (!cloned->originalUnit)     cloned->originalUnit = orig.unit;
    cloned->originalClass = orig.originalClass
      ? orig.originalClass
      : orig.cls->name;

    // cloned method isn't in any method table yet, so trash its
    // index.
    cloned->clsIdx = std::numeric_limits<uint32_t>::max();
    return clone_closures(index, std::move(cloned), clonedClosures);
  }

  static bool merge_inits(const LocalIndex& index,
                          const php::Class& cls,
                          const ClassInfo2& cinfo,
                          SString name,
                          std::vector<std::unique_ptr<php::Func>>& clones) {
    auto const existing = [&] () -> const php::Func* {
      for (auto const& m : cls.methods) {
        if (m->name == name) return m.get();
      }
      return nullptr;
    }();

    std::unique_ptr<php::Func> cloned;

    auto const merge = [&] (const php::Func& f) {
      if (!cloned) {
        ClonedClosures clonedClosures;
        if (existing) {
          cloned = clone(
            index,
            *existing,
            existing->name,
            existing->attrs,
            cls,
            clonedClosures,
            true
          );
          assertx(clonedClosures.empty());
          if (!cloned) return false;
        } else {
          ITRACE(4, "- cloning {}::{} as {}::{}\n",
                 f.cls->name, f.name, cls.name, name);
          cloned = clone(index, f, f.name, f.attrs, cls, clonedClosures, true);
          assertx(clonedClosures.empty());
          return (bool)cloned;
        }
      }

      ITRACE(4, "- appending {}::{} into {}::{}\n",
             f.cls->name, f.name, cls.name, name);
      if (name == s_86cinit.get()) return append_86cinit(cloned.get(), f);
      return append_func(cloned.get(), f);
    };

    for (auto const tname : cinfo.usedTraits) {
      auto const& trait = index.classInfo(tname);
      auto const it = trait.methods.find(name);
      if (it == trait.methods.end()) continue;
      auto const& meth = index.meth(it->second);
      if (!merge(meth)) {
        ITRACE(4, "merge_inits: failed to merge {}::{}\n",
               meth.cls->name, name);
        return false;
      }
    }

    if (cloned) {
      ITRACE(4, "merge_inits: adding {}::{} to method table\n",
             cloned->cls->name, cloned->name);
      clones.emplace_back(std::move(cloned));
    }

    return true;
  }

  static bool merge_xinits(const LocalIndex& index,
                           const php::Class& cls,
                           const ClassInfo2& cinfo,
                           const State& state,
                           std::vector<std::unique_ptr<php::Func>>& clones) {
    auto const merge_one = [&] (SString name, Attr attr) {
      auto const unnecessary = std::all_of(
        cinfo.traitProps.begin(),
        cinfo.traitProps.end(),
        [&] (const php::Prop& p) {
         if ((p.attrs & (AttrStatic | AttrLSB)) != attr) return true;
         if (p.val.m_type != KindOfUninit) return true;
         if (p.attrs & AttrLateInit) return true;
         return false;
       }
      );
      if (unnecessary) return true;
      return merge_inits(index, cls, cinfo, name, clones);
    };

    if (!merge_one(s_86pinit.get(), AttrNone))             return false;
    if (!merge_one(s_86sinit.get(), AttrStatic))           return false;
    if (!merge_one(s_86linit.get(), AttrStatic | AttrLSB)) return false;

    auto const unnecessary = std::all_of(
      state.m_traitCns.begin(),
      state.m_traitCns.end(),
      [&] (const php::Const& c) {
        return !c.val || c.val->m_type != KindOfUninit;
      }
    );
    if (unnecessary) return true;
    return merge_inits(index, cls, cinfo, s_86cinit.get(), clones);
  }

  struct NewClosure {
    std::unique_ptr<php::Class> cls;
    std::unique_ptr<ClassInfo2> cinfo;
  };

  static std::vector<NewClosure> flatten_traits(const LocalIndex& index,
                                                php::Class& cls,
                                                ClassInfo2& cinfo,
                                                State& state) {
    if (cls.attrs & AttrNoExpandTrait) return {};
    if (cls.usedTraitNames.empty()) {
      assertx(cinfo.usedTraits.empty());
      return {};
    }

    ITRACE(4, "flatten traits: {}\n", cls.name);
    Trace::Indent indent;

    auto traitHasConstProp = cls.hasConstProp;
    for (auto const tname : cinfo.usedTraits) {
      auto const& trait = index.cls(tname);
      auto const& tinfo = index.classInfo(tname);
      if (tinfo.usedTraits.size() && !(trait.attrs & AttrNoExpandTrait)) {
        ITRACE(4, "Not flattening {} because of {}\n", cls.name, trait.name);
        return {};
      }
      if (is_noflatten_trait(&trait)) {
        ITRACE(
          4, "Not flattening {} because {} is annotated with __NoFlatten\n",
          cls.name, trait.name
        );
        return {};
      }
      if (tinfo.hasConstProp) traitHasConstProp = true;
    }

    std::vector<std::pair<SString, MethTabEntry*>> toAdd;
    for (auto& [name, mte] : cinfo.methods) {
      if (!mte.topLevel()) continue;
      if (mte.meth().cls->isame(cls.name)) continue;
      assertx(index.cls(mte.meth().cls).attrs & AttrTrait);
      toAdd.emplace_back(name, &mte);
    }

    if (!toAdd.empty()) {
      assertx(!cinfo.extraMethods.empty());
      std::sort(
        toAdd.begin(), toAdd.end(),
        [&] (auto const& a, auto const& b) {
          return
            state.methodIdx(index.m_ctx->name, cinfo.name, a.first) <
            state.methodIdx(index.m_ctx->name, cinfo.name, b.first);
        }
      );
    } else if (debug) {
      // When building the ClassInfos, we proactively added all
      // closures from usedTraits to the extraMethods map; but now
      // we're going to start from the used methods, and deduce which
      // closures actually get pulled in. Its possible *none* of the
      // methods got used, in which case, we won't need their closures
      // either. To be safe, verify that the only things in the map
      // are closures.
      for (auto const& mte : cinfo.extraMethods) {
        auto const& meth = index.meth(mte);
        always_assert(meth.isClosureBody);
      }
    }

    std::vector<std::unique_ptr<php::Func>> clones;
    ClonedClosures clonedClosures;

    for (auto const& [name, mte] : toAdd) {
      auto const& meth = index.meth(*mte);
      auto cloned = clone(
        index,
        meth,
        name,
        mte->attrs,
        cls,
        clonedClosures
      );
      if (!cloned) {
        ITRACE(4, "Not flattening {} because {}::{} could not be cloned\n",
               cls.name, mte->meth().cls, name);
        return {};
      }
      assertx(cloned->attrs & AttrTrait);
      clones.emplace_back(std::move(cloned));
    }

    if (!merge_xinits(index, cls, cinfo, state, clones)) {
      ITRACE(4, "Not flattening {} because we couldn't merge the 86xinits\n",
             cls.name);
      return {};
    }

    // We're now committed to flattening.
    ITRACE(3, "Flattening {}\n", cls.name);

    if (traitHasConstProp) {
      assertx(cinfo.hasConstProp);
      cls.hasConstProp = true;
    }
    cinfo.extraMethods.clear();

    for (auto [_, mte] : toAdd) mte->attrs |= AttrTrait;

    for (auto& p : cinfo.traitProps) {
      ITRACE(4, "- prop {}\n", p.name);
      cls.properties.emplace_back(std::move(p));
      cls.properties.back().attrs |= AttrTrait;
    }
    cinfo.traitProps.clear();

    for (auto& c : state.m_traitCns) {
      ITRACE(4, "- const {}\n", c.name);

      auto it = cinfo.clsConstants.find(c.name);
      assertx(it != cinfo.clsConstants.end());
      auto& cnsIdx = it->second;

      c.cls = cls.name;
      state.m_cnsFromTrait.erase(c.name);
      cnsIdx.cls = cls.name;
      cnsIdx.idx = cls.constants.size();
      cls.constants.emplace_back(std::move(c));
    }
    state.m_traitCns.clear();

    std::vector<NewClosure> newClosures;
    if (!clones.empty()) {
      auto const add = [&] (std::unique_ptr<php::Func> clone) {
        assertx(clone->cls == &cls);
        clone->clsIdx = cls.methods.size();

        if (!is_special_method_name(clone->name)) {
          auto it = cinfo.methods.find(clone->name);
          assertx(it != cinfo.methods.end());
          assertx(!it->second.meth().cls->isame(cls.name));
          it->second.setMeth(MethRef { cls.name, clone->clsIdx });
        } else {
          auto const [existing, emplaced] =
            cinfo.methods.emplace(clone->name, MethTabEntry { *clone });
          if (!emplaced) {
            assertx(existing->second.meth().cls->isame(cls.name));
            if (clone->name != s_86cinit.get()) {
              auto const idx = existing->second.meth().idx;
              clone->clsIdx = idx;
              cls.methods[idx] = std::move(clone);
              return;
            } else {
              existing->second.setMeth(MethRef { cls.name, clone->clsIdx });
            }
          }
        }

        cls.methods.emplace_back(std::move(clone));
      };

      auto cinit = [&] () -> std::unique_ptr<php::Func> {
        if (cls.methods.empty()) return nullptr;
        if (cls.methods.back()->name != s_86cinit.get()) return nullptr;
        auto init = std::move(cls.methods.back());
        cls.methods.pop_back();
        return init;
      }();

      for (auto& clone : clones) {
        ITRACE(4, "- meth {}\n", clone->name);
        if (clone->name == s_86cinit.get()) {
          cinit = std::move(clone);
          continue;
        }
        add(std::move(clone));
      }
      if (cinit) add(std::move(cinit));

      for (auto& [orig, clo] : clonedClosures) {
        ITRACE(4, "- closure {} as {}\n", orig->name, clo->name);
        assertx(clo->closureContextCls->isame(cls.name));
        assertx(clo->unit == cls.unit);

        assertx(clo->usedTraitNames.empty());
        State cloState;
        auto cloinfo = make_info(index, *clo, cloState);
        assertx(cloinfo);
        assertx(cloState.m_traitCns.empty());
        assertx(cloState.m_cnsFromTrait.empty());

        cinfo.closures.emplace_back(clo->name);
        newClosures.emplace_back(
          NewClosure { std::move(clo), std::move(cloinfo) }
        );
      }

      std::sort(
        newClosures.begin(),
        newClosures.end(),
        [] (auto const& p1, auto const& p2) {
          return string_data_lti{}(p1.cls->name, p2.cls->name);
        }
      );
    }

    struct EqHash {
      bool operator()(const PreClass::ClassRequirement& a,
                      const PreClass::ClassRequirement& b) const {
        return a.is_same(&b);
      }
      size_t operator()(const PreClass::ClassRequirement& a) const {
        return a.hash();
      }
    };
    hphp_fast_set<PreClass::ClassRequirement, EqHash, EqHash> reqs{
      cls.requirements.begin(),
      cls.requirements.end()
    };

    for (auto const tname : cinfo.usedTraits) {
      auto const& trait = index.cls(tname);
      for (auto const& req : trait.requirements) {
        if (reqs.emplace(req).second) cls.requirements.emplace_back(req);
      }
    }

    cls.attrs |= AttrNoExpandTrait;
    return newClosures;
  }
};

/*
 * "Fixups" a php::Unit by removing specified funcs from it, and
 * adding specified classes. This is needed to add closures created
 * from trait flattening into their associated units. While we're
 * doing this, we also remove redundant meth caller funcs here
 * (because it's convenient).
 */
struct UnitFixupJob {
  static std::string name() { return "hhbbc-flatten-fixup"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
  }
  static void fini() {}

  // For a given unit, the classes to add and the funcs to remove.
  struct Fixup {
    std::vector<SString> addClass;
    std::vector<SString> removeFunc;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(addClass)(removeFunc);
    }
  };

  static std::unique_ptr<php::Unit> run(std::unique_ptr<php::Unit> unit,
                                        const Fixup& fixup) {
    if (!fixup.removeFunc.empty()) {
      // If we want to remove a func, it should be in this unit.
      auto DEBUG_ONLY erased = false;
      unit->funcs.erase(
        std::remove_if(
          begin(unit->funcs),
          end(unit->funcs),
          [&] (SString func) {
            // This is a kinda dumb O(N^2) algorithm, but these lists
            // are typicaly size 1.
            auto const erase = std::any_of(
              begin(fixup.removeFunc),
              end(fixup.removeFunc),
              [&] (SString remove) { return remove == func; }
            );
            if (erase) erased = true;
            return erase;
          }
        ),
        end(unit->funcs)
      );
      assertx(erased);
    }

    unit->classes.insert(
      end(unit->classes),
      begin(fixup.addClass),
      end(fixup.addClass)
    );
    return unit;
  }
};

Job<FlattenJob> s_flattenJob;
Job<UnitFixupJob> s_unitFixupJob;

/*
 * For efficiency reasons, we want to do class flattening all in one
 * pass. However, this is tricky because the algorithm is naturally
 * iterative. You start at the classes with no dependencies (the roots
 * of the class hierarchy), and flow data down to each of their
 * children. This requires N passes, where N is the maximum depth of
 * the class hierarchy. N can get large.
 *
 * Instead when we flatten a class, we ensure that all of it's
 * dependencies (all the way up to the roots) are also present in the
 * job. Since we're doing this in one pass, none of the dependencies
 * will have any calculated information, and the job will have to do
 * this first.
 *
 * It is not, in general, possible to ensure that each dependency is
 * present in exactly one job (because the dependency may be shared by
 * lots of classes which are not bucketed together). So, any given
 * dependency may end up on multiple jobs and have the same
 * information calculated for it. This is fine, as it just results in
 * some wasted work.
 *
 * We perform flattening using the following approach:
 *
 * - First we find all leaf classes in the hierarchy. These are the
 *   classes which are not dependencies of anything.
 *
 * - Bucketize the leaf classes (using standard algorithm) into N buckets.
 *
 * - For each bucket, find all of the (transitive) dependencies of the
 *   leaves and add them to that bucket (as dependencies). As stated
 *   above, the same class may end up in multiple buckets as
 *   dependencies.
 *
 * - Each job takes two kinds of input. The first is the set of
 *   classes which are actually to be flattened. These will have the
 *   flattening results returned as output from the job. The second is
 *   the set of dependencies that are required to perform flattening
 *   on the first set of inputs. These will have the same flattening
 *   algorithm applied to them, but only to obtain intermediate state
 *   to calculate the output for the first set of inputs. Their
 *   results will be thrown away.
 *
 * - So far for each bucket (each bucket will map to one job), we have
 *   a set of input classes (the leafs), and all of the dependencies
 *   for each leaf.
 *
 * - We want flattened results for every class, not just the leafs, so
 *   the dependencies need to become inputs of the first kind in at
 *   least one bucket. So, for each dependency, in one of the buckets
 *   they're already present in, we "promote" it to a full input (and
 *   will receive output for it). This is done by hashing the bucket
 *   index and class name and picking the bucket that results in the
 *   lowest hash.
 *
 * - Run the jobs. Take the outputs and turn that into a set of
 *   updates, which we then apply to the Index data structures. Some
 *   of these updates require changes to the php::Unit, which we do a
 *   in separate set of "fixup" jobs at the end.
 *
*/

struct IndexFlattenMetadata {
  struct ClassMeta {
    std::vector<SString> deps;
    std::vector<SString> closures;
    size_t idx;
  };
  ISStringToOneT<ClassMeta> cls;
  std::vector<SString> allCls;
  SStringToOneT<std::vector<SString>> unitDeletions;
};
struct FlattenClassesWork {
  std::vector<SString> classes;
  std::vector<SString> deps;
};

std::vector<FlattenClassesWork>
flatten_classes_assign(const IndexFlattenMetadata& meta) {
  trace_time trace{"flatten classes assign"};
  trace.ignore_client_stats();

  // First calculate the classes which *aren't* leafs. A class is a
  // leaf if it is not depended on by another class. The sense is
  // inverted because we want to default construct the atomics.
  std::vector<std::atomic<bool>> isNotLeaf(meta.allCls.size());
  parallel::for_each(
    meta.allCls,
    [&] (SString cls) {
      auto const onDep = [&] (SString d) {
        auto const it = meta.cls.find(d);
        if (it == meta.cls.end()) return;
        assertx(it->second.idx < isNotLeaf.size());
        isNotLeaf[it->second.idx] = true;
      };
      auto const& clsMeta = meta.cls.at(cls);
      for (auto const d : clsMeta.deps)       onDep(d);
      for (auto const clo : clsMeta.closures) onDep(clo);
    }
  );

  constexpr size_t kBucketSize = 2000;

  // Bucketize all of the leaf classes
  auto buckets = consistently_bucketize(
    [&] {
      std::vector<SString> l;
      auto const size = meta.allCls.size();
      assertx(size == isNotLeaf.size());
      l.reserve(size);
      for (size_t i = 0; i < size; ++i) {
        if (!isNotLeaf[i]) l.emplace_back(meta.allCls[i]);
      }
      return l;
    }(),
    kBucketSize
  );

  struct DepHashState {
    std::mutex lock;
    size_t lowestHash{std::numeric_limits<size_t>::max()};
    size_t lowestBucket{std::numeric_limits<size_t>::max()};
  };
  std::vector<DepHashState> depHashState{meta.allCls.size()};

  // Store all of the (transitive) dependencies for every class,
  // calculated lazily. LockFreeLazy ensures that multiple classes can
  // access this concurrently and safely calculate it on demand.
  struct DepLookup {
    ISStringSet deps;
    // Whether this class is instantiable
    bool instantiable{false};
  };
  std::vector<LockFreeLazy<DepLookup>> allDeps{meta.allCls.size()};

  // Look up all of the transitive dependencies for the given class.
  auto const findAllDeps = [&] (SString cls,
                                ISStringSet& visited,
                                auto const& self) -> const DepLookup& {
    static const DepLookup empty;

    auto const it = meta.cls.find(cls);
    if (it == meta.cls.end()) {
      FTRACE(
        4, "{} is not instantiable because it is missing\n",
        cls
      );
      return empty;
    }

    // The class exists, so look up it's dependency information.
    auto const idx = it->second.idx;
    auto const& deps = it->second.deps;
    auto const& closures = it->second.closures;

    // Check for cycles. A class involved in cyclic inheritance is not
    // instantiable (and has no dependencies). This needs to be done
    // before accessing the LockFreeLazy below, because if we are in a
    // cycle, we'll deadlock when we do so.
    auto const emplaced = visited.emplace(cls).second;
    if (!emplaced) {
      FTRACE(
        4, "{} is not instantiable because it forms a dependency "
        "cycle with itself\n", cls
      );
      return empty;
    }
    SCOPE_EXIT { visited.erase(cls); };

    assertx(idx < allDeps.size());
    return allDeps[idx].get(
      [&] {
        // Otherwise get all of the transitive dependencies of it's
        // dependencies and combine them.
        DepLookup out;
        out.instantiable = true;
        auto const onDep = [&] (SString d) {
          auto const& lookup = self(d, visited, self);
          out.deps.insert(begin(lookup.deps), end(lookup.deps));
          if (!lookup.instantiable) {
            // If the dependency is not instantiable, this isn't
            // either. Note, however, we still need to preserve the
            // already gathered dependencies, since they'll have to be
            // placed in some bucket.
            if (out.instantiable) {
              FTRACE(
                4, "{} is not instantiable because it depends on {}, "
                "which is not instantiable\n",
                cls, d
              );
            }
            out.instantiable = false;
          } else {
            // Only add this if instantiable.
            out.deps.emplace(d);
          }
        };

        for (auto const d : deps)       onDep(d);
        for (auto const clo : closures) onDep(clo);
        return out;
      }
    );
  };

  // For each bucket (which right now just contains the leaf classes),
  // find all the transitive dependencies those leaf classes need. A
  // dependency might end up in multiple buckets (because multiple
  // leafs in different buckets depend on it). We only want to
  // actually perform the flattening for those dependencies in one of
  // the buckets. So, we need a tie-breaker. We hash the name of the
  // dependency along with the bucket number. The bucket that the
  // dependency is present in with the lowest hash is what "wins".
  auto const bucketDeps = parallel::gen(
    buckets.size(),
    [&] (size_t bucketIdx) {
      assertx(bucketIdx < buckets.size());
      auto& bucket = buckets[bucketIdx];

      // Gather up all dependencies for this bucket, and remove
      // non-instantiable leaf classes.
      ISStringSet deps;
      bucket.erase(
        std::remove_if(
          begin(bucket),
          end(bucket),
          [&] (SString cls) {
            ISStringSet visited;
            auto const& lookup = findAllDeps(cls, visited, findAllDeps);
            deps.insert(begin(lookup.deps), end(lookup.deps));
            return !lookup.instantiable;
          }
        ),
        end(bucket)
      );

      // Nothing already in the bucket should be in the dependency set
      // (because the classes in the bucket are all leafs).
      if (debug) {
        for (auto const c : bucket) {
          always_assert(!deps.count(c));
        }
      }

      // For each dependency, store the bucket with the lowest hash.
      for (auto const d : deps) {
        auto const& depMeta = meta.cls.at(d);
        auto const idx = depMeta.idx;
        assertx(idx < depHashState.size());
        auto& s = depHashState[idx];
        auto const hash = hash_int64_pair(
          d->hashStatic(),
          bucketIdx
        );
        std::lock_guard<std::mutex> _{s.lock};
        if (hash < s.lowestHash) {
          s.lowestHash = hash;
          s.lowestBucket = bucketIdx;
        } else if (hash == s.lowestHash) {
          s.lowestBucket = std::min(s.lowestBucket, bucketIdx);
        }
      }

      return deps;
    }
  );

  // Now for each bucket, "promote" dependencies into a full input
  // class. The dependency is promoted in the bucket with the lowest
  // hash, which we've already calculated.
  assertx(buckets.size() == bucketDeps.size());
  auto const work = parallel::gen(
    buckets.size(),
    [&] (size_t bucketIdx) {
      auto& bucket = buckets[bucketIdx];
      auto const& deps = bucketDeps[bucketIdx];

      std::vector<SString> depOut;
      depOut.reserve(deps.size());

      for (auto const d : deps) {
        // Calculate the hash for the dependency for this bucket. If
        // the hash equals the already calulated lowest hash, promote
        // this dependency.
        auto const idx = meta.cls.at(d).idx;
        assertx(idx < depHashState.size());
        auto const& s = depHashState[idx];
        auto const hash = hash_int64_pair(
          d->hashStatic(),
          bucketIdx
        );
        if (hash == s.lowestHash && bucketIdx == s.lowestBucket) {
          bucket.emplace_back(d);
        } else {
          // Otherwise keep it as a dependency.
          depOut.emplace_back(d);
        }
      }

      // Keep deterministic ordering. Make sure there's no duplicates.
      std::sort(bucket.begin(), bucket.end(), string_data_lti{});
      std::sort(depOut.begin(), depOut.end(), string_data_lti{});
      assertx(std::adjacent_find(bucket.begin(), bucket.end()) == bucket.end());
      assertx(std::adjacent_find(depOut.begin(), depOut.end()) == depOut.end());
      return FlattenClassesWork{
        std::move(bucket),
        std::move(depOut)
      };
    }
  );

  if (Trace::moduleEnabled(Trace::hhbbc_index, 5)) {
    for (size_t i = 0; i < work.size(); ++i) {
      auto const& [classes, deps] = work[i];
      FTRACE(5, "flatten work item #{}:\n", i);
      FTRACE(5, "  classes ({}):\n", classes.size());
      for (auto const DEBUG_ONLY c : classes) FTRACE(5, "    {}\n", c);
      FTRACE(5, "  deps ({}):\n", deps.size());
      for (auto const DEBUG_ONLY d : deps) FTRACE(5, "    {}\n", d);
    }
  }

  return work;
}

// Run FixupUnitJob on all of the given fixups and store the new
// php::Unit refs in the Index.
void flatten_classes_fixup_units(IndexData& index,
                                 SStringToOneT<UnitFixupJob::Fixup> allFixups) {
  trace_time trace("flatten classes fixup units", index.sample);

  using namespace folly::gen;

  auto const run = [&] (std::vector<SString> units) -> coro::Task<void> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

    std::vector<UnitFixupJob::Fixup> fixups;

    // Gather up the fixups and ensure a deterministic ordering.
    fixups.reserve(units.size());
    for (auto const unit : units) {
      auto f = std::move(allFixups.at(unit));
      assertx(!f.addClass.empty() || !f.removeFunc.empty());
      std::sort(f.addClass.begin(), f.addClass.end(), string_data_lti{});
      std::sort(f.removeFunc.begin(), f.removeFunc.end(), string_data_lt{});
      fixups.emplace_back(std::move(f));
    }
    auto fixupRefs =
      HPHP_CORO_AWAIT(index.client->storeMulti(std::move(fixups)));
    assertx(fixupRefs.size() == units.size());

    std::vector<std::tuple<UniquePtrRef<php::Unit>, Ref<UnitFixupJob::Fixup>>>
      inputs;
    inputs.reserve(units.size());

    for (size_t i = 0, size = units.size(); i < size; ++i) {
      inputs.emplace_back(
        index.unitRefs.at(units[i]),
        std::move(fixupRefs[i])
      );
    }

    auto config = HPHP_CORO_AWAIT(index.configRef->getCopy());
    auto outputs = HPHP_CORO_AWAIT(index.client->exec(
      s_unitFixupJob,
      std::move(config),
      std::move(inputs)
    ));
    assertx(outputs.size() == units.size());

    // Every unit is already in the Index table, so we can overwrite
    // them without locking.
    for (size_t i = 0, size = units.size(); i < size; ++i) {
      index.unitRefs.at(units[i]) = std::move(outputs[i]);
    }

    HPHP_CORO_RETURN_VOID;
  };

  constexpr size_t kBucketSize = 3000;

  // Bucketize by unit
  auto buckets = consistently_bucketize(
    [&] {
      std::vector<SString> sorted;
      sorted.reserve(allFixups.size());
      for (auto& [unit, _] : allFixups) sorted.emplace_back(unit);
      std::sort(sorted.begin(), sorted.end(), string_data_lt{});
      return sorted;
    }(),
    kBucketSize
  );

  coro::wait(coro::collectRange(
    from(buckets)
      | move
      | map([&] (std::vector<SString> units) {
          return run(std::move(units)).scheduleOn(index.executor->sticky());
        })
      | as<std::vector>()
  ));
}

void flatten_classes(IndexData& index, IndexFlattenMetadata meta) {
  trace_time trace("flatten classes", index.sample);

  using namespace folly::gen;

  struct Update {
    SString name;
    UniquePtrRef<php::Class> cls;
    UniquePtrRef<ClassInfo2> cinfo;
    SString unitToAddTo;
  };
  using UpdateVec = std::vector<Update>;

  auto const run = [&] (FlattenClassesWork work) -> coro::Task<UpdateVec> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

    if (work.classes.empty()) {
      assertx(work.deps.empty());
      HPHP_CORO_RETURN(UpdateVec{});
    }

    auto classes = from(work.classes)
      | map([&] (SString c) { return index.classRefs.at(c); })
      | as<std::vector>();
    auto deps = from(work.deps)
      | map([&] (SString c) { return index.classRefs.at(c); })
      | as<std::vector>();

    auto config = HPHP_CORO_AWAIT(index.configRef->getCopy());
    auto results = HPHP_CORO_AWAIT(
      index.client->exec(
        s_flattenJob,
        std::move(config),
        singleton_vec(std::make_tuple(std::move(classes), std::move(deps)))
      )
    );
    // Every flattening job is a single work-unit, so we should only
    // ever get one result for each one.
    assertx(results.size() == 1);
    auto& [clsRefs, cinfoRefs, classMetaRef] = results[0];
    assertx(clsRefs.size() == cinfoRefs.size());

    // We need the output metadata, but everything else stays
    // uploaded.
    auto const clsMeta =
      HPHP_CORO_AWAIT(index.client->load(std::move(classMetaRef)));

    // Create the updates by combining the job output (but skipping
    // over uninstantiable classes).
    UpdateVec updates;
    updates.reserve(work.classes.size() * 3);

    size_t outputIdx = 0;
    for (auto const name : work.classes) {
      if (clsMeta.uninstantiable.count(name)) continue;
      assertx(outputIdx < clsRefs.size());
      updates.emplace_back(
        Update{
          name,
          std::move(clsRefs[outputIdx]),
          std::move(cinfoRefs[outputIdx]),
          nullptr
        }
      );
      ++outputIdx;
    }

    for (auto const& [unit, names] : clsMeta.newClosures) {
      for (auto const name : names) {
        assertx(outputIdx < clsRefs.size());
        updates.emplace_back(
          Update{
            name,
            std::move(clsRefs[outputIdx]),
            std::move(cinfoRefs[outputIdx]),
            unit
          }
        );
        ++outputIdx;
      }
    }
    assertx(outputIdx == clsRefs.size());

    HPHP_CORO_MOVE_RETURN(updates);
  };

  // Calculate the grouping of classes into work units for flattening,
  // perform the flattening, and gather all updates from the jobs.
  auto allUpdates = [&] {
    auto assignments = flatten_classes_assign(meta);

    trace_time trace2("flatten classes work", index.sample);
    return coro::wait(coro::collectRange(
      from(assignments)
        | move
        | map([&] (FlattenClassesWork w) {
            return run(std::move(w)).scheduleOn(index.executor->sticky());
          })
        | as<std::vector>()
    ));
  }();

  // Now take the updates and apply them to the Index tables. This
  // needs to be done in a single threaded context (per data
  // structure). This also gathers up all the fixups needed.
  SStringToOneT<UnitFixupJob::Fixup> unitFixups;
  {
    trace_time trace2("flatten classes update");
    trace2.ignore_client_stats();

    parallel::parallel(
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            index.classRefs.insert_or_assign(
              update.name,
              std::move(update.cls)
            );
          }
        }
      },
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            always_assert(
              index.classInfoRefs.emplace(
                update.name,
                std::move(update.cinfo)
              ).second
            );
          }
        }
      },
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            if (!update.unitToAddTo) continue;
            unitFixups[update.unitToAddTo].addClass.emplace_back(update.name);
          }
        }
        for (auto& [unit, deletions] : meta.unitDeletions) {
          unitFixups[unit].removeFunc = std::move(deletions);
        }
      }
    );
  }

  // Apply the fixups
  flatten_classes_fixup_units(index, std::move(unitFixups));
}

//////////////////////////////////////////////////////////////////////

// Set up the async state, populate the (initial) table of
// extern-worker refs in the Index, and build some metadata needed for
// class flattening.
IndexFlattenMetadata make_remote(IndexData& index,
                                 Config config,
                                 Index::Input input,
                                 std::unique_ptr<coro::TicketExecutor> executor,
                                 std::unique_ptr<Client> client,
                                 DisposeCallback dispose) {
  trace_time tracer("make remote");
  tracer.ignore_client_stats();

  index.executor = std::move(executor);
  index.client = std::move(client);
  index.disposeClient = std::move(dispose);

  // Kick off the storage of the global config. We'll start early so
  // it will (hopefully) be done before we need it.
  index.configRef = std::make_unique<coro::AsyncValue<Ref<Config>>>(
    [&index, config = std::move(config)] () mutable {
      return index.client->store(std::move(config));
    },
    index.executor->sticky()
  );

  IndexFlattenMetadata flattenMeta;
  SStringToOneT<SString> methCallerUnits;

  flattenMeta.cls.reserve(input.classes.size());
  flattenMeta.allCls.reserve(input.classes.size());

  // Add unit and class information to their appropriate tables. This
  // is also where we'll detect duplicate funcs and class names (which
  // should be caught earlier during parsing).
  for (auto& unit : input.units) {
    FTRACE(5, "unit {} -> {}\n", unit.name, unit.unit.id().toString());
    always_assert_flog(
      index.unitRefs.emplace(unit.name, std::move(unit.unit)).second,
      "Duplicate unit: {}",
      unit.name
    );
  }
  for (auto& cls : input.classes) {
    FTRACE(5, "class {} -> {}\n", cls.name, cls.cls.id().toString());
    always_assert_flog(
      index.classRefs.emplace(cls.name, std::move(cls.cls)).second,
      "Duplicate class: {}",
      cls.name
    );

    auto& meta = flattenMeta.cls[cls.name];
    if (cls.closureContext) {
      flattenMeta.cls[cls.closureContext].closures.emplace_back(cls.name);
    }
    meta.deps = std::move(cls.dependencies);
    meta.idx = flattenMeta.allCls.size();
    flattenMeta.allCls.emplace_back(cls.name);
  }
  // Funcs have an additional wrinkle, however. A func might be a meth
  // caller. Meth callers are special in that they might be present
  // (with the same name) in multiple units. However only one "wins"
  // and is actually emitted in the repo. We detect that here and
  // select a winner. The "losing" meth callers will be actually
  // removed from their unit after class flattening.
  for (auto& func : input.funcs) {
    FTRACE(5, "func {} -> {}\n", func.name, func.func.id().toString());

    if (func.methCallerUnit) {
      // If this meth caller a duplicate of one we've already seen?
      auto const [existing, emplaced] =
        methCallerUnits.emplace(func.name, func.methCallerUnit);
      if (!emplaced) {
        // It is. The duplicate shouldn't be in the same unit,
        // however.
        always_assert_flog(
          existing->second != func.methCallerUnit,
          "Duplicate meth-caller {} in same unit {}",
          func.name,
          func.methCallerUnit
        );
        // The winner is the one with the unit with the "lesser"
        // name. This is completely arbitrary.
        if (string_data_lt{}(func.methCallerUnit, existing->second)) {
          // This one wins. Schedule the older entry for deletion and
          // take over it's position in the map.
          FTRACE(
            4, "  meth caller {} from unit {} taking priority over unit {}",
            func.name, func.methCallerUnit, existing->second
          );
          flattenMeta.unitDeletions[existing->second].emplace_back(func.name);
          existing->second = func.methCallerUnit;
          index.funcRefs.at(func.name) = std::move(func.func);
        } else {
          // This one loses. Schedule it for deletion.
          flattenMeta.unitDeletions[func.methCallerUnit].emplace_back(
            func.name
          );
        }
        continue;
      }
      // It's not. Treat it like anything else.
    }

    // If not a meth caller, treat it like anything else.
    always_assert_flog(
      index.funcRefs.emplace(func.name, std::move(func.func)).second,
      "Duplicate func: {}",
      func.name
    );
  }

  return flattenMeta;
}

//////////////////////////////////////////////////////////////////////

// Convert the ClassInfo2s we loaded from extern-worker into their
// equivalent ClassInfos (and store it in the Index).
void make_class_infos_local(IndexData& index,
                            std::vector<std::unique_ptr<ClassInfo2>> remote) {
  trace_time tracer("make class-infos local");
  tracer.ignore_client_stats();

  assertx(index.allClassInfos.empty());
  assertx(index.classInfo.empty());

  // First create a ClassInfo for each ClassInfo2. Since a ClassInfo
  // can refer to other ClassInfos, we can't do much more at this
  // stage.
  index.allClassInfos = parallel::map(
    remote,
    [&] (const std::unique_ptr<ClassInfo2>& cinfo) {
      auto out = std::make_unique<ClassInfo>();

      auto const it = index.classes.find(cinfo->name);
      always_assert_flog(
        it != end(index.classes),
        "Class-info for {} has no associated php::Class in index",
        cinfo->name
      );
      out->cls = it->second;
      return out;
    }
  );

  // Build table mapping name to ClassInfo.
  for (auto& cinfo : index.allClassInfos) {
    always_assert(
      index.classInfo.emplace(cinfo->cls->name, cinfo.get()).second
    );
  }
  index.allClassInfos.shrink_to_fit();

  auto const get = [&] (SString name) {
    auto const it = index.classInfo.find(name);
    always_assert_flog(
      it != end(index.classInfo),
      "Class-info for {} not found in index",
      name
    );
    return it->second;
  };

  auto const vec = [&] (auto const& src, auto& dst) {
    dst.reserve(src.size());
    for (auto const s : src) dst.emplace_back(get(s));
    dst.shrink_to_fit();
  };

  std::mutex extraMethodLock;

  // Now that we can map name to ClassInfo, we can populate the rest
  // of the fields in each ClassInfo.
  parallel::for_each(
    remote,
    [&] (std::unique_ptr<ClassInfo2>& rcinfo) {
      auto const cinfo = get(rcinfo->name);
      if (rcinfo->parent) {
        cinfo->parent = get(rcinfo->parent);
      }

      vec(rcinfo->declInterfaces, cinfo->declInterfaces);
      vec(rcinfo->includedEnums, cinfo->includedEnums);
      vec(rcinfo->usedTraits, cinfo->usedTraits);
      vec(rcinfo->baseList, cinfo->baseList);

      cinfo->traitProps = std::move(rcinfo->traitProps);

      cinfo->implInterfaces.reserve(rcinfo->implInterfaces.size());
      for (auto const iface : rcinfo->implInterfaces) {
        cinfo->implInterfaces.emplace(iface, get(iface));
      }

      cinfo->clsConstants.reserve(rcinfo->clsConstants.size());
      for (auto const& [name, cns] : rcinfo->clsConstants) {
        auto const it = index.classes.find(cns.cls);
        always_assert_flog(
          it != end(index.classes),
          "php::Class for {} not found in index",
          name
        );
        cinfo->clsConstants.emplace(
          name,
          ClassInfo::ConstIndex { it->second, cns.idx }
        );
      }

      {
        std::vector<std::pair<SString, MethTabEntry>> methods;
        methods.reserve(cinfo->methods.size());
        for (auto const& [name, mte] : rcinfo->methods) {
          methods.emplace_back(name, mte);
        }
        std::sort(
          begin(methods), end(methods),
          [] (auto const& p1, auto const& p2) { return p1.first < p2.first; }
        );
        cinfo->methods.insert(
          folly::sorted_unique, begin(methods), end(methods)
        );
        cinfo->methods.shrink_to_fit();
      }

      if (!rcinfo->extraMethods.empty()) {
        // This is rare. Only happens with unflattened traits, so we
        // taking a lock here is fine.
        std::lock_guard<std::mutex> _{extraMethodLock};
        auto& extra = index.classExtraMethodMap[cinfo->cls];
        for (auto const& meth : rcinfo->extraMethods) {
          extra.emplace(func_from_meth_ref(index, meth));
        }
      }

      cinfo->hasConstProp = rcinfo->hasConstProp;
      cinfo->hasReifiedParent = rcinfo->hasReifiedParent;

      // Free memory as we go.
      rcinfo.reset();
    }
  );
  remote.clear();
}

// Switch to "local" mode, in which all calculations are expected to
// be done locally (not using extern-worker). This involves
// downloading everything out of extern-worker and converting it.
void make_local(IndexData& index) {
  trace_time tracer("make local", index.sample);

  // We're going to be downloading all bytecode, so to avoid wasted
  // memory, try to re-use identical bytecode.
  php::Func::BytecodeReuser reuser;
  php::Func::s_reuser = &reuser;
  SCOPE_EXIT { php::Func::s_reuser = nullptr; };

  // For speed, split up the unit loading into chunks.
  constexpr size_t kLoadChunkSize = 2500;
  constexpr size_t kMaxConcurrentChunks = 300;

  // Chunk everything we need to load:
  struct Chunk {
    std::vector<UniquePtrRef<php::Class>> classes;
    std::vector<UniquePtrRef<ClassInfo2>> classInfos;
    std::vector<UniquePtrRef<php::Func>> funcs;
    std::vector<UniquePtrRef<php::Unit>> units;

    size_t size() const {
      return
        classes.size() + classInfos.size() +
        funcs.size() + units.size();
    }
    bool empty() const {
      return
        classes.empty() && classInfos.empty() &&
        funcs.empty() && units.empty();
    }
  };
  std::vector<Chunk> chunks;
  Chunk current;

  auto const added = [&] {
    if (current.size() >= kLoadChunkSize) {
      chunks.emplace_back(std::move(current));
    }
  };
  for (auto& [_, unit] : index.unitRefs) {
    current.units.emplace_back(std::move(unit));
    added();
  }
  for (auto& [_, cls] : index.classRefs) {
    current.classes.emplace_back(std::move(cls));
    added();
  }
  for (auto& [_, cinfo] : index.classInfoRefs) {
    current.classInfos.emplace_back(std::move(cinfo));
    added();
  }
  for (auto& [_, func] : index.funcRefs) {
    current.funcs.emplace_back(std::move(func));
    added();
  }
  if (!current.empty()) chunks.emplace_back(std::move(current));

  std::mutex lock;
  auto program = std::make_unique<php::Program>();

  // Index stores ClassInfos, not ClassInfo2s, so we need a place to
  // store them until we convert it.
  std::vector<std::unique_ptr<ClassInfo2>> remoteClassInfos;
  remoteClassInfos.reserve(index.classInfoRefs.size());

  // For each chunk, load it, and add it to the php::Program.
  auto const loadAndParse = [&] (Chunk chunk) -> coro::Task<void> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

    auto [classes, classInfos, funcs, units] = HPHP_CORO_AWAIT(coro::collect(
      index.client->load(std::move(chunk.classes)),
      index.client->load(std::move(chunk.classInfos)),
      index.client->load(std::move(chunk.funcs)),
      index.client->load(std::move(chunk.units))
    ));

    {
      std::scoped_lock<std::mutex> _{lock};
      for (auto& unit : units) {
        program->units.emplace_back(std::move(unit));
      }
      for (auto& cls : classes) {
        program->classes.emplace_back(std::move(cls));
      }
      for (auto& func : funcs) {
        program->funcs.emplace_back(std::move(func));
      }
      remoteClassInfos.insert(
        end(remoteClassInfos),
        std::make_move_iterator(begin(classInfos)),
        std::make_move_iterator(end(classInfos))
      );
    }
    HPHP_CORO_RETURN_VOID;
  };

  // We've moved all refs out of these tables. Free them now to save
  // memory before we load anything.
  decltype(index.unitRefs){}.swap(index.unitRefs);
  decltype(index.classRefs){}.swap(index.classRefs);
  decltype(index.funcRefs){}.swap(index.funcRefs);
  decltype(index.classInfoRefs){}.swap(index.classInfoRefs);

  // Load everything
  std::vector<coro::TaskWithExecutor<void>> tasks;
  tasks.reserve(chunks.size());
  for (auto& c : chunks) {
    tasks.emplace_back(
      loadAndParse(std::move(c)).scheduleOn(index.executor->sticky())
    );
  }
  coro::wait(
    coro::collectRangeWindowed(
      std::move(tasks),
      kMaxConcurrentChunks
    )
  );

  // Done with any extern-worker stuff at this point:
  index.configRef.reset();

  Logger::FInfo(
    "{}",
    index.client->getStats().toString(
      "hhbbc",
      folly::sformat(
        "{:,} units, {:,} classes, {:,} class-infos, {:,} funcs",
        program->units.size(),
        program->classes.size(),
        remoteClassInfos.size(),
        program->funcs.size()
      )
    )
  );

  if (index.sample) {
    index.client->getStats().logSample("hhbbc", *index.sample);
    index.sample->setStr(
      "hhbbc_fellback",
      index.client->fellback() ? "true" : "false"
    );
  }

  index.disposeClient(
    std::move(index.executor),
    std::move(index.client)
  );
  index.disposeClient = decltype(index.disposeClient){};

  program->units.shrink_to_fit();
  program->classes.shrink_to_fit();
  program->funcs.shrink_to_fit();
  index.program = std::move(program);

  // For now we don't require system constants in any extern-worker
  // stuff we do. So we can just add it to the Index now.
  add_system_constants_to_index(index);
  // Buid Index data structures from the php::Program.
  add_program_to_index(index);
  // Convert the ClassInfo2s into ClassInfos.
  make_class_infos_local(index, std::move(remoteClassInfos));
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

std::vector<SString> Index::Input::makeDeps(const php::Class& cls) {
  std::vector<SString> deps;
  if (cls.parentName) deps.emplace_back(cls.parentName);
  deps.insert(deps.end(), cls.interfaceNames.begin(), cls.interfaceNames.end());
  deps.insert(deps.end(), cls.usedTraitNames.begin(), cls.usedTraitNames.end());
  deps.insert(
    deps.end(),
    cls.includedEnumNames.begin(),
    cls.includedEnumNames.end()
  );
  return deps;
}

//////////////////////////////////////////////////////////////////////

Index::Index(Input input,
             Config config,
             std::unique_ptr<coro::TicketExecutor> executor,
             std::unique_ptr<Client> client,
             DisposeCallback dispose,
             StructuredLogEntry* sample)
  : m_data{std::make_unique<IndexData>(this)}
{
  trace_time tracer("create index", sample);
  m_data->sample = sample;

  auto flattenMeta = make_remote(
    *m_data,
    std::move(config),
    std::move(input),
    std::move(executor),
    std::move(client),
    std::move(dispose)
  );
  flatten_classes(*m_data, std::move(flattenMeta));

  make_local(*m_data);

  // This looks stupid, but using resize means FuncInfo needs to be
  // copyable/movable but we have an std::atomic in it (which is
  // not). This avoids that.
  m_data->funcInfo = decltype(m_data->funcInfo)(m_data->nextFuncId);

  // Part of the index building routines happens before the various asserted
  // index invariants hold.  These each may depend on computations from
  // previous functions, so be careful changing the order here.
  compute_subclass_list(*m_data);
  clean_86reifiedinit_methods(*m_data); // uses the base class lists
  mark_no_override_methods(*m_data);
  find_mocked_classes(*m_data);
  mark_const_props(*m_data);
  auto const logging = Trace::moduleEnabledRelease(Trace::hhbbc_time, 1);
  m_data->compute_iface_vtables = std::thread([&] {
      HphpSessionAndThread _{Treadmill::SessionKind::HHBBC};
      auto const enable =
        logging && !Trace::moduleEnabledRelease(Trace::hhbbc_time, 1);
      Trace::BumpRelease bumper(Trace::hhbbc_time, -1, enable);
      compute_iface_vtables(*m_data);
    }
  );
  define_func_families(*m_data);        // AttrNoOverride, iface_vtables,
                                        // subclass_list
  mark_no_override_classes(*m_data);
  check_invariants(*m_data);

  trace_time tracer_2("initialize return types", sample);
  std::vector<const php::Func*> all_funcs;
  all_funcs.reserve(m_data->funcs.size() + m_data->methods.size());
  for (auto const fn : m_data->funcs) {
    all_funcs.push_back(fn.second);
  }
  for (auto const fn : m_data->methods) {
    all_funcs.push_back(fn.second);
  }
  parallel::for_each(
    all_funcs,
    [&] (const php::Func* f) { init_return_type(f); }
  );
}

// Defined here so IndexData is a complete type for the unique_ptr
// destructor.
Index::~Index() {}

//////////////////////////////////////////////////////////////////////

const php::Program& Index::program() const {
  return *m_data->program;
}

StructuredLogEntry* Index::sample() const {
  return m_data->sample;
}

//////////////////////////////////////////////////////////////////////

const php::Class* Index::lookup_closure_context(const php::Class& cls) const {
  if (!cls.closureContextCls) return &cls;
  return m_data->classes.at(cls.closureContextCls);
}

const php::Unit* Index::lookup_func_unit(const php::Func& func) const {
  return m_data->units.at(func.unit);
}

const php::Unit* Index::lookup_func_original_unit(const php::Func& func) const {
  auto const unit = func.originalUnit ? func.originalUnit : func.unit;
  return m_data->units.at(unit);
}

const php::Unit* Index::lookup_class_unit(const php::Class& cls) const {
  return m_data->units.at(cls.unit);
}

const php::Class* Index::lookup_const_class(const php::Const& cns) const {
  return m_data->classes.at(cns.cls);
}

//////////////////////////////////////////////////////////////////////

void Index::for_each_unit_func(const php::Unit& unit,
                               std::function<void(const php::Func&)> f) const {
  for (auto const func : unit.funcs) {
    f(*m_data->funcs.at(func));
  }
}

void Index::for_each_unit_func_mutable(php::Unit& unit,
                                       std::function<void(php::Func&)> f) {
  for (auto const func : unit.funcs) {
    f(*m_data->funcs.at(func));
  }
}

void Index::for_each_unit_class(
    const php::Unit& unit,
    std::function<void(const php::Class&)> f) const {
  for (auto const cls : unit.classes) {
    f(*m_data->classes.at(cls));
  }
}

void Index::for_each_unit_class_mutable(php::Unit& unit,
                                        std::function<void(php::Class&)> f) {
  for (auto const cls : unit.classes) {
    f(*m_data->classes.at(cls));
  }
}

//////////////////////////////////////////////////////////////////////

void Index::mark_no_bad_redeclare_props(php::Class& cls) const {
  /*
   * Keep a list of properties which have not yet been found to redeclare
   * anything inequivalently. Start out by putting everything on the list. Then
   * walk up the inheritance chain, removing collisions as we find them.
   */
  std::vector<php::Prop*> props;
  for (auto& prop : cls.properties) {
    if (prop.attrs & (AttrStatic | AttrPrivate)) {
      // Static and private properties never redeclare anything so need not be
      // considered.
      attribute_setter(prop.attrs, true, AttrNoBadRedeclare);
      continue;
    }
    attribute_setter(prop.attrs, false, AttrNoBadRedeclare);
    props.emplace_back(&prop);
  }

  auto currentCls = [&]() -> const ClassInfo* {
    auto const rcls = resolve_class(&cls);
    if (rcls.val.left()) return nullptr;
    return rcls.val.right();
  }();
  // If there's one more than one resolution for the class, be conservative and
  // we'll treat everything as possibly redeclaring.
  if (!currentCls) props.clear();

  while (!props.empty()) {
    auto const parent = currentCls->parent;
    if (!parent) {
      // No parent. We're done, so anything left on the prop list is
      // AttrNoBadRedeclare.
      for (auto& prop : props) {
        attribute_setter(prop->attrs, true, AttrNoBadRedeclare);
      }
      break;
    }

    auto const findParentProp = [&] (SString name) -> const php::Prop* {
      for (auto& prop : parent->cls->properties) {
        if (prop.name == name) return &prop;
      }
      for (auto& prop : parent->traitProps) {
        if (prop.name == name) return &prop;
      }
      return nullptr;
    };

    // Remove any properties which collide with the current class.

    auto const propRedeclares = [&] (php::Prop* prop) {
      auto const pprop = findParentProp(prop->name);
      if (!pprop) return false;

      // We found a property being redeclared. Check if the type-hints on
      // the two are equivalent.
      auto const equivOneTCPair =
      [&](const TypeConstraint& tc1, const TypeConstraint& tc2) {
        // Try the cheap check first, use the index otherwise. Two
        // type-constraints are equivalent if all the possible values of one
        // satisfies the other, and vice-versa.
        if (!tc1.maybeInequivalentForProp(tc2)) return true;
        return
          satisfies_constraint(
            Context{},
            lookup_constraint(Context{}, tc1),
            tc2
          ) && satisfies_constraint(
            Context{},
            lookup_constraint(Context{}, tc2),
            tc1
          );
      };
      auto const equiv = [&] {
        if (!equivOneTCPair(prop->typeConstraint, pprop->typeConstraint)) {
          return false;
        }
        for (auto ub : prop->ubs) {
          applyFlagsToUB(ub, prop->typeConstraint);
          auto foundEquiv = false;
          for (auto pub : pprop->ubs) {
            applyFlagsToUB(pub, pprop->typeConstraint);
            if (equivOneTCPair(ub, pub)) {
              foundEquiv = true;
              break;
            }
          }
          if (!foundEquiv) return false;
        }
        return true;
      };
      // If the property in the parent is static or private, the property in
      // the child isn't actually redeclaring anything. Otherwise, if the
      // type-hints are equivalent, remove this property from further
      // consideration and mark it as AttrNoBadRedeclare.
      if ((pprop->attrs & (AttrStatic | AttrPrivate)) || equiv()) {
        attribute_setter(prop->attrs, true, AttrNoBadRedeclare);
      }
      return true;
    };

    props.erase(
      std::remove_if(props.begin(), props.end(), propRedeclares),
      props.end()
    );

    currentCls = parent;
  }

  auto const possibleOverride =
    std::any_of(
      cls.properties.begin(),
      cls.properties.end(),
      [&](const php::Prop& prop) { return !(prop.attrs & AttrNoBadRedeclare); }
    );

  // Mark all resolutions of this class as having any possible bad redeclaration
  // props, even if there's not an unique resolution.
  auto const it = m_data->classInfo.find(cls.name);
  if (it != end(m_data->classInfo)) {
    auto const cinfo = it->second;
    if (cinfo->cls == &cls) {
      cinfo->hasBadRedeclareProp = possibleOverride;
    }
  }
}

/*
 * Rewrite the initial values for any AttrSystemInitialValue properties. If the
 * properties' type-hint does not admit null values, change the initial value to
 * one (if possible) to one that is not null. This is only safe to do so if the
 * property is not redeclared in a derived class or if the redeclaration does
 * not have a null system provided default value. Otherwise, a property can have
 * a null value (even if its type-hint doesn't allow it) without the JIT
 * realizing that its possible.
 *
 * Note that this ignores any unflattened traits. This is okay because
 * properties pulled in from traits which match an already existing property
 * can't change the initial value. The runtime will clear AttrNoImplicitNullable
 * on any property pulled from the trait if it doesn't match an existing
 * property.
 */
void Index::rewrite_default_initial_values() const {
  trace_time tracer("rewrite default initial values", m_data->sample);

  /*
   * Use dataflow across the whole program class hierarchy. Start from the
   * classes which have no derived classes and flow up the hierarchy. We flow
   * the set of properties which have been assigned a null system provided
   * default value. If a property with such a null value flows into a class
   * which declares a property with the same name (and isn't static or private),
   * than that property is forced to be null as well.
   */
  using PropSet = folly::F14FastSet<SString>;
  using OutState = folly::F14FastMap<const ClassInfo*, PropSet>;
  using Worklist = folly::F14FastSet<const ClassInfo*>;

  OutState outStates;
  outStates.reserve(m_data->allClassInfos.size());

  // List of Class' still to process this iteration
  using WorkList = std::vector<const ClassInfo*>;
  using WorkSet = folly::F14FastSet<const ClassInfo*>;

  WorkList workList;
  WorkSet workSet;
  auto const enqueue = [&] (const ClassInfo& cls) {
    auto const result = workSet.insert(&cls);
    if (!result.second) return;
    workList.emplace_back(&cls);
  };

  // Start with all the leaf classes
  for (auto const& cinfo : m_data->allClassInfos) {
    if (!(cinfo->cls->attrs & AttrNoOverride)) continue;
    enqueue(*cinfo);
  }

  WorkList oldWorkList;
  int iter = 1;
  while (!workList.empty()) {
    FTRACE(
      4, "rewrite_default_initial_values round #{}: {} items\n",
      iter, workList.size()
    );
    ++iter;

    std::swap(workList, oldWorkList);
    workList.clear();
    workSet.clear();
    for (auto const& cinfo : oldWorkList) {
      // Retrieve the set of properties which are flowing into this Class and
      // have to be null.
      auto inState = [&] () -> Optional<PropSet> {
        PropSet in;
        for (auto const& sub : cinfo->subclassList) {
          if (sub == cinfo || sub->parent != cinfo) continue;
          auto const it = outStates.find(sub);
          if (it == outStates.end()) return std::nullopt;
          in.insert(it->second.begin(), it->second.end());
        }
        return in;
      }();
      if (!inState) continue;

      // Modify the in-state depending on the properties declared on this Class
      auto const cls = cinfo->cls;
      for (auto const& prop : cls->properties) {
        if (prop.attrs & (AttrStatic | AttrPrivate)) {
          // Private or static properties can't be redeclared
          inState->erase(prop.name);
          continue;
        }
        // Ignore properties which have actual user provided initial values or
        // are LateInit.
        if (!(prop.attrs & AttrSystemInitialValue) ||
            (prop.attrs & AttrLateInit)) {
          continue;
        }
        // Forced to be null, nothing to do
        if (inState->count(prop.name) > 0) continue;

        // Its not forced to be null. Find a better default value. If its null
        // anyways, force any properties this redeclares to be null as well.
        auto const defaultValue = prop.typeConstraint.defaultValue();
        if (defaultValue.m_type == KindOfNull) inState->insert(prop.name);
      }

      // Push the in-state to the out-state.
      auto const result = outStates.emplace(std::make_pair(cinfo, *inState));
      if (result.second) {
        if (cinfo->parent) enqueue(*cinfo->parent);
      } else {
        // There shouldn't be cycles in the inheritance tree, so the out state
        // of Class', once set, should never change.
        assertx(result.first->second == *inState);
      }
    }
  }

  // Now that we've processed all the classes, rewrite the property initial
  // values, unless they are forced to be nullable.
  for (auto& c : m_data->program->classes) {
    if (is_closure(*c)) continue;

    auto const out = [&] () -> Optional<PropSet> {
      Optional<PropSet> props;
      auto const range = m_data->classInfo.equal_range(c->name);
      for (auto it = range.first; it != range.second; ++it) {
        if (it->second->cls != c.get()) continue;
        auto const outStateIt = outStates.find(it->second);
        if (outStateIt == outStates.end()) return std::nullopt;
        if (!props) props.emplace();
        props->insert(outStateIt->second.begin(), outStateIt->second.end());
      }
      return props;
    }();

    for (auto& prop : c->properties) {
      auto const nullable = [&] {
        if (!(prop.attrs & (AttrStatic | AttrPrivate))) {
          if (!out || out->count(prop.name)) return true;
        }
        if (!(prop.attrs & AttrSystemInitialValue)) return false;
        return prop.typeConstraint.defaultValue().m_type == KindOfNull;
      }();

      attribute_setter(prop.attrs, !nullable, AttrNoImplicitNullable);
      if (!(prop.attrs & AttrSystemInitialValue)) continue;
      if (prop.val.m_type == KindOfUninit) {
        assertx(prop.attrs & AttrLateInit);
        continue;
      }

      prop.val = [&] {
        if (nullable) return make_tv<KindOfNull>();
        // Give the 86reified_prop a special default value to avoid
        // pessimizing the inferred type (we want it to always be a
        // vec of a specific size).
        if (prop.name == s_86reified_prop.get()) {
          return get_default_value_of_reified_list(c->userAttributes);
        }
        return prop.typeConstraint.defaultValue();
      }();
    }
  }
}

void Index::preinit_bad_initial_prop_values() {
  trace_time tracer("preinit bad initial prop values", m_data->sample);
  parallel::for_each(
    m_data->allClassInfos,
    [&] (std::unique_ptr<ClassInfo>& cinfo) {
      if (is_used_trait(*cinfo->cls)) return;

      cinfo->hasBadInitialPropValues = false;
      for (auto& prop : const_cast<php::Class*>(cinfo->cls)->properties) {
        if (prop_might_have_bad_initial_value(*this, *cinfo->cls, prop)) {
          cinfo->hasBadInitialPropValues = true;
          prop.attrs = (Attr)(prop.attrs & ~AttrInitialSatisfiesTC);
        } else {
          prop.attrs |= AttrInitialSatisfiesTC;
        }
      }
    }
  );
}

void Index::preresolve_type_structures() {
  trace_time tracer("pre-resolve type-structures", m_data->sample);

  // First resolve and update type-aliases. We do this first because
  // the resolutions may help us resolve the type-constants below
  // faster.
  struct TAUpdate {
    php::TypeAlias* typeAlias;
    SArray ts;
  };

  auto const taUpdates = parallel::map(
    m_data->program->units,
    [&] (const std::unique_ptr<php::Unit>& unit) {
      CompactVector<TAUpdate> updates;
      for (auto const& typeAlias : unit->typeAliases) {
        assertx(typeAlias->resolvedTypeStructure.isNull());
        if (auto const ts =
            resolve_type_structure(*this, *typeAlias).sarray()) {
          updates.emplace_back(TAUpdate{ typeAlias.get(), ts });
        }
      }
      return updates;
    }
  );

  parallel::for_each(
    taUpdates,
    [&] (const CompactVector<TAUpdate>& updates) {
      for (auto const& u : updates) {
        assertx(u.ts->isStatic());
        assertx(u.ts->isDictType());
        assertx(!u.ts->empty());
        u.typeAlias->resolvedTypeStructure =
          Array::attach(const_cast<ArrayData*>(u.ts));
      }
    }
  );

  // Then do the type-constants. Here we not only resolve the
  // type-structures, we make a copy of each type-constant for each
  // class. The reason is that a type-structure may be resolved
  // differently for each class in the inheritance hierarchy (due to
  // this::). By making a separate copy for each class, we can resolve
  // the type-structure specifically for that class.
  struct CnsUpdate {
    ClassInfo* to;
    ClassInfo::ConstIndex from;
    php::Const cns;
  };

  auto const cnsUpdates = parallel::map(
    m_data->allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      CompactVector<CnsUpdate> updates;
      for (auto const& kv : cinfo->clsConstants) {
        auto const& cns = *kv.second;
        assertx(!cns.resolvedTypeStructure);
        if (!cns.val.has_value()) continue;
        if (cns.kind != ConstModifiers::Kind::Type) continue;
        assertx(tvIsDict(*cns.val));

        // If we can resolve it, schedule an update
        if (auto const resolved =
            resolve_type_structure(*this, cns, *cinfo->cls).sarray()) {
          auto newCns = cns;
          newCns.resolvedTypeStructure = resolved;
          updates.emplace_back(CnsUpdate{ cinfo.get(), kv.second, newCns });
        } else if (cinfo->cls != kv.second.cls) {
          // Even if we can't, we need to copy it anyways (unless it's
          // already in it's original location).
          updates.emplace_back(CnsUpdate{ cinfo.get(), kv.second, cns });
        }
      }
      return updates;
    }
  );

  parallel::for_each(
    cnsUpdates,
    [&] (const CompactVector<CnsUpdate>& updates) {
      for (auto const& u : updates) {
        assertx(u.cns.val.has_value());
        assertx(u.cns.kind == ConstModifiers::Kind::Type);

        if (u.to->cls == u.from.cls) {
          assertx(u.from.idx < u.to->cls->constants.size());
          const_cast<php::Class*>(u.to->cls)->constants[u.from.idx] = u.cns;
        } else {
          auto const idx = u.to->cls->constants.size();
          const_cast<php::Class*>(u.to->cls)->constants.emplace_back(u.cns);
          u.to->clsConstants.insert_or_assign(
            u.cns.name,
            ClassInfo::ConstIndex{ u.to->cls, (uint32_t)idx }
          );
        }
      }
    }
  );

  // Now that everything has been updated, calculate the invariance
  // for each resolved type-structure. For each class constant,
  // examine all subclasses and see how the resolved type-structure
  // changes.
  parallel::for_each(
    m_data->allClassInfos,
    [&] (std::unique_ptr<ClassInfo>& cinfo) {
      for (auto& cns : const_cast<php::Class*>(cinfo->cls)->constants) {
        assertx(cns.invariance == php::Const::Invariance::None);
        if (cns.kind != ConstModifiers::Kind::Type) continue;
        if (!cns.val.has_value()) continue;
        if (!cns.resolvedTypeStructure) continue;

        auto const checkClassname =
          tvIsString(cns.resolvedTypeStructure->get(s_classname));

        // Assume it doesn't change
        auto invariance = php::Const::Invariance::Same;
        for (auto const& s : cinfo->subclassList) {
          assertx(invariance != php::Const::Invariance::None);
          assertx(
            IMPLIES(!checkClassname,
                    invariance != php::Const::Invariance::ClassnamePresent)
          );
          if (s == cinfo.get()) continue;

          auto const it = s->clsConstants.find(cns.name);
          assertx(it != s->clsConstants.end());
          if (it->second.cls != s->cls) continue;
          auto const& scns = *it->second;

          // Overridden in some strange way. Be pessimistic.
          if (!scns.val.has_value() ||
              scns.kind != ConstModifiers::Kind::Type) {
            invariance = php::Const::Invariance::None;
            break;
          }

          // The resolved type structure in this subclass is not the
          // same.
          if (scns.resolvedTypeStructure != cns.resolvedTypeStructure) {
            if (!scns.resolvedTypeStructure) {
              // It's not even resolved here, so we can't assume
              // anything.
              invariance = php::Const::Invariance::None;
              break;
            }
            // We might still be able to assert that a classname is
            // always present, or a resolved type structure at least
            // exists.
            if (invariance == php::Const::Invariance::Same ||
                invariance == php::Const::Invariance::ClassnamePresent) {
              invariance =
                (checkClassname &&
                 tvIsString(scns.resolvedTypeStructure->get(s_classname)))
                ? php::Const::Invariance::ClassnamePresent
                : php::Const::Invariance::Present;
            }
          }
        }

        if (invariance != php::Const::Invariance::None) {
          cns.invariance = invariance;
        }
      }
    }
  );
}

//////////////////////////////////////////////////////////////////////

const CompactVector<const php::Class*>*
Index::lookup_closures(const php::Class* cls) const {
  auto const it = m_data->classClosureMap.find(cls);
  if (it != end(m_data->classClosureMap)) {
    return &it->second;
  }
  return nullptr;
}

const hphp_fast_set<const php::Func*>*
Index::lookup_extra_methods(const php::Class* cls) const {
  if (cls->attrs & AttrNoExpandTrait) return nullptr;
  auto const it = m_data->classExtraMethodMap.find(cls);
  if (it != end(m_data->classExtraMethodMap)) {
    return &it->second;
  }
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

res::Class Index::resolve_class(const php::Class* cls) const {

  auto const it = m_data->classInfo.find(cls->name);
  if (it != end(m_data->classInfo)) {
    auto const cinfo = it->second;
    if (cinfo->cls == cls) {
      return res::Class { cinfo };
    }
  }

  // We know its a class, not an enum or type alias, so return
  // by name
  return res::Class { cls->name.get() };
}

Optional<res::Class> Index::resolve_class(Context ctx,
                                          SString clsName) const {
  clsName = normalizeNS(clsName);

  if (ctx.cls) {
    if (ctx.cls->name->isame(clsName)) {
      return resolve_class(ctx.cls);
    }
    if (ctx.cls->parentName && ctx.cls->parentName->isame(clsName)) {
      if (auto const parent = resolve_class(ctx.cls).parent()) return parent;
    }
  }

  auto const it = m_data->classInfo.find(clsName);
  if (it != end(m_data->classInfo)) {
    auto const tinfo = it->second;
    /*
     * If the preresolved ClassInfo is Unique we can give it out.
     */
    assertx(tinfo->cls->attrs & AttrUnique);
    if (debug && m_data->typeAliases.count(clsName)) {
      std::fprintf(stderr, "non unique \"unique\" class: %s\n",
                   tinfo->cls->name->data());

      auto const ta = m_data->typeAliases.find(clsName);
      if (ta != end(m_data->typeAliases)) {
        std::fprintf(stderr, "   and type-alias %s\n",
                      ta->second->name->data());
      }
      always_assert(false);
    }
    return res::Class { tinfo };
  }
  // We refuse to have name-only resolutions of enums and typeAliases,
  // so that all name only resolutions can be treated as classes.
  if (!m_data->enums.count(clsName) &&
      !m_data->typeAliases.count(clsName)) {
    return res::Class { clsName };
  }

  return std::nullopt;
}

Optional<res::Class> Index::selfCls(const Context& ctx) const {
  if (!ctx.cls || is_used_trait(*ctx.cls)) return std::nullopt;
  return resolve_class(ctx.cls);
}

Optional<res::Class> Index::parentCls(const Context& ctx) const {
  if (!ctx.cls || !ctx.cls->parentName) return std::nullopt;
  if (auto const parent = resolve_class(ctx.cls).parent()) return parent;
  return resolve_class(ctx, ctx.cls->parentName);
}

const php::TypeAlias* Index::lookup_type_alias(SString name) const {
  auto const it = m_data->typeAliases.find(name);
  if (it == m_data->typeAliases.end()) return nullptr;
  return it->second;
}

Index::ResolvedInfo<Optional<res::Class>>
Index::resolve_type_name(SString inName) const {
  Optional<hphp_fast_set<const void*>> seen;

  auto nullable = false;
  auto name = inName;

  for (unsigned i = 0; ; ++i) {
    name = normalizeNS(name);
    auto const cls_it = m_data->classInfo.find(name);
    if (cls_it != end(m_data->classInfo)) {
      auto const cinfo = cls_it->second;
      assertx(cinfo->cls->attrs & AttrUnique);
      if (!(cinfo->cls->attrs & AttrEnum)) {
        return { AnnotType::Object, nullable, res::Class { cinfo } };
      }
      auto const& tc = cinfo->cls->enumBaseTy;
      assertx(!tc.isNullable());
      if (!tc.isUnresolved()) {
        auto const type = tc.isMixed() ? AnnotType::ArrayKey : tc.type();
        assertx(type != AnnotType::Object);
        return { type, nullable, {} };
      }
      name = tc.typeName();
    } else {
      auto const ta_it = m_data->typeAliases.find(name);
      if (ta_it == end(m_data->typeAliases)) break;
      auto const ta = ta_it->second;
      assertx(ta->attrs & AttrUnique);
      nullable = nullable || ta->nullable;
      if (ta->type != AnnotType::Unresolved) {
        assertx(ta->type != AnnotType::Object);
        return { ta->type, nullable, {} };
      }
      name = ta->value;
    }

    // deal with cycles. Since we don't expect to
    // encounter them, just use a counter until we hit a chain length
    // of 10, then start tracking the names we resolve.
    if (i == 10) {
      seen.emplace();
      seen->insert(name);
    } else if (i > 10) {
      if (!seen->insert(name).second) {
        return { AnnotType::Unresolved, false, {} };
      }
    }
  }

  return { AnnotType::Object, nullable, res::Class { name } };
}

struct Index::ConstraintResolution {
  /* implicit */ ConstraintResolution(Type type)
    : type{std::move(type)}
    , maybeMixed{false} {}
  ConstraintResolution(Optional<Type> type, bool maybeMixed)
    : type{std::move(type)}
    , maybeMixed{maybeMixed} {}

  Optional<Type> type;
  bool maybeMixed;
};

Index::ConstraintResolution Index::resolve_named_type(
  const Context& ctx, SString name, const Type& candidate) const {

  auto const res = resolve_type_name(name);

  if (res.nullable && candidate.subtypeOf(BInitNull)) return TInitNull;

  if (res.type == AnnotType::Unresolved) return TInitCell;

  if (res.type == AnnotType::Object) {
    auto resolve = [&] (const res::Class& rcls) -> Optional<Type> {
      if (!interface_supports_non_objects(rcls.name()) ||
          candidate.subtypeOf(BOptObj)) {
        return subObj(rcls);
      }

      if (candidate.subtypeOf(BOptVec)) {
        if (interface_supports_arrlike(rcls.name())) return TVec;
      } else if (candidate.subtypeOf(BOptDict)) {
        if (interface_supports_arrlike(rcls.name())) return TDict;
      } else if (candidate.subtypeOf(BOptKeyset)) {
        if (interface_supports_arrlike(rcls.name())) return TKeyset;
      } else if (candidate.subtypeOf(BOptStr)) {
        if (interface_supports_string(rcls.name())) return TStr;
      } else if (candidate.subtypeOf(BOptInt)) {
        if (interface_supports_int(rcls.name())) return TInt;
      } else if (candidate.subtypeOf(BOptDbl)) {
        if (interface_supports_double(rcls.name())) return TDbl;
      }
      return std::nullopt;
    };

    auto ty = resolve(*res.value);
    if (ty && res.nullable) *ty = opt(std::move(*ty));
    return ConstraintResolution{ std::move(ty), false };
  }

  return get_type_for_annotated_type(ctx, res.type, res.nullable, nullptr,
                                     candidate);
}

std::pair<res::Class, const php::Class*>
Index::resolve_closure_class(Context ctx, SString name) const {
  auto const it = m_data->classes.find(name);
  always_assert_flog(
    it != m_data->classes.end(),
    "Unknown closure class `{}`",
    name
  );
  auto const cls = it->second;
  assertx(cls->unit == ctx.unit->filename);
  assertx(is_closure(*cls));
  auto const rcls = resolve_class(cls);

  // Closure classes must be unique and defined in the unit that uses
  // the CreateCl opcode, so resolution must succeed.
  always_assert_flog(
    rcls.resolved(),
    "A closure class ({}) failed to resolve",
    cls->name
  );

  return { rcls, cls };
}

res::Class Index::builtin_class(SString name) const {
  auto const rcls = resolve_class(Context {}, name);
  always_assert_flog(
    rcls.has_value() &&
    rcls->val.right() &&
    (rcls->val.right()->cls->attrs & AttrBuiltin),
    "A builtin class ({}) failed to resolve",
    name->data()
  );
  return *rcls;
}

res::Class Index::wait_handle_class() const {
  return m_data->lazyWaitHandleCls.get(
    [&] {
      auto const awaitable = builtin_class(s_Awaitable.get());
      auto const without = awaitable.withoutNonRegular();
      assertx(without.has_value());
      return *without;
    }
  );
}

// Given a DCls, return the most specific res::Func for that DCls. For
// intersections, this will call process/general on every component of
// the intersection and combine the results. For non-intersections, it
// will call process/general on the sole member of the DCls. process
// is called to obtain a res::Func from a ClassInfo. If a ClassInfo
// isn't available, general will be called instead.
template <typename P, typename G>
res::Func Index::rfunc_from_dcls(const DCls& dcls,
                                 SString name,
                                 const P& process,
                                 const G& general) {
  if (!dcls.isIsect()) {
    // If this isn't an intersection, there's only one cinfo to
    // process and we're done.
    auto const cinfo = dcls.cls().val.right();
    if (!cinfo) return general(dcls.containsNonRegular());
    return process(cinfo, dcls.isExact(), dcls.containsNonRegular());
  }

  /*
   * Otherwise get a res::Func for all members of the intersection and
   * combine them together. Since the DCls represents a class which is
   * a subtype of every ClassInfo in the list, every res::Func we get
   * is true.
   *
   * The relevant res::Func types in order from most general to more
   * specific are:
   *
   * MethodName -> FuncFamily -> MethodOrMissing -> Method -> Missing
   *
   * Since every res::Func in the intersection is true, we take the
   * res::Func which is most specific. Two different res::Funcs cannot
   * be contradict. For example, we shouldn't get a Method and a
   * Missing since one implies there's no func and the other implies
   * one specific func. Or two different res::Funcs shouldn't resolve
   * to two different methods.
   */

  using Func = res::Func;

  auto missing = TriBool::Maybe;
  Func::Isect isect;
  const php::Func* singleMethod = nullptr;

  for (auto const i : dcls.isect()) {
    auto const cinfo = i.val.right();
    if (!cinfo) continue;

    auto const func = process(cinfo, false, dcls.containsNonRegular());
    match<void>(
      func.val,
      [&] (Func::MethodName) {},
      [&] (Func::Method m) {
        assertx(IMPLIES(singleMethod, singleMethod == m.func));
        assertx(IMPLIES(singleMethod, isect.families.empty()));
        assertx(missing != TriBool::Yes);
        if (!singleMethod) {
          singleMethod = m.func;
          isect.families.clear();
        }
        missing = TriBool::No;
      },
      [&] (Func::MethodFamily fam) {
        if (missing == TriBool::Yes) {
          assertx(!singleMethod);
          assertx(isect.families.empty());
          return;
        }
        if (singleMethod) {
          assertx(missing != TriBool::Yes);
          assertx(isect.families.empty());
          return;
        }
        assertx(missing == TriBool::Maybe);
        isect.families.emplace_back(fam.family);
        isect.regularOnly |= fam.regularOnly;
      },
      [&] (Func::MethodOrMissing m) {
        assertx(IMPLIES(singleMethod, singleMethod == m.func));
        assertx(IMPLIES(singleMethod, isect.families.empty()));
        if (missing == TriBool::Yes) {
          assertx(!singleMethod);
          assertx(isect.families.empty());
          return;
        }
        if (!singleMethod) {
          singleMethod = m.func;
          isect.families.clear();
        }
      },
      [&] (Func::Missing) {
        assertx(missing != TriBool::No);
        singleMethod = nullptr;
        isect.families.clear();
        missing = TriBool::Yes;
      },
      [&] (Func::FuncName)     { always_assert(false); },
      [&] (FuncInfo*)          { always_assert(false); },
      [&] (const Func::Isect&) { always_assert(false); }
    );
  }

  // If we got a method, that always wins. Again, every res::Func is
  // true, and method is more specific than a FuncFamily, so it is
  // preferred.
  if (singleMethod) {
    assertx(missing != TriBool::Yes);
    // If missing is Maybe, then *every* resolution was to a
    // MethodName or MethodOrMissing, so include that fact here by
    // using MethodOrMissing.
    if (missing == TriBool::Maybe) {
      return Func { Func::MethodOrMissing { singleMethod } };
    }
    return Func { Func::Method { singleMethod } };
  }
  // We only got unresolved classes. If missing is TriBool::Yes, the
  // function doesn't exist. Otherwise be pessimistic.
  if (isect.families.empty()) {
    if (missing == TriBool::Yes) return Func { Func::Missing { name } };
    assertx(missing == TriBool::Maybe);
    return general(dcls.containsNonRegular());
  }
  // Isect case. Isects always might contain missing funcs.
  assertx(missing == TriBool::Maybe);

  // We could add a FuncFamily multiple times, so remove duplicates.
  std::sort(begin(isect.families), end(isect.families));
  isect.families.erase(
    std::unique(begin(isect.families), end(isect.families)),
    end(isect.families)
  );
  // If everything simplifies down to a single FuncFamily, just use
  // that.
  if (isect.families.size() == 1) {
    return Func { Func::MethodFamily { isect.families[0], isect.regularOnly } };
  }
  return Func { std::move(isect) };
}

res::Func Index::resolve_method(Context ctx,
                                const Type& thisType,
                                SString name) const {
  assertx(thisType.subtypeOf(BCls) || thisType.subtypeOf(BObj));

  using Func = res::Func;

  /*
   * Without using the class type, try to infer a set of methods
   * using just the method name. This will, naturally, not produce
   * as precise a set as when using the class type, but it's better
   * than nothing. For all of these results, we need to include the
   * possibility of the method not existing (we cannot rule that out
   * for this situation).
   */
  auto const general = [&] (bool includeNonRegular) {
    assertx(name != s_construct.get());

    // We don't produce name-only global func families for special
    // methods, so be conservative. We don't call special methods in a
    // context where we'd expect to not know the class, so it's not
    // worthwhile.
    if (is_special_method_name(name)) {
      return Func { Func::MethodName { name } };
    }

    // Lookup up the name-only global func families for this name. If
    // we don't have one, the method cannot exist because it contains
    // every method with that name in the program.
    auto const famIt = m_data->methodFamilies.find(name);
    if (famIt == end(m_data->methodFamilies)) {
      return Func { Func::Missing { name } };
    }

    // The entry exists. Consult the correct data in it, depending on
    // whether we're including non-regular classes or not.
    auto const& entry = includeNonRegular
      ? famIt->second.m_all
      : famIt->second.m_regular;
    assertx(entry.isEmpty() || entry.isIncomplete());

    if (auto const ff = entry.funcFamily()) {
      return Func { Func::MethodFamily { ff, !includeNonRegular } };
    } else if (auto const f = entry.func()) {
      return Func { Func::MethodOrMissing { f } };
    } else {
      return Func { Func::Missing { name } };
    }
  };

  auto const process = [&] (ClassInfo* cinfo,
                            bool isExact,
                            bool includeNonRegular) {
    assertx(name != s_construct.get());

    auto const methIt = cinfo->methods.find(name);
    if (methIt == end(cinfo->methods)) {
      // We don't store metadata for special methods, so be
      // pessimistic (the lack of a method entry does not mean the
      // call might fail at runtme).
      if (is_special_method_name(name)) {
        return Func { Func::MethodName { name } };
      }
      // We're only considering this class, not it's subclasses. Since
      // it doesn't exist here, the resolution will always fail.
      if (isExact) return Func { Func::Missing { name } };
      // The method isn't present on this class, but it might be in
      // the subclasses. In most cases try a general lookup to get a
      // slightly better type than nothing.
      if (includeNonRegular ||
          !(cinfo->cls->attrs & (AttrInterface|AttrAbstract))) {
        return general(includeNonRegular);
      }

      // A special case is if we're only considering regular classes,
      // and this is an interface or abstract class. For those, we
      // "expand" the method families table to include any methods
      // defined in *all* regular subclasses. This is needed to
      // preserve monotonicity. Check this now.
      auto const famIt = cinfo->methodFamilies.find(name);
      // If no entry, treat it pessimistically like the rest of the
      // cases.
      if (famIt == end(cinfo->methodFamilies)) return general(false);

      // We found an entry. This cannot be empty (remember the method
      // is guaranteed to exist on *all* regular subclasses), and must
      // be complete (for the same reason). Use it.
      auto const& entry = famIt->second;
      assertx(!entry.isEmpty());
      assertx(entry.isComplete());
      if (auto const ff = entry.funcFamily()) {
        return Func { Func::MethodFamily { ff, true } };
      } else if (auto const func = entry.func()) {
        return Func { Func::Method { func } };
      } else {
        always_assert(false);
      }
    }
    // The method on this class.
    auto const& meth = methIt->second;
    auto const ftarget = func_from_meth_ref(*m_data, meth.meth());

    // We don't store method family information about special methods
    // and they have special inheritance semantics.
    if (is_special_method_name(name)) {
      // If we know the class exactly, we can use ftarget.
      if (isExact) return Func { Func::Method { ftarget } };
      // The method isn't overwritten, but they don't inherit, so it
      // could be missing.
      if (meth.attrs & AttrNoOverride) {
        return Func { Func::MethodOrMissing { ftarget } };
      }
      // Otherwise be pessimistic.
      return Func { Func::MethodName { name } };
    }

    // Private method handling: Private methods have special lookup
    // rules. If we're in the context of a particular class, and that
    // class defines a private method, an instance of the class will
    // always call that private method (even if overridden) in that
    // context.
    assertx(cinfo->cls);
    if (ctx.cls == cinfo->cls) {
      // The context matches the current class. If we've looked up a
      // private method (defined on this class), then that's what
      // we'll call.
      if ((meth.attrs & AttrPrivate) && meth.topLevel()) {
        return Func { Func::Method { ftarget } };
      }
    } else if ((meth.attrs & AttrPrivate) || meth.hasPrivateAncestor()) {
      // Otherwise the context doesn't match the current class. If the
      // looked up method is private, or has a private ancestor,
      // there's a chance we'll call that method (or
      // ancestor). Otherwise there's no private method in the
      // inheritance tree we'll call.
      auto const ancestor = [&] () -> const php::Func* {
        if (!ctx.cls) return nullptr;
        // Look up the ClassInfo corresponding to the context:
        auto const it = m_data->classInfo.find(ctx.cls->name);
        if (it == end(m_data->classInfo)) return nullptr;
        auto const ctxCInfo = it->second;
        // Is this context a parent of our class?
        if (!cinfo->derivedFrom(*ctxCInfo)) return nullptr;
        // It is. See if it defines a private method.
        auto const it2 = ctxCInfo->methods.find(name);
        if (it2 == end(ctxCInfo->methods)) return nullptr;
        auto const& mte = it2->second;
        // If it defines a private method, use it.
        if ((mte.attrs & AttrPrivate) && mte.topLevel()) {
          return func_from_meth_ref(*m_data, mte.meth());
        }
        // Otherwise do normal lookup.
        return nullptr;
      }();
      if (ancestor) return Func { Func::Method { ancestor } };
    }
    // If none of the above cases trigger, we still might call a
    // private method (in a child class), but the func-family logic
    // below will handle that.

    // If we're only including regular subclasses, and this class
    // itself isn't regular, the result may not necessarily include
    // ftarget.
    if (!includeNonRegular && !is_regular_class(*cinfo->cls)) {
      // We're not including this base class. If we're exactly this
      // class, there's no method at all. It will always be missing.
      if (isExact) return Func { Func::Missing { name } };
      if (meth.noOverrideRegular()) {
        // The method isn't overridden in a subclass, but we can't
        // use the base class either. This leaves two cases. Either
        // the method isn't overridden because there are no regular
        // subclasses (in which case there's no resolution at all), or
        // because there's regular subclasses, but they use the same
        // method (in which case the result is just ftarget).
        if (!cinfo->hasRegularSubclass) {
          return Func { Func::Missing { name } };
        }
        return Func { Func::Method { ftarget } };
      }
      // We can't use the base class (because it's non-regular), but
      // the method is overridden by a regular subclass.

      // Since this is a non-regular class and we want the result for
      // the regular subset, we need to consult the aux table first.
      auto const auxIt = cinfo->methodFamiliesAux.find(name);
      if (auxIt != end(cinfo->methodFamiliesAux)) {
        // Found an entry in the aux table. Use whatever it provides.
        auto const& aux = auxIt->second;
        if (auto const ff = aux.funcFamily()) {
          return Func { Func::MethodFamily { ff, true } };
        } else if (auto const f = aux.func()) {
          return aux.isComplete()
            ? Func { Func::Method { f } }
            : Func { Func::MethodOrMissing { f } };
        } else {
          return Func { Func::Missing { name } };
        }
      }
      // No entry in the aux table. The result is the same as the
      // normal table, so fall through and use that.
    } else if (isExact ||
               meth.attrs & AttrNoOverride ||
               (!includeNonRegular && meth.noOverrideRegular())) {
      // Either we want all classes, or the base class is regular. If
      // the method isn't overridden we know it must be just ftarget
      // (the override bits include it being missing in a subclass, so
      // we know it cannot be missing either).
      return Func { Func::Method { ftarget } };
    }

    // Look up the entry in the normal method family table and use
    // whatever is there.
    auto const famIt = cinfo->methodFamilies.find(name);
    assertx(famIt != end(cinfo->methodFamilies));
    auto const& fam = famIt->second;
    assertx(!fam.isEmpty());

    if (auto const ff = fam.funcFamily()) {
      return Func { Func::MethodFamily { ff, !includeNonRegular } };
    } else if (auto const f = fam.func()) {
      return (!includeNonRegular || fam.isComplete())
        ? Func { Func::Method { f } }
        : Func { Func::MethodOrMissing { f } };
    } else {
      always_assert(false);
    }
  };

  auto const isClass = thisType.subtypeOf(BCls);
  if (name == s_construct.get()) {
    if (isClass) return Func { Func::MethodName { s_construct.get() } };
    return resolve_ctor(thisType);
  }

  if (isClass) {
    if (!is_specialized_cls(thisType)) return general(true);
  } else if (!is_specialized_obj(thisType)) {
    return general(false);
  }
  return rfunc_from_dcls(
    isClass ? dcls_of(thisType) : dobj_of(thisType),
    name,
    process,
    general
  );
}

res::Func Index::resolve_ctor(const Type& obj) const {
  assertx(obj.subtypeOf(BObj));

  using Func = res::Func;

  // Can't say anything useful if we don't know the object type.
  if (!is_specialized_obj(obj)) {
    return Func { Func::MethodName { s_construct.get() } };
  }

  return rfunc_from_dcls(
    dobj_of(obj),
    s_construct.get(),
    [&] (ClassInfo* cinfo, bool isExact, bool includeNonRegular) {
      // We're dealing with an object here, which never uses
      // non-regular classes.
      assertx(!includeNonRegular);

      // See if this class has a ctor.
      auto const methIt = cinfo->methods.find(s_construct.get());
      if (methIt == end(cinfo->methods)) {
        // There's no ctor on this class. This doesn't mean the ctor
        // won't exist at runtime, it might get the default ctor, so
        // we have to be conservative.
        return Func { Func::MethodName { s_construct.get() } };
      }

      // We have a ctor, but it might be overridden in a subclass.
      auto const& mte = methIt->second;
      assertx(!(mte.attrs & AttrStatic));
      auto const ftarget = func_from_meth_ref(*m_data, mte.meth());
      assertx(!(ftarget->attrs & AttrStatic));

      // If this class is known exactly, or we know nothing overrides
      // this ctor, we know this ctor is precisely it.
      if (isExact || mte.noOverrideRegular()) {
        // If this class isn't regular, and doesn't have any regular
        // subclasses (or if it's exact), this resolution will always
        // fail.
        if (!is_regular_class(*cinfo->cls) &&
            (isExact || !cinfo->hasRegularSubclass)) {
          return Func { Func::Missing { s_construct.get() } };
        }
        return Func { Func::Method { ftarget } };
      }

      // If this isn't a regular class, we need to check the "aux"
      // entry first (which always has priority when only looking at
      // the regular subset).
      if (!is_regular_class(*cinfo->cls)) {
        auto const auxIt = cinfo->methodFamiliesAux.find(s_construct.get());
        if (auxIt != end(cinfo->methodFamiliesAux)) {
          auto const& aux = auxIt->second;
          if (auto const ff = aux.funcFamily()) {
            return Func { Func::MethodFamily { ff, true } };
          } else if (auto const f = aux.func()) {
            return aux.isComplete()
              ? Func { Func::Method { f } }
              : Func { Func::MethodOrMissing { f } };
          } else {
            // Ctor doesn't exist in any regular subclasses. This can
            // happen with interfaces. The ctor might get the default
            // ctor at runtime, so be conservative.
            return Func { Func::MethodName { s_construct.get() } };
          }
        }
      }
      // Otherwise this class is regular (in which case there's just
      // method families, or there's no entry in aux, which means the
      // regular subset entry is the same as the full entry.

      auto const famIt = cinfo->methodFamilies.find(s_construct.get());
      assertx(famIt != cinfo->methodFamilies.end());
      auto const& fam = famIt->second;
      assertx(!fam.isEmpty());

      if (auto const ff = fam.funcFamily()) {
        return Func { Func::MethodFamily { ff, true } };
      } else if (auto const f = fam.func()) {
        // Since we're looking at the regular subset, we can assume
        // the set is complete, regardless of the flag on fam.
        return Func { Func::Method { f } };
      } else {
        always_assert(false);
      }
    },
    [&] (bool includeNonRegular) {
      assertx(!includeNonRegular);
      return Func { Func::MethodName { s_construct.get() } };
    }
  );
}

res::Func
Index::resolve_func_helper(const php::Func* func, SString name) const {
  auto name_only = [&] (bool renamable) {
    return res::Func { res::Func::FuncName { name, renamable } };
  };

  // no resolution
  if (func == nullptr) return name_only(false);

  // single resolution, in whole-program mode, that's it
  assertx(func->attrs & AttrUnique);
  return do_resolve(func);
}

res::Func Index::resolve_func(Context /*ctx*/, SString name) const {
  name = normalizeNS(name);
  auto const it = m_data->funcs.find(name);
  return resolve_func_helper((it != end(m_data->funcs)) ? it->second : nullptr, name);
}

/*
 * Gets a type for the constraint.
 *
 * If getSuperType is true, the type could be a super-type of the
 * actual type constraint (eg TCell). Otherwise its guaranteed that
 * for any t, t.subtypeOf(get_type_for_constraint<false>(ctx, tc, t)
 * implies t would pass the constraint.
 *
 * The candidate type is used to disambiguate; if we're applying a
 * Traversable constraint to a TObj, we should return
 * subObj(Traversable).  If we're applying it to an Array, we should
 * return Array.
 */
template<bool getSuperType>
Type Index::get_type_for_constraint(Context ctx,
                                    const TypeConstraint& tc,
                                    const Type& candidate) const {
  assertx(IMPLIES(!tc.isCheckable(),
                   tc.isMixed() ||
                   (tc.isUpperBound() &&
                    RuntimeOption::EvalEnforceGenericsUB == 0)));

  if (getSuperType) {
    /*
     * Soft hints (@Foo) are not checked.
     * Also upper-bound type hints are not checked when they do not error.
     */
    if (tc.isSoft() ||
        (RuntimeOption::EvalEnforceGenericsUB < 2 && tc.isUpperBound())) {
      return TCell;
    }
  }

  auto const res = get_type_for_annotated_type(
    ctx,
    tc.type(),
    tc.isNullable(),
    tc.isObject() ? tc.clsName() : tc.typeName(),
    candidate
  );
  if (res.type) return *res.type;
  // If the type constraint might be mixed, then the value could be
  // uninit. Any other type constraint implies TInitCell.
  return getSuperType ? (res.maybeMixed ? TCell : TInitCell) : TBottom;
}

bool Index::prop_tc_maybe_unenforced(const php::Class& propCls,
                                     const TypeConstraint& tc) const {
  assertx(tc.validForProp());
  if (RuntimeOption::EvalCheckPropTypeHints <= 2) return true;
  if (!tc.isCheckable()) return true;
  if (tc.isSoft()) return true;
  if (tc.isUpperBound() && RuntimeOption::EvalEnforceGenericsUB < 2) {
    return true;
  }
  auto const res = get_type_for_annotated_type(
    Context { nullptr, nullptr, &propCls },
    tc.type(),
    tc.isNullable(),
    tc.isObject() ? tc.clsName() : tc.typeName(),
    TCell
  );
  return res.maybeMixed;
}

Index::ConstraintResolution Index::get_type_for_annotated_type(
  Context ctx, AnnotType annot, bool nullable,
  SString name, const Type& candidate) const {

  if (candidate.subtypeOf(BInitNull) && nullable) {
    return TInitNull;
  }

  auto mainType = [&]() -> ConstraintResolution {
    switch (getAnnotMetaType(annot)) {
    case AnnotMetaType::Precise: {
      auto const dt = getAnnotDataType(annot);

      switch (dt) {
      case KindOfNull:         return TNull;
      case KindOfBoolean:      return TBool;
      case KindOfInt64:        return TInt;
      case KindOfDouble:       return TDbl;
      case KindOfPersistentString:
      case KindOfString:       return TStr;
      case KindOfPersistentVec:
      case KindOfVec:          return TVec;
      case KindOfPersistentDict:
      case KindOfDict:         return TDict;
      case KindOfPersistentKeyset:
      case KindOfKeyset:       return TKeyset;
      case KindOfResource:     return TRes;
      case KindOfClsMeth:      return TClsMeth;
      case KindOfObject:
        return resolve_named_type(ctx, name, candidate);
      case KindOfUninit:
      case KindOfRFunc:
      case KindOfFunc:
      case KindOfRClsMeth:
      case KindOfClass:
      case KindOfLazyClass:
        always_assert_flog(false, "Unexpected DataType");
        break;
      }
      break;
    }
    case AnnotMetaType::Mixed:
      /*
       * Here we handle "mixed", typevars, and some other ignored
       * typehints (ex. "(function(..): ..)" typehints).
       */
      return { TCell, true };
    case AnnotMetaType::Nothing:
    case AnnotMetaType::NoReturn:
      return TBottom;
    case AnnotMetaType::Nonnull:
      if (candidate.subtypeOf(BInitNull)) return TBottom;
      if (!candidate.couldBe(BInitNull))  return candidate;
      return unopt(candidate);
    case AnnotMetaType::This:
      if (auto s = selfCls(ctx)) return setctx(subObj(*s));
      break;
    case AnnotMetaType::Callable:
      break;
    case AnnotMetaType::Number:
      return TNum;
    case AnnotMetaType::ArrayKey:
      if (candidate.subtypeOf(BInt)) return TInt;
      if (candidate.subtypeOf(BStr)) return TStr;
      return TArrKey;
    case AnnotMetaType::VecOrDict:
      if (candidate.subtypeOf(BVec)) return TVec;
      if (candidate.subtypeOf(BDict)) return TDict;
      return union_of(TVec, TDict);
    case AnnotMetaType::ArrayLike:
      if (candidate.subtypeOf(BVec)) return TVec;
      if (candidate.subtypeOf(BDict)) return TDict;
      if (candidate.subtypeOf(BKeyset)) return TKeyset;
      return TArrLike;
    case AnnotMetaType::Classname:
      if (candidate.subtypeOf(BStr)) return TStr;
      if (!RuntimeOption::EvalClassnameNotices) {
        if (candidate.subtypeOf(BCls)) return TCls;
        if (candidate.subtypeOf(BLazyCls)) return TLazyCls;
      }
      break;
    case AnnotMetaType::Unresolved:
      return resolve_named_type(ctx, name, candidate);
    }
    return ConstraintResolution{ std::nullopt, false };
  }();

  if (mainType.type && nullable) {
    if (mainType.type->subtypeOf(BBottom)) {
      if (candidate.couldBe(BInitNull)) {
        mainType.type = TInitNull;
      }
    } else if (!mainType.type->couldBe(BInitNull)) {
      mainType.type = opt(*mainType.type);
    }
  }
  return mainType;
}

Type Index::lookup_constraint(Context ctx,
                              const TypeConstraint& tc,
                              const Type& t) const {
  return get_type_for_constraint<true>(ctx, tc, t);
}

bool Index::satisfies_constraint(Context ctx, const Type& t,
                                 const TypeConstraint& tc) const {
  auto const tcType = get_type_for_constraint<false>(ctx, tc, t);
  return t.moreRefined(tcType);
}

bool Index::could_have_reified_type(Context ctx,
                                    const TypeConstraint& tc) const {
  if (ctx.func->isClosureBody) {
    for (auto i = ctx.func->params.size();
         i < ctx.func->locals.size();
         ++i) {
      auto const name = ctx.func->locals[i].name;
      if (!name) return false; // named locals do not appear after unnamed local
      if (isMangledReifiedGenericInClosure(name)) return true;
    }
    return false;
  }
  if (!tc.isObject() && !tc.isUnresolved()) return false;
  auto const name = tc.isObject() ? tc.clsName() : tc.typeName();
  auto const resolved = resolve_type_name(name);
  if (resolved.type == AnnotType::Unresolved) return true;
  if (resolved.type != AnnotType::Object) return false;
  return resolved.value->couldHaveReifiedGenerics();
}

std::tuple<Type, bool> Index::verify_param_type(Context ctx, uint32_t paramId,
                                                Type t) const {
  // Builtins verify parameter types differently.
  if (ctx.func->isNative) return { t, true };

  auto const& pinfo = ctx.func->params[paramId];
  bool effectFree = true;
  std::vector<const TypeConstraint*> tcs{&pinfo.typeConstraint};
  for (auto const& tc : pinfo.upperBounds) tcs.push_back(&tc);

  for (auto const tc : tcs) {
    if (!tc->isCheckable()) continue;
    if (satisfies_constraint(ctx, t, *tc)) continue;

    effectFree = false;

    if (tc->mayCoerce() && t.couldBe(BCls | BLazyCls)) {
      // Add the result of possible coercion.
      t = union_of(std::move(t), TStr);
    }

    auto tcTy = lookup_constraint(ctx, *tc);
    if (tc->isThis()) {
      auto const cls = selfCls(ctx);
      if (cls && cls->couldBeMocked()) tcTy = unctx(std::move(tcTy));
    }

    t = intersection_of(std::move(t), std::move(tcTy));
  }

  return { t, effectFree };
}

TriBool
Index::supports_async_eager_return(res::Func rfunc) const {
  return match<TriBool>(
    rfunc.val,
    [&](res::Func::FuncName)   { return TriBool::Maybe; },
    [&](res::Func::MethodName) { return TriBool::Maybe; },
    [&](FuncInfo* finfo) {
      return func_supports_AER(finfo->func);
    },
    [&](res::Func::Method m) {
      return func_supports_AER(m.func);
    },
    [&](res::Func::MethodFamily fam) {
      return fam.family->infoFor(fam.regularOnly).m_static->m_supportsAER;
    },
    [&](res::Func::MethodOrMissing m) { return func_supports_AER(m.func); },
    [&](res::Func::Missing) { return TriBool::No; },
    [&](const res::Func::Isect& i) {
      auto aer = TriBool::Maybe;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (info.m_supportsAER == TriBool::Maybe) continue;
        assertx(IMPLIES(aer != TriBool::Maybe, aer == info.m_supportsAER));
        if (aer == TriBool::Maybe) aer = info.m_supportsAER;
      }
      return aer;
    }
  );
}

bool Index::is_effect_free_raw(const php::Func* func) const {
  return func_info(*m_data, func)->effectFree;
}

bool Index::is_effect_free(Context ctx, const php::Func* func) const {
  add_dependency(*m_data, func, ctx, Dep::InlineDepthLimit);
  return func_info(*m_data, func)->effectFree;
}

bool Index::is_effect_free(Context ctx, res::Func rfunc) const {
  auto const processFF = [&] (FuncFamily* ff, bool regularOnly) {
    for (auto const possible : ff->possibleFuncs()) {
      if (regularOnly && !possible.inRegular()) continue;
      auto const func = possible.ptr();
      add_dependency(*m_data, func, ctx, Dep::InlineDepthLimit);
      auto const effectFree = func_info(*m_data, func)->effectFree;
      if (!effectFree) return false;
    }
    return true;
  };

  return match<bool>(
    rfunc.val,
    [&](res::Func::FuncName)   { return false; },
    [&](res::Func::MethodName) { return false; },
    [&](FuncInfo* finfo)       {
      add_dependency(*m_data, finfo->func, ctx, Dep::InlineDepthLimit);
      return finfo->effectFree;
    },
    [&] (res::Func::Method m) {
      add_dependency(*m_data, m.func, ctx, Dep::InlineDepthLimit);
      return func_info(*m_data, m.func)->effectFree;
    },
    [&] (res::Func::MethodFamily fam) {
      return processFF(fam.family, fam.regularOnly);
    },
    [&] (res::Func::MethodOrMissing m) { return false; },
    [&] (res::Func::Missing) { return false; },
    [&] (const res::Func::Isect& i) {
      for (auto const ff : i.families) {
        if (processFF(ff, i.regularOnly)) return true;
      }
      return false;
    }
  );
}

// Helper function: Given a DCls, visit every subclass it represents,
// passing it to the given callable. If the callable returns false,
// stop iteration. Return false if any of the classes is unresolved,
// true otherwise. This is used to simplify the below functions which
// need to iterate over all possible subclasses and union the results.
template <typename F>
bool Index::visit_every_dcls_cls(const DCls& dcls, const F& f) const {
  if (dcls.isExact()) {
    auto const cinfo = dcls.cls().val.right();
    if (!cinfo) return false;
    if (dcls.containsNonRegular() || is_regular_class(*cinfo->cls)) {
      f(cinfo);
    }
    return true;
  } else if (dcls.isSub()) {
    auto const cinfo = dcls.cls().val.right();
    if (!cinfo) return false;
    if (cinfo->cls->attrs & AttrTrait) {
      if (dcls.containsNonRegular()) f(cinfo);
    } else {
      for (auto const sub : cinfo->subclassList) {
        if (!dcls.containsNonRegular() && !is_regular_class(*sub->cls)) {
          continue;
        }
        if (!f(sub)) break;
      }
    }
    return true;
  }
  auto const& isect = dcls.isect();
  assertx(isect.size() > 1);

  auto unresolved = false;
  res::Class::visitEverySub(
    isect,
    dcls.containsNonRegular(),
    [&] (res::Class c) {
      if (auto const cinfo = c.val.right()) {
        return f(cinfo);
      }
      unresolved = true;
      return false;
    }
  );
  return !unresolved;
}

ClsConstLookupResult<> Index::lookup_class_constant(Context ctx,
                                                    const Type& cls,
                                                    const Type& name) const {
  ITRACE(4, "lookup_class_constant: ({}) {}::{}\n",
         show(ctx), show(cls), show(name));
  Trace::Indent _;

  using R = ClsConstLookupResult<>;

  auto const conservative = [] {
    ITRACE(4, "conservative\n");
    return R{ TInitCell, TriBool::Maybe, true };
  };

  auto const notFound = [] {
    ITRACE(4, "not found\n");
    return R{ TBottom, TriBool::No, false };
  };

  if (!is_specialized_cls(cls)) return conservative();

  // We could easily support the case where we don't know the constant
  // name, but know the class (like we do for properties), by unioning
  // together all possible constants. However it very rarely happens,
  // but when it does, the set of constants to union together can be
  // huge and it becomes very expensive.
  if (!is_specialized_string(name)) return conservative();
  auto const sname = sval_of(name);

  // If this lookup is safe to cache. Some classes can have a huge
  // number of subclasses and unioning together all possible constants
  // can become very expensive. We can aleviate some of this expense
  // by caching results. We cannot cache a result when we use 86cinit
  // analysis since that can change.
  auto cachable = true;

  auto const process = [&] (const ClassInfo* ci) {
    ITRACE(4, "{}:\n", ci->cls->name);
    Trace::Indent _;

    // Does the constant exist on this class?
    auto const it = ci->clsConstants.find(sname);
    if (it == ci->clsConstants.end()) return notFound();

    // Is it a value and is it non-abstract (we only deal with
    // concrete constants).
    auto const& cns = *it->second.get();
    if (cns.kind != ConstModifiers::Kind::Value) return notFound();
    if (!cns.val.has_value()) return notFound();

    // Determine the constant's value and return it
    auto const r = [&] {
      if (cns.val->m_type == KindOfUninit) {
        // Constant is defined by a 86cinit. Use the result from
        // analysis and add a dependency. We cannot cache in this
        // case.
        cachable = false;
        auto const cnsCls = m_data->classes.at(cns.cls);
        if (ctx.func) {
          auto const cinit = cnsCls->methods.back().get();
          assertx(cinit->name == s_86cinit.get());
          add_dependency(*m_data, cinit, ctx, Dep::ClsConst);
        }

        ITRACE(4, "(dynamic)\n");
        auto const it =
          m_data->clsConstTypes.find(std::make_pair(cnsCls, cns.name));
        auto const type =
          (it == m_data->clsConstTypes.end()) ? TInitCell : it->second.type;
        return R{ type, TriBool::Yes, true };
      }

      // Fully resolved constant with a known value
      return R{ from_cell(*cns.val), TriBool::Yes, false };
    }();
    ITRACE(4, "-> {}\n", show(r));
    return r;
  };

  auto const& dcls = dcls_of(cls);
  if (dcls.isSub()) {
    // Before anything, look up this entry in the cache. We don't
    // bother with the cache for the exact case because it's quick and
    // there's little point.
    auto const cinfo = dcls.cls().val.right();
    if (!cinfo) return conservative();
    if (auto const it =
        m_data->clsConstLookupCache.find(std::make_pair(cinfo->cls, sname));
        it != m_data->clsConstLookupCache.end()) {
      ITRACE(4, "cache hit: {}\n", show(it->second));
      return it->second;
    }
  }

  Optional<R> result;
  auto const resolved = visit_every_dcls_cls(
    dcls,
    [&] (const ClassInfo* cinfo) {
      if (result) ITRACE(5, "-> {}\n", show(*result));
      auto r = process(cinfo);
      if (!result) {
        result.emplace(std::move(r));
      } else {
        *result |= r;
      }
      return true;
    }
  );
  if (!resolved) return conservative();
  assertx(result.has_value());

  // Save this for future lookups if we can
  if (dcls.isSub() && cachable) {
    auto const cinfo = dcls.cls().val.right();
    assertx(cinfo);
    m_data->clsConstLookupCache.emplace(
      std::make_pair(cinfo->cls, sname),
      *result
    );
  }

  ITRACE(4, "-> {}\n", show(*result));
  return *result;
}

ClsTypeConstLookupResult<>
Index::lookup_class_type_constant(
    const Type& cls,
    const Type& name,
    const ClsTypeConstLookupResolver& resolver) const {
  ITRACE(4, "lookup_class_type_constant: {}::{}\n", show(cls), show(name));
  Trace::Indent _;

  using R = ClsTypeConstLookupResult<>;

  auto const conservative = [] {
    ITRACE(4, "conservative\n");
    return R{
      TypeStructureResolution { TSDictN, true },
      TriBool::Maybe,
      TriBool::Maybe
    };
  };

  auto const notFound = [] {
    ITRACE(4, "not found\n");
    return R {
      TypeStructureResolution { TBottom, false },
      TriBool::No,
      TriBool::No
    };
  };

  // Unlike lookup_class_constant, we distinguish abstract from
  // not-found, as the runtime sometimes treats them differently.
  auto const abstract = [] {
    ITRACE(4, "abstract\n");
    return R {
      TypeStructureResolution { TBottom, false },
      TriBool::No,
      TriBool::Yes
    };
  };

  if (!is_specialized_cls(cls)) return conservative();

  // As in lookup_class_constant, we could handle this, but it's not
  // worth it.
  if (!is_specialized_string(name)) return conservative();
  auto const sname = sval_of(name);

  auto const process = [&] (const ClassInfo* ci) {
    ITRACE(4, "{}:\n", ci->cls->name);
    Trace::Indent _;

    // Does the type constant exist on this class?
    auto const it = ci->clsConstants.find(sname);
    if (it == ci->clsConstants.end()) return notFound();

    // Is it an actual non-abstract type-constant?
    auto const& cns = *it->second;
    if (cns.kind != ConstModifiers::Kind::Type) return notFound();
    if (!cns.val.has_value()) return abstract();

    assertx(tvIsDict(*cns.val));
    ITRACE(4, "({}) {}\n", cns.cls, show(dict_val(val(*cns.val).parr)));

    // If we've been given a resolver, use it. Otherwise resolve it in
    // the normal way.
    auto resolved = resolver
      ? resolver(cns, *ci->cls)
      : resolve_type_structure(*this, cns, *ci->cls);

    // The result of resolve_type_structure isn't, in general,
    // static. However a type-constant will always be, so force that
    // here.
    assertx(resolved.type.is(BBottom) || resolved.type.couldBe(BUnc));
    resolved.type &= TUnc;
    auto const r = R{
      std::move(resolved),
      TriBool::Yes,
      TriBool::No
    };
    ITRACE(4, "-> {}\n", show(r));
    return r;
  };

  auto const& dcls = dcls_of(cls);

  Optional<R> result;
  auto const resolved = visit_every_dcls_cls(
    dcls,
    [&] (const ClassInfo* cinfo) {
      if (result) {
        ITRACE(5, "-> {}\n", show(*result));
      }
      auto r = process(cinfo);
      if (!result) {
        result.emplace(std::move(r));
      } else {
        *result |= r;
      }
      return true;
    }
  );
  if (!resolved) return conservative();
  assertx(result.has_value());

  ITRACE(4, "-> {}\n", show(*result));
  return *result;
}

Type Index::lookup_constant(Context ctx, SString cnsName) const {
  auto iter = m_data->constants.find(cnsName);
  if (iter == end(m_data->constants)) {
    return TInitCell;
  }

  auto constant = iter->second;
  if (type(constant->val) != KindOfUninit) {
    return from_cell(constant->val);
  }

  auto const func_name = Constant::funcNameFromName(cnsName);
  assertx(func_name && "func_name will never be nullptr");

  auto rfunc = resolve_func(ctx, func_name);
  return lookup_return_type(ctx, nullptr, rfunc, Dep::ConstVal);
}

bool Index::func_depends_on_arg(const php::Func* func, int arg) const {
  auto const& finfo = *func_info(*m_data, func);
  return arg >= finfo.unusedParams.size() || !finfo.unusedParams.test(arg);
}

Type Index::lookup_foldable_return_type(Context ctx,
                                        const CallContext& calleeCtx) const {
  auto const func = calleeCtx.callee;
  constexpr auto max_interp_nexting_level = 2;
  static __thread uint32_t interp_nesting_level;
  static __thread Context base_ctx;

  auto const ctxType = adjust_closure_context(*this, calleeCtx);

  // Don't fold functions when staticness mismatches
  if (!func->isClosureBody) {
    if ((func->attrs & AttrStatic) && ctxType.couldBe(TObj)) return TInitCell;
    if (!(func->attrs & AttrStatic) && ctxType.couldBe(TCls)) return TInitCell;
  }

  auto const& finfo = *func_info(*m_data, func);
  if (finfo.effectFree && is_scalar(finfo.returnTy)) return finfo.returnTy;

  auto showArgs DEBUG_ONLY = [] (const CompactVector<Type>& a) {
    std::string ret, sep;
    for (auto& arg : a) {
      folly::format(&ret, "{}{}", sep, show(arg));
      sep = ",";
    };
    return ret;
  };

  {
    ContextRetTyMap::const_accessor acc;
    if (m_data->foldableReturnTypeMap.find(acc, calleeCtx)) {
      FTRACE_MOD(
        Trace::hhbbc, 4,
        "Found foldableReturnType for {}{}{} with args {} (hash: {})\n",
        func->cls ? func->cls->name : staticEmptyString(),
        func->cls ? "::" : "",
        func->name,
        showArgs(calleeCtx.args),
        CallContextHashCompare{}.hash(calleeCtx));

      assertx(is_scalar(acc->second));
      return acc->second;
    }
  }

  if (frozen()) {
    FTRACE_MOD(
      Trace::hhbbc, 4,
      "MISSING: foldableReturnType for {}{}{} with args {} (hash: {})\n",
      func->cls ? func->cls->name : staticEmptyString(),
      func->cls ? "::" : "",
      func->name,
      showArgs(calleeCtx.args),
      CallContextHashCompare{}.hash(calleeCtx));
    return TInitCell;
  }

  if (!interp_nesting_level) {
    base_ctx = ctx;
  } else if (interp_nesting_level > max_interp_nexting_level) {
    add_dependency(*m_data, func, base_ctx, Dep::InlineDepthLimit);
    return TInitCell;
  }

  auto const contextType = [&] {
    ++interp_nesting_level;
    SCOPE_EXIT { --interp_nesting_level; };

    auto const wf = php::WideFunc::cns(func);
    auto const fa = analyze_func_inline(
      *this,
      AnalysisContext { m_data->units.at(func->unit), wf, func->cls },
      ctxType,
      calleeCtx.args,
      CollectionOpts::EffectFreeOnly
    );
    return fa.effectFree ? fa.inferredReturn : TInitCell;
  }();

  if (!is_scalar(contextType)) return TInitCell;

  ContextRetTyMap::accessor acc;
  if (m_data->foldableReturnTypeMap.insert(acc, calleeCtx)) {
    acc->second = contextType;
  } else {
    // someone beat us to it
    assertx(acc->second == contextType);
  }
  return contextType;
}

Type Index::lookup_return_type(Context ctx,
                               MethodsInfo* methods,
                               const php::Func* f,
                               Dep dep) const {
  if (methods) {
    if (auto ret = methods->lookupReturnType(*f)) {
      return unctx(std::move(*ret));
    }
  }
  auto it = func_info(*m_data, f);
  if (it->func) {
    assertx(it->func == f);
    add_dependency(*m_data, f, ctx, dep);
    return it->returnTy;
  }
  return TInitCell;
}

Type Index::lookup_return_type(Context ctx,
                               MethodsInfo* methods,
                               res::Func rfunc,
                               Dep dep) const {
  auto const funcFamily = [&] (FuncFamily* fam, bool regularOnly) {
    add_dependency(*m_data, fam, ctx, dep);
    return fam->infoFor(regularOnly).m_returnTy.get(
      [&] {
        auto ret = TBottom;
        for (auto const pf : fam->possibleFuncs()) {
          if (regularOnly && !pf.inRegular()) continue;
          auto const finfo = func_info(*m_data, pf.ptr());
          if (!finfo->func) return TInitCell;
          ret |= unctx(finfo->returnTy);
          if (!ret.strictSubtypeOf(BInitCell)) return ret;
        }
        return ret;
      }
    );
  };
  auto const meth = [&] (const php::Func* func) {
    if (methods) {
      if (auto ret = methods->lookupReturnType(*func)) {
        return unctx(std::move(*ret));
      }
    }
    add_dependency(*m_data, func, ctx, dep);
    auto const finfo = func_info(*m_data, func);
    if (!finfo->func) return TInitCell;
    return unctx(finfo->returnTy);
  };

  return match<Type>(
    rfunc.val,
    [&] (res::Func::FuncName)   { return TInitCell; },
    [&] (res::Func::MethodName) { return TInitCell; },
    [&] (FuncInfo* finfo) {
      add_dependency(*m_data, finfo->func, ctx, dep);
      return unctx(finfo->returnTy);
    },
    [&] (res::Func::Method m)          { return meth(m.func); },
    [&] (res::Func::MethodFamily fam)  {
      return funcFamily(fam.family, fam.regularOnly);
    },
    [&] (res::Func::MethodOrMissing m) { return meth(m.func); },
    [&] (res::Func::Missing)           { return TBottom; },
    [&] (const res::Func::Isect& i) {
      auto ty = TInitCell;
      for (auto const ff : i.families) {
        ty &= funcFamily(ff, i.regularOnly);
      }
      return ty;
    }
  );
}

Type Index::lookup_return_type(Context caller,
                               MethodsInfo* methods,
                               const CompactVector<Type>& args,
                               const Type& context,
                               res::Func rfunc,
                               Dep dep) const {
  auto const funcFamily = [&] (FuncFamily* fam, bool regularOnly) {
    add_dependency(*m_data, fam, caller, dep);
    auto ret = fam->infoFor(regularOnly).m_returnTy.get(
      [&] {
        auto ty = TBottom;
        for (auto const pf : fam->possibleFuncs()) {
          if (regularOnly && !pf.inRegular()) continue;
          auto const finfo = func_info(*m_data, pf.ptr());
          if (!finfo->func) return TInitCell;
          ty |= finfo->returnTy;
          if (!ty.strictSubtypeOf(BInitCell)) return ty;
        }
        return ty;
      }
    );
    return return_with_context(std::move(ret), context);
  };
  auto const meth = [&] (const php::Func* func) {
    auto const finfo = func_info(*m_data, func);
    if (!finfo->func) return TInitCell;

    auto returnType = [&] {
      if (methods) {
        if (auto ret = methods->lookupReturnType(*func)) {
          return *ret;
        }
      }
      add_dependency(*m_data, func, caller, dep);
      return finfo->returnTy;
    }();

    return context_sensitive_return_type(
      *m_data,
      { finfo->func, args, context },
      std::move(returnType)
    );
  };

  return match<Type>(
    rfunc.val,
    [&] (res::Func::FuncName) {
      return lookup_return_type(caller, methods, rfunc, dep);
    },
    [&] (res::Func::MethodName) {
      return lookup_return_type(caller, methods, rfunc, dep);
    },
    [&] (FuncInfo* finfo) {
      add_dependency(*m_data, finfo->func, caller, dep);
      return context_sensitive_return_type(
        *m_data,
        { finfo->func, args, context },
        finfo->returnTy
      );
    },
    [&] (res::Func::Method m)          { return meth(m.func); },
    [&] (res::Func::MethodFamily fam)  {
      return funcFamily(fam.family, fam.regularOnly);
    },
    [&] (res::Func::MethodOrMissing m) { return meth(m.func); },
    [&] (res::Func::Missing)           { return TBottom; },
    [&] (const res::Func::Isect& i) {
      auto ty = TInitCell;
      for (auto const ff : i.families) {
        ty &= funcFamily(ff, i.regularOnly);
      }
      return ty;
    }
  );
}

CompactVector<Type>
Index::lookup_closure_use_vars(const php::Func* func,
                               bool move) const {
  assertx(func->isClosureBody);

  auto const numUseVars = closure_num_use_vars(func);
  if (!numUseVars) return {};
  auto const it = m_data->closureUseVars.find(func->cls);
  if (it == end(m_data->closureUseVars)) {
    return CompactVector<Type>(numUseVars, TCell);
  }
  if (move) return std::move(it->second);
  return it->second;
}

std::pair<Type, size_t> Index::lookup_return_type_raw(const php::Func* f) const {
  auto it = func_info(*m_data, f);
  if (it->func) {
    assertx(it->func == f);
    return { it->returnTy, it->returnRefinements };
  }
  return { TInitCell, 0 };
}

bool Index::lookup_this_available(const php::Func* f) const {
  return !(f->cls->attrs & AttrTrait) && !(f->attrs & AttrStatic);
}

Optional<uint32_t> Index::lookup_num_inout_params(
  Context,
  res::Func rfunc
) const {
  return match<Optional<uint32_t>>(
    rfunc.val,
    [&] (res::Func::FuncName s) -> Optional<uint32_t> {
      if (s.renamable) return std::nullopt;
      auto const it = m_data->funcs.find(s.name);
      return it != end(m_data->funcs)
       ? func_num_inout(it->second)
       : 0;
    },
    [&] (res::Func::MethodName s) -> Optional<uint32_t> {
      return std::nullopt;
    },
    [&] (FuncInfo* finfo) {
      return func_num_inout(finfo->func);
    },
    [&] (res::Func::Method m) {
      return func_num_inout(m.func);
    },
    [&] (res::Func::MethodFamily fam) -> Optional<uint32_t> {
      return fam.family->infoFor(fam.regularOnly).m_static->m_numInOut;
    },
    [&] (res::Func::MethodOrMissing m) {
      return func_num_inout(m.func);
    },
    [&] (res::Func::Missing) { return 0; },
    [&] (const res::Func::Isect& i) {
      Optional<uint32_t> numInOut;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (!info.m_numInOut) continue;
        assertx(IMPLIES(numInOut, *numInOut == *info.m_numInOut));
        if (!numInOut) numInOut = info.m_numInOut;
      }
      return numInOut;
    }
  );
}

PrepKind Index::lookup_param_prep(Context,
                                  res::Func rfunc,
                                  uint32_t paramId) const {
  auto const fromFuncFamily = [&] (FuncFamily* ff, bool regularOnly) {
    auto const& info = *ff->infoFor(regularOnly).m_static;
    if (paramId >= info.m_paramPreps.size()) {
      return PrepKind{TriBool::No, TriBool::No};
    }
    return info.m_paramPreps[paramId];
  };

  return match<PrepKind>(
    rfunc.val,
    [&] (res::Func::FuncName s) {
      if (s.renamable) return PrepKind{TriBool::Maybe, TriBool::Maybe};
      auto const it = m_data->funcs.find(s.name);
      return it != end(m_data->funcs)
        ? func_param_prep(it->second, paramId)
        : PrepKind{TriBool::No, TriBool::Yes};
    },
    [&] (res::Func::MethodName s) {
      return PrepKind{TriBool::Maybe, TriBool::Maybe};
    },
    [&] (FuncInfo* finfo) {
      return func_param_prep(finfo->func, paramId);
    },
    [&] (res::Func::Method m) {
      return func_param_prep(m.func, paramId);
    },
    [&] (res::Func::MethodFamily fam) {
      return fromFuncFamily(fam.family, fam.regularOnly);
    },
    [&] (res::Func::MethodOrMissing m) {
      return func_param_prep(m.func, paramId);
    },
    [&] (res::Func::Missing) {
      return PrepKind{TriBool::No, TriBool::Yes};
    },
    [&] (const res::Func::Isect& i) {
      auto inOut = TriBool::Maybe;
      auto readonly = TriBool::Maybe;

      for (auto const ff : i.families) {
        auto const prepKind = fromFuncFamily(ff, i.regularOnly);
        if (prepKind.inOut != TriBool::Maybe) {
          assertx(IMPLIES(inOut != TriBool::Maybe, inOut == prepKind.inOut));
          if (inOut == TriBool::Maybe) inOut = prepKind.inOut;
        }

        if (prepKind.readonly != TriBool::Maybe) {
          assertx(
            IMPLIES(readonly != TriBool::Maybe, readonly == prepKind.readonly)
          );
          if (readonly == TriBool::Maybe) readonly = prepKind.readonly;
        }
      }

      return PrepKind{inOut, readonly};
    }
  );
}

TriBool Index::lookup_return_readonly(
  Context,
  res::Func rfunc
) const {
  return match<TriBool>(
    rfunc.val,
    [&] (res::Func::FuncName s) {
      if (s.renamable) return TriBool::Maybe;
      auto const it = m_data->funcs.find(s.name);
      return it != end(m_data->funcs)
        ? yesOrNo(it->second->isReadonlyReturn)
        : TriBool::No; // if the function doesnt exist, we will error anyway
    },
    [&] (res::Func::MethodName s) { return TriBool::Maybe; },
    [&] (FuncInfo* finfo) {
      return yesOrNo(finfo->func->isReadonlyReturn);
    },
    [&] (res::Func::Method m) {
      return yesOrNo(m.func->isReadonlyReturn);
    },
    [&] (res::Func::MethodFamily fam) {
      return fam.family->infoFor(fam.regularOnly).m_static->m_isReadonlyReturn;
    },
    [&] (res::Func::MethodOrMissing m) {
      return yesOrNo(m.func->isReadonlyReturn);
    },
    [&] (res::Func::Missing) { return TriBool::No; },
    [&] (const res::Func::Isect& i) {
      auto readOnly = TriBool::Maybe;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (info.m_isReadonlyReturn == TriBool::Maybe) continue;
        assertx(IMPLIES(readOnly != TriBool::Maybe,
                        readOnly == info.m_isReadonlyReturn));
        if (readOnly == TriBool::Maybe) readOnly = info.m_isReadonlyReturn;
      }
      return readOnly;
    }
  );
}

TriBool Index::lookup_readonly_this(
  Context,
  res::Func rfunc
) const {
  return match<TriBool>(
    rfunc.val,
    [&] (res::Func::FuncName s) {
      if (s.renamable) return TriBool::Maybe;
      auto const it = m_data->funcs.find(s.name);
      return it != end(m_data->funcs)
        ? yesOrNo(it->second->isReadonlyThis)
        : TriBool::Yes; // if the function doesnt exist, we will error anyway
    },
    [&] (res::Func::MethodName s) { return TriBool::Maybe; },
    [&] (FuncInfo* finfo) {
      return yesOrNo(finfo->func->isReadonlyThis);
    },
    [&] (res::Func::Method m) {
      return yesOrNo(m.func->isReadonlyThis);
    },
    [&] (res::Func::MethodFamily fam) {
      return fam.family->infoFor(fam.regularOnly).m_static->m_isReadonlyThis;
    },
    [&] (res::Func::MethodOrMissing m) {
      return yesOrNo(m.func->isReadonlyThis);
    },
    [&] (res::Func::Missing) { return TriBool::No; },
    [&] (const res::Func::Isect& i) {
      auto readOnly = TriBool::Maybe;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (info.m_isReadonlyThis == TriBool::Maybe) continue;
        assertx(IMPLIES(readOnly != TriBool::Maybe,
                        readOnly == info.m_isReadonlyThis));
        if (readOnly == TriBool::Maybe) readOnly = info.m_isReadonlyThis;
      }
      return readOnly;
    }
  );
}

PropState
Index::lookup_private_props(const php::Class* cls,
                            bool move) const {
  auto it = m_data->privatePropInfo.find(cls);
  if (it != end(m_data->privatePropInfo)) {
    if (move) return std::move(it->second);
    return it->second;
  }
  return make_unknown_propstate(
    *this,
    cls,
    [&] (const php::Prop& prop) {
      return (prop.attrs & AttrPrivate) && !(prop.attrs & AttrStatic);
    }
  );
}

PropState
Index::lookup_private_statics(const php::Class* cls,
                              bool move) const {
  auto it = m_data->privateStaticPropInfo.find(cls);
  if (it != end(m_data->privateStaticPropInfo)) {
    if (move) return std::move(it->second);
    return it->second;
  }
  return make_unknown_propstate(
    *this,
    cls,
    [&] (const php::Prop& prop) {
      return (prop.attrs & AttrPrivate) && (prop.attrs & AttrStatic);
    }
  );
}

PropState Index::lookup_public_statics(const php::Class* cls) const {
  auto const cinfo = [&] () -> const ClassInfo* {
    auto const it = m_data->classInfo.find(cls->name);
    if (it == end(m_data->classInfo)) return nullptr;
    return it->second;
  }();

  PropState state;
  for (auto const& prop : cls->properties) {
    if (!(prop.attrs & (AttrPublic|AttrProtected)) ||
        !(prop.attrs & AttrStatic)) {
      continue;
    }

    auto [ty, everModified] = [&] {
      if (!cinfo) return std::make_pair(TInitCell, true);
      auto const it = cinfo->publicStaticProps.find(prop.name);
      assertx(it != end(cinfo->publicStaticProps));
      return std::make_pair(
        remove_uninit(it->second.inferredType),
        it->second.everModified
      );
    }();
    state.emplace(
      prop.name,
      PropStateElem<>{
        std::move(ty),
        &prop.typeConstraint,
        prop.attrs,
        everModified
      }
    );
  }
  return state;
}

/*
 * Entry point for static property lookups from the Index. Return
 * metadata about a `cls'::`name' static property access in the given
 * context.
 */
PropLookupResult<> Index::lookup_static(Context ctx,
                                        const PropertiesInfo& privateProps,
                                        const Type& cls,
                                        const Type& name) const {
  ITRACE(4, "lookup_static: {} {}::${}\n", show(ctx), show(cls), show(name));
  Trace::Indent _;

  using R = PropLookupResult<>;

  // First try to obtain the property name as a static string
  auto const sname = [&] () -> SString {
    // Treat non-string names conservatively, but the caller should be
    // checking this.
    if (!is_specialized_string(name)) return nullptr;
    return sval_of(name);
  }();

  // Conservative result when we can't do any better. The type can be
  // anything, and anything might throw.
  auto const conservative = [&] {
    ITRACE(4, "conservative\n");
    return R{
      TInitCell,
      sname,
      TriBool::Maybe,
      TriBool::Maybe,
      TriBool::Maybe,
      TriBool::Maybe,
      TriBool::Maybe,
      true
    };
  };

  // If we don't know what `cls' is, there's not much we can do.
  if (!is_specialized_cls(cls)) return conservative();

  // Turn the context class into a ClassInfo* for convenience.
  const ClassInfo* ctxCls = nullptr;
  if (ctx.cls) {
    // I don't think this can ever fail (we should always be able to
    // resolve the class since we're currently processing it). If it
    // does, be conservative.
    auto const rCtx = resolve_class(ctx.cls);
    if (rCtx.val.left()) return conservative();
    ctxCls = rCtx.val.right();
  }

  auto const& dcls = dcls_of(cls);
  auto const start = dcls.cls();

  Optional<R> result;
  auto const resolved = visit_every_dcls_cls(
    dcls,
    [&] (const ClassInfo* cinfo) {
      auto r = lookup_static_impl(
        *m_data,
        ctx,
        ctxCls,
        privateProps,
        cinfo,
        sname,
        dcls.isSub() && !sname && cinfo != start.val.right()
      );
      ITRACE(4, "{} -> {}\n", cinfo->cls->name, show(r));
      if (!result) {
        result.emplace(std::move(r));
      } else {
        *result |= r;
      }
      return true;
    }
  );
  if (!resolved) return conservative();
  assertx(result.has_value());

  ITRACE(4, "union -> {}\n", show(*result));
  return *result;
}

Type Index::lookup_public_prop(const Type& obj, const Type& name) const {
  if (!is_specialized_obj(obj)) return TCell;

  if (!is_specialized_string(name)) return TCell;
  auto const sname = sval_of(name);

  auto ty = TBottom;
  auto const resolved = visit_every_dcls_cls(
    dobj_of(obj),
    [&] (const ClassInfo* cinfo) {
      ty |= lookup_public_prop_impl(
        *m_data,
        cinfo,
        sname
      );
      return ty.strictSubtypeOf(TCell);
    }
  );
  if (!resolved) return TCell;
  return ty;
}

Type Index::lookup_public_prop(const php::Class* cls, SString name) const {
  auto const it = m_data->classInfo.find(cls->name);
  if (it == end(m_data->classInfo)) {
    return TCell;
  }
  return lookup_public_prop_impl(*m_data, it->second, name);
}

bool Index::lookup_class_init_might_raise(Context ctx, res::Class cls) const {
  return cls.val.match(
    []  (SString) { return true; },
    [&] (ClassInfo* cinfo) {
      return class_init_might_raise(*m_data, ctx, cinfo);
    }
  );
}

void Index::join_iface_vtable_thread() const {
  if (m_data->compute_iface_vtables.joinable()) {
    m_data->compute_iface_vtables.join();
  }
}

Slot
Index::lookup_iface_vtable_slot(const php::Class* cls) const {
  return folly::get_default(m_data->ifaceSlotMap, cls, kInvalidSlot);
}

//////////////////////////////////////////////////////////////////////

/*
 * Entry point for static property type mutation from the Index. Merge
 * `val' into the known type for any accessible `cls'::`name' static
 * property. The mutation will be recovered into either
 * `publicMutations' or `privateProps' depending on the properties
 * found. Mutations to AttrConst properties are ignored, unless
 * `ignoreConst' is true.
 */
PropMergeResult<> Index::merge_static_type(
    Context ctx,
    PublicSPropMutations& publicMutations,
    PropertiesInfo& privateProps,
    const Type& cls,
    const Type& name,
    const Type& val,
    bool checkUB,
    bool ignoreConst,
    bool mustBeReadOnly) const {
  ITRACE(
    4, "merge_static_type: {} {}::${} {}\n",
    show(ctx), show(cls), show(name), show(val)
  );
  Trace::Indent _;

  assertx(val.subtypeOf(BInitCell));

  using R = PropMergeResult<>;

  // In some cases we might try to merge Bottom if we're in
  // unreachable code. This won't affect anything, so just skip out
  // early.
  if (val.subtypeOf(BBottom)) return R{ TBottom, TriBool::No };

  // Try to turn the given property name into a static string
  auto const sname = [&] () -> SString {
    // Non-string names are treated conservatively here. The caller
    // should be checking for these and doing the right thing.
    if (!is_specialized_string(name)) return nullptr;
    return sval_of(name);
  }();

  // The case where we don't know `cls':
  auto const unknownCls = [&] {
    if (!sname) {
      // Very bad case. We don't know `cls' or the property name. This
      // mutation can be affecting anything, so merge it into all
      // properties (this drops type information for public
      // properties).
      ITRACE(4, "unknown class and prop. merging everything\n");
      publicMutations.mergeUnknown(ctx);
      privateProps.mergeInAllPrivateStatics(
        *this, unctx(val), ignoreConst, mustBeReadOnly
      );
    } else {
      // Otherwise we don't know `cls', but do know the property
      // name. We'll store this mutation separately and union it in to
      // any lookup with the same name.
      ITRACE(4, "unknown class. merging all props with name {}\n", sname);

      publicMutations.mergeUnknownClass(sname, unctx(val));

      // Assume that it could possibly affect any private property with
      // the same name.
      privateProps.mergeInPrivateStatic(
        *this, sname, unctx(val), ignoreConst, mustBeReadOnly
      );
    }

    // To be conservative, say we might throw and be conservative about
    // conversions.
    return PropMergeResult<>{
      loosen_likeness(val),
      TriBool::Maybe
    };
  };

  // check if we can determine the class.
  if (!is_specialized_cls(cls)) return unknownCls();

  const ClassInfo* ctxCls = nullptr;
  if (ctx.cls) {
    auto const rCtx = resolve_class(ctx.cls);
    // We should only be not able to resolve our own context if the
    // class is not instantiable. In that case, the merge can't
    // happen.
    if (rCtx.val.left()) return R{ TBottom, TriBool::No };
    ctxCls = rCtx.val.right();
  }

  auto const mergePublic = [&] (const ClassInfo* ci,
                                const php::Prop& prop,
                                const Type& val) {
    publicMutations.mergeKnown(ci, prop, val);
  };

  auto const& dcls = dcls_of(cls);
  Optional<res::Class> start;
  if (!dcls.isIsect()) start = dcls.cls();

  Optional<R> result;
  auto const resolved = visit_every_dcls_cls(
    dcls,
    [&] (const ClassInfo* cinfo) {
      auto r = merge_static_type_impl(
        *m_data,
        ctx,
        mergePublic,
        privateProps,
        ctxCls,
        cinfo,
        sname,
        val,
        checkUB,
        ignoreConst,
        mustBeReadOnly,
        dcls.isSub() && !sname && cinfo != start->val.right()
      );
      ITRACE(4, "{} -> {}\n", cinfo->cls->name, show(r));
      if (!result) {
        result.emplace(std::move(r));
      } else {
        *result |= r;
      }
      return true;
    }
  );
  if (!resolved) return unknownCls();
  assertx(result.has_value());
  ITRACE(4, "union -> {}\n", show(*result));
  return *result;
}

//////////////////////////////////////////////////////////////////////

DependencyContext Index::dependency_context(const Context& ctx) const {
  return dep_context(*m_data, ctx);
}

void Index::use_class_dependencies(bool f) {
  if (f != m_data->useClassDependencies) {
    m_data->dependencyMap.clear();
    m_data->useClassDependencies = f;
  }
}

void Index::init_public_static_prop_types() {
  trace_time tracer("init public static prop types", m_data->sample);

  for (auto const& cinfo : m_data->allClassInfos) {
    for (auto const& prop : cinfo->cls->properties) {
      if (!(prop.attrs & (AttrPublic|AttrProtected)) ||
          !(prop.attrs & AttrStatic)) {
        continue;
      }

      /*
       * If the initializer type is TUninit, it means an 86sinit provides the
       * actual initialization type or it is AttrLateInit.  So we don't want to
       * include the Uninit (which isn't really a user-visible type for the
       * property) or by the time we union things in we'll have inferred nothing
       * much.
       */
      auto const initial = [&] {
        auto const tyRaw = from_cell(prop.val);
        if (tyRaw.subtypeOf(BUninit)) return TBottom;
        if (prop.attrs & AttrSystemInitialValue) return tyRaw;
        return adjust_type_for_prop(
          *this, *cinfo->cls, &prop.typeConstraint, tyRaw
        );
      }();

      cinfo->publicStaticProps[prop.name] =
        PublicSPropEntry {
          union_of(
            adjust_type_for_prop(
              *this,
              *cinfo->cls,
              &prop.typeConstraint,
              TInitCell
            ),
            initial
          ),
          initial,
          &prop,
          0,
          false,
          true
      };
    }
  }
}

void Index::refine_class_constants(
    const Context& ctx,
    const CompactVector<std::pair<size_t, Type>>& resolved,
    DependencyContextSet& deps) {
  if (!resolved.size()) return;

  auto changed = false;
  auto& constants = ctx.func->cls->constants;

  for (auto const& c : resolved) {
    assertx(c.first < constants.size());
    auto& cnst = constants[c.first];
    assertx(cnst.kind == ConstModifiers::Kind::Value);

    auto const key = std::make_pair(ctx.func->cls, cnst.name);

    auto& types = m_data->clsConstTypes;

    always_assert(cnst.val && type(*cnst.val) == KindOfUninit);
    if (auto const val = tv(c.second)) {
      assertx(val->m_type != KindOfUninit);
      cnst.val = *val;
      // Deleting from the types map is too expensive, so just leave
      // any entry. We won't look at it if val is set.
      changed = true;
    } else {
      auto const old = [&] {
        auto const it = types.find(key);
        return (it == types.end()) ? ClsConstInfo{ TInitCell, 0 } : it->second;
      }();

      if (c.second.strictlyMoreRefined(old.type)) {
        if (old.refinements < options.returnTypeRefineLimit) {
          types.insert_or_assign(
            key,
            ClsConstInfo{ c.second, old.refinements+1 }
          );
          changed = true;
        } else {
          FTRACE(
            1, "maxed out refinements for class constant {}::{}\n",
            ctx.func->cls->name, cnst.name
          );
        }
      } else {
        always_assert_flog(
          c.second.moreRefined(old.type),
          "Class constant type invariant violated for {}::{}\n"
          "    {} is not at least as refined as {}\n",
          ctx.func->cls->name,
          cnst.name,
          show(c.second),
          show(old.type)
        );
      }
    }
  }

  if (changed) {
    find_deps(*m_data, ctx.func, Dep::ClsConst, deps);
  }
}

void Index::refine_constants(const FuncAnalysisResult& fa,
                             DependencyContextSet& deps) {
  auto const& func = fa.ctx.func;
  if (func->cls != nullptr) return;

  auto const val = tv(fa.inferredReturn);
  if (!val) return;

  auto const cns_name = Constant::nameFromFuncName(func->name);
  if (!cns_name) return;

  auto& cs = fa.ctx.unit->constants;
  auto it = std::find_if(
    cs.begin(),
    cs.end(),
    [&] (auto const& c) {
      return cns_name->same(c->name);
    });
  assertx(it != cs.end() && "Did not find constant");
  (*it)->val = val.value();
  find_deps(*m_data, func, Dep::ConstVal, deps);
}

void Index::fixup_return_type(const php::Func* func,
                              Type& retTy) const {
  if (func->isGenerator) {
    if (func->isAsync) {
      // Async generators always return AsyncGenerator object.
      retTy = objExact(builtin_class(s_AsyncGenerator.get()));
    } else {
      // Non-async generators always return Generator object.
      retTy = objExact(builtin_class(s_Generator.get()));
    }
  } else if (func->isAsync) {
    // Async functions always return WaitH<T>, where T is the type returned
    // internally.
    retTy = wait_handle(*this, std::move(retTy));
  }
}

void Index::init_return_type(const php::Func* func) {
  if ((func->attrs & AttrBuiltin) || func->isMemoizeWrapper) {
    return;
  }

  auto make_type = [&] (const TypeConstraint& tc) {
    if (tc.isSoft() ||
        (RuntimeOption::EvalEnforceGenericsUB < 2 && tc.isUpperBound())) {
      return TBottom;
    }
    auto const cls = func->cls && func->cls->closureContextCls
       ? m_data->classes.at(func->cls->closureContextCls)
       : func->cls;
    return lookup_constraint(
      Context { m_data->units.at(func->unit), func, cls },
      tc
    );
  };

  auto const finfo = create_func_info(*m_data, func);

  auto tcT = make_type(func->retTypeConstraint);
  if (tcT.is(BBottom)) return;

  if (func->hasInOutArgs) {
    std::vector<Type> types;
    types.emplace_back(intersection_of(TInitCell, std::move(tcT)));
    for (auto& p : func->params) {
      if (!p.inout) continue;
      auto t = make_type(p.typeConstraint);
      if (t.is(BBottom)) return;
      types.emplace_back(intersection_of(TInitCell, std::move(t)));
    }
    tcT = vec(std::move(types));
  }

  tcT = loosen_all(to_cell(std::move(tcT)));

  FTRACE(4, "Pre-fixup return type for {}{}{}: {}\n",
         func->cls ? func->cls->name->data() : "",
         func->cls ? "::" : "",
         func->name, show(tcT));
  fixup_return_type(func, tcT);
  FTRACE(3, "Initial return type for {}{}{}: {}\n",
         func->cls ? func->cls->name->data() : "",
         func->cls ? "::" : "",
         func->name, show(tcT));
  finfo->returnTy = std::move(tcT);
}

void Index::refine_return_info(const FuncAnalysisResult& fa,
                               DependencyContextSet& deps) {
  auto const& func = fa.ctx.func;
  auto const finfo = create_func_info(*m_data, func);

  auto const error_loc = [&] {
    return folly::sformat(
      "{} {}{}",
      func->unit,
      func->cls
        ? folly::to<std::string>(func->cls->name->data(), "::")
        : std::string{},
      func->name
    );
  };

  auto dep = Dep{};
  if (finfo->retParam == NoLocalId && fa.retParam != NoLocalId) {
    // This is just a heuristic; it doesn't mean that the value passed
    // in was returned, but that the value of the parameter at the
    // point of the RetC was returned. We use it to make (heuristic)
    // decisions about whether to do inline interps, so we only allow
    // it to change once (otherwise later passes might not do the
    // inline interp, and get worse results, which could trigger other
    // assertions in Index::refine_*).
    dep = Dep::ReturnTy;
    finfo->retParam = fa.retParam;
  }

  auto unusedParams = ~fa.usedParams;
  if (finfo->unusedParams != unusedParams) {
    dep = Dep::ReturnTy;
    always_assert_flog(
        (finfo->unusedParams | unusedParams) == unusedParams,
        "Index unusedParams decreased in {}.\n",
        error_loc()
    );
    finfo->unusedParams = unusedParams;
  }

  auto resetFuncFamilies = false;
  if (fa.inferredReturn.strictlyMoreRefined(finfo->returnTy)) {
    if (finfo->returnRefinements < options.returnTypeRefineLimit) {
      finfo->returnTy = fa.inferredReturn;
      // We've modifed the return type, so reset any cached FuncFamily
      // return types.
      resetFuncFamilies = true;
      dep = is_scalar(fa.inferredReturn) ?
        Dep::ReturnTy | Dep::InlineDepthLimit : Dep::ReturnTy;
      finfo->returnRefinements += fa.localReturnRefinements + 1;
      if (finfo->returnRefinements > options.returnTypeRefineLimit) {
        FTRACE(1, "maxed out return type refinements at {}\n", error_loc());
      }
    } else {
      FTRACE(1, "maxed out return type refinements at {}\n", error_loc());
    }
  } else {
    always_assert_flog(
      fa.inferredReturn.moreRefined(finfo->returnTy),
      "Index return type invariant violated in {}.\n"
      "   {} is not at least as refined as {}\n",
      error_loc(),
      show(fa.inferredReturn),
      show(finfo->returnTy)
    );
  }

  always_assert_flog(
    !finfo->effectFree || fa.effectFree,
    "Index effectFree changed from true to false in {} {}{}.\n",
    func->unit,
    func->cls
      ? folly::to<std::string>(func->cls->name->data(), "::")
      : std::string{},
    func->name);

  if (finfo->effectFree != fa.effectFree) {
    finfo->effectFree = fa.effectFree;
    dep = Dep::InlineDepthLimit | Dep::ReturnTy;
  }

  if (dep != Dep{}) {
    find_deps(*m_data, func, dep, deps);
    if (resetFuncFamilies) {
      assertx(has_dep(dep, Dep::ReturnTy));
      auto const regular = finfo->regularClassMethod.load();
      for (auto const ff : finfo->families) {
        // Reset the cached return type information for all the
        // FuncFamilies this function is a part of. Always reset the
        // "all" information, and if this function is on a regular
        // class and there's regular subset information, reset that
        // too.
        if (!ff->m_all.m_returnTy.reset() &&
            (!regular || !ff->m_regular ||
             !ff->m_regular->m_returnTy.reset())) {
          continue;
        }
        // Only load the deps for this func family if we're the ones
        // who successfully reset. Only one thread needs to do it.
        find_deps(*m_data, ff, Dep::ReturnTy, deps);
      }
    }
  }
}

bool Index::refine_closure_use_vars(const php::Class* cls,
                                    const CompactVector<Type>& vars) {
  assertx(is_closure(*cls));

  for (auto i = uint32_t{0}; i < vars.size(); ++i) {
    always_assert_flog(
      vars[i].equivalentlyRefined(unctx(vars[i])),
      "Closure cannot have a used var with a context dependent type"
    );
  }

  auto& current = [&] () -> CompactVector<Type>& {
    std::lock_guard<std::mutex> _{closure_use_vars_mutex};
    return m_data->closureUseVars[cls];
  }();

  always_assert(current.empty() || current.size() == vars.size());
  if (current.empty()) {
    current = vars;
    return true;
  }

  auto changed = false;
  for (auto i = uint32_t{0}; i < vars.size(); ++i) {
    if (vars[i].strictSubtypeOf(current[i])) {
      changed = true;
      current[i] = vars[i];
    } else {
      always_assert_flog(
        vars[i].moreRefined(current[i]),
        "Index closure_use_var invariant violated in {}.\n"
        "   {} is not at least as refined as {}\n",
        cls->name,
        show(vars[i]),
        show(current[i])
      );
    }
  }

  return changed;
}

template<class Container>
void refine_private_propstate(Container& cont,
                              const php::Class* cls,
                              const PropState& state) {
  assertx(!is_used_trait(*cls));
  auto* elm = [&] () -> typename Container::value_type* {
    std::lock_guard<std::mutex> _{private_propstate_mutex};
    auto it = cont.find(cls);
    if (it == end(cont)) {
      if (!state.empty()) cont[cls] = state;
      return nullptr;
    }
    return &*it;
  }();

  if (!elm) return;

  for (auto& kv : state) {
    auto& target = elm->second[kv.first];
    assertx(target.tc == kv.second.tc);
    always_assert_flog(
      kv.second.ty.moreRefined(target.ty),
      "PropState refinement failed on {}::${} -- {} was not a subtype of {}\n",
      cls->name->data(),
      kv.first->data(),
      show(kv.second.ty),
      show(target.ty)
    );
    target.ty = kv.second.ty;

    if (kv.second.everModified) {
      always_assert_flog(
        target.everModified,
        "PropState refinement failed on {}::${} -- "
        "everModified flag went from false to true\n",
        cls->name->data(),
        kv.first->data()
      );
    } else {
      target.everModified = false;
    }
  }
}

void Index::refine_private_props(const php::Class* cls,
                                 const PropState& state) {
  refine_private_propstate(m_data->privatePropInfo, cls, state);
}

void Index::refine_private_statics(const php::Class* cls,
                                   const PropState& state) {
  // We can't store context dependent types in private statics since they
  // could be accessed using different contexts.
  auto cleanedState = PropState{};
  for (auto const& prop : state) {
    auto& elem = cleanedState[prop.first];
    elem.ty = unctx(prop.second.ty);
    elem.tc = prop.second.tc;
    elem.attrs = prop.second.attrs;
    elem.everModified = prop.second.everModified;
  }

  refine_private_propstate(m_data->privateStaticPropInfo, cls, cleanedState);
}

void Index::record_public_static_mutations(const php::Func& func,
                                           PublicSPropMutations mutations) {
  if (!mutations.m_data) {
    m_data->publicSPropMutations.erase(&func);
    return;
  }
  m_data->publicSPropMutations.insert_or_assign(&func, std::move(mutations));
}

void Index::update_static_prop_init_val(const php::Class* cls,
                                        SString name) const {
  auto const cls_it = m_data->classInfo.find(cls->name);
  if (cls_it == end(m_data->classInfo)) {
    return;
  }
  auto const cinfo = cls_it->second;
  if (cinfo->cls != cls) {
    return;
  }
  auto const it = cinfo->publicStaticProps.find(name);
  if (it != cinfo->publicStaticProps.end()) {
    it->second.initialValueResolved = true;
  }
}

void Index::refine_public_statics(DependencyContextSet& deps) {
  trace_time update("update public statics");

  // Union together the mutations for each function, including the functions
  // which weren't analyzed this round.
  auto nothing_known = false;
  PublicSPropMutations::UnknownMap unknown;
  PublicSPropMutations::KnownMap known;
  for (auto const& mutations : m_data->publicSPropMutations) {
    if (!mutations.second.m_data) continue;
    if (mutations.second.m_data->m_nothing_known) {
      nothing_known = true;
      break;
    }

    for (auto const& kv : mutations.second.m_data->m_unknown) {
      auto const ret = unknown.insert(kv);
      if (!ret.second) ret.first->second |= kv.second;
    }
    for (auto const& kv : mutations.second.m_data->m_known) {
      auto const ret = known.insert(kv);
      if (!ret.second) ret.first->second |= kv.second;
    }
  }

  if (nothing_known) {
    // We cannot go from knowing the types to not knowing the types (this is
    // equivalent to widening the types).
    always_assert(!m_data->seenPublicSPropMutations);
    return;
  }
  m_data->seenPublicSPropMutations = true;

  // Refine known class state
  for (auto const& cinfo : m_data->allClassInfos) {
    for (auto& kv : cinfo->publicStaticProps) {
      auto knownClsType = [&] {
        auto const it = known.find(
          PublicSPropMutations::KnownKey { cinfo.get(), kv.first }
        );
        // If we didn't see a mutation, the type is TBottom.
        return it == end(known) ? TBottom : it->second;
      }();

      auto unknownClsType = [&] {
        auto const it = unknown.find(kv.first);
        // If we didn't see a mutation, the type is TBottom.
        return it == end(unknown) ? TBottom : it->second;
      }();

      // We can't keep context dependent types in public properties.
      auto newType = adjust_type_for_prop(
        *this,
        *cinfo->cls,
        &kv.second.prop->typeConstraint,
        unctx(union_of(std::move(knownClsType), std::move(unknownClsType)))
      );

      if (!newType.is(BBottom)) {
        always_assert_flog(
          kv.second.everModified,
          "Static property index invariant violated on {}::{}:\n"
          " everModified flag went from false to true",
          cinfo->cls->name->data(),
          kv.first->data()
        );
      } else {
        kv.second.everModified = false;
      }

      if (kv.second.initialValueResolved) {
        for (auto& prop : cinfo->cls->properties) {
          if (prop.name != kv.first) continue;
          kv.second.initializerType = from_cell(prop.val);
          kv.second.initialValueResolved = false;
          break;
        }
        assertx(!kv.second.initialValueResolved);
      }

      // The type from the indexer doesn't contain the in-class initializer
      // types. Add that here.
      auto effectiveType =
        union_of(std::move(newType), kv.second.initializerType);

      /*
       * We may only shrink the types we recorded for each property. (If a
       * property type ever grows, the interpreter could infer something
       * incorrect at some step.)
       */
      always_assert_flog(
        effectiveType.subtypeOf(kv.second.inferredType),
        "Static property index invariant violated on {}::{}:\n"
        "  {} is not a subtype of {}",
        cinfo->cls->name->data(),
        kv.first->data(),
        show(effectiveType),
        show(kv.second.inferredType)
      );

      // Put a limit on the refinements to ensure termination. Since we only
      // ever refine types, we can stop at any point and still maintain
      // correctness.
      if (effectiveType.strictSubtypeOf(kv.second.inferredType)) {
        if (kv.second.refinements + 1 < options.publicSPropRefineLimit) {
          find_deps(*m_data, kv.second.prop, Dep::PublicSProp, deps);
          kv.second.inferredType = std::move(effectiveType);
          ++kv.second.refinements;
        } else {
          FTRACE(
            1, "maxed out public static property refinements for {}:{}\n",
            cinfo->cls->name->data(),
            kv.first->data()
          );
        }
      }
    }
  }
}

void Index::refine_bad_initial_prop_values(const php::Class* cls,
                                           bool value,
                                           DependencyContextSet& deps) {
  assertx(!is_used_trait(*cls));
  auto const it = m_data->classInfo.find(cls->name);
  if (it == end(m_data->classInfo)) {
    return;
  }
  auto const cinfo = it->second;
  if (cinfo->cls != cls) {
    return;
  }
  always_assert_flog(
    cinfo->hasBadInitialPropValues || !value,
    "Bad initial prop values going from false to true on {}",
    cls->name->data()
  );

  if (cinfo->hasBadInitialPropValues && !value) {
    cinfo->hasBadInitialPropValues = false;
    find_deps(*m_data, cls, Dep::PropBadInitialValues, deps);
  }
}

bool Index::frozen() const {
  return m_data->frozen;
}

void Index::freeze() {
  m_data->frozen = true;
  m_data->ever_frozen = true;
}

/*
 * Note that these functions run in separate threads, and
 * intentionally don't bump Trace::hhbbc_time. If you want to see
 * these times, set TRACE=hhbbc_time:1
 */
#define CLEAR(x)                                \
  {                                             \
    trace_time _{"clearing " #x};               \
    _.ignore_client_stats();                    \
    (x).clear();                                \
  }

void Index::cleanup_pre_analysis() {
  trace_time _{"cleanup pre analysis", m_data->sample};
  CLEAR(m_data->methods);
}

void Index::cleanup_for_final() {
  trace_time _{"cleanup for final", m_data->sample};
  CLEAR(m_data->dependencyMap);
}

void Index::cleanup_post_emit() {
  trace_time _{"cleanup post emit", m_data->sample};

  std::vector<std::function<void()>> clearers;
  #define CLEAR_PARALLEL(x) clearers.push_back([&] CLEAR(x));
  CLEAR_PARALLEL(m_data->classes);
  CLEAR_PARALLEL(m_data->funcs);
  CLEAR_PARALLEL(m_data->typeAliases);
  CLEAR_PARALLEL(m_data->enums);
  CLEAR_PARALLEL(m_data->constants);
  CLEAR_PARALLEL(m_data->modules);
  CLEAR_PARALLEL(m_data->units);

  CLEAR_PARALLEL(m_data->classClosureMap);
  CLEAR_PARALLEL(m_data->classExtraMethodMap);

  CLEAR_PARALLEL(m_data->classInfo);

  CLEAR_PARALLEL(m_data->privatePropInfo);
  CLEAR_PARALLEL(m_data->privateStaticPropInfo);
  CLEAR_PARALLEL(m_data->publicSPropMutations);
  CLEAR_PARALLEL(m_data->ifaceSlotMap);
  CLEAR_PARALLEL(m_data->closureUseVars);

  CLEAR_PARALLEL(m_data->methodFamilies);

  CLEAR_PARALLEL(m_data->funcFamilies);
  CLEAR_PARALLEL(m_data->funcFamilyStaticInfos);

  CLEAR_PARALLEL(m_data->clsConstTypes);
  CLEAR_PARALLEL(m_data->clsConstLookupCache);

  CLEAR_PARALLEL(m_data->foldableReturnTypeMap);
  CLEAR_PARALLEL(m_data->contextualReturnTypes);

  parallel::for_each(clearers, [] (const std::function<void()>& f) { f(); });

  {
    trace_time t{"reset funcInfo"};
    t.ignore_client_stats();
    parallel::for_each(
      m_data->funcInfo,
      [] (auto& u) {
        u.returnTy = TBottom;
        u.families.clear();
      }
    );
    m_data->funcInfo.clear();
  }

  // Class-infos and program need to be freed after all Type instances
  // are destroyed, as Type::checkInvariants may try to access them.

  {
    trace_time t{"reset allClassInfos"};
    t.ignore_client_stats();
    parallel::for_each(m_data->allClassInfos, [] (auto& u) { u.reset(); });
    m_data->allClassInfos.clear();
  }

  {
    trace_time t{"reset program"};
    t.ignore_client_stats();
    parallel::for_each(m_data->program->units, [] (auto& u) { u.reset(); });
    parallel::for_each(m_data->program->classes, [] (auto& u) { u.reset(); });
    parallel::for_each(m_data->program->funcs, [] (auto& f) { f.reset(); });
    m_data->program.reset();
  }
}

void Index::thaw() {
  m_data->frozen = false;
}

//////////////////////////////////////////////////////////////////////

res::Func Index::do_resolve(const php::Func* f) const {
  auto const finfo = create_func_info(*m_data, f);
  return res::Func { finfo };
};

// Return true if we know for sure that one php::Class must derive
// from another at runtime, in all possible instantiations.
bool Index::must_be_derived_from(const php::Class* cls,
                                 const php::Class* parent) const {
  if (cls == parent) return true;
  auto const clsIt = m_data->classInfo.find(cls->name);
  auto const parentIt = m_data->classInfo.find(parent->name);
  if (clsIt == end(m_data->classInfo) || parentIt == end(m_data->classInfo)) {
    return false;
  }
  return clsIt->second->derivedFrom(*parentIt->second);
}

//////////////////////////////////////////////////////////////////////

PublicSPropMutations::Data& PublicSPropMutations::get() {
  if (!m_data) m_data = std::make_unique<Data>();
  return *m_data;
}

void PublicSPropMutations::mergeKnown(const ClassInfo* ci,
                                      const php::Prop& prop,
                                      const Type& val) {
  ITRACE(4, "PublicSPropMutations::mergeKnown: {} {} {}\n",
         ci->cls->name->data(), prop.name, show(val));

  auto const res = get().m_known.emplace(
    KnownKey { const_cast<ClassInfo*>(ci), prop.name }, val
  );
  if (!res.second) res.first->second |= val;
}

void PublicSPropMutations::mergeUnknownClass(SString prop, const Type& val) {
  ITRACE(4, "PublicSPropMutations::mergeUnknownClass: {} {}\n",
         prop, show(val));

  auto const res = get().m_unknown.emplace(prop, val);
  if (!res.second) res.first->second |= val;
}

void PublicSPropMutations::mergeUnknown(Context ctx) {
  ITRACE(4, "PublicSPropMutations::mergeUnknown\n");

  /*
   * We have a case here where we know neither the class nor the static
   * property name.  This means we have to pessimize public static property
   * types for the entire program.
   *
   * We could limit it to pessimizing them by merging the `val' type, but
   * instead we just throw everything away---this optimization is not
   * expected to be particularly useful on programs that contain any
   * instances of this situation.
   */
  std::fprintf(
    stderr,
    "NOTE: had to mark everything unknown for public static "
    "property types due to dynamic code.  -fanalyze-public-statics "
    "will not help for this program.\n"
    "NOTE: The offending code occured in this context: %s\n",
    show(ctx).c_str()
  );
  get().m_nothing_known = true;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

template<>
struct BlobEncoderHelper<std::unique_ptr<HHBBC::ClassInfo2>> {
  template <typename SerDe>
  static void serde(SerDe& sd, std::unique_ptr<HHBBC::ClassInfo2>& p) {
    if constexpr (SerDe::deserializing) {
      p = std::make_unique<HHBBC::ClassInfo2>();
    } else {
      assertx(p);
    }
    sd(*p);
  }
};

//////////////////////////////////////////////////////////////////////

}
