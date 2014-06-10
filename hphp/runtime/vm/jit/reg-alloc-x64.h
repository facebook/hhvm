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

#ifndef incl_HPHP_VM_REG_ALLOC_X64_H_
#define incl_HPHP_VM_REG_ALLOC_X64_H_

#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/native-calls.h"

namespace HPHP {  namespace JIT {

using NativeCalls::CallMap;

namespace X64 {

// okStore is true if cgStore can take c as an immediate without
// using any scratch registers.
bool okStore(int64_t c) { return true; }

// return true if cgCallHelper and ArgGroup accept c as immediate.
// passing large immediates as args on the stack will implicitly
// clobber rCgGP (R11).
bool okArg(int64_t c) { return true/*isI32(c)*/; }

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
    if (i == 1) return true; // func
    if (i == 2 && type <= Type::Cls) return true;     // objOrCls is Cls
    if (i == 2 && type <= Type::Nullptr) return true; // objOrCls is null
    break;
  case StRetVal:
    if (i == 1) return okStore(cint); // value->cgStore
    break;
  case StaticLocInitCached:
    if (i == 1) return okStore(cint); // value->cgStore
    break;
  case StContArRaw:
    if (i == 2) return okStore(cint); // value->x64-store-imm
    break;
  case StContArValue:
    if (i == 1) return okStore(cint); // value->cgStore
    break;
  case LdGblAddr:
    if (i == 0) return okArg(cint); // name passed to ArgGroup.ssa
    break;
  case StElem:
    if (i == 1) return isI32(cint); // idx used as d32 imm offset
    break;
  case LdElem:
    if (i == 1) return isI32(cint); // byte offset used in MemoryRef
    break;
  case StRef:
    if (i == 1) return okStore(cint);
    break;
  case CallBuiltin:
    return true;
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
  case LdClsPropAddrOrNull:
    if (i == 0) return okArg(cint); // cls -> ArgGroup.ssa().
    if (i == 1) return okArg(cint); // prop -> ArgGroup.ssa().
    break;
  case LdClsPropAddrOrRaise:
    if (i == 0) return okArg(cint); // cls -> ArgGroup.ssa().
    if (i == 1) return okArg(cint); // prop -> ArgGroup.ssa().
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
    if (i == 1) return isI32(cint);
    break;
  case AddInt:
  case AndInt:
  case OrInt:
  case XorInt:
    if (i == 1) return !inst.src(0)->isConst() && isI32(cint); // X op C
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
    if (i == 1) return isI32(cint);
    break;
  case CheckPackedArrayBounds:
    if (i == 1) return isI32(cint); // idx
    break;
  case AKExists:
    // both args are passed to cgCallHelper
    if (i == 0) return okArg(cint);
    if (i == 1) return okArg(cint);
    break;
  case CheckBounds:
    if (i == 0) { // idx
      return !inst.src(1)->isConst() && isI32(cint) && cint >= 0;
    }
    if (i == 1) { // size
      return !inst.src(0)->isConst() && isI32(cint) && cint >= 0;
    }
    break;
  case VerifyParamCls:
  case VerifyRetCls:
    if (i == 1) return isI32(cint); // constraint class ptr
    break;
  case FunctionReturnHook:
    if (i == 1) return okStore(cint); // return value
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
  // MixedArray elements, // Map elements, and RefData inner values.  We don't
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

    case CallBuiltin:
      return srcIdx < inst.numSrcs();

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

}}}

#endif
