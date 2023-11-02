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

#include <vector>
#include <utility>

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/type-structure.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/context.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * The result of a function-at-a-time type analysis, as needed for
 * updating the index. Does not include per-block state.
 */
struct FuncAnalysisResult {
  /*
   * Initializes this structure so rpoBlocks contains the func's
   * blocks according to rpoSortAddDVs(), each bdata entry has an
   * rpoId index, and all block states are uninitialized.
   */
  explicit FuncAnalysisResult(AnalysisContext);

  FuncAnalysisResult(FuncAnalysisResult&&) = default;
  FuncAnalysisResult& operator=(FuncAnalysisResult&&) = default;

  /*
   * FuncAnalysis carries the Context it was created for because
   * generally you'll need it again when you look at the analysis
   * results.
   *
   * Note that the Context is adjusted to account for the fact that
   * Closure __invoke methods run in the context of a class other than
   * their declaring class.  So ctx.func->cls will not be the same as
   * ctx->cls in this case.
   */
  Context ctx;

  /*
   * If this function allocates closures, this maps each of those
   * closure classes to the types of its used variables, in their
   * declared order.
   */
  ClosureUseVarMap closureUseTypes;

  /*
   * The inferred function return type.  May be TBottom if the
   * function never returns.
   */
  Type inferredReturn;

  /*
   * The number of times that inferredReturn was refined inside
   * analyze_class. We need this to track accurately the total number
   * of refinements.
   */
  size_t localReturnRefinements = 0;

  /*
   * If the function returns one of its parameters, the index of that
   * parameter. MaxLocalId and above indicate that it doesn't return a
   * parameter.
   */
  LocalId retParam{MaxLocalId};

  /*
   * Flag to indicate that the function is effectFree, in the sense
   * that calls to it can be constant folded or dced (note that calls
   * are never truly effect free, because profilers could be enabled,
   * or other surprise flags could fire - but we ignore that for this
   * flag).
   */
  bool effectFree{false};

  /*
   * Flag to indicate that an iterator's base was unchanged on at least one path
   * to that iterator's release. If this is false, we can skip doing the more
   * expensive LIter optimization pass (because it will never succeed).
   */
  bool hasInvariantIterBase{false};

  /*
   * A set of call contexts that we failed to fold.
   */
  hphp_fast_set<CallContext, CallContextHasher> unfoldableFuncs;

  /*
   * Bitset representing which parameters may affect the result of the
   * function, assuming it produces one. Note that the parameter type
   * verification does not count as a use in this context.
   */
  std::bitset<64> usedParams;

  /*
   * For an 86cinit, any constants that we inferred a type for. For
   * 86pinit or 86sinit, any resolved initial values for
   * properties. These two cases are disjoint, so we save memory by
   * using a variant type.
   */
  UniqueEither<ResolvedConstants*, ResolvedPropInits*> resolvedInitializers;

  /*
   * Public static property mutations in this function.
   */
  PublicSPropMutations publicSPropMutations;

  /*
   * Vector of block updates
   */
  CompactVector<std::pair<BlockId, CompressedBlockUpdate>> blockUpdates;
};

struct FuncAnalysis : FuncAnalysisResult {
  struct BlockData {
    uint32_t rpoId;
    State stateIn;
    bool noThrow{true};
  };

  explicit FuncAnalysis(AnalysisContext);

  FuncAnalysis(FuncAnalysis&&) = default;
  FuncAnalysis& operator=(FuncAnalysis&&) = default;

  // Block ids in a reverse post order, with DV initializers.
  std::vector<BlockId> rpoBlocks;

  // Block data is indexed by Block::id.
  std::vector<BlockData> bdata;
};

/*
 * Store the worklist of functions to be analyzed during
 * class-at-a-time analysis (along with intra-class dependencies).
 */
struct ClassAnalysisWorklist {
  const php::Func* next();

  void scheduleForProp(SString name);
  void scheduleForPropMutate(SString name);
  void scheduleForReturnType(const php::Func& callee);

  void addPropDep(SString name, const php::Func& f) {
    propDeps[name].emplace(&f);
  }
  void addPropMutateDep(SString name, const php::Func& f) {
    propMutateDeps[name].emplace(&f);
  }
  void addReturnTypeDep(const php::Func& callee, const php::Func& f) {
    returnTypeDeps[&callee].emplace(&f);
  }

  bool empty() const { return worklist.empty(); }

  // Put a func on the worklist. Return true if the func was
  // scheduled, false if it was already on the list.
  bool schedule(const php::Func& f);

  const hphp_fast_set<const php::Func*>*
  depsForReturnType(const php::Func& f) const {
    auto const it = returnTypeDeps.find(&f);
    if (it == returnTypeDeps.end()) return nullptr;
    return &it->second;
  }

private:
  hphp_fast_set<const php::Func*> inWorklist;
  std::deque<const php::Func*> worklist;

  template <typename T> using Deps =
    hphp_fast_map<T, hphp_fast_set<const php::Func*>>;
  Deps<SString> propDeps;
  Deps<SString> propMutateDeps;
  Deps<const php::Func*> returnTypeDeps;
};

struct ClassAnalysisWork {
  ClassAnalysisWorklist worklist;
  hphp_fast_map<const php::Func*, Index::ReturnType> returnTypes;
  hphp_fast_map<const php::Func*, hphp_fast_set<SString>> propMutators;
  bool noPropRefine = false;
};

/*
 * The result of a class-at-a-time analysis.
 */
struct ClassAnalysis {
  explicit ClassAnalysis(const Context& ctx) :
      ctx(ctx) {}

  ClassAnalysis(ClassAnalysis&&) = default;
  ClassAnalysis& operator=(ClassAnalysis&&) = default;

  // The context that describes the class we did this analysis for.
  Context ctx;

  // FuncAnalysis results for each of the methods on the class, and
  // for each closure allocated in the class's context.
  CompactVector<FuncAnalysisResult> methods;
  CompactVector<FuncAnalysisResult> closures;

  // Inferred types for private instance and static properties.
  PropState privateProperties;
  PropState privateStatics;

  ResolvedPropInits resolvedProps;

  ClassAnalysisWork* work{nullptr};
};

//////////////////////////////////////////////////////////////////////

// Local state for resolving class constants and reaching a fixed
// point.
struct ClsConstantWork {
  ClsConstantWork(const Index&, const php::Class&);

  ClsConstLookupResult lookup(SString);

  void update(SString, Type);
  void add(SString);
  SString next();

  void setCurrent(SString n) {
    assertx(!current);
    current = n;
  }
  void clearCurrent(SString n) {
    assertx(current == n);
    current = nullptr;
  }

  void schedule(SString);

  const php::Class& cls;
  hphp_fast_map<SString, ClsConstInfo> constants;

  SString current{nullptr};
  hphp_fast_map<SString, hphp_fast_set<SString>> deps;
  hphp_fast_set<SString> inWorklist;
  std::deque<SString> worklist;
};

//////////////////////////////////////////////////////////////////////

/*
 * Perform a flow-sensitive type analysis on a function, using the
 * given Index and Context when we need information about things
 * outside of this function.
 *
 * This routine makes no changes to the php::Func.
 */
FuncAnalysis analyze_func(const Index&, const AnalysisContext&,
                          CollectionOpts opts);

/*
 * Perform a flow-sensitive type analysis on a function, using the
 * given Index and Context when we need information about things
 * outside of this function, and assuming that the arguments to the
 * function have the supplied types.
 *
 * This function is used to perform callsite-sensitive type inference.
 *
 * Currently this is not supported for closure bodies.
 */
FuncAnalysis analyze_func_inline(const Index&,
                                 const AnalysisContext&,
                                 const Type& thisType,
                                 const CompactVector<Type>& args,
                                 ClsConstantWork* clsCnsWork = nullptr,
                                 CollectionOpts opts = {});

/*
 * Perform an analysis for a whole php::Class at a time.
 *
 * This involves doing a analyze_func call on each of its functions,
 * and inferring some whole-class information at the same time.
 */
ClassAnalysis analyze_class(const Index&, const Context&);

/*
 * Represents the various interp state at some particular instruction
 * within a block, with an ability to "rewind" the state to previous
 * instructions (up until the entry of the block).
 */
struct PropagatedStates {
  // Initialize with the end of block state, and the gathered undo log
  // corresponding to that block interp.
  PropagatedStates(State&&, StateMutationUndo);

  // Rewind the state one instruction. Must not be at the beginning of
  // the block.
  void next();

  // Stack and local types:
  const CompactVector<Type>& stack() const { return m_stack; }
  const CompactVector<Type>& locals() const { return m_locals; }

  // The type for a particular local *after* the current instruction
  // runs.
  const Type& localAfter(LocalId id) const {
    for (auto const& p : m_afterLocals) {
      if (p.first == id) return p.second;
    }
    assertx(id < m_locals.size());
    return m_locals[id];
  }

  // The value pushed by current instruction (IE, the top of the stack
  // before next()).
  const Optional<Type>& lastPush() const { return m_lastPush; }

  // Interp flags for the current instruction.
  bool wasPEI() const { return currentMark().wasPEI; }
  bool unreachable() const { return currentMark().unreachable; }
  auto const& mayReadLocalSet() const { return currentMark().mayReadLocalSet; }

private:
  const StateMutationUndo::Mark& currentMark() const {
    assertx(!m_undos.events.empty());
    auto const mark =
      boost::get<StateMutationUndo::Mark>(&m_undos.events.back());
    assertx(mark);
    return *mark;
  }

  Optional<Type> m_lastPush;
  CompactVector<Type> m_stack;
  CompactVector<Type> m_locals;
  CompactVector<std::pair<LocalId, Type>> m_afterLocals;
  StateMutationUndo m_undos;
};

/*
 * Interpret a block and return a PropagatedStates corresponding to
 * the state at the end of the block.
 *
 * Pre: stateIn.initialized == true
 */
PropagatedStates
locally_propagated_states(const Index&,
                          const AnalysisContext&,
                          CollectedInfo& collect,
                          BlockId bid,
                          State stateIn);

/*
 * Propagate a block input State to find the output state for a particular
 * target of the block.  This is used to update the in state for a block added
 * to the CFG in between analysis rounds.
 */
State locally_propagated_bid_state(const Index& index,
                                   const AnalysisContext& ctx,
                                   CollectedInfo& collect,
                                   BlockId bid,
                                   State state,
                                   BlockId targetBid);

//////////////////////////////////////////////////////////////////////

/*
 * Resolve a type-constraint into its equivalent set of HHBBC types.
 *
 * In general, a type-constraint cannot be represented exactly by a
 * single HHBBC type, so a lower and upper bound is provided
 * instead.
 *
 * A "candidate" type can be provided which will be applied to the
 * type-constraint and can further constrain the output types. This
 * is useful for the magic interfaces, whose lower-bound cannot be
 * precisely represented by a single type.
 */
struct ConstraintType {
  // Lower bound of constraint. Any type which is a subtype of this
  // is guaranteed to pass a type-check without any side-effects.
  Type lower;
  // Upper bound of constraint. Any type which does not intersect
  // with this is guaranteed to always fail a type-check.
  Type upper;
  // If this type-constraint might promote a "classish" type to a
  // static string as a side-effect.
  TriBool coerceClassToString{TriBool::No};
  // Whether this type-constraint might map to a mixed
  // type-hint. The mixed type-hint has special semantics when it
  // comes to properties.
  bool maybeMixed{false};
};
ConstraintType lookup_constraint(const Index&,
                                 const Context&,
                                 const TypeConstraint&,
                                 const Type& candidate = TCell);

/*
 * Returns a tuple containing a type after the parameter type
 * verification, a flag indicating whether the verification is a no-op
 * (because it always passes without any conversion), and a flag
 * indicating whether the verification is effect free (the
 * verification could convert a type without causing a side-effect).
 */
std::tuple<Type, bool, bool> verify_param_type(const Index&,
                                               const Context&,
                                               uint32_t paramId,
                                               const Type&);

/*
 * Given a type, adjust the type for the given type-constraint. If there's no
 * type-constraint, or if property type-hints aren't being enforced, then return
 * the type as is. This might return TBottom if the type is not compatible with
 * the type-hint.
 */
Type adjust_type_for_prop(const Index& index,
                          const php::Class& propCls,
                          const TypeConstraint* tc,
                          const Type& ty);

/*
 * Resolve a type-constraint to its equivalent
 * ConstraintType. Candidate can be used to narrow the type in some
 * situations. The std::functions are called when classes need to be
 * resolved, or for "this" type-hints.
 */
ConstraintType
type_from_constraint(const TypeConstraint& tc,
                     const Type& candidate,
                     const std::function<Optional<res::Class>(SString)>& resolve,
                     const std::function<Optional<Type>()>& self);

/*
 * Given two ConstraintTypes, compute and return the union.
 */
ConstraintType union_constraint(const ConstraintType& a,
                                const ConstraintType& b);
std::string show(const ConstraintType&);

//////////////////////////////////////////////////////////////////////

/*
 * Try to resolve self/parent types in the given context.
 */
Optional<Type> selfCls(const Index&, const Context&);
Optional<Type> selfClsExact(const Index&, const Context&);

Optional<Type> parentCls(const Index&, const Context&);
Optional<Type> parentClsExact(const Index&, const Context&);

//////////////////////////////////////////////////////////////////////

/*
 * Resolve a builtin class with the given name. This just calls
 * index.resolve_class(), but has some additional sanity checks that
 * the resultant class is a builtin.
 */
res::Class builtin_class(const Index&, SString);

//////////////////////////////////////////////////////////////////////

}
