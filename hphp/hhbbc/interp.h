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

#include <functional>
#include <vector>
#include <bitset>

#include "hphp/hhbbc/bc.h"
#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP::HHBBC {

struct PropertiesInfo;
struct CollectedInfo;
struct State;
struct StateMutationUndo;
struct StepFlags;
struct Bytecode;
struct ISS;
namespace php { struct Block; }
namespace res { struct Func; }

//////////////////////////////////////////////////////////////////////

constexpr auto kReadOnlyConstant = kInvalidDataType;

//////////////////////////////////////////////////////////////////////

struct BlockUpdateInfo {
  BlockId fallthrough{NoBlockId};
  uint32_t unchangedBcs{0};
  CompactVector<Bytecode> replacedBcs;
};

/*
 * RunFlags are information about running an entire block in the
 * interpreter.
 */
struct RunFlags {
  /*
   * If this is not none, the interpreter executed a return in this
   * block, with this type.
   */
  Optional<Type> returned;

  /*
   * If returned is set, and the returned value was a parameter,
   * retParam will be set to the parameter's id; otherwise it will be
   * NoLocalId.
   */
  LocalId retParam{NoLocalId};
  BlockUpdateInfo updateInfo;

  bool noThrow{true};
};

//////////////////////////////////////////////////////////////////////

constexpr int kMaxTrackedLocals = 512;

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
   * from before the instruction across all throw exit edges.
   *
   * Some instructions that can throw with mid-opcode states need to
   * handle those cases specially.
   */
  bool wasPEI = true;

  /*
   * If an instruction sets this flag, it means that if it pushed a
   * type with a constant value, it had no side effects other than
   * computing the value which was pushed.  This means the instruction
   * can be replaced with pops of its inputs followed by a push of the
   * constant.
   */
  bool canConstProp = false;

  /*
   * If an instruction sets this flag, it means that this
   * instruction doesn't prevent a call to the containing function
   * from being discarded if its result is unneeded.
   *
   * Instructions that are marked canConstProp that also produce a
   * constant result automatically set this flag.
   */
  bool effectFree = false;

  /*
   * Set by impl_vec to indicate that this instruction was already
   * dealt with via reduce.
   */
  bool reduced = false;

  /*
   * If set to something other than NoBlockId, then this block
   * unconditionally falls through to that block.
   */
  BlockId jmpDest = NoBlockId;

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
   * If this is not none, the interpreter executed a return on this
   * step, with this type.
   */
  Optional<Type> returned;

  /*
   * If returned is set, and the returned value was a parameter,
   * retParam will be set to the parameter's id; otherwise it will be
   * NoLocalId.
   */
  LocalId retParam{NoLocalId};
};

//////////////////////////////////////////////////////////////////////

/*
 * Context for running the block interpreter (either on a single
 * instruction, or for a whole block).
 */
struct Interp {
  const IIndex& index;
  AnalysisContext ctx;
  CollectedInfo& collect;
  const BlockId bid;
  const php::Block* blk;
  State& state;
  StateMutationUndo* undo = nullptr;
};

/*
 * Step a single instruction in the block, and hand back flags
 *
 * This entry point is used to propagate block entry states to
 * mid-block positions after the global analysis has already finished.
 */
StepFlags step(Interp&, const Bytecode& op);

/*
 * Run a whole block.  Returns a type that should be merged into the
 * function return value, or TBottom.
 *
 * If a branch is taken or an exception is thrown, the supplied
 * callback is used to indicate when/where the state referenced in the
 * Interp structure should be propagated.
 *
 * If the PropagateFn is called with a nullptr State, it means that
 * the given block should be re-processed.
 */
using PropagateFn = std::function<void (BlockId, const State*)>;
RunFlags run(Interp&, const State& in, PropagateFn);

/*
 * Dispatch a bytecode to the default interpreter.
 *
 * This entry point is used by custom interpreters that need to add
 * some logic to the default interpreter but want to run it otherwise.
 * Calling step() does not give control over the state (ISS instance)
 * which a custom interpreter may need to specialize.
 */
void default_dispatch(ISS&, const Bytecode&);

/*
 * Can this call be converted to an FCallBuiltin
 */
bool optimize_builtin(ISS& env, const php::Func* func, const FCallArgs& fca);

Optional<Type>
const_fold(ISS& env, uint32_t nArgs, uint32_t numExtraInputs,
           const php::Func& phpFunc, bool variadicsPacked);

/*
 * Extracts name from the type either by using a reified name specialization or
 * by looking at the typed value
 */
SString getNameFromType(const Type& t);

//////////////////////////////////////////////////////////////////////

}
