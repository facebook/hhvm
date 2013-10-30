/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP { namespace Transl {

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
  Class* ctx;
} __attribute__((aligned(16)));
static_assert(offsetof(MInstrState, tvScratch) % 16 == 0,
              "MInstrState members require 16-byte alignment for SSE");
static_assert(sizeof(MInstrState) - sizeof(uintptr_t) // return address
              < kReservedRSPTotalSpace,
              "MInstrState is too large for the rsp scratch space "
              "in enterTCHelper");

/* Helper functions for translated code */

/**
 * Only use in case of extreme shadiness.
 *
 * There are a few cases in the JIT where we allocate a pre-live ActRec on
 * the stack, call updateMarker() and then a CPP helper
 * (methodCacheSlowPath, loadArrayFunctionContext) to fill the ActRec with
 * te function that it needs to call. In the case that the helper throws an
 * exception, the unwinder sees a strange stack marker that has moved while
 * the bytecode offset remains unchanged and believes it has decref work to
 * do; we need the unwinder to ignore the half-built ActRec allocated on
 * the stack when building its back-trace and certainly to avoid attempting
 * to decref its contents. We achieve this by overwriting the ActRec cells
 * with null TypeValues.
 *
 * A TypedValue* is returned here to allow the CPP helper to write whatever
 * it needs to be decref'd into those eval cells, to ensure that the
 * unwinder leaves state the same as it was before the call into FPush
 * bytecode that started this whole stack trace.
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
ArrayData* array_add(ArrayData* a1, ArrayData* a2);

/* Helper functions for conversion instructions that are too
 * complicated to inline
 */
ArrayData* convCellToArrHelper(TypedValue tv);
int64_t convArrToBoolHelper(const ArrayData* a);
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
void VerifyParamTypeFail(int param);
void VerifyParamTypeCallable(TypedValue value, int param);
void VerifyParamTypeSlow(const Class* cls,
                         const Class* constraint,
                         int param,
                         const TypeConstraint* expected);

void raise_error_sd(const StringData* sd);

RefData* closureStaticLocInit(StringData* name, ActRec* fp, TypedValue val);

bool instanceOfHelper(const Class* objClass, const Class* testClass);

int64_t ak_exist_string(ArrayData* arr, StringData* key);
int64_t ak_exist_int(ArrayData* arr, int64_t key);
int64_t ak_exist_string_obj(ObjectData* obj, StringData* key);
int64_t ak_exist_int_obj(ObjectData* obj, int64_t key);

TypedValue arrayIdxI(ArrayData*, int64_t, TypedValue);
TypedValue arrayIdxS(ArrayData*, StringData*, TypedValue);
TypedValue arrayIdxSi(ArrayData*, StringData*, TypedValue);

TypedValue* ldGblAddrHelper(StringData* name);
TypedValue* ldGblAddrDefHelper(StringData* name);

int64_t switchDoubleHelper(int64_t val, int64_t base, int64_t nTargets);
int64_t switchStringHelper(StringData* s, int64_t base, int64_t nTargets);
int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets);

typedef FixedStringMap<TCA,true> SSwitchMap;
TCA sswitchHelperFast(const StringData* val, const SSwitchMap* table, TCA* def);

void tv_release_generic(TypedValue* tv);
void tv_release_typed(RefData* pv, DataType dt);

Cell lookupCnsHelper(const TypedValue* tv,
                     StringData* nm,
                     bool error);
Cell lookupCnsUHelper(const TypedValue* tv,
                      StringData* nm,
                      StringData* fallback);

void checkFrame(ActRec* fp, Cell* sp, bool checkLocals);
void traceCallback(ActRec* fp, Cell* sp, int64_t pcOff, void* rip);

void loadArrayFunctionContext(ArrayData* arr, ActRec* preLiveAR, ActRec* fp);

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
void setArgInActRec(ActRec* ar, int argNum, uint64_t datum, DataType t);
int shuffleArgsForMagicCall(ActRec* ar);

void raiseMissingArgument(const char* name, int expected, int got);

/*
 * Just calls tlsBase, but not inlined, so it can be called from the TC.
 */
uintptr_t tlsBaseNoInline();

}}

#endif
