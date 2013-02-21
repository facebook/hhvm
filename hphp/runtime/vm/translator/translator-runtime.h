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
#ifndef incl_TRANSLATOR_RUNTIME_H_
#define incl_TRANSLATOR_RUNTIME_H_

#include "runtime/base/types.h"
#include "runtime/vm/translator/abi-x64.h"

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
  bool baseStrOff;
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
RefData* box_value(TypedValue tv);
void raisePropertyOnNonObject();
void raiseUndefProp(ObjectData* base, const StringData* name);

int64_t switchDoubleHelper(int64_t val, int64_t base, int64_t nTargets);
int64_t switchStringHelper(StringData* s, int64_t base, int64_t nTargets);
int64_t switchObjHelper(ObjectData* o, int64_t base, int64_t nTargets);

} } }

#endif
