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
#ifndef incl_HPHP_JIT_IRGEN_EXIT_H_
#define incl_HPHP_JIT_IRGEN_EXIT_H_

#include <vector>
#include <functional>

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/types.h"  // TransFlags

#include "hphp/runtime/vm/jit/ir-builder.h"

// This header has to include internal, because it uses things like
// peekSpillValues.
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { struct StringData; }
namespace HPHP { namespace jit {
struct HTS;
struct Block;
}}

namespace HPHP { namespace jit { namespace irgen {

//////////////////////////////////////////////////////////////////////

Block* makeExit(HTS&, Offset targetBcOff = -1);
Block* makeExit(HTS&, TransFlags trflags);
Block* makeExit(HTS&, Offset targetBcOff,
                std::vector<SSATmp*>& spillValues,
                TransFlags trflags = TransFlags{});
Block* makeExitWarn(HTS&,
                    Offset targetBcOff,
                    std::vector<SSATmp*>& spillValues,
                    const StringData* warning);
Block* makeExitError(HTS&, SSATmp* msg, Block* catchBlock);
Block* makeExitNullThis(HTS&);
Block* makePseudoMainExit(HTS&, Offset targetBcOff = -1);
Block* makeExitOpt(HTS&, TransID);
Block* makeExitSlow(HTS&);

/*
 * Implementation for the above.  Takes spillValues, target offset, and a flag
 * for whether to make a no-IR exit.
 *
 * Also takes a CustomExit() function that may perform more operations and
 * optionally return a single additional SSATmp* (otherwise nullptr) to spill
 * on the stack before exiting.
 */
enum class ExitFlag {
  Interp,     // will bail to the interpreter to execute at least one BC instr
  JIT,        // will attempt to use the JIT to create a new translation
  // DelayedMarker means to use the current instruction marker
  // instead of one for targetBcOff.
  DelayedMarker,
};
using CustomExit = std::function<SSATmp* ()>;
Block* makeExitImpl(HTS&,
                    Offset targetBcOff,
                    ExitFlag flag,
                    std::vector<SSATmp*>& spillValues,
                    const CustomExit& customFn,
                    TransFlags trflags = TransFlags{});

//////////////////////////////////////////////////////////////////////

/*
 * Generate a side-exit that runs the code from `exit' before leaving the
 * region.
 */
template<class ExitLambda>
Block* makeSideExit(HTS& env, Offset targetBcOff, ExitLambda exit) {
  auto spillValues = peekSpillValues(env);
  return makeExitImpl(env, targetBcOff,
    ExitFlag::DelayedMarker, spillValues, exit);
}

//////////////////////////////////////////////////////////////////////

/*
 * Create a catch block with a user-defined body (usually empty or a
 * SpillStack). Regardless of what body() does, it must return the current
 * stack pointer. This is a block to be invoked by the unwinder while unwinding
 * through a call to C++ from translated code. When attached to an instruction
 * as its taken field, code will be generated and the block will be registered
 * with the unwinder automatically.
 */
template<class Body>
Block* makeCatchImpl(HTS& env, Body body) {
  auto const exit = env.irb->makeExit(Block::Hint::Unused);

  BlockPusher bp(*env.irb, makeMarker(env, bcOff(env)), exit);
  gen(env, BeginCatch);
  auto sp = body();
  gen(env, EndCatch, fp(env), sp);

  return exit;
}

/*
 * Create a catch block with no SpillStack. Some of our optimizations rely on
 * the ability to insert code on *every* path out of a trace, so we can't
 * simply elide the catch block in the cases that want an empty body.
 */
Block* makeCatchNoSpill(HTS& env);

/*
 * Create a catch block that spills the current state of the eval stack.
 *
 * Note: declared in ht-internal right now.
 */
// Block* makeCatch(HTS&);

//////////////////////////////////////////////////////////////////////

}}}

#endif
