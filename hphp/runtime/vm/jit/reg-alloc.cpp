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
#include "hphp/runtime/vm/jit/native-calls.h"

namespace HPHP {
namespace JIT{

using namespace JIT::reg;
using NativeCalls::CallMap;

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
    switch (arch()) {
      case Arch::X64:
        return X64::rVmSp;
      case Arch::ARM:
        return ARM::rVmSp;
    }
    not_reached();
  }

  // LdContActRec and LdAFWHActRec, loading a generator's AR, is the only time
  // we have a pointer to an AR that is not in rVmFp.
  if (opc != LdContActRec && opc != LdAFWHActRec && tmp.isA(Type::FramePtr)) {
    switch (arch()) {
      case Arch::X64:
        return X64::rVmFp;
      case Arch::ARM:
        return ARM::rVmFp;
    }
    not_reached();
  }

  if (opc == DefMIStateBase) {
    assert(tmp.isA(Type::PtrToCell));
    switch (arch()) {
      case Arch::X64:
        return PhysReg(reg::rsp);
      case Arch::ARM:
        return PhysReg(vixl::sp);
    }
    not_reached();
  }
  return InvalidReg;
}

namespace {
// This implements an array of arrays of bools, one for each declared
// source operand of each instruction.  True means the operand must
// be a const; i.e. it was declared with C(T) instead of S(T).
struct ConstSrcTable {
  auto static constexpr MaxSrc = 8;
  bool table[kNumOpcodes][MaxSrc];
  ConstSrcTable() {
    int op = 0;
    int i;

#define NA
#define S(...)   i++;
#define C(type)  table[op][i++] = true;
#define CStr     table[op][i++] = true;
#define SNumInt  i++;
#define SNum     i++;
#define SUnk     i++;
#define SSpills
#define O(opcode, dstinfo, srcinfo, flags) \
    i = 0; \
    srcinfo \
    op++;

    IR_OPCODES

#undef O
#undef NA
#undef SAny
#undef S
#undef C
#undef CStr
#undef SNum
#undef SUnk
#undef SSpills

  }
  bool mustBeConst(int op, int i) const {
    return i < MaxSrc ? table[op][i] : false;
  }
};
const ConstSrcTable g_const_table;

// Return true if the ith source operand must be a constant.  Most
// of this information comes from the table above, but a few instructions
// have complex signatures, so we handle them individually.
bool mustUseConst(const IRInstruction& inst, int i) {
  auto check = [&](bool b) {
    assert(!b || inst.src(i)->isConst());
    return b;
  };
  // handle special cases we can't derive from IR_OPCODES macro
  switch (inst.op()) {
  case LdAddr: return check(i == 1); // offset
  case Call: return check(i == 1); // returnBcOffset
  case CallBuiltin: return check(i == 0); // f
  default: break;
  }
  return check(g_const_table.mustBeConst(int(inst.op()), i));
}
}

bool isI32(int64_t c) { return c == int32_t(c); }
bool isU32(int64_t c) { return c == uint32_t(c); }
namespace ARM {

// Return true if the CodeGenerator method for this instruction can
// handle an immediate for the ith source operand, usually by selecting
// a special form of the necessary instruction.  The value of the immediate
// can affect this decision; we look at the value here, and trust it
// blindly in CodeGenerator.
bool mayUseConst(const IRInstruction& inst, unsigned i) {
  assert(inst.src(i)->isConst());
  union {
    int64_t cint;
    double cdouble;
  };
  auto type = inst.src(i)->type();
  cint = type.hasRawVal() ? type.rawVal() : 0;
  // (almost?) any instruction that accepts a GPR, can accept XZR in
  // place of an immediate zero. TODO #3827905
  switch (inst.op()) {
  case GuardRefs:
    if (i == 1) return inst.src(2)->intVal() == 0; // nParams
    if (i == 3) { // mask64
      return vixl::Assembler::IsImmLogical(cint, vixl::kXRegSize);
    }
    if (i == 4) { // vals64
      return vixl::Assembler::IsImmArithmetic(cint);
    }
    break;
  case AddInt:
  case SubInt:
  case EqInt:
  case NeqInt:
  case LtInt:
  case GtInt:
  case LteInt:
  case GteInt:
    if (i == 1) {
      return vixl::Assembler::IsImmArithmetic(cint);
    }
    break;

  //TODO: t3944093 add constraints for existing arm codegen
  default:
    break;
  }
  if (CallMap::hasInfo(inst.op())) {
    // shuffleArgs() knows what to do with immediates.
    // TODO: #3634984 ... but it needs a scratch register
    return true;
  }
  return false;
}

Constraint srcConstraint(const IRInstruction& inst, unsigned i) {
  Constraint c { Constraint::GP };
  if (inst.src(i)->isConst() && mayUseConst(inst, i)) {
    c |= Constraint::IMM;
  }
  return c;
}

Constraint dstConstraint(const IRInstruction& inst, unsigned i) {
  return Constraint::GP;
}

}

namespace X64 {

// okStore is true if cgStore can take c as an immediate without
// using any scratch registers.
bool okStore(int64_t c) { return true; }

bool okCmp(int64_t c) { return isI32(c); }

// return true if CodeGenerator supports this operand as an
// immediate value.
// pre: the src must actually be a const
bool mayUseConst(const IRInstruction& inst, unsigned i) {
  assert(inst.src(i)->isConst());
  union {
    int64_t cint;
    double cdouble;
  };
  auto type = inst.src(i)->type();
  cint = type.hasRawVal() ? type.rawVal() : 0;
  switch (inst.op()) {
  case GuardRefs:
    if (i == 1) return inst.src(2)->intVal() == 0; // nParams
    if (i == 3) return isU32(cint); // mask64
    if (i == 4) return isU32(cint); // vals64
    break;
  case LdPackedArrayElem:
  case CheckPackedArrayElemNull:
    if (i == 1) return true; // idx; can use reg+imm addressing mode
    break;
  case SpillStack:
    if (i >= 2) return okStore(cint);
    break;
  case SpillFrame:
    if (i == 2) return true; // func
    if (i == 3) return type <= Type::Cls; // objOrCls
    break;
  case Call:
    if (i == 2) return true; // func
    if (i >= 3) return okStore(cint);
    break;
  case CallBuiltin:
    if (i >= 2) return true; // args -> ArgGroup.ssa()
    break;
  case StLoc:
  case StLocNT:
    if (i == 1) return okStore(cint); // value
    break;
  case StProp:
  case StMem:
    if (i == 2) return okStore(cint); // value
    break;
  case StRaw:
    if (i == 2) return okStore(cint); // value
    break;
  case Jmp:
  case Shuffle:
    // doRegMoves handles immediates.
    // TODO: #3634984 ... but it needs a scratch register
    return true;
  case LdClsPropAddr:
  case LdClsPropAddrCached:
    if (i == 0) return true; // cls -> ArgGroup.ssa().
    break;
  case Same: case NSame:
  case Eq:   case EqX:
  case Neq:  case NeqX:
  case Lt:   case LtX:
  case Gt:   case GtX:
  case Lte:  case LteX:
  case Gte:  case GteX:
    if (i == 1) {
      // cases in cgCmpHelper()
      auto type0 = inst.src(0)->type();
      if (type0 <= Type::Str && type <= Type::Str) return true; // call
      if (type0 <= Type::Bool && type <= Type::Bool) return true;
      if (type0 <= Type::Obj && type <= Type::Int) return true;
      if (type0 <= Type::Arr && type <= Type::Arr) return true;
    }
    break;
  case JmpEq:  case SideExitJmpEq:  case ReqBindJmpEq:
  case JmpNeq: case SideExitJmpNeq: case ReqBindJmpNeq:
  case JmpGt:  case SideExitJmpGt:  case ReqBindJmpGt:
  case JmpGte: case SideExitJmpGte: case ReqBindJmpGte:
  case JmpLt:  case SideExitJmpLt:  case ReqBindJmpLt:
  case JmpLte: case SideExitJmpLte: case ReqBindJmpLte:
    if (i == 1) {
      // cases in emitCompare()
      auto type0 = inst.src(0)->type();
      if (type0 <= Type::Bool && type <= Type::Bool) return true;
      if (type0 <= Type::Cls && type <= Type::Cls) return isI32(cint);
    }
    break;
  case EqInt:
  case NeqInt:
  case LtInt:
  case GtInt:
  case LteInt:
  case GteInt:
  case JmpEqInt:  case SideExitJmpEqInt:  case ReqBindJmpEqInt:
  case JmpNeqInt: case SideExitJmpNeqInt: case ReqBindJmpNeqInt:
  case JmpGtInt:  case SideExitJmpGtInt:  case ReqBindJmpGtInt:
  case JmpGteInt: case SideExitJmpGteInt: case ReqBindJmpGteInt:
  case JmpLtInt:  case SideExitJmpLtInt:  case ReqBindJmpLtInt:
  case JmpLteInt: case SideExitJmpLteInt: case ReqBindJmpLteInt:
    // cases in emitCompareInt()
    if (i == 1) return okCmp(cint);
    break;
  case SubInt:
    if (i == 0) return cint == 0 && !inst.src(1)->isConst(); // 0-X
    if (i == 1) return !inst.src(0)->isConst() && isI32(cint); // X-C
    break;
  case Shl: case Shr:
    if (i == 1) return true; // shift amount
    break;
  case XorBool:
    if (i == 1) return isI32(cint);
    break;
  case Mov:
    return true; // x64 can mov a 64-bit imm into a register.
  case StringIsset:
    if (i == 1) return okCmp(cint);
    break;
  case CheckPackedArrayBounds:
    if (i == 1) return okCmp(cint); // idx
    break;
  case CheckBounds:
    if (i == 0) { // idx
      return !inst.src(1)->isConst() && okCmp(cint) && cint >= 0;
    }
    if (i == 1) { // size
      return !inst.src(0)->isConst() && okCmp(cint) && cint >= 0;
    }
    break;
  case VerifyParamCls:
  case VerifyRetCls:
    if (i == 1) return okCmp(cint); // constraint class ptr
    break;
  case FunctionExitSurpriseHook:
    if (i == 2) return okStore(cint); // return value
    break;
  default:
    break;
  }
  if (CallMap::hasInfo(inst.op())) {
    // shuffleArgs() knows what to do with immediates.
    // TODO: #3634984 ... but it needs a scratch register for
    // big constants, so handle big immediates here.
    return true;
  }
  return false;
}

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
  if (src->isConst() && mayUseConst(inst, i)) {
    c |= Constraint::IMM;
  }
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
  if (mustUseConst(inst, i)) return Constraint::IMM;
  switch (arch()) {
    case Arch::X64:
      return X64::srcConstraint(inst, i);
    case Arch::ARM:
      return ARM::srcConstraint(inst, i);
  }
  not_reached();
}

Constraint dstConstraint(const IRInstruction& inst, unsigned i) {
  auto r = forceAlloc(*inst.dst(i));
  if (r != InvalidReg) return r;
  switch (arch()) {
    case Arch::X64:
      return X64::dstConstraint(inst, i);
    case Arch::ARM:
      return ARM::dstConstraint(inst, i);
  }
  not_reached();
}

}}
