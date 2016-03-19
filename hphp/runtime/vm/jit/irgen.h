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
#ifndef incl_HPHP_JIT_IRGEN_H_
#define incl_HPHP_JIT_IRGEN_H_

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/member-key.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
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
struct IRGS;
struct IRInstruction;
struct NormalizedInstruction;
struct SSATmp;

namespace irgen {

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

///////////////////////////////////////////////////////////////////////////////

/*
 * Type checks and assertions.
 *
 * TODO(#5706706): the stack versions should not be exported, except that
 * RegionDesc::Location::Stack needs some fixes first.
 */
void checkType(IRGS&, const RegionDesc::Location&, Type,
               Offset dest, bool outerOnly);
void assertTypeStack(IRGS&, BCSPOffset, Type);
void assertTypeLocal(IRGS&, uint32_t id, Type);
void assertTypeLocation(IRGS&, const RegionDesc::Location&, Type);

/*
 * Type predictions.
 */
void predictType(IRGS&, const RegionDesc::Location&, Type);

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

///////////////////////////////////////////////////////////////////////////////

/*
 * Creates a no-op IR instruction that branches to an exit.
 *
 * These placeholder instructions are later removed after any passes that want
 * to use them for their exits.
 */
void makeExitPlaceholder(IRGS&);

/*
 * Support for translation counters of various types, including reoptimization
 * (CheckCold).
 */
void incTransCounter(IRGS&);
void incProfCounter(IRGS&, TransID);
void checkCold(IRGS&, TransID);

uint64_t curProfCount(const IRGS& env);

/*
 * If ringbuffer tracing is enabled, generate a ringbuffer entry associated
 * with a SrcKey or string.
 */
void ringbufferEntry(IRGS&, Trace::RingBufferType, SrcKey, int level = 1);
void ringbufferMsg(IRGS&, Trace::RingBufferType, const StringData*,
                   int level = 1);

///////////////////////////////////////////////////////////////////////////////

/*
 * For handling PUNT-based interpOnes.  When we PUNT, an exception is thrown
 * and the whole region is retried, with a bit set to interp the instruction
 * that failed.
 */
void interpOne(IRGS&, const NormalizedInstruction&);

///////////////////////////////////////////////////////////////////////////////

/*
 * Before translating/processing each bytecode instruction, the driver
 * of the irgen module calls this function to move to the next
 * bytecode instruction (`newSk') to translate.
 *
 * The flag `lastBcInst' should be set if this is the last bytecode in
 * a region that's being translated.
 */
void prepareForNextHHBC(IRGS&, const NormalizedInstruction*,
                        SrcKey newSk, bool lastBcInst);

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
void endBlock(IRGS&, Offset next);

/*
 * When we're done creating the IRUnit, this function must be called to ensure
 * all the IR invariants hold.
 */
void sealUnit(IRGS&);

///////////////////////////////////////////////////////////////////////////////

/*
 * Called when we're starting to inline something.  Returns true iff
 * it succeeds.
 */
bool beginInlining(IRGS&,
                   unsigned numParams,
                   const Func* target,
                   Offset returnBcOffset,
                   Block* returnTarget,
                   bool multipleReturns);

/*
 * Called when all blocks of the inner most inlined frame have been emitted
 */
void endInlining(IRGS& env);

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
void inlSingletonSProp(IRGS&, const Func*, PC clsOp, PC propOp);
void inlSingletonSLoc(IRGS&, const Func*, PC op);

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
Type publicTopType(const IRGS& env, BCSPOffset);

/*
 * Return the proven or predicted Type for the given location.
 */
Type provenType(const IRGS&, const Location&);
Type predictedType(const IRGS&, const Location&);

///////////////////////////////////////////////////////////////////////////////
/*
 * Forward-declare an irgen::emitFoo function for each bytecode Foo.
 *
 * The arguments to the functions are pre-unpacked bytecode immediates.
 */

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
#define IMM_KA         MemberKey

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
#undef IMM_KA

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
