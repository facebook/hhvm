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

#pragma once

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/bytecode.h"

#include "hphp/runtime/vm/jit/is-type-struct-profile.h"
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

/* Helper functions for translated code */

void setNewElem(tv_lval base, TypedValue val);
void setNewElemVec(tv_lval base, TypedValue val);
void setNewElemDict(tv_lval base, TypedValue val);
ArrayData* addNewElemVec(ArrayData* keyset, TypedValue v);
ArrayData* addNewElemKeyset(ArrayData* keyset, TypedValue v);
ArrayData* addElemIntKeyHelper(ArrayData* ad, int64_t key, TypedValue val);
ArrayData* addElemStringKeyHelper(ArrayData* ad, StringData* key,
                                  TypedValue val);
ArrayData* dictAddElemIntKeyHelper(ArrayData* ad, int64_t key, TypedValue val);
ArrayData* dictAddElemStringKeyHelper(ArrayData* ad, StringData* key,
                                      TypedValue val);
/* Helper functions for conversion instructions that are too
 * complicated to inline
 */
ArrayData* convArrLikeToVecHelper(ArrayData* a);
ArrayData* convObjToVecHelper(ObjectData* o);
ArrayData* convArrLikeToDictHelper(ArrayData* a);
ArrayData* convObjToDictHelper(ObjectData* o);
ArrayData* convArrLikeToKeysetHelper(ArrayData* a);
ArrayData* convObjToKeysetHelper(ObjectData* o);
ArrayData* convClsMethToVecHelper(ClsMethDataRef clsmeth);
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

void raiseUndefProp(ObjectData* base, const StringData* name);
void throwUndefVariable(StringData* nm);
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

tv_lval ldGblAddrDefHelper(StringData* name);

TypedValue* getSPropOrNull(const Class* cls,
                           const StringData* name,
                           Class* ctx,
                           bool ignoreLateInit,
                           bool disallowConst,
                           bool mustBeMutable,
                           bool mustBeReadOnly);
TypedValue* getSPropOrRaise(const Class* cls,
                            const StringData* name,
                            Class* ctx,
                            bool ignoreLateInit,
                            bool disallowConst,
                            bool mustBeMutable,
                            bool mustBeReadOnly);

int64_t switchDoubleHelper(double val, int64_t base, int64_t nTargets);
int64_t switchStringHelper(StringData* s, int64_t base, int64_t nTargets);
int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets);

void checkFrame(ActRec* fp, TypedValue* sp, bool fullCheck);

void loadArrayFunctionContext(ArrayData*, ActRec* preLiveAR, ActRec* fp);

const Func* loadClassCtor(Class* cls, Class* ctx);
const Func* lookupClsMethodHelper(const Class* cls, const StringData* methName,
                                  ObjectData* obj, const Class* ctx);

TypedValue lookupClsCns(const Class* cls, const StringData* cnsName);
int lookupClsCtxCns(const Class* cls, const StringData* cnsName);

// These shuffle* functions are the JIT's version of bytecode.cpp's
// shuffleExtraStackArgs
void trimExtraArgs(ActRec* ar);
void shuffleExtraArgsVariadic(ActRec* ar);

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
bool isTypeStructHelper(ArrayData*, TypedValue, rds::Handle);
void profileIsTypeStructHelper(ArrayData*, IsTypeStructProfile*);
[[noreturn]] void throwAsTypeStructExceptionHelper(ArrayData*, TypedValue);
ArrayData* errorOnIsAsExpressionInvalidTypesHelper(ArrayData*);

/* Reified generics helpers
 * Both functions decref the input array by turning it into a static array
 */
ArrayData* recordReifiedGenericsAndGetTSList(ArrayData*);

ArrayData* loadClsTypeCnsHelper(
  const Class* cls,
  const StringData* name,
  bool no_throw_on_undefined
);

void raiseCoeffectsCallViolationHelper(const Func*, uint64_t, uint64_t);

[[noreturn]] void throwOOBException(TypedValue base, TypedValue key);
[[noreturn]] void invalidArrayKeyHelper(const ArrayData* ad, TypedValue key);

namespace MInstrHelpers {
TypedValue setOpElem(tv_lval base, TypedValue key, TypedValue val, SetOpOp op);
StringData* stringGetI(StringData*, uint64_t);
uint64_t pairIsset(c_Pair*, int64_t);
uint64_t vectorIsset(c_Vector*, int64_t);
TypedValue incDecElem(tv_lval base, TypedValue key, IncDecOp op);
tv_lval elemVecIU(tv_lval base, int64_t key);
}

//////////////////////////////////////////////////////////////////////

}}
