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

#ifndef incl_HPHP_TRANSLATOR_RUNTIME_H_
#define incl_HPHP_TRANSLATOR_RUNTIME_H_

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/vm/bytecode.h"

#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/types.h"

struct _Unwind_Exception;

namespace HPHP {
//////////////////////////////////////////////////////////////////////

class Func;
class c_Pair;
class c_Vector;
struct MInstrState;

namespace jit {
//////////////////////////////////////////////////////////////////////

struct ClassProfile;
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
    tvWriteNull(actRecCell + ar_cell);
  }
  return actRecCell + HPHP::kNumActRecCells - 1;
}

ArrayData* addNewElemHelper(ArrayData* a, TypedValue value);
ArrayData* addElemIntKeyHelper(ArrayData* ad, int64_t key, TypedValue val);
ArrayData* addElemStringKeyHelper(ArrayData* ad, StringData* key,
                                  TypedValue val);
void setNewElem(TypedValue* base, Cell val);
void setNewElemArray(TypedValue* base, Cell val);
void bindNewElemIR(TypedValue* base, RefData* val, MInstrState* mis);
RefData* boxValue(TypedValue tv);
ArrayData* arrayAdd(ArrayData* a1, ArrayData* a2);
TypedValue setOpElem(TypedValue* base, TypedValue key,
                     Cell val, MInstrState* mis, SetOpOp op);
TypedValue incDecElem(TypedValue* base, TypedValue key,
                      MInstrState* mis, IncDecOp op);
/* Helper functions for conversion instructions that are too
 * complicated to inline
 */
ArrayData* convCellToArrHelper(TypedValue tv);
int64_t convArrToDblHelper(ArrayData* a);
int64_t convStrToDblHelper(const StringData* s);
int64_t convCellToDblHelper(TypedValue tv);
int64_t convArrToIntHelper(ArrayData* a);
ObjectData* convCellToObjHelper(TypedValue tv);
StringData* convDblToStrHelper(int64_t i);
StringData* convIntToStrHelper(int64_t i);
StringData* convObjToStrHelper(ObjectData* o);
StringData* convResToStrHelper(ResourceData* o);
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
void VerifyRetTypeFail(const TypedValue value);

void raise_error_sd(const StringData* sd);

RefData* closureStaticLocInit(StringData* name, ActRec* fp, TypedValue val);

bool ak_exist_string(ArrayData* arr, StringData* key);
bool ak_exist_string_obj(ObjectData* obj, StringData* key);
bool ak_exist_int_obj(ObjectData* obj, int64_t key);

TypedValue arrayIdxI(ArrayData*, int64_t, TypedValue);
TypedValue arrayIdxIc(ArrayData*, int64_t, TypedValue);
TypedValue arrayIdxS(ArrayData*, StringData*, TypedValue);
TypedValue arrayIdxSi(ArrayData*, StringData*, TypedValue);

TypedValue genericIdx(TypedValue, TypedValue, TypedValue);
TypedValue mapIdx(ObjectData*, StringData*, TypedValue);

TypedValue getMemoKeyHelper(TypedValue tv);

int32_t arrayVsize(ArrayData*);

TypedValue* ldGblAddrHelper(StringData* name);
TypedValue* ldGblAddrDefHelper(StringData* name);

TypedValue* getSPropOrNull(const Class* cls,
    const StringData* name, Class* ctx);
TypedValue* getSPropOrRaise(const Class* cls,
    const StringData* name, Class* ctx);

int64_t switchDoubleHelper(int64_t val, int64_t base, int64_t nTargets);
int64_t switchStringHelper(StringData* s, int64_t base, int64_t nTargets);
int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets);

typedef FixedStringMap<TCA,true> SSwitchMap;
TCA sswitchHelperFast(const StringData* val, const SSwitchMap* table, TCA* def);

void tv_release_generic(TypedValue* tv);

void profileObjClassHelper(ClassProfile*, ObjectData*);

Cell lookupCnsHelper(const TypedValue* tv,
                     StringData* nm,
                     bool error);
Cell lookupCnsUHelper(const TypedValue* tv,
                      StringData* nm,
                      StringData* fallback);
void lookupClsMethodHelper(Class* cls,
                           StringData* meth,
                           ActRec* ar,
                           ActRec* fp);

void checkFrame(ActRec* fp, Cell* sp, bool fullCheck, Offset bcOff);
void traceCallback(ActRec* fp, Cell* sp, Offset pcOff);

void loadArrayFunctionContext(ArrayData*, ActRec* preLiveAR, ActRec* fp);
void fpushCufHelperArray(ArrayData*, ActRec* preLiveAR, ActRec* fp);
void fpushCufHelperString(StringData*, ActRec* preLiveAR, ActRec* fp);

const Func* loadClassCtor(Class* cls);
const Func* lookupUnknownFunc(const StringData*);
const Func* lookupFallbackFunc(const StringData*, const StringData*);

Class* lookupKnownClass(Class** cache, const StringData* clsName);

TypedValue lookupClassConstantTv(TypedValue* cache,
                                 const NamedEntity* ne,
                                 const StringData* cls,
                                 const StringData* cns);

ObjectData* colAddNewElemCHelper(ObjectData* coll, TypedValue value);
ObjectData* colAddElemCHelper(ObjectData* coll, TypedValue key,
                              TypedValue value);

// These shuffle* functions are the JIT's version of bytecode.cpp's
// shuffleExtraStackArgs
void trimExtraArgs(ActRec* ar);
void shuffleExtraArgsMayUseVV(ActRec* ar);
void shuffleExtraArgsVariadic(ActRec* ar);
void shuffleExtraArgsVariadicAndVV(ActRec* ar);

void raiseMissingArgument(const Func* func, int got);

rds::Handle lookupClsRDSHandle(const StringData* name);

/*
 * Insert obj into the set of live objects to be destructed at the end of the
 * request.
 */
void registerLiveObj(ObjectData* obj);

/*
 * Throw a VMSwitchMode exception.
 */
void throwSwitchMode() ATTRIBUTE_NORETURN;

namespace MInstrHelpers {
StringData* stringGetI(StringData*, uint64_t);
uint64_t pairIsset(c_Pair*, int64_t);
uint64_t vectorIsset(c_Vector*, int64_t);
void bindElemC(TypedValue*, TypedValue, RefData*, MInstrState*);
void setWithRefElemC(TypedValue*, TypedValue, TypedValue, MInstrState*);
void setWithRefNewElem(TypedValue*, TypedValue, MInstrState*);
}

/*
 * Just calls tlsBase, but not inlined, so it can be called from the TC.
 */
uintptr_t tlsBaseNoInline();

//////////////////////////////////////////////////////////////////////

}}

#endif
