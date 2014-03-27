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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/bytecode.h"

namespace HPHP { namespace JIT {


/* MInstrState is stored right above the reserved spill space on the C++
 * stack. */
#define MISOFF(nm)                                         \
  (offsetof(MInstrState, nm) + kReservedRSPSpillSpace)

const size_t kReservedRSPMInstrStateSpace = RESERVED_STACK_MINSTR_STATE_SPACE;
const size_t kReservedRSPSpillSpace       = RESERVED_STACK_SPILL_SPACE;
const size_t kReservedRSPTotalSpace       = RESERVED_STACK_TOTAL_SPACE;

//////////////////////////////////////////////////////////////////////

struct MInstrState {
  // Room for this structure is allocated on the stack before we
  // make a call into the tc, so this first element is padding for
  // the return address pushed by the call.
  uintptr_t returnAddress;
  uintptr_t padding; // keep the following TV's SSE friendly.
  union {
    // This space is used for both vector instructions and
    // the return value of builtin functions that return by reference.
    // Since we don't ever use the two at the same time, it is
    // OK to use a union.
    TypedValue tvScratch;
    TypedValue tvBuiltinReturn;
  };
  TypedValue tvRef;
  TypedValue tvRef2;
  TypedValue tvResult;
  TypedValue tvVal;
} __attribute__((aligned(16)));
static_assert(offsetof(MInstrState, tvScratch) % 16 == 0,
              "MInstrState members require 16-byte alignment for SSE");
static_assert(sizeof(MInstrState) - sizeof(uintptr_t) // return address
              < kReservedRSPTotalSpace,
              "MInstrState is too large for the rsp scratch space "
              "in enterTCHelper");

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
  return actRecCell;
}

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

void raisePropertyOnNonObject();
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

int64_t ak_exist_string(ArrayData* arr, StringData* key);
int64_t ak_exist_int(ArrayData* arr, int64_t key);
int64_t ak_exist_string_obj(ObjectData* obj, StringData* key);
int64_t ak_exist_int_obj(ObjectData* obj, int64_t key);

TypedValue arrayIdxI(ArrayData*, int64_t, TypedValue);
TypedValue arrayIdxS(ArrayData*, StringData*, TypedValue);
TypedValue arrayIdxSi(ArrayData*, StringData*, TypedValue);

TypedValue genericIdx(TypedValue, TypedValue, TypedValue);

int32_t arrayVsize(ArrayData*);

TypedValue* ldGblAddrHelper(StringData* name);
TypedValue* ldGblAddrDefHelper(StringData* name);

int64_t switchDoubleHelper(int64_t val, int64_t base, int64_t nTargets);
int64_t switchStringHelper(StringData* s, int64_t base, int64_t nTargets);
int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets);

typedef FixedStringMap<TCA,true> SSwitchMap;
TCA sswitchHelperFast(const StringData* val, const SSwitchMap* table, TCA* def);

void tv_release_generic(TypedValue* tv);

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

void checkFrame(ActRec* fp, Cell* sp, bool checkLocals);
void traceCallback(ActRec* fp, Cell* sp, int64_t pcOff, void* rip);

void loadArrayFunctionContext(ArrayData*, ActRec* preLiveAR, ActRec* fp);
void fpushCufHelperArray(ArrayData*, ActRec* preLiveAR, ActRec* fp);
void fpushCufHelperString(StringData*, ActRec* preLiveAR, ActRec* fp);

const Func* lookupUnknownFunc(const StringData*);

Class* lookupKnownClass(Class** cache, const StringData* clsName);

TypedValue lookupClassConstantTv(TypedValue* cache,
                                 const NamedEntity* ne,
                                 const StringData* cls,
                                 const StringData* cns);

ObjectData* newColHelper(uint32_t type, uint32_t size);
ObjectData* colAddNewElemCHelper(ObjectData* coll, TypedValue value);
ObjectData* colAddElemCHelper(ObjectData* coll, TypedValue key,
                              TypedValue value);

void trimExtraArgs(ActRec* ar);

void raiseMissingArgument(const char* name, int expected, int got);

RDS::Handle lookupClsRDSHandle(const StringData* name);

/*
 * Just calls tlsBase, but not inlined, so it can be called from the TC.
 */
uintptr_t tlsBaseNoInline();

}}

#endif
