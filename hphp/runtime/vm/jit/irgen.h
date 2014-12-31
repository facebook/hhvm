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
 * To use this code, the client creates an HTS structure, and calls these
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
namespace detail { SSATmp* genInstruction(HTS& env, IRInstruction*); }
template<class... Args>
SSATmp* gen(HTS& env, Opcode op, Args&&... args) {
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
SSATmp* cns(HTS& env, Args&&... args) {
  return env.unit.cns(std::forward<Args>(args)...);
}

/*
 * Guards, checks and type assertions.  Guards and checks are the same thing,
 * except that guards must happen at the first bytecode offset in the region.
 *
 * TODO(#5706706): the stack versions should not be exported, except that
 * RegionDesc::Location::Stack needs some fixes first.
 */
void assertTypeStack(HTS&, uint32_t idx, Type);
void checkTypeStack(HTS&, uint32_t idx, Type, Offset dest);
void assertTypeLocation(HTS&, const RegionDesc::Location&, Type);
void checkTypeLocation(HTS&, const RegionDesc::Location&, Type, Offset dest);
void guardTypeLocation(HTS&, const RegionDesc::Location& loc, Type,
  bool outerOnly);

/*
 * Special type of guards for param-passing reffiness.  These guards/checks are
 * needed when an FPush* instruction is in a different region from its FCall,
 * and we don't know statically whether the callee will want arguments by
 * reference.
 */
void guardRefs(HTS&, int64_t entryArDelta, const std::vector<bool>& mask,
  const std::vector<bool>& vals);
void checkRefs(HTS&, int64_t entryArDelta, const std::vector<bool>& mask,
  const std::vector<bool>& vals, Offset);

/*
 * After all initial guards instructions have been emitted, the client of this
 * module calls the following function to allow some "region header" code to be
 * emitted.
 */
void prepareEntry(HTS&);

//////////////////////////////////////////////////////////////////////

/*
 * Support for translation counters of various types, including reoptimization
 * (CheckCold).
 */
void incTransCounter(HTS&);
void incProfCounter(HTS&, TransID);
void checkCold(HTS&, TransID);

/*
 * Generate a ringbuffer update for executing a particular SrcKey.
 */
void ringbuffer(HTS&, Trace::RingBufferType t, SrcKey sk, int level = 1);

//////////////////////////////////////////////////////////////////////

/*
 * For handling PUNT-based interpOnes.  When we PUNT, an exception is thrown
 * and the whole region is retried, with a bit set to interp the instruction
 * that failed.
 */
void interpOne(HTS&, const NormalizedInstruction&);

//////////////////////////////////////////////////////////////////////

/*
 * After each bytecode that is translated/processed, the driver of the ht
 * module calls this function to move to the next bytecode offset (`newOff') to
 * translate.
 *
 * The flag `lastBcOff' should be set if this is the last bytecode in a region
 * that's being translated.
 */
void prepareForNextHHBC(HTS&,
                        const NormalizedInstruction*,
                        Offset newOff,
                        bool lastBcOff);

/*
 * This function causes ht to generate a SpillStack, and is called for
 * presumably good reasons by the region translator, in situations that differ
 * depending on IRGenMode.
 */
void prepareForSideExit(HTS&);

/*
 * When done translating a region, or a block in a region, these calls are
 * made.
 */
void endRegion(HTS&);
void endRegion(HTS&, Offset);
void endBlock(HTS&, Offset next, bool nextIsMerge);

//////////////////////////////////////////////////////////////////////

/*
 * Called when we're starting to inline something.
 */
void beginInlining(HTS&,
                   unsigned numParams,
                   const Func* target,
                   Offset returnBcOffset);

/*
 * Returns whether the HTS is currently inlining or not.
 */
bool isInlining(const HTS&);

/*
 * We do two special-case optimizations to partially inline 'singleton'
 * accessor functions (functions that just return a static local or static
 * property if it's not null).
 *
 * This is exposed publically because the region translator drives inlining
 * decisions.
 */
void inlSingletonSProp(HTS&, const Func*, const Op* clsOp, const Op* propOp);
void inlSingletonSLoc(HTS&, const Func*, const Op* op);

//////////////////////////////////////////////////////////////////////

/*
 * Several state-inspecition functions are used during translation or tracelet
 * formation.
 */

/*
 * The logical stack offset in the current situation.
 */
size_t spOffset(const HTS& env);

/*
 * Access the type of the top of the stack, without making any change to the
 * HTS.  This means that the type returned is /not/ necessarily constrained.
 */
Type publicTopType(const HTS& env, int32_t);

/*
 * Returns a predicted Type for the given location, used for tracelet analysis.
 */
Type predictedTypeFromLocation(HTS&, const Location&);

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

#define O(name, imms, ...) void emit##name(HTS& imms);
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
