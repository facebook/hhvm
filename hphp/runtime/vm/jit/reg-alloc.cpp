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

#include "hphp/runtime/vm/jit/reg-alloc.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"

namespace HPHP { namespace jit {

using namespace jit::reg;
using NativeCalls::CallMap;

TRACE_SET_MOD(hhir);

PhysReg forceAlloc(const SSATmp& tmp) {
  auto inst = tmp.inst();
  auto opc = inst->op();

  // TODO(t5485866) Our manipulations to vmsp must be SSA to play nice with
  // LLVM. In the X64 backend, this causes enough extra reg-reg copies to
  // measurably impact performance, so keep forcing things into rVmSp for
  // now. We should be able to remove this completely once the necessary
  // improvements are made to vxls.
  auto const forceStkPtrs = arch() != Arch::X64 || !RuntimeOption::EvalJitLLVM;

  if (forceStkPtrs && tmp.isA(Type::StkPtr)) {
    assert_flog(
      opc == DefSP ||
      opc == ReDefSP ||
      opc == Call ||
      opc == CallArray ||
      opc == ContEnter ||
      opc == SpillStack ||
      opc == SpillFrame ||
      opc == CufIterSpillFrame ||
      opc == ExceptionBarrier ||
      opc == RetAdjustStack ||
      opc == InterpOne ||
      opc == InterpOneCF ||
      opc == Mov ||
      opc == CheckStk ||
      opc == GuardStk ||
      opc == AssertStk ||
      opc == CastStk ||
      opc == CastStkIntToDbl ||
      opc == CoerceStk ||
      opc == SideExitGuardStk  ||
      opc == DefLabel ||
      opc == HintStkInner ||
      MInstrEffects::supported(opc),
      "unexpected StkPtr dest from {}",
      opcodeName(opc)
    );
    return mcg->backEnd().rVmSp();
  }

  // LdContActRec and LdAFWHActRec, loading a generator's AR, is the only time
  // we have a pointer to an AR that is not in rVmFp.
  if (opc != LdContActRec && opc != LdAFWHActRec && tmp.isA(Type::FramePtr)) {
    return mcg->backEnd().rVmFp();
  }

  if (opc == DefMIStateBase) {
    assert(tmp.isA(Type::PtrToGen));
    return mcg->backEnd().rVmTl();
  }
  return InvalidReg;
}

// Assign virtual registers to all SSATmps used or defined in reachable
// blocks. This assigns a value register to constants defined by DefConst,
// because some HHIR instructions require them. Ordinary Gen values with
// a known DataType only get one register. Assign "wide" locations when
// possible (when all uses and defs can be wide). These will be assigned
// SIMD registers later.
void assignRegs(IRUnit& unit, Vunit& vunit, CodegenState& state,
                const BlockList& blocks, BackEnd* backend) {
  // visit instructions to find tmps eligible to use SIMD registers
  auto const try_wide = !packed_tv && RuntimeOption::EvalHHIRAllocSIMDRegs;
  boost::dynamic_bitset<> not_wide(unit.numTmps());
  StateVector<SSATmp,SSATmp*> tmps(unit, nullptr);
  for (auto block : blocks) {
    for (auto& inst : *block) {
      for (uint32_t i = 0, n = inst.numSrcs(); i < n; i++) {
        auto s = inst.src(i);
        tmps[s] = s;
        if (!try_wide || !backend->storesCell(inst, i)) {
          not_wide.set(s->id());
        }
      }
      for (auto& d : inst.dsts()) {
        tmps[&d] = &d;
        if (!try_wide || inst.isControlFlow() || !backend->loadsCell(inst)) {
          not_wide.set(d.id());
        }
      }
    }
  }
  // visit each tmp, assign 1 or 2 registers to each.
  for (auto tmp : tmps) {
    if (!tmp) continue;
    auto forced = forceAlloc(*tmp);
    if (forced != InvalidReg) {
      state.locs[tmp] = Vloc{forced};
      UNUSED Reg64 r = forced;
      FTRACE(kRegAllocLevel, "force t{} in {}\n", tmp->id(), reg::regname(r));
      continue;
    }
    if (tmp->inst()->is(DefConst)) {
      auto c = tmp->isA(Type::Bool) ? vunit.makeConst(tmp->boolVal())
                                    : vunit.makeConst(tmp->rawVal());
      state.locs[tmp] = Vloc{c};
      FTRACE(kRegAllocLevel, "const t{} in %{}\n", tmp->id(), size_t(c));
    } else {
      if (tmp->numWords() == 2) {
        if (!not_wide.test(tmp->id())) {
          auto r = vunit.makeReg();
          state.locs[tmp] = Vloc{Vloc::kWide, r};
          FTRACE(kRegAllocLevel, "def t{} in wide %{}\n", tmp->id(), size_t(r));
        } else {
          auto data = vunit.makeReg();
          auto type = vunit.makeReg();
          state.locs[tmp] = Vloc{data, type};
          FTRACE(kRegAllocLevel, "def t{} in %{},%{}\n", tmp->id(),
                 size_t(data), size_t(type));
        }
      } else {
        auto data = vunit.makeReg();
        state.locs[tmp] = Vloc{data};
        FTRACE(kRegAllocLevel, "def t{} in %{}\n", tmp->id(), size_t(data));
      }
    }
  }
}

}}
