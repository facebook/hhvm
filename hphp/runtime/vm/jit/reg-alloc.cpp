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
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include <boost/dynamic_bitset.hpp>

namespace HPHP { namespace jit {

using namespace jit::reg;
using NativeCalls::CallMap;

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * Return true if this instruction can load a TypedValue using a 16-byte load
 * into a SIMD register.
 */
bool loadsCell(Opcode op) {
  switch (op) {
  case LdStk:
  case LdLoc:
  case LdMem:
  case LdContField:
  case LdElem:
  case LdRef:
  case LdStaticLocCached:
  case LookupCns:
  case LookupClsCns:
  case CGetProp:
  case VGetProp:
  case ArrayGet:
  case MapGet:
  case CGetElem:
  case VGetElem:
  case ArrayIdx:
  case GenericIdx:
    switch (arch()) {
    case Arch::X64: return true;
    case Arch::ARM: return false;
    }
    not_reached();

  default:
    return false;
  }
}

/*
 * Returns true if the instruction can store source operand srcIdx to
 * memory as a cell using a 16-byte store.  (implying its okay to
 * clobber TypedValue.m_aux)
 */
bool storesCell(const IRInstruction& inst, uint32_t srcIdx) {
  switch (arch()) {
  case Arch::X64: break;
  case Arch::ARM: return false;
  }

  // If this function returns true for an operand, then the register allocator
  // may give it an XMM register, and the instruction will store the whole 16
  // bytes into memory.  Therefore it's important *not* to return true if the
  // TypedValue.m_aux field in memory has important data.  This is the case for
  // MixedArray elements, Map elements, and RefData inner values.  We don't
  // have StMem in here since it sometimes stores to RefDatas.
  switch (inst.op()) {
  case StRetVal:
  case StLoc:
    return srcIdx == 1;

  case StElem:
    return srcIdx == 2;

  case StStk:
    return srcIdx == 1;

  case CallBuiltin:
    return srcIdx < inst.numSrcs();

  default:
    return false;
  }
}

}

//////////////////////////////////////////////////////////////////////

PhysReg forceAlloc(const SSATmp& tmp) {
  if (tmp.type() <= TBottom) return InvalidReg;

  auto inst = tmp.inst();
  auto opc = inst->op();

  auto const forceStkPtrs = [&] {
    switch (arch()) {
    case Arch::X64: return false;
    case Arch::ARM: return true;
    }
    not_reached();
  }();

  if (forceStkPtrs && tmp.isA(TStkPtr)) {
    assert_flog(
      opc == DefSP ||
      opc == Mov,
      "unexpected StkPtr dest from {}",
      opcodeName(opc)
    );
    return mcg->backEnd().rVmSp();
  }

  // LdContActRec and LdAFWHActRec, loading a generator's AR, is the only time
  // we have a pointer to an AR that is not in rVmFp.
  if (opc != LdContActRec && opc != LdAFWHActRec && tmp.isA(TFramePtr)) {
    return mcg->backEnd().rVmFp();
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
                const BlockList& blocks) {
  // visit instructions to find tmps eligible to use SIMD registers
  auto const try_wide = RuntimeOption::EvalHHIRAllocSIMDRegs;
  boost::dynamic_bitset<> not_wide(unit.numTmps());
  StateVector<SSATmp,SSATmp*> tmps(unit, nullptr);
  for (auto block : blocks) {
    for (auto& inst : *block) {
      for (uint32_t i = 0, n = inst.numSrcs(); i < n; i++) {
        auto s = inst.src(i);
        tmps[s] = s;
        if (!try_wide || !storesCell(inst, i)) {
          not_wide.set(s->id());
        }
      }
      for (auto& d : inst.dsts()) {
        tmps[d] = d;
        if (!try_wide || inst.isControlFlow() || !loadsCell(inst.op())) {
          not_wide.set(d->id());
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
      auto const type = tmp->type();
      Vreg c;
      if (type.subtypeOfAny(TNull, TNullptr)) {
        c = vunit.makeConst(0);
      } else if (type <= TBool) {
        c = vunit.makeConst(tmp->boolVal());
      } else if (type <= TDbl) {
        c = vunit.makeConst(tmp->dblVal());
      } else {
        c = vunit.makeConst(tmp->rawVal());
      }
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

void getEffects(const Abi& abi, const Vinstr& i,
                RegSet& uses, RegSet& across, RegSet& defs) {
  uses = defs = across = RegSet();
  switch (i.op) {
    case Vinstr::mccall:
    case Vinstr::call:
    case Vinstr::callm:
    case Vinstr::callr:
      defs = abi.all() - abi.calleeSaved;
      switch (arch()) {
        case Arch::ARM:
          defs.add(PhysReg(arm::rLinkReg));
          defs.remove(PhysReg(arm::rVmFp));
          break;
        case Arch::X64:
          defs.remove(reg::rbp);
          break;
      }
      break;
    case Vinstr::bindcall:
      defs = abi.all();
      switch (arch()) {
      case Arch::ARM: break;
      case Arch::X64: defs.remove(x64::rVmTl); break;
      }
      break;
    case Vinstr::contenter:
    case Vinstr::callstub:
      defs = abi.all();
      switch (arch()) {
      case Arch::ARM: defs.remove(PhysReg(arm::rVmFp)); break;
      case Arch::X64: defs -= reg::rbp | x64::rVmTl; break;
      }
      break;
    case Vinstr::callfaststub:
      defs = abi.all() - abi.calleeSaved - x64::kGPCallerSaved;
      break;
    case Vinstr::cqo:
      uses = RegSet(reg::rax);
      defs = reg::rax | reg::rdx;
      break;
    case Vinstr::idiv:
      uses = defs = reg::rax | reg::rdx;
      break;
    case Vinstr::shlq:
    case Vinstr::sarq:
      across = RegSet(reg::rcx);
      break;
    // arm instrs
    case Vinstr::hostcall:
      defs = (abi.all() - abi.calleeSaved) |
             RegSet(PhysReg(arm::rHostCallReg));
      break;
    case Vinstr::vcall:
    case Vinstr::vinvoke:
    case Vinstr::vcallstub:
      always_assert(false && "Unsupported instruction in vxls");
    default:
      break;
  }
}

//////////////////////////////////////////////////////////////////////

}}
