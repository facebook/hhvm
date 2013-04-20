/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "runtime/base/types.h"
#include "runtime/vm/translator/abi-x64.h"
#include "runtime/vm/translator/targetcache.h"

namespace HPHP { namespace VM { namespace Transl {

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
              < kReservedRSPScratchSpace,
              "MInstrState is too large for the rsp scratch space "
              "in enterTCHelper");

/* Helper functions for translated code */

ArrayData* addElemIntKeyHelper(ArrayData* ad, int64_t key, TypedValue val);
ArrayData* addElemStringKeyHelper(ArrayData* ad, StringData* key,
                                  TypedValue val);
TypedValue setNewElem(TypedValue* base, Cell val);
void bindNewElemIR(TypedValue* base, RefData* val, MInstrState* mis);
RefData* box_value(TypedValue tv);
ArrayData* convCellToArrHelper(TypedValue tv);

/* Helper functions for conversion instructions that are too
complicated to inline */

inline int64_t reinterpretDblAsInt(double d) {
  union {
    int64_t intval;
    double dblval;
  } u;
  u.dblval = d;
  return u.intval;
}

inline double reinterpretIntAsDbl(int64_t i) {
  union {
    int64_t intval;
    double dblval;
  } u;
  u.intval = i;
  return u.dblval;
}

int64_t convArrToBoolHelper(const ArrayData* a);
int64_t convStrToBoolHelper(const StringData* s);
int64_t convCellToBoolHelper(TypedValue tv);
int64_t convArrToDblHelper(ArrayData* a);
int64_t convStrToDblHelper(const StringData* s);
int64_t convCellToDblHelper(TypedValue tv);
int64_t convArrToIntHelper(ArrayData* a);
int64_t convDblToIntHelper(int64_t i);
int64_t convStrToIntHelper(const StringData* s);
int64_t convCellToIntHelper(TypedValue tv);
ObjectData* convCellToObjHelper(TypedValue tv);
StringData* convDblToStrHelper(int64_t i);
StringData* convIntToStrHelper(int64_t i);
StringData* convObjToStrHelper(ObjectData* o);
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

int64_t switchDoubleHelper(int64_t val, int64_t base, int64_t nTargets);
int64_t switchStringHelper(StringData* s, int64_t base, int64_t nTargets);
int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets);

RefData* staticLocInit(StringData* name, ActRec* fp, TypedValue val);
RefData* staticLocInitCached(StringData* name, ActRec* fp, TypedValue val,
                             TargetCache::CacheHandle ch);

} } }

#endif
