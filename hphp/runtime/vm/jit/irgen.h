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

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/member-key.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/inline-state.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/ringbuffer.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace HPHP {

struct ArrayData;
struct Func;
struct ImmVector;
struct StringData;

namespace jit {

struct Block;
struct IRInstruction;
struct NormalizedInstruction;
struct SSATmp;

namespace irgen {

struct IRGS;

///////////////////////////////////////////////////////////////////////////////

/*
 * The public interface to the HHIR frontend is exported here.  Don't include
 * other headers outside of the implementation of this module.
 *
 * This module is used for two tasks:
 *
 *   o It is used by the region translator, and does most of the work of
 *     parsing HHBC into HHIR when we're trying to create an IRUnit out of a
 *     RegionDesc.
 *
 *   o The tracelet region selector uses this code to generate a RegionDesc.
 *
 * To use this code, the client creates an IRGS structure, and calls these
 * irgen::foo methods on it.
 *
 * TODO: we should push translateRegion and selectTracelet into this module, so
 * we can just export functions that do the above two tasks.
 */

///////////////////////////////////////////////////////////////////////////////

/*
 * The main function for generating new IRInstructions.  Attempts to append an
 * IRInstruction at the end of the current Block.
 *
 * Uses the same argument list format as IRUnit::gen.
 */
namespace detail { SSATmp* genInstruction(IRGS& env, IRInstruction*); }
template<class... Args>
SSATmp* gen(IRGS& env, Opcode op, Args&&... args) {
  return makeInstruction(
    [&] (IRInstruction* inst) { return detail::genInstruction(env, inst); },
    op,
    env.irb->nextBCContext(),
    std::forward<Args>(args)...
  );
}

/*
 * Create constant-valued SSATmps inside the IRUnit we're creating.
 */
template<class... Args>
SSATmp* cns(IRGS& env, Args&&... args) {
  return env.unit.cns(std::forward<Args>(args)...);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Type checks and assertions.
 */
void checkType(IRGS&, const Location&, Type, Block* exit);
void assertTypeStack(IRGS&, BCSPRelOffset, Type);
void assertTypeLocal(IRGS&, uint32_t id, Type);
void assertTypeLocation(IRGS&, const Location&, Type);
SSATmp* loadLocation(IRGS&, const Location&);

/*
 * After all initial guards instructions have been emitted, the client of this
 * module calls the following function to allow some "region header" code to be
 * emitted.
 */
void prepareEntry(IRGS&);

///////////////////////////////////////////////////////////////////////////////

/*
 * Sync VM regs eagerly
 */
void eagerVMSync(IRGS&, IRSPRelOffset spOff);

///////////////////////////////////////////////////////////////////////////////

/*
 * Support for Profiling counters, including reoptimization (CheckCold).
 */
void incProfCounter(IRGS&, TransID);
void checkCold(IRGS&, TransID);

/*
 * Exit if code coverage is enabled for the current function.
 */
void checkCoverage(IRGS& env);

/*
 * Exit if debugger interrupt is set on the current function.
 */
void checkDebuggerIntr(IRGS& env, SrcKey sk);

/*
 * Exit if exception breakpoint is set by the debugger.
 */
void checkDebuggerExceptionIntr(IRGS& env, Block* slowExit);

uint64_t curProfCount(const IRGS& env);
uint64_t calleeProfCount(const IRGS& env, const RegionDesc& calleeRegion);

/*
 * If ringbuffer tracing is enabled, generate a ringbuffer entry associated
 * with a SrcKey or string.
 */
void ringbufferEntry(IRGS&, Trace::RingBufferType, SrcKey, int level = 1);
void ringbufferMsg(IRGS&, Trace::RingBufferType, const StringData*,
                   int level = 1);

///////////////////////////////////////////////////////////////////////////////

/*
 * Before translating/processing each bytecode instruction, the driver
 * of the irgen module calls this function to move to the next
 * bytecode instruction (`newSk') to translate.
 */
void prepareForNextHHBC(IRGS&, SrcKey newSk);

/*
 * After translating each bytecode instruction, the driver of the
 * irgen module calls this function to signal that it has finished
 * processing the HHBC instruction.
 */
void finishHHBC(IRGS&);

/*
 * When done translating a region, or a block in a region, these calls are
 * made.
 */
void endRegion(IRGS&);
void endRegion(IRGS&, SrcKey);
void endBlock(IRGS&, SrcKey nextSk);

/*
 * When we're done creating the IRUnit, this function must be called to ensure
 * all the IR invariants hold.
 */
void sealUnit(IRGS&);

///////////////////////////////////////////////////////////////////////////////

/*
 * Returns whether `env' is currently inlining or not.
 */
bool isInlining(const IRGS& env);

/*
 * Returns the current depth of inlining in `env`.
 */
uint16_t inlineDepth(const IRGS& env);

/*
 * Begin inlining. Always succeeds.
 *
 * When doing gen-time inlining, we set up a series of IR instructions that
 * looks like this:
 *
 *   fp0  = DefFP
 *   sp   = DefFrameRelSP<offset>/DefRegSP<offset>
 *
 *   // ... normal stuff happens ...
 *   // ... probably some StStks due to argument expressions
 *   // FCall*:
 *     fp2   = BeginInlining<offset,func,cost,numArgs> sp fp
 *
 *         // ... callee body ...
 *
 *     EndInlining fp2
 */
void beginInlining(IRGS& env,
                   SrcKey entry,
                   SSATmp* ctx,
                   Offset asyncEagerOffset,
                   int cost,
                   SSATmp* calleeFP);

/*
 * End the current inlined frame, after all its blocks have been emitted.
 *
 * This decrefs locals and $this and pushes the return value onto the caller's
 * eval stack, in addition to the actual control transfer and bookkeeping done
 * by implInlineReturn().
 */
bool endInlining(IRGS& env, const RegionDesc& calleeRegion);

/*
 * Begin inlining func into a dummy region used to measure the cost of
 * inlining func. This will generate a region that cannot be executed.
 *
 * Simulating the inlining measures the cost of pushing a dummy frame (or not if
 * we are able to elide it) and any effects that may have on alias analysis.
 */
void conjureBeginInlining(IRGS& env,
                          SrcKey entry,
                          Type thisType,
                          const std::vector<Type>& inputs);

/*
 * Close an inlined function inserted using conjureBeginInlining; returns false
 * if the inlined region would have been invalid for inlining. As with
 * conjureBeginInlining, this function should not be used in a region that will
 * be executed.
 */
bool conjureEndInlining(IRGS& env,
                        const RegionDesc& calleeRegion,
                        bool builtin);

///////////////////////////////////////////////////////////////////////////////
/*
 * State introspection.
 *
 * These functions should only be used for inspecting state.  None of them
 * constrain types, so if the type information is actually used, it must be
 * constrained appropriately.
 */

/*
 * Access the type of the top of the stack.
 */
Type publicTopType(const IRGS& env, BCSPRelOffset);

/*
 * Return the proven Type for the given location.
 */
Type provenType(const IRGS&, const Location&);

///////////////////////////////////////////////////////////////////////////////
/*
 * Forward-declare an irgen::emitFoo function for each bytecode Foo.
 *
 * The arguments to the functions are pre-unpacked bytecode immediates.
 */

#define IMM_BLA        const ImmVector&
#define IMM_SLA        const ImmVector&
#define IMM_VSA        const ImmVector&
#define IMM_IVA        uint32_t
#define IMM_I64A       int64_t
#define IMM_LA         int32_t
#define IMM_NLA        NamedLocal
#define IMM_ILA        int32_t
#define IMM_IA         int32_t
#define IMM_DA         double
#define IMM_SA         const StringData*
#define IMM_RATA       RepoAuthType
#define IMM_AA         const ArrayData*
#define IMM_BA         Offset
#define IMM_OA(subop)  subop
#define IMM_KA         MemberKey
#define IMM_LAR        LocalRange
#define IMM_ITA        IterArgs
#define IMM_FCA        FCallArgs

#define NA /*  */
#define ONE(x0)              , IMM_##x0
#define TWO(x0, x1)          , IMM_##x0, IMM_##x1
#define THREE(x0, x1, x2)    , IMM_##x0, IMM_##x1, IMM_##x2
#define FOUR(x0, x1, x2, x3) , IMM_##x0, IMM_##x1, IMM_##x2, IMM_##x3
#define FIVE(x0, x1, x2, x3, x4) , IMM_##x0, IMM_##x1, IMM_##x2, IMM_##x3, IMM_##x4
#define SIX(x0, x1, x2, x3, x4, x5) , IMM_##x0, IMM_##x1, IMM_##x2, IMM_##x3, IMM_##x4, IMM_##x5

#define O(name, imms, ...) void emit##name(IRGS& imms);
  OPCODES
#undef O

#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef SIX

#undef IMM_MA
#undef IMM_BLA
#undef IMM_SLA
#undef IMM_VSA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_NLA
#undef IMM_ILA
#undef IMM_IA
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA
#undef IMM_KA
#undef IMM_LAR
#undef IMM_ITA
#undef IMM_FCA

///////////////////////////////////////////////////////////////////////////////

}}}
