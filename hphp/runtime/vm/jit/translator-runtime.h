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
struct c_Pair;
struct c_Vector;

namespace jit {
//////////////////////////////////////////////////////////////////////

struct ArrayKindProfile;
struct TypeConstraint;

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
template <bool intishWarn>
ArrayData* addElemStringKeyHelper(ArrayData* ad, StringData* key,
                                  TypedValue val);
ArrayData* dictAddElemIntKeyHelper(ArrayData* ad, int64_t key, TypedValue val);
ArrayData* dictAddElemStringKeyHelper(ArrayData* ad, StringData* key,
                                      TypedValue val);
void setNewElem(TypedValue* base, Cell val);
void setNewElemArray(TypedValue* base, Cell val);
void setNewElemVec(TypedValue* base, Cell val);
RefData* boxValue(TypedValue tv);
ArrayData* arrayAdd(ArrayData* a1, ArrayData* a2);
/* Helper functions for conversion instructions that are too
 * complicated to inline
 */
ArrayData* convCellToArrHelper(TypedValue tv);
ArrayData* convVecToArrHelper(ArrayData* a);
ArrayData* convDictToArrHelper(ArrayData* a);
ArrayData* convKeysetToArrHelper(ArrayData* a);
ArrayData* convArrToVecHelper(ArrayData* a);
ArrayData* convDictToVecHelper(ArrayData* a);
ArrayData* convKeysetToVecHelper(ArrayData* a);
ArrayData* convObjToVecHelper(ObjectData* o);
ArrayData* convCellToVecHelper(TypedValue tv);
ArrayData* convArrToDictHelper(ArrayData* a);
ArrayData* convVecToDictHelper(ArrayData* a);
ArrayData* convKeysetToDictHelper(ArrayData* a);
ArrayData* convObjToDictHelper(ObjectData* o);
ArrayData* convCellToDictHelper(TypedValue tv);
ArrayData* convArrToKeysetHelper(ArrayData* a);
ArrayData* convVecToKeysetHelper(ArrayData* a);
ArrayData* convDictToKeysetHelper(ArrayData* a);
ArrayData* convObjToKeysetHelper(ObjectData* o);
ArrayData* convCellToKeysetHelper(TypedValue tv);
int64_t convObjToDblHelper(const ObjectData* o);
int64_t convArrToDblHelper(ArrayData* a);
int64_t convStrToDblHelper(const StringData* s);
int64_t convResToDblHelper(const ResourceHdr* r);
int64_t convCellToDblHelper(TypedValue tv);
ObjectData* convCellToObjHelper(TypedValue tv);
StringData* convDblToStrHelper(int64_t i);
StringData* convIntToStrHelper(int64_t i);
StringData* convObjToStrHelper(ObjectData* o);
StringData* convResToStrHelper(ResourceHdr* o);
StringData* convCellToStrHelper(TypedValue tv);


bool coerceCellToBoolHelper(TypedValue tv, int64_t argNum, const Func* func);
int64_t coerceStrToDblHelper(StringData* sd, int64_t argNum, const Func* func);
int64_t coerceCellToDblHelper(TypedValue tv, int64_t argNum, const Func* func);
int64_t coerceStrToIntHelper(StringData* sd, int64_t argNum, const Func* func);
int64_t coerceCellToIntHelper(TypedValue tv, int64_t argNum, const Func* func);

void raiseUndefProp(ObjectData* base, const StringData* name);
void raiseUndefVariable(StringData* nm);
void VerifyParamTypeSlow(const Class* cls,
                         const Class* constraint,
                         const HPHP::TypeConstraint* expected,
                         int param);
void VerifyParamTypeCallable(TypedValue value, int param);
void VerifyParamTypeFail(int param);
void VerifyRetTypeSlow(const Class* cls,
                       const Class* constraint,
                       const HPHP::TypeConstraint* expected,
                       const TypedValue value);
void VerifyRetTypeCallable(TypedValue value);
void VerifyRetTypeFail(TypedValue* value);

void raise_error_sd(const StringData* sd);

TypedValue arrayIdxI(ArrayData*, int64_t, TypedValue);
TypedValue arrayIdxS(ArrayData*, StringData*, TypedValue);

template <bool intishWarn>
TypedValue arrayIdxSi(ArrayData*, StringData*, TypedValue);

TypedValue dictIdxI(ArrayData*, int64_t, TypedValue);
TypedValue dictIdxS(ArrayData*, StringData*, TypedValue);

TypedValue keysetIdxI(ArrayData*, int64_t, TypedValue);
TypedValue keysetIdxS(ArrayData*, StringData*, TypedValue);

TypedValue* ldGblAddrDefHelper(StringData* name);

TypedValue* getSPropOrNull(const Class* cls,
    const StringData* name, Class* ctx);
TypedValue* getSPropOrRaise(const Class* cls,
    const StringData* name, Class* ctx);

int64_t switchDoubleHelper(int64_t val, int64_t base, int64_t nTargets);
int64_t switchStringHelper(StringData* s, int64_t base, int64_t nTargets);
int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets);

void checkFrame(ActRec* fp, Cell* sp, bool fullCheck, Offset bcOff);

void loadArrayFunctionContext(ArrayData*, ActRec* preLiveAR, ActRec* fp);
void fpushCufHelperArray(ArrayData*, ActRec* preLiveAR, ActRec* fp);
void fpushCufHelperString(StringData*, ActRec* preLiveAR, ActRec* fp);

const Func* loadClassCtor(Class* cls, ActRec* fp);

// These shuffle* functions are the JIT's version of bytecode.cpp's
// shuffleExtraStackArgs
void trimExtraArgs(ActRec* ar);
void shuffleExtraArgsMayUseVV(ActRec* ar);
void shuffleExtraArgsVariadic(ActRec* ar);
void shuffleExtraArgsVariadicAndVV(ActRec* ar);

void raiseMissingArgument(const Func* func, int got);

Class* lookupClsRDS(const StringData* name);

/*
 * Insert obj into the set of live objects to be destructed at the end of the
 * request.
 */
void registerLiveObj(ObjectData* obj);

/* Check if a method of the given name exists on the class. */
bool methodExistsHelper(Class*, StringData*);

/*
 * Throw a VMSwitchMode exception.
 */
[[noreturn]] void throwSwitchMode();

[[noreturn]] void throwOOBException(TypedValue base, TypedValue key);
[[noreturn]] void invalidArrayKeyHelper(const ArrayData* ad, TypedValue key);

namespace MInstrHelpers {
template<bool intishWarn>
TypedValue setOpElem(TypedValue* base, TypedValue key, Cell val, SetOpOp op);
StringData* stringGetI(StringData*, uint64_t);
uint64_t pairIsset(c_Pair*, int64_t);
uint64_t vectorIsset(c_Vector*, int64_t);
template <bool intishWarn>
void bindElemC(TypedValue*, TypedValue, RefData*);
template <bool intishWarn>
void setWithRefElem(TypedValue*, TypedValue, TypedValue);
template<bool intishWarn>
TypedValue incDecElem(TypedValue* base, TypedValue key, IncDecOp op);
void bindNewElem(TypedValue* base, RefData* val);
TypedValue* elemVecID(TypedValue* base, int64_t key);
TypedValue* elemVecIU(TypedValue* base, int64_t key);
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

/*
 * Exception thrown to indicate that a parameter could not be coerced when
 * calling an HNI builtin function.
 */
struct TVCoercionException : std::runtime_error {
  TVCoercionException(const Func* func, int arg_num,
                      DataType actual, DataType expected);
  TypedValue tv() const { return m_tv; }
private:
  req::root<TypedValue> m_tv;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
