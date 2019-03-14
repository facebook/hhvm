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
#ifndef incl_HHBBC_INTERP_STATE_H_
#define incl_HHBBC_INTERP_STATE_H_

#include <vector>
#include <string>
#include <map>

#include <boost/variant.hpp>

#include <folly/Optional.h>

#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/bc.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

struct ClassAnalysis;
struct FuncAnalysis;

//////////////////////////////////////////////////////////////////////

/*
 * Types of a FPI regions.  (What sort of function call is being
 * made.)
 */
enum class FPIKind {
  Unknown,     // Nothing is known.
  CallableArr, // May be an ObjMeth or a ClsMeth.
  Func,        // Definitely a non-member function.
  Ctor,        // Definitely a constructor for an object.
  ObjMeth,     // Definitely a method on an object (possibly __call).
  ObjMethNS,   // ObjMeth, but allows obj to be null.
  ClsMeth,     // Definitely a static method on a class (possibly__callStatic).
  ObjInvoke,   // Closure invoke or __invoke on an object.
  Builtin,     // Resolved builtin call; we will convert params and FCall as
               // we go
};

/*
 * Information about a pre-live ActRec.  Part of state tracked in
 * State.
 */
struct ActRec {
  explicit ActRec(FPIKind kind,
                  Type calledOn,
                  folly::Optional<res::Class> c = folly::none,
                  folly::Optional<res::Func> f = folly::none,
                  folly::Optional<res::Func> f2 = folly::none)
    : kind(kind)
    , cls(std::move(c))
    , func(std::move(f))
    , fallbackFunc(std::move(f2))
    , context(std::move(calledOn))
  {}

  FPIKind kind;
  bool foldable{false};
  BlockId pushBlk{NoBlockId};
  folly::Optional<res::Class> cls;
  folly::Optional<res::Func> func;
  // Possible fallback func if we cannot determine which will be called.
  folly::Optional<res::Func> fallbackFunc;
  // isCtx of context is whether it matches caller's context
  Type context;
};

/*
 * State of an iterator in the program.
 *
 * We track iterator liveness precisely, so if an iterator is DeadIter, its
 * definitely dead and vice-versa. We only track "normal" iterators (non-weak,
 * non-mutable), so iterators not of those type are considered "dead".
 */
struct DeadIter {};
struct LiveIter {
  IterTypes types;
  // The local that an iterator was initialized with (and which has not been
  // changed since).
  LocalId baseLocal = NoLocalId;
  // The local that is known to be equivalent to the current key in the
  // iterator. Used to detect "safe" writes.
  LocalId keyLocal  = NoLocalId;
  // The block id where this iterator was initialized. If there's more than one
  // such block, NoBlockId.
  BlockId initBlock = NoBlockId;
};
using Iter = boost::variant<DeadIter, LiveIter>;

/*
 * Tag indicating what sort of thing contains the current member base.
 *
 * The base is always the unboxed version of the type, and its location could be
 * inside of a Ref. So, for example, a base with BaseLoc::Frame could be located
 * inside of a Ref that is pointed to by the Frame. (We may want to distinguish
 * these two cases at some point if we start trying to track real information
 * about Refs, but not yet.)
 *
 * Note that if we're in an array-chain, the base location always reflects the
 * location of the array which started the array-chain.
 */
enum class BaseLoc {
  None,

  /*
   * Base is in a number of possible places after an Elem op.  It
   * cannot possibly be in an object property (although it certainly
   * may alias one). See miElem for details.
   *
   * This is only used if its unclear if its actually in an array. If it is
   * definitely in an array, then arrayChain will be non-empty and the base
   * location will reflect where the array is located.
   */
  Elem,

  /*
   * Base is in possible locations after a Prop op.  This means it
   * possibly lives in a property on an object, but possibly not
   * (e.g. it could be a null in tvScratch).  See miProp for
   * details.
   *
   * If it is definitely known to be a property in an object, the
   * locTy in the Base will be a subtype of TObj.
   */
  Prop,

  /*
   * Known to be a static property on an object. This is only
   * possible as an initial base.
   */
  StaticProp,

  /*
   * Known to be contained in the current frame as a local, as the
   * frame $this, by the evaluation stack, or inside $GLOBALS.  Only
   * possible as initial bases.
   */
  Local,
  This,
  Stack,
  Global,
};

/*
 * Information about the current member base's type and location.
 */
struct Base {
  explicit Base(Type type = {},
                BaseLoc loc = BaseLoc::None,
                Type locTy = {},
                SString locName = {},
                LocalId locLocal = NoLocalId,
                uint32_t locSlot = 0)
    : type{std::move(type)}
    , loc{loc}
    , locTy{std::move(locTy)}
    , locName{locName}
    , locLocal{locLocal}
    , locSlot{locSlot} {}

  Type type;
  BaseLoc loc;

  /*
   * We also need to track effects of intermediate dims on the type of the base.
   * So we have a type, name, and possibly associated local or stack slot for
   * the base's container.
   *
   * For StaticProp, locName is the name of the property if known, or nullptr,
   * and locTy is the type of the class containing the static property.
   *
   * For Prop, locName is the name of the property if it was known, and locTy
   * gives as much information about the object type it is in.  (If we actually
   * *know* it is in an object, locTy will be a subtype of TObj.)
   *
   * For Local, locName is the name of the local if known, or nullptr, and
   * locLocal is the LocalId corresponding to the local (or NoLocalId if not
   * known).
   *
   * For Stack, locSlot is the stack index of the corresponding stack slot.
   */
  Type locTy;
  SString locName;
  LocalId locLocal;
  uint32_t locSlot;
};

// An element on the eval stack
struct StackElem {
  Type type;
  // A location which is known to have an equivalent value to this
  // stack value. This could be a valid LocalId, the special value
  // StackDupId to indicate that its equivalent to the stack element
  // below it, the special value StackThisId to indicate that its the
  // value of $this, or NoLocalId if it has no known equivalents.
  // Note that the location may not match the stack value wrt Uninit.
  LocalId equivLoc;

  bool operator==(const StackElem& other) const {
    return type == other.type && equivLoc == other.equivLoc;
  }
};

/*
 * Used to track the state of the binding between locals, and their
 * corresponding static (if any).
 */
enum class LocalStaticBinding {
  // This local is not bound to a local static
  None,
  // This local might be bound to its local static
  Maybe,
  // This local is known to be bound to its local static
  Bound
};

/*
 * A program state at a position in a php::Block.
 *
 * The `initialized' flag indicates whether the state knows anything.  All
 * other fields are invalid if a state is not initialized, and notably, after
 * all analysis has run, any blocks that still don't have initialized input
 * states are not reachable.
 *
 * The `unreachable' flag means we've produced this state from analysis, but
 * the program cannot reach this program position.  This flag has two uses:
 *
 *    o It allows us to determine arbitrary mid-block positions where code
 *      becomes unreachable, and eliminate that code in optimize.cpp.
 *
 *    o HHBC invariants can complicate removing unreachable code in FPI
 *      regions---see the rules in bytecode.specification.  Inside FPI regions,
 *      we still do abstract interpretation of the unreachable code, but this
 *      flag is used when merging states to allow the interpreter to analyze
 *      blocks that are unreachable without pessimizing states for reachable
 *      blocks that would've been their successors.
 *
 * One other note: having the interpreter visit blocks when they are
 * unreachable still potentially merges types into object properties that
 * aren't possible at runtime.  We're only doing this to handle FPI regions for
 * now, but it's not ideal.
 *
 */
struct State {
  State() {
    initialized = unreachable = false;
    speculatedIsUnconditional = false;
    speculatedIsFallThrough = false;
  };
  State(const State&) = default;
  State(State&&) = default;
  State& operator=(const State&) = default;
  State& operator=(State&&) = default;

  uint8_t initialized : 1;
  uint8_t unreachable : 1;
  // if set, speculated is where we end up when we fall through
  uint8_t speculatedIsFallThrough : 1;
  // if set, speculated is taken unconditionally
  uint8_t speculatedIsUnconditional : 1;
  // when speculated is set, the number of extra pops to be inserted
  uint8_t speculatedPops{};
  uint32_t speculated = NoBlockId;
  LocalId thisLoc = NoLocalId;
  Type thisType;
  CompactVector<Type> locals;
  CompactVector<Iter> iters;
  CompactVector<Type> clsRefSlots;
  CompactVector<StackElem> stack;
  CompactVector<ActRec> fpiStack;

  struct MInstrState {
    /*
     * The current member base. Updated as we move through bytecodes
     * representing the operation.
     */
    Base base{};

    /*
     * Chains of member operations on array elements will affect the type of
     * something further back in the member instruction. This vector tracks the
     * base,key type pair that was used at each stage. See
     * interp-minstr.cpp:resolveArrayChain().
     */
    struct ArrayChainEnt {
      Type base;
      Type key;
      LocalId keyLoc;
    };
    using ArrayChain = CompactVector<ArrayChainEnt>;
    ArrayChain arrayChain;
  };
  MInstrState mInstrState;

  /*
   * Mapping of a local to other locals which are known to have
   * equivalent values. This equivalence ignores Uninit; users should
   * compare types if they care.
   */
  CompactVector<LocalId> equivLocals;

  /*
   * LocalStaticBindings. Only allocated on demand.
   */
  CompactVector<LocalStaticBinding> localStaticBindings;
};

/*
 * States are EqualityComparable (provided they are in-states for the
 * same block).
 */
bool operator==(const ActRec&, const ActRec&);
bool operator!=(const ActRec&, const ActRec&);

/*
 * Return a copy of a State without copying either the evaluation
 * stack or FPI stack.
 */
State without_stacks(const State&);

//////////////////////////////////////////////////////////////////////

/*
 * PropertiesInfo packages the PropState for private instance and
 * static properties, which is cross-block information collected in
 * CollectedInfo.
 *
 * During analysis the ClassAnalysis* is available and the PropState is
 * retrieved from there. However during optimization the ClassAnalysis is
 * not available and the PropState has to be retrieved off the Class in
 * the Index. In that case cls is nullptr and the PropState fields are
 * populated.
 */
struct PropertiesInfo {
  PropertiesInfo(const Index&, Context, ClassAnalysis*);

  PropState& privateProperties();
  PropState& privateStatics();
  const PropState& privateProperties() const;
  const PropState& privateStatics() const;

  void setBadPropInitialValues();

private:
  ClassAnalysis* const m_cls;
  PropState m_privateProperties;
  PropState m_privateStatics;
};

//////////////////////////////////////////////////////////////////////

/*
 * Map from closure classes to types for each of their used vars.
 * Shows up in a few different interpreter structures.
 */
using ClosureUseVarMap = hphp_fast_map<
  php::Class*,
  CompactVector<Type>
>;

/*
 * Merge the types in the vector as possible use vars for the closure
 * `clo' into the destination map.
 */
void merge_closure_use_vars_into(ClosureUseVarMap& dst,
                                 php::Class* clo,
                                 CompactVector<Type>);

//////////////////////////////////////////////////////////////////////

enum class CollectionOpts {
  TrackConstantArrays = 1,
  Inlining = 2,
  EffectFreeOnly = 4,
  Optimizing = 8,
  Speculating = 16,
};

inline CollectionOpts operator|(CollectionOpts o1, CollectionOpts o2) {
  return static_cast<CollectionOpts>(
    static_cast<int>(o1) | static_cast<int>(o2)
  );
}

inline CollectionOpts operator&(CollectionOpts o1, CollectionOpts o2) {
  return static_cast<CollectionOpts>(
    static_cast<int>(o1) & static_cast<int>(o2)
  );
}

inline CollectionOpts operator-(CollectionOpts o1, CollectionOpts o2) {
  return static_cast<CollectionOpts>(
    static_cast<int>(o1) & ~static_cast<int>(o2)
  );
}

inline bool any(CollectionOpts o) { return static_cast<int>(o); }

/*
 * Area used for writing down any information that is collected across
 * a series of step operations (possibly cross block).
 */
struct CollectedInfo {
  explicit CollectedInfo(const Index& index,
                         Context ctx,
                         ClassAnalysis* cls,
                         CollectionOpts opts,
                         const FuncAnalysis* fa = nullptr);

  ClosureUseVarMap closureUseTypes;
  PropertiesInfo props;
  ConstantMap cnsMap;
  hphp_fast_set<std::pair<const php::Func*, BlockId>>
    unfoldableFuncs;
  bool mayUseVV{false};
  bool effectFree{true};
  bool hasInvariantIterBase{false};
  bool readsUntrackedConstants{false};
  CollectionOpts opts{CollectionOpts::TrackConstantArrays};
  bool (*propagate_constants)(const Bytecode& bc, State& state,
                              BytecodeVec& out) = nullptr;
  CompactVector<Type> localStaticTypes;
  /*
   * See FuncAnalysisResult for details.
   */
  std::bitset<64> usedParams;

  PublicSPropMutations publicSPropMutations;
};

//////////////////////////////////////////////////////////////////////

/*
 * State merging functions, based on the union_of operation for types.
 *
 * These return true if the destination state changed.
 */
bool merge_into(ActRec&, const ActRec&);
bool merge_into(State&, const State&);

/*
 * State merging functions, based on the widening_union operation.
 * See analyze.cpp for details on when this is needed.
 */
bool widen_into(State&, const State&);
void widen_props(PropState&);

//////////////////////////////////////////////////////////////////////

/*
 * Functions to show various aspects of interpreter state as strings.
 */
std::string show(const ActRec& a);
std::string show(const php::Func&, const Base& b);
std::string show(const php::Func&, const State::MInstrState&);
std::string show(const php::Func&, const Iter&);
std::string property_state_string(const PropertiesInfo&);
std::string state_string(const php::Func&, const State&, const CollectedInfo&);

//////////////////////////////////////////////////////////////////////

}}

#endif
