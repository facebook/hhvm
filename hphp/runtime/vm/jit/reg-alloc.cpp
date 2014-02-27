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

PhysReg forceAlloc(const SSATmp& tmp) {
  auto inst = tmp.inst();
  auto opc = inst->op();

  // Note that the point of StashGeneratorSP is to save a StkPtr
  // somewhere other than rVmSp.  (TODO(#2288359): make rbx not
  // special.)
  if (opc != StashGeneratorSP && tmp.isA(Type::StkPtr)) {
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
  if (opc != LdContActRec && opc != LdAFWHActRec && tmp.isA(Type::FramePtr)) {
    return arch() == Arch::X64 ? X64::rVmFp : ARM::rVmFp;
  }

  if (opc == DefMIStateBase) {
    assert(tmp.isA(Type::PtrToCell));
    return arch() == Arch::X64 ? PhysReg(reg::rsp) : PhysReg(vixl::sp);
  }
  return InvalidReg;
}

namespace ARM {
Constraint dstConstraint(const IRInstruction& inst, unsigned i) {
  return Constraint::GP;
}

Constraint srcConstraint(const IRInstruction& inst, unsigned i) {
  return Constraint::GP;
}
}

namespace X64 {

/*
 * Return true if this instruction can load a TypedValue using a 16-byte
 * load into a SIMD register.  Note that this function returns
 * false for instructions that load internal meta-data, such as Func*,
 * Class*, etc.
 */
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

/*
 * Returns true if the instruction can store source operand srcIdx to
 * memory as a cell using a 16-byte store.  (implying its okay to
 * clobber TypedValue.m_aux)
 */
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

Constraint srcConstraint(const IRInstruction& inst, unsigned i) {
  Constraint c { Constraint::GP };
  auto src = inst.src(i);
  if (src->type() <= Type::Dbl) {
    c |= Constraint::SIMD;
  } else if (!packed_tv && storesCell(inst, i) && src->numWords() == 2) {
    // don't do this for packed_tv because the type byte is hard to access
    // don't do it for control-flow instructions because we assume the
    // instruction is looking at the type-byte.
    c |= Constraint::SIMD;
  }
  return c;
}

// Return true by default to indicate a result register is required even
// if the instruction's dst is unused.  It's only useful to return false
// for instructions that have a side effect, *and* have the CodeGenerator
// logic to detect no result register and emit better code.
bool needsUnusedReg(const IRInstruction& inst, unsigned dst) {
  // helpers that check for InvalidReg dest:
  // cgCallHelper
  // cgLoad
  // cgLoadTypedValue
  switch (inst.op()) {
  default: return true;

  // These have an explicit check for InvalidReg dst
  case CheckNonNull:

  // These use cgCallHelper, which has an explicit check
  case CallBuiltin:

  // these use cgLdFuncCachedCommon, which checks for no dest
  case LdFuncCached:
  case LdFuncCachedU:
  case LdFuncCachedSafe:

  // These are optional branches, so DCE may not remove them, and use
  // cgLoad, which checks for InvalidReg
  case LdThis:
  case LdRef:
  case LdMem:
     break;
  }
  return false;
}

Constraint dstConstraint(const IRInstruction& inst, unsigned i) {
  Constraint c { Constraint::GP };
  auto dst = inst.dst(i);
  if (dst->type() <= Type::Dbl) {
    c |= Constraint::SIMD;
  } else if (!packed_tv && loadsCell(inst.op()) && !inst.isControlFlow() &&
             dst->numWords() == 2) {
    // we don't do this for packed_tv because the type byte is at offset 1
    // we don't do this for isControlFlow() under the assumption the instruction
    // is some kind of type-check, no such instruction would be peeking into
    // the XMM to examine the type-byte.
    c |= Constraint::SIMD;
  }
  if (!needsUnusedReg(inst, i)) {
    c |= Constraint::VOID;
  }
  return c;
}
}

Constraint srcConstraint(const IRInstruction& inst, unsigned i) {
  auto r = forceAlloc(*inst.src(i));
  if (r != InvalidReg) return r;
  return arch() == Arch::X64 ? X64::srcConstraint(inst, i) :
         ARM::srcConstraint(inst, i);
}

Constraint dstConstraint(const IRInstruction& inst, unsigned i) {
  auto r = forceAlloc(*inst.dst(i));
  if (r != InvalidReg) return r;
  return arch() == Arch::X64 ? X64::dstConstraint(inst, i) :
         ARM::dstConstraint(inst, i);
}

}} // HPHP::JIT
