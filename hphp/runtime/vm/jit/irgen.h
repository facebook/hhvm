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
#ifndef incl_HPHP_JIT_IRGEN_H_
#define incl_HPHP_JIT_IRGEN_H_

#include <cstdint>

#include "hphp/util/ringbuffer.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/hhbc.h"

namespace HPHP { struct SrcKey; }
namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

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

namespace irgen {

//////////////////////////////////////////////////////////////////////

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
    env.irb->curMarker(),
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

/*
 * Guards, checks and type assertions.  Guards and checks are the same thing,
 * except that guards must happen at the first bytecode offset in the region.
 *
 * TODO(#5706706): the stack versions should not be exported, except that
 * RegionDesc::Location::Stack needs some fixes first.
 */
void assertTypeStack(IRGS&, BCSPOffset, Type);
void checkTypeStack(IRGS&, BCSPOffset, Type, Offset dest, bool outerOnly);
void checkTypeLocal(IRGS&, uint32_t locId, Type, Offset dest, bool outerOnly);
void assertTypeLocation(IRGS&, const RegionDesc::Location&, Type);
void checkTypeLocation(IRGS&, const RegionDesc::Location&, Type, Offset dest,
                       bool outerOnly);

/*
 * Type predictions.
 */
void predictTypeLocation(IRGS&, const RegionDesc::Location&, Type);
void predictTypeStack(IRGS&, int32_t offset, Type);
void predictTypeLocal(IRGS&, uint32_t locId, Type);

/*
 * Special type of guards for param-passing reffiness. These checks are needed
 * when an FPush* instruction is in a different region from its FCall, and we
 * don't know statically whether the callee will want arguments by reference.
 */
void checkRefs(IRGS&, int64_t entryArDelta, const std::vector<bool>& mask,
               const std::vector<bool>& vals, Offset);

/*
 * After all initial guards instructions have been emitted, the client of this
 * module calls the following function to allow some "region header" code to be
 * emitted.
 */
void prepareEntry(IRGS&);

/*
 * Creates a no-op IR instruction that branches to an exit. These placeholder
 * instructions are later removed after any passes that want to use them for
 * their exits.
 */
void makeExitPlaceholder(IRGS&);

//////////////////////////////////////////////////////////////////////

/*
 * Support for translation counters of various types, including reoptimization
 * (CheckCold).
 */
void incTransCounter(IRGS&);
void incProfCounter(IRGS&, TransID);
void checkCold(IRGS&, TransID);

/*
 * If ringbuffer tracing is enabled, generate a ringbuffer entry associated
 * with a SrcKey or string.
 */
void ringbufferEntry(IRGS&, Trace::RingBufferType, SrcKey, int level = 1);
void ringbufferMsg(IRGS&, Trace::RingBufferType, const StringData*,
                   int level = 1);

//////////////////////////////////////////////////////////////////////

/*
 * For handling PUNT-based interpOnes.  When we PUNT, an exception is thrown
 * and the whole region is retried, with a bit set to interp the instruction
 * that failed.
 */
void interpOne(IRGS&, const NormalizedInstruction&);

//////////////////////////////////////////////////////////////////////

/*
 * Before translating/processing each bytecode instruction, the driver
 * of the irgen module calls this function to move to the next
 * bytecode instruction (`newSk') to translate.
 *
 * The flag `lastBcInst' should be set if this is the last bytecode in
 * a region that's being translated.
 */
void prepareForNextHHBC(IRGS&,
                        const NormalizedInstruction*,
                        SrcKey newSk,
                        bool lastBcInst);

/*
 * After translating each bytecode instruction, the driver of the
 * irgen module calls this function to signal that it has finished
 * processing the HHBC instruction.
 */
void finishHHBC(IRGS&);

/*
 * This is called before emitting instructions that can jump to a
 * block corresponding to a control-flow merge point at the bytecode
 * level.
 */
void prepareForHHBCMergePoint(IRGS&);

/*
 * This is called by the region translator to force the stack to be
 * spilled due to a potential side exit.  This is just an
 * optimization, which enables smashing a branch in the main code
 * region.
 */
void prepareForSideExit(IRGS&);

/*
 * When done translating a region, or a block in a region, these calls are
 * made.
 */
void endRegion(IRGS&);
void endRegion(IRGS&, SrcKey);
void endBlock(IRGS&, Offset next, bool nextIsMerge);

/*
 * When we're done creating the IRUnit, this function must be called to ensure
 * all the IR invariants hold.
 */
void sealUnit(IRGS&);

//////////////////////////////////////////////////////////////////////

/*
 * Called when we're starting to inline something.  Returns true iff
 * it succeeds.
 */
bool beginInlining(IRGS&,
                   unsigned numParams,
                   const Func* target,
                   Offset returnBcOffset);

/*
 * Returns whether the IRGS is currently inlining or not.
 */
bool isInlining(const IRGS&);

/*
 * We do two special-case optimizations to partially inline 'singleton'
 * accessor functions (functions that just return a static local or static
 * property if it's not null).
 *
 * This is exposed publically because the region translator drives inlining
 * decisions.
 */
void inlSingletonSProp(IRGS&, const Func*, const Op* clsOp, const Op* propOp);
void inlSingletonSLoc(IRGS&, const Func*, const Op* op);

//////////////////////////////////////////////////////////////////////

/*
 * Several state-inspecition functions are used during translation or tracelet
 * formation.
 */

/*
 * The logical stack depth (from an hhbc perspective) in the current situation.
 */
FPInvOffset logicalStackDepth(const IRGS& env);

/*
 * Access the type of the top of the stack, without making any change to the
 * IRGS.  This means that the type returned is /not/ necessarily constrained.
 */
Type publicTopType(const IRGS& env, BCSPOffset);

/*
 * Returns a predicted Type for the given location, used for tracelet analysis.
 */
Type predictedTypeFromLocation(const IRGS&, const Location&);
Type predictedTypeFromLocal(const IRGS&, uint32_t locId);
Type predictedTypeFromStack(const IRGS&, BCSPOffset slot);

/*
 * Returns the proven Type for the given location.
 */
Type provenTypeFromLocation(const IRGS&, const Location&);
Type provenTypeFromLocal(const IRGS&, uint32_t locId);
Type provenTypeFromStack(const IRGS&, BCSPOffset slot);

/*
 * Assuming env is ready to translate a member instruction, analyze the type of
 * the base value and return an appropriately-specialized TypeConstraint if a
 * supported operation is detected.
 */
TypeConstraint mInstrBaseConstraint(const IRGS& env, Type predicted);

//////////////////////////////////////////////////////////////////////

/*
 * Forward-declare an irgen::emitFoo function for each bytecode Foo.
 *
 * The arguments to the functions are pre-unpacked bytecode immediates.
 */

#define IMM_MA         int /* unused dummy placeholder */
#define IMM_BLA        const ImmVector&
#define IMM_SLA        const ImmVector&
#define IMM_ILA        const ImmVector&
#define IMM_VSA        const ImmVector&
#define IMM_IVA        int32_t
#define IMM_I64A       int64_t
#define IMM_LA         int32_t
#define IMM_IA         int32_t
#define IMM_DA         double
#define IMM_SA         const StringData*
#define IMM_RATA       RepoAuthType
#define IMM_AA         const ArrayData*
#define IMM_BA         Offset
#define IMM_OA(subop)  subop

#define NA /*  */
#define ONE(x0)              , IMM_##x0
#define TWO(x0, x1)          , IMM_##x0, IMM_##x1
#define THREE(x0, x1, x2)    , IMM_##x0, IMM_##x1, IMM_##x2
#define FOUR(x0, x1, x2, x3) , IMM_##x0, IMM_##x1, IMM_##x2, IMM_##x3

#define O(name, imms, ...) void emit##name(IRGS& imms);
  OPCODES
#undef O

#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR

#undef IMM_MA
#undef IMM_BLA
#undef IMM_SLA
#undef IMM_ILA
#undef IMM_VSA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_IA
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

}}

#endif
