/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HHBBC_ABSTRACT_INTERP_H_
#define incl_HHBBC_ABSTRACT_INTERP_H_

#include <vector>
#include <map>

#include "folly/Optional.h"

#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/representation.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * Information about a pre-live ActRec.  Part of state tracked in
 * State.
 */
struct ActRec {
  explicit ActRec(FPIKind kind, folly::Optional<res::Func> f = folly::none)
    : kind(kind)
    , func(f)
  {}

  FPIKind kind;
  folly::Optional<res::Func> func;
};

/*
 * A program state at a position in a php::Block.  Only the block
 * entry states are saved for the FuncAnalysis result structure.
 */
struct State {
  bool initialized = false;
  bool thisAvailable = false;
  std::vector<Type> locals;
  std::vector<Type> stack;
  std::vector<ActRec> fpiStack;

  // Private property types on the current object.
  PropState privateProperties;
};

/*
 * States are EqualityComparible (provided they are in-states for the
 * same block).
 */
bool operator==(const ActRec&, const ActRec&);
bool operator!=(const ActRec&, const ActRec&);
bool operator==(const State&, const State&);
bool operator!=(const State&, const State&);

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

  // FuncAnalysis carries the Context it was created for because using
  // the wrong FuncAnalysis is a pretty bad thing to do.  (Yes I did
  // it.)  And any function that wants to look at a FuncAnalysis is
  // going to need the source Context.
  Context ctx;

  // Blocks in a reverse post order, with DV initializers.
  std::vector<borrowed_ptr<php::Block>> rpoBlocks;

  // Block data is indexed by Block::id.
  std::vector<BlockData> bdata;

  // The inferred function return type.  May be TBottom if the
  // function never returns.
  Type inferredReturn;
};

struct ClassAnalysis {
  explicit ClassAnalysis(Context ctx) : ctx(ctx) {}

  ClassAnalysis(ClassAnalysis&&) = default;
  ClassAnalysis& operator=(ClassAnalysis&&) = default;

  // The context that describes the class we did this analysis for.
  Context ctx;

  // FuncAnalysis results for each of the methods on the class.
  std::vector<FuncAnalysis> methods;

  // Inferred types for private properties.
  PropState privateProperties;
};

//////////////////////////////////////////////////////////////////////

/*
 * Perform a flow-sensitive type analysis on a function, using the
 * given Index and Context when we need information about things
 * outside of this function.
 *
 * This routine makes no changes to the php::Func.
 */
FuncAnalysis analyze_func(const Index&, Context);

/*
 * Perform an analysis for a whole php::Class at a time.
 *
 * This involves doing a analyze_func call on each of its functions,
 * and inferring some whole-class information at the same time.
 */
ClassAnalysis analyze_class(const Index&, Context);

/*
 * Use information from an analyze call to perform various
 * optimizations on a function.
 *
 * The Index should be unchanged since the one that was provided to
 * the corresponding analyze_func call.
 *
 * This routine may modify the php::Blocks attached to the passed-in
 * php::Func, but it won't modify the top-level meta-data in the
 * php::Func itself.
 */
void optimize_func(const Index&, const FuncAnalysis&);

/*
 * Combine the above two routines.  Convenient for single_unit.
 */
void analyze_and_optimize_func(const Index&, Context);

//////////////////////////////////////////////////////////////////////

}}

#endif
