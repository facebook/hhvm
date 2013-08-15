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

#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"
#include "hphp/util/util.h"

#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/translator-x64-internal.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/x64-util.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/native-calls.h"

namespace HPHP {
namespace JIT {

namespace {

//////////////////////////////////////////////////////////////////////

using namespace Util;
using namespace Transl;
using namespace Transl::reg;

TRACE_SET_MOD(hhir);

} // unnamed namespace

namespace CodeGenHelpersX64 {

/*
 * It's not normally ok to directly use tracelet abi registers in
 * codegen, unless you're directly dealing with an instruction that
 * does near-end-of-tracelet glue.  (Or also we sometimes use them
 * just for some static_assertions relating to calls to helpers from
 * tx64 that hardcode these registers.)
 */
using Transl::rVmFp;
using Transl::rVmSp;

void emitEagerSyncPoint(Asm& a, const HPHP::Opcode* pc, const Offset spDiff) {
  static COff spOff = offsetof(VMExecutionContext, m_stack) +
    Stack::topOfStackOffset();
  static COff fpOff = offsetof(VMExecutionContext, m_fp);
  static COff pcOff = offsetof(VMExecutionContext, m_pc);

  /* we can't use rAsm because the pc store uses it as a
     temporary */
  Reg64 rEC = reg::rdi;

  a.   push(rEC);
  emitGetGContext(a, rEC);
  a.   storeq(rVmFp, rEC[fpOff]);
  if (spDiff) {
    a.   lea(rVmSp[spDiff], rAsm);
    a.   storeq(rAsm, rEC[spOff]);
  } else {
    a.   storeq(rVmSp, rEC[spOff]);
  }
  a.   storeq(pc, rEC[pcOff]);
  a.   pop(rEC);
}

// emitEagerVMRegSave --
//   Inline. Saves regs in-place in the TC. This is an unusual need;
//   you probably want to lazily save these regs via recordCall and
//   its ilk.
void emitEagerVMRegSave(Asm& a, RegSaveFlags flags) {
  bool saveFP = bool(flags & RegSaveFlags::SaveFP);
  bool savePC = bool(flags & RegSaveFlags::SavePC);
  assert((flags & ~(RegSaveFlags::SavePC | RegSaveFlags::SaveFP)) ==
         RegSaveFlags::None);

  Reg64 pcReg = rdi;
  PhysReg rEC = rAsm;
  assert(!kSpecialCrossTraceRegs.contains(rdi));

  emitGetGContext(a, rEC);

  static COff spOff = offsetof(VMExecutionContext, m_stack) +
    Stack::topOfStackOffset();
  static COff fpOff = offsetof(VMExecutionContext, m_fp) - spOff;
  static COff pcOff = offsetof(VMExecutionContext, m_pc) - spOff;

  assert(spOff != 0);
  a.    addq   (spOff, r64(rEC));
  a.    storeq (rVmSp, *rEC);
  if (savePC) {
    // We're going to temporarily abuse rVmSp to hold the current unit.
    Reg64 rBC = rVmSp;
    a.  push   (rBC);
    // m_fp -> m_func -> m_unit -> m_bc + pcReg
    a.  loadq  (rVmFp[AROFF(m_func)], rBC);
    a.  loadq  (rBC[Func::unitOff()], rBC);
    a.  loadq  (rBC[Unit::bcOff()], rBC);
    a.  addq   (rBC, pcReg);
    a.  storeq (pcReg, rEC[pcOff]);
    a.  pop    (rBC);
  }
  if (saveFP) {
    a.  storeq (rVmFp, rEC[fpOff]);
  }
}

void emitGetGContext(Asm& a, PhysReg dest) {
  emitTLSLoad<ExecutionContext>(a, g_context, dest);
}

// IfCountNotStatic --
//   Emits if (%reg->_count != RefCountStaticValue) { ... }.
//   May short-circuit this check if the type is known to be
//   static already.
struct IfCountNotStatic {
  typedef CondBlock<FAST_REFCOUNT_OFFSET,
                    RefCountStaticValue,
                    CC_Z,
                    field_type(RefData, m_count)> NonStaticCondBlock;
  NonStaticCondBlock *m_cb; // might be null
  IfCountNotStatic(Asm& a,
                   PhysReg reg,
                   DataType t = KindOfInvalid) {

    // Objects and variants cannot be static
    if (t != KindOfObject && t != KindOfResource && t != KindOfRef) {
      m_cb = new NonStaticCondBlock(a, reg);
    } else {
      m_cb = nullptr;
    }
  }

  ~IfCountNotStatic() {
    delete m_cb;
  }
};

void emitIncRef(Asm& as, PhysReg base) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitAssertRefCount(as, base);
  }
  // emit incref
  as.incl(base[FAST_REFCOUNT_OFFSET]);
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    // Assert that the ref count is greater than zero
    emitAssertFlagsNonNegative(as);
  }
}

void emitIncRefCheckNonStatic(Asm& a, PhysReg base, DataType dtype) {
  { // if !static then
    IfCountNotStatic ins(a, base, dtype);
    emitIncRef(a, base);
  } // endif
}

void emitIncRefGenericRegSafe(Asm& a, PhysReg base, int disp, PhysReg tmpReg) {
  { // if RC
    IfRefCounted irc(a, base, disp);
    a.    load_reg64_disp_reg64(base, disp + TVOFF(m_data),
                                tmpReg);
    { // if !static
      IfCountNotStatic ins(a, tmpReg);
      a.  incl(tmpReg[FAST_REFCOUNT_OFFSET]);
    } // endif
  } // endif
}

void emitAssertFlagsNonNegative(Asm& as) {
  ifThen(as, CC_NGE, [&] { as.ud2(); });
}

void emitAssertRefCount(Asm& as, PhysReg base) {
  as.cmpl(HPHP::RefCountStaticValue, base[FAST_REFCOUNT_OFFSET]);
  ifThen(as, CC_NBE, [&] { as.ud2(); });
}

}}}
