/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
  ClsMeth,     // Definitely a static method on a class (possibly__callStatic).
  ObjInvoke,   // Closure invoke or __invoke on an object.
};

/*
 * Information about a pre-live ActRec.  Part of state tracked in
 * State.
 */
struct ActRec {
  explicit ActRec(FPIKind kind,
                  folly::Optional<res::Class> c = folly::none,
                  folly::Optional<res::Func> f = folly::none)
    : kind(kind)
    , cls(std::move(c))
    , func(std::move(f))
  {}

  FPIKind kind;
  folly::Optional<res::Class> cls;
  folly::Optional<res::Func> func;
};

/*
 * State of an iterator in the program.
 */
struct UnknownIter {};
struct TrackedIter { std::pair<Type,Type> kv; };
using Iter = boost::variant< UnknownIter
                           , TrackedIter
                           >;

/*
 * Tag indicating what sort of thing contains the current member base.
 *
 * The base is always the unboxed version of the type, and its
 * location could be inside of a Ref.  So, for example, a base with
 * BaseLoc::Frame could be located inside of a Ref that is pointed
 * to by the Frame.  (We may want to distinguish these two cases at
 * some point if we start trying to track real information about
 * Refs, but not yet.)
 */
enum class BaseLoc {
  /*
   * Base is in a number of possible places after an Elem op.  It
   * cannot possibly be in an object property (although it certainly
   * may alias one).  See miElem for details.  Not all post-elem ops
   * use this location (see LocalArrChain).
   *
   * If it is definitely in an array, the locTy in the Base will be
   * a subtype of TArr.
   */
  PostElem,

  /*
   * Base is in possible locations after a Prop op.  This means it
   * possibly lives in a property on an object, but possibly not
   * (e.g. it could be a null in tvScratch).  See miProp for
   * details.
   *
   * If it is definitely known to be a property in an object, the
   * locTy in the Base will be a subtype of TObj.
   */
  PostProp,

  /*
   * Known to be a static property on an object.  This is only
   * possible as an initial base.
   */
  StaticObjProp,

  /*
   * The base is inside of a local that contains a specialized array
   * type, and the arrayChain is non-empty.
   *
   * When the location is set to this, the chain will continue as
   * long as we keep staying inside specialized array types.  If it
   * moves to something like a ?Arr type, we must leave the chain
   * when the base moves.
   */
  LocalArrChain,

  /*
   * Known to be contained in the current frame as a local, as the
   * frame $this, by the evaluation stack, or inside $GLOBALS.  Only
   * possible as initial bases.
   */
  Frame,
  FrameThis,
  EvalStack,
  Global,

  /*
   * If we've execute an operation that's known to fatal, we use
   * this BaseLoc.
   */
  Fataled,
};

/*
 * Information about the current member base's type and location.
 */
struct Base {
  Type type;
  BaseLoc loc;

  /*
   * We also need to track effects of intermediate dims on the type
   * of the base.  So we have a type, name, and possibly associated
   * local for the base's container.
   *
   * For StaticObjProp, locName this is the name of the property if
   * known, or nullptr, and locTy is the type of the class
   * containing the static property.
   *
   * Similarly, if loc is PostProp, locName is the name of the
   * property if it was known, and locTy gives as much information
   * about the object type it is in.  (If we actually *know* it is
   * in an object, locTy will be a subtype of TObj.)
   */
  Type locTy;
  SString locName;
  borrowed_ptr<php::Local> local;
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
  bool initialized = false;
  bool unreachable = false;
  bool thisAvailable = false;
  std::vector<Type> locals;
  std::vector<Iter> iters;
  std::vector<Type> stack;
  std::vector<ActRec> fpiStack;

  /*
   * The current member base. Updated as we move through bytecodes representing
   * the operation.
   */
  Base base;

  /*
   * Chains of member operations on array elements will all affect the type of
   * something further back in the member instruction.  Currently this is just
   * used for locals.  This vector tracks the base,key type pair that was used
   * at each stage.  See irgen-minstr.cpp:resolveArrayChain().
   */
  std::vector<std::pair<Type,Type>> arrayChain;
};

/*
 * States are EqualityComparable (provided they are in-states for the
 * same block).
 */
bool operator==(const ActRec&, const ActRec&);
bool operator!=(const ActRec&, const ActRec&);
bool operator==(const State&, const State&);
bool operator!=(const State&, const State&);

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
using ClosureUseVarMap = std::map<
  borrowed_ptr<php::Class>,
  std::vector<Type>
>;

/*
 * Merge the types in the vector as possible use vars for the closure
 * `clo' into the destination map.
 */
void merge_closure_use_vars_into(ClosureUseVarMap& dst,
                                 borrowed_ptr<php::Class> clo,
                                 std::vector<Type>);

//////////////////////////////////////////////////////////////////////

/*
 * Area used for writing down any information that is collected across
 * a series of step operations (possibly cross block).
 */
struct CollectedInfo {
  explicit CollectedInfo(const Index& index,
                         Context ctx,
                         ClassAnalysis* cls,
                         PublicSPropIndexer* publicStatics)
    : props{index, ctx, cls}
    , publicStatics{publicStatics}
  {}

  ClosureUseVarMap closureUseTypes;
  PropertiesInfo props;
  PublicSPropIndexer* const publicStatics;
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
bool widen_into(PropState&, const PropState&);
bool widen_into(State&, const State&);

//////////////////////////////////////////////////////////////////////

/*
 * Functions to show various aspects of interpreter state as strings.
 */
std::string show(const ActRec& a);
std::string property_state_string(const PropertiesInfo&);
std::string state_string(const php::Func&, const State&);

//////////////////////////////////////////////////////////////////////

}}

#endif
