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

#ifndef incl_HPHP_TRANSLATOR_RUNTIME_H_
#define incl_HPHP_TRANSLATOR_RUNTIME_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/bytecode.h"

#include "hphp/runtime/vm/jit/types.h"

struct _Unwind_Exception;

namespace HPHP {
//////////////////////////////////////////////////////////////////////

struct Func;
struct Iter;
struct MInstrState;
struct TypeConstraint;
struct c_Pair;
struct c_Vector;
enum class ICMode : int8_t;

namespace jit {
//////////////////////////////////////////////////////////////////////

struct ArrayKindProfile;

//////////////////////////////////////////////////////////////////////

/* Helper functions for translated code */

/*
 * Only use in case of extreme shadiness.
 *
 * There are a few cases in the JIT where we allocate a pre-live
 * ActRec on the stack, and then call a helper that may re-enter the
 * VM (e.g. for autoload) to do the rest of the work filling it out.
 * Examples are methodCacheSlowPath or loadArrayFunctionContext.
 *
 * In these situations, we set up a "strange marker" by calling
 * updateMarker() before the instruction is done (but after the
 * pre-live ActRec is pushed).  This marker will have a lower SP than
 * the start of the instruction, but the PC will still point at the
 * instruction.  This is done so that if we need to re-enter from the
 * C++ helper we don't clobber the pre-live ActRec.
 *
 * However, if we throw, the unwinder won't think we're in the FPI
 * region yet.  So in the case that the helper throws an exception,
 * the unwinder will believe it has to decref three normal stack slots
 * (where the pre-live ActRec is).  We need the unwinder to ignore the
 * half-built ActRec allocated on the stack and certainly to avoid
 * attempting to decref its contents.  We achieve this by overwriting
 * the ActRec cells with nulls.
 *
 * A TypedValue* is also returned here to allow the CPP helper to
 * write whatever it needs to be decref'd into one of the eval cells,
 * to ensure that the unwinder leaves state the same as it was before
 * the call into FPush bytecode that threw.
 */
inline TypedValue* arPreliveOverwriteCells(ActRec *preLiveAR) {
  auto actRecCell = reinterpret_cast<TypedValue*>(preLiveAR);
  for (size_t ar_cell = 0; ar_cell < HPHP::kNumActRecCells; ++ar_cell) {
    tvWriteNull(*(actRecCell + ar_cell));
  }
  return actRecCell + HPHP::kNumActRecCells - 1;
}

ArrayData* addNewElemHelper(ArrayData* a, TypedValue value);
ArrayData* addElemIntKeyHelper(ArrayData* ad, int64_t key, TypedValue val);
template <ICMode intishCast>
ArrayData* addElemStringKeyHelper(ArrayData* ad, StringData* key,
                                  TypedValue val);
ArrayData* dictAddElemIntKeyHelper(ArrayData* ad, int64_t key, TypedValue val);
ArrayData* dictAddElemStringKeyHelper(ArrayData* ad, StringData* key,
                                      TypedValue val);
RefData* boxValue(TypedValue tv);
ArrayData* arrayAdd(ArrayData* a1, ArrayData* a2);
/* Helper functions for conversion instructions that are too
 * complicated to inline
 */
ArrayData* convCellToArrHelper(TypedValue tv);
ArrayData* convArrToNonDVArrHelper(ArrayData* a);
ArrayData* convVecToArrHelper(ArrayData* a);
ArrayData* convDictToArrHelper(ArrayData* a);
ArrayData* convShapeToArrHelper(ArrayData* a);
ArrayData* convKeysetToArrHelper(ArrayData* a);
ArrayData* convArrToVecHelper(ArrayData* a);
ArrayData* convDictToVecHelper(ArrayData* a);
ArrayData* convShapeToVecHelper(ArrayData* a);
ArrayData* convKeysetToVecHelper(ArrayData* a);
ArrayData* convObjToVecHelper(ObjectData* o);
ArrayData* convCellToVecHelper(TypedValue tv);
ArrayData* convArrToDictHelper(ArrayData* a);
ArrayData* convShapeToDictHelper(ArrayData* a);
ArrayData* convVecToDictHelper(ArrayData* a);
ArrayData* convKeysetToDictHelper(ArrayData* a);
ArrayData* convObjToDictHelper(ObjectData* o);
ArrayData* convCellToDictHelper(TypedValue tv);
ArrayData* convArrToKeysetHelper(ArrayData* a);
ArrayData* convVecToKeysetHelper(ArrayData* a);
ArrayData* convDictToKeysetHelper(ArrayData* a);
ArrayData* convShapeToKeysetHelper(ArrayData* a);
ArrayData* convObjToKeysetHelper(ObjectData* o);
ArrayData* convCellToKeysetHelper(TypedValue tv);
double convObjToDblHelper(const ObjectData* o);
double convArrToDblHelper(ArrayData* a);
double convStrToDblHelper(const StringData* s);
double convResToDblHelper(const ResourceHdr* r);
double convCellToDblHelper(TypedValue tv);
ObjectData* convCellToObjHelper(TypedValue tv);
StringData* convDblToStrHelper(double i);
StringData* convIntToStrHelper(int64_t i);
StringData* convObjToStrHelper(ObjectData* o);
StringData* convResToStrHelper(ResourceHdr* o);


bool coerceCellToBoolHelper(TypedValue tv, int64_t argNum, const Func* func);
double coerceStrToDblHelper(StringData* sd, int64_t argNum, const Func* func);
double coerceCellToDblHelper(TypedValue tv, int64_t argNum, const Func* func);
int64_t coerceStrToIntHelper(StringData* sd, int64_t argNum, const Func* func);
int64_t coerceCellToIntHelper(TypedValue tv, int64_t argNum, const Func* func);


void raiseUndefProp(ObjectData* base, const StringData* name);
void raiseUndefVariable(StringData* nm);
void VerifyParamTypeSlow(const Class* cls,
                         const Class* constraint,
                         const TypeConstraint* expected,
                         int param);
void VerifyParamTypeCallable(TypedValue value, int param);
void VerifyParamTypeFail(int param);
void VerifyRetTypeSlow(int32_t id,
                       const Class* cls,
                       const Class* constraint,
                       const TypeConstraint* expected,
                       const TypedValue value);
void VerifyRetTypeCallable(int32_t id, TypedValue value);
void VerifyRetTypeFail(int32_t id, TypedValue* value);

void VerifyReifiedLocalTypeImpl(int32_t, ArrayData*);
void VerifyReifiedReturnTypeImpl(TypedValue, ArrayData*);

void raise_error_sd(const StringData* sd);

TypedValue arrayIdxI(ArrayData*, int64_t, TypedValue);
TypedValue arrayIdxS(ArrayData*, StringData*, TypedValue);

template <ICMode intishCast>
TypedValue arrayIdxSi(ArrayData*, StringData*, TypedValue);

TypedValue dictIdxI(ArrayData*, int64_t, TypedValue);
TypedValue dictIdxS(ArrayData*, StringData*, TypedValue);

TypedValue keysetIdxI(ArrayData*, int64_t, TypedValue);
TypedValue keysetIdxS(ArrayData*, StringData*, TypedValue);

// Get the first/last key or value from ArrLike type.
template <bool isFirst>
TypedValue vecFirstLast(ArrayData* a);
template <bool isFirst, bool isKey>
TypedValue arrFirstLast(ArrayData* a);

TypedValue* ldGblAddrDefHelper(StringData* name);

TypedValue* getSPropOrNull(const Class* cls,
                           const StringData* name,
                           Class* ctx,
                           bool ignoreLateInit);
TypedValue* getSPropOrRaise(const Class* cls,
                            const StringData* name,
                            Class* ctx,
                            bool ignoreLateInit);

int64_t switchDoubleHelper(double val, int64_t base, int64_t nTargets);
int64_t switchStringHelper(StringData* s, int64_t base, int64_t nTargets);
int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets);

void checkFrame(ActRec* fp, Cell* sp, bool fullCheck, Offset bcOff);

void loadArrayFunctionContext(ArrayData*, ActRec* preLiveAR, ActRec* fp);

const Func* loadClassCtor(Class* cls, ActRec* fp);

// These shuffle* functions are the JIT's version of bytecode.cpp's
// shuffleExtraStackArgs
void trimExtraArgs(ActRec* ar);
void shuffleExtraArgsMayUseVV(ActRec* ar);
void shuffleExtraArgsVariadic(ActRec* ar);
void shuffleExtraArgsVariadicAndVV(ActRec* ar);

void raiseMissingArgument(const Func* func, int got);
void raiseTooManyArguments(const Func* func, int got);

Class* lookupClsRDS(const StringData* name);

/* Check if a method of the given name exists on the class. */
bool methodExistsHelper(Class*, StringData*);

/* Is/As Helpers */
ArrayData* resolveTypeStructHelper(
  uint32_t n,
  const TypedValue* values,
  const Class* declaringCls,
  const Class* calledCls,
  bool suppress,
  bool isOrAsOp
);
bool isTypeStructHelper(ArrayData*, Cell);
void asTypeStructHelper(ArrayData*, Cell);

/* Reified generics helpers */
StringData*
recordReifiedGenericsAndGetName(uint32_t n, const TypedValue* values);
ArrayData*
recordReifiedGenericsAndGetTSList(uint32_t n, const TypedValue* values);
/*
 * Throw a VMSwitchMode exception.
 */
[[noreturn]] void throwSwitchMode();

[[noreturn]] void throwOOBException(TypedValue base, TypedValue key);
[[noreturn]] void invalidArrayKeyHelper(const ArrayData* ad, TypedValue key);

namespace MInstrHelpers {
void setNewElem(tv_lval base, Cell val, const MInstrPropState*);
void setNewElemArray(tv_lval base, Cell val);
void setNewElemVec(tv_lval base, Cell val);
template<ICMode intishCast>
TypedValue setOpElem(tv_lval base, TypedValue key, Cell val, SetOpOp op,
                     const MInstrPropState*);
StringData* stringGetI(StringData*, uint64_t);
uint64_t pairIsset(c_Pair*, int64_t);
uint64_t vectorIsset(c_Vector*, int64_t);
template <ICMode intishCast>
void bindElemC(tv_lval, TypedValue, RefData*, const MInstrPropState*);
template<ICMode intishCast>
TypedValue incDecElem(tv_lval base, TypedValue key, IncDecOp op,
                      const MInstrPropState*);
void bindNewElem(tv_lval base, RefData* val, const MInstrPropState*);
tv_lval elemVecID(tv_lval base, int64_t key);
tv_lval elemVecIU(tv_lval base, int64_t key);
}

/*
 * Just calls tlsBase, but not inlined, so it can be called from the TC.
 */
uintptr_t tlsBaseNoInline();

//////////////////////////////////////////////////////////////////////

/*
 * If the current builtin function `func' was called in a strict context,
 * verify that `tv' is the correct type for `argNum' or attempt to convert it
 * to the correct type, fataling on failure.
 *
 * If PHP7_ScalarType is false or EnableHipHopSyntax is true, this call does
 * nothing.
 */
void tvCoerceIfStrict(TypedValue& tv, int64_t argNum, const Func* func);

//////////////////////////////////////////////////////////////////////

}}

#endif
