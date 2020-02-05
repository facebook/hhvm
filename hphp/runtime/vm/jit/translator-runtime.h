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
struct ClsMethDataRef;
struct Iter;
struct MInstrState;
struct TypeConstraint;
struct c_Pair;
struct c_Vector;

namespace jit {
//////////////////////////////////////////////////////////////////////

struct ArrayKindProfile;

//////////////////////////////////////////////////////////////////////

/* Helper functions for translated code */

ArrayData* addNewElemHelper(ArrayData* a, TypedValue value);
ArrayData* addElemIntKeyHelper(ArrayData* ad, int64_t key, TypedValue val);
ArrayData* addElemStringKeyHelper(ArrayData* ad, StringData* key,
                                  TypedValue val);
ArrayData* dictAddElemIntKeyHelper(ArrayData* ad, int64_t key, TypedValue val);
ArrayData* dictAddElemStringKeyHelper(ArrayData* ad, StringData* key,
                                      TypedValue val);
ArrayData* arrayAdd(ArrayData* a1, ArrayData* a2);
/* Helper functions for conversion instructions that are too
 * complicated to inline
 */
ArrayData* convTVToArrHelper(TypedValue tv);
ArrayData* convArrToNonDVArrHelper(ArrayData* a);
ArrayData* convVecToArrHelper(ArrayData* a);
ArrayData* convDictToArrHelper(ArrayData* a);
ArrayData* convKeysetToArrHelper(ArrayData* a);
ArrayData* convArrToVecHelper(ArrayData* a);
ArrayData* convDictToVecHelper(ArrayData* a);
ArrayData* convKeysetToVecHelper(ArrayData* a);
ArrayData* convObjToVecHelper(ObjectData* o);
ArrayData* convArrToDictHelper(ArrayData* a);
ArrayData* convVecToDictHelper(ArrayData* a);
ArrayData* convKeysetToDictHelper(ArrayData* a);
ArrayData* convObjToDictHelper(ObjectData* o);
ArrayData* convArrToKeysetHelper(ArrayData* a);
ArrayData* convVecToKeysetHelper(ArrayData* a);
ArrayData* convDictToKeysetHelper(ArrayData* a);
ArrayData* convObjToKeysetHelper(ObjectData* o);
ArrayData* convClsMethToArrHelper(ClsMethDataRef clsmeth);
ArrayData* convClsMethToVArrHelper(ClsMethDataRef clsmeth);
ArrayData* convClsMethToVecHelper(ClsMethDataRef clsmeth);
ArrayData* convClsMethToDArrHelper(ClsMethDataRef clsmeth);
ArrayData* convClsMethToDictHelper(ClsMethDataRef clsmeth);
ArrayData* convClsMethToKeysetHelper(ClsMethDataRef clsmeth);
double convObjToDblHelper(const ObjectData* o);
double convArrToDblHelper(ArrayData* a);
double convStrToDblHelper(const StringData* s);
double convResToDblHelper(const ResourceHdr* r);
double convTVToDblHelper(TypedValue tv);
StringData* convDblToStrHelper(double i);
StringData* convIntToStrHelper(int64_t i);
StringData* convObjToStrHelper(ObjectData* o);
StringData* convResToStrHelper(ResourceHdr* o);

void raiseUndefProp(ObjectData* base, const StringData* name);
void raiseUndefVariable(StringData* nm);
void VerifyParamTypeSlow(const Class* cls,
                         const Class* constraint,
                         const TypeConstraint* expected,
                         int param);
void VerifyParamTypeCallable(TypedValue value, int param);
void VerifyParamTypeFail(int param, const TypeConstraint*);
void VerifyRetTypeSlow(int32_t id,
                       const Class* cls,
                       const Class* constraint,
                       const TypeConstraint* expected,
                       const TypedValue value);
void VerifyRetTypeCallable(int32_t id, TypedValue value);
void VerifyRetTypeFail(int32_t id, TypedValue* value, const TypeConstraint* tc);

void VerifyReifiedLocalTypeImpl(int32_t, ArrayData*);
void VerifyReifiedReturnTypeImpl(TypedValue, ArrayData*);

void VerifyParamRecDescImpl(const RecordDesc* rec,
                            const RecordDesc* constraint,
                            const TypeConstraint* expected,
                            int param);
void VerifyRetRecDescImpl(int32_t id,
                          const RecordDesc* rec,
                          const RecordDesc* constraint,
                          const TypeConstraint* expected,
                          TypedValue val);

void raise_error_sd(const StringData* sd);

void raiseClsMethPropConvertNotice(
  const TypeConstraint*, bool, const Class*, const StringData*);

TypedValue arrayIdxI(ArrayData*, int64_t, TypedValue);
TypedValue arrayIdxS(ArrayData*, StringData*, TypedValue);
TypedValue arrayIdxScan(ArrayData*, StringData*, TypedValue);

TypedValue dictIdxI(ArrayData*, int64_t, TypedValue);
TypedValue dictIdxS(ArrayData*, StringData*, TypedValue);
TypedValue dictIdxScan(ArrayData*, StringData*, TypedValue);

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
                           bool ignoreLateInit,
                           bool disallowConst);
TypedValue* getSPropOrRaise(const Class* cls,
                            const StringData* name,
                            Class* ctx,
                            bool ignoreLateInit,
                            bool disallowConst);

int64_t switchDoubleHelper(double val, int64_t base, int64_t nTargets);
int64_t switchStringHelper(StringData* s, int64_t base, int64_t nTargets);
int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets);

void checkFrame(ActRec* fp, TypedValue* sp, bool fullCheck);

void loadArrayFunctionContext(ArrayData*, ActRec* preLiveAR, ActRec* fp);

const Func* loadClassCtor(Class* cls, ActRec* fp);
const Func* lookupClsMethodHelper(const Class* cls, const StringData* methName,
                                  ObjectData* obj, const Class* ctx);

// These shuffle* functions are the JIT's version of bytecode.cpp's
// shuffleExtraStackArgs
void trimExtraArgs(ActRec* ar);
void shuffleExtraArgsVariadic(ActRec* ar);

[[noreturn]] void throwMissingArgument(const Func* func, int got);
void raiseTooManyArguments(const Func* func, int got);
void raiseTooManyArgumentsPrologue(const Func* func, ArrayData* unpackArgs);

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
bool isTypeStructHelper(ArrayData*, TypedValue);
[[noreturn]] void throwAsTypeStructExceptionHelper(ArrayData*, TypedValue);
ArrayData* errorOnIsAsExpressionInvalidTypesHelper(ArrayData*);

/* Reified generics helpers
 * Both functions decref the input array by turning it into a static array
 */
ArrayData* recordReifiedGenericsAndGetTSList(ArrayData*);

[[noreturn]] void throwOOBException(TypedValue base, TypedValue key);
[[noreturn]] void invalidArrayKeyHelper(const ArrayData* ad, TypedValue key);

namespace MInstrHelpers {
void setNewElem(tv_lval base, TypedValue val, const MInstrPropState*);
void setNewElemArray(tv_lval base, TypedValue val);
void setNewElemVec(tv_lval base, TypedValue val);
TypedValue setOpElem(tv_lval base, TypedValue key, TypedValue val, SetOpOp op,
                     const MInstrPropState*);
StringData* stringGetI(StringData*, uint64_t);
uint64_t pairIsset(c_Pair*, int64_t);
uint64_t vectorIsset(c_Vector*, int64_t);
TypedValue incDecElem(tv_lval base, TypedValue key, IncDecOp op,
                      const MInstrPropState*);
tv_lval elemVecID(tv_lval base, int64_t key);
tv_lval elemVecIU(tv_lval base, int64_t key);
}

/*
 * Just calls tlsBase, but not inlined, so it can be called from the TC.
 */
uintptr_t tlsBaseNoInline();

//////////////////////////////////////////////////////////////////////

}}

#endif
