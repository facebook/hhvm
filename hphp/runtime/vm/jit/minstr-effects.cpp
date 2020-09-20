/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/jit/frame-state.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

template<typename T> T bad_value() { not_reached(); }

Opcode canonicalOp(Opcode op) {
  if (op == ElemUX || op == UnsetElem)     return UnsetElem;
  if (op == SetRange || op == SetRangeRev) return SetRange;

  return opcodeHasFlags(op, MInstrProp) ? SetProp
       : opcodeHasFlags(op, MInstrElem) ? SetElem
       : bad_value<Opcode>();
}

void getBaseType(Opcode rawOp, bool predict,
                 Type& baseType, bool& baseValChanged) {
  always_assert(baseType <= TCell);
  auto const op = canonicalOp(rawOp);

  // Deal with possible promotion to stdClass or array
  if ((op == SetElem || op == SetProp) &&
      baseType.maybe(TNull | TBool | TStr)) {
    auto newBase = op == SetProp ? TObj : TArr;

    if (predict) {
      /* If the output type will be used as a prediction and not as fact, we
       * can be optimistic here. Assume no promotion for string bases and
       * promotion in other cases. */
      baseType = baseType <= TStr ? TStr : newBase;
    } else if (baseType <= TStr && rawOp == SetElem) {
      /* If the base is known to be a string and the operation is exactly
       * SetElem, we're guaranteed that either the base will end as a
       * StaticStr or the instruction will throw an exception and side
       * exit. */
      baseType = TStaticStr;
    } else if (baseType <= TStr && rawOp == SetNewElem) {
      /* If the string base is empty, it will be promoted to an
       * array. Otherwise the base will be left alone and we'll fatal. */
      baseType = TArr;
    } else {
      /* Regardless of whether or not promotion happens, we know the base
       * cannot be Null after the operation. If the base was a subtype of Null
       * this will give newBase. */
      baseType = (baseType - TNull) | newBase;
    }

    baseValChanged = true;
  }

  if ((op == SetElem || op == SetRange || op == UnsetElem) &&
      baseType.maybe(TArrLike | TStr | TRecord | TClsMeth)) {
    /* Member operations never cause us to lose the vanilla bit. */
    auto const vanilla = baseType.arrSpec().vanilla();

    /* Modifying an array or string element, even when COW doesn't kick in,
     * produces a new SSATmp for the base. StaticArr/StaticStr may be promoted
     * to CountedArr/CountedStr. */
    baseValChanged = true;

    if (baseType.maybe(TVArr)) baseType |= TCountedVArr;
    if (baseType.maybe(TDArr)) baseType |= TCountedDArr;
    if (baseType.maybe(TVec)) baseType |= TCountedVec;
    if (baseType.maybe(TDict)) baseType |= TCountedDict;
    if (baseType.maybe(TKeyset)) baseType |= TCountedKeyset;
    if (baseType.maybe(TStr)) baseType |= TCountedStr;
    if (baseType.maybe(TClsMeth)) {
      baseType |= RO::EvalHackArrDVArrs ? TCountedVec : TCountedVArr;
    }
    if (vanilla) baseType = baseType.narrowToVanilla();
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

MInstrEffects::MInstrEffects(const Opcode rawOp, const Type origBase) {
  // Note: MInstrEffects wants to manipulate pointer types in some situations
  // for historical reasons.  We'll eventually change that.
  bool const is_ptr = origBase <= TLvalToCell;
  auto const basePtr = is_ptr ? origBase.ptrKind() : Ptr::Bottom;
  baseType = origBase.derefIfPtr();

  baseTypeChanged = baseValChanged = false;

  getBaseType(rawOp, false, baseType, baseValChanged);

  baseType = is_ptr ? baseType.lval(basePtr) : baseType;
  baseTypeChanged = baseType != origBase;
  baseValChanged = baseValChanged || baseTypeChanged;
}

//////////////////////////////////////////////////////////////////////

}}
