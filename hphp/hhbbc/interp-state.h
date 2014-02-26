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
#ifndef incl_HHBBC_INTERP_STATE_H_
#define incl_HHBBC_INTERP_STATE_H_

#include <vector>
#include <string>
#include <bitset>

#include "folly/Optional.h"

#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/bc.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

struct ClassAnalysis;

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
 * A program state at a position in a php::Block.
 */
struct State {
  bool initialized = false;
  bool thisAvailable = false;
  std::vector<Type> locals;
  std::vector<Type> stack;
  std::vector<ActRec> fpiStack;
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
 * Return a copy of a State without copying either the evaluation
 * stack or FPI stack.
 */
State without_stacks(const State&);

//////////////////////////////////////////////////////////////////////

/*
 * PropertiesInfo returns the PropState for private instance and static
 * properties.
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
 * State merging functions.
 *
 * These return true if the destination state changed.
 */
bool merge_into(PropState&, const PropState&);
bool merge_into(ActRec&, const ActRec&);
bool merge_into(State&, const State&);

//////////////////////////////////////////////////////////////////////

/*
 * Functions to show various aspects of interpreter state as strings.
 */
std::string show(const ActRec& a);
std::string property_state_string(const PropertiesInfo&);
std::string state_string(const php::Func&, const State&);

//////////////////////////////////////////////////////////////////////

constexpr int kMaxTrackedLocals = 128;

/*
 * StepFlags are information about the effects of a single opcode.
 * Each single-instruction step of the interpreter sends various
 * effects information back to the caller in this structure.
 */
struct StepFlags {
  /*
   * Potentially Exception-throwing Instruction.
   *
   * Instructions are assumed to be PEIs unless the abstract
   * interpreter says they aren't.  A PEI must propagate the state
   * from before the instruction across all factored exit edges.
   *
   * Some instructions that can throw with mid-opcode states need to
   * handle those cases specially.
   */
  bool wasPEI = true;

  /*
   * If a conditional branch at the end of the BB was known to be
   * taken (e.g. because the condition was a constant), this flag
   * indicates the state doesn't need to be propagated to the
   * fallthrough block.
   */
  bool tookBranch = false;

  /*
   * If true, we made a call to a function that never returns.
   */
  bool calledNoReturn = false;

  /*
   * If an instruction sets this flag, it means that if it pushed a
   * type with a constant value, it had no side effects other than
   * computing the value which was pushed.  This means the instruction
   * can be replaced with pops of its inputs followed by a push of the
   * constant.
   */
  bool canConstProp = false;

  /*
   * If an instruction may read or write to locals, these flags
   * indicate which ones.  We don't track this information for local
   * ids past kMaxTrackedLocals, which are assumed to always be in
   * this set.
   *
   * This is currently used to try to leave out unnecessary type
   * assertions on locals (for options.FilterAssertions), and as a
   * conservative list of variables that should be added to the gen
   * set for global dce.
   *
   * The latter use means these flags must be conservative in the
   * direction of which locals are read.  That is: an instruction may
   * not read a local that isn't mentioned in this set.
   */
  std::bitset<kMaxTrackedLocals> mayReadLocalSet;

  /*
   * If the instruction on this step could've been replaced with
   * cheaper bytecode, this is the list of bytecode that can be used.
   */
  folly::Optional<std::vector<Bytecode>> strengthReduced;

  /*
   * If this is not none, the interpreter executed a return on this
   * step, with this type.
   */
  folly::Optional<Type> returned;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
