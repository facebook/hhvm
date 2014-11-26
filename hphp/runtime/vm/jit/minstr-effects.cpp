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
#include "hphp/runtime/vm/jit/minstr-effects.h"

#include "hphp/runtime/vm/jit/local-effects.h"
#include "hphp/runtime/vm/jit/frame-state.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

Opcode canonicalOp(Opcode op) {
  if (op == ElemUX || op == ElemUXStk ||
      op == UnsetElem || op == UnsetElemStk) {
    return UnsetElem;
  }
  if (op == SetWithRefElem ||
      op == SetWithRefElemStk ||
      op == SetWithRefNewElem ||
      op == SetWithRefNewElemStk) {
    return SetWithRefElem;
  }
  return opcodeHasFlags(op, MInstrProp) ? SetProp
       : opcodeHasFlags(op, MInstrElem) ? SetElem
       : bad_value<Opcode>();
}

void getBaseType(Opcode rawOp, bool predict,
                 Type& baseType, bool& baseValChanged) {
  always_assert(baseType.notBoxed());
  auto const op = canonicalOp(rawOp);

  // Deal with possible promotion to stdClass or array
  if ((op == SetElem || op == SetProp || op == SetWithRefElem) &&
      baseType.maybe(Type::Null | Type::Bool | Type::Str)) {
    auto newBase = op == SetProp ? Type::Obj : Type::Arr;

    if (predict) {
      /* If the output type will be used as a prediction and not as fact, we
       * can be optimistic here. Assume no promotion for string bases and
       * promotion in other cases. */
      baseType = baseType <= Type::Str ? Type::Str : newBase;
    } else if (baseType <= Type::Str &&
               (rawOp == SetElem || rawOp == SetElemStk)) {
      /* If the base is known to be a string and the operation is exactly
       * SetElem, we're guaranteed that either the base will end as a
       * CountedStr or the instruction will throw an exception and side
       * exit. */
      baseType = Type::CountedStr;
    } else if (baseType <= Type::Str &&
               (rawOp == SetNewElem || rawOp == SetNewElemStk)) {
      /* If the string base is empty, it will be promoted to an
       * array. Otherwise the base will be left alone and we'll fatal. */
      baseType = Type::Arr;
    } else {
      /* Regardless of whether or not promotion happens, we know the base
       * cannot be Null after the operation. If the base was a subtype of Null
       * this will give newBase. */
      baseType = (baseType - Type::Null) | newBase;
    }

    baseValChanged = true;
  }

  if ((op == SetElem || op == UnsetElem || op == SetWithRefElem) &&
      baseType.maybe(Type::Arr | Type::Str)) {
    /* Modifying an array or string element, even when COW doesn't kick in,
     * produces a new SSATmp for the base. StaticArr/StaticStr may be promoted
     * to CountedArr/CountedStr. */
    baseValChanged = true;
    if (baseType.maybe(Type::StaticArr)) baseType |= Type::CountedArr;
    if (baseType.maybe(Type::StaticStr)) baseType |= Type::CountedStr;
  }
}

//////////////////////////////////////////////////////////////////////

}

// minstrBaseIdx returns the src index for inst's base operand.
int minstrBaseIdx(Opcode opc) {
  return opcodeHasFlags(opc, MInstrProp) ? 0 :
         opcodeHasFlags(opc, MInstrElem) ? 0 :
         bad_value<int>();
}

bool MInstrEffects::supported(Opcode op) {
  return opcodeHasFlags(op, MInstrProp | MInstrElem);
}
bool MInstrEffects::supported(const IRInstruction* inst) {
  return supported(inst->op());
}

void MInstrEffects::get(const IRInstruction* inst,
                        const FrameStateMgr& frame,
                        LocalStateHook& hook) {
  // If the base for this instruction is a local address, the helper call might
  // have side effects on the local's value
  auto const base = inst->src(minstrBaseIdx(inst->op()));
  auto const locInstr = base->inst();

  // Right now we require that the address of any affected local is the
  // immediate source of the base tmp.  This isn't actually specified in the ir
  // spec right now but will intend to make it more general soon.
  if (locInstr->op() != LdLocAddr) return;

  auto const locId = locInstr->extra<LdLocAddr>()->locId;
  auto const baseType = frame.localType(locId);

  MInstrEffects effects(inst->op(), baseType.ptr(Ptr::Frame));
  if (effects.baseTypeChanged || effects.baseValChanged) {
    auto const ty = effects.baseType.derefIfPtr();
    if (ty.isBoxed()) {
      hook.setLocalType(locId, Type::BoxedInitCell);
      hook.setBoxedLocalPrediction(locId, ty);
    } else {
      hook.setLocalType(locId, ty);
    }
  }

}

MInstrEffects::MInstrEffects(const Opcode rawOp, const Type origBase) {
  baseType = origBase;
  // Note: MInstrEffects wants to manipulate pointer types in some situations
  // for historical reasons.  We'll eventually change that.
  always_assert(baseType.isPtr() ^ baseType.notPtr());
  auto const basePtr = baseType.isPtr();
  auto const basePtrKind = basePtr ? baseType.ptrKind() : Ptr::Unk;
  baseType = baseType.derefIfPtr();

  // Only certain types of bases are supported now but this list may expand in
  // the future.
  assert_not_implemented(basePtr ||
                         baseType.subtypeOfAny(Type::Obj, Type::Arr));

  baseTypeChanged = baseValChanged = false;

  // Process the inner and outer types separately and then recombine them,
  // since the minstr operations all operate on the inner cell of boxed
  // bases. We treat the new inner type as a prediction because it will be
  // verified the next time we load from the box.
  auto inner = (baseType & Type::BoxedCell).innerType();
  auto outer = baseType & Type::Cell;
  getBaseType(rawOp, false, outer, baseValChanged);
  getBaseType(rawOp, true, inner, baseValChanged);

  baseType = inner.box() | outer;
  baseType = basePtr ? baseType.ptr(basePtrKind) : baseType;

  baseTypeChanged = baseType != origBase;

  /* Boxed bases may have their inner value changed but the value of the box
   * will never change. */
  baseValChanged = !origBase.isBoxed() && (baseValChanged || baseTypeChanged);
}

//////////////////////////////////////////////////////////////////////

}}
