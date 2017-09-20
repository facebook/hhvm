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
#ifndef incl_HHBBC_ANALYZE_H_
#define incl_HHBBC_ANALYZE_H_

#include <vector>
#include <utility>

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * The result of a function-at-a-time type analysis.
 *
 * For each block, contains an input state describing the types of
 * locals, stack elements, etc.
 */
struct FuncAnalysis {
  using BlockData = struct { uint32_t rpoId; State stateIn; };

  /*
   * Initializes this structure so rpoBlocks contains the func's
   * blocks according to rpoSortAddDVs(), each bdata entry has an
   * rpoId index, and all block states are uninitialized.
   */
  explicit FuncAnalysis(Context);

  FuncAnalysis(FuncAnalysis&&) = default;
  FuncAnalysis& operator=(FuncAnalysis&&) = default;

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

  // Blocks in a reverse post order, with DV initializers.
  std::vector<borrowed_ptr<php::Block>> rpoBlocks;

  // Block data is indexed by Block::id.
  std::vector<BlockData> bdata;

  /*
   * The inferred function return type.  May be TBottom if the
   * function never returns.
   */
  Type inferredReturn;

  /*
   * If this function allocates closures, this maps each of those
   * closure classes to the types of its used variables, in their
   * declared order.
   */
  ClosureUseVarMap closureUseTypes;

  /*
   * With HardConstProp enabled, the set of constants that this
   * function could define.
   */
  ConstantMap cnsMap;

  /*
   * Reads a constant thats not in the index (yet - this can only
   * happen on the first iteration). We'll need to revisit it.
   */
  bool readsUntrackedConstants{false};

  /*
   * Flag to indicate that the function does something that requires a
   * variable environment.
   */
  bool mayUseVV;

  /*
   * Flag to indicate that the function is effectFree, in the sense
   * that calls to it can be constant folded or dced (note that calls
   * are never truly effect free, because profilers could be enabled,
   * or other surprise flags could fire - but we ignore that for this
   * flag).
   */
  bool effectFree{false};

  /*
   * A set of functions that are called with constant args, but which
   * are not foldable.
   */
  std::unordered_set<borrowed_ptr<const php::Func>> unfoldableFuncs;

  /*
   * Known types of local statics.
   */
  CompactVector<Type> localStaticTypes;
};

/*
 * The result of a class-at-a-time analysis.
 */
struct ClassAnalysis {
  ClassAnalysis(Context ctx, bool anyInterceptable) :
      ctx(ctx), anyInterceptable(anyInterceptable) {}

  ClassAnalysis(ClassAnalysis&&) = default;
  ClassAnalysis& operator=(ClassAnalysis&&) = default;

  // The context that describes the class we did this analysis for.
  Context ctx;

  // FuncAnalysis results for each of the methods on the class, and
  // for each closure allocated in the class's context.
  CompactVector<FuncAnalysis> methods;
  CompactVector<FuncAnalysis> closures;

  // Constants which we resolved by evaluating the 86cinit
  // The size_t is the index into ctx.cls->constants
  CompactVector<std::pair<size_t,TypedValue>> resolvedConstants;

  // Inferred types for private instance and static properties.
  PropState privateProperties;
  PropState privateStatics;
  bool anyInterceptable;
};

//////////////////////////////////////////////////////////////////////

/*
 * Perform a flow-sensitive type analysis on a function, using the
 * given Index and Context when we need information about things
 * outside of this function.
 *
 * This routine makes no changes to the php::Func.
 */
FuncAnalysis analyze_func(const Index&, Context, CollectionOpts opts);

/*
 * Analyze a function like analyze_func, but exposing gathered CollectedInfo
 * results.  The CollectedInfo structure can be initialized by the caller to
 * enable collecting some pass-specific types of information (e.g. public
 * static property types).
 */
FuncAnalysis analyze_func_collect(const Index&, Context, CollectedInfo&);

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
                                 Context,
                                 std::vector<Type> args,
                                 CollectionOpts opts =
                                 CollectionOpts::TrackConstantArrays);

/*
 * Perform an analysis for a whole php::Class at a time.
 *
 * This involves doing a analyze_func call on each of its functions,
 * and inferring some whole-class information at the same time.
 */
ClassAnalysis analyze_class(const Index&, Context);

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
                          const FuncAnalysis&,
                          borrowed_ptr<const php::Block>,
                          State stateIn);

//////////////////////////////////////////////////////////////////////

}}

#endif
