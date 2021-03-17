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
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/context.h"

namespace HPHP { namespace HHBBC {

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
   * A set of pair of functions and their push blocks that we failed to fold.
   */
  hphp_fast_set<std::pair<const php::Func*, BlockId>> unfoldableFuncs;

  /*
   * Bitset representing which parameters may affect the result of the
   * function, assuming it produces one. Note that VerifyParamType
   * does not count as a use in this context.
   */
  std::bitset<64> usedParams;

  /*
   * For an 86cinit, any constants that we resolved.
   * The size_t is the index into ctx.cls->constants
   */
  CompactVector<std::pair<size_t,TypedValue>> resolvedConstants;

  /*
   * Public static property mutations in this function.
   */
  PublicSPropMutations publicSPropMutations;

  /*
   * Vector of block updates
   */
  CompactVector<std::pair<BlockId, BlockUpdateInfo>> blockUpdates;
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

  // Whether this class might have a bad initial value for a property.
  bool badPropInitialValues{false};
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
                                 CollectionOpts opts = {});

/*
 * Perform an analysis for a whole php::Class at a time.
 *
 * This involves doing a analyze_func call on each of its functions,
 * and inferring some whole-class information at the same time.
 */
ClassAnalysis analyze_class(const Index&, const Context&);

/*
 * Propagate a block input State to each instruction in the block.
 *
 * Returns a vector that is parallel to the instruction array in the
 * block, with one extra element.  The vector contains a state before
 * each instruction, and the StepFlags for executing that instruction.
 *
 * The last element in the vector contains the state after the last
 * instruction in the block, with undefined StepFlags.
 *
 * Pre: stateIn.initialized == true
 */
std::vector<std::pair<State,StepFlags>>
locally_propagated_states(const Index&,
                          const AnalysisContext&,
                          CollectedInfo& collect,
                          BlockId bid,
                          State stateIn);

/*
 * Propagate a block input State to find the output state for a particular
 * target of the block.  This is used to update the in state for a block added
 * to the CFG in betwee analysis rounds.
 */
State locally_propagated_bid_state(const Index& index,
                                   const AnalysisContext& ctx,
                                   CollectedInfo& collect,
                                   BlockId bid,
                                   State state,
                                   BlockId targetBid);
//////////////////////////////////////////////////////////////////////

}}
