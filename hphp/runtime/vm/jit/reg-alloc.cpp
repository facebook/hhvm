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
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/abi-arm.h"

namespace HPHP {
namespace JIT{

using namespace JIT::reg;

TRACE_SET_MOD(hhir);

PhysReg forceAlloc(SSATmp& dst) {
  auto inst = dst.inst();
  auto opc = inst->op();

  // Note that the point of StashGeneratorSP is to save a StkPtr
  // somewhere other than rVmSp.  (TODO(#2288359): make rbx not
  // special.)
  bool abnormalStkPtr = opc == StashGeneratorSP;

  if (!abnormalStkPtr && dst.isA(Type::StkPtr)) {
    assert(opc == DefSP ||
           opc == ReDefSP ||
           opc == ReDefGeneratorSP ||
           opc == PassSP ||
           opc == DefInlineSP ||
           opc == Call ||
           opc == CallArray ||
           opc == SpillStack ||
           opc == SpillFrame ||
           opc == CufIterSpillFrame ||
           opc == ExceptionBarrier ||
           opc == RetAdjustStack ||
           opc == InterpOne ||
           opc == InterpOneCF ||
           opc == GenericRetDecRefs ||
           opc == CheckStk ||
           opc == GuardStk ||
           opc == AssertStk ||
           opc == CastStk ||
           opc == CoerceStk ||
           opc == SideExitGuardStk  ||
           MInstrEffects::supported(opc));
    return arch() == Arch::X64 ? X64::rVmSp : ARM::rVmSp;
  }

  // LdContActRec and LdAFWHActRec, loading a generator's AR, is the only time
  // we have a pointer to an AR that is not in rVmFp.
  bool abnormalFramePtr = opc == LdContActRec || opc == LdAFWHActRec;

  if (!abnormalFramePtr && dst.isA(Type::FramePtr)) {
    return arch() == Arch::X64 ? X64::rVmFp : ARM::rVmFp;
  }

  if (opc == DefMIStateBase) {
    assert(dst.isA(Type::PtrToCell));
    return arch() == Arch::X64 ? PhysReg(reg::rsp) : PhysReg(vixl::sp);
  }
  return InvalidReg;
}

bool loadsCell(Opcode op) {
  switch (op) {
    case LdStack:
    case LdLoc:
    case LdMem:
    case LdProp:
    case LdElem:
    case LdPackedArrayElem:
    case LdRef:
    case LdThis:
    case LdStaticLocCached:
    case LookupCns:
    case LookupClsCns:
    case CGetProp:
    case VGetProp:
    case VGetPropStk:
    case ArrayGet:
    case MapGet:
    case CGetElem:
    case VGetElem:
    case VGetElemStk:
    case ArrayIdx:
    case GenericIdx:
      return true;

    default:
      return false;
  }
}

bool storesCell(const IRInstruction& inst, uint32_t srcIdx) {
  // If this function returns true for an operand, then the register allocator
  // may give it an XMM register, and the instruction will store the whole 16
  // bytes into memory.  Therefore it's important *not* to return true if the
  // TypedValue.m_aux field in memory has important data.  This is the case for
  // HphpArray elements, // Map elements, and RefData inner values.  We don't
  // have StMem in here since it sometimes stores to RefDatas.
  switch (inst.op()) {
    case StRetVal:
    case StLoc:
    case StLocNT:
      return srcIdx == 1;

    case StProp:
    case StElem:
      return srcIdx == 2;

    case ArraySet:
    case MapSet:
      return srcIdx == 3;

    case SpillStack:
      return srcIdx >= 2 && srcIdx < inst.numSrcs();

    case Call:
      return srcIdx >= 3 && srcIdx < inst.numSrcs();

    case CallBuiltin:
      return srcIdx >= 1 && srcIdx < inst.numSrcs();

    case FunctionExitSurpriseHook:
      return srcIdx == 2;

    default:
      return false;
  }
}

}} // HPHP::JIT
