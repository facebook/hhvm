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
#ifndef incl_HPHP_JIT_CODE_GEN_HELPERS_ARM_H
#define incl_HPHP_JIT_CODE_GEN_HELPERS_ARM_H

#include "hphp/vixl/a64/macro-assembler-a64.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace JIT { namespace ARM {

/*
 * Intelligently chooses between Add, Mov, and no-op.
 */
void emitRegGetsRegPlusImm(vixl::MacroAssembler& as,
                           const vixl::Register& dstReg,
                           const vixl::Register& srcReg,
                           int64_t imm);

/*
 * The callee's half of within-TC calls. The caller uses a native call
 * instruction to get to either the bind-call stub (which needs to know the
 * address to smash) or the eventual destination (which needs to put the return
 * address into the ActRec). This fulfills both needs.
 */
void emitStoreRetIntoActRec(vixl::MacroAssembler& a);

/*
 * All calls should go through here, because they need to be implemented
 * differently depending on whether we're simulating ARM or running native.
 * Returns the address at which to record a fixup, if you need to.
 */
JIT::TCA emitCall(vixl::MacroAssembler& a, CppCall call);

/*
 * Swaps two registers. Uses XOR swap, so will not touch memory, flags, or any
 * other registers. XOR swap is the best.
 */
void emitXorSwap(vixl::MacroAssembler& a,
                 const vixl::Register& r1, const vixl::Register& r2);

/*
 * Check the surprise flags. If surprised, call functionEnterHelper.
 */
void emitCheckSurpriseFlagsEnter(CodeBlock& mainCode, CodeBlock& stubsCode,
                                 bool inTracelet, JIT::FixupMap& fixupMap,
                                 JIT::Fixup fixup);

/*
 * Increments the current (at translation time) translation counter.
 */
void emitTransCounterInc(vixl::MacroAssembler& a);

/*
 * Emits an incref after checking only the static bit, not the type.
 */
void emitIncRefKnownType(vixl::MacroAssembler& a,
                         const vixl::Register& dataReg,
                         size_t disp);
/*
 * The most generic form of incref. Given a register+offset indicating a pointer
 * to a TypedValue, it checks the type for refcounted-ness and the inner
 * object's count for static-ness before doing the incref.
 */
void emitIncRefGeneric(vixl::MacroAssembler& a,
                       const vixl::Register& data,
                       size_t disp);

/*
 * Enables access to objects with the __thread storage class. See the
 * explanatory comment in code-gen-helpers-x64.h for how thread-local storage
 * works.
 */
#ifdef USE_GCC_FAST_TLS
template<typename T>
inline void emitTLSLoad(vixl::MacroAssembler& a,
                        const ThreadLocalNoCheck<T>& datum,
                        const vixl::Register& destReg) {
  using namespace vixl;
  a.   Mov  (rHostCallReg, JIT::tlsBaseNoInline);
  a.   Push (x30, x29);
  a.   HostCall(0);
  // tlsBaseNoInline doesn't need a sync point
  a.   Pop  (x29, x30);

  a.   Add  (rReturnReg, rReturnReg,
             uintptr_t(&datum.m_node.m_p) - JIT::tlsBase());
  // Now rReturnReg holds a pointer to *a pointer to* the object.
  a.   Ldr  (destReg, rReturnReg[0]);
}


#else
template<typename T>
inline void emitTLSLoad(vixl::MacroAssembler& a,
                        const ThreadLocalNoCheck<T>& datum,
                        const vixl::Register& destReg) {
  // ARM-simulation mode isn't supported yet if FAST_TLS is off.
  not_implemented();
}
#endif // USE_GCC_FAST_TLS

}}}

#endif
